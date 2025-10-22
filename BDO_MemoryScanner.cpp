#include "pch.h"
#include "BDO_MemoryScanner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <regex>
#include <TlHelp32.h>
#include <Psapi.h>

BDOMemoryScanner::BDOMemoryScanner(size_t threadCount) 
    : hProcess(NULL), processId(0), threadCount(threadCount) {
    if (threadCount == 0) {
        this->threadCount = std::thread::hardware_concurrency();
        if (this->threadCount == 0) {
            this->threadCount = 4; // Default to 4 threads
        }
    }
}

BDOMemoryScanner::~BDOMemoryScanner() {
    Shutdown();
}

bool BDOMemoryScanner::Initialize(DWORD pid) {
    if (pid == 0) {
        return false;
    }
    
    processId = pid;
    hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, processId);
    
    if (!hProcess) {
        return false;
    }
    
    return true;
}

bool BDOMemoryScanner::AttachToProcess(const std::wstring& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    DWORD targetPid = 0;
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (wcscmp(pe32.szExeFile, processName.c_str()) == 0) {
                targetPid = pe32.th32ProcessID;
                break;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    
    if (targetPid == 0) {
        return false;
    }
    
    return Initialize(targetPid);
}

void BDOMemoryScanner::Shutdown() {
    StopScan();
    
    // Wait for all threads to finish
    for (auto& thread : scanThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    scanThreads.clear();
    
    // Unfreeze all values
    {
        std::lock_guard<std::mutex> lock(frozenMutex);
        for (auto& [address, frozen] : frozenValues) {
            frozen->shouldFreeze = false;
            if (frozen->freezeThread.joinable()) {
                frozen->freezeThread.join();
            }
        }
        frozenValues.clear();
    }
    
    if (hProcess) {
        CloseHandle(hProcess);
        hProcess = NULL;
    }
    
    processId = 0;
    currentResults.clear();
    previousResults.clear();
    savedScans.clear();
    memoryRegions.clear();
}

void BDOMemoryScanner::EnumerateMemoryRegions(const ScanFilter& filter) {
    memoryRegions.clear();
    
    MEMORY_BASIC_INFORMATION mbi;
    PVOID address = 0;
    SIZE_T totalSize = 0;
    
    while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.State == MEM_COMMIT && IsValidRegion({mbi.BaseAddress, mbi.RegionSize, mbi.Protect, mbi.State, mbi.Type}, filter)) {
            memoryRegions.push_back({mbi.BaseAddress, mbi.RegionSize, mbi.Protect, mbi.State, mbi.Type});
            totalSize += mbi.RegionSize;
        }
        address = (PUCHAR)mbi.BaseAddress + mbi.RegionSize;
    }
    
    progress.totalBytes = totalSize;
}

bool BDOMemoryScanner::IsValidRegion(const MemoryRegion& region, const ScanFilter& filter) {
    // Check protection
    if (region.protection < filter.minProtection) {
        return false;
    }
    
    // Check size
    if (region.size < filter.minSize || region.size > filter.maxSize) {
        return false;
    }
    
    // Check type
    if (filter.privateMemory && !(region.type & MEM_PRIVATE)) {
        return false;
    }
    
    if (filter.mapped && !(region.type & MEM_MAPPED)) {
        return false;
    }
    
    // Check if executable
    if (filter.executable && !(region.protection & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) {
        return false;
    }
    
    // Check if writable
    if (filter.writable && !(region.protection & (PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE))) {
        return false;
    }
    
    // Check include/exclude addresses
    if (!filter.excludeAddresses.empty()) {
        for (PVOID excludeAddr : filter.excludeAddresses) {
            if (excludeAddr >= region.baseAddress && excludeAddr < (PUCHAR)region.baseAddress + region.size) {
                return false;
            }
        }
    }
    
    if (!filter.includeAddresses.empty()) {
        bool found = false;
        for (PVOID includeAddr : filter.includeAddresses) {
            if (includeAddr >= region.baseAddress && includeAddr < (PUCHAR)region.baseAddress + region.size) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    
    return true;
}

bool BDOMemoryScanner::FirstScan(ValueType type, const std::string& valueStr, ScanType scanType, ScanAlignment alignment, const ScanFilter& filter) {
    if (progress.isRunning) {
        return false;
    }
    
    // Clear previous results
    previousResults.clear();
    currentResults.clear();
    
    // Convert string to value
    auto value = StringToValue(valueStr, type);
    
    // Enumerate memory regions
    EnumerateMemoryRegions(filter);
    
    if (memoryRegions.empty()) {
        return false;
    }
    
    // Start scan
    progress.isRunning = true;
    progress.shouldStop = false;
    progress.scannedBytes = 0;
    progress.foundResults = 0;
    progress.startTime = std::chrono::steady_clock::now();
    
    // Distribute work among threads
    size_t regionsPerThread = memoryRegions.size() / threadCount;
    size_t remainingRegions = memoryRegions.size() % threadCount;
    
    scanThreads.clear();
    size_t regionIndex = 0;
    
    for (size_t i = 0; i < threadCount; ++i) {
        size_t regionCount = regionsPerThread;
        if (i < remainingRegions) {
            regionCount++;
        }
        
        std::vector<MemoryRegion> threadRegions;
        for (size_t j = 0; j < regionCount && regionIndex < memoryRegions.size(); ++j) {
            threadRegions.push_back(memoryRegions[regionIndex++]);
        }
        
        if (!threadRegions.empty()) {
            scanThreads.emplace_back(&BDOMemoryScanner::WorkerThread, this, i, threadRegions, type, value, scanType, alignment);
        }
    }
    
    // Wait for all threads to complete
    for (auto& thread : scanThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    progress.isRunning = false;
    progress.endTime = std::chrono::steady_clock::now();
    
    // Sort results by address
    SortResults();
    
    return true;
}

bool BDOMemoryScanner::FirstScanUnknown(ValueType type, ScanAlignment alignment, const ScanFilter& filter) {
    if (progress.isRunning) {
        return false;
    }
    
    // Clear previous results
    previousResults.clear();
    currentResults.clear();
    
    // Enumerate memory regions
    EnumerateMemoryRegions(filter);
    
    if (memoryRegions.empty()) {
        return false;
    }
    
    // Start scan
    progress.isRunning = true;
    progress.shouldStop = false;
    progress.scannedBytes = 0;
    progress.foundResults = 0;
    progress.startTime = std::chrono::steady_clock::now();
    
    // For unknown scan, we store all values
    for (const auto& region : memoryRegions) {
        if (progress.shouldStop) {
            break;
        }
        
        ScanRegionUnknown(region, type, alignment);
    }
    
    progress.isRunning = false;
    progress.endTime = std::chrono::steady_clock::now();
    
    // Sort results by address
    SortResults();
    
    return true;
}

bool BDOMemoryScanner::NextScan(ScanType scanType, const std::string& valueStr) {
    if (progress.isRunning || currentResults.empty()) {
        return false;
    }
    
    // Move current results to previous
    previousResults = std::move(currentResults);
    currentResults.clear();
    
    // Start scan
    progress.isRunning = true;
    progress.shouldStop = false;
    progress.scannedBytes = 0;
    progress.foundResults = 0;
    progress.startTime = std::chrono::steady_clock::now();
    
    // Scan only addresses from previous results
    for (const auto& prevResult : previousResults) {
        if (progress.shouldStop) {
            break;
        }
        
        // Read current value
        std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> currentValue;
        if (!GetValue(prevResult.address, prevResult.type, currentValue)) {
            continue;
        }
        
        bool shouldAdd = false;
        
        switch (scanType) {
            case ScanType::Changed:
                shouldAdd = (currentValue != prevResult.value);
                break;
            case ScanType::Unchanged:
                shouldAdd = (currentValue == prevResult.value);
                break;
            case ScanType::Increased:
                // Compare based on type
                std::visit([&](auto&& prevVal, auto&& currVal) {
                    using T = std::decay_t<decltype(prevVal)>;
                    using U = std::decay_t<decltype(currVal)>;
                    if constexpr (std::is_same_v<T, U> && std::is_arithmetic_v<T>) {
                        shouldAdd = currVal > prevVal;
                    }
                }, prevResult.value, currentValue);
                break;
            case ScanType::Decreased:
                std::visit([&](auto&& prevVal, auto&& currVal) {
                    using T = std::decay_t<decltype(prevVal)>;
                    using U = std::decay_t<decltype(currVal)>;
                    if constexpr (std::is_same_v<T, U> && std::is_arithmetic_v<T>) {
                        shouldAdd = currVal < prevVal;
                    }
                }, prevResult.value, currentValue);
                break;
            default:
                if (!valueStr.empty()) {
                    auto compareValue = StringToValue(valueStr, prevResult.type);
                    std::visit([&](auto&& currVal, auto&& compVal) {
                        using T = std::decay_t<decltype(currVal)>;
                        using U = std::decay_t<decltype(compVal)>;
                        if constexpr (std::is_same_v<T, U>) {
                            shouldAdd = CompareValue(currVal, compVal, scanType);
                        }
                    }, currentValue, compareValue);
                }
                break;
        }
        
        if (shouldAdd) {
            ScanResult result;
            result.address = prevResult.address;
            result.value = currentValue;
            result.type = prevResult.type;
            result.regionSize = prevResult.regionSize;
            result.protection = prevResult.protection;
            result.timestamp = std::chrono::steady_clock::now();
            
            std::lock_guard<std::mutex> lock(scannerMutex);
            currentResults.push_back(result);
            progress.foundResults++;
        }
        
        progress.scannedBytes += GetValueSize(prevResult.type);
    }
    
    progress.isRunning = false;
    progress.endTime = std::chrono::steady_clock::now();
    
    return true;
}

void BDOMemoryScanner::WorkerThread(size_t threadId, const std::vector<MemoryRegion>& assignedRegions, 
                                    ValueType type, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value, 
                                    ScanType scanType, ScanAlignment alignment) {
    for (const auto& region : assignedRegions) {
        if (progress.shouldStop) {
            break;
        }
        
        ScanRegion(region, type, value, scanType, alignment);
    }
}

void BDOMemoryScanner::ScanRegion(const MemoryRegion& region, ValueType type, 
                                  const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value, 
                                  ScanType scanType, ScanAlignment alignment) {
    // Allocate buffer for the entire region
    std::vector<BYTE> buffer(region.size);
    SIZE_T bytesRead;
    
    if (!ReadProcessMemory(hProcess, region.baseAddress, buffer.data(), region.size, &bytesRead)) {
        return;
    }
    
    SIZE_T valueSize = GetValueSize(type);
    SIZE_T step = static_cast<SIZE_T>(alignment);
    
    for (SIZE_T offset = 0; offset <= bytesRead - valueSize; offset += step) {
        if (progress.shouldStop) {
            break;
        }
        
        PVOID address = (PUCHAR)region.baseAddress + offset;
        bool match = false;
        
        // Check value based on type
        std::visit([&](auto&& searchValue) {
            using T = std::decay_t<decltype(searchValue)>;
            
            if constexpr (std::is_same_v<T, BYTE>) {
                if (type == ValueType::Byte) {
                    BYTE currentValue = buffer[offset];
                    match = CompareValue(currentValue, searchValue, scanType);
                    if (match) {
                        AddResult(address, currentValue, type, region);
                    }
                }
            } else if constexpr (std::is_same_v<T, WORD>) {
                if (type == ValueType::Word) {
                    WORD currentValue = *reinterpret_cast<WORD*>(&buffer[offset]);
                    match = CompareValue(currentValue, searchValue, scanType);
                    if (match) {
                        AddResult(address, currentValue, type, region);
                    }
                }
            } else if constexpr (std::is_same_v<T, DWORD>) {
                if (type == ValueType::Dword) {
                    DWORD currentValue = *reinterpret_cast<DWORD*>(&buffer[offset]);
                    match = CompareValue(currentValue, searchValue, scanType);
                    if (match) {
                        AddResult(address, currentValue, type, region);
                    }
                }
            } else if constexpr (std::is_same_v<T, QWORD>) {
                if (type == ValueType::Qword) {
                    QWORD currentValue = *reinterpret_cast<QWORD*>(&buffer[offset]);
                    match = CompareValue(currentValue, searchValue, scanType);
                    if (match) {
                        AddResult(address, currentValue, type, region);
                    }
                }
            } else if constexpr (std::is_same_v<T, float>) {
                if (type == ValueType::Float) {
                    float currentValue = *reinterpret_cast<float*>(&buffer[offset]);
                    match = CompareValue(currentValue, searchValue, scanType);
                    if (match) {
                        AddResult(address, currentValue, type, region);
                    }
                }
            } else if constexpr (std::is_same_v<T, double>) {
                if (type == ValueType::Double) {
                    double currentValue = *reinterpret_cast<double*>(&buffer[offset]);
                    match = CompareValue(currentValue, searchValue, scanType);
                    if (match) {
                        AddResult(address, currentValue, type, region);
                    }
                }
            } else if constexpr (std::is_same_v<T, std::string>) {
                if (type == ValueType::String) {
                    std::string currentValue(reinterpret_cast<char*>(&buffer[offset]), searchValue.length());
                    match = (currentValue == searchValue);
                    if (match) {
                        AddResult(address, currentValue, type, region);
                    }
                }
            } else if constexpr (std::is_same_v<T, std::vector<BYTE>>) {
                if (type == ValueType::ByteArray) {
                    match = CompareBytes(&buffer[offset], searchValue.data(), searchValue.size());
                    if (match) {
                        std::vector<BYTE> currentValue(buffer.begin() + offset, buffer.begin() + offset + searchValue.size());
                        AddResult(address, currentValue, type, region);
                    }
                }
            }
        }, value);
        
        progress.scannedBytes += step;
    }
}

void BDOMemoryScanner::ScanRegionUnknown(const MemoryRegion& region, ValueType type, ScanAlignment alignment) {
    // Allocate buffer for the entire region
    std::vector<BYTE> buffer(region.size);
    SIZE_T bytesRead;
    
    if (!ReadProcessMemory(hProcess, region.baseAddress, buffer.data(), region.size, &bytesRead)) {
        return;
    }
    
    SIZE_T valueSize = GetValueSize(type);
    SIZE_T step = static_cast<SIZE_T>(alignment);
    
    for (SIZE_T offset = 0; offset <= bytesRead - valueSize; offset += step) {
        if (progress.shouldStop) {
            break;
        }
        
        PVOID address = (PUCHAR)region.baseAddress + offset;
        
        // Store value based on type
        switch (type) {
            case ValueType::Byte: {
                BYTE value = buffer[offset];
                AddResult(address, value, type, region);
                break;
            }
            case ValueType::Word: {
                WORD value = *reinterpret_cast<WORD*>(&buffer[offset]);
                AddResult(address, value, type, region);
                break;
            }
            case ValueType::Dword: {
                DWORD value = *reinterpret_cast<DWORD*>(&buffer[offset]);
                AddResult(address, value, type, region);
                break;
            }
            case ValueType::Qword: {
                QWORD value = *reinterpret_cast<QWORD*>(&buffer[offset]);
                AddResult(address, value, type, region);
                break;
            }
            case ValueType::Float: {
                float value = *reinterpret_cast<float*>(&buffer[offset]);
                AddResult(address, value, type, region);
                break;
            }
            case ValueType::Double: {
                double value = *reinterpret_cast<double*>(&buffer[offset]);
                AddResult(address, value, type, region);
                break;
            }
            default:
                break;
        }
        
        progress.scannedBytes += step;
    }
}

void BDOMemoryScanner::AddResult(PVOID address, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value, 
                                 ValueType type, const MemoryRegion& region) {
    ScanResult result;
    result.address = address;
    result.value = value;
    result.type = type;
    result.regionSize = region.size;
    result.protection = region.protection;
    result.timestamp = std::chrono::steady_clock::now();
    
    std::lock_guard<std::mutex> lock(scannerMutex);
    currentResults.push_back(result);
    progress.foundResults++;
}

void BDOMemoryScanner::SortResults() {
    std::lock_guard<std::mutex> lock(scannerMutex);
    std::sort(currentResults.begin(), currentResults.end(), 
              [](const ScanResult& a, const ScanResult& b) {
                  return a.address < b.address;
              });
}

bool BDOMemoryScanner::CompareBytes(const BYTE* current, const BYTE* compare, SIZE_T size) {
    return memcmp(current, compare, size) == 0;
}

SIZE_T BDOMemoryScanner::GetValueSize(ValueType type) {
    switch (type) {
        case ValueType::Byte:
            return sizeof(BYTE);
        case ValueType::Word:
            return sizeof(WORD);
        case ValueType::Dword:
            return sizeof(DWORD);
        case ValueType::Qword:
            return sizeof(QWORD);
        case ValueType::Float:
            return sizeof(float);
        case ValueType::Double:
            return sizeof(double);
        default:
            return 0;
    }
}

ValueType BDOMemoryScanner::DetectValueType(const std::string& valueStr) {
    // Try to detect type from string
    if (valueStr.find('.') != std::string::npos) {
        // Contains decimal point - likely float or double
        try {
            double d = std::stod(valueStr);
            if (d >= -FLT_MAX && d <= FLT_MAX) {
                return ValueType::Float;
            }
            return ValueType::Double;
        } catch (...) {
            return ValueType::Unknown;
        }
    }
    
    // Try to parse as integer
    try {
        long long value = std::stoll(valueStr);
        if (value >= 0 && value <= UCHAR_MAX) {
            return ValueType::Byte;
        } else if (value >= SHRT_MIN && value <= USHRT_MAX) {
            return ValueType::Word;
        } else if (value >= INT_MIN && value <= UINT_MAX) {
            return ValueType::Dword;
        } else {
            return ValueType::Qword;
        }
    } catch (...) {
        // Not a number - assume string
        return ValueType::String;
    }
}

std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> BDOMemoryScanner::StringToValue(const std::string& str, ValueType type) {
    switch (type) {
        case ValueType::Byte:
            return static_cast<BYTE>(std::stoul(str));
        case ValueType::Word:
            return static_cast<WORD>(std::stoul(str));
        case ValueType::Dword:
            return static_cast<DWORD>(std::stoul(str));
        case ValueType::Qword:
            return static_cast<QWORD>(std::stoull(str));
        case ValueType::Float:
            return std::stof(str);
        case ValueType::Double:
            return std::stod(str);
        case ValueType::String:
            return str;
        case ValueType::ByteArray: {
            std::vector<BYTE> bytes;
            std::istringstream iss(str);
            std::string byteStr;
            while (iss >> std::setw(2) >> byteStr) {
                bytes.push_back(static_cast<BYTE>(std::stoul(byteStr, nullptr, 16)));
            }
            return bytes;
        }
        default:
            return BYTE(0);
    }
}

std::string BDOMemoryScanner::ValueToString(const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value) {
    return std::visit([](auto&& val) -> std::string {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return val;
        } else if constexpr (std::is_same_v<T, std::vector<BYTE>>) {
            std::stringstream ss;
            for (BYTE b : val) {
                ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b) << " ";
            }
            return ss.str();
        } else {
            return std::to_string(val);
        }
    }, value);
}

std::vector<ScanResult> BDOMemoryScanner::GetResults() {
    std::lock_guard<std::mutex> lock(scannerMutex);
    return currentResults;
}

size_t BDOMemoryScanner::GetResultCount() {
    std::lock_guard<std::mutex> lock(scannerMutex);
    return currentResults.size();
}

bool BDOMemoryScanner::GetValue(PVOID address, ValueType type, std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value) {
    switch (type) {
        case ValueType::Byte: {
            BYTE val;
            if (ReadValue(address, val)) {
                value = val;
                return true;
            }
            break;
        }
        case ValueType::Word: {
            WORD val;
            if (ReadValue(address, val)) {
                value = val;
                return true;
            }
            break;
        }
        case ValueType::Dword: {
            DWORD val;
            if (ReadValue(address, val)) {
                value = val;
                return true;
            }
            break;
        }
        case ValueType::Qword: {
            QWORD val;
            if (ReadValue(address, val)) {
                value = val;
                return true;
            }
            break;
        }
        case ValueType::Float: {
            float val;
            if (ReadValue(address, val)) {
                value = val;
                return true;
            }
            break;
        }
        case ValueType::Double: {
            double val;
            if (ReadValue(address, val)) {
                value = val;
                return true;
            }
            break;
        }
        case ValueType::String: {
            std::string val;
            if (ReadString(address, val, 256)) {
                value = val;
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

bool BDOMemoryScanner::SetValue(PVOID address, ValueType type, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value) {
    SIZE_T bytesWritten;
    
    return std::visit([&](auto&& val) -> bool {
        using T = std::decay_t<decltype(val)>;
        
        if constexpr (std::is_same_v<T, std::string>) {
            return WriteProcessMemory(hProcess, address, val.c_str(), val.length() + 1, &bytesWritten) != 0;
        } else if constexpr (std::is_same_v<T, std::vector<BYTE>>) {
            return WriteProcessMemory(hProcess, address, val.data(), val.size(), &bytesWritten) != 0;
        } else {
            return WriteProcessMemory(hProcess, address, &val, sizeof(T), &bytesWritten) != 0;
        }
    }, value);
}

bool BDOMemoryScanner::FreezeValue(PVOID address, ValueType type, const std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>& value) {
    std::lock_guard<std::mutex> lock(frozenMutex);
    
    // Check if already frozen
    if (frozenValues.find(address) != frozenValues.end()) {
        return false;
    }
    
    auto frozen = std::make_unique<FrozenValue>();
    frozen->address = address;
    frozen->type = type;
    frozen->value = value;
    frozen->shouldFreeze = true;
    
    // Start freeze thread
    frozen->freezeThread = std::thread(&BDOMemoryScanner::FreezeThread, this, frozen.get());
    
    frozenValues[address] = std::move(frozen);
    return true;
}

bool BDOMemoryScanner::UnfreezeValue(PVOID address) {
    std::lock_guard<std::mutex> lock(frozenMutex);
    
    auto it = frozenValues.find(address);
    if (it == frozenValues.end()) {
        return false;
    }
    
    it->second->shouldFreeze = false;
    if (it->second->freezeThread.joinable()) {
        it->second->freezeThread.join();
    }
    
    frozenValues.erase(it);
    return true;
}

bool BDOMemoryScanner::IsValueFrozen(PVOID address) {
    std::lock_guard<std::mutex> lock(frozenMutex);
    return frozenValues.find(address) != frozenValues.end();
}

void BDOMemoryScanner::FreezeThread(FrozenValue* frozen) {
    while (frozen->shouldFreeze) {
        SetValue(frozen->address, frozen->type, frozen->value);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool BDOMemoryScanner::ReadString(PVOID address, std::string& str, SIZE_T maxLength) {
    std::vector<char> buffer(maxLength);
    SIZE_T bytesRead;
    
    if (!ReadProcessMemory(hProcess, address, buffer.data(), maxLength, &bytesRead)) {
        return false;
    }
    
    // Find null terminator
    size_t len = 0;
    for (size_t i = 0; i < bytesRead; ++i) {
        if (buffer[i] == '\0') {
            len = i;
            break;
        }
    }
    
    str.assign(buffer.data(), len);
    return true;
}

const ScanProgress& BDOMemoryScanner::GetProgress() const {
    return progress;
}

void BDOMemoryScanner::StopScan() {
    progress.shouldStop = true;
}

bool BDOMemoryScanner::IsScanRunning() {
    return progress.isRunning;
}

double BDOMemoryScanner::GetScanSpeed() {
    if (!progress.isRunning || progress.scannedBytes == 0) {
        return 0.0;
    }
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - progress.startTime).count();
    
    if (duration == 0) {
        return 0.0;
    }
    
    return static_cast<double>(progress.scannedBytes) / (duration / 1000.0);
}

bool BDOMemoryScanner::CreateSnapshot(const std::string& name) {
    std::lock_guard<std::mutex> lock(scannerMutex);
    savedScans[name] = currentResults;
    return true;
}

bool BDOMemoryScanner::CompareWithSnapshot(const std::string& name, ScanType compareType) {
    std::lock_guard<std::mutex> lock(scannerMutex);
    
    auto it = savedScans.find(name);
    if (it == savedScans.end()) {
        return false;
    }
    
    previousResults = it->second;
    return NextScan(compareType);
}

bool BDOMemoryScanner::DeleteSnapshot(const std::string& name) {
    std::lock_guard<std::mutex> lock(scannerMutex);
    return savedScans.erase(name) > 0;
}

std::vector<std::string> BDOMemoryScanner::GetSnapshotNames() {
    std::lock_guard<std::mutex> lock(scannerMutex);
    std::vector<std::string> names;
    
    for (const auto& [name, results] : savedScans) {
        names.push_back(name);
    }
    
    return names;
}

std::string BDOMemoryScanner::GetValueTypeString(ValueType type) {
    switch (type) {
        case ValueType::Byte: return "Byte";
        case ValueType::Word: return "Word";
        case ValueType::Dword: return "Dword";
        case ValueType::Qword: return "Qword";
        case ValueType::Float: return "Float";
        case ValueType::Double: return "Double";
        case ValueType::String: return "String";
        case ValueType::ByteArray: return "ByteArray";
        default: return "Unknown";
    }
}

std::string BDOMemoryScanner::GetScanTypeString(ScanType type) {
    switch (type) {
        case ScanType::ExactValue: return "Exact Value";
        case ScanType::BiggerThan: return "Bigger Than";
        case ScanType::SmallerThan: return "Smaller Than";
        case ScanType::Between: return "Between";
        case ScanType::Unknown: return "Unknown";
        case ScanType::Changed: return "Changed";
        case ScanType::Unchanged: return "Unchanged";
        case ScanType::Increased: return "Increased";
        case ScanType::Decreased: return "Decreased";
        case ScanType::IncreasedBy: return "Increased By";
        case ScanType::DecreasedBy: return "Decreased By";
        default: return "Unknown";
    }
}

// MemoryValueMonitor implementation
MemoryValueMonitor::MemoryValueMonitor(BDOMemoryScanner* scanner) : scanner(scanner) {}

MemoryValueMonitor::~MemoryValueMonitor() {
    StopMonitoring();
}

bool MemoryValueMonitor::AddAddress(PVOID address, ValueType type) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    monitoredAddresses.push_back({address, type});
    return true;
}

bool MemoryValueMonitor::RemoveAddress(PVOID address) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    auto it = std::remove_if(monitoredAddresses.begin(), monitoredAddresses.end(),
                            [address](const auto& pair) { return pair.first == address; });
    if (it != monitoredAddresses.end()) {
        monitoredAddresses.erase(it, monitoredAddresses.end());
        valueHistory.erase(address);
        return true;
    }
    return false;
}

void MemoryValueMonitor::ClearAddresses() {
    std::lock_guard<std::mutex> lock(monitorMutex);
    monitoredAddresses.clear();
    valueHistory.clear();
}

bool MemoryValueMonitor::StartMonitoring(std::chrono::milliseconds interval) {
    if (isMonitoring) {
        return false;
    }
    
    updateInterval = interval;
    isMonitoring = true;
    monitorThread = std::thread(&MemoryValueMonitor::MonitorLoop, this);
    return true;
}

void MemoryValueMonitor::StopMonitoring() {
    if (isMonitoring) {
        isMonitoring = false;
        if (monitorThread.joinable()) {
            monitorThread.join();
        }
    }
}

bool MemoryValueMonitor::IsMonitoring() {
    return isMonitoring;
}

void MemoryValueMonitor::MonitorLoop() {
    while (isMonitoring) {
        {
            std::lock_guard<std::mutex> lock(monitorMutex);
            for (const auto& [address, type] : monitoredAddresses) {
                std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> value;
                if (scanner->GetValue(address, type, value)) {
                    valueHistory[address].push_back(value);
                    
                    // Keep only last 1000 values
                    if (valueHistory[address].size() > 1000) {
                        valueHistory[address].erase(valueHistory[address].begin());
                    }
                }
            }
        }
        
        std::this_thread::sleep_for(updateInterval);
    }
}

std::vector<std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>> MemoryValueMonitor::GetValueHistory(PVOID address) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    auto it = valueHistory.find(address);
    if (it != valueHistory.end()) {
        return it->second;
    }
    return {};
}

std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> MemoryValueMonitor::GetCurrentValue(PVOID address) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    auto it = valueHistory.find(address);
    if (it != valueHistory.end() && !it->second.empty()) {
        return it->second.back();
    }
    return BYTE(0);
}

bool MemoryValueMonitor::HasValueChanged(PVOID address) {
    std::lock_guard<std::mutex> lock(monitorMutex);
    auto it = valueHistory.find(address);
    if (it != valueHistory.end() && it->second.size() >= 2) {
        return it->second[it->second.size() - 1] != it->second[it->second.size() - 2];
    }
    return false;
}

// Stub implementations for missing advanced functions
bool BDOMemoryScanner::AnalyzeStructure(PVOID address, SIZE_T maxSize, StructureInfo& info) {
    // Stub implementation - would analyze memory structure
    return false;
}

bool BDOMemoryScanner::FindPointerPath(PVOID sourceAddress, PVOID targetAddress, std::vector<SIZE_T>& offsets, SIZE_T maxDepth) {
    // Stub implementation - would find pointer chains
    return false;
}

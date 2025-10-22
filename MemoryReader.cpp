#include "MemoryReader.h"
#include <psapi.h>
#include <sstream>

#pragma comment(lib, "psapi.lib")

MemoryReader::MemoryReader() : processHandle(nullptr), processId(0), baseAddress(0) {
}

MemoryReader::~MemoryReader() {
    Detach();
}

bool MemoryReader::AttachToProcess(const std::wstring& processName) {
    processId = FindProcessId(processName);
    if (processId == 0) {
        std::wcout << L"[-] Process not found: " << processName << std::endl;
        return false;
    }
    
    return AttachToProcess(processId);
}

bool MemoryReader::AttachToProcess(DWORD pid) {
    processId = pid;
    
    // Try with full access first
    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    
    // If that fails, try with limited access
    if (!processHandle) {
        processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, processId);
    }
    
    if (!processHandle) {
        std::cout << "[-] Failed to open process. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    // Get base address
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(processHandle, hMods, sizeof(hMods), &cbNeeded)) {
        baseAddress = (uintptr_t)hMods[0];
    }
    
    std::cout << "[+] Attached to process (PID: " << processId << ")" << std::endl;
    std::cout << "[+] Base address: 0x" << std::hex << baseAddress << std::dec << std::endl;
    
    return true;
}

void MemoryReader::Detach() {
    if (processHandle) {
        CloseHandle(processHandle);
        processHandle = nullptr;
        processId = 0;
        baseAddress = 0;
    }
}

bool MemoryReader::ReadBytes(uintptr_t address, void* buffer, SIZE_T size) {
    SIZE_T bytesRead;
    if (ReadProcessMemory(processHandle, (LPCVOID)address, buffer, size, &bytesRead)) {
        return bytesRead == size;
    }
    return false;
}

bool MemoryReader::WriteBytes(uintptr_t address, const void* buffer, SIZE_T size) {
    SIZE_T bytesWritten;
    if (WriteProcessMemory(processHandle, (LPVOID)address, buffer, size, &bytesWritten)) {
        return bytesWritten == size;
    }
    return false;
}

uintptr_t MemoryReader::ReadPointerChain(uintptr_t baseAddr, const std::vector<uintptr_t>& offsets) {
    uintptr_t address = baseAddr;
    
    for (size_t i = 0; i < offsets.size(); i++) {
        address = Read<uintptr_t>(address);
        if (address == 0) {
            return 0;
        }
        address += offsets[i];
    }
    
    return address;
}

uintptr_t MemoryReader::FindPattern(const char* pattern, const char* mask, uintptr_t startAddress, SIZE_T searchSize) {
    if (!processHandle) return 0;
    
    // If no start address specified, use base address
    if (startAddress == 0) {
        startAddress = baseAddress;
    }
    
    // If no size specified, scan up to 50MB
    if (searchSize == 0) {
        searchSize = 0x3200000; // 50MB
    }
    
    SIZE_T patternLength = strlen(mask);
    std::vector<char> buffer(searchSize);
    SIZE_T bytesRead;
    
    if (!ReadProcessMemory(processHandle, (LPCVOID)startAddress, buffer.data(), searchSize, &bytesRead)) {
        return 0;
    }
    
    for (SIZE_T i = 0; i < bytesRead - patternLength; i++) {
        if (ComparePattern(&buffer[i], pattern, mask)) {
            return startAddress + i;
        }
    }
    
    return 0;
}

std::vector<uintptr_t> MemoryReader::FindPatternAll(const char* pattern, const char* mask) {
    std::vector<uintptr_t> results;
    
    if (!processHandle) return results;
    
    MEMORY_BASIC_INFORMATION mbi;
    uintptr_t address = 0;
    SIZE_T patternLength = strlen(mask);
    
    while (VirtualQueryEx(processHandle, (LPCVOID)address, &mbi, sizeof(mbi))) {
        if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_GUARD) == 0 && mbi.Protect != PAGE_NOACCESS) {
            std::vector<char> buffer(mbi.RegionSize);
            SIZE_T bytesRead;
            
            if (ReadProcessMemory(processHandle, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
                for (SIZE_T i = 0; i < bytesRead - patternLength; i++) {
                    if (ComparePattern(&buffer[i], pattern, mask)) {
                        results.push_back((uintptr_t)mbi.BaseAddress + i);
                    }
                }
            }
        }
        
        address = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
    }
    
    return results;
}

uintptr_t MemoryReader::ScanAOB(const std::string& pattern) {
    // Convert hex string pattern to bytes
    // Format: "48 8B 0D ?? ?? ?? ??"
    std::vector<int> patternBytes;
    std::string mask;
    
    std::istringstream iss(pattern);
    std::string byteStr;
    
    while (iss >> byteStr) {
        if (byteStr == "??" || byteStr == "?") {
            patternBytes.push_back(0);
            mask += '?';
        } else {
            patternBytes.push_back(std::stoi(byteStr, nullptr, 16));
            mask += 'x';
        }
    }
    
    // Convert to char array for FindPattern
    std::vector<char> patternChars(patternBytes.size());
    for (size_t i = 0; i < patternBytes.size(); i++) {
        patternChars[i] = static_cast<char>(patternBytes[i]);
    }
    
    return FindPattern(patternChars.data(), mask.c_str());
}

uintptr_t MemoryReader::GetModuleBase(const std::wstring& moduleName) {
    if (!processHandle) return 0;
    
    HMODULE hMods[1024];
    DWORD cbNeeded;
    
    if (EnumProcessModules(processHandle, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            wchar_t szModName[MAX_PATH];
            if (GetModuleFileNameExW(processHandle, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t))) {
                std::wstring modName(szModName);
                if (modName.find(moduleName) != std::wstring::npos) {
                    return (uintptr_t)hMods[i];
                }
            }
        }
    }
    
    return 0;
}

SIZE_T MemoryReader::GetModuleSize(const std::wstring& moduleName) {
    uintptr_t moduleBase = GetModuleBase(moduleName);
    if (moduleBase == 0) return 0;
    
    MODULEINFO modInfo;
    HMODULE hModule = (HMODULE)moduleBase;
    
    if (GetModuleInformation(processHandle, hModule, &modInfo, sizeof(modInfo))) {
        return modInfo.SizeOfImage;
    }
    
    return 0;
}

bool MemoryReader::EnableDebugPrivileges() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;
    
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }
    
    if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid)) {
        CloseHandle(hToken);
        return false;
    }
    
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
        CloseHandle(hToken);
        return false;
    }
    
    CloseHandle(hToken);
    return true;
}

DWORD MemoryReader::FindProcessId(const std::wstring& processName) {
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return 0;
}

bool MemoryReader::ComparePattern(const char* data, const char* pattern, const char* mask) {
    for (; *mask; ++mask, ++data, ++pattern) {
        if (*mask == 'x' && *data != *pattern) {
            return false;
        }
    }
    return true;
}


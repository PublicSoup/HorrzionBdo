/*
 * BDO Kernel Client Implementation
 * Professional user-mode interface implementation
 */

#include "pch.h"
#include "BDO_KernelClient.h"
#include <TlHelp32.h>
#include <iostream>
#include <sstream>
#include <iomanip>

// Constructor
BDOKernelInterface::BDOKernelInterface()
    : hDriver(INVALID_HANDLE_VALUE)
    , targetPid(0)
    , gameBase(0)
    , connected(false)
    , antiCheatBypassed(false)
    , totalReads(0)
    , totalWrites(0)
    , failedOperations(0)
    , lastError(0)
{
}

// Destructor
BDOKernelInterface::~BDOKernelInterface() {
    Disconnect();
}

// Connect to kernel driver
bool BDOKernelInterface::Connect(const wchar_t* driverPath) {
    if (connected) {
        return true;
    }
    
    hDriver = CreateFileW(
        driverPath,
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        SetLastError(::GetLastError());
        std::wcout << L"[!] Failed to connect to driver: " << driverPath << std::endl;
        std::wcout << L"[!] Error: " << GetLastError() << std::endl;
        std::wcout << L"[!] Make sure driver is loaded:" << std::endl;
        std::wcout << L"    sc create BDOKernelMem type=kernel binPath=<path>\\BDO_KernelDriver.sys" << std::endl;
        std::wcout << L"    sc start BDOKernelMem" << std::endl;
        return false;
    }
    
    connected = true;
    std::wcout << L"[+] Connected to kernel driver: " << driverPath << std::endl;
    return true;
}

// Disconnect from driver
void BDOKernelInterface::Disconnect() {
    if (hDriver != INVALID_HANDLE_VALUE) {
        CloseHandle(hDriver);
        hDriver = INVALID_HANDLE_VALUE;
    }
    connected = false;
    targetPid = 0;
    gameBase = 0;
}

// Attach to process by PID
bool BDOKernelInterface::AttachToProcess(DWORD processId) {
    if (!connected) {
        std::cout << "[!] Not connected to driver" << std::endl;
        return false;
    }
    
    targetPid = processId;
    
    // Get process info
    PROCESS_INFO info = {0};
    info.ProcessId = targetPid;
    
    if (GetProcessInfo(info)) {
        gameBase = info.BaseAddress;
        std::cout << "[+] Attached to PID: " << targetPid << std::endl;
        std::cout << "[+] Process: " << info.ProcessName << std::endl;
        std::cout << "[+] Base: 0x" << std::hex << gameBase << std::dec << std::endl;
        std::cout << "[+] PEB: 0x" << std::hex << info.PEB << std::dec << std::endl;
        std::cout << "[+] Protected: " << (info.IsProtected ? "Yes" : "No") << std::endl;
        return true;
    }
    
    return false;
}

// Attach to process by name
bool BDOKernelInterface::AttachToProcess(const wchar_t* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    DWORD foundPid = 0;
    if (Process32FirstW(snapshot, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, processName) == 0) {
                foundPid = pe32.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &pe32));
    }
    
    CloseHandle(snapshot);
    
    if (foundPid == 0) {
        std::wcout << L"[!] Process not found: " << processName << std::endl;
        return false;
    }
    
    std::wcout << L"[+] Found process: " << processName << L" (PID: " << foundPid << L")" << std::endl;
    return AttachToProcess(foundPid);
}

// Detach from process
bool BDOKernelInterface::DetachFromProcess() {
    targetPid = 0;
    gameBase = 0;
    antiCheatBypassed = false;
    return true;
}

// Read buffer from memory
bool BDOKernelInterface::ReadBuffer(uint64_t address, void* buffer, size_t size) {
    if (!connected || targetPid == 0) {
        return false;
    }
    
    MEMORY_OPERATION memOp = {0};
    memOp.ProcessId = targetPid;
    memOp.Address = address;
    memOp.Buffer = buffer;
    memOp.Size = size;
    memOp.Write = FALSE;
    
    bool success = SendIOCTL(IOCTL_READ_MEMORY, &memOp, sizeof(memOp), &memOp, sizeof(memOp));
    
    if (success) {
        totalReads++;
    } else {
        failedOperations++;
    }
    
    return success;
}

// Write buffer to memory
bool BDOKernelInterface::WriteBuffer(uint64_t address, const void* buffer, size_t size) {
    if (!connected || targetPid == 0) {
        return false;
    }
    
    MEMORY_OPERATION memOp = {0};
    memOp.ProcessId = targetPid;
    memOp.Address = address;
    memOp.Buffer = const_cast<void*>(buffer);
    memOp.Size = size;
    memOp.Write = TRUE;
    
    bool success = SendIOCTL(IOCTL_WRITE_MEMORY, &memOp, sizeof(memOp), nullptr, 0);
    
    if (success) {
        totalWrites++;
    } else {
        failedOperations++;
    }
    
    return success;
}

// Read string
bool BDOKernelInterface::ReadString(uint64_t address, std::string& str, size_t maxLength) {
    std::vector<char> buffer(maxLength + 1, 0);
    if (ReadBuffer(address, buffer.data(), maxLength)) {
        str = std::string(buffer.data());
        return true;
    }
    return false;
}

// Read wide string
bool BDOKernelInterface::ReadWString(uint64_t address, std::wstring& str, size_t maxLength) {
    std::vector<wchar_t> buffer(maxLength + 1, 0);
    if (ReadBuffer(address, buffer.data(), maxLength * sizeof(wchar_t))) {
        str = std::wstring(buffer.data());
        return true;
    }
    return false;
}

// Read pointer
uint64_t BDOKernelInterface::ReadPointer(uint64_t address) {
    uint64_t value = 0;
    Read(address, value);
    return value;
}

// Read multi-level pointer
uint64_t BDOKernelInterface::ReadMultiLevelPointer(uint64_t base, const std::vector<uint64_t>& offsets) {
    uint64_t address = base;
    
    for (size_t i = 0; i < offsets.size(); i++) {
        address = ReadPointer(address);
        if (address == 0) {
            return 0;
        }
        address += offsets[i];
    }
    
    return address;
}

// Get module base address
bool BDOKernelInterface::GetModuleBase(const wchar_t* moduleName, uint64_t& baseAddress, size_t& moduleSize) {
    if (!connected || targetPid == 0) {
        return false;
    }
    
    MODULE_REQUEST modReq = {0};
    modReq.ProcessId = targetPid;
    wcsncpy_s(modReq.ModuleName, 260, moduleName, _TRUNCATE);
    
    if (SendIOCTL(IOCTL_GET_MODULE_BASE, &modReq, sizeof(modReq), &modReq, sizeof(modReq))) {
        baseAddress = modReq.BaseAddress;
        moduleSize = modReq.ModuleSize;
        return true;
    }
    
    return false;
}

// Get game base address
uint64_t BDOKernelInterface::GetGameBase() {
    if (gameBase == 0) {
        RefreshGameBase();
    }
    return gameBase;
}

// Refresh game base address
bool BDOKernelInterface::RefreshGameBase() {
    size_t moduleSize = 0;
    const wchar_t* moduleNames[] = {
        L"BlackDesert64.exe",
        L"BlackDesert32.exe",
        L"BlackDesertEAC.exe"
    };
    
    for (const wchar_t* name : moduleNames) {
        if (GetModuleBase(name, gameBase, moduleSize)) {
            std::wcout << L"[+] Found game module: " << name << std::endl;
            std::wcout << L"[+] Base: 0x" << std::hex << gameBase << std::dec << std::endl;
            std::wcout << L"[+] Size: 0x" << std::hex << moduleSize << std::dec << std::endl;
            return true;
        }
    }
    
    return false;
}

// Pattern scan
uint64_t BDOKernelInterface::PatternScan(const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask,
                                         uint64_t start, uint64_t end) {
    if (!connected || targetPid == 0) {
        return 0;
    }
    
    if (start == 0) {
        start = gameBase;
    }
    if (end == 0) {
        end = gameBase + 0x10000000; // 256MB default scan range
    }
    
    PATTERN_SCAN_REQUEST scanReq = {0};
    scanReq.ProcessId = targetPid;
    scanReq.StartAddress = start;
    scanReq.EndAddress = end;
    scanReq.PatternLength = pattern.size();
    
    if (pattern.size() > 256) {
        std::cout << "[!] Pattern too large (max 256 bytes)" << std::endl;
        return 0;
    }
    
    memcpy(scanReq.Pattern, pattern.data(), pattern.size());
    memcpy(scanReq.Mask, mask.data(), mask.size());
    
    if (SendIOCTL(IOCTL_PATTERN_SCAN, &scanReq, sizeof(scanReq), &scanReq, sizeof(scanReq))) {
        return scanReq.ResultAddress;
    }
    
    return 0;
}

// AOB scan (hex string with wildcards)
uint64_t BDOKernelInterface::AOBScan(const char* pattern) {
    std::vector<BYTE> bytes;
    std::vector<BYTE> mask;
    
    std::istringstream iss(pattern);
    std::string token;
    
    while (iss >> token) {
        if (token == "??" || token == "?") {
            bytes.push_back(0x00);
            mask.push_back(0x00);
        } else {
            bytes.push_back((BYTE)std::stoul(token, nullptr, 16));
            mask.push_back(0xFF);
        }
    }
    
    return PatternScan(bytes, mask);
}

// Pattern scan all occurrences
std::vector<uint64_t> BDOKernelInterface::PatternScanAll(const std::vector<BYTE>& pattern,
                                                          const std::vector<BYTE>& mask,
                                                          uint64_t start, uint64_t end) {
    std::vector<uint64_t> results;
    
    uint64_t currentStart = (start == 0) ? gameBase : start;
    uint64_t scanEnd = (end == 0) ? gameBase + 0x10000000 : end;
    
    while (currentStart < scanEnd) {
        uint64_t result = PatternScan(pattern, mask, currentStart, scanEnd);
        if (result == 0) {
            break;
        }
        
        results.push_back(result);
        currentStart = result + pattern.size();
    }
    
    return results;
}

// Get process information
bool BDOKernelInterface::GetProcessInfo(PROCESS_INFO& info) {
    if (!connected) {
        return false;
    }
    
    return SendIOCTL(IOCTL_GET_PROCESS_INFO, &info, sizeof(info), &info, sizeof(info));
}

// Get process name
std::string BDOKernelInterface::GetProcessName() {
    PROCESS_INFO info = {0};
    info.ProcessId = targetPid;
    
    if (GetProcessInfo(info)) {
        return std::string(info.ProcessName);
    }
    
    return "";
}

// Check if process is protected
bool BDOKernelInterface::IsProcessProtected() {
    PROCESS_INFO info = {0};
    info.ProcessId = targetPid;
    
    if (GetProcessInfo(info)) {
        return info.IsProtected;
    }
    
    return false;
}

// Bypass anti-cheat
bool BDOKernelInterface::BypassAntiCheat() {
    if (!connected || targetPid == 0) {
        return false;
    }
    
    std::cout << "[*] Attempting anti-cheat bypass..." << std::endl;
    
    if (SendIOCTL(IOCTL_BYPASS_AC, &targetPid, sizeof(targetPid), nullptr, 0)) {
        antiCheatBypassed = true;
        std::cout << "[+] Anti-cheat bypass successful!" << std::endl;
        return true;
    }
    
    std::cout << "[!] Anti-cheat bypass failed" << std::endl;
    return false;
}

// Check if address is valid
bool BDOKernelInterface::IsAddressValid(uint64_t address) {
    BYTE test;
    return ReadBuffer(address, &test, 1);
}

// Get PEB address
uint64_t BDOKernelInterface::GetPEB() {
    PROCESS_INFO info = {0};
    info.ProcessId = targetPid;
    
    if (GetProcessInfo(info)) {
        return info.PEB;
    }
    
    return 0;
}

// Get statistics
BDOKernelInterface::Statistics BDOKernelInterface::GetStatistics() const {
    Statistics stats;
    stats.totalReads = totalReads;
    stats.totalWrites = totalWrites;
    stats.failedOperations = failedOperations;
    
    uint64_t totalOps = totalReads + totalWrites;
    if (totalOps > 0) {
        stats.successRate = (double)(totalOps - failedOperations) / totalOps * 100.0;
    } else {
        stats.successRate = 100.0;
    }
    
    return stats;
}

// Reset statistics
void BDOKernelInterface::ResetStatistics() {
    totalReads = 0;
    totalWrites = 0;
    failedOperations = 0;
}

// Get last error string
std::string BDOKernelInterface::GetLastErrorString() const {
    if (lastError == 0) {
        return "No error";
    }
    
    char buffer[256];
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        lastError,
        0,
        buffer,
        sizeof(buffer),
        nullptr
    );
    
    return std::string(buffer);
}

// Set last error
void BDOKernelInterface::SetLastError(DWORD error) {
    lastError = error;
}

// Send IOCTL to driver
bool BDOKernelInterface::SendIOCTL(DWORD ioctl, LPVOID inBuffer, DWORD inSize, 
                                   LPVOID outBuffer, DWORD outSize) {
    if (hDriver == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        ioctl,
        inBuffer,
        inSize,
        outBuffer,
        outSize,
        &bytesReturned,
        nullptr
    );
    
    if (!result) {
        SetLastError(::GetLastError());
    }
    
    return result != FALSE;
}

// ============================================================================
// BDOGameMemory Implementation
// ============================================================================

BDOGameMemory::BDOGameMemory(BDOKernelInterface* kernelInterface)
    : kernel(kernelInterface)
    , ownsKernel(false)
{
    memset(&cache, 0, sizeof(cache));
    
    if (kernel == nullptr) {
        kernel = new BDOKernelInterface();
        ownsKernel = true;
    }
}

BDOGameMemory::~BDOGameMemory() {
    if (ownsKernel && kernel) {
        delete kernel;
    }
}

bool BDOGameMemory::Initialize() {
    if (!kernel->IsConnected()) {
        if (!kernel->Connect()) {
            return false;
        }
    }
    
    if (kernel->GetTargetPID() == 0) {
        if (!kernel->AttachToProcess(L"BlackDesert64.exe")) {
            return false;
        }
    }
    
    // Attempt anti-cheat bypass
    kernel->BypassAntiCheat();
    
    return true;
}

bool BDOGameMemory::IsCacheValid(DWORD timestamp) const {
    return (GetTickCount() - timestamp) < CACHE_TIMEOUT_MS;
}

void BDOGameMemory::InvalidateCache() {
    memset(&cache, 0, sizeof(cache));
}

// Get player base address
uint64_t BDOGameMemory::GetPlayerBase() {
    if (IsCacheValid(cache.playerBaseTime) && cache.playerBase != 0) {
        return cache.playerBase;
    }
    
    // Method 1: Pattern scan
    uint64_t result = kernel->AOBScan(Patterns::PlayerBase);
    
    if (result != 0) {
        // Resolve RIP-relative address
        int32_t offset = 0;
        kernel->Read(result + 3, offset);
        cache.playerBase = result + 7 + offset;
        cache.playerBaseTime = GetTickCount();
        
        std::cout << "[+] Player base resolved: 0x" << std::hex << cache.playerBase << std::dec << std::endl;
        return cache.playerBase;
    }
    
    // Method 2: Static offset (fallback)
    cache.playerBase = kernel->GetGameBase() + Offsets::PlayerController;
    cache.playerBaseTime = GetTickCount();
    
    return cache.playerBase;
}

// Get entity list base
uint64_t BDOGameMemory::GetEntityListBase() {
    if (IsCacheValid(cache.entityListTime) && cache.entityList != 0) {
        return cache.entityList;
    }
    
    uint64_t result = kernel->AOBScan(Patterns::EntityList);
    
    if (result != 0) {
        int32_t offset = 0;
        kernel->Read(result + 3, offset);
        cache.entityList = result + 7 + offset;
        cache.entityListTime = GetTickCount();
        return cache.entityList;
    }
    
    cache.entityList = kernel->GetGameBase() + Offsets::EntityListBase;
    cache.entityListTime = GetTickCount();
    
    return cache.entityList;
}

// Get player data
bool BDOGameMemory::GetPlayerData(PlayerData& data) {
    uint64_t playerBase = GetPlayerBase();
    if (playerBase == 0) {
        return false;
    }
    
    uint64_t playerController = kernel->ReadPointer(playerBase);
    if (playerController == 0) {
        return false;
    }
    
    uint64_t localPlayer = kernel->ReadPointer(playerController + Offsets::LocalPlayer);
    if (localPlayer == 0) {
        return false;
    }
    
    // Read all stats
    kernel->Read(localPlayer + Offsets::Health, data.health);
    kernel->Read(localPlayer + Offsets::MaxHealth, data.maxHealth);
    kernel->Read(localPlayer + Offsets::Mana, data.mana);
    kernel->Read(localPlayer + Offsets::MaxMana, data.maxMana);
    kernel->Read(localPlayer + Offsets::Stamina, data.stamina);
    kernel->Read(localPlayer + Offsets::MaxStamina, data.maxStamina);
    kernel->Read(localPlayer + Offsets::Level, data.level);
    kernel->Read(localPlayer + Offsets::Experience, data.experience);
    kernel->Read(localPlayer + Offsets::Silver, data.silver);
    kernel->Read(localPlayer + Offsets::Weight, data.weight);
    kernel->Read(localPlayer + Offsets::MaxWeight, data.maxWeight);
    kernel->Read(localPlayer + Offsets::PosX, data.x);
    kernel->Read(localPlayer + Offsets::PosY, data.y);
    kernel->Read(localPlayer + Offsets::PosZ, data.z);
    kernel->Read(localPlayer + Offsets::Rotation, data.rotation);
    kernel->Read(localPlayer + Offsets::IsInCombat, data.inCombat);
    kernel->Read(localPlayer + Offsets::Target, data.targetEntity);
    
    return true;
}

// Get entity list
bool BDOGameMemory::GetEntityList(std::vector<EntityData>& entities) {
    entities.clear();
    
    uint64_t entityListBase = GetEntityListBase();
    if (entityListBase == 0) {
        return false;
    }
    
    uint64_t entityList = kernel->ReadPointer(entityListBase);
    if (entityList == 0) {
        return false;
    }
    
    int entityCount = 0;
    kernel->Read(entityList + Offsets::EntityCount, entityCount);
    
    if (entityCount <= 0 || entityCount > 10000) {
        return false;
    }
    
    uint64_t entityArray = kernel->ReadPointer(entityList + Offsets::EntityArray);
    if (entityArray == 0) {
        return false;
    }
    
    for (int i = 0; i < entityCount; i++) {
        uint64_t entity = kernel->ReadPointer(entityArray + (i * 8));
        if (entity == 0) {
            continue;
        }
        
        EntityData data;
        data.address = entity;
        
        kernel->Read(entity + Offsets::EntityType, data.type);
        kernel->Read(entity + Offsets::EntityHealth, data.health);
        kernel->Read(entity + Offsets::EntityPosition, data.x);
        kernel->Read(entity + Offsets::EntityPosition + 4, data.y);
        kernel->Read(entity + Offsets::EntityPosition + 8, data.z);
        kernel->Read(entity + Offsets::EntityDistance, data.distance);
        
        data.isAlive = data.health > 0;
        
        entities.push_back(data);
    }
    
    return true;
}

// Get nearby entities
bool BDOGameMemory::GetNearbyEntities(std::vector<EntityData>& entities, float maxDistance) {
    std::vector<EntityData> allEntities;
    if (!GetEntityList(allEntities)) {
        return false;
    }
    
    entities.clear();
    for (const auto& entity : allEntities) {
        if (entity.distance <= maxDistance) {
            entities.push_back(entity);
        }
    }
    
    return true;
}

// Calculate distance
float BDOGameMemory::CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

// Check if entity is in range
bool BDOGameMemory::IsEntityInRange(const EntityData& entity, float range) {
    return entity.distance <= range;
}



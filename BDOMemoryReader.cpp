#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <vector>
#include <string>
#include <Psapi.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

// Direct syscall implementations to bypass hooks
extern "C" NTSTATUS NTAPI NtReadVirtualMemory(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T Size,
    PSIZE_T NumberOfBytesRead
);

extern "C" NTSTATUS NTAPI NtWriteVirtualMemory(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T Size,
    PSIZE_T NumberOfBytesWritten
);

class BDOMemoryReader {
private:
    HANDLE hProcess;
    DWORD processId;
    uintptr_t baseAddress;
    
    // Bypass method flags
    bool useDirectSyscall = true;
    bool useKernelDriver = false;
    
public:
    BDOMemoryReader() : hProcess(nullptr), processId(0), baseAddress(0) {}
    
    ~BDOMemoryReader() {
        if (hProcess) CloseHandle(hProcess);
    }
    
    // Find Black Desert process
    bool FindGame() {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);
        
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        
        if (Process32First(snapshot, &entry)) {
            do {
                // Black Desert process names
                if (!_wcsicmp(entry.szExeFile, L"BlackDesert64.exe") ||
                    !_wcsicmp(entry.szExeFile, L"BlackDesertEAC.exe") ||
                    !_wcsicmp(entry.szExeFile, L"BlackDesertPatcher64.exe")) {
                    
                    processId = entry.th32ProcessID;
                    CloseHandle(snapshot);
                    
                    std::wcout << L"[+] Found Black Desert: " << entry.szExeFile 
                               << L" (PID: " << processId << L")" << std::endl;
                    return true;
                }
            } while (Process32Next(snapshot, &entry));
        }
        
        CloseHandle(snapshot);
        return false;
    }
    
    // Open process with minimal rights to avoid detection
    bool OpenGameProcess() {
        // Try different access rights to avoid detection
        DWORD accessRights[] = {
            PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
            PROCESS_VM_READ,
            PROCESS_QUERY_LIMITED_INFORMATION
        };
        
        for (DWORD rights : accessRights) {
            hProcess = OpenProcess(rights, FALSE, processId);
            if (hProcess) {
                std::cout << "[+] Opened process with access rights: 0x" 
                         << std::hex << rights << std::dec << std::endl;
                return true;
            }
        }
        
        std::cout << "[-] Failed to open process. Error: " << GetLastError() << std::endl;
        return false;
    }
    
    // Get module base address
    uintptr_t GetModuleBase(const wchar_t* moduleName) {
        HMODULE hMods[1024];
        DWORD cbNeeded;
        
        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
            for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
                wchar_t szModName[MAX_PATH];
                
                if (GetModuleFileNameEx(hProcess, hMods[i], szModName, MAX_PATH)) {
                    if (wcsstr(szModName, moduleName) != nullptr) {
                        MODULEINFO modInfo;
                        if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo))) {
                            return (uintptr_t)modInfo.lpBaseOfDll;
                        }
                    }
                }
            }
        }
        return 0;
    }
    
    // Read memory using direct syscall (bypasses user-mode hooks)
    template<typename T>
    T ReadMemorySyscall(uintptr_t address) {
        T value = {};
        SIZE_T bytesRead = 0;
        
        NTSTATUS status = NtReadVirtualMemory(
            hProcess,
            (PVOID)address,
            &value,
            sizeof(T),
            &bytesRead
        );
        
        if (!NT_SUCCESS(status)) {
            std::cout << "[-] NtReadVirtualMemory failed: 0x" << std::hex << status << std::dec << std::endl;
        }
        
        return value;
    }
    
    // Read memory using standard API (will be hooked)
    template<typename T>
    T ReadMemoryAPI(uintptr_t address) {
        T value = {};
        ReadProcessMemory(hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
        return value;
    }
    
    // Main read function - chooses method based on settings
    template<typename T>
    T Read(uintptr_t address) {
        if (useDirectSyscall) {
            return ReadMemorySyscall<T>(address);
        } else {
            return ReadMemoryAPI<T>(address);
        }
    }
    
    // Read string
    std::string ReadString(uintptr_t address, size_t maxLength = 256) {
        std::string result;
        char buffer[256] = {0};
        SIZE_T bytesRead = 0;
        
        if (useDirectSyscall) {
            NtReadVirtualMemory(hProcess, (PVOID)address, buffer, 
                              min(maxLength, sizeof(buffer)), &bytesRead);
        } else {
            ReadProcessMemory(hProcess, (LPCVOID)address, buffer, 
                            min(maxLength, sizeof(buffer)), &bytesRead);
        }
        
        result = std::string(buffer);
        return result;
    }
    
    // Pattern scanning to find addresses
    uintptr_t PatternScan(const char* pattern, const char* mask, uintptr_t start = 0, uintptr_t end = 0x7FFFFFFFFFFF) {
        MEMORY_BASIC_INFORMATION mbi = {};
        uintptr_t currentAddress = start;
        
        size_t patternLength = strlen(mask);
        
        while (currentAddress < end && VirtualQueryEx(hProcess, (LPCVOID)currentAddress, &mbi, sizeof(mbi))) {
            if (mbi.State == MEM_COMMIT && 
                (mbi.Protect == PAGE_EXECUTE_READWRITE || 
                 mbi.Protect == PAGE_EXECUTE_READ || 
                 mbi.Protect == PAGE_READWRITE)) {
                
                std::vector<BYTE> buffer(mbi.RegionSize);
                SIZE_T bytesRead = 0;
                
                if (useDirectSyscall) {
                    NtReadVirtualMemory(hProcess, mbi.BaseAddress, buffer.data(), 
                                      mbi.RegionSize, &bytesRead);
                } else {
                    ReadProcessMemory(hProcess, mbi.BaseAddress, buffer.data(), 
                                    mbi.RegionSize, &bytesRead);
                }
                
                if (bytesRead > 0) {
                    for (size_t i = 0; i < bytesRead - patternLength; i++) {
                        bool found = true;
                        
                        for (size_t j = 0; j < patternLength; j++) {
                            if (mask[j] == 'x' && buffer[i + j] != (BYTE)pattern[j]) {
                                found = false;
                                break;
                            }
                        }
                        
                        if (found) {
                            return (uintptr_t)mbi.BaseAddress + i;
                        }
                    }
                }
            }
            
            currentAddress = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
        }
        
        return 0;
    }
    
    // AOB (Array of Bytes) scan
    uintptr_t AOBScan(const std::string& aob, uintptr_t start = 0, uintptr_t end = 0x7FFFFFFFFFFF) {
        std::vector<BYTE> bytes;
        std::string mask;
        
        // Parse AOB string (format: "48 8B 05 ?? ?? ?? ?? 48 85 C9")
        std::istringstream iss(aob);
        std::string byte;
        
        while (iss >> byte) {
            if (byte == "??" || byte == "?") {
                bytes.push_back(0x00);
                mask += "?";
            } else {
                bytes.push_back((BYTE)std::stoul(byte, nullptr, 16));
                mask += "x";
            }
        }
        
        return PatternScan((const char*)bytes.data(), mask.c_str(), start, end);
    }
    
    // Get base address of main module
    uintptr_t GetBaseAddress() {
        if (baseAddress == 0) {
            baseAddress = GetModuleBase(L"BlackDesert64.exe");
        }
        return baseAddress;
    }
    
    // Multi-level pointer reading
    uintptr_t ReadPointerChain(uintptr_t base, const std::vector<uintptr_t>& offsets) {
        uintptr_t address = Read<uintptr_t>(base);
        
        for (size_t i = 0; i < offsets.size() - 1; i++) {
            if (address == 0) return 0;
            address = Read<uintptr_t>(address + offsets[i]);
        }
        
        if (offsets.size() > 0 && address != 0) {
            address += offsets.back();
        }
        
        return address;
    }
    
    void SetUseDirectSyscall(bool use) {
        useDirectSyscall = use;
        std::cout << "[*] Memory reading method: " 
                  << (use ? "Direct Syscall" : "Windows API") << std::endl;
    }
};

// Example: Black Desert memory structures and offsets
namespace BDO {
    // These offsets change with game updates - examples only
    namespace Offsets {
        // 2024 example offsets (need to be updated)
        constexpr uintptr_t PlayerBase = 0x3A5B2C8;
        constexpr uintptr_t EntityList = 0x3A5B2D0;
        
        // Player offsets
        constexpr uintptr_t Health = 0x2C;
        constexpr uintptr_t MaxHealth = 0x30;
        constexpr uintptr_t Mana = 0x34;
        constexpr uintptr_t MaxMana = 0x38;
        constexpr uintptr_t Level = 0x7C;
        constexpr uintptr_t PosX = 0x40;
        constexpr uintptr_t PosY = 0x44;
        constexpr uintptr_t PosZ = 0x48;
    }
    
    // Patterns for finding addresses dynamically
    namespace Patterns {
        // Example patterns (need real ones from game)
        const char* PlayerBasePattern = "\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC9\x74\x00\x48\x8B\x01";
        const char* PlayerBaseMask = "xxx????xxxx?xxx";
        
        const char* EntityListPattern = "\x48\x8B\x15\x00\x00\x00\x00\x48\x85\xD2\x74\x00\x48\x8B\x12";
        const char* EntityListMask = "xxx????xxxx?xxx";
    }
}

int main() {
    std::cout << "================================" << std::endl;
    std::cout << "Black Desert Memory Reader" << std::endl;
    std::cout << "Educational Purpose Only" << std::endl;
    std::cout << "================================\n" << std::endl;
    
    BDOMemoryReader reader;
    
    // Step 1: Find game
    std::cout << "[*] Searching for Black Desert process..." << std::endl;
    if (!reader.FindGame()) {
        std::cout << "[-] Black Desert not found. Make sure the game is running." << std::endl;
        return 1;
    }
    
    // Step 2: Open process
    std::cout << "[*] Opening process..." << std::endl;
    if (!reader.OpenGameProcess()) {
        std::cout << "[-] Failed to open process. Try running as administrator." << std::endl;
        return 1;
    }
    
    // Step 3: Get base address
    uintptr_t baseAddress = reader.GetBaseAddress();
    if (baseAddress == 0) {
        std::cout << "[-] Failed to get base address" << std::endl;
        return 1;
    }
    std::cout << "[+] Base address: 0x" << std::hex << baseAddress << std::dec << std::endl;
    
    // Step 4: Try different reading methods
    std::cout << "\n[*] Testing memory reading methods..." << std::endl;
    
    // Test with direct syscall (bypasses user-mode hooks)
    reader.SetUseDirectSyscall(true);
    std::cout << "\n[*] Attempting to read with direct syscall..." << std::endl;
    
    // Try to find player base using pattern
    std::cout << "[*] Scanning for player base pattern..." << std::endl;
    uintptr_t playerBaseAddr = reader.AOBScan("48 8B 05 ?? ?? ?? ?? 48 85 C9", baseAddress);
    
    if (playerBaseAddr != 0) {
        std::cout << "[+] Found pattern at: 0x" << std::hex << playerBaseAddr << std::dec << std::endl;
        
        // Calculate actual address from RIP-relative instruction
        int32_t offset = reader.Read<int32_t>(playerBaseAddr + 3);
        uintptr_t playerBase = playerBaseAddr + 7 + offset;
        
        std::cout << "[+] Player base: 0x" << std::hex << playerBase << std::dec << std::endl;
        
        // Read player data
        uintptr_t player = reader.Read<uintptr_t>(playerBase);
        if (player != 0) {
            float health = reader.Read<float>(player + BDO::Offsets::Health);
            float maxHealth = reader.Read<float>(player + BDO::Offsets::MaxHealth);
            uint32_t level = reader.Read<uint32_t>(player + BDO::Offsets::Level);
            
            std::cout << "\n[Player Data]" << std::endl;
            std::cout << "  Health: " << health << "/" << maxHealth << std::endl;
            std::cout << "  Level: " << level << std::endl;
        }
    } else {
        std::cout << "[-] Pattern not found. Offsets may have changed." << std::endl;
        
        // Try static address as fallback
        std::cout << "\n[*] Trying static addresses..." << std::endl;
        uintptr_t staticPlayerBase = baseAddress + BDO::Offsets::PlayerBase;
        
        // Try to read using pointer chain
        std::vector<uintptr_t> offsets = {0x8, 0x10, 0x30, BDO::Offsets::Health};
        uintptr_t healthAddr = reader.ReadPointerChain(staticPlayerBase, offsets);
        
        if (healthAddr != 0) {
            float health = reader.Read<float>(healthAddr);
            std::cout << "[*] Health value: " << health << std::endl;
        }
    }
    
    // Alternative: Test with Windows API (will likely be blocked)
    std::cout << "\n[*] Testing standard Windows API (likely blocked)..." << std::endl;
    reader.SetUseDirectSyscall(false);
    
    // Try to read a simple value
    uint32_t testValue = reader.Read<uint32_t>(baseAddress);
    if (testValue != 0) {
        std::cout << "[!] Surprisingly, API read worked: 0x" << std::hex << testValue << std::dec << std::endl;
    } else {
        std::cout << "[*] As expected, API read was blocked by anti-cheat" << std::endl;
    }
    
    std::cout << "\n[*] Memory reading test complete." << std::endl;
    std::cout << "\n[!] Note: If reading failed, it's because:" << std::endl;
    std::cout << "    1. Anti-cheat is blocking memory access" << std::endl;
    std::cout << "    2. Offsets have changed with game update" << std::endl;
    std::cout << "    3. Additional bypass methods are needed" << std::endl;
    
    return 0;
}


#include "BDOAntiCheatResearch.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>
#include <dbghelp.h>
#include <winternl.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")

// Implementation class
class BDOAntiCheatResearch::Impl {
public:
    std::map<std::string, std::vector<APICall>> apiCallHistory;
    bool isMonitoring = false;
    std::vector<BypassTechnique> bypassTechniques;
    
    Impl() {
        InitializeBypassTechniques();
    }
    
    void InitializeBypassTechniques() {
        // Educational bypass techniques
        bypassTechniques.push_back({
            "Manual Mapping",
            "Load DLL without using standard Windows APIs",
            []() { return false; },  // Placeholder
            false,
            7
        });
        
        bypassTechniques.push_back({
            "Direct Syscalls",
            "Bypass user-mode hooks by calling kernel directly",
            []() { return false; },  // Placeholder
            false,
            5
        });
        
        bypassTechniques.push_back({
            "Handle Hijacking",
            "Use existing handles to access protected memory",
            []() { return false; },  // Placeholder
            false,
            8
        });
    }
};

// Constructor and Destructor
BDOAntiCheatResearch::BDOAntiCheatResearch() 
    : pImpl(std::make_unique<Impl>()), gameProcess(nullptr), gameProcessId(0), gameWindow(nullptr) {
}

BDOAntiCheatResearch::~BDOAntiCheatResearch() {
    Cleanup();
}

// Initialize research tools
bool BDOAntiCheatResearch::Initialize() {
    std::cout << "[*] Initializing BDO Anti-Cheat Research Framework..." << std::endl;
    
    // Enable debug privileges
    if (!EnableDebugPrivileges()) {
        std::cout << "[-] Failed to enable debug privileges" << std::endl;
        return false;
    }
    
    // Find Black Desert window
    gameWindow = FindWindowA("BlackDesertWindowClass", nullptr);
    if (!gameWindow) {
        gameWindow = FindWindowA(nullptr, "Black Desert");
        if (!gameWindow) {
            std::cout << "[-] Black Desert window not found" << std::endl;
            return false;
        }
    }
    
    // Get process ID
    GetWindowThreadProcessId(gameWindow, &gameProcessId);
    if (gameProcessId == 0) {
        std::cout << "[-] Failed to get process ID" << std::endl;
        return false;
    }
    
    std::cout << "[+] Found Black Desert (PID: " << gameProcessId << ")" << std::endl;
    
    // Load pattern database
    LoadPatternDatabase("patterns.db");
    
    return true;
}

bool BDOAntiCheatResearch::AttachToGame() {
    if (!OpenGameProcess()) {
        return false;
    }
    
    // Get loaded modules
    loadedModules = GetProcessModules();
    std::cout << "[+] Found " << loadedModules.size() << " loaded modules" << std::endl;
    
    // Initial anti-cheat detection
    auto antiCheatInfo = DetectAntiCheat();
    if (antiCheatInfo.isActive) {
        std::cout << "[!] Anti-cheat detected: " << 
                     std::string(antiCheatInfo.name.begin(), antiCheatInfo.name.end()) << std::endl;
        std::cout << "[!] Protection level: " << antiCheatInfo.protectionLevel << "/10" << std::endl;
    }
    
    return true;
}

void BDOAntiCheatResearch::Cleanup() {
    StopAPIMonitoring();
    CloseGameProcess();
    SavePatternDatabase("patterns.db");
}

// Anti-Cheat Detection
BDOAntiCheatResearch::AntiCheatInfo BDOAntiCheatResearch::DetectAntiCheat() {
    AntiCheatInfo info;
    info.isActive = false;
    info.protectionLevel = 0;
    
    // Check for XIGNCODE
    if (IsXigncodeActive()) {
        info.name = L"XIGNCODE3";
        info.isActive = true;
        info.protectionLevel = 8;
        
        // Find XIGNCODE hooks
        if (IsModuleLoaded(BDOResearch::AntiCheat::XIGNCODE_DLL)) {
            auto hooks = DetectHooks();
            for (const auto& hook : hooks) {
                info.hookAddresses.push_back(hook.address);
            }
        }
        
        // Protected functions by XIGNCODE
        info.protectedFunctions = {
            "OpenProcess", "ReadProcessMemory", "WriteProcessMemory",
            "VirtualQueryEx", "CreateToolhelp32Snapshot", "NtQuerySystemInformation"
        };
    }
    
    // Check for EasyAntiCheat
    if (IsEACActive()) {
        info.name = L"EasyAntiCheat";
        info.isActive = true;
        info.protectionLevel = 9;
        
        // EAC specific protected functions
        info.protectedFunctions.push_back("NtReadVirtualMemory");
        info.protectedFunctions.push_back("NtWriteVirtualMemory");
    }
    
    return info;
}

bool BDOAntiCheatResearch::IsXigncodeActive() {
    // Check for XIGNCODE module
    if (IsModuleLoaded(BDOResearch::AntiCheat::XIGNCODE_DLL)) {
        return true;
    }
    
    // Check for XIGNCODE driver
    if (IsKernelDriverActive(BDOResearch::AntiCheat::XIGNCODE_DRV)) {
        return true;
    }
    
    // Check for XIGNCODE process
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(snapshot, &pe32)) {
            do {
                if (wcsstr(pe32.szExeFile, L"xigncode") != nullptr ||
                    wcsstr(pe32.szExeFile, L"x3.exe") != nullptr) {
                    CloseHandle(snapshot);
                    return true;
                }
            } while (Process32NextW(snapshot, &pe32));
        }
        CloseHandle(snapshot);
    }
    
    return false;
}

bool BDOAntiCheatResearch::IsEACActive() {
    // Check for EAC module
    if (IsModuleLoaded(BDOResearch::AntiCheat::EAC_DLL)) {
        return true;
    }
    
    // Check for EAC service
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(snapshot, &pe32)) {
            do {
                if (wcscmp(pe32.szExeFile, BDOResearch::AntiCheat::EAC_SERVICE) == 0) {
                    CloseHandle(snapshot);
                    return true;
                }
            } while (Process32NextW(snapshot, &pe32));
        }
        CloseHandle(snapshot);
    }
    
    return false;
}

// Memory Access Research
std::vector<BDOAntiCheatResearch::MemoryRegion> BDOAntiCheatResearch::ScanMemoryRegions() {
    std::vector<MemoryRegion> regions;
    
    if (!gameProcess) return regions;
    
    MEMORY_BASIC_INFORMATION mbi;
    uintptr_t address = 0;
    
    while (VirtualQueryEx(gameProcess, (LPCVOID)address, &mbi, sizeof(mbi))) {
        if (mbi.State == MEM_COMMIT) {
            MemoryRegion region;
            region.baseAddress = (uintptr_t)mbi.BaseAddress;
            region.size = mbi.RegionSize;
            region.protection = mbi.Protect;
            
            // Determine region type
            if (mbi.Type == MEM_IMAGE) {
                region.type = "Code";
            } else if (mbi.Type == MEM_PRIVATE) {
                region.type = "Private";
            } else if (mbi.Type == MEM_MAPPED) {
                region.type = "Mapped";
            } else {
                region.type = "Unknown";
            }
            
            // Check if protected
            region.isProtected = (mbi.Protect & PAGE_GUARD) || 
                               (mbi.Protect == PAGE_NOACCESS);
            
            // Check for hooks (simplified)
            region.hasHooks = IsAddressHooked(region.baseAddress);
            
            regions.push_back(region);
        }
        
        address += mbi.RegionSize;
    }
    
    return regions;
}

template<typename T>
std::optional<T> BDOAntiCheatResearch::SafeReadMemory(uintptr_t address) {
    if (!IsAddressValid(address)) {
        return std::nullopt;
    }
    
    // Add anti-detection delay
    RandomDelay();
    
    T value;
    SIZE_T bytesRead;
    
    // Use direct syscall if hooks detected
    if (IsAddressHooked((uintptr_t)ReadProcessMemory)) {
        // Would implement direct syscall here
        // For now, fallback to regular API
    }
    
    if (ReadProcessMemory(gameProcess, (LPCVOID)address, &value, sizeof(T), &bytesRead)) {
        if (bytesRead == sizeof(T)) {
            return value;
        }
    }
    
    return std::nullopt;
}

// Pattern Scanning
std::vector<uintptr_t> BDOAntiCheatResearch::PatternScan(const std::string& pattern, const std::string& mask) {
    std::vector<uintptr_t> results;
    
    if (!gameProcess) return results;
    
    // Convert pattern string to bytes
    std::vector<BYTE> patternBytes;
    std::istringstream iss(pattern);
    std::string byte;
    
    while (iss >> std::hex >> byte) {
        patternBytes.push_back(static_cast<BYTE>(std::stoi(byte, nullptr, 16)));
    }
    
    // Scan all memory regions
    auto regions = ScanMemoryRegions();
    for (const auto& region : regions) {
        if (region.isProtected) continue;  // Skip protected regions
        
        std::vector<BYTE> buffer(region.size);
        SIZE_T bytesRead;
        
        if (ReadProcessMemory(gameProcess, (LPCVOID)region.baseAddress, 
                            buffer.data(), region.size, &bytesRead)) {
            for (size_t i = 0; i <= bytesRead - patternBytes.size(); i++) {
                if (ComparePattern(&buffer[i], patternBytes.data(), mask.c_str())) {
                    results.push_back(region.baseAddress + i);
                }
            }
        }
    }
    
    return results;
}

// Hook Detection
std::vector<BDOAntiCheatResearch::HookInfo> BDOAntiCheatResearch::DetectHooks() {
    std::vector<HookInfo> hooks;
    
    // Common hooked functions
    std::vector<std::pair<std::string, std::string>> commonAPIs = {
        {"kernel32.dll", "OpenProcess"},
        {"kernel32.dll", "ReadProcessMemory"},
        {"kernel32.dll", "WriteProcessMemory"},
        {"ntdll.dll", "NtQuerySystemInformation"},
        {"ntdll.dll", "NtReadVirtualMemory"},
        {"user32.dll", "GetAsyncKeyState"},
        {"user32.dll", "GetCursorPos"}
    };
    
    for (const auto& [module, function] : commonAPIs) {
        HMODULE hModule = GetModuleHandleA(module.c_str());
        if (hModule) {
            FARPROC proc = GetProcAddress(hModule, function.c_str());
            if (proc) {
                uintptr_t address = (uintptr_t)proc;
                if (IsAddressHooked(address)) {
                    HookInfo hook = AnalyzeHook(address);
                    hook.functionName = module + "!" + function;
                    hooks.push_back(hook);
                }
            }
        }
    }
    
    return hooks;
}

bool BDOAntiCheatResearch::IsAddressHooked(uintptr_t address) {
    if (!gameProcess) return false;
    
    BYTE bytes[16];
    SIZE_T bytesRead;
    
    if (ReadProcessMemory(gameProcess, (LPCVOID)address, bytes, sizeof(bytes), &bytesRead)) {
        // Check for common hook patterns
        if (bytes[0] == 0xE9 || bytes[0] == 0xE8) {  // JMP or CALL
            return true;
        }
        
        if (bytes[0] == 0x48 && bytes[1] == 0xB8) {  // MOV RAX, IMM64
            return true;
        }
        
        if (bytes[0] == 0xCC) {  // INT3 breakpoint
            return true;
        }
        
        if (bytes[0] == 0xFF && (bytes[1] == 0x25 || bytes[1] == 0x15)) {  // JMP/CALL indirect
            return true;
        }
    }
    
    return false;
}

BDOAntiCheatResearch::HookInfo BDOAntiCheatResearch::AnalyzeHook(uintptr_t address) {
    HookInfo info;
    info.address = address;
    info.type = BDOResearch::HookType::INLINE_HOOK;  // Default
    
    // Read hook bytes
    BYTE bytes[32];
    SIZE_T bytesRead;
    
    if (ReadProcessMemory(gameProcess, (LPCVOID)address, bytes, sizeof(bytes), &bytesRead)) {
        info.hookedBytes.assign(bytes, bytes + bytesRead);
        
        // Analyze hook type
        if (bytes[0] == 0xE9) {  // JMP
            info.type = BDOResearch::HookType::INLINE_HOOK;
            // Calculate jump target
            int32_t offset = *(int32_t*)&bytes[1];
            info.jumpTarget = address + 5 + offset;
        } else if (bytes[0] == 0xCC) {  // Breakpoint
            info.type = BDOResearch::HookType::HARDWARE_BREAKPOINT;
        } else if (bytes[0] == 0x48 && bytes[1] == 0xB8) {  // MOV RAX
            info.type = BDOResearch::HookType::INLINE_HOOK;
            info.jumpTarget = *(uintptr_t*)&bytes[2];
        }
    }
    
    return info;
}

// Process Integrity
BDOAntiCheatResearch::IntegrityCheck BDOAntiCheatResearch::CheckProcessIntegrity() {
    IntegrityCheck check;
    
    // Check for debugger
    check.debuggerPresent = IsBeingDebugged();
    
    // Check for VM
    check.vmDetected = IsRunningInVM();
    
    // Check for sandbox
    check.sandboxDetected = false;  // Implement sandbox detection
    
    // Check for common hooking tools
    check.hookingToolsDetected = false;
    std::vector<std::wstring> hookingTools = {
        L"CheatEngine", L"x64dbg", L"IDA", L"OllyDbg", 
        L"ReClass", L"ProcessHacker", L"Wireshark"
    };
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(snapshot, &pe32)) {
            do {
                for (const auto& tool : hookingTools) {
                    if (wcsstr(pe32.szExeFile, tool.c_str()) != nullptr) {
                        check.hookingToolsDetected = true;
                        check.detectedTools.push_back(
                            std::string(pe32.szExeFile, 
                                      pe32.szExeFile + wcslen(pe32.szExeFile))
                        );
                    }
                }
            } while (Process32NextW(snapshot, &pe32));
        }
        CloseHandle(snapshot);
    }
    
    // CRC check (simplified)
    check.crcCheck = true;  // Would implement actual CRC checking
    
    return check;
}

bool BDOAntiCheatResearch::IsBeingDebugged() {
    // Multiple debugger detection methods
    
    // Method 1: IsDebuggerPresent
    if (IsDebuggerPresent()) {
        return true;
    }
    
    // Method 2: CheckRemoteDebuggerPresent
    BOOL isDebuggerPresent = FALSE;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebuggerPresent)) {
        if (isDebuggerPresent) {
            return true;
        }
    }
    
    // Method 3: PEB check
    PPEB peb = (PPEB)__readgsqword(0x60);
    if (peb && peb->BeingDebugged) {
        return true;
    }
    
    // Method 4: NtGlobalFlag check
    if (peb && (*(PDWORD)((PBYTE)peb + 0xBC) & 0x70)) {
        return true;
    }
    
    return false;
}

bool BDOAntiCheatResearch::IsRunningInVM() {
    // Check for VM artifacts
    
    // Method 1: Check for VM-specific registry keys
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                     "SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum",
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char buffer[256];
        DWORD bufferSize = sizeof(buffer);
        DWORD type;
        
        if (RegQueryValueExA(hKey, "0", nullptr, &type, 
                           (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            std::string diskName(buffer);
            if (diskName.find("VMware") != std::string::npos ||
                diskName.find("VBOX") != std::string::npos ||
                diskName.find("Virtual") != std::string::npos) {
                RegCloseKey(hKey);
                return true;
            }
        }
        RegCloseKey(hKey);
    }
    
    // Method 2: CPUID check
    int cpuInfo[4];
    __cpuid(cpuInfo, 1);
    if ((cpuInfo[2] >> 31) & 1) {  // Hypervisor bit
        return true;
    }
    
    return false;
}

// Thread Analysis
std::vector<BDOAntiCheatResearch::ThreadInfo> BDOAntiCheatResearch::AnalyzeThreads() {
    std::vector<ThreadInfo> threads;
    
    if (!gameProcess) return threads;
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, gameProcessId);
    if (snapshot != INVALID_HANDLE_VALUE) {
        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);
        
        if (Thread32First(snapshot, &te32)) {
            do {
                if (te32.th32OwnerProcessID == gameProcessId) {
                    ThreadInfo info;
                    info.threadId = te32.th32ThreadID;
                    
                    // Get thread start address
                    HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
                    if (hThread) {
                        // Would get start address here
                        info.startAddress = 0;  // Placeholder
                        
                        // Check if thread belongs to known module
                        info.moduleName = GetModuleName(info.startAddress);
                        
                        // Check if suspicious
                        info.isSuspicious = info.moduleName.empty() || 
                                          info.moduleName.find("unknown") != std::string::npos;
                        
                        CloseHandle(hThread);
                    }
                    
                    threads.push_back(info);
                }
            } while (Thread32Next(snapshot, &te32));
        }
        CloseHandle(snapshot);
    }
    
    return threads;
}

// Bypass Research
std::vector<BDOAntiCheatResearch::BypassTechnique> BDOAntiCheatResearch::GetKnownBypassTechniques() {
    return pImpl->bypassTechniques;
}

bool BDOAntiCheatResearch::TestBypassTechnique(const std::string& techniqueName) {
    for (auto& technique : pImpl->bypassTechniques) {
        if (technique.name == techniqueName) {
            technique.isWorking = technique.testFunction();
            return technique.isWorking;
        }
    }
    return false;
}

// Reporting
BDOAntiCheatResearch::ResearchReport BDOAntiCheatResearch::GenerateReport() {
    ResearchReport report;
    
    report.timestamp = std::chrono::system_clock::now();
    report.antiCheat = DetectAntiCheat();
    report.integrity = CheckProcessIntegrity();
    report.hooks = DetectHooks();
    report.protectedRegions = ScanMemoryRegions();
    report.threads = AnalyzeThreads();
    report.drivers = EnumerateDrivers();
    
    return report;
}

void BDOAntiCheatResearch::SaveReport(const ResearchReport& report, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;
    
    file << "=== BDO Anti-Cheat Research Report ===" << std::endl;
    
    // Timestamp
    auto time_t = std::chrono::system_clock::to_time_t(report.timestamp);
    file << "Generated: " << std::ctime(&time_t) << std::endl;
    
    // Anti-cheat info
    file << "\n[Anti-Cheat Detection]" << std::endl;
    file << "Active: " << (report.antiCheat.isActive ? "Yes" : "No") << std::endl;
    if (report.antiCheat.isActive) {
        file << "Name: " << std::string(report.antiCheat.name.begin(), 
                                       report.antiCheat.name.end()) << std::endl;
        file << "Protection Level: " << report.antiCheat.protectionLevel << "/10" << std::endl;
        file << "Hook Count: " << report.antiCheat.hookAddresses.size() << std::endl;
    }
    
    // Integrity check
    file << "\n[Process Integrity]" << std::endl;
    file << "Debugger Present: " << (report.integrity.debuggerPresent ? "Yes" : "No") << std::endl;
    file << "VM Detected: " << (report.integrity.vmDetected ? "Yes" : "No") << std::endl;
    file << "Hooking Tools: " << (report.integrity.hookingToolsDetected ? "Yes" : "No") << std::endl;
    
    // Hooks
    file << "\n[Detected Hooks]" << std::endl;
    file << "Total Hooks: " << report.hooks.size() << std::endl;
    for (const auto& hook : report.hooks) {
        file << std::hex << "  - 0x" << hook.address << " (" << hook.functionName << ")" << std::endl;
    }
    
    // Protected regions
    file << "\n[Protected Memory Regions]" << std::endl;
    int protectedCount = 0;
    for (const auto& region : report.protectedRegions) {
        if (region.isProtected) {
            protectedCount++;
            file << std::hex << "  - 0x" << region.baseAddress 
                 << " (Size: 0x" << region.size << ")" << std::endl;
        }
    }
    file << "Total Protected: " << protectedCount << std::endl;
    
    file.close();
}

void BDOAntiCheatResearch::PrintReport(const ResearchReport& report) {
    std::cout << "\n=== BDO Anti-Cheat Research Report ===" << std::endl;
    
    // Anti-cheat status
    std::cout << "\n[Anti-Cheat Status]" << std::endl;
    if (report.antiCheat.isActive) {
        std::wcout << L"  Active: " << report.antiCheat.name << std::endl;
        std::cout << "  Protection Level: " << report.antiCheat.protectionLevel << "/10" << std::endl;
        std::cout << "  Hooked Functions: " << report.antiCheat.protectedFunctions.size() << std::endl;
    } else {
        std::cout << "  No active anti-cheat detected" << std::endl;
    }
    
    // Integrity
    std::cout << "\n[Environment Check]" << std::endl;
    std::cout << "  Debugger: " << (report.integrity.debuggerPresent ? "DETECTED" : "Clean") << std::endl;
    std::cout << "  Virtual Machine: " << (report.integrity.vmDetected ? "DETECTED" : "Clean") << std::endl;
    std::cout << "  Suspicious Tools: " << (report.integrity.hookingToolsDetected ? "DETECTED" : "Clean") << std::endl;
    
    // Hooks
    std::cout << "\n[Hook Analysis]" << std::endl;
    std::cout << "  Total Hooks Found: " << report.hooks.size() << std::endl;
    
    // Memory
    int protectedRegions = 0;
    for (const auto& region : report.protectedRegions) {
        if (region.isProtected) protectedRegions++;
    }
    std::cout << "\n[Memory Protection]" << std::endl;
    std::cout << "  Total Regions: " << report.protectedRegions.size() << std::endl;
    std::cout << "  Protected Regions: " << protectedRegions << std::endl;
    
    // Threads
    int suspiciousThreads = 0;
    for (const auto& thread : report.threads) {
        if (thread.isSuspicious) suspiciousThreads++;
    }
    std::cout << "\n[Thread Analysis]" << std::endl;
    std::cout << "  Total Threads: " << report.threads.size() << std::endl;
    std::cout << "  Suspicious Threads: " << suspiciousThreads << std::endl;
}

// Helper Methods
bool BDOAntiCheatResearch::OpenGameProcess() {
    if (gameProcessId == 0) return false;
    
    gameProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, gameProcessId);
    if (!gameProcess) {
        // Try with reduced privileges
        gameProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, 
                                 FALSE, gameProcessId);
    }
    
    return gameProcess != nullptr;
}

void BDOAntiCheatResearch::CloseGameProcess() {
    if (gameProcess) {
        CloseHandle(gameProcess);
        gameProcess = nullptr;
    }
}

std::vector<HMODULE> BDOAntiCheatResearch::GetProcessModules() {
    std::vector<HMODULE> modules;
    
    if (!gameProcess) return modules;
    
    HMODULE hMods[1024];
    DWORD cbNeeded;
    
    if (EnumProcessModules(gameProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            modules.push_back(hMods[i]);
        }
    }
    
    return modules;
}

bool BDOAntiCheatResearch::IsModuleLoaded(const std::wstring& moduleName) {
    for (const auto& module : loadedModules) {
        wchar_t modName[MAX_PATH];
        if (GetModuleFileNameExW(gameProcess, module, modName, MAX_PATH)) {
            if (wcsstr(modName, moduleName.c_str()) != nullptr) {
                return true;
            }
        }
    }
    return false;
}

bool BDOAntiCheatResearch::EnableDebugPrivileges() {
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

void BDOAntiCheatResearch::RandomDelay(int min, int max) {
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(min, max);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
}

bool BDOAntiCheatResearch::IsAddressValid(uintptr_t address) {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(gameProcess, (LPCVOID)address, &mbi, sizeof(mbi))) {
        return (mbi.State == MEM_COMMIT) && 
               (mbi.Protect != PAGE_NOACCESS) && 
               !(mbi.Protect & PAGE_GUARD);
    }
    return false;
}

bool BDOAntiCheatResearch::ComparePattern(const BYTE* data, const BYTE* pattern, const char* mask) {
    for (; *mask; ++mask, ++data, ++pattern) {
        if (*mask == 'x' && *data != *pattern) {
            return false;
        }
    }
    return true;
}

std::string BDOAntiCheatResearch::GetModuleName(uintptr_t address) {
    for (const auto& module : loadedModules) {
        MODULEINFO modInfo;
        if (GetModuleInformation(gameProcess, module, &modInfo, sizeof(modInfo))) {
            uintptr_t base = (uintptr_t)modInfo.lpBaseOfDll;
            uintptr_t end = base + modInfo.SizeOfImage;
            
            if (address >= base && address < end) {
                wchar_t modName[MAX_PATH];
                if (GetModuleFileNameExW(gameProcess, module, modName, MAX_PATH)) {
                    std::wstring ws(modName);
                    return std::string(ws.begin(), ws.end());
                }
            }
        }
    }
    return "unknown";
}

// Driver enumeration
std::vector<BDOAntiCheatResearch::DriverInfo> BDOAntiCheatResearch::EnumerateDrivers() {
    std::vector<DriverInfo> drivers;
    
    // This would require kernel-level access or admin privileges
    // Simplified implementation
    
    return drivers;
}

bool BDOAntiCheatResearch::IsKernelDriverActive(const std::wstring& driverName) {
    // Check if specific kernel driver is loaded
    // Would require admin privileges to fully implement
    
    SC_HANDLE scManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
    if (scManager) {
        // Would enumerate services and check for driver
        CloseServiceHandle(scManager);
    }
    
    return false;
}

// Pattern database
void BDOAntiCheatResearch::LoadPatternDatabase(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return;
    
    // Load saved patterns
    // Format: name|pattern_bytes
    
    file.close();
}

void BDOAntiCheatResearch::SavePatternDatabase(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return;
    
    // Save patterns
    for (const auto& [name, pattern] : patternDatabase) {
        file << name << "|";
        for (const auto& byte : pattern) {
            file << std::hex << std::setw(2) << std::setfill('0') 
                 << static_cast<int>(byte) << " ";
        }
        file << std::endl;
    }
    
    file.close();
}

// Game-specific offsets
BDOAntiCheatResearch::GameOffsets BDOAntiCheatResearch::FindGameOffsets() {
    GameOffsets offsets;
    
    // These would be found through pattern scanning
    // Example patterns for BDO (these change with updates)
    
    // Player base pattern
    auto playerBaseResults = PatternScan("48 8B 0D ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 01", "xxx????xxxxxxx");
    if (!playerBaseResults.empty()) {
        offsets.playerBase = playerBaseResults[0];
    }
    
    // Entity list pattern
    auto entityListResults = PatternScan("48 8B 15 ?? ?? ?? ?? 48 85 D2 74 ?? 48 8B 12", "xxx????xxxxxxx");
    if (!entityListResults.empty()) {
        offsets.entityList = entityListResults[0];
    }
    
    return offsets;
}

// Template instantiations
template std::optional<int> BDOAntiCheatResearch::SafeReadMemory<int>(uintptr_t);
template std::optional<float> BDOAntiCheatResearch::SafeReadMemory<float>(uintptr_t);
template std::optional<uintptr_t> BDOAntiCheatResearch::SafeReadMemory<uintptr_t>(uintptr_t);
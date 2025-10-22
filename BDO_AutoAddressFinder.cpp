/*
 * BDO_AutoAddressFinder.cpp
 * Automatically finds BDO memory addresses using kernel driver
 * NO CHEAT ENGINE NEEDED - uses BDO_SecureKernel pattern scanning
 */

#include "BDO_SecureClient.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

// Find BDO process
DWORD FindBDO() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    
    PROCESSENTRY32W entry = {sizeof(entry)};
    if (Process32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, L"BlackDesert64.exe") == 0) {
                CloseHandle(snap);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snap, &entry));
    }
    
    CloseHandle(snap);
    return 0;
}

// Calculate RIP-relative address
ULONG64 ResolveRIPRelative(SecureKernelClient& client, DWORD pid, ULONG64 instructionAddr, int instructionLength) {
    // Read the offset (4 bytes after the instruction starts, usually at offset 3)
    DWORD offset = 0;
    if (!client.Read<DWORD>(pid, instructionAddr + 3, offset)) {
        return 0;
    }
    
    // RIP-relative: NextInstruction + Offset
    ULONG64 nextInstr = instructionAddr + instructionLength;
    ULONG64 targetAddr = nextInstr + (LONG)offset; // Sign-extend offset
    
    return targetAddr;
}

// Patterns from BDO_PATTERNS.txt
struct BDOPattern {
    const char* name;
    const char* pattern;
    const char* description;
    int instructionLength; // For RIP-relative calculation
};

BDOPattern g_Patterns[] = {
    {"PlayerBase1", "48 8B 0D ?? ?? ?? ?? 48 85 C9", "MOV RCX, [RIP+offset]; TEST RCX", 7},
    {"PlayerBase2", "48 8B 05 ?? ?? ?? ?? 48 85 C0", "MOV RAX, [RIP+offset]; TEST RAX", 7},
    {"PlayerBase3", "48 8B 15 ?? ?? ?? ?? 48 85 D2 74", "MOV RDX, [RIP+offset]; TEST RDX; JZ", 7},
    {"PlayerBase4", "4C 8B 05 ?? ?? ?? ?? 4D 85 C0", "MOV R8, [RIP+offset]; TEST R8", 7},
    {"EntityList", "48 8B 1D ?? ?? ?? ?? 48 85 DB 74", "MOV RBX, [RIP+offset]; TEST RBX; JZ", 7},
    {"WorldBase", "48 8B 35 ?? ?? ?? ?? 48 85 F6", "MOV RSI, [RIP+offset]; TEST RSI", 7},
};

int main() {
    std::cout << "================================================================================\n";
    std::cout << "BDO Auto Address Finder - NO CHEAT ENGINE NEEDED\n";
    std::cout << "================================================================================\n\n";
    
    std::cout << "This tool uses your kernel driver to scan BDO memory\n";
    std::cout << "Xigncode3 won't detect it because it runs in kernel mode!\n\n";
    
    // Find BDO
    std::cout << "[*] Looking for Black Desert...\n";
    DWORD bdoPid = FindBDO();
    if (bdoPid == 0) {
        std::cerr << "[-] Black Desert not running\n";
        std::cerr << "[!] Start BDO first, then run this\n";
        return 1;
    }
    std::cout << "[+] Found BDO (PID: " << bdoPid << ")\n\n";
    
    // Initialize kernel client
    std::cout << "[*] Initializing kernel driver...\n";
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cerr << "[-] Failed: " << client.GetLastErrorString() << "\n";
        std::cerr << "[!] Make sure BDO_SecureKernel.sys is loaded\n";
        std::cerr << "[!] Run: install_driver.bat\n";
        return 1;
    }
    std::cout << "[+] Kernel driver connected\n\n";
    
    // Enable stealth so Xigncode3 doesn't detect us
    std::cout << "[*] Enabling stealth...\n";
    client.EnableDriverHiding();
    client.RemoveXigncodeCallbacks();
    std::cout << "[+] Stealth enabled\n\n";
    
    // Get BDO base address
    std::cout << "[*] Getting BDO base address...\n";
    ULONG64 baseAddr = 0;
    if (!client.GetProcessBaseAddress(bdoPid, baseAddr)) {
        std::cerr << "[-] Failed to get base address\n";
        return 1;
    }
    std::cout << "[+] BDO Base: 0x" << std::hex << baseAddr << std::dec << "\n\n";
    
    // Scan main module (typically 256MB)
    ULONG64 startAddr = baseAddr;
    ULONG64 endAddr = baseAddr + 0x10000000; // 256MB
    
    std::cout << "================================================================================\n";
    std::cout << "Scanning for patterns...\n";
    std::cout << "Range: 0x" << std::hex << startAddr << " - 0x" << endAddr << std::dec << "\n";
    std::cout << "This may take 1-2 minutes...\n";
    std::cout << "================================================================================\n\n";
    
    // Try each pattern
    int patternsFound = 0;
    
    for (const auto& pattern : g_Patterns) {
        std::cout << "[" << pattern.name << "] Scanning for: " << pattern.pattern << "\n";
        std::cout << "  Description: " << pattern.description << "\n";
        
        ULONG64 result = 0;
        if (client.ScanPattern(bdoPid, startAddr, endAddr, pattern.pattern, result)) {
            std::cout << "  [FOUND] Pattern at: 0x" << std::hex << result << std::dec << "\n";
            
            // Calculate RIP-relative address
            ULONG64 pointerAddr = ResolveRIPRelative(client, bdoPid, result, pattern.instructionLength);
            if (pointerAddr != 0) {
                std::cout << "  [CALC] Pointer location: 0x" << std::hex << pointerAddr << std::dec << "\n";
                
                // Read the actual pointer value
                ULONG64 actualPointer = 0;
                if (client.Read<ULONG64>(bdoPid, pointerAddr, actualPointer)) {
                    std::cout << "  [VALUE] Points to: 0x" << std::hex << actualPointer << std::dec << "\n";
                    
                    if (actualPointer != 0) {
                        std::cout << "  [SUCCESS] This looks valid!\n";
                        patternsFound++;
                        
                        // Save to file for later use
                        FILE* f = fopen("BDO_FOUND_ADDRESSES.txt", "a");
                        if (f) {
                            fprintf(f, "%s=0x%llX (pointer at 0x%llX)\n", 
                                    pattern.name, actualPointer, pointerAddr);
                            fclose(f);
                        }
                    }
                }
            }
        } else {
            std::cout << "  [NOT FOUND] Pattern not in scanned range\n";
        }
        std::cout << "\n";
    }
    
    std::cout << "================================================================================\n";
    std::cout << "Scan Complete!\n";
    std::cout << "================================================================================\n";
    std::cout << "Patterns found: " << patternsFound << " / " << (sizeof(g_Patterns) / sizeof(BDOPattern)) << "\n";
    
    if (patternsFound > 0) {
        std::cout << "\n[SUCCESS] Found addresses saved to: BDO_FOUND_ADDRESSES.txt\n";
        std::cout << "\nNext steps:\n";
        std::cout << "1. Open BDO_FOUND_ADDRESSES.txt\n";
        std::cout << "2. Use PlayerBase address in BDO_PracticalBot.cpp\n";
        std::cout << "3. Update the offset: baseAddr + 0x5A8B1C8 with real address\n";
        std::cout << "4. Recompile and test\n";
    } else {
        std::cout << "\n[WARN] No patterns found\n";
        std::cout << "Patterns may have changed with BDO update\n";
        std::cout << "\nSolutions:\n";
        std::cout << "1. Try scanning smaller ranges (game just updated?)\n";
        std::cout << "2. Check BDO_PATTERNS.txt for more patterns\n";
        std::cout << "3. Use IDA/Ghidra to find new patterns\n";
        std::cout << "4. Check community forums for updated patterns\n";
    }
    
    client.Shutdown();
    
    std::cout << "\nPress Enter to exit...\n";
    std::cin.get();
    
    return 0;
}


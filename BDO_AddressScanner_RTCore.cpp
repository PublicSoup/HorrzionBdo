/*
 * BDO_AddressScanner_RTCore.cpp
 * Address scanner using YOUR EXISTING RTCore64Driver
 * NO CUSTOM DRIVER NEEDED - uses what you already have!
 */

#include "BDO_RTCore64_Driver.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

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

// Get process base address from PEB
ULONG64 GetProcessBaseRTCore(RTCore64Driver& driver, DWORD pid) {
    // This is a simplified version - you'd need to:
    // 1. Get process EPROCESS
    // 2. Get PEB from EPROCESS
    // 3. Read ImageBaseAddress from PEB
    
    // For now, return common base for x64 processes
    return 0x140000000;
}

// Pattern scanning using RTCore64
ULONG64 ScanPatternRTCore(RTCore64Driver& driver, ULONG64 start, ULONG64 end, 
                          const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask) {
    const SIZE_T CHUNK_SIZE = 0x10000; // 64KB chunks
    std::vector<BYTE> buffer(CHUNK_SIZE);
    
    for (ULONG64 addr = start; addr < end; addr += CHUNK_SIZE) {
        SIZE_T readSize = (SIZE_T)min(CHUNK_SIZE, end - addr);
        
        // Read chunk
        if (!driver.ReadMemory(addr, buffer.data(), readSize)) {
            continue;
        }
        
        // Search for pattern
        for (SIZE_T i = 0; i <= readSize - pattern.size(); i++) {
            bool match = true;
            
            for (SIZE_T j = 0; j < pattern.size(); j++) {
                if (mask[j] == 0xFF && buffer[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            
            if (match) {
                return addr + i;
            }
        }
    }
    
    return 0;
}

// Parse pattern string "48 8B 0D ?? ?? ?? ??"
bool ParsePattern(const char* patternStr, std::vector<BYTE>& pattern, std::vector<BYTE>& mask) {
    pattern.clear();
    mask.clear();
    
    std::string str(patternStr);
    std::istringstream iss(str);
    std::string token;
    
    while (iss >> token) {
        if (token == "?" || token == "??") {
            pattern.push_back(0x00);
            mask.push_back(0x00);
        } else {
            int value = std::stoi(token, nullptr, 16);
            pattern.push_back((BYTE)value);
            mask.push_back(0xFF);
        }
    }
    
    return !pattern.empty();
}

int main() {
    std::cout << "================================================================================\n";
    std::cout << "BDO Address Scanner - Using RTCore64 (Your Existing Driver)\n";
    std::cout << "================================================================================\n\n";
    
    // Find BDO
    std::cout << "[*] Looking for Black Desert...\n";
    DWORD bdoPid = FindBDO();
    if (bdoPid == 0) {
        std::cerr << "[-] Black Desert not running - start it first!\n";
        return 1;
    }
    std::cout << "[+] Found BDO (PID: " << bdoPid << ")\n\n";
    
    // Connect to RTCore64
    std::cout << "[*] Connecting to RTCore64...\n";
    RTCore64Driver driver;
    if (!driver.Connect()) {
        std::cerr << "[-] Failed to connect to RTCore64\n";
        std::cerr << "[!] Install MSI Afterburner: https://www.msi.com/Landing/afterburner\n";
        return 1;
    }
    std::cout << "[+] Connected to RTCore64\n\n";
    
    // Get base address
    std::cout << "[*] Getting BDO base address...\n";
    ULONG64 baseAddr = GetProcessBaseRTCore(driver, bdoPid);
    std::cout << "[+] Using base: 0x" << std::hex << baseAddr << std::dec << "\n\n";
    
    // Test if we can read
    WORD signature = 0;
    if (!driver.ReadMemory(baseAddr, &signature, sizeof(signature))) {
        std::cerr << "[-] Cannot read BDO memory\n";
        std::cerr << "[!] RTCore64 may not support virtual memory directly\n";
        std::cerr << "[!] You need to use BDO_SecureKernel.sys instead\n";
        driver.Disconnect();
        return 1;
    }
    
    if (signature == 0x5A4D) {
        std::cout << "[+] Memory access working! (PE signature: MZ)\n\n";
    }
    
    // Scan for patterns
    const char* patterns[] = {
        "48 8B 0D ?? ?? ?? ?? 48 85 C9",
        "48 8B 05 ?? ?? ?? ?? 48 85 C0",
        "48 8B 15 ?? ?? ?? ?? 48 85 D2 74",
        "4C 8B 05 ?? ?? ?? ?? 4D 85 C0",
    };
    
    std::cout << "================================================================================\n";
    std::cout << "Scanning for player patterns...\n";
    std::cout << "Range: 256MB from base\n";
    std::cout << "This will take 2-5 minutes...\n";
    std::cout << "================================================================================\n\n";
    
    ULONG64 endAddr = baseAddr + 0x10000000; // 256MB
    
    for (int i = 0; i < 4; i++) {
        std::vector<BYTE> pattern, mask;
        ParsePattern(patterns[i], pattern, mask);
        
        std::cout << "[" << (i+1) << "] Scanning: " << patterns[i] << "\n";
        
        ULONG64 result = ScanPatternRTCore(driver, baseAddr, endAddr, pattern, mask);
        
        if (result != 0) {
            std::cout << "    [FOUND] at 0x" << std::hex << result << std::dec << "\n";
            
            // Save to file
            FILE* f = fopen("BDO_FOUND_ADDRESSES.txt", "a");
            if (f) {
                fprintf(f, "Pattern%d=0x%llX (%s)\n", i+1, result, patterns[i]);
                fclose(f);
            }
        } else {
            std::cout << "    [NOT FOUND]\n";
        }
    }
    
    std::cout << "\nScan complete! Check BDO_FOUND_ADDRESSES.txt\n";
    
    driver.Disconnect();
    return 0;
}


/*
 * EXAMPLE_USAGE.cpp
 * Complete working examples of using BDO Secure Kernel
 * Copy and paste these examples into your own code
 */

#include "BDO_SecureClient.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

//=============================================================================
// EXAMPLE 1: Basic Memory Reading
//=============================================================================

void Example1_BasicRead() {
    std::cout << "\n=== EXAMPLE 1: Basic Memory Reading ===\n";
    
    // Initialize client
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "Error: " << client.GetLastErrorString() << "\n";
        return;
    }
    
    // Get current process for testing
    DWORD pid = GetCurrentProcessId();
    
    // Read a value from our own memory
    int testValue = 12345;
    int readValue = 0;
    
    if (client.Read<int>(pid, (ULONG64)&testValue, readValue)) {
        std::cout << "Success! Read value: " << readValue << "\n";
    } else {
        std::cout << "Failed to read\n";
    }
    
    client.Shutdown();
}

//=============================================================================
// EXAMPLE 2: Reading BDO Memory
//=============================================================================

void Example2_ReadBDO() {
    std::cout << "\n=== EXAMPLE 2: Reading BDO Memory ===\n";
    
    // Find BDO process
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry = {sizeof(entry)};
    DWORD bdoPid = 0;
    
    if (Process32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, L"BlackDesert64.exe") == 0) {
                bdoPid = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    
    if (bdoPid == 0) {
        std::cout << "Black Desert not running\n";
        return;
    }
    
    std::cout << "Found BDO (PID: " << bdoPid << ")\n";
    
    // Initialize client
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "Error: " << client.GetLastErrorString() << "\n";
        return;
    }
    
    // Enable stealth
    client.EnableDriverHiding();
    client.RemoveXigncodeCallbacks();
    
    // Get base address
    ULONG64 baseAddr = 0;
    if (!client.GetProcessBaseAddress(bdoPid, baseAddr)) {
        std::cout << "Failed to get base address\n";
        client.Shutdown();
        return;
    }
    
    std::cout << "Base address: 0x" << std::hex << baseAddr << std::dec << "\n";
    
    // Read PE signature to verify access
    WORD signature = 0;
    if (client.Read<WORD>(bdoPid, baseAddr, signature)) {
        if (signature == 0x5A4D) { // 'MZ'
            std::cout << "Successfully reading BDO memory!\n";
        }
    } else {
        std::cout << "Cannot read BDO memory (Xigncode3 blocking)\n";
    }
    
    client.Shutdown();
}

//=============================================================================
// EXAMPLE 3: Pattern Scanning
//=============================================================================

void Example3_PatternScan() {
    std::cout << "\n=== EXAMPLE 3: Pattern Scanning ===\n";
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "Error: " << client.GetLastErrorString() << "\n";
        return;
    }
    
    DWORD pid = GetCurrentProcessId();
    
    // Create test pattern
    BYTE testData[1024];
    for (int i = 0; i < 1024; i++) {
        testData[i] = (BYTE)i;
    }
    
    // Insert pattern at offset 500
    BYTE pattern[] = { 0x48, 0x8B, 0x0D, 0xAA, 0xBB, 0xCC, 0xDD };
    memcpy(&testData[500], pattern, sizeof(pattern));
    
    // Scan for it
    ULONG64 startAddr = (ULONG64)testData;
    ULONG64 endAddr = startAddr + sizeof(testData);
    ULONG64 result = 0;
    
    std::cout << "Scanning for pattern...\n";
    
    if (client.ScanPattern(pid, startAddr, endAddr, "48 8B 0D ?? ?? ?? ??", result)) {
        std::cout << "Found at: 0x" << std::hex << result << std::dec << "\n";
        std::cout << "Expected: 0x" << std::hex << (startAddr + 500) << std::dec << "\n";
        
        if (result == startAddr + 500) {
            std::cout << "SUCCESS! Pattern found at correct location\n";
        }
    } else {
        std::cout << "Pattern not found\n";
    }
    
    client.Shutdown();
}

//=============================================================================
// EXAMPLE 4: Reading Player Stats (Template for BDO)
//=============================================================================

struct BDOPlayerStats {
    int hp;
    int maxHp;
    int mp;
    int maxMp;
    float posX;
    float posY;
    float posZ;
    long long silver;
    int level;
};

bool ReadPlayerStats(SecureKernelClient& client, DWORD bdoPid, ULONG64 baseAddr, BDOPlayerStats& stats) {
    // NOTE: These offsets are EXAMPLES - find real ones with Cheat Engine!
    
    // Example pointer chain: [[base + 0x5A8B1C8] + 0x18] + 0x30 = player object
    ULONG64 playerMgr = baseAddr + 0x5A8B1C8;
    ULONG64 playerPtr = 0;
    
    // Read first pointer
    if (!client.Read<ULONG64>(bdoPid, playerMgr, playerPtr)) {
        return false;
    }
    
    if (playerPtr == 0) return false;
    
    // Follow pointer
    ULONG64 playerObj = 0;
    if (!client.Read<ULONG64>(bdoPid, playerPtr + 0x18, playerObj)) {
        return false;
    }
    
    if (playerObj == 0) return false;
    
    playerObj += 0x30;
    
    // Read stats (example offsets)
    client.Read<int>(bdoPid, playerObj + 0x28, stats.hp);
    client.Read<int>(bdoPid, playerObj + 0x2C, stats.maxHp);
    client.Read<int>(bdoPid, playerObj + 0x30, stats.mp);
    client.Read<int>(bdoPid, playerObj + 0x34, stats.maxMp);
    client.Read<float>(bdoPid, playerObj + 0x40, stats.posX);
    client.Read<float>(bdoPid, playerObj + 0x44, stats.posY);
    client.Read<float>(bdoPid, playerObj + 0x48, stats.posZ);
    client.Read<long long>(bdoPid, playerObj + 0x70, stats.silver);
    client.Read<int>(bdoPid, playerObj + 0x60, stats.level);
    
    return true;
}

void Example4_PlayerStats() {
    std::cout << "\n=== EXAMPLE 4: Reading Player Stats ===\n";
    std::cout << "NOTE: This example uses placeholder addresses!\n";
    std::cout << "You must find real addresses with Cheat Engine\n\n";
    
    // This is just showing the structure - won't work without real addresses
    std::cout << "See ReadPlayerStats() function for implementation\n";
}

//=============================================================================
// EXAMPLE 5: Auto Health Bot
//=============================================================================

void Example5_HealthBot() {
    std::cout << "\n=== EXAMPLE 5: Auto Health Bot ===\n";
    
    // This is the basic structure - modify with real addresses
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "Failed to initialize\n";
        return;
    }
    
    std::cout << "This is a template. To make it work:\n";
    std::cout << "1. Find BDO process ID\n";
    std::cout << "2. Find health address with Cheat Engine\n";
    std::cout << "3. Update addresses in code\n";
    std::cout << "4. Recompile and run\n";
    std::cout << "\nSee BDO_PracticalBot.cpp for full implementation\n";
    
    client.Shutdown();
}

//=============================================================================
// EXAMPLE 6: Writing Memory (Be Careful!)
//=============================================================================

void Example6_WriteMemory() {
    std::cout << "\n=== EXAMPLE 6: Writing Memory ===\n";
    std::cout << "WARNING: Writing to game memory can cause crashes!\n\n";
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "Failed to initialize\n";
        return;
    }
    
    // Safe test: write to our own process
    DWORD pid = GetCurrentProcessId();
    int testValue = 100;
    
    std::cout << "Original value: " << testValue << "\n";
    
    int newValue = 999;
    if (client.Write<int>(pid, (ULONG64)&testValue, newValue)) {
        std::cout << "Write successful\n";
        std::cout << "New value: " << testValue << "\n";
    } else {
        std::cout << "Write failed\n";
    }
    
    client.Shutdown();
}

//=============================================================================
// EXAMPLE 7: Finding Player Base with Pattern
//=============================================================================

void Example7_FindPlayerBase() {
    std::cout << "\n=== EXAMPLE 7: Finding Player Base ===\n";
    
    // This shows HOW to find dynamic addresses
    // Actual patterns change with each BDO update
    
    std::cout << "Pattern for player pointer (example):\n";
    std::cout << "  48 8B 0D ?? ?? ?? ??  (MOV RCX, [RIP+offset])\n";
    std::cout << "  48 85 C9              (TEST RCX, RCX)\n\n";
    
    std::cout << "Steps:\n";
    std::cout << "1. Use IDA/Ghidra to find this pattern in BlackDesert64.exe\n";
    std::cout << "2. Calculate: baseAddr + offset from RIP\n";
    std::cout << "3. That address contains the player pointer\n";
    std::cout << "4. Read the pointer to get player object\n\n";
    
    std::cout << "Or use Cheat Engine's pointer scanner\n";
}

//=============================================================================
// Main Menu
//=============================================================================

int main() {
    std::cout << "================================================================================\n";
    std::cout << "BDO Secure Kernel - Usage Examples\n";
    std::cout << "================================================================================\n";
    std::cout << "\nSelect example:\n";
    std::cout << "1. Basic memory reading\n";
    std::cout << "2. Reading BDO memory\n";
    std::cout << "3. Pattern scanning\n";
    std::cout << "4. Reading player stats (template)\n";
    std::cout << "5. Auto health bot (template)\n";
    std::cout << "6. Writing memory\n";
    std::cout << "7. Finding player base\n";
    std::cout << "0. Exit\n";
    std::cout << "\nChoice: ";
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
        case 1: Example1_BasicRead(); break;
        case 2: Example2_ReadBDO(); break;
        case 3: Example3_PatternScan(); break;
        case 4: Example4_PlayerStats(); break;
        case 5: Example5_HealthBot(); break;
        case 6: Example6_WriteMemory(); break;
        case 7: Example7_FindPlayerBase(); break;
        case 0: return 0;
        default: std::cout << "Invalid choice\n";
    }
    
    std::cout << "\nPress Enter to exit...\n";
    std::cin.ignore();
    std::cin.get();
    
    return 0;
}


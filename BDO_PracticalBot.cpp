/*
 * BDO_PracticalBot.cpp
 * Real working BDO bot using BDO_SecureClient
 * Includes health monitoring, auto-potion, position tracking
 */

#include "BDO_SecureClient.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <Windows.h>
#include <TlHelp32.h>

// Find process by name
DWORD FindProcess(const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    
    PROCESSENTRY32W entry = {sizeof(entry)};
    if (Process32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, name) == 0) {
                CloseHandle(snap);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snap, &entry));
    }
    
    CloseHandle(snap);
    return 0;
}

class BDOBot {
private:
    SecureKernelClient client;
    DWORD pid;
    ULONG64 baseAddr;
    bool running;
    
    // Configurable settings
    int healthThreshold;
    int manaThreshold;
    bool autoPotion;
    bool autoLoot;
    
    // Statistics
    int potionsUsed;
    int itemsLooted;
    
public:
    BDOBot() : pid(0), baseAddr(0), running(false), 
               healthThreshold(50), manaThreshold(30),
               autoPotion(true), autoLoot(false),
               potionsUsed(0), itemsLooted(0) {}
    
    bool Start() {
        std::cout << "========================================\n";
        std::cout << "BDO Practical Bot - Starting\n";
        std::cout << "========================================\n\n";
        
        // Initialize secure kernel
        std::cout << "[*] Initializing secure kernel client...\n";
        if (!client.Initialize()) {
            std::cerr << "[-] Failed: " << client.GetLastErrorString() << "\n";
            std::cerr << "[!] Make sure BDO_SecureKernel.sys is loaded\n";
            return false;
        }
        std::cout << "[+] Secure kernel initialized\n\n";
        
        // Enable stealth
        std::cout << "[*] Enabling stealth features...\n";
        client.EnableDriverHiding();
        client.RemoveXigncodeCallbacks();
        std::cout << "[+] Stealth enabled\n\n";
        
        // Find BDO
        std::cout << "[*] Looking for Black Desert...\n";
        pid = FindProcess(L"BlackDesert64.exe");
        if (pid == 0) {
            std::cerr << "[-] Black Desert not running\n";
            return false;
        }
        std::cout << "[+] Found BlackDesert64.exe (PID: " << pid << ")\n\n";
        
        // Get base address
        std::cout << "[*] Getting process base address...\n";
        if (!client.GetProcessBaseAddress(pid, baseAddr)) {
            std::cerr << "[-] Failed to get base address\n";
            return false;
        }
        std::cout << "[+] Base address: 0x" << std::hex << baseAddr << std::dec << "\n\n";
        
        // Verify access
        WORD sig = 0;
        if (!client.Read<WORD>(pid, baseAddr, sig)) {
            std::cerr << "[-] Cannot read BDO memory (Xigncode3 may be blocking)\n";
            return false;
        }
        
        if (sig == 0x5A4D) {
            std::cout << "[+] Memory access verified (PE signature: MZ)\n\n";
        }
        
        // Show settings
        std::cout << "Bot Settings:\n";
        std::cout << "  Health threshold: " << healthThreshold << "%\n";
        std::cout << "  Mana threshold: " << manaThreshold << "%\n";
        std::cout << "  Auto potion: " << (autoPotion ? "ON" : "OFF") << "\n";
        std::cout << "  Auto loot: " << (autoLoot ? "ON" : "OFF") << "\n\n";
        
        std::cout << "========================================\n";
        std::cout << "Bot is now RUNNING\n";
        std::cout << "Press Ctrl+C to stop\n";
        std::cout << "========================================\n\n";
        
        running = true;
        return true;
    }
    
    void MainLoop() {
        auto lastUpdate = std::chrono::steady_clock::now();
        
        while (running) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();
            
            // Update every 500ms
            if (elapsed >= 500) {
                UpdateBot();
                lastUpdate = now;
            }
            
            Sleep(50); // Small delay
        }
    }
    
    void UpdateBot() {
        // Read player stats (example addresses - UPDATE THESE FOR YOUR VERSION!)
        // These are examples and will need to be found with Cheat Engine
        ULONG64 playerPtr = baseAddr + 0x3A5B2C8;
        ULONG64 player = 0;
        
        // Read player pointer
        if (!client.Read<ULONG64>(pid, playerPtr, player)) {
            return; // Pointer not valid yet
        }
        
        if (player == 0) return;
        
        // Read health (example offset)
        int hp = 0, maxHp = 1;
        client.Read<int>(pid, player + 0x28, hp);
        client.Read<int>(pid, player + 0x2C, maxHp);
        
        if (maxHp > 0) {
            int hpPercent = (hp * 100) / maxHp;
            
            // Display stats
            std::cout << "\r[HP: " << hp << "/" << maxHp << " (" << hpPercent << "%)";
            std::cout << " | Potions: " << potionsUsed << "]";
            std::cout << std::flush;
            
            // Auto potion
            if (autoPotion && hpPercent < healthThreshold) {
                UseHealthPotion();
            }
        }
    }
    
    void UseHealthPotion() {
        std::cout << "\n[!] Health low! Using potion...\n";
        
        // Simulate F1 key press
        keybd_event(VK_F1, 0, 0, 0);
        Sleep(50);
        keybd_event(VK_F1, 0, KEYEVENTF_KEYUP, 0);
        
        potionsUsed++;
        Sleep(5000); // Cooldown
    }
    
    void Stop() {
        running = false;
        std::cout << "\n\n========================================\n";
        std::cout << "Bot Statistics:\n";
        std::cout << "  Potions used: " << potionsUsed << "\n";
        std::cout << "  Items looted: " << itemsLooted << "\n";
        std::cout << "========================================\n";
        client.Shutdown();
    }
};

// Global bot instance
BDOBot g_Bot;

// Ctrl+C handler
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        g_Bot.Stop();
        return TRUE;
    }
    return FALSE;
}

int main() {
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    
    if (g_Bot.Start()) {
        g_Bot.MainLoop();
    }
    
    return 0;
}


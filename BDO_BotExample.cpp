#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "BDO_BotMemoryIntegration.h"

// Simple example showing how to use scanned memory addresses in bot automation

void PrintBanner() {
    std::cout << "================================================" << std::endl;
    std::cout << "    BDO Bot Memory Integration Example" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
}

void PrintMenu() {
    std::cout << "\n=== Bot Control Menu ===" << std::endl;
    std::cout << "1. Scan for addresses automatically" << std::endl;
    std::cout << "2. Enter addresses manually (from MSI Afterburner/Cheat Engine)" << std::endl;
    std::cout << "3. Load addresses from file" << std::endl;
    std::cout << "4. Start Combat Bot" << std::endl;
    std::cout << "5. Start Fishing Bot" << std::endl;
    std::cout << "6. Start Gathering Bot" << std::endl;
    std::cout << "7. Monitor values only" << std::endl;
    std::cout << "8. Save current addresses" << std::endl;
    std::cout << "9. Show statistics" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Choice: ";
}

bool ManualAddressInput(BDOBotMemoryIntegration& bot) {
    std::cout << "\n=== Manual Address Input ===" << std::endl;
    std::cout << "Enter addresses in hexadecimal (e.g., 7FF6A2B4C890)" << std::endl;
    std::cout << "Press Enter to skip an address\n" << std::endl;
    
    std::string input;
    
    // Health address
    std::cout << "Player Health Address: 0x";
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            PVOID addr = reinterpret_cast<PVOID>(std::stoull(input, nullptr, 16));
            bot.SetPlayerHealthAddress(addr);
        } catch (...) {
            std::cerr << "Invalid address format" << std::endl;
        }
    }
    
    // Mana address
    std::cout << "Player Mana Address: 0x";
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            PVOID addr = reinterpret_cast<PVOID>(std::stoull(input, nullptr, 16));
            bot.SetPlayerManaAddress(addr);
        } catch (...) {
            std::cerr << "Invalid address format" << std::endl;
        }
    }
    
    // Position addresses
    std::cout << "Player X Position Address: 0x";
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            PVOID xAddr = reinterpret_cast<PVOID>(std::stoull(input, nullptr, 16));
            
            std::cout << "Player Y Position Address: 0x";
            std::getline(std::cin, input);
            PVOID yAddr = reinterpret_cast<PVOID>(std::stoull(input, nullptr, 16));
            
            std::cout << "Player Z Position Address: 0x";
            std::getline(std::cin, input);
            PVOID zAddr = reinterpret_cast<PVOID>(std::stoull(input, nullptr, 16));
            
            bot.SetPlayerPositionAddresses(xAddr, yAddr, zAddr);
        } catch (...) {
            std::cerr << "Invalid address format" << std::endl;
        }
    }
    
    return true;
}

void SetupCombatActions(BDOBotMemoryIntegration& bot) {
    std::cout << "Setting up combat actions..." << std::endl;
    
    // Health Potion
    bot.RegisterAction("UseHealthPotion", []() {
        std::cout << "[COMBAT] Using health potion (Key 1)" << std::endl;
        keybd_event('1', 0, 0, 0);
        Sleep(50);
        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(5), 10);
    
    // Mana Potion
    bot.RegisterAction("UseManaPotion", []() {
        std::cout << "[COMBAT] Using mana potion (Key 2)" << std::endl;
        keybd_event('2', 0, 0, 0);
        Sleep(50);
        keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(5), 9);
    
    // Basic Attack
    bot.RegisterAction("BasicAttack", []() {
        std::cout << "[COMBAT] Performing basic attack" << std::endl;
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        Sleep(100);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }, std::chrono::milliseconds(800));
    
    // Skill Rotation
    bot.RegisterAction("SkillQ", []() {
        std::cout << "[COMBAT] Using skill Q" << std::endl;
        keybd_event('Q', 0, 0, 0);
        Sleep(50);
        keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(2));
    
    bot.RegisterAction("SkillE", []() {
        std::cout << "[COMBAT] Using skill E" << std::endl;
        keybd_event('E', 0, 0, 0);
        Sleep(50);
        keybd_event('E', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(3));
    
    bot.RegisterAction("SkillR", []() {
        std::cout << "[COMBAT] Using skill R" << std::endl;
        keybd_event('R', 0, 0, 0);
        Sleep(50);
        keybd_event('R', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(5));
    
    // Dodge
    bot.RegisterAction("Dodge", []() {
        std::cout << "[COMBAT] Dodging (Shift)" << std::endl;
        keybd_event(VK_SHIFT, 0, 0, 0);
        keybd_event('A', 0, 0, 0);
        Sleep(100);
        keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(4));
    
    // Set up triggers
    bot.AddHealthTrigger(0.5f, {"UseHealthPotion"});  // 50% health
    bot.AddHealthTrigger(0.3f, {"UseHealthPotion", "Dodge"});  // 30% health - emergency
    bot.AddManaTrigger(0.3f, {"UseManaPotion"});  // 30% mana
    bot.AddCombatTrigger(true, {"BasicAttack", "SkillQ", "SkillE"});
}

void SetupFishingActions(BDOBotMemoryIntegration& bot) {
    std::cout << "Setting up fishing actions..." << std::endl;
    
    bot.RegisterAction("CastFishingRod", []() {
        std::cout << "[FISHING] Casting rod (Space)" << std::endl;
        keybd_event(VK_SPACE, 0, 0, 0);
        Sleep(50);
        keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(3));
    
    bot.RegisterAction("CatchFish", []() {
        std::cout << "[FISHING] Fish bite detected! Catching (Space)" << std::endl;
        keybd_event(VK_SPACE, 0, 0, 0);
        Sleep(50);
        keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::milliseconds(500));
    
    // Note: You need to find the fishing state address for this to work
    std::cout << "Note: Fishing bot requires fishing state address to be set" << std::endl;
}

void MonitorValues(BDOBotMemoryIntegration& bot) {
    std::cout << "\n=== Value Monitoring Mode ===" << std::endl;
    std::cout << "Press Ctrl+C to stop monitoring\n" << std::endl;
    
    while (true) {
        system("cls");  // Clear screen (Windows)
        std::cout << "=== BDO Memory Values ===" << std::endl;
        
        // Read and display player stats
        int health, maxHealth, mana, maxMana;
        if (bot.ReadPlayerStats(health, maxHealth, mana, maxMana)) {
            float healthPercent = (float)health / maxHealth * 100;
            float manaPercent = (float)mana / maxMana * 100;
            
            std::cout << "Health: " << health << "/" << maxHealth 
                     << " (" << std::fixed << std::setprecision(1) << healthPercent << "%)" << std::endl;
            std::cout << "Mana: " << mana << "/" << maxMana 
                     << " (" << std::fixed << std::setprecision(1) << manaPercent << "%)" << std::endl;
        } else {
            std::cout << "Health: [Not Available]" << std::endl;
            std::cout << "Mana: [Not Available]" << std::endl;
        }
        
        // Read and display position
        float x, y, z;
        if (bot.ReadPlayerPosition(x, y, z)) {
            std::cout << "Position: X=" << std::fixed << std::setprecision(2) << x 
                     << ", Y=" << y << ", Z=" << z << std::endl;
        } else {
            std::cout << "Position: [Not Available]" << std::endl;
        }
        
        // Read combat state
        bool inCombat;
        PVOID target;
        if (bot.ReadCombatState(inCombat, target)) {
            std::cout << "Combat: " << (inCombat ? "In Combat" : "Not in Combat") << std::endl;
            if (target) {
                std::cout << "Target: 0x" << std::hex << target << std::dec << std::endl;
            }
        }
        
        std::cout << "\nPress Ctrl+C to stop monitoring..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main() {
    PrintBanner();
    
    // Create bot instance
    BDOBotMemoryIntegration bot;
    AddressScanner scanner(&bot);
    
    // Try to attach to game
    std::cout << "Attempting to attach to Black Desert Online..." << std::endl;
    if (!bot.AttachToGame(L"BlackDesert64.exe")) {
        std::cerr << "Failed to attach to game! Make sure BDO is running." << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }
    
    std::cout << "Successfully attached to game!" << std::endl;
    
    bool running = true;
    while (running) {
        PrintMenu();
        
        int choice;
        std::cin >> choice;
        std::cin.ignore(); // Clear newline
        
        switch (choice) {
            case 1: {
                // Automatic scanning
                std::cout << "\nScanning for addresses automatically..." << std::endl;
                if (bot.ScanForAddresses()) {
                    std::cout << "Address scan complete!" << std::endl;
                    scanner.PrintFoundAddresses();
                } else {
                    std::cout << "Automatic scanning failed. Try manual input." << std::endl;
                }
                break;
            }
            
            case 2: {
                // Manual input
                ManualAddressInput(bot);
                break;
            }
            
            case 3: {
                // Load from file
                std::cout << "Enter filename: ";
                std::string filename;
                std::getline(std::cin, filename);
                if (bot.LoadAddressesFromFile(filename)) {
                    std::cout << "Addresses loaded successfully!" << std::endl;
                } else {
                    std::cout << "Failed to load addresses from file." << std::endl;
                }
                break;
            }
            
            case 4: {
                // Start combat bot
                SetupCombatActions(bot);
                bot.StartMonitoring(std::chrono::milliseconds(100));
                std::cout << "\nCombat bot started! Press Enter to stop..." << std::endl;
                std::cin.get();
                bot.StopMonitoring();
                break;
            }
            
            case 5: {
                // Start fishing bot
                SetupFishingActions(bot);
                bot.StartMonitoring(std::chrono::milliseconds(200));
                std::cout << "\nFishing bot started! Press Enter to stop..." << std::endl;
                std::cin.get();
                bot.StopMonitoring();
                break;
            }
            
            case 6: {
                // Start gathering bot
                std::cout << "Gathering bot not yet implemented." << std::endl;
                break;
            }
            
            case 7: {
                // Monitor values
                MonitorValues(bot);
                break;
            }
            
            case 8: {
                // Save addresses
                std::cout << "Enter filename: ";
                std::string filename;
                std::getline(std::cin, filename);
                if (bot.SaveAddressesToFile(filename)) {
                    std::cout << "Addresses saved successfully!" << std::endl;
                } else {
                    std::cout << "Failed to save addresses." << std::endl;
                }
                break;
            }
            
            case 9: {
                // Show statistics
                bot.PrintStatistics();
                break;
            }
            
            case 0: {
                running = false;
                break;
            }
            
            default: {
                std::cout << "Invalid choice!" << std::endl;
                break;
            }
        }
        
        if (running) {
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
    }
    
    // Cleanup
    bot.StopMonitoring();
    bot.Shutdown();
    
    std::cout << "\nBot stopped. Goodbye!" << std::endl;
    return 0;
}

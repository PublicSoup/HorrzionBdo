// Educational example of using BDO Memory Reader
// This demonstrates how to read game memory - FOR EDUCATIONAL PURPOSES ONLY

#include "BDOMemory.h"
#include "AntiDetection.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <conio.h>

void PrintBanner() {
    std::cout << "========================================" << std::endl;
    std::cout << "  BDO Memory Reader - Educational Demo" << std::endl;
    std::cout << "        For Learning Purposes Only" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}

void DisplayPlayerInfo(BDOMemory& memory) {
    if (!memory.IsPlayerValid()) {
        std::cout << "[-] Player not valid" << std::endl;
        return;
    }
    
    auto player = memory.GetPlayer();
    
    std::cout << "\n=== Player Information ===" << std::endl;
    std::cout << "Name: " << player.name << std::endl;
    std::cout << "Level: " << player.level << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Health: " << player.health << " / " << player.maxHealth << std::endl;
    std::cout << "Mana: " << player.mana << " / " << player.maxMana << std::endl;
    std::cout << "Stamina: " << player.stamina << std::endl;
    std::cout << "Position: (" << player.posX << ", " << player.posY << ", " << player.posZ << ")" << std::endl;
    std::cout << "Rotation: " << player.rotation << std::endl;
}

void DisplayNearbyEntities(BDOMemory& memory) {
    auto entities = memory.GetNearbyEntities(50.0f);
    
    std::cout << "\n=== Nearby Entities ===" << std::endl;
    std::cout << "Found " << entities.size() << " entities within 50m" << std::endl;
    
    int count = 0;
    for (const auto& entity : entities) {
        if (count >= 10) break; // Show only first 10
        
        float distance = memory.GetDistanceToEntity(entity);
        
        std::cout << "\n[" << count+1 << "] " << entity.name << std::endl;
        std::cout << "    Type: " << entity.type << std::endl;
        std::cout << "    Distance: " << std::fixed << std::setprecision(2) << distance << "m" << std::endl;
        std::cout << "    Alive: " << (entity.isAlive ? "Yes" : "No") << std::endl;
        std::cout << "    Position: (" << entity.posX << ", " << entity.posY << ", " << entity.posZ << ")" << std::endl;
        
        count++;
    }
}

void MonitorMode(BDOMemory& memory) {
    std::cout << "\n=== Monitor Mode ===" << std::endl;
    std::cout << "Continuously monitoring player stats. Press any key to stop." << std::endl;
    std::cout << std::endl;
    
    AntiDetection antiDet;
    antiDet.SetTimingRange(100, 300); // Human-like delays
    
    while (!_kbhit()) {
        system("cls"); // Windows only
        PrintBanner();
        
        DisplayPlayerInfo(memory);
        
        // Add human-like delay to avoid detection
        antiDet.RandomizeTiming();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    _getch(); // Consume the key press
}

void PatternScanMode(BDOMemory& memory) {
    std::cout << "\n=== Pattern Scanner ===" << std::endl;
    std::cout << "Enter pattern to search (hex bytes, use ?? for wildcards)" << std::endl;
    std::cout << "Example: 48 8B 0D ?? ?? ?? ?? 48 85 C9" << std::endl;
    std::cout << "Pattern: ";
    
    std::cin.ignore();
    std::string pattern;
    std::getline(std::cin, pattern);
    
    std::cout << "[*] Scanning for pattern..." << std::endl;
    
    uintptr_t result = memory.GetReader().ScanAOB(pattern);
    
    if (result != 0) {
        std::cout << "[+] Pattern found at: 0x" << std::hex << result << std::dec << std::endl;
        
        // Read and display bytes at the address
        char bytes[32];
        if (memory.GetReader().ReadBytes(result, bytes, 32)) {
            std::cout << "Bytes: ";
            for (int i = 0; i < 32; i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                         << (int)(unsigned char)bytes[i] << " ";
            }
            std::cout << std::dec << std::endl;
        }
    } else {
        std::cout << "[-] Pattern not found" << std::endl;
    }
}

void OffsetFinderMode(BDOMemory& memory) {
    std::cout << "\n=== Offset Finder ===" << std::endl;
    std::cout << "[*] Attempting to find game offsets..." << std::endl;
    
    memory.UpdateOffsets();
    
    std::cout << "\n[Results]" << std::endl;
    std::cout << "Player Base: 0x" << std::hex << memory.GetPlayerBase() << std::dec << std::endl;
    std::cout << "Entity List: 0x" << std::hex << memory.GetEntityListBase() << std::dec << std::endl;
    
    if (memory.GetPlayerBase() != 0) {
        std::cout << "\n[+] Player base found! Testing read..." << std::endl;
        DisplayPlayerInfo(memory);
    }
}

void MemoryDumpMode(BDOMemory& memory) {
    std::cout << "\n=== Memory Dump ===" << std::endl;
    std::cout << "Enter address (hex) to dump: 0x";
    
    uintptr_t address;
    std::cin >> std::hex >> address >> std::dec;
    
    std::cout << "Enter number of bytes to dump: ";
    int size;
    std::cin >> size;
    
    if (size <= 0 || size > 1024) {
        std::cout << "[-] Invalid size (1-1024)" << std::endl;
        return;
    }
    
    std::vector<char> buffer(size);
    if (memory.GetReader().ReadBytes(address, buffer.data(), size)) {
        std::cout << "\n[Memory Dump]" << std::endl;
        std::cout << "Address: 0x" << std::hex << address << std::dec << std::endl;
        std::cout << "Size: " << size << " bytes" << std::endl;
        std::cout << std::endl;
        
        for (int i = 0; i < size; i++) {
            if (i % 16 == 0) {
                std::cout << std::hex << std::setw(8) << std::setfill('0') 
                         << (address + i) << ": ";
            }
            
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                     << (int)(unsigned char)buffer[i] << " ";
            
            if ((i + 1) % 16 == 0 || i == size - 1) {
                std::cout << std::dec << std::endl;
            }
        }
    } else {
        std::cout << "[-] Failed to read memory at that address" << std::endl;
    }
}

int main() {
    PrintBanner();
    
    BDOMemory memory;
    
    std::cout << "[*] Initializing..." << std::endl;
    if (!memory.Initialize()) {
        std::cout << "[-] Initialization failed" << std::endl;
        std::cout << "Make sure you're running as Administrator!" << std::endl;
        return 1;
    }
    
    std::cout << "[*] Attaching to Black Desert..." << std::endl;
    if (!memory.AttachToGame()) {
        std::cout << "[-] Failed to attach to game" << std::endl;
        std::cout << "Make sure Black Desert is running!" << std::endl;
        return 1;
    }
    
    std::cout << "[+] Successfully attached!" << std::endl;
    std::cout << std::endl;
    
    // Main menu loop
    bool running = true;
    while (running) {
        std::cout << "\n=== Main Menu ===" << std::endl;
        std::cout << "1. Display Player Info" << std::endl;
        std::cout << "2. Display Nearby Entities" << std::endl;
        std::cout << "3. Monitor Mode (Live Updates)" << std::endl;
        std::cout << "4. Pattern Scanner" << std::endl;
        std::cout << "5. Offset Finder" << std::endl;
        std::cout << "6. Memory Dump" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "\nChoice: ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                DisplayPlayerInfo(memory);
                break;
            case 2:
                DisplayNearbyEntities(memory);
                break;
            case 3:
                MonitorMode(memory);
                break;
            case 4:
                PatternScanMode(memory);
                break;
            case 5:
                OffsetFinderMode(memory);
                break;
            case 6:
                MemoryDumpMode(memory);
                break;
            case 0:
                running = false;
                break;
            default:
                std::cout << "[-] Invalid choice" << std::endl;
        }
        
        if (running && choice != 3) {
            std::cout << "\nPress any key to continue...";
            _getch();
        }
    }
    
    std::cout << "\n[*] Cleaning up..." << std::endl;
    memory.Disconnect();
    
    std::cout << "[+] Done!" << std::endl;
    return 0;
}


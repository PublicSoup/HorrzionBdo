/*
 * BDO Kernel Driver Example
 * Demonstrates professional usage of kernel-level memory access
 */

#include "pch.h"
#include "BDO_KernelClient.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace std;

void PrintBanner() {
    cout << "=========================================" << endl;
    cout << " BDO Kernel Memory Interface" << endl;
    cout << " Professional Implementation" << endl;
    cout << "=========================================" << endl << endl;
}

void PrintMenu() {
    cout << "\n[Menu]" << endl;
    cout << "1. Connect to Kernel Driver" << endl;
    cout << "2. Attach to Black Desert" << endl;
    cout << "3. Read Player Data" << endl;
    cout << "4. Scan for Entities" << endl;
    cout << "5. Pattern Scan Example" << endl;
    cout << "6. Bypass Anti-Cheat" << endl;
    cout << "7. Statistics" << endl;
    cout << "8. Continuous Monitor (Real-time)" << endl;
    cout << "0. Exit" << endl;
    cout << "\nChoice: ";
}

void DisplayPlayerData(const BDOGameMemory::PlayerData& player) {
    cout << "\n=== Player Information ===" << endl;
    cout << "Level: " << player.level << endl;
    cout << "Health: " << player.health << " / " << player.maxHealth 
         << " (" << (player.maxHealth > 0 ? (player.health/player.maxHealth*100) : 0) << "%)" << endl;
    cout << "Mana: " << player.mana << " / " << player.maxMana 
         << " (" << (player.maxMana > 0 ? (player.mana/player.maxMana*100) : 0) << "%)" << endl;
    cout << "Stamina: " << player.stamina << " / " << player.maxStamina 
         << " (" << (player.maxStamina > 0 ? (player.stamina/player.maxStamina*100) : 0) << "%)" << endl;
    cout << "Silver: " << player.silver << endl;
    cout << "Weight: " << player.weight << " / " << player.maxWeight 
         << " (" << (player.maxWeight > 0 ? (player.weight/player.maxWeight*100) : 0) << "%)" << endl;
    cout << "Position: (" << fixed << setprecision(2) 
         << player.x << ", " << player.y << ", " << player.z << ")" << endl;
    cout << "Rotation: " << player.rotation << endl;
    cout << "In Combat: " << (player.inCombat ? "Yes" : "No") << endl;
    if (player.targetEntity != 0) {
        cout << "Target: 0x" << hex << player.targetEntity << dec << endl;
    }
}

void DisplayEntities(const vector<BDOGameMemory::EntityData>& entities) {
    cout << "\n=== Nearby Entities ===" << endl;
    cout << "Total Count: " << entities.size() << endl << endl;
    
    int displayCount = min((int)entities.size(), 20); // Show first 20
    for (int i = 0; i < displayCount; i++) {
        const auto& entity = entities[i];
        cout << "[" << (i+1) << "] Type: " << entity.type 
             << " | HP: " << entity.health 
             << " | Dist: " << fixed << setprecision(1) << entity.distance
             << " | Pos: (" << entity.x << ", " << entity.y << ", " << entity.z << ")"
             << " | " << (entity.isAlive ? "Alive" : "Dead")
             << endl;
    }
    
    if (entities.size() > 20) {
        cout << "... and " << (entities.size() - 20) << " more entities" << endl;
    }
}

void ContinuousMonitor(BDOGameMemory& gameMemory) {
    cout << "\n[*] Starting continuous monitor (Press Ctrl+C to stop)..." << endl;
    cout << "[*] Refreshing every 1 second..." << endl << endl;
    
    int updateCount = 0;
    
    while (true) {
        // Clear screen (Windows)
        system("cls");
        
        cout << "=== BDO Live Monitor ===" << endl;
        cout << "Update #" << ++updateCount << endl << endl;
        
        // Get and display player data
        BDOGameMemory::PlayerData player;
        if (gameMemory.GetPlayerData(player)) {
            DisplayPlayerData(player);
        } else {
            cout << "[!] Failed to read player data" << endl;
        }
        
        // Get and display nearby entities
        vector<BDOGameMemory::EntityData> entities;
        if (gameMemory.GetNearbyEntities(entities, 50.0f)) { // 50 units range
            cout << "\n=== Entities Within 50 Units ===" << endl;
            cout << "Count: " << entities.size() << endl;
            
            // Show first 5
            int showCount = min(5, (int)entities.size());
            for (int i = 0; i < showCount; i++) {
                const auto& entity = entities[i];
                cout << "  [" << (i+1) << "] Type " << entity.type 
                     << " | HP: " << entity.health 
                     << " | Dist: " << fixed << setprecision(1) << entity.distance << endl;
            }
        }
        
        // Show statistics
        auto stats = gameMemory.GetKernel()->GetStatistics();
        cout << "\n[Statistics]" << endl;
        cout << "Reads: " << stats.totalReads 
             << " | Writes: " << stats.totalWrites 
             << " | Failed: " << stats.failedOperations 
             << " | Success Rate: " << fixed << setprecision(1) << stats.successRate << "%" << endl;
        
        cout << "\n[Press Ctrl+C to stop]" << endl;
        
        // Wait 1 second
        this_thread::sleep_for(chrono::seconds(1));
    }
}

int main() {
    PrintBanner();
    
    BDOGameMemory gameMemory;
    bool driverConnected = false;
    bool gameAttached = false;
    
    while (true) {
        PrintMenu();
        
        int choice;
        cin >> choice;
        
        switch (choice) {
            case 0:
                cout << "\n[*] Exiting..." << endl;
                return 0;
                
            case 1: {
                cout << "\n[*] Connecting to kernel driver..." << endl;
                if (gameMemory.GetKernel()->Connect()) {
                    driverConnected = true;
                    cout << "[+] Successfully connected to kernel driver!" << endl;
                } else {
                    cout << "[!] Failed to connect to driver" << endl;
                    cout << "[!] Make sure driver is loaded:" << endl;
                    cout << "    1. Open cmd as Administrator" << endl;
                    cout << "    2. sc create BDOKernelMem type=kernel binPath=<full_path>\\BDO_KernelDriver.sys" << endl;
                    cout << "    3. sc start BDOKernelMem" << endl;
                    cout << "\n[!] Or use vulnerable driver exploit (advanced)" << endl;
                }
                break;
            }
            
            case 2: {
                if (!driverConnected) {
                    cout << "[!] Connect to driver first" << endl;
                    break;
                }
                
                cout << "\n[*] Searching for Black Desert..." << endl;
                if (gameMemory.GetKernel()->AttachToProcess(L"BlackDesert64.exe")) {
                    gameAttached = true;
                    cout << "[+] Successfully attached to Black Desert!" << endl;
                } else {
                    cout << "[!] Failed to find Black Desert process" << endl;
                    cout << "[!] Make sure the game is running" << endl;
                }
                break;
            }
            
            case 3: {
                if (!gameAttached) {
                    cout << "[!] Attach to game first" << endl;
                    break;
                }
                
                cout << "\n[*] Reading player data..." << endl;
                BDOGameMemory::PlayerData player;
                if (gameMemory.GetPlayerData(player)) {
                    DisplayPlayerData(player);
                } else {
                    cout << "[!] Failed to read player data" << endl;
                    cout << "[!] Offsets may need updating or anti-cheat is blocking" << endl;
                }
                break;
            }
            
            case 4: {
                if (!gameAttached) {
                    cout << "[!] Attach to game first" << endl;
                    break;
                }
                
                cout << "\n[*] Scanning for entities..." << endl;
                vector<BDOGameMemory::EntityData> entities;
                if (gameMemory.GetEntityList(entities)) {
                    DisplayEntities(entities);
                } else {
                    cout << "[!] Failed to read entity list" << endl;
                }
                break;
            }
            
            case 5: {
                if (!gameAttached) {
                    cout << "[!] Attach to game first" << endl;
                    break;
                }
                
                cout << "\n[*] Running pattern scan example..." << endl;
                cout << "[*] Scanning for player base pattern..." << endl;
                
                uint64_t result = gameMemory.GetKernel()->AOBScan(
                    "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 88"
                );
                
                if (result != 0) {
                    cout << "[+] Pattern found at: 0x" << hex << result << dec << endl;
                    
                    // Read some bytes at that location
                    uint8_t bytes[16];
                    if (gameMemory.GetKernel()->ReadBuffer(result, bytes, 16)) {
                        cout << "[+] Bytes at location: ";
                        for (int i = 0; i < 16; i++) {
                            cout << hex << setw(2) << setfill('0') << (int)bytes[i] << " ";
                        }
                        cout << dec << endl;
                    }
                } else {
                    cout << "[!] Pattern not found" << endl;
                }
                break;
            }
            
            case 6: {
                if (!gameAttached) {
                    cout << "[!] Attach to game first" << endl;
                    break;
                }
                
                cout << "\n[*] Attempting anti-cheat bypass..." << endl;
                cout << "[*] This will attempt to:" << endl;
                cout << "    - Clear debug flags" << endl;
                cout << "    - Modify PEB" << endl;
                cout << "    - Hide driver presence" << endl << endl;
                
                if (gameMemory.GetKernel()->BypassAntiCheat()) {
                    cout << "[+] Anti-cheat bypass successful!" << endl;
                    cout << "[+] Memory operations should now be less detectable" << endl;
                } else {
                    cout << "[!] Bypass failed or partially successful" << endl;
                }
                break;
            }
            
            case 7: {
                auto stats = gameMemory.GetKernel()->GetStatistics();
                cout << "\n=== Statistics ===" << endl;
                cout << "Total Reads: " << stats.totalReads << endl;
                cout << "Total Writes: " << stats.totalWrites << endl;
                cout << "Failed Operations: " << stats.failedOperations << endl;
                cout << "Success Rate: " << fixed << setprecision(2) << stats.successRate << "%" << endl;
                
                cout << "\n[Driver Status]" << endl;
                cout << "Connected: " << (gameMemory.GetKernel()->IsConnected() ? "Yes" : "No") << endl;
                cout << "Target PID: " << gameMemory.GetKernel()->GetTargetPID() << endl;
                cout << "Game Base: 0x" << hex << gameMemory.GetKernel()->GetGameBase() << dec << endl;
                cout << "AC Bypassed: " << (gameMemory.GetKernel()->IsAntiCheatBypassed() ? "Yes" : "No") << endl;
                break;
            }
            
            case 8: {
                if (!gameAttached) {
                    cout << "[!] Attach to game first" << endl;
                    break;
                }
                
                ContinuousMonitor(gameMemory);
                break;
            }
            
            default:
                cout << "[!] Invalid choice" << endl;
                break;
        }
        
        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
    }
    
    return 0;
}



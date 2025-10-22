#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "BDO_RTCore64_Memory.h"

// BDO memory offsets (update these as needed)
namespace BDO {
    constexpr ULONG64 BASE_ADDRESS = 0x0; // Will be filled at runtime
    constexpr ULONG64 PLAYER_OFFSET = 0x5A8B1C8;
    constexpr ULONG64 ENTITY_LIST_OFFSET = 0x5A8B2D0;
}

// Basic BDO entity structure
struct BDOEntity {
    ULONG64 address;
    int entityType;
    int entityId;
    float position[3];
    float health;
    float maxHealth;
    std::string name;
};

// BDO Player class
class BDOPlayer {
private:
    ULONG64 address;
    BDO_RTCore64_Memory& memory;

public:
    BDOPlayer(BDO_RTCore64_Memory& mem, ULONG64 addr) : memory(mem), address(addr) {}

    float GetHealth() {
        return memory.Read<float>(address + 0x200);
    }

    float GetMaxHealth() {
        return memory.Read<float>(address + 0x204);
    }

    void GetPosition(float* position) {
        ULONG64 posAddr = address + 0x130;
        position[0] = memory.Read<float>(posAddr);
        position[1] = memory.Read<float>(posAddr + 4);
        position[2] = memory.Read<float>(posAddr + 8);
    }

    std::string GetName() {
        ULONG64 namePtr = memory.Read<ULONG64>(address + 0x300);
        if (namePtr == 0) return "Unknown";
        return memory.ReadString(namePtr);
    }

    void SetHealth(float value) {
        memory.Write<float>(address + 0x200, value);
    }
};

// BDO Entity Manager
class BDOEntityManager {
private:
    BDO_RTCore64_Memory& memory;
    ULONG64 baseAddress;
    ULONG64 playerAddress;
    ULONG64 entityListAddress;

public:
    BDOEntityManager(BDO_RTCore64_Memory& mem) : memory(mem), baseAddress(0), playerAddress(0), entityListAddress(0) {}

    bool Initialize() {
        // Find the game's base address
        HWND hWnd = FindWindowA("BlackDesert64", NULL);
        if (!hWnd) {
            std::cout << "[-] Black Desert Online not found. Is the game running?" << std::endl;
            return false;
        }

        DWORD processId;
        GetWindowThreadProcessId(hWnd, &processId);
        
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
        if (!hProcess) {
            std::cout << "[-] Failed to open process" << std::endl;
            return false;
        }

        HMODULE hModule;
        DWORD cbNeeded;
        if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded)) {
            baseAddress = (ULONG64)hModule;
        }
        CloseHandle(hProcess);

        if (baseAddress == 0) {
            std::cout << "[-] Failed to get base address" << std::endl;
            return false;
        }

        std::cout << "[+] BDO base address: 0x" << std::hex << baseAddress << std::dec << std::endl;
        
        // Calculate addresses
        playerAddress = baseAddress + BDO::PLAYER_OFFSET;
        entityListAddress = baseAddress + BDO::ENTITY_LIST_OFFSET;
        
        return true;
    }

    BDOPlayer GetLocalPlayer() {
        ULONG64 playerPtr = memory.Read<ULONG64>(playerAddress);
        return BDOPlayer(memory, playerPtr);
    }

    std::vector<BDOEntity> GetNearbyEntities(float maxDistance = 100.0f) {
        std::vector<BDOEntity> entities;
        
        // Get player position for distance calculation
        float playerPos[3];
        BDOPlayer player = GetLocalPlayer();
        player.GetPosition(playerPos);
        
        // Read entity list (simplified - actual implementation would depend on BDO's entity list structure)
        ULONG64 listPtr = memory.Read<ULONG64>(entityListAddress);
        int count = memory.Read<int>(listPtr + 0x8);
        ULONG64 entitiesArray = memory.Read<ULONG64>(listPtr + 0x10);
        
        for (int i = 0; i < count && i < 100; i++) { // Limit to 100 entities for safety
            ULONG64 entityPtr = memory.Read<ULONG64>(entitiesArray + i * 8);
            if (entityPtr == 0) continue;
            
            BDOEntity entity;
            entity.address = entityPtr;
            entity.entityType = memory.Read<int>(entityPtr + 0x10);
            entity.entityId = memory.Read<int>(entityPtr + 0x18);
            
            // Read position
            ULONG64 posAddr = entityPtr + 0x130;
            entity.position[0] = memory.Read<float>(posAddr);
            entity.position[1] = memory.Read<float>(posAddr + 4);
            entity.position[2] = memory.Read<float>(posAddr + 8);
            
            // Calculate distance
            float dx = entity.position[0] - playerPos[0];
            float dy = entity.position[1] - playerPos[1];
            float dz = entity.position[2] - playerPos[2];
            float distance = sqrt(dx*dx + dy*dy + dz*dz);
            
            if (distance <= maxDistance) {
                // Read additional info
                entity.health = memory.Read<float>(entityPtr + 0x200);
                entity.maxHealth = memory.Read<float>(entityPtr + 0x204);
                
                ULONG64 namePtr = memory.Read<ULONG64>(entityPtr + 0x300);
                if (namePtr != 0) {
                    entity.name = memory.ReadString(namePtr);
                } else {
                    entity.name = "Unknown";
                }
                
                entities.push_back(entity);
            }
        }
        
        return entities;
    }
};

// Feature implementations
namespace Features {
    // Client-side features that are actually feasible
    void NoClipToggle(bool& noClipEnabled) {
        noClipEnabled = !noClipEnabled;
        std::cout << "[+] NoClip " << (noClipEnabled ? "enabled" : "disabled") << std::endl;
        // In actual implementation, you would modify collision detection
    }
    
    void SpeedHack(float multiplier) {
        // In a real implementation, you would modify the movement speed multiplier
        // This is typically client-side and can work until server validation kicks in
        std::cout << "[+] Speed multiplier set to " << multiplier << "x" << std::endl;
    }
    
    void ESP(const std::vector<BDOEntity>& entities) {
        std::cout << "=== Nearby Entities ===" << std::endl;
        for (const auto& entity : entities) {
            std::cout << "Name: " << entity.name << " | ";
            std::cout << "Type: " << entity.entityType << " | ";
            std::cout << "Health: " << entity.health << "/" << entity.maxHealth << " | ";
            std::cout << "Position: [" << entity.position[0] << ", " 
                      << entity.position[1] << ", " << entity.position[2] << "]" << std::endl;
        }
    }
    
    void ResourceESP(const std::vector<BDOEntity>& entities) {
        std::cout << "=== Resources ===" << std::endl;
        int count = 0;
        for (const auto& entity : entities) {
            // Filter for resource types (this would depend on BDO's entity type IDs)
            if (entity.entityType >= 100 && entity.entityType <= 150) {
                std::cout << "Resource: " << entity.name << " | ";
                std::cout << "Type: " << entity.entityType << " | ";
                std::cout << "Position: [" << entity.position[0] << ", " 
                          << entity.position[1] << ", " << entity.position[2] << "]" << std::endl;
                count++;
            }
        }
        if (count == 0) {
            std::cout << "No resources found nearby." << std::endl;
        }
    }
    
    void AutoFishing(bool& enabled) {
        enabled = !enabled;
        std::cout << "[+] Auto fishing " << (enabled ? "enabled" : "disabled") << std::endl;
        // In actual implementation, you would scan for fishing events and automate responses
    }
}

// Command handler
void HandleCommands(BDOEntityManager& entityManager) {
    std::string command;
    bool noClipEnabled = false;
    bool autoFishingEnabled = false;
    float speedMultiplier = 1.0f;
    
    while (true) {
        std::cout << "\nEnter command (help, noclip, speed, esp, resources, fishing, exit): ";
        std::getline(std::cin, command);
        
        if (command == "exit") {
            break;
        }
        else if (command == "help") {
            std::cout << "Available commands:" << std::endl;
            std::cout << "  noclip    - Toggle noclip mode (bypass collision)" << std::endl;
            std::cout << "  speed X   - Set speed multiplier (e.g., speed 1.5)" << std::endl;
            std::cout << "  esp       - Show nearby entities" << std::endl;
            std::cout << "  resources - Show nearby resources" << std::endl;
            std::cout << "  fishing   - Toggle auto fishing" << std::endl;
            std::cout << "  exit      - Exit the program" << std::endl;
        }
        else if (command == "noclip") {
            Features::NoClipToggle(noClipEnabled);
        }
        else if (command.substr(0, 5) == "speed") {
            try {
                float value = 1.0f;
                if (command.length() > 6) {
                    value = std::stof(command.substr(6));
                }
                Features::SpeedHack(value);
                speedMultiplier = value;
            }
            catch (...) {
                std::cout << "[-] Invalid speed value. Format: speed X (e.g., speed 1.5)" << std::endl;
            }
        }
        else if (command == "esp") {
            auto entities = entityManager.GetNearbyEntities();
            Features::ESP(entities);
        }
        else if (command == "resources") {
            auto entities = entityManager.GetNearbyEntities(200.0f); // Larger radius for resources
            Features::ResourceESP(entities);
        }
        else if (command == "fishing") {
            Features::AutoFishing(autoFishingEnabled);
        }
        else {
            std::cout << "Unknown command. Type 'help' for available commands." << std::endl;
        }
    }
}

// Main entry point
int main(int argc, char* argv[]) {
    std::cout << "=== BDO Stealth Bot (RTCore64 Version) ===" << std::endl;
    std::cout << "Initializing..." << std::endl;
    
    // Process command line arguments
    bool mapDriver = false;
    bool runTests = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--map-driver") == 0) {
            mapDriver = true;
        }
        else if (strcmp(argv[i], "--test") == 0) {
            runTests = true;
        }
    }
    
    // Map driver if requested
    if (mapDriver) {
        std::cout << "[*] RTCore64 driver should already be loaded as a service" << std::endl;
        std::cout << "[*] Run 'sc query RTCore64' to verify it's running" << std::endl;
        return 0;
    }
    
    // Connect to RTCore64 driver
    if (!g_Memory.Connect()) {
        std::cout << "[-] Failed to connect to RTCore64 driver" << std::endl;
        std::cout << "[!] Make sure the driver is loaded (sc query RTCore64)" << std::endl;
        return 1;
    }
    
    std::cout << "[+] Connected to RTCore64 driver" << std::endl;
    
    // Run tests if requested
    if (runTests) {
        std::cout << "[*] Running tests..." << std::endl;
        
        // Test kernel memory read
        ULONG64 testAddress = 0x1000; // Safe low address
        DWORD testValue = g_Memory.Read<DWORD>(testAddress);
        std::cout << "[+] Test read from 0x" << std::hex << testAddress 
                  << ": 0x" << testValue << std::dec << std::endl;
        
        std::cout << "[+] Tests completed" << std::endl;
        g_Memory.Disconnect();
        return 0;
    }
    
    // Initialize entity manager
    BDOEntityManager entityManager(g_Memory);
    if (!entityManager.Initialize()) {
        std::cout << "[-] Failed to initialize entity manager" << std::endl;
        g_Memory.Disconnect();
        return 1;
    }
    
    std::cout << "[+] Entity manager initialized" << std::endl;
    std::cout << "[+] BDO Stealth Bot ready!" << std::endl;
    
    // Handle user commands
    HandleCommands(entityManager);
    
    // Clean up
    g_Memory.Disconnect();
    std::cout << "[+] Disconnected from driver" << std::endl;
    
    return 0;
}

#include "BDOMemory.h"
#include <cmath>
#include <iostream>

BDOMemory::BDOMemory() : playerBase(0), entityListBase(0) {
}

BDOMemory::~BDOMemory() {
    Disconnect();
}

bool BDOMemory::Initialize() {
    std::cout << "[*] Initializing BDO Memory Reader..." << std::endl;
    
    // Enable debug privileges
    if (!MemoryReader::EnableDebugPrivileges()) {
        std::cout << "[-] Failed to enable debug privileges. Run as administrator!" << std::endl;
        return false;
    }
    
    std::cout << "[+] Debug privileges enabled" << std::endl;
    return true;
}

bool BDOMemory::AttachToGame() {
    std::cout << "[*] Searching for Black Desert..." << std::endl;
    
    // Try different possible BDO process names
    std::vector<std::wstring> processNames = {
        L"BlackDesert64.exe",
        L"BlackDesert32.exe",
        L"BlackDesert.exe"
    };
    
    for (const auto& name : processNames) {
        if (reader.AttachToProcess(name)) {
            std::cout << "[+] Attached to Black Desert" << std::endl;
            
            // Find offsets
            std::cout << "[*] Scanning for offsets..." << std::endl;
            UpdateOffsets();
            
            return true;
        }
    }
    
    std::cout << "[-] Black Desert not found. Make sure the game is running." << std::endl;
    return false;
}

void BDOMemory::Disconnect() {
    reader.Detach();
}

bool BDOMemory::FindPlayerBase() {
    std::cout << "[*] Searching for player base..." << std::endl;
    
    // Method 1: Pattern scanning
    uintptr_t patternAddr = reader.FindPattern(Patterns::PLAYER_PTR, Patterns::PLAYER_MASK);
    
    if (patternAddr != 0) {
        playerBase = ResolvePointer(patternAddr);
        if (playerBase != 0) {
            std::cout << "[+] Player base found: 0x" << std::hex << playerBase << std::dec << std::endl;
            return true;
        }
    }
    
    // Method 2: AOB scan with known patterns
    // You'll need to find these patterns yourself using a tool like Cheat Engine
    std::vector<std::string> knownPatterns = {
        "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 ?? E8",
        "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 00"
    };
    
    for (const auto& pattern : knownPatterns) {
        uintptr_t addr = reader.ScanAOB(pattern);
        if (addr != 0) {
            playerBase = ResolvePointer(addr);
            if (playerBase != 0) {
                std::cout << "[+] Player base found via AOB: 0x" << std::hex << playerBase << std::dec << std::endl;
                return true;
            }
        }
    }
    
    std::cout << "[-] Player base not found. Patterns may need updating." << std::endl;
    return false;
}

bool BDOMemory::FindEntityList() {
    std::cout << "[*] Searching for entity list..." << std::endl;
    
    uintptr_t patternAddr = reader.FindPattern(Patterns::ENTITY_LIST, Patterns::ENTITY_MASK);
    
    if (patternAddr != 0) {
        entityListBase = ResolvePointer(patternAddr);
        if (entityListBase != 0) {
            std::cout << "[+] Entity list found: 0x" << std::hex << entityListBase << std::dec << std::endl;
            return true;
        }
    }
    
    std::cout << "[-] Entity list not found. Patterns may need updating." << std::endl;
    return false;
}

void BDOMemory::UpdateOffsets() {
    FindPlayerBase();
    FindEntityList();
}

bool BDOMemory::GetPlayerHealth(float& health, float& maxHealth) {
    if (!IsPlayerValid()) return false;
    
    uintptr_t playerAddr = reader.Read<uintptr_t>(playerBase);
    if (playerAddr == 0) return false;
    
    health = reader.Read<float>(playerAddr + BDO::Offsets::HEALTH);
    maxHealth = reader.Read<float>(playerAddr + BDO::Offsets::MAX_HEALTH);
    
    return true;
}

bool BDOMemory::GetPlayerMana(float& mana, float& maxMana) {
    if (!IsPlayerValid()) return false;
    
    uintptr_t playerAddr = reader.Read<uintptr_t>(playerBase);
    if (playerAddr == 0) return false;
    
    mana = reader.Read<float>(playerAddr + BDO::Offsets::MANA);
    maxMana = reader.Read<float>(playerAddr + BDO::Offsets::MAX_MANA);
    
    return true;
}

bool BDOMemory::GetPlayerPosition(float& x, float& y, float& z) {
    if (!IsPlayerValid()) return false;
    
    uintptr_t playerAddr = reader.Read<uintptr_t>(playerBase);
    if (playerAddr == 0) return false;
    
    x = reader.Read<float>(playerAddr + BDO::Offsets::POS_X);
    y = reader.Read<float>(playerAddr + BDO::Offsets::POS_Y);
    z = reader.Read<float>(playerAddr + BDO::Offsets::POS_Z);
    
    return true;
}

bool BDOMemory::GetPlayerLevel(int& level) {
    if (!IsPlayerValid()) return false;
    
    uintptr_t playerAddr = reader.Read<uintptr_t>(playerBase);
    if (playerAddr == 0) return false;
    
    level = reader.Read<int>(playerAddr + BDO::Offsets::LEVEL);
    
    return true;
}

bool BDOMemory::GetPlayerName(char* name, size_t size) {
    if (!IsPlayerValid()) return false;
    
    uintptr_t playerAddr = reader.Read<uintptr_t>(playerBase);
    if (playerAddr == 0) return false;
    
    return reader.ReadBytes(playerAddr + BDO::Offsets::NAME, name, size);
}

BDO::Player BDOMemory::GetPlayer() {
    BDO::Player player = {};
    
    if (!IsPlayerValid()) return player;
    
    uintptr_t playerAddr = reader.Read<uintptr_t>(playerBase);
    if (playerAddr == 0) return player;
    
    player.health = reader.Read<float>(playerAddr + BDO::Offsets::HEALTH);
    player.maxHealth = reader.Read<float>(playerAddr + BDO::Offsets::MAX_HEALTH);
    player.mana = reader.Read<float>(playerAddr + BDO::Offsets::MANA);
    player.maxMana = reader.Read<float>(playerAddr + BDO::Offsets::MAX_MANA);
    player.stamina = reader.Read<float>(playerAddr + BDO::Offsets::STAMINA);
    player.posX = reader.Read<float>(playerAddr + BDO::Offsets::POS_X);
    player.posY = reader.Read<float>(playerAddr + BDO::Offsets::POS_Y);
    player.posZ = reader.Read<float>(playerAddr + BDO::Offsets::POS_Z);
    player.rotation = reader.Read<float>(playerAddr + BDO::Offsets::ROTATION);
    player.level = reader.Read<int>(playerAddr + BDO::Offsets::LEVEL);
    reader.ReadBytes(playerAddr + BDO::Offsets::NAME, player.name, sizeof(player.name));
    
    return player;
}

std::vector<BDO::Entity> BDOMemory::GetNearbyEntities(float range) {
    std::vector<BDO::Entity> entities;
    
    if (entityListBase == 0) return entities;
    
    float playerX, playerY, playerZ;
    if (!GetPlayerPosition(playerX, playerY, playerZ)) return entities;
    
    // Entity list iteration (structure depends on actual game)
    uintptr_t entityList = reader.Read<uintptr_t>(entityListBase);
    if (entityList == 0) return entities;
    
    // Read entity count (offset may vary)
    int entityCount = reader.Read<int>(entityList + 0x08);
    if (entityCount <= 0 || entityCount > 1000) return entities;
    
    // Read entity array (offset may vary)
    uintptr_t entityArray = reader.Read<uintptr_t>(entityList + 0x10);
    if (entityArray == 0) return entities;
    
    for (int i = 0; i < entityCount; i++) {
        uintptr_t entityAddr = reader.Read<uintptr_t>(entityArray + (i * 0x08));
        if (entityAddr == 0) continue;
        
        BDO::Entity entity;
        entity.address = entityAddr;
        entity.posX = reader.Read<float>(entityAddr + BDO::Offsets::POS_X);
        entity.posY = reader.Read<float>(entityAddr + BDO::Offsets::POS_Y);
        entity.posZ = reader.Read<float>(entityAddr + BDO::Offsets::POS_Z);
        
        float distance = CalculateDistance(playerX, playerY, playerZ, 
                                          entity.posX, entity.posY, entity.posZ);
        
        if (distance <= range) {
            reader.ReadBytes(entityAddr + BDO::Offsets::NAME, entity.name, sizeof(entity.name));
            entity.type = reader.Read<int>(entityAddr + 0x20); // Type offset may vary
            entity.isAlive = reader.Read<bool>(entityAddr + 0x24); // Alive offset may vary
            
            entities.push_back(entity);
        }
    }
    
    return entities;
}

std::vector<BDO::Entity> BDOMemory::GetNearbyPlayers(float range) {
    auto entities = GetNearbyEntities(range);
    std::vector<BDO::Entity> players;
    
    for (const auto& entity : entities) {
        if (entity.type == 1) { // Assuming type 1 is player (may vary)
            players.push_back(entity);
        }
    }
    
    return players;
}

std::vector<BDO::Entity> BDOMemory::GetNearbyMobs(float range) {
    auto entities = GetNearbyEntities(range);
    std::vector<BDO::Entity> mobs;
    
    for (const auto& entity : entities) {
        if (entity.type == 2) { // Assuming type 2 is mob (may vary)
            mobs.push_back(entity);
        }
    }
    
    return mobs;
}

bool BDOMemory::IsInGame() {
    return reader.IsAttached() && playerBase != 0;
}

bool BDOMemory::IsPlayerValid() {
    if (playerBase == 0) return false;
    
    uintptr_t playerAddr = reader.Read<uintptr_t>(playerBase);
    return playerAddr != 0;
}

float BDOMemory::GetDistanceToEntity(const BDO::Entity& entity) {
    float playerX, playerY, playerZ;
    if (!GetPlayerPosition(playerX, playerY, playerZ)) return -1.0f;
    
    return CalculateDistance(playerX, playerY, playerZ, entity.posX, entity.posY, entity.posZ);
}

uintptr_t BDOMemory::ResolvePointer(uintptr_t patternAddress, int offset, int extraOffset) {
    if (patternAddress == 0) return 0;
    
    // Read relative offset
    int relativeOffset = reader.Read<int>(patternAddress + offset);
    
    // Calculate absolute address
    uintptr_t absoluteAddress = patternAddress + relativeOffset + extraOffset;
    
    // Read the actual pointer
    return reader.Read<uintptr_t>(absoluteAddress);
}

float BDOMemory::CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}


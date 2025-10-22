#ifndef BDO_MEMORY_H
#define BDO_MEMORY_H

#include "MemoryReader.h"
#include <string>
#include <vector>

// Black Desert Online specific memory structures and offsets
namespace BDO {
    // These offsets change with game updates - you'll need to find them yourself
    struct Offsets {
        // Base pointers (example addresses - WILL CHANGE with updates)
        static constexpr uintptr_t PLAYER_BASE = 0x0;  // Find this yourself
        static constexpr uintptr_t ENTITY_LIST = 0x0;  // Find this yourself
        
        // Player offsets (relative to player base)
        static constexpr uintptr_t HEALTH = 0x2C;
        static constexpr uintptr_t MAX_HEALTH = 0x30;
        static constexpr uintptr_t MANA = 0x34;
        static constexpr uintptr_t MAX_MANA = 0x38;
        static constexpr uintptr_t STAMINA = 0x3C;
        static constexpr uintptr_t POS_X = 0x40;
        static constexpr uintptr_t POS_Y = 0x44;
        static constexpr uintptr_t POS_Z = 0x48;
        static constexpr uintptr_t ROTATION = 0x50;
        static constexpr uintptr_t NAME = 0x60;
        static constexpr uintptr_t LEVEL = 0x80;
    };
    
    // Player structure
    struct Player {
        float health;
        float maxHealth;
        float mana;
        float maxMana;
        float stamina;
        float posX;
        float posY;
        float posZ;
        float rotation;
        int level;
        char name[64];
    };
    
    // Entity structure
    struct Entity {
        uintptr_t address;
        float posX;
        float posY;
        float posZ;
        char name[64];
        int type;
        bool isAlive;
    };
}

class BDOMemory {
private:
    MemoryReader reader;
    uintptr_t playerBase;
    uintptr_t entityListBase;
    
public:
    BDOMemory();
    ~BDOMemory();
    
    // Connection
    bool Initialize();
    bool AttachToGame();
    void Disconnect();
    
    // Offset finding (pattern scanning)
    bool FindPlayerBase();
    bool FindEntityList();
    void UpdateOffsets();
    
    // Player data reading
    bool GetPlayerHealth(float& health, float& maxHealth);
    bool GetPlayerMana(float& mana, float& maxMana);
    bool GetPlayerPosition(float& x, float& y, float& z);
    bool GetPlayerLevel(int& level);
    bool GetPlayerName(char* name, size_t size);
    BDO::Player GetPlayer();
    
    // Entity reading
    std::vector<BDO::Entity> GetNearbyEntities(float range = 50.0f);
    std::vector<BDO::Entity> GetNearbyPlayers(float range = 50.0f);
    std::vector<BDO::Entity> GetNearbyMobs(float range = 50.0f);
    
    // Utility
    bool IsInGame();
    bool IsPlayerValid();
    float GetDistanceToEntity(const BDO::Entity& entity);
    
    // Pattern signatures for finding offsets
    struct Patterns {
        // These are example patterns - you need to find actual ones
        static constexpr const char* PLAYER_PTR = "\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9";
        static constexpr const char* PLAYER_MASK = "xxx????xxx";
        
        static constexpr const char* ENTITY_LIST = "\x48\x8B\x15\x00\x00\x00\x00\x48\x85\xD2";
        static constexpr const char* ENTITY_MASK = "xxx????xxx";
    };
    
    // Direct memory access
    MemoryReader& GetReader() { return reader; }
    uintptr_t GetPlayerBase() const { return playerBase; }
    uintptr_t GetEntityListBase() const { return entityListBase; }
    
private:
    uintptr_t ResolvePointer(uintptr_t patternAddress, int offset = 3, int extraOffset = 7);
    float CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2);
};

#endif // BDO_MEMORY_H


#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <memory>

// Black Desert Online Specific Kernel Cheat Implementation

// BDO Anti-Cheat Components
namespace BDOAntiCheat {
    // XIGNCODE3 driver and process names
    constexpr const wchar_t* XIGNCODE_DRIVER = L"xhunter1.sys";
    constexpr const wchar_t* XIGNCODE_PROCESS = L"xigncode.exe";
    
    // Easy Anti-Cheat components
    constexpr const wchar_t* EAC_DRIVER = L"easyanticheat.sys";
    constexpr const wchar_t* EAC_SERVICE = L"EasyAntiCheat";
    
    // Registry keys used by XIGNCODE
    constexpr const wchar_t* XIGNCODE_REG_KEY1 = L"SYSTEM\\CurrentControlSet\\Services\\xhunter1";
    constexpr const wchar_t* XIGNCODE_REG_KEY2 = L"SYSTEM\\CurrentControl001\\Services\\xhunter1";
}

// BDO Game Structure Offsets (these need to be updated per game version)
namespace BDOOffsets {
    // Base addresses
    constexpr DWORD64 LOCAL_PLAYER_BASE = 0x5A8B1C8;      // LocalPlayer pointer
    constexpr DWORD64 ENTITY_LIST_BASE = 0x5A8B2D0;       // Entity list
    constexpr DWORD64 CAMERA_BASE = 0x5A90000;             // Camera/ViewMatrix
    constexpr DWORD64 UI_BASE = 0x5AA0000;                // UI Manager
    
    // Player offsets
    namespace Player {
        constexpr DWORD POSITION_X = 0x130;
        constexpr DWORD POSITION_Y = 0x134;
        constexpr DWORD POSITION_Z = 0x138;
        constexpr DWORD HEALTH = 0x200;
        constexpr DWORD MAX_HEALTH = 0x204;
        constexpr DWORD MANA = 0x208;
        constexpr DWORD MAX_MANA = 0x20C;
        constexpr DWORD STAMINA = 0x210;
        constexpr DWORD LEVEL = 0x220;
        constexpr DWORD CLASS_ID = 0x224;
        constexpr DWORD NAME_PTR = 0x300;
        constexpr DWORD GUILD_PTR = 0x320;
        constexpr DWORD MOUNT_PTR = 0x400;
        constexpr DWORD TARGET_PTR = 0x450;
    }
    
    // Entity offsets
    namespace Entity {
        constexpr DWORD TYPE = 0x10;           // Entity type (player, mob, npc, etc)
        constexpr DWORD ID = 0x18;             // Unique entity ID
        constexpr DWORD POSITION = 0x130;      // Position vector
        constexpr DWORD HEALTH = 0x200;        // Current health
        constexpr DWORD MAX_HEALTH = 0x204;    // Max health
        constexpr DWORD NAME_PTR = 0x300;      // Name string pointer
        constexpr DWORD DISTANCE = 0x500;      // Distance from local player
        constexpr DWORD IS_ALIVE = 0x510;      // Alive status
        constexpr DWORD IS_VISIBLE = 0x514;    // Visibility check
    }
    
    // Camera/View offsets
    namespace Camera {
        constexpr DWORD VIEW_MATRIX = 0x1C0;   // 4x4 view matrix
        constexpr DWORD FOV = 0x200;           // Field of view
        constexpr DWORD POSITION = 0x210;      // Camera position
        constexpr DWORD ROTATION = 0x220;      // Camera rotation
    }
}

// BDO Entity Types
enum class BDOEntityType : DWORD {
    Unknown = 0,
    LocalPlayer = 1,
    OtherPlayer = 2,
    Monster = 3,
    NPC = 4,
    Gatherable = 5,
    Loot = 6,
    Mount = 7,
    Pet = 8,
    WorldObject = 9
};

// BDO Player Classes
enum class BDOClass : DWORD {
    Warrior = 0,
    Ranger = 1,
    Sorceress = 2,
    Berserker = 3,
    Tamer = 4,
    Musa = 5,
    Maehwa = 6,
    Valkyrie = 7,
    Ninja = 8,
    Kunoichi = 9,
    Wizard = 10,
    Witch = 11,
    DarkKnight = 12,
    Striker = 13,
    Mystic = 14,
    Archer = 15,
    Lahn = 16,
    Shai = 17,
    Guardian = 18,
    Hashashin = 19,
    Nova = 20,
    Sage = 21,
    Corsair = 22,
    Drakania = 23
};

// BDO Specific Memory Patterns
namespace BDOPatterns {
    // Pattern to find LocalPlayer pointer
    inline const BYTE LOCAL_PLAYER_PATTERN[] = { 
        0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00,  // mov rax, [BlackDesert64.exe+offset]
        0x48, 0x85, 0xC0,                          // test rax, rax
        0x74, 0x0E,                                // je
        0x48, 0x8B, 0x40, 0x08                     // mov rax, [rax+08]
    };
    inline const char* LOCAL_PLAYER_MASK = "xxx????xxxxxxxx";
    
    // Pattern to find EntityList
    inline const BYTE ENTITY_LIST_PATTERN[] = {
        0x48, 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,  // mov rcx, [BlackDesert64.exe+offset]
        0x48, 0x8B, 0x01,                          // mov rax, [rcx]
        0x48, 0x85, 0xC0                           // test rax, rax
    };
    inline const char* ENTITY_LIST_MASK = "xxx????xxxxxx";
    
    // Pattern to find Camera/ViewMatrix
    inline const BYTE CAMERA_PATTERN[] = {
        0xF3, 0x0F, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00,  // movss xmm0, [offset]
        0xF3, 0x0F, 0x11, 0x45, 0x00                     // movss [rbp+?], xmm0
    };
    inline const char* CAMERA_MASK = "xxxx????xxxxx";
}

// BDO Kernel Cheat Interface
class BDOKernelCheat {
private:
    HANDLE hDriver;
    DWORD processId;
    PVOID moduleBase;
    SIZE_T moduleSize;
    bool initialized;
    
    // Cached addresses
    struct {
        PVOID localPlayer;
        PVOID entityList;
        PVOID camera;
        PVOID uiManager;
    } addresses;
    
    // Anti-cheat bypass status
    struct {
        bool xigncodeBypass;
        bool eacBypass;
        bool integrityBypass;
    } bypassStatus;
    
public:
    BDOKernelCheat();
    ~BDOKernelCheat();
    
    // Initialization
    bool Initialize(DWORD bdoProcessId);
    void Shutdown();
    
    // Anti-cheat bypass
    bool BypassXigncode();
    bool BypassEAC();
    bool DisableAntiCheatCallbacks();
    
    // Pattern scanning
    PVOID FindPattern(const BYTE* pattern, const char* mask, SIZE_T patternSize);
    bool UpdateOffsets();
    
    // Player functions
    bool GetLocalPlayer(PVOID& playerPtr);
    bool GetPlayerPosition(float position[3]);
    bool GetPlayerHealth(float& health, float& maxHealth);
    bool GetPlayerMana(float& mana, float& maxMana);
    bool GetPlayerLevel(int& level);
    bool GetPlayerClass(BDOClass& playerClass);
    bool GetPlayerName(std::wstring& name);
    
    // Entity functions
    std::vector<PVOID> GetNearbyEntities(float maxDistance = 100.0f);
    bool GetEntityInfo(PVOID entity, BDOEntityType& type, float position[3], float& health);
    bool IsEntityAlive(PVOID entity);
    bool IsEntityVisible(PVOID entity);
    float GetDistanceToEntity(PVOID entity);
    
    // Combat functions
    bool GetTarget(PVOID& targetPtr);
    bool SetTarget(PVOID entity);
    bool IsInCombat();
    
    // Camera/ESP functions
    bool GetViewMatrix(float matrix[16]);
    bool WorldToScreen(const float worldPos[3], float screenPos[2]);
    
    // Utility functions
    bool IsInGame();
    bool IsLoadingScreen();
    std::string GetCurrentMap();
    
    // Memory operations (wrapped for safety)
    template<typename T>
    bool Read(PVOID address, T& value);
    
    template<typename T>
    bool Write(PVOID address, const T& value);
    
    bool ReadBuffer(PVOID address, PVOID buffer, SIZE_T size);
    bool WriteBuffer(PVOID address, PVOID buffer, SIZE_T size);
    
private:
    // Internal helpers
    bool FindGameModule();
    bool InitializeAddresses();
    bool VerifyGameVersion();
    void ApplyAntiDetection();
};

// BDO Bot Features
class BDOBotFeatures {
private:
    BDOKernelCheat* cheat;
    
public:
    BDOBotFeatures(BDOKernelCheat* kernelCheat) : cheat(kernelCheat) {}
    
    // Farming bot
    void RunFarmingBot(const std::vector<float>& waypoints);
    void RunGrindingBot(BDOEntityType targetType = BDOEntityType::Monster);
    
    // Fishing bot
    void RunFishingBot();
    bool DetectFishBite();
    void CastFishingRod();
    
    // Gathering bot
    void RunGatheringBot();
    std::vector<PVOID> FindGatherables(float radius);
    
    // Combat assistant
    void RunCombatAssist();
    bool AutoTarget(BDOEntityType preferredType);
    void AutoPotion(float healthThreshold = 0.5f);
    
    // ESP overlay data
    struct ESPData {
        std::vector<PVOID> players;
        std::vector<PVOID> monsters;
        std::vector<PVOID> npcs;
        std::vector<PVOID> loot;
        float viewMatrix[16];
    };
    
    bool GetESPData(ESPData& data);
};

// Template implementations
template<typename T>
bool BDOKernelCheat::Read(PVOID address, T& value) {
    return ReadBuffer(address, &value, sizeof(T));
}

template<typename T>
bool BDOKernelCheat::Write(PVOID address, const T& value) {
    return WriteBuffer(address, (PVOID)&value, sizeof(T));
}

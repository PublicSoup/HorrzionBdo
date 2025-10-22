/*
 * BDO Kernel Client Interface
 * User-mode interface for kernel driver communication
 * Professional implementation for production use
 */

#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <memory>
#include <stdint.h>

// IOCTL codes (must match driver)
#define IOCTL_READ_MEMORY       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_MEMORY      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_MODULE_BASE   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PATTERN_SCAN      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PROCESS_INFO  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTECT_MEMORY    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HIDE_DRIVER       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x906, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BYPASS_AC         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x907, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Request structures
#pragma pack(push, 1)
struct MEMORY_OPERATION {
    ULONG ProcessId;
    ULONG64 Address;
    PVOID Buffer;
    SIZE_T Size;
    BOOLEAN Write;
};

struct MODULE_REQUEST {
    ULONG ProcessId;
    WCHAR ModuleName[260];
    ULONG64 BaseAddress;
    SIZE_T ModuleSize;
};

struct PATTERN_SCAN_REQUEST {
    ULONG ProcessId;
    ULONG64 StartAddress;
    ULONG64 EndAddress;
    BYTE Pattern[256];
    BYTE Mask[256];
    SIZE_T PatternLength;
    ULONG64 ResultAddress;
};

struct PROCESS_INFO {
    ULONG ProcessId;
    ULONG64 BaseAddress;
    ULONG64 PEB;
    BOOLEAN IsProtected;
    CHAR ProcessName[260];
};
#pragma pack(pop)

/*
 * BDO Kernel Interface Class
 * Professional interface for kernel-level memory operations
 */
class BDOKernelInterface {
private:
    HANDLE hDriver;
    DWORD targetPid;
    uint64_t gameBase;
    bool connected;
    bool antiCheatBypassed;
    
    // Performance metrics
    uint64_t totalReads;
    uint64_t totalWrites;
    uint64_t failedOperations;
    
    // Internal helpers
    bool SendIOCTL(DWORD ioctl, LPVOID inBuffer, DWORD inSize, LPVOID outBuffer, DWORD outSize);
    
public:
    BDOKernelInterface();
    ~BDOKernelInterface();
    
    // Connection management
    bool Connect(const wchar_t* driverPath = L"\\\\.\\BDOKernelMem");
    void Disconnect();
    bool IsConnected() const { return connected; }
    
    // Process management
    bool AttachToProcess(DWORD processId);
    bool AttachToProcess(const wchar_t* processName);
    bool DetachFromProcess();
    DWORD GetTargetPID() const { return targetPid; }
    
    // Memory operations
    template<typename T>
    bool Read(uint64_t address, T& value);
    
    template<typename T>
    bool Write(uint64_t address, const T& value);
    
    bool ReadBuffer(uint64_t address, void* buffer, size_t size);
    bool WriteBuffer(uint64_t address, const void* buffer, size_t size);
    bool ReadString(uint64_t address, std::string& str, size_t maxLength = 256);
    bool ReadWString(uint64_t address, std::wstring& str, size_t maxLength = 256);
    
    // Advanced memory operations
    uint64_t ReadPointer(uint64_t address);
    uint64_t ReadMultiLevelPointer(uint64_t base, const std::vector<uint64_t>& offsets);
    
    // Module operations
    bool GetModuleBase(const wchar_t* moduleName, uint64_t& baseAddress, size_t& moduleSize);
    uint64_t GetGameBase();
    bool RefreshGameBase();
    
    // Pattern scanning
    uint64_t PatternScan(const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask, 
                         uint64_t start = 0, uint64_t end = 0);
    uint64_t AOBScan(const char* pattern); // Space-separated hex bytes with ?? for wildcards
    std::vector<uint64_t> PatternScanAll(const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask,
                                          uint64_t start = 0, uint64_t end = 0);
    
    // Process information
    bool GetProcessInfo(PROCESS_INFO& info);
    std::string GetProcessName();
    bool IsProcessProtected();
    
    // Anti-cheat bypass
    bool BypassAntiCheat();
    bool IsAntiCheatBypassed() const { return antiCheatBypassed; }
    
    // Utility functions
    bool IsAddressValid(uint64_t address);
    uint64_t GetPEB();
    
    // Statistics
    struct Statistics {
        uint64_t totalReads;
        uint64_t totalWrites;
        uint64_t failedOperations;
        double successRate;
    };
    Statistics GetStatistics() const;
    void ResetStatistics();
    
    // Error handling
    DWORD GetLastError() const { return lastError; }
    std::string GetLastErrorString() const;
    
private:
    DWORD lastError;
    void SetLastError(DWORD error);
};

/*
 * BDO Game Memory Manager
 * High-level interface for BDO-specific memory operations
 */
class BDOGameMemory {
private:
    BDOKernelInterface* kernel;
    bool ownsKernel;
    
    // Cached addresses
    struct CachedAddresses {
        uint64_t playerBase;
        uint64_t entityList;
        uint64_t uiManager;
        uint64_t networkManager;
        uint64_t worldManager;
        
        // Timestamps for cache invalidation
        DWORD playerBaseTime;
        DWORD entityListTime;
        DWORD uiManagerTime;
        DWORD networkManagerTime;
        DWORD worldManagerTime;
    } cache;
    
    static const DWORD CACHE_TIMEOUT_MS = 5000; // 5 seconds
    
    bool IsCacheValid(DWORD timestamp) const;
    void InvalidateCache();
    
public:
    BDOGameMemory(BDOKernelInterface* kernelInterface = nullptr);
    ~BDOGameMemory();
    
    bool Initialize();
    
    // BDO Specific Offsets (2024 - Update as needed)
    struct Offsets {
        // Player structure
        static constexpr uint64_t PlayerController = 0x04B1E2A8;
        static constexpr uint64_t LocalPlayer = 0x08;
        
        // Player stats
        static constexpr uint64_t Health = 0x2C;
        static constexpr uint64_t MaxHealth = 0x30;
        static constexpr uint64_t Mana = 0x34;
        static constexpr uint64_t MaxMana = 0x38;
        static constexpr uint64_t Stamina = 0x3C;
        static constexpr uint64_t MaxStamina = 0x40;
        
        // Player info
        static constexpr uint64_t Level = 0x7C;
        static constexpr uint64_t Experience = 0x80;
        static constexpr uint64_t Silver = 0xA4;
        static constexpr uint64_t Weight = 0xC8;
        static constexpr uint64_t MaxWeight = 0xCC;
        
        // Position
        static constexpr uint64_t Position = 0x140;
        static constexpr uint64_t PosX = 0x140;
        static constexpr uint64_t PosY = 0x144;
        static constexpr uint64_t PosZ = 0x148;
        static constexpr uint64_t Rotation = 0x14C;
        
        // Combat
        static constexpr uint64_t CombatState = 0x1A0;
        static constexpr uint64_t Target = 0x1B0;
        static constexpr uint64_t IsInCombat = 0x1C0;
        
        // Entity list
        static constexpr uint64_t EntityListBase = 0x04B1E2B0;
        static constexpr uint64_t EntityCount = 0x08;
        static constexpr uint64_t EntityArray = 0x10;
        
        // Entity structure
        static constexpr uint64_t EntityType = 0x20;
        static constexpr uint64_t EntityName = 0x30;
        static constexpr uint64_t EntityHealth = 0x2C;
        static constexpr uint64_t EntityPosition = 0x140;
        static constexpr uint64_t EntityDistance = 0x180;
    };
    
    // BDO Patterns for dynamic address resolution
    struct Patterns {
        // Player base pattern
        static constexpr const char* PlayerBase = 
            "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 88 ?? ?? ?? ??";
        
        // Entity list pattern
        static constexpr const char* EntityList = 
            "48 8B 0D ?? ?? ?? ?? 48 85 C9 0F 84 ?? ?? ?? ?? 44 8B 81";
        
        // UI manager pattern
        static constexpr const char* UIManager = 
            "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 ?? E8 ?? ?? ?? ?? 48 8B C8";
    };
    
    // Player data structure
    struct PlayerData {
        float health;
        float maxHealth;
        float mana;
        float maxMana;
        float stamina;
        float maxStamina;
        int level;
        uint64_t experience;
        uint64_t silver;
        float weight;
        float maxWeight;
        float x, y, z;
        float rotation;
        bool inCombat;
        uint64_t targetEntity;
        std::string name;
    };
    
    // Entity data structure
    struct EntityData {
        uint64_t address;
        uint32_t type;
        std::string name;
        float health;
        float maxHealth;
        float x, y, z;
        float distance;
        bool isAlive;
    };
    
    // High-level operations
    bool GetPlayerData(PlayerData& data);
    bool GetEntityList(std::vector<EntityData>& entities);
    bool GetNearbyEntities(std::vector<EntityData>& entities, float maxDistance);
    
    // Address resolution
    uint64_t GetPlayerBase();
    uint64_t GetEntityListBase();
    uint64_t GetUIManager();
    uint64_t GetNetworkManager();
    uint64_t GetWorldManager();
    
    // Utility
    float CalculateDistance(float x1, float y1, float z1, float x2, float y2, float z2);
    bool IsEntityInRange(const EntityData& entity, float range);
    
    BDOKernelInterface* GetKernel() { return kernel; }
};

// Template implementations
template<typename T>
inline bool BDOKernelInterface::Read(uint64_t address, T& value) {
    return ReadBuffer(address, &value, sizeof(T));
}

template<typename T>
inline bool BDOKernelInterface::Write(uint64_t address, const T& value) {
    return WriteBuffer(address, &value, sizeof(T));
}



#pragma once

#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <random>

// BDO Memory Address Resolver
// Advanced system for dynamically finding and resolving game memory addresses

class BDOMemoryResolver {
public:
    struct MemoryRegion {
        PVOID baseAddress;
        SIZE_T regionSize;
        DWORD protection;
        DWORD state;
        DWORD type;
        std::string description;
    };

    struct Pattern {
        std::vector<BYTE> bytes;
        std::vector<BYTE> mask;
        std::string description;
        size_t offset;
        bool relative;
    };

    struct ResolvedAddress {
        std::string name;
        PVOID address;
        PVOID baseAddress;
        size_t offset;
        bool valid;
        std::chrono::steady_clock::time_point lastUpdate;
        std::string description;
    };

    struct BDOOffsets {
        // Player data offsets
        PVOID playerBase;
        PVOID playerHealth;
        PVOID playerMana;
        PVOID playerStamina;
        PVOID playerPosition;
        PVOID playerRotation;
        PVOID playerLevel;
        PVOID playerSilver;
        PVOID playerWeight;
        PVOID playerInventory;
        
        // Combat system offsets
        PVOID combatState;
        PVOID skillCooldowns;
        PVOID targetInfo;
        PVOID damageInfo;
        PVOID skillBar;
        
        // UI system offsets
        PVOID uiBase;
        PVOID chatSystem;
        PVOID menuSystem;
        PVOID inventoryUI;
        PVOID skillUI;
        
        // Network system offsets
        PVOID networkBase;
        PVOID packetHandler;
        PVOID sendFunction;
        PVOID recvFunction;
        
        // Game state offsets
        PVOID gameState;
        PVOID worldTime;
        PVOID weatherSystem;
        PVOID npcManager;
        PVOID itemManager;
    };

private:
    HANDLE hProcess;
    DWORD processId;
    PVOID baseAddress;
    SIZE_T imageSize;
    std::map<std::string, ResolvedAddress> resolvedAddresses;
    std::map<std::string, Pattern> patterns;
    std::vector<MemoryRegion> memoryRegions;
    std::mutex resolverMutex;
    std::mt19937 rng;
    
    // BDO-specific patterns
    void InitializePatterns();
    
    // Memory scanning functions
    std::vector<PVOID> ScanPattern(const Pattern& pattern, PVOID startAddress, SIZE_T size);
    std::vector<PVOID> ScanPatternInRegion(const Pattern& pattern, const MemoryRegion& region);
    std::vector<PVOID> ScanPatternInProcess(const Pattern& pattern);
    
    // Address resolution functions
    PVOID ResolveAddress(const std::string& name, const Pattern& pattern);
    PVOID ResolveAddressWithOffset(const std::string& name, const Pattern& pattern, size_t offset);
    PVOID ResolveAddressWithChain(const std::string& name, const std::vector<size_t>& offsets);
    
    // Memory region functions
    void EnumerateMemoryRegions();
    bool IsValidMemoryRegion(const MemoryRegion& region);
    bool IsExecutableRegion(const MemoryRegion& region);
    bool IsWritableRegion(const MemoryRegion& region);
    
    // Pattern matching functions
    bool MatchPattern(PVOID address, const Pattern& pattern);
    bool MatchPatternWithMask(PVOID address, const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask);
    
    // Utility functions
    std::string AddressToString(PVOID address);
    std::string AddressToHexString(PVOID address);
    PVOID StringToAddress(const std::string& str);
    bool IsValidAddress(PVOID address);
    
    // BDO-specific resolution functions
    PVOID ResolvePlayerBase();
    PVOID ResolveCombatSystem();
    PVOID ResolveUISystem();
    PVOID ResolveNetworkSystem();
    PVOID ResolveGameState();
    
    // Anti-detection functions
    void AddRandomDelay();
    void VaryScanTiming();
    bool UseIndirectAccess();
    
public:
    BDOMemoryResolver();
    ~BDOMemoryResolver();
    
    // Initialization
    bool Initialize(DWORD pid);
    bool AttachToProcess(const std::wstring& processName);
    void DetachFromProcess();
    
    // Address resolution
    bool ResolveAllAddresses();
    bool ResolveAddress(const std::string& name);
    bool ResolveBDOOffsets(BDOOffsets& offsets);
    
    // Address access
    PVOID GetAddress(const std::string& name);
    bool SetAddress(const std::string& name, PVOID address);
    bool IsAddressValid(const std::string& name);
    
    // Memory operations
    bool ReadMemory(PVOID address, PVOID buffer, SIZE_T size);
    bool WriteMemory(PVOID address, PVOID buffer, SIZE_T size);
    bool ReadString(PVOID address, std::string& str, SIZE_T maxLength = 256);
    bool WriteString(PVOID address, const std::string& str);
    
    // Pattern scanning
    std::vector<PVOID> ScanForPattern(const std::string& patternName);
    std::vector<PVOID> ScanForCustomPattern(const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask);
    std::vector<PVOID> ScanForString(const std::string& str);
    
    // Memory region operations
    std::vector<MemoryRegion> GetMemoryRegions();
    std::vector<MemoryRegion> GetExecutableRegions();
    std::vector<MemoryRegion> GetWritableRegions();
    
    // Address management
    void AddPattern(const std::string& name, const Pattern& pattern);
    void RemovePattern(const std::string& name);
    void ClearResolvedAddresses();
    
    // BDO-specific functions
    bool ResolvePlayerData(BDOOffsets& offsets);
    bool ResolveCombatData(BDOOffsets& offsets);
    bool ResolveUIData(BDOOffsets& offsets);
    bool ResolveNetworkData(BDOOffsets& offsets);
    
    // Utility functions
    std::string GetAddressInfo(const std::string& name);
    void PrintResolvedAddresses();
    void SaveAddressesToFile(const std::string& filename);
    bool LoadAddressesFromFile(const std::string& filename);
    
    // Anti-detection
    void EnableAntiDetection(bool enable);
    void SetScanDelay(int minMs, int maxMs);
    void SetMaxScanSize(SIZE_T maxSize);
    
    // Debugging
    void EnableDebugOutput(bool enable);
    void LogMessage(const std::string& message);
    
private:
    bool debugEnabled;
    bool antiDetectionEnabled;
    int minScanDelay;
    int maxScanDelay;
    SIZE_T maxScanSize;
    std::ofstream debugLog;
};

// BDO-specific pattern definitions
namespace BDOPatterns {
    // Player data patterns
    extern const BDOMemoryResolver::Pattern PLAYER_BASE_PATTERN;
    extern const BDOMemoryResolver::Pattern HEALTH_OFFSET_PATTERN;
    extern const BDOMemoryResolver::Pattern MANA_OFFSET_PATTERN;
    extern const BDOMemoryResolver::Pattern STAMINA_OFFSET_PATTERN;
    extern const BDOMemoryResolver::Pattern POSITION_OFFSET_PATTERN;
    extern const BDOMemoryResolver::Pattern ROTATION_OFFSET_PATTERN;
    extern const BDOMemoryResolver::Pattern LEVEL_OFFSET_PATTERN;
    extern const BDOMemoryResolver::Pattern SILVER_OFFSET_PATTERN;
    extern const BDOMemoryResolver::Pattern WEIGHT_OFFSET_PATTERN;
    extern const BDOMemoryResolver::Pattern INVENTORY_OFFSET_PATTERN;
    
    // Combat system patterns
    extern const BDOMemoryResolver::Pattern COMBAT_STATE_PATTERN;
    extern const BDOMemoryResolver::Pattern SKILL_COOLDOWN_PATTERN;
    extern const BDOMemoryResolver::Pattern TARGET_INFO_PATTERN;
    extern const BDOMemoryResolver::Pattern DAMAGE_INFO_PATTERN;
    extern const BDOMemoryResolver::Pattern SKILL_BAR_PATTERN;
    
    // UI system patterns
    extern const BDOMemoryResolver::Pattern UI_BASE_PATTERN;
    extern const BDOMemoryResolver::Pattern CHAT_SYSTEM_PATTERN;
    extern const BDOMemoryResolver::Pattern MENU_SYSTEM_PATTERN;
    extern const BDOMemoryResolver::Pattern INVENTORY_UI_PATTERN;
    extern const BDOMemoryResolver::Pattern SKILL_UI_PATTERN;
    
    // Network system patterns
    extern const BDOMemoryResolver::Pattern NETWORK_BASE_PATTERN;
    extern const BDOMemoryResolver::Pattern PACKET_HANDLER_PATTERN;
    extern const BDOMemoryResolver::Pattern SEND_FUNCTION_PATTERN;
    extern const BDOMemoryResolver::Pattern RECV_FUNCTION_PATTERN;
    
    // Game state patterns
    extern const BDOMemoryResolver::Pattern GAME_STATE_PATTERN;
    extern const BDOMemoryResolver::Pattern WORLD_TIME_PATTERN;
    extern const BDOMemoryResolver::Pattern WEATHER_SYSTEM_PATTERN;
    extern const BDOMemoryResolver::Pattern NPC_MANAGER_PATTERN;
    extern const BDOMemoryResolver::Pattern ITEM_MANAGER_PATTERN;
}

// Utility functions
namespace MemoryUtils {
    std::vector<BYTE> StringToBytes(const std::string& str);
    std::vector<BYTE> HexStringToBytes(const std::string& hex);
    std::string BytesToHexString(const std::vector<BYTE>& bytes);
    std::string AddressToHexString(PVOID address);
    PVOID HexStringToAddress(const std::string& hex);
    bool IsValidPattern(const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask);
    size_t CalculatePatternOffset(const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask);
}

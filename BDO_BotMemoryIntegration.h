#pragma once

#include "BDO_AdvancedMemory.h"
#include "BDO_MemoryScanner.h"
#include <windows.h>
#include <map>
#include <functional>
#include <atomic>
#include <thread>
#include <chrono>
#include <queue>

// Memory Address Integration System for Bot Automation
// This system connects scanned memory addresses to bot actions

// Known BDO memory addresses structure
struct BDOAddresses {
    // Player Information
    PVOID playerBase = nullptr;
    PVOID playerHealth = nullptr;
    PVOID playerMaxHealth = nullptr;
    PVOID playerMana = nullptr;
    PVOID playerMaxMana = nullptr;
    PVOID playerStamina = nullptr;
    PVOID playerMaxStamina = nullptr;
    PVOID playerLevel = nullptr;
    PVOID playerExp = nullptr;
    PVOID playerWeight = nullptr;
    PVOID playerMaxWeight = nullptr;
    PVOID playerSilver = nullptr;
    
    // Position & Movement
    PVOID playerX = nullptr;
    PVOID playerY = nullptr;
    PVOID playerZ = nullptr;
    PVOID playerRotation = nullptr;
    PVOID playerSpeed = nullptr;
    PVOID mountStatus = nullptr;
    
    // Combat
    PVOID combatState = nullptr;
    PVOID targetEntity = nullptr;
    PVOID targetHealth = nullptr;
    PVOID targetMaxHealth = nullptr;
    PVOID targetDistance = nullptr;
    PVOID skillCooldowns = nullptr;
    PVOID attackSpeed = nullptr;
    
    // Inventory
    PVOID inventoryBase = nullptr;
    PVOID inventorySlots = nullptr;
    PVOID inventoryCount = nullptr;
    PVOID equipmentBase = nullptr;
    
    // UI & Game State
    PVOID gameState = nullptr;
    PVOID uiState = nullptr;
    PVOID chatWindow = nullptr;
    PVOID questLog = nullptr;
    PVOID minimap = nullptr;
    
    // Fishing
    PVOID fishingState = nullptr;
    PVOID fishingRodDurability = nullptr;
    PVOID fishBiteIndicator = nullptr;
    
    // Gathering
    PVOID gatheringTool = nullptr;
    PVOID gatheringEnergy = nullptr;
    PVOID nearbyResources = nullptr;
};

// Memory value conditions for triggers
struct MemoryCondition {
    enum Type {
        LESS_THAN,
        GREATER_THAN,
        EQUAL_TO,
        NOT_EQUAL_TO,
        BETWEEN,
        CHANGED,
        INCREASED,
        DECREASED
    };
    
    Type type;
    std::variant<int, float, double> value1;
    std::variant<int, float, double> value2; // For BETWEEN condition
    
    template<typename T>
    bool Check(T currentValue) const {
        return std::visit([&](auto&& val1) -> bool {
            using V = std::decay_t<decltype(val1)>;
            if constexpr (std::is_arithmetic_v<T> && std::is_arithmetic_v<V>) {
                switch (type) {
                    case LESS_THAN: return currentValue < static_cast<T>(val1);
                    case GREATER_THAN: return currentValue > static_cast<T>(val1);
                    case EQUAL_TO: return currentValue == static_cast<T>(val1);
                    case NOT_EQUAL_TO: return currentValue != static_cast<T>(val1);
                    case BETWEEN:
                        return std::visit([&](auto&& val2) -> bool {
                            return currentValue >= static_cast<T>(val1) && 
                                   currentValue <= static_cast<T>(val2);
                        }, value2);
                    default: return false;
                }
            }
            return false;
        }, value1);
    }
};

// Bot action triggered by memory conditions
struct BotAction {
    std::string name;
    std::function<void()> action;
    std::chrono::milliseconds cooldown;
    std::chrono::steady_clock::time_point lastExecuted;
    bool enabled = true;
    int priority = 0; // Higher priority executes first
};

// Memory trigger linking conditions to actions
struct MemoryTrigger {
    std::string name;
    PVOID address;
    ValueType valueType;
    MemoryCondition condition;
    std::vector<std::string> actionNames;
    bool enabled = true;
    bool continuous = false; // Keep triggering while condition is true
    std::chrono::steady_clock::time_point lastTriggered;
};

class BDOBotMemoryIntegration {
private:
    std::unique_ptr<BDOAdvancedMemory> memory;
    BDOAddresses addresses;
    std::map<std::string, BotAction> actions;
    std::map<std::string, MemoryTrigger> triggers;
    std::atomic<bool> isRunning{false};
    std::thread monitorThread;
    std::mutex actionMutex;
    std::queue<std::string> actionQueue;
    
    // Address validation
    std::map<std::string, std::pair<PVOID*, std::string>> addressMap;
    
    // Statistics
    struct Stats {
        std::atomic<size_t> totalScans{0};
        std::atomic<size_t> successfulScans{0};
        std::atomic<size_t> triggersActivated{0};
        std::atomic<size_t> actionsExecuted{0};
        std::chrono::steady_clock::time_point startTime;
    } stats;
    
    // Private methods
    void InitializeAddressMap();
    void MonitorLoop();
    void ProcessTriggers();
    void ExecuteQueuedActions();
    bool ValidateAddress(PVOID address);
    void UpdateAddressValue(const std::string& name, PVOID address);
    
public:
    BDOBotMemoryIntegration();
    ~BDOBotMemoryIntegration();
    
    // Initialization
    bool Initialize(DWORD processId);
    bool AttachToGame(const std::wstring& processName = L"BlackDesert64.exe");
    void Shutdown();
    
    // Address Scanning and Management
    bool ScanForAddresses();
    bool ScanForAddress(const std::string& addressName, const std::string& pattern);
    bool LoadAddressesFromFile(const std::string& filename);
    bool SaveAddressesToFile(const std::string& filename);
    bool UpdateAddress(const std::string& name, PVOID address);
    PVOID GetAddress(const std::string& name);
    
    // Manual Address Setting (for use with external scanners like MSI Afterburner)
    bool SetPlayerHealthAddress(PVOID address);
    bool SetPlayerManaAddress(PVOID address);
    bool SetPlayerPositionAddresses(PVOID x, PVOID y, PVOID z);
    bool SetCombatAddresses(PVOID combatState, PVOID target);
    bool SetInventoryBaseAddress(PVOID address);
    
    // Bot Actions
    void RegisterAction(const std::string& name, std::function<void()> action, 
                       std::chrono::milliseconds cooldown = std::chrono::milliseconds(0), 
                       int priority = 0);
    void UnregisterAction(const std::string& name);
    void EnableAction(const std::string& name, bool enable);
    void ExecuteAction(const std::string& name);
    
    // Memory Triggers
    void AddTrigger(const std::string& name, PVOID address, ValueType type,
                   MemoryCondition condition, const std::vector<std::string>& actions,
                   bool continuous = false);
    void AddHealthTrigger(float threshold, const std::vector<std::string>& actions);
    void AddManaTrigger(float threshold, const std::vector<std::string>& actions);
    void AddWeightTrigger(float threshold, const std::vector<std::string>& actions);
    void AddCombatTrigger(bool inCombat, const std::vector<std::string>& actions);
    void RemoveTrigger(const std::string& name);
    void EnableTrigger(const std::string& name, bool enable);
    
    // Monitoring
    bool StartMonitoring(std::chrono::milliseconds interval = std::chrono::milliseconds(100));
    void StopMonitoring();
    bool IsMonitoring() const { return isRunning; }
    
    // Memory Reading Helpers
    bool ReadPlayerStats(int& health, int& maxHealth, int& mana, int& maxMana);
    bool ReadPlayerPosition(float& x, float& y, float& z);
    bool ReadCombatState(bool& inCombat, PVOID& target);
    bool ReadInventoryInfo(int& usedSlots, int& totalSlots);
    float GetPlayerHealthPercentage();
    float GetPlayerManaPercentage();
    float GetPlayerWeightPercentage();
    
    // Pre-configured Bot Setups
    void SetupCombatBot();
    void SetupFishingBot();
    void SetupGatheringBot();
    void SetupGrindingBot();
    
    // Statistics
    void PrintStatistics();
    size_t GetTotalScans() const { return stats.totalScans; }
    size_t GetTriggersActivated() const { return stats.triggersActivated; }
    size_t GetActionsExecuted() const { return stats.actionsExecuted; }
};

// Helper class for easy address scanning
class AddressScanner {
private:
    BDOBotMemoryIntegration* integration;
    
public:
    AddressScanner(BDOBotMemoryIntegration* bot) : integration(bot) {}
    
    // Scan for common game values
    bool ScanForHealth(int currentHealth);
    bool ScanForMana(int currentMana);
    bool ScanForLevel(int level);
    bool ScanForSilver(long long silver);
    bool ScanForPosition(float x, float y, float z);
    
    // Pattern-based scanning
    bool ScanForPlayerBase();
    bool ScanForUIBase();
    bool ScanForInventoryBase();
    
    // Verification
    bool VerifyAddresses();
    void PrintFoundAddresses();
};

// Example bot implementations
class CombatBot {
private:
    BDOBotMemoryIntegration* integration;
    
public:
    CombatBot(BDOBotMemoryIntegration* bot) : integration(bot) {}
    
    void Setup() {
        // Register combat actions
        integration->RegisterAction("UseHealthPotion", [this]() { UseHealthPotion(); }, std::chrono::seconds(5));
        integration->RegisterAction("UseManaPotion", [this]() { UseManaPotion(); }, std::chrono::seconds(5));
        integration->RegisterAction("AttackTarget", [this]() { AttackTarget(); }, std::chrono::milliseconds(500));
        integration->RegisterAction("FindNewTarget", [this]() { FindNewTarget(); }, std::chrono::seconds(1));
        
        // Add triggers
        integration->AddHealthTrigger(0.5f, {"UseHealthPotion"});
        integration->AddManaTrigger(0.3f, {"UseManaPotion"});
        integration->AddCombatTrigger(true, {"AttackTarget"});
        integration->AddCombatTrigger(false, {"FindNewTarget"});
    }
    
private:
    void UseHealthPotion();
    void UseManaPotion();
    void AttackTarget();
    void FindNewTarget();
};

// Memory pattern definitions for BDO (using BDOPatterns from BDO_MemoryResolver.h)
namespace BDOPatternsIntegration {
    // Additional patterns specific to bot integration
    const std::string INVENTORY_BASE_PATTERN = "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 1A 48 8B 01";
    
    // Offsets from base addresses (update these for your game version)
    constexpr size_t HEALTH_OFFSET = 0x2C;
    constexpr size_t MAX_HEALTH_OFFSET = 0x30;
    constexpr size_t MANA_OFFSET = 0x34;
    constexpr size_t MAX_MANA_OFFSET = 0x38;
    constexpr size_t POSITION_X_OFFSET = 0x40;
    constexpr size_t POSITION_Y_OFFSET = 0x44;
    constexpr size_t POSITION_Z_OFFSET = 0x48;
    constexpr size_t LEVEL_OFFSET = 0x60;
    constexpr size_t COMBAT_STATE_OFFSET = 0x100;
}

#pragma once

#include "BDO_MemoryResolver.h"
#include "BDO_MemoryScanner.h"
#include "BDO_RTCore64_Driver.h"
#include <windows.h>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <random>
#include <fstream>

// Advanced BDO Memory Management System
// Provides high-level memory operations and anti-detection features

class BDOAdvancedMemory {
public:
    struct MemoryHook {
        std::string name;
        PVOID originalAddress;
        PVOID hookAddress;
        std::vector<BYTE> originalBytes;
        std::vector<BYTE> hookBytes;
        bool active;
        std::chrono::steady_clock::time_point installTime;
    };

    struct MemoryPatch {
        std::string name;
        PVOID address;
        std::vector<BYTE> originalBytes;
        std::vector<BYTE> patchBytes;
        bool applied;
        std::chrono::steady_clock::time_point applyTime;
    };

    struct MemoryScan {
        std::string name;
        std::vector<BYTE> pattern;
        std::vector<BYTE> mask;
        std::vector<PVOID> results;
        std::chrono::steady_clock::time_point lastScan;
        bool autoRescan;
        int rescanInterval; // seconds
    };

    struct MemoryProtection {
        PVOID address;
        SIZE_T size;
        DWORD originalProtection;
        DWORD newProtection;
        bool active;
    };

private:
    std::unique_ptr<BDOMemoryResolver> resolver;
    std::unique_ptr<BDOMemoryScanner> scanner;
    std::unique_ptr<MemoryValueMonitor> valueMonitor;
    std::map<std::string, MemoryHook> hooks;
    std::map<std::string, MemoryPatch> patches;
    std::map<std::string, MemoryScan> scans;
    std::vector<MemoryProtection> protections;
    std::mutex memoryMutex;
    std::mt19937 rng;
    
    // Anti-detection features
    bool antiDetectionEnabled;
    bool stealthMode;
    int minDelay;
    int maxDelay;
    std::chrono::steady_clock::time_point lastOperation;
    
    // Memory operations
    bool ReadMemoryInternal(PVOID address, PVOID buffer, SIZE_T size);
    bool WriteMemoryInternal(PVOID address, PVOID buffer, SIZE_T size);
    bool ProtectMemoryInternal(PVOID address, SIZE_T size, DWORD protection);
    
    // Hook management
    bool InstallHookInternal(const std::string& name, PVOID address, const std::vector<BYTE>& hookBytes);
    bool RemoveHookInternal(const std::string& name);
    bool RestoreOriginalBytes(PVOID address, const std::vector<BYTE>& originalBytes);
    
    // Patch management
    bool ApplyPatchInternal(const std::string& name, PVOID address, const std::vector<BYTE>& patchBytes);
    bool RemovePatchInternal(const std::string& name);
    
    // Scan management
    void PerformScanInternal(const std::string& name);
    void AutoRescan();
    
    // Anti-detection
    void AddRandomDelay();
    void VaryTiming();
    bool IsOperationSafe();
    void LogOperation(const std::string& operation);
    
    // Utility functions
    std::vector<BYTE> ReadBytes(PVOID address, SIZE_T size);
    bool WriteBytes(PVOID address, const std::vector<BYTE>& bytes);
    DWORD GetMemoryProtection(PVOID address);
    bool IsValidAddress(PVOID address);
    
public:
    BDOAdvancedMemory();
    ~BDOAdvancedMemory();
    
    // Initialization
    bool Initialize(DWORD processId);
    bool AttachToProcess(const std::wstring& processName);
    void DetachFromProcess();
    
    // Memory operations
    bool ReadMemory(PVOID address, PVOID buffer, SIZE_T size);
    bool WriteMemory(PVOID address, PVOID buffer, SIZE_T size);
    bool ReadString(PVOID address, std::string& str, SIZE_T maxLength = 256);
    bool WriteString(PVOID address, const std::string& str);
    
    // Advanced memory operations
    bool ReadMemoryChain(PVOID baseAddress, const std::vector<size_t>& offsets, PVOID buffer, SIZE_T size);
    bool WriteMemoryChain(PVOID baseAddress, const std::vector<size_t>& offsets, PVOID buffer, SIZE_T size);
    bool ReadPointerChain(PVOID baseAddress, const std::vector<size_t>& offsets, PVOID& result);
    bool WritePointerChain(PVOID baseAddress, const std::vector<size_t>& offsets, PVOID value);
    
    // Memory protection
    bool ProtectMemory(PVOID address, SIZE_T size, DWORD protection);
    bool UnprotectMemory(PVOID address, SIZE_T size);
    bool RestoreMemoryProtection(PVOID address, SIZE_T size);
    
    // Hook management
    bool InstallHook(const std::string& name, PVOID address, const std::vector<BYTE>& hookBytes);
    bool RemoveHook(const std::string& name);
    bool IsHookActive(const std::string& name);
    std::vector<std::string> GetActiveHooks();
    
    // Patch management
    bool ApplyPatch(const std::string& name, PVOID address, const std::vector<BYTE>& patchBytes);
    bool RemovePatch(const std::string& name);
    bool IsPatchApplied(const std::string& name);
    std::vector<std::string> GetAppliedPatches();
    
    // Scan management
    bool AddScan(const std::string& name, const std::vector<BYTE>& pattern, const std::vector<BYTE>& mask, bool autoRescan = false, int rescanInterval = 60);
    bool RemoveScan(const std::string& name);
    bool PerformScan(const std::string& name);
    bool PerformAllScans();
    std::vector<PVOID> GetScanResults(const std::string& name);
    std::vector<std::string> GetActiveScans();
    
    // Advanced memory scanning
    bool ScanForValue(ValueType type, const std::string& value, ScanType scanType = ScanType::ExactValue);
    bool ScanForUnknown(ValueType type);
    bool NextScan(ScanType scanType, const std::string& value = "");
    std::vector<ScanResult> GetScanResults();
    bool CreateMemorySnapshot(const std::string& name);
    bool CompareSnapshot(const std::string& name, ScanType compareType = ScanType::Changed);
    
    // Memory structure analysis
    bool AnalyzeMemoryStructure(PVOID address, SIZE_T maxSize, BDOMemoryScanner::StructureInfo& info);
    bool FindPointerChain(PVOID sourceAddress, PVOID targetAddress, std::vector<SIZE_T>& offsets);
    bool DumpMemoryRegion(PVOID address, SIZE_T size, const std::string& filename);
    
    // Value monitoring
    bool MonitorValue(PVOID address, ValueType type);
    bool StopMonitoringValue(PVOID address);
    std::vector<std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>>> GetValueHistory(PVOID address);
    bool FreezeValue(PVOID address, ValueType type, const std::string& value);
    bool UnfreezeValue(PVOID address);
    
    // BDO-specific functions
    bool ReadPlayerHealth(int& current, int& maximum);
    bool ReadPlayerMana(int& current, int& maximum);
    bool ReadPlayerStamina(int& current, int& maximum);
    bool ReadPlayerWeight(float& current, float& maximum);
    bool ReadPlayerPosition(float& x, float& y, float& z);
    bool ReadPlayerRotation(float& x, float& y, float& z);
    bool ReadPlayerLevel(int& level);
    bool ReadPlayerSilver(long long& silver);
    
    bool WritePlayerHealth(int current, int maximum);
    bool WritePlayerMana(int current, int maximum);
    bool WritePlayerStamina(int current, int maximum);
    bool WritePlayerWeight(float current, float maximum);
    bool WritePlayerPosition(float x, float y, float z);
    bool WritePlayerRotation(float x, float y, float z);
    bool WritePlayerLevel(int level);
    bool WritePlayerSilver(long long silver);
    
    // Combat system
    bool ReadCombatState(int& state);
    bool WriteCombatState(int state);
    bool ReadSkillCooldown(int skillId, float& cooldown);
    bool WriteSkillCooldown(int skillId, float cooldown);
    bool ReadTargetInfo(int& targetId, float& distance);
    bool WriteTargetInfo(int targetId, float distance);
    
    // UI system
    bool ReadChatMessage(int index, std::string& message);
    bool WriteChatMessage(const std::string& message);
    bool ReadMenuState(int& state);
    bool WriteMenuState(int state);
    bool ReadInventorySlot(int slot, int& itemId, int& quantity);
    bool WriteInventorySlot(int slot, int itemId, int quantity);
    
    // Network system
    bool ReadPacketData(int packetId, std::vector<BYTE>& data);
    bool WritePacketData(int packetId, const std::vector<BYTE>& data);
    bool InterceptPacket(int packetId, bool intercept);
    bool IsPacketIntercepted(int packetId);
    
    // Game state
    bool ReadGameState(int& state);
    bool WriteGameState(int state);
    bool ReadWorldTime(float& time);
    bool WriteWorldTime(float time);
    bool ReadWeatherState(int& state);
    bool WriteWeatherState(int state);
    
    // Anti-detection
    void EnableAntiDetection(bool enable);
    void EnableStealthMode(bool enable);
    void SetOperationDelay(int minMs, int maxMs);
    void SetMaxOperationsPerSecond(int maxOps);
    
    // Monitoring
    void StartMonitoring();
    void StopMonitoring();
    bool IsMonitoring();
    void SetMonitoringInterval(int seconds);
    
    // Utility functions
    std::string GetAddressInfo(PVOID address);
    void PrintMemoryInfo();
    void SaveMemoryState(const std::string& filename);
    bool LoadMemoryState(const std::string& filename);
    bool ValidateMemoryIntegrity();
    std::vector<PVOID> FindReferences(PVOID targetAddress);
    bool IsAddressExecutable(PVOID address);
    bool IsAddressWritable(PVOID address);
    SIZE_T GetMemoryRegionSize(PVOID address);
    std::string AddressToHexString(PVOID address);
    
    // Debug functions
    void EnableDebugOutput(bool enable);
    void LogMessage(const std::string& message);
    void PrintDebugInfo();
    
private:
    bool monitoring;
    int monitoringInterval;
    std::thread monitoringThread;
    bool debugEnabled;
    std::ofstream debugLog;
    
    void MonitoringLoop();
    void LogOperation(const std::string& operation, PVOID address, SIZE_T size);
};

// BDO-specific memory constants
namespace BDOMemory {
    // Player data offsets (these change with game updates)
    constexpr size_t PLAYER_BASE_OFFSET = 0x3A5B2C8;
    constexpr size_t HEALTH_OFFSET = 0x2C;
    constexpr size_t MANA_OFFSET = 0x30;
    constexpr size_t STAMINA_OFFSET = 0x34;
    constexpr size_t POSITION_OFFSET = 0x40;
    constexpr size_t ROTATION_OFFSET = 0x50;
    constexpr size_t LEVEL_OFFSET = 0x60;
    constexpr size_t SILVER_OFFSET = 0x70;
    constexpr size_t WEIGHT_OFFSET = 0x80;
    constexpr size_t INVENTORY_OFFSET = 0x90;
    
    // Combat system offsets
    constexpr size_t COMBAT_STATE_OFFSET = 0x100;
    constexpr size_t SKILL_COOLDOWN_OFFSET = 0x110;
    constexpr size_t TARGET_INFO_OFFSET = 0x120;
    constexpr size_t DAMAGE_INFO_OFFSET = 0x130;
    
    // UI system offsets
    constexpr size_t UI_BASE_OFFSET = 0x200;
    constexpr size_t CHAT_SYSTEM_OFFSET = 0x210;
    constexpr size_t MENU_SYSTEM_OFFSET = 0x220;
    constexpr size_t INVENTORY_UI_OFFSET = 0x230;
    
    // Network system offsets
    constexpr size_t NETWORK_BASE_OFFSET = 0x300;
    constexpr size_t PACKET_HANDLER_OFFSET = 0x310;
    constexpr size_t SEND_FUNCTION_OFFSET = 0x320;
    constexpr size_t RECV_FUNCTION_OFFSET = 0x330;
    
    // Game state offsets
    constexpr size_t GAME_STATE_OFFSET = 0x400;
    constexpr size_t WORLD_TIME_OFFSET = 0x410;
    constexpr size_t WEATHER_SYSTEM_OFFSET = 0x420;
    constexpr size_t NPC_MANAGER_OFFSET = 0x430;
    constexpr size_t ITEM_MANAGER_OFFSET = 0x440;
    
    // Memory protection constants
    constexpr DWORD READ_ONLY = PAGE_READONLY;
    constexpr DWORD READ_WRITE = PAGE_READWRITE;
    constexpr DWORD EXECUTE_READ = PAGE_EXECUTE_READ;
    constexpr DWORD EXECUTE_READ_WRITE = PAGE_EXECUTE_READWRITE;
    
    // Hook types
    constexpr int HOOK_TYPE_INLINE = 0;
    constexpr int HOOK_TYPE_IAT = 1;
    constexpr int HOOK_TYPE_EAT = 2;
    constexpr int HOOK_TYPE_SSDT = 3;
    
    // Patch types
    constexpr int PATCH_TYPE_NOP = 0;
    constexpr int PATCH_TYPE_JMP = 1;
    constexpr int PATCH_TYPE_CALL = 2;
    constexpr int PATCH_TYPE_RET = 3;
    
    // Scan types
    constexpr int SCAN_TYPE_PATTERN = 0;
    constexpr int SCAN_TYPE_STRING = 1;
    constexpr int SCAN_TYPE_FUNCTION = 2;
    constexpr int SCAN_TYPE_STRUCTURE = 3;
}

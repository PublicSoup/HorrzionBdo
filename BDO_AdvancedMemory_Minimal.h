#pragma once

#include "BDO_MemoryResolver.h"
#include <windows.h>
#include <memory>
#include <random>
#include <fstream>

// Minimal BDO Memory Management System
// Simplified version without scanner dependencies

class BDOAdvancedMemory {
public:
    BDOAdvancedMemory();
    ~BDOAdvancedMemory();

    // Process management
    bool AttachToProcess(const std::wstring& processName);
    void DetachFromProcess();

    // Anti-detection
    void EnableAntiDetection(bool enable);
    void EnableStealthMode(bool enable);
    void SetOperationDelay(int minMs, int maxMs);

    // Memory operations
    bool ReadMemory(PVOID address, PVOID buffer, SIZE_T size);
    bool WriteMemory(PVOID address, PVOID buffer, SIZE_T size);

    // BDO-specific functions
    bool ReadPlayerHealth(int& current, int& max);
    bool ReadPlayerMana(int& current, int& max);
    bool ReadPlayerWeight(float& current, float& max);
    bool ReadCombatState(int& state);
    bool ReadGameState(int& state);

    // Monitoring
    void StopMonitoring();

    // Debug
    void EnableDebugOutput(bool enable);
    void LogMessage(const std::string& message);

private:
    std::unique_ptr<BDOMemoryResolver> resolver;
    std::mt19937 rng;

    // Anti-detection features
    bool antiDetectionEnabled;
    bool stealthMode;
    int minDelay;
    int maxDelay;

    // Monitoring
    bool monitoring;
    int monitoringInterval;
    bool debugEnabled;
    std::ofstream debugLog;

    void LogOperation(const std::string& operation, PVOID address, SIZE_T size);
};


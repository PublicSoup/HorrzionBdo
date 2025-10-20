#include "pch.h"
#include "BDO_BotMemoryIntegration.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

BDOBotMemoryIntegration::BDOBotMemoryIntegration() 
    : memory(std::make_unique<BDOAdvancedMemory>()) {
    InitializeAddressMap();
    stats.startTime = std::chrono::steady_clock::now();
}

BDOBotMemoryIntegration::~BDOBotMemoryIntegration() {
    Shutdown();
}

void BDOBotMemoryIntegration::InitializeAddressMap() {
    // Map string names to address pointers for easy access
    addressMap["playerBase"] = {&addresses.playerBase, "Base address of player structure"};
    addressMap["playerHealth"] = {&addresses.playerHealth, "Current player health"};
    addressMap["playerMaxHealth"] = {&addresses.playerMaxHealth, "Maximum player health"};
    addressMap["playerMana"] = {&addresses.playerMana, "Current player mana"};
    addressMap["playerMaxMana"] = {&addresses.playerMaxMana, "Maximum player mana"};
    addressMap["playerStamina"] = {&addresses.playerStamina, "Current player stamina"};
    addressMap["playerMaxStamina"] = {&addresses.playerMaxStamina, "Maximum player stamina"};
    addressMap["playerLevel"] = {&addresses.playerLevel, "Player level"};
    addressMap["playerExp"] = {&addresses.playerExp, "Player experience"};
    addressMap["playerWeight"] = {&addresses.playerWeight, "Current carry weight"};
    addressMap["playerMaxWeight"] = {&addresses.playerMaxWeight, "Maximum carry weight"};
    addressMap["playerSilver"] = {&addresses.playerSilver, "Player silver/money"};
    addressMap["playerX"] = {&addresses.playerX, "Player X coordinate"};
    addressMap["playerY"] = {&addresses.playerY, "Player Y coordinate"};
    addressMap["playerZ"] = {&addresses.playerZ, "Player Z coordinate"};
    addressMap["playerRotation"] = {&addresses.playerRotation, "Player rotation"};
    addressMap["combatState"] = {&addresses.combatState, "Combat state flag"};
    addressMap["targetEntity"] = {&addresses.targetEntity, "Current target entity"};
    addressMap["targetHealth"] = {&addresses.targetHealth, "Target health"};
    addressMap["inventoryBase"] = {&addresses.inventoryBase, "Inventory base address"};
    addressMap["gameState"] = {&addresses.gameState, "Game state"};
}

bool BDOBotMemoryIntegration::Initialize(DWORD processId) {
    if (!memory->Initialize(processId)) {
        std::cerr << "Failed to initialize memory system" << std::endl;
        return false;
    }
    
    std::cout << "Memory system initialized for process ID: " << processId << std::endl;
    return true;
}

bool BDOBotMemoryIntegration::AttachToGame(const std::wstring& processName) {
    if (!memory->AttachToProcess(processName)) {
        std::cerr << "Failed to attach to game process" << std::endl;
        return false;
    }
    
    std::wcout << L"Successfully attached to " << processName << std::endl;
    
    // Try to scan for common addresses
    std::cout << "Scanning for game addresses..." << std::endl;
    ScanForAddresses();
    
    return true;
}

void BDOBotMemoryIntegration::Shutdown() {
    StopMonitoring();
    memory->Shutdown();
}

bool BDOBotMemoryIntegration::ScanForAddresses() {
    std::cout << "Starting comprehensive address scan..." << std::endl;
    stats.totalScans++;
    
    bool success = false;
    
    // Scan for player base using pattern
    if (ScanForAddress("playerBase", BDOPatterns::PLAYER_BASE_PATTERN)) {
        std::cout << "Found player base address" << std::endl;
        
        // Calculate offsets from player base
        if (addresses.playerBase) {
            addresses.playerHealth = (PUCHAR)addresses.playerBase + BDOPatterns::HEALTH_OFFSET;
            addresses.playerMaxHealth = (PUCHAR)addresses.playerBase + BDOPatterns::MAX_HEALTH_OFFSET;
            addresses.playerMana = (PUCHAR)addresses.playerBase + BDOPatterns::MANA_OFFSET;
            addresses.playerMaxMana = (PUCHAR)addresses.playerBase + BDOPatterns::MAX_MANA_OFFSET;
            addresses.playerX = (PUCHAR)addresses.playerBase + BDOPatterns::POSITION_X_OFFSET;
            addresses.playerY = (PUCHAR)addresses.playerBase + BDOPatterns::POSITION_Y_OFFSET;
            addresses.playerZ = (PUCHAR)addresses.playerBase + BDOPatterns::POSITION_Z_OFFSET;
            addresses.playerLevel = (PUCHAR)addresses.playerBase + BDOPatterns::LEVEL_OFFSET;
            addresses.combatState = (PUCHAR)addresses.playerBase + BDOPatterns::COMBAT_STATE_OFFSET;
            
            success = true;
            stats.successfulScans++;
        }
    }
    
    // Scan for UI base
    if (ScanForAddress("uiBase", BDOPatterns::UI_BASE_PATTERN)) {
        std::cout << "Found UI base address" << std::endl;
        success = true;
    }
    
    // Scan for inventory base
    if (ScanForAddress("inventoryBase", BDOPatterns::INVENTORY_BASE_PATTERN)) {
        std::cout << "Found inventory base address" << std::endl;
        success = true;
    }
    
    return success;
}

bool BDOBotMemoryIntegration::ScanForAddress(const std::string& addressName, const std::string& pattern) {
    // Convert pattern string to bytes
    std::vector<BYTE> patternBytes;
    std::vector<BYTE> maskBytes;
    
    std::istringstream iss(pattern);
    std::string byte;
    while (iss >> byte) {
        if (byte == "??") {
            patternBytes.push_back(0);
            maskBytes.push_back(0);
        } else {
            patternBytes.push_back(static_cast<BYTE>(std::stoul(byte, nullptr, 16)));
            maskBytes.push_back(0xFF);
        }
    }
    
    // Perform pattern scan
    memory->AddScan(addressName, patternBytes, maskBytes);
    if (memory->PerformScan(addressName)) {
        auto results = memory->GetScanResults(addressName);
        if (!results.empty()) {
            UpdateAddress(addressName, results[0]);
            return true;
        }
    }
    
    return false;
}

bool BDOBotMemoryIntegration::UpdateAddress(const std::string& name, PVOID address) {
    auto it = addressMap.find(name);
    if (it != addressMap.end()) {
        *it->second.first = address;
        std::cout << "Updated " << name << " to 0x" << std::hex << address << std::endl;
        return true;
    }
    return false;
}

PVOID BDOBotMemoryIntegration::GetAddress(const std::string& name) {
    auto it = addressMap.find(name);
    if (it != addressMap.end()) {
        return *it->second.first;
    }
    return nullptr;
}

bool BDOBotMemoryIntegration::SetPlayerHealthAddress(PVOID address) {
    if (ValidateAddress(address)) {
        addresses.playerHealth = address;
        std::cout << "Set player health address to 0x" << std::hex << address << std::endl;
        return true;
    }
    return false;
}

bool BDOBotMemoryIntegration::SetPlayerManaAddress(PVOID address) {
    if (ValidateAddress(address)) {
        addresses.playerMana = address;
        std::cout << "Set player mana address to 0x" << std::hex << address << std::endl;
        return true;
    }
    return false;
}

bool BDOBotMemoryIntegration::SetPlayerPositionAddresses(PVOID x, PVOID y, PVOID z) {
    if (ValidateAddress(x) && ValidateAddress(y) && ValidateAddress(z)) {
        addresses.playerX = x;
        addresses.playerY = y;
        addresses.playerZ = z;
        std::cout << "Set player position addresses" << std::endl;
        return true;
    }
    return false;
}

bool BDOBotMemoryIntegration::SetCombatAddresses(PVOID combatState, PVOID target) {
    if (ValidateAddress(combatState)) {
        addresses.combatState = combatState;
        addresses.targetEntity = target;
        std::cout << "Set combat addresses" << std::endl;
        return true;
    }
    return false;
}

bool BDOBotMemoryIntegration::SetInventoryBaseAddress(PVOID address) {
    if (ValidateAddress(address)) {
        addresses.inventoryBase = address;
        std::cout << "Set inventory base address to 0x" << std::hex << address << std::endl;
        return true;
    }
    return false;
}

void BDOBotMemoryIntegration::RegisterAction(const std::string& name, std::function<void()> action, 
                                            std::chrono::milliseconds cooldown, int priority) {
    BotAction botAction;
    botAction.name = name;
    botAction.action = action;
    botAction.cooldown = cooldown;
    botAction.priority = priority;
    botAction.lastExecuted = std::chrono::steady_clock::now() - cooldown; // Allow immediate execution
    
    actions[name] = botAction;
    std::cout << "Registered action: " << name << std::endl;
}

void BDOBotMemoryIntegration::UnregisterAction(const std::string& name) {
    actions.erase(name);
}

void BDOBotMemoryIntegration::EnableAction(const std::string& name, bool enable) {
    auto it = actions.find(name);
    if (it != actions.end()) {
        it->second.enabled = enable;
    }
}

void BDOBotMemoryIntegration::ExecuteAction(const std::string& name) {
    auto it = actions.find(name);
    if (it != actions.end() && it->second.enabled) {
        auto now = std::chrono::steady_clock::now();
        if (now - it->second.lastExecuted >= it->second.cooldown) {
            it->second.action();
            it->second.lastExecuted = now;
            stats.actionsExecuted++;
        }
    }
}

void BDOBotMemoryIntegration::AddTrigger(const std::string& name, PVOID address, ValueType type,
                                        MemoryCondition condition, const std::vector<std::string>& actionNames,
                                        bool continuous) {
    MemoryTrigger trigger;
    trigger.name = name;
    trigger.address = address;
    trigger.valueType = type;
    trigger.condition = condition;
    trigger.actionNames = actionNames;
    trigger.continuous = continuous;
    trigger.enabled = true;
    
    triggers[name] = trigger;
    std::cout << "Added trigger: " << name << std::endl;
}

void BDOBotMemoryIntegration::AddHealthTrigger(float threshold, const std::vector<std::string>& actions) {
    if (!addresses.playerHealth || !addresses.playerMaxHealth) {
        std::cerr << "Health addresses not set" << std::endl;
        return;
    }
    
    MemoryCondition condition;
    condition.type = MemoryCondition::LESS_THAN;
    condition.value1 = threshold;
    
    AddTrigger("LowHealthTrigger", addresses.playerHealth, ValueType::Dword, condition, actions);
}

void BDOBotMemoryIntegration::AddManaTrigger(float threshold, const std::vector<std::string>& actions) {
    if (!addresses.playerMana || !addresses.playerMaxMana) {
        std::cerr << "Mana addresses not set" << std::endl;
        return;
    }
    
    MemoryCondition condition;
    condition.type = MemoryCondition::LESS_THAN;
    condition.value1 = threshold;
    
    AddTrigger("LowManaTrigger", addresses.playerMana, ValueType::Dword, condition, actions);
}

void BDOBotMemoryIntegration::AddCombatTrigger(bool inCombat, const std::vector<std::string>& actions) {
    if (!addresses.combatState) {
        std::cerr << "Combat state address not set" << std::endl;
        return;
    }
    
    MemoryCondition condition;
    condition.type = inCombat ? MemoryCondition::EQUAL_TO : MemoryCondition::NOT_EQUAL_TO;
    condition.value1 = inCombat ? 1 : 0;
    
    AddTrigger(inCombat ? "InCombatTrigger" : "OutOfCombatTrigger", 
               addresses.combatState, ValueType::Dword, condition, actions, true);
}

void BDOBotMemoryIntegration::RemoveTrigger(const std::string& name) {
    triggers.erase(name);
}

void BDOBotMemoryIntegration::EnableTrigger(const std::string& name, bool enable) {
    auto it = triggers.find(name);
    if (it != triggers.end()) {
        it->second.enabled = enable;
    }
}

bool BDOBotMemoryIntegration::StartMonitoring(std::chrono::milliseconds interval) {
    if (isRunning) {
        return false;
    }
    
    isRunning = true;
    monitorThread = std::thread([this, interval]() {
        while (isRunning) {
            MonitorLoop();
            std::this_thread::sleep_for(interval);
        }
    });
    
    std::cout << "Started memory monitoring" << std::endl;
    return true;
}

void BDOBotMemoryIntegration::StopMonitoring() {
    if (isRunning) {
        isRunning = false;
        if (monitorThread.joinable()) {
            monitorThread.join();
        }
        std::cout << "Stopped memory monitoring" << std::endl;
    }
}

void BDOBotMemoryIntegration::MonitorLoop() {
    ProcessTriggers();
    ExecuteQueuedActions();
}

void BDOBotMemoryIntegration::ProcessTriggers() {
    for (auto& [name, trigger] : triggers) {
        if (!trigger.enabled || !trigger.address) {
            continue;
        }
        
        // Read current value
        std::variant<BYTE, WORD, DWORD, QWORD, float, double, std::string, std::vector<BYTE>> value;
        if (!memory->GetValue(trigger.address, trigger.valueType, value)) {
            continue;
        }
        
        // Check condition
        bool conditionMet = false;
        
        std::visit([&](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_arithmetic_v<T>) {
                // Special handling for percentage-based triggers
                if (name == "LowHealthTrigger" && addresses.playerMaxHealth) {
                    int maxHealth;
                    memory->ReadMemory(addresses.playerMaxHealth, &maxHealth, sizeof(int));
                    float percentage = static_cast<float>(val) / maxHealth;
                    conditionMet = trigger.condition.Check(percentage);
                } else if (name == "LowManaTrigger" && addresses.playerMaxMana) {
                    int maxMana;
                    memory->ReadMemory(addresses.playerMaxMana, &maxMana, sizeof(int));
                    float percentage = static_cast<float>(val) / maxMana;
                    conditionMet = trigger.condition.Check(percentage);
                } else {
                    conditionMet = trigger.condition.Check(val);
                }
            }
        }, value);
        
        // Execute actions if condition is met
        if (conditionMet) {
            auto now = std::chrono::steady_clock::now();
            if (!trigger.continuous || now - trigger.lastTriggered > std::chrono::seconds(1)) {
                for (const auto& actionName : trigger.actionNames) {
                    std::lock_guard<std::mutex> lock(actionMutex);
                    actionQueue.push(actionName);
                }
                trigger.lastTriggered = now;
                stats.triggersActivated++;
            }
        }
    }
}

void BDOBotMemoryIntegration::ExecuteQueuedActions() {
    std::lock_guard<std::mutex> lock(actionMutex);
    
    // Sort actions by priority
    std::vector<std::string> sortedActions;
    while (!actionQueue.empty()) {
        sortedActions.push_back(actionQueue.front());
        actionQueue.pop();
    }
    
    std::sort(sortedActions.begin(), sortedActions.end(), [this](const std::string& a, const std::string& b) {
        auto itA = actions.find(a);
        auto itB = actions.find(b);
        if (itA != actions.end() && itB != actions.end()) {
            return itA->second.priority > itB->second.priority;
        }
        return false;
    });
    
    // Execute sorted actions
    for (const auto& actionName : sortedActions) {
        ExecuteAction(actionName);
    }
}

bool BDOBotMemoryIntegration::ValidateAddress(PVOID address) {
    if (!address) {
        return false;
    }
    
    // Try to read a byte from the address to validate it
    BYTE testByte;
    return memory->ReadMemory(address, &testByte, sizeof(testByte));
}

bool BDOBotMemoryIntegration::ReadPlayerStats(int& health, int& maxHealth, int& mana, int& maxMana) {
    if (!addresses.playerHealth || !addresses.playerMaxHealth || 
        !addresses.playerMana || !addresses.playerMaxMana) {
        return false;
    }
    
    bool success = true;
    success &= memory->ReadMemory(addresses.playerHealth, &health, sizeof(int));
    success &= memory->ReadMemory(addresses.playerMaxHealth, &maxHealth, sizeof(int));
    success &= memory->ReadMemory(addresses.playerMana, &mana, sizeof(int));
    success &= memory->ReadMemory(addresses.playerMaxMana, &maxMana, sizeof(int));
    
    return success;
}

bool BDOBotMemoryIntegration::ReadPlayerPosition(float& x, float& y, float& z) {
    if (!addresses.playerX || !addresses.playerY || !addresses.playerZ) {
        return false;
    }
    
    bool success = true;
    success &= memory->ReadMemory(addresses.playerX, &x, sizeof(float));
    success &= memory->ReadMemory(addresses.playerY, &y, sizeof(float));
    success &= memory->ReadMemory(addresses.playerZ, &z, sizeof(float));
    
    return success;
}

bool BDOBotMemoryIntegration::ReadCombatState(bool& inCombat, PVOID& target) {
    if (!addresses.combatState) {
        return false;
    }
    
    int combatFlag;
    if (memory->ReadMemory(addresses.combatState, &combatFlag, sizeof(int))) {
        inCombat = (combatFlag != 0);
        
        if (addresses.targetEntity) {
            memory->ReadMemory(addresses.targetEntity, &target, sizeof(PVOID));
        }
        
        return true;
    }
    
    return false;
}

float BDOBotMemoryIntegration::GetPlayerHealthPercentage() {
    int health, maxHealth;
    if (ReadPlayerStats(health, maxHealth, health, maxHealth)) {
        return static_cast<float>(health) / maxHealth;
    }
    return 1.0f;
}

float BDOBotMemoryIntegration::GetPlayerManaPercentage() {
    int health, maxHealth, mana, maxMana;
    if (ReadPlayerStats(health, maxHealth, mana, maxMana)) {
        return static_cast<float>(mana) / maxMana;
    }
    return 1.0f;
}

void BDOBotMemoryIntegration::SetupCombatBot() {
    std::cout << "Setting up combat bot..." << std::endl;
    
    // Register combat actions
    RegisterAction("UseHealthPotion", []() {
        std::cout << "[ACTION] Using health potion" << std::endl;
        // Simulate key press for health potion (key 1)
        keybd_event('1', 0, 0, 0);
        Sleep(50);
        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(5), 10);
    
    RegisterAction("UseManaPotion", []() {
        std::cout << "[ACTION] Using mana potion" << std::endl;
        // Simulate key press for mana potion (key 2)
        keybd_event('2', 0, 0, 0);
        Sleep(50);
        keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(5), 9);
    
    RegisterAction("AttackTarget", []() {
        std::cout << "[ACTION] Attacking target" << std::endl;
        // Simulate left mouse click
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        Sleep(100);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }, std::chrono::milliseconds(500), 5);
    
    // Add triggers
    AddHealthTrigger(0.5f, {"UseHealthPotion"});  // Use potion when health < 50%
    AddManaTrigger(0.3f, {"UseManaPotion"});      // Use potion when mana < 30%
    AddCombatTrigger(true, {"AttackTarget"});      // Attack when in combat
}

void BDOBotMemoryIntegration::PrintStatistics() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - stats.startTime);
    
    std::cout << "\n=== Bot Statistics ===" << std::endl;
    std::cout << "Runtime: " << duration.count() << " seconds" << std::endl;
    std::cout << "Total Scans: " << stats.totalScans << std::endl;
    std::cout << "Successful Scans: " << stats.successfulScans << std::endl;
    std::cout << "Triggers Activated: " << stats.triggersActivated << std::endl;
    std::cout << "Actions Executed: " << stats.actionsExecuted << std::endl;
    std::cout << "=====================\n" << std::endl;
}

bool BDOBotMemoryIntegration::LoadAddressesFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string name;
        std::string addressStr;
        
        if (std::getline(iss, name, '=') && std::getline(iss, addressStr)) {
            try {
                PVOID address = reinterpret_cast<PVOID>(std::stoull(addressStr, nullptr, 16));
                UpdateAddress(name, address);
            } catch (...) {
                std::cerr << "Failed to parse address: " << line << std::endl;
            }
        }
    }
    
    file.close();
    std::cout << "Loaded addresses from " << filename << std::endl;
    return true;
}

bool BDOBotMemoryIntegration::SaveAddressesToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "# BDO Bot Memory Addresses" << std::endl;
    file << "# Generated: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
    file << std::endl;
    
    for (const auto& [name, pair] : addressMap) {
        PVOID address = *pair.first;
        if (address) {
            file << name << "=0x" << std::hex << reinterpret_cast<uintptr_t>(address) << std::endl;
        }
    }
    
    file.close();
    std::cout << "Saved addresses to " << filename << std::endl;
    return true;
}

// AddressScanner implementation
bool AddressScanner::ScanForHealth(int currentHealth) {
    std::cout << "Scanning for health value: " << currentHealth << std::endl;
    
    if (integration->memory->ScanForValue(ValueType::Dword, std::to_string(currentHealth))) {
        auto results = integration->memory->GetScanResults();
        std::cout << "Found " << results.size() << " potential health addresses" << std::endl;
        
        if (!results.empty()) {
            // Use the first result for now
            integration->SetPlayerHealthAddress(results[0].address);
            return true;
        }
    }
    
    return false;
}

bool AddressScanner::ScanForPlayerBase() {
    // Convert pattern to bytes
    std::vector<BYTE> pattern = {0x48, 0x8B, 0x05};  // mov rax, [rip+offset]
    std::vector<BYTE> mask = {0xFF, 0xFF, 0xFF};
    
    integration->memory->AddScan("PlayerBaseScan", pattern, mask);
    if (integration->memory->PerformScan("PlayerBaseScan")) {
        auto results = integration->memory->GetScanResults("PlayerBaseScan");
        if (!results.empty()) {
            integration->UpdateAddress("playerBase", results[0]);
            return true;
        }
    }
    
    return false;
}

void AddressScanner::PrintFoundAddresses() {
    std::cout << "\n=== Found Addresses ===" << std::endl;
    
    for (const auto& [name, pair] : integration->addressMap) {
        PVOID address = *pair.first;
        if (address) {
            std::cout << std::setw(20) << std::left << name << ": 0x" 
                     << std::hex << reinterpret_cast<uintptr_t>(address) 
                     << " - " << pair.second << std::endl;
        }
    }
    
    std::cout << "=====================\n" << std::endl;
}

// BDO Stealth Bot - Main Application
// Disguised as Windows System Monitor to avoid detection

#include "pch.h"
#include "BDO_StealthGUI.h"
#include "BDO_MemoryResolver.h"
#include "BDO_AdvancedMemory.h"
#include "BDO_DriverInterface.h"
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

// External kernel interface
extern BdoKernelInterface g_KernelInterface;

// Global variables
std::unique_ptr<BDOStealthGUI> g_GUI;
std::unique_ptr<BDOAdvancedMemory> g_Memory;
std::unique_ptr<BDOMemoryResolver> g_Resolver;
bool g_BotRunning = false;
bool g_StealthMode = true;

// Bot configuration
struct BotConfig {
    bool enableAutoHealing;
    bool enableAutoMana;
    bool enableAutoSelling;
    bool enableAutoCombat;
    bool enableStealthMode;
    bool enableAntiDetection;
    int minHealthThreshold;
    int minManaThreshold;
    float maxWeightThreshold;
    int potionCooldown;
    int skillCooldown;
    int updateInterval;
} g_Config;

// Bot state
struct BotState {
    int currentHealth;
    int maxHealth;
    int currentMana;
    int maxMana;
    float currentWeight;
    float maxWeight;
    int combatState;
    int gameState;
    bool inCombat;
    bool weightFull;
    bool needsHealing;
    bool needsMana;
    std::chrono::steady_clock::time_point lastHealthCheck;
    std::chrono::steady_clock::time_point lastManaCheck;
    std::chrono::steady_clock::time_point lastWeightCheck;
    std::chrono::steady_clock::time_point lastCombatCheck;
} g_State;

// Function prototypes
bool InitializeBot();
void ShutdownBot();
void RunBot();
void UpdateBotState();
void ProcessBotLogic();
void CreateBotGUI();
void UpdateBotGUI();
bool ConnectToDriver();
void DisconnectFromDriver();
void AddStealthDelay();
void LogBotMessage(const std::string& message);

int bot_main() {
    // Initialize console for debugging
    // AllocConsole();
    // freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    // freopen_s((FILE**)stdin, "CONIN$", "r", stdin);
    // freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
    
    std::cout << "BDO Stealth Bot v2.0 (Kernel Edition)" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Initialize kernel interface
    std::cout << "Connecting to kernel driver..." << std::endl;
    if (g_KernelInterface.Connect()) {
        std::cout << "[SUCCESS] Kernel driver connected - memory operations will bypass anti-cheat" << std::endl;
    } else {
        std::cout << "[WARNING] Kernel driver not found - using fallback mode (may be detected)" << std::endl;
        std::cout << "Run the manual mapper (--map-driver) to load the kernel driver" << std::endl;
    }
    
    // Initialize bot
    std::cout << "Initializing bot systems..." << std::endl;
    if (!InitializeBot()) {
        std::cout << "Failed to initialize bot systems" << std::endl;
        MessageBoxA(nullptr, "Failed to initialize bot systems", "Error", MB_OK);
        return 1;
    }
    std::cout << "Bot systems initialized successfully" << std::endl;
    
    // Create GUI
    std::cout << "Creating GUI..." << std::endl;
    g_GUI = std::make_unique<BDOStealthGUI>();
    if (!g_GUI->Initialize(GetModuleHandle(nullptr), SW_SHOW)) {
        std::cout << "Failed to initialize GUI" << std::endl;
        MessageBoxA(nullptr, "Failed to initialize GUI", "Error", MB_OK);
        ShutdownBot();
        return 1;
    }
    std::cout << "GUI initialized successfully" << std::endl;
    MessageBoxA(nullptr, "GUI initialized successfully!", "Success", MB_OK);
    
    // Create bot GUI
    CreateBotGUI();
    
    // Note: Kernel driver functionality removed for user-mode build
    
    // Start bot thread
    std::thread botThread(RunBot);
    
    // Run GUI
    g_GUI->Run();
    
    // Stop bot
    g_BotRunning = false;
    if (botThread.joinable()) {
        botThread.join();
    }
    
    // Cleanup
    ShutdownBot();
    
    return 0;
}

bool InitializeBot() {
    // Initialize configuration
    g_Config.enableAutoHealing = true;
    g_Config.enableAutoMana = true;
    g_Config.enableAutoSelling = true;
    g_Config.enableAutoCombat = true;
    g_Config.enableStealthMode = true;
    g_Config.enableAntiDetection = true;
    g_Config.minHealthThreshold = 60;
    g_Config.minManaThreshold = 40;
    g_Config.maxWeightThreshold = 95.0f;
    g_Config.potionCooldown = 3000; // 3 seconds
    g_Config.skillCooldown = 2000;  // 2 seconds
    g_Config.updateInterval = 100;  // 100ms
    
    // Initialize state
    g_State.currentHealth = 0;
    g_State.maxHealth = 0;
    g_State.currentMana = 0;
    g_State.maxMana = 0;
    g_State.currentWeight = 0.0f;
    g_State.maxWeight = 0.0f;
    g_State.combatState = 0;
    g_State.gameState = 0;
    g_State.inCombat = false;
    g_State.weightFull = false;
    g_State.needsHealing = false;
    g_State.needsMana = false;
    g_State.lastHealthCheck = std::chrono::steady_clock::now();
    g_State.lastManaCheck = std::chrono::steady_clock::now();
    g_State.lastWeightCheck = std::chrono::steady_clock::now();
    g_State.lastCombatCheck = std::chrono::steady_clock::now();
    
    // Initialize memory system
    g_Memory = std::make_unique<BDOAdvancedMemory>();
    g_Resolver = std::make_unique<BDOMemoryResolver>();
    
    // Enable anti-detection
    if (g_Config.enableAntiDetection) {
        g_Memory->EnableAntiDetection(true);
        g_Memory->EnableStealthMode(true);
        g_Memory->SetOperationDelay(10, 50);
    }
    
    // Attach to BDO process (simulated for demo)
    if (!g_Memory->AttachToProcess(L"BlackDesert64.exe")) {
        std::cout << "BDO process not found. Running in demo mode." << std::endl;
        // Continue anyway for demo purposes
    }
    
    // Resolve memory addresses (simulated for demo)
    if (!g_Resolver->AttachToProcess(L"BlackDesert64.exe")) {
        std::cout << "Failed to attach memory resolver. Running in demo mode." << std::endl;
        // Continue anyway for demo purposes
    }
    
    if (!g_Resolver->ResolveAllAddresses()) {
        std::cout << "Failed to resolve memory addresses. Running in demo mode." << std::endl;
        // Continue anyway for demo purposes
    }
    
    g_BotRunning = true;
    
    LogBotMessage("Bot initialized successfully");
    return true;
}

void ShutdownBot() {
    g_BotRunning = false;
    
    // Note: Kernel driver functionality removed for user-mode build
    
    // Cleanup memory system
    if (g_Memory) {
        g_Memory->DetachFromProcess();
        g_Memory.reset();
    }
    
    if (g_Resolver) {
        g_Resolver->DetachFromProcess();
        g_Resolver.reset();
    }
    
    // Cleanup GUI
    if (g_GUI) {
        g_GUI->Shutdown();
        g_GUI.reset();
    }
    
    LogBotMessage("Bot shutdown complete");
}

void RunBot() {
    LogBotMessage("Bot thread started");
    
    while (g_BotRunning) {
        try {
            // Add stealth delay
            AddStealthDelay();
            
            // Update bot state
            UpdateBotState();
            
            // Process bot logic
            ProcessBotLogic();
            
            // Update GUI
            if (g_GUI) {
                UpdateBotGUI();
            }
            
            // Sleep for update interval
            std::this_thread::sleep_for(std::chrono::milliseconds(g_Config.updateInterval));
            
        } catch (const std::exception& e) {
            LogBotMessage("Error in bot thread: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    LogBotMessage("Bot thread stopped");
}

void UpdateBotState() {
    if (!g_Memory) {
        return;
    }
    
    // Read player health
    if (g_Memory->ReadPlayerHealth(g_State.currentHealth, g_State.maxHealth)) {
        g_State.needsHealing = (g_State.currentHealth < g_Config.minHealthThreshold);
    }
    
    // Read player mana
    if (g_Memory->ReadPlayerMana(g_State.currentMana, g_State.maxMana)) {
        g_State.needsMana = (g_State.currentMana < g_Config.minManaThreshold);
    }
    
    // Read player weight
    if (g_Memory->ReadPlayerWeight(g_State.currentWeight, g_State.maxWeight)) {
        g_State.weightFull = (g_State.currentWeight >= g_Config.maxWeightThreshold);
    }
    
    // Read combat state
    g_Memory->ReadCombatState(g_State.combatState);
    g_State.inCombat = (g_State.combatState > 0);
    
    // Read game state
    g_Memory->ReadGameState(g_State.gameState);
}

void ProcessBotLogic() {
    if (!g_Memory) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    
    // Health management
    if (g_Config.enableAutoHealing && g_State.needsHealing) {
        auto healthElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_State.lastHealthCheck).count();
        if (healthElapsed >= g_Config.potionCooldown) {
            // Use health potion (F1 key)
            INPUT input = {0};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = VK_F1;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
            
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
            
            g_State.lastHealthCheck = now;
            LogBotMessage("Used health potion");
        }
    }
    
    // Mana management
    if (g_Config.enableAutoMana && g_State.needsMana) {
        auto manaElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_State.lastManaCheck).count();
        if (manaElapsed >= g_Config.potionCooldown) {
            // Use mana potion (F2 key)
            INPUT input = {0};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = VK_F2;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
            
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
            
            g_State.lastManaCheck = now;
            LogBotMessage("Used mana potion");
        }
    }
    
    // Weight management
    if (g_Config.enableAutoSelling && g_State.weightFull) {
        auto weightElapsed = std::chrono::duration_cast<std::chrono::seconds>(now - g_State.lastWeightCheck).count();
        if (weightElapsed >= 60) { // Check every minute
            // Open inventory (I key)
            INPUT input = {0};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = 'I';
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
            
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
            
            // Wait for inventory to open
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            // Close inventory (ESC key)
            input.ki.wVk = VK_ESCAPE;
            input.ki.dwFlags = 0;
            SendInput(1, &input, sizeof(INPUT));
            
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1, &input, sizeof(INPUT));
            
            g_State.lastWeightCheck = now;
            LogBotMessage("Processed weight management");
        }
    }
    
    // Combat management
    if (g_Config.enableAutoCombat && g_State.inCombat) {
        auto combatElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_State.lastCombatCheck).count();
        if (combatElapsed >= g_Config.skillCooldown) {
            // Perform combat rotation (Q, E, R, F)
            std::vector<WORD> skills = {'Q', 'E', 'R', 'F'};
            
            for (WORD skill : skills) {
                INPUT input = {0};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = skill;
                input.ki.dwFlags = 0;
                SendInput(1, &input, sizeof(INPUT));
                
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
                
                // Add random delay between skills
                std::uniform_int_distribution<int> delayDist(200, 800);
                std::mt19937 rng(std::random_device{}());
                std::this_thread::sleep_for(std::chrono::milliseconds(delayDist(rng)));
            }
            
            g_State.lastCombatCheck = now;
            LogBotMessage("Performed combat rotation");
        }
    }
}

void CreateBotGUI() {
    if (!g_GUI) {
        return;
    }
    
    // Create system monitor GUI (disguise)
    g_GUI->CreateSystemMonitorGUI();
    
    // Add bot-specific controls
    g_GUI->AddButton("startBotBtn", "Start Monitoring", 10, 250, 120, 30, []() {
        g_BotRunning = true;
        LogBotMessage("Bot started");
    });
    
    g_GUI->AddButton("stopBotBtn", "Stop Monitoring", 140, 250, 120, 30, []() {
        g_BotRunning = false;
        LogBotMessage("Bot stopped");
    });
    
    g_GUI->AddButton("settingsBtn", "Settings", 270, 250, 120, 30, []() {
        LogBotMessage("Settings clicked");
    });
    
    // Add status displays
    g_GUI->AddText("healthStatus", "Health: 0/0", 10, 300, 200, 20, 0xFFFFFFFF);
    g_GUI->AddText("manaStatus", "Mana: 0/0", 10, 320, 200, 20, 0xFFFFFFFF);
    g_GUI->AddText("weightStatus", "Weight: 0/0", 10, 340, 200, 20, 0xFFFFFFFF);
    g_GUI->AddText("combatStatus", "Combat: Inactive", 10, 360, 200, 20, 0xFFFFFFFF);
    g_GUI->AddText("botStatus", "Bot Status: Stopped", 10, 380, 200, 20, 0xFFFFFFFF);
    
    // Add progress bars
    g_GUI->AddProgressBar("healthBar", "Health", 10, 410, 200, 20);
    g_GUI->AddProgressBar("manaBar", "Mana", 10, 440, 200, 20);
    g_GUI->AddProgressBar("weightBar", "Weight", 10, 470, 200, 20);
    
    // Add checkboxes
    g_GUI->AddCheckbox("autoHealing", "Auto Healing", 10, 500, 200, 20, [](bool checked) {
        g_Config.enableAutoHealing = checked;
        LogBotMessage("Auto healing: " + std::string(checked ? "Enabled" : "Disabled"));
    });
    
    g_GUI->AddCheckbox("autoMana", "Auto Mana", 10, 520, 200, 20, [](bool checked) {
        g_Config.enableAutoMana = checked;
        LogBotMessage("Auto mana: " + std::string(checked ? "Enabled" : "Disabled"));
    });
    
    g_GUI->AddCheckbox("autoSelling", "Auto Selling", 10, 540, 200, 20, [](bool checked) {
        g_Config.enableAutoSelling = checked;
        LogBotMessage("Auto selling: " + std::string(checked ? "Enabled" : "Disabled"));
    });
    
    g_GUI->AddCheckbox("autoCombat", "Auto Combat", 10, 560, 200, 20, [](bool checked) {
        g_Config.enableAutoCombat = checked;
        LogBotMessage("Auto combat: " + std::string(checked ? "Enabled" : "Disabled"));
    });
    
    g_GUI->AddCheckbox("stealthMode", "Stealth Mode", 10, 580, 200, 20, [](bool checked) {
        g_Config.enableStealthMode = checked;
        g_StealthMode = checked;
        if (g_Memory) {
            g_Memory->EnableStealthMode(checked);
        }
        LogBotMessage("Stealth mode: " + std::string(checked ? "Enabled" : "Disabled"));
    });
    
    // Add sliders
    g_GUI->AddSlider("healthThreshold", "Health Threshold", 10, 610, 200, 30, 0.0f, 100.0f, [](float value) {
        g_Config.minHealthThreshold = (int)value;
        LogBotMessage("Health threshold set to: " + std::to_string((int)value));
    });
    
    g_GUI->AddSlider("manaThreshold", "Mana Threshold", 10, 650, 200, 30, 0.0f, 100.0f, [](float value) {
        g_Config.minManaThreshold = (int)value;
        LogBotMessage("Mana threshold set to: " + std::to_string((int)value));
    });
    
    g_GUI->AddSlider("weightThreshold", "Weight Threshold", 10, 690, 200, 30, 0.0f, 100.0f, [](float value) {
        g_Config.maxWeightThreshold = value;
        LogBotMessage("Weight threshold set to: " + std::to_string(value));
    });
    
    // Set initial values
    g_GUI->SetCheckboxChecked("autoHealing", g_Config.enableAutoHealing);
    g_GUI->SetCheckboxChecked("autoMana", g_Config.enableAutoMana);
    g_GUI->SetCheckboxChecked("autoSelling", g_Config.enableAutoSelling);
    g_GUI->SetCheckboxChecked("autoCombat", g_Config.enableAutoCombat);
    g_GUI->SetCheckboxChecked("stealthMode", g_Config.enableStealthMode);
    g_GUI->SetSliderValue("healthThreshold", (float)g_Config.minHealthThreshold);
    g_GUI->SetSliderValue("manaThreshold", (float)g_Config.minManaThreshold);
    g_GUI->SetSliderValue("weightThreshold", g_Config.maxWeightThreshold);
}

void UpdateBotGUI() {
    if (!g_GUI) {
        return;
    }
    
    // Update status text
    std::string healthText = "Health: " + std::to_string(g_State.currentHealth) + "/" + std::to_string(g_State.maxHealth);
    g_GUI->SetText("healthStatus", healthText);
    
    std::string manaText = "Mana: " + std::to_string(g_State.currentMana) + "/" + std::to_string(g_State.maxMana);
    g_GUI->SetText("manaStatus", manaText);
    
    std::string weightText = "Weight: " + std::to_string(g_State.currentWeight) + "/" + std::to_string(g_State.maxWeight);
    g_GUI->SetText("weightStatus", weightText);
    
    std::string combatText = "Combat: " + std::string(g_State.inCombat ? "Active" : "Inactive");
    g_GUI->SetText("combatStatus", combatText);
    
    std::string botText = "Bot Status: " + std::string(g_BotRunning ? "Running" : "Stopped");
    g_GUI->SetText("botStatus", botText);
    
    // Update progress bars
    if (g_State.maxHealth > 0) {
        float healthPercent = (float)g_State.currentHealth / g_State.maxHealth * 100.0f;
        g_GUI->SetProgressBarProgress("healthBar", healthPercent);
    }
    
    if (g_State.maxMana > 0) {
        float manaPercent = (float)g_State.currentMana / g_State.maxMana * 100.0f;
        g_GUI->SetProgressBarProgress("manaBar", manaPercent);
    }
    
    if (g_State.maxWeight > 0) {
        float weightPercent = g_State.currentWeight / g_State.maxWeight * 100.0f;
        g_GUI->SetProgressBarProgress("weightBar", weightPercent);
    }
    
    // Update system monitor GUI
    g_GUI->UpdateSystemMonitorGUI();
}

// Driver functions removed for user-mode build

void AddStealthDelay() {
    if (g_StealthMode) {
        std::uniform_int_distribution<int> delayDist(10, 50);
        std::mt19937 rng(std::random_device{}());
        std::this_thread::sleep_for(std::chrono::milliseconds(delayDist(rng)));
    }
}

void LogBotMessage(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::string timestamp = std::to_string(time_t);
    std::string logMessage = "[" + timestamp + "] " + message;
    
    std::cout << logMessage << std::endl;
    
    // Also update GUI if available
    if (g_GUI) {
        // This would update a log display in the GUI
        // For now, we'll just print to console
    }
}

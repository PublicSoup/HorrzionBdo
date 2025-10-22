#include "pch.h"
#include "BDO_StealthGUI.h"
#include "BDO_MemoryResolver.h"
#include "BDO_AdvancedMemory.h"
#include "BDO_DriverInterface.h"
#include "BDO_RTCore64_Driver.h"
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

// External kernel interfaces
extern BdoKernelInterface g_KernelInterface;
extern RTCore64Driver g_RTCore64;

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
    
    // Initialize RTCore64 driver (highest priority)
    std::cout << "Connecting to RTCore64 driver..." << std::endl;
    if (g_RTCore64.Connect()) {
        std::cout << "[SUCCESS] RTCore64 driver connected - memory operations will bypass anti-cheat" << std::endl;
    } else {
        std::cout << "[WARNING] RTCore64 driver not found - trying alternative drivers" << std::endl;
        
        // Fall back to original kernel driver
        std::cout << "Connecting to kernel driver..." << std::endl;
        if (g_KernelInterface.Connect()) {
            std::cout << "[SUCCESS] Kernel driver connected - memory operations will bypass anti-cheat" << std::endl;
        } else {
            std::cout << "[WARNING] No kernel drivers found - using fallback mode (may be detected)" << std::endl;
            std::cout << "Run the RTCore64 test (--rtcore64) to check driver status" << std::endl;
        }
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
    std::cout << "Creating bot GUI..." << std::endl;
    CreateBotGUI();
    
    // Start bot thread
    std::cout << "Starting bot thread..." << std::endl;
    std::thread botThread(RunBot);
    botThread.detach();
    
    // Initialize default configuration
    g_Config.enableAutoHealing = true;
    g_Config.enableAutoMana = true;
    g_Config.enableAutoSelling = false;
    g_Config.enableAutoCombat = false;
    g_Config.enableStealthMode = true;
    g_Config.enableAntiDetection = true;
    g_Config.minHealthThreshold = 50;
    g_Config.minManaThreshold = 30;
    g_Config.maxWeightThreshold = 80.0f;
    g_Config.potionCooldown = 3000;
    g_Config.skillCooldown = 1000;
    g_Config.updateInterval = 100;
    
    // Run message loop
    if (g_GUI) {
        g_GUI->Run();
    } else {
        std::cout << "GUI not initialized, running in console mode" << std::endl;
        std::cout << "Press Ctrl+C to exit" << std::endl;
        while (g_BotRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    // Cleanup
    ShutdownBot();
    
    return 0;
}

// Main entry point
int main(int argc, char* argv[]) {
    // Process command line arguments
    bool useRTCore64 = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--rtcore64") == 0) {
            useRTCore64 = true;
        }
    }
    
    // Process RTCore64 flags
    if (useRTCore64) {
        // Check for comprehensive test flag
        bool comprehensiveTest = false;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--comprehensive") == 0) {
                comprehensiveTest = true;
                break;
            }
        }
        
        // Run the appropriate test
        if (comprehensiveTest) {
            return rtcore64_comprehensive_test_main();
        } else {
            return rtcore64_main();
        }
    }
    std::cout << "================================================" << std::endl;
    std::cout << "    BDO Stealth Bot - Initializing..." << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
    
    // Initialize bot components
    try {
        g_Memory = std::make_unique<BDOAdvancedMemory>();
        g_Resolver = std::make_unique<BDOMemoryResolver>();
        
        std::cout << "[OK] Memory systems initialized" << std::endl;
        
        // Attach to BDO process
        if (!g_Memory->AttachToProcess(L"BlackDesert64.exe")) {
            std::cerr << std::endl;
            std::cerr << "========================================" << std::endl;
            std::cerr << "  ATTACHMENT FAILED - DIAGNOSTICS" << std::endl;
            std::cerr << "========================================" << std::endl;
            std::cerr << std::endl;
            std::cerr << "Please review the detailed log above." << std::endl;
            std::cerr << std::endl;
            std::cerr << "Common Solutions:" << std::endl;
            std::cerr << "1. Start Black Desert Online and log into a character" << std::endl;
            std::cerr << "2. Verify process name is BlackDesert64.exe in Task Manager" << std::endl;
            std::cerr << "3. Run this program as Administrator" << std::endl;
            std::cerr << "4. Check if XignCode is blocking (NtOpenProcess should bypass)" << std::endl;
            std::cerr << std::endl;
            std::cerr << "What was attempted (check log above):" << std::endl;
            std::cerr << "- Process search and PID detection" << std::endl;
            std::cerr << "- NtOpenProcess (anti-hook method)" << std::endl;
            std::cerr << "- Standard OpenProcess (fallback)" << std::endl;
            std::cerr << "- Base address detection" << std::endl;
            std::cerr << "- Memory read test" << std::endl;
            std::cerr << std::endl;
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.get();
            return 1;
        }
        
        std::cout << "[OK] Attached to BDO process" << std::endl;
        
        // Enable stealth features
        g_Memory->EnableAntiDetection(true);
        g_Memory->EnableStealthMode(true);
        g_Memory->SetOperationDelay(10, 50);
        
        std::cout << "[OK] Stealth features enabled" << std::endl;
        std::cout << std::endl;
        std::cout << "Bot is now running. Press Ctrl+C to stop." << std::endl;
        std::cout << "================================================" << std::endl;
        
        // Keep the bot running
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            // Read and display player stats periodically
            int currentHealth, maxHealth;
            int currentMana, maxMana;
            
            if (g_Memory->ReadPlayerHealth(currentHealth, maxHealth)) {
                // Bot is working
            }
            
            if (g_Memory->ReadPlayerMana(currentMana, maxMana)) {
                // Bot is working
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[EXCEPTION] " << e.what() << std::endl;
        std::cout << std::endl << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }
    
    return 0;
}

bool InitializeBot() {
    // Set up default configuration
    g_Config.enableAutoHealing = true;
    g_Config.enableAutoMana = true;
    g_Config.enableAutoSelling = false;
    g_Config.enableAutoCombat = false;
    g_Config.enableStealthMode = true;
    g_Config.enableAntiDetection = true;
    g_Config.minHealthThreshold = 50;
    g_Config.minManaThreshold = 30;
    g_Config.maxWeightThreshold = 80.0f;
    g_Config.potionCooldown = 3000;
    g_Config.skillCooldown = 1000;
    g_Config.updateInterval = 100;
    
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
    
    // Create memory systems
    g_Memory = std::make_unique<BDOAdvancedMemory>();
    g_Resolver = std::make_unique<BDOMemoryResolver>();
    
    // Create GUI
    g_GUI = std::make_unique<BDOStealthGUI>();
    
    // Connect to BDO process
    if (!g_Memory->AttachToProcess(L"BlackDesert64.exe")) {
        std::cout << "Failed to attach to BDO process" << std::endl;
        std::cout << "Running in demo mode" << std::endl;
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
    
    // Disconnect from drivers
    if (g_RTCore64.IsConnected()) {
        g_RTCore64.Disconnect();
        LogBotMessage("Disconnected from RTCore64 driver");
    }
    
    if (g_KernelInterface.IsConnected()) {
        g_KernelInterface.Disconnect();
        LogBotMessage("Disconnected from kernel driver");
    }
    
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

// Connect to driver
bool ConnectToDriver() {
    // Try RTCore64 driver first (highest priority)
    if (g_RTCore64.Connect()) {
        LogBotMessage("Connected to RTCore64 driver");
        return true;
    }
    
    // Fall back to original kernel driver
    if (g_KernelInterface.Connect()) {
        LogBotMessage("Connected to kernel driver");
        return true;
    }
    
    LogBotMessage("Failed to connect to any kernel driver");
    return false;
}

// Disconnect from driver
void DisconnectFromDriver() {
    if (g_RTCore64.IsConnected()) {
        g_RTCore64.Disconnect();
        LogBotMessage("Disconnected from RTCore64 driver");
    }
    
    if (g_KernelInterface.IsConnected()) {
        g_KernelInterface.Disconnect();
        LogBotMessage("Disconnected from kernel driver");
    }
}

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

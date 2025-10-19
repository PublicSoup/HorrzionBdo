#include <iostream>
#include "Platform.h"
#include <memory>
#include <thread>
#include "AntiDetection.h"
#include "BDOAntiDetection.h"
#include "BDOConfig.h"

// Function to demonstrate basic anti-detection
void DemoBasicAntiDetection() {
    std::cout << "\n=== Basic Anti-Detection Demo ===\n";
    
    AntiDetection antiDet;
    antiDet.SetTimingRange(100, 500);
    antiDet.SetPauseChance(15);
    antiDet.SetJitterRange(10.0);
    
    std::cout << "Adding random delays and mouse jitter...\n";
    for (int i = 0; i < 3; i++) {
        antiDet.RandomizeTiming();
        antiDet.AddMouseJitter();
        
        if (antiDet.ShouldTakeRandomPause()) {
            std::cout << "Taking random pause...\n";
            antiDet.RandomDelay(500, 1000);
        }
    }
    std::cout << "Basic demo complete.\n";
}

// Function to demonstrate BDO-specific features
void DemoBDOFeatures() {
    std::cout << "\n=== Black Desert Online Anti-Detection System ===\n";
    std::cout << "Advanced bot detection evasion for BDO\n\n";
    
    // Create BDO anti-detection instance
    auto bdoAntiDet = std::make_unique<BDOAntiDetection>();
    
    // Initialize and find BDO window
    if (bdoAntiDet->Initialize()) {
        std::cout << "✓ BDO window found and initialized\n";
    } else {
        std::cout << "✗ BDO not running - continuing in simulation mode\n";
    }
    
    // Configure skill level (0.0 = beginner, 1.0 = expert)
    bdoAntiDet->SetSkillLevel(0.6);  // Intermediate player
    std::cout << "✓ Skill level set to intermediate (0.6)\n";
    
    // Enable advanced input method (PostMessage instead of SendInput)
    bdoAntiDet->EnableAdvancedMode(true);
    std::cout << "✓ Advanced input mode enabled (less detectable)\n\n";
}

// Demonstrate human-like mouse movements
void DemoMouseMovement() {
    std::cout << "\n=== Human-like Mouse Movement Demo ===\n";
    
    auto bdoAntiDet = std::make_unique<BDOAntiDetection>();
    
    POINT startPos;
    GetCursorPos(&startPos);
    std::cout << "Starting position: (" << startPos.x << ", " << startPos.y << ")\n";
    
    // Demo 1: Bezier curve movement
    std::cout << "\n1. Bezier curve movement (smooth, natural curves):\n";
    int targetX = startPos.x + 200;
    int targetY = startPos.y + 100;
    
    std::cout << "   Moving to (" << targetX << ", " << targetY << ") with curves...\n";
    bdoAntiDet->MoveMouseBezier(targetX, targetY, 500);  // 500ms duration
    
    // Demo 2: Human movement with overshoot
    std::cout << "\n2. Human-like movement with occasional overshoot:\n";
    targetX = startPos.x;
    targetY = startPos.y;
    
    std::cout << "   Moving back with human characteristics...\n";
    bdoAntiDet->MoveMouseHuman(targetX, targetY);
    
    // Demo 3: Add mouse noise/tremor
    std::cout << "\n3. Adding natural mouse tremor:\n";
    for (int i = 0; i < 5; i++) {
        bdoAntiDet->AddMouseNoise(3);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Mouse movement demo complete.\n";
}

// Demonstrate combat rotation system
void DemoCombatRotation() {
    std::cout << "\n=== Combat Rotation Demo ===\n";
    std::cout << "Simulating warrior PvE rotation with human-like execution\n\n";
    
    auto bdoAntiDet = std::make_unique<BDOAntiDetection>();
    bdoAntiDet->SetSkillLevel(0.7);  // Skilled player
    
    // Warrior PvE rotation keys
    std::vector<WORD> warriorRotation = {
        'Q',           // Spinning Slash
        VK_LBUTTON,    // Basic Attack
        VK_RBUTTON,    // Heavy Strike
        'F',           // Grab
        VK_SHIFT       // Ultimate
    };
    
    std::cout << "Executing combat rotation:\n";
    std::cout << "- Q: Spinning Slash\n";
    std::cout << "- LMB: Basic Attack\n";
    std::cout << "- RMB: Heavy Strike\n";
    std::cout << "- F: Grab\n";
    std::cout << "- Shift: Ultimate\n\n";
    
    // Execute rotation (simplified for demo)
    std::cout << "Note: Actual key presses disabled for safety\n";
    
    // Simulate combat mistakes
    std::cout << "\nAdding realistic combat mistakes:\n";
    for (int i = 0; i < 3; i++) {
        std::cout << "- Mistake " << (i+1) << ": ";
        bdoAntiDet->AddCombatMistake();
        std::cout << "Applied\n";
    }
}

// Demonstrate life skill automation
void DemoLifeSkills() {
    std::cout << "\n=== Life Skills Automation Demo ===\n";
    
    auto bdoAntiDet = std::make_unique<BDOAntiDetection>();
    bdoAntiDet->SetSkillLevel(0.4);  // Casual life skiller
    
    std::cout << "\n1. Gathering Pattern (3 iterations):\n";
    std::cout << "   Note: Simulated only - not executing actual gathering\n";
    // bdoAntiDet->GatheringPattern(3);  // Commented for safety
    std::cout << "   - Would search for nodes\n";
    std::cout << "   - Press R to gather\n";
    std::cout << "   - Wait 3 seconds\n";
    std::cout << "   - Move to next node\n";
    
    std::cout << "\n2. Fishing Pattern:\n";
    std::cout << "   Note: Simulated only - not executing actual fishing\n";
    // bdoAntiDet->FishingPattern();  // Commented for safety
    std::cout << "   - Cast rod with Space\n";
    std::cout << "   - Wait 3-5 minutes for bite\n";
    std::cout << "   - Complete mini-game\n";
    
    std::cout << "\n3. Processing Pattern (2 batches):\n";
    std::cout << "   Note: Simulated only - not executing actual processing\n";
    // bdoAntiDet->ProcessingPattern(2);  // Commented for safety
    std::cout << "   - Open processing with L\n";
    std::cout << "   - Select materials\n";
    std::cout << "   - Wait for completion\n";
}

// Demonstrate profile management
void DemoProfileSystem() {
    std::cout << "\n=== Profile Management System Demo ===\n";
    
    // Create profile manager
    BDOProfileManager profileManager("./profiles/");
    
    std::cout << "\n1. Creating default profiles...\n";
    profileManager.CreateDefaultProfiles();
    
    std::cout << "\n2. Available profiles:\n";
    auto profiles = profileManager.ListProfiles();
    for (const auto& profile : profiles) {
        std::cout << "   - " << profile << "\n";
    }
    
    std::cout << "\n3. Loading different profiles:\n";
    
    // Load and display beginner profile
    profileManager.SetCurrentProfile("Beginner");
    auto& beginnerConfig = profileManager.GetCurrentProfile();
    std::cout << "\nBeginner Profile:\n";
    std::cout << "   - Skill Level: " << beginnerConfig.behavior.skillLevel << "\n";
    std::cout << "   - Mistake Chance: " << beginnerConfig.behavior.mistakeChance << "\n";
    std::cout << "   - Reaction Time: " << beginnerConfig.behavior.minReactionTime 
              << "-" << beginnerConfig.behavior.maxReactionTime << "ms\n";
    
    // Load and display advanced profile
    profileManager.SetCurrentProfile("Advanced");
    auto& advancedConfig = profileManager.GetCurrentProfile();
    std::cout << "\nAdvanced Profile:\n";
    std::cout << "   - Skill Level: " << advancedConfig.behavior.skillLevel << "\n";
    std::cout << "   - Mistake Chance: " << advancedConfig.behavior.mistakeChance << "\n";
    std::cout << "   - Reaction Time: " << advancedConfig.behavior.minReactionTime 
              << "-" << advancedConfig.behavior.maxReactionTime << "ms\n";
    
    std::cout << "\n4. Custom profile example:\n";
    BDOConfig customConfig = BDOConfig::GetDefaultProfile();
    customConfig.profileName = "MyCustomProfile";
    customConfig.characterClass = "Sorceress";
    customConfig.behavior.skillLevel = 0.8;
    customConfig.combat.skillRotation = {"SHIFT+LMB", "SHIFT+RMB", "C", "W+F"};
    
    profileManager.SaveProfile("MyCustomProfile", customConfig);
    std::cout << "   Custom profile 'MyCustomProfile' created and saved.\n";
}

// Demonstrate pattern analysis and variation
void DemoPatternAnalysis() {
    std::cout << "\n=== Pattern Analysis & Variation Demo ===\n";
    
    auto bdoAntiDet = std::make_unique<BDOAntiDetection>();
    
    std::cout << "\n1. Simulating repetitive actions...\n";
    for (int i = 0; i < 10; i++) {
        bdoAntiDet->AddHumanDelay(BDO::ActionType::COMBAT_SKILL);
        std::cout << "   Action " << (i+1) << " executed\n";
    }
    
    std::cout << "\n2. Analyzing pattern...\n";
    bdoAntiDet->AnalyzeActionPattern();
    
    if (bdoAntiDet->IsPatternTooRepetitive()) {
        std::cout << "   ⚠ Pattern detected as too repetitive!\n";
        std::cout << "   Introducing variation...\n";
        bdoAntiDet->IntroduceVariation();
    } else {
        std::cout << "   ✓ Pattern appears natural\n";
    }
    
    std::cout << "\n3. Session management:\n";
    double sessionHours = bdoAntiDet->GetSessionDuration();
    std::cout << "   Current session duration: " << sessionHours << " hours\n";
    
    if (bdoAntiDet->ShouldTakeBreak()) {
        std::cout << "   ⚠ Break recommended based on fatigue/time\n";
        std::cout << "   Would take 5-15 minute break...\n";
    } else {
        std::cout << "   ✓ No break needed yet\n";
    }
}

// Main menu system
void ShowMenu() {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║     Black Desert Online Anti-Detection System     ║\n";
    std::cout << "║            Advanced Bot Evasion Suite             ║\n";
    std::cout << "╚═══════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "Select a demonstration:\n";
    std::cout << "1. Basic Anti-Detection Features\n";
    std::cout << "2. BDO-Specific Initialization\n";
    std::cout << "3. Human-like Mouse Movement\n";
    std::cout << "4. Combat Rotation System\n";
    std::cout << "5. Life Skills Automation\n";
    std::cout << "6. Profile Management System\n";
    std::cout << "7. Pattern Analysis & Variation\n";
    std::cout << "8. Run All Demos\n";
    std::cout << "0. Exit\n";
    std::cout << "\nEnter choice: ";
}

int main() {
    std::cout << "Initializing Black Desert Online Anti-Detection System...\n";
    
    bool running = true;
    while (running) {
        ShowMenu();
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                DemoBasicAntiDetection();
                break;
            case 2:
                DemoBDOFeatures();
                break;
            case 3:
                DemoMouseMovement();
                break;
            case 4:
                DemoCombatRotation();
                break;
            case 5:
                DemoLifeSkills();
                break;
            case 6:
                DemoProfileSystem();
                break;
            case 7:
                DemoPatternAnalysis();
                break;
            case 8:
                std::cout << "\n=== Running All Demonstrations ===\n";
                DemoBasicAntiDetection();
                DemoBDOFeatures();
                DemoMouseMovement();
                DemoCombatRotation();
                DemoLifeSkills();
                DemoProfileSystem();
                DemoPatternAnalysis();
                std::cout << "\n=== All Demonstrations Complete ===\n";
                break;
            case 0:
                running = false;
                std::cout << "\nExiting... Thank you for using BDO Anti-Detection System!\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
        
        if (running && choice != 0) {
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore();
            std::cin.get();
        }
    }
    
    return 0;
}
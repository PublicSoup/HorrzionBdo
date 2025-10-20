# BDO Bot Memory Integration Guide

This guide explains how to integrate memory addresses found with external scanners (like MSI Afterburner, Cheat Engine, or our built-in scanner) into the bot automation system.

## Quick Start

### Step 1: Find Memory Addresses

You can find memory addresses using any of these methods:

#### Method A: Using Our Built-in Scanner
```cpp
BDOBotMemoryIntegration bot;
bot.AttachToGame(L"BlackDesert64.exe");

// Automatic scanning
bot.ScanForAddresses();

// Or manual scanning for specific values
AddressScanner scanner(&bot);
scanner.ScanForHealth(1000);  // If your current health is 1000
scanner.ScanForMana(500);     // If your current mana is 500
```

#### Method B: Using MSI Afterburner / Cheat Engine
1. Open MSI Afterburner or Cheat Engine
2. Attach to BlackDesert64.exe
3. Search for your current health value (e.g., 1000)
4. Get hit or heal to change health
5. Search for the new value
6. Repeat until you have 1-2 addresses
7. Note down the address (e.g., 0x7FF6A2B4C890)

#### Method C: Loading from File
Create a file `addresses.txt`:
```
playerHealth=0x7FF6A2B4C890
playerMaxHealth=0x7FF6A2B4C894
playerMana=0x7FF6A2B4C898
playerMaxMana=0x7FF6A2B4C89C
```

Then load it:
```cpp
bot.LoadAddressesFromFile("addresses.txt");
```

### Step 2: Set Up the Bot with Found Addresses

```cpp
// Initialize bot
BDOBotMemoryIntegration bot;
bot.AttachToGame(L"BlackDesert64.exe");

// Set addresses manually (from MSI Afterburner/Cheat Engine)
bot.SetPlayerHealthAddress((PVOID)0x7FF6A2B4C890);
bot.SetPlayerManaAddress((PVOID)0x7FF6A2B4C898);
bot.SetPlayerPositionAddresses(
    (PVOID)0x7FF6A2B4C8A0,  // X
    (PVOID)0x7FF6A2B4C8A4,  // Y
    (PVOID)0x7FF6A2B4C8A8   // Z
);

// Set up automated actions
bot.SetupCombatBot();

// Start monitoring
bot.StartMonitoring();
```

### Step 3: Create Custom Automation

```cpp
// Register custom actions
bot.RegisterAction("Heal", []() {
    std::cout << "Using healing skill" << std::endl;
    keybd_event('Q', 0, 0, 0);
    Sleep(50);
    keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
}, std::chrono::seconds(3));  // 3 second cooldown

// Add triggers based on memory values
bot.AddHealthTrigger(0.3f, {"Heal"});  // Heal when health < 30%

// Custom trigger for specific memory address
MemoryCondition lowStamina;
lowStamina.type = MemoryCondition::LESS_THAN;
lowStamina.value1 = 50;  // Stamina < 50

bot.AddTrigger("LowStamina", 
    (PVOID)0x7FF6A2B4C8B0,  // Stamina address
    ValueType::Dword, 
    lowStamina, 
    {"RestAction"});
```

## Complete Example: Combat Bot

```cpp
#include "BDO_BotMemoryIntegration.h"
#include <iostream>

int main() {
    // Create bot instance
    BDOBotMemoryIntegration bot;
    
    // Attach to game
    if (!bot.AttachToGame(L"BlackDesert64.exe")) {
        std::cerr << "Failed to attach to game!" << std::endl;
        return 1;
    }
    
    // Try automatic scanning first
    if (!bot.ScanForAddresses()) {
        std::cout << "Automatic scanning failed. Please enter addresses manually." << std::endl;
        
        // Manual address input
        std::cout << "Enter player health address (hex): ";
        std::string healthAddr;
        std::cin >> healthAddr;
        bot.SetPlayerHealthAddress((PVOID)std::stoull(healthAddr, nullptr, 16));
    }
    
    // Set up combat automation
    bot.RegisterAction("UseHealthPotion", []() {
        keybd_event('1', 0, 0, 0);
        Sleep(50);
        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(5), 10);  // Priority 10, cooldown 5s
    
    bot.RegisterAction("UseManaPotion", []() {
        keybd_event('2', 0, 0, 0);
        Sleep(50);
        keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(5), 9);
    
    bot.RegisterAction("BasicAttack", []() {
        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        Sleep(100);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }, std::chrono::milliseconds(500));
    
    bot.RegisterAction("SkillRotation", []() {
        // Skill 1
        keybd_event('Q', 0, 0, 0);
        Sleep(50);
        keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
        Sleep(500);
        
        // Skill 2
        keybd_event('E', 0, 0, 0);
        Sleep(50);
        keybd_event('E', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(3));
    
    // Add triggers
    bot.AddHealthTrigger(0.5f, {"UseHealthPotion"});
    bot.AddManaTrigger(0.3f, {"UseManaPotion"});
    bot.AddCombatTrigger(true, {"BasicAttack", "SkillRotation"});
    
    // Start monitoring
    bot.StartMonitoring(std::chrono::milliseconds(100));
    
    // Run bot
    std::cout << "Bot is running. Press Enter to stop..." << std::endl;
    std::cin.ignore();
    std::cin.get();
    
    // Stop and print stats
    bot.StopMonitoring();
    bot.PrintStatistics();
    
    // Save addresses for next time
    bot.SaveAddressesToFile("bdo_addresses.txt");
    
    return 0;
}
```

## Finding Addresses with Different Tools

### Using Cheat Engine
1. Open Cheat Engine
2. Click "Select a process" and choose BlackDesert64.exe
3. Enter your current health value in "Value" field
4. Click "First Scan"
5. Get hit or heal in game
6. Enter new health value and click "Next Scan"
7. Repeat until you have 1-2 addresses
8. Right-click address → "What writes to this address"
9. Note the base address and offset

### Using MSI Afterburner (with RivaTuner)
1. Enable OSD in MSI Afterburner
2. Use RivaTuner Statistics Server
3. Add custom OSD items for memory addresses
4. Monitor values in real-time
5. Note stable addresses

### Using Our Scanner
```cpp
// Scan for unknown values (when you don't know the exact value)
bot.memory->ScanForUnknown(ValueType::Dword);

// Change the value in game (e.g., take damage)
// Then scan for changed values
bot.memory->NextScan(ScanType::Changed);

// Repeat until you find the address
bot.memory->NextScan(ScanType::Decreased);  // If health decreased
```

## Common BDO Memory Addresses (Update for your version)

```cpp
// Example addresses (these change with game updates!)
const PVOID PLAYER_BASE = (PVOID)0x143B5A2C8;
const PVOID HEALTH_OFFSET = (PVOID)0x2C;
const PVOID MANA_OFFSET = (PVOID)0x30;
const PVOID STAMINA_OFFSET = (PVOID)0x34;
const PVOID POSITION_X_OFFSET = (PVOID)0x40;
const PVOID POSITION_Y_OFFSET = (PVOID)0x44;
const PVOID POSITION_Z_OFFSET = (PVOID)0x48;

// Calculate actual addresses
PVOID healthAddr = (PUCHAR)PLAYER_BASE + (uintptr_t)HEALTH_OFFSET;
```

## Advanced Automation Examples

### Farming Bot
```cpp
void SetupFarmingBot(BDOBotMemoryIntegration& bot) {
    // Register farming actions
    bot.RegisterAction("PlantSeed", []() {
        keybd_event('R', 0, 0, 0);
        Sleep(50);
        keybd_event('R', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(1));
    
    bot.RegisterAction("WaterPlant", []() {
        keybd_event('F', 0, 0, 0);
        Sleep(50);
        keybd_event('F', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(2));
    
    // Add position-based trigger
    MemoryCondition nearFarm;
    nearFarm.type = MemoryCondition::BETWEEN;
    nearFarm.value1 = 1000.0f;  // X coordinate range
    nearFarm.value2 = 1100.0f;
    
    bot.AddTrigger("NearFarm", bot.GetAddress("playerX"), 
                   ValueType::Float, nearFarm, {"PlantSeed"});
}
```

### Fishing Bot
```cpp
void SetupFishingBot(BDOBotMemoryIntegration& bot) {
    bot.RegisterAction("CastRod", []() {
        // Space to cast
        keybd_event(VK_SPACE, 0, 0, 0);
        Sleep(50);
        keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
    });
    
    bot.RegisterAction("CatchFish", []() {
        // Space to catch when bite detected
        keybd_event(VK_SPACE, 0, 0, 0);
        Sleep(50);
        keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
    });
    
    // Monitor fishing state address
    MemoryCondition fishBite;
    fishBite.type = MemoryCondition::EQUAL_TO;
    fishBite.value1 = 1;  // Fish bite state
    
    bot.AddTrigger("FishBite", (PVOID)0x7FF6A2B4D000, 
                   ValueType::Dword, fishBite, {"CatchFish"});
}
```

### Auto-Potion Based on Exact HP
```cpp
void SetupSmartHealing(BDOBotMemoryIntegration& bot) {
    // Different potions for different HP levels
    bot.RegisterAction("UseSmallPotion", []() {
        keybd_event('1', 0, 0, 0);
        Sleep(50);
        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
    });
    
    bot.RegisterAction("UseLargePotion", []() {
        keybd_event('2', 0, 0, 0);
        Sleep(50);
        keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
    });
    
    // Use small potion when HP between 50-70%
    MemoryCondition mediumHP;
    mediumHP.type = MemoryCondition::BETWEEN;
    mediumHP.value1 = 0.5f;
    mediumHP.value2 = 0.7f;
    
    // Use large potion when HP < 30%
    bot.AddHealthTrigger(0.3f, {"UseLargePotion"});
}
```

## Troubleshooting

### Addresses Change After Game Restart
- Use pointer chains instead of static addresses
- Implement pattern scanning to find base addresses
- Save and load address offsets rather than absolute addresses

### Bot Not Triggering Actions
1. Verify addresses are correct:
```cpp
int health, maxHealth, mana, maxMana;
if (bot.ReadPlayerStats(health, maxHealth, mana, maxMana)) {
    std::cout << "Health: " << health << "/" << maxHealth << std::endl;
    std::cout << "Mana: " << mana << "/" << maxMana << std::endl;
}
```

2. Check trigger conditions:
```cpp
float healthPercent = bot.GetPlayerHealthPercentage();
std::cout << "Health %: " << healthPercent * 100 << std::endl;
```

3. Enable debug output:
```cpp
bot.memory->EnableDebugOutput(true);
```

### Performance Issues
- Increase monitoring interval: `bot.StartMonitoring(std::chrono::milliseconds(200));`
- Reduce number of active triggers
- Use memory filtering to scan smaller regions

## Safety Tips

1. **Always test in safe areas first**
2. **Start with non-combat features** (auto-potion, auto-buff)
3. **Add randomization to actions** to appear more human-like
4. **Monitor game updates** as they may change memory addresses
5. **Use at your own risk** - respect game ToS

## Next Steps

1. Implement more sophisticated patterns
2. Add machine learning for combat decisions
3. Create GUI for easy configuration
4. Implement memory encryption bypass
5. Add network packet monitoring

Remember: This is for educational purposes. Always respect the game's terms of service and use responsibly.

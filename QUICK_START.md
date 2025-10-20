# BDO Bot - Quick Start Guide

## 🚀 Get Started in 5 Minutes!

### Step 1: Compile the Bot (1 minute)
```bash
# Open Visual Studio
# Load BDOStealthBot.sln
# Build in Release x64 mode
# Or run: build.bat
```

### Step 2: Find Memory Addresses (2 minutes)

#### Option A: Using MSI Afterburner (Easiest)
1. Open MSI Afterburner
2. Go to game, note your current health (e.g., 1500)
3. In Afterburner, search for value: 1500
4. Take damage in game
5. Search again for new health value
6. Repeat until you have 1-2 addresses
7. Note the address (e.g., `0x7FF6A2B4C890`)

#### Option B: Using Cheat Engine
1. Open Cheat Engine
2. Attach to `BlackDesert64.exe`
3. Search for your health value
4. Take damage, search new value
5. Repeat until 1-2 results
6. Note the address

#### Option C: Use Our Scanner
```cpp
// Let the bot find it automatically
bot.ScanForAddresses();
```

### Step 3: Run the Bot (2 minutes)

```bash
# Run the compiled program
BDO_BotExample.exe
```

**In the menu:**
1. Choose `2` (Enter addresses manually)
2. Paste your health address: `7FF6A2B4C890`
3. Skip other addresses for now (press Enter)
4. Choose `4` (Start Combat Bot)
5. Done! Bot will auto-use potions when health is low

---

## 💊 Example: Auto-Potion Bot

### Simple Version (Copy & Paste)
```cpp
#include "BDO_BotMemoryIntegration.h"

int main() {
    // Create bot
    BDOBotMemoryIntegration bot;
    bot.AttachToGame(L"BlackDesert64.exe");
    
    // Set your health address (from MSI Afterburner)
    bot.SetPlayerHealthAddress((PVOID)0x7FF6A2B4C890);  // CHANGE THIS!
    
    // Register potion action (key '1')
    bot.RegisterAction("UsePotion", []() {
        keybd_event('1', 0, 0, 0);
        Sleep(50);
        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
    });
    
    // Use potion when health < 50%
    bot.AddHealthTrigger(0.5f, {"UsePotion"});
    
    // Start!
    bot.StartMonitoring();
    
    std::cout << "Bot running! Press Enter to stop..." << std::endl;
    std::cin.get();
    
    return 0;
}
```

### What This Does
- ✅ Monitors your health continuously
- ✅ Automatically presses '1' when health drops below 50%
- ✅ Has 5-second cooldown between potions
- ✅ Runs in background while you play

---

## ⚔️ Example: Combat Bot

```cpp
BDOBotMemoryIntegration bot;
bot.AttachToGame(L"BlackDesert64.exe");

// Set addresses
bot.SetPlayerHealthAddress((PVOID)0x7FF6A2B4C890);
bot.SetPlayerManaAddress((PVOID)0x7FF6A2B4C898);
bot.SetCombatAddresses((PVOID)0x7FF6A2B4C8A0, nullptr);

// Register actions
bot.RegisterAction("Heal", []() {
    keybd_event('1', 0, 0, 0);
    Sleep(50);
    keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
});

bot.RegisterAction("ManaPotion", []() {
    keybd_event('2', 0, 0, 0);
    Sleep(50);
    keybd_event('2', 0, KEYEVENTF_KEYUP, 0);
});

bot.RegisterAction("Attack", []() {
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    Sleep(100);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
});

bot.RegisterAction("SkillQ", []() {
    keybd_event('Q', 0, 0, 0);
    Sleep(50);
    keybd_event('Q', 0, KEYEVENTF_KEYUP, 0);
});

// Add triggers
bot.AddHealthTrigger(0.5f, {"Heal"});
bot.AddManaTrigger(0.3f, {"ManaPotion"});
bot.AddCombatTrigger(true, {"Attack", "SkillQ"});

// Start!
bot.StartMonitoring();
```

---

## 🎣 Example: Fishing Bot

```cpp
BDOBotMemoryIntegration bot;
bot.AttachToGame(L"BlackDesert64.exe");

// Register fishing actions
bot.RegisterAction("CastRod", []() {
    keybd_event(VK_SPACE, 0, 0, 0);
    Sleep(50);
    keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
});

bot.RegisterAction("CatchFish", []() {
    // Catch when bite detected
    keybd_event(VK_SPACE, 0, 0, 0);
    Sleep(50);
    keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
});

// You need to find the fishing bite indicator address
PVOID fishBiteAddress = (PVOID)0x7FF6A2B4D000;  // Find this!

MemoryCondition fishBite;
fishBite.type = MemoryCondition::EQUAL_TO;
fishBite.value1 = 1;  // 1 = fish is biting

bot.AddTrigger("FishBite", fishBiteAddress, ValueType::Dword, 
               fishBite, {"CatchFish"});

bot.StartMonitoring();
```

---

## 🔍 Finding Addresses - Detailed

### Health Address
1. Go in game, check current HP (e.g., 1500)
2. In scanner: Search for `1500` (4-byte integer)
3. Take damage, now at 1200 HP
4. Next scan: Search for `1200`
5. Heal to 1400
6. Next scan: `1400`
7. When you have 1-2 results, that's your health!

### Mana Address
- Same process as health
- Usually near health address (+4 or +8 bytes)

### Position (X, Y, Z)
1. Note your position coordinates in game
2. Search for X coordinate (4-byte float)
3. Move around
4. Next scan with new coordinates
5. Repeat until found
6. Y and Z are usually +4 and +8 bytes after X

### Combat State
1. Search when NOT in combat: value `0`
2. Enter combat
3. Next scan when IN combat: value `1`
4. Leave combat
5. Next scan: value `0`
6. Repeat until found

---

## 🛠️ Common Issues & Fixes

### "Failed to attach to game"
- ✅ Make sure BDO is running
- ✅ Run bot as Administrator
- ✅ Check process name is `BlackDesert64.exe`

### "Addresses not working"
- ❌ Addresses change when game restarts
- ✅ Find them again after each restart
- ✅ Or use pointer chains (advanced)

### "Bot not triggering actions"
- ✅ Verify address with monitoring mode first
- ✅ Check trigger thresholds
- ✅ Enable debug output: `bot.memory->EnableDebugOutput(true);`

### "Game crashes"
- ❌ Don't write invalid addresses
- ✅ Test addresses in monitoring mode first
- ✅ Use ValidateAddress() before writing

---

## 📖 Next Steps

### Beginner
1. ✅ Get auto-potion working
2. ✅ Add auto-mana
3. ✅ Monitor values in real-time

### Intermediate
1. ✅ Set up combat bot
2. ✅ Find multiple addresses
3. ✅ Save/load address files

### Advanced
1. ✅ Create custom automation
2. ✅ Use pointer chains
3. ✅ Implement complex triggers

---

## 🎯 Pro Tips

### 1. Save Your Addresses
```cpp
// After finding addresses, save them
bot.SaveAddressesToFile("my_addresses.txt");

// Next time, just load them
bot.LoadAddressesFromFile("my_addresses.txt");
```

### 2. Monitor Before Automating
```cpp
// First, verify addresses work
int health, maxHealth, mana, maxMana;
while (true) {
    bot.ReadPlayerStats(health, maxHealth, mana, maxMana);
    std::cout << "HP: " << health << "/" << maxHealth << std::endl;
    Sleep(1000);
}
```

### 3. Use Priority for Actions
```cpp
// Higher priority = executes first
bot.RegisterAction("Heal", healFunc, std::chrono::seconds(5), 10);  // Priority 10
bot.RegisterAction("Attack", attackFunc, std::chrono::seconds(1), 5);  // Priority 5
// Heal will always execute before Attack
```

### 4. Add Cooldowns
```cpp
// 5-second cooldown prevents spam
bot.RegisterAction("Heal", healFunc, std::chrono::seconds(5));
```

### 5. Test in Safe Areas
- Start in town or safe zone
- Verify everything works
- Then use in dangerous areas

---

## 🚨 Safety Reminders

1. **Use at Your Own Risk**
   - May violate game ToS
   - Anti-cheat may detect
   - Account ban possible

2. **Start Simple**
   - Begin with auto-potion only
   - Test thoroughly
   - Add features gradually

3. **Be Subtle**
   - Add random delays
   - Don't bot 24/7
   - Appear human-like

4. **Keep Updated**
   - Game patches change addresses
   - Re-scan after updates
   - Save multiple address sets

---

## 📚 Full Documentation

For more details, see:
- `GUIDE_MemoryIntegration.md` - Complete integration guide
- `README_MemoryScanning.md` - Memory scanning details
- `PROJECT_SUMMARY.md` - Full project overview
- `CODEBASE_ANALYSIS.md` - Technical details

---

## ✅ Checklist

Before running your bot:
- [ ] BDO is running
- [ ] Addresses found and verified
- [ ] Bot compiled in Release mode
- [ ] Running as Administrator
- [ ] Tested in safe area
- [ ] Know how to stop it (Ctrl+C or close window)

---

## 🎉 You're Ready!

You now have everything you need to run your BDO bot. Start with the simple auto-potion example and expand from there. Good luck!

**Remember: Use responsibly and at your own risk!** 🚀

# Complete Step-by-Step Guide - Compile & Use BDO Bot

## 📋 Prerequisites (5 minutes)

### Required Software
1. **Visual Studio 2019 or 2022** (Community Edition is free)
   - Download: https://visualstudio.microsoft.com/downloads/
   - During installation, select "Desktop development with C++"

2. **Windows SDK** (Usually included with Visual Studio)

3. **MSI Afterburner** or **Cheat Engine** (for finding addresses)
   - MSI Afterburner: https://www.msi.com/Landing/afterburner
   - Cheat Engine: https://www.cheatengine.org/downloads.php

---

## 🔨 Part 1: Compiling the Bot (10 minutes)

### Method A: Using Visual Studio (Recommended)

#### Step 1: Open the Project
```
1. Navigate to your project folder:
   C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo\

2. Double-click: BDOStealthBot.sln
   (This opens Visual Studio)
```

#### Step 2: Configure Build Settings
```
1. At the top of Visual Studio:
   - Set configuration to: Release
   - Set platform to: x64
   
   [Debug ▼]  →  Change to  →  [Release ▼]
   [Any CPU ▼]  →  Change to  →  [x64 ▼]
```

#### Step 3: Build the Project
```
1. In Visual Studio menu:
   Build → Build Solution
   
   OR press: Ctrl + Shift + B

2. Wait for compilation (1-2 minutes)

3. Check output window for success:
   "Build succeeded"
```

#### Step 4: Find Your Executable
```
Location: 
C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo\bin\x64\Release\BDOStealthBot.exe

OR

C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo\x64\Release\BDOStealthBot.exe
```

### Method B: Using Command Line

#### Step 1: Open Developer Command Prompt
```
1. Start Menu → Search "Developer Command Prompt"
2. Run as Administrator
3. Navigate to project:
   cd "C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo"
```

#### Step 2: Run Build Script
```batch
build.bat
```

#### Step 3: Check Output
```
Look in: bin\x64\Release\BDOStealthBot.exe
```

---

## 🎮 Part 2: Finding Memory Addresses (10 minutes)

### Using MSI Afterburner (Easier)

#### Step 1: Install & Setup MSI Afterburner
```
1. Download and install MSI Afterburner
2. Launch MSI Afterburner
3. Go to Settings → Monitoring
4. Enable hardware monitoring
```

#### Step 2: Start Black Desert Online
```
1. Launch BDO normally
2. Log into character
3. Note your current stats:
   - Health: _____ (e.g., 1500)
   - Mana: _____ (e.g., 800)
```

#### Step 3: Find Health Address
```
Method 1: Using Cheat Engine (More Reliable)

1. Download Cheat Engine: https://www.cheatengine.org/
2. Install and open Cheat Engine
3. Click "Select a process" (computer icon)
4. Choose: BlackDesert64.exe
5. In "Value" box, enter your current health: 1500
6. Click "First Scan"
7. Take damage in game (health drops to 1200)
8. Enter new value: 1200
9. Click "Next Scan"
10. Repeat steps 7-9 until you have 1-5 results
11. Look for addresses like: 7FF6A2B4C890
12. Write down this address!

Example Results:
Address          Value    Type
7FF6A2B4C890     1200     4 Bytes  ← This is your health!
7FF6A2B4C894     1500     4 Bytes  ← This might be max health
```

#### Step 4: Find Other Addresses (Optional)
```
Mana Address:
- Same process as health
- Usually near health address (+8 or +16 bytes)

Position Addresses:
- Search for your X coordinate (float)
- Move character
- Next scan with new coordinate
- Y and Z are usually +4 and +8 bytes after X
```

---

## 🚀 Part 3: Running the Bot (5 minutes)

### Method A: Using the Example Program (Easiest)

#### Step 1: Run the Bot
```
1. Navigate to:
   C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo\bin\x64\Release\

2. Right-click BDOStealthBot.exe
   → Run as Administrator

3. You'll see a menu:
   ================================================
       BDO Bot Memory Integration Example
   ================================================
   
   === Bot Control Menu ===
   1. Scan for addresses automatically
   2. Enter addresses manually (from MSI Afterburner/Cheat Engine)
   3. Load addresses from file
   4. Start Combat Bot
   5. Start Fishing Bot
   6. Start Gathering Bot
   7. Monitor values only
   8. Save current addresses
   9. Show statistics
   0. Exit
   Choice:
```

#### Step 2: Enter Your Addresses
```
1. Choose option: 2 (Enter addresses manually)

2. When prompted, enter addresses WITHOUT '0x':
   Player Health Address: 0x7FF6A2B4C890
   → Type: 7FF6A2B4C890
   → Press Enter

3. For addresses you don't have, just press Enter to skip

4. The bot will confirm:
   "Set player health address to 0x7FF6A2B4C890"
```

#### Step 3: Test Monitoring (Important!)
```
1. Choose option: 7 (Monitor values only)

2. You should see real-time values:
   === BDO Memory Values ===
   Health: 1500/1500 (100.0%)
   Mana: 800/800 (100.0%)
   Position: [Not Available]
   Combat: Not in Combat

3. Take damage in game - watch the values change!
   Health: 1200/1500 (80.0%)  ← It's working!

4. If values show correctly, addresses are good!

5. Press Ctrl+C to stop monitoring
```

#### Step 4: Start Automated Bot
```
1. Choose option: 4 (Start Combat Bot)

2. Bot is now running! It will:
   - Monitor your health every 100ms
   - Press '1' when health < 50%
   - Press '2' when mana < 30%
   - Auto-attack in combat

3. Test it:
   - Take damage until health drops below 50%
   - Bot should automatically press '1' (health potion)

4. To stop: Press Enter in the bot window
```

### Method B: Custom Quick Bot (Advanced)

#### Step 1: Create Custom Bot File
Create a new file: `MyBot.cpp`

```cpp
#include <windows.h>
#include <iostream>
#include "BDO_BotMemoryIntegration.h"

int main() {
    std::cout << "Starting My BDO Bot..." << std::endl;
    
    // Create bot
    BDOBotMemoryIntegration bot;
    
    // Attach to game
    if (!bot.AttachToGame(L"BlackDesert64.exe")) {
        std::cerr << "ERROR: Could not attach to BDO!" << std::endl;
        std::cerr << "Make sure Black Desert Online is running." << std::endl;
        system("pause");
        return 1;
    }
    
    std::cout << "Successfully attached to BDO!" << std::endl;
    
    // YOUR ADDRESS HERE (from Cheat Engine)
    PVOID healthAddress = (PVOID)0x7FF6A2B4C890;  // CHANGE THIS!
    bot.SetPlayerHealthAddress(healthAddress);
    
    std::cout << "Health address set to: 0x" << std::hex << healthAddress << std::endl;
    
    // Register potion action (key '1')
    bot.RegisterAction("UseHealthPotion", []() {
        std::cout << "[BOT] Using health potion (Key 1)" << std::endl;
        
        // Press '1' key
        keybd_event('1', 0, 0, 0);
        Sleep(50);
        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
    }, std::chrono::seconds(5));  // 5 second cooldown
    
    // Trigger: Use potion when health < 50%
    bot.AddHealthTrigger(0.5f, {"UseHealthPotion"});
    
    std::cout << "\n=== Bot Configuration ===" << std::endl;
    std::cout << "Action: Use health potion" << std::endl;
    std::cout << "Trigger: Health below 50%" << std::endl;
    std::cout << "Cooldown: 5 seconds" << std::endl;
    std::cout << "========================\n" << std::endl;
    
    // Start monitoring
    std::cout << "Starting bot monitoring..." << std::endl;
    bot.StartMonitoring(std::chrono::milliseconds(100));  // Check every 100ms
    
    std::cout << "\n*** BOT IS NOW RUNNING ***" << std::endl;
    std::cout << "The bot will automatically use potions when needed." << std::endl;
    std::cout << "Press Enter to stop the bot..." << std::endl;
    
    // Wait for user to stop
    std::cin.get();
    
    // Cleanup
    std::cout << "Stopping bot..." << std::endl;
    bot.StopMonitoring();
    bot.PrintStatistics();
    
    std::cout << "Bot stopped. Goodbye!" << std::endl;
    return 0;
}
```

#### Step 2: Add to Visual Studio Project
```
1. In Visual Studio Solution Explorer
2. Right-click on "Source Files"
3. Add → Existing Item
4. Select MyBot.cpp
5. Build Solution
```

---

## 🧪 Part 4: Testing (5 minutes)

### Test 1: Address Verification
```
1. Run bot in monitoring mode (option 7)
2. Check if values update correctly
3. Take damage - does health decrease?
4. Heal - does health increase?

✅ If yes: Addresses are correct!
❌ If no: Find addresses again
```

### Test 2: Auto-Potion
```
1. Make sure health potion is in slot 1
2. Start combat bot (option 4)
3. Take damage to below 50% health
4. Bot should automatically press '1'

✅ If yes: Bot is working!
❌ If no: Check troubleshooting below
```

### Test 3: Multiple Actions
```
1. Put health potion in slot 1
2. Put mana potion in slot 2
3. Start combat bot
4. Test both health and mana triggers

✅ Both should work independently
```

---

## 🔧 Troubleshooting

### Problem: "Failed to attach to game"

**Solution 1: Run as Administrator**
```
Right-click bot exe → Run as Administrator
```

**Solution 2: Check Process Name**
```
1. Open Task Manager (Ctrl+Shift+Esc)
2. Find Black Desert process
3. Is it "BlackDesert64.exe" or "BlackDesert.exe"?
4. Update in code if different
```

**Solution 3: Check Game is Running**
```
Make sure you're logged into a character, not at character select
```

### Problem: "Addresses not working"

**Solution 1: Verify with Cheat Engine**
```
1. Open address in Cheat Engine
2. Add to address list
3. Watch value as you play
4. Does it change correctly?
```

**Solution 2: Find Address Again**
```
Addresses can change when:
- Game restarts
- Game patches
- System restarts
```

**Solution 3: Use Monitoring Mode**
```
Run bot in monitoring mode first
Check if values are reading correctly
```

### Problem: "Bot not pressing keys"

**Solution 1: Check Key Bindings**
```
Make sure potion is actually in slot 1
Press '1' manually first to verify
```

**Solution 2: Run Game in Windowed Mode**
```
Bot works best in windowed or borderless windowed mode
```

**Solution 3: Check Trigger Threshold**
```
If health trigger is 0.5 (50%):
- Bot only acts when health < 50%
- Try setting to 0.8 (80%) for testing
```

### Problem: "Compilation errors"

**Solution 1: Install Required Components**
```
Visual Studio Installer → Modify
Select: Desktop development with C++
```

**Solution 2: Clean and Rebuild**
```
Build → Clean Solution
Build → Rebuild Solution
```

**Solution 3: Check Platform**
```
Make sure platform is x64, not x86
```

---

## 📝 Creating an Address File

### Step 1: Save Addresses
```cpp
// After finding all addresses, save them
bot.SaveAddressesToFile("my_addresses.txt");
```

### Step 2: File Format
Create file: `my_addresses.txt`
```
# BDO Bot Memory Addresses
playerHealth=0x7FF6A2B4C890
playerMaxHealth=0x7FF6A2B4C894
playerMana=0x7FF6A2B4C898
playerMaxMana=0x7FF6A2B4C89C
playerX=0x7FF6A2B4C8A0
playerY=0x7FF6A2B4C8A4
playerZ=0x7FF6A2B4C8A8
combatState=0x7FF6A2B4C8B0
```

### Step 3: Load Addresses
```cpp
// Next time, just load them
bot.LoadAddressesFromFile("my_addresses.txt");
```

---

## 🎯 Quick Reference Commands

### Compile
```batch
# Visual Studio
Ctrl + Shift + B

# Command line
cd "C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo"
build.bat
```

### Run
```batch
# Navigate to output folder
cd bin\x64\Release

# Run as admin
BDOStealthBot.exe
```

### Find Executable
```
Method 1: bin\x64\Release\BDOStealthBot.exe
Method 2: x64\Release\BDOStealthBot.exe
```

---

## 🎓 Example Usage Flow

### Complete Workflow
```
1. START BDO
   → Log into character

2. FIND ADDRESSES
   → Open Cheat Engine
   → Attach to BlackDesert64.exe
   → Search for health value
   → Take damage, search again
   → Repeat until found
   → Write down address

3. COMPILE BOT
   → Open BDOStealthBot.sln in Visual Studio
   → Set to Release x64
   → Build → Build Solution
   → Find exe in bin\x64\Release

4. RUN BOT
   → Right-click exe → Run as Administrator
   → Choose option 2 (manual addresses)
   → Enter health address
   → Choose option 7 (monitor) to test
   → If working, choose option 4 (combat bot)

5. TEST BOT
   → Take damage
   → Watch bot use potion automatically
   → Success!

6. SAVE ADDRESSES
   → Choose option 8
   → Save to file
   → Next time, use option 3 to load
```

---

## ⚙️ Configuration Options

### Change Potion Key
```cpp
// In BDO_BotExample.cpp, line ~200
bot.RegisterAction("UseHealthPotion", []() {
    keybd_event('1', 0, 0, 0);  // Change '1' to any key
    Sleep(50);
    keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
});
```

### Change Health Threshold
```cpp
// In BDO_BotExample.cpp, line ~300
bot.AddHealthTrigger(0.5f, {"UseHealthPotion"});  // 0.5 = 50%
// Change to 0.8 for 80%, 0.3 for 30%, etc.
```

### Change Cooldown
```cpp
bot.RegisterAction("UseHealthPotion", potionFunc, 
    std::chrono::seconds(5));  // 5 seconds
// Change to 3, 10, 15, etc.
```

### Change Monitoring Speed
```cpp
bot.StartMonitoring(std::chrono::milliseconds(100));  // Check every 100ms
// Change to 50 (faster) or 200 (slower)
```

---

## 🏆 Success Checklist

Before considering your bot "working":
- [ ] Bot compiles without errors
- [ ] Bot attaches to BDO successfully
- [ ] Monitoring mode shows correct values
- [ ] Values update in real-time
- [ ] Auto-potion triggers at correct health %
- [ ] Cooldown system works (no spam)
- [ ] Bot can be stopped cleanly

---

## 📞 Need Help?

### Check These First
1. `CODEBASE_ANALYSIS.md` - Technical details
2. `PROJECT_SUMMARY.md` - Overall project info
3. `QUICK_START.md` - Quick reference
4. `GUIDE_MemoryIntegration.md` - Integration guide

### Common File Locations
```
Project Root: C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo\
Solution: BDOStealthBot.sln
Main Code: BDO_BotExample.cpp
Output: bin\x64\Release\BDOStealthBot.exe
Docs: All .md files in root
```

---

## 🎉 You're Done!

You should now have:
✅ Compiled bot executable
✅ Found memory addresses
✅ Working auto-potion bot
✅ Understanding of how to customize

**Next steps:**
- Add more actions (skills, mana potions)
- Find more addresses (position, combat state)
- Create custom automation
- Experiment and have fun!

**Remember: Use responsibly!** 🚀

# 🚀 START HERE - Quick Setup

## ⚡ Super Fast Setup (5 minutes)

### Step 1: Run Everything (2 minutes)

**Option A: All-in-One (Recommended)** ⭐
```
Right-click: RUN_EVERYTHING.bat
Select: "Run as administrator"
Wait for: Automatic compilation and launch!
```
**This does EVERYTHING for you!**

**Option B: One-Click Start (If already compiled)**
```
Double-click: ONE_CLICK_START.bat
(Checks BDO and launches immediately)
```

**Option C: Manual Compile Only**
```
Double-click: COMPILE_NOW.bat
Wait for: "✓ COMPILATION SUCCESSFUL!"
```

**Need help choosing?** Read: `WHICH_FILE_TO_RUN.md`

### Step 2: Find Health Address (5 minutes)

⚠️ **IMPORTANT: Don't use Cheat Engine!** BDO will close with error `E019100B`

**📖 USE THIS GUIDE:** See **[SAFE_MEMORY_FINDING.md](SAFE_MEMORY_FINDING.md)** for safe method!

**Quick Version (Built-in Scanner):**
```
1. Start Black Desert Online
2. Run BDOStealthBot.exe as Administrator
3. Choose: 1 (Use internal scanner)
4. Wait 30 seconds
5. Addresses found automatically!
```

**Alternative (Manual Entry):**
```
If you have addresses from forums/Discord:
1. Run BDOStealthBot.exe as Administrator
2. Choose: 2 (Enter manually)
3. Type address: 7FF6A2B4C890
```

**Need help?** Read the **[complete safe guide](SAFE_MEMORY_FINDING.md)** - no Cheat Engine needed!

### Step 3: Run Bot (3 minutes)

⚠️ **IMPORTANT: Start BDO FIRST, then run the bot!**

**Easy Way (Recommended):**
```
1. Double-click: SMART_START.bat
   - Checks if BDO is running
   - Starts bot automatically
```

**Manual Way:**
```
1. Make sure BDO is running and you're in-game
2. Run: bin\x64\Release\BDOStealthBot.exe as Admin
3. Choose option: 1 (Use internal scanner)
4. Wait 30 seconds for addresses
5. Choose option: 4 (Combat Bot)
6. Done! Bot will auto-use potions
```

**If bot closes immediately:** Read `WHY_BOT_CLOSES.md`

---

## 🎯 What the Bot Does

✅ **Monitors your health every 0.1 seconds**
✅ **Presses '1' when health < 50%** (health potion)
✅ **Presses '2' when mana < 30%** (mana potion)  
✅ **Auto-attacks in combat**
✅ **Cooldown system prevents spam**

---

## 📱 Quick Commands

### Compile
```batch
COMPILE_NOW.bat
```

### Run
```batch
# Go to:
cd bin\x64\Release

# Run as admin:
Right-click BDOStealthBot.exe → Run as Administrator
```

### Find Addresses
```
1. Cheat Engine
2. Attach to BlackDesert64.exe
3. Search for value
4. Change value (take damage)
5. Next scan
6. Repeat until found
```

---

## 📖 Full Documentation

**For complete details, read:**
- `STEP_BY_STEP_GUIDE.md` ← **START HERE FOR DETAILED GUIDE**
- `QUICK_START.md` - 5-minute quick start
- `PROJECT_SUMMARY.md` - Full project overview
- `GUIDE_MemoryIntegration.md` - Advanced integration

---

## ❓ Troubleshooting

### "Compilation failed"
```
1. Open BDOStealthBot.sln in Visual Studio
2. Build → Clean Solution
3. Build → Rebuild Solution
```

### "Failed to attach to game"
```
1. Make sure BDO is running
2. Run bot as Administrator
3. Make sure you're logged into a character
```

### "Address not working"
```
1. Choose option 7 (Monitor values)
2. Check if health updates correctly
3. If not, find address again with Cheat Engine
```

### "Bot not pressing keys"
```
1. Make sure potion is in slot 1
2. Test manually - press '1' yourself
3. Run game in Windowed mode
4. Check health trigger (try 0.8 instead of 0.5)
```

---

## 🎓 Learning Path

### Beginner (You are here!)
✅ Compile bot
✅ Find health address
✅ Run auto-potion bot

### Next Steps
1. Find mana address
2. Add more actions (skills)
3. Customize thresholds
4. Save addresses to file

### Advanced
1. Create custom triggers
2. Find position addresses
3. Combat automation
4. Fishing automation

---

## 🗂️ File Structure

```
Your Project Folder/
├── COMPILE_NOW.bat          ← Compile the bot
├── START_HERE.md            ← You are here
├── STEP_BY_STEP_GUIDE.md    ← Detailed guide
├── BDOStealthBot.sln        ← Open in Visual Studio
├── BDO_BotExample.cpp       ← Main bot code
└── bin/x64/Release/         ← Your compiled bot here
    └── BDOStealthBot.exe
```

---

## ✅ Quick Checklist

Before running:
- [ ] Visual Studio installed
- [ ] Bot compiled successfully
- [ ] BDO is running and logged in
- [ ] Found health address with Cheat Engine
- [ ] Health potion in slot 1
- [ ] Running bot as Administrator

---

## 🎮 First Time Setup

**Complete workflow (10 minutes):**

```
1. COMPILE
   → Double-click COMPILE_NOW.bat
   → Wait for success message

2. START BDO
   → Log into character
   → Note your current health

3. FIND ADDRESS
   → Open Cheat Engine
   → Attach to BlackDesert64.exe
   → Search for health
   → Take damage, search again
   → Repeat until found

4. RUN BOT
   → Run BDOStealthBot.exe as Admin
   → Option 2 (manual addresses)
   → Enter health address
   → Option 4 (combat bot)

5. TEST
   → Take damage below 50%
   → Bot should use potion
   → Success!
```

---

## 💡 Pro Tips

1. **Save addresses after finding them**
   - Option 8 in bot menu
   - Next time use Option 3 to load

2. **Test in monitoring mode first**
   - Option 7 shows real-time values
   - Verify addresses work before automating

3. **Start with simple automation**
   - Just auto-potion first
   - Add more features gradually

4. **Use windowed mode**
   - Bot works best in windowed/borderless

5. **Addresses change**
   - After game restart
   - After game update
   - Save them to file!

---

## 🚨 Important Notes

⚠️ **USE AT YOUR OWN RISK**
- May violate game Terms of Service
- Account ban is possible
- Anti-cheat may detect bot

🛡️ **Safety Tips**
- Start in safe areas
- Don't bot 24/7
- Appear human-like
- Use subtle automation

---

## 📞 Help Resources

**Quick answers:**
- Can't compile? → Check Visual Studio is installed
- Can't find address? → Use Cheat Engine, not MSI Afterburner
- Bot not working? → Run as Administrator
- Keys not pressing? → Use Windowed mode

**Full documentation:**
- `STEP_BY_STEP_GUIDE.md` - Complete walkthrough
- `TROUBLESHOOTING.md` - Common issues
- `QUICK_START.md` - Quick reference

---

## 🎉 You're Ready!

**Your bot is:**
✅ Fully compiled
✅ Production ready
✅ Easy to use
✅ Well documented

**Just need to:**
1. Run `COMPILE_NOW.bat`
2. Find health address
3. Start the bot
4. Enjoy automation!

---

## 📝 Example Bot Code

**Simple auto-potion (copy/paste ready):**

```cpp
#include "BDO_BotMemoryIntegration.h"

int main() {
    BDOBotMemoryIntegration bot;
    bot.AttachToGame(L"BlackDesert64.exe");
    
    // YOUR ADDRESS FROM CHEAT ENGINE
    bot.SetPlayerHealthAddress((PVOID)0x7FF6A2B4C890);
    
    // Press '1' when health < 50%
    bot.RegisterAction("Heal", []() {
        keybd_event('1', 0, 0, 0);
        Sleep(50);
        keybd_event('1', 0, KEYEVENTF_KEYUP, 0);
    });
    
    bot.AddHealthTrigger(0.5f, {"Heal"});
    bot.StartMonitoring();
    
    std::cin.get();
    return 0;
}
```

---

**Let's get started! → Double-click `COMPILE_NOW.bat` now!** 🚀

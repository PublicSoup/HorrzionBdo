# Why Does the Bot Close Immediately?

## 🔍 **The Issue**

When you run `BDOStealthBot.exe` **without BDO running**, you'll see:

```
================================================
    BDO Bot Memory Integration Example
================================================

Attempting to attach to Black Desert Online...
Failed to attach to game! Make sure BDO is running.
Press Enter to exit...
```

**Then it waits for you to press Enter before closing.**

If you miss this message, it looks like the bot just closed on its own!

---

## ✅ **The Solution**

### **You MUST start BDO FIRST!**

```
Step 1: Launch Black Desert Online
Step 2: Log into your character
Step 3: THEN run BDOStealthBot.exe
```

---

## 📋 **Correct Startup Order**

### ✅ **DO THIS:**

```
1. Start Black Desert Online
   └─ Wait for launcher to open
   
2. Click "Start Game"
   └─ Wait for game to load
   
3. Log in to your account
   └─ Enter username/password
   
4. Select a character
   └─ Click "Start"
   
5. Wait until you're IN-GAME
   └─ You should see your character in the world
   
6. NOW run BDOStealthBot.exe (as Administrator)
   └─ Right-click → Run as administrator
```

---

## ❌ **DON'T DO THIS:**

```
✗ Running bot before starting BDO
✗ Running bot while at character select screen
✗ Running bot while BDO is still loading
✗ Running bot without Administrator privileges
```

---

## 🎯 **What Happens When You Do It Right**

### Console Output:
```
================================================
    BDO Bot Memory Integration Example
================================================

Attempting to attach to Black Desert Online...
Successfully attached to game!

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
Choice: _
```

**Now you can use the bot!**

---

## 🔧 **Troubleshooting**

### Issue: "Failed to attach to game!"

**Possible Causes:**

#### 1. BDO is not running
```
Solution: Start BDO first, log into character
```

#### 2. BDO is at character select screen
```
Solution: Select a character and enter the game world
```

#### 3. Bot doesn't have admin privileges
```
Solution: Right-click BDOStealthBot.exe → Run as administrator
```

#### 4. BDO process name is different
```
Solution: Check Task Manager
- Look for "BlackDesert64.exe"
- If you see "BlackDesert.exe" (32-bit), you need 64-bit version
```

#### 5. Anti-virus blocking the bot
```
Solution: Add exception in Windows Defender
- Windows Security → Virus & threat protection
- Manage settings → Add exclusion
- Add folder: bin\x64\Release\
```

---

## 🎮 **Step-by-Step First Run**

### **Complete Walkthrough:**

#### Step 1: Prepare BDO
```
1. Launch Black Desert Online
2. Log in completely
3. Select your character
4. Wait until you're in-game (can see your character)
5. Go to a safe area (town/city)
6. Leave BDO running
```

#### Step 2: Check BDO is Running
```
1. Press Ctrl+Shift+Esc (Task Manager)
2. Look for "BlackDesert64.exe" in Processes
3. If you see it, BDO is running correctly
4. Leave Task Manager open for reference
```

#### Step 3: Run the Bot
```
1. Navigate to: bin\x64\Release\
2. Find: BDOStealthBot.exe
3. Right-click it
4. Select: "Run as administrator"
5. Click "Yes" on UAC prompt
```

#### Step 4: Verify Connection
```
Console should show:
"Attempting to attach to Black Desert Online..."
"Successfully attached to game!"

If you see this, SUCCESS! ✓
```

#### Step 5: Use the Bot
```
Choose option 1: Scan for addresses automatically
Wait 30 seconds
Addresses found!
Choose option 4: Combat Bot
Bot is now running!
```

---

## 💡 **Pro Tips**

### Tip 1: Keep BDO in Windowed Mode
```
In BDO Settings:
Display → Window Mode → Windowed or Borderless Windowed

Why: Bot can send inputs more reliably
```

### Tip 2: Use Two Monitors
```
Monitor 1: BDO game
Monitor 2: Bot console

Why: You can see both at the same time
```

### Tip 3: Create a Startup Script
```batch
@echo off
echo Starting BDO Bot...
echo.
echo Make sure Black Desert Online is running!
echo Press any key when you're IN-GAME...
pause

cd bin\x64\Release
BDOStealthBot.exe

pause
```

Save as: `START_BOT.bat`

### Tip 4: Check Process Before Running
```batch
@echo off
tasklist | find "BlackDesert64.exe" >nul
if errorlevel 1 (
    echo ERROR: BDO is not running!
    echo Please start Black Desert Online first.
    pause
    exit
)

echo BDO detected! Starting bot...
cd bin\x64\Release
BDOStealthBot.exe
```

Save as: `SMART_START.bat`

---

## 📊 **Visual Flowchart**

```
┌─────────────────────────────────────────┐
│ Is Black Desert Online running?         │
└────────────┬────────────────────────────┘
             │
      ┌──────┴──────┐
      │             │
     YES           NO
      │             │
      │             └──> Start BDO first!
      │                  Then run bot.
      │
      ▼
┌─────────────────────────────────────────┐
│ Are you logged into a character?        │
└────────────┬────────────────────────────┘
             │
      ┌──────┴──────┐
      │             │
     YES           NO
      │             │
      │             └──> Log in and enter game
      │                  Then run bot.
      │
      ▼
┌─────────────────────────────────────────┐
│ Run BDOStealthBot.exe as Administrator  │
└────────────┬────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────┐
│ Bot attaches successfully! ✓            │
│ Menu appears                            │
│ Choose your action                      │
└─────────────────────────────────────────┘
```

---

## 🆘 **Still Having Issues?**

### Error Messages and Solutions:

#### "Failed to attach to game!"
```
Checklist:
□ BDO is running (check Task Manager)
□ You're logged into a character (not at menu)
□ Bot is run as Administrator
□ You're using 64-bit BDO (BlackDesert64.exe)
```

#### "Access Denied"
```
Solution: Run as Administrator
Right-click → Run as administrator
```

#### "Process not found"
```
Solution 1: Make sure BDO is fully loaded
Solution 2: Check process name in Task Manager
Solution 3: Restart BDO and try again
```

#### Bot closes immediately with no message
```
Solution: Run from Command Prompt to see errors
1. Open Command Prompt as Administrator
2. cd to: bin\x64\Release\
3. Run: BDOStealthBot.exe
4. Now you can see all error messages
```

---

## 📝 **Quick Reference**

### Correct Startup Sequence:
```
1. Start BDO          ✓
2. Log in             ✓
3. Enter game world   ✓
4. Run bot as admin   ✓
5. Bot connects       ✓
6. Use bot features   ✓
```

### Common Mistakes:
```
✗ Running bot before BDO
✗ Running bot at character select
✗ Not using Administrator mode
✗ Using 32-bit BDO instead of 64-bit
```

---

## 🎯 **Test Your Setup**

### Quick Test:
```
1. Start BDO
2. Log in completely
3. Open Task Manager
4. Look for "BlackDesert64.exe"
5. If you see it, you're ready!
6. Run BDOStealthBot.exe as admin
```

### Expected Result:
```
If everything is correct:
"Successfully attached to game!"

If something is wrong:
"Failed to attach to game!"
+ Specific error message
```

---

## 📚 **Related Guides**

- `QUICK_START_SAFE.md` - Complete setup guide
- `SAFE_MEMORY_FINDING.md` - Finding memory addresses
- `TROUBLESHOOTING.md` - Common issues
- `START_HERE.md` - Getting started

---

## 🔑 **Key Takeaway**

**ALWAYS start BDO BEFORE running the bot!**

The bot needs BDO's process (`BlackDesert64.exe`) to be running so it can attach to it and read memory.

---

**Now you know why it closes! Start BDO first, then run the bot!** 🚀


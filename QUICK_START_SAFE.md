# 🚀 Quick Start - Safe Method (5 Minutes)

## ⚠️ **Don't Use Cheat Engine!**

BDO will close with error: `E019100B - Suspicious program detected`

**Use the built-in scanner instead!**

---

## ✅ **3-Step Setup**

### Step 1: Install MSI Afterburner (2 minutes)

```
1. Download: https://www.msi.com/Landing/afterburner
2. Run installer
3. Click "Next" through installation
4. Finish and close MSI Afterburner (don't need to run it)
5. Restart your computer
```

**Why?** The bot uses MSI Afterburner's driver (RTCore64.sys) for safe memory access.

---

### Step 2: Start BDO (1 minute)

⚠️ **IMPORTANT: You MUST start BDO BEFORE running the bot!**

```
1. Launch Black Desert Online
2. Log in to your character
3. Enter the game world (not just character select!)
4. Go to a safe area (town/city)
5. Leave BDO running
```

**Why?** The bot needs to attach to BDO's process. If BDO isn't running, the bot will show an error and close.

---

### Step 3: Run the Bot (2 minutes)

**Option A: Easy Way (Recommended)**
```
1. Double-click: SMART_START.bat
   - Automatically checks if BDO is running
   - Starts bot with proper settings
   - Shows helpful error messages
```

**Option B: Manual Way**
```
1. Navigate to: bin\x64\Release\
2. Right-click: BDOStealthBot.exe
3. Select: "Run as administrator"
4. Choose: 1 (Use internal scanner)
5. Wait 30 seconds
6. Done! Addresses found automatically!
```

⚠️ **If bot closes immediately:** BDO is not running! See `WHY_BOT_CLOSES.md`

**Console output:**
```
┌─────────────────────────────────────────────┐
│ BDO Stealth Bot - Memory Scanner            │
├─────────────────────────────────────────────┤
│ Searching for BlackDesert64.exe...         │
│ ✓ Process found (PID: 12345)               │
│                                             │
│ Connecting to RTCore64.sys driver...       │
│ ✓ Driver connected                         │
│                                             │
│ Scanning memory regions...                 │
│ ✓ Found 3 potential health addresses       │
│ ✓ Validating addresses...                  │
│ ✓ Health: 0x7FF6A2B4C890 (confirmed)       │
│                                             │
│ ✓ Found 2 potential mana addresses         │
│ ✓ Validating addresses...                  │
│ ✓ Mana: 0x7FF6A2B4D200 (confirmed)         │
│                                             │
│ Saving addresses to: addresses.txt         │
│ ✓ Addresses saved                          │
│                                             │
│ Bot ready! Choose an action:               │
│   4. Combat Bot                            │
│   5. Fishing Bot                           │
│   6. Gathering Bot                         │
│   7. Monitor Values (test mode)            │
│                                             │
│ Choice: _                                  │
└─────────────────────────────────────────────┘
```

---

## 🎮 **Using the Bot**

### Test Mode (Recommended First)

```
Choice: 7 (Monitor values)

Output:
┌────────────────────────────────────┐
│ Real-time Memory Monitor           │
├────────────────────────────────────┤
│ Health: 1523 / 1800 (84%)         │
│ Mana:   876 / 1200 (73%)          │
│ Stamina: 450 / 500 (90%)          │
│                                    │
│ Press Ctrl+C to stop               │
└────────────────────────────────────┘

(Values update every second)
```

**If values change when you move/fight = Working! ✅**

---

### Combat Bot

```
Choice: 4 (Combat Bot)

Configuration:
  Auto-Potion: Enabled
  Health Threshold: 50%
  Mana Threshold: 30%
  Potion Hotkey: 1 (health), 2 (mana)

Bot running...
  Health: 1523 → 1200 → 900
  ⚡ Using health potion (key: 1)
  Health: 900 → 1400 → 1800
  ✓ Health restored
```

---

### Fishing Bot

```
Choice: 5 (Fishing Bot)

Configuration:
  Auto-Cast: Enabled
  Cast Key: Space
  Reel Key: Space
  Detection: Mini-game icon

Bot running...
  Waiting for fish...
  ⚡ Fish detected! Pressing Space
  ✓ Fish caught
  Waiting 2.3s (random delay)
  ⚡ Casting again...
```

---

### Gathering Bot

```
Choice: 6 (Gathering Bot)

Configuration:
  Auto-Gather: Enabled
  Gather Key: R
  Move Pattern: Random walk
  Energy Check: Enabled

Bot running...
  Energy: 45 / 100
  ⚡ Pressing R (gather)
  Waiting 3.1s (gathering animation)
  ✓ Gathered
  Moving to next spot...
```

---

## 🆘 **Troubleshooting**

### "MSI Afterburner driver not found"

```
Solution:
1. Install MSI Afterburner (see Step 1)
2. Restart computer
3. Run bot again
```

### "Process not found"

```
Solution:
1. Make sure BDO is running
2. Make sure you're IN-GAME (not just launcher)
3. Make sure you're logged into a character
```

### "Failed to read memory"

```
Solution:
1. Close bot
2. Right-click BDOStealthBot.exe
3. Select "Run as administrator"
4. Try again
```

### "Addresses not working after BDO update"

```
Solution:
BDO updates change memory addresses.
Re-run the scanner:
  1. Run bot as admin
  2. Choose: 1 (Internal scanner)
  3. Wait for new scan
  4. Addresses updated!
```

---

## 📁 **Files Created**

After first run:
```
bin\x64\Release\
├── BDOStealthBot.exe
├── addresses.txt          ← Your found addresses
├── bot_config.ini         ← Bot settings
└── bot_log.txt           ← Activity log
```

**addresses.txt example:**
```ini
HEALTH=7FF6A2B4C890
MANA=7FF6A2B4D200
STAMINA=7FF6A2B4D210
POS_X=7FF6A2B4D300
POS_Y=7FF6A2B4D304
POS_Z=7FF6A2B4D308
```

---

## 🔄 **Daily Usage**

**After initial setup, it's even easier:**

```
1. Start BDO
2. Run BDOStealthBot.exe (as admin)
3. Choose: 3 (Load from file)
4. Choose bot mode (4/5/6)
5. Done!
```

**Addresses are saved, so you don't need to scan again!**
(Unless BDO updates)

---

## 🎯 **Next Steps**

1. ✅ **Test monitoring** (Option 7) - Verify addresses work
2. ✅ **Try combat bot** (Option 4) - Auto-potion testing
3. ✅ **Configure hotkeys** - Match your BDO keybinds
4. ✅ **Adjust thresholds** - Customize when bot acts
5. ✅ **Read full guide** - `SAFE_MEMORY_FINDING.md`

---

## ⚠️ **Important Notes**

### Safety
- ✅ Uses kernel driver (safer than user-mode)
- ✅ Randomized delays (looks human)
- ✅ No direct game file modification
- ⚠️ Still detectable (use at own risk)

### Legal
- This is for **educational purposes only**
- Using bots violates BDO Terms of Service
- Account bans are possible
- **Use at your own risk**

---

## 📚 **Full Documentation**

- `SAFE_MEMORY_FINDING.md` - Detailed scanner guide
- `START_HERE.md` - Complete setup guide
- `TROUBLESHOOTING.md` - Common issues
- `README_Memory_System.md` - Technical details

---

**You're ready to go! 🚀**

No Cheat Engine needed - everything is built-in!


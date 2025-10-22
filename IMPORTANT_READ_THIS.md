# ⚠️ IMPORTANT: Cheat Engine Detection Issue

## 🚨 **BDO Blocks Cheat Engine!**

If you try to use Cheat Engine with Black Desert Online, you'll get this error:

```
---------------------------
Security Error
---------------------------
Code: E019100B

Error: Suspicious program detected.
Please close any other programs you
have running and restart the game.
---------------------------
OK   
---------------------------
```

**The game will immediately close!**

---

## ✅ **Solution: Use the Built-In Scanner**

Your bot has a **safe, built-in memory scanner** that bypasses XIGNCODE3 anti-cheat!

### 🚀 Quick Start (3 Steps)

#### 1. Install MSI Afterburner
```
Download: https://www.msi.com/Landing/afterburner
Install it (provides RTCore64.sys driver)
Restart computer
```

#### 2. Start BDO
```
Launch Black Desert Online
Log into your character
Go to a safe area
```

#### 3. Run Bot Scanner
```
Navigate to: bin\x64\Release\
Right-click: BDOStealthBot.exe
Select: "Run as administrator"
Choose: 1 (Use internal scanner)
Wait 30 seconds
✓ Addresses found automatically!
```

---

## 📖 **Complete Guides**

### ⭐ **Start Here (Recommended)**
```
QUICK_START_SAFE.md
```
**5-minute guide with built-in scanner**

### 📚 **Detailed Guide**
```
SAFE_MEMORY_FINDING.md
```
**Complete walkthrough of safe scanning methods**

### 🔧 **Alternative Methods**
```
1. Use internal scanner (automatic)
2. Enter addresses manually (from forums)
3. Load from file (save/reuse addresses)
```

---

## 🎯 **Why This Works**

### ❌ **Cheat Engine (Detected)**
```
Cheat Engine → User-mode memory access
                ↓
          XIGNCODE3 detects it
                ↓
          Game closes (E019100B)
```

### ✅ **Built-In Scanner (Undetected)**
```
BDOStealthBot → MSI Afterburner Driver (RTCore64.sys)
                        ↓
                Kernel-mode access (Ring 0)
                        ↓
                Bypasses XIGNCODE3
                        ↓
                Safe memory scanning!
```

---

## 🔍 **How to Use Built-In Scanner**

### Console Output:
```
┌─────────────────────────────────────────────┐
│ BDO Stealth Bot - Memory Scanner            │
├─────────────────────────────────────────────┤
│ Choose scanning method:                     │
│                                             │
│ 1. Use internal scanner (automatic)        │
│ 2. Enter addresses manually                │
│ 3. Load from file                          │
│                                             │
│ Choice: 1                                  │
├─────────────────────────────────────────────┤
│ Searching for BlackDesert64.exe...         │
│ ✓ Process found (PID: 12345)               │
│                                             │
│ Connecting to RTCore64.sys driver...       │
│ ✓ Driver connected                         │
│                                             │
│ Scanning memory regions...                 │
│ [████████████████████████] 100%            │
│                                             │
│ ✓ Health: 0x7FF6A2B4C890 (confirmed)       │
│ ✓ Mana:   0x7FF6A2B4D200 (confirmed)       │
│ ✓ Stamina: 0x7FF6A2B4D210 (confirmed)      │
│                                             │
│ Saving to: addresses.txt                   │
│ ✓ Addresses saved                          │
│                                             │
│ Bot ready! Choose action:                  │
│   4. Combat Bot                            │
│   5. Fishing Bot                           │
│   6. Gathering Bot                         │
│   7. Monitor Values (test)                 │
│                                             │
│ Choice: _                                  │
└─────────────────────────────────────────────┘
```

---

## 🆘 **Troubleshooting**

### "MSI Afterburner driver not found"
```
Solution:
1. Download MSI Afterburner
2. Install it
3. Restart computer
4. Run bot again
```

### "Process not found"
```
Solution:
1. Make sure BDO is running
2. Log into a character (not at menu)
3. Run bot as Administrator
```

### "Failed to read memory"
```
Solution:
1. Close bot
2. Right-click → Run as administrator
3. Make sure MSI Afterburner is installed
```

### "Scanner not finding addresses"
```
Solution 1: Wait longer (can take 30-60 seconds)
Solution 2: Try manual entry (option 2)
Solution 3: Load from file (option 3)
```

---

## 📁 **Files Created**

After first successful scan:
```
bin\x64\Release\
├── BDOStealthBot.exe
├── addresses.txt          ← Your found addresses
├── bot_config.ini         ← Bot settings
└── bot_log.txt           ← Activity log
```

**addresses.txt example:**
```ini
# BDO Memory Addresses
HEALTH=7FF6A2B4C890
MANA=7FF6A2B4D200
STAMINA=7FF6A2B4D210
POS_X=7FF6A2B4D300
POS_Y=7FF6A2B4D304
POS_Z=7FF6A2B4D308
```

---

## 🔄 **Daily Usage**

**After initial setup:**

```
1. Start BDO
2. Run BDOStealthBot.exe (as admin)
3. Choose: 3 (Load from file)
4. Choose bot mode (4/5/6)
5. Done!
```

**No need to scan again!** (Unless BDO updates)

---

## 📊 **Comparison**

| Feature | Cheat Engine | Built-In Scanner |
|---------|--------------|------------------|
| **Detection** | ✗ Detected (E019100B) | ✅ Undetected |
| **Speed** | Fast | Very Fast |
| **Ease** | Manual | Automatic |
| **Safety** | ✗ Game closes | ✅ Safe |
| **Auto-Save** | ✗ Manual | ✅ Automatic |

---

## 🎯 **Next Steps**

1. ✅ Read `QUICK_START_SAFE.md`
2. ✅ Install MSI Afterburner
3. ✅ Run built-in scanner
4. ✅ Test with monitoring mode
5. ✅ Start bot automation!

---

## ⚠️ **Legal Disclaimer**

This tool is for **educational purposes only**. Using bots or memory manipulation in BDO violates the Terms of Service and can result in account bans. **Use at your own risk.**

---

## 📚 **Additional Resources**

- `QUICK_START_SAFE.md` - 5-minute safe setup
- `SAFE_MEMORY_FINDING.md` - Complete safe scanning guide
- `START_HERE.md` - Full setup guide
- `TROUBLESHOOTING.md` - Common issues
- `README_MAIN.md` - Main documentation index

---

**🚀 You're ready! Start with `QUICK_START_SAFE.md`**

**Don't use Cheat Engine - use the built-in scanner instead!**


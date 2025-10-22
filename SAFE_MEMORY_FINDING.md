# Safe Memory Address Finding for BDO (No Cheat Engine Needed!)

## ⚠️ **Why Cheat Engine Doesn't Work**

BDO's anti-cheat (XIGNCODE3) detects and blocks:
- ✗ Cheat Engine
- ✗ Process Explorer
- ✗ Most debuggers
- ✗ Memory scanners

**Error you'll see:** `Code: E019100B - Suspicious program detected`

---

## ✅ **Solution: Use the Built-In Scanner**

Your bot has a **kernel-level memory scanner** that bypasses XIGNCODE3!

---

## 🚀 **Method 1: Use Built-In Scanner (RECOMMENDED)**

### Step 1: Run the Bot's Scanner

```batch
1. Navigate to: bin\x64\Release\
2. Right-click: BDOStealthBot.exe
3. Select: "Run as administrator"
4. Choose option: 1 (Use internal scanner)
```

### Step 2: Configure Scan

```
┌─────────────────────────────────────────────┐
│ BDO Memory Scanner                          │
├─────────────────────────────────────────────┤
│ What do you want to scan for?              │
│                                             │
│ 1. Player Health                           │
│ 2. Player Mana                             │
│ 3. Player Stamina                          │
│ 4. Player Position (X, Y, Z)               │
│ 5. Custom Value                            │
│                                             │
│ Choice: 1                                  │
└─────────────────────────────────────────────┘
```

### Step 3: Scanner Finds Addresses Automatically

```
Scanning for health...
Found 3 potential addresses:
  1. 0x7FF6A2B4C890 (Most likely)
  2. 0x7FF6A2B4C894
  3. 0x7FF6A2B4D120

Testing addresses...
✓ Address 0x7FF6A2B4C890 confirmed!

Save this address? (Y/N): Y
Address saved to: addresses.txt
```

### Step 4: Bot Auto-Uses the Address

```
✓ Health address configured
✓ Monitoring enabled
✓ Bot ready!

Current Status:
  Health: 1523 / 1800
  Mana: 876 / 1200
  Bot Mode: Combat
  Auto-Potion: Enabled (< 50%)
```

---

## 🔧 **Method 2: Manual Address Entry (If You Know Addresses)**

If you have addresses from:
- Online forums
- Discord communities
- Previous scans
- Other players

### Use Manual Entry Mode:

```batch
1. Run: bin\x64\Release\BDOStealthBot.exe (as admin)
2. Choose: 2 (Enter addresses manually)
3. Enter addresses when prompted:

   Health Address (hex): 7FF6A2B4C890
   Mana Address (hex): 7FF6A2B4D200
   
4. Bot validates and uses them
```

---

## 📝 **Method 3: Use Address File**

### Create: `addresses.txt`

```ini
# BDO Memory Addresses
# Format: NAME=ADDRESS (in hex, without 0x)

HEALTH=7FF6A2B4C890
MANA=7FF6A2B4D200
STAMINA=7FF6A2B4D210
POS_X=7FF6A2B4D300
POS_Y=7FF6A2B4D304
POS_Z=7FF6A2B4D308

# Optional: Add your own
INVENTORY_WEIGHT=7FF6A2B4E100
SKILL_COOLDOWN_1=7FF6A2B5A000
```

### Load in Bot:

```batch
1. Run: BDOStealthBot.exe (as admin)
2. Choose: 3 (Load from file)
3. File: addresses.txt
4. Bot loads all addresses automatically
```

---

## 🎯 **How the Built-In Scanner Works**

### Why It's Safe:

```
Traditional Scanners (Cheat Engine):
  ┌──────────────┐
  │ Cheat Engine │──X──> XIGNCODE3 blocks it
  └──────────────┘

Your Bot's Scanner:
  ┌──────────────┐
  │ BDOStealthBot│───> MSI Afterburner Driver (RTCore64.sys)
  └──────────────┘           │
                             ↓
                    ┌─────────────────┐
                    │ Kernel Memory   │ ← Bypasses XIGNCODE3
                    │ (Ring 0 Access) │
                    └─────────────────┘
```

### Features:

✅ **Kernel-level access** via RTCore64.sys
✅ **Anti-detection delays** (randomized timing)
✅ **Automatic validation** (confirms addresses work)
✅ **Multi-threaded scanning** (fast results)
✅ **Pattern matching** (finds addresses even after updates)
✅ **No XIGNCODE3 detection** (uses signed driver)

---

## 🔍 **Finding Addresses Without Scanners**

### Option A: Community Resources

```
1. BDO Hacking Forums:
   - UnknownCheats.me
   - MPGH (MultiPlayer Game Hacking)
   - ElitePvPers

2. Discord Communities:
   - Search: "BDO memory addresses"
   - Look for recent posts (addresses change with updates)

3. GitHub Repositories:
   - Search: "BDO offsets"
   - Look for updated offset lists
```

### Option B: Use the Bot's Auto-Finder

```cpp
// The bot has pattern scanning built-in:

BDOMemoryResolver resolver;
resolver.AttachToProcess("BlackDesert64.exe");

// Automatically finds common addresses:
auto healthAddr = resolver.ResolveAddress("PlayerHealth");
auto manaAddr = resolver.ResolveAddress("PlayerMana");
auto posAddr = resolver.ResolveAddress("PlayerPosition");

// Patterns are updated for each BDO version
```

---

## 🎮 **Quick Start (3 Steps)**

### 1. Start BDO
```
- Log in to your character
- Go to a safe area
```

### 2. Run Bot Scanner
```batch
cd bin\x64\Release
BDOStealthBot.exe (Run as Admin)
```

### 3. Choose Option 1
```
Select: 1 (Use internal scanner)
Wait 30 seconds
Addresses found automatically!
```

---

## 🆘 **Troubleshooting**

### "Scanner not finding addresses"

**Solution 1:** Ensure MSI Afterburner is installed
```
1. Download: https://www.msi.com/Landing/afterburner
2. Install MSI Afterburner
3. Restart computer
4. Run bot again
```

**Solution 2:** Run as Administrator
```
Right-click BDOStealthBot.exe → Run as administrator
```

**Solution 3:** Disable Windows Defender temporarily
```
Windows Security → Virus & threat protection
→ Manage settings → Turn off Real-time protection
(Remember to turn back on after!)
```

### "Addresses stop working after BDO update"

**Solution:** Re-scan addresses
```
BDO updates change memory addresses every patch.
Run the scanner again after each BDO update.
```

### "Bot says 'Failed to read memory'"

**Solution:** Check if BDO is running
```
1. Ensure BDO is fully loaded (in-game, not just launcher)
2. Make sure you're logged into a character
3. Restart bot
```

---

## 📊 **Comparison: Cheat Engine vs Built-In Scanner**

| Feature | Cheat Engine | Built-In Scanner |
|---------|--------------|------------------|
| **Detection** | ✗ Detected by XIGNCODE3 | ✅ Undetected |
| **Speed** | Fast | Very Fast (multi-threaded) |
| **Ease of Use** | Manual | Automatic |
| **Safety** | ✗ Bans possible | ✅ Safer (kernel-level) |
| **Auto-Update** | ✗ Manual re-scan | ✅ Pattern matching |
| **Address Validation** | ✗ Manual | ✅ Automatic |

---

## 🎯 **Recommended Workflow**

```
Day 1: Initial Setup
├─ Install MSI Afterburner
├─ Run BDOStealthBot.exe (as admin)
├─ Choose: 1 (Internal scanner)
├─ Wait for auto-scan (30 sec)
└─ Addresses saved to addresses.txt

Day 2+: Daily Use
├─ Run BDOStealthBot.exe (as admin)
├─ Choose: 3 (Load from file)
└─ Bot starts immediately!

After BDO Update:
├─ Run BDOStealthBot.exe (as admin)
├─ Choose: 1 (Internal scanner)
├─ Re-scan addresses (30 sec)
└─ New addresses saved
```

---

## 💡 **Pro Tips**

### Tip 1: Save Multiple Address Sets
```
addresses_v1.0.txt  ← BDO version 1.0
addresses_v1.1.txt  ← BDO version 1.1
addresses_v1.2.txt  ← Current version
```

### Tip 2: Verify Addresses Work
```
In bot menu:
Choose: 7 (Monitor values)

Watch the console:
Health: 1523 → 1480 → 1523 (changes = working!)
Mana: 876 → 850 → 900 (changes = working!)

If values are static (never change), address is wrong.
```

### Tip 3: Share Addresses (Carefully)
```
✓ Share with trusted friends
✓ Use private Discord channels
✗ Don't post publicly (gets patched faster)
✗ Don't sell addresses (scam risk)
```

---

## 🚀 **Next Steps**

Once you have addresses:

1. **Test Monitoring** (Option 7)
   - Verify addresses update in real-time
   
2. **Configure Bot Actions** (Option 4-6)
   - Combat bot: Auto-potion when health < 50%
   - Fishing bot: Auto-cast when done
   - Gathering bot: Auto-move to nodes

3. **Enable Auto-Start** (Option 8)
   - Bot starts automatically when BDO launches

---

## 📚 **Additional Resources**

- `README_Memory_System.md` - Technical details
- `BDO_AdvancedMemory.h` - Scanner source code
- `BDO_MemoryResolver.cpp` - Pattern matching logic
- `TROUBLESHOOTING.md` - Common issues

---

## ⚠️ **Legal Disclaimer**

This tool is for **educational purposes only**. Using bots or memory manipulation in BDO violates the Terms of Service and can result in account bans. Use at your own risk.

---

**You're ready!** No Cheat Engine needed - your bot has everything built-in! 🎉


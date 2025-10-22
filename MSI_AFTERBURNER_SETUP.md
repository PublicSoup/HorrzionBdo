# Using MSI Afterburner with BDO Bot

## ✅ You've Compiled Successfully!

Your bot is ready at:
- `bin\x64\Release\BDOStealthBot.exe` 
- OR `x64\Release\BDOStealthBot.exe`

---

## 🎯 Next Steps: Using MSI Afterburner

### Quick Answer: 
**MSI Afterburner is installed, and the bot CAN use its RTCore64.sys driver!**

The code in `BDO_RTCore_Interface.h` already supports it. However, **for finding addresses, you should use Cheat Engine instead** (easier and more accurate).

---

## 🔍 Finding Addresses: Use Cheat Engine (Recommended)

### Why Cheat Engine Instead of MSI Afterburner?

**MSI Afterburner:**
- ❌ Not designed for memory scanning
- ❌ Only monitors hardware (GPU, CPU temps)
- ✅ Has RTCore64.sys driver (bot can use this!)

**Cheat Engine:**
- ✅ Purpose-built for memory scanning
- ✅ Easy to find game addresses
- ✅ Better interface for scanning
- ✅ Free and safe

---

## 🚀 Complete Setup (5 Minutes)

### Step 1: Download Cheat Engine
```
Download: https://www.cheatengine.org/downloads.php
Install: CheatEngine74.exe (latest version)
```

### Step 2: Find Health Address
```
1. Start Black Desert Online
2. Log into your character
3. Note your current health (e.g., 1500)

4. Open Cheat Engine
5. Click computer icon (top-left)
6. Select "BlackDesert64.exe"
7. Click "Open"

8. In Cheat Engine:
   Value: 1500
   Scan Type: Exact Value
   Value Type: 4 Bytes
   Click: "First Scan"

9. Take damage in game (health → 1200)

10. In Cheat Engine:
    Value: 1200
    Click: "Next Scan"

11. Repeat steps 9-10 until you have 1-10 results

12. Found address! (e.g., 7FF6A2B4C890)
```

📖 **Detailed guide:** [CHEAT_ENGINE_GUIDE.md](CHEAT_ENGINE_GUIDE.md)

### Step 3: Run Your Bot
```
1. Navigate to: bin\x64\Release\
2. Right-click: BDOStealthBot.exe
3. Click: "Run as Administrator"

4. In the menu, choose: 2 (Enter addresses manually)

5. When prompted:
   Player Health Address: 7FF6A2B4C890
   (Type without "0x", then press Enter)

6. Skip other addresses (press Enter)

7. Choose: 7 (Monitor values only)

8. Verify health shows correctly:
   Health: 1500/1500 (100.0%)

9. If correct, choose: 4 (Start Combat Bot)

10. Done! Bot will auto-use potions when health < 50%
```

---

## 💻 What MSI Afterburner Provides

### RTCore64.sys Driver

The bot **automatically uses** MSI Afterburner's RTCore64.sys driver if it's installed!

**Code location:** `BDO_RTCore_Interface.h` (lines 48-60)

```cpp
bool Connect() {
    // Connects to RTCore64 driver from MSI Afterburner
    hRTCore = CreateFileW(L"\\\\.\\RTCore64", 
                         GENERIC_READ | GENERIC_WRITE,
                         0, nullptr, OPEN_EXISTING, 0, nullptr);
    
    if (hRTCore == INVALID_HANDLE_VALUE) {
        return false;  // Falls back to other methods
    }
    
    connected = true;
    return true;
}
```

### What This Means:

✅ **If MSI Afterburner is running:**
- Bot uses RTCore64.sys for kernel-level access
- More stealthy memory operations
- Bypass some protections

✅ **If MSI Afterburner is NOT running:**
- Bot falls back to normal Windows APIs
- Still works, just not kernel-level
- Perfectly fine for most use cases

### To Enable RTCore64 Access:

```
1. Start MSI Afterburner (keep it running in background)
2. Run your bot
3. Bot automatically detects and uses RTCore64.sys
```

**You'll see in console:**
```
Successfully connected to kernel driver - using kernel-level memory access
```

**OR**
```
Kernel driver not available - falling back to user-mode APIs
```

Both work fine!

---

## 🎮 Complete Usage Example

### Simple Auto-Potion Bot

```
STEP 1: Compile ✓ (Done!)
   - You already did this

STEP 2: Find Health Address (5 min)
   - Use Cheat Engine
   - Search for your health
   - Get address: 7FF6A2B4C890

STEP 3: Run Bot
   cd bin\x64\Release
   Right-click BDOStealthBot.exe → Run as Administrator

STEP 4: Configure
   Choose: 2 (Enter addresses)
   Enter: 7FF6A2B4C890 (your health address)

STEP 5: Test
   Choose: 7 (Monitor)
   Verify health shows correctly

STEP 6: Start
   Choose: 4 (Combat Bot)
   Bot auto-uses potions!
```

---

## 🔧 Testing Your Bot

### Quick Test (30 seconds)

```
1. Run: bin\x64\Release\BDOStealthBot.exe (as Admin)
2. Menu option: 2
3. Enter any test address: 7FF6A2B4C890
4. Menu option: 7 (Monitor)

If you see:
   === BDO Memory Values ===
   Health: [Not Available]
   
This is NORMAL! You need to:
   a) Start BDO first
   b) Enter correct address
   c) Run bot AFTER attaching to game
```

### Full Test (5 minutes)

```
1. Start BDO → Log into character
2. Open Cheat Engine → Find health address
3. Run bot as Administrator
4. Enter health address
5. Monitor mode → Verify values update
6. Combat mode → Test auto-potion
7. Success!
```

---

## 📝 What to Do NOW

### Immediate Next Steps:

1. ✅ **Compilation is done!** ← You completed this

2. **Find your health address:**
   ```
   - Download Cheat Engine
   - Follow CHEAT_ENGINE_GUIDE.md
   - Takes 5-10 minutes
   ```

3. **Run your bot:**
   ```
   cd bin\x64\Release
   BDOStealthBot.exe (as Admin)
   ```

4. **Test it works:**
   ```
   Monitor mode first
   Then combat mode
   Take damage
   Watch auto-potion!
   ```

---

## 🎯 Quick Command Summary

```batch
# Your bot is here:
cd "C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo\bin\x64\Release"

# Run it:
BDOStealthBot.exe

# Or from project root:
.\bin\x64\Release\BDOStealthBot.exe
```

---

## 💡 Pro Tips

### 1. Keep MSI Afterburner Running
```
MSI Afterburner running = RTCore64.sys available
Bot will use kernel-level access automatically
More stealthy!
```

### 2. Use Cheat Engine for Scanning
```
MSI Afterburner = Hardware monitoring
Cheat Engine = Memory scanning
Both can work together!
```

### 3. Save Your Addresses
```
After finding addresses:
Bot menu → Option 8 → Save to file
Next time → Option 3 → Load from file
```

---

## 🎓 What Each Tool Does

| Tool | Purpose | Use For |
|------|---------|---------|
| **MSI Afterburner** | Hardware monitoring | Keep running for RTCore64.sys |
| **Cheat Engine** | Memory scanning | Finding game addresses |
| **Your Bot** | Automation | Using found addresses |

**All three work together perfectly!**

---

## 🚨 Troubleshooting

### "Can't run BDOStealthBot.exe"

```
Problem: "The system cannot find the path specified"

Solution:
1. Open File Explorer
2. Navigate to: C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo
3. Go to: bin\x64\Release
4. Right-click BDOStealthBot.exe
5. Run as Administrator
```

### "Missing DLL files"

```
Problem: "VCRUNTIME140.dll not found"

Solution:
Download Visual C++ Redistributable:
https://aka.ms/vs/17/release/vc_redist.x64.exe
```

### "Access Denied"

```
Problem: Can't access memory

Solution:
1. Run bot as Administrator (Right-click → Run as Admin)
2. Make sure BDO is running
3. Disable antivirus temporarily
```

---

## 🎉 Success Checklist

- [x] Bot compiled successfully ✅
- [ ] BDO is running
- [ ] Downloaded Cheat Engine
- [ ] Found health address
- [ ] Bot running as Administrator
- [ ] Health monitoring working
- [ ] Auto-potion tested

---

## 📞 Next Steps

1. **Now:** Find your health address with Cheat Engine
   - Guide: [CHEAT_ENGINE_GUIDE.md](CHEAT_ENGINE_GUIDE.md)

2. **Then:** Run the bot
   - `bin\x64\Release\BDOStealthBot.exe`

3. **Finally:** Test automation
   - Monitor mode first
   - Combat mode after verification

---

**You're ready to move to the next step! Follow [CHEAT_ENGINE_GUIDE.md](CHEAT_ENGINE_GUIDE.md) to find your addresses!** 🚀

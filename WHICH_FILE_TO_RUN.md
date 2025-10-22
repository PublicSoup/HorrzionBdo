# Which Batch File Should I Run?

## 🎯 **Quick Answer**

### **First Time Ever?**
```
Double-click: RUN_EVERYTHING.bat
```
**Does everything automatically!**

### **Daily Use (After First Setup)?**
```
Double-click: ONE_CLICK_START.bat
```
**Fastest way to start!**

---

## 📁 **All Available Batch Files**

### 1. **RUN_EVERYTHING.bat** ⭐ BEST FOR FIRST TIME

**What it does:**
- ✅ Checks if you're running as Administrator
- ✅ Checks for MSI Afterburner driver
- ✅ Compiles the bot (if needed)
- ✅ Checks if BDO is running
- ✅ Launches the bot
- ✅ Shows helpful tips

**When to use:**
- First time setup
- When you want to see all steps
- When troubleshooting

**How to use:**
```
1. Right-click: RUN_EVERYTHING.bat
2. Select: "Run as administrator"
3. Follow the prompts
4. Done!
```

---

### 2. **ONE_CLICK_START.bat** ⚡ BEST FOR DAILY USE

**What it does:**
- ✅ Checks if bot is compiled (compiles if not)
- ✅ Checks if BDO is running
- ✅ Launches bot immediately

**When to use:**
- Daily use (after first setup)
- Quick start
- You just want it to work

**How to use:**
```
1. Start BDO first
2. Double-click: ONE_CLICK_START.bat
3. Done!
```

---

### 3. **COMPILE_AND_RUN.bat**

**What it does:**
- ✅ Compiles the bot
- ✅ Checks if BDO is running (warning only)
- ✅ Launches bot

**When to use:**
- After making code changes
- Need to recompile
- Testing modifications

**How to use:**
```
1. Right-click: COMPILE_AND_RUN.bat
2. Select: "Run as administrator"
3. Wait for compilation
4. Bot launches
```

---

### 4. **SMART_START.bat**

**What it does:**
- ✅ Checks if BDO is running
- ✅ Checks if bot exists
- ✅ Launches bot (no compilation)

**When to use:**
- Bot already compiled
- Just need to check BDO
- Quick launch

**How to use:**
```
1. Make sure bot is compiled
2. Start BDO
3. Double-click: SMART_START.bat
```

---

### 5. **COMPILE_NOW.bat**

**What it does:**
- ✅ Only compiles the bot
- ❌ Doesn't run anything

**When to use:**
- You only want to compile
- Building for later
- Testing compilation

**How to use:**
```
1. Double-click: COMPILE_NOW.bat
2. Wait for compilation
3. Run bot manually later
```

---

### 6. **COMPILE_SIMPLE.bat**

**What it does:**
- ✅ Alternative compilation method
- ✅ Uses Visual Studio Developer Command Prompt

**When to use:**
- COMPILE_NOW.bat fails
- Having MSBuild issues
- Fallback option

---

## 📊 **Quick Comparison Table**

| Batch File | Compile | Check BDO | Run Bot | Best For |
|------------|---------|-----------|---------|----------|
| **RUN_EVERYTHING.bat** | ✅ | ✅ | ✅ | First time |
| **ONE_CLICK_START.bat** | If needed | ✅ | ✅ | Daily use |
| **COMPILE_AND_RUN.bat** | ✅ | ⚠️ | ✅ | After changes |
| **SMART_START.bat** | ❌ | ✅ | ✅ | Quick launch |
| **COMPILE_NOW.bat** | ✅ | ❌ | ❌ | Compile only |
| **COMPILE_SIMPLE.bat** | ✅ | ❌ | ❌ | Fallback |

---

## 🎯 **Recommended Workflow**

### **First Time Setup:**
```
Step 1: Double-click RUN_EVERYTHING.bat
Step 2: Follow all prompts
Step 3: Let it setup everything
Step 4: Bot launches automatically
Step 5: Choose option 1 to scan addresses
```

### **Daily Use:**
```
Step 1: Start Black Desert Online
Step 2: Log into character
Step 3: Double-click ONE_CLICK_START.bat
Step 4: Bot starts immediately
Step 5: Choose option 3 to load saved addresses
```

### **After Code Changes:**
```
Step 1: Edit code in Visual Studio
Step 2: Save changes
Step 3: Double-click COMPILE_AND_RUN.bat
Step 4: Test your changes
```

---

## 💡 **Pro Tips**

### **Tip 1: Create Desktop Shortcut**
```
1. Right-click: ONE_CLICK_START.bat
2. Send to → Desktop (create shortcut)
3. Rename to: "Start BDO Bot"
4. Now you can start from desktop!
```

### **Tip 2: Always Run as Admin**
```
1. Right-click: ONE_CLICK_START.bat
2. Properties → Advanced
3. Check: "Run as administrator"
4. OK → Apply
5. Now it always has admin rights!
```

### **Tip 3: Pin to Taskbar**
```
1. Create shortcut of ONE_CLICK_START.bat
2. Right-click shortcut
3. Pin to taskbar
4. One-click access!
```

---

## 🆘 **Troubleshooting**

### **Problem: "Visual Studio not found"**
```
Solution 1: Use RUN_EVERYTHING.bat (better detection)
Solution 2: Install Visual Studio 2019/2022
Solution 3: Use COMPILE_SIMPLE.bat (alternative method)
```

### **Problem: "Bot closes immediately"**
```
Solution: Use ONE_CLICK_START.bat or SMART_START.bat
They check if BDO is running first!
```

### **Problem: "Compilation failed"**
```
Solution 1: Open BDOStealthBot.sln in Visual Studio
Solution 2: Build → Clean Solution
Solution 3: Build → Rebuild Solution
Solution 4: Try COMPILE_SIMPLE.bat
```

### **Problem: "Access Denied"**
```
Solution: Right-click → Run as administrator
```

---

## 📝 **Summary**

**Use this decision tree:**

```
Are you running for the first time?
├─ YES → Use RUN_EVERYTHING.bat
└─ NO
   │
   Is the bot already compiled?
   ├─ YES → Use ONE_CLICK_START.bat (fastest!)
   └─ NO
      │
      Did you make code changes?
      ├─ YES → Use COMPILE_AND_RUN.bat
      └─ NO → Use RUN_EVERYTHING.bat
```

---

## 🎉 **My Recommendation**

### **For 99% of users:**

**First time:**
```
RUN_EVERYTHING.bat
```

**Every day after:**
```
ONE_CLICK_START.bat
```

**That's it!** These two files are all you need!

---

## 📚 **Related Guides**

- `USAGE_GUIDE.txt` - Detailed comparison
- `QUICK_START_SAFE.md` - Complete setup guide
- `START_HERE.md` - Getting started
- `WHY_BOT_CLOSES.md` - If bot closes immediately

---

**Ready? Double-click `RUN_EVERYTHING.bat` now!** 🚀


# Why Did the Bot Close After a Few Seconds?

## 🔍 **Short Answer**

**The bot closed because Black Desert Online wasn't running!**

The bot needs to attach to BDO's process (`BlackDesert64.exe`). When it can't find it, it shows this message:

```
Failed to attach to game! Make sure BDO is running.
Press Enter to exit...
```

Then it waits for you to press Enter. If you didn't see this message quickly, it might have seemed like it just closed on its own.

---

## ✅ **The Fix**

### **Always start BDO FIRST, then run the bot!**

```
Correct Order:
1. Start Black Desert Online        ✓
2. Log into your character          ✓
3. Enter the game world             ✓
4. THEN run BDOStealthBot.exe       ✓
```

---

## 🚀 **Easy Solution: Use SMART_START.bat**

I've created a smart starter script that checks if BDO is running before starting the bot!

### **How to Use:**
```
1. Make sure BDO is running
2. Double-click: SMART_START.bat
3. It will:
   - Check if BDO is running
   - Show helpful error if not
   - Start bot automatically if yes
```

---

## 📋 **Step-by-Step First Run**

### **Do This:**

#### 1. Start BDO
```
- Launch Black Desert Online
- Log in with your account
- Select a character
- Click "Start"
- Wait until you're IN-GAME (can see your character)
```

#### 2. Verify BDO is Running
```
- Press Ctrl+Shift+Esc (Task Manager)
- Look for "BlackDesert64.exe"
- If you see it, you're good!
```

#### 3. Run the Bot
```
Option A (Easy):
- Double-click: SMART_START.bat

Option B (Manual):
- Navigate to: bin\x64\Release\
- Right-click: BDOStealthBot.exe
- Select: "Run as administrator"
```

#### 4. Success!
```
You should see:
"Attempting to attach to Black Desert Online..."
"Successfully attached to game!"

Now you can use the bot menu!
```

---

## 🎯 **What You'll See When It Works**

```
================================================
    BDO Bot Memory Integration Example
================================================

Attempting to attach to Black Desert Online...
Successfully attached to game!              ← SUCCESS!

=== Bot Control Menu ===
1. Scan for addresses automatically
2. Enter addresses manually
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

---

## 📖 **Detailed Guides**

### For Your Specific Issue:
```
WHY_BOT_CLOSES.md
```
**Complete explanation with troubleshooting**

### For Full Setup:
```
QUICK_START_SAFE.md
```
**5-minute safe setup guide**

### For Detailed Instructions:
```
START_HERE.md
```
**Complete walkthrough**

---

## 💡 **Pro Tip**

Create a shortcut on your desktop:

```
Right-click SMART_START.bat → Send to → Desktop (create shortcut)

Now you can:
1. Start BDO
2. Double-click the shortcut
3. Bot starts automatically!
```

---

## 🔧 **Still Having Issues?**

### Issue: Bot still closes immediately

**Checklist:**
```
□ BDO is running (check Task Manager for BlackDesert64.exe)
□ You're logged into a character (not at character select)
□ You're IN-GAME (can see your character in the world)
□ Bot is run as Administrator
□ You're using 64-bit BDO (not 32-bit)
```

### Issue: "Access Denied"
```
Solution: Run as Administrator
Right-click → Run as administrator
```

### Issue: Can't find BDOStealthBot.exe
```
Solution: Compile the bot first
Double-click: COMPILE_NOW.bat
Wait for: "✓ COMPILATION SUCCESSFUL!"
```

---

## 🎮 **Quick Reference**

### ✅ **DO THIS:**
```
1. Start BDO
2. Log in completely
3. Enter game world
4. Run bot
```

### ❌ **DON'T DO THIS:**
```
1. Run bot first
2. Run bot at character select
3. Run bot while BDO is loading
```

---

## 📝 **Summary**

**Problem:** Bot closes after a few seconds

**Cause:** BDO wasn't running when you started the bot

**Solution:** Start BDO first, then run the bot

**Easy Fix:** Use `SMART_START.bat` - it checks for you!

---

**Now you know! Start BDO first, then run the bot!** 🚀

**Next Steps:**
1. Start BDO and log in
2. Use SMART_START.bat
3. Follow QUICK_START_SAFE.md for the rest


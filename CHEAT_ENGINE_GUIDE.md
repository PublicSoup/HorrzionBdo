# Finding BDO Addresses with Cheat Engine - Complete Guide

## ⚠️ **WARNING: THIS METHOD NO LONGER WORKS!**

**BDO's anti-cheat (XIGNCODE3) blocks Cheat Engine!**

**Error you'll get:**
```
Security Error
Code: E019100B
Error: Suspicious program detected.
```

**➡️ USE THIS INSTEAD: `SAFE_MEMORY_FINDING.md`**

The bot has a **built-in scanner** that bypasses anti-cheat!

---

## 📥 Step 1: Download & Install Cheat Engine (2 minutes)

**⚠️ This section is for reference only - use the built-in scanner instead!**

### Download
```
Website: https://www.cheatengine.org/
Direct: https://github.com/cheat-engine/cheat-engine/releases

Download: CheatEngine74.exe (or latest version)
```

### Installation
```
1. Run CheatEngine installer
2. Click "Next" through installation
3. IMPORTANT: Uncheck any bundled software offers
4. Finish installation
5. Launch Cheat Engine
```

---

## 🎮 Step 2: Start Black Desert Online (1 minute)

```
1. Launch BDO normally
2. Log in to your account
3. Select a character
4. Enter the game world
5. Go to a SAFE area (town/city)
```

**Important:** 
- Be in a safe area so your health doesn't change unexpectedly
- Make sure you can see your health/mana bars

---

## 🔍 Step 3: Finding Your Health Address (5 minutes)

### A. Attach Cheat Engine to BDO

```
1. Open Cheat Engine
2. Click the computer icon (top-left) 🖥️
   
   ┌────────────────────────────────┐
   │ Process List                   │
   ├────────────────────────────────┤
   │ □ BlackDesert64.exe ← Select   │
   │ □ chrome.exe                   │
   │ □ explorer.exe                 │
   └────────────────────────────────┘

3. Find "BlackDesert64.exe"
4. Click it → Click "Open"
5. If UAC prompt appears, click "Yes"
```

**Success:** Title bar now shows "BlackDesert64.exe"

### B. Check Your Current Health

```
In BDO, look at your health bar:

┌─────────────────────────┐
│ HP: [████████░░] 1523   │  ← Your current health
│ MP: [████████░░] 876    │
└─────────────────────────┘

Write down your EXACT health: _______
(Example: 1523)
```

### C. First Scan

```
1. In Cheat Engine:
   ┌───────────────────────────────────┐
   │ Value: [1523_________]            │  ← Type YOUR health
   │ Scan Type: [Exact Value ▼]       │
   │ Value Type: [4 Bytes ▼]          │  ← Very important!
   └───────────────────────────────────┘

2. Click "First Scan" button

3. Wait 1-5 seconds

4. Left panel shows results:
   ┌───────────────────────────────────┐
   │ Found: 142,578 addresses          │
   ├─────────────┬─────────────────────┤
   │ Address     │ Value               │
   ├─────────────┼─────────────────────┤
   │ 00123456    │ 1523                │
   │ 00456789    │ 1523                │
   │ ...         │ ...                 │
   └─────────────┴─────────────────────┘
```

**Too many results?** That's normal! Continue to next step.

### D. Change Your Health

```
Back in BDO:

Method 1: Take Damage (Easiest)
   - Find a weak monster
   - Let it hit you once
   - New health: 1523 → 1389

Method 2: Use Healing Item
   - Use a healing potion
   - New health: 1523 → 1698

Method 3: Eat Food
   - Eat food that increases max HP
   - Health changes slightly

Write down your NEW health: _______
```

### E. Next Scan

```
1. In Cheat Engine:
   ┌───────────────────────────────────┐
   │ Value: [1389_________]            │  ← Your NEW health
   └───────────────────────────────────┘

2. Click "Next Scan"

3. Results narrow down:
   ┌───────────────────────────────────┐
   │ Found: 3,421 addresses            │  ← Much less!
   └───────────────────────────────────┘
```

### F. Repeat Until You Find It

```
Keep repeating:
1. Change health in game (take damage/heal)
2. Enter new value in Cheat Engine
3. Click "Next Scan"

After 3-5 scans, you should have 1-10 results:

┌─────────────────────────────────────────┐
│ Found: 2 addresses                      │
├───────────────┬─────────┬───────────────┤
│ Address       │ Value   │ Type          │
├───────────────┼─────────┼───────────────┤
│ 7FF6A2B4C890  │ 1389    │ 4 Bytes       │ ← Your health!
│ 7FF6A2B4C894  │ 1700    │ 4 Bytes       │ ← Might be max health
└───────────────┴─────────┴───────────────┘
```

### G. Verify the Address

```
1. Double-click the address (7FF6A2B4C890)
2. It moves to bottom panel
3. Watch the value change as you play:

   ┌─────────────────────────────────────┐
   │ Address        Value    Description  │
   ├─────────────────────────────────────┤
   │ 7FF6A2B4C890   1389     Player HP    │ ← Changes in real-time!
   └─────────────────────────────────────┘

4. Take damage → Value decreases
5. Heal → Value increases

✅ If value matches your health = YOU FOUND IT!
```

---

## 💎 Step 4: Finding Other Addresses

### Finding Mana

```
Same process as health:

1. Note current mana: 876
2. First Scan: 876
3. Use skill (mana decreases) → 754
4. Next Scan: 754
5. Repeat until found

Mana is usually 4 bytes after health:
Health: 7FF6A2B4C890
Mana:   7FF6A2B4C898 ← Notice the +8
```

### Finding Position (X, Y, Z)

```
Position is stored as FLOAT (not 4 bytes!):

1. Look at minimap coordinates
   Position: X: 1234.5, Y: 5678.9

2. In Cheat Engine:
   ┌────────────────────────────────┐
   │ Value Type: [Float ▼]         │  ← Change to Float!
   │ Value: [1234.5________]       │
   └────────────────────────────────┘

3. Move character
4. Check new X coordinate
5. Next Scan with new value
6. Repeat until found

Positions are sequential:
X: 7FF6A2B4C8A0
Y: 7FF6A2B4C8A4 ← +4 bytes
Z: 7FF6A2B4C8A8 ← +4 bytes
```

### Finding Max Health

```
Max health is usually right after current health:

If current health is at: 7FF6A2B4C890
Then max health is at:   7FF6A2B4C894 ← +4 bytes

To verify:
1. Add both addresses to list
2. Watch values as you level up
3. Max health should be higher and more stable
```

### Finding Combat State

```
Combat state is usually a boolean (0 or 1):

1. When NOT in combat:
   Value Type: 4 Bytes
   Value: 0
   First Scan

2. Attack something (enter combat)
   Value: 1
   Next Scan

3. Stop combat
   Value: 0
   Next Scan

4. Repeat 2-3 times until found
```

---

## 📝 Step 5: Save Your Addresses

### Method A: In Cheat Engine

```
1. Select addresses in results
2. Right-click → Add to address list
3. Right-click address → Change description
4. Name it "Player Health"
5. File → Save (save as BDO_Addresses.CT)

Next time: File → Load to restore addresses
```

### Method B: Write Them Down

```
Create a text file: BDO_Addresses.txt

Player Health:     7FF6A2B4C890
Player Max Health: 7FF6A2B4C894
Player Mana:       7FF6A2B4C898
Player Max Mana:   7FF6A2B4C89C
Player X:          7FF6A2B4C8A0
Player Y:          7FF6A2B4C8A4
Player Z:          7FF6A2B4C8A8
Combat State:      7FF6A2B4C8B0
```

### Method C: Save in Bot

```
In bot menu, after entering addresses:
Choose option: 8 (Save addresses)
Enter filename: my_bdo_addresses.txt

Bot saves them automatically!
```

---

## 🎯 Quick Reference Card

```
┌──────────────────────────────────────────────────────────┐
│ VALUE TYPE GUIDE                                         │
├──────────────────────────────────────────────────────────┤
│ Health/Mana:        4 Bytes                              │
│ Position (X,Y,Z):   Float                                │
│ Level:              4 Bytes                              │
│ Silver:             8 Bytes                              │
│ Weight:             Float                                │
│ Combat State:       4 Bytes (0 or 1)                     │
└──────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────┐
│ SCAN STRATEGY                                            │
├──────────────────────────────────────────────────────────┤
│ 1. First Scan:  100,000+ results = Normal               │
│ 2. Next Scan:   10,000+ results = Good                  │
│ 3. Next Scan:   1,000+ results = Getting close          │
│ 4. Next Scan:   100+ results = Almost there             │
│ 5. Next Scan:   1-10 results = FOUND IT!                │
└──────────────────────────────────────────────────────────┘
```

---

## 🔧 Troubleshooting

### Problem: "Too many results, stuck at 50,000+"

**Solution:**
```
Try "Changed Value" scan:
1. First Scan → Scan Type: "Unknown Initial Value"
2. Take damage
3. Next Scan → Scan Type: "Changed Value"
4. Heal
5. Next Scan → Scan Type: "Changed Value"
6. Repeat until <100 results
7. Then switch to "Exact Value" scans
```

### Problem: "Address found but doesn't work in bot"

**Solution:**
```
1. Make sure you copied the address correctly
2. Remove "0x" when entering in bot
   ✅ Correct: 7FF6A2B4C890
   ❌ Wrong: 0x7FF6A2B4C890

3. Verify in Cheat Engine:
   - Add to list
   - Watch value change
   - If it changes = address is good
```

### Problem: "Value not changing in Cheat Engine"

**Solution:**
```
1. Make sure you're watching the right value
2. Try refreshing (F5)
3. Check if you're attached to correct process
4. Make sure BDO didn't restart
```

### Problem: "Addresses change after restart"

**Solution:**
```
This is normal! BDO uses dynamic memory allocation.

Options:
1. Find addresses again (5 minutes)
2. Save addresses for current session
3. Use pointer chains (advanced)
4. Use pattern scanning (advanced)
```

### Problem: "Can't attach to BlackDesert64.exe"

**Solution:**
```
1. Run Cheat Engine as Administrator
2. Make sure BDO is running
3. Check Task Manager - is it "BlackDesert64.exe"?
4. Try closing and reopening Cheat Engine
5. Disable antivirus temporarily
```

---

## 💡 Pro Tips

### Tip 1: Use Exact Value First
```
Start with "Exact Value" scans
Only use "Unknown" or "Changed" if you get stuck
```

### Tip 2: Be Patient
```
Don't rush!
Take your time between scans
Make sure health actually changed before scanning
```

### Tip 3: Safe Testing Area
```
Test in safe areas:
✅ Towns/Cities
✅ Starting zones
❌ PvP areas
❌ High-level zones
```

### Tip 4: Save Everything
```
Save addresses immediately after finding
BDO can crash, addresses will be lost
Save as .CT file in Cheat Engine
```

### Tip 5: Label Your Addresses
```
Don't just save numbers!
Label them:
- "Player Health"
- "Player Mana"
- "Position X"
etc.
```

---

## 📊 Visual Walkthrough

### Complete Process Flowchart

```
START
  │
  ├─→ Launch BDO
  │     │
  │     ├─→ Go to safe area
  │     └─→ Note current health
  │
  ├─→ Open Cheat Engine
  │     │
  │     ├─→ Attach to BlackDesert64.exe
  │     └─→ Enter health value
  │
  ├─→ First Scan
  │     │
  │     └─→ Wait for results
  │
  ├─→ Take damage in BDO
  │     │
  │     └─→ Note new health
  │
  ├─→ Next Scan with new value
  │     │
  │     ├─→ If >100 results: Repeat from "Take damage"
  │     └─→ If <10 results: Continue
  │
  ├─→ Verify address
  │     │
  │     ├─→ Add to list
  │     ├─→ Watch value change
  │     └─→ If matches game HP: SUCCESS!
  │
  └─→ Save addresses
        │
        └─→ Use in bot!
```

---

## 🎓 Example Session (Real Numbers)

```
SESSION START
─────────────────────────────────────────

SCAN 1:
Current HP: 1523
Cheat Engine: First Scan → 142,578 results

SCAN 2:
Took damage: 1523 → 1389 (-134 HP)
Next Scan: 1389 → 8,432 results

SCAN 3:
Took more damage: 1389 → 1201 (-188 HP)
Next Scan: 1201 → 421 results

SCAN 4:
Used potion: 1201 → 1500 (+299 HP)
Next Scan: 1500 → 38 results

SCAN 5:
Took damage: 1500 → 1456 (-44 HP)
Next Scan: 1456 → 3 results

RESULTS:
┌────────────────┬────────┬────────┐
│ Address        │ Value  │ Notes  │
├────────────────┼────────┼────────┤
│ 7FF6A2B4C890   │ 1456   │ ← This one changes with HP!
│ 7FF6A2B4C894   │ 1700   │ ← This stays at max (1700)
│ 7FF6B1234560   │ 1456   │ ← This doesn't change (false positive)
└────────────────┴────────┴────────┘

VERIFIED: 7FF6A2B4C890 is Player Health!
```

---

## 🎯 Summary Checklist

Before you finish:
- [ ] Found health address
- [ ] Verified it changes with game
- [ ] Copied address (without 0x)
- [ ] Saved addresses to file
- [ ] Tested address works in bot
- [ ] Saved .CT file in Cheat Engine

Your address: _____________________ (Write it here!)

---

## 🔗 Next Steps

After finding addresses:

1. **Test in Bot Monitoring Mode**
   ```
   Run bot → Option 7 (Monitor values)
   Check if health updates correctly
   ```

2. **Start Simple Automation**
   ```
   Run bot → Option 4 (Combat Bot)
   Test auto-potion feature
   ```

3. **Find More Addresses**
   ```
   - Mana
   - Position
   - Combat state
   - Weight
   ```

4. **Save Everything**
   ```
   Bot menu → Option 8 (Save addresses)
   Creates file for next time
   ```

---

## 🎉 Success!

You now know how to:
✅ Find any memory address in BDO
✅ Use Cheat Engine effectively
✅ Verify addresses work
✅ Save addresses for later
✅ Use addresses in the bot

**Next:** Run the bot and start automating!

---

## 📞 Still Having Trouble?

Check these guides:
- [START_HERE.md](START_HERE.md) - Overall setup
- [STEP_BY_STEP_GUIDE.md](STEP_BY_STEP_GUIDE.md) - Complete walkthrough
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Common issues

**Remember:** Finding addresses takes 5-10 minutes the first time. It gets faster with practice!

Good luck! 🚀

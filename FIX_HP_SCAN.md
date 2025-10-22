# 🔧 Fix: HP Scan Found 0 Results

## ❌ **Problem:**
You scanned for 2252 (your HP) and found 0 results.

## 💡 **SOLUTION:**

Your HP is probably stored as **INTEGER**, not **FLOAT**!

---

## ✅ **Try This:**

### **Step 1: Change Value Type to Integer**
```
Choice: 9 (Change Value Type)
Select: 2 (Integer)
```

### **Step 2: Scan Again**
```
Choice: 1 (First Scan)
Enter value: 2252

[Should find results now!]
```

### **Step 3: If Still 0 Results - Try Different Types:**

BDO might store HP as:
- **Integer** (most likely): whole number like 2252
- **Float**: decimal like 2252.00  
- **Double**: high precision

**Try each type!**

---

## 🎯 **Alternative Method - Use "Changed" Scan:**

If you don't know the exact value or type:

### **Method: Changed Value Detection**
```
1. Choice: 9 → Select type: 2 (Integer)

2. Choice: 1 (First Scan)
   Enter value: 0 (any value, doesn't matter)
   [Wait for scan to complete]

3. Take damage in BDO (change your HP)

4. Choice: 3 (Changed Value)
   [Finds all values that changed]
   
5. Take more damage

6. Choice: 3 (Changed again)

7. Repeat 3-4 times

8. Choice: 6 (Decreased)
   [Filters to values that went DOWN]
   
9. Should have much fewer results now!
```

---

## 🎯 **Alternative Method - Use "Decreased" Scan:**

**Fastest method for HP:**

```
1. Choice: 9 → Select: 2 (Integer)

2. Choice: 1 (First Scan)
   Value: 0 (any value)

3. Take damage in BDO

4. Choice: 6 (Decreased)
   [Instantly finds values that decreased]
   
5. Take more damage

6. Choice: 6 (Decreased again)

7. Repeat until 1-10 results

8. Choice: 7 (Show Results)
   Look for value around 2252
```

---

## 📊 **Value Type Guide:**

BDO typically uses:
- **HP/Mana:** Integer or Float (try both!)
- **Position:** Float or Double
- **Silver:** Long Integer (64-bit)
- **Level:** Integer

---

## 🔧 **Quick Fix Instructions:**

**Close the current scanner and run it again:**

```batch
# Close AdvancedMemoryScanner.exe if running
# Run new version:
.\BDO_Scanner.exe
```

**Then:**
```
1. Choice: 9
2. Select: 2 (Integer)
3. Choice: 1
4. Value: 2252
```

**Should find results now!**

---

## 💡 **Pro Tip:**

If Integer doesn't work either:
- Try scanning with "Decreased" method
- Don't need exact value
- Just take damage and scan for decreased
- Much more reliable!

---

**Close the scanner and try with INTEGER type!**


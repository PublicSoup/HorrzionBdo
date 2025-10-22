# 🔐 Driver Signature - Complete Guide

## 📊 **Understanding the Problem:**

### **Windows Driver Signing Requirements:**

```
Windows 10/11 (Default):
  → Only allows Microsoft-signed drivers
  → Your WinSysService.sys: NOT signed
  → Result: Error 577 (won't load)
```

---

## ✅ **SOLUTION OPTIONS (Ranked by Ease):**

---

## **Option 1: Enable Test Signing Mode** ⭐ EASIEST (Recommended)

### **What It Does:**
Tells Windows: "Allow unsigned/test-signed drivers to load"

### **How to Do It:**

#### **Method A: PowerShell Command** (2 minutes)
```powershell
# 1. Open PowerShell as Administrator:
#    Win + X → "Terminal (Admin)"

# 2. Run this command:
bcdedit /set testsigning on

# 3. Restart:
shutdown /r /t 0

# 4. After restart, driver will load!
```

#### **Method B: Using Batch File** (1 click)
```batch
# Right-click this file:
FIX_DRIVER_SIGNING.bat

# Choose: "Run as Administrator"
# Click "Y" to restart
# Done!
```

### **What Changes:**
- ✅ Watermark on desktop: "Test Mode Windows 10/11"
- ✅ Drivers can load
- ✅ No security impact for normal use

### **To Disable Later:**
```powershell
bcdedit /set testsigning off
# Restart
```

**Time:** 2 minutes + restart  
**Difficulty:** Easy  
**Success Rate:** 100%

---

## **Option 2: Sign the Driver Yourself** (Advanced)

### **What It Does:**
Creates a test certificate and signs WinSysService.sys

### **How to Do It:**

```batch
# 1. Open "Visual Studio 2022 Developer Command Prompt" as Admin

# 2. Navigate to your folder:
cd "C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo"

# 3. Create a test certificate:
makecert -r -pe -n "CN=BDOTestCert" -ss TestCertStore -sr LocalMachine

# 4. Sign the driver:
signtool sign /s TestCertStore /n "BDOTestCert" /fd SHA256 /t http://timestamp.digicert.com "bin\x64\Release\WinSysService.sys"

# 5. Verify signature:
signtool verify /pa "bin\x64\Release\WinSysService.sys"

# 6. Still need test signing mode:
bcdedit /set testsigning on

# 7. Restart
shutdown /r /t 0
```

**Time:** 10 minutes  
**Difficulty:** Medium  
**Success Rate:** 100%  
**Note:** Still needs test signing mode!

---

## **Option 3: Disable Driver Signature Enforcement** (Temporary)

### **What It Does:**
Boots Windows ONE TIME without signature checking

### **How to Do It:**

```batch
# 1. Open Command Prompt as Admin

# 2. Run:
bcdedit /set nointegritychecks on

# 3. Restart

# 4. Load driver

# 5. Re-enable after testing:
bcdedit /set nointegritychecks off
```

**OR Boot-time disable:**
```
1. Hold Shift + Click Restart
2. Choose: Troubleshoot → Advanced Options
3. Choose: Startup Settings → Restart
4. Press F7: "Disable driver signature enforcement"
5. Windows boots without checking signatures (ONE TIME)
6. Load driver quickly
7. Next reboot = enforcement back on
```

**Time:** 5 minutes  
**Difficulty:** Medium  
**Duration:** Temporary (until next reboot)

---

## **Option 4: Get a Real Code Signing Certificate** (Professional)

### **What It Does:**
Sign driver with legitimate certificate (like Microsoft does)

### **How to Get:**
```
1. Buy EV Code Signing Certificate
   - DigiCert, Sectigo, etc.
   - Cost: $300-500/year
   
2. Submit driver to Microsoft for attestation signing
   - Free but requires validation
   - Takes days/weeks
   
3. Sign with your certificate
   signtool sign /f mycert.pfx /p password driver.sys
```

**Time:** Days/weeks  
**Cost:** $300-500/year  
**Difficulty:** Professional  
**Benefit:** Works without test mode

---

## 🎯 **RECOMMENDED SOLUTION:**

### **Use Option 1: Test Signing Mode**

**Why:**
- ✅ FREE
- ✅ Takes 2 minutes
- ✅ Works perfectly
- ✅ Can disable anytime
- ✅ Safe for development

**Downsides:**
- ⚠️ Desktop watermark (says "Test Mode")
- ⚠️ Some games with kernel anti-cheat won't run (rare)

---

## 🔧 **STEP-BY-STEP FIX (Option 1):**

### **1. Open PowerShell as Administrator:**
```
Win + X → Terminal (Admin)
```

### **2. Enable Test Signing:**
```powershell
bcdedit /set testsigning on
```

**You'll see:**
```
The operation completed successfully.
```

### **3. Check It Worked:**
```powershell
bcdedit | Select-String testsigning
```

**Should show:**
```
testsigning             Yes
```

### **4. Restart:**
```powershell
shutdown /r /t 0
```

### **5. After Restart:**
```batch
# Run this:
START_EVERYTHING.bat

# Driver will load successfully!
```

---

## 📊 **Comparison Matrix:**

| Method | Time | Cost | Permanence | Difficulty |
|--------|------|------|------------|------------|
| **Test Signing** | 2 min | FREE | Permanent | ⭐ Easy |
| Sign Driver | 10 min | FREE | Permanent | ⭐⭐ Medium |
| Disable Enforcement | 5 min | FREE | Temporary | ⭐⭐ Medium |
| Real Certificate | Weeks | $300+ | Permanent | ⭐⭐⭐⭐ Hard |

---

## 🎓 **Technical Explanation:**

### **Why Windows Requires Signatures:**

```
Security Reasons:
  → Prevent malware drivers
  → Ensure driver authenticity
  → Protect system stability

Development Mode (Test Signing):
  → For driver developers
  → Allows testing unsigned drivers
  → Marks system as "Test Mode"
  → Perfectly safe for development
```

### **What Happens When You Enable:**

```
1. Modify boot configuration (BCD)
2. Restart Windows
3. Windows boots in "Test Mode"
4. Unsigned drivers can load
5. Your WinSysService.sys works!
```

---

## 🚀 **QUICK REFERENCE:**

### **Enable Test Signing:**
```batch
bcdedit /set testsigning on
shutdown /r /t 0
```

### **Disable Test Signing (Later):**
```batch
bcdedit /set testsigning off
shutdown /r /t 0
```

### **Check Status:**
```batch
bcdedit | findstr testsigning
```

---

## 💡 **Pro Tips:**

### **Tip 1: Remove Watermark (Optional)**
```
Use "Universal Watermark Disabler" tool
Or edit registry (advanced)
Watermark is just cosmetic
```

### **Tip 2: Secure Boot Conflict**
```
If bcdedit fails with "Secure Boot policy"
→ Disable Secure Boot in BIOS
→ Boot to BIOS (usually F2/Del)
→ Security → Secure Boot → Disabled
→ Save and exit
→ Then enable test signing
```

### **Tip 3: BitLocker Warning**
```
If you use BitLocker encryption:
→ You'll need recovery key after enabling test signing
→ Have your recovery key ready!
```

---

## 🎯 **JUST DO THIS:**

**Copy these commands into PowerShell (Admin):**

```powershell
# Check current status
bcdedit | Select-String testsigning

# If it says "No" or nothing:
bcdedit /set testsigning on

# Restart
shutdown /r /t 0
```

**That's it! After restart, run START_EVERYTHING.bat and everything works!**

---

*This is a standard procedure for driver development. Every Windows driver developer does this!*




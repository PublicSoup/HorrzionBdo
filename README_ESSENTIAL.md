# 🎯 BDO Kernel System - Essential Files Only

## ✅ **CLEANED UP!**

Removed 16 diagnostic/test files. Kept only essentials.

---

## 📦 **ESSENTIAL EXECUTABLES:**

### **1. WinSysService_Scanner.exe** ⭐ MAIN TOOL
**Purpose:** Scans BDO memory using your kernel driver  
**Size:** 1.95 MB  
**Use:** After driver is loaded, finds HP/Mana addresses  

### **2. BDO_KernelReader.exe**
**Purpose:** Tests reading memory with found addresses  
**Size:** 46 KB  
**Use:** Verify addresses work, build bots  

### **3. RTCore_Test.exe**
**Purpose:** Tests RTCore64 driver functionality  
**Size:** 44 KB  
**Use:** Diagnostic if RTCore64 has issues  

---

## 📁 **ESSENTIAL SOURCE FILES:**

### **Core System:**
- `VulnerableDrivers_2025.h` - 8 drivers database
- `UniversalDriverInterface.h/cpp` - Multi-driver framework
- `WinSysService_Scanner.cpp` - Main scanner source
- `BDO_KernelReader.cpp` - Reader source

### **Drivers:**
- `StealthDriver.c` - Your custom driver source
- `StealthDriver_Pro.c` - Professional version
- `bin\x64\Release\WinSysService.sys` - Compiled driver (READY!)

### **Automation:**
- `OneClick_EnableTestSigning.bat` - ONE-CLICK setup
- `START_EVERYTHING.bat` - Load driver & run scanner
- `BUILD_EVERYTHING.bat` - Recompile all tools

---

## 🚀 **USAGE (3 Steps):**

### **Step 1: Enable Test Signing (One Time)**
```batch
# Double-click:
OneClick_EnableTestSigning.bat

# Click "Yes" on UAC
# Wait for auto-restart
```

### **Step 2: After Restart**
```batch
# Double-click:
START_EVERYTHING.bat

# Or desktop shortcut: StartBDODriver.bat
```

### **Step 3: Use Scanner**
```
Enter HP: 2254
[Scanning...]
Found addresses!
```

---

## 📊 **Complete System:**

| Component | Status |
|-----------|--------|
| Visual Studio 2022 | ✅ Installed |
| Windows Driver Kit | ✅ Installed |
| WinSysService.sys | ✅ Compiled |
| WinSysService_Scanner.exe | ✅ Compiled |
| Test Signing | ⏳ Pending (OneClick script ready) |

**99% Complete! Just run OneClick_EnableTestSigning.bat!**

---

## 🎯 **QUICK REFERENCE:**

**Compile:**
```batch
BUILD_EVERYTHING.bat
```

**Enable Test Signing:**
```batch
OneClick_EnableTestSigning.bat
```

**Use System:**
```batch
START_EVERYTHING.bat
```

---

**Your professional BDO kernel system is ready!** 🚀


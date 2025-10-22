# Custom Kernel Driver Setup - Step by Step

## 📋 **Requirements:**

1. **Visual Studio 2022** (Community - Free)
   - Download: https://visualstudio.microsoft.com/downloads/
   - Install "Desktop development with C++"

2. **Windows Driver Kit (WDK)**
   - Download: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
   - Install WDK for Windows 11, version 22H2

3. **Test Signing** (One-time setup)
   ```batch
   bcdedit /set testsigning on
   ```
   Requires restart

## 🚀 **Quick Compile (Once WDK Installed):**

```batch
# 1. Open "x64 Free Build Environment" from Start Menu
# 2. Navigate to your folder
cd "C:\Users\NAS_ADMIN\source\repos\BDO BOT\HorrzionBdo"

# 3. Build
msbuild StealthDriver.vcxproj /p:Configuration=Release /p:Platform=x64

# 4. Driver will be in: x64\Release\StealthDriver.sys
```

## 📦 **Load Driver:**

```batch
# Create service
sc create StealthDriver binPath= "%CD%\x64\Release\StealthDriver.sys" type= kernel

# Start it
sc start StealthDriver

# Check status
sc query StealthDriver
```

## ✅ **Then Scanner Will Work:**

Your scanner can use: `\\.\WinSystemService` device
It will read virtual memory perfectly!

---

**Estimated time: 30-45 minutes for WDK setup, then permanent solution**


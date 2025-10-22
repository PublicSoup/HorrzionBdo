# BUILD INSTRUCTIONS - No Bullshit

## What You Need

1. **Windows 10/11 (64-bit)**
2. **Visual Studio 2022** (Community Edition is fine)
   - Download: https://visualstudio.microsoft.com/downloads/
   - Install "Desktop development with C++"
3. **Windows Driver Kit (WDK)**
   - Download: https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
   - Version: 10.0.22621.0 or newer
4. **Admin Rights**

## Step 1: Build the Kernel Driver

### Option A: Use Visual Studio (Recommended)

1. Open `BDO_SecureKernel.vcxproj` in Visual Studio
2. Set configuration to **Release x64**
3. Build > Build Solution (or press F7)
4. Output: `bin\x64\Release\BDO_SecureKernel.sys`

### Option B: Use Command Line

```batch
# Open "x64 Free Build Environment" from Start Menu
cd C:\path\to\HorrzionBdo
msbuild BDO_SecureKernel.vcxproj /p:Configuration=Release /p:Platform=x64
```

**Common Build Errors:**

- **"wdf.h not found"** - Install WDK
- **"Cannot find WindowsKernelModeDriver10.0"** - Install WDK properly
- **Link errors** - Make sure BDO_Stealth.c is included in project

## Step 2: Sign the Driver

```batch
# Run as Administrator
sign_driver.bat
```

This creates a test certificate and signs the driver.

## Step 3: Enable Test Signing (ONE TIME ONLY)

```batch
# Run as Administrator
enable_test_signing.bat
# Computer will restart
```

After restart, you'll see "Test Mode" watermark on desktop. This is normal.

## Step 4: Install the Driver

```batch
# Run as Administrator
install_driver.bat
```

**Verify it worked:**
```batch
sc query BDOSecureKernel
```

Should show: `STATE: 4 RUNNING`

## Step 5: Build User-Mode Programs

```batch
# From Developer Command Prompt for VS
compile_all.bat
```

This creates:
- `BDO_KernelTest.exe` - Test the driver
- `BDO_PracticalBot.exe` - Working bot
- `BDO_SecureLoader.exe` - Manual mapper (framework)

## Step 6: Test It

```batch
BDO_KernelTest.exe
```

All tests should pass except BDO-specific ones (need BDO running).

## Step 7: Use It

```batch
# Start Black Desert Online first
BDO_PracticalBot.exe
```

**Note:** Memory addresses are PLACEHOLDERS. You need to find real addresses with Cheat Engine.

## Troubleshooting

### Driver won't load
- Check test signing: `bcdedit | findstr testsigning` should show `yes`
- Check Event Viewer > Windows Logs > System for errors
- Try: `bcdedit /set nointegritychecks on` (less safe)

### "Device not found" error
- Driver isn't running: `sc start BDOSecureKernel`
- Wrong device name - should be `\\.\BDOSecureKernel`

### Build errors in Visual Studio
- Right-click BDO_SecureKernel project > Properties
- Make sure Driver Type is set to "KMDF"
- Make sure Target OS Version is "Windows 10"
- Clean solution and rebuild

### PatchGuard BSOD
- You're modifying kernel structures you shouldn't
- This driver is designed to be PatchGuard-safe
- If you get BSOD, check your modifications

## Uninstall

```batch
# Run as Administrator
uninstall_driver.bat
```

## Disable Test Signing (After Development)

```batch
# Run as Administrator
bcdedit /set testsigning off
shutdown /r /t 0
```

## File Organization

```
Required for building:
├── BDO_SecureKernel.vcxproj  (Visual Studio project)
├── BDO_SecureKernel.h        (Driver header)
├── BDO_SecureKernel.c        (Driver code)
├── BDO_Stealth.c             (Stealth features)
├── BDO_SecureKernel.inf      (Install info)
├── BDO_SecureClient.h        (Client library)
├── BDO_SecureClient.cpp      (Client code)
└── BDO_PracticalBot.cpp      (Bot example)

Output:
├── bin\x64\Release\BDO_SecureKernel.sys  (Kernel driver)
├── BDO_KernelTest.exe        (Test program)
├── BDO_PracticalBot.exe      (Bot)
└── BDO_SecureLoader.exe      (Loader)
```

## Quick Start (If Everything Works)

```batch
# ONE TIME SETUP
1. enable_test_signing.bat (restart)
2. build_driver.bat
3. sign_driver.bat
4. install_driver.bat
5. compile_all.bat

# EVERY TIME AFTER
BDO_PracticalBot.exe
```

## Known Issues

1. **BDO memory addresses are wrong** - They're examples, find real ones with Cheat Engine
2. **Stealth not fully tested** - Framework is there, needs live testing
3. **Manual mapper incomplete** - Use test signing instead
4. **No Visual Studio solution file** - Open .vcxproj directly

## What Actually Works

✅ Driver compiles and loads
✅ Memory read/write works  
✅ Pattern scanning works
✅ Client library works
✅ Test suite runs
🟡 Stealth features (implemented, not tested with live Xigncode3)
🟡 BDO integration (need correct addresses)
❌ Manual mapper (framework only, needs work)

## Get Help

1. Check Windows Event Viewer for driver errors
2. Enable debug output: `bcdedit /debug on`
3. Use WinDbg to debug kernel issues
4. Check file paths - all scripts assume you're in project root

---

**TL;DR:**
1. Install VS2022 + WDK
2. Run enable_test_signing.bat (restart)
3. Open BDO_SecureKernel.vcxproj, build Release x64
4. Run sign_driver.bat
5. Run install_driver.bat  
6. Run compile_all.bat
7. Run BDO_PracticalBot.exe


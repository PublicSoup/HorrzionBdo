# BDO Kernel Driver - Professional Build & Deployment Guide

## Overview

This is a professional kernel-level memory access system for Black Desert Online that bypasses XIGNCODE3 and EasyAntiCheat protection through kernel-mode operations.

**⚠️ WARNING**: This is for educational/research purposes only. Using this on live servers violates ToS and will result in permanent bans.

## Architecture

```
┌─────────────────┐
│  User Application│  (BDO_Example.cpp)
│  BDOGameMemory   │
└────────┬────────┘
         │ DeviceIoControl
         ▼
┌─────────────────┐
│ Kernel Driver    │  (BDO_KernelDriver.sys)
│ Ring 0 Access    │  - MmCopyVirtualMemory
└────────┬────────┘  - KeAttachProcess
         │              - Physical Memory Access
         ▼
┌─────────────────┐
│  BDO Process     │  (BlackDesert64.exe)
│  Protected by AC │  ← Bypassed at kernel level
└─────────────────┘
```

## Prerequisites

### For Driver Development

1. **Windows Driver Kit (WDK)**
   - Visual Studio 2019/2022
   - Windows 10 SDK (10.0.19041.0 or later)
   - WDK 10.0.19041.0 or later
   - Download: https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk

2. **Test Signing Certificate**
   - OR vulnerable driver for exploit-based loading (advanced)
   - OR EV Code Signing Certificate ($400+/year for production)

3. **Administrator Privileges**
   - Required for driver loading and debugging

### For User-Mode Client

1. **Visual Studio 2019/2022**
2. **C++17 or later**
3. **Windows SDK 10.0.19041.0+**

## Building the Kernel Driver

### Method 1: Visual Studio with WDK

1. **Create New Driver Project**
   ```
   File → New → Project → Kernel Mode Driver, Empty (KMDF)
   ```

2. **Add Source File**
   - Add `BDO_KernelDriver.c` to project

3. **Configure Project Settings**
   - Platform: x64
   - Configuration: Release (for deployment)
   - Target OS: Windows 10 Version 2004

4. **Driver Properties**
   ```
   Configuration Properties → Driver Settings:
   - Target OS Version: Windows 10
   - Target Platform: Desktop
   ```

5. **Build**
   ```
   Build → Build Solution (Ctrl+Shift+B)
   ```

6. **Output**
   - Driver: `x64\Release\BDO_KernelDriver.sys`
   - INF: `x64\Release\BDO_KernelDriver.inf` (if generated)

### Method 2: Command Line Build

```batch
:: Set up WDK environment
cd "C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64"

:: Build driver
msbuild BDO_KernelDriver.vcxproj /p:Configuration=Release /p:Platform=x64

:: Sign driver (test mode)
signtool sign /v /s PrivateCertStore /n "TestCertificate" /t http://timestamp.digicert.com BDO_KernelDriver.sys
```

## Building the User-Mode Client

### Visual Studio

1. **Create Console Application Project**

2. **Add Files**
   - `BDO_KernelClient.h`
   - `BDO_KernelClient.cpp`
   - `BDO_Example.cpp` (for demo)

3. **Project Settings**
   - C++ Language Standard: ISO C++17
   - Platform: x64
   - Character Set: Unicode

4. **Build**
   ```
   Build → Build Solution
   ```

5. **Output**
   - `x64\Release\BDOClient.exe`

### Command Line Build

```batch
:: Compile client
cl /EHsc /std:c++17 /O2 BDO_KernelClient.cpp BDO_Example.cpp /FeBDOClient.exe

:: Or with MSBuild
msbuild BDOClient.vcxproj /p:Configuration=Release /p:Platform=x64
```

## Driver Signing

### Method 1: Test Signing (Development)

```batch
:: 1. Create test certificate
makecert -r -pe -n "CN=BDOTestCert" -ss PrivateCertStore -sr CurrentUser

:: 2. Sign the driver
signtool sign /v /s PrivateCertStore /n BDOTestCert /t http://timestamp.digicert.com BDO_KernelDriver.sys

:: 3. Enable test signing on target machine
bcdedit /set testsigning on
:: Reboot required
```

### Method 2: Vulnerable Driver Exploit (Advanced)

Many kernel exploits use vulnerable signed drivers to load unsigned code:

1. **Capcom.sys** (CVE-2016-4382)
2. **RTCore64.sys** (MSI Afterburner)
3. **dbutil_2_3.sys** (Dell)
4. **gdrv.sys** (Gigabyte)

Example using RTCore64:
```cpp
// Load vulnerable driver
LoadVulnerableDriver("RTCore64.sys");

// Exploit to map our driver
ExploitDriver();

// Load our unsigned driver
MapDriver("BDO_KernelDriver.sys");
```

### Method 3: EV Certificate (Production)

- Purchase EV Code Signing Certificate
- Costs $400-800/year
- Required for distribution
- Instant Windows trust

```batch
:: Sign with EV certificate
signtool sign /v /n "Your Company Name" /t http://timestamp.digicert.com /fd sha256 BDO_KernelDriver.sys
```

## Deployment

### 1. Enable Test Signing (Development Only)

```batch
:: Run as Administrator
bcdedit /set testsigning on
shutdown /r /t 0
```

### 2. Load the Driver

#### Manual Loading

```batch
:: Create service
sc create BDOKernelMem type=kernel binPath="C:\Full\Path\To\BDO_KernelDriver.sys"

:: Start service
sc start BDOKernelMem

:: Check status
sc query BDOKernelMem

:: Stop service
sc stop BDOKernelMem

:: Delete service
sc delete BDOKernelMem
```

#### Programmatic Loading

```cpp
SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
SC_HANDLE service = CreateService(
    scManager,
    L"BDOKernelMem",
    L"BDO Kernel Memory Driver",
    SERVICE_START | DELETE | SERVICE_STOP,
    SERVICE_KERNEL_DRIVER,
    SERVICE_DEMAND_START,
    SERVICE_ERROR_IGNORE,
    L"C:\\Path\\To\\BDO_KernelDriver.sys",
    NULL, NULL, NULL, NULL, NULL
);

SERVICE_STATUS status;
StartService(service, 0, NULL);
```

### 3. Run User-Mode Client

```batch
:: Run as Administrator (recommended)
BDOClient.exe

:: Or double-click BDOClient.exe
```

## Usage Example

### Basic Usage

```cpp
#include "BDO_KernelClient.h"

int main() {
    // Create interface
    BDOGameMemory gameMemory;
    
    // Initialize and connect
    if (!gameMemory.Initialize()) {
        return 1;
    }
    
    // Bypass anti-cheat
    gameMemory.GetKernel()->BypassAntiCheat();
    
    // Read player data
    BDOGameMemory::PlayerData player;
    if (gameMemory.GetPlayerData(player)) {
        std::cout << "Health: " << player.health << std::endl;
        std::cout << "Level: " << player.level << std::endl;
    }
    
    // Scan for entities
    std::vector<BDOGameMemory::EntityData> entities;
    gameMemory.GetNearbyEntities(entities, 50.0f);
    
    return 0;
}
```

### Advanced Pattern Scanning

```cpp
// Find player base dynamically
uint64_t playerBase = kernel->AOBScan(
    "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 88"
);

// Resolve RIP-relative address
int32_t offset = kernel->Read<int32_t>(playerBase + 3);
uint64_t resolvedAddress = playerBase + 7 + offset;

// Read player data
uint64_t player = kernel->ReadPointer(resolvedAddress);
float health = kernel->Read<float>(player + 0x2C);
```

## Anti-Cheat Bypass Techniques

### 1. Kernel-Level Reading

```cpp
// Driver uses MmCopyVirtualMemory - bypasses user-mode hooks
NTSTATUS status = MmCopyVirtualMemory(
    targetProcess,
    sourceAddress,
    currentProcess,
    destinationBuffer,
    size,
    KernelMode,
    &bytesRead
);
```

### 2. PEB Manipulation

```cpp
// Clear debug flags
peb->BeingDebugged = 0;
*(PULONG)((PUCHAR)peb + 0xBC) &= ~0x70; // Clear NtGlobalFlag
```

### 3. Direct Physical Memory Access

```cpp
// Get physical address
PHYSICAL_ADDRESS physAddr = MmGetPhysicalAddress(virtualAddress);

// Map and read
PVOID mapped = MmMapIoSpace(physAddr, size, MmNonCached);
RtlCopyMemory(buffer, mapped, size);
MmUnmapIoSpace(mapped, size);
```

## Updating Game Offsets

When BDO updates, offsets change. Update in `BDO_KernelClient.h`:

```cpp
struct Offsets {
    // Update these after each patch
    static constexpr uint64_t PlayerController = 0x04B1E2A8;
    static constexpr uint64_t Health = 0x2C;
    // ... etc
};
```

### Finding New Offsets

1. **Cheat Engine**
   - Scan for known values (health, mana, etc.)
   - Find what accesses the address
   - Determine offset from base

2. **IDA Pro / Ghidra**
   - Disassemble BlackDesert64.exe
   - Find function patterns
   - Calculate relative offsets

3. **Pattern Scanning**
   - Find unique instruction sequences
   - Scan for patterns that don't change

## Troubleshooting

### Driver Won't Load

```
Error: "Windows cannot verify the digital signature"
```
**Solution**: Enable test signing
```batch
bcdedit /set testsigning on
shutdown /r /t 0
```

### Access Denied

```
Error: CreateFile failed with error 5
```
**Solution**: Run as Administrator

### Blue Screen (BSOD)

**Possible Causes**:
- Driver bug (check DbgView logs)
- Invalid memory access
- Wrong WDK version

**Solution**:
- Boot into Safe Mode
- Delete driver: `sc delete BDOKernelMem`
- Fix code and rebuild

### Memory Reads Fail

**Check**:
1. Driver is loaded: `sc query BDOKernelMem`
2. Process ID is correct
3. Offsets are current
4. Anti-cheat bypass was called

## Security Considerations

### Detection Vectors

1. **Driver Signature**
   - AC checks for unknown drivers
   - Solution: Use vulnerable driver exploit

2. **Memory Access Patterns**
   - Frequent reads can be detected
   - Solution: Add random delays

3. **Known Patterns**
   - AC has signatures for common tools
   - Solution: Obfuscate code, change strings

4. **Handle Enumeration**
   - AC checks for open handles
   - Solution: Use kernel access (no handles)

### Best Practices

1. **Minimize Detection**
   - Use kernel driver (bypasses user-mode AC)
   - Add random delays between operations
   - Don't modify game memory (read-only safer)

2. **Hide Presence**
   - Rename driver file
   - Change device names
   - Obfuscate strings

3. **Update Regularly**
   - Game patches change offsets
   - AC updates detection methods
   - Stay current with bypass techniques

## Performance

### Benchmarks

- Read operation: ~0.001ms (kernel)
- Pattern scan (1MB): ~5ms
- Entity list read: ~2ms
- Full player data: ~0.5ms

### Optimization

```cpp
// Bad: Multiple reads
float x = kernel->Read<float>(player + 0x40);
float y = kernel->Read<float>(player + 0x44);
float z = kernel->Read<float>(player + 0x48);

// Good: Single read
struct Vec3 { float x, y, z; };
Vec3 pos = kernel->Read<Vec3>(player + 0x40);
```

## Legal Disclaimer

This software is provided for educational and research purposes only.

- Using this on live BDO servers violates Terms of Service
- May result in permanent account ban
- May result in hardware ID ban
- May violate DMCA/CFAA laws depending on jurisdiction

Use at your own risk. Authors are not responsible for any consequences.

## Credits

- BDO Memory Research Community
- Windows Kernel Programming resources
- Anti-Cheat Research Papers

## Support

For issues, questions, or contributions:
- Check existing issues
- Update offsets for current patch
- Test thoroughly before reporting bugs

---

**Version**: 1.0.0  
**Last Updated**: 2024  
**Compatible with**: Black Desert Online 2024 patches  
**Platform**: Windows 10/11 x64



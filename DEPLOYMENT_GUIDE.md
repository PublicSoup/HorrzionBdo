# BDO Secure Kernel Driver - Deployment Guide

## Prerequisites

### Software Requirements

1. **Windows 10/11 (64-bit)** - Version 1809 or later
2. **Visual Studio 2022** - With Desktop Development with C++
3. **Windows Driver Kit (WDK)** - Version 10.0.22621.0 or later
4. **Windows SDK** - Version 10.0.22621.0 or later
5. **Administrator Privileges** - Required for driver loading

### Hardware Requirements

- **x64 Processor** - AMD64 or Intel 64
- **4GB RAM minimum** - 8GB recommended
- **Virtualization** (optional) - For testing in VM

## Installation Steps

### Option 1: Test Signing (Development/Testing)

This is the recommended method for development and testing.

#### Step 1: Enable Test Signing

```batch
REM Open Command Prompt as Administrator
bcdedit /set testsigning on

REM Restart required
shutdown /r /t 0
```

After reboot, you'll see "Test Mode" watermark on desktop.

#### Step 2: Build the Driver

```batch
REM Open "x64 Free Build Environment" command prompt
cd "C:\path\to\HorrzionBdo"

REM Build driver
build_driver.bat
```

Output: `bin\x64\Release\BDO_SecureKernel.sys`

#### Step 3: Create Test Certificate

```batch
REM Create certificate
makecert -r -pe -n "CN=BDOTestCert" -ss TestCertStore -sr LocalMachine

REM Export certificate (optional)
certmgr /s TestCertStore /c /n "BDOTestCert" /all /r LocalMachine /s root
```

#### Step 4: Sign the Driver

```batch
REM Sign with test certificate
signtool sign /s TestCertStore /n "BDOTestCert" /t http://timestamp.digicert.com "bin\x64\Release\BDO_SecureKernel.sys"

REM Verify signature
signtool verify /pa /v "bin\x64\Release\BDO_SecureKernel.sys"
```

#### Step 5: Install the Driver

```batch
REM Method A: Using sc.exe
sc create BDOSecureKernel binPath= "C:\path\to\BDO_SecureKernel.sys" type= kernel
sc start BDOSecureKernel

REM Method B: Using INF file
rundll32.exe setupapi,InstallHinfSection DefaultInstall 132 BDO_SecureKernel.inf
```

#### Step 6: Verify Driver Loaded

```batch
sc query BDOSecureKernel

REM Or use PowerShell
Get-Service BDOSecureKernel
```

Expected output:
```
SERVICE_NAME: BDOSecureKernel
STATE: RUNNING
```

### Option 2: Manual Mapping (Production/Stealth)

This method loads the driver without creating a service, making it harder to detect.

#### Requirements

- MSI Afterburner installed (for RTCore64.sys)
- BDO_SecureLoader.exe compiled

#### Steps

1. **Install MSI Afterburner**
   - Download from: https://www.msi.com/Landing/afterburner
   - Install with default settings
   - Verifies RTCore64.sys is present

2. **Run Loader**
   ```batch
   REM As Administrator
   BDO_SecureLoader.exe BDO_SecureKernel.sys
   ```

3. **Verify Loading**
   ```cpp
   SecureKernelClient client;
   if (client.Initialize()) {
       // Driver loaded successfully
   }
   ```

## Using the Driver

### Basic Usage Example

```cpp
#include "BDO_SecureClient.h"

int main() {
    // Initialize client
    SecureKernelClient client;
    if (!client.Initialize()) {
        printf("Failed to initialize: %s\n", client.GetLastErrorString().c_str());
        return 1;
    }
    
    // Get Black Desert process ID (use Process Hacker or Task Manager)
    DWORD bdoProcessId = 12345; // Replace with actual PID
    
    // Get game base address
    ULONG64 baseAddress = 0;
    if (!client.GetProcessBaseAddress(bdoProcessId, baseAddress)) {
        printf("Failed to get base address\n");
        return 1;
    }
    
    printf("BDO Base Address: 0x%llX\n", baseAddress);
    
    // Read player health (example address)
    ULONG64 healthAddress = baseAddress + 0x3A5B2C8 + 0x28;
    int health = 0;
    
    if (client.Read<int>(bdoProcessId, healthAddress, health)) {
        printf("Player Health: %d\n", health);
    }
    
    // Enable stealth features
    client.EnableDriverHiding();
    
    client.Shutdown();
    return 0;
}
```

### Integration with Existing Code

```cpp
// In BDO_AdvancedMemory.cpp
#include "BDO_SecureClient.h"

bool BDOAdvancedMemory::Initialize(DWORD processId) {
    // Try secure kernel client first
    if (g_SecureClient.Initialize()) {
        KdPrint(("Using BDO Secure Kernel\n"));
        return true;
    }
    
    // Fallback to RTCore64 or other methods
    KdPrint(("Falling back to RTCore64\n"));
    return InitializeRTCore64();
}
```

## Uninstallation

### Test-Signed Driver

```batch
REM Stop driver
sc stop BDOSecureKernel

REM Delete service
sc delete BDOSecureKernel

REM Disable test signing (optional)
bcdedit /set testsigning off
shutdown /r /t 0
```

### Manually Mapped Driver

```batch
REM Driver will unload automatically on system restart
REM Or trigger unload via IOCTL (if implemented)
```

## Troubleshooting

### Driver Won't Load

**Error: "The driver has been blocked from loading"**

Solution:
1. Check if test signing is enabled: `bcdedit | findstr testsigning`
2. Verify driver is signed: `signtool verify /pa BDO_SecureKernel.sys`
3. Check Windows Event Log: Event Viewer > Windows Logs > System

**Error: "The service cannot be started"**

Solution:
1. Check file path in service configuration
2. Verify driver file exists and is accessible
3. Run as Administrator
4. Check dependencies

**Error: "STATUS_OBJECT_NAME_NOT_FOUND"**

Solution:
- Driver not loaded properly
- Symbolic link not created
- Check with: `sc query BDOSecureKernel`

### Client Can't Connect

**Error: "Failed to open driver device"**

Solution:
1. Verify driver is running: `sc query BDOSecureKernel`
2. Check device exists: `\\.\BDOSecureKernel`
3. Run client as Administrator
4. Check Windows firewall/antivirus

### Memory Operations Fail

**Error: "STATUS_ACCESS_VIOLATION"**

Solution:
- Verify address is valid
- Check process still running
- Verify address is in user space (< 0x7FFF...)
- Use correct process ID

**Error: "STATUS_INVALID_ADDRESS"**

Solution:
- Address is in kernel space
- Use user-mode address only
- Verify pointer chain

## Security Considerations

### Detection Risk

**Low Risk Actions:**
- Driver running in background
- Occasional memory reads (<10/sec)
- Reading only, no modifications

**Medium Risk Actions:**
- Frequent memory access (100+/sec)
- Pattern scanning large regions
- Writing memory values

**High Risk Actions:**
- Modifying game code
- Hooking game functions
- Obvious cheat behavior (teleport, etc.)

### Best Practices

1. **Limit Operation Frequency**
   - Max 100 operations per second
   - Add random delays (50-200ms)
   - Batch operations when possible

2. **Stealth Features**
   - Enable driver hiding: `client.EnableDriverHiding()`
   - Remove callbacks: `client.RemoveXigncodeCallbacks()`
   - Use legitimate driver name

3. **Memory Access Patterns**
   - Don't read in predictable patterns
   - Vary read sizes
   - Read extra bytes and discard
   - Use XOR encryption

4. **Testing**
   - Always test on alternate account first
   - Use VM for initial testing
   - Monitor for detection patterns
   - Be prepared for bans

### Legal Disclaimer

**IMPORTANT**: This driver is for educational and research purposes only. Using it to cheat in online games:

- Violates Terms of Service
- May result in permanent bans
- May result in hardware ID bans
- Is ethically wrong
- Ruins gameplay for others

Use responsibly and legally.

## Advanced Configuration

### Debug Mode

Enable verbose logging:

```c
// In BDO_SecureKernel.c
#define DBG 1

// View with DebugView or WinDbg
// Output: [BDO] Operation details
```

### Custom IOCTL Codes

To avoid detection, randomize IOCTL codes:

```c
// Generate random base
#define IOCTL_BASE (0x800 + (rand() % 0x100))
#define IOCTL_SECURE_READ CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE+1, ...)
```

Recompile both driver and client with same codes.

### Performance Tuning

Optimize for your use case:

```c
// Increase cache size for frequent small reads
#define CACHE_SIZE (16 * 1024)  // 16KB

// Adjust scan chunk size
#define SCAN_CHUNK_SIZE (0x100000)  // 1MB

// Set max concurrent operations
#define MAX_CONCURRENT_OPS 4
```

## Monitoring and Maintenance

### Check Driver Health

```batch
REM PowerShell script
$service = Get-Service BDOSecureKernel
if ($service.Status -eq "Running") {
    Write-Host "Driver is healthy" -ForegroundColor Green
} else {
    Write-Host "Driver is not running" -ForegroundColor Red
}
```

### Performance Metrics

Monitor operations:

```cpp
// Track operation count
static int operationCount = 0;
operationCount++;

if (operationCount % 1000 == 0) {
    printf("Performed %d operations\n", operationCount);
}
```

### Update Detection

Stay informed about game updates:
- BDO updates may change addresses
- Pattern signatures may need updates
- Xigncode3 may add new detection methods

## Support

For issues or questions:
- Check documentation files
- Review existing codebase
- Test in isolated environment
- Monitor Windows Event Logs

## Version History

- **v1.0.0** (2024-10-21): Initial release
  - KMDF-based driver
  - Secure IOCTL interface
  - XOR encryption
  - Pattern scanning
  - Stealth features

---

*Document Version: 1.0*
*Last Updated: 2024-10-21*


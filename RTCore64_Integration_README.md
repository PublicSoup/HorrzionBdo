# RTCore64 Integration for BDO Stealth Bot

This document describes the integration of RTCore64 driver into the BDO Stealth Bot project.

## Overview

RTCore64.sys is a vulnerable driver from MSI Afterburner that allows kernel-level memory access. This integration provides a clean interface to use RTCore64 for bypassing BDO's anti-cheat protection.

## Files Added

### Core Driver Integration
- **BDO_RTCore64_Driver.h/cpp** - Driver interface for RTCore64
- **BDO_RTCore64_Error.h/cpp** - Error handling system for RTCore64 operations

### Memory System Integration
- Updated **BDO_MemoryResolver.h/cpp** - Now uses RTCore64 as primary memory access method
- Updated **BDO_AdvancedMemory.h/cpp** - Now uses RTCore64 for all memory operations

### Testing
- **RTCore64_Test.cpp** - Basic test program for RTCore64 driver
- **RTCore64_Comprehensive_Test.cpp** - Comprehensive test suite for RTCore64 integration
- **RTCore64_Main.cpp** - Entry point for RTCore64 testing

### Scripts
- **build_rtcore64.bat** - Builds the RTCore64 version
- **run_rtcore64_test.bat** - Runs the basic RTCore64 test
- **run_rtcore64_comprehensive_test.bat** - Runs the comprehensive test suite

## Setup Instructions

### 1. Install MSI Afterburner

Download and install MSI Afterburner from the official website:
https://www.msi.com/Landing/afterburner/graphics-cards

### 2. Set Up RTCore64 Driver

After installing MSI Afterburner, the RTCore64.sys driver should be located at:
`C:\Program Files (x86)\MSI Afterburner\RTCore64.sys`

Create and start the RTCore64 service:
```cmd
sc create RTCore64 type= kernel binPath= "C:\Program Files (x86)\MSI Afterburner\RTCore64.sys" DisplayName= "RTCore64"
sc start RTCore64
```

### 3. Verify Driver Status

Check if the RTCore64 service is running:
```cmd
sc query RTCore64
```

### 4. Build the Project

Build the RTCore64 version of the project:
```cmd
build_rtcore64.bat
```

### 5. Test the Integration

Run the basic test:
```cmd
run_rtcore64_test.bat
```

Run the comprehensive test suite:
```cmd
run_rtcore64_comprehensive_test.bat
```

## Usage in Code

### Basic Memory Operations

```cpp
// Include the header
#include "BDO_RTCore64_Driver.h"

// Connect to the driver
if (g_RTCore64.Connect()) {
    // Read memory
    DWORD value;
    g_RTCore64.ReadMemory(address, &value, sizeof(value));
    
    // Write memory
    DWORD newValue = 100;
    g_RTCore64.WriteMemory(address, &newValue, sizeof(newValue));
    
    // Disconnect when done
    g_RTCore64.Disconnect();
}
```

### Using with Memory Resolver

The `BDOMemoryResolver` class now automatically uses RTCore64 as its primary memory access method. No code changes are needed to benefit from RTCore64 integration.

### Using with Advanced Memory

The `BDOAdvancedMemory` class now uses RTCore64 for all memory operations. Simply use the class as before, and it will automatically use RTCore64 when available.

## Error Handling

The RTCore64 integration includes a comprehensive error handling system:

```cpp
// Get the last error
RTCore64Error error = g_RTCore64ErrorHandler.GetLastError();

// Check error code
if (error.code != RTCore64ErrorCode::SUCCESS) {
    std::cout << "Error: " << g_RTCore64ErrorHandler.FormatErrorMessage(error) << std::endl;
}

// Enable error logging
g_RTCore64ErrorHandler.EnableConsoleLogging(true);
g_RTCore64ErrorHandler.EnableFileLogging(true, "RTCore64_Error.log");
```

## Command Line Options

The BDO Stealth Bot now supports the following command line options:

- `--rtcore64` - Run the RTCore64 test
- `--rtcore64 --comprehensive` - Run the comprehensive RTCore64 test suite

## Troubleshooting

### Driver Not Found

If the RTCore64 driver is not found, make sure:
1. MSI Afterburner is installed
2. The RTCore64 service is created and started
3. You have administrator privileges

### Access Denied

If you get "Access Denied" errors:
1. Make sure you're running as administrator
2. Check if another process is using the driver
3. Restart your computer and try again

### Memory Read/Write Failures

If memory operations fail:
1. Check the error log for details
2. Verify the address is valid
3. Try smaller read/write operations
4. Ensure the process has the necessary permissions

## Security Considerations

Using RTCore64.sys poses security risks as it's a vulnerable driver. Use only for educational purposes and in controlled environments.

## References

- [MSI Afterburner](https://www.msi.com/Landing/afterburner/graphics-cards)
- [Windows Driver Development](https://docs.microsoft.com/en-us/windows-hardware/drivers/)
- [Kernel Memory Access](https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/accessing-user-data-from-kernel-mode)
- [DeviceIoControl Function](https://docs.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-deviceiocontrol)

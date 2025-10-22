# RTCore64 Integration for BDO Stealth Bot

This document describes the integration of RTCore64 driver into the BDO Stealth Bot project.

## Overview

RTCore64.sys is a vulnerable driver from MSI Afterburner that allows kernel-level memory access. This integration provides a clean interface to use RTCore64 for bypassing BDO's anti-cheat protection.

## Files Added

1. **BDO_RTCore64_Driver.h**
   - Header file defining the RTCore64Driver class
   - Provides interface for kernel memory operations

2. **BDO_RTCore64_Driver.cpp**
   - Implementation of the RTCore64Driver class
   - Handles communication with the RTCore64 driver

3. **RTCore64_Test.cpp**
   - Test program to verify RTCore64 driver functionality
   - Can be used to check if the driver is properly loaded

## How to Use

### 1. Load the RTCore64 Driver

Before using the RTCore64 integration, make sure the driver is loaded:

```cmd
sc query RTCore64
```

If not loaded, load it:

```cmd
sc create RTCore64 type= kernel binPath= "C:\Program Files (x86)\MSI Afterburner\RTCore64.sys"
sc start RTCore64
```

### 2. Using in Your Code

```cpp
// Include the header
#include "BDO_RTCore64_Driver.h"

// Connect to the driver
if (!g_RTCore64.Connect()) {
    // Handle error
    return;
}

// Read memory
DWORD value;
if (g_RTCore64.ReadMemory(address, &value, sizeof(value))) {
    // Use value
}

// Write memory
DWORD newValue = 100;
g_RTCore64.WriteMemory(address, &newValue, sizeof(newValue));

// Disconnect when done
g_RTCore64.Disconnect();
```

### 3. Integration with BDO_MemoryResolver

To use RTCore64 with the existing memory resolver:

```cpp
// In BDO_MemoryResolver.cpp

bool BDOMemoryResolver::ReadMemory(PVOID address, PVOID buffer, SIZE_T size) {
    // Try RTCore64 first
    if (g_RTCore64.IsConnected() || g_RTCore64.Connect()) {
        if (g_RTCore64.ReadMemory((ULONG64)address, buffer, size)) {
            return true;
        }
    }
    
    // Fall back to original implementation
    // ...
}
```

## Building the Test Program

The RTCore64_Test.cpp file is included in the project. To build it:

1. Set it as the startup project in Visual Studio
2. Build and run

## Security Considerations

Using RTCore64.sys poses security risks as it's a vulnerable driver. Use only for educational purposes and in controlled environments.

## Troubleshooting

If you encounter issues:

1. Make sure MSI Afterburner is installed
2. Verify the RTCore64 service is running
3. Check if test signing is enabled (`bcdedit /set testsigning on`)
4. Disable driver blocklist if needed

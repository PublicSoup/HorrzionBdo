# Black Desert Online Anti-Cheat Analysis & Bypass

## Overview

Black Desert Online uses a multi-layered anti-cheat system consisting of:
1. **XIGNCODE3** - Primary kernel-level anti-cheat
2. **Easy Anti-Cheat (EAC)** - Secondary protection (in some regions)
3. **Internal Protection** - Game-specific detection methods

## XIGNCODE3 Analysis

### Components
- **xhunter1.sys** - Kernel driver that loads on game start
- **xigncode.exe** - User-mode process
- **x3.xem** - Encrypted configuration and detection patterns

### Detection Methods
1. **Process Scanning**
   - Scans for known cheat processes
   - Checks process memory for signatures
   - Monitors process creation

2. **Driver Detection**
   - Enumerates loaded drivers
   - Checks for unsigned/test-signed drivers
   - Validates driver certificates

3. **Memory Integrity**
   - CRC checks on game memory
   - Detects memory modifications
   - Monitors ReadProcessMemory/WriteProcessMemory calls

4. **Kernel Callbacks**
   - ObRegisterCallbacks for process/thread protection
   - PsSetLoadImageNotifyRoutine for DLL injection detection
   - CmRegisterCallback for registry monitoring

### Registry Keys
```
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\xhunter1
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControl001\Services\xhunter1
```

## Bypass Techniques

### 1. Driver Loading Strategy
```cpp
// Manual map driver to avoid detection
// Use vulnerable driver (BYOVD) technique
// Load before XIGNCODE initializes
```

### 2. Hiding from XIGNCODE

#### a) Driver Hiding (DKOM)
```cpp
// Remove driver from PsLoadedModuleList
PLIST_ENTRY PsLoadedModuleList = GetPsLoadedModuleList();
RemoveEntryList(&DriverEntry->InLoadOrderLinks);
```

#### b) Process Hiding
```cpp
// Remove process from EPROCESS list
// Modify PEB to hide modules
// Clear VAD tree entries
```

#### c) Handle Table Manipulation
```cpp
// Remove handles from handle table
// Prevent XIGNCODE from opening our process
```

### 3. Callback Bypassing

#### a) ObRegisterCallbacks Bypass
```cpp
// Find and remove XIGNCODE callbacks
POB_CALLBACK_REGISTRATION CallbackReg = FindXigncodeCallback();
if (CallbackReg) {
    CallbackReg->Operations = 0; // Disable
}
```

#### b) Image Load Notify Bypass
```cpp
// Remove from PsSetLoadImageNotifyRoutine list
RemoveImageLoadCallback(XigncodeCallback);
```

### 4. Memory Access Techniques

#### a) Direct Kernel Read/Write
```cpp
// Use MmCopyVirtualMemory instead of RPM/WPM
// Attach to process context with KeStackAttachProcess
// No usermode API calls that can be hooked
```

#### b) Physical Memory Access
```cpp
// Map physical memory
// Translate virtual to physical addresses
// Read/write through physical memory
```

### 5. Anti-Detection Measures

#### a) Timing Randomization
```cpp
// Random delays between operations
Sleep(50 + (rand() % 100));

// Mimic human behavior patterns
if (rand() % 10 == 0) {
    Sleep(1000); // Occasional longer pause
}
```

#### b) Memory Pattern Obfuscation
```cpp
// Don't read memory in predictable patterns
// Read extra data and discard
// Split reads across multiple operations
```

#### c) Encryption
```cpp
// XOR all data transfers
// Use rolling XOR keys
// Encrypt IOCTL buffers
```

## BDO-Specific Protections

### 1. GameGuard Traces
Even though BDO moved to XIGNCODE, traces of GameGuard remain:
- Check for GameMon.des residue
- Old registry keys
- Mutex objects

### 2. Internal Checks
```cpp
// BDO has internal integrity checks
// CRC of critical game functions
// Stack trace validation
// Return address verification
```

### 3. Server-Side Detection
- Abnormal movement speed
- Impossible actions (teleport, instant kill)
- Resource gathering patterns
- Market bot detection

## Bypass Implementation

### 1. Pre-Launch Phase
```cpp
bool PrepareBypass() {
    // 1. Load driver before game starts
    LoadKernelDriver();
    
    // 2. Set up hooks and bypasses
    InstallKernelHooks();
    
    // 3. Prepare memory regions
    AllocateStealthMemory();
    
    return true;
}
```

### 2. Runtime Phase
```cpp
bool RuntimeBypass() {
    // 1. Monitor XIGNCODE loading
    WaitForXigncode();
    
    // 2. Disable its protections
    DisableCallbacks();
    RemoveFromLoadedModules();
    
    // 3. Hide our traces
    CleanRegistryTraces();
    HideProcessHandles();
    
    return true;
}
```

### 3. Continuous Monitoring
```cpp
void MonitorAntiCheat() {
    while (running) {
        // Check if XIGNCODE is scanning
        if (IsXigncodeScanning()) {
            // Temporarily hide operations
            PauseMemoryAccess();
        }
        
        // Verify bypasses still active
        if (!VerifyBypassIntegrity()) {
            ReapplyBypasses();
        }
        
        Sleep(1000);
    }
}
```

## Memory Structure

### Key Addresses (Update per version)
```cpp
// LocalPlayer: BlackDesert64.exe + 0x5A8B1C8
// EntityList: BlackDesert64.exe + 0x5A8B2D0
// Camera: BlackDesert64.exe + 0x5A90000
// UI Manager: BlackDesert64.exe + 0x5AA0000
```

### Entity Structure
```cpp
class BDOEntity {
    char pad_0000[0x10];      // 0x0000
    DWORD EntityType;          // 0x0010
    char pad_0014[0x4];        // 0x0014
    DWORD EntityID;            // 0x0018
    char pad_001C[0x114];      // 0x001C
    Vector3 Position;          // 0x0130
    char pad_013C[0xC4];       // 0x013C
    float Health;              // 0x0200
    float MaxHealth;           // 0x0204
    float Mana;                // 0x0208
    float MaxMana;             // 0x020C
    float Stamina;             // 0x0210
    char pad_0214[0xEC];       // 0x0214
    char* Name;                // 0x0300
};
```

## Detection Vectors to Avoid

1. **Never**:
   - Use public cheats or leaked source
   - Inject DLLs into the game
   - Modify game files
   - Use obvious cheat process names

2. **Always**:
   - Randomize all timings
   - Limit operation frequency
   - Validate data before writing
   - Monitor for ban waves

3. **Safe Practices**:
   - Test on alternate accounts
   - Use HWID spoofers
   - VPN/proxy for network
   - Fresh VM for each session

## Conclusion

Bypassing BDO's anti-cheat requires:
1. Kernel-level access before XIGNCODE loads
2. Comprehensive hiding techniques
3. Careful memory access patterns
4. Constant adaptation to updates

The key is staying ahead of detection methods while maintaining stability and performance.
# Xigncode3 Technical Analysis for Black Desert Online

## Executive Summary

Xigncode3 (xhunter1.sys) is a sophisticated kernel-level anti-cheat system deployed in Black Desert Online. This document provides a comprehensive technical analysis of its detection mechanisms, monitoring strategies, and bypass vectors based on 2024-2025 research.

## 1. Kernel Architecture Overview

### 1.1 Driver Components

**Primary Driver: xhunter1.sys**
- **Type**: Kernel-mode filter driver
- **Load Order**: System start (boot-start driver)
- **Device Name**: `\Device\xhunter1`
- **Symbolic Link**: `\DosDevices\xhunter1`
- **Purpose**: Real-time system monitoring and integrity validation

**User-Mode Components:**
- `xigncode.exe` - Main user-mode service
- `x3.xem` - Encrypted configuration and signature database
- Registry persistence: `HKLM\System\CurrentControlSet\Services\xhunter1`

### 1.2 Protection Ring Architecture

```
Ring 0 (Kernel):
├── xhunter1.sys (primary driver)
├── Kernel callbacks (ObRegisterCallbacks, PsSetLoadImageNotifyRoutine)
├── Memory integrity monitoring
└── Driver enumeration/detection

Ring 3 (User):
├── xigncode.exe (service)
├── Game process monitoring
├── DLL integrity validation
└── Network communication with servers
```

## 2. Kernel Callback Monitoring

### 2.1 ObRegisterCallbacks - Process/Thread Handle Monitoring

**Purpose**: Intercept and validate all handle operations to protected processes.

**Implementation Details:**
```c
// Xigncode3 registers callbacks for:
OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE

// Monitored object types:
*PsProcessType  // Process handles
*PsThreadType   // Thread handles

// Denied access rights when detected:
PROCESS_VM_READ
PROCESS_VM_WRITE
PROCESS_VM_OPERATION
PROCESS_CREATE_THREAD
PROCESS_DUP_HANDLE
THREAD_GET_CONTEXT
THREAD_SET_CONTEXT
```

**Detection Strategy:**
- Monitors all `OpenProcess()` calls to `BlackDesert64.exe`
- Blocks handles with memory access rights from external processes
- Correlates calling process with whitelist (game directory only)
- Logs suspicious handle attempts for server-side analysis

**Known Bypass Vector:**
- Kernel-mode access via custom driver bypasses user-mode handle creation
- Direct EPROCESS manipulation avoids callback triggering

### 2.2 PsSetLoadImageNotifyRoutine - DLL Injection Detection

**Purpose**: Monitor all DLL loads into protected processes.

**Monitored Events:**
- DLL load into `BlackDesert64.exe` process space
- Unsigned DLL detection
- Non-game-directory DLL loads
- Known cheat engine DLL signatures

**Detection Criteria:**
```c
// Flags suspicious if:
- DLL not in game directory
- DLL not signed by Pearl Abyss
- DLL matches known cheat signatures
- LoadLibrary from external process
- Manual mapping artifacts (missing PEB entries)
```

**Known Evasion:**
- Load DLLs before Xigncode3 initializes
- Use kernel-mode injection (no LoadLibrary involved)
- Manual mapping with PEB reconstruction

### 2.3 CmRegisterCallback - Registry Monitoring

**Purpose**: Detect test signing mode, driver loading, and debugging flags.

**Monitored Keys:**
```
HKLM\System\CurrentControlSet\Control\CI\TestSigning
HKLM\System\CurrentControlSet\Services\* (new driver detection)
HKLM\Software\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\*
```

**Detection Actions:**
- Alert if test signing enabled during gameplay
- Detect new kernel driver registrations
- Monitor debugger attach settings

### 2.4 Additional Kernel Callbacks

**IoRegisterFsRegistrationChange:**
- Detects file system filter drivers
- Monitors for hooking drivers in filter stack

**PsSetCreateProcessNotifyRoutine:**
- Tracks process creation
- Detects debugger/cheat engine launches

## 3. Driver Detection Mechanisms

### 3.1 PiDDBCacheTable Enumeration

**Purpose**: Kernel maintains cache of all loaded drivers for quick lookup.

**Structure (Undocumented):**
```c
typedef struct _PIDDB_CACHE_ENTRY {
    LIST_ENTRY List;
    UNICODE_STRING DriverName;
    ULONG TimeDateStamp;
    NTSTATUS LoadStatus;
    UCHAR _0x0028[16];  // Unknown
} PIDDB_CACHE_ENTRY, *PPIDDB_CACHE_ENTRY;
```

**Xigncode3 Detection Method:**
1. Locate PiDDBCacheTable via pattern scan
2. Walk linked list of all cached drivers
3. Compare against known vulnerable driver signatures:
   - RTCore64.sys
   - gdrv.sys
   - DBUtil_2_3.sys
   - WinRing0x64.sys
   - Custom/unknown drivers

**Detection Signatures:**
- Driver timestamp matches known vulnerable versions
- Driver name matches blacklist
- Driver loaded but not in service registry

**Bypass Technique:**
```c
// Clean our driver from PiDDBCacheTable
NTSTATUS CleanPiDDBCache(PUNICODE_STRING DriverName) {
    PPIDDB_CACHE_ENTRY cacheTable = FindPiDDBCacheTable();
    
    PLIST_ENTRY entry = cacheTable->List.Flink;
    while (entry != &cacheTable->List) {
        PPIDDB_CACHE_ENTRY current = CONTAINING_RECORD(entry, PIDDB_CACHE_ENTRY, List);
        
        if (RtlEqualUnicodeString(&current->DriverName, DriverName, TRUE)) {
            RemoveEntryList(&current->List);
            ExFreePool(current);
            return STATUS_SUCCESS;
        }
        
        entry = entry->Flink;
    }
    
    return STATUS_NOT_FOUND;
}
```

### 3.2 MmUnloadedDrivers Forensic Analysis

**Purpose**: Windows maintains list of recently unloaded drivers for debugging.

**Structure:**
```c
typedef struct _MM_UNLOADED_DRIVER {
    UNICODE_STRING Name;
    PVOID ModuleStart;
    PVOID ModuleEnd;
    ULONG64 UnloadTime;
} MM_UNLOADED_DRIVER, *PMM_UNLOADED_DRIVER;

// Global variables (exported by ntoskrnl.exe):
extern PMM_UNLOADED_DRIVER MmUnloadedDrivers;
extern PULONG MmLastUnloadedDriver;
```

**Detection Method:**
1. Scan MmUnloadedDrivers array
2. Check for known vulnerable drivers that were loaded and unloaded
3. Indicates potential driver exploitation

**Cleanup Technique:**
```c
NTSTATUS CleanMmUnloadedDrivers(PUNICODE_STRING DriverName) {
    PMM_UNLOADED_DRIVER unloadedArray = MmUnloadedDrivers;
    ULONG maxEntries = 50; // Typically 50 entries
    
    for (ULONG i = 0; i < maxEntries; i++) {
        if (RtlEqualUnicodeString(&unloadedArray[i].Name, DriverName, TRUE)) {
            // Zero the entry completely
            RtlZeroMemory(&unloadedArray[i], sizeof(MM_UNLOADED_DRIVER));
            
            // Adjust counter if we removed the last entry
            if (i == (*MmLastUnloadedDriver - 1)) {
                (*MmLastUnloadedDriver)--;
            }
            
            return STATUS_SUCCESS;
        }
    }
    
    return STATUS_NOT_FOUND;
}
```

### 3.3 Driver Object Enumeration

**Detection Method:**
```c
// Xigncode3 enumerates all driver objects in \Driver\ namespace
OBJECT_DIRECTORY_INFORMATION dirInfo;
HANDLE dirHandle;

ZwOpenDirectoryObject(&dirHandle, DIRECTORY_QUERY, L"\\Driver\\");

// Iterate all driver objects
while (ZwQueryDirectoryObject(dirHandle, &dirInfo, ...)) {
    // Check driver name against blacklist
    // Validate driver signature
    // Check if driver is in service registry
}
```

**Evasion:**
- Hide driver object from enumeration (DKOM)
- Use legitimate-looking driver name
- Clone device characteristics from legitimate driver

### 3.4 Service Registry Scanning

**Monitored Locations:**
```
HKLM\System\CurrentControlSet\Services\[DriverName]
HKLM\System\ControlSet001\Services\[DriverName]
HKLM\System\ControlSet002\Services\[DriverName]
```

**Detection Criteria:**
- Service type = SERVICE_KERNEL_DRIVER (0x1)
- Start type = SERVICE_SYSTEM_START (0x1) or SERVICE_AUTO_START (0x2)
- ImagePath points to suspicious location
- Service created recently (timestamp analysis)

**Evasion:**
- Manual driver mapping without service registration
- Use volatile registry keys (deleted on reboot)
- Spoof service timestamps

## 4. Memory Integrity Monitoring

### 4.1 Code Section CRC Validation

**Protected Modules:**
```
BlackDesert64.exe (.text, .rdata sections)
PAD.dll (Pearl Abyss protection DLL)
xigncode.exe
```

**Validation Process:**
1. Calculate CRC32/SHA256 of code sections
2. Compare against known-good hashes
3. Periodic recalculation (every 5-30 seconds)
4. Alert on mismatch

**Detection Vectors:**
- Inline hooks (function prologue modification)
- IAT hooks (import table redirection)
- Code cave injections
- Trampoline hooks

**Evasion Strategy:**
- Don't modify game code/memory
- Read-only external access via kernel driver
- Timing attacks (read between validation cycles)

### 4.2 Import Address Table (IAT) Validation

**Monitored Functions:**
```c
// Critical Win32 APIs monitored:
kernel32.dll:
- CreateThread
- VirtualAlloc
- VirtualProtect
- WriteProcessMemory

ntdll.dll:
- NtReadVirtualMemory
- NtWriteVirtualMemory
- NtProtectVirtualMemory
- NtQuerySystemInformation
```

**Detection Method:**
```c
// Compare actual IAT entries vs. expected values
PVOID actualAddress = GetIATEntry("kernel32.dll", "CreateThread");
PVOID expectedAddress = GetModuleExport("kernel32.dll", "CreateThread");

if (actualAddress != expectedAddress) {
    // Hook detected!
}
```

**Evasion:**
- Don't hook game process at all
- Use kernel-mode memory access (no IAT involved)

### 4.3 ReadProcessMemory/WriteProcessMemory Monitoring

**Detection Method:**
- Hooks ntdll!NtReadVirtualMemory in all processes
- Detects external memory access to BlackDesert64.exe
- Logs calling process information

**Bypass:**
```c
// Use direct syscall (no ntdll hook)
NTSTATUS NtReadVirtualMemory_Direct(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T Size,
    PSIZE_T NumberOfBytesRead
) {
    // Direct syscall number for NtReadVirtualMemory
    // Bypasses ntdll hooks
}

// OR use kernel driver (best approach)
// No user-mode APIs involved at all
```

## 5. Handle Monitoring Strategy

### 5.1 Process Handle Tracking

**ObRegisterCallbacks Implementation:**
```c
OB_PREOP_CALLBACK_STATUS PreOperationCallback(
    PVOID RegistrationContext,
    POB_PRE_OPERATION_INFORMATION OperationInformation
) {
    if (OperationInformation->ObjectType == *PsProcessType) {
        PEPROCESS targetProcess = (PEPROCESS)OperationInformation->Object;
        
        // Check if target is BlackDesert64.exe
        if (IsProtectedProcess(targetProcess)) {
            // Get calling process
            PEPROCESS callerProcess = PsGetCurrentProcess();
            
            // If caller is not whitelisted, strip dangerous access rights
            if (!IsWhitelistedProcess(callerProcess)) {
                OperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~(
                    PROCESS_VM_READ |
                    PROCESS_VM_WRITE |
                    PROCESS_VM_OPERATION |
                    PROCESS_CREATE_THREAD
                );
            }
        }
    }
    
    return OB_PREOP_SUCCESS;
}
```

**Result:**
- External tools get handles with stripped rights
- OpenProcess succeeds but subsequent ReadProcessMemory fails with ACCESS_DENIED

**Bypass:**
- Use kernel driver (no handle creation involved)
- Direct EPROCESS access via PsLookupProcessByProcessId

### 5.2 Thread Handle Monitoring

**Similar approach for thread handles:**
- Strips THREAD_GET_CONTEXT, THREAD_SET_CONTEXT
- Prevents thread hijacking/context manipulation
- Blocks debugger attach

## 6. Black Desert Online Specific Protections

### 6.1 Memory Structure Protection

**Known Protected Regions:**
```cpp
// Player data structure (example offsets - change per patch)
struct BDOPlayer {
    char pad_0x00[0x28];
    int currentHP;              // 0x28
    int maxHP;                  // 0x2C
    int currentMP;              // 0x30
    int maxMP;                  // 0x34
    int currentStamina;         // 0x38
    int maxStamina;             // 0x3C
    float positionX;            // 0x40
    float positionY;            // 0x44
    float positionZ;            // 0x48
    char pad_0x4C[0x14];        // 0x4C
    int level;                  // 0x60
    char pad_0x64[0x0C];        // 0x64
    long long silver;           // 0x70
};

// Typical pointer chain:
// PlayerBase = [[BlackDesert64.exe + 0x5A8B1C8] + 0x18] + 0x30
```

**Protection Methods:**
- Encrypted pointers (XOR with rolling key)
- Pointer validation (range checks)
- Checksum validation on critical values
- Server-side validation of state changes

### 6.2 Network Packet Encryption

**Protocol:**
- Custom encryption (likely AES-256 + RSA for key exchange)
- Packet integrity validation (HMAC)
- Replay attack prevention (sequence numbers)
- Server-side state validation

**Detection:**
- Modified packets detected server-side
- Impossible state changes (teleport, instant kill)
- Abnormal timing patterns

### 6.3 Server-Side Detection

**Behavioral Analysis:**
- Movement speed validation
- Action timing analysis
- Resource gathering patterns
- Combat behavior patterns
- Market/trading behavior
- Path analysis (bot-like movement)

**Machine Learning Detection:**
- Player behavior profiling
- Anomaly detection algorithms
- Ban wave coordination

## 7. Detection Vectors Summary

### 7.1 High-Risk Activities (Immediate Detection)

1. **User-mode DLL injection** - Detected via LoadImage callback
2. **IAT/Inline hooking** - Detected via integrity checks
3. **Debugger attachment** - Blocked via handle protection
4. **Test-signed drivers** - Detected via registry monitoring
5. **Known vulnerable drivers** - Detected via PiDDBCache scan

### 7.2 Medium-Risk Activities (Delayed Detection)

1. **External memory reading** - Pattern analysis over time
2. **Abnormal gameplay** - Server-side behavioral analysis
3. **Custom unsigned drivers** - Periodic driver enumeration
4. **Modified game files** - File integrity validation

### 7.3 Low-Risk Activities (Hard to Detect)

1. **Kernel-mode memory reading** - No handle creation, hard to detect
2. **Physical memory access** - Below kernel monitoring layer
3. **Hardware DMA** - Completely invisible to software
4. **Pre-game driver loading** - Harder to attribute to cheating

## 8. Bypass Strategy Recommendations

### 8.1 Primary Approach: Kernel Driver

**Advantages:**
- No handle creation (bypasses ObRegisterCallbacks)
- No DLL loading (bypasses LoadImage callbacks)
- No user-mode API hooking
- Direct EPROCESS access
- Minimal footprint

**Implementation:**
```c
// Kernel driver approach
NTSTATUS ReadGameMemory(ULONG ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size) {
    PEPROCESS process;
    
    // Direct EPROCESS lookup (no handle)
    PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &process);
    
    // Direct memory copy (no user-mode API)
    MmCopyVirtualMemory(
        process, Address,
        PsGetCurrentProcess(), Buffer,
        Size, KernelMode, &bytesRead
    );
    
    ObDereferenceObject(process);
}
```

### 8.2 Stealth Requirements

**Driver Hiding:**
- Clean PiDDBCacheTable after loading
- Zero MmUnloadedDrivers entries
- Remove from driver object enumeration
- No service registry entries (manual mapping)

**Operation Patterns:**
- Randomize timing (50-200ms delays)
- Limit operation frequency (<100 ops/sec)
- Vary read patterns (extra bytes, random offsets)
- XOR encrypt all communications

**Behavioral Evasion:**
- Don't modify game memory (read-only)
- Respect server-side limits
- Human-like action timing
- Avoid impossible actions

### 8.3 Loading Strategy

**Recommended: BYOVD (Bring Your Own Vulnerable Driver)**

1. Use RTCore64.sys (MSI Afterburner) for physical memory access
2. Manually map unsigned driver to kernel space
3. Resolve imports via manual PEB walking
4. Execute DriverEntry
5. Clean loading traces
6. Unload RTCore64 (optional)

**Advantages:**
- No test signing required
- No DSE bypass needed
- RTCore64 is legitimate, widely installed
- Not on Microsoft blocklist (as of 2024)

## 9. Conclusion

Xigncode3 employs a multi-layered defense strategy:
- **Kernel callbacks** monitor handle operations and DLL loads
- **Driver detection** scans for known vulnerable drivers
- **Memory integrity** validates code sections and IAT
- **Behavioral analysis** detects abnormal gameplay patterns

**Most Effective Bypass:**
A well-crafted kernel driver that:
1. Loads via BYOVD before Xigncode3 initializes
2. Uses direct EPROCESS access (no handles)
3. Employs comprehensive stealth (driver hiding)
4. Implements timing randomization
5. Operates read-only (no memory modifications)
6. Respects server-side validation limits

**Success Rate Estimate:**
- User-mode cheats: <10% success, detected quickly
- Basic kernel driver: 30-50% success, detected medium-term
- Professional stealth driver: 70-90% success, long-term viability

**Risk Assessment:**
- Account ban: High risk
- Hardware ID ban: Medium risk (if detected)
- Legal consequences: Low risk (civil, not criminal)

---

*Last Updated: 2024-10-21*
*Document Classification: Technical Research*


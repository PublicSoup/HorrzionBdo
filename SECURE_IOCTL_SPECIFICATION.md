# Secure IOCTL Interface Specification

## Overview

This document defines the secure communication protocol between user-mode applications and the BDO Secure Kernel driver, including comprehensive input validation, XOR encryption, and error handling.

## IOCTL Codes

```c
#define IOCTL_SECURE_READ       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SECURE_WRITE      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_MODULE_BASE   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PATTERN_SCAN      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HIDE_DRIVER       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REMOVE_CALLBACKS  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x906, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PROCESS_BASE  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x907, METHOD_BUFFERED, FILE_ANY_ACCESS)
```

## Request Structures

### SECURE_MEMORY_REQUEST

Used for IOCTL_SECURE_READ and IOCTL_SECURE_WRITE operations.

```c
typedef struct _SECURE_MEMORY_REQUEST {
    ULONG Magic;                // 0x42444F4B for validation
    ULONG ProcessId;           // Target process ID
    ULONG64 Address;           // Target memory address
    ULONG Size;                // Size of data to read/write
    UCHAR XORKey;              // Rolling XOR encryption key
    UCHAR Data[1];             // Variable length data (write only)
} SECURE_MEMORY_REQUEST;
```

**Validation Checklist:**
- Magic number must equal 0x42444F4B
- ProcessId must be valid (> 4)
- Address must be in user space (< 0xFFFF800000000000)
- Size must be > 0 and <= 64MB
- Integer overflow check: Address + Size > Address

### MODULE_INFO_REQUEST

Used for IOCTL_GET_MODULE_BASE.

```c
typedef struct _MODULE_INFO_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    WCHAR ModuleName[260];     // Module name to find
    ULONG64 BaseAddress;       // Output: module base address
    ULONG64 ModuleSize;        // Output: module size
} MODULE_INFO_REQUEST;
```

### PATTERN_SCAN_REQUEST

Used for IOCTL_PATTERN_SCAN.

```c
typedef struct _PATTERN_SCAN_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    ULONG64 StartAddress;      // Scan range start
    ULONG64 EndAddress;        // Scan range end
    UCHAR Pattern[256];        // Byte pattern
    UCHAR Mask[256];           // Mask (0xFF=must match, 0x00=wildcard)
    ULONG PatternLength;       // Length of pattern
    ULONG64 ResultAddress;     // Output: address where found
} PATTERN_SCAN_REQUEST;
```

## Security Features

### 1. XOR Encryption

All data transferred is encrypted using a rolling XOR cipher:

```c
VOID XOREncryptDecrypt(PUCHAR Data, SIZE_T Size, UCHAR Key) {
    UCHAR currentKey = Key;
    
    for (SIZE_T i = 0; i < Size; i++) {
        Data[i] ^= currentKey;
        currentKey = (currentKey * 31 + 17) & 0xFF; // Rolling key
    }
}
```

**Properties:**
- Simple and fast
- Different key for each byte (rolling)
- Client provides initial key
- Prevents simple memory scanning

### 2. Input Validation

Every IOCTL handler implements:

```c
NTSTATUS HandleSecureRequest(PVOID Input, SIZE_T InputLen, ...) {
    // 1. Validate buffer size
    if (InputLen < sizeof(EXPECTED_STRUCT)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    REQUEST* req = (REQUEST*)Input;
    
    // 2. Validate magic number
    if (req->Magic != 0x42444F4B) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // 3. Validate process ID
    if (req->ProcessId < 4) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // 4. Validate address range
    if (req->Address >= 0xFFFF800000000000ULL) {
        return STATUS_INVALID_ADDRESS;
    }
    
    // 5. Validate size limits
    if (req->Size == 0 || req->Size > MAX_TRANSFER_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // 6. Check integer overflow
    if (req->Address + req->Size < req->Address) {
        return STATUS_INTEGER_OVERFLOW;
    }
    
    // 7. Process request with exception handling
    __try {
        return ProcessRequest(req);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }
}
```

### 3. Exception Handling

All memory operations wrapped in __try/__except:

```c
__try {
    status = MmCopyVirtualMemory(...);
}
__except(EXCEPTION_EXECUTE_HANDLER) {
    status = GetExceptionCode();
    KdPrint(("[BDO] Exception: 0x%X\n", status));
}
```

## Usage Examples

### Read Memory

**User-Mode:**
```cpp
SecureKernelClient client;
client.Initialize();

int health = 0;
bool success = client.Read<int>(processId, healthAddress, health);
```

**Driver Processing:**
1. Validate request structure
2. Call SafeReadProcessMemory()
3. XOR encrypt result
4. Return to user-mode
5. Client XOR decrypts

### Write Memory

**User-Mode:**
```cpp
int newHealth = 9999;
bool success = client.Write<int>(processId, healthAddress, newHealth);
```

**Driver Processing:**
1. Validate request structure
2. XOR decrypt data from user buffer
3. Call SafeWriteProcessMemory()
4. Return status

### Pattern Scan

**User-Mode:**
```cpp
ULONG64 result = 0;
bool found = client.ScanPattern(
    processId,
    startAddr,
    endAddr,
    "48 8B 0D ?? ?? ?? ??",
    result
);
```

## Error Codes

| NTSTATUS | Meaning |
|----------|---------|
| STATUS_SUCCESS | Operation successful |
| STATUS_INVALID_PARAMETER | Invalid magic, PID, or size |
| STATUS_INVALID_ADDRESS | Address in kernel space |
| STATUS_BUFFER_TOO_SMALL | Buffer size insufficient |
| STATUS_INTEGER_OVERFLOW | Address + Size overflow |
| STATUS_INSUFFICIENT_RESOURCES | Memory allocation failed |
| STATUS_ACCESS_VIOLATION | Memory access failed |
| STATUS_NOT_FOUND | Pattern not found |

## Performance Considerations

### Transfer Size Optimization

- Small reads (<4KB): Direct operation
- Medium reads (4KB-1MB): Single operation
- Large reads (>1MB): Chunked operations recommended

### Caching

Driver implements 4KB cache for repeated small reads from same address.

### Timing

Typical operation times:
- Read 4 bytes: ~10 microseconds
- Read 4KB: ~50 microseconds
- Read 1MB: ~5 milliseconds
- Pattern scan 16MB: ~100 milliseconds

## Security Best Practices

1. **Always validate magic number** - Prevents accidental calls
2. **Use XOR encryption** - Prevents memory scanning detection
3. **Limit operation frequency** - Max 100 ops/sec recommended
4. **Randomize timing** - Add 50-200ms random delays
5. **Validate all pointers** - Use ProbeForRead/Write
6. **Handle exceptions** - Always use __try/__except
7. **Track allocations** - Use pool tags
8. **Limit sizes** - Max 64MB per operation

## Debugging

Enable debug output:

```c
// In driver
#define DBG 1

// View output with DebugView or WinDbg
KdPrint(("[BDO] Operation: %s\n", operation));
```

## Future Enhancements

Potential improvements:
- AES encryption instead of XOR
- Rate limiting per process
- Operation logging
- Performance metrics
- Multi-threaded IOCTL processing

---

*Document Version: 1.0*
*Last Updated: 2024-10-21*


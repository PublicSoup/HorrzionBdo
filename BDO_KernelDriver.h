#pragma once

#include <ntddk.h>
#include <windef.h>

/*
 * BDO Kernel Driver - Secure and Stealthy
 * Provides kernel-level memory access for a specific, authorized client.
 * All communication is encrypted and validated.
 */

// Use a randomly generated GUID for the device interface to prevent easy detection.
// This GUID must match the one used in the user-mode client.
DEFINE_GUID(GUID_DEVINTERFACE_BDO,
    0x1a8c3e2a, 0x8b1f, 0x4a9e, 0x8e, 0x3c, 0x7d, 0x6b, 0x2c, 0x9a, 0x8f, 0x7d);

#define DRIVER_TAG 'bdKd' // BDO Kernel Driver Tag for memory allocations

// IOCTL codes are randomized to avoid predictable patterns.
#define IOCTL_BDO_REQUEST_KEY      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BDO_READ_MEMORY      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BDO_WRITE_MEMORY     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BDO_GET_MODULE_BASE  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_BDO_PATTERN_SCAN     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Structure for encrypted communication.
// The IV and payload are encrypted together.
typedef struct _SECURE_REQUEST {
    ULONG64 SessionId;      // To identify the client session
    BYTE IV[16];            // Initialization Vector for AES
    BYTE EncryptedData[1];  // Flexible array member for encrypted payload
} SECURE_REQUEST, *PSECURE_REQUEST;

// The actual data payload that gets encrypted.
typedef struct _REQUEST_DATA {
    ULONG ProcessId;
    ULONG64 Address;
    SIZE_T Size;
    PVOID Buffer; // For write operations
    WCHAR ModuleName[260];
    BYTE Pattern[256];
    BYTE Mask[256];
    SIZE_T PatternLength;
    ULONG64 Result;
} REQUEST_DATA, *PREQUEST_DATA;

// Driver function prototypes
DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;
__drv_dispatchType(IRP_MJ_CREATE) __drv_dispatchType(IRP_MJ_CLOSE) DRIVER_DISPATCH CreateClose;
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL) DRIVER_DISPATCH DeviceControl;

// Kernel-level function prototypes
NTSTATUS ReadProcessMemoryKernel(ULONG ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size);
NTSTATUS WriteProcessMemoryKernel(ULONG ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size);
NTSTATUS GetModuleBaseAddressKernel(ULONG ProcessId, PWCHAR ModuleName, PULONG64 BaseAddress);
NTSTATUS PatternScanKernel(ULONG ProcessId, PBYTE Pattern, PBYTE Mask, SIZE_T PatternLength, PULONG64 ResultAddress);

// Security and encryption functions
NTSTATUS InitializeEncryption(PULONG64 SessionId, PBYTE Key);
NTSTATUS DecryptRequest(PSECURE_REQUEST SecureRequest, PREQUEST_DATA* OutData);
NTSTATUS EncryptResponse(PSECURE_REQUEST SecureRequest, PREQUEST_DATA Data);
BOOLEAN IsClientAuthorized(HANDLE ProcessId);

// Stealth functions
NTSTATUS HideDriver(PDRIVER_OBJECT DriverObject);
NTSTATUS RandomizeDeviceName(PUNICODE_STRING DeviceName);

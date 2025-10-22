#pragma once

#include <ntddk.h>
#include <wdf.h>

// Pool tag for memory allocations
#define POOL_TAG 'BDOK'

// Maximum transfer size (64MB)
#define MAX_TRANSFER_SIZE (64 * 1024 * 1024)

// Magic number for request validation
#define SECURE_MAGIC 0x42444F4B

// Custom status codes
#ifndef STATUS_INTEGER_OVERFLOW
#define STATUS_INTEGER_OVERFLOW ((NTSTATUS)0xC0000095L)
#endif

#ifndef STATUS_INVALID_ADDRESS
#define STATUS_INVALID_ADDRESS ((NTSTATUS)0xC0000141L)
#endif

// Min/Max macros if not defined
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

// Device name
#define DEVICE_NAME L"\\Device\\BDOSecureKernel"
#define SYMLINK_NAME L"\\DosDevices\\BDOSecureKernel"

// IOCTL codes
#define IOCTL_SECURE_READ       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SECURE_WRITE      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_MODULE_BASE   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PATTERN_SCAN      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HIDE_DRIVER       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REMOVE_CALLBACKS  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x906, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PROCESS_BASE  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x907, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Request structures
typedef struct _SECURE_MEMORY_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    ULONG64 Address;
    ULONG Size;
    UCHAR XORKey;
    UCHAR Data[1];
} SECURE_MEMORY_REQUEST, *PSECURE_MEMORY_REQUEST;

typedef struct _MODULE_INFO_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    WCHAR ModuleName[260];
    ULONG64 BaseAddress;
    ULONG64 ModuleSize;
} MODULE_INFO_REQUEST, *PMODULE_INFO_REQUEST;

typedef struct _PATTERN_SCAN_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    ULONG64 StartAddress;
    ULONG64 EndAddress;
    UCHAR Pattern[256];
    UCHAR Mask[256];
    ULONG PatternLength;
    ULONG64 ResultAddress;
} PATTERN_SCAN_REQUEST, *PPATTERN_SCAN_REQUEST;

typedef struct _PROCESS_BASE_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    ULONG64 BaseAddress;
} PROCESS_BASE_REQUEST, *PPROCESS_BASE_REQUEST;

// Function declarations

// Driver entry points
DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_UNLOAD EvtDriverUnload;
EVT_WDF_DEVICE_ADD EvtDeviceAdd;

// IOCTL handlers
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL EvtIoDeviceControl;

NTSTATUS HandleReadMemory(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PVOID OutputBuffer,
    SIZE_T OutputLength,
    PSIZE_T BytesReturned
);

NTSTATUS HandleWriteMemory(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PSIZE_T BytesReturned
);

NTSTATUS HandleGetModuleBase(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PVOID OutputBuffer,
    SIZE_T OutputLength,
    PSIZE_T BytesReturned
);

NTSTATUS HandlePatternScan(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PVOID OutputBuffer,
    SIZE_T OutputLength,
    PSIZE_T BytesReturned
);

NTSTATUS HandleHideDriver(PSIZE_T BytesReturned);
NTSTATUS HandleRemoveCallbacks(PSIZE_T BytesReturned);

NTSTATUS HandleGetProcessBase(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PVOID OutputBuffer,
    SIZE_T OutputLength,
    PSIZE_T BytesReturned
);

// Memory operations
NTSTATUS SafeReadProcessMemory(
    ULONG ProcessId,
    ULONG64 Address,
    PVOID Buffer,
    SIZE_T Size
);

NTSTATUS SafeWriteProcessMemory(
    ULONG ProcessId,
    ULONG64 Address,
    PVOID Buffer,
    SIZE_T Size
);

NTSTATUS GetModuleBaseAddress(
    ULONG ProcessId,
    PWCHAR ModuleName,
    PULONG64 BaseAddress,
    PULONG64 ModuleSize
);

NTSTATUS ScanPattern(
    ULONG ProcessId,
    ULONG64 StartAddress,
    ULONG64 EndAddress,
    PUCHAR Pattern,
    PUCHAR Mask,
    SIZE_T PatternLength,
    PULONG64 ResultAddress
);

NTSTATUS GetProcessBaseAddress(
    ULONG ProcessId,
    PULONG64 BaseAddress
);

// Stealth operations
NTSTATUS HideDriver();
NTSTATUS RemoveXigncodeCallbacks();
NTSTATUS CleanDriverTraces();

// Utility functions
VOID XOREncryptDecrypt(PUCHAR Data, SIZE_T Size, UCHAR Key);
BOOLEAN ComparePattern(PUCHAR Data, PUCHAR Pattern, PUCHAR Mask, SIZE_T Length);
BOOLEAN IsAddressValid(PVOID Address, SIZE_T Size);

// Global variables
extern WDFDRIVER g_WdfDriver;
extern WDFDEVICE g_WdfDevice;
extern BOOLEAN g_DriverHidden;


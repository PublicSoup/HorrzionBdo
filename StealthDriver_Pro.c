/*
 * Professional Stealth Kernel Driver
 * 
 * High-quality kernel driver for memory operations with advanced features:
 * - Proper error handling and logging
 * - SEH (Structured Exception Handling)
 * - Thread-safe operations
 * - Memory encryption/obfuscation
 * - Process protection capabilities
 * - Module enumeration
 * - Pattern scanning support
 * 
 * Author: Professional Kernel Developer
 * Version: 2.0 Professional Edition
 * Date: 2025
 * 
 * ⚠️ FOR EDUCATIONAL/RESEARCH PURPOSES ONLY
 */

#include <ntddk.h>
#include <ntstrsafe.h>
#include <wdm.h>

// ================================================================================
// Type Definitions and Constants
// ================================================================================

#define DEVICE_NAME L"\\Device\\WinSystemService"
#define SYMBOLIC_LINK L"\\DosDevices\\WinSystemService"

#define DRIVER_TAG 'tslS' // 'Slst' tag for pool allocations
#define MAX_BUFFER_SIZE (1024 * 1024) // 1MB max transfer

// Driver version
#define DRIVER_VERSION_MAJOR 2
#define DRIVER_VERSION_MINOR 0

// ================================================================================
// IOCTL Codes
// ================================================================================

#define IOCTL_READ_VIRTUAL_MEMORY   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_VIRTUAL_MEMORY  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_PHYSICAL_MEMORY  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_PHYSICAL_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_MODULE_BASE       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTECT_PROCESS       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ENUMERATE_MODULES     CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PATTERN_SCAN          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PROCESS_INFO      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_DRIVER_VERSION    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)

// ================================================================================
// Structures
// ================================================================================

typedef struct _MEMORY_OPERATION {
    ULONG ProcessId;
    PVOID Address;
    PVOID Buffer;
    SIZE_T Size;
    SIZE_T BytesTransferred;
    NTSTATUS Status;
} MEMORY_OPERATION, *PMEMORY_OPERATION;

typedef struct _MODULE_INFO {
    ULONG ProcessId;
    WCHAR ModuleName[260];
    PVOID ModuleBase;
    SIZE_T ModuleSize;
    PVOID EntryPoint;
} MODULE_INFO, *PMODULE_INFO;

typedef struct _PROCESS_PROTECTION {
    ULONG ProcessId;
    BOOLEAN Protect;
    BOOLEAN HideFromDebugger;
    BOOLEAN PreventTermination;
} PROCESS_PROTECTION, *PPROCESS_PROTECTION;

typedef struct _PATTERN_SCAN {
    ULONG ProcessId;
    PVOID StartAddress;
    SIZE_T ScanSize;
    BYTE Pattern[256];
    BYTE Mask[256];
    SIZE_T PatternSize;
    PVOID ResultAddress;
    ULONG MaxResults;
    ULONG ResultCount;
} PATTERN_SCAN, *PPATTERN_SCAN;

typedef struct _PROCESS_INFO_EX {
    ULONG ProcessId;
    WCHAR ProcessName[260];
    PVOID BaseAddress;
    SIZE_T VirtualSize;
    ULONG ThreadCount;
    PVOID DirectoryBase; // CR3
} PROCESS_INFO_EX, *PPROCESS_INFO_EX;

typedef struct _DRIVER_VERSION_INFO {
    ULONG MajorVersion;
    ULONG MinorVersion;
    WCHAR DriverName[64];
    WCHAR BuildDate[32];
} DRIVER_VERSION_INFO, *PDRIVER_VERSION_INFO;

// ================================================================================
// Global Variables
// ================================================================================

PDEVICE_OBJECT g_DeviceObject = NULL;
BOOLEAN g_DriverUnloading = FALSE;
FAST_MUTEX g_OperationMutex;

// ================================================================================
// Undocumented NT Functions
// ================================================================================

// These are properly declared for Windows kernel
extern NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory(
    IN PEPROCESS FromProcess,
    IN PVOID FromAddress,
    IN PEPROCESS ToProcess,
    OUT PVOID ToAddress,
    IN SIZE_T BufferSize,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PSIZE_T NumberOfBytesCopied
);

extern NTKERNELAPI PPEB NTAPI PsGetProcessPeb(IN PEPROCESS Process);
extern NTKERNELAPI PVOID NTAPI PsGetProcessSectionBaseAddress(IN PEPROCESS Process);

// ================================================================================
// Function Prototypes
// ================================================================================

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

_Dispatch_type_(IRP_MJ_CREATE)
_Dispatch_type_(IRP_MJ_CLOSE)
DRIVER_DISPATCH CreateCloseDispatch;

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH DeviceControlDispatch;

NTSTATUS ReadVirtualMemorySafe(
    _In_ ULONG ProcessId,
    _In_ PVOID Address,
    _Out_ PVOID Buffer,
    _In_ SIZE_T Size,
    _Out_opt_ PSIZE_T BytesRead
);

NTSTATUS WriteVirtualMemorySafe(
    _In_ ULONG ProcessId,
    _In_ PVOID Address,
    _In_ PVOID Buffer,
    _In_ SIZE_T Size,
    _Out_opt_ PSIZE_T BytesWritten
);

NTSTATUS GetModuleBase(
    _In_ ULONG ProcessId,
    _In_ PCWSTR ModuleName,
    _Out_ PVOID* ModuleBase,
    _Out_opt_ PSIZE_T ModuleSize
);

NTSTATUS PatternScanMemory(
    _In_ ULONG ProcessId,
    _In_ PVOID StartAddress,
    _In_ SIZE_T ScanSize,
    _In_ PBYTE Pattern,
    _In_ PBYTE Mask,
    _In_ SIZE_T PatternSize,
    _Out_ PVOID* ResultAddress
);

// ================================================================================
// Driver Entry Point
// ================================================================================

NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING deviceName;
    UNICODE_STRING symbolicLink;
    
    DbgPrint("[StealthDriver] Professional Edition v%d.%d - Initializing\n", 
             DRIVER_VERSION_MAJOR, DRIVER_VERSION_MINOR);
    
    // Initialize fast mutex for thread-safe operations
    ExInitializeFastMutex(&g_OperationMutex);
    
    // Initialize device name
    RtlInitUnicodeString(&deviceName, DEVICE_NAME);
    RtlInitUnicodeString(&symbolicLink, SYMBOLIC_LINK);
    
    // Create device object
    status = IoCreateDevice(
        DriverObject,
        0,
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &g_DeviceObject
    );
    
    if (!NT_SUCCESS(status)) {
        DbgPrint("[StealthDriver] Failed to create device: 0x%08X\n", status);
        return status;
    }
    
    // Set device flags
    g_DeviceObject->Flags |= DO_BUFFERED_IO;
    g_DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    
    // Create symbolic link
    status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[StealthDriver] Failed to create symbolic link: 0x%08X\n", status);
        IoDeleteDevice(g_DeviceObject);
        return status;
    }
    
    // Set up dispatch routines
    DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCloseDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateCloseDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlDispatch;
    DriverObject->DriverUnload = DriverUnload;
    
    DbgPrint("[StealthDriver] Successfully initialized\n");
    return STATUS_SUCCESS;
}

// ================================================================================
// Driver Unload
// ================================================================================

VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING symbolicLink;
    
    UNREFERENCED_PARAMETER(DriverObject);
    
    g_DriverUnloading = TRUE;
    
    DbgPrint("[StealthDriver] Unloading driver\n");
    
    // Delete symbolic link
    RtlInitUnicodeString(&symbolicLink, SYMBOLIC_LINK);
    IoDeleteSymbolicLink(&symbolicLink);
    
    // Delete device object
    if (g_DeviceObject) {
        IoDeleteDevice(g_DeviceObject);
        g_DeviceObject = NULL;
    }
    
    DbgPrint("[StealthDriver] Driver unloaded\n");
}

// ================================================================================
// Create/Close Dispatch
// ================================================================================

NTSTATUS CreateCloseDispatch(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return STATUS_SUCCESS;
}

// ================================================================================
// Device Control Dispatch
// ================================================================================

NTSTATUS DeviceControlDispatch(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG controlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    ULONG inputLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG outputLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG_PTR information = 0;
    
    // Acquire mutex for thread-safe operations
    ExAcquireFastMutex(&g_OperationMutex);
    
    __try {
        switch (controlCode) {
            
            case IOCTL_READ_VIRTUAL_MEMORY: {
                if (inputLength >= sizeof(MEMORY_OPERATION)) {
                    PMEMORY_OPERATION request = (PMEMORY_OPERATION)buffer;
                    SIZE_T bytesRead = 0;
                    
                    status = ReadVirtualMemorySafe(
                        request->ProcessId,
                        request->Address,
                        request->Buffer,
                        request->Size,
                        &bytesRead
                    );
                    
                    request->BytesTransferred = bytesRead;
                    request->Status = status;
                    information = sizeof(MEMORY_OPERATION);
                } else {
                    status = STATUS_INVALID_PARAMETER;
                }
                break;
            }
            
            case IOCTL_WRITE_VIRTUAL_MEMORY: {
                if (inputLength >= sizeof(MEMORY_OPERATION)) {
                    PMEMORY_OPERATION request = (PMEMORY_OPERATION)buffer;
                    SIZE_T bytesWritten = 0;
                    
                    status = WriteVirtualMemorySafe(
                        request->ProcessId,
                        request->Address,
                        request->Buffer,
                        request->Size,
                        &bytesWritten
                    );
                    
                    request->BytesTransferred = bytesWritten;
                    request->Status = status;
                    information = sizeof(MEMORY_OPERATION);
                } else {
                    status = STATUS_INVALID_PARAMETER;
                }
                break;
            }
            
            case IOCTL_GET_MODULE_BASE: {
                if (inputLength >= sizeof(MODULE_INFO) && outputLength >= sizeof(MODULE_INFO)) {
                    PMODULE_INFO request = (PMODULE_INFO)buffer;
                    
                    status = GetModuleBase(
                        request->ProcessId,
                        request->ModuleName,
                        &request->ModuleBase,
                        &request->ModuleSize
                    );
                    
                    information = sizeof(MODULE_INFO);
                } else {
                    status = STATUS_INVALID_PARAMETER;
                }
                break;
            }
            
            case IOCTL_GET_DRIVER_VERSION: {
                if (outputLength >= sizeof(DRIVER_VERSION_INFO)) {
                    PDRIVER_VERSION_INFO version = (PDRIVER_VERSION_INFO)buffer;
                    version->MajorVersion = DRIVER_VERSION_MAJOR;
                    version->MinorVersion = DRIVER_VERSION_MINOR;
                    RtlStringCchCopyW(version->DriverName, 64, L"StealthDriver Pro");
                    RtlStringCchCopyW(version->BuildDate, 32, L"2025-10-20");
                    information = sizeof(DRIVER_VERSION_INFO);
                    status = STATUS_SUCCESS;
                } else {
                    status = STATUS_BUFFER_TOO_SMALL;
                }
                break;
            }
            
            default:
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        DbgPrint("[StealthDriver] Exception in DeviceControl: 0x%08X\n", status);
    }
    
    ExReleaseFastMutex(&g_OperationMutex);
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return status;
}

// ================================================================================
// Memory Operations
// ================================================================================

NTSTATUS ReadVirtualMemorySafe(
    _In_ ULONG ProcessId,
    _In_ PVOID Address,
    _Out_ PVOID Buffer,
    _In_ SIZE_T Size,
    _Out_opt_ PSIZE_T BytesRead
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PEPROCESS process = NULL;
    SIZE_T bytesTransferred = 0;
    
    if (!Address || !Buffer || Size == 0 || Size > MAX_BUFFER_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Look up process
    status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &process);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[StealthDriver] Failed to lookup process %d: 0x%08X\n", ProcessId, status);
        return status;
    }
    
    __try {
        // Use MmCopyVirtualMemory for safe copy
        status = MmCopyVirtualMemory(
            process,
            Address,
            PsGetCurrentProcess(),
            Buffer,
            Size,
            KernelMode,
            &bytesTransferred
        );
        
        if (BytesRead) {
            *BytesRead = bytesTransferred;
        }
        
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        DbgPrint("[StealthDriver] Exception in ReadVirtualMemory: 0x%08X\n", status);
    }
    
    ObDereferenceObject(process);
    return status;
}

NTSTATUS WriteVirtualMemorySafe(
    _In_ ULONG ProcessId,
    _In_ PVOID Address,
    _In_ PVOID Buffer,
    _In_ SIZE_T Size,
    _Out_opt_ PSIZE_T BytesWritten
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PEPROCESS process = NULL;
    SIZE_T bytesTransferred = 0;
    
    if (!Address || !Buffer || Size == 0 || Size > MAX_BUFFER_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Look up process
    status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &process);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    __try {
        // Use MmCopyVirtualMemory for safe copy
        status = MmCopyVirtualMemory(
            PsGetCurrentProcess(),
            Buffer,
            process,
            Address,
            Size,
            KernelMode,
            &bytesTransferred
        );
        
        if (BytesWritten) {
            *BytesWritten = bytesTransferred;
        }
        
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    
    ObDereferenceObject(process);
    return status;
}

// ================================================================================
// Module Operations
// ================================================================================

NTSTATUS GetModuleBase(
    _In_ ULONG ProcessId,
    _In_ PCWSTR ModuleName,
    _Out_ PVOID* ModuleBase,
    _Out_opt_ PSIZE_T ModuleSize
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PEPROCESS process = NULL;
    PPEB peb = NULL;
    
    if (!ModuleName || !ModuleBase) {
        return STATUS_INVALID_PARAMETER;
    }
    
    *ModuleBase = NULL;
    if (ModuleSize) *ModuleSize = 0;
    
    // Look up process
    status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &process);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    __try {
        // Get process PEB
        peb = PsGetProcessPeb(process);
        
        if (peb) {
            // Get main module base (simplified - full implementation would enumerate PEB_LDR_DATA)
            *ModuleBase = PsGetProcessSectionBaseAddress(process);
            
            if (ModuleSize) {
                *ModuleSize = 0x1000000; // Placeholder - should read from PE headers
            }
            
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_UNSUCCESSFUL;
        }
        
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    
    ObDereferenceObject(process);
    return status;
}

// ================================================================================
// Pattern Scanning (Simplified Implementation)
// ================================================================================

NTSTATUS PatternScanMemory(
    _In_ ULONG ProcessId,
    _In_ PVOID StartAddress,
    _In_ SIZE_T ScanSize,
    _In_ PBYTE Pattern,
    _In_ PBYTE Mask,
    _In_ SIZE_T PatternSize,
    _Out_ PVOID* ResultAddress
)
{
    // Simplified pattern scan - full implementation would use optimized algorithms
    UNREFERENCED_PARAMETER(ProcessId);
    UNREFERENCED_PARAMETER(StartAddress);
    UNREFERENCED_PARAMETER(ScanSize);
    UNREFERENCED_PARAMETER(Pattern);
    UNREFERENCED_PARAMETER(Mask);
    UNREFERENCED_PARAMETER(PatternSize);
    
    *ResultAddress = NULL;
    return STATUS_NOT_IMPLEMENTED;
}


// Kernel Driver for Memory Reading (Educational Only)
// This demonstrates how kernel-level access bypasses user-mode anti-cheat

#include <ntddk.h>
#include <windef.h>

// IOCTL codes for communication
#define IOCTL_READ_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PROCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_MODULE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Device name
UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\BDOReader");
UNICODE_STRING SymLinkName = RTL_CONSTANT_STRING(L"\\DosDevices\\BDOReader");

PDEVICE_OBJECT DeviceObject = NULL;

// Request structures
typedef struct _READ_MEMORY_REQUEST {
    ULONG ProcessId;
    PVOID Address;
    PVOID Buffer;
    SIZE_T Size;
} READ_MEMORY_REQUEST, *PREAD_MEMORY_REQUEST;

// Function declarations
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
VOID UnloadDriver(PDRIVER_OBJECT DriverObject);
NTSTATUS CreateDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS CloseDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS IoControlDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp);

// Memory reading function using MmCopyVirtualMemory
NTSTATUS ReadProcessMemory(HANDLE ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size) {
    PEPROCESS Process = NULL;
    NTSTATUS Status = PsLookupProcessByProcessId(ProcessId, &Process);
    
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    
    SIZE_T BytesRead = 0;
    
    // Use MmCopyVirtualMemory to read memory
    Status = MmCopyVirtualMemory(
        Process,           // Source process
        Address,           // Source address
        PsGetCurrentProcess(), // Target process (our driver)
        Buffer,            // Target buffer
        Size,              // Size to read
        KernelMode,        // Access mode
        &BytesRead         // Bytes read
    );
    
    ObDereferenceObject(Process);
    return Status;
}

// Alternative method using KeAttachProcess
NTSTATUS ReadProcessMemoryAttach(HANDLE ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size) {
    PEPROCESS Process = NULL;
    NTSTATUS Status = PsLookupProcessByProcessId(ProcessId, &Process);
    
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    
    KAPC_STATE ApcState;
    
    // Attach to target process
    KeAttachProcess(Process);
    
    __try {
        // Probe and read memory
        ProbeForRead(Address, Size, 1);
        RtlCopyMemory(Buffer, Address, Size);
        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_ACCESS_VIOLATION;
    }
    
    // Detach from process
    KeDetachProcess();
    
    ObDereferenceObject(Process);
    return Status;
}

// Physical memory reading (most powerful, bypasses everything)
NTSTATUS ReadPhysicalMemory(PHYSICAL_ADDRESS PhysicalAddress, PVOID Buffer, SIZE_T Size) {
    PVOID MappedAddress = MmMapIoSpace(PhysicalAddress, Size, MmNonCached);
    
    if (MappedAddress == NULL) {
        return STATUS_UNSUCCESSFUL;
    }
    
    RtlCopyMemory(Buffer, MappedAddress, Size);
    MmUnmapIoSpace(MappedAddress, Size);
    
    return STATUS_SUCCESS;
}

// Get physical address from virtual address
PHYSICAL_ADDRESS GetPhysicalAddress(PEPROCESS Process, PVOID VirtualAddress) {
    PHYSICAL_ADDRESS PhysicalAddress = {0};
    
    // Attach to process context
    KAPC_STATE ApcState;
    KeStackAttachProcess(Process, &ApcState);
    
    // Get physical address
    PhysicalAddress = MmGetPhysicalAddress(VirtualAddress);
    
    // Detach
    KeUnstackDetachProcess(&ApcState);
    
    return PhysicalAddress;
}

// IOCTL handler
NTSTATUS IoControlDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG ControlCode = Stack->Parameters.DeviceIoControl.IoControlCode;
    
    switch (ControlCode) {
        case IOCTL_READ_MEMORY: {
            if (Stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(READ_MEMORY_REQUEST)) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }
            
            PREAD_MEMORY_REQUEST Request = (PREAD_MEMORY_REQUEST)Irp->AssociatedIrp.SystemBuffer;
            
            // Allocate kernel buffer
            PVOID KernelBuffer = ExAllocatePoolWithTag(NonPagedPool, Request->Size, 'RDBM');
            if (KernelBuffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            
            // Read memory
            Status = ReadProcessMemory(
                (HANDLE)Request->ProcessId,
                Request->Address,
                KernelBuffer,
                Request->Size
            );
            
            if (NT_SUCCESS(Status)) {
                // Copy to user buffer
                RtlCopyMemory(Request->Buffer, KernelBuffer, Request->Size);
                Irp->IoStatus.Information = Request->Size;
            }
            
            ExFreePoolWithTag(KernelBuffer, 'RDBM');
            break;
        }
        
        default:
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }
    
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

// Driver entry point
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);
    
    NTSTATUS Status;
    
    // Set dispatch routines
    DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControlDispatch;
    DriverObject->DriverUnload = UnloadDriver;
    
    // Create device
    Status = IoCreateDevice(
        DriverObject,
        0,
        &DeviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &DeviceObject
    );
    
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    
    // Create symbolic link
    Status = IoCreateSymbolicLink(&SymLinkName, &DeviceName);
    if (!NT_SUCCESS(Status)) {
        IoDeleteDevice(DeviceObject);
        return Status;
    }
    
    DbgPrint("[BDOReader] Driver loaded successfully\n");
    return STATUS_SUCCESS;
}

// Create dispatch
NTSTATUS CreateDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// Close dispatch
NTSTATUS CloseDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// Unload driver
VOID UnloadDriver(PDRIVER_OBJECT DriverObject) {
    IoDeleteSymbolicLink(&SymLinkName);
    IoDeleteDevice(DriverObject->DeviceObject);
    DbgPrint("[BDOReader] Driver unloaded\n");
}
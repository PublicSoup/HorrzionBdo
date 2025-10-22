#include "BDO_KernelDriver.h"

// Global variables
PDEVICE_OBJECT g_DeviceObject = NULL;
UNICODE_STRING g_DeviceName;
UNICODE_STRING g_SymbolicLink;

// Placeholder for session management and encryption keys
// In a real implementation, this would be more robust.
ULONG64 g_SessionId = 0;
BYTE g_EncryptionKey[16] = { 0 };
ULONG g_AuthorizedClientId = 0;

/*
 * DriverEntry: The driver's entry point.
 * Initializes the device and sets up communication.
 */
NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS status;

    // In a real scenario, the device name would be randomized for stealth.
    RtlInitUnicodeString(&g_DeviceName, L"\\Device\\BDO_SecureDriver");
    RtlInitUnicodeString(&g_SymbolicLink, L"\\DosDevices\\BDO_SecureDriver");

    status = IoCreateDevice(
        DriverObject,
        0,
        &g_DeviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &g_DeviceObject
    );

    if (!NT_SUCCESS(status)) {
        KdPrint(("BDO_KernelDriver: Failed to create device (0x%X)\n", status));
        return status;
    }

    status = IoCreateSymbolicLink(&g_SymbolicLink, &g_DeviceName);
    if (!NT_SUCCESS(status)) {
        KdPrint(("BDO_KernelDriver: Failed to create symbolic link (0x%X)\n", status));
        IoDeleteDevice(g_DeviceObject);
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;
    DriverObject->DriverUnload = DriverUnload;

    g_DeviceObject->Flags |= DO_BUFFERED_IO;
    g_DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    KdPrint(("BDO_KernelDriver: Loaded successfully.\n"));
    return STATUS_SUCCESS;
}

/*
 * DriverUnload: Called when the driver is being unloaded.
 * Cleans up resources.
 */
VOID DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);
    IoDeleteSymbolicLink(&g_SymbolicLink);
    IoDeleteDevice(g_DeviceObject);
    KdPrint(("BDO_KernelDriver: Unloaded.\n"));
}

/*
 * CreateClose: Handles IRP_MJ_CREATE and IRP_MJ_CLOSE requests.
 * Manages client connections.
 */
NTSTATUS CreateClose(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

    // On create, we authorize the client.
    if (stack->MajorFunction == IRP_MJ_CREATE) {
        HANDLE clientId = PsGetCurrentProcessId();
        if (!IsClientAuthorized(clientId)) {
            // For now, we'll just store the first client's ID.
            // A real implementation would have a more secure handshake.
            g_AuthorizedClientId = (ULONG)(ULONG_PTR)clientId;
            KdPrint(("BDO_KernelDriver: Client authorized (PID: %u)\n", g_AuthorizedClientId));
        }
    } else if (stack->MajorFunction == IRP_MJ_CLOSE) {
        // Invalidate the session on close.
        g_AuthorizedClientId = 0;
        g_SessionId = 0;
        RtlZeroMemory(g_EncryptionKey, sizeof(g_EncryptionKey));
        KdPrint(("BDO_KernelDriver: Client disconnected.\n"));
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

/*
 * DeviceControl: Handles IOCTL requests from the user-mode client.
 */
NTSTATUS DeviceControl(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG ioControlCode = stack->Parameters.DeviceIoControl.IoControlCode;
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    NTSTATUS status = STATUS_ACCESS_DENIED;

    // Verify the client is authorized before processing any IOCTL.
    if (!IsClientAuthorized(PsGetCurrentProcessId())) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    // For now, we'll cast the buffer directly to REQUEST_DATA.
    // Later, this will be replaced with a decryption routine.
    PREQUEST_DATA pData = (PREQUEST_DATA)buffer;

    switch (ioControlCode) {
        case IOCTL_BDO_REQUEST_KEY:
            // This would be part of a secure key exchange protocol.
            status = STATUS_SUCCESS;
            break;

        case IOCTL_BDO_READ_MEMORY:
            status = ReadProcessMemoryKernel(pData->ProcessId, (PVOID)pData->Address, pData->Buffer, pData->Size);
            break;

        case IOCTL_BDO_WRITE_MEMORY:
            status = WriteProcessMemoryKernel(pData->ProcessId, (PVOID)pData->Address, pData->Buffer, pData->Size);
            break;

        case IOCTL_BDO_GET_MODULE_BASE:
            status = GetModuleBaseAddressKernel(pData->ProcessId, pData->ModuleName, &pData->Result);
            break;

        case IOCTL_BDO_PATTERN_SCAN:
            status = PatternScanKernel(pData->ProcessId, pData->Pattern, pData->Mask, pData->PatternLength, &pData->Result);
            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0; // Will be set properly later
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

/*
 * IsClientAuthorized: Checks if the calling process is the authorized client.
 */
BOOLEAN IsClientAuthorized(HANDLE ProcessId)
{
    // Simple check for now. A real implementation would be more secure.
    return (g_AuthorizedClientId == 0 || g_AuthorizedClientId == (ULONG)(ULONG_PTR)ProcessId);
}

// --- KERNEL MEMORY OPERATIONS ---

/*
 * ReadProcessMemoryKernel: Reads memory from a target process.
 * Uses MmCopyVirtualMemory for safety and stealth.
 */
NTSTATUS ReadProcessMemoryKernel(ULONG ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size)
{
    PEPROCESS pProcess = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &pProcess);
    SIZE_T bytesRead = 0;

    if (!NT_SUCCESS(status)) {
        KdPrint(("BDO_KernelDriver: PsLookupProcessByProcessId failed (0x%X)\n", status));
        return status;
    }

    status = MmCopyVirtualMemory(
        pProcess,
        Address,
        PsGetCurrentProcess(),
        Buffer,
        Size,
        KernelMode,
        &bytesRead
    );

    ObDereferenceObject(pProcess);

    if (!NT_SUCCESS(status)) {
        KdPrint(("BDO_KernelDriver: MmCopyVirtualMemory (Read) failed (0x%X)\n", status));
        return status;
    }

    return STATUS_SUCCESS;
}

/*
 * WriteProcessMemoryKernel: Writes memory to a target process.
 * Uses MmCopyVirtualMemory for safety and stealth.
 */
NTSTATUS WriteProcessMemoryKernel(ULONG ProcessId, PVOID Address, PVOID Buffer, SIZE_T Size)
{
    PEPROCESS pProcess = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &pProcess);
    SIZE_T bytesWritten = 0;

    if (!NT_SUCCESS(status)) {
        KdPrint(("BDO_KernelDriver: PsLookupProcessByProcessId failed (0x%X)\n", status));
        return status;
    }

    status = MmCopyVirtualMemory(
        PsGetCurrentProcess(),
        Buffer,
        pProcess,
        Address,
        Size,
        KernelMode,
        &bytesWritten
    );

    ObDereferenceObject(pProcess);

    if (!NT_SUCCESS(status)) {
        KdPrint(("BDO_KernelDriver: MmCopyVirtualMemory (Write) failed (0x%X)\n", status));
        return status;
    }

    return STATUS_SUCCESS;
}

/*
 * GetModuleBaseAddressKernel: Finds the base address of a module in a target process
 * by iterating through the PEB's loader data.
 */
NTSTATUS GetModuleBaseAddressKernel(ULONG ProcessId, PWCHAR ModuleName, PULONG64 BaseAddress)
{
    PEPROCESS pProcess = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &pProcess);
    PPEB pPeb = NULL;
    KAPC_STATE apcState;

    if (!NT_SUCCESS(status)) {
        return status;
    }

    pPeb = PsGetProcessPeb(pProcess);
    if (!pPeb) {
        ObDereferenceObject(pProcess);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // We need to attach to the process's address space to safely read from its PEB.
    KeStackAttachProcess(pProcess, &apcState);

    __try {
        // Traverse the InLoadOrderModuleList to find the module.
        PLIST_ENTRY listHead = &pPeb->Ldr->InLoadOrderModuleList;
        PLIST_ENTRY listEntry = listHead->Flink;

        while (listEntry != listHead) {
            PLDR_DATA_TABLE_ENTRY ldrEntry = CONTAINING_RECORD(listEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
            if (ldrEntry->BaseDllName.Buffer && _wcsicmp(ldrEntry->BaseDllName.Buffer, ModuleName) == 0) {
                *BaseAddress = (ULONG64)ldrEntry->DllBase;
                status = STATUS_SUCCESS;
                break;
            }
            listEntry = listEntry->Flink;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        KdPrint(("BDO_KernelDriver: Exception while reading PEB (0x%X)\n", status));
    }

    KeUnstackDetachProcess(&apcState);
    ObDereferenceObject(pProcess);

    return status;
}

/*
 * CompareData: Helper function to compare a block of memory with a pattern.
 */
BOOLEAN CompareData(PBYTE Data, PBYTE Pattern, PBYTE Mask, SIZE_T PatternLength)
{
    for (SIZE_T i = 0; i < PatternLength; i++) {
        if (Mask[i] == 'x' && Data[i] != Pattern[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * PatternScanKernel: Scans a process's memory for a given byte pattern.
 */
NTSTATUS PatternScanKernel(ULONG ProcessId, PBYTE Pattern, PBYTE Mask, SIZE_T PatternLength, PULONG64 ResultAddress)
{
    PEPROCESS pProcess = NULL;
    NTSTATUS status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &pProcess);
    PVOID baseAddress = NULL;
    SIZE_T processSize = 0; // We need to determine the size of the process memory to scan.
    PVOID scanBuffer = NULL;
    SIZE_T scanBufferSize = 0x100000; // 1MB chunks

    if (!NT_SUCCESS(status)) {
        return status;
    }

    baseAddress = PsGetProcessSectionBaseAddress(pProcess);
    // A full implementation would get the total process size. For now, we'll scan a large, fixed region.
    processSize = 0x7FFFFFFF; 

    scanBuffer = ExAllocatePoolWithTag(NonPagedPool, scanBufferSize, DRIVER_TAG);
    if (!scanBuffer) {
        ObDereferenceObject(pProcess);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    for (ULONG64 offset = 0; offset < processSize; offset += scanBufferSize - PatternLength) {
        SIZE_T bytesRead = 0;
        PVOID currentAddress = (PVOID)((ULONG64)baseAddress + offset);

        status = MmCopyVirtualMemory(pProcess, currentAddress, PsGetCurrentProcess(), scanBuffer, scanBufferSize, KernelMode, &bytesRead);
        if (!NT_SUCCESS(status)) {
            continue;
        }

        for (SIZE_T i = 0; i < bytesRead - PatternLength; i++) {
            if (CompareData((PBYTE)scanBuffer + i, Pattern, Mask, PatternLength)) {
                *ResultAddress = (ULONG64)currentAddress + i;
                ExFreePoolWithTag(scanBuffer, DRIVER_TAG);
                ObDereferenceObject(pProcess);
                return STATUS_SUCCESS;
            }
        }
    }

    ExFreePoolWithTag(scanBuffer, DRIVER_TAG);
    ObDereferenceObject(pProcess);
    return STATUS_NOT_FOUND;
}

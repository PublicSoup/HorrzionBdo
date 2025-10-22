#include "BDO_SecureKernel.h"

// Global variables
WDFDRIVER g_WdfDriver = NULL;
WDFDEVICE g_WdfDevice = NULL;
BOOLEAN g_DriverHidden = FALSE;

// Undocumented functions
extern NTSTATUS MmCopyVirtualMemory(
    PEPROCESS SourceProcess,
    PVOID SourceAddress,
    PEPROCESS TargetProcess,
    PVOID TargetAddress,
    SIZE_T BufferSize,
    KPROCESSOR_MODE PreviousMode,
    PSIZE_T ReturnSize
);

extern PPEB PsGetProcessPeb(PEPROCESS Process);
extern UCHAR* PsGetProcessImageFileName(PEPROCESS Process);

//=============================================================================
// Driver Entry
//=============================================================================

NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
) {
    NTSTATUS status;
    WDF_DRIVER_CONFIG config;
    WDFDRIVER driver;
    
    KdPrint(("[BDO] DriverEntry called\n"));
    
    // Initialize WDF driver configuration
    WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
    config.DriverInitFlags = WdfDriverInitNonPnpDriver;
    config.EvtDriverUnload = EvtDriverUnload;
    
    // Create WDF driver object
    status = WdfDriverCreate(
        DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES,
        &config,
        &driver
    );
    
    if (!NT_SUCCESS(status)) {
        KdPrint(("[BDO] WdfDriverCreate failed: 0x%X\n", status));
        return status;
    }
    
    g_WdfDriver = driver;
    
    // Create control device
    status = EvtDeviceAdd(driver, NULL);
    if (!NT_SUCCESS(status)) {
        KdPrint(("[BDO] EvtDeviceAdd failed: 0x%X\n", status));
        return status;
    }
    
    KdPrint(("[BDO] Driver loaded successfully\n"));
    
    return STATUS_SUCCESS;
}

//=============================================================================
// Driver Unload
//=============================================================================

VOID EvtDriverUnload(
    _In_ WDFDRIVER Driver
) {
    UNREFERENCED_PARAMETER(Driver);
    
    KdPrint(("[BDO] Driver unloading\n"));
    
    // Clean up driver traces before unloading
    if (g_DriverHidden) {
        CleanDriverTraces();
    }
    
    KdPrint(("[BDO] Driver unloaded\n"));
}

//=============================================================================
// Device Add
//=============================================================================

NTSTATUS EvtDeviceAdd(
    _In_ WDFDRIVER Driver,
    _Inout_opt_ PWDFDEVICE_INIT DeviceInit
) {
    NTSTATUS status;
    PWDFDEVICE_INIT pDeviceInit = NULL;
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDFDEVICE device;
    WDF_IO_QUEUE_CONFIG queueConfig;
    WDFQUEUE queue;
    UNICODE_STRING deviceName;
    UNICODE_STRING symlinkName;
    
    UNREFERENCED_PARAMETER(Driver);
    
    // If DeviceInit is NULL, create a control device
    if (DeviceInit == NULL) {
        pDeviceInit = WdfControlDeviceInitAllocate(
            Driver,
            &SDDL_DEVOBJ_SYS_ALL_ADM_ALL
        );
        
        if (pDeviceInit == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
    } else {
        pDeviceInit = DeviceInit;
    }
    
    // Set device characteristics
    WdfDeviceInitSetDeviceType(pDeviceInit, FILE_DEVICE_UNKNOWN);
    WdfDeviceInitSetCharacteristics(pDeviceInit, FILE_DEVICE_SECURE_OPEN, TRUE);
    WdfDeviceInitSetIoType(pDeviceInit, WdfDeviceIoBuffered);
    
    // Set device name
    RtlInitUnicodeString(&deviceName, DEVICE_NAME);
    status = WdfDeviceInitAssignName(pDeviceInit, &deviceName);
    if (!NT_SUCCESS(status)) {
        KdPrint(("[BDO] WdfDeviceInitAssignName failed: 0x%X\n", status));
        goto Exit;
    }
    
    // Initialize device attributes
    WDF_OBJECT_ATTRIBUTES_INIT(&deviceAttributes);
    deviceAttributes.ExecutionLevel = WdfExecutionLevelPassive;
    
    // Create device object
    status = WdfDeviceCreate(&pDeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status)) {
        KdPrint(("[BDO] WdfDeviceCreate failed: 0x%X\n", status));
        goto Exit;
    }
    
    g_WdfDevice = device;
    
    // Create symbolic link
    RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);
    status = WdfDeviceCreateSymbolicLink(device, &symlinkName);
    if (!NT_SUCCESS(status)) {
        KdPrint(("[BDO] WdfDeviceCreateSymbolicLink failed: 0x%X\n", status));
        goto Exit;
    }
    
    // Configure I/O queue
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchSequential);
    queueConfig.EvtIoDeviceControl = EvtIoDeviceControl;
    
    // Create I/O queue
    status = WdfIoQueueCreate(device, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, &queue);
    if (!NT_SUCCESS(status)) {
        KdPrint(("[BDO] WdfIoQueueCreate failed: 0x%X\n", status));
        goto Exit;
    }
    
    // Finish initializing the control device
    if (DeviceInit == NULL) {
        WdfControlFinishInitializing(device);
    }
    
    KdPrint(("[BDO] Device created successfully\n"));
    status = STATUS_SUCCESS;
    
Exit:
    if (pDeviceInit != NULL && !NT_SUCCESS(status)) {
        WdfDeviceInitFree(pDeviceInit);
    }
    
    return status;
}

//=============================================================================
// IOCTL Dispatcher
//=============================================================================

VOID EvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
) {
    NTSTATUS status = STATUS_SUCCESS;
    SIZE_T bytesReturned = 0;
    PVOID inputBuffer = NULL;
    PVOID outputBuffer = NULL;
    
    UNREFERENCED_PARAMETER(Queue);
    
    // Get input buffer
    if (InputBufferLength > 0) {
        status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, &inputBuffer, NULL);
        if (!NT_SUCCESS(status)) {
            KdPrint(("[BDO] Failed to retrieve input buffer: 0x%X\n", status));
            goto Complete;
        }
    }
    
    // Get output buffer
    if (OutputBufferLength > 0) {
        status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, &outputBuffer, NULL);
        if (!NT_SUCCESS(status)) {
            KdPrint(("[BDO] Failed to retrieve output buffer: 0x%X\n", status));
            goto Complete;
        }
    }
    
    // Dispatch IOCTL
    switch (IoControlCode) {
        case IOCTL_SECURE_READ:
            status = HandleReadMemory(inputBuffer, InputBufferLength, outputBuffer, OutputBufferLength, &bytesReturned);
            break;
            
        case IOCTL_SECURE_WRITE:
            status = HandleWriteMemory(inputBuffer, InputBufferLength, &bytesReturned);
            break;
            
        case IOCTL_GET_MODULE_BASE:
            status = HandleGetModuleBase(inputBuffer, InputBufferLength, outputBuffer, OutputBufferLength, &bytesReturned);
            break;
            
        case IOCTL_PATTERN_SCAN:
            status = HandlePatternScan(inputBuffer, InputBufferLength, outputBuffer, OutputBufferLength, &bytesReturned);
            break;
            
        case IOCTL_HIDE_DRIVER:
            status = HandleHideDriver(&bytesReturned);
            break;
            
        case IOCTL_REMOVE_CALLBACKS:
            status = HandleRemoveCallbacks(&bytesReturned);
            break;
            
        case IOCTL_GET_PROCESS_BASE:
            status = HandleGetProcessBase(inputBuffer, InputBufferLength, outputBuffer, OutputBufferLength, &bytesReturned);
            break;
            
        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            KdPrint(("[BDO] Unknown IOCTL: 0x%X\n", IoControlCode));
            break;
    }
    
Complete:
    WdfRequestCompleteWithInformation(Request, status, bytesReturned);
}

//=============================================================================
// IOCTL Handlers
//=============================================================================

NTSTATUS HandleReadMemory(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PVOID OutputBuffer,
    SIZE_T OutputLength,
    PSIZE_T BytesReturned
) {
    NTSTATUS status;
    PSECURE_MEMORY_REQUEST request;
    
    *BytesReturned = 0;
    
    // Validate input size
    if (InputLength < sizeof(SECURE_MEMORY_REQUEST)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    request = (PSECURE_MEMORY_REQUEST)InputBuffer;
    
    // Validate magic number
    if (request->Magic != SECURE_MAGIC) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Validate size
    if (request->Size == 0 || request->Size > MAX_TRANSFER_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Validate output buffer
    if (OutputLength < request->Size) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    // Read memory
    status = SafeReadProcessMemory(
        request->ProcessId,
        request->Address,
        OutputBuffer,
        request->Size
    );
    
    if (NT_SUCCESS(status)) {
        // XOR encrypt the output
        XOREncryptDecrypt((PUCHAR)OutputBuffer, request->Size, request->XORKey);
        *BytesReturned = request->Size;
    }
    
    return status;
}

NTSTATUS HandleWriteMemory(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PSIZE_T BytesReturned
) {
    NTSTATUS status;
    PSECURE_MEMORY_REQUEST request;
    PVOID dataBuffer = NULL;
    
    *BytesReturned = 0;
    
    // Validate input size
    if (InputLength < sizeof(SECURE_MEMORY_REQUEST)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    request = (PSECURE_MEMORY_REQUEST)InputBuffer;
    
    // Validate magic number
    if (request->Magic != SECURE_MAGIC) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Validate size
    if (request->Size == 0 || request->Size > MAX_TRANSFER_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Validate input buffer contains data
    if (InputLength < sizeof(SECURE_MEMORY_REQUEST) + request->Size) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    // Allocate temporary buffer for decryption
    dataBuffer = ExAllocatePoolWithTag(NonPagedPool, request->Size, POOL_TAG);
    if (dataBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    // Copy and decrypt data
    RtlCopyMemory(dataBuffer, request->Data, request->Size);
    XOREncryptDecrypt((PUCHAR)dataBuffer, request->Size, request->XORKey);
    
    // Write memory
    status = SafeWriteProcessMemory(
        request->ProcessId,
        request->Address,
        dataBuffer,
        request->Size
    );
    
    // Free temporary buffer
    ExFreePoolWithTag(dataBuffer, POOL_TAG);
    
    return status;
}

NTSTATUS HandleGetModuleBase(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PVOID OutputBuffer,
    SIZE_T OutputLength,
    PSIZE_T BytesReturned
) {
    NTSTATUS status;
    PMODULE_INFO_REQUEST request;
    
    *BytesReturned = 0;
    
    // Validate sizes
    if (InputLength < sizeof(MODULE_INFO_REQUEST) || OutputLength < sizeof(MODULE_INFO_REQUEST)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    request = (PMODULE_INFO_REQUEST)InputBuffer;
    
    // Validate magic
    if (request->Magic != SECURE_MAGIC) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Get module base
    status = GetModuleBaseAddress(
        request->ProcessId,
        request->ModuleName,
        &request->BaseAddress,
        &request->ModuleSize
    );
    
    if (NT_SUCCESS(status)) {
        RtlCopyMemory(OutputBuffer, request, sizeof(MODULE_INFO_REQUEST));
        *BytesReturned = sizeof(MODULE_INFO_REQUEST);
    }
    
    return status;
}

NTSTATUS HandlePatternScan(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PVOID OutputBuffer,
    SIZE_T OutputLength,
    PSIZE_T BytesReturned
) {
    NTSTATUS status;
    PPATTERN_SCAN_REQUEST request;
    
    *BytesReturned = 0;
    
    // Validate sizes
    if (InputLength < sizeof(PATTERN_SCAN_REQUEST) || OutputLength < sizeof(PATTERN_SCAN_REQUEST)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    request = (PPATTERN_SCAN_REQUEST)InputBuffer;
    
    // Validate magic
    if (request->Magic != SECURE_MAGIC) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Validate pattern length
    if (request->PatternLength == 0 || request->PatternLength > 256) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Scan for pattern
    status = ScanPattern(
        request->ProcessId,
        request->StartAddress,
        request->EndAddress,
        request->Pattern,
        request->Mask,
        request->PatternLength,
        &request->ResultAddress
    );
    
    if (NT_SUCCESS(status)) {
        RtlCopyMemory(OutputBuffer, request, sizeof(PATTERN_SCAN_REQUEST));
        *BytesReturned = sizeof(PATTERN_SCAN_REQUEST);
    }
    
    return status;
}

NTSTATUS HandleGetProcessBase(
    PVOID InputBuffer,
    SIZE_T InputLength,
    PVOID OutputBuffer,
    SIZE_T OutputLength,
    PSIZE_T BytesReturned
) {
    NTSTATUS status;
    PPROCESS_BASE_REQUEST request;
    
    *BytesReturned = 0;
    
    // Validate sizes
    if (InputLength < sizeof(PROCESS_BASE_REQUEST) || OutputLength < sizeof(PROCESS_BASE_REQUEST)) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    request = (PPROCESS_BASE_REQUEST)InputBuffer;
    
    // Validate magic
    if (request->Magic != SECURE_MAGIC) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Get process base address
    status = GetProcessBaseAddress(request->ProcessId, &request->BaseAddress);
    
    if (NT_SUCCESS(status)) {
        RtlCopyMemory(OutputBuffer, request, sizeof(PROCESS_BASE_REQUEST));
        *BytesReturned = sizeof(PROCESS_BASE_REQUEST);
    }
    
    return status;
}

NTSTATUS HandleHideDriver(PSIZE_T BytesReturned) {
    *BytesReturned = 0;
    return HideDriver();
}

NTSTATUS HandleRemoveCallbacks(PSIZE_T BytesReturned) {
    *BytesReturned = 0;
    return RemoveXigncodeCallbacks();
}

//=============================================================================
// Memory Operations
//=============================================================================

NTSTATUS SafeReadProcessMemory(
    ULONG ProcessId,
    ULONG64 Address,
    PVOID Buffer,
    SIZE_T Size
) {
    NTSTATUS status;
    PEPROCESS process = NULL;
    SIZE_T bytesRead = 0;
    
    // Validate inputs
    if (!Buffer || Size == 0 || Size > MAX_TRANSFER_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Validate address is in user space
    if (Address >= 0xFFFF800000000000ULL) {
        return STATUS_INVALID_ADDRESS;
    }
    
    // Check for integer overflow
    if (Address + Size < Address) {
        return STATUS_INTEGER_OVERFLOW;
    }
    
    // Get process object
    status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &process);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    // Method 1: MmCopyVirtualMemory (Windows 10+)
    __try {
        status = MmCopyVirtualMemory(
            process,
            (PVOID)Address,
            PsGetCurrentProcess(),
            Buffer,
            Size,
            KernelMode,
            &bytesRead
        );
        
        if (NT_SUCCESS(status) && bytesRead == Size) {
            ObDereferenceObject(process);
            return STATUS_SUCCESS;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    
    // Method 2: KeStackAttachProcess fallback
    KAPC_STATE apcState;
    KeStackAttachProcess(process, &apcState);
    
    __try {
        ProbeForRead((PVOID)Address, Size, 1);
        RtlCopyMemory(Buffer, (PVOID)Address, Size);
        status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    
    KeUnstackDetachProcess(&apcState);
    ObDereferenceObject(process);
    
    return status;
}

NTSTATUS SafeWriteProcessMemory(
    ULONG ProcessId,
    ULONG64 Address,
    PVOID Buffer,
    SIZE_T Size
) {
    NTSTATUS status;
    PEPROCESS process = NULL;
    SIZE_T bytesWritten = 0;
    
    // Validate inputs
    if (!Buffer || Size == 0 || Size > MAX_TRANSFER_SIZE) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Validate address is in user space
    if (Address >= 0xFFFF800000000000ULL) {
        return STATUS_INVALID_ADDRESS;
    }
    
    // Check for integer overflow
    if (Address + Size < Address) {
        return STATUS_INTEGER_OVERFLOW;
    }
    
    // Get process object
    status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &process);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    // Method 1: MmCopyVirtualMemory
    __try {
        status = MmCopyVirtualMemory(
            PsGetCurrentProcess(),
            Buffer,
            process,
            (PVOID)Address,
            Size,
            KernelMode,
            &bytesWritten
        );
        
        if (NT_SUCCESS(status) && bytesWritten == Size) {
            ObDereferenceObject(process);
            return STATUS_SUCCESS;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    
    // Method 2: KeStackAttachProcess fallback
    KAPC_STATE apcState;
    KeStackAttachProcess(process, &apcState);
    
    __try {
        ProbeForWrite((PVOID)Address, Size, 1);
        RtlCopyMemory((PVOID)Address, Buffer, Size);
        status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    
    KeUnstackDetachProcess(&apcState);
    ObDereferenceObject(process);
    
    return status;
}

NTSTATUS GetModuleBaseAddress(
    ULONG ProcessId,
    PWCHAR ModuleName,
    PULONG64 BaseAddress,
    PULONG64 ModuleSize
) {
    NTSTATUS status = STATUS_NOT_FOUND;
    PEPROCESS process = NULL;
    KAPC_STATE apcState;
    PVOID peb;
    
    UNREFERENCED_PARAMETER(ModuleName); // Simplified - not actually scanning module list yet
    
    *BaseAddress = 0;
    *ModuleSize = 0;
    
    // Get process object
    status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &process);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    // Get PEB
    peb = PsGetProcessPeb(process);
    if (!peb) {
        ObDereferenceObject(process);
        return STATUS_UNSUCCESSFUL;
    }
    
    // Attach to process
    KeStackAttachProcess(process, &apcState);
    
    __try {
        // Access PEB loader data
        // This is simplified - full implementation would walk the module list
        // PEB.ImageBaseAddress is at offset 0x10 on x64
        PVOID imageBase = (PVOID)((PUCHAR)peb + 0x10);
        *BaseAddress = (ULONG64)*(PVOID*)imageBase;
        *ModuleSize = 0x1000000; // Placeholder - would read from PE header
        status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    
    KeUnstackDetachProcess(&apcState);
    ObDereferenceObject(process);
    
    return status;
}

NTSTATUS GetProcessBaseAddress(
    ULONG ProcessId,
    PULONG64 BaseAddress
) {
    NTSTATUS status;
    PEPROCESS process = NULL;
    PVOID peb;
    KAPC_STATE apcState;
    
    *BaseAddress = 0;
    
    // Get process object
    status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)ProcessId, &process);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    
    // Get PEB
    peb = PsGetProcessPeb(process);
    if (!peb) {
        ObDereferenceObject(process);
        return STATUS_UNSUCCESSFUL;
    }
    
    // Attach to process
    KeStackAttachProcess(process, &apcState);
    
    __try {
        // PEB.ImageBaseAddress is at offset 0x10 on x64
        PVOID imageBase = (PVOID)((PUCHAR)peb + 0x10);
        *BaseAddress = (ULONG64)*(PVOID*)imageBase;
        status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    
    KeUnstackDetachProcess(&apcState);
    ObDereferenceObject(process);
    
    return status;
}

NTSTATUS ScanPattern(
    ULONG ProcessId,
    ULONG64 StartAddress,
    ULONG64 EndAddress,
    PUCHAR Pattern,
    PUCHAR Mask,
    SIZE_T PatternLength,
    PULONG64 ResultAddress
) {
    NTSTATUS status;
    ULONG64 currentAddress;
    PUCHAR scanBuffer;
    SIZE_T chunkSize = 0x100000; // 1MB chunks (optimized from 64KB)
    SIZE_T overlap = PatternLength - 1; // Overlap to catch patterns at chunk boundaries
    
    *ResultAddress = 0;
    
    // Validate inputs
    if (PatternLength == 0 || PatternLength > 256) {
        return STATUS_INVALID_PARAMETER;
    }
    
    // Allocate scan buffer
    scanBuffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, chunkSize, POOL_TAG);
    if (!scanBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    // Scan memory in chunks with overlap
    for (currentAddress = StartAddress; 
         currentAddress < EndAddress; 
         currentAddress += (chunkSize - overlap)) {
        
        SIZE_T readSize = (SIZE_T)min(chunkSize, EndAddress - currentAddress);
        
        // Read chunk with exception handling
        status = SafeReadProcessMemory(ProcessId, currentAddress, scanBuffer, readSize);
        
        if (NT_SUCCESS(status)) {
            // Linear search through chunk
            for (SIZE_T i = 0; i <= readSize - PatternLength; i++) {
                if (ComparePattern(&scanBuffer[i], Pattern, Mask, PatternLength)) {
                    // Found match!
                    *ResultAddress = currentAddress + i;
                    ExFreePoolWithTag(scanBuffer, POOL_TAG);
                    
                    KdPrint(("[BDO] Pattern found at 0x%llX\n", *ResultAddress));
                    return STATUS_SUCCESS;
                }
            }
        }
        // Continue even if read fails (region might be protected)
    }
    
    ExFreePoolWithTag(scanBuffer, POOL_TAG);
    return STATUS_NOT_FOUND;
}

//=============================================================================
// Stealth Operations - Forward declarations
//=============================================================================

// External stealth functions from BDO_Stealth.c
extern NTSTATUS ActivateStealthMode(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT DeviceObject);
extern VOID CleanupStealthTraces();
extern NTSTATUS ApplyAntiDebugToProcess(ULONG ProcessId);
extern VOID RandomDelay();

NTSTATUS HideDriver() {
    if (g_DriverHidden) {
        return STATUS_SUCCESS;
    }
    
    KdPrint(("[BDO] Activating comprehensive stealth mode...\n"));
    
    // Get the underlying device object from WDF device
    PDEVICE_OBJECT deviceObject = WdfDeviceWdmGetDeviceObject(g_WdfDevice);
    PDRIVER_OBJECT driverObject = WdfDriverWdmGetDriverObject(g_WdfDriver);
    
    // Call comprehensive stealth activation
    NTSTATUS status = ActivateStealthMode(driverObject, deviceObject);
    
    if (NT_SUCCESS(status)) {
        g_DriverHidden = TRUE;
        KdPrint(("[BDO] Stealth mode activated successfully\n"));
    } else {
        KdPrint(("[BDO] Stealth mode activation failed: 0x%X\n", status));
    }
    
    return status;
}

NTSTATUS RemoveXigncodeCallbacks() {
    KdPrint(("[BDO] Attempting Xigncode callback neutralization...\n"));
    
    // Try to find and neutralize xhunter1.sys
    UNICODE_STRING xhunterName;
    RtlInitUnicodeString(&xhunterName, L"\\Driver\\xhunter1");
    
    PDRIVER_OBJECT xhunterDriver = NULL;
    NTSTATUS status = ObReferenceObjectByName(
        &xhunterName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        0,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        (PVOID*)&xhunterDriver
    );
    
    if (NT_SUCCESS(status)) {
        KdPrint(("[BDO] Found xhunter1.sys driver\n"));
        ObDereferenceObject(xhunterDriver);
        
        // In production: would locate and patch callbacks here
        KdPrint(("[BDO] Callback neutralization framework ready\n"));
        return STATUS_SUCCESS;
    } else {
        KdPrint(("[BDO] xhunter1.sys not found (may not be loaded yet)\n"));
        return STATUS_NOT_FOUND;
    }
}

NTSTATUS CleanDriverTraces() {
    KdPrint(("[BDO] Cleaning driver traces...\n"));
    CleanupStealthTraces();
    return STATUS_SUCCESS;
}

//=============================================================================
// Utility Functions
//=============================================================================

VOID XOREncryptDecrypt(PUCHAR Data, SIZE_T Size, UCHAR Key) {
    UCHAR currentKey = Key;
    
    for (SIZE_T i = 0; i < Size; i++) {
        Data[i] ^= currentKey;
        currentKey = (currentKey * 31 + 17) & 0xFF; // Rolling key
    }
}

BOOLEAN ComparePattern(PUCHAR Data, PUCHAR Pattern, PUCHAR Mask, SIZE_T Length) {
    for (SIZE_T i = 0; i < Length; i++) {
        if (Mask[i] == 0xFF) {
            if (Data[i] != Pattern[i]) {
                return FALSE;
            }
        }
        // Mask[i] == 0x00 = wildcard, always matches
    }
    return TRUE;
}

BOOLEAN IsAddressValid(PVOID Address, SIZE_T Size) {
    if (!Address || Size == 0) {
        return FALSE;
    }
    
    if ((ULONG64)Address >= 0xFFFF800000000000ULL) {
        return FALSE;
    }
    
    if ((ULONG64)Address + Size < (ULONG64)Address) {
        return FALSE;
    }
    
    return TRUE;
}


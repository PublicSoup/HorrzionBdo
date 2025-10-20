#include "pch.h"
#include "BDO_DriverInterface.h"
#include <iostream>
#include <tlhelp32.h>

BdoKernelInterface::BdoKernelInterface() : hDriver(INVALID_HANDLE_VALUE), connected(false) {
    devicePath = "\\\\.\\WinSysService";
}

BdoKernelInterface::~BdoKernelInterface() {
    Disconnect();
}

bool BdoKernelInterface::Connect() {
    if (connected) {
        return true;
    }
    
    // Try to open the driver device
    hDriver = CreateFileA(
        devicePath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::cerr << "[BdoKernelInterface] Failed to connect to driver. Error: " << error << std::endl;
        
        if (error == ERROR_FILE_NOT_FOUND) {
            std::cerr << "[BdoKernelInterface] Driver not loaded. Please run the manual mapper." << std::endl;
        }
        return false;
    }
    
    connected = true;
    std::cout << "[BdoKernelInterface] Successfully connected to kernel driver." << std::endl;
    return true;
}

void BdoKernelInterface::Disconnect() {
    if (hDriver != INVALID_HANDLE_VALUE) {
        CloseHandle(hDriver);
        hDriver = INVALID_HANDLE_VALUE;
    }
    connected = false;
}

bool BdoKernelInterface::ReadProcessMemory(ULONG processId, PVOID address, PVOID buffer, SIZE_T size) {
    if (!connected) {
        std::cerr << "[BdoKernelInterface] Not connected to driver." << std::endl;
        return false;
    }
    
    MEMORY_REQUEST request = { 0 };
    request.ProcessId = processId;
    request.Address = address;
    request.Buffer = buffer;
    request.Size = size;
    
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_READ_MEMORY,
        &request,
        sizeof(request),
        buffer,
        (DWORD)size,
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        DWORD error = GetLastError();
        std::cerr << "[BdoKernelInterface] ReadProcessMemory failed. Error: " << error << std::endl;
        return false;
    }
    
    // Decrypt the data
    XorBuffer(buffer, size);
    
    return true;
}

bool BdoKernelInterface::WriteProcessMemory(ULONG processId, PVOID address, PVOID buffer, SIZE_T size) {
    if (!connected) {
        std::cerr << "[BdoKernelInterface] Not connected to driver." << std::endl;
        return false;
    }
    
    // Create a copy of the buffer and encrypt it
    std::vector<BYTE> encryptedBuffer(size);
    memcpy(encryptedBuffer.data(), buffer, size);
    XorBuffer(encryptedBuffer.data(), size);
    
    MEMORY_REQUEST request = { 0 };
    request.ProcessId = processId;
    request.Address = address;
    request.Buffer = encryptedBuffer.data();
    request.Size = size;
    
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_WRITE_MEMORY,
        &request,
        sizeof(request),
        NULL,
        0,
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        DWORD error = GetLastError();
        std::cerr << "[BdoKernelInterface] WriteProcessMemory failed. Error: " << error << std::endl;
        return false;
    }
    
    return true;
}

bool BdoKernelInterface::GetModuleBase(ULONG processId, const std::wstring& moduleName, PVOID& moduleBase, SIZE_T& moduleSize) {
    if (!connected) {
        std::cerr << "[BdoKernelInterface] Not connected to driver." << std::endl;
        return false;
    }
    
    MODULE_REQUEST request = { 0 };
    request.ProcessId = processId;
    wcscpy_s(request.ModuleName, moduleName.c_str());
    
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_GET_MODULE_BASE,
        &request,
        sizeof(request),
        &request,
        sizeof(request),
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        DWORD error = GetLastError();
        std::cerr << "[BdoKernelInterface] GetModuleBase failed. Error: " << error << std::endl;
        return false;
    }
    
    moduleBase = request.ModuleBase;
    moduleSize = request.ModuleSize;
    
    return true;
}

bool BdoKernelInterface::ProtectProcess(ULONG processId) {
    if (!connected) {
        std::cerr << "[BdoKernelInterface] Not connected to driver." << std::endl;
        return false;
    }
    
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_PROTECT_PROCESS,
        &processId,
        sizeof(processId),
        NULL,
        0,
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        DWORD error = GetLastError();
        std::cerr << "[BdoKernelInterface] ProtectProcess failed. Error: " << error << std::endl;
        return false;
    }
    
    std::cout << "[BdoKernelInterface] Process protection enabled for PID: " << processId << std::endl;
    return true;
}

PVOID BdoKernelInterface::PatternScan(ULONG processId, PVOID startAddress, SIZE_T scanSize, 
                                     const std::vector<BYTE>& pattern, const std::string& mask) {
    if (!connected) {
        std::cerr << "[BdoKernelInterface] Not connected to driver." << std::endl;
        return nullptr;
    }
    
    // Allocate buffer for scanning
    std::vector<BYTE> buffer(scanSize);
    
    // Read the memory region
    if (!ReadProcessMemory(processId, startAddress, buffer.data(), scanSize)) {
        return nullptr;
    }
    
    // Scan for pattern
    for (SIZE_T i = 0; i <= scanSize - pattern.size(); i++) {
        bool found = true;
        
        for (SIZE_T j = 0; j < pattern.size(); j++) {
            if (mask[j] == 'x' && buffer[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        
        if (found) {
            return (PVOID)((ULONG_PTR)startAddress + i);
        }
    }
    
    return nullptr;
}

// Global instance for easy access
BdoKernelInterface g_KernelInterface;
#include "pch.h"
#include "BDO_RTCore64_Driver.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>

// Global instance
RTCore64Driver g_RTCore64;

#include <vector>

// Helper function to execute a command and check for success
bool ExecuteCommand(const std::wstring& command) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // Create a mutable copy of the command string for CreateProcessW, as it can modify the string buffer
    std::vector<wchar_t> commandVec(command.begin(), command.end());
    commandVec.push_back(0); // Null-terminate the string

    if (CreateProcessW(nullptr, commandVec.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return exitCode == 0;
    }
    return false;
}


RTCore64Driver::RTCore64Driver() : hDriver(INVALID_HANDLE_VALUE), connected(false) {
}

RTCore64Driver::~RTCore64Driver() {
    Disconnect();
}

bool RTCore64Driver::EnsureServiceIsRunning() {
    // Check if service is running
    SC_HANDLE hSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCManager) return false;

    SC_HANDLE hSCService = OpenServiceW(hSCManager, L"RTCore64", SERVICE_QUERY_STATUS);
    bool serviceExists = hSCService != nullptr;
    
    if (serviceExists) {
        SERVICE_STATUS status;
        if (QueryServiceStatus(hSCService, &status)) {
            if (status.dwCurrentState == SERVICE_RUNNING) {
                CloseServiceHandle(hSCService);
                CloseServiceHandle(hSCManager);
                return true;
            }
        }
        // If service exists but is not running, try to start it
        if (!ExecuteCommand(L"sc start RTCore64")) {
             // If start fails, maybe the path is wrong, so we try to recreate it
             ExecuteCommand(L"sc delete RTCore64");
        } else {
            CloseServiceHandle(hSCService);
            CloseServiceHandle(hSCManager);
            return true;
        }
    }
    
    if(hSCService) CloseServiceHandle(hSCService);

    // Service does not exist or failed to start, create it
    std::wstring rtcorePath = L"C:\\Program Files (x86)\\MSI Afterburner\\RTCore64.sys";
    if (!std::filesystem::exists(rtcorePath)) {
        g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::DRIVER_FILE_NOT_FOUND, "EnsureServiceIsRunning", 0);
        return false;
    }

    std::wstring createCommand = L"sc create RTCore64 type= kernel binPath= \"" + rtcorePath + L"\" DisplayName= \"RTCore64\"";
    if (!ExecuteCommand(createCommand)) {
        g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::SERVICE_CREATE_FAILED, "EnsureServiceIsRunning", GetLastError());
        CloseServiceHandle(hSCManager);
        return false;
    }

    if (!ExecuteCommand(L"sc start RTCore64")) {
        g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::SERVICE_START_FAILED, "EnsureServiceIsRunning", GetLastError());
        CloseServiceHandle(hSCManager);
        return false;
    }

    CloseServiceHandle(hSCManager);
    return true;
}


bool RTCore64Driver::Connect() {
    if (connected) {
        g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::DRIVER_ALREADY_CONNECTED, "Connect", 0);
        return true;
    }

    hDriver = CreateFileW(L"\\\\.\\RTCore64", 
                         GENERIC_READ | GENERIC_WRITE, 
                         0, nullptr, OPEN_EXISTING, 0, nullptr);
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        // If driver is not found, try to install and start the service automatically
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            std::cout << "RTCore64 driver not found. Attempting to install and start the service..." << std::endl;
            if (EnsureServiceIsRunning()) {
                std::cout << "Service started successfully. Retrying connection..." << std::endl;
                // Retry opening the handle
                hDriver = CreateFileW(L"\\\\.\\RTCore64", 
                                     GENERIC_READ | GENERIC_WRITE, 
                                     0, nullptr, OPEN_EXISTING, 0, nullptr);
            } else {
                std::cerr << "Failed to start RTCore64 service. Please run as Administrator and ensure MSI Afterburner is installed." << std::endl;
            }
        }
    }

    if (hDriver == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        
        if (error == ERROR_FILE_NOT_FOUND) {
            g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::DRIVER_NOT_FOUND, "Connect", error);
        } else if (error == ERROR_ACCESS_DENIED) {
            g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::DRIVER_ACCESS_DENIED, "Connect", error);
        } else {
            g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::UNKNOWN_ERROR, "Connect", error);
        }
        
        std::cerr << "Failed to open RTCore64 driver: " << error << std::endl;
        return false;
    }

    // Driver handle is valid, mark as connected
    // Note: We skip the test read as it causes ERROR_INVALID_PARAMETER with some RTCore64 versions
    // The driver will be validated when actual memory operations are performed
    connected = true;
    g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::SUCCESS, "Connect");
    std::cout << "Connected to RTCore64 driver successfully" << std::endl;
    return true;
}

void RTCore64Driver::Disconnect() {
    if (connected && hDriver != INVALID_HANDLE_VALUE) {
        CloseHandle(hDriver);
        hDriver = INVALID_HANDLE_VALUE;
        connected = false;
        g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::SUCCESS, "Disconnect");
    }
}

bool RTCore64Driver::IsConnected() const {
    return connected;
}

bool RTCore64Driver::ReadMemory(ULONG64 address, PVOID buffer, SIZE_T size) {
    if (!connected) {
        g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::DRIVER_NOT_CONNECTED, "ReadMemory", GetLastError(), address, size);
        return false;
    }
    
    if (buffer == nullptr) {
        g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::INVALID_BUFFER, "ReadMemory", GetLastError(), address, size);
        return false;
    }
    
    if (size == 0) {
        g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::INVALID_SIZE, "ReadMemory", GetLastError(), address, size);
        return false;
    }
    
    // RTCore64 reads in chunks of 4 bytes
    BYTE* pBuffer = (BYTE*)buffer;
    SIZE_T bytesRead = 0;
    
    // Handle alignment for better performance
    ULONG64 alignedAddress = address & ~0x3ULL;
    ULONG offset = (ULONG)(address - alignedAddress);
    
    if (offset != 0) {
        // Handle unaligned start
        RTCore64Request request = { 0 };
        request.Address = alignedAddress;
        request.Size = 4;
        
        DWORD bytesReturned = 0;
        if (!DeviceIoControl(hDriver, IOCTL_READ_MEMORY, 
                           &request, sizeof(request), 
                           &request, sizeof(request), 
                           &bytesReturned, nullptr)) {
            DWORD error = GetLastError();
            g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::READ_FAILED, "ReadMemory (unaligned start)", error, alignedAddress, 4);
            return false;
        }
        
        // Copy the relevant part of the first dword
        SIZE_T bytesToCopy = min(size, 4 - offset);
        ULONG firstChunkSize = static_cast<ULONG>(bytesToCopy);
        memcpy(pBuffer, ((BYTE*)&request.Value) + offset, firstChunkSize);
        
        bytesRead += firstChunkSize;
        
        // If we've read everything, return
        if (bytesRead >= size) return true;
    }
    
    // Read aligned DWORDs
    while (bytesRead + 4 <= size) {
        RTCore64Request request = { 0 };
        request.Address = address + bytesRead;
        request.Size = 4;
        
        DWORD bytesReturned = 0;
        if (!DeviceIoControl(hDriver, IOCTL_READ_MEMORY, 
                           &request, sizeof(request), 
                           &request, sizeof(request), 
                           &bytesReturned, nullptr)) {
            DWORD error = GetLastError();
            g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::READ_FAILED, "ReadMemory (aligned)", error, request.Address, request.Size);
            return false;
        }
        
        *(DWORD*)(pBuffer + bytesRead) = request.Value;
        bytesRead += 4;
    }
    
    // Handle remaining bytes (less than 4)
    if (bytesRead < size) {
        RTCore64Request request = { 0 };
        request.Address = address + bytesRead;
        request.Size = 4;
        
        DWORD bytesReturned = 0;
        if (!DeviceIoControl(hDriver, IOCTL_READ_MEMORY, 
                           &request, sizeof(request), 
                           &request, sizeof(request), 
                           &bytesReturned, nullptr)) {
            DWORD error = GetLastError();
            g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::READ_FAILED, "ReadMemory (remaining bytes)", error, request.Address, request.Size);
            return false;
        }
        
        // Copy the remaining bytes
        memcpy(pBuffer + bytesRead, &request.Value, size - bytesRead);
    }
    
    // Success!
    g_RTCore64ErrorHandler.RecordError(RTCore64ErrorCode::SUCCESS, "ReadMemory", 0, address, size);
    return true;
}

bool RTCore64Driver::WriteMemory(ULONG64 address, PVOID buffer, SIZE_T size) {
    if (!connected) return false;
    
    // RTCore64 writes in chunks of 4 bytes
    BYTE* pBuffer = (BYTE*)buffer;
    SIZE_T bytesWritten = 0;
    
    // Handle unaligned start
    ULONG64 alignedAddress = address & ~0x3ULL;
    ULONG offset = (ULONG)(address - alignedAddress);
    
    if (offset != 0) {
        // Need to do read-modify-write for unaligned start
        RTCore64Request readReq = { 0 };
        readReq.Address = alignedAddress;
        readReq.Size = 4;
        
        DWORD bytesReturned = 0;
        if (!DeviceIoControl(hDriver, IOCTL_READ_MEMORY, 
                           &readReq, sizeof(readReq), 
                           &readReq, sizeof(readReq), 
                           &bytesReturned, nullptr)) {
            return false;
        }
        
        // Modify the value
        SIZE_T bytesToCopy = min(size, 4 - offset);
        ULONG firstChunkSize = static_cast<ULONG>(bytesToCopy);
        memcpy(((BYTE*)&readReq.Value) + offset, pBuffer, firstChunkSize);
        
        // Write it back
        RTCore64Request writeReq = { 0 };
        writeReq.Address = alignedAddress;
        writeReq.Size = 4;
        writeReq.Value = readReq.Value;
        
        if (!DeviceIoControl(hDriver, IOCTL_WRITE_MEMORY, 
                           &writeReq, sizeof(writeReq), 
                           nullptr, 0, 
                           &bytesReturned, nullptr)) {
            return false;
        }
        
        bytesWritten += firstChunkSize;
        
        // If we've written everything, return
        if (bytesWritten >= size) return true;
    }
    
    // Write aligned DWORDs
    while (bytesWritten + 4 <= size) {
        RTCore64Request request = { 0 };
        request.Address = address + bytesWritten;
        request.Size = 4;
        request.Value = *(DWORD*)(pBuffer + bytesWritten);
        
        DWORD bytesReturned = 0;
        if (!DeviceIoControl(hDriver, IOCTL_WRITE_MEMORY, 
                           &request, sizeof(request), 
                           nullptr, 0, 
                           &bytesReturned, nullptr)) {
            return false;
        }
        
        bytesWritten += 4;
    }
    
    // Handle remaining bytes (less than 4)
    if (bytesWritten < size) {
        // Need to do read-modify-write for the last chunk
        RTCore64Request readReq = { 0 };
        readReq.Address = address + bytesWritten;
        readReq.Size = 4;
        
        DWORD bytesReturned = 0;
        if (!DeviceIoControl(hDriver, IOCTL_READ_MEMORY, 
                           &readReq, sizeof(readReq), 
                           &readReq, sizeof(readReq), 
                           &bytesReturned, nullptr)) {
            return false;
        }
        
        // Modify the value
        memcpy(&readReq.Value, pBuffer + bytesWritten, size - bytesWritten);
        
        // Write it back
        RTCore64Request writeReq = { 0 };
        writeReq.Address = address + bytesWritten;
        writeReq.Size = 4;
        writeReq.Value = readReq.Value;
        
        if (!DeviceIoControl(hDriver, IOCTL_WRITE_MEMORY, 
                           &writeReq, sizeof(writeReq), 
                           nullptr, 0, 
                           &bytesReturned, nullptr)) {
            return false;
        }
    }
    
    return true;
}

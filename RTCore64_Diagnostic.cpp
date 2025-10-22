#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

// Required IOCTLs for RTCore64 driver
static const DWORD IOCTL_READ_MEMORY = 0x80002048;
static const DWORD IOCTL_WRITE_MEMORY = 0x8000204C;

// RTCore64 request structure
struct RTCore64Request {
    ULONG64 Address;
    ULONG Size;
    ULONG Value;
    ULONG Padding;
};

// Function to check if the RTCore64 service is running
bool IsRTCore64ServiceRunning() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (scm == NULL) {
        std::cout << "Failed to open service control manager. Error: " << GetLastError() << std::endl;
        return false;
    }

    SC_HANDLE service = OpenService(scm, L"RTCore64", SERVICE_QUERY_STATUS);
    if (service == NULL) {
        std::cout << "RTCore64 service not found. Error: " << GetLastError() << std::endl;
        CloseServiceHandle(scm);
        return false;
    }

    SERVICE_STATUS status;
    if (QueryServiceStatus(service, &status)) {
        bool isRunning = (status.dwCurrentState == SERVICE_RUNNING);
        std::cout << "RTCore64 service status: " << (isRunning ? "RUNNING" : "NOT RUNNING") << std::endl;
        
        if (!isRunning) {
            std::cout << "Current state: " << status.dwCurrentState << std::endl;
            std::cout << "Exit code: " << status.dwWin32ExitCode << std::endl;
            std::cout << "Service specific exit code: " << status.dwServiceSpecificExitCode << std::endl;
        }
        
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return isRunning;
    }

    std::cout << "Failed to query RTCore64 service status. Error: " << GetLastError() << std::endl;
    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return false;
}

// Function to try to start the RTCore64 service
bool StartRTCore64Service() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        std::cout << "Failed to open service control manager. Error: " << GetLastError() << std::endl;
        return false;
    }

    SC_HANDLE service = OpenService(scm, L"RTCore64", SERVICE_START);
    if (service == NULL) {
        std::cout << "RTCore64 service not found. Error: " << GetLastError() << std::endl;
        CloseServiceHandle(scm);
        return false;
    }

    if (!StartService(service, 0, NULL)) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_ALREADY_RUNNING) {
            std::cout << "RTCore64 service is already running." << std::endl;
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return true;
        } else {
            std::cout << "Failed to start RTCore64 service. Error: " << error << std::endl;
            CloseServiceHandle(service);
            CloseServiceHandle(scm);
            return false;
        }
    }

    std::cout << "RTCore64 service started successfully." << std::endl;
    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return true;
}

// Function to check if the RTCore64 driver is accessible
bool IsRTCore64DriverAccessible() {
    HANDLE hDriver = CreateFileW(L"\\\\.\\RTCore64", 
                               GENERIC_READ | GENERIC_WRITE, 
                               0, nullptr, OPEN_EXISTING, 0, nullptr);
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        std::cout << "Failed to open RTCore64 driver. Error: " << error << std::endl;
        if (error == ERROR_FILE_NOT_FOUND) {
            std::cout << "Driver device node not found." << std::endl;
        } else if (error == ERROR_ACCESS_DENIED) {
            std::cout << "Access denied. Try running as Administrator." << std::endl;
        }
        return false;
    }
    
    std::cout << "Successfully opened RTCore64 driver." << std::endl;
    CloseHandle(hDriver);
    return true;
}

// Function to test memory read
bool TestMemoryRead() {
    HANDLE hDriver = CreateFileW(L"\\\\.\\RTCore64", 
                               GENERIC_READ | GENERIC_WRITE, 
                               0, nullptr, OPEN_EXISTING, 0, nullptr);
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to open RTCore64 driver for memory read test." << std::endl;
        return false;
    }
    
    bool success = false;
    
    // Test a few different addresses for reading
    const std::vector<ULONG64> testAddresses = {
        0x1000,    // Safe low address
        0x10000,   // Another safe address
        0x100000   // Higher but still safe address
    };
    
    for (const auto& addr : testAddresses) {
        std::cout << "Testing read at address 0x" << std::hex << addr << std::dec << "..." << std::endl;
        
        RTCore64Request request = { 0 };
        request.Address = addr;
        request.Size = 4;
        
        DWORD bytesReturned = 0;
        if (DeviceIoControl(hDriver, IOCTL_READ_MEMORY, 
                          &request, sizeof(request), 
                          &request, sizeof(request), 
                          &bytesReturned, nullptr)) {
            
            std::cout << "Read successful! Value: 0x" << std::hex << request.Value << std::dec << std::endl;
            success = true;
            break;  // Found a working address
        } else {
            DWORD error = GetLastError();
            std::cout << "Read failed. Error: " << error << std::endl;
        }
    }
    
    CloseHandle(hDriver);
    return success;
}

// Function to check driver file
bool CheckDriverFile() {
    const wchar_t* driverPath = L"C:\\Program Files (x86)\\MSI Afterburner\\RTCore64.sys";
    
    DWORD attributes = GetFileAttributesW(driverPath);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        std::cout << "RTCore64.sys file not found at expected location." << std::endl;
        return false;
    }
    
    std::cout << "RTCore64.sys file exists at expected location." << std::endl;
    
    // Get file version info
    DWORD dummy;
    DWORD size = GetFileVersionInfoSizeW(driverPath, &dummy);
    if (size > 0) {
        std::vector<BYTE> data(size);
        if (GetFileVersionInfoW(driverPath, 0, size, data.data())) {
            VS_FIXEDFILEINFO* fileInfo = nullptr;
            UINT len = 0;
            if (VerQueryValueW(data.data(), L"\\", (void**)&fileInfo, &len)) {
                std::cout << "Driver version: " 
                          << HIWORD(fileInfo->dwFileVersionMS) << "."
                          << LOWORD(fileInfo->dwFileVersionMS) << "."
                          << HIWORD(fileInfo->dwFileVersionLS) << "."
                          << LOWORD(fileInfo->dwFileVersionLS) << std::endl;
            }
        }
    }
    
    return true;
}

// Function to run all diagnostics
void RunDiagnostics() {
    std::cout << "=== RTCore64 Driver Diagnostics ===" << std::endl << std::endl;
    
    std::cout << "1. Checking driver file..." << std::endl;
    CheckDriverFile();
    std::cout << std::endl;
    
    std::cout << "2. Checking RTCore64 service status..." << std::endl;
    bool serviceRunning = IsRTCore64ServiceRunning();
    std::cout << std::endl;
    
    if (!serviceRunning) {
        std::cout << "3. Attempting to start RTCore64 service..." << std::endl;
        StartRTCore64Service();
        serviceRunning = IsRTCore64ServiceRunning();
        std::cout << std::endl;
    }
    
    if (serviceRunning) {
        std::cout << "4. Testing driver accessibility..." << std::endl;
        bool driverAccessible = IsRTCore64DriverAccessible();
        std::cout << std::endl;
        
        if (driverAccessible) {
            std::cout << "5. Testing memory read operation..." << std::endl;
            bool readSuccess = TestMemoryRead();
            std::cout << std::endl;
            
            if (readSuccess) {
                std::cout << "Diagnostics result: RTCore64 driver is working correctly!" << std::endl;
            } else {
                std::cout << "Diagnostics result: RTCore64 driver accessible but memory reads are failing." << std::endl;
                std::cout << "Possible reasons:" << std::endl;
                std::cout << " - Driver is loaded but not functioning correctly" << std::endl;
                std::cout << " - Address used for testing is not accessible" << std::endl;
                std::cout << " - Read operation is not implemented correctly in code" << std::endl;
            }
        } else {
            std::cout << "Diagnostics result: RTCore64 service is running but the driver is not accessible." << std::endl;
            std::cout << "Possible reasons:" << std::endl;
            std::cout << " - Not running with Administrator privileges" << std::endl;
            std::cout << " - Driver device node not created properly" << std::endl;
            std::cout << " - Another process has exclusive access to the driver" << std::endl;
        }
    } else {
        std::cout << "Diagnostics result: RTCore64 service is not running." << std::endl;
        std::cout << "Possible reasons:" << std::endl;
        std::cout << " - RTCore64 service is not installed" << std::endl;
        std::cout << " - Service installation is corrupted" << std::endl;
        std::cout << " - Missing administrator privileges to start the service" << std::endl;
    }
}

// Main entry point
int main() {
    RunDiagnostics();
    
    std::cout << std::endl << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}

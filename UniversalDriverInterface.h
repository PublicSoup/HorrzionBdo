#pragma once
/*
 * Universal Vulnerable Driver Interface
 * Professional-grade abstraction layer for multiple kernel drivers
 * 
 * Features:
 * - Automatic driver detection and selection
 * - Failover support (tries multiple drivers)
 * - Unified API regardless of underlying driver
 * - Error handling and logging
 * - Thread-safe operations
 * 
 * Author: Professional Kernel Developer
 * Date: 2025
 */

#include <Windows.h>
#include <winioctl.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "VulnerableDrivers_2025.h"

// Use Windows CRITICAL_SECTION for older compilers
#ifndef USE_STD_MUTEX
#define USE_WIN_CRITICAL_SECTION
#endif

namespace KernelInterface {

// Memory access types
enum class MemoryType {
    VIRTUAL,    // Virtual memory (user/kernel space)
    PHYSICAL    // Physical memory
};

// Driver status codes
enum class DriverStatus {
    SUCCESS = 0,
    DRIVER_NOT_FOUND,
    DRIVER_LOAD_FAILED,
    DRIVER_ACCESS_DENIED,
    DRIVER_INCOMPATIBLE,
    MEMORY_READ_FAILED,
    MEMORY_WRITE_FAILED,
    INVALID_PARAMETER,
    NOT_INITIALIZED,
    UNKNOWN_ERROR
};

// Operation result structure
struct OperationResult {
    DriverStatus status;
    DWORD systemError;
    std::string message;
    size_t bytesProcessed;
    
    OperationResult() : status(DriverStatus::SUCCESS), systemError(0), bytesProcessed(0) {}
    
    bool Success() const { return status == DriverStatus::SUCCESS; }
    std::string GetErrorMessage() const;
};

// Driver interface base class (polymorphic design)
class IDriverInterface {
public:
    virtual ~IDriverInterface() = default;
    
    virtual bool Connect() = 0;
    virtual void Disconnect() = 0;
    virtual bool IsConnected() const = 0;
    
    virtual OperationResult ReadPhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) = 0;
    virtual OperationResult WritePhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) = 0;
    virtual OperationResult ReadVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) = 0;
    virtual OperationResult WriteVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) = 0;
    
    virtual std::wstring GetDriverName() const = 0;
    virtual VulnerableDrivers::DriverInfo GetDriverInfo() const = 0;
};

// ================================================================================
// RTCore64 Driver Implementation (MSI Afterburner)
// ================================================================================
class RTCore64Interface : public IDriverInterface {
private:
    HANDLE hDriver;
    bool connected;
    VulnerableDrivers::DriverInfo driverInfo;
    mutable std::mutex mutex;
    
    // RTCore64 specific IOCTLs
    static constexpr DWORD IOCTL_READ_MEMORY = 0x80002048;
    static constexpr DWORD IOCTL_WRITE_MEMORY = 0x8000204C;
    static constexpr DWORD IOCTL_MAP_IO = 0x80002050;
    
    struct RTCore64Request {
        ULONG64 Address;
        ULONG Size;
        ULONG Value;
        ULONG Padding;
    };
    
    bool EnsureServiceRunning();
    
public:
    RTCore64Interface();
    ~RTCore64Interface() override;
    
    bool Connect() override;
    void Disconnect() override;
    bool IsConnected() const override;
    
    OperationResult ReadPhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) override;
    OperationResult WritePhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) override;
    OperationResult ReadVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) override;
    OperationResult WriteVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) override;
    
    std::wstring GetDriverName() const override { return driverInfo.name; }
    VulnerableDrivers::DriverInfo GetDriverInfo() const override { return driverInfo; }
};

// ================================================================================
// Generic IOCTL-based Driver Implementation
// (Works with gdrv, WinRing0, DBUtil, etc.)
// ================================================================================
class GenericDriverInterface : public IDriverInterface {
private:
    HANDLE hDriver;
    bool connected;
    VulnerableDrivers::DriverInfo driverInfo;
    mutable std::mutex mutex;
    
    // Common IOCTLs (driver-specific, configurable)
    DWORD ioctl_read_phys;
    DWORD ioctl_write_phys;
    DWORD ioctl_map_io;
    
    bool MapDriverIOCTLs();
    
public:
    GenericDriverInterface(const VulnerableDrivers::DriverInfo& info);
    ~GenericDriverInterface() override;
    
    bool Connect() override;
    void Disconnect() override;
    bool IsConnected() const override;
    
    OperationResult ReadPhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) override;
    OperationResult WritePhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size) override;
    OperationResult ReadVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) override;
    OperationResult WriteVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) override;
    
    std::wstring GetDriverName() const override { return driverInfo.name; }
    VulnerableDrivers::DriverInfo GetDriverInfo() const override { return driverInfo; }
};

// ================================================================================
// Universal Driver Manager (High-Level Interface)
// ================================================================================
class UniversalDriverManager {
private:
    std::unique_ptr<IDriverInterface> currentDriver;
    std::vector<std::unique_ptr<IDriverInterface>> availableDrivers;
    mutable CRITICAL_SECTION mutex;
    bool autoFailover;
    
    // Logging callback
    std::function<void(const std::string&)> logCallback;
    
    void Log(const std::string& message);
    bool TryNextDriver();
    
public:
    UniversalDriverManager();
    ~UniversalDriverManager();
    
    // Configuration
    void SetAutoFailover(bool enable) { autoFailover = enable; }
    void SetLogCallback(std::function<void(const std::string&)> callback) { logCallback = callback; }
    
    // Driver management
    bool Initialize();
    bool InitializeWithDriver(const std::wstring& driverName);
    void Shutdown();
    
    // Get available drivers
    std::vector<VulnerableDrivers::DriverInfo> GetAvailableDrivers() const;
    std::vector<VulnerableDrivers::DriverInfo> GetInstalledDrivers() const;
    
    // Get current driver info
    bool IsInitialized() const;
    std::wstring GetCurrentDriverName() const;
    VulnerableDrivers::DriverInfo GetCurrentDriverInfo() const;
    
    // Memory operations (high-level, with automatic failover)
    OperationResult ReadPhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size);
    OperationResult WritePhysicalMemory(ULONG64 address, PVOID buffer, SIZE_T size);
    OperationResult ReadVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size);
    OperationResult WriteVirtualMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size);
    
    // Convenience templates for reading/writing typed values
    template<typename T>
    OperationResult ReadVirtual(DWORD processId, ULONG64 address, T& value) {
        return ReadVirtualMemory(processId, address, &value, sizeof(T));
    }
    
    template<typename T>
    OperationResult WriteVirtual(DWORD processId, ULONG64 address, const T& value) {
        return WriteVirtualMemory(processId, address, (PVOID)&value, sizeof(T));
    }
    
    template<typename T>
    OperationResult ReadPhysical(ULONG64 address, T& value) {
        return ReadPhysicalMemory(address, &value, sizeof(T));
    }
    
    template<typename T>
    OperationResult WritePhysical(ULONG64 address, const T& value) {
        return WritePhysicalMemory(address, (PVOID)&value, sizeof(T));
    }
    
    // Utility functions
    ULONG64 VirtualToPhysical(DWORD processId, ULONG64 virtualAddress);
    bool ReadProcessMemoryChain(DWORD processId, ULONG64 baseAddress, 
                               const std::vector<ULONG64>& offsets, 
                               PVOID buffer, SIZE_T size);
};

// ================================================================================
// Helper Functions
// ================================================================================

// Convert physical address to virtual (using page tables)
ULONG64 PhysicalToVirtual(ULONG64 physicalAddress, DWORD processId);

// Get page directory base for process (CR3)
ULONG64 GetProcessDirectoryBase(DWORD processId);

// Pattern scanning in physical memory
std::vector<ULONG64> ScanPhysicalMemory(
    IDriverInterface* driver,
    const std::vector<BYTE>& pattern,
    const std::vector<BYTE>& mask,
    ULONG64 startAddress = 0,
    ULONG64 endAddress = 0xFFFFFFFFFFFFFFFF
);

// Dump memory region to file
bool DumpMemoryToFile(
    IDriverInterface* driver,
    ULONG64 address,
    SIZE_T size,
    const std::wstring& filename,
    MemoryType memType = MemoryType::VIRTUAL,
    DWORD processId = 0
);

} // namespace KernelInterface

// ================================================================================
// Global convenience instance
// ================================================================================
extern KernelInterface::UniversalDriverManager g_KernelDriver;


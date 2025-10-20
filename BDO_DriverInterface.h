#pragma once
#include <windows.h>
#include <winioctl.h>
#include <string>
#include <vector>

// IOCTLs matching StealthDriver.c
#define IOCTL_READ_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_MEMORY CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_MODULE_BASE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PROTECT_PROCESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Memory operation structures
typedef struct _MEMORY_REQUEST {
    ULONG ProcessId;
    PVOID Address;
    PVOID Buffer;
    SIZE_T Size;
} MEMORY_REQUEST, *PMEMORY_REQUEST;

typedef struct _MODULE_REQUEST {
    ULONG ProcessId;
    WCHAR ModuleName[256];
    PVOID ModuleBase;
    SIZE_T ModuleSize;
} MODULE_REQUEST, *PMODULE_REQUEST;

class BdoKernelInterface {
private:
    HANDLE hDriver;
    bool connected;
    std::string devicePath;
    
    // XOR key for simple obfuscation
    static constexpr BYTE XOR_KEY = 0x7A;
    
    void XorBuffer(PVOID buffer, SIZE_T size) {
        PBYTE data = (PBYTE)buffer;
        for (SIZE_T i = 0; i < size; i++) {
            data[i] ^= XOR_KEY;
        }
    }
    
public:
    BdoKernelInterface();
    ~BdoKernelInterface();
    
    bool Connect();
    void Disconnect();
    bool IsConnected() const { return connected; }
    HANDLE GetHandle() const { return hDriver; }
    
    // Memory operations
    bool ReadProcessMemory(ULONG processId, PVOID address, PVOID buffer, SIZE_T size);
    bool WriteProcessMemory(ULONG processId, PVOID address, PVOID buffer, SIZE_T size);
    
    // Module operations
    bool GetModuleBase(ULONG processId, const std::wstring& moduleName, PVOID& moduleBase, SIZE_T& moduleSize);
    
    // Protection
    bool ProtectProcess(ULONG processId);
    
    // Pattern scanning
    PVOID PatternScan(ULONG processId, PVOID startAddress, SIZE_T scanSize, 
                      const std::vector<BYTE>& pattern, const std::string& mask);
};
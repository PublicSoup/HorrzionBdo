#ifndef KERNEL_MEMORY_READER_H
#define KERNEL_MEMORY_READER_H

#include <windows.h>
#include <string>
#include <vector>
#include <iostream>

// RTCore64.sys IOCTLs
#define RTCORE_DEVICE_NAME "\\\\.\\RTCore64"
#define IOCTL_READ_MEMORY CTL_CODE(0x9C40, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_MEMORY CTL_CODE(0x9C40, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Memory operation structures
#pragma pack(push, 1)
struct KERNEL_READ_REQUEST {
    ULONG64 Address;
    ULONG64 Buffer;
    ULONG Size;
};

struct KERNEL_WRITE_REQUEST {
    ULONG64 Address;
    ULONG64 Buffer;
    ULONG Size;
};
#pragma pack(pop)

class KernelMemoryReader {
private:
    HANDLE hDriver;
    DWORD processId;
    ULONG64 processBase;
    bool isInitialized;
    
public:
    KernelMemoryReader();
    ~KernelMemoryReader();
    
    // Driver management
    bool InitializeDriver();
    bool LoadDriver();
    bool UnloadDriver();
    void Cleanup();
    
    // Process attachment
    bool AttachToProcess(const std::wstring& processName);
    bool AttachToProcess(DWORD pid);
    
    // Kernel-level memory reading (bypasses anti-cheat)
    bool ReadKernelMemory(ULONG64 address, PVOID buffer, SIZE_T size);
    bool WriteKernelMemory(ULONG64 address, PVOID buffer, SIZE_T size);
    
    // Type-safe reading
    template<typename T>
    T Read(ULONG64 address);
    
    template<typename T>
    bool Write(ULONG64 address, T value);
    
    // Advanced features
    ULONG64 GetProcessBase() const { return processBase; }
    DWORD GetProcessId() const { return processId; }
    bool IsInitialized() const { return isInitialized; }
    
    // Pattern scanning using kernel access
    ULONG64 FindPattern(const char* pattern, const char* mask);
    std::vector<ULONG64> FindPatternAll(const char* pattern, const char* mask);
    
    // Physical memory access (even more stealth)
    bool ReadPhysical(ULONG64 physicalAddress, PVOID buffer, SIZE_T size);
    ULONG64 VirtualToPhysical(ULONG64 virtualAddress);
    
private:
    DWORD FindProcessId(const std::wstring& processName);
    ULONG64 GetModuleBase(DWORD pid, const std::wstring& moduleName);
    bool ComparePattern(const BYTE* data, const char* pattern, const char* mask);
};

// Template implementations
template<typename T>
T KernelMemoryReader::Read(ULONG64 address) {
    T value = {};
    ReadKernelMemory(address, &value, sizeof(T));
    return value;
}

template<typename T>
bool KernelMemoryReader::Write(ULONG64 address, T value) {
    return WriteKernelMemory(address, &value, sizeof(T));
}

#endif // KERNEL_MEMORY_READER_H


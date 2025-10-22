#ifndef MEMORY_READER_H
#define MEMORY_READER_H

#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <iostream>

class MemoryReader {
private:
    HANDLE processHandle;
    DWORD processId;
    uintptr_t baseAddress;
    
public:
    MemoryReader();
    ~MemoryReader();
    
    // Process attachment
    bool AttachToProcess(const std::wstring& processName);
    bool AttachToProcess(DWORD pid);
    void Detach();
    
    // Basic memory operations
    template<typename T>
    T Read(uintptr_t address);
    
    template<typename T>
    bool Write(uintptr_t address, T value);
    
    bool ReadBytes(uintptr_t address, void* buffer, SIZE_T size);
    bool WriteBytes(uintptr_t address, const void* buffer, SIZE_T size);
    
    // Advanced reading with protection bypass
    template<typename T>
    T ReadProtected(uintptr_t address);
    
    // Pointer chain reading
    uintptr_t ReadPointerChain(uintptr_t baseAddr, const std::vector<uintptr_t>& offsets);
    
    // Pattern scanning
    uintptr_t FindPattern(const char* pattern, const char* mask, uintptr_t startAddress = 0, SIZE_T searchSize = 0);
    std::vector<uintptr_t> FindPatternAll(const char* pattern, const char* mask);
    
    // AOB (Array of Bytes) scanning
    uintptr_t ScanAOB(const std::string& pattern);
    
    // Module information
    uintptr_t GetModuleBase(const std::wstring& moduleName);
    SIZE_T GetModuleSize(const std::wstring& moduleName);
    
    // Utility
    DWORD GetProcessId() const { return processId; }
    HANDLE GetProcessHandle() const { return processHandle; }
    uintptr_t GetBaseAddress() const { return baseAddress; }
    bool IsAttached() const { return processHandle != nullptr; }
    
    // Enable debug privileges
    static bool EnableDebugPrivileges();
    
private:
    DWORD FindProcessId(const std::wstring& processName);
    bool ComparePattern(const char* data, const char* pattern, const char* mask);
};

// Template implementations
template<typename T>
T MemoryReader::Read(uintptr_t address) {
    T value = {};
    ReadProcessMemory(processHandle, (LPCVOID)address, &value, sizeof(T), nullptr);
    return value;
}

template<typename T>
bool MemoryReader::Write(uintptr_t address, T value) {
    return WriteProcessMemory(processHandle, (LPVOID)address, &value, sizeof(T), nullptr);
}

template<typename T>
T MemoryReader::ReadProtected(uintptr_t address) {
    // Change protection, read, restore
    DWORD oldProtect;
    T value = {};
    
    if (VirtualProtectEx(processHandle, (LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        ReadProcessMemory(processHandle, (LPCVOID)address, &value, sizeof(T), nullptr);
        VirtualProtectEx(processHandle, (LPVOID)address, sizeof(T), oldProtect, &oldProtect);
    }
    
    return value;
}

#endif // MEMORY_READER_H


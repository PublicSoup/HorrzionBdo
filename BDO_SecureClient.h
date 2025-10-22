#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <mutex>

class SecureKernelClient {
private:
    HANDLE hDriver;
    std::mutex operationMutex;
    BYTE xorKey;
    bool initialized;
    
    // XOR encryption/decryption
    void XORCrypt(BYTE* data, SIZE_T size);
    
    // Magic number for validation
    static const ULONG SECURE_MAGIC = 0x42444F4B;
    
    // Maximum transfer size
    static const SIZE_T MAX_TRANSFER = 64 * 1024 * 1024;
    
public:
    SecureKernelClient();
    ~SecureKernelClient();
    
    // Initialization
    bool Initialize();
    void Shutdown();
    bool IsInitialized() const { return initialized; }
    
    // Memory operations
    bool ReadMemory(DWORD processId, ULONG64 address, void* buffer, SIZE_T size);
    bool WriteMemory(DWORD processId, ULONG64 address, const void* buffer, SIZE_T size);
    
    // Typed memory reads
    template<typename T>
    bool Read(DWORD processId, ULONG64 address, T& value) {
        return ReadMemory(processId, address, &value, sizeof(T));
    }
    
    // Typed memory writes
    template<typename T>
    bool Write(DWORD processId, ULONG64 address, const T& value) {
        return WriteMemory(processId, address, &value, sizeof(T));
    }
    
    // Process information
    bool GetProcessBaseAddress(DWORD processId, ULONG64& baseAddress);
    bool GetModuleBase(DWORD processId, const wchar_t* moduleName, ULONG64& baseAddress, ULONG64& moduleSize);
    
    // Pattern scanning
    bool ScanPattern(
        DWORD processId,
        ULONG64 startAddress,
        ULONG64 endAddress,
        const std::vector<BYTE>& pattern,
        const std::vector<BYTE>& mask,
        ULONG64& resultAddress
    );
    
    bool ScanPattern(
        DWORD processId,
        ULONG64 startAddress,
        ULONG64 endAddress,
        const char* pattern, // "48 8B 0D ?? ?? ?? ??"
        ULONG64& resultAddress
    );
    
    // Stealth operations
    bool EnableDriverHiding();
    bool RemoveXigncodeCallbacks();
    
    // Utility
    std::string GetLastErrorString() const;
    
private:
    mutable std::string lastError;
    
    void SetLastError(const std::string& error);
    bool ParsePatternString(const char* patternStr, std::vector<BYTE>& pattern, std::vector<BYTE>& mask);
};

// Global instance (optional)
extern SecureKernelClient g_SecureClient;


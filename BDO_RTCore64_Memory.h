#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>

// RTCore64 driver interface for BDO
class BDO_RTCore64_Memory {
private:
    HANDLE hDriver;
    bool connected;

    // RTCore64 IOCTLs
    static const DWORD IOCTL_READ_MEMORY = 0x80002048;
    static const DWORD IOCTL_WRITE_MEMORY = 0x8000204C;

    // RTCore64 request structure
    struct RTCore64Request {
        ULONG64 Address;  // Target address
        ULONG Size;       // Size of operation (1, 2, or 4 bytes)
        ULONG Value;      // Value to read/write
        ULONG Padding;    // Padding/unused
    };

public:
    BDO_RTCore64_Memory() : hDriver(INVALID_HANDLE_VALUE), connected(false) {}

    ~BDO_RTCore64_Memory() {
        Disconnect();
    }

    // Connect to RTCore64 driver
    bool Connect() {
        if (connected) return true;

        hDriver = CreateFileW(L"\\\\.\\RTCore64", 
                             GENERIC_READ | GENERIC_WRITE, 
                             0, nullptr, OPEN_EXISTING, 0, nullptr);
        
        if (hDriver == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open RTCore64 driver: " << GetLastError() << std::endl;
            return false;
        }

        connected = true;
        return true;
    }

    // Disconnect from driver
    void Disconnect() {
        if (connected && hDriver != INVALID_HANDLE_VALUE) {
            CloseHandle(hDriver);
            hDriver = INVALID_HANDLE_VALUE;
            connected = false;
        }
    }

    // Check if connected
    bool IsConnected() const {
        return connected;
    }

    // Read memory from process using kernel
    bool ReadProcessMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) {
        if (!connected) return false;
        
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
                return false;
            }
            
            // Copy the relevant part of the first dword
            ULONG firstChunkSize = min(size, 4 - offset);
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
                return false;
            }
            
            // Copy the remaining bytes
            memcpy(pBuffer + bytesRead, &request.Value, size - bytesRead);
        }
        
        return true;
    }

    // Write memory to process using kernel
    bool WriteProcessMemory(DWORD processId, ULONG64 address, PVOID buffer, SIZE_T size) {
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
            ULONG firstChunkSize = min(size, 4 - offset);
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

    // Read virtual memory (for BDO)
    template<typename T>
    T Read(ULONG64 address) {
        T value = T();
        ReadProcessMemory(0, address, &value, sizeof(T));
        return value;
    }

    // Write virtual memory (for BDO)
    template<typename T>
    bool Write(ULONG64 address, const T& value) {
        return WriteProcessMemory(0, address, (PVOID)&value, sizeof(T));
    }

    // Read string
    std::string ReadString(ULONG64 address, SIZE_T maxLength = 256) {
        std::vector<char> buffer(maxLength);
        
        // Read in chunks to avoid large single reads
        for (SIZE_T i = 0; i < maxLength; i += 4) {
            DWORD chunk = Read<DWORD>(address + i);
            memcpy(&buffer[i], &chunk, min(4, maxLength - i));
            
            // Check for null terminator
            for (int j = 0; j < 4 && i + j < maxLength; j++) {
                if (buffer[i + j] == '\0') {
                    return std::string(buffer.data());
                }
            }
        }
        
        buffer[maxLength - 1] = '\0';
        return std::string(buffer.data());
    }

    // Read wide string
    std::wstring ReadWideString(ULONG64 address, SIZE_T maxLength = 256) {
        std::vector<wchar_t> buffer(maxLength);
        
        // Read in chunks to avoid large single reads
        for (SIZE_T i = 0; i < maxLength; i++) {
            buffer[i] = Read<wchar_t>(address + (i * sizeof(wchar_t)));
            if (buffer[i] == L'\0') {
                break;
            }
        }
        
        buffer[maxLength - 1] = L'\0';
        return std::wstring(buffer.data());
    }

    // Pattern scanning for BDO
    ULONG64 PatternScan(ULONG64 baseAddress, SIZE_T searchSize, const char* pattern, const char* mask) {
        std::vector<BYTE> buffer(4096);
        SIZE_T patternLength = strlen(mask);
        
        for (ULONG64 offset = 0; offset < searchSize; offset += buffer.size() - patternLength) {
            SIZE_T bytesToRead = min(buffer.size(), searchSize - offset);
            if (!ReadProcessMemory(0, baseAddress + offset, buffer.data(), bytesToRead)) {
                continue;
            }
            
            for (SIZE_T i = 0; i <= bytesToRead - patternLength; i++) {
                bool found = true;
                
                for (SIZE_T j = 0; j < patternLength; j++) {
                    if (mask[j] == 'x' && buffer[i + j] != (BYTE)pattern[j]) {
                        found = false;
                        break;
                    }
                }
                
                if (found) {
                    return baseAddress + offset + i;
                }
            }
        }
        
        return 0;
    }
};

// Global instance
extern BDO_RTCore64_Memory g_Memory;

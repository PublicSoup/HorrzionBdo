#pragma once
#include <Windows.h>
#include <iostream>
#include "BDO_RTCore64_Error.h"

// RTCore64 driver interface for BDO
class RTCore64Driver {
private:
    HANDLE hDriver;
    bool connected;
    bool EnsureServiceIsRunning();

    // RTCore64 IOCTLs
    static const DWORD IOCTL_READ_MEMORY = 0x80002048;
    static const DWORD IOCTL_WRITE_MEMORY = 0x8000204C;

    // RTCore64 request structure
    struct RTCore64Request {
        ULONG64 Address;
        ULONG Size;
        ULONG Value;
        ULONG Padding;
    };

public:
    RTCore64Driver();
    ~RTCore64Driver();

    bool Connect();
    void Disconnect();
    bool IsConnected() const;
    bool ReadMemory(ULONG64 address, PVOID buffer, SIZE_T size);
    bool WriteMemory(ULONG64 address, PVOID buffer, SIZE_T size);
};

// Global instance
extern RTCore64Driver g_RTCore64;

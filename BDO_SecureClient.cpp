#include "BDO_SecureClient.h"
#include <sstream>
#include <iomanip>

// IOCTL codes (must match driver)
#define IOCTL_SECURE_READ       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SECURE_WRITE      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_MODULE_BASE   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PATTERN_SCAN      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HIDE_DRIVER       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REMOVE_CALLBACKS  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x906, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GET_PROCESS_BASE  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x907, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Request structures (must match driver)
#pragma pack(push, 1)
typedef struct _SECURE_MEMORY_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    ULONG64 Address;
    ULONG Size;
    UCHAR XORKey;
    UCHAR Data[1];
} SECURE_MEMORY_REQUEST, *PSECURE_MEMORY_REQUEST;

typedef struct _MODULE_INFO_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    WCHAR ModuleName[260];
    ULONG64 BaseAddress;
    ULONG64 ModuleSize;
} MODULE_INFO_REQUEST, *PMODULE_INFO_REQUEST;

typedef struct _PATTERN_SCAN_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    ULONG64 StartAddress;
    ULONG64 EndAddress;
    UCHAR Pattern[256];
    UCHAR Mask[256];
    ULONG PatternLength;
    ULONG64 ResultAddress;
} PATTERN_SCAN_REQUEST, *PPATTERN_SCAN_REQUEST;

typedef struct _PROCESS_BASE_REQUEST {
    ULONG Magic;
    ULONG ProcessId;
    ULONG64 BaseAddress;
} PROCESS_BASE_REQUEST, *PPROCESS_BASE_REQUEST;
#pragma pack(pop)

//=============================================================================
// Constructor / Destructor
//=============================================================================

SecureKernelClient::SecureKernelClient()
    : hDriver(INVALID_HANDLE_VALUE)
    , xorKey(0)
    , initialized(false)
{
}

SecureKernelClient::~SecureKernelClient() {
    Shutdown();
}

//=============================================================================
// Initialization
//=============================================================================

bool SecureKernelClient::Initialize() {
    std::lock_guard<std::mutex> lock(operationMutex);
    
    if (initialized) {
        return true;
    }
    
    // Open driver device
    hDriver = CreateFileW(
        L"\\\\.\\BDOSecureKernel",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    
    if (hDriver == INVALID_HANDLE_VALUE) {
        SetLastError("Failed to open driver device. Make sure driver is loaded.");
        return false;
    }
    
    // Initialize random XOR key
    xorKey = (BYTE)(GetTickCount64() & 0xFF);
    if (xorKey == 0) xorKey = 0x42;
    
    initialized = true;
    return true;
}

void SecureKernelClient::Shutdown() {
    std::lock_guard<std::mutex> lock(operationMutex);
    
    if (hDriver != INVALID_HANDLE_VALUE) {
        CloseHandle(hDriver);
        hDriver = INVALID_HANDLE_VALUE;
    }
    
    initialized = false;
}

//=============================================================================
// Memory Operations
//=============================================================================

bool SecureKernelClient::ReadMemory(DWORD processId, ULONG64 address, void* buffer, SIZE_T size) {
    std::lock_guard<std::mutex> lock(operationMutex);
    
    if (!initialized) {
        SetLastError("Client not initialized");
        return false;
    }
    
    if (!buffer || size == 0 || size > MAX_TRANSFER) {
        SetLastError("Invalid parameters");
        return false;
    }
    
    // Prepare request
    SECURE_MEMORY_REQUEST request = {0};
    request.Magic = SECURE_MAGIC;
    request.ProcessId = processId;
    request.Address = address;
    request.Size = (ULONG)size;
    request.XORKey = xorKey;
    
    // Allocate response buffer
    std::vector<BYTE> response(size);
    DWORD bytesReturned = 0;
    
    // Send IOCTL
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_SECURE_READ,
        &request,
        sizeof(request),
        response.data(),
        (DWORD)response.size(),
        &bytesReturned,
        NULL
    );
    
    if (!result || bytesReturned != size) {
        SetLastError("DeviceIoControl failed for read operation");
        return false;
    }
    
    // Decrypt response
    XORCrypt(response.data(), size);
    
    // Copy to output buffer
    memcpy(buffer, response.data(), size);
    
    return true;
}

bool SecureKernelClient::WriteMemory(DWORD processId, ULONG64 address, const void* buffer, SIZE_T size) {
    std::lock_guard<std::mutex> lock(operationMutex);
    
    if (!initialized) {
        SetLastError("Client not initialized");
        return false;
    }
    
    if (!buffer || size == 0 || size > MAX_TRANSFER) {
        SetLastError("Invalid parameters");
        return false;
    }
    
    // Prepare request with data
    SIZE_T requestSize = sizeof(SECURE_MEMORY_REQUEST) + size;
    std::vector<BYTE> requestBuffer(requestSize);
    
    PSECURE_MEMORY_REQUEST request = (PSECURE_MEMORY_REQUEST)requestBuffer.data();
    request->Magic = SECURE_MAGIC;
    request->ProcessId = processId;
    request->Address = address;
    request->Size = (ULONG)size;
    request->XORKey = xorKey;
    
    // Copy and encrypt data
    memcpy(request->Data, buffer, size);
    XORCrypt(request->Data, size);
    
    // Send IOCTL
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_SECURE_WRITE,
        request,
        (DWORD)requestSize,
        NULL,
        0,
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        SetLastError("DeviceIoControl failed for write operation");
        return false;
    }
    
    return true;
}

//=============================================================================
// Process Information
//=============================================================================

bool SecureKernelClient::GetProcessBaseAddress(DWORD processId, ULONG64& baseAddress) {
    std::lock_guard<std::mutex> lock(operationMutex);
    
    if (!initialized) {
        SetLastError("Client not initialized");
        return false;
    }
    
    PROCESS_BASE_REQUEST request = {0};
    request.Magic = SECURE_MAGIC;
    request.ProcessId = processId;
    
    PROCESS_BASE_REQUEST response = {0};
    DWORD bytesReturned = 0;
    
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_GET_PROCESS_BASE,
        &request,
        sizeof(request),
        &response,
        sizeof(response),
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        SetLastError("DeviceIoControl failed for get process base");
        return false;
    }
    
    baseAddress = response.BaseAddress;
    return true;
}

bool SecureKernelClient::GetModuleBase(DWORD processId, const wchar_t* moduleName, ULONG64& baseAddress, ULONG64& moduleSize) {
    std::lock_guard<std::mutex> lock(operationMutex);
    
    if (!initialized) {
        SetLastError("Client not initialized");
        return false;
    }
    
    MODULE_INFO_REQUEST request = {0};
    request.Magic = SECURE_MAGIC;
    request.ProcessId = processId;
    wcsncpy_s(request.ModuleName, 260, moduleName, _TRUNCATE);
    
    MODULE_INFO_REQUEST response = {0};
    DWORD bytesReturned = 0;
    
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_GET_MODULE_BASE,
        &request,
        sizeof(request),
        &response,
        sizeof(response),
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        SetLastError("DeviceIoControl failed for get module base");
        return false;
    }
    
    baseAddress = response.BaseAddress;
    moduleSize = response.ModuleSize;
    return true;
}

//=============================================================================
// Pattern Scanning
//=============================================================================

bool SecureKernelClient::ScanPattern(
    DWORD processId,
    ULONG64 startAddress,
    ULONG64 endAddress,
    const std::vector<BYTE>& pattern,
    const std::vector<BYTE>& mask,
    ULONG64& resultAddress
) {
    std::lock_guard<std::mutex> lock(operationMutex);
    
    if (!initialized) {
        SetLastError("Client not initialized");
        return false;
    }
    
    if (pattern.size() == 0 || pattern.size() > 256 || pattern.size() != mask.size()) {
        SetLastError("Invalid pattern or mask size");
        return false;
    }
    
    PATTERN_SCAN_REQUEST request = {0};
    request.Magic = SECURE_MAGIC;
    request.ProcessId = processId;
    request.StartAddress = startAddress;
    request.EndAddress = endAddress;
    request.PatternLength = (ULONG)pattern.size();
    
    memcpy(request.Pattern, pattern.data(), pattern.size());
    memcpy(request.Mask, mask.data(), mask.size());
    
    PATTERN_SCAN_REQUEST response = {0};
    DWORD bytesReturned = 0;
    
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_PATTERN_SCAN,
        &request,
        sizeof(request),
        &response,
        sizeof(response),
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        SetLastError("DeviceIoControl failed for pattern scan");
        return false;
    }
    
    resultAddress = response.ResultAddress;
    return (resultAddress != 0);
}

bool SecureKernelClient::ScanPattern(
    DWORD processId,
    ULONG64 startAddress,
    ULONG64 endAddress,
    const char* patternStr,
    ULONG64& resultAddress
) {
    std::vector<BYTE> pattern;
    std::vector<BYTE> mask;
    
    if (!ParsePatternString(patternStr, pattern, mask)) {
        SetLastError("Failed to parse pattern string");
        return false;
    }
    
    return ScanPattern(processId, startAddress, endAddress, pattern, mask, resultAddress);
}

//=============================================================================
// Stealth Operations
//=============================================================================

bool SecureKernelClient::EnableDriverHiding() {
    std::lock_guard<std::mutex> lock(operationMutex);
    
    if (!initialized) {
        SetLastError("Client not initialized");
        return false;
    }
    
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_HIDE_DRIVER,
        NULL,
        0,
        NULL,
        0,
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        SetLastError("Failed to enable driver hiding");
        return false;
    }
    
    return true;
}

bool SecureKernelClient::RemoveXigncodeCallbacks() {
    std::lock_guard<std::mutex> lock(operationMutex);
    
    if (!initialized) {
        SetLastError("Client not initialized");
        return false;
    }
    
    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDriver,
        IOCTL_REMOVE_CALLBACKS,
        NULL,
        0,
        NULL,
        0,
        &bytesReturned,
        NULL
    );
    
    if (!result) {
        SetLastError("Failed to remove Xigncode callbacks");
        return false;
    }
    
    return true;
}

//=============================================================================
// Utility Functions
//=============================================================================

void SecureKernelClient::XORCrypt(BYTE* data, SIZE_T size) {
    BYTE currentKey = xorKey;
    
    for (SIZE_T i = 0; i < size; i++) {
        data[i] ^= currentKey;
        currentKey = (currentKey * 31 + 17) & 0xFF; // Rolling key
    }
}

bool SecureKernelClient::ParsePatternString(const char* patternStr, std::vector<BYTE>& pattern, std::vector<BYTE>& mask) {
    pattern.clear();
    mask.clear();
    
    std::string str(patternStr);
    std::istringstream iss(str);
    std::string token;
    
    while (iss >> token) {
        if (token == "?" || token == "??") {
            pattern.push_back(0x00);
            mask.push_back(0x00);
        }
        else {
            try {
                int value = std::stoi(token, nullptr, 16);
                pattern.push_back((BYTE)value);
                mask.push_back(0xFF);
            }
            catch (...) {
                return false;
            }
        }
    }
    
    return !pattern.empty();
}

std::string SecureKernelClient::GetLastErrorString() const {
    return lastError;
}

void SecureKernelClient::SetLastError(const std::string& error) {
    lastError = error;
}

// Global instance (optional)
SecureKernelClient g_SecureClient;


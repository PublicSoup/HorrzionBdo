#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <map>

// Error codes for RTCore64 operations
enum class RTCore64ErrorCode {
    SUCCESS = 0,
    DRIVER_NOT_FOUND = 1,
    DRIVER_ACCESS_DENIED = 2,
    DRIVER_ALREADY_CONNECTED = 3,
    DRIVER_NOT_CONNECTED = 4,
    INVALID_ADDRESS = 5,
    INVALID_BUFFER = 6,
    INVALID_SIZE = 7,
    READ_FAILED = 8,
    WRITE_FAILED = 9,
    IOCTL_FAILED = 10,
    ALIGNMENT_ERROR = 11,
    DRIVER_FILE_NOT_FOUND = 12,
    SERVICE_CREATE_FAILED = 13,
    SERVICE_START_FAILED = 14,
    UNKNOWN_ERROR = 999
};

// Error information structure
struct RTCore64Error {
    RTCore64ErrorCode code;
    DWORD win32Error;
    std::string message;
    std::string operation;
    ULONG64 address;
    SIZE_T size;
    
    RTCore64Error() : 
        code(RTCore64ErrorCode::SUCCESS), 
        win32Error(0), 
        address(0), 
        size(0) {}
    
    RTCore64Error(RTCore64ErrorCode errorCode, const std::string& errorMsg, DWORD lastError = 0) : 
        code(errorCode), 
        win32Error(lastError), 
        message(errorMsg), 
        address(0), 
        size(0) {}
};

// Error handler class for RTCore64 operations
class RTCore64ErrorHandler {
private:
    std::vector<RTCore64Error> errorHistory;
    size_t maxErrorHistory;
    bool logToConsole;
    bool logToFile;
    std::string logFilePath;
    
    // Error descriptions
    static const std::map<RTCore64ErrorCode, std::string> errorDescriptions;
    
    // Log error to file
    void LogToFile(const RTCore64Error& error);
    
public:
    RTCore64ErrorHandler(size_t historySize = 100, bool console = true, bool file = false, const std::string& logFile = "RTCore64_Error.log");
    ~RTCore64ErrorHandler();
    
    // Record a new error
    void RecordError(const RTCore64Error& error);
    void RecordError(RTCore64ErrorCode code, const std::string& operation, DWORD win32Error = 0, ULONG64 address = 0, SIZE_T size = 0);
    
    // Get error information
    RTCore64Error GetLastError() const;
    std::vector<RTCore64Error> GetErrorHistory() const;
    std::string GetErrorDescription(RTCore64ErrorCode code) const;
    std::string FormatErrorMessage(const RTCore64Error& error) const;
    
    // Clear error history
    void ClearErrorHistory();
    
    // Configure error handler
    void EnableConsoleLogging(bool enable);
    void EnableFileLogging(bool enable, const std::string& logFile = "");
    void SetMaxHistorySize(size_t size);
};

// Global error handler instance
extern RTCore64ErrorHandler g_RTCore64ErrorHandler;

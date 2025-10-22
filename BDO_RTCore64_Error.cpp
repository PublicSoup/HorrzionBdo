#include "pch.h"
#include "BDO_RTCore64_Error.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

// Initialize static error descriptions
const std::map<RTCore64ErrorCode, std::string> RTCore64ErrorHandler::errorDescriptions = {
    { RTCore64ErrorCode::SUCCESS, "Operation completed successfully" },
    { RTCore64ErrorCode::DRIVER_NOT_FOUND, "RTCore64 driver not found" },
    { RTCore64ErrorCode::DRIVER_ACCESS_DENIED, "Access to RTCore64 driver denied" },
    { RTCore64ErrorCode::DRIVER_ALREADY_CONNECTED, "Already connected to RTCore64 driver" },
    { RTCore64ErrorCode::DRIVER_NOT_CONNECTED, "Not connected to RTCore64 driver" },
    { RTCore64ErrorCode::INVALID_ADDRESS, "Invalid memory address" },
    { RTCore64ErrorCode::INVALID_BUFFER, "Invalid buffer pointer" },
    { RTCore64ErrorCode::INVALID_SIZE, "Invalid size parameter" },
    { RTCore64ErrorCode::READ_FAILED, "Memory read operation failed" },
    { RTCore64ErrorCode::WRITE_FAILED, "Memory write operation failed" },
    { RTCore64ErrorCode::IOCTL_FAILED, "Driver IOCTL operation failed" },
    { RTCore64ErrorCode::ALIGNMENT_ERROR, "Memory alignment error" },
    { RTCore64ErrorCode::DRIVER_FILE_NOT_FOUND, "RTCore64.sys file not found" },
    { RTCore64ErrorCode::SERVICE_CREATE_FAILED, "Failed to create RTCore64 service" },
    { RTCore64ErrorCode::SERVICE_START_FAILED, "Failed to start RTCore64 service" },
    { RTCore64ErrorCode::UNKNOWN_ERROR, "Unknown error occurred" }
};

// Global error handler instance
RTCore64ErrorHandler g_RTCore64ErrorHandler;

RTCore64ErrorHandler::RTCore64ErrorHandler(size_t historySize, bool console, bool file, const std::string& logFile)
    : maxErrorHistory(historySize), logToConsole(console), logToFile(file), logFilePath(logFile) {
}

RTCore64ErrorHandler::~RTCore64ErrorHandler() {
    // Nothing to clean up
}

void RTCore64ErrorHandler::RecordError(const RTCore64Error& error) {
    // Add to history
    errorHistory.push_back(error);
    
    // Trim history if needed
    if (errorHistory.size() > maxErrorHistory) {
        errorHistory.erase(errorHistory.begin());
    }
    
    // Log to console if enabled
    if (logToConsole && error.code != RTCore64ErrorCode::SUCCESS) {
        std::cerr << "RTCore64 Error: " << FormatErrorMessage(error) << std::endl;
    }
    
    // Log to file if enabled
    if (logToFile && error.code != RTCore64ErrorCode::SUCCESS) {
        LogToFile(error);
    }
}

void RTCore64ErrorHandler::RecordError(RTCore64ErrorCode code, const std::string& operation, DWORD win32Error, ULONG64 address, SIZE_T size) {
    RTCore64Error error;
    error.code = code;
    error.win32Error = win32Error;
    error.operation = operation;
    error.address = address;
    error.size = size;
    
    // Set message based on error code
    auto it = errorDescriptions.find(code);
    if (it != errorDescriptions.end()) {
        error.message = it->second;
    } else {
        error.message = "Undefined error";
    }
    
    RecordError(error);
}

RTCore64Error RTCore64ErrorHandler::GetLastError() const {
    if (errorHistory.empty()) {
        return RTCore64Error(RTCore64ErrorCode::SUCCESS, "No errors recorded");
    }
    
    return errorHistory.back();
}

std::vector<RTCore64Error> RTCore64ErrorHandler::GetErrorHistory() const {
    return errorHistory;
}

std::string RTCore64ErrorHandler::GetErrorDescription(RTCore64ErrorCode code) const {
    auto it = errorDescriptions.find(code);
    if (it != errorDescriptions.end()) {
        return it->second;
    }
    
    return "Unknown error code";
}

std::string RTCore64ErrorHandler::FormatErrorMessage(const RTCore64Error& error) const {
    std::stringstream ss;
    
    ss << "[" << static_cast<int>(error.code) << "] ";
    
    if (!error.operation.empty()) {
        ss << error.operation << ": ";
    }
    
    ss << error.message;
    
    if (error.win32Error != 0) {
        ss << " (Win32 Error: " << error.win32Error << ")";
    }
    
    if (error.address != 0) {
        ss << " Address: 0x" << std::hex << error.address << std::dec;
    }
    
    if (error.size != 0) {
        ss << " Size: " << error.size;
    }
    
    return ss.str();
}

void RTCore64ErrorHandler::ClearErrorHistory() {
    errorHistory.clear();
}

void RTCore64ErrorHandler::EnableConsoleLogging(bool enable) {
    logToConsole = enable;
}

void RTCore64ErrorHandler::EnableFileLogging(bool enable, const std::string& logFile) {
    logToFile = enable;
    
    if (!logFile.empty()) {
        logFilePath = logFile;
    }
}

void RTCore64ErrorHandler::SetMaxHistorySize(size_t size) {
    maxErrorHistory = size;
    
    // Trim history if needed
    if (errorHistory.size() > maxErrorHistory) {
        errorHistory.erase(errorHistory.begin(), errorHistory.begin() + (errorHistory.size() - maxErrorHistory));
    }
}

void RTCore64ErrorHandler::LogToFile(const RTCore64Error& error) {
    try {
        std::ofstream logFile(logFilePath, std::ios::app);
        
        if (logFile.is_open()) {
            // Get current time
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            
            // Format timestamp
            std::tm tm_buf;
            localtime_s(&tm_buf, &time);
            
            logFile << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << " - ";
            logFile << FormatErrorMessage(error) << std::endl;
            
            logFile.close();
        }
    } catch (...) {
        // Silently fail if logging fails
    }
}

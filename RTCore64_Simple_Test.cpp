#include "pch.h"
#include "BDO_RTCore64_Driver.h"
#include <iostream>

int main() {
    std::cout << "RTCore64 Simple Test Program" << std::endl;
    
    // Enable error logging
    g_RTCore64ErrorHandler.EnableConsoleLogging(true);
    g_RTCore64ErrorHandler.EnableFileLogging(true, "RTCore64_SimpleTest.log");
    
    // Connect to the driver
    std::cout << "Connecting to RTCore64 driver..." << std::endl;
    if (g_RTCore64.Connect()) {
        std::cout << "Successfully connected to RTCore64 driver" << std::endl;
        
        // Test a simple memory read
        std::cout << "Testing memory read at 0x1000..." << std::endl;
        DWORD value = 0;
        if (g_RTCore64.ReadMemory(0x1000, &value, sizeof(value))) {
            std::cout << "Read successful! Value: 0x" << std::hex << value << std::dec << std::endl;
        } else {
            std::cout << "Read failed!" << std::endl;
            
            // Get error information
            RTCore64Error error = g_RTCore64ErrorHandler.GetLastError();
            std::cout << "Error: " << g_RTCore64ErrorHandler.FormatErrorMessage(error) << std::endl;
        }
        
        // Disconnect
        g_RTCore64.Disconnect();
        std::cout << "Disconnected from driver" << std::endl;
    } else {
        std::cout << "Failed to connect to RTCore64 driver" << std::endl;
        
        // Get error information
        RTCore64Error error = g_RTCore64ErrorHandler.GetLastError();
        std::cout << "Error: " << g_RTCore64ErrorHandler.FormatErrorMessage(error) << std::endl;
    }
    
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}

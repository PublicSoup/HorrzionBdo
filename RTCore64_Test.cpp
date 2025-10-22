#include "pch.h"
#include "BDO_RTCore64_Driver.h"

int rtcore_test_main(int argc, char* argv[]) {
    std::cout << "RTCore64 Test Program" << std::endl;
    
    // Connect to RTCore64 driver
    if (!g_RTCore64.Connect()) {
        std::cout << "Failed to connect to RTCore64 driver" << std::endl;
        std::cout << "Make sure the driver is loaded (sc query RTCore64)" << std::endl;
        return 1;
    }
    
    std::cout << "Connected to RTCore64 driver" << std::endl;
    
    // Test kernel memory read
    ULONG64 testAddress = 0x1000; // Safe low address
    DWORD testValue = 0;
    
    if (g_RTCore64.ReadMemory(testAddress, &testValue, sizeof(testValue))) {
        std::cout << "Test read successful: 0x" << std::hex << testValue << std::dec << std::endl;
    } else {
        std::cout << "Test read failed" << std::endl;
    }
    
    // Disconnect
    g_RTCore64.Disconnect();
    std::cout << "Disconnected from driver" << std::endl;
    
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}

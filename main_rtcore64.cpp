#include <Windows.h>
#include <iostream>
#include <string>
#include "BDO_MemoryResolver.h"
#include "BDO_RTCore64_Integration.h"
#include "BDO_RTCore64_Patch.cpp"

// Main entry point for RTCore64 version
int main(int argc, char* argv[]) {
    std::cout << "=== BDO Stealth Bot (RTCore64 Version) ===" << std::endl;
    std::cout << "Initializing..." << std::endl;
    
    // Process command line arguments
    bool testMode = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0) {
            testMode = true;
        }
    }
    
    // Initialize RTCore64
    if (!RTCore64Hooks::Initialize()) {
        std::cout << "Failed to initialize RTCore64" << std::endl;
        std::cout << "Make sure the driver is loaded (sc query RTCore64)" << std::endl;
        return 1;
    }
    
    std::cout << "RTCore64 initialized successfully" << std::endl;
    
    // Test mode
    if (testMode) {
        std::cout << "Running in test mode..." << std::endl;
        
        // Test kernel memory read
        ULONG64 testAddress = 0x1000; // Safe low address
        DWORD testValue = 0;
        
        if (RTCore64Hooks::ReadMemory((PVOID)testAddress, &testValue, sizeof(testValue))) {
            std::cout << "Test read successful: 0x" << std::hex << testValue << std::dec << std::endl;
        } else {
            std::cout << "Test read failed" << std::endl;
        }
        
        // Clean up and exit
        RTCore64Hooks::Shutdown();
        return 0;
    }
    
    // Initialize memory resolver
    BDOMemoryResolver resolver;
    
    // Attach to BDO process
    if (!resolver.AttachToProcess(L"BlackDesert64.exe")) {
        std::cout << "Failed to attach to BDO process" << std::endl;
        std::cout << "Make sure the game is running" << std::endl;
        RTCore64Hooks::Shutdown();
        return 1;
    }
    
    std::cout << "Attached to BDO process" << std::endl;
    
    // Resolve BDO offsets
    BDOMemoryResolver::BDOOffsets offsets;
    if (!resolver.ResolveBDOOffsets(offsets)) {
        std::cout << "Failed to resolve BDO offsets" << std::endl;
        RTCore64Hooks::Shutdown();
        return 1;
    }
    
    std::cout << "BDO offsets resolved successfully" << std::endl;
    
    // Main loop
    std::cout << "BDO Stealth Bot ready!" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    
    while (true) {
        // Your bot logic here
        
        // Sleep to avoid high CPU usage
        Sleep(100);
        
        // Check for exit
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }
    }
    
    // Clean up
    resolver.DetachFromProcess();
    RTCore64Hooks::Shutdown();
    
    std::cout << "BDO Stealth Bot exited" << std::endl;
    return 0;
}

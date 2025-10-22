#include "BDO_MemoryResolver.h"
#include "BDO_RTCore64_Integration.h"

// This file patches the existing memory system to use RTCore64

// Hook into the memory resolver's initialization
bool PatchMemoryResolver() {
    // Initialize RTCore64 connection
    if (!g_RTCore64.Connect()) {
        std::cerr << "Failed to connect to RTCore64 driver" << std::endl;
        std::cerr << "Make sure the driver is loaded (sc query RTCore64)" << std::endl;
        return false;
    }
    
    std::cout << "Connected to RTCore64 driver successfully" << std::endl;
    return true;
}

// These functions should be called from your main application
namespace RTCore64Hooks {
    // Hook into the memory read function
    bool ReadMemory(PVOID address, PVOID buffer, SIZE_T size) {
        // Try RTCore64 first
        if (g_RTCore64.IsConnected()) {
            return g_RTCore64.ReadProcessMemory(0, (ULONG64)address, buffer, size);
        }
        
        // Fall back to default implementation
        return false;
    }
    
    // Hook into the memory write function
    bool WriteMemory(PVOID address, PVOID buffer, SIZE_T size) {
        // Try RTCore64 first
        if (g_RTCore64.IsConnected()) {
            return g_RTCore64.WriteProcessMemory(0, (ULONG64)address, buffer, size);
        }
        
        // Fall back to default implementation
        return false;
    }
    
    // Initialize RTCore64
    bool Initialize() {
        return PatchMemoryResolver();
    }
    
    // Shutdown RTCore64
    void Shutdown() {
        g_RTCore64.Disconnect();
    }
}

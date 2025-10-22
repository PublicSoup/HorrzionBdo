/*
 * BDO_SimpleIntegration.cpp
 * Standalone integration example - no dependencies on existing code
 * Shows how to integrate BDO_SecureClient into any project
 */

#include "BDO_SecureClient.h"
#include <iostream>

class SimpleBDOMemory {
private:
    SecureKernelClient client;
    DWORD processId;
    ULONG64 processBase;
    bool initialized;
    
public:
    SimpleBDOMemory() : processId(0), processBase(0), initialized(false) {}
    
    bool Initialize(DWORD pid) {
        std::cout << "[SimpleBDO] Initializing...\n";
        
        processId = pid;
        
        // Initialize secure kernel
        if (!client.Initialize()) {
            std::cerr << "[SimpleBDO] Failed: " << client.GetLastErrorString() << "\n";
            return false;
        }
        
        std::cout << "[SimpleBDO] Secure kernel connected\n";
        
        // Enable stealth
        client.EnableDriverHiding();
        client.RemoveXigncodeCallbacks();
        
        // Get process base
        if (!client.GetProcessBaseAddress(pid, processBase)) {
            std::cerr << "[SimpleBDO] Failed to get base address\n";
            client.Shutdown();
            return false;
        }
        
        std::cout << "[SimpleBDO] Base: 0x" << std::hex << processBase << std::dec << "\n";
        initialized = true;
        return true;
    }
    
    void Shutdown() {
        if (initialized) {
            client.Shutdown();
            initialized = false;
        }
    }
    
    // Simple read/write wrappers
    template<typename T>
    bool Read(ULONG64 address, T& value) {
        if (!initialized) return false;
        return client.Read<T>(processId, address, value);
    }
    
    template<typename T>
    bool Write(ULONG64 address, const T& value) {
        if (!initialized) return false;
        return client.Write<T>(processId, address, value);
    }
    
    // Pointer chain reading
    bool ReadPointerChain(const std::vector<ULONG64>& offsets, PVOID buffer, SIZE_T size) {
        if (offsets.empty()) return false;
        
        ULONG64 currentAddr = processBase + offsets[0];
        
        // Follow pointer chain
        for (size_t i = 1; i < offsets.size(); i++) {
            ULONG64 pointer = 0;
            if (!Read<ULONG64>(currentAddr, pointer)) {
                return false;
            }
            
            if (pointer == 0) {
                return false; // Null pointer in chain
            }
            
            currentAddr = pointer + offsets[i];
        }
        
        // Read final value
        return client.ReadMemory(processId, currentAddr, buffer, size);
    }
    
    // BDO-specific helpers (with placeholder offsets)
    bool ReadPlayerHealth(int& hp, int& maxHp) {
        // Example: [[base + 0x5A8B1C8] + 0x18] + 0x30 = player
        // Player + 0x28 = HP, Player + 0x2C = MaxHP
        
        std::vector<ULONG64> hpOffsets = {0x5A8B1C8, 0x18, 0x30 + 0x28};
        std::vector<ULONG64> maxHpOffsets = {0x5A8B1C8, 0x18, 0x30 + 0x2C};
        
        if (!ReadPointerChain(hpOffsets, &hp, sizeof(hp))) return false;
        if (!ReadPointerChain(maxHpOffsets, &maxHp, sizeof(maxHp))) return false;
        
        return true;
    }
    
    ULONG64 GetProcessBase() const { return processBase; }
    DWORD GetProcessId() const { return processId; }
    bool IsInitialized() const { return initialized; }
};

// Example usage
int main() {
    std::cout << "========================================\n";
    std::cout << "Simple BDO Memory Integration Example\n";
    std::cout << "========================================\n\n";
    
    // This demonstrates standalone usage without BDO_AdvancedMemory dependencies
    
    DWORD bdoPid = 12345; // Get from Task Manager
    
    SimpleBDOMemory bdo;
    if (!bdo.Initialize(bdoPid)) {
        return 1;
    }
    
    std::cout << "\nReady to read BDO memory!\n";
    std::cout << "Process Base: 0x" << std::hex << bdo.GetProcessBase() << std::dec << "\n\n";
    
    // Example: Read a value
    int testValue = 0;
    if (bdo.Read<int>(bdo.GetProcessBase() + 0x1000, testValue)) {
        std::cout << "Successfully read value: " << testValue << "\n";
    }
    
    // Example: Try to read player health (will fail with placeholder offsets)
    int hp = 0, maxHp = 0;
    if (bdo.ReadPlayerHealth(hp, maxHp)) {
        std::cout << "HP: " << hp << "/" << maxHp << "\n";
    } else {
        std::cout << "HP read failed (offsets are placeholders)\n";
    }
    
    bdo.Shutdown();
    
    std::cout << "\nIntegration example complete\n";
    return 0;
}


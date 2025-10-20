#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include "BDO_AdvancedMemory.h"
#include "BDO_MemoryScanner.h"

// Test program to demonstrate memory scanning and value manipulation features

void PrintSeparator() {
    std::cout << "========================================" << std::endl;
}

void PrintResults(const std::vector<BDOMemoryScanner::ScanResult>& results, size_t maxDisplay = 10) {
    std::cout << "Found " << results.size() << " results" << std::endl;
    
    size_t displayCount = std::min(results.size(), maxDisplay);
    for (size_t i = 0; i < displayCount; ++i) {
        const auto& result = results[i];
        std::cout << std::setfill('0') << std::setw(16) << std::hex << reinterpret_cast<uintptr_t>(result.address);
        std::cout << " : ";
        
        // Display value based on type
        std::visit([](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "\"" << val << "\"";
            } else if constexpr (std::is_same_v<T, std::vector<BYTE>>) {
                for (BYTE b : val) {
                    std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b) << " ";
                }
            } else {
                std::cout << std::dec << val;
            }
        }, result.value);
        
        std::cout << std::endl;
    }
    
    if (results.size() > maxDisplay) {
        std::cout << "... and " << (results.size() - maxDisplay) << " more results" << std::endl;
    }
}

void TestBasicScanning() {
    std::cout << "\n=== Basic Memory Scanning Test ===" << std::endl;
    
    BDOAdvancedMemory memory;
    
    // Test variables in our process
    int testInt = 12345;
    float testFloat = 3.14159f;
    double testDouble = 2.71828;
    std::string testString = "BDO Memory Test";
    
    std::cout << "Test variables created:" << std::endl;
    std::cout << "Integer: " << testInt << " at 0x" << std::hex << &testInt << std::endl;
    std::cout << "Float: " << std::dec << testFloat << " at 0x" << std::hex << &testFloat << std::endl;
    std::cout << "Double: " << std::dec << testDouble << " at 0x" << std::hex << &testDouble << std::endl;
    std::cout << "String: \"" << testString << "\" at 0x" << std::hex << testString.data() << std::endl;
    
    // Initialize memory system with current process
    if (!memory.Initialize(GetCurrentProcessId())) {
        std::cout << "Failed to initialize memory system" << std::endl;
        return;
    }
    
    PrintSeparator();
    
    // Scan for integer
    std::cout << "\nScanning for integer value: " << std::dec << testInt << std::endl;
    if (memory.ScanForValue(ValueType::Dword, std::to_string(testInt))) {
        auto results = memory.GetScanResults();
        PrintResults(results);
        
        // Verify we found our test variable
        bool foundOurVariable = false;
        for (const auto& result : results) {
            if (result.address == &testInt) {
                std::cout << "Successfully found our test variable!" << std::endl;
                foundOurVariable = true;
                break;
            }
        }
        
        if (!foundOurVariable) {
            std::cout << "Warning: Did not find our test variable in results" << std::endl;
        }
    }
    
    PrintSeparator();
    
    // Test changed/unchanged scanning
    std::cout << "\nTesting changed value scanning..." << std::endl;
    std::cout << "Creating snapshot before change..." << std::endl;
    memory.CreateMemorySnapshot("before_change");
    
    // Change the value
    int oldValue = testInt;
    testInt = 54321;
    std::cout << "Changed integer from " << oldValue << " to " << testInt << std::endl;
    
    // Scan for changed values
    std::cout << "Scanning for changed values..." << std::endl;
    if (memory.CompareSnapshot("before_change", ScanType::Changed)) {
        auto results = memory.GetScanResults();
        std::cout << "Found " << results.size() << " changed values" << std::endl;
    }
    
    // Test value freezing
    PrintSeparator();
    std::cout << "\nTesting value freezing..." << std::endl;
    
    int freezeTest = 100;
    std::cout << "Original value: " << freezeTest << " at 0x" << std::hex << &freezeTest << std::endl;
    
    // Freeze the value
    if (memory.FreezeValue(&freezeTest, ValueType::Dword, "200")) {
        std::cout << "Value frozen to 200" << std::endl;
        
        // Try to change it
        freezeTest = 300;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "After attempting to set to 300, value is: " << std::dec << freezeTest << std::endl;
        
        // Unfreeze
        memory.UnfreezeValue(&freezeTest);
        std::cout << "Value unfrozen" << std::endl;
        
        freezeTest = 400;
        std::cout << "After unfreezing and setting to 400, value is: " << freezeTest << std::endl;
    }
}

void TestValueMonitoring() {
    std::cout << "\n=== Value Monitoring Test ===" << std::endl;
    
    BDOAdvancedMemory memory;
    if (!memory.Initialize(GetCurrentProcessId())) {
        std::cout << "Failed to initialize memory system" << std::endl;
        return;
    }
    
    // Create a variable that changes over time
    int changingValue = 0;
    std::cout << "Monitoring value at 0x" << std::hex << &changingValue << std::endl;
    
    // Start monitoring
    memory.MonitorValue(&changingValue, ValueType::Dword);
    
    // Change the value several times
    for (int i = 0; i < 10; ++i) {
        changingValue = i * 10;
        std::cout << "Value changed to: " << std::dec << changingValue << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Get value history
    auto history = memory.GetValueHistory(&changingValue);
    std::cout << "\nValue history (" << history.size() << " entries):" << std::endl;
    
    for (const auto& val : history) {
        std::visit([](auto&& v) {
            std::cout << "  " << v << std::endl;
        }, val);
    }
    
    memory.StopMonitoringValue(&changingValue);
}

void TestMemoryStructureAnalysis() {
    std::cout << "\n=== Memory Structure Analysis Test ===" << std::endl;
    
    // Create a test structure
    struct TestStruct {
        int health;
        float mana;
        double stamina;
        char name[32];
        PVOID nextPtr;
    };
    
    TestStruct testData = {
        100,        // health
        75.5f,      // mana
        90.25,      // stamina
        "TestPlayer", // name
        nullptr     // nextPtr
    };
    
    std::cout << "Test structure at 0x" << std::hex << &testData << std::endl;
    std::cout << "Size: " << std::dec << sizeof(TestStruct) << " bytes" << std::endl;
    std::cout << "Members:" << std::endl;
    std::cout << "  health: " << testData.health << " (offset: 0x" << std::hex << offsetof(TestStruct, health) << ")" << std::endl;
    std::cout << "  mana: " << std::dec << testData.mana << " (offset: 0x" << std::hex << offsetof(TestStruct, mana) << ")" << std::endl;
    std::cout << "  stamina: " << std::dec << testData.stamina << " (offset: 0x" << std::hex << offsetof(TestStruct, stamina) << ")" << std::endl;
    std::cout << "  name: \"" << testData.name << "\" (offset: 0x" << std::hex << offsetof(TestStruct, name) << ")" << std::endl;
    std::cout << "  nextPtr: 0x" << std::hex << testData.nextPtr << " (offset: 0x" << std::hex << offsetof(TestStruct, nextPtr) << ")" << std::endl;
    
    BDOAdvancedMemory memory;
    if (!memory.Initialize(GetCurrentProcessId())) {
        std::cout << "Failed to initialize memory system" << std::endl;
        return;
    }
    
    // Analyze the structure
    BDOMemoryScanner::StructureInfo info;
    if (memory.AnalyzeMemoryStructure(&testData, sizeof(TestStruct), info)) {
        std::cout << "\nStructure analysis results:" << std::endl;
        std::cout << "Detected size: " << info.size << " bytes" << std::endl;
        
        for (const auto& [offset, type] : info.memberTypes) {
            std::cout << "  Offset 0x" << std::hex << offset << ": ";
            std::cout << memory.GetScanResults()[0].scanner->GetValueTypeString(type) << std::endl;
        }
    }
}

void TestPointerChains() {
    std::cout << "\n=== Pointer Chain Test ===" << std::endl;
    
    // Create a chain of pointers
    int finalValue = 42;
    int* level3 = &finalValue;
    int** level2 = &level3;
    int*** level1 = &level2;
    int**** basePtr = &level1;
    
    std::cout << "Pointer chain:" << std::endl;
    std::cout << "Base: 0x" << std::hex << basePtr << std::endl;
    std::cout << "  -> 0x" << std::hex << level1 << std::endl;
    std::cout << "    -> 0x" << std::hex << level2 << std::endl;
    std::cout << "      -> 0x" << std::hex << level3 << std::endl;
    std::cout << "        -> 0x" << std::hex << &finalValue << " = " << std::dec << finalValue << std::endl;
    
    BDOAdvancedMemory memory;
    if (!memory.Initialize(GetCurrentProcessId())) {
        std::cout << "Failed to initialize memory system" << std::endl;
        return;
    }
    
    // Find pointer chain
    std::vector<SIZE_T> offsets;
    if (memory.FindPointerChain(basePtr, &finalValue, offsets)) {
        std::cout << "\nFound pointer chain with " << offsets.size() << " offsets:" << std::endl;
        for (size_t i = 0; i < offsets.size(); ++i) {
            std::cout << "  [+" << std::hex << offsets[i] << "]" << std::endl;
        }
    }
}

void TestMemoryProtection() {
    std::cout << "\n=== Memory Protection Test ===" << std::endl;
    
    BDOAdvancedMemory memory;
    if (!memory.Initialize(GetCurrentProcessId())) {
        std::cout << "Failed to initialize memory system" << std::endl;
        return;
    }
    
    int testValue = 100;
    PVOID address = &testValue;
    
    std::cout << "Test value at 0x" << std::hex << address << std::endl;
    
    // Check memory properties
    std::cout << "Is executable: " << (memory.IsAddressExecutable(address) ? "Yes" : "No") << std::endl;
    std::cout << "Is writable: " << (memory.IsAddressWritable(address) ? "Yes" : "No") << std::endl;
    std::cout << "Region size: 0x" << std::hex << memory.GetMemoryRegionSize(address) << " bytes" << std::endl;
    std::cout << "Protection: 0x" << std::hex << memory.GetMemoryProtection(address) << std::endl;
}

void ShowMenu() {
    std::cout << "\n=== BDO Memory Scanner Test Menu ===" << std::endl;
    std::cout << "1. Test Basic Scanning" << std::endl;
    std::cout << "2. Test Value Monitoring" << std::endl;
    std::cout << "3. Test Memory Structure Analysis" << std::endl;
    std::cout << "4. Test Pointer Chains" << std::endl;
    std::cout << "5. Test Memory Protection" << std::endl;
    std::cout << "6. Run All Tests" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Choice: ";
}

int main() {
    std::cout << "BDO Advanced Memory Scanner Test Program" << std::endl;
    std::cout << "========================================" << std::endl;
    
    bool running = true;
    while (running) {
        ShowMenu();
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                TestBasicScanning();
                break;
            case 2:
                TestValueMonitoring();
                break;
            case 3:
                TestMemoryStructureAnalysis();
                break;
            case 4:
                TestPointerChains();
                break;
            case 5:
                TestMemoryProtection();
                break;
            case 6:
                TestBasicScanning();
                TestValueMonitoring();
                TestMemoryStructureAnalysis();
                TestPointerChains();
                TestMemoryProtection();
                break;
            case 0:
                running = false;
                break;
            default:
                std::cout << "Invalid choice!" << std::endl;
                break;
        }
        
        if (running) {
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore();
            std::cin.get();
        }
    }
    
    std::cout << "Exiting..." << std::endl;
    return 0;
}

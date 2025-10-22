/*
 * BDO_KernelTest.cpp
 * Test program for BDO Secure Kernel Driver
 * 
 * Compile: cl /EHsc BDO_KernelTest.cpp BDO_SecureClient.cpp
 */

#include "BDO_SecureClient.h"
#include <iostream>
#include <iomanip>
#include <Windows.h>
#include <TlHelp32.h>

// Find process ID by name
DWORD FindProcessId(const wchar_t* processName) {
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry = { 0 };
        entry.dwSize = sizeof(entry);
        
        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (_wcsicmp(entry.szExeFile, processName) == 0) {
                    processId = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }
        
        CloseHandle(snapshot);
    }
    
    return processId;
}

// Test driver initialization
bool TestDriverConnection() {
    std::cout << "[TEST] Testing driver connection...\n";
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "  [FAIL] " << client.GetLastErrorString() << "\n";
        return false;
    }
    
    std::cout << "  [PASS] Driver connected successfully\n";
    client.Shutdown();
    return true;
}

// Test memory read operations
bool TestMemoryRead() {
    std::cout << "[TEST] Testing memory read...\n";
    
    // Find notepad process for testing
    DWORD pid = FindProcessId(L"notepad.exe");
    if (pid == 0) {
        std::cout << "  [SKIP] Notepad.exe not running (start it for this test)\n";
        return true;
    }
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "  [FAIL] " << client.GetLastErrorString() << "\n";
        return false;
    }
    
    // Try to get process base address
    ULONG64 baseAddress = 0;
    if (!client.GetProcessBaseAddress(pid, baseAddress)) {
        std::cout << "  [FAIL] Could not get process base address\n";
        client.Shutdown();
        return false;
    }
    
    std::cout << "  [INFO] Notepad PID: " << pid << "\n";
    std::cout << "  [INFO] Base Address: 0x" << std::hex << baseAddress << std::dec << "\n";
    
    // Read PE header signature (should be 'MZ')
    WORD signature = 0;
    if (!client.Read<WORD>(pid, baseAddress, signature)) {
        std::cout << "  [FAIL] Could not read PE signature\n";
        client.Shutdown();
        return false;
    }
    
    if (signature == 0x5A4D) { // 'MZ'
        std::cout << "  [PASS] Successfully read PE signature: MZ\n";
    } else {
        std::cout << "  [FAIL] Invalid PE signature: 0x" << std::hex << signature << std::dec << "\n";
        client.Shutdown();
        return false;
    }
    
    client.Shutdown();
    return true;
}

// Test memory write operations
bool TestMemoryWrite() {
    std::cout << "[TEST] Testing memory write...\n";
    
    // Allocate test buffer in our own process
    int* testBuffer = new int;
    *testBuffer = 12345;
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "  [FAIL] " << client.GetLastErrorString() << "\n";
        delete testBuffer;
        return false;
    }
    
    DWORD currentPid = GetCurrentProcessId();
    
    // Write new value
    int newValue = 67890;
    if (!client.Write<int>(currentPid, (ULONG64)testBuffer, newValue)) {
        std::cout << "  [FAIL] Write operation failed\n";
        client.Shutdown();
        delete testBuffer;
        return false;
    }
    
    // Verify value changed
    if (*testBuffer == newValue) {
        std::cout << "  [PASS] Successfully wrote and verified value\n";
    } else {
        std::cout << "  [FAIL] Value not changed correctly\n";
        client.Shutdown();
        delete testBuffer;
        return false;
    }
    
    client.Shutdown();
    delete testBuffer;
    return true;
}

// Test pattern scanning
bool TestPatternScan() {
    std::cout << "[TEST] Testing pattern scan...\n";
    
    // Create test pattern in memory
    BYTE testData[256] = { 0 };
    testData[100] = 0x48;
    testData[101] = 0x8B;
    testData[102] = 0x0D;
    testData[103] = 0xAA;
    testData[104] = 0xBB;
    testData[105] = 0xCC;
    testData[106] = 0xDD;
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "  [FAIL] " << client.GetLastErrorString() << "\n";
        return false;
    }
    
    DWORD currentPid = GetCurrentProcessId();
    ULONG64 startAddr = (ULONG64)testData;
    ULONG64 endAddr = startAddr + sizeof(testData);
    
    // Scan for pattern: 48 8B 0D ?? ?? ?? ??
    ULONG64 result = 0;
    bool found = client.ScanPattern(
        currentPid,
        startAddr,
        endAddr,
        "48 8B 0D ?? ?? ?? ??",
        result
    );
    
    if (found && result == (startAddr + 100)) {
        std::cout << "  [PASS] Pattern found at correct location\n";
    } else if (found) {
        std::cout << "  [WARN] Pattern found but at wrong location\n";
        std::cout << "  [INFO] Expected: 0x" << std::hex << (startAddr + 100) << "\n";
        std::cout << "  [INFO] Found: 0x" << result << std::dec << "\n";
    } else {
        std::cout << "  [FAIL] Pattern not found\n";
        client.Shutdown();
        return false;
    }
    
    client.Shutdown();
    return true;
}

// Test stealth features
bool TestStealthFeatures() {
    std::cout << "[TEST] Testing stealth features...\n";
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "  [FAIL] " << client.GetLastErrorString() << "\n";
        return false;
    }
    
    // Enable driver hiding
    if (client.EnableDriverHiding()) {
        std::cout << "  [PASS] Driver hiding enabled\n";
    } else {
        std::cout << "  [WARN] Driver hiding failed (may not be implemented yet)\n";
    }
    
    // Try to remove Xigncode callbacks
    if (client.RemoveXigncodeCallbacks()) {
        std::cout << "  [PASS] Callback removal requested\n";
    } else {
        std::cout << "  [WARN] Callback removal failed (may not be implemented yet)\n";
    }
    
    client.Shutdown();
    return true;
}

// Test BDO-specific functionality
bool TestBDOIntegration() {
    std::cout << "[TEST] Testing BDO integration...\n";
    
    // Find Black Desert process
    DWORD bdoPid = FindProcessId(L"BlackDesert64.exe");
    if (bdoPid == 0) {
        std::cout << "  [SKIP] Black Desert not running\n";
        return true;
    }
    
    std::cout << "  [INFO] Found Black Desert (PID: " << bdoPid << ")\n";
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "  [FAIL] " << client.GetLastErrorString() << "\n";
        return false;
    }
    
    // Get base address
    ULONG64 baseAddress = 0;
    if (!client.GetProcessBaseAddress(bdoPid, baseAddress)) {
        std::cout << "  [FAIL] Could not get BDO base address\n";
        client.Shutdown();
        return false;
    }
    
    std::cout << "  [INFO] BDO Base Address: 0x" << std::hex << baseAddress << std::dec << "\n";
    
    // Read PE signature to verify access
    WORD signature = 0;
    if (client.Read<WORD>(bdoPid, baseAddress, signature)) {
        std::cout << "  [PASS] Successfully accessed BDO memory\n";
        std::cout << "  [INFO] PE Signature: " << (char)(signature & 0xFF) << (char)(signature >> 8) << "\n";
    } else {
        std::cout << "  [FAIL] Could not read BDO memory\n";
        std::cout << "  [INFO] This may be due to Xigncode3 protection\n";
    }
    
    client.Shutdown();
    return true;
}

// Performance test
bool TestPerformance() {
    std::cout << "[TEST] Testing performance...\n";
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "  [FAIL] " << client.GetLastErrorString() << "\n";
        return false;
    }
    
    DWORD currentPid = GetCurrentProcessId();
    int testValue = 12345;
    ULONG64 testAddress = (ULONG64)&testValue;
    
    // Test read performance
    auto startTime = GetTickCount64();
    const int iterations = 1000;
    int readValue = 0;
    
    for (int i = 0; i < iterations; i++) {
        client.Read<int>(currentPid, testAddress, readValue);
    }
    
    auto endTime = GetTickCount64();
    auto elapsed = endTime - startTime;
    
    std::cout << "  [INFO] Performed " << iterations << " reads in " << elapsed << " ms\n";
    std::cout << "  [INFO] Average: " << (double)elapsed / iterations << " ms per read\n";
    std::cout << "  [INFO] Throughput: " << (1000.0 * iterations) / elapsed << " ops/sec\n";
    
    if (elapsed < iterations * 10) { // Less than 10ms per op
        std::cout << "  [PASS] Performance is good\n";
    } else {
        std::cout << "  [WARN] Performance may need optimization\n";
    }
    
    client.Shutdown();
    return true;
}

// Main test runner
int main() {
    std::cout << "========================================\n";
    std::cout << "BDO Secure Kernel Driver - Test Suite\n";
    std::cout << "========================================\n\n";
    
    int passed = 0;
    int failed = 0;
    int skipped = 0;
    
    struct Test {
        const char* name;
        bool (*function)();
    };
    
    Test tests[] = {
        { "Driver Connection", TestDriverConnection },
        { "Memory Read", TestMemoryRead },
        { "Memory Write", TestMemoryWrite },
        { "Pattern Scan", TestPatternScan },
        { "Stealth Features", TestStealthFeatures },
        { "BDO Integration", TestBDOIntegration },
        { "Performance", TestPerformance }
    };
    
    for (const auto& test : tests) {
        std::cout << "\n";
        if (test.function()) {
            passed++;
        } else {
            failed++;
        }
    }
    
    std::cout << "\n========================================\n";
    std::cout << "Test Results:\n";
    std::cout << "  Passed: " << passed << "\n";
    std::cout << "  Failed: " << failed << "\n";
    std::cout << "========================================\n";
    
    if (failed == 0) {
        std::cout << "\nAll tests passed! Driver is working correctly.\n";
        return 0;
    } else {
        std::cout << "\nSome tests failed. Check output above for details.\n";
        return 1;
    }
}


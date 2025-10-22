/*
 * BDO_StressTest.cpp
 * Stress test for BDO Secure Kernel driver
 * Tests high load, concurrent access, and error handling
 */

#include "BDO_SecureClient.h"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <Windows.h>

std::atomic<int> totalOperations(0);
std::atomic<int> failedOperations(0);
std::atomic<bool> running(true);

// Worker thread that hammers the driver
void StressWorker(int threadId, DWORD targetPid, ULONG64 testAddress) {
    SecureKernelClient client;
    
    if (!client.Initialize()) {
        std::cerr << "[Thread " << threadId << "] Failed to initialize\n";
        return;
    }
    
    std::cout << "[Thread " << threadId << "] Started\n";
    
    int localOps = 0;
    int localFails = 0;
    
    while (running) {
        // Rapid read test
        int value = 0;
        if (client.Read<int>(targetPid, testAddress, value)) {
            localOps++;
        } else {
            localFails++;
        }
        
        // Small delay to prevent complete CPU saturation
        if (localOps % 100 == 0) {
            Sleep(1);
        }
    }
    
    totalOperations += localOps;
    failedOperations += localFails;
    
    client.Shutdown();
    std::cout << "[Thread " << threadId << "] Stopped - " << localOps << " ops\n";
}

// Test invalid inputs
void TestErrorHandling(DWORD pid) {
    SecureKernelClient client;
    client.Initialize();
    
    std::cout << "\n[Error Handling Tests]\n";
    
    // Test 1: Invalid address (kernel space)
    std::cout << "[1] Testing kernel space address... ";
    int value = 0;
    if (!client.Read<int>(pid, 0xFFFFF80000000000, value)) {
        std::cout << "PASS (correctly rejected)\n";
    } else {
        std::cout << "FAIL (should have rejected)\n";
    }
    
    // Test 2: Null address
    std::cout << "[2] Testing null address... ";
    if (!client.Read<int>(pid, 0, value)) {
        std::cout << "PASS (correctly rejected)\n";
    } else {
        std::cout << "FAIL (should have rejected)\n";
    }
    
    // Test 3: Huge size
    std::cout << "[3] Testing huge size... ";
    std::vector<BYTE> hugeBuffer(100 * 1024 * 1024); // 100MB
    if (!client.ReadMemory(pid, 0x1000, hugeBuffer.data(), hugeBuffer.size())) {
        std::cout << "PASS (correctly rejected)\n";
    } else {
        std::cout << "FAIL (should have rejected)\n";
    }
    
    // Test 4: Invalid PID
    std::cout << "[4] Testing invalid PID... ";
    if (!client.Read<int>(0xFFFFFFFF, 0x1000, value)) {
        std::cout << "PASS (correctly rejected)\n";
    } else {
        std::cout << "FAIL (should have rejected)\n";
    }
    
    client.Shutdown();
}

// Test concurrent access
void TestConcurrency() {
    std::cout << "\n[Concurrent Access Test]\n";
    std::cout << "Starting 8 threads for 10 seconds...\n\n";
    
    DWORD currentPid = GetCurrentProcessId();
    int testValue = 12345;
    ULONG64 testAddr = (ULONG64)&testValue;
    
    running = true;
    totalOperations = 0;
    failedOperations = 0;
    
    std::vector<std::thread> threads;
    
    // Start 8 worker threads
    for (int i = 0; i < 8; i++) {
        threads.emplace_back(StressWorker, i, currentPid, testAddr);
    }
    
    // Run for 10 seconds
    auto startTime = std::chrono::steady_clock::now();
    while (true) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime
        ).count();
        
        if (elapsed >= 10) break;
        
        std::cout << "\r[" << elapsed << "s] Operations: " << totalOperations 
                  << " | Failed: " << failedOperations 
                  << " | Rate: " << (totalOperations / (elapsed + 1)) << " ops/sec"
                  << std::flush;
        
        Sleep(100);
    }
    
    std::cout << "\n\nStopping threads...\n";
    running = false;
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "\n[Results]\n";
    std::cout << "Total operations: " << totalOperations << "\n";
    std::cout << "Failed operations: " << failedOperations << "\n";
    std::cout << "Success rate: " << (100.0 * (totalOperations - failedOperations) / totalOperations) << "%\n";
}

// Test large transfers
void TestLargeTransfers() {
    std::cout << "\n[Large Transfer Test]\n";
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "Failed to initialize\n";
        return;
    }
    
    DWORD currentPid = GetCurrentProcessId();
    
    // Test various sizes
    SIZE_T sizes[] = { 1024, 4096, 64*1024, 1024*1024, 16*1024*1024 };
    
    for (SIZE_T size : sizes) {
        std::vector<BYTE> buffer(size);
        
        // Fill with pattern
        for (SIZE_T i = 0; i < size; i++) {
            buffer[i] = (BYTE)(i & 0xFF);
        }
        
        ULONG64 addr = (ULONG64)buffer.data();
        
        std::cout << "Testing " << (size / 1024) << " KB... ";
        
        std::vector<BYTE> readBuffer(size);
        auto start = std::chrono::high_resolution_clock::now();
        
        if (client.ReadMemory(currentPid, addr, readBuffer.data(), size)) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            
            // Verify data
            bool correct = (memcmp(buffer.data(), readBuffer.data(), size) == 0);
            
            std::cout << (correct ? "PASS" : "FAIL") 
                      << " (" << duration << " us, " 
                      << (size / 1024.0 / 1024.0) / (duration / 1000000.0) << " MB/s)\n";
        } else {
            std::cout << "FAIL (read error)\n";
        }
    }
    
    client.Shutdown();
}

// Test pattern scanning under load
void TestPatternScanning() {
    std::cout << "\n[Pattern Scanning Test]\n";
    
    SecureKernelClient client;
    if (!client.Initialize()) {
        std::cout << "Failed to initialize\n";
        return;
    }
    
    DWORD currentPid = GetCurrentProcessId();
    
    // Create test data with patterns
    std::vector<BYTE> testData(1024 * 1024); // 1MB
    for (SIZE_T i = 0; i < testData.size(); i++) {
        testData[i] = (BYTE)(rand() % 256);
    }
    
    // Insert known pattern at offset 500000
    BYTE pattern[] = { 0x48, 0x8B, 0x0D, 0xAA, 0xBB, 0xCC, 0xDD };
    memcpy(&testData[500000], pattern, sizeof(pattern));
    
    ULONG64 startAddr = (ULONG64)testData.data();
    ULONG64 endAddr = startAddr + testData.size();
    
    std::cout << "Scanning 1MB for pattern... ";
    
    ULONG64 result = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    bool found = client.ScanPattern(
        currentPid,
        startAddr,
        endAddr,
        "48 8B 0D ?? ?? ?? ??",
        result
    );
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    if (found && result == (startAddr + 500000)) {
        std::cout << "PASS (" << duration << " ms)\n";
    } else {
        std::cout << "FAIL (expected 0x" << std::hex << (startAddr + 500000) 
                  << ", got 0x" << result << std::dec << ")\n";
    }
    
    client.Shutdown();
}

int main() {
    std::cout << "========================================\n";
    std::cout << "BDO Secure Kernel - Stress Test\n";
    std::cout << "========================================\n\n";
    
    std::cout << "WARNING: This test will heavily load the driver\n";
    std::cout << "Monitor system stability and watch for BSODs\n";
    std::cout << "Press Enter to continue...\n";
    std::cin.get();
    
    // Test 1: Error handling
    TestErrorHandling(GetCurrentProcessId());
    
    // Test 2: Large transfers
    TestLargeTransfers();
    
    // Test 3: Pattern scanning
    TestPatternScanning();
    
    // Test 4: Concurrent stress
    TestConcurrency();
    
    std::cout << "\n========================================\n";
    std::cout << "Stress Test Complete\n";
    std::cout << "========================================\n";
    std::cout << "\nMonitor system for:\n";
    std::cout << "1. Memory leaks (use PoolMon)\n";
    std::cout << "2. Handle leaks (use Process Explorer)\n";
    std::cout << "3. System stability (leave running 24h)\n";
    std::cout << "4. Event Viewer errors\n\n";
    
    std::cout << "Press Enter to exit...\n";
    std::cin.get();
    
    return 0;
}


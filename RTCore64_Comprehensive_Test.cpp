#include "pch.h"
#include "BDO_RTCore64_Driver.h"
#include "BDO_RTCore64_Error.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>

// External RTCore64 driver instance
extern RTCore64Driver g_RTCore64;

// Test result structure
struct TestResult {
    std::string testName;
    bool success;
    std::string message;
    double executionTime; // milliseconds
};

// Test suite class
class RTCore64TestSuite {
private:
    std::vector<TestResult> results;
    bool verbose;
    
    // Helper to measure execution time
    template<typename Func>
    TestResult RunTest(const std::string& testName, Func testFunc) {
        TestResult result;
        result.testName = testName;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            std::tie(result.success, result.message) = testFunc();
        } catch (const std::exception& e) {
            result.success = false;
            result.message = std::string("Exception: ") + e.what();
        } catch (...) {
            result.success = false;
            result.message = "Unknown exception";
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.executionTime = std::chrono::duration<double, std::milli>(end - start).count();
        
        results.push_back(result);
        
        // Print result immediately if verbose
        if (verbose) {
            PrintTestResult(result);
        }
        
        return result;
    }
    
    // Print a single test result
    void PrintTestResult(const TestResult& result) {
        std::cout << "[" << (result.success ? "PASS" : "FAIL") << "] " 
                  << std::left << std::setw(40) << result.testName 
                  << " - " << std::fixed << std::setprecision(2) << result.executionTime << " ms";
        
        if (!result.message.empty()) {
            std::cout << " - " << result.message;
        }
        
        std::cout << std::endl;
    }
    
    // Test functions
    std::pair<bool, std::string> TestConnection() {
        if (g_RTCore64.IsConnected() || g_RTCore64.Connect()) {
            return {true, "Successfully connected to RTCore64 driver"};
        } else {
            return {false, "Failed to connect to RTCore64 driver"};
        }
    }
    
    std::pair<bool, std::string> TestReadMemory() {
        if (!g_RTCore64.IsConnected() && !g_RTCore64.Connect()) {
            return {false, "Not connected to RTCore64 driver"};
        }
        
        // Try multiple safe addresses until one works
        const std::vector<ULONG64> testAddresses = {
            0x1000,    // Low address
            0x10000,   // Another safe address
            0x100000,  // Higher but still safe address
            0x1000000  // Even higher address
        };
        
        for (const auto& address : testAddresses) {
            DWORD value = 0;
            if (g_RTCore64.ReadMemory(address, &value, sizeof(value))) {
                std::stringstream ss;
                ss << "Successfully read memory at 0x" << std::hex << address 
                   << ", value: 0x" << std::hex << value;
                return {true, ss.str()};
            }
            // Continue to next address if this one fails
        }
        
        // If we get here, all reads failed
        RTCore64Error error = g_RTCore64ErrorHandler.GetLastError();
        std::string errorMsg = "Failed to read memory: " + g_RTCore64ErrorHandler.FormatErrorMessage(error);
        return {false, errorMsg};
    }
    
    std::pair<bool, std::string> TestAlignedReads() {
        if (!g_RTCore64.IsConnected() && !g_RTCore64.Connect()) {
            return {false, "Not connected to RTCore64 driver"};
        }
        
        // Test aligned reads of different sizes
        ULONG64 address = 0x1000;
        bool success = true;
        std::string message;
        
        // Test DWORD read (4 bytes, aligned)
        DWORD dwordValue = 0;
        if (!g_RTCore64.ReadMemory(address, &dwordValue, sizeof(dwordValue))) {
            success = false;
            message += "Failed to read DWORD; ";
        }
        
        // Test WORD read (2 bytes, aligned)
        WORD wordValue = 0;
        if (!g_RTCore64.ReadMemory(address, &wordValue, sizeof(wordValue))) {
            success = false;
            message += "Failed to read WORD; ";
        }
        
        // Test BYTE read (1 byte)
        BYTE byteValue = 0;
        if (!g_RTCore64.ReadMemory(address, &byteValue, sizeof(byteValue))) {
            success = false;
            message += "Failed to read BYTE; ";
        }
        
        if (success) {
            return {true, "All aligned reads successful"};
        } else {
            return {false, message};
        }
    }
    
    std::pair<bool, std::string> TestUnalignedReads() {
        if (!g_RTCore64.IsConnected() && !g_RTCore64.Connect()) {
            return {false, "Not connected to RTCore64 driver"};
        }
        
        // Test unaligned reads
        ULONG64 address = 0x1001; // Unaligned address
        bool success = true;
        std::string message;
        
        // Test 3-byte read (unaligned)
        BYTE buffer[3] = {0};
        if (!g_RTCore64.ReadMemory(address, buffer, sizeof(buffer))) {
            success = false;
            message += "Failed to read 3 bytes; ";
        }
        
        // Test 5-byte read (crossing DWORD boundary)
        BYTE buffer2[5] = {0};
        if (!g_RTCore64.ReadMemory(address, buffer2, sizeof(buffer2))) {
            success = false;
            message += "Failed to read 5 bytes; ";
        }
        
        // Test 7-byte read (crossing multiple DWORD boundaries)
        BYTE buffer3[7] = {0};
        if (!g_RTCore64.ReadMemory(address, buffer3, sizeof(buffer3))) {
            success = false;
            message += "Failed to read 7 bytes; ";
        }
        
        if (success) {
            return {true, "All unaligned reads successful"};
        } else {
            return {false, message};
        }
    }
    
    std::pair<bool, std::string> TestLargeReads() {
        if (!g_RTCore64.IsConnected() && !g_RTCore64.Connect()) {
            return {false, "Not connected to RTCore64 driver"};
        }
        
        // Test large reads
        ULONG64 address = 0x1000;
        bool success = true;
        std::string message;
        
        // Test 1KB read
        std::vector<BYTE> buffer1(1024, 0);
        if (!g_RTCore64.ReadMemory(address, buffer1.data(), buffer1.size())) {
            success = false;
            message += "Failed to read 1KB; ";
        }
        
        // Test 4KB read
        std::vector<BYTE> buffer2(4096, 0);
        if (!g_RTCore64.ReadMemory(address, buffer2.data(), buffer2.size())) {
            success = false;
            message += "Failed to read 4KB; ";
        }
        
        if (success) {
            return {true, "Large reads successful"};
        } else {
            return {false, message};
        }
    }
    
    std::pair<bool, std::string> TestErrorHandling() {
        // Clear previous errors
        g_RTCore64ErrorHandler.ClearErrorHistory();
        
        // Test with invalid parameters
        ULONG64 address = 0x1000;
        bool success = true;
        std::string message;
        
        // Test with null buffer
        if (g_RTCore64.ReadMemory(address, nullptr, 4)) {
            success = false;
            message += "Read with null buffer should fail; ";
        }
        
        // Test with zero size
        DWORD value = 0;
        if (g_RTCore64.ReadMemory(address, &value, 0)) {
            success = false;
            message += "Read with zero size should fail; ";
        }
        
        // Test with invalid address
        if (g_RTCore64.ReadMemory(0xFFFFFFFFFFFFFFFF, &value, sizeof(value))) {
            success = false;
            message += "Read with invalid address should fail; ";
        }
        
        // Check if errors were recorded
        auto errors = g_RTCore64ErrorHandler.GetErrorHistory();
        if (errors.empty()) {
            success = false;
            message += "No errors were recorded; ";
        }
        
        if (success) {
            return {true, "Error handling working correctly"};
        } else {
            return {false, message};
        }
    }
    
    std::pair<bool, std::string> TestPerformance() {
        if (!g_RTCore64.IsConnected() && !g_RTCore64.Connect()) {
            return {false, "Not connected to RTCore64 driver"};
        }
        
        // Test read performance
        ULONG64 address = 0x1000;
        DWORD value = 0;
        int iterations = 1000;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; i++) {
            if (!g_RTCore64.ReadMemory(address, &value, sizeof(value))) {
                return {false, "Read failed during performance test"};
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        double totalTime = std::chrono::duration<double, std::milli>(end - start).count();
        double avgTime = totalTime / iterations;
        
        std::stringstream ss;
        ss << "Average read time: " << std::fixed << std::setprecision(3) << avgTime 
           << " ms (" << iterations << " iterations)";
        
        return {true, ss.str()};
    }
    
public:
    RTCore64TestSuite(bool verboseOutput = true) : verbose(verboseOutput) {}
    
    // Run all tests
    void RunAllTests(bool verboseOutput) {
        verbose = verboseOutput;
        std::cout << "=== RTCore64 Comprehensive Test Suite ===" << std::endl;
        
        // Basic connectivity tests
        RunTest("Connection Test", [this]() { return TestConnection(); });
        
        // Memory read tests
        RunTest("Basic Read Test", [this]() { return TestReadMemory(); });
        RunTest("Aligned Reads Test", [this]() { return TestAlignedReads(); });
        RunTest("Unaligned Reads Test", [this]() { return TestUnalignedReads(); });
        RunTest("Large Reads Test", [this]() { return TestLargeReads(); });
        
        // Error handling tests
        RunTest("Error Handling Test", [this]() { return TestErrorHandling(); });
        
        // Performance tests
        RunTest("Performance Test", [this]() { return TestPerformance(); });
        
        // Print summary
        PrintSummary();
    }
    
    // Print test summary
    void PrintSummary() {
        int passed = 0;
        int failed = 0;
        double totalTime = 0.0;
        
        for (const auto& result : results) {
            if (result.success) {
                passed++;
            } else {
                failed++;
            }
            totalTime += result.executionTime;
        }
        
        std::cout << std::endl << "=== Test Summary ===" << std::endl;
        std::cout << "Total Tests: " << results.size() << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        std::cout << "Total Time: " << std::fixed << std::setprecision(2) << totalTime << " ms" << std::endl;
        
        if (failed > 0) {
            std::cout << std::endl << "Failed Tests:" << std::endl;
            for (const auto& result : results) {
                if (!result.success) {
                    std::cout << "- " << result.testName << ": " << result.message << std::endl;
                }
            }
        }
    }
};

// Entry point for comprehensive test
int rtcore64_comprehensive_test_main() {
    std::cout << "Starting RTCore64 Comprehensive Test..." << std::endl;
    
    // Enable error logging
    g_RTCore64ErrorHandler.EnableConsoleLogging(true);
    g_RTCore64ErrorHandler.EnableFileLogging(true, "RTCore64_Test.log");
    
    // Run test suite
    RTCore64TestSuite testSuite;
    testSuite.RunAllTests(true);
    
    std::cout << std::endl << "Press Enter to exit..." << std::endl;
    std::cin.get();
    
    return 0;
}

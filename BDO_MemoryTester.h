#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <iostream>
#include <iomanip>

// Comprehensive testing and validation system for memory operations

namespace BDO {
    namespace Testing {

        // Test result structure
        struct TestResult {
            std::string testName;
            bool passed;
            std::string message;
            std::chrono::milliseconds duration;
            std::string category;
        };

        // Test statistics
        struct TestStats {
            int totalTests = 0;
            int passedTests = 0;
            int failedTests = 0;
            std::chrono::milliseconds totalDuration{ 0 };
            
            float GetPassRate() const {
                return totalTests > 0 ? (float)passedTests / totalTests * 100.0f : 0.0f;
            }
        };

        // Memory validation result
        struct ValidationResult {
            bool isValid;
            std::string reason;
            PVOID address;
            SIZE_T size;
        };

        class MemoryTester {
        private:
            std::vector<TestResult> results;
            TestStats stats;
            bool verbose;
            
        public:
            MemoryTester(bool verboseOutput = true) : verbose(verboseOutput) {}
            
            // Run a test with automatic timing
            void RunTest(const std::string& name, const std::string& category, std::function<bool()> testFunc) {
                auto start = std::chrono::high_resolution_clock::now();
                
                if (verbose) {
                    std::cout << "[TEST] Running: " << name << "..." << std::endl;
                }
                
                bool passed = false;
                std::string message;
                
                try {
                    passed = testFunc();
                    message = passed ? "PASSED" : "FAILED";
                } catch (const std::exception& e) {
                    passed = false;
                    message = std::string("EXCEPTION: ") + e.what();
                } catch (...) {
                    passed = false;
                    message = "UNKNOWN EXCEPTION";
                }
                
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                
                TestResult result;
                result.testName = name;
                result.passed = passed;
                result.message = message;
                result.duration = duration;
                result.category = category;
                
                results.push_back(result);
                stats.totalTests++;
                stats.totalDuration += duration;
                
                if (passed) {
                    stats.passedTests++;
                    if (verbose) {
                        std::cout << "  ✓ PASSED (" << duration.count() << "ms)" << std::endl;
                    }
                } else {
                    stats.failedTests++;
                    if (verbose) {
                        std::cout << "  ✗ FAILED: " << message << " (" << duration.count() << "ms)" << std::endl;
                    }
                }
            }
            
            // Validate memory address
            ValidationResult ValidateAddress(PVOID address, SIZE_T size = sizeof(DWORD)) {
                ValidationResult result;
                result.address = address;
                result.size = size;
                result.isValid = true;
                
                // Check if address is null
                if (!address) {
                    result.isValid = false;
                    result.reason = "Address is NULL";
                    return result;
                }
                
                // Check if address is in valid range
                ULONG_PTR addr = reinterpret_cast<ULONG_PTR>(address);
                if (addr < 0x10000) {
                    result.isValid = false;
                    result.reason = "Address is too low (< 0x10000)";
                    return result;
                }
                
                // Check if address is aligned (for performance)
                if (size >= 4 && (addr % 4) != 0) {
                    result.reason = "Warning: Address not 4-byte aligned";
                }
                
                // Try to read from address
                MEMORY_BASIC_INFORMATION mbi;
                if (VirtualQuery(address, &mbi, sizeof(mbi)) == 0) {
                    result.isValid = false;
                    result.reason = "VirtualQuery failed - invalid memory region";
                    return result;
                }
                
                // Check if memory is committed
                if (mbi.State != MEM_COMMIT) {
                    result.isValid = false;
                    result.reason = "Memory not committed";
                    return result;
                }
                
                // Check if memory is readable
                if (!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) {
                    result.isValid = false;
                    result.reason = "Memory not readable";
                    return result;
                }
                
                result.reason = "Valid";
                return result;
            }
            
            // Test memory read operation
            bool TestMemoryRead(HANDLE hProcess, PVOID address, SIZE_T size) {
                std::vector<BYTE> buffer(size);
                SIZE_T bytesRead = 0;
                
                if (!ReadProcessMemory(hProcess, address, buffer.data(), size, &bytesRead)) {
                    return false;
                }
                
                return bytesRead == size;
            }
            
            // Test memory write operation
            bool TestMemoryWrite(HANDLE hProcess, PVOID address, const void* data, SIZE_T size) {
                SIZE_T bytesWritten = 0;
                
                if (!WriteProcessMemory(hProcess, address, data, size, &bytesWritten)) {
                    return false;
                }
                
                return bytesWritten == size;
            }
            
            // Test pattern scanning
            bool TestPatternScan(const std::vector<BYTE>& memory, const std::string& pattern, const std::string& mask) {
                if (pattern.length() != mask.length()) {
                    return false;
                }
                
                for (size_t i = 0; i < memory.size() - pattern.length(); i++) {
                    bool found = true;
                    for (size_t j = 0; j < pattern.length(); j++) {
                        if (mask[j] == 'x' && memory[i + j] != pattern[j]) {
                            found = false;
                            break;
                        }
                    }
                    if (found) {
                        return true;
                    }
                }
                
                return false;
            }
            
            // Print test results
            void PrintResults() {
                std::cout << "\n============================================================" << std::endl;
                std::cout << "                    TEST RESULTS" << std::endl;
                std::cout << "============================================================\n" << std::endl;
                
                // Group by category
                std::map<std::string, std::vector<TestResult>> categorized;
                for (const auto& result : results) {
                    categorized[result.category].push_back(result);
                }
                
                // Print each category
                for (const auto& [category, tests] : categorized) {
                    std::cout << "Category: " << category << std::endl;
                    std::cout << "------------------------------------------------------------" << std::endl;
                    
                    for (const auto& test : tests) {
                        std::cout << "  " << (test.passed ? "✓" : "✗") << " " 
                                  << std::setw(40) << std::left << test.testName 
                                  << " (" << test.duration.count() << "ms)";
                        
                        if (!test.passed) {
                            std::cout << " - " << test.message;
                        }
                        std::cout << std::endl;
                    }
                    std::cout << std::endl;
                }
                
                // Print statistics
                std::cout << "============================================================" << std::endl;
                std::cout << "Total Tests:   " << stats.totalTests << std::endl;
                std::cout << "Passed:        " << stats.passedTests << " (" << std::fixed << std::setprecision(1) << stats.GetPassRate() << "%)" << std::endl;
                std::cout << "Failed:        " << stats.failedTests << std::endl;
                std::cout << "Total Time:    " << stats.totalDuration.count() << "ms" << std::endl;
                std::cout << "Average Time:  " << (stats.totalTests > 0 ? stats.totalDuration.count() / stats.totalTests : 0) << "ms per test" << std::endl;
                std::cout << "============================================================\n" << std::endl;
                
                if (stats.failedTests == 0) {
                    std::cout << "🎉 ALL TESTS PASSED! 🎉" << std::endl;
                } else {
                    std::cout << "⚠️  SOME TESTS FAILED - Review above for details" << std::endl;
                }
            }
            
            // Get statistics
            const TestStats& GetStats() const {
                return stats;
            }
            
            // Get all results
            const std::vector<TestResult>& GetResults() const {
                return results;
            }
            
            // Clear results
            void Clear() {
                results.clear();
                stats = TestStats();
            }
            
            // Save results to file
            bool SaveToFile(const std::string& filename) {
                std::ofstream file(filename);
                if (!file.is_open()) {
                    return false;
                }
                
                file << "BDO Memory Tester - Test Results\n";
                file << "Generated: " << std::time(nullptr) << "\n\n";
                
                file << "Summary:\n";
                file << "  Total Tests: " << stats.totalTests << "\n";
                file << "  Passed: " << stats.passedTests << "\n";
                file << "  Failed: " << stats.failedTests << "\n";
                file << "  Pass Rate: " << stats.GetPassRate() << "%\n";
                file << "  Total Duration: " << stats.totalDuration.count() << "ms\n\n";
                
                file << "Detailed Results:\n";
                file << "------------------------------------------------------------\n";
                
                for (const auto& result : results) {
                    file << (result.passed ? "[PASS]" : "[FAIL]") << " "
                         << result.category << " - " << result.testName
                         << " (" << result.duration.count() << "ms)";
                    
                    if (!result.passed) {
                        file << " - " << result.message;
                    }
                    file << "\n";
                }
                
                file.close();
                return true;
            }
        };

        // Pre-built test suites
        class TestSuites {
        public:
            // Test basic memory operations
            static void TestBasicMemory(MemoryTester& tester, HANDLE hProcess, PVOID testAddress) {
                tester.RunTest("Read DWORD", "Basic Memory", [&]() {
                    DWORD value = 0;
                    SIZE_T bytesRead = 0;
                    return ReadProcessMemory(hProcess, testAddress, &value, sizeof(DWORD), &bytesRead) && bytesRead == sizeof(DWORD);
                });
                
                tester.RunTest("Read QWORD", "Basic Memory", [&]() {
                    QWORD value = 0;
                    SIZE_T bytesRead = 0;
                    return ReadProcessMemory(hProcess, testAddress, &value, sizeof(QWORD), &bytesRead) && bytesRead == sizeof(QWORD);
                });
                
                tester.RunTest("Read Float", "Basic Memory", [&]() {
                    float value = 0.0f;
                    SIZE_T bytesRead = 0;
                    return ReadProcessMemory(hProcess, testAddress, &value, sizeof(float), &bytesRead) && bytesRead == sizeof(float);
                });
            }
            
            // Test address validation
            static void TestAddressValidation(MemoryTester& tester) {
                tester.RunTest("Validate NULL address", "Validation", [&]() {
                    auto result = tester.ValidateAddress(nullptr);
                    return !result.isValid; // Should be invalid
                });
                
                tester.RunTest("Validate low address", "Validation", [&]() {
                    auto result = tester.ValidateAddress(reinterpret_cast<PVOID>(0x1000));
                    return !result.isValid; // Should be invalid
                });
            }
            
            // Test pattern scanning
            static void TestPatternScanning(MemoryTester& tester) {
                std::vector<BYTE> testData = { 0x48, 0x8B, 0x05, 0x12, 0x34, 0x56, 0x78 };
                std::string pattern = "\x48\x8B\x05";
                std::string mask = "xxx";
                
                tester.RunTest("Find exact pattern", "Pattern Scan", [&]() {
                    return tester.TestPatternScan(testData, pattern, mask);
                });
                
                tester.RunTest("Pattern with wildcard", "Pattern Scan", [&]() {
                    std::string wildcardPattern = "\x48\x00\x05";
                    std::string wildcardMask = "x?x";
                    return tester.TestPatternScan(testData, wildcardPattern, wildcardMask);
                });
            }
        };

    } // namespace Testing
} // namespace BDO


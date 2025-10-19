#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <conio.h>
#include "BDOAntiCheatResearch.h"
#include "BDOAntiDetection.h"

void PrintBanner() {
    std::cout << R"(
    ╔════════════════════════════════════════════════════════════════╗
    ║     Black Desert Online - Anti-Cheat Research Framework       ║
    ║                    Educational Purpose Only                    ║
    ╔════════════════════════════════════════════════════════════════╝
    )" << std::endl;
}

void PrintMenu() {
    std::cout << "\n[Main Menu]" << std::endl;
    std::cout << "1. Detect Active Anti-Cheat" << std::endl;
    std::cout << "2. Scan Memory Regions" << std::endl;
    std::cout << "3. Detect Hooks" << std::endl;
    std::cout << "4. Check Process Integrity" << std::endl;
    std::cout << "5. Analyze Threads" << std::endl;
    std::cout << "6. Find Game Offsets" << std::endl;
    std::cout << "7. Test Bypass Techniques (Research)" << std::endl;
    std::cout << "8. Generate Full Report" << std::endl;
    std::cout << "9. Memory Pattern Scanner" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "\nChoice: ";
}

void DetectAntiCheat(BDOAntiCheatResearch& research) {
    std::cout << "\n[*] Detecting Anti-Cheat Systems..." << std::endl;
    
    auto info = research.DetectAntiCheat();
    
    if (info.isActive) {
        std::wcout << L"[+] Anti-Cheat Detected: " << info.name << std::endl;
        std::cout << "[+] Protection Level: " << info.protectionLevel << "/10" << std::endl;
        std::cout << "[+] Number of Hooks: " << info.hookAddresses.size() << std::endl;
        
        std::cout << "\n[+] Protected Functions:" << std::endl;
        for (const auto& func : info.protectedFunctions) {
            std::cout << "    - " << func << std::endl;
        }
        
        if (!info.hookAddresses.empty()) {
            std::cout << "\n[+] Hook Addresses:" << std::endl;
            for (const auto& addr : info.hookAddresses) {
                std::cout << "    - 0x" << std::hex << addr << std::dec << std::endl;
            }
        }
    } else {
        std::cout << "[!] No active anti-cheat detected (unusual for BDO)" << std::endl;
    }
}

void ScanMemoryRegions(BDOAntiCheatResearch& research) {
    std::cout << "\n[*] Scanning Memory Regions..." << std::endl;
    
    auto regions = research.ScanMemoryRegions();
    
    std::cout << "[+] Found " << regions.size() << " memory regions" << std::endl;
    
    // Statistics
    int protectedCount = 0;
    int hookedCount = 0;
    size_t totalSize = 0;
    
    for (const auto& region : regions) {
        if (region.isProtected) protectedCount++;
        if (region.hasHooks) hookedCount++;
        totalSize += region.size;
    }
    
    std::cout << "\n[Memory Statistics]" << std::endl;
    std::cout << "  Total Regions: " << regions.size() << std::endl;
    std::cout << "  Protected Regions: " << protectedCount << std::endl;
    std::cout << "  Hooked Regions: " << hookedCount << std::endl;
    std::cout << "  Total Memory: " << (totalSize / 1024 / 1024) << " MB" << std::endl;
    
    // Show first 10 protected regions
    std::cout << "\n[Protected Regions (First 10)]" << std::endl;
    int count = 0;
    for (const auto& region : regions) {
        if (region.isProtected && count < 10) {
            std::cout << "  0x" << std::hex << region.baseAddress 
                     << " - Size: 0x" << region.size 
                     << " - Type: " << region.type << std::dec << std::endl;
            count++;
        }
    }
}

void DetectHooks(BDOAntiCheatResearch& research) {
    std::cout << "\n[*] Detecting Hooks..." << std::endl;
    
    auto hooks = research.DetectHooks();
    
    std::cout << "[+] Found " << hooks.size() << " hooks" << std::endl;
    
    for (const auto& hook : hooks) {
        std::cout << "\n[Hook Details]" << std::endl;
        std::cout << "  Function: " << hook.functionName << std::endl;
        std::cout << "  Address: 0x" << std::hex << hook.address << std::dec << std::endl;
        std::cout << "  Type: ";
        
        switch (hook.type) {
            case BDOResearch::HookType::INLINE_HOOK:
                std::cout << "Inline Hook" << std::endl;
                break;
            case BDOResearch::HookType::IAT_HOOK:
                std::cout << "IAT Hook" << std::endl;
                break;
            case BDOResearch::HookType::HARDWARE_BREAKPOINT:
                std::cout << "Hardware Breakpoint" << std::endl;
                break;
            default:
                std::cout << "Unknown" << std::endl;
        }
        
        if (hook.jumpTarget != 0) {
            std::cout << "  Jump Target: 0x" << std::hex << hook.jumpTarget << std::dec << std::endl;
        }
        
        // Show first few hooked bytes
        if (!hook.hookedBytes.empty()) {
            std::cout << "  Hooked Bytes: ";
            for (size_t i = 0; i < std::min(size_t(8), hook.hookedBytes.size()); i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                         << static_cast<int>(hook.hookedBytes[i]) << " ";
            }
            std::cout << std::dec << std::endl;
        }
    }
}

void CheckIntegrity(BDOAntiCheatResearch& research) {
    std::cout << "\n[*] Checking Process Integrity..." << std::endl;
    
    auto check = research.CheckProcessIntegrity();
    
    std::cout << "\n[Integrity Check Results]" << std::endl;
    std::cout << "  CRC Check: " << (check.crcCheck ? "PASSED" : "FAILED") << std::endl;
    std::cout << "  Debugger Present: " << (check.debuggerPresent ? "YES (!)": "No") << std::endl;
    std::cout << "  VM Detected: " << (check.vmDetected ? "YES (!)" : "No") << std::endl;
    std::cout << "  Sandbox Detected: " << (check.sandboxDetected ? "YES (!)" : "No") << std::endl;
    std::cout << "  Hooking Tools: " << (check.hookingToolsDetected ? "DETECTED (!)" : "Clean") << std::endl;
    
    if (!check.detectedTools.empty()) {
        std::cout << "\n[Detected Tools]" << std::endl;
        for (const auto& tool : check.detectedTools) {
            std::cout << "  - " << tool << std::endl;
        }
    }
    
    if (check.debuggerPresent || check.vmDetected || check.hookingToolsDetected) {
        std::cout << "\n[!] WARNING: Detection risk is HIGH!" << std::endl;
        std::cout << "[!] Anti-cheat may flag or ban this session" << std::endl;
    }
}

void AnalyzeThreads(BDOAntiCheatResearch& research) {
    std::cout << "\n[*] Analyzing Threads..." << std::endl;
    
    auto threads = research.AnalyzeThreads();
    
    std::cout << "[+] Found " << threads.size() << " threads" << std::endl;
    
    int suspiciousCount = 0;
    for (const auto& thread : threads) {
        if (thread.isSuspicious) {
            suspiciousCount++;
            std::cout << "\n[Suspicious Thread]" << std::endl;
            std::cout << "  Thread ID: " << thread.threadId << std::endl;
            std::cout << "  Start Address: 0x" << std::hex << thread.startAddress << std::dec << std::endl;
            std::cout << "  Module: " << (thread.moduleName.empty() ? "Unknown" : thread.moduleName) << std::endl;
        }
    }
    
    std::cout << "\n[Summary]" << std::endl;
    std::cout << "  Total Threads: " << threads.size() << std::endl;
    std::cout << "  Suspicious Threads: " << suspiciousCount << std::endl;
}

void FindOffsets(BDOAntiCheatResearch& research) {
    std::cout << "\n[*] Finding Game Offsets..." << std::endl;
    std::cout << "[!] This may take several seconds..." << std::endl;
    
    auto offsets = research.FindGameOffsets();
    
    std::cout << "\n[Game Offsets]" << std::endl;
    std::cout << "  Player Base: 0x" << std::hex << offsets.playerBase << std::dec << std::endl;
    std::cout << "  Entity List: 0x" << std::hex << offsets.entityList << std::dec << std::endl;
    std::cout << "  World Base: 0x" << std::hex << offsets.worldBase << std::dec << std::endl;
    std::cout << "  UI Base: 0x" << std::hex << offsets.uiBase << std::dec << std::endl;
    
    if (!offsets.dynamicOffsets.empty()) {
        std::cout << "\n[Dynamic Offsets]" << std::endl;
        for (const auto& [name, offset] : offsets.dynamicOffsets) {
            std::cout << "  " << name << ": 0x" << std::hex << offset << std::dec << std::endl;
        }
    }
    
    // Example of reading memory using found offsets
    if (offsets.playerBase != 0) {
        std::cout << "\n[*] Attempting to read player data..." << std::endl;
        
        // Read health
        auto healthAddr = offsets.playerBase + 0x2C;  // Example offset
        auto health = research.SafeReadMemory<int>(healthAddr);
        if (health.has_value()) {
            std::cout << "  Player Health: " << health.value() << std::endl;
        } else {
            std::cout << "  Failed to read health (protected or invalid)" << std::endl;
        }
    }
}

void TestBypassTechniques(BDOAntiCheatResearch& research) {
    std::cout << "\n[*] Testing Bypass Techniques (Educational Research)" << std::endl;
    std::cout << "[!] WARNING: These techniques are for research only!" << std::endl;
    std::cout << "[!] Using them in the actual game violates ToS!" << std::endl;
    
    auto techniques = research.GetKnownBypassTechniques();
    
    std::cout << "\n[Available Techniques]" << std::endl;
    for (size_t i = 0; i < techniques.size(); i++) {
        std::cout << i + 1 << ". " << techniques[i].name << std::endl;
        std::cout << "   Description: " << techniques[i].description << std::endl;
        std::cout << "   Risk Level: " << techniques[i].detectionRisk << "/10" << std::endl;
        std::cout << "   Status: " << (techniques[i].isWorking ? "Working" : "Detected") << std::endl;
    }
    
    std::cout << "\nSelect technique to test (0 to cancel): ";
    int choice;
    std::cin >> choice;
    
    if (choice > 0 && choice <= techniques.size()) {
        std::cout << "\n[*] Testing: " << techniques[choice-1].name << std::endl;
        
        bool result = research.TestBypassTechnique(techniques[choice-1].name);
        
        if (result) {
            std::cout << "[+] Technique appears to be working" << std::endl;
        } else {
            std::cout << "[-] Technique is detected or failed" << std::endl;
        }
    }
}

void PatternScanner(BDOAntiCheatResearch& research) {
    std::cout << "\n[Pattern Scanner]" << std::endl;
    std::cout << "Enter pattern (hex bytes separated by spaces, use ?? for wildcards):" << std::endl;
    std::cout << "Example: 48 89 5C 24 ?? 48 89 74 24 ??" << std::endl;
    std::cout << "Pattern: ";
    
    std::cin.ignore();
    std::string patternInput;
    std::getline(std::cin, patternInput);
    
    // Parse pattern and mask
    std::string pattern;
    std::string mask;
    std::istringstream iss(patternInput);
    std::string byte;
    
    while (iss >> byte) {
        if (byte == "??") {
            pattern += "00 ";
            mask += "?";
        } else {
            pattern += byte + " ";
            mask += "x";
        }
    }
    
    std::cout << "[*] Scanning for pattern..." << std::endl;
    auto results = research.PatternScan(pattern, mask);
    
    std::cout << "[+] Found " << results.size() << " matches" << std::endl;
    
    // Show first 10 results
    for (size_t i = 0; i < std::min(size_t(10), results.size()); i++) {
        std::cout << "  [" << i+1 << "] 0x" << std::hex << results[i] << std::dec << std::endl;
    }
    
    if (results.size() > 10) {
        std::cout << "  ... and " << (results.size() - 10) << " more" << std::endl;
    }
}

void GenerateReport(BDOAntiCheatResearch& research) {
    std::cout << "\n[*] Generating comprehensive report..." << std::endl;
    std::cout << "[*] This will take several seconds..." << std::endl;
    
    auto report = research.GenerateReport();
    
    // Print to console
    research.PrintReport(report);
    
    // Save to file
    std::string filename = "bdo_research_report_" + 
                          std::to_string(std::chrono::system_clock::to_time_t(report.timestamp)) + 
                          ".txt";
    
    research.SaveReport(report, filename);
    std::cout << "\n[+] Report saved to: " << filename << std::endl;
}

int main() {
    PrintBanner();
    
    BDOAntiCheatResearch research;
    
    std::cout << "[*] Initializing research framework..." << std::endl;
    if (!research.Initialize()) {
        std::cout << "[-] Failed to initialize. Make sure:" << std::endl;
        std::cout << "    1. You're running as administrator" << std::endl;
        std::cout << "    2. Black Desert is running" << std::endl;
        std::cout << "    3. Anti-virus is not blocking the tool" << std::endl;
        return 1;
    }
    
    std::cout << "[*] Attaching to Black Desert..." << std::endl;
    if (!research.AttachToGame()) {
        std::cout << "[-] Failed to attach to game process" << std::endl;
        std::cout << "[!] This might be due to anti-cheat protection" << std::endl;
        return 1;
    }
    
    std::cout << "[+] Successfully attached!" << std::endl;
    std::cout << "[!] Remember: This tool is for educational research only!" << std::endl;
    std::cout << "[!] Using memory manipulation in online games violates ToS!" << std::endl;
    
    bool running = true;
    while (running) {
        PrintMenu();
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                DetectAntiCheat(research);
                break;
            case 2:
                ScanMemoryRegions(research);
                break;
            case 3:
                DetectHooks(research);
                break;
            case 4:
                CheckIntegrity(research);
                break;
            case 5:
                AnalyzeThreads(research);
                break;
            case 6:
                FindOffsets(research);
                break;
            case 7:
                TestBypassTechniques(research);
                break;
            case 8:
                GenerateReport(research);
                break;
            case 9:
                PatternScanner(research);
                break;
            case 0:
                running = false;
                break;
            default:
                std::cout << "[!] Invalid choice" << std::endl;
        }
        
        if (running) {
            std::cout << "\nPress any key to continue...";
            _getch();
        }
    }
    
    std::cout << "\n[*] Cleaning up..." << std::endl;
    research.Cleanup();
    
    std::cout << "[+] Research session ended" << std::endl;
    
    return 0;
}
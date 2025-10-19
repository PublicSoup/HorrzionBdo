#include <iostream>
#include <windows.h>
#include "AntiDetection.h"

// Example function that shows proper handle conversion
void ProcessHandle(HANDLE hProcess) {
    // Example process operation
    if (hProcess != nullptr) {
        std::cout << "Processing handle: " << hProcess << std::endl;
    }
}

int main() {
    // Create an instance of AntiDetection (fixing "identifier antiDet is undefined")
    AntiDetection antiDet;
    
    // Configure anti-detection parameters
    antiDet.SetTimingRange(100, 500);  // Random delays between 100-500ms
    antiDet.SetPauseChance(15);        // 15% chance of random pause
    antiDet.SetJitterRange(10.0);      // Max jitter of 10 pixels
    
    std::cout << "AntiDetection System Demonstration\n";
    std::cout << "===================================\n\n";
    
    // Example 1: Using RandomizeTiming
    std::cout << "1. Adding random timing delay..." << std::endl;
    antiDet.RandomizeTiming();
    std::cout << "   Delay completed.\n\n";
    
    // Example 2: Using AddMouseJitter
    std::cout << "2. Adding mouse jitter..." << std::endl;
    POINT originalPos;
    GetCursorPos(&originalPos);
    std::cout << "   Original position: (" << originalPos.x << ", " << originalPos.y << ")" << std::endl;
    
    antiDet.AddMouseJitter();
    
    POINT newPos;
    GetCursorPos(&newPos);
    std::cout << "   New position: (" << newPos.x << ", " << newPos.y << ")" << std::endl;
    std::cout << "   Jitter applied: (" << (newPos.x - originalPos.x) << ", " << (newPos.y - originalPos.y) << ")\n\n";
    
    // Example 3: Using ShouldTakeRandomPause
    std::cout << "3. Checking for random pauses (10 iterations):" << std::endl;
    int pauseCount = 0;
    for (int i = 0; i < 10; i++) {
        if (antiDet.ShouldTakeRandomPause()) {
            std::cout << "   Iteration " << (i+1) << ": Taking a random pause..." << std::endl;
            antiDet.RandomDelay(1000, 3000);  // Pause for 1-3 seconds
            pauseCount++;
        } else {
            std::cout << "   Iteration " << (i+1) << ": No pause needed." << std::endl;
        }
        
        // Add some timing variation between checks
        antiDet.RandomizeTiming();
    }
    std::cout << "   Total pauses taken: " << pauseCount << " out of 10\n\n";
    
    // Example 4: Fixing the "uintptr_t to HANDLE" conversion error
    std::cout << "4. Demonstrating proper handle conversion:" << std::endl;
    
    // If you have a uintptr_t that needs to be converted to HANDLE
    uintptr_t addressValue = 0x12345678;  // Example address
    
    // Proper way to convert uintptr_t to HANDLE
    HANDLE hProcess = reinterpret_cast<HANDLE>(addressValue);
    
    std::cout << "   Original uintptr_t value: 0x" << std::hex << addressValue << std::endl;
    std::cout << "   Converted HANDLE value: " << hProcess << std::dec << std::endl;
    
    // Use the handle in a function that expects HANDLE
    ProcessHandle(hProcess);
    
    std::cout << "\n5. Simulating automated actions with anti-detection:" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "   Action " << (i+1) << ":" << std::endl;
        
        // Before each action, add timing randomization
        antiDet.RandomizeTiming();
        
        // Simulate an action (e.g., clicking)
        std::cout << "     - Performing action..." << std::endl;
        
        // Add mouse jitter to make it look more human-like
        antiDet.AddMouseJitter();
        
        // Check if we should take a random pause
        if (antiDet.ShouldTakeRandomPause()) {
            std::cout << "     - Taking a natural pause..." << std::endl;
            antiDet.RandomDelay(500, 1500);
        }
        
        std::cout << "     - Action completed.\n";
    }
    
    std::cout << "\nDemonstration complete!\n";
    
    return 0;
}
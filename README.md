# AntiDetection System for HorrzionBdo

## Overview
This project implements an AntiDetection class that helps simulate human-like behavior in automated systems. It addresses the following compilation errors:
- Missing member functions: `RandomizeTiming`, `AddMouseJitter`, `ShouldTakeRandomPause`
- Undefined identifier `antiDet`
- Type conversion issues from `uintptr_t` to `HANDLE`

## Files Structure
```
/workspace/
├── AntiDetection.h      # Header file with class declaration
├── AntiDetection.cpp    # Implementation of AntiDetection class
├── main.cpp            # Example usage and demonstration
├── Makefile           # Build configuration
└── README.md          # This file
```

## Features
- **RandomizeTiming()**: Adds random delays between actions to simulate human timing
- **AddMouseJitter()**: Adds small random movements to mouse positions
- **ShouldTakeRandomPause()**: Determines if a random pause should be taken
- **Configurable parameters**: Timing ranges, pause chances, and jitter amounts

## Building the Project

### Windows (MinGW/MSYS2)
```bash
make
```

### Or compile manually:
```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp AntiDetection.cpp -o antidetection_demo -luser32
```

## Usage Example

```cpp
#include "AntiDetection.h"

int main() {
    // Create an instance (fixes "identifier antiDet is undefined")
    AntiDetection antiDet;
    
    // Configure parameters
    antiDet.SetTimingRange(100, 500);  // Random delays 100-500ms
    antiDet.SetPauseChance(15);        // 15% chance of pause
    antiDet.SetJitterRange(10.0);      // Max 10 pixel jitter
    
    // Use in your automation loop
    while (running) {
        antiDet.RandomizeTiming();     // Add random delay
        
        // Your action here
        
        antiDet.AddMouseJitter();      // Add mouse variation
        
        if (antiDet.ShouldTakeRandomPause()) {
            antiDet.RandomDelay(1000, 3000);  // Pause 1-3 seconds
        }
    }
    
    return 0;
}
```

## Fixing Common Errors

### 1. Missing Member Functions
All required member functions are now implemented in the AntiDetection class.

### 2. Undefined Identifier "antiDet"
Create an instance of the class:
```cpp
AntiDetection antiDet;  // Creates the object
```

### 3. uintptr_t to HANDLE Conversion
Use proper casting:
```cpp
uintptr_t addressValue = 0x12345678;
HANDLE hProcess = reinterpret_cast<HANDLE>(addressValue);
```

## API Reference

### Constructor
- `AntiDetection()`: Initializes with default settings

### Configuration Methods
- `SetTimingRange(int min, int max)`: Set delay range in milliseconds
- `SetPauseChance(int chance)`: Set random pause probability (0-100)
- `SetJitterRange(double maxJitter)`: Set maximum mouse jitter in pixels

### Action Methods
- `RandomizeTiming()`: Execute a random delay
- `AddMouseJitter()`: Apply mouse position variation
- `ShouldTakeRandomPause()`: Check if pause should occur
- `RandomDelay(int minMs, int maxMs)`: Custom delay range

## Notes
- This implementation uses Windows API (windows.h) for mouse operations
- Requires C++17 or later
- Link with user32.lib (-luser32) for mouse functions

## License
This code is provided as a solution to fix the compilation errors.
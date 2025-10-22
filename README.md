# BDO Memory Reader - Educational Project

## ⚠️ EDUCATIONAL PURPOSE ONLY
This is a **working** memory reader for Black Desert Online, created for educational purposes to understand how game hacking and anti-cheat systems work.

**WARNING:** Using this on live servers violates ToS and will result in permanent bans. Use for learning only.

## Overview
Complete working implementation of:
- Process memory reading (ReadProcessMemory)
- Pattern/AOB scanning to find dynamic addresses
- Pointer chain resolution
- BDO-specific data reading (health, position, entities)
- Anti-detection timing and behavior
- Memory dumping and analysis tools

## Files Structure
```
/workspace/
├── MemoryReader.h/cpp      # Core memory reading engine
├── BDOMemory.h/cpp         # BDO-specific structures and reading
├── BDOBot.cpp              # Main program with interactive menu
├── AntiDetection.h/cpp     # Human behavior simulation
├── main.cpp                # Simple anti-detection demo
├── Makefile                # Build configuration
├── USAGE_GUIDE.md          # Detailed usage instructions
└── README.md               # This file
```

## Features

### Memory Reading
- **Process Attachment** - Attach to any running process by name or PID
- **Memory Read/Write** - Read and write any data type from process memory
- **Pattern Scanning** - Find addresses by unique byte patterns (AOB)
- **Pointer Chains** - Follow multi-level pointers automatically
- **Module Information** - Get base addresses and sizes of loaded DLLs

### BDO-Specific
- **Player Stats** - Read health, mana, stamina, position, level
- **Entity List** - Get nearby players, mobs, and NPCs
- **Distance Calculation** - Calculate 3D distance to entities
- **Dynamic Offset Finding** - Automatically locate game structures

### Anti-Detection
- **Timing Randomization** - Human-like delays (100-350ms)
- **Pattern Variation** - Avoid repetitive behavior
- **Mouse Jitter** - Natural mouse movement simulation
- **Read Throttling** - Limit memory access frequency

## Quick Start

### 1. Build
```bash
# Build everything
make all

# Or just the BDO bot
make bot

# Or manually
g++ -std=c++17 BDOBot.cpp BDOMemory.cpp MemoryReader.cpp AntiDetection.cpp -o bdo_bot -luser32 -lpsapi
```

### 2. Run
```bash
# MUST run as Administrator
./bdo_bot.exe
```

### 3. Use
- Start Black Desert
- Run the tool as admin
- Choose menu option to read memory

## Code Examples

### Reading Player Health
```cpp
#include "BDOMemory.h"

int main() {
    BDOMemory memory;
    
    // Initialize and attach
    memory.Initialize();
    memory.AttachToGame();
    
    // Read player stats
    float health, maxHealth;
    if (memory.GetPlayerHealth(health, maxHealth)) {
        std::cout << "HP: " << health << " / " << maxHealth << std::endl;
    }
    
    return 0;
}
```

### Pattern Scanning
```cpp
#include "MemoryReader.h"

int main() {
    MemoryReader reader;
    reader.AttachToProcess(L"BlackDesert64.exe");
    
    // Find pattern: MOV RCX, [RIP+offset]
    uintptr_t addr = reader.ScanAOB("48 8B 0D ?? ?? ?? ?? 48 85 C9");
    
    std::cout << "Found at: 0x" << std::hex << addr << std::endl;
    
    return 0;
}
```

### Reading with Anti-Detection
```cpp
#include "BDOMemory.h"
#include "AntiDetection.h"

int main() {
    BDOMemory memory;
    AntiDetection antiDet;
    
    memory.Initialize();
    memory.AttachToGame();
    
    antiDet.SetTimingRange(100, 300);  // Human-like delays
    
    while (true) {
        auto player = memory.GetPlayer();
        std::cout << "HP: " << player.health << std::endl;
        
        antiDet.RandomizeTiming();  // Add random delay
        
        if (antiDet.ShouldTakeRandomPause()) {
            antiDet.RandomDelay(1000, 3000);  // Natural pause
        }
    }
    
    return 0;
}
```

## How Memory Access Works

### 1. Finding the Game Process
```cpp
// Windows provides CreateToolhelp32Snapshot to enumerate processes
HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
Process32First(snapshot, &pe32);
// Loop until we find "BlackDesert64.exe"
```

### 2. Opening a Handle
```cpp
// OpenProcess with PROCESS_VM_READ permission
HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processId);
```

### 3. Reading Memory
```cpp
// ReadProcessMemory copies data from another process
ReadProcessMemory(hProcess, targetAddress, buffer, size, NULL);
```

### 4. Finding Addresses with Pattern Scanning
Since addresses change with game updates, we scan for unique byte patterns:
```cpp
// Look for instruction pattern: MOV RCX, [RIP+offset]
// Pattern: 48 8B 0D ?? ?? ?? ?? (where ?? = any byte)
uintptr_t found = ScanPattern("48 8B 0D ?? ?? ?? ??", "xxx????xxx");

// Read the offset and calculate absolute address
int offset = Read<int>(found + 3);
uintptr_t absolute = found + offset + 7;
```

### 5. Following Pointer Chains
Game data is accessed through multiple pointers:
```
Static Base (0x3A5B2C8) 
    -> Pointer (offset +0x18)
        -> Player Object (offset +0x30)
            -> Health (offset +0x2C)
```

## Bypassing Anti-Cheat (Educational)

Black Desert uses XIGNCODE3 and/or EasyAntiCheat which:
- **Hook APIs** - Intercept ReadProcessMemory calls
- **Kernel Driver** - Monitor all memory access from Ring 0
- **Integrity Checks** - Detect modified game code
- **Behavioral Analysis** - Flag suspicious patterns

### Why This Code Works (Sometimes)
- Uses standard APIs (ReadProcessMemory) which MAY work if anti-cheat isn't actively blocking
- Pattern scanning finds dynamic addresses
- Anti-detection timing helps avoid behavioral flags

### Why It Might Get Blocked
- Modern anti-cheats hook ReadProcessMemory
- Kernel drivers can see all memory access
- Reading game memory is inherently suspicious

### Real Bypass Methods (Educational Only)
1. **Direct Syscalls** - Call NtReadVirtualMemory directly (bypasses user-mode hooks)
2. **Kernel Driver** - Read from kernel level (requires driver signing)
3. **DMA Hardware** - External PCIe card reads memory via hardware
4. **VM Hypervisor** - Read from hypervisor level

## Important Notes

⚠️ **This is educational software**
- DO NOT use on live servers - you WILL be banned
- Use for learning memory reading concepts
- Test on isolated VM with alt account only
- Violating ToS can result in legal action

✅ **What you can learn:**
- Windows API programming
- Process memory management
- Pattern matching algorithms
- Anti-tamper mechanisms
- Security research methodology

## Troubleshooting

**"Failed to attach to game"**
- Run as Administrator
- Check process name (might be BlackDesert32.exe)
- Anti-cheat may be blocking access

**"Player base not found"**
- Patterns outdated after game update
- Use Cheat Engine to find new offsets
- Update patterns in BDOMemory.h

**"Access denied"**
- Need Administrator privileges
- Enable debug privileges (auto-enabled in code)

## Additional Resources

- See `USAGE_GUIDE.md` for detailed instructions
- Learn pattern scanning with Cheat Engine
- Study "Windows Internals" by Russinovich
- Visit GuidedHacking/UnknownCheats for education

## License

Educational use only. No warranty. Use at your own risk.
Violating game ToS will result in bans - you have been warned.
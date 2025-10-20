# BDO Advanced Memory System

## Overview
This advanced memory system provides comprehensive memory address resolution and management for Black Desert Online bot development. It includes pattern scanning, memory protection, hooking, patching, and anti-detection features.

## Features

### 🔍 **Memory Address Resolution**
- **Dynamic Pattern Scanning** - Automatically finds memory addresses using byte patterns
- **Multi-Region Scanning** - Scans executable, readable, and writable memory regions
- **Pattern Matching** - Supports wildcard patterns and custom masks
- **Address Caching** - Caches resolved addresses for performance
- **Auto-Update** - Automatically updates addresses when patterns change

### 🛡️ **Memory Protection & Anti-Detection**
- **Memory Protection Bypass** - Bypasses DEP, ASLR, and other protections
- **Stealth Mode** - Operates in stealth mode to avoid detection
- **Random Delays** - Adds random delays to operations
- **Timing Variation** - Varies operation timing to appear human-like
- **Operation Throttling** - Limits operations per second

### 🔧 **Advanced Memory Operations**
- **Memory Hooking** - Install and manage memory hooks
- **Memory Patching** - Apply and remove memory patches
- **Memory Scanning** - Continuous scanning with auto-rescan
- **Pointer Chains** - Read/write through complex pointer chains
- **Memory Protection** - Change memory protection attributes

### 🎮 **BDO-Specific Features**
- **Player Data Access** - Read/write player health, mana, weight, etc.
- **Combat System** - Access combat state, skill cooldowns, target info
- **UI System** - Read/write chat, menu, inventory data
- **Network System** - Intercept and modify network packets
- **Game State** - Access world time, weather, NPC data

## Files

### Core Files
- **`BDO_MemoryResolver.h`** - Main memory resolver class
- **`BDO_MemoryResolver.cpp`** - Memory resolver implementation
- **`BDO_AdvancedMemory.h`** - Advanced memory management
- **`BDO_AdvancedMemory.cpp`** - Advanced memory implementation

### Example Files
- **`BDO_MemoryExample.cpp`** - Basic usage example
- **`BDO_Bot_Integration.cpp`** - Bot integration example

## Usage

### Basic Usage

```cpp
#include "BDO_MemoryResolver.h"

// Create resolver
BDOMemoryResolver resolver;

// Attach to BDO process
if (resolver.AttachToProcess(L"BlackDesert64.exe")) {
    // Resolve all addresses
    resolver.ResolveAllAddresses();
    
    // Get specific address
    PVOID playerBase = resolver.GetAddress("PlayerBase");
    
    // Read player health
    int health, maxHealth;
    if (resolver.ReadMemory((PUCHAR)playerBase + 0x2C, &health, sizeof(health))) {
        std::cout << "Player Health: " << health << std::endl;
    }
}
```

### Advanced Usage

```cpp
#include "BDO_AdvancedMemory.h"

// Create advanced memory manager
BDOAdvancedMemory memory;

// Initialize with anti-detection
memory.Initialize(processId);
memory.EnableAntiDetection(true);
memory.EnableStealthMode(true);

// Read player data
int health, maxHealth;
memory.ReadPlayerHealth(health, maxHealth);

// Write player data
memory.WritePlayerHealth(100, 100);

// Install memory hook
std::vector<BYTE> hookBytes = {0x90, 0x90, 0x90}; // NOP instructions
memory.InstallHook("MyHook", address, hookBytes);

// Apply memory patch
std::vector<BYTE> patchBytes = {0xEB, 0x00}; // JMP instruction
memory.ApplyPatch("MyPatch", address, patchBytes);
```

### Bot Integration

```cpp
class MyBDOBot {
private:
    std::unique_ptr<BDOAdvancedMemory> memory;
    
public:
    bool Initialize() {
        memory = std::make_unique<BDOAdvancedMemory>();
        return memory->AttachToProcess(L"BlackDesert64.exe");
    }
    
    void UpdatePlayerState() {
        int health, maxHealth;
        if (memory->ReadPlayerHealth(health, maxHealth)) {
            if (health < 50) {
                UseHealthPotion();
            }
        }
    }
};
```

## Configuration

### Anti-Detection Settings
```cpp
// Enable anti-detection
memory->EnableAntiDetection(true);

// Enable stealth mode
memory->EnableStealthMode(true);

// Set operation delays
memory->SetOperationDelay(10, 50); // 10-50ms random delay

// Set max operations per second
memory->SetMaxOperationsPerSecond(10);
```

### Memory Protection
```cpp
// Protect memory region
memory->ProtectMemory(address, size, PAGE_READONLY);

// Unprotect memory region
memory->UnprotectMemory(address, size);

// Restore original protection
memory->RestoreMemoryProtection(address, size);
```

## BDO-Specific Functions

### Player Data
```cpp
// Read player data
int health, maxHealth;
memory->ReadPlayerHealth(health, maxHealth);

float weight, maxWeight;
memory->ReadPlayerWeight(weight, maxWeight);

float x, y, z;
memory->ReadPlayerPosition(x, y, z);

// Write player data
memory->WritePlayerHealth(100, 100);
memory->WritePlayerWeight(50.0f, 100.0f);
```

### Combat System
```cpp
// Read combat state
int combatState;
memory->ReadCombatState(combatState);

// Read skill cooldown
float cooldown;
memory->ReadSkillCooldown(skillId, cooldown);

// Write combat state
memory->WriteCombatState(1);
```

### UI System
```cpp
// Read chat message
std::string message;
memory->ReadChatMessage(0, message);

// Write chat message
memory->WriteChatMessage("Hello World");

// Read inventory slot
int itemId, quantity;
memory->ReadInventorySlot(0, itemId, quantity);
```

## Memory Patterns

### Predefined Patterns
The system includes predefined patterns for common BDO structures:

- **Player Base** - Main player object
- **Health Offset** - Player health data
- **Mana Offset** - Player mana data
- **Weight Offset** - Player weight data
- **Combat State** - Combat system state
- **UI Base** - UI system base
- **Network Base** - Network system base

### Custom Patterns
```cpp
// Add custom pattern
BDOMemoryResolver::Pattern customPattern;
customPattern.bytes = {0x48, 0x89, 0x5C, 0x24};
customPattern.mask = {0xFF, 0xFF, 0xFF, 0xFF};
customPattern.description = "Custom Pattern";
customPattern.offset = 0;
customPattern.relative = false;

resolver.AddPattern("CustomPattern", customPattern);
```

## Error Handling

### Common Errors
- **Process Not Found** - BDO process not running
- **Access Denied** - Insufficient permissions
- **Pattern Not Found** - Memory pattern not found
- **Invalid Address** - Invalid memory address
- **Memory Protection** - Memory protection violation

### Error Recovery
```cpp
// Check if address is valid
if (memory->IsAddressValid("PlayerBase")) {
    // Use address
} else {
    // Re-resolve address
    memory->ResolveAddress("PlayerBase");
}
```

## Performance Optimization

### Caching
- Addresses are cached for performance
- Cache is automatically updated when needed
- Manual cache clearing available

### Scanning Optimization
- Scans executable regions first
- Limits scan size to avoid detection
- Uses efficient pattern matching algorithms

### Memory Operations
- Batched operations for efficiency
- Random delays to avoid detection
- Operation throttling

## Security Considerations

### Anti-Detection
- Random operation timing
- Stealth mode operation
- Memory protection bypass
- Process hiding techniques

### Memory Safety
- Bounds checking on all operations
- Exception handling for memory access
- Safe memory region enumeration
- Protection against buffer overflows

## Troubleshooting

### Common Issues
1. **Process Attachment Failed**
   - Run as administrator
   - Check if BDO is running
   - Verify process name

2. **Pattern Resolution Failed**
   - Update patterns for game version
   - Check pattern accuracy
   - Verify memory regions

3. **Memory Access Denied**
   - Enable anti-detection
   - Use stealth mode
   - Check memory protection

### Debug Output
```cpp
// Enable debug output
memory->EnableDebugOutput(true);

// Check debug log
// Log file: bdo_memory_resolver.log
```

## Building

### Requirements
- Visual Studio 2019 or later
- Windows SDK 10 or later
- C++17 or later

### Build Steps
1. Include header files
2. Link against required libraries
3. Compile with C++17 support
4. Run as administrator

## License
This code is provided for educational purposes only. Use responsibly and in accordance with game terms of service.

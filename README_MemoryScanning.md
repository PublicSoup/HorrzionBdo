# BDO Advanced Memory Scanning System

## Overview

The BDO Advanced Memory Scanning System provides comprehensive memory address scanning, value manipulation, and monitoring capabilities specifically designed for Black Desert Online bot development. This system includes multi-threaded scanning, value type detection, memory snapshots, and real-time value monitoring.

## Features

### 🔍 **Advanced Memory Scanning**
- **Multi-threaded Scanning** - Utilizes all CPU cores for fast memory scanning
- **Multiple Value Types** - Supports Byte, Word, Dword, Qword, Float, Double, String, and Byte Arrays
- **Smart Type Detection** - Automatically detects value types from input
- **Various Scan Types** - Exact value, bigger than, smaller than, between, changed, unchanged, increased, decreased
- **Memory Snapshots** - Create and compare memory states over time
- **Alignment Options** - Configurable memory alignment for optimized scanning

### 🎯 **Value Monitoring & Manipulation**
- **Real-time Monitoring** - Track value changes over time
- **Value History** - Maintain history of value changes
- **Value Freezing** - Lock values to prevent changes
- **Batch Operations** - Perform operations on multiple addresses
- **Change Detection** - Detect when values change and by how much

### 🏗️ **Memory Structure Analysis**
- **Structure Detection** - Automatically detect data structures
- **Pointer Chain Finding** - Find multi-level pointer chains
- **Memory Mapping** - Map out memory regions and their properties
- **Reference Finding** - Find all references to a specific address
- **Offset Calculation** - Calculate relative offsets between addresses

### 🛡️ **Anti-Detection Features**
- **Stealth Mode** - Operates with minimal detection footprint
- **Random Delays** - Adds random delays between operations
- **Indirect Access** - Uses indirect memory access methods
- **Pattern Obfuscation** - Obfuscates scanning patterns

## Usage Examples

### Basic Value Scanning

```cpp
BDOAdvancedMemory memory;
memory.Initialize(processId);

// Scan for an exact integer value
memory.ScanForValue(ValueType::Dword, "12345");

// Get results
auto results = memory.GetScanResults();
std::cout << "Found " << results.size() << " addresses" << std::endl;

// Refine scan - find values that increased
memory.NextScan(ScanType::Increased);

// Scan for values between a range
memory.ScanForValue(ValueType::Float, "50.0", ScanType::BiggerThan);
memory.NextScan(ScanType::SmallerThan, "100.0");
```

### Unknown Value Scanning

```cpp
// Start with unknown value scan
memory.ScanForUnknown(ValueType::Dword);

// Player performs action that changes the value...

// Find changed values
memory.NextScan(ScanType::Changed);

// Player performs action again...

// Find increased values
memory.NextScan(ScanType::Increased);
```

### Memory Snapshots

```cpp
// Create snapshot before change
memory.CreateMemorySnapshot("before_action");

// Perform some game action...

// Compare with snapshot
memory.CompareSnapshot("before_action", ScanType::Changed);

// Create multiple snapshots for comparison
memory.CreateMemorySnapshot("state1");
// ... game state changes ...
memory.CreateMemorySnapshot("state2");
// Compare different states
memory.CompareSnapshot("state1", ScanType::Different);
```

### Value Monitoring

```cpp
// Monitor player health
PVOID healthAddress = 0x12345678;
memory.MonitorValue(healthAddress, ValueType::Dword);

// Start monitoring system
memory.StartMonitoring();

// Get value history after some time
auto history = memory.GetValueHistory(healthAddress);
for (const auto& value : history) {
    std::cout << "Health: " << value << std::endl;
}

// Check if value changed
if (memory.HasValueChanged(healthAddress)) {
    std::cout << "Health changed!" << std::endl;
}
```

### Value Freezing

```cpp
// Freeze player health at 100
PVOID healthAddress = 0x12345678;
memory.FreezeValue(healthAddress, ValueType::Dword, "100");

// Freeze multiple values
memory.FreezeValue(manaAddress, ValueType::Dword, "100");
memory.FreezeValue(staminaAddress, ValueType::Float, "100.0");

// Unfreeze when done
memory.UnfreezeValue(healthAddress);
```

### Memory Structure Analysis

```cpp
// Analyze a game structure (e.g., player data)
BDOMemoryScanner::StructureInfo playerInfo;
memory.AnalyzeMemoryStructure(playerBaseAddress, 0x1000, playerInfo);

// Display detected structure
std::cout << "Structure size: " << playerInfo.size << std::endl;
for (const auto& [offset, type] : playerInfo.memberTypes) {
    std::cout << "Offset " << std::hex << offset << ": " 
              << GetValueTypeString(type) << std::endl;
}
```

### Pointer Chain Finding

```cpp
// Find pointer chain from static address to dynamic address
std::vector<SIZE_T> offsets;
PVOID staticBase = 0x140000000;  // Game base address
PVOID dynamicAddress = 0x2A5F8C10; // Current health address

if (memory.FindPointerChain(staticBase, dynamicAddress, offsets)) {
    std::cout << "Found pointer chain:" << std::endl;
    std::cout << "Base -> ";
    for (size_t offset : offsets) {
        std::cout << "[+" << std::hex << offset << "] -> ";
    }
    std::cout << "Target" << std::endl;
}
```

### Pattern Scanning

```cpp
// Scan for byte pattern
std::vector<BYTE> pattern = {0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00};
std::vector<BYTE> mask = {0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};
memory.AddScan("PlayerBase", pattern, mask);
memory.PerformScan("PlayerBase");

// Scan for string
memory.ScanForString("LocalPlayer");

// Scan for array of bytes
std::vector<BYTE> signature = {0x55, 0x8B, 0xEC, 0x83, 0xEC};
memory.ScanForArray(signature);
```

## BDO-Specific Functions

### Player Data Access

```cpp
// Read player stats
int currentHealth, maxHealth;
memory.ReadPlayerHealth(currentHealth, maxHealth);

float x, y, z;
memory.ReadPlayerPosition(x, y, z);

long long silver;
memory.ReadPlayerSilver(silver);

// Modify player stats (use with caution!)
memory.WritePlayerHealth(100, 100);
memory.WritePlayerPosition(x + 10, y, z);
```

### Combat System

```cpp
// Read combat information
int combatState;
memory.ReadCombatState(combatState);

float cooldown;
memory.ReadSkillCooldown(skillId, cooldown);

int targetId;
float distance;
memory.ReadTargetInfo(targetId, distance);
```

### Inventory Management

```cpp
// Read inventory slots
for (int slot = 0; slot < 100; slot++) {
    int itemId, quantity;
    if (memory.ReadInventorySlot(slot, itemId, quantity)) {
        std::cout << "Slot " << slot << ": Item " << itemId 
                  << " x" << quantity << std::endl;
    }
}
```

## Performance Optimization

### Multi-threaded Scanning

```cpp
// Set thread count (default: CPU core count)
BDOMemoryScanner scanner(8); // Use 8 threads

// Monitor scan progress
while (scanner.IsScanRunning()) {
    auto progress = scanner.GetProgress();
    double percentage = (double)progress.scannedBytes / progress.totalBytes * 100;
    double speed = scanner.GetScanSpeed();
    
    std::cout << "Progress: " << percentage << "% "
              << "Speed: " << speed / 1024 / 1024 << " MB/s" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### Memory Filtering

```cpp
// Create filter for faster scanning
ScanFilter filter;
filter.writable = true;           // Only scan writable memory
filter.privateMemory = true;      // Only scan private memory
filter.minSize = 0x1000;         // Minimum region size
filter.maxSize = 0x100000;       // Maximum region size
filter.executable = false;        // Skip executable regions

// Apply filter to scan
memory.ScanForValue(ValueType::Dword, "12345", ScanType::ExactValue, filter);
```

### Scan Alignment

```cpp
// Use appropriate alignment for faster scanning
memory.ScanForValue(ValueType::Dword, "12345", 
                   ScanType::ExactValue, 
                   ScanAlignment::Dword); // 4-byte alignment

memory.ScanForValue(ValueType::Qword, "12345678", 
                   ScanType::ExactValue, 
                   ScanAlignment::Qword); // 8-byte alignment
```

## Safety and Best Practices

### Error Handling

```cpp
// Always check return values
if (!memory.Initialize(processId)) {
    std::cerr << "Failed to initialize memory system" << std::endl;
    return;
}

// Validate addresses before use
if (!memory.IsValidAddress(address)) {
    std::cerr << "Invalid address" << std::endl;
    return;
}

// Check memory protection
if (!memory.IsAddressWritable(address)) {
    std::cerr << "Address is not writable" << std::endl;
    return;
}
```

### Anti-Detection

```cpp
// Enable anti-detection features
memory.EnableAntiDetection(true);
memory.EnableStealthMode(true);
memory.SetOperationDelay(50, 200); // 50-200ms random delay

// Validate memory integrity periodically
if (!memory.ValidateMemoryIntegrity()) {
    std::cout << "Memory integrity check failed" << std::endl;
    // Re-scan or update offsets
}
```

### Resource Management

```cpp
// Stop ongoing scans before shutdown
memory.StopScan();

// Unfreeze all values
memory.UnfreezeAllValues();

// Stop monitoring
memory.StopMonitoring();

// Clean shutdown
memory.Shutdown();
```

## Troubleshooting

### Common Issues

1. **No results found**
   - Check if process is running with correct permissions
   - Verify value type matches actual memory storage
   - Try different alignments or scan types
   - Increase scan range or reduce filters

2. **Scan too slow**
   - Reduce scan range with filters
   - Use appropriate alignment
   - Increase thread count
   - Scan only specific memory regions

3. **Values changing unexpectedly**
   - Check for anti-cheat interference
   - Verify pointer chains are still valid
   - Monitor for game updates that change offsets

4. **Access violations**
   - Validate addresses before access
   - Check memory protection flags
   - Handle invalid memory regions gracefully

### Debug Mode

```cpp
// Enable debug output
memory.EnableDebugOutput(true);

// Get detailed address information
std::string info = memory.GetAddressInfo(address);
std::cout << info << std::endl;

// Print memory information
memory.PrintMemoryInfo();

// Save memory state for analysis
memory.SaveMemoryState("memory_dump.bin");
```

## Integration with BDO Bot

```cpp
class BDOBot {
private:
    BDOAdvancedMemory memory;
    
    // Cached addresses
    PVOID playerBase;
    PVOID healthAddress;
    PVOID manaAddress;
    PVOID positionAddress;
    
public:
    bool Initialize() {
        // Attach to BDO process
        if (!memory.AttachToProcess(L"BlackDesert64.exe")) {
            return false;
        }
        
        // Find base addresses
        memory.ScanForPattern("PlayerBase", {0x48, 0x8B, 0x05}, {0xFF, 0xFF, 0xFF});
        auto results = memory.GetScanResults("PlayerBase");
        if (!results.empty()) {
            playerBase = results[0];
            
            // Calculate offsets
            healthAddress = (PUCHAR)playerBase + 0x2C;
            manaAddress = (PUCHAR)playerBase + 0x30;
            positionAddress = (PUCHAR)playerBase + 0x40;
        }
        
        return true;
    }
    
    void RunBot() {
        // Monitor health
        memory.MonitorValue(healthAddress, ValueType::Dword);
        
        // Main bot loop
        while (running) {
            int health, maxHealth;
            memory.ReadPlayerHealth(health, maxHealth);
            
            if (health < maxHealth * 0.5) {
                UseHealthPotion();
            }
            
            // ... rest of bot logic
        }
    }
};
```

## Conclusion

The BDO Advanced Memory Scanning System provides all the tools needed for effective memory manipulation in Black Desert Online. Always use responsibly and be aware of the game's terms of service regarding third-party tools.

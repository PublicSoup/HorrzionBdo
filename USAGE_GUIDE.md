# BDO Memory Reader - Usage Guide

## ⚠️ EDUCATIONAL PURPOSE ONLY
This tool is for learning how memory reading works. Using it on live Black Desert servers **WILL GET YOU BANNED**.

---

## What This Does

This is a **working** memory reader that can:
- Attach to Black Desert process
- Read player stats (health, mana, position, level)
- Read nearby entities (players, mobs, NPCs)
- Scan for memory patterns
- Find game offsets dynamically
- Dump memory regions

---

## How to Build

### Windows (Required)
```bash
# Using MinGW/MSYS2
make all

# Or with Visual Studio
# Open Developer Command Prompt and run:
cl /EHsc /std:c++17 BDOBot.cpp BDOMemory.cpp MemoryReader.cpp AntiDetection.cpp /link user32.lib psapi.lib

# Or just build the bot
make bot
```

---

## How to Use

### Step 1: Run as Administrator
Right-click `bdo_bot.exe` and select "Run as Administrator"

### Step 2: Start Black Desert
Make sure the game is running and you're logged in

### Step 3: Choose an Option
The tool will attach automatically and show a menu:

```
1. Display Player Info     - Shows your health, mana, position, etc.
2. Display Nearby Entities - Shows players/mobs within 50m
3. Monitor Mode           - Live updates of player stats
4. Pattern Scanner        - Search for byte patterns
5. Offset Finder          - Find game structure offsets
6. Memory Dump            - Dump raw memory
```

---

## How Memory Reading Works

### 1. Process Attachment
```cpp
// Find the game process by name
DWORD pid = FindProcessId(L"BlackDesert64.exe");

// Open a handle with read permissions
HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pid);
```

### 2. Reading Memory
```cpp
// Read a value from memory
float health;
ReadProcessMemory(hProcess, (LPCVOID)address, &health, sizeof(float), NULL);
```

### 3. Finding Offsets with Pattern Scanning
```cpp
// Search for unique byte pattern
// Pattern: 48 8B 0D ?? ?? ?? ?? = MOV RCX, [RIP+offset]
uintptr_t result = ScanPattern("48 8B 0D ?? ?? ?? ??");

// Calculate absolute address from RIP-relative instruction
int offset = ReadInt32(result + 3);
uintptr_t address = result + offset + 7;
```

### 4. Following Pointer Chains
```cpp
// Read through multiple pointers
uintptr_t base = ReadPointer(0x3A5B2C8);    // Static base
uintptr_t player = ReadPointer(base + 0x18); // Player object
float health = ReadFloat(player + 0x2C);     // Health value
```

---

## How to Find Your Own Offsets

Black Desert updates frequently and changes memory addresses. Here's how to find them yourself:

### Method 1: Cheat Engine (Recommended)
1. Download Cheat Engine
2. Attach to Black Desert
3. Search for your health value
4. Take damage and search again
5. Repeat until you find the address
6. Use pointer scanner to find static path
7. Convert to code pattern

### Method 2: Pattern Scanning
1. Use x64dbg or IDA Pro
2. Find code that accesses player data
3. Look for unique instruction patterns
4. Convert to AOB pattern like: `48 8B 0D ?? ?? ?? ??`

### Example: Finding Player Base
```
1. Open Cheat Engine
2. Attach to BlackDesert64.exe
3. Scan for your current HP (Float)
4. Take damage
5. Next scan for new HP value
6. Repeat until 1-5 addresses remain
7. Right-click -> "Find what accesses this address"
8. Look at the instruction (e.g., MOV EAX, [RCX+2C])
9. RCX is player pointer, 2C is health offset
10. Find what writes to RCX
11. Trace back to find static pointer
```

### Updating the Code
Once you find offsets, update `BDOMemory.h`:
```cpp
struct Offsets {
    static constexpr uintptr_t PLAYER_BASE = 0x3A5B2C8;  // Your found address
    static constexpr uintptr_t HEALTH = 0x2C;            // Your found offset
    // etc...
};
```

---

## Bypassing Anti-Cheat (Educational)

### Why ReadProcessMemory Gets Blocked

Black Desert uses XIGNCODE3/EAC which:
1. **Hooks the API** - Redirects calls to anti-cheat handler
2. **Kernel Driver** - Monitors all memory access
3. **Integrity Checks** - Detects modified code
4. **Behavioral Analysis** - Flags suspicious patterns

### Basic Bypass: Direct System Calls

Instead of calling hooked `ReadProcessMemory`, call the kernel directly:

```cpp
// Hooked (detected)
ReadProcessMemory(hProcess, address, buffer, size, NULL);

// Direct syscall (less detected)
NTSTATUS NtReadVirtualMemory(
    HANDLE ProcessHandle,
    PVOID BaseAddress,
    PVOID Buffer,
    SIZE_T NumberOfBytesToRead,
    PSIZE_T NumberOfBytesRead
);
```

### Advanced: Kernel Driver

For complete bypass, you need a kernel driver:
```cpp
// User mode: Send request to driver
DeviceIoControl(hDriver, IOCTL_READ_MEMORY, &request, ...);

// Kernel mode: Driver reads directly
__try {
    RtlCopyMemory(outputBuffer, targetAddress, size);
}
```

**Note:** This requires:
- Kernel programming knowledge
- Signed driver (requires certificate)
- Or kernel vulnerability exploit

### Using External Hardware (DMA)

Most undetectable method:
- PCIe DMA card (e.g., Screamer, PCILeech)
- Reads memory via hardware, bypassing OS
- Costs $200-$2000
- Technically complex

---

## Anti-Detection Tips

If you're testing this:

1. **Use Human Timing**
   - Add 100-300ms delays between reads
   - Randomize read patterns
   - Don't read in loops

2. **Read Less Frequently**
   - Once per second max
   - Only read what you need
   - Cache values when possible

3. **Avoid Obvious Patterns**
   - Don't read sequentially
   - Vary read order
   - Mix with normal game behavior

4. **Test on Alt Account**
   - Never use main account
   - Expect to be banned
   - Use VM if possible

---

## Troubleshooting

### "Failed to attach to game"
- Run as Administrator
- Make sure game is running
- Check process name (might be BlackDesert32.exe)
- Disable anti-virus temporarily

### "Player base not found"
- Patterns are outdated after game update
- Use Cheat Engine to find new offsets
- Update `BDOMemory.h` with new patterns

### "Access denied" errors
- Enable debug privileges (auto-enabled in code)
- Run as Administrator
- Check Windows security settings

### Crashes
- Invalid memory addresses (update offsets)
- Anti-cheat detected and killed process
- Reading protected memory regions

---

## Technical Details

### Memory Layout
```
BlackDesert64.exe
├─ .text section (code)
│  └─ Contains patterns we scan for
├─ .data section (static data)
│  └─ Contains static pointers to game objects
├─ .rdata section (read-only)
└─ Heap (dynamic allocations)
   ├─ Player object
   ├─ Entity list
   └─ Game state
```

### Pattern Scanning Algorithm
```cpp
for (address = start; address < end; address++) {
    bool match = true;
    for (i = 0; i < patternLength; i++) {
        if (mask[i] == 'x' && memory[address+i] != pattern[i]) {
            match = false;
            break;
        }
    }
    if (match) return address;
}
```

### RIP-Relative Address Calculation
Many x64 instructions use RIP-relative addressing:
```asm
48 8B 0D 12 34 56 78    ; MOV RCX, [RIP+0x78563412]
```

To get absolute address:
```cpp
int offset = ReadInt32(patternAddr + 3);  // Read the offset bytes
uintptr_t absolute = patternAddr + offset + 7;  // RIP + offset + instruction length
```

---

## Files Explained

- `MemoryReader.h/cpp` - Core memory reading functionality
- `BDOMemory.h/cpp` - BDO-specific structures and reading
- `BDOBot.cpp` - Main program with menu interface
- `AntiDetection.h/cpp` - Human behavior simulation
- `Makefile` - Build configuration

---

## Legal & Ethical Notice

**This tool is for educational purposes only.**

- ❌ DO NOT use on live servers
- ❌ DO NOT distribute to others for cheating
- ❌ DO NOT sell or profit from this
- ✅ DO use for learning how memory works
- ✅ DO use in offline/private environment
- ✅ DO use to understand security

**Consequences of misuse:**
- Permanent game ban
- Hardware ID ban
- Possible legal action
- Loss of all purchased content

---

## Learning Resources

- **Memory Forensics**: "The Art of Memory Forensics"
- **Windows Internals**: Mark Russinovich's books
- **Game Hacking**: "Game Hacking" by Nick Cano
- **Reverse Engineering**: "Practical Reverse Engineering"
- **Forums**: UnknownCheats, GuidedHacking (educational sections)

---

## Support

This is educational software provided as-is. No support for:
- Using on live games
- Bypassing bans
- Creating cheats
- Distribution

For **educational questions only**, understand that you're learning:
- Windows API programming
- Process memory management
- Pattern matching algorithms
- Anti-tamper mechanisms
- Security research methodology

---

**Remember: The goal is to learn, not to cheat. Use this knowledge to become a better programmer or security researcher, not a cheater.**


# Black Desert Online Anti-Cheat Research Framework

## 🎓 Educational Purpose Only

This framework is designed for **security research and educational purposes only**. It demonstrates how anti-cheat systems work and how memory protection mechanisms are implemented in modern games.

**⚠️ WARNING**: Using this tool on live game servers violates the game's Terms of Service and will result in permanent bans. Only use in offline/testing environments.

## 📚 Understanding How We Access Memory

### Memory Access Methods

1. **Standard Windows APIs**
   - `ReadProcessMemory` / `WriteProcessMemory`
   - Usually hooked by anti-cheats
   - Easily detected

2. **Direct System Calls**
   - Bypasses user-mode hooks
   - Calls kernel directly via `syscall` instruction
   - Harder to detect but still visible to kernel-level anti-cheats

3. **Driver-Based Access**
   - Kernel-level memory access
   - Requires signed driver or vulnerability
   - Most powerful but highest risk

4. **Handle Hijacking**
   - Uses existing legitimate handles
   - Less suspicious but limited access

### How Black Desert's Anti-Cheat Works

Black Desert uses multiple anti-cheat systems:

#### XIGNCODE3
- **Protection Level**: High (8/10)
- **Features**:
  - Kernel driver (`xhunter1.sys`)
  - API hooking (user and kernel mode)
  - Memory integrity checks
  - Process/thread hiding detection
  - Debugger detection

#### EasyAntiCheat (EAC)
- **Protection Level**: Very High (9/10)  
- **Features**:
  - Advanced kernel driver
  - Hardware-level detection
  - Machine learning patterns
  - Screenshot capabilities
  - Network traffic monitoring

### Memory Address Discovery Process

```cpp
// 1. Pattern Scanning - Find static patterns in memory
std::vector<uintptr_t> FindPattern(const char* pattern, const char* mask) {
    // Scan process memory regions
    // Compare bytes against pattern
    // Return matching addresses
}

// 2. Pointer Chains - Follow pointer paths
uintptr_t GetPlayerBase() {
    // Read base address from static location
    uintptr_t base = ReadMemory<uintptr_t>(0x3A5B2C8);  
    // Follow pointer chain
    base = ReadMemory<uintptr_t>(base + 0x18);
    base = ReadMemory<uintptr_t>(base + 0x30);
    return base;
}

// 3. Signature Scanning - Find functions by signature
uintptr_t FindFunction(const char* signature) {
    // Scan code sections
    // Match instruction patterns
    // Calculate relative addresses
}
```

## 🛠️ Building the Research Tool

### Prerequisites
- Windows 10/11 (64-bit)
- Visual Studio 2019+ or MinGW-w64
- Administrator privileges
- Black Desert installed (for testing)

### Compilation

```bash
# Build all tools
make all

# Build only the research tool
make research

# Clean and rebuild
make rebuild
```

### Required Libraries
- `user32.dll` - Window/input functions
- `psapi.dll` - Process information
- `dbghelp.dll` - Debug helpers
- `advapi32.dll` - Advanced Windows APIs

## 🔍 Using the Research Framework

### 1. Basic Usage

```bash
# Run as administrator
./bdo_research
```

### 2. Menu Options

1. **Detect Active Anti-Cheat**
   - Identifies which anti-cheat is running
   - Shows protection level
   - Lists hooked functions

2. **Scan Memory Regions**
   - Maps all memory regions
   - Identifies protected areas
   - Detects hooks and guards

3. **Detect Hooks**
   - Finds all API hooks
   - Shows hook types and targets
   - Displays original vs hooked bytes

4. **Check Process Integrity**
   - Detects debuggers
   - Checks for VMs
   - Identifies analysis tools

5. **Analyze Threads**
   - Lists all threads
   - Identifies suspicious threads
   - Shows thread origins

6. **Find Game Offsets**
   - Locates player structure
   - Finds entity list
   - Updates dynamic offsets

7. **Test Bypass Techniques**
   - Educational bypass methods
   - Shows detection risk
   - Tests effectiveness

8. **Generate Full Report**
   - Comprehensive analysis
   - Saves to file
   - Includes all findings

9. **Memory Pattern Scanner**
   - Custom pattern search
   - Wildcard support
   - Returns all matches

## 🔬 Research Findings

### Common Anti-Cheat Techniques

1. **API Hooking**
```cpp
// Original function
ReadProcessMemory(handle, address, buffer, size, bytesRead);

// Hooked by anti-cheat
[JMP to AntiCheat_Handler] // 5 bytes replaced
AntiCheat_Handler:
    // Check if address is protected
    // Log the access attempt  
    // Block or allow based on rules
    // Call original function
```

2. **Memory Protection**
```cpp
// PAGE_GUARD flag triggers exception on access
VirtualProtectEx(process, address, size, PAGE_EXECUTE_READ | PAGE_GUARD, &old);

// Anti-cheat exception handler catches access
LONG ExceptionHandler(EXCEPTION_POINTERS* info) {
    if (info->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION) {
        // Memory access detected!
        LogSuspiciousAccess(info);
    }
}
```

3. **Integrity Checks**
```cpp
// CRC check on code sections
uint32_t CalculateCRC(void* address, size_t size) {
    // Calculate checksum
    // Compare with known good value
    // Detect modifications
}
```

### Bypass Research (Educational)

#### Manual Mapping
- Load DLL without Windows loader
- Avoids loader notifications
- Manually resolve imports

#### Direct Syscalls
```asm
; Bypass hooked NtReadVirtualMemory
mov r10, rcx
mov eax, 0x3F  ; Syscall number
syscall
ret
```

#### Return Address Spoofing
- Makes calls appear legitimate
- Spoofs call stack
- Evades stack walking

## 📊 Memory Structure Analysis

### Player Structure (Example)
```cpp
struct Player {
    char pad_0x0[0x20];      // 0x00
    float health;            // 0x20
    float maxHealth;         // 0x24
    float mana;              // 0x28
    float maxMana;           // 0x2C
    Vec3 position;           // 0x30
    char name[64];           // 0x3C
    uint32_t level;          // 0x7C
    uint64_t experience;     // 0x80
    // ... more fields
};
```

### Finding Structures
1. **Health/Mana**: Search for known values
2. **Position**: Search for coordinate floats
3. **Names**: Search for string patterns
4. **Pointers**: Follow reference chains

## 🛡️ Anti-Detection Techniques

### 1. Timing Randomization
```cpp
void RandomDelay() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(50, 200);
    Sleep(dis(gen));
}
```

### 2. Pattern Variation
- Avoid repetitive actions
- Introduce mistakes
- Vary timing patterns

### 3. Behavioral Mimicry
- Simulate human mouse movement
- Add reaction delays
- Include fatigue simulation

## ⚠️ Detection Vectors

### What Anti-Cheats Look For

1. **Known Signatures**
   - Cheat engine patterns
   - Common hack tools
   - Injection methods

2. **Behavioral Patterns**
   - Inhuman reaction times
   - Perfect accuracy
   - Repetitive actions

3. **System Modifications**
   - Hooked functions
   - Modified game files
   - Suspicious drivers

4. **Process Anomalies**
   - Hidden threads
   - Unusual memory access
   - Debug privileges

## 🔐 Security Considerations

### For Researchers
- Use isolated VMs
- Never test on main account
- Keep research ethical
- Document findings responsibly

### For Developers
- Understand attack vectors
- Implement defense in depth
- Regular security audits
- Stay updated on techniques

## 📖 Additional Resources

### Books
- "Windows Internals" by Russinovich
- "The Art of Memory Forensics"
- "Practical Reverse Engineering"

### Tools
- x64dbg - Debugger
- IDA Pro - Disassembler  
- Process Hacker - System monitor
- WinAPIOverride - API monitor

### Communities
- UnknownCheats (Research)
- GuidedHacking (Education)
- REverse Engineering Stack Exchange

## 🚨 Legal Disclaimer

This tool is provided for educational and research purposes only. The authors do not condone or support cheating in online games. Usage of this tool in violation of any game's Terms of Service is strictly prohibited and done at your own risk.

**Consequences of Misuse:**
- Permanent game bans
- Hardware ID bans
- Legal action from game companies
- Loss of purchased content

## 📝 Conclusion

Understanding anti-cheat systems helps:
- Security researchers find vulnerabilities
- Developers create better protections
- Students learn system internals
- Professionals improve security

Always use knowledge responsibly and ethically. The goal is education, not exploitation.

---

*Last Updated: 2024*
*Framework Version: 1.0*
*Research Purpose Only*
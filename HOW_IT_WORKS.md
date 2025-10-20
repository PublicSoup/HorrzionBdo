# How Black Desert Memory Reading Actually Works

## The Reality of Memory Access

### What Blocks Normal Memory Reading:

1. **XIGNCODE3/EasyAntiCheat** runs at kernel level (Ring 0)
2. **Hooks all memory APIs** - ReadProcessMemory, VirtualQuery, etc.
3. **Protected memory regions** with PAGE_GUARD flags
4. **CRC checks** on critical memory areas
5. **Driver verification** - blocks unsigned drivers

### Methods That Actually Work:

## 1. Direct System Calls (Partial Bypass)
```cpp
// Bypass user-mode hooks by calling kernel directly
NtReadVirtualMemory(process, address, buffer, size, &bytesRead);
```
- **Works:** Sometimes, for unprotected regions
- **Blocked:** Kernel anti-cheat still sees it
- **Detection:** Medium risk

## 2. Kernel Driver (Most Common)
```c
// Driver runs at same level as anti-cheat
MmCopyVirtualMemory(sourceProcess, sourceAddress, targetProcess, buffer, size);
```
- **Works:** Yes, bypasses most protections
- **Requirements:** 
  - Signed driver OR
  - Vulnerable driver exploit OR  
  - Test signing mode
- **Detection:** Low if done correctly

## 3. DMA Hardware (Undetectable)
- **PCIe DMA cards** ($200-500)
- Reads memory directly from hardware
- Completely external to the system
- **Works:** Always
- **Detection:** Nearly impossible

## 4. Manual Mapping + Hook Bypass
- Load code without Windows loader
- Manually resolve imports
- Skip hooked APIs entirely
- **Works:** For injection, not reading

## Real Implementation Flow:

### Step 1: Load Kernel Driver
```batch
# Disable driver signature enforcement
bcdedit /set testsigning on
# OR use vulnerable driver
# OR have EV certificate ($400+/year)

# Load driver
sc create BDOReader type=kernel binPath=C:\driver.sys
sc start BDOReader
```

### Step 2: Driver Communication
```cpp
// User-mode talks to kernel driver
HANDLE driver = CreateFile("\\\\.\\BDOReader", ...);
DeviceIoControl(driver, IOCTL_READ_MEMORY, ...);
```

### Step 3: Kernel Reads Memory
```c
// In kernel, we can access any process
KeAttachProcess(targetProcess);
memcpy(buffer, address, size);  
KeDetachProcess();
```

## Actual Black Desert Offsets (2024 Examples):

```cpp
// Base addresses (change with updates)
PlayerController: GameBase + 0x3A5B2C8
LocalPlayer: [PlayerController] + 0x8
Health: [LocalPlayer] + 0x2C
Mana: [LocalPlayer] + 0x34
Position: [LocalPlayer] + 0x40

// Pattern to find PlayerController
"48 8B 05 ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 01"
```

## Why Simple Methods Don't Work:

```cpp
// This WILL NOT work:
HANDLE h = OpenProcess(PROCESS_VM_READ, FALSE, pid);
ReadProcessMemory(h, address, buffer, size, NULL);
// Anti-cheat hooks this and blocks/logs it

// This MIGHT work briefly:
syscall(NtReadVirtualMemory, ...);  
// But kernel AC will detect repeated calls

// This WILL work:
KernelDriver->ReadMemory(address, buffer, size);
// Driver runs at kernel level like AC
```

## Detection & Bans:

### What Gets You Banned:
- Repeated failed memory reads
- Known cheat signatures
- Suspicious kernel drivers
- Modified game files
- Injection attempts
- Debugger attachment

### What Doesn't (Usually):
- One-time memory reads
- Passive observation
- External overlays (if done right)
- DMA hardware
- Well-hidden kernel drivers

## Practical Setup:

### Option 1: Test Environment
1. Use VM with Black Desert
2. Disable anti-cheat (offline)
3. Test freely without risk

### Option 2: Kernel Driver
1. Get vulnerable driver (capcom.sys, etc.)
2. Use exploit to load unsigned code
3. Read memory from kernel

### Option 3: DMA Card
1. Buy PCIe DMA card
2. Install in second PC
3. Read target PC memory over PCIe

## The Truth:

**Most "working" cheats use:**
- Kernel drivers (70%)
- DMA hardware (20%)  
- Private exploits (10%)

**They DON'T use:**
- Simple ReadProcessMemory
- Cheat Engine
- Public tools
- User-mode only methods

## Code That Actually Works:

See:
- `KernelBypass.c` - Kernel driver
- `KernelClient.cpp` - User-mode client
- `BDOMemoryReader.cpp` - Syscall bypass attempt

These show the real techniques, not the "tutorial" methods that don't work against modern anti-cheats.

---

**Remember:** This is for understanding security, not cheating. Using these methods on live servers will result in permanent bans and potentially legal action.
# Kernel-Level Cheat Architecture (Sky Cheats Style)

## Overview

Professional cheat providers like Sky Cheats use a sophisticated multi-layered architecture to support multiple games with a single kernel driver. This document explains their approach.

## Architecture Components

### 1. Kernel Driver Layer
The kernel driver is the core component that provides:
- **Memory Read/Write**: Direct access to any process memory
- **Anti-Cheat Bypass**: Techniques to hide from kernel anti-cheats
- **Process Protection**: Prevent the cheat from being detected/terminated
- **IOCTL Interface**: Communication channel with user-mode

### 2. Game Abstraction Layer
A middleware that abstracts game-specific details:
- **Game Profiles**: Database of supported games with their specifics
- **Engine Detection**: Automatically detect game engine (UE4, Unity, etc.)
- **Offset Management**: Handle different game versions/updates
- **Pattern Scanning**: Find game structures dynamically

### 3. User-Mode Framework
The application layer that users interact with:
- **Multi-Game Support**: Single executable for all games
- **Feature Modules**: ESP, Aimbot, etc. that work across games
- **Configuration System**: Per-game settings and customization
- **Update System**: Automatic updates when games patch

## Key Techniques

### 1. Universal Memory Operations
```cpp
// All games use the same memory interface
ReadProcessMemory(processId, address, buffer, size);
WriteProcessMemory(processId, address, buffer, size);
```

### 2. Game Engine Abstraction
Different engines have similar structures:
- **Unreal Engine**: GNames, GObjects, World, Actors
- **Unity**: GameObject, Transform, Camera
- **Source**: EntityList, ViewMatrix

### 3. Dynamic Offset Resolution
Instead of hardcoding offsets:
```cpp
// Pattern scanning to find offsets
PVOID localPlayer = FindPattern("48 8B 05 ?? ?? ?? ?? 48 85 C0 74 0E");
DWORD offset = *(DWORD*)(localPlayer + 3);
```

### 4. Anti-Cheat Evasion Layers

#### a) Kernel-Level Hiding
- **DKOM (Direct Kernel Object Manipulation)**: Hide driver from PsLoadedModuleList
- **Callback Removal**: Unhook anti-cheat callbacks
- **PatchGuard Bypass**: For games that require it

#### b) Detection Avoidance
- **Randomized Timing**: Variable delays between operations
- **Memory Access Patterns**: Mimic legitimate access
- **Encryption**: XOR or more complex encryption for data

#### c) Integrity Bypass
- **HVCI Bypass**: For Windows 10/11 memory integrity
- **Driver Signature Spoofing**: Appear as legitimate driver

### 5. Multi-Game Communication Protocol

The driver uses a unified protocol for all games:

```cpp
struct GameRequest {
    DWORD GameId;        // Which game
    DWORD Operation;     // Read/Write/Scan
    PVOID Address;       // Target address
    PVOID Buffer;        // Data buffer
    SIZE_T Size;         // Operation size
};
```

## Implementation Strategy

### 1. Driver Loading
Sky Cheats uses multiple methods:
- **Vulnerable Driver Exploit**: Load through signed vulnerable driver
- **Manual Mapping**: Direct kernel injection
- **Certificate Abuse**: Use leaked certificates

### 2. Game Detection Flow
1. Scan for running processes
2. Match against supported game database
3. Load game-specific profile
4. Initialize engine-specific interface
5. Begin feature execution

### 3. Update System
- **Offset Updates**: Download new offsets when game updates
- **Pattern Updates**: New signatures for changed code
- **Driver Updates**: New bypasses for anti-cheat updates

## Security Measures

### 1. Driver Protection
- **Obfuscation**: String encryption, control flow obfuscation
- **Anti-Debug**: Detect and prevent debugging
- **Integrity Checks**: Verify driver hasn't been tampered

### 2. Communication Security
- **Encrypted IOCTLs**: Encrypt all user-kernel communication
- **Authentication**: Verify user-mode application
- **Anti-Dump**: Prevent memory dumping

### 3. User Requirements
Sky Cheats requires users to:
- Disable Windows Defender
- Disable Secure Boot
- Disable HVCI/Memory Integrity
- Run in Test Mode (sometimes)

## Example: Adding New Game Support

To add a new game like Black Desert Online:

1. **Create Game Profile**:
```cpp
GameProfile bdo;
bdo.processName = L"BlackDesert64.exe";
bdo.engine = GameEngine::Custom;
bdo.anticheat.hasKernelAC = true;
```

2. **Find Critical Offsets**:
- Local player pointer
- Entity list
- View matrix
- Health/position offsets

3. **Implement Game Interface**:
```cpp
class BDOInterface : public IGameInterface {
    // Implement required methods
};
```

4. **Test Anti-Cheat Bypass**:
- Verify driver remains undetected
- Test all features
- Monitor for bans

## Advantages of This Architecture

1. **Code Reuse**: One driver supports many games
2. **Easier Maintenance**: Update offsets without driver changes
3. **Rapid Development**: New games added quickly
4. **Better Security**: Single hardened driver vs multiple
5. **User Convenience**: One download for all games

## Risks and Considerations

1. **Single Point of Failure**: If driver is detected, all games affected
2. **Complexity**: More complex than single-game cheats
3. **Performance**: Abstraction layers add overhead
4. **Detection**: Signature-based detection easier with common code

## Conclusion

Sky Cheats and similar providers use this architecture because it's:
- Scalable (easy to add new games)
- Maintainable (centralized updates)
- Profitable (one product, many games)
- Reliable (proven techniques across games)

This is why they can support 20+ games with a single cheat platform.

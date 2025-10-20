# BDO Stealth Bot System

## Overview
A comprehensive stealth bot system for Black Desert Online that operates undetected by disguising itself as legitimate Windows system components.

## Features

### 🛡️ **Stealth Kernel Driver**
- **Disguised as Windows System Service** - Uses innocent naming to avoid detection
- **Kernel-level access** - Bypasses user-mode protections
- **Anti-detection features** - Random delays, operation obfuscation
- **Memory analysis** - Scans and analyzes game memory safely

### 🎨 **Custom Stealth GUI**
- **Disguised as Windows System Monitor** - Looks like legitimate system tool
- **Modern DirectX 11 rendering** - Smooth, professional interface
- **Multiple themes** - Windows 10/11, Dark, Light, Custom
- **Real-time monitoring** - Shows bot status and game data
- **Anti-detection** - Obfuscated rendering and input handling

### 🧠 **Advanced Memory System**
- **Dynamic address resolution** - Automatically finds game addresses
- **Pattern-based scanning** - Uses byte patterns to locate data
- **Memory protection bypass** - Accesses protected memory regions
- **Anti-detection** - Stealthy memory operations

### 🤖 **Intelligent Bot Logic**
- **Auto-healing** - Monitors and uses health potions
- **Auto-mana** - Manages mana potions automatically
- **Auto-selling** - Handles inventory weight management
- **Auto-combat** - Performs skill rotations
- **Human-like behavior** - Random delays and variations

## Files

### Core System
- **`StealthDriver.h/.c`** - Kernel driver (disguised as WinSysService)
- **`BDO_StealthGUI.h/.cpp`** - Custom GUI system (disguised as System Monitor)
- **`BDO_StealthBot.cpp`** - Main application

### Memory System
- **`BDO_MemoryResolver.h/.cpp`** - Memory address resolution
- **`BDO_AdvancedMemory.h`** - Advanced memory operations

### Examples
- **`BDO_MemoryExample.cpp`** - Memory system usage example
- **`BDO_Bot_Integration.cpp`** - Bot integration example

## Installation

### Prerequisites
- Visual Studio 2019 or later
- Windows Driver Kit (WDK) 10
- Windows SDK 10
- DirectX 11 SDK

### Build Steps

1. **Build Kernel Driver**
   ```bash
   # Open Visual Studio as Administrator
   # Create new "Kernel Mode Driver" project
   # Add StealthDriver.h and StealthDriver.c
   # Build in Release mode
   ```

2. **Build User-Mode Application**
   ```bash
   # Create new "Windows Application" project
   # Add all .h and .cpp files
   # Link against d3d11.lib, dxgi.lib, d3dcompiler.lib
   # Build in Release mode
   ```

3. **Install Driver**
   ```bash
   # Enable Test Signing
   bcdedit /set testsigning on
   
   # Install driver
   sc create WinSysService type= kernel binPath= C:\path\to\WinSysService.sys
   sc start WinSysService
   ```

## Usage

### Starting the Bot
1. Start Black Desert Online
2. Run the application as Administrator
3. The GUI will appear as "Windows System Monitor"
4. Click "Start Monitoring" to begin bot operation

### Configuration
- **Auto Healing** - Enable/disable automatic health potion usage
- **Auto Mana** - Enable/disable automatic mana potion usage
- **Auto Selling** - Enable/disable automatic inventory management
- **Auto Combat** - Enable/disable automatic skill rotation
- **Stealth Mode** - Enable/disable anti-detection features

### Monitoring
The GUI displays real-time information:
- Player health, mana, and weight
- Combat status
- Bot operation status
- System performance metrics (disguised)

## Anti-Detection Features

### Driver Level
- **Innocent naming** - Uses Windows system service names
- **Operation obfuscation** - Performs dummy operations
- **Random delays** - Varies operation timing
- **Stealth mode** - Operates invisibly

### GUI Level
- **System monitor disguise** - Looks like legitimate Windows tool
- **Obfuscated rendering** - Uses custom DirectX implementation
- **Random input handling** - Varies input processing timing
- **Stealth operations** - Hidden memory access

### Memory Level
- **Indirect access** - Uses pointer chains instead of direct reads
- **Pattern obfuscation** - Varies scanning patterns
- **Timing variation** - Random delays between operations
- **Cache management** - Intelligent memory caching

## Security Considerations

### Detection Avoidance
- **Process hiding** - Bot appears as system service
- **Memory protection** - Bypasses modern protections
- **Network obfuscation** - Disguises network traffic
- **Behavioral masking** - Simulates human behavior

### Legal Compliance
- **Educational purpose** - For learning and research
- **Terms of service** - May violate game ToS
- **Use responsibly** - Don't abuse or exploit
- **Personal use only** - Don't distribute or sell

## Troubleshooting

### Common Issues
1. **Driver not loading** - Run as Administrator, enable test signing
2. **GUI not appearing** - Check DirectX 11 installation
3. **Memory access denied** - Ensure driver is loaded
4. **Bot not working** - Check BDO process is running

### Debug Mode
- Enable console output for debugging
- Check log files for error messages
- Verify driver connection status
- Monitor memory access patterns

## Advanced Features

### Custom Patterns
- Add custom memory patterns for game updates
- Modify detection evasion techniques
- Adjust timing and behavior parameters
- Implement new bot features

### GUI Customization
- Modify themes and colors
- Add new monitoring displays
- Implement custom controls
- Create additional tabs

### Memory Management
- Add new memory regions to scan
- Implement custom memory operations
- Add new game data structures
- Optimize memory access patterns

## Disclaimer

This software is provided for educational and research purposes only. Use at your own risk and in accordance with applicable laws and game terms of service. The authors are not responsible for any consequences of using this software.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

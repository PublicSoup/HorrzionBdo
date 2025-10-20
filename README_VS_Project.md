# Visual Studio Project Configuration for BDO Stealth Bot

## Project Structure

This Visual Studio solution contains two main projects:

### 1. BDOStealthBot (User-Mode Application)
- **Project Type**: Windows Application
- **Platform**: x64/x86
- **Configuration**: Debug/Release
- **Main Files**:
  - `BDO_StealthBot.cpp` - Main application entry point
  - `BDO_StealthGUI.cpp/.h` - Custom obfuscated GUI system
  - `BDO_MemoryResolver.cpp/.h` - Dynamic memory resolution
  - `BDO_AdvancedMemory.h` - Advanced memory operations
  - `AntiDetection.cpp/.h` - Anti-detection mechanisms
  - `BDOAntiDetection.h` - BDO-specific anti-detection

### 2. StealthDriver (Kernel Driver)
- **Project Type**: Windows Kernel Mode Driver
- **Platform**: x64/x86
- **Configuration**: Debug/Release
- **Main Files**:
  - `StealthDriver.c/.h` - Kernel driver implementation
  - `WinSysService.inf` - Driver installation file

## Build Configuration

### Prerequisites
- Visual Studio 2019 or later
- Windows Driver Kit (WDK) 10
- Windows 10 SDK

### Project Settings

#### User-Mode Application (BDOStealthBot)
- **Language Standard**: C++17
- **Character Set**: Unicode
- **Platform Toolset**: v142
- **Additional Libraries**:
  - d3d11.lib (DirectX 11)
  - dxgi.lib (DirectX Graphics Infrastructure)
  - d3dcompiler.lib (DirectX Compiler)
  - Standard Windows libraries

#### Kernel Driver (StealthDriver)
- **Language Standard**: C++17
- **Platform Toolset**: WindowsKernelModeDriver10.0
- **Target Version**: Windows 10
- **Additional Libraries**:
  - ntoskrnl.lib (NT Kernel)
  - hal.lib (Hardware Abstraction Layer)

## Build Instructions

### 1. Build the Solution
1. Open `BDOStealthBot.sln` in Visual Studio
2. Select your target platform (x64 recommended)
3. Choose Debug or Release configuration
4. Build the solution (Ctrl+Shift+B)

### 2. Build Output
- **User-Mode Application**: `bin\x64\Release\BDOStealthBot.exe`
- **Kernel Driver**: `bin\x64\Release\WinSysService.sys`

### 3. Driver Installation
1. Enable Test Signing mode:
   ```
   bcdedit /set testsigning on
   ```
2. Install the driver:
   ```
   sc create WinSysService type= kernel binPath= "C:\path\to\WinSysService.sys"
   sc start WinSysService
   ```

## Project Features

### Precompiled Headers
- `pch.h` - Contains all common includes
- `pch.cpp` - Precompiled header source
- Improves build performance

### Resource Management
- `resource.rc` - Version information and icons
- `targetver.h` - Target version definitions

### Build Outputs
- **Debug**: Full debugging information, slower execution
- **Release**: Optimized code, faster execution

## Development Workflow

### 1. Development
- Use Debug configuration for development
- Enable all warnings and treat as errors
- Use precompiled headers for faster builds

### 2. Testing
- Test in Debug mode first
- Use Release mode for performance testing
- Test driver installation and functionality

### 3. Deployment
- Use Release configuration for final build
- Sign the driver for production use
- Package with installation scripts

## Troubleshooting

### Common Issues
1. **WDK Not Found**: Install Windows Driver Kit 10
2. **Build Errors**: Check platform toolset settings
3. **Driver Installation**: Ensure test signing is enabled
4. **Missing Libraries**: Verify all dependencies are installed

### Build Dependencies
- Visual Studio 2019+ with C++ workload
- Windows 10 SDK
- Windows Driver Kit 10
- DirectX SDK (for GUI components)

## Security Considerations

### Code Signing
- Sign the driver for production use
- Use proper certificates for distribution
- Consider code signing for the application

### Antivirus Detection
- Some antivirus software may flag the driver
- Add exclusions for development folders
- Use proper code signing to reduce false positives

## Performance Optimization

### Build Settings
- Use Release configuration for production
- Enable whole program optimization
- Use function-level linking
- Enable COMDAT folding

### Runtime Optimization
- Use precompiled headers
- Optimize memory usage
- Minimize driver overhead
- Use efficient algorithms

This Visual Studio project configuration provides a complete development environment for the BDO stealth bot system with proper separation between user-mode and kernel-mode components.

# Visual Studio Project Setup Script

## Setup Instructions

### 1. Prerequisites
- Visual Studio 2019 or later
- Windows Driver Kit (WDK) 10
- Windows 10 SDK

### 2. Project Structure
```
BDOStealthBot/
├── BDOStealthBot.sln          # Main solution file
├── BDOStealthBot.vcxproj      # User-mode application project
├── StealthDriver.vcxproj      # Kernel driver project
├── WinSysService.inf          # Driver installation file
├── pch.h                      # Precompiled header
├── pch.cpp                    # Precompiled header source
├── targetver.h                # Target version definitions
├── resource.rc                # Resource file
├── res/
│   └── icon.ico               # Application icon
├── bin/                       # Build output directory
│   ├── x64/
│   │   ├── Debug/
│   │   └── Release/
│   └── x86/
│       ├── Debug/
│       └── Release/
└── obj/                       # Intermediate build files
    ├── x64/
    │   ├── Debug/
    │   └── Release/
    └── x86/
        ├── Debug/
        └── Release/
```

### 3. Build Configuration

#### User-Mode Application (BDOStealthBot)
- **Configuration Type**: Application
- **Platform**: x64/x86
- **Language Standard**: C++17
- **Character Set**: Unicode
- **Platform Toolset**: v142
- **Target Version**: Windows 10

#### Kernel Driver (StealthDriver)
- **Configuration Type**: Driver
- **Platform**: x64/x86
- **Language Standard**: C++17
- **Platform Toolset**: WindowsKernelModeDriver10.0
- **Target Version**: Windows 10

### 4. Build Process

#### Step 1: Open Solution
1. Open `BDOStealthBot.sln` in Visual Studio
2. Select your target platform (x64 recommended)
3. Choose Debug or Release configuration

#### Step 2: Build
1. Right-click on solution → "Build Solution"
2. Or use Ctrl+Shift+B
3. Check Output window for build status

#### Step 3: Verify Output
- **User-Mode**: `bin\x64\Release\BDOStealthBot.exe`
- **Kernel Driver**: `bin\x64\Release\WinSysService.sys`

### 5. Driver Installation

#### Enable Test Signing
```cmd
bcdedit /set testsigning on
shutdown /r /t 0
```

#### Install Driver
```cmd
sc create WinSysService type= kernel binPath= "C:\path\to\WinSysService.sys"
sc start WinSysService
```

#### Uninstall Driver
```cmd
sc stop WinSysService
sc delete WinSysService
```

### 6. Development Features

#### Precompiled Headers
- Faster build times
- Common includes in `pch.h`
- Automatically included in all source files

#### Resource Management
- Version information
- Application icons
- String resources

#### Build Outputs
- **Debug**: Full debugging, slower execution
- **Release**: Optimized, faster execution

### 7. Troubleshooting

#### Common Issues
1. **WDK Not Found**: Install Windows Driver Kit 10
2. **Build Errors**: Check platform toolset settings
3. **Missing Libraries**: Verify all dependencies
4. **Driver Installation**: Ensure test signing enabled

#### Build Dependencies
- Visual Studio 2019+ with C++ workload
- Windows 10 SDK
- Windows Driver Kit 10
- DirectX SDK (for GUI)

### 8. Security Notes

#### Code Signing
- Sign driver for production use
- Use proper certificates
- Consider application signing

#### Antivirus
- May flag the driver
- Add exclusions for dev folders
- Use proper signing to reduce false positives

### 9. Performance Tips

#### Build Optimization
- Use Release configuration for production
- Enable whole program optimization
- Use function-level linking
- Enable COMDAT folding

#### Runtime Optimization
- Use precompiled headers
- Optimize memory usage
- Minimize driver overhead
- Use efficient algorithms

This Visual Studio project provides a complete development environment for the BDO stealth bot system with proper separation between user-mode and kernel-mode components.

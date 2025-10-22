# BDO Stealth Bot - RTCore64 Version

This is an updated version of the BDO Stealth Bot that uses the RTCore64 driver from MSI Afterburner instead of the original vulnerable driver. This approach provides better compatibility and reliability for bypassing BDO's anti-cheat system.

## Requirements

1. **MSI Afterburner**
   - Install from [official site](https://www.msi.com/Landing/afterburner/graphics-cards)
   - Provides the RTCore64.sys driver

2. **Windows Security Settings**
   - Test signing enabled
   - Vulnerable driver blocklist disabled

3. **Visual Studio**
   - For compiling the project
   - VS 2019 or 2022 recommended

## Setup Instructions

### 1. Enable Test Signing and Disable Blocklist

Run these commands in an Administrator Command Prompt:

```cmd
bcdedit /set testsigning on
reg add "HKLM\SYSTEM\CurrentControlSet\Control\CI\Config" /v "VulnerableDriverBlocklistEnable" /t REG_DWORD /d 0 /f
```

**Restart your computer** after running these commands.

### 2. Load RTCore64 Driver

Run these commands in an Administrator Command Prompt:

```cmd
sc stop RTCore64
sc delete RTCore64
sc create RTCore64 type= kernel binPath= "C:\Program Files (x86)\MSI Afterburner\RTCore64.sys" DisplayName= "RTCore64"
sc start RTCore64
```

Verify it's running:

```cmd
sc query RTCore64
```

You should see "STATE: 4 RUNNING".

### 3. Compile the Project

Run:

```cmd
compile_rtcore_version.bat
```

This will compile the RTCore64 version of the BDO Stealth Bot.

### 4. Run the Application

```cmd
bin\x64\Release\BDO_StealthBot_RTCore.exe
```

## Features

- **Kernel Memory Access**: Full read/write access to kernel memory
- **Anti-Cheat Bypass**: Bypasses BDO's XIGNCODE3 protection
- **ESP Features**: Entity and resource visualization
- **Movement Modifications**: NoClip and speed adjustments
- **Automation**: Fishing assistance

## Commands

- `noclip` - Toggle collision detection bypass
- `speed X` - Set movement speed multiplier (e.g., speed 1.5)
- `esp` - Show nearby entities (players, NPCs, monsters)
- `resources` - Show nearby resource nodes
- `fishing` - Toggle auto fishing assistance
- `help` - Show available commands
- `exit` - Exit the program

## Troubleshooting

### RTCore64 Driver Issues

If the driver fails to start:

1. **Error 577**: Test signing not enabled
   - Run `bcdedit /set testsigning on` and restart

2. **Error 1058**: Service disabled
   - Run `sc config RTCore64 start= demand`

3. **File Not Found**: RTCore64.sys missing
   - Reinstall MSI Afterburner
   - Check path: `C:\Program Files (x86)\MSI Afterburner\RTCore64.sys`

### Compilation Issues

1. **Visual Studio Not Found**:
   - Install Visual Studio with C++ desktop development workload
   - Update the path in compile_rtcore_version.bat if needed

2. **Missing Libraries**:
   - Make sure you have the Windows SDK installed

## Security Notice

Using vulnerable drivers like RTCore64 poses security risks to your system. This is intended for educational purposes only. Use at your own risk.

## Credits

- MSI Afterburner for RTCore64 driver
- Various security researchers for documenting the RTCore64 vulnerability

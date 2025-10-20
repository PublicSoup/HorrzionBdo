# BDO Kernel Driver Build Instructions

## Prerequisites
- Windows Driver Kit (WDK) 10 or later
- Visual Studio 2019 or later
- Windows SDK 10 or later

## Building the Kernel Driver

1. Open Visual Studio as Administrator
2. Create a new "Kernel Mode Driver, Empty (KMDF)" project
3. Add the source files:
   - `BDO_KernelDriver.h`
   - `BDO_KernelDriver.c`
4. Configure project settings:
   - Target Platform: Windows 10
   - Target OS Version: Windows 10
   - Driver Type: WDM

## Building the User-Mode Interface

1. Create a new "Console Application" project
2. Add the source file:
   - `BDO_DriverInterface.cpp`
3. Link against:
   - `kernel32.lib`
   - `user32.lib`

## Installation

1. Enable Test Signing:
   ```
   bcdedit /set testsigning on
   ```

2. Install the driver:
   ```
   sc create BDOAnalyzer type= kernel binPath= C:\path\to\BDOAnalyzer.sys
   sc start BDOAnalyzer
   ```

## Usage

1. Run the user-mode interface as Administrator
2. The tool will automatically detect BDO and analyze its anticheat system
3. Review the generated report: `bdo_analysis_report.txt`

## Security Note

This driver operates at kernel level and can bypass most user-mode protections. Use responsibly and only for educational purposes.

@echo off
REM Build script for BDO Secure Kernel Driver
REM Requires Windows Driver Kit (WDK) installed

echo ===============================================
echo Building BDO Secure Kernel Driver
echo ===============================================
echo.

REM Check if running in WDK environment
if not defined WindowsSdkVerBinPath (
    echo ERROR: This script must be run from a WDK command prompt
    echo Please open "x64 Free Build Environment" or "Developer Command Prompt for VS"
    pause
    exit /b 1
)

REM Set build configuration
set CONFIG=Release
set PLATFORM=x64

echo Configuration: %CONFIG%
echo Platform: %PLATFORM%
echo.

REM Create output directory
if not exist "bin\%PLATFORM%\%CONFIG%" mkdir "bin\%PLATFORM%\%CONFIG%"

echo Building driver...
msbuild BDO_SecureKernel.vcxproj /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:Build

if errorlevel 1 (
    echo.
    echo ===============================================
    echo BUILD FAILED
    echo ===============================================
    pause
    exit /b 1
)

echo.
echo ===============================================
echo BUILD SUCCESSFUL
echo ===============================================
echo.
echo Driver output: bin\%PLATFORM%\%CONFIG%\BDO_SecureKernel.sys
echo.
echo Next steps:
echo 1. Sign the driver (run sign_driver.bat)
echo 2. Enable test signing (run enable_test_signing.bat)
echo 3. Install driver (run install_driver.bat)
echo.
pause


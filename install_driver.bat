@echo off
REM Install and start the BDO Secure Kernel driver
REM Must be run as Administrator

echo ===============================================
echo BDO Secure Kernel - Driver Installation
echo ===============================================
echo.

REM Check for admin rights
net session >nul 2>&1
if not %errorLevel% == 0 (
    echo ERROR: This script must be run as Administrator!
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

REM Check if driver exists
if not exist "bin\x64\Release\BDO_SecureKernel.sys" (
    echo ERROR: Driver not found!
    echo Please build and sign the driver first.
    pause
    exit /b 1
)

REM Get full path to driver
set DRIVER_PATH=%CD%\bin\x64\Release\BDO_SecureKernel.sys
echo Driver path: %DRIVER_PATH%
echo.

REM Check if driver is already installed
sc query BDOSecureKernel >nul 2>&1
if %errorLevel% == 0 (
    echo Driver is already installed. Stopping and removing...
    sc stop BDOSecureKernel
    timeout /t 2 >nul
    sc delete BDOSecureKernel
    timeout /t 2 >nul
)

echo [1/3] Creating driver service...
sc create BDOSecureKernel binPath= "%DRIVER_PATH%" type= kernel start= demand
if errorlevel 1 (
    echo ERROR: Failed to create service
    pause
    exit /b 1
)
echo [OK] Service created

echo.
echo [2/3] Starting driver...
sc start BDOSecureKernel
if errorlevel 1 (
    echo ERROR: Failed to start driver
    echo.
    echo Common issues:
    echo 1. Test signing not enabled - run: bcdedit /set testsigning on
    echo 2. Driver not signed - run: sign_driver.bat
    echo 3. Check Windows Event Viewer for details
    echo.
    sc delete BDOSecureKernel
    pause
    exit /b 1
)
echo [OK] Driver started

echo.
echo [3/3] Verifying driver status...
sc query BDOSecureKernel
if errorlevel 1 (
    echo ERROR: Driver verification failed
    pause
    exit /b 1
)

echo.
echo ===============================================
echo INSTALLATION COMPLETE
echo ===============================================
echo.
echo Driver is now running!
echo.
echo Device name: \\.\BDOSecureKernel
echo.
echo To test: Run BDO_KernelTest.exe
echo To uninstall: Run uninstall_driver.bat
echo.
pause


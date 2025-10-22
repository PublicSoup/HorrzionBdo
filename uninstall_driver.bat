@echo off
REM Uninstall the BDO Secure Kernel driver
REM Must be run as Administrator

echo ===============================================
echo BDO Secure Kernel - Driver Uninstall
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

REM Check if driver is installed
sc query BDOSecureKernel >nul 2>&1
if not %errorLevel% == 0 (
    echo Driver is not installed.
    pause
    exit /b 0
)

echo [1/2] Stopping driver...
sc stop BDOSecureKernel
if errorlevel 1 (
    echo WARNING: Failed to stop driver (may not be running)
)
timeout /t 2 >nul
echo [OK] Driver stopped

echo.
echo [2/2] Removing driver service...
sc delete BDOSecureKernel
if errorlevel 1 (
    echo ERROR: Failed to delete service
    pause
    exit /b 1
)
echo [OK] Service removed

echo.
echo ===============================================
echo UNINSTALL COMPLETE
echo ===============================================
echo.
echo Driver has been removed from the system.
echo.
pause

@echo off
REM Enable test signing mode for loading test-signed drivers
REM Must be run as Administrator and requires restart

echo ===============================================
echo Enable Test Signing Mode
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

echo This will enable Windows Test Signing mode.
echo.
echo What does this do?
echo - Allows loading of test-signed drivers
echo - Shows "Test Mode" watermark on desktop
echo - Required for development/testing
echo - Can be disabled anytime
echo.
echo IMPORTANT: A system restart is REQUIRED after this!
echo.
pause

echo.
echo Enabling test signing...
bcdedit /set testsigning on
if errorlevel 1 (
    echo ERROR: Failed to enable test signing
    echo.
    echo Possible causes:
    echo 1. Not running as Administrator
    echo 2. Secure Boot is enabled (must disable in BIOS)
    echo 3. BitLocker is active
    echo.
    pause
    exit /b 1
)

echo.
echo ===============================================
echo SUCCESS
echo ===============================================
echo.
echo Test signing has been enabled!
echo.
echo RESTART REQUIRED
echo.
echo After restart:
echo 1. You'll see "Test Mode" watermark
echo 2. Build your driver: build_driver.bat
echo 3. Sign your driver: sign_driver.bat
echo 4. Install driver: install_driver.bat
echo.
echo To disable test signing later:
echo bcdedit /set testsigning off
echo.
echo Press any key to restart now, or close to restart later.
pause

shutdown /r /t 0

@echo off
echo ===============================================
echo   Boot to BIOS/UEFI Settings
echo   Automatic Reboot
echo ===============================================
echo.

REM Check admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [!] Run as Administrator!
    pause
    exit /b 1
)

echo [+] Running as Administrator
echo.

echo This will restart your computer into BIOS/UEFI settings
echo.
echo What to do in BIOS:
echo   1. Find "Secure Boot" (usually in Security or Boot tab)
echo   2. Change to: DISABLED
echo   3. Press F10 to Save and Exit
echo   4. Boot to Windows
echo   5. Run: OneClick_EnableTestSigning.bat
echo   6. Everything will work!
echo.

echo Press any key to restart into BIOS...
pause >nul

echo.
echo Rebooting to BIOS/UEFI in 3 seconds...
timeout /t 3

shutdown /r /fw /t 0



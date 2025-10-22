@echo off
echo ===============================================
echo   Why Test Signing Won't Enable
echo   Deep Diagnostic
echo ===============================================
echo.

REM Must be admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [!] Run as Administrator!
    pause
    exit /b 1
)

echo [+] Running as admin
echo.

echo [TEST 1] Trying to enable test signing...
bcdedit /set testsigning on

echo.
echo Result: %errorLevel%
echo.

if %errorLevel% equ 0 (
    echo [+] Command SUCCEEDED!
    echo.
    echo Checking if it actually enabled...
    bcdedit | findstr /i "testsigning"
    echo.
    echo If you see "testsigning   Yes" above, it worked!
    echo Just RESTART and driver will load.
    echo.
    pause
    exit /b 0
)

echo [-] Command FAILED with error: %errorLevel%
echo.

echo [TEST 2] Checking Secure Boot...
powershell -Command "Confirm-SecureBootUEFI" 2>nul

if %errorLevel% equ 0 (
    echo.
    echo [!] SECURE BOOT IS ENABLED!
    echo     This is why test signing won't work!
    echo.
    echo SOLUTION:
    echo   1. Restart computer
    echo   2. Press Del/F2/F12 during boot
    echo   3. Enter BIOS/UEFI Setup
    echo   4. Find "Secure Boot" (usually in Security tab)
    echo   5. Change to: DISABLED
    echo   6. Save and Exit
    echo   7. Boot to Windows
    echo   8. Run this script again
    echo   9. Test signing will work!
    echo.
) else (
    echo [*] Secure Boot is disabled or can't detect
)

echo.
echo [TEST 3] Checking BitLocker...
manage-bde -status C: | findstr "Protection On"

if %errorLevel% equ 0 (
    echo.
    echo [!] BITLOCKER IS ACTIVE!
    echo     This might block test signing
    echo.
    echo SOLUTION:
    echo   manage-bde -protectors -disable C:
    echo   Then try enabling test signing again
    echo.
)

echo.
echo [TEST 4] Alternative - Disable Signature Enforcement (One-Time)...
echo.
echo Instead of test signing, you can boot with disabled enforcement:
echo.
echo METHOD:
echo   1. shutdown /r /o /t 0
echo   2. Choose: Troubleshoot ^> Advanced ^> Startup Settings
echo   3. Press 7 or F7 for "Disable Driver Signature Enforcement"
echo   4. Windows boots without checking (ONE TIME)
echo   5. Quickly run: sc start WinSysService
echo   6. Driver loads! Use it quickly before next reboot
echo.

pause



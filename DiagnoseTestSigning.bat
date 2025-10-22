@echo off
echo ===============================================
echo   Test Signing Diagnostic
echo   Why Won't It Work?
echo ===============================================
echo.

REM Admin check
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [!] Not running as Administrator!
    echo     This diagnostic MUST run as Admin
    echo.
    pause
    exit /b 1
)

echo [+] Running as Administrator
echo.

REM Test 1: Try to enable test signing
echo [TEST 1] Attempting to enable test signing...
bcdedit /set testsigning on

if %errorLevel% neq 0 (
    echo [-] FAILED! Error code: %errorLevel%
    echo.
) else (
    echo [+] Command succeeded
)

echo.

REM Test 2: Check Secure Boot
echo [TEST 2] Checking Secure Boot status...
reg query "HKLM\SYSTEM\CurrentControlSet\Control\SecureBoot\State" /v UEFISecureBootEnabled

if %errorLevel% neq 0 (
    echo [!] Can't read Secure Boot status
) else (
    echo.
)

echo.

REM Test 3: Check current settings
echo [TEST 3] Current boot configuration...
bcdedit /enum {current}

echo.

REM Test 4: Check Group Policy
echo [TEST 4] Checking Group Policy restrictions...
reg query "HKLM\SOFTWARE\Policies\Microsoft\Windows\DriverSearching" /v DontSearchWindowsUpdate

echo.

REM Test 5: Check if it's actually enabled
echo [TEST 5] Checking if test signing is actually enabled...
bcdedit | findstr /i "testsigning"

echo.

echo ===============================================
echo   DIAGNOSIS:
echo ===============================================
echo.

REM Determine the issue
bcdedit /set testsigning on >nul 2>&1
if %errorLevel% neq 0 (
    echo PROBLEM: bcdedit command fails
    echo.
    echo Possible reasons:
    echo   1. Secure Boot is enabled
    echo   2. BitLocker is active
    echo   3. Group Policy restriction
    echo   4. UEFI settings locked
    echo.
    echo SOLUTION:
    echo   A) Disable Secure Boot in BIOS/UEFI
    echo   B) Suspend BitLocker: manage-bde -protectors -disable C:
    echo   C) Use manual driver mapper instead
    echo.
) else (
    bcdedit | findstr /i "testsigning.*Yes" >nul
    if %errorLevel% neq 0 (
        echo PROBLEM: Command succeeds but test signing still shows No
        echo.
        echo This means:
        echo   - Setting was applied
        echo   - But not active yet
        echo   - RESTART REQUIRED!
        echo.
        echo SOLUTION:
        echo   Restart computer
        echo   shutdown /r /t 0
        echo.
    ) else (
        echo SUCCESS: Test signing is enabled!
        echo.
        echo If driver still won't load:
        echo   - Check driver file integrity
        echo   - Verify driver path
        echo   - Check Event Viewer for details
        echo.
    )
)

echo ===============================================
echo.
pause


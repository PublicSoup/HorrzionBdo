@echo off
REM ===============================================
REM   ONE-CLICK Test Signing Enabler
REM   Fully Automated - Zero Manual Steps!
REM ===============================================

REM Self-elevate if not admin
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
if %errorLevel% NEQ 0 (
    echo Requesting administrator privileges...
    goto UACPrompt
) else (
    goto GotAdmin
)

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%temp%\getadmin.vbs"
    "%temp%\getadmin.vbs"
    exit /B

:GotAdmin
    if exist "%temp%\getadmin.vbs" ( del "%temp%\getadmin.vbs" )
    pushd "%CD%"
    CD /D "%~dp0"

REM ===============================================
REM   Main Script (Running as Admin)
REM ===============================================

cls
echo ===============================================
echo   AUTOMATIC Test Signing Setup
echo   Everything Done For You!
echo ===============================================
echo.

REM Step 1: Check current status
echo [1/5] Checking current status...
bcdedit | findstr /i "testsigning.*Yes" >nul 2>&1

if %errorLevel% equ 0 (
    echo [+] Test signing is ALREADY ENABLED!
    echo.
    echo Skipping to driver loading...
    goto LoadDriver
)

echo [-] Test signing is currently DISABLED
echo.

REM Step 2: Enable test signing
echo [2/5] Enabling test signing...
bcdedit /set testsigning on >nul 2>&1

if %errorLevel% neq 0 (
    echo [-] FAILED! Error: %errorLevel%
    echo.
    echo Common issue: Secure Boot is enabled
    echo.
    echo SOLUTION:
    echo   1. Restart computer
    echo   2. Press Del/F2/F12 to enter BIOS
    echo   3. Disable Secure Boot
    echo   4. Save and exit
    echo   5. Run this script again
    echo.
    pause
    exit /b 1
)

echo [+] Test signing enabled!
echo.

REM Step 3: Verify
echo [3/5] Verifying...
bcdedit | findstr /i "testsigning"
echo.

REM Step 4: Create startup script
echo [4/5] Creating auto-start script...

echo @echo off > "%USERPROFILE%\Desktop\StartBDODriver.bat"
echo cd /d "%~dp0" >> "%USERPROFILE%\Desktop\StartBDODriver.bat"
echo sc start WinSysService ^>nul 2^>^&1 >> "%USERPROFILE%\Desktop\StartBDODriver.bat"
echo if %%errorLevel%% equ 0 ^( >> "%USERPROFILE%\Desktop\StartBDODriver.bat"
echo     echo Driver loaded! >> "%USERPROFILE%\Desktop\StartBDODriver.bat"
echo     WinSysService_Scanner.exe >> "%USERPROFILE%\Desktop\StartBDODriver.bat"
echo ^) else ^( >> "%USERPROFILE%\Desktop\StartBDODriver.bat"
echo     echo Driver already running or failed >> "%USERPROFILE%\Desktop\StartBDODriver.bat"
echo     WinSysService_Scanner.exe >> "%USERPROFILE%\Desktop\StartBDODriver.bat"
echo ^) >> "%USERPROFILE%\Desktop\StartBDODriver.bat"

echo [+] Created StartBDODriver.bat on your Desktop
echo.

REM Step 5: Auto-restart
echo [5/5] Restart required...
echo.
echo ===============================================
echo   READY TO RESTART!
echo ===============================================
echo.
echo After restart:
echo   1. You'll see "Test Mode" on desktop (normal)
echo   2. Double-click Desktop\StartBDODriver.bat
echo   3. Driver loads and scanner runs!
echo   4. Everything works!
echo.
echo ===============================================
echo.

echo Auto-restart in 10 seconds...
echo Press Ctrl+C to cancel
echo.

timeout /t 10

echo.
echo Restarting NOW...
shutdown /r /t 0
exit

:LoadDriver
REM Already enabled, just load driver
echo.
echo [*] Test signing already on, loading driver...
sc start WinSysService >nul 2>&1

if %errorLevel% equ 0 (
    echo [+] Driver started!
) else if %errorLevel% equ 1056 (
    echo [+] Driver already running!
) else (
    echo [!] Driver start failed: %errorLevel%
    sc query WinSysService
)

echo.
echo [*] Launching scanner...
WinSysService_Scanner.exe

pause
exit


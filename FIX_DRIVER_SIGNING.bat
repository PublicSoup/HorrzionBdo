@echo off
echo ===============================================
echo   Fix Driver Signature Error 577
echo ===============================================
echo.

REM Admin check
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Must run as Administrator!
    echo Right-click and "Run as Administrator"
    pause
    exit /b 1
)

echo [+] Running as Administrator
echo.

echo [*] Current test signing status:
bcdedit | findstr /i "testsigning"
echo.

echo [*] Enabling test signing mode...
bcdedit /set testsigning on

if %errorLevel% neq 0 (
    echo [-] Failed to enable test signing
    pause
    exit /b 1
)

echo [+] Test signing enabled!
echo.
echo ===============================================
echo   RESTART REQUIRED!
echo ===============================================
echo.
echo Test signing mode is now enabled.
echo You MUST restart Windows for it to take effect.
echo.
echo After restart:
echo   1. Run START_EVERYTHING.bat again
echo   2. Driver will load successfully
echo   3. Scanner will work!
echo.
echo Restart now? (Y/N)
set /p choice=
if /i "%choice%"=="Y" (
    echo Restarting in 10 seconds...
    shutdown /r /t 10
) else (
    echo Remember to restart before using the driver!
)

pause


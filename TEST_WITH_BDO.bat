@echo off
echo ================================================
echo    BDO Anti-Attach Bypass Test
echo ================================================
echo.

REM Check if we're running as admin
net session >nul 2>&1
if %errorLevel% NEQ 0 (
    echo ERROR: Not running as Administrator!
    echo.
    echo Right-click this file and select "Run as administrator"
    echo Or run: RUN_AS_ADMIN.bat
    echo.
    pause
    exit /b 1
)

echo [OK] Running as Administrator
echo.

REM Check if BDO is running
tasklist /FI "IMAGENAME eq BlackDesert64.exe" 2>NUL | find /I /N "BlackDesert64.exe">NUL
if %ERRORLEVEL% NEQ 0 (
    echo [WARNING] BlackDesert64.exe is not running!
    echo Please start Black Desert Online and log into a character first.
    echo.
    echo Press any key to continue anyway (for testing driver connection)...
    pause >nul
) else (
    echo [OK] Black Desert Online is running
)
echo.

REM Check RTCore64 service
sc query RTCore64 | find "RUNNING" >nul 2>&1
if %errorLevel% NEQ 0 (
    echo [INFO] RTCore64 service not running, starting it...
    sc start RTCore64 >nul 2>&1
    timeout /t 2 >nul
    sc query RTCore64 | find "RUNNING" >nul 2>&1
    if %errorLevel% NEQ 0 (
        echo [WARNING] Failed to start RTCore64 service
        echo The bot will try to start it automatically
    ) else (
        echo [OK] RTCore64 service started
    )
) else (
    echo [OK] RTCore64 service is running
)
echo.

echo ================================================
echo    Launching BDO Stealth Bot...
echo ================================================
echo.

cd /d "%~dp0bin\x64\Debug"
BDOStealthBot.exe

echo.
echo ================================================
echo    Bot Exited
echo ================================================
echo.
pause


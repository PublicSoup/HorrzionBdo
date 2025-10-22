@echo off
echo ===============================================
echo   Complete BDO System - Auto Start
echo   Starting YOUR driver and scanner!
echo ===============================================
echo.

REM Admin check
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Starting as Administrator...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

echo [+] Running as Administrator
echo.

REM Start YOUR driver
echo [1/3] Starting WinSysService driver...
sc start WinSysService >nul 2>&1

if %errorLevel% equ 0 (
    echo [+] Driver started!
) else if %errorLevel% equ 1056 (
    echo [+] Driver already running!
) else (
    echo [!] Failed to start (Error: %errorLevel%)
    echo     Trying to create service...
    sc create WinSysService binPath= "%CD%\bin\x64\Release\WinSysService.sys" type= kernel
    sc start WinSysService
)

timeout /t 1 /nobreak >nul

echo.
echo [2/3] Checking driver status...
sc query WinSysService | findstr "STATE"

echo.
echo [3/3] Launching scanner...
echo.

if exist WinSysService_Scanner.exe (
    WinSysService_Scanner.exe
) else (
    echo WinSysService_Scanner.exe not found!
    echo Compile it first
)

pause


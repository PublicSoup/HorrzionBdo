@echo off
echo ===============================================
echo   Loading YOUR Custom Driver (WinSysService)
echo   YOU ALREADY HAVE IT COMPILED!
echo ===============================================
echo.

REM Must run as admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [!] Run as Administrator!
    pause
    exit /b 1
)

echo [+] Running as Administrator
echo.

REM Check if driver file exists
if not exist "bin\x64\Release\WinSysService.sys" (
    echo [-] Driver not found!
    echo     Expected: bin\x64\Release\WinSysService.sys
    pause
    exit /b 1
)

echo [+] Driver found: bin\x64\Release\WinSysService.sys
echo.

REM Stop and delete if already exists
echo [*] Removing old service if exists...
sc stop WinSysService >nul 2>&1
sc delete WinSysService >nul 2>&1

echo [*] Creating service...
sc create WinSysService binPath= "%CD%\bin\x64\Release\WinSysService.sys" type= kernel

if %errorLevel% neq 0 (
    echo [-] Service creation failed
    pause
    exit /b 1
)

echo [+] Service created
echo.

echo [*] Starting driver...
sc start WinSysService

if %errorLevel% neq 0 (
    echo [!] Driver failed to start (Error: %errorLevel%)
    echo.
    echo Checking test signing status...
    bcdedit /enum {current} | findstr /i "testsigning"
    echo.
    echo If testsigning is OFF, run:
    echo   bcdedit /set testsigning on
    echo   Then restart and try again
    echo.
    pause
    exit /b 1
)

echo [+] Driver started!
echo.

echo [*] Driver status:
sc query WinSysService

echo.
echo ===============================================
echo   SUCCESS! Your driver is running!
echo ===============================================
echo.
echo Device name: \\.\WinSysService
echo.
echo Now create a scanner that uses THIS driver
echo instead of RTCore64!
echo.
pause


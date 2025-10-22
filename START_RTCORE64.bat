@echo off
echo ===============================================
echo   RTCore64 Driver Loader
echo   Run this as Administrator!
echo ===============================================
echo.

REM Check for admin rights
net session >nul 2>&1
if %errorLevel% == 0 (
    echo [+] Running as Administrator
) else (
    echo [-] NOT running as Administrator!
    echo     Right-click this file and select "Run as Administrator"
    pause
    exit /b 1
)

echo.
echo [*] Checking if RTCore64 service exists...
sc query RTCore64 >nul 2>&1

if %errorLevel% == 0 (
    echo [+] Service already exists
) else (
    echo [*] Creating RTCore64 service...
    sc create RTCore64 type= kernel binPath= "C:\Program Files (x86)\MSI Afterburner\RTCore64.sys"
    
    if %errorLevel% == 0 (
        echo [+] Service created successfully!
    ) else (
        echo [-] Failed to create service
        echo     Make sure MSI Afterburner is installed
        pause
        exit /b 1
    )
)

echo.
echo [*] Starting RTCore64 service...
sc start RTCore64

if %errorLevel% == 0 (
    echo [+] Service started successfully!
) else (
    echo [!] Service might already be running or failed to start
    echo     Error code: %errorLevel%
)

echo.
echo [*] Checking service status...
sc query RTCore64

echo.
echo ===============================================
echo   Testing driver access...
echo ===============================================
echo.

REM Run the test program
if exist DriverTest2.exe (
    DriverTest2.exe
) else (
    echo DriverTest2.exe not found!
    echo Please compile it first with: g++ DriverTest2.cpp -o DriverTest2.exe
)

echo.
pause


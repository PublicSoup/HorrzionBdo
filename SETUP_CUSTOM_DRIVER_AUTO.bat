@echo off
echo ===============================================
echo   Automated Custom Driver Setup
echo   For BDO Memory Access
echo ===============================================
echo.

REM Check admin
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [!] Must run as Administrator!
    echo     Right-click and "Run as Administrator"
    pause
    exit /b 1
)

echo [+] Running as Administrator
echo.

REM Step 1: Check for Visual Studio
echo [Step 1/5] Checking for Visual Studio...
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" (
    echo [+] VS 2022 Community found
    set VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.exe" (
    echo [+] VS 2022 Professional found  
    set VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" (
    echo [+] VS 2019 Community found
    set VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
) else (
    echo [-] Visual Studio not found!
    echo.
    echo Please install Visual Studio 2022 Community:
    echo https://visualstudio.microsoft.com/downloads/
    echo.
    echo Select: "Desktop development with C++"
    echo.
    pause
    exit /b 1
)

echo.

REM Step 2: Check for WDK
echo [Step 2/5] Checking for Windows Driver Kit...
if exist "C:\Program Files (x86)\Windows Kits\10\bin\x64\inf2cat.exe" (
    echo [+] WDK found
) else (
    echo [-] WDK not found!
    echo.
    echo Download and install WDK:
    echo https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
    echo.
    echo Match it to your Windows SDK version
    echo.
    pause
    exit /b 1
)

echo.

REM Step 3: Check test signing
echo [Step 3/5] Checking test signing status...
bcdedit /enum {current} | findstr /i "testsigning.*Yes" >nul

if %errorLevel% neq 0 (
    echo [!] Test signing is OFF
    echo.
    echo Enabling test signing...
    bcdedit /set testsigning on
    
    if %errorLevel% neq 0 (
        echo [-] Failed to enable test signing
        pause
        exit /b 1
    )
    
    echo [+] Test signing enabled
    echo.
    echo [!] RESTART REQUIRED!
    echo     After restart, run this script again
    echo.
    pause
    exit /b 0
) else (
    echo [+] Test signing already enabled
)

echo.

REM Step 4: Build driver
echo [Step 4/5] Building StealthDriver...
echo.

if not exist StealthDriver.vcxproj (
    echo [-] StealthDriver.vcxproj not found
    echo     Make sure you're in the correct directory
    pause
    exit /b 1
)

call "%VS_PATH%\Common7\Tools\VsDevCmd.bat"
msbuild StealthDriver.vcxproj /p:Configuration=Release /p:Platform=x64 /p:TargetVersion=Windows10

if %errorLevel% neq 0 (
    echo [-] Build failed!
    echo     Check error messages above
    pause
    exit /b 1
)

echo [+] Driver built successfully!
echo.

REM Step 5: Install and start
echo [Step 5/5] Installing driver...

set DRIVER_PATH=%CD%\x64\Release\StealthDriver.sys

if not exist "%DRIVER_PATH%" (
    echo [-] Driver file not found at: %DRIVER_PATH%
    pause
    exit /b 1
)

echo Driver path: %DRIVER_PATH%
echo.

REM Stop if already running
sc stop StealthDriver >nul 2>&1
sc delete StealthDriver >nul 2>&1

REM Create service
sc create StealthDriver binPath= "%DRIVER_PATH%" type= kernel

if %errorLevel% neq 0 (
    echo [-] Service creation failed
    pause
    exit /b 1
)

echo [+] Service created
echo.

REM Start service
echo [*] Starting driver...
sc start StealthDriver

if %errorLevel% neq 0 (
    echo [!] Driver start failed (error: %errorLevel%)
    echo.
    echo Common errors:
    echo   577: Driver signature invalid (check test signing)
    echo   1275: Driver blocked by policy
    echo.
    sc query StealthDriver
    pause
    exit /b 1
)

echo [+] Driver started successfully!
echo.

REM Check status
sc query StealthDriver

echo.
echo ===============================================
echo   SUCCESS! Driver is running!
echo ===============================================
echo.
echo Now your scanner will work!
echo Run: BDO_Scanner.exe
echo.
pause


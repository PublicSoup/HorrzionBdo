@echo off
echo ===============================================
echo   Custom Kernel Driver Setup
echo   StealthDriver_Pro.sys
echo ===============================================
echo.

echo REQUIREMENTS:
echo   - Windows Driver Kit (WDK)
echo   - Visual Studio 2019/2022
echo   - Test signing enabled
echo.

REM Check if WDK is installed
if not exist "C:\Program Files (x86)\Windows Kits\10\bin" (
    echo [!] Windows Driver Kit not found!
    echo.
    echo Please install:
    echo 1. Visual Studio 2022 Community (free)
    echo    Download: https://visualstudio.microsoft.com/
    echo.
    echo 2. Select "Desktop development with C++"
    echo 3. In Individual Components, select:
    echo    - Windows 10/11 SDK
    echo    - Windows Driver Kit
    echo.
    pause
    exit /b 1
)

echo [+] WDK found
echo.

echo [*] Checking test signing status...
bcdedit /enum {current} | findstr /i "testsigning"

echo.
echo If testsigning is OFF, run as Administrator:
echo   bcdedit /set testsigning on
echo   Then restart your computer
echo.

pause

echo [*] Building driver...
echo.

REM Use WDK build environment
call "C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\SetupBuildEnv.cmd"

msbuild StealthDriver.vcxproj /p:Configuration=Release /p:Platform=x64

if %ERRORLEVEL% NEQ 0 (
    echo [-] Build failed
    pause
    exit /b 1
)

echo [+] Driver built successfully!
echo.

echo [*] Installing driver...
sc create StealthDriver binPath= "%CD%\x64\Release\StealthDriver.sys" type= kernel

if %ERRORLEVEL% NEQ 0 (
    echo [!] Service creation failed
    echo     Driver might already be installed
)

echo.
echo [*] Starting driver...
sc start StealthDriver

echo.
echo Driver status:
sc query StealthDriver

echo.
echo ===============================================
echo   Driver ready! (if no errors above)
echo ===============================================
pause


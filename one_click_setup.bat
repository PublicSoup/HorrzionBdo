@echo off
echo ========================================
echo   BDO Kernel Cheat - One-Click Setup
echo ========================================
echo.

:: Check for admin privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator!
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

echo [+] Administrator privileges confirmed.
echo.

:: Get the directory where this script is located
set SCRIPT_DIR=%~dp0
echo Script directory: %SCRIPT_DIR%

echo [STEP 1/5] Disabling Windows Security Features...
echo ------------------------------------------------
echo Disabling Test Signing...
bcdedit /set testsigning on >nul 2>&1

echo Disabling Vulnerable Driver Blocklist...
reg add "HKLM\SYSTEM\CurrentControlSet\Control\CI\Config" /v "VulnerableDriverBlocklistEnable" /t REG_DWORD /d 0 /f >nul 2>&1

echo Disabling HVCI/Memory Integrity...
reg add "HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\HypervisorEnforcedCodeIntegrity" /v "Enabled" /t REG_DWORD /d 0 /f >nul 2>&1

echo [+] Security features disabled.
echo [!] You may need to restart for some changes to take effect.
echo.

echo [STEP 2/5] Building the Project...
echo ----------------------------------
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to set up Visual Studio environment.
    pause
    exit /b 1
)

echo Building solution...
msbuild "%SCRIPT_DIR%BDOStealthBot.sln" /p:Configuration=Release /p:Platform=x64 /p:TargetName=BDO_OneClick
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo [+] Build completed successfully.
echo.

echo [STEP 3/5] Loading Kernel Driver...
echo -----------------------------------
echo Attempting to load kernel driver via manual mapper...
cd "%SCRIPT_DIR%bin\x64\Release"
BDO_OneClick.exe --map-driver
if %ERRORLEVEL% NEQ 0 (
    echo [!] Driver loading failed. This is normal on first run.
    echo [!] You may need to restart your computer first.
    echo.
    echo Continuing with tests anyway...
)
cd "%SCRIPT_DIR%"
echo.

echo [STEP 4/5] Testing System...
echo ---------------------------
echo Running comprehensive tests...
cd "%SCRIPT_DIR%bin\x64\Release"
BDO_OneClick.exe --test
set TEST_RESULT=%ERRORLEVEL%
cd "%SCRIPT_DIR%"
echo.

echo [STEP 5/5] Final Status...
echo -------------------------
if %TEST_RESULT% EQU 0 (
    echo ========================================
    echo           SUCCESS!
    echo ========================================
    echo.
    echo All tests passed! Your kernel cheat is working.
    echo.
    echo To use with Black Desert Online:
    echo 1. Start Black Desert Online
    echo 2. Log into the game
    echo 3. Run: BDOStealthBot.exe
    echo.
) else (
    echo ========================================
    echo           PARTIAL SUCCESS
    echo ========================================
    echo.
    echo Some tests failed. This is normal if:
    echo - Black Desert Online is not running
    echo - You haven't restarted after disabling security
    echo - The driver needs to be reloaded
    echo.
    echo Try running this script again after:
    echo 1. Restarting your computer
    echo 2. Starting Black Desert Online
    echo.
)

echo ========================================
echo           QUICK COMMANDS
echo ========================================
echo.
echo Load Driver:     BDOStealthBot.exe --map-driver
echo Test System:     BDOStealthBot.exe --test  
echo Start Bot:       BDOStealthBot.exe
echo.
echo ========================================

pause

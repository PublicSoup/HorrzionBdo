@echo off
echo ========================================
echo   BDO Bot - Easy Compilation Script
echo ========================================
echo.

REM Check if Visual Studio is installed
where msbuild >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Visual Studio not found!
    echo Please install Visual Studio 2019 or 2022
    echo Download: https://visualstudio.microsoft.com/downloads/
    pause
    exit /b 1
)

echo [1/4] Checking project files...
if not exist "BDOStealthBot.sln" (
    echo ERROR: BDOStealthBot.sln not found!
    echo Make sure you're in the correct directory
    pause
    exit /b 1
)
echo     ✓ Project files found

echo.
echo [2/4] Cleaning previous build...
msbuild BDOStealthBot.sln /t:Clean /p:Configuration=Release /p:Platform=x64 /nologo /verbosity:quiet
echo     ✓ Clean complete

echo.
echo [3/4] Compiling bot (this may take 1-2 minutes)...
msbuild BDOStealthBot.sln /t:Build /p:Configuration=Release /p:Platform=x64 /nologo /verbosity:minimal

if %errorlevel% neq 0 (
    echo.
    echo ❌ COMPILATION FAILED!
    echo.
    echo Common fixes:
    echo 1. Open BDOStealthBot.sln in Visual Studio
    echo 2. Build ^> Clean Solution
    echo 3. Build ^> Rebuild Solution
    echo 4. Check error messages in Output window
    pause
    exit /b 1
)

echo     ✓ Compilation successful!

echo.
echo [4/4] Locating executable...
if exist "bin\x64\Release\BDOStealthBot.exe" (
    echo     ✓ Found at: bin\x64\Release\BDOStealthBot.exe
    set "EXE_PATH=bin\x64\Release\BDOStealthBot.exe"
) else if exist "x64\Release\BDOStealthBot.exe" (
    echo     ✓ Found at: x64\Release\BDOStealthBot.exe
    set "EXE_PATH=x64\Release\BDOStealthBot.exe"
) else (
    echo     ⚠ Could not locate executable
    echo     Check: bin\x64\Release\ or x64\Release\
    pause
    exit /b 1
)

echo.
echo ========================================
echo   ✓ COMPILATION SUCCESSFUL!
echo ========================================
echo.
echo Executable location: %EXE_PATH%
echo.
echo NEXT STEPS:
echo 1. Start Black Desert Online
echo 2. Run the bot as Administrator
echo 3. Use Cheat Engine to find health address
echo 4. Enter address in bot menu
echo 5. Start automation!
echo.
echo Press any key to run the bot now, or close to exit...
pause >nul

echo.
echo Starting bot as Administrator...
echo (If UAC prompt appears, click Yes)
echo.
powershell -Command "Start-Process '%EXE_PATH%' -Verb RunAs"

echo.
echo Bot launched! Check the new window.
echo.
pause

@echo off
echo ========================================
echo   BDO Kernel Cheat Test Runner
echo ========================================
echo.

echo [1/3] Building the test program...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > nul
msbuild "BDOStealthBot.sln" /p:Configuration=Release /p:Platform=x64 /p:TargetName=BDO_Test
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo [2/3] Checking if kernel driver is loaded...
sc query WinSysService > nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: Kernel driver not loaded!
    echo.
    echo To load the driver, run:
    echo   BDOStealthBot.exe --map-driver
    echo.
    echo Then restart this test.
    echo.
    pause
    exit /b 1
)

echo [+] Kernel driver is loaded.

echo [3/3] Running tests...
echo.
cd bin\x64\Release
BDO_Test.exe
cd ..\..\..

echo.
echo Test completed!
pause

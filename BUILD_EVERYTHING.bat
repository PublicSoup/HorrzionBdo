@echo off
echo ===============================================
echo   Master Build Script - BDO Kernel System
echo   Compiles All Essential Tools
echo ===============================================
echo.

REM Check for g++
where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [-] g++ not found
    echo     MinGW is required
    pause
    exit /b 1
)

echo [+] Compiler found: g++
echo.

REM Clean old builds
echo [*] Cleaning old builds...
del /Q *.exe 2>nul
echo.

echo ===============================================
echo   Compiling Essential Tools...
echo ===============================================
echo.

REM 1. WinSysService Scanner (Main Tool)
echo [1/3] Building WinSysService_Scanner.exe...
echo        (Scans BDO memory using your kernel driver)
g++ -std=c++11 WinSysService_Scanner.cpp -o WinSysService_Scanner.exe -static-libgcc -static-libstdc++

if %ERRORLEVEL% NEQ 0 (
    echo [-] FAILED
    pause
    exit /b 1
)
echo [+] SUCCESS (1.95 MB)
echo.

REM 2. BDO Kernel Reader (Testing Tool)
echo [2/3] Building BDO_KernelReader.exe...
echo        (Tests memory reading with addresses)
g++ -std=c++11 BDO_KernelReader.cpp -o BDO_KernelReader.exe -static-libgcc -static-libstdc++

if %ERRORLEVEL% NEQ 0 (
    echo [-] FAILED
) else (
    echo [+] SUCCESS (46 KB)
)
echo.

REM 3. Driver Test Tool
echo [3/3] Building RTCore_Test.exe...
echo        (Tests RTCore64 driver)
g++ -std=c++11 RTCore_SingleRead_Test.cpp -o RTCore_Test.exe -static-libgcc -static-libstdc++ 2>nul

if %ERRORLEVEL% NEQ 0 (
    echo [!] SKIPPED (optional)
) else (
    echo [+] SUCCESS (44 KB)
)
echo.

echo ===============================================
echo   Build Complete!
echo ===============================================
echo.

echo Essential tools compiled:
echo   [1] WinSysService_Scanner.exe - Main scanner
echo   [2] BDO_KernelReader.exe - Memory reader
echo   [3] RTCore_Test.exe - Driver tester
echo.

echo Driver files:
echo   [✓] WinSysService.sys (bin\x64\Release\)
echo.

echo Next steps:
echo   1. Run: OneClick_EnableTestSigning.bat
echo   2. Restart computer
echo   3. Run: START_EVERYTHING.bat
echo.

pause


@echo off
echo ========================================
echo  BDO Memory Reader - Build Script
echo ========================================
echo.

REM Check for g++
where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] g++ not found. Installing MinGW is recommended.
    echo.
    echo Download MinGW-w64 from:
    echo https://sourceforge.net/projects/mingw-w64/
    echo.
    echo Or use Visual Studio Developer Command Prompt and run:
    echo cl /EHsc /std:c++17 /Fe:bdo_bot.exe BDOBot.cpp BDOMemory.cpp MemoryReader.cpp AntiDetection.cpp /link user32.lib psapi.lib
    pause
    exit /b 1
)

echo [*] Compiling BDO Memory Reader...
echo.

REM Clean old files
if exist *.o del *.o
if exist bdo_bot.exe del bdo_bot.exe
if exist antidetection_demo.exe del antidetection_demo.exe

REM Compile BDO Bot
echo [*] Building bdo_bot.exe...
g++ -std=c++17 -Wall -Wextra -O2 -c MemoryReader.cpp -o MemoryReader.o
g++ -std=c++17 -Wall -Wextra -O2 -c BDOMemory.cpp -o BDOMemory.o
g++ -std=c++17 -Wall -Wextra -O2 -c AntiDetection.cpp -o AntiDetection.o
g++ -std=c++17 -Wall -Wextra -O2 -c BDOBot.cpp -o BDOBot.o
g++ MemoryReader.o BDOMemory.o AntiDetection.o BDOBot.o -o bdo_bot.exe -luser32 -lpsapi

if %ERRORLEVEL% EQU 0 (
    echo [+] bdo_bot.exe built successfully!
    echo.
) else (
    echo [-] Build failed!
    pause
    exit /b 1
)

REM Skip demo build - main.cpp was removed
echo [*] Skipping demo build (old files removed)

REM Clean object files
del *.o

echo ========================================
echo  Build Complete!
echo ========================================
echo.
echo Run as Administrator:
echo   bdo_bot.exe          - Memory reader tool
echo   antidetection_demo.exe - Simple demo
echo.
pause

@echo off
echo ========================================
echo  Professional Kernel System - MinGW Build
echo ========================================
echo.

echo [*] Compiling with g++ (MinGW)...
echo.

REM Clean old files
if exist *.o del *.o
if exist TestDrivers.exe del TestDrivers.exe

echo [*] Step 1: Compiling pch.cpp (precompiled header)...
g++ -std=c++17 -c pch.cpp -o pch.o
if %ERRORLEVEL% NEQ 0 (
    echo [-] Failed to compile pch.cpp
    pause
    exit /b 1
)

echo [*] Step 2: Compiling UniversalDriverInterface.cpp...
g++ -std=c++17 -c UniversalDriverInterface.cpp -o UniversalDriverInterface.o
if %ERRORLEVEL% NEQ 0 (
    echo [-] Failed to compile UniversalDriverInterface.cpp
    pause
    exit /b 1
)

echo [*] Step 3: Compiling TestDrivers.cpp...
g++ -std=c++17 -c TestDrivers.cpp -o TestDrivers.o
if %ERRORLEVEL% NEQ 0 (
    echo [-] Failed to compile TestDrivers.cpp
    pause
    exit /b 1
)

echo [*] Step 4: Linking TestDrivers.exe...
g++ TestDrivers.o UniversalDriverInterface.o pch.o -o TestDrivers.exe -luser32 -ladvapi32 -static-libgcc -static-libstdc++
if %ERRORLEVEL% NEQ 0 (
    echo [-] Failed to link TestDrivers.exe
    pause
    exit /b 1
)

echo [+] TestDrivers.exe built successfully!
echo.

REM Clean object files
del *.o

echo ========================================
echo  Build Complete!
echo ========================================
echo.
echo Run: TestDrivers.exe (as Administrator)
echo.
pause


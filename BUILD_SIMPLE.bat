@echo off
echo ========================================
echo  Simple Driver Test - MinGW Build
echo ========================================
echo.

echo [*] Compiling SimpleDriverTest.cpp...
g++ -std=c++11 SimpleDriverTest.cpp -o SimpleDriverTest.exe -luser32 -ladvapi32 -static-libgcc -static-libstdc++

if %ERRORLEVEL% NEQ 0 (
    echo [-] Failed to compile
    pause
    exit /b 1
)

echo [+] SimpleDriverTest.exe built successfully!
echo.
echo Run: SimpleDriverTest.exe (as Administrator)
echo.
pause


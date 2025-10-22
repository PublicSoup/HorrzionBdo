@echo off
echo ========================================
echo  Professional Kernel System - Build
echo ========================================
echo.

REM Check for Visual Studio
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] Visual Studio not found in PATH
    echo     Please run from Visual Studio Developer Command Prompt
    echo     Or use: "Developer Command Prompt for VS 2022"
    pause
    exit /b 1
)

echo [*] Compiling Professional Kernel System...
echo.

REM Clean old files
if exist *.obj del *.obj
if exist TestDrivers.exe del TestDrivers.exe
if exist BDO_Professional.exe del BDO_Professional.exe

echo [*] Building Universal Driver Interface...
cl /c /std:c++17 /EHsc /I. UniversalDriverInterface.cpp
if %ERRORLEVEL% NEQ 0 (
    echo [-] Failed to compile UniversalDriverInterface.cpp
    pause
    exit /b 1
)

echo [*] Building Driver Test Program...
cl /c /std:c++17 /EHsc /I. TestDrivers.cpp
if %ERRORLEVEL% NEQ 0 (
    echo [-] Failed to compile TestDrivers.cpp
    pause
    exit /b 1
)

REM Link test program
link TestDrivers.obj UniversalDriverInterface.obj /OUT:TestDrivers.exe
if %ERRORLEVEL% NEQ 0 (
    echo [-] Failed to link TestDrivers.exe
    pause
    exit /b 1
)

echo [+] TestDrivers.exe built successfully!
echo.

REM Build main BDO bot if source exists
if exist BDO_StealthBot.cpp (
    echo [*] Building main BDO bot with professional kernel...
    cl /c /std:c++17 /EHsc /I. /D"USE_PROFESSIONAL_KERNEL" BDO_StealthBot.cpp
    cl /c /std:c++17 /EHsc /I. BDO_AdvancedMemory.cpp
    cl /c /std:c++17 /EHsc /I. BDO_MemoryScanner.cpp
    cl /c /std:c++17 /EHsc /I. AntiDetection.cpp
    cl /c /std:c++17 /EHsc /I. pch.cpp /Ycpch.h
    
    link BDO_StealthBot.obj BDO_AdvancedMemory.obj BDO_MemoryScanner.obj ^
         AntiDetection.obj UniversalDriverInterface.obj pch.obj ^
         /OUT:BDO_Professional.exe d3d11.lib dxgi.lib user32.lib kernel32.lib
    
    if %ERRORLEVEL% EQU 0 (
        echo [+] BDO_Professional.exe built successfully!
    )
)

REM Clean object files
del *.obj

echo.
echo ========================================
echo  Build Complete!
echo ========================================
echo.
echo Available programs:
echo   TestDrivers.exe          - Check available drivers
if exist BDO_Professional.exe echo   BDO_Professional.exe    - Main bot with professional kernel
echo.
echo Run TestDrivers.exe first to see what drivers you have!
echo.
pause


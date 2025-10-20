@echo off
echo ========================================
echo   Building BDO Stealth Solution
echo ========================================

echo.
echo [1/3] Ensuring no previous instances are running...
taskkill /f /im BDOStealthBot.exe > nul 2>&1
del "%~dp0bin\x64\Release\BDOStealthBot.exe" > nul 2>&1
echo.

echo [2/3] Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to set up Visual Studio environment.
    pause
    exit /b 1
)

echo [3/3] Building solution with MSBuild...
set RAND_NAME=BDOStealthBot_%RANDOM%
msbuild "%~dp0BDOStealthBot.sln" /p:Configuration=Release /p:Platform=x64 /p:TargetName=%RAND_NAME%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MSBuild failed!
    pause
    exit /b 1
)

echo.
echo [+] Renaming temporary build to final executable...
move /Y "%~dp0bin\x64\Release\%RAND_NAME%.exe" "%~dp0bin\x64\Release\BDOStealthBot.exe" > nul

echo.
echo ========================================
echo       SOLUTION BUILD SUCCEEDED!
echo ========================================
echo.
echo Driver:  %~dp0bin\x64\Release\WinSysService.sys
echo App:     %~dp0bin\x64\Release\BDOStealthBot.exe
echo.
pause

@echo off
echo ===============================================
echo   Testing Advanced Memory Scanner
echo ===============================================
echo.

echo This will test if the scanner works properly
echo.

if not exist AdvancedMemoryScanner.exe (
    echo [-] AdvancedMemoryScanner.exe not found!
    echo     Compile it first with BUILD_WITH_MINGW.bat
    pause
    exit /b 1
)

echo [+] AdvancedMemoryScanner.exe found
echo.

echo Starting notepad as test process...
start notepad.exe
timeout /t 2

echo.
echo Now the scanner will look for notepad...
echo Press 0 to exit when you see the menu
echo.
pause

AdvancedMemoryScanner.exe

echo.
echo Test complete!
pause


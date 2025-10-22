@echo off
REM Comprehensive testing for BDO Secure Kernel
REM Must be run as Administrator

echo ===============================================
echo BDO Secure Kernel - Test Suite
echo ===============================================
echo.

REM Check for admin rights
net session >nul 2>&1
if not %errorLevel% == 0 (
    echo ERROR: This script must be run as Administrator!
    pause
    exit /b 1
)

echo [Phase 1: Driver Status Check]
echo ===============================================
sc query BDOSecureKernel
if errorlevel 1 (
    echo [-] Driver not loaded
    echo [!] Run install_driver.bat first
    pause
    exit /b 1
)
echo [+] Driver is loaded
echo.

echo [Phase 2: Device Access Test]
echo ===============================================
if exist "\\.\\BDOSecureKernel" (
    echo [+] Device accessible
) else (
    echo [-] Device not accessible
)
echo.

echo [Phase 3: User-Mode Test Suite]
echo ===============================================
if not exist "BDO_KernelTest.exe" (
    echo [-] BDO_KernelTest.exe not found
    echo [!] Run compile_all.bat first
    pause
    exit /b 1
)

echo Running BDO_KernelTest...
echo.
BDO_KernelTest.exe
echo.

echo [Phase 4: Driver Verifier Setup]
echo ===============================================
echo.
echo WARNING: Driver Verifier can cause BSOD if driver has bugs
echo This will enable comprehensive driver testing
echo.
echo Do you want to enable Driver Verifier? (Y/N)
set /p ENABLE_VERIFIER=

if /i "%ENABLE_VERIFIER%"=="Y" (
    echo.
    echo Enabling Driver Verifier for BDOSecureKernel.sys...
    verifier /standard /driver BDO_SecureKernel.sys
    
    if errorlevel 1 (
        echo [-] Failed to enable Driver Verifier
    ) else (
        echo [+] Driver Verifier enabled
        echo [!] RESTART REQUIRED for verifier to take effect
        echo [!] Driver will be tested on next boot
        echo.
        echo To check results after restart:
        echo   verifier /query
        echo.
        echo To disable verifier:
        echo   verifier /reset
    )
)
echo.

echo [Phase 5: Memory Leak Check]
echo ===============================================
echo Checking pool allocations...
echo Tag: BDOK
findstr /C:"BDOK" C:\Windows\System32\poolmon.txt 2>nul
if errorlevel 1 (
    echo [!] Pool monitor not enabled
    echo Run: gflags /r +ptg
) else (
    echo [+] Check complete
)
echo.

echo [Phase 6: Event Log Check]
echo ===============================================
echo Checking for kernel errors...
wevtutil qe System /c:10 /rd:true /f:text /q:"*[System[Provider[@Name='BDOSecureKernel']]]" 2>nul
if errorlevel 1 (
    echo [+] No errors in event log
) else (
    echo [!] Found events - check above
)
echo.

echo ===============================================
echo Test Suite Complete
echo ===============================================
echo.
echo Next steps:
echo 1. If Driver Verifier enabled, restart computer
echo 2. After restart, check: verifier /query
echo 3. Run stress tests with: BDO_PracticalBot.exe
echo 4. Monitor system stability for 24 hours
echo 5. Check for memory leaks with PoolMon
echo.
echo To disable verifier after testing:
echo   verifier /reset
echo   restart computer
echo.
pause


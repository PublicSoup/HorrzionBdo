@echo off
REM Complete solution - tests what you have, uses what works

echo ===============================================
echo BDO Complete Working Solution
echo ===============================================
echo.
echo Testing your existing code first...
echo.

REM Check for RTCore64
echo [Test 1] Checking for RTCore64 (MSI Afterburner)...
if exist "C:\Program Files (x86)\MSI Afterburner\RTCore64.sys" (
    echo [+] RTCore64 found - can use existing driver!
    set HAS_RTCORE=1
) else (
    echo [-] RTCore64 not found
    echo [!] Install MSI Afterburner to get it
    set HAS_RTCORE=0
)
echo.

REM Check for gdrv
echo [Test 2] Checking for gdrv.sys (Gigabyte)...
if exist "C:\Windows\System32\drivers\gdrv.sys" (
    echo [+] gdrv.sys found - can use this!
    set HAS_GDRV=1
) else (
    echo [-] gdrv.sys not found
    set HAS_GDRV=0
)
echo.

REM Check if test signing is enabled
echo [Test 3] Checking test signing status...
bcdedit | findstr /C:"testsigning" | findstr /C:"Yes" >nul
if errorlevel 1 (
    echo [-] Test signing NOT enabled
    set HAS_TESTSIGNING=0
) else (
    echo [+] Test signing enabled!
    set HAS_TESTSIGNING=1
)
echo.

echo ===============================================
echo RECOMMENDATION:
echo ===============================================
echo.

if %HAS_RTCORE%==1 (
    echo OPTION 1 [EASIEST]: Use RTCore64 directly
    echo   - Already installed
    echo   - Already signed
    echo   - No test signing needed
    echo   - Run: BDO_AddressScanner_RTCore.exe
    echo.
    echo   Steps:
    echo     1. compile_all.bat
    echo     2. BDO_AddressScanner_RTCore.exe
    echo.
)

if %HAS_TESTSIGNING%==1 (
    echo OPTION 2 [BETTER]: Use custom driver
    echo   - More features
    echo   - Better stealth
    echo   - Test signing already enabled
    echo.
    echo   Steps:
    echo     1. BUILD_EVERYTHING_SECURE.bat
    echo     2. install_driver.bat
    echo     3. BDO_AutoAddressFinder.exe
    echo.
)

if %HAS_TESTSIGNING%==0 if %HAS_RTCORE%==0 (
    echo YOU NEED ONE OF THESE:
    echo.
    echo Option A: Install MSI Afterburner ^(easiest^)
    echo   Download: https://www.msi.com/Landing/afterburner
    echo   Then: compile_all.bat
    echo   Then: BDO_AddressScanner_RTCore.exe
    echo.
    echo Option B: Enable test signing
    echo   Run: enable_test_signing.bat ^(restart required^)
    echo   Then: BUILD_EVERYTHING_SECURE.bat
    echo   Then: install_driver.bat
    echo   Then: BDO_AutoAddressFinder.exe
    echo.
)

echo ===============================================
pause


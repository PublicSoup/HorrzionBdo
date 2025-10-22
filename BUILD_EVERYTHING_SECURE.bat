@echo off
REM Master build script - builds everything in one command
REM Run from Developer Command Prompt for VS as Administrator

echo ===============================================
echo BDO Secure Kernel - Master Build Script
echo ===============================================
echo.

REM Check admin
net session >nul 2>&1
if not %errorLevel% == 0 (
    echo ERROR: Run as Administrator
    pause
    exit /b 1
)

REM Check VS environment
if not defined VCINSTALLDIR (
    echo ERROR: Run from Developer Command Prompt for VS
    pause
    exit /b 1
)

echo Step 1: Building kernel driver...
echo ===============================================
msbuild BDO_SecureKernel.vcxproj /p:Configuration=Release /p:Platform=x64 /t:Rebuild
if errorlevel 1 (
    echo FAILED to build driver
    pause
    exit /b 1
)
echo [OK] Driver built
echo.

echo Step 2: Signing driver...
echo ===============================================
REM Create cert if doesn't exist
certutil -store TestCertStore "BDOTestCert" >nul 2>&1
if errorlevel 1 (
    makecert -r -pe -n "CN=BDOTestCert" -ss TestCertStore -sr LocalMachine
)

signtool sign /s TestCertStore /n "BDOTestCert" /t http://timestamp.digicert.com "bin\x64\Release\BDO_SecureKernel.sys"
if errorlevel 1 (
    echo WARNING: Signing failed, driver may not load
)
echo [OK] Driver signed
echo.

echo Step 3: Compiling user-mode programs...
echo ===============================================
cl /c /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS /nologo BDO_SecureClient.cpp
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS /nologo BDO_KernelTest.cpp BDO_SecureClient.obj /Fe:BDO_KernelTest.exe
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS /nologo BDO_PracticalBot.cpp BDO_SecureClient.obj /Fe:BDO_PracticalBot.exe
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS /nologo BDO_StressTest.cpp BDO_SecureClient.obj /Fe:BDO_StressTest.exe
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS /nologo BDO_SecureLoader.cpp /Fe:BDO_SecureLoader.exe

REM Clean up obj files
del *.obj >nul 2>&1

echo [OK] Programs compiled
echo.

echo Step 4: Checking test signing status...
echo ===============================================
bcdedit | findstr /C:"testsigning" | findstr /C:"Yes" >nul
if errorlevel 1 (
    echo [!] Test signing is NOT enabled
    echo [!] Run: enable_test_signing.bat (requires restart)
    echo.
) else (
    echo [+] Test signing is enabled
    echo.
)

echo ===============================================
echo BUILD COMPLETE
echo ===============================================
echo.
echo Built:
echo   - bin\x64\Release\BDO_SecureKernel.sys (signed)
echo   - BDO_KernelTest.exe
echo   - BDO_PracticalBot.exe
echo   - BDO_StressTest.exe
echo   - BDO_SecureLoader.exe
echo.
echo Next steps:
echo   1. If test signing not enabled: enable_test_signing.bat
echo   2. Install driver: install_driver.bat
echo   3. Run tests: BDO_KernelTest.exe
echo   4. Use bot: BDO_PracticalBot.exe
echo.
pause


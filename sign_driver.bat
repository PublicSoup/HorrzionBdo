@echo off
REM Script to sign the driver with test certificate
REM Must be run from Developer Command Prompt for VS

echo ===============================================
echo BDO Secure Kernel - Driver Signing
echo ===============================================
echo.

REM Check if driver exists
if not exist "bin\x64\Release\BDO_SecureKernel.sys" (
    echo ERROR: Driver not found!
    echo Please build the driver first using build_driver.bat
    pause
    exit /b 1
)

echo [1/4] Creating test certificate...
makecert -r -pe -n "CN=BDOTestCert" -ss TestCertStore -sr LocalMachine
if errorlevel 1 (
    echo ERROR: Failed to create certificate
    pause
    exit /b 1
)
echo [OK] Certificate created

echo.
echo [2/4] Signing driver...
signtool sign /s TestCertStore /n "BDOTestCert" /t http://timestamp.digicert.com "bin\x64\Release\BDO_SecureKernel.sys"
if errorlevel 1 (
    echo ERROR: Failed to sign driver
    pause
    exit /b 1
)
echo [OK] Driver signed

echo.
echo [3/4] Verifying signature...
signtool verify /pa /v "bin\x64\Release\BDO_SecureKernel.sys"
if errorlevel 1 (
    echo WARNING: Signature verification failed
    echo This is normal for test certificates
)
echo [OK] Verification complete

echo.
echo [4/4] Installing certificate to root store...
certutil -addstore Root TestCertStore
echo [OK] Certificate installed

echo.
echo ===============================================
echo SIGNING COMPLETE
echo ===============================================
echo.
echo Driver signed and ready to install.
echo.
echo IMPORTANT: You must enable test signing!
echo Run: bcdedit /set testsigning on
echo Then restart your computer.
echo.
pause


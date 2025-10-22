@echo off
REM Compile all user-mode components

echo ===============================================
echo Compiling BDO Secure Kernel Components
echo ===============================================
echo.

REM Check for Visual Studio
if not defined VCINSTALLDIR (
    echo ERROR: Visual Studio environment not detected
    echo Please run from "Developer Command Prompt for VS"
    pause
    exit /b 1
)

echo [1/4] Compiling BDO_SecureClient...
cl /c /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS BDO_SecureClient.cpp
if errorlevel 1 goto error

echo [2/4] Compiling BDO_KernelTest...
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS BDO_KernelTest.cpp BDO_SecureClient.obj /link /Fe:BDO_KernelTest.exe
if errorlevel 1 goto error

echo [3/4] Compiling BDO_PracticalBot...
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS BDO_PracticalBot.cpp BDO_SecureClient.obj /link /Fe:BDO_PracticalBot.exe
if errorlevel 1 goto error

echo [4/5] Compiling BDO_SecureLoader...
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS BDO_SecureLoader.cpp /link /Fe:BDO_SecureLoader.exe
if errorlevel 1 goto error

echo [5/6] Compiling BDO_StressTest...
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS BDO_StressTest.cpp BDO_SecureClient.obj /link /Fe:BDO_StressTest.exe
if errorlevel 1 goto error

echo [6/7] Compiling EXAMPLE_USAGE...
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS EXAMPLE_USAGE.cpp BDO_SecureClient.obj /link /Fe:EXAMPLE_USAGE.exe
if errorlevel 1 goto error

echo [7/8] Compiling BDO_SimpleIntegration...
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS BDO_SimpleIntegration.cpp BDO_SecureClient.obj /link /Fe:BDO_SimpleIntegration.exe
if errorlevel 1 goto error

echo [8/9] Compiling BDO_AutoAddressFinder...
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS BDO_AutoAddressFinder.cpp BDO_SecureClient.obj /link /Fe:BDO_AutoAddressFinder.exe
if errorlevel 1 goto error

echo [9/9] Compiling BDO_AddressScanner_RTCore...
cl /c /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS /nologo BDO_RTCore64_Driver.cpp 2>nul
cl /c /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS /nologo BDO_RTCore64_Error.cpp 2>nul
cl /EHsc /std:c++17 /W3 /D_CRT_SECURE_NO_WARNINGS /nologo BDO_AddressScanner_RTCore.cpp BDO_RTCore64_Driver.obj BDO_RTCore64_Error.obj /link /Fe:BDO_AddressScanner_RTCore.exe 2>nul
if errorlevel 1 (
    echo [!] RTCore scanner compilation failed - missing dependencies
    echo [!] This is OK if you don't have BDO_RTCore64_Driver.cpp
)

echo.
echo ===============================================
echo COMPILATION SUCCESSFUL
echo ===============================================
echo.
echo Executables created:
echo   - BDO_KernelTest.exe (test custom driver)
echo   - BDO_PracticalBot.exe (working bot)
echo   - BDO_SecureLoader.exe (BYOVD loader)
echo   - BDO_StressTest.exe (stress testing)
echo   - EXAMPLE_USAGE.exe (usage examples)
echo   - BDO_SimpleIntegration.exe (standalone integration)
echo   - BDO_AutoAddressFinder.exe (scanner with custom driver)
echo   - BDO_AddressScanner_RTCore.exe (scanner with RTCore64)
echo.
echo Next: Install driver with install_driver.bat
echo.
goto end

:error
echo.
echo ===============================================
echo COMPILATION FAILED
echo ===============================================
pause
exit /b 1

:end
pause


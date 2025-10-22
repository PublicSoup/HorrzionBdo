@echo off
REM Update git repository with all new files

echo ===============================================
echo Git Repository Update - BDO Secure Kernel
echo ===============================================
echo.

REM Check if git is available
git --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Git is not installed or not in PATH
    echo Download from: https://git-scm.com/downloads
    pause
    exit /b 1
)

echo Current git status:
echo ===============================================
git status
echo.

echo ===============================================
echo This will add ALL new files to git
echo ===============================================
echo.
echo New files include:
echo   - BDO_SecureKernel.* (driver files)
echo   - BDO_SecureClient.* (client library)
echo   - BDO_Stealth.c (stealth features)
echo   - All test programs
echo   - All build scripts
echo   - All documentation
echo.
echo Press Ctrl+C to cancel, or
pause

echo.
echo [1/4] Adding all new files...
git add BDO_SecureKernel.h
git add BDO_SecureKernel.c
git add BDO_SecureKernel.inf
git add BDO_SecureKernel.vcxproj
git add BDO_Stealth.c
git add BDO_SecureClient.h
git add BDO_SecureClient.cpp
git add BDO_KernelTest.cpp
git add BDO_PracticalBot.cpp
git add BDO_StressTest.cpp
git add EXAMPLE_USAGE.cpp
git add BDO_SimpleIntegration.cpp
git add BDO_SecureLoader.cpp
git add BDO_AdvancedMemoryIntegration.cpp
git add *.bat
git add *.md
git add *.txt

echo [OK] Files staged

echo.
echo [2/4] Checking what will be committed...
git status
echo.

echo [3/4] Committing changes...
git commit -m "Complete BDO Secure Kernel Driver Implementation - KMDF driver with Xigncode3 bypass, client library, stealth features, test suite, and full documentation"

if errorlevel 1 (
    echo.
    echo Commit failed or nothing to commit
    pause
    exit /b 1
)

echo [OK] Committed

echo.
echo [4/4] Pushing to remote repository...
git push

if errorlevel 1 (
    echo.
    echo WARNING: Push failed
    echo You may need to set up remote repository first
    echo Or use: git push origin main
    pause
    exit /b 1
)

echo [OK] Pushed to remote

echo.
echo ===============================================
echo GIT UPDATE COMPLETE
echo ===============================================
echo.
echo All files have been committed and pushed
echo.
echo View on GitHub:
git remote get-url origin
echo.
pause


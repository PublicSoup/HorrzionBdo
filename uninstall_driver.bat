@echo off
title BDO Stealth Bot - Driver Uninstallation Script
color 0E

echo ========================================
echo  BDO Stealth Bot - Driver Uninstallation
echo ========================================
echo.

REM Check if running as administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator!
    echo Right-click and select "Run as administrator"
    echo.
    pause
    exit /b 1
)

echo [1/5] Checking if service exists...
sc query WinSysService >nul 2>&1
if %errorLevel% neq 0 (
    echo Service WinSysService not found. Nothing to uninstall.
    echo.
    pause
    exit /b 0
)

echo [2/5] Stopping service...
sc stop WinSysService
if %errorLevel% neq 0 (
    echo WARNING: Failed to stop service gracefully.
    echo Attempting to force stop...
    sc stop WinSysService /force
)

echo [3/5] Waiting for service to stop...
timeout /t 3 /nobreak >nul

echo [4/5] Deleting service...
sc delete WinSysService
if %errorLevel% neq 0 (
    echo ERROR: Failed to delete service!
    echo.
    echo Possible solutions:
    echo 1. Ensure you are running as Administrator
    echo 2. Check if the service is still running
    echo 3. Try restarting your computer
    echo.
    pause
    exit /b 1
)
echo Service deleted successfully.

echo [5/5] Uninstallation completed!
echo.
echo ========================================
echo      DRIVER UNINSTALLATION SUMMARY
echo ========================================
echo.
echo Service Name: WinSysService
echo Status: Removed
echo.
echo ========================================
echo.
echo Optional: Disable test signing
echo Run: bcdedit /set testsigning off
echo Then restart your computer
echo.
pause

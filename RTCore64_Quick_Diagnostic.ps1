# RTCore64 Quick Diagnostic Script
# This script checks the RTCore64 driver status without requiring compilation

Write-Host "=== RTCore64 Driver Quick Diagnostic ===" -ForegroundColor Cyan
Write-Host ""

# 1. Check if running as Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
Write-Host "1. Administrator Check:" -ForegroundColor Yellow
if ($isAdmin) {
    Write-Host "   [PASS] Running as Administrator" -ForegroundColor Green
} else {
    Write-Host "   [WARN] NOT running as Administrator - some operations may fail" -ForegroundColor Red
    Write-Host "   Please run PowerShell as Administrator for full diagnostics" -ForegroundColor Red
}
Write-Host ""

# 2. Check if RTCore64.sys file exists
Write-Host "2. Driver File Check:" -ForegroundColor Yellow
$driverPath = "C:\Program Files (x86)\MSI Afterburner\RTCore64.sys"
if (Test-Path $driverPath) {
    $fileInfo = Get-Item $driverPath
    Write-Host "   [PASS] RTCore64.sys found" -ForegroundColor Green
    Write-Host "   Path: $driverPath" -ForegroundColor Gray
    Write-Host "   Size: $($fileInfo.Length) bytes" -ForegroundColor Gray
    Write-Host "   Modified: $($fileInfo.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Host "   [FAIL] RTCore64.sys NOT found at expected location" -ForegroundColor Red
    Write-Host "   Please install MSI Afterburner" -ForegroundColor Red
}
Write-Host ""

# 3. Check RTCore64 service status
Write-Host "3. Service Status Check:" -ForegroundColor Yellow
try {
    $service = Get-Service -Name "RTCore64" -ErrorAction Stop
    Write-Host "   [PASS] RTCore64 service found" -ForegroundColor Green
    Write-Host "   Status: $($service.Status)" -ForegroundColor Gray
    Write-Host "   Start Type: $($service.StartType)" -ForegroundColor Gray
    
    if ($service.Status -eq "Running") {
        Write-Host "   [PASS] Service is RUNNING" -ForegroundColor Green
    } else {
        Write-Host "   [WARN] Service is NOT running (Status: $($service.Status))" -ForegroundColor Red
        
        if ($isAdmin) {
            Write-Host "   Attempting to start service..." -ForegroundColor Yellow
            try {
                Start-Service -Name "RTCore64" -ErrorAction Stop
                Write-Host "   [PASS] Service started successfully" -ForegroundColor Green
            } catch {
                Write-Host "   [FAIL] Failed to start service: $($_.Exception.Message)" -ForegroundColor Red
            }
        } else {
            Write-Host "   Run as Administrator to attempt starting the service" -ForegroundColor Yellow
        }
    }
} catch {
    Write-Host "   [FAIL] RTCore64 service NOT found" -ForegroundColor Red
    Write-Host "   Error: $($_.Exception.Message)" -ForegroundColor Red
    
    if ($isAdmin -and (Test-Path $driverPath)) {
        Write-Host "   Attempting to create service..." -ForegroundColor Yellow
        try {
            $result = sc.exe create RTCore64 type= kernel binPath= "`"$driverPath`"" DisplayName= "RTCore64"
            if ($LASTEXITCODE -eq 0) {
                Write-Host "   [PASS] Service created successfully" -ForegroundColor Green
                Write-Host "   Attempting to start service..." -ForegroundColor Yellow
                $result = sc.exe start RTCore64
                if ($LASTEXITCODE -eq 0) {
                    Write-Host "   [PASS] Service started successfully" -ForegroundColor Green
                } else {
                    Write-Host "   [FAIL] Failed to start service" -ForegroundColor Red
                }
            } else {
                Write-Host "   [FAIL] Failed to create service" -ForegroundColor Red
            }
        } catch {
            Write-Host "   [FAIL] Error creating service: $($_.Exception.Message)" -ForegroundColor Red
        }
    } else {
        Write-Host "   Run as Administrator to create the service" -ForegroundColor Yellow
    }
}
Write-Host ""

# 4. Test driver accessibility
Write-Host "4. Driver Accessibility Check:" -ForegroundColor Yellow
try {
    # Try to open the driver device
    Add-Type -TypeDefinition @"
    using System;
    using System.Runtime.InteropServices;
    using Microsoft.Win32.SafeHandles;
    
    public class RTCoreDriver {
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern SafeFileHandle CreateFile(
            string lpFileName,
            uint dwDesiredAccess,
            uint dwShareMode,
            IntPtr lpSecurityAttributes,
            uint dwCreationDisposition,
            uint dwFlagsAndAttributes,
            IntPtr hTemplateFile);
            
        public const uint GENERIC_READ = 0x80000000;
        public const uint GENERIC_WRITE = 0x40000000;
        public const uint OPEN_EXISTING = 3;
    }
"@
    
    $handle = [RTCoreDriver]::CreateFile("\\.\\RTCore64", 
        [RTCoreDriver]::GENERIC_READ -bor [RTCoreDriver]::GENERIC_WRITE,
        0, [IntPtr]::Zero, [RTCoreDriver]::OPEN_EXISTING, 0, [IntPtr]::Zero)
    
    if (!$handle.IsInvalid) {
        Write-Host "   [PASS] Successfully opened RTCore64 driver device" -ForegroundColor Green
        $handle.Close()
    } else {
        $error = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
        Write-Host "   [FAIL] Failed to open driver device (Error: $error)" -ForegroundColor Red
        if ($error -eq 5) {
            Write-Host "   Error 5 = Access Denied. Run as Administrator." -ForegroundColor Red
        } elseif ($error -eq 2) {
            Write-Host "   Error 2 = File Not Found. Service may not be running." -ForegroundColor Red
        }
    }
} catch {
    Write-Host "   [FAIL] Error testing driver: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Summary
Write-Host "=== Diagnostic Summary ===" -ForegroundColor Cyan
if ((Test-Path $driverPath) -and ($service.Status -eq "Running")) {
    Write-Host "RTCore64 driver appears to be properly installed and running." -ForegroundColor Green
    Write-Host "If you're still having issues, the problem may be with:" -ForegroundColor Yellow
    Write-Host "  - Memory address access permissions" -ForegroundColor Yellow
    Write-Host "  - Application-specific code issues" -ForegroundColor Yellow
    Write-Host "  - Anti-virus blocking operations" -ForegroundColor Yellow
} else {
    Write-Host "RTCore64 driver is NOT properly configured." -ForegroundColor Red
    Write-Host "Please address the issues identified above." -ForegroundColor Red
}
Write-Host ""
Write-Host "Press any key to exit..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")


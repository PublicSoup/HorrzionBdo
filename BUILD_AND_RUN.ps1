# BDO Bot - Complete Build and Run Script
# PowerShell script for reliable compilation and execution

param(
    [switch]$SkipBDOCheck,
    [switch]$CompileOnly,
    [switch]$RunOnly
)

$ErrorActionPreference = "Stop"

# Change to script directory
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptPath

Write-Host ""
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host "     BDO BOT - BUILD AND RUN SCRIPT" -ForegroundColor Cyan
Write-Host "============================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Working directory: $scriptPath" -ForegroundColor Gray
Write-Host ""

# Check if running as Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "[WARNING] Not running as Administrator!" -ForegroundColor Yellow
    Write-Host "The bot may not work properly without admin rights." -ForegroundColor Yellow
    Write-Host ""
    $continue = Read-Host "Continue anyway? (Y/N)"
    if ($continue -ne "Y" -and $continue -ne "y") {
        exit 1
    }
}

# Function to find MSBuild
function Find-MSBuild {
    Write-Host "[1/4] Searching for MSBuild..." -ForegroundColor Green
    
    # VS 2022 locations
    $paths = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild.exe"
    )
    
    foreach ($path in $paths) {
        if (Test-Path $path) {
            Write-Host "  [OK] Found: $path" -ForegroundColor Green
            return $path
        }
    }
    
    # Try PATH
    $msbuild = Get-Command msbuild -ErrorAction SilentlyContinue
    if ($msbuild) {
        Write-Host "  [OK] Found in PATH: $($msbuild.Source)" -ForegroundColor Green
        return $msbuild.Source
    }
    
    Write-Host "  [ERROR] MSBuild not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install Visual Studio 2019 or 2022:" -ForegroundColor Yellow
    Write-Host "  https://visualstudio.microsoft.com/downloads/" -ForegroundColor Yellow
    Write-Host ""
    return $null
}

# Function to compile
function Build-Project {
    param([string]$msbuildPath)
    
    Write-Host ""
    Write-Host "[2/4] Compiling project..." -ForegroundColor Green
    
    if (-not (Test-Path "BDOStealthBot.sln")) {
        Write-Host "  [ERROR] BDOStealthBot.sln not found!" -ForegroundColor Red
        Write-Host "  Make sure you're in the project directory." -ForegroundColor Yellow
        return $false
    }
    
    # Clean
    Write-Host "  Cleaning previous build..." -ForegroundColor Gray
    & $msbuildPath "BDOStealthBot.sln" /t:Clean /p:Configuration=Release /p:Platform=x64 /nologo /verbosity:quiet
    
    # Build
    Write-Host "  Building (this may take 1-2 minutes)..." -ForegroundColor Gray
    & $msbuildPath "BDOStealthBot.sln" /t:Build /p:Configuration=Release /p:Platform=x64 /nologo /verbosity:minimal
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "  [ERROR] Compilation failed!" -ForegroundColor Red
        Write-Host ""
        Write-Host "Try these solutions:" -ForegroundColor Yellow
        Write-Host "  1. Open BDOStealthBot.sln in Visual Studio" -ForegroundColor Yellow
        Write-Host "  2. Build -> Clean Solution" -ForegroundColor Yellow
        Write-Host "  3. Build -> Rebuild Solution" -ForegroundColor Yellow
        Write-Host ""
        return $false
    }
    
    Write-Host "  [OK] Compilation successful!" -ForegroundColor Green
    return $true
}

# Function to check BDO
function Test-BDORunning {
    Write-Host ""
    Write-Host "[3/4] Checking if BDO is running..." -ForegroundColor Green
    
    $bdo = Get-Process -Name "BlackDesert64" -ErrorAction SilentlyContinue
    if ($bdo) {
        Write-Host "  [OK] Black Desert Online detected (PID: $($bdo.Id))" -ForegroundColor Green
        return $true
    } else {
        Write-Host "  [WARNING] Black Desert Online is NOT running!" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Please start BDO first:" -ForegroundColor Yellow
        Write-Host "  1. Launch Black Desert Online" -ForegroundColor Yellow
        Write-Host "  2. Log into your character" -ForegroundColor Yellow
        Write-Host "  3. Enter the game world" -ForegroundColor Yellow
        Write-Host ""
        return $false
    }
}

# Function to run bot
function Start-Bot {
    Write-Host ""
    Write-Host "[4/4] Starting bot..." -ForegroundColor Green
    
    $exePath = "bin\x64\Release\BDOStealthBot.exe"
    
    if (-not (Test-Path $exePath)) {
        Write-Host "  [ERROR] Bot executable not found!" -ForegroundColor Red
        Write-Host "  Expected: $exePath" -ForegroundColor Yellow
        return $false
    }
    
    Write-Host "  [OK] Launching: $exePath" -ForegroundColor Green
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Cyan
    Write-Host "     BOT STARTING - CONSOLE WILL OPEN" -ForegroundColor Cyan
    Write-Host "============================================================" -ForegroundColor Cyan
    Write-Host ""
    
    # Change to bot directory and run
    Push-Location "bin\x64\Release"
    Start-Process -FilePath "BDOStealthBot.exe" -Wait
    Pop-Location
    
    return $true
}

# Main execution
try {
    # Step 1: Find MSBuild (skip if RunOnly)
    if (-not $RunOnly) {
        $msbuild = Find-MSBuild
        if (-not $msbuild) {
            exit 1
        }
        
        # Step 2: Compile
        $compiled = Build-Project -msbuildPath $msbuild
        if (-not $compiled) {
            exit 1
        }
    }
    
    # Exit if CompileOnly
    if ($CompileOnly) {
        Write-Host ""
        Write-Host "Compilation complete! Use -RunOnly to run the bot." -ForegroundColor Green
        exit 0
    }
    
    # Step 3: Check BDO (skip if SkipBDOCheck)
    if (-not $SkipBDOCheck) {
        $bdoRunning = Test-BDORunning
        if (-not $bdoRunning) {
            Write-Host "Continue without BDO running? (Y/N): " -NoNewline -ForegroundColor Yellow
            $continue = Read-Host
            if ($continue -ne "Y" -and $continue -ne "y") {
                Write-Host ""
                Write-Host "Start BDO and run this script again." -ForegroundColor Yellow
                exit 1
            }
        }
    }
    
    # Step 4: Run bot
    $started = Start-Bot
    if (-not $started) {
        exit 1
    }
    
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host "     BOT SESSION COMPLETE" -ForegroundColor Green
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host ""
    
} catch {
    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Red
    Write-Host "     ERROR OCCURRED" -ForegroundColor Red
    Write-Host "============================================================" -ForegroundColor Red
    Write-Host ""
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    exit 1
}

Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")


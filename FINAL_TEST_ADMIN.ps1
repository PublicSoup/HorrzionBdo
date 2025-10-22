# Check if running as admin
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (!$isAdmin) {
    Write-Host "Not running as Administrator. Restarting with elevation..." -ForegroundColor Yellow
    Start-Process powershell -ArgumentList "-ExecutionPolicy Bypass -File `"$PSCommandPath`"" -Verb RunAs
    exit
}

Write-Host "Running as Administrator!" -ForegroundColor Green
Write-Host ""

# Navigate to bot directory
cd "$PSScriptRoot\bin\x64\Debug"

Write-Host "Launching BDO Stealth Bot..." -ForegroundColor Cyan
Write-Host "Watch for diagnostic messages..." -ForegroundColor Cyan
Write-Host ""

# Run the bot
.\BDOStealthBot.exe

Write-Host ""
Write-Host "Press any key to exit..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")


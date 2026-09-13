# Meridian Terminal Windows Offline Installer (PowerShell)
param (
    [string]$InstallDir = "$env:LOCALAPPDATA\Programs\MeridianTerminal"
)

Write-Host "==============================================" -ForegroundColor Cyan
Write-Host "    Meridian Terminal Windows Installer       " -ForegroundColor Cyan
Write-Host "==============================================" -ForegroundColor Cyan

New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
New-Item -ItemType Directory -Force -Path "$InstallDir\bin" | Out-Null

Copy-Item -Path ".\build\meridian-shell.exe" -Destination "$InstallDir\bin\meridian-shell.exe" -ErrorAction SilentlyContinue
Copy-Item -Path ".\build\meridian.exe" -Destination "$InstallDir\bin\meridian.exe" -ErrorAction SilentlyContinue
Copy-Item -Path ".\meridian.desktop" -Destination "$InstallDir\meridian.desktop" -ErrorAction SilentlyContinue

# Add to User PATH if not present
$UserPath = [Environment]::GetEnvironmentVariable("PATH", "User")
if ($UserPath -notlike "*$InstallDir\bin*") {
    [Environment]::SetEnvironmentVariable("PATH", "$UserPath;$InstallDir\bin", "User")
    Write-Host "-> Added $InstallDir\bin to User PATH." -ForegroundColor Green
}

Write-Host "`nMeridian Terminal successfully installed on Windows!" -ForegroundColor Green
Write-Host "You can now run 'meridian-shell' or 'meridian' in PowerShell/CMD/Terminal." -ForegroundColor Yellow


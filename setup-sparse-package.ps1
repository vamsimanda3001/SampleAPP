# setup-sparse-package.ps1
# One-time setup: builds the sparse MSIX, installs the dev cert, and registers the package.
# Run this from an ELEVATED (Admin) PowerShell prompt.
#
# Usage:  .\setup-sparse-package.ps1
#         .\setup-sparse-package.ps1 -Uninstall   (to remove everything)

param(
    [switch]$Uninstall
)

$ErrorActionPreference = "Stop"
$scriptDir  = Split-Path -Parent $MyInvocation.MyCommand.Path
$sparseDir  = Join-Path $scriptDir "sparse"
$msixPath   = Join-Path $sparseDir "RemoteDesktopProviderSample.msix"
$pfxPath    = Join-Path $sparseDir "DevCert.pfx"
$cerPath    = Join-Path $sparseDir "DevCert.cer"
$manifestDir = $sparseDir
$exeDir     = Join-Path $scriptDir "out\build\default\Debug"

$sdkBin     = "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64"
$makeAppx   = Join-Path $sdkBin "makeappx.exe"
$signTool   = Join-Path $sdkBin "signtool.exe"

$packageName = "RemoteDesktopProviderSample"
# Read password from environment variable, or prompt the user
$pfxPassword = $env:PFX_PASSWORD
if (-not $pfxPassword) {
    $pfxPassword = Read-Host -Prompt "Enter PFX password"
}

# ── Uninstall ────────────────────────────────────────────────────────────────
if ($Uninstall) {
    Write-Host "`n=== Removing sparse package ===" -ForegroundColor Yellow
    $pkg = Get-AppxPackage -Name $packageName -ErrorAction SilentlyContinue
    if ($pkg) {
        Remove-AppxPackage $pkg.PackageFullName
        Write-Host "  Removed package: $($pkg.PackageFullName)"
    } else {
        Write-Host "  Package not installed."
    }
    Write-Host "Done." -ForegroundColor Green
    exit 0
}

# ── Pre-flight checks ───────────────────────────────────────────────────────
if (-not (Test-Path $makeAppx)) {
    Write-Error "MakeAppx.exe not found at: $makeAppx"
}
if (-not (Test-Path $signTool)) {
    Write-Error "SignTool.exe not found at: $signTool"
}

# ── Step 1: Build the sparse MSIX ───────────────────────────────────────────
Write-Host "`n=== Step 1: Building sparse MSIX ===" -ForegroundColor Cyan
if (Test-Path $msixPath) { Remove-Item $msixPath -Force }

# MakeAppx pack from the sparse directory (manifest + Assets)
& $makeAppx pack /d $manifestDir /p $msixPath /nv
if ($LASTEXITCODE -ne 0) { throw "MakeAppx failed" }
Write-Host "  Created: $msixPath"

# ── Step 2: Sign the MSIX ───────────────────────────────────────────────────
Write-Host "`n=== Step 2: Signing MSIX ===" -ForegroundColor Cyan
& $signTool sign /fd SHA256 /a /f $pfxPath /p $pfxPassword $msixPath
if ($LASTEXITCODE -ne 0) { throw "SignTool failed" }
Write-Host "  Signed successfully."

# ── Step 3: Install dev cert in TrustedPeople ────────────────────────────────
Write-Host "`n=== Step 3: Installing dev certificate ===" -ForegroundColor Cyan
$existingCert = Get-ChildItem Cert:\CurrentUser\TrustedPeople |
    Where-Object { $_.Subject -eq "CN=RemoteDesktopProviderSample" }
if ($existingCert) {
    Write-Host "  Certificate already installed."
} else {
    Import-Certificate -FilePath $cerPath -CertStoreLocation Cert:\CurrentUser\TrustedPeople | Out-Null
    Write-Host "  Installed cert to CurrentUser\TrustedPeople"
}

# ── Step 4: Register the sparse package ──────────────────────────────────────
Write-Host "`n=== Step 4: Registering sparse package ===" -ForegroundColor Cyan

# Remove existing package if present
$existing = Get-AppxPackage -Name $packageName -ErrorAction SilentlyContinue
if ($existing) {
    Write-Host "  Removing existing package..."
    Remove-AppxPackage $existing.PackageFullName
}

# Register with ExternalLocation pointing to our exe directory
# The manifest must be IN the exe directory for identity to work.
# CMakeLists.txt copies it there during build.
$manifestInExeDir = Join-Path $exeDir "AppxManifest.xml"
if (-not (Test-Path $manifestInExeDir)) {
    Write-Error "AppxManifest.xml not found in exe directory. Build the project first."
}
Add-AppxPackage -Register $manifestInExeDir -ExternalLocation $exeDir
Write-Host "  Registered sparse package from: $exeDir"

# ── Verify ──────────────────────────────────────────────────────────────────
Write-Host "`n=== Verification ===" -ForegroundColor Green
$pkg = Get-AppxPackage -Name $packageName
if ($pkg) {
    Write-Host "  Package Full Name : $($pkg.PackageFullName)"
    Write-Host "  PFN               : $($pkg.PackageFamilyName)"
    Write-Host "  Install Location  : $($pkg.InstallLocation)"
    Write-Host "  Status            : $($pkg.Status)"
    Write-Host "`n  SUCCESS! The app now has package identity." -ForegroundColor Green
    Write-Host "  Run the exe from: $exeDir\RemoteDesktopProviderSample.exe"
} else {
    Write-Error "Package registration failed - package not found."
}

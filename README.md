# RemoteDesktopProviderSample

A C++/WinRT Win32 desktop sample app that exercises the [`Windows.System.RemoteDesktop.Provider`](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider?view=winrt-28000) namespace.

This app demonstrates how a third-party provider can register Cloud PCs with the Windows shell (Task View / Switch), read registrar state, and interact with the Remote Desktop Provider APIs.

## What It Does (Phase 1)

- **RemoteDesktopInfo** — Constructs a Cloud PC entry with an ID and display name, then appends it to the registrar so the Windows shell can discover it.
- **RemoteDesktopRegistrar** — Reads `IsSwitchToLocalSessionEnabled()` and enumerates registered `DesktopInfos`.
- **Sparse Package Identity** — Uses a sparse MSIX package so the unpackaged Win32 exe gets package identity at runtime (required by the APIs).
- **Limited Access Feature (LAF)** — Unlocks the gated API using a token tied to the app's Package Family Name.

## Prerequisites

| Requirement | Details |
|---|---|
| OS | Windows 11 (Build 26100+) with **Developer Mode** enabled |
| IDE | Visual Studio 2022 / 18 2026 Enterprise |
| Windows SDK | 10.0.26100.0 or later (provides `cppwinrt.exe`, `MakeAppx.exe`, `SignTool.exe`) |
| CMake | Bundled with Visual Studio or standalone 3.20+ |
| LAF Token | Request from `lafaccessrequests@microsoft.com` (see [LAF Setup](#laf-token-setup)) |

## Build

```powershell
# Configure (one-time)
cmake --preset default

# Build
cmake --build --preset default
```

The exe is produced at `out\build\default\Debug\RemoteDesktopProviderSample.exe`.

## Setup (One-Time)

The app needs **package identity** to call the Remote Desktop Provider APIs. A sparse MSIX package provides this without sandboxing.

### 1. Create a Self-Signed Certificate

```powershell
# Generate a self-signed cert (CN must match AppxManifest.xml Publisher)
$cert = New-SelfSignedCertificate -Type Custom `
    -Subject "CN=RemoteDesktopProviderSample" `
    -KeyUsage DigitalSignature `
    -FriendlyName "RDP Provider Sample Dev Cert" `
    -CertStoreLocation "Cert:\CurrentUser\My" `
    -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3")

# Export PFX (pick a password)
$pw = ConvertTo-SecureString -String "YourPassword" -Force -AsPlainText
Export-PfxCertificate -Cert $cert -FilePath sparse\DevCert.pfx -Password $pw

# Export public cert
Export-Certificate -Cert $cert -FilePath sparse\DevCert.cer
```

### 2. Register the Sparse Package

```powershell
# Set env var with your PFX password, then run setup
$env:PFX_PASSWORD = "YourPassword"
.\setup-sparse-package.ps1
```

This builds the MSIX, signs it, installs the cert, and registers the package.

### 3. LAF Token Setup

The Remote Desktop Provider APIs are gated behind a **Limited Access Feature**. You need a token tied to your Package Family Name (PFN).

1. Find your PFN:
   ```powershell
   Get-AppxPackage *RemoteDesktopProviderSample* | Select-Object PackageFamilyName
   ```
2. Email `lafaccessrequests@microsoft.com` requesting access to the **Remote Desktop Provider API**, providing your full PFN (e.g. `RemoteDesktopProviderSample_955ksfw34s3d4`).
3. Set the token as an environment variable before running:
   ```powershell
   $env:LAF_TOKEN = "your-token-here"
   ```

## Run

```powershell
.\out\build\default\Debug\RemoteDesktopProviderSample.exe
```

Expected output:
```
RemoteDesktopProviderSample starting...
=== Package Identity Check ===
  [OK] Package Full Name: RemoteDesktopProviderSample_1.0.0.0_x64__955ksfw34s3d4
=== Unlocking Limited Access Feature ===
  LAF Status: Available
=== RemoteDesktopInfo Demo ===
  Created RemoteDesktopInfo: Id=sample-cloud-pc-id-2, DisplayName=Sample Cloud PC-2
  Appended to DesktopInfos
=== RemoteDesktopRegistrar Demo ===
  IsSwitchToLocalSessionEnabled = true
  DesktopInfos count = 1
```

After running, verify the Cloud PC appears in the registry:
```
HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\RemoteSystemProviders\<PFN>\<id>
```

## How Sparse Package Identity Works

A Win32 desktop app normally runs without package identity. The sparse MSIX pattern gives it identity without sandboxing:

1. **AppxManifest.xml** declares the package with `uap10:AllowExternalContent="true"` and `uap10:RuntimeBehavior="win32App"`.
2. **Embedded fusion manifest** (`src/RemoteDesktopProviderSample.exe.manifest`) contains an `<msix>` element that tells the Windows loader to activate package identity when starting the exe.
3. **Registration** via `Add-AppxPackage -Register ... -ExternalLocation` links the manifest to the exe's directory.

The `<msix>` element in the fusion manifest is **the critical piece** — without it, the process starts without identity even if the package is registered.

## Project Structure

```
├── CMakeLists.txt                  # Build config (C++20, cppwinrt, /MANIFEST:NO)
├── CMakePresets.json               # VS 18 2026 x64 Debug/Release presets
├── setup-sparse-package.ps1        # One-time: build MSIX, sign, register
├── .gitignore
├── README.md
├── src/
│   ├── main.cpp                    # App entry point + API demos
│   ├── app.rc                      # LAF identity resource + RT_MANIFEST
│   └── RemoteDesktopProviderSample.exe.manifest  # Fusion manifest with <msix>
└── sparse/
    ├── AppxManifest.xml            # Sparse package manifest
    └── Assets/
        └── StoreLogo.png           # Placeholder icon
```

## API Coverage

| API | Status |
|---|---|
| `RemoteDesktopInfo` (ctor, Id, DisplayName) | ✅ Implemented |
| `RemoteDesktopRegistrar.DesktopInfos` | ✅ Implemented |
| `RemoteDesktopRegistrar.IsSwitchToLocalSessionEnabled` | ✅ Implemented |
| `RemoteDesktopRegistrar.ConnectionCenterRequested` | 🔜 Phase 4 |
| `RemoteDesktopConnectionInfo` | 🔜 Phase 2 |
| `RemoteDesktopConnectionRemoteInfo` | 🔜 Phase 3 |

## License

Internal sample — not for public distribution.

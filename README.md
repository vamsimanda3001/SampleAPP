# RemoteDesktopProviderSample

A C++/WinRT Win32 desktop sample app that exercises the [`Windows.System.RemoteDesktop.Provider`](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider?view=winrt-28000) namespace.

## Overview

This sample demonstrates how a third-party remote desktop provider (e.g., a Cloud PC client) integrates with the Windows shell to enable features like **Task View switching** and **Cloud PC discovery**. The app uses the `Windows.System.RemoteDesktop.Provider` namespace — a set of Limited Access Feature (LAF) APIs that allow providers to register remote desktops, manage connection state, and respond to user-initiated actions from the local Windows session.

The APIs require **package identity** (achieved via a sparse MSIX package) and a **LAF unlock token** obtained from Microsoft.

## API Reference

All APIs live under `Windows.System.RemoteDesktop.Provider`. They are gated behind a [Limited Access Feature](https://learn.microsoft.com/en-us/uwp/api/windows.applicationmodel.limitedaccessfeatures) — you must unlock them with a token before use.

### Classes

| Class | Description | Docs |
|---|---|---|
| [`RemoteDesktopInfo`](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopinfo?view=winrt-28000) | Represents a single remote desktop (Cloud PC). Constructed with an `id` and `displayName`. The provider creates one of these for each Cloud PC it manages and appends it to the registrar so the Windows shell can discover it. | [API Docs](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopinfo?view=winrt-28000) |
| [`RemoteDesktopRegistrar`](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar?view=winrt-28000) | Static class that manages the collection of registered remote desktops. Exposes `DesktopInfos` (an `IVector<RemoteDesktopInfo>` of registered Cloud PCs) and `IsSwitchToLocalSessionEnabled()` (whether the user can switch back to the local desktop). | [API Docs](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar?view=winrt-28000) |
| [`RemoteDesktopConnectionInfo`](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopconnectioninfo?view=winrt-28000) | Represents a remote desktop connection on the **local** side. Used to report connection status changes, trigger a switch to the local session, or perform local actions (e.g., open Bluetooth settings) from the remote session. | [API Docs](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopconnectioninfo?view=winrt-28000) |
| [`RemoteDesktopConnectionRemoteInfo`](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopconnectionremoteinfo?view=winrt-28000) | Represents a remote desktop connection on the **remote** side. Fires events when the user requests a switch to the local session or a local action. The provider subscribes to these events and acts accordingly. Implements `IClosable`. | [API Docs](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopconnectionremoteinfo?view=winrt-28000) |
| [`PerformLocalActionRequestedEventArgs`](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.performlocalactionrequestedeventargs?view=winrt-28000) | Event args for `RemoteDesktopConnectionRemoteInfo.PerformLocalActionRequested`. Contains the `Action` property indicating which local action the user requested. | [API Docs](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.performlocalactionrequestedeventargs?view=winrt-28000) |

### Enums

| Enum | Values | Description |
|---|---|---|
| [`RemoteDesktopConnectionStatus`](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopconnectionstatus?view=winrt-28000) | `Connecting`, `Connected`, `UserInputNeeded`, `Disconnected` | Connection lifecycle states reported via `RemoteDesktopConnectionInfo.SetConnectionStatus()`. |
| [`RemoteDesktopLocalAction`](https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktoplocalaction?view=winrt-28000) | `ShowBluetoothSettings`, `ShowSystemSoundSettings`, `ShowSystemDisplaySettings`, `ShowSystemAccountSettings`, `ShowLocalSettings` | Local actions that can be triggered from a remote session via `PerformLocalActionFromRemote()`. |

### How the APIs Work Together

1. **Registration**: The provider creates `RemoteDesktopInfo` objects and appends them to `RemoteDesktopRegistrar.DesktopInfos()`. This writes entries to the registry at `HKCU\...\RemoteSystemProviders\<PFN>\<id>` so the Windows shell (Task View) can discover them.

2. **Connection (Local Side)**: When a user launches a Cloud PC, the provider obtains a `RemoteDesktopConnectionInfo` via `GetForLaunchUri()` and reports status changes (`Connecting` → `Connected` → `Disconnected`).

3. **Connection (Remote Side)**: On the remote session, the provider uses `RemoteDesktopConnectionRemoteInfo` to listen for `SwitchToLocalSessionRequested` and `PerformLocalActionRequested` events, then acts on them.

## What This Sample Does

### Currently Implemented (Phase 1)

- **`RemoteDesktopInfo`** — Constructs a Cloud PC entry with an ID and display name, reads back the `Id` and `DisplayName` properties, then appends it to `RemoteDesktopRegistrar.DesktopInfos()` to register it with the Windows shell.
- **`RemoteDesktopRegistrar`** — Reads the `IsSwitchToLocalSessionEnabled` property and enumerates all registered `DesktopInfos` entries.
- **Sparse Package Identity** — Uses a sparse MSIX package so the unpackaged Win32 exe gets package identity at runtime (required by the APIs).
- **Limited Access Feature (LAF)** — Unlocks the gated APIs using a token tied to the app's Package Family Name.

### Planned

| Phase | API | What It Will Do |
|---|---|---|
| Phase 2 | `RemoteDesktopConnectionInfo` | `GetForLaunchUri`, `SetConnectionStatus`, `SwitchToLocalSession`, `PerformLocalActionFromRemote` |
| Phase 3 | `RemoteDesktopConnectionRemoteInfo` | `IsSwitchSupported`, `ReportSwitched`, subscribe to `SwitchToLocalSessionRequested` and `PerformLocalActionRequested` events |
| Phase 4 | `RemoteDesktopRegistrar` | Subscribe to `ConnectionCenterRequested` event |

## Prerequisites

| Requirement | Details |
|---|---|
| OS | Windows 11 (Build 26100+) with **Developer Mode** enabled |
| IDE | Visual Studio 2022 / 18 2026 Enterprise |
| Windows SDK | 10.0.26100.0 or later (provides `cppwinrt.exe`, `MakeAppx.exe`, `SignTool.exe`) |
| CMake | Bundled with Visual Studio or standalone 3.20+ |
| LAF Token | Request from `lafaccessrequests@microsoft.com` (see [LAF Setup](#3-laf-token-setup)) |

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

> **⚠️ Important:** Test the certificate setup and app registration on a clean machine (different from your dev machine) to ensure the steps work end-to-end. Certificate issues are common and easier to catch early on a fresh environment.

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
│   ├── main.cpp                    # Orchestrator — entry point + WndProc
│   ├── utils.h / utils.cpp         # Helpers: Log, CheckPackageIdentity, UnlockLAF
│   ├── RemoteDesktopInfo.h/.cpp    # Exercises RemoteDesktopInfo class
│   ├── RemoteDesktopRegistrar.h/.cpp # Exercises RemoteDesktopRegistrar class
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

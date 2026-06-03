// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/// @file RemoteDesktopInfo.cpp
/// @brief Demonstrates the RemoteDesktopInfo class from Windows.System.RemoteDesktop.Provider.
///
/// RemoteDesktopInfo represents a single remote desktop (Cloud PC).
/// This file only tests the class itself — constructor and properties.
/// Registration (appending to DesktopInfos) is in RemoteDesktopRegistrar.cpp.
///
/// API docs: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopinfo?view=winrt-28000

#include "pch.h"
#include "RemoteDesktopInfo.h"
#include "utils.h"

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;

void DemoRemoteDesktopInfo()
{
    Log(L"=== RemoteDesktopInfo Demo ===");

    // --- Constructor: RemoteDesktopInfo(string id, string displayName) ---
    // The constructor can throw RPC_E_SERVERFAULT (0x80010105) if the Remote Desktop
    // Provider COM server is unavailable, or E_ACCESSDENIED if the LAF has not been
    // unlocked. Ensure UnlockLimitedAccessFeature() succeeds before calling this.
    rdp::RemoteDesktopInfo info{ nullptr };
    try
    {
        info = rdp::RemoteDesktopInfo{ L"sample-cloud-pc-id-2", L"Sample Cloud PC-2" };
    }
    catch (const winrt::hresult_error& ex)
    {
        // Most common cause: the LAF was not unlocked (E_ACCESSDENIED) or the process
        // lacks package identity (RPC_E_SERVERFAULT). Recovery: run setup-sparse-package.ps1
        // and set the LAF_TOKEN environment variable, then restart the app.
        Log(L"  RemoteDesktopInfo constructor failed (0x%08X): %s",
            static_cast<uint32_t>(ex.code()), ex.message().c_str());
        Log(L"  Recovery: ensure LAF is unlocked and sparse package is registered.");
        return;
    }

    Log(L"  Created RemoteDesktopInfo:");
    Log(L"    Id          = %s", info.Id().c_str());          // Property: Id { get; }
    Log(L"    DisplayName = %s", info.DisplayName().c_str()); // Property: DisplayName { get; }
}

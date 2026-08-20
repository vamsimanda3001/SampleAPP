// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/// @file RemoteDesktopRegistrar.cpp
/// @brief Demonstrates the RemoteDesktopRegistrar class from Windows.System.RemoteDesktop.Provider.
///
/// RemoteDesktopRegistrar is a static class that manages registered Cloud PCs.
/// This file exercises all its static methods:
///   - IsSwitchToLocalSessionEnabled() — whether the user can switch to local desktop
///   - DesktopInfos() — IVector of registered Cloud PCs, including Append and enumeration
///
/// API docs: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar?view=winrt-28000

#include "pch.h"
#include "RemoteDesktopRegistrar.h"
#include "utils.h"

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;

void DemoRemoteDesktopRegistrar()
{
    Log(L"=== RemoteDesktopRegistrar Demo ===");

    // Sample Cloud PC identity. Kept as constexpr locals so the id/name cannot drift
    // between the append and remove steps below.
    constexpr wchar_t sampleId[] = L"sample-cloud-pc-id";
    constexpr wchar_t sampleDisplayName[] = L"Sample Cloud PC";

    // --- Static method: IsSwitchToLocalSessionEnabled ---
    // Although defined as a property in WinRT IDL, C++/WinRT projects it as a static method.
    // Guarded like the other Provider calls: it can throw E_ACCESSDENIED when the LAF is
    // not unlocked or the process lacks package identity.
    // See: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar.isswitchtolocalsessionenabled?view=winrt-28000
    try
    {
        bool switchEnabled = rdp::RemoteDesktopRegistrar::IsSwitchToLocalSessionEnabled();
        Log(L"  IsSwitchToLocalSessionEnabled = %s", switchEnabled ? L"true" : L"false");
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  IsSwitchToLocalSessionEnabled failed (0x%08X): %s",
            static_cast<uint32_t>(ex.code()), ex.message().c_str());
    }

    // --- Property: DesktopInfos ---
    // Returns IVector<RemoteDesktopInfo> of all registered Cloud PCs.
    // Appending writes to registry: HKCU\...\RemoteSystemProviders\<PFN>\<id>
    // See: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar.desktopinfos?view=winrt-28000
    // DesktopInfos() can throw E_ACCESSDENIED if the LAF is not unlocked or the process
    // lacks package identity. Recovery: run setup-sparse-package.ps1 and set LAF_TOKEN.
    winrt::Windows::Foundation::Collections::IVector<rdp::RemoteDesktopInfo> desktopInfos{ nullptr };
    try
    {
        desktopInfos = rdp::RemoteDesktopRegistrar::DesktopInfos();
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  DesktopInfos() failed (0x%08X): %s",
            static_cast<uint32_t>(ex.code()), ex.message().c_str());
        Log(L"  Recovery: ensure LAF is unlocked and sparse package is registered.");
        return;
    }

    // Cache the size once rather than re-evaluating the projected call each iteration.
    uint32_t count = desktopInfos.Size();
    Log(L"  DesktopInfos count = %u", count);

    for (uint32_t i = 0; i < count; ++i)
    {
        rdp::RemoteDesktopInfo entry = desktopInfos.GetAt(i);
        Log(L"    [%u] Id=%s  DisplayName=%s", i, entry.Id().c_str(), entry.DisplayName().c_str());
    }

    // --- Append: register a sample Cloud PC ---
    // Append can throw E_ACCESSDENIED if the LAF is not unlocked.
    rdp::RemoteDesktopInfo info{ sampleId, sampleDisplayName };
    try
    {
        desktopInfos.Append(info);
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  Append failed (0x%08X): %s",
            static_cast<uint32_t>(ex.code()), ex.message().c_str());
        Log(L"  Recovery: verify LAF token is valid and the feature status is Available.");
        return;
    }
    Log(L"  Appended '%s' to DesktopInfos -- should now appear in registry", sampleId);

    // --- RemoveAt: clean up the entry we just added ---
    // Demonstrating RemoveAt keeps the sample self-cleaning and idempotent: the sample id
    // stays stable and re-running does not accumulate duplicate registry entries.
    // The freshly appended entry is the last element in the vector.
    try
    {
        desktopInfos.RemoveAt(desktopInfos.Size() - 1);
        Log(L"  Removed '%s' from DesktopInfos -- registry entry cleaned up", sampleId);
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  RemoveAt failed (0x%08X): %s",
            static_cast<uint32_t>(ex.code()), ex.message().c_str());
    }
}

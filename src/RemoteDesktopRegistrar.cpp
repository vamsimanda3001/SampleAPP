/// @file RemoteDesktopRegistrar.cpp
/// @brief Demonstrates the RemoteDesktopRegistrar class from Windows.System.RemoteDesktop.Provider.
///
/// RemoteDesktopRegistrar is a static class that manages registered Cloud PCs.
/// This file tests all its static properties and methods:
///   - IsSwitchToLocalSessionEnabled (property)
///   - DesktopInfos (property) — including Append and enumeration
///
/// API docs: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar?view=winrt-28000

#include "RemoteDesktopRegistrar.h"
#include "utils.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.RemoteDesktop.Provider.h>

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;

/// Exercises RemoteDesktopRegistrar's static properties and methods.
void DemoRemoteDesktopRegistrar()
{
    Log(L"=== RemoteDesktopRegistrar Demo ===");
    try
    {
        // --- Property: IsSwitchToLocalSessionEnabled ---
        // See: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar.isswitchtolocalsessionenabled?view=winrt-28000
        bool switchEnabled = rdp::RemoteDesktopRegistrar::IsSwitchToLocalSessionEnabled();
        Log(L"  IsSwitchToLocalSessionEnabled = %s", switchEnabled ? L"true" : L"false");

        // --- Property: DesktopInfos ---
        // Returns IVector<RemoteDesktopInfo> of all registered Cloud PCs.
        // Appending writes to registry: HKCU\...\RemoteSystemProviders\<PFN>\<id>
        // See: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar.desktopinfos?view=winrt-28000
        auto desktopInfos = rdp::RemoteDesktopRegistrar::DesktopInfos();
        Log(L"  DesktopInfos count = %u", desktopInfos.Size());

        // Register a sample Cloud PC (skip if already present)
        const wchar_t* sampleId = L"sample-cloud-pc-id-2";
        bool alreadyExists = false;
        for (uint32_t i = 0; i < desktopInfos.Size(); ++i)
        {
            auto entry = desktopInfos.GetAt(i);
            Log(L"    [%u] Id=%s  DisplayName=%s", i, entry.Id().c_str(), entry.DisplayName().c_str());
            if (entry.Id() == sampleId)
            {
                alreadyExists = true;
            }
        }

        if (!alreadyExists)
        {
            rdp::RemoteDesktopInfo info{ sampleId, L"Sample Cloud PC-2" };
            desktopInfos.Append(info);
            Log(L"  Appended '%s' to DesktopInfos — should now appear in registry", sampleId);
        }
        else
        {
            Log(L"  '%s' already registered, skipping Append", sampleId);
        }
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  ERROR (0x%08X): %s", static_cast<uint32_t>(ex.code()), ex.message().c_str());
    }
}

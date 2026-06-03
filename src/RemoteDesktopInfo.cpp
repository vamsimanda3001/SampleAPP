/// @file RemoteDesktopInfo.cpp
/// @brief Demonstrates the RemoteDesktopInfo class from Windows.System.RemoteDesktop.Provider.
///
/// RemoteDesktopInfo represents a single remote desktop (Cloud PC). The provider
/// constructs one per Cloud PC and appends it to RemoteDesktopRegistrar::DesktopInfos()
/// so the Windows shell (Task View) can discover and display it.
///
/// API docs: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopinfo?view=winrt-28000

#include "pch.h"
#include "RemoteDesktopInfo.h"
#include "utils.h"

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;

/// Exercises RemoteDesktopInfo: constructs an instance, reads its properties,
/// and registers it with the shell via RemoteDesktopRegistrar::DesktopInfos().
void DemoRemoteDesktopInfo()
{
    Log(L"=== RemoteDesktopInfo Demo ===");
    try
    {
        // --- Test constructor and properties ---
        // RemoteDesktopInfo(string id, string displayName)
        rdp::RemoteDesktopInfo info{ L"sample-cloud-pc-id-2", L"Sample Cloud PC-2" };

        Log(L"  Created RemoteDesktopInfo:");
        Log(L"    Id          = %s", info.Id().c_str());          // Property: Id { get; }
        Log(L"    DisplayName = %s", info.DisplayName().c_str()); // Property: DisplayName { get; }

        // --- Test registration via RemoteDesktopRegistrar::DesktopInfos() ---
        // Appending to this IVector writes an entry to the registry at:
        //   HKCU\...\RemoteSystemProviders\<PFN>\<id>
        // See: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar.desktopinfos?view=winrt-28000
        auto desktopInfos = rdp::RemoteDesktopRegistrar::DesktopInfos();

        bool alreadyExists = false;
        for (uint32_t i = 0; i < desktopInfos.Size(); ++i)
        {
            if (desktopInfos.GetAt(i).Id() == info.Id())
            {
                alreadyExists = true;
                Log(L"  Already registered in DesktopInfos (index %u)", i);
                break;
            }
        }

        if (!alreadyExists)
        {
            desktopInfos.Append(info);
            Log(L"  Appended to DesktopInfos — should now appear in registry");
        }
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  ERROR (0x%08X): %s", static_cast<uint32_t>(ex.code()), ex.message().c_str());
    }
}

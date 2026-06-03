/// @file RemoteDesktopInfo.cpp
/// @brief Demonstrates the RemoteDesktopInfo class from Windows.System.RemoteDesktop.Provider.
///
/// RemoteDesktopInfo represents a single remote desktop (Cloud PC).
/// This file only tests the class itself — constructor and properties.
/// Registration (appending to DesktopInfos) is in RemoteDesktopRegistrar.cpp.
///
/// API docs: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopinfo?view=winrt-28000

#include "RemoteDesktopInfo.h"
#include "utils.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.RemoteDesktop.Provider.h>

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;

/// Exercises RemoteDesktopInfo: constructor and read-only properties.
void DemoRemoteDesktopInfo()
{
    Log(L"=== RemoteDesktopInfo Demo ===");
    try
    {
        // --- Constructor: RemoteDesktopInfo(string id, string displayName) ---
        rdp::RemoteDesktopInfo info{ L"sample-cloud-pc-id-2", L"Sample Cloud PC-2" };

        Log(L"  Created RemoteDesktopInfo:");
        Log(L"    Id          = %s", info.Id().c_str());          // Property: Id { get; }
        Log(L"    DisplayName = %s", info.DisplayName().c_str()); // Property: DisplayName { get; }
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  ERROR (0x%08X): %s", static_cast<uint32_t>(ex.code()), ex.message().c_str());
    }
}

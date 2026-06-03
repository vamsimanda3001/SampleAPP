/// @file RemoteDesktopRegistrar.cpp
/// @brief Demonstrates the RemoteDesktopRegistrar class from Windows.System.RemoteDesktop.Provider.
///
/// RemoteDesktopRegistrar is a static class that provides read-only properties
/// for querying the state of registered remote desktops. Unlike RemoteDesktopInfo
/// (which tests constructors and functions like Append), this file tests
/// static properties — read-only accessors that return current system state.
///
/// API docs: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar?view=winrt-28000

#include "pch.h"
#include "RemoteDesktopRegistrar.h"
#include "utils.h"

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;

/// Exercises RemoteDesktopRegistrar's static properties.
/// Note: This tests properties (read-only state queries), whereas
/// DemoRemoteDesktopInfo() tests functions (constructing and appending).
void DemoRemoteDesktopRegistrar()
{
    Log(L"=== RemoteDesktopRegistrar Demo (Static Properties) ===");
    try
    {
        // --- Property: IsSwitchToLocalSessionEnabled ---
        // Returns whether the user can switch from the remote session back to the local desktop.
        // See: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar.isswitchtolocalsessionenabled?view=winrt-28000
        bool switchEnabled = rdp::RemoteDesktopRegistrar::IsSwitchToLocalSessionEnabled();
        Log(L"  IsSwitchToLocalSessionEnabled = %s", switchEnabled ? L"true" : L"false");

        // --- Property: DesktopInfos ---
        // Just read the count here; the full enumeration and Append logic
        // lives in DemoRemoteDesktopInfo() to avoid duplication.
        // See: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopregistrar.desktopinfos?view=winrt-28000
        auto desktopInfos = rdp::RemoteDesktopRegistrar::DesktopInfos();
        Log(L"  DesktopInfos count = %u", desktopInfos.Size());
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  ERROR (0x%08X): %s", static_cast<uint32_t>(ex.code()), ex.message().c_str());
    }
}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/// @file RemoteDesktopConnectionInfo.cpp
/// @brief Demonstrates the RemoteDesktopConnectionInfo class from Windows.System.RemoteDesktop.Provider.
///
/// RemoteDesktopConnectionInfo represents a single, live remote desktop connection (a Cloud PC
/// session that the OS launched). A provider obtains an instance by calling the static
/// GetForLaunchUri() with the activation URI the shell handed it, then drives the connection's
/// lifecycle through the instance methods:
///   - GetForLaunchUri(Uri, WindowId)        -- static factory; registers a window as the remote desktop
///   - SetConnectionStatus(status)           -- reports Connecting/Connected/UserInputNeeded/Disconnected
///   - SwitchToLocalSession()                -- asks the OS to switch back to the local session
///   - PerformLocalActionFromRemote(action)  -- invokes a local Settings action on behalf of the remote
///
/// This sample runs on a devbox with no real Cloud PC launch, so GetForLaunchUri() is expected to
/// fail (there is no connection associated with our synthetic URI). We still enumerate every
/// RemoteDesktopConnectionStatus and RemoteDesktopLocalAction value, derive a real WindowId, and
/// -- when an instance is available -- exercise each instance method. Every Provider call is guarded
/// so the sample degrades gracefully instead of crashing.
///
/// API docs: https://learn.microsoft.com/en-us/uwp/api/windows.system.remotedesktop.provider.remotedesktopconnectioninfo?view=winrt-28000

#include "pch.h"
#include "RemoteDesktopConnectionInfo.h"
#include "utils.h"

// Win32 <-> WinRT bridge that turns an HWND into a Windows.UI.WindowId. Declared in the
// Windows SDK (um\windows.ui.interop.h); requires NTDDI_VERSION >= NTDDI_WIN10_CU.
#include <windows.ui.interop.h>

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;

// Human-readable name for each RemoteDesktopConnectionStatus value.
static const wchar_t* ToString(rdp::RemoteDesktopConnectionStatus status)
{
    switch (status)
    {
    case rdp::RemoteDesktopConnectionStatus::Connecting:      return L"Connecting";
    case rdp::RemoteDesktopConnectionStatus::Connected:       return L"Connected";
    case rdp::RemoteDesktopConnectionStatus::UserInputNeeded: return L"UserInputNeeded";
    case rdp::RemoteDesktopConnectionStatus::Disconnected:    return L"Disconnected";
    default:                                                  return L"<unknown>";
    }
}

// Human-readable name for each RemoteDesktopLocalAction value.
static const wchar_t* ToString(rdp::RemoteDesktopLocalAction action)
{
    switch (action)
    {
    case rdp::RemoteDesktopLocalAction::ShowBluetoothSettings:     return L"ShowBluetoothSettings";
    case rdp::RemoteDesktopLocalAction::ShowSystemSoundSettings:   return L"ShowSystemSoundSettings";
    case rdp::RemoteDesktopLocalAction::ShowSystemDisplaySettings: return L"ShowSystemDisplaySettings";
    case rdp::RemoteDesktopLocalAction::ShowSystemAccountSettings: return L"ShowSystemAccountSettings";
    case rdp::RemoteDesktopLocalAction::ShowLocalSettings:         return L"ShowLocalSettings";
    default:                                                       return L"<unknown>";
    }
}

void DemoRemoteDesktopConnectionInfo(HWND hwnd)
{
    Log(L"=== RemoteDesktopConnectionInfo Demo ===");

    // --- Enum: RemoteDesktopConnectionStatus ---
    // The values a provider reports via SetConnectionStatus. Logged here so every value in the
    // namespace is exercised even when no live connection is available to set them on.
    constexpr rdp::RemoteDesktopConnectionStatus allStatuses[] = {
        rdp::RemoteDesktopConnectionStatus::Connecting,
        rdp::RemoteDesktopConnectionStatus::Connected,
        rdp::RemoteDesktopConnectionStatus::UserInputNeeded,
        rdp::RemoteDesktopConnectionStatus::Disconnected,
    };
    Log(L"  RemoteDesktopConnectionStatus values:");
    for (auto status : allStatuses)
    {
        Log(L"    %-16s = %d", ToString(status), static_cast<int>(status));
    }

    // --- Enum: RemoteDesktopLocalAction ---
    // The local Settings actions a provider can invoke via PerformLocalActionFromRemote.
    constexpr rdp::RemoteDesktopLocalAction allActions[] = {
        rdp::RemoteDesktopLocalAction::ShowBluetoothSettings,
        rdp::RemoteDesktopLocalAction::ShowSystemSoundSettings,
        rdp::RemoteDesktopLocalAction::ShowSystemDisplaySettings,
        rdp::RemoteDesktopLocalAction::ShowSystemAccountSettings,
        rdp::RemoteDesktopLocalAction::ShowLocalSettings,
    };
    Log(L"  RemoteDesktopLocalAction values:");
    for (auto action : allActions)
    {
        Log(L"    %-25s = %d", ToString(action), static_cast<int>(action));
    }

    // --- Derive a WindowId from our HWND ---
    // GetForLaunchUri needs a WindowId to register the window as the surface hosting the remote
    // desktop. winrt::Windows::UI::WindowId and ABI::Windows::UI::WindowId are layout-compatible
    // (both a single uint64_t), so the interop out-param can be reinterpret_cast safely.
    winrt::Windows::UI::WindowId windowId{};
    try
    {
        winrt::check_hresult(
            ::GetWindowIdFromWindow(hwnd, reinterpret_cast<ABI::Windows::UI::WindowId*>(&windowId)));
        Log(L"  Derived WindowId = %llu", windowId.Value);
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  GetWindowIdFromWindow failed (0x%08X): %s",
            static_cast<uint32_t>(ex.code()), ex.message().c_str());
        Log(L"  Cannot demo GetForLaunchUri without a WindowId -- skipping.");
        return;
    }

    // --- Static factory: GetForLaunchUri(Uri, WindowId) ---
    // In production the shell activates the provider with a launch URI that identifies the Cloud PC
    // session; the provider passes that exact URI here. On this devbox we synthesize a URI, so the OS
    // has no connection to associate and the call is expected to fail. That failure is the intended,
    // gracefully-handled path -- it proves the call site and argument marshalling are correct.
    const winrt::Windows::Foundation::Uri launchUri{ L"ms-cloudpc:launch?id=sample-cloud-pc-id" };
    rdp::RemoteDesktopConnectionInfo connectionInfo{ nullptr };
    try
    {
        connectionInfo = rdp::RemoteDesktopConnectionInfo::GetForLaunchUri(launchUri, windowId);
        Log(L"  GetForLaunchUri succeeded -- live connection acquired.");
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  GetForLaunchUri failed (0x%08X): %s",
            static_cast<uint32_t>(ex.code()), ex.message().c_str());
        Log(L"  Expected on a devbox with no real Cloud PC launch. In production the shell supplies");
        Log(L"  the activation URI. Skipping instance-method demo (no connection to drive).");
        return;
    }

    // --- Instance methods (only reachable with a live connection) ---
    // Each call is guarded independently so one failure does not mask the others.

    // SetConnectionStatus: report each lifecycle state to the OS.
    for (auto status : allStatuses)
    {
        try
        {
            connectionInfo.SetConnectionStatus(status);
            Log(L"  SetConnectionStatus(%s) OK", ToString(status));
        }
        catch (const winrt::hresult_error& ex)
        {
            Log(L"  SetConnectionStatus(%s) failed (0x%08X): %s",
                ToString(status), static_cast<uint32_t>(ex.code()), ex.message().c_str());
        }
    }

    // PerformLocalActionFromRemote: open each local Settings page on behalf of the remote.
    for (auto action : allActions)
    {
        try
        {
            connectionInfo.PerformLocalActionFromRemote(action);
            Log(L"  PerformLocalActionFromRemote(%s) OK", ToString(action));
        }
        catch (const winrt::hresult_error& ex)
        {
            Log(L"  PerformLocalActionFromRemote(%s) failed (0x%08X): %s",
                ToString(action), static_cast<uint32_t>(ex.code()), ex.message().c_str());
        }
    }

    // SwitchToLocalSession: ask the OS to bring the user back to their local session.
    try
    {
        connectionInfo.SwitchToLocalSession();
        Log(L"  SwitchToLocalSession() OK");
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  SwitchToLocalSession() failed (0x%08X): %s",
            static_cast<uint32_t>(ex.code()), ex.message().c_str());
    }
}

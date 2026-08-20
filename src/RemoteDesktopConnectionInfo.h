// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/// @file RemoteDesktopConnectionInfo.h
/// @brief Declaration for the RemoteDesktopConnectionInfo demo function.

#pragma once

#include <windows.h>

/// Exercises RemoteDesktopConnectionInfo: the static GetForLaunchUri factory plus the
/// instance methods (SetConnectionStatus, SwitchToLocalSession, PerformLocalActionFromRemote),
/// and enumerates the RemoteDesktopConnectionStatus and RemoteDesktopLocalAction values.
///
/// @param hwnd A live top-level window handle. GetForLaunchUri requires a WindowId, which is
///             derived from this HWND via the Windows.UI interop bridge.
void DemoRemoteDesktopConnectionInfo(HWND hwnd);

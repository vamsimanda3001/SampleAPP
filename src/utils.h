/// @file utils.h
/// @brief Utility functions independent of the Provider namespace APIs.
///
/// Contains logging, package identity checks, and LAF unlock — none of which
/// use Windows.System.RemoteDesktop.Provider types directly.

#pragma once

#include <cstdio>

/// Initialize console output and open a log file next to the exe.
void InitLog();

/// Close the log file handle.
void CloseLog();

/// Log a formatted wide-string to console, OutputDebugString, and log file.
void Log(const wchar_t* fmt, ...);

/// Verify the process has sparse package identity. Returns true if identity is present.
bool CheckPackageIdentity();

/// Unlock the Remote Desktop Provider LAF using the LAF_TOKEN environment variable.
/// Returns true if the feature status is Available or AvailableWithoutToken.
bool UnlockLimitedAccessFeature();

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/// @file utils.h
/// @brief Utility functions independent of the Provider namespace APIs.
///
/// Contains logging, package identity checks, and LAF unlock — none of which
/// use Windows.System.RemoteDesktop.Provider types directly.

#pragma once

#include <cstdio>

/// Allocates a console window and opens output.log next to the exe.
void InitLog();

/// Close the log file handle opened by InitLog().
void CloseLog();

/// Load KEY=VALUE pairs from a .env file into the process environment.
/// Searches next to the exe first, then walks up parent directories to find .env.
/// Only sets variables that are not already present in the environment.
void LoadEnvFile();

/// Printf-style wide-string logging to console, OutputDebugString, and log file.
void Log(const wchar_t* fmt, ...);

/// Check whether this process has sparse package identity.
/// If identity is missing, enumerates registered packages and prints diagnostic info.
/// Returns true if the process has package identity.
bool CheckPackageIdentity();

/// Unlock the Remote Desktop Provider Limited Access Feature.
/// Reads the LAF token from the LAF_TOKEN environment variable (set via .env or shell).
/// Returns true if the feature status is Available or AvailableWithoutToken.
/// @note The first-chance exception 0x80040111 during TryUnlockFeature is normal and expected.
bool UnlockLimitedAccessFeature();

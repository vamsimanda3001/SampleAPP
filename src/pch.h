// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/// @file pch.h
/// @brief Precompiled header — heavy/common includes parsed once to speed up builds.
///
/// Every .cpp in this project implicitly includes this file via CMake's
/// target_precompile_headers(). Add new WinRT projection headers here as
/// Phase 2-4 classes are implemented.

#pragma once

// ─── Windows SDK ────────────────────────────────────────────────────────────
#include <windows.h>
#include <appmodel.h>

// ─── C++ Standard Library ───────────────────────────────────────────────────
#include <cstdio>
#include <cstdarg>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// ─── C++/WinRT base + common projections ────────────────────────────────────
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

// ─── WinRT projections used by this project ─────────────────────────────────
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Management.Deployment.h>
#include <winrt/Windows.System.RemoteDesktop.Provider.h>

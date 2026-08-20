// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/// @file utils.cpp
/// @brief Implementation of utility functions (logging, package identity, LAF).

#include "pch.h"
#include "utils.h"

namespace app = winrt::Windows::ApplicationModel;

// ─── Logging ────────────────────────────────────────────────────────────────
static FILE* g_logFile = nullptr;

void InitLog()
{
    AllocConsole();
    FILE* fp = nullptr;
    freopen_s(&fp, "CONOUT$", "w", stdout);

    wchar_t exeBuf[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exeBuf, MAX_PATH);
    std::filesystem::path exeDir = std::filesystem::path(exeBuf).parent_path();
    std::wstring logPath = (exeDir / L"output.log").wstring();
    _wfopen_s(&g_logFile, logPath.c_str(), L"w");
}

void CloseLog()
{
    if (g_logFile)
    {
        fclose(g_logFile);
        g_logFile = nullptr;
    }
}

void LoadEnvFile()
{
    // Find exe directory
    wchar_t exeBuf[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exeBuf, MAX_PATH);
    std::filesystem::path dir = std::filesystem::path(exeBuf).parent_path();

    // Walk up from exe directory looking for .env
    std::filesystem::path envPath;
    while (true)
    {
        std::filesystem::path candidate = dir / L".env";
        if (std::filesystem::exists(candidate))
        {
            envPath = candidate;
            break;
        }
        std::filesystem::path parent = dir.parent_path();
        if (parent == dir)
        {
            break;
        }
        dir = parent;
    }

    if (envPath.empty())
    {
        return;
    }

    std::ifstream file(envPath);
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Skip comments and blank lines
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        // Find the '=' separator
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos)
        {
            continue;
        }

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        // Trim trailing \r (CRLF line endings)
        if (!value.empty() && value.back() == '\r')
        {
            value.pop_back();
        }

        // Convert to wide strings and set env var (only if not already set)
        std::wstring wKey(key.begin(), key.end());
        std::wstring wVal(value.begin(), value.end());

        // Don't overwrite vars already set in the shell
        if (GetEnvironmentVariableW(wKey.c_str(), nullptr, 0) == 0)
        {
            SetEnvironmentVariableW(wKey.c_str(), wVal.c_str());
        }
    }
}

void Log(const wchar_t* fmt, ...)
{
    wchar_t buf[1024];
    va_list args;
    va_start(args, fmt);
    vswprintf_s(buf, fmt, args);
    va_end(args);

    wprintf(L"%s\n", buf);
    OutputDebugStringW(buf);
    OutputDebugStringW(L"\n");
    if (g_logFile)
    {
        fwprintf(g_logFile, L"%s\n", buf);
        fflush(g_logFile);
    }
}

// ─── Package Identity Check ─────────────────────────────────────────────────
bool CheckPackageIdentity()
{
    Log(L"=== Package Identity Check ===");

    wchar_t exePath[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    Log(L"  Exe path: %s", exePath);

    UINT32 length = 0;
    LONG rc = GetCurrentPackageFullName(&length, nullptr);

    if (rc == APPMODEL_ERROR_NO_PACKAGE)
    {
        Log(L"  [FAIL] No package identity detected.");

        Log(L"  Checking if sparse package is registered...");
        try
        {
            winrt::Windows::Management::Deployment::PackageManager pm;
            winrt::Windows::Foundation::Collections::IIterable<winrt::Windows::ApplicationModel::Package> packages = pm.FindPackagesForUser(L"");
            bool found = false;
            for (const auto& pkg : packages)
            {
                if (pkg.Id().Name() == L"RemoteDesktopProviderSample")
                {
                    found = true;
                    Log(L"  [INFO] Package IS registered:");
                    Log(L"    FullName        = %s", pkg.Id().FullName().c_str());
                    Log(L"    FamilyName      = %s", pkg.Id().FamilyName().c_str());
                    Log(L"    InstalledPath   = %s", pkg.InstalledPath().c_str());
                    Log(L"    EffectivePath   = %s", pkg.EffectivePath().c_str());
                    Log(L"    IsOptional      = %s", pkg.IsOptional() ? L"true" : L"false");
                    Log(L"    SignatureKind   = %d", static_cast<int>(pkg.SignatureKind()));
                    Log(L"    EffectiveExternalPath = %s", pkg.EffectiveExternalPath().c_str());
                    break;
                }
            }
            if (!found)
            {
                Log(L"  [INFO] Package NOT registered. Run setup-sparse-package.ps1 first.");
            }
        }
        catch (const winrt::hresult_error& ex)
        {
            Log(L"  [WARN] Could not enumerate packages: 0x%08X %s",
                static_cast<uint32_t>(ex.code()), ex.message().c_str());
        }

        return false;
    }

    std::wstring fullName(length, L'\0');
    rc = GetCurrentPackageFullName(&length, fullName.data());
    if (rc == ERROR_SUCCESS)
    {
        fullName.resize(length - 1);
        Log(L"  [OK] Package Full Name: %s", fullName.c_str());
    }

    length = 0;
    GetCurrentPackageFamilyName(&length, nullptr);
    std::wstring familyName(length, L'\0');
    GetCurrentPackageFamilyName(&length, familyName.data());
    familyName.resize(length - 1);
    Log(L"  [OK] Package Family Name: %s", familyName.c_str());

    // Lower-level GetCurrentPackageId diagnostic
    {
        UINT32 bufLen = 0;
        LONG rc2 = GetCurrentPackageId(&bufLen, nullptr);
        Log(L"  GetCurrentPackageId rc = %ld, bufLen = %u", rc2, bufLen);
        if (rc2 == ERROR_INSUFFICIENT_BUFFER && bufLen > 0)
        {
            std::vector<uint8_t> buf(bufLen);
            rc2 = GetCurrentPackageId(&bufLen, buf.data());
            if (rc2 == ERROR_SUCCESS)
            {
                auto* pkgId = reinterpret_cast<PACKAGE_ID*>(buf.data());
                Log(L"  PackageId.name = %s", pkgId->name);
                Log(L"  PackageId.publisher = %s", pkgId->publisher);
            }
        }
    }

    return true;
}

// ─── LAF Unlock ─────────────────────────────────────────────────────────────
bool UnlockLimitedAccessFeature()
{
    Log(L"=== Unlocking Limited Access Feature ===");

    // Defensive check: LAF requires package identity. Fail early with a clear message
    // rather than letting TryUnlockFeature throw a cryptic RPC_E_SERVERFAULT.
    {
        UINT32 length = 0;
        if (GetCurrentPackageFullName(&length, nullptr) == APPMODEL_ERROR_NO_PACKAGE)
        {
            Log(L"  [ERROR] No package identity -- LAF unlock requires a sparse package.");
            Log(L"  Recovery: run setup-sparse-package.ps1 to register the sparse package.");
            return false;
        }
    }

    DWORD tokenLen = GetEnvironmentVariableW(L"LAF_TOKEN", nullptr, 0);
    if (tokenLen == 0)
    {
        Log(L"  [ERROR] LAF_TOKEN not set. Create .env with LAF_TOKEN=your-token");
        return false;
    }
    std::wstring token(tokenLen, L'\0');
    GetEnvironmentVariableW(L"LAF_TOKEN", token.data(), tokenLen);
    token.resize(tokenLen - 1);

    // TryUnlockFeature can throw hresult_error if the package identity or attestation
    // string is invalid. The first-chance 0x80040111 (CO_E_OBJNOTCONNECTED) is expected
    // and handled internally by the API; it does not indicate a real failure.
    app::LimitedAccessFeatureRequestResult result{ nullptr };
    try
    {
        result = app::LimitedAccessFeatures::TryUnlockFeature(
            L"com.microsoft.windows.system.remotedesktop.provider_v1",
            token.c_str(),
            L"955ksfw34s3d4 has registered their use of com.microsoft.windows.system.remotedesktop.provider_v1 with Microsoft and agrees to the terms of use.");
    }
    catch (const winrt::hresult_error& ex)
    {
        // Typical causes: invalid PFN in attestation string, or the COM server for LAF
        // is not available on this OS build. Recovery: verify the PFN matches the
        // sparse package and that the OS is Windows 11 Build 26100+.
        Log(L"  TryUnlockFeature threw (0x%08X): %s",
            static_cast<uint32_t>(ex.code()), ex.message().c_str());
        Log(L"  Recovery: verify PFN, OS build (26100+), and LAF_TOKEN value.");
        return false;
    }

    app::LimitedAccessFeatureStatus status = result.Status();
    if (status == app::LimitedAccessFeatureStatus::Available)
    {
        Log(L"  LAF Status: Available");
        return true;
    }
    else if (status == app::LimitedAccessFeatureStatus::AvailableWithoutToken)
    {
        Log(L"  LAF Status: AvailableWithoutToken");
        return true;
    }
    else if (status == app::LimitedAccessFeatureStatus::Unavailable)
    {
        Log(L"  LAF Status: Unavailable");
    }
    else if (status == app::LimitedAccessFeatureStatus::Unknown)
    {
        Log(L"  LAF Status: Unknown");
    }
    else
    {
        Log(L"  LAF Status: %d", static_cast<int>(status));
    }

    return false;
}

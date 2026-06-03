/// @file utils.cpp
/// @brief Implementation of utility functions (logging, package identity, LAF).

#include "pch.h"
#include "utils.h"

namespace app = winrt::Windows::ApplicationModel;

// ─── Logging ────────────────────────────────────────────────────────────────
static FILE* g_logFile = nullptr;

/// Allocates a console window and opens output.log next to the exe.
void InitLog()
{
    AllocConsole();
    FILE* fp = nullptr;
    freopen_s(&fp, "CONOUT$", "w", stdout);

    wchar_t exeDir[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exeDir, MAX_PATH);
    *wcsrchr(exeDir, L'\\') = L'\0';
    std::wstring logPath = std::wstring(exeDir) + L"\\output.log";
    _wfopen_s(&g_logFile, logPath.c_str(), L"w");
}

/// Closes the log file handle opened by InitLog().
void CloseLog()
{
    if (g_logFile)
    {
        fclose(g_logFile);
        g_logFile = nullptr;
    }
}

/// Loads KEY=VALUE pairs from a .env file into the process environment.
/// Searches next to the exe first, then walks up parent directories until
/// the repo root (where .gitignore lives) or drive root is reached.
void LoadEnvFile()
{
    // Find exe directory
    wchar_t exeDir[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exeDir, MAX_PATH);
    *wcsrchr(exeDir, L'\\') = L'\0';

    // Walk up from exe directory looking for .env
    std::wstring dir = exeDir;
    std::wstring envPath;
    while (true)
    {
        std::wstring candidate = dir + L"\\.env";
        if (GetFileAttributesW(candidate.c_str()) != INVALID_FILE_ATTRIBUTES)
        {
            envPath = candidate;
            break;
        }
        // Go up one level
        auto pos = dir.find_last_of(L'\\');
        if (pos == std::wstring::npos || pos == 0)
            break;
        dir = dir.substr(0, pos);
    }

    if (envPath.empty())
        return;

    FILE* f = nullptr;
    _wfopen_s(&f, envPath.c_str(), L"r");
    if (!f)
        return;

    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        // Skip comments and blank lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;

        // Find the '=' separator
        char* eq = strchr(line, '=');
        if (!eq)
            continue;

        *eq = '\0';
        char* value = eq + 1;

        // Trim trailing newline from value
        size_t len = strlen(value);
        while (len > 0 && (value[len - 1] == '\n' || value[len - 1] == '\r'))
            value[--len] = '\0';

        // Convert to wide strings and set env var (only if not already set)
        wchar_t wKey[256]{}, wVal[256]{};
        MultiByteToWideChar(CP_UTF8, 0, line, -1, wKey, 256);
        MultiByteToWideChar(CP_UTF8, 0, value, -1, wVal, 256);

        // Don't overwrite vars already set in the shell
        wchar_t existing[4]{};
        if (GetEnvironmentVariableW(wKey, existing, 4) == 0)
        {
            SetEnvironmentVariableW(wKey, wVal);
        }
    }

    fclose(f);
}

/// Printf-style wide-string logging to console, OutputDebugString, and log file.
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
/// Checks whether this process has sparse package identity.
/// If identity is missing, attempts to find the package in the registered packages list
/// and prints diagnostic info. Returns true if the process has package identity.
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
            auto packages = pm.FindPackagesForUser(L"");
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
            std::vector<BYTE> buf(bufLen);
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
/// Unlocks the Remote Desktop Provider Limited Access Feature.
/// Reads the LAF token from the LAF_TOKEN environment variable (set via .env or shell).
/// Returns true if the feature is unlocked (Available or AvailableWithoutToken).
///
/// The first-chance exception 0x80040111 during TryUnlockFeature is normal and expected.
bool UnlockLimitedAccessFeature()
{
    Log(L"=== Unlocking Limited Access Feature ===");
    try
    {
        wchar_t tokenBuf[256]{};
        if (!GetEnvironmentVariableW(L"LAF_TOKEN", tokenBuf, 256))
        {
            Log(L"  [ERROR] LAF_TOKEN not set. Create .env with LAF_TOKEN=your-token");
            return false;
        }

        auto result = app::LimitedAccessFeatures::TryUnlockFeature(
            L"com.microsoft.windows.system.remotedesktop.provider_v1",
            tokenBuf,
            L"955ksfw34s3d4 has registered their use of com.microsoft.windows.system.remotedesktop.provider_v1 with Microsoft and agrees to the terms of use.");

        auto status = result.Status();
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
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  LAF ERROR (0x%08X): %s", static_cast<uint32_t>(ex.code()), ex.message().c_str());
    }
    return false;
}

// RemoteDesktopProviderSample — exercises Windows.System.RemoteDesktop.Provider APIs
#include <windows.h>
#include <appmodel.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Management.Deployment.h>
#include <winrt/Windows.System.RemoteDesktop.Provider.h>

#include <cstdio>
#include <string>
#include <vector>

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;
namespace app = winrt::Windows::ApplicationModel;

// ─── Forward declarations ───────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool UnlockLimitedAccessFeature();
void RunRemoteDesktopInfoDemo();
void RunRegistrarDemo();

// ─── Helpers ────────────────────────────────────────────────────────────────
static FILE* g_logFile = nullptr;

void Log(const wchar_t* fmt, ...)
{
    wchar_t buf[1024];
    va_list args;
    va_start(args, fmt);
    vswprintf_s(buf, fmt, args);
    va_end(args);

    // Print to console, debug output, and log file
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

    // 1. Show where THIS exe is running from
    wchar_t exePath[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    Log(L"  Exe path: %s", exePath);

    // 2. Check GetCurrentPackageFullName
    UINT32 length = 0;
    LONG rc = GetCurrentPackageFullName(&length, nullptr);

    if (rc == APPMODEL_ERROR_NO_PACKAGE)
    {
        Log(L"  [FAIL] No package identity detected.");

        // 3. Check if the package is at least registered on this machine
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

    // We have identity
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

    return true;
}

// ─── LAF Unlock ─────────────────────────────────────────────────────────────
bool UnlockLimitedAccessFeature()
{
    Log(L"=== Unlocking Limited Access Feature ===");
    try
    {
        // LAF token from LAF_TOKEN env var (set in .env or shell).
        // Create .env file with: LAF_TOKEN=your-token
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

// ─── Phase 1: RemoteDesktopInfo ─────────────────────────────────────────────
void RunRemoteDesktopInfoDemo()
{
    Log(L"=== RemoteDesktopInfo Demo ===");
    try
    {
        // Construct a RemoteDesktopInfo with an ID and display name
        rdp::RemoteDesktopInfo info{ L"sample-cloud-pc-id-2", L"Sample Cloud PC-2" };

        Log(L"  Created RemoteDesktopInfo:");
        Log(L"    Id          = %s", info.Id().c_str());
        Log(L"    DisplayName = %s", info.DisplayName().c_str());

        // Add it to the DesktopInfos collection — this persists it to the
        // registry so the Windows shell can see it
        auto desktopInfos = rdp::RemoteDesktopRegistrar::DesktopInfos();
        
        // Avoid duplicates: check if this ID already exists
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

// ─── Phase 1: RemoteDesktopRegistrar ────────────────────────────────────────
void RunRegistrarDemo()
{
    Log(L"=== RemoteDesktopRegistrar Demo ===");
    try
    {
        // Check if switching to local session is enabled
        bool switchEnabled = rdp::RemoteDesktopRegistrar::IsSwitchToLocalSessionEnabled();
        Log(L"  IsSwitchToLocalSessionEnabled = %s", switchEnabled ? L"true" : L"false");

        // Enumerate registered desktop infos
        auto desktopInfos = rdp::RemoteDesktopRegistrar::DesktopInfos();
        Log(L"  DesktopInfos count = %u", desktopInfos.Size());

        for (uint32_t i = 0; i < desktopInfos.Size(); ++i)
        {
            auto entry = desktopInfos.GetAt(i);
            Log(L"    [%u] Id=%s  DisplayName=%s", i, entry.Id().c_str(), entry.DisplayName().c_str());
        }
    }
    catch (const winrt::hresult_error& ex)
    {
        Log(L"  ERROR (0x%08X): %s", static_cast<uint32_t>(ex.code()), ex.message().c_str());
    }
}

// ─── Window procedure ──────────────────────────────────────────────────────
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        // Unlock LAF before calling gated APIs
        UnlockLimitedAccessFeature();
        RunRemoteDesktopInfoDemo();
        RunRegistrarDemo();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ─── Entry point ────────────────────────────────────────────────────────────
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    winrt::init_apartment();

    // Allocate a console so we can see Log() output
    AllocConsole();
    FILE* fp = nullptr;
    freopen_s(&fp, "CONOUT$", "w", stdout);

    // Also log to a file next to the exe for easy retrieval
    wchar_t exeDir[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exeDir, MAX_PATH);
    *wcsrchr(exeDir, L'\\') = L'\0';
    std::wstring logPath = std::wstring(exeDir) + L"\\output.log";
    _wfopen_s(&g_logFile, logPath.c_str(), L"w");

    Log(L"RemoteDesktopProviderSample starting... (PID=%u)", GetCurrentProcessId());

    // Check package identity FIRST — before anything else
    bool hasIdentity = CheckPackageIdentity();

    // Also try the lower-level GetCurrentPackageId API
    {
        UINT32 bufLen = 0;
        LONG rc = GetCurrentPackageId(&bufLen, nullptr);
        Log(L"  GetCurrentPackageId rc = %ld, bufLen = %u", rc, bufLen);
        if (rc == ERROR_INSUFFICIENT_BUFFER && bufLen > 0)
        {
            std::vector<BYTE> buf(bufLen);
            rc = GetCurrentPackageId(&bufLen, buf.data());
            if (rc == ERROR_SUCCESS)
            {
                auto* pkgId = reinterpret_cast<PACKAGE_ID*>(buf.data());
                Log(L"  PackageId.name = %s", pkgId->name);
                Log(L"  PackageId.publisher = %s", pkgId->publisher);
            }
        }
    }

    // Register window class
    const wchar_t CLASS_NAME[] = L"RDPProviderSampleClass";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Remote Desktop Provider Sample",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
    {
        Log(L"Failed to create window (error %u)", GetLastError());
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    winrt::uninit_apartment();
    return static_cast<int>(msg.wParam);
}

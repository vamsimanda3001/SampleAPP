#include "RemoteDesktopRegistrar.h"
#include "utils.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.RemoteDesktop.Provider.h>

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;

void DemoRemoteDesktopRegistrar()
{
    Log(L"=== RemoteDesktopRegistrar Demo ===");
    try
    {
        bool switchEnabled = rdp::RemoteDesktopRegistrar::IsSwitchToLocalSessionEnabled();
        Log(L"  IsSwitchToLocalSessionEnabled = %s", switchEnabled ? L"true" : L"false");

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

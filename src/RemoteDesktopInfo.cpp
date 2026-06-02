#include "RemoteDesktopInfo.h"
#include "utils.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.RemoteDesktop.Provider.h>

namespace rdp = winrt::Windows::System::RemoteDesktop::Provider;

void DemoRemoteDesktopInfo()
{
    Log(L"=== RemoteDesktopInfo Demo ===");
    try
    {
        rdp::RemoteDesktopInfo info{ L"sample-cloud-pc-id-2", L"Sample Cloud PC-2" };

        Log(L"  Created RemoteDesktopInfo:");
        Log(L"    Id          = %s", info.Id().c_str());
        Log(L"    DisplayName = %s", info.DisplayName().c_str());

        auto desktopInfos = rdp::RemoteDesktopRegistrar::DesktopInfos();

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

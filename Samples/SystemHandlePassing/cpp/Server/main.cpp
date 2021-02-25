#include <Windows.h>

#include <iostream>

#include <wrl\module.h>

#include <wil\common.h>
#include <wil\com.h>
#include <wil\resource.h>
#include <wil\result.h>

#include "..\ProxyStub\ISparkleFinisher_h.h"

using namespace ::Microsoft::WRL;

static wil::slim_event s_exitEvent;

static void s_releaseNotifier()
{
    s_exitEvent.SetEvent();
}

int __cdecl wmain(int argc, wchar_t** argv)
try
{
    // Prepare COM
    auto uninit = wil::CoInitializeEx(COINIT_MULTITHREADED);

    // Leverage WRL to create basic factories for our objects
    // and get them ready to accept calls.
    auto& mod = Module<OutOfProc>::Create(&s_releaseNotifier);

    THROW_IF_FAILED(mod.RegisterObjects());

    // Wait to hear back from the notifier.
    s_exitEvent.wait(); // infinite wait

    THROW_IF_FAILED(mod.UnregisterObjects());

    // Cleanup of all handles and resources is automatic when using WIL types!

    return 0;
}
catch (const wil::ResultException& ex)
{
    std::cout << "Sparkle Finish Server Failed with error: '" << ex.what() << "'" << std::endl;
}
catch (const std::exception& ex)
{
    std::cout << "Sparkle Finish Server Failed with error: '" << ex.what() << "'" << std::endl;
}

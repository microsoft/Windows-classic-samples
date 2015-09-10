#include <SpellCheckProvider.h>
#include <SpellCheck.h>
#include "SampleSpellChecker.h"
#include <atlbase.h>
#include "resource.h"

class CSampleSpellCheckProviderModule : public CAtlDllModuleT<CSampleSpellCheckProviderModule>
{
} _AtlModule;

// Dll Entry Point
extern "C" BOOL
WINAPI DllMain(_In_ HMODULE hModule, _In_ DWORD dwReason, _In_ PVOID lpvReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            ::DisableThreadLibraryCalls(hModule);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return _AtlModule.DllMain(dwReason, lpvReserved);
}

STDAPI DllCanUnloadNow()
{
    return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ PVOID * ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer()
{
    BOOL fRegisterTypeLib = FALSE;
    HRESULT hr = _AtlModule.DllRegisterServer(fRegisterTypeLib);
    return hr;
}

STDAPI DllUnregisterServer()
{
    BOOL fRegisterTypeLib = FALSE;
    HRESULT hr = _AtlModule.DllUnregisterServer(fRegisterTypeLib);
    return hr;
}

// teddll.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "tedutil.h"
#include "topoviewerwindow.h"
#include "serialization.h"
#include "topoviewercontroller.h"
#include "tedtestsink.h"

class CTedUtilModule : public CAtlDllModuleT<CTedUtilModule>
{
public:
    DECLARE_LIBID(LIBID_TedUtil);
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_TEDUTIL, "{1FFD2046-C6C7-462d-A51F-7F85A375171E}");
// Override CAtlDllModuleT members
};

CTedUtilModule _Module;

OBJECT_ENTRY_AUTO(CLSID_CXMLDataSaver, CXMLDataSaver)
OBJECT_ENTRY_AUTO (CLSID_CXMLDataLoader, CXMLDataLoader)
OBJECT_ENTRY_AUTO(CLSID_CTopoViewerController, CTopoViewerController)
OBJECT_ENTRY_AUTO(CLSID_CTedTestSink, CTedTestSink)

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    return _Module.DllMain(dwReason, lpReserved);
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow()
{
    return _Module.DllCanUnloadNow();
}


STDAPI DllRegisterServer(void)
{
    return _Module.DllRegisterServer();
}

STDAPI DllUnregisterServer(void)
{
    return _Module.DllUnregisterServer();
}

STDAPI DllGetClassObject(
  __in REFCLSID rclsid,  
  __in REFIID riid,      
  __deref_out LPVOID FAR* ppv
)
{
    return _Module.DllGetClassObject(rclsid, riid, ppv);
}

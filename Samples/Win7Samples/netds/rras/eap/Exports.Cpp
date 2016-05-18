/*

Copyright (c) 1999 - 2000 Microsoft Corporation, all rights reserved

Description:
    Implementation of DLL Exports.

*/

#include <windows.h>
#include <lmcons.h>
#include <raseapif.h>
#include <rtutils.h>
#include "ceapcfg.h"
#include <initguid.h>
#include <atlimpl.cpp>

extern "C"
{
    extern  DWORD        g_dwEapTraceId;
    extern  HINSTANCE    g_hInstance;
}

CComModule  _Module;

const IID IID_IEAPProviderConfig = {
    0x66A2DB19,
    0xD706,
    0x11D0,
    {0xA3, 0x7B, 0x00, 0xC0, 0x4F, 0xC9, 0xDA, 0x04}
};

// Define the EAP UI GUIDs here
const CLSID CLSID_EapCfg = { /* 3b9aae60-a032-11d2-95f6-00104b98f3f5 */
    0x3b9aae60,
    0xa032,
    0x11d2,
    {0x95, 0xf6, 0x00, 0x10, 0x4b, 0x98, 0xf3, 0xf5}
};

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_EapCfg, CEapCfg)
END_OBJECT_MAP()

/*

Notes:
    DLL Entry Point

*/

extern "C"
BOOL WINAPI
DllMain(
    HINSTANCE   hInstance,
    DWORD       dwReason,
    LPVOID      /*lpReserved*/
)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;
        g_dwEapTraceId = TraceRegister(L"SampleEAP");
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        TraceDeregister(g_dwEapTraceId);
        g_dwEapTraceId = INVALID_TRACEID;
        _Module.Term();
    }

    return(TRUE);
}

/*

Notes:
    Used to determine whether the DLL can be unloaded by OLE
    
*/

STDAPI
DllCanUnloadNow(
    VOID
)
{
    if (0 == _Module.GetLockCount())
    {
        return(S_OK);
    }
    else
    {
        return(S_FALSE);
    }
}

/*

Notes:
    Returns a class factory to create an object of the requested type
    
*/

STDAPI
DllGetClassObject(
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID*     ppv
)
{
    return(_Module.GetClassObject(rclsid, riid, ppv));
}

/*

Notes:
    Adds entries to the system registry. Registers object, typelib and all
    interfaces in typelib
    
*/

STDAPI
DllRegisterServer(
    VOID
)
{
    return(_Module.RegisterServer(FALSE /* bRegTypeLib */));
}

/*

Notes:
    Removes entries from the system registry
    
*/

STDAPI
DllUnregisterServer(
    VOID
)
{
    _Module.UnregisterServer();
    return(S_OK);
}

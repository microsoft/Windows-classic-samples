// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "resource.h"
#include "SdkShvModule.h"
#include "SdkShvCF.h"
#include "new"


CSdkShvModule sdkModule;

// DLL Entry Point
HMODULE g_hModule=NULL;
LONG g_nComObjsInUse = 0;


BOOL APIENTRY DllMain(_In_ HANDLE hModule,
                      _In_ DWORD reason,
                      _In_opt_ void* /*lpReserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        // Save module handle for the registration method
		g_hModule = (HMODULE)hModule ;
    }

    return TRUE ;
}


// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    // A DLL is no longer in use when it is not managing any existing objects
    // (the reference count on all of its objects is 0). 
    if (g_nComObjsInUse == 0)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}


// Returns a class factory to create an object of the requested type
_Check_return_
STDAPI  DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE; 

    //Check if the requested COM object is implemented in this DLL
    if (rclsid == CLSID_SampleShv)
    {
        CSdkShvCF *pShaFact = new (std::nothrow) CSdkShvCF();
        if (pShaFact == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pShaFact->QueryInterface(riid , ppv);

            if (FAILED(hr))
            {
                delete pShaFact;
            }
        }
    }
    return hr;
}


// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    HRESULT hr = S_OK;

    hr = sdkModule.RegisterServer();
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    // Register the SDK SHV with SHV Host using the Registration API.
    hr = sdkModule.RegisterSdkShv();

Cleanup:
    return hr;
}


// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    HRESULT ret = S_OK;

    // Unregister the SDK SHV with SHV Host using the Registration API.
    HRESULT hr = sdkModule.UnregisterSdkShv();
    if (FAILED(hr))
    {
        ret = hr;
    }

    hr = sdkModule.UnregisterServer();
    if (FAILED(hr))
    {
        ret = hr;
    }
   
    return ret;
}


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// SdkShaInfo.cpp : Implementation of DLL Exports.

#include "Windows.h"
#include "resource.h"
#include "ComponentInfo.h"
#include "SdkShaInfoCF.h"
#include "SdkCommon.h"
#include "dllmain.h"
#include "new"
using namespace SDK_SAMPLE_COMMON;

extern LONG g_nComObjsInUse;

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
    if (rclsid == CLSID_ComponentInfo)
    {
        CSdkShaInfoCF *pShaFact = new (std::nothrow) CSdkShaInfoCF();
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
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = sdkModule.RegisterServer();

    return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = sdkModule.UnregisterServer();

	return hr;
}

// DllInstall - Adds/Removes entries to the system registry per user
//              per machine.	
STDAPI DllInstall(
    _In_ BOOL bInstall, 
    _In_z_ LPCWSTR /*pCmdLine*/)
{
    HRESULT hr = E_FAIL;
    static const wchar_t userSwitch[] = L"user";

    if (bInstall)
    {	
    	hr = DllRegisterServer();
    	if (FAILED(hr))
    	{	
    		DllUnregisterServer();
    	}
    }
    else
    {
    	hr = DllUnregisterServer();
    }

    return hr;
}



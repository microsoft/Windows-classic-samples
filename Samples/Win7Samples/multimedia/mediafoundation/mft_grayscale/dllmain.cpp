//////////////////////////////////////////////////////////////////////////
//
// dllmain.cpp : Implements DLL exports and COM class factory
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Note: This source file implements the class factory for the transform,
//       plus the following DLL functions:
//       - DllMain
//       - DllCanUnloadNow
//       - DllRegisterServer
//       - DllUnregisterServer
//       - DllGetClassObject
//
//////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include "MFT_Grayscale.h"
#include "Grayscale.h"
#include "ClassFactory.h"   // Implements IClassFactory
#include "registry.h"       // Helpers to register COM objects.

#include <initguid.h>
#include "GrayscaleGuids.h"

HMODULE g_hModule;      // DLL module handle

DEFINE_CLASSFACTORY_SERVER_LOCK;


// Table of objects for the class factory (CLSID + creation function)
ClassFactoryData g_ClassFactories[] =
{
    {   &CLSID_GrayscaleMFT, CGrayscale::CreateInstance }
};
      
DWORD g_numClassFactories = ARRAY_SIZE(g_ClassFactories);


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = (HMODULE)hModule;
        TRACE_INIT();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        TRACE_CLOSE();
        break;
    }
    return TRUE;
}

STDAPI DllCanUnloadNow()
{
    if (!ClassFactory::IsLocked())
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}


STDAPI DllRegisterServer()
{
    assert(g_hModule != NULL); 

    HRESULT hr = S_OK;

    // Register the CLSID for CoCreateInstance.
    hr = RegisterObject(g_hModule, CLSID_GrayscaleMFT, TEXT("Grayscale Video Effect"), TEXT("Both"));

    // Register the MFT for MFT enumeration.
    if (SUCCEEDED(hr))
    {
        hr = MFTRegister(
            CLSID_GrayscaleMFT,         // CLSID
            MFT_CATEGORY_VIDEO_EFFECT,  // Category
            L"Grayscale Video Effect",  // Friendly name
            0,                          // Reserved, must be zero.
            0,
            NULL,
            0,
            NULL,
            NULL
            );
    }
    return hr;
}

STDAPI DllUnregisterServer()
{
    // Unregister the MFT.
    MFTUnregister(CLSID_GrayscaleMFT);

    // Unregister the CLSID.
    UnregisterObject(CLSID_GrayscaleMFT);

    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv)
{
    ClassFactory *pFactory = NULL;

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE; // Default to failure

    // Find an entry in our look-up table for the specified CLSID.
    for (DWORD index = 0; index < g_numClassFactories; index++)
    {
        if (*g_ClassFactories[index].pclsid == clsid)
        {
            // Found an entry. Create a new class factory object.
            pFactory = new ClassFactory(g_ClassFactories[index].pfnCreate);
            if (pFactory)
            {
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = pFactory->QueryInterface(riid, ppv);
    }
    SAFE_RELEASE(pFactory);

    return hr;
}



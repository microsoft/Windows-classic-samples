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
// Note: This source file implements the class factory for the DMO,
//       plus the following DLL functions:
//       - DllMain
//       - DllCanUnloadNow
//       - DllRegisterServer
//       - DllUnregisterServer
//       - DllGetClassObject
//
//////////////////////////////////////////////////////////////////////////

#include "AudioDelayMFT.h"

#include "ClassFactory.h"   // Implements IClassFactory
#include "registry.h"       // Helpers to register COM objects.

#include <initguid.h>
#include "AudioDelayUuids.h"

HMODULE g_hModule;          // DLL module handle

DEFINE_CLASSFACTORY_SERVER_LOCK;

WCHAR* g_sFriendlyName =  L"Audio Delay MFT";

// g_ClassFactories: Array of class factory data.
// Defines a look-up table of CLSIDs and corresponding creation functions.

ClassFactoryData g_ClassFactories[] =
{
    {   &CLSID_DelayMFT, CDelayMFT::CreateInstance }
};
      
const DWORD g_numClassFactories = ARRAY_SIZE(g_ClassFactories);

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = (HMODULE)hModule;
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
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
    HRESULT hr;
    
    // Register the MFT's CLSID as a COM object.
    hr = RegisterObject(g_hModule, CLSID_DelayMFT, g_sFriendlyName, TEXT("Both"));

    // Register the MFT in the audio effects category.
    if (SUCCEEDED(hr))
    {
        MFT_REGISTER_TYPE_INFO tinfo [] =
        {
            { MFMediaType_Audio, MFAudioFormat_PCM }
        };

        hr = MFTRegister(
            CLSID_DelayMFT,             // CLSID
            MFT_CATEGORY_AUDIO_EFFECT,  // Category
            g_sFriendlyName, 
            0, // Reserved
            ARRAY_SIZE(tinfo),
            tinfo,
            ARRAY_SIZE(tinfo),
            tinfo,
            NULL
            );

    }
    return hr;
}

STDAPI DllUnregisterServer()
{
    // Unregister the CLSID
    UnregisterObject(CLSID_DelayMFT);

    // Unregister from the category.
    MFTUnregister(CLSID_DelayMFT);

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



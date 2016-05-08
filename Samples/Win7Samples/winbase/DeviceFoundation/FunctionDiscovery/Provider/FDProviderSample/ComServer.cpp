// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      Implemations of Class Factory and DLL exports for the COM Server

#include "stdafx.h"

// TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
// 
// Generate a new UUID to use here.
//
// Every provider must have its own CLSID with a unique UUID
// The provider's installer must register this UUID as the COM class 
// for the provider and reference the UUID as the class for the 
// provider when registering the provider in the registry.
//
// TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO - TODO
const GUID CLSID_FDProvider = { 0x8c19066a, 0x643a, 0x4586, { 0x92, 0xb2, 0xa7, 0x85, 0xb9, 0xd, 0x76, 0x6f }};

class TClassFactory: 
    public IClassFactory,
    public IEXEHostControl
{
public:
    // IUnknown
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(
        REFIID riid, 
        __deref_out_opt void** ppv);

    // IClassFactory
    STDMETHODIMP CreateInstance(
        __in_opt IUnknown* punkOuter, 
        REFIID iid, 
        __deref_out_opt void** ppv);

    STDMETHODIMP LockServer(
        BOOL fLock);

    // IEXEHostControl
    STDMETHODIMP RegisterProviderLifetimeNotificationCallback(
        PFNProviderLifetimeNotificationCallback pfnProviderLifetimeNotificationCallback);

    STDMETHODIMP LogoffNotification(
        DWORD hSessionId);

    // Constructor / Destuctor
    TClassFactory();
    ~TClassFactory();

protected:
    LONG m_cRef;
    PFNProviderLifetimeNotificationCallback m_pfnProviderLifetimeNotificationCallback;
}; // TClassFactory


// Global variable that support COM DLL server
LONG g_cLockCount = 0;

//---------------------------------------------------------------------------
// Begin TClassFactory implemetation
//---------------------------------------------------------------------------

TClassFactory::TClassFactory():
    m_cRef(1),
    m_pfnProviderLifetimeNotificationCallback(NULL)
{ 
    IncModuleCount(); 
}  // TClassFactory::TClassFactory

TClassFactory::~TClassFactory() 
{ 
    DecModuleCount(); 
}  // TClassFactory::~TClassFactory

STDMETHODIMP_(ULONG) TClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}  // TClassFactory::AddRef

STDMETHODIMP_(ULONG) TClassFactory::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}  // TClassFactory::Release

STDMETHODIMP TClassFactory::QueryInterface(
    REFIID riid, 
    __deref_out_opt void** ppv)
{
    HRESULT hr = S_OK;

    if (ppv)
    {
        *ppv = NULL;        
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppv = (IUnknown*)(IClassFactory*) this;
        }
        else if (IID_IClassFactory == riid)
        {
            AddRef();
            *ppv = (IClassFactory*) this;
        }
        else if (__uuidof(IEXEHostControl) == riid)
        {
            AddRef();
            *ppv = (IEXEHostControl*) this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}  // TClassFactory::QueryInterface

STDMETHODIMP TClassFactory::CreateInstance(
    __in_opt IUnknown* pUnkownOuter, 
    REFIID riid, 
    __deref_out_opt void** ppv)
{
    HRESULT    hr = S_OK;
    IUnknown*  pUnknown = NULL;

    if (ppv)
    {
        *ppv = NULL;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        if (pUnkownOuter)
        {
            hr =  CLASS_E_NOAGGREGATION;
        }
    }

    if (S_OK == hr)
    {
        pUnknown = new(std::nothrow) TFunctionDiscoveryProvider(m_pfnProviderLifetimeNotificationCallback);
        if (!pUnknown)
        {
            hr =  E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
        hr = pUnknown->QueryInterface(
            riid, 
            ppv);
    }

    if (pUnknown)
    {
        pUnknown->Release();
    }

    return hr;
}  // TClassFactory::CreateInstance

STDMETHODIMP TClassFactory::LockServer(
    BOOL fLock)
{
    if (fLock)
    {
        IncModuleCount();
    }
    else
    {
        DecModuleCount();
    }

    return S_OK;
}  // TClassFactory::LockServer

// IEXEHostControl
STDMETHODIMP TClassFactory::RegisterProviderLifetimeNotificationCallback(
        PFNProviderLifetimeNotificationCallback pfnProviderLifetimeNotificationCallback)
{
    m_pfnProviderLifetimeNotificationCallback = pfnProviderLifetimeNotificationCallback;

    return S_OK;
} // TClassFactory::RegisterProviderLifetimeNotificationCallback

STDMETHODIMP TClassFactory::LogoffNotification(
    DWORD hSessionId)
{
    TFunctionDiscoveryProvider::LogoffNotification(hSessionId);

    return S_OK;
} // TClassFactory::LogoffNotification

//---------------------------------------------------------------------------
// End TClassFactory implemetation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Begin COM DLL server implementation
//---------------------------------------------------------------------------

inline VOID IncModuleCount()
{
    InterlockedIncrement(&g_cLockCount);
}  // IncModuleCount

inline VOID DecModuleCount()
{
    InterlockedDecrement(&g_cLockCount);
}  // DecModuleCount

extern "C"
{

BOOL APIENTRY DllMain(
    HMODULE hModule,
    ULONG ulReason,
    __in_opt PVOID pReserved)
{
    BOOL fRetVal = TRUE;

    if (DLL_PROCESS_ATTACH == ulReason)
    {
        // Disable thread attach notifications
        fRetVal = DisableThreadLibraryCalls(hModule);
    }

    return fRetVal;
} // DllMain

HRESULT APIENTRY DllGetClassObject(
    __in REFCLSID clsid, 
    __in REFIID riid, 
    void** ppv)
{
    HRESULT hr = S_OK;
    TClassFactory* pClassFactory = NULL;

    if (ppv)
    {
        *ppv = NULL;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        if (CLSID_FDProvider != clsid)
        {
            hr = CLASS_E_CLASSNOTAVAILABLE;
        }
    }

    if (S_OK == hr)
    {
        pClassFactory = new(std::nothrow) TClassFactory;

        if (!pClassFactory)
        {
            hr =  E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
        hr = pClassFactory->QueryInterface(riid, ppv);
    }

    if (pClassFactory)
    {
        pClassFactory->Release();
    }

    return hr;
}  // DLLGetClassObject

HRESULT APIENTRY DllCanUnloadNow()
{
    return (g_cLockCount == 0) ? S_OK : S_FALSE;
}  // DllCanUnloadNow

} // extern "C"

//---------------------------------------------------------------------------
// End COM DLL server implementation
//---------------------------------------------------------------------------
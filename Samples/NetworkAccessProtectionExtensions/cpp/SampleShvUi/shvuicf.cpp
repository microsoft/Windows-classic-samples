// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "shvuicf.h"
#include <new>

long g_serverLock = 0; //lock count on server
long g_cObjRefCount = 0;
extern SHVUIClassFactoryData* g_FactoryData;

///////////////////////////////////////////////////////////
//
// Static variables
//
LONG ShvUIClassFactory::s_cServerLocks = 0 ;    // Count of locks
HMODULE ShvUIClassFactory::s_hModule = NULL ;   // DLL module handle

ShvUIClassFactory::ShvUIClassFactory()
    :m_cRef(1)
{
    InterlockedIncrement(&g_cObjRefCount);
}

ShvUIClassFactory::~ShvUIClassFactory()
{
    InterlockedDecrement(&g_cObjRefCount);
}
    
STDMETHODIMP_(ULONG) ShvUIClassFactory::AddRef(void)
{
    return InterlockedIncrement((PLONG)&m_cRef);
}

STDMETHODIMP_(ULONG) ShvUIClassFactory::Release(void)
{
    ULONG res = InterlockedDecrement((PLONG)&m_cRef);
    if (res == 0)
    {
       delete this;
    }
    return res;
}

STDMETHODIMP ShvUIClassFactory::QueryInterface(
    _In_ const IID& riid, 
    _Out_ void** ppvObject)
{
    if (IID_IUnknown == riid || IID_IClassFactory == riid)
    {
        *ppvObject = static_cast<IClassFactory*>(this);
    }
    else 
    {
       *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    
    reinterpret_cast<IUnknown *>(*ppvObject)->AddRef();
    
    return S_OK;
}


STDMETHODIMP 
ShvUIClassFactory::CreateInstance(
    _In_opt_ IUnknown *pUnkOuter, 
    _In_ REFIID riid, 
    _Outptr_ void **ppvObject)
{
    HRESULT hr =S_OK;
    // aggregation not supported
    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    // create the CoClass and return the interface
    ShvUI* pShvUI = NULL;
    pShvUI = new (std::nothrow) ShvUI();

    if (pShvUI == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = pShvUI->QueryInterface(riid, ppvObject);

    if (FAILED(hr))
    {
        delete pShvUI;
    }

    return hr;
    
}

STDMETHODIMP 
ShvUIClassFactory::LockServer(
    __RPC__in BOOL fLock)
{
    if (fLock)
    {
        InterlockedIncrement(&g_serverLock);
    }
    else
    {
        InterlockedDecrement(&g_serverLock);
    }

    return S_OK;    
}



//Out of proc server support

//
// Start the factory
//
BOOL ShvUIClassFactory::StartFactory()
{
    SHVUIClassFactoryData* pFactoryData = g_FactoryData;

    // Create the class factory for this component.
    IClassFactory* pIFactory = new (std::nothrow) ShvUIClassFactory() ;
    if (NULL == pIFactory)
    {
        return FALSE;
    }

    // Register the class factory.
    DWORD doRegister;
    HRESULT hr = ::CoRegisterClassObject(
                      pFactoryData->m_pCLSID,
                      static_cast<IUnknown*>(pIFactory),
                      CLSCTX_LOCAL_SERVER,
                      REGCLS_MULTIPLEUSE,
                      &doRegister) ;
    if (FAILED(hr))
    {
        pIFactory->Release() ;
        return FALSE ;
    }
    pFactoryData->m_pIClassFactory = pIFactory;
    pFactoryData->m_register = doRegister;

    return TRUE ;
}

//
// Stop the factory
//
void ShvUIClassFactory::StopFactory()
{
    SHVUIClassFactoryData* pFactoryData = g_FactoryData;

    DWORD doRegister = pFactoryData->m_register;
    if (doRegister != 0)
    {
        ::CoRevokeClassObject(doRegister);
    }
    IClassFactory* pIFactory = pFactoryData->m_pIClassFactory;

    if (pIFactory != NULL)
    {
        pIFactory->Release();
    }
}


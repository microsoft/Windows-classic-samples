// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "precomp.h"
#include "shvuicf.h"
#include "registration.h"

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

STDMETHODIMP ShvUIClassFactory::QueryInterface(REFIID riid, void** ppvObject)
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
    IUnknown *pUnkOuter, 
    REFIID riid, 
    void **ppvObject)
{
    HRESULT hr =S_OK;
    // aggregation not supported
    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    // create the CoClass and return the interface
    ShvUI* pShvUI = NULL;
    pShvUI = new ShvUI();

    if (pShvUI == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pShvUI->AddRef();

    hr = pShvUI->QueryInterface(riid, ppvObject);

    // if QI failed Release will remove the object
    pShvUI->Release();

    return hr;
    
}

STDMETHODIMP 
ShvUIClassFactory::LockServer(
    BOOL fLock)
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
    IClassFactory* pIFactory = new ShvUIClassFactory() ;


	// Register the class factory.
    DWORD dwRegister ;

    HRESULT hr = ::CoRegisterClassObject(
	                  pFactoryData->m_pCLSID,
	                  static_cast<IUnknown*>(pIFactory),
	                  CLSCTX_LOCAL_SERVER,
	                  REGCLS_MULTIPLEUSE,
	                  &dwRegister) ;
    if (FAILED(hr))
    {
        pIFactory->Release() ;
        return FALSE ;
    }
    pFactoryData->m_pIClassFactory = pIFactory;
    pFactoryData->m_dwRegister = dwRegister;

    return TRUE ;
}

//
// Stop the factory
//
void ShvUIClassFactory::StopFactory()
{
    SHVUIClassFactoryData* pFactoryData = g_FactoryData;

    DWORD dwRegister = pFactoryData->m_dwRegister;
    if (dwRegister != 0)
    {
        ::CoRevokeClassObject(dwRegister);
    }
    IClassFactory* pIFactory = pFactoryData->m_pIClassFactory;

    if (pIFactory != NULL)
    {
        pIFactory->Release();
    }
}

HRESULT ShvUIClassFactory::Register()
{
	ShvUIRegisterServer() ;
	return S_OK ;
}   
	
HRESULT ShvUIClassFactory::Unregister()
{
	ShvUIUnRegisterServer() ;
	return S_OK ;
}

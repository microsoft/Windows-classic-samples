// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "SdkShaInfoCF.h"
#include "ComponentInfo.h"
#include "new"

extern LONG g_nComObjsInUse;

CSdkShaInfoCF::CSdkShaInfoCF() :
m_nRefCount(0)
{   
    InterlockedIncrement(&g_nComObjsInUse) ;
}

CSdkShaInfoCF::~CSdkShaInfoCF()
{
    InterlockedDecrement(&g_nComObjsInUse);
}


IFACEMETHODIMP CSdkShaInfoCF::CreateInstance(
    /* [in] */ __RPC__in_opt IUnknown *pUnkOuter,
    /* [in] */ __RPC__in REFIID riid,
    /* [out] */ __RPC__deref_out void **ppvObject)
{
    HRESULT hr;

    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    CComponentInfo* pObject = new (std::nothrow) CComponentInfo();
    if (pObject == NULL)
    {
        return E_OUTOFMEMORY ;
    }

    hr = pObject->QueryInterface(riid, ppvObject);

    if (FAILED(hr))
    {
        delete pObject;
    }

    return hr;
 }


IFACEMETHODIMP CSdkShaInfoCF::LockServer(
    /* [in] */ BOOL /*fLock*/)
{
    return E_NOTIMPL;
}


IFACEMETHODIMP CSdkShaInfoCF::QueryInterface(
            /* [in] */ __RPC__in REFIID riid , 
            /* [out] */ __RPC__deref_out void **ppObj)
{
    if (riid == IID_IUnknown)
    {
        *ppObj = static_cast<IUnknown*>(this); 
        AddRef() ;
        return S_OK;
    }

    if (riid == IID_IClassFactory)
    {
        *ppObj = static_cast<IClassFactory*>(this);
        AddRef() ;
        return S_OK;
    }

    *ppObj = NULL ;
    return E_NOINTERFACE ;
}

IFACEMETHODIMP_(ULONG) CSdkShaInfoCF::AddRef()
{
    return InterlockedIncrement(&m_nRefCount) ;
}

IFACEMETHODIMP_(ULONG) CSdkShaInfoCF::Release()
{     
    LONG nRefCount = InterlockedDecrement(&m_nRefCount) ;
    if (nRefCount == 0)
    {
        delete this;
    }

    return nRefCount;
}




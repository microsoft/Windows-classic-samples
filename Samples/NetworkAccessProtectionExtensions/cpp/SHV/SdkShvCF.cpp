// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "SdkShvCF.h"
#include "SampleShv.h"
#include "new"

extern LONG g_nComObjsInUse;

CSdkShvCF::CSdkShvCF() :
m_nRefCount(0)
{   
    InterlockedIncrement(&g_nComObjsInUse) ;
}

CSdkShvCF::~CSdkShvCF()
{
    InterlockedDecrement(&g_nComObjsInUse);
}


IFACEMETHODIMP CSdkShvCF::CreateInstance(
    /* [in] */ __RPC__in_opt IUnknown *pUnkOuter,
    /* [in] */ __RPC__in REFIID riid,
    /* [out] */ __RPC__deref_out void **ppvObject)
{
    HRESULT hr;

    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    SDK_SAMPLE_SHV::CSampleShv* pObject = new (std::nothrow) SDK_SAMPLE_SHV::CSampleShv();
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


IFACEMETHODIMP CSdkShvCF::LockServer(
    /* [in] */ BOOL /*fLock*/)
{
    return E_NOTIMPL;
}


IFACEMETHODIMP CSdkShvCF::QueryInterface(
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

IFACEMETHODIMP_(ULONG) CSdkShvCF::AddRef()
{
    return InterlockedIncrement(&m_nRefCount) ;
}

IFACEMETHODIMP_(ULONG) CSdkShvCF::Release()
{     
    LONG nRefCount = InterlockedDecrement(&m_nRefCount) ;
    if (nRefCount == 0)
    {
        delete this;
    }

    return nRefCount;
}




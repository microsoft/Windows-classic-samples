////////////////////////////////////////////////////////////////////////

//

//	Factory.cpp

//

//	Module:	WMI high performance provider sample code

//

//	This is a standard class factory implementation for the CHiPerfProvider

//	object.  

//
// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include "BasicHiPerf.h"
#include "Factory.h"

#define _UNICODE
#define UNICODE

extern long g_lObjects;
extern long g_lLocks;

//////////////////////////////////////////////////////////////
//
//
//	CClassFactory
//
//
//////////////////////////////////////////////////////////////

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppv)
//////////////////////////////////////////////////////////////
//
//	Standard QueryInterface
//
//	Parameters:
//		riid	- the ID of the requested interface
//		ppv		- a pointer to the interface pointer
//
//////////////////////////////////////////////////////////////
{
    if(riid == IID_IUnknown)
        *ppv = (LPVOID)(IUnknown*)this;
    else if(riid == IID_IClassFactory)
        *ppv = (LPVOID)(IClassFactory*)this;
	else return E_NOINTERFACE;

	((IUnknown*)*ppv)->AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
//////////////////////////////////////////////////////////////
//
//	Standard COM AddRef
//
//////////////////////////////////////////////////////////////
{
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
//////////////////////////////////////////////////////////////
//
//	Standard COM Release
//
//////////////////////////////////////////////////////////////
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;

    return lRef;
}

STDMETHODIMP CClassFactory::CreateInstance(
		/* [in] */ IUnknown* pUnknownOuter, 
		/* [in] */ REFIID iid, 
		/* [out] */ LPVOID *ppv)
//////////////////////////////////////////////////////////////
//
//	Standard COM CreateInstance
//
//////////////////////////////////////////////////////////////
{
	HRESULT hRes;
	CHiPerfProvider *pProvider = NULL;

	*ppv = NULL;

	// We do not support aggregation
	// =============================

	if (pUnknownOuter)
		return CLASS_E_NOAGGREGATION;

	// Create the provider object
	// ==========================

	pProvider = new CHiPerfProvider;

	if (!pProvider)
		return E_OUTOFMEMORY;

	// Retrieve the requested interface
	// ================================

	hRes = pProvider->QueryInterface(iid, ppv);
	if (FAILED(hRes))
	{
		delete pProvider;
		return hRes;
	}

	return S_OK;
}

STDMETHODIMP CClassFactory::LockServer(
		/* [in] */ BOOL bLock)
//////////////////////////////////////////////////////////////
//
//	Standard COM LockServer
//
//////////////////////////////////////////////////////////////
{
	if (bLock)
		InterlockedIncrement(&g_lLocks);
	else
		InterlockedDecrement(&g_lLocks);

	return S_OK;
}

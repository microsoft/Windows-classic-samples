// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  Provider.cpp
//
// Description: Event consumer provider class implementation
//    
//
// History:
//
// **************************************************************************

#include "stdafx.h"
#include "Provider.h"
#include "Consumer.h"
#include <objbase.h>

CProvider::CProvider(CListBox *pOutputList)
{
	m_cRef = 0L;
	m_pOutputList = pOutputList;
}

CProvider::~CProvider()
{
}

STDMETHODIMP CProvider::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv=NULL;

    if (riid == IID_IUnknown || riid == IID_IWbemEventConsumerProvider)
        *ppv=this;

    if (*ppv != NULL)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CProvider::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CProvider::Release(void)
{
    if (--m_cRef != 0L)
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP CProvider::Initialize(LPWSTR wszUser, LONG lFlags,
								   LPWSTR wszNamespace, LPWSTR wszLocale,
								   IWbemServices __RPC_FAR *pNamespace,
								   IWbemContext __RPC_FAR *pCtx,
								   IWbemProviderInitSink __RPC_FAR *pInitSink)
{

    // Tell CIMOM that we are initialized
    pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
    return WBEM_S_NO_ERROR;
}

//-----------------------------------------------------------
STDMETHODIMP CProvider::FindConsumer(
						IWbemClassObject* pLogicalConsumer,
						IWbemUnboundObjectSink** ppConsumer)
{
	CConsumer* pSink = new CConsumer(m_pOutputList);
	if (pSink)
	{
		return pSink->QueryInterface(IID_IWbemUnboundObjectSink, 
                                        (void**)ppConsumer);
	}
	 return WBEM_E_FAILED;
}


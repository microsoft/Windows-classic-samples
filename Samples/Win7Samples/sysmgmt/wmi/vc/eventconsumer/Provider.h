// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  Provider.cpp
//
// Description: Event consumer provider class definition
//    
//
// History:
//
// **************************************************************************

#include <wbemcli.h>
#include <wbemprov.h>

class CProvider : public IWbemEventConsumerProvider
{
public:
	CProvider(CListBox	*pOutputList);
	~CProvider();

    // IUnknown members
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

	STDMETHOD(Initialize)( 
			LPWSTR pszUser,
			LONG lFlags,
			LPWSTR pszNamespace,
			LPWSTR pszLocale,
			IWbemServices __RPC_FAR *pNamespace,
			IWbemContext __RPC_FAR *pCtx,
			IWbemProviderInitSink __RPC_FAR *pInitSink);

    STDMETHOD(FindConsumer)(
			IWbemClassObject* pLogicalConsumer,
			IWbemUnboundObjectSink** ppConsumer);

private:

	DWORD m_cRef;
	CListBox	*m_pOutputList;
};

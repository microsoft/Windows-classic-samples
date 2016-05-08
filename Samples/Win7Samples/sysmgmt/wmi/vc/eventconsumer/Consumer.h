// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  Consumer.h
//
// Description: Event consumer class definition
//    
//
// History:
//
// **************************************************************************

#include <wbemcli.h>
#include <wbemprov.h>

class CConsumer : public IWbemUnboundObjectSink
{
public:
	CConsumer(CListBox      *pOutputList);
	~CConsumer();

    // IUnknown members
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHOD(IndicateToConsumer)(IWbemClassObject *pLogicalConsumer,
									long lNumObjects,
									IWbemClassObject **ppObjects);

private:

	DWORD m_cRef;
	LPCTSTR ErrorString(HRESULT hRes);
	LPWSTR ValueToString(VARIANT *pValue, WCHAR **pbuf);
	CListBox        *m_pOutputList;
};

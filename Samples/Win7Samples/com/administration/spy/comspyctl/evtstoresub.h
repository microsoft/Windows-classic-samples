// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __EVTSTORESUB_H__
#define __EVTSTORESUB_H__

#include "resource.h"       // main symbols
#include <eventsys.h>		// Definitions for the COM+ Event system

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CEvtStoreSub
Summary: Subscriber for the event published by the event system
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CEvtStoreSub : 
	public CSysLCESub,
	public IEventObjectChange
{
public:
	CEvtStoreSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}

	CEvtStoreSub()
	{
		m_pSpy = NULL;
	}

DECLARE_NOT_AGGREGATABLE(CEvtStoreSub)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CEvtStoreSub)
	COM_INTERFACE_ENTRY(IEventObjectChange)
	COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
END_COM_MAP()

// CSysLCESub - virtual methods defined
    virtual EventEnum EventType() { return EventStore; }
	virtual REFCLSID EventCLSID() { return CLSID_EventObjectChange; }
    virtual REFIID EventIID() { return IID_IEventObjectChange; }

// IEventObjectChange Methods
STDMETHOD(ChangedSubscription)
   (/* [in] */ EOC_ChangeType changeType,
	/* [in] */ BSTR bstrSubscriptionID);

STDMETHOD(ChangedEventClass)
   (/* [in] */ EOC_ChangeType changeType,
	/* [in] */ BSTR bstrEventClassID);

STDMETHOD(ChangedPublisher)
   (/* [in] */ EOC_ChangeType changeType,
	/* [in] */ BSTR bstrPublisherID);

// CEvtStoreSub Methods
STDMETHOD (AddSubscriptionInfo)
   (/* [in] */ CComSpy* pSpy,
	/* [in] */ BSTR bstrSubscriptionID);

STDMETHOD (AddEventClassInfo)
   (/* [in] */ CComSpy* pSpy,
	/* [in] */ BSTR bstrEventClassID);
};

#endif //__EVTSTORESUB_H__

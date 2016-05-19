// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __TXSUB_H_
#define __TXSUB_H_

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CTxSub
Summary: Transaction Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CTxSub : 
	public CSysLCESub,
	public IComTransactionEvents
{
public:

	CTxSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}
	CTxSub()
	{
		m_pSpy = NULL;
	}

	DECLARE_NOT_AGGREGATABLE(CTxSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CTxSub)
		COM_INTERFACE_ENTRY(IComTransactionEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return Transaction; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComTransactionEvents; }

public:
	STDMETHOD (OnTransactionStart)(COMSVCSEVENTINFO * pInfo, REFGUID guidTx, REFGUID tsid, BOOL bRoot);
	STDMETHOD (OnTransactionPrepare)(COMSVCSEVENTINFO * pInfo, REFGUID guidTx, BOOL bVoteYes);
	STDMETHOD (OnTransactionAbort)(COMSVCSEVENTINFO * pInfo, REFGUID guidTx);
	STDMETHOD (OnTransactionCommit)(COMSVCSEVENTINFO * pInfo, REFGUID guidTx);


};

#endif //__TXSUB_H_

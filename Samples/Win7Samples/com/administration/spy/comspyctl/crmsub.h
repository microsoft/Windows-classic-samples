// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __CRMSUB_H__
#define __CRMSUB_H__

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CCRMSub
Summary: CRM Event Subscriber
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CCRMSub : 
	public CSysLCESub,
	public IComCRMEvents
{
public:

	CCRMSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}
	CCRMSub()
	{
		m_pSpy = NULL;
	}

DECLARE_NOT_AGGREGATABLE(CCRMSub)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CCRMSub)
	COM_INTERFACE_ENTRY(IComCRMEvents)
	COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
END_COM_MAP()

    virtual EventEnum EventType() { return CRM; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComCRMEvents; }

///// IComCRMEvents


	STDMETHOD(OnCRMRecoveryStart)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidApp);

	STDMETHOD(OnCRMRecoveryDone)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidApp);

	STDMETHOD(OnCRMCheckpoint)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidApp);

	STDMETHOD(OnCRMBegin)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID,
				GUID guidActivity,
				GUID guidTx,
				WCHAR szProgIdCompensator[64],			// NOTE: hardcoded
				WCHAR szDescription[64]);				// NOTE: hardcoded

	STDMETHOD(OnCRMPrepare)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID);

	STDMETHOD(OnCRMCommit)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID);

	STDMETHOD(OnCRMAbort)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID);

	STDMETHOD(OnCRMIndoubt)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID);

	STDMETHOD(OnCRMDone)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID);

	STDMETHOD(OnCRMRelease)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID);

	STDMETHOD(OnCRMAnalyze)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID,
				DWORD dwCrmRecordType,
				DWORD dwRecordSize);

	STDMETHOD(OnCRMWrite)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID,
				BOOL bVariants,
				DWORD dwRecordSize);

	STDMETHOD(OnCRMForget)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID);

	STDMETHOD(OnCRMForce)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID);

	STDMETHOD(OnCRMDeliver)(
				COMSVCSEVENTINFO * pInfo,
				GUID guidClerkCLSID,
				BOOL bVariants,
				DWORD dwRecordSize);

};
#endif //__CRMSUB_H__



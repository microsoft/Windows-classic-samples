// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __RESOURCESUB_H_
#define __RESOURCESUB_H_

#include "resource.h"       // main symbols

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CResourceSub
Summary: Resource Events Subscriber 
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class ATL_NO_VTABLE CResourceSub : 
	public CSysLCESub, 
	public IComResourceEvents

{
public:
	CResourceSub(CComSpy * pSpy)
	{
		m_pSpy = pSpy;
	}
	CResourceSub()
	{
		m_pSpy = NULL;
	}

	DECLARE_NOT_AGGREGATABLE(CResourceSub)
	DECLARE_GET_CONTROLLING_UNKNOWN()

	BEGIN_COM_MAP(CResourceSub)
		COM_INTERFACE_ENTRY(IComResourceEvents)
		COM_INTERFACE_ENTRY_CHAIN(CSysLCESub)
	END_COM_MAP()

    virtual EventEnum EventType() { return Resource; }
	virtual REFCLSID EventCLSID() { return CLSID_ComServiceEvents; }
    virtual REFIID EventIID() { return IID_IComResourceEvents; }

// IResourceSub
public:
	STDMETHOD(OnResourceCreate)(COMSVCSEVENTINFO * pInfo, ULONG64 ObjectID, LPCWSTR pwszType,ULONG64 resId, BOOL enlisted);
	STDMETHOD(OnResourceAllocate)(COMSVCSEVENTINFO * pInfo, ULONG64 ObjectID, LPCWSTR pwszType,ULONG64 resId, BOOL enlisted, DWORD NumRated, DWORD Rating);	
	STDMETHOD(OnResourceRecycle)(COMSVCSEVENTINFO * pInfo, ULONG64 ObjectID, LPCWSTR pwszType,ULONG64 resId);
	STDMETHOD(OnResourceDestroy)(COMSVCSEVENTINFO * pInfo, ULONG64 ObjectID, HRESULT hr, LPCWSTR pwszType,ULONG64 resId);
	STDMETHOD(OnResourceTrack)(COMSVCSEVENTINFO * pInfo, ULONG64 ObjectID, LPCWSTR pwszType,ULONG64 resId, BOOL enlisted);
};

#endif //__RESOURCESUB_H_

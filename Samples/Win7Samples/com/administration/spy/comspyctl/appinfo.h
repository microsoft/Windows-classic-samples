// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __APP_INFO_H__
#define __APP_INFO_H__

#include "ComSpyCtl.h"

class CComSpy;

typedef enum FilterType {NoFilter = 0, AppID = 1, ProcessID = 2} FILTERTYPE;


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
Class:   CAppInfo
Summary: CAppInfo manages COM+ Event subscriptions for COM+ Applications.
		 Depending on the flag settings, this can be a COM+ Catalog App,
		 a running COM+ process, or a special case that represents 'all'
		 Applications (does not filter by App).		 
		 The COM+ Events basically are filtered by AppID or PID (or both) for
		 a given Event Type.  This SDK example does not support the case
		 of filtering by both an AppID and a PID at the same time.	   		   
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/ 
class CAppInfo
{
private:
	CComSpy * m_pSpy;
	bool m_bReadyForDelete;
	SUBSCRIBERMAP * m_EventMap;		
	CComBSTR m_sAppName;		
	FILTERTYPE m_FilterType;
	CComBSTR m_sAppID;
	long m_PID;
	
	// Cannot call Default Constrcutor or Initialze since they are Private
	CAppInfo();	
	void Initialze(CComSpy * pSpy);

public:
	CAppInfo(LPCWSTR pwszAppName, CComSpy * pSpy);
	CAppInfo(LPCWSTR pwszAppName, CComSpy * pSpy, long nPID);
	CAppInfo(LPCWSTR pwszAppName, CComSpy * pSpy, LPCWSTR pwszAppID);

	~CAppInfo();

	CComBSTR & AppName() {return m_sAppName;}
	FILTERTYPE GetFilterType() {return m_FilterType;}
	SUBSCRIBERMAP* GetEvents() {return m_EventMap;}

	void SetDeleteFlag(bool bDelete) { m_bReadyForDelete = bDelete; }
	bool IsReadyForDelete() { return m_bReadyForDelete; }

	// Subscription helpers.
	bool RemoveAllSubscriptions();
	bool AddSubscription(EventEnum e);
	bool RemoveSubscription(EventEnum e);

	bool IsSubscribed(EventEnum e);
	bool IsSubscribedToAny();
};

#endif


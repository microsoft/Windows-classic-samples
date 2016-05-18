// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
// Module Name:
//    WcnApiHlpr.h
//
// Abstract:
//	This file includes all the necessary declarations for implementing the 
//  IFunctionDiscoveryNotification interface
// 

#pragma once

#include <atlbase.h>
#include <atlcom.h>
#include <wlanapi.h>

//Function Discovery
#include <FunctionDiscoveryApi.h>
#include <FunctionDiscovery.h>
#include <FunctionDiscoveryKeys.h>

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>
#include <objbase.h>

/// STL
#include <vector>
#include <list>

/// WCN
#include <initguid.h>
#include "WcnApi.h"




class CWcnFdDiscoveryNotify:
    public CComObjectRootEx<CComObjectThreadModel>,
    public IFunctionDiscoveryNotification
{
   public:

      BEGIN_COM_MAP(CWcnFdDiscoveryNotify)
         COM_INTERFACE_ENTRY(IFunctionDiscoveryNotification)
      END_COM_MAP()

    CWcnFdDiscoveryNotify();
    ~CWcnFdDiscoveryNotify();

    HRESULT CWcnFdDiscoveryNotify::Init(__in BOOL bTurnOnSoftAP);
    HRESULT WcnFDSearchStart(__in UUID* pUUID, __in PWSTR pSearchSSID);
    BOOL WaitForAnyDiscoveryEvent(DWORD Timeout_ms);

	BOOL GetWCNDeviceInstance( __deref_out_opt IWCNDevice** ppWcnDevice);

	//
	// IFunctionDiscoveryNotification
	//

    HRESULT STDMETHODCALLTYPE OnUpdate(
            __in      QueryUpdateAction          enumQueryUpdateAction,
            __in      FDQUERYCONTEXT             fdqcQueryContext,
            __in      IFunctionInstance         *pIFunctionInstance);

    HRESULT STDMETHODCALLTYPE OnError(
            __in      HRESULT                    hrFD,
            __in      FDQUERYCONTEXT             fdqcQueryContext,
            __in      const WCHAR               *pszProvider);

    HRESULT STDMETHODCALLTYPE OnEvent(
            __in      DWORD                      dwEventID,
            __in      FDQUERYCONTEXT             fdqcQueryContext,
            __in      const WCHAR               *pszProvider);

    private:


	   CComPtr<IFunctionDiscovery> m_pFunctionDiscovery;
       CComPtr<IFunctionInstanceCollectionQuery> m_pFunctionInstanceCollectionQuery;
	   CComPtr<IFunctionInstanceCollection> m_pFiCollection;
       CComPtr<IFunctionInstance> m_pFunctionInstance;  

	   WCHAR wszUUID[64];
	   WCHAR wszSSIDInHex[2 * DOT11_SSID_MAX_LENGTH + 1]; // 2 * DOT11_MAX_SSID_LENGTH + null terminator
	   BOOL bUseSSID;
       HANDLE anySearchEvent;       // set on _any_ event (device added, removed, etc)
};


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
// Module Name:
//    WcnConnectNotify.h
//
// Abstract:
//		This file includes the necessary declarations for implementing the IWCNConnectNotify Interface
// 


#ifndef WCNCONNECTNOTIFY_H
#define WCNCONNECTNOTIFY_H

#pragma once

#include <atlbase.h>
#include <atlcom.h>

#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <strsafe.h>

/// STL
#include <vector>
#include <list>

/// WCN
#include "WcnApi.h"


class
WcnConnectNotification :
   public CComObjectRootEx<CComMultiThreadModel>,
   public IWCNConnectNotify
{
   public:

      static const UINT CONNECT_TIME_OUT = 180000;      

      BEGIN_COM_MAP(WcnConnectNotification)
         COM_INTERFACE_ENTRY(IWCNConnectNotify)
      END_COM_MAP()

      WcnConnectNotification();
      ~WcnConnectNotification();

	  HRESULT Init();

	  //IWcnConnectNotify 
      HRESULT STDMETHODCALLTYPE ConnectSucceeded();
      HRESULT STDMETHODCALLTYPE ConnectFailed(HRESULT hrFailure);

      HRESULT WaitForConnectionResult();

      HANDLE connectEndEvent;

      BOOL connectSucceededCallBackInvoked;
      BOOL connectFailedCallBackInvoked;
};


#endif

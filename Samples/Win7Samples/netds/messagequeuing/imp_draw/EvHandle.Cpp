// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------
//
// MQEventHandler.cpp : implementation file
//

#include "stdafx.h"
#include <afxdisp.h>
#include "Imp_Draw.h"
#include "Imp_Dlg.h"
#import <mqoa.tlb> no_namespace

// This is actually included in Imp_DrawDlg.h, but left here for reference purposes.
//#include "MQEventHandler.h"


// NOTE: DISP_FUNCTION_ID is a slighly unusual dispatch map macro, chosen because we
// neeed to specify the DispID of the functions we implement as well as the information
// normally given to the DISP_FUNCTION macro.

BEGIN_DISPATCH_MAP(CMSMQEventHandler, CCmdTarget)
	DISP_FUNCTION_ID(CMSMQEventHandler, "Arrived", 0, Arrived, VT_EMPTY, VTS_DISPATCH VTS_I4)
	DISP_FUNCTION_ID(CMSMQEventHandler, "ArrivedError", 1, ArrivedError, VT_EMPTY, VTS_DISPATCH VTS_I4 VTS_I4)
END_DISPATCH_MAP()


CMSMQEventHandler::CMSMQEventHandler()
{
	EnableAutomation();
	m_dwMyCookie = 0;
	m_pMyCP = NULL;
}

CMSMQEventHandler::~CMSMQEventHandler()
{
	if (m_pMyCP)	// If I'm currently connected to a connection point
	{
		m_pMyCP->Unadvise(m_dwMyCookie);	// Disconnect, giving it the identifier it gave me earlier,
		m_pMyCP->Release();					// and release it.
	};
	
}
//=--------------------------------------------------------------------------=
// CMSMQEventHandler::Arrived
// CMSMQEventHandler::ArrivedError
//=--------------------------------------------------------------------------=
// Purpose: These functions forward an "Arrived" or "ArrivedError" event from 
//			the queue to the event handler in the dialog code.
//  
//
// Parameters:
//    IDispatch*		-	The queue sending the event
//	  lCursor			-	Cursor value sent with the event.  Unused in
//							this function.
//
// Output:
//    none
//
// Notes:
//		These functions act nearly identically.  The only difference is if
//		a normal event arrives, S_OK is sent as the error code; if an
//		error event arrives, that error is passed on to the dialog's event
//		handler.
// 

void CMSMQEventHandler::Arrived(IDispatch* pdispQueue, long lCursor)
{
	CImp_DrawDlg* pDlg = (CImp_DrawDlg*) AfxGetApp()->m_pMainWnd;
	pDlg->Arrived(pdispQueue, S_OK, lCursor);
}

void CMSMQEventHandler::ArrivedError(IDispatch* pdispQueue, long lErrorCode, long lCursor)
{
	CImp_DrawDlg* pDlg = (CImp_DrawDlg*) AfxGetApp()->m_pMainWnd;
	pDlg->Arrived(pdispQueue, lErrorCode, lCursor);
}


//=--------------------------------------------------------------------------=
// CMSMQEventHandler::AdviseSource
//=--------------------------------------------------------------------------=
// Purpose: This function connects this class to an MSMQ connection point,
//			given an interface pointer to an MSMQ Event object.
//  
//
// Parameters:
//    IDispatch*	-	An MSMQ Event object to connect to
//
// Output:
//    HRESULT		-	S_OK if successful, otherwise any possible result from
//						QueryInterface, FindConnectionPoint, or Advise.
//
// Notes:
//		On the call to Advise, the GetIDispatch function (an MFC function) is 
//		called to retrieve the IDispatch pointer of this class in order to give
//		it to the connection point.  The cookie returned by Advise is stored
//		in a member variable, m_dwCookie.  The cookie is used when disconnecting
//		from the connection point, in the destructor.
// 

HRESULT CMSMQEventHandler::AdviseSource(IDispatch* pDispatch)
{
	LPCONNECTIONPOINTCONTAINER	pCPCont = NULL;
	HRESULT						hr;

	hr = pDispatch->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPCont);
	if (FAILED(hr))
		return hr;

	hr = pCPCont->FindConnectionPoint(__uuidof(_DMSMQEventEvents), &m_pMyCP);
	pCPCont->Release();
	if (FAILED(hr))
		return hr;

	return m_pMyCP->Advise(GetIDispatch(TRUE), &m_dwMyCookie);
};


//=--------------------------------------------------------------------------=
// CMSMQEventHandler::GetInterfaceHook
//=--------------------------------------------------------------------------=
// Purpose: This function is called by MFC's implementation of QueryInterface
//			and allows us to return our IDispatch interface in response to
//			DIID__DMSMQEventEvents
//  
//
// Parameters:
//    void*			-	IID that was passed to QueryInterface
//
// Output:
//    LPUNKNOWN		-	Our IDispatch if the passed iid is DIID__DMSMQEventEvents,
//						otherwise NULL.
//
// Notes:
//		CCmdTarget allows a derived class some control on the behavior of
//		QueryInterface. It calls this virtual function before it starts to
//		search in its map for the requested interface. This allows us to respond
//		with our IDispatch interface when requested for the sink interface
//		DIID__DMSMQEventEvents.
//		For other iid's we return NULL. This tells CCmdTarget to continue
//		the search for the interface in its maps
// 

LPUNKNOWN CMSMQEventHandler::GetInterfaceHook(const void* piid)
{
	if (IsEqualIID(__uuidof(_DMSMQEventEvents), *(IID*)piid))
	{
		return GetIDispatch(FALSE);
	}
	else
	{
		return NULL;
	}
}

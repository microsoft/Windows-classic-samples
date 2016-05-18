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
// MQEventHandler.h : header file
// 


class CMSMQEventHandler : public CCmdTarget
{
public:

	CMSMQEventHandler();
	~CMSMQEventHandler();


	virtual void Arrived(	/*[in]*/ IDispatch* pdispQueue, 
							/*[in]*/ long lCursor);
	virtual void ArrivedError(	/*[in]*/ IDispatch* pdispQueue, 
								/*[in]*/ long lErrorCode, 
								/*[in]*/ long lCursor);

	HRESULT AdviseSource(IDispatch* pDispatch);

	LPUNKNOWN GetInterfaceHook(const void* piid);

	DECLARE_DISPATCH_MAP()

private:
	DWORD				m_dwMyCookie;
	LPCONNECTIONPOINT	m_pMyCP;

};


						
					

	
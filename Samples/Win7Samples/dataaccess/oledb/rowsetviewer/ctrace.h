//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CTRACE.H
//
//-----------------------------------------------------------------------------------

#ifndef _CTRACE_H_
#define _CTRACE_H_
										   

/////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////
#define TRACE_ADDREF							CIntTrace::TraceAddRef
#define TRACE_QI								CIntTrace::TraceQI
#define TRACE_METHOD							CIntTrace::TraceMethod
#define TRACE_NOTIFICATION						CIntTrace::TraceNotification
#define TRACE_RELEASE_(pv, name, ulExpectedRef)	CIntTrace::TraceRelease((IUnknown**)&pv, name, ulExpectedRef);
#define TRACE_RELEASE(pv, name)					TRACE_RELEASE_(pv, name, 1)



/////////////////////////////////////////////////////////////////////
// CIntTrace
//
/////////////////////////////////////////////////////////////////////
class CIntTrace : public CRichEditLite
{
public:
	//constructors
	CIntTrace(CMainWindow* pCMainWindow);
	virtual ~CIntTrace();

	//messages
	virtual BOOL	OnRButtonDown(WPARAM fwKeys, REFPOINTS pts);
	virtual BOOL	OnContextMenu(HWND hWnd, REFPOINTS pts);

	virtual BOOL	OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL	OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags);
	virtual BOOL	OnUpdateOutputSelected();
			 
	//ListBox (Notifcations)
	virtual HRESULT Clear();
	virtual void	OutputTextFmt(DWORD dwMask, COLORREF dwColor, WCHAR* pwszFmt, ...);
	virtual void	OutputText(DWORD dwMask, COLORREF dwColor, WCHAR* pwszText);
	virtual void	OutputLineEnd();
	virtual void	OutputIndent(ULONG cIndentLevel);
	
	//Static functions, so everyone has access to tracing support...
	static  CIntTrace*	GetTrace();
	static  ULONG	TraceAddRef(IUnknown* pIUnknown, WCHAR* pwszText);
	static	ULONG	TraceRelease(IUnknown** ppIUnknown, WCHAR* pwszText, ULONG ulExpectedRefCount = 1);
	static  HRESULT TraceQI(IUnknown* pIUnknown, REFIID riid, IUnknown** ppIUnknown, WCHAR* pwszFmt = L"IUnknown");
	static	HRESULT TraceMethod(HRESULT hrActual, WCHAR* pwszFmt, ...);
	static	HRESULT TraceNotification(DWORD dwNotifyType, HRESULT hrVeto, WCHAR* pwszInterface, WCHAR* pwszMethod, WCHAR* pwszParams, ...);


	//Interface
	virtual COptionsSheet*	GetOptions();

	//Data
	CMainWindow*						m_pCMainWindow;
	ULONG								m_ulNestingLevel;

};



/////////////////////////////////////////////////////////////////////
// CMDITrace
//
/////////////////////////////////////////////////////////////////////
class CMDITrace : public CMDIChildLite
{
public:
	//constructors
	CMDITrace(CMainWindow* pCMainWindow);
	virtual ~CMDITrace();

	virtual BOOL	PreCreateWindow(CREATESTRUCTW& cs);
	virtual BOOL	OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL	AutoPosition(BOOL fDefaultPosition = TRUE);

	virtual BOOL	OnDestroy();
	virtual BOOL	OnClose();
	virtual BOOL	OnInitialUpdate();
	virtual BOOL	OnMDIActivate(BOOL bActivate, HWND hWndActivate, HWND hWndDeactivate);

	//members
	virtual BOOL	UpdateControls();
	
	//Messages
	virtual BOOL	OnSize(WPARAM nType,REFPOINTS pts);
	virtual BOOL	OnSetFocus(HWND hWndPrevFocus);

	//Overloads
	virtual BOOL	OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL	OnNotify(INT idCtrl, NMHDR* pNMHDR);
	virtual BOOL	OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags);

	//Data
	CMainWindow*	m_pCMainWindow;
	CIntTrace*			m_pCTrace;
};




#endif	//_CTRACE_H_

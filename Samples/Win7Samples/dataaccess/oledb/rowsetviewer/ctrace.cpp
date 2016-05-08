//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module CTRACE.CPP
//
//-----------------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"
#include "CTrace.h"


////////////////////////////////////////////////////////////////
// CIntTrace::CIntTrace
//
/////////////////////////////////////////////////////////////////
CIntTrace::CIntTrace(CMainWindow* pCMainWindow)
{
	//Data
	ASSERT(pCMainWindow);
	m_pCMainWindow		= pCMainWindow;

	//TraceProvider
	m_ulNestingLevel			= 0;
}


////////////////////////////////////////////////////////////////
// CIntTrace::~CIntTrace
//
/////////////////////////////////////////////////////////////////
CIntTrace::~CIntTrace()
{
}


////////////////////////////////////////////////////////////////
// CIntTrace::GetOptions
//
/////////////////////////////////////////////////////////////////
COptionsSheet*	CIntTrace::GetOptions()
{
	return m_pCMainWindow->GetOptions();
}


////////////////////////////////////////////////////////////////
// CIntTrace::OnRButtonDown
//
/////////////////////////////////////////////////////////////////
BOOL CIntTrace::OnRButtonDown(WPARAM fwKeys, REFPOINTS pts)
{
	//NOTE: The right mouse button doesn't automatically activate the MDI window...
	m_pCMainWindow->MDIActivate(m_hWndParent);

	//xPos, yPos are Relative to the Client Area...
	DisplayContextMenu( 
						m_hWnd,
						IDM_NOTIFYMENU, 
						pts,
						m_pCMainWindow->GetWnd(),
						TRUE
						);
	return TRUE;
}

////////////////////////////////////////////////////////////////
// CIntTrace::OnContextMenu
//
/////////////////////////////////////////////////////////////////
BOOL CIntTrace::OnContextMenu(HWND hWnd, REFPOINTS pts)
{
	DisplayContextMenu( 
						hWnd,
						IDM_NOTIFYMENU, 
						pts,
						m_pCMainWindow->GetWnd()
						);
	return  TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CIntTrace::OnUpdateCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CIntTrace::OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags)
{
	switch(nID)
	{
		//Editing
		ON_COMMAND_UI_ENABLED(IDM_CLEARNOTIFYWINDOW,			TRUE)
		ON_COMMAND_UI_ENABLED(IDM_COPYNOTIFYWINDOW,				OnUpdateOutputSelected())
		
		//Notifications
		ON_COMMAND_UI_CHECKED(IDM_NOTIFY_S_OK,					m_pCMainWindow->m_pCListener->GetReturnValue() == S_OK)
		ON_COMMAND_UI_CHECKED(IDM_NOTIFY_S_FALSE,				m_pCMainWindow->m_pCListener->GetReturnValue() == S_FALSE)
		ON_COMMAND_UI_CHECKED(IDM_NOTIFY_S_UNWANTEDPHASE,		m_pCMainWindow->m_pCListener->GetReturnValue() == DB_S_UNWANTEDPHASE)
		ON_COMMAND_UI_CHECKED(IDM_NOTIFY_S_UNWANTEDREASON,		m_pCMainWindow->m_pCListener->GetReturnValue() == DB_S_UNWANTEDREASON)
		ON_COMMAND_UI_CHECKED(IDM_NOTIFY_E_FAIL,				m_pCMainWindow->m_pCListener->GetReturnValue() == E_FAIL)
	};

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CIntTrace::OnUpdateOutputSelected
//
/////////////////////////////////////////////////////////////////////////////
BOOL CIntTrace::OnUpdateOutputSelected()
{
	INDEX iStartPos = 0;
	INDEX iEndPos = 0;

	//Cut/Copy is only enabled - if something is selected.
	SendMessage(m_hWnd, EM_GETSEL, (WPARAM)&iStartPos, (LPARAM)&iEndPos);
	return iStartPos != iEndPos;
}


/////////////////////////////////////////////////////////////////////
// CIntTrace::OnCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CIntTrace::OnCommand(UINT iID, HWND hWndCtrl)
{
	switch(iID)
	{
		//Editing
		ON_COMMAND(IDM_CLEARNOTIFYWINDOW,				Clear())
		ON_COMMAND(IDM_COPYNOTIFYWINDOW,				SendMessage(m_hWnd, WM_COPY, 0, 0))

		//Notifications
		ON_COMMAND(IDM_NOTIFY_S_OK,						m_pCMainWindow->m_pCListener->SetReturnValue(S_OK))
		ON_COMMAND(IDM_NOTIFY_S_FALSE,					m_pCMainWindow->m_pCListener->SetReturnValue(S_FALSE))
		ON_COMMAND(IDM_NOTIFY_S_UNWANTEDPHASE,			m_pCMainWindow->m_pCListener->SetReturnValue(DB_S_UNWANTEDPHASE))
		ON_COMMAND(IDM_NOTIFY_S_UNWANTEDREASON,			m_pCMainWindow->m_pCListener->SetReturnValue(DB_S_UNWANTEDREASON))
		ON_COMMAND(IDM_NOTIFY_E_FAIL,					m_pCMainWindow->m_pCListener->SetReturnValue(E_FAIL))
	};

	return FALSE;
}
		
//////////////////////////////////////////////////////////////////
// CIntTrace::OutputLineEnd
//
//////////////////////////////////////////////////////////////////
void CIntTrace::OutputLineEnd()
{
	//Standard MultiLine Edit Controls require (\r\n)
	//Anything else results in no line break
	//Even RichEdit controls "\n" works but when copying to text files they are lost
	ReplaceSel(L"\r\n", TRUE);
}


//////////////////////////////////////////////////////////////////
// CIntTrace::OutputText
//
//////////////////////////////////////////////////////////////////
void CIntTrace::OutputTextFmt(DWORD dwMask, COLORREF dwColor, WCHAR* pwszFmt, ...)
{
	ASSERT(pwszFmt);

	va_list		marker;
	WCHAR		wszBuffer[MAX_QUERY_LEN];

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf_s(wszBuffer, MAX_QUERY_LEN, _TRUNCATE, pwszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_NAME_LEN
	wszBuffer[MAX_QUERY_LEN-1] = wEOL;	
	
	//Delegate 				  
	OutputText(dwMask, dwColor, wszBuffer);
}



//////////////////////////////////////////////////////////////////
// CIntTrace::OutputText
//
//////////////////////////////////////////////////////////////////
void CIntTrace::OutputText(DWORD dwMask, COLORREF dwColor, WCHAR* pwszText)
{
	ASSERT(pwszText);

	//No-op - no output window...
	if(m_hWnd)
	{
		//Move the Caret to the End
		SetSel(LONG_MAX, LONG_MAX);
		//Append the New String
		ReplaceSel(pwszText, FALSE, dwMask, dwColor);
	}
}


//////////////////////////////////////////////////////////////////
// CIntTrace::OutputIndent
//
//////////////////////////////////////////////////////////////////
void CIntTrace::OutputIndent(ULONG cIndentLevel)
{
	for(ULONG i=0; i<cIndentLevel; i++)
		OutputText(0, 0, L"\t");
}

			
//////////////////////////////////////////////////////////////////
// CIntTrace::GetTrace
//
//////////////////////////////////////////////////////////////////
CIntTrace*	CIntTrace::GetTrace()
{ 
	CMainWindow* pCMainWindow = (CMainWindow*)CAppLite::m_pCMainWindow;
	if(pCMainWindow)
	{
		if(pCMainWindow->m_pCMDITrace)
			return pCMainWindow->m_pCMDITrace->m_pCTrace;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////
// CIntTrace::TraceAddRef
//
//////////////////////////////////////////////////////////////////
ULONG CIntTrace::TraceAddRef(IUnknown* pIUnknown, WCHAR* pwszFmt)
{
	ULONG ulRefCount = 0;

	//AddRef
	if(pIUnknown)
	{
		ulRefCount = pIUnknown->AddRef();

		CIntTrace* pCTrace = GetTrace();
		if(pCTrace)
		{
			if(NOTIFY_ADDREF & pCTrace->GetOptions()->m_dwNotifyOpts)
			{
				pCTrace->OutputTextFmt(0, 0, L"%s::AddRef() - %d", pwszFmt ? pwszFmt : L"IUnknown", ulRefCount);
				pCTrace->OutputLineEnd();
			}
		}
	}

	return ulRefCount;
}


//////////////////////////////////////////////////////////////////
// CIntTrace::TraceRelease
//
//////////////////////////////////////////////////////////////////
ULONG CIntTrace::TraceRelease(IUnknown** ppIUnknown, WCHAR* pwszFmt, ULONG ulExpectedRefCount)
{
	ULONG ulRefCount = 0;

	//Release
	if(ppIUnknown && *ppIUnknown)
	{
		//Actual Release and NULL pointer
		ulRefCount = (*ppIUnknown)->Release();
		*ppIUnknown = NULL;

		//Trace
		CIntTrace* pCTrace = GetTrace();
		if(pCTrace)
		{
			if(NOTIFY_RELEASE & pCTrace->GetOptions()->m_dwNotifyOpts)
			{
				pCTrace->OutputTextFmt(0, 0, L"%s::Release() - %d", pwszFmt ? pwszFmt : L"IUnknown", ulRefCount);
				pCTrace->OutputLineEnd();
			}
		}

		//Display Errors if RefCount is not what is expected...
		if(ulExpectedRefCount==0 && ulRefCount!=0)
			DisplayRefCountErrors(pwszFmt, ulRefCount, ulExpectedRefCount);
	}

	return ulRefCount;
}


//////////////////////////////////////////////////////////////////
// HRESULT CIntTrace::TraceQI
//
//////////////////////////////////////////////////////////////////
HRESULT CIntTrace::TraceQI(IUnknown* pIUnknown, REFIID riid, IUnknown** ppIUnknown, WCHAR* pwszFmt)
{
	HRESULT hr = S_OK;

	if(pIUnknown)
	{
		//QueryInterface
		hr = pIUnknown->QueryInterface(riid, (void**)ppIUnknown);

		CIntTrace* pCTrace = GetTrace();
		if(pCTrace)
		{
			if(NOTIFY_QI & pCTrace->GetOptions()->m_dwNotifyOpts)
				TraceMethod(hr, L"%s::QueryInterface(%s, &0x%p)", pwszFmt ? pwszFmt : L"IUnknown", GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL);
		}
	}
	
	return hr;
}


//////////////////////////////////////////////////////////////////
// HRESULT CIntTrace::TraceMethod
//
//////////////////////////////////////////////////////////////////
HRESULT CIntTrace::TraceMethod(HRESULT hrActual, WCHAR* pwszFmt, ...)
{
	ASSERT(pwszFmt);
	CIntTrace* pCTrace = GetTrace();
	if(pCTrace)
	{
		DWORD dwNotifyOpts = pCTrace->GetOptions()->m_dwNotifyOpts;
		if(dwNotifyOpts & NOTIFY_OLEDB)
		{
			//Only if the user is interested in this message
			va_list		marker;
			WCHAR		wszBuffer[MAX_QUERY_LEN];

			// Use format and arguements as input
			//This version will not overwrite the stack, since it only copies
			//upto the max size of the array
			va_start(marker, pwszFmt);
			_vsnwprintf_s(wszBuffer, MAX_QUERY_LEN, _TRUNCATE, pwszFmt, marker);
			va_end(marker);

			//Make sure there is a NULL Terminator, vsnwprintf will not copy
			//the terminator if length==MAX_NAME_LEN
			wszBuffer[MAX_QUERY_LEN-1] = wEOL;	

			//Colored Errors
			DWORD dwMask = FAILED(hrActual) ? (CFM_BOLD | CFM_COLOR) : hrActual!=S_OK ? CFM_BOLD : 0;
			DWORD dwColor = RGB_RED;
			
			//Delegate 				  
			pCTrace->OutputTextFmt(dwMask, dwColor, L"%s - %S", wszBuffer, GetErrorName(hrActual));
			pCTrace->OutputLineEnd();
		}
	}
	
	return hrActual;
}


//////////////////////////////////////////////////////////////////
// HRESULT CIntTrace::TraceNotification
//
//////////////////////////////////////////////////////////////////
HRESULT CIntTrace::TraceNotification(DWORD dwNotifyType, HRESULT hrVeto, WCHAR* pwszInterface, WCHAR* pwszMethod, WCHAR* pwszParams, ...)
{
	ASSERT(pwszParams);
	CIntTrace* pCTrace = GetTrace();
	if(pCTrace)
	{
		DWORD dwNotifyOpts = pCTrace->GetOptions()->m_dwNotifyOpts;
		WCHAR wszBuffer[MAX_QUERY_LEN] = {0};
		BOOL fShowVeto = (dwNotifyOpts & NOTIFY_PROMPTVETO) && (dwNotifyType & (NOTIFY_IDBASYNCHNOTIFY | NOTIFY_IROWSETNOTIFY | NOTIFY_IROWPOSITIONCHANGE));

		//Only if the user is interested in this message
		if(dwNotifyOpts & dwNotifyType || fShowVeto)
		{
			va_list		marker;

			// Use format and arguements as input
			//This version will not overwrite the stack, since it only copies
			//upto the max size of the array
			va_start(marker, pwszParams);
			_vsnwprintf_s(wszBuffer, MAX_QUERY_LEN, _TRUNCATE, pwszParams, marker);
			va_end(marker);

			//Make sure there is a NULL Terminator, vsnwprintf will not copy
			//the terminator if length==MAX_NAME_LEN
			wszBuffer[MAX_QUERY_LEN-1] = wEOL;	
		}

		//Do we need to prompt for Accept or veto?
		if(fShowVeto)
		{
			hrVeto = S_OK;
			if(IDCANCEL == wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON1, 
				L"Received Notification", 
				L"%s::%s\n"
				L"%s\n\n"
				L"'OK' - Accept notification, 'CANCEL' - Veto Notification", 
				pwszInterface,
				pwszMethod,
				wszBuffer
				))
			{
				hrVeto = S_FALSE;
			}
		}

		//Only if the user is interested in this message
		if(dwNotifyOpts & dwNotifyType)
		{
			//Colored Errors
			DWORD dwMask = FAILED(hrVeto) ? (CFM_BOLD | CFM_COLOR) : hrVeto!=S_OK ? CFM_BOLD : CFM_COLOR;
			DWORD dwColor = FAILED(hrVeto) ? RGB_RED : RGB_DARK_GREEN;
			

			//Now acutally output the Notification to the trace
			pCTrace->OutputTextFmt(dwMask, dwColor, L"\t%s::%s%s - %S", pwszInterface, pwszMethod, wszBuffer, GetErrorName(hrVeto));
			pCTrace->OutputLineEnd();
		}
	}	
	return hrVeto;
}





////////////////////////////////////////////////////////////////
// CIntTrace::Clear
//
/////////////////////////////////////////////////////////////////
HRESULT CIntTrace::Clear()
{
	//Select all the Text and Cut it...
	//This method allows the text to be undone...
//	SendMessage(m_hWnd, EM_SETSEL, 0, -1);
//	SendMessage(m_hWnd, WM_CUT, 0, 0);

	//Since this window is ReadOnly, we can't Cut it, we can only delete...
	SendMessage(m_hWnd, WM_SETTEXT, 0, (LPARAM)"");
	return S_OK;
}





////////////////////////////////////////////////////////////////
// CMDITrace::CMDITrace
//
/////////////////////////////////////////////////////////////////
CMDITrace::CMDITrace(CMainWindow* pCMainWindow)
{
	//Objects
	ASSERT(pCMainWindow);
	m_pCMainWindow = pCMainWindow;
	
	//Controls
	m_pCTrace		= new CIntTrace(pCMainWindow);
}


////////////////////////////////////////////////////////////////
// CMDITrace::~CMDITrace
//
/////////////////////////////////////////////////////////////////
CMDITrace::~CMDITrace()
{
	//Controls
	SAFE_DELETE(m_pCTrace);
}


////////////////////////////////////////////////////////////////
// CMDITrace::PreCreateWindow
//
/////////////////////////////////////////////////////////////////
BOOL CMDITrace::PreCreateWindow(CREATESTRUCTW& cs)
{
	//Load Saved Window Positions
	memset(&m_wndPlacement,	0, sizeof(m_wndPlacement));
	GetRegEntry(HKEY_ROWSETVIEWER, wszTRACE_KEY, L"WinPosition", &m_wndPlacement, sizeof(m_wndPlacement), NULL);

	//Window Hidden?
	if(m_wndPlacement.length)
	{
		if(m_wndPlacement.showCmd == SW_HIDE)
			cs.style &= ~WS_VISIBLE;
	}
		
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDITrace::AutoPosition
//
/////////////////////////////////////////////////////////////////
BOOL CMDITrace::AutoPosition(BOOL fDefaultPosition)
{
	if(fDefaultPosition || m_wndPlacement.length == 0)
	{
		//Default setting, bottom right corner (1/4 of the client area)...
		SIZE sizeClient = GetClientSize(m_pCMainWindow->m_hWndMDIClient);
		return MoveWindow(m_hWnd, (INT)((float)sizeClient.cx * 0.25), (INT)((float)sizeClient.cy * 0.75), (INT)((float)sizeClient.cx * 0.75), (INT)((float)sizeClient.cy * 0.25), TRUE);
	}
	else
	{
		return SetWindowPlacement();
	}
}


////////////////////////////////////////////////////////////////
// CMDITrace::OnCreate
//
/////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//Create Trace Window
	//NOTE: The older richedit control (RichEd32.dll - 1.0 version) requires the cx,cy to be none-0
	//Otherwise the control will fail to be created.
	SIZE size = GetClientSize(m_hWnd);
	m_pCTrace->Create(m_hWnd, m_pCMainWindow->m_hLibRichEdit20 ? (IsUnicodeOS() ? L"RichEdit20W" : L"RichEdit20A") : L"RICHEDIT", NULL, ID_MDITRACE, 
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | WS_TABSTOP | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_DISABLENOSCROLL | ES_MULTILINE | ES_NOHIDESEL | ES_SELECTIONBAR | ES_READONLY, WS_EX_CLIENTEDGE,
		0, 0, size.cx, size.cy);
	m_pCTrace->OnCreate(NULL);

	//By default the RichEdit controls have a HUGE text.  Instead of setting it
	//to a small size font which is difficult for some international lcids, we will
	//set it to the same font as the Tree control...
	m_pCTrace->SetFont(m_pCMainWindow->m_pCMDIObjects->m_pCObjTree->GetFont());

	//Window Position
	AutoPosition(m_wndPlacement.length == 0/*fDefaultPosition*/);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMDITrace::OnDestroy
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnDestroy()
{
	//Save Window Positions
	if(GetWindowPlacement())
		SetRegEntry(HKEY_ROWSETVIEWER, wszTRACE_KEY, L"WinPosition",	&m_wndPlacement,	sizeof(m_wndPlacement));

	//Delegate
	return CMDIChildLite::OnDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CMDITrace::OnClose
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnClose()
{
	//NOTE:  OnClose only gets hit for MDI Child Windows when the user actually
	//closes the window.  If the mainframe window is closed directly then OnClose is never fired...
	
	//Due to this, if the user has closed the window before exiting the app, then all we really 
	//want to do is make the window not-visible, so if they bring the window up again it has all
	//the info saved.
	ShowWindow(SW_HIDE);

	//NOTE:  Don't delegate the call, since we don't want to "DestroyWindow"
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDITrace::OnInitialUpdate
//
/////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnInitialUpdate()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CMDITrace::OnMDIActivate
//
/////////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnMDIActivate(BOOL bActivate, HWND hWndActivate, HWND hWndDeactivate)
{
	//So see if this window is being activated or deactivated
	if(hWndActivate == m_hWnd)
	{
		//Refresh all Controls
		UpdateControls();
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CMDITrace::UpdateControls
//
/////////////////////////////////////////////////////////////////
BOOL CMDITrace::UpdateControls()
{
	//ToolBar Buttons
	m_pCMainWindow->UpdateControls();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CMDITrace::OnUpdateCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags)
{
	return m_pCTrace->OnUpdateCommand(hMenu, nID, pdwFlags);
}


/////////////////////////////////////////////////////////////////////
// CMDITrace::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	//Delegate
	return m_pCTrace->OnNotify(idCtrl, pNMHDR);
}



/////////////////////////////////////////////////////////////////////
// CMDITrace::OnCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnCommand(UINT iID, HWND hWndCtrl)
{
	return m_pCTrace->OnCommand(iID, hWndCtrl);
}


/////////////////////////////////////////////////////////////////////
// CMDITrace::OnSize
//
/////////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnSize(WPARAM nType, REFPOINTS pts)
{
	ASSERT(m_pCTrace->m_hWnd);
	
	switch(nType)
	{
		case SIZE_RESTORED:
		case SIZE_MAXIMIZED:
		{
			if(pts.x && pts.y)
			{
				//Obtain window sizes...
				MoveWindow(m_pCTrace->m_hWnd, 0, 0, pts.x, pts.y, TRUE);
			
				//Call default procedure first, 
				//to let MDI position the child & then move its children
				//We simply do this by returning false, to indicate we didn't handle it...
				return FALSE;
			}
			break;
		}
	};

	return FALSE;
}                


/////////////////////////////////////////////////////////////////////////////
// CMDITrace::OnSetFocus
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMDITrace::OnSetFocus(HWND hWndPrevFocus)
{
	m_pCTrace->SetFocus();
	return TRUE;
}


//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module CDIALOGLITE.CPP
//
//-----------------------------------------------------------------------------------



//////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#include "Headers.h"


//////////////////////////////////////////////////////////////////////////
// Globals
//
//////////////////////////////////////////////////////////////////////////
CAppLite*		g_pCAppLite = NULL;
CAppLite*		GetAppLite()					{ return g_pCAppLite;		}
void			SetAppLite(CAppLite* pCAppLite)	{ g_pCAppLite = pCAppLite;	}



//////////////////////////////////////////////////////////////////////////////
// WinMain
//
//////////////////////////////////////////////////////////////////////////////
int PASCAL WinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	LPSTR lpCmdLine, 
	int nCmdShow
	)
{
	//Should have a static CAppLite object at this point...
	CAppLite* pCAppLite = GetAppLite();
	ASSERT(pCAppLite);

	//Create CWndApp
	if(!pCAppLite->AppInitialize(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
		return -1;
	
	//Init
	if(!pCAppLite->InitInstance())
		return pCAppLite->ExitInstance();

	//Make sure we have a CMainWindow setup at this point...
	if(!CAppLite::m_pCMainWindow || !CAppLite::m_pCMainWindow->m_hWnd)
		return pCAppLite->ExitInstance();

	//Run
	if(!pCAppLite->Run())
		return pCAppLite->ExitInstance();

	//Exit
	return pCAppLite->ExitInstance();
}




HWND			CAppLite::m_hWndModeless = NULL;	//Static
CFrameWndLite*	CAppLite::m_pCMainWindow = NULL;	//Static
/////////////////////////////////////////////////////////////////////
// CAppLite::CAppLite
//
/////////////////////////////////////////////////////////////////////
CAppLite::CAppLite(UINT nAppID)
{
	m_nAppID = nAppID;

	//Need to setup global CAppLite
	SetAppLite(this);
}


/////////////////////////////////////////////////////////////////////
// CAppLite::~CAppLite
//
/////////////////////////////////////////////////////////////////////
CAppLite::~CAppLite()
{
	SetAppLite(NULL);
}


/////////////////////////////////////////////////////////////////////
// CAppLite::AppInitialize
//
/////////////////////////////////////////////////////////////////////
BOOL CAppLite::AppInitialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, CHAR* pszCmdLine, INT nCmdShow)
{
	m_hInstance		= hInstance;
	m_hPrevInstance	= hPrevInstance;
	m_pszCmdLine	= pszCmdLine;
	m_nCmdShow		= nCmdShow;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CAppLite::InitInstance
//
/////////////////////////////////////////////////////////////////////
BOOL CAppLite::InitInstance()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CAppLite::Run
//
/////////////////////////////////////////////////////////////////////
int CAppLite::Run()
{
	MSG		msg;

	//load accelerators
	HACCEL	hAccel = LoadAccelerators(m_hInstance, MAKEINTRESOURCE(m_nAppID));
	
	// acquire and dispatch messages until a WM_QUIT message is received
	while(GetMessage(&msg, NULL, 0, 0))
	{           
		//Modeless dialog boxes...
		if(m_hWndModeless && IsWindow(m_hWndModeless) && IsDialogMessage(m_hWndModeless, &msg))
			continue;

		//Main Window Translation?
		if(m_pCMainWindow)
		{
			if(m_pCMainWindow->PreTranslateMessage(&msg))
				continue;
		}
		
		//Check for App accelerators
		if(hAccel && TranslateAccelerator(m_pCMainWindow->m_hWnd, hAccel, &msg))
			continue;
		
        // if the message does not need special processing, dispatch it
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CAppLite::ExitInstance
//
/////////////////////////////////////////////////////////////////////
int CAppLite::ExitInstance()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::CWndLite
//
/////////////////////////////////////////////////////////////////////
CWndLite::CWndLite(HWND hWndParent, UINT nID)
{
	m_hWnd			= NULL;
	m_hWndParent	= NULL;
	m_pwszClassName	= NULL;
	m_nID			= 0;
	m_bUnicodeMsg	= IsUnicodeOS();

	if(hWndParent && nID)
		CreateIndirect(hWndParent, nID);

	//SubClass
	m_pSubClassProc	 = NULL;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::~CWndLite
//
/////////////////////////////////////////////////////////////////////
CWndLite::~CWndLite()
{
	SAFE_FREE(m_pwszClassName);
}


/////////////////////////////////////////////////////////////////////
// CWndLite::PreCreateWindow
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::PreCreateWindow(CREATESTRUCTW& cs)
{
	return TRUE;
}



/////////////////////////////////////////////////////////////////////
// CWndLite::Create
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::Create(HWND hWndParent, WCHAR* pwszClassName,
						WCHAR* pwszWindowName, UINT uID,
						DWORD dwStyle, DWORD dwExStyle, 
						int x, int y, int cx, int cy)
{
	ASSERT(IsDestroyed());
	m_hWndParent = hWndParent;

	//Setup CreateStruct
	CREATESTRUCTW cs;
	cs.dwExStyle		= dwExStyle;
	cs.lpszClass		= pwszClassName;
	cs.lpszName			= pwszWindowName;
	cs.style			= dwStyle;
	cs.x				= x;
	cs.y				= y;
	cs.cx				= cx;
	cs.cy				= cy;
	cs.hwndParent		= hWndParent;
	cs.hMenu			= (HMENU)(ULONG_PTR)uID;
	cs.hInstance		= GetAppLite()->m_hInstance;
	cs.lpCreateParams	= this;

	//Allow Modification of CreateParams...
	if(!PreCreateWindow(cs))
		return FALSE;

	//Convert the Params
	CHAR szClassName[MAX_NAME_LEN] = {0};
	ConvertToMBCS(pwszClassName, szClassName, MAX_NAME_LEN);
	CHAR szWindowName[MAX_NAME_LEN] = {0};
	ConvertToMBCS(pwszWindowName, szWindowName, MAX_NAME_LEN);

	//Copy the Class Name
	SAFE_FREE(m_pwszClassName);
	m_pwszClassName = wcsDuplicate(pwszClassName);

	//Actually CreateWindowEx
	m_hWnd = CreateWindowExA(cs.dwExStyle, szClassName,
			szWindowName, cs.style, cs.x, cs.y, cs.cx, cs.cy,
			cs.hwndParent, cs.hMenu, cs.hInstance, cs.lpCreateParams);
		
	//Assoicate this window with this class
	if(m_hWnd)
	{
		SetThis(m_hWnd, this);
		return OnInitialUpdate();
	}

	GETLASTERROR(m_hWnd)
	return NULL;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::CreateIndirect
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::CreateIndirect(HWND hWndParent, UINT nID)
{
	m_nID			= nID;

	if(nID)
	{
		//Its a control on a window
		m_hWnd			= ::GetDlgItem(hWndParent, m_nID);
		m_hWndParent	= hWndParent;
	}
	else
	{
		//Its just a window
		m_hWnd			= hWndParent;
		m_hWndParent	= GetParent(m_hWnd);
	}

	//TODO:
	//Some messages (ie: EM_GETTEXTRANGE), since their is not a W-unicode and A-ansi version), 
	//the message is completly based upon how you created the window.  If we created the window
	//incorectly, then we are limited to how the resource created it which is basically how you
	//compile the source.  Its also dependent upon weither you subclass the window.
	//(SetWindowLongPtrA or SetWindowLongW).
#ifndef UNICODE	
	m_bUnicodeMsg	= FALSE;
#endif

	if(m_hWnd)
	{
		//NOTE: You only need to save the "this" pointer if your going
		//to subclass the window.  Otherwise two simple classes on a window control
		//will end up overwriting each other.  So we moved this to SubClassWindow
		return OnInitialUpdate();
	}
	
	return FALSE;
}



/////////////////////////////////////////////////////////////////////
// CWndLite::OnInitialUpdate
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnInitialUpdate()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::DestroyWindow
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::DestroyWindow()
{
	if(m_hWnd)
	{
		::DestroyWindow(m_hWnd);
		m_hWnd = NULL;
		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::SubClassWindow
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::SubClassWindow(WNDPROC pWndProc)
{
	//NOTE: This method does not handle multiple levels...
	ASSERT(m_pSubClassProc == NULL);

	//Save the "this" pointer (in case we haven't already, for the case where
	//we are subclassing a window thats already created - CreateIndirect).
	SetThis(m_hWnd, this);

	m_pSubClassProc = (WNDPROC)GetWindowLongPtr(m_hWnd, GWLP_WNDPROC);
    SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)pWndProc);
	return TRUE;
}

	
/////////////////////////////////////////////////////////////////////
// CWndLite::OnCreate
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnCreate(CREATESTRUCT* pCREATESTRUCT)
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnCommandNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnCommand(UINT iID, HWND hWndCtrl)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnUpdateCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags)
{
	return FALSE;
}

	
/////////////////////////////////////////////////////////////////////
// CWndLite::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnActivate
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnActivate(UINT fActive, UINT fMinimized, HWND hWndPrevious)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnClose
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnClose()
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnDestroy
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnDestroy()
{
	//Remove our Class from the hWnd lookup...
	SetThis(m_hWnd, NULL);
	
	//Remove window items...
	m_hWnd			= NULL;
	m_hWndParent	= NULL;
	SAFE_FREE(m_pwszClassName);
	m_pSubClassProc	= NULL;	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnTimer
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnTimer(WPARAM nIDEvent)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnDropFiles
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnDropFiles(HDROP hDrop)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnSysCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnSysCommand(WPARAM nCmdType, REFPOINTS pts)
{
	return FALSE;
}
	
/////////////////////////////////////////////////////////////////////
// CWndLite::OnSize
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnSize(WPARAM nType, REFPOINTS pts)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// CWndLite::OnSizing
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnSizing(WPARAM nSize, REFPOINTS pts)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnMove
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnMove(REFPOINTS pts)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// CWndLite::OnMouseMove
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnMouseMove(WPARAM nHittest, REFPOINTS pts)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnDblclk
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnDblclk(WPARAM fwKeys, REFPOINTS pts)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnLButtonDown
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnLButtonDown(WPARAM fwKeys, REFPOINTS pts)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnLButtonUp
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnLButtonUp(WPARAM fwKeys, REFPOINTS pts)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnRButtonDown
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnRButtonDown(WPARAM fwKeys, REFPOINTS pts)
{
	//NOTE: The right mouse button doesn't automatically activate the MDI window...
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnRButtonUp
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnRButtonUp(WPARAM fwKeys, REFPOINTS pts)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnSetCursor
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnSetCursor(HWND hWnd, INT nHittest, INT nMouseMsg)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnContextMenu
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnContextMenu(HWND hWnd, REFPOINTS pts)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnChar
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnChar(TCHAR chCharCode, LPARAM lKeyData)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnKeyDown
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// CWndLite::OnSysKeyDown
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnSysKeyDown(WPARAM nVirtKey, LPARAM lKeyData)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// CWndLite::OnVScroll
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnVScroll(int nScrollCode, int nPos, HWND hWnd)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnHScroll
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnHScroll(int nScrollCode, int nPos, HWND hWnd)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::OnNCMouseMove
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnNCMouseMove(WPARAM nHittest, REFPOINTS pts)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// CWndLite::OnNCButtonDown
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnNCButtonDown(WPARAM nHittest, REFPOINTS pts)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CWndLite::OnSetFocus
//
/////////////////////////////////////////////////////////////////////////////
BOOL CWndLite::OnSetFocus(HWND hWndPrevFocus)
{
	return FALSE;
}


////////////////////////////////////////////////////////////////
// CWndLite::OnMenuSelect
//
/////////////////////////////////////////////////////////////////
BOOL CWndLite::OnMenuSelect(UINT uID)
{
	return FALSE;
}


////////////////////////////////////////////////////////////////
// CWndLite::OnInitMenuPopup
//
/////////////////////////////////////////////////////////////////
BOOL CWndLite::OnInitMenuPopup(HMENU hMenu, UINT uPos, BOOL fSysMenu)
{
	//Ignore the msg if it is for a system menu
	if(fSysMenu)
		return FALSE;
	
	//Go through the menu items for current popup menu
	//and enable/disable menu item, if required
	
	//Also note that HandleMenuPos is a recursive algortym.  This is for the
	//case where I need to detmerine if a "submenu->" is enabled or diasbled.
	//The only way to really tell is to know if there are any subitems that
	//are needed, but some of the subitems inturn may be "submenu->".
	//Therefore a item will be enabled if there is at least one subitem 
	//The is wanted, and disabled if there are no subitems wanted.
	INT iMenuItems = GetMenuItemCount(hMenu);
	DWORD dwFlags = 0;
	for(LONG i=0; i<iMenuItems; i++)
	{
		dwFlags = 0;
		if(HandleMenuPos(hMenu, i, &dwFlags))
		{
			EnableMenuItem(hMenu, i, MF_BYPOSITION | dwFlags);
			CheckMenuItem(hMenu, i, MF_BYPOSITION | dwFlags);
		}
	}
	
	return TRUE;
}

			
////////////////////////////////////////////////////////////////
// CWndLite::HandleMenuPos
//
/////////////////////////////////////////////////////////////////
BOOL CWndLite::HandleMenuPos(HMENU hMenu, UINT uPos, DWORD* pdwFlags)
{
	ASSERT(pdwFlags);
	*pdwFlags = 0;
	ULONG ulMenuID = GetMenuItemID(hMenu, uPos);

	//SubMenu
	if(ulMenuID == ULONG_MAX)
	{
		BOOL bHandled = FALSE;
		
		//Recursive Alogorytm
		HMENU hSubMenu = GetSubMenu(hMenu, uPos);
		INT iItems = GetMenuItemCount(hSubMenu);
		for(LONG i=0; i<iItems; i++)
		{
			//As soon as we find one menu item that we handle, we can return
			//since we now know the entire sub menu needs to be enabled for this item...
			if(HandleMenuPos(hSubMenu, i, pdwFlags))
			{
				bHandled = TRUE;
				if(*pdwFlags == MF_ENABLED)
					return TRUE;
			}
		}
	
		return bHandled;
	}

	//Otherwise we have a valid Menu ID...
	return OnUpdateCommand(hMenu, ulMenuID, pdwFlags);
}


/////////////////////////////////////////////////////////////////////
// CWndLite::HandleMessage
//
/////////////////////////////////////////////////////////////////////
BOOL CWndLite::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_COMMAND:
		{
			UINT wNotifyCode	= HIWORD(wParam);	// notification code 
			UINT iID			= LOWORD(wParam);	// item, control, or accelerator identifier 
			HWND hWndCtrl		= (HWND)lParam;		// handle of control  

			//Filter out any Control Notification codes
			//	wNotifyCode		- Specifies the notification code if the message is from a control. If the message is from an accelerator, this parameter is 1. If the message is from a menu, this parameter is 0. 
			//	wID 			- Specifies the identifier of the menu item, control, or accelerator. 
			//	hwndCtl 		- Identifies the control sending the message if the message is from a control. Otherwise, this parameter is NULL. 
			if(hWndCtrl && wNotifyCode)
				return OnCommandNotify(wNotifyCode, iID, hWndCtrl);
			
			return OnCommand(iID, hWndCtrl);
		}

		case WM_SYSCOMMAND:
			return OnSysCommand(wParam, MAKEPOINTS(lParam));
		
		case WM_ACTIVATE:
			return OnActivate(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);

		case WM_MENUSELECT:
			return OnMenuSelect(LOWORD(wParam));
		
		case WM_INITMENUPOPUP:
			return OnInitMenuPopup((HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
		
		case WM_NOTIFY:
			return OnNotify((INT)wParam, (NMHDR*)lParam);

		case WM_CLOSE:
			return OnClose();
	 
		case WM_DESTROY:
			return OnDestroy();

		case WM_TIMER:
			return OnTimer(wParam);

		case WM_DROPFILES:
			return OnDropFiles((HDROP)wParam);

		case WM_SIZE:
			return OnSize(wParam, MAKEPOINTS(lParam));

		case WM_SIZING:
			return OnSizing(wParam, MAKEPOINTS(lParam));

		case WM_MOVE:
			return OnMove(MAKEPOINTS(lParam));

		case WM_MOUSEMOVE:
			return OnMouseMove(wParam, MAKEPOINTS(lParam));

		case WM_LBUTTONDBLCLK:
			return OnDblclk(wParam, MAKEPOINTS(lParam));

		case WM_LBUTTONDOWN:
			return OnLButtonDown(wParam, MAKEPOINTS(lParam));

		case WM_LBUTTONUP:
			return OnLButtonUp(wParam, MAKEPOINTS(lParam));

		case WM_RBUTTONDOWN:
			return OnRButtonDown(wParam, MAKEPOINTS(lParam));

		case WM_RBUTTONUP:
			return OnRButtonUp(wParam, MAKEPOINTS(lParam));

		case WM_CONTEXTMENU:
			return OnContextMenu((HWND)wParam, MAKEPOINTS(lParam));

		case WM_CHAR:
			return OnChar((TCHAR)wParam, lParam);

		case WM_KEYDOWN:
			return OnKeyDown(wParam, lParam);

		case WM_SYSKEYDOWN:
			return OnSysKeyDown(wParam, lParam);

		case WM_VSCROLL:
			return OnVScroll(LOWORD(wParam), HIWORD(wParam), (HWND) lParam);

		case WM_HSCROLL:
			return OnHScroll(LOWORD(wParam), HIWORD(wParam), (HWND) lParam);

		case WM_NCMOUSEMOVE:
			return OnNCMouseMove(wParam, MAKEPOINTS(lParam));

		case WM_NCLBUTTONDOWN:
			return OnNCButtonDown(wParam, MAKEPOINTS(lParam));

		case WM_SETFOCUS:
			return OnSetFocus((HWND)wParam);
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CWndLite::UnhandledMessage
//
/////////////////////////////////////////////////////////////////////
LRESULT CWndLite::UnhandledMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////
// CWndLite::WndProc
//
/////////////////////////////////////////////////////////////////////
LRESULT WINAPI CWndLite::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CWndLite* pCWndLite = (CWndLite*)GetThis(hWnd);

	switch(msg)
	{
		case WM_CREATE:
        {
			//Save the Window Handle
			CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;
			pCWndLite = (CWndLite*)pCS->lpCreateParams;
			pCWndLite->m_hWnd = hWnd;

			//Save the "this" pointer
			SetThis(hWnd, pCWndLite);
			if(pCWndLite->OnCreate(pCS))
				return 0;
			return -1;
		}
	};

	//Otherwise just pass on message to original control
	if(pCWndLite)
	{
		//Pass onto our handler
		if(pCWndLite->HandleMessage(hWnd, msg, wParam, lParam))
			return 0;

		//Pass onto SubClass'd window, if their is one...
		if(pCWndLite->m_pSubClassProc)
			return CallWindowProc(pCWndLite->m_pSubClassProc, hWnd, msg, wParam, lParam);

		//Otherwise pass onto our default handler
		return pCWndLite->UnhandledMessage(hWnd, msg, wParam, lParam);
	}
		
	return DefWindowProc(hWnd, msg, wParam, lParam);
}



/////////////////////////////////////////////////////////////////////
// CMDIChildLite::CMDIChildLite
//
/////////////////////////////////////////////////////////////////////
CMDIChildLite::CMDIChildLite()
{
}


/////////////////////////////////////////////////////////////////////
// CMDIChildLite::~CMDIChildLite
//
/////////////////////////////////////////////////////////////////////
CMDIChildLite::~CMDIChildLite()
{
}


/////////////////////////////////////////////////////////////////////
// CMDIChildLite::Create
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChildLite::Create
(	
	HWND	hWndMDIClient, 
	WCHAR*	pwszClassName,
	WCHAR*	pwszWindowName, 
	UINT	uID, 
	HICON	hIcon, 
	DWORD	dwStyle,	
	DWORD	dwExStyle,
	int		x, 
	int		y, 
	int		cx, 
	int		cy
)
{
	ASSERT(IsDestroyed());
	m_hWndParent = hWndMDIClient;
	WNDCLASS wc;
	
	//Convert the Params
	CHAR szClassName[MAX_NAME_LEN] = {0};
	ConvertToMBCS(pwszClassName, szClassName, MAX_NAME_LEN);
	CHAR szWindowName[MAX_NAME_LEN] = {0};
	ConvertToMBCS(pwszWindowName, szWindowName, MAX_NAME_LEN);

	// Register MDI Window classes, if we haven't done so already...
	if(!GetClassInfo(GetAppLite()->m_hInstance, szClassName, &wc))
	{
		wc.style			= 0;
		wc.lpfnWndProc		= MDIWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= DLGWINDOWEXTRA;
		wc.hInstance		= GetAppLite()->m_hInstance;
		wc.hIcon 			= hIcon;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName		= MAKEINTRESOURCE(uID);
		wc.lpszClassName	= szClassName;

		// register MDI Child Window Class
		if(!RegisterClass(&wc))
			return FALSE;
	}

	//Setup CreateStruct
	CREATESTRUCTW cs;
	cs.dwExStyle		= dwExStyle;
	cs.lpszClass		= pwszClassName;
	cs.lpszName			= pwszWindowName;
	cs.style			= dwStyle;
	cs.x				= x;
	cs.y				= y;
	cs.cx				= cx;
	cs.cy				= cy;
	cs.hwndParent		= m_hWndParent;
	cs.hMenu			= (HMENU)(ULONG_PTR)uID;
	cs.hInstance		= GetAppLite()->m_hInstance;
	cs.lpCreateParams	= this;

	//Allow Modification of CreateParams...
	if(!PreCreateWindow(cs))
		return FALSE;

	//Setup MDICREATESTRUCT for real create
	MDICREATESTRUCTA mcs;
	mcs.szClass		= szClassName;
	mcs.szTitle		= szWindowName;
	mcs.hOwner		= cs.hInstance;
	mcs.x			= cs.x;
	mcs.y			= cs.y;
	mcs.cx			= cs.cx;
	mcs.cy			= cs.cy;
	mcs.style		= cs.style;
	mcs.lParam		= (LPARAM)this;
		
	//Copy the Class Name
	SAFE_FREE(m_pwszClassName);
	m_pwszClassName = wcsDuplicate(pwszClassName);

	//Create the window through the MDICLIENT window
	m_hWnd = (HWND)::SendMessage(hWndMDIClient, WM_MDICREATE, 0, (LPARAM)&mcs);

	//Assoicate this window with this class
	if(m_hWnd)
	{
		SetThis(m_hWnd, this);
		OnInitialUpdate();
	}

	return m_hWnd != NULL;
}


/////////////////////////////////////////////////////////////////////
// CMDIChildLite::HandleMessage
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChildLite::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_MDIACTIVATE:
			return OnMDIActivate(lParam ? TRUE : FALSE, (HWND)lParam, (HWND)wParam);
	}

	//Otherwise delegate
	return CWndLite::HandleMessage(hWnd, msg, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////
// CMDIChildLite::OnMDIActivate
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChildLite::OnMDIActivate(BOOL bActivate, HWND hWndActivate, HWND hWndDeactivate)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CMDIChildLite::AutoPosition
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIChildLite::AutoPosition(BOOL fDefaultPosition)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CMDIChildLite::UnhandledMessage
//
/////////////////////////////////////////////////////////////////////
LRESULT CMDIChildLite::UnhandledMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefMDIChildProc(hWnd, msg, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////
// CMDIChildLite::MDIWndProc
//
/////////////////////////////////////////////////////////////////////
LRESULT WINAPI CMDIChildLite::MDIWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
        {
			//Save the Window Handle
			CREATESTRUCT* pCS = (CREATESTRUCT*)lParam;
			MDICREATESTRUCT* pMDICS = (MDICREATESTRUCT*)pCS->lpCreateParams;
			CMDIChildLite* pCMDIChildLite = (CMDIChildLite*)pMDICS->lParam;

			//Save the "this" pointer
			SetThis(hWnd, pCMDIChildLite);
			
			//NOTE:  Inside CMDIChildLite::Create it has to call SendMessage
			//with MDICREATE inorder to create the window.  The problem is that
			//the window handle is returned from the message, and user code
			//within OnCreate may need the window handle to setup controls...
			pCMDIChildLite->m_hWnd = hWnd;
			if(pCMDIChildLite->OnCreate(pCS))
				return 0;
			return -1;
		}
	};

	//Otherwise just pass on message to original control
	return WndProc(hWnd, msg, wParam, lParam);
}




/////////////////////////////////////////////////////////////////////
// CFrameWndLite::CFrameWndLite
//
/////////////////////////////////////////////////////////////////////
CFrameWndLite::CFrameWndLite()
{
}


/////////////////////////////////////////////////////////////////////
// CFrameWndLite::~CFrameWndLite
//
/////////////////////////////////////////////////////////////////////
CFrameWndLite::~CFrameWndLite()
{
}


/////////////////////////////////////////////////////////////////////
// CFrameWndLite::Create
//
/////////////////////////////////////////////////////////////////////
BOOL CFrameWndLite::Create(HWND hWndParent, WCHAR* pwszClassName,
						WCHAR* pwszWindowName, UINT uID, HICON hIcon, 
						DWORD dwStyle, DWORD dwExStyle, 
						int x, int y, int cx, int cy)
{
	HINSTANCE hInstance = GetAppLite()->m_hInstance;
	DWORD dwResult = 0;
/*
	//TODO:
	//Left in for the truely UNICODE port
	
	 if(IsUnicodeOS() && 0)
	{
		//Setup the Structure
		WNDCLASSEXW	wc;
		wc.cbSize			= sizeof(WNDCLASSEXW);
		wc.style			= 0;
		wc.lpfnWndProc		= WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= hInstance;
		wc.hIcon			= hIcon;
		wc.hCursor			= LoadCursorW(NULL, (LPWSTR)IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE+1);
		wc.lpszMenuName		= MAKEINTRESOURCEW(uID);
		wc.lpszClassName	= pwszClassName;
		wc.hIconSm			= hIcon;

		//Register the window class
		dwResult = RegisterClassExW(&wc);
	}
	else
	{
*/		CHAR szClassName[MAX_NAME_LEN];
		ConvertToMBCS(pwszClassName, szClassName, MAX_NAME_LEN);

		//Setup the Structure
		WNDCLASSEXA	wc;
		wc.cbSize			= sizeof(WNDCLASSEXA);
		wc.style			= 0;
		wc.lpfnWndProc		= WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= hInstance;
		wc.hIcon			= hIcon;
		wc.hCursor			= LoadCursorA(NULL, (LPSTR)IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE+1);
		wc.lpszMenuName		= MAKEINTRESOURCEA(uID);
		wc.lpszClassName	= szClassName;
		wc.hIconSm			= hIcon;

		//Register the window class
		dwResult = RegisterClassExA(&wc);
/*	}
*/
	//Now actually Create the Window
	if(dwResult)
	{
		//Delegate
		CWndLite::Create(hWndParent, pwszClassName, pwszWindowName, NULL,
						dwStyle, dwExStyle, x, y, cx, cy);
	}

	return m_hWnd != NULL;
}


/////////////////////////////////////////////////////////////////////
// CFrameWndLite::PreTranslateMessage
//
/////////////////////////////////////////////////////////////////////
BOOL	CFrameWndLite::PreTranslateMessage(MSG* pmsg)
{
	return FALSE;
}

	
/////////////////////////////////////////////////////////////////////
// CFrameWndLite::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CFrameWndLite::OnNotify(INT idCtrl, NMHDR* pNMHDR) 
{
	//The ToolBar and Menu always send the message to the FrameWindow, and we need 
	//to delegate the message to our the child windows...
	switch(pNMHDR->code)
	{
		ON_COMMAND(TTN_NEEDTEXT, OnToolTip(idCtrl, pNMHDR));
	};

	//Delegate
	return CWndLite::OnNotify(idCtrl, pNMHDR);
}


/////////////////////////////////////////////////////////////////////
// CFrameWndLite::OnToolTip
//
/////////////////////////////////////////////////////////////////////
void CFrameWndLite::OnToolTip(INT idCtrl, NMHDR* pNMHDR)
{
	// Display the ToolTip text.
	LPTOOLTIPTEXTA lpToolTipText = (LPTOOLTIPTEXTA)pNMHDR;

	//Load the ToolTip string from the resource...
	static CHAR szBuffer[MAX_NAME_LEN];
	LoadStringA(GetAppLite()->m_hInstance, (UINT)lpToolTipText->hdr.idFrom, szBuffer, MAX_NAME_LEN);
	lpToolTipText->lpszText = szBuffer;
}


/////////////////////////////////////////////////////////////////////
// CFrameWndLite::OnClose
//
/////////////////////////////////////////////////////////////////////
BOOL CFrameWndLite::OnClose()
{
	return DestroyWindow();
}


/////////////////////////////////////////////////////////////////////
// CFrameWndLite::OnDestroy
//
/////////////////////////////////////////////////////////////////////
BOOL CFrameWndLite::OnDestroy()
{
	PostQuitMessage(0);

	//Delegate
	return CWndLite::OnDestroy();
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::CMDIFrameLite
//
/////////////////////////////////////////////////////////////////////
CMDIFrameLite::CMDIFrameLite()
{
	m_hWndMDIClient = NULL;
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::~CMDIFrameLite
//
/////////////////////////////////////////////////////////////////////
CMDIFrameLite::~CMDIFrameLite()
{
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::UnhandledMessage
//
/////////////////////////////////////////////////////////////////////
LRESULT CMDIFrameLite::UnhandledMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Need to pass unhandled messges to the MDI client...
	return DefFrameProc(hWnd, m_hWndMDIClient, msg, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::OnCreate
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIFrameLite::OnCreate(CREATESTRUCT* pCREATESTRUCT)
{
	//Delegate
	if(CFrameWndLite::OnCreate(pCREATESTRUCT))
	{	
		//Now Create the MDI Client...
		return OnCreateClient(pCREATESTRUCT);
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::OnDestroy
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIFrameLite::OnDestroy()
{
	//Remove window items...
	m_hWndMDIClient = NULL;

	//Delegate
	return CFrameWndLite::OnDestroy();
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::OnCreateClient
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIFrameLite::OnCreateClient(CREATESTRUCT* pCREATESTRUCT)
{
	DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | /*WS_VSCROLL | WS_HSCROLL |*/ WS_VISIBLE | CS_DBLCLKS;
	DWORD dwExStyle = WS_EX_CLIENTEDGE;

	//Now Setup MDI CreateStruct
	CLIENTCREATESTRUCT	ccs;
	ccs.hWindowMenu		= NULL;//GetSubMenu(GetMenu(m_hWnd), 9/*IDMENU_WINDOW*/);//TODO			//TODO
	ccs.idFirstChild	= IDC_MDICHILD;

	//Now we need to Create the MDI Client Window...
	m_hWndMDIClient = CreateWindowExA(dwExStyle, "MDICLIENT", NULL, dwStyle, 
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			m_hWnd, (HMENU)ID_MDICLIENT, GetAppLite()->m_hInstance, (LPSTR)&ccs);

	return m_hWndMDIClient != NULL;
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::OnCommandNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIFrameLite::OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl)
{
	CWndLite* pCWndLite = GetActiveWindow();
	if(pCWndLite)
		if(pCWndLite->OnCommandNotify(wNotifyCode, iID, hWndCtrl))
			return TRUE;

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::OnCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIFrameLite::OnCommand(UINT iID, HWND hWndCtrl)
{
	CWndLite* pCWndLite = GetActiveWindow();
	if(pCWndLite)
		if(pCWndLite->OnCommand(iID, hWndCtrl))
			return TRUE;

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::OnUpdateCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIFrameLite::OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags)
{
	CWndLite* pCWndLite = GetActiveWindow();
	if(pCWndLite)
		if(pCWndLite->OnUpdateCommand(hMenu, nID, pdwFlags))
			return TRUE;

	return FALSE;
}

	
/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CMDIFrameLite::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	CWndLite* pCWndLite = GetActiveWindow();
	if(pCWndLite)
		if(pCWndLite->OnNotify(idCtrl, pNMHDR))
			return TRUE;

	//Delegate
	return CFrameWndLite::OnNotify(idCtrl, pNMHDR);
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::PreTranslateMessage
//
/////////////////////////////////////////////////////////////////////
BOOL	CMDIFrameLite::PreTranslateMessage(MSG* pmsg)
{
	//Check for MDI accelerators
	return m_hWndMDIClient && TranslateMDISysAccel(m_hWndMDIClient, pmsg);
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::GetActiveWindow
//
/////////////////////////////////////////////////////////////////////
CMDIChildLite* CMDIFrameLite::GetActiveWindow(WCHAR* pwszClassName)
{
	ASSERT(m_hWndMDIClient);
		
	//Obtain the first Active Child Window...
	HWND hWndChild = GetWindow(m_hWndMDIClient, GW_CHILD);
	if(hWndChild)
	{
		//Return the first child of the specified type...
		CMDIChildLite* pCMDIChildLite = (CMDIChildLite*)GetThis(hWndChild);
		if(pCMDIChildLite)
		{
			if(pwszClassName)
			{
				if(StringCompare(pwszClassName, pCMDIChildLite->m_pwszClassName))
					return pCMDIChildLite;
			}
			else
			{
				return pCMDIChildLite;
			}
		}
	}

	return NULL;
}


/////////////////////////////////////////////////////////////////////
// CMDIFrameLite::FindWindow
//
/////////////////////////////////////////////////////////////////////
CMDIChildLite* CMDIFrameLite::FindWindow(WCHAR* pwszClassName)
{
	ASSERT(pwszClassName);

	//Obtain the first Active Child Window...
	HWND hWndChild = GetWindow(m_hWndMDIClient, GW_CHILD);
	while(hWndChild)
	{
		CMDIChildLite* pCMDIChildLite = (CMDIChildLite*)GetThis(hWndChild);
		if(pCMDIChildLite)
		{
			//Return the first child of the specified type...
			if(StringCompare(pCMDIChildLite->m_pwszClassName, pwszClassName))
				return pCMDIChildLite;
		}

		//Get the Next Window
		hWndChild = GetWindow(hWndChild, GW_HWNDNEXT);
	}

	return NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CMDIFrameLite::OnAutoPosition
//
/////////////////////////////////////////////////////////////////////////////
void CMDIFrameLite::OnAutoPosition()
{
	//Auto Position all children...
	HWND hWndChild = GetWindow(m_hWndMDIClient, GW_CHILD);
	while(hWndChild)
	{
		CMDIChildLite* pCMDIChildLite = (CMDIChildLite*)GetThis(hWndChild);
		if(pCMDIChildLite)
			pCMDIChildLite->AutoPosition();

		//Get the Next Window
		hWndChild = GetWindow(hWndChild, GW_HWNDNEXT);
	}
}



/////////////////////////////////////////////////////////////////////
// CDialogLite::CDialogLite
//
/////////////////////////////////////////////////////////////////////
CDialogLite::CDialogLite(UINT uIDD)
{
	m_uIDD		= uIDD;
	m_fModal	= TRUE;
}


/////////////////////////////////////////////////////////////////////
// CDialogLite::~CDialogLite
//
/////////////////////////////////////////////////////////////////////
CDialogLite::~CDialogLite()
{
}


/////////////////////////////////////////////////////////////////////
// CDialogLite::OnInitDialog
//
/////////////////////////////////////////////////////////////////////
BOOL CDialogLite::OnInitDialog()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CDialogLite::OnActivate
//
/////////////////////////////////////////////////////////////////////
BOOL CDialogLite::OnActivate(UINT fActive, UINT fMinimized, HWND hWndPrevious)
{
	if(!m_fModal)
		CAppLite::m_hWndModeless = fActive ? m_hWnd : NULL;

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CDialogLite::OnOK
//
/////////////////////////////////////////////////////////////////////
BOOL CDialogLite::OnOK()
{
	EndDialog(IDOK);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
// CDialogLite::OnCancel
//
/////////////////////////////////////////////////////////////////////
BOOL CDialogLite::OnCancel()
{
	EndDialog(IDCANCEL);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CDialogLite::DoModal
//
/////////////////////////////////////////////////////////////////////
LRESULT CDialogLite::DoModal(HWND hWndParent)
{
	//Modal Dialog Box
	m_hWndParent	= hWndParent;
	m_fModal		= TRUE;
	return DisplayDialog(m_uIDD, m_hWndParent, DlgProc, (LPARAM)this);
}


/////////////////////////////////////////////////////////////////////
// CDialogLite::CreateDlg
//
/////////////////////////////////////////////////////////////////////
HWND CDialogLite::CreateDlg(HWND hWndParent)
{
	//Modeless (non-Modal) Dialog Box
	m_hWndParent	= hWndParent;
	m_fModal		= FALSE;
	return CreateDialogParam(GetAppLite()->m_hInstance, MAKEINTRESOURCE(m_uIDD), m_hWndParent, DlgProc, (LPARAM)this);
}


/////////////////////////////////////////////////////////////////////
// CDialogLite::EndDialog
//
/////////////////////////////////////////////////////////////////////
void CDialogLite::EndDialog(int nResult)
{
	::EndDialog(m_hWnd, nResult);
}


/////////////////////////////////////////////////////////////////////
// CDialogLite::HandleMessage
//
/////////////////////////////////////////////////////////////////////
BOOL CDialogLite::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_COMMAND:
		{
			UINT wNotifyCode	= HIWORD(wParam);	// notification code 
			UINT iID			= LOWORD(wParam);	// item, control, or accelerator identifier 
			HWND hWndCtrl		= (HWND)lParam;		// handle of control  

			//Filter out any Control Notification codes
			//	wNotifyCode		- Specifies the notification code if the message is from a control. If the message is from an accelerator, this parameter is 1. If the message is from a menu, this parameter is 0. 
			//	wID 			- Specifies the identifier of the menu item, control, or accelerator. 
			//	hwndCtl 		- Identifies the control sending the message if the message is from a control. Otherwise, this parameter is NULL. 
			if(hWndCtrl && wNotifyCode)
				return OnCommandNotify(wNotifyCode, iID, hWndCtrl);
			
			//We are only interested in the following
			switch(iID)
			{
				case IDOK:
					return OnOK();

				case IDCANCEL:
					return OnCancel();
			};
			break;
		}
	};

	//Otherwise pass onto our main window handler
	return CWndLite::HandleMessage(hWnd, msg, wParam, lParam);
}
		

/////////////////////////////////////////////////////////////////////
// CDialogLite::DlgProc
//
/////////////////////////////////////////////////////////////////////
INT_PTR WINAPI CDialogLite::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDialogLite* pCDialogLite = (CDialogLite*)GetThis(hWnd);

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			//Save the Window Handle
			pCDialogLite = (CDialogLite*)lParam;
			pCDialogLite->m_hWnd = hWnd;

			//Save the "this" pointer
			SetThis(hWnd, pCDialogLite);
			return pCDialogLite->OnInitDialog();
		}
	};

	if(pCDialogLite)
		return pCDialogLite->HandleMessage(hWnd, msg, wParam, lParam);

	return FALSE;
};




/////////////////////////////////////////////////////////////////////
// CSplitterLite::CSplitterLite
//
/////////////////////////////////////////////////////////////////////
CSplitterLite::CSplitterLite(HWND hWndParent, UINT nID)
	: CWndLite(hWndParent, nID)
{
	m_pCWndTop		= NULL;
	m_pCWndBottom	= NULL;
	m_pCWndLeft		= NULL;
	m_pCWndRight	= NULL;

	m_hCursorOld	= NULL;
	m_hCursorTop	= NULL;
	m_hCursorRight	= NULL;

	m_iBorderCX		= 0;
	m_iBorderCY		= 0;
}


/////////////////////////////////////////////////////////////////////
// CSplitterLite::SetSplitter
//
/////////////////////////////////////////////////////////////////////
BOOL CSplitterLite::SetSplitter(CSplitterLite* pCWndTop, CSplitterLite* pCWndBottom, CSplitterLite* pCWndLeft, CSplitterLite* pCWndRight)
{
	m_pCWndTop		= pCWndTop;
	m_pCWndBottom	= pCWndBottom;
	m_pCWndLeft		= pCWndLeft;
	m_pCWndRight	= pCWndRight;

	//Load the appropiate cursor
	m_hCursorOld	= NULL;
	m_hCursorTop	= LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
	m_hCursorRight	= LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));

	m_iBorderCX		= GetSystemMetrics(SM_CXFIXEDFRAME);
	m_iBorderCY		= GetSystemMetrics(SM_CYFIXEDFRAME);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CSplitterLite::OnBorder
//
/////////////////////////////////////////////////////////////////////
BOOL CSplitterLite::OnBorder(REFPOINTS pts, INT iBorder)
{
	RECT rect;
	GetWindowRect(m_hWnd, &rect);
	
	switch(iBorder)
	{
		case HTLEFT:
			return pts.x <= (rect.left + m_iBorderCX);

		case HTRIGHT:
			return pts.x >= (rect.right - m_iBorderCX);

		case HTTOP:
			return pts.y <= (rect.top + m_iBorderCY);

		case HTBOTTOM:
			return pts.y >= (rect.bottom - m_iBorderCY);
	};

	return FALSE;
}

				
/////////////////////////////////////////////////////////////////////
// CSplitterLite::OnNCMouseMove
//
/////////////////////////////////////////////////////////////////////
BOOL CSplitterLite::OnNCMouseMove(WPARAM nHittest, REFPOINTS pts)
{
	//HitTest
	if(nHittest == HTBORDER)
	{
		//HTBOTTOM
		if(m_pCWndBottom && OnBorder(pts, HTBOTTOM))
		{
			 if(m_pCWndBottom->m_pCWndTop)
				 m_hCursorOld = SetCursor(m_hCursorTop);
		}
		//HTTOP
		else if(m_pCWndTop && OnBorder(pts, HTTOP))
		{
			 if(m_pCWndTop->m_pCWndBottom)
				 m_hCursorOld = SetCursor(m_hCursorTop);
		}
		//HTRIGHT
		else if(m_pCWndRight && OnBorder(pts, HTRIGHT))
		{
			if(m_pCWndRight->m_pCWndLeft)
				m_hCursorOld = SetCursor(m_hCursorRight);
		}
		//HTLEFT
		else if(m_pCWndLeft && OnBorder(pts, HTLEFT))
		{
			 if(m_pCWndLeft->m_pCWndRight)
				 m_hCursorOld = SetCursor(m_hCursorRight);
		}
	}
	else
	{
		//Restore the Cursor
		if(m_hCursorOld)
		{
			SetCursor(m_hCursorOld);
			m_hCursorOld = NULL;
		}
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CSplitterLite::OnNCButtonDown
//
/////////////////////////////////////////////////////////////////////
BOOL CSplitterLite::OnNCButtonDown(WPARAM nHittest, REFPOINTS ptsScreen)
{
	//WM_NCLBUTTONDOWN is in Screen Coordinates, relateive to the desktop.
	
	if(nHittest == HTBORDER)
	{
		POINT pt;

		//Is the Mouse on the Bottom Border?
		if(m_pCWndBottom && OnBorder(ptsScreen, HTBOTTOM))
		{
			//Create the "Bounding" Rectangle...
			RECT rectLeft	= GetClientCoords(m_hWndParent, m_hWnd);
			RECT rectRight	= GetClientCoords(m_hWndParent, m_pCWndBottom->m_hWnd);
			BOOL fVertical  = FALSE;

			//StartTracking...
			if(StartTracking(ptsScreen, rectLeft, rectRight, fVertical, &pt))
			{
				//MoveWindow is in Screen Coordinates.
				MoveWindow(m_hWnd, rectLeft.left, rectLeft.top, rectLeft.right-rectLeft.left, pt.y-rectLeft.top, FALSE);
				MoveWindow(m_pCWndBottom->m_hWnd, rectRight.left, pt.y, rectRight.right-rectRight.left, rectRight.bottom - pt.y, FALSE);

				if(m_pCWndRight && m_pCWndRight->m_pCWndBottom)
				{
					HWND hWndRight	= m_pCWndRight->m_hWnd;
					rectRight		= GetClientCoords(m_hWndParent, hWndRight);
					MoveWindow(hWndRight, rectRight.left, rectRight.top, rectRight.right-rectRight.left, pt.y - rectRight.top, FALSE);
				}

				if(m_pCWndBottom->m_pCWndRight && m_pCWndBottom->m_pCWndRight->m_pCWndTop)
				{
					HWND hWndRight	= m_pCWndBottom->m_pCWndRight->m_hWnd;
					rectRight		= GetClientCoords(m_hWndParent, hWndRight);
					MoveWindow(hWndRight, rectRight.left, pt.y, rectRight.right-rectRight.left, rectRight.bottom - pt.y, FALSE);
				}

				//Now invalidate all windows
				InvalidateRect(m_hWndParent, NULL, FALSE);
				return TRUE;
			}
		}

		//Is the Mouse on the Top Border?
		if(m_pCWndTop && OnBorder(ptsScreen, HTTOP))
		{
			//Create the "Bounding" Rectangle...
			RECT rectLeft	= GetClientCoords(m_hWndParent, m_pCWndTop->m_hWnd);
			RECT rectRight	= GetClientCoords(m_hWndParent, m_hWnd);
			BOOL fVertical  = FALSE;

			//StartTracking...
			if(StartTracking(ptsScreen, rectLeft, rectRight, fVertical, &pt))
			{
				//MoveWindow is in Screen Coordinates.
				MoveWindow(m_pCWndTop->m_hWnd, rectLeft.left, rectLeft.top, rectLeft.right-rectLeft.left, pt.y-rectLeft.top, FALSE);
				MoveWindow(m_hWnd, rectRight.left, pt.y, rectRight.right-rectRight.left, rectRight.bottom - pt.y, FALSE);

				if(m_pCWndRight && m_pCWndRight->m_pCWndTop)
				{
					HWND hWndRight	= m_pCWndRight->m_hWnd;
					rectRight		= GetClientCoords(m_hWndParent, hWndRight);
					MoveWindow(hWndRight, rectRight.left, pt.y, rectRight.right-rectRight.left, rectRight.bottom - pt.y, FALSE);
				}

				if(m_pCWndTop->m_pCWndRight && m_pCWndTop->m_pCWndRight->m_pCWndBottom)
				{
					HWND hWndRight	= m_pCWndTop->m_pCWndRight->m_hWnd;
					rectRight	= GetClientCoords(m_hWndParent, hWndRight);
					MoveWindow(hWndRight, rectRight.left, rectRight.top, rectRight.right-rectRight.left, pt.y - rectRight.top, FALSE);
				}

				//Now invalidate all windows
				InvalidateRect(m_hWndParent, NULL, FALSE);
				return TRUE;
			}
		}

		//Is the Mouse on the Left Border?
		if(m_pCWndLeft && OnBorder(ptsScreen, HTLEFT))
		{
			//Create the "Bounding" Rectangle...
			RECT rectLeft	= GetClientCoords(m_hWndParent, m_pCWndLeft->m_hWnd);
			RECT rectRight	= GetClientCoords(m_hWndParent, m_hWnd);
			BOOL fVertical  = TRUE;

			//StartTracking...
			if(StartTracking(ptsScreen, rectLeft, rectRight, fVertical, &pt))
			{
				//MoveWindow is in Screen Coordinates.
				MoveWindow(m_pCWndLeft->m_hWnd, rectLeft.left, rectLeft.top, pt.x - rectLeft.left, rectLeft.bottom - rectLeft.top, FALSE);
				MoveWindow(m_hWnd, pt.x, rectRight.top, rectRight.right-pt.x, rectRight.bottom - rectRight.top, FALSE);

				if(m_pCWndLeft->m_pCWndBottom)
				{
					HWND hWndBottom = m_pCWndLeft->m_pCWndBottom->m_hWnd;
					RECT rectBottom	= GetClientCoords(m_hWndParent, hWndBottom);
					MoveWindow(hWndBottom, rectBottom.left, rectBottom.top, pt.x-rectBottom.left, rectBottom.bottom-rectBottom.top, FALSE);
				}

				//Now invalidate all windows
				InvalidateRect(m_hWndParent, NULL, FALSE);
				return TRUE;
			}
		}

		//Is the Mouse on the Right Border?
		if(m_pCWndRight && OnBorder(ptsScreen, HTRIGHT) && m_pCWndRight->m_pCWndLeft)
		{
			//Create the "Bounding" Rectangle...
			RECT rectLeft	= GetClientCoords(m_hWndParent, m_hWnd);
			RECT rectRight	= GetClientCoords(m_hWndParent, m_pCWndRight->m_hWnd);
			BOOL fVertical  = TRUE;

			//StartTracking...
			if(StartTracking(ptsScreen, rectLeft, rectRight, fVertical, &pt))
			{
				//MoveWindow is in Screen Coordinates.
				MoveWindow(m_hWnd, rectLeft.left, rectLeft.top, pt.x - rectLeft.left, rectLeft.bottom - rectLeft.top, FALSE);
				MoveWindow(m_pCWndRight->m_hWnd, pt.x, rectRight.top, rectRight.right-pt.x, rectRight.bottom - rectRight.top, FALSE);

				if(m_pCWndTop)
				{
					HWND hWndTop = m_pCWndTop->m_hWnd;
					RECT rectTop = GetClientCoords(m_hWndParent, hWndTop);
					MoveWindow(hWndTop, rectTop.left, rectTop.top, pt.x-rectTop.left, rectTop.bottom-rectTop.top, FALSE);
				}

				if(m_pCWndBottom)
				{
					HWND hWndBottom = m_pCWndBottom->m_hWnd;
					RECT rectBottom	= GetClientCoords(m_hWndParent, hWndBottom);
					MoveWindow(hWndBottom, rectBottom.left, rectBottom.top, pt.x-rectBottom.left, rectBottom.bottom-rectBottom.top, FALSE);
				}
				
				//Now invalidate all windows
				InvalidateRect(m_hWndParent, NULL, FALSE);
				return TRUE;
			}
		}
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CSplitterLite::DrawSplitter
//
/////////////////////////////////////////////////////////////////////
BOOL CSplitterLite::DrawSplitter(HDC hDC, BOOL fVertical, REFRECT rectLeft, REFRECT rectRight, REFPOINTS ptsClient)
{
	if(fVertical)
	{
		//Vertical Splitter
		//PatBlt - in Client Coordinates
		DWORD dwTop		= min(rectLeft.top, rectRight.top);
		DWORD dwHeight  = max(rectLeft.bottom, rectRight.bottom) - dwTop;
		PatBlt(hDC, ptsClient.x, dwTop, m_iBorderCX, dwHeight, DSTINVERT);
	}
	else
	{
		//Horizontal Splitter
		//PatBlt - in Client Coordinates
		DWORD dwLeft	= min(rectLeft.left, rectRight.left);
		DWORD dwWidth	= max(rectLeft.right, rectRight.right) - dwLeft;
		PatBlt(hDC, dwLeft, ptsClient.y, dwWidth, m_iBorderCY, DSTINVERT);
	}
	return TRUE;
}



/////////////////////////////////////////////////////////////////////
// CSplitterLite::StartTracking
//
/////////////////////////////////////////////////////////////////////
BOOL CSplitterLite::StartTracking(REFPOINTS ptsScreen, REFRECT rectLeft, REFRECT rectRight, BOOL fVertical, POINT* pPoint)
{
	ASSERT(pPoint);
	BOOL bTracking = TRUE;
	MSG msgModal;

	//Capture mouse so Frame window gets mouse messages
	HDC hDC = GetDC(m_hWndParent);
	SetCapture(m_hWnd);  
	m_hCursorOld = SetCursor(fVertical ? m_hCursorRight : m_hCursorTop);
	BOOL fWithinBounds = FALSE;

	RECT rect;
	GetWindowRect(m_hWnd, &rect);
	POINT ptLocal = { rect.left, rect.top };
	ScreenToClient(m_hWndParent, &ptLocal);

	//Convert Screen Coordinates to Client Coordinates...
	POINT ptClient = { ptsScreen.x, ptsScreen.y};
	ScreenToClient(m_hWndParent, &ptClient);
	POINTS ptsClient = { (SHORT)ptClient.x, (SHORT)ptClient.y };

	//Draw splitter
	DrawSplitter(hDC, fVertical, rectLeft, rectRight, ptsClient);
			
	//While the split-bar is being dragged
	while (bTracking)
	{   
		//Get mouse message
		GetMessage(&msgModal, NULL, WM_MOUSEFIRST, WM_MOUSELAST);
		switch (msgModal.message)
		{
			case WM_MOUSEMOVE:
			{	
				//WM_MOUSEMOVE is in Client Coordindates, relateive to the client window.
				SHORT px = LOWORD(msgModal.lParam);
				SHORT py = HIWORD(msgModal.lParam);

				//Only redraw if different position than last time
				if((fVertical && px != ptsClient.x) || 
					(!fVertical && py != ptsClient.y))
				{
					//Draw splitter - erase previous
					DrawSplitter(hDC, fVertical, rectLeft, rectRight, ptsClient);

					//Get new mouse positions
					ptsClient.x = (SHORT)ptLocal.x + px;
					ptsClient.y = (SHORT)ptLocal.y + py;
					
					//Draw splitter - draw new
					DrawSplitter(hDC, fVertical, rectLeft, rectRight, ptsClient);
				}
				break;
			}

			// End of split-bar drag          
			case WM_LBUTTONUP:     
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONDBLCLK:
			{
				//WM_LBUTTONUP is in Client Coordindates, relateive to the client window.
				SHORT px = LOWORD(msgModal.lParam);
				SHORT py = HIWORD(msgModal.lParam);

				//Draw splitter - erase previous
				DrawSplitter(hDC, fVertical, rectLeft, rectRight, ptsClient);
				
				// Get new mouse positions
				ptsClient.x = (SHORT)ptLocal.x + px;
				ptsClient.y = (SHORT)ptLocal.y + py;
				
				//We Now need to resize the Window
				//Only adjust windows if Mouse is within Max/Min Coords
				if(fVertical)
					fWithinBounds = (ptsClient.x > rectLeft.left && ptsClient.x < rectRight.right);
				else
					fWithinBounds = (ptsClient.y > rectLeft.top && ptsClient.y < rectRight.bottom);
				
				bTracking = FALSE;  // Break out of tracking loop
				break;
			}
		}
	}

	ReleaseCapture();
	SetCursor(m_hCursorOld);
	ReleaseDC(m_hWnd, hDC);

	if(fWithinBounds)
	{
		pPoint->x = ptsClient.x;
		pPoint->y = ptsClient.y;
		return TRUE;
	}
								
	return FALSE;
}
			

/////////////////////////////////////////////////////////////////////
// CScrollBarLite::CScrollBarLite
//
/////////////////////////////////////////////////////////////////////
CScrollBarLite::CScrollBarLite()
{
}


/////////////////////////////////////////////////////////////////////
// CScrollBarLite::~CScrollBarLite
//
/////////////////////////////////////////////////////////////////////
CScrollBarLite::~CScrollBarLite()
{
}



////////////////////////////////////////////////////////////////
// CScrollBarLite::SetScroll
//
/////////////////////////////////////////////////////////////////
INT	CScrollBarLite::SetScroll(INT lPos, BOOL bRedraw)
{
	//This function is a "superset" of the SetScrollPos, but it takes into
	//acount the caller may not know the boundary of the scrollbar, and adjusts...
	
	//First obtain the scroll bar ranges...
	INT nMin, nMax;
	if(GetScrollRange(&nMin, &nMax))
	{
		lPos = min(lPos, nMax);
		lPos = max(lPos, nMin);

		//Set the new range
		return SetScrollPos(lPos, bRedraw);
	}
	
	return 0;
}


//////////////////////////////////////////////////////////////////
// CScrollBarLite::SetScrollInfo
//
//////////////////////////////////////////////////////////////////
INT CScrollBarLite::SetScrollInfo(INT iPos, INT iRangeSize, INT iPageSize, BOOL fRedraw)
{
	SCROLLINFO ScrollInfo = { sizeof(SCROLLINFO), SIF_ALL, 0, iRangeSize, iPageSize, iPos, 0};
	
	//SetScrollInfo
	return ::SetScrollInfo(m_hWnd, SB_CTL, &ScrollInfo, fRedraw);
}


//////////////////////////////////////////////////////////////////
// CScrollBarLite::GetScrollInfo
//
//////////////////////////////////////////////////////////////////
BOOL CScrollBarLite::GetScrollInfo(INT* piPos, INT* piRangeSize, INT* piPageSize)
{
	INT iPos = 0;
	INT iRangeSize = 0;
	INT iPageSize = 0;

	SCROLLINFO ScrollInfo = { sizeof(SCROLLINFO), SIF_ALL, 0, iRangeSize, iPageSize, iPos, 0};
	
	//SetScrollInfo
	BOOL bResult = ::GetScrollInfo(m_hWnd, SB_CTL, &ScrollInfo);

	//Fill in arguments
	if(piPos)
		*piPos = iPos;
	if(piRangeSize)
		*piRangeSize = iRangeSize;
	if(piPageSize)
		*piPageSize = iPageSize;

	return bResult;
}



/////////////////////////////////////////////////////////////////////
// CEditBoxLite::CEditBoxLite
//
/////////////////////////////////////////////////////////////////////
CEditBoxLite::CEditBoxLite(HWND hWndParent, UINT nID)
	: CSplitterLite(hWndParent, nID)
{
}


/////////////////////////////////////////////////////////////////////
// CEditBoxLite::~CEditBoxLite
//
/////////////////////////////////////////////////////////////////////
CEditBoxLite::~CEditBoxLite()
{
}


/////////////////////////////////////////////////////////////////////
// CEditBoxLite::OnCreate
//
/////////////////////////////////////////////////////////////////////
BOOL CEditBoxLite::OnCreate(CREATESTRUCT* pCREATESTRUCT)
{
	//SubClass the window
	SubClassWindow(WndProc);
	
	//Delegate
	return CSplitterLite::OnCreate(pCREATESTRUCT);
}


/////////////////////////////////////////////////////////////////////
// CEditBoxLite::GetTextRange
//
/////////////////////////////////////////////////////////////////////
INDEX CEditBoxLite::GetTextRange(CHARRANGE& cr, WCHAR* pwszBuffer)
{
	//With EM_GETTEXTRANGE (Since their is not a W-unicode and A-ansi version), the message
	//is completly based upon how you created the window.  Since we currently only
	//call this from CreateIndirect (ANSI) we just assume this...
	if(m_bUnicodeMsg)
	{
		TEXTRANGEW tr; 
		tr.chrg = cr; 
		tr.lpstrText = pwszBuffer; 
		
		return (INDEX)::SendMessage(m_hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	}
	else
	{
		CHAR szBuffer[MAX_QUERY_LEN] = {0};

		TEXTRANGEA tr; 
		tr.chrg = cr; 
		tr.lpstrText = szBuffer; 

		INDEX lReturn = (INDEX)::SendMessage(m_hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
		ConvertToWCHAR(szBuffer, pwszBuffer, cr.cpMax - cr.cpMin);
		return lReturn;
	}
}


/////////////////////////////////////////////////////////////////////
// CEditBoxLite::GetTextRange
//
/////////////////////////////////////////////////////////////////////
INDEX CEditBoxLite::GetTextRange(CHARRANGE& cr, CHAR* pszBuffer)
{
	//With EM_GETTEXTRANGE (Since their is not a W-unicode and A-ansi version), the message
	//is completly based upon how you created the window.  Since we currently only
	//call this from CreateIndirect (ANSI) we just assume this...
	if(m_bUnicodeMsg)
	{
		WCHAR wszBuffer[MAX_QUERY_LEN] = {0};

		TEXTRANGEW tr; 
		tr.chrg = cr; 
		tr.lpstrText = wszBuffer; 

		INDEX lReturn = (INDEX)::SendMessage(m_hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
		ConvertToMBCS(wszBuffer, pszBuffer, cr.cpMax - cr.cpMin);
		return lReturn;
	}
	else
	{
		TEXTRANGEA tr; 
		tr.chrg = cr; 
		tr.lpstrText = pszBuffer; 
		
		return (INDEX)::SendMessage(m_hWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
	}
}


/////////////////////////////////////////////////////////////////////
// CEditBoxLite::GetLine
//
/////////////////////////////////////////////////////////////////////
INDEX CEditBoxLite::GetLine(INDEX iLine, CHAR* pszBuffer, ULONG ulMaxSize)
{
	ASSERT(pszBuffer && ulMaxSize);
	
	//Get the Text for this Line
	//EM_GETLINE assumes the first byte of the buffer
	//indicates the total size of the buffer
	((WORD*)pszBuffer)[0] = (WORD)(ulMaxSize-1);
	INDEX iChars = (INDEX)SendMessage(m_hWnd, EM_GETLINE, iLine, (LPARAM)pszBuffer);

	//Supposedly EM_GETLINE doesn't contain a NULL Terminator?!
	pszBuffer[min((ULONG)iChars, ulMaxSize)] = EOL;
	return iChars;
}

		
/////////////////////////////////////////////////////////////////////
// CEditBoxLite::Save
//
/////////////////////////////////////////////////////////////////////
BOOL CEditBoxLite::Save(LPCWSTR pwszFileName)
{
	CFileLite cFile;
	CHAR szBuffer[MAX_QUERY_LEN];
	
	//Open the file, (prompt if error)
	if(FAILED(cFile.Open(pwszFileName, GENERIC_WRITE, 0, CREATE_ALWAYS, TRUE)))
		return FALSE;

	//Loop over all lines in the RichEdit control and write to the file...
	//Copy the contents of this ListBox to the destination...
	INDEX iCount = GetLineCount();
	for(INDEX i=0; i<iCount; i++)
	{
		//Get this line from the Control
		if(!GetLine(i, szBuffer, MAX_NAME_LEN))
			break;

		//Blast it to the file...
		cFile.Write(szBuffer);
		cFile.Write(wWndEOL);
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CEditBoxLite::ReplaceSelAll
//
/////////////////////////////////////////////////////////////////////
void CEditBoxLite::ReplaceAll(LPCWSTR pwszString, bool bReplaceAll, bool bHighlight)
{
	if(pwszString)
	{
		INDEX iStartPos = 0;
		INDEX iStartPos2 = 0;
		INDEX iEndPos = 0;
		INDEX iEndPos2 = 0;

		if(bReplaceAll)
		{
			//Replace the entire text?
			SetSel(0, -1);
		}
		else
		{
			//Obtain the Selected Text Start and End Positions
			GetSel(&iStartPos, &iEndPos);
		}

		//Replace the Selection
		ReplaceSel(pwszString, TRUE/*fCanUndo*/);
		
		//Make sure the replacement is visible...
		ScrollCaret();

		if(bHighlight)
		{
			if(bReplaceAll)
			{
				//Highlight all the text...
				SetSel(0, -1);
			}
			else
			{
				//Highlight the text we used...
				GetSel(&iStartPos2, &iEndPos2);
				SetSel(iStartPos, iEndPos2);
			}
		}
	}
}



/////////////////////////////////////////////////////////////////////
// CXmlDOM::CXmlDOM
//
/////////////////////////////////////////////////////////////////////
CXmlDOM::CXmlDOM()
{
}


/////////////////////////////////////////////////////////////////////
// CXmlDOM::~CXmlDOM
//
/////////////////////////////////////////////////////////////////////
CXmlDOM::~CXmlDOM()
{
}


/////////////////////////////////////////////////////////////////////
// CXmlDOM::PrintNode
//
/////////////////////////////////////////////////////////////////////
HRESULT CXmlDOM::PrintNode(IXMLDOMNode* spNode, CConsole& rConsole, ULONG ullevel)
{
	HRESULT hr = S_OK;
	CComBSTR				bstrName;
	CComVariant				varValue;

	CComPtr<IXMLDOMNamedNodeMap>	spAttrs;
	CComPtr<IXMLDOMNode>			spChild;

	//Write out this node
	if(spNode)
	{
		//Output: Indentation Level
		for(ULONG i=0; i<ullevel; i++)
			rConsole << L"    ";

		//Node Name
		TESTC(hr = spNode->get_nodeName(&bstrName));
		
		//Node type
		DOMNodeType nodeType;
		TESTC(hr = spNode->get_nodeType(&nodeType));
		
		//Node Value
		TESTC(hr = spNode->get_nodeValue(&varValue));
		varValue.ChangeType(VT_BSTR);

		switch(nodeType)
		{
			case NODE_PROCESSING_INSTRUCTION:
				rConsole << RGB_BLUE << L"<?" << RGB_NONE;
				rConsole << RGB_BLUE  << bstrName << RGB_NONE;

				//Output: Attributes
				PrintAttributes(spNode, rConsole);

				//Output: >
				rConsole << RGB_BLUE << L" ?>" << RGB_NONE;
				rConsole.WriteLine();
				break;

			case NODE_DOCUMENT_TYPE:
			{	
				//<!DOCTYPE root (View Source for full doctype...)> 
				rConsole << RGB_BLUE << L"<!DOCTYPE " << bstrName;
				rConsole << CA_ITALIC << L" (View Source for full doctype...)" << CA_NORMAL;
				rConsole.WriteLine();
				break;
			}
			
			case NODE_TEXT:
				//This is the text of a node, not an element (ie: doesn't need tags)
				rConsole << CA_BOLD << V_BSTR(&varValue) << CA_NORMAL;
				rConsole.WriteLine();
				break;

			default:
				ASSERT(!"Unhandled Node Type?");
				//FALL THROUGH TO DISPLAY UNKNOWN ELEMENT

			case NODE_ELEMENT:
			case NODE_DOCUMENT:
				//Output: <ElementName
				rConsole << RGB_BLUE << L"<" << RGB_NONE;
				rConsole << RGB_DARK_RED  << bstrName << RGB_NONE;
				
				//Output: Attributes
				PrintAttributes(spNode, rConsole);
				
				//Output: Value
				rConsole << CA_BOLD << V_BSTR(&varValue) << CA_NORMAL;

				//Output: >
				rConsole << RGB_BLUE << L">" << RGB_NONE;
				rConsole.WriteLine();
				break;

		}
				
		//Now recurse into any children...
		TESTC(hr = spNode->get_firstChild(&spChild));
		while(spChild)
		{
			CComPtr<IXMLDOMNode>	spNext;
			
			//Recurse for all siblings...
			TESTC(hr = PrintNode(spChild, rConsole, ullevel+1));
			TESTC(hr = spChild->get_nextSibling(&spNext));
			
			spChild.Attach(spNext);
			spNext.Detach();
		}
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////
// CXmlDOM::PrintAttributes
//
/////////////////////////////////////////////////////////////////////
HRESULT CXmlDOM::PrintAttributes(IXMLDOMNode* spNode, CConsole& rConsole)
{
	HRESULT hr = S_OK;
	CComBSTR				bstrName;
	CComVariant				varValue;

	CComPtr<IXMLDOMNamedNodeMap>	spAttrs;
	CComPtr<IXMLDOMNode>			spChild;

	//Write out the Attributes
	if(spNode)
	{
		//Write out all attributes
		TESTC(hr = spNode->get_attributes(&spAttrs));
		if(spAttrs)
		{
			TESTC(hr = spAttrs->nextNode(&spChild));
			while(spChild)
			{
				//Attribute name
				TESTC(hr = spChild->get_nodeName(&bstrName));
				
				//Attribute Value
				TESTC(hr = spChild->get_nodeValue(&varValue));
				varValue.ChangeType(VT_BSTR);

				//Node type
				DOMNodeType nodeType;
				TESTC(hr = spNode->get_nodeType(&nodeType));
				
				switch(nodeType)
				{
					case NODE_PROCESSING_INSTRUCTION:
						//Output: AttributeName="
						rConsole << RGB_BLUE;
						rConsole << L" " << bstrName; 
						rConsole << L"=\"";

						//Output: AttributeValue
						rConsole << V_BSTR(&varValue);
   
						//Output: "
						rConsole << L"\"" << RGB_NONE;
						break;


					default:
						ASSERT(!"Unhandled Node Type?");
						//FALL THROUGH TO DISPLAY UNKNOWN ELEMENT

					case NODE_ELEMENT:
						//Output: AttributeName="
						rConsole << RGB_DARK_RED;
						rConsole << L" " << bstrName; 
						rConsole << RGB_BLUE << L"=\"";

						//Output: AttributeValue
						rConsole << RGB_NONE << CA_BOLD << V_BSTR(&varValue);
   
						//Output: "
						rConsole << RGB_BLUE << CA_NORMAL << L"\"" << RGB_NONE;
						break;
				};

				bstrName.Empty();
				varValue.Clear();

				//Next Attribute
				spChild.Release();
				TESTC(hr = spAttrs->nextNode(&spChild));
			}
		}
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////
// CRichEditLite::CRichEditLite
//
/////////////////////////////////////////////////////////////////////
CRichEditLite::CRichEditLite(HWND hWndParent, UINT nID)
	: CEditBoxLite(hWndParent, nID)
{
}


/////////////////////////////////////////////////////////////////////
// CRichEditLite::~CRichEditLite
//
/////////////////////////////////////////////////////////////////////
CRichEditLite::~CRichEditLite()
{
}


/////////////////////////////////////////////////////////////////////
// CRichEditLite::OnCreate
//
/////////////////////////////////////////////////////////////////////
BOOL CRichEditLite::OnCreate(CREATESTRUCT* pCREATESTRUCT)
{
	//URL detection
	SendMessage(m_hWnd, EM_AUTOURLDETECT, TRUE, 0);
	SendMessage(m_hWnd, EM_SETEVENTMASK, 0, ENM_LINK);

	//Delegate
	return CEditBoxLite::OnCreate(pCREATESTRUCT);
}


/////////////////////////////////////////////////////////////////////
// CRichEditLite::SetWordWrap
//
/////////////////////////////////////////////////////////////////////
BOOL CRichEditLite::SetWordWrap(BOOL fWordWrap)
{
	//WordWrap
	// 0 - Wrap to window
	// 1 - Do not wrap lines
	return (BOOL)SendMessage(m_hWnd, EM_SETTARGETDEVICE, (WPARAM)NULL, fWordWrap ? 0 : 1); 
}


/////////////////////////////////////////////////////////////////////
// CRichEditLite::SetDefaultFont
//
/////////////////////////////////////////////////////////////////////
LRESULT CRichEditLite::SetDefaultFont()
{
	//Default Font
	//If you wise not to replace the font with this default, then just
	//set cbSize = 0 before calling ::Create.
	CHARFORMATW charFormat;
	memset(&charFormat, 0, sizeof(CHARFORMATW));
	charFormat.cbSize		= sizeof(CHARFORMATW);
	charFormat.dwMask		= CFM_SIZE | CFM_COLOR | CFM_FACE | CFM_OFFSET | CFM_CHARSET | CFM_LINK | CFM_PROTECTED | CFM_STRIKEOUT | CFM_UNDERLINE | CFM_ITALIC | CFM_BOLD;
	charFormat.dwEffects	= CFE_AUTOCOLOR;
	charFormat.yHeight		= 0x60;
	StringCopy(charFormat.szFaceName, L"MS Sans Serif", LF_FACESIZE);

	//By default the font for RichEdit controls is HUGE!  By default we will replace
	//the font with the default for Edit Controls which is Sans-Serif size 8...
	if(charFormat.cbSize)
		return SetCharFormat(SCF_ALL, charFormat);

	return 0;
}

	
/////////////////////////////////////////////////////////////////////
// CRichEditLite::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CRichEditLite::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	switch(pNMHDR->code)
	{
		ON_COMMAND(EN_LINK, OnLink(idCtrl, (ENLINK*)pNMHDR));
	};

	//Delegate
	return CEditBoxLite::OnNotify(idCtrl, pNMHDR);
}


/////////////////////////////////////////////////////////////////////
// CRichEditLite::OnLink
//
/////////////////////////////////////////////////////////////////////
BOOL CRichEditLite::OnLink(INT idCtrl, ENLINK* pEnLink)
{
	//Default Implementation of OnLink, of actually executing the link...
	if(pEnLink)
	{
		switch(pEnLink->msg)
		{
			case WM_LBUTTONDOWN:
			{
				//Obtain the link text.
				if(pEnLink->chrg.cpMax - pEnLink->chrg.cpMin <= MAX_QUERY_LEN)
				{
					CWaitCursor waitCursor;
					if(IsUnicodeOS())
					{
						WCHAR wszLink[MAX_QUERY_LEN] = {0};
						GetTextRange(pEnLink->chrg, wszLink);

						//ShellExecute returns a value greater than 32 is successful (nice design!)
						//NOTE: ShellExecute returns a HINSTANCE, (for backard compatibility), but really this is a LONG
						return (LONG_PTR)ShellExecuteW(m_hWnd, L"open", wszLink, NULL, NULL, SW_SHOWNORMAL) > 32;
					}
					else
					{
						CHAR szLink[MAX_QUERY_LEN] = {0};
						GetTextRange(pEnLink->chrg, szLink);

						//ShellExecute returns a value greater than 32 is successful (nice design!)
						//NOTE: ShellExecute returns a HINSTANCE, (for backard compatibility), but really this is a LONG
						return (LONG_PTR)ShellExecuteA(m_hWnd, "open", szLink, NULL, NULL, SW_SHOWNORMAL) > 32;
					}
				}
			}
		};
	}
	
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CRichEditLite::ReplaceSel
//
/////////////////////////////////////////////////////////////////////
void CRichEditLite::ReplaceSel(LPCWSTR pwszString, bool bCanUndo, DWORD dwMask, DWORD dwColor)
{
	//Change to bold and/or color
	if(dwMask)
		SetEffects(dwMask, dwMask & CFM_BOLD ? CFE_BOLD : 0, dwColor);
	
	//Delegate
	ReplaceSel(pwszString, bCanUndo);

	//Change back
	if(dwMask)
		SetEffects(dwMask, CFE_AUTOCOLOR, 0);
}

	
/////////////////////////////////////////////////////////////////////
// CRichEditLite::Append
//
/////////////////////////////////////////////////////////////////////
void CRichEditLite::Append(LPCWSTR pwszString, bool bCanUndo, DWORD dwMask, DWORD dwColor)
{
	//Move the position to the end
	SetSel(LONG_MAX, LONG_MAX);
	
	//Delegate
	ReplaceSel(pwszString, bCanUndo, dwMask, dwColor);
}


/////////////////////////////////////////////////////////////////////
// CRichEditLite::GetSelectedText
//
/////////////////////////////////////////////////////////////////////
WCHAR* CRichEditLite::GetSelectedText(BOOL fEntireLine)
{
	INDEX cpMin = 0;
	INDEX cpMax = 0;
	WCHAR* pwsz = NULL;
	HRESULT hr = S_OK;

	//Obtain the EditBox text (rules below).
	//  1.  First try to obtain whatever text is highlighted (even multiline highlight)
	//  2.  Or if nothing is highlighted take the entire line of the cursor

	//Obtain the Selected Text Start and End Positions
	GetSel(&cpMin, &cpMax);

	//If there is no selected Text
	//Just get the entire line, as if it were highlighted...
	if(cpMin == cpMax)
	{
		//Obtain the first character of the selected line...
		INDEX iCharIndex = LineIndex(-1);
		INDEX iLineLength = LineLength(cpMin);

		if(fEntireLine)
		{
			cpMin = (LONG)iCharIndex;
			cpMax = (LONG)(iCharIndex + iLineLength);
		}

		//Obtain the current word, if nothing is selected...
		if(cpMin == cpMax)
		{	
			//TODO
/*			//Find the begining of the whole word that the cursor is over...
			while(cpMin > iCharIndex)
			{
				if(!iswalnum(pwsz[cpMin - iCharIndex - 1]))
					break;
				cpMin--;
			}

			//Find the ending of the whole word that the cursor is over...
			while(cpMax < (iCharIndex + iLineLength))
			{
				if(!iswalnum(pwsz[cpMax - iCharIndex]))
					break;
				cpMax++;
			}
*/		}

		//Highlight the text we used...
		SetSel(cpMin, cpMax);
	}
	
	//Now that we have a selection...
	ASSERT(cpMax>=cpMin && cpMax>=0 && cpMin>=0);

	//Simple, just obtain the selected text...
	INDEX iMaxChars = (cpMax-cpMin+1)+1;
		
	//Allocate a buffer large enough to hold all the text...
	//NOTE:  We pass in the length to wSendMessage so it knows the conversion length...
	SAFE_ALLOC(pwsz, WCHAR, iMaxChars);
	wSendMessage(m_hWnd, EM_GETSELTEXT, iMaxChars, pwsz);

CLEANUP:
	return pwsz;
}




/////////////////////////////////////////////////////////////////////
// CRichEditLite::SetEffects
//
/////////////////////////////////////////////////////////////////////
BOOL CRichEditLite::SetEffects(DWORD dwMask, DWORD	dwEffects, COLORREF	crTextColor)
{
	CHARFORMATW charFormat;
	charFormat.cbSize		= sizeof(CHARFORMATW);
	charFormat.dwMask		= dwMask;
	charFormat.dwEffects	= dwEffects;
	charFormat.crTextColor	= crTextColor;
	return SetCharFormat(SCF_SELECTION, charFormat);
}


/////////////////////////////////////////////////////////////////////
// CRichEditLite::SetCharFormat
//
/////////////////////////////////////////////////////////////////////
BOOL CRichEditLite::SetCharFormat(DWORD dwMask, CHARFORMATW& charFormatW)
{
	//Unfortunatly the RichEdit20W requires a CHARFORMATW structure (size) to be passed in
	//and will ignore the CHARFORMATA size, and the same is true for RichEdit20A with CHARFORMATA.
	//This is a unique structure in that this is not a pointer to a string szFaceName, but the 
	//string buffer is a part of the structure, making cbSize different.  Even though we don't 
	//have the maks indicating the face name is valid, it still does validation and will ignore
	//any mismatched size with the use of the control...

	if(IsUnicodeOS())
	{
		return (BOOL)SendMessageW(m_hWnd, EM_SETCHARFORMAT, dwMask, (LPARAM)&charFormatW);
	}
	else
	{
		//Convert the CHARFORMATW to CHARFORMATA
		CHARFORMATA charFormatA;
		memcpy(&charFormatA, &charFormatW, sizeof(CHARFORMATA));
		charFormatA.cbSize = sizeof(CHARFORMATA);
		
		//Convert the Unicode inline string to MBCS
		ConvertToMBCS(charFormatW.szFaceName, charFormatA.szFaceName, LF_FACESIZE);
		return (BOOL)SendMessageA(m_hWnd, EM_SETCHARFORMAT, dwMask, (LPARAM)&charFormatA);
	}
}




/////////////////////////////////////////////////////////////////////
// CRichEditLite::FindText
//
/////////////////////////////////////////////////////////////////////
INDEX CRichEditLite::FindText(DWORD dwFindFlags, FINDTEXTEXW* pFindTextW)
{
	INDEX lPos = -1;

	if(IsUnicodeOS())
	{
		//EM_FINDTEXT
		lPos = SendMessageW(m_hWnd, EM_FINDTEXTEXW, dwFindFlags, (LPARAM)pFindTextW);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pFindTextW->lpstrText, szBuffer, MAX_NAME_LEN);
		
		//Input Args
		FINDTEXTEXA FindTextA;
		FindTextA.chrg			= pFindTextW->chrg;
		FindTextA.lpstrText		= szBuffer;
		
		//EM_FINDTEXT
		lPos = SendMessageA(m_hWnd, EM_FINDTEXTEX, dwFindFlags, (LPARAM)&FindTextA);

		//Output Params
		pFindTextW->chrgText	= FindTextA.chrgText;
	}
	
	return lPos;
}


/////////////////////////////////////////////////////////////////////////////
// CRichEditLite::FindNext
//
/////////////////////////////////////////////////////////////////////////////
BOOL CRichEditLite::FindNext(DWORD dwFindFlags, WCHAR* pwszText)
{
	//Obtain the Current Position of the 'caret'
	CHARRANGE chrg;
	GetSel(&chrg);
	BOOL fForward = (dwFindFlags & FR_DOWN);

	//This now is the "next" or "previous" positions to search from...
	//NOTE: We have to use "Min" on backward searching otherwise it will continue to find 
	//the selected word over and over...
	chrg.cpMin	= fForward ? chrg.cpMax : chrg.cpMin;
	chrg.cpMax  = fForward ? LONG_MAX	: 0;

	FINDTEXTEXW findTextEx;
	findTextEx.chrg			= chrg;
	findTextEx.lpstrText	= pwszText;
	
	//First search from the point specified to the bottom (if forward)
	INDEX iPos = FindText(dwFindFlags, &findTextEx);
	if(iPos == -1)
	{
		//If not found, then switch to start from the begining (if forward) to the org starting point.
		findTextEx.chrg.cpMin  = fForward ? 0 : LONG_MAX;
		findTextEx.chrg.cpMax  = chrg.cpMax;

		//Search
		iPos = FindText(dwFindFlags, &findTextEx);
	}

	if(iPos != -1)
	{								 
		//Found a match, Select it...
		SetSel(findTextEx.chrgText.cpMin, findTextEx.chrgText.cpMax);
		return TRUE;
	}

	//Otherwise, not found
	return FALSE;
}

	

/////////////////////////////////////////////////////////////////////
// CListBoxLite::CListBoxLite
//
/////////////////////////////////////////////////////////////////////
CListBoxLite::CListBoxLite(HWND hWndParent, UINT nID)
	: CWndLite(hWndParent, nID)
{
}


/////////////////////////////////////////////////////////////////////
// CListBoxLite::~CListBoxLite
//
/////////////////////////////////////////////////////////////////////
CListBoxLite::~CListBoxLite()
{
}


/////////////////////////////////////////////////////////////////////
// CListBoxLite::OnCommandNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CListBoxLite::OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl) 
{
	switch(wNotifyCode)
	{
		case LBN_DBLCLK:
		{	
			if(OnDblClk(iID, hWndCtrl))
				return TRUE;
			break;
		}
	};

	//Otherwise Delegate
	return CWndLite::OnCommandNotify(wNotifyCode, iID, hWndCtrl);
}


/////////////////////////////////////////////////////////////////////
// CListBoxLite::OnDblClk
//
/////////////////////////////////////////////////////////////////////
BOOL CListBoxLite::OnDblClk(INT iID, HWND hWndCtrl)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::CListViewLite
//
/////////////////////////////////////////////////////////////////////
CListViewLite::CListViewLite(HWND hWndParent, UINT nID)
	: CSplitterLite(hWndParent, nID)
{
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::~CListViewLite
//
/////////////////////////////////////////////////////////////////////
CListViewLite::~CListViewLite()
{
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::OnCreate
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::OnCreate(CREATESTRUCT* pCREATESTRUCT)
{
	//SubClass the window
	SubClassWindow(WndProc);
	
	//Delegate
	return CSplitterLite::OnCreate(pCREATESTRUCT);
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	switch(pNMHDR->code)
	{
		ON_COMMAND(LVN_COLUMNCLICK,		OnColumnClick(idCtrl, (NMLISTVIEW*)pNMHDR));
		ON_COMMAND(LVN_ITEMACTIVATE,	OnItemActivate(idCtrl, (NMLISTVIEW*)pNMHDR));
		ON_COMMAND(LVN_ITEMCHANGED,		OnItemChanged(idCtrl, (NMLISTVIEW*)pNMHDR));
	};

	//Delegate
	return CSplitterLite::OnNotify(idCtrl, pNMHDR);
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::OnColumnClick
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::OnColumnClick(INT idCtrl, NMLISTVIEW* pNMListView)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::OnItemActivate
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::OnItemActivate(INT idCtrl, NMLISTVIEW* pNMListView)
{
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// CListViewLite::OnItemChanged
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::OnItemChanged(INT idCtrl, NMLISTVIEW* pNMListView)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::InsertItem
//
/////////////////////////////////////////////////////////////////////
INDEX CListViewLite::InsertItem(INDEX iItem, INDEX iSubItem, WCHAR* wszName, LPARAM lParam, INT iImage, UINT iState, UINT iStateMask)
{							   	
	return LV_InsertItem(m_hWnd, iItem, iSubItem, wszName, lParam, iImage, iState, iStateMask);
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::SetItemText
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::SetItemText(INDEX iItem, INDEX iSubItem, WCHAR* pwszName)
{							   	
	return LV_SetItemText(m_hWnd, iItem, iSubItem, pwszName);
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::SetItemState
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::SetItemState(INDEX iItem, INDEX iSubItem, UINT iState, UINT iStateMask)
{							   	
	return LV_SetItemState(m_hWnd, iItem, iSubItem, iState, iStateMask);
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::GetItemImage
//
/////////////////////////////////////////////////////////////////////
INT CListViewLite::GetItemImage(INDEX iItem, INDEX iSubItem)
{							   	
	return LV_GetItemImage(m_hWnd, iItem, iSubItem);
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::SetItemImage
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::SetItemImage(INDEX iItem, INDEX iSubItem, INT iImage)
{							   	
	return LV_SetItemImage(m_hWnd, iItem, iSubItem, iImage);
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::SetItemParam
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::SetItemParam(INDEX iItem, INDEX iSubItem, LPARAM lParam)
{							   	
	return LV_SetItemParam(m_hWnd, iItem, iSubItem, lParam);
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::SetColumn
//
/////////////////////////////////////////////////////////////////////
BOOL CListViewLite::SetColumn(INDEX iColumn, WCHAR* wszName, INT iImage)
{							   	
	ULONG dwMask = LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM;
	INT dwFmt = LVCFMT_LEFT;
	if(iImage != IMAGE_NONE)
	{
		dwMask |= LVCF_IMAGE;
		dwFmt |= LVCFMT_IMAGE;
	}
	
	//LVM_SETCOLUMN
	if(IsUnicodeOS())
	{
		//Setup LVM_SETCOLUMN
		LV_COLUMNW lvColumnHeader = { dwMask, dwFmt, 0, wszName, 0, 0, iImage, 0};
		return (BOOL)SendMessage(m_hWnd, LVM_SETCOLUMNW, (WPARAM)iColumn, (LPARAM)&lvColumnHeader);
	}
	else
	{
		//Setup LVM_SETCOLUMN
		CHAR szBuffer[MAX_NAME_LEN];
		LV_COLUMNA lvColumnHeader = { dwMask, dwFmt, 0, szBuffer, 0, 0, iImage, 0};
		ConvertToMBCS(wszName, szBuffer, MAX_NAME_LEN);
		return (BOOL)SendMessage(m_hWnd, LVM_SETCOLUMNA, (WPARAM)iColumn, (LPARAM)&lvColumnHeader);
	}
}


/////////////////////////////////////////////////////////////////////
// CListViewLite::InsertColumn
//
/////////////////////////////////////////////////////////////////////
INDEX CListViewLite::InsertColumn(INDEX iColumn, WCHAR* wszName, INT iImage)
{							   	
	return LV_InsertColumn(m_hWnd, iColumn, wszName, iImage);
}

/////////////////////////////////////////////////////////////////////
// CListViewLite::GetItemParam
//
/////////////////////////////////////////////////////////////////////
LPARAM CListViewLite::GetItemParam(INDEX iItem)
{							   	
	return LV_GetItemParam(m_hWnd, iItem, 0);
}

/////////////////////////////////////////////////////////////////////
// CListViewLite::GetItemText
//
/////////////////////////////////////////////////////////////////////
INDEX CListViewLite::GetItemText(INDEX iItem, INDEX iSubItem, WCHAR* wszName, ULONG ulMaxChars)
{
	return LV_GetItemText(m_hWnd, iItem, iSubItem, wszName, ulMaxChars);
}

	
/////////////////////////////////////////////////////////////////////
// CListViewLite::HitTest
//
/////////////////////////////////////////////////////////////////////
INDEX CListViewLite::HitTest(REFPOINTS pts, UINT* pFlags, BOOL fClientCoords)
{
	LVHITTESTINFO lvHitInfo = {{pts.x, pts.y}, 0, 0};
	if(!fClientCoords)
		ScreenToClient(m_hWnd, &lvHitInfo.pt);

	//LVM_HITTEST
	INDEX iItem = (INDEX)SendMessage(m_hWnd, LVM_HITTEST, 0, (LPARAM)&lvHitInfo);
	if(pFlags)
		*pFlags = lvHitInfo.flags;

	return iItem;
}


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::CTreeViewLite
//
/////////////////////////////////////////////////////////////////////
CTreeViewLite::CTreeViewLite(HWND hWndParent, UINT nID)
	: CSplitterLite(hWndParent, nID)
{
	m_hDraggingItem = NULL;
}


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::~CTreeViewLite
//
/////////////////////////////////////////////////////////////////////
CTreeViewLite::~CTreeViewLite()
{
}


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::OnCreate
//
/////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::OnCreate(CREATESTRUCT* pCREATESTRUCT)
{
	//SubClass the window
	SubClassWindow(WndProc);
	
	//Delegate
	return CSplitterLite::OnCreate(pCREATESTRUCT);
}


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	switch(pNMHDR->code)
	{
		ON_COMMAND(TVN_SELCHANGED,		OnSelChanged(idCtrl, (NMTREEVIEW*)pNMHDR));
		ON_COMMAND(TVN_BEGINLABELEDIT,	OnBeginLabelEdit(idCtrl, (NMTVDISPINFO*)pNMHDR));
		ON_COMMAND(TVN_ENDLABELEDIT,	OnEndLabelEdit(idCtrl, (NMTVDISPINFO*)pNMHDR));
		ON_COMMAND(TVN_BEGINDRAG,		OnBeginDrag(idCtrl, (NMTREEVIEW*)pNMHDR));
	};

	//Delegate
	return CSplitterLite::OnNotify(idCtrl, pNMHDR);
}


/////////////////////////////////////////////////////////////////////////////
// CTreeViewLite::OnSelChanged
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::OnSelChanged(INT idCtrl, NMTREEVIEW* pNMTreeView) 
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CTreeViewLite::OnBeginLabelEdit
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::OnBeginLabelEdit(INT idCtrl, NMTVDISPINFO* ptvdi) 
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CTreeViewLite::OnEndLabelEdit
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::OnEndLabelEdit(INT idCtrl, NMTVDISPINFO* ptvdi) 
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CTreeViewLite::OnBeginDrag
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::OnBeginDrag(INT idCtrl, NMTREEVIEW* pNMTreeView)
{
    //Tell the TreeView control to create an image to use for dragging. 
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
    HIMAGELIST hImageList = TreeView_CreateDragImage(m_hWnd, hItem);  
   
	RECT rect;
	TreeView_GetItemRect(m_hWnd, pNMTreeView->itemNew.hItem, &rect, FALSE);
	m_hDraggingItem = hItem;

	//Start the drag operation.
	ULONG level = 0;

	do 
	{ 
		hItem = TreeView_GetParent(m_hWnd, hItem); 
		level++; 
	} 
	while(hItem);   
	
	LONG xIndent = TreeView_GetIndent( m_hWnd ) * level;
    ImageList_BeginDrag(hImageList, 0,  pNMTreeView->ptDrag.x - rect.left - xIndent,  pNMTreeView->ptDrag.y - rect.top); 

	//Hide the mouse cursor, and direct mouse input to the parent window.  
    ShowCursor( FALSE ); 
	SetCapture(m_hWnd);

	ImageList_DragEnter(m_hWnd, pNMTreeView->ptDrag.x - rect.left - xIndent, pNMTreeView->ptDrag.y);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::OnMouseMove
//
/////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::OnMouseMove(WPARAM nHittest, REFPOINTS pts)
{
	if(m_hDraggingItem)
	{
		ImageList_DragMove(pts.x, pts.y);

		//If the cursor is on an item, hilite it as the drop target
		HTREEITEM hItem = HitTest(pts);
		if(hItem)
		{
			ImageList_DragLeave( m_hWnd ); 
			TreeView_SelectDropTarget(m_hWnd, hItem); 
			ImageList_DragEnter( m_hWnd, pts.x, pts.y);
		}
		return TRUE;
	}
		 
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::OnLButtonUp
//
/////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::OnLButtonUp(WPARAM fwKeys, REFPOINTS pts)
{
	if(m_hDraggingItem)
	{
		//End the Drag
		ImageList_EndDrag();
		ReleaseCapture();
		ShowCursor(TRUE);
		ImageList_DragLeave( m_hWnd ); 
		TreeView_SelectDropTarget(m_hWnd, NULL);

		m_hDraggingItem = NULL;
		return TRUE;
	}
	
	return FALSE;
}	


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::GetItemParam
//
/////////////////////////////////////////////////////////////////////
LPARAM CTreeViewLite::GetItemParam(HTREEITEM hTreeItem)
{
	return TV_GetItemParam(m_hWnd, hTreeItem);
}


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::SetItemState
//
/////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::SetItemState(HTREEITEM hItem, UINT iState, UINT iStateMask)
{
	return TV_SetItemState(m_hWnd, hItem, iState, iStateMask);
}


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::SetItemImage
//
/////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::SetItemImage(HTREEITEM hItem, INT iImage, INT iSelectedImage)
{
	//TVM_SETITEM
	TVITEM tvItem = { TVIF_IMAGE|TVIF_SELECTEDIMAGE, hItem, 0, 0, NULL, 0, iImage, iSelectedImage, 0, 0};
	return (BOOL)SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)&tvItem);
}
	

/////////////////////////////////////////////////////////////////////
// CTreeViewLite::SetItemParam
//
/////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::SetItemParam(HTREEITEM hItem, LPARAM lParam)
{
	//TVM_SETITEM
	TVITEM tvItem = { TVIF_PARAM, hItem, 0, 0, NULL, 0, 0, 0, 0, lParam};
	return (BOOL)SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)&tvItem);
}


/////////////////////////////////////////////////////////////////////
// CTreeViewLite::SetItemText
//
/////////////////////////////////////////////////////////////////////
BOOL CTreeViewLite::SetItemText(HTREEITEM hItem, WCHAR* pwszText)
{
	if(IsUnicodeOS())
	{
		//TVM_SETITEM
		TVITEMW tvItem = { TVIF_TEXT, hItem, 0, 0, pwszText, 0, 0, 0, 0, 0};
		return (BOOL)SendMessageW(m_hWnd, TVM_SETITEMW, 0, (LPARAM)&tvItem);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN+1];
		ConvertToMBCS(pwszText, szBuffer, MAX_NAME_LEN);
		
		//TVM_SETITEM
		TVITEMA tvItem = { TVIF_TEXT, hItem, 0, 0, szBuffer, 0, 0, 0, 0, 0};
		return (BOOL)SendMessageA(m_hWnd, TVM_SETITEMA, 0, (LPARAM)&tvItem);
	}
}


/////////////////////////////////////////////////////////////////////
// HTREEITEM CTreeViewLite::InsertItem
//
/////////////////////////////////////////////////////////////////////
HTREEITEM CTreeViewLite::InsertItem(HTREEITEM hParent, HTREEITEM hInsAfter, WCHAR* wszName, LPARAM lParam, INT iImage, INT iSelectedImage, UINT iState, UINT iStateMask)
{
	return TV_InsertItem(m_hWnd, hParent, hInsAfter, wszName, lParam, iImage, iSelectedImage, iState, iStateMask);
}


/////////////////////////////////////////////////////////////////////
// HTREEITEM CTreeViewLite::HitTest
//
/////////////////////////////////////////////////////////////////////
HTREEITEM CTreeViewLite::HitTest(REFPOINTS pts, UINT* pFlags, BOOL fClientCoords)
{
	TVHITTESTINFO tvHitInfo = {{pts.x, pts.y}, 0, NULL};
	if(!fClientCoords)
		ScreenToClient(m_hWnd, &tvHitInfo.pt);

	//TVM_HITTEST
	HTREEITEM hItem = (HTREEITEM)SendMessage(m_hWnd, TVM_HITTEST, 0, (LPARAM)&tvHitInfo);
	if(pFlags)
		*pFlags = tvHitInfo.flags;

	return hItem;
}
		


/////////////////////////////////////////////////////////////////////
// CComboBoxLite::CComboBoxLite
//
/////////////////////////////////////////////////////////////////////
CComboBoxLite::CComboBoxLite(HWND hWndParent, UINT nID)
	: CWndLite(hWndParent, nID)
{
	//NOTE: The "Height" of the combo indicates the "dropdown" height of the combo...

	
	m_iSavedSel		= CB_ERR;
}


/////////////////////////////////////////////////////////////////////
// CComboBoxLite::~CComboBoxLite
//
/////////////////////////////////////////////////////////////////////
CComboBoxLite::~CComboBoxLite()
{
}


/////////////////////////////////////////////////////////////////////////////
// CComboBoxLite::Populate
//
/////////////////////////////////////////////////////////////////////////////
BOOL CComboBoxLite::Populate(ULONG cItems, const WIDENAMEMAP* rgItems)
{
	//Remove any existing Data
	ResetContent();

	//Fill in the Combo Box...
	for(ULONG i=0; i<cItems; i++)
		AddString(rgItems[i].pwszName, (LPARAM)rgItems[i].lItem);

	//Restore the Default Selection...
	RestoreSelection();
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CComboBoxLite::OnCommandNotify
//
/////////////////////////////////////////////////////////////////////////////
BOOL CComboBoxLite::OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl) 
{
	switch(wNotifyCode)
	{
		case CBN_SELCHANGE:
		{	
			if(OnSelChange(iID, hWndCtrl))
				return TRUE;
			break;
		}

		case CBN_DROPDOWN:
		{	
			if(OnDropDown(iID, hWndCtrl))
				return TRUE;
			break;
		}
	};

	//Otherwise Delegate
	return CWndLite::OnCommandNotify(wNotifyCode, iID, hWndCtrl);
}


/////////////////////////////////////////////////////////////////////////////
// CComboBoxLite::OnSelChange
//
/////////////////////////////////////////////////////////////////////////////
BOOL CComboBoxLite::OnSelChange(INT iID, HWND hWndCtrl) 
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CComboBoxLite::OnDropDown
//
/////////////////////////////////////////////////////////////////////////////
BOOL CComboBoxLite::OnDropDown(INT iID, HWND hWndCtrl) 
{
	return FALSE;
}


////////////////////////////////////////////////////////////////
// CComboBoxLite::AddString
//
/////////////////////////////////////////////////////////////////
INDEX CComboBoxLite::AddString(WCHAR* pwszString, LPARAM lParam)
{
	//Delegate to our other function to add just the string to the combo
	INDEX iItem = AddString(pwszString);
	if(iItem!=CB_ERR)
	{
		//Now set the item data for this item...
		SetItemParam(iItem, lParam);
	}
		
	return iItem;
}


/////////////////////////////////////////////////////////////////////
// CComboBoxLite::GetSelText
//
/////////////////////////////////////////////////////////////////////
INDEX CComboBoxLite::GetSelText(WCHAR* pwszString, ULONG ulMaxSize)
{
	ASSERT(pwszString);
	ASSERT(ulMaxSize);

	//Try to obtain the Current Selection
	INDEX iSel = GetCurSel();
	
	//This may fail, if the current selection is entered (DropDown instead of DropList)
	if(iSel == CB_ERR)
	{					  
		GetWindowText(pwszString, ulMaxSize);
	}
	else
	{
		//Should be limiting the text if this ASSERT is hit!
		//Length does not include the NULL Terminator
		LRESULT ulLength = GetTextLength(iSel);
		ASSERT((ULONG)ulLength < ulMaxSize);
		
		//Obtain the text...
		if((ULONG)ulLength < ulMaxSize)
			GetText(iSel, pwszString);
	}

	return iSel;
}


//////////////////////////////////////////////////////////////////
// CComboBoxLite::SetSelText
//
//////////////////////////////////////////////////////////////////
INDEX CComboBoxLite::SetSelText(WCHAR* pwszString, BOOL fAddItem)
{
	ASSERT(pwszString);
	
	//Try to find the Indicated Text
	INDEX iSel = FindStringExact(pwszString);
	
	//If not found, just add it to the list (if desired)
	if(iSel == CB_ERR)
	{
		if(fAddItem)
		{
			iSel = AddString(pwszString, 0);
		}
		else
		{
			SetCurSel(iSel);
			SetWindowText(pwszString);
		}
	}
	
	if(iSel != CB_ERR)
		iSel = SetCurSel(iSel);

	return iSel;
}

					
////////////////////////////////////////////////////////////////
// CComboBoxLite::SetSelValue
//
/////////////////////////////////////////////////////////////////
INDEX CComboBoxLite::SetSelValue(LPARAM lParam)
{
	//Loop through all Combo Item Values and Select specified one...
	INDEX iCount = GetCount();
	for(INDEX i=0; i<iCount; i++)
	{
		if(lParam == GetItemParam(i))
		{
			SetCurSel(i);
			return i;			
		}
	}

	return CB_ERR;
}



/////////////////////////////////////////////////////////////////////
// CComboBoxLite::GetSelValue
//
/////////////////////////////////////////////////////////////////////
LPARAM CComboBoxLite::GetSelValue()
{
	//Obtain the Current Selection
	INDEX iSel		= GetCurSel();
	LPARAM lParam	= CB_ERR;

	//This may fail, if the current selection is entered (DropDown instead of DropList)
	if(iSel == CB_ERR)
	{					  
		WCHAR* pwszText = GetWindowText();
		WCHAR* pwszEnd = NULL;
		if(pwszText)
		{
			//Convert to LPARAM
			if(!ConvertToLONG(pwszText, (LONG*)&lParam))
				lParam = CB_ERR;
			SAFE_FREE(pwszText);
		}
	}
	else
	{
		lParam = GetItemParam(iSel);
	}

	return lParam;
}


/////////////////////////////////////////////////////////////////////
// CButtonLite::CButtonLite
//
/////////////////////////////////////////////////////////////////////
CButtonLite::CButtonLite(HWND hWndParent, UINT nID)
	: CWndLite(hWndParent, nID)
{
}


/////////////////////////////////////////////////////////////////////
// CButtonLite::~CButtonLite
//
/////////////////////////////////////////////////////////////////////
CButtonLite::~CButtonLite()
{
}


/////////////////////////////////////////////////////////////////////
// CComboBoxEx::CComboBoxEx
//
/////////////////////////////////////////////////////////////////////
CComboBoxEx::CComboBoxEx(HWND hWndParent, UINT nID)
	: CComboBoxLite(hWndParent, nID)
{
}


/////////////////////////////////////////////////////////////////////
// CComboBoxEx::~CComboBoxEx
//
/////////////////////////////////////////////////////////////////////
CComboBoxEx::~CComboBoxEx()
{
}


/////////////////////////////////////////////////////////////////////
// CComboBoxEx::OnInitialUpdate
//
/////////////////////////////////////////////////////////////////////
BOOL CComboBoxEx::OnInitialUpdate()
{
	SetUnicodeFormat(IsUnicodeOS());
	
	//Delegate
	return CComboBoxLite::OnInitialUpdate();
}

	
/////////////////////////////////////////////////////////////////////
// CComboBoxEx::InsertItem
//
/////////////////////////////////////////////////////////////////////
INDEX CComboBoxEx::InsertItem(INDEX iItem, WCHAR* pwszName, LPARAM lParam, INT iIndent, INT iImage, UINT iSelectedImage)
{
	INDEX iIndex = CB_ERR;
	
	//Figure out the mask
	UINT   dwMask = 0;
	if(lParam != PARAM_NONE)
		dwMask |= CBEIF_LPARAM;
	if(pwszName )
		dwMask |= CBEIF_TEXT;
	if(iIndent != PARAM_NONE)
		dwMask |= CBEIF_INDENT;
	if(iImage != IMAGE_NONE)
		dwMask |= CBEIF_IMAGE;
	if(iSelectedImage != IMAGE_NONE)
		dwMask |= CBEIF_SELECTEDIMAGE;
	
	//CBEM_INSERTITEM
	//NOTE: In the OnInitialUpdate we set the ComboBoxEx to be Unicode if on
	//a unicode OS and ANSI is not.  Even thought resource has ComboBoxEx (ANSI) the control
	//allow the user tha change the setting at runtime...
	if(IsUnicodeOS())
	{
		COMBOBOXEXITEMW comboItem = { dwMask, iItem, pwszName, 0/*cchTextMax*/, iImage, iSelectedImage, 0/*iOverlay*/, iIndent, lParam };
		iIndex = SendMessageW(m_hWnd, CBEM_INSERTITEMW, 0, (LPARAM)&comboItem);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN+1]={0};
		ConvertToMBCS(pwszName, szBuffer, MAX_NAME_LEN);

		COMBOBOXEXITEMA comboItem = { dwMask, iItem, szBuffer, 0/*cchTextMax*/, iImage, iSelectedImage, 0/*iOverlay*/, iIndent, lParam };
		iIndex = SendMessageA(m_hWnd, CBEM_INSERTITEMA, 0, (LPARAM)&comboItem);
	}

	return iIndex;
}


/////////////////////////////////////////////////////////////////////
// CToolBarLite::CToolBarLite
//
/////////////////////////////////////////////////////////////////////
CToolBarLite::CToolBarLite()
{
}


/////////////////////////////////////////////////////////////////////
// CToolBarLite::~CToolBarLite
//
/////////////////////////////////////////////////////////////////////
CToolBarLite::~CToolBarLite()
{
}


/////////////////////////////////////////////////////////////////////
// CToolBarLite::Create
//
/////////////////////////////////////////////////////////////////////
BOOL CToolBarLite::Create(HWND hWndParent, UINT nID,  UINT nBitmapID, UINT cButtons, TBBUTTON* rgButtons, DWORD dwStyle)
{
	ASSERT(IsDestroyed());
	m_hWndParent = hWndParent;
	HINSTANCE hInstance = GetAppLite()->m_hInstance;
	
	//Create ToolBar
	m_hWnd = CreateToolbarEx( 
		hWndParent,				// parent
		dwStyle,
		nID,					// toolbar id
		3,                      // number of bitmaps
		hInstance,	            // mod instance
		nBitmapID,				// resource ID for bitmap
		rgButtons,				// address of buttons
		cButtons,				// number of buttons
		16,16,                  // width & height of buttons
		16,16,                  // width & height of bitmaps
		sizeof(TBBUTTON));      // structure size

    //Create ToolTips
    HWND hWndTT = (HWND)SendMessage(m_hWnd, TB_GETTOOLTIPS, 0, 0);
	if(hWndTT)
    {
	    TOOLINFOA toolInfo;
		memset(&toolInfo, 0, sizeof(TOOLINFO));
        toolInfo.cbSize		= sizeof(TOOLINFO);
        toolInfo.uFlags		= TTF_CENTERTIP;
        toolInfo.hwnd		= m_hWnd;
        toolInfo.uId		= (UINT)ID_TOOLBAR;
        toolInfo.hinst		= hInstance;
        toolInfo.lpszText	= (LPSTR)LPSTR_TEXTCALLBACKA;

        // Set up the ToolTips
        SendMessage(hWndTT, TTM_ADDTOOLA, 0, (LPARAM)&toolInfo);
	}
	return m_hWnd != NULL;
}


/////////////////////////////////////////////////////////////////////
// CStatusBarLite::CStatusBarLite
//
/////////////////////////////////////////////////////////////////////
CStatusBarLite::CStatusBarLite()
{
}


/////////////////////////////////////////////////////////////////////
// CStatusBarLite::~CStatusBarLite
//
/////////////////////////////////////////////////////////////////////
CStatusBarLite::~CStatusBarLite()
{
}


/////////////////////////////////////////////////////////////////////
// CStatusBarLite::Create
//
/////////////////////////////////////////////////////////////////////
BOOL CStatusBarLite::Create(HWND hWndParent, UINT nID,  WCHAR* pwszText, DWORD dwStyle)
{
	ASSERT(IsDestroyed());
	m_hWndParent = hWndParent;
	
	//Create StatusBar
	if(IsUnicodeOS())
	{
		m_hWnd = CreateStatusWindowW(dwStyle, pwszText, hWndParent, nID);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszText, szBuffer, MAX_NAME_LEN);
		m_hWnd = CreateStatusWindowA(dwStyle, szBuffer, hWndParent, nID);
	}

	return m_hWnd != NULL;

}


/////////////////////////////////////////////////////////////////////
// CPropPageLite::CPropPageLite
//
/////////////////////////////////////////////////////////////////////
CPropPageLite::CPropPageLite(UINT uIDD, TCHAR* ptszTitle)
	: CDialogLite(uIDD)
{
	memset(&m_psPage, 0, sizeof(PROPSHEETPAGE));

    //PROPSHEETPAGE
	m_psPage.dwSize			= sizeof(PROPSHEETPAGE);
    m_psPage.dwFlags		= PSP_USETITLE;
    m_psPage.hInstance		= GetAppLite()->m_hInstance;
    m_psPage.pszTemplate	= MAKEINTRESOURCE(uIDD);
    m_psPage.pszIcon		= NULL;
    m_psPage.pfnDlgProc		= PropPageProc;
    m_psPage.pszTitle		= ptszTitle;
    m_psPage.lParam			= (INT_PTR)this;

	//Parent Property Sheet
	m_pCPropSheet			= NULL;
}


/////////////////////////////////////////////////////////////////////
// CPropPageLite::~CPropPageLite
//
/////////////////////////////////////////////////////////////////////
CPropPageLite::~CPropPageLite()
{
}



/////////////////////////////////////////////////////////////////////
// CPropPageLite::CPropPageLite
//
/////////////////////////////////////////////////////////////////////
void CPropPageLite::SetParent(CPropSheetLite* pCPropSheetLite)
{
	ASSERT(pCPropSheetLite);
	m_pCPropSheet = pCPropSheetLite;
}


/////////////////////////////////////////////////////////////////////
// CPropPageLite::OnApply
//
/////////////////////////////////////////////////////////////////////
BOOL CPropPageLite::OnApply()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CPropPageLite::OnOK
//
/////////////////////////////////////////////////////////////////////
BOOL CPropPageLite::OnOK()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CPropPageLite::OnReset
//
/////////////////////////////////////////////////////////////////////
BOOL CPropPageLite::OnReset()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CPropPageLite::OnCancel
//
/////////////////////////////////////////////////////////////////////
BOOL CPropPageLite::OnCancel()
{
	return TRUE;
}



/////////////////////////////////////////////////////////////////////
// CPropPageLite::OnSetActive
//
/////////////////////////////////////////////////////////////////////
BOOL CPropPageLite::OnSetActive()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CPropPageLite::OnKillActive
//
/////////////////////////////////////////////////////////////////////
BOOL CPropPageLite::OnKillActive()
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CPropPageLite::HandleMessage
//
/////////////////////////////////////////////////////////////////////
BOOL CPropPageLite::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_NOTIFY:
		{	
			switch (((NMHDR*)lParam)->code) 
    		{
				case PSN_SETACTIVE:
				{	
					if(!OnSetActive())
					{
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}

					return FALSE;
				}

				case PSN_KILLACTIVE:
				{
					if(!OnKillActive())
					{
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}

					return FALSE;
				}

				case PSN_APPLY:	//OK
				{
					if(!OnApply())
					{
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}

					return FALSE;
				}

				case PSN_RESET: //CANCEL
					OnReset();
					return FALSE;
			}
		}
	};

	//Pass the message onto the Dialog
	return CDialogLite::HandleMessage(hWnd, msg, wParam, lParam);
}




/////////////////////////////////////////////////////////////////////
// CPropPageLite::PropPageProc
//
/////////////////////////////////////////////////////////////////////
INT_PTR WINAPI CPropPageLite::PropPageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CPropPageLite* pCPropPageLite = (CPropPageLite*)GetThis(hWnd);
		
	switch(msg)
	{
		case WM_INITDIALOG:
		{	
			//Save the Window Handle
			PROPSHEETPAGE*ps = (PROPSHEETPAGE*)lParam;
			CPropPageLite* pCPropPageLite = (CPropPageLite*)ps->lParam;
			pCPropPageLite->m_hWnd = hWnd;

			//Save the "this" pointer
			SetThis(hWnd, pCPropPageLite);
			return pCPropPageLite->OnInitDialog();
		}
	};

	if(pCPropPageLite)
		return pCPropPageLite->HandleMessage(hWnd, msg, wParam, lParam);

	return FALSE;
}



/////////////////////////////////////////////////////////////////////
// CPropSheetLite::CPropSheetLite
//
/////////////////////////////////////////////////////////////////////
CPropSheetLite::CPropSheetLite(TCHAR* ptszTitle, HICON hIcon)
{
	memset(&m_psHeader, 0, sizeof(PROPSHEETPAGE));

    //PROPSHEETHEADER
    m_psHeader.dwSize			= sizeof(PROPSHEETHEADER);
    m_psHeader.dwFlags			= PSH_USEHICON | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    m_psHeader.hwndParent		= NULL;
    m_psHeader.hInstance		= GetAppLite()->m_hInstance;
	m_psHeader.hIcon			= hIcon;
    m_psHeader.pszCaption		= ptszTitle;
    m_psHeader.nPages			= 0;
    m_psHeader.nStartPage		= 0;
    m_psHeader.ppsp				= NULL;
}


/////////////////////////////////////////////////////////////////////
// CPropSheetLite::~CPropSheetLite
//
/////////////////////////////////////////////////////////////////////
CPropSheetLite::~CPropSheetLite()
{
	SAFE_FREE(m_psHeader.ppsp);
}



/////////////////////////////////////////////////////////////////////
// CPropSheetLite::AddPage
//
/////////////////////////////////////////////////////////////////////
void CPropSheetLite::AddPage(CPropPageLite* pCPropPageLite)
{
	ASSERT(pCPropPageLite);
	HRESULT hr = S_OK;
	
	//Set the Pages parent (this sheet)
	pCPropPageLite->SetParent(this);

	//Enlarge the array
	SAFE_REALLOC(m_psHeader.ppsp, PROPSHEETPAGE, m_psHeader.nPages+1);
	
	//Copy into our array
	memcpy((void*)&m_psHeader.ppsp[m_psHeader.nPages], &pCPropPageLite->m_psPage, sizeof(PROPSHEETPAGE));
	m_psHeader.nPages++;

CLEANUP:
	;
}


/////////////////////////////////////////////////////////////////////
// CPropSheetLite::DoModal
//
/////////////////////////////////////////////////////////////////////
LRESULT CPropSheetLite::DoModal(HWND hWndParent, INT nStartPage)
{
	//Display the Property Sheet
    m_psHeader.hwndParent	= hWndParent;
	m_psHeader.nStartPage	= nStartPage;
    return PropertySheet(&m_psHeader);
}



/////////////////////////////////////////////////////////////////////
// CTabLite::CTabLite
//
/////////////////////////////////////////////////////////////////////
CTabLite::CTabLite(HWND hWndParent, UINT nID)
	: CSplitterLite(hWndParent, nID)
{
}


/////////////////////////////////////////////////////////////////////
// CTabLite::~CTabLite
//
/////////////////////////////////////////////////////////////////////
CTabLite::~CTabLite()
{
}


/////////////////////////////////////////////////////////////////////
// CTabLite::OnCreate
//
/////////////////////////////////////////////////////////////////////
BOOL CTabLite::OnCreate(CREATESTRUCT* pCREATESTRUCT)
{
	//SubClass the window
	SubClassWindow(WndProc);

	//Delegate
	return CSplitterLite::OnCreate(pCREATESTRUCT);
}


/////////////////////////////////////////////////////////////////////
// CTabLite::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CTabLite::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	switch(pNMHDR->code)
	{
		ON_COMMAND(TCN_SELCHANGE,		OnSelChange((INT)pNMHDR->idFrom, pNMHDR->hwndFrom));
		ON_COMMAND(TCN_SELCHANGING,		OnSelChanging((INT)pNMHDR->idFrom, pNMHDR->hwndFrom));
//		ON_COMMAND(TCN_GETOBJECT,		OnGetObject((NMOBJECTNOTIFY*)pNMHDR));
	};

	return FALSE;				
}


/////////////////////////////////////////////////////////////////////////////
// CTabLite::SetCurSel
//
/////////////////////////////////////////////////////////////////////////////
INDEX CTabLite::SetCurSel(INDEX iItem, BOOL fSendNotification) 
{
	//NOTE: We have to send this message our selves due to the following MSDN Article
	//"This method selects a tab in a tab control. A tab control does not send a 
	//TCN_SELCHANGING or TCN_SELCHANGE notification message when a tab is selected using 
	//this method. These notifications are sent, using WM_NOTIFY, when the user clicks or 
	//uses the keyboard to change tabs."
	if(fSendNotification)
	{
		NMHDR nmHdr = { m_hWnd, IDP_TABS, TCN_SELCHANGING };
		SendMessage(m_hWnd, WM_NOTIFY, 0, (LPARAM)&nmHdr);
	}

	//Send the message
	INDEX iSel = (INDEX)::SendMessageA(m_hWnd, TCM_SETCURSEL, iItem, 0);

	//Post
	if(fSendNotification)
	{
		NMHDR nmHdr = { m_hWnd, IDP_TABS, TCN_SELCHANGE };
		SendMessage(m_hWnd, WM_NOTIFY, 0, (LPARAM)&nmHdr);
	}
	
	return iSel;
}

									
/////////////////////////////////////////////////////////////////////////////
// CTabLite::OnSelChange
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTabLite::OnSelChange(INT iID, HWND hWndCtrl) 
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CTabLite::OnSelChanging
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTabLite::OnSelChanging(INT iID, HWND hWndCtrl) 
{
	return FALSE;
}

////////////////////////////////////////////////////////////////
// CTabLite::FindTab
//
/////////////////////////////////////////////////////////////////
INDEX CTabLite::FindTab(LPARAM lParam)
{
	//Try and find this suite in the list
	INDEX cItems = GetItemCount();
	for(INDEX iItem=0; iItem<cItems; iItem++)
	{
		LPARAM lCurParam = GetItemData(iItem);
		if(lParam == lCurParam)
			return iItem;
	}

	return CB_ERR;
}


/////////////////////////////////////////////////////////////////////
// CTabLite::GetItemData
//
/////////////////////////////////////////////////////////////////////
LPARAM CTabLite::GetItemData(INDEX iItem)
{							   	
	LPARAM lParam = CB_ERR;

	//Delegate
	if(GetItem(iItem, NULL, 0, &lParam))
		return lParam;

	return CB_ERR;
}


/////////////////////////////////////////////////////////////////////
// CTabLite::GetItem
//
/////////////////////////////////////////////////////////////////////
BOOL CTabLite::GetItem(INDEX iItem, WCHAR* pwszText, ULONG ulMaxLength, LPARAM* plParam)
{							   	
	//Calculate the Mask/flags
	UINT dwMask = 0;
	if(pwszText && ulMaxLength)
		dwMask |= TCIF_TEXT;
//	if(iImage != IMAGE_NONE)
//		dwMask |= TCIF_IMAGE;
//	if(dwState != STATE_NONE)
//		dwMask |= TCIF_STATE;
	if(plParam)
		dwMask |= TCIF_PARAM;

	if(IsUnicodeOS())
	{
		//TCM_GETITEMW
		TCITEMW tcItem = { dwMask, 0, 0, pwszText, ulMaxLength, 0, 0/*lParam*/};
		if(SendMessageW(m_hWnd, TCM_GETITEMW, iItem, (LPARAM)&tcItem))
		{
			if(plParam)
				*plParam = tcItem.lParam;
			return TRUE;
		}
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN+1];
		
		//TCM_GETITEMA
		TCITEMA tcItem = { dwMask, 0, 0, szBuffer, MAX_NAME_LEN, 0, 0/*lParam*/};
		if(SendMessageA(m_hWnd, TCM_GETITEMA, iItem, (LPARAM)&tcItem))
		{
			if(pwszText)
				ConvertToWCHAR(szBuffer, pwszText, ulMaxLength);
			if(plParam)
				*plParam = tcItem.lParam;
			return TRUE;
		}
	}
	

	//Otherwise
	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CTabLite::SetItemText
//
/////////////////////////////////////////////////////////////////////
BOOL CTabLite::SetItemText(INDEX iItem, LPCWSTR pwszText)
{
	//Delegate
	return SetItem(iItem, pwszText);
}


/////////////////////////////////////////////////////////////////////
// CTabLite::SetItem
//
/////////////////////////////////////////////////////////////////////
BOOL CTabLite::SetItem(INDEX iItem, LPCWSTR pwszText, LPARAM lParam, INT iImage, DWORD dwState, DWORD dwStateMask)
{
	//Calculate the Mask/flags
	UINT dwMask = 0;
	if(pwszText)
		dwMask |= TCIF_TEXT;
	if(iImage != IMAGE_NONE)
		dwMask |= TCIF_IMAGE;
	if(dwState != STATE_NONE)
		dwMask |= TCIF_STATE;
	if(lParam != PARAM_NONE)
		dwMask |= TCIF_PARAM;

	if(IsUnicodeOS())
	{
		//TVM_SETITEM
		TCITEMW tcItem = { dwMask, dwState, dwStateMask, (WCHAR*)pwszText, 0/*cchTextMax*/, iImage, lParam };
		return (BOOL)SendMessageW(m_hWnd, TCM_SETITEMW, iItem, (LPARAM)&tcItem);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN+1];
		ConvertToMBCS(pwszText, szBuffer, MAX_NAME_LEN);
		
		//TVM_SETITEM
		TCITEMA tcItem = { dwMask, dwState, dwStateMask, szBuffer, 0/*cchTextMax*/, iImage, lParam };
		return (BOOL)SendMessageA(m_hWnd, TCM_SETITEMA, iItem, (LPARAM)&tcItem);
	}
}


/////////////////////////////////////////////////////////////////////
// CTabLite::InsertItem
//
/////////////////////////////////////////////////////////////////////
INDEX CTabLite::InsertItem(INDEX iItem, LPCWSTR pwszText, LPARAM lParam, INT iImage, DWORD dwState, DWORD dwStateMask)
{							   	
	//Calculate the Mask/flags
	UINT dwMask = 0;
	if(pwszText)
		dwMask |= TCIF_TEXT;
	if(iImage != IMAGE_NONE)
		dwMask |= TCIF_IMAGE;
	if(dwState != STATE_NONE)
		dwMask |= TCIF_STATE;
	if(lParam != PARAM_NONE)
		dwMask |= TCIF_PARAM;
	
	if(IsUnicodeOS())
	{
		//TCM_INSERTITEM
		TCITEMW tcItem = { dwMask, dwState, dwStateMask, (WCHAR*)pwszText, 0/*cchTextMax*/, iImage, lParam };
		return (INDEX)SendMessageW(m_hWnd, TCM_INSERTITEMW, iItem, (LPARAM)&tcItem);
	}
	else
	{
		CHAR szBuffer[MAX_QUERY_LEN+1] = {0};
		ConvertToMBCS(pwszText, szBuffer, MAX_QUERY_LEN);

		//TCM_INSERTITEM
		TCITEMA tcItem = { dwMask, dwState, dwStateMask, szBuffer, 0/*cchTextMax*/, iImage, lParam };
		return (INDEX)SendMessageA(m_hWnd, TCM_INSERTITEMA, iItem, (LPARAM)&tcItem);
	}

	return 0;
}





/////////////////////////////////////////////////////////////////////////////
// CCmdLineLite
//
/////////////////////////////////////////////////////////////////////////////
CCmdLineLite::CCmdLineLite()
{
	m_wszLastCmd[0] = wEOL;
}


/////////////////////////////////////////////////////////////////////////////
// CCmdLineLite
//
/////////////////////////////////////////////////////////////////////////////
CCmdLineLite::~CCmdLineLite()
{
}


/////////////////////////////////////////////////////////////////////////////
// CCmdLineLite::ResetParser
//
/////////////////////////////////////////////////////////////////////////////
void CCmdLineLite::ResetParser()
{
	m_wszLastCmd[0] = wEOL;
}


/////////////////////////////////////////////////////////////////////////////
// CCmdLineLite::ParseParam
//
/////////////////////////////////////////////////////////////////////////////
void CCmdLineLite::ParseParam( WCHAR* pwszParam, BOOL bFlag, BOOL bLast )
{
}


/////////////////////////////////////////////////////////////////////////////
// CCmdLineLite::ParseCommandLine
//
/////////////////////////////////////////////////////////////////////////////
BOOL CCmdLineLite::ParseCommandLine()
{
	//NOTE:  This is a simple command line parser which relies upon the Runtime to seperate
	//the arguments into argc/argv pairs.  Basically the runtime determines arguments by words
	//seperated soley by spaces, unless they are enclosed in double quotes.  If an argument
	//has spaces or needs embedded quotes, you need to close the argument in double quotes
	//and the embedded quotes need to be single quotes, (as single quotes are not recognized
	//and are ignored)
	WCHAR wszBuffer[MAX_QUERY_LEN];
	
	for(INT i=1; i<__argc; i++)
	{
		//TODO remove conversion code for NT unicode support (GetCommandLineW)
		//Can always do the opposite conversion on Win95 if only GetCommandLineA is supported.
		WCHAR* pwszParam = wszBuffer;
		ConvertToWCHAR(__argv[i], pwszParam, MAX_QUERY_LEN);
		
		BOOL bFlag = FALSE;
		BOOL bLast = ((i + 1) == __argc);
		if(pwszParam[0] == L'-' || pwszParam[0] == L'/')
		{
			// remove flag specifier
			bFlag = TRUE;
			pwszParam++;
		}
		
		ParseParam(pwszParam, bFlag, bLast);
	}

	//TRUE - CmdLine specified, FALSE - no command line args
	return __argc >1;
}


/////////////////////////////////////////////////////////////////////////////
// CCmdLineLite::ParseFile
//
/////////////////////////////////////////////////////////////////////////////
BOOL CCmdLineLite::ParseFile(WCHAR* pwszFileName)
{
	if(!pwszFileName)
		return FALSE;

	static CHAR szBuffer[MAX_QUERY_LEN+1] = {0};
	CHAR*  pszToken = NULL;
	BOOL bFlag = TRUE;

	//Reset the Parser
	ResetParser();	

	//Open the file - for reading...
	CFileLite cFile;
	if(FAILED(cFile.Open(pwszFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)))
		return FALSE;

	//Start reading...
	//NOTE:  We read the entire steam in ANSI, since this is the native format of the file.
	//We get major speed improvements reading all characters first, then converting to Unicode.
	//This method also prevens bugs with converting MBCS chars to unicode when all the characters
	//have not been read, since we may be converting and leading or trailing byte independent
	//of the context or other characters related...
	while(!cFile.IsEOF())
	{
		CHAR	chToken = 0;
		CHAR	chQuote = 0;
		LONG	cQuotes = 0;
		ULONG	cChars = 0;
		BOOL	bLast = FALSE;

		szBuffer[0] = EOL;
		pszToken = szBuffer;

		//Obtain the string
		//Sure wish getLine allowed more than 1 deliminator
		//Since its doesn't I get 1 character at a time until either a space
		//or carriage return it hit...
		while(!cFile.IsEOF() && cChars<MAX_QUERY_LEN)
		{
			if(cFile.Read(sizeof(CHAR), &chToken)!=S_OK)
				break;
			
			//Quote Character
			if(chToken == '\"' || chToken == '\'')
			{
				//First Quote - remember which quote char used
				if(cQuotes==0 && cChars==0)
					chQuote = chToken;
			
				//Start or End of Quote
				if(chToken == chQuote)
					cQuotes = cQuotes ? cQuotes-1 : cQuotes+1;
			
				//Don't include the first or the last quote
				if(cChars==0 || cQuotes==0)
					continue;
			}

			//End of Token
			if(cQuotes==0)
			{
				//End of Token indicators (not inside quotes)
				if(chToken==' ' || chToken=='\n' || cFile.IsEOF())
					break;

				//Line Feed are ignored (not inside quotes)
				if(chToken=='\r')
					continue;
			}

			*pszToken = chToken;
			pszToken++;
			cChars++;
		}

		//End of this token...
		*pszToken = EOL;

		//Determine if this is a flag or parameter
		//Its only a flag if it wasn't inside a quoted option and begins with '/' or '-'
		if(!chQuote && (szBuffer[0] == '/' || szBuffer[0] == '-'))
		{
			bFlag = TRUE;				//This is a flag
			pszToken = &szBuffer[1];	//Remove the flag
		}
		else
		{
			bFlag = FALSE;
			pszToken = szBuffer;
		}
		
		//Last Argument in the file...
		bLast = cFile.IsEOF();
		
		//Parse The Command Line...
		//Ignore whitespace, but whitespace enclosed in quotes is counted as a param...
		if(pszToken[0] || chQuote)
		{
			//Now convert the Token into Unicode...
			static WCHAR wszBuffer[MAX_QUERY_LEN] = {0};
			ConvertToWCHAR(pszToken, wszBuffer, MAX_QUERY_LEN);

			//Parse this parameter...
			ParseParam(wszBuffer, bFlag, bLast);
		}
	}

	return TRUE;
}



/////////////////////////////////////////////////////////////////////
// CFileLite
//
/////////////////////////////////////////////////////////////////////
CFileLite::CFileLite()
{
	m_hFile		= NULL;
	m_bEOF		= FALSE;
	
	m_bUnicode	= FALSE;
	m_cbWritten	= 0;
	m_cbRead	= 0;
}


/////////////////////////////////////////////////////////////////////
// ~CFileLite
//
/////////////////////////////////////////////////////////////////////
CFileLite::~CFileLite()
{
	//Cannot Call Virtual function from destructor
	CFileLite::Close();
}


/////////////////////////////////////////////////////////////////////
// CFileLite::Open
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::Open(LPCWSTR pwszFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, BOOL fDisplayDialog, HWND hWndParent)
{
	ASSERT(m_hFile == NULL);
	
	if(IsUnicodeOS())
	{
		m_hFile = CreateFileW
					(
						pwszFileName, 
						dwDesiredAccess,					// access (read-write) mode
						dwShareMode,						// share mode
						NULL,								// pointer to security attributes
						dwCreationDisposition,				// how to create
						FILE_ATTRIBUTE_NORMAL,				// file attributes
						NULL								// handle to file with attributes to copy
					);

		if(m_hFile==INVALID_HANDLE_VALUE)
		{
			if(fDisplayDialog)
			{
				wMessageBox(hWndParent, MB_TASKMODAL | MB_ICONERROR | MB_OK, wsz_ERROR, L"Unable to open file \"%s\", make sure File Name and Path are valid", pwszFileName);
				if(hWndParent)
					SetFocus(hWndParent);
			}
			m_hFile = NULL;
		}
	}
	else
	{
		static CHAR szFileName[MAX_NAME_LEN] = {0};
		ConvertToMBCS(pwszFileName, szFileName, MAX_NAME_LEN);

		//Delegate
		return Open(szFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition,
						fDisplayDialog, hWndParent);
	}

	return m_hFile ? S_OK : E_FAIL;
}


/////////////////////////////////////////////////////////////////////
// CFileLite::Open
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::Open(LPCSTR pszFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, BOOL fDisplayDialog, HWND hWndParent)
{
	ASSERT(m_hFile == NULL);
	
	m_hFile = CreateFileA
				(
					pszFileName, 
					dwDesiredAccess,					// access (read-write) mode
					dwShareMode,						// share mode
					NULL,								// pointer to security attributes
					dwCreationDisposition,				// how to create
					FILE_ATTRIBUTE_NORMAL,				// file attributes
					NULL								// handle to file with attributes to copy
				);

	if(m_hFile==INVALID_HANDLE_VALUE)
	{
		if(fDisplayDialog)
		{
			wMessageBox(hWndParent, MB_TASKMODAL | MB_ICONERROR | MB_OK, wsz_ERROR, L"Unable to open file \"%S\", make sure File Name and Path are valid", pszFileName);
			if(hWndParent)
				SetFocus(hWndParent);
		}
		m_hFile = NULL;
	}
 
	return m_hFile ? S_OK : E_FAIL;
}


/////////////////////////////////////////////////////////////////////
// CFileLite::Seek
//
/////////////////////////////////////////////////////////////////////
DWORD CFileLite::Seek(LONG lDistanceToMove, DWORD dwOrigin)
{
	if(m_hFile)
		return SetFilePointer(m_hFile, lDistanceToMove, NULL, dwOrigin);

	return 0;
}

	
/////////////////////////////////////////////////////////////////////
// CFileLite::DetermineUnicode
//
/////////////////////////////////////////////////////////////////////
BOOL CFileLite::DetermineUnicode()
{
	//When you open a file in Notepad, Notepad calls a Win32 function named IsTextUnicode. 
	//This function determines whether the file uses Unicode. If the file starts with the 
	//conventional signature for Unicode the BOM U+FEFF it knows to treat the file as Unicode. 
	//(Notepad always adds a BOM to a Unicode file when saving it and hides it again when opening 
	//the file.) If there is no BOM, IsTextUnicode can only guess whether the file uses Unicode 
	//based on a number of rules (described in the Visual C++ 2 documentation of IsTextUnicode).
	if(!m_cbRead)
	{
		//If we have not read any characters yet, we need to read the first 2 characters
		//(which may contain the BOM - Byte Order Mark) to determine if unicode or MBCS format
		WCHAR wBOM = wEOL;
		if(ReadFile(m_hFile, &wBOM, sizeof(wBOM), &m_cbRead, NULL))
		{
			if(wBOM == UNICODE_BYTE_ORDER_MARK)
			{
				//Unicode file
				m_bUnicode = TRUE;
			}
			else
			{
				//Otherwise its not Unicode, and the bytes we just read are real
				//data, so "unread" them by moving the seek pointer back...
				Seek(-(LONG)m_cbRead, FILE_CURRENT);
				m_bUnicode = FALSE;
			}
		}
	}

	return m_bUnicode;
}

	
/////////////////////////////////////////////////////////////////////
// CFileLite::Read
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::Read(ULONG cbBytes, WCHAR* pwszText, ULONG* pcbRead)
{
	ULONG cbRead = 0;
	HRESULT hr = S_OK;

	if(m_hFile && pwszText)
	{
		if(DetermineUnicode())
		{
			//The file is Unicode, just read directly into unicode string
			hr = ReadBytes(cbBytes, (BYTE*)pwszText, &cbRead);
		}
		else
		{
			static CHAR szBuffer[MAX_QUERY_LEN] = {0};

			//The file is MBCS, conversion is required to Unicode string
			hr = ReadBytes(min(cbBytes/2, sizeof(szBuffer)), (BYTE*)szBuffer, &cbRead);
			if(SUCCEEDED(hr))
			{
				ConvertToWCHAR(szBuffer, pwszText, cbBytes);
				cbRead = cbRead*sizeof(WCHAR);
			}
		}
	}

	if(pcbRead)
		*pcbRead = cbRead;
	return hr;
}


/////////////////////////////////////////////////////////////////////
// CFileLite::Read
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::Read(ULONG cbBytes, CHAR* pszText, ULONG* pcbRead)
{
	ULONG cbRead = 0;
	HRESULT hr = S_OK;

	if(m_hFile && pszText)
	{
		if(DetermineUnicode())
		{
			static WCHAR wszBuffer[MAX_QUERY_LEN] = {0};

			//The file is Unicode, conversion is required to MBCS string
			hr = ReadBytes(min(cbBytes*2, sizeof(wszBuffer)), (BYTE*)wszBuffer, &cbRead);
			if(SUCCEEDED(hr))
			{
				ConvertToMBCS(wszBuffer, pszText, cbBytes);
				cbRead = cbRead/sizeof(WCHAR);
			}
		}
		else
		{
			//The file is MBCS, just read directly from the stream
			hr = ReadBytes(cbBytes, (BYTE*)pszText, &cbRead);
		}
	}

	if(pcbRead)
		*pcbRead = cbRead;
	return hr;
}


/////////////////////////////////////////////////////////////////////
// CFileLite::ReadBytes
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::ReadBytes(ULONG cbBytes, BYTE* rgBytes, ULONG* pcbRead)
{
	ULONG cbRead = 0;
	HRESULT hr = S_OK;

	if(m_hFile && cbBytes)
	{
		ASSERT(rgBytes);
		
		//Read from the file...
		if(ReadFile(m_hFile, rgBytes, cbBytes, &cbRead, NULL))
		{
			if(cbBytes == cbRead)
				hr = S_OK;
			else
				hr = S_FALSE;
		}
		else
		{
			hr = E_FAIL;
		}
	}

	if(pcbRead)
		*pcbRead = cbRead;

	m_bEOF = (hr != S_OK);
	return hr;
}


/////////////////////////////////////////////////////////////////////
// CFileLite::Write
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::Write(LPCWSTR pwszText, ULONG* pcbWritten)
{
	ULONG cbWritten = 0;
	HRESULT hr = S_OK;

	if(m_hFile && pwszText)
	{
		if(IsUnicode())
		{
			//The file is Unicode, just directly write out the stream of unicode bytes
			hr = WriteBytes((ULONG)(wcslen(pwszText)*sizeof(WCHAR)), (BYTE*)pwszText, &cbWritten);
		}
		else
		{
			//The File is ANSI(MBCS), but the string is Unicode 
			//Conversion required...
			static CHAR szBuffer[MAX_QUERY_LEN] = {0};
			ConvertToMBCS(pwszText, szBuffer, MAX_QUERY_LEN);

			hr = WriteBytes((ULONG)(strlen(szBuffer)*sizeof(CHAR)), (BYTE*)szBuffer, &cbWritten);
		}
	}

	if(pcbWritten)
		*pcbWritten = cbWritten;
	return hr;
}


/////////////////////////////////////////////////////////////////////
// CFileLite::Write
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::Write(LPCSTR pszText, ULONG* pcbWritten)
{
	ULONG cbWritten = 0;
	HRESULT hr = S_OK;

	if(m_hFile && pszText)
	{
		if(IsUnicode())
		{
			//The file is Unicode, but the string is ANSI
			//Conversion required...
			static WCHAR wszBuffer[MAX_QUERY_LEN] = {0};
			ConvertToWCHAR(pszText, wszBuffer, MAX_QUERY_LEN);

			hr = WriteBytes((ULONG)(wcslen(wszBuffer)*sizeof(WCHAR)), (BYTE*)wszBuffer, &cbWritten);
		}
		else
		{
			//The File is ANSI(MBCS), just directly write out the stream of bytes
			//to MBCS before writting it directly to the file...
			hr = WriteBytes((ULONG)(strlen(pszText)*sizeof(CHAR)), (BYTE*)pszText, &cbWritten);
		}
	}

	if(pcbWritten)
		*pcbWritten = cbWritten;
	return hr;
}


/////////////////////////////////////////////////////////////////////
// CFileLite::WriteBytes
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::WriteBytes(ULONG cBytes, BYTE* rgBytes, ULONG* pcbWritten)
{
	ULONG cbWritten = 0;
	HRESULT hr = S_OK;

	if(m_hFile && cBytes)
	{
		ASSERT(rgBytes);

		//When saving as a Unicode Text file, Notepad always writes out a byte order mark (BOM)
		//Unicode character U+FEFF as the first Unicode character in a file. 
		//It uses this character (and not the file extension) to help it distinguish Unicode text 
		//from other data.
		if(IsUnicode() && !m_cbWritten)
		{
			//If the file format is Unicode, and we haven't written any bytes to file
			//yet then write out the BOM (Byte Order Mark) to indicate unicode.
			WriteFile(m_hFile, &UNICODE_BYTE_ORDER_MARK, sizeof(UNICODE_BYTE_ORDER_MARK), &m_cbWritten, NULL);
		}

		//Output to the file
		if(WriteFile(m_hFile, rgBytes, cBytes, &cbWritten, NULL))
			hr = S_OK;
		else
			hr = E_FAIL;
	}

	if(pcbWritten)
		*pcbWritten = cbWritten;
	return hr;
}



/////////////////////////////////////////////////////////////////////
// CFileLite::WriteFormat
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::WriteFormat(LPCWSTR pwszFmt, ...)
{
	if(!IsOpen())
		return S_OK;

	va_list		marker;
	WCHAR		wszBuffer[MAX_QUERY_LEN];

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf_s(wszBuffer, MAX_QUERY_LEN, _TRUNCATE, pwszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_QUERY_LEN
	wszBuffer[MAX_QUERY_LEN-1] = wEOL;
	
	//Delegate
	return Write(wszBuffer);
}

/////////////////////////////////////////////////////////////////////
// CFileLite::Flush
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::Flush()
{
	if(m_hFile)
	{
//		return FlushFileBuffers(m_hFile) ? S_OK : E_FAIL;
	}

	return S_OK;
}

/////////////////////////////////////////////////////////////////////
// CFileLite::Close
//
/////////////////////////////////////////////////////////////////////
HRESULT CFileLite::Close()
{
	if(m_hFile)
	{
		Flush();
		CloseHandle(m_hFile);
	}
	
	m_hFile		= NULL;
	m_bEOF		= FALSE;
	m_cbWritten	= 0;
	m_cbRead	= 0;
	return S_OK;
}

//////////////////////////////////////////////////////////////////
// LV_InsertColumn
//
//////////////////////////////////////////////////////////////////
INDEX LV_InsertColumn(HWND hWnd, INDEX iColumn, WCHAR* wszName, INT iImage)
{
	ULONG dwMask = LVCF_TEXT | LVCF_FMT | LVCF_SUBITEM;
	INT dwFmt = LVCFMT_LEFT;
	if(iImage != IMAGE_NONE)
	{
		dwMask |= LVCF_IMAGE;
		dwFmt |= LVCFMT_IMAGE;
	}
	
	//LVM_INSERTCOLUMN
	if(IsUnicodeOS())
	{
		//Setup LV_COLUMNINFO
		LV_COLUMNW lvColumnHeader = { dwMask, dwFmt, 0, wszName, 0, 0, iImage, 0};
		return (INDEX)SendMessage(hWnd, LVM_INSERTCOLUMNW, (WPARAM)iColumn, (LPARAM)&lvColumnHeader);
	}
	else
	{
		//Setup LV_COLUMNINFO
		CHAR szBuffer[MAX_NAME_LEN];
		LV_COLUMNA lvColumnHeader = { dwMask, dwFmt, 0, szBuffer, 0, 0, iImage, 0};
		ConvertToMBCS(wszName, szBuffer, MAX_NAME_LEN);
		return (INDEX)SendMessage(hWnd, LVM_INSERTCOLUMNA, (WPARAM)iColumn, (LPARAM)&lvColumnHeader);
	}
}


//////////////////////////////////////////////////////////////////
// LV_InsertItem
//
//////////////////////////////////////////////////////////////////
INDEX LV_InsertItem(HWND hWnd, INDEX iItem, INDEX iSubItem, WCHAR* wszName, LPARAM lParam, INT iImage, UINT iState, UINT iStateMask)
{
	//Calculate the Mask/flags
	ULONG dwMask = 0;
	if(wszName)
		dwMask |= LVIF_TEXT;
	if(iImage != IMAGE_NONE)
		dwMask |= LVIF_IMAGE;
	if(iState != STATE_NONE)
		dwMask |= LVIF_STATE;
	if(iSubItem == 0)
		dwMask |= LVIF_PARAM;
	
	if(IsUnicodeOS())
	{
		//LVM_INSERTITEM
		if(iSubItem==0)
		{
			LV_ITEMW lvItem = { dwMask, (INT)iItem, (INT)iSubItem, iState, iStateMask, wszName, 0, iImage, lParam, 0};
			return (INDEX)SendMessageW(hWnd, LVM_INSERTITEMW, 0, (LPARAM)&lvItem);
		}
		//LVM_SETITEM
		else
		{
			LV_ITEMW lvItem = { dwMask, (INT)iItem, (INT)iSubItem, iState, iStateMask, wszName, 0, iImage, lParam, 0};
			return (INDEX)SendMessageW(hWnd, LVM_SETITEMW, 0, (LPARAM)&lvItem);
		}
	}
	else
	{
		CHAR szBuffer[MAX_QUERY_LEN+1];
		ConvertToMBCS(wszName, szBuffer, MAX_QUERY_LEN);

		//LVM_INSERTITEM
		if(iSubItem==0)
		{
			LV_ITEMA lvItem = { dwMask, (INT)iItem, (INT)iSubItem, iState, iStateMask, szBuffer, 0, iImage, lParam, 0};
			return (INDEX)SendMessageA(hWnd, LVM_INSERTITEMA, 0, (LPARAM)&lvItem);
		}
		//LVM_SETITEM
		else
		{
			LV_ITEMA lvItem = { dwMask, (INT)iItem, (INT)iSubItem, iState, iStateMask, szBuffer, 0, iImage, lParam, 0};
			return (INDEX)SendMessageA(hWnd, LVM_SETITEMA, 0, (LPARAM)&lvItem);
		}
	}

	return LVM_ERR;
}


//////////////////////////////////////////////////////////////////
// LV_SetItemText
//
//////////////////////////////////////////////////////////////////
BOOL LV_SetItemText(HWND hWnd, INDEX iItem, INDEX iSubItem, WCHAR* pwszName)
{
	if(IsUnicodeOS())
	{
		//LVM_SETITEM
		LV_ITEMW lvItem = { LVIF_TEXT, (INT)iItem, (INT)iSubItem, 0, 0, pwszName, 0, 0, 0, 0};
		return (BOOL)SendMessage(hWnd, LVM_SETITEMTEXTW, (WPARAM)iItem, (LPARAM)&lvItem);
	}						
	else
	{
		CHAR szBuffer[MAX_NAME_LEN+1];
		ConvertToMBCS(pwszName, szBuffer, MAX_NAME_LEN);

		//LVM_SETITEM
		LV_ITEMA lvItem = { LVIF_TEXT, (INT)iItem, (INT)iSubItem, 0, 0, szBuffer, 0, 0, 0, 0};
		return (BOOL)SendMessageA(hWnd, LVM_SETITEMTEXTA, (WPARAM)iItem, (LPARAM)&lvItem);
	}
}


//////////////////////////////////////////////////////////////////
// LV_SetItemState
//
//////////////////////////////////////////////////////////////////
BOOL LV_SetItemState(HWND hWnd, INDEX iItem, INDEX iSubItem, UINT iState, UINT iStateMask)
{
	//LVM_SETITEM
	LV_ITEM lvItem = { LVIF_STATE, (INT)iItem, (INT)iSubItem, iState, iStateMask, NULL, 0, 0, 0, 0};
	return (BOOL)SendMessage(hWnd, LVM_SETITEMSTATE, (WPARAM)iItem, (LPARAM)&lvItem);
}

//////////////////////////////////////////////////////////////////
// LV_SetItemImage
//
//////////////////////////////////////////////////////////////////
BOOL LV_SetItemImage(HWND hWnd, INDEX iItem, INDEX iSubItem, INT iImage)
{
	//LVM_SETITEM (With IMAGE mask)
	LV_ITEM lvItem = { LVIF_IMAGE, (INT)iItem, (INT)iSubItem, 0, 0, NULL, 0, iImage, 0, 0};
	return (BOOL)SendMessage(hWnd, LVM_SETITEM, 0, (LPARAM)&lvItem);
}

//////////////////////////////////////////////////////////////////
// LV_SetItemParam
//
//////////////////////////////////////////////////////////////////
BOOL LV_SetItemParam(HWND hWnd, INDEX iItem, INDEX iSubItem, LPARAM lParam)
{
	//LVM_SETITEM (With IMAGE mask)
	LV_ITEM lvItem = { LVIF_PARAM, (INT)iItem, (INT)iSubItem, 0, 0, NULL, 0, 0, lParam, 0};
	return (BOOL)SendMessage(hWnd, LVM_SETITEM, 0, (LPARAM)&lvItem);
}


//////////////////////////////////////////////////////////////////
// LV_GetItemText
//
//////////////////////////////////////////////////////////////////
INDEX LV_GetItemText(HWND hWnd, INDEX iItem, INDEX iSubItem, WCHAR* pwszName, ULONG ulMaxSize)
{
	ASSERT(pwszName);
	pwszName[0] = wEOL;
	INDEX lReturn = 0;

	//LVM_GETITEMTEXT
	if(IsUnicodeOS())
	{
		LV_ITEMW lvItem = { LVIF_TEXT, (INT)iItem, (INT)iSubItem, 0, 0, pwszName, ulMaxSize, 0, 0, 0};
		lReturn = (INDEX)SendMessage(hWnd, LVM_GETITEMTEXTW, (WPARAM)iItem, (LPARAM)&lvItem);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN] = {0};

		LV_ITEMA lvItem = { LVIF_TEXT, (INT)iItem, (INT)iSubItem, 0, 0, szBuffer, MAX_NAME_LEN, 0, 0, 0};
		lReturn = (INDEX)SendMessage(hWnd, LVM_GETITEMTEXTA, (WPARAM)iItem, (LPARAM)&lvItem);

		ConvertToWCHAR(szBuffer, pwszName, ulMaxSize);
	}

	return lReturn;
}

//////////////////////////////////////////////////////////////////
// LV_GetItemState
//
//////////////////////////////////////////////////////////////////
UINT LV_GetItemState(HWND hWnd, INDEX iItem, UINT iMask)
{
	//LVM_GETITEMSTATE
	return 	(UINT)SendMessage(hWnd, LVM_GETITEMSTATE, (WPARAM)iItem, (LPARAM)iMask);
}

//////////////////////////////////////////////////////////////////
// LV_GetItemImage
//
//////////////////////////////////////////////////////////////////
INT LV_GetItemImage(HWND hWnd, INDEX iItem, INDEX iSubItem)
{
	//LVM_GETITEM
	LV_ITEM lvItem = { LVIF_IMAGE, (INT)iItem, (INT)iSubItem, 0, 0, 0, 0, 0, 0, 0};
	SendMessage(hWnd, LVM_GETITEM, 0, (LPARAM)&lvItem);
	return lvItem.iImage;
}

//////////////////////////////////////////////////////////////////
// LV_GetItemParam
//
//////////////////////////////////////////////////////////////////
LPARAM LV_GetItemParam(HWND hWnd, INDEX iItem, INDEX iSubItem)
{
	//LVM_GETITEM
	LV_ITEM lvItem = { LVIF_PARAM, (INT)iItem, (INT)iSubItem, 0, 0, 0, 0, 0, 0, 0};
	SendMessage(hWnd, LVM_GETITEM, 0, (LPARAM)&lvItem);
	return lvItem.lParam;
}


//////////////////////////////////////////////////////////////////
// LV_GetSelItems
//
//////////////////////////////////////////////////////////////////
LRESULT LV_GetSelItems(HWND hWnd, INDEX* pcItems, INDEX** prgSelItems, LPARAM** prgSelParams)
{
	//Get the total Selected Items
	HRESULT hr = S_OK;
	INDEX i,iSelItem =0;
	INDEX cItems = (INDEX)SendMessage(hWnd, LVM_GETSELECTEDCOUNT, 0, 0);

	//Alloc Output Array
	if(prgSelItems)
		SAFE_ALLOC(*prgSelItems, INDEX, cItems);
	if(prgSelParams)
		SAFE_ALLOC(*prgSelParams, LPARAM, cItems);

	//Find all params of Selected Items
	iSelItem = (INDEX)SendMessage(hWnd, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
	for(i=0; i<cItems; i++)
	{
		if(prgSelItems)
			(*prgSelItems)[i] = iSelItem;
		if(prgSelParams)
			(*prgSelParams)[i] = LV_GetItemParam(hWnd, iSelItem, 0);
		iSelItem = (INDEX)SendMessage(hWnd, LVM_GETNEXTITEM, (WPARAM)iSelItem, (LPARAM)LVNI_SELECTED);
	}

CLEANUP:
	if(pcItems)
		*pcItems = cItems;
	return cItems;
}


//////////////////////////////////////////////////////////////////
// LV_GetAllItems
//
//////////////////////////////////////////////////////////////////
LRESULT LV_GetAllItems(HWND hWnd, INDEX* pcItems, INDEX** prgItems, LPARAM** prgParams)
{
	//Get the total Items
	HRESULT hr = S_OK;
	INDEX i=0;
	INDEX cItems = (INDEX)SendMessage(hWnd, LVM_GETITEMCOUNT, 0, 0);

	//Alloc Output Array
	if(prgItems)
		SAFE_ALLOC(*prgItems, INDEX, cItems);
	if(prgParams)
		SAFE_ALLOC(*prgParams, LPARAM, cItems);

	//Find all Items
	for(i=0; i<cItems; i++)
	{
		if(prgItems)
			(*prgItems)[i] = i;
		if(prgParams)
			(*prgParams)[i] = LV_GetItemParam(hWnd, i, 0);
	}

CLEANUP:
	if(pcItems)
		*pcItems = cItems;
	return cItems;
}


////////////////////////////////////////////////////////////////
// LV_FindItem
//
/////////////////////////////////////////////////////////////////
INDEX LV_FindItem(HWND hWnd, LPARAM lParam, INDEX iStart)
{
	//LVM_FINDITEM
	LV_FINDINFO lvFindInfo = { LVIF_PARAM, 0, lParam, 0, 0};
	return (INDEX)SendMessage(hWnd, LVM_FINDITEM, (WPARAM)iStart, (LPARAM)&lvFindInfo);
}


//////////////////////////////////////////////////////////////////
// TV_InsertItem
//
//////////////////////////////////////////////////////////////////
HTREEITEM TV_InsertItem(HWND hWnd, HTREEITEM hParent, HTREEITEM hInsAfter, WCHAR* wszName, LPARAM lParam, INT iImage, INT iSelectedImage, UINT iState, UINT iStateMask)
{
	//Calculate the Mask/flags
	ULONG dwMask = 0;
	if(wszName)
		dwMask |= LVIF_TEXT;
	if(iImage != IMAGE_NONE)
		dwMask |= (LVIF_IMAGE | TVIF_SELECTEDIMAGE);
	if(iState != STATE_NONE)
		dwMask |= LVIF_STATE;
	if(lParam != PARAM_NONE)
		dwMask |= LVIF_PARAM;
	
	if(IsUnicodeOS())
	{
		//TVM_INSERTITEM
		TV_INSERTSTRUCTW tvInsertStruct = { hParent, hInsAfter, { dwMask, 0, iState, iStateMask, wszName, 0, iImage, iSelectedImage, 0, lParam} };
		return (HTREEITEM)SendMessageW(hWnd, TVM_INSERTITEMW, 0, (LPARAM)&tvInsertStruct);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN+1];
		ConvertToMBCS(wszName, szBuffer, MAX_NAME_LEN);

		//TVM_INSERTITEM
		TV_INSERTSTRUCTA tvInsertStruct = { hParent, hInsAfter, { dwMask, 0, iState, iStateMask, szBuffer, 0, iImage, iSelectedImage, 0, lParam} };
		return (HTREEITEM)SendMessageA(hWnd, TVM_INSERTITEMA, 0, (LPARAM)&tvInsertStruct);
	}
}


//////////////////////////////////////////////////////////////////
// TV_GetItemText
//
//////////////////////////////////////////////////////////////////
BOOL TV_GetItemText(HWND hWnd, HTREEITEM hItem, WCHAR* pwszBuffer, LONG ulMaxSize)
{
	ASSERT(pwszBuffer);
	BOOL bReturn = 0;
	
	if(IsUnicodeOS())
	{
		TVITEMW tvItem = { TVIF_TEXT, hItem, 0, 0, pwszBuffer, ulMaxSize, 0, 0, 0, 0};
		bReturn = (BOOL)SendMessage(hWnd, TVM_GETITEMW, 0, (LPARAM)&tvItem);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN] = {0};

		TVITEMA tvItem = { TVIF_TEXT, hItem, 0, 0, szBuffer, MAX_NAME_LEN, 0, 0, 0, 0};
		bReturn = (BOOL)SendMessage(hWnd, TVM_GETITEMA, 0, (LPARAM)&tvItem);

		ConvertToWCHAR(szBuffer, pwszBuffer, ulMaxSize);
	}

	return bReturn;
}


//////////////////////////////////////////////////////////////////
// TV_GetItemParam
//
//////////////////////////////////////////////////////////////////
LPARAM TV_GetItemParam(HWND hWnd, HTREEITEM hItem)
{
	//no-op
	if(hItem == NULL)
		return NULL;
	
	TVITEM tvItem = { TVIF_PARAM, hItem, 0, 0, NULL, 0, 0, 0, 0, 0};

	//GetItem
	SendMessage(hWnd, TVM_GETITEM, 0, (LPARAM)&tvItem);

	//return the lParam
	return tvItem.lParam;
}


//////////////////////////////////////////////////////////////////
// TV_SetItemState
//
//////////////////////////////////////////////////////////////////
BOOL TV_SetItemState(HWND hWnd, HTREEITEM hItem, UINT iState, UINT iStateMask)
{
	//TVM_SETITEM
	TVITEM tvItem = { TVIF_STATE, hItem, iState, iStateMask, NULL, 0, 0, 0, 0, 0};
	return (BOOL)SendMessage(hWnd, TVM_SETITEM, 0, (LPARAM)&tvItem);
}


//////////////////////////////////////////////////////////////////
// TV_FindItem
//
//////////////////////////////////////////////////////////////////
HTREEITEM TV_FindItem(HWND hWnd, HTREEITEM hParent, WCHAR* wszName)
{
	ASSERT(hWnd);
	ASSERT(wszName);
	CHAR szBuffer[MAX_NAME_LEN];

	//Convert to MBCS
	CHAR szName[MAX_NAME_LEN];
	ConvertToMBCS(wszName, szName, MAX_NAME_LEN);

	TVITEMA tvItemA = { TVIF_TEXT, 0, 0, 0, szBuffer, MAX_NAME_LEN, 0, 0, 0, 0};
	tvItemA.hItem = (HTREEITEM)SendMessage(hWnd, TVM_GETNEXTITEM, (WPARAM)TVGN_CHILD, (LPARAM)hParent);
	while(tvItemA.hItem)
	{
		//Try to find this string in the Tree
		if(SendMessage(hWnd, TVM_GETITEM, 0, (LPARAM)&tvItemA))
		{
			if(strcmp(szName, tvItemA.pszText)==0)
				return tvItemA.hItem;
		}

		//Otherwise get the NextItem and continue...
		tvItemA.hItem = (HTREEITEM)SendMessage(hWnd, TVM_GETNEXTITEM, (WPARAM)TVGN_NEXT, (LPARAM)tvItemA.hItem);
	}

	return NULL;
}


/////////////////////////////////////////////////////////////////////
// CWaitCursor
//
/////////////////////////////////////////////////////////////////////
CWaitCursor::CWaitCursor()
{	
	m_hPrevCursor = NULL; 
	WaitCursor();
}


/////////////////////////////////////////////////////////////////////
// ~CWaitCursor
//
/////////////////////////////////////////////////////////////////////
CWaitCursor::~CWaitCursor()
{
	RestoreCursor();
};

	
/////////////////////////////////////////////////////////////////////
// CWaitCursor::WaitCursor
//
/////////////////////////////////////////////////////////////////////
void CWaitCursor::WaitCursor()
{
	static HCURSOR	hWaitCursor = LoadCursor(NULL, IDC_WAIT);
	
	//Set the "HourGlass" wait cursor
	m_hPrevCursor = SetCursor(hWaitCursor);
};


/////////////////////////////////////////////////////////////////////
// CWaitCursor::RestoreCursor
//
/////////////////////////////////////////////////////////////////////
void CWaitCursor::RestoreCursor()
{
	//Restore the previous cursor...
	SetCursor(m_hPrevCursor);
};



////////////////////////////////////////////////////////////////
// DisplayContextMenu
//
/////////////////////////////////////////////////////////////////
BOOL DisplayContextMenu(HWND hWnd, UINT iID, REFPOINTS rPts, HWND hWndParent, BOOL fRelCords)
{
	//Load the SubMenu
	HMENU hMenu = LoadMenu(GetAppLite()->m_hInstance, MAKEINTRESOURCE(iID));
	HMENU hSubMenu = GetSubMenu(hMenu, 0);

	RECT rect;
	GetWindowRect(hWnd, &rect);
	POINTS pts = rPts;	//Make a copy to modify const...

	//Coordinates might be Screen Coordinates or Relative
	if(fRelCords)
	{
		rect.top	+= pts.y;	
		rect.bottom += pts.y;	
		rect.left	+= pts.x;	
		rect.right	+= pts.x;	
	}

	//This message may have come from the keyboard
	//Just display at upper left corner
	if(pts.x < rect.left || pts.x > rect.right || pts.y < rect.top || pts.y > rect.bottom)
	{
		pts.x = (SHORT)rect.left + 5;
		pts.y = (SHORT)rect.top + 5;
	}
	
	//Display SubMenu
	//Wants (x,y) in Screen Coordinates
	BOOL bResult = TrackPopupMenu(hSubMenu,
          TPM_LEFTALIGN | TPM_RIGHTBUTTON,
          pts.x,
          pts.y,
          0,
          hWndParent,
          NULL);

	DestroyMenu(hSubMenu);
	DestroyMenu(hMenu);
	return bResult;
}


////////////////////////////////////////////////////////////////
// DisplayDialog
//
/////////////////////////////////////////////////////////////////
LRESULT		DisplayDialog(UINT uID, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParam)
{
	//Display DialogBoxParam and dump any errors...
	LRESULT lResult = DialogBoxParam(GetAppLite()->m_hInstance, MAKEINTRESOURCE(uID), hWndParent, lpDialogFunc, lParam);
	GETLASTERROR(lResult != -1);

	return lResult;
}



/////////////////////////////////////////////////////////////////////
// CDropObject::CDropObject
//
/////////////////////////////////////////////////////////////////////
CDropObject::CDropObject()
{
	//IUnknown
	m_cRef = 0;
}


/////////////////////////////////////////////////////////////////////
// CDropObject::~CDropObject
//
/////////////////////////////////////////////////////////////////////
CDropObject::~CDropObject()
{
}


/////////////////////////////////////////////////////////////////
// CDropObject::AddRef
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CDropObject::AddRef()
{																
	//AddRef
	return ++m_cRef;
}																


/////////////////////////////////////////////////////////////////
// CDropObject::Release
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CDropObject::Release()
{
	//Release
	if(--m_cRef)					
		return m_cRef;											
																
	delete this;												
	return 0;													
}																

/////////////////////////////////////////////////////////////////
// CDropObject::QueryInterface
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::QueryInterface(REFIID riid, LPVOID *ppv)
{
	if(!ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	//IUNKNOWN
	if(riid == IID_IUnknown)
		*ppv = this;
	else if(riid == IID_IDataObject)
		*ppv = (IDataObject*)this;
	
	if(*ppv)
	{
		((IUnknown*)(*ppv))->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


/////////////////////////////////////////////////////////////////
// CDropObject::GetData
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium)
{
	return E_NOTIMPL;
}


/////////////////////////////////////////////////////////////////
// CDropObject::GetDataHere
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
	return E_NOTIMPL;
}


/////////////////////////////////////////////////////////////////
// CDropObject::QueryGetData
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::QueryGetData(FORMATETC* pformatetc)
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////
// CDropObject::GetCanonicalFormatEtc
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut)
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////
// CDropObject::SetData
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////
// CDropObject::EnumFormatEtc
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
	return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////
// CDropObject::DAdvise
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
	return E_NOTIMPL;
}


/////////////////////////////////////////////////////////////////
// CDropObject::DUnadvise
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::DUnadvise(DWORD dwConnection)
{
	return E_NOTIMPL;
}


/////////////////////////////////////////////////////////////////
// CDropObject::EnumDAdvise
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
	return E_NOTIMPL;
}



/////////////////////////////////////////////////////////////////////
// CDropSource::CDropSource
//
/////////////////////////////////////////////////////////////////////
CDropSource::CDropSource()
{
	//IUnknown
	m_cRef = 0;
}


/////////////////////////////////////////////////////////////////////
// CDropSource::~CDropSource
//
/////////////////////////////////////////////////////////////////////
CDropSource::~CDropSource()
{
}


/////////////////////////////////////////////////////////////////
// CDropSource::AddRef
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CDropSource::AddRef()
{																
	//AddRef
	return ++m_cRef;
}																


/////////////////////////////////////////////////////////////////
// CDropSource::Release
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CDropSource::Release()
{
	//Release
	if(--m_cRef)					
		return m_cRef;											
																
	delete this;												
	return 0;													
}																

/////////////////////////////////////////////////////////////////
// CDropSource::QueryInterface
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropSource::QueryInterface(REFIID riid, LPVOID *ppv)
{
	if(!ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	//IUNKNOWN
	if(riid == IID_IUnknown)
		*ppv = this;
	else if(riid == IID_IDropSource)
		*ppv = (IDropSource*)this;
	
	if(*ppv)
	{
		((IUnknown*)(*ppv))->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


/////////////////////////////////////////////////////////////////
// CDropSource::QueryContinueDrag
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
	return E_NOTIMPL;
}


/////////////////////////////////////////////////////////////////
// CDropSource::GiveFeedback
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropSource::GiveFeedback(DWORD dwEffect)
{
	return E_NOTIMPL;
}




/////////////////////////////////////////////////////////////////////
// CDropTarget::CDropTarget
//
/////////////////////////////////////////////////////////////////////
CDropTarget::CDropTarget()
{
	//IUnknown
	m_cRef = 0;
}


/////////////////////////////////////////////////////////////////////
// CDropTarget::~CDropTarget
//
/////////////////////////////////////////////////////////////////////
CDropTarget::~CDropTarget()
{
}


/////////////////////////////////////////////////////////////////
// CDropTarget::AddRef
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CDropTarget::AddRef()
{																
	//AddRef
	return ++m_cRef;
}																


/////////////////////////////////////////////////////////////////
// CDropTarget::Release
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CDropTarget::Release()
{
	//Release
	if(--m_cRef)					
		return m_cRef;											
																
	delete this;												
	return 0;													
}																

/////////////////////////////////////////////////////////////////
// CDropTarget::QueryInterface
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropTarget::QueryInterface(REFIID riid, LPVOID *ppv)
{
	if(!ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	//IUNKNOWN
	if(riid == IID_IUnknown)
		*ppv = this;
	else if(riid == IID_IDropTarget)
		*ppv = (IDropTarget*)this;
	
	if(*ppv)
	{
		((IUnknown*)(*ppv))->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


/////////////////////////////////////////////////////////////////
// CDropTarget::DragEnter
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	ASSERT(pDataObj);
	ASSERT(pdwEffect);
	*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}


/////////////////////////////////////////////////////////////////
// CDropTarget::DragOver
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	ASSERT(pdwEffect);
	*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}


/////////////////////////////////////////////////////////////////
// CDropTarget::DragLeave
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropTarget::DragLeave()
{
	return S_OK;
}


/////////////////////////////////////////////////////////////////
// CDropTarget::Drop
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP	CDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	ASSERT(pdwEffect);
	ASSERT(pDataObj);
	*pdwEffect = DROPEFFECT_COPY;
	HRESULT hr = S_OK;

	//FORMATC structure
	FORMATETC DataFormat = 
		{
		CF_HDROP,
		NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
		};

	//STGMEDIUM Structure
	STGMEDIUM Medium;
	memset(&Medium, 0, sizeof(STGMEDIUM));

	//NOTE: If your only dealing with Dragging and Dropping Files
	//you can more simply just use WM_DROPFILES instead of implementing 
	//this interface setting everything up, etc...

	//Obtain the Data...
	if(SUCCEEDED(hr = pDataObj->GetData(&DataFormat, &Medium)))
	{
		HDROP hDrop = (HDROP)Medium.hGlobal;
		
		//Obtain the number of files...
		UINT cFiles = DragQueryFile(hDrop, (UINT)-1, NULL, 0); 
		
		//For every file...
		for(ULONG iFile=0; iFile<cFiles; iFile++)
		{
			CHAR szFileName[_MAX_PATH]; 
			
			DragQueryFile(hDrop, iFile, szFileName, NUMELE(szFileName)); 
			wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONERROR | MB_OK, 
				wsz_INFO, L"File %d: %S", iFile+1, szFileName);
		}
	}
		
//CLEANUP:
	return hr;
}

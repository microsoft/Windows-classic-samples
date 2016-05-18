//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module CDIALOGLITE.H
//
//-----------------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////
#ifndef __CDIALOGLITE_H__
#define __CDIALOGLITE_H__


//////////////////////////////////////////////////////////////////////////
// Macros
//
//////////////////////////////////////////////////////////////////////////
#define ON_COMMAND(ID, func)			case ID: (func); return TRUE;
#define ON_COMMAND_UI(ID, flags)		case ID: (*pdwFlags = (flags)); return TRUE;
#define ON_COMMAND_UI_ENABLED(ID, func)	ON_COMMAND_UI(ID, (func) ? MF_ENABLED : MF_GRAYED)
#define ON_COMMAND_UI_CHECKED(ID, func)	ON_COMMAND_UI(ID, (func) ? MF_CHECKED : MF_UNCHECKED)

#define STATEIMAGEINDEX(iImage)			INDEXTOSTATEIMAGEMASK(iImage+1)
#define BST2STATE(dwValue)				((dwValue) ? BST_CHECKED : BST_UNCHECKED)
#define BST3STATE(dwValue)				((dwValue) ? ((dwValue) == BST_INDETERMINATE ? (dwValue) : BST_CHECKED) : BST_UNCHECKED)

//INDEX
typedef LRESULT	INDEX;
const INDEX		INVALID_INDEX	= -1;


//////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#include <richedit.h>
#include <exdisp.h>			//IWebBrowser2, DIID_DWebBrowserEvents2
#include <exdispid.h>		//DISPID_DOCUMENTCOMPLETE
#include <mshtmdid.h>		//DISPID_AMBIENT_DLCONTROL
#include <msxml.h>			//DOM

#include "Error.h"


//////////////////////////////////////////////////////////////////////////
// Typedefs
//
//////////////////////////////////////////////////////////////////////////
#define REFPOINTS	const POINTS &
#define REFRECT		const RECT &


//////////////////////////////////////////////////////////////////////////
// Forwards
//
//////////////////////////////////////////////////////////////////////////
class CPropSheetLite;
class CFrameWndLite;
class CAppLite;


//////////////////////////////////////////////////////////////////////////
// Globals
//
//////////////////////////////////////////////////////////////////////////
CAppLite*		GetAppLite();
void			SetAppLite(CAppLite* pCAppLite);

BOOL			DisplayContextMenu(HWND hWnd, UINT iID, REFPOINTS pts, HWND hWndParent, BOOL fRelCords = FALSE);
LRESULT			DisplayDialog(UINT uID, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM lParam);


/////////////////////////////////////////////////////////////////////
// CAppLite
//
/////////////////////////////////////////////////////////////////////
class CAppLite
{
public:
	CAppLite(UINT nAppID);
	virtual ~CAppLite();

	BOOL			AppInitialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, CHAR* pszCmdLine, INT nCmdShow);
	virtual BOOL	InitInstance();
	virtual int		ExitInstance();
	virtual int		Run();

//protected:
	//data
	HINSTANCE		m_hInstance;
	HINSTANCE		m_hPrevInstance;
	CHAR*			m_pszCmdLine;
	INT				m_nCmdShow;

	UINT			m_nAppID;

	static	HWND			m_hWndModeless;
	static	CFrameWndLite*	m_pCMainWindow;
};


/////////////////////////////////////////////////////////////////////
// CWndLite
//
/////////////////////////////////////////////////////////////////////
class CWndLite
{
public:
	//constructors
	CWndLite(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CWndLite();

	//Window Proc
	static LRESULT WINAPI	WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL			HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			UnhandledMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Inline helpers, (that deal with hWnds)
	HWND	GetDlgItem(INT iResourceID)					{ return ::GetDlgItem(m_hWnd, iResourceID);					}
	UINT	IsDlgButtonChecked(INT iResourceID)			{ return ::IsDlgButtonChecked(m_hWnd, iResourceID);			}
	BOOL	CheckDlgButton(INT iResourceID, UINT uCheck){ ASSERT(uCheck == BST_CHECKED || uCheck == BST_UNCHECKED || BST_INDETERMINATE); return ::CheckDlgButton(m_hWnd, iResourceID, uCheck);		}

	BOOL	IsVisible()									{ return ::IsWindowVisible(m_hWnd);							}
	HFONT	GetFont()									{ return (HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0);	}
	void	SetFont(HFONT hFont)						{ ::SendMessage(m_hWnd, WM_SETFONT, (WPARAM)hFont, 0);		}
	BOOL	EnableWindow(BOOL bEnable = TRUE)			{ return ::EnableWindow(m_hWnd, bEnable);					}
	HWND	SetFocus()									{ return ::SetFocus(m_hWnd);								}

	HWND	GetWnd()									{ /*ASSERT(m_hWnd);*/ return m_hWnd;						}
	BOOL	GetWindowPlacement()						{ memset(&m_wndPlacement, 0, sizeof(WINDOWPLACEMENT)); m_wndPlacement.length	= sizeof(WINDOWPLACEMENT); return ::GetWindowPlacement(m_hWnd, &m_wndPlacement); } 
	BOOL	SetWindowPlacement()						{ m_wndPlacement.length	= sizeof(WINDOWPLACEMENT); return ::SetWindowPlacement(m_hWnd, &m_wndPlacement); } 
	
	WCHAR*	GetWindowText()										{ return wGetWindowText(m_hWnd);										}
	LRESULT	GetWindowText(WCHAR* pwszBuffer, ULONG ulMaxSize)	{ return wSendMessage(m_hWnd, WM_GETTEXT, ulMaxSize, pwszBuffer);		}
	BOOL	SetWindowText(WCHAR* pwszString)					{ return (BOOL)wSendMessage(m_hWnd, WM_SETTEXT, 0,		 pwszString);	}

	LONG	ShowWindow(UINT nCmdShow)					{ return ::ShowWindow(m_hWnd, nCmdShow);										}
	LONG	UpdateWindow()								{ return ::UpdateWindow(m_hWnd);												}

public:	
//protected:
	virtual BOOL	PreCreateWindow(CREATESTRUCTW& cs);
	virtual	BOOL	Create(HWND hWndParent, WCHAR* pwszClassName,
								WCHAR* pwszWindowName, UINT uID,
								DWORD dwStyle = WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | WS_HSCROLL, 
								DWORD dwExStyle = WS_EX_CLIENTEDGE,
								int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, 
								int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT);

	virtual BOOL	CreateIndirect(HWND hWndParent, UINT nID = 0);
	virtual BOOL	OnInitialUpdate();
	virtual	BOOL	DestroyWindow();
	virtual BOOL	SubClassWindow(WNDPROC pWndProc);

	virtual BOOL	OnMenuSelect(UINT uID);
	virtual BOOL	OnInitMenuPopup(HMENU hMenu, UINT uPos, BOOL fSysMenu);
	virtual BOOL	HandleMenuPos(HMENU hMenu, UINT uPos, DWORD* pdwFlags);

	virtual BOOL	OnCreate(CREATESTRUCT* pCREATESTRUCT);
	virtual BOOL	OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL	OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl);
	virtual BOOL	OnNotify(INT idCtrl, NMHDR* pNMHDR);
	virtual BOOL	OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags);

	virtual BOOL	OnActivate(UINT fActive, UINT fMinimized, HWND hWndPrevious);
	virtual BOOL	OnClose();
	virtual BOOL	OnDestroy();
	virtual BOOL	OnTimer(WPARAM nIDEvent);
	virtual BOOL	OnDropFiles(HDROP hDrop);
	virtual BOOL	OnSysCommand(WPARAM nCmdType, REFPOINTS pts);

	virtual BOOL	OnSize(WPARAM nType, REFPOINTS pts);
	virtual BOOL	OnSizing(WPARAM nType, REFPOINTS pts);
	virtual BOOL	OnMove(REFPOINTS pts);
	virtual BOOL	OnMouseMove(WPARAM nHittest, REFPOINTS pts);
	
	virtual BOOL	OnDblclk(WPARAM fwKeys, REFPOINTS pts);
	virtual BOOL	OnLButtonDown(WPARAM fwKeys, REFPOINTS pts);
	virtual BOOL	OnLButtonUp(WPARAM fwKeys, REFPOINTS pts);
	virtual BOOL	OnRButtonDown(WPARAM fwKeys, REFPOINTS pts);
	virtual BOOL	OnRButtonUp(WPARAM fwKeys, REFPOINTS pts);
	virtual BOOL	OnSetCursor(HWND hWnd, INT nHittest, INT nMouseMsg);
	
	virtual BOOL	OnContextMenu(HWND hWnd, REFPOINTS pts);
	virtual BOOL	OnChar(TCHAR chCharCode, LPARAM lKeyData);
	virtual BOOL	OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData);
	virtual BOOL	OnSysKeyDown(WPARAM nVirtKey, LPARAM lKeyData);
	virtual BOOL	OnVScroll(int nScrollCode, int nPos, HWND hWnd);
	virtual BOOL	OnHScroll(int nScrollCode, int nPos, HWND hWnd);
	virtual BOOL	OnNCMouseMove(WPARAM nHittest, REFPOINTS pts);
	virtual BOOL	OnNCButtonDown(WPARAM nHittest, REFPOINTS pts);
	virtual BOOL	OnSetFocus(HWND hWndPrevFocus);

	//Protect against leaks (reusing the window without destroying it...)
	virtual	BOOL	IsDestroyed()						{ return !m_hWnd && !m_hWndParent && !m_pSubClassProc && !m_pwszClassName;	}

	//Data
	HWND			m_hWnd;
	HWND			m_hWndParent;
	WINDOWPLACEMENT	m_wndPlacement;
	WCHAR*			m_pwszClassName;
	UINT			m_nID;
	BOOL			m_bUnicodeMsg;

	//SubClass
	WNDPROC			m_pSubClassProc;
};



/////////////////////////////////////////////////////////////////////
// CMDIChildLite
//
/////////////////////////////////////////////////////////////////////
class CMDIChildLite : public CWndLite
{
public:
	//Constructors
	CMDIChildLite();
	virtual ~CMDIChildLite();

	//Window Proc
	static LRESULT WINAPI	MDIWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL			HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			UnhandledMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Messages
	virtual BOOL			OnMDIActivate(BOOL bActivate, HWND hWndActivate, HWND hWndDeactivate);

	//Helpers
	virtual BOOL			AutoPosition(BOOL fDefaultPosition = TRUE);

//protected:
	virtual	BOOL			Create(HWND hWndParent, WCHAR* pwszClassName,
								WCHAR* pwszWindowName, UINT uID, HICON hIcon,
								DWORD dwStyle = WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_BORDER, 
								DWORD dwExStyle = WS_EX_CLIENTEDGE,
								int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, 
								int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT);

//protected:
	//data
};


/////////////////////////////////////////////////////////////////////
// CFrameWndLite
//
/////////////////////////////////////////////////////////////////////
class CFrameWndLite : public CWndLite
{
public:
	//constructors
	CFrameWndLite();
	virtual ~CFrameWndLite();

	virtual	BOOL	Create(HWND hWndParent, WCHAR* pwszClassName,
								WCHAR* pwszWindowName, UINT uID, HICON hIcon,
								DWORD dwStyle = WS_OVERLAPPEDWINDOW, 
								DWORD dwExStyle = WS_EX_APPWINDOW,
								int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, 
								int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT);
	
	virtual BOOL	PreTranslateMessage(MSG* pmsg);

	//Messages
	virtual BOOL	OnNotify(INT idCtrl, NMHDR* pNMHDR);
	virtual BOOL	OnClose();
	virtual BOOL	OnDestroy();

	//Messages
	virtual void	OnToolTip(INT idCtrl, NMHDR* pNMHDR);
};



/////////////////////////////////////////////////////////////////////
// CMDIFrameLite
//
/////////////////////////////////////////////////////////////////////
class CMDIFrameLite : public CFrameWndLite
{
public:
	//constructors
	CMDIFrameLite();
	virtual ~CMDIFrameLite();

	//Overloads
	virtual LRESULT	UnhandledMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL	PreTranslateMessage(MSG* pmsg);

	//Overloads
	virtual BOOL	OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL	OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl);
	virtual BOOL	OnNotify(INT idCtrl, NMHDR* pNMHDR);
	virtual BOOL	OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags);

	//Messages
	virtual BOOL	OnCreate(CREATESTRUCT* pCREATESTRUCT);
	virtual BOOL	OnCreateClient(CREATESTRUCT* pCREATESTRUCT);
	virtual BOOL	OnDestroy();

	//Helpers
	virtual void	OnAutoPosition();

	//Helpers
	virtual void	MDIActivate(HWND hWnd)	{ SendMessage(m_hWndMDIClient, WM_MDIACTIVATE, (WPARAM)hWnd, 0);	} 
	virtual void	MDIDestroy(HWND hWnd)	{ SendMessage(m_hWndMDIClient, WM_MDIDESTROY, (WPARAM)hWnd, 0);		}

	//Helpers
	virtual CMDIChildLite* GetActiveWindow(WCHAR* pwszClassName = NULL);
	virtual CMDIChildLite* FindWindow(WCHAR* pwszClassName = NULL);

	//Protect against leaks (reusing the window without destroying it...)
	virtual	BOOL	IsDestroyed()			{ return !m_hWndMDIClient && CFrameWndLite::IsDestroyed();			}

//protected:
	//Data
	HWND	m_hWndMDIClient;
};


/////////////////////////////////////////////////////////////////////
// CDialogLite
//
/////////////////////////////////////////////////////////////////////
class CDialogLite : public CWndLite
{
public:
	//constructors
	CDialogLite(UINT uIDD = 0);
	virtual ~CDialogLite();

	//members
	virtual LRESULT			DoModal(HWND hWndParent);		//Modal
	virtual HWND			CreateDlg(HWND hWndParent);		//Modeless (non-Modal)
	virtual void			EndDialog(int nResult);

	//Dialog Proc
	static	INT_PTR WINAPI	DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL			HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//protected:
	virtual BOOL			OnInitDialog();
	virtual BOOL			OnActivate(UINT fActive, UINT fMinimized, HWND hWndPrevious);

	virtual BOOL			OnOK();
	virtual BOOL			OnCancel();
	
	//Data
	UINT			m_uIDD;
	BOOL			m_fModal;
};



/////////////////////////////////////////////////////////////////////
// CSplitterLite
//
/////////////////////////////////////////////////////////////////////
class CSplitterLite	: public CWndLite
{
public:
	//constructors
	CSplitterLite(HWND hWndParent = NULL, UINT nID = 0);

	//Helpers
	virtual BOOL	SetSplitter(CSplitterLite* pCWndTop, CSplitterLite* pCWndBottom = NULL, CSplitterLite* pCWndLeft = NULL, CSplitterLite* pCWndRight = NULL);
	virtual BOOL	StartTracking(REFPOINTS ptsScreen, REFRECT rectLeft, REFRECT rectRight, BOOL fVertical, POINT* pPoint);
	virtual BOOL	DrawSplitter(HDC hDC, BOOL fVertical, REFRECT rectLeft, REFRECT rectRight, REFPOINTS ptsClient);
	
	//Messages
	virtual BOOL	OnNCMouseMove(WPARAM nHittest, REFPOINTS pts);
	virtual BOOL	OnNCButtonDown(WPARAM nHittest, REFPOINTS pts);
	virtual BOOL	OnBorder(REFPOINTS pts, INT iBorder);

//protected:
	//Data
	CSplitterLite*	m_pCWndTop;
	CSplitterLite*	m_pCWndBottom;
	CSplitterLite*	m_pCWndLeft;
	CSplitterLite*	m_pCWndRight;

	HCURSOR			m_hCursorOld;
	HCURSOR			m_hCursorTop;
	HCURSOR			m_hCursorRight;

	INT				m_iBorderCX;
	INT				m_iBorderCY;
};


/////////////////////////////////////////////////////////////////////
// CEditBoxLite
//
/////////////////////////////////////////////////////////////////////
class CEditBoxLite : public CSplitterLite
{
public:
	//constructors
	CEditBoxLite(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CEditBoxLite();

	//Messages
	virtual BOOL	OnCreate(CREATESTRUCT* pCREATESTRUCT);

	//Interface
	virtual void	SetSel(INDEX iMin, INDEX iMax)							{ SendMessage(m_hWnd, EM_SETSEL, (WPARAM)iMin, (LPARAM)iMax);	}
	virtual BOOL	GetSel(INDEX* piMin, INDEX* piMax)						{ CHARRANGE cr; BOOL bReturn = GetSel(&cr); if(piMin) *piMin = cr.cpMin;  if(piMax) *piMax = cr.cpMax; return bReturn;		}
	virtual BOOL	GetSel(CHARRANGE* pcr)									{ ASSERT(pcr); SendMessageA(m_hWnd, EM_GETSEL, (WPARAM)&pcr->cpMin, (LPARAM)&pcr->cpMax); return pcr->cpMin != pcr->cpMax;	}

	//Inline helpers, (that deal with hWnds)
	BOOL			SetReadOnly(bool bReadOnly)								{ return (BOOL)SendMessage(m_hWnd, EM_SETREADONLY, bReadOnly, 0L);								}
	INDEX			LineIndex(INDEX nLine) 									{ return (INDEX)SendMessage(m_hWnd, EM_LINEINDEX, nLine, 0);									}
	INDEX			LineLength(INDEX nStartChar)							{ return (INDEX)SendMessage(m_hWnd, EM_LINELENGTH, nStartChar, 0);								}
	void			LineScroll(INDEX nLines, INDEX nChars = 0)				{ SendMessage(m_hWnd, EM_LINESCROLL, nChars, nLines);											}
	LRESULT			SetOptions(UINT dwOptions, UINT fOperation = ECOOP_OR)	{ return SendMessage(m_hWnd, EM_SETOPTIONS, fOperation, dwOptions);								}
	
	BOOL			CanPaste()												{ return (BOOL)SendMessage(m_hWnd, EM_CANPASTE, 0, 0);											}
	void			Cut()													{ SendMessage(m_hWnd, WM_CUT, 0, 0);															}
	void			Copy()													{ SendMessage(m_hWnd, WM_COPY, 0, 0);															}	
	void			Paste()													{ SendMessage(m_hWnd, WM_PASTE, 0, 0);															}	
	BOOL			ScrollCaret()											{ return (BOOL)SendMessage(m_hWnd, EM_SCROLLCARET, 0, 0);										}

	virtual void	ReplaceSel(LPCWSTR pwszString, bool bCanUndo)			{ wSendMessage(m_hWnd, EM_REPLACESEL, (WPARAM)bCanUndo, (WCHAR*)pwszString);					}
	virtual void	ReplaceAll(LPCWSTR pwszString, bool bReplaceAll = FALSE, bool fHighlight = FALSE);

	INDEX			GetLineCount()											{ return (INDEX)SendMessage(m_hWnd, EM_GETLINECOUNT, 0, 0);										}
	INDEX			GetLine(INDEX iLine, CHAR* pszBuffer, ULONG ulMaxSize);
	INDEX			GetTextRange(CHARRANGE& cr, WCHAR* pwszBuffer);
	INDEX			GetTextRange(CHARRANGE& cr, CHAR* pszBuffer);

	//Helpers
	BOOL			Save(LPCWSTR pwszFileName);

//protected:
	//data
};


/////////////////////////////////////////////////////////////////////////////
// COLORA
//
// These are the same as defined in CFM_*.  But we need a "enumerable" type
// so that we can easily pass these to operators, and it knows from the type
// which function to call
//
/////////////////////////////////////////////////////////////////////////////
enum COLORA
{
	CA_NORMAL			=	0,
	CA_BOLD				=	CFM_BOLD,
	CA_ITALIC			=	CFM_ITALIC,
	CA_UNDERLINE		=	CFM_UNDERLINE,
	CA_STRIKEOUT		=	CFM_STRIKEOUT,
	CA_PROTECTED		=	CFM_PROTECTED,
	CA_LINK				=	CFM_LINK,
	CA_AUTO_COLOR		=	CFE_AUTOCOLOR,
	CA_COLOR			=	CFM_COLOR,
	CA_DISABLE			=	CFM_CHARSET,
};

enum OUTPUT_SPECIAL
{
	NEWLINE				=	00000001,
};

/////////////////////////////////////////////////////////////////////////////
// CConsole
//
/////////////////////////////////////////////////////////////////////////////
class CConsole	/*: public CStringCache*/
{
public:
	//Constructors
	CConsole()
	{
		m_pwszNewLine	= wWndEOL;
	}
	virtual ~CConsole()
	{
	}

	//Interface
	inline	CConsole&	GetConsole()	{ return *this;	}

	//Strings
	virtual CConsole&	operator << (LPCWSTR pwsz )
	{
		Write(pwsz);
		return GetConsole();
	}
//	virtual CConsole&	operator << (OUTPUT_SPECIAL eSpecial);
//	virtual CConsole&	operator << (void* pv);

	//Overloads
	virtual HRESULT		Write(LPCWSTR pwsz) = 0;
	virtual HRESULT		WriteLine(LPCWSTR pwsz = NULL)
	{
		if(pwsz)
			Write(pwsz);
		return Write(m_pwszNewLine);
	}

	//Helpers
//	virtual HRESULT		WriteMBCS(LPCSTR pwsz);

	//Colors
	virtual void		SetColor(COLORREF refColor)
	{
	}
	virtual void		SetColorAttrib(COLORA attColor, BOOL fEnable = TRUE)
	{
	}
	virtual	CConsole&	operator << (COLORREF refColor)
	{
		SetColor(refColor);
		return GetConsole();
	}
	virtual	CConsole&	operator << (COLORA attColor)
	{
		SetColorAttrib(attColor);
		return GetConsole();
	}

protected:
	//Overloads
	//TODO: We should be implementing this here, the derived class of the console should...
//	virtual HRESULT WriteCore(ULONG cbBytes, BYTE* rgBytes, ULONG* pcbWritten = NULL);

	//Data
	WCHAR*				m_pwszNewLine;

private:	
	//NOTE: Don't use MBCS - requires expensive conversions...
//	virtual CConsole&	operator << (LPCSTR pwsz );
};


/////////////////////////////////////////////////////////////////////
// CRichEditLite
//
/////////////////////////////////////////////////////////////////////
class CRichEditLite : public CEditBoxLite, public CConsole
{
public:
	//constructors
	CRichEditLite(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CRichEditLite();

	//Messages
	virtual BOOL	OnCreate(CREATESTRUCT* pCREATESTRUCT);
	virtual BOOL	OnNotify(INT idCtrl, NMHDR* pNMHDR);
	virtual BOOL	SetWordWrap(BOOL fWordWrap);

	//Messages
	virtual BOOL	OnLink(INT idCtrl, ENLINK* pEnLink);

	//Abstract - CConsole implementation
	virtual HRESULT	Write(LPCWSTR pwszText)
	{
		Append(pwszText);
		return S_OK;
	}
	virtual void		SetColor(COLORREF refColor)
	{
		if(refColor == RGB_NONE)
		{
			//Restore color
			SetEffects(CFM_COLOR, CFE_AUTOCOLOR, 0);
		}
		else
		{
			//Change color
			SetEffects(CFM_COLOR, 0, refColor);
		}
	}
	virtual void		SetColorAttrib(COLORA attColor, BOOL fEnable = TRUE)
	{
		if(attColor == CA_NORMAL)
		{
			//Restore (all) Attributes
			//NOTE: Just simply using ULONG_MAX as the mask causes problems since
			//other things (ie: charset, etc) are also affected...
			SetEffects(CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_PROTECTED | CFM_LINK, 0, 0);
		}
		else
		{
			//Change Attributes
			SetEffects(attColor, fEnable ? attColor : ~attColor, 0);
		}
	}

	//Interface
	//NOTE: We overload Get and Set since for RichEdit controls these have different messages for > 64k
	virtual void	SetSel(INDEX iMin, INDEX iMax)							{ CHARRANGE cr = { (LONG)iMin, (LONG)iMax }; SendMessage(m_hWnd, EM_EXSETSEL, 0, (LPARAM)&cr);	}
	virtual BOOL	GetSel(INDEX* piMin, INDEX* piMax)						{ CHARRANGE cr; BOOL bReturn = GetSel(&cr); if(piMin) *piMin = cr.cpMin;  if(piMax) *piMax = cr.cpMax; return bReturn;		}
	virtual BOOL	GetSel(CHARRANGE* pcr)									{ ASSERT(pcr); SendMessageA(m_hWnd, EM_EXGETSEL, 0, (LPARAM)pcr); return pcr->cpMin != pcr->cpMax;	}

	//Inline helpers, (that deal with hWnds)
	INDEX			LineFromChar(INDEX nIndex)								{ return (INDEX)SendMessage(m_hWnd, EM_EXLINEFROMCHAR, 0, nIndex);			}
	
	BOOL			SetEffects(DWORD dwMask, DWORD	dwEffects, COLORREF	crTextColor);
	BOOL			SetCharFormat(DWORD dwMask, CHARFORMATW& charFormatW);

	virtual void	ReplaceSel(LPCWSTR pwszString, bool bCanUndo)			{ wSendMessage(m_hWnd, EM_REPLACESEL, (WPARAM)bCanUndo, (WCHAR*)pwszString);	}
	virtual void	ReplaceSel(LPCWSTR pwszString, bool bCanUndo, DWORD dwMask, DWORD dwColor);
	virtual void	Append(LPCWSTR pwszString, bool bCanUndo = TRUE, DWORD dwMask = 0, DWORD dwColor = 0);

	virtual INDEX	FindText(DWORD dwFindFlags, FINDTEXTEXW* pFINDTEXT);
	virtual BOOL	FindNext(DWORD dwFindFlags, WCHAR* pwszText);

	//Helpers
	LRESULT			SetDefaultFont();
	WCHAR*			GetSelectedText(BOOL fEntireLine = TRUE);


//protected:
	//data
};



/////////////////////////////////////////////////////////////////////
// CScrollBarLite
//
/////////////////////////////////////////////////////////////////////
class CScrollBarLite : public CSplitterLite
{
public:
	//constructors
	CScrollBarLite();
	virtual ~CScrollBarLite();

	//Messages

	//Inline helpers, (that deal with hWnds)
	virtual INT		SetScrollPos( int nPos, BOOL bRedraw = TRUE )				{ return ::SetScrollPos(m_hWnd, SB_CTL, nPos, bRedraw);			}
	virtual INT		GetScrollPos()												{ return ::GetScrollPos(m_hWnd, SB_CTL);						}

	virtual BOOL	SetScrollRange( int nMin, int nMax, BOOL bRedraw = TRUE )	{ return ::SetScrollRange(m_hWnd, SB_CTL, nMin, nMax, bRedraw);	}
	virtual BOOL	GetScrollRange( int* pnMin, int* pnMax )					{ return ::GetScrollRange(m_hWnd, SB_CTL, pnMin, pnMax);		}

	//Helpers
	virtual INT		SetScroll( int nPos, BOOL bRedraw = TRUE );
	virtual INT		SetScrollInfo(INT iPos, INT iRangeSize, INT iPageSize, BOOL fRedraw = TRUE);
	virtual BOOL	GetScrollInfo(INT* piPos, INT* piRangeSize, INT* piPageSize);

//protected:
	//data
};


/////////////////////////////////////////////////////////////////////
// CListBoxLite
//
/////////////////////////////////////////////////////////////////////
class CListBoxLite : public CWndLite
{
public:
	//constructors
	CListBoxLite(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CListBoxLite();

	//Messages
	virtual BOOL	OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl);

	//Overloads
	virtual BOOL	OnDblClk(INT iID, HWND hWndCtrl);

	//Inline helpers, (that deal with hWnds)
	virtual INDEX	SetCurSel(WPARAM nSelect)					{ return (INDEX)::SendMessage(m_hWnd,	LB_SETCURSEL, nSelect, 0);			}
	virtual INDEX	GetCurSel() const							{ return (INDEX)::SendMessage(m_hWnd,	LB_GETCURSEL, 0, 0);						}
	virtual INDEX	AddString(WCHAR* pwszString)				{ return (INDEX)wSendMessage(m_hWnd,	LB_ADDSTRING,	0, pwszString);				}
	virtual INDEX	DeleteString(WPARAM wParam)					{ return (INDEX)wSendMessage(m_hWnd,	LB_DELETESTRING, wParam, 0);		}
	virtual INDEX	GetText(INDEX nIndex, WCHAR* pwszBuffer)	{ return (INDEX)wSendMessage(m_hWnd,	LB_GETTEXT, nIndex, pwszBuffer);			}
	virtual INDEX	GetTextLen(INDEX nIndex)					{ return (INDEX)::SendMessage(m_hWnd,	LB_GETTEXTLEN, nIndex, 0); }

	virtual BOOL	SetItemParam(INDEX nIndex, LPARAM lParam)	{ return ::SendMessage(m_hWnd,	LB_SETITEMDATA, nIndex, lParam) != CB_ERR;			}
	virtual LPARAM	GetItemParam(INDEX nIndex) const			{ return ::SendMessage(m_hWnd,	LB_GETITEMDATA, nIndex, 0);					}

	virtual INDEX	GetCount()									{ return (INDEX)::SendMessage(m_hWnd, LB_GETCOUNT, 0, 0);	}
	//Helpers
	
//protected:
	//data
};


/////////////////////////////////////////////////////////////////////
// CListViewLite
//
/////////////////////////////////////////////////////////////////////
class CListViewLite : public CSplitterLite
{
public:
	//constructors
	CListViewLite(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CListViewLite();

	//Messages
	virtual BOOL	OnCreate(CREATESTRUCT* pCREATESTRUCT);
	virtual BOOL	OnNotify(INT idCtrl, NMHDR* pNMHDR);

	virtual BOOL	OnColumnClick(INT idCtrl, NMLISTVIEW* pNMListView);
	virtual BOOL	OnItemActivate(INT idCtrl, NMLISTVIEW* pNMListView);
	virtual BOOL	OnItemChanged(INT idCtrl, NMLISTVIEW* pNMListView);

	//Inline helpers, (that deal with hWnds)
	BOOL			DeleteColumn(INDEX nCol)								{ return (BOOL)SendMessage(m_hWnd, LVM_DELETECOLUMN, nCol, 0);								}
	BOOL			DeleteItem(INDEX iItem)									{ return (BOOL)SendMessage(m_hWnd, LVM_DELETEITEM, iItem, 0L);								}
	BOOL			DeleteAllItems()										{ return (BOOL)SendMessage(m_hWnd, LVM_DELETEALLITEMS, 0, 0L);								}
	INDEX			GetNextItem(INDEX iItem, INT nFlags)					{ return (INDEX)SendMessage(m_hWnd, LVM_GETNEXTITEM, iItem, MAKELPARAM(nFlags, 0));			}
	INDEX			GetColumnWidth(INDEX nCol)								{ return (INDEX)SendMessage(m_hWnd, LVM_GETCOLUMNWIDTH, nCol, 0);							}
	BOOL			SetColumnWidth(INDEX nCol, INT cx)						{ return (BOOL)SendMessage(m_hWnd, LVM_SETCOLUMNWIDTH, nCol, MAKELPARAM(cx, 0));			}
	UINT			GetItemState(INDEX iItem, UINT nMask)					{ return (UINT)SendMessage(m_hWnd, LVM_GETITEMSTATE, iItem, nMask);							}
	BOOL			EnsureVisible(INDEX iItem, bool fPartialOK = FALSE)		{ return (BOOL)SendMessage(m_hWnd, LVM_ENSUREVISIBLE, iItem, MAKELPARAM(fPartialOK, 0));	}
	INDEX			GetItemCount()											{ return (INDEX)SendMessage(m_hWnd, LVM_GETITEMCOUNT, 0, 0);								}
	INDEX			GetCountPerPage()										{ return (INDEX)SendMessage(m_hWnd, LVM_GETCOUNTPERPAGE, 0, 0);								}
	HWND			EditLabel(INDEX iItem)									{ return (HWND)SendMessage(m_hWnd, LVM_EDITLABEL, iItem, 0L);								}

	//Helpers
	LPARAM			GetItemParam(INDEX iItem);
	INDEX			GetItemText(INDEX iItem, INDEX iSubItem, WCHAR* wszName, ULONG ulMaxChars);
	INDEX			InsertColumn(INDEX iColumn, WCHAR* wszName, INT iImage = IMAGE_NONE);
	BOOL			SetColumn(INDEX iColumn, WCHAR* wszName, INT iImage = IMAGE_NONE);

	INDEX			InsertItem(INDEX iItem, INDEX iSubItem, WCHAR* wszName, LPARAM lParam = PARAM_NONE, INT iImage = IMAGE_NONE, UINT iState = IMAGE_NONE, UINT iStateMask = IMAGE_NONE);
	BOOL			SetItemText(INDEX iItem, INDEX iSubItem, WCHAR* pwszName);
	BOOL			SetItemState(INDEX iItem, INDEX iSubItem, UINT iState, UINT iStateMask);
	INDEX			HitTest(REFPOINTS pts, UINT* pFlags = NULL, BOOL fClientCoords = TRUE);
	
	INT				GetItemImage(INDEX iItem, INDEX iSubItem = 0);
	BOOL			SetItemImage(INDEX iItem, INDEX iSubItem, INT iImage);
	BOOL			SetItemParam(INDEX iItem, INDEX iSubItem, LPARAM lParam);
};



/////////////////////////////////////////////////////////////////////
// CTreeViewLite
//
/////////////////////////////////////////////////////////////////////
class CTreeViewLite : public CSplitterLite
{
public:
	//constructors
	CTreeViewLite(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CTreeViewLite();

	//Messages
	virtual BOOL	OnCreate(CREATESTRUCT* pCREATESTRUCT);
	virtual BOOL	OnNotify(INT idCtrl, NMHDR* pNMHDR);

	virtual BOOL	OnSelChanged(INT idCtrl, NMTREEVIEW* pNMTreeView);
	virtual BOOL	OnBeginLabelEdit(INT idCtrl, NMTVDISPINFO* ptvdi);
	virtual BOOL	OnEndLabelEdit(INT idCtrl, NMTVDISPINFO* ptvdi);
	virtual BOOL	OnBeginDrag(INT idCtrl, NMTREEVIEW* pNMTreeView);
	virtual BOOL	OnMouseMove(WPARAM nHittest, REFPOINTS pts);
	virtual BOOL	OnLButtonUp(WPARAM fwKeys, REFPOINTS pts);

	//Inline helpers, (that deal with hWnds)
	BOOL			SelectItem(HTREEITEM hItem)								{ ASSERT(IsWindow(m_hWnd));		return (BOOL)SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);		}
	HTREEITEM		GetSelectedItem()										{ ASSERT(IsWindow(m_hWnd));		return (HTREEITEM)SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CARET, 0);				}
	BOOL			DeleteAllItems()										{ ASSERT(IsWindow(m_hWnd));		return (BOOL)SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);				}
	HTREEITEM		GetNextItem(HTREEITEM hItem, UINT nCode = TVGN_NEXT)	{ ASSERT(IsWindow(m_hWnd));		return (HTREEITEM)SendMessage(m_hWnd, TVM_GETNEXTITEM, nCode, (LPARAM)hItem);		}
	HTREEITEM		GetChildItem(HTREEITEM hItem)							{ ASSERT(IsWindow(m_hWnd));		return (HTREEITEM)SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hItem);	}
	HTREEITEM		GetParentItem(HTREEITEM hItem) 							{ ASSERT(IsWindow(m_hWnd));		return (HTREEITEM)SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);	}
	BOOL			DeleteItem(HTREEITEM hItem)								{ ASSERT(IsWindow(m_hWnd));		return (BOOL)SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)hItem);					}
	BOOL			EnsureVisible(HTREEITEM hItem)							{ ASSERT(IsWindow(m_hWnd));		return (BOOL)SendMessage(m_hWnd, TVM_ENSUREVISIBLE, 0, (LPARAM)hItem);				}
	BOOL			Expand(HTREEITEM hItem, UINT flag = TVE_EXPAND)			{ ASSERT(IsWindow(m_hWnd));		return (BOOL)SendMessage(m_hWnd, TVM_EXPAND, flag, (LPARAM)hItem);					}

	//Helpers
	LPARAM			GetItemParam(HTREEITEM hItem);
	HTREEITEM		HitTest(REFPOINTS pts, UINT* pFlags = NULL, BOOL fClientCoords = TRUE);
	BOOL			SetItemState(HTREEITEM hItem, UINT iState, UINT iStateMask);
	BOOL			SetItemImage(HTREEITEM hItem, INT iImage, INT iSelectedImage);
	BOOL			SetItemParam(HTREEITEM hItem, LPARAM lParam);

	HTREEITEM		InsertItem(HTREEITEM hParent, HTREEITEM hInsAfter, WCHAR* wszName, LPARAM lParam = PARAM_NONE, INT iImage = IMAGE_NONE, INT iSelectedImage = IMAGE_NONE, UINT iState = IMAGE_NONE, UINT iStateMask = IMAGE_NONE);
	BOOL			SetItemText(HTREEITEM hItem, WCHAR* pwszText);

protected:
	//Data
	HTREEITEM		m_hDraggingItem;
};



/////////////////////////////////////////////////////////////////////
// CComboBoxLite
//
/////////////////////////////////////////////////////////////////////
class CComboBoxLite : public CWndLite
{
public:
	//constructors
	CComboBoxLite(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CComboBoxLite();

	//Messages
	virtual BOOL		OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl);

	//Overloads
	virtual BOOL		OnSelChange(INT iID, HWND hWndCtrl);
	virtual BOOL		OnDropDown(INT iID, HWND hWndCtrl);

	//Inline helpers, (that deal with hWnds)
	virtual INDEX		GetCount()										{ return (INDEX)::SendMessage(m_hWnd,	CB_GETCOUNT, 0, 0);					}
	virtual INDEX		GetCurSel() const								{ return (INDEX)::SendMessage(m_hWnd,	CB_GETCURSEL, 0, 0);				}
	virtual INDEX		SetCurSel(WPARAM nSelect)						{ return (INDEX)::SendMessage(m_hWnd,	CB_SETCURSEL, nSelect, 0);			}
	
	virtual INDEX		AddString(WCHAR* pwszString)					{ return (INDEX)::wSendMessage(m_hWnd,	CB_ADDSTRING,	0, pwszString);		}
	virtual INDEX		DeleteString(INDEX nIndex)						{ return (INDEX)::SendMessage(m_hWnd,	CB_DELETESTRING, nIndex, 0);		}
	virtual BOOL		SetItemParam(INDEX nIndex, LPARAM lParam)		{ return ::SendMessage(m_hWnd,	CB_SETITEMDATA, nIndex, lParam) != CB_ERR;	}
	virtual void		ResetContent()									{ ::SendMessage(m_hWnd,	CB_RESETCONTENT, 0, 0);								}		

	virtual LPARAM		GetItemParam(INDEX nIndex) const					{ return ::SendMessage(m_hWnd,	CB_GETITEMDATA, nIndex, 0);					}
	virtual LRESULT		GetTextLength(INDEX nIndex)						{ return ::SendMessage(m_hWnd,	CB_GETLBTEXTLEN, nIndex, 0);				}
	virtual INDEX		GetText(INDEX nIndex, WCHAR* pwszBuffer)		{ return (INDEX)::wSendMessage(m_hWnd,	CB_GETLBTEXT, nIndex, pwszBuffer);	}
	virtual INDEX		FindString(WCHAR* pwszString, INDEX nStartIndex = -1)				{ return (INDEX)wSendMessage(m_hWnd,	CB_FINDSTRING,		nStartIndex, pwszString);		}
	virtual INDEX		FindStringExact(WCHAR* pwszString, INDEX nStartIndex = -1)			{ return (INDEX)wSendMessage(m_hWnd,	CB_FINDSTRINGEXACT,	nStartIndex, pwszString);		}
	
	//Helpers
	virtual INDEX		GetSelText(WCHAR* pwszBuffer, ULONG ulMaxSize);
	virtual INDEX		SetSelText(WCHAR* pwszBuffer, BOOL fAddItem = FALSE);
	virtual INDEX		SetSelValue(LPARAM lParam);
	virtual LPARAM		GetSelValue();
	virtual INDEX		AddString(WCHAR* pwszString, LPARAM lParam);
	
	//Helpers
	virtual INDEX		SaveSelection()									{ return m_iSavedSel = GetCurSel();											}
	virtual	INDEX		RestoreSelection()								{ return SetCurSel(m_iSavedSel);											}
	virtual BOOL		Populate(ULONG cItems, const WIDENAMEMAP* rgItems);


//protected:
	//data
	INDEX		m_iSavedSel;
};


/////////////////////////////////////////////////////////////////////
// CButtonLite
//
/////////////////////////////////////////////////////////////////////
class CButtonLite : public CWndLite
{
public:
	//constructors
	CButtonLite(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CButtonLite();

	//Inline helpers, (that deal with hWnds)
	virtual UINT	IsButtonChecked()				{ return ::IsDlgButtonChecked(m_hWndParent, m_nID);							}
	virtual BOOL	CheckButton(UINT uCheck)		{ ASSERT(uCheck == BST_CHECKED || uCheck == BST_UNCHECKED || BST_INDETERMINATE); return ::CheckDlgButton(m_hWndParent, m_nID, uCheck);		}

//protected:
	//data
};


/////////////////////////////////////////////////////////////////////
// CComboBoxEx
//
/////////////////////////////////////////////////////////////////////
class CComboBoxEx : public CComboBoxLite
{
public:
	//constructors
	CComboBoxEx(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CComboBoxEx();

	//Overrides
	virtual BOOL		OnInitialUpdate();
	
	//Messages
	INDEX				InsertItem(INDEX iItem, WCHAR* pwszName, LPARAM lParam = PARAM_NONE, INT iIndent = PARAM_NONE, INT iImage = IMAGE_NONE, UINT iSelectedImage = IMAGE_NONE);

	//Inline helpers, (that deal with hWnds)
	//NOTE: AddString is not processed by ComboBoxEx so we have to fabricate the functionality
	virtual INDEX		AddString(WCHAR* pwszString)										{ return InsertItem(-1/*iItem*/, pwszString);								}
	virtual INDEX		AddString(WCHAR* pwszString, LPARAM lParam)							{ return InsertItem(-1/*iItem*/, pwszString, lParam);						}

	virtual BOOL		SetUnicodeFormat(BOOL fUnicode = TRUE)								{ return (BOOL)::SendMessage(m_hWnd, CBEM_SETUNICODEFORMAT, fUnicode, 0);	}
	
	//NOTE: FindString is not processed by ComboBoxEx, but FindStringExact is...
	virtual INDEX		FindString(WCHAR* pwszString, INDEX nStartIndex = -1)				{ ASSERT(!L"FindString is not processed by ComboBoxEx, change code to use FindStringExact.");   return FindStringExact(pwszString, nStartIndex);	}

	//protected:
	//data
};


/////////////////////////////////////////////////////////////////////
// CToolBarLite
//
/////////////////////////////////////////////////////////////////////
class CToolBarLite : public CWndLite
{
public:
	//constructors
	CToolBarLite();
	virtual ~CToolBarLite();

	virtual BOOL Create(HWND hWndParent, UINT nID,  UINT nBitmapID, UINT cButtons, TBBUTTON* rgButtons, DWORD dwStyle = WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS /*| TBSTYLE_WRAPABLE | TBSTYLE_ALTDRAG | CCS_ADJUSTABLE*/);

	//Helpers
	virtual BOOL EnableButton(UINT nID, bool fEnable)			{ return (BOOL)SendMessage(m_hWnd, TB_ENABLEBUTTON, nID, fEnable);				}
};


/////////////////////////////////////////////////////////////////////
// CStatusBarLite
//
/////////////////////////////////////////////////////////////////////
class CStatusBarLite : public CWndLite
{
public:
	//constructors
	CStatusBarLite();
	virtual ~CStatusBarLite();

	//Creation
	virtual BOOL Create(HWND hWndParent, UINT nID,  WCHAR* pwszText = L"", DWORD dwStyle = WS_CHILD | WS_VISIBLE);

	//Helpers
	virtual BOOL SetText(WCHAR* pwszText, int nPane, int nType)	{ return (BOOL)wSendMessage(m_hWnd, SB_SETTEXTW, (nPane|nType), pwszText);		}
	virtual BOOL SetParts(int nParts, int* pWidths)				{ return (BOOL)SendMessage(m_hWnd, SB_SETPARTS, nParts, (LPARAM)pWidths);		}
};


/////////////////////////////////////////////////////////////////////
// CPropPageLite
//
/////////////////////////////////////////////////////////////////////
class CPropPageLite : public CDialogLite
{
public:
	//constructors
	CPropPageLite(UINT uIDD, TCHAR* ptszTitle = NULL);
	virtual ~CPropPageLite();

	//Dialog Proc
	static INT_PTR WINAPI	PropPageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL			HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Interface
	virtual void				SetParent(CPropSheetLite* pCPropSheetLite);
	virtual CPropSheetLite*		GetParent()	{ return m_pCPropSheet; }

//protected:
	virtual BOOL	OnSetActive();
	virtual BOOL	OnKillActive();

	virtual BOOL	OnOK();
	virtual BOOL	OnApply();

	virtual BOOL	OnCancel();
	virtual BOOL	OnReset();
	
	//Data
	PROPSHEETPAGE   m_psPage;

	//Parent Property Sheet
	CPropSheetLite*	m_pCPropSheet;
};


/////////////////////////////////////////////////////////////////////
// CPropSheetLite
//
/////////////////////////////////////////////////////////////////////
class CPropSheetLite : public CWndLite
{
public:
	//constructors
	CPropSheetLite(TCHAR* ptszTitle = NULL, HICON hIcon = NULL);
	virtual ~CPropSheetLite();

//protected:
	virtual void		AddPage(CPropPageLite* pCPropPageLite);
	virtual LRESULT		DoModal(HWND hWndParent, INT nStartPage = 0);

	//Data
	PROPSHEETHEADER m_psHeader;
};



/////////////////////////////////////////////////////////////////////////////
// CTabLite
//
/////////////////////////////////////////////////////////////////////////////
class CTabLite : public CSplitterLite
{
public:
	//constructors
	CTabLite(HWND hWndParent = NULL, UINT nID = 0);
	virtual ~CTabLite();

	//messages
	virtual BOOL	OnCreate(CREATESTRUCT* pCREATESTRUCT);
	virtual BOOL	OnNotify(INT idCtrl, NMHDR* pNMHDR);

	virtual BOOL	OnSelChange(INT iID, HWND hWndCtrl);
	virtual BOOL	OnSelChanging(INT iID, HWND hWndCtrl);

	//Inlines
	virtual INDEX	GetItemCount()											{ return (INDEX)::SendMessageA(m_hWnd, TCM_GETITEMCOUNT, 0, 0);			}
	virtual INDEX	GetCurSel() const										{ return (INDEX)::SendMessageA(m_hWnd, TCM_GETCURSEL, 0, 0);			}
	virtual INDEX	SetCurSel(INDEX iItem, BOOL fSendNotification = FALSE);

	virtual BOOL	DeleteAllItems()										{ return (BOOL)::SendMessageA(m_hWnd, TCM_DELETEALLITEMS, 0, 0);		}
	virtual BOOL	DeleteItem(INDEX iItem)									{ return (BOOL)::SendMessageA(m_hWnd, TCM_DELETEITEM, iItem, 0);		}
	virtual void	AdjustRect(RECT* pRect, BOOL fLarger = FALSE)			{ SendMessageA(m_hWnd, TCM_ADJUSTRECT, fLarger, (LPARAM)pRect);			}
	virtual INDEX	FindTab(LPARAM lParam);

	//Helpers
	BOOL			GetItem(INDEX iItem, WCHAR* pwszText, ULONG ulMaxLength, LPARAM* plParam = NULL);
	LPARAM			GetItemData(INDEX iItem);

	virtual BOOL	SetItem(INDEX iItem, LPCWSTR pwszName, LPARAM lParam = PARAM_NONE, INT iImage = IMAGE_NONE, DWORD dwState = STATE_NONE, DWORD dwStateMask = STATE_NONE);
	virtual BOOL	SetItemText(INDEX iItem, LPCWSTR pwszName);

	virtual INDEX	InsertItem(INDEX iItem, LPCWSTR pwszText, LPARAM lParam = PARAM_NONE, INT iImage = IMAGE_NONE, DWORD dwState = STATE_NONE, DWORD dwStateMask = STATE_NONE);

//protected:
	//data
};



//////////////////////////////////////////////////////////////////////////
// CCmdLineLite
//
//////////////////////////////////////////////////////////////////////////
class CCmdLineLite
{
protected:

	// Attributes
public:
	CCmdLineLite();
	virtual ~CCmdLineLite();

// Operations
	virtual void ResetParser();

//protected:
	virtual void ParseParam(WCHAR* pwszParam, BOOL bFlag, BOOL bLast );
	virtual BOOL ParseCommandLine();
	virtual BOOL ParseFile(WCHAR* pwszFileName);

	WCHAR	m_wszLastCmd[MAX_QUERY_LEN];
};


/////////////////////////////////////////////////////////////////////
// CWaitCursor
//
/////////////////////////////////////////////////////////////////////
class CWaitCursor
{
public:
	//Constructors
	CWaitCursor();
	virtual ~CWaitCursor();

	//Helpers
	void WaitCursor();
	void RestoreCursor();

protected:
	//Data
	HCURSOR m_hPrevCursor;
};


static const WCHAR UNICODE_BYTE_ORDER_MARK = 0xFEFF;
/////////////////////////////////////////////////////////////////////
// CFileLite
//
/////////////////////////////////////////////////////////////////////
class CFileLite
{
public:
	//constructor
	CFileLite();
	virtual ~CFileLite();

	//Open
	virtual HRESULT Open(LPCWSTR pwszFileName, DWORD dwDesiredAccess = GENERIC_READ, DWORD dwShareMode = FILE_SHARE_READ, DWORD dwCreationDisposition = OPEN_EXISTING, BOOL fDisplayDialog = FALSE, HWND hWndParent = NULL);
	virtual HRESULT Open(LPCSTR pszFileName, DWORD dwDesiredAccess = GENERIC_READ, DWORD dwShareMode = FILE_SHARE_READ, DWORD dwCreationDisposition = OPEN_EXISTING, BOOL fDisplayDialog = FALSE, HWND hWndParent = NULL);
	
	//Low Level Byte Operations
	virtual HRESULT ReadBytes(ULONG cbBytes, BYTE* rgBytes, ULONG* pcbRead = NULL);
	virtual HRESULT WriteBytes(ULONG cBytes, BYTE* rgBytes, ULONG* pcbWritten = NULL);
	
	//Read
	virtual HRESULT Read(ULONG cbBytes, WCHAR* pwszText, ULONG* pcbRead = NULL);
	virtual HRESULT Read(ULONG cbBytes, CHAR* pszText, ULONG* pcbRead = NULL);
	
	//Write
	virtual HRESULT Write(LPCWSTR pwszText, ULONG* pcbWritten = NULL);
	virtual HRESULT Write(LPCSTR pszText, ULONG* pcbWritten = NULL);
	virtual HRESULT WriteFormat(LPCWSTR pwszFmt, ...);

	//Helpers
	virtual HRESULT Flush();
	virtual HRESULT Close();
	virtual DWORD	Seek(LONG lDistanceToMove, DWORD dwOrigin);

	//Interface
	virtual BOOL	IsOpen()							{ return m_hFile != NULL;	}
	virtual BOOL    IsEOF()								{ return m_bEOF;			}

	//Unicode vs. MBCS
	virtual BOOL	IsUnicode()							{ return m_bUnicode;		}
	virtual void	SetUnicode(BOOL bUnicode = TRUE)	{ m_bUnicode = bUnicode;	}
	virtual BOOL	DetermineUnicode();

private:
	//Data
	HANDLE	m_hFile;
	BOOL	m_bEOF;

	BOOL	m_bUnicode;
	ULONG	m_cbWritten;
	ULONG	m_cbRead;
};


/////////////////////////////////////////////////////////////////////
// CDropObject
//
/////////////////////////////////////////////////////////////////////
class CDropObject : public IDataObject
{
public:
	//constructor
	CDropObject();
	virtual ~CDropObject();

	//IUnknown
	STDMETHODIMP			QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)	AddRef();
    STDMETHODIMP_(ULONG)	Release();

	//Interface
    STDMETHODIMP			GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium);
    STDMETHODIMP			GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium);
    STDMETHODIMP			QueryGetData(FORMATETC* pformatetc);
    STDMETHODIMP			GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut);
    STDMETHODIMP			SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);
    STDMETHODIMP			EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);
    STDMETHODIMP			DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection);
    STDMETHODIMP			DUnadvise(DWORD dwConnection);
    STDMETHODIMP			EnumDAdvise(IEnumSTATDATA** ppenumAdvise);

private:
	//Data

	//IUnknown
	ULONG					m_cRef;
};


/////////////////////////////////////////////////////////////////////
// CDropSource
//
/////////////////////////////////////////////////////////////////////
class CDropSource : public IDropSource
{
public:
	//constructor
	CDropSource();
	virtual ~CDropSource();

	//IUnknown
	STDMETHODIMP			QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)	AddRef();
    STDMETHODIMP_(ULONG)	Release();

	//Interface
    STDMETHODIMP			QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
    STDMETHODIMP			GiveFeedback(DWORD dwEffect);

private:
	//Data

	//IUnknown
	ULONG					m_cRef;
};


/////////////////////////////////////////////////////////////////////
// CDropTarget
//
/////////////////////////////////////////////////////////////////////
class CDropTarget : public IDropTarget
{
public:
	//constructor
	CDropTarget();
	virtual ~CDropTarget();

	//IUnknown
	STDMETHODIMP			QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)	AddRef();
    STDMETHODIMP_(ULONG)	Release();

	//Interface
    STDMETHODIMP			DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP			DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    STDMETHODIMP			DragLeave();
    STDMETHODIMP			Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);

private:
	//Data

	//IUnknown
	ULONG					m_cRef;
};



///////////////////////////////////////////////////////////////////////////////
// COleClient
// 
///////////////////////////////////////////////////////////////////////////////
class COleClient : public CWndLite, 
//					public IDocHostUIHandler, 
//					public IDocHostShowUI, 
					public IOleClientSite, 
					public IOleInPlaceSite, 
					public IDispatch
{
public:
	//Constructors
	COleClient(HWND hWndParent = NULL, UINT nID = 0)
		: CWndLite(hWndParent, nID)
	{
		m_cRef = 0;
	}
	virtual ~COleClient()
	{
	}
	
	//IUnknown
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
	{
		*ppvObject = NULL;
	   if(riid == IID_IUnknown)
		   *ppvObject = (IOleClientSite*)this;
	   else if(riid == IID_IDispatch)
		   *ppvObject = (IDispatch*)this;
	   else if(riid == IID_IOleClientSite)
		   *ppvObject = (IOleClientSite*)this;
	   else if(riid == IID_IOleInPlaceSite)
		   *ppvObject = (IOleInPlaceSite*)this;
	   else if(riid == IID_IOleWindow)
		   *ppvObject = (IOleWindow*)this;
	   else if(riid == DIID_DWebBrowserEvents2)
		   *ppvObject = (IDispatch*)this;
		else
		   return E_NOINTERFACE;

	   return S_OK;
	}

    STDMETHOD_(ULONG, AddRef)()
	{
		return ++m_cRef;
	}

    STDMETHOD_(ULONG, Release)()
	{
		return --m_cRef;
	}

	//IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
		LCID lcid, DISPID* rgdispid)
	{
		return E_NOTIMPL;
	}

	STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                               DISPPARAMS* pDispParams, VARIANT* pvarResult,
                               EXCEPINFO*  pExcepInfo,  UINT* puArgErr)

	{
		return E_NOTIMPL;
	}

	//IOleClientSite
   STDMETHOD(SaveObject)(void)
	{
      return S_OK;
	}

   STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
	{
		ATLTRACENOTIMPL(_T("IOleClientSite::GetMoniker"));
	}

   STDMETHOD(GetContainer)(IOleContainer **ppContainer)
	{
		ATLTRACENOTIMPL(_T("IOleClientSite::GetContainer"));
	}

   STDMETHOD(ShowObject)(void)
	{
      return S_OK;
	}

   STDMETHOD(OnShowWindow)(BOOL fShow)
	{
      return S_OK;
	}

   STDMETHOD(RequestNewObjectLayout)(void)
	{
      return S_OK;
	}

 // IOleWindow
   STDMETHOD(GetWindow)(HWND *phwnd)
	{
	   *phwnd = m_hWnd;
      return S_OK;
	}

   STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode)
	{
      return S_OK;
	}

 // IOleInPlaceSite
   STDMETHOD(CanInPlaceActivate)(void)
	{
	   return S_OK;
	}

   STDMETHOD(OnInPlaceActivate)(void)
	{
		return S_OK;
	}

   STDMETHOD(OnUIActivate)(void)
	{
      return S_OK;
	}

   STDMETHOD(GetWindowContext)(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
	                            LPRECT lprcPosRect, LPRECT lprcClipRect,
                               LPOLEINPLACEFRAMEINFO lpFrameInfo)
	{
		GetClientRect(m_hWnd, lprcPosRect);
		GetClientRect(m_hWnd, lprcClipRect);
		return S_OK;
	}

   STDMETHOD(Scroll)(SIZE scrollExtant)
	{
      return S_OK;
	}

   STDMETHOD(OnUIDeactivate)(BOOL fUndoable)
	{
      return S_OK;
	}

   STDMETHOD(OnInPlaceDeactivate)( void)
	{
		return S_OK;
	}

   STDMETHOD(DiscardUndoState)( void)
	{
      return S_OK;
	}

   STDMETHOD(DeactivateAndUndo)( void)
	{
      return S_OK;
	}

   STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect)
	{
      return S_OK;
	}

protected:
	//Data

	//IUnknown
	ULONG					m_cRef;
};



///////////////////////////////////////////////////////////////////////////////
// CWebBrowser
// 
///////////////////////////////////////////////////////////////////////////////
class CWebBrowser : public COleClient
{
public:
	//Constructors
	CWebBrowser(HWND hWndParent = NULL, UINT nID = 0)
		: COleClient(hWndParent, nID)
	{
	}
	virtual ~CWebBrowser()
	{
	}

	//Helpers
	virtual BOOL	OnInitialUpdate()
	{
		HRESULT hr = S_OK;
		DWORD dwCookie = 0;

		//Create the WebBrowser Control
		m_spOleObject.Release();
		TESTC(hr = m_spOleObject.CoCreateInstance(CLSID_WebBrowser));

		//Obtain the WebBrowser interface
		m_spWebBrowser2.Release();
		TESTC(hr = m_spOleObject->QueryInterface(&m_spWebBrowser2));

		//Advise our liseners
		TESTC(hr = AtlAdvise(m_spWebBrowser2, (IDispatch*)this, DIID_DWebBrowserEvents2, &dwCookie));

		//Set the client to be this window
		TESTC(hr = m_spOleObject->SetClientSite(this));

	CLEANUP:
		return SUCCEEDED(hr);
	}

	virtual	HRESULT	Show(LONG iVerb =  OLEIVERB_INPLACEACTIVATE)
	{
		HRESULT hr = S_OK;
		MSG msg;
		RECT rect;
		GetClientRect(m_hWnd, &rect);

		//OLEIVERB_HIDE
		TESTC(hr = m_spOleObject->DoVerb(iVerb, &msg, this, 0, m_hWnd, &rect));

	CLEANUP:
		return hr;
	}

	virtual	HRESULT	Navigate(LPWSTR pwszURL)
	{
		HRESULT hr = S_OK;
		
		//Navigate
		TESTC(hr = m_spWebBrowser2->Navigate(pwszURL, NULL, NULL, NULL, NULL));
		
		//Show
		TESTC(hr = Show());

	CLEANUP:
		return hr;
	}

	virtual	HRESULT	Load(IStream* pIStream)
	{
		HRESULT hr = S_OK;

		//We need a defualt page
		TESTC(hr = Navigate(L"about:blank"));
 
		//Save the stream for later use, since we have to wait for DocumentComplete
		//Before we can finish loading the stream.  Se OnDocumentComplete for the remainder of 
		//this function...
		m_spLoadStream = pIStream;

	CLEANUP:
		return hr;
	}

	//
	// IDispatch Methods
	//
	STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
								   DISPPARAMS* pDispParams, VARIANT* pvarResult,
								   EXCEPINFO*  pExcepInfo,  UINT* puArgErr)
	{
	   switch (dispidMember)
	   {
		  case DISPID_DOWNLOADBEGIN:
			 OnDownloadBegin();
			 break;

		  case DISPID_DOWNLOADCOMPLETE:
			 OnDownloadComplete();
			 break;

		  case DISPID_DOCUMENTCOMPLETE:
			 OnDocumentComplete();
			 break;

		  case DISPID_NAVIGATECOMPLETE:
			 OnNavigateComplete();
			 break;

		case DISPID_AMBIENT_DLCONTROL:
			OnDownloadControl();
			break;
	
		default:
			 return DISP_E_MEMBERNOTFOUND;
	   }

	   return S_OK;
	}

	//Dispatched Messages
	virtual BOOL	OnDownloadBegin()
	{
		return FALSE;
	}
	virtual BOOL	OnDownloadComplete()
	{
		return FALSE;
	}
	virtual BOOL	OnDocumentComplete()
	{
		HRESULT hr = S_OK;

		//Default Implementation
		if(m_spLoadStream)
		{
			CComPtr<IDispatch>				spDocument;
			CComPtr<IPersistStreamInit>		spPersistStream;


			CComPtr<IStream>				spStream;
    		CHAR szHTMLText[] = "<html><h1>Stream Test</h1><p>This HTML content is being loaded from a stream.</html>";


			HGLOBAL hHTMLText = GlobalAlloc(GPTR, lstrlen(szHTMLText)+1);
			TESTC(hr = CreateStreamOnHGlobal(hHTMLText, TRUE, &spStream));
    
   			//Obtain the document
			TESTC(hr = m_spWebBrowser2->get_Document(&spDocument));
			TESTC(hr = spDocument->QueryInterface(&spPersistStream));
    		
			//Load the passed in stream
			TESTC(hr = spPersistStream->InitNew());
			TESTC(hr = spPersistStream->Load(/*m_spLoadStream*/spStream));
//			TESTC(hr = m_spWebBrowser2->Refresh());

			//We are now done with the stream
			m_spLoadStream.Release();
		}
		
	CLEANUP:
		return SUCCEEDED(hr);
	}
	virtual BOOL	OnNavigateComplete()
	{
		return FALSE;
	}
	virtual BOOL	OnDownloadControl()
	{
		return FALSE;
	}

//protected:
	//Data
   CComPtr<IOleObject>		m_spOleObject;
   CComPtr<IWebBrowser2>	m_spWebBrowser2;
   CComPtr<IStream>			m_spLoadStream;
};



/////////////////////////////////////////////////////////////////////
// _NoAddRefReleaseQIOnSmartPtr
//
// Make sure all calls go through the object, not overloaded operators.
// This serves two purposes, firs tit prevent users from using spComPtr->Release(),
// otherwise they have release the internal pointer, but our object still has a reference.
// Second it ensures all calls (AddRef, Release, QI), actually go through the class
// so we can correctly trace all (IUnknown) calls...
/////////////////////////////////////////////////////////////////////
template <class T>
class _NoAddRefReleaseQIOnSmartPtr : public T
{
	private:
		STDMETHOD_(ULONG,	AddRef)()=0;
		STDMETHOD_(ULONG,	Release)()=0;
		STDMETHOD_(HRESULT, QI)(REFIID, void**)=0;
};


/////////////////////////////////////////////////////////////////////
// CSmartPtr
//
// CComPtr
/////////////////////////////////////////////////////////////////////
template <class T> 
class CSmartPtr
{
public:
	//Constructors
	CSmartPtr()
	{
		m_p = NULL;
	}
	~CSmartPtr()
	{
		Release();
	}
	
	//Inlines
	inline	REFIID	iid()	{ return __uuidof(T);	}
	
	//Operators
	_NoAddRefReleaseQIOnSmartPtr<T>* operator->() const
	{
		ASSERT(m_p);
		return (_NoAddRefReleaseQIOnSmartPtr<T>*)m_p;
	}

	//The assert on operator& usually indicates a bug.  If this is really
	//what is needed, however, take the address of the p member explicitly.
	T** operator&()
	{
		ASSERT(!m_p);
		return &m_p;
	}

	operator T*() const
	{
		return (T*)m_p;
	}

	bool operator!() const
	{
		return (m_p == NULL);
	}

	bool operator==(T* pT) const
	{
		return m_p == pT;
	}

	//IUnknown
	ULONG	AddRef()
	{
		ASSERT(m_p);
		return m_p->AddRef();
	}

	ULONG	Release()
	{
		ULONG ulRefCount = 0;

		if(m_p)
		{
			ulRefCount = m_p->Release();
			m_p = NULL;
		}

		return ulRefCount;
	}

	template <class Q> HRESULT QueryInterface(Q** pp) const
	{
		ASSERT(pp && !*pp);
		return m_p->QueryInterface(__uuidof(Q), (void**)pp);
	}

	//Helpers
	HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
	{
		ASSERT(!m_p);
		return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&m_p);
	}

	HRESULT CoCreateInstance(LPCOLESTR szProgID, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
	{
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(szProgID, &clsid);
		
		if(SUCCEEDED(hr))
			hr = CoCreateInstance(clsid, pUnkOuter, dwClsContext);
		return hr;
	}

protected:
	T*	m_p;
};


/////////////////////////////////////////////////////////////////////
// CSmartPtrTrace
//
/////////////////////////////////////////////////////////////////////
template <class T> 
class CSmartPtrTrace	: public CSmartPtr<T>
{
public:
	//Overload
	ULONG	AddRef()
	{
		//Delegate
		ULONG ulRefCount = CSmartPtr<T>::AddRef();

		//Trace the call
		Write(GetInterfaceName(__uuidof(T)));
		WriteLine(L"::AddRef() - ");
		return ulRefCount;
	}

	//Overload
	ULONG	Release()
	{
		//Delegate
		ULONG ulRefCount = CSmartPtr<T>::Release();

		//Trace the call
		Write(GetInterfaceName(__uuidof(T)));
		WriteLine(L"::Release() - ");
		return ulRefCount;
	}

	//Overload
	template <class Q> HRESULT QueryInterface(Q** pp) const
	{
		//Delegate
		HRESULT hr = CSmartPtr<T>::QueryInterface(pp);

		//Trace the call
//		CSmartPtrTrace<T>::Write(GetInterfaceName(__uuidof(T)));
//		CSmartPtrTrace<T>::Write(L"::QueryInterface(");
//		CSmartPtrTrace<T>::Write(GetInterfaceName(__uuidof(Q)));
//		CSmartPtrTrace<T>::WriteLine(L") - ");
		return hr;
	}

	//Helpers
	HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL)
	{
		//Delegate
		HRESULT hr = CSmartPtr<T>::CoCreateInstance(rclsid, pUnkOuter, dwClsContext);
		
		//Trace the call
//		WriteLine(L"CoCreateInstance(..." WIDESTRING(__uuidof(T)) "...");
		return hr;
	}

	inline void	Write(LPCWSTR pwsz)
	{
		TRACE(pwsz);
//		CTrace* pCTrace = GetTrace();
//		if(pCTrace)
//			pCTrace->OutputText(0, 0, pwsz);
	}

	inline void	WriteLine(LPCWSTR pwsz)
	{
		Write(pwsz);
		Write(wWndEOL);
//		CTrace* pCTrace = GetTrace();
//		if(pCTrace)
//		{
//			pCTrace->OutputText(0, 0, pwsz);
//			pCTrace->OutputLineEnd();
//		}
	}
};


///////////////////////////////////////////////////////////////////////////////
// CXmlDOM
// 
///////////////////////////////////////////////////////////////////////////////
class CXmlDOM
{
public:
	//Constructors
	CXmlDOM();
	virtual ~CXmlDOM();

	//Helpers
	virtual HRESULT	Create()
	{
		HRESULT hr = S_OK;

		//Create the DOM
		TESTC(hr = m_spXMLDocument.CoCreateInstance(CLSID_DOMDocument));

		//TODO: Why is the DOM Asynch by default?
		//This makes it so an app has to register to events just to know when its complete...
		//We will make the Document Synchquential unless the user makes it asynch before they call load...
		TESTC(hr = m_spXMLDocument->put_async(VARIANT_FALSE));

	CLEANUP:
		return hr;
	}
	
	virtual HRESULT	LoadStream(IUnknown* pIUnkStream)
	{
		//Delegate
		CComVariant varXML = pIUnkStream;
		return LoadFile(&varXML);
	}

	virtual HRESULT	LoadXML(BSTR bstrXML)
	{
		HRESULT hr = S_OK;
		VARIANT_BOOL bSuccessful = VARIANT_FALSE;

		//Load the document
		TESTC(hr = m_spXMLDocument->loadXML(bstrXML, &bSuccessful));

		if(!bSuccessful)
			TESTC(hr = E_FAIL);

	CLEANUP:
		return hr;
	}

	virtual HRESULT	LoadFile(VARIANT* pVariant)
	{
		ASSERT(pVariant);
		HRESULT hr = S_OK;
		VARIANT_BOOL bSuccessful = VARIANT_FALSE;

		//Load the document
		TESTC(hr = m_spXMLDocument->load(*pVariant, &bSuccessful));

		if(!bSuccessful)
			TESTC(hr = E_FAIL);

	CLEANUP:
		return hr;
	}

	virtual HRESULT	Save(BSTR bstrFileName)
	{
		//Prevent a copy of the BSTR
		CComVariant	varFileName;
		varFileName.vt		= VT_BSTR;
		varFileName.bstrVal	= bstrFileName;

		//Delegate
		HRESULT hr = Save(&varFileName);

		varFileName.bstrVal	= NULL;
		return hr;
	}

	virtual HRESULT	Save(VARIANT* pVariant)
	{
		ASSERT(pVariant);
		HRESULT hr = S_OK;
		
		//Save the XML
		TESTC(hr = m_spXMLDocument->save(*pVariant));

	CLEANUP:
		return hr;
	}

	virtual HRESULT PrintNode(IXMLDOMNode* spNode, CConsole& rConsole, ULONG ullevel = 0);
	virtual HRESULT PrintAttributes(IXMLDOMNode* spNode, CConsole& rConsole);

//protected:
	//Data
	CSmartPtr<IXMLDOMDocument>	m_spXMLDocument;
};



/////////////////////////////////////////////////////////////////////
// CComWSTR
//
/////////////////////////////////////////////////////////////////////
class CComWSTR
{
protected:
	//Data
	WCHAR*	m_str;

private:
	//NOTE: Don't use Copy Constructors or Operators - Very DANGEROUS!
	//We have already have to track down a bug where SAFE_ALLOC(cwstr, WCHAR, 1),
	//corrupted memory later one, since it really means cwstr = "garbage-no-terminator-memory"
	CComWSTR(LPCOLESTR pSrc)				{	ASSERT(FALSE);	}
	CComWSTR(const CComWSTR& src)			{	ASSERT(FALSE);	}
//	CComWSTR& operator=(const CComWSTR& src){	ASSERT(FALSE);	}
//	CComWSTR& operator=(LPCOLESTR pSrc)		{	ASSERT(FALSE);	}

public:
	//Constructors
	CComWSTR()
	{
		m_str = NULL;
	}
	virtual ~CComWSTR()
	{
		Empty();
	}
	void Empty()
	{
		SAFE_FREE(m_str);
	}
	CComWSTR& operator+=(LPCOLESTR pSrc)
	{
		Append(pSrc);
		return *this;
	}
	bool operator!() const
	{
		return (m_str == NULL);
	}
	WCHAR** operator&()
	{
		ASSERT(m_str == NULL);
		return &m_str;
	}

	//Cast operator - so we can use this class in instances of just WCHAR
	operator WCHAR*() const
	{
		return m_str;
	}
	
	//Helpers
	unsigned int Length() const
	{
		return (m_str == NULL) ? 0 : (unsigned int)wcslen(m_str);
	}
	WCHAR* Copy() const
	{
		return wcsDuplicate(m_str);
	}
	HRESULT CopyFrom(LPCOLESTR pSrc)
	{
		Empty();
		m_str = wcsDuplicate(pSrc);
		return S_OK;
	}

	//Non allocating functions - quick and easy attaching to exiting strings
	void Attach(WCHAR* src)
	{
		ASSERT(m_str == NULL);
		m_str = src;
	}
	WCHAR* Detach()
	{
		WCHAR* s = m_str;
		m_str = NULL;
		return s;
	}

	//Appending
	HRESULT Append(LPCOLESTR pSrc)
	{
		return Append(pSrc, pSrc ? (unsigned int)wcslen(pSrc) : 0);
	}
	HRESULT Append(LPCOLESTR pSrc, int nLen)
	{
		HRESULT hr = S_OK;
		
		if(nLen)
		{
			ASSERT(pSrc);
			int n1 = Length();
			
			//Realloc the current string
			SAFE_REALLOC(m_str, WCHAR, n1+nLen+1);
			memcpy(m_str+n1, pSrc, nLen*sizeof(WCHAR));
			m_str[n1+nLen] = NULL;
		}
		
	CLEANUP:
		return hr;
	}
};


/////////////////////////////////////////////////////////////////////
// ListView Helpers
//
/////////////////////////////////////////////////////////////////////
INDEX		LV_InsertColumn(HWND hWnd, INDEX iColumn, WCHAR* wszName, INT iImage = IMAGE_NONE);
INDEX		LV_InsertItem(HWND hWnd, INDEX iItem, INDEX iSubItem, WCHAR* wszName, LPARAM lParam = PARAM_NONE, INT iImage = IMAGE_NONE, UINT iState = IMAGE_NONE, UINT iStateMask = IMAGE_NONE);

BOOL		LV_SetItemState(HWND hWnd, INDEX iItem, INDEX iSubItem, UINT iState, UINT iStateMask);
BOOL		LV_SetItemText(HWND hWnd, INDEX iItem, INDEX iSubItem, WCHAR* pwszName);
BOOL		LV_SetItemImage(HWND hWnd, INDEX iItem, INDEX iSubItem, INT iImage);
BOOL		LV_SetItemParam(HWND hWnd, INDEX iItem, INDEX iSubItem, LPARAM lParam);

INDEX		LV_GetItemText(HWND hWnd, INDEX iItem, INDEX iSubItem, WCHAR* pwszName, ULONG ulMaxSize);
UINT		LV_GetItemState(HWND hWnd, INDEX iItem, UINT iMask = 0);
INT			LV_GetItemImage(HWND hWnd, INDEX iItem, INDEX iSubItem);
LPARAM		LV_GetItemParam(HWND hWnd, INDEX iItem, INDEX iSubItem);

INDEX		LV_FindItem(HWND hWnd, LPARAM lParam, INDEX iStart = -1);
														 
LRESULT		LV_GetSelItems(HWND hWnd, INDEX* pcItems = NULL, INDEX** prgSelItems = NULL, LPARAM** prgSelParams = NULL);
LRESULT		LV_GetAllItems(HWND hWnd, INDEX* pcItems = NULL, INDEX** prgItems = NULL, LPARAM** prgParams = NULL);


/////////////////////////////////////////////////////////////////////
// TreeView Helpers
//
/////////////////////////////////////////////////////////////////////
HTREEITEM	TV_InsertItem(HWND hWnd, HTREEITEM hParent, HTREEITEM hInsAfter, WCHAR* wszName, LPARAM lParam = 0, INT iImage = 0, INT iSelectedImage = 0, UINT iState = 0, UINT iStateMask = 0);

BOOL		TV_GetItemText(HWND hWnd, HTREEITEM hItem, WCHAR* pwszBuffer, LONG ulMaxSize);
LPARAM		TV_GetItemParam(HWND hWnd, HTREEITEM hItem);
BOOL		TV_SetItemState(HWND hWnd, HTREEITEM hItem, UINT iState, UINT iStateMask);

HTREEITEM	TV_FindItem(HWND hWnd, HTREEITEM hParent, WCHAR* wszName);



#endif //__CDIALOGLITE_H__

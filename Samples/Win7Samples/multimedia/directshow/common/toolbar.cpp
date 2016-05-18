//////////////////////////////////////////////////////////////////////////
// Toolbar.cpp: Toolbar and rebar control classes.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "wincontrol.h"
#include "toolbar.h"

//--------------------------------------------------------------------------------------
// Toolbar constructor.
//--------------------------------------------------------------------------------------

Toolbar::Toolbar()
{
	m_hImageListNormal = NULL;
	m_hImageListHot = NULL;
	m_hImageListDisabled = NULL;
}


Toolbar::Button::Button(int bitmap, int command)
{
    ZeroMemory(this, sizeof(TBBUTTON));
    iBitmap = bitmap;
    idCommand = command;
    fsState = TBSTATE_ENABLED;
    fsStyle = BTNS_BUTTON;
}

//--------------------------------------------------------------------------------------
// Toolbar destructor.
//--------------------------------------------------------------------------------------

Toolbar::~Toolbar()
{
    // Delete the image lists
	if (m_hImageListNormal)
	{
		DeleteObject(m_hImageListNormal);
	}
	if (m_hImageListHot)
	{
		DeleteObject(m_hImageListHot);
	}
	if (m_hImageListDisabled)
	{
		DeleteObject(m_hImageListDisabled);
	}
}

//--------------------------------------------------------------------------------------
// Toolbar::Create
// Description: Creates an instance of the toolbar class.
//--------------------------------------------------------------------------------------

HRESULT Toolbar::Create(HINSTANCE hInstance, HWND hParent, DWORD_PTR id, DWORD dwStyle)
{

    dwStyle |= WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | CCS_NODIVIDER;

    m_hwnd = CreateWindowEx(0, TOOLBARCLASSNAME, (LPTSTR) NULL, 
        dwStyle, 0, 0, 0, 0, hParent, (HMENU)id, hInstance, NULL); 

    if (m_hwnd == 0)
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }


    SendMessage(TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
    return S_OK;
}

//--------------------------------------------------------------------------------------
// Toolbar::AddButton
// Description: Add a button to the toolbar.
//--------------------------------------------------------------------------------------

HRESULT Toolbar::AddButton(const Toolbar::Button& button)
{
    assert(m_hwnd != NULL);

    if (SendMessage(TB_ADDBUTTONS, 1, (LPARAM)&button))
    {
        return S_OK;
    }
    else
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }
}

//--------------------------------------------------------------------------------------
// Toolbar::Check
// Description: Checks or unchecks a button in the toolbar.
//--------------------------------------------------------------------------------------

HRESULT Toolbar::Check(int id, BOOL fCheck)
{
    assert(m_hwnd != NULL);
    if (SendMessage(TB_CHECKBUTTON, id, fCheck))
    {
        return S_OK;
    }
    else
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }
}

//--------------------------------------------------------------------------------------
// Toolbar::Enable
// Description: Enables or disables a button in the toolbar.
//--------------------------------------------------------------------------------------

HRESULT Toolbar::Enable(int id, BOOL fEnable)
{
    assert(m_hwnd != NULL);
    if (SendMessage(TB_ENABLEBUTTON, id, fEnable))
    {
        return S_OK;
    }
    else
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }
}

//--------------------------------------------------------------------------------------
// Toolbar::SetButtonImage
// Description: Sets the bitmap for a button in the toolbar.
//
// bitmap: Image index (in the image list)
//--------------------------------------------------------------------------------------

HRESULT Toolbar::SetButtonImage(int id, int bitmap)
{
    assert(m_hwnd != NULL);

	TBBUTTONINFO tbbi;
	ZeroMemory(&tbbi, sizeof(tbbi));


	tbbi.cbSize = sizeof(tbbi);
	tbbi.dwMask = TBIF_IMAGE;
	tbbi.iImage = bitmap;

	if (SendMessage(TB_SETBUTTONINFO, id, (LPARAM)&tbbi))
	{
		return S_OK;
	}
    else
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }
}


///////////////////////////////////////////////////////////////////////
// Name: ShowToolTip
// Desc: Show a tool tip for a toolbar button.
//
// Call this method when the app receives a TTN_GETDISPINFO notification.
// The method loads a string resource with the same resource ID as the
// button ID. 
//
// hInstance: Handle to the application instance
// pDispInfo: Pointer to the NMTTDISPINFO struct from the WM_NOTFIY
//            message.
// 
///////////////////////////////////////////////////////////////////////

HRESULT Toolbar::ShowToolTip(NMTTDISPINFO *pDispInfo)
{
    const int cchBuffer = 80; // size of the buffer in the NMTTDISPINFO struct.

    // Check if the NMTTDISPINFO structure contains a button ID.
    // (It might contain nothing, or an HWND.)
    if (pDispInfo->hdr.idFrom != 0 && !(pDispInfo->uFlags & TTF_IDISHWND))
    {
        // Tell the tooltip control to load a string with the same resource ID.

        pDispInfo->hinst = GetInstance();
        pDispInfo->lpszText = (LPTSTR)pDispInfo->hdr.idFrom;

		// This flag means "store the string yourself, and stop asking me"
        pDispInfo->uFlags = TTF_DI_SETITEM;
    }

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Toolbar::SetImageList
// Description: Sets an image list for the toolbar.
//
// state: Which button state this image list will apply to.
// nBitmapID: Resource ID of the bitmap that contains the images.
// buttonSize: Size of each button image.
// numButtons: Number of button images
// mask: Color mask
//--------------------------------------------------------------------------------------

HRESULT Toolbar::SetImageList(
	ButtonState state, 
	UINT nBitmapID, 
	const Size& buttonSize, 
	DWORD numButtons, 
	COLORREF mask
	)
{
	HRESULT hr = S_OK;
	HBITMAP hBitmap = NULL;
	HIMAGELIST hImageListTmp;
	int iImageList;

	// Create the image list
	hImageListTmp = ImageList_Create(
		buttonSize.cx, buttonSize.cy, ILC_COLOR24 | ILC_MASK, numButtons, 0);
	if (hImageListTmp == NULL)
	{
		hr = __HRESULT_FROM_WIN32(GetLastError());
	}

	// Load the bitmap for the image list.
	if (SUCCEEDED(hr))
	{
		hBitmap = LoadBitmap(GetInstance(), MAKEINTRESOURCE(nBitmapID));
		if (hBitmap == NULL)
		{
			hr = __HRESULT_FROM_WIN32(GetLastError());
		}
	}


	// Add masked bitmap to image list
	if (SUCCEEDED(hr))
	{
		iImageList = ImageList_AddMasked(hImageListTmp, hBitmap, mask);
		if (iImageList == -1) // Failure code is -1
		{
			hr = __HRESULT_FROM_WIN32(GetLastError());
		}
	}

	// Set the image list on the toolbar and store the image list handle.
	if (SUCCEEDED(hr))
	{
		switch (state)
		{
		case Normal:
			m_hImageListNormal = hImageListTmp;
			SendMessage(TB_SETIMAGELIST, 0, (LPARAM)hImageListTmp);
			break;

		case Hot:
			m_hImageListNormal = hImageListTmp;
			SendMessage(TB_SETHOTIMAGELIST, 0, (LPARAM)hImageListTmp);
			break;

		case Disabled:
			m_hImageListNormal = hImageListTmp;
			SendMessage(TB_SETDISABLEDIMAGELIST, 0, (LPARAM)hImageListTmp);
			break;

		default:
			hr =  E_INVALIDARG;
			DeleteObject(hImageListTmp);
		}
	}

	if (hBitmap)
	{
		DeleteObject(hBitmap);
	}

	return hr;
}


/////////////////


//--------------------------------------------------------------------------------------
// Rebar constructor.
//--------------------------------------------------------------------------------------

Rebar::Rebar()
{

}

//--------------------------------------------------------------------------------------
// Rebar::Create
// Description: Creates an instance of the rebar class.
//--------------------------------------------------------------------------------------

HRESULT Rebar::Create(HINSTANCE hInstance, HWND hParent, DWORD_PTR id, DWORD dwStyle)
{
    dwStyle |= WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | 
        RBS_VARHEIGHT | RBS_AUTOSIZE | RBS_BANDBORDERS | CCS_NODIVIDER; 

    m_hwnd = CreateWindowEx(0, REBARCLASSNAME, (LPTSTR) NULL, 
        dwStyle, 0, 0, 0, 0, hParent, (HMENU)id, hInstance, NULL); 

    if (m_hwnd == 0)
    {
        return __HRESULT_FROM_WIN32(GetLastError());
    }

	REBARINFO rbi;
	rbi.cbSize = sizeof(REBARINFO);
	rbi.fMask  = 0;
	SendMessage(RB_SETBARINFO, 0, (LPARAM)&rbi);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Rebar::AddBand
// Description: Add a new band to the rebar control.
//--------------------------------------------------------------------------------------

HRESULT Rebar::AddBand(HWND hBand, UINT id)
{
    assert(hBand != NULL);
    assert(m_hwnd != NULL);

	BOOL bIsToolbar = FALSE;
	int nBtnCount = 0;

	const DWORD STRING_LEN = 32;
	WCHAR szClassName[STRING_LEN];

	if (0 == GetClassName(hBand, szClassName, STRING_LEN))
	{
		return __HRESULT_FROM_WIN32(GetLastError());
	}

	// Note: per MSDN, string returned by GetClassName is always null terminated (but may be truncated)
	if (wcscmp(szClassName, TOOLBARCLASSNAME) == 0)
	{
		bIsToolbar = TRUE;
	}

	if (bIsToolbar)
	{
		// Get number of buttons on the toolbar
		nBtnCount = (int)::SendMessage(hBand, TB_BUTTONCOUNT, 0, 0);
	}

	// Set band info structure
	REBARBANDINFO rbBand;
    ZeroMemory(&rbBand, sizeof(rbBand));

	rbBand.cbSize = REBARBANDINFO_V6_SIZE;

    rbBand.fMask = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE | RBBIM_IDEALSIZE;
    rbBand.fStyle = RBBS_CHILDEDGE ;
	if (nBtnCount > 0)
    {
        // add chevron style for toolbar with buttons
		rbBand.fStyle |= RBBS_USECHEVRON;
    }
	rbBand.hwndChild = hBand;
	rbBand.wID = id;


	// Calculate the size of the band
	BOOL bRet = FALSE;
    RECT rcTmp = { 0, 0, 0, 0 };

    if (nBtnCount > 0)
	{
		bRet = (BOOL)::SendMessage(hBand, TB_GETITEMRECT, nBtnCount - 1, (LPARAM)&rcTmp);

        rbBand.cx = nBtnCount * (rcTmp.right - rcTmp.left); // length of band
		rbBand.cyMinChild = rcTmp.bottom - rcTmp.top;  // minimum height 
		rbBand.cxMinChild = rbBand.cx;  // minimum width
	}
	else	// no buttons, either not a toolbar or really has no buttons
	{
		bRet = ::GetWindowRect(hBand, &rcTmp);
		rbBand.cx = rcTmp.right - rcTmp.left;
		rbBand.cxMinChild = rbBand.cx;
		rbBand.cyMinChild = rcTmp.bottom - rcTmp.top;
	}
	rbBand.cxIdeal = rbBand.cx;  // ?? Not sure this is right

	// Add the band
	LRESULT lRes = SendMessage(RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbBand);
	if(lRes == 0)
	{
		return E_FAIL;
	}

	DWORD dwExStyle = (DWORD)::SendMessage(hBand, TB_GETEXTENDEDSTYLE, 0, 0L);
	::SendMessage(hBand, TB_SETEXTENDEDSTYLE, 0, dwExStyle | TBSTYLE_EX_HIDECLIPPEDBUTTONS);

	return S_OK;

}


//--------------------------------------------------------------------------------------
// Rebar::ShowBand
// Description: Show or hide a band in the rebar.
//--------------------------------------------------------------------------------------

HRESULT Rebar::ShowBand(UINT id, BOOL bShow)
{
    UINT position;
    HRESULT hr = BandIdToIndex(id, &position);
    if (FAILED(hr))
    {
        return hr;
    }

    BOOL result = (BOOL)SendMessage(RB_SHOWBAND, position, bShow);
    if (result)
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

//--------------------------------------------------------------------------------------
// Rebar::BandIdToIndex
// Description: Convert a band ID into an index.
//--------------------------------------------------------------------------------------

HRESULT Rebar::BandIdToIndex(UINT id, UINT *pIndex)
{
    assert(m_hwnd != NULL);
    assert(pIndex != NULL);

    LRESULT result = SendMessage(RB_IDTOINDEX, id, 0);
    if (result == -1)
    {
        return E_FAIL;
    }
    else
    {
        *pIndex = (UINT)result;
        return S_OK;
    }
}

/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          TSFEdit.cpp

   Description:   CTSFEditWnd Class Implementation

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include "TSFEdit.h"
#include "Globals.h"
#include <ctffunc.h>

/*
None of the GUIDs in TSATTRS.H are defined in a LIB, so you have to include 
INITGUID.H just before the first time you include TSATTRS.H
*/
#include <initguid.h>
#include <tsattrs.h>
#include <tchar.h>

/**************************************************************************
   local function prototypes
**************************************************************************/

/**************************************************************************
   global variables and definitions
**************************************************************************/

TCHAR   g_szTSFEditClassName[] = TEXT("TSFEditWndClass");

#define THIS_POINTER_OFFSET   GWLP_USERDATA

//#define USE_ASSOC_FOCUS

/**************************************************************************

   CTSFEditWnd::CTSFEditWnd()

**************************************************************************/

CTSFEditWnd::CTSFEditWnd(HINSTANCE hInstance, HWND hwndParent)
{
    m_hWnd = NULL;
    m_hwndEdit = NULL;
    m_hwndStatus = NULL;
    m_hInst = hInstance;
    m_ObjRefCount = 1;
    m_hwndParent = hwndParent;
    m_pThreadMgr = NULL;
    m_pDocMgr = NULL;
    m_pPrevDocMgr = NULL;
    m_pContext = NULL;
    m_fLocked = FALSE;
    m_dwLockType = 0;
    m_fPendingLockUpgrade = FALSE;
    m_acpStart = 0;
    m_acpEnd = 0;
    m_fInterimChar = FALSE;
    m_ActiveSelEnd = TS_AE_START;
    m_pServices = NULL;
    m_cCompositions = 0;
    m_pCategoryMgr = NULL;
    m_pDisplayAttrMgr = NULL;
    m_fLayoutChanged = FALSE;
    m_fNotify = TRUE;
    m_cchOldLength = 0;

    ZeroMemory(&m_AdviseSink, sizeof(m_AdviseSink));
    ZeroMemory(&m_rgCompositions, sizeof(m_rgCompositions));
}

/**************************************************************************

   CTSFEditWnd::~CTSFEditWnd()

**************************************************************************/

CTSFEditWnd::~CTSFEditWnd()
{
    /*
    Make sure the advise sink is cleaned up. This should have been done 
    before, but this is just in case.
    */
    _ClearAdviseSink(&m_AdviseSink);

    if(m_pServices)
    {
        m_pServices->Release();
        m_pServices = NULL;
    }
    
    _Uninitialize();
}

/**************************************************************************

   CTSFEditWnd::_Initialize()

**************************************************************************/

BOOL CTSFEditWnd::_Initialize(ITfThreadMgr *ptm, TfClientId tfcId)
{
    HRESULT hr;

    InitCommonControls();
    
    m_tfClientID = tfcId;
    hr = ptm->QueryInterface(IID_ITfThreadMgr, (LPVOID*)&m_pThreadMgr);
    if(FAILED(hr))
    {
        return FALSE;
    }

    hr = CoCreateInstance(  CLSID_TF_CategoryMgr,
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_ITfCategoryMgr, 
                            (LPVOID*)&m_pCategoryMgr);
    if(FAILED(hr))
    {
        return FALSE;
    }
    

    //create the display attribute manager
    hr = CoCreateInstance(  CLSID_TF_DisplayAttributeMgr,
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_ITfDisplayAttributeMgr, 
                            (LPVOID*)&m_pDisplayAttrMgr);
    if(FAILED(hr))
    {
        return FALSE;
    }

    
    //create the document manager
    hr = m_pThreadMgr->CreateDocumentMgr(&m_pDocMgr);
    if(FAILED(hr))
    {
        return FALSE;
    }

    //create the context
    hr = m_pDocMgr->CreateContext(  tfcId, 
                                    0, 
                                    (ITextStoreACP*)this, 
                                    &m_pContext, 
                                    &m_EditCookie);
    if(FAILED(hr))
    {
        return FALSE;
    }

    //push the context onto the document stack
    hr = m_pDocMgr->Push(m_pContext);
    if(FAILED(hr))
    {
        return FALSE;
    }
    
    WNDCLASS  wc;

    //If the class is not already registered, register it.
    if(!GetClassInfo(m_hInst, g_szTSFEditClassName, &wc))
    {
        ZeroMemory(&wc, sizeof(wc));

        wc.style          = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc    = CTSFEditWnd::_WndProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = sizeof(CTSFEditWnd*);
        wc.hInstance      = m_hInst;
        wc.hIcon          = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_MAIN_ICON));
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName   = MAKEINTRESOURCE(IDR_MAIN_MENU);
        wc.lpszClassName  = g_szTSFEditClassName;

        if(0 == RegisterClass(&wc))
        {
            return FALSE;
        }
    }
    //create the main window
    m_hWnd = CreateWindowEx(    0,
                                g_szTSFEditClassName,
                                NULL,
                                WS_OVERLAPPED |
                                    WS_CHILD |
                                    WS_VISIBLE |
                                    WS_TABSTOP,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                m_hwndParent,
                                NULL,
                                m_hInst,
                                (LPVOID)this);

    if(NULL != m_hWnd)
    {
        m_hwndEdit = CreateWindowEx(    WS_EX_CLIENTEDGE,
                                        TEXT("edit"),
                                        NULL,
                                        WS_CLIPSIBLINGS | 
                                            WS_CHILD | 
                                            WS_VISIBLE | 
                                            WS_BORDER | 
                                            WS_VSCROLL | 
                                            ES_AUTOVSCROLL | 
                                            ES_MULTILINE,
                                        0, 0, 0, 0,
                                        m_hWnd,
                                        (HMENU)IDC_EDIT,
                                        m_hInst,
                                        NULL);
        if(NULL == m_hwndEdit)
        {
            return FALSE;
        }

        //create the status bar
        m_hwndStatus = CreateStatusWindow(  WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                                            NULL,
                                            m_hWnd,
                                            IDC_STATUSBAR);

        if(m_hwndStatus)
        {
            _UpdateStatusBar();
        }

#ifdef USE_ASSOC_FOCUS
        /*
        Associate the focus with this window. The TSF Manager watches for 
        focus changes throughout the system. When a window handle that has 
        been associated gets the focus, it then knows the window receiving 
        the focus is TSF enabled.
        */
        hr = m_pThreadMgr->AssociateFocus(m_hwndEdit, m_pDocMgr, &m_pPrevDocMgr);
#endif

        //initialize the supported attributes
        TfGuidAtom guidatom;
        
        //mode bias
        m_rgAttributes[ATTR_INDEX_MODEBIAS].dwFlags = 0;
        m_rgAttributes[ATTR_INDEX_MODEBIAS].attrid = &GUID_PROP_MODEBIAS;
        VariantInit(&(m_rgAttributes[ATTR_INDEX_MODEBIAS].varValue));
        hr = m_pCategoryMgr->RegisterGUID(GUID_MODEBIAS_NONE, &guidatom);
        if(FAILED(hr))
        {
            guidatom = TF_INVALID_GUIDATOM;
        }
        m_rgAttributes[ATTR_INDEX_MODEBIAS].varDefaultValue.vt = VT_I4;
        m_rgAttributes[ATTR_INDEX_MODEBIAS].varDefaultValue.lVal = guidatom;

        //text orientation - this is a VT_I4 that is always zero in this app
        m_rgAttributes[ATTR_INDEX_TEXT_ORIENTATION].dwFlags = 0;
        m_rgAttributes[ATTR_INDEX_TEXT_ORIENTATION].attrid = &TSATTRID_Text_Orientation;
        VariantInit(&m_rgAttributes[ATTR_INDEX_TEXT_ORIENTATION].varValue);
        m_rgAttributes[ATTR_INDEX_TEXT_ORIENTATION].varDefaultValue.vt = VT_I4;
        m_rgAttributes[ATTR_INDEX_TEXT_ORIENTATION].varDefaultValue.lVal = 0;

        //vertical writing - this is a VT_BOOL that is always FALSE in this app
        m_rgAttributes[ATTR_INDEX_TEXT_VERTICALWRITING].dwFlags = 0;
        m_rgAttributes[ATTR_INDEX_TEXT_VERTICALWRITING].attrid = &TSATTRID_Text_VerticalWriting;
        VariantInit(&m_rgAttributes[ATTR_INDEX_TEXT_VERTICALWRITING].varValue);
        m_rgAttributes[ATTR_INDEX_TEXT_VERTICALWRITING].varDefaultValue.vt = VT_BOOL;
        m_rgAttributes[ATTR_INDEX_TEXT_VERTICALWRITING].varDefaultValue.lVal = FALSE;

        _InitFunctionProvider();
        
        UpdateWindow(m_hWnd);

        return TRUE;
    }

    return FALSE;
}

/**************************************************************************

   CTSFEditWnd::_Uninitialize()

**************************************************************************/

void CTSFEditWnd::_Uninitialize()
{
    if(m_pThreadMgr)
    {
        _UninitFunctionProvider();

#ifdef USE_ASSOC_FOCUS
        ITfDocumentMgr *pTempDocMgr = NULL;

        /*
        Its okay if m_pPrevDocMgr is NULL as this will just disassociate the 
        focus from the window.
        */
        m_pThreadMgr->AssociateFocus(m_hwndEdit, m_pPrevDocMgr, &pTempDocMgr);

        if(pTempDocMgr)
        {
            pTempDocMgr->Release();
        }
    
        if(m_pPrevDocMgr)
        {
            m_pPrevDocMgr->Release();
            m_pPrevDocMgr = NULL;
        }
#endif

        int i;

        for (i = 0; i < NUM_SUPPORTED_ATTRS; i++)
        {
            VariantClear(&m_rgAttributes[i].varValue);
            VariantClear(&m_rgAttributes[i].varDefaultValue);
            m_rgAttributes[i].dwFlags = ATTR_FLAG_NONE;
        }

        if(m_pDocMgr)
        {
            //pop all of the contexts off of the stack
            m_pDocMgr->Pop(TF_POPF_ALL);
        
            m_pDocMgr->Release();
            m_pDocMgr = NULL;
        }

        if(m_pDisplayAttrMgr)
        {
            m_pDisplayAttrMgr->Release();
            m_pDisplayAttrMgr = NULL;
        }
        
        if(m_pContext)
        {
            m_pContext->Release();
            m_pContext = NULL;
        }
    
        if(m_pCategoryMgr)
        {
            m_pCategoryMgr->Release();
            m_pCategoryMgr = NULL;
        }

        m_pThreadMgr->Release();
        m_pThreadMgr = NULL;
    }
}

/**************************************************************************

   CTSFEditWnd::_GetWindow()
   
**************************************************************************/

HWND CTSFEditWnd::_GetWindow()
{
    return m_hWnd;
}

/**************************************************************************

   CTSFEditWnd::_OnGetPreservedKey()
   
**************************************************************************/

HRESULT CTSFEditWnd::_OnGetPreservedKey()
{
    HRESULT hr;
    ITfKeystrokeMgr *pKeyMgr;
    
    hr = m_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (LPVOID*)&pKeyMgr);
    if(SUCCEEDED(hr))
    {
        GUID            guid;
        TF_PRESERVEDKEY tfPreKey;

        tfPreKey.uVKey = 'F';
        tfPreKey.uModifiers = TF_MOD_CONTROL;
        
        hr = pKeyMgr->GetPreservedKey(m_pContext, &tfPreKey, &guid);

        if(SUCCEEDED(hr) && !IsEqualGUID(guid, GUID_NULL))
        {
            BOOL fPreserved;

            hr = pKeyMgr->IsPreservedKey(guid, &tfPreKey, &fPreserved);

        
            BOOL    fEaten;

            guid.Data1 = 12;
            hr = pKeyMgr->SimulatePreservedKey(m_pContext, guid, &fEaten);
        }
        
        pKeyMgr->Release();
    }

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::_WndProc()
   
**************************************************************************/

LRESULT CALLBACK CTSFEditWnd::_WndProc( HWND hWnd, 
                                        UINT uMessage, 
                                        WPARAM wParam, 
                                        LPARAM lParam)
{
    CTSFEditWnd *pThis = (CTSFEditWnd*)GetWindowLongPtr(hWnd, THIS_POINTER_OFFSET);

    if((NULL == pThis) && (uMessage != WM_NCCREATE))
    {
        return DefWindowProc(hWnd, uMessage, wParam, lParam);
    }
    
    switch (uMessage)
    {
    case WM_NCCREATE:
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
            pThis = (CTSFEditWnd*)(lpcs->lpCreateParams);
            SetWindowLongPtr(hWnd, THIS_POINTER_OFFSET, (LONG_PTR)pThis);

            //set the window handle
            pThis->m_hWnd = hWnd;

            /*
            AddRef() the object. Release() will be called in WM_NCDESTROY. 
            Many owners will call Release during their WM_DESTROY, but the 
            child window isn't destroyed until after the parent, so the object 
            gets deleted while the window still exists. Calling Release() 
            ourselves in WM_NCDESTROY ensures the object exists for the entire 
            life of the window.
            */
            pThis->AddRef();
        }
        break;

    case WM_CREATE:
        return pThis->_OnCreate();

    case WM_SIZE:
        return pThis->_OnSize(wParam, lParam);

    case WM_DESTROY:
        return pThis->_OnDestroy();

    case WM_SETFOCUS:
        return pThis->_OnSetFocus();

    case WM_KILLFOCUS:
        return pThis->_OnKillFocus();

    case WM_COMMAND:
        return pThis->_OnCommand(   GET_WM_COMMAND_ID(wParam, lParam), 
                                    GET_WM_COMMAND_CMD(wParam, lParam), 
                                    GET_WM_COMMAND_HWND(wParam, lParam));

    case WM_NCDESTROY:
        pThis->Release();
        
        pThis->m_hWnd = NULL;
        break;

    }

    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

/**************************************************************************

   CTSFEditWnd::_OnCreate()

**************************************************************************/

LRESULT CTSFEditWnd::_OnCreate(VOID)
{
    return 0;
}

/**************************************************************************

   CTSFEditWnd::_OnDestroy()

**************************************************************************/

LRESULT CTSFEditWnd::_OnDestroy(VOID)
{
    _Uninitialize();
    
    PostQuitMessage(0);
    
    return 0;
}

/**************************************************************************

   CTSFEditWnd::_OnCommand()

**************************************************************************/

LRESULT CTSFEditWnd::_OnCommand(WORD wID, WORD wCmd, HWND hWnd)
{
    switch(wID)
    {
    case IDC_EDIT:
        switch(wCmd)
        {
        case EN_SETFOCUS:
            _OnEditSetFocus();
            break;

        case EN_KILLFOCUS:
            _OnEditKillFocus();
            break;
        
        case EN_CHANGE:
            _OnEditChange();
            break;

        }
        break;
    }

    return 0;
}

/**************************************************************************

   CTSFEditWnd::_OnSetFocus()

**************************************************************************/

LRESULT CTSFEditWnd::_OnSetFocus(VOID)
{
    OutputDebugString(TEXT("CTSFEditWnd::_OnSetFocus\n"));
    
    SetFocus(m_hwndEdit);

    return 0;
}

/**************************************************************************

   CTSFEditWnd::_OnEditSetFocus()

**************************************************************************/

LRESULT CTSFEditWnd::_OnEditSetFocus(VOID)
{
    OutputDebugString(TEXT("CTSFEditWnd::_OnEditSetFocus\n"));
    
#ifndef USE_ASSOC_FOCUS
    m_pThreadMgr->SetFocus(m_pDocMgr);
#endif

    return 0;
}

/**************************************************************************

   CTSFEditWnd::_OnEditChange()

**************************************************************************/

LRESULT CTSFEditWnd::_OnEditChange(void)
{
    if(m_fNotify && m_AdviseSink.pTextStoreACPSink && (m_AdviseSink.dwMask & TS_AS_TEXT_CHANGE))
    {
        DWORD           dwFlags;
        TS_TEXTCHANGE   tc;
        ULONG           cch;

        cch = GetWindowTextLength(m_hwndEdit);

        /*
        dwFlags can be 0 or TS_TC_CORRECTION
        */
        dwFlags = 0;

        tc.acpStart = 0;
        tc.acpOldEnd = m_cchOldLength;
        tc.acpNewEnd = cch;

        m_AdviseSink.pTextStoreACPSink->OnTextChange(dwFlags, &tc);

        m_cchOldLength = cch;
    }
    
    return 0;
}

/**************************************************************************

   CTSFEditWnd::_OnKillFocus()

**************************************************************************/

LRESULT CTSFEditWnd::_OnKillFocus(VOID)
{
    OutputDebugString(TEXT("CTSFEditWnd::_OnKillFocus\n"));
    
    return 0;
}

/**************************************************************************

   CTSFEditWnd::_OnEditKillFocus()

**************************************************************************/

LRESULT CTSFEditWnd::_OnEditKillFocus(VOID)
{
    OutputDebugString(TEXT("CTSFEditWnd::_OnEditKillFocus\n"));
    
    return 0;
}

/**************************************************************************

   CTSFEditWnd::_OnNotify()

**************************************************************************/

LRESULT CTSFEditWnd::_OnNotify(UINT, LPNMHDR)
{
    return 0;
}

/**************************************************************************

   CTSFEditWnd::_OnSize()

**************************************************************************/

LRESULT CTSFEditWnd::_OnSize(WPARAM wParam, LPARAM lParam)
{
    //adjust the size and location of the status bar
    SendMessage(m_hwndStatus, WM_SIZE, wParam, lParam);

    RECT    rc;

    GetWindowRect(m_hwndStatus, &rc);
    
    MoveWindow( m_hwndEdit, 
                0, 
                0, 
                LOWORD(lParam), 
                HIWORD(lParam) - (rc.bottom - rc.top), 
                TRUE);

    m_AdviseSink.pTextStoreACPSink->OnLayoutChange(TS_LC_CHANGE, EDIT_VIEW_COOKIE);

    return 0;
}

/**************************************************************************

   CTSFEditWnd::_IsLocked()

**************************************************************************/

BOOL CTSFEditWnd::_IsLocked(DWORD dwLockType) 
{ 
    if(m_dwInternalLockType)
    {
        return TRUE;
    }

    return m_fLocked && (m_dwLockType & dwLockType); 
}

/**************************************************************************

   CTSFEditWnd::_ClearAdviseSink()

**************************************************************************/

HRESULT CTSFEditWnd::_ClearAdviseSink(PADVISE_SINK pAdviseSink)
{
    if(pAdviseSink->punkID)
    {
        pAdviseSink->punkID->Release();
        pAdviseSink->punkID = NULL;
    }

    if(pAdviseSink->pTextStoreACPSink)
    {
        pAdviseSink->pTextStoreACPSink->Release();
        pAdviseSink->pTextStoreACPSink = NULL;
    }

    pAdviseSink->dwMask = 0;

    return S_OK;
}

/**************************************************************************

   CTSFEditWnd::_LockDocument()

**************************************************************************/

BOOL CTSFEditWnd::_LockDocument(DWORD dwLockFlags)
{
    if(m_fLocked)
    {
        return FALSE;
    }
    
    m_fLocked = TRUE;
    m_dwLockType = dwLockFlags;
    
    return TRUE;
}

/**************************************************************************

    CTSFEditWnd::_InternalLockDocument()

**************************************************************************/

BOOL CTSFEditWnd::_InternalLockDocument(DWORD dwLockFlags)
{
    m_dwInternalLockType = dwLockFlags;
    
    return TRUE;
}

/**************************************************************************

   CTSFEditWnd::_UnlockDocument

**************************************************************************/

void CTSFEditWnd::_UnlockDocument()
{
    HRESULT hr;
    
    m_fLocked = FALSE;
    m_dwLockType = 0;
    
    //if there is a pending lock upgrade, grant it
    if(m_fPendingLockUpgrade)
    {
        m_fPendingLockUpgrade = FALSE;

        RequestLock(TS_LF_READWRITE, &hr);
    }

    //if any layout changes occurred during the lock, notify the manager
    if(m_fLayoutChanged)
    {
        m_fLayoutChanged = FALSE;
        m_AdviseSink.pTextStoreACPSink->OnLayoutChange(TS_LC_CHANGE, EDIT_VIEW_COOKIE);
    }
}

/**************************************************************************

    CTSFEditWnd::_InternalUnlockDocument()

**************************************************************************/

void CTSFEditWnd::_InternalUnlockDocument()
{
    m_dwInternalLockType = 0;
}

/**************************************************************************

   CTSFEditWnd::_GetCurrentSelection()

**************************************************************************/

BOOL CTSFEditWnd::_GetCurrentSelection(void)
{
    //get the selection from the edit control
    ::SendMessage(m_hwndEdit, EM_GETSEL, (WPARAM)&m_acpStart, (LPARAM)&m_acpEnd);

    return TRUE;
}

/**************************************************************************

    CTSFEditWnd::_OnUpdate()

**************************************************************************/

void CTSFEditWnd::_OnUpdate(void)
{
    //something changed, but it is not known what specifically changed
    //just update the status bar
    _UpdateStatusBar();
}

/**************************************************************************

   CTSFEditWnd::_UpdateStatusBar()

**************************************************************************/

void CTSFEditWnd::_UpdateStatusBar(void)
{
    int     nParts[2];
    HDC     hdc;
    HFONT   hFont;
    SIZE    size;
    TCHAR   szComposition[MAX_PATH];

    hdc = GetDC(m_hwndStatus);
    hFont = (HFONT)SendMessage(m_hwndStatus, WM_GETFONT, 0, 0);
    hFont = (HFONT)SelectObject(hdc, hFont);

    if(m_cCompositions)
    {
        _tcscpy_s(szComposition, ARRAYSIZE(szComposition), TEXT("In Composition"));
    }
    else
    {
        _tcscpy_s(szComposition, ARRAYSIZE(szComposition), TEXT("No Composition"));
    }

    GetTextExtentPoint32(hdc, szComposition, lstrlen(szComposition), &size);
    nParts[0] = size.cx + (GetSystemMetrics(SM_CXEDGE) * 4);

    nParts[1] = -1;
    
    SendMessage(m_hwndStatus, SB_SIMPLE, FALSE, 0);
    SendMessage(m_hwndStatus, SB_SETPARTS, ARRAYSIZE(nParts), (LPARAM)nParts);

    SendMessage(m_hwndStatus, SB_SETTEXT, 0, (LPARAM)szComposition);
    SendMessage(m_hwndStatus, SB_SETTEXT, 1, (LPARAM)TEXT(""));

    //reset the DC
    SelectObject(hdc, hFont);
    ReleaseDC(m_hwndStatus, hdc);

    //enable/disable the menu items as necessary
    HMENU hMenu = GetMenu(m_hwndParent);
    if(NULL != hMenu)
    {
    }

}

/**************************************************************************

    CTSFEditWnd::_OnInitMenuPopup()

**************************************************************************/

LRESULT CTSFEditWnd::_OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
    HMENU hMenu = (HMENU)wParam;
    
    if(NULL != hMenu)
    {
        EnableMenuItem(hMenu, IDM_TERMINATE_COMPOSITION, m_cCompositions ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_RECONVERT, _CanReconvertSelection() ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_PLAYBACK, _CanPlaybackSelection() ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_LOAD, !_IsLocked(TS_LF_READ) ? MF_ENABLED : MF_DISABLED | MF_GRAYED);
    }

    return 0;
}

/**************************************************************************

   CTSFEditWnd::_ClearRequestedAttributes()

**************************************************************************/

void CTSFEditWnd::_ClearRequestedAttributes(void)
{
    int i;

    for (i = 0; i < NUM_SUPPORTED_ATTRS; i++)
    {
        VariantClear(&m_rgAttributes[i].varValue);
        m_rgAttributes[i].dwFlags = ATTR_FLAG_NONE;
    }
}

/**************************************************************************

    CTSFEditWnd::_ClearText()

**************************************************************************/

void CTSFEditWnd::_ClearText(void)
{
    //can't do this if someone has a lock
    if(_IsLocked(TS_LF_READ))
    {
        return;
    }

    _LockDocument(TS_LF_READWRITE);
    
    //empty the text in the edit control, but don't send a change notification
    BOOL    fOldNotify = m_fNotify;
    m_fNotify = FALSE;
    SetWindowTextW(m_hwndEdit, NULL);
    m_fNotify = fOldNotify;

    //update current selection
    m_acpStart = m_acpEnd = 0;
    
    //notify TSF about the changes
    m_AdviseSink.pTextStoreACPSink->OnSelectionChange();
    
    _OnEditChange();
    
    _UnlockDocument();
    
    // make sure to send the OnLayoutChange notification AFTER releasing the lock
    // so clients can do something useful during the notification
    m_AdviseSink.pTextStoreACPSink->OnLayoutChange(TS_LC_CHANGE, EDIT_VIEW_COOKIE);
}

/**************************************************************************

    CTSFEditWnd::_GetText()

**************************************************************************/

HRESULT CTSFEditWnd::_GetText(LPWSTR *ppwsz, LPLONG pcch)
{
    DWORD   cch;
    LPWSTR  pwszText;
    
    *ppwsz = NULL;
    
    cch = GetWindowTextLength(m_hwndEdit);
    pwszText = (LPWSTR)GlobalAlloc(GMEM_FIXED, (cch + 1) * sizeof(WCHAR));
    if(NULL == pwszText)
    {
        return E_OUTOFMEMORY;
    }
    
    GetWindowTextW(m_hwndEdit, pwszText, cch + 1);

    *ppwsz = pwszText;
    
    if(pcch)
    {
        *pcch = cch;
    }
    
    return S_OK;
}

/**************************************************************************

    CTSFEditWnd::_GetDisplayAttributes()

**************************************************************************/

void CTSFEditWnd::_GetDisplayAttributes(void)
{
    HRESULT             hr;
    const GUID          *rGuidProperties[1];
    ITfReadOnlyProperty *pTrackProperty;

    //get the tracking property for the attributes
    rGuidProperties[0] = &GUID_PROP_ATTRIBUTE;
    hr = m_pContext->TrackProperties(   rGuidProperties,
                                        1,
                                        NULL,
                                        0,
                                        &pTrackProperty);
    if(SUCCEEDED(hr))
    {
        ITfRangeACP *pRangeAllText;
        LONG        acpEnd;

        //get the range of the entire text
        acpEnd = GetWindowTextLength(m_hwndEdit);
        hr = m_pServices->CreateRange(0, acpEnd, &pRangeAllText);
        if(SUCCEEDED(hr))
        {
            IEnumTfRanges   *pEnumRanges;

            hr = pTrackProperty->EnumRanges(m_EditCookie, &pEnumRanges, pRangeAllText);
            if(SUCCEEDED(hr))
            {
                ITfRange    *pPropRange;
                ULONG       uFetched;

                /*
                Each range in pEnumRanges represents a span of text that has 
                the same properties specified in TrackProperties.
                */
                while((hr = pEnumRanges->Next(1, &pPropRange, &uFetched)) == S_OK && uFetched) 
                {
                    //get the attribute property for the property range
                    VARIANT var;

                    VariantInit(&var);

                    hr = pTrackProperty->GetValue(m_EditCookie, pPropRange, &var);
                    if(SUCCEEDED(hr))
                    {
                        /*
                        The property is actually a VT_UNKNOWN that contains an 
                        IEnumTfPropertyValue object.
                        */
                        IEnumTfPropertyValue    *pEnumPropertyVal;

                        hr = var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, (LPVOID*)&pEnumPropertyVal);
                        if(SUCCEEDED(hr))
                        {
                            TF_PROPERTYVAL tfPropVal;

                            while((hr = pEnumPropertyVal->Next(1, &tfPropVal, &uFetched)) == S_OK && uFetched) 
                            {
                                if(VT_EMPTY == tfPropVal.varValue.vt)
                                {
                                    //the property for this range has no value
                                    continue;
                                }
                                else if(VT_I4 == tfPropVal.varValue.vt)
                                {
                                    //the property is a guidatom
                                    TfGuidAtom  gaVal;
                                    GUID        guid;

                                    gaVal = (TfGuidAtom)tfPropVal.varValue.lVal;

                                    hr = m_pCategoryMgr->GetGUID(gaVal, &guid);
                                    if(SUCCEEDED(hr))
                                    {
                                        ITfDisplayAttributeInfo *pDispInfo;

                                        hr = m_pDisplayAttrMgr->GetDisplayAttributeInfo(guid, &pDispInfo, NULL);
                                        if(SUCCEEDED(hr))
                                        {
                                            TF_DISPLAYATTRIBUTE da;
                                            
                                            hr = pDispInfo->GetAttributeInfo(&da);

                                            {
                                                GUID    guidDispInfo;
                                                hr = pDispInfo->GetGUID(&guidDispInfo);
                                                guidDispInfo.Data1 = 0;
                                            }

                                            {
                                                BSTR    bstr;

                                                hr = pDispInfo->GetDescription(&bstr);
                                                if(SUCCEEDED(hr))
                                                {
                                                    SysFreeString(bstr);
                                                }
                                            }
                                            
                                            pDispInfo->Release();
                                        }
                                    }
                                }
                                else
                                {
                                    //the property is not recognized
                                }
                            }
                            
                            pEnumPropertyVal->Release();
                        }
                        
                        VariantClear(&var);
                    }
                    
                    pPropRange->Release();
                }
                
                pEnumRanges->Release();
            }

            pRangeAllText->Release();
        }
        
        pTrackProperty->Release();
    }
}

/**************************************************************************

    CTSFEditWnd::_GetTextOwner()

**************************************************************************/

void CTSFEditWnd::_GetTextOwner(void)
{
    HRESULT             hr;
    const GUID          *rGuidProperties[1];
    ITfReadOnlyProperty *pTrackProperty;

    //get the tracking property for the attributes
    rGuidProperties[0] = &GUID_PROP_TEXTOWNER;
    hr = m_pContext->TrackProperties(   rGuidProperties,
                                        1,
                                        NULL,
                                        0,
                                        &pTrackProperty);
    if(SUCCEEDED(hr))
    {
        ITfRangeACP *pRangeAllText;
        LONG        acpEnd;

        //get the range of the entire text
        acpEnd = GetWindowTextLength(m_hwndEdit);
        hr = m_pServices->CreateRange(0, acpEnd, &pRangeAllText);
        if(SUCCEEDED(hr))
        {
            IEnumTfRanges   *pEnumRanges;

            hr = pTrackProperty->EnumRanges(m_EditCookie, &pEnumRanges, pRangeAllText);
            if(SUCCEEDED(hr))
            {
                ITfRange    *pPropRange;
                ULONG       uFetched;

                /*
                Each range in pEnumRanges represents a span of text that has 
                the same properties specified in TrackProperties.
                */
                while((hr = pEnumRanges->Next(1, &pPropRange, &uFetched)) == S_OK && uFetched) 
                {
                    //get the attribute property for the property range
                    VARIANT var;

                    VariantInit(&var);

                    hr = pTrackProperty->GetValue(m_EditCookie, pPropRange, &var);
                    if(SUCCEEDED(hr))
                    {
                        /*
                        The property is actually a VT_UNKNOWN that contains an 
                        IEnumTfPropertyValue object.
                        */
                        IEnumTfPropertyValue    *pEnumPropertyVal;

                        hr = var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, (LPVOID*)&pEnumPropertyVal);
                        if(SUCCEEDED(hr))
                        {
                            TF_PROPERTYVAL tfPropVal;

                            while((hr = pEnumPropertyVal->Next(1, &tfPropVal, &uFetched)) == S_OK && uFetched) 
                            {
                                /*
                                The GUID_PROP_TEXTOWNER attribute value is the CLSID of the text service that owns the text. If the text is not owned, the value is VT_EMPTY.
                                */
                                if(VT_EMPTY == tfPropVal.varValue.vt)
                                {
                                    //the text is not owned
                                    continue;
                                }
                                else if(VT_I4 == tfPropVal.varValue.vt)
                                {
                                    //the property is a guidatom that represents the CLSID of the text service that owns the text.
                                    TfGuidAtom  gaVal;
                                    CLSID       clsidOwner;

                                    gaVal = (TfGuidAtom)tfPropVal.varValue.lVal;

                                    hr = m_pCategoryMgr->GetGUID(gaVal, &clsidOwner);
                                    if(SUCCEEDED(hr))
                                    {
                                    }
                                }
                                else
                                {
                                    //the property is not recognized
                                }
                            }
                            
                            pEnumPropertyVal->Release();
                        }
                        
                        VariantClear(&var);
                    }
                    
                    pPropRange->Release();
                }
                
                pEnumRanges->Release();
            }

            pRangeAllText->Release();
        }
        
        pTrackProperty->Release();
    }
}

/**************************************************************************

    CTSFEditWnd::_GetReadingText()

**************************************************************************/

void CTSFEditWnd::_GetReadingText(void)
{
    HRESULT             hr;
    const GUID          *rGuidProperties[1];
    ITfReadOnlyProperty *pTrackProperty;

    //get the tracking property for the attributes
    rGuidProperties[0] = &GUID_PROP_READING;
    hr = m_pContext->TrackProperties(   rGuidProperties,
                                        1,
                                        NULL,
                                        0,
                                        &pTrackProperty);
    if(SUCCEEDED(hr))
    {
        ITfRangeACP *pRangeAllText;
        LONG        acpEnd;

        //get the range of the entire text
        acpEnd = GetWindowTextLength(m_hwndEdit);
        hr = m_pServices->CreateRange(0, acpEnd, &pRangeAllText);
        if(SUCCEEDED(hr))
        {
            IEnumTfRanges   *pEnumRanges;

            hr = pTrackProperty->EnumRanges(m_EditCookie, &pEnumRanges, pRangeAllText);
            if(SUCCEEDED(hr))
            {
                ITfRange    *pPropRange;
                ULONG       uFetched;

                /*
                Each range in pEnumRanges represents a span of text that has 
                the same properties specified in TrackProperties.
                */
                while((hr = pEnumRanges->Next(1, &pPropRange, &uFetched)) == S_OK && uFetched) 
                {
                    //get the attribute property for the property range
                    VARIANT var;

                    VariantInit(&var);

                    hr = pTrackProperty->GetValue(m_EditCookie, pPropRange, &var);
                    if(SUCCEEDED(hr))
                    {
                        /*
                        The property is actually a VT_UNKNOWN that contains an 
                        IEnumTfPropertyValue object.
                        */
                        IEnumTfPropertyValue    *pEnumPropertyVal;

                        hr = var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, (LPVOID*)&pEnumPropertyVal);
                        if(SUCCEEDED(hr))
                        {
                            TF_PROPERTYVAL tfPropVal;

                            while((hr = pEnumPropertyVal->Next(1, &tfPropVal, &uFetched)) == S_OK && uFetched) 
                            {
                                /*
                                The GUID_PROP_TEXTOWNER attribute value is the CLSID of the text service that owns the text. If the text is not owned, the value is VT_EMPTY.
                                */
                                if(VT_EMPTY == tfPropVal.varValue.vt)
                                {
                                    //the text is not owned
                                    continue;
                                }
                                else if(VT_BSTR == tfPropVal.varValue.vt)
                                {
                                    //the property is a BSTR.
                                    tfPropVal.varValue.bstrVal;

                                    HRESULT hr;
                                    WCHAR   wsz[MAX_PATH];
                                    ULONG   cch;
                                    hr = pPropRange->GetText(m_EditCookie, 0, wsz, MAX_PATH-1, &cch);
                                    wsz[cch] = 0;
                                }
                                else
                                {
                                    //the property is not recognized
                                }
                            }
                            
                            pEnumPropertyVal->Release();
                        }
                        
                        VariantClear(&var);
                    }
                    
                    pPropRange->Release();
                }
                
                pEnumRanges->Release();
            }

            pRangeAllText->Release();
        }
        
        pTrackProperty->Release();
    }
}

/**************************************************************************

    CTSFEditWnd::_GetComposing()

**************************************************************************/

void CTSFEditWnd::_GetComposing(void)
{
    HRESULT             hr;
    const GUID          *rGuidProperties[1];
    ITfReadOnlyProperty *pTrackProperty;

    //get the tracking property for the attributes
    rGuidProperties[0] = &GUID_PROP_COMPOSING;
    hr = m_pContext->TrackProperties(   rGuidProperties,
                                        1,
                                        NULL,
                                        0,
                                        &pTrackProperty);
    if(SUCCEEDED(hr))
    {
        ITfRangeACP *pRangeAllText;
        LONG        acpEnd;

        //get the range of the entire text
        acpEnd = GetWindowTextLength(m_hwndEdit);
        hr = m_pServices->CreateRange(0, acpEnd, &pRangeAllText);
        if(SUCCEEDED(hr))
        {
            IEnumTfRanges   *pEnumRanges;

            hr = pTrackProperty->EnumRanges(m_EditCookie, &pEnumRanges, pRangeAllText);
            if(SUCCEEDED(hr))
            {
                ITfRange    *pPropRange;
                ULONG       uFetched;

                /*
                Each range in pEnumRanges represents a span of text that has 
                the same properties specified in TrackProperties.
                */
                while((hr = pEnumRanges->Next(1, &pPropRange, &uFetched)) == S_OK && uFetched) 
                {
                    //get the attribute property for the property range
                    VARIANT var;

                    VariantInit(&var);

                    hr = pTrackProperty->GetValue(m_EditCookie, pPropRange, &var);
                    if(SUCCEEDED(hr))
                    {
                        /*
                        The property is actually a VT_UNKNOWN that contains an 
                        IEnumTfPropertyValue object.
                        */
                        IEnumTfPropertyValue    *pEnumPropertyVal;

                        hr = var.punkVal->QueryInterface(IID_IEnumTfPropertyValue, (LPVOID*)&pEnumPropertyVal);
                        if(SUCCEEDED(hr))
                        {
                            TF_PROPERTYVAL tfPropVal;

                            while((hr = pEnumPropertyVal->Next(1, &tfPropVal, &uFetched)) == S_OK && uFetched) 
                            {
                                /*
                                The GUID_PROP_COMPOSING attribute value is a VT_I4 that contains a boolean indicating if the text is part of a composition.
                                */
                                BOOL    fComposing = FALSE;

                                if(VT_EMPTY == tfPropVal.varValue.vt)
                                {
                                    //the text is not part of a composition
                                }
                                else if(VT_I4 == tfPropVal.varValue.vt)
                                {
                                    //the property is a VT_I4.
                                    if(tfPropVal.varValue.lVal)
                                    {
                                        //The text is part of a composition
                                        fComposing = TRUE;
                                    }
                                }
                                else
                                {
                                    //the property is not recognized
                                }
                            }
                            
                            pEnumPropertyVal->Release();
                        }
                        
                        VariantClear(&var);
                    }
                    
                    pPropRange->Release();
                }
                
                pEnumRanges->Release();
            }

            pRangeAllText->Release();
        }
        
        pTrackProperty->Release();
    }
}

/**************************************************************************

    CTSFEditWnd::_CanReconvertSelection()

**************************************************************************/

BOOL CTSFEditWnd::_CanReconvertSelection(void)
{
    BOOL                fConv = FALSE;
    HRESULT             hr;
    ITfFunctionProvider *pFuncProv;
    
    _InternalLockDocument(TS_LF_READ);

    hr = m_pThreadMgr->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, &pFuncProv);
    if(SUCCEEDED(hr))
    {
        ITfFnReconversion  *pRecon;
        
        hr = pFuncProv->GetFunction(GUID_NULL, IID_ITfFnReconversion, (IUnknown**)&pRecon);
        if(SUCCEEDED(hr))
        {
            TF_SELECTION    ts;
            ULONG           uFetched;

            hr = m_pContext->GetSelection(m_EditCookie, TF_DEFAULT_SELECTION, 1, &ts, &uFetched);
            if(SUCCEEDED(hr))
            {
                ITfRange    *pRange = NULL;
            
                hr = pRecon->QueryRange(ts.range, &pRange, &fConv);
                if(SUCCEEDED(hr) && pRange)
                {
                    pRange->Release();
                }
            
            ts.range->Release();
            }
            
            pRecon->Release();
        }

        pFuncProv->Release();
    }

    _InternalUnlockDocument();

    return fConv;
}

/**************************************************************************

    CTSFEditWnd::_Reconvert()

**************************************************************************/

void CTSFEditWnd::_Reconvert(void)
{
    HRESULT hr;
    ITfFunctionProvider *pFuncProv;
    
    _InternalLockDocument(TS_LF_READ);

    hr = m_pThreadMgr->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, &pFuncProv);
    if(SUCCEEDED(hr))
    {
        ITfFnReconversion  *pRecon;
        
        hr = pFuncProv->GetFunction(GUID_NULL, IID_ITfFnReconversion, (IUnknown**)&pRecon);
        if(SUCCEEDED(hr))
        {
            TF_SELECTION    ts;
            ULONG           uFetched;

            hr = m_pContext->GetSelection(m_EditCookie, TF_DEFAULT_SELECTION, 1, &ts, &uFetched);
            if(SUCCEEDED(hr))
            {
                ITfRange    *pRange;
                BOOL        fConv;
            
                //get the range that covers the text to be reconverted
                hr = pRecon->QueryRange(ts.range, &pRange, &fConv);
                if(SUCCEEDED(hr) && pRange)
                {
                    {
                        WCHAR   wsz[MAX_PATH];
                        ULONG   cch = 0;
                        pRange->GetText(m_EditCookie, 0, wsz, MAX_PATH-1, &cch);
                        wsz[cch] = 0;
                    }
                    
                    //get the list of reconversion candidates
                    ITfCandidateList    *pCandList;
                    hr = pRecon->GetReconversion(pRange, &pCandList);
                    if(SUCCEEDED(hr))
                    {
                        ULONG   i;
                        ULONG   uCandidateCount = 0;

                        hr = pCandList->GetCandidateNum(&uCandidateCount);

                        for(i = 0; i < uCandidateCount; i++)
                        {
                            ITfCandidateString  *pCandString;

                            hr = pCandList->GetCandidate(i, &pCandString);
                            if(SUCCEEDED(hr))
                            {
                                BSTR    bstr;

                                hr = pCandString->GetString(&bstr);
                                if(SUCCEEDED(hr))
                                {
                                    OutputDebugString(TEXT("\tCandidate - \""));
                                    OutputDebugStringW(bstr);
                                    OutputDebugString(TEXT("\"\n"));
                                    SysFreeString(bstr);
                                }

                                pCandString->Release();
                            }
                        }
                        
                        pCandList->Release();
                    }
                    //cause the reconversion to happen
                    hr = pRecon->Reconvert(pRange);

                }
            }
            
            pRecon->Release();
        }

        pFuncProv->Release();
    }
    
    _InternalUnlockDocument();
}

/**************************************************************************

    CTSFEditWnd::_CanPlaybackSelection()

**************************************************************************/

BOOL CTSFEditWnd::_CanPlaybackSelection(void)
{
    BOOL                fCanPlayback = FALSE;
    HRESULT             hr;
    ITfFunctionProvider *pFuncProv;
    
    _InternalLockDocument(TS_LF_READ);

    hr = m_pThreadMgr->GetFunctionProvider(CLSID_SapiLayr, &pFuncProv);
    if(SUCCEEDED(hr))
    {
        ITfFnPlayBack *pPlayback;
        
        hr = pFuncProv->GetFunction(GUID_NULL, IID_ITfFnPlayBack, (IUnknown**)&pPlayback);
        if(SUCCEEDED(hr))
        {
            TF_SELECTION    ts;
            ULONG           uFetched;

            hr = m_pContext->GetSelection(m_EditCookie, TF_DEFAULT_SELECTION, 1, &ts, &uFetched);
            if(SUCCEEDED(hr))
            {
                ITfRange    *pRange = NULL;
            
                hr = pPlayback->QueryRange(ts.range, &pRange, &fCanPlayback);
                if(SUCCEEDED(hr) && pRange)
                {
                    pRange->Release();
                }
            
            ts.range->Release();
            }
            
            pPlayback->Release();
        }

        pFuncProv->Release();
    }

    _InternalUnlockDocument();

    return fCanPlayback;
}

/**************************************************************************

    CTSFEditWnd::_Playback()

**************************************************************************/

void CTSFEditWnd::_Playback(void)
{
    HRESULT             hr;
    ITfFunctionProvider *pFuncProv;
    
    _InternalLockDocument(TS_LF_READ);

    hr = m_pThreadMgr->GetFunctionProvider(CLSID_SapiLayr, &pFuncProv);
    if(SUCCEEDED(hr))
    {
        ITfFnPlayBack *pPlayback;
        
        hr = pFuncProv->GetFunction(GUID_NULL, IID_ITfFnPlayBack, (IUnknown**)&pPlayback);
        if(SUCCEEDED(hr))
        {
            TF_SELECTION    ts;
            ULONG           uFetched;

            hr = m_pContext->GetSelection(m_EditCookie, TF_DEFAULT_SELECTION, 1, &ts, &uFetched);
            if(SUCCEEDED(hr))
            {
                BOOL        fCanPlayback;
                ITfRange    *pRange = NULL;
            
                hr = pPlayback->QueryRange(ts.range, &pRange, &fCanPlayback);
                if(SUCCEEDED(hr) && pRange)
                {
                    hr = pPlayback->Play(pRange);
                    
                    pRange->Release();
                }
            
            ts.range->Release();
            }
            
            pPlayback->Release();
        }

        pFuncProv->Release();
    }

    _InternalUnlockDocument();
}


///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

/**************************************************************************

   CTSFEditWnd::QueryInterface

**************************************************************************/

STDMETHODIMP CTSFEditWnd::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
    *ppReturn = NULL;

    //IUnknown
    if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITextStoreACP))
    {
        *ppReturn = (ITextStoreACP*)this;
    }

    //ITfContextOwnerCompositionSink
    else if(IsEqualIID(riid, IID_ITfContextOwnerCompositionSink))
    {
        *ppReturn = (ITfContextOwnerCompositionSink*)this;
    }

    //ITfFunctionProvider
    else if(IsEqualIID(riid, IID_ITfFunctionProvider))
    {
        *ppReturn = (ITfFunctionProvider*)this;
    }

    if(*ppReturn)
    {
        (*(LPUNKNOWN*)ppReturn)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}                                             

/**************************************************************************

   CTSFEditWnd::AddRef

**************************************************************************/

STDMETHODIMP_(DWORD) CTSFEditWnd::AddRef()
{
    return ++m_ObjRefCount;
}


/**************************************************************************

   CTSFEditWnd::Release

**************************************************************************/

STDMETHODIMP_(DWORD) CTSFEditWnd::Release()
{
    if(--m_ObjRefCount == 0)
    {
        delete this;
        return 0;
    }
   
    return m_ObjRefCount;
}


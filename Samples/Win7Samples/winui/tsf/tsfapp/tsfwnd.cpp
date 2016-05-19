/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          TSFWnd.cpp

   Description:   CTSFMainWnd Class Implementation

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include "TSFWnd.h"
#include "Globals.h"

/**************************************************************************
   local function prototypes
**************************************************************************/

/**************************************************************************
   global variables and definitions
**************************************************************************/

TCHAR g_szTSFMainClassName[] = TEXT("TSFMainWndClass");
TCHAR g_szTSFAppTitle[] = TEXT("TSF Test Application");

#define THIS_POINTER_OFFSET   GWLP_USERDATA

/**************************************************************************

   CTSFMainWnd::CTSFMainWnd()

**************************************************************************/

CTSFMainWnd::CTSFMainWnd(HINSTANCE hInstance)
{
    m_hWnd = NULL;
    m_hInst = hInstance;
    m_pTSFEditWnd = NULL;
    m_tfClientID = 0;
}

/**************************************************************************

   CTSFMainWnd::~CTSFMainWnd()

**************************************************************************/

CTSFMainWnd::~CTSFMainWnd()
{
    if(m_hWnd)
    {
        DestroyWindow(m_hWnd);
    }
    
    if(g_pThreadMgr)
    {
        g_pThreadMgr->Deactivate();
        
        g_pThreadMgr->Release();
        g_pThreadMgr = NULL;
    }
}

/**************************************************************************

   CTSFMainWnd::Initialize()

**************************************************************************/

BOOL CTSFMainWnd::Initialize(int nCmdShow)
{
    HRESULT hr;
#if 1
    hr = CoCreateInstance(  CLSID_TF_ThreadMgr, 
                            NULL, 
                            CLSCTX_INPROC_SERVER, 
                            IID_ITfThreadMgr, 
                            (void**)&g_pThreadMgr);
#else
    hr = TF_CreateThreadMgr(&g_pThreadMgr);
#endif
    if(SUCCEEDED(hr))
    {
        hr = g_pThreadMgr->Activate(&m_tfClientID);

        if(SUCCEEDED(hr))
        {
            WNDCLASS  wc;

            ZeroMemory(&wc, sizeof(wc));
   
            wc.style          = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc    = CTSFMainWnd::_WndProc;
            wc.cbClsExtra     = 0;
            wc.cbWndExtra     = sizeof(CTSFMainWnd*);
            wc.hInstance      = m_hInst;
            wc.hIcon          = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_MAIN_ICON));
            wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
            wc.hbrBackground  = (HBRUSH)(COLOR_DESKTOP + 1);
            wc.lpszMenuName   = MAKEINTRESOURCE(IDR_MAIN_MENU);
            wc.lpszClassName  = g_szTSFMainClassName;

            if(0 != RegisterClass(&wc))
            {
                //create the main window
                m_hWnd = CreateWindowEx(    0,
                                            g_szTSFMainClassName,
                                            g_szTSFAppTitle,
                                            WS_OVERLAPPEDWINDOW,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            NULL,
                                            NULL,
                                            m_hInst,
                                            (LPVOID)this);

                if(NULL != m_hWnd)
                {
                    ShowWindow(m_hWnd, nCmdShow);
                    UpdateWindow(m_hWnd);
            
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

/**************************************************************************

   CTSFMainWnd::_CleanupEditWnd()

**************************************************************************/

void CTSFMainWnd::_CleanupEditWnd(BOOL fNuke)
{
    if(NULL != m_pTSFEditWnd)
    {
        if(fNuke)
        {
            delete m_pTSFEditWnd;
        }

        else
        {
            m_pTSFEditWnd->Release();
        }
    
        m_pTSFEditWnd = NULL;
    }
}

/**************************************************************************

   CTSFMainWnd::_WndProc()
   
**************************************************************************/

LRESULT CALLBACK CTSFMainWnd::_WndProc( HWND hWnd, 
                                        UINT uMessage, 
                                        WPARAM wParam, 
                                        LPARAM lParam)
{
    CTSFMainWnd *pThis = (CTSFMainWnd*)GetWindowLongPtr(hWnd, THIS_POINTER_OFFSET);

    if((NULL == pThis) && (uMessage != WM_NCCREATE))
    {
        return DefWindowProc(hWnd, uMessage, wParam, lParam);
    }
    
    switch (uMessage)
    {
    case WM_NCCREATE:
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
            pThis = (CTSFMainWnd*)(lpcs->lpCreateParams);
            SetWindowLongPtr(hWnd, THIS_POINTER_OFFSET, (LONG_PTR)pThis);

            //set the window handle
            pThis->m_hWnd = hWnd;
        }
        break;

    case WM_CREATE:
        return pThis->_OnCreate();

    case WM_SIZE:
        return pThis->_OnSize(LOWORD(lParam), HIWORD(lParam));

    case WM_DESTROY:
        return pThis->_OnDestroy();

    case WM_SETFOCUS:
        return pThis->_OnSetFocus();

    case WM_KILLFOCUS:
        return pThis->_OnKillFocus();

    case WM_ACTIVATE:
        return pThis->_OnActivate(wParam);

    case WM_INITMENUPOPUP:
        return pThis->_OnInitMenuPopup(wParam, lParam);

    case WM_COMMAND:
        return pThis->_OnCommand(   GET_WM_COMMAND_ID(wParam, lParam), 
                                    GET_WM_COMMAND_CMD(wParam, lParam), 
                                    GET_WM_COMMAND_HWND(wParam, lParam));

    case WM_NCDESTROY:
        pThis->m_hWnd = NULL;
        break;

    }

    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

/**************************************************************************

   CTSFMainWnd::)OnCreate()

**************************************************************************/

LRESULT CTSFMainWnd::_OnCreate(VOID)
{
    m_pTSFEditWnd = new CTSFEditWnd(m_hInst, m_hWnd);

    if(!m_pTSFEditWnd)
    {
        return -1;
    }

    if(!m_pTSFEditWnd->_Initialize(g_pThreadMgr, m_tfClientID))
    {
        _CleanupEditWnd(TRUE);
        
        return -1;
    }

    return 0;
}

/**************************************************************************

   CTSFMainWnd::_OnDestroy()

**************************************************************************/

LRESULT CTSFMainWnd::_OnDestroy(VOID)
{
    _CleanupEditWnd(FALSE);

    PostQuitMessage(0);
    
    return 0;
}

/**************************************************************************

   CTSFMainWnd::_OnCommand()

**************************************************************************/

LRESULT CTSFMainWnd::_OnCommand(WORD wID, WORD wCmd, HWND hWnd)
{
    switch(wID)
    {
    case IDM_EXIT:
        PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        break;
    
    case IDM_ABOUT:
        MessageBox(m_hWnd, TEXT(""), g_szTSFAppTitle, MB_OK | MB_ICONINFORMATION);
        break;
    
    case IDM_GETPRESERVEDKEY:
        m_pTSFEditWnd->_OnGetPreservedKey();
        break;

    case IDM_GETDISPATTR:
        m_pTSFEditWnd->_GetDisplayAttributes();
        break;

    case IDM_GET_TEXTOWNER:
        m_pTSFEditWnd->_GetTextOwner();
        break;
    
    case IDM_GET_READING:
        m_pTSFEditWnd->_GetReadingText();
        break;

    case IDM_GET_COMPOSING:
        m_pTSFEditWnd->_GetComposing();
        break;
    
    case IDM_TERMINATE_COMPOSITION:
        m_pTSFEditWnd->_TerminateAllCompositions();
        break;
    
    case IDM_RECONVERT:
        m_pTSFEditWnd->_Reconvert();
        break;

    case IDM_PLAYBACK:
        m_pTSFEditWnd->_Playback();
        break;
    
    case IDM_LOAD:
        {
            TCHAR   szFile[MAX_PATH];

            if(_GetFileName(m_hWnd, szFile, MAX_PATH, TRUE))
            {
                m_pTSFEditWnd->_LoadFromFile(szFile);
            }
        }
        break;

    case IDM_SAVE:
        {
            TCHAR   szFile[MAX_PATH];

            if(_GetFileName(m_hWnd, szFile, MAX_PATH, FALSE))
            {
                m_pTSFEditWnd->_SaveToFile(szFile);
            }
        }
        break;
    
    case IDM_TEST:
        m_pTSFEditWnd->_OnTest();
        break;
    }
    
    return 0;
}

/**************************************************************************

   CTSFMainWnd::_OnSetFocus()

**************************************************************************/

LRESULT CTSFMainWnd::_OnSetFocus(VOID)
{
    SetFocus(m_pTSFEditWnd->_GetWindow());
    
    return 0;
}

/**************************************************************************

   CTSFMainWnd::_OnKillFocus()

**************************************************************************/

LRESULT CTSFMainWnd::_OnKillFocus(VOID)
{
    return 0;
}

/**************************************************************************

   CTSFMainWnd::_OnNotify()

**************************************************************************/

LRESULT CTSFMainWnd::_OnNotify(UINT, LPNMHDR)
{
    return 0;
}

/**************************************************************************

   CTSFMainWnd::_OnSize()

**************************************************************************/

LRESULT CTSFMainWnd::_OnSize(WORD wWidth, WORD wHeight)
{
    //position the edit window to fill the client area
    MoveWindow(m_pTSFEditWnd->_GetWindow(), 0, 0, wWidth, wHeight, TRUE);
    
    return 0;
}

/**************************************************************************

   CTSFMainWnd::_OnActivate()

**************************************************************************/

LRESULT CTSFMainWnd::_OnActivate(WPARAM)
{
    return 0;
}

/**************************************************************************

	CTSFMainWnd::_OnInitMenuPopup()

**************************************************************************/

LRESULT CTSFMainWnd::_OnInitMenuPopup(WPARAM wParam, LPARAM lParam)
{
    return m_pTSFEditWnd->_OnInitMenuPopup(wParam, lParam);
}


/**************************************************************************

	CTSFMainWnd::GetFileName()

**************************************************************************/

BOOL CTSFMainWnd::_GetFileName(HWND hwndOwner, LPTSTR lpszFile, ULONG uChars, BOOL fOpen)
{
    OPENFILENAME    ofn;
    BOOL            fReturn = FALSE;

    *lpszFile = 0;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner   = hwndOwner;
    ofn.lpstrFilter = TEXT("TSFApp Files (*.tsf)\0*.tsf\0");
    ofn.lpstrFile   = lpszFile;
    ofn.nMaxFile    = uChars;
    ofn.Flags       = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = TEXT("tsf");

    if(fOpen)
    {
        ofn.Flags |= OFN_FILEMUSTEXIST;

        fReturn = GetOpenFileName(&ofn);
    }
    else
    {
        fReturn = GetSaveFileName(&ofn);
    }

    return fReturn;
}


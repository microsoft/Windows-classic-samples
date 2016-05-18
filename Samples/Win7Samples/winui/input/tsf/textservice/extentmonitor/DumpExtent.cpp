//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  DumpExtent.cpp
//
//          DumpExtent
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"
#include "TextService.h"
#include "EditSession.h"
#include "MemoryStream.h"

#define GETTEXT_CNT 256


//+---------------------------------------------------------------------------
//
// CLSIDToStringW
//
//----------------------------------------------------------------------------

BOOL CLSIDToStringW(REFGUID refGUID, WCHAR *pch)
{
    static const BYTE GuidMap[] = {3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                   8, 9, '-', 10, 11, 12, 13, 14, 15};

    static const WCHAR szDigits[] = L"0123456789ABCDEF";

    int i;
    WCHAR *p = pch;

    const BYTE * pBytes = (const BYTE *) &refGUID;

    *p++ = L'{';
    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == L'-')
        {
            *p++ = L'-';
        }
        else
        {
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *p++ = L'}';
    *p   = L'\0';

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// CDumpExtentEditSession
//
//----------------------------------------------------------------------------

class CDumpExtentEditSession : public CEditSessionBase
{
public:
    CDumpExtentEditSession(CExtentMonitorTextService *pTextService, ITfContext *pContext, UINT nEventId) : CEditSessionBase(pTextService, pContext)
    {
        _nEventId = nEventId;
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

    UINT _nEventId;

};

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CDumpExtentEditSession::DoEditSession(TfEditCookie ec)
{
    _pTextService->_DumpExtent(ec, _pContext, _nEventId);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// DumpExtentFocusContext
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::DumpExtentFocusContext(UINT nEventId)
{
    ITfDocumentMgr *pDocMgrFocus;
    if ((_pThreadMgr->GetFocus(&pDocMgrFocus) == S_OK) &&
        (pDocMgrFocus != NULL))
    {
        ITfContext *pContext;
        if (SUCCEEDED(pDocMgrFocus->GetBase(&pContext)))
        {
            DumpExtent(pContext, nEventId);
            pContext->Release();
        }
        pDocMgrFocus->Release();
    }
}

//+---------------------------------------------------------------------------
//
// DumpExtent
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::DumpExtent(ITfContext *pContext, UINT nEventId)
{
    CDumpExtentEditSession *pEditSession;
    HRESULT hr = E_FAIL;

    if ((pEditSession = new CDumpExtentEditSession(this, pContext, nEventId)) == NULL)
        goto Exit;

    pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READ, &hr);

    pEditSession->Release();

Exit:
    return;
}


//+---------------------------------------------------------------------------
//
// _DumpExtent
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_DumpExtent(TfEditCookie ec, ITfContext *pContext, UINT nEventId)
{
    if (!_pMemStream)
        return;

    ClearStream(_pMemStream);

    switch (nEventId)
    {
        case DE_EVENTID_ACTIVATE:        
            AddStringToStream(_pMemStream, L"Event: Activate\r\n"); 
            break;
        case DE_EVENTID_ONSETFOCUS:
            AddStringToStream(_pMemStream, L"Event: OnSetFocus\r\n"); 
            break;
        case DE_EVENTID_ONENDEDIT:
            AddStringToStream(_pMemStream, L"Event: OnEndEdit\r\n"); 
            break;
        case DE_EVENTID_ONLAYOUTCHANGE:
            AddStringToStream(_pMemStream, L"Event: OnLayoutChange\r\n"); 
            break;
        case DE_EVENTID_FROMLANGUAGEBAR:
            AddStringToStream(_pMemStream, L"Event: From LanguageBar\r\n"); 
            break;
        default:
            AddStringToStream(_pMemStream, L"Event: Unknoen\r\n"); 
            break;
    }

    WCHAR sz[512];
    ITfContextView *pView = NULL;
    TF_SELECTION sel;

    memset(&_rcStartPos , 0, sizeof(RECT));
    memset(&_rcEndPos , 0, sizeof(RECT));
    memset(&_rcSelection , 0, sizeof(RECT));
    memset(&_rcView , 0, sizeof(RECT));

    if (SUCCEEDED(pContext->GetActiveView(&pView)))
    {
         ITfRange *pRange;
         RECT rc;
         BOOL fClipped;
         HWND hwnd;

         AddStringToStream(_pMemStream, L"Wnd Handle - ");
         if (SUCCEEDED(pView->GetWnd(&hwnd)))
         {
             WCHAR szWndClass[32];
             if (IsWindow(hwnd)) 
             {
                 GetClassNameW(hwnd, szWndClass, ARRAYSIZE(szWndClass));
                 StringCchPrintf(sz, ARRAYSIZE(sz), L"%08x %s\r\n", (DWORD)(ULONG_PTR)hwnd, szWndClass);
                 AddStringToStream(_pMemStream, sz);
             }
             else
             {
                 AddStringToStream(_pMemStream, L"null window handle\r\n");
             }
         }

         AddStringToStream(_pMemStream, L"Screen Ext\r\n");
         if (SUCCEEDED(pView->GetScreenExt(&rc)))
         {
             StringCchPrintf(sz, ARRAYSIZE(sz), L"    (%d, %d, %d, %d) - (%d, %d)\r\n", rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top);
             AddStringToStream(_pMemStream, sz);
             _rcView = rc;
         }

         AddStringToStream(_pMemStream, L"Start Pos\r\n");
         if (SUCCEEDED(pContext->GetStart(ec, &pRange)))
         {
             if (SUCCEEDED(pView->GetTextExt(ec, pRange, &rc, &fClipped)))
             {
                 StringCchPrintf(sz, ARRAYSIZE(sz), L"    (%d, %d, %d, %d) - (%d, %d) %s\r\n", rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top, fClipped ? L"Clipped" : L"");

                 AddStringToStream(_pMemStream, sz);
                 _rcStartPos = rc;
             }

             pRange->Release();
         }

         AddStringToStream(_pMemStream, L"End Pos\r\n");
         if (SUCCEEDED(pContext->GetEnd(ec, &pRange)))
         {
             if (SUCCEEDED(pView->GetTextExt(ec, pRange, &rc, &fClipped)))
             {
                 StringCchPrintf(sz, ARRAYSIZE(sz), L"    (%d, %d, %d, %d) - (%d, %d) %s\r\n", rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top, fClipped ? L"Clipped" : L"");

                 AddStringToStream(_pMemStream, sz);
                 _rcEndPos = rc;
             }

             pRange->Release();
         }

         AddStringToStream(_pMemStream, L"Selection Pos\r\n");
         ULONG cFetched;
         if (SUCCEEDED(pContext->GetSelection(ec, 0, 1, &sel, &cFetched)))
         {
             if (sel.range)
             {
                 if (SUCCEEDED(pView->GetTextExt(ec, sel.range, &rc, &fClipped)))
                 {
                     StringCchPrintf(sz, ARRAYSIZE(sz), L"    (%d, %d, %d, %d) - (%d, %d) %s\r\n", rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top, fClipped ? L"Clipped" : L"");

                     AddStringToStream(_pMemStream, sz);
                     _rcSelection = rc;
                 }
                 AddStringToStream(_pMemStream, L"    ");
                 _DumpRange(ec, sel.range);
                 AddStringToStream(_pMemStream, L"\r\n");
                 sel.range->Release();
             }
         }

         AddStringToStream(_pMemStream, L"Char Pos\r\n");
         if (SUCCEEDED(pContext->GetStart(ec, &pRange)))
         {
             LONG cch;
             memset(&_rcRanges[0] , 0, sizeof(RECT) * ARRAYSIZE(_rcRanges));
             if (SUCCEEDED(pRange->ShiftEnd(ec, 1, &cch, NULL)) && cch)
             {
                 for (int i = 0; i < ARRAYSIZE(_rcRanges); i++)
                 {
                     if (SUCCEEDED(pView->GetTextExt(ec, pRange, &rc, &fClipped)))
                     {
                         StringCchPrintf(sz, ARRAYSIZE(sz), L"    (%d, %d, %d, %d) - (%d, %d) %s", rc.left, rc.top, rc.right, rc.bottom, rc.right - rc.left, rc.bottom - rc.top, fClipped ? L"Clipped" : L"");
                         AddStringToStream(_pMemStream, sz);

                         AddStringToStream(_pMemStream, L" ");
                         ITfRange *pRangeTmp;
                         if (SUCCEEDED(pRange->Clone(&pRangeTmp)))
                         {
                             _DumpRange(ec, pRangeTmp);
                             pRangeTmp->Release();
                         }
                         AddStringToStream(_pMemStream, L"\r\n");

                         OffsetRect(&rc, 0 - _rcView.left, 0 - _rcView.top);
                         _rcRanges[i] = rc;
                     }

                     if (FAILED(pRange->ShiftEnd(ec, 1, &cch, NULL)) || !cch)
                         break;

                     if (FAILED(pRange->ShiftStart(ec, 1, &cch, NULL)) || !cch)
                         break;
                 }

             }
             pRange->Release();
         }

         pView->Release();
    }

    _EnsurePopupWindow();

    if (IsShownExtentVisualWindows())
        _UpdateExtentVisualWindows();

    if (IsShownRangeExtentViewer())
        _UpdateRangeExtentViewer();

    return;
}

//+---------------------------------------------------------------------------
//
// _DumpRange
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_DumpRange(TfEditCookie ec, ITfRange *prange)
{
    ULONG cchOut = 0;
    WCHAR sz[512];

    sz[0] = L'\0';
    prange->GetText(ec, TF_TF_MOVESTART, sz, ARRAYSIZE(sz) - 1, &cchOut);
    sz[cchOut] = L'\0';
    AddStringToStream(_pMemStream, sz);

    return;
}


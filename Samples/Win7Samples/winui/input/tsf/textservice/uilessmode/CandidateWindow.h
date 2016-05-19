//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  CandidateWindow.h
//
//          CCandidateWindow declaration.
//
//////////////////////////////////////////////////////////////////////

#ifndef CANDIDATEWINDOW_H
#define CANDIDATEWINDOW_H

#define MAX_CAND_STR 50

//+---------------------------------------------------------------------------
//
// CCandidateWindow
//
//----------------------------------------------------------------------------

class CCandidateWindow : public ITfCandidateListUIElementBehavior
{
public:
    CCandidateWindow(CTextService *pTextService);
    ~CCandidateWindow();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfUIElement
    STDMETHODIMP GetDescription(BSTR *bstr);
    STDMETHODIMP GetGUID(GUID *pguid);
    STDMETHODIMP Show(BOOL bShow);
    STDMETHODIMP IsShown(BOOL *pbShow);

    // ITfCandidateListUIElement
    STDMETHODIMP GetUpdatedFlags(DWORD *pdwFlags);
    STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppDocumentMgr);
    STDMETHODIMP GetCount(UINT *puCount);
    STDMETHODIMP GetSelection(UINT *puIndex);
    STDMETHODIMP GetString(UINT uIndex, BSTR *pstr);
    STDMETHODIMP GetPageIndex(UINT *pIndex,
                              UINT uSize,
                              UINT *puPageCnt);
    STDMETHODIMP SetPageIndex(UINT *pIndex,
                              UINT uPageCnt);
    STDMETHODIMP GetCurrentPage(UINT *puPage);

    // ITfCandidateListUIElementBehavior
    STDMETHODIMP SetSelection(UINT nIndex);
    STDMETHODIMP Finalize();
    STDMETHODIMP Abort();

    static BOOL _InitWindowClass();
    static void _UninitWindowClass();

    BOOL _Create();
    void _Destroy();

    void _Move(int x, int y);
    void _Begin();
    void _End();

    HRESULT _OnKeyDown(UINT uVKey);
    HRESULT _OnKeyUp(UINT uVKey);

private:
    void _GetCompositionText();
    void _InitList();
    void _CallUpdateUIElement();
    void _Next();
    void _Prev();
    void _NextPage();
    void _PrevPage();

    static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static void _SetThis(HWND hwnd, LPARAM lParam)
    {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 
                         (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
    }

    static CCandidateWindow *_GetThis(HWND hwnd)
    {
        return (CCandidateWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    static ATOM _atomWndClass;

    HWND _hwnd;
    CTextService *_pTextService;

    // the converted text
    WCHAR _szText[256];

    // Candidate list UIElement information
    UINT _uCandList;
    WCHAR *_arCandStr[MAX_CAND_STR];
    UINT _uSelection;
    UINT _arPageIndex[MAX_CAND_STR];
    UINT _uPageCnt;
    DWORD _dwUpdatetFlags;

    DWORD _dwUIElementId;

    BOOL _bInShowMode;

    LONG _cRef;
};

#endif // CANDIDATEWINDOW_H

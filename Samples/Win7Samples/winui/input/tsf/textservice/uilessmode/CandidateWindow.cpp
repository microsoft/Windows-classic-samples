//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  CandidateWindow.cpp
//
//          CCandidateWindow class
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TextService.h"
#include "CandidateWindow.h"
#include "EditSession.h"
#include "CandidateList.h"
#include "time.h"

#define CAND_WIDTH     200
#define CAND_HEIGHT    50

ATOM CCandidateWindow::_atomWndClass = 0;

const TCHAR c_szCandidateDescription[] = TEXT("Dummy Candidate Window");

/* 3e5fdd2d-bbf6-46a9-aded-b480fe18f8d0 */
const GUID c_guidCandUIElement = {
    0x3e5fdd2d,
    0xbbf6,
    0x46a9,
    {0xad, 0xed, 0xb4, 0x80, 0xfe, 0x18, 0xf8, 0xd0}
  };

//+---------------------------------------------------------------------------
//
// CGetCompositionEditSession
//
//----------------------------------------------------------------------------

class CGetCompositionEditSession : public CEditSessionBase
{
public:
    CGetCompositionEditSession(CTextService *pTextService, ITfComposition *pComposition, ITfContext *pContext) : CEditSessionBase(pTextService, pContext)
    {
        _pComposition = pComposition;
        _pComposition->AddRef();
        _szText[0] = L'\0';
    }

    ~CGetCompositionEditSession()
    {
        _pComposition->Release();
    }


    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

    WCHAR *GetText()
    {
        return _szText;
    }

private:
    ITfComposition *_pComposition;
    WCHAR _szText[256];
};

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CGetCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    ITfRange *pRange;
    ULONG cch = 0;

    if (_pComposition->GetRange(&pRange) != S_OK)
        return E_FAIL;

    pRange->GetText(ec, 0, _szText, ARRAYSIZE(_szText) - 1, &cch);
    _szText[cch] = L'\0';

    pRange->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CCandidateWindow::CCandidateWindow(CTextService *pTextService)
{
    _hwnd = NULL;
    _pTextService = pTextService;
    _pTextService->AddRef();

    _bInShowMode = FALSE;

    int i;
    _uCandList = 0;
    for (i = 0; i < MAX_CAND_STR; i++)
        _arCandStr[i] = NULL;

    _uPageCnt = 0;
    for (i = 0; i < MAX_CAND_STR; i++)
        _arPageIndex[i] = 0;

    _uSelection = 0;
    _dwUpdatetFlags = 0;

    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CCandidateWindow::~CCandidateWindow()
{
    _pTextService->Release();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfUIElement) ||
        IsEqualIID(riid, IID_ITfCandidateListUIElement) ||
        IsEqualIID(riid, IID_ITfCandidateListUIElementBehavior))
    {
        *ppvObj = (ITfCandidateListUIElementBehavior *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCandidateWindow::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CCandidateWindow::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// GetDescription
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::GetDescription(BSTR *bstr)
{
   *bstr = SysAllocString(L"Sample Candidate Window");
   return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetGUID
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::GetGUID(GUID *pguid)
{
   *pguid = c_guidCandUIElement;
   return S_OK;
}

//+---------------------------------------------------------------------------
//
// Show
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::Show(BOOL bShow)
{
   if (!_bInShowMode)
       return E_UNEXPECTED;

   if (bShow)
       ShowWindow(_hwnd, SW_SHOWNA);
   else
   {
       ShowWindow(_hwnd, SW_HIDE);
       _CallUpdateUIElement();
   }
   return S_OK;
}

//+---------------------------------------------------------------------------
//
// IsShown
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::IsShown(BOOL *pbShow)
{
   *pbShow = IsWindowVisible(_hwnd);
   return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetUpdatedFlags
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::GetUpdatedFlags(DWORD *pdwFlags)
{
    *pdwFlags = _dwUpdatetFlags;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetDocumentMgr
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::GetDocumentMgr(ITfDocumentMgr **ppDocumentMgr)
{
    *ppDocumentMgr = NULL;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetCount
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::GetCount(UINT *puCount)
{
    *puCount = _uCandList;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetGUID
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::GetSelection(UINT *puIndex)
{
    *puIndex = _uSelection;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetString
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::GetString(UINT uIndex, BSTR *pstr)
{
    *pstr = SysAllocString(_arCandStr[uIndex]);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetPageIndex
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt)
{
    UINT i;
    HRESULT hr = S_OK;

    if (uSize >= _uPageCnt)
        uSize = _uPageCnt;
    else
        hr = S_FALSE;

    for (i = 0; i < uSize; i++)
    {
        *pIndex = _arPageIndex[i];
        pIndex++;
    }

    *puPageCnt = _uPageCnt;
    return hr;
}

//+---------------------------------------------------------------------------
//
// SetPageIndex
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::SetPageIndex(UINT *pIndex, UINT uPageCnt)
{
    UINT i;
    for (i = 0; i < uPageCnt; i++)
    {
        _arPageIndex[i] = *pIndex;
        pIndex++;
    }
    _uPageCnt = uPageCnt;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetCurrentPage
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::GetCurrentPage(UINT *puPage)
{
    UINT i;
    if (!puPage)
        return E_INVALIDARG;

    *puPage = 0;

    if (_uPageCnt == 0)
        return E_UNEXPECTED;

    if (_uPageCnt == 1)
    {
        *puPage = 0;
        return S_OK;
    }

    for (i = 1; i < _uPageCnt; i++)
    {
        if (_arPageIndex[i] > _uSelection)
            break;
    }
  
    *puPage = i - 1;
    return S_OK;
}


//+---------------------------------------------------------------------------
//
// SetSelection
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::SetSelection(UINT nIndex)
{
    if (nIndex >= _uCandList)
        return E_INVALIDARG;

    UINT uOldPage;
    UINT uNewPage;

    GetCurrentPage(&uOldPage);
    _uSelection = nIndex;
    GetCurrentPage(&uNewPage);

    _dwUpdatetFlags = TF_CLUIE_SELECTION;
    if (uNewPage != uOldPage)
        _dwUpdatetFlags |= TF_CLUIE_CURRENTPAGE;

    _CallUpdateUIElement();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Finalize
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::Finalize()
{
    if (_pTextService->_GetCandidateList())
        _pTextService->_GetCandidateList()->_EndCandidateList();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Abort
//
//----------------------------------------------------------------------------

STDAPI CCandidateWindow::Abort()
{
    if (_pTextService->_GetCandidateList())
        _pTextService->_GetCandidateList()->_EndCandidateList();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitWindowClass
//
//----------------------------------------------------------------------------

/* static */
BOOL CCandidateWindow::_InitWindowClass()
{
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = CCandidateWindow::_WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("TextServiceCandidateWindow");

    _atomWndClass = RegisterClass(&wc);

    return (_atomWndClass != 0);
}


//+---------------------------------------------------------------------------
//
// _UninitClass
//
//----------------------------------------------------------------------------

/* static */
void CCandidateWindow::_UninitWindowClass()
{
    if (_atomWndClass != 0)
    {
        UnregisterClass((LPCTSTR)_atomWndClass, g_hInst);
    }
}


//+---------------------------------------------------------------------------
//
// _Create
//
//----------------------------------------------------------------------------

BOOL CCandidateWindow::_Create()
{
    _hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                           (LPCTSTR)_atomWndClass,
                           TEXT("TextService Candidate Window"),
                           WS_BORDER | WS_DISABLED | WS_POPUP,
                           0, 0,
                           CAND_WIDTH, CAND_HEIGHT,
                           NULL,
                           NULL,
                           g_hInst,
                           this);

    return (_hwnd != NULL);
}

//+---------------------------------------------------------------------------
//
// _Destroy
//
//----------------------------------------------------------------------------

void CCandidateWindow::_Destroy()
{
    if (_hwnd != NULL)
    {
        DestroyWindow(_hwnd);
        _hwnd = NULL;
    }
}

//+---------------------------------------------------------------------------
//
// _Move
//
//----------------------------------------------------------------------------

void CCandidateWindow::_Move(int x, int y)
{
    if (_hwnd != NULL)
    {
        RECT rc;
        GetWindowRect(_hwnd, &rc);
        MoveWindow(_hwnd, x, y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }
}

//+---------------------------------------------------------------------------
//
// _GetCompositionText
//
//----------------------------------------------------------------------------

void CCandidateWindow::_GetCompositionText()
{
    ITfContext *pContext = NULL;
    ITfRange *pRange = NULL;
    CGetCompositionEditSession *pGetCompositionEditSession;
    HRESULT hr;

    if (_pTextService->_GetComposition() == NULL) {
        return;
    }

    if (_pTextService->_GetComposition()->GetRange(&pRange) != S_OK) {
        goto Exit;
    }

    if (pRange->GetContext(&pContext) != S_OK) {
        goto Exit;
    }

    if (pGetCompositionEditSession = new CGetCompositionEditSession(_pTextService, _pTextService->_GetComposition(), pContext)) {
        pContext->RequestEditSession(_pTextService->_GetClientId(), 
                                     pGetCompositionEditSession, 
                                     TF_ES_READ | TF_ES_SYNC, 
                                     &hr);

        StringCchCopy(_szText, ARRAYSIZE(_szText), pGetCompositionEditSession->GetText());
        pGetCompositionEditSession->Release();
    }

Exit:
    if (pRange)
        pRange->Release();
    if (pContext)
        pContext->Release();
}


//+---------------------------------------------------------------------------
//
// _InitList
//
//----------------------------------------------------------------------------

void CCandidateWindow::_InitList()
{
    BOOL fRand = FALSE;
    BOOL fLong = FALSE;
    UINT uListNum = 5;

    //
    // Get text from composing
    //
    _GetCompositionText();

    WCHAR *psz = _szText;
    if (*psz)
    {
        if (*psz == L'R')
        {
            fRand = TRUE;
            psz++;
        }

        if (*psz == L'L')
        {
            fLong = TRUE;
            psz++;
        }
    }

    uListNum = _wtoi(psz);

    UINT i;
    
    for (i = 0; i < _uCandList; i++)
    {
        if (_arCandStr[i])
            delete _arCandStr[i];
    }

    _uCandList = 0;
    for (i = 0; i < MAX_CAND_STR; i++)
        _arCandStr[i] = NULL;

    _uPageCnt = 0;
    for (i = 0; i < MAX_CAND_STR; i++)
        _arPageIndex[i] = 0;

    if (uListNum)
    {
        if (uListNum > MAX_CAND_STR)
            uListNum = MAX_CAND_STR;

        _uCandList = uListNum;

        srand( (unsigned)time( NULL ) );

        for (i = 0; i < _uCandList; i++)
        {
            _arCandStr[i] = new WCHAR[50];
            if (fLong)
                StringCchPrintf(_arCandStr[i], 50, L"LongLongLong%d", i);
            else if (fRand)
            {
                int nLen = rand() % 15 + 1;
                int nCur = 0;
                while(nCur < nLen)
                   _arCandStr[i][nCur++] = rand() % 26 + L'A';
                _arCandStr[i][nCur++] = L'\0';
            }
            else
                StringCchPrintf(_arCandStr[i], 50, L"Test%d", i);
        }

        _uPageCnt = (_uCandList / 10) + 1;

        for (i = 0; i < _uPageCnt; i++)
            _arPageIndex[i] = i * 10;

        _uSelection = 0;
    }

    _dwUpdatetFlags = TF_CLUIE_DOCUMENTMGR |
                      TF_CLUIE_COUNT       |
                      TF_CLUIE_SELECTION   |
                      TF_CLUIE_STRING      |
                      TF_CLUIE_PAGEINDEX   |
                      TF_CLUIE_CURRENTPAGE;
}

//+---------------------------------------------------------------------------
//
// _Begin
//
//----------------------------------------------------------------------------

void CCandidateWindow::_Begin()
{
    ITfUIElementMgr *pUIElementMgr;
    BOOL bShow = TRUE;

    _InitList();

    if (!_bInShowMode)
    {
        if (SUCCEEDED(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr,
                                                         (void **)&pUIElementMgr)))
        {
            pUIElementMgr->BeginUIElement(this, &bShow, &_dwUIElementId);
            if (!bShow)
                pUIElementMgr->UpdateUIElement(_dwUIElementId);
            pUIElementMgr->Release();
        }
    }


    if (bShow)
        ShowWindow(_hwnd, SW_SHOWNA);

    _bInShowMode = TRUE;
}

//+---------------------------------------------------------------------------
//
// _End
//
//----------------------------------------------------------------------------

void CCandidateWindow::_End()
{
    if (_bInShowMode)
    {
        ITfUIElementMgr *pUIElementMgr;
        if (SUCCEEDED(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr, 
                                                         (void **)&pUIElementMgr)))
        {
            pUIElementMgr->EndUIElement(_dwUIElementId);
            pUIElementMgr->Release();
        }
    }
    ShowWindow(_hwnd, SW_HIDE);
    _bInShowMode = FALSE;
}

//+---------------------------------------------------------------------------
//
// _CallUpdateUIElement
//
//----------------------------------------------------------------------------

void CCandidateWindow::_CallUpdateUIElement()
{
    //
    // we don't have to call UpdateUIElement when we show our own UI.
    //
    if (_bInShowMode && !IsWindowVisible(_hwnd))
    {
        ITfUIElementMgr *pUIElementMgr;
        if (SUCCEEDED(_pTextService->_GetThreadMgr()->QueryInterface(IID_ITfUIElementMgr, 
                                                         (void **)&pUIElementMgr)))
        {
            pUIElementMgr->UpdateUIElement(_dwUIElementId);
            pUIElementMgr->Release();
        }
    }
}

//+---------------------------------------------------------------------------
//
// _Next
//
//----------------------------------------------------------------------------

void CCandidateWindow::_Next()
{
    UINT uOldPage;
    UINT uNewPage;

    GetCurrentPage(&uOldPage);
    _uSelection++;
    if (_uSelection >= _uCandList)
        _uSelection = 0;
    GetCurrentPage(&uNewPage);

    _dwUpdatetFlags = TF_CLUIE_SELECTION;
    if (uNewPage != uOldPage)
        _dwUpdatetFlags |= TF_CLUIE_CURRENTPAGE;

    _CallUpdateUIElement();
}

//+---------------------------------------------------------------------------
//
// _Prev
//
//----------------------------------------------------------------------------

void CCandidateWindow::_Prev()
{
    UINT uOldPage;
    UINT uNewPage;

    GetCurrentPage(&uOldPage);
    if (_uSelection > 0)
        _uSelection--;
    else
        _uSelection = _uCandList - 1;

    GetCurrentPage(&uNewPage);

    _dwUpdatetFlags = TF_CLUIE_SELECTION;
    if (uNewPage != uOldPage)
        _dwUpdatetFlags |= TF_CLUIE_CURRENTPAGE;

    _CallUpdateUIElement();
}

//+---------------------------------------------------------------------------
//
// _NextPage
//
//----------------------------------------------------------------------------

void CCandidateWindow::_NextPage()
{
    UINT uOldPage;
    UINT uNewPage;

    GetCurrentPage(&uOldPage);
    uNewPage = uOldPage + 1;
    if (uNewPage >= _uPageCnt)
        uNewPage = 0;

    _uSelection = _arPageIndex[uNewPage];

    _dwUpdatetFlags = TF_CLUIE_SELECTION;
    if (uNewPage != uOldPage)
        _dwUpdatetFlags |= TF_CLUIE_CURRENTPAGE;

    _CallUpdateUIElement();
}

//+---------------------------------------------------------------------------
//
// _PrevPage
//
//----------------------------------------------------------------------------

void CCandidateWindow::_PrevPage()
{
    UINT uOldPage;
    UINT uNewPage;

    GetCurrentPage(&uOldPage);
    if (uOldPage > 0)
        uNewPage = uOldPage - 1;
    else
        uNewPage = _uPageCnt - 1;

    _uSelection = _arPageIndex[uNewPage];

    _dwUpdatetFlags = TF_CLUIE_SELECTION;
    if (uNewPage != uOldPage)
        _dwUpdatetFlags |= TF_CLUIE_CURRENTPAGE;

    _CallUpdateUIElement();
}
//+---------------------------------------------------------------------------
//
// _OnKeyDown
//
//----------------------------------------------------------------------------

HRESULT CCandidateWindow::_OnKeyDown(UINT uVKey)
{
    switch (uVKey)
    {
        case VK_UP:
            _Prev();
            break;
        case VK_DOWN:
            _Next();
            break;
        case VK_NEXT:
            _NextPage();
            break;
        case VK_PRIOR:
            _PrevPage();
            break;
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _OnKeyUp
//
//----------------------------------------------------------------------------

HRESULT CCandidateWindow::_OnKeyUp(UINT uVKey)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _WindowProc
//
// Cand window proc.
//----------------------------------------------------------------------------

/* static */
LRESULT CALLBACK CCandidateWindow::_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (uMsg)
    {
        case WM_CREATE:
            _SetThis(hwnd, lParam);
            return 0;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            SetBkMode(hdc, TRANSPARENT);
            TextOut(hdc, 0, 0, c_szCandidateDescription, lstrlen(c_szCandidateDescription));
            EndPaint(hwnd, &ps);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


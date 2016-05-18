//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  LanguageBar.cpp
//
//          Language Bar UI code.
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TextService.h"
#include "Resource.h"
#include "PopupWindow.h"

//
// The cookie for the sink to CLangBarItemButton.
//
#define TEXTSERVICE_LANGBARITEMSINK_COOKIE 0x0fab0fab

//
// The ids of the menu item of the language bar button.
//
#define MENUITEM_INDEX_SHOWPOPUPWINDOW          0
#define MENUITEM_INDEX_UPDATEPOPUPWINDOW        1
#define MENUITEM_INDEX_SHOWEXTENTVISUALWINDOWS  2
#define MENUITEM_INDEX_SHOWRANGEEXTENTVIEWER    3
#define MENUITEM_INDEX_SHOWRANGEFROMPOINTVIEWER 4

//
// The descriptions of the menu item of the language bar button.
//
static WCHAR c_szMenuItemDescriptionShowPopupWindow[] = L"Show PopupWindow";
static WCHAR c_szMenuItemDescriptionUpdatePopupWindow[] = L"Update PopupWindow";
static WCHAR c_szMenuItemDescriptionShowExtentVisualWindows[] = L"Show Positions";
static WCHAR c_szMenuItemDescriptionShowRangeExtentViewer[] = L"Show RangeExtentView";
static WCHAR c_szMenuItemDescriptionShowRangeFromPointViewer[] = L"Show RangeFromPointView";

//+---------------------------------------------------------------------------
//
// CLangBarItemButton class
//
//----------------------------------------------------------------------------

class CLangBarItemButton : public ITfLangBarItemButton,
                           public ITfSource
{
public:
    CLangBarItemButton(CExtentMonitorTextService *pTextService);
    ~CLangBarItemButton();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfLangBarItem
    STDMETHODIMP GetInfo(TF_LANGBARITEMINFO *pInfo);
    STDMETHODIMP GetStatus(DWORD *pdwStatus);
    STDMETHODIMP Show(BOOL fShow);
    STDMETHODIMP GetTooltipString(BSTR *pbstrToolTip);

    // ITfLangBarItemButton
    STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT *prcArea);
    STDMETHODIMP InitMenu(ITfMenu *pMenu);
    STDMETHODIMP OnMenuSelect(UINT wID);
    STDMETHODIMP GetIcon(HICON *phIcon);
    STDMETHODIMP GetText(BSTR *pbstrText);

    // ITfSource
    STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie);
    STDMETHODIMP UnadviseSink(DWORD dwCookie);

private:
    ITfLangBarItemSink *_pLangBarItemSink;
    TF_LANGBARITEMINFO _tfLangBarItemInfo;

    CExtentMonitorTextService *_pTextService;
    LONG _cRef;
};

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CLangBarItemButton::CLangBarItemButton(CExtentMonitorTextService *pTextService)
{
    DllAddRef();

    //
    // initialize TF_LANGBARITEMINFO structure.
    //
    _tfLangBarItemInfo.clsidService = c_clsidExtentMonitorTextService;    // This LangBarItem belongs to this TextService.
    _tfLangBarItemInfo.guidItem = c_guidLangBarItemButton;   // GUID of this LangBarItem.
    _tfLangBarItemInfo.dwStyle = TF_LBI_STYLE_BTN_MENU;      // This LangBar is a button type with a menu.
    _tfLangBarItemInfo.ulSort = 0;                           // The position of this LangBar Item is not specified.
    StringCchCopy(_tfLangBarItemInfo.szDescription, ARRAYSIZE(_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC);                        // Set the description of this LangBar Item.

    // Initialize the sink pointer to NULL.
    _pLangBarItemSink = NULL;

    _pTextService = pTextService;
    _pTextService->AddRef();

    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CLangBarItemButton::~CLangBarItemButton()
{
    DllRelease();
    _pTextService->Release();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItem) ||
        IsEqualIID(riid, IID_ITfLangBarItemButton))
    {
        *ppvObj = (ITfLangBarItemButton *)this;
    }
    else if (IsEqualIID(riid, IID_ITfSource))
    {
        *ppvObj = (ITfSource *)this;
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

STDAPI_(ULONG) CLangBarItemButton::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CLangBarItemButton::Release()
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
// GetInfo
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetInfo(TF_LANGBARITEMINFO *pInfo)
{
    *pInfo = _tfLangBarItemInfo;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetStatus
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetStatus(DWORD *pdwStatus)
{
    *pdwStatus = 0;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Show
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::Show(BOOL fShow)
{
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// GetTooltipString
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetTooltipString(BSTR *pbstrToolTip)
{
    *pbstrToolTip = SysAllocString(LANGBAR_ITEM_DESC);

    return (*pbstrToolTip == NULL) ? E_OUTOFMEMORY : S_OK;
}

//+---------------------------------------------------------------------------
//
// OnClick
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::OnClick(TfLBIClick click, POINT pt, const RECT *prcArea)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// InitMenu
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::InitMenu(ITfMenu *pMenu)
{
    // 
    // Add the fisrt menu item.
    // 
    DWORD dwFlags = 0;
    if (_pTextService->_GetPopupWindow() &&
        _pTextService->_GetPopupWindow()->IsShown())
    {
        dwFlags |= TF_LBMENUF_CHECKED;
    }

    pMenu->AddMenuItem(MENUITEM_INDEX_SHOWPOPUPWINDOW,
                       dwFlags, 
                       NULL, 
                       NULL, 
                       c_szMenuItemDescriptionShowPopupWindow, 
                       (ULONG)wcslen(c_szMenuItemDescriptionShowPopupWindow), 
                       NULL);

    pMenu->AddMenuItem(MENUITEM_INDEX_UPDATEPOPUPWINDOW,
                       0, 
                       NULL, 
                       NULL, 
                       c_szMenuItemDescriptionUpdatePopupWindow, 
                       (ULONG)wcslen(c_szMenuItemDescriptionUpdatePopupWindow), 
                       NULL);

    pMenu->AddMenuItem(MENUITEM_INDEX_SHOWEXTENTVISUALWINDOWS,
                       _pTextService->IsShownExtentVisualWindows() ? TF_LBMENUF_CHECKED :  0, 
                       NULL, 
                       NULL, 
                       c_szMenuItemDescriptionShowExtentVisualWindows, 
                       (ULONG)wcslen(c_szMenuItemDescriptionShowExtentVisualWindows), 
                       NULL);

    pMenu->AddMenuItem(MENUITEM_INDEX_SHOWRANGEEXTENTVIEWER,
                       _pTextService->IsShownRangeExtentViewer() ? TF_LBMENUF_CHECKED :  0, 
                       NULL, 
                       NULL, 
                       c_szMenuItemDescriptionShowRangeExtentViewer, 
                       (ULONG)wcslen(c_szMenuItemDescriptionShowRangeExtentViewer), 
                       NULL);

    pMenu->AddMenuItem(MENUITEM_INDEX_SHOWRANGEFROMPOINTVIEWER,
                       _pTextService->IsShownRangeFromPointViewer() ? TF_LBMENUF_CHECKED :  0, 
                       NULL, 
                       NULL, 
                       c_szMenuItemDescriptionShowRangeFromPointViewer, 
                       (ULONG)wcslen(c_szMenuItemDescriptionShowRangeFromPointViewer), 
                       NULL);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnMenuSelect
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::OnMenuSelect(UINT wID)
{
    //
    // This is callback when the menu item is selected.
    //
    switch (wID)
    {
        case MENUITEM_INDEX_SHOWPOPUPWINDOW:
            if (_pTextService->_GetPopupWindow())
            {
                if (_pTextService->_GetPopupWindow()->IsShown())
                    _pTextService->_GetPopupWindow()->Hide();
                else
                    _pTextService->_GetPopupWindow()->Show();
            }
            break;

        case MENUITEM_INDEX_UPDATEPOPUPWINDOW:
            _pTextService->DumpExtentFocusContext(DE_EVENTID_FROMLANGUAGEBAR);
            if (_pTextService->_GetPopupWindow())
            {
                if (_pTextService->_GetPopupWindow()->IsShown())
                    _pTextService->_GetPopupWindow()->Show();
            }
            break;

        case MENUITEM_INDEX_SHOWEXTENTVISUALWINDOWS:
            if (_pTextService->IsShownExtentVisualWindows())
                _pTextService->_HideExtentVisualWindows();
            else
                _pTextService->_UpdateExtentVisualWindows();
            break;

        case MENUITEM_INDEX_SHOWRANGEEXTENTVIEWER:
            if (_pTextService->IsShownRangeExtentViewer())
                _pTextService->_HideRangeExtentViewer();
            else
                _pTextService->_UpdateRangeExtentViewer();
            break;

        case MENUITEM_INDEX_SHOWRANGEFROMPOINTVIEWER:
            if (_pTextService->IsShownRangeFromPointViewer())
                _pTextService->_HideRangeFromPointViewer();
            else
                _pTextService->_UpdateRangeFromPointViewer();
            break;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetIcon
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetIcon(HICON *phIcon)
{
    *phIcon = (HICON)LoadImage(g_hInst, TEXT("IDI_TEXTSERVICE"), IMAGE_ICON, 16, 16, 0);
 
    return (*phIcon != NULL) ? S_OK : E_FAIL;
}

//+---------------------------------------------------------------------------
//
// GetText
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetText(BSTR *pbstrText)
{
    *pbstrText = SysAllocString(LANGBAR_ITEM_DESC);

    return (*pbstrText == NULL) ? E_OUTOFMEMORY : S_OK;
}

//+---------------------------------------------------------------------------
//
// AdviseSink
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    //
    // We allow only ITfLangBarItemSink interface.
    //
    if (!IsEqualIID(IID_ITfLangBarItemSink, riid))
        return CONNECT_E_CANNOTCONNECT;

    //
    // We support only one sink once.
    //
    if (_pLangBarItemSink != NULL)
        return CONNECT_E_ADVISELIMIT;

    //
    // Query the ITfLangBarItemSink interface and store it into _pLangBarItemSink.
    //
    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = NULL;
        return E_NOINTERFACE;
    }

    //
    // return our cookie.
    //
    *pdwCookie = TEXTSERVICE_LANGBARITEMSINK_COOKIE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// UnadviseSink
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::UnadviseSink(DWORD dwCookie)
{
    // 
    // Check the given cookie.
    // 
    if (dwCookie != TEXTSERVICE_LANGBARITEMSINK_COOKIE)
        return CONNECT_E_NOCONNECTION;

    //
    // If there is nno connected sink, we just fail.
    //
    if (_pLangBarItemSink == NULL)
        return CONNECT_E_NOCONNECTION;

    _pLangBarItemSink->Release();
    _pLangBarItemSink = NULL;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitLanguageBar
//
//----------------------------------------------------------------------------

BOOL CExtentMonitorTextService::_InitLanguageBar()
{
    ITfLangBarItemMgr *pLangBarItemMgr;
    BOOL fRet;

    if (_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) != S_OK)
        return FALSE;

    fRet = FALSE;

    if ((_pLangBarItem = new CLangBarItemButton(this)) == NULL)
        goto Exit;

    if (pLangBarItemMgr->AddItem(_pLangBarItem) != S_OK)
    {
        _pLangBarItem->Release();
        _pLangBarItem = NULL;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    pLangBarItemMgr->Release();
    return fRet;
}

//+---------------------------------------------------------------------------
//
// _UninitLanguageBar
//
//----------------------------------------------------------------------------

void CExtentMonitorTextService::_UninitLanguageBar()
{
    ITfLangBarItemMgr *pLangBarItemMgr;

    if (_pLangBarItem == NULL)
        return;

    if (_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) == S_OK)
    {
        pLangBarItemMgr->RemoveItem(_pLangBarItem);
        pLangBarItemMgr->Release();
    }

    _pLangBarItem->Release();
    _pLangBarItem = NULL;
}

//
// langbar.cpp
//
// Language bar ui code.
//

#include "globals.h"
#include "mark.h"
#include "resource.h"

#define MARK_LANGBARITEMSINK_COOKIE 0x0fab0fab

const struct
{
    const WCHAR *pchDesc;
    void (*pfnHandler)(CMarkTextService *_this);
}
c_rgMenuItems[] =
{
    { L"Start/End Composition", CMarkTextService::_Menu_OnComposition },
    { L"Set Global Compartment", CMarkTextService::_Menu_OnSetGlobalCompartment },
    { L"Set Case Property", CMarkTextService::_Menu_OnSetCaseProperty },
    { L"Set Custom Property", CMarkTextService::_Menu_OnSetCustomProperty },
    { L"View Case Property", CMarkTextService::_Menu_OnViewCaseProperty },
    { L"View Custom Property", CMarkTextService::_Menu_OnViewCustomProperty },
};

class CLangBarItemButton : public ITfLangBarItemButton,
                           public ITfSource
{
public:
    CLangBarItemButton(CMarkTextService *pMark);
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
    CMarkTextService *_pMark;
    ITfLangBarItemSink *_pLangBarItemSink;
    TF_LANGBARITEMINFO _tfLangBarItemInfo;
    LONG _cRef;
};

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CLangBarItemButton::CLangBarItemButton(CMarkTextService *pMark)
{
    DllAddRef();

    _tfLangBarItemInfo.clsidService = c_clsidMarkTextService;
    _tfLangBarItemInfo.guidItem = c_guidLangBarItemButton;
    _tfLangBarItemInfo.dwStyle = TF_LBI_STYLE_BTN_MENU;
    _tfLangBarItemInfo.ulSort = 0;
    wcscpy_s(_tfLangBarItemInfo.szDescription, ARRAYSIZE(_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC);

    _pMark = pMark;
    _pMark->AddRef();
    _pLangBarItemSink = NULL;

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
    _pMark->Release();
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
    int i;

    for (i=0; i<ARRAYSIZE(c_rgMenuItems); i++)
    {
        pMenu->AddMenuItem(i, 0, NULL, NULL, c_rgMenuItems[i].pchDesc, (ULONG)wcslen(c_rgMenuItems[i].pchDesc), NULL);
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnMenuSelect
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::OnMenuSelect(UINT wID)
{
    c_rgMenuItems[wID].pfnHandler(_pMark);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetIcon
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetIcon(HICON *phIcon)
{
    *phIcon = (HICON)LoadImage(g_hInst, TEXT("IDI_MARK"), IMAGE_ICON, 16, 16, 0);

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
    if (!IsEqualIID(IID_ITfLangBarItemSink, riid))
        return CONNECT_E_CANNOTCONNECT;

    if (_pLangBarItemSink != NULL)
        return CONNECT_E_ADVISELIMIT;

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = NULL;
        return E_NOINTERFACE;
    }

    *pdwCookie = MARK_LANGBARITEMSINK_COOKIE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// UnadviseSink
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::UnadviseSink(DWORD dwCookie)
{
    if (dwCookie != MARK_LANGBARITEMSINK_COOKIE)
        return CONNECT_E_NOCONNECTION;

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

BOOL CMarkTextService::_InitLanguageBar()
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

void CMarkTextService::_UninitLanguageBar()
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

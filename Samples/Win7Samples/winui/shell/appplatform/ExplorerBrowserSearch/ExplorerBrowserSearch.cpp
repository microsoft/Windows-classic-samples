// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#define STRICT_TYPED_ITEMIDS    // in case you do IDList stuff you want this on for better type saftey

#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA

#include <structuredquery.h>
#include <new>  // std::nothrow
#include "ShellHelpers.h"
#include "ResizeableDialog.h"
#include "resource.h"

#define IDT_SEARCHSTART     1       // ID for the search timer
#define SEARCH_TIMER_DELAY  250     // 250 milliseconds timeout

struct
{
    PCWSTR pszPropertyName;
    PCWSTR pszSemanticType;
}
const g_rgGenericProperties[] =
{
    { L"System.Generic.String",          L"System.StructuredQueryType.String" },
    { L"System.Generic.Integer",         L"System.StructuredQueryType.Integer" },
    { L"System.Generic.DateTime",        L"System.StructuredQueryType.DateTime" },
    { L"System.Generic.Boolean",         L"System.StructuredQueryType.Boolean" },
    { L"System.Generic.FloatingPoint",   L"System.StructuredQueryType.FloatingPoint" }
};

class CExplorerBrowserSearchApp : public IServiceProvider, public ICommDlgBrowser2, public IExplorerBrowserEvents
{
public:
    CExplorerBrowserSearchApp() : _cRef(1), _hdlg(NULL), _fPerformRenavigate(FALSE),
        _fSearchStringEmpty(TRUE), _peb(NULL), _pqp(NULL), _hrOleInit(OleInitialize(NULL))
    {
    }

    HRESULT DoModal(HWND hwnd)
    {
        DialogBoxParam(GetModuleHINSTANCE(), MAKEINTRESOURCE(IDD_DIALOG1), hwnd, s_DlgProc, (LPARAM)this);
        return S_OK;
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CExplorerBrowserSearchApp, IServiceProvider),
            QITABENT(CExplorerBrowserSearchApp, ICommDlgBrowser),
            QITABENT(CExplorerBrowserSearchApp, ICommDlgBrowser2),
            QITABENT(CExplorerBrowserSearchApp, IExplorerBrowserEvents),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IServiceProvider
    IFACEMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv)
    {
        *ppv = NULL;

        HRESULT hr = E_NOINTERFACE;
        if (guidService == SID_SExplorerBrowserFrame)
        {
            hr = QueryInterface(riid, ppv);
        }
        return hr;
    }

    // ICommDlgBrowser
    IFACEMETHODIMP OnDefaultCommand(IShellView * /* psv */)
    {
        return S_OK;
    }

    IFACEMETHODIMP OnStateChange(IShellView * /* psv */, ULONG uChange)
    {
        if (uChange == CDBOSC_SELCHANGE)
        {
            _OnSelChange();
        }
        return S_OK;
    }

    IFACEMETHODIMP IncludeObject(IShellView * /* psv */, PCUITEMID_CHILD /* pidl */)
    {
        return S_OK;
    }

    // ICommDlgBrowser2
    IFACEMETHODIMP Notify(IShellView * /* ppshv */ , DWORD /* dwNotifyType */)
    {
        return S_OK;
    }

    IFACEMETHODIMP GetDefaultMenuText(IShellView * /* ppshv */, PWSTR /* pszText */, int /* cchMax */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP GetViewFlags(DWORD *pdwFlags)
    {
        // setting this flag is needed to avoid the poor perf of having
        // ICommDlgBrowser::IncludeObject() for every item when the result
        // set is large.
        *pdwFlags = CDB2GVF_NOINCLUDEITEM;
        return S_OK;
    }

    // IExplorerBrowserEvents
    IFACEMETHODIMP OnViewCreated(IShellView * /* psv */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnNavigationPending(PCIDLIST_ABSOLUTE /* pidlFolder */)
    {
        return S_OK;
    }

    IFACEMETHODIMP OnNavigationComplete(PCIDLIST_ABSOLUTE /* pidlFolder */)
    {
        if (_fPerformRenavigate)
        {
            KillTimer(_hdlg, IDT_SEARCHSTART);
            _OnSearch();
            _fPerformRenavigate = FALSE;
        }
        return S_OK;
    }

    IFACEMETHODIMP OnNavigationFailed(PCIDLIST_ABSOLUTE /* pidlFolder */)
    {
        return E_NOTIMPL;
    }

private:
    ~CExplorerBrowserSearchApp()
    {
        if (SUCCEEDED(_hrOleInit))
        {
            OleUninitialize();
        }
    }

    static INT_PTR CALLBACK s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CExplorerBrowserSearchApp *pssa = reinterpret_cast<CExplorerBrowserSearchApp *>(GetWindowLongPtr(hdlg, DWLP_USER));
        if (uMsg == WM_INITDIALOG)
        {
            pssa = reinterpret_cast<CExplorerBrowserSearchApp *>(lParam);
            pssa->_hdlg = hdlg;
            SetWindowLongPtr(hdlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pssa));
        }
        return pssa ? pssa->_DlgProc(uMsg, wParam, lParam) : 0;
    }

    INT_PTR _DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnInitializeDialog();
    void _OnDestroyDialog();
    void _OnOpenItem();
    void _OnSelChange();
    void _OnSearch();
    void _UpdateSearchIcon();
    HRESULT _GetSampleQueryItem(REFIID riid, void **ppv);
    HRESULT _GetSelectedItem(REFIID riid, void **ppv);
    HRESULT _GetCustomQueryItem(PCWSTR pszQueryString, REFIID riid, void **ppv);

    static LRESULT CALLBACK s_SearchIconProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
    LRESULT _SearchIconProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    long _cRef;
    HWND _hdlg;
    HRESULT _hrOleInit;
    IExplorerBrowser *_peb;
    IQueryParser *_pqp;
    BOOL _fPerformRenavigate;
    BOOL _fSearchStringEmpty;
    DWORD _dwCookie;
    HBITMAP _hbmpClear;
    HBITMAP _hbmpClearHot;
    HBITMAP _hbmpSearch;

    static const ANCHOR c_rgAnchors[7];
    RECT _rgAnchorOffsets[ARRAYSIZE(c_rgAnchors)];
};

const ANCHOR CExplorerBrowserSearchApp::c_rgAnchors[] =
{
  { IDC_OPEN_ITEM,          AF_LEFT | AF_BOTTOM },
  { IDC_NAME,               AF_LEFT | AF_RIGHT | AF_BOTTOM },
  { IDC_EXPLORER_BROWSER,   AF_LEFT | AF_RIGHT | AF_TOP | AF_BOTTOM },
  { IDC_GRIPPER,            AF_RIGHT | AF_BOTTOM },
  { IDC_SEARCHBOXBGND,      AF_RIGHT | AF_TOP },
  { IDC_SEARCHIMG,          AF_RIGHT | AF_TOP },
  { IDC_SEARCHBOX,          AF_RIGHT | AF_TOP },
};

HRESULT CreateQueryParser(IQueryParser **ppqp)
{
    *ppqp = NULL;

    IQueryParserManager *pqpm;
    HRESULT hr = CoCreateInstance(__uuidof(QueryParserManager), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pqpm));
    if (SUCCEEDED(hr))
    {
        IQueryParser *pqp;
        hr = pqpm->CreateLoadedParser(L"SystemIndex", LOCALE_USER_DEFAULT, IID_PPV_ARGS(&pqp));
        if (SUCCEEDED(hr))
        {
            // Initialize the query parser and set default search property types
            hr = pqpm->InitializeOptions(FALSE, TRUE, pqp);
            for (int i = 0; i < ARRAYSIZE(g_rgGenericProperties) && SUCCEEDED(hr); i++)
            {
                PROPVARIANT propvar;
                hr = InitPropVariantFromString(g_rgGenericProperties[i].pszPropertyName, &propvar);
                if (SUCCEEDED(hr))
                {
                    hr = pqp->SetMultiOption(SQMO_DEFAULT_PROPERTY, g_rgGenericProperties[i].pszSemanticType, &propvar);
                    PropVariantClear(&propvar);
                }
            }

            if (SUCCEEDED(hr))
            {
                pqp->QueryInterface(IID_PPV_ARGS(ppqp));
            }
            pqp->Release();
        }
        pqpm->Release();
    }
    return hr;
}

HRESULT ParseStructuredQuery(PCWSTR pszString, IQueryParser *pqp, ICondition **ppc)
{
    *ppc = NULL;

    IQuerySolution *pqs;
    HRESULT hr = pqp->Parse(pszString, NULL, &pqs);
    if (SUCCEEDED(hr))
    {
        ICondition *pc;
        hr = pqs->GetQuery(&pc, NULL);
        if (SUCCEEDED(hr))
        {
            SYSTEMTIME st;
            GetLocalTime(&st);
            hr = pqs->Resolve(pc, SQRO_DONT_SPLIT_WORDS, &st, ppc);
            pc->Release();
        }
        pqs->Release();
    }
    return hr;
}

HRESULT AddStructuredQueryCondition(ISearchFolderItemFactory *psfif, IQueryParser *pqp, PCWSTR pszQuery)
{
    ICondition *pc;
    HRESULT hr = ParseStructuredQuery(pszQuery, pqp, &pc);
    if (SUCCEEDED(hr))
    {
        hr = psfif->SetCondition(pc);
        pc->Release();
    }
    return hr;
}

HRESULT AddCustomCondition(ISearchFolderItemFactory *psfif)
{
    IConditionFactory *pcf;
    HRESULT hr = CoCreateInstance(__uuidof(ConditionFactory), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcf));
    if (SUCCEEDED(hr))
    {
        // pv does not have to be freed
        PROPVARIANT pv;
        pv.vt = VT_LPWSTR;
        pv.pwszVal = L"*.jpg";
        ICondition *pc;
        hr = pcf->MakeLeaf(L"System.FileName", COP_DOSWILDCARDS, NULL, &pv, NULL, NULL, NULL, FALSE, &pc);
        if (SUCCEEDED(hr))
        {
            hr = psfif->SetCondition(pc);
            pc->Release();
        }
        pcf->Release();
    }
    return hr;
}

HRESULT SetScope(ISearchFolderItemFactory *psfif)
{
    // Set scope to the Pictures library.
    IShellItem *psi;
    // you can use SHGetKnownFolderItem instead of SHCreateItemInKnownFolder on Win7 and greater
    HRESULT hr = SHCreateItemInKnownFolder(FOLDERID_PicturesLibrary, 0, NULL, IID_PPV_ARGS(&psi));
    if (SUCCEEDED(hr))
    {
        IShellItemArray *psia;
        hr = SHCreateShellItemArrayFromShellItem(psi, IID_PPV_ARGS(&psia));
        if (SUCCEEDED(hr))
        {
            hr = psfif->SetScope(psia);
            psia->Release();
        }
        psi->Release();
    }
    else
    {
        // If no Pictures library is available (on Vista, for example),
        // set scope to the Pictures and Public Pictures folders
        PIDLIST_ABSOLUTE rgItemIDs[2];
        hr = SHGetKnownFolderIDList(FOLDERID_Pictures, 0, NULL, &rgItemIDs[0]);
        if (SUCCEEDED(hr))
        {
            hr = SHGetKnownFolderIDList(FOLDERID_PublicPictures, 0, NULL, &rgItemIDs[1]);
            if (SUCCEEDED(hr))
            {
                IShellItemArray *psia;
                hr = SHCreateShellItemArrayFromIDLists(ARRAYSIZE(rgItemIDs), rgItemIDs, &psia);
                if (SUCCEEDED(hr))
                {
                    hr = psfif->SetScope(psia);
                    psia->Release();
                }
                ILFree(rgItemIDs[1]);
            }
            ILFree(rgItemIDs[0]);
        }
    }
    return hr;
}

HRESULT CExplorerBrowserSearchApp::_GetSampleQueryItem(REFIID riid, void **ppv)
{
    *ppv = NULL;

    ISearchFolderItemFactory *psfif;
    HRESULT hr = CoCreateInstance(CLSID_SearchFolderItemFactory, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&psfif));
    if (SUCCEEDED(hr))
    {
        hr = psfif->SetDisplayName(L"Sample Query");
        if (SUCCEEDED(hr))
        {
            hr = SetScope(psfif);
            if (SUCCEEDED(hr))
            {
                // Creates a structured query from AQS
                // Other examples of AQS queries:
                //       "System.IsFolder:=FALSE"
                //       "Tags:trees"
                //       "DateTaken:earlier this month"
                // Multiple AQS query terms can be used together to build more complex queries
                //       "DateTaken:earlier this month Type:JPG Size:medium Name:w"
                hr = AddStructuredQueryCondition(psfif, _pqp, L"kind:picture");
                if (SUCCEEDED(hr))
                {
                    // Here is another way to add a custom query condition
                    // hr = AddCustomCondition(psfif);

                    hr = psfif->GetShellItem(riid, ppv);
                }
            }
        }

        psfif->Release();
    }

    return hr;
}

HRESULT CExplorerBrowserSearchApp::_GetCustomQueryItem(PCWSTR pszQueryString, REFIID riid, void **ppv)
{
    *ppv = NULL;

    ISearchFolderItemFactory *psfif;
    HRESULT hr = CoCreateInstance(CLSID_SearchFolderItemFactory, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&psfif));
    if (SUCCEEDED(hr))
    {
        hr = psfif->SetDisplayName(pszQueryString);
        if (SUCCEEDED(hr))
        {
            hr = SetScope(psfif);
            if (SUCCEEDED(hr))
            {
                hr = AddStructuredQueryCondition(psfif, _pqp, pszQueryString);
                if (SUCCEEDED(hr))
                {
                    hr = psfif->GetShellItem(riid, ppv);
                }
            }
        }

        psfif->Release();
    }

    return hr;
}

void CExplorerBrowserSearchApp::_OnInitializeDialog()
{
    InitResizeData(_hdlg, c_rgAnchors, ARRAYSIZE(c_rgAnchors), _rgAnchorOffsets);

    SetDialogIcon(_hdlg, SIID_APPLICATION);

    RECT rc;
    GetWindowRectInClient(GetDlgItem(_hdlg, IDC_EXPLORER_BROWSER), &rc);

    // Load the searchbox icons
    _hbmpClear = LoadBitmap(GetModuleHINSTANCE(), MAKEINTRESOURCE(IDB_CLEARWH));
    _hbmpClearHot = LoadBitmap(GetModuleHINSTANCE(), MAKEINTRESOURCE(IDB_CLEARHOT));
    _hbmpSearch = LoadBitmap(GetModuleHINSTANCE(), MAKEINTRESOURCE(IDB_SEARCHWH));

    // Register the searchbox icon to receive hover and mouseclick events
    SetWindowSubclass(GetDlgItem(_hdlg, IDC_SEARCHIMG), s_SearchIconProc, (UINT_PTR)this, 0);

    if (_hbmpClear && _hbmpClearHot && _hbmpSearch)
    {
        HRESULT hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&_peb));
        if (SUCCEEDED(hr))
        {
            // note, SetSite(NULL) happens in _OnDestroyDialog()
            IUnknown_SetSite(_peb, static_cast<IServiceProvider *>(this));

            FOLDERSETTINGS fs = {};
            fs.ViewMode = FVM_ICON;
            fs.fFlags = FWF_HIDEFILENAMES | FWF_NOSUBFOLDERS | FWF_NOCOLUMNHEADER;

            hr = _peb->Initialize(_hdlg, &rc, &fs);
            if (SUCCEEDED(hr))
            {
                hr = _peb->Advise(this, &_dwCookie);
                if (SUCCEEDED(hr))
                {
                    // Create and save off the query parser for reuse in searching
                    hr = CreateQueryParser(&_pqp);
                    if (SUCCEEDED(hr))
                    {
                        IShellItem *psi;
                        hr = _GetSampleQueryItem(IID_PPV_ARGS(&psi));
                        if (SUCCEEDED(hr))
                        {
                            hr = _peb->BrowseToObject(psi, NULL);
                            psi->Release();
                        }
                    }
                }
            }
        }
        // If we fail to initialize properly, close the dialog
        if (FAILED(hr))
        {
            EndDialog(_hdlg, IDCLOSE);
        }
    }
}

void CExplorerBrowserSearchApp::_OnDestroyDialog()
{
    ClearDialogIcon(_hdlg);

    // Free the searchbox icons
    DeleteObject(_hbmpClear);
    DeleteObject(_hbmpClearHot);
    DeleteObject(_hbmpSearch);

    if (_peb)
    {
        IUnknown_SetSite(_peb, NULL);
        _peb->Unadvise(_dwCookie);
        _peb->Destroy();
        _peb->Release();
        _peb = NULL;
    }

    if (_pqp)
    {
        _pqp->Release();
    }
}

HRESULT CExplorerBrowserSearchApp::_GetSelectedItem(REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_FAIL;

    IFolderView2* pfv;
    if (_peb && SUCCEEDED(_peb->GetCurrentView(IID_PPV_ARGS(&pfv))))
    {
        hr = GetItemFromView(pfv, -1, riid, ppv);
        pfv->Release();
    }
    return hr;
}

void CExplorerBrowserSearchApp::_OnSelChange()
{
    IShellItem *psi;
    HRESULT hr = _GetSelectedItem(IID_PPV_ARGS(&psi));
    if (SUCCEEDED(hr))
    {
        PWSTR pszName;
        hr = psi->GetDisplayName(SIGDN_NORMALDISPLAY, &pszName);
        if (SUCCEEDED(hr))
        {
            SetDlgItemText(_hdlg, IDC_NAME, pszName);
            CoTaskMemFree(pszName);
        }
        psi->Release();
    }
}

void CExplorerBrowserSearchApp::_OnOpenItem()
{
    IShellItem *psi;
    HRESULT hr = _GetSelectedItem(IID_PPV_ARGS(&psi));
    if (SUCCEEDED(hr))
    {
        hr = ShellExecuteItem(_hdlg, NULL, psi);
        psi->Release();
    }
}

void CExplorerBrowserSearchApp::_OnSearch()
{
    HWND hwndSearchBox = GetDlgItem(_hdlg, IDC_SEARCHBOX);
    int cchAlloc = GetWindowTextLength(hwndSearchBox) + 1;
    PWSTR pszSearchString = (PWSTR)CoTaskMemAlloc(sizeof(*pszSearchString) * cchAlloc);
    if (pszSearchString)
    {
        *pszSearchString = 0;
        GetWindowText(hwndSearchBox, pszSearchString, cchAlloc);

        IShellItem *psi;
        HRESULT hr = _GetCustomQueryItem(pszSearchString, IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            hr = _peb->BrowseToObject(psi, 0);
            // The BrowseToObject call is asynchronous, so if it fails because previous navigation
            // is in progress, make sure we re-navigate to process this search query
            if (hr == HRESULT_FROM_WIN32(ERROR_BUSY))
            {
                _fPerformRenavigate = TRUE;
            }
            psi->Release();
        }
        CoTaskMemFree(pszSearchString);
    }
}

void CExplorerBrowserSearchApp::_UpdateSearchIcon()
{
    int cchText = GetWindowTextLength(GetDlgItem(_hdlg, IDC_SEARCHBOX));
    HWND hwndSearchboxImage = GetDlgItem(_hdlg, IDC_SEARCHIMG);

    // If the search box was empty but is no longer, switch icons to "clear"
    if (_fSearchStringEmpty && (cchText != 0))
    {
        _fSearchStringEmpty = FALSE;
        SendMessage(hwndSearchboxImage, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)_hbmpClear);
    }
    else if (!_fSearchStringEmpty && (cchText == 0))
    {
        // When the search box becomes empty again, switch icons to "search"
        _fSearchStringEmpty = TRUE;
        SendMessage(hwndSearchboxImage, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)_hbmpSearch);
    }
}

LRESULT CALLBACK CExplorerBrowserSearchApp::s_SearchIconProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR /* dwRefData */)
{
    CExplorerBrowserSearchApp *pebqa = (CExplorerBrowserSearchApp *) uIdSubclass;
    return pebqa ? pebqa->_SearchIconProc(hWnd, uMsg, wParam, lParam) : DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CExplorerBrowserSearchApp::_SearchIconProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // This function takes care of the hover effects and pressing the "clear" button.
    // Note that the "clear" button is displayed only when there is text in the searchbox.

    switch (uMsg)
    {
        case WM_LBUTTONDOWN:
            if (!_fSearchStringEmpty)
            {
                // If the clear icon is pressed, clear the search box
                SetWindowText(GetDlgItem(_hdlg, IDC_SEARCHBOX), L"");
            }
            break;

        case WM_MOUSELEAVE:
            if (!_fSearchStringEmpty)
            {
                // If the mouse has left the clear icon, show it as white
                SendMessage(GetDlgItem(_hdlg, IDC_SEARCHIMG), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)_hbmpClear);
            }
            break;

        case WM_MOUSEMOVE:
            if (!_fSearchStringEmpty)
            {
                // Request that we be notified when the mouse leaves the icon
                TRACKMOUSEEVENT tme = {};
                tme.cbSize      = sizeof(tme);
                tme.dwFlags     = TME_LEAVE;
                tme.hwndTrack   = GetDlgItem(_hdlg, IDC_SEARCHIMG);
                TrackMouseEvent(&tme);

                // If the mouse is hovering over the clear icon, show it as blue
                SendMessage(GetDlgItem(_hdlg, IDC_SEARCHIMG), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)_hbmpClearHot);
            }
            break;

        case WM_DESTROY:
            RemoveWindowSubclass(GetDlgItem(_hdlg, IDC_SEARCHIMG), s_SearchIconProc, (UINT_PTR)this);
            break;

    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR CExplorerBrowserSearchApp::_DlgProc(UINT uMsg, WPARAM wParam, LPARAM /* lParam */)
{
    INT_PTR iRet = 1;   // default for all handled cases in switch below

    switch (uMsg)
    {
        case WM_INITDIALOG:
            _OnInitializeDialog();
            break;

        case WM_DESTROY:
            // Clean up the search timer
            KillTimer(_hdlg, IDT_SEARCHSTART);
            _OnDestroyDialog();
            break;

        case WM_SIZE:
            OnSize(_hdlg, c_rgAnchors, ARRAYSIZE(c_rgAnchors), _rgAnchorOffsets);
            if (_peb)
            {
                RECT rc;
                GetWindowRectInClient(GetDlgItem(_hdlg, IDC_EXPLORER_BROWSER), &rc);
                _peb->SetRect(NULL, rc);
            }

            RedrawWindow(_hdlg, NULL, NULL, RDW_INVALIDATE);
            break;

        case WM_COMMAND:
            {
                const UINT idCmd = LOWORD(wParam);
                switch (idCmd)
                {
                    case IDOK:
                    case IDCANCEL:
                    case IDCLOSE:
                        return EndDialog(_hdlg, idCmd);

                    case IDC_OPEN_ITEM:
                        _OnOpenItem();
                        break;

                    case IDC_SEARCHBOX:
                        switch (HIWORD(wParam))
                        {
                            case EN_CHANGE:
                                // Update search box icon if necessary
                                _UpdateSearchIcon();
                                // Delay search processing to aggregate keystrokes
                                SetTimer(_hdlg, IDT_SEARCHSTART, SEARCH_TIMER_DELAY, NULL);
                                break;
                        }
                        break;
                }
            }
            break;

        case WM_TIMER:
            {
                // Search timer delay expired, process search
                KillTimer(_hdlg, IDT_SEARCHSTART);
                _OnSearch();
            }
            break;

        default:
            iRet = 0;
            break;
    }
    return iRet;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CExplorerBrowserSearchApp *pdlg = new (std::nothrow) CExplorerBrowserSearchApp();
        if (pdlg)
        {
            pdlg->DoModal(NULL);
            pdlg->Release();
        }
        CoUninitialize();
    }

    return 0;
}

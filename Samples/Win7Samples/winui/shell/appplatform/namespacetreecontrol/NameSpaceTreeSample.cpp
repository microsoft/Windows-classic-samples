// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#define STRICT_TYPED_ITEMIDS
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <new>
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "propsys.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// pass a NULL shell item to cleanup the allocated bitmap

HRESULT SetItemImageImageInStaticControl(HWND hwndStatic, IShellItem *psi)
{
    HBITMAP hbmp = NULL;
    HRESULT hr = S_OK;
    if (psi)
    {
        IShellItemImageFactory *psiif;
        hr = psi->QueryInterface(IID_PPV_ARGS(&psiif));
        if (SUCCEEDED(hr))
        {
            RECT rc;
            GetWindowRect(hwndStatic, &rc);
            UINT dxdy = min(rc.right - rc.left, rc.bottom - rc.top);    // make it square
            SIZE size = { dxdy, dxdy };

            hr = psiif->GetImage(size, SIIGBF_RESIZETOFIT, &hbmp);
            psiif->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        HGDIOBJ hgdiOld = (HGDIOBJ) SendMessage(hwndStatic, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) hbmp);
        if (hgdiOld)
        {
            DeleteObject(hgdiOld);  // if there was an old one clean it up
        }
    }

    return hr;
}

HINSTANCE g_hinst = 0;

class CNameSpaceTreeHost : public IServiceProvider,
                           public INameSpaceTreeControlEvents
{
public:
    CNameSpaceTreeHost() : _cRef(1), _hdlg(NULL), _pnstc(NULL), _pnstc2(NULL), _dwAdviseCookie(0)
    {
    }

    HRESULT DoModal(HWND hwnd)
    {
        DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, s_DlgProc, (LPARAM)this);
        return S_OK;
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CNameSpaceTreeHost, IServiceProvider),
            QITABENT(CNameSpaceTreeHost, INameSpaceTreeControlEvents),
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
            delete this;
        return cRef;
    }

    // IServiceProvider
    IFACEMETHODIMP QueryService(REFGUID /*guidService*/, REFIID /*riid*/, void **ppv)
    {
        HRESULT hr = E_NOINTERFACE;
        *ppv = NULL;
        return hr;
    }

    // INameSpaceTreeControlEvents
    IFACEMETHODIMP OnItemClick(IShellItem * /*psi*/, NSTCEHITTEST /*nstceHitTest*/, NSTCECLICKTYPE /*nstceClickType*/) { return S_FALSE;  }
    IFACEMETHODIMP OnPropertyItemCommit(IShellItem * /*psi*/) { return S_FALSE; }
    IFACEMETHODIMP OnItemStateChanging(IShellItem * /*psi*/, NSTCITEMSTATE /*nstcisMask*/, NSTCITEMSTATE /*nstcisState*/) {  return S_OK;  }
    IFACEMETHODIMP OnItemStateChanged(IShellItem * /*psi*/, NSTCITEMSTATE /*nstcisMask*/, NSTCITEMSTATE /*nstcisState*/) { return S_OK; }
    IFACEMETHODIMP OnSelectionChanged(IShellItemArray *psiaSelection)
    {
        IShellItem *psi;
        HRESULT hr = psiaSelection->GetItemAt(0, &psi);
        if (SUCCEEDED(hr))
        {
            IShellItem2 *psi2;
            hr = psi->QueryInterface(IID_PPV_ARGS(&psi2));
            if (SUCCEEDED(hr))
            {
                _InspectItem(psi2);
                psi2->Release();
            }
            psi->Release();
        }
        return S_OK;
    }
    IFACEMETHODIMP OnKeyboardInput(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)  { return S_FALSE; }
    IFACEMETHODIMP OnBeforeExpand(IShellItem * /*psi*/) { return S_OK; }
    IFACEMETHODIMP OnAfterExpand(IShellItem * /*psi*/) { return S_OK; }
    IFACEMETHODIMP OnBeginLabelEdit(IShellItem * /*psi*/) { return S_OK; }
    IFACEMETHODIMP OnEndLabelEdit(IShellItem * /*psi*/) { return S_OK; }
    IFACEMETHODIMP OnGetToolTip(IShellItem * /*psi*/, LPWSTR /*pszTip*/, int /*cchTip*/) { return E_NOTIMPL; }
    IFACEMETHODIMP OnBeforeItemDelete(IShellItem * /*psi*/) { return E_NOTIMPL; }
    IFACEMETHODIMP OnItemAdded(IShellItem * /*psi*/, BOOL /*fIsRoot*/) { return E_NOTIMPL; }
    IFACEMETHODIMP OnItemDeleted(IShellItem * /*psi*/, BOOL /*fIsRoot*/) { return E_NOTIMPL; }
    IFACEMETHODIMP OnBeforeContextMenu(IShellItem * /*psi*/, REFIID /*riid*/, void **ppv) {  *ppv = NULL; return E_NOTIMPL; }
    IFACEMETHODIMP OnAfterContextMenu(IShellItem * /*psi*/, IContextMenu * /*pcmIn*/, REFIID /*riid*/, void **ppv) { *ppv = NULL; return E_NOTIMPL; }
    IFACEMETHODIMP OnBeforeStateImageChange(IShellItem * /*psi*/) { return S_OK; }
    IFACEMETHODIMP OnGetDefaultIconIndex(IShellItem * /*psi*/, int * /*piDefaultIcon*/, int * /*piOpenIcon*/) { return E_NOTIMPL; }

private:
    ~CNameSpaceTreeHost()
    {
        // COM Member variables have been released in WM_DESTROY.
    }

    static INT_PTR CALLBACK s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CNameSpaceTreeHost *pnsth = reinterpret_cast<CNameSpaceTreeHost*>(GetWindowLongPtr(hdlg, DWLP_USER));
        if (uMsg == WM_INITDIALOG)
        {
            pnsth = reinterpret_cast<CNameSpaceTreeHost*>(lParam);
            pnsth->_hdlg = hdlg;
            SetWindowLongPtr(hdlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pnsth));
        }
        return pnsth ? pnsth->_DlgProc(uMsg, wParam, lParam) : FALSE;
    }

    INT_PTR _DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnInitDlg();
    BOOL _OnCommand(int id);
    void _OnDestroyDlg();
    void _OnOpen();
    void _InspectItem(IShellItem2 *psi);
    void _InitializeRootsAndControls();
    void _SetCheckBoxState(int id, BOOL fChecked, BOOL fEnabled);

    long _cRef;
    HWND _hdlg;
    DWORD _dwAdviseCookie;
    INameSpaceTreeControl  *_pnstc;
    INameSpaceTreeControl2 *_pnstc2;
};

void _SetDialogIcon(HWND hdlg, SHSTOCKICONID siid)
{
    SHSTOCKICONINFO sii = {sizeof(sii)};
    if (SUCCEEDED(SHGetStockIconInfo(siid, SHGFI_ICON | SHGFI_SMALLICON, &sii)))
    {
        SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM) sii.hIcon);
    }
    if (SUCCEEDED(SHGetStockIconInfo(siid, SHGFI_ICON | SHGFI_LARGEICON, &sii)))
    {
        SendMessage(hdlg, WM_SETICON, ICON_BIG, (LPARAM) sii.hIcon);
    }
}

void _ClearDialogIcon(HWND hdlg)
{
    DestroyIcon((HICON)SendMessage(hdlg, WM_GETICON, ICON_SMALL, 0));
    DestroyIcon((HICON)SendMessage(hdlg, WM_GETICON, ICON_BIG, 0));
}

void CNameSpaceTreeHost::_SetCheckBoxState(int id, BOOL fChecked, BOOL fEnabled)
{
    EnableWindow(GetDlgItem(_hdlg, id), fEnabled);
    CheckDlgButton(_hdlg, id, fChecked);
}

void CNameSpaceTreeHost::_InitializeRootsAndControls()
{
    _pnstc->RemoveAllRoots();

    BOOL fEnableStyleChange;
    NSTCSTYLE  nsctsFlags;
    NSTCSTYLE2 nsctsFlags2;
    if (_pnstc2)
    {
        fEnableStyleChange = TRUE;
        _pnstc2->GetControlStyle(NSTCS_HASEXPANDOS | NSTCS_HASLINES | NSTCS_FULLROWSELECT | NSTCS_HORIZONTALSCROLL | NSTCS_RICHTOOLTIP | NSTCS_AUTOHSCROLL | NSTCS_EMPTYTEXT, &nsctsFlags);
        _pnstc2->GetControlStyle2(NSTCS2_DISPLAYPADDING | NSTCS2_DISPLAYPINNEDONLY | NTSCS2_NOSINGLETONAUTOEXPAND, &nsctsFlags2);
    }
    else
    {
        // When running downlevel INameSpaceTreeControl2 may not be available
        // Set styles to defaults.
        fEnableStyleChange = FALSE;
        nsctsFlags  = NSTCS_HASEXPANDOS | NSTCS_ROOTHASEXPANDO | NSTCS_FADEINOUTEXPANDOS | NSTCS_NOINFOTIP | NSTCS_ALLOWJUNCTIONS | NSTCS_SHOWSELECTIONALWAYS | NSTCS_FULLROWSELECT;
        nsctsFlags2 = NSTCS2_DEFAULT;

    }
    _SetCheckBoxState(IDC_EXPANDOS,         nsctsFlags  & NSTCS_HASEXPANDOS,            fEnableStyleChange);
    _SetCheckBoxState(IDC_LINES,            nsctsFlags  & NSTCS_HASLINES,               fEnableStyleChange);
    _SetCheckBoxState(IDC_FULLROWSELECT,    nsctsFlags  & NSTCS_FULLROWSELECT,          fEnableStyleChange);
    _SetCheckBoxState(IDC_HORIZONTALSCROLL, nsctsFlags  & NSTCS_HORIZONTALSCROLL,       fEnableStyleChange);
    _SetCheckBoxState(IDC_PADDING,          nsctsFlags2 & NSTCS2_DISPLAYPADDING,        fEnableStyleChange);
    _SetCheckBoxState(IDC_FILTERPINNED,     nsctsFlags2 & NSTCS2_DISPLAYPINNEDONLY,     fEnableStyleChange);
    _SetCheckBoxState(IDC_AUTOEXPAND,       nsctsFlags2 & NTSCS2_NOSINGLETONAUTOEXPAND, fEnableStyleChange);

    // CLSID_CommonPlacesFolder

    IShellItem *psiFavorites;
    HRESULT hr = SHCreateItemFromParsingName(L"shell:::{323CA680-C24D-4099-B94D-446DD2D7249E}", NULL, IID_PPV_ARGS(&psiFavorites));
    if (SUCCEEDED(hr))
    {
        // Add a visible root
        _pnstc->AppendRoot(psiFavorites, SHCONTF_NONFOLDERS, NSTCRS_VISIBLE | NSTCRS_EXPANDED, NULL); // ignore result

        IShellItem *psiDesktop;
        hr = SHCreateItemInKnownFolder(FOLDERID_Desktop, 0, NULL, IID_PPV_ARGS(&psiDesktop));
        if (SUCCEEDED(hr))
        {
            // Add hidden root
            _pnstc->AppendRoot(psiDesktop, SHCONTF_FOLDERS, NSTCRS_HIDDEN | NSTCRS_EXPANDED, NULL); // ignore result
            psiDesktop->Release();
        }
        psiFavorites->Release();
    }
}

void CNameSpaceTreeHost::_OnInitDlg()
{
    _SetDialogIcon(_hdlg, SIID_APPLICATION);

    HWND hwndStatic = GetDlgItem(_hdlg, IDC_BROWSER);
    if (hwndStatic)
    {
        RECT rc;
        GetWindowRect(hwndStatic, &rc);
        MapWindowRect(HWND_DESKTOP, _hdlg, &rc);

        HRESULT hr = CoCreateInstance(CLSID_NamespaceTreeControl, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&_pnstc));
        if (SUCCEEDED(hr))
        {
            const NSTCSTYLE nsctsFlags = NSTCS_HASEXPANDOS |            // Show expandos
                                         NSTCS_ROOTHASEXPANDO |         // Root nodes have expandos
                                         NSTCS_FADEINOUTEXPANDOS |      // Fade-in-out based on focus
                                         NSTCS_NOINFOTIP |              // Don't show infotips
                                         NSTCS_ALLOWJUNCTIONS |         // Show folders such as zip folders and libraries
                                         NSTCS_SHOWSELECTIONALWAYS |    // Show selection when NSC doesn't have focus
                                         NSTCS_FULLROWSELECT;           // Select full width of item
            hr = _pnstc->Initialize(_hdlg, &rc, nsctsFlags);
            if (SUCCEEDED(hr))
            {
                // New Windows 7 features
                if (SUCCEEDED(_pnstc->QueryInterface(IID_PPV_ARGS(&_pnstc2))))
                {
                    NSTCSTYLE2 nscts2Flags = NSTCS2_DISPLAYPADDING |            // Padding between top-level nodes
                                             NTSCS2_NOSINGLETONAUTOEXPAND |     // Don't auto-expand nodes with a single child node
                                             NSTCS2_INTERRUPTNOTIFICATIONS |    // Register for interrupt notifications on a per-node basis
                                             NSTCS2_DISPLAYPINNEDONLY |         // Filter on pinned property
                                             NTSCS2_NEVERINSERTNONENUMERATED;   // Don't insert items with property SFGAO_NONENUMERATED
                    hr = _pnstc2->SetControlStyle2(nscts2Flags, nscts2Flags);
                }
                if (SUCCEEDED(hr))
                {
                    _pnstc->TreeAdvise(static_cast<INameSpaceTreeControlEvents *>(this), &_dwAdviseCookie);
                    IUnknown_SetSite(_pnstc, static_cast<IServiceProvider *>(this));

                    _InitializeRootsAndControls();
                }
            }
        }
    }
}

BOOL CNameSpaceTreeHost::_OnCommand(int id)
{
    BOOL bRet = TRUE;
    switch (id)
    {
    case IDOK:
    case IDCANCEL:
    case IDC_CANCEL:
        bRet = EndDialog(_hdlg, TRUE);
        break;

    case IDC_EXPANDOS:
        _pnstc2->SetControlStyle(NSTCS_HASEXPANDOS, IsDlgButtonChecked(_hdlg, IDC_EXPANDOS) ? NSTCS_HASEXPANDOS : 0);
        _InitializeRootsAndControls();
        break;

    case IDC_LINES:
        _pnstc2->SetControlStyle(NSTCS_HASLINES, IsDlgButtonChecked(_hdlg, IDC_LINES) ? NSTCS_HASLINES : 0);
        _InitializeRootsAndControls();
        break;

    case IDC_FULLROWSELECT:
        _pnstc2->SetControlStyle(NSTCS_FULLROWSELECT, IsDlgButtonChecked(_hdlg, IDC_FULLROWSELECT) ? NSTCS_FULLROWSELECT : 0);
        _InitializeRootsAndControls();
        break;

    case IDC_PADDING:
        _pnstc2->SetControlStyle2(NSTCS2_DISPLAYPADDING, IsDlgButtonChecked(_hdlg, IDC_PADDING) ? NSTCS2_DISPLAYPADDING : NSTCS2_DEFAULT);
        _InitializeRootsAndControls();
        break;

    case IDC_FILTERPINNED:
        _pnstc2->SetControlStyle2(NSTCS2_DISPLAYPINNEDONLY, IsDlgButtonChecked(_hdlg, IDC_FILTERPINNED) ? NSTCS2_DISPLAYPINNEDONLY : NSTCS2_DEFAULT);
        _InitializeRootsAndControls();
        break;

    case IDC_AUTOEXPAND:
        _pnstc2->SetControlStyle2(NTSCS2_NOSINGLETONAUTOEXPAND, IsDlgButtonChecked(_hdlg, IDC_AUTOEXPAND) ? NTSCS2_NOSINGLETONAUTOEXPAND : NSTCS2_DEFAULT);
        _InitializeRootsAndControls();
        break;

    case IDC_HORIZONTALSCROLL:
        _pnstc2->SetControlStyle(NSTCS_HORIZONTALSCROLL, IsDlgButtonChecked(_hdlg, IDC_HORIZONTALSCROLL) ? NSTCS_HORIZONTALSCROLL : 0);
        _InitializeRootsAndControls();
        break;

    case IDC_EXPLORE:
        _OnOpen();
        break;
    };
    return TRUE;
}

void CNameSpaceTreeHost::_OnDestroyDlg()
{
    _ClearDialogIcon(_hdlg);

    // cleanup the allocated HBITMAP
    SetItemImageImageInStaticControl(GetDlgItem(_hdlg, IDC_IMAGE), NULL);

    if (_pnstc2)
    {
        _pnstc2->Release();
        _pnstc2 = NULL;
    }
    if (_pnstc)
    {
        if (_dwAdviseCookie != -1)
        {
            _pnstc->TreeUnadvise(_dwAdviseCookie);
            _dwAdviseCookie = (DWORD)-1;
        }

        IUnknown_SetSite(_pnstc, NULL);
        _pnstc->Release();
        _pnstc = NULL;
    }
}

void CNameSpaceTreeHost::_OnOpen()
{
    IShellItemArray *psiaItems;
    HRESULT hr = _pnstc->GetSelectedItems(&psiaItems);
    if (SUCCEEDED(hr))
    {
        IShellItem *psi;
        hr = psiaItems->GetItemAt(0, &psi);
        if (SUCCEEDED(hr))
        {
            PIDLIST_ABSOLUTE pidl;
            hr = SHGetIDListFromObject(psi, &pidl);
            if (SUCCEEDED(hr))
            {
                SHELLEXECUTEINFO ei = { sizeof(ei) };
                ei.fMask = SEE_MASK_INVOKEIDLIST;
                ei.hwnd = _hdlg;
                ei.nShow = SW_NORMAL;
                ei.lpIDList = pidl;

                ShellExecuteEx(&ei);
                ILFree(pidl);
            }
            psi->Release();
        }
        psiaItems->Release();
    }
}

const SFGAOF sfgaofAll =
    SFGAO_CANCOPY           |
    SFGAO_CANMOVE           |
    SFGAO_CANLINK           |
    SFGAO_STORAGE           |
    SFGAO_CANRENAME         |
    SFGAO_CANDELETE         |
    SFGAO_HASPROPSHEET      |
    SFGAO_DROPTARGET        |
    SFGAO_ENCRYPTED         |
    SFGAO_ISSLOW            |
    SFGAO_GHOSTED           |
    SFGAO_SHARE             |
    SFGAO_READONLY          |
    SFGAO_HIDDEN            |
//  SFGAO_HASSUBFOLDER      |
    SFGAO_REMOVABLE         |
    SFGAO_COMPRESSED        |
    SFGAO_BROWSABLE         |
    SFGAO_NONENUMERATED     |
    SFGAO_NEWCONTENT        |
    SFGAO_LINK              |
    SFGAO_STREAM            |
    SFGAO_FILESYSTEM        |
    SFGAO_FILESYSANCESTOR   |
    SFGAO_FOLDER            |
    SFGAO_STORAGEANCESTOR;

#define MAP_ENTRY(x) {L#x, x}

HRESULT ShellAttributesToString(SFGAOF sfgaof, PWSTR *ppsz)
{
    *ppsz = NULL;

    static const struct { PCWSTR pszName; SFGAOF sfgaof; } c_rgItemAttributes[] =
    {
        // note, SFGAO_HASSUBFOLDER is too expensive to compute
        // and has been excluded from this list
        MAP_ENTRY(SFGAO_STREAM),
        MAP_ENTRY(SFGAO_FOLDER),
        MAP_ENTRY(SFGAO_FILESYSTEM),
        MAP_ENTRY(SFGAO_FILESYSANCESTOR),
        MAP_ENTRY(SFGAO_STORAGE),
        MAP_ENTRY(SFGAO_STORAGEANCESTOR),
        MAP_ENTRY(SFGAO_LINK),
        MAP_ENTRY(SFGAO_CANCOPY),
        MAP_ENTRY(SFGAO_CANMOVE),
        MAP_ENTRY(SFGAO_CANLINK),
        MAP_ENTRY(SFGAO_CANRENAME),
        MAP_ENTRY(SFGAO_CANDELETE),
        MAP_ENTRY(SFGAO_HASPROPSHEET),
        MAP_ENTRY(SFGAO_DROPTARGET),
        MAP_ENTRY(SFGAO_ENCRYPTED),
        MAP_ENTRY(SFGAO_ISSLOW),
        MAP_ENTRY(SFGAO_GHOSTED),
        MAP_ENTRY(SFGAO_SHARE),
        MAP_ENTRY(SFGAO_READONLY),
        MAP_ENTRY(SFGAO_HIDDEN),
        MAP_ENTRY(SFGAO_REMOVABLE),
        MAP_ENTRY(SFGAO_COMPRESSED),
        MAP_ENTRY(SFGAO_BROWSABLE),
        MAP_ENTRY(SFGAO_NONENUMERATED),
        MAP_ENTRY(SFGAO_NEWCONTENT),
    };

    WCHAR sz[512] = {};
    StringCchPrintf(sz, ARRAYSIZE(sz), L"0x%08X", sfgaof);
    for (int i = 0; i < ARRAYSIZE(c_rgItemAttributes); i++)
    {
        if (c_rgItemAttributes[i].sfgaof & sfgaof)
        {
            if (sz[0])
            {
                StringCchCat(sz, ARRAYSIZE(sz), L", ");
            }

            StringCchCat(sz, ARRAYSIZE(sz), c_rgItemAttributes[i].pszName);
        }
    }
    return SHStrDup(sz, ppsz);
}

void CNameSpaceTreeHost::_InspectItem(IShellItem2 *psi)
{
    SetItemImageImageInStaticControl(GetDlgItem(_hdlg, IDC_IMAGE), psi); // ignore result

    PWSTR psz;
    HRESULT hr = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &psz);
    SetDlgItemText(_hdlg, IDC_NAME, SUCCEEDED(hr) ? psz : L"");
    CoTaskMemFree(psz);

    SetDlgItemText(_hdlg, IDC_PATH, L"");
    IShellLink *psl;
    hr = psi->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARGS(&psl));
    if (SUCCEEDED(hr))
    {
        PIDLIST_ABSOLUTE pidl;
        hr = psl->GetIDList(&pidl);
        if (S_OK == hr)
        {
            hr = SHGetNameFromIDList(pidl, SIGDN_DESKTOPABSOLUTEPARSING, &psz);
            if (SUCCEEDED(hr))
            {
                SetDlgItemText(_hdlg, IDC_PATH, psz);
                CoTaskMemFree(psz);
            }
            CoTaskMemFree(pidl);
        }
        psl->Release();
    }

    PWSTR pszAttributes = NULL;
    SFGAOF sfgaof;
    hr = psi->GetAttributes(sfgaofAll, &sfgaof);
    if (SUCCEEDED(hr))
    {
        ShellAttributesToString(sfgaof, &pszAttributes);
    }

    SetDlgItemText(_hdlg, IDC_ATTRIBUTES, pszAttributes ? pszAttributes : L"");
    CoTaskMemFree(pszAttributes);
}

INT_PTR CNameSpaceTreeHost::_DlgProc(UINT uMsg, WPARAM wParam, LPARAM)
{
    BOOL fRet = TRUE;
    switch (uMsg)
    {
    case WM_INITDIALOG:
        _OnInitDlg();
        break;

    case WM_COMMAND:
        fRet = _OnCommand(LOWORD(wParam));
        break;

    case WM_DESTROY:
        _OnDestroyDlg();
        break;

    default:
        fRet = FALSE;
        break;
    }
    return fRet;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    g_hinst = hInstance;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        OleInitialize(0);   // for drag and drop

        CNameSpaceTreeHost *pdlg = new (std::nothrow) CNameSpaceTreeHost();
        if (pdlg)
        {
            pdlg->DoModal(NULL);
            pdlg->Release();
        }

        OleUninitialize();
        CoUninitialize();
    }

    return 0;
}

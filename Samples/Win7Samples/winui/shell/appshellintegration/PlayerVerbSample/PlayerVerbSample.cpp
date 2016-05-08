// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#define STRICT_TYPED_ITEMIDS

#include <windows.h>
#include <windowsx.h>
#include <strsafe.h>
#include "ShellHelpers.h"
#include "DragDropHelpers.h"
#include "RegisterExtension.h"
#include "ApplicationVerb.h"
#include "ResizeableDialog.h"
#include <new>  // std::nothrow
#include "resource.h"

WCHAR const c_szApplicationName[] = L"Player Verb Sample";

// request an IShellItem or related interface
HRESULT GetSelectedItemFromSite(IUnknown *punkSite, REFIID riid, void **ppv)
{
    *ppv = NULL;
    IFolderView2 *pfv;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_SFolderView, IID_PPV_ARGS(&pfv));
    if (SUCCEEDED(hr))
    {
        hr = GetItemFromView(pfv, -1, riid, ppv);
        pfv->Release();
    }
    return hr;
}

// Return IShellItemArray object from the selection in the view, accessed via the site pointer
HRESULT GetSelectionFromSite(IUnknown *punkSite, BOOL fNoneImpliesFolder, IShellItemArray **ppsia)
{
    *ppsia = NULL;
    IFolderView2 *pfv;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_SFolderView, IID_PPV_ARGS(&pfv));
    if (SUCCEEDED(hr))
    {
        hr = pfv->GetSelection(fNoneImpliesFolder, ppsia);
        pfv->Release();
    }
    return hr;
}

// CFileOpenBasketPicker is a helper class to use the file dialog in a modal way such that
// users can easily pick multiple files. it does this by overriding the normal "Open" button
// with an "Add" button that passes the selection back to this app
//
// one case this class does not support is selecting folders this way. this has
// the issue of the "Open" button being overloaded for "nav into the folder" and "add the folder"
// one way to deal with this is to add a new button, "Add Folder", the
// PickFolderAndItem samples demonstrate this

// to use this provide an application object the implements the method "AddItems"
// as a template parameter passed to the constructor
//
//    CFileOpenBasketPicker<CPlayerApplication> fileOpenBasketPicker(this);
//    fileOpenBasketPicker.PickItems(_hdlg, L"Player Sample");

DWORD const c_idAdd     = 601;
DWORD const c_idDone    = 602;

template <class TApplication> class CFileOpenBasketPicker : public IFileDialogEvents, public IFileDialogControlEvents
{
public:
    // note FOS_PICKFOLDERS implies picking files and folders
    CFileOpenBasketPicker(TApplication *pApp, FILEOPENDIALOGOPTIONS options) : _pApp(pApp), _options(options)
    {
    }

    // psiInitialFolder and pszTitle are optional
    HRESULT PickItems(HWND hwnd, IShellItem *psiInitialFolder, PCWSTR pszTitle)
    {
        IFileOpenDialog *pfd;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
        if (SUCCEEDED(hr))
        {
            DWORD dwCookie;
            hr = pfd->Advise(this, &dwCookie);
            if (SUCCEEDED(hr))
            {
                DWORD dwOptions;
                if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
                {
                    // don't put the file dialog in FOS_PICKFOLDERS mode
                    // as we support this implictly in the basket mode
                    pfd->SetOptions(dwOptions | (_options & ~FOS_PICKFOLDERS));
                }

                // this flag lets the client pick folder mode that uses the
                // Add/Open/Done button model
                if (FOS_PICKFOLDERS & _options)
                {
                    IFileDialogCustomize *pfdc;
                    if (SUCCEEDED(pfd->QueryInterface(&pfdc)))
                    {
                        pfdc->AddPushButton(c_idAdd, L"Add current folder   ");
                        pfdc->Release();
                    }
                }

                if (psiInitialFolder)
                {
                    pfd->AddPlace(psiInitialFolder, FDAP_TOP);
                    pfd->SetDefaultFolder(psiInitialFolder);
                }

                // Win7 has a way to override the "Cancel" button text, use that
                // here when on Win7
                IFileDialog2 *pfd2;
                if (SUCCEEDED(pfd->QueryInterface(&pfd2)))
                {
                    pfd2->SetCancelButtonLabel(L"Done");
                    pfd2->Release();
                }
                else
                {
                    IFileDialogCustomize *pfdc;
                    if (SUCCEEDED(pfd->QueryInterface(&pfdc)))
                    {
                        pfdc->AddPushButton(c_idDone, L"Done");
                        pfdc->Release();
                    }
                }

                if (pszTitle)
                {
                    pfd->SetTitle(pszTitle);
                }

                // the items selected are passed back via OnFileOk()
                // so we don't process the results of the dialog
                hr = pfd->Show(hwnd);

                pfd->Unadvise(dwCookie);
            }
            pfd->Release();
        }
        return hr;
    }

private:

    // Keep the COM identity of this object private so it does not get in the way
    // of the auto-complete of the methods we expect client to use.

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CFileOpenBasketPicker, IFileDialogEvents),
            QITABENT(CFileOpenBasketPicker, IFileDialogControlEvents),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef() { return 3; }
    IFACEMETHODIMP_(ULONG) Release() { return 2; }

    // IFileDialogEvents
    IFACEMETHODIMP OnFileOk(IFileDialog *pfd)
    {
        IShellItemArray *psia;
        HRESULT hr = GetSelectionFromSite(pfd, FOS_PICKFOLDERS & _options, &psia);
        if (SUCCEEDED(hr))
        {
            _DoAdd(psia);
            psia->Release();
        }
        return S_FALSE; // S_FALSE keeps the dialog up, return S_OK to allows it to dismiss
    }

    IFACEMETHODIMP OnFolderChanging(IFileDialog * /*pfd*/, IShellItem * /*psi*/)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnFolderChange(IFileDialog * /*pfd*/)
    {
        // this event happens after a navigation is complete
        // if selecting folders is supported update the
        // text of the Open/Add button here based on the selection
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnSelectionChange(IFileDialog *pfd)
    {
        if (FOS_PICKFOLDERS & _options)
        {
            // here is the design for the text of the "Add" button
            // Single select item      "Add file"
            // Single select folder    "Add folder"
            // Multiselect             "Add items"
            // Null select             "Add current folder"

            IFileDialogCustomize *pfdc;
            if (SUCCEEDED(pfd->QueryInterface(&pfdc)))
            {
                // GetSelectionFromSite() fails on no selection, that => the current folder
                // so default to that value
                PCWSTR pszLabel = L"Add current folder";

                IShellItemArray *psia;
                if (SUCCEEDED(GetSelectionFromSite(pfd, FALSE, &psia)))
                {
                    DWORD count;
                    if (SUCCEEDED(psia->GetCount(&count)))
                    {
                        if (count == 1)
                        {
                            IShellItem *psi;
                            if (SUCCEEDED(psia->GetItemAt(0, &psi)))
                            {
                                SFGAOF attributes;
                                if (S_OK == psi->GetAttributes(SFGAO_FOLDER, &attributes))
                                {
                                    pszLabel = L"Add folder";
                                }
                                else
                                {
                                    pszLabel = L"Add file";
                                }
                                psi->Release();
                            }
                        }
                        else if (count > 1)
                        {
                            pszLabel = L"Add items";
                        }
                    }
                    psia->Release();
                }
                pfdc->SetControlLabel(c_idAdd, pszLabel);
                pfdc->Release();
            }
        }
        else
        {
            IFileDialogCustomize *pfdc;
            HRESULT hr = pfd->QueryInterface(&pfdc);
            if (SUCCEEDED(hr))
            {
                // update the text of the Open/Add button here based on the selection

                // we can't use pfd->GetCurrentSelection() as that does not provide
                // access to the true selection state of t he items in the view
                IShellItem *psi;
                hr = GetSelectedItemFromSite(pfd, IID_PPV_ARGS(&psi));
                if (SUCCEEDED(hr))
                {
                    SFGAOF attr;
                    hr = psi->GetAttributes(SFGAO_FOLDER | SFGAO_STREAM, &attr);
                    if (SUCCEEDED(hr) && (SFGAO_FOLDER == attr))
                    {
                        pfd->SetOkButtonLabel(L"Open");
                    }
                    else
                    {
                        // non folder selected
                        pfd->SetOkButtonLabel(L"Add");
                    }
                    psi->Release();
                }
                pfdc->Release();
            }
        }

        return S_OK;
    }

    IFACEMETHODIMP OnShareViolation(IFileDialog * /*pfd*/, IShellItem * /*psi*/, FDE_SHAREVIOLATION_RESPONSE * /*pResponse*/) { return E_NOTIMPL; }
    IFACEMETHODIMP OnTypeChange(IFileDialog * /*pfd*/) { return E_NOTIMPL; }
    IFACEMETHODIMP OnOverwrite(IFileDialog * /*pfd*/, IShellItem * /*psi*/, FDE_OVERWRITE_RESPONSE * /*pResponse*/) { return E_NOTIMPL;}

    // IFileDialogControlEvents
    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize * /*pfdc*/, DWORD /*dwIDCtl*/, DWORD /*dwIDItem*/)  { return E_NOTIMPL; }

    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *pfdc, DWORD dwIDCtl)
    {
        switch (dwIDCtl)
        {
        case c_idAdd:
            {
                // instead of using IFileDialog::GetCurrentSelection() we need to get the
                // selection from the view as that handles the "none implies folder" case
                IShellItemArray *psia;
                HRESULT hr = GetSelectionFromSite(pfdc, TRUE, &psia);
                if (SUCCEEDED(hr))
                {
                    _DoAdd(psia);
                    psia->Release();
                }
            }
            break;

        case c_idDone:
            IFileDialog *pfd;
            if (SUCCEEDED(pfdc->QueryInterface(&pfd)))
            {
                pfd->Close(S_OK);
                pfd->Release();
            }
            break;

        default:
            break;
        }

        return S_OK;
    }

    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize * /*pfdc*/, DWORD /*dwIDCtl*/, BOOL /*bChecked*/) { return E_NOTIMPL; }
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize * /*pfdc*/, DWORD /*dwIDCtl*/) { return E_NOTIMPL; }

    void _DoAdd(IShellItemArray *psia)
    {
        // Callers taht specify FOS_ALLOWMULTISELECT must provide TApplication::AddItems()
        // to take the array of items. apps that do not must provide TApplication::AddItem()
        // that takes the single selection.

        __if_exists(TApplication::AddItems)
        {
            _pApp->AddItems(psia);  // callback to the app with the items
        }

        __if_not_exists(TApplication::AddItems)
        {
            __if_exists(TApplication::AddItem)
            {
                IShellItem2 *psi;
                if (SUCCEEDED(GetItemAt(psia, 0, IID_PPV_ARGS(&psi))))
                {
                    _pApp->AddItem(psi);    // callback to the app with only one item
                    psi->Release();
                }
            }
        }
    }

    // This method is disabled.
    // User-defined assignment operator is necessary for /W4 /WX since a default
    // one cannot be created by the compiler because this class contains const members.
    CFileOpenBasketPicker & operator=(const CFileOpenBasketPicker &);

    FILEOPENDIALOGOPTIONS const _options;
    TApplication *_pApp;
};

// class to wrap CLSID_ExplorerBrowser to make it easier to program
class CExplorerBrowser
{
public:
    CExplorerBrowser() : _hwndHost(NULL), _peb(NULL)
    {
    }

    ~CExplorerBrowser()
    {
        SafeRelease(&_peb);
    }

    // hwndHost is used to position the explorer browser control. resize it and
    // call OnSize() to resize the browser window. the parent of this window
    // is used to init the explorer browser
    HRESULT Initialize(HWND hwndHost, IUnknown *punkSite)
    {
        _hwndHost = hwndHost;

        HRESULT hr = CoCreateInstance(CLSID_ExplorerBrowser, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&_peb));
        if (SUCCEEDED(hr))
        {
            IUnknown_SetSite(_peb, punkSite);

            FOLDERSETTINGS fs = {};
            fs.ViewMode = static_cast<UINT>(FVM_AUTO);
            fs.fFlags = FWF_SHOWSELALWAYS;

            RECT rc;
            GetWindowRectInClient(_hwndHost, &rc);

            hr = _peb->Initialize(GetParent(_hwndHost), &rc, &fs);
            if (SUCCEEDED(hr))
            {
                // init the exporer browser so that we can use the results folder
                // as the data source. that enables us to program the contents of
                // the view via IResultsFolder

                hr = _peb->FillFromObject(NULL, EBF_NODROPTARGET);
            }
        }
        return hr;
    }

    void Destroy()
    {
        if (_peb)
        {
            IUnknown_SetSite(_peb, NULL);
            _peb->Destroy();
            _peb->Release();
            _peb = NULL;
        }
    }

    void OnSize() const
    {
        if (_peb)
        {
            RECT rc;
            GetWindowRectInClient(_hwndHost, &rc);
            _peb->SetRect(NULL, rc);
        }
    }

    HRESULT GetView(REFIID riid, void **ppv) const
    {
        *ppv = NULL;
        return _peb ? _peb->GetCurrentView(riid, ppv) : E_FAIL;
    }

    HRESULT GetFolder(REFIID riid, void **ppv) const
    {
        *ppv = NULL;
        IFolderView *pfv;
        HRESULT hr = GetView(IID_PPV_ARGS(&pfv));
        if (SUCCEEDED(hr))
        {
            hr = pfv->GetFolder(riid, ppv);
            pfv->Release();
        }
        return hr;
    }

    HRESULT GetItemFromView(int iItem, REFIID riid, void **ppv) const
    {
        *ppv = NULL;
        IFolderView2* pfv;
        HRESULT hr = GetView(IID_PPV_ARGS(&pfv));
        if (SUCCEEDED(hr))
        {
            hr = ::GetItemFromView(pfv, iItem, riid, ppv);
            pfv->Release();
        }
        return hr;
    }

    int GetItemCount() const
    {
        int cItems = 0;
        IFolderView2* pfv;
        HRESULT hr = GetView(IID_PPV_ARGS(&pfv));
        if (SUCCEEDED(hr))
        {
            hr = pfv->ItemCount(SVGIO_ALLVIEW, &cItems);
            pfv->Release();
        }
        return cItems;
    }

private:
    IExplorerBrowser *_peb;
    HWND _hwndHost;
};


__inline DWORD GetPerfTime()
{
    static __int64 freq;
    __int64 curtime;

    if (!freq)
        QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    QueryPerformanceCounter((LARGE_INTEGER *)&curtime);
    return (DWORD)((curtime * 1000) / freq);
}


class CPlayerApplication : public CDragDropHelper, public ICommDlgBrowser2, public IServiceProvider
{
public:
    CPlayerApplication() : _cRef(1), _hdlg(NULL), _pViewToTrack(NULL)
    {
        _verbPlay.SetApplication(this);
        _verbAddToQueue.SetApplication(this);
    }

    HRESULT ClearQueue() const
    {
        IResultsFolder *prf;
        HRESULT hr = _explorerBrowser.GetFolder(IID_PPV_ARGS(&prf));
        if (SUCCEEDED(hr))
        {
            prf->RemoveAll();
            prf->Release();
        }
        return S_OK;
    }

    HRESULT AddToQueue(IShellItem *psi) const
    {
        IResultsFolder *prf;
        HRESULT hr = _explorerBrowser.GetFolder(IID_PPV_ARGS(&prf));
        if (SUCCEEDED(hr))
        {
            hr = prf->AddItem(psi);
            prf->Release();
        }
        return hr;
    }

    HRESULT AddItems(IShellItemArray *psia) const
    {
        IResultsFolder *prf;
        HRESULT hr = _explorerBrowser.GetFolder(IID_PPV_ARGS(&prf));
        if (SUCCEEDED(hr))
        {
            DWORD iCount;
            hr = psia->GetCount(&iCount);
            for (DWORD i = 0; SUCCEEDED(hr) && (i < iCount); i++)
            {
                IShellItem *psi;
                hr = GetItemAt(psia, i, IID_PPV_ARGS(&psi));
                if (SUCCEEDED(hr))
                {
                    prf->AddItem(psi);
                    psi->Release();
                }
            }

            prf->Release();
        }
        return S_OK;
    }

    void ActivateWithKeyboardOverride(DWORD grfKeyState) const
    {
        // key modifiers leave the player in the background
        if (!((MK_CONTROL | MK_SHIFT) & grfKeyState))
        {
            SetForegroundWindow(_hdlg);
        }
    }

    HRESULT DoModal(HWND hwnd)
    {
        DialogBoxParam(GetModuleHINSTANCE(), MAKEINTRESOURCE(IDD_DIALOG), hwnd, s_DlgProc, (LPARAM)this);
        return S_OK;
    }

    // CApplicationVerb provides us access to the site via this callback
    // when the verb is given the site, use that to discover the view
    // that invoked teh verb so it can be tracked

    void SetSite(IUnknown *punkSite)
    {
        if (punkSite && !_pViewToTrack)
        {
            // capture the folder view on first play
            IUnknown_QueryService(punkSite, SID_SFolderView, IID_PPV_ARGS(&_pViewToTrack));
        }
    }

    void ReportResults(PCWSTR psz)
    {
        SetDlgItemText(_hdlg, IDC_STATUS, psz);
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static QITAB const qit[] =
        {
            QITABENT(CPlayerApplication, IDropTarget),
            QITABENT(CPlayerApplication, IServiceProvider),
            QITABENT(CPlayerApplication, ICommDlgBrowser),
            QITABENT(CPlayerApplication, ICommDlgBrowser2),
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
        _OpenSelectedItem();
        return S_OK;
    }

    IFACEMETHODIMP OnStateChange(IShellView * /* psv */, ULONG uChange)
    {
        if (uChange == CDBOSC_SELCHANGE && _pViewToTrack)
        {
            IShellItem *psi;
            HRESULT hr = _explorerBrowser.GetItemFromView(-1, IID_PPV_ARGS(&psi));
            if (SUCCEEDED(hr))
            {
                IParentAndItem *ppai;
                hr = psi->QueryInterface(&ppai);
                if (SUCCEEDED(hr))
                {
                    PITEMID_CHILD pidlChild;
                    hr = ppai->GetParentAndItem(NULL, NULL, &pidlChild);
                    if (SUCCEEDED(hr))
                    {
                        hr = _pViewToTrack->SelectItem(pidlChild, SVSI_SELECT | SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS);
                        if (FAILED(hr))
                        {
                            SafeRelease(&_pViewToTrack);
                        }
                        CoTaskMemFree(pidlChild);
                    }
                    ppai->Release();
                }
                psi->Release();
            }
        }
        return S_OK;
    }

    IFACEMETHODIMP IncludeObject(IShellView * /*psv*/, PCUITEMID_CHILD /*pidl*/)
    {
        return S_OK;
    }

    // ICommDlgBrowser2
    IFACEMETHODIMP Notify(IShellView * /*ppshv*/, DWORD /*dwNotifyType*/)
    {
        return S_OK;
    }

    IFACEMETHODIMP GetDefaultMenuText(IShellView * /*ppshv*/, PWSTR /*pszText*/, int /*cchMax*/)
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

private:
    ~CPlayerApplication()
    {
        SafeRelease(&_pViewToTrack);
    }

    // async verb example
    class __declspec(uuid("4a4f70f8-0f4d-46dc-a4ee-3611308d885f"))
        CPlayVerb : public CApplicationVerb<CPlayerApplication, CPlayVerb>
    {
        DWORD _startTimeMS;
        DWORD _countItems;

    public:
        CPlayVerb() : CApplicationVerb(AVF_ASYNC | AVF_ONE_IMPLIES_ALL) {}

        void StartVerb()
        {
            _countItems = 0;
            _startTimeMS = GetPerfTime();

            CPlayerApplication *pApp = GetApp();
            if (pApp)
            {
                pApp->ActivateWithKeyboardOverride(GetKeyState());
                pApp->ClearQueue();        // clear the queue first
            }
        }

        void OnItem(IShellItem *psi)
        {
            _countItems++;

            CPlayerApplication *pApp = GetApp();
            if (pApp)
            {
                pApp->AddToQueue(psi);
            }
        }

        void EndVerb()
        {
            CPlayerApplication *pApp = GetApp();
            if (pApp)
            {
                DWORD const totalTimeMS = GetPerfTime() - _startTimeMS;
                DWORD const itemPerSecond = _countItems * 1000 / totalTimeMS;

                WCHAR sz[128];
                StringCchPrintf(sz, ARRAYSIZE(sz), L"%d.%d seconds, %d items/second", totalTimeMS / 1000, totalTimeMS % 1000, itemPerSecond);
                pApp->ReportResults(sz);
            }
        }
    };

    // async verb example
    class __declspec(uuid("2b987da6-2a4b-4ab2-b11a-bd5bff6b3759"))
        CAddToQueueVerb : public CApplicationVerb<CPlayerApplication, CAddToQueueVerb>
    {
        DWORD _startTimeMS;
        DWORD _countItems;

    public:
        CAddToQueueVerb() : CApplicationVerb(AVF_ASYNC) {}

        void StartVerb()
        {
            _countItems = 0;
            _startTimeMS = GetPerfTime();

            CPlayerApplication *pApp = GetApp();
            if (pApp)
            {
                pApp->ActivateWithKeyboardOverride(GetKeyState());
            }
        }

        void OnItem(IShellItem *psi)
        {
            _countItems++;
            CPlayerApplication *pApp = GetApp();
            if (pApp)
            {
                pApp->AddToQueue(psi);
            }
        }

        void EndVerb()
        {
            CPlayerApplication *pApp = GetApp();
            if (pApp)
            {
                DWORD const totalTimeMS = GetPerfTime() - _startTimeMS;
                DWORD const itemPerSecond = _countItems * 1000 / totalTimeMS;

                WCHAR sz[128];
                StringCchPrintf(sz, ARRAYSIZE(sz), L"%d.%d seconds, %d items/second", totalTimeMS / 1000, totalTimeMS % 1000, itemPerSecond);
                pApp->ReportResults(sz);
            }
        }
    };

    // CDragDropHelper override
    virtual HRESULT OnDrop(IShellItemArray *psia, DWORD /*grfKeyState*/)
    {
        // invoke the verb implementation for drag and drop, this gets all of the
        // same features including container support
        _verbAddToQueue.SetSelection(psia);
        _verbAddToQueue.Execute();
        return S_OK;
    }

    void _AddItems()
    {
        CFileOpenBasketPicker<CPlayerApplication> fileOpenBasketPicker(this, FOS_PICKFOLDERS | FOS_ALLOWMULTISELECT | FOS_ALLNONSTORAGEITEMS);
        fileOpenBasketPicker.PickItems(_hdlg, NULL, c_szApplicationName);
    }

    void _OpenSelectedItem()
    {
        IShellItem *psi;
        HRESULT hr = _explorerBrowser.GetItemFromView(-1, IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            hr = ShellExecuteItem(_hdlg, NULL, psi);
            psi->Release();
        }
    }

    static INT_PTR CALLBACK s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CPlayerApplication *pcd = (CPlayerApplication*) (LONG_PTR) GetWindowLongPtr(hdlg, DWLP_USER);
        if (uMsg == WM_INITDIALOG)
        {
            pcd = (CPlayerApplication *) lParam;
            pcd->_hdlg = hdlg;
            SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pcd);
        }
        return pcd ? pcd->_DlgProc(uMsg, wParam, lParam) : FALSE;
    }

    INT_PTR _DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void _OnInitDlg();
    void _OnDestroyDlg();

    long _cRef;
    HWND _hdlg;
    CExplorerBrowser _explorerBrowser;
    IShellView *_pViewToTrack;

    CPlayVerb _verbPlay;
    CAddToQueueVerb _verbAddToQueue;

    static ANCHOR const c_rgAnchors[6];
    RECT _rgAnchorOffsets[ARRAYSIZE(c_rgAnchors)];
};

ANCHOR const CPlayerApplication::c_rgAnchors[] =
{
  { IDC_EXPLORER_BROWSER,   AF_LEFT | AF_RIGHT | AF_TOP | AF_BOTTOM },
  { IDC_OPEN,               AF_LEFT | AF_BOTTOM },
  { IDC_CLEAR,              AF_LEFT | AF_BOTTOM },
  { IDC_ADD,                AF_LEFT | AF_BOTTOM },
  { IDC_STATUS,             AF_LEFT | AF_BOTTOM },
  { IDC_GRIPPER,            AF_RIGHT | AF_BOTTOM },
};

void CPlayerApplication::_OnInitDlg()
{
    InitResizeData(_hdlg, c_rgAnchors, ARRAYSIZE(c_rgAnchors), _rgAnchorOffsets);

    InitializeDragDropHelper(_hdlg, DROPIMAGE_COPY, L"Add %1 to Queue");

    _verbPlay.Register();
    _verbAddToQueue.Register();

    SetDialogIcon(_hdlg, SIID_APPLICATION);

    HRESULT hr = _explorerBrowser.Initialize(GetDlgItem(_hdlg, IDC_EXPLORER_BROWSER), static_cast<IDropTarget *>(this));
    if (SUCCEEDED(hr))
    {
        IColumnManager *pcm;
        hr = _explorerBrowser.GetView(IID_PPV_ARGS(&pcm));
        if (SUCCEEDED(hr))
        {
            PROPERTYKEY const rgkeys[] = { PKEY_ItemNameDisplay, PKEY_Music_TrackNumber, PKEY_Title, PKEY_Music_AlbumArtist, PKEY_Music_AlbumTitle, PKEY_ItemPathDisplayNarrow };
            hr = pcm->SetColumns(rgkeys, ARRAYSIZE(rgkeys));
            pcm->Release();
        }
    }
}

void CPlayerApplication::_OnDestroyDlg()
{
    TerminateDragDropHelper();
    _verbPlay.UnRegister();
    _verbAddToQueue.UnRegister();
    _explorerBrowser.Destroy();
}

INT_PTR CPlayerApplication::_DlgProc(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        _OnInitDlg();
        break;

    case WM_SIZE:
        OnSize(_hdlg, c_rgAnchors, ARRAYSIZE(c_rgAnchors), _rgAnchorOffsets);
        _explorerBrowser.OnSize();
        break;

    case WM_COMMAND:
        {
            int const idCmd = GET_WM_COMMAND_ID(wParam, lParam);
            switch (idCmd)
            {
            case IDOK:
            case IDCANCEL:
                return EndDialog(_hdlg, idCmd);

            case IDC_OPEN:
                _OpenSelectedItem();
                break;

            case IDC_CLEAR:
                ClearQueue();
                break;

            case IDC_ADD:
                _AddItems();
                break;
            }
        }
        break;

    case WM_DESTROY:
        _OnDestroyDlg();
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

WCHAR const c_szRegisterCmdLineOption[] = L"-register";
WCHAR const c_szUnRegisterCmdLineOption[] = L"-unRegister";
WCHAR const c_szNoRegisterCmdLineOption[] = L"-noRegister";
WCHAR const c_szEmbeddingCmdLineOption[] = L"-Embedding";   // COM passes this when activating local servers

WCHAR const c_szPlayVerb[] = L"PlayerSample.Play";
WCHAR const c_szPlayVerbName[] = L"Play (Sample App)";
WCHAR const c_szAddToQueueVerb[] = L"PlayerSample.AddToQueue";
WCHAR const c_szAddToQUeueVerbName[] = L"Add to Queue (Sample App)";

PCWSTR const rgAssociationElementsMusic[] =
{
    L"SystemFileAssociations\\Directory.Audio", // music folders
    L"Stack.System.Music",                      // music stacks anywhere
    L"Stack.Audio",                             // stacks in music library
    L"SystemFileAssociations\\Audio",           // music items
};

PCWSTR const rgAssociationElementsPhotos[] =
{
    L"SystemFileAssociations\\Directory.Image", // picture folders
    L"Stack.System.Photo",                      // photo stacks anywhere
    L"Stack.Image",                             // stacks in photo library
    L"SystemFileAssociations\\Image",           // photo items
};

HRESULT RegisterApp()
{
    CRegisterExtension rePlay(__uuidof(CPlayerApplication::CPlayVerb));
    CRegisterExtension reAddToQueue(__uuidof(CPlayerApplication::CAddToQueueVerb));

    HRESULT hr = rePlay.RegisterPlayerVerbs(rgAssociationElementsPhotos, ARRAYSIZE(rgAssociationElementsPhotos),
    // HRESULT hr = rePlay.RegisterPlayerVerbs(rgAssociationElementsMusic, ARRAYSIZE(rgAssociationElementsMusic),
        c_szPlayVerb, c_szPlayVerbName);
    if (SUCCEEDED(hr))
    {
        hr = reAddToQueue.RegisterPlayerVerbs(rgAssociationElementsPhotos, ARRAYSIZE(rgAssociationElementsPhotos),
        // hr = reAddToQueue.RegisterPlayerVerbs(rgAssociationElementsMusic, ARRAYSIZE(rgAssociationElementsMusic),
            c_szAddToQueueVerb, c_szAddToQUeueVerbName);
    }
    return hr;
}

HRESULT UnRegisterApp()
{
    // note these default to per user registration
    CRegisterExtension rePlay(__uuidof(CPlayerApplication::CPlayVerb));
    CRegisterExtension reAddToQueue(__uuidof(CPlayerApplication::CAddToQueueVerb));

    HRESULT hr; // best effort, don't track failure
    hr = rePlay.UnRegisterVerbs(rgAssociationElementsPhotos, ARRAYSIZE(rgAssociationElementsPhotos), c_szPlayVerb);
    hr = rePlay.UnRegisterVerbs(rgAssociationElementsMusic, ARRAYSIZE(rgAssociationElementsMusic), c_szPlayVerb);
    hr = reAddToQueue.UnRegisterVerbs(rgAssociationElementsPhotos, ARRAYSIZE(rgAssociationElementsPhotos), c_szAddToQueueVerb);
    hr = reAddToQueue.UnRegisterVerbs(rgAssociationElementsMusic, ARRAYSIZE(rgAssociationElementsMusic), c_szAddToQueueVerb);
    return hr;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR pszCmdLine, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        DisableComExceptionHandling();

        if (StrStrI(pszCmdLine, c_szRegisterCmdLineOption))
        {
            hr = RegisterApp();
        }
        else if (StrStrI(pszCmdLine, c_szUnRegisterCmdLineOption))
        {
            hr = UnRegisterApp();
        }
        else
        {
            if (!StrStrI(pszCmdLine, c_szEmbeddingCmdLineOption) &&
                !StrStrI(pszCmdLine, c_szNoRegisterCmdLineOption))
            {
                TASKDIALOGCONFIG taskDialogParams = { sizeof(taskDialogParams) };
                taskDialogParams.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION;

                TASKDIALOG_BUTTON buttons[] =
                {
                    { 100, L"Register Verbs" },
                    { 101, L"UnRegister Verbs" },
                    { 102, L"Run the sample" },
                };

                taskDialogParams.pButtons = buttons;
                taskDialogParams.cButtons = ARRAYSIZE(buttons);

                taskDialogParams.pszMainInstruction =
                    L"Player App SDK Sample. This sample demonstrates how to impelment verbs and drag and drop " \
                    L"including verbs on shell containers (folders, stacks, librarys) and support for OpenSearch items";
                taskDialogParams.pszWindowTitle = c_szApplicationName;

                int selectedId;
                hr = TaskDialogIndirect(&taskDialogParams, &selectedId, NULL, NULL);
                if (SUCCEEDED(hr))
                {
                    if (100 == selectedId)
                    {
                        hr = RegisterApp();
                    }
                    else if (101 == selectedId)
                    {
                        hr = UnRegisterApp();
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    }
                }
            }

            CPlayerApplication *pdlg = new (std::nothrow) CPlayerApplication();
            if (pdlg)
            {
                pdlg->DoModal(NULL);
                pdlg->Release();
            }
        }

        CoUninitialize();
    }

    return 0;
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// This application demonstrates how to use the shell change notification system for
// watching a location in the shell namespace for changes.
// This can be used to keep track of the current state of a library since that
// is a shell container that participates in this system.

#define STRICT_TYPED_ITEMIDS    // in case you do IDList stuff you want this on for better type saftey
#include <windows.h>
#include <strsafe.h>
#include "ShellHelpers.h"
#include "DragDropHelpers.h"
#include "ResizeableDialog.h"
#include "LogWindow.h"
#include "resource.h"
#include <new>

// This class encapsulates the registration and dispatching of shell change notification events
//
// To use this class:
// 1) Derive a class from this class. The derived class will implement the virtual
//    method OnChangeNotify() that is called when the events occur.
//
// 2) Create an HWND and designate a message (in the WM_USER range) that will be used to
//    dispatch the events. This HWND and MSG is passed to StartWatching() along with the
//    item you want to watch.
//
// 3) In your window procedure, on receipt of the notification message, call OnChangeMessage().
//
// 4) Declare and implement OnChangeNotify() and write the code there that handles the events.

class CShellItemChangeWatcher
{
public:
    CShellItemChangeWatcher() : _ulRegister(0)
    {
    }

    ~CShellItemChangeWatcher()
    {
        StopWatching();
    }

    // lEvents is SHCNE_XXX values like SHCNE_ALLEVENTS
    // fRecursive means to listen for all events under this folder
    HRESULT StartWatching(IShellItem *psi, HWND hwnd, UINT uMsg, long lEvents, BOOL fRecursive)
    {
        StopWatching();

        PIDLIST_ABSOLUTE pidlWatch;
        HRESULT hr = SHGetIDListFromObject(psi, &pidlWatch);
        if (SUCCEEDED(hr))
        {
            SHChangeNotifyEntry const entries[] = { pidlWatch, fRecursive };

            int const nSources = SHCNRF_ShellLevel | SHCNRF_InterruptLevel | SHCNRF_NewDelivery;
            _ulRegister = SHChangeNotifyRegister(hwnd, nSources, lEvents, uMsg, ARRAYSIZE(entries), entries);
            hr = _ulRegister != 0 ? S_OK : E_FAIL;

            CoTaskMemFree(pidlWatch);
        }
        return hr;
    }

    void StopWatching()
    {
        if (_ulRegister)
        {
            SHChangeNotifyDeregister(_ulRegister);
            _ulRegister = 0;
        }
    }

    // in your window procedure call this message to dispatch the events
    void OnChangeMessage(WPARAM wParam, LPARAM lParam)
    {
        long lEvent;
        PIDLIST_ABSOLUTE *rgpidl;
        HANDLE hNotifyLock = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &rgpidl, &lEvent);
        if (hNotifyLock)
        {
            if (IsItemNotificationEvent(lEvent))
            {
                IShellItem2 *psi1 = NULL, *psi2 = NULL;

                if (rgpidl[0])
                {
                    SHCreateItemFromIDList(rgpidl[0], IID_PPV_ARGS(&psi1));
                }

                if (rgpidl[1])
                {
                    SHCreateItemFromIDList(rgpidl[1], IID_PPV_ARGS(&psi2));
                }

                // derived class implements this method, that is where the events are delivered
                OnChangeNotify(lEvent, psi1, psi2);

                SafeRelease(&psi1);
                SafeRelease(&psi2);
            }
            else
            {
                // dispatch non-item events here in the future
            }
            SHChangeNotification_Unlock(hNotifyLock);
        }
    }

    // derived class implements this event
    virtual void OnChangeNotify(long lEvent, IShellItem2 *psi1, IShellItem2 *psi2) = 0;

private:

    bool IsItemNotificationEvent(long lEvent)
    {
        return !(lEvent & (SHCNE_UPDATEIMAGE | SHCNE_ASSOCCHANGED | SHCNE_EXTENDED_EVENT | SHCNE_FREESPACE | SHCNE_DRIVEADDGUI | SHCNE_SERVERDISCONNECT));
    }

    ULONG _ulRegister;
};

#define MAP_ENTRY(x) {L#x, x}

PCWSTR EventName(long lEvent)
{
    PCWSTR psz = L"";

    static const struct { PCWSTR pszName; long lEvent; } c_rgEventNames[] =
    {
        MAP_ENTRY(SHCNE_RENAMEITEM),
        MAP_ENTRY(SHCNE_CREATE),
        MAP_ENTRY(SHCNE_DELETE),
        MAP_ENTRY(SHCNE_MKDIR),
        MAP_ENTRY(SHCNE_RMDIR),
        MAP_ENTRY(SHCNE_MEDIAINSERTED),
        MAP_ENTRY(SHCNE_MEDIAREMOVED),
        MAP_ENTRY(SHCNE_DRIVEREMOVED),
        MAP_ENTRY(SHCNE_DRIVEADD),
        MAP_ENTRY(SHCNE_NETSHARE),
        MAP_ENTRY(SHCNE_NETUNSHARE),
        MAP_ENTRY(SHCNE_ATTRIBUTES),
        MAP_ENTRY(SHCNE_UPDATEDIR),
        MAP_ENTRY(SHCNE_UPDATEITEM),
        MAP_ENTRY(SHCNE_SERVERDISCONNECT),
        MAP_ENTRY(SHCNE_DRIVEADDGUI),
        MAP_ENTRY(SHCNE_RENAMEFOLDER),
        MAP_ENTRY(SHCNE_FREESPACE),
        MAP_ENTRY(SHCNE_UPDATEITEM),
    };
    for (int i = 0; i < ARRAYSIZE(c_rgEventNames); i++)
    {
        if (c_rgEventNames[i].lEvent == lEvent)
        {
            psz = c_rgEventNames[i].pszName;
            break;
        }
    }
    return psz;
}

PCWSTR StringToEmpty(PCWSTR psz)
{
    return psz ? psz : L"";
}

HRESULT GetShellItemFromCommandLine(REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_FAIL;
    int cArgs;
    PWSTR *ppszCmd = CommandLineToArgvW(GetCommandLineW(), &cArgs);
    if (ppszCmd && cArgs > 1)
    {
        WCHAR szSpec[MAX_PATH];
        szSpec[0] = 0;

        // skip all parameters that begin with "-" or "/" to try to find the
        // file name. this enables parameters to be present on the cmd line
        // and not get in the way of this function
        for (int i = 1; (szSpec[0] == 0) && (i < cArgs); i++)
        {
            if ((*ppszCmd[i] != L'-') && (*ppszCmd[i] != L'/'))
            {
                StringCchCopyW(szSpec, ARRAYSIZE(szSpec), ppszCmd[i]);
                PathUnquoteSpacesW(szSpec);
            }
        }

        hr = szSpec[0] ? S_OK : E_FAIL; // protect against empty
        if (SUCCEEDED(hr))
        {
            hr = SHCreateItemFromParsingName(szSpec, NULL, riid, ppv);
            if (FAILED(hr))
            {
                WCHAR szFolder[MAX_PATH];
                GetCurrentDirectoryW(ARRAYSIZE(szFolder), szFolder);
                hr = PathAppendW(szFolder, szSpec) ? S_OK : E_FAIL;
                if (SUCCEEDED(hr))
                {
                    hr = SHCreateItemFromParsingName(szFolder, NULL, riid, ppv);
                }
            }
        }
    }
    return hr;
}

/*
    usage:

    CItemIterator itemIterator(psi);

    while (itemIterator.MoveNext())
    {
        IShellItem2 *psi;
        hr = itemIterator.GetCurrent(IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {

            psi->Release();
        }
    }
*/

class CItemIterator
{
public:
    CItemIterator(IShellItem *psi) : _hr(SHGetIDListFromObject(psi, &_pidlFull)), _psfCur(NULL)
    {
        _Init();
    }

    CItemIterator(PCIDLIST_ABSOLUTE pidl) : _hr(SHILCloneFull(pidl, &_pidlFull)), _psfCur(NULL)
    {
        _Init();
    }

    ~CItemIterator()
    {
        CoTaskMemFree(_pidlFull);
        SafeRelease(&_psfCur);
    }

    bool MoveNext()
    {
        bool fMoreItems = false;

        if (SUCCEEDED(_hr))
        {
            if (NULL == _pidlRel)
            {
                fMoreItems = true;
                _pidlRel = _pidlFull;   // first item, might be empty if it is the desktop
            }
            else if (!ILIsEmpty(_pidlRel))
            {
                PCUITEMID_CHILD pidlChild = (PCUITEMID_CHILD)_pidlRel;  // save the current segment for binding
                _pidlRel = ILNext(_pidlRel);

                // if we are not at the end setup for the next itteration
                if (!ILIsEmpty(_pidlRel))
                {
                    const WORD cbSave = _pidlRel->mkid.cb;  // avoid cloning for the child by truncating temporarily
                    _pidlRel->mkid.cb = 0;                  // make this a child

                    IShellFolder *psfNew;
                    _hr = _psfCur->BindToObject(pidlChild, NULL, IID_PPV_ARGS(&psfNew));
                    if (SUCCEEDED(_hr))
                    {
                        _psfCur->Release();
                        _psfCur = psfNew;   // transfer ownership
                        fMoreItems = true;
                    }

                    _pidlRel->mkid.cb = cbSave; // restore previous ID size
                }
            }
        }
        return fMoreItems;
    }

    HRESULT GetCurrent(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        if (SUCCEEDED(_hr))
        {
            // create teh childID by truncating _pidlRel temporarily
            PUIDLIST_RELATIVE pidlNext = ILNext(_pidlRel);
            const WORD cbSave = pidlNext->mkid.cb;  // save old cb
            pidlNext->mkid.cb = 0;                  // make _pidlRel a child

            _hr = SHCreateItemWithParent(NULL, _psfCur, (PCUITEMID_CHILD)_pidlRel, riid, ppv);

            pidlNext->mkid.cb = cbSave;             // restore old cb
        }
        return _hr;
    }

    HRESULT GetResult() const { return _hr; }
    PCUIDLIST_RELATIVE GetRelativeIDList() const { return _pidlRel; }

private:
    void _Init()
    {
        _pidlRel = NULL;

        if (SUCCEEDED(_hr))
        {
            _hr = SHGetDesktopFolder(&_psfCur);
        }
    }

    HRESULT _hr;
    PIDLIST_ABSOLUTE _pidlFull;
    PUIDLIST_RELATIVE _pidlRel;
    IShellFolder *_psfCur;
};

// debugging helper that returns a string that represents the IDList in
// this form "[computer][C:][Foo][bar.txt]".
HRESULT GetIDListName(IShellItem *psi, PWSTR *ppsz)
{
    *ppsz = NULL;
    HRESULT hr = E_FAIL;

    WCHAR szFullName[2048];
    szFullName[0] = 0;
    PWSTR pszOutput = szFullName;
    size_t cchOutput = ARRAYSIZE(szFullName);

    CItemIterator itemIterator(psi);
    while (itemIterator.MoveNext())
    {
        IShellItem2 *psi;
        hr = itemIterator.GetCurrent(IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            PWSTR pszName;
            hr = psi->GetDisplayName(SIGDN_PARENTRELATIVE, &pszName);
            if (SUCCEEDED(hr))
            {
                // ignore errors, this is for debugging only
                StringCchCatEx(pszOutput, cchOutput, L"[", &pszOutput, &cchOutput, 0);
                StringCchCatEx(pszOutput, cchOutput, pszName, &pszOutput, &cchOutput, 0);
                StringCchCatEx(pszOutput, cchOutput, L"]", &pszOutput, &cchOutput, 0);
                CoTaskMemFree(pszName);
            }

            psi->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = SHStrDup(szFullName, ppsz);
    }
    return hr;
}

enum GROUPID
{
    GROUPID_DEFAULT,
    GROUPID_NAMES,
};

typedef struct
{
    GROUPID groupid;
    PCWSTR pszGroupName;
} LOG_GROUP;

const LOG_GROUP c_rgGroupInfo[] =
{
    { GROUPID_NAMES, L"Event" },
};

const UINT c_notifyMessage = WM_USER + 200;

class CChangeNotifyApp : public CDragDropHelper, public CShellItemChangeWatcher
{
public:
    CChangeNotifyApp() : _logWindow(c_rgGroupInfo, ARRAYSIZE(c_rgGroupInfo)),
        _cRef(1), _hdlg(NULL), _psiDrop(NULL)
    {
    }

    HRESULT DoModal(HWND hwnd)
    {
        DialogBoxParam(GetModuleHINSTANCE(), MAKEINTRESOURCE(IDD_DIALOG), hwnd, s_DlgProc, (LPARAM)this);
        return S_OK;
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CChangeNotifyApp, IDropTarget),
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

    void OnChangeNotify(long lEvent, IShellItem2 *psi1, IShellItem2 *psi2)
    {
        PWSTR pszLeft = NULL, pszRight = NULL;

        if (psi1)
        {
            GetIDListName(psi1, &pszLeft);
        }

        if (psi2)
        {
            GetIDListName(psi2, &pszRight);
        }

        if (lEvent == SHCNE_RENAMEITEM || lEvent == SHCNE_RENAMEFOLDER)
        {
            _logWindow.LogMessagePrintf(GROUPID_NAMES, EventName(lEvent),
                L"%s ==> %s", StringToEmpty(pszLeft), StringToEmpty(pszRight));
        }
        else
        {
            _logWindow.LogMessagePrintf(GROUPID_NAMES, EventName(lEvent),
                L"%s , %s", StringToEmpty(pszLeft), StringToEmpty(pszRight));
        }

        CoTaskMemFree(pszLeft);
        CoTaskMemFree(pszRight);

        _logWindow.AutoAdjustListView();
    }

private:
    ~CChangeNotifyApp()
    {
        _FreeItem();
    }

    static INT_PTR CALLBACK s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CChangeNotifyApp *pssa = reinterpret_cast<CChangeNotifyApp *>(GetWindowLongPtr(hdlg, DWLP_USER));
        if (uMsg == WM_INITDIALOG)
        {
            pssa = reinterpret_cast<CChangeNotifyApp *>(lParam);
            pssa->_hdlg = hdlg;
            SetWindowLongPtr(hdlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pssa));
        }
        return pssa ? pssa->_DlgProc(uMsg, wParam, lParam) : FALSE;
    }

    INT_PTR _DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual HRESULT OnDrop(IShellItemArray *psia, DWORD grfKeyState);

    void _OnInitDlg();
    void _OnDestroyDlg();

    void _PickItem()
    {
        IFileDialog *pfd;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
        if (SUCCEEDED(hr))
        {
            DWORD dwOptions;
            if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
            {
                pfd->SetOptions(dwOptions | FOS_ALLNONSTORAGEITEMS | FOS_PICKFOLDERS);
            }

            pfd->SetTitle(L"Item Picker");

            hr = pfd->Show(_hdlg);
            if (SUCCEEDED(hr))
            {
                IShellItem *psi;
                hr = pfd->GetResult(&psi);
                if (SUCCEEDED(hr))
                {
                    _FreeItem();

                    if (SUCCEEDED(psi->QueryInterface(&_psiDrop)))
                    {
                        _StartWatching();
                    }
                    psi->Release();
                }
            }

            pfd->Release();
        }
    }

    void _StartWatching()
    {
        _logWindow.ResetContents();

        BOOL const fRecursive = IsDlgButtonChecked(_hdlg, IDC_RECURSIVE);
        StartWatching(_psiDrop, _hdlg, c_notifyMessage, SHCNE_ALLEVENTS, fRecursive);

        PWSTR pszName;
        if (SUCCEEDED(_psiDrop->GetDisplayName(SIGDN_NORMALDISPLAY, &pszName)))
        {
            WCHAR szTitle[128];
            StringCchPrintf(szTitle, ARRAYSIZE(szTitle), L"Watching - %s", pszName);

            _logWindow.LogMessagePrintf(GROUPID_NAMES, L"Watching", L"%s %s", pszName, fRecursive ? L"(Recursive)" : L"");
            _logWindow.AutoAdjustListView();

            SetWindowText(_hdlg, szTitle);
            CoTaskMemFree(pszName);
        }
    }

    void _FreeItem()
    {
        SafeRelease(&_psiDrop);
    }

    long _cRef;
    HWND _hdlg;
    IShellItem2 *_psiDrop;
    CLogWindow<LOG_GROUP, GROUPID> _logWindow;

    static const ANCHOR c_rgAnchors[5];
    RECT _rgAnchorOffsets[ARRAYSIZE(c_rgAnchors)];
};

const ANCHOR CChangeNotifyApp::c_rgAnchors[] =
{
    { IDC_PICK,         AF_LEFT | AF_BOTTOM },
    { IDC_LISTVIEW,     AF_LEFT | AF_RIGHT | AF_TOP | AF_BOTTOM },
    { IDC_RECURSIVE,    AF_RIGHT | AF_BOTTOM },
    { IDC_GRIPPER,      AF_RIGHT | AF_BOTTOM },
};

HRESULT CChangeNotifyApp::OnDrop(IShellItemArray *psia, DWORD)
{
    _FreeItem();

    // hold the dropped item for later in _psiDrop
    HRESULT hr = GetItemAt(psia, 0, IID_PPV_ARGS(&_psiDrop));
    if (SUCCEEDED(hr))
    {
        _StartWatching();
    }
    return S_OK;
}

void CChangeNotifyApp::_OnInitDlg()
{
    InitResizeData(_hdlg, c_rgAnchors, ARRAYSIZE(c_rgAnchors), _rgAnchorOffsets);
    SetDialogIcon(_hdlg, SIID_APPLICATION);

    InitializeDragDropHelper(_hdlg, DROPIMAGE_LABEL, L"Listen for Changes on %1");

    _logWindow.InitListView(GetDlgItem(_hdlg, IDC_LISTVIEW));

    // optional cmd line param
    if (SUCCEEDED(GetShellItemFromCommandLine(IID_PPV_ARGS(&_psiDrop))))
    {
        _StartWatching();
    }
}

void CChangeNotifyApp::_OnDestroyDlg()
{
    ClearDialogIcon(_hdlg);
    TerminateDragDropHelper();

    StopWatching();
}

INT_PTR CChangeNotifyApp::_DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bRet = TRUE;   // default for all handled cases in switch below

    switch (uMsg)
    {
    case WM_INITDIALOG:
        _OnInitDlg();
        break;

    case WM_COMMAND:
        {
            const int idCmd = GET_WM_COMMAND_ID(wParam, lParam);
            switch (idCmd)
            {
            case IDOK:
            case IDCANCEL:
                return EndDialog(_hdlg, idCmd);

            case IDC_RECURSIVE:
                if (_psiDrop)
                {
                    _StartWatching();
                }
                break;

            case IDC_PICK:
                _PickItem();
                break;
            }
        }
        break;

    case WM_NOTIFY:
        {
            NMHDR *pnm = (NMHDR*)lParam;
            if (pnm->idFrom == IDC_LISTVIEW)
            {
                _logWindow.OnNotify(pnm);
            }
        }
        break;

    case WM_SIZE:
        OnSize(_hdlg, c_rgAnchors, ARRAYSIZE(c_rgAnchors), _rgAnchorOffsets);
        break;

    case WM_DESTROY:
        _OnDestroyDlg();
        break;

    case c_notifyMessage:
        // This is the message that will be sent when changes are detected in the shell namespace
        OnChangeMessage(wParam, lParam);
        break;

    default:
        bRet = FALSE;
    }
    return bRet;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CChangeNotifyApp *pdlg = new (std::nothrow) CChangeNotifyApp();
        if (pdlg)
        {
            pdlg->DoModal(NULL);
            pdlg->Release();
        }
        CoUninitialize();
    }
    return 0;
}

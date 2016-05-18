// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <new>
#include <shlobj.h>
#include "resource.h"

// Setup common controls v6 the easy way
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// window message to inform main window to close a file
#define WM_FILEINUSE_CLOSEFILE (WM_USER + 1)

// this class implements the interface necessary to negotiate with the explorer
// when it hits sharing violations due to the file being open

class CFileIsInUseImpl : public IFileIsInUse
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IFileIsInUse
    IFACEMETHODIMP GetAppName(PWSTR *ppszName);
    IFACEMETHODIMP GetUsage(FILE_USAGE_TYPE *pfut);
    IFACEMETHODIMP GetCapabilities(DWORD *pdwCapabilitiesFlags);
    IFACEMETHODIMP GetSwitchToHWND(HWND *phwnd);
    IFACEMETHODIMP CloseFile();

    static HRESULT s_CreateInstance(HWND hwnd, PCWSTR pszFilePath, FILE_USAGE_TYPE fut, DWORD dwCapabilities, REFIID riid, void **ppv);

private:
    CFileIsInUseImpl();
    ~CFileIsInUseImpl();

    HRESULT _Initialize(HWND hwnd, PCWSTR pszFilePath, FILE_USAGE_TYPE fut, DWORD dwCapabilities);
    HRESULT _AddFileToROT();
    HRESULT _RemoveFileFromROT();

    long _cRef;
    WCHAR _szFilePath[MAX_PATH];
    HWND _hwnd;
    DWORD _dwCapabilities;
    DWORD _dwCookie;
    FILE_USAGE_TYPE _fut;
};

CFileIsInUseImpl::CFileIsInUseImpl(): _cRef(1), _hwnd(NULL), _fut(FUT_GENERIC), _dwCapabilities(0), _dwCookie(0)
{
    _szFilePath[0]  = '\0';
}

CFileIsInUseImpl::~CFileIsInUseImpl()
{
    _RemoveFileFromROT();
}

HRESULT CFileIsInUseImpl::_Initialize(HWND hwnd, PCWSTR pszFilePath, FILE_USAGE_TYPE fut, DWORD dwCapabilities)
{
    _hwnd  = hwnd;
    _fut   = fut;
    _dwCapabilities = dwCapabilities;
    HRESULT hr = StringCchCopy(_szFilePath, ARRAYSIZE(_szFilePath), pszFilePath);
    if (SUCCEEDED(hr))
    {
        hr = _AddFileToROT();
    }
    return hr;
}

HRESULT CFileIsInUseImpl::s_CreateInstance(HWND hwnd, PCWSTR pszFilePath, FILE_USAGE_TYPE fut, DWORD dwCapabilities, REFIID riid, void **ppv)
{
    CFileIsInUseImpl *pfiu = new (std::nothrow) CFileIsInUseImpl();
    HRESULT hr = (pfiu) ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pfiu->_Initialize(hwnd, pszFilePath, fut, dwCapabilities);
        if (SUCCEEDED(hr))
        {
            hr = pfiu->QueryInterface(riid, ppv);
        }
        pfiu->Release();
    }
    return hr;
}

HRESULT CFileIsInUseImpl::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CFileIsInUseImpl, IFileIsInUse),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CFileIsInUseImpl::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFileIsInUseImpl::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
    {
        delete this;
    }
    return cRef;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::CloseFile()
{
    // Notify main application window that we need to close
    // the file handle associated with this entry.  We do
    // not pass anything with this message since this sample
    // will only have a single file open at a time.
    SendMessage(_hwnd, WM_FILEINUSE_CLOSEFILE, (WPARAM)NULL, (LPARAM)NULL);
    _RemoveFileFromROT();
    return S_OK;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::GetAppName(PWSTR *ppszName)
{
    HRESULT hr = E_FAIL;
    WCHAR szModule[MAX_PATH];
    UINT cch = GetModuleFileName(NULL, szModule, ARRAYSIZE(szModule));
    if (cch != 0)
    {
        hr = SHStrDup(PathFindFileName(szModule), ppszName);
    }
    return hr;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::GetUsage(FILE_USAGE_TYPE *pfut)
{
    *pfut = _fut;
    return S_OK;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::GetCapabilities(DWORD *pdwCapabilitiesFlags)
{
    *pdwCapabilitiesFlags = _dwCapabilities;
    return S_OK;
}

// IFileIsInUse

HRESULT CFileIsInUseImpl::GetSwitchToHWND(HWND *phwnd)
{
    *phwnd = _hwnd;
    return S_OK;
}

HRESULT CFileIsInUseImpl::_AddFileToROT()
{
    IRunningObjectTable *prot;
    HRESULT hr = GetRunningObjectTable(NULL, &prot);
    if (SUCCEEDED(hr))
    {
        IMoniker *pmk;
        hr = CreateFileMoniker(_szFilePath, &pmk);
        if (SUCCEEDED(hr))
        {
            // Add ROTFLAGS_ALLOWANYCLIENT to make this work accross security boundaries
            hr = prot->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE | ROTFLAGS_ALLOWANYCLIENT,
                                static_cast<IFileIsInUse *>(this), pmk, &_dwCookie);
            if (hr == CO_E_WRONG_SERVER_IDENTITY)
            {
                // this failure is due to ROTFLAGS_ALLOWANYCLIENT and the fact that we don't
                // have the AppID registered for our CLSID. Try again without ROTFLAGS_ALLOWANYCLIENT
                // knowing that this means this can only work in the scope of apps running with the
                // same MIC level.
                hr = prot->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE,
                                    static_cast<IFileIsInUse *>(this), pmk, &_dwCookie);
            }
            pmk->Release();
        }
        prot->Release();
    }
    return hr;
}

HRESULT CFileIsInUseImpl::_RemoveFileFromROT()
{
    IRunningObjectTable *prot;
    HRESULT hr = GetRunningObjectTable(NULL, &prot);
    if (SUCCEEDED(hr))
    {
        if (_dwCookie)
        {
            hr = prot->Revoke(_dwCookie);
            _dwCookie = 0;
        }

        prot->Release();
    }
    return hr;
}

// Text for instructions on dialog
wchar_t const c_szInstructions[] = L"Drag and Drop a file here or click Open File... from the File menu";

// Default usage type flag to use with our IFileIsInUse implementation
#define FUT_DEFAULT FUT_EDITING

// Default capability flags to use with our IFileIsInUse implementation
#define OF_CAP_DEFAULT  OF_CAP_CANCLOSE | OF_CAP_CANSWITCHTO

// GUID associated with our application used to register
// as the AppID to run as InteractiveUser.  This should be
// unique to your application.
wchar_t const c_szClassGUID[] = L"{E9B568E4-297B-4576-A0DE-ACD9B229CCF3}";

class CFileInUseApp : public IDropTarget
{
public:
    CFileInUseApp() : _cRef(1), _hwnd(NULL), _hFile(INVALID_HANDLE_VALUE), _pfiu(NULL)
    {
    }

    HRESULT DoModal(HWND hwnd)
    {
        HRESULT hr = CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&_pdth));
        if (SUCCEEDED(hr))
        {
            DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAINDLG), hwnd, s_DlgProc, (LPARAM)this);
        }
        return hr;
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CFileInUseApp, IDropTarget),
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
        ULONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IDropTarget
    IFACEMETHODIMP DragEnter(IDataObject *pdtobj, DWORD /*grfKeyState*/, POINTL pt, DWORD *pdwEffect)
    {
        *pdwEffect &= DROPEFFECT_COPY | DROPEFFECT_LINK | DROPEFFECT_MOVE;
        if (_pdth)
        {
            POINT ptT = { pt.x, pt.y };
            _pdth->DragEnter(_hwnd, pdtobj, &ptT, *pdwEffect);
        }
        return S_OK;
    }

    IFACEMETHODIMP DragOver(DWORD /*grfKeyState*/, POINTL pt, DWORD *pdwEffect)
    {
        if (_pdth)
        {
            POINT ptT = { pt.x, pt.y };
            _pdth->DragOver(&ptT, *pdwEffect);
        }
        return S_OK;
    }

    IFACEMETHODIMP DragLeave()
    {
        if (_pdth)
        {
            _pdth->DragLeave();
        }
        return S_OK;
    }

    IFACEMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

private:
    ~CFileInUseApp()
    {
        _CloseFile();
        if (_pdth)
        {
            _pdth->Release();
        }
    }

    static INT_PTR CALLBACK s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CFileInUseApp *pcd;
        if (uMsg == WM_INITDIALOG)
        {
            pcd = reinterpret_cast<CFileInUseApp *>(lParam);
            pcd->_hwnd = hdlg;
            SetWindowLongPtr(hdlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pcd));
        }
        else
        {
            pcd = reinterpret_cast<CFileInUseApp *>(GetWindowLongPtr(hdlg, DWLP_USER));
        }
        return pcd ? pcd->_DlgProc(uMsg, wParam, lParam) : FALSE;
    }

    INT_PTR _DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnInitDlg();
    void _OnDestroy();
    void _OnOpenFile();
    void _OnCommand(UINT uCmd, HWND hwndNotify, UINT uCode);

    HRESULT _OpenFile(PCWSTR pszPath);
    void _CloseFile();

    long _cRef;
    HWND _hwnd;
    HANDLE _hFile;
    IDropTargetHelper *_pdth;
    IFileIsInUse *_pfiu;
};

// IDropTarget methods

IFACEMETHODIMP CFileInUseApp::Drop(IDataObject *pdtobj, DWORD /*grfKeyState*/, POINTL pt, DWORD *pdwEffect)
{
    if (_pdth)
    {
        POINT ptT = { pt.x, pt.y };
        _pdth->Drop(pdtobj, &ptT, *pdwEffect);
    }

    // Create a IShellItemArray from the IDataObject
    IShellItemArray *psia;
    HRESULT hr = SHCreateShellItemArrayFromDataObject(pdtobj, IID_PPV_ARGS(&psia));
    if (SUCCEEDED(hr))
    {
        // For this sample, we only open the first item that
        // was dragged and dropped to our application.
        IShellItem *psi;
        hr = psia->GetItemAt(0, &psi);
        if (SUCCEEDED(hr))
        {
            // Get the full path of the file
            PWSTR pszPath;
            hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
            if (SUCCEEDED(hr))
            {
                // Open the file
                hr = _OpenFile(pszPath);
                CoTaskMemFree(pszPath);
            }
            psi->Release();
        }
        psia->Release();
    }
    DragLeave();
    return S_OK;
}

HRESULT CFileInUseApp::_OpenFile(PCWSTR pszPath)
{
    // Close the file if it is already opened
    _CloseFile();
    // Initialize the IFileIsInUse object.  We use some default flags here
    // as an example.  If you modify these you will notice Windows Explorer
    // modify its File In Use dialog contents accordingly to match the usage
    // type and available capabilities.
    HRESULT hr = CFileIsInUseImpl::s_CreateInstance(_hwnd, pszPath, FUT_DEFAULT, OF_CAP_DEFAULT, IID_PPV_ARGS(&_pfiu));
    if (SUCCEEDED(hr))
    {
        // The lack of FILE_SHARE_READ or FILE_SHARE_WRITE attributes for the dwShareMode
        // parameter will cause the file to be locked from other processes.
        _hFile = CreateFile(pszPath, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (INVALID_HANDLE_VALUE != _hFile)
        {
            // Display the path in the
            PathSetDlgItemPath(_hwnd, IDC_INFO, pszPath);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        if (FAILED(hr))
        {
            // We failed somewhere above. Cleanup
            // our file in use object.
            _pfiu->Release();
            _pfiu = NULL;
        }
    }
    return hr;
}

void CFileInUseApp::_CloseFile()
{
    // Close the file handle
    if (INVALID_HANDLE_VALUE != _hFile)
    {
        CloseHandle(_hFile);
        _hFile = INVALID_HANDLE_VALUE;
    }

    // Release the IFileIsInUse instance which will
    // remove it from the Running Object Table
    if (_pfiu)
    {
        _pfiu->Release();
        _pfiu = NULL;
    }

    // Remove the file path from the dialog
    SetDlgItemText(_hwnd, IDC_INFO, c_szInstructions);
}

void CFileInUseApp::_OnInitDlg()
{
    // Initialize instructions
    SetDlgItemText(_hwnd, IDC_INFO, c_szInstructions);

    // Setup the application window
    // for drag and drop
    RegisterDragDrop(_hwnd, this);
}

void CFileInUseApp::_OnDestroy()
{
    // Remove drag and drop capabilities
    // from the application window
    RevokeDragDrop(_hwnd);
}

void CFileInUseApp::_OnOpenFile()
{
    WCHAR szPath[MAX_PATH] = {};

    OPENFILENAME ofn = {sizeof(ofn)};
    ofn.hwndOwner    = _hwnd;
    ofn.lpstrFilter  = L"All Files\0*.*\0";
    ofn.lpstrFile    = szPath;
    ofn.nMaxFile     = ARRAYSIZE(szPath);

    BOOL fOk = GetOpenFileName(&ofn);
    if (fOk)
    {
        // Open the file that was selected in
        // the Open File dialog
        _OpenFile(szPath);
    }
}

void CFileInUseApp::_OnCommand(UINT uCmd, HWND /*hwndNotify*/, UINT /*uCode*/)
{
    // The user has clicked a menu item
    switch (uCmd)
    {
    case IDM_FILE_OPENFILE:
        _OnOpenFile();
        break;

    case IDM_FILE_EXIT:
        _CloseFile();
        EndDialog(_hwnd, TRUE);
        break;

    case IDM_FILE_CLOSEFILE:
        _CloseFile();
        break;
    }
}

INT_PTR CFileInUseApp::_DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR lret = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        _OnInitDlg();
        break;

    case WM_CLOSE:
        _CloseFile();
        EndDialog(_hwnd, FALSE);
        break;

    case WM_INITMENUPOPUP:
        {
            // Disable the Close File menu item if it is not open
            UINT const uState = (INVALID_HANDLE_VALUE == _hFile) ? MF_GRAYED : MF_ENABLED;
            EnableMenuItem(GetMenu(_hwnd), IDM_FILE_CLOSEFILE, MF_BYCOMMAND | uState);
        }
        break;

    case WM_FILEINUSE_CLOSEFILE:
        // We have been notified from the IFileIsInUse object
        // to close the file
        _CloseFile();
        break;

    case WM_COMMAND:
        _OnCommand(LOWORD(wParam), (HWND)lParam, HIWORD(wParam));
        break;

    case WM_DESTROY:
        _OnDestroy();
        break;
    }
    return lret;
}

//
// NOTE:
// IRunningObjectTable::Register(.., ROTFLAGS_ALLOWANYCLIENT) requires AppID
// registration so COM can inspect our security seetings. without this the call
// Register() will fail with CO_E_WRONG_SERVER_IDENTITY(0x80004015) "The class is configured to run as a security
// id different from the caller"
//
// HKLM\Software\Classes\AppID\app.exe
// AppID = "{app clsid}"
//
// HKLM\Software\Classes\AppID\{app clsid}
// RunAs = "Interactive User"
//
// NOTE: this code requires write access to HKLM
HRESULT _RegisterThisAppRunAsInteractiveUser(PCWSTR pszCLSID)
{
    HRESULT hr = E_INVALIDARG;

    WCHAR szModule[MAX_PATH];
    if (GetModuleFileName(NULL, szModule, ARRAYSIZE(szModule)))
    {
        WCHAR szKey[MAX_PATH];
        hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), L"Software\\Classes\\AppID\\%s", PathFindFileName(szModule));
        if (SUCCEEDED(hr))
        {
            HKEY hk;
            LSTATUS ls = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL);
            hr = HRESULT_FROM_WIN32(ls);
            if (SUCCEEDED(hr))
            {
                RegSetValueEx(hk, L"AppID", 0, REG_SZ, (BYTE *)pszCLSID, sizeof(*pszCLSID) * (lstrlen(pszCLSID) + 1));
                RegCloseKey(hk);

                hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), L"Software\\Classes\\AppID\\%s", pszCLSID);
                if (SUCCEEDED(hr))
                {
                    ls = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKey, 0, NULL, 0, KEY_WRITE, NULL, &hk, NULL);
                    hr = HRESULT_FROM_WIN32(ls);
                    if (SUCCEEDED(hr))
                    {
                        RegSetValueEx(hk, L"RunAs", 0, REG_SZ, (BYTE *)L"Interactive User", sizeof(L"Interactive User"));
                        RegCloseKey(hk);
                    }
                }
            }
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    // Initialize for drag and drop
    HRESULT hr = OleInitialize(0);
    if (SUCCEEDED(hr))
    {
        _RegisterThisAppRunAsInteractiveUser(c_szClassGUID);
        CFileInUseApp *pdlg = new (std::nothrow) CFileInUseApp();
        if (pdlg)
        {
            pdlg->DoModal(NULL);
            pdlg->Release();
        }
        OleUninitialize();
    }
    return 0;
}

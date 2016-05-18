// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <wchar.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <new>
#include "resource.h"

#define WM_COPY_END (WM_USER + 1)

// Setup common controls v6 the easy way
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Max buffer size for displaying sink messages in list view
#define MAX_BUFF 1024

// The sink type labels we care about
// These are displayed in the list view

// Sink type enumeration
enum SINK_TYPE_ENUM
{
    SINK_TYPE_STARTOPERATIONS = 0,
    SINK_TYPE_FINISHOPERATIONS,
    SINK_TYPE_PRECOPYITEM,
    SINK_TYPE_POSTCOPYITEM,
    SINK_TYPE_UPDATEPROGRESS
};

const PCWSTR g_rgpszSinkType[] =
{
    L"StartOperations",
    L"FinishOperations",
    L"PreCopyItem",
    L"PostCopyItem",
    L"UpdateProgress"
};

// Structure we pass to our worker thread
struct THREAD_INFO
{
    IStream *pstm;
    HWND hwnd;
};

#define OPERATION_FLAGS_DEFAULT FOF_NOCONFIRMMKDIR

class CFileOpProgSinkApp : public IFileOperationProgressSink
{
public:
    CFileOpProgSinkApp() : _cRef(1), _hwnd(NULL), _hwndLV(NULL)
    {
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CFileOpProgSinkApp, IFileOperationProgressSink),
            {0},
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
        if (0 == cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IFileOperationProgressSink
    IFACEMETHODIMP StartOperations();
    IFACEMETHODIMP FinishOperations(HRESULT hrResult);
    IFACEMETHODIMP PreRenameItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/, PCWSTR /*pszNewName*/)
    {
        return S_OK;
    }
    IFACEMETHODIMP PostRenameItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/, PCWSTR /*pszNewName*/, HRESULT /*hrRename*/, IShellItem * /*psiNewlyCreated*/)
    {
        return S_OK;
    }
    IFACEMETHODIMP PreMoveItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/)
    {
        return S_OK;
    }
    IFACEMETHODIMP PostMoveItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/,
        IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/, HRESULT /*hrNewName*/, IShellItem * /*psiNewlyCreated*/)
    {
        return S_OK;
    }
    IFACEMETHODIMP PreCopyItem(DWORD dwFlags, IShellItem *psiItem,
        IShellItem *psiDestinationFolder, PCWSTR pszNewName);
    IFACEMETHODIMP PostCopyItem(DWORD dwFlags, IShellItem *psiItem,
        IShellItem *psiDestinationFolder, PCWSTR pwszNewName, HRESULT hrCopy,
        IShellItem *psiNewlyCreated);
    IFACEMETHODIMP PreDeleteItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/)
    {
        return S_OK;
    }
    IFACEMETHODIMP PostDeleteItem(DWORD /*dwFlags*/, IShellItem * /*psiItem*/, HRESULT /*hrDelete*/, IShellItem * /*psiNewlyCreated*/)
    {
        return S_OK;
    }
    IFACEMETHODIMP PreNewItem(DWORD /*dwFlags*/, IShellItem * /*psiDestinationFolder*/, PCWSTR /*pszNewName*/)
    {
        return S_OK;
    }
    IFACEMETHODIMP PostNewItem(DWORD /*dwFlags*/, IShellItem * /*psiDestinationFolder*/,
        PCWSTR /*pszNewName*/, PCWSTR /*pszTemplateName*/, DWORD /*dwFileAttributes*/, HRESULT /*hrNew*/, IShellItem * /*psiNewItem*/)
    {
        return S_OK;
    }
    IFACEMETHODIMP UpdateProgress(UINT iWorkTotal, UINT iWorkSoFar);
    IFACEMETHODIMP ResetTimer()
    {
        return S_OK;
    }
    IFACEMETHODIMP PauseTimer()
    {
        return S_OK;
    }
    IFACEMETHODIMP ResumeTimer()
    {
        return S_OK;
    }

    HRESULT DoModal()
    {
        DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, s_DlgProc, (LPARAM)this);
        return S_OK;
    }

private:

    ~CFileOpProgSinkApp(){}

    static INT_PTR CALLBACK s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CFileOpProgSinkApp *pcd;
        if (uMsg == WM_INITDIALOG)
        {
            pcd = reinterpret_cast<CFileOpProgSinkApp *>(lParam);
            pcd->_hwnd = hdlg;
            SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)pcd);
        }
        else
        {
            pcd = reinterpret_cast<CFileOpProgSinkApp *>(GetWindowLongPtr(hdlg, DWLP_USER));
        }
        return pcd ? pcd->_DlgProc(uMsg, wParam, lParam) : FALSE;
    }

    INT_PTR _DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void _OnInitDlg();
    void _OnCopyStart();
    void _OnCopyEnd();
    void _OnClear();

    void _AddSinkItem(SINK_TYPE_ENUM eSinkType, PCWSTR pszDescription);

    static DWORD WINAPI _CopyThreadProc(void *pvoid);

    long   _cRef;
    HWND   _hwnd;
    HWND   _hwndLV;
};


// IFileOperationProgressSink
IFACEMETHODIMP CFileOpProgSinkApp::StartOperations()
{
    _AddSinkItem(SINK_TYPE_STARTOPERATIONS, NULL);
    return S_OK;
}

// IFileOperationProgressSink
IFACEMETHODIMP CFileOpProgSinkApp::FinishOperations(HRESULT)
{
    _AddSinkItem(SINK_TYPE_FINISHOPERATIONS, NULL);
    return S_OK;
}

// IFileOperationProgressSink
IFACEMETHODIMP CFileOpProgSinkApp::PreCopyItem(DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder, PCWSTR)
{
    PWSTR pszItem;
    HRESULT hr = psiItem->GetDisplayName(SIGDN_FILESYSPATH, &pszItem);
    if (SUCCEEDED(hr))
    {
        PWSTR pszDest;
        hr = psiDestinationFolder->GetDisplayName(SIGDN_FILESYSPATH, &pszDest);
        if (SUCCEEDED(hr))
        {
            WCHAR szBuff[MAX_BUFF];
            hr = StringCchPrintf(szBuff, ARRAYSIZE(szBuff), L"Flags: %u, Item: %s, Destination: %s",
                                 dwFlags, pszItem, pszDest);
            if (SUCCEEDED(hr))
            {
                _AddSinkItem(SINK_TYPE_PRECOPYITEM, szBuff);
            }
            CoTaskMemFree(pszDest);
        }
        CoTaskMemFree(pszItem);
    }
    return S_OK;
}

// IFileOperationProgressSink
IFACEMETHODIMP CFileOpProgSinkApp::PostCopyItem(DWORD dwFlags, IShellItem *psiItem, IShellItem *psiDestinationFolder,
                                               PCWSTR, HRESULT hrCopy, IShellItem *)
{
    PWSTR pszItem;
    HRESULT hr = psiItem->GetDisplayName(SIGDN_FILESYSPATH, &pszItem);
    if (SUCCEEDED(hr))
    {
        PWSTR pszDest;
        hr = psiDestinationFolder->GetDisplayName(SIGDN_FILESYSPATH, &pszDest);
        if (SUCCEEDED(hr))
        {
            WCHAR szBuff[MAX_BUFF];
            hr = StringCchPrintf(szBuff, ARRAYSIZE(szBuff),
                                 L"Flags: %u, HRESULT: 0x%x, Item: %s, Destination: %s",
                                 dwFlags, hrCopy, pszItem, pszDest);
            if (SUCCEEDED(hr))
            {
                _AddSinkItem(SINK_TYPE_POSTCOPYITEM, szBuff);
            }
            CoTaskMemFree(pszDest);
        }
        CoTaskMemFree(pszItem);
    }
    return S_OK;
}

// IFileOperationProgressSink
IFACEMETHODIMP CFileOpProgSinkApp::UpdateProgress(UINT iWorkTotal, UINT iWorkSoFar)
{
    WCHAR szBuff[MAX_BUFF];
    if (SUCCEEDED(StringCchPrintf(szBuff, ARRAYSIZE(szBuff), L"Total Work: %u, Work Completed: %u", iWorkTotal, iWorkSoFar)))
    {
        _AddSinkItem(SINK_TYPE_UPDATEPROGRESS, szBuff);
    }
    return S_OK;
}

void CFileOpProgSinkApp::_OnCopyStart()
{
    // Disable the buttons on the dialog
    EnableWindow(GetDlgItem(_hwnd, ID_COPY), FALSE);
    EnableWindow(GetDlgItem(_hwnd, ID_CLEAR), FALSE);

    // Enable the list view
    EnableWindow(_hwndLV, TRUE);

    // Ensure list view is cleared for
    // the new operation
    ListView_DeleteAllItems(_hwndLV);

    THREAD_INFO *pti = (THREAD_INFO *)LocalAlloc(LPTR, sizeof(*pti));
    if (pti)
    {
        pti->hwnd = _hwnd;
        // We want to marshall over the IFileOperationProgressSink
        // to the worker thread.
        if (SUCCEEDED(CoMarshalInterThreadInterfaceInStream(IID_IFileOperationProgressSink, this, &pti->pstm)))
        {
            // Launch the copy thread.  We do our operation on a
            // separate thread so we do not block the UI. We must
            // marshall over our IFileOperationProgressSink.
            if (SHCreateThread(_CopyThreadProc, pti, CTF_COINIT, NULL))
            {
                // thread assumes ownership
                pti = NULL;
            }
            else
            {
                // Restore dialog state
                _OnCopyEnd();
                // release our stream
                pti->pstm->Release();
            }
        }

        if (pti)
        {
            LocalFree(pti);
        }
    }
}

void CFileOpProgSinkApp::_OnCopyEnd()
{
    // Enable the buttons on the dialog
    EnableWindow(GetDlgItem(_hwnd, ID_COPY), TRUE);
    EnableWindow(GetDlgItem(_hwnd, ID_CLEAR), TRUE);
}

void CFileOpProgSinkApp::_OnClear()
{
    // Clear the edit controls
    SetDlgItemText(_hwnd, IDC_SRC, L"");
    SetDlgItemText(_hwnd, IDC_DEST, L"");

    // Clear and disable the list view
    ListView_DeleteAllItems(_hwndLV);
    EnableWindow(_hwndLV, FALSE);
}

void CFileOpProgSinkApp::_AddSinkItem(SINK_TYPE_ENUM eSinkType, PCWSTR pszDescription)
{
    // Create a new list view item for this sink message
    LVITEM lvi = {};
    lvi.iItem = ListView_GetItemCount(_hwndLV);
    lvi.mask  = LVIF_TEXT | LVIF_STATE;
    ListView_InsertItem(_hwndLV, &lvi);

    // Add the sink type to the list item
    lvi.mask     = LVIF_TEXT;
    lvi.pszText  = const_cast<PWSTR>(g_rgpszSinkType[(int)eSinkType]);
    ListView_SetItem(_hwndLV, &lvi);

    // Get the current time
    WCHAR szTime[50] = {};
    _wstrtime_s(szTime, ARRAYSIZE(szTime));

    // Add the time to the list item
    lvi.mask     = LVIF_TEXT;
    lvi.iSubItem = 1;
    lvi.pszText  = szTime;
    ListView_SetItem(_hwndLV, &lvi);

    // Add the description for the event to the list item
    lvi.mask     = LVIF_TEXT;
    lvi.iSubItem = 2;
    lvi.pszText  = const_cast<PWSTR>(pszDescription);
    ListView_SetItem(_hwndLV, &lvi);
}

// List view column labels

#define SZ_COL_SINKTYPE    L"Type"
#define SZ_COL_TIME        L"Time"
#define SZ_COL_DESCRIPTION L"Description"

void CFileOpProgSinkApp::_OnInitDlg()
{
    // Initialize the list view which shows the
    // sink results
    _hwndLV = GetDlgItem(_hwnd, IDC_SINKLIST);

    ListView_SetExtendedListViewStyle(_hwndLV, LVS_EX_FULLROWSELECT);

    // Initialize the columns
    LV_COLUMN lvc = {};

    lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvc.fmt     = LVCFMT_LEFT;
    lvc.cx      = 100;
    lvc.pszText = SZ_COL_SINKTYPE;
    ListView_InsertColumn(_hwndLV, 0, &lvc);

    lvc.cx      = 100;
    lvc.pszText = SZ_COL_TIME;
    ListView_InsertColumn(_hwndLV, 1, &lvc);

    lvc.cx      = 225;
    lvc.pszText = SZ_COL_DESCRIPTION;
    ListView_InsertColumn(_hwndLV, 2, &lvc);

    // Disable the list view by default
    EnableWindow(_hwndLV, FALSE);

    // Setup edit controls for auto-complete
    SHAutoComplete(GetDlgItem(_hwnd, IDC_SRC), SHACF_FILESYSTEM);
    SHAutoComplete(GetDlgItem(_hwnd, IDC_DEST), SHACF_FILESYSTEM);
}

INT_PTR CFileOpProgSinkApp::_DlgProc(UINT uMsg, WPARAM wParam, LPARAM)
{
    INT_PTR lret = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        _OnInitDlg();
        break;

    case WM_CLOSE:
        // Ensure our copy thread is done
        _OnCopyEnd();
        EndDialog(_hwnd, FALSE);
        break;

    case WM_COPY_END:
        // Copy thread has ended
        _OnCopyEnd();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_COPY:
            _OnCopyStart();
            break;
        case ID_CLEAR:
            _OnClear();
            break;
        }
        break;
    }
    return lret;
}

// Thread to perform the operation

DWORD WINAPI CFileOpProgSinkApp::_CopyThreadProc(void *pvoid)
{
    THREAD_INFO *pti = reinterpret_cast<THREAD_INFO *>(pvoid);
    // Perform the operation
    IFileOperation *pfo;
    // Create the file operation object
    HRESULT hr = CoCreateInstance(__uuidof(FileOperation), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
    if (SUCCEEDED(hr))
    {
        IFileOperationProgressSink *pfops;
        // Get our marshalled IFileOperationProgressSink
        hr = CoGetInterfaceAndReleaseStream(pti->pstm, IID_PPV_ARGS(&pfops));
        pti->pstm = NULL;
        if (SUCCEEDED(hr))
        {
            // Setup our callback interface (IFileOperationProgressSink)
            DWORD dwCookie = 0;
            hr = pfo->Advise(pfops, &dwCookie);
            if (SUCCEEDED(hr))
            {
                // Get the source and destination paths of the copy operation
                WCHAR szSrcPath[MAX_PATH] = {};
                hr = (GetDlgItemText(pti->hwnd, IDC_SRC, szSrcPath, ARRAYSIZE(szSrcPath)) > 0) ? S_OK : E_FAIL;
                if (SUCCEEDED(hr))
                {
                    // Create an IShellItem from the supplied source path
                    IShellItem *psiFrom;
                    hr = SHCreateItemFromParsingName(szSrcPath, NULL, IID_PPV_ARGS(&psiFrom));
                    if (SUCCEEDED(hr))
                    {
                        WCHAR szDestPath[MAX_PATH] = {};
                        hr = (GetDlgItemText(pti->hwnd, IDC_DEST, szDestPath, ARRAYSIZE(szDestPath)) > 0) ? S_OK : E_FAIL;
                        if (SUCCEEDED(hr))
                        {
                            IShellItem *psiTo;
                            // Create an IShellItem from the supplied path
                            hr = SHCreateItemFromParsingName(szDestPath, NULL, IID_PPV_ARGS(&psiTo));
                            if (SUCCEEDED(hr))
                            {
                                // Add the copy operation.  We do not add the IFileOperationProgressSink
                                // here since we already did this in call to Advise().  If you add it
                                // again here you will get duplicate sink notifications for the PreCopyItem
                                // and PostCopyItem.
                                hr = pfo->CopyItem(psiFrom, psiTo, NULL, NULL);
                                psiTo->Release();
                            }
                        }
                        psiFrom->Release();
                    }
                }

                if (SUCCEEDED(hr))
                {
                    // Set the main dialog as the owner of any UI (progress, confirmations)
                    hr = pfo->SetOwnerWindow(pti->hwnd);
                    if (SUCCEEDED(hr))
                    {
                        // Set our default operation flags for the operation
                        hr = pfo->SetOperationFlags(OPERATION_FLAGS_DEFAULT);
                        if (SUCCEEDED(hr))
                        {
                            // Perform the operation to copy the item
                            hr = pfo->PerformOperations();
                        }
                    }
                }
                // Remove the callback
                pfo->Unadvise(dwCookie);
            }
            pfops->Release();
        }
        pfo->Release();
    }

    if (pti->pstm)
    {
        pti->pstm->Release();
    }

    // Notify the main window that we are done
    PostMessage(pti->hwnd, WM_COPY_END, NULL, NULL);

    // Clean up the passed in THREAD_INFO struct
    LocalFree(pti);

    return 0;
}

int WINAPI wWinMain(HINSTANCE /*hinst*/, HINSTANCE /*hinstPrev*/, PWSTR /*pszCmdLine*/, int /*nCmdShow*/)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CFileOpProgSinkApp *pdlg = new (std::nothrow) CFileOpProgSinkApp();
        if (pdlg)
        {
            pdlg->DoModal();
            pdlg->Release();
        }
        CoUninitialize();
    }
    return 0;
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#define STRICT_TYPED_ITEMIDS    // in case you do IDList stuff you want this on for better type saftey

#pragma once
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define STRICT_TYPED_ITEMIDS
#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <shobjidl.h>
#include <strsafe.h>
#include <new>
#include "resource.h"

#define MAX_TREE_DEPTH 255

HINSTANCE g_hinst = NULL;

// Forward Declarations
INT_PTR CALLBACK MainPageDialogProc(HWND hwndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK BackupDialogProc(HWND hwndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT DoBackup(HWND hwnd);
HRESULT ShowBackupFolderPicker(HWND hwnd, IShellItem **ppsi);
HRESULT WalkFolderContents(HWND hwndTreeView, IShellItem *psiItem, HTREEITEM hTreeParent);
HTREEITEM AddItemToTreeView(HWND hwndTreeView, IShellItem *psiItem, HTREEITEM hTreeParent);

// INamespaceWalkCB implementation
class CNameSpaceWalkCB : public INamespaceWalkCB2
{
public:
    CNameSpaceWalkCB(HWND hwndTreeView, HTREEITEM hTreeParent): _cRef(1), _iCurTreeDepth(0), _hwndTreeView(hwndTreeView)
    {
        _rghTreeParentItemArray[_iCurTreeDepth] = hTreeParent;
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CNameSpaceWalkCB, INamespaceWalkCB),
            QITABENT(CNameSpaceWalkCB, INamespaceWalkCB2),
            {0},
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(DWORD) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(DWORD) Release()
    {
        ULONG cRef = InterlockedDecrement(&_cRef);
        if (0 == cRef)
        {
            delete this;
        }
        return cRef;
    }

    // INamespaceWalkCB
    IFACEMETHODIMP FoundItem(IShellFolder *psf, PCUITEMID_CHILD pidl)
    {
        IShellItem *psi;
        HRESULT hr = SHCreateItemWithParent(NULL, psf, pidl, IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            hr = AddItemToTreeView(_hwndTreeView, psi, _rghTreeParentItemArray[_iCurTreeDepth]) ? S_OK : E_FAIL;
            psi->Release();
        }
        return hr;
    }

    IFACEMETHODIMP EnterFolder(IShellFolder *psf, PCUITEMID_CHILD pidl)
    {
        HRESULT hr = (_iCurTreeDepth < ARRAYSIZE(_rghTreeParentItemArray) - 1) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            IShellItem *psi;
            hr = SHCreateItemWithParent(NULL, psf, pidl, IID_PPV_ARGS(&psi));
            if (SUCCEEDED(hr))
            {
                _rghTreeParentItemArray[_iCurTreeDepth + 1] = AddItemToTreeView(_hwndTreeView, psi, _rghTreeParentItemArray[_iCurTreeDepth]);
                hr = _rghTreeParentItemArray[_iCurTreeDepth + 1] ? S_OK : E_FAIL;
                _iCurTreeDepth++;
                psi->Release();
            }
        }
        return hr;
    }

    IFACEMETHODIMP LeaveFolder(IShellFolder * /*psf*/, PCUITEMID_CHILD /*pidl*/)
    {
        HRESULT hr = _iCurTreeDepth > 0 ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            _rghTreeParentItemArray[_iCurTreeDepth--] = NULL;
        }
        return hr;
    }


    IFACEMETHODIMP InitializeProgressDialog(PWSTR *ppszTitle, PWSTR *ppszCancel)
    { *ppszTitle = NULL; *ppszCancel = NULL; return E_NOTIMPL; }


    // INamespaceWalkCB2
    IFACEMETHODIMP WalkComplete(HRESULT /*hr*/)
    { return S_OK; }

private:
    ~CNameSpaceWalkCB()
    { }

    long _cRef;
    int _iCurTreeDepth;
    HWND _hwndTreeView;
    HTREEITEM _rghTreeParentItemArray[MAX_TREE_DEPTH];
};

HRESULT DoBackup(HWND hwnd)
{
    IShellItem *psi;
    HRESULT hr = ShowBackupFolderPicker(hwnd, &psi);
    if (SUCCEEDED(hr))
    {
        HWND hwndTreeView = GetDlgItem(hwnd, IDC_BACKUPTREE);
        hr = hwndTreeView ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            // Enumerate the locations if the folder is a library
            IShellLibrary *psl;
            if (SUCCEEDED(SHLoadLibraryFromItem(psi, STGM_READ, IID_PPV_ARGS(&psl))))
            {
                // Make sure the library is up-to-date
                SHResolveLibrary(psi);

                HTREEITEM htiLibrary = AddItemToTreeView(hwndTreeView, psi, TVI_ROOT);
                hr = htiLibrary ? S_OK : E_FAIL;
                if (htiLibrary)
                {
                    IShellItemArray *psiaFolders;
                    hr = psl->GetFolders(LFF_FORCEFILESYSTEM, IID_PPV_ARGS(&psiaFolders));
                    if (SUCCEEDED(hr))
                    {
                        DWORD cFolders;
                        hr = psiaFolders->GetCount(&cFolders);
                        if (SUCCEEDED(hr))
                        {
                            UINT cFoldersAdded = 0;
                            for (UINT i = 0; SUCCEEDED(hr) && i < cFolders; i++)
                            {
                                IShellItem *psiTemp;
                                hr = psiaFolders->GetItemAt(i, &psiTemp);
                                if (SUCCEEDED(hr))
                                {
                                    // Walk the contents of this location
                                    hr = WalkFolderContents(hwndTreeView, psiTemp, htiLibrary);
                                    if (SUCCEEDED(hr))
                                    {
                                        cFoldersAdded++;
                                    }
                                    psiTemp->Release();
                                }
                            }

                            if (cFoldersAdded)
                            {
                                TreeView_SortChildren(hwndTreeView, htiLibrary, FALSE);
                            }
                        }
                        psiaFolders->Release();
                    }
                }
                psl->Release();
            }
            else
            {
                hr = WalkFolderContents(hwndTreeView, psi, TVI_ROOT);
                if (SUCCEEDED(hr))
                {
                    TreeView_SortChildren(hwndTreeView, TVI_ROOT, FALSE);
                }
            }
        }
        psi->Release();
    }
    return hr;
}

// Folder picker
HRESULT ShowBackupFolderPicker(HWND hwnd, IShellItem **ppsi)
{
    *ppsi = NULL;
    IFileOpenDialog *pfod;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC,  IID_PPV_ARGS(&pfod));
    if (SUCCEEDED(hr))
    {
        hr = pfod->SetOptions(FOS_PICKFOLDERS);
        if (SUCCEEDED(hr))
        {
            hr = pfod->Show(hwnd);
            if (SUCCEEDED(hr))
            {
                hr = pfod->GetResult(ppsi);
            }
        }
        pfod->Release();
    }
    return hr;
}

HRESULT WalkFolderContents(HWND hwndTreeView, IShellItem *psiItem, HTREEITEM hTreeParent)
{
    INamespaceWalkCB *pnswcb = new (std::nothrow) CNameSpaceWalkCB(hwndTreeView, hTreeParent);
    HRESULT hr = pnswcb ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        INamespaceWalk *pnsw;
        hr = CoCreateInstance(CLSID_NamespaceWalker, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pnsw));
        if (SUCCEEDED(hr))
        {
            hr = pnsw->Walk(psiItem, NSWF_ASYNC | NSWF_DONT_TRAVERSE_LINKS | NSWF_DONT_ACCUMULATE_RESULT, MAX_TREE_DEPTH, pnswcb);
            pnsw->Release();
        }
        pnswcb->Release();
    }
    return hr;
}

HTREEITEM AddItemToTreeView(HWND hwndTreeView, IShellItem *psiItem, HTREEITEM hTreeParent)
{
    HTREEITEM htiAdded = NULL;

    PWSTR pszName;
    if (SUCCEEDED(psiItem->GetDisplayName(SIGDN_NORMALDISPLAY, &pszName)))
    {
        TVINSERTSTRUCT tvins = {};
        tvins.item.mask       = TVIF_TEXT;
        tvins.item.pszText    = pszName;
        tvins.hInsertAfter    = TVI_LAST;
        tvins.hParent         = hTreeParent;

        htiAdded = TreeView_InsertItem(hwndTreeView, &tvins);
        CoTaskMemFree(pszName);
    }
    return htiAdded;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int /*nCmdShow*/)
{
    g_hinst = hInstance;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        UINT cPages = 0;
        HPROPSHEETPAGE rhpsp[2] = {};

        // Create the main page of the wizard
        PROPSHEETPAGE psp = {sizeof(psp)};
        psp.dwFlags        = PSP_USEHEADERTITLE;
        psp.hInstance      = g_hinst;
        psp.pszTemplate    = MAKEINTRESOURCE(IDD_MainPage);
        psp.pfnDlgProc     = MainPageDialogProc;
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_MAINTITLE);

        rhpsp[cPages] = CreatePropertySheetPage(&psp);
        if (rhpsp[cPages])
        {
            cPages++;

            // Create the Backup page of the wizard
            psp.dwFlags        = PSP_USEHEADERTITLE;
            psp.pszTemplate    = MAKEINTRESOURCE(IDD_BackupPage);
            psp.pfnDlgProc     = BackupDialogProc;
            psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_BACKUPTITLE);

            rhpsp[cPages] = CreatePropertySheetPage(&psp);
            if (rhpsp[cPages])
            {
                cPages++;
            }
        }

        if (cPages == ARRAYSIZE(rhpsp))
        {
            PROPSHEETHEADER psh = {sizeof(psh)};
            psh.dwFlags    = PSH_AEROWIZARD;
            psh.hInstance  = g_hinst;
            psh.nPages     = ARRAYSIZE(rhpsp);
            psh.phpage     = rhpsp;
            psh.pszCaption = MAKEINTRESOURCE(IDS_APP_TITLE);
            PropertySheet(&psh);
        }
        else
        {
            for (UINT iPage = 0; iPage < cPages; iPage++)
            {
                DestroyPropertySheetPage(rhpsp[iPage]);
            }
        }
        CoUninitialize();
    }
    return 0;
}

INT_PTR CALLBACK MainPageDialogProc(HWND hwndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_ShowWizButtons(hwndDialog,
                    PSWIZB_SHOW,
                    PSWIZB_BACK | PSWIZB_CANCEL | PSWIZB_FINISH | PSWIZB_NEXT | PSWIZB_RESTORE);
                break;
            }
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BACKUP:
            PropSheet_ShowWizButtons(hwndDialog, PSWIZB_NEXT, PSWIZB_NEXT);
            PropSheet_PressButton(hwndDialog, PSBTN_NEXT);
            break;
        }
        break;
    }
    return 0;
}

INT_PTR CALLBACK BackupDialogProc(HWND hwndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;
            switch (pnmh->code)
            {
            case PSN_SETACTIVE:
                PropSheet_ShowWizButtons(hwndDialog,
                    PSWIZB_BACK | PSWIZB_FINISH,
                    PSWIZB_BACK | PSWIZB_CANCEL | PSWIZB_FINISH | PSWIZB_NEXT | PSWIZB_RESTORE);
                break;
            case PSN_WIZBACK:
                TreeView_DeleteAllItems(GetDlgItem(hwndDialog, IDC_BACKUPTREE));
                break;
            }
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BACKUPADDDIR:
            DoBackup(hwndDialog);
            break;

        case IDC_BACKUPREMOVEDIR:
            {
                HWND hwndTreeView = GetDlgItem(hwndDialog, IDC_BACKUPTREE);
                if (hwndTreeView)
                {
                    HTREEITEM hTreeItem = TreeView_GetSelection(hwndTreeView);
                    if (hTreeItem)
                    {
                        TreeView_DeleteItem(hwndTreeView, hTreeItem);
                    }
                }
            }
            break;
        }
        break;
    }
    return 0;
}

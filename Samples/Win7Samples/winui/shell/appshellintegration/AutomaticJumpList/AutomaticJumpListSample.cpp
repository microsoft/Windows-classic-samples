// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#define NTDDI_VERSION NTDDI_WIN7  // Specifies that the minimum required platform is Windows 7.
#define WIN32_LEAN_AND_MEAN       // Exclude rarely-used stuff from Windows headers
#define STRICT_TYPED_ITEMIDS      // Utilize strictly typed IDLists

// Windows Header Files:
#include <windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <strsafe.h>

// Header Files for Jump List features
#include <objectarray.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <knownfolders.h>
#include <shlobj.h>

#include "resource.h"
#include "FileRegistrations.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define REGPATH_SAMPLE        L"Software\\Microsoft\\Samples\\AutomaticJumpListSample"
#define REGVAL_RECENTCATEGORY L"RecentCategorySelected"

HINSTANCE g_hInst = NULL;

WCHAR const c_szTitle[] = L"AutomaticJumpListSample";
WCHAR const c_szWindowClass[] = L"AUTOMATICJUMPLISTSAMPLE";

PCWSTR const c_rgpszFiles[] =
{
    L"Microsoft_Sample_1.txt",
    L"Microsoft_Sample_2.txt",
    L"Microsoft_Sample_3.doc",
    L"Microsoft_Sample_4.doc"
};

// Creates a set of sample files in the current user's Documents directory to use as items in the
// custom category inserted into the Jump List.
HRESULT CreateSampleFiles()
{
    PWSTR pszPathDocuments;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, NULL, &pszPathDocuments);
    if (SUCCEEDED(hr))
    {
        for (UINT i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(c_rgpszFiles); i++)
        {
            WCHAR szPathSample[MAX_PATH];
            hr = PathCombine(szPathSample, pszPathDocuments, c_rgpszFiles[i]) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                IStream *pstm;
                hr = SHCreateStreamOnFileEx(szPathSample, (STGM_WRITE | STGM_FAILIFTHERE), FILE_ATTRIBUTE_NORMAL, TRUE, NULL, &pstm);
                if (SUCCEEDED(hr))
                {
                    PCWSTR pszText = L"This is a sample file for the CustomJumpListSample.\r\n";
                    ULONG cb = (sizeof(pszText[0]) * (lstrlen(pszText) + 1));
                    hr = IStream_Write(pstm, pszText, cb);
                    pstm->Release();
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_EXISTS) == hr)
                {
                    // If the file exists, we're ok, we'll just reuse it
                    hr = S_OK;
                }
            }
        }
        CoTaskMemFree(pszPathDocuments);
    }
    return hr;
}

// Cleans up the sample files that were created in the current user's Documents directory
void CleanupSampleFiles()
{
    PWSTR pszPathDocuments;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, NULL, &pszPathDocuments);
    if (SUCCEEDED(hr))
    {
        // Don't abort the loop if we fail to cleanup a file, we still want to try to clean up the rest
        for (UINT i = 0; i < ARRAYSIZE(c_rgpszFiles); i++)
        {
            WCHAR szPathSample[MAX_PATH];
            hr = PathCombine(szPathSample, pszPathDocuments, c_rgpszFiles[i]) ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                DeleteFile(szPathSample);
            }
        }
        CoTaskMemFree(pszPathDocuments);
    }
}

// Selects an item using the common File Open dialog, to simulate opening a document, an operation that should
// result in the selected item being added to the application's automatic Jump List.
void OpenItem(HWND hwnd)
{
    IFileOpenDialog *pdlg;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pdlg));
    if (SUCCEEDED(hr))
    {
        const COMDLG_FILTERSPEC c_rgTypes[] =
        {
            {L"Sample File Types (*.txt;*.doc)", L"*.txt;*.doc"},
        };
        hr = pdlg->SetFileTypes(ARRAYSIZE(c_rgTypes), c_rgTypes);
        if (SUCCEEDED(hr))
        {
            hr = pdlg->SetFileTypeIndex(1);
            if (SUCCEEDED(hr))
            {
                // Start in the Documents folder, where the sample files were created
                IShellItem *psiFolder;
                hr = SHCreateItemInKnownFolder(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL, IID_PPV_ARGS(&psiFolder));
                if (SUCCEEDED(hr))
                {
                    hr = pdlg->SetDefaultFolder(psiFolder);
                    if (SUCCEEDED(hr))
                    {
                        hr = pdlg->Show(hwnd);
                        if (SUCCEEDED(hr))
                        {
                            IShellItem *psi;
                            hr = pdlg->GetResult(&psi);
                            if (SUCCEEDED(hr))
                            {
                                // Unless FOS_DONTADDTORECENT is set via IFileDialog(and derivatives)::SetOptions
                                // prior to calling IFileDialog::Show, the common File Open/Save dialogs will
                                // call SHAddToRecentDocs on the application's behalf.  However, it is preferred
                                // that applications manually call this method in all locations where documents
                                // are opened via user action, to ensure no documents are left out of the
                                // Jump List.  Jump Lists handle the duplicate calls so that items are not
                                // improperly promoted in the Recent or Frequent lists when their usage is reported
                                // many times in rapid succession.
                                SHAddToRecentDocs(SHARD_SHELLITEM, psi);
                                psi->Release();
                            }
                        }
                    }
                    psiFolder->Release();
                }
            }
        }
        pdlg->Release();
    }
}

// Removes all items in the automatic Jump List for the calling application, except for items the user has pinned
// to the Jump List.  The list of pinned items is not accessible to applications.
void ClearHistory()
{
    IApplicationDestinations *pad;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationDestinations, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pad));
    if (SUCCEEDED(hr))
    {
        hr = pad->RemoveAllDestinations();
        pad->Release();
    }
}

// Sets the Known Category (Frequent or Recent) that is displayed in the Jump List for this application.  Document
// creation applications are typically best served by the Recent category, while media consumption applications
// usually utilize the Frequent category.  Applications should never request that BOTH categories appear in the
// same Jump List, as the two categories may present duplicates of each other.
HRESULT SetKnownCategory(BOOL fRecentSelected)
{
    // The visible categories are controlled via the ICustomDestinationList interface.  If not customized,
    // applications will get the Recent category by default.
    ICustomDestinationList *pcdl;
    HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
    if (SUCCEEDED(hr))
    {
        // The cMinSlots and poaRemoved values can be ignored when only a Known Category is being added - those
        // parameters apply only to applications adding custom categories or tasks to the Jump List.s
        UINT cMinSlots;
        IObjectArray *poaRemoved;
        hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));
        if (SUCCEEDED(hr))
        {
            // Adds a known category, which is filled with items collected for the automatic Jump List.
            // If an application also adds other custom categories (see the CustomJumpList sample), the categories
            // are displayed in the order they are appended to the list.  When combining custom categories with
            // known categories, duplicates are not removed, so applications should only provide items in
            // custom categories that will not appear in the known categories.
            hr = pcdl->AppendKnownCategory(fRecentSelected ? KDC_RECENT : KDC_FREQUENT);
            if (SUCCEEDED(hr))
            {
                hr = pcdl->CommitList();
            }
            poaRemoved->Release();
        }
        pcdl->Release();
    }
    return hr;
}

// Sets the visible known category and updates the menu to reflect the current state
void SetCategory(const HWND hWnd, const BOOL fRecentSelected)
{
    if (SUCCEEDED(SetKnownCategory(fRecentSelected)))
    {
        HMENU hMenu = GetMenu(hWnd);
        if (hMenu)
        {
            MENUITEMINFO mii = {};
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_FTYPE | MIIM_STATE;
            mii.fType = MFT_RADIOCHECK;
            mii.fState = MFS_ENABLED | (fRecentSelected ? MFS_CHECKED : 0);
            SetMenuItemInfo(hMenu, IDM_CATEGORY_RECENT, FALSE, &mii);
            mii.fState = MFS_ENABLED | (!fRecentSelected ? MFS_CHECKED : 0);
            SetMenuItemInfo(hMenu, IDM_CATEGORY_FREQUENT, FALSE, &mii);

            CloseHandle(hMenu);
        }

        DWORD cbRecentSelected = sizeof(fRecentSelected);
        SHSetValue(HKEY_CURRENT_USER, REGPATH_SAMPLE, REGVAL_RECENTCATEGORY, REG_DWORD, &fRecentSelected, cbRecentSelected);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_FILE_OPEN:
                OpenItem(hWnd);
                break;
            case IDM_FILE_CLEARHISTORY:
                ClearHistory();
                break;
            case IDM_FILE_DEREGISTERFILETYPES:
                {
                    CleanupSampleFiles();

                    PCWSTR pszMessage;
                    HRESULT hr = UnRegisterFileTypeHandlers();
                    if (E_ACCESSDENIED == hr)
                    {
                        pszMessage = L"Please run this application as an administrator to remove file type registrations.";
                    }
                    else if (FAILED(hr))
                    {
                        pszMessage = L"Unable to remove file type registrations.";
                    }
                    else
                    {
                        pszMessage = L"File type registrations were successfully removed.";
                    }
                    MessageBox(hWnd, pszMessage, c_szTitle, MB_OK);
                    break;
                }
            case IDM_CATEGORY_RECENT:
                SetCategory(hWnd, TRUE);
                break;
            case IDM_CATEGORY_FREQUENT:
                SetCategory(hWnd, FALSE);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pszCmdLine, int nCmdShow)
{
    g_hInst = hInstance;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        if (!AreFileTypesRegistered())
        {
            PCWSTR pszMessage = NULL;
            hr = RegisterToHandleFileTypes();
            if (E_ACCESSDENIED == hr)
            {
                pszMessage = L"Please relaunch this application as an administrator to register for the required file types.";
            }
            else if (FAILED(hr))
            {
                pszMessage = L"Unable to register the required file types.";
            }
            else
            {
                pszMessage = L"The required file types were successfully registered.";
            }
            MessageBox(NULL, pszMessage, c_szTitle, MB_OK);
        }

        if (SUCCEEDED(hr))
        {
            if (FAILED(CreateSampleFiles()))
            {
                MessageBox(NULL, L"Unable to create the sample files.", c_szTitle, MB_OK);
            }

            if (pszCmdLine && lstrlen(pszCmdLine) > 0)
            {
                MessageBox(NULL, pszCmdLine, c_szTitle, MB_OK);

                // If available, it is preferable to pass an IShellItem (SHARD_SHELLITEM) or IDList (SHARD_PIDL), as they
                // can represent items that do not have a file path.  However, parsing a path to produce an IShellItem or
                // IDList will incur I/O unnecessarily for the calling application.  Instead, if only a path is available,
                // pass it to SHAddToRecentDocs, to avoid the extraneous parsing work.
                SHAddToRecentDocs(SHARD_PATHW, pszCmdLine);
            }

            WNDCLASSEX wcex     = {sizeof(WNDCLASSEX)};
            wcex.style          = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc    = WndProc;
            wcex.cbClsExtra     = 0;
            wcex.cbWndExtra     = 0;
            wcex.hInstance      = hInstance;
            wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUTOMATICJUMPLISTSAMPLE));
            wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
            wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
            wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_AUTOMATICJUMPLISTSAMPLE);
            wcex.lpszClassName  = c_szWindowClass;

            RegisterClassEx(&wcex);

            HWND hWnd = CreateWindow(c_szWindowClass, c_szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 300, 200, NULL, NULL, hInstance, NULL);
            if (hWnd)
            {
                BOOL fRecentSelected;
                DWORD cbRecentSelected = sizeof (fRecentSelected);
                DWORD dwType;
                if (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, REGPATH_SAMPLE, REGVAL_RECENTCATEGORY, &dwType, &fRecentSelected, &cbRecentSelected))
                {
                   fRecentSelected = TRUE;
                }
                SetCategory(hWnd, fRecentSelected);

                ShowWindow(hWnd, nCmdShow);

                MSG msg;
                while (GetMessage(&msg, NULL, 0, 0))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        CoUninitialize();
    }
    return 0;
}
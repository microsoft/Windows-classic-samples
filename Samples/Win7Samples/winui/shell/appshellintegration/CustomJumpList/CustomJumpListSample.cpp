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

HINSTANCE g_hInst = NULL;

WCHAR const c_szTitle[] = L"Custom Jump List Sample";
WCHAR const c_szWindowClass[] = L"CUSTOMJUMPLISTSAMPLE";

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

// Creates a CLSID_ShellLink to insert into the Tasks section of the Jump List.  This type of Jump
// List item allows the specification of an explicit command line to execute the task.
HRESULT _CreateShellLink(PCWSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl)
{
    IShellLink *psl;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
    if (SUCCEEDED(hr))
    {
        // Determine our executable's file path so the task will execute this application
        WCHAR szAppPath[MAX_PATH];
        if (GetModuleFileName(NULL, szAppPath, ARRAYSIZE(szAppPath)))
        {
            hr = psl->SetPath(szAppPath);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetArguments(pszArguments);
                if (SUCCEEDED(hr))
                {
                    // The title property is required on Jump List items provided as an IShellLink
                    // instance.  This value is used as the display name in the Jump List.
                    IPropertyStore *pps;
                    hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
                    if (SUCCEEDED(hr))
                    {
                        PROPVARIANT propvar;
                        hr = InitPropVariantFromString(pszTitle, &propvar);
                        if (SUCCEEDED(hr))
                        {
                            hr = pps->SetValue(PKEY_Title, propvar);
                            if (SUCCEEDED(hr))
                            {
                                hr = pps->Commit();
                                if (SUCCEEDED(hr))
                                {
                                    hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
                                }
                            }
                            PropVariantClear(&propvar);
                        }
                        pps->Release();
                    }
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        psl->Release();
    }
    return hr;
}

// The Tasks category of Jump Lists supports separator items.  These are simply IShellLink instances
// that have the PKEY_AppUserModel_IsDestListSeparator property set to TRUE.  All other values are
// ignored when this property is set.
HRESULT _CreateSeparatorLink(IShellLink **ppsl)
{
    IPropertyStore *pps;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pps));
    if (SUCCEEDED(hr))
    {
        PROPVARIANT propvar;
        hr = InitPropVariantFromBoolean(TRUE, &propvar);
        if (SUCCEEDED(hr))
        {
            hr = pps->SetValue(PKEY_AppUserModel_IsDestListSeparator, propvar);
            if (SUCCEEDED(hr))
            {
                hr = pps->Commit();
                if (SUCCEEDED(hr))
                {
                    hr = pps->QueryInterface(IID_PPV_ARGS(ppsl));
                }
            }
            PropVariantClear(&propvar);
        }
        pps->Release();
    }
    return hr;
}

// Builds the collection of task items and adds them to the Task section of the Jump List.  All tasks
// should be added to the canonical "Tasks" category by calling ICustomDestinationList::AddUserTasks.
HRESULT _AddTasksToList(ICustomDestinationList *pcdl)
{
    IObjectCollection *poc;
    HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
    if (SUCCEEDED(hr))
    {
        IShellLink * psl;
        hr = _CreateShellLink(L"/Task1", L"Task 1", &psl);
        if (SUCCEEDED(hr))
        {
            hr = poc->AddObject(psl);
            psl->Release();
        }

        if (SUCCEEDED(hr))
        {
            hr = _CreateShellLink(L"/Task2", L"Second Task", &psl);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = _CreateSeparatorLink(&psl);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = _CreateShellLink(L"/Task3", L"Task 3", &psl);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            IObjectArray * poa;
            hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
            if (SUCCEEDED(hr))
            {
                // Add the tasks to the Jump List. Tasks always appear in the canonical "Tasks"
                // category that is displayed at the bottom of the Jump List, after all other
                // categories.
                hr = pcdl->AddUserTasks(poa);
                poa->Release();
            }
        }
        poc->Release();
    }
    return hr;
}

// Determines if the provided IShellItem is listed in the array of items that the user has removed
bool _IsItemInArray(IShellItem *psi, IObjectArray *poaRemoved)
{
    bool fRet = false;
    UINT cItems;
    if (SUCCEEDED(poaRemoved->GetCount(&cItems)))
    {
        IShellItem *psiCompare;
        for (UINT i = 0; !fRet && i < cItems; i++)
        {
            if (SUCCEEDED(poaRemoved->GetAt(i, IID_PPV_ARGS(&psiCompare))))
            {
                int iOrder;
                fRet = SUCCEEDED(psiCompare->Compare(psi, SICHINT_CANONICAL, &iOrder)) && (0 == iOrder);
                psiCompare->Release();
            }
        }
    }
    return fRet;
}

// Adds a custom category to the Jump List.  Each item that should be in the category is added to
// an ordered collection, and then the category is appended to the Jump List as a whole.
HRESULT _AddCategoryToList(ICustomDestinationList *pcdl, IObjectArray *poaRemoved)
{
    IObjectCollection *poc;
    HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
    if (SUCCEEDED(hr))
    {
        for (UINT i = 0; i < ARRAYSIZE(c_rgpszFiles); i++)
        {
            IShellItem *psi;
            if (SUCCEEDED(SHCreateItemInKnownFolder(FOLDERID_Documents, KF_FLAG_DEFAULT, c_rgpszFiles[i], IID_PPV_ARGS(&psi))))
            {
                // Items listed in the removed list may not be re-added to the Jump List during this
                // list-building transaction.  They should not be re-added to the Jump List until
                // the user has used the item again.  The AppendCategory call below will fail if
                // an attempt to add an item in the removed list is made.
                if (!_IsItemInArray(psi, poaRemoved))
                {
                    poc->AddObject(psi);
                }
                psi->Release();
            }
        }

        IObjectArray *poa;
        hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
        if (SUCCEEDED(hr))
        {
            // Add the category to the Jump List.  If there were more categories, they would appear
            // from top to bottom in the order they were appended.
            hr = pcdl->AppendCategory(L"Custom Category", poa);
            poa->Release();
        }
        poc->Release();
    }
    return hr;
}

// Builds a new custom Jump List for this application.
void CreateJumpList()
{
    // Create the custom Jump List object.
    ICustomDestinationList *pcdl;
    HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
    if (SUCCEEDED(hr))
    {
        // Custom Jump Lists follow a push model - applications are responsible for providing an updated
        // list anytime the contents should be changed.  Lists are generated in a list-building
        // transaction that starts by calling BeginList.  Until the list is committed, Windows will
        // display the previous version of the list, if available.
        //
        // The cMinSlots out parameter indicates the minimum number of items that the Jump List UI is
        // guaranteed to display.  Applications can provide more items when building a custom Jump List,
        // but the extra items may not be displayed.  The number is dependant upon a number of factors,
        // such as screen resolution and the "Number of recent items to display in Jump Lists" user setting.
        // See the MSDN documentation on BeginList for more information.
        //
        // The IObjectArray returned from BeginList contains a list of items the user has chosen to remove
        // from their Jump List.  Applications must respect the user's removal of an item and not re-add any
        // item in the removed list during this list-building transaction.  Applications should also clear any
        // persited usage-tracking data for any item in the removed list.  If the user begins using a
        // previously removed item in the future, it may be re-added to the list.
        UINT cMinSlots;
        IObjectArray *poaRemoved;
        hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));
        if (SUCCEEDED(hr))
        {
            // Add content to the Jump List.
            hr = _AddCategoryToList(pcdl, poaRemoved);
            if (SUCCEEDED(hr))
            {
                hr = _AddTasksToList(pcdl);
                if (SUCCEEDED(hr))
                {
                    // Commit the list-building transaction.
                    hr = pcdl->CommitList();
                }
            }
            poaRemoved->Release();
        }
        pcdl->Release();
    }
}

// Removes that existing custom Jump List for this application.
void DeleteJumpList()
{
    ICustomDestinationList *pcdl;
    HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
    if (SUCCEEDED(hr))
    {
        hr = pcdl->DeleteList(NULL);
        pcdl->Release();
    }
}

// Window proc for this application.
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

            case IDM_FILE_CREATECUSTOMJUMPLIST:
                CreateJumpList();
                break;

            case IDM_FILE_DELETECUSTOMJUMPLIST:
                DeleteJumpList();
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
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

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

            if (pszCmdLine && *pszCmdLine)
            {
                MessageBox(NULL, pszCmdLine, c_szTitle, MB_OK);
            }

            WNDCLASSEX wcex     = {sizeof(wcex)};
            wcex.style          = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc    = WndProc;
            wcex.hInstance      = hInstance;
            wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CUSTOMJUMPLISTSAMPLE));
            wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
            wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
            wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_CUSTOMJUMPLISTSAMPLE);
            wcex.lpszClassName  = c_szWindowClass;

            RegisterClassEx(&wcex);

            HWND hWnd = CreateWindow(c_szWindowClass, c_szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 300, 200, NULL, NULL, hInstance, NULL);
            if (hWnd)
            {
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

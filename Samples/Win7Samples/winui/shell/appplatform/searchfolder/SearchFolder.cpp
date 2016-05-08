// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#define STRICT_TYPED_ITEMIDS
#include <windows.h>
#include <structuredquery.h>
#include <strsafe.h>
#include <shlobj.h>
#include <propkey.h>
#include <propvarutil.h>

// since this sample displays UI set the manifest to pick the right version of comctl32.dll
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Creates an instance of ICondition for the predicate "Kind=Document AND Size>10240"
HRESULT GetCondition(ICondition **ppCondition)
{
    *ppCondition = NULL;

    // Create the condition factory.  This interface helps create conditions.
    IConditionFactory2 *pConditionFactory;
    HRESULT hr = CoCreateInstance(CLSID_ConditionFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pConditionFactory));
    if (SUCCEEDED(hr))
    {
        ICondition *pConditionKind;
        hr = pConditionFactory->CreateStringLeaf(PKEY_Kind, COP_EQUAL, L"Document", NULL, CONDITION_CREATION_DEFAULT, IID_PPV_ARGS(&pConditionKind));
        if (SUCCEEDED(hr))
        {
            ICondition *pConditionSize;
            hr = pConditionFactory->CreateIntegerLeaf(PKEY_Size, COP_GREATERTHAN, 10240, CONDITION_CREATION_DEFAULT, IID_PPV_ARGS(&pConditionSize));
            if (SUCCEEDED(hr))
            {
                // Once all of the leaf conditions are created successfully, "AND" them together
                ICondition *rgConditions[] = {pConditionKind, pConditionSize};
                hr = pConditionFactory->CreateCompoundFromArray(CT_AND_CONDITION, rgConditions, ARRAYSIZE(rgConditions), CONDITION_CREATION_DEFAULT, IID_PPV_ARGS(ppCondition));
                pConditionSize->Release();
            }
            pConditionKind->Release();
        }
        pConditionFactory->Release();
    }
    return hr;
}

// This opens up the common file dialog to an IShellItem and waits for the user to select a file from the results.
// It then displays the name of the selected item in a message box.
HRESULT OpenCommonFileDialogTo(IShellItem *pShellItemSearch)
{
    // Create an instance of IFileOpenDialog
    IFileDialog* pFileDialog;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
    if (SUCCEEDED(hr))
    {
        // Set it to the folder we want to show
        hr = pFileDialog->SetFolder(pShellItemSearch);
        if (SUCCEEDED(hr))
        {
            // Show the File Dialog
            hr = pFileDialog->Show(NULL);
            if (SUCCEEDED(hr))
            {
                // Now get the file that the user selected
                IShellItem *pShellItemSelected;
                hr = pFileDialog->GetResult(&pShellItemSelected);
                if (SUCCEEDED(hr))
                {
                    // Get the name from that file
                    PWSTR pszName;
                    hr = pShellItemSelected->GetDisplayName(SIGDN_NORMALDISPLAY, &pszName);
                    if (SUCCEEDED(hr))
                    {
                        // Display it back to the user
                        WCHAR szMsg[128];
                        StringCchPrintf(szMsg, ARRAYSIZE(szMsg), L"You Chose '%s'\r", pszName);

                        MessageBox(NULL, szMsg, L"Search Folder Sample", MB_OK);
                        CoTaskMemFree(pszName);
                    }
                    pShellItemSelected->Release();
                }
            }
        }
        pFileDialog->Release();
    }
    return hr;
}

// Create a shell item array object that can be accessed using IObjectCollection
// or IShellItemArray. IObjectCollection lets items be added or removed from the collection.
// For code that needs to run on Vista use SHCreateShellItemArrayFromIDLists()
HRESULT CreateShellItemArray(REFIID riid, void **ppv)
{
    *ppv = NULL;
    IShellLibrary *pLibrary;
    HRESULT hr = CoCreateInstance(CLSID_ShellLibrary, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pLibrary));
    if (SUCCEEDED(hr))
    {
        hr = pLibrary->GetFolders(LFF_ALLITEMS, riid, ppv);
        pLibrary->Release();
    }
    return hr;
}

// This helper creates the scope object that is a collection of shell items that
// define where the search will operate.
HRESULT CreateScope(IShellItemArray **ppsia)
{
    *ppsia = NULL;

    IObjectCollection *pObjects;
    HRESULT hr = CreateShellItemArray(IID_PPV_ARGS(&pObjects));
    if (SUCCEEDED(hr))
    {
        IShellItem *psi;
        if (SUCCEEDED(SHCreateItemInKnownFolder(FOLDERID_DocumentsLibrary, 0, NULL, IID_PPV_ARGS(&psi))))
        {
            pObjects->AddObject(psi);
            psi->Release();
        }

        // Other items can be added to pObjects similar to the code above.

        hr = pObjects->QueryInterface(ppsia);

        pObjects->Release();
    }
    return hr;
}

// This program is a sample of how to use the ISearchFolderItemFactory interface to create search folders.
// It creates a search and then opens up the Common File Dialog displaying the results of the search.

int wmain()
{
    // File dialog drag and drop feature requires OLE
    HRESULT hr = OleInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        ISearchFolderItemFactory *pSearchFolderItemFactory;
        hr = CoCreateInstance(CLSID_SearchFolderItemFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSearchFolderItemFactory));
        if (SUCCEEDED(hr))
        {
            IShellItemArray *psiaScope;
            hr = CreateScope(&psiaScope);
            if (SUCCEEDED(hr))
            {
                hr = pSearchFolderItemFactory->SetScope(psiaScope);
                if (SUCCEEDED(hr))
                {
                    // Sets the display name of the search
                    hr = pSearchFolderItemFactory->SetDisplayName(L"Sample Query");
                    if (SUCCEEDED(hr))
                    {
                        ICondition* pCondition;
                        hr = GetCondition(&pCondition);
                        if (SUCCEEDED(hr))
                        {
                            // Sets the condition for pSearchFolderItemFactory
                            hr = pSearchFolderItemFactory->SetCondition(pCondition);
                            if (SUCCEEDED(hr))
                            {
                                // This retrieves an IShellItem of the search.  It is a virtual child of the desktop.
                                IShellItem* pShellItemSearch;
                                hr = pSearchFolderItemFactory->GetShellItem(IID_PPV_ARGS(&pShellItemSearch));
                                if (SUCCEEDED(hr))
                                {
                                    OpenCommonFileDialogTo(pShellItemSearch);
                                    pShellItemSearch->Release();
                                }
                            }
                            pCondition->Release();
                        }
                    }
                }
                psiaScope->Release();
            }
            pSearchFolderItemFactory->Release();
        }
        OleUninitialize();
    }
    return 0;
}

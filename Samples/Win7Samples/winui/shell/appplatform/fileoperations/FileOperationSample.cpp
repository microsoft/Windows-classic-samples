// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#define STRICT_TYPED_ITEMIDS

#include <windows.h>      // Standard include
#include <Shellapi.h>     // Included for shell constants such as FO_* values
#include <shlobj.h>       // Required for necessary shell dependencies
#include <strsafe.h>      // Including StringCch* helpers

wchar_t const c_szSampleFileName[] = L"SampleFile";
wchar_t const c_szSampleFileNewname[] = L"NewName";
wchar_t const c_szSampleFileExt[] = L"txt";
wchar_t const c_szSampleSrcDir[] = L"FileOpSampleSource";
wchar_t const c_szSampleDstDir[] = L"FileOpSampleDestination";
const UINT c_cMaxFilesToCreate = 10;

HRESULT CreateAndInitializeFileOperation(REFIID riid, void **ppv)
{
    *ppv = NULL;
    // Create the IFileOperation object
    IFileOperation *pfo;
    HRESULT hr = CoCreateInstance(__uuidof(FileOperation), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
    if (SUCCEEDED(hr))
    {
        // Set the operation flags.  Turn off  all UI
        // from being shown to the user during the
        // operation.  This includes error, confirmation
        // and progress dialogs.
        hr = pfo->SetOperationFlags(FOF_NO_UI);
        if (SUCCEEDED(hr))
        {
            hr = pfo->QueryInterface(riid, ppv);
        }
        pfo->Release();
    }
    return hr;
}

//  Synopsis:  Create the source and destination folders for the sample
//
//  Arguments: psiSampleRoot  - Item of the parent folder where the sample folders will be created
//             ppsiSampleSrc  - On success contains the source folder item to be used for sample operations
//             ppsiSampleDst  - On success contains the destination folder item to be used for sample operations
//
//  Returns:   S_OK if successful
HRESULT CreateSampleFolders(IShellItem *psiSampleRoot, IShellItem **ppsiSampleSrc, IShellItem **ppsiSampleDst)
{
    *ppsiSampleSrc = NULL;
    *ppsiSampleDst = NULL;
    IFileOperation *pfo;
    HRESULT hr = CreateAndInitializeFileOperation(IID_PPV_ARGS(&pfo));
    if (SUCCEEDED(hr))
    {
        // Use the file operation to create a source and destination folder
        hr = pfo->NewItem(psiSampleRoot, FILE_ATTRIBUTE_DIRECTORY, c_szSampleSrcDir, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            hr = pfo->NewItem(psiSampleRoot, FILE_ATTRIBUTE_DIRECTORY, c_szSampleDstDir, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                hr = pfo->PerformOperations();
                if (SUCCEEDED(hr))
                {
                    // Now that the folders have been created, create items for them.  This is just an optimization so
                    // that the sample does not have to rebind to these items for each sample type.
                    IShellItem *psiSrc = NULL;
                    IShellItem *psiDst = NULL;
                    hr = SHCreateItemFromRelativeName(psiSampleRoot, c_szSampleSrcDir, NULL, IID_PPV_ARGS(&psiSrc));
                    if (SUCCEEDED(hr))
                    {
                        hr = SHCreateItemFromRelativeName(psiSampleRoot, c_szSampleDstDir, NULL, IID_PPV_ARGS(&psiDst));
                        if (SUCCEEDED(hr))
                        {
                            *ppsiSampleSrc = psiSrc;
                            *ppsiSampleDst = psiDst;
                            // Caller takes ownership
                            psiSrc = NULL;
                            psiDst = NULL;
                        }
                    }
                    if (psiSrc)
                    {
                        psiSrc->Release();
                    }
                    if (psiDst)
                    {
                        psiDst->Release();
                    }
                }
            }
        }
    }
    return hr;
}

//  Synopsis:  Creates all of the files needed by this sample the requested known folder
//
//  Arguments: psiFolder  - Folder that will contain the sample files
//
//  Returns:   S_OK if successful
HRESULT CreateSampleFiles(IShellItem *psiFolder)
{
    IFileOperation *pfo;
    HRESULT hr = CreateAndInitializeFileOperation(IID_PPV_ARGS(&pfo));
    if (SUCCEEDED(hr))
    {
        WCHAR szSampleFileName[MAX_PATH];
        hr = StringCchPrintfW(szSampleFileName, ARRAYSIZE(szSampleFileName), L"%s.%s", c_szSampleFileName, c_szSampleFileExt);
        if (SUCCEEDED(hr))
        {
            // the file to be used for the single copy sample
            hr = pfo->NewItem(psiFolder, FILE_ATTRIBUTE_NORMAL, szSampleFileName, NULL, NULL);
            // the files to be used for the multiple copy sample
            for (UINT i = 0; SUCCEEDED(hr) && i < c_cMaxFilesToCreate; i++)
            {
                hr = StringCchPrintfW(szSampleFileName, ARRAYSIZE(szSampleFileName), L"%s%u.%s", c_szSampleFileName, i, c_szSampleFileExt);
                if (SUCCEEDED(hr))
                {
                    hr = pfo->NewItem(psiFolder, FILE_ATTRIBUTE_NORMAL, szSampleFileName, NULL, NULL);
                }
            }
            if (SUCCEEDED(hr))
            {
                hr = pfo->PerformOperations();
            }
        }
        pfo->Release();
    }
    return hr;
}

//  Synopsis:  Deletes the files/folders created by this sample
//
//  Arguments: psiSrc  - Source folder item
//             psiDst  - Destination folder item
//
//  Returns:   S_OK if successful
HRESULT DeleteSampleFiles(IShellItem *psiSrc, IShellItem *psiDst)
{
    IFileOperation *pfo;
    HRESULT hr = CreateAndInitializeFileOperation(IID_PPV_ARGS(&pfo));
    if (SUCCEEDED(hr))
    {
        hr = pfo->DeleteItem(psiSrc, NULL);
        if (SUCCEEDED(hr))
        {
            hr = pfo->DeleteItem(psiDst, NULL);
            if (SUCCEEDED(hr))
            {
                hr = pfo->PerformOperations();
            }
        }
        pfo->Release();
    }
    return hr;
}

//  Synopsis:  This example copies a single item from the sample source folder
//             to the sample dest folder using a new item name.
//
//  Arguments: psiSrc  - Source folder item
//             psiDst  - Destination folder item
//
//  Returns:   S_OK if successful
HRESULT CopySingleFile(IShellItem *psiSrc, IShellItem *psiDst)
{
    // Create the IFileOperation object
    IFileOperation *pfo;
    HRESULT hr = CreateAndInitializeFileOperation(IID_PPV_ARGS(&pfo));
    if (SUCCEEDED(hr))
    {
        WCHAR szSampleFileName[MAX_PATH];
        hr = StringCchPrintfW(szSampleFileName, ARRAYSIZE(szSampleFileName), L"%s.%s", c_szSampleFileName, c_szSampleFileExt);
        if (SUCCEEDED(hr))
        {
            IShellItem *psiSrcFile;
            hr = SHCreateItemFromRelativeName(psiSrc, szSampleFileName, NULL, IID_PPV_ARGS(&psiSrcFile));
            if (SUCCEEDED(hr))
            {
                WCHAR szNewName[MAX_PATH];
                hr = StringCchPrintfW(szNewName, ARRAYSIZE(szNewName), L"%s.%s", c_szSampleFileNewname, c_szSampleFileExt);
                if (SUCCEEDED(hr))
                {
                    hr = pfo->CopyItem(psiSrcFile, psiDst, szNewName, NULL);
                    if (SUCCEEDED(hr))
                    {
                        hr = pfo->PerformOperations();
                    }
                }
                psiSrcFile->Release();
            }
        }
        pfo->Release();
    }
    return hr;
}

//  Synopsis:  Creates an IShellItemArray containing the sample files to be used
//             in the CopyMultipleFiles sample
//
//  Arguments: psiSrc  - Source folder item
//
//  Returns:   S_OK if successful
HRESULT CreateShellItemArrayOfSampleFiles(IShellItem *psiSrc, REFIID riid, void **ppv)
{
    *ppv = NULL;
    IShellFolder *psfSampleSrc;
    HRESULT hr = psiSrc->BindToHandler(NULL, BHID_SFObject, IID_PPV_ARGS(&psfSampleSrc));
    if (SUCCEEDED(hr))
    {
        PITEMID_CHILD rgpidlChildren[c_cMaxFilesToCreate] = {0};
        for (UINT i = 0; SUCCEEDED(hr) && i < ARRAYSIZE(rgpidlChildren); i++)
        {
            WCHAR szSampleFileName[MAX_PATH];
            hr = StringCchPrintfW(szSampleFileName, ARRAYSIZE(szSampleFileName), L"%s%u.%s", c_szSampleFileName, i, c_szSampleFileExt);
            if (SUCCEEDED(hr))
            {
                hr = psfSampleSrc->ParseDisplayName(NULL, NULL, szSampleFileName, NULL, (PIDLIST_RELATIVE *)&rgpidlChildren[i], NULL);
            }
        }
        if (SUCCEEDED(hr))
        {
            IShellItemArray *psia;
            hr = SHCreateShellItemArray(NULL, psfSampleSrc, c_cMaxFilesToCreate, &rgpidlChildren[0], &psia);
            if (SUCCEEDED(hr))
            {
                hr = psia->QueryInterface(riid, ppv);
                psia->Release();
            }
        }
        for (UINT i = 0; i < ARRAYSIZE(rgpidlChildren); i++)
        {
            CoTaskMemFree(rgpidlChildren[i]);
        }
        psfSampleSrc->Release();
    }
    return hr;
}

//  Synopsis:  This example creates multiple files under the specified folder
//             path and copies them to the same directory with a new name.
//
//  Arguments: psiSrc  - Source folder item
//             psiDst  - Destination folder item
//
//  Returns:   S_OK if successful
HRESULT CopyMultipleFiles(IShellItem *psiSrc, IShellItem *psiDst)
{
    // Create the IFileOperation object
    IFileOperation *pfo;
    HRESULT hr = CreateAndInitializeFileOperation(IID_PPV_ARGS(&pfo));
    if (SUCCEEDED(hr))
    {
        IShellItemArray *psiaSampleFiles;
        hr = CreateShellItemArrayOfSampleFiles(psiSrc, IID_PPV_ARGS(&psiaSampleFiles));
        if (SUCCEEDED(hr))
        {
            hr = pfo->CopyItems(psiaSampleFiles, psiDst);
            if (SUCCEEDED(hr))
            {
                hr = pfo->PerformOperations();
            }
            psiaSampleFiles->Release();
        }
        pfo->Release();
    }
    return hr;
}

int wmain()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        // Get the documents known folder.  This folder will be used to create subfolders
        // for the sample source and destination
        IShellItem *psiDocuments;
        hr = SHCreateItemInKnownFolder(FOLDERID_Documents, KF_FLAG_DEFAULT_PATH, NULL, IID_PPV_ARGS(&psiDocuments));
        if (SUCCEEDED(hr))
        {
            IShellItem *psiSampleSrc;
            IShellItem *psiSampleDst;
            hr = CreateSampleFolders(psiDocuments, &psiSampleSrc, &psiSampleDst);
            if (SUCCEEDED(hr))
            {
                hr = CreateSampleFiles(psiSampleSrc);
                if (SUCCEEDED(hr))
                {
                    hr = CopySingleFile(psiSampleSrc, psiSampleDst);
                    if (SUCCEEDED(hr))
                    {
                        hr = CopyMultipleFiles(psiSampleSrc, psiSampleDst);
                    }
                }
                DeleteSampleFiles(psiSampleSrc, psiSampleDst);

                psiSampleSrc->Release();
                psiSampleDst->Release();
            }
            psiDocuments->Release();
        }
        CoUninitialize();
    }
    return 0;
}

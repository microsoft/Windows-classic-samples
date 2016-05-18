// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#define STRICT_TYPED_ITEMIDS

#define NTDDI_VERSION NTDDI_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>    // IStream helpers

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' "\
                       "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define IDC_DOCUMENTSLIBRARY    101
#define IDC_FOLDERPICKER        102
#define IDC_SHBROWSEFORFOLDER   103
#define IDC_FILEDIALOGITEM      104

const wchar_t c_szSampleFolderName[] =   L"ShellStorageSample";
const wchar_t c_szSampleFileName[] =     L"ShellStorageSample.txt";
const wchar_t c_szSampleFileContents[] = L"This sample file created by the ShellStorage SDK sample";

HRESULT CreateFileInContainer(IShellItem *psi, PCWSTR pszFileName, PCWSTR pszContents)
{
    IStorage *pstorage;
    HRESULT hr = psi->BindToHandler(NULL, BHID_Storage, IID_PPV_ARGS(&pstorage));
    if (SUCCEEDED(hr))
    {
        IStream *pstream;
        hr = pstorage->CreateStream(pszFileName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
                                    0, 0, &pstream);
        if (SUCCEEDED(hr))
        {
            hr = IStream_Write(pstream, pszContents, sizeof(*pszContents) * lstrlen(pszContents));
            if (SUCCEEDED(hr))
            {
                hr = pstream->Commit(STGC_OVERWRITE);
            }
            pstream->Release();
        }
        pstorage->Release();
    }
    return hr;
}

// Same as above but creates a folder
HRESULT CreateFolderInContainer(IShellItem *psi, PCWSTR pszFolderName)
{
    IStorage *pstorage;
    HRESULT hr = psi->BindToHandler(NULL, BHID_Storage, IID_PPV_ARGS(&pstorage));
    if (SUCCEEDED(hr))
    {
        IStorage *pnewstorage;
        hr = pstorage->CreateStorage(pszFolderName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
                                     0, 0, &pnewstorage);
        if (SUCCEEDED(hr))
        {
            hr = pnewstorage->Commit(STGC_OVERWRITE);
            pnewstorage->Release();
        }
        pstorage->Release();
    }
    return hr;
}

HRESULT CreateBindCtxWithMode(DWORD grfMode, IBindCtx** ppbc)
{
    HRESULT hr = CreateBindCtx(0, ppbc);
    if(SUCCEEDED(hr))
    {
        BIND_OPTS boptions = {sizeof(BIND_OPTS), 0, grfMode, 0};
        (*ppbc)->SetBindOptions(&boptions);
    }
    return hr;
}

// Writes to a given file
HRESULT CreateFileFromItem(IShellItem *psi, PCWSTR pszContents)
{
    IBindCtx *pbc;
    HRESULT hr = CreateBindCtxWithMode(STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, &pbc);
    if (SUCCEEDED(hr))
    {
        IStream *pstream;
        hr = psi->BindToHandler(pbc, BHID_Stream, IID_PPV_ARGS(&pstream));
        if (SUCCEEDED(hr))
        {
            hr = IStream_Write(pstream, pszContents, sizeof(*pszContents) * lstrlen(pszContents));
            if (SUCCEEDED(hr))
            {
                hr = pstream->Commit(STGC_OVERWRITE);
            }
            pstream->Release();
        }
        pbc->Release();
    }
    return hr;
}

HRESULT ExportToDocumentsLibrary()
{
    IShellItem *psi;
    HRESULT hr = SHCreateItemInKnownFolder(FOLDERID_DocumentsLibrary, 0, NULL, IID_PPV_ARGS(&psi));

    if (FAILED(hr))
    {
        // Vista does not have a Documents Library, use the Documents Folder
        hr = SHCreateItemInKnownFolder(FOLDERID_Documents, 0, NULL, IID_PPV_ARGS(&psi));
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateFileInContainer(psi, c_szSampleFileName, c_szSampleFileContents);
        if (SUCCEEDED(hr))
        {
            hr = CreateFolderInContainer(psi, c_szSampleFolderName);
        }
        psi->Release();
    }
    return hr;
}

HRESULT ExportToFolderPicker()
{
    IFileOpenDialog *pfod;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pfod));
    if (SUCCEEDED(hr))
    {
        hr = pfod->SetOptions(FOS_PICKFOLDERS);
        if (SUCCEEDED(hr))
        {
            hr = pfod->Show(NULL);
            if (SUCCEEDED(hr))
            {
                IShellItem *psi;
                hr = pfod->GetResult(&psi);
                if (SUCCEEDED(hr))
                {
                    hr = CreateFileInContainer(psi, c_szSampleFileName, c_szSampleFileContents);
                    if (SUCCEEDED(hr))
                    {
                        hr = CreateFolderInContainer(psi, c_szSampleFolderName);
                    }
                    psi->Release();
                }
            }
        }
        pfod->Release();
    }
    return hr;
}

HRESULT ExportToSHBrowseForFolder()
{
    BROWSEINFO bi = {};
    bi.ulFlags = BIF_USENEWUI;

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);
    HRESULT hr = pidl ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        IShellItem *psi;
        hr = SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            hr = CreateFileInContainer(psi, c_szSampleFileName, c_szSampleFileContents);
            if (SUCCEEDED(hr))
            {
                hr = CreateFolderInContainer(psi, c_szSampleFolderName);
            }
            psi->Release();
        }
        CoTaskMemFree(pidl);
    }
    return hr;
}

HRESULT ExportToFileDialogItem()
{
    IFileSaveDialog *pfsd;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pfsd));
    if (SUCCEEDED(hr))
    {
        hr = pfsd->SetFileName(c_szSampleFileName);
        if (SUCCEEDED(hr))
        {
            COMDLG_FILTERSPEC const rgSaveTypes[] =
            {
                { L"Text Documents", L"*.txt" },
                { L"All Files", L"*.*" },
            };

            hr = pfsd->SetFileTypes(ARRAYSIZE(rgSaveTypes), rgSaveTypes);
            if (SUCCEEDED(hr))
            {
                hr = pfsd->Show(NULL);
                if (SUCCEEDED(hr))
                {
                    IShellItem *psi;
                    hr = pfsd->GetResult(&psi);
                    if (SUCCEEDED(hr))
                    {
                        hr = CreateFileFromItem(psi, c_szSampleFileContents);
                        psi->Release();
                    }
                }
            }
        }
        pfsd->Release();
    }
    return hr;
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        TASKDIALOGCONFIG taskDialogParams = { sizeof(taskDialogParams) };
        taskDialogParams.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION;

        const TASKDIALOG_BUTTON buttons[] =
        {
            // For Vista save in the Documents folder
            { IDC_DOCUMENTSLIBRARY,  L"Documents Library" },
            { IDC_FOLDERPICKER,      L"Pick using Folder Picker..." },
            { IDC_SHBROWSEFORFOLDER, L"Pick using SHBrowseForFolder..." },
            { IDC_FILEDIALOGITEM,    L"Create one item using save dialog..." }
        };

        taskDialogParams.pButtons = buttons;
        taskDialogParams.dwCommonButtons = TDCBF_CLOSE_BUTTON;
        taskDialogParams.cButtons = ARRAYSIZE(buttons);
        taskDialogParams.pszMainInstruction = L"Select where to create items";
        taskDialogParams.pszWindowTitle =     L"Shell Storage Sample";

        int selectedId;
        BOOL fDone = FALSE;
        while (!fDone)
        {
            hr = TaskDialogIndirect(&taskDialogParams, &selectedId, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                if (selectedId == IDCANCEL || selectedId == IDCLOSE)
                {
                    fDone = TRUE;
                }
                else if (selectedId == IDC_DOCUMENTSLIBRARY)
                {
                    ExportToDocumentsLibrary();
                }
                else if (selectedId == IDC_FOLDERPICKER)
                {
                    ExportToFolderPicker();
                }
                else if (selectedId == IDC_SHBROWSEFORFOLDER)
                {
                    ExportToSHBrowseForFolder();
                }
                else if (selectedId == IDC_FILEDIALOGITEM)
                {
                    ExportToFileDialogItem();
                }
            }
        }
        CoUninitialize();
    }
    return 0;
}

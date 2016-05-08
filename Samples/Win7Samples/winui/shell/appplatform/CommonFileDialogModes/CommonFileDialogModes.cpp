// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#define STRICT_TYPED_ITEMIDS
#include <shlobj.h>
#include <shlwapi.h>
#include "ShellHelpers.h"
#include <strsafe.h>
#include <new>

const DWORD c_idAdd     = 601;
const DWORD c_idDone    = 602;

#define IDC_PICKITEM              100
#define IDC_PICKCONTAINER         101
#define IDC_FILEOPENBASKETPICKER  102
#define IDC_PICKFILESANDFOLDERS   103

/* Utility Classes and Functions *************************************************************************************************/

/*
    Usage:

    CItemIterator itemIterator(psi);

    while (itemIterator.MoveNext())
    {
        IShellItem2 *psi;
        hr = itemIterator.GetCurrent(IID_PPV_ARGS(&psi));
        if (SUCCEEDED(hr))
        {
            // Perform action on psi
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
                _pidlRel = _pidlFull;   // First item - Might be empty if it is the desktop
            }
            else if (!ILIsEmpty(_pidlRel))
            {
                PCUITEMID_CHILD pidlChild = (PCUITEMID_CHILD)_pidlRel;  // Save the current segment for binding
                _pidlRel = ILNext(_pidlRel);

                // If we are not at the end setup for the next iteration
                if (!ILIsEmpty(_pidlRel))
                {
                    const WORD cbSave = _pidlRel->mkid.cb;  // Avoid cloning for the child by truncating temporarily
                    _pidlRel->mkid.cb = 0;                  // Make this a child

                    IShellFolder *psfNew;
                    _hr = _psfCur->BindToObject(pidlChild, NULL, IID_PPV_ARGS(&psfNew));
                    if (SUCCEEDED(_hr))
                    {
                        _psfCur->Release();
                        _psfCur = psfNew;   // Transfer ownership
                        fMoreItems = true;
                    }

                    _pidlRel->mkid.cb = cbSave; // Restore previous ID size
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
            // Create the childID by truncating _pidlRel temporarily
            PUIDLIST_RELATIVE pidlNext = ILNext(_pidlRel);
            const WORD cbSave = pidlNext->mkid.cb;  // Save old cb
            pidlNext->mkid.cb = 0;                  // Make _pidlRel a child

            _hr = SHCreateItemWithParent(NULL, _psfCur, (PCUITEMID_CHILD)_pidlRel, riid, ppv);

            pidlNext->mkid.cb = cbSave;             // Restore old cb
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
                // Ignore errors, this is for debugging only
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

void DeletePerUserDialogState()
{
    IFileDialog *pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Delete window size, MRU and other saved data for testing initial case
        pfd->ClearClientData();
        pfd->Release();
    }
}

void ReportSelectedItems(IUnknown *punkSite, IShellItemArray *psia)
{
    DWORD cItems;
    HRESULT hr = psia->GetCount(&cItems);
    for (DWORD i = 0; SUCCEEDED(hr) && (i < cItems); i++)
    {
        IShellItem *psi;
        hr = psia->GetItemAt(i, &psi);
        if (SUCCEEDED(hr))
        {
            PWSTR pszName;
            hr = GetIDListName(psi, &pszName);
            if (SUCCEEDED(hr))
            {
                HWND hwnd;
                IUnknown_GetWindow(punkSite, &hwnd);
                int nButton;
                const TASKDIALOG_COMMON_BUTTON_FLAGS buttonFlags = (i == (cItems - 1)) ? TDCBF_OK_BUTTON : TDCBF_OK_BUTTON | TDCBF_CANCEL_BUTTON;
                WCHAR szMsg[128];
                StringCchPrintf(szMsg, ARRAYSIZE(szMsg), L"Item %d of %d added to basket", i + 1, cItems);
                if (SUCCEEDED(TaskDialog(hwnd, 0, L"Items Addded to Basket", szMsg, pszName, buttonFlags, NULL, &nButton)))
                {
                    hr = (nButton == IDCANCEL) ? HRESULT_FROM_WIN32(ERROR_CANCELLED) : S_OK;
                }
                CoTaskMemFree(pszName);
            }
            psi->Release();
        }
    }
}

void ReportSelectedItemsFromSite(IUnknown *punkSite)
{
    IShellItemArray *psia;
    HRESULT hr = GetSelectionFromSite(punkSite, TRUE, &psia);
    if (SUCCEEDED(hr))
    {
        ReportSelectedItems(punkSite, psia);
        psia->Release();
    }
}

/* Picking a file ****************************************************************************************************************/

void PickItem()
{
    IFileDialog *pfd;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        if (SUCCEEDED(pfd->Show(NULL)))
        {
            IShellItem *psi;
            if (SUCCEEDED(pfd->GetResult(&psi)))
            {
                PWSTR pszPath;
                if (SUCCEEDED(GetIDListName(psi, &pszPath)))
                {
                    MessageBox(NULL, pszPath, L"Selected Item", MB_OK);
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
}

/* Picking a container ***********************************************************************************************************/

void PickContainer()
{
    IFileDialog *pfd;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
        {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        if (SUCCEEDED(pfd->Show(NULL)))
        {
            IShellItem *psi;
            if (SUCCEEDED(pfd->GetResult(&psi)))
            {
                PWSTR pszPath;
                if (SUCCEEDED(GetIDListName(psi, &pszPath)))
                {
                    MessageBox(NULL, pszPath, L"Selected Container", MB_OK);
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
}

/* Picking Files in Basket Mode **************************************************************************************************/

class CFileOpenBasketPickerCallback : public IFileDialogEvents, public IFileDialogControlEvents
{
public:
    CFileOpenBasketPickerCallback()
    {
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CFileOpenBasketPickerCallback, IFileDialogEvents),
            QITABENT(CFileOpenBasketPickerCallback, IFileDialogControlEvents),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    // This class makes special assumptions about how it is used, specifically
    // 1) This class will only reside on the stack.
    // 2) Components that consume this object have well-defined reference lifetimes.
    //    In this case, this is only consumed by the file dialog advise and unadvise.
    //    Unadvising will release the file dialog's only reference to this object.
    //
    // Do not do this for heap allocated objects.
    IFACEMETHODIMP_(ULONG) AddRef()  { return 3; }
    IFACEMETHODIMP_(ULONG) Release() { return 2; }

    // IFileDialogEvents
    IFACEMETHODIMP OnFileOk(IFileDialog *pfd)
    {
        // if this button is in the "Add" mode then do this, otherwise return S_OK
        IFileOpenDialog *pfod;
        HRESULT hr = pfd->QueryInterface(IID_PPV_ARGS(&pfod));
        if (SUCCEEDED(hr))
        {
            IShellItemArray *psia;
            hr = pfod->GetSelectedItems(&psia);
            if (SUCCEEDED(hr))
            {
                ReportSelectedItems(pfd, psia);
                psia->Release();
            }
            pfod->Release();
        }
        return S_FALSE; // S_FALSE keeps the dialog up; return S_OK to allow it to dismiss.
    }

    IFACEMETHODIMP OnFolderChanging(IFileDialog * /* pfd */, IShellItem * /* psi */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnFolderChange(IFileDialog * /* pfd */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnSelectionChange(IFileDialog *pfd)
    {
        // Update the text of the Open/Add button here based on the selection
        IShellItem *psi;
        HRESULT hr = pfd->GetCurrentSelection(&psi);
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
                pfd->SetOkButtonLabel(L"Add");
            }
            psi->Release();
        }
        return S_OK;
    }

    IFACEMETHODIMP OnShareViolation(IFileDialog * /* pfd */, IShellItem * /* psi */, FDE_SHAREVIOLATION_RESPONSE * /* pResponse */) { return E_NOTIMPL; }
    IFACEMETHODIMP OnTypeChange(IFileDialog * /* pfd */) { return E_NOTIMPL; }
    IFACEMETHODIMP OnOverwrite(IFileDialog * /* pfd */, IShellItem * /* psi */, FDE_OVERWRITE_RESPONSE * /* pResponse */) { return E_NOTIMPL;}

    // IFileDialogControlEvents
    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize * /* pfdc */, DWORD /* dwIDCtl */, DWORD /* dwIDItem */)  { return E_NOTIMPL; }

    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *pfdc, DWORD dwIDCtl)
    {
        switch (dwIDCtl)
        {
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

    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize * /* pfdc */, DWORD /* dwIDCtl */, BOOL /* bChecked */) { return E_NOTIMPL; }
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize * /* pfdc */, DWORD /* dwIDCtl */) { return E_NOTIMPL; }
};

// This sample demonstrates how to use the file dialog in a modal way such that
// users can easily pick multiple files. It does this by overriding the normal "Open" button
// with an "Add" button that passes the selection back to this app.
//
// One case this sample does not support is selecting folders this way. This has
// the issue of the "Open" button being overloaded for "navigate into the folder" and "add the folder."
// One way to deal with this is to add a new button, "Add Folder", the PickFilesAndFolders sample demonstrates this.
void FileOpenBasketPicker()
{
    IFileOpenDialog *pfd;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        CFileOpenBasketPickerCallback foacb;
        DWORD dwCookie;
        if (SUCCEEDED(pfd->Advise(&foacb, &dwCookie)))
        {
            DWORD dwOptions;
            if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
            {
                pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT | FOS_ALLNONSTORAGEITEMS);
            }

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

            pfd->SetTitle(L"File Open Modal Basket Picker Sample");

            // We do not process the results of the dialog since
            // the selected items are passed back via OnFileOk()
            pfd->Show(NULL); // hr intentionally ignored

            pfd->Unadvise(dwCookie);
        }
        pfd->Release();
    }
}

/* Picking Files and Folders in Basket Mode **************************************************************************************/

class CPickFilesAndFoldersCallback : public IFileDialogEvents, public IFileDialogControlEvents
{
public:
    CPickFilesAndFoldersCallback()
    {
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CPickFilesAndFoldersCallback, IFileDialogEvents),
            QITABENT(CPickFilesAndFoldersCallback, IFileDialogControlEvents),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    // This class makes special assumptions about how it is used, specifically
    // 1) This class will only reside on the stack.
    // 2) Components that consume this object have well-defined reference lifetimes.
    //    In this case, this is only consumed by the file dialog advise and unadvise.
    //    Unadvising will release the file dialog's only reference to this object.
    //
    // Do not do this for heap allocated objects.
    IFACEMETHODIMP_(ULONG) AddRef()  { return 3; }
    IFACEMETHODIMP_(ULONG) Release() { return 2; }

    // IFileDialogEvents
    IFACEMETHODIMP OnFileOk(IFileDialog *pfd)
    {
        ReportSelectedItemsFromSite(pfd);
        return S_FALSE; // S_FALSE keeps the dialog up, return S_OK to allows it to dismiss
    }

    IFACEMETHODIMP OnFolderChanging(IFileDialog * /* pfd */, IShellItem * /* psi */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnFolderChange(IFileDialog * /* pfd */)
    {
        return E_NOTIMPL;
    }

    IFACEMETHODIMP OnSelectionChange(IFileDialog *pfd)
    {
        // Design for the text of the "Add" button
        // ---------------------------------------
        // Single select item      "Add file"
        // Single select folder    "Add folder"
        // Multiselect             "Add items"
        // Null select             "Add current folder"
        IFileDialogCustomize *pfdc;
        if (SUCCEEDED(pfd->QueryInterface(&pfdc)))
        {
            // GetSelectionFromSite() fails on no selection
            // When that happens, default to the current folder.
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

        return S_OK;
    }

    IFACEMETHODIMP OnShareViolation(IFileDialog * /* pfd */, IShellItem * /* psi */, FDE_SHAREVIOLATION_RESPONSE * /* pResponse */) { return E_NOTIMPL; }
    IFACEMETHODIMP OnTypeChange(IFileDialog * /* pfd */) { return E_NOTIMPL; }
    IFACEMETHODIMP OnOverwrite(IFileDialog * /* pfd */, IShellItem * /* psi */ , FDE_OVERWRITE_RESPONSE * /* pResponse */) { return E_NOTIMPL;}

    // IFileDialogControlEvents
    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize * /* pfdc */, DWORD /* dwIDCtl */, DWORD /* dwIDItem */)  { return E_NOTIMPL; }

    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize *pfdc, DWORD dwIDCtl)
    {
        switch (dwIDCtl)
        {
        case c_idAdd:
            // Instead of using IFileDialog::GetCurrentSelection(), we need to get the
            // selection from the view to handle the "no selection implies folder" case
            ReportSelectedItemsFromSite(pfdc);
            break;

        case c_idDone:
            {
                IFileDialog *pfd;
                if (SUCCEEDED(pfdc->QueryInterface(&pfd)))
                {
                    pfd->Close(S_OK);
                    pfd->Release();
                }
            }
            break;

        default:
            break;
        }

        return S_OK;
    }

    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize * /* pfdc */, DWORD /* dwIDCtl */, BOOL /* bChecked */) { return E_NOTIMPL; }
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize * /* pfdc */, DWORD /* dwIDCtl */) { return E_NOTIMPL; }
};

void PickFilesAndFolders()
{
    IFileOpenDialog *pfd;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
    {
        CPickFilesAndFoldersCallback foacb;
        DWORD dwCookie;
        if (SUCCEEDED(pfd->Advise(&foacb, &dwCookie)))
        {
            DWORD dwOptions;
            if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
            {
                pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT | FOS_ALLNONSTORAGEITEMS);
            }

            IFileDialogCustomize *pfdc;
            if (SUCCEEDED(pfd->QueryInterface(&pfdc)))
            {
                // The spacing pads the button a bit.
                pfdc->AddPushButton(c_idAdd, L" Add current folder ");
                pfdc->Release();
            }

            IFileDialog2 *pfd2;
            if (SUCCEEDED(pfd->QueryInterface(&pfd2)))
            {
                pfd2->SetCancelButtonLabel(L"Done");
                pfd2->Release();
            }
            else
            {
                // pre Win7 we need to add a 3rd button, ugly but workable
                IFileDialogCustomize *pfdc;
                if (SUCCEEDED(pfd->QueryInterface(&pfdc)))
                {
                    pfdc->AddPushButton(c_idDone, L"Done");
                    pfdc->Release();
                }
            }

            pfd->SetTitle(L"Pick Files and Folder Sample");

            // the items selected are passed back via OnFileOk()
            // so we don't process the results of the dialog
            pfd->Show(NULL); // hr intentionally ignored
            pfd->Unadvise(dwCookie);
        }
        pfd->Release();
    }
}

// Application entry point
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    HRESULT hr = OleInitialize(0);
    if (SUCCEEDED(hr))
    {
        TASKDIALOGCONFIG taskDialogParams = { sizeof(taskDialogParams) };
        taskDialogParams.dwFlags = TDF_USE_COMMAND_LINKS | TDF_ALLOW_DIALOG_CANCELLATION;

        TASKDIALOG_BUTTON const buttons[] =
        {
            { IDC_PICKITEM,             L"Pick File" },
            { IDC_PICKCONTAINER,        L"Pick Folder" },
            { IDC_FILEOPENBASKETPICKER, L"Pick Files (Basket Mode)" },
            { IDC_PICKFILESANDFOLDERS,  L"Pick Files and Folders (Basket Mode)" },
        };

        taskDialogParams.pButtons = buttons;
        taskDialogParams.cButtons = ARRAYSIZE(buttons);
        taskDialogParams.pszMainInstruction = L"Pick the file dialog samples you want to try";
        taskDialogParams.pszWindowTitle = L"Common File Dialog Modes";

        while (SUCCEEDED(hr))
        {
            int selectedId;
            hr = TaskDialogIndirect(&taskDialogParams, &selectedId, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                if (selectedId == IDCANCEL)
                {
                    break;
                }
                else if (selectedId == IDC_PICKITEM)
                {
                    PickItem();
                }
                else if (selectedId == IDC_PICKCONTAINER)
                {
                    PickContainer();
                }
                else if (selectedId == IDC_FILEOPENBASKETPICKER)
                {
                    FileOpenBasketPicker();
                }
                else if (selectedId == IDC_PICKFILESANDFOLDERS)
                {
                    PickFilesAndFolders();
                }
            }
        }
        OleUninitialize();
    }
    return 0;
}

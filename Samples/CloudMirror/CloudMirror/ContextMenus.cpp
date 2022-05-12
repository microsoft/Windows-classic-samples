// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "ContextMenus.h"
#include <winrt\Windows.Storage.Provider.h>
#include <winrt\Windows.Foundation.h>

namespace winrt
{
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Provider;
}

IFACEMETHODIMP TestExplorerCommandHandler::GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name)
{
    *name = nullptr;
    return SHStrDup(L"TestCommand", name);
}

IFACEMETHODIMP TestExplorerCommandHandler::GetState(_In_opt_ IShellItemArray*, _In_ BOOL, _Out_ EXPCMDSTATE* cmdState) 
{ 
    *cmdState = ECS_ENABLED; 
    return S_OK; 
}

IFACEMETHODIMP TestExplorerCommandHandler::GetFlags(_Out_ EXPCMDFLAGS* flags) 
{ 
    *flags = ECF_DEFAULT; 
    return S_OK; 
}

winrt::fire_and_forget TestExplorerCommandHandler::InvokeAsync(_In_opt_ IShellItemArray* selection)
{
    winrt::com_ptr<IShellItemArray> selectionCopy;
    selectionCopy.copy_from(selection);
    winrt::agile_ref<IShellItemArray> agileSelectionCopy{ selectionCopy };

    auto strongThis = get_strong(); // prevent destruction while coroutine is running

    co_await winrt::resume_background();

    try
    {

        wprintf(L"Cloud Provider Command received\n");

        // If you want to show UI on invoke, do it in your own process.
        // Do not use the Explorer window as an owner, because Explorer
        // is not aware that the provider's context menu is still using it.

        //
        // Set a new custom state on the selected files
        //
        winrt::StorageProviderItemProperty prop;
        prop.Id(3);
        prop.Value(L"Value3");
        // This icon is just for the sample. You should provide your own branded icon here
        prop.IconResource(L"shell32.dll,-259");

        DWORD count;
        winrt::check_hresult(agileSelectionCopy.get()->GetCount(&count));
        for (DWORD i = 0; i < count; i++)
        {
            winrt::com_ptr<IShellItem> shellItem;
            winrt::check_hresult(agileSelectionCopy.get()->GetItemAt(i, shellItem.put()));

            winrt::com_array<wchar_t> fullPath;
            winrt::check_hresult(shellItem->GetDisplayName(SIGDN_FILESYSPATH, winrt::put_abi(fullPath)));

            auto fileAttributes = GetFileAttributes(fullPath.data());
            if (!(fileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                winrt::IStorageItem item = winrt::StorageFile::GetFileFromPathAsync(fullPath.data()).get();
                winrt::StorageProviderItemProperties::SetAsync(item, { prop }).get();
            }

            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, static_cast<void*>(fullPath.data()), nullptr);
        }
    }
    catch (...)
    {
        wprintf(L"Cloud Provider Command failed\n");
    }
}

IFACEMETHODIMP TestExplorerCommandHandler::Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*) 
{
    TestExplorerCommandHandler::InvokeAsync(selection);
    
    return S_OK;
}

IFACEMETHODIMP TestExplorerCommandHandler::SetSite(_In_opt_ IUnknown *site)
{
    _site.copy_from(site);
    return S_OK;
}
IFACEMETHODIMP TestExplorerCommandHandler::GetSite(_In_ REFIID riid, _COM_Outptr_ void **site)
{
    return _site->QueryInterface(riid, site);
}


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "ContextMenus.h"
#include <winrt\Windows.Storage.Provider.h>

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

IFACEMETHODIMP TestExplorerCommandHandler::Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*) 
{ 
    try
    {
        HWND hwnd = nullptr;

        if (_site)
        {
            // Get the HWND of the browser from the site to parent our message box to
            winrt::com_ptr<IUnknown> browser;
            winrt::check_hresult(IUnknown_QueryService(_site.get(), SID_STopLevelBrowser, __uuidof(browser), browser.put_void()));
            IUnknown_GetWindow(browser.get(), &hwnd);
        }

        wprintf(L"Cloud Provider Command received\n");

        //
        // Set a new custom state on the selected files
        //
        auto customProperties{ winrt::single_threaded_vector<winrt::StorageProviderItemProperty>() };
        winrt::StorageProviderItemProperty prop;
        prop.Id(3);
        prop.Value(L"Value3");
        // This icon is just for the sample. You should provide your own branded icon here
        prop.IconResource(L"shell32.dll,-259");
        customProperties.Append(prop);

        DWORD count;
        winrt::check_hresult(selection->GetCount(&count));
        for (DWORD i = 0; i < count; i++)
        {
            winrt::com_ptr<IShellItem> shellItem;
            winrt::check_hresult(selection->GetItemAt(i, shellItem.put()));

            winrt::com_array<wchar_t> fullPath;
            winrt::check_hresult(shellItem->GetDisplayName(SIGDN_FILESYSPATH, winrt::put_abi(fullPath)));

            winrt::IStorageItem item = winrt::StorageFile::GetFileFromPathAsync(fullPath.data()).get();
            winrt::StorageProviderItemProperties::SetAsync(item, customProperties).get();

            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, static_cast<void*>(fullPath.data()), nullptr);
        }
    }
    catch (...)
    {
        return winrt::to_hresult();
    }

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


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "ClassFactory.h"
#include "ThumbnailProvider.h"
#include "ContextMenus.h"
#include "CustomStateProvider.h"
#include "UriSource.h"

//===============================================================
// ShellServices
//
//    Registers a bunchof COM objects that implement the various
//    whizbangs and gizmos that Shell needs for things like
//    thumbnails, context menus, and custom states.
//
// Fakery Factor:
//
//    Not a lot here. The classes referenced are all fakes,
//    but you could prolly modify them with ease.
//
//===============================================================


void ShellServices::InitAndStartServiceTask()
{
    auto task = concurrency::create_task([]()
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);

        winrt::check_hresult(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

        DWORD cookie;
        auto thumbnailProviderClassFactory = winrt::make<ClassFactory<ThumbnailProvider>>();
        winrt::check_hresult(CoRegisterClassObject(__uuidof(ThumbnailProvider), thumbnailProviderClassFactory.get(), CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &cookie));

        auto contextMenuClassFactory = winrt::make<ClassFactory<TestExplorerCommandHandler>>();
        winrt::check_hresult(CoRegisterClassObject(__uuidof(TestExplorerCommandHandler), contextMenuClassFactory.get(), CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &cookie));

        auto customStateProvider = winrt::make<ClassFactory<winrt::CloudMirror::implementation::CustomStateProvider>>();
        winrt::check_hresult(CoRegisterClassObject(CLSID_CustomStateProvider, customStateProvider.get(), CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &cookie));

        auto uriSource = winrt::make<ClassFactory<winrt::CloudMirror::implementation::UriSource>>();
        winrt::check_hresult(CoRegisterClassObject(CLSID_UriSource, uriSource.get(), CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &cookie));

        winrt::handle dummyEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr));
        if (!dummyEvent)
        {
            winrt::throw_last_error();
        }
        DWORD index;
        HANDLE temp = dummyEvent.get();
        CoWaitForMultipleHandles(COWAIT_DISPATCH_CALLS, INFINITE, 1, &temp, &index);
    });
}
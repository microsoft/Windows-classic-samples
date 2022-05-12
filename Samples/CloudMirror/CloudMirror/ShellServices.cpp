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
#include "MyStatusUISourceFactory.h"
#include "MyStorageProviderUICommand.h"

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

namespace
{
    template<typename T>
    DWORD make_and_register_class_object()
    {
        DWORD cookie;
        auto factory = winrt::make<ClassFactory<T>>();
        winrt::check_hresult(CoRegisterClassObject(__uuidof(T), factory.get(), CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE, &cookie));
        return cookie;
    }
}

void ShellServices::InitAndStartServiceTask()
{
    auto task = std::thread([]()
    {
        winrt::init_apartment(winrt::apartment_type::single_threaded);

        make_and_register_class_object<ThumbnailProvider>();
        make_and_register_class_object<TestExplorerCommandHandler>();
        make_and_register_class_object<winrt::CloudMirror::implementation::CustomStateProvider>();
        make_and_register_class_object<winrt::CloudMirror::implementation::UriSource>();
        make_and_register_class_object<winrt::CloudMirror::implementation::MyStatusUISourceFactory>();

        winrt::handle dummyEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr));
        if (!dummyEvent)
        {
            winrt::throw_last_error();
        }
        DWORD index;
        HANDLE temp = dummyEvent.get();
        CoWaitForMultipleHandles(COWAIT_DISPATCH_CALLS, INFINITE, 1, &temp, &index);
    });
    task.detach();
}

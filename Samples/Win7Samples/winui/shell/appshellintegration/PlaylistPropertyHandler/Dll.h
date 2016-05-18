// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <new>  // std::nothrow

class __declspec(uuid("a8090ad8-8f40-4c62-85b7-28fd60244ade")) CPlaylistPropertyHandler;
HRESULT CPlaylistPropertyHandler_CreateInstance(REFIID riid, void **ppv);
HRESULT RegisterHandler();
HRESULT UnregisterHandler();

void DllAddRef();
void DllRelease();

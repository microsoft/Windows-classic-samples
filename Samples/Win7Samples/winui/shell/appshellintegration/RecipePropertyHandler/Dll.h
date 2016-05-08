// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <new>  // std::nothrow

class __declspec(uuid("50D9450F-2A80-4F08-93B9-2EB526477D1A")) CRecipePropertyHandler;
HRESULT CRecipePropertyHandler_CreateInstance(REFIID riid, void **ppv);
HRESULT RegisterHandler();
HRESULT UnregisterHandler();

void DllAddRef();
void DllRelease();

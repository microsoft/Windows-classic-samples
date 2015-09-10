//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <MI.h>
#include <Windows.h>
#include <strsafe.h>
#include "MSFT_WindowsService.h"

#define SYSTEM_CREATION_CLASS_NAME L"CIM_System"
#define CLASS_CREATION_NAME L"MSFT_WindowsService"

MI_Result EnumerateServices(
    _In_ MI_Context* context,
    _In_ MI_Boolean keysOnly);

MI_Result GetServiceInstance(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsService* instanceName);

void Invoke_StartService(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsService* instanceName);

void Invoke_StopService(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsService* instanceName);

MI_Result ResultFromWin32Error(
    DWORD error);

LPVOID AllocateMemory(SIZE_T dwBytes);

void FreeMemory(LPVOID lpMem);

MI_Result SetService(
    _Out_ MSFT_WindowsService* self,
    _In_ SC_HANDLE *phSvcCtlMgr,
    _In_ ENUM_SERVICE_STATUS *,
    _In_ MI_Context* context);




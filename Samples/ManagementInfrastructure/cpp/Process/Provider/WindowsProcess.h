//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <windows.h>
#include <Psapi.h>
#include <strsafe.h>
#include <MI.h>

#define INFO_BUFFER_SIZE 256
#define OS_CREATION_CLASS_NAME L"CIM_OperatingSystem"
#define CLASS_CREATION_NAME L"MSFT_WindowsProcess"
#define CS_CREATION_CLASS_NAME L"CIM_ComputerSystem"

MI_Result EnumerateProcesses(
    _In_ MI_Context* context,
    _In_ MI_Boolean keysOnly);

MI_Result GetProcessInstance(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* instanceName);

MI_Result DeleteProcessInstance(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* instanceName);

MI_Result ModifyProcessInstance(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* modifiedInstance);

MI_Result Invoke_SetPriority(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MSFT_WindowsProcess_SetPriority* in);

MI_Result ResultFromWin32Error(
    DWORD error);

MI_Result ConvertFileTimeToDateTime(
    _In_ LPFILETIME pfTime, 
    _Out_ MI_Datetime *pdTime);

//
// Create a process based on given instance.
// Only CommandLine property will be used to create a process.
// The newly created process will be posted back to client
// upon successfully creation.
//
MI_Result InvokeIntrisicCreateMethod(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess* newInstance);

//
// Create a process based on given argument,
// CommandLine argument will be used here.
// The reference to newly created process will be posted back to client
// upon successfully creation.
//
MI_Result InvokeExtrinsicCreateMethod(
    _In_ MI_Context* context,
    _In_ const MSFT_WindowsProcess_Create* createArg);

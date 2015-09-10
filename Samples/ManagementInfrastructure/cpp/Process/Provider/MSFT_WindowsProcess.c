//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <MI.h>
#include "MSFT_WindowsProcess.h"
#include "WindowsProcess.h"

void MI_CALL MSFT_WindowsProcess_Load(
    _Outptr_result_maybenull_ MSFT_WindowsProcess_Self** self,
    _In_opt_ MI_Module_Self* selfModule,
    _In_ MI_Context* context)
{
    MI_UNREFERENCED_PARAMETER(selfModule);

    *self = NULL;
    MI_Context_PostResult(context, MI_RESULT_OK);
}

void MI_CALL MSFT_WindowsProcess_Unload(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context)
{
    MI_UNREFERENCED_PARAMETER(self);

    MI_Context_PostResult(context, MI_RESULT_OK);
}

void MI_CALL MSFT_WindowsProcess_EnumerateInstances(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter)
{
    MI_Result result = MI_RESULT_OK;

    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(propertySet);
    MI_UNREFERENCED_PARAMETER(filter);

    result = EnumerateProcesses(context, keysOnly);
    if (result == MI_RESULT_ACCESS_DENIED)
    {
        MI_Context_PostError(context,
            result,
            MI_RESULT_TYPE_MI,
            MI_T("Access denied. Please try to run client process with elevated priviledge."));
    }
    else if (result != MI_RESULT_OK)
    {
        MI_Context_PostError(context,
            result,
            MI_RESULT_TYPE_MI,
            MI_T(""));
    }
    else
    {
        MI_Context_PostResult(context, result);
    }
}

void MI_CALL MSFT_WindowsProcess_GetInstance(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MI_PropertySet* propertySet)
{
    MI_Result result;
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(instanceName);
    MI_UNREFERENCED_PARAMETER(propertySet);

    result = GetProcessInstance(context, instanceName);

    MI_Context_PostResult(context, result);
}

void MI_CALL MSFT_WindowsProcess_CreateInstance(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsProcess* newInstance)
{
    MI_Result result;
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(newInstance);
    result = InvokeIntrisicCreateMethod(context, newInstance);
    MI_Context_PostResult(context, result);
}

void MI_CALL MSFT_WindowsProcess_ModifyInstance(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsProcess* modifiedInstance,
    _In_opt_ const MI_PropertySet* propertySet)
{
    MI_Result result;
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(propertySet);

    result = ModifyProcessInstance(context, modifiedInstance);
    MI_Context_PostResult(context, result);
}

void MI_CALL MSFT_WindowsProcess_DeleteInstance(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsProcess* instanceName)
{
    MI_Result result;
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);

    result = DeleteProcessInstance(context, instanceName);

    MI_Context_PostResult(context, result);
}

void MI_CALL MSFT_WindowsProcess_Invoke_RequestStateChange(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_z_ const MI_Char* methodName,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MSFT_WindowsProcess_RequestStateChange* in)
{
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(methodName);
    MI_UNREFERENCED_PARAMETER(instanceName);
    MI_UNREFERENCED_PARAMETER(in);

    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

void MI_CALL MSFT_WindowsProcess_Invoke_SetPriority(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_z_ const MI_Char* methodName,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MSFT_WindowsProcess_SetPriority* in)
{
    MI_Result result;
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(methodName);

    result = Invoke_SetPriority(context, instanceName, in);
    
    MI_Context_PostResult(context, result);
}

void MI_CALL MSFT_WindowsProcess_Invoke_Create(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_z_ const MI_Char* methodName,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MSFT_WindowsProcess_Create* in)
{
    MI_Result result;
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(methodName);
    MI_UNREFERENCED_PARAMETER(instanceName);
    MI_UNREFERENCED_PARAMETER(in);

    if (in != NULL)
    {
        result = InvokeExtrinsicCreateMethod(context, in);
    }
    else
    {
        result = MI_RESULT_INVALID_PARAMETER;
    }
    MI_Context_PostResult(context, result);
}


//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <MI.h>
#include "MSFT_WindowsServiceProcess.h"

#include <stdlib.h>
#include "WindowsServiceProcess.h"


void MI_CALL MSFT_WindowsServiceProcess_Load(
    _Outptr_result_maybenull_ MSFT_WindowsServiceProcess_Self** self,
    _In_opt_ MI_Module_Self* selfModule,
    _In_ MI_Context* context)
{
    MI_UNREFERENCED_PARAMETER(selfModule);

    *self = NULL;
    MI_Context_PostResult(context, MI_RESULT_OK);
}

void MI_CALL MSFT_WindowsServiceProcess_Unload(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context)
{
    MI_UNREFERENCED_PARAMETER(self);

    MI_Context_PostResult(context, MI_RESULT_OK);
}

void MI_CALL MSFT_WindowsServiceProcess_EnumerateInstances(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter)
{
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(propertySet);
    MI_UNREFERENCED_PARAMETER(keysOnly);
    MI_UNREFERENCED_PARAMETER(filter);

    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

void MI_CALL MSFT_WindowsServiceProcess_GetInstance(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsServiceProcess* instanceName,
    _In_opt_ const MI_PropertySet* propertySet)
{
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(instanceName);
    MI_UNREFERENCED_PARAMETER(propertySet);

    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

void MI_CALL MSFT_WindowsServiceProcess_CreateInstance(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsServiceProcess* newInstance)
{
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(newInstance);

    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

void MI_CALL MSFT_WindowsServiceProcess_ModifyInstance(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsServiceProcess* modifiedInstance,
    _In_opt_ const MI_PropertySet* propertySet)
{
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(modifiedInstance);
    MI_UNREFERENCED_PARAMETER(propertySet);

    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

void MI_CALL MSFT_WindowsServiceProcess_DeleteInstance(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsServiceProcess* instanceName)
{
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(instanceName);

    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

void MI_CALL MSFT_WindowsServiceProcess_AssociatorInstancesService(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const CIM_Service* instanceName,
    _In_z_ const MI_Char* resultClass,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter)
{
    MI_Result result = MI_RESULT_OK;
    MI_Value serviceName;
    MI_Type serviceType;

    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(instanceName);
    MI_UNREFERENCED_PARAMETER(resultClass);
    MI_UNREFERENCED_PARAMETER(propertySet);
    MI_UNREFERENCED_PARAMETER(keysOnly);
    MI_UNREFERENCED_PARAMETER(filter);

    result = MI_Instance_GetElement(&instanceName->__instance, L"Name", &serviceName, &serviceType, NULL, NULL);
    if( result != MI_RESULT_OK)
    {
        MI_Context_PostResult(context, result);
        return;
    }    

    if( serviceType != MI_STRING)
    {
        MI_Context_PostResult(context, MI_RESULT_INVALID_PARAMETER);
        return;
    }        
    
    GetProcessInstances(context, nameSpace, serviceName.string, NULL);
}

void MI_CALL MSFT_WindowsServiceProcess_AssociatorInstancesProcess(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const CIM_Process* instanceName,
    _In_z_ const MI_Char* resultClass,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter)
{
    MI_Result result = MI_RESULT_OK;
    MI_Value tempHandleID;    
    MI_Type handleType;
    MI_Uint32 handleID;
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(instanceName);
    MI_UNREFERENCED_PARAMETER(resultClass);
    MI_UNREFERENCED_PARAMETER(propertySet);
    MI_UNREFERENCED_PARAMETER(keysOnly);
    MI_UNREFERENCED_PARAMETER(filter);

    result = MI_Instance_GetElement(&instanceName->__instance, L"Handle", &tempHandleID, &handleType, NULL, NULL);
    if( result != MI_RESULT_OK)
    {
        MI_Context_PostResult(context, result);
        return;
    }

    if( handleType != MI_STRING)
    {
        MI_Context_PostResult(context, MI_RESULT_INVALID_PARAMETER);
        return;
    }    
    
    handleID =_wtol(tempHandleID.string);
    if( handleID == 0 )
    {
        MI_Context_PostResult(context, MI_RESULT_INVALID_PARAMETER);
        return;
    }
    GetServiceInstances(context, nameSpace, handleID, NULL);
}

void MI_CALL MSFT_WindowsServiceProcess_ReferenceInstancesService(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const CIM_Service* instanceName,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter)
{   
    MI_Result result = MI_RESULT_OK;
    MI_Value serviceName;
    MI_Type serviceType;

    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(instanceName);
    MI_UNREFERENCED_PARAMETER(propertySet);
    MI_UNREFERENCED_PARAMETER(keysOnly);
    MI_UNREFERENCED_PARAMETER(filter);
    
    result = MI_Instance_GetElement(&instanceName->__instance, L"Name", &serviceName, &serviceType, NULL, NULL);
    if( result != MI_RESULT_OK)
    {
        MI_Context_PostResult(context, result);
        return;
    }    

    if( serviceType != MI_STRING)
    {
        MI_Context_PostResult(context, MI_RESULT_INVALID_PARAMETER);
        return;
    }        
    
    GetProcessInstances(context, nameSpace, serviceName.string, &instanceName->__instance);

}

void MI_CALL MSFT_WindowsServiceProcess_ReferenceInstancesProcess(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const CIM_Process* instanceName,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter)
{   
    MI_Result result = MI_RESULT_OK;
    MI_Value tempHandleID;    
    MI_Type handleType;
    MI_Uint32 handleID;    
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(instanceName);
    MI_UNREFERENCED_PARAMETER(propertySet);
    MI_UNREFERENCED_PARAMETER(keysOnly);
    MI_UNREFERENCED_PARAMETER(filter);

    result = MI_Instance_GetElement(&instanceName->__instance, L"Handle", &tempHandleID, &handleType, NULL, NULL);
    if( result != MI_RESULT_OK)
    {
        MI_Context_PostResult(context, result);
        return;
    }

    if( handleType != MI_STRING)
    {
        MI_Context_PostResult(context, MI_RESULT_INVALID_PARAMETER);
        return;
    }
    
    handleID =_wtol(tempHandleID.string);
    if( handleID == 0 )
    {
        MI_Context_PostResult(context, MI_RESULT_INVALID_PARAMETER);
        return;
    }
    GetServiceInstances(context, nameSpace, handleID, &instanceName->__instance);

}


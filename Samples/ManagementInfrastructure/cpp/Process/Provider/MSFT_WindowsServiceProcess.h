//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

/*
**==============================================================================
**
** WARNING: THIS FILE WAS AUTOMATICALLY GENERATED. PLEASE DO NOT EDIT.
**
**==============================================================================
*/
#ifndef _MSFT_WindowsServiceProcess_h
#define _MSFT_WindowsServiceProcess_h

#include <MI.h>
#include "CIM_ServiceProcess.h"
#include "CIM_Service.h"
#include "CIM_Process.h"

/*
**==============================================================================
**
** MSFT_WindowsServiceProcess [MSFT_WindowsServiceProcess]
**
** Keys:
**    Service
**    Process
**
**==============================================================================
*/

typedef struct _MSFT_WindowsServiceProcess /* extends CIM_ServiceProcess */
{
    MI_Instance __instance;
    /* CIM_ServiceProcess properties */
    /*KEY*/ CIM_Service_ConstRef Service;
    /*KEY*/ CIM_Process_ConstRef Process;
    MI_ConstUint16Field ExecutionType;
    /* MSFT_WindowsServiceProcess properties */
}
MSFT_WindowsServiceProcess;

typedef struct _MSFT_WindowsServiceProcess_Ref
{
    MSFT_WindowsServiceProcess* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceProcess_Ref;

typedef struct _MSFT_WindowsServiceProcess_ConstRef
{
    MI_CONST MSFT_WindowsServiceProcess* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceProcess_ConstRef;

typedef struct _MSFT_WindowsServiceProcess_Array
{
    struct _MSFT_WindowsServiceProcess** data;
    MI_Uint32 size;
}
MSFT_WindowsServiceProcess_Array;

typedef struct _MSFT_WindowsServiceProcess_ConstArray
{
    struct _MSFT_WindowsServiceProcess MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
MSFT_WindowsServiceProcess_ConstArray;

typedef struct _MSFT_WindowsServiceProcess_ArrayRef
{
    MSFT_WindowsServiceProcess_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceProcess_ArrayRef;

typedef struct _MSFT_WindowsServiceProcess_ConstArrayRef
{
    MSFT_WindowsServiceProcess_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceProcess_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl MSFT_WindowsServiceProcess_rtti;

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Construct(
    _Out_ MSFT_WindowsServiceProcess* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &MSFT_WindowsServiceProcess_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Clone(
    _In_ const MSFT_WindowsServiceProcess* self,
    _Outptr_ MSFT_WindowsServiceProcess** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL MSFT_WindowsServiceProcess_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &MSFT_WindowsServiceProcess_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Destruct(_Inout_ MSFT_WindowsServiceProcess* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Delete(_Inout_ MSFT_WindowsServiceProcess* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Post(
    _In_ const MSFT_WindowsServiceProcess* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Set_Service(
    _Inout_ MSFT_WindowsServiceProcess* self,
    _In_ const CIM_Service* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&x,
        MI_REFERENCE,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_SetPtr_Service(
    _Inout_ MSFT_WindowsServiceProcess* self,
    _In_ const CIM_Service* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&x,
        MI_REFERENCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Clear_Service(
    _Inout_ MSFT_WindowsServiceProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Set_Process(
    _Inout_ MSFT_WindowsServiceProcess* self,
    _In_ const CIM_Process* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&x,
        MI_REFERENCE,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_SetPtr_Process(
    _Inout_ MSFT_WindowsServiceProcess* self,
    _In_ const CIM_Process* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&x,
        MI_REFERENCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Clear_Process(
    _Inout_ MSFT_WindowsServiceProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Set_ExecutionType(
    _Inout_ MSFT_WindowsServiceProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->ExecutionType)->value = x;
    ((MI_Uint16Field*)&self->ExecutionType)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceProcess_Clear_ExecutionType(
    _Inout_ MSFT_WindowsServiceProcess* self)
{
    memset((void*)&self->ExecutionType, 0, sizeof(self->ExecutionType));
    return MI_RESULT_OK;
}

/*
**==============================================================================
**
** MSFT_WindowsServiceProcess provider function prototypes
**
**==============================================================================
*/

/* The developer may optionally define this structure */
typedef struct _MSFT_WindowsServiceProcess_Self MSFT_WindowsServiceProcess_Self;

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_Load(
    _Outptr_result_maybenull_ MSFT_WindowsServiceProcess_Self** self,
    _In_opt_ MI_Module_Self* selfModule,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_Unload(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_EnumerateInstances(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_GetInstance(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsServiceProcess* instanceName,
    _In_opt_ const MI_PropertySet* propertySet);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_CreateInstance(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsServiceProcess* newInstance);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_ModifyInstance(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsServiceProcess* modifiedInstance,
    _In_opt_ const MI_PropertySet* propertySet);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_DeleteInstance(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsServiceProcess* instanceName);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_AssociatorInstancesService(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const CIM_Service* instanceName,
    _In_z_ const MI_Char* resultClass,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_AssociatorInstancesProcess(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const CIM_Process* instanceName,
    _In_z_ const MI_Char* resultClass,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_ReferenceInstancesService(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const CIM_Service* instanceName,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceProcess_ReferenceInstancesProcess(
    _In_opt_ MSFT_WindowsServiceProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const CIM_Process* instanceName,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter);


#endif /* _MSFT_WindowsServiceProcess_h */

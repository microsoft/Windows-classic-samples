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
#ifndef _CIM_ServiceProcess_h
#define _CIM_ServiceProcess_h

#include <MI.h>
#include "CIM_Service.h"
#include "CIM_Process.h"

/*
**==============================================================================
**
** CIM_ServiceProcess [CIM_ServiceProcess]
**
** Keys:
**    Service
**    Process
**
**==============================================================================
*/

typedef struct _CIM_ServiceProcess
{
    MI_Instance __instance;
    /* CIM_ServiceProcess properties */
    /*KEY*/ CIM_Service_ConstRef Service;
    /*KEY*/ CIM_Process_ConstRef Process;
    MI_ConstUint16Field ExecutionType;
}
CIM_ServiceProcess;

typedef struct _CIM_ServiceProcess_Ref
{
    CIM_ServiceProcess* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_ServiceProcess_Ref;

typedef struct _CIM_ServiceProcess_ConstRef
{
    MI_CONST CIM_ServiceProcess* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_ServiceProcess_ConstRef;

typedef struct _CIM_ServiceProcess_Array
{
    struct _CIM_ServiceProcess** data;
    MI_Uint32 size;
}
CIM_ServiceProcess_Array;

typedef struct _CIM_ServiceProcess_ConstArray
{
    struct _CIM_ServiceProcess MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
CIM_ServiceProcess_ConstArray;

typedef struct _CIM_ServiceProcess_ArrayRef
{
    CIM_ServiceProcess_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_ServiceProcess_ArrayRef;

typedef struct _CIM_ServiceProcess_ConstArrayRef
{
    CIM_ServiceProcess_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_ServiceProcess_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl CIM_ServiceProcess_rtti;

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Construct(
    _Out_ CIM_ServiceProcess* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &CIM_ServiceProcess_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Clone(
    _In_ const CIM_ServiceProcess* self,
    _Outptr_ CIM_ServiceProcess** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL CIM_ServiceProcess_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &CIM_ServiceProcess_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Destruct(_Inout_ CIM_ServiceProcess* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Delete(_Inout_ CIM_ServiceProcess* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Post(
    _In_ const CIM_ServiceProcess* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Set_Service(
    _Inout_ CIM_ServiceProcess* self,
    _In_ const CIM_Service* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&x,
        MI_REFERENCE,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_SetPtr_Service(
    _Inout_ CIM_ServiceProcess* self,
    _In_ const CIM_Service* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&x,
        MI_REFERENCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Clear_Service(
    _Inout_ CIM_ServiceProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Set_Process(
    _Inout_ CIM_ServiceProcess* self,
    _In_ const CIM_Process* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&x,
        MI_REFERENCE,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_SetPtr_Process(
    _Inout_ CIM_ServiceProcess* self,
    _In_ const CIM_Process* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&x,
        MI_REFERENCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Clear_Process(
    _Inout_ CIM_ServiceProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Set_ExecutionType(
    _Inout_ CIM_ServiceProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->ExecutionType)->value = x;
    ((MI_Uint16Field*)&self->ExecutionType)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ServiceProcess_Clear_ExecutionType(
    _Inout_ CIM_ServiceProcess* self)
{
    memset((void*)&self->ExecutionType, 0, sizeof(self->ExecutionType));
    return MI_RESULT_OK;
}


#endif /* _CIM_ServiceProcess_h */

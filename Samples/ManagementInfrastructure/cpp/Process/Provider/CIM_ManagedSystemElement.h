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
#ifndef _CIM_ManagedSystemElement_h
#define _CIM_ManagedSystemElement_h

#include <MI.h>
#include "CIM_ManagedElement.h"

/*
**==============================================================================
**
** CIM_ManagedSystemElement [CIM_ManagedSystemElement]
**
** Keys:
**
**==============================================================================
*/

typedef struct _CIM_ManagedSystemElement /* extends CIM_ManagedElement */
{
    MI_Instance __instance;
    /* CIM_ManagedElement properties */
    MI_ConstStringField InstanceID;
    MI_ConstStringField Caption;
    MI_ConstStringField Description;
    MI_ConstStringField ElementName;
    /* CIM_ManagedSystemElement properties */
    MI_ConstDatetimeField InstallDate;
    MI_ConstStringField Name;
    MI_ConstUint16AField OperationalStatus;
    MI_ConstStringAField StatusDescriptions;
    MI_ConstStringField Status;
    MI_ConstUint16Field HealthState;
    MI_ConstUint16Field CommunicationStatus;
    MI_ConstUint16Field DetailedStatus;
    MI_ConstUint16Field OperatingStatus;
    MI_ConstUint16Field PrimaryStatus;
}
CIM_ManagedSystemElement;

typedef struct _CIM_ManagedSystemElement_Ref
{
    CIM_ManagedSystemElement* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_ManagedSystemElement_Ref;

typedef struct _CIM_ManagedSystemElement_ConstRef
{
    MI_CONST CIM_ManagedSystemElement* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_ManagedSystemElement_ConstRef;

typedef struct _CIM_ManagedSystemElement_Array
{
    struct _CIM_ManagedSystemElement** data;
    MI_Uint32 size;
}
CIM_ManagedSystemElement_Array;

typedef struct _CIM_ManagedSystemElement_ConstArray
{
    struct _CIM_ManagedSystemElement MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
CIM_ManagedSystemElement_ConstArray;

typedef struct _CIM_ManagedSystemElement_ArrayRef
{
    CIM_ManagedSystemElement_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_ManagedSystemElement_ArrayRef;

typedef struct _CIM_ManagedSystemElement_ConstArrayRef
{
    CIM_ManagedSystemElement_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_ManagedSystemElement_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl CIM_ManagedSystemElement_rtti;

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Construct(
    _Out_ CIM_ManagedSystemElement* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &CIM_ManagedSystemElement_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clone(
    _In_ const CIM_ManagedSystemElement* self,
    _Outptr_ CIM_ManagedSystemElement** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL CIM_ManagedSystemElement_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &CIM_ManagedSystemElement_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Destruct(_Inout_ CIM_ManagedSystemElement* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Delete(_Inout_ CIM_ManagedSystemElement* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Post(
    _In_ const CIM_ManagedSystemElement* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_InstanceID(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_SetPtr_InstanceID(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_InstanceID(
    _Inout_ CIM_ManagedSystemElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_Caption(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_SetPtr_Caption(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_Caption(
    _Inout_ CIM_ManagedSystemElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_Description(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_SetPtr_Description(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_Description(
    _Inout_ CIM_ManagedSystemElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_ElementName(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_SetPtr_ElementName(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_ElementName(
    _Inout_ CIM_ManagedSystemElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_InstallDate(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->InstallDate)->value = x;
    ((MI_DatetimeField*)&self->InstallDate)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_InstallDate(
    _Inout_ CIM_ManagedSystemElement* self)
{
    memset((void*)&self->InstallDate, 0, sizeof(self->InstallDate));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_Name(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_SetPtr_Name(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_Name(
    _Inout_ CIM_ManagedSystemElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        5);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_OperationalStatus(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_reads_opt_(size) const MI_Uint16* data,
    _In_ MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        6,
        (MI_Value*)&arr,
        MI_UINT16A,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_SetPtr_OperationalStatus(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_reads_opt_(size) const MI_Uint16* data,
    _In_ MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        6,
        (MI_Value*)&arr,
        MI_UINT16A,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_OperationalStatus(
    _Inout_ CIM_ManagedSystemElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        6);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_StatusDescriptions(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_reads_opt_(size) const MI_Char** data,
    _In_ MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        7,
        (MI_Value*)&arr,
        MI_STRINGA,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_SetPtr_StatusDescriptions(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_reads_opt_(size) const MI_Char** data,
    _In_ MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        7,
        (MI_Value*)&arr,
        MI_STRINGA,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_StatusDescriptions(
    _Inout_ CIM_ManagedSystemElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        7);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_Status(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_SetPtr_Status(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_Status(
    _Inout_ CIM_ManagedSystemElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        8);
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_HealthState(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->HealthState)->value = x;
    ((MI_Uint16Field*)&self->HealthState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_HealthState(
    _Inout_ CIM_ManagedSystemElement* self)
{
    memset((void*)&self->HealthState, 0, sizeof(self->HealthState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_CommunicationStatus(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->CommunicationStatus)->value = x;
    ((MI_Uint16Field*)&self->CommunicationStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_CommunicationStatus(
    _Inout_ CIM_ManagedSystemElement* self)
{
    memset((void*)&self->CommunicationStatus, 0, sizeof(self->CommunicationStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_DetailedStatus(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->DetailedStatus)->value = x;
    ((MI_Uint16Field*)&self->DetailedStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_DetailedStatus(
    _Inout_ CIM_ManagedSystemElement* self)
{
    memset((void*)&self->DetailedStatus, 0, sizeof(self->DetailedStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_OperatingStatus(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->OperatingStatus)->value = x;
    ((MI_Uint16Field*)&self->OperatingStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_OperatingStatus(
    _Inout_ CIM_ManagedSystemElement* self)
{
    memset((void*)&self->OperatingStatus, 0, sizeof(self->OperatingStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Set_PrimaryStatus(
    _Inout_ CIM_ManagedSystemElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->PrimaryStatus)->value = x;
    ((MI_Uint16Field*)&self->PrimaryStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_ManagedSystemElement_Clear_PrimaryStatus(
    _Inout_ CIM_ManagedSystemElement* self)
{
    memset((void*)&self->PrimaryStatus, 0, sizeof(self->PrimaryStatus));
    return MI_RESULT_OK;
}


#endif /* _CIM_ManagedSystemElement_h */

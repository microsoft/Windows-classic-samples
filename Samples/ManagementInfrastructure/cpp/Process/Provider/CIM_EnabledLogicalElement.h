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
#ifndef _CIM_EnabledLogicalElement_h
#define _CIM_EnabledLogicalElement_h

#include <MI.h>
#include "CIM_LogicalElement.h"
#include "CIM_ConcreteJob.h"

/*
**==============================================================================
**
** CIM_EnabledLogicalElement [CIM_EnabledLogicalElement]
**
** Keys:
**
**==============================================================================
*/

typedef struct _CIM_EnabledLogicalElement /* extends CIM_LogicalElement */
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
    /* CIM_LogicalElement properties */
    /* CIM_EnabledLogicalElement properties */
    MI_ConstUint16Field EnabledState;
    MI_ConstStringField OtherEnabledState;
    MI_ConstUint16Field RequestedState;
    MI_ConstUint16Field EnabledDefault;
    MI_ConstDatetimeField TimeOfLastStateChange;
    MI_ConstUint16AField AvailableRequestedStates;
    MI_ConstUint16Field TransitioningToState;
}
CIM_EnabledLogicalElement;

typedef struct _CIM_EnabledLogicalElement_Ref
{
    CIM_EnabledLogicalElement* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_EnabledLogicalElement_Ref;

typedef struct _CIM_EnabledLogicalElement_ConstRef
{
    MI_CONST CIM_EnabledLogicalElement* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_EnabledLogicalElement_ConstRef;

typedef struct _CIM_EnabledLogicalElement_Array
{
    struct _CIM_EnabledLogicalElement** data;
    MI_Uint32 size;
}
CIM_EnabledLogicalElement_Array;

typedef struct _CIM_EnabledLogicalElement_ConstArray
{
    struct _CIM_EnabledLogicalElement MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
CIM_EnabledLogicalElement_ConstArray;

typedef struct _CIM_EnabledLogicalElement_ArrayRef
{
    CIM_EnabledLogicalElement_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_EnabledLogicalElement_ArrayRef;

typedef struct _CIM_EnabledLogicalElement_ConstArrayRef
{
    CIM_EnabledLogicalElement_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_EnabledLogicalElement_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl CIM_EnabledLogicalElement_rtti;

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Construct(
    _Out_ CIM_EnabledLogicalElement* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &CIM_EnabledLogicalElement_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clone(
    _In_ const CIM_EnabledLogicalElement* self,
    _Outptr_ CIM_EnabledLogicalElement** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL CIM_EnabledLogicalElement_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &CIM_EnabledLogicalElement_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Destruct(_Inout_ CIM_EnabledLogicalElement* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Delete(_Inout_ CIM_EnabledLogicalElement* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Post(
    _In_ const CIM_EnabledLogicalElement* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_InstanceID(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_InstanceID(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_InstanceID(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_Caption(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_Caption(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_Caption(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_Description(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_Description(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_Description(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_ElementName(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_ElementName(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_ElementName(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_InstallDate(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->InstallDate)->value = x;
    ((MI_DatetimeField*)&self->InstallDate)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_InstallDate(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->InstallDate, 0, sizeof(self->InstallDate));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_Name(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_Name(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_Name(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        5);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_OperationalStatus(
    _Inout_ CIM_EnabledLogicalElement* self,
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

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_OperationalStatus(
    _Inout_ CIM_EnabledLogicalElement* self,
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

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_OperationalStatus(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        6);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_StatusDescriptions(
    _Inout_ CIM_EnabledLogicalElement* self,
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

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_StatusDescriptions(
    _Inout_ CIM_EnabledLogicalElement* self,
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

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_StatusDescriptions(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        7);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_Status(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_Status(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_Status(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        8);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_HealthState(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->HealthState)->value = x;
    ((MI_Uint16Field*)&self->HealthState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_HealthState(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->HealthState, 0, sizeof(self->HealthState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_CommunicationStatus(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->CommunicationStatus)->value = x;
    ((MI_Uint16Field*)&self->CommunicationStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_CommunicationStatus(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->CommunicationStatus, 0, sizeof(self->CommunicationStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_DetailedStatus(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->DetailedStatus)->value = x;
    ((MI_Uint16Field*)&self->DetailedStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_DetailedStatus(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->DetailedStatus, 0, sizeof(self->DetailedStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_OperatingStatus(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->OperatingStatus)->value = x;
    ((MI_Uint16Field*)&self->OperatingStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_OperatingStatus(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->OperatingStatus, 0, sizeof(self->OperatingStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_PrimaryStatus(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->PrimaryStatus)->value = x;
    ((MI_Uint16Field*)&self->PrimaryStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_PrimaryStatus(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->PrimaryStatus, 0, sizeof(self->PrimaryStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_EnabledState(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->EnabledState)->value = x;
    ((MI_Uint16Field*)&self->EnabledState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_EnabledState(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->EnabledState, 0, sizeof(self->EnabledState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_OtherEnabledState(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        15,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_OtherEnabledState(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        15,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_OtherEnabledState(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        15);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_RequestedState(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->RequestedState)->value = x;
    ((MI_Uint16Field*)&self->RequestedState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_RequestedState(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->RequestedState, 0, sizeof(self->RequestedState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_EnabledDefault(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->EnabledDefault)->value = x;
    ((MI_Uint16Field*)&self->EnabledDefault)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_EnabledDefault(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->EnabledDefault, 0, sizeof(self->EnabledDefault));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_TimeOfLastStateChange(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->TimeOfLastStateChange)->value = x;
    ((MI_DatetimeField*)&self->TimeOfLastStateChange)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_TimeOfLastStateChange(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->TimeOfLastStateChange, 0, sizeof(self->TimeOfLastStateChange));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_AvailableRequestedStates(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_reads_opt_(size) const MI_Uint16* data,
    _In_ MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        19,
        (MI_Value*)&arr,
        MI_UINT16A,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_SetPtr_AvailableRequestedStates(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_reads_opt_(size) const MI_Uint16* data,
    _In_ MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        19,
        (MI_Value*)&arr,
        MI_UINT16A,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_AvailableRequestedStates(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        19);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Set_TransitioningToState(
    _Inout_ CIM_EnabledLogicalElement* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->TransitioningToState)->value = x;
    ((MI_Uint16Field*)&self->TransitioningToState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_Clear_TransitioningToState(
    _Inout_ CIM_EnabledLogicalElement* self)
{
    memset((void*)&self->TransitioningToState, 0, sizeof(self->TransitioningToState));
    return MI_RESULT_OK;
}

/*
**==============================================================================
**
** CIM_EnabledLogicalElement.RequestStateChange()
**
**==============================================================================
*/

typedef struct _CIM_EnabledLogicalElement_RequestStateChange
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
    /*IN*/ MI_ConstUint16Field RequestedState;
    /*OUT*/ CIM_ConcreteJob_ConstRef Job;
    /*IN*/ MI_ConstDatetimeField TimeoutPeriod;
}
CIM_EnabledLogicalElement_RequestStateChange;

MI_EXTERN_C MI_CONST MI_MethodDecl CIM_EnabledLogicalElement_RequestStateChange_rtti;

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Construct(
    _Out_ CIM_EnabledLogicalElement_RequestStateChange* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructParameters(context, &CIM_EnabledLogicalElement_RequestStateChange_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Clone(
    _In_ const CIM_EnabledLogicalElement_RequestStateChange* self,
    _Outptr_ CIM_EnabledLogicalElement_RequestStateChange** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Destruct(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Delete(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Post(
    _In_ const CIM_EnabledLogicalElement_RequestStateChange* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Set_MIReturn(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Clear_MIReturn(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Set_RequestedState(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->RequestedState)->value = x;
    ((MI_Uint16Field*)&self->RequestedState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Clear_RequestedState(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self)
{
    memset((void*)&self->RequestedState, 0, sizeof(self->RequestedState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Set_Job(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self,
    _In_ const CIM_ConcreteJob* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&x,
        MI_REFERENCE,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_SetPtr_Job(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self,
    _In_ const CIM_ConcreteJob* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&x,
        MI_REFERENCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Clear_Job(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Set_TimeoutPeriod(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->TimeoutPeriod)->value = x;
    ((MI_DatetimeField*)&self->TimeoutPeriod)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_EnabledLogicalElement_RequestStateChange_Clear_TimeoutPeriod(
    _Inout_ CIM_EnabledLogicalElement_RequestStateChange* self)
{
    memset((void*)&self->TimeoutPeriod, 0, sizeof(self->TimeoutPeriod));
    return MI_RESULT_OK;
}


#endif /* _CIM_EnabledLogicalElement_h */

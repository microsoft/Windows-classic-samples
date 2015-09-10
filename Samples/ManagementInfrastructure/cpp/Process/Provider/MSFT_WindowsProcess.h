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
#ifndef _MSFT_WindowsProcess_h
#define _MSFT_WindowsProcess_h

#include <MI.h>
#include "CIM_Process.h"
#include "CIM_ConcreteJob.h"

/*
**==============================================================================
**
** MSFT_WindowsProcess [MSFT_WindowsProcess]
**
** Keys:
**    CSCreationClassName
**    CSName
**    OSCreationClassName
**    OSName
**    CreationClassName
**    Handle
**
**==============================================================================
*/

typedef struct _MSFT_WindowsProcess /* extends CIM_Process */
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
    /* CIM_Process properties */
    /*KEY*/ MI_ConstStringField CSCreationClassName;
    /*KEY*/ MI_ConstStringField CSName;
    /*KEY*/ MI_ConstStringField OSCreationClassName;
    /*KEY*/ MI_ConstStringField OSName;
    /*KEY*/ MI_ConstStringField CreationClassName;
    /*KEY*/ MI_ConstStringField Handle;
    MI_ConstUint32Field Priority;
    MI_ConstUint16Field ExecutionState;
    MI_ConstStringField OtherExecutionDescription;
    MI_ConstDatetimeField CreationDate;
    MI_ConstDatetimeField TerminationDate;
    MI_ConstUint64Field KernelModeTime;
    MI_ConstUint64Field UserModeTime;
    MI_ConstUint64Field WorkingSetSize;
    /* MSFT_WindowsProcess properties */
    MI_ConstStringField CommandLine;
}
MSFT_WindowsProcess;

typedef struct _MSFT_WindowsProcess_Ref
{
    MSFT_WindowsProcess* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsProcess_Ref;

typedef struct _MSFT_WindowsProcess_ConstRef
{
    MI_CONST MSFT_WindowsProcess* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsProcess_ConstRef;

typedef struct _MSFT_WindowsProcess_Array
{
    struct _MSFT_WindowsProcess** data;
    MI_Uint32 size;
}
MSFT_WindowsProcess_Array;

typedef struct _MSFT_WindowsProcess_ConstArray
{
    struct _MSFT_WindowsProcess MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
MSFT_WindowsProcess_ConstArray;

typedef struct _MSFT_WindowsProcess_ArrayRef
{
    MSFT_WindowsProcess_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsProcess_ArrayRef;

typedef struct _MSFT_WindowsProcess_ConstArrayRef
{
    MSFT_WindowsProcess_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsProcess_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl MSFT_WindowsProcess_rtti;

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Construct(
    _Out_ MSFT_WindowsProcess* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &MSFT_WindowsProcess_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clone(
    _In_ const MSFT_WindowsProcess* self,
    _Outptr_ MSFT_WindowsProcess** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL MSFT_WindowsProcess_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &MSFT_WindowsProcess_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Destruct(_Inout_ MSFT_WindowsProcess* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Delete(_Inout_ MSFT_WindowsProcess* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Post(
    _In_ const MSFT_WindowsProcess* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_InstanceID(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_InstanceID(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_InstanceID(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_Caption(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_Caption(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_Caption(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_Description(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_Description(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_Description(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_ElementName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_ElementName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_ElementName(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_InstallDate(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->InstallDate)->value = x;
    ((MI_DatetimeField*)&self->InstallDate)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_InstallDate(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->InstallDate, 0, sizeof(self->InstallDate));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_Name(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_Name(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_Name(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        5);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_OperationalStatus(
    _Inout_ MSFT_WindowsProcess* self,
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

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_OperationalStatus(
    _Inout_ MSFT_WindowsProcess* self,
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

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_OperationalStatus(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        6);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_StatusDescriptions(
    _Inout_ MSFT_WindowsProcess* self,
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

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_StatusDescriptions(
    _Inout_ MSFT_WindowsProcess* self,
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

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_StatusDescriptions(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        7);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_Status(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_Status(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_Status(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        8);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_HealthState(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->HealthState)->value = x;
    ((MI_Uint16Field*)&self->HealthState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_HealthState(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->HealthState, 0, sizeof(self->HealthState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_CommunicationStatus(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->CommunicationStatus)->value = x;
    ((MI_Uint16Field*)&self->CommunicationStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_CommunicationStatus(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->CommunicationStatus, 0, sizeof(self->CommunicationStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_DetailedStatus(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->DetailedStatus)->value = x;
    ((MI_Uint16Field*)&self->DetailedStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_DetailedStatus(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->DetailedStatus, 0, sizeof(self->DetailedStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_OperatingStatus(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->OperatingStatus)->value = x;
    ((MI_Uint16Field*)&self->OperatingStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_OperatingStatus(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->OperatingStatus, 0, sizeof(self->OperatingStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_PrimaryStatus(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->PrimaryStatus)->value = x;
    ((MI_Uint16Field*)&self->PrimaryStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_PrimaryStatus(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->PrimaryStatus, 0, sizeof(self->PrimaryStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_EnabledState(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->EnabledState)->value = x;
    ((MI_Uint16Field*)&self->EnabledState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_EnabledState(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->EnabledState, 0, sizeof(self->EnabledState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_OtherEnabledState(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        15,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_OtherEnabledState(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        15,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_OtherEnabledState(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        15);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_RequestedState(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->RequestedState)->value = x;
    ((MI_Uint16Field*)&self->RequestedState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_RequestedState(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->RequestedState, 0, sizeof(self->RequestedState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_EnabledDefault(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->EnabledDefault)->value = x;
    ((MI_Uint16Field*)&self->EnabledDefault)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_EnabledDefault(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->EnabledDefault, 0, sizeof(self->EnabledDefault));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_TimeOfLastStateChange(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->TimeOfLastStateChange)->value = x;
    ((MI_DatetimeField*)&self->TimeOfLastStateChange)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_TimeOfLastStateChange(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->TimeOfLastStateChange, 0, sizeof(self->TimeOfLastStateChange));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_AvailableRequestedStates(
    _Inout_ MSFT_WindowsProcess* self,
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

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_AvailableRequestedStates(
    _Inout_ MSFT_WindowsProcess* self,
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

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_AvailableRequestedStates(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        19);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_TransitioningToState(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->TransitioningToState)->value = x;
    ((MI_Uint16Field*)&self->TransitioningToState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_TransitioningToState(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->TransitioningToState, 0, sizeof(self->TransitioningToState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_CSCreationClassName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        21,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_CSCreationClassName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        21,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_CSCreationClassName(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        21);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_CSName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        22,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_CSName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        22,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_CSName(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        22);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_OSCreationClassName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        23,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_OSCreationClassName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        23,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_OSCreationClassName(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        23);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_OSName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        24,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_OSName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        24,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_OSName(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        24);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_CreationClassName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        25,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_CreationClassName(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        25,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_CreationClassName(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        25);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_Handle(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        26,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_Handle(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        26,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_Handle(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        26);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_Priority(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->Priority)->value = x;
    ((MI_Uint32Field*)&self->Priority)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_Priority(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->Priority, 0, sizeof(self->Priority));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_ExecutionState(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->ExecutionState)->value = x;
    ((MI_Uint16Field*)&self->ExecutionState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_ExecutionState(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->ExecutionState, 0, sizeof(self->ExecutionState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_OtherExecutionDescription(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        29,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_OtherExecutionDescription(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        29,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_OtherExecutionDescription(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        29);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_CreationDate(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->CreationDate)->value = x;
    ((MI_DatetimeField*)&self->CreationDate)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_CreationDate(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->CreationDate, 0, sizeof(self->CreationDate));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_TerminationDate(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->TerminationDate)->value = x;
    ((MI_DatetimeField*)&self->TerminationDate)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_TerminationDate(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->TerminationDate, 0, sizeof(self->TerminationDate));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_KernelModeTime(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint64 x)
{
    ((MI_Uint64Field*)&self->KernelModeTime)->value = x;
    ((MI_Uint64Field*)&self->KernelModeTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_KernelModeTime(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->KernelModeTime, 0, sizeof(self->KernelModeTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_UserModeTime(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint64 x)
{
    ((MI_Uint64Field*)&self->UserModeTime)->value = x;
    ((MI_Uint64Field*)&self->UserModeTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_UserModeTime(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->UserModeTime, 0, sizeof(self->UserModeTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_WorkingSetSize(
    _Inout_ MSFT_WindowsProcess* self,
    _In_ MI_Uint64 x)
{
    ((MI_Uint64Field*)&self->WorkingSetSize)->value = x;
    ((MI_Uint64Field*)&self->WorkingSetSize)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_WorkingSetSize(
    _Inout_ MSFT_WindowsProcess* self)
{
    memset((void*)&self->WorkingSetSize, 0, sizeof(self->WorkingSetSize));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Set_CommandLine(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        35,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPtr_CommandLine(
    _Inout_ MSFT_WindowsProcess* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        35,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Clear_CommandLine(
    _Inout_ MSFT_WindowsProcess* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        35);
}

/*
**==============================================================================
**
** MSFT_WindowsProcess.RequestStateChange()
**
**==============================================================================
*/

typedef struct _MSFT_WindowsProcess_RequestStateChange
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
    /*IN*/ MI_ConstUint16Field RequestedState;
    /*OUT*/ CIM_ConcreteJob_ConstRef Job;
    /*IN*/ MI_ConstDatetimeField TimeoutPeriod;
}
MSFT_WindowsProcess_RequestStateChange;

MI_EXTERN_C MI_CONST MI_MethodDecl MSFT_WindowsProcess_RequestStateChange_rtti;

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Construct(
    _Out_ MSFT_WindowsProcess_RequestStateChange* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructParameters(context, &MSFT_WindowsProcess_RequestStateChange_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Clone(
    _In_ const MSFT_WindowsProcess_RequestStateChange* self,
    _Outptr_ MSFT_WindowsProcess_RequestStateChange** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Destruct(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Delete(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Post(
    _In_ const MSFT_WindowsProcess_RequestStateChange* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Set_MIReturn(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Clear_MIReturn(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Set_RequestedState(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->RequestedState)->value = x;
    ((MI_Uint16Field*)&self->RequestedState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Clear_RequestedState(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self)
{
    memset((void*)&self->RequestedState, 0, sizeof(self->RequestedState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Set_Job(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self,
    _In_ const CIM_ConcreteJob* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&x,
        MI_REFERENCE,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_SetPtr_Job(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self,
    _In_ const CIM_ConcreteJob* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&x,
        MI_REFERENCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Clear_Job(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Set_TimeoutPeriod(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->TimeoutPeriod)->value = x;
    ((MI_DatetimeField*)&self->TimeoutPeriod)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_RequestStateChange_Clear_TimeoutPeriod(
    _Inout_ MSFT_WindowsProcess_RequestStateChange* self)
{
    memset((void*)&self->TimeoutPeriod, 0, sizeof(self->TimeoutPeriod));
    return MI_RESULT_OK;
}

/*
**==============================================================================
**
** MSFT_WindowsProcess.SetPriority()
**
**==============================================================================
*/

typedef struct _MSFT_WindowsProcess_SetPriority
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
    /*IN*/ MI_ConstUint32Field Priority;
}
MSFT_WindowsProcess_SetPriority;

MI_EXTERN_C MI_CONST MI_MethodDecl MSFT_WindowsProcess_SetPriority_rtti;

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPriority_Construct(
    _Out_ MSFT_WindowsProcess_SetPriority* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructParameters(context, &MSFT_WindowsProcess_SetPriority_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPriority_Clone(
    _In_ const MSFT_WindowsProcess_SetPriority* self,
    _Outptr_ MSFT_WindowsProcess_SetPriority** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPriority_Destruct(
    _Inout_ MSFT_WindowsProcess_SetPriority* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPriority_Delete(
    _Inout_ MSFT_WindowsProcess_SetPriority* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPriority_Post(
    _In_ const MSFT_WindowsProcess_SetPriority* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPriority_Set_MIReturn(
    _Inout_ MSFT_WindowsProcess_SetPriority* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPriority_Clear_MIReturn(
    _Inout_ MSFT_WindowsProcess_SetPriority* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPriority_Set_Priority(
    _Inout_ MSFT_WindowsProcess_SetPriority* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->Priority)->value = x;
    ((MI_Uint32Field*)&self->Priority)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_SetPriority_Clear_Priority(
    _Inout_ MSFT_WindowsProcess_SetPriority* self)
{
    memset((void*)&self->Priority, 0, sizeof(self->Priority));
    return MI_RESULT_OK;
}

/*
**==============================================================================
**
** MSFT_WindowsProcess.Create()
**
**==============================================================================
*/

typedef struct _MSFT_WindowsProcess_Create
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
    /*IN*/ MI_ConstStringField CommandLine;
    /*OUT*/ CIM_Process_ConstRef Process;
}
MSFT_WindowsProcess_Create;

MI_EXTERN_C MI_CONST MI_MethodDecl MSFT_WindowsProcess_Create_rtti;

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Construct(
    _Out_ MSFT_WindowsProcess_Create* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructParameters(context, &MSFT_WindowsProcess_Create_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Clone(
    _In_ const MSFT_WindowsProcess_Create* self,
    _Outptr_ MSFT_WindowsProcess_Create** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Destruct(
    _Inout_ MSFT_WindowsProcess_Create* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Delete(
    _Inout_ MSFT_WindowsProcess_Create* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Post(
    _In_ const MSFT_WindowsProcess_Create* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Set_MIReturn(
    _Inout_ MSFT_WindowsProcess_Create* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Clear_MIReturn(
    _Inout_ MSFT_WindowsProcess_Create* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Set_CommandLine(
    _Inout_ MSFT_WindowsProcess_Create* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_SetPtr_CommandLine(
    _Inout_ MSFT_WindowsProcess_Create* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Clear_CommandLine(
    _Inout_ MSFT_WindowsProcess_Create* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Set_Process(
    _Inout_ MSFT_WindowsProcess_Create* self,
    _In_ const CIM_Process* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&x,
        MI_REFERENCE,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_SetPtr_Process(
    _Inout_ MSFT_WindowsProcess_Create* self,
    _In_ const CIM_Process* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&x,
        MI_REFERENCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsProcess_Create_Clear_Process(
    _Inout_ MSFT_WindowsProcess_Create* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

/*
**==============================================================================
**
** MSFT_WindowsProcess provider function prototypes
**
**==============================================================================
*/

/* The developer may optionally define this structure */
typedef struct _MSFT_WindowsProcess_Self MSFT_WindowsProcess_Self;

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_Load(
    _Outptr_result_maybenull_ MSFT_WindowsProcess_Self** self,
    _In_opt_ MI_Module_Self* selfModule,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_Unload(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_EnumerateInstances(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_ const MI_PropertySet* propertySet,
    _In_ MI_Boolean keysOnly,
    _In_opt_ const MI_Filter* filter);

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_GetInstance(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MI_PropertySet* propertySet);

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_CreateInstance(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsProcess* newInstance);

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_ModifyInstance(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsProcess* modifiedInstance,
    _In_opt_ const MI_PropertySet* propertySet);

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_DeleteInstance(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ const MSFT_WindowsProcess* instanceName);

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_Invoke_RequestStateChange(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_z_ const MI_Char* methodName,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MSFT_WindowsProcess_RequestStateChange* in);

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_Invoke_SetPriority(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_z_ const MI_Char* methodName,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MSFT_WindowsProcess_SetPriority* in);

MI_EXTERN_C void MI_CALL MSFT_WindowsProcess_Invoke_Create(
    _In_opt_ MSFT_WindowsProcess_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_z_ const MI_Char* methodName,
    _In_ const MSFT_WindowsProcess* instanceName,
    _In_opt_ const MSFT_WindowsProcess_Create* in);


#endif /* _MSFT_WindowsProcess_h */

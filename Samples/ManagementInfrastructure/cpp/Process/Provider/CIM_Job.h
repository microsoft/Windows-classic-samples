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
#ifndef _CIM_Job_h
#define _CIM_Job_h

#include <MI.h>
#include "CIM_LogicalElement.h"

/*
**==============================================================================
**
** CIM_Job [CIM_Job]
**
** Keys:
**
**==============================================================================
*/

typedef struct _CIM_Job /* extends CIM_LogicalElement */
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
    /* CIM_Job properties */
    MI_ConstStringField JobStatus;
    MI_ConstDatetimeField TimeSubmitted;
    MI_ConstDatetimeField ScheduledStartTime;
    MI_ConstDatetimeField StartTime;
    MI_ConstDatetimeField ElapsedTime;
    MI_ConstUint32Field JobRunTimes;
    MI_ConstUint8Field RunMonth;
    MI_ConstSint8Field RunDay;
    MI_ConstSint8Field RunDayOfWeek;
    MI_ConstDatetimeField RunStartInterval;
    MI_ConstUint16Field LocalOrUtcTime;
    MI_ConstDatetimeField UntilTime;
    MI_ConstStringField Notify;
    MI_ConstStringField Owner;
    MI_ConstUint32Field Priority;
    MI_ConstUint16Field PercentComplete;
    MI_ConstBooleanField DeleteOnCompletion;
    MI_ConstUint16Field ErrorCode;
    MI_ConstStringField ErrorDescription;
    MI_ConstUint16Field RecoveryAction;
    MI_ConstStringField OtherRecoveryAction;
}
CIM_Job;

typedef struct _CIM_Job_Ref
{
    CIM_Job* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_Job_Ref;

typedef struct _CIM_Job_ConstRef
{
    MI_CONST CIM_Job* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_Job_ConstRef;

typedef struct _CIM_Job_Array
{
    struct _CIM_Job** data;
    MI_Uint32 size;
}
CIM_Job_Array;

typedef struct _CIM_Job_ConstArray
{
    struct _CIM_Job MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
CIM_Job_ConstArray;

typedef struct _CIM_Job_ArrayRef
{
    CIM_Job_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_Job_ArrayRef;

typedef struct _CIM_Job_ConstArrayRef
{
    CIM_Job_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_Job_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl CIM_Job_rtti;

MI_INLINE MI_Result MI_CALL CIM_Job_Construct(
    _Out_ CIM_Job* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &CIM_Job_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clone(
    _In_ const CIM_Job* self,
    _Outptr_ CIM_Job** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL CIM_Job_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &CIM_Job_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Destruct(_Inout_ CIM_Job* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Delete(_Inout_ CIM_Job* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Post(
    _In_ const CIM_Job* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_InstanceID(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_InstanceID(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_InstanceID(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_Caption(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_Caption(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_Caption(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_Description(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_Description(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        2,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_Description(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        2);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_ElementName(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_ElementName(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        3,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_ElementName(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        3);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_InstallDate(
    _Inout_ CIM_Job* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->InstallDate)->value = x;
    ((MI_DatetimeField*)&self->InstallDate)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_InstallDate(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->InstallDate, 0, sizeof(self->InstallDate));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_Name(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_Name(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_Name(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        5);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_OperationalStatus(
    _Inout_ CIM_Job* self,
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

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_OperationalStatus(
    _Inout_ CIM_Job* self,
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

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_OperationalStatus(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        6);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_StatusDescriptions(
    _Inout_ CIM_Job* self,
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

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_StatusDescriptions(
    _Inout_ CIM_Job* self,
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

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_StatusDescriptions(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        7);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_Status(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_Status(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_Status(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        8);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_HealthState(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->HealthState)->value = x;
    ((MI_Uint16Field*)&self->HealthState)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_HealthState(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->HealthState, 0, sizeof(self->HealthState));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_CommunicationStatus(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->CommunicationStatus)->value = x;
    ((MI_Uint16Field*)&self->CommunicationStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_CommunicationStatus(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->CommunicationStatus, 0, sizeof(self->CommunicationStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_DetailedStatus(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->DetailedStatus)->value = x;
    ((MI_Uint16Field*)&self->DetailedStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_DetailedStatus(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->DetailedStatus, 0, sizeof(self->DetailedStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_OperatingStatus(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->OperatingStatus)->value = x;
    ((MI_Uint16Field*)&self->OperatingStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_OperatingStatus(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->OperatingStatus, 0, sizeof(self->OperatingStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_PrimaryStatus(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->PrimaryStatus)->value = x;
    ((MI_Uint16Field*)&self->PrimaryStatus)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_PrimaryStatus(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->PrimaryStatus, 0, sizeof(self->PrimaryStatus));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_JobStatus(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        14,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_JobStatus(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        14,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_JobStatus(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        14);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_TimeSubmitted(
    _Inout_ CIM_Job* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->TimeSubmitted)->value = x;
    ((MI_DatetimeField*)&self->TimeSubmitted)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_TimeSubmitted(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->TimeSubmitted, 0, sizeof(self->TimeSubmitted));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_ScheduledStartTime(
    _Inout_ CIM_Job* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->ScheduledStartTime)->value = x;
    ((MI_DatetimeField*)&self->ScheduledStartTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_ScheduledStartTime(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->ScheduledStartTime, 0, sizeof(self->ScheduledStartTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_StartTime(
    _Inout_ CIM_Job* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->StartTime)->value = x;
    ((MI_DatetimeField*)&self->StartTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_StartTime(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->StartTime, 0, sizeof(self->StartTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_ElapsedTime(
    _Inout_ CIM_Job* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->ElapsedTime)->value = x;
    ((MI_DatetimeField*)&self->ElapsedTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_ElapsedTime(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->ElapsedTime, 0, sizeof(self->ElapsedTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_JobRunTimes(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->JobRunTimes)->value = x;
    ((MI_Uint32Field*)&self->JobRunTimes)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_JobRunTimes(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->JobRunTimes, 0, sizeof(self->JobRunTimes));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_RunMonth(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint8 x)
{
    ((MI_Uint8Field*)&self->RunMonth)->value = x;
    ((MI_Uint8Field*)&self->RunMonth)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_RunMonth(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->RunMonth, 0, sizeof(self->RunMonth));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_RunDay(
    _Inout_ CIM_Job* self,
    _In_ MI_Sint8 x)
{
    ((MI_Sint8Field*)&self->RunDay)->value = x;
    ((MI_Sint8Field*)&self->RunDay)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_RunDay(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->RunDay, 0, sizeof(self->RunDay));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_RunDayOfWeek(
    _Inout_ CIM_Job* self,
    _In_ MI_Sint8 x)
{
    ((MI_Sint8Field*)&self->RunDayOfWeek)->value = x;
    ((MI_Sint8Field*)&self->RunDayOfWeek)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_RunDayOfWeek(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->RunDayOfWeek, 0, sizeof(self->RunDayOfWeek));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_RunStartInterval(
    _Inout_ CIM_Job* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->RunStartInterval)->value = x;
    ((MI_DatetimeField*)&self->RunStartInterval)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_RunStartInterval(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->RunStartInterval, 0, sizeof(self->RunStartInterval));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_LocalOrUtcTime(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->LocalOrUtcTime)->value = x;
    ((MI_Uint16Field*)&self->LocalOrUtcTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_LocalOrUtcTime(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->LocalOrUtcTime, 0, sizeof(self->LocalOrUtcTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_UntilTime(
    _Inout_ CIM_Job* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->UntilTime)->value = x;
    ((MI_DatetimeField*)&self->UntilTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_UntilTime(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->UntilTime, 0, sizeof(self->UntilTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_Notify(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        26,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_Notify(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        26,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_Notify(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        26);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_Owner(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        27,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_Owner(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        27,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_Owner(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        27);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_Priority(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->Priority)->value = x;
    ((MI_Uint32Field*)&self->Priority)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_Priority(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->Priority, 0, sizeof(self->Priority));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_PercentComplete(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->PercentComplete)->value = x;
    ((MI_Uint16Field*)&self->PercentComplete)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_PercentComplete(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->PercentComplete, 0, sizeof(self->PercentComplete));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_DeleteOnCompletion(
    _Inout_ CIM_Job* self,
    _In_ MI_Boolean x)
{
    ((MI_BooleanField*)&self->DeleteOnCompletion)->value = x;
    ((MI_BooleanField*)&self->DeleteOnCompletion)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_DeleteOnCompletion(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->DeleteOnCompletion, 0, sizeof(self->DeleteOnCompletion));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_ErrorCode(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->ErrorCode)->value = x;
    ((MI_Uint16Field*)&self->ErrorCode)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_ErrorCode(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->ErrorCode, 0, sizeof(self->ErrorCode));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_ErrorDescription(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        32,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_ErrorDescription(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        32,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_ErrorDescription(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        32);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_RecoveryAction(
    _Inout_ CIM_Job* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->RecoveryAction)->value = x;
    ((MI_Uint16Field*)&self->RecoveryAction)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_RecoveryAction(
    _Inout_ CIM_Job* self)
{
    memset((void*)&self->RecoveryAction, 0, sizeof(self->RecoveryAction));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_Set_OtherRecoveryAction(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        34,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Job_SetPtr_OtherRecoveryAction(
    _Inout_ CIM_Job* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        34,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Job_Clear_OtherRecoveryAction(
    _Inout_ CIM_Job* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        34);
}

/*
**==============================================================================
**
** CIM_Job.KillJob()
**
**==============================================================================
*/

typedef struct _CIM_Job_KillJob
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
    /*IN*/ MI_ConstBooleanField DeleteOnKill;
}
CIM_Job_KillJob;

MI_EXTERN_C MI_CONST MI_MethodDecl CIM_Job_KillJob_rtti;

MI_INLINE MI_Result MI_CALL CIM_Job_KillJob_Construct(
    _Out_ CIM_Job_KillJob* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructParameters(context, &CIM_Job_KillJob_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Job_KillJob_Clone(
    _In_ const CIM_Job_KillJob* self,
    _Outptr_ CIM_Job_KillJob** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL CIM_Job_KillJob_Destruct(
    _Inout_ CIM_Job_KillJob* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Job_KillJob_Delete(
    _Inout_ CIM_Job_KillJob* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Job_KillJob_Post(
    _In_ const CIM_Job_KillJob* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Job_KillJob_Set_MIReturn(
    _Inout_ CIM_Job_KillJob* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_KillJob_Clear_MIReturn(
    _Inout_ CIM_Job_KillJob* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_KillJob_Set_DeleteOnKill(
    _Inout_ CIM_Job_KillJob* self,
    _In_ MI_Boolean x)
{
    ((MI_BooleanField*)&self->DeleteOnKill)->value = x;
    ((MI_BooleanField*)&self->DeleteOnKill)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Job_KillJob_Clear_DeleteOnKill(
    _Inout_ CIM_Job_KillJob* self)
{
    memset((void*)&self->DeleteOnKill, 0, sizeof(self->DeleteOnKill));
    return MI_RESULT_OK;
}


#endif /* _CIM_Job_h */

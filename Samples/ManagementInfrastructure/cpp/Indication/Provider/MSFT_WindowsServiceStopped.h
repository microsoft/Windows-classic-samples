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
#ifndef _MSFT_WindowsServiceStopped_h
#define _MSFT_WindowsServiceStopped_h

#include <MI.h>
#include "CIM_InstModification.h"

/*
**==============================================================================
**
** MSFT_WindowsServiceStopped [MSFT_WindowsServiceStopped]
**
** Keys:
**
**==============================================================================
*/

typedef struct _MSFT_WindowsServiceStopped /* extends CIM_InstModification */
{
    MI_Instance __instance;
    /* CIM_Indication properties */
    MI_ConstStringField IndicationIdentifier;
    MI_ConstStringAField CorrelatedIndications;
    MI_ConstDatetimeField IndicationTime;
    MI_ConstUint16Field PerceivedSeverity;
    MI_ConstStringField OtherSeverity;
    MI_ConstStringField IndicationFilterName;
    MI_ConstStringField SequenceContext;
    MI_ConstSint64Field SequenceNumber;
    /* CIM_InstIndication properties */
    MI_ConstReferenceField SourceInstance;
    MI_ConstStringField SourceInstanceModelPath;
    MI_ConstStringField SourceInstanceHost;
    /* CIM_InstModification properties */
    MI_ConstReferenceField PreviousInstance;
    /* MSFT_WindowsServiceStopped properties */
}
MSFT_WindowsServiceStopped;

typedef struct _MSFT_WindowsServiceStopped_Ref
{
    MSFT_WindowsServiceStopped* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceStopped_Ref;

typedef struct _MSFT_WindowsServiceStopped_ConstRef
{
    MI_CONST MSFT_WindowsServiceStopped* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceStopped_ConstRef;

typedef struct _MSFT_WindowsServiceStopped_Array
{
    struct _MSFT_WindowsServiceStopped** data;
    MI_Uint32 size;
}
MSFT_WindowsServiceStopped_Array;

typedef struct _MSFT_WindowsServiceStopped_ConstArray
{
    struct _MSFT_WindowsServiceStopped MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
MSFT_WindowsServiceStopped_ConstArray;

typedef struct _MSFT_WindowsServiceStopped_ArrayRef
{
    MSFT_WindowsServiceStopped_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceStopped_ArrayRef;

typedef struct _MSFT_WindowsServiceStopped_ConstArrayRef
{
    MSFT_WindowsServiceStopped_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceStopped_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl MSFT_WindowsServiceStopped_rtti;

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Construct(
    _Out_ MSFT_WindowsServiceStopped* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &MSFT_WindowsServiceStopped_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clone(
    _In_ const MSFT_WindowsServiceStopped* self,
    _Outptr_ MSFT_WindowsServiceStopped** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL MSFT_WindowsServiceStopped_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &MSFT_WindowsServiceStopped_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Destruct(_Inout_ MSFT_WindowsServiceStopped* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Delete(_Inout_ MSFT_WindowsServiceStopped* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Post(
    _In_ const MSFT_WindowsServiceStopped* self,
    _In_ MI_Context* context,
    MI_Uint32 subscriptionIDCount,
    _In_z_ const MI_Char* bookmark)
{
    return MI_Context_PostIndication(context,
        &self->__instance,
        subscriptionIDCount,
        bookmark);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_IndicationIdentifier(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_SetPtr_IndicationIdentifier(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_IndicationIdentifier(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_CorrelatedIndications(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_reads_opt_(size) const MI_Char** data,
    _In_ MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&arr,
        MI_STRINGA,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_SetPtr_CorrelatedIndications(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_reads_opt_(size) const MI_Char** data,
    _In_ MI_Uint32 size)
{
    MI_Array arr;
    arr.data = (void*)data;
    arr.size = size;
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        1,
        (MI_Value*)&arr,
        MI_STRINGA,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_CorrelatedIndications(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_IndicationTime(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->IndicationTime)->value = x;
    ((MI_DatetimeField*)&self->IndicationTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_IndicationTime(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    memset((void*)&self->IndicationTime, 0, sizeof(self->IndicationTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_PerceivedSeverity(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->PerceivedSeverity)->value = x;
    ((MI_Uint16Field*)&self->PerceivedSeverity)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_PerceivedSeverity(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    memset((void*)&self->PerceivedSeverity, 0, sizeof(self->PerceivedSeverity));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_OtherSeverity(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_SetPtr_OtherSeverity(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_OtherSeverity(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        4);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_IndicationFilterName(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_SetPtr_IndicationFilterName(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_IndicationFilterName(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        5);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_SequenceContext(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        6,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_SetPtr_SequenceContext(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        6,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_SequenceContext(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        6);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_SequenceNumber(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_ MI_Sint64 x)
{
    ((MI_Sint64Field*)&self->SequenceNumber)->value = x;
    ((MI_Sint64Field*)&self->SequenceNumber)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_SequenceNumber(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    memset((void*)&self->SequenceNumber, 0, sizeof(self->SequenceNumber));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_SourceInstance(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_ const MI_Instance* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&x,
        MI_INSTANCE,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_SetPtr_SourceInstance(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_ const MI_Instance* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_SourceInstance(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        8);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_SourceInstanceModelPath(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        9,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_SetPtr_SourceInstanceModelPath(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        9,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_SourceInstanceModelPath(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        9);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_SourceInstanceHost(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        10,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_SetPtr_SourceInstanceHost(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        10,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_SourceInstanceHost(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        10);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Set_PreviousInstance(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_ const MI_Instance* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        11,
        (MI_Value*)&x,
        MI_INSTANCE,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_SetPtr_PreviousInstance(
    _Inout_ MSFT_WindowsServiceStopped* self,
    _In_ const MI_Instance* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        11,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStopped_Clear_PreviousInstance(
    _Inout_ MSFT_WindowsServiceStopped* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        11);
}

/*
**==============================================================================
**
** MSFT_WindowsServiceStopped provider function prototypes
**
**==============================================================================
*/

/* The developer may optionally define this structure */
typedef struct _MSFT_WindowsServiceStopped_Self MSFT_WindowsServiceStopped_Self;

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStopped_Load(
    _Outptr_result_maybenull_ MSFT_WindowsServiceStopped_Self** self,
    _In_opt_ MI_Module_Self* selfModule,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStopped_Unload(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStopped_EnableIndications(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* indicationsContext,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStopped_DisableIndications(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* indicationsContext,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStopped_Subscribe(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_ const MI_Filter* filter,
    _In_opt_z_ const MI_Char* bookmark,
    _In_ MI_Uint64  subscriptionID,
    _Outptr_result_maybenull_ void** subscriptionSelf);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStopped_Unsubscribe(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ MI_Uint64  subscriptionID,
    _In_opt_ void* subscriptionSelf);


#endif /* _MSFT_WindowsServiceStopped_h */

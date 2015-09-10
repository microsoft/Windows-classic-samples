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
#ifndef _MSFT_WindowsServiceStarted_h
#define _MSFT_WindowsServiceStarted_h

#include <MI.h>
#include "CIM_InstModification.h"

/*
**==============================================================================
**
** MSFT_WindowsServiceStarted [MSFT_WindowsServiceStarted]
**
** Keys:
**
**==============================================================================
*/

typedef struct _MSFT_WindowsServiceStarted /* extends CIM_InstModification */
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
    /* MSFT_WindowsServiceStarted properties */
}
MSFT_WindowsServiceStarted;

typedef struct _MSFT_WindowsServiceStarted_Ref
{
    MSFT_WindowsServiceStarted* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceStarted_Ref;

typedef struct _MSFT_WindowsServiceStarted_ConstRef
{
    MI_CONST MSFT_WindowsServiceStarted* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceStarted_ConstRef;

typedef struct _MSFT_WindowsServiceStarted_Array
{
    struct _MSFT_WindowsServiceStarted** data;
    MI_Uint32 size;
}
MSFT_WindowsServiceStarted_Array;

typedef struct _MSFT_WindowsServiceStarted_ConstArray
{
    struct _MSFT_WindowsServiceStarted MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
MSFT_WindowsServiceStarted_ConstArray;

typedef struct _MSFT_WindowsServiceStarted_ArrayRef
{
    MSFT_WindowsServiceStarted_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceStarted_ArrayRef;

typedef struct _MSFT_WindowsServiceStarted_ConstArrayRef
{
    MSFT_WindowsServiceStarted_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceStarted_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl MSFT_WindowsServiceStarted_rtti;

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Construct(
    _Out_ MSFT_WindowsServiceStarted* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &MSFT_WindowsServiceStarted_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clone(
    _In_ const MSFT_WindowsServiceStarted* self,
    _Outptr_ MSFT_WindowsServiceStarted** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL MSFT_WindowsServiceStarted_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &MSFT_WindowsServiceStarted_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Destruct(_Inout_ MSFT_WindowsServiceStarted* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Delete(_Inout_ MSFT_WindowsServiceStarted* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Post(
    _In_ const MSFT_WindowsServiceStarted* self,
    _In_ MI_Context* context,
    MI_Uint32 subscriptionIDCount,
    _In_z_ const MI_Char* bookmark)
{
    return MI_Context_PostIndication(context,
        &self->__instance,
        subscriptionIDCount,
        bookmark);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_IndicationIdentifier(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_SetPtr_IndicationIdentifier(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_IndicationIdentifier(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_CorrelatedIndications(
    _Inout_ MSFT_WindowsServiceStarted* self,
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

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_SetPtr_CorrelatedIndications(
    _Inout_ MSFT_WindowsServiceStarted* self,
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

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_CorrelatedIndications(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_IndicationTime(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->IndicationTime)->value = x;
    ((MI_DatetimeField*)&self->IndicationTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_IndicationTime(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    memset((void*)&self->IndicationTime, 0, sizeof(self->IndicationTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_PerceivedSeverity(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->PerceivedSeverity)->value = x;
    ((MI_Uint16Field*)&self->PerceivedSeverity)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_PerceivedSeverity(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    memset((void*)&self->PerceivedSeverity, 0, sizeof(self->PerceivedSeverity));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_OtherSeverity(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_SetPtr_OtherSeverity(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_OtherSeverity(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        4);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_IndicationFilterName(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_SetPtr_IndicationFilterName(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_IndicationFilterName(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        5);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_SequenceContext(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        6,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_SetPtr_SequenceContext(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        6,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_SequenceContext(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        6);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_SequenceNumber(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_ MI_Sint64 x)
{
    ((MI_Sint64Field*)&self->SequenceNumber)->value = x;
    ((MI_Sint64Field*)&self->SequenceNumber)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_SequenceNumber(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    memset((void*)&self->SequenceNumber, 0, sizeof(self->SequenceNumber));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_SourceInstance(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_ const MI_Instance* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&x,
        MI_INSTANCE,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_SetPtr_SourceInstance(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_ const MI_Instance* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        8,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_SourceInstance(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        8);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_SourceInstanceModelPath(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        9,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_SetPtr_SourceInstanceModelPath(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        9,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_SourceInstanceModelPath(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        9);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_SourceInstanceHost(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        10,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_SetPtr_SourceInstanceHost(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        10,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_SourceInstanceHost(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        10);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Set_PreviousInstance(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_ const MI_Instance* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        11,
        (MI_Value*)&x,
        MI_INSTANCE,
        0);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_SetPtr_PreviousInstance(
    _Inout_ MSFT_WindowsServiceStarted* self,
    _In_ const MI_Instance* x)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        11,
        (MI_Value*)&x,
        MI_INSTANCE,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceStarted_Clear_PreviousInstance(
    _Inout_ MSFT_WindowsServiceStarted* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        11);
}

/*
**==============================================================================
**
** MSFT_WindowsServiceStarted provider function prototypes
**
**==============================================================================
*/

/* The developer may optionally define this structure */
typedef struct _MSFT_WindowsServiceStarted_Self MSFT_WindowsServiceStarted_Self;

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStarted_Load(
    _Outptr_result_maybenull_ MSFT_WindowsServiceStarted_Self** self,
    _In_opt_ MI_Module_Self* selfModule,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStarted_Unload(
    _In_opt_ MSFT_WindowsServiceStarted_Self* self,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStarted_EnableIndications(
    _In_opt_ MSFT_WindowsServiceStarted_Self* self,
    _In_ MI_Context* indicationsContext,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStarted_DisableIndications(
    _In_opt_ MSFT_WindowsServiceStarted_Self* self,
    _In_ MI_Context* indicationsContext,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStarted_Subscribe(
    _In_opt_ MSFT_WindowsServiceStarted_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_ const MI_Filter* filter,
    _In_opt_z_ const MI_Char* bookmark,
    _In_ MI_Uint64  subscriptionID,
    _Outptr_result_maybenull_ void** subscriptionSelf);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceStarted_Unsubscribe(
    _In_opt_ MSFT_WindowsServiceStarted_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ MI_Uint64  subscriptionID,
    _In_opt_ void* subscriptionSelf);


#endif /* _MSFT_WindowsServiceStarted_h */

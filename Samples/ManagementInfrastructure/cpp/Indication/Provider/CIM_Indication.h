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
#ifndef _CIM_Indication_h
#define _CIM_Indication_h

#include <MI.h>

/*
**==============================================================================
**
** CIM_Indication [CIM_Indication]
**
** Keys:
**
**==============================================================================
*/

typedef struct _CIM_Indication
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
}
CIM_Indication;

typedef struct _CIM_Indication_Ref
{
    CIM_Indication* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_Indication_Ref;

typedef struct _CIM_Indication_ConstRef
{
    MI_CONST CIM_Indication* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_Indication_ConstRef;

typedef struct _CIM_Indication_Array
{
    struct _CIM_Indication** data;
    MI_Uint32 size;
}
CIM_Indication_Array;

typedef struct _CIM_Indication_ConstArray
{
    struct _CIM_Indication MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
CIM_Indication_ConstArray;

typedef struct _CIM_Indication_ArrayRef
{
    CIM_Indication_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_Indication_ArrayRef;

typedef struct _CIM_Indication_ConstArrayRef
{
    CIM_Indication_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
CIM_Indication_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl CIM_Indication_rtti;

MI_INLINE MI_Result MI_CALL CIM_Indication_Construct(
    _Out_ CIM_Indication* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &CIM_Indication_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Clone(
    _In_ const CIM_Indication* self,
    _Outptr_ CIM_Indication** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL CIM_Indication_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &CIM_Indication_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Destruct(_Inout_ CIM_Indication* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Delete(_Inout_ CIM_Indication* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Post(
    _In_ const CIM_Indication* self,
    _In_ MI_Context* context,
    MI_Uint32 subscriptionIDCount,
    _In_z_ const MI_Char* bookmark)
{
    return MI_Context_PostIndication(context,
        &self->__instance,
        subscriptionIDCount,
        bookmark);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Set_IndicationIdentifier(
    _Inout_ CIM_Indication* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_SetPtr_IndicationIdentifier(
    _Inout_ CIM_Indication* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        0,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Clear_IndicationIdentifier(
    _Inout_ CIM_Indication* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Set_CorrelatedIndications(
    _Inout_ CIM_Indication* self,
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

MI_INLINE MI_Result MI_CALL CIM_Indication_SetPtr_CorrelatedIndications(
    _Inout_ CIM_Indication* self,
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

MI_INLINE MI_Result MI_CALL CIM_Indication_Clear_CorrelatedIndications(
    _Inout_ CIM_Indication* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        1);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Set_IndicationTime(
    _Inout_ CIM_Indication* self,
    _In_ MI_Datetime x)
{
    ((MI_DatetimeField*)&self->IndicationTime)->value = x;
    ((MI_DatetimeField*)&self->IndicationTime)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Clear_IndicationTime(
    _Inout_ CIM_Indication* self)
{
    memset((void*)&self->IndicationTime, 0, sizeof(self->IndicationTime));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Set_PerceivedSeverity(
    _Inout_ CIM_Indication* self,
    _In_ MI_Uint16 x)
{
    ((MI_Uint16Field*)&self->PerceivedSeverity)->value = x;
    ((MI_Uint16Field*)&self->PerceivedSeverity)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Clear_PerceivedSeverity(
    _Inout_ CIM_Indication* self)
{
    memset((void*)&self->PerceivedSeverity, 0, sizeof(self->PerceivedSeverity));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Set_OtherSeverity(
    _Inout_ CIM_Indication* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_SetPtr_OtherSeverity(
    _Inout_ CIM_Indication* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        4,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Clear_OtherSeverity(
    _Inout_ CIM_Indication* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        4);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Set_IndicationFilterName(
    _Inout_ CIM_Indication* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_SetPtr_IndicationFilterName(
    _Inout_ CIM_Indication* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        5,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Clear_IndicationFilterName(
    _Inout_ CIM_Indication* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        5);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Set_SequenceContext(
    _Inout_ CIM_Indication* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        6,
        (MI_Value*)&str,
        MI_STRING,
        0);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_SetPtr_SequenceContext(
    _Inout_ CIM_Indication* self,
    _In_z_ const MI_Char* str)
{
    return self->__instance.ft->SetElementAt(
        (MI_Instance*)&self->__instance,
        6,
        (MI_Value*)&str,
        MI_STRING,
        MI_FLAG_BORROW);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Clear_SequenceContext(
    _Inout_ CIM_Indication* self)
{
    return self->__instance.ft->ClearElementAt(
        (MI_Instance*)&self->__instance,
        6);
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Set_SequenceNumber(
    _Inout_ CIM_Indication* self,
    _In_ MI_Sint64 x)
{
    ((MI_Sint64Field*)&self->SequenceNumber)->value = x;
    ((MI_Sint64Field*)&self->SequenceNumber)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL CIM_Indication_Clear_SequenceNumber(
    _Inout_ CIM_Indication* self)
{
    memset((void*)&self->SequenceNumber, 0, sizeof(self->SequenceNumber));
    return MI_RESULT_OK;
}


#endif /* _CIM_Indication_h */

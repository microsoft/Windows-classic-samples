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
#ifndef _MSFT_WindowsServiceManager_h
#define _MSFT_WindowsServiceManager_h

#include <MI.h>
#include "MSFT_WindowsService.h"

/*
**==============================================================================
**
** MSFT_WindowsServiceManager [MSFT_WindowsServiceManager]
**
** Keys:
**
**==============================================================================
*/

typedef struct _MSFT_WindowsServiceManager
{
    MI_Instance __instance;
    /* MSFT_WindowsServiceManager properties */
}
MSFT_WindowsServiceManager;

typedef struct _MSFT_WindowsServiceManager_Ref
{
    MSFT_WindowsServiceManager* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceManager_Ref;

typedef struct _MSFT_WindowsServiceManager_ConstRef
{
    MI_CONST MSFT_WindowsServiceManager* value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceManager_ConstRef;

typedef struct _MSFT_WindowsServiceManager_Array
{
    struct _MSFT_WindowsServiceManager** data;
    MI_Uint32 size;
}
MSFT_WindowsServiceManager_Array;

typedef struct _MSFT_WindowsServiceManager_ConstArray
{
    struct _MSFT_WindowsServiceManager MI_CONST* MI_CONST* data;
    MI_Uint32 size;
}
MSFT_WindowsServiceManager_ConstArray;

typedef struct _MSFT_WindowsServiceManager_ArrayRef
{
    MSFT_WindowsServiceManager_Array value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceManager_ArrayRef;

typedef struct _MSFT_WindowsServiceManager_ConstArrayRef
{
    MSFT_WindowsServiceManager_ConstArray value;
    MI_Boolean exists;
    MI_Uint8 flags;
}
MSFT_WindowsServiceManager_ConstArrayRef;

MI_EXTERN_C MI_CONST MI_ClassDecl MSFT_WindowsServiceManager_rtti;

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_Construct(
    _Out_ MSFT_WindowsServiceManager* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructInstance(context, &MSFT_WindowsServiceManager_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_Clone(
    _In_ const MSFT_WindowsServiceManager* self,
    _Outptr_ MSFT_WindowsServiceManager** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Boolean MI_CALL MSFT_WindowsServiceManager_IsA(
    _In_ const MI_Instance* self)
{
    MI_Boolean res = MI_FALSE;
    return MI_Instance_IsA(self, &MSFT_WindowsServiceManager_rtti, &res) == MI_RESULT_OK && res;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_Destruct(_Inout_ MSFT_WindowsServiceManager* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_Delete(_Inout_ MSFT_WindowsServiceManager* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_Post(
    _In_ const MSFT_WindowsServiceManager* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

/*
**==============================================================================
**
** MSFT_WindowsServiceManager.GetWindowsServices()
**
**==============================================================================
*/

typedef struct _MSFT_WindowsServiceManager_GetWindowsServices
{
    MI_Instance __instance;
    /*OUT*/ MI_ConstUint32Field MIReturn;
    /*IN*/ MI_ConstUint32Field status;
}
MSFT_WindowsServiceManager_GetWindowsServices;

MI_EXTERN_C MI_CONST MI_MethodDecl MSFT_WindowsServiceManager_GetWindowsServices_rtti;

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_GetWindowsServices_Construct(
    _Out_ MSFT_WindowsServiceManager_GetWindowsServices* self,
    _In_ MI_Context* context)
{
    return MI_Context_ConstructParameters(context, &MSFT_WindowsServiceManager_GetWindowsServices_rtti,
        (MI_Instance*)&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_GetWindowsServices_Clone(
    _In_ const MSFT_WindowsServiceManager_GetWindowsServices* self,
    _Outptr_ MSFT_WindowsServiceManager_GetWindowsServices** newInstance)
{
    return MI_Instance_Clone(
        &self->__instance, (MI_Instance**)newInstance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_GetWindowsServices_Destruct(
    _Inout_ MSFT_WindowsServiceManager_GetWindowsServices* self)
{
    return MI_Instance_Destruct(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_GetWindowsServices_Delete(
    _Inout_ MSFT_WindowsServiceManager_GetWindowsServices* self)
{
    return MI_Instance_Delete(&self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_GetWindowsServices_Post(
    _In_ const MSFT_WindowsServiceManager_GetWindowsServices* self,
    _In_ MI_Context* context)
{
    return MI_Context_PostInstance(context, &self->__instance);
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_GetWindowsServices_Set_MIReturn(
    _Inout_ MSFT_WindowsServiceManager_GetWindowsServices* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->MIReturn)->value = x;
    ((MI_Uint32Field*)&self->MIReturn)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_GetWindowsServices_Clear_MIReturn(
    _Inout_ MSFT_WindowsServiceManager_GetWindowsServices* self)
{
    memset((void*)&self->MIReturn, 0, sizeof(self->MIReturn));
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_GetWindowsServices_Set_status(
    _Inout_ MSFT_WindowsServiceManager_GetWindowsServices* self,
    _In_ MI_Uint32 x)
{
    ((MI_Uint32Field*)&self->status)->value = x;
    ((MI_Uint32Field*)&self->status)->exists = 1;
    return MI_RESULT_OK;
}

MI_INLINE MI_Result MI_CALL MSFT_WindowsServiceManager_GetWindowsServices_Clear_status(
    _Inout_ MSFT_WindowsServiceManager_GetWindowsServices* self)
{
    memset((void*)&self->status, 0, sizeof(self->status));
    return MI_RESULT_OK;
}

/*
**==============================================================================
**
** MSFT_WindowsServiceManager provider function prototypes
**
**==============================================================================
*/

/* The developer may optionally define this structure */
typedef struct _MSFT_WindowsServiceManager_Self MSFT_WindowsServiceManager_Self;

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceManager_Load(
    _Outptr_result_maybenull_ MSFT_WindowsServiceManager_Self** self,
    _In_opt_ MI_Module_Self* selfModule,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceManager_Unload(
    _In_opt_ MSFT_WindowsServiceManager_Self* self,
    _In_ MI_Context* context);

MI_EXTERN_C void MI_CALL MSFT_WindowsServiceManager_Invoke_GetWindowsServices(
    _In_opt_ MSFT_WindowsServiceManager_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_z_ const MI_Char* methodName,
    _In_ const MSFT_WindowsServiceManager* instanceName,
    _In_opt_ const MSFT_WindowsServiceManager_GetWindowsServices* in);


#endif /* _MSFT_WindowsServiceManager_h */

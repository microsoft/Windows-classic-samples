//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <MI.h>
#include "MSFT_WindowsServiceStopped.h"
#include "WindowsService.h"

void MI_CALL MSFT_WindowsServiceStopped_Load(
    _Outptr_result_maybenull_ MSFT_WindowsServiceStopped_Self** self,
    _In_opt_ MI_Module_Self* selfModule,
    _In_ MI_Context* context)
{
    MI_UNREFERENCED_PARAMETER(selfModule);

    *self = NULL;
    MI_Context_PostResult(context, MI_RESULT_OK);
}

void MI_CALL MSFT_WindowsServiceStopped_Unload(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* context)
{
    MI_UNREFERENCED_PARAMETER(self);

    MI_Context_PostResult(context, MI_RESULT_OK);
}

void MI_CALL MSFT_WindowsServiceStopped_EnableIndications(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* indicationsContext,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className)
{
    /* TODO: store indicationsContext for later usage */
    /* NOTE: Do not call MI_Context_PostResult on this context */

    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(indicationsContext);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);

    EnableServiceStoppedIndication(indicationsContext, nameSpace);
}

void MI_CALL MSFT_WindowsServiceStopped_DisableIndications(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* indicationsContext,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className)
{
    /* TODO: stop background thread that monitors subscriptions */

    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);

    {
        MI_Result r = DisableServiceStoppedIndication(indicationsContext);
        MI_Context_PostResult(indicationsContext, r);
    }
}

void MI_CALL MSFT_WindowsServiceStopped_Subscribe(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_opt_ const MI_Filter* filter,
    _In_opt_z_ const MI_Char* bookmark,
    _In_ MI_Uint64  subscriptionID,
    _Outptr_result_maybenull_ void** subscriptionSelf)
{
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(filter);
    MI_UNREFERENCED_PARAMETER(bookmark);
    MI_UNREFERENCED_PARAMETER(subscriptionID);
    MI_UNREFERENCED_PARAMETER(subscriptionSelf);

    // whenever a client start a subscription to this indication class,
    // this function will be invoked, query expression can be retrieved
    // from filter parameter.
    *subscriptionSelf = NULL;
    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}

void MI_CALL MSFT_WindowsServiceStopped_Unsubscribe(
    _In_opt_ MSFT_WindowsServiceStopped_Self* self,
    _In_ MI_Context* context,
    _In_opt_z_ const MI_Char* nameSpace,
    _In_opt_z_ const MI_Char* className,
    _In_ MI_Uint64  subscriptionID,
    _In_opt_ void* subscriptionSelf)
{
    MI_UNREFERENCED_PARAMETER(self);
    MI_UNREFERENCED_PARAMETER(nameSpace);
    MI_UNREFERENCED_PARAMETER(className);
    MI_UNREFERENCED_PARAMETER(subscriptionID);
    MI_UNREFERENCED_PARAMETER(subscriptionSelf);

    // whenever a client stop a subscription to this indication class,
    // this function will be invoked. The subscription can be identified
    // by subscriptionID, which is passed along with the call to
    // MSFT_WindowsServiceStopped_Subscribe
    MI_Context_PostResult(context, MI_RESULT_NOT_SUPPORTED);
}


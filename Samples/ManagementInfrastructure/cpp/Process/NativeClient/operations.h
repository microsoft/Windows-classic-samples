//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#pragma once

/* Operation selector */
void Do_Operation(MI_Application *miApplication, _In_opt_z_ const wchar_t *computerName, _In_opt_z_ const wchar_t *protocol, _In_z_ const wchar_t *namespaceName, const wchar_t *className);

/* Carry out enumeration, either synchronously or asynchronously */
void Do_Enumerate(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className);

/* Carry out get, either synchronously or asynchronously */
void Do_Get(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className);

/* Carry out create, either synchronously or asynchronously */
void Do_Create(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className);

/* Carry out delete, either synchronously or asynchronously */
void Do_Delete(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className);

/* Carry out modify, either synchronously or asynchronously */
void Do_Modify(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className);

/* Carry out method invocation, either synchronously or asynchronously */
void Do_Method(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className);

/* Carry out instance query, either synchronously or asynchronously */
void Do_Query(MI_Session *miSession, _In_z_ const wchar_t *namespaceName);

/* Carry out indication subscribe, either synchronously or asynchronously */
void Do_Subscribe(MI_Session *miSession, _In_z_ const wchar_t *namespaceName);

/* Carry out association, either synchronously or asynchronously */
void Do_Association(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className);

/* Carry out reference, either synchronously or asynchronously */
void Do_Reference(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className);

/* Structure used for asynchronous operations to pass context from the main thread to the callback thread */
struct InstanceResultCallback_Context
{
    HANDLE asyncNotificationHandle;
    MI_Uint32 numberInstances;
    MI_Result finalResult;
    MI_Boolean keysOnly;
};

/* Asynchronous instance callback function to dump the results */
void MI_CALL InstanceResultCallback(
    _In_opt_     MI_Operation *miOperation,
    _In_     void *callbackContext, 
    _In_opt_ const MI_Instance *miInstance,
             MI_Boolean moreResults,
    _In_     MI_Result miResult,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation));

/* Asynchronous indication callback function to dump the results */
void MI_CALL IndicationResultCallback(
    _In_opt_     MI_Operation *miOperation,
    _In_     void *callbackContext, 
    _In_opt_ const MI_Instance *miInstance,
    _In_opt_z_ const MI_Char *bookmark,
    _In_opt_z_ const MI_Char *machineID,
             MI_Boolean moreResults,
    _In_     MI_Result miResult,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation));

/* Asynchronous method out parameter streaming callback function. */
void MI_CALL StreamedResultCallback(
    _In_ MI_Operation *operation,
    _In_ void *callbackContext,
    _In_z_ const MI_Char *parameterName,
    _In_ MI_Type resultType,
    _In_ const MI_Value *result,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation));

/* Asynchronous callback for displaying progress information sent from the provider. */
void MI_CALL WriteProgressCallback(
    _In_     MI_Operation *operation,
    _In_opt_ void *callbackContext, 
    _In_z_   const MI_Char *activity,
    _In_z_   const MI_Char *currentOperation,
    _In_z_   const MI_Char *statusDescription,
             MI_Uint32 percentageComplete,
             MI_Uint32 secondsRemaining);

/* Asynchronous callback for displaying message information sent from the provider. */
void MI_CALL WriteMessageCallback(
    _In_     MI_Operation *operation,
    _In_opt_ void *callbackContext, 
             MI_Uint32 channel,
    _In_z_   const MI_Char *message);

/* Asynchronous callback for displaying none terminating operation error information sent from the provider. */
void MI_CALL WriteErrorCallback(
    _In_     MI_Operation *operation,
    _In_opt_ void *callbackContext, 
    _In_ MI_Instance*instance,
    _In_opt_ MI_Result (MI_CALL * writeErrorResult)(_In_ MI_Operation *operation, 
                                                    MI_OperationCallback_ResponseType response));
/* Asynchronous callback for prompting the user to continue or not that is sent from the provider. */
void MI_CALL PromptUserCallback(
    _In_     MI_Operation *operation,
    _In_opt_ void *callbackContext, 
    _In_z_   const MI_Char *message,
             MI_PromptType promptType,
    _In_opt_ MI_Result (MI_CALL * promptUserResult)(_In_ MI_Operation *operation, 
                                                      MI_OperationCallback_ResponseType response));

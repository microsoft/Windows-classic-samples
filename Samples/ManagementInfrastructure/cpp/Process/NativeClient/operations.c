//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#include <windows.h>
#include <mi.h>
#include "utilities.h"
#include "operations.h"

/* Do_Operation() gets user to select the operation and calls into a function to carry it out. */
void Do_Operation(MI_Application *miApplication, _In_opt_z_ const wchar_t *computerName, _In_opt_z_ const wchar_t *protocol, _In_z_ const wchar_t *namespaceName, const wchar_t *className)
{
    MI_Session miSession = MI_SESSION_NULL;
    MI_Result miResult = MI_RESULT_OK;
    wchar_t selection;

    /* Create a session to against the specified computer name.  NULL means local and uses WMI-DCOM by default,
     * or when name is specified will use WS-Man protcol by default.
     * The session must be closed via a call to MI_Session_Close().
     */
    miResult = MI_Application_NewSession(miApplication, protocol, computerName, NULL, NULL, NULL, &miSession);
    if (miResult != MI_RESULT_OK)
    {
        /* This API is likely to fail with invalid parameter or out of memory errors. */
        wprintf(L"MI_Application_NewSession failed, error = %s\n", MI_Result_To_String(miResult));
        goto noSessionError;
    }

    do
    {
        /* Helper API to retrieve the operation selection */
        selection = GetUserSelection(
                    L"Which type of operation do you want to run?\n"
                    L"\t[1] Instance Get\n"
                    L"\t[2] Instance Create\n"
                    L"\t[3] Instance Delete\n"
                    L"\t[4] Instance Modify\n"
                    L"\t[5] Instance Enumerate\n"
                    L"\t[6] Instance Query\n"
                    L"\t[7] Instance Association\n"
                    L"\t[8] Instance Reference\n"
                    L"\t[9] Instance Method\n"
                    L"\t[A] Indication Subscriptions\n"
                    L"\t[0] back to options\n",
                    L"0123456789a");

        switch(selection)
        {
        case L'1':
            Do_Get(&miSession, namespaceName, className);
            break;
        case L'2':
            Do_Create(&miSession, namespaceName, className);
            break;
        case L'3':
            Do_Delete(&miSession, namespaceName, className);
            break;
        case L'4':
            Do_Modify(&miSession, namespaceName, className);
            break;
        case L'5':
            Do_Enumerate(&miSession, namespaceName, className);
            break;
        case L'6':
            Do_Query(&miSession, namespaceName);
            break;
        case L'7':
            Do_Association(&miSession, namespaceName, className);
            break;
        case L'8':
            Do_Reference(&miSession, namespaceName, className);
            break;
        case L'9':
            Do_Method(&miSession, namespaceName, className);
            break;
        case L'a':
            Do_Subscribe(&miSession, namespaceName);
            break;
        }
    } while (selection != L'0' && miResult == MI_RESULT_OK);

    miResult = MI_Session_Close(&miSession, NULL, NULL);
    if (miResult != MI_RESULT_OK)
    {
        /* This API is likely to fail with invalid parameter, out of memory errors or access denied.
         * When an out of memory error happens, the session will shut down as best it can.
         * Invalid parameter means a programming error happened.
         * Access denied means the security context while calling into the Close() is different from
         * when the session was created.  This will be a programming error and could happen if closing 
         * from a different thread and forgetting to impersonate.
         */
        wprintf(L"MI_Session_Close failed, error = %s\n", MI_Result_To_String(miResult));
        goto noSessionError;
    }

noSessionError:
    return;
}

/* InstanceResultCallback() is a callback function for all asynchronous instance operation results. */
void MI_CALL InstanceResultCallback(
    _In_opt_     MI_Operation *miOperation,
    _In_     void *callbackContext, 
    _In_opt_ const MI_Instance *miInstance,
             MI_Boolean moreResults,
    _In_     MI_Result miResult,
    _In_opt_z_ const MI_Char *errorString,
    _In_opt_ const MI_Instance *errorDetails,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    struct InstanceResultCallback_Context *actualContext = (struct InstanceResultCallback_Context*) callbackContext;

    if (miInstance)
    {
        /* If we have an instance it implies the operation was successful.  We will dump the instance
         * to the screen. 
         */
        wprintf(L"------------------------------------------\n");
        Dump_MI_Instance(miInstance, actualContext->keysOnly, 0);
        actualContext->numberInstances ++;
    }

    if (moreResults == MI_FALSE)
    {
        MI_Result _miResult; /* temporary internal result */

        /* This state means the callback will not be called again as there are no more results.
         * We can cleanup the operation objects and, if necessary, notify the caller that 
         * the operation is finished.
         */
        wprintf(L"------------------------------------------\n");
        if (miResult != MI_RESULT_OK)
        {
            /* Operation failed, we we will dump all the error information we have. */
            wprintf(L"Operation failed, MI_Result=%s, errorString=%s, errorDetails=\n", MI_Result_To_String(miResult), errorString);
            Dump_MI_Instance(errorDetails, MI_FALSE, 0);
        }
        else
        {
            /* Succeeded, so we will dump the final instance count. */
            wprintf(L"Operation succeeded, number of instances=%u\n", actualContext->numberInstances);
        }
        wprintf(L"------------------------------------------\n");
        
        if (resultAcknowledgement)
        {
            /* By default an Auto-acknowledgement is requested with the operation, which means that any 
             * result data is only valid in this asyncronous callback, and will be deleted then the 
             * callback returns.
             * Manual acknowlegement allows the result data to be queued to a different thread for processing if needed, and
             * when finished with can be acknowledged in this way. No new results will be delivered until the data is 
             * acknowledged.  
             * Do not take too long to process the data though as the server may give up on the operation and cancel it.
             */
            MI_Result _miResult;
            _miResult = resultAcknowledgement(miOperation);
            if (_miResult != MI_RESULT_OK)
            {
                /* Failure to acknowledge happens as a result of invalid operation handle, out of memory, 
                 * or access denied.  Access denied can happen if the identity used to create the session and operation
                 * is different to that being used to close the operation.  For the callback, this is handled by
                 * the MI infrastructure.
                 */
                wprintf(L"Instance acknowledgement callback failed, error %s\n", MI_Result_To_String(_miResult));
            }
        }

        /* This operation is asynchronous, to the operation will close asynchronously. */
        if (miOperation != NULL)
        {
            _miResult = MI_Operation_Close(miOperation);
            if (_miResult != MI_RESULT_OK)
            {
                /* Failure to close operation happens as a result of invalid operation handle, out of memory, 
                 * or access denied.  Access denied can happen if the identity used to create the session and operation
                 * is different to that being used to close the operation.  For the callback, this is handled by
                 * the MI infrastructure.
                 */
                wprintf(L"MI_Operation_Close failed, error = %s\n", MI_Result_To_String(_miResult));
            }
        }
        actualContext->finalResult = miResult;

        /* Notify the main waiting thread we are done.
         * The way this sample uses asynchronous instance operations it would actually be better to 
         * to use the synchronous APIs as they may be a little more efficient.  If this was a service
         * it is possible that everything could be very asynchronous in nature with no real need for a 
         * waiting thread.
         */
        SetEvent(actualContext->asyncNotificationHandle);
    }
    else
    {
        if (resultAcknowledgement)
        {
            /* By default an Auto-acknowledgement is requested with the operation, which means that any 
             * result data is only valid in this asyncronous callback, and will be deleted then the 
             * callback returns.
             * Manual acknowlegement allows the result data to be queued to a different thread for processing if needed, and
             * when finished with can be acknowledged in this way. No new results will be delivered until the data is 
             * acknowledged.  
             * Do not take too long to process the data though as the server may give up on the operation and cancel it.
             */
            MI_Result _miResult;
            _miResult = resultAcknowledgement(miOperation);
            if (_miResult != MI_RESULT_OK)
            {
                /* Failure to acknowledge happens as a result of invalid operation handle, out of memory, 
                 * or access denied.  Access denied can happen if the identity used to create the session and operation
                 * is different to that being used to close the operation.  For the callback, this is handled by
                 * the MI infrastructure.
                 */
                wprintf(L"Instance acknowledgement callback failed, error %s\n", MI_Result_To_String(_miResult));
            }
        }
    }
}

/* IndicationResultCallback() is a callback function for all asynchronous subscribe operation results. */
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
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    struct InstanceResultCallback_Context *actualContext = (struct InstanceResultCallback_Context*) callbackContext;

    if (miInstance)
    {
        /* If we have an instance it implies the operation was successful.  We will dump the instance
         * to the screen. 
         */
        wprintf(L"------------------------------------------\n");
        Dump_MI_Instance(miInstance, actualContext->keysOnly, 0);
        actualContext->numberInstances ++;
    }

    if (moreResults == MI_FALSE)
    {
        MI_Result _miResult; /* temporary internal result */

        /* This state means the callback will not be called again as there are no more results.
         * We can cleanup the operation objects and, if necessary, notify the caller that 
         * the operation is finished.
         */
        wprintf(L"------------------------------------------\n");
        if (actualContext->numberInstances == 0 && miResult != MI_RESULT_OK)
        {
            /* Operation failed, we we will dump all the error information we have. */
            wprintf(L"Operation failed, MI_Result=%s, errorString=%s, errorDetails=\n", MI_Result_To_String(miResult), errorString);
            Dump_MI_Instance(errorDetails, MI_FALSE, 0);
        }
        else
        {
            /* Succeeded, so we will dump the final instance count. */
            wprintf(L"Operation succeeded, number of instances=%u\n", actualContext->numberInstances);
        }
        wprintf(L"------------------------------------------\n");
        
        if (resultAcknowledgement)
        {
            /* By default an Auto-acknowledgement is requested with the operation, which means that any 
             * result data is only valid in this asyncronous callback, and will be deleted then the 
             * callback returns.
             * Manual acknowlegement allows the result data to be queued to a different thread for processing if needed, and
             * when finished with can be acknowledged in this way. No new results will be delivered until the data is 
             * acknowledged.  
             * Do not take too long to process the data though as the server may give up on the operation and cancel it.
             */
            MI_Result _miResult;
            _miResult = resultAcknowledgement(miOperation);
            if (_miResult != MI_RESULT_OK)
            {
                /* Failure to acknowledge happens as a result of invalid operation handle, out of memory, 
                 * or access denied.  Access denied can happen if the identity used to create the session and operation
                 * is different to that being used to close the operation.  For the callback, this is handled by
                 * the MI infrastructure.
                 */
                wprintf(L"indication acknowledgement callback failed, error %s\n", MI_Result_To_String(_miResult));
            }
        }

        /* This operation is asynchronous, to the operation will close asynchronously. */
        if (miOperation != NULL)
        {
            _miResult = MI_Operation_Close(miOperation);
            if (_miResult != MI_RESULT_OK)
            {
                /* Failure to close operation happens as a result of invalid operation handle, out of memory, 
                 * or access denied.  Access denied can happen if the identity used to create the session and operation
                 * is different to that being used to close the operation.  For the callback, this is handled by
                 * the MI infrastructure.
                 */
                wprintf(L"MI_Operation_Close failed, error = %s\n", MI_Result_To_String(_miResult));
            }
        }
        actualContext->finalResult = miResult;
    }
    else
    {
        if (resultAcknowledgement)
        {
            /* By default an Auto-acknowledgement is requested with the operation, which means that any 
             * result data is only valid in this asyncronous callback, and will be deleted then the 
             * callback returns.
             * Manual acknowlegement allows the result data to be queued to a different thread for processing if needed, and
             * when finished with can be acknowledged in this way. No new results will be delivered until the data is 
             * acknowledged.  
             * Do not take too long to process the data though as the server may give up on the operation and cancel it.
             */
            MI_Result _miResult;
            _miResult = resultAcknowledgement(miOperation);
            if (_miResult != MI_RESULT_OK)
            {
                /* Failure to acknowledge happens as a result of invalid operation handle, out of memory, 
                 * or access denied.  Access denied can happen if the identity used to create the session and operation
                 * is different to that being used to close the operation.  For the callback, this is handled by
                 * the MI infrastructure.
                 */
                wprintf(L"indication acknowledgement callback failed, error %s\n", MI_Result_To_String(_miResult));
            }
        }
        /* Just a reminder message to the user of how to close the operation. */
        wprintf(L"Press [0] to end subscription\n");
    }

    /* Bookmarks and machine IDs are not covered in this sample. */
    MI_UNREFERENCED_PARAMETER(bookmark);
    MI_UNREFERENCED_PARAMETER(machineID);
}

/* StreamedResultCallback() is a callback function for method operations that use streamed out parameters. 
 * A method out parameter needs to be marked with a [stream] qualifier for this callback to be called.
 * The callback may be called multiple times for a streamed parameter.
 */
void MI_CALL StreamedResultCallback(
    _In_ MI_Operation *operation,
    _In_ void *callbackContext,
    _In_z_ const MI_Char *parameterName,
    _In_ MI_Type resultType,
    _In_ const MI_Value *result,
    _In_opt_ MI_Result (MI_CALL * resultAcknowledgement)(_In_ MI_Operation *operation))
{
    MI_UNREFERENCED_PARAMETER(callbackContext);

    /* Print the parameter results. */
    wprintf(L"Streamed result callback\n");
    Print_Element(parameterName, result, resultType, 0, 0);

    if (resultAcknowledgement)
    {
        /* By default an Auto-acknowledgement is requested with the operation, which means that any 
         * result data is only valid in this asyncronous callback, and will be deleted then the 
         * callback returns.
         * Manual acknowlegement allows the result data to be queued to a different thread for processing if needed, and
         * when finished with can be acknowledged in this way. No new results will be delivered until the data is 
         * acknowledged.  
         * Do not take too long to process the data though as the server may give up on the operation and cancel it.
         */

        MI_Result miResult;
        miResult = resultAcknowledgement(operation);
        if (miResult != MI_RESULT_OK)
        {
            /* Failure to acknowledge happens as a result of invalid operation handle, out of memory, 
             * or access denied.  Access denied can happen if the identity used to create the session and operation
             * is different to that being used to close the operation.  For the callback, this is handled by
             * the MI infrastructure.
             */
            wprintf(L"Streamed acknowledge callback failed, error %s\n", MI_Result_To_String(miResult));
        }
    }
}

/* WriteProgressCallback() is a callback function used to report operation progress from the provider. */
void MI_CALL WriteProgressCallback(
    _In_     MI_Operation *operation,
    _In_opt_ void *callbackContext, 
    _In_z_   const MI_Char *activity,
    _In_z_   const MI_Char *currentOperation,
    _In_z_   const MI_Char *statusDescription,
             MI_Uint32 percentageComplete,
             MI_Uint32 secondsRemaining)
{
    MI_UNREFERENCED_PARAMETER(operation);
    MI_UNREFERENCED_PARAMETER(callbackContext);

    wprintf(L"Progress indicator callback. \nActivity: %s\nCurrent operation: %s\nStatus description: %s\nPercentage complete: %u\nSeconds remaining: %u\n",
            activity, currentOperation, statusDescription, percentageComplete, secondsRemaining);
}

/* WriteMessageCallback() is a callback function used to report messages from the provider during an operation. */
void MI_CALL WriteMessageCallback(
    _In_     MI_Operation *operation,
    _In_opt_ void *callbackContext, 
             MI_Uint32 channel,
    _In_z_   const MI_Char *message)
{
    MI_UNREFERENCED_PARAMETER(operation);
    MI_UNREFERENCED_PARAMETER(callbackContext);

    if (channel == MI_WRITEMESSAGE_CHANNEL_WARNING)
    {
        wprintf(L"Write message callback (warning): %s\n", message);
    }
    else if (channel == MI_WRITEMESSAGE_CHANNEL_VERBOSE)
    {
        wprintf(L"Write message callback (verbose): %s\n", message);
    }
    else if (channel == MI_WRITEMESSAGE_CHANNEL_DEBUG)
    {
        wprintf(L"Write message callback (debug): %s\n", message);
    }
    else
    {
        wprintf(L"Write message callback (%u): %s\n", channel, message);
    }
}

/* WriteErrorCallback() is a callback function used to report non-terminating errors from the provider during an operation, with
 * an option (configurable) to report back to the provider to continue or terminate the operation. */
void MI_CALL WriteErrorCallback(
    _In_     MI_Operation *operation,
    _In_opt_ void *callbackContext, 
    _In_ MI_Instance*instance,
    _In_opt_ MI_Result (MI_CALL * writeErrorResult)(_In_ MI_Operation *operation, 
                                                    MI_OperationCallback_ResponseType response))
{
    MI_UNREFERENCED_PARAMETER(callbackContext);

    wprintf(L"Write error callback\n");
    Dump_MI_Instance(instance, MI_FALSE, 0);

    if (writeErrorResult)
    {
        MI_Result miResult;
        MI_Char selection;
        
        selection = GetUserSelection(L"[1] No" 
                                     L"[2] Yes"
                                     L"[3] No to all"
                                     L"[4] Yes to all",
                                     L"1234");

        miResult = writeErrorResult(operation, (MI_OperationCallback_ResponseType) (L'1' - selection));
        if (miResult != MI_RESULT_OK)
        {
            /* Failure to acknowledge happens as a result of invalid operation handle, out of memory, 
             * or access denied.  Access denied can happen if the identity used to create the session and operation
             * is different to that being used to close the operation.  For the callback, this is handled by
             * the MI infrastructure.
             */
            wprintf(L"Write error response callback failed, error %s\n", MI_Result_To_String(miResult));
        }
    }
}

/* PromptUserCallback() is a callback function used to prompt the user if they want to continue with an operation. */
void MI_CALL PromptUserCallback(
    _In_     MI_Operation *operation,
    _In_opt_ void *callbackContext, 
    _In_z_   const MI_Char *message,
             MI_PromptType promptType,
    _In_opt_ MI_Result (MI_CALL * promptUserResult)(_In_ MI_Operation *operation, 
                                                      MI_OperationCallback_ResponseType response))
{
    MI_UNREFERENCED_PARAMETER(callbackContext);

    wprintf(L"Prompt user callback (%s): %s\n", promptType==MI_PROMPTTYPE_NORMAL?L"Normal":L"Critical", message);

    if (promptUserResult)
    {
        MI_Result miResult;
        MI_Char selection;
        
        selection = GetUserSelection(L"[1] No" 
                                     L"[2] Yes"
                                     L"[3] No to all"
                                     L"[4] Yes to all",
                                     L"1234");

        miResult = promptUserResult(operation, (MI_OperationCallback_ResponseType) (L'1' - selection));
        if (miResult != MI_RESULT_OK)
        {
            /* Failure to acknowledge happens as a result of invalid operation handle, out of memory, 
             * or access denied.  Access denied can happen if the identity used to create the session and operation
             * is different to that being used to close the operation.  For the callback, this is handled by
             * the MI infrastructure.
             */
            wprintf(L"prompt user response callback failed, error %s\n", MI_Result_To_String(miResult));
        }
    }
}

//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <windows.h>
#include <strsafe.h>
#include <mi.h>
#include "utilities.h"
#include "operations.h"

void Do_Method_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className, const wchar_t *methodName, MI_Instance *keyedInstance, MI_Instance *inboundParameters);
void Do_Method_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className, const wchar_t *methodName, MI_Instance *keyedInstance, MI_Instance *inboundParameters);

/* Do_Modify() gets the class declaration for the class and gives the user a list of methods to choose from.  If the method non-static 
 * the user is asked to input the key properties to identify the instance to execute the method against.  The user can then input all 
 * the In parameters for the method. Finally the user selects from synchronous or asynchronous, before calling into the function to 
 * carry out the operation.
 */
void Do_Method(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className)
{
    wchar_t methodSelector;
    MI_Operation classOperation = MI_OPERATION_NULL;
    MI_Result miResult;
    MI_Result _miResult;
    MI_Class *miClass = NULL;
    const MI_Char *errorMessage;
    MI_Instance *errorDetails;
    MI_Uint32 methodCount;
    MI_Uint32 methodIndex;
    MI_Char methodSelectionList[75];
    MI_ParameterSet methodParameterSet;
    MI_Uint32 parameterCount;
    MI_Uint32 parameterIndex;
    const MI_Char *methodName;
    MI_Instance *inboundMethodParameters = NULL;
    MI_Application miApplication = MI_APPLICATION_NULL;
    MI_QualifierSet qualifierSet;
    MI_Type qualifierType;
    MI_Uint32 qualifierFlags;
    MI_Value qualifierValue;
    MI_Uint32 qualifierIndex;
    MI_Instance *methodInstance = NULL;

    /* Retrieves the class declaration synchronously so we can find all the methods on the class, and if the methods are static or not. */
    MI_Session_GetClass(miSession, 0, NULL, namespaceName, className, NULL, &classOperation);
    
    /* Retrieve the single instance result */
    _miResult = MI_Operation_GetClass(&classOperation, &miClass, NULL, &miResult, &errorMessage, &errorDetails);
    if (_miResult != MI_RESULT_OK)
    {
        /* This API is likely to fail with invalid parameter, out of memory errors or access denied.
         * When an out of memory error happens, the operation will shut down as best it can.
         * Invalid parameter means a programming error happened.
         * Access denied means the security context while calling into the Close() is different from
         * when the operation was created.  This will be a programming error and could happen if closing 
         * from a different thread and forgetting to impersonate.
         */
        wprintf(L"MI_Operation_GetClass failed, error %s", MI_Result_To_String(_miResult));
        goto DoCloseClass;
    }

    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"Failed to retrieve class declaration, error %s, errorMessage = %s, errorInstance=\n", MI_Result_To_String(miResult), errorMessage);
        Dump_MI_Instance(errorDetails, MI_FALSE, 0);
        goto DoCloseClass;
    }
    /* The class we have retrieved will be valid until we close the classOperation operation handle */

    /* We will now enumerate over the methods in the class to give the user an option of which to execute */
    miResult = MI_Class_GetMethodCount(miClass, &methodCount);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"MI_Class_GetMethodCount, error %s\n", MI_Result_To_String(miResult));
        goto DoCloseClass;
    }

    wprintf(L"Select the method you want to execute:\n");
    
    methodSelectionList[0] = L'0';

    for (methodIndex = 0; (methodIndex < methodCount) && ((methodIndex+2) < sizeof(methodSelectionList)/sizeof(methodSelectionList[0])); methodIndex++)
    {
        miResult = MI_Class_GetMethodAt(miClass, methodIndex, &methodName, NULL, NULL);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"MI_Class_GetMethodAt, error %s\n", MI_Result_To_String(miResult));
            goto DoCloseClass;
        }
        wprintf(L"\t[%c] %s\n", methodIndex + L'1', methodName);
        methodSelectionList[methodIndex+1] = L'1' + (MI_Char) methodIndex;
    }
    wprintf(L"\t[0] back to operation choice\n");
    methodSelectionList[methodIndex+1] = L'\0';
    methodSelector = GetUserSelection(
                L"",
                methodSelectionList);

    if (methodSelector == L'0')
    {
        goto DoCloseClass;
    }

    /* Method is selected, now need to determine if the method is static or not by looking at the qualifiers on the method.
     * A qualifier of static==MI_TRUE would mean static, otherwise we will get the method keyed instance.
     */
    miResult = MI_Class_GetMethodAt(miClass, methodSelector-L'1', &methodName, &qualifierSet, &methodParameterSet);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"MI_Class_GetMethodAt, error %s\n", MI_Result_To_String(miResult));
        goto DoCloseClass;
    }

    /* Get the qualifier by name */
    miResult = MI_QualifierSet_GetQualifier(&qualifierSet, L"static", &qualifierType, &qualifierFlags, &qualifierValue, &qualifierIndex);
    if (miResult == MI_RESULT_NOT_FOUND)
    {
        /* Non-static method, so create a keyed instance for the method */
        miResult = CreateInboundInstance(miSession, namespaceName, className, MI_TRUE, &methodInstance);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"Failed to create inbound method instance, error %s\n", MI_Result_To_String(miResult));
            goto DoCloseClass;
        }
    }
    else if (miResult != MI_RESULT_OK)
    {
        wprintf(L"MI_QualifierSet_GetQualifier, error %s\n", MI_Result_To_String(miResult));
        goto DoCloseClass;
    }
    else
    {
        if (qualifierType != MI_BOOLEAN)
        {
            wprintf(L"static qualifier type should be boolean\n");
            goto DoCloseClass;
        }
        if (qualifierValue.boolean == MI_FALSE)
        {
            /* Non-static method, so create a keyed instance for the method */
            miResult = CreateInboundInstance(miSession, namespaceName, className, MI_TRUE, &methodInstance);
            if (miResult != MI_RESULT_OK)
            {
                wprintf(L"Failed to create inbound method instance, error %s\n", MI_Result_To_String(miResult));
                goto DoCloseClass;
            }
        }
    }
    if (methodInstance == NULL)
    {
        wprintf(L"Method is static so no key instance is required\n");
    }

    /* Next we are going to create a parameter instance and add one for each of the In parameters to the method */
    miResult = MI_ParameterSet_GetParameterCount(&methodParameterSet, &parameterCount);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"MI_ParameterSet_GetParameterCount, error %s\n", MI_Result_To_String(miResult));
        goto DoCloseClass;
    }

    /* Need the application to create the instance */
    miResult = MI_Session_GetApplication(miSession, &miApplication);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"MI_Session_GetApplication failed, error %s\n", MI_Result_To_String(miResult));
        goto DoCloseClass;
    }

    /* Enumerate through all the parameters to find all the Inbound ones, then input a value for them. */
    for (parameterIndex = 0; parameterIndex < parameterCount; parameterIndex++)
    {
        const MI_Char *parameterName;
        MI_Type parameterType;
        MI_Char *referenceClass;
        MI_Char tmpBuffer[MAX_PATH];

        /* Doing a double get because we need to re-use the tmpBuffer. */
        miResult = MI_ParameterSet_GetParameterAt(&methodParameterSet, parameterIndex, &parameterName, &parameterType, &referenceClass, &qualifierSet);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"MI_ParameterSet_GetParameterAt, error %s\n", MI_Result_To_String(miResult));
            goto DoCloseClass;
        }

        miResult = MI_QualifierSet_GetQualifier(&qualifierSet, L"in", &qualifierType, &qualifierFlags, &qualifierValue, &qualifierIndex);
        if (miResult == MI_RESULT_NOT_FOUND)
        {
            /* Not an in parameter */
            continue;
        }
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"MI_QualifierSet_GetQualifier, error %s\n", MI_Result_To_String(miResult));
            goto DoCloseClass;
        }
        if (qualifierType != MI_BOOLEAN)
        {
            wprintf(L"IN qualifier type should be boolean\n");
            goto DoCloseClass;
        }
        if (qualifierValue.boolean == MI_FALSE)
        {
            /* Not an inbound parameter, so ignore */
            continue;
        }
        if (inboundMethodParameters == NULL)
        {
            /* we need to create one as we have some in parameters */

            /* This is a weakly typed 'dynamic' instance.  There are no typed properties, but we can determine 
             * this from the method and add them in a typed way as we go.  This instance needs to be deleted
             * with a call to MI_Instance_Delete().
             */
            miResult = MI_Application_NewInstance(&miApplication, L"__parameters", NULL, &inboundMethodParameters);
            if (miResult != MI_RESULT_OK)
            {
                wprintf(L"MI_Application_NewInstance failed, error %s\n", MI_Result_To_String(miResult));
                goto DoCloseClass;
            }
            wprintf(L"Enter method parameters:\n");
        }
        /* This is an inbound parameter so call helper to input the value and add the property to the dynamic instance */
        miResult = SetInstanceProperty(inboundMethodParameters, parameterName, parameterType, 0, NULL, tmpBuffer, sizeof(tmpBuffer)/sizeof(tmpBuffer[0]), MI_TRUE);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"Failed to set method parameter, error %s\n", MI_Result_To_String(miResult));
            goto DoCloseClass;
        }
    }

    /* We have all the information to execute the method so choose from synchronous and asynchronous. */
    {
		wchar_t synchronous;

		synchronous = GetUserSelection(
					L"How do you want the Invoke operation to be carried out?\n"
					L"\t[1] Synchronous\n"
					L"\t[2] Asynchronous\n"
					L"\t[0] back to operation choice\n",
					L"012");
		if (synchronous == L'0')
		{
			return;
		}

		switch(synchronous)
		{
		case L'1':
			Do_Method_Synchronous(miSession, namespaceName, className, methodName, methodInstance, inboundMethodParameters);
			break;
		case L'2':
			Do_Method_Asynchronous(miSession, namespaceName, className, methodName, methodInstance, inboundMethodParameters);
			break;
		}
    }
DoCloseClass:
    if (inboundMethodParameters)
    {
        miResult = MI_Instance_Delete(inboundMethodParameters);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"MI_Instance_Delete for inbound method parameter instance failed, error %s\n", MI_Result_To_String(miResult));
            goto DoCloseClass;
        }
    }

    if (methodInstance != NULL)
    {
        miResult = MI_Instance_Delete(methodInstance);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"MI_Instance_Delete for inbound method key instance failed, error %s\n", MI_Result_To_String(miResult));
            goto DoCloseClass;
        }
    }
    /* All operations must be closed.  If an operation is not closed the owning session will hang until the operations
     * are closed fully.  MI_Operation_Close will cancel an operation if it is still running, however results must be
     * consumed before the close can complete fully.  
     * For synchronous operations the MI_Operation_Close() method is synchronous until the final result has been consumed 
     * (moreResults == MI_FALSE).
     */
    _miResult = MI_Operation_Close(&classOperation);
    if (_miResult != MI_RESULT_OK)
    {
        /* This API is likely to fail with invalid parameter, out of memory errors or access denied.
         * When an out of memory error happens, the operation will shut down as best it can.
         * Invalid parameter means a programming error happened.
         * Access denied means the security context while calling into the Close() is different from
         * when the operation was created.  This will be a programming error and could happen if closing 
         * from a different thread and forgetting to impersonate.
         */
        wprintf(L"MI_Operation_Close failed, error %s\n", MI_Result_To_String(_miResult));
    }

}

/* Do_Method_Synchronous() carries out an method operation synchronously, retrieving the result
 * on the same thread.  The result can be retrieved on any thread, but that would be unusual for a 
 * synchronous operation. 
 */
void Do_Method_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className, const wchar_t *methodName, MI_Instance *keyedInstance, MI_Instance *inboundParameters)
{
    MI_Operation miOperation = MI_OPERATION_NULL;
    MI_Instance *miInstance;
	MI_Result miResult;
	MI_Result _miResult;
	const wchar_t *errorMessage;
	MI_Instance *completionDetails;
    MI_Boolean moreResults;

    /* Note that the identity of the thread needs to be the same as the one the session was created on. */

    /* Note, although this sample does not include the PowerShell callbacks for extended semantics, they are allowed
     * on synchronous operations.  Allowable callbacks are:
     *      MI_OperationCallbacks.writeError
     *      MI_OperationCallbacks.writeMessage
     *      MI_OperationCallbacks.writeProgress
     */

    /* Note, methods can have outbound parameters that are marked for streaming ([stream] qualifier).  The synchronous sample is retrieving 
     * all the objects synchronously and not supporting streaming.  This still works, but it means all the streamed parameter parts are 
     * cached on the server and one big instance result is sent at the end.  This can cause the memory to spike on the server and client
     * when this is done.  A synchronous client can add a streamed parameter callback in the same way that the asyncronous sample
     * does by setting the MI_OperationCallbacks.streamedParameterResult parameter and passing to the Invoke operation.
     */

    /* Initiate the CreateInstance operation.  Synchronous results are always retrieved through a call MI_Operation_GetInstance(). 
     * All operations must be closed with a call to MI_Operation_Close(), but all results must be processed before that.
     * The operation can be cancelled via MI_Operation_Cancel(), although even then all results must be consumed before the operation
     * is closed. 
     */
    MI_Session_Invoke(miSession, 0, NULL, namespaceName, className, methodName, keyedInstance, inboundParameters, NULL, &miOperation);
    
    /* We always need to look through results until moreResults == MI_FALSE.  For synchronous operations without 
     * PowerShell callbacks it is not very likely to get more than one result from MI_Operation_GetInstance,
     * but it is always best to be sure, especially if you choose to add the PowerShell callbacks at a later data
     * and forget to update the retrieval to a loop.
     */
    do
    {
        /* Retrieve the single instance result.  remember, we need to call this API until moreResults == MI_FALSE */
        _miResult = MI_Operation_GetInstance(&miOperation, &miInstance, &moreResults, &miResult, &errorMessage, &completionDetails);
        if (_miResult != MI_RESULT_OK)
        {
            /* If this function returns a failure it means that an invalid parameter was passed in, or the identity of the thread
             * is different from the identity the operation was created with.  Both imply programming error.
             */
            wprintf(L"MI_Operation_GetInstance failed, errorString=%s\n", MI_Result_To_String(_miResult));
        }
        else
        {
            /* A result (success or failure) has been received. */
            wprintf(L"------------------------------------------\n");
            if (miResult != MI_RESULT_OK)
            {
                wprintf(L"Operation failed, MI_Result=%s, errorString=%s, errorDetails=\n", MI_Result_To_String(miResult), errorMessage);
                Dump_MI_Instance(completionDetails, MI_FALSE, 0);
            }
            else if (miInstance)
            {
                Dump_MI_Instance(miInstance, MI_FALSE, 0);
            }
            else if (moreResults == MI_TRUE)
            {
                wprintf(L"More results are due and we have no instance, we will keep trying!\n");
            }
            wprintf(L"------------------------------------------\n");
        }
    } while (moreResults == MI_TRUE);

    /* All operations must be closed.  If an operation is not closed the owning session will hang until the operations
     * are closed fully.  MI_Operation_Close will cancel an operation if it is still running, however results must be
     * consumed before the close can complete fully.  
     * For synchronous operations the MI_Operation_Close() method is synchronous until the final result has been consumed 
     * (moreResults == MI_FALSE).
     */
    _miResult = MI_Operation_Close(&miOperation);
    if (_miResult != MI_RESULT_OK)
    {
        /* This API is likely to fail with invalid parameter, out of memory errors or access denied.
         * When an out of memory error happens, the operation will shut down as best it can.
         * Invalid parameter means a programming error happened.
         * Access denied means the security context while calling into the Close() is different from
         * when the operation was created.  This will be a programming error and could happen if closing 
         * from a different thread and forgetting to impersonate.
         */
        wprintf(L"MI_Operation_Close failed, error %s\n", MI_Result_To_String(_miResult));
    }
}

/* Do_Method_Asynchronous() carries out an instance method invocation operation asynchronously. The asynchronous callback
 * will with the final result. 
 */
void Do_Method_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className, const wchar_t *methodName, MI_Instance *keyedInstance, MI_Instance *inboundParameters)
{
    MI_Operation miOperation = MI_OPERATION_NULL;
    MI_OperationCallbacks miOperationCallbacks = MI_OPERATIONCALLBACKS_NULL;
    struct InstanceResultCallback_Context instanceCallback_Context = {0};

    /* Create a notification event that we wait on until the operation has completed.
     * Note: This sample is demonstrating the asynchronous calling pattern for setting up
     * the operation, but to call a single operation and wait on a notification like
     * this example should really use the synchronous operations instead.
     */
    instanceCallback_Context.asyncNotificationHandle = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (instanceCallback_Context.asyncNotificationHandle == NULL)
    {
        wprintf(L"Failed to create a Windows Event, windows error %u\n", GetLastError());
        goto NoHandleError; 
    }

    /* Add optional context information to callback structure so we can hold state
     * throughout the operation
     */
    miOperationCallbacks.callbackContext = &instanceCallback_Context;

    /* Set instance callback function.  This will keep being called until it is
     * called with moreResults==MI_FALSE.
     * It is this callback that puts instance operations into asyncronous mode.
     */
    miOperationCallbacks.instanceResult = InstanceResultCallback;

    /* Optional callback (allowed for synchronous operations also), that receive
     * non-terminating operation error reports from a provider.
     */
    miOperationCallbacks.writeError = WriteErrorCallback;

    /* Optional callback (allowed for synchronous operations also), that receive
     * non-terminating operation error reports from a provider.
     */
    miOperationCallbacks.writeMessage = WriteMessageCallback;

    /* Optional callback (allowed for synchronous operations also), that receive
     * non-terminating operation error reports from a provider.
     */
    miOperationCallbacks.writeProgress = WriteProgressCallback;

    /* Opttional callback (allowed for synchronous operations also), that receive
     * streamed out parameters.  Streamed out parameters allow larger amounts 
     * of data to be passed back to the client in small pieces.  Only array parameters
     * are allowed to be streamed.  The streamed data may be a single element of the array
     * and so the type is not an array, OR a part of the array may be returned.
     */
    miOperationCallbacks.streamedParameterResult = StreamedResultCallback;

    /* Start the operation */
    MI_Session_Invoke(miSession, 0, NULL, namespaceName, className, methodName, keyedInstance, inboundParameters, &miOperationCallbacks, &miOperation);

    /* InstanceResultCallback() will always be called back for asyncronous operations, so wait for it to finish */
    
    WaitForSingleObject(instanceCallback_Context.asyncNotificationHandle, INFINITE);

    CloseHandle(instanceCallback_Context.asyncNotificationHandle);

    /* Final miResult is here if needed: instanceCallback_Context.finalResult 
     * Any data from the callback cannot be accessed here because the lifetime of the data is 
     * only valid in the callback and until the operation is closed.
     * In this sample the operation handle is closed inside the instance callback.
     */

NoHandleError:

    return;
}
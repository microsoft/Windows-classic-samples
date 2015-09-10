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

void Do_Modify_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, MI_Instance *modifyInstance);
void Do_Modify_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, MI_Instance *modifyInstance);

/* Do_Modify() prompts the user to input the key properties to identify the object to get, retrieves the instance from the server, 
 * allows the user to change parameters, then selects from synchronous or asynchronous. The final instance is then passed to the 
 * actual function to carry out the operation.
 */
void Do_Modify(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className)
{
    wchar_t synchronous;
    MI_Instance *keyIndex;
    MI_Char inputBuffer[50];
    MI_Result miResult;
    MI_Result _miResult;
    MI_Operation miOperation = MI_OPERATION_NULL;
    MI_Instance *modifyInstance;
    const MI_Char *errorMessage;
    MI_Instance *errorDetails;

    /* Helper function to create an MI_Instance which contains the classes keys to pass into the Get operation.
     * Delete the instance once done with MI_Instance_Delete(). */
    miResult = CreateInboundInstance(miSession, namespaceName, className, MI_TRUE, &keyIndex);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"Failed to create a keyed instance for the operation, error %s\n", MI_Result_To_String(miResult));
        return;
    }

    /* Synchronous Get operation to retrieve the instance that needs modifying */
    MI_Session_GetInstance(miSession, 0, NULL, namespaceName, keyIndex, NULL, &miOperation);
    _miResult = MI_Operation_GetInstance(&miOperation, &modifyInstance, NULL, &miResult, &errorMessage, &errorDetails);
    if (_miResult != MI_RESULT_OK)
    {
        /* If this function returns a failure it means that an invalid parameter was passed in, or the identity of the thread
         * is different from the identity the operation was created with.  Both imply programming error.
         */
        wprintf(L"MI_Operation_GetInstance failed, error %s\n", MI_Result_To_String(_miResult));
        goto operationCleanup;
    }
    if (miResult != MI_RESULT_OK)
    {
        /* Get operation failed, possibly because they keyed object did not exist. */
        wprintf(L"Get operation failed to retrieve the instance to modify, error %s\n", MI_Result_To_String(miResult));
        goto operationCleanup;
    }

    /* Allow the user to select a property to change, and input new value.  Keep doing this until the property name
     * input is blank.
     */
    for (;;)
    {
        const MI_Char *elementName;
        MI_Value elementValue;
        MI_Type elementType;
        MI_Uint32 elementIndex;

        /* Dump the latest version of the instance we are modifying. */
        Dump_MI_Instance(modifyInstance, MI_FALSE, 0);

        /* Retrieve the property name whos value needs changing. */
        GetUserInputString(L"Input property name to change value, blank to continue", inputBuffer, sizeof(inputBuffer)/sizeof(inputBuffer[0]), L"");
        if (inputBuffer[0] == L'\0')
        {
            /* Empty property name means we are ready to save the instance */
            break;
        }

        /* Note: Doing GetElement followed by GetElementAt in order to retrieve element name pointer from instance so we can reuse the input buffer */
        miResult = MI_Instance_GetElement(modifyInstance, inputBuffer, NULL, NULL, NULL, &elementIndex);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"Failed to get property, error %s\n", MI_Result_To_String(miResult));
        }

        miResult = MI_Instance_GetElementAt(modifyInstance, elementIndex, &elementName, &elementValue, &elementType, NULL);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"Failed to get property, error %s\n", MI_Result_To_String(miResult));
        }

        /* Helper function to input a new value and set it on the instance */
        miResult = SetInstanceProperty(modifyInstance, elementName, elementType, 0, &elementValue, inputBuffer, sizeof(inputBuffer)/sizeof(inputBuffer[0]), MI_FALSE);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"Failed to set property value, error %s\n", MI_Result_To_String(miResult));
        }
    };

    /* Select synchronous or asynchronous operation */
    synchronous = GetUserSelection(
                L"How do you want the Modify operation to be carried out?\n"
                L"\t[1] Synchronous\n"
                L"\t[2] Asynchronous\n"
                L"\t[0] back to operation choice\n",
                L"012");
    if (synchronous == L'0')
    {
        goto operationCleanup;
    }

    switch(synchronous)
    {
    case L'1':
        Do_Modify_Synchronous(miSession, namespaceName, modifyInstance);
        break;
    case L'2':
        Do_Modify_Asynchronous(miSession, namespaceName, modifyInstance);
        break;
    }

operationCleanup:
    /* Close the Get operation handle here will also dispose of the modifyInstance we retrieved from the server. */
    miResult = MI_Operation_Close(&miOperation);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"MI_Operation_Close, error %s\n", MI_Result_To_String(miResult));
    }

    /* Delete instance from CreateInboundInstance() call. */
    if (keyIndex)
    {
        miResult = MI_Instance_Delete(keyIndex);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"MI_Instance_Delete, error %s\n", MI_Result_To_String(miResult));
        }
    }
}

/* Do_Modify_Synchronous() carries out an instance Modify operation synchronously, retrieving the result
 * on the same thread.  The result can be retrieved on any thread, but that would be unusual for a 
 * synchronous operation. 
 */
void Do_Modify_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, MI_Instance *modifyInstance)
{
    MI_Result miResult;
    MI_Result _miResult;
    MI_Operation miOperation = MI_OPERATION_NULL;
    MI_Instance *miInstance = NULL;
    MI_Boolean moreResults;
    MI_Char *errorMessage = NULL;
    MI_Instance *completionDetails = NULL;

    /* Note that the identity of the thread needs to be the same as the one the session was created on. */

    /* Note, although this sample does not include the PowerShell callbacks for extended semantics, they are allowed
     * on synchronous operations.  Allowable callbacks are:
     *      MI_OperationCallbacks.writeError
     *      MI_OperationCallbacks.writeMessage
     *      MI_OperationCallbacks.writeProgress
     */

    /* Initiate the ModifyInstance operation.  Synchronous results are always retrieved through a call MI_Operation_GetInstance(). 
     * All operations must be closed with a call to MI_Operation_Close(), but all results must be processed before that.
     * The operation can be cancelled via MI_Operation_Cancel(), although even then all results must be consumed before the operation
     * is closed. 
     */
    MI_Session_ModifyInstance(miSession, 0, NULL, namespaceName, modifyInstance, NULL, &miOperation);

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

/* Do_Modify_Asynchronous() carries out an instance Modify operation asynchronously. The asynchronous callback
 * will with the final result. 
 */
void Do_Modify_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, MI_Instance *modifyInstance)
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
        return; 
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


    /* Start the operation */
    MI_Session_ModifyInstance(miSession, 0, NULL, namespaceName, modifyInstance, &miOperationCallbacks, &miOperation);

    /* InstanceResultCallback() will always be called back for asyncronous operations, so wait for it to finish */
    
    WaitForSingleObject(instanceCallback_Context.asyncNotificationHandle, INFINITE);

    CloseHandle(instanceCallback_Context.asyncNotificationHandle);

    /* Final miResult is here if needed: instanceCallback_Context.finalResult 
     * Any data from the callback cannot be accessed here because the lifetime of the data is 
     * only valid in the callback and until the operation is closed.
     * In this sample the operation handle is closed inside the instance callback.
     */

}

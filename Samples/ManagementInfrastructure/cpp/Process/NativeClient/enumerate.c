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

void Do_Enumerate_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className, MI_Boolean keysOnly);
void Do_Enumerate_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className, MI_Boolean keysOnly);

/* Do_Enumerate() prompts the user for some options, then selects from synchronous or asynchronous. */
void Do_Enumerate(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className)
{
    wchar_t synchronous;
    wchar_t keysOnly;

    keysOnly = GetUserSelection(
                L"How do you want the Enumerate operation to return property keys only?\n"
                L"\t[1] All instance properties\n"
                L"\t[2] Instance key properties only\n"
                L"\t[0] back to operation choice\n",
                L"012");
    if (keysOnly == L'0')
    {
        return;
    }

    synchronous = GetUserSelection(
                L"How do you want the Enumerate operation to be carried out?\n"
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
        Do_Enumerate_Synchronous(miSession, namespaceName, className, keysOnly==L'2' ? MI_TRUE : MI_FALSE);
        break;
    case L'2':
        Do_Enumerate_Asynchronous(miSession, namespaceName, className, keysOnly==L'2' ? MI_TRUE : MI_FALSE);
        break;
    }
}

/* Do_Enumerate_Synchronous() carries out an instance Enumerate operation synchronously, retrieving all results
 * on the same thread.  The results can be retrieved on any thread, but that would be unusual for a 
 * synchronous operation. 
 */
void Do_Enumerate_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className, MI_Boolean keysOnly)
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Operation miOperation = MI_OPERATION_NULL;
    MI_Boolean moreResults;
    const MI_Char *errorString = NULL;
    const MI_Instance *errorDetails = NULL;
    MI_Uint32 instanceCount = 0;
    MI_Result _miResult;

    /* Note that the identity of the thread needs to be the same as the one the session was created on. */

    /* Note, although this sample does not include the PowerShell callbacks for extended semantics, they are allowed
     * on synchronous operations.  Allowable callbacks are:
     *      MI_OperationCallbacks.writeError
     *      MI_OperationCallbacks.writeMessage
     *      MI_OperationCallbacks.writeProgress
     */

    /* Initiate the EnumerateInstances operation.  Synchronous results are always retrieved through a call MI_Operation_GetInstance(). 
     * All operations must be closed with a call to MI_Operation_Close(), but all results must be processed before that.
     * The operation can be cancelled via MI_Operation_Cancel(), although even then all results must be consumed before the operation
     * is closed. 
     */
    MI_Session_EnumerateInstances(miSession, 0, NULL, namespaceName, className, keysOnly, NULL, &miOperation);

    /* Must loop through all results until moreResults == MI_FALSE */
    do
    {
        MI_Instance *miInstance;
        MI_Result _miResult;

        /* Retrieve a single instance result */
        _miResult = MI_Operation_GetInstance(&miOperation, &miInstance, &moreResults, &miResult, &errorString, &errorDetails);
        if (_miResult != MI_RESULT_OK)
        {
            /* If this function returns a failure it means that an invalid parameter was passed in, or the identity of the thread
             * is different from the identity the operation was created with.  Both imply programming error.
             */
            wprintf(L"MI_Operation_GetInstance failed due to a programming error, error = %s\n", MI_Result_To_String(_miResult));
            break;
        }
        if (miInstance)
        {
            /* Dump the instance result */
            wprintf(L"------------------------------------------\n");
            Dump_MI_Instance(miInstance, keysOnly, 0);
            instanceCount++;
        }

    } while (miResult == MI_RESULT_OK && moreResults == MI_TRUE);

    /* moreResults == MI_FALSE, dump the final outcome of the operation */
    wprintf(L"------------------------------------------\n");
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"Operation failed, MI_Result=%s, errorString=%s, errorDetails=\n", MI_Result_To_String(miResult), errorString);
        Dump_MI_Instance(errorDetails, MI_FALSE, 0);
    }
    else
    {
        wprintf(L"Operation succeeded, number of instances=%u\n", instanceCount);
    }
    wprintf(L"------------------------------------------\n");

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

/* Do_Enumerate_Asynchronous() carries out an instance enumeration operation asynchronously. The asynchronous callback
 * will keep being called until moreResults==MI_FALSE. 
 */
void Do_Enumerate_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className, MI_Boolean keysOnly)
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
    instanceCallback_Context.keysOnly = keysOnly;

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
    MI_Session_EnumerateInstances(miSession, 0, NULL, namespaceName, className, keysOnly, &miOperationCallbacks, &miOperation);

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

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

void Do_Association_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, MI_Instance *keyInstance, MI_Boolean keysOnly, _In_opt_z_ MI_Char *associationClass, _In_opt_z_ MI_Char *resultClass, _In_opt_z_ MI_Char *roleProperty, _In_opt_z_ MI_Char *resultRoleProperty);
void Do_Association_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, MI_Instance *keyInstance, MI_Boolean keysOnly, _In_opt_z_ MI_Char *associationClass, _In_opt_z_ MI_Char *resultClass, _In_opt_z_ MI_Char *roleProperty, _In_opt_z_ MI_Char *resultRoleProperty);

/* Do_Association() creates a key instance for the association and retrieves some extra optional filtering information
 * used for the assoociation, then selects from synchronous or asynchronous.  */
void Do_Association(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *className)
{
    wchar_t synchronous;
    wchar_t keysOnly;
    MI_Result miResult;
    MI_Instance *keyInstance;
    MI_Char _associationClass[50];
    MI_Char *associationClass = NULL;
    MI_Char _resultClass[50];
    MI_Char *resultClass = NULL;
    MI_Char _roleProperty[50];
    MI_Char *roleProperty = NULL;
    MI_Char _resultRoleProperty[50];
    MI_Char *resultRoleProperty = NULL;

    /* Helper function to create an MI_Instance which contains the classes keys to pass into the association operation.
     * Delete the instance once done with MI_Instance_Delete(). */
    miResult = CreateInboundInstance(miSession, namespaceName, className, MI_TRUE, &keyInstance);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"Failed to create a keyed instance for the operation, error %s\n", MI_Result_To_String(miResult));
        return;
    }

    /* Asks for the association class filter.  This allows the results to be filtered to just one association class. */
    wprintf(L"Enter associator filters (all optional)\n");
    GetUserInputString(L"association class", _associationClass, sizeof(_associationClass)/sizeof(_associationClass[0]), L"");
    if (_associationClass[0] != 0)
    {
        associationClass = _associationClass;
    }
    /* Ask for the result class filter.  This allows results to be filtered to just this final class type. */
    GetUserInputString(L"result class", _resultClass, sizeof(_resultClass)/sizeof(_resultClass[0]), L"");
    if (_resultClass[0] != 0)
    {
        resultClass = _resultClass;
    }
    /* Asks for association role property filter.  This allows for filtering based on the property name within the
     * association class itself that points to the initial key instance.
     */
    GetUserInputString(L"role property", _roleProperty, sizeof(_roleProperty)/sizeof(_roleProperty[0]), L"");
    if (_roleProperty[0] != 0)
    {
        roleProperty = _roleProperty;
    }
    /* Ask for the association result role property filter.  This allows for filtering based on the property name within 
     * the association class itself that points to the result class.
     */
    GetUserInputString(L"result role property", _resultRoleProperty, sizeof(_resultRoleProperty)/sizeof(_resultRoleProperty[0]), L"");
    if (_resultRoleProperty[0] != 0)
    {
        resultRoleProperty = _resultRoleProperty;
    }

    /* Asks user if full result instances are required, or just the key properties */
    keysOnly = GetUserSelection(
                L"How do you want the Associator operation to return property keys only?\n"
                L"\t[1] All instance properties\n"
                L"\t[2] Instance key properties only\n"
                L"\t[0] back to operation choice\n",
                L"012");

    /* ASk user if synchronous or asynchronous operation is required. */
    synchronous = GetUserSelection(
                L"How do you want the Associator operation to be carried out?\n"
                L"\t[1] Synchronous\n"
                L"\t[2] Asynchronous\n"
                L"\t[0] back to operation choice\n",
                L"012");
    switch(synchronous)
    {
    case L'1':
        Do_Association_Synchronous(miSession, namespaceName, keyInstance, keysOnly==L'2' ? MI_TRUE : MI_FALSE, associationClass, resultClass, roleProperty, resultRoleProperty);
        break;
    case L'2':
        Do_Association_Asynchronous(miSession, namespaceName, keyInstance, keysOnly==L'2' ? MI_TRUE : MI_FALSE, associationClass, resultClass, roleProperty, resultRoleProperty);
        break;
    }

    /* Delete instance from CreateInboundInstance() call. */
    miResult = MI_Instance_Delete(keyInstance);
    if (miResult != MI_RESULT_OK)
    {
        /* Invalid parameter is the only likely error which would imply a programming error. */
        wprintf(L"MI_Instance_Delete, error %s\n", MI_Result_To_String(miResult));
        return;
    }

}

/* Do_Association_Synchronous() carries out an instance association operation synchronously, retrieving all results
 * on the same thread.  The results can be retrieved on any thread, but that would be unusual for a 
 * synchronous operation. 
 */
void Do_Association_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, MI_Instance *keyInstance, MI_Boolean keysOnly, _In_opt_z_ MI_Char *associationClass, _In_opt_z_ MI_Char *resultClass, _In_opt_z_ MI_Char *roleProperty, _In_opt_z_ MI_Char *resultRoleProperty)
{
    MI_Result miResult = MI_RESULT_OK;
    MI_Result _miResult;
    MI_Operation miOperation = MI_OPERATION_NULL;
    MI_Boolean moreResults;
    const MI_Char *errorString = NULL;
    const MI_Instance *errorDetails = NULL;
    MI_Uint32 instanceCount = 0;

    /* Note that the identity of the thread needs to be the same as the one the session was created on. */

    /* Note, although this sample does not include the PowerShell callbacks for extended semantics, they are allowed
     * on synchronous operations.  Allowable callbacks are:
     *      MI_OperationCallbacks.writeError
     *      MI_OperationCallbacks.writeMessage
     *      MI_OperationCallbacks.writeProgress
     */

    /* Initiate the associator operation.  Synchronous results are always retrieved through a call MI_Operation_GetInstance(). 
     * All operations must be closed with a call to MI_Operation_Close(), but all results must be processed before that.
     * The operation can be cancelled via MI_Operation_Cancel(), although even then all results must be consumed before the operation
     * is closed. 
     */
    MI_Session_AssociatorInstances(miSession, 0, NULL, namespaceName, keyInstance, associationClass, resultClass, roleProperty, resultRoleProperty, keysOnly, NULL, &miOperation);

    /* Must loop through all results until moreResults == MI_FALSE */
    do
    {
        MI_Instance *miInstance;

        /* Retrieve a single instance result */
        _miResult = MI_Operation_GetInstance(&miOperation, &miInstance, &moreResults, &miResult, &errorString, &errorDetails);
        if (_miResult != MI_RESULT_OK)
        {
            /* If this function returns a failure it means that an invalid parameter was passed in, or the identity of the thread
             * is different from the identity the operation was created with.  Both imply programming error.
             */
            wprintf(L"MI_Operation_GetInstance failed, error = %s\n", MI_Result_To_String(_miResult));
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

/* Do_Association_Asynchronous() carries out an instance association operation asynchronously. The asynchronous callback
 * will keep being called until moreResults==MI_FALSE. 
 */
void Do_Association_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, MI_Instance *keyInstance, MI_Boolean keysOnly, _In_opt_z_ MI_Char *associationClass, _In_opt_z_ MI_Char *resultClass, _In_opt_z_ MI_Char *roleProperty, _In_opt_z_ MI_Char *resultRoleProperty)
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
    MI_Session_AssociatorInstances(miSession, 0, NULL, namespaceName, keyInstance, associationClass, resultClass, roleProperty, resultRoleProperty, keysOnly, &miOperationCallbacks, &miOperation);

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

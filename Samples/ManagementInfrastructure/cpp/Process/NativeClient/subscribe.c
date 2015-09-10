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

void Do_Subscribe_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *queryString, const wchar_t *queryDialect);
void Do_Subscribe_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *queryString, const wchar_t *queryDialect);

/* Do_Subscribe() retrieves a indication query from the user, selects from synchronous or asynchronous and executes the operation.  */
void Do_Subscribe(MI_Session *miSession, _In_z_ const wchar_t *namespaceName)
{
	MI_Char queryString[MAX_PATH];
	MI_Char queryDialect[MAX_PATH];
	wchar_t synchronous;

    /* Input the indication query string */
    GetUserInputString(L"Indication query string", queryString, sizeof(queryString)/sizeof(queryString[0]), L"select * from MSFT_ServiceStarted");

    /* Input the query dialect.  WMI supports WQL, but some systems may support the standard CQL dialect, or others. */
    GetUserInputString(L"Indication query dialect", queryDialect, sizeof(queryDialect)/sizeof(queryDialect[0]), L"WQL");


	synchronous = GetUserSelection(
				L"How do you want the Subscribe operation to be carried out?\n"
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
		Do_Subscribe_Synchronous(miSession, namespaceName, queryString, queryDialect);
		break;
	case L'2':
		Do_Subscribe_Asynchronous(miSession, namespaceName, queryString, queryDialect);
		break;
	}
}

struct SYNC_THREAD_INFO
{
	MI_Operation miOperation;
	MI_Uint32 instanceCount;
	MI_Result miResult;
	const wchar_t *errorMessage;
	MI_Instance *errorDetails;
};

/* _Do_Subscribe_Synchronous is run on a worker thread to retrieve all results.  When moreResults==MI_FALSE the thread will exit. */
DWORD WINAPI _Do_Subscribe_Synchronous(
  __in  LPVOID lpParameter
)
{
	MI_Boolean moreResults;
	struct SYNC_THREAD_INFO *threadInfo = (struct SYNC_THREAD_INFO *) lpParameter;

    /* Must loop through all results until moreResults == MI_FALSE */
	do
    {
        MI_Instance *miInstance;
        MI_Result _miResult;

        /* Retrieve a single indication result */
		_miResult = MI_Operation_GetIndication(&threadInfo->miOperation, &miInstance, NULL, NULL, &moreResults, &threadInfo->miResult, &threadInfo->errorMessage, &threadInfo->errorDetails);
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
            /* Dump the indication instance result */
            wprintf(L"------------------------------------------\n");
            Dump_MI_Instance(miInstance, MI_FALSE, 0);
            threadInfo->instanceCount++;
        }

        if (moreResults)
        {
            /* Small reminder to user how to exit the subscription */
		    wprintf(L"Press [0] to end subscription\n");
        }
        else
        {
            /* Dump the final outcome of the operation.  The errorMessage and errorDetails will be valid only
             * until the operation is closed.
             */
            wprintf(L"------------------------------------------\n");
            if ((threadInfo->instanceCount == 0) && threadInfo->miResult != MI_RESULT_OK)
            {
                wprintf(L"Operation failed, MI_Result=%s, errorString=%s, errorDetails=\n", MI_Result_To_String(threadInfo->miResult),threadInfo->errorMessage);
                Dump_MI_Instance(threadInfo->errorDetails, MI_FALSE, 0);
            }
            else
            {
                wprintf(L"Operation succeeded, number of instances=%u\n", threadInfo->instanceCount);
            }
            wprintf(L"------------------------------------------\n");

            wprintf(L"Press [0] to continue\n");
        }

    } while (threadInfo->miResult == MI_RESULT_OK && moreResults == MI_TRUE);

    /* moreResults == MI_FALSE, we will now exit the thread so the main thread can close the operation and continue. */

    return 0;
}


/* Do_Subscribe_Synchronous() creates a new thread to carry out the subscribe request so the main thread has 
 * an easy way of cancelling the operation when a user presses any key. Because subscriptions are normally
 * long running operations it is recommended to use the asynchronous version of the API.
 */
void Do_Subscribe_Synchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *queryString, const wchar_t *queryDialect)
{
    struct SYNC_THREAD_INFO threadInfo;
    MI_Result _miResult;
    HANDLE hThread;

    memset(&threadInfo, 0, sizeof(threadInfo));

    /* Note that the identity of the thread needs to be the same as the one the session was created on. */

    /* Note, although this sample does not include the PowerShell callbacks for extended semantics, they are allowed
     * on synchronous operations.  Allowable callbacks are:
     *      MI_OperationCallbacks.writeError
     *      MI_OperationCallbacks.writeMessage
     *      MI_OperationCallbacks.writeProgress
     */

    /* Initiate the subscribe operation.  Synchronous indication results are always retrieved through a call MI_Operation_GetIndication(). 
     * All operations must be closed with a call to MI_Operation_Close(), but all results must be processed before that.
     * The operation can be cancelled via MI_Operation_Cancel(), although even then all results must be consumed before the operation.
     * is closed. 
     */
    MI_Session_Subscribe(miSession, 0, NULL, namespaceName, queryDialect, queryString, NULL, NULL, &threadInfo.miOperation);

    /* Create a thread to do the synchronous retrieval of the results.
     * Note: The results need to be retrieved using the same security context that the operation was executed with.
     * If this thread is impersonating then the impersonation token will need to be transferred to the retrieval thread.
     */
    hThread = CreateThread(NULL, 0, _Do_Subscribe_Synchronous, &threadInfo, 0, NULL);
    if (hThread == NULL)
    {
        wprintf(L"MI_Operation_Cancel failed to CreateThread, error %d\n", GetLastError());
        return;
    }

    /* Block until the user presses 0. */
    GetUserSelection(
        L"Press [0] to end subscription\n",
        L"0");


    /* Now we need to cancel the operation as subscribe operations tend to not finish on their own, unless an error occurs. */
    _miResult = MI_Operation_Cancel(&threadInfo.miOperation, MI_REASON_NONE);
    if (_miResult != MI_RESULT_OK)
    {
            /* If this function returns a failure it means that an invalid parameter was passed in, or the identity of the thread
             * is different from the identity the operation was created with.  Both imply programming error.
             */
        wprintf(L"MI_Operation_Cancel failed, error %s\n", MI_Result_To_String(_miResult));
    }

    /* The retrieval thread will get a cancellation type error and will stop retriving the results.  At that point
     * the thread will shut down.  We will wait until that happens.
     */
    WaitForSingleObject(hThread, INFINITE);

    /* All operations must be closed.  If an operation is not closed the owning session will hang until the operations
     * are closed fully.  MI_Operation_Close will cancel an operation if it is still running, however results must be
     * consumed before the close can complete fully.  
     * For synchronous operations the MI_Operation_Close() method is synchronous until the final result has been consumed 
     * (moreResults == MI_FALSE).
     */
    _miResult = MI_Operation_Close(&threadInfo.miOperation);
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


/* Do_Subscribe_Asynchronous() carries out an subscribe operation asynchronously. The asynchronous callback
 * will keep being called until moreResults==MI_FALSE. 
 */
void Do_Subscribe_Asynchronous(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, const wchar_t *queryString, const wchar_t *queryDialect)
{
    MI_Operation miOperation = MI_OPERATION_NULL;
    MI_OperationCallbacks miOperationCallbacks = MI_OPERATIONCALLBACKS_NULL;
    struct InstanceResultCallback_Context instanceCallback_Context = {0};
	MI_Result _miResult;


    /* Add optional context information to callback structure so we can hold state
     * throughout the operation
     */
    miOperationCallbacks.callbackContext = &instanceCallback_Context;

    /* Set indication callback function.  This will keep being called until it is
     * called with moreResults==MI_FALSE.
     * It is this callback that puts subscribe operations into asyncronous mode.
     * Note: This is an indication result callback, NOT the instance result callback.
     */
    miOperationCallbacks.indicationResult = IndicationResultCallback;

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
    MI_Session_Subscribe(miSession, 0, NULL, namespaceName, queryDialect, queryString, NULL, &miOperationCallbacks, &miOperation);

    /* Wait for the user to press 0 so we can cancel the operation.  Subscribe operations tend to run for long periods of time
     * and are likely to stay active until the application shuts down.  We will simulate this by adding the user interaction.
     */
	GetUserSelection(
				L"Press [0] to end subscription\n",
				L"0");
    
	_miResult = MI_Operation_Cancel(&miOperation, MI_REASON_NONE);
    if (_miResult != MI_RESULT_OK)
    {
            /* If this function returns a failure it means that an invalid parameter was passed in, or the identity of the thread
             * is different from the identity the operation was created with.  Both imply programming error.
             */
        wprintf(L"MI_Operation_Cancel failed, error %s\n", MI_Result_To_String(_miResult));
    }

    /* Final miResult is here if needed: instanceCallback_Context.finalResult 
     * Any data from the callback cannot be accessed here because the lifetime of the data is 
     * only valid in the callback and until the operation is closed.
     * In this sample the operation handle is closed inside the instance callback.
     */
}

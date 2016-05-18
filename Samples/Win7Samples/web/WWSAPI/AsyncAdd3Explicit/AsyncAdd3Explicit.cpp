//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"

// Worker function that adds two numbers
HRESULT DoAdd(
    __in int a, 
    __in int b, 
    __out int* result, 
    __in_opt WS_ERROR* error)
{
    HRESULT hr;
    static const WS_STRING errorString = WS_STRING_VALUE(L"Negative numbers are not supported.");

    // To illustrate error handling, we won't support negative numbers
    if (a < 0 || b < 0)
    {
        // Add error information to error object  
        if (error != NULL)
        {
            WsAddErrorString(error, &errorString);
        }

        hr = E_NOTIMPL;
    }
    else
    {
        *result = a + b;
        hr = S_OK;
    }

    return hr;
}

// A struct to maintain the in/out parameters to the Add function
struct AddParameters
{
    int a;
    int b;
    int* sumPointer;
    WS_ERROR* error;
    WS_ASYNC_CONTEXT asyncContext;
};

// A thread function that adds two numbers
DWORD WINAPI AdderThread(
    __in void* threadParameter)
{
    // Get the parameters for Add which were passed in CreateThread
    AddParameters* addParameters = (AddParameters*)threadParameter;

    // Do the addition
    HRESULT hr = DoAdd(
        addParameters->a, 
        addParameters->b, 
        addParameters->sumPointer, 
        addParameters->error);

    // Make a copy of the async context
    WS_ASYNC_CONTEXT asyncContext = addParameters->asyncContext;

    // Free the parameters
    HeapFree(GetProcessHeap(), 0, addParameters);

    // Notify the caller that the async operation is complete
    // Since we have a dedicated thread for the callback, we can invoke long
    (asyncContext.callback)(hr, WS_LONG_CALLBACK, asyncContext.callbackState);

    return 1;
}

// An example of a function that can be called asynchronously
HRESULT Add(
    __in int a, 
    __in int b, 
    __out int* sumPointer, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    if (asyncContext == NULL)
    {
        // Invoked synchronously, so do the addition on calling thread
        return DoAdd(a, b, sumPointer, error);
    }
    else
    {
        // Invoked asynchronously

        // Decide whether to complete synchronously or asynchronously
        if (b == 0)
        {
            // Complete synchronously.  We have this case just as an illustration
            // that synchronous completion is possible when invoked asynchronously.
            return DoAdd(a, b, sumPointer, error);
        }
        else
        {
            // Complete asynchronously

            // Alloc space for in/out parameters
            AddParameters* addParameters;
            addParameters = (AddParameters*)HeapAlloc(GetProcessHeap(), 0, sizeof(AddParameters));
            if (addParameters == NULL)
            {
                return E_OUTOFMEMORY;
            }

            // Make a copy of in/out parameters
            addParameters->a = a;
            addParameters->b = b;
            addParameters->sumPointer = sumPointer;
            addParameters->error = error;
            addParameters->asyncContext = *asyncContext;

            // Create a thread which will do the work, passing parameters
            HANDLE threadHandle = CreateThread(NULL, 0, AdderThread, addParameters, 0, NULL);
            if (threadHandle == NULL)
            {
                // Free the parameters
                HeapFree(GetProcessHeap(), 0, addParameters);
                return HRESULT_FROM_WIN32(GetLastError());
            }

            // Close returned thread handle
            CloseHandle(threadHandle);

            // Indicate asynchronous completion
            return WS_S_ASYNC;
        }
    }
}

// Caller allocated state for use by AddThree.  In c++, this might be a class and
// the caller would be unaware of the internal details.
struct ADD_STATE
{
    WS_ERROR* error;
    WS_ASYNC_CONTEXT asyncContext;
    int c;
    int sum;
    int* result;
    BOOL async;
};

HRESULT CALLBACK Add2(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in ADD_STATE* addState)
{
    if (SUCCEEDED(hr))
    {
        // The operation succeeded, set the out parameter
        *addState->result = addState->sum;
    }
    if (addState->async)
    {
        // If the operation was async, then we need to invoke the callback
        addState->asyncContext.callback(hr, callbackModel, addState->asyncContext.callbackState);
    }
    return hr;
}

void CALLBACK Add2(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state)
{
    ADD_STATE* addState = (ADD_STATE*)state;
    // Mark the operation as being async
    addState->async = TRUE;
    Add2(hr, callbackModel, addState);
}

HRESULT CALLBACK Add1(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in ADD_STATE* addState)
{
    if (SUCCEEDED(hr))
    {
        // Add the third value, and continue at Add2
        WS_ASYNC_CONTEXT add2;
        add2.callback = Add2;
        add2.callbackState = addState;
        hr = Add(addState->sum, addState->c, &addState->sum, (addState->asyncContext.callback != NULL ? &add2 : NULL), addState->error);
        if (hr == WS_S_ASYNC)
        {
            // Add2 will get called asynchronously
            return hr;
        }
    }
    // Operation completed synchronously, so no callback will occur, so call directly
    return Add2(hr, callbackModel, addState);
}

void CALLBACK Add1(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state)
{
    ADD_STATE* addState = (ADD_STATE*)state;
    // Mark the operation as being async
    addState->async = TRUE;
    Add1(hr, callbackModel, addState);
}

HRESULT CALLBACK AddThree(
    __in ADD_STATE* addState, 
    __in int a, 
    __in int b, 
    __in int c, 
    __out int* result, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    HRESULT hr = S_OK;

    // Determine if the function should be executed synchronously or asynchronously
    if (asyncContext != NULL)
    {
        addState->asyncContext = *asyncContext;
    }
    else
    {
        addState->asyncContext.callback = NULL;
    }
    // Set up the state for the operation
    addState->error = error;
    addState->result = result;
    addState->c = c;
    addState->sum = 0;
    addState->async = FALSE;

    // Add the first two values and continue at Add1
    WS_ASYNC_CONTEXT add1;
    add1.callback = Add1;
    add1.callbackState = addState;
    hr = Add(a, b, &addState->sum, (addState->asyncContext.callback != NULL ? &add1 : NULL), error);
    if (hr == WS_S_ASYNC)
    {
        // Add1 will get called asynchronously
        return hr;
    }
    // Operation completed synchronously, so no callback will occur, so call directly
    return Add1(hr, WS_SHORT_CALLBACK, addState);
}

void CALLBACK AddThreeComplete(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* state)
{
    UNREFERENCED_PARAMETER(hr);
    UNREFERENCED_PARAMETER(callbackModel);

    HANDLE handle = (HANDLE)state;
    SetEvent(handle);
}

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    
    // Some numbers to add asynchronously
    // Add has the behavior that if the second parameter is 0, it will perform synchronously
    int ints[] = 
    { 
        1, 0, 0, // First add sync,  second add sync
        2, 1, 0, // First add async, second add sync
        3, 0, 2, // First add sync,  second add async
        4, 3, 2, // First add async, second add async
    };
    
    // Set up the event that will get signalled each time AddThree is complete
    HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (handle == NULL)
    {
        goto Exit;
    }
    
    // Set up the callback to use when performing the addition asynchronously
    WS_ASYNC_CONTEXT addThreeComplete;
    addThreeComplete.callback = AddThreeComplete;
    addThreeComplete.callbackState = handle;
    
    // Declare private data for AddThree
    ADD_STATE addState;
    
    // Perform the additions synchronously and asynchronously
    for (ULONG loop = 0; loop < 2; loop++)
    {
        // Add sets of integers that will cause different execution behavior when added asynchronously
        for (ULONG i = 0; i < sizeof(ints) / sizeof(int); i += 3)
        {
            wprintf(L"Adding %d,%d,%d %s...\n", ints[i], ints[i + 1], ints[i + 2], (loop == 0 ? L"synchronously" : L"asynchronously"));
    
            // Set up how the function will be called
            WS_ASYNC_CONTEXT* asyncContext;
            if (loop == 0)
            {
                // Perform the addition synchronously
                asyncContext = NULL;
            }
            else
            {
                // Perform the addition asynchronously
                asyncContext = &addThreeComplete;
            }
    
            // Perform the addition
            int sum;
            hr = AddThree(&addState, ints[i], ints[i + 1], ints[i + 2], &sum, asyncContext, NULL);
    
            if (hr == WS_S_ASYNC)
            {
                // If the operation is being performed asynchronously, then wait for it to complete
                WaitForSingleObject(handle, INFINITE);
            }
            if (SUCCEEDED(hr))
            {
                wprintf(L"Result: %d\n", sum);
            }
            else
            {
                wprintf(L"AddThree failed.\n");
            }
        }
    }
    Exit:
    if (handle != NULL)
    {
        CloseHandle(handle);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}

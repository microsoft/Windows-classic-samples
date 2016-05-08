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

// Print out rich error info
void PrintError(
    __in HRESULT errorCode, 
    __in_opt WS_ERROR* error)
{
    wprintf(L"Failure: errorCode=0x%lx\n", errorCode);

    if (errorCode == E_INVALIDARG || errorCode == WS_E_INVALID_OPERATION)
    {
        // Correct use of the APIs should never generate these errors
        wprintf(L"The error was due to an invalid use of an API.  This is likely due to a bug in the program.\n");
        DebugBreak();
    }

    HRESULT hr = S_OK;
    if (error != NULL)
    {
        ULONG errorCount;
        hr = WsGetErrorProperty(error, WS_ERROR_PROPERTY_STRING_COUNT, &errorCount, sizeof(errorCount));
        if (FAILED(hr))
        {
            goto Exit;
        }
        for (ULONG i = 0; i < errorCount; i++)
        {
            WS_STRING string;
            hr = WsGetErrorString(error, i, &string);
            if (FAILED(hr))
            {
                goto Exit;
            }
            wprintf(L"%.*s\n", string.length, string.chars);
        }
    }
Exit:
    if (FAILED(hr))
    {
        wprintf(L"Could not get error string (errorCode=0x%lx)\n", hr);
    }
}

//
// Async implementation
//

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

//
// Sync caller
//
void CallAddSync(
    __in int a, 
    __in int b)
{
    HRESULT hr;
    WS_ERROR* error = NULL;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    int sum;
    hr = Add(a, b, &sum, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    wprintf(L"%d\n", sum);
    
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    
    if (error != NULL)
    {
        WsFreeError(error);
    }
}

//
// Async caller
//

// Used to store state used to synchronize between callback and main thread
struct AddCompletion
{
    HANDLE eventHandle;
    HRESULT errorCode;
};

// WS_ASYNC_CALLBACK that is called when add completes asynchronously
void CALLBACK OnAddComplete(
    __in HRESULT hr, 
    __in WS_CALLBACK_MODEL callbackModel, 
    __in void* callbackState)
{
    UNREFERENCED_PARAMETER(callbackModel);
    
    // Get the callback state
    AddCompletion* addCompletion = (AddCompletion*)callbackState;

    // Store the HRESULT
    addCompletion->errorCode = hr;

    // Since we just set an event, we are ok with a callbackModel of WS_SHORT_CALLBACK or WS_LONG_CALLBACK

    // Signal main thread that the operation is complete
    SetEvent(addCompletion->eventHandle);
}

void CallAddAsync(
    __in int a, 
    __in int b)
{
    HRESULT hr;
    WS_ERROR* error = NULL;
    HANDLE eventHandle = NULL;
    int* sumPointer = NULL;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create an event handle that will be signaled in callback
    eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (eventHandle == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
    // Store information used by the callback when add completes
    AddCompletion addCompletion;
    addCompletion.eventHandle = eventHandle;
    
    // Allocate space to store return value
    sumPointer = (int*)HeapAlloc(GetProcessHeap(), 0, sizeof(int));
    if (sumPointer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    // Specify the callback to call if function completes synchronously
    // along with the state to pass to the callback (AddCompletion structure)
    WS_ASYNC_CONTEXT asyncContext;
    asyncContext.callback = OnAddComplete;
    asyncContext.callbackState = &addCompletion;
    
    // Call the function asynchronously
    hr = Add(a, b, sumPointer, &asyncContext, error);
    
    // Zero out asyncContext to illustrate that async function should have copied it
    ZeroMemory(&asyncContext, sizeof(asyncContext));
    
    if (hr == WS_S_ASYNC)
    {
        // Function completed asynchronously
    
        // Wait for callback to signal completion
        if (WaitForSingleObject(eventHandle, INFINITE) != WAIT_OBJECT_0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    
        // Get error code that was stored by callback
        hr = addCompletion.errorCode;
    }
    
    wprintf(L"%d\n", *sumPointer);
    
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    
    if (eventHandle != NULL)
    {
        CloseHandle(eventHandle);
    }
    
    if (sumPointer != NULL)
    {
        // Free value
        HeapFree(GetProcessHeap(), 0, sumPointer);
    }
    
    if (error != NULL)
    {
        WsFreeError(error);
    }
}

// Main entry point
int __cdecl wmain()
{
    CallAddSync(0, 0);
    CallAddSync(1, 2);
    CallAddAsync(0, 0);
    CallAddAsync(1, 2);
    return 0;
}

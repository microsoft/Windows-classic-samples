// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#include "DebugHelper.h"
#include <strsafe.h>


// Print a debug string: global-scope method, which simplifies calls into
// this class.
void DebugPrintfW(LPCWSTR pFormat, ...)
{
    static DebugHelper debugHelper;

    va_list vl;
    va_start(vl,pFormat);

    // Call the class's public method.
    debugHelper.DebugVPrintfW(pFormat,vl);

    va_end(vl);
}


// Print a debug string: one external interface.
void DebugHelper::DebugPrintfW(LPCWSTR pFormat, ...)
{
    va_list vl;
    va_start(vl,pFormat);

    // Call the class's public method.
    DebugVPrintfW(pFormat,vl);

    va_end(vl);
}


// Print a debug string: helper method, called by wrapper methods.
void DebugHelper::DebugVPrintfW(LPCWSTR pFormat, va_list vl)
{
    HRESULT hr = S_OK;

    SYSTEMTIME st = {0};

    if (debugOutputBuffer &&
        debugOutputBufferTemp)
    {
        GetLocalTime(&st);

        // Print the line's prefix.
        hr = StringCbPrintfW(debugOutputBufferTemp, 
                             debugOutputBufferLen,
                             L"[T%d: %02u/%02u/%02u @ %02u:%02u:%02u.%03u] %s\n",
                             GetCurrentThreadId(),
                             st.wYear,
                             st.wMonth,
                             st.wDay,
                             st.wHour,
                             st.wMinute,
                             st.wSecond,
                             st.wMilliseconds,
                             pFormat);
        if (FAILED(hr))
        {
            OutputDebugStringW(L"DebugHelper: Caller attempted to use a debug output format string that was too long!\n");
            goto Cleanup;
        }


        // Print the given arguments as per the resultant format.
        hr = StringCbVPrintfW(debugOutputBuffer,
                              debugOutputBufferLen,
                              debugOutputBufferTemp,
                              vl);
        if (FAILED(hr))
        {
            OutputDebugStringW(L"DebugHelper: Caller attempted to print a debug string that was too long!\n");
            goto Cleanup;
        }


        // Print the string to the debugger.
        OutputDebugStringW(debugOutputBuffer);

    }
    else
    {
        hr = E_POINTER;
        OutputDebugStringW(L"DebugHelper: Bad temporary buffer pointers\n");
        goto Cleanup;
    }
        

    Cleanup:
    if (FAILED(hr))
    {
        // Fail in a useful manner -- just display the caller's literal
        // format string in the debugger, without expanding any format
        // fields. (The output strings may still be useful to help diagnose
        // problems, whether or not any actual runtime values are shown.)

        OutputDebugStringW(pFormat);
        OutputDebugStringW(L"\n");
    }

    return;
}

    
// Constructor.
DebugHelper::DebugHelper()
{
    heapHandle = NULL;
    debugOutputBuffer = NULL;
    debugOutputBufferTemp = NULL;

    Initialize();
}


// Destructor.
DebugHelper::~DebugHelper()
{
    Finalize();
}


// The code behind the constructor.
void DebugHelper::Initialize()
{
    // Create a heap to contain our memory allocations.
    heapHandle = HeapCreate( 0, 1, 0 );
    if ( heapHandle == NULL )
    {
        OutputDebugStringW(L"DebugHelper::Initialize(): Failed to create heap!  Unable to create data buffers for formatted output strings.\n");
        goto Cleanup;
    }

    //
    // Allocate space for the two buffers.
    //

    // Buffer #1: final output string.
    debugOutputBuffer     = (PWCHAR)HeapAlloc( heapHandle, HEAP_ZERO_MEMORY,
                                               debugOutputBufferLen);

    // Buffer #2: intermediate output string; contains data prepended to every
    //            debug output string, such as a timestamp.
    debugOutputBufferTemp = (PWCHAR)HeapAlloc( heapHandle, HEAP_ZERO_MEMORY,
                                               debugOutputBufferLen);

    // Make sure we either have both buffers, or neither buffer.
    if (debugOutputBuffer == NULL || 
        debugOutputBufferTemp == NULL)
    {
        OutputDebugStringW(L"\n"
                           L"DebugHelper::Initialize(): Failed to create formatted output string buffers!\n"
                           L"                           Debug output strings will be displayed _without_ expanding format fields (i.e., \"%d\").\n\n");

        // Free any partial memory already allocated.
        CleanupMem();

        goto Cleanup;
    }

    // Report that the class is completely initialized.
    OutputDebugStringW(L"DebugHelper::Initialize(): Initialization completed successfully.\n");

 Cleanup:
    return;
}


// The code behind the destructor.
void DebugHelper::Finalize()
{
    OutputDebugStringW(L"DebugHelper::Finalize(): Finalize called, cleaning up object.\n");
    CleanupMem();
}

// Cleanup dynamic resources used by this class.
// Called either at destruction time, or when Initialization encounters problems allocating.
void DebugHelper::CleanupMem()
{
    BOOL fOk = FALSE;

    if (heapHandle)
    {
        // Report that the class is being cleaned up, if we can.
        if (debugOutputBuffer || debugOutputBufferTemp)
        {
            OutputDebugStringW(L"DebugHelper::Cleanup(): Freeing all allocated memory, and cleaning up this object.\n");
        }

        // Release the memory used by the two buffers.

        if (debugOutputBufferTemp)
        {
            fOk = HeapFree(heapHandle, 0, debugOutputBufferTemp);
            if ( fOk == FALSE )
            {
                OutputDebugStringW(L"DebugHelper::Cleanup(): Failed to free temp buffer!Heap memory LEAKED!!!\n");
            }
        }

        if (debugOutputBuffer)
        {
            fOk = HeapFree(heapHandle, 0, debugOutputBuffer);
            if ( fOk == FALSE )
            {
                OutputDebugStringW(L"DebugHelper::Cleanup(): Failed to free buffer!Heap memory LEAKED!!!\n");
            }
        }

        // Release the containing heap.
        fOk = HeapDestroy( heapHandle );
        if ( fOk == FALSE )
        {
            OutputDebugStringW(L"DebugHelper::Cleanup(): Failed to destroy heap!Heap object LEAKED!!!\n");
        }
        else
        {
            OutputDebugStringW(L"DebugHelper::Cleanup(): Heap successfully destroyed.\n");
        }
    }
    else
    {
        OutputDebugStringW(L"DebugHelper::Cleanup(): .\n");
    }

    debugOutputBufferTemp = NULL;
    debugOutputBuffer = NULL;
    heapHandle = NULL;

}




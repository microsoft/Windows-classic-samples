// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#pragma once
#ifndef __DEBUGHELPER_H__
#define __DEBUGHELPER_H__

#include <windows.h>


// The length of the memory buffers used to construct the final output string.
static const size_t DEBUGHELPER_OUTPUT_BUFFER_LENGTH = 512 * sizeof(WCHAR);


// This class provides a simple way to print debugger output, using
// printf-like format strings.
class DebugHelper
{
 public:
    // Constructor.
    DebugHelper();

    // Destructor.
    ~DebugHelper();


    // Print a debug string: one external interface.
    void DebugPrintfW(LPCWSTR pFormat, ...);


    // Print a debug string: helper method, called by wrapper methods.
    void DebugVPrintfW(LPCWSTR pFormat, va_list vl);



 private:
    // The code behind the constructor.
    void Initialize();

    // The code behind the destructor.
    void Finalize();

    // Cleans up allocated resources
    void CleanupMem();


    // Heap handle used for the buffers below.
    HANDLE heapHandle;

    // Used for the final debug output string.
    PWCHAR debugOutputBuffer;

    // Used for an intermediate string (thread id, timestamp, etc.)
    PWCHAR debugOutputBufferTemp;

    // Size of both buffers.
    const static size_t debugOutputBufferLen = DEBUGHELPER_OUTPUT_BUFFER_LENGTH;


    // Assignment operator not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation of
    // the assignment operator, which isn't guaranteed to handle member
    // variables correctly.
    DebugHelper& operator=(const DebugHelper &rhs) throw();
    
    // Copy constructor not implemented. This explicitly prevents the
    // compiler from automatically providing a default implementation
    // of the copy constructor, which isn't guaranteed to handle member
    // variables correctly.
    DebugHelper(const DebugHelper &rhs) throw();

};


// Print a debug string: global-scope method, which simplifies calls into
// this class.
void DebugPrintfW(LPCWSTR pFormat, ...);



#endif  //  __DEBUGHELPER_H__

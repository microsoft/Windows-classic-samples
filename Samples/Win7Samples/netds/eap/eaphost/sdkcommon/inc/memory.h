// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#ifndef _MEMORY_H_
#define _MEMORY_H_

#pragma once

#include <windows.h>

namespace SDK_METHOD_SAMPLE_COMMON
{

/// The heap handle to use for all allocations.
extern HANDLE g_localHeap;

/// Set up a private heap for our memory allocations.
DWORD
InitializeHeap(
);

/// Clean up our private heap, and all contained memory allocations.
DWORD
CleanupHeap(
);

/// Allocate memory on our heap.
DWORD
AllocateMemory(
    IN     DWORD sizeInBytes,
    IN OUT PVOID *pBuffer
);

/// Free memory allocated on our heap using AllocateMemory().
DWORD
FreeMemory(
    IN OUT PVOID *pBuffer
);


}// End "namespace SDK_METHOD_SAMPLE_COMMON

#endif _MEMORY_H_


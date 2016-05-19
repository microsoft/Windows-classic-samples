// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"
#include "memory.h"
#include "SdkCommon.h"

namespace SDK_METHOD_SAMPLE_COMMON
{

/// The heap handle to use for all allocations.  
HANDLE g_localHeap = NULL;

/**
  * Initialize a local memory heap for our use.
  *
  *
  * @return A Win32 error code, indicating success or failure.
  *
  *
  * @note One heap is shared across all threads that uses this code; therefore,
  *       care should be taken to ensure this function is called only once.
  *       For example, calling it inside DllMain() will ensure this.
  */
DWORD
InitializeHeap(
)
{
	DWORD dwErr = ERROR_SUCCESS;

	g_localHeap = NULL;   // Make sure the global value is initialized.

	g_localHeap = HeapCreate(0,  // Serialize access to the heap
	                                       1,   // Create a 0-byte heap; allocations will increase it, as necessary.
	                                       0);  // Heap is growable; limited only by available memory
	if (! g_localHeap)
	{
		dwErr = GetLastError();
		EapTrace("InitializeHeap(): Heap creation failed -- hit error %d!", dwErr);
	}

	return dwErr;
}


/**
  * Clean up the local memory heap.
  *
  * This will free our local heap, and all allocations inside it.
  *
  *
  * @return A Win32 error code, indicating success or failure.
  *
  *
  * @note One heap is shared across all threads that uses this code; therefore,
  *       care should be taken to ensure this function is called only once.
  *       For example, calling it inside DllCanUnloadNow() will ensure this.
  *
  * @note When this function succeeds, it will also initialize the heap handle
  *       to NULL, to prevent the handle from being used again.
  */
DWORD
CleanupHeap(
)
{
	DWORD dwErr = ERROR_SUCCESS;
	BOOL  fOk   = TRUE;

	// Sanity check.
	if (! g_localHeap)
	{
		EapTrace("CleanupHeap(): Heap has not been created yet; exiting.");
		goto LDone;
	}   

	// Free the heap.
	fOk = HeapDestroy(g_localHeap);
	if (! fOk)
	{
		dwErr = GetLastError();
		EapTrace("CleanupHeap(): Heap destruction failed -- hit error %d!", dwErr);
		goto LDone;
	}

	// Reinitialize the heap handle.
	g_localHeap = NULL;

LDone:
	return dwErr;
}

    

/**
  * Allocate memory, using our local heap.
  *
  *
  * @param  sizeInBytes  [in]  The byte-length of the buffer to allocate.
  *
  * @param  ppBuffer     [out] Pointer to the new data buffer.
  *
  *
  * @return A Win32 error code, indicating success or failure.
  *             ERROR_INVALID_HANDLE - if AllocateMemory is called before InitializeHeap().
  */
DWORD
AllocateMemory(
    IN     DWORD sizeInBytes,
    IN OUT PVOID *ppBuffer
)
{
	DWORD dwErr   = ERROR_SUCCESS;

	// Sanity checks.
	if (ppBuffer == NULL)
	{
		EapTrace("AllocateMemory(): Invalid buffer pointer passed in!");
		dwErr = ERROR_INVALID_PARAMETER;
		goto LDone;
	}

	if (! g_localHeap)
	{
		EapTrace("AllocateMemory(): Heap has not been created yet!");
		dwErr = ERROR_INVALID_HANDLE;
		goto LDone;
	}   

	// Allocate a new memory block on the heap.  Use HEAP_ZERO_MEMORY to
	// initialize the memory block, so callers don't have to do so.
	*ppBuffer = (PBYTE)HeapAlloc(g_localHeap, HEAP_ZERO_MEMORY, sizeInBytes);
	if (*ppBuffer == NULL)
	{
		EapTrace("AllocateMemory(): Not enough memory");
		dwErr = ERROR_OUTOFMEMORY;
		goto LDone;
	}

LDone:
	return dwErr;
}


/**
  * Free memory allocations from our local heap.
  *
  *
  * @param  ppBuffer   [out] Pointer to the data buffer to free.
  *
  *
  * @return A Win32 error code, indicating success or failure.
  *             ERROR_INVALID_HANDLE - if FreeMemory is called before InitializeHeap().
  *
  *
  * @note When this function succeeds, it will also initialize the buffer
  *       pointer to NULL, to prevent the calling function from referencing
  *       the former allocation again.
  */
DWORD
FreeMemory(
    IN OUT PVOID *ppBuffer
)
{
	DWORD dwErr = ERROR_SUCCESS;
	BOOL  fOk   = TRUE;

	// Sanity checks.
	if (ppBuffer == NULL)
	{
		EapTrace("FreeMemory(): Invalid buffer pointer passed in!");
		dwErr = ERROR_INVALID_PARAMETER;
		goto LDone;
	}

	if (! g_localHeap)
	{
		EapTrace("FreeMemory(): Heap has not been created yet!");
		dwErr = ERROR_INVALID_HANDLE;
		goto LDone;
	}   

	// Don't try to free NULL pointers.
	if (*ppBuffer == NULL)
	{
		// Ignore this silently.
		goto LDone;
	}

	fOk = HeapFree(g_localHeap, 0, *ppBuffer);
	if (! fOk)
	{
		dwErr = GetLastError();
		EapTrace("FreeMemory(): Error %d while freeing memory!", dwErr);
		goto LDone;
	}

	// Re-initialize the buffer pointer.
	*ppBuffer = NULL;

LDone:
	return dwErr;
}

}// End "namespace SDK_METHOD_SAMPLE_COMMON

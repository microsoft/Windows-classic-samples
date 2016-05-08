// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"
#include <windows.h>
#include <rtutils.h>
#include "SampleSupplicant.h"
#include "memory.h"

using namespace SDK_METHOD_SAMPLE_COMMON;

/// Pointer to the Identity Request Packet
BYTE *g_pIdentityRequest = NULL;
/// Size of the Identity Request Packet
DWORD g_dwIdentityRequestPacketLen = 0;
/// Handle to tracing calls.
DWORD g_dwEapTraceId;

int __cdecl main()
{
	DWORD retCode = 0;

	//"SampleSupplicant.log" - the trace file that will be generated.
	g_dwEapTraceId = TraceRegister(L"SampleSupplicant");

	//
	// Create the heap we'll be using for memory allocations.
	//
	retCode = InitializeHeap();
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}

	//
	// Initialize the resources used for authentication.
	//
	retCode = Initialize();
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}

	//
	// Fake the presence of a NAS (Access Point, Remote Access Server) that will send the supplicant the 
	// identity request. Here, we generate ourselves the Identity Request Packet.
	//
	retCode = MakeIdentityRequestMessage(&g_pIdentityRequest, &g_dwIdentityRequestPacketLen);
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}

	//
	// Begin the actual authentication.
	//
	retCode = BeginAuthentication();
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}

Cleanup:
	//
	//Clean up the resources used for authentication.
	//
	CleanUp();
	
	//
	//Free the buffer that stores the initial Identity Request Packet.
	//
	if(g_pIdentityRequest != NULL)
		FreeMemory((PVOID *)&g_pIdentityRequest);

	TraceDeregister(g_dwEapTraceId);
	g_dwEapTraceId = INVALID_TRACEID;
	
	//
	// Clean up our internal heap.
	//
	CleanupHeap();

	return retCode;
}

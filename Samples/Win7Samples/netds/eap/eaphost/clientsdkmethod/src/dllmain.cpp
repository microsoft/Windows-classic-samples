// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"
#include <windows.h>
#include <rtutils.h>
#include "memory.h"
#include "SdkCommon.h"
#include "SampleEapMethodClient.h"

using namespace SDK_METHOD_SAMPLE_COMMON;

DWORD        g_dwEapTraceId;
HINSTANCE   g_hInstance;


/**
  * Implementation of the DllMain API function.
  *
  * This function is called when loading & unloading the DLL.
  *
  * @return This function always returns TRUE.
  *
  * @note During initialization, the code registers itself for trace logging; initializing the heap
  *           that will be used to allocate memory in this dll.
  */
BOOL
WINAPI
DllMain(
   HINSTANCE hInstance,
   DWORD dwReason,
   LPVOID lpReserved
   )
{
	DWORD retCode = ERROR_SUCCESS;

	// We don't use this parameter, so hide it from PREfast and Lint.
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		//"SampleEapPeer.log" - the trace file that will be generated.
		g_dwEapTraceId = TraceRegister(L"SampleEapPeer");

		g_hInstance = hInstance;

		// Create the heap we'll be using for memory allocations.
		retCode = InitializeHeap();
		if(retCode != ERROR_SUCCESS)
			goto Cleanup;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TraceDeregister(g_dwEapTraceId);
		g_dwEapTraceId = INVALID_TRACEID;

		// Clean up our internal heap.
		retCode = CleanupHeap();
		if(retCode != ERROR_SUCCESS)
			goto Cleanup;
	}

Cleanup:
	if(retCode != ERROR_SUCCESS)
		return FALSE;
	else
		return TRUE;
}


/**
  * Implementation of the DllRegisterServer API function.
  *
  * This function is called by "regsvr32 filename.dll". This DLL uses this API
  * function to register itself with EAP Host and to create its default registry 
  * configuration.
  *
  * @return An HRESULT value indicating success or failure.
  */
STDAPI
DllRegisterServer( VOID )
{
	DWORD dwDisp = 0;
	DWORD retCode = 0;
	HKEY hkeapHost = 0;
	HKEY hkeapMethod = 0;
	wchar_t *dllpathValue = NULL;
	DWORD dllPathValueLength = 0;
	DWORD peerDialogValue = 0;

	// Check if the key -- "HKLM\System\CurrentControlSet\Services\EapHost\Methods"
	// exist. The absence of key implies EapHost is not properly installed.
	retCode = RegCreateKeyExW(HKEY_LOCAL_MACHINE, eapHostKeyName, 
							0, NULL, REG_OPTION_NON_VOLATILE,
							KEY_ALL_ACCESS, NULL, &hkeapHost, &dwDisp);
	if(retCode != ERROR_SUCCESS)
	{
		// Trace Error --- EapHost not properly installed.
		goto Cleanup;
	}

	// Create the subkey -- "311\40" which is the MethodId of the Sample.
	retCode = RegCreateKeyExW(hkeapHost, eapMethodName, 
							0, NULL, REG_OPTION_NON_VOLATILE,
							KEY_ALL_ACCESS, NULL, &hkeapMethod, &dwDisp);
	if(retCode != ERROR_SUCCESS)
	{
		// Trace Error --- EapMethod Key could not be created properly.
		goto Cleanup;
	}

	// Set the Value --- "PeerFriendlyName" = "SdkPeerEapMethod"
	retCode = RegSetValueExW(hkeapMethod,
					peerFriendlyName, 
					0,
					REG_SZ,
					(LPBYTE)peerFriendlyNameValue,
					(DWORD)sizeof(peerFriendlyNameValue));
	if(retCode != ERROR_SUCCESS)
	{
		// Trace Error --- Could not set Peer Friendly Name
		goto Cleanup;
	}

	// Get the complete location of the Peer Eap Method Dll Path.
	// Location = Current Directory + Dll Name
	retCode = GetFullPath(dllpathValue, dllPathValueLength, (LPWSTR)peerMethodDllName, sizeof(peerMethodDllName));
	if(retCode != ERROR_SUCCESS)
		goto Cleanup;

	// Set the Value --- "PeerDllPath" = "CurrentDirectory + DllName"
	retCode = RegSetValueExW(hkeapMethod,
				peerDllPath,
				0, 
				REG_EXPAND_SZ, 
				(LPBYTE)dllpathValue,
				dllPathValueLength);
	if(retCode != ERROR_SUCCESS)
	{
		// Trace Error --- Could not set Peer Dll Path
		goto Cleanup;
	} 

	// Set the Value --- "Properties"
	retCode = RegSetValueExW(hkeapMethod,
					properties,
					0,
					REG_DWORD,
					(LPBYTE) &propertiesValue, 
					sizeof(DWORD));
	if(retCode != ERROR_SUCCESS)
	{
		// Trace Error --- Could not set Properties 
		goto Cleanup;
	}

	// Set the Value --- "PeerInvokeUsernameDialog" = 0
	retCode = RegSetValueExW(hkeapMethod,
					peerInvokeUserNameDialog,
					0, 
					REG_DWORD, 
					(LPBYTE) &peerDialogValue, 
					sizeof(DWORD));
	if(retCode != ERROR_SUCCESS)
	{
		// Trace Error --- Could not set peerInvokeUserNameDialog 
		goto Cleanup;
	}

	// Set the Value --- "PeerInvokePasswordDialog" = 0
	retCode = RegSetValueExW(hkeapMethod,
					peerInvokePasswordDialog,
					0,
					REG_DWORD, 
					(LPBYTE) &peerDialogValue,
					sizeof(DWORD));
	if(retCode != ERROR_SUCCESS)
	{
		// Trace Error --- Could not set peerInvokePasswordDialog 
		goto Cleanup;
	}

Cleanup:
	if(hkeapMethod)
		RegCloseKey(hkeapMethod);
	if(hkeapHost)
		RegCloseKey(hkeapHost);
	FreeMemory((PVOID *)&dllpathValue);

	return HRESULT_FROM_WIN32(retCode);
}


/**
  * Implementation of the DllUnregisterServer API function.
  *
  * This function is called by "regsvr32 /u filename.dll". This DLL uses this
  * API function to unregister itself with EAPHost (removing itself from the list
  * of EAP Methods), and to delete its registry configuration.
  *
  *
  * @return An HRESULT value indicating success or failure.
  */
STDAPI
DllUnregisterServer( VOID )
{
	DWORD retCode = ERROR_SUCCESS;
	HKEY hkeapHostMethod = 0;

	// Check if the key -- "HKLM\System\CurrentControlSet\Services\EapHost\Methods\311"
	// exist. The absence of key implies EapHost or SdkEapMethod is not properly installed.
	retCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, eapHostMethodKeyName,
					0, KEY_ALL_ACCESS, &hkeapHostMethod);
	if(retCode != ERROR_SUCCESS)
	{
		//  Trace Error --- EapHost or EapMethod not properly installed.
		goto Cleanup;
	}

	// Delete the subkey - "40" which is the MethodId of Sample Eap.
	retCode = RegDeleteKeyW(hkeapHostMethod, eapMethodId);
	if(retCode != ERROR_SUCCESS)
	{
		//  Trace Error --- EapHost or EapMethod not properly installed.
		goto Cleanup;
	}

Cleanup:
	if(hkeapHostMethod)
		RegCloseKey(hkeapHostMethod);
	
	return HRESULT_FROM_WIN32(retCode);
}

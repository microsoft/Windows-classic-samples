// Copyright (c)  Microsoft Corporation, All Rights Reserved
///////////////////////////////////////////////////////////////////
//
//	SamplCli.cpp : Refresher client implementation file
//
//	RefTest is a simple WMI client that demonstrates how to use a 
//	high performance refresher.  It will add a number of 
//	Win32_BasicHiPerf instances as well as a single Win32_BasicHiPerf
//	enumerator.  The optional command line argument specifies a remote
//	WMI connection.  The default is the local machine.  The syntax
//	is as follows:
//
//	refreshertest.exe <server name>
//
//	  server name: the name of the remote machine where the provider
//		id located. 
//
//	Notes: 
//	
//		1) Ensure that Win32_BasicHiPerf is properly set up.  See the
//			BasicHiPerf\BasicHiPerf.html file.
//
//		2) Error handling has been minimized in the sample code for 
//			the purpose of clarity.
//
///////////////////////////////////////////////////////////////////

#define UNICODE

#include <windows.h>
#include <stdio.h>
#include <wbemcli.h>
#include <strsafe.h>

#include "RefClint.h"

void RefreshLoop();

///////////////////////////////////////////////////////////////////
//
//	Globals and constants
//
///////////////////////////////////////////////////////////////////

IWbemServices*	g_pNameSpace = NULL;		// A WMI namespace pointer 

///////////////////////////////////////////////////////////////////
//
//	Code
//
///////////////////////////////////////////////////////////////////

int main( int argc, char *argv[] )
///////////////////////////////////////////////////////////////////
//
//	Entry point function to exercise IWbemObjectInternals interface.
//
///////////////////////////////////////////////////////////////////
{
	HRESULT hRes = WBEM_NO_ERROR;

	IWbemLocator*	pWbemLocator = NULL;

	BSTR	bstrNameSpace = NULL;

	WCHAR	wcsSvrName[MAX_PATH + 1];
	StringCbCopyW(wcsSvrName, sizeof(wcsSvrName), L"." );

	// Initialize COM
	// ==============

	hRes = CoInitializeEx( NULL, COINIT_MULTITHREADED );
	if ( FAILED( hRes ) )
		return 0;

	// Setup default security parameters
	// =================================
	//  NOTE:
	//  When using asynchronous WMI API's remotely in an environment where the "Local System" account 
	//  has no network identity (such as non-Kerberos domains), the authentication level of 
	//  RPC_C_AUTHN_LEVEL_NONE is needed. However, lowering the authentication level to 
	//  RPC_C_AUTHN_LEVEL_NONE makes your application less secure. It is wise to
	//	use semi-synchronous API's for accessing WMI data and events instead of the asynchronous ones.

	hRes = CoInitializeSecurity( NULL, -1, NULL, NULL, 
											RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
											RPC_C_IMP_LEVEL_IMPERSONATE, 
											NULL, 
											EOAC_SECURE_REFS, //change to EOAC_NONE if you change dwAuthnLevel to RPC_C_AUTHN_LEVEL_NONE
											NULL );
	if ( FAILED( hRes ) )
		goto cleanup;


// Initialize the environment based on the command line arguments
// ==============================================================

	if ( argc >= 2 )
	{
		// The remote server name
		// ======================
		memset(wcsSvrName, NULL, sizeof(wcsSvrName));

		MultiByteToWideChar( CP_ACP, 0L, argv[1], -1, wcsSvrName, sizeof(wcsSvrName)/sizeof (WCHAR) - 1);
	}


// Attach to WinMgmt
// =================

	// Get the local locator object
	// ============================

	hRes = CoCreateInstance( CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**) &pWbemLocator );
	if (FAILED(hRes))
		goto cleanup;

	// Connect to the desired namespace
	// ================================

	WCHAR	wszNameSpace[MAX_PATH + 20];
	StringCbPrintfW(wszNameSpace, sizeof(wszNameSpace), L"\\\\%s\\root\\cimv2", wcsSvrName );

	bstrNameSpace = SysAllocString( wszNameSpace );
	if (bstrNameSpace == NULL)
		goto cleanup;

	hRes = pWbemLocator->ConnectServer(	bstrNameSpace,	// NameSpace Name
										NULL,			// UserName
										NULL,			// Password
										NULL,			// Locale
										0L,				// Security Flags
										NULL,			// Authority
										NULL,			// Wbem Context
										&g_pNameSpace	// Namespace
										);

	if ( FAILED( hRes ) )
		goto cleanup;

	// Start the refresh loop
	// ======================
	
	RefreshLoop();


// Cleanup
// =======
cleanup:
	SysFreeString( bstrNameSpace );

	if ( NULL != g_pNameSpace )
		g_pNameSpace->Release();

	if ( NULL != pWbemLocator )
		pWbemLocator->Release();

	CoUninitialize();

	return 0;
}

void RefreshLoop()
///////////////////////////////////////////////////////////////////
//
//	RefreshLoop will create a new refresher and configure it with
//	a set of instances and an enumerator.  It will then enter a loop
//	which refreshes and displays the counter data.  Once it has 
//	completed the loop, the members of the refresher are removed.
//
///////////////////////////////////////////////////////////////////
{
	HRESULT		hRes		= WBEM_NO_ERROR;
	long		lLoopCount	= 0;		// Refresh loop counter
	CRefClient	aRefClient;

// Initialize our container class
// ==============================

	hRes = aRefClient.Initialize(g_pNameSpace);
	if (FAILED(hRes))
		goto cleanup;


// Add items to the refresher
// ==========================

	// Add an enumerator
	// =================

	hRes = aRefClient.AddEnum();
	if ( FAILED( hRes ) )
		goto cleanup;

	// Add objects
	// ===========

	hRes = aRefClient.AddObjects();
	if ( FAILED( hRes ) )
		goto cleanup; 


// Begin the refreshing loop
// =========================

	for ( lLoopCount = 0; lLoopCount < cdwNumReps; lLoopCount++ )
	{
		// Refresh!!
		// =========

		hRes = aRefClient.Refresh();
		if ( FAILED ( hRes ) )
		{
			printf("Refresh failed: 0x%X\n", hRes);
			goto cleanup;
		}

		printf( "Refresh number: %d\n", lLoopCount );

		aRefClient.ShowObjectData();
		aRefClient.ShowEnumeratorData();

		printf( "\n" );

	}	// FOR Refresh

// Cleanup
// =======
cleanup:

	aRefClient.RemoveEnum();
	aRefClient.RemoveObjects();

	return;
}


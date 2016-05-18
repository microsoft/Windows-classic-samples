//***************************************************************************

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//	File:  WMIDiskPerf.cpp
//
//	Description:
//		Connects to the WMI server and calls DiskPerfDetails( ) to
//		retrieve partition properties.
//
//	Part of :	WMIDiskPerf
//
//  History:	
//
//***************************************************************************


#include <wbemidl.h>
#include <lmcons.h>
#include <stdio.h>
#include <strsafe.h>


void			DiskPerfDetails( IWbemServices * pIWbemServices );
BOOL			InitSecurity( void );
DWORD WINAPI	WaitThread( HANDLE hWait );


//==============================================================================
//	main( )
//==============================================================================
int main( int argc, char **argv )
{
	HRESULT	hr;
	DWORD	ThreadId, dwLen = MAX_COMPUTERNAME_LENGTH + 1;
	HANDLE	hWait, hThread;
	BSTR	pNamespace;
	TCHAR	szSysName[MAX_COMPUTERNAME_LENGTH + 3];

	IWbemLocator	*pIWbemLocator  = NULL;
	IWbemServices	*pIWbemServices = NULL;

	if ( !SUCCEEDED( CoInitialize( NULL ) ) || !InitSecurity( ) ) 
	{
		printf( "COM stuff is hosed!\r\n" );

		return 0;
	}

	// Create an instance of the WbemLocator interface.
	if ( CoCreateInstance( CLSID_WbemLocator,
	                       NULL,
	                       CLSCTX_INPROC_SERVER,
	                       IID_IWbemLocator,
	                       (LPVOID *) &pIWbemLocator ) == S_OK )
	{

		StringCbCopy(szSysName, sizeof(szSysName), "\\\\");
		BOOL bRes = GetComputerName( szSysName + 2, &dwLen );
		if (!bRes)
		{
			printf("Could not get local machine name.\n");
			CoUninitialize();
			return 0;
		}
		pNamespace = SysAllocString( L"root\\WMI" );
		if (!pNamespace)
		{
			printf("Out of memory\n");
			CoUninitialize();
			return 0;
		}
		// Display connect message and start wait feedback thread
		printf( "Connecting to %s WMI server at ", szSysName );
		wprintf( L"%s...", pNamespace );

		// make a wait event and kick off wait thread
		hWait = CreateEvent( NULL, TRUE, FALSE, NULL );
		hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE) WaitThread, hWait, 0, &ThreadId );

		// connect and get the IWbemServices pointer
		hr = pIWbemLocator->ConnectServer( pNamespace,
		                                   NULL,
		                                   NULL,
		                                   0L,
		                                   0L,
		                                   NULL,
		                                   NULL,
		                                   &pIWbemServices );

		// kill wait feedback thread
		SetEvent( hWait );
		// make sure thread is gone
		WaitForSingleObject( hThread, INFINITE );
		CloseHandle( hWait );
		SysFreeString( pNamespace );

		// See what WBEM/WMI says about DiskPerf
		if ( hr == WBEM_NO_ERROR )
		{	
			printf( "\nConnected with %s\n", szSysName );

			// done with locator object
			pIWbemLocator->Release( ); 

			
			// go enumerate diskperf instances...
			DiskPerfDetails( pIWbemServices );
			pIWbemServices->Release( );
		}
		else
		{	
			printf( "\nFailed to connect with %s, Error: 0x%8lX\n", szSysName, hr );
		}
	}
	else
	{	
		printf( "WMI services not present or unavailable!\n" );
	}

	CoUninitialize( );

	return 0;
}


//==============================================================================
//	WaitThread( HANDLE hWait )
//==============================================================================
DWORD WINAPI WaitThread( HANDLE hWait )
{
	while ( WaitForSingleObject( hWait, 300 ) == WAIT_TIMEOUT )
	{
		printf(".");
	}
	printf("\n");

	return 0;
}


//==============================================================================
//	InitSecurity( void )
//	Initialize COM security for DCOM services.
//==============================================================================
BOOL InitSecurity( void )
{
	// Adjust the security to allow client impersonation.
	HRESULT hres = CoInitializeSecurity
								( NULL, -1, NULL, NULL, 
								RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
								RPC_C_IMP_LEVEL_IMPERSONATE, 
								NULL, 
								EOAC_SECURE_REFS, 
								NULL );

	return SUCCEEDED( hres );
}


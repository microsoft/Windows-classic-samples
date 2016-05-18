/* Copyright (c) 1997-2002  Microsoft Corporation

	Module Name:

		WorkerThread.c

	Abstract:

		IIS maintains a pool of threads to handle incoming HTTP requests.  When all of
		these threads are in use, new requests will be rejected.  If all the pool threads
		are in a wait state (for instance, running ISAPI dlls that are waiting for a query
		on a remote database to complete), IIS may reject incoming requests even if there
		is plenty of CPU power to handle them.

		One way of avoiding this situation is to offload processing of these types of
		requests to a worker thread, releasing the IIS thread back to the pool so that it
		can be used for another request.  This basic sample demonstrates how to implement
		this in an ISAPI dll.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <httpext.h>
#include <stdio.h>

/* global variables */

DWORD g_dwThreadCount = 0;

/* local functions prototypes */

DWORD WINAPI WorkerFunction(LPVOID); 

BOOL SendHttpHeaders(EXTENSION_CONTROL_BLOCK *, LPCSTR , LPCSTR, BOOL);

/*
	Purpose:

		This is required ISAPI Extension DLL entry point.

	Arguments:

		pVer - poins to extension version info structure 

	Returns:

		always returns TRUE
*/

BOOL WINAPI GetExtensionVersion(OUT HSE_VERSION_INFO *pVer)
{
	/* tell the server our version number and extension description */

	pVer->dwExtensionVersion = MAKELONG(HSE_VERSION_MINOR, HSE_VERSION_MAJOR);

	strncpy_s(pVer->lpszExtensionDesc, sizeof(pVer->lpszExtensionDesc), "ISAPI Worker Thread Extension Sample", HSE_MAX_EXT_DLL_NAME_LEN - 1);

	return TRUE;
}

/*
	Purpose:

		Create a thread to handle extended processing. It will be passed 
		the address of a function ("WorkerFunction") to run, and the address 
		of the ECB associated with this session.

	Arguments:

		pECB - pointer to the extenstion control block 

	Returns:

		HSE_STATUS_PENDING to mark this request as pending
*/

DWORD WINAPI HttpExtensionProc(IN EXTENSION_CONTROL_BLOCK *pECB)
{
	DWORD dwThreadID;

	/* NOTE: in production environment you'd probably want to limit the number of threads created */

	CreateThread(	NULL,              // Pointer to thread security attributes 
								0,                 // Initial thread stack size, in bytes 
								WorkerFunction,    // Pointer to thread function 
								pECB,              // The ECB is the argument for the new thread
								0,                 // Creation flags 
								&dwThreadID);      // Pointer to returned thread identifier 

	/* update global thread count */

	InterlockedIncrement(&g_dwThreadCount);

	/* Return HSE_STATUS_PENDING to release IIS pool thread without losing connection */

	return HSE_STATUS_PENDING;
}

/*
	Purpose:

		Show how to perform extended processing in ISAPI DLL without tying IIS threads. 

	Arguments:

		vECB - points to current extension control block

	Returns:

		returns 0 
*/

DWORD WINAPI WorkerFunction(IN LPVOID vECB)
{
	char szHeader[] = "Content-type: text/html\r\n\r\n";
	char szContent[] = "<html> <form method=get action=WorkerThread.dll><h1>Worker Thread Sample</h1><hr>"
										 "<input type=submit value=\"Send Request\"> </form></html>";

	/* Initialize local ECB pointer to void pointer passed to thread */

	EXTENSION_CONTROL_BLOCK *pECB = vECB;

	DWORD dwSize = strlen(szContent);

	/* Send outgoing header */

	SendHttpHeaders(pECB, "200 OK", szHeader, FALSE);

	/* Simulate extended processing for 5 seconds */

	Sleep(5000);

	/* Send content */

	pECB->WriteClient(pECB->ConnID, szContent, &dwSize, 0);

	/* Inform server that the request has been satisfied, and the connection may now be dropped */

	pECB->ServerSupportFunction(pECB->ConnID, HSE_REQ_DONE_WITH_SESSION, NULL, NULL, NULL);

	/* update global thread count */
	
	InterlockedDecrement(&g_dwThreadCount);

	return 0;
}

/*
	Purpose:

		Send specified HTTP status string and any additional header strings
		using new ServerSupportFunction() request HSE_SEND_HEADER_EX_INFO

	Arguments:

		pECB - pointer to the extension control block
		pszStatus - HTTP status string (e.g. "200 OK")
		pszHeaders - any additional headers, separated by CRLFs and 
								terminated by empty line

	Returns:

		TRUE if headers were successfully sent
		FALSE otherwise
*/

BOOL SendHttpHeaders(EXTENSION_CONTROL_BLOCK *pECB, LPCSTR pszStatus, LPCSTR pszHeaders, BOOL fKeepConnection)
{
	HSE_SEND_HEADER_EX_INFO header_ex_info;

	BOOL success;

	header_ex_info.pszStatus = pszStatus;
	header_ex_info.pszHeader = pszHeaders;
	header_ex_info.cchStatus = strlen(pszStatus);
	header_ex_info.cchHeader = strlen(pszHeaders);
	header_ex_info.fKeepConn = fKeepConnection;

	success = pECB->ServerSupportFunction(pECB->ConnID,	HSE_REQ_SEND_RESPONSE_HEADER_EX, &header_ex_info,	NULL,	NULL);

	return success;
}

/*
	Routine Description:

		This function is called when the WWW service is shutdown.

	Arguments:

		dwFlags - HSE_TERM_ADVISORY_UNLOAD or HSE_TERM_MUST_UNLOAD

	Return Value:

		TRUE when extension is ready to be unloaded,
*/

BOOL WINAPI TerminateExtension(IN DWORD dwFlags)
{
	/* Our threads are deterministic and will complete not much later than 5 seconds from now */

	/* wait for all threads to terminate, sleeping for 1 sec */

	while (g_dwThreadCount > 0)
		SleepEx(1000, FALSE);
	
	/* make sure the last thread indeed exited */

	SleepEx(1000, FALSE);

	return TRUE;
}




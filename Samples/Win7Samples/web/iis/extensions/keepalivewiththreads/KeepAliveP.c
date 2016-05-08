/* Copyright (c) 1997-2002 Microsoft Corporation

	Module Name: 
	
		KeepAliveP.c

	Abstract:

		Sample ISAPI Extension demonstrating Keep-Alive with a thread pool.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <httpext.h>
#include <stdio.h>
#include "threadpool.h"

/*
	Purpose:

		Initialize the thread pool when the DLL is loaded by IIS.

	Arguments:

		hinstDLL - DLL instance handle
		fdwReason - notification code
		lpdvContext - reserved

	Returns:

		TRUE if notification was successfully processed by the DLL
		FALSE to indicate a failure
*/

BOOL WINAPI DllMain(IN HINSTANCE hinstDll, IN DWORD fdwReason, IN LPVOID lpvContext) 
{
	BOOL fReturn = TRUE;

	switch (fdwReason) {

		case DLL_PROCESS_ATTACH :

			fReturn = InitThreadPool();

			break;
	}

	return fReturn;
}

/*
	Description:

		This is required ISAPI Extension DLL entry point.

	Arguments:

		pVer - poins to extension version info structure 

	Returns:

		always returns TRUE
*/

BOOL WINAPI GetExtensionVersion(OUT HSE_VERSION_INFO *pVer)
{
	pVer->dwExtensionVersion = MAKELONG(HSE_VERSION_MINOR, HSE_VERSION_MAJOR);

	strncpy_s(pVer->lpszExtensionDesc, sizeof(pVer->lpszExtensionDesc), "ISAPI Keep-Alive with Thread Pool Extension Sample", HSE_MAX_EXT_DLL_NAME_LEN);

	return TRUE;
}

/*
	Description:

		Demonstrate usage of persistent connections serviced by a thread pool.

	Arguments:

		pECB - points to the extension control block

	Returns:

		HSE_STATUS_PENDING if request was successfully queued 
		HSE_SUCCESS_AND_KEEP_CONN if request was served immediately
			(presumably because the queue was full)
*/

DWORD WINAPI HttpExtensionProc(IN EXTENSION_CONTROL_BLOCK *pECB)
{
	DWORD dwSize;
	HSE_SEND_HEADER_EX_INFO HeaderExInfo;

	char szHeader[] = 
		"Connection: Keep-Alive\r\n"
		"Content-Length: %lu\r\n"
		"Content-type: text/html\r\n\r\n";

	char szBusyMessage[] = 
		"<html> <form method=get action=KeepAliveP.dll> <input type=submit> "
		"<br>pECB->ConnID=%lu  <br>Server was too busy. </form></html>";

	char szBuffer[4096];
	char szBuffer2[4096];

	EnterCriticalSection(&csQueueLock);

	if (!AddWorkQueueEntry(pECB)) {

		/* if ECB could not be assigned */
		
		LeaveCriticalSection(&csQueueLock);

		sprintf_s(szBuffer2, sizeof(szBuffer2), szBusyMessage, pECB->ConnID);

		/* Send outgoing header */

		sprintf_s(szBuffer, sizeof(szBuffer), szHeader, strlen(szBuffer2));
		HeaderExInfo.pszHeader = szBuffer;
		HeaderExInfo.cchHeader = strlen( szBuffer );
		HeaderExInfo.pszStatus = "200 OK";
		HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
		HeaderExInfo.fKeepConn = TRUE;

		pECB->ServerSupportFunction(pECB->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX, &HeaderExInfo, NULL, NULL);

		/* Send content */

		dwSize = strlen(szBuffer2);

		pECB->WriteClient(pECB->ConnID, szBuffer2, &dwSize, 0);

		return HSE_STATUS_SUCCESS_AND_KEEP_CONN;

	} else {

		/* release 1 thread from the pool */

		ReleaseSemaphore(hWorkSem, 1, NULL);

		LeaveCriticalSection(&csQueueLock);
	}

	return HSE_STATUS_PENDING;
}

/*
	Description:

		This function is called when the WWW service is shutdown

	Arguments:

		dwFlags - HSE_TERM_ADVISORY_UNLOAD or HSE_TERM_MUST_UNLOAD

	Return Value:

		TRUE if extension is ready to be unloaded,
		FALSE otherwise
*/

BOOL WINAPI TerminateExtension(IN DWORD dwFlags)
{
	return TRUE;
}

/*
	Description:

		Worker thread function - simulates extended processing of the HTTP request

	Arguments:

		pvThreadNum - thread number

	Returns:

		alsways returns 0
*/

DWORD WINAPI WorkerFunction(IN LPVOID pvThreadNum)
{
	EXTENSION_CONTROL_BLOCK *pECB;
	DWORD dwRet, dwState, dwSize;
	HSE_SEND_HEADER_EX_INFO HeaderExInfo;

	DWORD dwThreadNum = (DWORD)pvThreadNum;

	/* This header will be filled in with the content length */

	char szHeader[] = 
		"Connection: Keep-Alive\r\nContent-Length: %lu\r\n"
		"Content-type: text/html\r\n\r\n";

	char szContent[] = 
		"<html> <form method=get action=KeepAliveP.dll><input type=submit> " 
		"<br>pECB->ConnID=%lu  <br>dwThreadNum=%lu</form></html>";

	char szBuffer[4096];
	char szBuffer2[4096];

	for (;;) {

		dwRet = WaitForSingleObject(hWorkSem, INFINITE);

		if (dwRet == WAIT_OBJECT_0) {

			EnterCriticalSection(&csQueueLock);

			if (GetWorkQueueEntry(&pECB)) {	
	              
				/* Found work to do */

				LeaveCriticalSection(&csQueueLock);
				sprintf_s(szBuffer2, sizeof(szBuffer2), szContent, pECB->ConnID, dwThreadNum);

				/* Send outgoing header */

				sprintf_s(szBuffer, sizeof(szBuffer), szHeader, strlen(szBuffer2));

				HeaderExInfo.pszHeader = szBuffer;
				HeaderExInfo.cchHeader = strlen( szBuffer );
				HeaderExInfo.pszStatus = "200 OK";
				HeaderExInfo.cchStatus = strlen( HeaderExInfo.pszStatus );
				HeaderExInfo.fKeepConn = TRUE;

				pECB->ServerSupportFunction(pECB->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX, &HeaderExInfo, NULL, NULL);

				/* Simulate extended processing */
				
				Sleep(3000);

				/* Send content */

				dwSize = strlen(szBuffer2);

				pECB->WriteClient(pECB->ConnID, szBuffer2, &dwSize, 0);

				/* Tell IIS to keep the connection open */

				dwState = HSE_STATUS_SUCCESS_AND_KEEP_CONN;

				pECB->ServerSupportFunction(pECB->ConnID, HSE_REQ_DONE_WITH_SESSION, &dwState, NULL, 0);

			} else {			

				/* No item found is unexpected condition - exit thread */

				LeaveCriticalSection(&csQueueLock);

				ExitThread(0);
			}

		} else {

			/* Leave the infinite loop */
		
			break;
		}
	}

	return 0;
}

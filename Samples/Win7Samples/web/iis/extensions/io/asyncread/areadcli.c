/* Copyright (c) 1997-2002 Microsoft Corporation

Module Name:

    AReadCli.c

Abstract:

    This module demonstrates using asynchronous ReadClient call
    to read data sent from client. On return, this sends the 
    followings back to client:
    
    1) the number of bytes intended to send
    2) the number of bytes actually read.
    3) data content 
*/

#include <windows.h>
#include <stdio.h>
#include <httpext.h>

#define MAX_BUF_SIZE        (49152)                 /* Max number of bytes for each read - 48K */
#define MEM_ALLOC_THRESHOLD (1024 * 1024)           /* Default heap size is 1M */

typedef struct _IO_WORK_ITEM {

    PBYTE                    pbDATAFromClient;   /* Grand read data holder */
    DWORD                    cbReadSoFar;        /* Number of bytes read so far */
                                                 /* and is used as index for pbDATAFromClient */
    EXTENSION_CONTROL_BLOCK *pecb;

}  IO_WORK_ITEM, * PIOWI;

/* Sample Form for doing a POST method */

static CHAR             g_szPostForm[] =
                        "<h2>Asychronous ReadClient Sample Post Form</h2><p>\r\n\r\n"
                        "This demonstrates a post request being sent by this form to the sample ISAPI - AReadCli.dll<p>\r\n\r\n"
                        "AReadCli.dll reads data posted by this form and send it back to the browser.<p>\r\n"
                        "<h3>Post Form</h3>\r\n"
                        "Please enter data below:<br>\r\n"
                        "<form method=POST action=\"areadcli.dll\">\r\n"
                        "<textarea name=\"Data\" cols=48 rows=4></textarea>\r\n\r\n"
                        "<input type=submit> <input type=reset>\r\n"
                        "</form>";

/* Report read data */

static CHAR             g_szReport[] = 
                        "Bytes count including \"Data=\"  \r\n"
                        "ECB Total Bytes:    %d.\r\n"
                        "Actual Read Bytes:  %d.\r\n";
                
DWORD DoInit(IN OUT PIOWI piowi);
VOID DoCleanUp(IN PIOWI piowi);
DWORD DoAsyncReadClient(IN PIOWI piowi);
VOID WINAPI AsyncReadClientIoCompletion(IN LPEXTENSION_CONTROL_BLOCK pecb, IN PVOID pContext, IN DWORD cbIO, IN DWORD dwError);
DWORD SendMSGToClient(IN LPEXTENSION_CONTROL_BLOCK pecb, IN LPCSTR pszErrorMsg);
LPVOID AR_Allocate(IN LPEXTENSION_CONTROL_BLOCK pecb, IN DWORD dwSize);
BOOL AR_Free(IN LPEXTENSION_CONTROL_BLOCK pecb, IN LPVOID pvData);

/*
	Description:

		This function DllLibMain() is the main initialization function for
		this DLL. It initializes local variables and prepares it to be invoked
		subsequently.

	Arguments:

		hinstDll          Instance Handle of the DLL
		fdwReason         Reason why NT called this DLL
		lpvReserved       Reserved parameter for future use.

	Return Value:

		fReturns TRUE if successful; otherwise FALSE is fReturned.
*/

__stdcall DllMain(IN HINSTANCE hinstDll, IN DWORD fdwReason, IN LPVOID lpvContext OPTIONAL)
{
	BOOL fReturn = TRUE;

	switch (fdwReason) {

		case DLL_PROCESS_ATTACH :
			
				OutputDebugString("Initializing the global data for areadcli.dll\n");

				/* Prevent the system from calling DllMain when threads are created or destroyed. */
				
				DisableThreadLibraryCalls(hinstDll);

				/* Initialize various data and modules. */
				
				break;

		case DLL_PROCESS_DETACH :

				if (lpvContext != NULL) 
				{ 
				}

				break;

		default:

			break;
	} 

	return fReturn;
} 

/*
	Description:

		Sets the ISAPI extension version information.

	Arguments:

		Version     pointer to HSE_VERSION_INFO structure

	Return Value:

		TRUE
*/

BOOL WINAPI GetExtensionVersion(HSE_VERSION_INFO *pVer)
{
	pVer->dwExtensionVersion = MAKELONG(HSE_VERSION_MINOR, HSE_VERSION_MAJOR);

	strncpy_s((LPSTR) pVer->lpszExtensionDesc, sizeof(pVer->lpszExtensionDesc), "Asynchronous Read Client Sample ISAPI DLL", HSE_MAX_EXT_DLL_NAME_LEN);

	return TRUE;
}

/*
	Description:

		This is the main routine for any ISAPI application. Inside DoASyncReadClient,
		proper action will be performed to read data from client asynchronously. Any data 
		read from client will be sent back to the client by using synchronous WriteClient.

	Arguments:

		pecb        pointer to ECB containing parameters related to the request.

	Return Value:

		HSE_STATUS_SUCCESS
		HSE_STATUS_PENDING
		HSE_STATUS_ERROR
*/

DWORD WINAPI HttpExtensionProc(LPEXTENSION_CONTROL_BLOCK pecb)
{
	PIOWI   piowi;
	DWORD   hseStatus = HSE_STATUS_SUCCESS;

	/*
		The string length of textarea name "Data=" is 5. 
		Available bytes <= 5 indicates that no user-
		entered data has been sent, and the post form 
		is shown.
	*/

	if (pecb->cbAvailable <= 5) {  

			hseStatus = SendMSGToClient(pecb, g_szPostForm);

	} else {

		piowi  = (PIOWI)LocalAlloc(LMEM_FIXED, sizeof(IO_WORK_ITEM));

		if (NULL == piowi) {

			SetLastError(ERROR_NOT_ENOUGH_MEMORY);

			return HSE_STATUS_ERROR;
		}

		piowi->pecb = pecb;

		/*
			Init Grand data holder, assign the first chunk(read-ahead chunk)
			and update the index (cbRreadSoFar) of the grand data holder.
		*/
		
		hseStatus = DoInit(piowi);
	  
		if (HSE_STATUS_ERROR != hseStatus) {

			/* Now we are ready to do asynchronous readclient here */

			hseStatus = DoAsyncReadClient(piowi);
	
			if (hseStatus != HSE_STATUS_PENDING) {

				/* When IO finishes, tell IIS we will end the session. Also clean up other resources here */

				DoCleanUp(piowi);
			}
		}
	}
	    
	return hseStatus;
}

/*
	Description:

		This function is called when the WWW service is shutdown

	Arguments:

		dwFlags - HSE_TERM_ADVISORY_UNLOAD or HSE_TERM_MUST_UNLOAD

	Return Value:

		TRUE
*/
#pragma warning( disable : 4100 )

BOOL WINAPI TerminateExtension(DWORD dwFlags)
{
	return TRUE;
}

/*
	Description:

		Prepare header, SendHeaderExInfo struct and write whatever
		message is intended to send to the client.

	Arguments:

		pecb        - pointer to ECB containing parameters related to the request.
		pszMsg      - pointer to the body of the message that is sent to the content.

	Return Value:

		HSE_STATUS_SUCCESS 
		HSE_STATUS_ERROR
*/

DWORD SendMSGToClient(IN LPEXTENSION_CONTROL_BLOCK  pecb, IN LPCSTR pszMsg)
{
	HSE_SEND_HEADER_EX_INFO	SHEI;

	BOOL    fReturn;
	DWORD   cbText;
	DWORD   hseStatus = HSE_STATUS_SUCCESS;
	CHAR    *pszText = NULL;

	CHAR    szStatus[] = "200 OK";
	CHAR    szHeaderBase[] = "Content-type: text/html\r\n\r\n";


	/*
		Populate SendHeaderExInfo struct
	
		NOTE we must send Content-Length header with correct 
		byte count in order for keep-alive to work.
	*/

	SHEI.pszStatus = szStatus;
	SHEI.pszHeader = szHeaderBase;
	SHEI.cchStatus = lstrlen(szStatus);
	SHEI.cchHeader = lstrlen(szHeaderBase);
	SHEI.fKeepConn = FALSE;

	/* Build page */

	cbText = strlen("<head><title>Simple Async Read Client Sample</title></head>\n<body></body>\n") + strlen(pszMsg) + 1;

	pszText = (PCHAR)AR_Allocate(pecb, cbText);

	/* Verify allocation */

	if (NULL == pszText) {

		SetLastError(ERROR_NOT_ENOUGH_MEMORY);

		return HSE_STATUS_ERROR;
	}

	
	strcpy_s(pszText, sizeof(pszText), "<head><title>Simple Async Read Client Sample</title></head>\n");
	strcat_s(pszText, sizeof(pszText), "<body>");
	strcat_s(pszText, sizeof(pszText), pszMsg);
	strcat_s(pszText, sizeof(pszText), "</body>\n");

	cbText = (DWORD)strlen(pszText); 
	    
	/* Send header and body text to client */

	fReturn = pecb->ServerSupportFunction(pecb->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX, &SHEI, NULL, NULL);

	fReturn &= pecb->WriteClient(pecb->ConnID, pszText, &cbText, 0);

	if (!fReturn)
		hseStatus = HSE_STATUS_ERROR;

	AR_Free(pecb, pszText);

	return hseStatus;
} 

/*
	Description:

		The caller of the asynchrnous read client.

	Arguments:

		piowi       pointer to the work item

	Return Value:

		HSE_STATUS_SUCCESS
		HSE_STATUS_PENDING
		HSE_STATUS_ERROR
*/

DWORD DoAsyncReadClient(IN PIOWI piowi)
{   
	BOOL    fReturn;
	CHAR    szTmp[MAX_BUF_SIZE];
	DWORD   dwFlags;
	DWORD   cbTotalToRead = MAX_BUF_SIZE;
	DWORD   hseStatus =  HSE_STATUS_PENDING;
	BYTE    *pbData = NULL;

	/*
		Check if cbTotalBytes == cbAvailable if so lpbData contains all the data sent by 
		the client, and complete the session. 
	*/

	if (piowi->pecb->cbTotalBytes == piowi->pecb->cbAvailable) {
	    
		/* Construct the report and write it to client */

		pbData = (PBYTE)AR_Allocate(piowi->pecb, piowi->pecb->cbAvailable + MAX_BUF_SIZE);
	      
		if (NULL == pbData) {

			SetLastError(ERROR_NOT_ENOUGH_MEMORY);

			return HSE_STATUS_ERROR;
		}

		sprintf_s(pbData, sizeof(pbData), g_szReport, piowi->pecb->cbTotalBytes, piowi->pecb->cbAvailable);
	              
		strcat_s(pbData, sizeof(pbData), piowi->pecb->lpbData);  

		hseStatus = SendMSGToClient(piowi->pecb, pbData);

		AR_Free(piowi->pecb, pbData);

		DoCleanUp(piowi);
	  
		return hseStatus; // HSE_STATUS_SUCCESS or HSE_STATUS_ERROR;
	}

	/*
		There is more to read. Set a call back function and context that will 
		be used for handling asynchrnous IO operations. This only needs to set up once.
	*/

	fReturn =	piowi->pecb->ServerSupportFunction(piowi->pecb->ConnID, HSE_REQ_IO_COMPLETION, AsyncReadClientIoCompletion, 0, (LPDWORD)piowi);  

	if (!fReturn) {

		sprintf_s (szTmp, sizeof(szTmp), "Problem occurred at ServerSupportFunction() sending HSE_REQ_IO_COMPLETION request.");

		SendMSGToClient(piowi->pecb, szTmp);
	    
		return HSE_STATUS_ERROR;
	}

	/*
		Fire off the call to perform an asynchronus read from the client. 
	
		We need to first check if the size of the remaining chunk 
		is less than MAX_BUF_SIZE, if so just read what is available, 
		otherwise read MAX_BUF_SIZE bytes of data.
	*/ 

	cbTotalToRead = piowi->pecb->cbTotalBytes - piowi->cbReadSoFar;

	if (cbTotalToRead > MAX_BUF_SIZE)
		cbTotalToRead = MAX_BUF_SIZE;

	dwFlags = HSE_IO_ASYNC;

	/* append the new chunk to buffer, cbReadSoFar indexes the byte right after the last written byte in the buffer */

	fReturn = piowi->pecb->ServerSupportFunction(piowi->pecb->ConnID, HSE_REQ_ASYNC_READ_CLIENT, piowi->pbDATAFromClient +	piowi->cbReadSoFar, &cbTotalToRead, &dwFlags);

	if (!fReturn) {
	    
		sprintf_s(szTmp, sizeof(szTmp), "Problem occurred at ServerSupportFunction() sending HSE_REQ_ASYNC_READ_CLIENT request.");

		SendMSGToClient(piowi->pecb, szTmp);
	    
		hseStatus = HSE_STATUS_ERROR;
	}

	return hseStatus;
}

/*
	Description:

		This is the callback function for handling completions of asynchronous ReadClient.
		This function resubmits additional IO to read the next chunk of data from the 
		client. If there is no more data to read or problem during operation, this function 
		will inform IIS that it is about to end the request session.

	Arguments:

		pecb        - extension control block
		pContext    - this is a IO_WORK_ITEM
		cbIO        - bytes just read
		dwError     - Win32 error status code

	fReturn Value:

		None
*/

VOID WINAPI AsyncReadClientIoCompletion(IN LPEXTENSION_CONTROL_BLOCK pECB, IN PVOID pContext, IN DWORD cbIO, IN DWORD dwError)
{
	BOOL fReturn;
	CHAR szTmp[MAX_BUF_SIZE];
	DWORD dwFlags;
	DWORD cbTotalToRead;
	BYTE *pbData = NULL;
	PIOWI piowi = (PIOWI)pContext;
	EXTENSION_CONTROL_BLOCK *pecb = piowi->pecb;

	if (ERROR_SUCCESS == dwError) {

		/* Read successfully, so update current total bytes read (aka index of grand data holder) */

		piowi->cbReadSoFar += cbIO;     
	                          
		/* If they are equal, we finish reading all bytes from client */
	  
		if (piowi->cbReadSoFar == pecb->cbTotalBytes) { 

			/* Construct the report and write it to client */

			pbData = (PBYTE)AR_Allocate(pecb, piowi->cbReadSoFar + MAX_BUF_SIZE);
	        
			if (NULL == pbData) {

				SetLastError(ERROR_NOT_ENOUGH_MEMORY);

				sprintf_s(szTmp, sizeof(szTmp), "Failed to allocate memory inside AsyncReadClientIoCompletion().");

				SendMSGToClient(pecb, szTmp);
	      
				DoCleanUp(piowi);

				return;
			}
	    
			sprintf_s(pbData, sizeof(pbData), g_szReport, pecb->cbTotalBytes, piowi->cbReadSoFar);

			piowi->pbDATAFromClient[piowi->cbReadSoFar] = 0; 

			strcat_s(pbData, sizeof(pbData), piowi->pbDATAFromClient);

			SendMSGToClient(pecb, pbData);
	    
			AR_Free(piowi->pecb, pbData);

			DoCleanUp(piowi);
	      
		} else {

			/*
				Still have more data to read...
			 
				We need to first check if the size of the remaining chunk 
				is less than MAX_BUF_SIZE, if so just read what is available, 
				otherwise read MAX_BUF_SIZE bytes of data.
			*/ 

			cbTotalToRead = pecb->cbTotalBytes - piowi->cbReadSoFar;

			if (cbTotalToRead > MAX_BUF_SIZE)
				cbTotalToRead = MAX_BUF_SIZE;
	    
			/* Fire off another call to perform an asynchronus read from the client. */

			dwFlags = HSE_IO_ASYNC;

			/* append the new chunk to buffer, cbReadSoFar indexes the byte right after the last written byte in the buffer */

			fReturn = pecb->ServerSupportFunction(pecb->ConnID, HSE_REQ_ASYNC_READ_CLIENT, piowi->pbDATAFromClient + piowi->cbReadSoFar, &cbTotalToRead, &dwFlags);

			if (!fReturn) { 

				sprintf_s(szTmp,  sizeof(szTmp), "Problem occurred at ServerSupportFunction() sending HSE_REQ_ASYNC_READ_CLIENT request.");

				SendMSGToClient(pecb, szTmp);

				DoCleanUp(piowi);
			}
		}

	} else {

		/* Error on read */

		SetLastError(dwError);
	  
		DoCleanUp(piowi);
	}
}

/*
	Description:

		Initialize the  Grand data holder, assign the first chunk(read-ahead chunk)
		and update the index (cbRreadSoFar) of the grand data holder.

	Arguments:

		piowi       pointer to the work item

	fReturn Value:

		HSE_STATUS_SUCCESS or HSE_STATUS_ERROR
*/

DWORD DoInit(IN OUT PIOWI piowi)
{
	piowi->pbDATAFromClient = (PBYTE)AR_Allocate(piowi->pecb, piowi->pecb->cbTotalBytes + MAX_BUF_SIZE);

	if (NULL == piowi->pbDATAFromClient) {

		SetLastError(ERROR_NOT_ENOUGH_MEMORY);

		return HSE_STATUS_ERROR;
	}

	/* The first chunk (read-ahead chunk) has arrived. */

	strcpy_s(piowi->pbDATAFromClient, strlen(piowi->pbDATAFromClient), piowi->pecb->lpbData);

	piowi->cbReadSoFar = piowi->pecb->cbAvailable;

	return HSE_STATUS_SUCCESS;
}

/*
	Description:

		End the session with IIS and clean up other previous allocated resources.

	Arguments:

		piowi       pointer to the work item

	Return Value:

		None
*/

VOID DoCleanUp(IN PIOWI piowi)
{
	if (piowi->pbDATAFromClient != NULL)
		AR_Free( piowi->pecb, piowi->pbDATAFromClient);

	piowi->pecb->ServerSupportFunction(piowi->pecb->ConnID, HSE_REQ_DONE_WITH_SESSION, NULL, NULL, NULL);

	LocalFree(piowi);

	return;
}

/*
	Description:

		Memory allocation routine. Two different Win32 API's to allocate
		bytes in memory, which is based on the number of bytes coming from 
		the client. If the size is greater than 1 M bytes VirtualAllocate is 
		used, otherwise HeapAllocate is used.

	Arguments:

		pecb        - pointer to ECB containing parameters related to the request.
		dwSize      - number of bytes to allocate

	Return Value:

		Pointer to void
*/

LPVOID AR_Allocate(IN LPEXTENSION_CONTROL_BLOCK pecb, IN DWORD dwSize)
{
	LPVOID pvData = NULL;

	if (pecb->cbTotalBytes > MEM_ALLOC_THRESHOLD)
		pvData = VirtualAlloc(NULL, dwSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	 else
		pvData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);

	return pvData;
}

/*
	Description:

		Freeing memory routine, a complementary routine to AR_Allocate. 
		Two different Win32 API's will be used to free up bytes in memory,
		which is based on the number of bytes coming from the client. If 
		the size is greater than 1 M bytes VirtualFree is used. Otherwise, 
		HeapFree is used.

	Arguments:

		pecb        - pointer to ECB containing parameters related to the request.
		pvData      - pointer to the data to be freed.

	Return Value:

		TRUE or FALSE
*/

BOOL AR_Free( IN LPEXTENSION_CONTROL_BLOCK pecb, IN LPVOID pvData)
{
	BOOL fReturn = FALSE;

	if (pecb->cbTotalBytes > MEM_ALLOC_THRESHOLD)
		fReturn = VirtualFree(pvData, 0, MEM_RELEASE);
	else
		fReturn = HeapFree(GetProcessHeap(), 0, pvData);

	return fReturn;
}

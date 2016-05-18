/* Copyright (c) 1995-2002  Microsoft Corporation

Module Name:

upcase.c

Abstract:

This filter converts HTML response data to upper case if requested
by the client. The request is made by specifying an extra 
subdirectory in the URL, which doesnt actually exist on the server,
but is used by the filter to indicate that upper case is desired.
The extra subdirectory is removed from the request before the HTTP
server sees the request.

When a request is received, the filter inspects the 
subdirectories specified in the URL. If a subdirectory is 'UC'
or 'uc', the subdirectory is removed from the request, and the
filter saves in its pFilterContext field, a value indicating that
the response data should be converted to upper case.

When the filter entrypoint is later called for the response, it 
checks the pFilterContext field and converts the data if 
the mime-type of the response indicates that its an html file.
This avoids conversions on binary data.

An example URL entered by a user might be:

http://www.example.com/sales/uc/projections.htm

While the functionality of this filter is somewhat contrived, this
filter does a good job of demonstrating the following features of
filters:
- parsing/modifying HTTP headers (in the request)
- modifying data following HTTP headers (in the response)
- saving state to be used by the filter later
- adding request/response level functionality to the server
(instead of using a mechanism like this to convert to 
uppercase, you may use it to do on-the-fly customized
translations of HTML pages (English -> French perhaps?)
*/      

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <httpfilt.h>

BOOL WINAPI __stdcall GetFilterVersion(HTTP_FILTER_VERSION *pVer)
{
	/* Specify the types and order of notification */

	pVer->dwFlags = (SF_NOTIFY_NONSECURE_PORT | SF_NOTIFY_URL_MAP |	SF_NOTIFY_SEND_RAW_DATA | SF_NOTIFY_ORDER_DEFAULT);

	pVer->dwFilterVersion = HTTP_FILTER_REVISION;

	strcpy_s(pVer->lpszFilterDesc, sizeof(pVer->lpszFilterDesc), "Upper case conversion filter, Version 1.0");

	return TRUE;
}

DWORD WINAPI __stdcall HttpFilterProc(HTTP_FILTER_CONTEXT *pfc, DWORD NotificationType, VOID *pvData)
{
	CHAR *pchIn, *pPhysPath;
	DWORD	cbBuffer,	cbtemp;
	PHTTP_FILTER_URL_MAP pURLMap;
	PHTTP_FILTER_RAW_DATA	pRawData;

	switch (NotificationType)	{

		case SF_NOTIFY_URL_MAP :

			/* Check the URL for a subdirectory in the form of /UC/ or /uc/ */

			pURLMap	=	(PHTTP_FILTER_URL_MAP)pvData;

			pPhysPath	=	pURLMap->pszPhysicalPath;

			pfc->pFilterContext	=	0;

			for ( ; *pPhysPath; pPhysPath++) 
			{
				/* Ensure that there are at least 4 characters (checking for "\UC\") left in the URL before checking */

				if (strlen(pPhysPath) > 3) 
				{
					if (*pPhysPath == '\\' && (*(pPhysPath + 1) == 'u' || *(pPhysPath + 1) == 'U') && (*(pPhysPath + 2) == 'c' || *(pPhysPath + 2) == 'C') && *(pPhysPath + 3) ==	'\\')
					{
						/* Now that we've found it, remove it by collapsing everything down */

						for ( ; *(pPhysPath + 3) ; pPhysPath++)
							*pPhysPath = *(pPhysPath + 3);

						/* NULL terminate the string */

						*pPhysPath = '\0'; 

						/* And set the flag to let the SF_NOTIFY_SEND_RAW_DATA handler know to uppercase the content */

						pfc->pFilterContext	=	(VOID	*)1;

						/* Break us out of the loop - note that this will only find the first instance of /UC/ in the URL */

						break;
					}
				}
			}

			break;

		case SF_NOTIFY_SEND_RAW_DATA :

			if (pfc->pFilterContext)
			{
				pRawData = (PHTTP_FILTER_RAW_DATA)pvData;

				pchIn	=	(BYTE	*)pRawData->pvInData;

				cbBuffer = 0;

				if (pfc->pFilterContext	== (VOID *)1)
				{
					/* 
					As this is the first block, scan through it until 2 CRLFs are seen, to pass
					all of the headers.
					*/

					for ( ; cbBuffer < pRawData->cbInData - 2; cbBuffer++)
					{
						if (pchIn[cbBuffer]	== '\n' && pchIn[cbBuffer + 2]	== '\n')
						{
							cbBuffer += 3;

							break;
						}

						cbBuffer++;
					}

					for (cbtemp = 0; cbtemp < (cbBuffer - 3); cbtemp++) 
					{
						if (pchIn[cbtemp] == '/' && pchIn[cbtemp + 1] == 'h' && pchIn[cbtemp + 2] == 't' && pchIn[cbtemp + 3] == 'm')
						{
							pfc->pFilterContext	=	(VOID	*)2;

							break;
						}
					}

					if (cbtemp ==	cbBuffer)
						pfc->pFilterContext	=	0; /* not an	html file */
				}

				/* Now uppercase everything */

				if (pfc->pFilterContext)
					for ( ; cbBuffer < pRawData->cbInData; cbBuffer++)
						pchIn[cbBuffer] = (pchIn[cbBuffer] >= 'a' && pchIn[cbBuffer] <= 'z') ? (pchIn[cbBuffer] - 'a' + 'A') : pchIn[cbBuffer];
			}

			break;

		default :

			break;				
	}
	
	return SF_STATUS_REQ_NEXT_NOTIFICATION;
}
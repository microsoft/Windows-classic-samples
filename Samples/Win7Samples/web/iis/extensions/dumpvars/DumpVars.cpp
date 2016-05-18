//	Copyright (c) 1997-2002  Microsoft Corporation
//
//	Module Name:
//
//		DumpVars.cpp
//
//	Abstract:
//
//		ISAPI Extension sample to dump server variables

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>
#include <httpext.h>

#define VAR_BUFFER_SIZE 1024	// Buffer size for server variables

//	Description:
//
//		Use WriteClient() function to dump the value of each server variable in the table.
//
//	Arguments:
//
//		pECB - pointer to the extenstion control block 
//
//	Returns:
//
//		HSE_STATUS_SUCCESS

DWORD WINAPI HttpExtensionProc(IN EXTENSION_CONTROL_BLOCK *pECB)
{
	char *aszServerVariables[] = {
		"APPL_MD_PATH", "APPL_PHYSICAL_PATH", "AUTH_PASSWORD",
		"AUTH_TYPE", "AUTH_USER", "CERT_COOKIE", "CERT_FLAGS",
		"CERT_ISSUER", "CERT_KEYSIZE", "CERT_SECRETKEYSIZE",
		"CERT_SERIALNUMBER", "CERT_SERVER_ISSUER",
		"CERT_SERVER_SUBJECT", "CERT_SUBJECT", "CONTENT_LENGTH",
		"CONTENT_TYPE", "HTTP_ACCEPT", "HTTPS", "HTTPS_KEYSIZE",
		"HTTPS_SECRETKEYSIZE", "HTTPS_SERVER_ISSUER",
		"HTTPS_SERVER_SUBJECT", "INSTANCE_ID", "INSTANCE_META_PATH",
		"PATH_INFO", "PATH_TRANSLATED", "QUERY_STRING",
		"REMOTE_ADDR", "REMOTE_HOST", "REMOTE_USER",
		"REQUEST_METHOD", "SCRIPT_NAME", "SERVER_NAME",
		"SERVER_PORT", "SERVER_PORT_SECURE", "SERVER_PROTOCOL",
		"SERVER_SOFTWARE", "URL"};

	DWORD dwBuffSize;
	DWORD dwError;

	HSE_SEND_HEADER_EX_INFO HeaderExInfo;

	// Send headers to the client

	HeaderExInfo.pszStatus = "200 OK";
	HeaderExInfo.pszHeader = "Content-type: text/html\r\n\r\n";
	HeaderExInfo.cchStatus = strlen(HeaderExInfo.pszStatus);
	HeaderExInfo.cchHeader = strlen(HeaderExInfo.pszHeader);
	HeaderExInfo.fKeepConn = FALSE;
	  
	pECB->ServerSupportFunction(pECB->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX, &HeaderExInfo, NULL, NULL);

	// Begin sending back HTML to the client

	char szHeader[] = "<HTML>\r\n<BODY><h1>Server Variable Dump</h1>\r\n<hr>\r\n";

	dwBuffSize = strlen(szHeader);

	pECB->WriteClient(pECB->ConnID, szHeader, &dwBuffSize, 0);

	DWORD dwNumVars = (sizeof aszServerVariables ) / (sizeof aszServerVariables[0]);

	// Get the server variables and send them

	char *szEIP = "ERROR_INVALID_PARAMETER";
	char *szEII = "ERROR_INVALID_INDEX";
	char *szEMD = "ERROR_MORE_DATA";
	char *szEND = "ERROR_NO_DATA";

	for (DWORD x = 0; x < dwNumVars; x++) {

		dwBuffSize = VAR_BUFFER_SIZE;

		char szValue[VAR_BUFFER_SIZE] = "";

		if (!pECB->GetServerVariable(pECB->ConnID, aszServerVariables[x], szValue, &dwBuffSize)) {

			// Analyze the problem and report result to user
			
			switch (dwError = GetLastError( )) {

				case ERROR_INVALID_PARAMETER :

					strcpy_s(szValue, sizeof(szEIP),  szEIP);

					break;

				case ERROR_INVALID_INDEX :

					strcpy_s(szValue, sizeof(szEII),  szEII );

					break;

				case ERROR_INSUFFICIENT_BUFFER :

					sprintf_s(szValue, VAR_BUFFER_SIZE, "ERROR_INSUFFICIENT_BUFFER - %d bytes required.", dwBuffSize);

					break;

				case ERROR_MORE_DATA :

					strcpy_s(szValue, sizeof(szEMD),  szEMD );

					break;

				case ERROR_NO_DATA :

					strcpy_s(szValue, sizeof(szEND),  szEND );

					break;

				default :

					sprintf_s(szValue, VAR_BUFFER_SIZE, "*** Error %d occured retrieving server variable ***", dwError);

					break;
			}
		}

		// Dump server variable name and value. Note that the buffer size for this output
		// is much more than is needed. In a production environment, a dynamically-allocated
		// buffer, properly sized, would make more sense.

		char szOutput[VAR_BUFFER_SIZE * 2];

		sprintf_s(szOutput, VAR_BUFFER_SIZE * 2,"%s: %s<br>\r\n", aszServerVariables[x], szValue);

		dwBuffSize = strlen(szOutput);

		// Send the line to client

		pECB->WriteClient(pECB->ConnID, szOutput, &dwBuffSize, 0);
	}

	// End HTML page

	char szFooter[] = "</BODY>\r\n</HTML>\r\n\r\n";

	dwBuffSize = strlen(szFooter);

	pECB->WriteClient(pECB->ConnID, szFooter, &dwBuffSize, 0);

	return HSE_STATUS_SUCCESS;
}

//	Description:
//
//		This is required ISAPI Extension DLL entry point.
//
//	Arguments:
//
//		pVer - poins to extension version info structure 
//
//	Returns:
//
//		always returns TRUE

BOOL WINAPI GetExtensionVersion(OUT HSE_VERSION_INFO *pVer)
{
	pVer->dwExtensionVersion = MAKELONG(HSE_VERSION_MINOR, HSE_VERSION_MAJOR);

	strncpy_s(pVer->lpszExtensionDesc, sizeof(pVer->lpszExtensionDesc), "DumpVars ISAPI Sample", HSE_MAX_EXT_DLL_NAME_LEN);

	return TRUE;
}

//	Description:
//
//		This function is called when the WWW service is shutdown
//
//	Arguments:
//
//		dwFlags - HSE_TERM_ADVISORY_UNLOAD or HSE_TERM_MUST_UNLOAD
//
//	Returns:
//
//		TRUE if extension is ready to be unloaded, FALSE otherwise

BOOL WINAPI TerminateExtension(IN DWORD dwFlags)
{
	return TRUE;
}


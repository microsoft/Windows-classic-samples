//	Copyright (c) 1997-2002  Microsoft Corporation
//
//	Module Name:
//
//		redirect.cpp
//
//	Abstract:
//
//	Redirect is a sample ISAPI extension to demonstrate redirecting
//	a request.  It redirects requests to a URL specified on the
//	query string.  

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <httpext.h>
#include <stdio.h>

#define BUFF_SIZE 2048

// local prototypes

DWORD SendInstructionPage(EXTENSION_CONTROL_BLOCK *pECB);

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
//		Always returns TRUE

BOOL WINAPI GetExtensionVersion(OUT HSE_VERSION_INFO *pVer)
{
	pVer->dwExtensionVersion = MAKELONG(HSE_VERSION_MINOR, HSE_VERSION_MAJOR);

	strncpy_s(pVer->lpszExtensionDesc, sizeof(pVer->lpszExtensionDesc), "Redirect ISAPI Sample", HSE_MAX_EXT_DLL_NAME_LEN);

	return TRUE;
}

//	Description:
//
//		If no query string is present, or if the query string is not identified as a
//		legal target for redirection, Redirect.dll will return a page to the client
//		with brief instructions for its use.
//
//		Redirections to a resource on the same server as the dll will be handled by
//		IIS and will be transparent to the browser (HSE_REQ_SEND_URL).
//
//		Redirections to a resource on a different server will result in an HTTP 302
//		response instructing the browser to obtain the resource from another location
//		(HSE_REQ_SEND_URL_REDIRECT_RESP).
//
//	Arguments:
//
//		pECB - pointer to the extenstion control block 
//
//	Returns:
//
//		HSE_STATUS_SUCCESS on successful completion
//		HSE_STATUS_ERROR on failure

DWORD WINAPI HttpExtensionProc(IN EXTENSION_CONTROL_BLOCK *pECB)
{
	// If no query string is present, return the instruction page

	if (!strlen(pECB->lpszQueryString))
		return SendInstructionPage(pECB);
	
	// Check to see if the redirect URL is on another server. If it is, use
	// HSE_REQ_SEND_URL_REDIRECT_RESP.  If it's on this local server, use
	// HSE_REQ_SEND_URL to return the specified URL without using an HTTP 302
	// status code.
	
	DWORD dwBuffSize = strlen(pECB->lpszQueryString);

	if (!_strnicmp(pECB->lpszQueryString, "http://", 7)) {

		pECB->ServerSupportFunction(pECB->ConnID, HSE_REQ_SEND_URL_REDIRECT_RESP, pECB->lpszQueryString, &dwBuffSize, NULL);

	} else {

		// Check to make sure that query string begins with a '/'.
		
		if (*(pECB->lpszQueryString) != '/')
			return SendInstructionPage( pECB );
	
		pECB->ServerSupportFunction(pECB->ConnID, HSE_REQ_SEND_URL, pECB->lpszQueryString, &dwBuffSize, NULL);
	}

	return HSE_STATUS_SUCCESS;
}

//	Description:
//
//		Send short http usage description to the user.
//
//	Arguments:
//
//		pECB - pointer to the extenstion control block 
//    
//	Returns:
//
//		HSE_STATUS_SUCCESS on successful completion
//		HSE_STATUS_ERROR on failure

DWORD SendInstructionPage(IN EXTENSION_CONTROL_BLOCK *pECB)
{
	char szStatus[] = "200 OK";
	char szContent[] =
	"<html>"
	"<head><title>Redirect URL</title></head>"
	"<body>"
	"<h1>Redirect.dll</h1>\r\n<hr>\r\n"
	"Redirect.dll returns the resource specified on the query string.<br>\r\n<br>\r\n"
	"To specify a resource on the same server as Redirect.dll, use the following form:<br>\r\n<br>\r\n"
	"<code> http://server/scripts/Redirect.dll?/virtualdir/file.htm </code><br>\r\n<br>\r\n"
	"To specify a resource on another server, use the following form:<br>\r\n<br>\r\n"
	"<code> http://server/scripts/Redirect.dll?http://server/virtualdir/file.htm </code>"
	"</body>"
	"</html>";

	char szHeaderBase[] = "Content-Length: %lu\r\nContent-type: text/html\r\n\r\n";
	char szHeader[BUFF_SIZE];

	// Fill in byte count in Content-Length header

	DWORD cchContent = strlen(szContent);

	sprintf_s(szHeader, sizeof(szHeader), szHeaderBase, cchContent);

	// Populate SendHeaderExInfo struct

	HSE_SEND_HEADER_EX_INFO SendHeaderExInfo;

	SendHeaderExInfo.pszStatus = szStatus;
	SendHeaderExInfo.pszHeader = szHeader;
	SendHeaderExInfo.cchStatus = strlen(szStatus);
	SendHeaderExInfo.cchHeader = strlen(szHeader);
	SendHeaderExInfo.fKeepConn = FALSE;

	// Send header

	if (!pECB->ServerSupportFunction(pECB->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX, &SendHeaderExInfo, NULL, NULL))
		return HSE_STATUS_ERROR;

	//  Send content

	if (!pECB->WriteClient(pECB->ConnID, szContent, &cchContent, 0))
		return HSE_STATUS_ERROR;

	return HSE_STATUS_SUCCESS;
}

//	Description:
//
//		This function is called when the WWW service is shutdown
//
//	Arguments:
//
//		dwFlags - HSE_TERM_ADVISORY_UNLOAD or HSE_TERM_MUST_UNLOAD
//
//	Return Value:
//
//		TRUE if extension is ready to be unloaded, FALSE otherwise

BOOL WINAPI TerminateExtension(IN DWORD dwFlags)
{
	return TRUE;
}




//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module PRVTRACE Implementation Module | This module contains header information for PRVTRACE.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 04-10-96	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 PRVTRACE Elements|
//
// @subindex PRVTRACE
//
//---------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////
#include "privstd.h"		//Private library common precompiled header
#include "PRVTRACE.H"
#include "Miscfunc.h"		//Conversion Routines...



//////////////////////////////////////////////////////////////////
// INT MessageBox
//
//////////////////////////////////////////////////////////////////
INT MessageBox(
	HWND hwnd,							// Parent window for message display
	UINT uiStyle,						// Style of message box
	CHAR* pszTitle,						// Title for message
	CHAR* pszFmt,						// Format string
	...									// Substitution parameters
	)
{
	va_list		marker;
	CHAR		szBuffer[MAX_TRACE_LEN];

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pszFmt);
	_vsnprintf(szBuffer, MAX_TRACE_LEN, pszFmt, marker);
	va_end(marker);
   
	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_TRACE_LEN
	szBuffer[MAX_TRACE_LEN-1] = '\0';

	//Delegate
	return MessageBoxA(hwnd, szBuffer, pszTitle, uiStyle);
}


//////////////////////////////////////////////////////////////////
// void InternalTrace
//
//////////////////////////////////////////////////////////////////
void InternalTrace(const WCHAR*	pwszFmt, ...)
{
	va_list		marker;
	WCHAR		wszBuffer[MAX_TRACE_LEN];
	CHAR		szBuffer[MAX_TRACE_LEN];

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf(wszBuffer, MAX_TRACE_LEN, pwszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_TRACE_LEN
	wszBuffer[MAX_TRACE_LEN-2] = L'\n';
	wszBuffer[MAX_TRACE_LEN-1] = L'\0';
	
	//Convert to MBCS
	ConvertToMBCS(wszBuffer, szBuffer, MAX_TRACE_LEN);
	
	//Output to the DebugWindow
	for(ULONG x=0; x <= strlen(szBuffer); x+=1029)
		OutputDebugStringA(szBuffer+x);
}


//////////////////////////////////////////////////////////////////
// void InternalTrace
//
//////////////////////////////////////////////////////////////////
void InternalTrace(const CHAR*  pszFmt, ...)
{
	va_list		marker;
	CHAR		szBuffer[MAX_TRACE_LEN];

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pszFmt);
	_vsnprintf(szBuffer, MAX_TRACE_LEN, pszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_TRACE_LEN
	szBuffer[MAX_TRACE_LEN-2] = '\n';
	szBuffer[MAX_TRACE_LEN-1] = '\0';	

	//Output to the DebugWindow
	for(ULONG x=0; x <= strlen(szBuffer); x+=1029)
			OutputDebugStringA(szBuffer);
}


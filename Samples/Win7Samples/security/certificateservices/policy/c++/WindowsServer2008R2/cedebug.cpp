//+---------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//	File:		cedebug.cpp
//
//	Contents:	Debug support
//
//----------------------------------------------------------------------------

#include <pch.cpp>

#pragma hdrstop

#include "celib.h"
#include <stdarg.h>


//+-------------------------------------------------------------------------
//
//  Function:  ceDbgPrintf
//
//  Synopsis:  outputs debug info to stdout and debugger
//
//  Returns:   number of chars output
//
//--------------------------------------------------------------------------

int WINAPIV
ceDbgPrintf(
    IN BOOL fDebug,
    IN LPCSTR lpFmt,
    ...)
{
    va_list arglist;
    CHAR ach[4096];
    int cch = 0;
    HANDLE hStdOut;
    DWORD dwErr;

    dwErr = GetLastError();
    if (fDebug)
    {
	try
	{
	    HRESULT hr;
	    va_start(arglist, lpFmt);
	    hr = StringCbVPrintfA(ach, sizeof(ach), lpFmt, arglist);
	    va_end(arglist);

	    if (S_OK == hr || STRSAFE_E_INSUFFICIENT_BUFFER == hr)
	    {
		if (STRSAFE_E_INSUFFICIENT_BUFFER == hr)
		{
		    StringCchCopyA(&ach[sizeof(ach) - 5], 5, "...\n");
		}
		ach[ARRAYSIZE(ach) - 1] = L'\0';
		cch = (int)strlen(ach);

		if (!IsDebuggerPresent())
		{
		    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		    if (hStdOut != INVALID_HANDLE_VALUE)
		    {
			fputs(ach, stdout);
			fflush(stdout);
		    }
		}
		OutputDebugStringA(ach);
	    }
	}
    catch (...)
	{
	    // return failure
	    cch = 0;
	}
    }
    SetLastError(dwErr);
    return(cch);
}

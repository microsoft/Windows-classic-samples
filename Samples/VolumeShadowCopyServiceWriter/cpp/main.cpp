/*
**++
**
** Copyright (c) Microsoft Corporation
**
**
** Module Name:
**
**	main.cpp
**
**
** Abstract:
**
**	Sample dependency writer
**
*/


///////////////////////////////////////////////////////////////////////////////
// Includes

#include "stdafx.h"
#include "main.h"
#include "swriter.h"
///////////////////////////////////////////////////////////////////////////////
// Declarations

HANDLE g_quitEvent = NULL;


///////////////////////////////////////////////////////////////////////////////

extern "C" int __cdecl  wmain(__in int argc, __in_ecount(argc)wchar_t **)
{
    UNREFERENCED_PARAMETER(argc);
	HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED );
	if (FAILED(hr))	{
	    wprintf(L"CoInitializeEx failed!");
	    return 1;
	}
	
   hr = ::CoInitializeSecurity(
       NULL,                                 //  IN PSECURITY_DESCRIPTOR         pSecDesc,
       -1,                                  //  IN LONG                         cAuthSvc,
       NULL,                                //  IN SOLE_AUTHENTICATION_SERVICE *asAuthSvc,
       NULL,                                //  IN void                        *pReserved1,
       RPC_C_AUTHN_LEVEL_PKT_PRIVACY,       //  IN DWORD                        dwAuthnLevel,
       RPC_C_IMP_LEVEL_IDENTIFY,            //  IN DWORD                        dwImpLevel,
       NULL,                                //  IN void                        *pAuthList,
       EOAC_NONE,
                                            //  IN DWORD                        dwCapabilities,
       NULL                                 //  IN void                        *pReserved3
       );
	if (FAILED(hr))	{
		wprintf(L"CoInitializeSecurity failed!");
		return 1;
	}

	g_quitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_quitEvent == NULL)	{
		wprintf(L"CreateEvent failed!");
		return 1;
	}

	// set a control handler that allows the writer to be shut down
	if (!::SetConsoleCtrlHandler(handler, TRUE))	{
		wprintf(L"SetConsoleSecurityHandler failed!");
		return 1;
	}

    DepWriter::StaticInitialize();
        
	// We want the writer to go out of scope before the return statement
	{
		DepWriter writer;
		hr = writer.Initialize();
		if (FAILED(hr))	{
		    wprintf(L"Writer init failed!");
		    return 1;
		}


		if(::WaitForSingleObject(g_quitEvent, INFINITE) != WAIT_OBJECT_0)	{
			wprintf(L"WaitForSingleObject failed!");
			return 1;
		}
		writer.Uninitialize();
	}
	
	return 0;	
}

BOOL WINAPI handler(DWORD)
{
	// we want to quit independent of what the control event was
	::SetEvent(g_quitEvent);

	return TRUE;
}

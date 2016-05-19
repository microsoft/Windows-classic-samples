#ifndef __COM_ROUTINGEXTENSION_SAMPLE
//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------
#define __COM_ROUTINGEXTENSION_SAMPLE
//
//Includes
//
#include <faxcomex.h>
#include <windows.h>
#include <winbase.h>
#include <stdlib.h>
#include <objbase.h>
#include <tchar.h>
#include <assert.h>
#include <shellapi.h>




#include <stdio.h>
#include <faxroute.h>
#include <winfax.h>
#include <faxext.h>
#include <strsafe.h>
#include <sddl.h>

#define STRSAFE_NO_DEPRECATE


//
// macros
//
#define ValidString( String ) ((String) ? (String) : L"" )

#ifdef DBG
#define ROUTEDEBUG( parm ) DebugPrint parm
#else
#define ROUTEDEBUG( parm )
#endif 

//Constants
#define ROUTEITGUID           L"{5797dee0-e738-11d0-83c0-00c04fb6e984}"
#define FAXROUTE              L"FaxRouteEvent"
#define LOGNAME               L"%temp%\\ReceiveLog.txt"
#define ININAME               L"%temp%\\FaxRoute.ini"
#define ROUTEDIR              L"SampleRouteFolder"

#define EXTENSIONNAME         L"SampleRoute Routing Extension"
#define EXTENSIONFRIENDLYNAME L"SampleRoute: Fax Route"
#define EXTENSIONPATH         L"%systemroot%\\system32\\FaxRouteIt.dll"

#define FAXROUTEMETHOD        L"Siren"
#define FAXROUTEFRIENDLYNAME  L"Routing Siren"
#define FAXROUTEFUNCTION      L"RouteIt"

#define STR_SIZE 1024

#define   REGVAL_RM_FLAGS_GUID                  TEXT("{0A9D7B3A-A35E-4cc6-B73C-1F635136057B}")

#define VISTA 6

// Default values for configuration
#define DEFAULT_FLAGS               0       // Routing method not enabled

// forward declarations
BOOL WriteRoutingInfoIntoIniFile( LPWSTR TiffFileName, PFAX_ROUTE FaxRoute ); 
void DebugPrint( LPWSTR, ... );
BOOL WINAPI ExtensionCallback( IN HANDLE FaxHandle, IN LPVOID Context, IN OUT LPWSTR MethodName, IN OUT LPWSTR FriendlyName, IN OUT LPWSTR FunctionName, IN OUT LPWSTR Guid );
HRESULT FaxExtInitializeConfig ( PFAX_EXT_GET_DATA pFaxExtGetData, PFAX_EXT_SET_DATA pFaxExtSetData, PFAX_EXT_REGISTER_FOR_EVENTS pFaxExtRegisterForEvents, PFAX_EXT_UNREGISTER_FOR_EVENTS  pFaxExtUnregisterForEvents, PFAX_EXT_FREE_BUFFER pFaxExtFreeBuffer);
DWORD ReadConfiguration (DWORD dwDevId);
BOOL IsServiceAuthSystem();
        
// globals
PFAXROUTEADDFILE    FaxRouteAddFile = NULL;
PFAXROUTEDELETEFILE FaxRouteDeleteFile =NULL;
PFAXROUTEGETFILE    FaxRouteGetFile= NULL;  
PFAXROUTEENUMFILES  FaxRouteEnumFiles =NULL;
PFAXROUTEMODIFYROUTINGDATA  FaxRouteModifyRoutingData;

HANDLE              hHeap;
HANDLE              hReceiveEvent;
CRITICAL_SECTION    csRoute;
LPWSTR              IniFile = NULL;
LPWSTR              LogFile = NULL;

// GUID of routing methods usage flags - used by the Microsoft Fax Routing Extension DLL:
DWORD g_dwFlags = 0;
HINSTANCE           g_hModule = NULL;

// Extension data callbacks into the server:
PFAX_EXT_GET_DATA               g_pFaxExtGetData = NULL;
PFAX_EXT_SET_DATA               g_pFaxExtSetData = NULL;
PFAX_EXT_REGISTER_FOR_EVENTS    g_pFaxExtRegisterForEvents = NULL;
PFAX_EXT_UNREGISTER_FOR_EVENTS  g_pFaxExtUnregisterForEvents = NULL;
PFAX_EXT_FREE_BUFFER            g_pFaxExtFreeBuffer = NULL;

#endif
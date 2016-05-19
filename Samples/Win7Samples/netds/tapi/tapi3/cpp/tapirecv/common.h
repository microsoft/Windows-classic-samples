/*

Copyright (c) 1999 - 2000  Microsoft Corporation


Module Name:

    common.h


Abstract:

    include files, definitions and declarations used in the app

*/


#define _WIN32_DCOM

#define WM_PRIVATETAPIEVENT   WM_USER+101

#include <stdio.h>
#include <windows.h>
#include <tapi3.h>
#include <crtdbg.h>

#include <control.h>
#include <strmif.h>

#include <uuids.h>
#include <mmsystem.h>
#include <amstream.h>

#include <vfw.h>
#include <Mtype.h>
#include <strsafe.h>


#include "WorkerThread.h"


//
// the event to signal when it's time to exit
//

extern HANDLE g_hExitEvent;


//
// the tapi object
//

extern ITTAPI *g_pTapi;


//
// The current call. We only process one call at a time
//

extern ITBasicCallControl *g_pCurrentCall;


//
// critical section for protecting the global current call
//

extern  CRITICAL_SECTION g_CurrentCallCritSection;


//
// thread for asynchronous message processing
//

extern CWorkerThread g_WorkerThread;


//
// logging functions
//

void LogMessage(CHAR *pszFormat, ... );

#define LogError LogMessage

void LogFormat(const WAVEFORMATEX *pWaveFormat);


//
// tapi event handler
//

HRESULT OnTapiEvent(TAPI_EVENT TapiEvent, IDispatch *pEvent);


//
// memory allocation routines
//

void *AllocateMemory(SIZE_T nMemorySize);

void FreeMemory(void* pMemory);

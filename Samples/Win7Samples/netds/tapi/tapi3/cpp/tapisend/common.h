/*

Copyright (c) 1999 - 2000  Microsoft Corporation


Module Name:

    common.h


Abstract:

    include files and declarations used throughout the application

*/



#define _WIN32_DCOM


#include <stdio.h>
#include <windows.h>
#include <tapi3.h>

#include <vfw.h>
#include <crtdbg.h>

#include <uuids.h>
#include <Mtype.h>

#include <amstream.h>

#include <vfwmsgs.h>

#include <deque>
#include <strsafe.h>



//
// logging functions
//

void LogMessage(CHAR *pszFormat, ... );

#define LogError LogMessage

void LogFormat(WAVEFORMATEX *pWaveFormat);


//
// memory allocation routines
//

void *AllocateMemory(SIZE_T nMemorySize);

void FreeMemory(void* pMemory);



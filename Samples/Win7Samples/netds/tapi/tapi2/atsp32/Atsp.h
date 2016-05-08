/*++

Copyright 1995 - 2000 Microsoft Corporation

Module Name:

    atsp.h

Notes:

--*/

#include <windows.h>
#include "tapi.h"
#include "tspi.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "resource.h"
#include <initguid.h> 


//                                                                      
// Line device GUID of MSP                                                    
//                                                  
// CLSID = s '// 23F7C678- 24E1 -48db- AE AC- 54 A6 F2 A0 10 A0


DEFINE_GUID(CLSID_SAMPMSP, 
0x23F7C678, 0x24E1, 0x48db, 0xAE, 0xAC, 0x54, 0xA6, 0xF2, 0xA0, 0x10, 0xA0);



#define  MAX_DEV_NAME_LENGTH    63
#define  ATSP_TIMEOUT           60000   // milliseconds


typedef struct _DRVLINE
{
    HTAPILINE               htLine;

    LINEEVENT               pfnEventProc;

	// 
	// MSP Variables

	DWORD				    dwMSPHandle;
	
    HTAPIMSPLINE			htMSPLineHandle;

	//

    DWORD                   dwDeviceID;

    char                    szComm[8];

    HTAPICALL               htCall;

    DWORD                   dwCallState;

    DWORD                   dwCallStateMode;

    DWORD                   dwMediaMode;

    HANDLE                  hComm;

    BOOL                    bDropInProgress;

    OVERLAPPED              Overlapped;

} DRVLINE, FAR *PDRVLINE;


typedef struct _DRVLINECONFIG
{
    char                    szPort[8];

    char                    szCommands[64];

} DRVLINECONFIG, FAR *PDRVLINECONFIG;


typedef struct _ASYNC_REQUEST
{
    DWORD                   dwRequestID;

    DWORD                   dwCommand;

    char                    szCommand[32];

    struct _ASYNC_REQUEST  *pNext;

} ASYNC_REQUEST, *PASYNC_REQUEST;


#if DBG

typedef struct _FUNC_PARAM
{
    char        *lpszVal;

    DWORD       dwVal;

} FUNC_PARAM, *PFUNC_PARAM;

#endif

typedef struct _FUNC_INFO
{

#if DBG

    char        *lpszFuncName;

    DWORD       dwNumParams;

    PFUNC_PARAM aParams;

#endif

    LONG        lResult;

} FUNC_INFO, *PFUNC_INFO;


DWORD               gdwLineDeviceIDBase;
DWORD               gdwPermanentProviderID;
HANDLE              ghInst;
ASYNC_COMPLETION    gpfnCompletionProc;

char gszAtspKey[]      = "Software\\Microsoft\\ATSP";
char gszNumLines[]     = "NumLines";
char gszDefLineConfigParams[] = "my new line,COM1,L0";

#if DBG

char gszTab[]          = "    ";
char gszhdLine[]       = "hdLine";
char gszhdCall[]       = "hdCall";
char gszdwSize[]       = "dwSize";
char gszhwndOwner[]    = "hwndOwner";
char gszdwDeviceID[]   = "dwDeviceID";
char gszdwRequestID[]  = "dwRequestID";
char gszlpCallParams[] = "lpCallParams";
char gszdwPermanentProviderID[] = "dwPermanentProviderID";

DWORD   gdwDebugLevel = 0;

void
CDECL
DebugOutput(
    DWORD   dwLevel,
    LPCSTR  lpszFormat,
    ...
    );

#define DBGOUT(arg) DebugOutput arg

LONG
PASCAL
Epilog(
    PFUNC_INFO  pInfo,
    LONG        lResult
    );

void
PASCAL
Prolog(
    PFUNC_INFO  pInfo
    );

#else

#define DBGOUT(arg)

#define Epilog(pAsyncRequestInfo, lResult) (lResult)

#define Prolog(pAsyncRequestInfo)

#endif



LPVOID
PASCAL
DrvAlloc(
    DWORD dwSize
    );

VOID
PASCAL
DrvFree(
    LPVOID lp
    );

void
PASCAL
SetCallState(
    PDRVLINE    pLine,
    DWORD       dwCallState,
    DWORD       dwCallStateMode
    );

BOOL
CALLBACK
ConfigDlgProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

LONG
PASCAL
ProviderInstall(
    char   *pszProviderName,
    BOOL    bNoMultipleInstance
    );

void
PASCAL
DropActiveCall(
    PDRVLINE    pLine
    );

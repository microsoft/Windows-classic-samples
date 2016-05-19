/*++

Copyright 1995 - 2000 Microsoft Corporation

Module Name:

    atsp.c

Notes:

--*/

               
#define TAPI_CURRENT_VERSION 0x00030000
#include "atsp.h"
#include <assert.h>
#include <strsafe.h>
#define assert_str(hr) assert(SUCCEEDED(hr) || !"strsafe")
#define MAX_BUF_SIZE 256

BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    if (dwReason ==  DLL_PROCESS_ATTACH)
    {
        ghInst = hDLL;

#if DBG
        {
            HKEY    hKey;
            DWORD   dwDataSize, dwDataType;
            char    szAtsp32DebugLevel[] = "Atsp32DebugLevel";


            RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,
                gszAtspKey,
                0,
                KEY_ALL_ACCESS,
                &hKey
                );

            dwDataSize = sizeof (DWORD);
            gdwDebugLevel=0;

            RegQueryValueEx(
                hKey,
                szAtsp32DebugLevel,
                0,
                &dwDataType,
                (LPBYTE) &gdwDebugLevel,
                &dwDataSize
                );

            RegCloseKey (hKey);
        }
#endif

    }

    return TRUE;
}


void
CommThread(
    PDRVLINE    pLine
    )
{
    char            buf[4];
    DWORD           dwThreadID = GetCurrentThreadId(), dwNumBytes;
    HANDLE          hComm = pLine->hComm, hEvent;
    LPOVERLAPPED    pOverlapped = &pLine->Overlapped;


    DBGOUT((
        3,
        "CommThread (id=%d): enter, port=%s",
        dwThreadID,
        pLine->szComm
        ));

    hEvent = pOverlapped->hEvent;
    buf[0] = buf[1] = '.';


    //
    // Loop waiting for i/o to complete (either the Write done in
    // TSPI_lineMakeCall or the Reads done to retrieve status info).
    // Note that TSPI_lineDrop or TSPI_lineCloseCall may set the
    // event to alert us that they're tearing down the call, in
    // which case we just exit.
    //

    for (;;)
    {
        if (WaitForSingleObject (hEvent, ATSP_TIMEOUT) == WAIT_OBJECT_0)
        {
            if (pLine->bDropInProgress == TRUE)
            {
                DBGOUT((2, "CommThread (id=%d): drop in progress"));
                goto CommThread_exit;
            }

            GetOverlappedResult (hComm, pOverlapped, &dwNumBytes, FALSE);
            ResetEvent (hEvent);
        }
        else
        {
            DBGOUT((2, "CommThread (id=%d): wait timeout"));
            SetCallState (pLine, LINECALLSTATE_IDLE, 0);
            goto CommThread_exit;
        }

        buf[1] &= 0x7f; // remove the parity bit

        DBGOUT((
            3,
            "CommThread (id=%d): read '%c'",
            dwThreadID,
            (buf[1] == '\r' ? '.' : buf[1])
            ));

        switch ((buf[0] << 8) + buf[1])
        {
        case 'CT':  // "CONNECT"
        case 'OK':  // "OK"

            SetCallState (pLine, LINECALLSTATE_CONNECTED, 0);
            goto CommThread_exit;

        case 'SY':  // "BUSY"
        case 'OR':  // "ERROR"
        case 'NO':  // "NO ANSWER", "NO DIALTONE", "NO CARRIER"

            SetCallState (pLine, LINECALLSTATE_IDLE, 0);
            goto CommThread_exit;

        default:

            break;
        }

        buf[0] = buf[1];

        ZeroMemory (pOverlapped, sizeof (OVERLAPPED) - sizeof (HANDLE));
        ReadFile (hComm, &buf[1], 1, &dwNumBytes, pOverlapped);
    }

CommThread_exit:

    CloseHandle (hEvent);
    DBGOUT((3, "CommThread (id=%d): exit", dwThreadID));
    ExitThread (0);
}


//
// We get a slough of C4047 (different levels of indrection) warnings down
// below in the initialization of FUNC_PARAM structs as a result of the
// real func prototypes having params that are types other than DWORDs,
// so since these are known non-interesting warnings just turn them off
//

#pragma warning (disable:4047)


//
// --------------------------- TAPI_lineXxx funcs -----------------------------
//


LONG
TSPIAPI TSPI_lineMSPIdentify(
    DWORD               dwDeviceID,
    GUID *              pCLSID
    )
{

   LONG  lResult = 0;

#if DBG
    FUNC_PARAM  params[] =
    {
        { "dwDeviceID", dwDeviceID }
    };

    FUNC_INFO   info =
    {
        "TSPI_lineMSPIdentify",
        1,
        params
    };

#endif


    Prolog (&info); 

	//
	// Identify the CLSID of the our MSP
	// this is the CLSID of Sample MSP in the 
	// TAPI3 SDK
	//
    
    *pCLSID = CLSID_SAMPMSP;
    
    return (Epilog (&info, lResult));

}


LONG
TSPIAPI TSPI_lineReceiveMSPData(   							
	HDRVLINE        hdLine,
    HDRVCALL        hdCall,         
    HDRVMSPLINE     hdMSPLine,		
    LPBYTE          pBuffer,
    DWORD           dwSize
    )
{
	LONG lResult             = 0;

#if DBG
	FUNC_PARAM  params[] =
	{
		{  gszhdLine, hdLine },
		{  gszhdCall, hdCall},
		{ "hdMSPLine", hdMSPLine},
		{ "pBuffer"  , pBuffer},
		{ "dwSize" , dwSize}
		
	};
	
	FUNC_INFO   info =
	{
		"TSPI_lineReceiveMSPData",
		5,
		params
	};
	
#endif
	
	Prolog (&info); 
	
	//
	// Place code to Process Data received from the MSP
	//
	
	return (Epilog (&info, lResult));
}

LONG
TSPIAPI		
TSPI_lineCreateMSPInstance(
	  HDRVLINE        hdLine,
      DWORD           dwAddressID,
      HTAPIMSPLINE    htMSPLine,
      LPHDRVMSPLINE   phdMSPLine
    )
{
    LONG		lResult = 0;
	PDRVLINE    pLine   = (PDRVLINE) hdLine;

#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,      hdLine},
        { "dwAddressID",  dwAddressID },
        { "htMSPLine",    htMSPLine  }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineCreateMSPInstance",
        3,
        params
    };

#endif


    Prolog (&info);
   
	//
	// we save the TAPI MSP handle as a member value
	// in the line in order to be able to send MSP messages
	// with the line/client identification to TAPISRV.
	// tapisrv identifies the client with the call handle and or the
	// MSP handle.. both cannot be ambiguous.
	//

	pLine->htMSPLineHandle = htMSPLine;


	//
	// we fake a TTSP handle for TAPI we are not keeping track of this
	//

	*phdMSPLine = 0;

	return (Epilog (&info, lResult));
    
}

LONG
TSPIAPI TSPI_lineCloseMSPInstance(
		HDRVMSPLINE         hdMSPLine
        )
{
   LONG  lResult = 0;

#if DBG
    FUNC_PARAM  params[] =
    {
        { "hdMSPLine", hdMSPLine}
    };

    FUNC_INFO   info =
    {
        "TSPI_lineCloseMSPInstance",
        1,
        params
    };

#endif


    Prolog (&info);

	//
	// no - op there is no information stored with the 
	// msp so we dont need to free anything or remove anything 
	//

    return (Epilog (&info, lResult));

}




LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE    hdLine
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine, hdLine }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineClose",
        1,
        params,
    };
#endif

    Prolog (&info);
    DrvFree ((PDRVLINE) hdLine);
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL    hdCall
    )
{
    PDRVLINE    pLine = (PDRVLINE) hdCall;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdCall, hdCall  }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineCloseCall",
        1,
        params
    };
#endif


    //
    // Note that in TAPI 2.0 TSPI_lineCloseCall can get called
    // without TSPI_lineDrop ever being called, so we need to
    // be prepared for either case.
    //

    Prolog (&info);
    DropActiveCall (pLine);
    pLine->htCall = NULL;
    return (Epilog (&info, 0));
}


LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,        hdLine       },
        { "dwMediaModes",   dwMediaModes },
        { gszlpCallParams,  lpCallParams }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineConditionalMediaDetection",
        3,
        params
    };
#endif


    //
    // This func is really a no-op for us, since we don't look
    // for incoming calls (though we do say we support them to
    // make apps happy)
    //

    Prolog (&info);
    return (Epilog (&info, 0));
}


LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    PDRVLINE    pLine = (PDRVLINE) hdCall;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwRequestID,        dwRequestID     },
        { gszhdCall,             hdCall          },
        { "lpsUserUserInfo",    lpsUserUserInfo },
        { gszdwSize,             dwSize          }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineDrop",
        4,
        params
    };
#endif


    Prolog (&info);
    DropActiveCall (pLine);
    SetCallState (pLine, LINECALLSTATE_IDLE, 0);
    (*gpfnCompletionProc)(dwRequestID, 0);
    return (Epilog (&info, dwRequestID));
}


LONG
TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD              dwDeviceID,
    DWORD              dwAddressID,
    DWORD              dwTSPIVersion,
    DWORD              dwExtVersion,
    LPLINEADDRESSCAPS  lpAddressCaps
    )
{

#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwDeviceID,     dwDeviceID      },
        { "dwAddressID",    dwAddressID     },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "dwExtVersion",   dwExtVersion    },
        { "lpAddressCaps",  lpAddressCaps   }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetAddressCaps",
        5,
        params
    };
#endif

    LONG        lResult = 0;


    Prolog (&info);

    if (dwAddressID != 0)
    {
        lResult = LINEERR_INVALADDRESSID;
    }

    lpAddressCaps->dwNeededSize =
    lpAddressCaps->dwUsedSize   = sizeof(LINEADDRESSCAPS);

    lpAddressCaps->dwLineDeviceID       = dwDeviceID;
    lpAddressCaps->dwAddressSharing     = LINEADDRESSSHARING_PRIVATE;
    lpAddressCaps->dwCallInfoStates     = LINECALLINFOSTATE_MEDIAMODE |
                                          LINECALLINFOSTATE_APPSPECIFIC;
    lpAddressCaps->dwCallerIDFlags      =
    lpAddressCaps->dwCalledIDFlags      =
    lpAddressCaps->dwRedirectionIDFlags =
    lpAddressCaps->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;
    lpAddressCaps->dwCallStates         = LINECALLSTATE_IDLE |
                                          LINECALLSTATE_OFFERING |
                                          LINECALLSTATE_ACCEPTED |
                                          LINECALLSTATE_DIALTONE |
                                          LINECALLSTATE_DIALING |
                                          LINECALLSTATE_CONNECTED |
                                          LINECALLSTATE_PROCEEDING |
                                          LINECALLSTATE_DISCONNECTED |
                                          LINECALLSTATE_UNKNOWN;
    lpAddressCaps->dwDialToneModes      = LINEDIALTONEMODE_UNAVAIL;
    lpAddressCaps->dwBusyModes          = LINEBUSYMODE_UNAVAIL;
    lpAddressCaps->dwSpecialInfo        = LINESPECIALINFO_UNAVAIL;
    lpAddressCaps->dwDisconnectModes    = LINEDISCONNECTMODE_NORMAL |
                                          LINEDISCONNECTMODE_BUSY |
                                          LINEDISCONNECTMODE_NOANSWER |
                                          LINEDISCONNECTMODE_UNAVAIL |
                                          LINEDISCONNECTMODE_NODIALTONE;
    lpAddressCaps->dwMaxNumActiveCalls  = 1;
    lpAddressCaps->dwAddrCapFlags       = LINEADDRCAPFLAGS_DIALED;
    lpAddressCaps->dwCallFeatures       = LINECALLFEATURE_ACCEPT |
                                          LINECALLFEATURE_ANSWER |
                                          LINECALLFEATURE_DROP |
                                          LINECALLFEATURE_SETCALLPARAMS;
    lpAddressCaps->dwAddressFeatures    = LINEADDRFEATURE_MAKECALL;

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,             hdLine         },
        { "dwAddressID",        dwAddressID     },
        { "lpAddressStatus",    lpAddressStatus }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetAddressStatus",
        3,
        params
    };
#endif

    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdLine;


    Prolog (&info);

    lpAddressStatus->dwNeededSize =
    lpAddressStatus->dwUsedSize   = sizeof(LINEADDRESSSTATUS);

    lpAddressStatus->dwNumActiveCalls  = (pLine->htCall ? 1 : 0);
    lpAddressStatus->dwAddressFeatures = LINEADDRFEATURE_MAKECALL;

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL            hdCall,
    LPDWORD             lpdwAddressID
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdCall,        hdCall          },
        { "lpdwAddressID",  lpdwAddressID   }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetCallAddressID",
        2,
        params
    };
#endif


    //
    // We only support 1 address (id=0)
    //

    Prolog (&info);
    *lpdwAddressID = 0;
    return (Epilog (&info, 0));
}


LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  lpLineInfo
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdCall,     hdCall      },
        { "lpLineInfo", lpLineInfo  }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetCallInfo",
        2,
        params
    };
#endif
    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdCall;


    Prolog (&info);

    lpLineInfo->dwNeededSize =
    lpLineInfo->dwUsedSize   = sizeof(LINECALLINFO);

    lpLineInfo->dwBearerMode         = LINEBEARERMODE_VOICE;
    lpLineInfo->dwMediaMode          = pLine->dwMediaMode;
    lpLineInfo->dwCallStates         = LINECALLSTATE_IDLE |
                                       LINECALLSTATE_DIALTONE |
                                       LINECALLSTATE_DIALING |
                                       LINECALLSTATE_CONNECTED |
                                       LINECALLSTATE_PROCEEDING |
                                       LINECALLSTATE_DISCONNECTED |
                                       LINECALLSTATE_UNKNOWN;
    lpLineInfo->dwOrigin             = LINECALLORIGIN_OUTBOUND;
    lpLineInfo->dwReason             = LINECALLREASON_DIRECT;
    lpLineInfo->dwCallerIDFlags      =
    lpLineInfo->dwCalledIDFlags      =
    lpLineInfo->dwConnectedIDFlags   =
    lpLineInfo->dwRedirectionIDFlags =
    lpLineInfo->dwRedirectingIDFlags = LINECALLPARTYID_UNAVAIL;

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpLineStatus
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdCall,         hdCall          },
        { "lpLineStatus",   lpLineStatus    }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetCallStatus",
        2,
        params
    };
#endif
    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdCall;


    Prolog (&info);

    lpLineStatus->dwNeededSize =
    lpLineStatus->dwUsedSize   = sizeof(LINECALLSTATUS);

    lpLineStatus->dwCallState  = pLine->dwCallState;

    if (pLine->dwCallState != LINECALLSTATE_IDLE)
    {
        lpLineStatus->dwCallFeatures = LINECALLFEATURE_DROP;
    }

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD           dwDeviceID,
    DWORD           dwTSPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
  HRESULT hr;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwDeviceID,     dwDeviceID      },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "dwExtVersion",   dwExtVersion    },
        { "lpLineDevCaps",  lpLineDevCaps   }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetDevCaps",
        4,
        params
    };
#endif

    LONG            lResult = 0;
    static WCHAR    szProviderInfo[] = L"AT-compatible modem service provider";

    #define PROVIDER_INFO_SIZE (37 * sizeof (WCHAR))

    Prolog (&info);

    lpLineDevCaps->dwNeededSize = sizeof (LINEDEVCAPS) + PROVIDER_INFO_SIZE +
        (MAX_DEV_NAME_LENGTH + 1) * sizeof (WCHAR);

    if (lpLineDevCaps->dwTotalSize >= lpLineDevCaps->dwNeededSize)
    {
        #define LINECONFIG_SIZE   (2 * (MAX_DEV_NAME_LENGTH + 1) + 40)

        char    szLineConfig[LINECONFIG_SIZE], szLineN[16], *p;
        HKEY    hKey;
        DWORD   dwDataSize, dwDataType;


        lpLineDevCaps->dwUsedSize = lpLineDevCaps->dwNeededSize;

        lpLineDevCaps->dwProviderInfoSize   = PROVIDER_INFO_SIZE;
        lpLineDevCaps->dwProviderInfoOffset = sizeof(LINEDEVCAPS);

        hr = StringCbCopyW((WCHAR *)(lpLineDevCaps + 1),lpLineDevCaps->dwTotalSize, szProviderInfo);
        assert_str(hr);

        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszAtspKey,
            0,
            KEY_ALL_ACCESS,
            &hKey
            );

        dwDataSize = LINECONFIG_SIZE;
        hr = StringCbPrintfA (szLineN,sizeof(szLineN), "Line%d", dwDeviceID - gdwLineDeviceIDBase);
        assert_str(hr);
        hr = StringCbCopyA (szLineConfig, sizeof(szLineConfig),gszDefLineConfigParams);		
        assert_str(hr);		

        RegQueryValueEx(
            hKey,
            szLineN,
            0,
            &dwDataType,
            (LPBYTE) szLineConfig,
            &dwDataSize
            );

        RegCloseKey (hKey);

        for (p = szLineConfig; *p != ','; p++);
        *p = 0;

        lpLineDevCaps->dwLineNameSize   = (lstrlen (szLineConfig) + 1) *
            sizeof (WCHAR);
        lpLineDevCaps->dwLineNameOffset = sizeof(LINEDEVCAPS) +
            PROVIDER_INFO_SIZE;

        MultiByteToWideChar(
            CP_ACP,
            MB_PRECOMPOSED,
            szLineConfig,
            -1,
            (WCHAR *) ((LPBYTE) (lpLineDevCaps + 1) + PROVIDER_INFO_SIZE),
            (lpLineDevCaps->dwLineNameSize)/sizeof(WCHAR)
            );
    }
    else
    {
        lpLineDevCaps->dwUsedSize = sizeof(LINEDEVCAPS);
    }

    lpLineDevCaps->dwStringFormat      = STRINGFORMAT_ASCII;
    lpLineDevCaps->dwAddressModes      = LINEADDRESSMODE_ADDRESSID;
    lpLineDevCaps->dwNumAddresses      = 1;
    lpLineDevCaps->dwBearerModes       = LINEBEARERMODE_VOICE;
    lpLineDevCaps->dwMaxRate           = 9600;
    lpLineDevCaps->dwMediaModes        = LINEMEDIAMODE_INTERACTIVEVOICE |
                                         LINEMEDIAMODE_DATAMODEM;


	//
	// Specify  LINEDEVCAPFLAGS_MSP so tapi knows to call lineMSPIdentify for
	// the MSP CLSID. Note: version must be 3.0 or higher
	//

    lpLineDevCaps->dwDevCapFlags       = LINEDEVCAPFLAGS_CLOSEDROP |
                                         LINEDEVCAPFLAGS_DIALBILLING |
                                         LINEDEVCAPFLAGS_DIALQUIET |
                                         LINEDEVCAPFLAGS_DIALDIALTONE |
										 LINEDEVCAPFLAGS_MSP;

	lpLineDevCaps->dwAddressTypes	   =  LINEADDRESSTYPE_PHONENUMBER;

    lpLineDevCaps->dwMaxNumActiveCalls = 1;
    lpLineDevCaps->dwRingModes         = 1;
    lpLineDevCaps->dwLineFeatures      = LINEFEATURE_MAKECALL;

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE    hdLine,
    DWORD       dwAddressID,
    HDRVCALL    hdCall,
    DWORD       dwSelect,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass,
    HANDLE      hTargetProcess
    )
{
HRESULT hr;

#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,             hdLine          },
        { "dwAddressID",        dwAddressID     },
        { gszhdCall,             hdCall          },
        { "dwSelect",           dwSelect        },
        { "lpDeviceID",         lpDeviceID      },
        { "lpszDeviceClass",    lpszDeviceClass },
        { "hTargetProcess",     hTargetProcess  }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetID",
        7,
        params
    };
#endif

    DWORD       dwNeededSize = sizeof(VARSTRING) + sizeof (DWORD);
    LONG        lResult = 0;
    PDRVLINE    pLine = (dwSelect == LINECALLSELECT_CALL ?
                    (PDRVLINE) hdCall : (PDRVLINE) hdLine);


    Prolog (&info);

    if (lstrcmpiW (lpszDeviceClass, L"tapi/line") == 0)
    {
        if (lpDeviceID->dwTotalSize < dwNeededSize)
        {
            lpDeviceID->dwUsedSize = 3*sizeof(DWORD);
        }
        else
        {
            lpDeviceID->dwUsedSize = dwNeededSize;

            lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
            lpDeviceID->dwStringSize   = sizeof(DWORD);
            lpDeviceID->dwStringOffset = sizeof(VARSTRING);

            *((LPDWORD)(lpDeviceID + 1)) = pLine->dwDeviceID;
        }

        lpDeviceID->dwNeededSize = dwNeededSize;
    }
    else if (lstrcmpiW (lpszDeviceClass, L"comm/datamodem") == 0)
    {
#ifdef _WIN64
        dwNeededSize += ((DWORD)strlen(pLine->szComm) + 1) * sizeof (WCHAR);
#else
        dwNeededSize += (strlen (pLine->szComm) + 1) * sizeof (WCHAR);
#endif

        if (lpDeviceID->dwTotalSize < dwNeededSize)
        {
            lpDeviceID->dwUsedSize = 3 * sizeof(DWORD);
        }
        else
        {
            HANDLE hCommDup = NULL;


            if (!pLine->htCall)
            {
                DBGOUT((1, "TSPI_lineGetID32: error, no active call"));

                lResult = LINEERR_OPERATIONFAILED;

                goto TSPI_lineGetID_epilog;
            }

            if (!DuplicateHandle(
                    GetCurrentProcess(),
                    pLine->hComm,
                    hTargetProcess,
                    &hCommDup,
                    0,
                    TRUE,
                    DUPLICATE_SAME_ACCESS
                    ))
            {
                DBGOUT((
                    1,
                    "TSPI_lineGetID: DupHandle failed, err=%ld",
                    GetLastError()
                    ));

                lResult = LINEERR_OPERATIONFAILED;

                goto TSPI_lineGetID_epilog;
            }

            lpDeviceID->dwUsedSize = dwNeededSize;

            lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
            lpDeviceID->dwStringSize   = dwNeededSize - sizeof(VARSTRING);
            lpDeviceID->dwStringOffset = sizeof(VARSTRING);

            *((HANDLE *)(lpDeviceID + 1)) = hCommDup;


            hr = StringCbCopyA(
                ((char *)(lpDeviceID + 1)) + sizeof (HANDLE),
                lpDeviceID->dwTotalSize - sizeof(VARSTRING) -sizeof(HANDLE),
                 pLine->szComm
                );
            assert_str(hr);

            MultiByteToWideChar(
                CP_ACP,
                0,
                pLine->szComm,
                -1,
                (WCHAR *)((LPBYTE)((((WCHAR *)(lpDeviceID + 1)))) + sizeof (HANDLE)),
                (lpDeviceID->dwTotalSize - sizeof(VARSTRING) -sizeof(HANDLE))/sizeof(WCHAR)
                );
        }

        lpDeviceID->dwNeededSize = dwNeededSize;
    }
    else
    {
        lResult = LINEERR_NODEVICE;
    }

TSPI_lineGetID_epilog:

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,            hdLine          },
        { "lpLineDevStatus",    lpLineDevStatus }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetLineDevStatus",
        2,
        params
    };
#endif

    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdLine;


    Prolog (&info);

    lpLineDevStatus->dwUsedSize =
    lpLineDevStatus->dwNeededSize = sizeof (LINEDEVSTATUS);

    lpLineDevStatus->dwNumActiveCalls = (pLine->htCall ? 1 : 0);
    //lpLineDevStatus->dwLineFeatures =
    lpLineDevStatus->dwDevStatusFlags = LINEDEVSTATUSFLAGS_CONNECTED |
                                        LINEDEVSTATUSFLAGS_INSERVICE;
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE    hdLine,
    LPDWORD     lpdwNumAddressIDs
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,            hdLine            },
        { "lpdwNumAddressIDs",  lpdwNumAddressIDs }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineGetNumAddressIDs",
        2,
        params
    };
#endif

    LONG        lResult = 0;
    PDRVLINE    pLine = (PDRVLINE) hdLine;


    //
    // We only support 1 address (id=0)
    //

    Prolog (&info);
    *lpdwNumAddressIDs = 1;
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    char        szCommands[64], szCommand[64], szDestAddress[128];
    DWORD       dwThreadID, dwNumBytes, dwError;
    PDRVLINE    pLine = (PDRVLINE) hdLine;
    HRESULT hr;
	
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwRequestID,       dwRequestID     },
        { gszhdLine,            hdLine          },
        { "htCall",             htCall          },
        { "lphdCall",           lphdCall        },
        { "lpszDestAddress",    szDestAddress   },
        { "dwCountryCode",      dwCountryCode   },
        { gszlpCallParams,      lpCallParams    }
    };
    FUNC_INFO info =
    {
        "TSPI_lineMakeCall",
        7,
        params
    };
#endif


    if (lpszDestAddress)
    {
        WideCharToMultiByte(
            CP_ACP,
            0,
            lpszDestAddress,
            -1,
            (LPSTR) szDestAddress,
            128,
            NULL,
            NULL
            );
    }

    Prolog (&info);


    //
    // Check to see if there's already another call
    //

    if (pLine->htCall)
    {
        (*gpfnCompletionProc)(dwRequestID, LINEERR_CALLUNAVAIL);
        goto TSPI_lineMakeCall_return;
    }


    //
    // Since we don't support TSPI_lineDial, fail if app tries
    // to pass a NULL lpszDestAddress (implying that app just
    // wants to go offhook)
    //

    if (lpszDestAddress == NULL)
    {
        (*gpfnCompletionProc)(dwRequestID, LINEERR_INVALADDRESS);
        goto TSPI_lineMakeCall_return;
    }


    //
    // Get the line's config info
    //

    {
        HKEY    hKey;
        DWORD   dwDataSize, dwDataType;
        char    szLineN[8], *pszConfig, *p, *p2;


        hr = StringCbPrintfA(
            szLineN,
            sizeof(szLineN),
            "Line%d",
            ((PDRVLINE) hdLine)->dwDeviceID - gdwLineDeviceIDBase
            );
        assert_str(hr);
        dwDataSize = 256;

        pszConfig = DrvAlloc (dwDataSize);

        if (!pszConfig) {
	      (*gpfnCompletionProc)(dwRequestID, LINEERR_NOMEM);
        	goto TSPI_lineMakeCall_return;
      	}

        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszAtspKey,
            0,
            KEY_ALL_ACCESS,
            &hKey
            );

        RegQueryValueEx(
            hKey,
            szLineN,
            0,
            &dwDataType,
            (LPBYTE) pszConfig,
            &dwDataSize
            );

        pszConfig[dwDataSize] = '\0';       // *pszConfig = "MyLine,COM1,L0"

        RegCloseKey (hKey);


        //
        // szComm
        //

        for (p = pszConfig; *p != ','; p++);
        p++;                                // *p = "COM1,L0"
        for (p2 = p; *p2 != ','; p2++);
        *p2 = 0;                            // *p = "COM1"

        hr = StringCbCopyA(pLine->szComm, sizeof(pLine->szComm),p );
        assert_str(hr);		

        //
        // szCommands
        //

        p2++;                               // *p2 = "L0"
        hr = StringCbCopyA(szCommands, sizeof(szCommands),p2);
        assert_str(hr);		
        DrvFree (pszConfig);
    }


    //
    // Open the port
    //

    if ((pLine->hComm = CreateFile(
            pLine->szComm,
            GENERIC_READ | GENERIC_WRITE,
            0, //FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, // no security attrs
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL  // no template file

            )) == INVALID_HANDLE_VALUE)
    {
        DBGOUT((
            3,
            "TSPI_lineMakeCall: CreateFile(%s) failed, err=%ld",
            pLine->szComm,
            GetLastError()
            ));

        (*gpfnCompletionProc)(dwRequestID, LINEERR_RESOURCEUNAVAIL);
        goto TSPI_lineMakeCall_return;
    }


    //
    // Setup up the modem command string.  If there's an initial 'T'
    // or 'P' (for Tone or Pulse) in the dest address then disregard
    // it.  Also if it's a voice call add the semi colon so we return
    // to cmd mode.
    //

    {
        char *p = (char *) szDestAddress;


        if (*p == 'T'  ||  *p == 'P')
        {
            p++;
        }

        if (lpCallParams &&
            lpCallParams->dwMediaMode != LINEMEDIAMODE_INTERACTIVEVOICE)
        {
            hr = StringCbPrintfA (szCommand, sizeof(szCommand),"AT%sDT%s\r", szCommands, p);
            assert_str(hr);
        }
        else
        {
            hr = StringCbPrintfA (szCommand, sizeof(szCommand),"AT%sDT%s;\r", szCommands, p);
            assert_str(hr);
        }
    }


    //
    // Init the data structure & tell tapi our handle to the call
    //

    pLine->htCall          = htCall;
    pLine->bDropInProgress = FALSE;
    pLine->dwMediaMode     = (lpCallParams ? lpCallParams->dwMediaMode :
        LINEMEDIAMODE_INTERACTIVEVOICE);

    *lphdCall = (HDRVCALL) pLine;


    //
    // Do an overlapped write, the comm thread will deal with the results
    //

    pLine->Overlapped.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

    if (!WriteFile(
            pLine->hComm,
            szCommand,
            lstrlen (szCommand),
            &dwNumBytes,
            &pLine->Overlapped
            )

        && (dwError = GetLastError()) != ERROR_IO_PENDING)
    {
        DBGOUT((
            1,
            "TSPI_lineMakeCall: WriteFile(%s) failed, error=%d",
            pLine->szComm,
            dwError
            ));

        pLine->htCall = NULL;
        CloseHandle (pLine->hComm);
        CloseHandle (pLine->Overlapped.hEvent);
        (*gpfnCompletionProc)(dwRequestID, LINEERR_OPERATIONFAILED);
        goto TSPI_lineMakeCall_return;
    }


    //
    // Complete the requests & set the initial call state
    //

    (*gpfnCompletionProc)(dwRequestID, 0);
    SetCallState (pLine, LINECALLSTATE_DIALING, 0);


    //
    // Spin the comm thread to handle the results of the Write above
    //

    {
        HANDLE hCommThread;


        if (!(hCommThread = CreateThread(
                (LPSECURITY_ATTRIBUTES) NULL,
                0,
                (LPTHREAD_START_ROUTINE) CommThread,
                pLine,
                0,
                &dwThreadID
                )))
        {
            DBGOUT((
                1,
                "TSPI_lineMakeCall: CreateThread failed, err=%ld",
                GetLastError()
                ));

            GetOverlappedResult(
                pLine->hComm,
                &pLine->Overlapped,
                &dwNumBytes,
                TRUE
                );

            SetCallState (pLine, LINECALLSTATE_IDLE, 0);
            CloseHandle (pLine->hComm);
            CloseHandle (pLine->Overlapped.hEvent);
            goto TSPI_lineMakeCall_return;
        }

        CloseHandle (hCommThread);
    }


TSPI_lineMakeCall_return:

    return (Epilog (&info, dwRequestID));
}


LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwDeviceID,        dwDeviceID      },
        { "dwLowVersion",       dwLowVersion    },
        { "dwHighVersion",      dwHighVersion   },
        { "lpdwTSPIVersion",    lpdwTSPIVersion }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineNegotiateTSPIVersion",
        4,
        params
    };
#endif

    Prolog (&info);

	// version 3.0 to allow MSP usage
	//

    *lpdwTSPIVersion = 0x00030000;
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineOpen(
    DWORD       dwDeviceID,
    HTAPILINE   htLine,
    LPHDRVLINE  lphdLine,
    DWORD       dwTSPIVersion,
    LINEEVENT   lpfnEventProc
    )
{
    LONG        lResult;
    PDRVLINE    pLine;
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszdwDeviceID,    dwDeviceID      },
        { "htLine",         htLine          },
        { "lphdLine",       lphdLine        },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "lpfnEventProc",  lpfnEventProc   }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineOpen",
        5,
        params
    };
#endif


    Prolog (&info);

    if ((pLine = DrvAlloc (sizeof (DRVLINE))))
    {
        pLine->htLine       = htLine;
        pLine->pfnEventProc = lpfnEventProc;
        pLine->dwDeviceID   = dwDeviceID;

        *lphdLine = (HDRVLINE) pLine;

        lResult = 0;
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE    hdLine,
    DWORD       dwMediaModes
    )
{
#if DBG
    FUNC_PARAM  params[] =
    {
        { gszhdLine,        hdLine       },
        { "dwMediaModes",   dwMediaModes }
    };
    FUNC_INFO   info =
    {
        "TSPI_lineSetDefaultMediaDetection",
        2,
        params
    };
#endif


    //
    // This func is really a no-op for us, since we don't look
    // for incoming calls (though we do say we support them to
    // make apps happy)
    //

    Prolog (&info);
    return (Epilog (&info, 0));
}


//
// ------------------------- TSPI_providerXxx funcs ---------------------------
//

LONG
TSPIAPI
TSPI_providerConfig(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    //
    // Although this func is never called by TAPI v2.0, we export
    // it so that the Telephony Control Panel Applet knows that it
    // can configure this provider via lineConfigProvider(),
    // otherwise Telephon.cpl will not consider it configurable
    //

    return 0;
}


LONG
TSPIAPI
TSPI_providerGenericDialogData(
    DWORD_PTR           dwObjectID,
    DWORD               dwObjectType,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "dwObjectID",     dwObjectID      },
        { "dwObjectType",   dwObjectType    },
        { "lpParams",       lpParams        },
        { "dwSize",         dwSize          }
    };
    FUNC_INFO   info =
    {
        "TSPI_providerGenericDialogData",
        4,
        params
    };
#endif


    Prolog (&info);
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD_PTR           dwNumLines,
    DWORD_PTR           dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "dwTSPIVersion",          dwTSPIVersion           },
        { gszdwPermanentProviderID, dwPermanentProviderID   },
        { "dwLineDeviceIDBase",     dwLineDeviceIDBase      },
        { "dwPhoneDeviceIDBase",    dwPhoneDeviceIDBase     },
        { "dwNumLines",             dwNumLines              },
        { "dwNumPhones",            dwNumPhones             },
        { "lpfnCompletionProc",     lpfnCompletionProc      }
    };
    FUNC_INFO   info =
    {
        "TSPI_providerInit",
        7,
        params
    };
#endif

    Prolog (&info);
    gdwLineDeviceIDBase = dwLineDeviceIDBase;
    gpfnCompletionProc  = lpfnCompletionProc;
    *lpdwTSPIOptions = LINETSPIOPTION_NONREENTRANT;
    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_providerInstall(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    //
    // Although this func is never called by TAPI v2.0, we export
    // it so that the Telephony Control Panel Applet knows that it
    // can add this provider via lineAddProvider(), otherwise
    // Telephon.cpl will not consider it installable
    //
    //

    return 0;
}


LONG
TSPIAPI
TSPI_providerRemove(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    //
    // Although this func is never called by TAPI v2.0, we export
    // it so that the Telephony Control Panel Applet knows that it
    // can remove this provider via lineRemoveProvider(), otherwise
    // Telephon.cpl will not consider it removable
    //

    return 0;
}


LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion,
    DWORD   dwPermanentProviderID
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "dwTSPIVersion",          dwTSPIVersion },
        { gszdwPermanentProviderID, dwPermanentProviderID   }
    };
    FUNC_INFO   info =
    {
        "TSPI_providerShutdown",
        2,
        params
    };
#endif


    Prolog (&info);

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TSPI_providerEnumDevices(
    DWORD       dwPermanentProviderID,
    LPDWORD     lpdwNumLines,
    LPDWORD     lpdwNumPhones,
    HPROVIDER   hProvider,
    LINEEVENT   lpfnLineCreateProc,
    PHONEEVENT  lpfnPhoneCreateProc
    )
{
   HKEY     hKey;
   DWORD    dwNumLines, dwDataType, dwDataSize;


   //
   // Retrieve the number of devices we're
   // configured for from our registry section
   //

   RegOpenKeyEx(
       HKEY_LOCAL_MACHINE,
       gszAtspKey,
       0,
       KEY_ALL_ACCESS,
       &hKey
       );

   dwDataSize = sizeof(dwNumLines);
   dwNumLines = 0;

   RegQueryValueEx(
       hKey,
       gszNumLines,
       0,
       &dwDataType,
       (LPBYTE) &dwNumLines,
       &dwDataSize
       );

   RegCloseKey (hKey);

   *lpdwNumLines  = dwNumLines;
   *lpdwNumPhones = 0;
   return 0;
}


LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPWSTR   lpszUIDLLName
    )
{
    LONG        lResult = 0;
    HRESULT     hr = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "lpsUIDLLName",  lpszUIDLLName }
    };
    FUNC_INFO   info =
    {
        "TSPI_providerUIIdentify",
        1,
        params
    };
#endif


    Prolog (&info);
    hr = StringCbCopyW(lpszUIDLLName,MAX_PATH, L"atsp32.tsp");
    assert_str(hr);
    return (Epilog (&info, lResult));
}


//
// ---------------------------- TUISPI_xxx funcs ------------------------------
//

LONG
TSPIAPI
TUISPI_lineConfigDialog(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass
    )
{
    char        szDeviceClass[128];
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "lpfnUIDLLCallback",   lpfnUIDLLCallback },
        { gszdwDeviceID,         dwDeviceID        },
        { gszhwndOwner,          hwndOwner         },
        { "lpszDeviceClass",     szDeviceClass     }
    };
    FUNC_INFO   info =
    {
        "TUISPI_lineConfigDialog",
        4,
        params
    };
#endif


    if (lpszDeviceClass)
    {
        WideCharToMultiByte(
            CP_ACP,
            0,
            lpszDeviceClass,
            -1,
            (LPSTR) szDeviceClass,
            128,
            NULL,
            NULL
            );
    }

    Prolog (&info);

    DialogBoxParam(
        ghInst,
        MAKEINTRESOURCE(IDD_DIALOG1),
        hwndOwner,
        (DLGPROC) ConfigDlgProc,
        0
        );

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TUISPI_providerConfig(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    LONG        lResult = 0;
#if DBG
    FUNC_PARAM  params[] =
    {
        { "lpfnUIDLLCallback",      lpfnUIDLLCallback },
        { gszhwndOwner,             hwndOwner    },
        { gszdwPermanentProviderID, dwPermanentProviderID   }
    };
    FUNC_INFO   info =
    {
        "TUISPI_providerConfig",
        3,
        params
    };
#endif


    Prolog (&info);

    DialogBoxParam(
        ghInst,
        MAKEINTRESOURCE(IDD_DIALOG1),
        hwndOwner,
        (DLGPROC) ConfigDlgProc,
        0
        );

    return (Epilog (&info, lResult));
}


LONG
TSPIAPI
TUISPI_providerInstall(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    LONG    lResult;


    if ((lResult = ProviderInstall ("atsp32.tsp", TRUE)) == 0)
    {
        DialogBoxParam(
            ghInst,
            MAKEINTRESOURCE(IDD_DIALOG1),
            hwndOwner,
            (DLGPROC) ConfigDlgProc,
            0
            );
    }

    return lResult;
}


LONG
TSPIAPI
TUISPI_providerRemove(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    HKEY    hKey;
    char    szSoftwareMsft[] = "Software\\Microsoft", szATSP[] = "ATSP";


    //
    // Clean up our registry section
    //

    RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        szSoftwareMsft,
        0,
        KEY_ALL_ACCESS,
        &hKey
        );

    RegDeleteKeyA (hKey, szATSP);
    RegCloseKey (hKey);
    return 0;
}


#pragma warning (default:4047)


//
// ---------------------- Misc private support routines -----------------------
//



void
PASCAL
EnableChildren(
    HWND    hwnd,
    BOOL    bEnable
    )
{
    int i;
    static int aiControlIDs[] =
    {
        IDC_DEVICES,
        IDC_NAME,
        IDC_PORT,
        IDC_COMMANDS,
        IDC_REMOVE,
        0
    };


    for (i = 0; aiControlIDs[i]; i++)
    {
        EnableWindow (GetDlgItem (hwnd, aiControlIDs[i]), bEnable);
    }
}


void
PASCAL
SelectDevice(
    HWND    hwnd,
    LRESULT     iDevice
    )
{
    SendDlgItemMessage (hwnd, IDC_DEVICES, LB_SETCURSEL, iDevice, 0);
    PostMessage(hwnd, WM_COMMAND, IDC_DEVICES | (LBN_SELCHANGE << 16), 0);
}


BOOL
CALLBACK
ConfigDlgProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    static  HKEY    hAtspKey;

    DWORD   dwDataSize;
    DWORD   dwDataType;
    HRESULT hr;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        char   *pBuf;
        DWORD   i, iNumLines;


        //
        // Create or open our configuration key in the registry.  If the
        // create fails it may well be that the current user does not
        // have write access to this portion of the registry, so we'll
        // just show a "read only" dialog and not allow user to make any
        // changes
        //

        {
            LONG    lResult;
            DWORD   dwDisposition;


            if ((lResult = RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszAtspKey,
                    0,
                    "",
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    (LPSECURITY_ATTRIBUTES) NULL,
                    &hAtspKey,
                    &dwDisposition

                    )) != ERROR_SUCCESS)
            {
                DBGOUT((
                    3,
                    "RegCreateKeyEx(%s,ALL_ACCESS) failed, err=%d",
                    gszAtspKey,
                    lResult
                    ));

                if ((lResult = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        gszAtspKey,
                        0,
                        KEY_QUERY_VALUE,
                        &hAtspKey

                        )) != ERROR_SUCCESS)
                {
                    DBGOUT((
                        3,
                        "RegOpenKeyEx(%s,ALL_ACCESS) failed, err=%d",
                        gszAtspKey,
                        lResult
                        ));

                    EndDialog (hwnd, 0);
                    return FALSE;
                }

                {
                    int i;
                    static int aiControlIDs[] =
                    {
                        IDC_NAME,
                        IDC_PORT,
                        IDC_COMMANDS,
                        IDC_ADD,
                        IDC_REMOVE,
                        IDOK,
                        0
                    };


                    for (i = 0; aiControlIDs[i]; i++)
                    {
                        EnableWindow(
                            GetDlgItem (hwnd, aiControlIDs[i]),
                            FALSE
                            );
                    }
                }
            }
        }


        //
        // Retrieve our configuration info from the registry
        //

        dwDataSize = sizeof(iNumLines);
        iNumLines = 0;

        RegQueryValueEx(
            hAtspKey,
            gszNumLines,
            0,
            &dwDataType,
            (LPBYTE) &iNumLines,
            &dwDataSize
            );

        SendDlgItemMessage(
            hwnd,
            IDC_NAME,
            EM_LIMITTEXT,
            MAX_DEV_NAME_LENGTH,
            0
            );

        SendDlgItemMessage(
            hwnd,
            IDC_COMMANDS,
            EM_LIMITTEXT,
            MAX_DEV_NAME_LENGTH,
            0
            );

        if (!(pBuf = DrvAlloc (MAX_BUF_SIZE)))
        	break;

        for (i = 0; i < iNumLines; i++)
        {
            char           *p, *p2, szLineN[8];
            PDRVLINECONFIG  pLineConfig;
            LONG            lResult;

	     if (!(pLineConfig = DrvAlloc (sizeof(DRVLINECONFIG))))
	     	break;

            hr = StringCbPrintfA (szLineN, sizeof(szLineN),"Line%d", i);
            assert_str(hr);

            dwDataSize = 256;

            lResult = RegQueryValueEx(
                hAtspKey,
                szLineN,
                0,
                &dwDataType,
                (LPBYTE) pBuf,
                &dwDataSize
                );


            //
            // If there was a problem, use the default config
            //

            if (0 != lResult)
            {
                hr = StringCbCopyA (pBuf,MAX_BUF_SIZE, gszDefLineConfigParams);
                assert_str(hr);
            }

            for (p = pBuf; *p != ','; p++);
            *p = 0;

            SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_ADDSTRING,
                0,
                (LPARAM) pBuf
                );

            SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_SETITEMDATA,
                i,
                (LPARAM) pLineConfig
                );

            p++;
            for (p2 = p; *p2 != ','; p2++);
            *p2 = 0;

            hr = StringCbCopyA (pLineConfig->szPort,sizeof(pLineConfig->szPort), p );
            assert_str(hr);


            p = p2 + 1;

            hr = StringCbCopyA (pLineConfig->szCommands,sizeof(pLineConfig->szCommands), p);
            assert_str(hr);

        }

        DrvFree (pBuf);


        //
        // Fill up the various controls with configuration options
        //

        {
            static char *aszPorts[] = { "COM1","COM2","COM3",NULL };

            for (i = 0; aszPorts[i]; i++)
            {
                SendDlgItemMessage(
                    hwnd,
                    IDC_PORT,
                    LB_ADDSTRING,
                    0,
                    (LPARAM) aszPorts[i]
                    );
            }
        }

        if (iNumLines == 0)
        {
            EnableChildren (hwnd, FALSE);
        }
        else
        {
            SelectDevice (hwnd, 0);
        }

        break;
    }
    case WM_COMMAND:
    {
        LRESULT             iSelection;
        PDRVLINECONFIG  pLineConfig;


        iSelection = SendDlgItemMessage(
            hwnd,
            IDC_DEVICES,
            LB_GETCURSEL,
            0,
            0
            );

        pLineConfig = (PDRVLINECONFIG) SendDlgItemMessage(
            hwnd,
            IDC_DEVICES,
            LB_GETITEMDATA,
            (WPARAM) iSelection,
            0
            );

        switch (LOWORD((DWORD)wParam))
        {
        case IDC_DEVICES:

            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                char buf[MAX_DEV_NAME_LENGTH + 1];


                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_GETTEXT,
                    iSelection,
                    (LPARAM) buf
                    );

                SetDlgItemText (hwnd, IDC_NAME, buf);

                SendDlgItemMessage(
                    hwnd,
                    IDC_PORT,
                    LB_SELECTSTRING,
                    (WPARAM) -1,
                    (LPARAM) pLineConfig->szPort
                    );

                SetDlgItemText (hwnd, IDC_COMMANDS, pLineConfig->szCommands);
            }

            break;

        case IDC_NAME:

            if ((HIWORD(wParam) == EN_CHANGE) && (iSelection != LB_ERR))
            {
                char    buf[MAX_DEV_NAME_LENGTH + 1];


                GetDlgItemText (hwnd, IDC_NAME, buf, MAX_DEV_NAME_LENGTH);

                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_DELETESTRING,
                    iSelection,
                    0
                    );

                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_INSERTSTRING,
                    iSelection,
                    (LPARAM) buf
                    );

                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_SETCURSEL,
                    iSelection,
                    0
                    );

                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_SETITEMDATA,
                    iSelection,
                    (LPARAM) pLineConfig
                    );
            }

            break;

        case IDC_PORT:

            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                iSelection = SendDlgItemMessage(
                    hwnd,
                    IDC_PORT,
                    LB_GETCURSEL,
                    0,
                    0
                    );

                SendDlgItemMessage(
                    hwnd,
                    IDC_PORT,
                    LB_GETTEXT,
                    iSelection,
                    (LPARAM) pLineConfig->szPort
                    );
            }

            break;

        case IDC_COMMANDS:

            if ((HIWORD(wParam) == EN_CHANGE) && (iSelection != LB_ERR))
            {
                GetDlgItemText(
                    hwnd,
                    IDC_COMMANDS,
                    pLineConfig->szCommands,
                    63
                    );
            }

            break;

        case IDC_ADD:
        {
            LRESULT             iNumLines;
			int				i = 2;
            char            szLineName[32];
            PDRVLINECONFIG  pLineConfig;

            if(!(pLineConfig = DrvAlloc (sizeof(DRVLINECONFIG))))
            	break;


            iNumLines = SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_GETCOUNT,
                0,
                0
                );

            hr = StringCbCopyA (pLineConfig->szPort, sizeof(pLineConfig->szPort),"COM1");
            assert_str(hr);


            hr = StringCbCopyA (szLineName,sizeof(szLineName), "my new line");
            assert_str(hr);


find_unique_line_name:

            if (SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_FINDSTRING,
                    (WPARAM) -1,
                    (LPARAM) szLineName

                    ) != LB_ERR)
            {
                hr = StringCbPrintfA (szLineName, sizeof(szLineName),"my new line%d", i++);
                assert_str(hr);
                goto find_unique_line_name;
            }

            SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_ADDSTRING,
                0,
                (LPARAM) szLineName
                );

            SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_SETITEMDATA,
                iNumLines,
                (LPARAM) pLineConfig
                );

            EnableChildren (hwnd, TRUE);

            SelectDevice (hwnd, iNumLines);

            SetFocus (GetDlgItem (hwnd, IDC_NAME));

            SendDlgItemMessage(
                hwnd,
                IDC_NAME,
                EM_SETSEL,
                0,
                (LPARAM) -1
                );

            break;
        }
        case IDC_REMOVE:
        {
            LRESULT iNumLines;


            DrvFree (pLineConfig);

            iNumLines = SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_DELETESTRING,
                iSelection,
                0
                );

            if (iNumLines == 0)
            {
                SetDlgItemText (hwnd, IDC_NAME, "");
                SetDlgItemText (hwnd, IDC_COMMANDS, "");

                EnableChildren (hwnd, FALSE);
            }
            else
            {
                SelectDevice (hwnd, 0);
            }

            break;
        }
        case IDOK:
        {
            int     i;
			LRESULT	iNumLines;
            char   *pBuf;


            //
            // Update the num lines & num phones values
            //

            if (!(pBuf = DrvAlloc (256)))
            	break;

            iNumLines = SendDlgItemMessage(
                hwnd,
                IDC_DEVICES,
                LB_GETCOUNT,
                0,
                0
                );

            RegSetValueEx(
                hAtspKey,
                gszNumLines,
                0,
                REG_DWORD,
                (LPBYTE) &iNumLines,
                sizeof(DWORD)
                );


            //
            // For each installed device save it's config info
            //

            for (i = 0; i < iNumLines; i++)
            {
                char szLineN[8];
                PDRVLINECONFIG pLineConfig;


                SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_GETTEXT,
                    i,
                    (LPARAM) pBuf
                    );

                pLineConfig = (PDRVLINECONFIG) SendDlgItemMessage(
                    hwnd,
                    IDC_DEVICES,
                    LB_GETITEMDATA,
                    i,
                    0
                    );

                hr = StringCbPrintfA(
                    pBuf + strlen (pBuf),
                    MAX_BUF_SIZE-strlen(pBuf),
                    ",%s,%s",
                    pLineConfig->szPort,
                    pLineConfig->szCommands
                    );
                assert_str(hr);

                hr = StringCbPrintfA (szLineN, sizeof(szLineN),"Line%d", i);
                assert_str(hr);

                RegSetValueEx(
                    hAtspKey,
                    szLineN,
                    0,
                    REG_SZ,
                    (LPBYTE) pBuf,
                    lstrlen (pBuf) + 1
                    );

                DrvFree (pLineConfig);
            }

            DrvFree (pBuf);

            // fall thru to EndDialog...
        }
        case IDCANCEL:

            RegCloseKey (hAtspKey);
            EndDialog (hwnd, 0);
            break;

        } // switch (LOWORD((DWORD)wParam))

        break;
    }
    } // switch (msg)

    return FALSE;
}


LPVOID
PASCAL
DrvAlloc(
    DWORD dwSize
    )
{
    return (LocalAlloc (LPTR, dwSize));
}


VOID
PASCAL
DrvFree(
    LPVOID lp
    )
{
    LocalFree (lp);
}


void
PASCAL
SetCallState(
    PDRVLINE    pLine,
    DWORD       dwCallState,
    DWORD       dwCallStateMode
    )
{
    if (dwCallState != pLine->dwCallState)
    {
        pLine->dwCallState     = dwCallState;
        pLine->dwCallStateMode = dwCallStateMode;

        (*pLine->pfnEventProc)(
            pLine->htLine,
            pLine->htCall,
            LINE_CALLSTATE,
            dwCallState,
            dwCallStateMode,
            pLine->dwMediaMode
            );
    }
}


#if DBG

void
PASCAL
Prolog(
    PFUNC_INFO  pInfo
    )
{
    DWORD i;


    DBGOUT((3, "%s: enter", pInfo->lpszFuncName));

    for (i = 0; i < pInfo->dwNumParams; i++)
    {
        if (pInfo->aParams[i].dwVal &&
            pInfo->aParams[i].lpszVal[3] == 'z') // lpszVal = "lpsz..."
        {
            DBGOUT((
                3,
                "%s%s=x%lx, '%s'",
                gszTab,
                pInfo->aParams[i].lpszVal,
                pInfo->aParams[i].dwVal,
                pInfo->aParams[i].dwVal
                ));
        }
        else
        {
            DBGOUT((
                3,
                "%s%s=x%lx",
                gszTab,
                pInfo->aParams[i].lpszVal,
                pInfo->aParams[i].dwVal
                ));
        }
    }
}


LONG
PASCAL
Epilog(
    PFUNC_INFO  pInfo,
    LONG        lResult
    )
{
    DBGOUT((3, "%s: returning x%x", pInfo->lpszFuncName, lResult));

    return lResult;
}


void
CDECL
DebugOutput(
    DWORD   dwDbgLevel,
    LPCSTR  lpszFormat,
    ...
    )
{
    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[128] = "ATSP32: ";
        va_list ap;


        va_start(ap, lpszFormat);

        _vsnwprintf_s(&buf[8],sizeof(buf)-8, 119, lpszFormat, ap );


        strncat_s (buf, sizeof(buf),"\n", 127 - strlen(buf));
		buf[127] = '\0';

        OutputDebugString (buf);

        va_end(ap);
    }
}

#endif


LONG
PASCAL
ProviderInstall(
    char   *pszProviderName,
    BOOL    bNoMultipleInstance
    )
{
    LONG    lResult;


    //
    // If only one installation instance of this provider is
    // allowed then we want to check the provider list to see
    // if the provider is already installed
    //

    if (bNoMultipleInstance)
    {
        LONG                (WINAPI *pfnGetProviderList)();
        DWORD               dwTotalSize, i;
        HINSTANCE           hTapi32;
        LPLINEPROVIDERLIST  pProviderList;
        LPLINEPROVIDERENTRY pProviderEntry;


        //
        // Load Tapi32.dll & get a pointer to the lineGetProviderList
        // func.  We don't want to statically link because this module
        // plays the part of both core SP & UI DLL, and we don't want
        // to incur the performance hit of automatically loading
        // Tapi32.dll when running as a core SP within Tapisrv.exe's
        // context.
        //

        if (!(hTapi32 = LoadLibrary ("tapi32.dll")))
        {
            DBGOUT((
                1,
                "LoadLibrary(tapi32.dll) failed, err=%d",
                GetLastError()
                ));

            lResult = LINEERR_OPERATIONFAILED;
            goto ProviderInstall_return;
        }

        if (!(pfnGetProviderList = (LONG (WINAPI *)())GetProcAddress(
                hTapi32,
                (LPCSTR) "lineGetProviderList"
                )))
        {
            DBGOUT((
                1,
                "GetProcAddr(lineGetProviderList) failed, err=%d",
                GetLastError()
                ));

            lResult = LINEERR_OPERATIONFAILED;
            goto ProviderInstall_unloadTapi32;
        }


        //
        // Loop until we get the full provider list
        //

        dwTotalSize = sizeof (LINEPROVIDERLIST);

        goto ProviderInstall_allocProviderList;

ProviderInstall_getProviderList:

        if ((lResult = (*pfnGetProviderList)(0x00020000, pProviderList)) != 0)
        {
            goto ProviderInstall_freeProviderList;
        }

        if (pProviderList->dwNeededSize > pProviderList->dwTotalSize)
        {
            dwTotalSize = pProviderList->dwNeededSize;

            LocalFree (pProviderList);

ProviderInstall_allocProviderList:

            if (!(pProviderList = LocalAlloc (LPTR, dwTotalSize)))
            {
                lResult = LINEERR_NOMEM;
                goto ProviderInstall_unloadTapi32;
            }

            pProviderList->dwTotalSize = dwTotalSize;

            goto ProviderInstall_getProviderList;
        }


        //
        // Inspect the provider list entries to see if this provider
        // is already installed
        //

        pProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) pProviderList) +
            pProviderList->dwProviderListOffset);

        for (i = 0; i < pProviderList->dwNumProviders; i++)
        {
            char   *pszInstalledProviderName = ((char *) pProviderList) +
                        pProviderEntry->dwProviderFilenameOffset,
                   *p;


            //
            // Make sure pszInstalledProviderName points at <filename>
            // and not <path>\filename by walking backeards thru the
            // string searching for last '\\'
            //

            p = pszInstalledProviderName +
                lstrlen (pszInstalledProviderName) - 1;

            for (; *p != '\\'  &&  p != pszInstalledProviderName; p--);

            pszInstalledProviderName =
                (p == pszInstalledProviderName ? p : p + 1);

            if (lstrcmpiA (pszInstalledProviderName, pszProviderName) == 0)
            {
                lResult = LINEERR_NOMULTIPLEINSTANCE;
                goto ProviderInstall_freeProviderList;
            }

            pProviderEntry++;
        }


        //
        // If here then the provider isn't currently installed,
        // so do whatever configuration stuff is necessary and
        // indicate SUCCESS
        //

        lResult = 0;


ProviderInstall_freeProviderList:

        LocalFree (pProviderList);

ProviderInstall_unloadTapi32:

        FreeLibrary (hTapi32);
    }
    else
    {
        //
        // Do whatever configuration stuff is necessary and return SUCCESS
        //

        lResult = 0;
    }

ProviderInstall_return:

    return lResult;
}


void
PASCAL
DropActiveCall(
    PDRVLINE    pLine
    )
{
    if (pLine->hComm)
    {
        DWORD       dwNumBytes, dwError;
        OVERLAPPED  overlapped;


        pLine->bDropInProgress = TRUE;

        SetEvent (pLine->Overlapped.hEvent);

        ZeroMemory (&overlapped, sizeof (OVERLAPPED));

        overlapped.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

        if (pLine->dwMediaMode != LINEMEDIAMODE_INTERACTIVEVOICE)
        {
            if (!WriteFile(
                    pLine->hComm,
                    "+++\r", 4,
                    &dwNumBytes,
                    &overlapped
                    ))
            {
                if ((dwError = GetLastError()) == ERROR_IO_PENDING)
                {
                    GetOverlappedResult(
                        pLine->hComm,
                        &overlapped,
                        &dwNumBytes,
                        TRUE
                        );

                    ResetEvent (overlapped.hEvent);
                }
                else
                {
                }
            }
        }

        if (!WriteFile (pLine->hComm, "ATH\r", 4, &dwNumBytes, &overlapped))
        {
            if ((dwError = GetLastError()) == ERROR_IO_PENDING)
            {
                GetOverlappedResult(
                    pLine->hComm,
                    &overlapped,
                    &dwNumBytes,
                    TRUE
                    );
            }
            else
            {
            }
        }

        CloseHandle (overlapped.hEvent);
        CloseHandle (pLine->hComm);
        pLine->hComm = NULL;
    }
}

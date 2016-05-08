/*++ 

Copyright (c) 1995 - 2000  Microsoft Corporation

Module Name:

    perfgen.c

Abstract:

    This file implements an Extensible Performance Object that displays
    generated signals

Created:    

    Bob Watson  28-Jul-1995

Revision History


--*/

//
//  Include Files
//

#include <windows.h>
#include <string.h>
#include <winperf.h>
#include <math.h>
#include "genctrs.h" // error message definition
#include "perfmsg.h"
#include "perfutil.h"
#include "datagen.h"

//  define constant value counter's value here, any number will do.

#define CONSTANT_VALUE_VALUE    49
//
//  References to constants which initialize the Object type definitions
//

extern SIGGEN_DATA_DEFINITION SigGenDataDefinition;
    
DWORD  dwOpenCount = 0;        // count of "Open" threads
BOOL   bInitOK     = FALSE;    // true = DLL initialized OK

//
//  Function Prototypes
//
//      these are used to insure that the data collection functions
//      accessed by Perflib will have the correct calling format.
//

PM_OPEN_PROC    OpenSigGenPerformanceData;
PM_COLLECT_PROC CollectSigGenPerformanceData;
PM_CLOSE_PROC   CloseSigGenPerformanceData;

typedef struct _WAVE_DATA {
    DWORD   dwPeriod;
    DWORD   dwAmplitude;
    LPWSTR  szInstanceName;
} WAVE_DATA, * PWAVE_DATA;

static WAVE_DATA wdInstance[]  =
{
    {1000,      100, L"  1 Second"},
    {10000,     100, L" 10 Second"},
    {20000,     100, L" 20 Second"},
    {50000,     100, L" 50 Second"},
    {100000,    100, L"100 Second"}
};

static const DWORD NUM_INSTANCES = (sizeof(wdInstance) / sizeof(wdInstance[0]));
static double      dPi           = 3.1415926590;
static double      d2Pi          = 2.0 * 3.1415926590;

static
DWORD
GetTimeInMilliSeconds()
{
    SYSTEMTIME  st;
    DWORD       dwReturn;

    GetSystemTime (& st);
    dwReturn  = (DWORD) st.wMilliseconds;
    dwReturn += (DWORD) st.wSecond *  1000L;
    dwReturn += (DWORD) st.wMinute * 60000L;
    dwReturn += (DWORD) st.wHour * 3600000L;
    dwReturn += (DWORD) st.wDay * 86500000L;

    // that's good enough for what it's for

    return dwReturn;
}

DWORD APIENTRY
OpenSigGenPerformanceData(
    LPWSTR lpDeviceNames
)

/*++

Routine Description:

    This routine will initialize the data structures used to pass
    data back to the registry

Arguments:

    Pointer to object ID of each device to be opened (PerfGen)

Return Value:

    None.

--*/

{
    LONG  status;
    HKEY  hKeyDriverPerf = NULL;
    DWORD size;
    DWORD type;
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;

    //
    //  Since WINLOGON is multi-threaded and will call this routine in
    //  order to service remote performance queries, this library
    //  must keep track of how many times it has been opened (i.e.
    //  how many threads have accessed it). the registry routines will
    //  limit access to the initialization routine to only one thread 
    //  at a time so synchronization (i.e. reentrancy) should not be 
    //  a problem
    //

    if (! dwOpenCount) {
        // open Eventlog interface

        hEventLog = MonOpenEventLog();

        // get counter and help index base values from registry
        //      Open key to registry entry
        //      read First Counter and First Help values
        //      update static data strucutures by adding base to 
        //          offset value in structure.

        status = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        "SYSTEM\\CurrentControlSet\\Services\\PerfGen\\Performance",
                        0L,
                        KEY_READ,
                        & hKeyDriverPerf);
        if (status != ERROR_SUCCESS) {
            REPORT_ERROR_DATA (GENPERF_UNABLE_OPEN_DRIVER_KEY, LOG_USER,
                &status, sizeof(status));
            // this is fatal, if we can't get the base values of the 
            // counter or help names, then the names won't be available
            // to the requesting application  so there's not much
            // point in continuing.
            goto OpenExitPoint;
        }

        size = sizeof(DWORD);
        status = RegQueryValueEx(
                        hKeyDriverPerf, 
                        "First Counter",
                        0L,
                        & type,
                        (LPBYTE) & dwFirstCounter,
                        & size);
        if (status != ERROR_SUCCESS) {
            REPORT_ERROR_DATA (GENPERF_UNABLE_READ_FIRST_COUNTER, LOG_USER,
                &status, sizeof(status));
            // this is fatal, if we can't get the base values of the 
            // counter or help names, then the names won't be available
            // to the requesting application  so there's not much
            // point in continuing.
            goto OpenExitPoint;
        }

        size = sizeof(DWORD);
        status = RegQueryValueEx(
                        hKeyDriverPerf, 
                        "First Help",
                        0L,
                        & type,
                        (LPBYTE) & dwFirstHelp,
                        & size);
        if (status != ERROR_SUCCESS) {
            REPORT_ERROR_DATA (GENPERF_UNABLE_READ_FIRST_HELP, LOG_USER,
                &status, sizeof(status));
            // this is fatal, if we can't get the base values of the 
            // counter or help names, then the names won't be available
            // to the requesting application  so there's not much
            // point in continuing.
            goto OpenExitPoint;
        }
 
        //
        //  NOTE: the initialization program could also retrieve
        //      LastCounter and LastHelp if they wanted to do 
        //      bounds checking on the new number. e.g.
        //
        //      counter->CounterNameTitleIndex += dwFirstCounter;
        //      if (counter->CounterNameTitleIndex > dwLastCounter) {
        //          LogErrorToEventLog (INDEX_OUT_OF_BOUNDS);
        //      }

        SigGenDataDefinition.SigGenObjectType.ObjectNameTitleIndex  += dwFirstCounter;
        SigGenDataDefinition.SigGenObjectType.ObjectHelpTitleIndex  += dwFirstHelp;

        // assign index of default counter (Sine Wave)
        SigGenDataDefinition.SigGenObjectType.DefaultCounter         = 0;
        SigGenDataDefinition.SineWaveDef.CounterNameTitleIndex      += dwFirstCounter;
        SigGenDataDefinition.SineWaveDef.CounterHelpTitleIndex      += dwFirstHelp;
        SigGenDataDefinition.TriangleWaveDef.CounterNameTitleIndex  += dwFirstCounter;
        SigGenDataDefinition.TriangleWaveDef.CounterHelpTitleIndex  += dwFirstHelp;
        SigGenDataDefinition.SquareWaveDef.CounterNameTitleIndex    += dwFirstCounter;
        SigGenDataDefinition.SquareWaveDef.CounterHelpTitleIndex    += dwFirstHelp;
        SigGenDataDefinition.ConstantValueDef.CounterNameTitleIndex += dwFirstCounter;
        SigGenDataDefinition.ConstantValueDef.CounterHelpTitleIndex += dwFirstHelp;

        bInitOK = TRUE; // ok to use this function
    }

    dwOpenCount ++;  // increment OPEN counter

    status = ERROR_SUCCESS; // for successful exit

OpenExitPoint:
    if (hKeyDriverPerf != NULL) RegCloseKey(hKeyDriverPerf);
    return status;
}

DWORD APIENTRY
CollectSigGenPerformanceData(
    IN      LPWSTR    lpValueName,
    IN OUT  LPVOID  * lppData,
    IN OUT  LPDWORD   lpcbTotalBytes,
    IN OUT  LPDWORD   lpNumObjectTypes
)
/*++

Routine Description:

    This routine will return the data for the Signal Generator counters.

Arguments:

   IN       LPWSTR   lpValueName
         pointer to a wide character string passed by registry.

   IN OUT   LPVOID   *lppData
         IN: pointer to the address of the buffer to receive the completed 
            PerfDataBlock and subordinate structures. This routine will
            append its data to the buffer starting at the point referenced
            by *lppData.
         OUT: points to the first byte after the data structure added by this
            routine. This routine updated the value at lppdata after appending
            its data.

   IN OUT   LPDWORD  lpcbTotalBytes
         IN: the address of the DWORD that tells the size in bytes of the 
            buffer referenced by the lppData argument
         OUT: the number of bytes added by this routine is writted to the 
            DWORD pointed to by this argument

   IN OUT   LPDWORD  NumObjectTypes
         IN: the address of the DWORD to receive the number of objects added 
            by this routine 
         OUT: the number of objects added by this routine is writted to the 
            DWORD pointed to by this argument

Return Value:

      ERROR_MORE_DATA if buffer passed is too small to hold data
         any error conditions encountered are reported to the event log if
         event logging is enabled.

      ERROR_SUCCESS  if success or any other error. Errors, however are
         also reported to the event log.

--*/
{
    //  Variables for reformating the data

    PERF_INSTANCE_DEFINITION * pPerfInstanceDefinition;
    DWORD   dwThisInstance;
    ULONG   SpaceNeeded;
    SIGGEN_DATA_DEFINITION   * pSigGenDataDefinition;
    DWORD   dwQueryType;
    DWORD   dwTime;
    DWORD   dwPhase;
    double  dPhase, dSin;
    LONG    lValue;
    PSIGGEN_COUNTER   pSC;

    //
    // before doing anything else, see if Open went OK
    //
    if (! bInitOK) {
        // unable to continue because open failed.
        * lpcbTotalBytes   = (DWORD) 0;
        * lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS; // yes, this is a successful exit
    }
    
    // see if this is a foreign (i.e. non-NT) computer data request 
    //
    dwQueryType = GetQueryType(lpValueName);
    
    if ((dwQueryType == QUERY_FOREIGN) || (dwQueryType == QUERY_COSTLY)) {
        // this routine does not service requests for data from
        // Non-NT computers
        * lpcbTotalBytes   = (DWORD) 0;
        * lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS) {
    if (! (IsNumberInUnicodeList(SigGenDataDefinition.SigGenObjectType.ObjectNameTitleIndex, lpValueName))) {
            // request received for data object not provided by this routine
            * lpcbTotalBytes   = (DWORD) 0;
            * lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

    pSigGenDataDefinition = (SIGGEN_DATA_DEFINITION *) * lppData;

    SpaceNeeded = sizeof(SIGGEN_DATA_DEFINITION) +
                    (NUM_INSTANCES * (sizeof(PERF_INSTANCE_DEFINITION) +
                    (24) +    // size of instance names
                    sizeof(SIGGEN_COUNTER)));

    if (* lpcbTotalBytes < SpaceNeeded) {
        * lpcbTotalBytes   = (DWORD) 0;
        * lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

    // Get current time for this sample
    //
    dwTime = GetTimeInMilliSeconds();
    //
    // Copy the (constant, initialized) Object Type and counter definitions
    //  to the caller's data buffer
    //
    memmove(pSigGenDataDefinition, & SigGenDataDefinition, sizeof(SIGGEN_DATA_DEFINITION));
    //
    //    Create data for return for each instance
    //
    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *) & pSigGenDataDefinition[1];

    for (dwThisInstance = 0; dwThisInstance < NUM_INSTANCES; dwThisInstance ++) {
        MonBuildInstanceDefinition(
                        pPerfInstanceDefinition,
                        (PVOID *) & pSC,
                        0,
                        0,
                        (DWORD) -1, // use name
                        wdInstance[dwThisInstance].szInstanceName);

        pSC->CounterBlock.ByteLength = sizeof (SIGGEN_COUNTER);

        //**********************************************************
        //
        // for this particular example, the data is "created" here.
        // normally it would be read from the appropriate device or 
        // application program.
        //
        //**********************************************************
    
        // comput phase for this instance period
        dwPhase = dwTime % wdInstance[dwThisInstance].dwPeriod;
        //
        // compute sinewave value here
        //
        dPhase  = (double) dwPhase / (double) wdInstance[dwThisInstance].dwPeriod;
        dPhase *= d2Pi;

        // the cosine function is used to keep the phase aligned with the
        // other wave forms
        dSin = -cos(dPhase);
        // adjust amplitude and add .5 to round to integer correctly
        dSin *= (double)((wdInstance[dwThisInstance].dwAmplitude) / 2.0) + 0.5;
    
        lValue  = (LONG) dSin;
        lValue += wdInstance[dwThisInstance].dwAmplitude / 2;   // to move negative values above 0

        // save sine value
        pSC->dwSineWaveValue = (DWORD) lValue;

        // compute triangle wave value here

        if (dwPhase < (wdInstance[dwThisInstance].dwPeriod / 2)) {
            lValue = (LONG)((dwPhase * wdInstance[dwThisInstance].dwAmplitude) / (wdInstance[dwThisInstance].dwPeriod / 2));
        } else {
            lValue = (LONG)(((wdInstance[dwThisInstance].dwPeriod - dwPhase) * wdInstance[dwThisInstance].dwAmplitude) /
                    (wdInstance[dwThisInstance].dwPeriod / 2));
        }
        // save triangle value
        pSC->dwTriangleWaveValue = (DWORD) lValue;

        //
        //  compute square wave value
        //
        if (dwPhase <= (wdInstance[dwThisInstance].dwPeriod / 2)) {
            lValue = 0;
        } else {
            lValue = (LONG)wdInstance[dwThisInstance].dwAmplitude;
        }
        // save square value
        pSC->dwSquareWaveValue = (DWORD) lValue;

        // finally the constant value (same fore every instance)
        pSC->dwConstantValue = (DWORD)CONSTANT_VALUE_VALUE;

        // update instance pointer for next instance
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pSC[1];
    }
    // update arguments for return

    * lppData = (PVOID) pPerfInstanceDefinition;
    
    * lpNumObjectTypes = 1;

    pSigGenDataDefinition->SigGenObjectType.TotalByteLength = 
                    * lpcbTotalBytes = (DWORD) ((PBYTE) pPerfInstanceDefinition - (PBYTE) pSigGenDataDefinition);

    // update instance count
    pSigGenDataDefinition->SigGenObjectType.NumInstances = NUM_INSTANCES;
    
    return ERROR_SUCCESS;
}

DWORD APIENTRY
CloseSigGenPerformanceData(
)

/*++

Routine Description:

    This routine closes the open handles to the Signal Gen counters.

Arguments:

    None.


Return Value:

    ERROR_SUCCESS

--*/

{
    if (! (-- dwOpenCount)) { // when this is the last thread...
        MonCloseEventLog();
    }
    return ERROR_SUCCESS;

}

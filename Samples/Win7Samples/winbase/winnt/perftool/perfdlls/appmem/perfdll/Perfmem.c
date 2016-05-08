/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1995 - 2000.  Microsoft Corporation.  All rights reserved.


Module Name:

    perfmem.c

Abstract:

    This file implements the Extensible Objects for  the application
    memory object

--*/
//
//  Include Files
//
#include <windows.h>
#include <string.h>
#include <winperf.h>
#include "..\inc\appmemi.h"     // definitions shared with the app. dll
#include "memctrs.h"            // error message definition
#include "perfmsg.h"            // message utilities
#include "perfutil.h"           // perf data utilities
#include "datamem.h"            // perf data structure definitions

//
//  References to constants which initialize the Object type definitions
//
extern APPMEM_DATA_DEFINITION AppMemDataDefinition;

static DWORD  dwOpenCount = 0;        // count of "Open" threads
static BOOL   bInitOK     = FALSE;    // true = DLL initialized OK

//
// App Mem counter data structures
//

static  HANDLE  hAppMemSharedMemory = NULL; // Handle of counter Shared Memory

static  PAPPMEM_DATA_HEADER pDataHeader = NULL; // pointer to header of shared mem
//
//  Function Prototypes
//
//      these are used to insure that the data collection functions
//      accessed by Perflib will have the correct calling format.
//

PM_OPEN_PROC    OpenAppMemPerformanceData;
PM_COLLECT_PROC CollectAppMemPerformanceData;
PM_CLOSE_PROC   CloseAppMemPerformanceData;

DWORD APIENTRY
OpenAppMemPerformanceData(
    LPWSTR lpDeviceNames
)
/*++

Routine Description:

    This routine will open and map the memory used by the App Mem DLL to
    pass performance data in. This routine also initializes the data
    structures used to pass data back to the registry

Arguments:

    Pointer to object ID of each device to be opened

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
   APPMEM_COUNTERS     ac;

   //
   //  Since REGSVC is multi-threaded and will call this routine in
   //  order to service remote performance queries, this library
   //  must keep track of how many times it has been opened (i.e.
   //  how many threads have accessed it). the registry routines will
   //  limit access to the initialization routine to only one thread
   //  at a time so synchronization (i.e. reentrancy) should not be
   //  a problem
   //

   if (dwOpenCount == 0)
   { // do this only on the first instance of the DLL
      // open Eventlog interface

      hEventLog = MonOpenEventLog();

      // open/create shared memory used by the application to pass performance values
      status = GetSharedMemoryDataHeader(
                                        & hAppMemSharedMemory, & pDataHeader,
                                        TRUE); // only read access is required
      if (status != ERROR_SUCCESS)
      {
         REPORT_ERROR_DATA (status, LOG_USER, NULL, 0);
         goto OpenExitPoint;
      }

      // get counter and help index base values from registry
      //      Open key to registry entry
      //      read First Counter and First Help values
      //      update static data strucutures by adding base to
      //          offset value in structure.

      status = RegOpenKeyEx(
                           HKEY_LOCAL_MACHINE,
                           "SYSTEM\\CurrentControlSet\\Services\\AppMem\\Performance",
                           0L,
                           KEY_READ,
                           & hKeyDriverPerf);
      if (status != ERROR_SUCCESS)
      {
         REPORT_ERROR_DATA(APPMEM_UNABLE_OPEN_DRIVER_KEY, LOG_USER, & status, sizeof(status));
         // this is fatal, if we can't get the base values of the
         // counter or help names, then the names won't be available
         // to the requesting application  so there's not much
         // point in continuing.
         goto OpenExitPoint;
      }

      size   = sizeof(DWORD);
      status = RegQueryValueEx(
                              hKeyDriverPerf,
                              "First Counter",
                              0L,
                              & type,
                              (LPBYTE) & dwFirstCounter,
                              & size);
      if (status != ERROR_SUCCESS)
      {
         REPORT_ERROR_DATA(APPMEM_UNABLE_READ_FIRST_COUNTER, LOG_USER, & status, sizeof(status));
         // this is fatal, if we can't get the base values of the
         // counter or help names, then the names won't be available
         // to the requesting application  so there's not much
         // point in continuing.
         goto OpenExitPoint;
      }

      size   = sizeof(DWORD);
      status = RegQueryValueEx(
                              hKeyDriverPerf,
                              "First Help",
                              0L,
                              & type,
                              (LPBYTE) & dwFirstHelp,
                              & size);
      if (status != ERROR_SUCCESS)
      {
         REPORT_ERROR_DATA(APPMEM_UNABLE_READ_FIRST_HELP, LOG_USER, & status, sizeof(status));
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

      AppMemDataDefinition.AppMemObjectType.ObjectNameTitleIndex      += dwFirstCounter;
      AppMemDataDefinition.AppMemObjectType.ObjectHelpTitleIndex      += dwFirstHelp;
      AppMemDataDefinition.AppMemBytesAllocated.CounterNameTitleIndex += dwFirstCounter;
      AppMemDataDefinition.AppMemBytesAllocated.CounterHelpTitleIndex += dwFirstHelp;
      AppMemDataDefinition.AppMemBytesAllocated.CounterOffset         +=
                                      (DWORD) ((LPBYTE) & ac.dwAppMemBytesAllocated - (LPBYTE) & ac);
      AppMemDataDefinition.AppMemAllocs.CounterNameTitleIndex         += dwFirstCounter;
      AppMemDataDefinition.AppMemAllocs.CounterHelpTitleIndex         += dwFirstHelp;
      AppMemDataDefinition.AppMemAllocs.CounterOffset                  =
                                      (DWORD) ((LPBYTE) & ac.dwAppMemAllocs - (LPBYTE) & ac);
      AppMemDataDefinition.AppMemAllocsSec.CounterNameTitleIndex      += dwFirstCounter;
      AppMemDataDefinition.AppMemAllocsSec.CounterHelpTitleIndex      += dwFirstHelp;
      AppMemDataDefinition.AppMemAllocsSec.CounterOffset               =
                                      (DWORD) ((LPBYTE) & ac.dwAppMemAllocsSec - (LPBYTE) & ac);
      AppMemDataDefinition.AppMemReAllocs.CounterNameTitleIndex       += dwFirstCounter;
      AppMemDataDefinition.AppMemReAllocs.CounterHelpTitleIndex       += dwFirstCounter;
      AppMemDataDefinition.AppMemReAllocs.CounterOffset                =
                                      (DWORD) ((LPBYTE) & ac.dwAppMemReAllocs - (LPBYTE) & ac);
      AppMemDataDefinition.AppMemReAllocsSec.CounterNameTitleIndex    += dwFirstCounter;
      AppMemDataDefinition.AppMemReAllocsSec.CounterHelpTitleIndex    += dwFirstHelp;
      AppMemDataDefinition.AppMemReAllocsSec.CounterOffset             =
                                      (DWORD) ((LPBYTE) & ac.dwAppMemReAllocsSec - (LPBYTE) & ac);
      AppMemDataDefinition.AppMemFrees.CounterNameTitleIndex          += dwFirstCounter;
      AppMemDataDefinition.AppMemFrees.CounterHelpTitleIndex          += dwFirstHelp;
      AppMemDataDefinition.AppMemFrees.CounterOffset                   =
                                      (DWORD) ((LPBYTE) & ac.dwAppMemFrees - (LPBYTE) & ac);
      AppMemDataDefinition.AppMemFreesSec.CounterNameTitleIndex       += dwFirstCounter;
      AppMemDataDefinition.AppMemFreesSec.CounterHelpTitleIndex       += dwFirstHelp;
      AppMemDataDefinition.AppMemFreesSec.CounterOffset                =
                                      (DWORD) ((LPBYTE) & ac.dwAppMemFreesSec - (LPBYTE) & ac);

      bInitOK = TRUE; // ok to use this function
   }

   dwOpenCount ++;         // increment OPEN counter
   status = ERROR_SUCCESS; // for successful exit

OpenExitPoint:
   if (hKeyDriverPerf != NULL) RegCloseKey(hKeyDriverPerf); // close key to registry

   return status;
}

DWORD APIENTRY
CollectAppMemPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
/*++

Routine Description:

    This routine will return the data for the Application memory counters

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

   ULONG                       SpaceNeeded;
   PERF_INSTANCE_DEFINITION  * pPerfInstanceDef;
   APPMEM_DATA_DEFINITION    * pAppMemDataDefinition;
   DWORD                       dwQueryType;
   DWORD                       dwInstanceIndex;
   PAPPMEM_INSTANCE            pThisAppInstanceData = NULL;
   PAPPMEM_COUNTERS            pAC;

   //
   // before doing anything else, see if Open went OK
   //
   if ((!bInitOK) || (pDataHeader == NULL))
   {
      // unable to continue because open failed.
      * lpcbTotalBytes   = (DWORD) 0;
      * lpNumObjectTypes = (DWORD) 0;
      return ERROR_SUCCESS; // yes, this is a successful exit
   }

   // see if this is a foreign (i.e. non-NT) computer data request
   //
   dwQueryType = GetQueryType(lpValueName);
   if (dwQueryType == QUERY_FOREIGN)
   {
      // this routine does not service requests for data from
      // Non-NT computers
      * lpcbTotalBytes   = (DWORD) 0;
      * lpNumObjectTypes = (DWORD) 0;
      return ERROR_SUCCESS;
   }

   if (dwQueryType == QUERY_ITEMS)
   {
      if (! (IsNumberInUnicodeList (AppMemDataDefinition.AppMemObjectType.ObjectNameTitleIndex, lpValueName)))
      {
         // request received for data object not provided by this routine
         * lpcbTotalBytes   = (DWORD) 0;
         * lpNumObjectTypes = (DWORD) 0;
         return ERROR_SUCCESS;
      }
   }

   // if here, then this must be one for us so load data structures

   pAppMemDataDefinition = (APPMEM_DATA_DEFINITION *) *lppData;

   // always return an "instance sized" buffer after the definition blocks
   // to prevent perfmon from reading bogus data. This is strictly a hack
   // to accomodate how PERFMON handles the "0" instance case.
   //  By doing this, perfmon won't choke when there are no instances
   //  and the counter object & counters will be displayed in the
   //  list boxes, even though no instances will be listed.

   SpaceNeeded = sizeof(APPMEM_DATA_DEFINITION) +
                 ((pDataHeader->dwInstanceCount > 0 ? pDataHeader->dwInstanceCount : 1) * (
                         sizeof(PERF_INSTANCE_DEFINITION) + MAX_SIZEOF_INSTANCE_NAME + sizeof(APPMEM_COUNTERS)));

   if (* lpcbTotalBytes < SpaceNeeded)
   {
      // not enough room so return nothing.
      * lpcbTotalBytes   = (DWORD) 0;
      * lpNumObjectTypes = (DWORD) 0;
      return ERROR_MORE_DATA;
   }

   // copy the object & counter definition information

   memmove(pAppMemDataDefinition, & AppMemDataDefinition, sizeof(APPMEM_DATA_DEFINITION));

   // prepare to read the instance data
   pPerfInstanceDef = (PERF_INSTANCE_DEFINITION *) & pAppMemDataDefinition[1];

   // point to the first instance structure in the shared buffer
   pThisAppInstanceData = FIRST_INUSE(pDataHeader);

   // process each of the instances in the shared memory buffer
   dwInstanceIndex = 0;
   while ((dwInstanceIndex < pDataHeader->dwInstanceCount) && (pThisAppInstanceData != NULL))
   {
      // initialize this instance
      MonBuildInstanceDefinition(
                                pPerfInstanceDef,
                                & pAC, // pointer to first byte after instance def
                                0,     // no parent
                                0,     //  object to reference
                                (DWORD) PERF_NO_UNIQUE_ID,
                                pThisAppInstanceData->wcszInstanceName);
      //
      //    collect and format app memory perf data from shared memory
      //

      pAC->CounterBlock.ByteLength = sizeof(APPMEM_COUNTERS);

      // set pointer to first counter data field

      pAC->dwAppMemBytesAllocated = pThisAppInstanceData->dwApplicationBytes;
      pAC->dwAppMemAllocs = pThisAppInstanceData->dwAllocCalls;
      pAC->dwAppMemAllocsSec = pThisAppInstanceData->dwAllocCalls;
      pAC->dwAppMemReAllocs = pThisAppInstanceData->dwReAllocCalls;
      pAC->dwAppMemReAllocsSec = pThisAppInstanceData->dwReAllocCalls;
      pAC->dwAppMemFrees = pThisAppInstanceData->dwFreeCalls;
      pAC->dwAppMemFreesSec = pThisAppInstanceData->dwFreeCalls;

      // setup for the next instance
      dwInstanceIndex++;
      pThisAppInstanceData =
      APPMEM_INST(pDataHeader, pThisAppInstanceData->dwOffsetOfNext);
      pPerfInstanceDef =
      (PERF_INSTANCE_DEFINITION *)((LPBYTE)pAC + sizeof(APPMEM_COUNTERS));
   }

   if (dwInstanceIndex == 0)
   {
      // zero fill one instance sized block of data if there are no
      // data instances

      memset(pPerfInstanceDef, 0,
              (sizeof(PERF_INSTANCE_DEFINITION) + MAX_SIZEOF_INSTANCE_NAME + sizeof(APPMEM_COUNTERS)));

      // adjust pointer to point to end of zeroed block
      (BYTE *) pPerfInstanceDef += (sizeof(PERF_INSTANCE_DEFINITION)
                                 + MAX_SIZEOF_INSTANCE_NAME + sizeof(APPMEM_COUNTERS));
   }

   // done with the shared memory so free the mutex if one was
   // acquired

   * lppData = (PVOID) pPerfInstanceDef;

   // update arguments for return

   * lpNumObjectTypes = 1;
   pAppMemDataDefinition->AppMemObjectType.NumInstances = dwInstanceIndex;

   pAppMemDataDefinition->AppMemObjectType.TotalByteLength =
   * lpcbTotalBytes = (DWORD)((PBYTE) pPerfInstanceDef - (PBYTE) pAppMemDataDefinition);

   return ERROR_SUCCESS;
}

DWORD APIENTRY
CloseAppMemPerformanceData(
)
/*++

Routine Description:

    This routine closes the open handles to Application Memory
        usage performance counters

Arguments:

    None.


Return Value:

    ERROR_SUCCESS

--*/
{
   if ((-- dwOpenCount) == 0)
   { // when this is the last thread...
      if (hAppMemSharedMemory != NULL) CloseHandle(hAppMemSharedMemory);
      pDataHeader = NULL;
      MonCloseEventLog();
   }

   return ERROR_SUCCESS;

}

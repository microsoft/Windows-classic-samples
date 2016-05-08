// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*++

Module Name:

    Diagevents.c

Abstract:

    This file contains sample code for a command line utility which dumps the 
    diagnostic events using the Windows Filtering Platform(WFP) API NDF 
    Helper Class Functions.

--*/

#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fwpmtypes.h>
#include <fwpmu.h>
#include "wfpeventutil.h"

#ifdef __cplusplus
}
#endif

#define NUM_FILTER_CONDITIONS 4
#define MIN_FACTOR 600000000

#define CHECK_RESULT(result) {\
   if (ERROR_SUCCESS != result) \
      goto cleanup;\
}

DWORD
GetMatchingEvents(
   __in UINT32 mins,
   __in ADDRESS_FAMILY version,
   __in_opt SOCKADDR_STORAGE* remoteAddr,
   __out FWPM_NET_EVENT0*** matchedEvents,
   __out UINT32* numMatchedEvents
)
/*++

Routine Description:

     This routine creates and fills an event enumeration template, and 
     retrieve events matching the template using WFP API. The diagnostic
     events API is used only in this function

Arguments:

    mins - period in minutes for which events should be retrieved going 
            back from current time.

	version - IP version 

    remoteAddr - pointer to IP address which should match the remote 
            address in events.

    matchedEvents - pointer to an array of pointers to events that have 
            period and/or remote address matching those specified.

    numMatchedEvents - pointer to the number of events matching the
            criterion.

Return Value:

    DWORD - If the function succeeds, the return value is ERROR_SUCCESS.

--*/
{
   DWORD result = ERROR_SUCCESS;
   UINT32 filterCondCount = 0;
   FWPM_NET_EVENT_ENUM_TEMPLATE0 eventTemplate;
   FWPM_FILTER_CONDITION0 filterConditions[NUM_FILTER_CONDITIONS] = {0};
   HANDLE fwpEngineHandle = NULL;
   HANDLE enumHandle = NULL;
   BOOLEAN engineOpen = FALSE;
   SYSTEMTIME curTimeSys;

   //-----------------------------------------
   // The period in which the event happened is set to current sytem time.
   GetSystemTime(&curTimeSys);
   if (!(SystemTimeToFileTime(&curTimeSys, &(eventTemplate.endTime))))
   {
      wprintf(L"Error in converting system time to file time.\n");
      result = GetLastError();
      CHECK_RESULT(result);
   }

   //-----------------------------------------
   // Set the lower end of the period in which events are of interest
   if (0 == mins) 
   {
      eventTemplate.startTime.dwLowDateTime = 0;
      eventTemplate.startTime.dwHighDateTime = 0; 
   } 
   else 
   {
      ULARGE_INTEGER filetime;
      ULONGLONG period;
      
      filetime.LowPart = eventTemplate.endTime.dwLowDateTime;
      filetime.HighPart = eventTemplate.endTime.dwHighDateTime;
      period = ((ULONGLONG)mins)*MIN_FACTOR;
      if (filetime.QuadPart > period)
      {
         filetime.QuadPart -= period;
      }
      else
      {
         filetime.QuadPart = 0;
      }
      eventTemplate.startTime.dwLowDateTime  = filetime.LowPart;
      eventTemplate.startTime.dwHighDateTime = filetime.HighPart;
   }

   //-----------------------------------------
   // Set the filter condition to look for events matching the remote address
   if (remoteAddr)
   {
	   if (AF_INET == version)
	   {
		   SOCKADDR_IN* ipV4Addr = (SOCKADDR_IN*)remoteAddr;
	   
		  filterConditions[filterCondCount].fieldKey = 
											  FWPM_CONDITION_IP_REMOTE_ADDRESS;
		  filterConditions[filterCondCount].matchType = FWP_MATCH_EQUAL;
		  filterConditions[filterCondCount].conditionValue.type = FWP_UINT32;
		  filterConditions[filterCondCount].conditionValue.uint32 = 
											  ntohl(ipV4Addr->sin_addr.s_addr);
		  filterCondCount++;
	   }
	   else if (AF_INET6 == version)
	   {
		   SOCKADDR_IN6* ipV6Addr = (SOCKADDR_IN6*)remoteAddr;
	   
		  filterConditions[filterCondCount].fieldKey = 
							FWPM_CONDITION_IP_REMOTE_ADDRESS;
		  filterConditions[filterCondCount].matchType = FWP_MATCH_EQUAL;
		  filterConditions[filterCondCount].conditionValue.type = 
							FWP_BYTE_ARRAY16_TYPE;
		  filterConditions[filterCondCount].conditionValue.byteArray16 = 
							(FWP_BYTE_ARRAY16 *)(ipV6Addr->sin6_addr.s6_bytes);
		  filterCondCount++;
	   }
   }

   //-----------------------------------------
   // Fill the event enumeration template
   eventTemplate.numFilterConditions = filterCondCount;
   eventTemplate.filterCondition = filterConditions;

   //-----------------------------------------
   // Open a session to the filter engine
   result = FwpmEngineOpen0(
            NULL, 
            RPC_C_AUTHN_WINNT, 
            NULL,
            NULL, 
            &fwpEngineHandle
            );
   CHECK_RESULT(result);

   //-----------------------------------------
   // Create an enumeration handle based on event template
   result = FwpmNetEventCreateEnumHandle0(
            fwpEngineHandle,
            &eventTemplate,
            &enumHandle
            );
   CHECK_RESULT(result);

   //-----------------------------------------
   // Get the events from enumeration
   result = FwpmNetEventEnum0(
            fwpEngineHandle,
            enumHandle, 
            INFINITE, 
            matchedEvents, 
            numMatchedEvents
            );
   CHECK_RESULT(result);
   //
   // fallthrough
   //
cleanup:
   //-----------------------------------------
   // Destroy enumeration handle
   if (enumHandle)   
   {
      FwpmNetEventDestroyEnumHandle0(fwpEngineHandle, enumHandle);
   }
   
   //-----------------------------------------
   // Close session after destroying enumeration handle
   if (fwpEngineHandle)   
   {
      FwpmEngineClose0(fwpEngineHandle);
   }
   
   return result;
}


int __cdecl wmain(int argc, const wchar_t* const argv[])
{
   DWORD result = ERROR_SUCCESS;
   UINT32 mins = 0;
   UINT32 num = 0;
   ADDRESS_FAMILY version;
   SOCKADDR_STORAGE remoteAddr = {0};
   FWPM_NET_EVENT0** matchedEvents;
   BOOL stop = FALSE;
   WSADATA wsaData;

   //-----------------------------------------
   // Initialize Winsock
   if (WSAStartup(MAKEWORD(2,2), &wsaData))
   {
      wprintf(L"Winsock library initialization failed\n");
      result = E_UNEXPECTED;
      CHECK_RESULT(result);
   }

   //-----------------------------------------
   // Parse and Validate the command line arguments
   result = ParseArguments(
               argc, 
               argv,
               &mins,
			   &version,
               &remoteAddr,
               &stop
               );
   CHECK_RESULT(result);
   if (stop)
   {
      goto cleanup;
   }

   //-----------------------------------------
   // Get events matching the arguments
   result = GetMatchingEvents(
               mins,
			   version,
               &remoteAddr,
               &matchedEvents,
               &num);

   CHECK_RESULT(result);

   if (num == 0)
   {
      wprintf(L"No events matching criterion\n");
      goto cleanup;
   }

   if (matchedEvents == NULL)
   {
      wprintf(L"Number of events is not zero. But event pointer NULL\n");
      result = ERROR_INVALID_DATA;
      goto cleanup;
   }
      
   //-----------------------------------------
   // Print event details of all events matching criterion
   PrintEvents(matchedEvents, num);

   //-----------------------------------------
   // Free memory allocated by engine for enumerated events
   FwpmFreeMemory0((VOID**)&matchedEvents);
   
   cleanup:	
	  WSACleanup();
      return result;
}

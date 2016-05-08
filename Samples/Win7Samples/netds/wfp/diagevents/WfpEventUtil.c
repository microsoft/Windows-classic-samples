// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*++

Module Name:

    WfpEventUtil.c

Abstract:

    This file contains utility routines for managing program input through
    command line arguments and printing program output in form of diagnostic
    event details.

--*/

#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <wchar.h>
#include <winsock2.h>
#include <stdlib.h>
#include <windows.h>
#include <sddl.h>
#include <fwpmtypes.h>
#include "wfpeventutil.h"

//Max length of IPv4 address as a string
#define ADDR_STR_LEN 64

//Buffer length for date and time strings
#define STR_BUF_SIZE 256

// Static array containing string description of peer that failed
static const wchar_t* const  mFailurePoint[] =
{
   L"None",
   L"Local",
   L"Remote"
};

// Static array containing string description of keying module
static const wchar_t* const  mKeyModuleType[] =
{
   L"IKE",
   L"AuthIP"
};

// Static array containing string description of IPsec traffic type.
static const wchar_t* const  mTrafficType[] = 
{
   L"Transport mode",
   L"Tunnel mode"
};

// Static array containing string description of IPsec peer role
static const wchar_t* const  mSARole[] = 
{
   L"Initiator",
   L"Responder"
};

// Static array containing string description of authentication method
static const wchar_t* const  mAuthmethodtype[]=
{
      L"Preshared Key",
      L"Certificate",
      L"Kerberos",
      L"Anonymous",
      L"SSL",
      L"NTLMv2",
      L"CGA",
      L"Auth type max"
};

// Static array containing string description of reason for packet drop.
static const wchar_t* const   mDropReason[] = 
{
   L"Classify drop",
   L"IPsec drop"
};

/////////////////////////////////
//Private function declarations
/////////////////////////////////

// Functions to print parts of event
VOID 
PrintEventHeader(
   __in FWPM_NET_EVENT0* pEvent
);

VOID 
PrintMMEvent(
   __in FWPM_NET_EVENT0* pEvent
);

VOID 
PrintQMEvent(
   __in FWPM_NET_EVENT0* pEvent
);

VOID
PrintEMEvent(
   __in FWPM_NET_EVENT0* pEvent
);

VOID
PrintClassifyEvent(
   __in FWPM_NET_EVENT0* pEvent
);

VOID
PrintKernelDropEvent(
   __in FWPM_NET_EVENT0* pEvent
);

VOID 
Usage(
   __in const wchar_t* name
);

/////////////////////////////////
// Public functions definitions
/////////////////////////////////

VOID
PrintEvents(   
   __in FWPM_NET_EVENT0** matchedEvents,
   __in UINT32 numMatchedEvents
)
/*++

Routine Description:

     This routine prints the diagnostic event for illustration purposes.

Arguments:

    matchedEvents - Array of pointer to the events which are to be printed
	numMatchedEvents - Number of event pointers in the array.

Return Value:

    None

--*/
{
   UINT32 num = 0;
   FWPM_NET_EVENT0* pEvent;
   
   for (num = 0; num < numMatchedEvents; num++)
   {
      pEvent = matchedEvents[num];

      switch(pEvent->type)
      {
         case FWPM_NET_EVENT_TYPE_IKEEXT_MM_FAILURE:
            PrintMMEvent(pEvent);
            break;
         case FWPM_NET_EVENT_TYPE_IKEEXT_QM_FAILURE : 
            PrintQMEvent(pEvent);
            break;
         case FWPM_NET_EVENT_TYPE_IKEEXT_EM_FAILURE : 
            PrintEMEvent(pEvent);
            break;
         case FWPM_NET_EVENT_TYPE_CLASSIFY_DROP : 
            PrintClassifyEvent(pEvent);
            break;
         case FWPM_NET_EVENT_TYPE_IPSEC_KERNEL_DROP:
            PrintKernelDropEvent(pEvent);
            break;
         default:
            return;
      }   
   }
      
   return;
}

DWORD
ParseArguments(
   __in int argc, 
   __in_ecount(argc) const wchar_t* const argv[],
   __out int* mins,
   __out ADDRESS_FAMILY* version,
   __out SOCKADDR_STORAGE* remoteIpAddr,
   __out BOOL* stop
)
/*++

Routine Description:

    This routine parses the command line arguments to generate inputs for 
	enumeration template.

Arguments:

   argc - Number of command line arguments

	argv - Array of command line arguments

	mins - Period (in minutes) in which events should be looked up (starting 
	from current time.

	version - IP version

	remoteIpAddr - Contains IP address or remote machine.

	stop - Indicates that an error occured while parsing arguments.

Return Value:

   Windows error code indicating the status of the operation. ERROR_SUCCESS
   on success

--*/
{
   DWORD result = ERROR_SUCCESS;
   int i =0;
   int addrSize;
   

   *stop = FALSE;
   *version = AF_UNSPEC;
   for(i=1;i<argc;i++)
   {
      if(argv[i]) 
      {
         if ((argv[i][0] == '-') || (argv[i][0] == '/'))
         {
            switch(argv[i][1])
            {
               case 't' :
                  if (++i >= argc) 
                  {
                     wprintf(L"Unspecified value for option %s\n", argv[--i]);
                     result = ERROR_BAD_ARGUMENTS;
                     goto cleanup;
                  }
                  *mins = _wtoi(argv[i]);
                  if (0>= *mins)
                  {
                     wprintf(L"Minutes should be greater than 0%i\n", *mins);
                     result = ERROR_BAD_ARGUMENTS;
                     goto cleanup;
                  }
                  break;

               case 'v' :
                  if (++i >= argc) 
                  {
                     wprintf(L"Unspecified value for option %s\n", argv[--i]);
                     result = ERROR_BAD_ARGUMENTS;
                     goto cleanup;
                  }
                  
                  switch(_wtoi(argv[i]))
                  {
                     case 4:
                        *version = AF_INET;
                        break;
                     case 6:
                        *version = AF_INET6;
                        break;
                     default:
                        wprintf(L"Version should be either 4 or 6\n");
                        result = ERROR_BAD_ARGUMENTS;
                        goto cleanup;
                        
                  }
                  break;
                  
               case 'r' :
                  if (*version == AF_UNSPEC)
                  {
                     wprintf(L"Specify version before Remote IP Address\n");
                     result = ERROR_BAD_ARGUMENTS;
                     goto cleanup;
                  }
                  
                  if (++i >= argc) 
                  {
                     wprintf(L"Unspecified value for option %s\n", argv[--i]);
                     result = ERROR_BAD_ARGUMENTS;
                     goto cleanup;
                  }

				  addrSize = sizeof(*remoteIpAddr);

                  if (0 != WSAStringToAddress(
                              (LPWSTR)argv[i], 
                              *version,
                              NULL,
                              (SOCKADDR*)remoteIpAddr,
                              &addrSize
                              ))
                  {
                     result =  ERROR_BAD_ARGUMENTS;
                     wprintf(L"IP Address format is incorrect\n");
                     goto cleanup;
                  }
                  break;
                  
               case '?' :
               case 'h' :
                  *stop = TRUE;
		  Usage(argv[0]);
                  goto cleanup;
               default :
                  wprintf(L"Unknown option %s\n",argv[i]);
                  result = ERROR_BAD_ARGUMENTS;
                  goto cleanup;
            }
         }
         else
         {
            wprintf(L"Unknown option %s\n",argv[i]);
            result = ERROR_BAD_ARGUMENTS;
            goto cleanup;
         }
      }
   }
   cleanup:
      if (ERROR_SUCCESS != result)
      {
         *stop = TRUE;
	 Usage(argv[0]);
      }
      return result;
}

/////////////////////////////////
// Private function definitions
/////////////////////////////////
VOID
PrintEventHeader(
   __in FWPM_NET_EVENT0* pEvent
)
/*++

Routine Description:

     This routine prints information in the event header for illustration 
     purposes

Arguments:

    pEvent - Pointer to the event

Return Value:

    None

--*/
{
   SYSTEMTIME systemTime = {0};
   SYSTEMTIME localTime = {0};
   LPTSTR sidString;
   WCHAR dateStr[STR_BUF_SIZE];
   WCHAR timeStr[STR_BUF_SIZE];

   // Static array containing string description of IP versions
   static const wchar_t* const  mIPVersion[]=
   {
       L"IPv4",
       L"IPv6"
   };

   // Static array containing string description of IKE event type
   static const wchar_t* const  mEventType[]=
   {
      L"Main Mode failure",
      L"Quick Mode failure", 
      L"User Mode failure",
      L"Classify drop",
      L"IPsec Kernel drop"
   };

   // Get Time
   FileTimeToSystemTime(&(pEvent->header.timeStamp), &systemTime);
   SystemTimeToTzSpecificLocalTime(NULL, &systemTime, &localTime);

   // Format time & date
   GetTimeFormat(
      LOCALE_USER_DEFAULT,              
      TIME_FORCE24HOURFORMAT,
      &localTime,
      NULL,        
      timeStr,
      STR_BUF_SIZE);
   
   GetDateFormat(
      LOCALE_USER_DEFAULT,
      0,
      &localTime,
      NULL,     
      dateStr,
      STR_BUF_SIZE);

   
   wprintf(L"%-16s:%s %s\n", L"Event Time", dateStr, timeStr);
   if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_IP_VERSION_SET)
   {
      wprintf(L"%-16s:%s\n", L"IP Version", mIPVersion[pEvent->header.ipVersion]);
   }
   if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_IP_PROTOCOL_SET)
   {
      wprintf(L"%-16s:%u\n", L"IP Protocol", (UINT32)pEvent->header.ipProtocol);
   }
      
   if (pEvent->header.ipVersion == FWP_IP_VERSION_V4)
   {
      // Print IPv4 addresses
      char * addrstr  = NULL;
      struct in_addr addr;

      if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_LOCAL_ADDR_SET)
      {
         addr.s_addr = htonl(pEvent->header.localAddrV4);
         addrstr = inet_ntoa(addr);
         if (addrstr)
         {
            wprintf(L"%-16s:%S\n", L"Local Address", addrstr);
         }
      }

      if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_REMOTE_ADDR_SET)
      {
         addr.s_addr = htonl(pEvent->header.remoteAddrV4);
         addrstr = inet_ntoa(addr);
         if (addrstr)
         {
            wprintf(L"%-16s:%S\n", L"Remote  Address", addrstr);
         }
      }
   }
   else
   {
      // Print IPv6 addresses
      int i;
      if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_LOCAL_ADDR_SET)
      {
         wprintf(L"%-16s:", L"Local Address");
         for (i=0; i<16; i=i+2)
         {
            wprintf(L"%02x%02x", 
					pEvent->header.localAddrV6.byteArray16[i],
					pEvent->header.localAddrV6.byteArray16[i+1]);
			if (14!=i) wprintf(L":");
         }
         wprintf(L"\n");
      }

      if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_REMOTE_ADDR_SET)
      {
		 wprintf(L"%-16s:", L"Remote Address");
         for (i=0; i<16; i=i+2)
         {
			wprintf(L"%02x%02x", 
					pEvent->header.remoteAddrV6.byteArray16[i],
					pEvent->header.remoteAddrV6.byteArray16[i+1]);
			if (14!=i) wprintf(L":");
         }
         wprintf(L"\n");      
      }
   }

   // Print local port
   if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_LOCAL_PORT_SET)
   {
      wprintf(L"%-16s:%u\n", L"Local Port",  (UINT32)pEvent->header.localPort);
   }
   
   // Print remote port
   if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_REMOTE_PORT_SET)
   {
      wprintf(L"%-16s:%u\n", L"Remote Port",  (UINT32)pEvent->header.remotePort);
   }

   // Print event type
   wprintf(L"%-16s:%s\n", L"Event Type", mEventType[pEvent->type]);

   // Print fully qualified path of the application
   if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_APP_ID_SET)
   {
      WCHAR* appid = (WCHAR*)pEvent->header.appId.data;
      wprintf(L"%-16s:%s[%u]\n", L"App Id[Size]", appid, pEvent->header.appId.size);
   }

   // Print User SID
   if (pEvent->header.flags & FWPM_NET_EVENT_FLAG_USER_ID_SET)
   {
      if (ConvertSidToStringSid(pEvent->header.userId, &sidString))
      {
         wprintf(L"%-16s:%s\n", L"SID", sidString);
         LocalFree(sidString);
      }
   }
}

VOID
PrintMMEvent(FWPM_NET_EVENT0* pEvent)
/*++

Routine Description:

     This routine prints information in the MM failure event for illustration 
     purposes.

Arguments:

    pEvent - Pointer to the event.

Return Value:

    None

--*/
{
   
   FWPM_NET_EVENT_IKEEXT_MM_FAILURE0* pMmEvent = pEvent->ikeMmFailure;

   // Static array containing string description of Main mode state.
   static const wchar_t* const  mMMState[] =
   {  
      L"IKE_MM_SA_STATE_NONE",
      L"IKE_MM_SA_STATE_SA_SENT",
      L"IKE_MM_SA_STATE_SSPI_SENT" ,
      L"IKE_MM_SA_STATE_FINAL",
      L"IKE_MM_SA_STATE_FINAL_SENT",
      L"IKE_MM_SA_STATE_COMPLETE"
   };
   
   PrintEventHeader(pEvent);
   wprintf(L"%-16s:%s%x\n", L"Failure Code", L"0x", pMmEvent->failureErrorCode);  
   wprintf(L"%-16s:", L"Flags");
   if (FWPM_NET_EVENT_IKEEXT_MM_FAILURE_FLAG_BENIGN & (pMmEvent->flags))
   {
      wprintf(L"Benign, ");
   }
   else
   {
      wprintf(L"Non-benign, ");
   }

   if (FWPM_NET_EVENT_IKEEXT_MM_FAILURE_FLAG_MULTIPLE & (pMmEvent->flags))
   {
      wprintf(L"Multiple Event Failure\n");
      wprintf(L"%-16s:%s%I64x\n", L"Failure Id", L"0x", pMmEvent->mmId); 
   }
   else
   {
      wprintf(L"Single Event Failure\n");
   }

   wprintf(L"%-16s:%s\n", L"Failure Point", mFailurePoint[pMmEvent->failurePoint]);
   wprintf(L"%-16s:%s\n", L"Keying Module", mKeyModuleType[pMmEvent->keyingModuleType]);
   wprintf(L"%-16s:%s\n", L"MM State", mMMState[pMmEvent->mmState]);
   wprintf(L"%-16s:%s\n", L"SA Role", mSARole[pMmEvent->saRole]);
   wprintf(L"%-16s:%s\n", L"MM Authmethod", mAuthmethodtype[pMmEvent->mmAuthMethod]);
   wprintf(L"%-16s:%s%I64x\n", L"Filter Id", L"0x", pMmEvent->mmFilterId);
   wprintf(L"\n");
}

VOID
PrintQMEvent(FWPM_NET_EVENT0* pEvent)
/*++

Routine Description:

     This routine prints information in the QM failure event for illustration 
     purposes.

Arguments:

    pEvent - Pointer to the event.

Return Value:

    None

--*/   
{
   FWPM_NET_EVENT_IKEEXT_QM_FAILURE0* pQmEvent = pEvent->ikeQmFailure;

   
   // Static array containing string description of Quick Mode state
   static const wchar_t* const  mQMState[] =
   {  
      L"IKE_QM_SA_STATE_NONE",
      L"IKE_QM_SA_STATE_INITIAL" ,
      L"IKE_QM_SA_STATE_FINAL",
      L"IKE_QM_SA_STATE_COMPLETE"
   };

   PrintEventHeader(pEvent);
   wprintf(L"%-16s:%s%x\n", L"Failure Code", L"0x", pQmEvent->failureErrorCode);  
   wprintf(L"%-16s:%s\n", L"Failure Point", mFailurePoint[pQmEvent->failurePoint]);
   wprintf(L"%-16s:%s\n", L"Keying Module", mKeyModuleType[pQmEvent->keyingModuleType]);
   wprintf(L"%-16s:%s\n", L"QM State", mQMState[pQmEvent->qmState]);
   wprintf(L"%-16s:%s\n", L"SA Role ", mSARole[pQmEvent->saRole]);
   wprintf(L"%-16s:%s\n", L"Traffic Type", mTrafficType[pQmEvent->saTrafficType]);
   wprintf(L"%-16s:%s%I64x\n", L"Filter Id", L"0x", pQmEvent->qmFilterId);
   wprintf(L"\n");
}

VOID
PrintEMEvent(FWPM_NET_EVENT0* pEvent)
/*++

Routine Description:

     This routine prints information in the EM failure event for illustration 
     purposes.

Arguments:

    pEvent - Pointer to the event.

Return Value:

    None

--*/
{
   FWPM_NET_EVENT_IKEEXT_EM_FAILURE0* pEmEvent = pEvent->ikeEmFailure;
   // Static array containing string description of User Mode state
   static const wchar_t* const  mEMState[] =
   {	
      L"IKE_EM_SA_STATE_NONE",
      L"IKE_EM_SA_STATE_SENT_ATTS",
      L"IKE_EM_SA_STATE_SSPI_SENT",
      L"IKE_EM_SA_STATE_AUTH_COMPLETE" ,
      L"IKE_EM_SA_STATE_FINAL",
      L"IKE_EM_SA_STATE_COMPLETE"
   };
   PrintEventHeader(pEvent);
   wprintf(L"%-16s:%s%x\n", L"Failure Code", L"0x", pEmEvent->failureErrorCode);  
   wprintf(L"%-16s:%s\n", L"Failure Point", mFailurePoint[pEmEvent->failurePoint]);
   wprintf(L"%-16s:%s\n", L"EM State", mEMState[pEmEvent->emState]);
   wprintf(L"%-16s:%s\n", L"SA Role", mSARole[pEmEvent->saRole]);
   wprintf(L"%-16s:%s\n", L"EM Authmethod", mAuthmethodtype[pEmEvent->emAuthMethod]);
   wprintf(L"%-16s:%s%I64x\n", L"Filter Id", L"0x", pEmEvent->qmFilterId);
   wprintf(L"\n");
}

VOID
PrintClassifyEvent(FWPM_NET_EVENT0* pEvent)
/*++

Routine Description:

     This routine prints information in the classify drop events for illustration 
     purposes.

Arguments:

    pEvent - Pointer to the event.

Return Value:

    None

--*/
{
   FWPM_NET_EVENT_CLASSIFY_DROP0* pClassifyEvent = pEvent->classifyDrop;
   PrintEventHeader(pEvent);
   wprintf(L"%-16s:%s%I64x\n", L"Filter Id", L"0x", pClassifyEvent->filterId);
   wprintf(L"%-16s:%s%x\n", L"Layer Id", L"0x",(UINT32)pClassifyEvent->layerId);
   wprintf(L"\n");
}

LPWSTR DisplayError(DWORD NTStatusMessage)
{
   LPWSTR lpMsgBuf = NULL;
   
   if (0 == FormatMessage( 
             FORMAT_MESSAGE_ALLOCATE_BUFFER | 
             FORMAT_MESSAGE_FROM_SYSTEM | 
             FORMAT_MESSAGE_IGNORE_INSERTS,
             NULL,
             NTStatusMessage,
             0,
             (LPTSTR) &lpMsgBuf,
             0,
             NULL))
   {
             return NULL;
   }

   return lpMsgBuf;
}

VOID
PrintKernelDropEvent(FWPM_NET_EVENT0* pEvent)
/*++

Routine Description:

     This routine prints information in the classify drop events for 
     illustration purposes.

Arguments:

    pEvent - Pointer to the event.

Return Value:

    None

--*/
{
   FWPM_NET_EVENT_IPSEC_KERNEL_DROP0* pKernelDropEvent = pEvent->ipsecDrop;
   LPWSTR failureMsg = NULL;

   static const wchar_t* const  mDirection[] =
   {
      L"Outbound",
      L"Inbound"
   };
   
   PrintEventHeader(pEvent);
   if (failureMsg = DisplayError(pKernelDropEvent->failureStatus))
   {
      wprintf(L"%-16s:%s\n", L"Failure status", failureMsg);
      LocalFree(failureMsg); 
   }

   wprintf(L"%-16s:%s\n", L"Direction", mDirection[pKernelDropEvent->direction]);
   wprintf(L"%-16s:%s%x\n", L"SPI", L"0x", pKernelDropEvent->spi);
   wprintf(L"%-16s:%s%I64x\n", L"Filter Id", L"0x", pKernelDropEvent->filterId);
   wprintf(L"%-16s:%s%x\n", L"Layer Id", L"0x",(UINT32)pKernelDropEvent->layerId);
   wprintf(L"\n");
}


VOID 
Usage(
   __in const wchar_t* name
)
/*++

Routine Description:

    This routine prints the intended usage for this program.

Arguments:

    progName - NULL terminated string representing the name of the executable

Return Value:

    None

--*/
{
   wprintf(L"Usage %s: [-t Mins] [-v IPversion] [-r RemoteAddr]\n", name);
   wprintf(L"-t Min: Events of only last Min minutes will be enumerated\n");
   wprintf(L"-v IP version: Specify IP Version (Required if remote IP Address is specified\n");
   wprintf(L"-r RemoteAddr: Enumerate events with IP Addr of the remote computer\n");
   wprintf(L"Example: %s -t 10 -v 4 -r 10.11.12.13\n", name);
}


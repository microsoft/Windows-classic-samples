/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:
    This C file includes sample code for working with contacts using
    the Microsoft Peer-to-Peer Collaboration APIs.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

Note:
    This peer to peer application requires global IPv6 connectivity.

--********************************************************************/

#pragma once

#include <p2p.h>

//Constants used throughout the code
//
#define STRING_BUFSIZE    500
#define INPUT_BUFSIZE     10
#define SHORT_TIMEOUT      (10 * 1000)
#define LONG_TIMEOUT      (30 * 1000)
#define MAX_CONTACT_INFO  (8 * 1024)
#define ERRBUFSIZE        1024
GUID MESSAGE_GUID;


//Some macros to replace frequently used blocks of code
//
#define IF_FAILED_PRINT_ERROR_AND_EXIT(hr, message) \
    if (FAILED(hr)) \
    { \
        wprintf(L"%s", message); \
        wprintf(L" HRESULT=0x%x\n", hr); \
        PrintError(hr); \
        goto exit; \
    }

#define GET_PROMPT_RESPONSE(hr, buf) \
    fflush(stdin); \
    hr = StringCbGets(buf, sizeof(buf)); \
    IF_FAILED_PRINT_ERROR_AND_EXIT(hr, L"StringCbGets failed.");


#define SAFE_FREE(x) \
   if(x != NULL)             \
   {                         \
      free(x);               \
      x = NULL;              \
   }


#define SAFE_PEER_FREE_DATA(x) \
   if(x != NULL)             \
   {                         \
      PeerFreeData(x);       \
      x = NULL;              \
   }

#define SAFE_PEER_END_ENUMERATION(x) \
   if(x != NULL)             \
   {                         \
      PeerEndEnumeration(x); \
      x = NULL;              \
   }

#define HRESULTTOWIN32(hres)                                \
            ((HRESULT_FACILITY(hres) == FACILITY_WIN32)     \
                ? HRESULT_CODE(hres)                        \
                : (hres))

#define celems(a)   (sizeof(a) / sizeof(a[0]))



//Helper functions to convert enums to strings
//
const WCHAR *ChangeType      (__in const PEER_CHANGE_TYPE nChangeType);
const WCHAR *PublicationScope(__in const PEER_PUBLICATION_SCOPE ePublicationScope);
const WCHAR *PresenceStatus  (__in const ULONG nIndex);

//Helper function to convert P2P and Win32 HRESULT codes to strings
//
void PrintError               (HRESULT hrError);

//Helper functions to print various P2P structures and enums to the console
//
void PrintInvitation          (__in PCPEER_INVITATION pInvitation);
void PrintEndpoint            (__in PCPEER_ENDPOINT pEndpoint);
void PrintFullEndpoint        (__in PCPEER_ENDPOINT pEndpoint, BOOL fFull);
void PrintContact             (__in PCPEER_CONTACT pContact);
void PrintContacts            ( __in_ecount(cContacts) PCPEER_CONTACT *ppContacts, __in const ULONG cContacts);
void PrintObject              (__in PCPEER_OBJECT pObject);
void PrintApplication         (__in PCPEER_APPLICATION pApplication);
void PrintPeopleNearMeInfo    (__in_ecount(cPeopleNearMe) PCPEER_PEOPLE_NEAR_ME *ppPeopleNearMe, __in const ULONG cPeopleNearMe);
void PrintPresenceInformation (__in PCPEER_PRESENCE_INFO pPresInfo);
void PrintGUID                (__in const GUID *guid);

//Helper functions for listing and selecting contacts and endpoints
HRESULT SelectMultipleEndpoints(__in unsigned int uNumEndpoints, __out PPEER_CONTACT **pppContacts, __out PPEER_ENDPOINT **pppEndpoints);
HRESULT SelectEndpoint(__out PPEER_CONTACT *ppContact, __out PPEER_ENDPOINT *ppEndpoint);
HRESULT SelectPNMEndpoint(__out PPEER_ENDPOINT *ppEndpoint);  //only allows selecting a PNM endpoint
HRESULT SelectContact(__out PPEER_CONTACT *pContact, __in BOOL fPrompt);

//Updating and flushing cached endpoint information
//
//Calls PeerCollabRegisterEvent, PeerCollabRefreshEndpointData, PeerCollabUnregisterEvent, PeerCollabGetEventData, PeerCollabUnregisterEvent
HRESULT RefreshEndpoint(__in PCPEER_ENDPOINT pcEndpoint);

//Calls PeerCollabDeleteEndpointData
HRESULT DeleteEndpointData(__in PCPEER_ENDPOINT pcEndpoint);

//Misc helper functions
HRESULT GetParentProcessHandle(HANDLE *pHandle);
BOOL PrintInvitationStatus(__in HANDLE *phInvite, __in unsigned int uNumEndpoints, __in BOOL fCancelInvitations);




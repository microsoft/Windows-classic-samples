/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    GraphChat.h

Abstract:

    This H file includes declarations for the chat application built
    with the Peer-to-Peer Graphing API.

--********************************************************************/
#pragma once


#include <p2p.h>
#include <windowsx.h>
#include <objbase.h>
#include <stdio.h>
#include <commdlg.h>
#include <commctrl.h>
#include <strsafe.h>
#include "resource.h"
#include <windows.h>
#include <pnrpns.h>

// Utility Macros
#define celems(a)   (sizeof(a) / sizeof(a[0]))

// Contants
#define GRAPHING_PORT       0      // Port to listen on.  Zero means to use a 
                                   //  a dynamic port.  This is good for using
                                   //  multiple instances on the same machine,
                                   //  but bad for getting through firewalls.
                                  
#define MAX_CHAT_MESSAGE    1024   // Maximum number of characters in a message
#define MAX_EVENT_MESSAGE   1024   // Maximum number of characters in an event message that we print
#define MAX_PEERNAME        149    // Maximum number of characters in a <Classifier> of the peername.

#define MAX_CLOUD_NAME      256    // Maximum length of a cloud name

#define MAX_LOADSTRING      100    // Maximum number of characters in a resource string

#define SB_PART_LISTENING     0    // status bar part number for "Listening" status
#define SB_PART_SYNCHRONIZED  1    // status bar part number for "Synchronized" status
#define SB_PART_CONNECTED     2    // status bar part number for "Connected" status
#define SB_PART_ADDRESS       3    // status bar part number for the address
#define SB_PART_NODE_ID       4    // status bar part number for node id
#define SB_PART_MESSAGE       5    // status bar part number for messages


//Structures
//
typedef struct graphchat_invitation_tag {
    BOOL        fGlobalScope;
    WCHAR       wzGraphId[MAX_PEERNAME];
} GRAPHCHAT_INVITATION, * PGRAPHCHAT_INVITATION;

// Forward declarations
INT_PTR CALLBACK MainProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK NewGraphProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK OpenGraphProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK WhisperProc(HWND, UINT, WPARAM, LPARAM);

HRESULT HandleNewGraph(HWND hDlg);
HRESULT HandleOpenGraph(HWND hDlg);
HRESULT HandleDirectConnection(HWND hDlg);
HRESULT CreateGraph(PCWSTR wzFriendlyName, PCWSTR wzPeerName);
HRESULT OpenGraph(PCWSTR wzIdentity, PCWSTR wzGraph);
HRESULT OpenDirectConnection();

HRESULT CreateIdentity();
HRESULT DeleteIdentity();

HRESULT AddChatRecord(PCWSTR pwzMessage);
HRESULT PrepareToChat(PCWSTR wzGraphId, BOOL fConnect);
HRESULT DiscoverAddress(PCWSTR wzGraphId, __out ULONG *pcEndpoints, __out PPEER_PNRP_ENDPOINT_INFO *pEndpoints);
HRESULT RegisterAddress(PCWSTR wzGraphId);
HRESULT UnregisterAddress(PCWSTR wzGraphId);

void ClearParticipantList(void);
void ClearNeighborList(void);

void SendDirectData();
void UpdateParticipant(PCWSTR wzPerson, BOOL fAdd);
void UpdateNeighbor(ULONGLONG ullNodeID, __in BOOL fAdd);
void CleanupGraph();
void ProcessSendButton(void);
void EnableDisconnectMenu();

void DisplayEvent(__in PEER_GRAPH_EVENT_DATA *pEventData);
void DisplayMessage(PCWSTR pwzPerson, PCWSTR pwzMsg);
#define DisplaySysMsg(wzMsg)  DisplayMessage(L"System", wzMsg)


void SetStatusPart(int sbPart, PCWSTR pwzStatus);
#define SetStatus(wzMsg) SetStatusPart(SB_PART_MESSAGE, wzMsg)

HRESULT GetErrorMsg(HRESULT hrError, ULONG cchMsg, __out_ecount(cchMsg) PWSTR pwzMsg);
void MsgErrHr(PCWSTR pwzMsg, HRESULT hr, PCSTR pszFunction);
#define DisplayHrError(pwzMsg, hr)   MsgErrHr(pwzMsg, hr, __FUNCTION__)

void CALLBACK EventCallback(PVOID lpParam, BOOLEAN reason);
HRESULT RegisterForEvents();

void    ProcessRecordChangeEvent(PEER_GRAPH_EVENT_DATA *pEventData);
void    ProcessStatusChangeEvent(DWORD dwStatus);
void    ProcessNodeChangeEvent(PEER_GRAPH_EVENT_DATA *pEventData);
void    ProcessDirectConnectionEvent(PEER_GRAPH_EVENT_DATA *pEventData);
void    ProcessNeighborConnectionEvent(__in PEER_GRAPH_EVENT_DATA *pEventData);
void    ProcessIncomingDataEvent(PEER_GRAPH_EVENT_DATA *pEventData);
void    ProcessConnectionRequiredEvent(PEER_GRAPH_EVENT_DATA *pEventData);

//People Near Me Forward Declarations
void EnableInviteMenu();
HRESULT SignIntoPNM();
HRESULT InviteSomeoneNearby();
HRESULT HandleAppInvite();
HRESULT RegisterApplication(PEER_APPLICATION_REGISTRATION_TYPE registrationType);

HRESULT WINAPI GetBestDestinationAddress(IN PPEER_PNRP_ENDPOINT_INFO pEndpoint, OUT SOCKADDR_IN6     *psaBestAddress);


/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation.  All rights reserved.

Module Name:

    GroupChat.h

Abstract:

    This is the main include file for standard system include files,
    or project specific include files.

--********************************************************************/

// C RunTime Header Files
#include <pshpack8.h>
#include <initguid.h>
#include <p2p.h>
#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <strsafe.h>

#include "resource.h"

// Utility Macros
#define celems(a)   (sizeof(a) / sizeof(a[0]))
#define DisplayHrError(pwzMsg, hr)   MsgErrHr(pwzMsg, hr, __FUNCTION__)

#define HRESULTTOWIN32(hres)                                \
            ((HRESULT_FACILITY(hres) == FACILITY_WIN32)     \
                ? HRESULT_CODE(hres)                        \
                : (hres))


// Constants
#define MAX_CHAT_MESSAGE   1024    // Maximum number of characters in a message
#define MAX_USERNAME        256    // Maximum number of characters in a user's name
#define MAX_GROUPNAME       256    // Maximum number of characters in a group name
#define MAX_IDENTITY        256    // Maximum number of characters in an identity
#define MAX_INVITATION   (1024*64) // Maximum size of an invitation
#define MAX_LOADSTRING      100    // Maximum size of resource string

#define SB_PART_STATUS        0    // status bar part number for "Listening"/"Connected"
#define SB_PART_MESSAGE       1    // status bar part number for generic messages

// Function callbacks
LRESULT  CALLBACK  MainProc(HWND, UINT, WPARAM, LPARAM);
LRESULT  CALLBACK  AboutProc(HWND, UINT, WPARAM, LPARAM);
LRESULT  CALLBACK  NewGroupProc(HWND, UINT, WPARAM, LPARAM);
LRESULT  CALLBACK  OpenGroupProc(HWND, UINT, WPARAM, LPARAM);
LRESULT  CALLBACK  JoinGroupProc(HWND, UINT, WPARAM, LPARAM);
LRESULT  CALLBACK  DeleteGroupProc(HWND, UINT, WPARAM, LPARAM);
LRESULT  CALLBACK  NewIdentityProc(HWND, UINT, WPARAM, LPARAM);
LRESULT  CALLBACK  SaveIdentityInfoProc(HWND, UINT, WPARAM, LPARAM);
LRESULT  CALLBACK  DeleteIdentityProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  CALLBACK  CreateInvitationProc(HWND, UINT, WPARAM, LPARAM);
LRESULT  CALLBACK  WhisperMessageProc(HWND, UINT, WPARAM, LPARAM);

// Handlers that extract the data from the dialogs and call utility functions
HRESULT HandleNewGroup(HWND hDlg);
HRESULT HandleOpenGroup(HWND hDlg);
HRESULT HandleJoinGroup(HWND hDlg);
HRESULT HandleDeleteGroup(HWND hDlg);
HRESULT HandleCreateIdentity(HWND hDlg);
HRESULT HandleCreateInvitation(HWND hDlg);
HRESULT HandleSaveIdentityInfo(HWND hDlg);
HRESULT HandleDeleteIdentity(HWND hDlg);
HRESULT HandleWhisperMessage(HWND hDlg);

// Routines that do the real work
HRESULT SaveIdentityInfo(PCWSTR pwzIdentity, PCWSTR pwzFile);
HRESULT CreateInvitation(PCWSTR wzIdentityInfoPath, PCWSTR wzInvitationPath);
HRESULT CreateGroup(PCWSTR wzFriendlyName, PCWSTR wzPeerName);
HRESULT OpenGroup(PCWSTR pwzIdentity, PCWSTR pwzGroupName);
HRESULT JoinGroup(PCWSTR pwzIdentity, PCWSTR pwzInvitation);
HRESULT DeleteGroup(PCWSTR pwzName, PCWSTR pwzIdentity);
HRESULT DeleteIdentity(PCWSTR pwzIdentity);

HRESULT RegisterForEvents();
HRESULT RefreshIdentityCombo(HWND hwnd, BOOL bAddNullIdentity, PEER_NAME_PAIR ***pppNamePairs);
HRESULT RefreshGroupCombo(HWND hwnd, PCWSTR pwzIdentity, PEER_NAME_PAIR ***pppNamePairs);
HRESULT PrepareToChat();
HRESULT AddChatRecord(PCWSTR pwzMessage);
HRESULT SetupDirectConnection();

void ProcessIncomingData(PEER_GROUP_EVENT_DATA * pData);

void UpdateParticipantList();
void CleanupGroup();
void BrowseHelper(HWND hDlg, int idEditbox, PCWSTR wzFileType, PCWSTR pwzFileExtension, BOOL fOpen);
void CALLBACK EventCallback(PVOID lpParam, BOOLEAN reason);

void CmdCloseGroup(void);

int    GetFriendlyNameForIdentity(PCWSTR pwzIdentity, __out PWSTR pwzName, int cchMax);
PCWSTR GetSelectedIdentity(HWND hDlg);
PCWSTR GetSelectedGroup(HWND hDlg);
PCWSTR GetSelectedChatMember( );
void SetStatus(PCWSTR pwzStatus);
void DisplayChatMessage(PCWSTR pwzIdentity, PCWSTR pwzMsg);
void DisplayMsg(PCWSTR pwzMsg);

void ProcessStatusChanged(DWORD dwStatus);

// Utility Routines
void DisplayError(PCWSTR pwzMsg);
HRESULT GetErrorMsg(HRESULT hrError, ULONG cchMsg, __out_ecount(cchMsg) PWSTR pwzMsg);
void MsgErrHr(PCWSTR pwzMsg, HRESULT hr, PCSTR pszFunction);


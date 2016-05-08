////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 1998-2001  Microsoft Corporation
//
//
// FileTerm.h : General declarations, definitions for FileTerm.cpp
//
////////////////////////////////////////////////////////////////////////////
#ifndef __FILETERM_H__
#define __FILETERM_H__

#define TAPI_TIMEOUT			2000
#define MSGBOX_NAME				_T("TAPI 3.1 File Terminals Sample")


//////////////////////////////////////////////////////////
// PROTOTYPES
//////////////////////////////////////////////////////////
INT_PTR
CALLBACK
MainDialogProc(
				HWND hDlg,
				UINT uMsg,
				WPARAM wParam,
				LPARAM lParam
				);

HRESULT
GetTerminal(
				IN	ITAddress *,
				IN	BSTR bstrMedia,
				OUT	ITTerminal ** ppTerminal
				);
HRESULT
RegisterTapiEventInterface();

HRESULT
UnRegisterTapiEventInterface();


HRESULT
ListenOnAddresses();

HRESULT
ListenOnThisAddress(
				IN	ITAddress * pAddress
				);

HRESULT
AnswerTheCall();

HRESULT 
PutPlayList(
				IN	ITTerminal *pITTerminal, 
				IN	BSTR bstrFileName
				);

HRESULT
DisconnectTheCall();

void
ReleaseTheCall();

void
DoMessage(
				IN	LPWSTR pszMessage
				);

void
SetStatusMessage(
				IN	LPWSTR pszMessage
				);

HRESULT
InitializeTapi();

void
ShutdownTapi();

#endif //__FILETERM_H__

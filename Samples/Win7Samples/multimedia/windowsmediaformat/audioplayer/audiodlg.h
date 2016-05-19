//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            AudioDialog.h
//
// Abstract:            Declarations of functions and global variables for the Dialog.
//
//*****************************************************************************

#if !defined(AFX_DIALOG_H__F5DBA316_C86C_40DF_9178_141E8219480F__INCLUDED_)
#define AFX_DIALOG_H__F5DBA316_C86C_40DF_9178_141E8219480F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AudioPlay.h"

#define OEMRESOURCE 
#define SMALLDLGSIZE 0.5
#define ERROR_DIALOG_TITLE  _T( "WM Audio Player Sample" )

typedef enum AUDIOSTATUS
{
	CLOSED = 0,
	STOP,
	PAUSE,
	PLAY,
	OPENING,
	ACQUIRINGLICENSE,
	INDIVIDUALIZING,
	STOPPING,
	READY,
	BUFFERING,
	LICENSEACQUIRED,
	INDIVIDUALIZED
} AUDIOSTATUS;

INT_PTR		CALLBACK DlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
DWORD_PTR	SetItemText( UINT nControlID, LPCWSTR pwszText );
void	SetCurrentStatus( AUDIOSTATUS currentStatus	);
void	SetTime( QWORD cnsTimeElapsed, QWORD cnsFileDuration );
BOOL	ShowOpenFileDialog();
BOOL	OnPlay();

extern HINSTANCE	g_hInst;
extern HWND			g_hwndDialog;

#endif // !defined(AFX_DIALOG_H__F5DBA316_C86C_40DF_9178_141E8219480F__INCLUDED_)

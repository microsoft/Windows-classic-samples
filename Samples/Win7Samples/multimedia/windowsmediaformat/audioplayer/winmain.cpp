//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Winmain.cpp
//
// Abstract:            Defines the entry point for the application.
//
//*****************************************************************************

#include "stdafx.h"
#include "AudioDlg.h"

//------------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.
//------------------------------------------------------------------------------
int APIENTRY WinMain( __in HINSTANCE hInstance,
					  __in_opt HINSTANCE hPrevInstance,
					  __in_opt LPSTR lpCmdLine,
					  __in int nCmdShow )
{
	//
	//Store the value of the instance handle
	//
	g_hInst = hInstance;

	DialogBox( hInstance, MAKEINTRESOURCE( IDD_WMADIALOG ), NULL, DlgProc );
	
	return( 0 );
}

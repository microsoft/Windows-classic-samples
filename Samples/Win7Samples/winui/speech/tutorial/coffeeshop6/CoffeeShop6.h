// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

/******************************************************************************
*   CoffeeShop6.h 
*       This module contains the base definitions for the CoffeeShop6 tutorial
*       application.
******************************************************************************/
#pragma once

#include "resource.h"

// Global Variables:
HINSTANCE                       g_hInst;		// current instance
TCHAR g_szCounterDisplay[MAX_LOADSTRING];       // Display String for counter
TCHAR g_szShopName[NORMAL_LOADSTRING];          // Name of the Coffee Shop
CComPtr<ISpRecoGrammar>         g_cpCmdGrammar; // Pointer to our grammar object
CComPtr<ISpRecoContext>         g_cpRecoCtxt;   // Pointer to our recognition context
CComPtr<ISpRecognizer>		    g_cpEngine;		// Pointer to our recognition engine instance
CComPtr<ISpVoice>               g_cpVoice;		// Pointer to our tts voice
PMSGHANDLER                     g_fpCurrentPane;// Pointer to current message handler

// Foward declarations of functions included in this code module:
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT             InitSAPI( HWND hWnd );
void                CleanupSAPI( void );
void                ProcessRecoEvent( HWND hWnd );
void                ExecuteCommand(ISpRecoResult *pPhrase, HWND hWnd);
LRESULT             EntryPaneProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT             CounterPaneProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT             OfficePaneProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
void                HandleFalseReco(ISpRecoResult *pRecoResult, HWND hWnd);
LRESULT             ManageEmployeesPaneProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
void                ManageEmployeesPaneCleanup( __in_ecount_opt(ulNumTokens) WCHAR** ppszTokenIds,
                                                CSpDynamicString*  ppcDescriptionString, ULONG ulNumTokens);
LRESULT             ChangeNamePaneProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );


// Declaration of UI specific routines located in display.cpp
ATOM				MyRegisterClass(HINSTANCE hInstance, WNDPROC WndProc);
BOOL				InitInstance(HINSTANCE, int);
void                EraseBackground( HDC hDC );
void                CleanupGDIObjects( void );
void                EntryPanePaint( HWND hWnd, LPCTSTR szName );
void                CounterPanePaint( HWND hWnd, LPCTSTR szCounterDisplay );
void                OfficePanePaint( HWND hWnd, LPCTSTR szName );
void                ManageEmployeesPanePaint( HWND hWnd, ULONG ulNumTokens, CSpDynamicString* ppcDescriptionString,
											  ULONG ulCurToken, UINT iEnumType );
void                ChangeNamePanePaint( HWND hWnd, LPCTSTR szCounterDisplay);


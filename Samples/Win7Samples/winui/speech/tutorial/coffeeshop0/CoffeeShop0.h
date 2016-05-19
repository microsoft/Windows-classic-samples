// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

/******************************************************************************
*   CoffeeShop0.h 
*       This module contains the base definitions for the CoffeeShop0 tutorial
*       application.
******************************************************************************/
#pragma once

#include "resource.h"

// Global Variables:
HINSTANCE                       g_hInst;		// current instance
TCHAR g_szCounterDisplay[MAX_LOADSTRING];       // Display String for counter
CComPtr<ISpRecoGrammar>         g_cpCmdGrammar; // Pointer to our grammar object
CComPtr<ISpRecoContext>         g_cpRecoCtxt;   // Pointer to our recognition context
CComPtr<ISpRecognizer>		g_cpEngine;		// Pointer to our recognition engine instance
PMSGHANDLER                     g_fpCurrentPane;// Pointer to current message handler

// Foward declarations of functions included in this code module:
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT             InitSAPI( HWND hWnd );
void                CleanupSAPI( void );
void                ProcessRecoEvent( HWND hWnd );
void                ExecuteCommand(ISpPhrase *pPhrase, HWND hWnd);
LRESULT             EntryPaneProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
LRESULT             CounterPaneProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

// Declaration of UI specific routines located in display.cpp
ATOM				MyRegisterClass(HINSTANCE hInstance, WNDPROC WndProc);
BOOL				InitInstance(HINSTANCE, int);
void                EraseBackground( HDC hDC );
void                CleanupGDIObjects( void );
void                EntryPanePaint( HWND hWnd );
void                CounterPanePaint( HWND hWnd, LPCTSTR szCounterDisplay );



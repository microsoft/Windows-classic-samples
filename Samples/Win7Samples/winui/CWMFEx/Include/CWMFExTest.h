//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2008  Microsoft Corporation.  All rights reserved.
//
//  CWMFExTest.h
//
//         The definitions for the necessary entry points and symbols.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

// defines

#define WNDCLASSNAME                L"CWMFExTestWindow"
#define CWMFEX_CONTROL              WM_APP+1
#define CWMFEX_ACK                  (0x0f0f0f0f)

#define WPARAM_CONTROL(dwMsgFlt, fForWnd)   (MAKEWPARAM((WORD)dwMsgFlt, (WORD)fForWnd))
#define IS_WPARAM_FORWND(wparam)            ((BOOL)(HIWORD((DWORD)wparam)))
#define WPARAM2MSGFLT(wparam)               ((DWORD)(LOWORD((DWORD)wparam)))

// entry points

VOID RunReceiver(HINSTANCE hInstance);
VOID RunSender();

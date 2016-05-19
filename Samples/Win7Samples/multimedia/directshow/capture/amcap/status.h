//------------------------------------------------------------------------------
// File: Status.h
//
// Desc: Header file for the Status Bar
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------
extern TCHAR szStatusClass[];


//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------
BOOL  statusInit(HANDLE hInst, HANDLE hPrev);
void  statusUpdateStatus(HWND hwnd, LPCTSTR lpsz);
int statusGetHeight(void);

/*
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1997 - 2000.  Microsoft Corporation.  All rights reserved.
*/

#if !defined (INC_MAIN_H)
#define INC_MAIN_H

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

// Access rights for our private objects
#define ACCESS_READ     1
#define ACCESS_MODIFY   2
#define ACCESS_DELETE   4


#define ACCESS_ALL  ACCESS_READ | ACCESS_MODIFY | ACCESS_DELETE

typedef struct tagObject {
   PSECURITY_DESCRIPTOR pSD;
   BOOL                 fContainer;
   struct tagObject     *Parent;
   struct tagObject     *Objects;
   HTREEITEM            hTreeItem;
} OBJECT, *POBJECT;



// Main.cpp
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

// Security.cpp

// Private Object functions
BOOL CheckAccess( POBJECT pObject, HANDLE hToken );
PSECURITY_DESCRIPTOR GetCreatorSD(WORD wIndex);
POBJECT NewObject( POBJECT pContainer, BOOL fContainer, WORD wIndex );

// Security Context functions
HANDLE GetClientToken( WORD wIndex );
HANDLE GetClientImpToken( WORD wIndex );
PSECURITY_DESCRIPTOR GetClientDescriptor( WORD wIndex );
BOOL NewLogon( HINSTANCE, WORD );
void GetLogonName( WORD wIndex, WCHAR szUser[], SIZE_T cchUser );
BOOL DefaultUser( void );
BOOL CALLBACK LogonDlgProc (HWND hwnd, UINT msg,
                     WPARAM wParam,  LPARAM lParam);

// PrivOjSI.cpp
extern "C"
HRESULT
CreateObjSecurityInfo(DWORD dwFlags,           // e.g. SI_EDIT_ALL | SI_ADVANCED | SI_CONTAINER
                      PSECURITY_DESCRIPTOR *ppSD,        // Program defined structure for objects
                      LPSECURITYINFO *ppObjSI,
                      WORD wClient,           // Index for client token
                      LPCWSTR pszServerName,  // Name of server on which SIDs will be resolved
                      BOOL    fContainer);     // This is the only way to name my generic objects



#endif // INC_MAIN_H


/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1997 - 2000.  Microsoft Corporation.  All rights reserved.

   Module Name:

       main.cpp

   Description:

       This module contains the code for processing the dialog box interface,
       including the private object tree view.

       TODO: Select an access level request before doing AccessCheck

--*/

#define UNICODE
#define _UNICODE

#include "resource.h"
#include "aclui.h"
#include "Main.h"

// GLOBALS
char        szAppName[] = "Object Security Sample";   // The name of this application
HINSTANCE   g_hInstance;

BOOL InitInstance(
        HINSTANCE          hInstance,
        int                nCmdShow);

//////////////////////////////////////////////////////////////////
// You gotta have a WinMain
//
//////////////////////////////////////////////////////////////////
int CALLBACK WinMain( HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine,
                      int nCmdShow)
{

   MSG      msg;
   HACCEL   hAccelTable;

   // Perform initializations that apply to a specific instance

   if( !InitInstance(hInstance, nCmdShow))
      return(FALSE);

   hAccelTable = LoadAccelerators (hInstance, MAKEINTRESOURCE(IDR_PRIVOBJ_ACC));

   // Acquire and dispatch messages until a WM_QUIT message is received.

   while( GetMessage( &msg, // message structure
                      NULL,   // handle of window receiving the message
                      0,      // lowest message to examine
                      0 ))     // highest message to examine
   {
      if( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
      {
            TranslateMessage(&msg);// Translates virtual key codes
            DispatchMessage(&msg); // Dispatches message to window
      }
   }


   return (int)(msg.wParam); // Returns the value from PostQuitMessage

   lpCmdLine; // This will prevent 'unused formal parameter' warnings
}

//////////////////////////////////////////////////////////////////
// Good old InitInstance
//
//////////////////////////////////////////////////////////////////
BOOL InitInstance(
        HINSTANCE          hInstance,
        int                nCmdShow)
{
        HWND            hWnd; // Main window handle.

        // Save the instance handle in static variable, which will be used in
        // many subsequence calls from this application to Windows.

        g_hInstance = hInstance; // Store instance handle in our global variable

        InitCommonControls();

        // Create a main window for this application instance.

        hWnd = CreateDialog( hInstance,
                             MAKEINTRESOURCE(IDD_DIALOG_MAIN),
                             NULL,
                             (DLGPROC)DlgProc );

        // If window could not be created, return "failure"
        if (!hWnd) {
                return (FALSE);
        }

        // Make the window visible; update its client area; and return "success"
        ShowWindow(hWnd, nCmdShow); // Show the window
        UpdateWindow(hWnd);         // Sends WM_PAINT message

        return (TRUE);              // We succeeded...

}

//////////////////////////////////////////////////////////////////
//
// AddItemToTree - adds items to a tree-view control.
//
// Returns: the handle of the newly added item.
//
// Parameters:
//    hwndTV - handle of the tree-view control
//    lpszItem - text of the item to add
//    hParent - the parent object
//    pObject - allows us to reference object through the tree view item
//
//////////////////////////////////////////////////////////////////
HTREEITEM AddItemToTree(
            HWND hwndTV,
            LPTSTR lpszItem,
            HTREEITEM hParent,
            POBJECT pObject
            )
{
   TV_INSERTSTRUCT IStruct;
   HTREEITEM hTreeItem;

   IStruct.item.mask = TVIF_TEXT | TVIF_PARAM;

   // Set the text of the item.
   IStruct.item.pszText = lpszItem;
   IStruct.item.cchTextMax = lstrlen(lpszItem);

   // use the item's data area to store pointer to object
   IStruct.item.lParam = (LPARAM) pObject;

   if( pObject->fContainer )
      IStruct.hInsertAfter = TVI_FIRST;
   else
      IStruct.hInsertAfter = TVI_LAST;

   IStruct.hParent = hParent;

   hTreeItem = TreeView_InsertItem( hwndTV, &IStruct );

   pObject->hTreeItem = hTreeItem;

   return hTreeItem;
}


//////////////////////////////////////////////////////////////////
// BOOL CenterDialog
//
//////////////////////////////////////////////////////////////////
BOOL CenterDialog(HWND hdlg)
{
    RECT  rcParent;                         // Parent window client rect
    RECT  rcDlg;                            // Dialog window rect
    int   nLeft, nTop;                      // Top-left coordinates
    int   cWidth, cHeight;                  // Width and height
    HWND  hwnd;

    // Get frame window client rect in screen coordinates
    hwnd = GetParent(hdlg);
    if(hwnd == NULL || hwnd == GetDesktopWindow())
    {
        rcParent.top = rcParent.left = 0;
        rcParent.right = GetSystemMetrics(SM_CXFULLSCREEN);
        rcParent.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    }
    else
    GetWindowRect(hwnd, &rcParent);

    // Determine the top-left point for the dialog to be centered
    GetWindowRect(hdlg, &rcDlg);
    cWidth  = rcDlg.right  - rcDlg.left;
    cHeight = rcDlg.bottom - rcDlg.top;
    nLeft   = rcParent.left +
                (((rcParent.right  - rcParent.left) - cWidth ) / 2);
    nTop    = rcParent.top  +
                (((rcParent.bottom - rcParent.top ) - cHeight) / 2);
    if (nLeft < 0) nLeft = 0;
    if (nTop  < 0) nTop  = 0;

    // Place the dialog
    return MoveWindow(hdlg, nLeft, nTop, cWidth, cHeight, TRUE);
}

//////////////////////////////////////////////////////////////////
// This is the main dialog box that presents the tree view
// presentation of our private objects and the buttons that
// allow you to do other things.
//
//////////////////////////////////////////////////////////////////
BOOL CALLBACK DlgProc (HWND hwnd, UINT msg,
                     WPARAM wParam,  LPARAM lParam)
{

   LRESULT  lCount;
   HANDLE   hToken;
   POBJECT  pObject=NULL,pNewObject=NULL;
   WORD     wClient;
   DWORD    dwResult;
   HTREEITEM hTreeItem;
   HTREEITEM hNewTreeItem=NULL;
   TV_ITEM   tvItem;
   WCHAR     szUser[256];
   DWORD     dwSIFlags=SI_EDIT_ALL | SI_ADVANCED | SI_NO_ACL_PROTECT; // Used by security editor
   HWND      hControlWnd;

   switch (msg)
   {

      case WM_CLOSE:
         DestroyWindow(hwnd);
         PostQuitMessage(1);
         return TRUE;


      case WM_INITDIALOG:

          CenterDialog(hwnd);
         //
         // Initialize the user list
         //
         DefaultUser();


         GetLogonName(0,szUser,256);
         SendMessage( GetDlgItem( hwnd,IDC_USERLIST ),
                      CB_ADDSTRING, 0, (LPARAM)szUser);

         SendMessage( GetDlgItem( hwnd,IDC_USERLIST ),
            CB_SETCURSEL, 0, 0L );

          return TRUE ;


      case WM_COMMAND:
            switch (LOWORD(wParam))
            {
               case IDOK:
                  DestroyWindow(hwnd);
                  PostQuitMessage(1);
                  return TRUE;

               case IDC_IMPERSONATE:
                  lCount = SendMessage( GetDlgItem( hwnd,IDC_USERLIST ),
                                        CB_GETCOUNT, 0, 0 );
                  if( NewLogon( g_hInstance, (WORD)lCount ) )
                  {
                     GetLogonName((WORD)lCount,szUser,256);
                     SendMessage( GetDlgItem( hwnd,IDC_USERLIST ),
                                  CB_ADDSTRING, 0, (LPARAM)szUser);
                  }
                  else
                     MessageBox(NULL, L"New Logon failed!", L"Logon Error", MB_OK);
                  return TRUE;

               case IDC_ACCESSCHECK:

                  hControlWnd = GetDlgItem(hwnd, IDC_OBJECT_TREE);

                  hTreeItem = TreeView_GetSelection(hControlWnd);

                  if( hTreeItem == NULL )
                  {
                     MessageBox( NULL, L"Select Container or Object first", L"Error", MB_OK );
                     break;
                  }

                  tvItem.mask = TVIF_PARAM;
                  tvItem.hItem = hTreeItem;

                  // get the tree view item
                  TreeView_GetItem(hControlWnd,&tvItem);

                  // get the object pointer from the tree view item
                  pObject = (POBJECT)tvItem.lParam;

                  wClient = (WORD)SendMessage( GetDlgItem( hwnd,IDC_USERLIST ),
                                               CB_GETCURSEL, 0, 0 );

                  hToken = GetClientImpToken( wClient );
                  dwResult = CheckAccess( pObject, hToken );

                  SetFocus(hControlWnd);

                  return TRUE;

               case IDC_NEW_TREE:

                  hControlWnd = GetDlgItem( hwnd, IDC_OBJECT_TREE );

                  pObject = NULL;

                  wClient = (WORD)SendMessage( GetDlgItem( hwnd,IDC_USERLIST ),
                                               CB_GETCURSEL, 0, 0 );

                  pNewObject = NewObject( pObject, TRUE, wClient );

                  pNewObject->fContainer = TRUE;

                  AddItemToTree( hControlWnd,
                                 L"Root",
                                 NULL,
                                 pNewObject );

                  SetFocus( hControlWnd );

                  return TRUE;

               case IDC_NEW_CONTAINER:

                  hControlWnd = GetDlgItem( hwnd, IDC_OBJECT_TREE );

                   hTreeItem = TreeView_GetSelection( hControlWnd );

                  if( hTreeItem == NULL )
                  {
                     MessageBox( NULL, L"Select Container first", L"Error", MB_OK );
                     break;
                  }
                  tvItem.mask = TVIF_PARAM;
                  tvItem.hItem = hTreeItem;

                  // Make sure it is a container object
                  TreeView_GetItem( hControlWnd, &tvItem );

                  // get the object pointer from the tree view item
                  pObject = (POBJECT)tvItem.lParam;

                  if( !pObject->fContainer )
                  {
                     MessageBox( NULL, L"Object not a container", L"Error", MB_OK );
                     break;
                  }

                  // If this were a actual secure application, I would want to
                  // make sure that the current client has the proper access
                  // right on this container to create a new container.
                  // ACCESS_CREATE_OBJECT?

                  wClient = (WORD)SendMessage( GetDlgItem( hwnd,IDC_USERLIST ),
                                               CB_GETCURSEL, 0, 0 );

                  pNewObject = NewObject( pObject, TRUE, wClient );

                  pNewObject->fContainer = TRUE;

                  hNewTreeItem = AddItemToTree(
                                     hControlWnd,
                                     L"Container",
                                     hTreeItem,
                                     pNewObject );

                  SetFocus( hControlWnd );

                  TreeView_SelectItem(
                      hControlWnd,
                      hNewTreeItem);

                  return TRUE;

               case IDC_NEW_OBJECT:

                  hControlWnd = GetDlgItem( hwnd, IDC_OBJECT_TREE );

                  hTreeItem = TreeView_GetSelection(
                     GetDlgItem( hwnd, IDC_OBJECT_TREE ) );

                  if( hTreeItem == NULL )
                  {
                     MessageBox( NULL, L"Select Container first", L"Error", MB_OK );
                     break;
                  }

                  tvItem.mask = TVIF_PARAM;
                  tvItem.hItem = hTreeItem;

                  // Make sure it is a container object
                  TreeView_GetItem( hControlWnd, &tvItem );

                  pObject = (POBJECT)tvItem.lParam;

                  if( !pObject->fContainer )
                  {
                     MessageBox( NULL, L"Object not a container", L"Error", MB_OK );
                     break;
                  }

                  // If this were a actual secure application, I would want to
                  // make sure that the current client has the proper access
                  // right on this container to create a new object.
                  // ACCESS_CREATE_OBJECT?

                  wClient = (WORD)SendMessage( GetDlgItem( hwnd,IDC_USERLIST ),
                                               CB_GETCURSEL, 0, 0 );

                  pNewObject = NewObject( pObject, FALSE, wClient );
                  pNewObject->fContainer = FALSE;

                  hNewTreeItem = AddItemToTree(
                                     hControlWnd,
                                     L"Object",
                                     hTreeItem,
                                     pNewObject );

                  SetFocus( hControlWnd );

                  TreeView_Select(
                      hControlWnd,
                      hNewTreeItem,
                      TVGN_FIRSTVISIBLE);

                  return TRUE;

               case IDC_SECURITY:

                  hControlWnd = GetDlgItem( hwnd, IDC_OBJECT_TREE );

                  hTreeItem = TreeView_GetSelection( hControlWnd );

                  if( hTreeItem == NULL )
                  {
                     MessageBox( NULL, L"Select Container or Object first", L"Error", MB_OK );
                     break;
                  }

                  tvItem.mask = TVIF_PARAM;
                  tvItem.hItem = hTreeItem;

                  // get the tree view item
                  TreeView_GetItem( hControlWnd, &tvItem );

                  // get the object pointer from the tree view item
                  pObject = (POBJECT)tvItem.lParam;

                  wClient = (WORD)SendMessage(GetDlgItem( hwnd,IDC_USERLIST ),
                                              CB_GETCURSEL, 0, 0);

                  LPSECURITYINFO psi;

                  if(SUCCEEDED(CreateObjSecurityInfo( dwSIFlags,
                                          &(pObject->pSD), &psi, wClient,
                                          L"", pObject->fContainer)))
                  {
                     EditSecurity(NULL,psi);
                     psi->Release();
                     tvItem.lParam = (LPARAM)pObject;
                     TreeView_SetItem(hControlWnd, &tvItem );
                  }
                  else
                     MessageBox(NULL, L"CreateObjSecurityInfo Failed", L"Error", MB_OK);

                  SetFocus(hControlWnd);

                  return TRUE;


            }
        default:
           break;

   }

   return FALSE;

}


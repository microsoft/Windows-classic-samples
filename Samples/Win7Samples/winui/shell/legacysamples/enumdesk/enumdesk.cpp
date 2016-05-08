/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          EnumDesk.cpp

   Description:   

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>
#include "EnumDesk.h"
#include "Tree.h"
#include "List.h"
#include "resource.h"

/**************************************************************************
   function prototypes
**************************************************************************/

int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL Main_InitApplication(HINSTANCE);
BOOL Main_InitInstance(HINSTANCE, int);
LRESULT CALLBACK Main_WndProc(HWND, UINT, WPARAM, LPARAM);
HIMAGELIST Main_CreateImageList(BOOL);
void Main_SizeChildren(HWND);
void Main_UpdateMenu(HWND, HMENU);
LRESULT Main_OnCommand(HWND, WPARAM, LPARAM);

/**************************************************************************
   global variables and definitions
**************************************************************************/

HINSTANCE      g_hInst;
TCHAR          g_szClassName[] = TEXT("EnumDeskWndClass");
HWND           g_hwndMain;
LPMALLOC       g_pMalloc;
UINT           g_uPosition;
IContextMenu2  *g_pcm2;

#define GAP_SIZE    2
#define MIN_SIZE    10

/******************************************************************************

   WinMain

******************************************************************************/

int PASCAL WinMain(  HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
UNREFERENCED_PARAMETER(lpCmdLine);

MSG  msg;

g_hInst = hInstance;

if(!hPrevInstance)
   if(!Main_InitApplication(hInstance))
      return FALSE;

CoInitialize(NULL);

//required to use the common controls
InitCommonControls();

/* Perform initializations that apply to a specific instance */

if (!Main_InitInstance(hInstance, nCmdShow))
   return FALSE;

/* Acquire and dispatch messages until a WM_QUIT uMessage is received. */

HACCEL   hAccels = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELS));

while(GetMessage( &msg, NULL, 0x00, 0x00))
   {
   if(!TranslateAccelerator(g_hwndMain, hAccels, &msg))
      {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      }
   }

CoUninitialize();

return (int)msg.wParam;
}

/******************************************************************************

   Main_InitApplication

******************************************************************************/

BOOL Main_InitApplication(HINSTANCE hInstance)
{
WNDCLASS  wc;

/* Fill in window class structure with parameters that describe the main window. */

wc.style          = 0;
wc.lpfnWndProc    = (WNDPROC)Main_WndProc;
wc.cbClsExtra     = 0;
wc.cbWndExtra     = 0;
wc.hInstance      = hInstance;
wc.hIcon          = LoadIcon(NULL, IDI_WINLOGO);
wc.hCursor        = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_SPLIT_CURSOR));
wc.hbrBackground  = GetSysColorBrush(COLOR_3DFACE);
wc.lpszMenuName   = MAKEINTRESOURCE(IDM_MAIN_MENU);
wc.lpszClassName  = g_szClassName;

/* Register the window class and return success/failure code. */

return RegisterClass(&wc);
}


/******************************************************************************

   Main_InitInstance

******************************************************************************/

BOOL Main_InitInstance( HINSTANCE hInstance,
                        int nCmdShow)
{
/* Create a main window for this application instance.  */

g_hwndMain = CreateWindowEx(  0,
                              g_szClassName,
                              TEXT("New EnumDesk Sample"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              NULL,
                              NULL,
                              hInstance,
                              NULL);

/* If window could not be created, return "failure" */

if(!g_hwndMain)
   return FALSE;

/* Make the window visible; update its client area; and return "success" */

ShowWindow(g_hwndMain, nCmdShow);
UpdateWindow(g_hwndMain);
return TRUE;

}

/******************************************************************************

   Main_WndProc

******************************************************************************/

LRESULT CALLBACK Main_WndProc(HWND hWnd,
                              UINT uMessage,
                              WPARAM wParam,
                              LPARAM lParam)
{
switch (uMessage)
   {
   case WM_NCCREATE:
      SHGetMalloc(&g_pMalloc);
      break;

   case WM_CREATE:
      {
      HIMAGELIST  himlLarge;
      HIMAGELIST  himlSmall;
      HWND        hwndTree;
      HWND        hwndList;

      g_uPosition = 0;
      
      //get the system image lists
      himlLarge = Main_CreateImageList(TRUE);
      himlSmall = Main_CreateImageList(FALSE);

      // create the TreeView control
      hwndTree = Tree_Create(g_hInst, hWnd, himlSmall);

      // create the ListView control
      hwndList = List_Create(g_hInst, hWnd, himlLarge, himlSmall);
      
      //initialize the TreeView control
      Tree_Init(hwndTree);

      SetFocus(hwndTree);
      }
      break;

   case WM_NOTIFY:
      {
      LPNMHDR  pnmh = (LPNMHDR) lParam;

      switch(pnmh->idFrom)
         {
         case IDC_TREEVIEW:
            return Tree_Notify(hWnd, lParam);
         
         case IDC_LISTVIEW:
            return List_Notify(hWnd, lParam);
         }
      }
      return 0;
   
   case WM_SIZE:
      {
      Main_SizeChildren(hWnd);
      }
      break;
   
   case WM_LBUTTONDOWN:
      SetCapture(hWnd);
      return 0;

   case WM_LBUTTONUP:
      ReleaseCapture();
      return 0;

   case WM_MOUSEMOVE:
      //if the left button is down
      if(GetKeyState(VK_LBUTTON) & 0x8000)
         {
         RECT  rc;

         GetClientRect(hWnd, &rc);

         //don't do anything if we are already at the minimum size
         if((g_uPosition == MIN_SIZE) && (LOWORD(lParam) <= MIN_SIZE))
            break;
         
         //don't do anything if we are already at the maximum size
         if(((rc.right - g_uPosition) == MIN_SIZE) && (LOWORD(lParam) >= g_uPosition))
            break;
         
         g_uPosition = LOWORD(lParam);

         //check for min and max
         if(g_uPosition < MIN_SIZE)
            g_uPosition = MIN_SIZE;

         if((rc.right - g_uPosition) < MIN_SIZE)
            g_uPosition = rc.right - MIN_SIZE;
         
         Main_SizeChildren(hWnd);
         }
      break;

   case WM_COMMAND:
      return Main_OnCommand(hWnd, wParam, lParam);

   case WM_DESTROY:
      //tell the list to release its current folder
      List_ReleaseCurrentFolder();
      
      PostQuitMessage(0);
      break;

   case WM_NCDESTROY:
      g_pMalloc->Release();
      g_pMalloc = NULL;
      break;
   
   case WM_SYSCOLORCHANGE:
      {
      HBRUSH   hbr = GetSysColorBrush(COLOR_3DFACE);

      hbr = (HBRUSH)SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbr);

      if(hbr)
         DeleteObject(hbr);
      }
      break;
   
   case WM_INITMENUPOPUP:
      Main_UpdateMenu(hWnd, (HMENU)wParam);
      //fall through
   
   case WM_DRAWITEM:
   case WM_MENUCHAR:
   case WM_MEASUREITEM:
      if(g_pcm2)
         {
         g_pcm2->HandleMenuMsg(uMessage, wParam, lParam);
         }
      break;

   case WM_CONTEXTMENU:
      {
      POINT ptScreen;
      POINT ptClient;

      ptScreen.x = GET_X_LPARAM(lParam);
      ptScreen.y = GET_Y_LPARAM(lParam);
      ptClient = ptScreen;
      ScreenToClient(hWnd, &ptClient);
      HWND  hwndOver = ChildWindowFromPoint(hWnd, ptClient);

      if(GetDlgItem(hWnd, IDC_TREEVIEW) == hwndOver)
         {
         Tree_DoContextMenu(hwndOver, &ptScreen);
         }
      else if(GetDlgItem(hWnd, IDC_LISTVIEW) == hwndOver)
         {
         List_DoContextMenu(hwndOver, &ptScreen);
         }
      }
      break;
   
   default:
      break;
   }
return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

/******************************************************************************

   Main_CreateImageList

******************************************************************************/

HIMAGELIST Main_CreateImageList(BOOL fLarge)
{
HIMAGELIST hImageList;
SHFILEINFO  sfi;

//get the system image list
hImageList = (HIMAGELIST)SHGetFileInfo(   TEXT("C:\\"), 
                                          0,
                                          &sfi, 
                                          sizeof(SHFILEINFO), 
                                          SHGFI_SYSICONINDEX | (fLarge ? 0 : SHGFI_SMALLICON));

return hImageList;
}

/******************************************************************************

   Main_SizeChildren

******************************************************************************/

void Main_SizeChildren(HWND hwndParent)
{
RECT  rc;

GetClientRect(hwndParent, &rc);

if(!g_uPosition)
   {
   g_uPosition = rc.right/3;
   }

//resize the TreeView control
MoveWindow( GetDlgItem(hwndParent, IDC_TREEVIEW),
            0,
            0,
            g_uPosition - GAP_SIZE,
            rc.bottom,
            TRUE);

//resize the ListView
MoveWindow( GetDlgItem(hwndParent, IDC_LISTVIEW),
            g_uPosition,
            0,
            rc.right - g_uPosition,
            rc.bottom,
            TRUE);
}

/******************************************************************************

   Main_UpdateMenu

******************************************************************************/

void Main_UpdateMenu(HWND hWnd, HMENU hMenu)
{
UINT  uID = 0;
DWORD dwStyle;

//uncheck all of these guys
CheckMenuItem(hMenu, IDM_LARGE_ICONS,  MF_BYCOMMAND | MF_UNCHECKED);
CheckMenuItem(hMenu, IDM_SMALL_ICONS,  MF_BYCOMMAND | MF_UNCHECKED);
CheckMenuItem(hMenu, IDM_LIST,  MF_BYCOMMAND | MF_UNCHECKED);

//check the appropriate view menu item
dwStyle = GetWindowLong(GetDlgItem(hWnd, IDC_LISTVIEW), GWL_STYLE);
switch(dwStyle & LVS_TYPEMASK)
   {
   case LVS_ICON:
      uID = IDM_LARGE_ICONS;
      break;

   case LVS_SMALLICON:
      uID = IDM_SMALL_ICONS;
      break;

   case LVS_LIST:
      uID = IDM_LIST;
      break;
   }
CheckMenuRadioItem(hMenu, IDM_LARGE_ICONS, IDM_LIST, uID,  MF_BYCOMMAND | MF_CHECKED);
}

/**************************************************************************

   Main_OnCommand()

**************************************************************************/

LRESULT Main_OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
UNREFERENCED_PARAMETER(lParam);

switch (GET_WM_COMMAND_ID(wParam, lParam))
   {
   case IDM_EXIT:
      DestroyWindow(hWnd);
      break;

   case IDM_LARGE_ICONS:
      {
      HWND  hwndListView = GetDlgItem(hWnd, IDC_LISTVIEW);
      DWORD dwStyle = GetWindowLong(hwndListView, GWL_STYLE);
      SetWindowLong(hwndListView, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_ICON);
      }
      break;

   case IDM_SMALL_ICONS:
      {
      HWND  hwndListView = GetDlgItem(hWnd, IDC_LISTVIEW);
      DWORD dwStyle = GetWindowLong(hwndListView, GWL_STYLE);
      SetWindowLong(hwndListView, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_SMALLICON);
      }
      break;

   case IDM_LIST:
      {
      HWND  hwndListView = GetDlgItem(hWnd, IDC_LISTVIEW);
      DWORD dwStyle = GetWindowLong(hwndListView, GWL_STYLE);
      SetWindowLong(hwndListView, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | LVS_LIST);
      }
      break;

   
   case IDM_REFRESH:
      List_Refresh(GetDlgItem(hWnd, IDC_LISTVIEW));
      break;
   }

return 0;
}

/**************************************************************************

   Pidl_GetNextItem()

**************************************************************************/

inline LPITEMIDLIST Pidl_GetNextItem(LPCITEMIDLIST pidl)
{
if(pidl)
   return (LPITEMIDLIST)(LPBYTE)(((LPBYTE)pidl) + pidl->mkid.cb);
else
   return NULL;
}

/**************************************************************************

   Pidl_Create()

**************************************************************************/

LPITEMIDLIST Pidl_Create(UINT cbSize)
{
LPITEMIDLIST pidl = NULL;

pidl = (LPITEMIDLIST) g_pMalloc->Alloc(cbSize);
if(pidl)
   ZeroMemory(pidl, cbSize);

return pidl;
}

/**************************************************************************

   Pidl_GetSize()

**************************************************************************/

UINT Pidl_GetSize(LPCITEMIDLIST pidl)
{
UINT           cbTotal = 0;
LPITEMIDLIST   pidlTemp = (LPITEMIDLIST) pidl;

if(pidlTemp)
   {
   while(pidlTemp->mkid.cb)
      {
      cbTotal += pidlTemp->mkid.cb;
      pidlTemp = Pidl_GetNextItem(pidlTemp);
      }  

   // Requires a 16 bit zero value for the NULL terminator
   cbTotal += 2 * sizeof(BYTE);
   }

return cbTotal;
}

/**************************************************************************

   Pidl_Copy()

**************************************************************************/

LPITEMIDLIST Pidl_Copy(LPCITEMIDLIST pidlSource)
{
LPITEMIDLIST   pidlTarget = NULL;
UINT           cbSource = 0;

if(NULL == pidlSource)
   return NULL;

//
// Allocate the new ITEMIDLIST
//
cbSource = Pidl_GetSize(pidlSource);
pidlTarget = (LPITEMIDLIST) g_pMalloc->Alloc(cbSource);
if(!pidlTarget)
   return NULL;

//
// Copy the source to the target
//
CopyMemory(pidlTarget, pidlSource, cbSource);

return pidlTarget;
}

/**************************************************************************

   Pidl_Concatenate()

**************************************************************************/

LPITEMIDLIST Pidl_Concatenate(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
LPITEMIDLIST   pidlNew;
UINT           cb1 = 0, 
               cb2 = 0;

//
// Pidl1 can possibly be NULL if it points to the desktop.  Since we only
// need a single NULL terminator, we remove the extra 2 bytes from the
// size of the first ITEMIDLIST.
//
if(pidl1)
   cb1 = Pidl_GetSize(pidl1) - (2 * sizeof(BYTE));

cb2 = Pidl_GetSize(pidl2);

//
// Create a new ITEMIDLIST that is the size of both pidl1 and pidl2, then
// copy pidl1 and pidl2 to the new list.
//
pidlNew = Pidl_Create(cb1 + cb2);
if(pidlNew)
   {
   if(pidl1)   
      CopyMemory(pidlNew, pidl1, cb1);

   CopyMemory(((LPBYTE)pidlNew) + cb1, pidl2, cb2);
   }

return pidlNew;
}

/**************************************************************************

   Pidl_Free()

**************************************************************************/

void Pidl_Free(LPITEMIDLIST pidl)
{
if(pidl)
   g_pMalloc->Free(pidl);
}


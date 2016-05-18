/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************
   Include Files
**************************************************************************/

#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include "VListVw.h"

/**************************************************************************
   Local Function Prototypes
**************************************************************************/

#define ErrorHandler() ErrorHandlerEx(__LINE__, __FILE__)
void ErrorHandlerEx(WORD, LPSTR);

LRESULT ListViewNotify(HWND, LPARAM);
void SwitchView(HWND, DWORD);
BOOL DoContextMenu(HWND, WPARAM, LPARAM);
void UpdateMenu(HWND, HMENU);
BOOL InsertListViewItems(HWND);
void PositionHeader(HWND);

/**************************************************************************
   Global Variables
**************************************************************************/

HINSTANCE   g_hInst;
TCHAR       g_szClassName[] = TEXT("VListVwClass");

#define ITEM_COUNT   100000

/******************************************************************************

   WinMain

******************************************************************************/

int PASCAL WinMain(  HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
MSG  msg;

g_hInst = hInstance;

if(!hPrevInstance)
   if(!InitApplication(hInstance))
      return FALSE;

//required to use the common controls
InitCommonControls();

/* Perform initializations that apply to a specific instance */

if (!InitInstance(hInstance, nCmdShow))
   return FALSE;

/* Acquire and dispatch messages until a WM_QUIT uMessage is received. */

while(GetMessage( &msg, NULL, 0x00, 0x00))
   {
   TranslateMessage(&msg);
   DispatchMessage(&msg);
   }

return (int)msg.wParam;
}


/******************************************************************************

   InitApplication

******************************************************************************/

BOOL InitApplication(HINSTANCE hInstance)
{
WNDCLASSEX  wcex;
ATOM        aReturn;

wcex.cbSize          = sizeof(WNDCLASSEX);
wcex.style           = 0;
wcex.lpfnWndProc     = (WNDPROC)MainWndProc;
wcex.cbClsExtra      = 0;
wcex.cbWndExtra      = 0;
wcex.hInstance       = hInstance;
wcex.hCursor         = LoadCursor(NULL, IDC_ARROW);
wcex.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);
wcex.lpszMenuName    = MAKEINTRESOURCE(IDM_MAIN_MENU);
wcex.lpszClassName   = g_szClassName;
wcex.hIcon           = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_MAINICON));
wcex.hIconSm         = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, 0);

aReturn = RegisterClassEx(&wcex);

if(0 == aReturn)
   {
   WNDCLASS wc;

   wc.style          = 0;
   wc.lpfnWndProc    = (WNDPROC)MainWndProc;
   wc.cbClsExtra     = 0;
   wc.cbWndExtra     = 0;
   wc.hInstance      = hInstance;
   wc.hIcon          = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_MAINICON));
   wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
   wc.lpszMenuName   = MAKEINTRESOURCE(IDM_MAIN_MENU);
   wc.lpszClassName  = g_szClassName;

   aReturn = RegisterClass(&wc);
   }

return aReturn;
}


/******************************************************************************

   InitInstance

******************************************************************************/

BOOL InitInstance(   HINSTANCE hInstance,
                     int nCmdShow)
{
HWND     hWnd;
TCHAR    szTitle[MAX_PATH] = TEXT("");

g_hInst = hInstance;

LoadString(g_hInst, IDS_APPTITLE, szTitle, sizeof(szTitle)/sizeof(szTitle[0]));

/* Create a main window for this application instance.  */
hWnd = CreateWindowEx(  0,
                        g_szClassName,
                        szTitle,
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

if (!hWnd)
   return FALSE;

/* Make the window visible; update its client area; and return "success" */

ShowWindow(hWnd, nCmdShow);
UpdateWindow(hWnd);
return TRUE;

}

/******************************************************************************

   MainWndProc

******************************************************************************/

LRESULT CALLBACK MainWndProc( HWND hWnd,
                              UINT uMessage,
                              WPARAM wParam,
                              LPARAM lParam)
{
static HWND hwndListView;

switch (uMessage)
   {
   case WM_CREATE:
      // create the TreeView control
      hwndListView = CreateListView(g_hInst, hWnd);
      
      //initialize the TreeView control
      InitListView(hwndListView);
      
      break;

   case WM_NOTIFY:
      return ListViewNotify(hWnd, lParam);
   
   case WM_SIZE:
      ResizeListView(hwndListView, hWnd);
      break;

   case WM_INITMENUPOPUP:
      UpdateMenu(hwndListView, GetMenu(hWnd));
      break;
   
   case WM_CONTEXTMENU:
      if(DoContextMenu(hWnd, wParam, lParam))
         return FALSE;
      break;
   
   case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam, lParam))
         {
         case IDM_LARGE_ICONS:
            SwitchView(hwndListView, LVS_ICON);
            break;
         
         case IDM_SMALL_ICONS:
            SwitchView(hwndListView, LVS_SMALLICON);
            break;
         
         case IDM_LIST:
            SwitchView(hwndListView, LVS_LIST);
            break;
         
         case IDM_REPORT:
            SwitchView(hwndListView, LVS_REPORT);
            break;
         
         case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
         
         case IDM_ABOUT:
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlgProc);
            break;   

         }
      break;

   case WM_DESTROY:
      PostQuitMessage(0);
      break;

   default:
      break;
   }
return DefWindowProc(hWnd, uMessage, wParam, lParam);
}


/******************************************************************************

   AboutDlgProc

******************************************************************************/

INT_PTR CALLBACK AboutDlgProc(   HWND hDlg, 
                                 UINT uMessage, 
                                 WPARAM wParam, 
                                 LPARAM lParam)
{
switch (uMessage)
   {
   case WM_INITDIALOG:
      return TRUE;
      
   case WM_COMMAND:
      switch(wParam)
         {
         case IDOK:
            EndDialog(hDlg, IDOK);
            break;

         case IDCANCEL:
            EndDialog(hDlg, IDOK);
            break;
         }
      return TRUE;
    } 
    
return FALSE;
}

/******************************************************************************

   CreateListView

******************************************************************************/

HWND CreateListView(HINSTANCE hInstance, HWND hwndParent)
{
DWORD       dwStyle;
HWND        hwndListView;
HIMAGELIST  himlSmall;
HIMAGELIST  himlLarge;
BOOL        bSuccess = TRUE;

dwStyle =   WS_TABSTOP | 
            WS_CHILD | 
            WS_BORDER | 
            WS_VISIBLE |
            LVS_AUTOARRANGE |
            LVS_REPORT | 
            LVS_OWNERDATA;
            
hwndListView = CreateWindowEx(   WS_EX_CLIENTEDGE,          // ex style
                                 WC_LISTVIEW,               // class name - defined in commctrl.h
                                 TEXT(""),                        // dummy text
                                 dwStyle,                   // style
                                 0,                         // x position
                                 0,                         // y position
                                 0,                         // width
                                 0,                         // height
                                 hwndParent,                // parent
                                 (HMENU)ID_LISTVIEW,        // ID
                                 g_hInst,                   // instance
                                 NULL);                     // no extra data

if(!hwndListView)
   return NULL;

ResizeListView(hwndListView, hwndParent);

//set the image lists
himlSmall = ImageList_Create(16, 16, ILC_COLORDDB | ILC_MASK, 1, 0);
himlLarge = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK, 1, 0);

if (himlSmall && himlLarge)
   {
   HICON hIcon;

   //set up the small image list
   hIcon = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_DISK), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
   ImageList_AddIcon(himlSmall, hIcon);

   //set up the large image list
   hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DISK));
   ImageList_AddIcon(himlLarge, hIcon);

   ListView_SetImageList(hwndListView, himlSmall, LVSIL_SMALL);
   ListView_SetImageList(hwndListView, himlLarge, LVSIL_NORMAL);
   }

return hwndListView;
}

/******************************************************************************

   ResizeListView

******************************************************************************/

void ResizeListView(HWND hwndListView, HWND hwndParent)
{
RECT  rc;

GetClientRect(hwndParent, &rc);

MoveWindow( hwndListView, 
            rc.left,
            rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            TRUE);

//only call this if we want the LVS_NOSCROLL style
//PositionHeader(hwndListView);
}

/******************************************************************************

   PositionHeader

   this needs to be called when the ListView is created, resized, the view is 
   changed or a WM_SYSPARAMETERCHANGE message is received

******************************************************************************/

void PositionHeader(HWND hwndListView)
{
HWND  hwndHeader = GetWindow(hwndListView, GW_CHILD);
DWORD dwStyle = GetWindowLong(hwndListView, GWL_STYLE);

/*To ensure that the first item will be visible, create the control without 
the LVS_NOSCROLL style and then add it here*/
dwStyle |= LVS_NOSCROLL;
SetWindowLong(hwndListView, GWL_STYLE, dwStyle);

//only do this if we are in report view and were able to get the header hWnd
if(((dwStyle & LVS_TYPEMASK) == LVS_REPORT) && hwndHeader)
   {
   RECT        rc;
   HD_LAYOUT   hdLayout;
   WINDOWPOS   wpos;

   GetClientRect(hwndListView, &rc);
   hdLayout.prc = &rc;
   hdLayout.pwpos = &wpos;

   Header_Layout(hwndHeader, &hdLayout);

   SetWindowPos(  hwndHeader, 
                  wpos.hwndInsertAfter, 
                  wpos.x, 
                  wpos.y,
                  wpos.cx, 
                  wpos.cy, 
                  wpos.flags | SWP_SHOWWINDOW);

   ListView_EnsureVisible(hwndListView, 0, FALSE);
   }
}

/******************************************************************************

   InitListView

******************************************************************************/

BOOL InitListView(HWND hwndListView)
{
LV_COLUMN   lvColumn;
int         i;
TCHAR       szString[5][20] = {TEXT("Main Column"), TEXT("Column 1"), TEXT("Column 2"), TEXT("Column 3"), TEXT("Column 4")};

//empty the list
ListView_DeleteAllItems(hwndListView);

//initialize the columns
lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
lvColumn.fmt = LVCFMT_LEFT;
lvColumn.cx = 120;
for(i = 0; i < 5; i++)
   {
   lvColumn.pszText = szString[i];
   ListView_InsertColumn(hwndListView, i, &lvColumn);
   }

InsertListViewItems(hwndListView);

return TRUE;
}

/******************************************************************************

   InsertListViewItems

******************************************************************************/

BOOL InsertListViewItems(HWND hwndListView)
{
//empty the list
ListView_DeleteAllItems(hwndListView);

//set the number of items in the list
ListView_SetItemCount(hwndListView, ITEM_COUNT);

return TRUE;
}

/**************************************************************************

   ListViewNotify()

**************************************************************************/

LRESULT ListViewNotify(HWND hWnd, LPARAM lParam)
{
LPNMHDR  lpnmh = (LPNMHDR) lParam;
HWND     hwndListView = GetDlgItem(hWnd, ID_LISTVIEW);

switch(lpnmh->code)
   {
   case LVN_GETDISPINFO:
      {
      LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;
      TCHAR szString[MAX_PATH];

      if(lpdi->item.iSubItem)
         {
         if(lpdi->item.mask & LVIF_TEXT)
            {
            _sntprintf_s(szString, _countof(szString), _TRUNCATE,
						 TEXT("Item %d - Column %d"),
						 lpdi->item.iItem + 1, lpdi->item.iSubItem);
            _tcsncpy_s(lpdi->item.pszText, lpdi->item.cchTextMax,
					   szString, _TRUNCATE);
            }
         }
      else
         {
         if(lpdi->item.mask & LVIF_TEXT)
            {
            _sntprintf_s(szString, _countof(szString), _TRUNCATE,
						 TEXT("Item %d"), lpdi->item.iItem + 1);
            _tcsncpy_s(lpdi->item.pszText, lpdi->item.cchTextMax,
					   szString, _TRUNCATE);
            }

         if(lpdi->item.mask & LVIF_IMAGE)
            {
            lpdi->item.iImage = 0;
            }
         }
      }
      return 0;

   case LVN_ODCACHEHINT:
      {
      LPNMLVCACHEHINT   lpCacheHint = (LPNMLVCACHEHINT)lParam;
      /*
      This sample doesn't use this notification, but this is sent when the 
      ListView is about to ask for a range of items. On this notification, 
      you should load the specified items into your local cache. It is still 
      possible to get an LVN_GETDISPINFO for an item that has not been cached, 
      therefore, your application must take into account the chance of this 
      occurring.
      */
      }
      return 0;

   case LVN_ODFINDITEM:
      {
      LPNMLVFINDITEM lpFindItem = (LPNMLVFINDITEM)lParam;
      /*
      This sample doesn't use this notification, but this is sent when the 
      ListView needs a particular item. Return -1 if the item is not found.
      */
      }
      return 0;
   }

return 0;
}

/**************************************************************************

   ErrorHandlerEx()

**************************************************************************/

void ErrorHandlerEx( WORD wLine, LPSTR lpszFile )
{
LPVOID lpvMessage;
DWORD  dwError;
TCHAR  szBuffer[256];

// Allow FormatMessage() to look up the error code returned by GetLastError
dwError = FormatMessage(   FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                              FORMAT_MESSAGE_FROM_SYSTEM, 
                           NULL, 
                           GetLastError(), 
                           MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
                           (LPTSTR)&lpvMessage, 
                           0, 
                           NULL);

// Check to see if an error occurred calling FormatMessage()
if (0 == dwError)
   {
   _sntprintf_s(szBuffer, _countof(szBuffer), _TRUNCATE,
				TEXT("An error occurred calling FormatMessage().")
				TEXT("Error Code %d"), 
				GetLastError());
   MessageBox( NULL, 
               szBuffer, 
               TEXT("Generic"), 
               MB_ICONSTOP | MB_ICONEXCLAMATION);
   return;
   }

// Display the error information along with the place the error happened.
_sntprintf_s(szBuffer, _countof(szBuffer), _TRUNCATE,
			 TEXT("Generic, Line=%d, File=%s"), wLine, lpszFile);
MessageBox(NULL, lpvMessage, szBuffer, MB_ICONEXCLAMATION | MB_OK);
}

/**************************************************************************

   SwitchView()

**************************************************************************/

void SwitchView(HWND hwndListView, DWORD dwView)
{
DWORD dwStyle = GetWindowLong(hwndListView, GWL_STYLE);

SetWindowLong(hwndListView, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwView);
ResizeListView(hwndListView, GetParent(hwndListView));
}

/**************************************************************************

   DoContextMenu()

**************************************************************************/

BOOL DoContextMenu(  HWND hWnd, 
                     WPARAM wParam, 
                     LPARAM lParam)
{
HWND  hwndListView = (HWND)wParam;
HMENU hMenuLoad,
      hMenu;

if(hwndListView != GetDlgItem(hWnd, ID_LISTVIEW))
   return FALSE;

hMenuLoad = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_CONTEXT_MENU));
hMenu = GetSubMenu(hMenuLoad, 0);

UpdateMenu(hwndListView, hMenu);

TrackPopupMenu(   hMenu,
                  TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                  LOWORD(lParam),
                  HIWORD(lParam),
                  0,
                  hWnd,
                  NULL);

DestroyMenu(hMenuLoad);

return TRUE;
}

/**************************************************************************

   UpdateMenu()

**************************************************************************/

void UpdateMenu(HWND hwndListView, HMENU hMenu)
{
UINT  uID = IDM_LIST;
DWORD dwStyle;

//uncheck all of these guys
CheckMenuItem(hMenu, IDM_LARGE_ICONS,  MF_BYCOMMAND | MF_UNCHECKED);
CheckMenuItem(hMenu, IDM_SMALL_ICONS,  MF_BYCOMMAND | MF_UNCHECKED);
CheckMenuItem(hMenu, IDM_LIST,  MF_BYCOMMAND | MF_UNCHECKED);
CheckMenuItem(hMenu, IDM_REPORT,  MF_BYCOMMAND | MF_UNCHECKED);

//check the appropriate view menu item
dwStyle = GetWindowLong(hwndListView, GWL_STYLE);
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
   
   case LVS_REPORT:
      uID = IDM_REPORT;
      break;
   }
CheckMenuRadioItem(hMenu, IDM_LARGE_ICONS, IDM_REPORT, uID,  MF_BYCOMMAND | MF_CHECKED);

}

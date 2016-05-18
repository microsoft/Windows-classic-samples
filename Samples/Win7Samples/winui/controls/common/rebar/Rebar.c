/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          ReBar.c
   
   Description:   ReBar sample implementation.

**************************************************************************/

#define STRICT

/**************************************************************************
   Include Files
**************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"

/**************************************************************************
   Local Function Prototypes
**************************************************************************/

int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
HWND BuildRebar(HWND);
void MoveRebar(HWND);
LRESULT HandleMenuPopup(HMENU);
LRESULT HandleCommand(HWND, WPARAM, LPARAM);

/**************************************************************************
   Global Variables
**************************************************************************/

#define ID_REBAR     1000
#define ID_BUTTON    2000
#define ID_COMBOBOX  2001

#define TOP    0x00
#define LEFT   0x01
#define BOTTOM 0x02
#define RIGHT  0x03

HINSTANCE   g_hInst;
WORD        g_wSide;

/******************************************************************************

   WinMain

   Parameters:

   Description:

   Returns:

******************************************************************************/

int PASCAL WinMain(  HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
MSG      msg;
INITCOMMONCONTROLSEX iccex;

iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
iccex.dwICC = ICC_COOL_CLASSES;
InitCommonControlsEx(&iccex);

if(!hPrevInstance)
   if(!InitApplication(hInstance))
      return FALSE;

if (!InitInstance(hInstance, nCmdShow))
   return FALSE;

while(GetMessage( &msg, NULL, 0x00, 0x00))
   {
   TranslateMessage(&msg);
   DispatchMessage(&msg);
   }

return (int)msg.wParam;
}

/*****************************************************************************

   InitApplication

   Parameters:

   Description:
   
   Returns:

*****************************************************************************/

BOOL InitApplication(HINSTANCE hInstance)
{
WNDCLASS  wc;

wc.style          = 0;
wc.lpfnWndProc    = (WNDPROC)MainWndProc;
wc.cbClsExtra     = 0;
wc.cbWndExtra     = 0;
wc.hInstance      = hInstance;
wc.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName   = MAKEINTRESOURCE(IDM_GENERICMENU);
wc.lpszClassName  = TEXT("GenericClass");

return RegisterClass(&wc);
}

/*****************************************************************************

   InitInstance

   Parameters:

   Description:
   
   Returns:

*****************************************************************************/

BOOL InitInstance(   HINSTANCE hInstance,
                     int nCmdShow)
{
HWND  hWnd;

g_hInst = hInstance;

hWnd = CreateWindowEx(  0,
                        TEXT("GenericClass"),
                        TEXT("Generic Application"),
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);

if (!hWnd)
   return FALSE;

ShowWindow(hWnd, nCmdShow);
UpdateWindow(hWnd);

return TRUE;

}

/*****************************************************************************

   MainWndProc

   Parameters:

   Description:
   
   Returns:

*****************************************************************************/

LRESULT CALLBACK MainWndProc( HWND hWnd,
                              UINT uMessage,
                              WPARAM wParam,
                              LPARAM lParam)
{
switch (uMessage)
   {
   case WM_CREATE:
      g_wSide = TOP;

      BuildRebar(hWnd);
      break;
   
   case WM_SIZE:
      MoveRebar(hWnd);
      break;

   case WM_DESTROY:
      PostQuitMessage(0);
      break;

   case WM_INITMENUPOPUP:
      return HandleMenuPopup((HMENU)wParam);
   
   case WM_COMMAND:
      return HandleCommand(hWnd, wParam, lParam);
      
   default:
      break;
   }
return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

/*****************************************************************************

   About

   Parameters:

   Description:
   
   Returns:

*****************************************************************************/

INT_PTR CALLBACK About( HWND hDlg, 
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
            return TRUE;

         case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
         }
      break;
    } 
    
return FALSE;
}

/*****************************************************************************

   BuildRebar

*****************************************************************************/

HWND BuildRebar(HWND hwndParent)
{
HWND     hwndRebar = NULL;
LRESULT  lResult;

hwndRebar = CreateWindowEx(   WS_EX_TOOLWINDOW, 
                              REBARCLASSNAME, 
                              NULL,
                              WS_VISIBLE |
                                 WS_BORDER | 
                                 WS_CHILD | 
                                 WS_CLIPCHILDREN | 
                                 WS_CLIPSIBLINGS | 
                                 RBS_VARHEIGHT | 
                                 RBS_BANDBORDERS | 
                                 ((g_wSide & 0x01) ? CCS_VERT : 0) | //g_wSide is odd if this is a vertical bar
                                 ((g_wSide == BOTTOM) ? CCS_BOTTOM : 0) |
                                 ((g_wSide == RIGHT) ? CCS_RIGHT : 0) |
                                 0,
                              0, 
                              0, 
                              200, 
                              100, 
                              hwndParent, 
                              (HMENU)ID_REBAR, 
                              g_hInst, 
                              NULL);

if(hwndRebar)
   {
   REBARINFO      rbi;
   HIMAGELIST     himlRebar;
   HICON          hIcon;
   REBARBANDINFO  rbbi;
   HWND           hwndChild;
   RECT           rc;
   TCHAR          szString[64];

   //set up the ReBar
   himlRebar = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK, 1, 0);
   hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));

   ImageList_AddIcon(himlRebar, hIcon);

   rbi.cbSize  = sizeof(rbi);
   rbi.fMask   = RBIM_IMAGELIST;
   rbi.himl    = himlRebar;
   lResult = SendMessage(hwndRebar, RB_SETBARINFO, 0, (LPARAM)&rbi);

   //add a band that contains a combobox
   hwndChild = CreateWindowEx(   0, 
                                 TEXT("combobox"), 
                                 NULL,
                                 WS_VISIBLE |
                                    WS_CHILD | 
                                    WS_TABSTOP |
                                    WS_VSCROLL |
                                    WS_CLIPCHILDREN | 
                                    WS_CLIPSIBLINGS | 
                                    CBS_AUTOHSCROLL | 
                                    CBS_DROPDOWN | 
                                    0,
                                 0, 
                                 0, 
                                 100, 
                                 200, 
                                 hwndRebar, 
                                 (HMENU)ID_COMBOBOX, 
                                 g_hInst, 
                                 NULL);

   //add some stuff to the combobox
   {
   int   i;

   SendMessage(hwndChild, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(TRUE, 0));
   
   for(i = 0; i < 25; i++)
      {
      _stprintf_s(szString, _countof(szString), TEXT("Item %d"), i + 1);
      SendMessage(hwndChild, CB_ADDSTRING, 0, (LPARAM)szString);
      }
   }
   
   GetWindowRect(hwndChild, &rc);
   
   ZeroMemory(&rbbi, sizeof(rbbi));
   rbbi.cbSize       = sizeof(REBARBANDINFO);
   rbbi.fMask        = RBBIM_SIZE | 
                        RBBIM_CHILD | 
                        RBBIM_CHILDSIZE | 
                        RBBIM_ID | 
                        RBBIM_STYLE | 
                        RBBIM_TEXT |
                        RBBIM_BACKGROUND |
                        RBBIM_IMAGE |
                        0;
   
   rbbi.cxMinChild   = rc.right - rc.left;
   rbbi.cyMinChild   = rc.bottom - rc.top;
   rbbi.cx           = 100;
   rbbi.fStyle       = RBBS_CHILDEDGE | 
                        RBBS_FIXEDBMP |
                        RBBS_GRIPPERALWAYS |
                        0;
   rbbi.wID          = ID_COMBOBOX;
   rbbi.hwndChild    = hwndChild;
   rbbi.lpText       = TEXT("ComboBox");
   rbbi.cch          = 2;
   rbbi.hbmBack      = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BACKGROUND));
   rbbi.iImage       = 0;

   lResult = SendMessage(hwndRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)(LPREBARBANDINFO)&rbbi);

   //add a band that contains a button
   hwndChild = CreateWindowEx(   0, 
                                 TEXT("button"), 
                                 TEXT("Button"),
                                 WS_CHILD | 
                                    BS_PUSHBUTTON | 
                                    0,
                                 0, 
                                 0, 
                                 100, 
                                 50, 
                                 hwndRebar, 
                                 (HMENU)ID_BUTTON, 
                                 g_hInst, 
                                 NULL);
   
   GetWindowRect(hwndChild, &rc);
   
   ZeroMemory(&rbbi, sizeof(rbbi));
   rbbi.cbSize       = sizeof(REBARBANDINFO);
   rbbi.fMask        = RBBIM_SIZE | 
                        RBBIM_CHILD | 
                        RBBIM_CHILDSIZE | 
                        RBBIM_ID | 
                        RBBIM_STYLE | 
                        RBBIM_TEXT |
                        RBBIM_BACKGROUND |
                        0;
   rbbi.cxMinChild   = rc.right - rc.left;
   rbbi.cyMinChild   = rc.bottom - rc.top;
   rbbi.cx           = 100;
   rbbi.fStyle       = RBBS_CHILDEDGE | 
                        RBBS_FIXEDBMP |
                        RBBS_GRIPPERALWAYS |
                        0;
   rbbi.wID          = ID_BUTTON;
   rbbi.hwndChild    = hwndChild;
   rbbi.lpText       = TEXT("Button");
   rbbi.hbmBack      = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BACKGROUND));


   lResult = SendMessage(hwndRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)(LPREBARBANDINFO)&rbbi);
   }

return hwndRebar;
}


/**************************************************************************

   MoveRebar()
   
**************************************************************************/

void MoveRebar(HWND hWnd)
{
RECT  rc,
      rcRebar;
int   x,
      y,
      cx,
      cy;

GetClientRect(hWnd, &rc);
GetWindowRect(GetDlgItem(hWnd, ID_REBAR), &rcRebar);

switch(g_wSide)
   {
   default:
   case TOP:
      //align the rebar along the top of the window
      x = 0;
      y = 0;
      cx = rc.right - rc.left;
      cy = rc.bottom - rc.top;
      break;

   case LEFT:
      //align the rebar along the left side of the window
      x = 0;
      y = 0;
      cx = rcRebar.right - rcRebar.left;
      cy = rc.bottom - rc.top;
      break;

   case BOTTOM:
      //align the rebar along the bottom of the window
      x = 0;
      y = rc.bottom - (rcRebar.bottom - rcRebar.top);
      cx = rc.right - rc.left;
      cy = rcRebar.bottom - rcRebar.top;
      break;

   case RIGHT:
      //align the coolbar along the right side of the window
      x = rc.right - (rcRebar.right - rcRebar.left);
      y = 0;
      cx = rcRebar.right - rcRebar.left;
      cy = rc.bottom - rc.top;
      break;
   }

MoveWindow( GetDlgItem(hWnd, ID_REBAR), 
            x, 
            y, 
            cx, 
            cy, 
            TRUE);
}

/**************************************************************************

   HandleMenuPopup()
   
**************************************************************************/

LRESULT HandleMenuPopup(HMENU hMenu)
{
UINT  uSelect;

switch(g_wSide)
   {
   default:
   case TOP:
      uSelect = IDM_TOP;
      break;

   case LEFT:
      uSelect = IDM_LEFT;
      break;

   case BOTTOM:
      uSelect = IDM_BOTTOM;
      break;

   case RIGHT:
      uSelect = IDM_RIGHT;
      break;
   }

CheckMenuRadioItem(hMenu, IDM_TOP, IDM_BOTTOM, uSelect, MF_BYCOMMAND);

return 0;
}

/**************************************************************************

   HandleCommand()
   
**************************************************************************/

LRESULT HandleCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
switch(GET_WM_COMMAND_ID(wParam, lParam))
   {
   case ID_BUTTON:
      break;

   case ID_COMBOBOX:
      break;

   case IDM_TOP:
      if(g_wSide != TOP)
         {
         //destroy the existing Rebar
         DestroyWindow(GetDlgItem(hWnd, ID_REBAR));
         
         //change to the new side
         g_wSide = TOP;

         //create the new Rebar
         BuildRebar(hWnd);

         }
      break;

   case IDM_BOTTOM:
      if(g_wSide != BOTTOM)
         {
         //destroy the existing Rebar
         DestroyWindow(GetDlgItem(hWnd, ID_REBAR));
         
         g_wSide = BOTTOM;

         //create the new Rebar
         BuildRebar(hWnd);

         }
      break;

   case IDM_LEFT:
      if(g_wSide != LEFT)
         {
         //destroy the existing Rebar
         DestroyWindow(GetDlgItem(hWnd, ID_REBAR));
         
         g_wSide = LEFT;

         //create the new Rebar
         BuildRebar(hWnd);

         }
      break;

   case IDM_RIGHT:
      if(g_wSide != RIGHT)
         {
         //destroy the existing Rebar
         DestroyWindow(GetDlgItem(hWnd, ID_REBAR));
         
         g_wSide = RIGHT;

         //create the new Rebar
         BuildRebar(hWnd);

         }
      break;
   
   case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
   
   case IDM_ABOUT:
      DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
      break;   
      
   }

return TRUE;
}

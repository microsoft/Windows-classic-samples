/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright (C) 1993 - 2000  Microsoft Corporation.  All Rights Reserved.

   MODULE:     Property.c

   PURPOSE:    Source module for the PROPERTY sample application

   PLATFORMS:  Windows 95, Windows NT

   FUNCTIONS:
      WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
      InitApplication(HINSTANCE);
      InitInstance(HINSTANCE, int);
      MainWndProc(HWND, UINT, WPARAM, LPARAM);
      About(HWND, UINT, WPARAM, LPARAM);
      DoModalPropSheet(HWND);
      DoModelessPropSheet(HWND);
      DoWizardPropSheet(HWND);
      ButtonsDlgProc(HWND, UINT, WPARAM, LPARAM);
      ComboDlgProc(HWND, UINT, WPARAM, LPARAM);
      PropSheetCallback(HWND, UINT, LPARAM);

   COMMENTS:
      
**************************************************************************/

#define STRICT

/**************************************************************************
   Include Files
**************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"

/**************************************************************************
   Local Function Prototypes
**************************************************************************/

int PASCAL       WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
BOOL             InitApplication(HINSTANCE);
BOOL             InitInstance(HINSTANCE, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR          DoModalPropSheet(HWND);
HWND             DoModelessPropSheet(HWND);
INT_PTR          DoWizardPropSheet(HWND);
LRESULT CALLBACK ButtonsDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ComboDlgProc(HWND, UINT, WPARAM, LPARAM);
void CALLBACK    PropSheetCallback(HWND, UINT, LPARAM);

/**************************************************************************
   Global Variables
**************************************************************************/

HINSTANCE   g_hInst;
HWND        g_hwndPropSheet,
            g_hwndMain;
BOOL        g_bWin95;
TCHAR       g_szClassName[] = TEXT("PropSheetClass");

/**************************************************************************

   WinMain()

**************************************************************************/

int PASCAL WinMain(  HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
MSG      msg;

g_hInst = hInstance;

//don't forget this
InitCommonControls();

if(!hPrevInstance)
   if(!InitApplication(hInstance))
      return FALSE;

if (!InitInstance(hInstance, nCmdShow))
   return FALSE;

while(GetMessage(&msg, NULL, 0x00, 0x00))
   {
   // If the modeless guy is up and is ready to be destroyed
   // (PropSheet_GetCurrentPageHwnd returns NULL) then destroy the dialog.
   
   // PropSheet_GetCurrentPageHwnd will return NULL after the OK or Cancel 
   // button has been pressed and all of the pages have been notified. The 
   // Apply button doesn't cause this to happen.
   if(g_hwndPropSheet && (NULL == PropSheet_GetCurrentPageHwnd(g_hwndPropSheet)))
      {
      //enable the parent first to prevent another window from becoming the foreground window
      EnableWindow(g_hwndMain, TRUE);
      DestroyWindow(g_hwndPropSheet);
      g_hwndPropSheet = NULL;
      }

   //use PropSheet_IsDialogMessage instead of IsDialogMessage
   if(!PropSheet_IsDialogMessage(g_hwndPropSheet, &msg))
      {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      }
   }

return (int)msg.wParam;
}

/**************************************************************************

   InitApplication()

**************************************************************************/

BOOL InitApplication(HINSTANCE hInstance)
{
OSVERSIONINFO  os;

ZeroMemory(&os, sizeof(os));
os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
GetVersionEx(&os);
if(os.dwMajorVersion >= 4)
   g_bWin95 = TRUE;
else
   g_bWin95 = FALSE;

if(g_bWin95)
   {
   WNDCLASSEX  wcex;

   ZeroMemory(&wcex, sizeof(wcex));
   
   wcex.cbSize          = sizeof(wcex);
   wcex.style           = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc     = (WNDPROC)MainWndProc;
   wcex.cbClsExtra      = 0;
   wcex.cbWndExtra      = 0;
   wcex.hInstance       = hInstance;
   wcex.hIcon           = LoadIcon(NULL, IDI_APPLICATION);
   wcex.hCursor         = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground   = GetStockObject(WHITE_BRUSH);
   wcex.lpszMenuName    = MAKEINTRESOURCE(IDR_MAIN_MENU);
   wcex.lpszClassName   = g_szClassName;

   return RegisterClassEx(&wcex);
   }
else
   {
   WNDCLASS  wc;

   ZeroMemory(&wc, sizeof(wc));
   
   wc.style          = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc    = (WNDPROC)MainWndProc;
   wc.cbClsExtra     = 0;
   wc.cbWndExtra     = 0;
   wc.hInstance      = hInstance;
   wc.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
   wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
   wc.hbrBackground  = GetStockObject(WHITE_BRUSH);
   wc.lpszMenuName   = MAKEINTRESOURCE(IDR_MAIN_MENU);
   wc.lpszClassName  = g_szClassName;

   return RegisterClass(&wc);
   }
}

/**************************************************************************

   InitInstance()

**************************************************************************/

BOOL InitInstance(   HINSTANCE hInstance,
                     int nCmdShow)
{
g_hwndMain = CreateWindowEx(  0,
                              g_szClassName,
                              TEXT("Property Sheet Application"),
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              NULL,
                              NULL,
                              hInstance,
                              NULL);

if (!g_hwndMain)
   {
   return FALSE;
   }

ShowWindow(g_hwndMain, nCmdShow);
UpdateWindow(g_hwndMain);

return TRUE;
}

/**************************************************************************

   MainWndProc()

**************************************************************************/

LRESULT CALLBACK MainWndProc( HWND hWnd,
                              UINT uMessage,
                              WPARAM wParam,
                              LPARAM lParam)
{
switch (uMessage)
   {
   case WM_CREATE:
      break;

   case WM_CLOSE:
      if(IsWindow(g_hwndPropSheet))
         DestroyWindow(g_hwndPropSheet);
      
      DestroyWindow(hWnd);
      break;
   
   case WM_DESTROY:
      PostQuitMessage(0);
      break;

   case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam, lParam))
         {
         case IDM_MODAL:
            DoModalPropSheet(hWnd);
            break;
            
         case IDM_MODELESS:
            g_hwndPropSheet = DoModelessPropSheet(hWnd);
            break;
            
         case IDM_WIZARD:
            DoWizardPropSheet(hWnd);
            break;
         
         case IDM_EXIT:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;
         
         case IDM_ABOUT:
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT_DIALOG), hWnd, About);
            break;
         }
      return TRUE;

   default:
      break;
   }
return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

/**************************************************************************

   About()

**************************************************************************/

INT_PTR CALLBACK About( HWND hWnd, 
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
            EndDialog(hWnd, IDOK);
            return TRUE;

         case IDCANCEL:
            EndDialog(hWnd, IDCANCEL);
            return TRUE;
         }
      break;
    } 
    
return FALSE;
}

/**************************************************************************

   DoModalPropSheet()

**************************************************************************/

INT_PTR DoModalPropSheet(HWND hwndOwner)
{
PROPSHEETPAGE psp[2];
PROPSHEETHEADER psh;

//Fill out the PROPSHEETPAGE data structure for the Background Color sheet
psp[0].dwSize        = sizeof(PROPSHEETPAGE);
psp[0].dwFlags       = PSP_USETITLE;
psp[0].hInstance     = g_hInst;
psp[0].pszTemplate   = MAKEINTRESOURCE(IDD_BUTTONS);
psp[0].pszIcon       = NULL;
psp[0].pfnDlgProc    = ButtonsDlgProc;
psp[0].pszTitle      = TEXT("Buttons");
psp[0].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Client Area Shape sheet
psp[1].dwSize        = sizeof(PROPSHEETPAGE);
psp[1].dwFlags       = PSP_USETITLE;
psp[1].hInstance     = g_hInst;
psp[1].pszTemplate   = MAKEINTRESOURCE(IDD_COMBOBOXES);
psp[1].pszIcon       = NULL;
psp[1].pfnDlgProc    = ComboDlgProc;
psp[1].pszTitle      = TEXT("Combo Boxes");
psp[1].lParam        = 0;

//Fill out the PROPSHEETHEADER
psh.dwSize           = sizeof(PROPSHEETHEADER);
psh.dwFlags          = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_USECALLBACK;
psh.hwndParent       = hwndOwner;
psh.hInstance        = g_hInst;
psh.pszIcon          = MAKEINTRESOURCE(IDI_BACKCOLOR);
psh.pszCaption       = TEXT("Modal Property Sheet");
psh.nPages           = sizeof(psp) / sizeof(PROPSHEETPAGE);
psh.ppsp             = (LPCPROPSHEETPAGE) &psp;
psh.pfnCallback      = (PFNPROPSHEETCALLBACK)PropSheetCallback;

//And finally display the modal property sheet
return PropertySheet(&psh);
}

/**************************************************************************

   DoModelessPropSheet()

**************************************************************************/

HWND DoModelessPropSheet(HWND hwndOwner)
{
PROPSHEETPAGE psp[2];
PROPSHEETHEADER psh;

//Fill out the PROPSHEETPAGE data structure for the Background Color sheet
psp[0].dwSize        = sizeof(PROPSHEETPAGE);
psp[0].dwFlags       = PSP_USETITLE;
psp[0].hInstance     = g_hInst;
psp[0].pszTemplate   = MAKEINTRESOURCE(IDD_BUTTONS);
psp[0].pszIcon       = NULL;
psp[0].pfnDlgProc    = ButtonsDlgProc;
psp[0].pszTitle      = TEXT("Buttons");
psp[0].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Client Area Shape sheet
psp[1].dwSize        = sizeof(PROPSHEETPAGE);
psp[1].dwFlags       = PSP_USETITLE;
psp[1].hInstance     = g_hInst;
psp[1].pszTemplate   = MAKEINTRESOURCE(IDD_COMBOBOXES);
psp[1].pszIcon       = NULL;
psp[1].pfnDlgProc    = ComboDlgProc;
psp[1].pszTitle      = TEXT("Combo Boxes");
psp[1].lParam        = 0;

//Fill out the PROPSHEETHEADER
psh.dwSize           = sizeof(PROPSHEETHEADER);
psh.dwFlags          = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_USECALLBACK| PSH_MODELESS;
psh.hwndParent       = hwndOwner;
psh.hInstance        = g_hInst;
psh.pszIcon          = MAKEINTRESOURCE(IDI_BACKCOLOR);
psh.pszCaption       = TEXT("Modeless Property Sheet");
psh.nPages           = sizeof(psp) / sizeof(PROPSHEETPAGE);
psh.ppsp             = (LPCPROPSHEETPAGE) &psp;
psh.pfnCallback      = (PFNPROPSHEETCALLBACK)PropSheetCallback;

//disable the parent to prevent another property sheet from being created - this will be re-enabled when the property sheet is destroyed.
EnableWindow(hwndOwner, FALSE);

//And finally display the modeless property sheet. It will be destroyed in our main message loop.
return (HWND)PropertySheet(&psh);
}

/**************************************************************************

   DoWizardPropSheet()

**************************************************************************/

INT_PTR DoWizardPropSheet(HWND hwndOwner)
{
PROPSHEETPAGE psp[2];
PROPSHEETHEADER psh;

//Fill out the PROPSHEETPAGE data structure for the Background Color sheet
psp[0].dwSize        = sizeof(PROPSHEETPAGE);
psp[0].dwFlags       = PSP_USETITLE;
psp[0].hInstance     = g_hInst;
psp[0].pszTemplate   = MAKEINTRESOURCE(IDD_BUTTONS);
psp[0].pszIcon       = NULL;
psp[0].pfnDlgProc    = ButtonsDlgProc;
psp[0].pszTitle      = TEXT("Buttons");
psp[0].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Client Area Shape sheet
psp[1].dwSize        = sizeof(PROPSHEETPAGE);
psp[1].dwFlags       = PSP_USETITLE;
psp[1].hInstance     = g_hInst;
psp[1].pszTemplate   = MAKEINTRESOURCE(IDD_COMBOBOXES);
psp[1].pszIcon       = NULL;
psp[1].pfnDlgProc    = ComboDlgProc;
psp[1].pszTitle      = TEXT("Combo Boxes");
psp[1].lParam        = 0;

//Fill out the PROPSHEETHEADER
psh.dwSize           = sizeof(PROPSHEETHEADER);
psh.dwFlags          = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_USEICONID | PSH_USECALLBACK;
psh.hwndParent       = hwndOwner;
psh.hInstance        = g_hInst;
psh.pszIcon          = MAKEINTRESOURCE(IDI_BACKCOLOR);
psh.pszCaption       = TEXT("Wizard Property Sheet");
psh.nPages           = sizeof(psp) / sizeof(PROPSHEETPAGE);
psh.ppsp             = (LPCPROPSHEETPAGE) &psp;
psh.pfnCallback      = (PFNPROPSHEETCALLBACK)PropSheetCallback;

//And finally display the Wizard property sheet
return PropertySheet(&psh);
}

/**************************************************************************

   ButtonsDlgProc()

**************************************************************************/

LRESULT CALLBACK ButtonsDlgProc( HWND hdlg,
                                 UINT uMessage,
                                 WPARAM wParam,
                                 LPARAM lParam)
{
LPNMHDR     lpnmhdr;

switch (uMessage)
   {
   // on any command notification, tell the property sheet to enable the Apply button
   case WM_COMMAND:
      PropSheet_Changed(GetParent(hdlg), hdlg);
      break;

   case WM_NOTIFY:
      lpnmhdr = (NMHDR FAR *)lParam;

      switch (lpnmhdr->code)
         {
         case PSN_APPLY:   //sent when OK or Apply button pressed
            break;

         case PSN_RESET:   //sent when Cancel button pressed
            break;
         
         case PSN_SETACTIVE:
            //this will be ignored if the property sheet is not a wizard
            PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_NEXT);
            break;

         default:
            break;
         }
      break;

   default:
      break;
   }

return FALSE;
}

/**************************************************************************

   ComboDlgProc()

**************************************************************************/

LRESULT CALLBACK ComboDlgProc(   HWND hdlg,
                                 UINT uMessage,
                                 WPARAM wParam,
                                 LPARAM lParam)
{
LPNMHDR     lpnmhdr;

switch (uMessage)
   {
   // on any command notification, tell the property sheet to enable the Apply button
   case WM_COMMAND:
      PropSheet_Changed(GetParent(hdlg), hdlg);
      break;

   case WM_NOTIFY:
      lpnmhdr = (NMHDR FAR *)lParam;

      switch (lpnmhdr->code)
         {
         case PSN_APPLY:   //sent when OK or Apply button pressed
            break;

         case PSN_RESET:   //sent when Cancel button pressed
            break;
         
         case PSN_SETACTIVE:
            //this will be ignored if the property sheet is not a wizard
            PropSheet_SetWizButtons(GetParent(hdlg), PSWIZB_BACK | PSWIZB_FINISH);
            return FALSE;

         default:
            break;
         }
      break;

   default:
   break;
   }

return FALSE;
}

/**************************************************************************

   PropSheetCallback()

**************************************************************************/

void CALLBACK PropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam)
{
switch(uMsg)
   {
   //called before the dialog is created, hwndPropSheet = NULL, lParam points to dialog resource
   case PSCB_PRECREATE:
      {
      LPDLGTEMPLATE  lpTemplate = (LPDLGTEMPLATE)lParam;

      if(!(lpTemplate->style & WS_SYSMENU))
         {
         lpTemplate->style |= WS_SYSMENU;
         }
      }
      break;

   //called after the dialog is created
   case PSCB_INITIALIZED:
      break;

   }
}

//THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Module:  aboutdlg.c
//
// Overview:   contains the windows functions used by the about box
//
// PLATFORMS:  Windows NT (only)
//
//  FUNCTIONS:  About() About box DlgProc
//
//
// Windows Header Files:
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>

// Local Header Files
#include "statlist.h"
#include "winutils.h"
#include "aboutdlg.h"
//
//  FUNCTION: About(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for "About" dialog box
//       This version allows greater flexibility over the contents of the 'About' box,
//       by pulling out values from the 'Version' resource.
//
//  MESSAGES:
//
// WM_INITDIALOG - initialize dialog box
// WM_COMMAND    - Input received
//
//
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static  HFONT hfontDlg;    // Font for dialog text
   static   HFONT hFinePrint; // Font for 'fine print' in dialog
   DWORD   dwVerInfoSize;     // Size of version information block
   LPSTR   lpVersion;         // String pointer to 'version' text
   DWORD   dwVerHnd=0;        // An 'ignored' parameter, always '0'
   UINT    uVersionLen;
   BOOL    bRetCode;
   int     i;
   char    szFullPath[256];
   char    szResult[256];
   char    szGetName[256];
   DWORD dwVersion;
   char  szVersion[40];
   DWORD dwResult;

   switch (message)
   {
   case WM_INITDIALOG:
      // hide window while it's being constructed
      ShowWindow (hDlg, SW_HIDE);

      // crate and load the fonts to be used for the text
      hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            VARIABLE_PITCH | FF_SWISS, "");
      hFinePrint = CreateFont(11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              VARIABLE_PITCH | FF_SWISS, "");

      // position the window in the center of the parent
      CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));

      // get the .exe path
      GetModuleFileName ((HINSTANCE)(GetWindowLongPtr(hDlg, GWLP_HINSTANCE)),
                         szFullPath, sizeof(szFullPath));

      // Now lets dive in and pull out the version information:
      dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
      if (dwVerInfoSize)
      {
         LPSTR   lpstrVffInfo;
         HANDLE  hMem;
         hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
         lpstrVffInfo  = GlobalLock(hMem);
         GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
         // The below 'hex' value looks a little confusing, but
         // essentially what it is, is the hexidecimal representation
         // of a couple different values that represent the language
         // and character set that we are wanting string values for.
         // 040904E4 is a very common one, because it means:
         //   US English, Windows MultiLingual characterset
         // Or to pull it all apart:
         // 04------        = SUBLANG_ENGLISH_USA
         // --09----        = LANG_ENGLISH
         // --11----        = LANG_JAPANESE
         // ----04E4 = 1252 = Codepage for Windows:Multilingual

         _snprintf_s(szGetName, 256, _TRUNCATE, "%sProductName", GetStringRes(IDS_VER_INFO_LANG));

         // Set the title of the dialog:
         bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
                                  (LPSTR)szGetName,
                                  (LPVOID)&lpVersion,
                                  (UINT *)&uVersionLen);

         // Notice order of version and string...
         _snprintf_s(szResult, 256, _TRUNCATE, "About %s", lpVersion);

         // set dialog caption
         SetWindowText (hDlg, szResult);

         // Walk through the dialog items that we want to replace:
         for (i = DLG_VERFIRST; i <= DLG_VERLAST; i++)
         {
            GetDlgItemText(hDlg, i, szResult, sizeof(szResult));
            _snprintf_s(szGetName, 256, _TRUNCATE, "%s%s", GetStringRes(IDS_VER_INFO_LANG), szResult);
            uVersionLen   = 0;
            lpVersion     = NULL;
            bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
                                           (LPSTR)szGetName,
                                           (LPVOID)&lpVersion,
                                           (UINT *)&uVersionLen);

            if ( bRetCode && uVersionLen && lpVersion)
            {
               // Replace dialog item text with version info
               strncpy_s(szResult, 256, lpVersion, _TRUNCATE);
               SetDlgItemText(hDlg, i, szResult);
            }
            else
            {
               dwResult = GetLastError();

               _snprintf_s(szResult, 256, _TRUNCATE, GetStringRes(IDS_VERSION_ERROR), dwResult);
               SetDlgItemText (hDlg, i, szResult);
            }
            SendMessage (GetDlgItem (hDlg, i), WM_SETFONT,
                         (UINT_PTR)((i==DLG_VERLAST)?hFinePrint:hfontDlg),
                         TRUE);
         } // for (i = DLG_VERFIRST; i <= DLG_VERLAST; i++)

         GlobalUnlock(hMem);
         GlobalFree(hMem);

      }
      else
      {
         // No version information available.

      } // if (dwVerInfoSize)

      SendMessage (GetDlgItem (hDlg, IDC_LABEL), WM_SETFONT,
                   (WPARAM)hfontDlg,(LPARAM)TRUE);

      // We are  using GetVersion rather then GetVersionEx
      // because earlier versions of Windows NT and Win32s
      // didn't include GetVersionEx:
      dwVersion = GetVersion();

      if (dwVersion < 0x80000000)
      {
         // Windows NT
         _snprintf_s(szVersion, 40, _TRUNCATE, "Microsoft Windows NT %u.%u (Build: %u)",
                   (DWORD)(LOBYTE(LOWORD(dwVersion))),
                   (DWORD)(HIBYTE(LOWORD(dwVersion))),
                   (DWORD)(HIWORD(dwVersion)) );
      }
      else if (LOBYTE(LOWORD(dwVersion))<4)
      {
         // Win32s
         _snprintf_s(szVersion, 40, _TRUNCATE, "Microsoft Win32s %u.%u (Build: %u)",
                   (DWORD)(LOBYTE(LOWORD(dwVersion))),
                   (DWORD)(HIBYTE(LOWORD(dwVersion))),
                   (DWORD)(HIWORD(dwVersion) & ~0x8000) );
      }
      else
      {
         // Windows 95
         _snprintf_s(szVersion, 40, _TRUNCATE, "Microsoft Windows 95 %u.%u",
                   (DWORD)(LOBYTE(LOWORD(dwVersion))),
                   (DWORD)(HIBYTE(LOWORD(dwVersion))) );
      }

      SetWindowText (GetDlgItem(hDlg, IDC_OSVERSION), szVersion);

      // now display the finished product
      ShowWindow (hDlg, SW_SHOW);
      return (TRUE);

   case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
      {
         EndDialog(hDlg, TRUE);
         DeleteObject (hfontDlg);
         DeleteObject (hFinePrint);
         return (TRUE);
      }
      break;
   }

   return FALSE;
}

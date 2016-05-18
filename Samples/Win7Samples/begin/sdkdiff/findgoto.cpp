// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * findgoto.cpp
 */

#include "precomp.h"
#include "table.h"

#include "state.h"
#include "sdkdiff.h"
#include "wdiffrc.h"

#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "section.h"
#include "compitem.h"
#include "complist.h"

#include "view.h"
#include "findgoto.h"

extern const CHAR szSdkDiff[];
extern VIEW current_view;

static const char szFindSearchDown[]  = "FindSearchDown";
static const char szFindMatchCase[]   = "FindMatchCase";
static const char szFindWholeWord[]   = "FindWholeWord";
static const char szFindStringXX[]    = "FindString%02d";

/*
 * DlgProc for the Find dialog
 *
 */
int FAR PASCAL
FindDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  HRESULT hr;
  switch (message)
    {
    case WM_INITDIALOG:
      {
      char rgchFindString[CCH_FINDSTRING];
      char rgchKey[32];
      int iString = 0;
      const BOOL fDown      = GetProfileInt(APPNAME, szFindSearchDown, 1);
      const BOOL fMatchCase = GetProfileInt(APPNAME, szFindMatchCase, 0);
      const BOOL fWholeWord = GetProfileInt(APPNAME, szFindWholeWord, 0);

      CheckDlgButton(hDlg, ((!fDown) ? IDC_OPT_UP : IDC_OPT_DOWN), BST_CHECKED);
      CheckDlgButton(hDlg, IDC_CHK_MATCHCASE, ((fMatchCase) ? BST_CHECKED : BST_UNCHECKED));
      CheckDlgButton(hDlg, IDC_CHK_WHOLEWORD, ((fWholeWord) ? BST_CHECKED : BST_UNCHECKED));
      SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_RESETCONTENT, 0, 0L);
      SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, EM_LIMITTEXT, CCH_FINDSTRING, 0L);

      for (iString = 0; iString < NUM_FINDSTRINGS; iString++)
        {
        hr = StringCchPrintf(rgchKey, 32, szFindStringXX, iString);
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_PRINTF);
        if ( !GetProfileString(APPNAME, rgchKey, "", rgchFindString, CCH_FINDSTRING)
          || !*rgchFindString)
          {
          break;
          }

        SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_INSERTSTRING, (WPARAM)-1, (LPARAM) rgchFindString);
        }

      return TRUE;
      }

    case WM_COMMAND:
      switch (wParam)
        {
        case IDOK:
          {
          char rgchText[CCH_FINDSTRING];
          const BOOL fWholeWord = (IsDlgButtonChecked(hDlg, IDC_CHK_WHOLEWORD) == BST_CHECKED);

          WriteProfileInt(APPNAME, szFindSearchDown, (IsDlgButtonChecked(hDlg, IDC_OPT_DOWN) == BST_CHECKED));
          WriteProfileInt(APPNAME, szFindMatchCase,  (IsDlgButtonChecked(hDlg, IDC_CHK_MATCHCASE) == BST_CHECKED));
          WriteProfileInt(APPNAME, szFindWholeWord,  fWholeWord);

          SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, WM_GETTEXT, CCH_FINDSTRING, (LPARAM) rgchText);
          if (*rgchText)
            {
            const LONG iCol = (view_isexpanded(current_view)) ? 2 : 1;
            char rgchBuf[CCH_FINDSTRING];
            char rgchKey[32];
            int iRet = (int) SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM) rgchText);
            int iString = 0;

            if (iRet != CB_ERR)
              {
              iString = iRet;

              do
                {
                SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_GETLBTEXT, iString, (LPARAM) rgchBuf);
                if (!My_mbsncmp((LPCSTR) rgchText, (LPCSTR) rgchBuf, CCH_FINDSTRING))
                  {
                  /* delete the string out of its old place */
                  SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_DELETESTRING, iString, 0L);
                  break;
                  }

                iString = (int) SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_FINDSTRINGEXACT, iString, (LPARAM) rgchText);
                } while (iString != CB_ERR && iString != iRet);
              }

            /* insert the new string at index zero */
            SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_INSERTSTRING, 0, (LPARAM) rgchText);

            for (iString = 0; iString < NUM_FINDSTRINGS; iString++)
              {
              *rgchBuf = 0;
              iRet = (int) SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_GETLBTEXT, iString, (LPARAM) rgchBuf);
              if (iRet <= 0 || iRet == CB_ERR || !*rgchBuf)
                break;

              hr = StringCchPrintf(rgchKey, 32, szFindStringXX, iString);
			  if (FAILED(hr))
				  OutputError(hr, IDS_SAFE_PRINTF);
              WriteProfileString(APPNAME, rgchKey, rgchBuf);
              }

            /* don't end the dlg if we didn't find a match */
            if (!FindString(hDlg, iCol, rgchText, 0, ((fWholeWord) ? 1 : -1)))
              return TRUE;
            }

          EndDialog(hDlg, wParam);
          return TRUE;
          }

        case IDCANCEL:
          EndDialog(hDlg, wParam);
          return TRUE;
        }
    }

  return FALSE;
}


/*
 * DlgProc for the Go To Line dialog
 *
 */
int FAR PASCAL
GoToLineDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
    {
    case WM_INITDIALOG:
      SendDlgItemMessage(hDlg, IDC_EDT_GOTOLINE, EM_LIMITTEXT, CCH_MAXDIGITS, 0L);
      SendDlgItemMessage(hDlg, IDC_EDT_GOTOLINE, WM_SETTEXT, 0, (LPARAM) "1");
      return TRUE;

    case WM_COMMAND:
      switch (wParam)
        {
        case IDOK:
          {
          const LONG lMax = view_getrowcount(current_view);
          char *pchT = NULL;
          int cNumeric = 0;
          char rgchBuf[256];

          SendDlgItemMessage(hDlg, IDC_EDT_GOTOLINE, WM_GETTEXT, CCH_MAXDIGITS + 1, (LPARAM) rgchBuf);

          /* eat leading whitespace */
          for (pchT = rgchBuf; *pchT && isspace((UCHAR)*pchT); pchT = CharNext(pchT))
          {
              ;
          }

          for ( ; *pchT; pchT = CharNext(pchT))
            {
            if (IsDBCSLeadByte(*pchT) || !isdigit((UCHAR)*pchT))
              break;
            
            cNumeric++;
            }

          /* if we didn't reach the end of the string, we have an invalid numeric string */
          if (!cNumeric)
            {
            MessageBox(hDlg, LoadRcString(IDS_GOTOLINE_INVALIDSTRING), szSdkDiff, MB_OK|MB_ICONSTOP|MB_TASKMODAL);
            return TRUE;
            }

          /* terminate the string after the numeric chars */
          *pchT = 0;

          /* go find the string */
          if (!FindString(hDlg, 0, rgchBuf, 1, 1))
            return TRUE;

          EndDialog(hDlg, wParam);
          return TRUE;
          }

        case IDCANCEL:
          EndDialog(hDlg, wParam);
          return TRUE;
        }
    }

  return FALSE;
}


/*
 * Cover function for string search
 *
 */
BOOL
FindString(HWND hwndParent, LONG iCol, const char *pszFind, int nSearchDirection, int nWholeWord)
{
  char rgchText[CCH_FINDSTRING];
  char rgchKey[32];
  BOOL fSearchDown = TRUE;    /* default is to search forward (down) */
  const BOOL fMatchCase = GetProfileInt(APPNAME, szFindMatchCase, 0);
  BOOL fWholeWord = FALSE;
  HRESULT hr;

  if (!nWholeWord)
    {
    fWholeWord = (BOOL) GetProfileInt(APPNAME, szFindWholeWord, 0);
    }
  else
    {
    fWholeWord = (nWholeWord == 1);
    }

  if (nSearchDirection < 0)   /* search backward (up) */
    {
    fSearchDown = FALSE;
    }
  else if (!nSearchDirection) /* look it up in the registry */
    {
    fSearchDown = GetProfileInt(APPNAME, szFindSearchDown, 1);
    }

  *rgchText = 0;
  if (pszFind)
    {
    /* use the arg string */
    My_mbsncpy((LPSTR) rgchText, (LPCSTR) pszFind, CCH_FINDSTRING);
    }
  else
    {
    /* look up last find string in registry */
    hr = StringCchPrintf(rgchKey, 32, szFindStringXX, 0);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_PRINTF);
    if (!GetProfileString(APPNAME, rgchKey, "", rgchText, CCH_FINDSTRING))
      *rgchText = 0;
    }

  if ( !*rgchText
    || !view_findstring(current_view, iCol, rgchText, fSearchDown, fMatchCase, fWholeWord))
    {
    char rgchMsg[CCH_FINDSTRING * 2];
    hr = StringCchPrintf(rgchMsg, (CCH_FINDSTRING*2), LoadRcString(IDS_FIND_NOTFOUND), rgchText);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_PRINTF);
	else
		MessageBox(hwndParent, rgchMsg, szSdkDiff, MB_OK|MB_ICONSTOP|MB_TASKMODAL);
    return FALSE;
    }

  return TRUE;
}

//THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Mainwnd: mainwnd.c
//
// PURPOSE: main window functions
//
// PLATFORMS:  Windows NT (only)
//
// FUNCTIONS:
//  DeleteAllCounters()     clears counters from query
//  On_WM_CREATE()          processes WM_CREATE message
//  On_IDM_GET_DATA()       gets the current perf data and updates window
//  On_IDM_ADD_COUNTERS()   displays browser & adds perf. counter to display
//  On_IDM_CLEAR_ALL()      clear all counters & refresh display
//  On_WM_COMMAND()         processes WM_COMMAND messages
//  On_WM_RBUTTONDOWN()     processes Right Mouse button clicks
//  On_WM_PAINT()           draws the current data in the main window
//  On_WM_DESTROY()         closes any open interfaces & memory allocations
//  WndProc()               main window procedure for this app's window
//
// SPECIAL INSTRUCTIONS: N/A
//
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <winperf.h>
#include <stdio.h>
#include <pdh.h>
#include <pdhmsg.h>

#include "statlist.h"
#include "winutils.h"
#include "mainwnd.h"
#include "aboutdlg.h"

// font for text in window
static   HFONT hFinePrint = NULL;

// PDH Query Handle for these counters
static HQUERY   hQuery = NULL;

// pointer to first item in counter list
static PCIB    pFirstCib = NULL;

int nTabStops[] = {300, 400, 500, 600, 700};
int nNumTabStops = sizeof(nTabStops) / sizeof(int);

#define NUM_STAT_SAMPLES    100

static
void
DeleteAllCounters ()
{
   PCIB   pCib, pCibOld;

   // close PDH Query
   // this removes all counters from the query as well as
   // removes the query item itself
   if (hQuery != NULL)
   {
      PdhCloseQuery (hQuery);
      hQuery = NULL;
   }

   // clean up any memory allocations
   pCib = pFirstCib;
   while ( pCib != NULL)
   {
      pCibOld = pCib;
      if (pCibOld->pCounterArray != NULL)
      {
         HeapFree (GetProcessHeap(), 0, pCibOld->pCounterArray);
      }
      pCib = pCib->pNext;
      HeapFree (GetProcessHeap(), 0, pCibOld);
   }
   pFirstCib = NULL;
}

// windows message functions
static
LRESULT
On_WM_CREATE (
             HWND hWnd,
             WPARAM wParam,
             LPARAM lParam)
{
   PDH_STATUS  pdhStatus;

   if (hQuery == NULL)
   {
      pdhStatus = PdhOpenQuery (NULL, 0, &hQuery);
   }
   if (hFinePrint == NULL)
   {
      hFinePrint = CreateFont(11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              VARIABLE_PITCH | FF_SWISS, "");
   }
   return ERROR_SUCCESS;
}

static
LRESULT
On_IDM_GET_DATA (
                HWND hWnd,
                WPARAM wParam,
                LPARAM lParam)
{
   PDH_STATUS  pdhStatus;
   DWORD       dwType;
   PCIB        pCib;
   PDH_FMT_COUNTERVALUE    pValue;
   PDH_RAW_COUNTER         pRaw;

   if (hQuery != NULL)
   {
      // get the current values of the query data
      pdhStatus = PdhCollectQueryData (hQuery);
      if (pdhStatus == ERROR_SUCCESS)
      {
         // loop through all counters and update the display values
         // and statistics
         for (pCib = pFirstCib; pCib != NULL; pCib = pCib->pNext)
         {
            // update "Last value"
            pdhStatus = PdhGetFormattedCounterValue (
                                                    pCib->hCounter, PDH_FMT_DOUBLE, &dwType, &pValue);
            pCib->dLastValue = pValue.doubleValue;
            // update "Raw Value" and statistics
            pdhStatus = PdhGetRawCounterValue (
                                              pCib->hCounter, &dwType, &pRaw);
            pCib->pCounterArray[pCib->dwNextIndex] = pRaw;
            pdhStatus = PdhComputeCounterStatistics (
                                                    pCib->hCounter,
                                                    PDH_FMT_DOUBLE,
                                                    pCib->dwFirstIndex,
                                                    ++pCib->dwLastIndex,
                                                    pCib->pCounterArray,
                                                    &pCib->pdhCurrentStats);
            // update pointers & indeces
            if (pCib->dwLastIndex < NUM_STAT_SAMPLES)
            {
               pCib->dwNextIndex = ++pCib->dwNextIndex % NUM_STAT_SAMPLES;
            }
            else
            {
               --pCib->dwLastIndex;
               pCib->dwNextIndex = pCib->dwFirstIndex;
               pCib->dwFirstIndex = ++pCib->dwFirstIndex % NUM_STAT_SAMPLES;
            }
         }
         // cause the window to be repainted with the new values
         // (NOTE: This isn't the most efficient method of
         // display updating.)
         InvalidateRect (hWnd, NULL, TRUE);
      }
   }
   return ERROR_SUCCESS;
}

static
LRESULT
On_IDM_ADD_COUNTERS (
                    HWND hWnd,
                    WPARAM wParam,
                    LPARAM lParam)
{
   PDH_BROWSE_DLG_CONFIG  BrowseInfo;
   CHAR            szCounterBuffer[COUNTER_STRING_SIZE];
   PDH_STATUS      pdhStatus;
   PCIB            pNewCib;

   BrowseInfo.bIncludeInstanceIndex = FALSE;
   BrowseInfo.bSingleCounterPerAdd = TRUE;
   BrowseInfo.bSingleCounterPerDialog = TRUE;
   BrowseInfo.bLocalCountersOnly = FALSE;
   BrowseInfo.bWildCardInstances = FALSE;
   BrowseInfo.bHideDetailBox = FALSE;
   BrowseInfo.bDisableMachineSelection = FALSE;
   BrowseInfo.bInitializePath = FALSE;
   BrowseInfo.bIncludeCostlyObjects = TRUE;
   BrowseInfo.bReserved = 0;
   BrowseInfo.hWndOwner = hWnd;
   BrowseInfo.szReturnPathBuffer = szCounterBuffer;
   BrowseInfo.cchReturnPathLength = COUNTER_STRING_SIZE;
   BrowseInfo.pCallBack = NULL;
   BrowseInfo.dwCallBackArg = 0;
   BrowseInfo.CallBackStatus = ERROR_SUCCESS;
   BrowseInfo.dwDefaultDetailLevel = PERF_DETAIL_WIZARD;
   BrowseInfo.szDialogBoxCaption = "Select a counter to monitor";
   BrowseInfo.szDataSource = NULL;


   pdhStatus = PdhBrowseCounters (&BrowseInfo);

   if (pdhStatus == ERROR_SUCCESS)
   {
      // add counter to the list
      pNewCib = (PCIB) HeapAlloc(GetProcessHeap(),
                                 HEAP_ZERO_MEMORY, sizeof(CIB));
      if (pNewCib != NULL)
      {
         // try to add the counter to the query
         pdhStatus = PdhAddCounter (hQuery,
                                    szCounterBuffer, 0, &pNewCib->hCounter);
         if (pdhStatus == ERROR_SUCCESS)
         {
            strncpy_s(pNewCib->szCounterPath, COUNTER_STRING_SIZE, szCounterBuffer, _TRUNCATE);
            // allocate the raw data buffer here
            pNewCib->pCounterArray = (PPDH_RAW_COUNTER)HeapAlloc (
                                                                 GetProcessHeap(), HEAP_ZERO_MEMORY,
                                                                 (sizeof(PDH_RAW_COUNTER) * NUM_STAT_SAMPLES));
            pNewCib->dwFirstIndex = 0;
            pNewCib->dwNextIndex = 0;
            pNewCib->dwLastIndex = 0;
            pNewCib->dLastValue = 0.0f;
            pNewCib->pdhCurrentStats.dwFormat = 0;
            pNewCib->pdhCurrentStats.count = 0;
            pNewCib->pdhCurrentStats.min.CStatus = PDH_CSTATUS_INVALID_DATA;
            pNewCib->pdhCurrentStats.min.largeValue = 0;
            pNewCib->pdhCurrentStats.max.CStatus = PDH_CSTATUS_INVALID_DATA;
            pNewCib->pdhCurrentStats.max.largeValue = 0;
            pNewCib->pdhCurrentStats.mean.CStatus = PDH_CSTATUS_INVALID_DATA;
            pNewCib->pdhCurrentStats.mean.largeValue = 0;

            //add to the top of the list
            pNewCib->pNext = pFirstCib;
            pFirstCib = pNewCib;

            // repaint window to get new entry
            InvalidateRect (hWnd, NULL, TRUE);
         } // else unable to add counter
      } // else unable to allocate pointer
   } // else user cancelled
   return ERROR_SUCCESS;
}

static
LRESULT
On_IDM_CLEAR_ALL (
                 HWND hWnd,
                 WPARAM wParam,
                 LPARAM lParam)
{
   PDH_STATUS  pdhStatus;

   // delete all current counters, then create a new query
   DeleteAllCounters();

   pdhStatus = PdhOpenQuery (NULL, 0, &hQuery);

   InvalidateRect (hWnd, NULL, TRUE);

   return ERROR_SUCCESS;
}

static
LRESULT
On_WM_COMMAND (
              HWND hWnd,
              WPARAM wParam,
              LPARAM lParam)
{
   int wmId, wmEvent;
   char szHelpFileName[MAX_PATH];
   BOOL bGotHelp;

   wmId    = LOWORD(wParam); // Remember, these are...
   wmEvent = HIWORD(wParam); // ...different for Win32!

   //Parse the menu selections:
   switch (wmId)
   {
   // File Menu Item
   case IDM_NEW:
   case IDM_OPEN:
   case IDM_SAVE:
   case IDM_SAVEAS:
      return (DefWindowProc(hWnd, WM_COMMAND, wParam, lParam));

   case IDM_EXIT:
      DestroyWindow (hWnd);
      return ERROR_SUCCESS;

      // configure menu item
   case IDM_ADD_COUNTERS:
      return On_IDM_ADD_COUNTERS (hWnd, wParam, lParam);

   case IDM_CLEAR_ALL:
      return On_IDM_CLEAR_ALL (hWnd, wParam, lParam);

      // get data menu item
   case IDM_GET_DATA:
      return On_IDM_GET_DATA (hWnd, wParam, lParam);


      // Help Menu Item
   case IDM_HELPTOPICS:
      _snprintf_s(szHelpFileName, MAX_PATH, _TRUNCATE, "%s.HLP", APPNAME);
      bGotHelp = WinHelp (hWnd, szHelpFileName, HELP_FINDER,(DWORD)0);
      if (!bGotHelp)
      {
         MessageBox (GetFocus(), GetStringRes(IDS_NO_HELP),
                     APPNAME, MB_OK|MB_ICONHAND);
      }
      return ERROR_SUCCESS;

   case IDM_ABOUT:
      DialogBox((HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)),
                "AboutBox", hWnd, (DLGPROC)About);
      return ERROR_SUCCESS;

   default:
      return (DefWindowProc(hWnd, WM_COMMAND, wParam, lParam));
   }
}

static LRESULT
On_WM_RBUTTONDOWN (
                  HWND hWnd,
                  WPARAM wParam,
                  LPARAM lParam)
{
   POINT pnt;
   HMENU hMenu;

   pnt.x = LOWORD(lParam);
   pnt.y = HIWORD(lParam);
   ClientToScreen(hWnd, (LPPOINT) &pnt);
   // This is where you would determine the appropriate 'context'
   // menu to bring up. Since this app has no real functionality,
   // we will just bring up the 'configure' menu:
   hMenu = GetSubMenu (GetMenu (hWnd), 1);
   if (hMenu)
   {
      TrackPopupMenu (hMenu, 0, pnt.x, pnt.y, 0, hWnd, NULL);
   }
   else
   {
      // Couldn't find the menu...
      MessageBeep(0);
   }

   return ERROR_SUCCESS;
}

static LRESULT
On_WM_PAINT (
            HWND hWnd,
            WPARAM wParam,
            LPARAM lParam)
{
   PAINTSTRUCT ps;
   HDC hdc;
   PCIB    pCib;
   int     nX, nY;
   LONG    lTextOutReturn;
   char    szOutputString[MAX_PATH + COUNTER_STRING_SIZE];
   int     nStringLength;

   hdc = BeginPaint (hWnd, &ps);

   nX = 0;
   nY = 0;
   // draw Title text
   strncpy_s(szOutputString, MAX_PATH, "Performance Counter\tLast Value\tMinimum\tMaximum\tAverage", _TRUNCATE);
   nStringLength = lstrlen(szOutputString) * sizeof(TCHAR);
   lTextOutReturn = TabbedTextOut (hdc, nX, nY,
                                   szOutputString, nStringLength,
                                   nNumTabStops, nTabStops, 0);
   nY += HIWORD(lTextOutReturn);

   // select the fine print font for this window
   SelectObject(hdc, hFinePrint);

   // for each CIB in the list draw the current text and value
   for (pCib = pFirstCib; pCib != NULL; pCib = pCib->pNext)
   {
      nStringLength = _snprintf_s(szOutputString, COUNTER_STRING_SIZE + MAX_PATH, _TRUNCATE, "%s\t%f\t%f\t%f\t%f",
                               pCib->szCounterPath, pCib->dLastValue,
                               pCib->pdhCurrentStats.min.doubleValue,
                               pCib->pdhCurrentStats.max.doubleValue,
                               pCib->pdhCurrentStats.mean.doubleValue);
      lTextOutReturn = TabbedTextOut (hdc, nX, nY,
                                      szOutputString, nStringLength,
                                      nNumTabStops, nTabStops, 0);
      nY += HIWORD(lTextOutReturn);
   }

   EndPaint (hWnd, &ps);
   return ERROR_SUCCESS;
}

static LRESULT
On_WM_DESTROY (
              HWND hWnd,
              WPARAM wParam,
              LPARAM lParam)
{
   char szHelpFileName[MAX_PATH];

   _snprintf_s(szHelpFileName, MAX_PATH, _TRUNCATE, "%s.HLP", APPNAME);
   // Tell WinHelp we don't need it any more...
   WinHelp (hWnd, szHelpFileName, HELP_QUIT,(DWORD)0);

   DeleteAllCounters();

   PostQuitMessage(0);
   return ERROR_SUCCESS;
}

//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  MESSAGES:
//
//    WM_COMMAND - process the application menu
//    WM_PAINT - Paint the main window
//    WM_DESTROY - post a quit message and return
//    WM_RBUTTONDOWN - Right mouse click -- put up context menu here if appropriate
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_CREATE:
      return On_WM_CREATE (hWnd, wParam, lParam);

   case WM_COMMAND:
      return On_WM_COMMAND (hWnd, wParam, lParam);

   case WM_RBUTTONDOWN: // RightClick in windows client area...
      return On_WM_RBUTTONDOWN (hWnd, wParam, lParam);

   case WM_PAINT:
      return On_WM_PAINT (hWnd, wParam, lParam);

   case WM_DESTROY:
      return On_WM_DESTROY (hWnd, wParam, lParam);

   default:
      return (DefWindowProc(hWnd, message, wParam, lParam));
   }
   return (0);
}

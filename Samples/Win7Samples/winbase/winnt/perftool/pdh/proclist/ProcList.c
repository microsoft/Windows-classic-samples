/*
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2000.  Microsoft Corporation.  All rights reserved.

    proclist.c

    sample program to list current processes using the PDH.DLL
    browsing functions

*/
#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE            1
#endif
#define tmain   wmain
#else
#define tmain   main
#endif

// this program only needs the "bare essential" windows header files
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <winperf.h>
#include <malloc.h>
#include <stdio.h>
#include <tchar.h>
#include <pdh.h>
#include <pdhmsg.h>

int
tmain ()
{
   PDH_STATUS  pdhStatus               = ERROR_SUCCESS;
   LPTSTR      szCounterListBuffer     = NULL;
   DWORD       dwCounterListSize       = 0;
   LPTSTR      szInstanceListBuffer    = NULL;
   DWORD       dwInstanceListSize      = 0;
   LPTSTR      szThisInstance          = NULL;


   // call the function to determine the required buffer size for the data
   pdhStatus = PdhEnumObjectItems(
                                  NULL,                   // reserved
                                  NULL,                   // local machine
                                  TEXT("Process"),        // object to enumerate
                                  szCounterListBuffer,    // pass in NULL buffers
                                  &dwCounterListSize,     // an 0 length to get
                                  szInstanceListBuffer,   // required size
                                  &dwInstanceListSize,    // of the buffers in chars
                                  PERF_DETAIL_WIZARD,     // counter detail level
                                  0);
   if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA)
   {
      // allocate the buffers and try the call again
      // PdhEnum functions will return ERROR_SUCCESS in WIN2K, but PDH_MORE_DATA in XP and later.
      // In either case, dwCounterListSize and dwInstanceListSize should contain correct buffer size needed.
      //
      szCounterListBuffer = (LPTSTR)malloc (
                                           (dwCounterListSize * sizeof (TCHAR)));
      szInstanceListBuffer = (LPTSTR)malloc (
                                            (dwInstanceListSize * sizeof (TCHAR)));
      if ((szCounterListBuffer != NULL) &&
          (szInstanceListBuffer != NULL))
      {
         pdhStatus = PdhEnumObjectItems (
                                        NULL,   // reserved
                                        NULL,   // local machine
                                        TEXT("Process"), // object to enumerate
                                        szCounterListBuffer,    // pass in NULL buffers
                                        &dwCounterListSize,     // an 0 length to get
                                        szInstanceListBuffer,   // required size
                                        &dwInstanceListSize,    // of the buffers in chars
                                        PERF_DETAIL_WIZARD,     // counter detail level
                                        0);
         if (pdhStatus == ERROR_SUCCESS)
         {
            _tprintf (TEXT("\nRunning Processes:"));
            // walk the return instance list
            for (szThisInstance = szInstanceListBuffer;
                *szThisInstance != 0;
                szThisInstance += lstrlen(szThisInstance) + 1)
            {
               _tprintf (TEXT("\n  %s"), szThisInstance);
            }
         }
      }
      else
      {
         _tprintf (TEXT("\nPROCLIST: unable to allocate buffers"));
      }
      if (szCounterListBuffer != NULL) free (szCounterListBuffer);
      if (szInstanceListBuffer != NULL) free (szInstanceListBuffer);
   }
   else
   {
      _tprintf (TEXT("\nPROCLIST: unable to determine the necessary buffer size required"));
   }
   return 0;
}

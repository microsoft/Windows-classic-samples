/*
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.

data type and constant definitions used by mainwnd.c
*/

#include <pdh.h>

// Use larger string buffer for counter path to accomondate longer counter/instance names.
#define COUNTER_STRING_SIZE 1024

typedef struct _CounterInfoBlock {
    struct _CounterInfoBlock *pNext;
    TCHAR   szCounterPath[COUNTER_STRING_SIZE];
    HCOUNTER    hCounter;
    PPDH_RAW_COUNTER    pCounterArray;
    DWORD       dwFirstIndex;
    DWORD       dwNextIndex;
    DWORD       dwLastIndex;
    PDH_STATISTICS  pdhCurrentStats;
    double      dLastValue;
} CIB, *PCIB;

// global functions found in mainwnd.c
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

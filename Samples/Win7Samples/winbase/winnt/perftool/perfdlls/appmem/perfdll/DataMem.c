/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1995 - 2000.  Microsoft Corporation.  All rights reserved.

Module Name:

    datamem.c

Abstract:

    a file containing the constant data structures used by the Performance
    Monitor data for the application heap performance counter object

    This file contains a set of constant data structures which are
    currently defined for the application heap Objects.

--*/
//
//  Include Files
//

#include <windows.h>
#include <winperf.h>
#include "..\pub\memctrnm.h"    // offset definitions
#include "datamem.h"            // data structure definitions

// dummy local variable
static    APPMEM_COUNTERS     ac;

//
//  Constant structure initializations
//      defined in datamem.h
//

APPMEM_DATA_DEFINITION AppMemDataDefinition = {
   {sizeof(APPMEM_DATA_DEFINITION) + sizeof(APPMEM_COUNTERS),
      sizeof(APPMEM_DATA_DEFINITION),
      sizeof(PERF_OBJECT_TYPE),
      APPMEMOBJ,
      0,
      APPMEMOBJ,
      0,
      PERF_DETAIL_NOVICE,
      (sizeof(APPMEM_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/
      sizeof(PERF_COUNTER_DEFINITION),
      0,  // application memory bytes is the default counter
      0,  // 0 instances to start with
      0,  // unicode instance names
      {0,0},
      {0,0}
   },
   {sizeof(PERF_COUNTER_DEFINITION),
      APPMEMALLOC,
      0,
      APPMEMALLOC,
      0,
      -5,
      PERF_DETAIL_NOVICE,
      PERF_COUNTER_RAWCOUNT,
      sizeof(ac.dwAppMemBytesAllocated),
      0,
   },
   {sizeof(PERF_COUNTER_DEFINITION),
      APPALLOCS,
      0,
      APPALLOCS,
      0,
      -1,
      PERF_DETAIL_NOVICE,
      PERF_COUNTER_RAWCOUNT,
      sizeof(ac.dwAppMemAllocs),
      0
   },
   {sizeof(PERF_COUNTER_DEFINITION),
      APPALLOCSSEC,
      0,
      APPALLOCSSEC,
      0,
      0,
      PERF_DETAIL_NOVICE,
      PERF_COUNTER_COUNTER,
      sizeof(ac.dwAppMemAllocsSec),
      0
   },
   {sizeof(PERF_COUNTER_DEFINITION),
      APPREALLOCS,
      0,
      APPREALLOCS,
      0,
      -1,
      PERF_DETAIL_NOVICE,
      PERF_COUNTER_RAWCOUNT,
      sizeof(ac.dwAppMemReAllocs),
      0
   },
   {sizeof(PERF_COUNTER_DEFINITION),
      APPREALLOCSSEC,
      0,
      APPREALLOCSSEC,
      0,
      0,
      PERF_DETAIL_NOVICE,
      PERF_COUNTER_COUNTER,
      sizeof(ac.dwAppMemReAllocsSec),
      0
   },
   {sizeof(PERF_COUNTER_DEFINITION),
      APPFREES,
      0,
      APPFREES,
      0,
      -1,
      PERF_DETAIL_NOVICE,
      PERF_COUNTER_RAWCOUNT,
      sizeof(ac.dwAppMemFrees),
      0
   },
   {sizeof(PERF_COUNTER_DEFINITION),
      APPFREESSEC,
      0,
      APPFREESSEC,
      0,
      0,
      PERF_DETAIL_NOVICE,
      PERF_COUNTER_COUNTER,
      sizeof(ac.dwAppMemFreesSec),
      0
   }
};

/*++ 

Copyright (c) 1995 - 2000  Microsoft Corporation

Module Name:

    datagen.c

Abstract:
       
    a file containing the constant data structures used by the Performance
    Monitor data for the Signal Generator Perf DLL

    This file contains a set of constant data structures which are
    currently defined for the Signal Generator Perf DLL.

Created:

    Bob Watson  28-Jul-1995

Revision History:

    None.

--*/
//
//  Include Files
//

#include <windows.h>
#include <winperf.h>
#include "genctrnm.h"
#include "datagen.h"

// dummy variable for field sizing.
static SIGGEN_COUNTER   sc;

//
//  Constant structure initializations 
//      defined in datagen.h
//

SIGGEN_DATA_DEFINITION SigGenDataDefinition = {

    {sizeof(SIGGEN_DATA_DEFINITION) + sizeof(SIGGEN_COUNTER),
    sizeof(SIGGEN_DATA_DEFINITION),
    sizeof(PERF_OBJECT_TYPE),
    SIGGEN_OBJ,
    0,
    SIGGEN_OBJ,
    0,
    PERF_DETAIL_NOVICE,
    (sizeof(SIGGEN_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/
        sizeof(PERF_COUNTER_DEFINITION),
    0   // assigned in Open Procedure
    PERF_NO_INSTANCES,
    0 
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
    SINE_WAVE,
    0,
    SINE_WAVE,
    0,
    0,
    PERF_DETAIL_NOVICE,
    PERF_COUNTER_RAWCOUNT,
    sizeof(sc.dwSineWaveValue),
    (DWORD_PTR)&(((PSIGGEN_COUNTER)0)->dwSineWaveValue)
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
    TRIANGLE_WAVE,
    0,
    TRIANGLE_WAVE,
    0,
    0,
    PERF_DETAIL_NOVICE,
    PERF_COUNTER_RAWCOUNT,
    sizeof(sc.dwTriangleWaveValue),
    (DWORD_PTR)&(((PSIGGEN_COUNTER)0)->dwTriangleWaveValue)
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
    SQUARE_WAVE,
    0,
    SQUARE_WAVE,
    0,
    0,
    PERF_DETAIL_NOVICE,
    PERF_COUNTER_RAWCOUNT,
    sizeof(sc.dwSquareWaveValue),
    (DWORD_PTR)&(((PSIGGEN_COUNTER)0)->dwSquareWaveValue)
    },
    {   sizeof(PERF_COUNTER_DEFINITION),
    CONSTANT_VALUE,
    0,
    CONSTANT_VALUE,
    0,
    0,
    PERF_DETAIL_NOVICE,
    PERF_COUNTER_RAWCOUNT,
    sizeof(sc.dwConstantValue),
    (DWORD_PTR)&(((PSIGGEN_COUNTER)0)->dwConstantValue)
    }
};

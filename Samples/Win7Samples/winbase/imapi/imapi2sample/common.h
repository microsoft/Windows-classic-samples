
#pragma once
#ifndef _IMAPI2TEST_COMMON_
#define _IMAPI2TEST_COMMON_

#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include <shlwapi.h>
#include <imapi2.h>
#include <imapi2error.h>
#include <imapi2fs.h>
#include <imapi2fserror.h>

#include <winioctl.h>

#include <atlbase.h>
#include <atlcom.h>
#include <strsafe.h>

#include "consoleUtil.h"
#include "DataWriter2Event.h"
#include "DiscMaster2Event.h"
#include "Erase2Event.h"
#include "AudioEvent.h"
#include "RawWriter2Event.h"
#include "imapi2sample.h"

// shlwapip.h
#ifdef OFFSETOFCLASS
    #undef OFFSETOFCLASS
#endif
//***   OFFSETOFCLASS -- (Copied from ATL, minor mods)
#define OFFSETOFCLASS(_IBase, _CDerived) \
    ((DWORD)(DWORD_PTR)(static_cast<_IBase*>((_CDerived*)0x100))-0x100)
#define MY_QI_CAST( _That, _IBase, _CDerived ) \
    ((IUnknown*)((LONG_PTR)(_That) + OFFSETOFCLASS(_IBase, _CDerived)))

#define MILLISECONDS_FROM_SECONDS(x) ((x)*1000)

__inline void FreeSysStringAndNull(BSTR &t)
{
    ::SysFreeString(t);
    t = NULL;
    return;
}
#define SafeArrayDestroyDataAndNull(x) \
{                                      \
    if ((x) != NULL)                   \
    {                                  \
        SafeArrayDestroyData(x);       \
        (x) = NULL;                    \
    }                                  \
}
#define CoTaskMemFreeAndNull(x) \
    CoTaskMemFree(x);           \
    (x) = NULL;
#define ReleaseAndNull(x)       \
{                               \
    if ((x) != NULL)            \
    {                           \
        (x)->Release();         \
        (x) = NULL;             \
    }                           \
}
#define LocalFreeAndNull(x)     \
{                               \
    if ((x) != NULL)            \
    {                           \
        LocalFree(x);           \
        (x) = NULL;             \
    }                           \
}

#ifndef IID_PPV_ARGS
#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), (static_cast<IUnknown *>(*(ppType)),reinterpret_cast<void**>(ppType))
#endif // definition of IID_PPV_ARGS required as it's not in the new tree yet.

// Defines for speed test
#define IMAPI_SECTORS_PER_SECOND_AT_1X_CD 75
#define IMAPI_SECTORS_PER_SECOND_AT_1X_DVD 680

#define FORMAT_DATA 1
#define FORMAT_TRACKATONCE 2
#define FORMAT_RAWCD 3

typedef struct _PROGRAM_OPTIONS {

    // defaults should all be logical if all are set to FALSE / NULL / 0

    // Write details
    BOOLEAN Write;
    BOOLEAN Image;
    BOOLEAN Audio;
    BOOLEAN Raw;
    BOOLEAN SessionAtOnceWrite;
    BOOLEAN CloseDisc;
    BOOLEAN Multi;
    BOOLEAN Iso;
    BOOLEAN Joliet;
    BOOLEAN UDF;

    WCHAR * FileName;
    WCHAR * BootFileName;
    WCHAR * VolumeName;

    // erase details
    BOOLEAN Erase;
    BOOLEAN FullErase;

    BOOLEAN Eject;
    BOOLEAN Close;

    ULONG WriterIndex; // store as zero-based index, print as 1-based
    // list the Writers
    BOOLEAN ListWriters;

    // Test Options
    BOOLEAN FreeSpace;

} PROGRAM_OPTIONS, *PPROGRAM_OPTIONS;

// Function prototypes

// from util.cpp
void PrintHR(HRESULT hr);

// from imapi2sample.cpp
HRESULT GetDiscRecorder(__in ULONG index, __out IDiscRecorder2 ** recorder);
void CalcElapsedTime(
   SYSTEMTIME * StartTime,
   SYSTEMTIME * FinishTime,
   SYSTEMTIME * ElapsedTime);

// from erase.cpp
HRESULT EraseMedia(ULONG index, BOOL full);


#endif // _IMAPI2TEST_COMMON_

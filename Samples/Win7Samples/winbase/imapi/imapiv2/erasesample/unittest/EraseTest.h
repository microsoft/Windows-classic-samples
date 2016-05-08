/*--

Copyright (C) Microsoft Corporation, 2006

--*/

#pragma once
#ifndef _ERASE_SAMPLE_TEST_
#define _ERASE_SAMPLE_TEST_

#include <atlcom.h>

#include "..\EraseSample\EraseSample.h"

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

#define ReleaseAndNull(x)       \
{                               \
    if ((x) != NULL)            \
    {                           \
        (x)->Release();         \
        (x) = NULL;             \
    }                           \
}

#ifndef IID_PPV_ARGS
#define IID_PPV_ARGS(ppType) __uuidof(**(ppType)), (static_cast<IUnknown *>(*(ppType)),reinterpret_cast<void**>(ppType))
#endif // definition of IID_PPV_ARGS required as it's not in the new tree yet.

// from erase.cpp
HRESULT TestErase(IDiscRecorder2* Recorder, VARIANT_BOOL FullErase);

#endif // _ERASE_SAMPLE_TEST_

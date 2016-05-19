// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) sizeof(*ArraySizer(a))
#endif

// For determining the length of an array. See ARRAYSIZE() macro. This function is never actually called.
template<class T, size_t N> char ( *ArraySizer ( T ( & ) [N] ) ) [N];

#define RDCJOIN2(a, b) a ## b
#define RDCJOIN(a, b) RDCJOIN2(a, b)
#define CAssert(fCExpr) extern int RDCJOIN(RDC_dummy_array, __LINE__) [(fCExpr) ? 1 : -1]
#define CAssert_(fCExpr) do { CAssert_(fCExpr); } while (0)

class CoInitWrapper
{
public:
    CoInitWrapper()
    {
        m_HR = CoInitializeEx ( 0, COINIT_MULTITHREADED );
    }

    ~CoInitWrapper()
    {
        if ( SUCCEEDED ( m_HR ) )
        {
            CoUninitialize();
        }
    }

    DebugHresult GetHRESULT() const
    {
        return m_HR;
    }
private:
    DebugHresult m_HR;
};


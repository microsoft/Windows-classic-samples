// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

extern LARGE_INTEGER g_LargeZero;
extern CRITICAL_SECTION g_MainLock;

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) sizeof(*ArraySizer(a))
#endif
// For determining the length of an array. See ARRAYSIZE() macro. This function is never actually called.
template<class T, size_t N> char ( *ArraySizer ( T ( & ) [N] ) ) [N];

template<class T> T Minimum ( T a, T b )
{
    return ( a < b ) ? a : b;
}
template<class T> T Maximum ( T a, T b )
{
    return ( a > b ) ? a : b;
}


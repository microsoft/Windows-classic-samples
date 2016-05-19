/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    Utils.h

Abstract:

    This h file includes sample code for packing WSAQUERYSET2 into
    a single buffer.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/

#include <winsock2.h>
#include <limits.h>

size_t GetWSAQuerySet2Size(__in  WSAQUERYSET2  *pQuerySet);

INT BuildSerializedQuerySet2(
    __in                               PWSAQUERYSET2  pQuerySet,
                                       size_t         cbSerializedQuerySet,
    __out_bcount(cbSerializedQuerySet) PBYTE          pbSerializedQuerySet);


//
// Define useful macros for rounding up to the next align boundary.
//


// If Ptr is not already aligned, then round it up until it is.
//
#ifndef ROUND_UP_POINTER
#define ROUND_UP_POINTER(Ptr,Pow2) \
        ( (LPVOID) ( (((ULONG_PTR)(Ptr))+(Pow2)-1) & (~(((LONG)(Pow2))-1)) ) )
#endif

// If Count is not already aligned, then round it up until it is.
//
#ifndef ROUND_UP_COUNT
#define ROUND_UP_COUNT(Count,Pow2) \
        ( ((Count)+(Pow2)-1) & (~(((LONG)(Pow2))-1)) )
#endif

#ifndef ALIGN_LPVOID
#define ALIGN_LPVOID            sizeof(LPVOID)
#endif


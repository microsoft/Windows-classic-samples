//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  MemoryStream.cpp
//
//          IStream Service functions.
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"

//+---------------------------------------------------------------------------
//
// CreateMemoryStream
//
//----------------------------------------------------------------------------

IStream* CreateMemoryStream()
{
    LPSTREAM lpStream = NULL;

    // Create a stream object on a memory block.
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE, 0);
    if (hGlobal != NULL)
    {
        HRESULT hr ;
        if (FAILED(hr = CreateStreamOnHGlobal(hGlobal, TRUE, &lpStream)))
        {
             GlobalFree(hGlobal);
        }
    }

    return lpStream;
}

//+---------------------------------------------------------------------------
//
// ClearStream
//
//----------------------------------------------------------------------------

void ClearStream(IStream *pStream)
{
    ULARGE_INTEGER ull;
    ull.QuadPart = 0;
    pStream->SetSize(ull);
}

//+---------------------------------------------------------------------------
//
// AddStringToStream
//
//----------------------------------------------------------------------------

void AddStringToStream(IStream *pStream, WCHAR *psz)
{
    pStream->Write(psz, lstrlenW(psz) * sizeof(WCHAR), NULL);
}


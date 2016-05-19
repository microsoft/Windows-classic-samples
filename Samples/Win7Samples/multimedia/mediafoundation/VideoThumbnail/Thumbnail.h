//////////////////////////////////////////////////////////////////////////
//
// ThumbnailGenerator: Creates thumbnail images from video files.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "sprite.h"

class ThumbnailGenerator
{
private:

    IMFSourceReader *m_pReader;
    FormatInfo      m_format;

public:

    ThumbnailGenerator();
    ~ThumbnailGenerator();


    HRESULT     OpenFile(const WCHAR* wszFileName);
    HRESULT     GetDuration(LONGLONG *phnsDuration);
    HRESULT     CanSeek(BOOL *pbCanSeek);

    HRESULT     CreateBitmaps(ID2D1RenderTarget *pRT, DWORD count, Sprite pSprites[]);

private:
    HRESULT     CreateBitmap(ID2D1RenderTarget *pRT, LONGLONG& hnsPos, Sprite *pSprite);
    HRESULT     SelectVideoStream();
    HRESULT     GetVideoFormat(FormatInfo *pFormat);
};







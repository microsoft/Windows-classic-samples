// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

#include <uianimation.h>
#include <d2d1.h>
#include <d2d1helper.h>

class CThumbnail
{
public:

    CThumbnail();
    ~CThumbnail();

    HRESULT Initialize(
        ID2D1Bitmap *pBitmap,
        IUIAnimationManager *pAnimationManager, 
        DOUBLE x,
        DOUBLE y
        );

    D2D1_SIZE_F GetSize();

    HRESULT Render(
        ID2D1HwndRenderTarget* pRT,
        ID2D1Brush *pBrushOutline
        );

    // X and Y Animation Variables

    IUIAnimationVariable *m_pAnimationVariableX;
    IUIAnimationVariable *m_pAnimationVariableY;

private:

    // Animation objects
    
    IUIAnimationManager *m_pAnimationManager;

    // D2D bitmap

    ID2D1Bitmap *m_pBitmap;
};

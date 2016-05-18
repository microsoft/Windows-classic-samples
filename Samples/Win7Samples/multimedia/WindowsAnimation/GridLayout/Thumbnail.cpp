// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "Thumbnail.h"
#include "UIAnimationSample.h"

const FLOAT OPACITY = 1.0;
const FLOAT LINE_WEIGHT = 1.0;

CThumbnail::CThumbnail() :
    m_pBitmap(NULL),
    m_pAnimationManager(NULL),
    m_pAnimationVariableX(NULL),
    m_pAnimationVariableY(NULL)
{
}

CThumbnail::~CThumbnail()
{
    SafeRelease(&m_pBitmap);
    SafeRelease(&m_pAnimationManager);
    SafeRelease(&m_pAnimationVariableX);
    SafeRelease(&m_pAnimationVariableY);
}

// Initialization

HRESULT CThumbnail::Initialize(
    ID2D1Bitmap *pBitmap,
    IUIAnimationManager *pAnimationManager,
    DOUBLE x,
    DOUBLE y
    )
{
    m_pBitmap = pBitmap;
    m_pBitmap->AddRef();

    m_pAnimationManager = pAnimationManager;
    m_pAnimationManager->AddRef();

    // Create the animation variables for the x and y coordinates

    HRESULT hr = m_pAnimationManager->CreateAnimationVariable(
        x,
        &m_pAnimationVariableX
        );
    if (SUCCEEDED(hr))
    {
        hr = m_pAnimationManager->CreateAnimationVariable(
            y,
            &m_pAnimationVariableY
            );
    }

    return hr;
}

//  Returns the size of the thumbnail bitmap

D2D1_SIZE_F CThumbnail::GetSize()
{
    return m_pBitmap->GetSize();
}

// Draws an outlined bitmap to the supplied render target

HRESULT CThumbnail::Render(
    ID2D1HwndRenderTarget* pRT,
    ID2D1Brush *pBrushOutline
    )
{
    D2D1_SIZE_F size = GetSize();
    
    DOUBLE x = 0;
    HRESULT hr = m_pAnimationVariableX->GetValue(
        &x
        );
    if (SUCCEEDED(hr))
    {
        DOUBLE y = 0;
        hr = m_pAnimationVariableY->GetValue(
            &y
            );
        if (SUCCEEDED(hr))
        {
            D2D1_RECT_F rc = D2D1::Rect<FLOAT>(
                static_cast<FLOAT>(x - 0.5 * size.width),
                static_cast<FLOAT>(y - 0.5 * size.height),
                static_cast<FLOAT>(x + 0.5 * size.width),
                static_cast<FLOAT>(y + 0.5 * size.height) 
                );
            
            pRT->DrawBitmap(
                m_pBitmap,
                rc,
                OPACITY
                );

            pRT->DrawRectangle(
                rc,
                pBrushOutline,
                LINE_WEIGHT
                );
        }
    }

    return hr;
}

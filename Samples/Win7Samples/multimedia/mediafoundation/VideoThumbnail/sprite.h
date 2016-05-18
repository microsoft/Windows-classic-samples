//////////////////////////////////////////////////////////////////////////
//
// Sprite: Manages drawing the thumbnail bitmap.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////


// NOTE: Normalized rectangles
//
// A normalized rectangle has (x,y) coordinates that range [0 ... 1],
// and is defined relative to another rectangle.
//
// For example, the upper left quandrant of a rectangle is specified
// using a normalized rectangle of { 0, 0, 0.5, 0.5 }.

#pragma once


struct FormatInfo
{
    UINT32          imageWidthPels;
    UINT32          imageHeightPels;
    BOOL            bTopDown;
    RECT            rcPicture;    // Corrected for pixel aspect ratio

    FormatInfo() : imageWidthPels(0), imageHeightPels(0), bTopDown(FALSE)
    {
        SetRectEmpty(&rcPicture);
    }
};


class Sprite
{
    enum State
    {
        CLEAR,
        PENDING,
        BITMAP
    };

    ID2D1Bitmap         *m_pBitmap;
    D2D1::Matrix3x2F    m_mat;

    BOOL            m_bTopDown;
    D2D1_SIZE_F     m_AspectRatio;

    D2D1_RECT_F     m_nrcBound;    // Bounding box, as a normalized rectangle.
    D2D1_RECT_F     m_fill;        // Actual fill rectangle in pixels.

    // Animation
    BOOL            m_bAnimating;
    float           m_timeStart;         // Start time for the animation
    float           m_timeEnd;           // Ending time.
    D2D1_RECT_F     m_nrcAnimDistance;   // Animation path. 
    D2D1_RECT_F     m_nrcStartPosition;  // Equal to m_nrcBound at m_timeStart.
    
    // Animation path is defined as follows:
    // Final bounding box = m_nrcBound + m_nrcAnimDistance

    // Wobble parameters
    float           m_fAngle;
    float           m_theta;

protected:
    
    // State:
    // Currently two states are defined, BITMAP and CLEAR, simply reflecting
    // whether the sprite contains a bitmap or not. 

    State           GetState() const { return (m_pBitmap ? BITMAP : CLEAR); }

public:
    Sprite();
    ~Sprite();

    void    SetBitmap(ID2D1Bitmap *pBitmap, const FormatInfo& format);

    void    AnimateBoundingBox(const D2D1_RECT_F& bound2, float time, float duration); 
    void    Update(ID2D1HwndRenderTarget *pRT, float time);
    void    Draw(ID2D1HwndRenderTarget *pRT);
    BOOL    HitTest(int x, int y);
    void    Clear();
};

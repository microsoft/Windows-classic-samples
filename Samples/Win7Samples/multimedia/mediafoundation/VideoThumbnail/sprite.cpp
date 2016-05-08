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

#include "videothumbnail.h"
#include "sprite.h"

#include <math.h>
#include <float.h>


D2D1_RECT_F LetterBoxRectF(D2D1_SIZE_F aspectRatio, const D2D1_RECT_F &rcDest);

const float WOBBLE_ANGLE = 10.0f;
const float WOBBLE_DECAY = 0.25f;



inline D2D1_RECT_F operator+(const D2D1_RECT_F& r1, const D2D1_RECT_F& r2)
{
    return D2D1::RectF( r1.left + r2.left, r1.top + r2.top, r1.right + r2.right, r1.bottom + r2.bottom );
}

inline D2D1_RECT_F operator-(const D2D1_RECT_F& r1, const D2D1_RECT_F& r2)
{
    return D2D1::RectF( r1.left - r2.left, r1.top - r2.top, r1.right - r2.right, r1.bottom - r2.bottom );
}

inline D2D1_RECT_F operator*(const D2D1_RECT_F& r1, float scale)
{
    return D2D1::RectF( r1.left * scale, r1.top * scale, r1.right * scale, r1.bottom * scale );
}


inline float Width(const D2D1_RECT_F& rect)
{
    return rect.right - rect.left;
}

inline float Height(const D2D1_RECT_F& rect)
{
    return rect.bottom - rect.top;
}




//-------------------------------------------------------------------
// Sprite constructor
//-------------------------------------------------------------------

Sprite::Sprite() 
 :  m_pBitmap(NULL), 
    m_bAnimating(FALSE), 
    m_timeStart(0), 
    m_timeEnd(0), 
    m_fAngle(0), 
    m_theta(0), 
    m_bTopDown(FALSE)
{
}

//-------------------------------------------------------------------
// Sprite destructor
//-------------------------------------------------------------------

Sprite::~Sprite()
{
    if (m_pBitmap)
    {
        m_pBitmap->Release();
    }
}


//-------------------------------------------------------------------
// SetBitmap: Sets the bitmap for the sprite.
//-------------------------------------------------------------------

void Sprite::SetBitmap(ID2D1Bitmap *pBitmap, const FormatInfo& format)
{
    SafeRelease(&m_pBitmap);

    if (pBitmap)
    {
        m_pBitmap = pBitmap;
        m_pBitmap->AddRef();
    }

    m_bTopDown = format.bTopDown;

    m_fill = m_nrcBound = D2D1::Rect<float>(0, 0, 0, 0);

    m_AspectRatio = D2D1::SizeF( (float)format.rcPicture.right, (float)format.rcPicture.bottom );
}


//-------------------------------------------------------------------
// Clear: Clears the bitmap.
//-------------------------------------------------------------------

void Sprite::Clear()
{
    SafeRelease(&m_pBitmap);

    m_fill = m_nrcBound = D2D1::Rect<float>(0, 0, 0, 0);

    m_AspectRatio = D2D1::SizeF(1, 1);
}


//-------------------------------------------------------------------
// AnimateBoundingBox
//
// Applies an animation path to the sprite.
//
// bound2: Final position of the bounding box, as a normalized rect.
// time: Current clock time.
// duration: Length of the animation, in seconds.
//-------------------------------------------------------------------

void Sprite::AnimateBoundingBox(const D2D1_RECT_F& bound2, float time, float duration)
{
    if (duration == 0.0f)
    {
        // Apply the new position immediately

        m_nrcBound = bound2;
        m_bAnimating = FALSE;
        m_fAngle = 0.0f;
    }
    else
    {
        // Set the animation parameters.

        m_timeStart = time;
        m_timeEnd = time + duration;

        m_nrcAnimDistance = bound2 - m_nrcBound;
        m_nrcStartPosition = m_nrcBound;

        m_fAngle = WOBBLE_ANGLE;

        m_bAnimating = TRUE;
    }
}


//-------------------------------------------------------------------
// Update: Updates the sprite, based on the clock time.
//-------------------------------------------------------------------

void Sprite::Update(ID2D1HwndRenderTarget *pRT, float time)
{
    if (GetState() == CLEAR)
    {
        return; // No bitmap; nothing to do.
    }

    if ((m_timeStart < time) && (m_timeEnd > time))
    {
        // We are in the middle of an animation. Interpolate the position.

        D2D1_RECT_F v = m_nrcAnimDistance * ( (time - m_timeStart) / (m_timeEnd - m_timeStart) );

        m_nrcBound = v + m_nrcStartPosition; 
    }
    else if (m_bAnimating && time >= m_timeEnd)
    {
        // We have reached the end of an animation. 
        // Set the final position (avoids any rounding errors)

        m_nrcBound = m_nrcStartPosition + m_nrcAnimDistance;
        m_bAnimating = FALSE;
    }

    // Compute the correct letterbox for the bitmap.
    //
    // TODO: Strictly, this only needs to be update if the bitmap changes 
    //       or the size of the render target changes. 

    D2D1_SIZE_F sizeBitmap = m_AspectRatio;

    D2D1_SIZE_F sizeRT = pRT->GetSize();

    D2D1_RECT_F rect = D2D1::RectF();

    rect.right = Width(m_nrcBound) * sizeRT.width;
    rect.bottom = Height(m_nrcBound) * sizeRT.height;

    m_fill = LetterBoxRectF(sizeBitmap, rect);
}


//-------------------------------------------------------------------
// Draw: Draws the sprite.
//-------------------------------------------------------------------

void Sprite::Draw(ID2D1HwndRenderTarget *pRT)
{
    if (GetState() == CLEAR)
    {
        return; // No bitmap; nothing to do.
    }

    D2D1_SIZE_F sizeRT = pRT->GetSize();

    const float width = Width(m_nrcBound) * sizeRT.width;
    const float height = Height(m_nrcBound) * sizeRT.height;

    // Start with an identity transform.
    D2D1::Matrix3x2F  mat = D2D1::Matrix3x2F::Identity();

    // If the image is bottom-up, flip around the x-axis. 
    if (m_bTopDown == 0)
    {
        mat = D2D1::Matrix3x2F::Scale( D2D1::SizeF(1, -1), D2D1::Point2F(0, height/2) );
    }

    // Apply wobble.
    if (m_fAngle >= FLT_EPSILON)
    {
        mat = mat * D2D1::Matrix3x2F::Rotation( m_fAngle * sinf(m_theta) , D2D1::Point2F( width/2, height/2 ) );

        // Reduce the wobble by the decay factor...
        m_theta += WOBBLE_DECAY;

        m_fAngle -= WOBBLE_DECAY;

        if (m_fAngle <= FLT_EPSILON)
        {
            m_fAngle = 0.0f;
        }
    }

    // Now translate the image relative to the bounding box.
    mat = mat * D2D1::Matrix3x2F::Translation(  m_nrcBound.left * sizeRT.width, m_nrcBound.top * sizeRT.height );

    pRT->SetTransform(mat);

    m_mat = mat;

    pRT->DrawBitmap(m_pBitmap, m_fill);
}


//-------------------------------------------------------------------
// HitTest: Returns true if (x,y) falls within the bitmap.
//-------------------------------------------------------------------

BOOL Sprite::HitTest(int x, int y)
{
    D2D1::Matrix3x2F mat = m_mat;

    // Use the inverse of our current transform matrix to transform the
    // point (x,y) from render-target space to model space.

    mat.Invert();

    D2D1_POINT_2F pt = mat.TransformPoint( D2D1::Point2F((float)x, (float)y) );

    if (pt.x >= m_fill.left && pt.x <= m_fill.right && pt.y >= m_fill.top && pt.y <= m_fill.bottom)
    {
        return true;
    }
    else
    {
        return false;
    }
}



//-------------------------------------------------------------------
// LetterBoxRectF
//
// Given a destination rectangle (rcDest) and an aspect ratio,
// returns a letterboxed rectangle within rcDest.
//-------------------------------------------------------------------

D2D1_RECT_F LetterBoxRectF(D2D1_SIZE_F aspectRatio, const D2D1_RECT_F &rcDest)
{
    float width, height;

    float SrcWidth = aspectRatio.width;
    float SrcHeight = aspectRatio.height;
    float DestWidth = Width(rcDest);
    float DestHeight = Height(rcDest);

    D2D1_RECT_F rcResult = D2D1::RectF();

    // Avoid divide by zero (even though MulDiv handles this)
    if (SrcWidth == 0 || SrcHeight == 0)
    {
        return rcResult;
    }

   // First try: Letterbox along the sides. ("pillarbox")
    width = DestHeight * SrcWidth / SrcHeight;
    height = DestHeight;
    if (width > DestWidth)
    {
        // Letterbox along the top and bottom.
        width = DestWidth;
        height = DestWidth * SrcHeight / SrcWidth;
    }

    // Fill in the rectangle
    
    rcResult.left = rcDest.left + ((DestWidth - width) / 2);
    rcResult.right = rcResult.left + width;
    rcResult.top = rcDest.top + ((DestHeight - height) / 2);
    rcResult.bottom = rcResult.top + height;

    return rcResult;
}

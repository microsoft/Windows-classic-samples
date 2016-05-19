// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Tells a flow layout where text is allowed to flow.
//
//----------------------------------------------------------------------------
#include "common.h"
#include "FlowSource.h"


STDMETHODIMP FlowLayoutSource::Reset()
{
    currentY_ = 0;
    return S_OK;
}


STDMETHODIMP FlowLayoutSource::SetSize(
    float width,
    float height
    )                     
{
    width_  = width;
    height_ = height;
    return S_OK;
}


STDMETHODIMP FlowLayoutSource::SetShape(FlowShape flowShape)
{
    flowShape_ = flowShape;
    return Reset();
}


STDMETHODIMP FlowLayoutSource::GetNextRect(float fontHeight, OUT RectF* nextRect)
{
    RectF& rect = *nextRect;

    // Set defaults.
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = width_;
    rect.bottom = height_;

    if (height_ <= 0 || width_ <= 0)
        return E_FAIL;

    float halfHeight = height_ / 2;
    float halfWidth  = width_  / 2;

    // Simple, hard-coded shape formulas.
    // You can add more shapes by adding a new enum in the header and extending
    // the switch statement.

    switch (flowShape_)
    {
    case FlowShapeFunnel:
        {
            float xShift = float(sin(currentY_ / height_ * M_PI * 3)) * 30;

            // Calculate slope to determine edges.
            rect.top    = currentY_;
            rect.bottom = currentY_ + fontHeight;
            rect.left   = xShift + (currentY_ / height_) * width_ / 2;
            rect.right  = width_ - rect.left;
        }
        break;

    case FlowShapeCircle:
        {
            float adjustedY  = (currentY_ + fontHeight/2) - halfHeight;

            // Determine x from y using circle formula d^2 = (x^2 + y^2).
            float x     = sqrt((halfHeight * halfHeight) - (adjustedY * adjustedY));
            rect.top    = currentY_;
            rect.bottom = currentY_ + fontHeight;
            rect.left   = halfWidth - x;
            rect.right  = halfWidth + x;
        }
        break;
    }

    // Advance down one line, based on font height.
    currentY_ += fontHeight;
    if (currentY_ >= height_)
    {
        // Crop any further lines.
        rect.left = rect.right = 0;
    }

    return S_OK;
}

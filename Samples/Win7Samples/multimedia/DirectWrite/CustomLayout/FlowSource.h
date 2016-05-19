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
#pragma once

class DECLSPEC_UUID("84600D90-8F09-480f-9275-D71125FCD0C3") FlowLayoutSource
    :   public ComBase<
            QiListSelf<FlowLayoutSource,
            QiList<IUnknown
        > > >
{
public:
    enum FlowShape
    {
        FlowShapeCircle,
        FlowShapeFunnel
    };

    struct RectF
    {
        float left;
        float top;
        float right;
        float bottom;
    };

    FlowLayoutSource()
    :   flowShape_(FlowShapeCircle),
        width_(300),
        height_(300)
    {
        Reset();
    }

    STDMETHODIMP Reset();
    STDMETHODIMP SetShape(FlowShape flowShape);
    STDMETHODIMP SetSize(float width, float height);
    STDMETHODIMP GetNextRect(float fontHeight, OUT RectF* nextRect);

protected:
    FlowShape flowShape_;
    float width_;
    float height_;

    float currentY_;
};

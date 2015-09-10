// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <math.h>
#include "Shape.h"

// Shape Constructors
CShape::CShape() :
    _d2dDriver(nullptr),
    _pRenderTarget(nullptr),
    _pPathGeometry(nullptr),
    _objectId(0),
    _isDraggable(true),
    _isSelected(false),
    _objectType(OBJECT_TYPE_REGULAR),
    _text(nullptr)
{
}

CShape::CShape(CD2DDriver *d2dDriver) :
    _d2dDriver(d2dDriver),
    _pRenderTarget(nullptr),
    _pPathGeometry(nullptr),
    _objectId(0),
    _isDraggable(true),
    _isSelected(false),
    _objectType(OBJECT_TYPE_REGULAR),
    _text(nullptr)
{
    InitializeDriver(d2dDriver);
}

// Shape Descructor
CShape::~CShape()
{
    if (_pPathGeometry != nullptr)
    {
        _pPathGeometry->Release();
        _pPathGeometry = nullptr;
    }
}

// initialize with d2dDriver
void CShape::InitializeDriver(CD2DDriver *d2dDriver)
{
    _d2dDriver = d2dDriver;
    // Get the render target for drawing to
    _pRenderTarget = _d2dDriver->GetRenderTarget();
}

// Initialize shape object
void CShape::InitializeObject(_In_reads_(numPoints) const POINT *points, _In_ long numPoints,
                              _In_ bool isDraggable, _In_ int objectId, _In_ DrawingColor color,
                              _In_ DrawingColor onSelectColor, _In_ OBJECT_TYPE objectType)
{
    _polygonRelative.numPoints = numPoints;
    _objectId = objectId;
    _isDraggable = isDraggable;
    _color = color;
    _onSelectColor = onSelectColor;
    _objectType = objectType;

    if (numPoints == 2)
    {
        _polygonRelative.isRect = true;
        _polygonRelative.rcBound.left = points[0].x;
        _polygonRelative.rcBound.top = points[0].y;
        _polygonRelative.rcBound.right = points[1].x;
        _polygonRelative.rcBound.bottom = points[1].y;

        // Initialize points
        _polygonRelative.points[0] = points[0];
        _polygonRelative.points[1].x = points[1].x;
        _polygonRelative.points[1].y = points[0].y;
        _polygonRelative.points[2] = points[1];
        _polygonRelative.points[3].x = points[0].x;
        _polygonRelative.points[3].y = points[1].y;
        _polygonRelative.points[4] = points[0];
        _polygonRelative.numPoints = 5;
    }
    else
    {
        _polygonRelative.isRect = false;

        // calculate width and height
        if (numPoints > 0)
        {
            _polygonRelative.points[0] = points[0];
            _polygonRelative.rcBound.left = _polygonRelative.rcBound.right = points[0].x;
            _polygonRelative.rcBound.top = _polygonRelative.rcBound.bottom = points[0].y;

            // Copy polygon points to _polygonRelative object and calculate polygon bounding box
            for (int i = 1; i < numPoints; i++)
            {
                _polygonRelative.points[i] = points[i];
                _polygonRelative.rcBound.left = min(_polygonRelative.rcBound.left, points[i].x);
                _polygonRelative.rcBound.right = max(_polygonRelative.rcBound.right, points[i].x);
                _polygonRelative.rcBound.top = min(_polygonRelative.rcBound.top, points[i].y);
                _polygonRelative.rcBound.bottom = max(_polygonRelative.rcBound.bottom, points[i].y);
            }
        }
    }

    _polygonAbsolute = _polygonRelative;
}

// Sets the default position, dimensions and color for the drawing object
void CShape::ResetState(_In_ float startX, _In_ float startY,
                        _In_ int scaledWidth, _In_ int scaledHeight,
                        _In_ float scale)
{
    // Set width and height of the client area
    // must adjust for dpi aware
    _width = scaledWidth;
    _height = scaledHeight;

    // Set coordinates used for rendering
    _xR = startX;
    _yR = startY;

    // scaling factor
    _scale = scale;
}

// Draw shape object
void CShape::Paint()
{
    if (!(_pRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        // DimGray is selected if it was clicked previously and pointer is over it
        bool chooseSelectBrush = _isSelected;
        if (chooseSelectBrush)
        {
            chooseSelectBrush = InRegion(_pointerPosition.x, _pointerPosition.y);
        }
        ID2D1LinearGradientBrush *brush = _d2dDriver->GetBrush(chooseSelectBrush ? _onSelectColor : _color);

        // Set positions of gradients based on the new coordinates of the objecs
        brush->SetStartPoint(_UnscaledToClient(*reinterpret_cast<POINT *>(&_polygonRelative.rcBound)));

        brush->SetEndPoint(_UnscaledToClient(*reinterpret_cast<POINT *>(&_polygonRelative.rcBound.right)));

        // Create path to draw
        if (_pPathGeometry != nullptr)
        {
            _pPathGeometry->Release();
        }
        _d2dDriver->CreatePathGeometry(&_pPathGeometry);
        ID2D1GeometrySink *pSink;
        _pPathGeometry->Open(&pSink);

        if (!_polygonRelative.isRect)
        {
            pSink->BeginFigure(
                _UnscaledToClient(_polygonRelative.points[0]),
                D2D1_FIGURE_BEGIN_FILLED);
        }
        else
        {
            pSink->BeginFigure(
                _UnscaledToClient(*reinterpret_cast<POINT *>(&_polygonRelative.rcBound)),
                D2D1_FIGURE_BEGIN_FILLED);
        }

        // Draw polygon
        for (int i = 1; i < _polygonRelative.numPoints; i++)
        {
            pSink->AddLine(_UnscaledToClient(_polygonRelative.points[i]));
        }

        pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
        pSink->Close();
        pSink->Release();

        // Fill the geometry that was created
        _pRenderTarget->FillGeometry(_pPathGeometry, brush);

        // Draw text if there's any
        if (_text != nullptr)
        {
            _pRenderTarget->DrawText(
                _text,
                _textLen,
                _d2dDriver->GetTextFormat(),
                _UnscaledToClient(_polygonRelative.rcBound),
                _d2dDriver->GetBrush(Orange));
        }

        // Create absolute polygon
        for (int i = 0; i < _polygonRelative.numPoints; i++)
        {
            _UnscaledToClient(_polygonRelative.points[i], &_polygonAbsolute.points[i]);
        }
        _UnscaledToClient(*reinterpret_cast<POINT *>(&_polygonRelative.rcBound), reinterpret_cast<POINT *>((&_polygonAbsolute.rcBound)));
        _UnscaledToClient(*reinterpret_cast<POINT *>(&_polygonRelative.rcBound.right), reinterpret_cast<POINT *>((&_polygonAbsolute.rcBound.right)));
    }
}

// Hit testing method handled with Direct2D
bool CShape::InRegion(_In_ long x, _In_ long y)
{
    BOOL b = FALSE;

    _pPathGeometry->FillContainsPoint(
        D2D1::Point2F(static_cast<float>(x), static_cast<float>(y)),
        D2D1::Matrix3x2F::Identity(),
        &b
    );

    return !!b;
}

// Pointer event handler
void CShape::OnPointerEvent(_In_ POINT pt, _In_ EVENT_TYPE et)
{
    switch (et)
    {
    case EVENT_TYPE_DOWN:
        _isSelected = true;
        break;
    case EVENT_TYPE_MOVE:
        if (_isDraggable)
        {
            _Translate(static_cast<float>(pt.x - _pointerPosition.x), static_cast<float>(pt.y - _pointerPosition.y));
        }
        break;
    case EVENT_TYPE_UP:
        _isSelected = false;
    default:
        break;
    }
    _pointerPosition = pt;
}

// Id of this shape
int CShape::GetObjectId() const
{
    return _objectId;
}

// Shape label (applicable for controls)
void CShape::SetText(_In_z_ wchar_t const *text)
{
    _text = text;
    _textLen = 0;
    while (_text[_textLen] != L'\0')
    {
        _textLen++;
    }
}

// Shape polygonal representation
POLYGON *CShape::GetPolygon()
{
    return &_polygonAbsolute;
}

// Shape type (e.g. regular shape or control shape)
OBJECT_TYPE CShape::GetObjectType() const
{
    return _objectType;
}

// Update shape position to ensure that it stays within client screen
void CShape::_EnsureVisible()
{
    // Initialize width height of object
    float width   = static_cast<float>(_polygonRelative.rcBound.right - _polygonRelative.rcBound.left) * _scale;
    float height  = static_cast<float>(_polygonRelative.rcBound.bottom - _polygonRelative.rcBound.top) * _scale;

    float x = _xR + static_cast<float>(_polygonRelative.rcBound.left) * _scale;
    x = max(0, min(x, static_cast<float>(_width) - width));
    _xR = x - static_cast<float>(_polygonRelative.rcBound.left) * _scale;

    float y = _yR + static_cast<float>(_polygonRelative.rcBound.top) * _scale;
    y = max(0, min(y, static_cast<float>(_height) - height));
    _yR = y - static_cast<float>(_polygonRelative.rcBound.top) * _scale;
}

// _Translate shape within client screen
void CShape::_Translate(_In_ float dX, _In_ float dY)
{
    _xR += dX;
    _yR += dY;

    // Make sure object stays on screen
    _EnsureVisible();
}

// Convert unscaled point to client coordinates
void CShape::_UnscaledToClient(_In_ POINT ptUnscaled, _Inout_ POINT *pClient)
{
    pClient->x = static_cast<long>(_xR + ptUnscaled.x * _scale);
    pClient->y = static_cast<long>(_yR + ptUnscaled.y * _scale);
}

// Convert unscaled point to client coordinates
D2D1_POINT_2F CShape::_UnscaledToClient(_In_ POINT unscaled)
{
    return D2D1::Point2F(floor(_xR + unscaled.x * _scale), floor(_yR + unscaled.y * _scale));
}

// Convert unscaled rectangle to client coordinates
D2D1_RECT_F CShape::_UnscaledToClient(_In_ RECT unscaled)
{
    return D2D1::RectF(
        floor(_xR + unscaled.left * _scale),
        floor(_yR + unscaled.top * _scale),
        floor(_xR + unscaled.right * _scale),
        floor(_yR + unscaled.bottom * _scale)
    );
}

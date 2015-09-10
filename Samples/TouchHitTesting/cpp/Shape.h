// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef SHAPE_H
#define SHAPE_H

#include "D2DDriver.h"

//
// Definitions
//
#define MAX_POLY_VERTICES          50  // Maximal number of points in polygonal shape
#define CURSOR_ID_MOUSE            0   // Identifier of mouse input
#define CURSOR_ID_NONE            -1   // Identifier of no input

//
// Structures
//

// Types of objects
enum OBJECT_TYPE
{
    OBJECT_TYPE_CONTROL,                // Used as a control (button)
    OBJECT_TYPE_REGULAR                 // Used as a shape (can be moved)
};

// Types of input events
enum EVENT_TYPE
{
    EVENT_TYPE_DOWN,                    // Touch, mouse or pen (pointer) down event
    EVENT_TYPE_MOVE,                    // Touch, mouse or pen move (update) event
    EVENT_TYPE_UP                       // Touch, mouse or pen up event
};

// Sources of input events
enum EVENT_SOURCE
{
    EVENT_SOURCE_POINTER,                // Pointer (touch or pen)
    EVENT_SOURCE_MOUSE                   // Mouse
};

// Input event structure
struct INPUT_EVENT
{
    POINT pt;                            // Location of input event
    EVENT_TYPE eventType;                // Type of the event (down, move or up)
    EVENT_SOURCE eventSource;            // Source of the event (pointer or mouse)
    int cursorId;                        // Identifiear associated with event
};

// Polygonal shape structure
struct POLYGON
{
    bool isRect;                          // Is the polygon rectangle
    long numPoints;                       // Number of vertices in polgyon
    POINT points[MAX_POLY_VERTICES];      // Polygon vertices
    RECT rcBound;                         // Polygon bounding box
};

// Shape class represents a windowless control that can be used as a button in application
// or used as a draggable polygonal object.
//
// Shape has following properties:
//     1) It can be control or regular object
//     2) It has main color and on selection color
//     3) It can be draggable with pointer
//     4) If it is control, it has a text displayed
//     5) It has polygonal shape
//
// Shape can be scaled and translated on the screen.
class CShape
{
public:
    CShape();
    CShape(CD2DDriver *d2dDriver);
    ~CShape();

    // initialize with d2dDriver
    void InitializeDriver(CD2DDriver *d2dDriver);

    // Initialize shape
    void InitializeObject(_In_reads_(numPoints) POINT const *points, _In_ long numPoints,
                          _In_ bool isDraggable, _In_ int objectId, _In_ DrawingColor color,
                          _In_ DrawingColor onSelectColor, _In_ OBJECT_TYPE objectType);

    // Reset shape to it's start position
    void ResetState(_In_ float startX, _In_ float startY,
                    _In_ int scaledWidth, _In_ int scaledHeight,
                    _In_ float scale);

    // Draw the shape
    void Paint();

    // Hit test point - returns true if point is within shape
    bool InRegion(_In_ long lX, _In_ long lY);

    // Respond to input event
    void OnPointerEvent(_In_ POINT pt, _In_ EVENT_TYPE et);

    // Properties
    int GetObjectId() const;
    void SetText(_In_z_ wchar_t const *text);
    POLYGON *GetPolygon();
    OBJECT_TYPE GetObjectType() const;

private:
    // Rendering
    CD2DDriver *_d2dDriver;
    ID2D1HwndRenderTarget *_pRenderTarget;
    ID2D1PathGeometry *_pPathGeometry;

    // Rendered top, left coordinates of object
    float _xR;
    float _yR;

    // Client (parent window) width and height
    int _width;
    int _height;

    // How much shape is scaled
    float _scale;

    // Object shape in relative coordinates (before scaling and translation)
    POLYGON _polygonRelative;
    // Object shape in absolute coordinates (scaling and translation applied)
    POLYGON _polygonAbsolute;

    // Is object draggable
    bool _isDraggable;

    // Object ID
    int _objectId;

    // Object colors
    DrawingColor _color;
    DrawingColor _onSelectColor;

    // Type of object
    OBJECT_TYPE _objectType;

    // Is object selected
    bool _isSelected;

    // Text of control type object
    wchar_t const *_text;
    unsigned int _textLen;

    // Pixel location where last pointer event occurred
    POINT _pointerPosition;

    // Move the object on the screen
    void _EnsureVisible();
    void _Translate(_In_ float dX, _In_ float dY);

    // Helper functions to deal with scaled object
    void _UnscaledToClient(_In_ POINT unscaled, _Inout_ POINT *pClient);
    D2D1_POINT_2F _UnscaledToClient(_In_ POINT unscaled);
    D2D1_RECT_F _UnscaledToClient(_In_ RECT unscaled);
};

#endif

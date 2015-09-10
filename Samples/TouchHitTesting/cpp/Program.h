// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef PROGRAM_H
#define PROGRAM_H

#include <new>
#include "Shape.h"

// Shapes list
struct ShapeRecord
{
    CShape *pShape;                      // Internal shape object
    int cursorId;                        // Cursor (mouse or pointer) identifier that 'holds' the object
    ShapeRecord *pNext;                 // Next object
};

class CProgram
{
public:
    CProgram();
    ~CProgram();

    // Initializes Core Objects, Manipulation Processors and Inertia Processors
    bool Initialize(_In_ HWND hWnd, _In_z_ const wchar_t *touchHitTestingOnText, _In_z_ const wchar_t *touchHitTestingOffText, _In_z_ const wchar_t *resetText);

    // Sets up the initial state of the objects
    void RenderInitialState(_In_ int width, _In_ int height);

    // Localizes point for high-DPI
    int GetLocalizedPointX(_In_ int x);
    int GetLocalizedPointY(_In_ int y);

    void ProcessInputEvent(_In_reads_(numInputEvents) INPUT_EVENT const *inputEvents, _In_ int numInputEvents);
    void RenderObjects();
    LRESULT OnTouchHitTesting(_In_ TOUCH_HIT_TESTING_INPUT const *pHitTestingInput);

private:
    // DimGray's actions
    void _OnTouchHitTestingOnOff(_Inout_ CShape *object);
    void _OnReset();

    void _SetupControls();

    // List of shapes (includes regular shapes and controls)
    // List contains shapes in their z-order
    ShapeRecord *_pShapes;

    // Add new shape to the list
    void _AddShape(_In_ CShape *pShape);
    // Move shape to the top of the list
    void _MoveToFront(_Inout_ ShapeRecord *pShapeRec);
    // Drawing
    void _DrawBackward(_In_ ShapeRecord const *pShapeRec);

    // Main window
    HWND _hwnd;

    // The client width and height
    int _width;
    int _height;

    // Scale for converting between dpi's
    float _dpiScaleX;
    float _dpiScaleY;

    CD2DDriver *_d2dDriver;

    // Event handlers
    void _DownEvent(_In_ INPUT_EVENT const &inputEvent);
    void _MoveEvent(_In_ INPUT_EVENT const &inputEvent);
    void _UpEvent(_In_ INPUT_EVENT const &inputEvent);

    // Hit test method
    ShapeRecord *_PointHitTest(_In_ int x, _In_ int y);

    // DimGray labels
    wchar_t const *_touchHitTestingOnText;
    wchar_t const *_touchHitTestingOffText;
    wchar_t const *_resetText;

    // Touch Hit Testing state
    bool _touchHitTestingEnabled;

    // static allocation of shapes
    static unsigned int const s_maxShapes = 20;
    ShapeRecord _shapeRecords[s_maxShapes];
    CShape _shapes[s_maxShapes];
    unsigned int _nextShapeRecord;
    unsigned int _nextShape;

    ShapeRecord *_MakeShapeRecord();
    CShape *_MakeShape(CD2DDriver *d2dDriver);
};

#endif

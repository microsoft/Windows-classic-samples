// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <stdio.h>
#include "Program.h"

//
// Definitions
//

#define CONTROL_ID_NONE                     0      // Identifier of regular object
#define CONTROL_ID_TOUCH_HIT_TESTING_ON_OFF 1001   // Identifier of Touch Hit Testing On/Off button
#define CONTROL_ID_RESET                    1002   // Identifier of Reset button
#define CONTROLS_AREA_SIZE                  300    // Relative size of area where regular shapes are initially drawn
#define DEFAULT_PPI                         96.0f  // Default dpi

CProgram::CProgram():
    _dpiScaleX(1.0f),
    _dpiScaleY(1.0f),
    _touchHitTestingEnabled(true),
    _hwnd(nullptr),
    _pShapes(nullptr),
    _d2dDriver(nullptr),
    _nextShapeRecord(0),
    _nextShape(0)
{
}

CProgram::~CProgram()
{
    delete _d2dDriver;
}

// Add new shape to the top of the list
void CProgram::_AddShape(_In_ CShape *pShape)
{
    // Create new record for this shape
    ShapeRecord *pShapeRecord = _MakeShapeRecord();

    pShapeRecord->pShape = pShape;
    pShapeRecord->pNext = nullptr;
    pShapeRecord->cursorId = CURSOR_ID_NONE;

    if (_pShapes == nullptr)
    {
        // The first entry
        _pShapes = pShapeRecord;
    }
    else
    {
        // Add to top
        pShapeRecord->pNext = _pShapes;
        _pShapes = pShapeRecord;
    }
}

// Move selected shape to the top
void CProgram::_MoveToFront(_Inout_ ShapeRecord *pShapeRec)
{
    // Find previos shape
    ShapeRecord *pShapeRecPrev = _pShapes;
    while (pShapeRecPrev != nullptr && (pShapeRecPrev->pNext != pShapeRec))
    {
        pShapeRecPrev = pShapeRecPrev->pNext;
    }
    // Skip current shape
    if (pShapeRecPrev != nullptr)
    {
        pShapeRecPrev->pNext = pShapeRec->pNext;
    }
    else
    {
        // Already on the top.
        return;
    }

    if (pShapeRec->pShape->GetObjectType() == OBJECT_TYPE_CONTROL)
    {
        // Move control to the top
        pShapeRec->pNext = _pShapes;
        _pShapes = pShapeRec;
    }
    else
    {
        // Otherwise, move it to the top after all controls
        ShapeRecord *pShapeRecNewPrev = _pShapes;
        while (pShapeRecNewPrev != nullptr &&
               pShapeRecNewPrev->pNext != nullptr &&
               (pShapeRecNewPrev->pNext->pShape->GetObjectType() == OBJECT_TYPE_CONTROL))
        {
            pShapeRecNewPrev = pShapeRecNewPrev->pNext;
        }
        if (pShapeRecNewPrev != nullptr)
        {
            pShapeRec->pNext = pShapeRecNewPrev->pNext;
            pShapeRecNewPrev->pNext = pShapeRec;
        }
        else
        {
            // No controls, put the object on the top
            pShapeRec->pNext = _pShapes;
            _pShapes = pShapeRec;
        }
    }
}

// Initialize and place all controls on the page
void CProgram::_SetupControls()
{
    CShape *object = nullptr;
    POINT poly[MAX_POLY_VERTICES];

    // Drawing parameters
    // Triangle side
    long triangleSide = CONTROLS_AREA_SIZE;
    // Triangle height
    long triangleHeight = triangleSide * 87 / 100;

    // 1st control
    // Rectangle
    poly[0].x = triangleSide   / 3;
    poly[0].y = triangleHeight * 2 / 3;
    poly[1].x = triangleSide   * 2 / 3;
    poly[1].y = triangleHeight;
    object = _MakeShape(_d2dDriver);
    object->InitializeObject(poly, 2, true, CONTROL_ID_NONE, Blue, Blue, OBJECT_TYPE_REGULAR);
    _AddShape(object);

    // 2nd control
    // Triangle
    poly[0].x = triangleSide   / 3;
    poly[0].y = triangleHeight * 2 / 3;
    poly[1].x = triangleSide   * 2 / 3;
    poly[1].y = triangleHeight * 2 / 3;
    poly[2].x = triangleSide   / 2;
    poly[2].y = triangleHeight / 3;
    object = _MakeShape(_d2dDriver);
    object->InitializeObject(poly, 3, true, CONTROL_ID_NONE, Orange, Orange, OBJECT_TYPE_REGULAR);
    _AddShape(object);

    // 3rd control
    // Touch Hit Testing On/Off button
    poly[0].x = -20;
    poly[0].y = CONTROLS_AREA_SIZE + 10;
    poly[1].x = CONTROLS_AREA_SIZE / 3 - 20;
    poly[1].y = CONTROLS_AREA_SIZE + 10 + CONTROLS_AREA_SIZE / 5;
    object = _MakeShape(_d2dDriver);
    object->InitializeObject(poly, 2, false, CONTROL_ID_TOUCH_HIT_TESTING_ON_OFF, DimGray, DarkGray, OBJECT_TYPE_CONTROL);
    object->SetText(_touchHitTestingOnText);
    _AddShape(object);

    // 4th control
    // Reset button
    poly[0].x = CONTROLS_AREA_SIZE + 20 - CONTROLS_AREA_SIZE / 3;
    poly[0].y = CONTROLS_AREA_SIZE + 10;
    poly[1].x = CONTROLS_AREA_SIZE + 20;
    poly[1].y = CONTROLS_AREA_SIZE + 10 + CONTROLS_AREA_SIZE / 5;
    object = _MakeShape(_d2dDriver);
    object->InitializeObject(poly, 2, false, CONTROL_ID_RESET, DimGray, DarkGray, OBJECT_TYPE_CONTROL);
    object->SetText(_resetText);
    _AddShape(object);
}

// Initialize page
bool CProgram::Initialize(_In_ HWND hWnd,
                          _In_z_ wchar_t const *touchHitTestingOnText,
                          _In_z_ wchar_t const *touchHitTestingOffText,
                          _In_z_ wchar_t const *resetText)
{
    bool success = true;

    _touchHitTestingOnText = touchHitTestingOnText;
    _touchHitTestingOffText = touchHitTestingOffText;
    _resetText = resetText;

    _hwnd = hWnd;

    // Enable Touch hit testing
    if (!RegisterTouchHitTestingWindow(_hwnd, _touchHitTestingEnabled ? TOUCH_HIT_TESTING_CLIENT : TOUCH_HIT_TESTING_DEFAULT))
    {
        // Registration failed...
        success = false;
        fprintf(stderr, "Failed to register touch hit testing window, error code %u.\n", GetLastError());
    }

    if (success)
    {
        // Calculate dpi for high-DPI systems
        HDC hdcScreen = GetDC(hWnd);

        if (hdcScreen)
        {
            // Direct2D automatically does work in logical, so compute the
            // scale to convert from physical to logical coordinates
            _dpiScaleX = static_cast<float>(DEFAULT_PPI / GetDeviceCaps(hdcScreen, LOGPIXELSX));
            _dpiScaleY = static_cast<float>(DEFAULT_PPI / GetDeviceCaps(hdcScreen, LOGPIXELSY));
            DeleteDC(hdcScreen);
        }
        else
        {
            success = false;
            fprintf(stderr, "Failed to GetDC from hwnd, error code %u.\n", GetLastError());
        }
    }

    // Create D2D driver
    if (success)
    {
        _d2dDriver = new(std::nothrow) CD2DDriver(hWnd);
        if (_d2dDriver == nullptr)
        {
            success = false;
            fprintf(stderr, "Failed to allocate memory for new object.\n");
        }
    }

    // Initialize D2D driver.
    if (success)
    {
        success = SUCCEEDED(_d2dDriver->Initialize());
    }

    // Create and initialize core objects
    if (success)
    {
        _SetupControls();
    }

    return success;
}

// Draw shapes in backward order.
// First draw shape with the lowest z-order and so on.
// The last drawn shape has the highest z-order.
void CProgram::_DrawBackward(_In_ ShapeRecord const *pShapeRec)
{
    if (pShapeRec != nullptr)
    {
        _DrawBackward(pShapeRec->pNext);
        pShapeRec->pShape->Paint();
    }
}

// Render whole page.
void CProgram::RenderObjects()
{
    _d2dDriver->BeginDraw();

    // Background
    _d2dDriver->RenderBackground(static_cast<float>(_width), static_cast<float>(_height));

    // Shapes (controls)
    _DrawBackward(_pShapes);

    _d2dDriver->EndDraw();
}

// Initial rendering of the page
void CProgram::RenderInitialState(_In_ int width, _In_ int height)
{
    if ((width == 0) || (height == 0))
    {
        // Window is minimized
        return;
    }

    _width = width;
    _height = height;

    int widthScaled = GetLocalizedPointX(width);
    int heightScaled = GetLocalizedPointY(height);

    // Scaling factor
    int minWH = widthScaled > heightScaled ? heightScaled : widthScaled;
    float fScale = static_cast<float>(minWH) / (2 * CONTROLS_AREA_SIZE);

    // Origin of the objects
    POINTF orig;

    orig.x = static_cast<float>(widthScaled - CONTROLS_AREA_SIZE * fScale) / 2.0f;
    orig.y = static_cast<float>(heightScaled - CONTROLS_AREA_SIZE * fScale) / 2.0f;

    if (_d2dDriver->GetRenderTarget())
    {
        HRESULT hr;
        D2D1_SIZE_U size;
        size.width = _width;
        size.height = _height;
        hr = _d2dDriver->GetRenderTarget()->Resize(size);

        if (FAILED(hr))
        {
            _d2dDriver->DiscardDeviceResources();
        }
    }

    // Assign the setup defined above to each of the core objects
    ShapeRecord *pShapeRec = _pShapes;
    while (pShapeRec != nullptr)
    {
        pShapeRec->pShape->ResetState(orig.x, orig.y, widthScaled, heightScaled, fScale);
        pShapeRec = pShapeRec->pNext;
    }

    RenderObjects();
}

int CProgram::GetLocalizedPointX(_In_ int x)
{
    return static_cast<int>(x * _dpiScaleX);
}

int CProgram::GetLocalizedPointY(_In_ int y)
{
    return static_cast<int>(y * _dpiScaleY);
}

void CProgram::ProcessInputEvent(_In_reads_(numInputEvents) INPUT_EVENT const *inputEvents, _In_ int numInputEvents)
{
    for (int i = 0; i < numInputEvents; i++)
    {
        switch (inputEvents[i].eventType)
        {
        case EVENT_TYPE_DOWN:
            _DownEvent(inputEvents[i]);
            break;
        case EVENT_TYPE_MOVE:
            _MoveEvent(inputEvents[i]);
            break;
        case EVENT_TYPE_UP:
        default:
            _UpEvent(inputEvents[i]);
            break;
        }
    }
}

// Return shape that is hit with given point.
ShapeRecord *CProgram::_PointHitTest(_In_ int x, _In_ int y)
{
    ShapeRecord *pShapeRec = _pShapes;
    while (pShapeRec != nullptr)
    {
        if (pShapeRec->pShape->InRegion(x, y))
        {
            return pShapeRec;
        }
        pShapeRec = pShapeRec->pNext;
    }

    return nullptr;
}

// On pointer down event select target shape (associate shape with cursor)
// and move shape to the front.
void CProgram::_DownEvent(_In_ INPUT_EVENT const &inputEvent)
{
    POINT pt = {GetLocalizedPointX(inputEvent.pt.x), GetLocalizedPointY(inputEvent.pt.y)};

    ShapeRecord *pShapeRec = _PointHitTest(pt.x, pt.y);
    if (pShapeRec != nullptr)
    {
        pShapeRec->cursorId = inputEvent.cursorId;
        pShapeRec->pShape->OnPointerEvent(pt, inputEvent.eventType);

        _MoveToFront(pShapeRec);

        // update
        RenderObjects();
    }
}

// On pointer move event, translate the shape that is associated with
// current cursor (if any).
void CProgram::_MoveEvent(_In_ INPUT_EVENT const &inputEvent)
{
    // find object held with this cursor
    ShapeRecord *pShapeRec = _pShapes;
    while (pShapeRec != nullptr && (pShapeRec->cursorId != inputEvent.cursorId))
    {
        pShapeRec = pShapeRec->pNext;
    }

    // move object
    if (pShapeRec != nullptr)
    {
        // move the object
        POINT pt = {GetLocalizedPointX(inputEvent.pt.x), GetLocalizedPointY(inputEvent.pt.y)};
        pShapeRec->pShape->OnPointerEvent(pt, inputEvent.eventType);

        // update
        RenderObjects();
    }
}

// Action associated with Touch Hit Testing button
// Flip the intelligent Touch Hit Testing state
void CProgram::_OnTouchHitTestingOnOff(_Inout_ CShape *object)
{
    _touchHitTestingEnabled = !_touchHitTestingEnabled;

    if (!RegisterTouchHitTestingWindow(_hwnd, _touchHitTestingEnabled ? TOUCH_HIT_TESTING_CLIENT : TOUCH_HIT_TESTING_DEFAULT))
    {
        // Registration failed... should never happen
    }

    object->SetText(_touchHitTestingEnabled ? _touchHitTestingOnText : _touchHitTestingOffText);

    RenderObjects();
}

// Action associated with Reset button
// Reset positions of the shapes on the page
void CProgram::_OnReset()
{
    RenderInitialState(_width, _height);
}

// On pointer up event, release the associated shape (if any).
void CProgram::_UpEvent(_In_ INPUT_EVENT const &inputEvent)
{
    // find object held with this cursor
    ShapeRecord *pShapeRec = _pShapes;
    while (pShapeRec != nullptr && (pShapeRec->cursorId != inputEvent.cursorId))
    {
        pShapeRec = pShapeRec->pNext;
    }

    if (pShapeRec != nullptr)
    {
        POINT pt = {GetLocalizedPointX(inputEvent.pt.x), GetLocalizedPointY(inputEvent.pt.y)};
        pShapeRec->pShape->OnPointerEvent(pt, inputEvent.eventType);

        // Release cursor from shape
        pShapeRec->cursorId = CURSOR_ID_NONE;

        // Is control clicked?
        if (pShapeRec->pShape->GetObjectId() != 0)
        {
            // Is the up point within control?
            ShapeRecord *pShapeRec2 = _PointHitTest(pt.x, pt.y);
            if (pShapeRec == pShapeRec2)
            {
                // What is the action?
                if (pShapeRec->pShape->GetObjectId() == CONTROL_ID_TOUCH_HIT_TESTING_ON_OFF)
                {
                    _OnTouchHitTestingOnOff(pShapeRec->pShape);
                }
                else
                {
                    _OnReset();
                }
            }
        }
    }
}

// Intelligent Touch Hit Testing handler - sample implementation of the Touch Hit Testing
// Algorithm.
LRESULT CProgram::OnTouchHitTesting(_In_ TOUCH_HIT_TESTING_INPUT const *pHitTestingInput)
{
    // Current and best touch hit testing results
    TOUCH_HIT_TESTING_PROXIMITY_EVALUATION bestResult;
    TOUCH_HIT_TESTING_PROXIMITY_EVALUATION currentResult;

    // Best touch hit testing target
    CShape *bestTarget = nullptr;

    // Initialize best result to the worst possible score
    bestResult.score = TOUCH_HIT_TESTING_PROXIMITY_FARTHEST;
    bestResult.adjustedPoint = pHitTestingInput->point;

    // Convert input arguments to client coordinates.
    // Input arguments are touch point, bounding box of touch contact
    // and non occluded part of touch contact bounding box.
    TOUCH_HIT_TESTING_INPUT hitTestingInput = *pHitTestingInput;
    ScreenToClient(_hwnd, &hitTestingInput.point);
    ScreenToClient(_hwnd, reinterpret_cast<POINT *>(&hitTestingInput.boundingBox));
    ScreenToClient(_hwnd, reinterpret_cast<POINT *>(&hitTestingInput.boundingBox.right));
    ScreenToClient(_hwnd, reinterpret_cast<POINT *>(&hitTestingInput.nonOccludedBoundingBox));
    ScreenToClient(_hwnd, reinterpret_cast<POINT *>(&hitTestingInput.nonOccludedBoundingBox.right));

    // Iterate through all shapes by their z-order (starting from shape with highest z-order)
    // Stop early if there's no chance to find better target.
    ShapeRecord *pShapeRec = _pShapes;
    while (pShapeRec != nullptr &&
        (bestResult.score > TOUCH_HIT_TESTING_PROXIMITY_CLOSEST))
    {
        BOOL retVal;
        POLYGON *poly = pShapeRec->pShape->GetPolygon();

        // Hit test current shape
        if (poly->isRect)
        {
            retVal = EvaluateProximityToRect(&poly->rcBound, &hitTestingInput, &currentResult);
        }
        else
        {
            retVal = EvaluateProximityToPolygon(poly->numPoints, poly->points, &hitTestingInput, &currentResult);
        }

        // Does the shape intersect touch contact and is it the best target so far?
        if (retVal &&
            (currentResult.score < bestResult.score))
        {
            // Even if this shape has better score than the score of the shape with higher z-order,
            // don't select this shape if previous adjusted point belongs to this shape.
            // In other words, if this shape overlaps shape with higher z-order, favor the shape with
            // higher z-order.
            if ((bestTarget == nullptr) ||
                !pShapeRec->pShape->InRegion(bestResult.adjustedPoint.x, bestResult.adjustedPoint.y))
            {
                // Finally, don't select target if adjusted touch point is not on the control.
                // That may happen if another shape with higher z-order overlaps this shape at adjusted touch
                // point.
                if (_PointHitTest(currentResult.adjustedPoint.x, currentResult.adjustedPoint.y) == pShapeRec)
                {
                    // This is the best touch target so far.
                    bestTarget = pShapeRec->pShape;
                    bestResult = currentResult;
                }
            }
        }

        // Evaluate next shape.
        pShapeRec = pShapeRec->pNext;
    }

    return PackTouchHitTestingProximityEvaluation(&hitTestingInput, &bestResult);
}


ShapeRecord *CProgram::_MakeShapeRecord()
{
    return &_shapeRecords[_nextShapeRecord++];
}

CShape *CProgram::_MakeShape(CD2DDriver* d2dDriver)
{
    _shapes[_nextShape].InitializeDriver(d2dDriver);
    return &_shapes[_nextShape++];
}

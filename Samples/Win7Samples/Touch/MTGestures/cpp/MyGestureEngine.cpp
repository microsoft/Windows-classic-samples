// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// MyGestureEngine.cpp: implementation of CMyGestureEngine class

#include "MyGestureEngine.h"

// Constructor
// in:
//      pcRect - pointer to the CDrawingObject class (rectangle)
CMyGestureEngine::CMyGestureEngine(CDrawingObject* pcRect)
:    CGestureEngine(),
    _pcRect(pcRect)
{
}

// Destructor
CMyGestureEngine::~CMyGestureEngine()
{
}

// Press and tap command
void CMyGestureEngine::ProcessPressAndTap()
{
    if (_pcRect)
    {
        _pcRect->ShiftColor();
    }
}

// Two finger tap command
void CMyGestureEngine::ProcessTwoFingerTap()
{
    if (_pcRect)
    {
        _pcRect->ToggleDrawDiagonals();
    }
}

// Zoom command
// in: 
//      dZoomFactor - scaling factor of zoom in/out
//      lZx         - x-coordinate of zoom center
//      lZy         - y-coordinate of zoom center
void CMyGestureEngine::ProcessZoom(const double dZoomFactor, const LONG lZx, const LONG lZy)
{
    if (_pcRect)
    {
        _pcRect->Zoom(dZoomFactor, lZx, lZy);
    }
}

// Pan/Inertia command
// in:
//      ldx - increment/decrement in x direction
//      ldy - increment/decrement in y direction
void CMyGestureEngine::ProcessMove(const LONG ldx, const LONG ldy)
{
    if (_pcRect)
    {
        _pcRect->Move(ldx, ldy);
    }
}

// Rotate command
// in:
//      dAngle  - angle of rotation
//      lOx     - x-coordinate of the center of rotation
//      lOy     - y-coordinate of the center of rotation
void CMyGestureEngine::ProcessRotate(const double dAngle, const LONG lOx, const LONG lOy)
{
    if (_pcRect)
    {
        _pcRect->Rotate(dAngle, lOx, lOy);
    }
}

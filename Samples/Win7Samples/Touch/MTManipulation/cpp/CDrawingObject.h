// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// DrawingObject.h
//
// Definition of helper class for drawing and manipulation of rectangle object.

#pragma once

// Windows header files
#include <windows.h>

// C RunTime header files
#include <math.h>

// CDrawingObject class holds information about the rectangle. Instead of storing
// two oposite angle points we do calculate them dynamically by using other 
// information that we store in this class about the rectangle. This way we do
// not get deformation of the rectangle due to precission issues.
class CDrawingObject
{
public:
    CDrawingObject();

    void ResetObject(const int cxClient, const int cyClient);
    void Paint(HDC hdc);
    void ApplyManipulationDelta(
        const FLOAT translationDeltaX,
        const FLOAT translationDeltaY,
        const FLOAT scaleDelta,
        const FLOAT rotationDelta);

public:
    ~CDrawingObject();

private:
    // We do retain center point of the rectangle (diagonal intersection)
    POINT _ptCenter;

    // Then we keep information about the size of the rectangle
    SIZE _szRect;

    // Zoom in/out will scale the width and the height by some factor
    double  _dScalingFactor; 

    // Here we store total rotation angle of the rectangle (from x-axis)
    double  _dRotationAngle; 
};

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

#define MAX_COLORS 5 // The number of colors that we are going to shift throuh

// CDrawingObject class holds information about the rectangle. Instead of storing
// two oposite corner points we do calculate them dynamically by using the other 
// information that we store in this class about the rectangle. This way we do 
// not get deformation of the rectangle due to precission issues.
class CDrawingObject
{
public:
    CDrawingObject();

    void ResetObject(const int cxClient, const int cyClient);

    void Paint(HDC hdc);
    void Move(LONG ldx, LONG ldy);
    void ToggleDrawDiagonals() { _bDrawDiagonals = !_bDrawDiagonals; }
    void Zoom(const double dZoomFactor, const LONG iZx, const LONG iZy);
    void Rotate(const double dAngle, const LONG iOx, const LONG iOy);
    void ShiftColor();

public:
    ~CDrawingObject();

private:
    // This is an array of colors that we will shift through when a user 
    // generates press and tap gesture
    static const COLORREF s_colors[];

    // We do retain the center point of the rectangle (diagonal intesection)
    LONG    _iCx; 
    LONG    _iCy; 

    // Then we keep information about width and height of the rectangle
    int     _iWidth; 
    int     _iHeight; 

    // Zooming in/out will scale the width and the height by some factor
    double  _dScalingFactor; 

    // Here we store total rotation angle of the rectangle (from x-axis)
    double  _dRotationAngle; 

    // This variable triggers drawing of diagonals if set to true
    bool    _bDrawDiagonals; 

    // This variable defines the color of the rectangle (index)
    int     _iColorIndex; 
};

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
// This class handles manipulation of the drawing object (rectangle) as a 
// response to multi-touch gestures
//
// Initially, we define rectangle to be in the middle of the client area. 
// Whenever user resizes the window the rectangle is placed in the middle of the
// client area with the width set to a half of the width of the client area and 
// height set to a half of the height of the client area. The rectangle is placed
// in the center of the client area. 
//
// Through the gestures user can zoom in, zoom out, move or rotate the rectangle. 
// By invoking two finger tap the user can add/remove diagonals from the drawing.
//

#include "DrawingObject.h"

// This macro is used to round double and cast it to LONG
#define ROUND_DOUBLE_TO_LONG(x) ((LONG)floor(0.5 + (x)))

// This is initialization of colors that we are going to shift through the 
// rectangle object whenever the user invokes press and tap gesture
const COLORREF CDrawingObject::s_colors[] = 
{
    RGB(0, 0, 0),     // black
    RGB(255, 255, 0), // yellow
    RGB(255, 0, 0),   // red
    RGB(0, 255, 0),   // green
    RGB(0, 0, 255)    // blue
};

// Default constructor
CDrawingObject::CDrawingObject()
{
    // The main window application is responsible to invoke ResetObject function
    // to initialize variables. 
    // It should be done whenever the main window gets WM_SIZE message.
}

// Destructor
CDrawingObject::~CDrawingObject()
{
}

// This function shifts color index by one
void CDrawingObject::ShiftColor()
{
    // increase current color index by one
    _iColorIndex++;

    if (MAX_COLORS == _iColorIndex)
    {
        // if index is out of the bound then reset it to zero
        _iColorIndex = 0;
    }
}

// This function resets the rectangle object information and it is called by 
// the main app whenever the user resizes the client area
// in:
//      cxClient - new width of the client area
//      cyClient - new height of the client area
void CDrawingObject::ResetObject(const int cxClient, const int cyClient)
{
    // Initial positon of the rectangle is the center of the client area
    _iCx = cxClient/2;
    _iCy = cyClient/2;

    // Initial width and height are half of the size of the client area
    _iWidth = cxClient/2;
    _iHeight = cyClient/2;

    // Initial scaling factor is 1.0 (no scaling)
    _dScalingFactor = 1.0;

    // Initial rotation angle is 0.0 (no rotation)
    _dRotationAngle = 0.0; 

    _bDrawDiagonals = false; // no drawing of the diagonals


    _iColorIndex = 0; // set initial color to black
}

// This function will be called by the main app whenever WM_PAINT message is 
// received. It is responsible to redraw the rectangle. Here we calculate the 
// positon of the rectangle corners.
// in:
//      hdc - handle to device context
void CDrawingObject::Paint(HDC hdc)
{
    // create new pen with a width 1
    HPEN hPen = CreatePen(PS_SOLID,1,s_colors[_iColorIndex]);

    // select new pen for drawing
    HGDIOBJ hPenOld = SelectObject(hdc,hPen);

    // first create a polyline that describes the rectangle scaled by the 
    // scaling factor
    POINT ptRect[5];    

    // upper left cofner
    ptRect[0].x = -(LONG)(_dScalingFactor * _iWidth/2);
    ptRect[0].y = -(LONG)(_dScalingFactor * _iHeight/2);

    // upper right corner
    ptRect[1].x = (LONG)(_dScalingFactor * _iWidth/2);
    ptRect[1].y = ptRect[0].y;

    // lower right corner
    ptRect[2].x = ptRect[1].x;
    ptRect[2].y = (LONG)(_dScalingFactor * _iHeight/2);

    // lower left corner
    ptRect[3].x = ptRect[0].x;
    ptRect[3].y = ptRect[2].y;
    
    // upper left corner, we are closing the rectangle
    ptRect[4] = ptRect[0];

    // now we should rotate the rectangle
    double dCos = cos(_dRotationAngle);
    double dSin = sin(_dRotationAngle);

    for (int i = 0; i < 5; i++)
    {
        LONG lDX = ptRect[i].x;
        LONG lDY = ptRect[i].y;

        ptRect[i].x = (LONG)(lDX*dCos + lDY*dSin);
        ptRect[i].y = (LONG)(lDY*dCos - lDX*dSin);
    }

    // finally we should translate the rectangle
    for (int i = 0; i < 5; i++)
    {
        ptRect[i].x += _iCx;
        ptRect[i].y += _iCy;
    }

    Polyline(hdc, ptRect, 5);

    if (_bDrawDiagonals)
    {
        // draw diagonals
        MoveToEx(hdc, ptRect[0].x, ptRect[0].y, NULL);
        LineTo(hdc, ptRect[2].x, ptRect[2].y);
        MoveToEx(hdc, ptRect[1].x, ptRect[1].y, NULL);
        LineTo(hdc, ptRect[3].x, ptRect[3].y);
    }

    // select old pen
    SelectObject(hdc, hPenOld);

    // destroy new pen
    DeleteObject(hPen);
}

// This function is responsible for translating the center of the rectangle.
// in:
//      ldx - shift of the x-coordinate
//      ldy - shift of the y-coordinate
void CDrawingObject::Move(LONG ldx, LONG ldy)
{
    _iCx += ldx;
    _iCy += ldy;
}

// This function zooms the rectanlge in/out
// in:
//      dZoomFactor - scaling factor of the zoom
//      iZx         - x coordinate of the zoom center
//      iZy         - y coordinate of the zoom center
void CDrawingObject::Zoom(const double dZoomFactor, const LONG iZx, const LONG iZy)
{
    _iCx = ROUND_DOUBLE_TO_LONG(iZx * (1.0-dZoomFactor) + _iCx * dZoomFactor);
    _iCy = ROUND_DOUBLE_TO_LONG(iZy * (1.0-dZoomFactor) + _iCy * dZoomFactor);

    _dScalingFactor *= dZoomFactor;
}

// This function rotates the rectangle
// in:
//      dAngle  - angle of rotation in radians
//      iOx     - x-coordinate of the rotation center
//      iOy     - y-coordinate of the rotation center
//
// Note that during the rotation gesture the user defines center of rotation. 
// This is going to move the center of the rectangle too. That is why we have to 
// recalculate new center of the rectangle.
void CDrawingObject::Rotate(const double dAngle, const LONG iOx, const LONG iOy)
{
    // rotate center point around point O

    double dCos = cos(dAngle);
    double dSin = sin(dAngle);

    LONG lDX = _iCx - iOx;
    LONG lDY = _iCy - iOy;

    _iCx = iOx + ROUND_DOUBLE_TO_LONG(lDX*dCos + lDY*dSin);
    _iCy = iOy + ROUND_DOUBLE_TO_LONG(lDY*dCos - lDX*dSin);

    _dRotationAngle += dAngle;
}

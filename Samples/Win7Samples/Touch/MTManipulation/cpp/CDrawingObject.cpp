// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//
// This class handles manipulation of drawing object (rectangle) as a reaction
// to multi touch gestures
//
// Initially, we define rectangle to be in the center of the client area. So whenever
// user resize window the rectangle is placed in the middle of client window with
// a width set to half of width of client window and height set to half of the heights of
// client window. Rectangle is placed in the center of the client area. 
//
// By using the fingers the user can zoom in, zoom out, move or rotate the rectangle. 
//

#include "CDrawingObject.h"

// Default constructor
CDrawingObject::CDrawingObject()
{
    // main window application is responsible to invoke ResetObject function to initialize variables.
    // It shoudl be done whenever main window gets WM_SIZE message.
}

// Destructor
CDrawingObject::~CDrawingObject()
{
}

// This function resets rectangle object information and it's called by main app
// whenever a user resizes client area
// in:
//      cxClient - new width of client window
//      cyClient - new heights size of client window
void CDrawingObject::ResetObject(const int cxClient, const int cyClient)
{
    // Initial positon of center point is the middle point of client window
    _ptCenter.x = cxClient/2;
    _ptCenter.y = cyClient/2;

    // Initial width and height are half a size of client window
    _szRect.cx = cxClient/2;
    _szRect.cy = cyClient/2;

    // Initial scaling factor is 1.0 (no scaling)
    _dScalingFactor = 1.0;

    // Initial rotation angle is 0.0 (no rotation)
    _dRotationAngle = 0.0; 
}

// This function will be called by the main app whenever WM_PAINT message is 
// received. It is responsible to redraw the rectangle. Here we calculate the 
// positon of the rectangle corners.
// in:
//      hdc - handle to device context
void CDrawingObject::Paint(HDC hdc)
{
    // create new blue pen with a width 1
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));

    // select new pen for drawing
    HGDIOBJ hPenOld = SelectObject(hdc, hPen);

    // first create a polyline that describes the rectangle scaled by the 
    // scaling factor
    POINT ptRect[5];    

    // upper left cofner
    ptRect[0].x = -(LONG)(_dScalingFactor * _szRect.cx/2);
    ptRect[0].y = -(LONG)(_dScalingFactor * _szRect.cy/2);

    // upper right corner
    ptRect[1].x = -ptRect[0].x;
    ptRect[1].y = ptRect[0].y;

    // lower right corner
    ptRect[2].x = ptRect[1].x;
    ptRect[2].y = -ptRect[0].y;

    // lower left corner
    ptRect[3].x = ptRect[0].x;
    ptRect[3].y = ptRect[2].y;
    
    // upper left corner, we are closing the rectangle
    ptRect[4] = ptRect[0];

    // now we should rotate and translate the rectangle
    double dCos = cos(_dRotationAngle);
    double dSin = sin(_dRotationAngle);

    for (int i = 0; i < 5; i++)
    {
        LONG lDX = ptRect[i].x;
        LONG lDY = ptRect[i].y;

        // rotation
        ptRect[i].x = (LONG)(lDX*dCos + lDY*dSin);
        ptRect[i].y = (LONG)(lDY*dCos - lDX*dSin);

        // translation
        ptRect[i].x += _ptCenter.x;
        ptRect[i].y += _ptCenter.y;
    }

    Polyline(hdc, ptRect,5);

    // draw diagonals
    MoveToEx(hdc, ptRect[0].x, ptRect[0].y, NULL);
    LineTo(hdc, ptRect[2].x, ptRect[2].y);
    MoveToEx(hdc, ptRect[1].x, ptRect[1].y, NULL);
    LineTo(hdc, ptRect[3].x, ptRect[3].y);

    // select old pen
    SelectObject(hdc, hPenOld);

    // destroy new pen
    DeleteObject(hPen);
}

// This function is responsible for manipulation of the rectangle.
// It is called from CManipulationEventSink class.
// in:
//      translationDeltaX - shift of the x-coordinate (1/100 of pixel units)
//      translationDeltaY - shift of the y-coordinate (1/100 of pixel units)
//             scaleDelta - scale factor (zoom in/out)
//          rotationDelta - rotation angle in radians
void CDrawingObject::ApplyManipulationDelta(
    const FLOAT translationDeltaX,
    const FLOAT translationDeltaY,
    const FLOAT scaleDelta,
    const FLOAT rotationDelta)
{
    _ptCenter.x += (LONG) (translationDeltaX / 100.0);
    _ptCenter.y += (LONG) (translationDeltaY / 100.0);

    _dScalingFactor *= scaleDelta;

    _dRotationAngle -= rotationDelta; // we are substracting because Y-axis is down
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef DRAWINGOBJECT_H
#define DRAWINGOBJECT_H

#include "D2DDriver.h"
#include <windows.h>

class CDrawingObject {
public:
    enum DrawingColor {Blue, Orange, Green, Red};

    CDrawingObject(HWND hwnd, CD2DDriver* d2dDriver);
    ~CDrawingObject();

    VOID ResetState(const FLOAT startX, const FLOAT startY, 
        const int ixClient, const int iyClient, 
        const int iScaledWidth, const int iScaledHeight,
        const DrawingColor colorChoice);
    VOID Paint();
    VOID Translate(FLOAT fdx, FLOAT fdy, BOOL bInertia);
    VOID Scale(const FLOAT fFactor);
    VOID Rotate(const FLOAT fAngle);
    BOOL InRegion(LONG lX, LONG lY);
    VOID RestoreRealPosition();

    // Public set method
    VOID SetManipulationOrigin(FLOAT x, FLOAT y);

    // Public get methods
    FLOAT GetPosY();
    FLOAT GetPosX();
    FLOAT GetWidth();
    FLOAT GetHeight();
    FLOAT GetCenterX();
    FLOAT GetCenterY();

private:
    VOID RotateVector(FLOAT* vector, FLOAT* tVector, FLOAT fAngle);
    VOID ComputeElasticPoint(FLOAT fIPt, FLOAT* fRPt, int iDimension);
    VOID UpdateBorders(); 
    VOID EnsureVisible();

    HWND m_hWnd;

    CD2DDriver* m_d2dDriver;

    // D2D brushes
    ID2D1HwndRenderTargetPtr	m_spRT;
    ID2D1LinearGradientBrushPtr m_pGlBrush;
    ID2D1LinearGradientBrushPtr m_currBrush;
    
    ID2D1RoundedRectangleGeometryPtr m_spRoundedRectGeometry;
    
    // Keeps the last matrix used to perform the rotate transform
    D2D_MATRIX_3X2_F m_lastMatrix;

    // Coordinates of where manipulation started
    FLOAT m_fOX;
    FLOAT m_fOY;

    // Internal top, left coordinates of object (Real inertia values)
    FLOAT m_fXI;
    FLOAT m_fYI;
    
    // Rendered top, left coordinates of object
    FLOAT m_fXR;
    FLOAT m_fYR;
    
    // Width and height of the object
    FLOAT m_fWidth;
    FLOAT m_fHeight;

    // Scaling factor applied to the object
    FLOAT m_fFactor;

    // Cumulative angular rotation applied to the object
    FLOAT m_fAngleCumulative;

    // Current angular rotation applied to object
    FLOAT m_fAngleApplied;

    // Delta x, y values
    FLOAT m_fdX;
    FLOAT m_fdY;

    // Right and bottom borders relative to the object's size
    int m_iBorderX;
    int m_iBorderY;

    // Client width and height
    int m_iCWidth;
    int m_iCHeight;
};

#endif
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "DrawingObject.h"
#include <math.h>

#define INITIAL_OBJ_WIDTH	200
#define INITIAL_OBJ_HEIGHT	200
#define DEFAULT_DIRECTION	0

CDrawingObject::CDrawingObject(HWND hwnd, CD2DDriver* d2dDriver) :
    m_hWnd(hwnd),
    m_d2dDriver(d2dDriver)
{
    // Get the render target for drawing to
    m_spRT = m_d2dDriver->GetRenderTarget();
}

CDrawingObject::~CDrawingObject()
{
}

// Sets the default position, dimensions and color for the drawing object
VOID CDrawingObject::ResetState(const FLOAT startX, const FLOAT startY, 
                                const int ixClient, const int iyClient,
                                const int iScaledWidth, const int iScaledHeight,
                                const DrawingColor colorChoice)
{
    // Set width and height of the client area
    // must adjust for dpi aware
    m_iCWidth = iScaledWidth;
    m_iCHeight = iScaledHeight;

    // Initialize width height of object
    m_fWidth   = INITIAL_OBJ_WIDTH;
    m_fHeight  = INITIAL_OBJ_HEIGHT;

    // Set outer elastic border
    UpdateBorders();

    // Set the top, left starting position

    // Set cooredinates given by processor
    m_fXI = startX;
    m_fYI = startY;

    // Set coordinates used for rendering
    m_fXR = startX;
    m_fYR = startY;

    // Set touch origin to 0
    m_fOX = 0.0f;
    m_fOY = 0.0f;

    // Initialize scaling factor
    m_fFactor = 1.0f;

    // Initialize angle
    m_fAngleCumulative = 0.0f;

    if(m_spRT)
    {
        HRESULT hr;
        D2D1_SIZE_U  size;
        size.width	= ixClient;
        size.height = iyClient;
        hr= m_spRT->Resize(size);

        if (FAILED(hr))
        {
            m_d2dDriver->DiscardDeviceResources();
            InvalidateRect(m_hWnd, NULL, FALSE);
        }
    }

    // Determines what brush to use for drawing this object and 
    // gets the brush from the D2DDriver class

    switch (colorChoice){
        case Blue:
            m_currBrush = m_d2dDriver->get_GradBrush(CD2DDriver::GRB_Blue);
            break;
        case Orange:
            m_currBrush = m_d2dDriver->get_GradBrush(CD2DDriver::GRB_Orange);
            break;
        case Green:
            m_currBrush = m_d2dDriver->get_GradBrush(CD2DDriver::GRB_Green);
            break;
        case Red:
            m_currBrush = m_d2dDriver->get_GradBrush(CD2DDriver::GRB_Red);
            break;
        default:
            m_currBrush = m_d2dDriver->get_GradBrush(CD2DDriver::GRB_Blue);
    }
}

VOID CDrawingObject::Paint()
{
    if(!(m_spRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        FLOAT fGlOffset = 2.5f;
    
        // Setup our matrices for performing transforms

        D2D_MATRIX_3X2_F rotateMatrix;
        D2D_MATRIX_3X2_F identityMatrix;
        identityMatrix = D2D1::Matrix3x2F::Identity();
        
        // Apply rotate transform
        
        rotateMatrix = D2D1::Matrix3x2F::Rotation(
            m_fAngleCumulative, 
            D2D1::Point2F(
                m_fXR + m_fWidth/2.0f, 
                m_fYR + m_fHeight/2.0f 
            )
        );
        
        m_spRT->SetTransform(&rotateMatrix);

        // Store the rotate matrix to be used in hit testing
        m_lastMatrix = rotateMatrix;

        // Get glossy brush
        m_pGlBrush = m_d2dDriver->get_GradBrush(CD2DDriver::GRB_Glossy);
        
        // Set positions of gradients based on the new coordinates of the objecs
        
        m_currBrush->SetStartPoint(
            D2D1::Point2F(
                m_fXR, 
                m_fYR
            )
        );
        
        m_currBrush->SetEndPoint(
            D2D1::Point2F(
                m_fXR, 
                m_fYR + m_fHeight
            )
        );
        
        m_pGlBrush->SetStartPoint(
            D2D1::Point2F(
                m_fXR, 
                m_fYR
            )
        );
        
        m_pGlBrush->SetEndPoint(
            D2D1::Point2F(
                m_fXR + m_fWidth/15.0f, 
                m_fYR + m_fHeight/2.0f
            )
        );

        // Create rectangle to draw

        D2D1_RECT_F rectangle = D2D1::RectF(
            m_fXR,
            m_fYR,
            m_fXR+m_fWidth,
            m_fYR+m_fHeight
        );
    
        D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
            rectangle,
            10.0f, 10.0f
        );

        // Create glossy effect

        D2D1_RECT_F glossyRect = D2D1::RectF(
            m_fXR+fGlOffset,
            m_fYR+fGlOffset,
            m_fXR+m_fWidth-fGlOffset,
            m_fYR+m_fHeight/2.0f
        );
        
        D2D1_ROUNDED_RECT glossyRoundedRect = D2D1::RoundedRect(
            glossyRect, 
            10.0f, 
            10.0f
        );

        // D2D requires that a geometry is created for the rectangle
        m_d2dDriver->CreateGeometryRoundedRect(
            roundedRect, 
            &m_spRoundedRectGeometry
        );

        // Fill the geometry that was created
        m_spRT->FillGeometry(
            m_spRoundedRectGeometry, 
            m_currBrush
        );

        // Draw glossy effect
        m_spRT->FillRoundedRectangle(
            &glossyRoundedRect,
            m_pGlBrush
        );

        // Restore our transform to nothing
        m_spRT->SetTransform(&identityMatrix);
    }
}

VOID CDrawingObject::Translate(FLOAT fdx, FLOAT fdy, BOOL bInertia)
{
    m_fdX = fdx;
    m_fdY = fdy;

    FLOAT fOffset[2];
    fOffset[0] = m_fOX - m_fdX;
    fOffset[1] = m_fOY - m_fdY;

    // Translate based on the offset caused by rotating 
    // and scaling in order to vary rotational behavior depending 
    // on where the manipulation started
    
    if(m_fAngleApplied != 0.0f)
    {
        FLOAT v1[2];
        v1[0] = GetCenterX() - fOffset[0];
        v1[1] = GetCenterY() - fOffset[1];

        FLOAT v2[2];
        RotateVector(v1, v2, m_fAngleApplied);

        m_fdX += v2[0] - v1[0];
        m_fdY += v2[1] - v1[1];
    }

    if(m_fFactor != 1.0f)
    {
        FLOAT v1[2];
        v1[0] = GetCenterX() - fOffset[0];
        v1[1] = GetCenterY() - fOffset[1];

        FLOAT v2[2];
        v2[0] = v1[0] * m_fFactor;
        v2[1] = v1[1] * m_fFactor;
        
        m_fdX += v2[0] - v1[0];
        m_fdY += v2[1] - v1[1];
    }

    m_fXI += m_fdX;
    m_fYI += m_fdY;

    // The following code handles the effect for 
    // bouncing off the edge of the screen.  It takes
    // the x,y coordinates computed by the inertia processor
    // and calculates the appropriate render coordinates
    // in order to achieve the effect.

    if (bInertia)
    {
        ComputeElasticPoint(m_fXI, &m_fXR, m_iBorderX);
        ComputeElasticPoint(m_fYI, &m_fYR, m_iBorderY);
    }
    else
    {
        m_fXR = m_fXI;
        m_fYR = m_fYI;

        // Make sure it stays on screen
        EnsureVisible();
    }
}

VOID CDrawingObject::EnsureVisible()
{
    m_fXR = max(0,min(m_fXI, (FLOAT)m_iCWidth-m_fWidth));
    m_fYR = max(0,min(m_fYI, (FLOAT)m_iCHeight-m_fHeight));
    RestoreRealPosition();
}

VOID CDrawingObject::Scale(const FLOAT dFactor)
{
    m_fFactor = dFactor;

    FLOAT scaledW = (dFactor-1) * m_fWidth;
    FLOAT scaledH = (dFactor-1) * m_fHeight;
    FLOAT scaledX = scaledW/2.0f;
    FLOAT scaledY = scaledH/2.0f;
    
    m_fXI -= scaledX;
    m_fYI -= scaledY;

    m_fWidth  += scaledW;
    m_fHeight += scaledH;
    
    // Only limit scaling in the case that the factor is not 1.0

    if(dFactor != 1.0f)
    {
        m_fXI = max(0, m_fXI);
        m_fYI = max(0, m_fYI);

        m_fWidth = min(min(m_iCWidth, m_iCHeight), m_fWidth);
        m_fHeight = min(min(m_iCWidth, m_iCHeight), m_fHeight);
    }

    // Readjust borders for the objects new size
    UpdateBorders();
}

VOID CDrawingObject::Rotate(const FLOAT fAngle)
{
    m_fAngleCumulative += fAngle;
    m_fAngleApplied = fAngle;
}

VOID CDrawingObject::SetManipulationOrigin(FLOAT x, FLOAT y)
{
    m_fOX = x;
    m_fOY = y;
}

// Helper method that rotates a vector using basic math transforms
VOID CDrawingObject::RotateVector(FLOAT *vector, FLOAT *tVector, FLOAT fAngle)
{
    FLOAT fAngleRads = fAngle / 180.0f * 3.14159f;
    FLOAT fSin = sin(fAngleRads);
    FLOAT fCos = cos(fAngleRads);

    FLOAT fNewX = (vector[0]*fCos) - (vector[1]*fSin);
    FLOAT fNewY = (vector[0]*fSin) + (vector[1]*fCos);

    tVector[0] = fNewX;
    tVector[1] = fNewY;
}

FLOAT CDrawingObject::GetPosY()
{
    return m_fYI;
}

FLOAT CDrawingObject::GetPosX()
{
    return m_fXI;
}

FLOAT CDrawingObject::GetWidth()
{
    return m_fWidth;
}

FLOAT CDrawingObject::GetHeight()
{
    return m_fHeight;
}

FLOAT CDrawingObject::GetCenterX()
{
    return m_fXI + m_fWidth/2.0f;
}

FLOAT CDrawingObject::GetCenterY()
{
    return m_fYI + m_fHeight/2.0f;
}

// Hit testing method handled with Direct2D
BOOL CDrawingObject::InRegion(LONG x, LONG y)
{
    BOOL b = FALSE;
    
    m_spRoundedRectGeometry->FillContainsPoint(
        D2D1::Point2F((FLOAT)x, (FLOAT)y),
        &m_lastMatrix, 
        &b
    );
    return b;
}

// Sets the internal coordinates to render coordinates
VOID CDrawingObject::RestoreRealPosition()
{
    m_fXI = m_fXR;
    m_fYI = m_fYR;
}

VOID CDrawingObject::UpdateBorders()
{
    m_iBorderX = m_iCWidth  - (int)m_fWidth;
    m_iBorderY = m_iCHeight - (int)m_fHeight;
}

// Computes the the elastic point and sets the render coordinates
VOID CDrawingObject::ComputeElasticPoint(FLOAT fIPt, FLOAT *fRPt, int iBSize)
{
    // If the border size is 0 then do not attempt
    // to calculate the render point for elasticity
    if(iBSize == 0)
        return;

    // Calculate render coordinate for elastic border effect

    // Divide the cumulative translation vector by the max border size
    int q = (int)abs(fIPt) / iBSize;
    int direction = q % 2;
    
    // Calculate the remainder this is the new render coordinate
    FLOAT newPt = abs(fIPt) - (FLOAT)(iBSize*q);
    
    if (direction == DEFAULT_DIRECTION)
    {
        *fRPt = newPt;
    }
    else
    {
        *fRPt = (FLOAT)iBSize - newPt;
    }
}

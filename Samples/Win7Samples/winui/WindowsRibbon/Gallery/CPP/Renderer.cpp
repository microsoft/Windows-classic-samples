// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Renderer.h"
#include <propvarutil.h>

#define SHAPE_MARGIN    5

COLORREF CRenderer::s_aColors[3] = {
                        RGB(255, 0, 0), // Red 
                        RGB(0, 255, 0), // Green
                        RGB(0, 0, 255), // Blue
                       };

void CRenderer::Initialize(HWND hWnd)
{
    m_hWnd = hWnd;
}

void CRenderer::GetRenderParam(__in RenderParam *pParameter)
{
    if (pParameter != NULL)
    {
        *pParameter = m_param;
    }
}

void CRenderer::UpdateRenderParam(RenderParam parameter)
{
    m_param = parameter;
    InvalidateRect(m_hWnd, NULL, TRUE);
}

void CRenderer::DrawShapes(HDC hdc, RECT& rect)
{
    SHAPE_TYPE  eShapeType = m_param.eShapeType;

    int nCount = m_param.eViewLayout + 1;

    SIZE size;
    size.cx = rect.right - rect.left;
    size.cy = rect.bottom - rect.top;

    HBRUSH hBrush = CreateSolidBrush(s_aColors[m_param.eShapeColor]);

    HGDIOBJ hOldBrush = ::SelectObject(hdc, hBrush);
    
    int nPenStyle;
    int nBorderSize = m_param.uBorderSize;
    switch (m_param.eBorderStyle)
    {
        case DASH:
            nPenStyle = PS_DASH;
            nBorderSize = 1;// Dash pen only works with a width of 1
            break;
        case SOLID:
            nPenStyle = PS_SOLID;
            break;
        default:
            nPenStyle = PS_NULL;
            break;
    }

    HPEN hPen = CreatePen(nPenStyle, nBorderSize, RGB(0, 0, 0));
    HGDIOBJ hOldPen = ::SelectObject(hdc, hPen);

    // Pick up the shorter length.
    int nLength = ( size.cx < size.cy ? size.cx : size.cy) / nCount;

    // Take margin into count
    nLength -= SHAPE_MARGIN;   

    if (nLength > 0)
    {
        // Now scale the length based on the parameter
        nLength = nLength * (m_param.eShapeSize + 1) / 3;
    }
    else
    {
        nLength = 0;
    }

    for (int i = 0; i < nCount ; i++)
    {
        for (int j = 0; j < nCount ; j++)
        {
            POINT ptCenter;
            ptCenter.y = rect.top + (2 * i + 1) * size.cy / (2 * nCount);
            ptCenter.x = rect.left + (2 * j + 1) * size.cx / (2 * nCount);

            switch (eShapeType)
            {
                case RECTANGLE:
                case ROUNDED_RECTANGLE:
                    DrawRectangle(hdc, ptCenter, nLength, eShapeType == ROUNDED_RECTANGLE);
                    break;
                case ELLIPSE:
                    DrawEllipse(hdc, ptCenter, nLength);
                    break;
                case DIAMOND:
                    DrawDiamond(hdc, ptCenter, nLength);
                    break;
            }
        }
    }

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);

    DeleteObject(hPen);
    DeleteObject(hBrush);
}


void CRenderer::DrawRectangle(HDC hdc, POINT& ptCenter, int nBoundingBoxLength, BOOL fIsRounded)
{
    if (fIsRounded)
    {
        int nRoundLength = nBoundingBoxLength / 10;
        if (nRoundLength < 2)
        {
            nRoundLength = 2;
        }

        RoundRect(hdc, ptCenter.x - nBoundingBoxLength / 2, ptCenter.y - nBoundingBoxLength / 2,
                    ptCenter.x + nBoundingBoxLength / 2, ptCenter.y + nBoundingBoxLength / 2, 
                    nRoundLength, nRoundLength);
    }
    else
    {
        Rectangle(hdc, ptCenter.x - nBoundingBoxLength / 2, ptCenter.y - nBoundingBoxLength / 2,
                    ptCenter.x + nBoundingBoxLength / 2, ptCenter.y + nBoundingBoxLength / 2);
    }
}

void CRenderer::DrawEllipse(HDC hdc, POINT& ptCenter, int nBoundingBoxLength)
{
    Ellipse(hdc, ptCenter.x - nBoundingBoxLength / 2, ptCenter.y - nBoundingBoxLength / 2,
                ptCenter.x + nBoundingBoxLength / 2, ptCenter.y + nBoundingBoxLength / 2);
}

void CRenderer::DrawDiamond(HDC hdc, POINT& ptCenter, int nBoundingBoxLength)
{
    POINT pt[4];
    int nLength = nBoundingBoxLength / 2;

    pt[0].x = ptCenter.x;
    pt[0].y = ptCenter.y - nLength;

    pt[1].x = ptCenter.x - nLength;
    pt[1].y = ptCenter.y;

    pt[2].x = ptCenter.x;
    pt[2].y = ptCenter.y + nLength;

    pt[3].x = ptCenter.x + nLength;
    pt[3].y = ptCenter.y;

    Polygon(hdc, pt, _countof(pt));
}

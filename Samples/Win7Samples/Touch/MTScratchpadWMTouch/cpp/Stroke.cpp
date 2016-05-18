// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// Stroke.cpp
//
// Implementation of helper classes for stroke storage, CStroke and
// CStrokeCollection.

// Windows header files
#include <windows.h>

// Application header files
#include "stroke.h"

// CStroke constructor.
CStroke::CStroke()
:   m_clr(RGB(0,0,0)),
    m_id(0)
{
}

// Draws a complete stroke onto given device context.
// in:
//      hDC     handle to device context
void CStroke::Draw(HDC hDC) const
{
    if (m_nCount < 2)
    {
        return;
    }

    HPEN hPen = CreatePen(PS_SOLID, 3, m_clr);
    HGDIOBJ hOldPen = SelectObject(hDC, hPen);
    Polyline(hDC, m_arrData, m_nCount);
    SelectObject(hDC, hOldPen);
    DeleteObject(hPen);
}

// Draw last segment of the stroke to the device context.
// in:
//      hDC     handle to device context
void CStroke::DrawLast(HDC hDC) const
{
    if (m_nCount < 2)
    {
        return;
    }

    HPEN hPen = CreatePen(PS_SOLID, 3, m_clr);
    HGDIOBJ hOldPen = SelectObject(hDC, hPen);
    MoveToEx(hDC, m_arrData[m_nCount-2].x, m_arrData[m_nCount-2].y, NULL);
    LineTo(hDC, m_arrData[m_nCount-1].x, m_arrData[m_nCount-1].y);
    SelectObject(hDC, hOldPen);
    DeleteObject(hPen);
}

///////////////////////////////////////////////////////////////////////////////

// Searches the collection for the given ID.
// in:
//      id      stroke ID
// returns:
//      stroke index in the array, or -1 if not found
int CStrokeCollection::FindStrokeById(int id) const
{
    for (int i = 0; i < m_nCount; ++i)
    {
        if (m_arrData[i]->GetId() == id)
        {
            return i;
        }
    }

    return -1;
}

// Draw the complete stroke collection onto given device context.
// in:
//      hDC     handle to device context
void CStrokeCollection::Draw(HDC hDC) const
{
    for (int i = 0; i < m_nCount; ++i)
    {
        m_arrData[i]->Draw(hDC);
    }
}

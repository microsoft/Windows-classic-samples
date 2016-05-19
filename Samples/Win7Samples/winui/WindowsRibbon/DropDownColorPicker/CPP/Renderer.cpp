// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Renderer.h"

void CRenderer::Initialize(HWND hwnd)
{
    m_hWnd = hwnd;
    m_highlightIndex = 0;
}

void CRenderer::Execute(UI_EXECUTIONVERB verb, ColorProperty color)
{
    m_colors[m_highlightIndex] = color;

    if (verb == UI_EXECUTIONVERB_CANCELPREVIEW)
    {
        m_colors[m_highlightIndex] = m_storedColor; 
    }

    if (verb == UI_EXECUTIONVERB_EXECUTE)
    {        
        if (++m_highlightIndex >= DDCP_WIDTH * DDCP_HEIGHT)
        {
            // Rewind index if out of bound.
            m_highlightIndex = 0;
        }
        m_storedColor = m_colors[m_highlightIndex];        
    }

    InvalidateRect(m_hWnd, NULL, FALSE);
}

void CRenderer::ClearColorGrid()
{
    for (int i = 0; i < DDCP_WIDTH*DDCP_HEIGHT; i++)
    {
        m_colors[i].type = UI_SWATCHCOLORTYPE_NOCOLOR;
    }

    // Reset index.
    m_highlightIndex = 0;

    InvalidateRect(m_hWnd, NULL, FALSE); 
}

COLORREF CRenderer::GetColor(int index)
{
    switch (m_colors[index].type)
    {
    case UI_SWATCHCOLORTYPE_NOCOLOR:        
        return RGB(255, 255, 255);  
    case UI_SWATCHCOLORTYPE_RGB:         
        return m_colors[index].color;
    case UI_SWATCHCOLORTYPE_AUTOMATIC:        
        return GetSysColor(COLOR_WINDOWTEXT);
    }
    return 0;
}

void CRenderer::Draw(HDC hdc, UINT ribbonHeight)
{    
    // Create yellow highlight pen.
    HPEN hPen = CreatePen(PS_INSIDEFRAME, 5, RGB(255,255,0));
    RECT canvas;
    GetClientRect(m_hWnd, &canvas);
    canvas.top += ribbonHeight;
    int cellHeight = (canvas.bottom - canvas.top) / DDCP_HEIGHT;
    int cellWidth = (canvas.right - canvas.left) / DDCP_WIDTH;

    for (int i = 0; i < DDCP_WIDTH * DDCP_HEIGHT; i++)
    {
        // Create brush from current color and select it.
        HBRUSH hBrush = CreateBrushFromColorProp(&m_colors[i]);
        HGDIOBJ hOldBrush = SelectObject(hdc, hBrush); 
        HGDIOBJ hOldPen = NULL;
        int col = i % DDCP_WIDTH;
        int row = i / DDCP_WIDTH;

        if (i == m_highlightIndex)
        {
            // Select highlight color pen.
            hOldPen = SelectObject(hdc, hPen); 
        }

        Rectangle(hdc, canvas.left+col*cellWidth, canvas.top+row*cellHeight, canvas.left+(col+1)*cellWidth, canvas.top+(row+1)*cellHeight);

        if (hOldPen)
        {
            SelectObject(hdc, hOldPen);
        }

        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);        
    }
    DeleteObject(hPen);
}

HBRUSH CRenderer::CreateBrushFromColorProp(ColorProperty* colorProp)
{    
    switch (colorProp->type)
    {
    case UI_SWATCHCOLORTYPE_NOCOLOR:
        // Set brush to white (for no color).
        return CreateSolidBrush(RGB(255, 255, 255));
    case UI_SWATCHCOLORTYPE_RGB:
        // Set brush to stored RGB color.
        return CreateSolidBrush(colorProp->color); 
    case UI_SWATCHCOLORTYPE_AUTOMATIC:
        // Set brush to system color.
        return CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
    }

    return NULL;
}
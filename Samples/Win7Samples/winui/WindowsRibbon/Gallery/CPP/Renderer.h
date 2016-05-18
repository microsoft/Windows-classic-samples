// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once
#include <windows.h>

// Enums used to define the parameters for the renderer.
typedef enum SHAPE_TYPE
{
    RECTANGLE           = 0,
    ELLIPSE             = 1,
    ROUNDED_RECTANGLE   = 2,
    DIAMOND             = 3,
} SHAPE_TYPE;

typedef enum SHAPE_COLOR
{
    RED   = 0,
    GREEN = 1,
    BLUE  = 2,
} SHAPE_COLOR;

typedef enum SHAPE_SIZE
{
    SMALL  = 0,
    MEDIUM = 1,
    LARGE  = 2,
} SHAPE_SIZE;

typedef enum BORDER_STYLE
{
    NONE  = 0,
    SOLID = 1,
    DASH  = 2,
} BORDER_STYLE;

typedef enum VIEW_LAYOUT
{
    LAYOUT1 = 0,
    LAYOUT2 = 1,
    LAYOUT3 = 2,
} VIEW_LAYOUT;

// Parameters to the renderer- updated by the controls in the ribbon.
typedef struct RenderParam
{
    SHAPE_TYPE      eShapeType;
    SHAPE_COLOR     eShapeColor;
    SHAPE_SIZE      eShapeSize;
    BORDER_STYLE    eBorderStyle;
    UINT            uBorderSize;
    VIEW_LAYOUT     eViewLayout;
} RenderParam;

// Class that draws the shapes according to the parameters specified through the ribbon.
class CRenderer
{
public:

    CRenderer::CRenderer()
    : m_hWnd(NULL)
    {
        // Initialize m_param to default.
        m_param.eShapeType = RECTANGLE;
        m_param.eShapeColor = RED;
        m_param.eShapeSize = SMALL;
        m_param.eBorderStyle = NONE;
        m_param.uBorderSize = 1;
        m_param.eViewLayout = LAYOUT1;
    }

    void Initialize(HWND hWnd);
    void GetRenderParam(__in RenderParam *pParameter);
    void UpdateRenderParam(RenderParam parameter);

    void DrawShapes(HDC hdc, RECT& rect);

private:

    void DrawRectangle(HDC hdc, POINT& ptCenter, int nBoundingBoxLength, BOOL fIsRounded);
    void DrawEllipse(HDC hdc, POINT& ptCenter, int nBoundingBoxLength);
    void DrawDiamond(HDC hdc, POINT& ptCenter, int nBoundingBoxLength);

    RenderParam     m_param;
    HWND            m_hWnd;

    static COLORREF s_aColors[3];
};


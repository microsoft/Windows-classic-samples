// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <assert.h>
#include <uiribbon.h>

#define DDCP_WIDTH 4
#define DDCP_HEIGHT 4

typedef struct ColorProperty
{
    UI_SWATCHCOLORTYPE type;    // Color type.
    COLORREF color;             // Color value.
} ColorProperty;

class CRenderer
{
public:
    void Initialize(HWND hwnd);
    void Execute(UI_EXECUTIONVERB verb, ColorProperty color);
    void ClearColorGrid();    
    COLORREF GetColor(int index);
    void Draw(HDC hdc, UINT ribbonHeight);
    
private:
    HBRUSH CreateBrushFromColorProp(ColorProperty* colorProp);

    HWND m_hWnd;                     // handle to window.
    int m_highlightIndex;            // highlight index.
    ColorProperty m_colors[DDCP_WIDTH * DDCP_HEIGHT]; // color array.
    ColorProperty m_storedColor; // preview color.
};
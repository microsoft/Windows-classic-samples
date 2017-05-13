//------------------------------------------------------------------------------
//
// File: ThemedButton.h
// Description: Implements a theme-aware button control.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//------------------------------------------------------------------------------

#pragma once

#include <uxtheme.h>
#include <vsstyle.h> // Defines PBS_xxxx

#pragma comment(lib, "UxTheme")
#pragma comment(lib, "Msimg32")     // links AlphaDrawBitmap()

const DWORD NUM_THEME_STATES = 6;



//------------------------------------------------------------------------------
// BitmapStripInfo
// Holds information about one image in a bitmap strip.
//------------------------------------------------------------------------------

struct BitmapStripInfo
{
    RECT    rc;            // Rectangle within the bitmap, containing the image.
    HBITMAP hBitmap;    // Handle to the bitmap.
};


//------------------------------------------------------------------------------
// BitmapStrip class
// Holds a bitmap that contains a strip of images.
//
// This class is used to manage the various images the themed button draws
// in different states.
//
// Possibly the standard image-list control could be used instead, but this 
// class is more straightforward for the requirements of this sample.
//------------------------------------------------------------------------------

class BitmapStrip
{
protected:
    HBITMAP m_hBitmap;      // Handle to the bitmap that contains the images.
    SIZE    m_size;         // Size of each image in the strip.
    UINT    m_cImages;      // Number of images. (m_size.cx * m_cImages <= width of bitmap)
public:
    BitmapStrip(): m_hBitmap(NULL), m_cImages(0)
    {
    }

    ~BitmapStrip()
    {
        if (m_hBitmap)
        {
            DeleteObject(m_hBitmap);
        }
    }

    BOOL    Load(int nID, int cImages);
    UINT    NumImages() const { return m_cImages; }
    BOOL    GetImage(UINT i, BitmapStripInfo *pInfo);
};


//------------------------------------------------------------------------------
// ThemedButton class
// Implements a theme-aware button.
//------------------------------------------------------------------------------

class ThemedButton //: public Control
{
private:

    HWND         m_hwnd;
    BitmapStrip  m_bitmap;
    DWORD        m_stateMap[ NUM_THEME_STATES ];  // Maps button states to images

    HTHEME       m_hTheme;   // Handle to the current theme.
    BOOL         m_bThemesActive;  // Are themes active?


    void         OpenTheme();
    void         CloseTheme();

    bool HasStyle(LONG style) const
    {
        return (GetWindowLong(m_hwnd, GWL_STYLE) & style) == style;
    }


public:

    ThemedButton();
    ~ThemedButton();

    void SetWindow(HWND hwnd) { m_hwnd = hwnd; }
    HWND Window() const { return m_hwnd; }

    void    ResetTheme();
    LRESULT Draw(const NMCUSTOMDRAW *pDraw);
    BOOL    LoadBitmap(int nID, int cImages);

    //  SetButtonImage: Set the image for a specified button state.
    //
    //  iState = PBS_xxx value, or (UINT)-1 to apply this image to all states.
    //  iImageIndex = index from the image strip
    BOOL    SetButtonImage(UINT iState, UINT iImageIndex);
};
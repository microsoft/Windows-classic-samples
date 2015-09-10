//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "pch.h"
#include "SampleDesktopWindow.h"
#include <ShellScalingApi.h>
#include <richedit.h>
#include <TextServ.h>
#include <Commctrl.h>
#include <Unknwn.h>

#define TWIPS_PER_PT    20
#define DEFAULT_FONT_PT 11
#define FONT_STEP       1.25

// <summary>
// This class implements a windowed rich edit control 
// available as a child window to the caller. 
// 
// The Rich edit control supports the following usages: 
//   - Incrementing and decrementing font size
//   - Responding to requested DPI changes. 
//   - Can be positioned either explicitly using coordinates, 
//       or relative to a rectangle. 
// </summary>
class CSampleRichEditWindow sealed
{
    public:
        CSampleRichEditWindow();
        ~CSampleRichEditWindow();

    public:
        // Initialize and position control.
        HRESULT Initialize(_In_ HWND hwndParent = NULL);
        HRESULT Position(_In_ UINT x, _In_ UINT y, _In_ UINT Width, _In_ UINT Height);
        HRESULT PositionRelativeToRect(_In_ RECT relativeRect);

        // Method to respond to DPI changes. 
        // In case of rich edit, the response to DPI changes is to scale text.
        HRESULT OnDPIChanged(_In_ float dpi);

        // Methods to format text within the control. 
        HRESULT IncrementFontSize();
        HRESULT DecrementFontSize();
        HRESULT SetFormatBold();
        HRESULT SetFormatItalic();
        HRESULT SetFormatUnderline();
        HRESULT SetFormatColor();
        HRESULT SetFormatBackgroundColor();
        HRESULT SetAlignment(_In_ WORD wAlignment);
        HRESULT IncrementIndent(_In_ INT lIncrement);
        HRESULT SetBulleted();
        HRESULT SetNumbered();
        void SetDefaultDPI(float dpi);
        HRESULT ResetDefaultContent();

    private:
        
        float default_dpi;

        typedef enum CHANGE_DIRECTION
        {
            CD_INCREMENT = 1,
            CD_DECREMENT = -1
        } CHANGE_DIRECTION;

        HRESULT GetResourceString(_In_ UINT stringID, _Outptr_ PCWSTR *stringText);
        HRESULT SetText(_In_ PCWSTR stringText);

        HRESULT SetDefaultFormat();
        HRESULT UpdateFontSize(_In_ CHANGE_DIRECTION direction);
        HRESULT ApplyZoomFactor();

        // Rich edit zoom factor is set using numerator/denominator pair.
        float m_currentDpi; 
        UINT m_numerator;
        UINT m_denominator;

        // Rich edit control requires MSFTedit.dll to be loaded.
        HMODULE m_hmodRichEdit;

        // Rich edit control is managed through window messages. 
        HWND m_hWnd;
};
//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "SampleRichEditControl.h"

CSampleRichEditWindow::CSampleRichEditWindow() :
    // Initialize scale factor to 100%
    m_numerator(100),
    m_denominator(100),

    // Initialize for loading rich edit module
    m_hmodRichEdit(nullptr),

    m_hWnd(nullptr)
{
}

CSampleRichEditWindow::~CSampleRichEditWindow()
{
    if (m_hWnd)
    {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }

    if (m_hmodRichEdit)
    {
        FreeLibrary(m_hmodRichEdit);
        m_hmodRichEdit = nullptr;
    }
}

// <summary>
// This method creates and initializes a child window for the Rich Edit control.
// Notes:
//    The rich edit window will be a child window to the hWnd passed in.
//    The window will be positioned using default child window values,
//    and rely on the caller to [re-]position the window as needed.
// </summary>
HRESULT
CSampleRichEditWindow::Initialize(
    _In_    HWND parentHWD
    )
{
    HRESULT hr = S_OK;
    BOOL fResult = FALSE;

    // Load MSFTEDIT.dll. This library is required for Rich Edit v4.1 or higher.
    m_hmodRichEdit = LoadLibrary(L"msftedit.dll");
    if (!m_hmodRichEdit)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }   

    // Create the rich edit window. 
    if (SUCCEEDED(hr))
    {
        m_hWnd = CreateWindowEx(WS_EX_LAYERED,          // Make this a layered window.
                                MSFTEDIT_CLASS,         // Associate with the rich edit class.

                                // Default text to use in window.
                                L"Enter text here.",

                                // Define as a visible child window.
                                // Also set flags to provide a multi-line rich edit experience.
                                WS_VISIBLE | WS_CHILD |
                                WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                                WS_VSCROLL | ES_WANTRETURN | ES_MULTILINE,

                                // Use default x, y, width and height as these will be set later.
                                CW_USEDEFAULT, CW_USEDEFAULT, 
                                CW_USEDEFAULT, CW_USEDEFAULT,

                                parentHWD,              // Set parent as requested.
                                NULL,                   // Do not attach to a menu.
                                GetModuleHandleW(NULL), // Associate to the calling application.
                                NULL                    // No additional structure needed to create this window.
                                ); 
    }
    if (!m_hWnd)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    // Set the rich edit control to be semi-transparent.
    if (SUCCEEDED(hr))
    {
        fResult = SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);
        if (!fResult)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Set this window to be on top of its siblings.
    if (SUCCEEDED(hr))
    {
        fResult = SetWindowPos(m_hWnd,
                               HWND_TOP,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                               SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
                               );
        if (!fResult)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Populate the rich edit control.
    if (SUCCEEDED(hr))
    {
        hr = ResetDefaultContent();
    }

    // Cleanup code.
    if (FAILED(hr))
    {
        if (m_hWnd)
        {
            fResult = DestroyWindow(m_hWnd);
            m_hWnd = nullptr;
        }
        if (m_hmodRichEdit)
        {
            fResult = FreeLibrary(m_hmodRichEdit);
            m_hmodRichEdit = nullptr;
        }
    }

    return hr;
}

// <summary>
// Position the rich edit control based on coordinates provided.
// </summary>
HRESULT
CSampleRichEditWindow::Position(
    _In_    UINT x,
    _In_    UINT y,
    _In_    UINT Width,
    _In_    UINT Height
    )
{
    HRESULT hr = S_OK;
    BOOL fResult;

    // Move the Rich Edit window to its location.
    fResult = MoveWindow(m_hWnd,     // Move myself
                         x,          // New position of the left side of the window
                         y,          // New position of the top side of the window
                         Width,      // New width
                         Height,     // New height
                         TRUE);     // Repaint the window
    if (!fResult)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

// <summary>
// Position the rich edit control relative to the dpi and RECT provided.
// </summary>
HRESULT
CSampleRichEditWindow::PositionRelativeToRect(
    _In_    RECT relativeRect
    )
{
    // Store target dimenions. 
    UINT childX;    // Position of the left side of the window.
    UINT childY;    // Position of the top side of the window.
    UINT childcX;   // Width of the window.
    UINT childcY;   // Height of the window.

    // Define offsets relative to the RECT provided.
    UINT parentHeight = 0;
    UINT offsetTop = 5;
    UINT offsetBottom = 5;
    UINT offsetRight = 10;
    UINT offsetLeft = 5;

    // Set the bounds for the child window based on offsets from RECT.
    parentHeight = relativeRect.bottom - relativeRect.top;
    childX = relativeRect.left + offsetLeft;
    childY = relativeRect.top + offsetTop;
    childcX = relativeRect.right - offsetRight;
    childcY = parentHeight - offsetTop - offsetBottom;

    return Position(childX, childY, childcX, childcY);
}

// <summary>
// Respond to an explicit DPI change, including updates to the zoom factor as needed.
// As the rich edit control uses a numerator/denominator pair that must not exceed 64,
// this class will provide numerator/denominator that will generate an implied fraction
// between 0 and 1.
// </summary>
HRESULT
CSampleRichEditWindow::OnDPIChanged(
    _In_    float dpi
    )
{
    HRESULT hr = S_OK;

    // Only take action if DPI is different from what control has stored.
    if (m_currentDpi != dpi)
    {
        m_numerator = (UINT) ((dpi / default_dpi) * 100.0F);
        hr = ApplyZoomFactor();
    }

    if (SUCCEEDED(hr))
    {
        // Update current DPI.
        m_currentDpi = dpi;
    }

    return hr;
}

void
CSampleRichEditWindow::SetDefaultDPI(float dpi)
{
    m_currentDpi = dpi;
    default_dpi = dpi;
}

HRESULT
CSampleRichEditWindow::IncrementFontSize()
{
    return UpdateFontSize(CD_INCREMENT);
}

HRESULT
CSampleRichEditWindow::DecrementFontSize()
{
    return UpdateFontSize(CD_DECREMENT);
}

// <summary>
// Enables caller to toggle the bold setting of the text in the control.
// In order to format the text in the control, we need to construct
// the parameters required to deliver the EM_SETCHARFORMAT message.
// </summary>
HRESULT
CSampleRichEditWindow::SetFormatBold()
{
    HRESULT hr = S_OK;
    LRESULT lr;
    CHARFORMAT2 charFormat;

    ZeroMemory(&charFormat, sizeof(charFormat));
    charFormat.cbSize = sizeof(charFormat);
    lr = SendMessage(m_hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    if (((charFormat.dwMask & CFM_BOLD) == CFM_BOLD) && ((charFormat.dwEffects & CFE_BOLD) == CFE_BOLD))
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_BOLD;
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    else
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_BOLD;
        charFormat.dwEffects |= CFE_BOLD;
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

// <summary>
// Enables caller to toggle the italic setting of the text in the control.
// In order to format the text in the control, we need to construct
// the parameters required to deliver the EM_SETCHARFORMAT message.
// </summary>
HRESULT
CSampleRichEditWindow::SetFormatItalic()
{
    HRESULT hr = S_OK;
    LRESULT lr;
    CHARFORMAT2 charFormat;
    
    ZeroMemory(&charFormat, sizeof(charFormat));
    charFormat.cbSize = sizeof(charFormat);
    lr = SendMessage(m_hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    if (((charFormat.dwMask & CFM_ITALIC) == CFM_ITALIC) && ((charFormat.dwEffects & CFE_ITALIC) == CFE_ITALIC))
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_ITALIC;
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    else
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_ITALIC;
        charFormat.dwEffects |= CFE_ITALIC;
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

// <summary>
// Enables caller to toggle the underline setting of the text in the control.
// In order to format the text in the control, we need to construct
// the parameters required to deliver the EM_SETCHARFORMAT message.
// </summary>
HRESULT
CSampleRichEditWindow::SetFormatUnderline()
{
    HRESULT hr = S_OK;
    LRESULT lr;
    CHARFORMAT2 charFormat;

    ZeroMemory(&charFormat, sizeof(charFormat));
    charFormat.cbSize = sizeof(charFormat);
    lr = SendMessage(m_hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    if (((charFormat.dwMask & CFM_UNDERLINE) == CFM_UNDERLINE) && ((charFormat.dwEffects & CFE_UNDERLINE) == CFE_UNDERLINE))
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_UNDERLINE;
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    else
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_UNDERLINE;
        charFormat.dwEffects |= CFE_UNDERLINE;
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

// <summary>
// Enables caller to set the color of the selected text in the control.
// In order to format the text in the control, we need to construct
// the parameters required to deliver the EM_SETCHARFORMAT message.
// </summary>
HRESULT
CSampleRichEditWindow::SetFormatColor()
{
    HRESULT hr = S_OK;
    LRESULT lr;
    CHARFORMAT2 charFormat;

    ZeroMemory(&charFormat, sizeof(charFormat));
    charFormat.cbSize = sizeof(charFormat);
    lr = SendMessage(m_hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    if (((charFormat.dwMask & CFM_COLOR) == CFM_COLOR) && (((charFormat.dwEffects & CFE_AUTOCOLOR) != CFE_AUTOCOLOR) && (charFormat.crTextColor==RGB(255,0,0))))
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_COLOR;
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    else
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_COLOR;
        charFormat.crTextColor = RGB(255, 0, 0);
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

// <summary>
// Enables caller to set the background color of the selected text in the control.
// In order to format the text in the control, we need to construct
// the parameters required to deliver the EM_SETCHARFORMAT message.
// </summary>
HRESULT
CSampleRichEditWindow::SetFormatBackgroundColor()
{
    HRESULT hr = S_OK;
    LRESULT lr;
    CHARFORMAT2 charFormat;

    ZeroMemory(&charFormat, sizeof(charFormat));
    charFormat.cbSize = sizeof(charFormat);
    lr = SendMessage(m_hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    if (((charFormat.dwMask & CFM_BACKCOLOR) == CFM_BACKCOLOR) && (((charFormat.dwEffects & CFE_AUTOCOLOR) != CFE_AUTOCOLOR) && (charFormat.crBackColor == RGB(255, 255, 0))))
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_BACKCOLOR;
        charFormat.crBackColor = RGB(255, 255, 255);
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    else
    {
        ZeroMemory(&charFormat, sizeof(charFormat));
        charFormat.cbSize = sizeof(charFormat);
        charFormat.dwMask = CFM_BACKCOLOR;
        charFormat.crBackColor = RGB(255, 255, 0);
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &charFormat);
    }
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

// <summary>
// Enables caller to set the alignment of the selected paragraph.
// In order to format the text in the control, we need to construct
// the parameters required to deliver the EM_SETCHARFORMAT message.
// </summary>
HRESULT
CSampleRichEditWindow::SetAlignment(
    _In_ WORD wAlignment
    )
{
    HRESULT hr = S_OK;
    LRESULT lr;
    PARAFORMAT2 paraFormat;
    ZeroMemory(&paraFormat, sizeof(paraFormat));
    paraFormat.cbSize = sizeof(paraFormat);
    paraFormat.dwMask = PFM_ALIGNMENT;
    paraFormat.wAlignment = wAlignment;
    lr = SendMessage(m_hWnd, EM_SETPARAFORMAT, 0, (LPARAM) &paraFormat);
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

// <summary>
// Enables caller to set the selected paragraph(s) as bulleted.
// In order to format the text in the control, we need to construct
// the parameters required to deliver the EM_SETPARAFORMAT message.
// </summary>
HRESULT
CSampleRichEditWindow::SetBulleted()
{
    HRESULT hr = S_OK;
    LRESULT lr;
    PARAFORMAT2 paraFormat;
    ZeroMemory(&paraFormat, sizeof(paraFormat));
    paraFormat.cbSize = sizeof(paraFormat);
    lr = SendMessage(m_hWnd, EM_GETPARAFORMAT, 0, (LPARAM) &paraFormat);
    if (((paraFormat.dwMask & PFM_NUMBERING) == PFM_NUMBERING) && (paraFormat.wNumbering == PFN_BULLET))
    {
        paraFormat.wNumbering = 0;
    }
    else
    {
        paraFormat.wNumbering = PFN_BULLET;
    }
    paraFormat.dwMask = PFM_NUMBERING;
    lr = SendMessage(m_hWnd, EM_SETPARAFORMAT, 0, (LPARAM) &paraFormat);
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

// <summary>
// Enables caller to set the selected paragraph(s) as numbered.
// In order to format the text in the control, we need to construct
// the parameters required to deliver the EM_SETPARAFORMAT message.
// </summary>
HRESULT
CSampleRichEditWindow::SetNumbered()
{
    HRESULT hr = S_OK;
    LRESULT lr;
    PARAFORMAT2 paraFormat;
    ZeroMemory(&paraFormat, sizeof(paraFormat));
    paraFormat.cbSize = sizeof(paraFormat);
    lr = SendMessage(m_hWnd, EM_GETPARAFORMAT, 0, (LPARAM) &paraFormat);
    if (((paraFormat.dwMask & PFM_NUMBERING) == PFM_NUMBERING) && (paraFormat.wNumbering == 7))
    {
        paraFormat.wNumbering = 0;
    }
    else
    {
        paraFormat.wNumbering = 7;
    }
    paraFormat.dwMask = PFM_NUMBERING;
    lr = SendMessage(m_hWnd, EM_SETPARAFORMAT, 0, (LPARAM) &paraFormat);
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

// <summary>
// Enables caller to set the alignment of the selected paragraph.
// In order to format the text in the control, we need to construct
// the parameters required to deliver the EM_SETCHARFORMAT message.
// </summary>
HRESULT
CSampleRichEditWindow::IncrementIndent(
    _In_ INT lIncrement
    )
{
    HRESULT hr = S_OK;
    LRESULT lr;
    PARAFORMAT2 paraFormat;
    ZeroMemory(&paraFormat, sizeof(paraFormat));
    paraFormat.cbSize = sizeof(paraFormat);
    lr = SendMessage(m_hWnd, EM_GETPARAFORMAT, 0, (LPARAM) &paraFormat);
    paraFormat.dwMask = PFM_STARTINDENT;
    paraFormat.dxStartIndent = (paraFormat.dxStartIndent + lIncrement < 0) ? 0 : paraFormat.dxStartIndent + lIncrement;
    lr = SendMessage(m_hWnd, EM_SETPARAFORMAT, 0, (LPARAM) &paraFormat);
    hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

// <summary>
// This method restores the contents of the rich edit control to defaults.
// Default configuration includes text content, as well as formatting.
// </summary>
HRESULT
CSampleRichEditWindow::ResetDefaultContent()
{
    HRESULT hr = S_OK;
    PCWSTR richEditText = NULL;

    // Set text to display in rich edit control from resource file.
    if (SUCCEEDED(hr))
    {
        hr = GetResourceString(IDS_LOREM_IPSUM, &richEditText);
    }
    if (SUCCEEDED(hr))
    {
        hr = SetText(richEditText);
    }

    // Initialize the default format for the rich edit control.
    if (SUCCEEDED(hr))
    {
        hr = SetDefaultFormat();
    }

    // Make sure control is formatted correctly for current DPI.
    if (SUCCEEDED(hr))
    {
        hr = ApplyZoomFactor();
    }

    return hr;
}

// <summary>
// This method retrieves a string based on the requested resource ID.
// Once the string is converted from resource ID to string, this method calls
// the overload to apply the text to the rich edit control.
// </summary>
HRESULT
CSampleRichEditWindow::GetResourceString(
    _In_        UINT stringID, 
    _Outptr_    PCWSTR * stringText
    )
{
    HRESULT hr = S_OK;
    BOOL fResult = FALSE;
    int stringSize = 0;
    HMODULE hMod = nullptr;

    // Get a non-ref counted handle to the current module.
    // Module handle is required to use LoadString.
    fResult = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                NULL,
                                &hMod);
    if (!fResult)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    // Per MSDN documentation, we can retrieve a read-only pointer by
    // passing in a PWSTR and a zero-size buffer.
    if (fResult)
    {
        stringSize = LoadStringW(hMod, stringID, (PWSTR)stringText, 0);
    }
    if (stringSize <= 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

// <summary>
// This method explictly applies the text requested to the
// rich edit control.
// </summary>
HRESULT
CSampleRichEditWindow::SetText(
    _In_    PCWSTR stringText
    )
{
    HRESULT hr = S_OK;
    LRESULT lr; 

    ASSERT(m_hWnd);
    ASSERT(stringText);

    lr = SendMessage(m_hWnd, WM_SETTEXT, (WPARAM) NULL, (LPARAM) stringText);
    if (!lr)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

// <summary>
// This method resets the rich edit control to default values
// for text formatting.  This method will use the default values for rich edit,
// except for bold and font color, which will be set explicitly.
// </summary>
HRESULT
CSampleRichEditWindow::SetDefaultFormat()
{
    HRESULT hr = S_OK;
    LRESULT lr;
    CHARFORMAT2 charFormat;
    ZeroMemory(&charFormat, sizeof(charFormat));

    // Gather the default settings that exist on the rich edit control. 
    charFormat.cbSize = sizeof(CHARFORMAT2) ;
    lr = SendMessage(m_hWnd, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM) &charFormat);

    if (lr)
    {
        // Set text to be bold. 
        charFormat.dwMask = CFM_BOLD;
        
        // Set text color to be black.
        charFormat.dwMask |= CFM_COLOR;
        charFormat.crTextColor = RGB(0, 0, 0);

        // Set font size using inline conversion to twips.
        charFormat.dwMask |= CFM_SIZE;
        charFormat.yHeight = (LONG) (DEFAULT_FONT_PT * TWIPS_PER_PT);

        // Notify the rich edit control of the requested format.
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &charFormat);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

// <summary>
// This method provides an internal implementation to explicitly
// increment or decrement font size.
// </summary>
HRESULT
CSampleRichEditWindow::UpdateFontSize(
    _In_ CHANGE_DIRECTION direction
    )
{
    HRESULT hr = S_OK;
    LRESULT lr;
    CHARFORMAT2 charFormat;
    ZeroMemory(&charFormat, sizeof(charFormat));

    // Gather the settings that exist on the rich edit control.
    charFormat.cbSize = sizeof(charFormat);
    lr = SendMessageW(m_hWnd, EM_GETCHARFORMAT, SCF_DEFAULT, (LPARAM) &charFormat);

    // Note that only the font size is changing.
    if (lr)
    {
        charFormat.dwMask = CFM_SIZE;
        if (direction == CD_INCREMENT)
        {
            charFormat.yHeight = (LONG) (charFormat.yHeight * FONT_STEP);
        }
        else
        {
            charFormat.yHeight = (LONG) (charFormat.yHeight / FONT_STEP);
        }

        // Notify rich edit control of new format.
        lr = SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &charFormat);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

// <summary>
// Set the zoom factor based on values stored in rich edit member variables.
// Once the zoom factor is applied, update the window to redraw.
// </summary>
HRESULT
CSampleRichEditWindow::ApplyZoomFactor()
{
    HRESULT hr = S_OK;
    LRESULT lr;
    
    lr = SendMessage(m_hWnd, EM_SETZOOM, (WPARAM) m_numerator, (LPARAM) m_denominator);
    if (!lr)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//      ChildWnds.cpp
//
// Description:
//      The file contains the definitions of the methods of the classes
//      CInkInputWnd and CRecoOutputWnd.
//      See the file ChildWnds.h for the definitions of the classes.
//--------------------------------------------------------------------------

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

// Windows header file
#include <windows.h>
#include <richedit.h>

// ATL header files:
#include <atlbase.h>        // defines CComModule, CComPtr, CComVariant
extern CComModule _Module;
#include <atlwin.h>         // defines CWindowImpl

// Tablet PC Automation interfaces header file
#include <msinkaut.h>

// The application header files
#include "resource.h"       // main symbols, including command ID's
#include "ChildWnds.h"      // contains the CInkInputWnd and CRecoOutputWnd definitions

////////////////////////////////////////////////////////
// CInkInputWnd methods
////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
// CInkInputWnd::CInkInputWnd
//
// Constructor.
//
// Parameters:
//      none
//
/////////////////////////////////////////////////////////
CInkInputWnd::CInkInputWnd()
    : m_cRows(0), m_cColumns(0), m_iMidline(-1)
{
    m_ptGridLT.x = m_ptGridLT.y = 0;
    m_szWritingBox.cx = m_szWritingBox.cy = 0;
    ::SetRectEmpty(&m_rcDrawnBox);
}

/////////////////////////////////////////////////////////
//
// CInkInputWnd::SetGuide
//
// Data members access method for setting the guide
// drawing parameters.
//
// Parameters:
//      _InkRecoGuide& irg  : see the Tablet PC Automation API
//                            reference for the description
//                            of the structure
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CInkInputWnd::SetGuide(
    const _InkRecoGuide& irg
    )
{
    // Initialize the data members with values from the recognition guide structure
    m_ptGridLT.x = irg.rectWritingBox.left;
    m_ptGridLT.y = irg.rectWritingBox.top;
    m_szWritingBox.cx = irg.rectWritingBox.right - irg.rectWritingBox.left;
    m_szWritingBox.cy = irg.rectWritingBox.bottom - irg.rectWritingBox.top;
    m_rcDrawnBox = irg.rectDrawnBox;
    m_iMidline = irg.midline;
    m_cRows = irg.cRows;
    m_cColumns = irg.cColumns;

    // Update the window
    if (IsWindow())
    {
        Invalidate();
    }
}

/////////////////////////////////////////////////////////
//
// CInkInputWnd::SetGuide
//
// Data members access method for setting the number
// of the guide's rows and columns.
//
// Parameters:
//      int iRows       : the number of rows
//      int iColumns    : the number of columns
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CInkInputWnd::SetRowsCols(
    int iRows,
    int iColumns
    )
{
    m_cRows = iRows;
    m_cColumns = iColumns;

    // Update the window
    if (IsWindow())
    {
        Invalidate();
    }
}

/////////////////////////////////////////////////////////
//
// CInkInputWnd::OnPaint
//
// The WM_PAINT message handler. The ATL calls this member
// function when Windows or an application makes a request
// to repaint a portion of the CInkInputWnd object's window.
// The method paints the window's background and draws
// the guide if necessary.
//
// Parameters:
//      defined in the ATL's MESSAGE_HANDLER macro,
//      none is used here
//
// Return Value (LRESULT):
//      always 0
//
/////////////////////////////////////////////////////////
LRESULT CInkInputWnd::OnPaint(
        UINT /*uMsg*/,
        WPARAM /*wParam*/,
        LPARAM /*lParam*/,
        BOOL& /*bHandled*/
        )
{
    RECT rcClip;
    if (FALSE == GetUpdateRect(&rcClip))
        return 0;   // there's no update region, so no painting is needed

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
    if (hdc == NULL)
        return 0;

    // Get the rectangle to paint.
    GetClipBox(hdc, &rcClip);

    // Paint the background.
    ::FillRect(hdc, &rcClip, (HBRUSH)::GetStockObject(DC_BRUSH));

    // Calculate the grid rectangle. Assume that if there are any guides,
    // they are either boxes or horizontal lines.
    RECT rcGrid = {m_ptGridLT.x, m_ptGridLT.y, 0,
                   m_ptGridLT.y + m_szWritingBox.cy * m_cRows};
    if (0 == m_cColumns && 0 != m_cRows)
    {
        rcGrid.right = rcClip.right;
    }
    else
    {
        rcGrid.right = m_ptGridLT.x + m_szWritingBox.cx * m_cColumns;
    }

    // Draw the guide grid, if it's visible and not empty.
    RECT rcVisible;
    if (FALSE == ::IsRectEmpty(&rcGrid)
         && TRUE == ::IntersectRect(&rcVisible, &rcGrid, &rcClip))
    {

        // Create a thin lightgray pen to draw the guides.
        HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(192, 192, 192));
        HGDIOBJ hOldPen = ::SelectObject(hdc, hPen);

        if (0 == m_cColumns)
        {
             // Draw horizontal lines at the bottom side of the guide's DrawnBox
            int iY = rcClip.top - ((rcClip.top - m_ptGridLT.y) % m_szWritingBox.cy) + m_rcDrawnBox.bottom;
            for (int iRow = (rcClip.top - m_ptGridLT.y) / m_szWritingBox.cy;
                 (iRow < m_cRows) && (iY < rcClip.bottom);
                 iRow++, iY += m_szWritingBox.cy)
            {
                ::MoveToEx(hdc, rcClip.left, iY, NULL);
                ::LineTo(hdc, rcClip.right, iY);
            }
        }
        else
        {
            // Draw boxes
            int iY = rcClip.top - ((rcClip.top - m_ptGridLT.y) % m_szWritingBox.cy);
            for (int iRow = (rcClip.top - m_ptGridLT.y) / m_szWritingBox.cy;
                 (iRow < m_cRows) && (iY < rcClip.bottom);
                 iRow++, iY += m_szWritingBox.cy)
            {
                int iX = rcClip.left - ((rcClip.left - m_ptGridLT.x) % m_szWritingBox.cx);
                RECT rcBox = m_rcDrawnBox;
                ::OffsetRect(&rcBox, iX, iY);
                for (int iCol = (rcClip.left - m_ptGridLT.x) / m_szWritingBox.cx;
                     (iCol < m_cColumns) && (rcBox.left < rcClip.right);
                     iCol++)
                {
                    ::Rectangle(hdc, rcBox.left, rcBox.top, rcBox.right, rcBox.bottom);
                    ::OffsetRect(&rcBox, m_szWritingBox.cx, 0);
                }
            }
        }

        // Restore the dc and delete the pen
        ::SelectObject(hdc, hOldPen);
        ::DeleteObject(hPen);
    }

    EndPaint(&ps);
    return 0;
}


////////////////////////////////////////////////////////
// CRecoOutputWnd methods
////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::CRecoOutputWnd
//
// Constructor.
//
// Parameters:
//      none
//
/////////////////////////////////////////////////////////
CRecoOutputWnd::CRecoOutputWnd()
        : m_hRichEdit(NULL), m_iSelStart(0)
{
}

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::~CRecoOutputWnd
//
// Destructor.
//
/////////////////////////////////////////////////////////
CRecoOutputWnd::~CRecoOutputWnd()
{
    if (NULL != m_hRichEdit)
    {
        ::FreeLibrary(m_hRichEdit);
    }
}

// Helper methods

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::~CRecoOutputWnd
//
// Destructor.
//
/////////////////////////////////////////////////////////
HWND CRecoOutputWnd::Create(
        HWND hwndParent,
        UINT nID
        )
{
    if (NULL == m_hWnd)
    {
        m_hRichEdit = ::LoadLibraryW(L"Riched20.dll");
        if (NULL != m_hRichEdit)
        {
            // Create a richedit control
            HWND hwndREdit = ::CreateWindowW(RICHEDIT_CLASSW,
                                             NULL,
                                             WS_VISIBLE | WS_CHILD | WS_BORDER
                                             | ES_SUNKEN | ES_MULTILINE | ES_READONLY,
                                             0, 0, 1, 1,
                                             hwndParent,
                                             (HMENU)nID,
                                             _Module.GetModuleInstance(),
                                             NULL
                                             );
            // Subclass it and attach to this CRecoOutputWnd object
            if (NULL != hwndREdit)
            {
                SubclassWindow(hwndREdit);
            }
        }
    }
    return m_hWnd;
}

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::ResetResults
//
// Empties the output strings.
//
// Parameters:  none
//
// Return Value (void): none
//
/////////////////////////////////////////////////////////
void CRecoOutputWnd::ResetResults()
{
    m_iSelStart = 0;
    if (IsWindow())
    {
        // Reset the text in the RichEdit control
        ::SendMessage(m_hWnd, WM_SETTEXT, 0, (LPARAM)L"");
    }
}

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::AddString
//
// Adds a recognition result string to the existing text
// in the RichEdit control
//
// Parameters:
//      COLORREF clr    : [in] output color for the string
//      BSTR bstr       : [in] recognition result of a stroke collection
//
// Return Value (void): none
//
/////////////////////////////////////////////////////////
void CRecoOutputWnd::AddString(
        COLORREF clr,
        BSTR bstr
        )
{
    // Set the string start position to the end of the text
    m_iSelStart = ::SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0);

    // If the string is not first, add a delimiting space character first
    if (0 != m_iSelStart)
    {
        ::SendMessageW(m_hWnd, EM_REPLACESEL, FALSE, (LPARAM)L" ");
        m_iSelStart++;
    }

    // Set the string format
    CHARFORMAT2W cf;
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR;
    cf.crTextColor = clr;
    cf.dwEffects = 0;
    ::SendMessageW(m_hWnd, EM_SETSEL, m_iSelStart, -1);
    ::SendMessageW(m_hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    // Add the string to the text
    if (NULL != bstr)
    {
        ::SendMessageW(m_hWnd, EM_REPLACESEL, FALSE, (LPARAM)bstr);
    }
}

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::UpdateString
//
// In the RichEdit control, updates the recognition result string
// of the current stroke collection
//
// Parameters:
//      BSTR bstr   : [in] recognition result string
//
// Return Value (void): none
//
/////////////////////////////////////////////////////////
void CRecoOutputWnd::UpdateString(
        BSTR bstr
        )
{
    // To update the string, select it in the RichEdit control ...
    ::SendMessageW(m_hWnd, EM_SETSEL, m_iSelStart, -1);
    // ... and replace the selected text
    ::SendMessageW(m_hWnd, EM_REPLACESEL, FALSE, (LPARAM)bstr);
    // The new text will inherit the format settings
}




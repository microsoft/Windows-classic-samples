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

// ATL header files:
#include <atlbase.h>        // defines CComModule, CComPtr, CComVariant
extern CComModule _Module;
#include <atlwin.h>         // defines CWindowImpl

// Tablet PC Automation interfaces header file
#include <msinkaut.h>

// The application header files
#include "resource.h"       // main symbols, including command ID's
#include "ChildWnds.h"      // contains the CInkInputWnd and CRecoOutputWnd definitions

#define CLR_BLUE    RGB(0x00,0x00,0x80)
#define CLR_GRAY    RGB(0x80,0x80,0x80)

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
//     none
//
/////////////////////////////////////////////////////////
CInkInputWnd::CInkInputWnd() : m_cRows(0), m_cColumns(0), m_iMidline(-1)
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
//     _InkRecoGuide& irg  : see the Tablet PC Automation API
//                            reference for the description
//                            of the structure
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CInkInputWnd::SetGuide(const _InkRecoGuide& irg)
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
//     int iRows       : the number of rows
//     int iColumns    : the number of columns
//
// Return Values (void):
//      none
//
/////////////////////////////////////////////////////////
void CInkInputWnd::SetRowsCols(int iRows, int iColumns)
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
//     defined in the ATL's MESSAGE_HANDLER macro,
//     none is used here
//
// Return Value (LRESULT):
//     always 0
//
/////////////////////////////////////////////////////////
LRESULT CInkInputWnd::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/,
                              LPARAM /*lParam*/, BOOL& /*bHandled*/)
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
//     none
//
/////////////////////////////////////////////////////////
CRecoOutputWnd::CRecoOutputWnd()
        : m_hFont(NULL), m_iFontName(-1), m_nGesture(0), m_bNewGesture(false)
{
    UpdateFont(::GetUserDefaultLangID());
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
    if (NULL != m_hFont)
    {
        ::DeleteObject(m_hFont);
    }
}

// Message handlers

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::OnPaint
//
// The WM_PAINT message handler. The ATL calls this member
// function when Windows or an application makes a request
// to repaint a portion of the CRecoOutputWnd object's window.
//
// Parameters:
//     defined in the ATL's MESSAGE_HANDLER macro,
//     none is used here
//
// Return Value (void):
//     none
//
/////////////////////////////////////////////////////////
LRESULT CRecoOutputWnd::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/,
                                LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    RECT rc;
    if (FALSE == GetUpdateRect(&rc))
        return 0;   // there's no update region, so no painting is needed

    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
    if (hdc == NULL)
        return 0;

    GetClientRect(&rc);

    // Paint the background
    ::FillRect(hdc, &rc, (HBRUSH)::GetStockObject(WHITE_BRUSH));

    // Use blue color to draw the latest results, and gray for the old ones
    COLORREF clrGesture, clrText;
    if (true == m_bNewGesture)
    {
        clrGesture = CLR_BLUE;
        clrText = CLR_GRAY;
    }
    else
    {
        clrGesture = CLR_GRAY;
        clrText = CLR_BLUE;
    }

    COLORREF clrOld = ::SetTextColor(hdc, clrGesture);

    // Output the name of the last gesture
    if (0 != m_nGesture)
    {
        TCHAR buffer[100];
        if (::LoadString(_Module.GetResourceInstance(),
                         m_nGesture, buffer, sizeof(buffer)/sizeof(buffer[0])) != 0)
        {
            ::TextOut(hdc, mc_iMarginX, mc_iMarginY, buffer, _tcslen(buffer));
        }
    }

    // Output the handwriting recognition results
    HGDIOBJ hOldFont;
    if (NULL != m_hFont)
    {
        hOldFont = ::SelectObject(hdc, m_hFont);
    }
    ::SetTextColor(hdc, clrText);
    for (int i = 0; i < mc_iNumResults; i++)
    {
        int iLength = m_bstrResults[i].Length();
        if (iLength)
        {
            ::TextOutW(hdc, mc_iMarginX, mc_iMarginY + (i + 1) * mc_iFontHeight,
                       m_bstrResults[i], iLength);
        }
    }

    // Restore the dc
    ::SetTextColor(hdc, clrOld);
    if (NULL != m_hFont)
    {
        ::SelectObject(hdc, hOldFont);
    }

    EndPaint(&ps);
    return 0;
}

// Helper methods

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::ResetResults
//
// Empties the output strings.
//
// Parameters:
//     none
//
// Return Value (void):
//     none
//
/////////////////////////////////////////////////////////
void CRecoOutputWnd::ResetResults()
{
    m_bNewGesture = false;
    m_nGesture = 0;
    for (int i = 0; i < mc_iNumResults; i++)
        m_bstrResults[i].Empty();
}

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::GetBestHeight
//
//      Returns a height of the window the best to show all
//      the strings (the mc_iNumResults result strings and
//      the name of the last geesture).
//
//      This method is called by CAdvRecoApp object when it updates
//      the layouts of its child windows.
//
/////////////////////////////////////////////////////////
int CRecoOutputWnd::GetBestHeight()
{
    return (mc_iMarginX * 2 + mc_iFontHeight * (mc_iNumResults + 1));
}

/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::UpdateFont
//
//     The method creates an appropriate font for text output
//     based on the language id. It's called by the application
//     when user selects a recognizer.
//
// Parameters:
//     LANGID wLangId      : [in] the id of the language of the text
//
// Return Value (bool):
//     true, if succeded to create a font appropriate for the language
//     false otherwise
//
/////////////////////////////////////////////////////////
bool CRecoOutputWnd::UpdateFont(LANGID wLangId)
{
    static LPCTSTR pszFontNames[] = { TEXT("MS Shell Dlg"), TEXT("MS Mincho"),
                                      TEXT("Mingliu"), TEXT("Gullim") };
    bool bOk = true;
    int i;
    DWORD dwCharSet;
    // Select a font by the primary language id (the lower byte)
    switch(wLangId & 0xFF)
    {
        default:
            i = 0;
            dwCharSet = DEFAULT_CHARSET;
            break;

        case LANG_JAPANESE:
            i = 1;
            dwCharSet = SHIFTJIS_CHARSET;
            break;

        case LANG_CHINESE:
            i = 2;
            dwCharSet = CHINESEBIG5_CHARSET;
            break;

        case LANG_KOREAN:
            i = 3;
            dwCharSet = JOHAB_CHARSET;
            break;
    }
    if (i != m_iFontName)
    {
        HFONT hFont = ::CreateFont(mc_iFontHeight, 0, 0, 0, 0, 0, 0, 0,
                                   dwCharSet,
                                   0, 0, 0, 0,
                                   pszFontNames[i] // typeface name
                                   );
        if (NULL != hFont)
        {
            if (NULL != m_hFont)
                ::DeleteObject(m_hFont);
            m_hFont = hFont;
            m_iFontName = i;
        }
        else
        {
            bOk = false;
        }
    }

    return bOk;
}


/////////////////////////////////////////////////////////
//
// CRecoOutputWnd::SetGesture
//
//     The method is called by the application from OnGesture
//     event handler. It accepts the id if the recognized gesture
//     and converts it to a string for output.
//
// Parameters:
//     InkGestureType idGesture : [in] the id of the language of the text
//
// Return Value (bool):
//     true - the name of the gesture is known to this application
//     false - otherwise
//
/////////////////////////////////////////////////////////
void CRecoOutputWnd::SetGestureName(UINT nGesture)
{
    m_nGesture = nGesture;
    m_bNewGesture = true;
}

// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------

//
// drawarea.cpp : implementation file
//



#include "stdafx.h"
#include "Imp_draw.h"
#include "Imp_dlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDrawArea

CDrawArea::CDrawArea()
{
}

CDrawArea::~CDrawArea()
{
}


BEGIN_MESSAGE_MAP(CDrawArea, CEdit)
	//{{AFX_MSG_MAP(CDrawArea)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_RBUTTONUP()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawArea message handlers

void CDrawArea::AddKeystroke(char *mbsKey)
{
	//
	// The backspace keystroke should erase the last character.
	//
	if (mbsKey[0] == VK_BACK)
	{
		int nStart, nEnd;
		GetSel(nStart, nEnd);
		SetSel(nEnd-1, nEnd);
		mbsKey[0] = '\0';

        //
        // Erasing a line feed should erase the preceding carriage return.
        //
        CString strText;
        GetWindowText(strText);
        int iLength = strText.GetLength();
        if (iLength > 0 && strText.GetAt(strText.GetLength() - 1) == '\n')
            SetSel(nEnd-2, nEnd);
	}

	//
	// Append a line feed to a carriage return.
	//
	else if (mbsKey[0] == VK_RETURN)
	{
		mbsKey[1] = '\xa';
		mbsKey[2] = '\0';
	}

	//
	// Store the new keystroke.
	//
	ReplaceSel(mbsKey);
}


void CDrawArea::AddLine(LINE line)
{
	//
	// Set the clipping region for the drawing.
	//
	CClientDC dc(this);
	RECT rect;
	GetClientRect(&rect);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rect); 
	dc.SelectClipRgn(&rgn);

	//
	// Draw the line and save it for redrawing.
	//
	dc.MoveTo(line.ptStart);
	dc.LineTo(line.ptEnd);
    m_listLines.AddTail(line);
}


void CDrawArea::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call the default.
	
	m_ptLast = point;
	SetFocus();
}


void CDrawArea::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call the default.
	
	if (nFlags & MK_LBUTTON)
	{
		//
		// Add the drawn line to the drawing.
		//
		LINE line;
		line.ptStart = m_ptLast;
		line.ptEnd = point;
        AddLine(line);

		//
		// Send the line to the friend queue.
		//
		((CImp_DrawDlg *)GetParent())->SendMouseMovement(line);

		//
		// Remember the end point of the line.
		//
		m_ptLast = point;
	}
}


void CDrawArea::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here.

	//
	// Set the clipping region for the drawing.
	//
	RECT rect;
	GetClientRect(&rect);
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rect); 
	dc.SelectClipRgn(&rgn);

	//
	// Erase the drawing area.
	//
	dc.FillSolidRect(&rect, GetSysColor(COLOR_MENU));

	//
	// Redisplay the text.
	//
    CString strText;
    GetWindowText(strText);
    dc.SelectStockObject(ANSI_VAR_FONT);
    rect.top++;
    rect.left++;
    dc.DrawText(strText, &rect, DT_TOP | DT_WORDBREAK);

	//
	// Redraw each line.
	//
	LINE line;
	POSITION posLine = m_listLines.GetHeadPosition();
	while (posLine != NULL)
	{
		line = m_listLines.GetNext(posLine);
		dc.MoveTo(line.ptStart);
		dc.LineTo(line.ptEnd);
	}

	// Do not call CEdit::OnPaint() for painting messages.
}


void CDrawArea::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call the default.
	
	//
	// Clear the drawing area.
	//
	SetSel(0, -1);
	Clear();
	m_listLines.RemoveAll();
	RedrawWindow();
}


BOOL CDrawArea::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call the default.
	
	return 0;
}


void CDrawArea::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here.

	HideCaret();
}


HBRUSH CDrawArea::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	// TODO: Change any attributes of the DC here.
	pDC->SetBkColor(GetSysColor(COLOR_MENU));
	
	// TODO: Return a non-NULL brush if the parent's handler should not be called.
	return HBRUSH((CBrush *)pDC->SelectStockObject(LTGRAY_BRUSH));
}


void CDrawArea::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call the default.

	switch (nChar)
	{
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:	
	case VK_HOME:	break;
	default:		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}


void CDrawArea::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call the default.
	((CImp_DrawDlg *)GetParent())->SendKeystroke(nChar);
	
	CEdit::OnChar(nChar, nRepCnt, nFlags);

	RedrawWindow();
}


void CDrawArea::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call the default.
}

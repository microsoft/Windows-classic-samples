// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:       
//      ChildWnds.h
//
// Description:
//      This file contains the definitions of the classes CInkInputWnd 
//      and CRecoOutputWnd, which are derived from the ATL's CWindowImpl
//      and used for creating the sample's child windows.
//		The methods of the classes are defined in the ChildWnds.cpp file.
//--------------------------------------------------------------------------

#pragma once

/////////////////////////////////////////////////////////
//                                          
// class CInkInputWnd
// 
// The CInkInputWnd class allows to create a simple window, that 
// may draw itself with lined or boxed guides or with no guides at all.
//
// An object of the class is used in the CAdvRecoApp for pen input.
//
/////////////////////////////////////////////////////////

class CInkInputWnd :
    public CWindowImpl<CInkInputWnd>
{
	// Data members 
	POINT   m_ptGridLT;			// the left-top corner of the guide grid
    SIZE    m_szWritingBox;		// the size of the guide's writing box
    RECT    m_rcDrawnBox;		// the guide's draw box rectangle relative to the writing box
    int     m_cRows;			// the number of rows in the guide
    int     m_cColumns;			// the number of columns in the guide
    int     m_iMidline;			// the position of the midline the writing box

public:

	// Constructor
    CInkInputWnd();

    // Data members access methods 
    void SetGuide(
            const _InkRecoGuide& irg
            );
    void SetRowsCols(
            int iRows, 
            int iColumns
            );

// Declare the objects' window class with NULL background (-1) to avoid flicking
// that happens because of delays between WM_ERASEBKGND and WM_PAINT messages 
// sent to the window. The background will be painted in the WM_PAINT handler.
DECLARE_WND_CLASS_EX(NULL, 0, -1)

// ATL macro's to declare which commands/messages the class is interested in
BEGIN_MSG_MAP(CInkInputWnd)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
END_MSG_MAP()
     
    // WM_PAINT message handler
	LRESULT OnPaint(
            UINT uMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL& bHandled
            );

};  // class CInkInputWnd



/////////////////////////////////////////////////////////
//                                          
// class CRecoOutputWnd
// 
// The CRecoOutputWnd class allows to create a window, that draws 
// a few lines of text in it.
// 
// An object of the class is used in the CAdvRecoApp to output 
// results of the recognition. 
//
/////////////////////////////////////////////////////////

class CRecoOutputWnd :
    public CWindowImpl<CRecoOutputWnd>
{
public:

// Data members

	HMODULE		m_hRichEdit;
	int			m_iSelStart;

// Constructor and destructor
    
	CRecoOutputWnd();
    ~CRecoOutputWnd();


	HWND Create(
            HWND hwndParent, 
            UINT nID
            );
    void AddString(
            COLORREF clr, 
            BSTR bstr
            );
    void UpdateString(
            BSTR bstr
            );
	void ResetResults();

// ATL macro's to declare which commands/messages the class is interested in.
BEGIN_MSG_MAP(CRecoOutputWnd)
END_MSG_MAP()

};  // class CRecoOutputWnd


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


// ChildView.h : interface of the CChildView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDVIEW_H__C382F3F2_96AC_11D2_9012_00C04FC2D3B8__INCLUDED_)
#define AFX_CHILDVIEW_H__C382F3F2_96AC_11D2_9012_00C04FC2D3B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dlgdest.h"
#include "dlgsrc.h"
#include "wave.h"

/////////////////////////////////////////////////////////////////////////////
// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

//  Attributes
public:
    SIZE    m_sizeInitPos;
    int     m_cChannels;

// Operations
public:
    //  
    BOOL    UpdateDialogs(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChildView)
	afx_msg void OnPaint( );
	afx_msg void OnMixerMerge( );
	afx_msg void OnUpdateMixerMerge( CCmdUI* pCmdUI );
	afx_msg void OnMixerPlay( );
	afx_msg void OnUpdateMixerPlay( CCmdUI* pCmdUI );
    afx_msg void OnMixerStop( );
    afx_msg void OnUpdateMixerStop( CCmdUI* pCmdUI );
	afx_msg void OnFileSave( );
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
    afx_msg void OnSetfocusEditValidbits( );
	afx_msg void OnFileOpen( );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CChildView* g_pChildView;

//
//  define the first non used speaker (currently)
//  SPEAKER_TOP_BACK_RIGHT << 1
//
#define SPEAKER_NOT_SPECIFIED   0x40000
#define SPEAKERS_USED           0x12


//
//  custom messages
//
#define WM_START_PLAYBACK   (WM_APP + 1)
#define WM_STOP_PLAYBACK    (WM_APP + 2)

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDVIEW_H__C382F3F2_96AC_11D2_9012_00C04FC2D3B8__INCLUDED_)

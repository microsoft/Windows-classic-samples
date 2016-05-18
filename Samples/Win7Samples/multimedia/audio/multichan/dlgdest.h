// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#if !defined(AFX_DLGDEST_H__3BF53D52_96B1_11D2_9012_00C04FC2D3B8__INCLUDED_)
#define AFX_DLGDEST_H__3BF53D52_96B1_11D2_9012_00C04FC2D3B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgDest.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgDest dialog

class CDlgDest : public CDialog
{
// Construction
public:
	CDlgDest(CWnd* pParent = NULL);     //  standard constructor
    ~CDlgDest( void );                  //  dtor
    void Create(void);
    void Update(void);
    void RecalcDependantVariables(void);

//  Attributes
    CWnd*                   m_pwndParent;   //  
    LPWAVEHDR               m_lpwhdr;
    WAVEFORMATEXTENSIBLE    m_wfExt;        //
    PBYTE                   m_pbData;       
    ULONG                   m_cbData;
    BOOL                    m_fPlayable;
    BOOL                    m_fDragging;
    CPoint                  m_ptPosInCaption;

//  Accessors
//  Overrides
    virtual LRESULT WindowProc( UINT, WPARAM, LPARAM );

// Dialog Data
	//{{AFX_DATA(CDlgDest)
	enum { IDD = IDD_DST };
	CStatic	    m_cChannelMask;
	CStatic	    m_cAveBitsPerSec;
	CButton	    m_cRemix;
	CEdit	    m_cValidBits;
	CStatic	    m_cChannels;
	CStatic	    m_cOutput;
	CComboBox	m_comboBitDepth;
	CComboBox	m_comboSampleRate;
	CComboBox	m_comboFormat;
	CButton	    m_butPlay;
    CButton     m_butStop;
	UINT	    m_wValidBitsPerSample;
	CString	    m_strChannelMask;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDest)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	// Generated message map functions
	//{{AFX_MSG(CDlgDest)
	afx_msg void OnRemix();
	afx_msg void OnPlay();
    afx_msg void OnStop();
	virtual BOOL OnInitDialog();
	afx_msg void OnComboSamplerate();
	afx_msg void OnComboWaveformat();
	afx_msg void OnComboBitdepth();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnEditValidbits();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CDlgDest*    g_pdlgDest;
extern HWAVEOUT     g_hwo;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDEST_H__3BF53D52_96B1_11D2_9012_00C04FC2D3B8__INCLUDED_)

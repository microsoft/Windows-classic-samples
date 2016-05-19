// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#if !defined(AFX_DLGSRC_H__D1BB3766_96B0_11D2_9012_00C04FC2D3B8__INCLUDED_)
#define AFX_DLGSRC_H__D1BB3766_96B0_11D2_9012_00C04FC2D3B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSrc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSrc dialog

/*
 *
 *  Class           :   CDlgSrc
 *  Purpose         :   represents one audio channel along with its UI
 *
 */
class CDlgSrc : public CDialog
{
//
//  Construction
//
public:
	                CDlgSrc             ( CWnd * );
                    ~CDlgSrc            ( void );                
    void            Create              ( LPCSTR, SIZE, DWORD );

//
//  Attributes
//
public:
    CWnd*           m_pwndParent;       //  link to parent
    LPWAVEHDR       m_lpwhdr;           //  ptr to a wave header
    PVOID           m_pvData;           //  ptr to data buffer
    ULONG           m_cbData;           //  size of the data buffer
    ULONG           m_cSamples;         //  estimated number of samples
    DWORD           m_dwChannelMask;    //  speakers used
    WAVEFORMATEX    m_wfx;              //  waveformatex structure
    UINT            m_nChannel;         //  channel's ID
    BOOL            m_fDragging;        //  dragging flag
    BOOL            m_fPlayable;        //  play flag
    CPoint          m_ptPosInCaption;   //  

//
//  Accessors
//
public:
    void			SetChannelNum		( UINT );								//  sets the channel ID
    BOOL            AquireData          ( WAVEFORMATEX *, PVOID, ULONG, UINT ); //  acquire data off of the multichan input wave

//  Overrides
    virtual LRESULT WindowProc          ( UINT, WPARAM, LPARAM );               //  message processing procedure

// Dialog Data
public:                                 //  MFC/UI stuff
	//{{AFX_DATA(CDlgSrc)
	enum            { IDD = IDD_SRC };  //  Identifier
	CButton	        m_butClose;         //  close button
    CButton         m_butPlay;          //  start button
    CButton         m_butStop;          //  stop button
	CStatic	        m_cInput;           
	CComboBox	    m_comboSpeaker;     //  speaker selection list
	BOOL	        m_fUse;             //  include flag; not used
	int             m_nSpeaker;         //  selected speaker
	CString	        m_strName;          //  window title
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSrc)
	protected:
	virtual void    DoDataExchange      (CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSrc)
	afx_msg void    OnComboSpeaker      ();                     //  speaker selection function
	afx_msg void    OnPlay              ();                     //  play function
    afx_msg void    OnStop              ();                     //  stop function
	virtual void    OnOK                ();                     //  
	virtual void    OnCancel            ();                     //
	afx_msg HBRUSH  OnCtlColor          ( CDC*, CWnd*, UINT );  //
	afx_msg void    OnLButtonDown       (UINT, CPoint );        //
	afx_msg void    OnLButtonUp         (UINT, CPoint );        //
	afx_msg void    OnMouseMove         (UINT, CPoint );        //
	virtual BOOL    OnInitDialog        ();                     //
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};  //  CDlgSrc

//
//  list of opened channels
//
extern CList<CDlgSrc*,CDlgSrc*>    g_listSources;

/*
 *	Inlines
 */

//
// ----------------------------------------------------------------------------------
// CDlgSrc::SetChannelNum
//  Updates m_nChannel and corresponding UI
// ----------------------------------------------------------------------------------
inline
void 
CDlgSrc::SetChannelNum
(
    UINT    nChannel
)
{
    char sz[10];
    m_nChannel = nChannel;

	if (SUCCEEDED(StringCchPrintfA(sz, 10, "%d", nChannel)))
	{
	    SetDlgItemText( IDC_CHANNELNUM, sz );
	}

}   //  CDglSrc::SetChannelNum


//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DLGSRC_H__D1BB3766_96B0_11D2_9012_00C04FC2D3B8__INCLUDED_)


// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// COMRTSDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "rtscom.h"

// CCOMRTSDlg dialog
class CCOMRTSDlg : public CDialog
{
// Construction
public:
	CCOMRTSDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_COMRTS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
	HCURSOR m_hCursor;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	// Dialog control members
	CStatic m_gbTestArea;
    CListBox m_ListBox;
	CButton* m_pCheck[4];
	CStatic m_staticGestureStatus;

public:
	// Message handlers
	afx_msg void OnBnClickedButtonClearTestArea();
    afx_msg void OnBnClickedCheck();
    afx_msg void OnBnClickedButtonUp();
    afx_msg void OnBnClickedButtonDown();

public:
	// Helper methods
	HRESULT InitRealTimeStylus();
	HRESULT InitPacketFilter();
	HRESULT InitDynamicRenderer();
	HRESULT InitGestureRecognizer();
	HRESULT InitGestureHandler();
	HRESULT InitCustomRenderer();
    //HRESULT InsertIntoPluginCollection(int nIndex);
    HRESULT InsertIntoPluginCollection(int nIndex, IStylusSyncPlugin* pSyncPlugin);
    HRESULT RemoveFromPluginCollection(int nIndex);
    void Clear();
    int FindPrecedingPlugin(int nIndex);
};

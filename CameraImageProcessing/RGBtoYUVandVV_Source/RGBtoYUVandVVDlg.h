
// RGBtoYUVandVVDlg.h : header file
//

#pragma once

#include "BitmapClass.h"
#include "RGBtoYUV.h"


// CRGBtoYUVandVVDlg dialog
class CRGBtoYUVandVVDlg : public CDialogEx
{
// Construction
public:
	CRGBtoYUVandVVDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RGBTOYUVANDVV_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);

private:

    BitmapClass myBitmap;

public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
private:
    CString m_csImageType;
    CString m_csChromaSampling;
public:
    afx_msg void OnBnClickedButton1();
private:
    CString m_csChromaSampleR;
public:
    afx_msg void OnCbnSelchangeCombo2();
};

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#if !defined(AFX_DEVICEPROP_H__D5EABECF_0B96_488F_AF56_F439B3720C3A__INCLUDED_)
#define AFX_DEVICEPROP_H__D5EABECF_0B96_488F_AF56_F439B3720C3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeviceProp.h : header file
//
#include "util.h"
#include "genericucpdlg.h"
/////////////////////////////////////////////////////////////////////////////
// DeviceProp dialog

class CDeviceProp : public CDialog
{
// Construction
public:
	CDeviceProp(CWnd* pParent = NULL);   // standard constructor
	CDeviceProp(IUPnPDevice *pDevice, CWnd* pParent = NULL);   // standard constructor
	~CDeviceProp();
	// Dialog Data
	//{{AFX_DATA(CDeviceProp)
	enum { IDD = IDD_DEVPROP_DIALOG };
	CButton	m_OkButton;
	CEdit	m_UPC;
	CEdit	m_UDN;
	CEdit	m_SerialNumber;
	CEdit	m_PresentationUrl;
	CEdit	m_ModelUrl;
	CEdit	m_ModelNumber;
	CEdit	m_ModelName;
	CEdit	m_ModelDescription;
	CEdit	m_ManufacturerUrl;
	CEdit	m_Manufacturer;
	CEdit	m_FriendlyName;
	CEdit	m_DeviceType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviceProp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeviceProp)
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void PrintTextToEditBox(CEdit *pEditBox, BSTR bstrPrintText, HRESULT hrCheck);
	IUPnPDevice* pCurrentDevice;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVICEPROP_H__D5EABECF_0B96_488F_AF56_F439B3720C3A__INCLUDED_)

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// GenericUCPDlg.h : header file
//

#if !defined(AFX_GenericUCPDLG_H__06D8F92D_3842_4BB8_9B3B_39AF51C4070A__INCLUDED_)
#define AFX_GenericUCPDLG_H__06D8F92D_3842_4BB8_9B3B_39AF51C4070A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <objbase.h>
#include <upnp.h>

#define DATA_BUFSIZE 2048


class CGenericUCPDlg;

// DeviceFinder Callback 
class CDeviceFinderCallback : 
	public CComObjectRootEx <CComSingleThreadModel>,
	public IUPnPDeviceFinderCallback
{
	private: 
		CGenericUCPDlg* m_pGenericUCPDlg;
	public:
	CDeviceFinderCallback() {};
	~CDeviceFinderCallback() {};

	BEGIN_COM_MAP(CDeviceFinderCallback)
        COM_INTERFACE_ENTRY(IUPnPDeviceFinderCallback)
    END_COM_MAP()
	
	// *** IUPnPDeviceFinderCallback methods ***
    STDMETHOD(DeviceAdded)(LONG lFindData, IUPnPDevice * pDevice);
    STDMETHOD(DeviceRemoved)(LONG lFindData, BSTR bstrUDN);
    STDMETHOD(SearchComplete)(LONG lFindData);
		
	// Other functions
	static CDeviceFinderCallback * Create();
	void SetDialogPointer(CGenericUCPDlg* pDialog);

};

// Service Callback 
class CServiceCallback : 
	public CComObjectRootEx <CComSingleThreadModel>,
	public IUPnPServiceCallback
{
	private: 
		CGenericUCPDlg* m_pGenericUCPDlg;
	public:

	CServiceCallback() {};
	~CServiceCallback() {};

	BEGIN_COM_MAP(CServiceCallback)
        COM_INTERFACE_ENTRY(IUPnPServiceCallback)
    END_COM_MAP()

	// *** IUPnPServiceCallback methods ***
	STDMETHOD (StateVariableChanged)(IUPnPService *pus, LPCWSTR pcwszStateVarName, VARIANT varValue);
	STDMETHOD (ServiceInstanceDied)(IUPnPService *pus);

	// Other functions
	static CServiceCallback * Create();
	void SetDialogPointer(CGenericUCPDlg* pDialog);
	
	
};


////////////////////////////////////////////////////////////////////////////
// CGenericUCPDlg dialog

class CGenericUCPDlg : 
	public CDialog
{
// Construction
public:
	CGenericUCPDlg(CWnd* pParent = NULL);	// standard constructor
	~CGenericUCPDlg();


	// Dialog Data
	//{{AFX_DATA(CGenericUCPDlg)
	enum { IDD = IDD_GENERICUCP_DIALOG };
	CEdit	m_VariableName;
	CEdit	m_ActionName;
	CStatic	m_StatusText;
	CStatic	m_EventText;
	CButton	m_ViewSCPD;
	CEdit	m_SCPDURL;
	CButton	m_Presentation;
	CButton	m_QueryVariable;
	CButton	m_InvokeAction;
	CButton	m_StartDiscovery;
	CButton	m_DeviceProperties;
	CComboBox	m_ServiceCombo;
	CComboBox	m_FindCombo;
	CComboBox	m_DeviceCombo;
	CButton	m_CloseApp;
	CEdit	m_ActionOutArgument;
	CEdit	m_ActionInArgument;
	CButton m_FindByType;
	CButton m_FindByUDN;
	CButton m_AsyncFind;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenericUCPDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	virtual BOOL PreTranslateMessage(MSG *);
// Implementation
protected:
	HICON m_hIcon;
		
	// Generated message map functions
	//{{AFX_MSG(CGenericUCPDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnAppAbout();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnFindByType();
	afx_msg void OnDiscoveryClicked();
	afx_msg void OnFindbyUDN();
	afx_msg void OnAsyncFind();
	afx_msg void OnSelEndOkComboDevice();
	afx_msg void OnSelEndOkComboService();
	afx_msg void OnDevpropClick();
	afx_msg void OnQueryVariable();
	afx_msg void OnInvokeAction();
	afx_msg void OnCloseClick();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void ProcessFindByType(BSTR bstrSearchType);
	void ProcessFindByUDN(BSTR bstrSearchType);
	void ProcessAsyncFind(BSTR bstrSearchType);

	int iNumberOfInArguments(TCHAR *tszString);
	void StopAsyncFindIfStarted();
	void InitializeUDNList();
	BOOL fFileExists(LPTSTR ptszFileName);
	void InitializeDevTypeArray();
	void ClearAllDataStructures(BOOL fClearFindCombo);

	void ClearDeviceCombo();
	void ClearServiceCombo();

	//Private variables
	IUPnPDeviceFinder* m_pDeviceFinder;

	LONG	m_lAsyncFindHandle;
	BOOL	m_fAsyncFindRunning;
	CStringArray m_astrDevType;


	//Callback object pointers
	IUPnPDeviceFinderCallback *m_pDeviceFinderCallback;
	IUPnPServiceCallback *m_pServiceCallback;

public:
	void AddDevice(IUPnPDevice *pDevice);
	void RemoveDevice(BSTR bstrUDN);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GenericUCPDLG_H__06D8F92D_3842_4BB8_9B3B_39AF51C4070A__INCLUDED_)



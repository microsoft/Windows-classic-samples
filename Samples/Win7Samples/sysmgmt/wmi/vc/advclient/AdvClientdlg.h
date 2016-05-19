// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  AdvClientDlg.h
//
// Description:
//	This file declares the CAdvClientDlg dialog class. It is the main
//		dialog for the tutorial.
// 
// Part of: WMI Tutorial #1.
//
// Used by: CAdvClientleApp::InitInstance().
//
// History:
//
// **************************************************************************

#if !defined(AFX_HMMENUMDLG_H__6AC7FBF9_FE04_11D0_AD84_00AA00B8E05A__INCLUDED_)
#define AFX_HMMENUMDLG_H__6AC7FBF9_FE04_11D0_AD84_00AA00B8E05A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <wbemcli.h>     // WMI interface declarations
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CAdvClientDlg dialog
class CAsyncQuerySink;

class CAdvClientDlg : public CDialog
{
// Construction
public:
	CAdvClientDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CAdvClientDlg();

// Dialog Data
	//{{AFX_DATA(CAdvClientDlg)
	enum { IDD = IDD_SAMPLE_DIALOG };
	CButton	m_diskDescriptions;
	CListBox	m_eventList;
	CButton	m_perm;
	CButton	m_temp;
	CButton	m_addEquipment;
	CButton	m_enumServicesAsync;
	CButton	m_enumServices;
	CButton	m_enumDisks;
	CButton	m_diskDetails;
	CButton	m_connect;
	CListBox	m_outputList;
	CString	m_namespace;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAdvClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnAddEquipment();
	afx_msg void OnConnect();
	afx_msg void OnEnumdisks();
	afx_msg void OnEnumservices();
	afx_msg void OnDiskdetails();
	afx_msg void OnEnumservicesasync();
	afx_msg void OnRegPerm();
	afx_msg void OnRegTemp();
	virtual void OnCancel();
	afx_msg void OnDiskPropsDescriptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

	BOOL m_regPerm, m_regTemp;

	CAsyncQuerySink *m_pQueryCallback;
	IWbemServices *m_pIWbemServices;
	IWbemServices *m_pOfficeService;	//this gets marshalled to the event thread

	BOOL EnsureOfficeNamespace(void);
	BOOL CheckOfficeNamespace(void);

	HRESULT AssociateToMachine(IWbemClassObject *pEquipInst);
	HRESULT GetCompSysRef(VARIANT *v);

	// event related.
    static DWORD WINAPI ThreadProc(LPVOID lpParameter);

	IStream * m_pStream;

	HANDLE m_ptrReady;		// the thread has done the work.
	HANDLE m_stopThread;	// telling the thread to stop.
    CRITICAL_SECTION m_critSect;
    DWORD m_threadId;
    HANDLE m_thread;

	void StopThread();
	BOOL PermRegistered();
	BOOL OnTempRegister();
	void OnTempUnregister();
	BOOL OnPermRegister();
	void OnPermUnregister();

	static STDMETHODIMP DisplayEvents(CAdvClientDlg *pThis, LONG lObjectCount,  IWbemClassObject **ppObjArray);

	// SampleViewer.mof 'creates' the consumer.
	BOOL GetConsumer(IWbemClassObject **pConsumer);
	BOOL AddFilter(IWbemClassObject **pFilter);
	BOOL AddBinding(IWbemClassObject *pConsumer, 
					IWbemClassObject *pFilter);

};
LPWSTR ValueToString(VARIANT *pValue, WCHAR **pbuf);
LPCTSTR ErrorString(SCODE sc);


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HMMENUMDLG_H__6AC7FBF9_FE04_11D0_AD84_00AA00B8E05A__INCLUDED_)

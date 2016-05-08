// APIDoc.h : interface of the CAPIDoc class
//
//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation

/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_APIDOC_H__C090CEF0_303B_11D1_8310_00C04FBEFCDA__INCLUDED_)
#define AFX_APIDOC_H__C090CEF0_303B_11D1_8310_00C04FBEFCDA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <wtsapi32.h>

#define SERVERNAME_MAXSTRLEN 255

class CAPIDoc : public CDocument
{
protected: // create from serialization only
	CAPIDoc();
	DECLARE_DYNCREATE(CAPIDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAPIDoc)
	public:
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetSessionID(DWORD p) {sessionID=p;};
	DWORD GetSessionID() {return sessionID;};
	LPSTR GetServerName() {return serverName;};
	void SetServerName(TCHAR* temp) {_tcscpy_s(serverName, SERVERNAME_MAXSTRLEN, temp);};
	void ReadTrustedDomains();
	DWORD count;
	PWTS_SERVER_INFO pServerInfo;
	DWORD sessionID;
	TCHAR* serverName;
	LPSTR domainName;
	virtual ~CAPIDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAPIDoc)
	afx_msg void OnRunTest();
	afx_msg void OnProcess();
	afx_msg void OnSession();
	afx_msg void OnShutdownSystem();
	afx_msg void OnSendMessage();
	afx_msg void OnWaitEvent();
	afx_msg void OnUserconfig();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	char m_ServerName[70];
};

extern CAPIDoc *g_pDoc;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APIDOC_H__C090CEF0_303B_11D1_8310_00C04FBEFCDA__INCLUDED_)

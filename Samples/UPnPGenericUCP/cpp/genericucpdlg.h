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
    public IUPnPDeviceFinderCallback
{
    private: 
        CGenericUCPDlg* m_pGenericUCPDlg;
        ULONG m_cRef;
    public:
    CDeviceFinderCallback();
    ~CDeviceFinderCallback();

    HRESULT __stdcall QueryInterface(_In_ const IID& iid, _Out_ void** ppv);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();

    // *** IUPnPDeviceFinderCallback methods ***
    STDMETHOD(DeviceAdded)(_In_ LONG lFindData, _In_ IUPnPDevice * pDevice);
    STDMETHOD(DeviceRemoved)(_In_ LONG lFindData, _In_ BSTR bstrUDN);
    STDMETHOD(SearchComplete)(_In_ LONG lFindData);
    
    // Other functions
    static CDeviceFinderCallback * Create();
    void SetDialogPointer(CGenericUCPDlg* pDialog);

};

// Service Callback 
class CServiceCallback : 
    public IUPnPServiceCallback
{
private: 
        CGenericUCPDlg* m_pGenericUCPDlg;
        ULONG m_cRef;
public:

    CServiceCallback();
    ~CServiceCallback();

    HRESULT __stdcall QueryInterface(_In_ const IID& iid, _Out_ void** pv);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();


    // *** IUPnPServiceCallback methods ***
    STDMETHOD (StateVariableChanged)(_In_ IUPnPService *pus, _In_ LPCWSTR pcwszStateVarName, _In_ VARIANT varValue);
    STDMETHOD (ServiceInstanceDied)(_In_ IUPnPService *pus);

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
    CButton m_AsynchControl;
    CButton m_DelaySubscription;
    CButton m_Subscribe;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CGenericUCPDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    //}}AFX_VIRTUAL

    virtual BOOL PreTranslateMessage(_In_ MSG *);
// Implementation
protected:
    HICON m_hIcon;
    	
    // Generated message map functions
    //{{AFX_MSG(CGenericUCPDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(_In_ UINT nID, _In_ LPARAM lParam);
    afx_msg void OnAppAbout();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnViewSCPD();
    afx_msg void OnFindByType();
    afx_msg void OnDiscoveryClicked();
    afx_msg void OnFindbyUDN();
    afx_msg void OnAsyncFind();
    afx_msg void OnSelEndOkComboDevice();
    afx_msg void OnSelEndOkComboService();
    afx_msg void OnDevpropClick();
    afx_msg void OnClickedViewSCPD();
    afx_msg void OnSubscribe();
    afx_msg void OnQueryVariable();
    afx_msg void OnInvokeAction();
    afx_msg void OnCloseClick();
    afx_msg void OnCheckBoxClick();
    afx_msg void OnClose();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void OnInvokeSyncAction(_In_ BSTR bstrActionName, _In_ int iCurrentService, _In_ VARIANT vaActionArgs);
    void OnInvokeAsyncAction(_In_ BSTR bstrActionName, _In_ int iCurrentService, _In_ VARIANT vaActionArgs);
    void OnQueryVariableSync(_In_ BSTR bstrVariableName, _In_ int iCurrentService);
    void OnQueryVariableAsync(_In_ BSTR bstrVariableName, _In_ int iCurrentService);
    void OnSubscribeSync();
    void OnSubscribeAsync();
    void ProcessFindByType(_In_ BSTR bstrSearchType);
    void ProcessFindByUDN(_In_ BSTR bstrSearchType);
    void ProcessAsyncFind(_In_ BSTR bstrSearchType);

    int iNumberOfInArguments(_In_  LPTSTR tszString);
    void StopAsyncFindIfStarted();
    void InitializeUDNList();
    BOOL fFileExists(_In_  LPTSTR ptszFileName);
    void InitializeDevTypeArray();
    void ClearAllDataStructures(_In_ BOOL fClearFindCombo);

    void ClearDeviceCombo();
    void ClearServiceCombo();

    //Private variables
    IUPnPDeviceFinder* m_pDeviceFinder;

    LONG	m_lAsyncFindHandle;
    HANDLE  m_hCloseEvent;
    ULONG64 m_ullRequestID;
    BOOL	m_fAsyncFindRunning;
    CStringArray m_astrDevType;


    //Callback object pointers
    IUPnPDeviceFinderCallback *m_pDeviceFinderCallback;
    IUPnPServiceCallback *m_pServiceCallback;

public:
    void AddDevice(_In_ IUPnPDevice *pDevice);
    void RemoveDevice(_In_ BSTR bstrUDN);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GenericUCPDLG_H__06D8F92D_3842_4BB8_9B3B_39AF51C4070A__INCLUDED_)



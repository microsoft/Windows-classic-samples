// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// HostedNetworkDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CHostedNetworkDlg dialog
class CHostedNetworkDlg : public CDialog
{
// Construction
public:
    CHostedNetworkDlg(CWnd* pParent = NULL);        // standard constructor

    ~CHostedNetworkDlg();                           // standard descructor

// Dialog Data
    enum { IDD = IDD_HOSTEDNETWORK_DIALOG };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

private:
    // private data members

    // ICS manager
    CIcsManager * m_IcsMgr;

    // Wlan manager
    CWlanManager m_WlanMgr;

    // Hosted network notification sink
    CNotificationSink * m_NotificationSink;

    bool m_fProcessNotifications;

    // List of WLAN devices
    CRefObjList<CWlanDevice *> m_WlanDeviceList;

    // List of connections (adapters)
    CRefObjList<CIcsConnectionInfo *> m_ConnectionList;

    // whether softAP will be started with full ICS
    bool m_IcsNeeded;

    // ICS enabled
    bool m_IcsEnabled;

    // Hosted network started
    bool m_HostedNetworkStarted;

    // GUID of hosted network adapter
    GUID m_HostedNetworkGuid;

    // messages for start/stop hosted network
    WCHAR m_strStartHostedNetwork[256];
    WCHAR m_strStopHostedNetwork[256];

    // Current name and key for the hosted network
    CAtlString m_CurrentName;
    CAtlString m_CurrentKey;

    // whether the app is running with admin privilege
    BOOL m_bIsAdmin;

    // whether ICS is allowed 
    bool m_bIsICSAllowed;

    // private methods
    HRESULT InitWlan();

    HRESULT InitIcs();
    void DeinitIcs();

    void ProcessNotifications();

    HRESULT StartHostedNetwork();

    HRESULT StopHostedNetwork();

    HRESULT EnableIcs();

    HRESULT DisableIcs();

    void OnOK() {};

    void OnDeviceAdd(CWlanDevice *);

    void OnDeviceRemove(CWlanDevice *);

    void OnDeviceUpdate(CWlanDevice *);

    void OnHostedNetworkStarted();

    void OnHostedNetworkStopped();

    void OnHostedNetworkNotAvailable();

    void OnHostedNetworkAvailable();

    void GetHostedNetworkInfo();

    void GetIcsInfo();

    void UpdateIcsConnectionList();

    HRESULT StartIcsIfNeeded();

    bool StopIcsIfNeeded();

    void GetWlanDeviceInfo();

    void PostNotification(LPCWSTR msg);

    void PostDeviceNotification(CWlanDevice *, int);

    void ClearNotifications() {m_NotificationOutputList.ResetContent();};

    void PostErrorMessage(LPCWSTR msg) {::MessageBox(m_hWnd, msg, NULL, MB_ICONERROR|MB_OK);};

    BOOL IsUserAdmin();

    bool IsICSAllowed();

    bool CheckValidSSIDKeyLen();

    // The following three functions update the device list
    void AddWlanDevice(CWlanDevice *);

    void RemoveWlanDevice(CWlanDevice *);

    void UpdateWlanDevice(CWlanDevice *);

    void DisableAllControls();

    void EnableAllControls();


// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg void OnClose();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedCheckEnableIcs();
    afx_msg void OnBnClickedButtonHostednetwork();
    afx_msg void OnBnClickedButtonClearNotifications();
    afx_msg void OnEnKillfocusEditName();

    LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:
    // Edit box for hosted network name
    CEdit m_NameEdit;
    // Edit box for host network security key
    CEdit m_KeyEdit;
    // Checkbox for enabling/disabling ICS
    CButton m_EnableIcsCheck;
    // Button to start/stop hosted network
    CButton m_HostedNetworkButton;
    // ComboBox for the list of connections that support ICS
    CComboBox m_ConnectionComboBox;
    // List box of notification output
    CListBox m_NotificationOutputList;
    // ListCtrl for WLAN devices
    CListCtrl m_DeviceListCtrl;
public:
    afx_msg void OnEnKillfocusEditKey();
    CStatic m_HostedNetworkStatusTxt;
};

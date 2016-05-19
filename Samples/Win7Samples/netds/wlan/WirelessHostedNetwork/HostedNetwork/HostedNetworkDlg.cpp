// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// HostedNetworkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HostedNetwork.h"
#include "HostedNetworkDlg.h"

#define MAX_KEY_LEN  63

/*
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
*/

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CHostedNetworkDlg dialog

CHostedNetworkDlg::CHostedNetworkDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CHostedNetworkDlg::IDD, pParent),
      m_NotificationSink(NULL),
      m_fProcessNotifications(false),
      m_IcsNeeded(false),
      m_IcsEnabled(false),
      m_HostedNetworkStarted(false),
      m_IcsMgr(NULL)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CHostedNetworkDlg::~CHostedNetworkDlg()
{
    // Unregister notification
    if (m_NotificationSink != NULL)
    {
        m_WlanMgr.UnadviseHostedNetworkNotification();

        delete m_NotificationSink;

        m_NotificationSink = NULL;
    }

    DeinitIcs();

    _ASSERT(m_IcsMgr == NULL);
}

void 
CHostedNetworkDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_NAME, m_NameEdit);
    DDX_Control(pDX, IDC_EDIT_KEY, m_KeyEdit);
    DDX_Control(pDX, IDC_CHECK_ENABLE_ICS, m_EnableIcsCheck);
    DDX_Control(pDX, IDC_BUTTON_HOSTEDNETWORK, m_HostedNetworkButton);
    DDX_Control(pDX, IDC_COMBO1, m_ConnectionComboBox);
    DDX_Control(pDX, IDC_LIST_NOTIFICATON, m_NotificationOutputList);
    DDX_Control(pDX, IDC_LIST_DEVICE, m_DeviceListCtrl);
    DDX_Control(pDX, IDC_STATIC_NAME2, m_HostedNetworkStatusTxt);
}

BEGIN_MESSAGE_MAP(CHostedNetworkDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_CHECK_ENABLE_ICS, CHostedNetworkDlg::OnBnClickedCheckEnableIcs)
    ON_BN_CLICKED(IDC_BUTTON_HOSTEDNETWORK, CHostedNetworkDlg::OnBnClickedButtonHostednetwork)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_NOTIFICATIONS, CHostedNetworkDlg::OnBnClickedButtonClearNotifications)
//    ON_EN_KILLFOCUS(IDC_EDIT_NAME, CHostedNetworkDlg::OnEnKillfocusEditName)
//    ON_EN_KILLFOCUS(IDC_EDIT_KEY, CHostedNetworkDlg::OnEnKillfocusEditKey)
END_MESSAGE_MAP()


// CHostedNetworkDlg message handlers

BOOL 
CHostedNetworkDlg::OnInitDialog()
{
    BOOL fRetCode = TRUE;

    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CAtlString strAboutMenu;
        fRetCode = strAboutMenu.LoadString(IDS_ABOUTBOX);

        if (!fRetCode)
        {
            BAIL();
        }

        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // set the length limit for SSID/key input
    m_NameEdit.LimitText(DOT11_SSID_MAX_LENGTH);
    m_KeyEdit.LimitText(MAX_KEY_LEN);
    m_HostedNetworkStatusTxt.SetWindowText(L"The hosted network is idle");

    // Initialize device image list
    CWlanDevice::InitDeviceImageList(AfxGetApp());

    CImageList * pImageList = CWlanDevice::GetDeviceImageList();

    // Set image list for the device listctrl
    m_DeviceListCtrl.SetImageList(pImageList, LVSIL_NORMAL);

    // Load the messages for start/stop hosted network
    LoadString (
        GetModuleHandle(NULL),
        IDS_START_HOSTED_NETWORK,
        m_strStartHostedNetwork,
        256
    );

    LoadString (
        GetModuleHandle(NULL),
        IDS_STOP_HOSTED_NETWORK,
        m_strStopHostedNetwork,
        256
    );

    // Following code is to initialize ICS manager

    // Step 1: check if the app is running under admin privilege
    // if not, use has no access to ICS settings,
    // disable the ICS settings and post message
    m_bIsAdmin = IsUserAdmin();
    m_bIsICSAllowed = IsICSAllowed();

    if (!m_bIsAdmin || !m_bIsICSAllowed)
    {    
        // Disable ICS controls
        // For Enable ICS
        m_EnableIcsCheck.EnableWindow(FALSE);
        // For Connection List
        m_ConnectionComboBox.EnableWindow(FALSE);

        m_IcsNeeded = FALSE;

        if (!m_bIsAdmin)
        {
            PostErrorMessage(L"The application is not running with Admin privilege, user will not have access to ICS controls.");
        }
        else
        {
            PostErrorMessage(L"ICS is disabled by domain administrator, user will not have access to ICS controls.");
        }

        m_NameEdit.SetFocus();
    }

    // Step 2: if user has admin privilege and ICS is allowed in the domain,
    // initialize ICS manager
    if (m_bIsAdmin && m_bIsICSAllowed)
    {
        // Initialize ICS
        if (FAILED(InitIcs()))
        {
            // Post a notificatoin
            PostNotification(L"Failed to initialize ICS manager.");
            
            fRetCode = FALSE;
            BAIL();
        }

        // Post a notificatoin
        PostNotification(L"Successfully initialized ICS manager.");

        // Get connection info
        m_IcsMgr->GetIcsConnections(m_ConnectionList);
    }

    // Following code is to Initialize WLAN
    if (FAILED(InitWlan()))
    {
        // Post a notificatoin
        PostNotification(L"Failed to initialize WiFi manager.");

        fRetCode = FALSE;
        BAIL();
    }

    // Post a notificatoin
    PostNotification(L"Successfully initialized WiFi manager.");

    // Get hosted network info
    GetHostedNetworkInfo();

    // Get ICS info. Must be called after GetHostedNetworkInfo().
    if (m_bIsAdmin && m_bIsICSAllowed)
    {
        ASSERT(m_IcsMgr);
        GetIcsInfo();
    }

    // During initialization: Set the app at the mode of start hosted network
    // regardless whether the hostednetwork has been started or not
    // This is to prevent the case that a hosted network started by other apps
    // cannot be stopped here.

    m_HostedNetworkButton.SetWindowTextW(m_strStartHostedNetwork);
    // Name
    m_NameEdit.EnableWindow();
    // Key
    m_KeyEdit.EnableWindow();
    // Set state
    m_HostedNetworkStarted = false;

    m_fProcessNotifications = true;

error:
    return fRetCode;
}

void 
CHostedNetworkDlg::OnSysCommand(
    UINT nID, 
    LPARAM lParam
    )
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void 
CHostedNetworkDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

void
CHostedNetworkDlg::OnClose()
{
    m_fProcessNotifications = false;
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR 
CHostedNetworkDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void 
CHostedNetworkDlg::OnBnClickedCheckEnableIcs()
{
    // TODO: Add your control notification handler code here
    if (m_EnableIcsCheck.GetCheck() == BST_CHECKED)
    {
        m_IcsNeeded = true;
        // Enable connection list
        m_ConnectionComboBox.EnableWindow();
    }
    else
    {
        m_IcsNeeded = false;
        // Disable connection list
        m_ConnectionComboBox.EnableWindow(FALSE);
    }
}

void 
CHostedNetworkDlg::OnBnClickedButtonHostednetwork()
{
    HRESULT     hr            = S_OK;
    HRESULT     hrPrintError  = S_OK;
    TCHAR       sErrorMsg[256];
    size_t      errorSize     = 256;
    LPCTSTR     sErrorFormat  = TEXT("%s. Error code %x.");
    LPCTSTR     sOperation;
    bool        bValidSSIDKey = false;

    // Check if the lengths of SSID and key are valid
    bValidSSIDKey = CheckValidSSIDKeyLen();
    if (!bValidSSIDKey)
    {
        return;
    }

    if (m_HostedNetworkStarted)
    {
        hr = StopHostedNetwork();
        sOperation = TEXT("Failed to stop the hostednetwork");
    }
    else
    {
        hr = StartHostedNetwork();
        sOperation = TEXT("Failed to start the hosted network");
    }
    
    if (FAILED(hr))
    {
        // Post an error message
        hrPrintError = StringCchPrintf(sErrorMsg, errorSize, sErrorFormat, sOperation, hr);
        if (hrPrintError != S_OK)
        {
            PostErrorMessage(sOperation);
        }
        else
        {
            PostErrorMessage(sErrorMsg);
        }
    }
}

LRESULT 
CHostedNetworkDlg::WindowProc(
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    if (WM_NEW_HN_NOTIFICATION == message)
    {
        if (m_fProcessNotifications)
        {
            ProcessNotifications();
        }
        return 0;
    }
    else
    {
        //
        // Default message processing
        //
        return CDialog::WindowProc(message, wParam, lParam);
    }
}

HRESULT 
CHostedNetworkDlg::InitWlan()
{
    HRESULT hr                      = S_OK;
    bool    bIsHostedNetworkStarted = FALSE;

    //
    // Create notification sink
    //
    _ASSERT(m_NotificationSink == NULL);

    m_NotificationSink = new(std::nothrow) CNotificationSink(m_hWnd);
    if (NULL == m_NotificationSink)
    {
        hr = E_OUTOFMEMORY;
        BAIL();
    }

    // 
    // Initialize WLAN manager
    //
    hr = m_WlanMgr.Init();
    BAIL_ON_FAILURE(hr);

    //
    // Set notification sink
    //
    hr = m_WlanMgr.AdviseHostedNetworkNotification(m_NotificationSink); 
    BAIL_ON_FAILURE(hr);

    hr = m_WlanMgr.IsHostedNetworkStarted(bIsHostedNetworkStarted); 
    BAIL_ON_FAILURE(hr);

    if (bIsHostedNetworkStarted)
    {
        m_HostedNetworkStatusTxt.SetWindowText(L"The hosted network is active");
    }

error:
    if (S_OK != hr && m_NotificationSink != NULL)
    {
        delete m_NotificationSink;
        m_NotificationSink = NULL;
    }

    return hr;
}

//
// Process all notifications in the queue.
// No lock is required because calls are serialized.
//
void 
CHostedNetworkDlg::ProcessNotifications()
{
    CHostedNetworkNotification * pNotification = NULL;
    CWlanDevice * pDevice = NULL;
    // 
    // Get the first notification
    //
    pNotification = m_NotificationSink->GetNextNotification();

    // pNotification could be NULL
    
    while (pNotification != NULL)
    {
        pDevice = (CWlanDevice *)pNotification->GetNotificationData();

        switch(pNotification->GetNotificationType())
        {
        case CHostedNetworkNotification::HostedNetworkNotAvailable:
            _ASSERT(NULL == pDevice);
            OnHostedNetworkNotAvailable();
            break;

        case CHostedNetworkNotification::HostedNetworkAvailable:
            _ASSERT(NULL == pDevice);
            OnHostedNetworkAvailable();
            break;
        
        case CHostedNetworkNotification::HostedNetworkStarted:
            _ASSERT(NULL == pDevice);
            PostNotification(L"Hosted network is now started.");
            m_HostedNetworkStatusTxt.SetWindowText(L"The hosted network is active");
            break;
        
        case CHostedNetworkNotification::HostedNetworkStopped:
            _ASSERT(NULL == pDevice);
            PostNotification(L"Hosted network is now stopped.");
            m_HostedNetworkStatusTxt.SetWindowText(L"The hosted network is idle");
            // Remove all devices from the device list
            m_DeviceListCtrl.DeleteAllItems();
            break;

        case CHostedNetworkNotification::DeviceAdd:
            _ASSERT(pDevice != NULL);
            if (pDevice != NULL)
            {
                OnDeviceAdd(pDevice);
            }
            break;

        case CHostedNetworkNotification::DeviceRemove:
            _ASSERT(pDevice != NULL);
            if (pDevice != NULL)
            {
                OnDeviceRemove(pDevice);
            }
            break;

        case CHostedNetworkNotification::DeviceUpdate:
            _ASSERT(pDevice != NULL);
            if (pDevice != NULL)
            {
                OnDeviceUpdate(pDevice);
            }
            break;
        }

        pNotification->Release();
        pNotification = NULL;

        //
        // Get the next notification
        //
        pNotification = m_NotificationSink->GetNextNotification();
    }
}

//
// Get hosted network info and set controls accordingly
//
void 
CHostedNetworkDlg::GetHostedNetworkInfo()
{
    HRESULT hr = S_OK;
    bool fStarted = false;

    //
    // Get hosted network name
    //
    hr = m_WlanMgr.GetHostedNetworkName(m_CurrentName);

    BAIL_ON_FAILURE(hr);

    // Get hosted network key
    hr = m_WlanMgr.GetHostedNetworkKey(m_CurrentKey);
    BAIL_ON_FAILURE(hr);

    // Get adapter GUID
    m_WlanMgr.GetHostedNetworkInterfaceGuid(m_HostedNetworkGuid);

    //
    // Check whether the hosted network is started or not
    //
    hr = m_WlanMgr.IsHostedNetworkStarted(fStarted);

    _ASSERT(S_OK == hr);
    BAIL_ON_FAILURE(hr);

    //
    // Update controls
    //

    // Set network name
    m_NameEdit.SetWindowTextW(m_CurrentName);
    
    // Set network key
    m_KeyEdit.SetWindowTextW(m_CurrentKey);

    // Set the button text
    m_HostedNetworkStarted = fStarted;
    m_HostedNetworkButton.SetWindowTextW(fStarted ? m_strStopHostedNetwork : m_strStartHostedNetwork);

    // Disable controls if needed
    if (fStarted)
    {
        // Name
        m_NameEdit.EnableWindow(FALSE);
        // Key
        m_KeyEdit.EnableWindow(FALSE);
        // Enable ICS
        m_EnableIcsCheck.EnableWindow(FALSE);
        // ICS connection list
        m_ConnectionComboBox.EnableWindow(FALSE);

        PostNotification(L"Hosted network has already started.");
    }

error:
    return;
}

void
CHostedNetworkDlg::GetWlanDeviceInfo()
{
    CRefObjList<CWlanStation *> stationList;

    if (SUCCEEDED(m_WlanMgr.GetStaionList(stationList)))
    {
        // add stations that are already connected
        while ( 0 != stationList.GetCount() )
        {
            CWlanStation* pStation = stationList.RemoveHead();
            
            // OnDeviceAdd(pStation);
            
            pStation->Release();

            pStation = NULL;
        }
    }
}

void
CHostedNetworkDlg::OnHostedNetworkStarted()
{
    // Post a notificatoin
    PostNotification(L"Started using hosted Network.");

    // Set state
    m_HostedNetworkStarted = true;

    // Get adapter GUID
    m_WlanMgr.GetHostedNetworkInterfaceGuid(m_HostedNetworkGuid);

    // Set button text and enable it
    m_HostedNetworkButton.SetWindowTextW(m_strStopHostedNetwork);
    m_HostedNetworkButton.EnableWindow();

    // SSID and key setting should be disabled since we cannot modify them now
    m_NameEdit.EnableWindow(FALSE);
    m_KeyEdit.EnableWindow(FALSE);
}

void
CHostedNetworkDlg::OnHostedNetworkStopped()
{
    if (m_HostedNetworkStarted)
    {
        // Post a notificatoin
        PostNotification(L"Stopped using hosted network.");

        // Stop ICS if needed
        StopIcsIfNeeded();

        // Set state
        m_HostedNetworkStarted = false;

        // Set button text
        m_HostedNetworkButton.SetWindowTextW(m_strStartHostedNetwork);

        EnableAllControls();
    }
}

void
CHostedNetworkDlg::OnHostedNetworkNotAvailable()
{
    // Post a notificatoin
    PostNotification(L"Hosted network is not available on current NIC(s).");
    m_HostedNetworkStatusTxt.SetWindowText(L"The hosted network is unavailable");

    // Set state
    m_HostedNetworkStarted = false;

    // Stop ICS if needed
    StopIcsIfNeeded();

    // Set button text and disable it -- we cannot enable hosted network now.
    m_HostedNetworkButton.SetWindowTextW(m_strStartHostedNetwork);
    m_HostedNetworkButton.EnableWindow(FALSE);
    
    // Enable ICS: We cannot do this since hosted network cannot be started
    m_EnableIcsCheck.EnableWindow(FALSE);    
    // Connection List
    m_ConnectionComboBox.EnableWindow(FALSE);
}

void
CHostedNetworkDlg::OnHostedNetworkAvailable()
{
    // Post a notificatoin
    PostNotification(L"Hosted network is available on current NIC(s).");
    
    m_HostedNetworkStatusTxt.SetWindowText(L"The hosted network is idle");

    // Set state
    m_HostedNetworkStarted = false;

    // Get hosted network info
    GetHostedNetworkInfo();

    if (m_bIsAdmin && m_bIsICSAllowed)
    {
        ASSERT(m_IcsMgr);
        // reset ICS to update ICS list
        m_IcsMgr->ResetIcsManager();

        // ICS list updated
        GetIcsInfo();
    }
    // Set button text
    m_HostedNetworkButton.SetWindowTextW(m_strStartHostedNetwork);
    EnableAllControls();
}

void
CHostedNetworkDlg::OnDeviceAdd(
    CWlanDevice * pDevice
    )
{
    _ASSERT(pDevice != NULL);

    if (pDevice != NULL)
    {
        // The device should not be in the list
        _ASSERT(!m_WlanDeviceList.IsInArray(pDevice));

        pDevice->AddRef();

        // Add the device to the list
        m_WlanDeviceList.AddTail(pDevice);

        // Update the device list view
        AddWlanDevice(pDevice);

        // Post a notification
        PostDeviceNotification(pDevice, CHostedNetworkNotification::DeviceAdd);
    }
}

void
CHostedNetworkDlg::OnDeviceRemove(
    CWlanDevice * pDevice
    )
{
    _ASSERT(pDevice != NULL);
    CWlanDevice * pRemovedDevice = NULL;

    //
    // Find the device and remove it from the device list
    //
    for (size_t i = 0; i < m_WlanDeviceList.GetCount(); i++)
    {
        POSITION pos = m_WlanDeviceList.FindIndex(i);
        CWlanDevice * pTmpDevice = m_WlanDeviceList.GetAt(pos);
        if (*pTmpDevice == *pDevice)
        {
            //
            // Found the device, remove it from the list
            //
            m_WlanDeviceList.RemoveAt(pos);
            pRemovedDevice = pTmpDevice;
            break;
        }
    }

    if (pRemovedDevice != NULL)
    {
        // Update the device list view
        RemoveWlanDevice(pRemovedDevice);

        // Post a notification
        PostDeviceNotification(pDevice, CHostedNetworkNotification::DeviceRemove);

        pRemovedDevice->Release();
        pRemovedDevice = NULL;
    }
}

void
CHostedNetworkDlg::OnDeviceUpdate(
    CWlanDevice * pDevice
    )
{
    _ASSERT(pDevice != NULL);
    CWlanDevice * pRemovedDevice = NULL;
    POSITION pos;

    //
    // Find the device and remove it from the device list
    //
    for (size_t i = 0; i < m_WlanDeviceList.GetCount(); i++)
    {
        pos = m_WlanDeviceList.FindIndex(i);
        CWlanDevice * pTmpDevice = m_WlanDeviceList.GetAt(pos);
        if (*pTmpDevice == *pDevice)
        {
            //
            // Found the device, update it with the new object
            //
            pDevice->AddRef();
            m_WlanDeviceList.SetAt(pos, pDevice);
            pRemovedDevice = pTmpDevice;
            break;
        }
    }

    if (pRemovedDevice != NULL)
    {
        // Update the device list view
        UpdateWlanDevice(pDevice);

        // Post a notification
        PostDeviceNotification(pDevice, CHostedNetworkNotification::DeviceUpdate);

        pRemovedDevice->Release();
        pRemovedDevice = NULL;
    }
}

HRESULT 
CHostedNetworkDlg::StartHostedNetwork()
{
    HRESULT     hr                    = S_OK;
    CString     strName;
    CAtlString  name;
    CString     strKey;
    CAtlString  key;
    bool        bHostedNetworkStarted = false;
    bool        bICSStarted           = false;

    DisableAllControls();

    // Set name
    m_NameEdit.GetWindowTextW(strName);
    name = strName;
    
    if (m_CurrentName != name)
    {
        // Set name only when needed
        hr = m_WlanMgr.SetHostedNetworkName(name);
        BAIL_ON_FAILURE(hr);
        m_CurrentName = name;
    }

    // Set key
    m_KeyEdit.GetWindowTextW(strKey);
    key = strKey;

    if (m_CurrentKey != key)
    {
        // Set key only when needed
        hr = m_WlanMgr.SetHostedNetworkKey(key);
        BAIL_ON_FAILURE(hr);
        m_CurrentKey = key;
    }

    // Cache the current ICS-enabled interfaces
    if (m_IcsNeeded)
    {
        m_IcsMgr->CacheICSIntfIndex();
        
        // Start ICS if:
        // 1. User has selected that softAP starts with full ICS, and
        // 2. the current ICS setting is different from the new setting
        hr = StartIcsIfNeeded();
        bICSStarted = true;

        // Force stop the currently running hosted network,
        // if there is any
        // this step is taken no matter  whether the start ICS 
        // succeeds or fails
        m_WlanMgr.IsHostedNetworkStarted(bHostedNetworkStarted);
        if (bHostedNetworkStarted)
        {
            m_WlanMgr.ForceStopHostedNetwork();
        }

        // if start ICS fails, bail out
        BAIL_ON_FAILURE(hr);
    }

    // Start hosted network
    hr = m_WlanMgr.StartHostedNetwork();
    
    // if start hosted network fails, bail out
    BAIL_ON_FAILURE(hr);

    m_WlanMgr.IsHostedNetworkStarted(bHostedNetworkStarted);
    OnHostedNetworkStarted();

error:
    if (hr != S_OK)
    {
        if (m_IcsNeeded && bICSStarted)
        {
            // restore the previous ICS settings
            m_IcsMgr->EnableICSonCache();
        }
        EnableAllControls();
    }
    return hr;
}

HRESULT 
CHostedNetworkDlg::StopHostedNetwork()
{
    HRESULT hr = S_OK;
    bool bIcsStopped = false;
    
    // Disable the button of Start/Stop hosted network
    m_HostedNetworkButton.EnableWindow(FALSE);

    // Stop full ICS if ICS is needed
    bIcsStopped = StopIcsIfNeeded();

    // If a previous running ICS is stopped
    // force stop the hosted network
    // otherwise, stop using the hosted network
    if (bIcsStopped)
    {
        hr = m_WlanMgr.ForceStopHostedNetwork();
    }
    else
    {
        hr = m_WlanMgr.StopHostedNetwork();
    }

    BAIL_ON_FAILURE(hr);

    OnHostedNetworkStopped();
error:
    return hr;
}

void
CHostedNetworkDlg::GetIcsInfo()
{
    // Get the list of ICS connections
    m_IcsMgr->GetIcsConnections(m_ConnectionList);

    // If the hosted network is started, check whether ICS is enabled for hosted network
    if (m_HostedNetworkStarted)
    {
        for (size_t i = 0; i < m_ConnectionList.GetCount(); i++)
        {
            CIcsConnectionInfo * pConn = m_ConnectionList.GetAt(m_ConnectionList.FindIndex(i));
            if (*pConn == m_HostedNetworkGuid)
            {
                m_IcsEnabled = (pConn->m_Supported && pConn->m_SharingEnabled && pConn->m_Private);
                break;
            }
        }
    }

    // Update control
    if (m_IcsEnabled)
    {
        m_IcsNeeded = true;
    }
    m_EnableIcsCheck.SetCheck(m_IcsNeeded ? BST_CHECKED : BST_UNCHECKED);

    // Update connection combobox
    UpdateIcsConnectionList();

    if (!m_IcsNeeded)
    {
        // Disable connection list
        m_ConnectionComboBox.EnableWindow(FALSE);
    }
    else
    {
        if (m_IcsEnabled)
        {
            PostNotification(L"ICS has already been started.");
        }
    }
}

void
CHostedNetworkDlg::UpdateIcsConnectionList()
{
    int nConnections = 0;

    // Empty the ICS connection combo box.
    for (int i = m_ConnectionComboBox.GetCount() - 1; i >= 0; i--)
    {
       m_ConnectionComboBox.DeleteString(i);
    }

    for (size_t i = 0; i < m_ConnectionList.GetCount(); i++)
    {
        CIcsConnectionInfo * pConn = m_ConnectionList.GetAt(m_ConnectionList.FindIndex(i));
        if (pConn->m_Supported && !(*pConn == m_HostedNetworkGuid))
        {
            //
            // Don't add a connection if it doesn't support ICS or it is the hosted network connection
            //
            nConnections++;

            int index = m_ConnectionComboBox.AddString(pConn->m_Name);
            
            // Set data pointer for the item.
            m_ConnectionComboBox.SetItemDataPtr(index, pConn);

            if (0 == index)
            {
                // Set the current selection to the first one by default
                m_ConnectionComboBox.SetCurSel(index);
            }

            if (m_HostedNetworkStarted && m_IcsNeeded && pConn->m_SharingEnabled && pConn->m_Public)
            {
                // Set the current selection
                m_ConnectionComboBox.SetCurSel(index);
            }
        }
    }
}

HRESULT
CHostedNetworkDlg::InitIcs()
{
    HRESULT hr = S_OK;
    CIcsManager * pIcsMgr = NULL;
    bool fDeinitCom = false;
    bool fDeinitNSMod = false;

    _ASSERT(NULL == m_IcsMgr);

    // initialize NS mod
    hr = NSModInit();
    BAIL_ON_FAILURE(hr);

    fDeinitNSMod = true;

    // initialize COM
    hr = ::CoInitializeEx(
            NULL,
            COINIT_MULTITHREADED
            );
    BAIL_ON_FAILURE(hr);

    fDeinitCom = true;

    // create ICS manager
    pIcsMgr = new(std::nothrow) CIcsManager();
    if (NULL == pIcsMgr)
    {
        hr = E_OUTOFMEMORY;
        BAIL();
    }

    // initialize ICS manager
    hr = pIcsMgr->InitIcsManager();
    BAIL_ON_FAILURE(hr);

    // Everything is fine
    // COM and NSMod are deinitialized later
    fDeinitCom = false;
    fDeinitNSMod = false;

    m_IcsMgr = pIcsMgr;
    pIcsMgr = NULL;

error:
    if (pIcsMgr != NULL)
    {
        delete pIcsMgr;
        pIcsMgr = NULL;
    }

    if (fDeinitCom)
    {
        ::CoUninitialize();
    }

    if (fDeinitNSMod)
    {
        NSModDeinit();
    }

    return hr;
}

void
CHostedNetworkDlg::DeinitIcs()
{
    if (m_IcsMgr != NULL)
    {
        //
        // ICS was successfully initialized.
        //
        delete m_IcsMgr;
        m_IcsMgr = NULL;

        ::CoUninitialize();
        NSModDeinit();
    }
}

void 
CHostedNetworkDlg::OnBnClickedButtonClearNotifications()
{
    // TODO: Add your control notification handler code here
    ClearNotifications();
}

HRESULT 
CHostedNetworkDlg::StartIcsIfNeeded()
{
    HRESULT hr            = S_OK;
    HRESULT hrPrintError  = S_OK;
    TCHAR   sErrorMsg[256];
    size_t  errorSize     = 256;
    LPCTSTR sErrorFormat  = TEXT("Failed to enable ICS. Error code %x.");

    _ASSERT(!m_IcsEnabled);

    if (!m_IcsNeeded)
    {
        // No need to start ICS
        BAIL();
    }

    // Get the public connecton
    CIcsConnectionInfo * pConn = (CIcsConnectionInfo *)m_ConnectionComboBox.GetItemDataPtr(m_ConnectionComboBox.GetCurSel());

    if (pConn != NULL)
    {
        // Start ICS
        hr = m_IcsMgr->EnableIcs(pConn->m_Guid, m_HostedNetworkGuid);

        if (hr != S_OK)
        {
            // reset ICS manager
            hr = m_IcsMgr->ResetIcsManager();

            if (S_OK == hr)
            {
                // try it again
                hr = m_IcsMgr->EnableIcs(pConn->m_Guid, m_HostedNetworkGuid);
            }
        }

        if (S_OK == hr)
        {
            PostNotification(L"Successfully enabled ICS.");
            m_IcsEnabled = true;
        }
        else
        {
            hrPrintError = StringCchPrintf(sErrorMsg, errorSize, sErrorFormat, hr);
            if (hrPrintError != S_OK)
            {
                PostErrorMessage(L"Failed to enable ICS.");
                PostNotification(L"Failed to enable ICS.");
            }
            else
            {
                PostErrorMessage(sErrorMsg);
                PostNotification(sErrorMsg);
            }
        }
    }

error:
    return hr;
}

bool
CHostedNetworkDlg::StopIcsIfNeeded()
{
    bool bICSStopped = false;
    if (m_IcsEnabled && m_IcsNeeded)
    {
        _ASSERT(m_IcsMgr != NULL);

        m_IcsMgr->DisableIcsOnAll();

        PostNotification(L"Successfully disabled ICS.");

        m_IcsEnabled = false;
        bICSStopped = true;
    }
    return bICSStopped;
}

void
CHostedNetworkDlg::AddWlanDevice(
    CWlanDevice *pDevice
    )
{
    _ASSERT(pDevice != NULL);

    CAtlString strFriendlyName;

    pDevice->GetFriendlyName(strFriendlyName);

    int nItem = m_DeviceListCtrl.GetItemCount();

    // add item
    m_DeviceListCtrl.InsertItem(nItem, strFriendlyName, pDevice->GetImageIndex());

    // associate the data point with the item
    m_DeviceListCtrl.SetItemData(nItem, (DWORD_PTR)pDevice);
}

void
CHostedNetworkDlg::RemoveWlanDevice(
    CWlanDevice *pDevice
    )
{
    _ASSERT(pDevice != NULL);
    
    int nItem = -1;

    do
    {
        nItem = m_DeviceListCtrl.GetNextItem(nItem, LVNI_ALL);

        if (-1 == nItem)
        {
            // no more item
            break;
        }

        CWlanDevice * pTmpDevice = (CWlanDevice *)m_DeviceListCtrl.GetItemData(nItem);

        _ASSERT(pTmpDevice != NULL);

        if (*pDevice == *pTmpDevice)
        {
            // found the match item, remove it
            m_DeviceListCtrl.DeleteItem(nItem);
            break;
        }

    } while (true);
}

void
CHostedNetworkDlg::UpdateWlanDevice(
    CWlanDevice *pDevice
    )
{
    _ASSERT(pDevice != NULL);
    
    int nItem = -1;

    do
    {
        nItem = m_DeviceListCtrl.GetNextItem(nItem, LVNI_ALL);

        if (-1 == nItem)
        {
            // no more item
            break;
        }

        CWlanDevice * pTmpDevice = (CWlanDevice *)m_DeviceListCtrl.GetItemData(nItem);

        _ASSERT(pTmpDevice != NULL);

        if (*pDevice == *pTmpDevice)
        {
            // found the match item, remove it and add it again with new data
            CAtlString strFriendlyName;

            pDevice->GetFriendlyName(strFriendlyName);

            m_DeviceListCtrl.DeleteItem(nItem);

            // add item
            m_DeviceListCtrl.InsertItem(nItem, strFriendlyName, pDevice->GetImageIndex());

            // associate the data point with the item
            m_DeviceListCtrl.SetItemData(nItem, (DWORD_PTR)pDevice);

            break;
        }

    } while (true);
}

void
CHostedNetworkDlg::DisableAllControls()
{
    // Name
    m_NameEdit.EnableWindow(FALSE);
    // Key
    m_KeyEdit.EnableWindow(FALSE);
    // Enable ICS
    m_EnableIcsCheck.EnableWindow(FALSE);
    // Connection List
    m_ConnectionComboBox.EnableWindow(FALSE);
    // Start/Stop hosted network
    m_HostedNetworkButton.EnableWindow(FALSE);
}

void
CHostedNetworkDlg::EnableAllControls()
{
    // Name
    m_NameEdit.EnableWindow();
    // Key
    m_KeyEdit.EnableWindow();

    if (m_bIsAdmin && m_bIsICSAllowed)
    {
        ASSERT(m_IcsMgr);

        // Enable ICS
        m_EnableIcsCheck.EnableWindow();
        // Connection List
        if (m_IcsNeeded)
        {
            m_ConnectionComboBox.EnableWindow();
        }
    }
    // Start/Stop hosted network
    m_HostedNetworkButton.EnableWindow();
}

void 
CHostedNetworkDlg::OnEnKillfocusEditName()
{
    // TODO: Add your control notification handler code here
    CString strName;

    m_NameEdit.GetWindowTextW(strName);

    if (strName.GetLength() > DOT11_SSID_MAX_LENGTH || strName.GetLength() < 1)
    {
        PostErrorMessage(L"Hosted network name contains 1 ~ 32 case-sensitive characters.");
        m_NameEdit.SetFocus();
    }
}


void CHostedNetworkDlg::OnEnKillfocusEditKey()
{
    // TODO: Add your control notification handler code here
    CString strKey;

    m_KeyEdit.GetWindowTextW(strKey);

    if (strKey.GetLength() > 63 || strKey.GetLength() < 8)
    {
        PostErrorMessage(L"Hosted network key contains 8 ~ 63 case-sensitive characters.");
        m_KeyEdit.SetFocus();
    }
}

bool CHostedNetworkDlg::CheckValidSSIDKeyLen()
{
    CString strName;
    bool    bSSIDValid   = true;
    bool    bKeyValid    = true;
    bool    bCheckPassed = false;

    m_NameEdit.GetWindowTextW(strName);
    if (strName.GetLength() > DOT11_SSID_MAX_LENGTH || strName.GetLength() < 1)
    {
        bSSIDValid = false;
    }

    m_KeyEdit.GetWindowTextW(strName);
    if (strName.GetLength() > 63 || strName.GetLength() < 8)
    {
        bKeyValid = false;
    }

    if (!bSSIDValid && bKeyValid)
    {
        PostErrorMessage(L"Hosted network name should contain 1 ~ 32 case-sensitive characters.");
        m_NameEdit.SetFocus();
    }
    else if (bSSIDValid && !bKeyValid)
    {
        PostErrorMessage(L"Hosted network key should contain 8 ~ 63 case-sensitive characters.");
        m_KeyEdit.SetFocus();
    }
    else if (!bSSIDValid && !bKeyValid)
    {
        PostErrorMessage(L"Hosted network name should contain 1 ~ 32 case-sensitive characters, and hosted network key contains 8 ~ 63 case-sensitive characters.");
        m_NameEdit.SetFocus();
    }
    else
    {
        bCheckPassed = true;
    }

    return bCheckPassed;
}

void 
CHostedNetworkDlg::PostDeviceNotification(
    CWlanDevice * pDevice, 
    int Event
    )
{
    _ASSERT(pDevice != NULL);
    bool fPostNotification = true;

    CAtlString strFriendlyName;
    pDevice->GetFriendlyName(strFriendlyName);

    CAtlString msg;

    switch(Event)
    {
    case CHostedNetworkNotification::DeviceAdd:
        msg = L"Device \""+strFriendlyName + L"\" joined the hosted network";
        break;

    case CHostedNetworkNotification::DeviceRemove:
        msg = L"Device \""+strFriendlyName + L"\" left the hosted network";
        break;

    case CHostedNetworkNotification::DeviceUpdate:
        msg = L"Device \""+strFriendlyName + L"\" was updated";
        break;

    default:
        fPostNotification = false;
    }

    if (fPostNotification)
    {
        PostNotification(msg.GetString());    
    }
}

#define MAX_NOTIFICATION_LENGTH 1024

void 
CHostedNetworkDlg::PostNotification(
    LPCWSTR msg
    ) 
{
    // Get current time
    CTime currTime = CTime::GetCurrentTime();
    WCHAR strNotification[MAX_NOTIFICATION_LENGTH + 1] = {0};

    if (SUCCEEDED(StringCchPrintf(
            strNotification,
            MAX_NOTIFICATION_LENGTH,
            L"[%02d/%02d/%04d:%02d:%2d:%02d] %ws",
            currTime.GetMonth(),
            currTime.GetDay(),
            currTime.GetYear(),
            currTime.GetHour(),
            currTime.GetMinute(),
            currTime.GetSecond(),
            msg
            )))
    {
        m_NotificationOutputList.AddString(strNotification);
    }
    else
    {
        m_NotificationOutputList.AddString(msg);
    }
};

BOOL 
CHostedNetworkDlg::IsUserAdmin()
{
    BOOL                     bIsAdmin              = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority           = SECURITY_NT_AUTHORITY;
    PSID                     AdministratorsGroup; 

    bIsAdmin = AllocateAndInitializeSid(
           &NtAuthority,
           2,
           SECURITY_BUILTIN_DOMAIN_RID,
           DOMAIN_ALIAS_RID_ADMINS,
           0, 
           0, 
           0, 
           0, 
           0, 
           0,
           &AdministratorsGroup
           ); 

    if(bIsAdmin) 
    {
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &bIsAdmin)) 
        {
            bIsAdmin = FALSE;
        } 
        FreeSid(AdministratorsGroup); 
    }

    return(bIsAdmin);
}

bool
CHostedNetworkDlg::IsICSAllowed()
{
    bool  bIsIcsAllowed = false;
    HKEY  hAllowIcs     = NULL; 
    DWORD dwError       = ERROR_SUCCESS;
    DWORD   ValLen = sizeof(DWORD);
    DWORD   dwAllowIcs = 1;

    dwError = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Policies\\Microsoft\\Windows\\Network Connections",
        NULL,
        KEY_READ,
        &hAllowIcs
        );

    if (ERROR_SUCCESS != dwError || !hAllowIcs)
    {
        BAIL();
    }

    dwError = RegGetValue(
        hAllowIcs,
        NULL,
        L"NC_ShowSharedAccessUI",
        RRF_RT_DWORD,
        NULL,
        &dwAllowIcs,
        &ValLen
        );
    if( dwError == ERROR_SUCCESS && dwAllowIcs == 0)
    {
        bIsIcsAllowed = false;
    }
    else
    {
        bIsIcsAllowed = true;
    }

error:
    if (hAllowIcs)
    {
        RegCloseKey(hAllowIcs);
    }
    return bIsIcsAllowed;
}
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// COMRTSDlg.cpp : implementation file
//

#include "stdafx.h"
#include "COMRTS.h"
#include "COMRTSDlg.h"
#include "PackMod.h"
#include "CustomRenderer.h"
#include "GestureHandler.h"

#include "msinkaut.h"
#include "rtscom.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CComPtr<IRealTimeStylus> g_pRealTimeStylus;
CComPtr<IDynamicRenderer> g_pDynamicRenderer;
CComPtr<IGestureRecognizer> g_pGestureRecognizer;
CComPtr<IPacketModifier> g_pPacketModifier;
CComPtr<ICustomRenderer> g_pCustomRenderer;
CGestureHandler* g_pGestureHandler;


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


// CCOMRTSDlg dialog
CCOMRTSDlg::CCOMRTSDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CCOMRTSDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCOMRTSDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_INKCOLLECTOR, m_gbTestArea);
    DDX_Control(pDX, IDC_STATICGESTURESTATUS, m_staticGestureStatus);
    DDX_Control(pDX, IDC_CHECKEDLIST, m_ListBox);
}

BEGIN_MESSAGE_MAP(CCOMRTSDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTONCLEARINK, OnBnClickedButtonClearTestArea)
    ON_BN_CLICKED(IDC_CHECK0, OnBnClickedCheck)
    ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck)
    ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck)
    ON_BN_CLICKED(IDC_CHECK3, OnBnClickedCheck)
    ON_BN_CLICKED(IDC_BUTTONUP, OnBnClickedButtonUp)
    ON_BN_CLICKED(IDC_BUTTONDOWN, OnBnClickedButtonDown)
END_MESSAGE_MAP()


// CCOMRTSDlg message handlers

BOOL CCOMRTSDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // Initialize the CheckBox array
    m_pCheck[0] = reinterpret_cast<CButton*>(this->GetDlgItem(IDC_CHECK0));
    m_pCheck[1] = reinterpret_cast<CButton*>(this->GetDlgItem(IDC_CHECK1));
    m_pCheck[2] = reinterpret_cast<CButton*>(this->GetDlgItem(IDC_CHECK2));
    m_pCheck[3] = reinterpret_cast<CButton*>(this->GetDlgItem(IDC_CHECK3));

    // Create and initialize the RealTimeStylus object
    if (SUCCEEDED(InitRealTimeStylus()))
    {
        // Create and initialize the PacketFilter plug-in
        if (SUCCEEDED(InitPacketFilter()))
        {
            TRACE(_T("Succeeded initializing PacketFilter plug-in\n"));
        }

        // Create and initialize the CustomRenderer plug-in
        if (SUCCEEDED(InitCustomRenderer()))
        {
            TRACE(_T("Succeeded initializing CustomRenderer plug-in\n"));
        }

        // Create and initialize the GestureRecognizer plug-in
        if (SUCCEEDED(InitGestureRecognizer()))
        {
            TRACE(_T("Succeeded initializing GestureRecognizer plug-in\n"));
        }

        // Create and initialize the DynamicRenderer plug-in
        if (SUCCEEDED(InitDynamicRenderer()))
        {
            TRACE(_T("Succeeded initializing DynamicRenderer plug-in\n"));
        }
    }
    else
    {
        TRACE(_T("Error initializing RealTimeStylus\n"));
    }

    // Initialize the checked list box
    m_ListBox.SetCurSel(0);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCOMRTSDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCOMRTSDlg::OnPaint()
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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCOMRTSDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


// Helper functions
HRESULT CCOMRTSDlg::InitRealTimeStylus()
{
    // Create an IRealTimeStylus object
    HRESULT hr = g_pRealTimeStylus.CoCreateInstance(CLSID_RealTimeStylus);

    if (SUCCEEDED(hr))
    {
        hr = g_pRealTimeStylus->put_HWND(reinterpret_cast<HANDLE_PTR>(m_gbTestArea.m_hWnd));

        if (SUCCEEDED(hr))
        {
            hr = g_pRealTimeStylus->put_Enabled(TRUE);
        }
    }
    return hr;
}

HRESULT CCOMRTSDlg::InitPacketFilter()
{
    // Create a PacketModifier plug-in
    HRESULT hr = g_pPacketModifier.CoCreateInstance(CLSID_PacketModifier);

    if (SUCCEEDED(hr))
    {
        CComPtr<IStylusSyncPlugin> spSyncPlugin;
        hr = g_pPacketModifier.QueryInterface(&spSyncPlugin);

        if (SUCCEEDED(hr))
        {
            ULONG nStylusPluginCount;
            hr = g_pRealTimeStylus->GetStylusSyncPluginCount(&nStylusPluginCount);

            if (SUCCEEDED(hr))
            {
                // Set the rectangle to restrict the packets
                RECT filterRect;
                RECT clientRect;
                HDC hDC = m_gbTestArea.GetDC()->GetSafeHdc();
                m_gbTestArea.GetClientRect(&clientRect);

                filterRect.top = clientRect.top + 50;
                filterRect.left = clientRect.left + 50;
                filterRect.bottom = clientRect.bottom - 50;
                filterRect.right = clientRect.right - 50;
                g_pPacketModifier->SetRectangle(hDC, filterRect);

                // Update the UI
                m_ListBox.AddString(L"Packet Filter");
                m_pCheck[nStylusPluginCount]->SetCheck(BST_CHECKED);

                // Add to the plug-in collection
                hr = InsertIntoPluginCollection(nStylusPluginCount, spSyncPlugin);
            }
        }
    }
    return hr;
}

HRESULT CCOMRTSDlg::InitCustomRenderer()
{
    // Create a CCustomRenderer object
    HRESULT hr = g_pCustomRenderer.CoCreateInstance(CLSID_CustomRenderer);

    if (SUCCEEDED(hr))
    {
        // Get a pointer to the IStylusSyncPlugin interface
        CComPtr<IStylusSyncPlugin> spSyncPlugin;
        hr = g_pCustomRenderer.QueryInterface(&spSyncPlugin);

        if (SUCCEEDED(hr))
        {
            ULONG nStylusPluginCount;
            hr = g_pRealTimeStylus->GetStylusSyncPluginCount(&nStylusPluginCount);

            if (SUCCEEDED(hr))
            {
                // Set the HDC so we can draw circles on the test area
                g_pCustomRenderer->SetHDC(m_gbTestArea.GetDC()->GetSafeHdc());

                // Update the UI
                m_ListBox.AddString(L"Custom Renderer");
                m_pCheck[nStylusPluginCount]->SetCheck(BST_CHECKED);

                // Add to the plug-in collection
                hr = InsertIntoPluginCollection(nStylusPluginCount, spSyncPlugin);
            }
        }
    }
    return hr;
}

HRESULT CCOMRTSDlg::InitGestureRecognizer()
{
    // Create an IGestureRecognizer object
    HRESULT hr = g_pGestureRecognizer.CoCreateInstance(CLSID_GestureRecognizer);

    if (SUCCEEDED(hr))
    {
        // Get a pointer to the IStylusSyncPlugin interface
        CComPtr<IStylusSyncPlugin> spSyncPlugin;
        hr = g_pGestureRecognizer.QueryInterface(&spSyncPlugin);

        if (SUCCEEDED(hr))
        {
            // Get the current count of plugins so we can
            // add this one to the end of the collection
            ULONG nStylusPluginCount;
            hr = g_pRealTimeStylus->GetStylusSyncPluginCount(&nStylusPluginCount);

            if (SUCCEEDED(hr))
            {

                // Update the UI
                m_ListBox.AddString(L"Gesture Recognizer");
                m_pCheck[nStylusPluginCount]->SetCheck(BST_CHECKED);

                // Add to the plug-in collection
                hr = InsertIntoPluginCollection(nStylusPluginCount, spSyncPlugin);

                if (SUCCEEDED(hr))
                {
                    // Set the gestures we want to receive, in this case, all of them
                    int iGestures[] = { IAG_AllGestures };
                    hr = g_pGestureRecognizer->EnableGestures(1, iGestures);

                    if (SUCCEEDED(hr))
                    {
                        hr = InitGestureHandler();

                        if (SUCCEEDED(hr))
                        {
                            // Enable the Gesture Recognizer
                            hr = g_pGestureRecognizer->put_Enabled(TRUE);
                        }
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT CCOMRTSDlg::InitGestureHandler()
{
    // Create an IGestureHandler object
    HRESULT hr = CoCreateInstance(CLSID_GestureHandler, NULL, CLSCTX_INPROC, IID_IGestureHandler, reinterpret_cast<VOID **>(&g_pGestureHandler));

    if (SUCCEEDED(hr))
    {
        // Get a pointer to the IStylusAsyncPlugin interface
        IStylusAsyncPlugin* pAsyncPlugin;
        hr = g_pGestureHandler->QueryInterface(IID_IStylusAsyncPlugin, reinterpret_cast<void**>(&pAsyncPlugin));

        if (SUCCEEDED(hr))
        {
            // Get the current count of plugins so we can
            // add this one to the end of the collection
            ULONG nAsyncPluginCount;
            hr = g_pRealTimeStylus->GetStylusAsyncPluginCount(&nAsyncPluginCount);

            if (SUCCEEDED(hr))
            {
                // Add the plug-in to the StylusAsyncPlugin collection
                hr = g_pRealTimeStylus->AddStylusAsyncPlugin(nAsyncPluginCount, pAsyncPlugin);

                if (SUCCEEDED(hr))
                {
                    // Pass the Gesture Handler a pointer to the
                    // status window so it can update the status
                    hr = g_pGestureHandler->SetStatusWindow(&m_staticGestureStatus);
                }
            }
        }
    }
    return hr;
}

HRESULT CCOMRTSDlg::InitDynamicRenderer()
{
    // Create an IDynamicRenderer object
    HRESULT hr = g_pDynamicRenderer.CoCreateInstance(CLSID_DynamicRenderer);

    if (SUCCEEDED(hr))
    {
        CComPtr<IStylusSyncPlugin> spSyncPlugin;
        hr = g_pDynamicRenderer.QueryInterface(&spSyncPlugin);

        if (SUCCEEDED(hr))
        {
            hr = g_pDynamicRenderer->put_HWND(reinterpret_cast<HANDLE_PTR>(m_gbTestArea.m_hWnd));

            if (SUCCEEDED(hr))
            {
                ULONG nStylusPluginCount;
                hr = g_pRealTimeStylus->GetStylusSyncPluginCount(&nStylusPluginCount);

                if (SUCCEEDED(hr))
                {
                    // Update the UI
                    m_ListBox.AddString(L"Dynamic Renderer");
                    m_pCheck[nStylusPluginCount]->SetCheck(BST_CHECKED);

                    // Add to the plug-in collection
                    hr = InsertIntoPluginCollection(nStylusPluginCount, spSyncPlugin);

                    if (SUCCEEDED(hr))
                    {
                        hr = g_pDynamicRenderer->put_Enabled(TRUE);
                    }
                }
            }
        }
    }
    return hr;
}


void CCOMRTSDlg::Clear()
{
    // Clear the stylus queues
    if (!SUCCEEDED(g_pRealTimeStylus->ClearStylusQueues()))
    {
        TRACE(_T("Error clearing stylus queues."));
    }

    // Clear the status text
    m_staticGestureStatus.SetWindowTextW(L"");

    // Redaw the window to clear the ink
    this->RedrawWindow();
}

int CCOMRTSDlg::FindPrecedingPlugin(int nIndex)
{
    int nInsertIndex = 0;

    CButton* pCheckBox;

    for (int i = IDC_CHECK0; i < IDC_CHECK0 + nIndex; i++)
    {
        pCheckBox = reinterpret_cast<CButton*>(this->GetDlgItem(i));

        if (pCheckBox->GetCheck())
        {
            nInsertIndex++;
        }
    }
    return nInsertIndex;
}

HRESULT CCOMRTSDlg::InsertIntoPluginCollection(int nIndex, IStylusSyncPlugin* pSyncPlugin)
{
    // Get the plug-in at the given index
    // IStylusSyncPlugin* pSyncPlugin = static_cast<IStylusSyncPlugin*>(m_ListBox.GetItemDataPtr(nIndex));

    if (NULL != pSyncPlugin)
    {
        int nInsertIndex = FindPrecedingPlugin(nIndex);

        // Set the listbox item to point to the plug-in
        m_ListBox.SetItemDataPtr(nInsertIndex, pSyncPlugin);

        // Insert the plug-in at the given index.
        return g_pRealTimeStylus->AddStylusSyncPlugin(nInsertIndex, pSyncPlugin);
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT CCOMRTSDlg::RemoveFromPluginCollection(int nIndex)
{
    // Get the plug-in at the given index
    IStylusSyncPlugin* pSyncPlugin = static_cast<IStylusSyncPlugin*>(m_ListBox.GetItemDataPtr(nIndex));

    // If we were able to locate the plug-in (pSyncPlugin is not null)
    if (NULL != pSyncPlugin)
    {
        int nInsertIndex = FindPrecedingPlugin(nIndex);

        // Remove the plug-in at the given index.
        return g_pRealTimeStylus->RemoveStylusSyncPlugin(nInsertIndex, &pSyncPlugin);
    }
    else
    {
        return E_FAIL;
    }
}


// Event Handlers
void CCOMRTSDlg::OnBnClickedButtonClearTestArea()
{
    // Clear the ink
    Clear();
}

void CCOMRTSDlg::OnBnClickedCheck()
{
    // Get a pointer to the CheckBox that was clicked
    CButton* pBtnFocus = reinterpret_cast<CButton*>(this->GetFocus());

    // Get the index of the CheckBox
    int nCtrlID = pBtnFocus->GetDlgCtrlID();
    int nCurSel = nCtrlID - IDC_CHECK0;

    // Select the corresponding item in the ListBox
    m_ListBox.SetCurSel(nCurSel);

    if (pBtnFocus->GetCheck())
    {
        // If the checkbox is checked,
        // add the plug-in to the RealTimeStylus

        // Get the item data pointer, which
        // points to the associated plug-in
        IStylusSyncPlugin* pCurSelPlugin = static_cast<IStylusSyncPlugin*>(m_ListBox.GetItemDataPtr(nCurSel));

        InsertIntoPluginCollection(nCurSel, pCurSelPlugin);
    }
    else
    {
        // If the checkbox is unchecked,
        // remove the plug-in from the RealTimeStylus
        RemoveFromPluginCollection(nCurSel);
    }

    // Clear the ink
    Clear();
}

void CCOMRTSDlg::OnBnClickedButtonUp()
{
    int nCurSel = m_ListBox.GetCurSel();

    // If we're not at the top position...
    if (nCurSel > 0)
    {
        // Store the state of the CheckBoxes
        int nCurSelChecked = m_pCheck[nCurSel]->GetCheck();
        int nPrevSelChecked = m_pCheck[nCurSel-1]->GetCheck();

        // Get the item data pointer, which
        // points to the associated plug-in
        IStylusSyncPlugin* pCurSelPlugin = static_cast<IStylusSyncPlugin*>(m_ListBox.GetItemDataPtr(nCurSel));

        // Store the currently selected item
        CString strSel;
        m_ListBox.GetText(nCurSel, strSel);

        // Remove the selection from the ListBox
        m_ListBox.DeleteString(nCurSel);

        // Remove the associated plug-in
        RemoveFromPluginCollection(nCurSel);

        // Insert the stored item string at the new location
        m_ListBox.InsertString(nCurSel-1, strSel);

        // Set the listbox selection to the new location
        m_ListBox.SetCurSel(nCurSel-1);

        // Add the plug-in back into the
        // collection at the new position
        InsertIntoPluginCollection(nCurSel-1, pCurSelPlugin);

        // Update the CheckBoxes
        m_pCheck[nCurSel]->SetCheck(nPrevSelChecked);
        m_pCheck[nCurSel-1]->SetCheck(nCurSelChecked);

        // Clear the ink
        Clear();
    }
}

void CCOMRTSDlg::OnBnClickedButtonDown()
{
    int nCurSel = m_ListBox.GetCurSel();

    // If we're not at the bottom position...
    if (nCurSel < 3)
    {
        // Store the state of the CheckBoxes
        int nCurSelChecked = m_pCheck[nCurSel]->GetCheck();
        int nNextSelChecked = m_pCheck[nCurSel+1]->GetCheck();

        // Get the item data pointer, which
        // points to the associated plug-in
        IStylusSyncPlugin* pCurSelPlugin = static_cast<IStylusSyncPlugin*>(m_ListBox.GetItemDataPtr(nCurSel));

        // Store the currently selected item
        CString strSel;
        m_ListBox.GetText(nCurSel, strSel);

        // Remove the selection from the ListBox
        m_ListBox.DeleteString(nCurSel);

        // Remove the associated plug-in
        RemoveFromPluginCollection(nCurSel);

        // Insert the stored item string at the new location
        m_ListBox.InsertString(nCurSel+1, strSel);

        // Set the listbox selection to the new location
        m_ListBox.SetCurSel(nCurSel+1);

        // Add the plug-in back into the
        // collection at the new position
        InsertIntoPluginCollection(nCurSel+1, pCurSelPlugin);

        // Update the CheckBoxes
        m_pCheck[nCurSel]->SetCheck(nNextSelChecked);
        m_pCheck[nCurSel+1]->SetCheck(nCurSelChecked);

        // Clear the ink
        Clear();
    }
}

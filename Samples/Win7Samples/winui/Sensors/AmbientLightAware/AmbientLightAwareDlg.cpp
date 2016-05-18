//-----------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation.  All rights reserved.
//
// Module:
//        AmbientLightAwareDlg.cpp
//
// Description:
//        Dialog for Ambient Light Aware SDK Sample
//
// Comments: 
//        Standard vc++ dialog created by VS 2005 wizard.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "AmbientLightAware.h"
#include "AmbientLightAwareSensorEvents.h"
#include "AmbientLightAwareSensorManagerEvents.h"
#include "AmbientLightAwareDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Wizard generated constructor
CAmbientLightAwareDlg::CAmbientLightAwareDlg(CWnd* pParent /*=NULL*/)
: CDialog(CAmbientLightAwareDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pSensorManagerEvents = NULL;
    ZeroMemory(&m_lfLogFont, sizeof(LOGFONT));
}

// Wizard generated destructor
CAmbientLightAwareDlg::~CAmbientLightAwareDlg()
{
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareDlg::CleanUp
//
// Description of function/method:
//        Helper function, called by main winapp to do clean up of private
//        members
//
// Parameters:
//        none
//
// Return Values:
//        S_OK
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareDlg::CleanUp()
{
    HRESULT hr = S_OK;

    hr = m_pSensorManagerEvents->Uninitialize();

    if (NULL != m_pSensorManagerEvents)
    {
        delete m_pSensorManagerEvents;
        m_pSensorManagerEvents = NULL;
    }

    return hr;
}

// Wizard generated function
void CAmbientLightAwareDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAmbientLightAwareDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CAmbientLightAwareDlg message handlers
// Wizard generated function.  Added helper function call inside.
BOOL CAmbientLightAwareDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // ************************************************************************
    // Added this helper function call to initialize sensor events
    // ************************************************************************
    InitAmbientLightAware();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
// Wizard generated function.  Added font size change for optimized text.
void CAmbientLightAwareDlg::OnPaint()
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

        // ********************************************************************
        // Change the font size for the optimzed text.
        // ********************************************************************
        CFont fontOptimzed;
        BOOL fRet = fontOptimzed.CreateFontIndirect(&m_lfLogFont);
        if (fRet)
        {
            CWnd* pStaticTextSample = GetDlgItem(IDC_STATIC_SAMPLE);
            if (NULL != pStaticTextSample)
            {
                pStaticTextSample->SetFont(&fontOptimzed);
            }
        }
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
// Wizard generated function
HCURSOR CAmbientLightAwareDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareDlg::InitAmbientLightAware
//
// Description of function/method:
//        Helper function, initializes sensor events class
//
// Parameters:
//        none
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareDlg::InitAmbientLightAware()
{
    HRESULT hr = S_OK;

    // save the font so we can easily change the text size in UpdateLux
    CFont* pFont;
    CWnd* pWnd = GetDlgItem(IDC_STATIC_SAMPLE);
    if (NULL != pWnd)
    {
        pFont = pWnd->GetFont();
        if (NULL != pFont)
        {
            LONG lRet = pFont->GetLogFont(&m_lfLogFont);
            if (0 != lRet)
            {
                m_pSensorManagerEvents = new CAmbientLightAwareSensorManagerEvents(this);
                hr = m_pSensorManagerEvents->Initialize();
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_POINTER;
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CAmbientLightAwareDlg::UpdateLux
//
// Description of function/method:
//        Callback function.  This function is called by the events class when
//        new data has been received.  This function then uses this information
//        to change the font size to be optimized for the current brightness
//        (lux).
//
//        This sample is not meant to be an ideal implementation, but just 
//        showing how sensor data can be collected and processed.
//
// Parameters:
//        lux:        The average lux value for all sensors
//        numSensors: The number of sensors reporting data
//
// Return Values:
//        TRUE on success, FALSE on failure
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CAmbientLightAwareDlg::UpdateLux(float lux, int numSensors)
{
    HRESULT hr = S_OK;
    CString strLux, strSensors;
    CFont fontSample;

    if(lux < 10.0)
    {
        // Darkness
        m_lfLogFont.lfHeight = 12; // A sample font size for dark environments
    }
    else if(lux < 300)
    {
        // Dim Indoors
        m_lfLogFont.lfHeight = 12; // A sample font size for dim indoor environments
    }
    else if(lux < 800)
    {
        // Normal Indoors
        m_lfLogFont.lfHeight = 12; // A sample font size for indoor environments
    }
    else if(lux < 10000)
    {
        // Bright Indoors
        m_lfLogFont.lfHeight = 16;  // A sample font size for bright indoor environments
    }
    else if(lux < 30000)
    {
        // Overcast Outdoors
        m_lfLogFont.lfHeight = 20;  // A sample font size for overcast environments
    }
    else
    {
        // Direct Sunlight
        m_lfLogFont.lfHeight = 30;  // A sample font size for sunny environments
    }

    strLux.Format(_T("Ambient light level: %.2f lux"), lux);
    SetDlgItemText(IDC_STATIC_LUX, strLux);

    strSensors.Format(_T("Sensors: %i"), numSensors);
    SetDlgItemText(IDC_STATIC_SENSORS, strSensors);

    // Force OnPaint which changes the text font to be optimized for this lux
    Invalidate();
    UpdateWindow();

    return hr;
}
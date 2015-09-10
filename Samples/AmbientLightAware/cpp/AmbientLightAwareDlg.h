//-----------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation.  All rights reserved.
//
// Module:
//        AmbientLightAwareDlg.h
//
// Description:
//        Dialog for Ambient Light Aware SDK Sample
//
// Comments: 
//        Standard vc++ dialog created by VS 2005 wizard.
//
//-----------------------------------------------------------------------------

#pragma once

// CAmbientLightAwareDlg dialog
class CAmbientLightAwareDlg : public CDialog
{
    // Construction
public:
    // standard constructor
    CAmbientLightAwareDlg(CWnd* pParent = NULL);
    virtual ~CAmbientLightAwareDlg();

    // Dialog Data
    enum { IDD = IDD_AMBIENTLIGHTAWARE_DIALOG };

    // Our callback function from CAmbientLightAwareEvents
    HRESULT UpdateLux(float lux, int numSensors);
    // Clean up function called by parent winapp
    HRESULT CleanUp();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

private:
    // helper function
    HRESULT InitAmbientLightAware();

    CAmbientLightAwareSensorManagerEvents* m_pSensorManagerEvents; // events class
    LOGFONT m_lfLogFont;                                           // font to be adjusted for current brightness
};

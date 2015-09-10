// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SCPDDisplay.h : header file
//
#include "util.h"
#include "genericucpdlg.h"
/////////////////////////////////////////////////////////////////////////////
// SCPDDosplay dialog

class CSCPDDisplay : public CDialog
{
// Construction
public:
    CSCPDDisplay(CWnd* pParent = NULL);   // standard constructor
    CSCPDDisplay(BSTR bstrDocument, CWnd* pParent = NULL);   // standard constructor
    ~CSCPDDisplay();
    // Dialog Data
    //{{AFX_DATA(CSCPDDisplay)
    enum { IDD = IDD_SCPD_DISPLAYDIALOG };
    CButton m_OkButton;
    CEdit   m_DocumentDisplay;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSCPDDisplay)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CSCPDDisplay)
    virtual BOOL OnInitDialog();
    afx_msg void OnOk();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    private:
    BSTR m_bstrSCPDDocument;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


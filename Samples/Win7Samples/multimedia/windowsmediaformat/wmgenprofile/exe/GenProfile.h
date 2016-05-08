//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            GenProfile.h
//
// Abstract:            The prototypes for the MFC dialog application
//
//*****************************************************************************

#if !defined(AFX_GENPROFILEEXE_H__F82EDDC9_9E82_49BD_B869_246CF3FFC9E8__INCLUDED_)
#define AFX_GENPROFILEEXE_H__F82EDDC9_9E82_49BD_B869_246CF3FFC9E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        // main symbols

/////////////////////////////////////////////////////////////////////////////
// CGenProfileApp:
// See GenProfileExe.cpp for the implementation of this class
//

class CGenProfileApp : public CWinApp
{
public:
    CGenProfileApp();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CGenProfileApp)
    public:
    virtual BOOL InitInstance();
    //}}AFX_VIRTUAL

// Implementation

    //{{AFX_MSG(CGenProfileApp)
        // NOTE - the ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENPROFILEEXE_H__F82EDDC9_9E82_49BD_B869_246CF3FFC9E8__INCLUDED_)

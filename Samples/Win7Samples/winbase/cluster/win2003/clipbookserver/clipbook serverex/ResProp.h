/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      ResProp.h
//
//  Implementation File:
//      ResProp.cpp
//
//  Description:
//      Definition of the resource extension property page classes.
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Include Files
/////////////////////////////////////////////////////////////////////////////

#ifndef _BASEPAGE_H_
#include "BasePage.h"   // for CBasePropertyPage
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"   // for CObjectPropert
#endif

/////////////////////////////////////////////////////////////////////////////
// Forward Class Declarations
/////////////////////////////////////////////////////////////////////////////

class CClipBookServerParamsPage;

/////////////////////////////////////////////////////////////////////////////
// External Class Declarations
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
//  CClipBookServerParamsPage
//
//  Purpose:
//      Parameters page for resources.
//
/////////////////////////////////////////////////////////////////////////////

class CClipBookServerParamsPage
    : public CBasePropertyPage
{
    DECLARE_DYNCREATE( CClipBookServerParamsPage )

//
//  Construction
//

public:
    CClipBookServerParamsPage( void );

//
//  Dialog Data
//

    //{{AFX_DATA(CClipBookServerParamsPage)
    enum { IDD = IDD_PP_CLIPBOOKSERVER_PARAMETERS };
    //}}AFX_DATA

protected:

// Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CClipBookServerParamsPage)
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    virtual const CObjectProperty * Pprops( void ) const    { return NULL; }
    virtual DWORD                   Cprops( void ) const    { return 0; }

//
//  Implementation
//

protected:
    BOOL    BAllRequiredFieldsPresent( void ) const;

    // Generated message map functions
    //{{AFX_MSG(CClipBookServerParamsPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeRequiredField();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};  //*** class CClipBookServerParamsPage

/////////////////////////////////////////////////////////////////////////////

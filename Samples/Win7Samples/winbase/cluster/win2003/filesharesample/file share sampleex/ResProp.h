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

class CFileShareSampleParamsPage;

/////////////////////////////////////////////////////////////////////////////
// External Class Declarations
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
//  CFileShareSampleParamsPage
//
//  Purpose:
//      Parameters page for resources.
//
/////////////////////////////////////////////////////////////////////////////

class CFileShareSampleParamsPage
    : public CBasePropertyPage
{
    DECLARE_DYNCREATE( CFileShareSampleParamsPage )

//
//  Construction
//

public:
    CFileShareSampleParamsPage( void );

//
//  Dialog Data
//

    //{{AFX_DATA(CFileShareSampleParamsPage)
    enum { IDD = IDD_PP_FILESHARESAMPLE_PARAMETERS };
    CEdit   m_editShareName;
    CEdit   m_editPath;
    CString m_strShareName;
    CString m_strPath;
    CString m_strRemark;
    DWORD   m_nMaxUsers;
    //}}AFX_DATA
    CString m_strPrevShareName;
    CString m_strPrevPath;
    CString m_strPrevRemark;
    DWORD   m_nPrevMaxUsers;

protected:
    enum
    {
        epropShareName,
        epropPath,
        epropRemark,
        epropMaxUsers,
        epropMAX
    };
    CObjectProperty     m_rgProps[ epropMAX ];

// Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CFileShareSampleParamsPage)
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    virtual const CObjectProperty * Pprops( void ) const    { return m_rgProps; }
    virtual DWORD                   Cprops( void ) const    { return sizeof( m_rgProps ) / sizeof( CObjectProperty ); }

//
//  Implementation
//

protected:
    BOOL    BAllRequiredFieldsPresent( void ) const;

    // Generated message map functions
    //{{AFX_MSG(CFileShareSampleParamsPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeRequiredField();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};  //*** class CFileShareSampleParamsPage

/////////////////////////////////////////////////////////////////////////////

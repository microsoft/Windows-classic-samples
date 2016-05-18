/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      BasePage.h
//
//  Implementation File:
//      BasePage.cpp
//
//  Description:
//      Definition of the CBasePropertyPage class.  This class provides base
//      functionality for extension DLL property pages.
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

#pragma warning( push )
#include <CluAdmEx.h>
#pragma warning( pop )

#include "ExtObj.h"     // for CExtObject
#include "PropList.h"   // for CClusPropList, CObjectProperty

/////////////////////////////////////////////////////////////////////////////
// CBasePropertyPage dialog
/////////////////////////////////////////////////////////////////////////////

class CBasePropertyPage
    : public CPropertyPage
{
    DECLARE_DYNCREATE( CBasePropertyPage )

//
//  Construction
//

public:
    CBasePropertyPage( void );
    CBasePropertyPage( UINT nIDTemplateIn, UINT nIDCaptionIn = 0 );

    virtual ~CBasePropertyPage( void )
    {
    } //*** ~CBasePropertyPage

    //
    //  Second phase construction.
    //

    virtual HRESULT         HrInit( CExtObject * peoInout );
    HRESULT                 HrCreatePage( void );

protected:
    void                    CommonConstruct( void );

    //
    //  Attributes
    //
    CExtObject *            m_peo;
    HPROPSHEETPAGE          m_hpage;

    IDD                     m_iddPropertyPage;
    IDD                     m_iddWizardPage;
    IDS                     m_idsCaption;

    CExtObject *            Peo( void ) const               { return m_peo; }

    IDD                     IddPropertyPage( void ) const   { return m_iddPropertyPage; }
    IDD                     IddWizardPage( void ) const     { return m_iddWizardPage; }
    IDS                     IdsCaption( void ) const        { return m_idsCaption; }

public:
    HPROPSHEETPAGE          Hpage( void ) const             { return m_hpage; }

    CLUADMEX_OBJECT_TYPE    Cot( void ) const
    {
        ASSERT( Peo() != NULL );
        return Peo()->Cot();
    }; //*** Cot

//
//  Dialog Data
//

    //{{AFX_DATA(CBasePropertyPage)
    enum { IDD = 0 };
    //}}AFX_DATA
    CStatic m_staticIcon;
    CStatic m_staticTitle;
    CString m_strTitle;

//
//  Overrides
//

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CBasePropertyPage)
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnApply();
    virtual LRESULT OnWizardBack();
    virtual LRESULT OnWizardNext();
    virtual BOOL OnWizardFinish();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    virtual DWORD           ScParseUnknownProperty(
                                  LPCWSTR                           pwszNameIn
                                , const CLUSPROP_BUFFER_HELPER &    rvalueIn
                                , size_t                            cbBufIn
                                )
    {
        UNREFERENCED_PARAMETER( pwszNameIn );
        UNREFERENCED_PARAMETER( rvalueIn );
        UNREFERENCED_PARAMETER( cbBufIn );
        return ERROR_SUCCESS;

    } //*** ScParseUnknownProperty

    virtual BOOL            BApplyChanges( void );
    virtual BOOL            BBuildPropList( CClusPropList & rcplInout, BOOL fNoNewPropsIn = FALSE );
    virtual void            DisplaySetPropsError( DWORD scIn, UINT idsOperIn ) const;

    virtual const CObjectProperty * Pprops( void ) const    { return NULL; }
    virtual DWORD                   Cprops( void ) const    { return 0; }

//
//  Implementation
//

protected:
    BOOL                    m_bBackPressed;
    BOOL                    m_bSaved;
    BOOL                    m_bDoDetach;

    BOOL                    BBackPressed( void ) const      { return m_bBackPressed; }
    BOOL                    BSaved( void ) const            { return m_bSaved; }
    void                    EnableNext( BOOL fEnableIn = TRUE );
    IWCWizardCallback *     PiWizardCallback( void ) const
    {
        ASSERT( Peo() != NULL );
        return Peo()->PiWizardCallback();
    }; //*** PiWizardCallback

    BOOL                    BWizard( void ) const
    {
        ASSERT( Peo() != NULL );
        return Peo()->BWizard();
    }; //*** BWizard

    HCLUSTER                Hcluster( void ) const
    {
        ASSERT( Peo() != NULL );
        return Peo()->Hcluster();
    }; //*** Hcluster

    DWORD                   ScParseProperties( CClusPropList & rcplIn );
    BOOL                    BSetPrivateProps(
                                  BOOL fValidateOnlyIn = FALSE
                                , BOOL fNoNewPropsIn = FALSE
                                );

    //
    //  Generated message map functions
    //

    //{{AFX_MSG(CBasePropertyPage)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    afx_msg void OnChangeCtrl();
    DECLARE_MESSAGE_MAP()

};  //*** class CBasePropertyPage

/////////////////////////////////////////////////////////////////////////////
// CPageList
/////////////////////////////////////////////////////////////////////////////

typedef CList< CBasePropertyPage *, CBasePropertyPage * > CPageList;

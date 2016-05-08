/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      ResProp.cpp
//
//  Description:
//      Implementation of the resource extension property page classes.
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "File Share SampleEx.h"
#include "ResProp.h"
#include "ExtObj.h"
#include "DDxDDv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileShareSampleParamsPage property page
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CFileShareSampleParamsPage, CBasePropertyPage )

/////////////////////////////////////////////////////////////////////////////
// Message Maps

BEGIN_MESSAGE_MAP( CFileShareSampleParamsPage, CBasePropertyPage )
    //{{AFX_MSG_MAP(CFileShareSampleParamsPage)
    ON_EN_CHANGE( IDC_PP_FILESHARESAMPLE_SHARENAME, OnChangeRequiredField )
    ON_EN_CHANGE( IDC_PP_FILESHARESAMPLE_PATH, OnChangeRequiredField )
    //}}AFX_MSG_MAP

    //
    //  TODO: Modify the following lines to represent the data displayed on this page.
    //

    ON_EN_CHANGE( IDC_PP_FILESHARESAMPLE_REMARK, CBasePropertyPage::OnChangeCtrl )
    ON_EN_CHANGE( IDC_PP_FILESHARESAMPLE_MAXUSERS, CBasePropertyPage::OnChangeCtrl )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CFileShareSampleParamsPage::CFileShareSampleParamsPage
//
//  Description:
//      Default constructor.
//
//  Arguments:
//      None.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
CFileShareSampleParamsPage::CFileShareSampleParamsPage( void )
    : CBasePropertyPage( CFileShareSampleParamsPage::IDD )
{
    //
    //  TODO: Modify the following lines to represent the data displayed on this page.
    //

    //{{AFX_DATA_INIT(CFileShareSampleParamsPage)
    m_strShareName = _T("");
    m_strPath = _T("");
    m_strRemark = _T("");
    m_nMaxUsers = static_cast< DWORD >( 4294967295 );
    //}}AFX_DATA_INIT

    // Setup the property array.
    {
        m_rgProps[ epropShareName ].Set( REGPARAM_FILESHARESAMPLE_SHARENAME, m_strShareName, m_strPrevShareName );
        m_rgProps[ epropPath ].SetExpandSz( REGPARAM_FILESHARESAMPLE_PATH, m_strPath, m_strPrevPath );
        m_rgProps[ epropRemark ].Set( REGPARAM_FILESHARESAMPLE_REMARK, m_strRemark, m_strPrevRemark );
        m_rgProps[ epropMaxUsers ].Set( REGPARAM_FILESHARESAMPLE_MAXUSERS, m_nMaxUsers, m_nPrevMaxUsers );
    } // Setup the property array

    m_iddPropertyPage = IDD_PP_FILESHARESAMPLE_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_FILESHARESAMPLE_PARAMETERS;

} //*** CFileShareSampleParamsPage::CFileShareSampleParamsPage

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CFileShareSampleParamsPage::DoDataExchange
//
//  Description:
//      Do data exchange between the dialog and the class.
//
//  Arguments:
//      pDXIn
//          Data exchange object.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CFileShareSampleParamsPage::DoDataExchange(
    CDataExchange * pDXIn
    )
{
    if ( ! pDXIn->m_bSaveAndValidate || ! BSaved() )
    {
        AFX_MANAGE_STATE( AfxGetStaticModuleState() );

        //
        //  TODO: Modify the following lines to represent the data displayed on this page.
        //

        //  {{AFX_DATA_MAP(CFileShareSampleParamsPage)
        DDX_Control( pDXIn, IDC_PP_FILESHARESAMPLE_SHARENAME, m_editShareName );
        DDX_Control( pDXIn, IDC_PP_FILESHARESAMPLE_PATH, m_editPath );
        DDX_Text( pDXIn, IDC_PP_FILESHARESAMPLE_SHARENAME, m_strShareName );
        DDX_Text( pDXIn, IDC_PP_FILESHARESAMPLE_PATH, m_strPath );
        DDX_Text( pDXIn, IDC_PP_FILESHARESAMPLE_REMARK, m_strRemark );
        DDX_Text( pDXIn, IDC_PP_FILESHARESAMPLE_MAXUSERS, m_nMaxUsers );
        //}}AFX_DATA_MAP

        //
        //  Handle numeric parameters.
        //

        if ( ! BBackPressed() )
        {
            DDX_Number(
                  pDXIn
                , IDC_PP_FILESHARESAMPLE_MAXUSERS
                , m_nMaxUsers
                , static_cast< DWORD >( 0 )
                , static_cast< DWORD >( 4294967295 )
                , FALSE /*bSigned*/
                );
        } // if: back button not pressed

        //
        //  TODO: Add any additional field validation here.
        //

        if ( pDXIn->m_bSaveAndValidate )
        {
            //
            //  Make sure all required fields are present.
            //

            if ( ! BBackPressed() )
            {
                DDV_RequiredText( pDXIn, IDC_PP_FILESHARESAMPLE_SHARENAME, IDC_PP_FILESHARESAMPLE_SHARENAME_LABEL, m_strShareName );
                DDV_RequiredText( pDXIn, IDC_PP_FILESHARESAMPLE_PATH, IDC_PP_FILESHARESAMPLE_PATH_LABEL, m_strPath );
            } // if: back button not pressed
        } // if: saving data from dialog
    } // if: not saving or haven't saved yet

    CBasePropertyPage::DoDataExchange( pDXIn );

} //*** CFileShareSampleParamsPage::DoDataExchange

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CFileShareSampleParamsPage::OnInitDialog
//
//  Description:
//      Handler for the WM_INITDIALOG message.
//
//  Arguments:
//      None.
//
//  Return Value:
//      TRUE
//          We need the focus to be set for us.
//
//      FALSE
//          We already set the focus to the proper control.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
CFileShareSampleParamsPage::OnInitDialog( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CBasePropertyPage::OnInitDialog();

    //
    //  TODO: Limit the size of the text that can be entered in edit controls.
    //

    //
    //  Return TRUE unless you set the focus to a control.
    //
    //  EXCEPTION: OCX Property Pages should return FALSE..
    //

    return TRUE;

} //*** CFileShareSampleParamsPage::OnInitDialog

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CFileShareSampleParamsPage::OnSetActive
//
//  Description:
//      Handler for the PSN_SETACTIVE message.
//
//  Arguments:
//      None.
//
//  Return Value:
//      TRUE
//          Page successfully initialized.
//
//      FALSE
//          Page not initialized.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
CFileShareSampleParamsPage::OnSetActive( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    // Enable/disable the Next/Finish button.
    if ( BWizard() )
    {
        EnableNext( BAllRequiredFieldsPresent() );
    } // if: displaying a wizard

    return CBasePropertyPage::OnSetActive();

} //*** CFileShareSampleParamsPage::OnSetActive

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CFileShareSampleParamsPage::OnChangeRequiredField
//
//  Description:
//      Handler for the EN_CHANGE message on the Share name or Path edit
//      controls.
//
//  Arguments:
//      None.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CFileShareSampleParamsPage::OnChangeRequiredField( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    OnChangeCtrl();

    if ( BWizard() )
    {
        EnableNext( BAllRequiredFieldsPresent() );
    } // if: displaying a wizard

} //*** CFileShareSampleParamsPage::OnChangeRequiredField

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CFileShareSampleParamsPage::BAllRequiredFieldsPresent
//
//  Description:
//      Handler for the EN_CHANGE message on the Share name or Path edit
//      controls.
//
//  Arguments:
//      None.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
CFileShareSampleParamsPage::BAllRequiredFieldsPresent( void ) const
{
    BOOL    fPresent;

    if ( 0
        || (m_editShareName.GetWindowTextLength() == 0)
        || (m_editPath.GetWindowTextLength() == 0)
        )
    {
        fPresent = FALSE;
    } // if: required field not present
    else
    {
        fPresent = TRUE;
    } // else: all required fields are present

    return fPresent;

} //*** CFileShareSampleParamsPage::BAllRequiredFieldsPresent

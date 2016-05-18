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
#include "ClipBook ServerEx.h"
#include "ResProp.h"
#include "ExtObj.h"
#include "DDxDDv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClipBookServerParamsPage property page
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CClipBookServerParamsPage, CBasePropertyPage )

/////////////////////////////////////////////////////////////////////////////
// Message Maps

BEGIN_MESSAGE_MAP( CClipBookServerParamsPage, CBasePropertyPage )
    //{{AFX_MSG_MAP(CClipBookServerParamsPage)
    //}}AFX_MSG_MAP

    //
    //  TODO: Modify the following lines to represent the data displayed on this page.
    //

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClipBookServerParamsPage::CClipBookServerParamsPage
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
CClipBookServerParamsPage::CClipBookServerParamsPage( void )
    : CBasePropertyPage( CClipBookServerParamsPage::IDD )
{
    //
    //  TODO: Modify the following lines to represent the data displayed on this page.
    //

    //{{AFX_DATA_INIT(CClipBookServerParamsPage)
    //}}AFX_DATA_INIT

    // Setup the property array.
    {
    } // Setup the property array

    m_iddPropertyPage = IDD_PP_CLIPBOOKSERVER_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_CLIPBOOKSERVER_PARAMETERS;

} //*** CClipBookServerParamsPage::CClipBookServerParamsPage

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClipBookServerParamsPage::DoDataExchange
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
CClipBookServerParamsPage::DoDataExchange(
    CDataExchange * pDXIn
    )
{
    if ( ! pDXIn->m_bSaveAndValidate || ! BSaved() )
    {
        AFX_MANAGE_STATE( AfxGetStaticModuleState() );

        //
        //  TODO: Modify the following lines to represent the data displayed on this page.
        //

        //  {{AFX_DATA_MAP(CClipBookServerParamsPage)
        //}}AFX_DATA_MAP

        //
        //  Handle numeric parameters.
        //

        if ( ! BBackPressed() )
        {
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
            } // if: back button not pressed
        } // if: saving data from dialog
    } // if: not saving or haven't saved yet

    CBasePropertyPage::DoDataExchange( pDXIn );

} //*** CClipBookServerParamsPage::DoDataExchange

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClipBookServerParamsPage::OnInitDialog
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
CClipBookServerParamsPage::OnInitDialog( void )
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

} //*** CClipBookServerParamsPage::OnInitDialog

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClipBookServerParamsPage::OnSetActive
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
CClipBookServerParamsPage::OnSetActive( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    // Enable/disable the Next/Finish button.
    if ( BWizard() )
    {
        EnableNext( BAllRequiredFieldsPresent() );
    } // if: displaying a wizard

    return CBasePropertyPage::OnSetActive();

} //*** CClipBookServerParamsPage::OnSetActive

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClipBookServerParamsPage::OnChangeRequiredField
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
CClipBookServerParamsPage::OnChangeRequiredField( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    OnChangeCtrl();

    if ( BWizard() )
    {
        EnableNext( BAllRequiredFieldsPresent() );
    } // if: displaying a wizard

} //*** CClipBookServerParamsPage::OnChangeRequiredField

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClipBookServerParamsPage::BAllRequiredFieldsPresent
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
CClipBookServerParamsPage::BAllRequiredFieldsPresent( void ) const
{
    return TRUE;

} //*** CClipBookServerParamsPage::BAllRequiredFieldsPresent

/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      BasePage.cpp
//
//  Description:
//      Implementation of the CBasePropertyPage class.
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
#include "ExtObj.h"
#include "BasePage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBasePropertyPage property page
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CBasePropertyPage, CPropertyPage )

/////////////////////////////////////////////////////////////////////////////
// Message Maps
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP( CBasePropertyPage, CPropertyPage )
    //{{AFX_MSG_MAP(CBasePropertyPage)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::CBasePropertyPage
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
CBasePropertyPage::CBasePropertyPage( void )
{
    CommonConstruct();

} //*** CBasePropertyPage::CBasePropertyPage

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::CBasePropertyPage
//
//  Description:
//      Default constructor.
//
//  Arguments:
//      nIDTemplateIn
//          Dialog template resource ID.
//
//      nIDCaptionIn
//          Caption string resource ID.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
CBasePropertyPage::CBasePropertyPage(
      UINT  nIDTemplateIn
    , UINT  nIDCaptionIn
    )
    : CPropertyPage( nIDTemplateIn, nIDCaptionIn )
{
    CommonConstruct();

} //*** CBasePropertyPage::CBasePropertyPage

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::CommonConstruct
//
//  Description:
//      Common construction.
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
CBasePropertyPage::CommonConstruct( void )
{
    //{{AFX_DATA_INIT(CBasePropertyPage)
    //}}AFX_DATA_INIT

    m_peo = NULL;
    m_hpage = NULL;
    m_bBackPressed = FALSE;
    m_bSaved = FALSE;

    m_iddPropertyPage = NULL;
    m_iddWizardPage = NULL;
    m_idsCaption = NULL;

    m_bDoDetach = FALSE;

} //*** CBasePropertyPage::CommonConstruct

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::HrInit
//
//  Description:
//      Initialize the page.
//
//  Arguments:
//      peoInout
//          Pointer to the extension object.
//
//  Return Value:
//      S_OK
//          Page initialized successfully.
//
//      Other HRESULTs
//          Page failed to initialize.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CBasePropertyPage::HrInit(
    CExtObject * peoInout
    )
{
    ASSERT( peoInout != NULL );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    HRESULT         hr = S_OK;
    CWaitCursor     wc;
    DWORD           sc = ERROR_SUCCESS;
    CClusPropList   cpl;

    m_peo = peoInout;

    //
    //  Don't display a help button.
    //

    m_psp.dwFlags &= ~PSP_HASHELP;

    //
    //  Construct the property page.
    //

    if ( Peo()->BWizard() )
    {
        ASSERT( IddWizardPage() != NULL);
        Construct( IddWizardPage(), IdsCaption() );
    } // if: adding page to wizard
    else
    {
        ASSERT( IddPropertyPage() != NULL );
        Construct( IddPropertyPage(), IdsCaption() );
    } // else: adding page to property sheet

    //
    //  Read the properties private to this resource and parse them.
    //

    ASSERT( Peo() != NULL );
    ASSERT( Peo()->PodObjData() );

    //
    //  Read the properties.
    //

    switch ( Cot() )
    {
        case CLUADMEX_OT_CLUSTER:
        {
            CClusterData * pccd = reinterpret_cast< CClusterData * >( Peo()->PodObjData() );
            ASSERT( pccd && (pccd->GetHCluster() != NULL) );
            sc = cpl.ScGetClusterProperties(
                                      pccd->GetHCluster()
                                    , CLUSCTL_CLUSTER_GET_PRIVATE_PROPERTIES
                                    );
            break;
        }
        case CLUADMEX_OT_NODE:
        {
            CNodeData * pcnd = reinterpret_cast< CNodeData * >( Peo()->PodObjData() );
            ASSERT( pcnd && (pcnd->GetHNode() != NULL) );
            sc = cpl.ScGetNodeProperties(
                                      pcnd->GetHNode()
                                    , CLUSCTL_NODE_GET_PRIVATE_PROPERTIES
                                    );
            break;
        }
        case CLUADMEX_OT_GROUP:
        {
            CGroupData * pcgd = reinterpret_cast< CGroupData * >( Peo()->PodObjData() );
            ASSERT( pcgd && (pcgd->GetHGroup() != NULL) );
            sc = cpl.ScGetGroupProperties(
                                      pcgd->GetHGroup()
                                    , CLUSCTL_GROUP_GET_PRIVATE_PROPERTIES
                                    );
            break;
        }
        case CLUADMEX_OT_RESOURCE:
        {
            CResData * pcrd = reinterpret_cast< CResData * >( Peo()->PodObjData() );
            ASSERT( pcrd && (pcrd->GetHResource() != NULL) );
            sc = cpl.ScGetResourceProperties(
                                      pcrd->GetHResource()
                                    , CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES
                                    );
            break;
        }
        case CLUADMEX_OT_RESOURCETYPE:
        {
            CResTypeData * pcrd = reinterpret_cast< CResTypeData * >( Peo()->PodObjData() );
            ASSERT( pcrd && (pcrd->StrName().GetLength() > 0) );
            sc = cpl.ScGetResourceTypeProperties(
                                      Hcluster()
                                    , pcrd->StrName()
                                    , CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES
                                    );
            break;
        }
        case CLUADMEX_OT_NETWORK:
        {
            CNetworkData * pcnd = reinterpret_cast< CNetworkData * >( Peo()->PodObjData() );
            ASSERT( pcnd && (pcnd->GetHNetwork() != NULL) );
            sc = cpl.ScGetNetworkProperties(
                                      pcnd->GetHNetwork()
                                    , CLUSCTL_NETWORK_GET_PRIVATE_PROPERTIES
                                    );
            break;
        }
        case CLUADMEX_OT_NETINTERFACE:
        {
            CNetInterfaceData * pcnd = reinterpret_cast< CNetInterfaceData * >( Peo()->PodObjData() );
            ASSERT( pcnd && (pcnd->GetHNetInterface() != NULL) );
            sc = cpl.ScGetNetInterfaceProperties(
                                      pcnd->GetHNetInterface()
                                    , CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES
                                    );
            break;
        }
        default:
            ASSERT( 0 );
    } // switch: object type

    //
    //  Parse the properties.
    //

    if ( sc == ERROR_SUCCESS )
    {
        // Parse the properties.
        try
        {
            sc = ScParseProperties( cpl );
        } // try
        catch ( CMemoryException * pme )
        {
            hr = E_OUTOFMEMORY;
            pme->Delete();
        } // catch: CMemoryException
    } // if: properties read successfully

    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    } // if: error parsing getting or parsing properties

Cleanup:

    return hr;

} //*** CBasePropertyPage::HrInit

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::HrCreatePage
//
//  Description:
//      Create the page.
//
//  Arguments:
//      None.
//
//  Return Value:
//      S_OK
//          Page created successfully.
//
//      Other HRESULTs
//          Error creating the page.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CBasePropertyPage::HrCreatePage( void )
{
    ASSERT( m_hpage == NULL );

    HRESULT hr = S_OK;

    m_hpage = CreatePropertySheetPage( reinterpret_cast< LPPROPSHEETPAGEW >( &m_psp ) );
    if ( m_hpage == NULL )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    } // if: error creating the page

    return hr;

} //*** CBasePropertyPage::HrCreatePage

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::ScParseProperties
//
//  Description:
//      Parse the properties of the resource.  This is in a separate function
//      from HrInit so that the optimizer can do a better job.
//
//  Arguments:
//      rcplIn
//          Cluster property list to parse.
//
//  Return Values:
//      ERROR_SUCCESS
//          Properties were parsed successfully.
//
//      Other HRESULTs
//          Error parsing the properties.  (Includes any error returned from
//          ScParseUnknownProperty().)
//
//  Exceptions Thrown:
//      Any exceptions from CString::operator=().
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CBasePropertyPage::ScParseProperties(
    CClusPropList & rcplIn
    )
{
    DWORD                   sc = ERROR_SUCCESS;
    DWORD                   cprop;
    const CObjectProperty * pprop;

    ASSERT( rcplIn.PbPropList() != NULL );

    sc = rcplIn.ScMoveToFirstProperty();
    while ( sc == ERROR_SUCCESS )
    {
        //
        // Parse known properties.
        //

        for ( pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop-- )
        {
            if ( NIStringCompareW( 
                                   rcplIn.PszCurrentPropertyName()
                                 , pprop->m_pwszName
                                 , rcplIn.CbhCurrentPropertyName().pName->cbLength / sizeof( WCHAR )
                                 ) == 0
               )
            {
                if ( rcplIn.CpfCurrentValueFormat() == pprop->m_propFormat )
                {
                    switch ( pprop->m_propFormat )
                    {
                        case CLUSPROP_FORMAT_SZ:
                        case CLUSPROP_FORMAT_EXPAND_SZ:
                            ASSERT(     (rcplIn.CbCurrentValueLength() == (lstrlenW( rcplIn.CbhCurrentValue().pStringValue->sz ) + 1) * sizeof( WCHAR ))
                                    ||  (   (rcplIn.CbCurrentValueLength() == 0)
                                        &&  (rcplIn.CbhCurrentValue().pStringValue->sz[ 0 ] == L'\0') ) );
                            *pprop->m_value.pstr = rcplIn.CbhCurrentValue().pStringValue->sz;
                            *pprop->m_valuePrev.pstr = rcplIn.CbhCurrentValue().pStringValue->sz;

                            //
                            //  See if we need to find an expanded version
                            //

                            if ( pprop->m_valueEx.pstr != NULL )
                            {
                                // Copy the non-expanded one just in case there isn't an expanded version
                                *pprop->m_valueEx.pstr = rcplIn.CbhCurrentValue().pStringValue->sz;

                                // See if they included an expanded version
                                rcplIn.ScMoveToNextPropertyValue( );
                                if ( rcplIn.CpfCurrentValueFormat( ) == CLUSPROP_FORMAT_EXPANDED_SZ )
                                {
                                    *pprop->m_valueEx.pstr = rcplIn.CbhCurrentValue().pStringValue->sz;
                                } // if: found expanded version

                            } // if: *pprop->m_valueEx.pstr is present
                            break;

                        case CLUSPROP_FORMAT_EXPANDED_SZ:
                            ASSERT(     (rcplIn.CbCurrentValueLength() == (lstrlenW( rcplIn.CbhCurrentValue().pStringValue->sz ) + 1) * sizeof( WCHAR ))
                                    ||  (   (rcplIn.CbCurrentValueLength() == 0)
                                        &&  (rcplIn.CbhCurrentValue().pStringValue->sz[ 0 ] == L'\0') ) );
                            *pprop->m_value.pstr = rcplIn.CbhCurrentValue().pStringValue->sz;
                            *pprop->m_valuePrev.pstr = rcplIn.CbhCurrentValue().pStringValue->sz;

                            //
                            //  See if we need to find an expanded version
                            //

                            if ( *pprop->m_valueEx.pstr ) // can not use != NULL because overloading tries to do a string compare!
                            {
                                // Copy the expanded version
                                *pprop->m_valueEx.pstr = rcplIn.CbhCurrentValue().pStringValue->sz;

                                // See if they included a non-expanded version
                                rcplIn.ScMoveToNextPropertyValue( );
                                if ( rcplIn.CpfCurrentValueFormat( ) == CLUSPROP_FORMAT_SZ )
                                {
                                    *pprop->m_value.pstr = rcplIn.CbhCurrentValue().pStringValue->sz;
                                    *pprop->m_valuePrev.pstr = rcplIn.CbhCurrentValue().pStringValue->sz;
                                } // if: found non-expanded version

                            } // if: *pprop->m_valueEx.pstr is present
                            break;

                        case CLUSPROP_FORMAT_DWORD:
                        case CLUSPROP_FORMAT_LONG:
                            ASSERT( rcplIn.CbCurrentValueLength() == sizeof( DWORD ) );
                            *pprop->m_value.pdw = rcplIn.CbhCurrentValue().pDwordValue->dw;
                            *pprop->m_valuePrev.pdw = rcplIn.CbhCurrentValue().pDwordValue->dw;
                            break;

                        case CLUSPROP_FORMAT_BINARY:
                        case CLUSPROP_FORMAT_MULTI_SZ:
                            *pprop->m_value.ppb = rcplIn.CbhCurrentValue().pBinaryValue->rgb;
                            *pprop->m_value.pcb = rcplIn.CbhCurrentValue().pBinaryValue->cbLength;
                            *pprop->m_valuePrev.ppb = rcplIn.CbhCurrentValue().pBinaryValue->rgb;
                            *pprop->m_valuePrev.pcb = rcplIn.CbhCurrentValue().pBinaryValue->cbLength;
                            break;

                        default:
                            ASSERT(0);  // don't know how to deal with this type
                    } // switch: property format

                    //
                    //  Exit the loop since we found the parameter.
                    //

                    break;
                }// if: found a type match
            } // if: found a string match
        } // for: each property that we know about

        //
        // If the property wasn't known, ask the derived class to parse it.
        //

        if ( cprop == 0 )
        {
            sc = ScParseUnknownProperty(
                          rcplIn.CbhCurrentPropertyName().pName->sz
                        , rcplIn.CbhCurrentValue()
                        , rcplIn.RPvlPropertyValue().CbDataLeft()
                        );
            if ( sc != ERROR_SUCCESS )
            {
                return sc;
            } // if: error parsing the unknown property
        } // if: property not parsed

        //
        // Advance the buffer pointer past the value in the value list.
        //

        sc = rcplIn.ScMoveToNextProperty();
    } // while: more properties to parse

    //
    // If we reached the end of the properties, fix the return code.
    //

    if ( sc == ERROR_NO_MORE_ITEMS )
    {
        sc = ERROR_SUCCESS;
    } // if: ended loop after parsing all properties

    return sc;

} //*** CBasePropertyPage::ScParseProperties

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::OnCreate
//
//  Description:
//      Handler for the WM_CREATE message.
//
//  Arguments:
//      lpCreateStructIn
//          Window create structure.
//
//  Return Value:
//      -1
//          Error.
//
//      0
//          Success.
//
//--
/////////////////////////////////////////////////////////////////////////////
int
CBasePropertyPage::OnCreate(
    LPCREATESTRUCT lpCreateStructIn
    )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    //
    // Attach the window to the property page structure.
    // This has been done once already in the main application, since the
    // main application owns the property sheet.  It needs to be done here
    // so that the window handle can be found in the DLL's handle map.
    //

    if ( FromHandlePermanent( m_hWnd ) == NULL ) // is the window handle already in the handle map
    {
        HWND hWnd = m_hWnd;
        m_hWnd = NULL;
        Attach( hWnd );
        m_bDoDetach = TRUE;
    } // if: is the window handle in the handle map

    return CPropertyPage::OnCreate( lpCreateStructIn );

} //*** CBasePropertyPage::OnCreate

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::OnDestroy
//
//  Description:
//      Handler for the WM_DESTROY message.
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
CBasePropertyPage::OnDestroy( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    //
    // Detach the window from the property page structure.
    // This will be done again by the main application, since it owns the
    // property sheet.  It needs to be done here so that the window handle
    // can be removed from the DLL's handle map.
    //

    if ( m_bDoDetach )
    {
        if ( m_hWnd != NULL )
        {
            HWND hWnd = m_hWnd;

            Detach();
            m_hWnd = hWnd;
        } // if: do we have a window handle?
    } // if: do we need to balance the attach we did with a detach?

    CPropertyPage::OnDestroy();

} //*** CBasePropertyPage::OnDestroy

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::DoDataExchange
//
//  Description:
//      Do data exchange between the dialog and the class.
//
//  Arguments:
//      pDXIn
//          Data exchange object
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CBasePropertyPage::DoDataExchange(
    CDataExchange * pDXIn
    )
{
    if ( ! pDXIn->m_bSaveAndValidate || ! BSaved() )
    {
        AFX_MANAGE_STATE( AfxGetStaticModuleState() );

        //{{AFX_DATA_MAP(CBasePropertyPage)
            // NOTE: the ClassWizard will add DDX and DDV calls here
        //}}AFX_DATA_MAP

        DDX_Control( pDXIn, IDC_PP_ICON, m_staticIcon );
        DDX_Control( pDXIn, IDC_PP_TITLE, m_staticTitle );

        if ( pDXIn->m_bSaveAndValidate )
        {
            if ( ! BBackPressed() )
            {
                CWaitCursor wc;

                //
                //  Validate the data.
                //

                if ( ! BSetPrivateProps( TRUE /*fValidateOnlyIn*/ ) )
                {
                    pDXIn->Fail();
                } // if: error setting private properties
            } // if: Back button not pressed
        } // if: saving data from dialog
        else
        {
            //
            //  Set the title.
            //

            DDX_Text( pDXIn, IDC_PP_TITLE, m_strTitle );
        } // if: not saving data
    }  // if: not saving or haven't saved yet

    //
    //  Call the base class method.
    //

    CPropertyPage::DoDataExchange( pDXIn );

} //*** CBasePropertyPage::DoDataExchange

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::OnInitDialog
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
CBasePropertyPage::OnInitDialog( void )
{
    ASSERT( Peo() != NULL );
    ASSERT( Peo()->PodObjData() != NULL );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    //
    //  Set the title string.
    //

    m_strTitle = Peo()->PodObjData()->StrName();

    //
    //  Call the base class method.
    //

    CPropertyPage::OnInitDialog();

    //
    //  Display an icon for the object.
    //

    if ( Peo()->Hicon() != NULL )
    {
        m_staticIcon.SetIcon( Peo()->Hicon() );
    } // if: an icon was specified

    //
    //  Return TRUE unless you set the focus to a control.
    //
    //  EXCEPTION: OCX Property Pages should return FALSE.
    //

    return TRUE;

} //*** CBasePropertyPage::OnInitDialog

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::OnSetActive
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
CBasePropertyPage::OnSetActive( void )
{
    HRESULT hr = S_OK;

    ASSERT( Peo() != NULL);
    ASSERT( Peo()->PodObjData() != NULL );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    //
    //  Reread the data.
    //

    hr = Peo()->HrGetObjectInfo();
    if ( FAILED( hr ) )
    {
        return FALSE;
    } // if: error getting object info

    //
    //  Set the title string.
    //

    m_strTitle = Peo()->PodObjData()->StrName();

    m_bBackPressed = FALSE;
    m_bSaved = FALSE;
    return CPropertyPage::OnSetActive();

} //*** CBasePropertyPage::OnSetActive

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::OnApply
//
//  Description:
//      Handler for the PSM_APPLY message.
//
//  Arguments:
//      None.
//
//  Return Value:
//      TRUE
//          Page successfully applied.
//
//      FALSE
//          Error applying page.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
CBasePropertyPage::OnApply( void )
{
    ASSERT( ! BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CWaitCursor wc;

    if ( BApplyChanges() == FALSE )
    {
        return FALSE;
    } // if: error applying changes

    return CPropertyPage::OnApply();

} //*** CBasePropertyPage::OnApply

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::OnWizardBack
//
//  Description:
//      Handler for the PSN_WIZBACK message.
//
//  Arguments:
//      None.
//
//  Return Value:
//      -1
//          Don't change the page.
//
//      0
//          Change the page.
//
//--
/////////////////////////////////////////////////////////////////////////////
LRESULT
CBasePropertyPage::OnWizardBack( void )
{
    LRESULT lResult = 0L;

    ASSERT( BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    lResult = CPropertyPage::OnWizardBack();
    if ( lResult != -1 )
    {
        m_bBackPressed = TRUE;
    } // if: no error occurred

    return lResult;

} //*** CBasePropertyPage::OnWizardBack

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::OnWizardNext
//
//  Description:
//      Handler for the PSN_WIZNEXT message.
//
//  Arguments:
//      None.
//
//  Return Value:
//      -1
//          Don't change the page.
//
//      0
//          Change the page.
//
//--
/////////////////////////////////////////////////////////////////////////////
LRESULT
CBasePropertyPage::OnWizardNext( void )
{
    ASSERT( BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CWaitCursor wc;

    //
    //  Update the data in the class from the page.
    //  This necessary because, while OnKillActive() will call UpdateData(),
    //  it is called after this method is called, and we need to be sure that
    //  data has been saved before we apply them.
    //

    if ( ! UpdateData( TRUE /*bSaveAndValidate*/ ) )
    {
        return -1;
    } // if: error updating data

    //
    //  Save the data in the sheet.
    //

    if ( BApplyChanges() == FALSE )
    {
        return -1;
    } // if: error applying changes

    //
    //  Create the object.
    //

    return CPropertyPage::OnWizardNext();

} //*** CBasePropertyPage::OnWizardNext

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::OnWizardFinish
//
//  Description:
//      Handler for the PSN_WIZFINISH message.
//
//  Arguments:
//      None.
//
//  Return Value:
//      FALSE
//          Don't change the page.
//
//      TRUE
//          Change the page.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
CBasePropertyPage::OnWizardFinish( void )
{
    ASSERT( BWizard() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CWaitCursor wc;

    //
    //  BUG! There should be no need to call UpdateData in this function.
    //  See BUG: Finish Button Fails Data Transfer from Page to Variables
    //  KB Article ID: Q150349
    //

    //
    //  Update the data in the class from the page.
    //

    if ( UpdateData( TRUE /*bSaveAndValidate*/ ) == FALSE )
    {
        return FALSE;
    } // if: error updating data

    // Save the data in the sheet.
    if ( BApplyChanges() == FALSE )
    {
        return FALSE;
    } // if: error applying changes

    return CPropertyPage::OnWizardFinish();

} //*** CBasePropertyPage::OnWizardFinish

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::OnChangeCtrl
//
//  Description:
//      Handler for the messages sent when a control is changed.  This
//      method can be specified in a message map if all that needs to be
//      done is enable the Apply button.
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
CBasePropertyPage::OnChangeCtrl( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    SetModified( TRUE );

} //*** CBasePropertyPage::OnChangeCtrl

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::EnableNext
//
//  Description:
//      Enables or disables the NEXT or FINISH button.
//
//  Arguments:
//      fEnableIn
//          TRUE = enable the button, FALSE = disable the button.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CBasePropertyPage::EnableNext(
    BOOL fEnableIn      // = TRUE
    )
{
    ASSERT( BWizard() );
    ASSERT( PiWizardCallback() );

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    PiWizardCallback()->EnableNext( reinterpret_cast< LONG * >( Hpage() ), fEnableIn );

} //*** CBasePropertyPage::EnableNext

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::BApplyChanges
//
//  Description:
//      Apply changes made on the page.
//
//  Arguments:
//      None.
//
//  Return Value:
//      TRUE
//          Page successfully applied.
//
//      FALSE
//          Error applying page.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
CBasePropertyPage::BApplyChanges( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CWaitCursor wc;

    //
    //  Save data.
    //

    return BSetPrivateProps();

} //*** CBasePropertyPage::BApplyChanges

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::BBuildPropList
//
//  Description:
//      Build the property list.
//
//  Arguments:
//      rcplIn
//          Cluster property list.
//
//      fNoNewPropsIn
//          TRUE = exclude properties marked with opfNew.
//
//  Return Value:
//      TRUE
//          Property list built successfully.
//
//      FALSE
//          Error building property list.
//
//  Exceptions Thrown:
//      Any exceptions thrown by CClusPropList::AddProp().
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
CBasePropertyPage::BBuildPropList(
      CClusPropList &   rcplIn
    , BOOL              fNoNewPropsIn     // = FALSE
    )
{
    BOOL                    fNewPropsFound = FALSE;
    DWORD                   cprop;
    const CObjectProperty * pprop;

    for ( pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop-- )
    {
        if ( fNoNewPropsIn && (pprop->m_dwFlags & CObjectProperty::opfNew) )
        {
            fNewPropsFound = TRUE;
            continue;
        } // if: no new props allowed and this is a new property

        switch ( pprop->m_propFormat )
        {
            case CLUSPROP_FORMAT_SZ:
                rcplIn.ScAddProp(
                          pprop->m_pwszName
                        , *pprop->m_value.pstr
                        , *pprop->m_valuePrev.pstr
                        );
                break;

            case CLUSPROP_FORMAT_EXPAND_SZ:
                rcplIn.ScAddExpandSzProp(
                          pprop->m_pwszName
                        , *pprop->m_value.pstr
                        , *pprop->m_valuePrev.pstr
                        );
                break;

            case CLUSPROP_FORMAT_DWORD:
                rcplIn.ScAddProp(
                          pprop->m_pwszName
                        , *pprop->m_value.pdw
                        , *pprop->m_valuePrev.pdw
                        );
                break;

            case CLUSPROP_FORMAT_LONG:
                rcplIn.ScAddProp(
                          pprop->m_pwszName
                        , *pprop->m_value.pl
                        , *pprop->m_valuePrev.pl
                        );
                break;

            case CLUSPROP_FORMAT_BINARY:
            case CLUSPROP_FORMAT_MULTI_SZ:
                rcplIn.ScAddProp(
                          pprop->m_pwszName
                        , *pprop->m_value.ppb
                        , *pprop->m_value.pcb
                        , *pprop->m_valuePrev.ppb
                        , *pprop->m_valuePrev.pcb
                        );
                break;

            default:
                ASSERT( 0 ); // don't know how to deal with this type
                return FALSE;

        } // switch: property format
    } // for: each property

    return ( !( fNoNewPropsIn || fNewPropsFound ) );

} //*** CBasePropertyPage::BBuildPropList

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::BSetPrivateProps
//
//  Description:
//      Set the private properties for this object.
//
//  Arguments:
//      fValidateOnlyIn
//          TRUE = only validate the data.
//
//      fNoNewPropsIn
//          TRUE = exclude properties marked with opfNew.
//
//  Return Value:
//      ERROR_SUCCESS
//          The operation was completed successfully.
//
//      Other Win32 values.
//          Failure.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
CBasePropertyPage::BSetPrivateProps(
      BOOL fValidateOnlyIn   // = FALSE
    , BOOL fNoNewPropsIn     // = FALSE
    )
{
    BOOL            fSuccess   = TRUE;
    CClusPropList   cpl( BWizard() /*bAlwaysAddProp*/ );

    ASSERT( Peo() != NULL );
    ASSERT( Peo()->PodObjData() );

    //
    //  Build the property list.
    //

    try
    {
        fSuccess = BBuildPropList( cpl, fNoNewPropsIn );
    } // try
    catch ( CException * pe )
    {
        pe->ReportError();
        pe->Delete();
        fSuccess = FALSE;
    } // catch: CException

    //
    //  Set the data.
    //

    if ( fSuccess )
    {
        if ( (cpl.PbPropList() != NULL) && (cpl.CbPropList() > 0) )
        {
            DWORD       sc = ERROR_SUCCESS;
            DWORD       dwControlCode;
            DWORD       cbProps;

            switch ( Cot() )
            {
                case CLUADMEX_OT_NODE:
                {
                    CNodeData * pcnd = reinterpret_cast< CNodeData * >( Peo()->PodObjData() );
                    ASSERT( pcnd && (pcnd->GetHNode() != NULL) );

                    //
                    //  Determine which control code to use.
                    //

                    if ( fValidateOnlyIn )
                    {
                        dwControlCode = CLUSCTL_NODE_VALIDATE_PRIVATE_PROPERTIES;
                    } // if: only validating data
                    else
                    {
                        dwControlCode = CLUSCTL_NODE_SET_PRIVATE_PROPERTIES;
                    } // else: setting data

                    //
                    //  Set private properties.
                    //

                    sc = ClusterNodeControl(
                                      pcnd->GetHNode()
                                    , NULL   // hNode
                                    , dwControlCode
                                    , cpl.PbPropList()
                                    , static_cast< DWORD >( cpl.CbPropList() )
                                    , NULL   // lpOutBuffer
                                    , 0      // nOutBufferSize
                                    , &cbProps
                                    );
                    break;
                } // case:

                case CLUADMEX_OT_GROUP:
                {
                    CGroupData * pcgd = reinterpret_cast< CGroupData * >( Peo()->PodObjData() );
                    ASSERT( pcgd && (pcgd->GetHGroup() != NULL) );

                    //
                    //  Determine which control code to use.
                    //

                    if ( fValidateOnlyIn )
                    {
                        dwControlCode = CLUSCTL_GROUP_VALIDATE_PRIVATE_PROPERTIES;
                    } // if: only validating data
                    else
                    {
                        dwControlCode = CLUSCTL_GROUP_SET_PRIVATE_PROPERTIES;
                    } // else: setting data

                    //
                    //  Set private properties.
                    //

                    sc = ClusterGroupControl(
                                      pcgd->GetHGroup()
                                    , NULL   // hNode
                                    , dwControlCode
                                    , cpl.PbPropList()
                                    , static_cast< DWORD >( cpl.CbPropList() )
                                    , NULL   // lpOutBuffer
                                    , 0      // nOutBufferSize
                                    , &cbProps
                                    );
                    break;
                } // case:

                case CLUADMEX_OT_RESOURCE:
                {
                    CResData * pcrd = reinterpret_cast< CResData * >( Peo()->PodObjData() );
                    ASSERT( pcrd && (pcrd->GetHResource() != NULL) );

                    //
                    //  Determine which control code to use.
                    //

                    if ( fValidateOnlyIn )
                    {
                        dwControlCode = CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES;
                    } // if: only validating data
                    else
                    {
                        dwControlCode = CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES;
                    } // else: setting data

                    //
                    //  Set private properties.
                    //

                    sc = ClusterResourceControl(
                                      pcrd->GetHResource()
                                    , NULL   // hNode
                                    , dwControlCode
                                    , cpl.PbPropList()
                                    , static_cast< DWORD >( cpl.CbPropList() )
                                    , NULL   // lpOutBuffer
                                    , 0      // nOutBufferSize
                                    , &cbProps
                                    );
                    break;
                } // case:

                case CLUADMEX_OT_RESOURCETYPE:
                {
                    CResTypeData * pcrd = reinterpret_cast< CResTypeData * >( Peo()->PodObjData() );
                    ASSERT( pcrd && (pcrd->StrName().GetLength() > 0) );

                    //
                    //  Determine which control code to use.
                    //

                    if ( fValidateOnlyIn )
                    {
                        dwControlCode = CLUSCTL_RESOURCE_TYPE_VALIDATE_PRIVATE_PROPERTIES;
                    } // if: only validating data
                    else
                    {
                        dwControlCode = CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES;
                    } // else: setting data

                    //
                    //  Set private properties.
                    //

                    sc = ClusterResourceTypeControl(
                                      Hcluster()
                                    , pcrd->StrName()
                                    , NULL   // hNode
                                    , dwControlCode
                                    , cpl.PbPropList()
                                    , static_cast< DWORD >( cpl.CbPropList() )
                                    , NULL   // lpOutBuffer
                                    , 0      // nOutBufferSize
                                    , &cbProps
                                    );
                    break;
                } // case:

                case CLUADMEX_OT_NETWORK:
                {
                    CNetworkData * pcnd = reinterpret_cast< CNetworkData * >( Peo()->PodObjData() );
                    ASSERT( pcnd && (pcnd->GetHNetwork() != NULL) );

                    //
                    //  Determine which control code to use.
                    //

                    if ( fValidateOnlyIn )
                    {
                        dwControlCode = CLUSCTL_NETWORK_VALIDATE_PRIVATE_PROPERTIES;
                    } // if: only validating data
                    else
                    {
                        dwControlCode = CLUSCTL_NETWORK_SET_PRIVATE_PROPERTIES;
                    } // else: setting data

                    //
                    //  Set private properties.
                    //

                    sc = ClusterNetworkControl(
                                      pcnd->GetHNetwork()
                                    , NULL   // hNode
                                    , dwControlCode
                                    , cpl.PbPropList()
                                    , static_cast< DWORD >( cpl.CbPropList() )
                                    , NULL   // lpOutBuffer
                                    , 0      // nOutBufferSize
                                    , &cbProps
                                    );
                    break;
                } // case:

                case CLUADMEX_OT_NETINTERFACE:
                {
                    CNetInterfaceData * pcnd = reinterpret_cast< CNetInterfaceData * >( Peo()->PodObjData() );
                    ASSERT( pcnd && (pcnd->GetHNetInterface() != NULL) );

                    //
                    //  Determine which control code to use.
                    //

                    if ( fValidateOnlyIn )
                    {
                        dwControlCode = CLUSCTL_NETINTERFACE_VALIDATE_PRIVATE_PROPERTIES;
                    } // if: only validating data
                    else
                    {
                        dwControlCode = CLUSCTL_NETINTERFACE_SET_PRIVATE_PROPERTIES;
                    } // else: setting data

                    //
                    //  Set private properties.
                    //

                    sc = ClusterNetInterfaceControl(
                                      pcnd->GetHNetInterface()
                                    , NULL   // hNode
                                    , dwControlCode
                                    , cpl.PbPropList()
                                    , static_cast< DWORD >( cpl.CbPropList() )
                                    , NULL   // lpOutBuffer
                                    , 0      // nOutBufferSize
                                    , &cbProps
                                    );
                    break;
                } // case:

                default:
                    ASSERT( 0 );

            } // switch: object type

            //
            //  Handle errors.
            //

            if ( sc != ERROR_SUCCESS )
            {
                if ( sc == ERROR_INVALID_PARAMETER )
                {
                    if ( ! fNoNewPropsIn )
                    {
                        fSuccess = BSetPrivateProps( fValidateOnlyIn, TRUE /*fNoNewPropsIn*/ );
                    } // if: new props are allowed
                    else
                        fSuccess = FALSE;
                } // if: invalid parameter error occurred
                else
                {
                    fSuccess = FALSE;
                } // else: some other error occurred

                //
                // If an error occurred, display an error message.
                //

                if ( ! fSuccess )
                {
                    DisplaySetPropsError( sc, fValidateOnlyIn ? IDS_ERROR_VALIDATING_PROPERTIES : IDS_ERROR_SETTING_PROPERTIES );
                    if ( sc == ERROR_RESOURCE_PROPERTIES_STORED )
                    {
                        fSuccess = TRUE;
                    } // if: properties only stored
                } // if: error occurred
            } // if: error setting/validating data
        } // if: there is data to set
    } // if: no errors building the property list

    //
    //  Save data locally.
    //

    if ( ! fValidateOnlyIn && fSuccess )
    {
        //
        //  Save new values as previous values.
        //

        try
        {
            DWORD                   cprop;
            const CObjectProperty * pprop;

            for ( pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop-- )
            {
                switch ( pprop->m_propFormat )
                {
                    case CLUSPROP_FORMAT_SZ:
                    case CLUSPROP_FORMAT_EXPAND_SZ:
                        ASSERT(pprop->m_value.pstr != NULL);
                        ASSERT(pprop->m_valuePrev.pstr != NULL);
                        *pprop->m_valuePrev.pstr = *pprop->m_value.pstr;
                        break;

                    case CLUSPROP_FORMAT_DWORD:
                    case CLUSPROP_FORMAT_LONG:
                        ASSERT( pprop->m_value.pdw != NULL );
                        ASSERT( pprop->m_valuePrev.pdw != NULL );
                        *pprop->m_valuePrev.pdw = *pprop->m_value.pdw;
                        break;

                    case CLUSPROP_FORMAT_BINARY:
                    case CLUSPROP_FORMAT_MULTI_SZ:
                        ASSERT( pprop->m_value.ppb != NULL );
                        ASSERT( *pprop->m_value.ppb != NULL );
                        ASSERT( pprop->m_value.pcb != NULL );
                        ASSERT( pprop->m_valuePrev.ppb != NULL );
                        ASSERT( *pprop->m_valuePrev.ppb != NULL );
                        ASSERT( pprop->m_valuePrev.pcb != NULL );
                        delete [] *pprop->m_valuePrev.ppb;
                        *pprop->m_valuePrev.ppb = new BYTE[ *pprop->m_value.pcb ];
                        CopyMemory( *pprop->m_valuePrev.ppb, *pprop->m_value.ppb, *pprop->m_value.pcb );
                        *pprop->m_valuePrev.pcb = *pprop->m_value.pcb;
                        break;

                    default:
                        ASSERT( 0 ); // don't know how to deal with this type

                } // switch: property format
            } // for: each property
        } // try
        catch ( CException * pe )
        {
            pe->ReportError();
            pe->Delete();
            fSuccess = FALSE;
        } // catch: CException
    } // if: not just validating and successful so far

    //
    // Indicate we successfully saved the properties.
    //

    if ( ! fValidateOnlyIn && fSuccess )
    {
        m_bSaved = TRUE;
    } // if: successfully saved data

    return fSuccess;

} //*** CBasePropertyPage::BSetPrivateProps

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CBasePropertyPage::DisplaySetPropsError
//
//  Routine Description:
//      Display an error caused by setting or validating properties.
//
//  Arguments:
//      scIn
//          Status to display error on.
//
//      idsOperIn
//          Operation message.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CBasePropertyPage::DisplaySetPropsError(
      DWORD    scIn
    , UINT     idsOperIn
    ) const
{
    CString strErrorMsg;
    CString strOperMsg;
    CString strMsgIdFmt;
    CString strMsgId;
    CString strMsg;

    UNREFERENCED_PARAMETER( idsOperIn );

    strOperMsg.LoadString( IDS_ERROR_SETTING_PROPERTIES );
    FormatError( strErrorMsg, scIn );
    strMsgIdFmt.LoadString( IDS_ERROR_MSG_ID );
    strMsgId.Format( strMsgIdFmt, scIn, scIn );
    strMsg.Format( _T("%s\n\n%s%s"), strOperMsg, strErrorMsg, strMsgId );
    AfxMessageBox( strMsg );

}  //*** CBasePropertyPage::DisplaySetPropsError

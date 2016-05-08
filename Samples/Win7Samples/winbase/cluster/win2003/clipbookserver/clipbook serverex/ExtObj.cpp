/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      ExtObj.cpp
//
//  Description:
//      Implementation of the CExtObject class, which implements the
//      extension interfaces required by a Microsoft Windows NT Cluster
//      Administrator Extension DLL.
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
#include "ExtObj.h"
#include "ExtObjData.h"
#include "ResProp.h"

/////////////////////////////////////////////////////////////////////////////
// CExtObject
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CExtObject::CExtObject
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
CExtObject::CExtObject( void )
{
    m_piData = NULL;
    m_piWizardCallback = NULL;
    m_bWizard = FALSE;

    m_lcid = NULL;
    m_hfont = NULL;
    m_hicon = NULL;
    m_hCluster = NULL;
    m_cobj = 0;
    m_podObjData = NULL;

} //*** CExtObject::CExtObject

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CExtObject::~CExtObject
//
//  Description:
//      Destructor.
//
//  Arguments:
//      None.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
CExtObject::~CExtObject( void )
{
    POSITION    pos = NULL;

    //
    // Release the data interface.
    //

    if ( m_piData != NULL )
    {
        m_piData->Release();
        m_piData = NULL;
    } // if:  we have a data interface pointer

    //
    // Release the wizard callback interface.
    //

    if ( m_piWizardCallback != NULL )
    {
        m_piWizardCallback->Release();
        m_piWizardCallback = NULL;
    } // if:  we have a wizard callback interface pointer

    //
    // Delete the pages.
    //

    pos = m_lpg.GetHeadPosition();
    while ( pos != NULL )
    {
        delete m_lpg.GetNext(pos);
    } // while:  more pages in the list

    //
    // Delete the object.
    //

    delete m_podObjData;
    m_podObjData = NULL;

} //*** CExtObject::~CExtObject

/////////////////////////////////////////////////////////////////////////////
// ISupportErrorInfo Implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CExtObject::InterfaceSupportsErrorInfo (ISupportErrorInfo)
//
//  Routine Description:
//      Indicates whether an interface suportes the IErrorInfo interface.
//      This interface is provided by ATL.
//
//  Arguments:
//      riid        Interface ID.
//
//  Return Value:
//      S_OK        Interface supports IErrorInfo.
//      S_FALSE     Interface does not support IErrorInfo.
//
//--
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP
CExtObject::InterfaceSupportsErrorInfo(
    REFIID riid
    )
{
    HRESULT hr = S_FALSE;
    int iiid = 0;
    static const IID * rgiid[] =
    {
          &IID_IWEExtendPropertySheet
        , &IID_IWEExtendWizard
    };

    for ( iiid = 0 ; iiid < sizeof( rgiid ) / sizeof( rgiid[ 0 ] ) ; iiid++ )
    {
        if ( ::InlineIsEqualGUID( *rgiid[ iiid ], riid ) )
        {
            hr = S_OK;
            goto Cleanup;
        } // if: found a matching IID
    } // for: each interface we support

Cleanup:

    return hr;

} //*** CExtObject::InterfaceSupportsErrorInfo

/////////////////////////////////////////////////////////////////////////////
// IWEExtendPropertySheet Implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CExtObject::CreatePropertySheetPages (IWEExtendPropertySheet)
//
//  Description:
//      Create property sheet pages and add them to the sheet.
//
//  Arguments:
//      piDataIn
//          IUnkown pointer from which to obtain interfaces for obtaining data
//          describing the object for which the sheet is being displayed.
//
//      piCallbackIn
//          Pointer to an IWCPropertySheetCallback interface for adding pages
//          to the sheet.
//
//  Return Value:
//      S_OK         Pages added successfully.
//      E_INVALIDARG    Invalid arguments to the function.
//      E_OUTOFMEMORY   Error allocating memory.
//      E_FAIL          Error creating a page.
//      E_NOTIMPL       Not implemented for this type of data.
//      hr             Any error codes from HrGetUIInfo() or HrSaveData().
//
//--
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP
CExtObject::CreatePropertySheetPages(
      IUnknown *                   piDataIn
    , IWCPropertySheetCallback *   piCallbackIn
    )
{
    HRESULT             hr = S_OK;
    CRuntimeClass **    pprtc = NULL;
    int                 irtc = 0;
    CBasePropertyPage * ppage = NULL;
    POSITION            pos = NULL;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    //
    // Validate the parameters.
    //

    if ( (piDataIn == NULL) || (piCallbackIn == NULL) )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    } // if:  all interfaces not specified

    //
    //  Since the MFC collection classes can throw exceptions a minimal try
    //  catch block is needed.
    //

    try
    {
        //
        // Get info about displaying UI.
        //

        hr = HrGetUIInfo( piDataIn );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        } // if:  error getting UI info

        //
        // Save the data.
        //

        hr = HrSaveData( piDataIn );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        } // if:  error saving data from host

        //
        // Delete any previous pages.
        //

        pos = m_lpg.GetHeadPosition();
        while ( pos != NULL )
        {
            delete m_lpg.GetNext( pos );
        } // while:  more pages in the list

        m_lpg.RemoveAll();

        //
        // Create property pages.
        //

        ASSERT( m_podObjData != NULL );
        hr = m_podObjData->GetPropertySheetPages( &pprtc );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        } // if: failed to get the property sheet pages.

        //
        // Create each page.
        //

        for ( irtc = 0 ; pprtc[ irtc ] != NULL ; irtc++ )
        {
            //
            // Create the page.
            //

            ppage = static_cast< CBasePropertyPage * >( pprtc[ irtc ]->CreateObject() );
            ASSERT( ppage->IsKindOf( pprtc[ irtc ] ) );

            //
            // Add it to the list.
            //

            m_lpg.AddTail( ppage );

            //
            // Initialize the property page.
            //

            hr = ppage->HrInit( this );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            } // if:  error initializing the page

            //
            // Create the page.
            //

            hr = ppage->HrCreatePage();
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            } // if:  error creating the page

            //
            // Add it to the property sheet.
            //

            hr = piCallbackIn->AddPropertySheetPage( reinterpret_cast< LONG * >( ppage->Hpage() ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            } // if:  error adding the page to the sheet
        } // for:  each page in the list
    } // try:
    catch ( CMemoryException * pme )
    {
        //
        //  An MFC memory exception occurred.
        //

        TRACE( _T("CExtObject::CreatePropetySheetPages() - Failed to add property page\n") );

        pme->Delete();
        hr = E_OUTOFMEMORY;
    } // catch:  MFC memory exception.
    catch( ... )
    {
        //
        //  An unknown exception occurred.
        //

        hr = E_UNEXPECTED;
    } // catch: anything.

Cleanup:

    //
    //  Contractually obligated to release these interfaces. Cluadmin did
    //  an AddRef on them before we were called.
    //

    piDataIn->Release();
    piCallbackIn->Release();

    return hr;

} //*** CExtObject::CreatePropertySheetPages

/////////////////////////////////////////////////////////////////////////////
// IWEExtendWizard Implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CExtObject::CreateWizardPages (IWEExtendWizard)
//
//  Description:
//      Create property sheet pages and add them to the wizard.
//
//  Arguments:
//      piDataIn
//          IUnkown pointer from which to obtain interfaces for obtaining data
//          describing the object for which the wizard is being displayed.
//
//      piCallbackIn
//          Pointer to an IWCPropertySheetCallback interface for adding pages
//          to the sheet.
//
//  Return Value:
//      S_OK         Pages added successfully.
//      E_INVALIDARG    Invalid arguments to the function.
//      E_OUTOFMEMORY   Error allocating memory.
//      E_FAIL          Error creating a page.
//      E_NOTIMPL       Not implemented for this type of data.
//      hr             Any error codes from HrGetUIInfo() or HrSaveData().
//
//--
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP
CExtObject::CreateWizardPages(
      IUnknown *           piDataIn
    , IWCWizardCallback *  piCallbackIn
    )
{
    HRESULT             hr = S_OK;
    CRuntimeClass **    pprtc = NULL;
    int                 irtc = 0;
    CBasePropertyPage * ppage = NULL;
    POSITION            pos = NULL;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    //
    // Validate the parameters.
    //

    if ( (piDataIn == NULL) || (piCallbackIn == NULL) )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    } // if:  all interfaces not specified

    //
    //  Since the MFC collection classes can throw exceptions a minimal try
    //  catch block is needed.
    //

    try
    {
        //
        // Get info about displaying UI.
        //

        hr = HrGetUIInfo( piDataIn );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        } // if:  error getting UI info

        //
        // Save the data.
        //

        hr = HrSaveData( piDataIn );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        } // if:  error saving data from host

        //
        // Delete any previous pages.
        //

        pos = m_lpg.GetHeadPosition();
        while ( pos != NULL )
        {
            delete m_lpg.GetNext( pos );
        } // while:  more pages in the list

        m_lpg.RemoveAll();

        //
        // Update our callback pointer.
        //

        piCallbackIn->AddRef();
        m_piWizardCallback = piCallbackIn;

        m_bWizard = TRUE;

        //
        // Create property pages.
        //

        ASSERT( m_podObjData != NULL );
        hr = m_podObjData->GetWizardPages( &pprtc );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        } // if: failed to get the wizard pages.

        //
        // Create each page.
        //

        for ( irtc = 0 ; pprtc[ irtc ] != NULL ; irtc++ )
        {
            //
            // Create the page.
            //

            ppage = static_cast< CBasePropertyPage * >( pprtc[ irtc ]->CreateObject() );
            ASSERT( ppage->IsKindOf( pprtc[ irtc ] ) );

            //
            // Add it to the list.
            //

            m_lpg.AddTail( ppage );

            //
            // Initialize the property page.
            //

            hr = ppage->HrInit( this );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            } // if:  error initializing the page

            //
            // Create the page.
            //

            hr = ppage->HrCreatePage();
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            } // if:  error creating the page

            //
            // Add it to the property sheet.
            //

            hr = piCallbackIn->AddWizardPage( reinterpret_cast< LONG * >( ppage->Hpage() ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            } // if:  error adding the page to the sheet
        } // for:  each page in the list
    } // try:
    catch ( CMemoryException * pme )
    {
        //
        //  An MFC memory exception occurred.
        //

        TRACE( _T("CExtObject::CreateWizardPages() - Failed to add wizard page\n") );

        pme->Delete();
        hr = E_OUTOFMEMORY;
    } // catch: MFC memory exception.
    catch( ... )
    {
        //
        //  An unknown exception occurred.
        //

        hr = E_UNEXPECTED;
    } // catch: anything.

Cleanup:

    //
    //  If we failed for any reason then we need to cleanup up the wizard
    //  callback interface if we saved it off.
    //

    if ( FAILED( hr ) )
    {
        if ( m_piWizardCallback == piCallbackIn )
        {
            m_piWizardCallback->Release();
            m_piWizardCallback = NULL;
        } // if: already saved interface pointer
    } // if:  error occurred

    //
    //  Contractually obligated to release these interfaces. Cluadmin did
    //  an AddRef on them before we were called.
    //

    piDataIn->Release();
    piCallbackIn->Release();

    return hr;

} //*** CExtObject::CreateWizardPages

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CExtObject::HrGetUIInfo
//
//  Description:
//      Get info about displaying UI.
//
//  Arguments:
//      piDataIn
//          IUnkown pointer from which to obtain interfaces for obtaining data
//          describing the object.
//
//  Return Value:
//      S_OK
//          Data saved successfully.
//
//      E_NOTIMPL
//          Not implemented for this type of data.
//
//      hr
//          Any error codes from IUnknown::QueryInterface(),
//          HrGetObjectName(), or HrGetResourceName().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CExtObject::HrGetUIInfo(
    IUnknown * piDataIn
    )
{
    ASSERT( piDataIn != NULL );

    HRESULT             hr = S_OK;
    IGetClusterUIInfo * pi = NULL;

    //
    // Save info about all types of objects.
    //

    hr = piDataIn->QueryInterface( IID_IGetClusterUIInfo, reinterpret_cast< LPVOID * >( &pi ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if:  error querying for interface

    m_lcid = pi->GetLocale();
    m_hfont = pi->GetFont();
    m_hicon = pi->GetIcon();

Cleanup:

    if ( pi != NULL )
    {
        pi->Release();
    } // if:

    return hr;

} //*** CExtObject::HrGetUIInfo

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CExtObject::HrSaveData
//
//  Routine Description:
//      Save data from the object so that it can be used for the life
//      of the object.
//
//  Arguments:
//      piDataIn
//          IUnkown pointer from which to obtain interfaces for obtaining data
//          describing the object.
//
//  Return Value:
//      S_OK
//          Data saved successfully.
//
//      E_NOTIMPL
//          Not implemented for this type of data.
//
//      hr
//          Any error codes from IUnknown::QueryInterface(),
//          HrGetObjectName(), or HrGetResourceName().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CExtObject::HrSaveData(
    IUnknown * piDataIn
    )
{
    ASSERT( piDataIn != NULL );

    HRESULT                 hr = S_OK;
    IGetClusterDataInfo *   pi = NULL;

    //
    //  Save the new piData interface.
    //

    if ( piDataIn != m_piData )
    {
        if ( m_piData != NULL )
        {
            m_piData->Release();
        } // if:  interface queried for previously

        piDataIn->AddRef();
        m_piData = piDataIn;
    } // if:  different data interface pointer

    //
    // Save info about all types of objects.
    //

    hr = piDataIn->QueryInterface( IID_IGetClusterDataInfo, reinterpret_cast< LPVOID * >( &pi ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if:  error querying for interface

    m_hCluster = pi->GetClusterHandle();

    m_cobj = pi->GetObjectCount();
    if ( m_cobj != 1 )  // We only have support for one selected object.
    {
        hr = E_NOTIMPL;
        goto Cleanup;
    } // if:  too many objects for us to handle

    //
    // Save info about this object.
    //

    hr = HrGetObjectInfo();
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if:

Cleanup:

    if ( pi != NULL )
    {
        pi->Release();
    } // if:

    return hr;

} //*** CExtObject::HrSaveData

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CExtObject::HrGetObjectInfo
//
//  Description:
//      Get information about the object.
//
//  Arguments:
//      None.
//
//  Return Value:
//      S_OK
//          Data saved successfully.
//
//      E_OUTOFMEMORY
//         Error allocating memory.
//
//      E_NOTIMPL
//          Not implemented for this type of data.
//
//      hr
//          Any error codes from IUnknown::QueryInterface(),
//          HrGetObjectName(), or HrGetResourceTypeName().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CExtObject::HrGetObjectInfo( void )
{
    HRESULT hr = S_OK;

    //
    // Delete the old object and create a new one.
    //

    delete m_podObjData;
    m_podObjData = NULL;

    hr = CObjData::S_HrCreateObject( m_piData, &m_podObjData );

    return hr;

} //*** CExtObject::HrGetObjectInfo

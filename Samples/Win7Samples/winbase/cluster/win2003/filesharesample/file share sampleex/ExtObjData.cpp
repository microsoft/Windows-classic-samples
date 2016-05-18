/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      ExtObjData.cpp
//
//  Description:
//      Implementation of the context menu functions and functions for
//      returning the property sheet and wizard pages.
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
#include "ExtObjData.h"
#include "resprop.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Constant Definitions
/////////////////////////////////////////////////////////////////////////////

#define CLUSTER_MENU_BASE_ID        0
#define CLUSTER_MENU_ID_1           (CLUSTER_MENU_BASE_ID + 1)

#define NODE_MENU_BASE_ID           10
#define NODE_MENU_ID_1              (NODE_MENU_BASE_ID + 1)

#define GROUP_MENU_BASE_ID          20
#define GROUP_MENU_ID_1             (GROUP_MENU_BASE_ID + 1)

#define RESOURCE_MENU_BASE_ID       30
#define RESOURCE_MENU_ID_1          (RESOURCE_MENU_BASE_ID + 1)

#define ALL_RESOURCES_MENU_BASE_ID  40
#define ALL_RESOURCES_MENU_ID_1     (ALL_RESOURCES_MENU_BASE_ID + 1)

#define RESTYPE_MENU_BASE_ID        50
#define RESTYPE_MENU_ID_1           (RESTYPE_MENU_BASE_ID + 1)

#define ALL_RESTYPES_MENU_BASE_ID   60
#define ALL_RESTYPES_MENU_ID_1      (ALL_RESTYPES_MENU_BASE_ID + 1)

#define NETWORK_MENU_BASE_ID        70
#define NETWORK_MENU_ID_1           (NETWORK_MENU_BASE_ID + 1)

#define NETINTERFACE_MENU_BASE_ID   80
#define NETINTERFACE_MENU_ID_1      (NETINTERFACE_MENU_BASE_ID + 1)

/////////////////////////////////////////////////////////////////////////////
// CObjData implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CObjData::S_HrCreateObject
//
//  Description:
//      Create a CObjData-derived object based on the requesting object type.
//
//  Arguments:
//
//  Return Value:
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CObjData::S_HrCreateObject(
      IUnknown *    piDataIn
    , CObjData **   ppodOut
    )
{
    HRESULT                 hr = S_OK;
    CObjData *              podData = NULL;
    CLUADMEX_OBJECT_TYPE    cot;
    IGetClusterObjectInfo * piObjInfo = NULL;

    ASSERT( piDataIn != NULL );
    ASSERT( ppodOut != NULL );

    hr = piDataIn->QueryInterface( IID_IGetClusterObjectInfo, reinterpret_cast< LPVOID * >( &piObjInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error querying for interface

    cot = piObjInfo->GetObjectType( 0 );
    piObjInfo->Release();
    piObjInfo = NULL;

    switch ( cot )
    {
        case CLUADMEX_OT_RESOURCE:
            podData = new CResData();
            break;

        case CLUADMEX_OT_RESOURCETYPE:
            podData = new CResTypeData();
            break;

        default:
            hr = E_NOTIMPL;
            goto Cleanup;
            break;
    } // switch: object type

    if ( podData == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    } // if: error allocating the object

    hr = podData->HrInitialize( piDataIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error initializing the object

    *ppodOut = podData;
    podData = NULL;         // We gave away ownership.

Cleanup:

    if ( piObjInfo != NULL )
    {
        piObjInfo->Release();
    } // if:

    delete podData;

    return hr;

} //*** CObjData::S_HrCreateObject

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CObjData::AddContextMenuItem
//
//  Description:
//      Add a context menu item.
//
//  Arguments:
//      piCallbackIn
//          Pointer to an IWCContextMenuCallback interface for adding menu
//          items to the context menu.
//
//  Return Value:
//      S_OK
//          Menu items added successfully.
//
//      hr
//          Any errors returned by IWCContextMenuCallback
//          ::AddExtensionMenuItem().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CObjData::AddContextMenuItem(
      IWCContextMenuCallback *  piCallbackIn
    , UINT                      nMenuItem
    , UINT                      nMenuItemDesc
    , UINT                      nBaseID
    , ULONG                     uFlags
    )
{
    HRESULT hr = S_OK;
    CString strMenuItem;
    CString strStatusBarText;
    LPWSTR  pwszMenuItem = NULL;
    LPWSTR  pwszStatusBarText = NULL;

    ASSERT( piCallbackIn != NULL );

    //
    //  Get strings.
    //

    strMenuItem.LoadString( nMenuItem );
    strStatusBarText.LoadString( nMenuItemDesc );
    pwszMenuItem = strMenuItem.GetBuffer( 1 );
    pwszStatusBarText = strStatusBarText.GetBuffer( 1 );

    //
    //  Add item to the context menu.
    //

    hr = piCallbackIn->AddExtensionMenuItem(
                              pwszMenuItem            // lpszName
                            , pwszStatusBarText       // lpszStatusBarText
                            , nBaseID                 // lCommandID
                            , 0                       // lSubCommandID - reserved, must be 0
                            , uFlags                  // uFlags
                            );

    //
    //  Release strings.
    //

    strMenuItem.ReleaseBuffer();
    strStatusBarText.ReleaseBuffer();

    return hr;

} //*** CObjData::AddContextMenuItem

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CObjData::HrInitialize
//
//  Description:
//      Initializes the object.
//
//  Description:
//
//  Arguments:
//
//  Return Value:
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CObjData::HrInitialize(
      IUnknown *    piDataIn
    )
{
    HRESULT                 hr = S_OK;
    BSTR                    bstrName = NULL;
    LONG                    cchName = 0;
    IGetClusterObjectInfo * piObjInfo = NULL;

    ASSERT( piDataIn != NULL );

    //
    // Get an interface pointer to IGetClusterObjectInfo.
    //

    hr = piDataIn->QueryInterface( IID_IGetClusterObjectInfo, reinterpret_cast< LPVOID * >( &piObjInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error querying for interface

    //
    // Allocate a buffer for the object name.
    //

    hr = piObjInfo->GetObjectName( 0, NULL, &cchName );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error getting the name length

    bstrName = SysAllocStringLen( NULL, cchName );
    if ( bstrName == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    } // if: error allocating a string

    //
    // Retrieve the object name.
    //

    hr = piObjInfo->GetObjectName( 0, bstrName, &cchName );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error getting the name

    //
    // The CString assignment operator makes a copy.
    //

    m_strName = bstrName;

Cleanup:

    SysFreeString( bstrName );

    if ( piObjInfo != NULL )
    {
        piObjInfo->Release();
    } // if:

    return hr;

} //*** CObjData::HrInitialize

/////////////////////////////////////////////////////////////////////////////
// CClusterData implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterData::HrInitialize
//
//  Description:
//      Initializes the object.
//
//  Arguments:
//
//  Return Value:
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CClusterData::HrInitialize(
    IUnknown * piDataIn
    )
{
    HRESULT                 hr = S_OK;
    IGetClusterDataInfo *   piClusInfo = NULL;
    LONG                    cchClusterName = 0;
    BSTR                    bstrClusterName = NULL;

    ASSERT( piDataIn != NULL );

    //
    // Initialize the base class.
    //

    hr = CObjData::HrInitialize( piDataIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error initializing the base class

    //
    // Initialize member variables to default values.
    //

    m_hCluster = NULL;

    //
    // Set the object type.
    //

    m_cot = CLUADMEX_OT_CLUSTER;

    //
    // Get an interface pointer to IGetClusterDataInfo.
    //

    hr = piDataIn->QueryInterface( IID_IGetClusterDataInfo, reinterpret_cast< LPVOID * >( &piClusInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error querying for interface

    //
    // Get a copy of the handle.  Don't free this handle - it's just a copy.
    //

    m_hCluster = piClusInfo->GetClusterHandle();
    ASSERT( m_hCluster != NULL );
    if ( m_hCluster == NULL )
    {
        hr = E_HANDLE;
        goto Cleanup;
    } // if: error querying for interface

    //
    // Allocate a buffer for and retrieve the name.
    //

    hr = piClusInfo->GetClusterName( NULL, &cchClusterName );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error getting the name length

    bstrClusterName = SysAllocStringLen( NULL, cchClusterName );
    if ( bstrClusterName == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    } // if: error allocating a string

    hr = piClusInfo->GetClusterName( bstrClusterName, &cchClusterName );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error getting the name

    //
    // The CString assignment operator makes a copy.
    //

    m_strClusterName = bstrClusterName;

Cleanup:

    if ( piClusInfo != NULL )
    {
        piClusInfo->Release();
    } // if:

    SysFreeString( bstrClusterName );

    return hr;

} //*** CClusterData::HrInitialize

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterData::AddContextMenuItems
//
//  Description:
//      Add menu items to the cluster object context menu.
//
//  Arguments:
//      piCallbackIn
//          Pointer to an IWCContextMenuCallback interface for adding menu
//          items to the context menu.
//
//  Return Value:
//      S_OK
//          Menu items added successfully.
//
//      hr
//          Any errors returned by IWCContextMenuCallback
//          ::AddExtensionMenuItem().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CClusterData::AddContextMenuItems(
      IWCContextMenuCallback * piCallbackIn
    )
{
    HRESULT hr = S_OK;

    ASSERT( piCallbackIn != NULL );

    hr = AddContextMenuItem(
                  piCallbackIn
                , IDS_CLUSTER_MENU_ITEM_1
                , IDS_CLUSTER_MENU_ITEM_1_DESC
                , CLUSTER_MENU_ID_1
                , MF_ENABLED
                );

    return hr;

} //*** CClusterData::AddContextMenuItems

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusterData::InvokeCommand
//
//  Description:
//      Invoke a command we added to a context menu.
//
//  Arguments:
//      nCommandIDIn
//          ID of the menu item to execute.  This is the same ID passed to the
//          IWCContextMenuCallback::AddExtensionMenuItem() method.
//
//  Return Value:
//      HRESULT
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CClusterData::InvokeCommand(
    ULONG nCommandIDIn
    )
{
    HRESULT hr = S_OK;

    switch ( nCommandIDIn )
    {
        case CLUSTER_MENU_ID_1:

            //
            //  TODO:  Implement your menu item here.
            //

            hr = S_OK;
            break;

        default:
            hr = E_NOTIMPL;
            break;
    } // switch: nCommandIDIn

    return hr;

} //*** CClusterData::InvokeCommand

/////////////////////////////////////////////////////////////////////////////
// CNodeData implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CNodeData::HrInitialize
//
//  Description:
//      Initializes the object.
//
//  Arguments:
//
//  Return Value:
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CNodeData::HrInitialize(
    IUnknown * piDataIn
    )
{
    HRESULT                 hr = S_OK;
    IGetClusterNodeInfo *   piNodeInfo = NULL;

    ASSERT( piDataIn != NULL );

    //
    // Initialize the base class.
    //

    hr = CObjData::HrInitialize( piDataIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error initializing the base class

    //
    // Initialize member variables to default values.
    //

    m_hNode = NULL;

    //
    // Set the object type.
    //

    m_cot = CLUADMEX_OT_NODE;

    //
    // Get an interface pointer to IGetClusterNodeInfo.
    //

    hr = piDataIn->QueryInterface( IID_IGetClusterNodeInfo, reinterpret_cast< LPVOID * >( &piNodeInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error querying for interface

    //
    // Get a copy of the handle.  Don't free this handle - it's just a copy.
    //

    m_hNode = piNodeInfo->GetNodeHandle( 0 );
    ASSERT( m_hNode != NULL );
    if ( m_hNode == NULL )
    {
        hr = E_HANDLE;
        goto Cleanup;
    } // if: error getting the node handle

Cleanup:

    if ( piNodeInfo != NULL )
    {
        piNodeInfo->Release();
    } // if:

    return hr;

} //*** CNodeData::HrInitialize

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CNodeData::AddContextMenuItems
//
//  Description:
//      Add menu items to the node object context menu.
//
//  Arguments:
//      piCallbackIn
//          Pointer to an IWCContextMenuCallback interface for adding menu
//          items to the context menu.
//
//  Return Value:
//      S_OK
//          Menu items added successfully.
//
//      hr
//          Any errors returned by IWCContextMenuCallback
//          ::AddExtensionMenuItem().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CNodeData::AddContextMenuItems(
    IWCContextMenuCallback * piCallbackIn
    )
{
    HRESULT hr = S_OK;

    ASSERT( piCallbackIn != NULL );

    hr = AddContextMenuItem(
              piCallbackIn
            , IDS_NODE_MENU_ITEM_1
            , IDS_NODE_MENU_ITEM_1_DESC
            , NODE_MENU_ID_1
            , MF_ENABLED
            );

    return hr;

} //*** CNodeData::AddContextMenuItems

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CNodeData::InvokeCommand
//
//  Description:
//      Invoke a command we added to a context menu.
//
//  Arguments:
//      nCommandIDIn
//          ID of the menu item to execute.  This is the same ID passed to the
//          IWCContextMenuCallback::AddExtensionMenuItem() method.
//
//  Return Value:
//      S_OK        Command invoked successfully.
//      E_NOTIMPL   Command ID not recognized.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CNodeData::InvokeCommand(
    ULONG nCommandIDIn
    )
{
    HRESULT hr = S_OK;

    switch ( nCommandIDIn )
    {
        case NODE_MENU_ID_1:

            //
            //  TODO:  Implement your menu item here
            //

            hr = S_OK;
            break;

        default:
            hr = E_NOTIMPL;
            break;
    } // switch: nCommandIDIn

    return hr;

} //*** CNodeData::InvokeCommand

/////////////////////////////////////////////////////////////////////////////
// CGroupData implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CGroupData::HrInitialize
//
//  Description:
//      Initializes the object.
//
//  Arguments:
//
//  Return Value:
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CGroupData::HrInitialize(
    IUnknown * piDataIn
    )
{
    HRESULT                 hr = S_OK;
    IGetClusterGroupInfo *  piGroupInfo = NULL;

    ASSERT( piDataIn != NULL );

    //
    // Initialize the base class.
    //

    hr = CObjData::HrInitialize( piDataIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error initializing the base class

    //
    // Initialize member variables to default values.
    //

    m_hGroup = NULL;

    //
    // Set the object type.
    //

    m_cot = CLUADMEX_OT_GROUP;

    //
    // Get an interface pointer to IGetClusterGroupInfo.
    //

    hr = piDataIn->QueryInterface( IID_IGetClusterGroupInfo, reinterpret_cast< LPVOID * >( &piGroupInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error querying for interface

    //
    // Get a copy of the handle.  Don't free this handle - it's just a copy.
    //

    m_hGroup = piGroupInfo->GetGroupHandle( 0 );
    ASSERT( m_hGroup != NULL );
    if ( m_hGroup == NULL )
    {
        hr = E_HANDLE;
        goto Cleanup;
    } // if: error getting the group handle

Cleanup:

    if ( piGroupInfo != NULL )
    {
        piGroupInfo->Release();
    } // if:

    return hr;

} //*** CGroupData::HrInitialize

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CGroupData::AddContextMenuItems
//
//  Description:
//      Add menu items to the group object context menu.
//
//  Arguments:
//      piCallbackIn
//          Pointer to an IWCContextMenuCallback interface for adding menu
//          items to the context menu.
//
//  Return Value:
//      S_OK
//          Menu items added successfully.
//
//      hr
//          Any errors returned by IWCContextMenuCallback
//          ::AddExtensionMenuItem().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT CGroupData::AddContextMenuItems(
      IWCContextMenuCallback * piCallbackIn
      )
{
    HRESULT hr = S_OK;

    ASSERT( piCallbackIn != NULL );

    hr = AddContextMenuItem(
              piCallbackIn
            , IDS_GROUP_MENU_ITEM_1
            , IDS_GROUP_MENU_ITEM_1_DESC
            , GROUP_MENU_ID_1
            , MF_ENABLED
            );

    return hr;

} //*** CGroupData::AddContextMenuItems

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CGroupData::InvokeCommand
//
//  Description:
//      Invoke a command we added to a context menu.
//
//  Arguments:
//      nCommandIDIn
//          ID of the menu item to execute.  This is the same ID passed to the
//          IWCContextMenuCallback::AddExtensionMenuItem() method.
//
//  Return Value:
//      S_OK        Command invoked successfully.
//      E_NOTIMPL   Command ID not recognized.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT CGroupData::InvokeCommand( ULONG nCommandIDIn )
{
    HRESULT hr = S_OK;

    switch ( nCommandIDIn )
    {
        case GROUP_MENU_ID_1:

            //
            //  TODO:  Implement your menu item here
            //

            hr = S_OK;
            break;

        default:
            hr = E_NOTIMPL;
            break;
    } // switch: nCommandIDIn

    return hr;

} //*** CGroupData::InvokeCommand

/////////////////////////////////////////////////////////////////////////////
// CResData implementation
/////////////////////////////////////////////////////////////////////////////

static CRuntimeClass * g_rgprtcResPSPages[] = {
    RUNTIME_CLASS( CFileShareSampleParamsPage ),
    NULL
    };

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CResData::HrInitialize
//
//  Description:
//      Initializes the object.
//
//  Arguments:
//
//  Return Value:
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CResData::HrInitialize(
    IUnknown * piDataIn
    )
{
    HRESULT                     hr = S_OK;
    IGetClusterResourceInfo *   piResInfo = NULL;
    LONG                        cchTypeName = 0;
    BSTR                        bstrTypeName = NULL;
    LPCWSTR                     pwszResTypes = g_wszResourceTypeNames;

    ASSERT( piDataIn != NULL );

    //
    // Initialize the base class.
    //

    hr = CObjData::HrInitialize( piDataIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error initializing the base class

    //
    // Initialize member variables to default values.
    //

    m_hResource = NULL;
    m_fSupportedType = FALSE;

    //
    // Set the object type.
    //

    m_cot = CLUADMEX_OT_RESOURCE;

    //
    // Set the pointers to our resource property sheet and wizard pages.
    //

    m_rgprtcPSPages = g_rgprtcResPSPages;
    m_rgprtcWizPages = g_rgprtcResPSPages;

    //
    // Get an interface pointer to IGetClusterResourceInfo.
    //

    hr = piDataIn->QueryInterface( IID_IGetClusterResourceInfo, reinterpret_cast< LPVOID * >( &piResInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error querying for interface

    //
    // Get a copy of the handle.  Don't free this handle, it's just a copy.
    //

    m_hResource = piResInfo->GetResourceHandle( 0 );
    ASSERT( m_hResource != NULL );
    if ( m_hResource == NULL )
    {
        hr = E_HANDLE;
        goto Cleanup;
    } // if: error querying for interface

    //
    // Allocate a buffer for and retrieve the resource type name.
    //

    hr = piResInfo->GetResourceTypeName( 0, NULL, &cchTypeName );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if:

    bstrTypeName = SysAllocStringLen( NULL, cchTypeName );
    if ( bstrTypeName == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    } // if:

    hr = piResInfo->GetResourceTypeName( 0, bstrTypeName, &cchTypeName );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if:

    //
    // The CString assignment operator makes a copy.
    //

    m_strResTypeName = bstrTypeName;

    //
    // Determine whether this is a supported type.
    //

    m_fSupportedType = FALSE;
    while ( *pwszResTypes != L'\0' )
    {
        if ( m_strResTypeName.CompareNoCase( pwszResTypes ) == 0 )
        {
            m_fSupportedType = TRUE;
            break;
        } // if:

        pwszResTypes += lstrlenW( pwszResTypes ) + 1;
    }  // while:  more resource types

Cleanup:

    if ( piResInfo != NULL )
    {
        piResInfo->Release();
    } // if:

    SysFreeString( bstrTypeName );

    return hr;

} //*** CResData::HrInitialize

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CResData::AddContextMenuItems
//
//  Description:
//      Add menu items to the resource object context menu.
//
//  Arguments:
//      piCallbackIn
//          Pointer to an IWCContextMenuCallback interface for adding menu
//          items to the context menu.
//
//  Return Value:
//      S_OK
//          Menu items added successfully.
//
//      hr
//          Any errors returned by IWCContextMenuCallback
//          ::AddExtensionMenuItem().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CResData::AddContextMenuItems(
    IWCContextMenuCallback * piCallbackIn
    )
{
    HRESULT hr = S_OK;
    CLUSTER_RESOURCE_STATE  crs = ClusterResourceStateUnknown;
    ULONG                   uFlags = MF_GRAYED;

    crs = GetClusterResourceState( m_hResource, NULL, NULL, NULL, NULL );

    if ( crs == ClusterResourceOnline )
    {
        uFlags = MF_ENABLED;
    } // if:

    hr = AddContextMenuItem(
              piCallbackIn
            , IDS_RESOURCE_MENU_ITEM_CALL_ISALIVE
            , IDS_RESOURCE_MENU_ITEM_CALL_ISALIVE_DESC
            , RESOURCE_MENU_ID_1
            , uFlags
            );

    return hr;

} //*** CResData::AddContextMenuItems

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CResData::InvokeCommand
//
//  Description:
//      Invoke a command we added to a context menu.
//
//  Arguments:
//      nCommandIDIn
//          ID of the menu item to execute.  This is the same ID passed to the
//          IWCContextMenuCallback::AddExtensionMenuItem() method.
//
//  Return Value:
//      S_OK        Command invoked successfully.
//      E_NOTIMPL   Command ID not recognized.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CResData::InvokeCommand(
    ULONG nCommandIDIn
    )
{
    HRESULT hr = S_OK;
    DWORD   sc = ERROR_SUCCESS;
    CString str;

    switch ( nCommandIDIn )
    {
        case RESOURCE_MENU_ID_1:
            sc = ClusterResourceControl(
                      m_hResource
                    , NULL
                    , CLUSCTL_RESOURCE_FILESHARESAMPLE_CALL_ISALIVE
                    , NULL
                    , 0
                    , NULL
                    , 0
                    , NULL
                    );
            if ( sc == ERROR_SUCCESS )
            {
                AfxMessageBox( L"Success", MB_OK );
            } // if: success
            else
            {
                if ( sc == ERROR_RESOURCE_FAILED )
                {
                    str.Format( L"The health check detected a failure!" );
                    AfxMessageBox( str, MB_ICONEXCLAMATION );
                } // if:
                else if ( sc == ERROR_CLUSTER_INVALID_REQUEST )
                {
                    str.Format( L"Invalid request - the resource is not online." );
                    AfxMessageBox( str, MB_ICONEXCLAMATION );
                } // else if: ERROR_CLUSTER_INVALID_REQUEST
                else
                {
                    str.Format( L"Unknown return value: %d", sc );
                    AfxMessageBox( str, MB_ICONEXCLAMATION );
                }
            } // else: failure

            hr = S_OK;
            break;

        case ALL_RESOURCES_MENU_ID_1:
            hr = S_OK;
            break;

        default:
            hr = E_NOTIMPL;
            break;
    } // switch: nCommandIDIn

    return hr;

} //*** CResData::InvokeCommand

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CResData::GetPropertySheetPages
//
//  Description:
//
//  Arguments:
//
//  Return Value:
//      S_OK        Success.
//      E_NOTIMPL   No property sheet pages for this resource.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CResData::GetPropertySheetPages(
    CRuntimeClass *** ppaPropSheetsOut
    )
{
    HRESULT hr = E_NOTIMPL;

    ASSERT( ppaPropSheetsOut != NULL );

    //
    // This class only extends File Share Sample resources.
    //

    if ( m_fSupportedType == FALSE )
    {
        goto Cleanup;
    } // if:

    if ( m_rgprtcPSPages != NULL )
    {
        hr = S_OK;
        *ppaPropSheetsOut = m_rgprtcPSPages;
    } // if:

Cleanup:

    return hr;

} //*** CResData::GetPropertySheetPages

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CResData::GetWizardPages
//
//  Description:
//
//  Arguments:
//
//  Return Value:
//      S_OK        Success.
//      E_NOTIMPL   No property sheet pages for this resource.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CResData::GetWizardPages(
    CRuntimeClass *** ppaWizardPagesOut
    )
{
    HRESULT hr = E_NOTIMPL;

    ASSERT( ppaWizardPagesOut != NULL );

    //
    // This class only extends File Share Sample resources.
    //

    if ( m_fSupportedType == FALSE )
    {
        goto Cleanup;
    } // if:

    if ( m_rgprtcWizPages != NULL )
    {
        hr = S_OK;
        *ppaWizardPagesOut = m_rgprtcWizPages;
    } // if:

Cleanup:

    return hr;

} //*** CResData::GetWizardPages

/////////////////////////////////////////////////////////////////////////////
// CResTypeData implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CResTypeData::HrInitialize
//
//  Description:
//      Initializes the object.
//
//  Arguments:
//
//  Return Value:
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CResTypeData::HrInitialize(
    IUnknown * piDataIn
    )
{
    HRESULT hr = S_OK;
    LPCWSTR pwszResTypes = g_wszResourceTypeNames;

    ASSERT( piDataIn != NULL );

    //
    // Initialize the base class.
    //

    hr = CObjData::HrInitialize( piDataIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error initializing the base class

    //
    // Set the object type.
    //

    m_cot = CLUADMEX_OT_RESOURCETYPE;

    m_fSupportedType = FALSE;
    while ( *pwszResTypes != L'\0' )
    {
        if ( StrName().CompareNoCase( pwszResTypes ) == 0 )
        {
            m_fSupportedType = TRUE;
            break;
        } // if:

        pwszResTypes += lstrlenW( pwszResTypes ) + 1;
    }  // while:  more resource types

Cleanup:

    return hr;

} //*** CResTypeData::HrInitialize

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CResTypeData::AddContextMenuItems
//
//  Description:
//      Add menu items to the resource type object context menu.
//
//  Arguments:
//      piCallbackIn
//          Pointer to an IWCContextMenuCallback interface for adding menu
//          items to the context menu.
//
//  Return Value:
//      S_OK
//          Menu items added successfully.
//
//      hr
//          Any errors returned by IWCContextMenuCallback
//          ::AddExtensionMenuItem().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CResTypeData::AddContextMenuItems(
    IWCContextMenuCallback * piCallbackIn
    )
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER( piCallbackIn );

    return hr;

} //*** CResTypeData::AddContextMenuItems

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CResTypeData::InvokeCommand
//
//  Description:
//      Invoke a command we added to a context menu.
//
//  Arguments:
//      nCommandIDIn
//          ID of the menu item to execute.  This is the same ID passed to the
//          IWCContextMenuCallback::AddExtensionMenuItem() method.
//
//  Return Value:
//      S_OK        Command invoked successfully.
//      E_NOTIMPL   Command ID not recognized.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CResTypeData::InvokeCommand(
    ULONG nCommandIDIn
    )
{
    HRESULT hr = S_OK;

    switch ( nCommandIDIn )
    {
        case RESTYPE_MENU_ID_1:

            //
            //  TODO:  Implement your menu item here
            //

            hr = S_OK;
            break;

        case ALL_RESTYPES_MENU_ID_1:
            hr = S_OK;
            break;

        default:
            hr = E_NOTIMPL;
            break;
    } // switch: nCommandIDIn

    return hr;

} //*** CResTypeData::InvokeCommand

/////////////////////////////////////////////////////////////////////////////
// CNetworkData implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CNetworkData::HrInitialize
//
//  Description:
//      Initializes the object.
//
//  Arguments:
//
//  Return Value:
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CNetworkData::HrInitialize(
    IUnknown * piDataIn
    )
{
    HRESULT                     hr = S_OK;
    IGetClusterNetworkInfo *    piNetInfo = NULL;

    ASSERT( piDataIn != NULL );

    //
    // Initialize the base class.
    //

    hr = CObjData::HrInitialize( piDataIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error initializing the base class

    //
    // Initialize member variables to default values.
    //

    m_hNetwork = NULL;

    //
    // Set the object type.
    //

    m_cot = CLUADMEX_OT_NETWORK;

    //
    // Get an interface pointer to IGetClusterNetworkInfo.
    //

    hr = piDataIn->QueryInterface( IID_IGetClusterNetworkInfo, reinterpret_cast< LPVOID * >( &piNetInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error querying for interface

    //
    // Get a copy of the handle.  Don't free this handle - it's just a copy.
    //

    m_hNetwork = piNetInfo->GetNetworkHandle( 0 );
    ASSERT( m_hNetwork != NULL );
    if ( m_hNetwork == NULL )
    {
        hr = E_HANDLE;
        goto Cleanup;
    } // if: error getting the network handle

Cleanup:

    if ( piNetInfo != NULL )
    {
        piNetInfo->Release();
    } // if:

    return hr;

} //*** CNetworkData::HrInitialize

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CNetworkData::AddContextMenuItems
//
//  Description:
//      Add menu items to the network object context menu.
//
//  Arguments:
//      piCallbackIn
//          Pointer to an IWCContextMenuCallback interface for adding menu
//          items to the context menu.
//
//  Return Value:
//      S_OK
//          Menu items added successfully.
//
//      hr
//          Any errors returned by IWCContextMenuCallback
//          ::AddExtensionMenuItem().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT CNetworkData::AddContextMenuItems(
      IWCContextMenuCallback * piCallbackIn
    )
{
    HRESULT     hr = S_OK;

    ASSERT( piCallbackIn != NULL );

    hr = AddContextMenuItem(
              piCallbackIn
            , IDS_NETWORK_MENU_ITEM_1
            , IDS_NETWORK_MENU_ITEM_1_DESC
            , NETWORK_MENU_ID_1
            , MF_ENABLED
            );

    return hr;

} //*** CNetworkData::AddContextMenuItems

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CNetworkData::InvokeCommand
//
//  Description:
//      Invoke a command we added to a context menu.
//
//  Arguments:
//      nCommandIDIn
//          ID of the menu item to execute.  This is the same ID passed to the
//          IWCContextMenuCallback::AddExtensionMenuItem() method.
//
//  Return Value:
//      S_OK        Command invoked successfully.
//      E_NOTIMPL   Command ID not recognized.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CNetworkData::InvokeCommand(
    ULONG nCommandIDIn
    )
{
    HRESULT hr = S_OK;

    switch ( nCommandIDIn )
    {
        case NETWORK_MENU_ID_1:

            //
            //  TODO:  Implement your menu item here
            //

            hr = S_OK;
            break;

        default:
            hr = E_NOTIMPL;
            break;
    } // switch: nCommandIDIn

    return hr;

} //*** CNetworkData::InvokeCommand

/////////////////////////////////////////////////////////////////////////////
// CNetInterfaceData implementation
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CNetInterfaceData::HrInitialize
//
//  Description:
//      Initializes the object.
//
//  Arguments:
//
//  Return Value:
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CNetInterfaceData::HrInitialize(
    IUnknown * piDataIn
    )
{
    HRESULT                         hr = S_OK;
    IGetClusterNetInterfaceInfo *   piNetInterfaceInfo = NULL;

    ASSERT( piDataIn != NULL );

    //
    // Initialize the base class.
    //

    hr = CObjData::HrInitialize( piDataIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error initializing the base class

    //
    // Initialize member variables to default values.
    //

    m_hNetInterface = NULL;

    //
    // Set the object type.
    //

    m_cot = CLUADMEX_OT_NETINTERFACE;

    //
    // Get an interface pointer to IGetClusterNetInterfaceInfo.
    //

    hr = piDataIn->QueryInterface( IID_IGetClusterNetInterfaceInfo, reinterpret_cast< LPVOID * >( &piNetInterfaceInfo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    } // if: error querying for interface

    //
    // Get a copy of the handle.  Don't free this handle - it's just a copy.
    //

    m_hNetInterface = piNetInterfaceInfo->GetNetInterfaceHandle( 0 );
    ASSERT( m_hNetInterface != NULL );
    if ( m_hNetInterface == NULL )
    {
        hr = E_HANDLE;
        goto Cleanup;
    } // if: error getting the netinterface handle

Cleanup:

    if ( piNetInterfaceInfo != NULL )
    {
        piNetInterfaceInfo->Release();
    } // if:

    return hr;

} //*** CNetInterfaceData::HrInitialize

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CNetInterfaceData::AddContextMenuItems
//
//  Description:
//      Add menu items to the network interface object context menu.
//
//  Arguments:
//      piCallbackIn
//          Pointer to an IWCContextMenuCallback interface for adding menu
//          items to the context menu.
//
//  Return Value:
//      S_OK
//          Menu items added successfully.
//
//      hr
//          Any errors returned by IWCContextMenuCallback
//          ::AddExtensionMenuItem().
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CNetInterfaceData::AddContextMenuItems(
    IWCContextMenuCallback * piCallbackIn
    )
{
    HRESULT hr = S_OK;

    ASSERT( piCallbackIn != NULL );

    hr = AddContextMenuItem(
              piCallbackIn
            , IDS_NETINTERFACE_MENU_ITEM_1
            , IDS_NETINTERFACE_MENU_ITEM_1_DESC
            , NETINTERFACE_MENU_ID_1
            , MF_ENABLED
            );

    return hr;

} //*** CNetInterfaceData::AddContextMenuItems

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CNetInterfaceData::InvokeCommand
//
//  Description:
//      Invoke a command we added to a context menu.
//
//  Arguments:
//      nCommandIDIn
//          ID of the menu item to execute.  This is the same ID passed to the
//          IWCContextMenuCallback::AddExtensionMenuItem() method.
//
//  Return Value:
//      S_OK        Command invoked successfully.
//      E_NOTIMPL   Command ID not recognized.
//
//--
/////////////////////////////////////////////////////////////////////////////
HRESULT
CNetInterfaceData::InvokeCommand(
    ULONG nCommandIDIn
    )
{
    HRESULT hr = S_OK;

    switch ( nCommandIDIn )
    {
        case NETINTERFACE_MENU_ID_1:

            //
            //  TODO:  Implement your menu item here
            //

            hr = S_OK;
            break;

        default:
            hr = E_NOTIMPL;
            break;
    } // switch: nCommandIDIn

    return hr;

} //*** CNetInterfaceData::InvokeCommand

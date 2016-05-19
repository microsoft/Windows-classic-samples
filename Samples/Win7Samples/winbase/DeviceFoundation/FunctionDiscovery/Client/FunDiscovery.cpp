// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "FunDiscovery.h"
#include <objbase.h>
#include <strsafe.h>

// Property Keys are GUIDs which don't mean much to a human.
// In order to display the Property Key's description instead of the
// GUID, we have defined a lookup table of key/description pairs.
//
// THIS IS ONLY FOR DISPLAY PURPOSES
//

#define KEY_ENTRY(x) { &PKEY_##x, L#x }

struct KEYENTRY
{
    const PROPERTYKEY * key;
    const WCHAR * pszDescription;
};

const KEYENTRY g_Keys[] = {
    KEY_ENTRY(ComputerName),
    KEY_ENTRY(FD_Visibility),
    KEY_ENTRY(Device_InstanceId),
    KEY_ENTRY(Device_Interface),
    KEY_ENTRY(Device_HardwareIds),
    KEY_ENTRY(Device_CompatibleIds),
    KEY_ENTRY(Device_Service),
    KEY_ENTRY(Device_Class),
    KEY_ENTRY(Device_ClassGuid),
    KEY_ENTRY(Device_Driver),
    KEY_ENTRY(Device_ConfigFlags),
    KEY_ENTRY(Device_Manufacturer),
    KEY_ENTRY(Device_FriendlyName),
    KEY_ENTRY(Device_LocationInfo),
    KEY_ENTRY(Device_PDOName),
    KEY_ENTRY(Device_Capabilities),
    KEY_ENTRY(Device_UINumber),
    KEY_ENTRY(Device_UpperFilters),
    KEY_ENTRY(Device_LowerFilters),
    KEY_ENTRY(Device_BusTypeGuid),
    KEY_ENTRY(Device_LegacyBusType),
    KEY_ENTRY(Device_BusNumber),
    KEY_ENTRY(Device_EnumeratorName),
    KEY_ENTRY(Device_QueueSize),
    KEY_ENTRY(Device_Status),
    KEY_ENTRY(Device_Comment),
    KEY_ENTRY(Device_Model),
    KEY_ENTRY(Device_DeviceDesc),
    KEY_ENTRY(Device_BIOSVersion),
    KEY_ENTRY(PNPX_DomainName),
    KEY_ENTRY(PNPX_ShareName),
    KEY_ENTRY(PNPX_GlobalIdentity),
    KEY_ENTRY(PNPX_Types),
    KEY_ENTRY(PNPX_Scopes),
    KEY_ENTRY(PNPX_XAddrs),
    KEY_ENTRY(PNPX_MetadataVersion),
    KEY_ENTRY(PNPX_Manufacturer),
    KEY_ENTRY(PNPX_ManufacturerUrl),
    KEY_ENTRY(PNPX_ModelName),
    KEY_ENTRY(PNPX_ModelNumber),
    KEY_ENTRY(PNPX_ModelUrl),
    KEY_ENTRY(PNPX_Upc),
    KEY_ENTRY(PNPX_PresentationUrl),
    KEY_ENTRY(PNPX_FriendlyName),
    KEY_ENTRY(PNPX_FirmwareVersion),
    KEY_ENTRY(PNPX_SerialNumber),
    KEY_ENTRY(PNPX_DeviceCategory),
    KEY_ENTRY(PNPX_PhysicalAddress),
    KEY_ENTRY(PNPX_NetworkInterfaceGuid),
    KEY_ENTRY(PNPX_NetworkInterfaceLuid),
    KEY_ENTRY(PNPX_Installable),
    KEY_ENTRY(PNPX_Associated),
    KEY_ENTRY(PNPX_IpAddress),
    KEY_ENTRY(PNPX_CompatibleTypes),
    KEY_ENTRY(PNPX_ServiceId),
    KEY_ENTRY(PNPX_ServiceTypes),
    KEY_ENTRY(PNPX_ServiceAddress),
    KEY_ENTRY(WNET_Scope),
    KEY_ENTRY(WNET_Type),
    KEY_ENTRY(WNET_DisplayType),
    KEY_ENTRY(WNET_Usage),
    KEY_ENTRY(WNET_LocalName),
    KEY_ENTRY(WNET_RemoteName),
    KEY_ENTRY(WNET_Comment),
    KEY_ENTRY(WNET_Provider),
    KEY_ENTRY(DriverPackage_VendorWebSite)
};

#define ARRAY_SIZE( x ) ( sizeof( x ) / sizeof( x[0] ))

// Constructor
CMyFDHelper::CMyFDHelper( ):
    m_lRefCount( 1 ),
    m_hAdd( NULL ),
    m_hRemove( NULL ),
    m_hChange( NULL )
{
}

// Destructor
CMyFDHelper::~CMyFDHelper( )
{
    if( NULL != m_hAdd )
        ::CloseHandle( m_hAdd );
    if( NULL != m_hRemove )
        ::CloseHandle( m_hRemove );
    if( NULL != m_hChange )
        ::CloseHandle( m_hChange );

    if( NULL != m_pFunDisc )
    {
        m_pFunDisc->Release( );
        m_pFunDisc = NULL;
    }

}


HRESULT CMyFDHelper::Initialize( )
{
    HRESULT hr = S_OK;

    // Create events for signaling notifications
    if( S_OK == hr )
    {
        m_hAdd = ::CreateEvent( NULL, FALSE, FALSE, NULL );
        if( NULL == m_hAdd )
            hr = E_OUTOFMEMORY;
    }

    if( S_OK == hr )
    {
        m_hRemove = ::CreateEvent( NULL, FALSE, FALSE, NULL );
        if( NULL == m_hRemove )
            hr = E_OUTOFMEMORY;
    }
    if( S_OK == hr )
    {
        m_hChange = ::CreateEvent( NULL, FALSE, FALSE, NULL );
        if( NULL == m_hChange )
            hr = E_OUTOFMEMORY;
    }

    // CoCreate function discovery
    if ( S_OK == hr )
        hr = CoCreateInstance( CLSID_FunctionDiscovery,
        NULL,
        CLSCTX_ALL,
        IID_IFunctionDiscovery,
        (LPVOID*) &m_pFunDisc );

    return hr;
}

// Outputs a list of all instances in the Category
HRESULT CMyFDHelper::ListFunctionInstances( const WCHAR* pszCategory )
{
    HRESULT hr = S_OK;

    IFunctionInstanceCollectionQuery * pQuery = NULL;
    IFunctionInstanceCollection * pCollection = NULL;
    IFunctionInstance * pInstance = NULL;
    IPropertyStore * pStore = NULL;

    DWORD dwCount = 0;

    //
    // Create an instance collection query.  This CMyFDHelper class
    // implements CFunctionDiscoveryNotificationWrapper and
    // therefore IFunctionDiscoveryNotification.  One of the parameters
    // to CreateInstanceCollectionQuery is a IFunctionDiscoveryNotification
    // pointer.  This object is sent query events.
    //
    if( S_OK == hr )
        hr = m_pFunDisc->CreateInstanceCollectionQuery( pszCategory,
        NULL,
        TRUE,
        this,
        NULL,
        &pQuery);

    // Execute the query.  If it's a local query, for example PnP
    // or the registry, hr will be set to S_OK and the collection
    // will be populated with instances.  If it's a network query
    // hr will be set to E_PENDING and the collection will be
    // empty.  All instances from a network query are returned
    // via notifications.

    if( S_OK == hr )
        hr = pQuery->Execute( &pCollection );

	// If this is a local query, we expect S_OK
    if( S_OK == hr )
        hr = pCollection->GetCount( &dwCount );

    // Loop through all instances returned in the collection
	// This is done only in the case of a local query

    for( DWORD i = 0; ( S_OK == hr && ( i < dwCount )); i++ )
    {
        if( S_OK == hr )
            hr = pCollection->Item( i, &pInstance );

        if( S_OK == hr )
            hr = pInstance->OpenPropertyStore( STGM_READ, &pStore );

        if( S_OK == hr )
            hr = DisplayProperties( pStore );
    }

    if( S_OK == hr )
        wprintf(L"Found %d instances on your system.\n\n", dwCount );

    if( NULL != pQuery )
        pQuery->Release( );
    if( NULL != pCollection )
        pCollection->Release( );
    if( NULL != pInstance )
        pInstance->Release( );
    if( NULL != pStore )
        pStore->Release( );

	// If it's a network query, we expected E_PENDING from the above
	// call to pQuery->Execute( &pCollection )
	// Instances will be returned via notifications, return S_OK
    if( E_PENDING == hr )
        hr = S_OK;

    return hr;
}

// Waits for the specified amount of time for an add, remove
// or change notification
HRESULT CMyFDHelper::WaitForChange( DWORD dwTimeout,
    const WCHAR* pszCategory,
    QueryUpdateAction eAction )
{

    DWORD dwEventIndex;
    HRESULT hr = S_OK;

    IFunctionInstanceCollectionQuery * pQuery = NULL;
    IFunctionInstanceCollection * pCollection = NULL;

    HANDLE hEvent = NULL;

    // Reset each event incase a notification was received by FD before
    // this sample was interested.

    if( m_hAdd )
        ::ResetEvent( m_hAdd );
    if( m_hRemove)
        ::ResetEvent( m_hRemove );
    if( m_hChange )
        ::ResetEvent( m_hChange );

    // Create a query to recieve notifications
    if( S_OK == hr )
        hr = m_pFunDisc->CreateInstanceCollectionQuery(
        pszCategory,
        NULL,
        TRUE,
        this,
        NULL,
        &pQuery);

    // Add a query constraint

    // If we're querying the PnP provider category this constraint
    // will tell the PnP provider not to populate the collection with current
    // PnP devices.  We will only get notifications as devices are added
    // and removed

    // If this method is called with a different category, the provider
    // will ignore the constraint.  It is PnP provider specific.
    if( S_OK == hr )
        hr = pQuery->AddQueryConstraint(
        PROVIDERPNP_QUERYCONSTRAINT_NOTIFICATIONSONLY,
        L"TRUE" );

    // Execute the query.  As long as the query exists
    // we will recieve notifications.
    if( S_OK == hr )
        hr = pQuery->Execute( &pCollection );

    // If it's a network query, we expect the E_PENDING
    if( E_PENDING == hr )
        hr = S_OK;

    if( QUA_ADD == eAction )
        hEvent = m_hAdd;
    else if( QUA_REMOVE == eAction )
        hEvent = m_hRemove;
    else if( QUA_CHANGE == eAction )
        hEvent = m_hChange;
    else
        hEvent = NULL;

    // Block and wait for a notification
    if(( S_OK == hr ) && ( NULL != hEvent ))
        hr = ::CoWaitForMultipleHandles( 0,
        dwTimeout,
        1,
        &hEvent,
        &dwEventIndex );

    if( RPC_S_CALLPENDING == hr )
        wprintf(L"Timeout!\n");

    // One device may correspond to multiple function instances
    // This sleep allows the OnUpdate call to output information
    // about each Function Instance.

    // THIS SLEEP IS MERELY FOR DISPLAY PURPOSES
    Sleep( 1000 );

    if( NULL != pCollection )
        pCollection->Release( );
    if( NULL != pQuery )
        pQuery->Release( );

    return hr;
}

// This method is the main notification sink
// In this implementation it displays the name of the
// added removed or modify device
HRESULT CMyFDHelper::OnUpdate( QueryUpdateAction eAction,
    FDQUERYCONTEXT fdqcQueryContext,
    IFunctionInstance *pInstance)
{
	UNREFERENCED_PARAMETER(fdqcQueryContext);

    if( NULL == pInstance )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    IPropertyStore * pStore;

    PROPVARIANT pvName;
    ::PropVariantInit( &pvName );

    // Open the property store
    hr = pInstance->OpenPropertyStore( STGM_READ, &pStore );

    // In PnP the device's friendly name could be in one of the
    // following property keys.  Check each.
    if( S_OK == hr )
        hr = pStore->GetValue(PKEY_Device_DeviceDesc, &pvName);
    if ( !pvName.pwszVal )
        hr = pStore->GetValue(PKEY_Device_FriendlyName, &pvName);

    // If there is no friendly name, get the provider instance
    // ID instead.  This ID is unique to the provider and is
    // used to identify the instance.
    if ( S_OK == hr && ( !pvName.pwszVal ))
    {
        pvName.vt = VT_LPWSTR;
        hr = pInstance->GetProviderInstanceID(&(pvName.pwszVal));
    }

    if(( S_OK == hr ) && ( pvName.vt == VT_LPWSTR ))
    {
        if( QUA_ADD == eAction )
        {
            wprintf( L"Added: %s\n\n", pvName.pwszVal );
            ::SetEvent( m_hAdd );
        }
        else if( QUA_REMOVE == eAction )
        {
            wprintf( L"Removed: %s\n\n", pvName.pwszVal );
            ::SetEvent( m_hRemove );
        }
        else if( QUA_CHANGE == eAction )
        {
            wprintf( L"Changed: %s\n\n", pvName.pwszVal );
            ::SetEvent( m_hChange );
        }
    }

    ::PropVariantClear( &pvName );
    pStore->Release( );

    return hr;
}

// Required for the implementation of IFunctionDiscoveryNotification
HRESULT CMyFDHelper::OnError(HRESULT hr,
    FDQUERYCONTEXT fdqcQueryContext,
    const WCHAR *pszProvider)
{
	UNREFERENCED_PARAMETER(fdqcQueryContext);

    if( NULL != pszProvider )
        wprintf( L"%s encountered 0x%x.", pszProvider, hr );

    return S_OK;
}

// Required for the implementation of IFunctionDiscoveryNotification
HRESULT CMyFDHelper::OnEvent(
    DWORD dwEventID,
    FDQUERYCONTEXT fdqcQueryContext,
    const WCHAR *pszProvider)
{
	UNREFERENCED_PARAMETER(dwEventID);
	UNREFERENCED_PARAMETER(fdqcQueryContext);

    if( NULL != pszProvider )
        wprintf( L"%s sent OnEvent notification.", pszProvider );

    return S_OK;
}

// Outputs all properties in a property store
HRESULT CMyFDHelper::DisplayProperties( IPropertyStore * pPStore )
{
    if ( NULL == pPStore )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    DWORD cProps = 0;

    //
    // Get the number of properties in the store and loop
    // through them all
    //
    hr = pPStore->GetCount(&cProps);
    for (DWORD p = 0; ( S_OK == hr && p < cProps ); p++)
    {
        PROPERTYKEY key;
        PROPVARIANT val;

        hr = pPStore->GetAt(p, &key);
        if( S_OK == hr )
        {
            hr = pPStore->GetValue(key, &val);
        }
        if ( S_OK == hr )
        {
            // Loop through that table defined at the beginning of this file
            // to find the key name

            WCHAR szKeyNameBuff[MAX_PATH];
            const WCHAR * pszKeyName = NULL;
            for (DWORD z = 0; z < ARRAY_SIZE(g_Keys) && NULL == pszKeyName; z++)
            {
                if (IsEqualPropertyKey(*g_Keys[z].key, key))
                {
                    pszKeyName = g_Keys[z].pszDescription;
                }
            }
            if (NULL == pszKeyName)
            {
                WCHAR szGuidKey[MAX_PATH];
                ::StringFromGUID2(key.fmtid, szGuidKey, ARRAY_SIZE(szGuidKey));
                StringCchPrintfW(szKeyNameBuff,
                    ARRAY_SIZE(szKeyNameBuff),
                    L"%s-%08x",
                    szGuidKey,
                    key.pid);

                pszKeyName = szKeyNameBuff;
            }
            switch(val.vt)
            {
            case VT_EMPTY:
                wprintf(L"%30s = (empty)\n", pszKeyName);
                break;
            case VT_LPWSTR:
                wprintf(L"%30s = %s\n", pszKeyName, val.pwszVal);
                break;
            case VT_UI4:
                wprintf(L"%30s = %08x\n", pszKeyName, val.ulVal);
                break;
            case VT_I4:
                wprintf(L"%30s = %08x\n", pszKeyName, val.ulVal);
                break;
            case VT_INT:
                wprintf(L"%30s = %08x\n", pszKeyName, val.intVal);
                break;
            case VT_UINT:
                wprintf(L"%30s = %08x\n", pszKeyName, val.uintVal);
                break;
            case VT_BOOL:
                wprintf(L"%30s = %s\n",
                    pszKeyName,
                    val.boolVal?L"TRUE":L"FALSE" );
                break;
            case VT_LPWSTR | VT_VECTOR:
                for (ULONG i = 0; i < val.calpwstr.cElems; i++)
                {
                    wprintf(L"%30s = %s\n",
                        pszKeyName,
                        val.calpwstr.pElems[i]);
                    pszKeyName = L"";
                }
                break;
            case VT_CLSID:
                {
                    WCHAR szGuid[MAX_PATH];
                    ::StringFromGUID2(*val.puuid, szGuid, ARRAY_SIZE(szGuid));
                    wprintf(L"%30s = %s\n", pszKeyName, szGuid);
                }
                break;
            default:
                wprintf(L"%30s = Variant Type %08x Unknown\n",
                    pszKeyName,
                    val.vt);
                break;
            }
            ::PropVariantClear(&val);
        }
    }
    return hr;
}



/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      RegExt.cpp
//
//  Description:
//      Implementation of routines for extension registration.
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
#include <ole2.h>
#include <StrSafe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const LPCWSTR REG_VALUE_ADMIN_EXTENSIONS = L"AdminExtensions";

/////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
/////////////////////////////////////////////////////////////////////////////

static DWORD
RegisterAnyCluAdminExtension(
      HCLUSTER      hClusterIn
    , LPCWSTR       pwszKeyNameIn
    , const CLSID * pClsidIn
    );

static DWORD
RegisterAnyCluAdminExtension(
      HKEY          hKeyIn
    , const CLSID * pClsidIn
    );

static DWORD
UnregisterAnyCluAdminExtension(
      HCLUSTER      hClusterIn
    , LPCWSTR       pwszKeyNameIn
    , const CLSID * pClsidIn
    );

static DWORD
UnregisterAnyCluAdminExtension(
      HKEY          hKeyIn
    , const CLSID * pClsidIn
    );

static DWORD
ReadValue(
      HKEY      hKeyIn
    , LPCWSTR   pwszValueNameIn
    , LPWSTR *  ppwszValueOut
    , size_t *  pcchValueOut
    );

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterCluAdminClusterExtension
//
//  Routine Description:
//      Register with the cluster database a Cluster Administrator Extension
//      DLL that extends the cluster object.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
RegisterCluAdminClusterExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;    
    HKEY    hKey = NULL;

    //
    //  Get the cluster registry key.
    //

    hKey = GetClusterKey( hClusterIn, KEY_ALL_ACCESS );
    if ( hKey == NULL )
    {
        sc = GetLastError();
    } // if: error getting cluster key
    else
    {
        //
        //  Register the extension.
        //

        sc = RegisterAnyCluAdminExtension( hKey, pClsidIn );

        ClusterRegCloseKey( hKey );
    }  // else:  GetClusterKey succeeded

    return sc;

}  //*** RegisterCluAdminClusterExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterCluAdminAllNodesExtension
//
//  Routine Description:
//      Register with the cluster database a Cluster Administrator Extension
//      DLL that extends all nodes.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
RegisterCluAdminAllNodesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD sc = ERROR_SUCCESS;

    sc = RegisterAnyCluAdminExtension( hClusterIn, L"Nodes", pClsidIn );

    return sc;

}  //*** RegisterCluAdminAllNodesExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterCluAdminAllGroupsExtension
//
//  Routine Description:
//      Register with the cluster database a Cluster Administrator Extension
//      DLL that extends all groups.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
RegisterCluAdminAllGroupsExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD sc = ERROR_SUCCESS;

    sc = RegisterAnyCluAdminExtension( hClusterIn, L"Groups", pClsidIn );

    return sc;

}  //*** RegisterCluAdminAllGroupsExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterCluAdminAllResourcesExtension
//
//  Routine Description:
//      Register with the cluster database a Cluster Administrator Extension
//      DLL that extends all resources.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
RegisterCluAdminAllResourcesExtension(
      HCLUSTER       hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = RegisterAnyCluAdminExtension( hClusterIn, L"Resources", pClsidIn );

    return sc;

}  //*** RegisterCluAdminAllResourcesExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterCluAdminAllResourceTypesExtension
//
//  Routine Description:
//      Register with the cluster database a Cluster Administrator Extension
//      DLL that extends all resource types.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
RegisterCluAdminAllResourceTypesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = RegisterAnyCluAdminExtension( hClusterIn, L"ResourceTypes", pClsidIn );

    return sc;

}  //*** RegisterCluAdminAllResourceTypesExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterCluAdminAllNetworksExtension
//
//  Routine Description:
//      Register with the cluster database a Cluster Administrator Extension
//      DLL that extends all networks.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
RegisterCluAdminAllNetworksExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = RegisterAnyCluAdminExtension( hClusterIn, L"Networks", pClsidIn );

    return sc;

}  //*** RegisterCluAdminAllNetworksExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterCluAdminAllNetInterfacesExtension
//
//  Routine Description:
//      Register with the cluster database a Cluster Administrator Extension
//      DLL that extends all network interfaces.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
RegisterCluAdminAllNetInterfacesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = RegisterAnyCluAdminExtension( hClusterIn, L"NetworkInterfaces", pClsidIn );

    return sc;

}  //*** RegisterCluAdminAllNetInterfacesExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterCluAdminResourceTypeExtension
//
//  Routine Description:
//      Register with the cluster database a Cluster Administrator Extension
//      DLL that extends resources of a specific type, or the resource type
//      itself.
//
//  Arguments:
//      hClusterIn        Handle to the cluster to modify.
//      pwszResourceType  Resource type name.
//      pClsidIn          Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS
//                          Extension registered successfully.
//
//      ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND
//                          The resource type has not been registered with
//                          the cluster.
//
//      Other Win32 error code if a failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
RegisterCluAdminResourceTypeExtension(
      HCLUSTER      hClusterIn
    , LPCWSTR       pwszResourceTypeIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;
    HKEY    hKey = NULL;

    //
    // Get the resource type registry key.
    //

    hKey = GetClusterResourceTypeKey( hClusterIn, pwszResourceTypeIn, KEY_ALL_ACCESS );
    if ( hKey == NULL )
    {
        sc = GetLastError();
        if ( sc == ERROR_FILE_NOT_FOUND )
        {
            //
            //  If we failed to open the key with 'file not found' that means that
            //  the key doesn't exist.  We'll return a 'res type not found' error,
            //  which is more accurate.
            //

            sc = ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND;
        } // if:
    } // if: error getting resource type key
    else
    {
        //
        //  Register the extension.
        //

        sc = RegisterAnyCluAdminExtension( hKey, pClsidIn );

        ClusterRegCloseKey( hKey );
    }  // else:  GetClusterResourceTypeKey succeeded

    return sc;

}  //*** RegisterCluAdminResourceTypeExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterCluAdminClusterExtension
//
//  Routine Description:
//      Unregister with the cluster database a Cluster Administrator Extension
//      DLL that extends the cluster object.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
UnregisterCluAdminClusterExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;
    HKEY    hKey = NULL;

    //
    //  Get the cluster registry key.
    //

    hKey = GetClusterKey(hClusterIn, KEY_ALL_ACCESS);
    if ( hKey == NULL )
    {
        sc = GetLastError();
    } // if: error getting cluster key
    else
    {
        //
        //  Unregister the extension.
        //

        sc = UnregisterAnyCluAdminExtension( hKey, pClsidIn );

        ClusterRegCloseKey( hKey );
    }  // else:  GetClusterKey succeeded

    return sc;

}  //*** UnregisterCluAdminClusterExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterCluAdminAllNodesExtension
//
//  Routine Description:
//      Unregister with the cluster database a Cluster Administrator Extension
//      DLL that extends all nodes.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
UnregisterCluAdminAllNodesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = UnregisterAnyCluAdminExtension( hClusterIn, L"Nodes", pClsidIn );

    return sc;

}  //*** UnregisterCluAdminAllNodesExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterCluAdminAllGroupsExtension
//
//  Routine Description:
//      Unregister with the cluster database a Cluster Administrator Extension
//      DLL that extends all groups.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
UnregisterCluAdminAllGroupsExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = UnregisterAnyCluAdminExtension( hClusterIn, L"Groups", pClsidIn );

    return sc;

}  //*** UnregisterCluAdminAllGroupsExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterCluAdminAllResourcesExtension
//
//  Routine Description:
//      Unregister with the cluster database a Cluster Administrator Extension
//      DLL that extends all resources.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
UnregisterCluAdminAllResourcesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = UnregisterAnyCluAdminExtension( hClusterIn, L"Resources", pClsidIn );

    return sc;

}  //*** UnregisterCluAdminAllResourcesExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterCluAdminAllResourceTypesExtension
//
//  Routine Description:
//      Unregister with the cluster database a Cluster Administrator Extension
//      DLL that extends all resource types.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
UnregisterCluAdminAllResourceTypesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = UnregisterAnyCluAdminExtension( hClusterIn, L"ResourceTypes", pClsidIn );

    return sc;

}  //*** UnregisterCluAdminAllResourceTypesExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterCluAdminAllNetworksExtension
//
//  Routine Description:
//      Unregister with the cluster database a Cluster Administrator Extension
//      DLL that extends all networks.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
UnregisterCluAdminAllNetworksExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = UnregisterAnyCluAdminExtension( hClusterIn, L"Networks", pClsidIn );

    return sc;

}  //*** UnregisterCluAdminAllNetworksExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterCluAdminAllNetInterfacesExtension
//
//  Routine Description:
//      Unregister with the cluster database a Cluster Administrator Extension
//      DLL that extends all network interfaces.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
UnregisterCluAdminAllNetInterfacesExtension(
      HCLUSTER      hClusterIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;

    sc = UnregisterAnyCluAdminExtension( hClusterIn, L"NetworkInterfaces", pClsidIn );

    return sc;

}  //*** UnregisterCluAdminAllNetInterfacesExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterCluAdminResourceTypeExtension
//
//  Routine Description:
//      Unregister with the cluster database a Cluster Administrator Extension
//      DLL that extends resources of a specific type, or the resource type
//      itself.
//
//  Arguments:
//      hClusterIn          Handle to the cluster to modify.
//      pwszResourceTypeIn  Resource type name.
//      pClsidIn            Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
UnregisterCluAdminResourceTypeExtension(
      HCLUSTER      hClusterIn
    , LPCWSTR       pwszResourceTypeIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;
    HKEY    hKey = NULL;

    //
    //  Get the resource type registry key.
    //

    hKey = GetClusterResourceTypeKey( hClusterIn, pwszResourceTypeIn, KEY_ALL_ACCESS );
    if ( hKey == NULL )
    {
        sc = GetLastError();
        if ( sc == ERROR_FILE_NOT_FOUND )
        {
            //
            //  If we failed to open the key with 'file not found' that means that
            //  the key doesn't exist.  We'll return a 'res type not found' error,
            //  which is more accurate.
            //

            sc = ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND;
        } // if:
    } // if: error getting resource type key
    else
    {
        //
        //  Unregister the extension.
        //

        sc = UnregisterAnyCluAdminExtension( hKey, pClsidIn );

        ClusterRegCloseKey( hKey );
    }  // else:  GetClusterResourceTypeKey succeeded

    return sc;

}  //*** UnregisterCluAdminResourceTypeExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterAnyCluAdminExtension
//
//  Routine Description:
//      Register any Cluster Administrator Extension DLL with the cluster
//      database.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pwszKeyNameIn   Key name.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
static DWORD
RegisterAnyCluAdminExtension(
      HCLUSTER      hClusterIn
    , LPCWSTR       pwszKeyNameIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;
    HKEY    hkeyCluster = NULL;
    HKEY    hKey = NULL;

    //
    //  Get the cluster key.
    //

    hkeyCluster = GetClusterKey( hClusterIn, KEY_ALL_ACCESS );
    if ( hkeyCluster == NULL )
    {
        sc = GetLastError();
    } // if: error getting cluster key
    else
    {
        //
        //  Get the specified key.
        //

        sc = ClusterRegOpenKey( hkeyCluster, pwszKeyNameIn, KEY_ALL_ACCESS, &hKey );
        if ( sc == ERROR_SUCCESS )
        {
            //
            //  Register the extension.
            //

            sc = RegisterAnyCluAdminExtension( hKey, pClsidIn );

            ClusterRegCloseKey( hKey );
        }  // if: ClusterRegOpenKey succeeded

        ClusterRegCloseKey( hkeyCluster );
    }  // if:  cluster key retrieved successfully

    return sc;

}  //*** RegisterAnyCluAdminExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  RegisterAnyCluAdminExtension
//
//  Routine Description:
//      Register any Cluster Administrator Extension DLL with the cluster
//      database.
//
//  Arguments:
//      hKeyIn          Cluster database key.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
static DWORD
RegisterAnyCluAdminExtension(
      HKEY          hKeyIn
    , const CLSID * pClsidIn
    )
{
    HRESULT     hr = S_OK;
    LPOLESTR    pwszClsid = NULL;
    size_t      cchClsidLen = 0;
    size_t      cchSize = 0;
    size_t      cchNewSize = 0;
    LPWSTR      pwszValue = NULL;
    LPWSTR      pwszNewValue = NULL;
    BOOL        bAlreadyRegistered = TRUE;
    DWORD       sc = ERROR_SUCCESS;

    //
    //  Convert the CLSID to a string.
    //

    hr = StringFromCLSID( *pClsidIn, &pwszClsid );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    } // if:

    //
    //  Get the length of the CLSID string.
    //

    cchClsidLen = lstrlenW( pwszClsid );

    //
    //  Read the current value.
    //

    sc = ReadValue( hKeyIn, REG_VALUE_ADMIN_EXTENSIONS, &pwszValue, &cchSize );
    if ( (sc == ERROR_FILE_NOT_FOUND) || ( (sc == ERROR_SUCCESS) && (cchSize == 0) ) )
    {
        //
        //  If the value was not found or it was and the length was zero then 
        //  the extension has not been registered.
        //

        bAlreadyRegistered = FALSE;
    } // if:
    else if ( sc == ERROR_SUCCESS )
    {
        //
        //  Check to see if the extension has been registered yet.
        //

        if ( pwszValue == NULL )
        {
        } // if: empty value found
        else
        {
            LPCWSTR pwszValueBuf = pwszValue;
            size_t  cchValueBuf = cchSize;
            size_t  cch = 0;

            while ( *pwszValueBuf != L'\0' )
            {
                if ( NIStringCompareW( pwszClsid, pwszValueBuf, cchClsidLen ) == 0)
                {
                    break;
                } //if: CLSID for this extension already in list

                cch = lstrlenW( pwszValueBuf ) + 1;
                pwszValueBuf += cch;
                cchValueBuf -= cch;
            }  // while:  more strings in the extension list

            bAlreadyRegistered = ( *pwszValueBuf != L'\0' );
        }  // else:  extension value exists
    } // else if:
    else
    {
        goto Cleanup;
    } // else:

    //
    //  Register the extension.
    //

    if ( bAlreadyRegistered == FALSE )
    {
        //
        //  Allocate a new buffer.
        //

        cchNewSize = cchSize + lstrlenW( pwszClsid ) + 1;
        if ( cchSize == 0 ) // Add size of final NULL if first entry.
        {
            cchNewSize++;
        } // if: no previous value

        pwszNewValue = new WCHAR[ cchNewSize ];
        if ( pwszNewValue == NULL )
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        } // if: error allocating memory
        else
        {
            LPCWSTR pwszValueBuf    = pwszValue;
            LPWSTR  pwszNewValueBuf = pwszNewValue;
            size_t  cchNewValueBuf  = cchNewSize;
            size_t  cch = 0;
            DWORD   dwType = 0;

            //
            //  Copy the existing extensions to the new buffer.
            //

            if ( pwszValue != NULL )
            {
                while ( *pwszValueBuf != L'\0' )
                {
                    hr = StringCchCopyW( pwszNewValueBuf, cchNewValueBuf, pwszValueBuf );
                    if ( FAILED( hr ) )
                    {
                        sc = HRESULT_CODE( hr );
                        goto Cleanup;
                    } // if:

                    cch = lstrlenW( pwszValueBuf ) + 1;
                    pwszValueBuf += cch;
                    pwszNewValueBuf += cch;
                    cchNewValueBuf -= cch;
                }  // while:  more strings in the extension list
            }  // if:  previous value buffer existed

            //
            //  Add the new CLSID to the list.
            //

            hr = StringCchCopyW( pwszNewValueBuf, cchNewValueBuf, pwszClsid );
            if ( FAILED( hr ) )
            {
                sc = HRESULT_CODE( hr );
                goto Cleanup;
            } // if:

            //
            //  Add the double NULL termination.
            //

            pwszNewValueBuf += lstrlenW( pwszClsid ) + 1;
            *pwszNewValueBuf = L'\0';

            //
            //  Write the value to the cluster database.
            //

            dwType = REG_MULTI_SZ;
            sc = ClusterRegSetValue(
                              hKeyIn
                            , REG_VALUE_ADMIN_EXTENSIONS
                            , dwType
                            , (LPBYTE) pwszNewValue
                            , static_cast< DWORD >( cchNewSize * sizeof( *pwszNewValue ) )
                            );
            if ( sc != ERROR_SUCCESS )
            {
                goto Cleanup;
            }
        }  // else:  new buffer allocated successfully
    }  // if:  extension not registered yet

Cleanup:

    delete [] pwszNewValue;
    delete [] pwszValue;

    CoTaskMemFree( pwszClsid );

    return sc;

}  //*** RegisterAnyCluAdminExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterAnyCluAdminExtension
//
//  Routine Description:
//      Unregister any Cluster Administrator Extension DLL with the cluster
//      database.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//      pwszKeyNameIn   Key name.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
static DWORD
UnregisterAnyCluAdminExtension(
      HCLUSTER      hClusterIn
    , LPCWSTR       pwszKeyNameIn
    , const CLSID * pClsidIn
    )
{
    DWORD   sc = ERROR_SUCCESS;
    HKEY    hKeyCluster = NULL;
    HKEY    hKey = NULL;

    //
    //  Get the cluster key.
    //

    hKeyCluster = GetClusterKey( hClusterIn, KEY_ALL_ACCESS );
    if ( hKeyCluster == NULL )
    {
        sc = GetLastError();
    } // if: error getting cluster key
    else
    {
        // Get the specified key.
        sc = ClusterRegOpenKey( hKeyCluster, pwszKeyNameIn, KEY_ALL_ACCESS, &hKey );
        if (sc == ERROR_SUCCESS)
        {
            // Unregister the extension.
            sc = UnregisterAnyCluAdminExtension( hKey, pClsidIn );

            ClusterRegCloseKey( hKey );
        }  // else:  GetClusterResourceTypeKey succeeded

        ClusterRegCloseKey( hKeyCluster );
    }  // if: cluster key retrieved successfully

    return sc;

}  //*** UnregisterAnyCluAdminExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  UnregisterAnyCluAdminExtension
//
//  Routine Description:
//      Unregister any Cluster Administrator Extension DLL with the cluster
//      database.
//
//  Arguments:
//      hKeyIn          Cluster database key.
//      pClsidIn        Extension's CLSID.
//
//  Return Value:
//      ERROR_SUCCESS   Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
static DWORD
UnregisterAnyCluAdminExtension(
      HKEY          hKeyIn
    , const CLSID * pClsidIn
    )
{
    HRESULT     hr = S_OK;
    LPOLESTR    pwszClsid = NULL;
    size_t      cchClsidLen = 0;
    size_t      cchSize = 0;
    size_t      cchNewSize = 0;
    LPWSTR      pwszValue = NULL;
    LPWSTR      pwszNewValue = NULL;
    BOOL        bAlreadyUnregistered = FALSE;
    DWORD       sc = ERROR_SUCCESS;

    //
    //  Convert the CLSID to a string.
    //

    hr = StringFromCLSID( *pClsidIn, &pwszClsid );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    } // if:

    //
    //  Get the length of the CLSID string.
    //

    cchClsidLen = lstrlenW( pwszClsid );

    //
    //  Read the current value.
    //

    sc = ReadValue( hKeyIn, REG_VALUE_ADMIN_EXTENSIONS, &pwszValue, &cchSize );
    if ( (sc == ERROR_FILE_NOT_FOUND) || ( (sc == ERROR_SUCCESS) && (cchSize == 0) ) )
    {
        //
        //  If the value was not found or it was and the length was zero then 
        //  the extension has been unregistered.
        //

        bAlreadyUnregistered = TRUE;
    } // if:
    else if ( sc == ERROR_SUCCESS )
    {
        LPCWSTR pwszValueBuf = pwszValue;
        size_t  cch = 0;
        size_t  cchValueBuf = cchSize;

        while ( *pwszValueBuf != L'\0' )
        {
            if ( NIStringCompareW( pwszClsid, pwszValueBuf, cchClsidLen ) == 0 )
            {
                break;
            } // if: CLSID for this extension found in the list

            cch = lstrlenW( pwszValueBuf ) + 1;
            pwszValueBuf += cch;
            cchValueBuf -= cch;
        }  // while:  more strings in the extension list

        bAlreadyUnregistered = (*pwszValueBuf == L'\0');
    } // else if:
    else
    {
        goto Cleanup;
    } // else:

    //
    //  Unregister the extension.
    //

    if ( bAlreadyUnregistered == FALSE )
    {
        //
        //  Allocate a new buffer.
        //

        cchNewSize = cchSize - lstrlenW(pwszClsid) - 1;
        if (cchNewSize == 1) // only the final null left
        {
            cchNewSize = 0;
        } // if:

        pwszNewValue = new WCHAR[ cchNewSize ];
        if ( pwszNewValue == NULL )
        {
            sc = ERROR_OUTOFMEMORY;
            goto Cleanup;
        } // if: error allocating memory
        else
        {
            LPCWSTR pwszValueBuf = pwszValue;
            LPWSTR  pwszNewValueBuf = pwszNewValue;
            size_t  cchValueBuf = cchSize;
            size_t  cchNewValueBuf = cchNewSize;
            size_t  cch = 0;
            DWORD   dwType = 0;

            //
            //  Copy the existing extensions to the new buffer.
            //

            if ( ( cchNewSize > 0 ) && ( pwszValue != NULL ) )
            {
                while ( *pwszValueBuf != L'\0' )
                {
                    if ( NIStringCompareW( pwszClsid, pwszValueBuf, cchClsidLen ) != 0 )
                    {
                        hr = StringCchCopyNW( pwszNewValueBuf, cchNewValueBuf, pwszValueBuf, cchValueBuf );
                        if ( FAILED( hr ) )
                        {
                            sc = HRESULT_CODE( hr );
                            goto Cleanup;
                        } // if:

                        cch = lstrlenW( pwszNewValueBuf ) + 1;
                        pwszNewValueBuf += cch;
                        cchNewValueBuf -= cch;
                    }  // if:  not CLSID being removed

                    cch = lstrlenW( pwszValueBuf ) + 1;
                    pwszValueBuf += cch;
                    cchValueBuf -= cch;
                }  // while:  more strings in the extension list

                *pwszNewValueBuf = L'\0';
            }  // if:  previous value buffer existed

            //
            //  Write the value to the cluster database.
            //

            dwType = REG_MULTI_SZ;
            sc = ClusterRegSetValue(
                              hKeyIn
                            , REG_VALUE_ADMIN_EXTENSIONS
                            , dwType
                            , (LPBYTE) pwszNewValue
                            , static_cast< DWORD >( cchNewSize * sizeof( *pwszNewValue ) )
                            );
            if ( sc != ERROR_SUCCESS )
            {
                goto Cleanup;
            } // if:
        }  // else:  new buffer allocated successfully
    }  // if:  extension not unregistered yet

Cleanup:

    delete [] pwszNewValue;
    delete [] pwszValue;

    CoTaskMemFree( pwszClsid );

    return sc;

}  //*** UnregisterAnyCluAdminExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  ReadValue
//
//  Routine Description:
//      Reads a value from the cluster database.
//
//  Arguments:
//      hKeyIn          Handle for the key to read from.
//      pwszValueNameIn Name of value to read.
//      ppwszValueOut   Address of pointer in which to return data.
//                      The string is allocated using new [] and must
//                      be deallocated by the calling delete [].
//      pcchValueOut    Size in characters of the allocated value buffer.
//
//  Return Value:
//      Any return values from ClusterRegQueryValue or errors from new.
//
//--
/////////////////////////////////////////////////////////////////////////////
static DWORD
ReadValue(
      HKEY      hKeyIn
    , LPCWSTR   pwszValueNameIn
    , LPWSTR *  ppwszValueOut
    , size_t *  pcchValueOut
    )
{
    DWORD   sc = ERROR_SUCCESS;
    DWORD   cbSize = 0;
    DWORD   cchSize = 0;
    DWORD   dwType = 0;
    LPWSTR  pwszValue = NULL;

    //
    //  Initialize the out parameters...
    //

    *ppwszValueOut = NULL;
    *pcchValueOut = 0;

    //
    //  Get the length of the value.
    //

    sc = ClusterRegQueryValue( hKeyIn, pwszValueNameIn, &dwType, NULL, &cbSize );
    if (   ( sc != ERROR_SUCCESS )
        && ( sc != ERROR_MORE_DATA ) )
    {
        goto Cleanup;
    }  // if:  error occurred

    ASSERT( (cbSize % sizeof( WCHAR ) ) == 0 ); // must be even
    cchSize = cbSize / sizeof( WCHAR );

    if ( cbSize > 0 )
    {
        //
        //  Allocate a value string.
        //

        pwszValue = new WCHAR[ cchSize ];
        if ( pwszValue == NULL )
        {
            sc =  ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }  // if:  error allocating memory

        //
        //  Read the the value.
        //

        sc = ClusterRegQueryValue( hKeyIn, pwszValueNameIn, &dwType, (LPBYTE) pwszValue, &cbSize );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        } // if:

         ASSERT( cchSize * sizeof( WCHAR ) == cbSize );

        *ppwszValueOut = pwszValue;
        pwszValue = NULL;                                   // give away ownership...
        *pcchValueOut = static_cast< size_t >( cchSize );
    }  // if:  value is not empty

Cleanup:

    delete [] pwszValue;

    return sc;

}  //*** ReadValue

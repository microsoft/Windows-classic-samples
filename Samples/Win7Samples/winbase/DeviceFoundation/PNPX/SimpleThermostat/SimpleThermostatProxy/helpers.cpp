////////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

// Public Headers
#include <windows.h>

// Sample Headers
#include "common.h"

//
// Globals
//
const WCHAR TYPE_STRING_UPNP[] = L"urn:schemas-upnp-org:device";
const WCHAR TYPE_STRING_WSD[] = L"http://schemas.xmlsoap.org/ws/2006/02/devprof/Device";

//------------------------------------------------------------------------------
// GetDeviceType
//      Returns the device type the FD function instance references
//------------------------------------------------------------------------------
HRESULT GetDeviceType( IPropertyStore* pPropStore, DEVICE_PROTOCOL_TYPE* pDeviceType )
{
    HRESULT     hr  = S_OK;
    ULONG       i   = 0;
    PROPVARIANT pv  = {0};

    if( NULL == pPropStore ||
        NULL == pDeviceType )
    {
        return E_INVALIDARG;
    }

    //
    // Get the types list of the thermostat device
    //
    hr = pPropStore->GetValue( PKEY_PNPX_Types, &pv );
    
    //
    // Search all the types until a recognized one is found. If no recognized
    // one is found, S_FALSE will be returned to the caller.
    //
    if( S_OK == hr &&
        (VT_VECTOR | VT_LPWSTR) == pv.vt )
    {
        for( i = 0; i < pv.calpwstr.cElems; i++ )
        {
            //
            // This function instance references a UPnP device
            //
            if( NULL != wcsstr( pv.calpwstr.pElems[i], TYPE_STRING_UPNP ) )
            {
                *pDeviceType = DEVICE_PROTOCOL_TYPE_UPNP;
                break;
            }
            //
            // This function instance references a WSD device
            //
            else if( NULL != wcsstr( pv.calpwstr.pElems[i], TYPE_STRING_WSD ) )
            {
                *pDeviceType = DEVICE_PROTOCOL_TYPE_WSD;
                break;
            }
        }
        if( i == pv.calpwstr.cElems )
        {
            hr = S_FALSE;
        }
    }
    else if( S_OK == hr )
    {
        hr = E_FAIL;
    }

    PropVariantClear(&pv);

    return hr;
}// GetDeviceType

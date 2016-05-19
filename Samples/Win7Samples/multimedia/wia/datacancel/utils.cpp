//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

//------------------------------------------------------------
//Please read the ReadME.txt which explains the purpose of the
//sample.
//-------------------------------------------------------------

#include "utils.h"

// Helper function to display an error message and an optional HRESULT
void ReportError( LPCTSTR pszMessage, HRESULT hr )
{
    if (S_OK != hr)
    {
        _tprintf( TEXT("\n%s: HRESULT: 0x%08X"), pszMessage, hr );
    }
    else
    {
        _tprintf( TEXT("\n%s"), pszMessage );
    }
}

// This function reads item property which returns BSTR like WIA_DIP_DEV_ID, WIA_DIP_DEV_NAME etc.
HRESULT ReadPropertyBSTR(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid , BSTR* pbstr)
{
    if( (!pWiaPropertyStorage) || (!pbstr))
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid arguments passed to ReadPropertyBSTR()"),hr);
        return hr;
    }
    //initialize out variables
    *pbstr = NULL;

    // Declare PROPSPECs and PROPVARIANTs, and initialize them.
    PROPSPEC PropSpec[1] = {0};
    PROPVARIANT PropVar[1];
    PropVariantInit(PropVar);
    
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;
    
    HRESULT hr = pWiaPropertyStorage->ReadMultiple(1,PropSpec,PropVar);
    if(S_OK == hr)
    {
        if(PropVar[0].vt == VT_BSTR)
        {
            *pbstr = SysAllocString(PropVar[0].bstrVal);
            if(!(*pbstr))
            {
                hr = E_OUTOFMEMORY;
                ReportError(TEXT("Failed to allocate memory for BSTR in function ReadPropertyBSTR()"),hr);
            }
        }
        else
        {
            ReportError(TEXT("Trying to read a property which doesn't return a BSTR"));
        }
    }
    else
    {
        ReportError(TEXT("Error calling pWiaPropertyStorage->ReadMultiple() in ReadPropertyBSTR()"),hr);
    }
    
    //Free PropVar array
    PropVariantClear(PropVar);
    return hr;
}

// This function reads item property which returns LONG 
HRESULT ReadPropertyLong(IWiaItem2* pWiaItem2, PROPID propid , LONG* lVal)
{
    if( !pWiaItem2)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("\nInvalid arguments passed to GetBrightnessContrast()"),hr);
        return hr;
    }
    //initialize out variables
    *lVal = 0;
    
    //Get the IWiaPropertyStorage interface for IWiaItem2 
    IWiaPropertyStorage* pWiaPropertyStorage = NULL;
    HRESULT hr = pWiaItem2->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
    if(SUCCEEDED(hr))
    {
        // Declare PROPSPECs and PROPVARIANTs, and initialize them.
        PROPSPEC PropSpec[1] = {0};
        PROPVARIANT PropVar[1];
        PropVariantInit(PropVar);
        
        PropSpec[0].ulKind = PRSPEC_PROPID;
        PropSpec[0].propid = propid;
        hr = pWiaPropertyStorage->ReadMultiple(1,PropSpec,PropVar);
        if(S_OK == hr)
        {
            if(PropVar[0].vt == VT_I4)
            {
                *lVal = PropVar[0].lVal;
            }
            else
            {
                ReportError(TEXT("Trying to read a property which doesn't return a LONG"));
            }
        }
        else
        {
            ReportError(TEXT("Error calling pWiaPropertyStorage->ReadMultiple() in ReadPropertyLong()"),hr);
        }

        //Free PropVar array
        PropVariantClear(PropVar);

        //Release pWiaPropertyStorage
        pWiaPropertyStorage->Release();
        pWiaPropertyStorage = NULL;
    }
    else
    {
        ReportError(TEXT("pWiaItem2->QueryInterface failed on IID_IWiaPropertyStorage"),hr);
    }
    
    return hr;
}

// This function reads item property which returns GUID like WIA_IPA_FORMAT, WIA_IPA_ITEM_CATEGORY etc.
HRESULT ReadPropertyGuid(IWiaItem2* pWiaItem2, PROPID propid , GUID* pguid_val)
{
    if( !pWiaItem2)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to ReadPropertyGuid()"),hr);
        return hr; 
    }
    //initialize out variables
    *pguid_val = GUID_NULL;
    
    //Get the IWiaPropertyStorage interface for IWiaItem2 
    IWiaPropertyStorage* pWiaPropertyStorage = NULL;
    HRESULT hr = pWiaItem2->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
    if(SUCCEEDED(hr))
    {
        // Declare PROPSPECs and PROPVARIANTs, and initialize them.
        PROPSPEC PropSpec[1] = {0};
        PROPVARIANT PropVar[1];
        PropVariantInit(PropVar);
        
        PropSpec[0].ulKind = PRSPEC_PROPID;
        PropSpec[0].propid = propid;
        hr = pWiaPropertyStorage->ReadMultiple(1,PropSpec,PropVar);
        if(S_OK == hr)
        {
            if(PropVar[0].vt == VT_CLSID)
            {
                memcpy(pguid_val,PropVar[0].puuid,sizeof(GUID));
            }
            else
            {
                ReportError(TEXT("Trying to read a property which doesn't return a Guid"));
            }

        }
        else
        {
            ReportError(TEXT("Error calling pWiaPropertyStorage->ReadMultiple() in ReadPropertyGuid()"),hr);
        }

        //Free PropVar array
        PropVariantClear(PropVar);
        
        //Release pWiaPropertyStorage 
        pWiaPropertyStorage->Release();
        pWiaPropertyStorage = NULL;
    }
    else
    {
        ReportError(TEXT("pWiaItem2->QueryInterface failed on IID_IWiaPropertyStorage"),hr);
    }
    
    return hr;
}

    
// This function writes item property which takes GUID like WIA_IPA_FORMAT, WIA_IPA_ITEM_CATEGORY etc.
HRESULT WritePropertyGuid(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid , GUID guid)
{
    if( !pWiaPropertyStorage)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to WritePropertyGuid()"),hr);
        return hr; 
    }

    // Declare PROPSPECs and PROPVARIANTs, and initialize them.
    PROPSPEC PropSpec[1] = {0};
    PROPVARIANT PropVar[1];
    PropVariantInit(PropVar);
        
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;

    //Fill values in Propvar which are to be written 
    PropVar[0].vt = VT_CLSID;
    PropVar[0].puuid = &guid;
    
    HRESULT hr = pWiaPropertyStorage->WriteMultiple(1,PropSpec,PropVar,WIA_IPA_FIRST);
    
    if(FAILED(hr)){
        ReportError(TEXT("pWiaPropertyStorage->WriteMultiple() failed in WritePropertyGuid()"),hr);
    }
    return hr;
}

// This function writes item property which takes LONG like WIA_IPA_PAGES etc.
HRESULT WritePropertyLong(IWiaPropertyStorage* pWiaPropertyStorage, PROPID propid , LONG lVal)
{
    if( !pWiaPropertyStorage)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to WritePropertyLong()"),hr);
        return hr;
    }

    // Declare PROPSPECs and PROPVARIANTs, and initialize them.
    PROPSPEC PropSpec[1] = {0};
    PROPVARIANT PropVar[1];
    PropVariantInit(PropVar);
        
    //Fill values in Propvar which are to be written 
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;
    PropVar[0].vt = VT_I4;
    PropVar[0].lVal = lVal;

    HRESULT hr = pWiaPropertyStorage->WriteMultiple(1,PropSpec,PropVar,WIA_IPA_FIRST);
    if(FAILED(hr)){
        ReportError(TEXT("pWiaPropertyStorage->WriteMultiple() failed in WritePropertyLong()"),hr);
    }
    return hr;
}


//This function reads such WIA device properties as Device ID, Device name and Device descripion
//Also it returns the device ID.
HRESULT ReadWiaPropsAndGetDeviceID( IWiaPropertyStorage *pWiaPropertyStorage ,BSTR* pbstrDeviceID )
{
    // Validate arguments
    if ((NULL == pWiaPropertyStorage) || (NULL == pbstrDeviceID) )
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to ReadWiaPropsAndGetDeviceID()"),hr);
        return hr; 
    }
    
    //Initialize out variables
    *pbstrDeviceID = NULL;
    
    BSTR  bstrdeviceName = NULL, bstrdeviceDesc = NULL;
    
    //Read device ID
    HRESULT hr = ReadPropertyBSTR(pWiaPropertyStorage,WIA_DIP_DEV_ID,pbstrDeviceID);
    if(FAILED(hr))
    {
        ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_ID in ReadWiaPropsAndGetDeviceID()"),hr);
        return hr;
    }
    
    //Read device name
    hr = ReadPropertyBSTR(pWiaPropertyStorage,WIA_DIP_DEV_NAME,&bstrdeviceName);
    if(FAILED(hr))
    {
        ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_NAME in ReadWiaPropsAndGetDeviceID()"),hr);
        return hr;
    }
    
    //Read device description
    hr = ReadPropertyBSTR(pWiaPropertyStorage,WIA_DIP_DEV_DESC,&bstrdeviceDesc);
    if(FAILED(hr))
    {
        ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_DESC in ReadWiaPropsAndGetDeviceID()"),hr);
        return hr;
    }
    
    _tprintf( TEXT("\n\n\nWIA_DIP_DEV_ID: %ws\n"), *pbstrDeviceID );
    _tprintf( TEXT("WIA_DIP_DEV_NAME: %ws\n"), bstrdeviceName );
    _tprintf( TEXT("WIA_DIP_DEV_DESC: %ws\n"), bstrdeviceDesc );
    
    return S_OK;
}

// A simple function which prints the item name 
HRESULT PrintItemName( IWiaItem2 *pIWiaItem2 )
{
    // Validate arguments
    if (NULL == pIWiaItem2)
    {
        HRESULT hr = E_INVALIDARG;
        _tprintf(TEXT("\nInvalid parameters passed"),hr);
        return hr;
    }
    
    // Get the IWiaPropertyStorage interface
    IWiaPropertyStorage *pWiaPropertyStorage = NULL;
    HRESULT hr = pIWiaItem2->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
    if (SUCCEEDED(hr))
    {
    
        BSTR bstrDevName = NULL;
        ReadPropertyBSTR(pWiaPropertyStorage,WIA_IPA_FULL_ITEM_NAME,&bstrDevName);
        _tprintf( TEXT("\n\nItem Name: %ws\n"), bstrDevName );

        // Release the IWiaPropertyStorage interface
        pWiaPropertyStorage->Release();
        pWiaPropertyStorage = NULL;
    }
    else
    {
        ReportError(TEXT("QueryInterface() failed on IID_IWiaPropertyStorage"),hr);
    }

    // Return the result of reading the properties
    return hr;
}


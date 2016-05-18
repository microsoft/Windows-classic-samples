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

#include "ItemTreeChanges.h"


// This function enumerates items recursively starting from the root item and prints the item name and also various properties 
// associated with the item. This function can be used to see the item tree supported by the scanner and thus it gives 
// a measure of the capabilities supported by the scanner.
HRESULT EnumerateItems( IWiaItem2 *pWiaItem2 )
{
    // Validate arguments
    if (NULL == pWiaItem2)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to EnumerateAndPreviewItems()"),hr);
        return hr;
    }
    
    //Print the item name.
    PrintItemName( pWiaItem2 );
    
    // Get the item type for this item
    LONG lItemType = 0;
    HRESULT hr = pWiaItem2->GetItemType( &lItemType );
    
    
    if (lItemType & WiaItemTypeProgrammableDataSource)
    {
        _tprintf(TEXT("\nProgrammable item"));
    }
    if (lItemType & WiaItemTypeTransfer)
    {
        _tprintf(TEXT("\nTransferrable item"));
    }
    else
    {
        _tprintf(TEXT("\nNon-transferrable item"));
    }
        
    if (lItemType & WiaItemTypeFile)
    {
        _tprintf(TEXT("\nFile item"));
    }
    if (lItemType & WiaItemTypeStorage)
    {
        _tprintf(TEXT("\nStorage item"));
    }   

    // find the item category
    GUID ItemCategory = GUID_NULL;
    ReadPropertyGuid(pWiaItem2,WIA_IPA_ITEM_CATEGORY,&ItemCategory );
    
    if(IsEqualIID(ItemCategory,WIA_CATEGORY_FINISHED_FILE))
    {
        //files uploaded to storage will be finished files
        _tprintf(TEXT("\nFinished file item"));
    }
        

    // If it is a folder, enumerate its children
    if (lItemType & WiaItemTypeFolder)
    {
        _tprintf(TEXT("\nFolder item"));
        // Get the child item enumerator for this item
        IEnumWiaItem2 *pEnumWiaItem2 = NULL;
        hr = pWiaItem2->EnumChildItems( 0,&pEnumWiaItem2 );
        if (SUCCEEDED(hr))
        {
            // We will loop until we get an error or pEnumWiaItem->Next returns
            // S_FALSE to signal the end of the list.
            while (S_OK == hr)
            {
                // Get the next child item
                IWiaItem2 *pChildWiaItem2 = NULL;
                hr = pEnumWiaItem2->Next( 1, &pChildWiaItem2, NULL );

                //
                // pEnumWiaItem->Next will return S_FALSE when the list is
                // exhausted, so check for S_OK before using the returned
                // value.
                //
                if (S_OK == hr)
                {
                    // Recurse into this item
                    EnumerateItems( pChildWiaItem2 );

                    // Release this item
                    pChildWiaItem2->Release();
                    pChildWiaItem2 = NULL;
                }
                else if (FAILED(hr))
                {
                    // Report that an error occurred during enumeration
                    ReportError( TEXT("Error calling pEnumWiaItem2->Next"), hr );
                }
            }

            // If the result of the enumeration is S_FALSE, since this
            // is normal, we will change it to S_OK
            if (S_FALSE == hr)
            {
                hr = S_OK;
            }

            // Release the enumerator
            pEnumWiaItem2->Release();
            pEnumWiaItem2 = NULL;
        }
    }
    return  hr;
}


//This function enumerates WIA devices and then creates an instance of each device.
//After that it calls EnumerateItems() on the root item got from creation of device.
HRESULT EnumerateWiaDevices( IWiaDevMgr2 *pWiaDevMgr2 )
{
    // Validate arguments
    if (NULL == pWiaDevMgr2)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to EnumerateWiaDevices()"),hr);
        return hr;
    }

    // Get a device enumerator interface
    IEnumWIA_DEV_INFO *pWiaEnumDevInfo = NULL;
    HRESULT hr = pWiaDevMgr2->EnumDeviceInfo( WIA_DEVINFO_ENUM_LOCAL, &pWiaEnumDevInfo );
    if (SUCCEEDED(hr))
    {
        // Reset the device enumerator to the beginning of the list
        hr = pWiaEnumDevInfo->Reset();
        if (SUCCEEDED(hr))
        {
            // We will loop until we get an error or pWiaEnumDevInfo->Next returns
            // S_FALSE to signal the end of the list.
            while (S_OK == hr)
            {
                // Get the next device's property storage interface pointer
                IWiaPropertyStorage *pWiaPropertyStorage = NULL;
                hr = pWiaEnumDevInfo->Next( 1, &pWiaPropertyStorage, NULL );

                // pWiaEnumDevInfo->Next will return S_FALSE when the list is
                // exhausted, so check for S_OK before using the returned
                // value.
                if (hr == S_OK)
                {
                    // Read some device properties - Device ID,name and descripion and return Device ID needed for creating Device
                    BSTR bstrDeviceID = NULL;
                    HRESULT hr1 = ReadWiaPropsAndGetDeviceID( pWiaPropertyStorage ,&bstrDeviceID);
                    if(SUCCEEDED(hr1))
                    {
                        // Call a function to create the device using device ID 
                        IWiaItem2 *pWiaRootItem2 = NULL;
                        hr1 = pWiaDevMgr2->CreateDevice( 0, bstrDeviceID, &pWiaRootItem2 );
                        if(SUCCEEDED(hr1))
                        {
                            // Enumerate items 
                            hr1 = EnumerateItems( pWiaRootItem2 );
                            if(FAILED(hr1))
                            {
                                ReportError(TEXT("EnumerateItems() failed in EnumerateWiaDevices()"),hr1);
                            }
                            //Release pWiaRootItem2
                            pWiaRootItem2->Release();
                            pWiaRootItem2 = NULL;
                        }
                        else
                        {
                            ReportError(TEXT("Error calling IWiaDevMgr2::CreateDevice()"),hr1);
                        }
                    }
                    else
                    {
                        ReportError(TEXT("ReadWiaPropsAndGetDeviceID() failed in EnumerateWiaDevices()"),hr1);
                    }
                        
                    // Release the device's IWiaPropertyStorage*
                    pWiaPropertyStorage->Release();
                    pWiaPropertyStorage = NULL;
                }
                else if (FAILED(hr))
                {
                    // Report that an error occurred during enumeration
                    ReportError( TEXT("Error calling IEnumWIA_DEV_INFO::Next()"), hr );
                }
            }
            
            //
            // If the result of the enumeration is S_FALSE, since this
            // is normal, we will change it to S_OK.
            //
            if (S_FALSE == hr)
            {
                hr = S_OK;
            }
        }
        else
        {
            // Report that an error occurred calling Reset()
            ReportError( TEXT("Error calling IEnumWIA_DEV_INFO::Reset()"), hr );
        }
        // Release the enumerator
        pWiaEnumDevInfo->Release();
        pWiaEnumDevInfo = NULL;
    }
    else
    {
        // Report that an error occurred trying to create the enumerator
        ReportError( TEXT("Error calling IWiaDevMgr2::EnumDeviceInfo"), hr );
    }

    // Return the result of the enumeration
    return hr;
}


// The entry function of the application
extern "C" 
int __cdecl _tmain( int, TCHAR *[] )
{
    // Initialize COM
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        // Create the device manager
        IWiaDevMgr2 *pWiaDevMgr2 = NULL;
        hr = CoCreateInstance( CLSID_WiaDevMgr2, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr2, (void**)&pWiaDevMgr2 );
        if (SUCCEEDED(hr))
        {
            //The following function enumerates all of the WIA devices and performs some function on each device.
            hr = EnumerateWiaDevices( pWiaDevMgr2 );
            if (FAILED(hr))
            {
                ReportError( TEXT("Error calling EnumerateWiaDevices"), hr );
            }

            // Release the device manager
            pWiaDevMgr2->Release();
            pWiaDevMgr2 = NULL;
        }
        else
        {
            ReportError( TEXT("CoCreateInstance() failed on CLSID_WiaDevMgr"), hr );
        }

        // Uninitialize COM
        CoUninitialize();
    }
    return 0;
}


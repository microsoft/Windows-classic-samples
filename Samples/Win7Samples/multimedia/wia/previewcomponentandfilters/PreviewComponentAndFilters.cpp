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

#include "PreviewComponentAndFilters.h"

//This function changes the brightness and contrast for the item and then updates the preview. Updating of the preview is done
//by the Image processing filter and no transfer takes place from the scanner.
HRESULT ChangePropsAndUpdatePreview(IWiaItem2* pWiaItem2 ,IWiaPropertyStorage* pWiaPropertyStorage, IWiaPreview* pWiaPreview, IWiaTransferCallback* pWiaTransferCallback)
{
    HRESULT hr = S_OK;
    if(  (!pWiaItem2) || (!pWiaTransferCallback) || (!pWiaPreview) || (!pWiaPropertyStorage) )
    {
        hr = E_INVALIDARG;
        ReportError(TEXT("Invalid arguments passed to ChangePropsAndUpdatePreview()"),hr);
        return hr;
    }

    //Check if Image Processing Filter is supported by the driver
    BOOL bImgFilterExists = FALSE;
    BSTR bstrFilterString = SysAllocString(WIA_IMAGEPROC_FILTER_STR);
    if(bstrFilterString)
    {
        //The third argument to CheckExtension() is unused currently 
        hr = pWiaItem2->CheckExtension(0,bstrFilterString,IID_IWiaImageFilter, &bImgFilterExists);
        if(FAILED(hr))
        {
            ReportError(TEXT("pWiaItem2->CheckExtension() failed in ChangePropsAndUpdatePreview()"),hr);
            return hr;
        }
        SysFreeString(bstrFilterString);
        bstrFilterString = NULL;
        if(bImgFilterExists)
        {  
            //Set different brightness and contrast for the item
            LONG lbrightness = 0, lcontrast=0;
            hr = GetBrightnessContrast(pWiaPropertyStorage,&lbrightness,&lcontrast);

            if(SUCCEEDED(hr))
            {
                WritePropertyLong(pWiaPropertyStorage,WIA_IPS_BRIGHTNESS,lbrightness);
                WritePropertyLong(pWiaPropertyStorage,WIA_IPS_CONTRAST,lcontrast);
                
            }
            else
            {
                ReportError(TEXT("GetBrightnessContrast() failed in ChangePropsAndUpdatePreview(). So brightness and contrast \
                have not been set to their max value.") ,hr);
            }
            //Get the updated preview  
            //The pWiaItem2 stream that was used for GetNewPreview() will be used for transfer in UpdatePreview() 
            //Hence the file represented by that stream will be overwritten by this call
            hr = pWiaPreview->UpdatePreview(0,pWiaItem2,pWiaTransferCallback);
            if (SUCCEEDED(hr))
            {
                _tprintf(TEXT("\npWiaPreview->UpdatePreview() SUCCEEDED in ChangePropsAndUpdatePreview()"));
            }
            else
            {
                ReportError(TEXT("pWiaPreview->UpdatePreview() FAILED in ChangePropsAndUpdatePreview()"),hr);
            }
        }
        else
        {
            _tprintf(TEXT("\nImage Processing Filter not supported for the item"));
        }

    }
    else
    {
        hr = E_OUTOFMEMORY;
        ReportError(TEXT("Could not allocate memory for bstrFilterString"),hr);
    }

    return hr;
}
            
// This function sets different brightness and contrast for the item. 
// Now we have used a formula to get different brightness and contrast from the previous values of brightness and contrast.
// We are setting these to the average of their respective nominal and min values.
HRESULT GetBrightnessContrast(IWiaPropertyStorage* pWiaPropertyStorage,LONG* lBrightness, LONG* lContrast)
{
    if( (!pWiaPropertyStorage))
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid arguments passed to GetBrightnessContrast()"),hr);
        return hr;
    }

    // Initialize out variables
    *lBrightness = 0;
    *lContrast = 0;

    // Declare PROPSPECs and PROPVARIANTs, and initialize them.
    PROPSPEC PropSpec[2] = {0};
    PROPVARIANT PropVar[2];
    PropVariantInit(PropVar);
    
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = WIA_IPS_BRIGHTNESS;
    PropSpec[1].ulKind = PRSPEC_PROPID;
    PropSpec[1].propid = WIA_IPS_CONTRAST;
    ULONG ulAccessFlags[2] = {0};
    
    HRESULT hr = pWiaPropertyStorage->GetPropertyAttributes(2,PropSpec,ulAccessFlags,PropVar);
    if(SUCCEEDED(hr))
    {
        // Setting brightness and contrast to average of their nominal and min values , if step size is not zero.
        // We have to make sure that we assign values to brightness and contrast which are a multiple of lStep.
        // If the step size is zero due to bug in the driver, set brightness and contrast to minimum value.
        LONG lStep = PropVar[0].cal.pElems[WIA_RANGE_STEP];
        LONG lMin = PropVar[0].cal.pElems[WIA_RANGE_MIN];
        LONG lNom = PropVar[0].cal.pElems[WIA_RANGE_NOM];
        LONG lRange = 0;
        if(0 != lStep)
        {
            lRange = (lNom - lMin)/lStep;
            *lBrightness = lMin + ( (lRange/2) * lStep );
        }
        else
        {
            *lBrightness = lMin;
        }
        
        lStep = PropVar[1].cal.pElems[WIA_RANGE_STEP];
        lMin = PropVar[1].cal.pElems[WIA_RANGE_MIN];
        lNom = PropVar[1].cal.pElems[WIA_RANGE_NOM];
        if(0 != lStep)
        {
            lRange = (lNom - lMin)/lStep;
            *lContrast = lMin + ( (lRange/2) * lStep );
        }
        else
        {
            *lContrast = lMin;
        }
          
    }
    else
    {
        ReportError(TEXT("pWiaPropertyStorage->GetPropertyAttributes() failed in GetBrightnessContrast()"),hr);
    }
    return hr;
}


    //
    // Constructor and destructor
    //
    CWiaTransferCallback::CWiaTransferCallback()
    {
        m_cRef             = 1;
        m_lPageCount       = 0;
        m_bstrFileExtension = NULL;
        m_bstrDirectoryName = NULL;
        memset(m_szFileName,0,sizeof(m_szFileName));
    }
    
    CWiaTransferCallback::~CWiaTransferCallback()
    {
        if(m_bstrDirectoryName)
        {
            SysFreeString(m_bstrDirectoryName);
            m_bstrDirectoryName = NULL;
        }
        
        if(m_bstrFileExtension)
        {
            SysFreeString(m_bstrFileExtension);
            m_bstrFileExtension = NULL;
        }

    }
    
    //This function initializes various members of class CWiaTransferCallback like directory where images will be downloaded, 
    //and filename extension.
    HRESULT CWiaTransferCallback::InitializeCallback(TCHAR* bstrDirectoryName, BSTR bstrExt)
    {
        HRESULT hr = S_OK;

        if(bstrDirectoryName)
        {
            m_bstrDirectoryName = SysAllocString(bstrDirectoryName);
            if(!m_bstrDirectoryName)
            {
                hr = E_OUTOFMEMORY;
                ReportError(TEXT("Failed to allocate memory for BSTR directory name"),hr);
                return hr;
            }
        }
        else
        {
            _tprintf(TEXT("\nNo directory name was given"));
            return E_INVALIDARG;
        }

        if (bstrExt)
        {
            m_bstrFileExtension = bstrExt;
        }
        
        return hr;
    }

    
    //
    // IUnknown functions
    //
    HRESULT CALLBACK CWiaTransferCallback::QueryInterface( REFIID riid, void **ppvObject )
    {
        // Validate arguments
        if (NULL == ppvObject)
        {
            HRESULT hr = E_INVALIDARG;
            ReportError(TEXT("Invalid argument passed to QueryInterface()"),hr);
            return hr;
        }

        // Return the appropropriate interface
        if (IsEqualIID( riid, IID_IUnknown ))
        {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if (IsEqualIID( riid, IID_IWiaTransferCallback ))
        {
            *ppvObject = static_cast<IWiaTransferCallback*>(this);
        }
        else
        {
            *ppvObject = NULL;
            return (E_NOINTERFACE);
        }

        // Increment the reference count before we return the interface
        reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
        return S_OK;
    }
    
    ULONG CALLBACK CWiaTransferCallback::AddRef()
    {
        return InterlockedIncrement((long*)&m_cRef);
    }    
    ULONG CALLBACK CWiaTransferCallback::Release()
    {
        LONG cRef = InterlockedDecrement((long*)&m_cRef);
        if (0 == cRef)
        {
            delete this;
        }
        return cRef;
    }
    
//
// IWiaTransferCallback functions
//
// This function is used by the application to notify the progress of the transfer
HRESULT STDMETHODCALLTYPE CWiaTransferCallback::TransferCallback(LONG lFlags, WiaTransferParams* pWiaTransferParams)
{
    HRESULT hr = S_OK;
    
    if(pWiaTransferParams == NULL)
    {
        hr = E_INVALIDARG;
        ReportError(TEXT("TransferCallback() was called with invalid args"),hr);
        return hr;
    }

    switch (pWiaTransferParams->lMessage)
    {
        case WIA_TRANSFER_MSG_STATUS:
            {
                _tprintf(TEXT("\nWIA_TRANSFER_MSG_STATUS - %ld%% complete"),pWiaTransferParams->lPercentComplete);
            }
            break;
        case WIA_TRANSFER_MSG_END_OF_STREAM:
            {
                _tprintf(TEXT("\nWIA_TRANSFER_MSG_END_OF_STREAM"));
            }
            break;
        case WIA_TRANSFER_MSG_END_OF_TRANSFER:
            {
                _tprintf(TEXT("\nWIA_TRANSFER_MSG_END_OF_TRANSFER"));
                _tprintf(TEXT("\nImage Transferred to file %ws"), m_szFileName);
            }
            break;
        default:
            break;
    }
    return hr;
}

// This function is called by WIA service to get data stream from the application.
HRESULT STDMETHODCALLTYPE CWiaTransferCallback::GetNextStream(LONG lFlags, BSTR bstrItemName, BSTR bstrFullItemName, IStream **ppDestination)
{
    _tprintf(TEXT("\nGetNextStream"));
    HRESULT hr = S_OK;
    
    if ( (!ppDestination) || (!bstrItemName) || (!m_bstrDirectoryName) )
    {
        hr = E_INVALIDARG;
        ReportError(TEXT("GetNextStream() was called with invalid parameters"),hr);
        return hr;
    }
    //Initialize out variables
    *ppDestination = NULL;
    
    if(m_bstrFileExtension)
    {
        StringCchPrintf(m_szFileName, ARRAYSIZE(m_szFileName), TEXT("%ws\\%ws.%ws"), m_bstrDirectoryName, bstrItemName, m_bstrFileExtension);
    }
    
    else
    {
        // Dont append extension if m_bstrFileExtension = NULL.
        StringCchPrintf(m_szFileName, ARRAYSIZE(m_szFileName), TEXT("%ws\\%ws"), m_bstrDirectoryName, bstrItemName);
    }
    
    hr = SHCreateStreamOnFile(m_szFileName,STGM_CREATE | STGM_READWRITE,ppDestination);
    if (SUCCEEDED(hr))
    {
        // We're not going to keep the Stream around, so don't AddRef.
        //  The caller will release the stream when done.
    }
    else
    {
        _tprintf(TEXT("\nFailed to Create a Stream on File %ws"),m_szFileName);
        *ppDestination = NULL;
    }
    return hr;
}
// End IWiaTransferCallback functions

//This function performs the following major steps:
//1.Calls GetPreviewComponent() to instantiate the WIA preview component and hence gets a pointer to IWiaPreview interface
//2.Calls IWiaPreview::GetNewPreview() to get unfiltered image from the driver for the first time for that item.
//3.Checks if Segmentation filter is supported by driver. If its supported calls IWiaPreview::DetectRegions() on that item
//  to split the image into sub-regions if there are any. IWiaPreview::DetectRegions() works by calling Segmentation filter internally.
//4.Changes brightness, contrast of the item and updates preview ( through a call to ChangePropsAndUpdatePreview() ).

HRESULT GetPreview( IWiaItem2 *pWiaItem2)
{
    // Validate arguments
    if (NULL == pWiaItem2)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("GetPreview() was called with invalid parameters"),hr);
        return hr;
    }
    IWiaPreview* pWiaPreview = NULL;
    HRESULT hr = pWiaItem2->GetPreviewComponent(0, &pWiaPreview);
    if(SUCCEEDED(hr))
    {
        CWiaTransferCallback *pWiaClassCallback = new CWiaTransferCallback;
        if (pWiaClassCallback)
        {
            // Get the IWiaTransferCallback interface from our callback class.
            IWiaTransferCallback *pWiaTransferCallback = NULL;
            hr = pWiaClassCallback->QueryInterface( IID_IWiaTransferCallback, (void**)&pWiaTransferCallback );
            if(SUCCEEDED(hr))
            {
                //Set the format for the item to BMP
                IWiaPropertyStorage* pWiaPropertyStorage = NULL;
                HRESULT hr = pWiaItem2->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
                if(SUCCEEDED(hr))
                {   
                    hr = WritePropertyGuid(pWiaPropertyStorage,WIA_IPA_FORMAT,WiaImgFmt_BMP);
                    if(FAILED(hr))
                    {
                        ReportError(TEXT("WritePropertyGuid() failed in GetPreview().Format couldn't be set to BMP"),hr);
                    }
           
                    //Get the file extension
                    BSTR bstrFileExtension = NULL;
                    ReadPropertyBSTR(pWiaPropertyStorage,WIA_IPA_FILENAME_EXTENSION, &bstrFileExtension);
            
                    //Get the temporary folder path which is the directory where we will download the images
                    TCHAR bufferTempPath[MAX_TEMP_PATH];
                    GetTempPath(MAX_TEMP_PATH , bufferTempPath);
            
                    //Find the item type and item category
                    LONG lItemType = 0;
                    hr = pWiaItem2->GetItemType( &lItemType );
        
                    GUID itemCategory = GUID_NULL;
                    ReadPropertyGuid(pWiaItem2,WIA_IPA_ITEM_CATEGORY,&itemCategory );
        
                    if(IsEqualGUID(itemCategory,WIA_CATEGORY_FEEDER))
                    {
                        //Set the no of pages = 1 since image processing filter can cache only one image at a time
                        WritePropertyLong(pWiaPropertyStorage,WIA_IPS_PAGES,1);
                    }
                    
                    //Initialize the callback class with the directory and file extension  
                    pWiaClassCallback->InitializeCallback(bufferTempPath,bstrFileExtension);
        
                    //call GetNewPreview() to get the unfiltered image from the driver 
                    hr = pWiaPreview->GetNewPreview(0,pWiaItem2,pWiaTransferCallback);
                    if (SUCCEEDED(hr))
                    {
                        _tprintf(TEXT("\npWiaPreview->GetNewPreview() SUCCEEDED for the item"));
                        //Check if segmentation filter is supported by the driver
                        LONG lSegmentation = WIA_USE_SEGMENTATION_FILTER;
                        hr = ReadPropertyLong(pWiaItem2, WIA_IPS_SEGMENTATION, &lSegmentation );

                        if (S_OK == hr)
                        {
                            _tprintf(TEXT("\nSegmentation filter is supported for the item"));
                            //Now check if the application should use the segmentation filter for multi-region scanning
                            if(WIA_USE_SEGMENTATION_FILTER == lSegmentation)
                            {
                                //Invoke the segmentation filter
                                hr = pWiaPreview->DetectRegions(0);
                            
                                if(S_OK == hr)
                                {
                                    //Its not necessary that child items will be created if DetectRegions() succeeds 
                                    _tprintf(TEXT("\npWiaPreview->DetectRegions() SUCCEEDED for the item"));
                                }
                                else if(FAILED( hr))
                                {
                                    ReportError(TEXT("pWiaPreview->DetectRegions() failed for the item"),hr);
                                }
                            }
                            
                        }
                        else if (S_FALSE == hr)
                        {
                            _tprintf(TEXT("\nSegmentation Filter not supported for the item\n"));
                        }
                        else if(FAILED(hr))
                        {
                            ReportError(TEXT("ReadPropertyLong() failed in GetPreview() for the item\n"),hr);
                        }

                        // Change properties and get Updated preview for the item
                        hr = ChangePropsAndUpdatePreview(pWiaItem2,pWiaPropertyStorage,pWiaPreview,pWiaTransferCallback);
            
                    }
                    else
                    {
                        ReportError(TEXT("pWiaPreview->GetNewPreview() FAILED for the item"),hr);
                    }

                    //Release pWiaPropertyStorage 
                    pWiaPropertyStorage->Release();
                    pWiaPropertyStorage = NULL;
                }
                else
                {
                    ReportError(TEXT("QueryInterface failed on IID_IWiaPropertyStorage"),hr);
                }

                // Release pWiaTransferCallback 
                pWiaTransferCallback->Release();
                pWiaTransferCallback = NULL;
            }
            else
            {
                ReportError(TEXT("QueryInterface failed on IID_IWiaTransferCallback"));
            }
            
            // Release our class callback.  It should now delete itself.
            pWiaClassCallback->Release();
            pWiaClassCallback = NULL;
        }
        else
        {
            ReportError( TEXT("Unable to create CWiaTransferCallback class instance") );
        }
        //Release pWiaPreview interface
        pWiaPreview->Release();
        pWiaPreview = NULL;

    }
    else
    {
        ReportError( TEXT("pWiaItem2->GetPreviewComponent() failed in GetPreview() function") );
    }
        
    return hr;
}


// This function enumerates items recursively starting from the root item and calls GetPreview() on those items
// after checking their item types.
// GetPreview() will only be called on file items. Folder items are recursively enumerated.
HRESULT EnumerateAndPreviewItems( IWiaItem2 *pWiaItem2 )
{
    // Validate arguments
    if (NULL == pWiaItem2)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to EnumerateAndPreviewItems()"),hr);
        return hr;
    }

    // Get the item type for this item
    LONG lItemType = 0;
    HRESULT hr = pWiaItem2->GetItemType( &lItemType );

    // Print the item name.
    PrintItemName( pWiaItem2 );

    //Find the item category
    GUID itemCategory = GUID_NULL;
    ReadPropertyGuid(pWiaItem2,WIA_IPA_ITEM_CATEGORY,&itemCategory );
     
    // If this is an transferrable file (except finished files since we can't set brightness,contrast etc for them) , preview it
    // Preview Component cannot be used for folder acquisitions
    if ( (lItemType & WiaItemTypeFile) && (lItemType & WiaItemTypeTransfer) && (itemCategory != WIA_CATEGORY_FINISHED_FILE) )
    {
        hr = GetPreview( pWiaItem2 );
    }

    // If it is a folder, enumerate its children
    if (lItemType & WiaItemTypeFolder)
    {
        // Get the child item enumerator for this item
        IEnumWiaItem2 *pEnumWiaItem2 = NULL;
        hr = pWiaItem2->EnumChildItems(0, &pEnumWiaItem2 );
        if (SUCCEEDED(hr))
        {
            // We will loop until we get an error or pEnumWiaItem->Next returns
            // S_FALSE to signal the end of the list.
            while (S_OK == hr )
            {
                // Get the next child item
                IWiaItem2 *pChildWiaItem2 = NULL;
                hr = pEnumWiaItem2->Next( 1, &pChildWiaItem2, NULL );

                // pEnumWiaItem->Next will return S_FALSE when the list is
                // exhausted, so check for S_OK before using the returned
                // value.
                if (S_OK == hr)
                {
                    // Recurse into this item
                    EnumerateAndPreviewItems( pChildWiaItem2 );

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
//After that it calls EnumerateAndPreviewItems() on the root item got from creation of device.
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
                            // Enumerate items and for each item do transfer
                            hr1 = EnumerateAndPreviewItems( pWiaRootItem2 );
                            if(FAILED(hr1))
                            {
                                ReportError(TEXT("EnumerateAndPreviewItems() failed in EnumerateWiaDevices()"),hr1);
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
            //The following function enumerates all of the WIA devices and performs some function on each device
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


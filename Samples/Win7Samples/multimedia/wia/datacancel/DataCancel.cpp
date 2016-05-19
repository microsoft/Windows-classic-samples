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

#include "DataCancel.h"

// This function downloads item after setting the format for the item and initializing callback (depending on category,itemtype)
// with the directory to download images to as well as the filename for the downloaded image.
HRESULT DownloadItem(IWiaItem2* pWiaItem2)
{
    if( (!pWiaItem2) )
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to DownloadItem()"),hr);
        return hr;
    }
     // Get the IWiaTransfer interface
    IWiaTransfer *pWiaTransfer = NULL;
    HRESULT hr = pWiaItem2->QueryInterface( IID_IWiaTransfer, (void**)&pWiaTransfer );
    if (SUCCEEDED(hr))
    {
        // Create our callback class
        CWiaTransferCallback *pWiaClassCallback = new CWiaTransferCallback;
        if (pWiaClassCallback)
        {
            // Get the IWiaTransferCallback interface from our callback class.
            IWiaTransferCallback *pWiaTransferCallback = NULL;
            hr = pWiaClassCallback->QueryInterface( IID_IWiaTransferCallback, (void**)&pWiaTransferCallback );
            if (SUCCEEDED(hr))
            {
                //Set the format for the item to BMP
                IWiaPropertyStorage* pWiaPropertyStorage = NULL;
                HRESULT hr = pWiaItem2->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
                if(SUCCEEDED(hr))
                {
                    hr = WritePropertyGuid(pWiaPropertyStorage,WIA_IPA_FORMAT,WiaImgFmt_BMP);
                    if(FAILED(hr))
                    {
                        ReportError(TEXT("WritePropertyGuid() failed in DownloadItem().Format couldn't be set to BMP"),hr);
                    }
           
                    //Get the file extension 
                    BSTR bstrFileExtension = NULL;
                    ReadPropertyBSTR(pWiaPropertyStorage,WIA_IPA_FILENAME_EXTENSION, &bstrFileExtension);
            
                    //Get the temporary folder path which is the directory where we will download the images
                    TCHAR bufferTempPath[MAX_TEMP_PATH];
                    GetTempPath(MAX_TEMP_PATH , bufferTempPath);
            
                    // Find out item type
                    GUID itemCategory = GUID_NULL;
                    ReadPropertyGuid(pWiaItem2,WIA_IPA_ITEM_CATEGORY,&itemCategory );
                    
                    BOOL bFeederTransfer = FALSE;
                    
                    if(IsEqualGUID(itemCategory,WIA_CATEGORY_FEEDER))
                    {
                        //Set WIA_IPS_PAGES to ALL_PAGES will enable transfer of all pages in the document feeder (multi-page transfer).
                        //If somebody wants to scan a specific number of pages say N, he should set WIA_IPS_PAGES to N. 
                        WritePropertyLong(pWiaPropertyStorage,WIA_IPS_PAGES,ALL_PAGES);
                         
                        bFeederTransfer = TRUE;
                    }

                    //Initialize the callback class with the directory, file extension and feeder flag
                    pWiaClassCallback->InitializeCallback(bufferTempPath,bstrFileExtension,bFeederTransfer,pWiaTransfer);
                    
                    //Now download file item
                    hr = pWiaTransfer->Download(0,pWiaTransferCallback);
                    if(S_OK == hr)
                    {
                        _tprintf(TEXT("\npWiaTransfer->Download() on file item SUCCEEDED"));
                    }
                    else if(S_FALSE == hr)
                    {
                        ReportError(TEXT("pWiaTransfer->Download() on file item returned S_FALSE. File may be empty"),hr);
                    }
                    else if(FAILED(hr))
                    {
                        ReportError(TEXT("pWiaTransfer->Download() on file item failed"),hr);
                    }
            
                    //Release pWiaPropertyStorage interface
                    pWiaPropertyStorage->Release();
                    pWiaPropertyStorage = NULL;
                }
                else
                {
                    ReportError(TEXT("QueryInterface failed on IID_IWiaPropertyStorage"),hr);
                }

                // Release the callback interface
                pWiaTransferCallback->Release();
                pWiaTransferCallback = NULL;
            }
            else
            {
                ReportError( TEXT("pWiaClassCallback->QueryInterface failed on IID_IWiaTransferCallback"), hr );
            }
            // Release our callback.  It should now delete itself.
            pWiaClassCallback->Release();
            pWiaClassCallback = NULL;
        }
        else
        {
            ReportError( TEXT("Unable to create CWiaTransferCallback class instance") );
        }
            
        // Release the IWiaTransfer
        pWiaTransfer->Release();
        pWiaTransfer = NULL;
    }
    else
    {
        ReportError( TEXT("pIWiaItem2->QueryInterface failed on IID_IWiaTransfer"), hr );
    }
    return hr;
}

    //
    // Constructor and destructor
    //
    CWiaTransferCallback::CWiaTransferCallback()
    {
        m_cRef             = 1;  // initializing it to 1 so that when object is created, we can call Release() on it.
        m_lPageCount       = 0;
        m_bFeederTransfer  = FALSE;
        m_bstrFileExtension = NULL;
        m_bstrDirectoryName = NULL;
        memset(m_szFileName,0,sizeof(m_szFileName));
        m_pWiaTransfer = NULL;
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

    // This function initializes various members of class CWiaTransferCallback like directory where images will be downloaded, 
    // filename extension and feeder transfer flag. It also initializes m_pWiaTransfer.
    HRESULT CWiaTransferCallback::InitializeCallback(TCHAR* bstrDirectoryName, BSTR bstrExt, BOOL bFeederTransfer, IWiaTransfer* pWiaTransfer )
    {
        HRESULT hr = S_OK;
        m_bFeederTransfer = bFeederTransfer;
        m_pWiaTransfer = pWiaTransfer;

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
            return E_INVALIDARG;
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
//
// This function is used by the application to notify the progress of the transfer.
// In this sample we will use this function to cancel the data transfer using the pointer to IWiaTransfer 
// stored as member by the callback class.
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
                
                // We will cancel the transfer of data as soon as WIA service calls the Application callback .ie. the first time 
                // TransferCallback() function is called.  
                if(pWiaTransferParams->lPercentComplete >= 0)
                {
                    //There are 2 ways of cancelling a data transfer 
                    // 1. Call IWiaTransfer::cancel() (recommended way)
                    // 2. Return S_FALSE from TransferCallback().
                    // Here we are using the 1st method. 
                    m_pWiaTransfer->Cancel();
                    _tprintf(TEXT("\nData Tranfer cancelled by user"));

                    // Alternately we can return S_FALSE from TransferCallback() as shown 
                    // below in pseudo code:
                    // 
                    // if(pWiaTransferParams->lPercentComplete >=0)
                    // {
                    //     _tprintf(TEXT("\nData Tranfer cancelled by user"));
                    //     return S_FALSE;
                    // }
                    //

                }
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
        //For feeder transfer, append the page count to the filename
        if (m_bFeederTransfer)
        {
            StringCchPrintf(m_szFileName, ARRAYSIZE(m_szFileName), TEXT("%ws\\%ws_page%d.%ws"), m_bstrDirectoryName, bstrItemName, ++m_lPageCount, m_bstrFileExtension);
        }
        else
        {
            StringCchPrintf(m_szFileName, ARRAYSIZE(m_szFileName), TEXT("%ws\\%ws.%ws"), m_bstrDirectoryName, bstrItemName, m_bstrFileExtension);
        }
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


// This function enumerates items recursively starting from the root item and calls DownloadItem() on those items
// after checking their item types.
// DownloadItem() will only be called on file items. Folder items are recursively enumerated.
HRESULT EnumerateAndDownloadItems( IWiaItem2 *pWiaItem2 )
{
    // Validate arguments
    if (NULL == pWiaItem2)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to EnumerateAndDownloadItems()"),hr);
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
     
    // If this is an transferrable file(except finished files) , download it.
    // We have deliberately left finished files for simplicity
    if ( (lItemType & WiaItemTypeFile) && (lItemType & WiaItemTypeTransfer) && (itemCategory != WIA_CATEGORY_FINISHED_FILE) )
    {
        hr = DownloadItem( pWiaItem2 );
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
                    EnumerateAndDownloadItems( pChildWiaItem2 );

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
        else
        {
            ReportError(TEXT("pWiaItem2->EnumChildItems() failed"),hr);
        }
    }
    return  hr;
}

//This function enumerates WIA devices and then creates an instance of each device.
//After that it calls EnumerateAndDownloadItems() on the root item got from creation of device.
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
                            hr1 = EnumerateAndDownloadItems( pWiaRootItem2 );
                            if(FAILED(hr1))
                            {
                                ReportError(TEXT("EnumerateAndDownloadItems() failed in EnumerateWiaDevices()"),hr1);
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


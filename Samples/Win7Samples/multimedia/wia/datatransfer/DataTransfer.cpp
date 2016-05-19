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

#include "DataTransfer.h"

// This function downloads item after setting the format for the item and initializing callback (depending on category,itemtype)
// with the directory to download images to as well as the filename for the downloaded image.
// The function downloads both file items as well as folder items depending on "bTransferFlag".
HRESULT DownloadItem(IWiaItem2* pWiaItem2 , BOOL bTransferFlag)
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
                    //Find out item category
                    GUID itemCategory = GUID_NULL;
                    ReadPropertyGuid(pWiaItem2,WIA_IPA_ITEM_CATEGORY,&itemCategory );
                    
                    if( (!IsEqualIID(itemCategory,WIA_CATEGORY_FINISHED_FILE)) || (!IsEqualIID(itemCategory,WIA_CATEGORY_FOLDER)) )
                    {
                        hr = WritePropertyGuid(pWiaPropertyStorage,WIA_IPA_FORMAT,WiaImgFmt_BMP);
                    }
                    
                    if(FAILED(hr))
                    {
                        ReportError(TEXT("WritePropertyGuid() failed in DownloadItem().Format couldn't be set to BMP"),hr);
                    }
           
                    //Get the file extension 
                    BSTR bstrFileExtension = NULL;
                    if(!IsEqualIID(itemCategory,WIA_CATEGORY_FOLDER))
                    {
                        ReadPropertyBSTR(pWiaPropertyStorage,WIA_IPA_FILENAME_EXTENSION, &bstrFileExtension);
                    }
            
                    //Get the temporary folder path which is the directory where we will download the images
                    TCHAR bufferTempPath[MAX_TEMP_PATH];
                    GetTempPath(MAX_TEMP_PATH , bufferTempPath);
            
                    
                    //Find the item type 
                    LONG lItemType = 0;
                    hr = pWiaItem2->GetItemType( &lItemType );
                    
                    BOOL bFeederTransfer = FALSE; 
                    if(IsEqualGUID(itemCategory,WIA_CATEGORY_FEEDER))
                    {
                        //Set WIA_IPS_PAGES to ALL_PAGES will enable transfer of all pages in the document feeder (multi-page transfer).
                        //If somebody wants to scan a specific number of pages say N, he should set WIA_IPS_PAGES to N. 
                        WritePropertyLong(pWiaPropertyStorage,WIA_IPS_PAGES,ALL_PAGES);
                         
                        bFeederTransfer = TRUE;
            
                    }
                    else if(lItemType & WiaItemTypeStorage)
                    {
                        //We are setting file extension null for storage items since we are uploading files already with an extension.
                        //If other storage items are present on the device, for them also we are assuming that they already have extension
                        //embedded in their names
                        bstrFileExtension = NULL;
                    }
                    
                    pWiaClassCallback->InitializeCallback(bufferTempPath,bstrFileExtension,bFeederTransfer);
                    
                    //Now download based on whether its a folder item or a file item
                    if(bTransferFlag == FOLDER_TRANSFER)
                    {
                        hr = pWiaTransfer->Download(WIA_TRANSFER_ACQUIRE_CHILDREN,pWiaTransferCallback);
                        if(S_OK == hr)
                        {
                            _tprintf(TEXT("\npWiaTransfer->Download() on folder item SUCCEEDED"));
                        }
                        else if(S_FALSE == hr)
                        {
                            ReportError(TEXT("pWiaTransfer->Download() on folder item returned S_FALSE. Folder may not be having child items"),hr);
                        }
                        else if(FAILED(hr))
                        {
                            ReportError(TEXT("pWiaTransfer->Download() on folder item failed"),hr);
                        }
                    }
                    else  
                    //FILE_TRANSFER
                    {
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

// Function to upload file/image to device storage. This function first creates a child item inside the root storage folder.
// Then uploads a file UPLOAD_FILENAME present in the current directory by getting its stream and giving the stream to 
// IWiaTransfer::upload() function.
HRESULT UploadToDeviceStorage(IWiaItem2* pWiaParentItem2 )
{
    if( (!pWiaParentItem2) )
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to UploadToDeviceStorage()"),hr);
        return hr;
    }
    
    // CHILDITEM_NAME is the file name for the child item
    // See DataTransfer.h file for its value
    BSTR bzChildItem_Name = SysAllocString(CHILDITEM_NAME);
    
    if(!bzChildItem_Name)
    {
        HRESULT hr = E_OUTOFMEMORY;
        ReportError(TEXT("Failed to allocate memory for bzChildItem_Name"),hr);
        return hr;
    }
    
    //Create child item which eventually will be the uploaded image 
    IWiaItem2* pWiaItemChild = NULL;
    HRESULT hr = pWiaParentItem2->CreateChildItem(WiaItemTypeImage|WiaItemTypeFile,0,bzChildItem_Name,&pWiaItemChild);
    
    if(SUCCEEDED(hr))
    {
                
        //Set the format for the child item as BMP so that any application downloading the uploaded item can check its format.
        IWiaPropertyStorage* pWiaChildPropertyStorage = NULL;
        hr = pWiaItemChild->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaChildPropertyStorage );
        if(SUCCEEDED(hr))
        {
            WritePropertyGuid(pWiaChildPropertyStorage,WIA_IPA_FORMAT,WiaImgFmt_BMP );

            //release pWiaChildPropertyStorage
            pWiaChildPropertyStorage->Release();
            pWiaChildPropertyStorage = NULL;
        }
        else
        {
            ReportError(TEXT("pWiaItemChild->QueryInterface failed for interface IID_IWiaPropertyStorage"),hr);
        }

        IStream* pUploadStream = NULL;
                                        
        //Create stream on UPLOAD_FILENAME which is present in current directory
        //See DataTransfer.h file for its value
        hr = SHCreateStreamOnFile(UPLOAD_FILENAME,STGM_READ, &pUploadStream);
        if(SUCCEEDED(hr))
        {
            //Get the IWiaTransfer interface of the child
            IWiaTransfer* pWiaTransferChild = NULL;
            hr = pWiaItemChild->QueryInterface( IID_IWiaTransfer, (void**)&pWiaTransferChild );
            if(SUCCEEDED(hr))
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
            
                        pWiaTransferChild->Upload(0,pUploadStream,pWiaTransferCallback);
            
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
            
                //Release pWiaTransferChild 
                pWiaTransferChild->Release();
                pWiaTransferChild = NULL;

            }
            else
            {
                ReportError(TEXT("pWiaItemChild->QueryInterface failed for interface IID_IWiaTransfer"),hr);
            }
        }
        else
        {
            ReportError(TEXT
            ("Failed to create Stream on FILE for upload. Make sure that UPLOAD_FILENAME is present in the current directory"),hr);
        }

        // Release pWiaItemChild
        pWiaItemChild->Release();
        pWiaItemChild = NULL;

    }  
    else
    {
        ReportError(TEXT("pWiaParentItem2->CreateChildItem failed"),hr);
    }

    return hr;
}
    //
    // Constructor and destructor for the class CWiaTransferCallback
    //
    CWiaTransferCallback::CWiaTransferCallback()
    {
        m_cRef             = 1;  // initializing it to 1 so that when object is created, we can call Release() on it. 
        m_lPageCount       = 0;
        m_bFeederTransfer  = FALSE;
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

    // This function initializes various members of class CWiaTransferCallback like directory where images will be downloaded, 
    // filename extension and feeder transfer flag
    HRESULT CWiaTransferCallback::InitializeCallback(TCHAR* bstrDirectoryName, BSTR bstrExt, BOOL bFeederTransfer)
    {
        HRESULT hr = S_OK;
        m_bFeederTransfer = bFeederTransfer;

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


//This function checks the type of the item and decides whether the item is to be downloaded or uploaded.
HRESULT TransferWiaItem( IWiaItem2 *pIWiaItem2)
{
    // Validate arguments
    if (NULL == pIWiaItem2)
    {
        HRESULT hr = E_INVALIDARG;
        _tprintf(TEXT("\nInvalid parameters passed"),hr);
        return hr;
    }
    
    LONG lItemType = 0;
    HRESULT hr = pIWiaItem2->GetItemType( &lItemType );
    //download all items which have WiaItemTypeTransfer flag set
    if(lItemType & WiaItemTypeTransfer)
    {
        // If it is a folder, do folder download . Hence with one API call, all the leaf nodes of this folder 
        // will be transferred
        if ((lItemType & WiaItemTypeFolder))
        {
            _tprintf(TEXT("\nThis is a folder item"));
            hr = DownloadItem(pIWiaItem2,FOLDER_TRANSFER);
        }
        
        // If this is an file type, do file download
        if (lItemType & WiaItemTypeFile )
        {
            _tprintf(TEXT("\nThis is a file item") );
            hr = DownloadItem(pIWiaItem2,FILE_TRANSFER);
        }
    }
        
    //If it is storage type , then upload image to it. We are uploading to the root storage item.
    //For an item with WiaItemTypeStorage flag set, its WIA_IPA_ITEM_CATEGORY will be WIA_CATEGORY_FOLDER
    else if( (lItemType & WiaItemTypeStorage))
    {
        _tprintf(TEXT("\nThis is a storage item"));
        hr = UploadToDeviceStorage(pIWiaItem2);
        if(SUCCEEDED(hr))
        {
            hr = DownloadItem(pIWiaItem2,FOLDER_TRANSFER);
        }
        else
        {
            _tprintf(TEXT("\nUploadToDeviceStorage() in TransferWiaItem() failed"));
        }
    }
    return hr;
}


// This function enumerates the root item and performs transfer(download/upload) of items got by enumeration. 
// While enumerating the root item, we can get both folder and file items. 
HRESULT EnumerateAndTransferItems( IWiaItem2 *pIWiaItem2Root )
{
    if (NULL == pIWiaItem2Root)
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid argument passed to EnumerateAndTransferItems()"),hr);
        return hr;
    }
    
    PrintItemName(pIWiaItem2Root);
    
    IEnumWiaItem2 *pEnumWiaItem2 = NULL;
    HRESULT hr = pIWiaItem2Root->EnumChildItems( 0, &pEnumWiaItem2 );
    if (SUCCEEDED(hr))
    {
    // We will loop until we get an error or pEnumWiaItem2->Next returns
    // S_FALSE to signal the end of the list.
        while (S_OK == hr)
        {
        // Get the next child item
            IWiaItem2 *pChildWiaItem2 = NULL;
            hr = pEnumWiaItem2->Next( 1, &pChildWiaItem2, NULL );
            if(S_OK == hr)
            {
                PrintItemName(pChildWiaItem2);
                TransferWiaItem(pChildWiaItem2);
                
                //Release pChildWiaItem2
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
        ReportError(TEXT("pIWiaItem2->EnumChildItems() failed in EnumerateItems"),hr);
    }
    return hr;
}

//This function enumerates WIA devices and then creates an instance of each device.
//After that it calls EnumerateAndTransferItems() on the root item got from creation of device.
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
                            hr1 = EnumerateAndTransferItems( pWiaRootItem2 );
                            if(FAILED(hr1))
                            {
                                ReportError(TEXT("EnumerateAndTransferItems() failed in EnumerateWiaDevices()"),hr1);
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
            // The below function enumerates all of the WIA devices and performs some function on each device.
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


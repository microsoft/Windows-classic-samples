/*******************************************************************************
 *
 *  (C) COPYRIGHT MICROSOFT CORPORATION
 *
 *  TITLE:       WIASSAMP.CPP
 *
 *  VERSION:     1.0
 *
 *  AUTHOR:      ShaunIv
 *
 *  DATE:        4/3/2002
 *
 *  DESCRIPTION: Simple command line program which demonstrates the following
 *               WIA procedures:
 *               
 *               o  Enumerating devices
 *               o  Reading properties
 *               o  Setting properties
 *               o  Enumerating items
 *               o  Transferring items to a file
 *
 *******************************************************************************/
#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <tchar.h>
#include <wia.h>

//
// Helper function to display an error message and an optional HRESULT
//
void ReportError( LPCTSTR pszMessage, HRESULT hr = S_OK )
{
    if (S_OK != hr)
    {
        _tprintf( TEXT("%s: HRESULT: 0x%08X\n"), pszMessage, hr );
    }
    else
    {
        _tprintf( TEXT("%s\n"), pszMessage );
    }
}

class CWiaDataCallback : public IWiaDataCallback
{
private:
    //
    // Reference count
    //
    LONG m_cRef;

private:
    //
    // No implementation
    //
    CWiaDataCallback( const CWiaDataCallback & );
    CWiaDataCallback &operator=( const CWiaDataCallback & );

public:
    //
    // Constructor and destructor
    //
    CWiaDataCallback()
        : m_cRef(1)
    {
    }
    virtual ~CWiaDataCallback()
    {
    }

    //
    // IUnknown functions
    //
    HRESULT CALLBACK QueryInterface( REFIID riid, void **ppvObject )
    {
        //
        // Validate arguments
        //
        if (NULL == ppvObject)
        {
            return E_INVALIDARG;
        }

        //
        // Return the appropropriate interface
        //
        if (IsEqualIID( riid, IID_IUnknown ))
        {
            *ppvObject = static_cast<CWiaDataCallback*>(this);
        }
        else if (IsEqualIID( riid, IID_IWiaDataCallback ))
        {
            *ppvObject = static_cast<CWiaDataCallback*>(this);
        }
        else
        {
            *ppvObject = NULL;
            return (E_NOINTERFACE);
        }

        //
        // Increment the reference count before we return the interface
        //
        reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
        return S_OK;
    }
    ULONG CALLBACK AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }    
    ULONG CALLBACK Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);
        if (0 == cRef)
        {
            delete this;
        }
        return cRef;
    }
    
    //
    // IWiaDataCallback functions
    //
    HRESULT CALLBACK BandedDataCallback(
        LONG lMessage,
        LONG lStatus,
        LONG lPercentComplete,
        LONG lOffset,
        LONG lLength,
        LONG lReserved,
        LONG lResLength,
        BYTE *pbBuffer )
    {
        UNREFERENCED_PARAMETER(lMessage);
        UNREFERENCED_PARAMETER(lStatus);
        UNREFERENCED_PARAMETER(lPercentComplete);
        UNREFERENCED_PARAMETER(lOffset);
        UNREFERENCED_PARAMETER(lLength);
        UNREFERENCED_PARAMETER(lReserved);
        UNREFERENCED_PARAMETER(lResLength);
        UNREFERENCED_PARAMETER(pbBuffer);
        switch (lMessage)
        {
        case IT_MSG_STATUS:
            _tprintf( TEXT("lPercentComplete: %d\n"), lPercentComplete );
            break;
        }

        return S_OK;
    }
};

HRESULT TransferWiaItem( IWiaItem *pWiaItem )
{
    //
    // Validate arguments
    //
    if (NULL == pWiaItem)
    {
        return E_INVALIDARG;
    }
    
    //
    // Get the IWiaPropertyStorage interface so we can set required properties
    //
    IWiaPropertyStorage *pWiaPropertyStorage = NULL;
    HRESULT hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
    if (SUCCEEDED(hr))
    {
        //
        // Prepare PROPSPECs and PROPVARIANTs for setting the
        // media type and format
        //
        PROPSPEC PropSpec[2] = {0};
        PROPVARIANT PropVariant[2] = {0};
        const ULONG c_nPropCount = sizeof(PropVariant)/sizeof(PropVariant[0]);

        //
        // Use BMP as the output format
        //
        GUID guidOutputFormat = WiaImgFmt_BMP;

        //
        // Initialize the PROPSPECs
        //
        PropSpec[0].ulKind = PRSPEC_PROPID;
        PropSpec[0].propid = WIA_IPA_FORMAT;
        PropSpec[1].ulKind = PRSPEC_PROPID;
        PropSpec[1].propid = WIA_IPA_TYMED;

        //
        // Initialize the PROPVARIANTs
        //
        PropVariant[0].vt = VT_CLSID;
        PropVariant[0].puuid = &guidOutputFormat;
        PropVariant[1].vt = VT_I4;
        PropVariant[1].lVal = TYMED_FILE;

        //
        // Set the properties
        //
        hr = pWiaPropertyStorage->WriteMultiple( c_nPropCount, PropSpec, PropVariant, WIA_IPA_FIRST );
        if (SUCCEEDED(hr))
        {
            //
            // Get the IWiaDataTransfer interface
            //
            IWiaDataTransfer *pWiaDataTransfer = NULL;
            hr = pWiaItem->QueryInterface( IID_IWiaDataTransfer, (void**)&pWiaDataTransfer );
            if (SUCCEEDED(hr))
            {
                //
                // Create our callback class
                //
                CWiaDataCallback *pCallback = new CWiaDataCallback;
                if (pCallback)
                {
                    //
                    // Get the IWiaDataCallback interface from our callback class.
                    //
                    IWiaDataCallback *pWiaDataCallback = NULL;
                    hr = pCallback->QueryInterface( IID_IWiaDataCallback, (void**)&pWiaDataCallback );
                    if (SUCCEEDED(hr))
                    {
                        //
                        // Perform the transfer using default settings
                        //
                        STGMEDIUM stgMedium = {0};
                        hr = pWiaDataTransfer->idtGetData( &stgMedium, pWiaDataCallback );
                        if (S_OK == hr)
                        {
                            //
                            // Print the filename (note that this filename is always
                            // a WCHAR string, not TCHAR.
                            //
                            _tprintf( TEXT("Transferred filename: %ws\n"), stgMedium.lpszFileName );

                            //
                            // Release any memory associated with the stgmedium
                            //
                            ReleaseStgMedium( &stgMedium );
                        }
                        else
                        {
                            ReportError( TEXT("pWiaDataTransfer->idtGetData failed"), hr );
                        }

                        //
                        // Release the callback interface
                        //
                        pWiaDataCallback->Release();
                        pWiaDataCallback = NULL;
                    }
                    else
                    {
                        ReportError( TEXT("pCallback->QueryInterface failed on IID_IWiaDataCallback"), hr );
                    }

                    //
                    // Release our callback.  It should now delete itself.
                    //
                    pCallback->Release();
                    pCallback = NULL;
                }
                else
                {
                    ReportError( TEXT("Unable to create CWiaDataCallback class instance") );
                }

                //
                // Release the IWiaDataTransfer
                //
                pWiaDataTransfer->Release();
                pWiaDataTransfer = NULL;
            }
            else
            {
                ReportError( TEXT("pWiaItem->QueryInterface failed on IID_IWiaDataTransfer"), hr );
            }
        }

        //
        // Release the IWiaPropertyStorage
        //
        pWiaPropertyStorage->Release();
        pWiaPropertyStorage = NULL;
    }
    else
    {
        ReportError( TEXT("pWiaItem->QueryInterface failed on IID_IWiaPropertyStorage"), hr );
    }

    return hr;
}

HRESULT PrintItemName( IWiaItem *pWiaItem )
{
    //
    // Validate arguments
    //
    if (NULL == pWiaItem)
    {
        return E_INVALIDARG;
    }

    //
    // Get the IWiaPropertyStorage interface
    //
    IWiaPropertyStorage *pWiaPropertyStorage = NULL;
    HRESULT hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
    if (SUCCEEDED(hr))
    {
        //
        // Declare PROPSPECs and PROPVARIANTs, and initialize them to zero.
        //
        PROPSPEC PropSpec[1] = {0};
        PROPVARIANT PropVar[1] = {0};

        //
        // How many properties are we querying for?
        //
        const ULONG c_nPropertyCount = sizeof(PropSpec)/sizeof(PropSpec[0]);

        //
        // Define which properties we want to read:
        // Device ID.  This is what we'd use to create
        // the device.
        //
        PropSpec[0].ulKind = PRSPEC_PROPID;
        PropSpec[0].propid = WIA_IPA_FULL_ITEM_NAME;

        //
        // Ask for the property values
        //
        hr = pWiaPropertyStorage->ReadMultiple( c_nPropertyCount, PropSpec, PropVar );
        if (SUCCEEDED(hr))
        {
            //
            // IWiaPropertyStorage::ReadMultiple will return S_FALSE if some
            // properties could not be read, so we have to check the return
            // types for each requested item.
            //
            
            //
            // Check the return type for the device ID
            //
            if (VT_BSTR == PropVar[0].vt)
            {
                //
                // Do something with the device ID
                //
                _tprintf( TEXT("Item Name: %ws\n"), PropVar[0].bstrVal );
            }

            //
            // Free the returned PROPVARIANTs
            //
            FreePropVariantArray( c_nPropertyCount, PropVar );
        }
        else
        {
            ReportError( TEXT("Error calling IWiaPropertyStorage::ReadMultiple"), hr );
        }

        //
        // Release the IWiaPropertyStorage interface
        //
        pWiaPropertyStorage->Release();
        pWiaPropertyStorage = NULL;
    }

    //
    // Return the result of reading the properties
    //
    return hr;
}

HRESULT CreateWiaDevice( IWiaDevMgr *pWiaDevMgr, BSTR bstrDeviceID, IWiaItem **ppWiaDevice )
{
    //
    // Validate arguments
    //
    if (NULL == pWiaDevMgr || NULL == bstrDeviceID || NULL == ppWiaDevice)
    {
        return E_INVALIDARG;
    }

    //
    // Initialize out variables
    //
    *ppWiaDevice = NULL;

    //
    // Create the WIA Device
    //
    HRESULT hr = pWiaDevMgr->CreateDevice( bstrDeviceID, ppWiaDevice );
    if (FAILED(hr))
    {
        ReportError( TEXT("Error calling IWiaDevMgr::CreateDevice"), hr );
    }

    //
    // Return the result of creating the device
    //
    return hr;
}

HRESULT CreateWiaDeviceManager( IWiaDevMgr **ppWiaDevMgr )
{
    //
    // Validate arguments
    //
    if (NULL == ppWiaDevMgr)
    {
        return E_INVALIDARG;
    }

    //
    // Initialize out variables
    //
    *ppWiaDevMgr = NULL;

    //
    // Create an instance of the device manager
    //
    HRESULT hr = CoCreateInstance( CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)ppWiaDevMgr );
    if (FAILED(hr))
    {
        ReportError( TEXT("CoCreateInstance failed on CLSID_WiaDevMgr"), hr );
    }

    //
    // Return the result of creating the device manager
    //
    return hr;
}

HRESULT ReadSomeWiaProperties( IWiaPropertyStorage *pWiaPropertyStorage )
{
    //
    // Validate arguments
    //
    if (NULL == pWiaPropertyStorage)
    {
        return E_INVALIDARG;
    }

    //
    // Declare PROPSPECs and PROPVARIANTs, and initialize them to zero.
    //
    PROPSPEC PropSpec[3] = {0};
    PROPVARIANT PropVar[3] = {0};

    //
    // How many properties are we querying for?
    //
    const ULONG c_nPropertyCount = sizeof(PropSpec)/sizeof(PropSpec[0]);

    //
    // Define which properties we want to read:
    // Device ID.  This is what we'd use to create
    // the device.
    //
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = WIA_DIP_DEV_ID;

    //
    // Device Name
    //
    PropSpec[1].ulKind = PRSPEC_PROPID;
    PropSpec[1].propid = WIA_DIP_DEV_NAME;

    //
    // Device description
    //
    PropSpec[2].ulKind = PRSPEC_PROPID;
    PropSpec[2].propid = WIA_DIP_DEV_DESC;

    //
    // Ask for the property values
    //
    HRESULT hr = pWiaPropertyStorage->ReadMultiple( c_nPropertyCount, PropSpec, PropVar );
    if (SUCCEEDED(hr))
    {
        //
        // IWiaPropertyStorage::ReadMultiple will return S_FALSE if some
        // properties could not be read, so we have to check the return
        // types for each requested item.
        //
        
        //
        // Check the return type for the device ID
        //
        if (VT_BSTR == PropVar[0].vt)
        {
            //
            // Do something with the device ID
            //
            _tprintf( TEXT("WIA_DIP_DEV_ID: %ws\n"), PropVar[0].bstrVal );
        }

        //
        // Check the return type for the device name
        //
        if (VT_BSTR == PropVar[1].vt)
        {
            //
            // Do something with the device name
            //
            _tprintf( TEXT("WIA_DIP_DEV_NAME: %ws\n"), PropVar[1].bstrVal );
        }

        //
        // Check the return type for the device description
        //
        if (VT_BSTR == PropVar[2].vt)
        {
            //
            // Do something with the device description
            //
            _tprintf( TEXT("WIA_DIP_DEV_DESC: %ws\n"), PropVar[2].bstrVal );
        }
        
        //
        // Free the returned PROPVARIANTs
        //
        FreePropVariantArray( c_nPropertyCount, PropVar );
    }
    else
    {
        ReportError( TEXT("Error calling IWiaPropertyStorage::ReadMultiple"), hr );
    }

    //
    // Return the result of reading the properties
    //
    return hr;
}

HRESULT EnumerateItems( IWiaItem *pWiaItem )
{
    //
    // Validate arguments
    //
    if (NULL == pWiaItem)
    {
        return E_INVALIDARG;
    }

    //
    // Get the item type for this item
    //
    LONG lItemType = 0;
    HRESULT hr = pWiaItem->GetItemType( &lItemType );

    //
    // Do something with this item.  Print the item name.
    //
    PrintItemName( pWiaItem );

    //
    // If this is an image, transfer it
    //
    if (lItemType & WiaItemTypeImage)
    {
        hr = TransferWiaItem( pWiaItem );
    }

    //
    // If it is a folder, or it has attachments, enumerate its children
    //
    if (lItemType & WiaItemTypeFolder ||
        lItemType & WiaItemTypeHasAttachments)
    {
        //
        // Get the child item enumerator for this item
        //    
        IEnumWiaItem *pEnumWiaItem = NULL;
        hr = pWiaItem->EnumChildItems( &pEnumWiaItem );
        if (SUCCEEDED(hr))
        {
            //
            // We will loop until we get an error or pEnumWiaItem->Next returns
            // S_FALSE to signal the end of the list.
            //
            while (S_OK == hr)
            {
                //
                // Get the next child item
                //
                IWiaItem *pChildWiaItem = NULL;
                hr = pEnumWiaItem->Next( 1, &pChildWiaItem, NULL );

                //
                // pEnumWiaItem->Next will return S_FALSE when the list is
                // exhausted, so check for S_OK before using the returned
                // value.
                //
                if (S_OK == hr)
                {
                    //
                    // Recurse into this item
                    //
                    hr = EnumerateItems( pChildWiaItem );

                    //
                    // Release this item
                    //
                    pChildWiaItem->Release();
                    pChildWiaItem = NULL;
                }
                else if (FAILED(hr))
                {
                    //
                    // Report that an error occurred during enumeration
                    //
                    ReportError( TEXT("Error calling pEnumWiaItem->Next"), hr );
                }
            }

            //
            // If the result of the enumeration is S_FALSE, since this
            // is normal, we will change it to S_OK
            //
            if (S_FALSE == hr)
            {
                hr = S_OK;
            }

            //
            // Release the enumerator
            //
            pEnumWiaItem->Release();
            pEnumWiaItem = NULL;
        }
    }
    return  hr;
}

HRESULT CreateWiaDeviceAndDoSomeStuff( IWiaDevMgr *pWiaDevMgr, IWiaPropertyStorage *pWiaPropertyStorage )
{
    //
    // Validate arguments
    //
    if (NULL == pWiaPropertyStorage)
    {
        return E_INVALIDARG;
    }

    //
    // Declare PROPSPECs and PROPVARIANTs, and initialize them to zero.
    //
    PROPSPEC PropSpec[1] = {0};
    PROPVARIANT PropVar[1] = {0};

    //
    // How many properties are we querying for?
    //
    const ULONG c_nPropertyCount = sizeof(PropSpec)/sizeof(PropSpec[0]);

    //
    // Define which properties we want to read.  We want the
    // device ID, so we can create the device.
    //
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = WIA_DIP_DEV_ID;

    //
    // Ask for the property values
    //
    HRESULT hr = pWiaPropertyStorage->ReadMultiple( c_nPropertyCount, PropSpec, PropVar );
    if (SUCCEEDED(hr))
    {
        //
        // Check the return type for the device ID to make
        // sure this property was actually read.
        //
        if (VT_BSTR == PropVar[0].vt)
        {
            //
            // Create the WIA device, which results in obtaining
            // the root IWiaItem
            //
            IWiaItem *pWiaRootItem = NULL;
            hr = CreateWiaDevice( pWiaDevMgr, PropVar[0].bstrVal, &pWiaRootItem );
            if (SUCCEEDED(hr))
            {
                //
                // Recursively enumerate the items
                // (This function will also do some stuff with each item)
                //
                hr = EnumerateItems( pWiaRootItem );

                //
                // Free the root item
                //
                pWiaRootItem->Release();
                pWiaRootItem = NULL;
            }
            else
            {
                ReportError( TEXT("Error calling CreateWiaDevice"), hr );
            }
        }

        //
        // Free the returned PROPVARIANTs
        //
        FreePropVariantArray( c_nPropertyCount, PropVar );
    }
    else
    {
        ReportError( TEXT("Error calling IWiaPropertyStorage::ReadMultiple"), hr );
    }

    //
    // Return the result of all of this
    //
    return hr;
}

HRESULT EnumerateWiaDevices( IWiaDevMgr *pWiaDevMgr )
{
    //
    // Validate arguments
    //
    if (NULL == pWiaDevMgr)
    {
        return E_INVALIDARG;
    }

    //
    // Get a device enumerator interface
    //
    IEnumWIA_DEV_INFO *pWiaEnumDevInfo = NULL;
    HRESULT hr = pWiaDevMgr->EnumDeviceInfo( WIA_DEVINFO_ENUM_LOCAL, &pWiaEnumDevInfo );
    if (SUCCEEDED(hr))
    {
        //
        // Reset the device enumerator to the beginning of the list
        //
        hr = pWiaEnumDevInfo->Reset();
        if (SUCCEEDED(hr))
        {
            //
            // We will loop until we get an error or pWiaEnumDevInfo->Next returns
            // S_FALSE to signal the end of the list.
            //
            while (S_OK == hr)
            {
                //
                // Get the next device's property storage interface pointer
                //
                IWiaPropertyStorage *pWiaPropertyStorage = NULL;
                hr = pWiaEnumDevInfo->Next( 1, &pWiaPropertyStorage, NULL );

                //
                // pWiaEnumDevInfo->Next will return S_FALSE when the list is
                // exhausted, so check for S_OK before using the returned
                // value.
                //
                if (hr == S_OK)
                {
                    //
                    // Do something with the device's IWiaPropertyStorage*
                    //
                    ReadSomeWiaProperties( pWiaPropertyStorage );

                    //
                    // Call a helper function to create the device
                    // and do some stuff with it.
                    //
                    CreateWiaDeviceAndDoSomeStuff( pWiaDevMgr, pWiaPropertyStorage );

                    //
                    // Release the device's IWiaPropertyStorage*
                    //
                    pWiaPropertyStorage->Release();
                    pWiaPropertyStorage = NULL;
                }
                else if (FAILED(hr))
                {
                    //
                    // Report that an error occurred during enumeration
                    //
                    ReportError( TEXT("Error calling pWiaEnumDevInfo->Next"), hr );
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
            //
            // Report that an error occurred calling Reset()
            //
            ReportError( TEXT("Error calling IEnumWIA_DEV_INFO::Reset()"), hr );
        }

        //
        // Release the enumerator
        //
        pWiaEnumDevInfo->Release();
        pWiaEnumDevInfo = NULL;
    }
    else
    {
        //
        // Report that an error occurred trying to create the enumerator
        //
        ReportError( TEXT("Error calling IWiaDevMgr::EnumDeviceInfo"), hr );
    }

    //
    // Return the result of the enumeration
    //
    return hr;
}

extern "C" 
int __cdecl _tmain( int, TCHAR *[] )
{
    //
    // Initialize COM
    //
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        //
        // Create the device manager
        //
        IWiaDevMgr *pWiaDevMgr = NULL;
        hr = CreateWiaDeviceManager( &pWiaDevMgr );
        if (SUCCEEDED(hr))
        {
            //
            // Enumerate all of the WIA devices
            //
            hr = EnumerateWiaDevices( pWiaDevMgr );
            if (FAILED(hr))
            {
                ReportError( TEXT("Error calling EnumerateWiaDevices"), hr );
            }

            //
            // Release the device manager
            //
            pWiaDevMgr->Release();
            pWiaDevMgr = NULL;
        }
        else
        {
            ReportError( TEXT("Error calling CreateWiaDeviceManager"), hr );
        }

        //
        // Uninitialize COM
        //
        CoUninitialize();
    }
    return 0;
}


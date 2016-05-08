/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#ifndef __WIAWRAP__
#define __WIAWRAP__

namespace WiaWrap
{

//////////////////////////////////////////////////////////////////////////
//
// ProgressCallback
//

/*++

    The ProgressCallback function is an application-defined callback function
    used with the WiaGetImage function.


    HRESULT 
    CALLBACK 
    ProgressCallback(
        LONG   lStatus,
        LONG   lPercentComplete,
        PVOID  pParam
    );


Parameters

    lStatus
        [in] Specifies a constant that indicates the status of the WIA device.
        Can be set to one of the following values;

            IT_STATUS_TRANSFER_FROM_DEVICE: Data is currently being 
            transferred from the WIA device. 

            IT_STATUS_PROCESSING_DATA: Data is currently being processed. 

            IT_STATUS_TRANSFER_TO_CLIENT: Data is currently being transferred 
            to the client's data buffer. 

    lPercentComplete
        [in] Specifies the percentage of the total data that has been 
        transferred so far.

    pParam
        [in] Specifies the application-defined value given in WiaGetImage


Return Values
    
    To continue the data transfer, return S_OK.

    To cancel the data transfer, return S_FALSE. 

    If the method fails, return a standard COM error code.

--*/

typedef HRESULT (CALLBACK *PFNPROGRESSCALLBACK)(
    LONG   lStatus,
    LONG   lPercentComplete,
    PVOID  pParam
);

//////////////////////////////////////////////////////////////////////////
//
// WiaGetImage
//

/*++

    The WiaGetImage function displays one or more dialog boxes that enable a 
    user to acquire multiple images from a WIA device and return them in an
    array of IStream interfaces. This method combines the functionality of 
    SelectDeviceDlg, DeviceDlg and idtGetBandedData API's to completely 
    encapsulate image acquisition within a single function call.


    HRESULT 
    WiaGetImage(
        HWND                 hWndParent,
        LONG                 lDeviceType,
        LONG                 lFlags,
        LONG                 lIntent,
        IWiaDevMgr          *pSuppliedWiaDevMgr,
        IWiaItem            *pSuppliedItemRoot,
        PFNPROGRESSCALLBACK  pfnProgressCallback,
        PVOID                pProgressCallbackParam,
        GUID                *pguidFormat,
        LONG                *plCount,
        IStream             ***pppStream
    );


Parameters

    hwndParent
        [in] Handle of the window that will own the dialog boxes.

    lDeviceType 
        [in] Specifies which type of WIA device to use. Can be set to one of 
        the following values;

            StiDeviceTypeDefault
            StiDeviceTypeScanner
            StiDeviceTypeDigitalCamera
            StiDeviceTypeStreamingVideo 

    lFlags 
        [in] Specifies dialog box behavior. Can be set to any combination of 
        the following values;

            0: Default behavior. 

            WIA_SELECT_DEVICE_NODEFAULT: Force this method to display the 
            Select Device dialog box. If this flag is not present, and if
            WiaGetImage finds only one matching device, it will not display 
            the Select Device dialog box. 

            WIA_DEVICE_DIALOG_SINGLE_IMAGE: Restrict image selection to a 
            single image in the device image acquisition dialog box.

            WIA_DEVICE_DIALOG_USE_COMMON_UI: Use the system UI, if available,
            rather than the vendor-supplied UI. If the system UI is not 
            available, the vendor UI is used. If neither UI is available, 
            the function returns E_NOTIMPL.

    lIntent 
        [in] Specifies what type of data the image is intended to represent. 

    pSuppliedWiaDevMgr
        [in] Pointer to the interface of the WIA device manager. This 
        interface is used when WiaGetImage displays the Select Device dialog 
        box. If the application passes NULL for this parameter, WiaGetImage 
        connects to the local WIA device manager.

    pSuppliedItemRoot 
        [in] Pointer to the interface of the hierarchical tree of IWiaItem 
        objects. If the application passes NULL for this parameter, 
        WiaGetImage displays the Select Device dialog box that lets the user 
        select the WIA input device. If the application specifies a WIA input 
        device by passing a valid pointer to the device's item tree for this 
        parameter, WiaGetImage does not display the Select Device dialog box, 
        instead, it will use the specified input device to acquire the image.

    pfnProgressCallback
        [in] Specifies the address of a callback function of type 
        PFNPROGRESSCALLBACK that is called periodically to provide data 
        transfer status notifications. If the application passes NULL
        for this parameter, WiaGetImage displays a simple progress dialog 
        with a status message, a progress bar and a cancel button. If the 
        application passes a valid function, WiaGetImage does not display
        this progress dialog, instead, it calls the specified function with
        the status message and the completion percentage values.

    pProgressCallbackParam
        [in] Specifies an argument to be passed to the callback function. 
        The value of the argument is specified by the application. This 
        parameter can be NULL. 

    pguidFormat 
        [in, out] On input, contains a pointer to a GUID that specifies the 
        format to use. On output, holds the format used. If the application
        passes GUID_NULL for this parameter, WiaGetImage uses the default 
        transfer format of the device and returns this format. If the 
        application passes NULL for this parameter, WiaGetImage uses the 
        default transfer format of the device but it does not return the 
        used format.

    plCount
        [out] Receives the number of items in the array indicated by the 
        pppStream parameter. 

    pppStream
        [out] Receives the address of an array of pointers to IStream 
        interfaces. Applications must call the IStream::Release method 
        for each element in the array of interface pointers they receive. 
        Applications must also free the pppStream array itself using 
        CoTaskMemFree.


Return Values
    
    WiaGetImage returns one of the following values or a standard COM error 
    if it fails for any other reason.

    S_OK: if the data is transferred successfully.

    S_FALSE: if the user cancels one of the device selection, image selection
    or image transfer dialog boxes.

    WIA_S_NO_DEVICE_AVAILABLE: if no WIA device is currently available.
    
    E_NOTIMPL: if no UI is available.

    
Remarks

    WiaGetImage returns the transferred images as stream objects in the 
    pppStream array parameter. The array is created with CoTaskMemAlloc 
    and the stream objects are created with CreateStreamOnHGlobal. The array 
    will contain a single entry if the WIA_DEVICE_DIALOG_SINGLE_IMAGE flag 
    is specified. Otherwise, it may contain one or more entries and the 
    count will be returned in the plCount parameter.

    The stream object will contain the image data. To create a GDI+ image 
    object from the stream object, use the Gdiplus::Image(IStream* stream)
    function. To obtain a pointer to the memory address of the data, use 
    the GetHGlobalFromStream API to obtain an HGLOBAL and use GlobalLock
    to obtain a direct pointer to the data.

--*/

HRESULT 
WiaGetImage(
    HWND                 hWndParent,
    LONG                 lDeviceType,
    LONG                 lFlags,
    LONG                 lIntent,
    IWiaDevMgr          *pSuppliedWiaDevMgr,
    IWiaItem            *pSuppliedItemRoot,
    PFNPROGRESSCALLBACK  pfnProgressCallback,
    PVOID                pProgressCallbackParam,
    GUID                *pguidFormat,
    LONG                *plCount,
    IStream             ***pppStream
);


//////////////////////////////////////////////////////////////////////////
//
// WiaGetNumDevices
//

/*++

    The WiaGetNumDevices function returns the number of WIA devices on
    the system.


    HRESULT 
    WiaGetNumDevices(
        IWiaDevMgr *pSuppliedWiaDevMgr,
        ULONG      *pulNumDevices
    );


Parameters

    pSuppliedWiaDevMgr
        [in] Pointer to the interface of the WIA device manager. If the 
        application passes NULL for this parameter, WiaGetNumDevices 
        connects to the local WIA device manager.

    pulNumDevices
        [out] Receives the number of WIA devices on the system.


Return Values
    
    WiaGetNumDevices returns S_OK on success or a standard COM error 
    if it fails for any reason.

--*/

HRESULT 
WiaGetNumDevices(
    IWiaDevMgr *pSuppliedWiaDevMgr,
    ULONG      *pulNumDevices
);


//////////////////////////////////////////////////////////////////////////
//
// ReadPropertyLong
//

/*++

    The ReadPropertyLong function reads a long integer value from a WIA 
    property storage


    HRESULT 
    ReadPropertyLong(
        IWiaPropertyStorage *pWiaPropertyStorage, 
        const PROPSPEC      *pPropSpec, 
        LONG                *plResult
    );

Parameters

    pWiaPropertyStorage
        [in] Pointer to the interface of the WIA property storage.

    PropSpec
        [in] Pointer to a PROPSPEC structure that specifies which 
        property is to be read. 

    plResult
        [out] Receives the value of the property specified by PropSpec


Return Values
    
    ReadPropertyLong returns S_OK on success or a standard COM error 
    if it fails for any reason.

--*/

HRESULT 
ReadPropertyLong(
    IWiaPropertyStorage *pWiaPropertyStorage, 
    const PROPSPEC      *pPropSpec, 
    LONG                *plResult
);


//////////////////////////////////////////////////////////////////////////
//
// ReadPropertyGuid
//

/*++

    The ReadPropertyGuid function reads a GUID value from a WIA property
    storage


    HRESULT 
    ReadPropertyGuid(
        IWiaPropertyStorage *pWiaPropertyStorage, 
        const PROPSPEC      *pPropSpec, 
        GUID                *pguidResult
    );


Parameters

    pWiaPropertyStorage
        [in] Pointer to the interface of the WIA property storage.

    pPropSpec
        [in] Pointer to a PROPSPEC structure that specifies which 
        property is to be read. 

    plResult
        [out] Receives the value of the property specified by PropSpec


Return Values
    
    ReadPropertyGuid returns S_OK on success or a standard COM error 
    if it fails for any reason.

--*/

HRESULT 
ReadPropertyGuid(
    IWiaPropertyStorage *pWiaPropertyStorage, 
    const PROPSPEC      *pPropSpec, 
    GUID                *pguidResult
);


//////////////////////////////////////////////////////////////////////////
//
// CComPtrArray
//

/*++

    CComPtrArray stores an array of COM interface pointers and performs
    reference counting through AddRef and Release methods. 
    
    CComPtrArray can be used with WiaGetImage and DeviceDlg functions 
    to provide automatic deallocation of the output arrays.

Methods

    CComPtrArray()
        Initializes the array pointer and the count to zero.

    CComPtrArray(int nCount)
        Allocates the array for with CoTaskMemAlloc for nCount items and 
        initializes the interface pointers to NULL

    Copy(const CComPtrArray& rhs)
        Allocates a new array with CoTaskMemAlloc, copies the interface 
        pointers and call AddRef() on the copied pointers.

  	Clear()
        Calls Release on the interface pointers in the array and
        deallocates the array with CoTaskMemFree

    CComPtrArray(const CComPtrArray& rhs)
        Calls the Copy method to copy the new contents.

    CComPtrArray &operator =(const CComPtrArray& rhs)
        Calls the Clear method to delete the current contents and calls 
        the Copy method to copy the new contents.

    ~CComPtrArray()
        Destructor, calls the Clear method

    operator T **()
        Returns the dereferenced value of the member pointer.

    bool operator!()
        Returns TRUE or FALSE, depending on whether the member pointer is 
        NULL or not.

    T ***operator&()
        Returns the address of the member pointer.

    LONG &Count()
        Returns a reference to the count.

--*/

template <class T>
class CComPtrArray
{
public:
    CComPtrArray()
    {
        m_pArray = NULL;
        m_nCount = 0;
    }

    explicit CComPtrArray(int nCount)
    {
        m_pArray = (T **) CoTaskMemAlloc(nCount * sizeof(T *));

        m_nCount = m_pArray == NULL ? 0 : nCount;

        for (int i = 0; i < m_nCount; ++i) 
        {
            m_pArray[i] = NULL;
        }
    }

    CComPtrArray(const CComPtrArray& rhs)
    {
        Copy(rhs);
    }

    ~CComPtrArray() 
    {
        Clear();
    }

    CComPtrArray &operator =(const CComPtrArray& rhs)
    {
        if (this != &rhs)
        {
            Clear();
            Copy(rhs);
        }

        return *this;
    }

    operator T **()
    {
        return m_pArray;
    }

    bool operator!()
    {
        return m_pArray == NULL;
    }

    T ***operator&()
    {
        return &m_pArray;
    }

    LONG &Count()
    {
        return m_nCount;
    }

	void Clear()
	{
        if (m_pArray != NULL) 
        {
            for (int i = 0; i < m_nCount; ++i) 
            {
                if (m_pArray[i] != NULL) 
                {
                    m_pArray[i]->Release();
                }
            }

            CoTaskMemFree(m_pArray);

            m_pArray = NULL;
            m_nCount = 0;
        }
	}

    void Copy(const CComPtrArray& rhs)
    {
        m_pArray = NULL;
        m_nCount = 0;

        if (rhs.m_pArray != NULL)
        {
            m_pArray = (T**) CoTaskMemAlloc(rhs.m_nCount * sizeof(T *));

            if (m_pArray != NULL)
            {
                m_nCount = rhs.m_nCount;

                for (int i = 0; i < m_nCount; ++i)
                {
                    m_pArray[i] = rhs.m_pArray[i];

                    if (m_pArray[i] != NULL)
                    {
                        m_pArray[i]->AddRef();
                    }
                }
            }
        }
    }

private:
    T    **m_pArray;
    LONG  m_nCount;
};


}; // namespace WiaWrap

#endif //__WIAWRAP__


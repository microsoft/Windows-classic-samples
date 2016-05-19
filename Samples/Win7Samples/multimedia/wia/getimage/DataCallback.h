/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#ifndef __DATACALLBACK__
#define __DATACALLBACK__

#include "WiaWrap.h"

namespace WiaWrap
{

//////////////////////////////////////////////////////////////////////////
//
// CDataCallback
//

/*++

    CDataCallback implements a WIA data callback object that stores the
    transferred data in an array of stream objects.

Methods

    CDataCallback
        Initializes the object

    BandedDataCallback
        This method is called periodically during data transfers. It can be
        called for one of these reasons;

        IT_MSG_DATA_HEADER: The method tries to allocate memory for the 
        image if the size is given in the header

        IT_MSG_DATA: The method adjusts the stream size and copies the new 
        data to the stream.

        case IT_MSG_STATUS: The method invoke the progress callback function.

        IT_MSG_TERMINATION or IT_MSG_NEW_PAGE: For BMP images, calculates the 
        image height if it is not given in the image header, fills in the 
        BITMAPFILEHEADER and stores this stream in the successfully 
        transferred images array

    ReAllocBuffer
        Increases the size of the current stream object, or creates a 
        new stream object if this is the first call.

    CopyToBuffer
        Copies new data to the current stream.

    StoreBuffer
        Stores the current stream in the array of successfully transferred
        images.

--*/

class CDataCallback : public IWiaDataCallback
{
public:
    CDataCallback(
        PFNPROGRESSCALLBACK  pfnProgressCallback,
        PVOID                pProgressCallbackParam,
        LONG                *plCount,
        IStream             ***pppStream
    );

    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IWiaDataCallback interface

    STDMETHOD(BandedDataCallback) (
        LONG  lReason,
        LONG  lStatus,
        LONG  lPercentComplete,
        LONG  lOffset,
        LONG  lLength,
        LONG  lReserved,
        LONG  lResLength,
        PBYTE pbBuffer
    );

    // CDataCallback methods

private:
    HRESULT ReAllocBuffer(ULONG nBufferSize);
    HRESULT CopyToBuffer(ULONG nOffset, LPCVOID pBuffer, ULONG nSize);
    HRESULT StoreBuffer();

private:
    LONG m_cRef;

	BOOL              m_bBMP;
    LONG              m_nHeaderSize;
    LONG              m_nDataSize;
    CComPtr<IStream>  m_pStream;

    PFNPROGRESSCALLBACK  m_pfnProgressCallback;
    PVOID                m_pProgressCallbackParam;

    LONG    *m_plCount;
    IStream ***m_pppStream;
};

}; // namespace WiaWrap

#endif //__DATACALLBACK__

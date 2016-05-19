/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#include "stdafx.h"
#include "resource.h"

#include "BitmapUtil.h"
#include "DataCallback.h"

namespace WiaWrap
{

//////////////////////////////////////////////////////////////////////////
//
// CDataCallback::CDataCallback
//

CDataCallback::CDataCallback(
    PFNPROGRESSCALLBACK  pfnProgressCallback,
    PVOID                pProgressCallbackParam,
    LONG                *plCount,
    IStream             ***pppStream
)
{
    m_cRef = 0;

    m_bBMP        = FALSE;
    m_nHeaderSize = 0;
    m_nDataSize   = 0;

    m_pfnProgressCallback    = pfnProgressCallback;
    m_pProgressCallbackParam = pProgressCallbackParam;

    m_plCount   = plCount;
    m_pppStream = pppStream;
}

//////////////////////////////////////////////////////////////////////////
//
// CDataCallback::QueryInterface
//

STDMETHODIMP CDataCallback::QueryInterface(REFIID iid, LPVOID *ppvObj)
{
    if (ppvObj == NULL)
    {
        return E_POINTER;
    }

    if (iid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*) this;
    }
    else if (iid == IID_IWiaDataCallback)
    {
        *ppvObj = (IWiaDataCallback *) this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// CDataCallback::AddRef
//

STDMETHODIMP_(ULONG) CDataCallback::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

//////////////////////////////////////////////////////////////////////////
//
// CDataCallback::Release
//

STDMETHODIMP_(ULONG) CDataCallback::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

//////////////////////////////////////////////////////////////////////////
//
// CDataCallback::BandedDataCallback
//

STDMETHODIMP CDataCallback::BandedDataCallback(
    LONG   lReason,
    LONG   lStatus,
    LONG   lPercentComplete,
    LONG   lOffset,
    LONG   lLength,
    LONG   lReserved,
    LONG   lResLength,
    PBYTE  pbBuffer
)
{
    HRESULT hr;

    // Parse the message

    switch (lReason)
    {
        case IT_MSG_DATA_HEADER:
        {
            PWIA_DATA_CALLBACK_HEADER pHeader = (PWIA_DATA_CALLBACK_HEADER) pbBuffer;

            // Determine if this is a BMP transfer

			m_bBMP = pHeader->guidFormatID == WiaImgFmt_MEMORYBMP || pHeader->guidFormatID == WiaImgFmt_BMP;

            // For WiaImgFmt_MEMORYBMP transfers, WIA does not send a BITMAPFILEHEADER before the data.
            // In this program, we desire all BMP files to contain a BITMAPFILEHEADER, so add it manually

            m_nHeaderSize = pHeader->guidFormatID == WiaImgFmt_MEMORYBMP ? sizeof(BITMAPFILEHEADER) : 0;

            // Allocate memory for the image if the size is given in the header

            if (pHeader != NULL && pHeader->lBufferSize != 0)
            {
                hr = ReAllocBuffer(m_nHeaderSize + pHeader->lBufferSize);

                if (FAILED(hr))
                {
                    return hr;
                }
            }
        
            break;
        }

        case IT_MSG_DATA:
        {
            // Invoke the callback function

            hr = m_pfnProgressCallback(lStatus, lPercentComplete, m_pProgressCallbackParam);

            if (FAILED(hr) || hr == S_FALSE)
            {
                return hr;
            }

            // If the buffer is not allocated yet and this is the first block, 
            // and the transferred image is in BMP format, allocate the buffer
            // according to the size information in the bitmap header

            if (m_pStream == NULL && lOffset == 0 && m_bBMP)
            {
                LONG nBufferSize = BitmapUtil::GetBitmapSize(pbBuffer);

                if (nBufferSize != 0)
                {
                    hr = ReAllocBuffer(m_nHeaderSize + nBufferSize);

                    if (FAILED(hr))
                    {
                        return hr;
                    }
                }
            }

            // If the transfer goes past the buffer, try to expand it

            if (m_nHeaderSize + lOffset + lLength > m_nDataSize)
            {
                hr = ReAllocBuffer(m_nHeaderSize + lOffset + lLength);

                if (FAILED(hr))
                {
                    return hr;
                }
            }

            // copy the transfer buffer

            hr = CopyToBuffer(m_nHeaderSize + lOffset, pbBuffer, lLength);

            if (FAILED(hr))
            {
                return hr;
            }

            break;
        }

        case IT_MSG_STATUS:
        {
            // Invoke the callback function

            hr = m_pfnProgressCallback(lStatus, lPercentComplete, m_pProgressCallbackParam);

            if (FAILED(hr) || hr == S_FALSE)
            {
                return hr;
            }

            break;
        }

        case IT_MSG_TERMINATION:
        case IT_MSG_NEW_PAGE:
        {
            if (m_pStream != NULL)
            {
                // For BMP files, we should validate the the image header
                // So, obtain the memory buffer from the stream

                if (m_bBMP)
                {
                    // Since the stream is created using CreateStreamOnHGlobal,
                    // we can get the memory buffer with GetHGlobalFromStream.

                    HGLOBAL hBuffer;

                    hr = GetHGlobalFromStream(m_pStream, &hBuffer);

                    if (FAILED(hr))
                    {
                        return hr;
                    }

                    PBITMAPFILEHEADER pBuffer = (PBITMAPFILEHEADER) GlobalLock(hBuffer);

                    if (pBuffer == NULL)
                    {
                        return HRESULT_FROM_WIN32(GetLastError());
                    }

                    // Some scroll-fed scanners may return 0 as the bitmap height
                    // In this case, calculate the image height and modify the header

                    BitmapUtil::FixBitmapHeight(pBuffer + 1, m_nDataSize, TRUE);

                    // For WiaImgFmt_MEMORYBMP transfers, the WIA service does not 
                    // include a BITMAPFILEHEADER preceeding the bitmap data. 
                    // In this case, fill in the BITMAPFILEHEADER structure.
        
                    if (m_nHeaderSize != 0)
                    {
                        BitmapUtil::FillBitmapFileHeader(pBuffer + 1, pBuffer);
                    }

                    GlobalUnlock(hBuffer);
                }

                // Store this buffer in the successfully transferred images array

				hr = StoreBuffer();
                
                if (FAILED(hr))
                {
                    return hr;
                }
            }

            break;
        }
    }

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// CDataCallback::ReAllocBuffer
//

HRESULT CDataCallback::ReAllocBuffer(ULONG nSize)
{
    HRESULT hr;

    // If m_pStream is not initialized yet, create a new stream object

    if (m_pStream == 0)
    {
        hr = CreateStreamOnHGlobal(0, TRUE, &m_pStream);

        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Next, set the size of the stream object

    ULARGE_INTEGER liSize = { nSize };

    hr = m_pStream->SetSize(liSize);

    if (FAILED(hr))
    {
        return hr;
    }

    m_nDataSize = nSize;

    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// CDataCallback::CopyToBuffer
//

HRESULT CDataCallback::CopyToBuffer(ULONG nOffset, LPCVOID pBuffer, ULONG nSize)
{
    HRESULT hr;

    // First move the stream pointer to the data offset

    LARGE_INTEGER liOffset = { nOffset };

    hr = m_pStream->Seek(liOffset, STREAM_SEEK_SET, 0);

    if (FAILED(hr))
    {
        return hr;
    }

    // Next, write the new data to the stream

    hr = m_pStream->Write(pBuffer, nSize, 0);

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}


//////////////////////////////////////////////////////////////////////////
//
// CDataCallback::StoreBuffer
//

HRESULT CDataCallback::StoreBuffer()
{
    // Increase the successfully transferred buffers array size

    IStream **ppStream = (IStream **) CoTaskMemRealloc(
		*m_pppStream, 
		(*m_plCount + 1) * sizeof(IStream *)
	);

    if (ppStream == NULL)
    {
        return E_OUTOFMEMORY;
    }

    *m_pppStream = ppStream;

    // Rewind the current buffer

    LARGE_INTEGER liZero = { 0 };

    m_pStream->Seek(liZero, STREAM_SEEK_SET, 0);

    // Store the current buffer as the last successfully transferred buffer

	(*m_pppStream)[*m_plCount] = m_pStream;
    (*m_pppStream)[*m_plCount]->AddRef();

    *m_plCount += 1;

    // Reset the current buffer

    m_pStream.Release();

    m_nDataSize = 0;

    return S_OK;
}

}; // namespace WiaWrap

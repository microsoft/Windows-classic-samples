/*--

Copyright (C) Microsoft Corporation, 2006

Implementation of CDiscInformation

--*/

#include "stdafx.h"

CDiscInformation::CDiscInformation() :
    m_DiscInfo(NULL),
    m_DiscInfoSize(0),
    m_DiscInfoAvailableSize(0)
{
    return;
}

CDiscInformation::~CDiscInformation()
{
    LocalFreeAndNull(m_DiscInfo); 
    m_DiscInfoSize = 0; 
    m_DiscInfoAvailableSize = 0;
    return;
}

HRESULT CDiscInformation::Init(__in IDiscRecorder2Ex* recorder, __in BOOLEAN reinit)
{
    HRESULT hr = S_OK;
    BYTE*   data = NULL;
    ULONG   dataSize = 0;

    // read disc info
    if (SUCCEEDED(hr))
    {
        hr = recorder->GetDiscInformation(&data, &dataSize);
    }
    // validate sizes
    if (SUCCEEDED(hr))
    {
        hr = ValidateInitData(data, dataSize);
    }
    // initialize the structure
    if (SUCCEEDED(hr))
    {
        hr = this->Init(data, dataSize, reinit);
    }
    // cleanup and return
    CoTaskMemFreeAndNull(data);

    return hr;
}

HRESULT CDiscInformation::Init(__in_bcount(BufferSize) BYTE* Buffer, __in LONG BufferSize, __in BOOLEAN Reuse)
{
    HRESULT hr = S_OK;

    if ((!Reuse) && (m_DiscInfo != NULL))
    {
        hr = E_INVALIDARG;
    }

    // make a copy of buffer (store locally)
    if (SUCCEEDED(hr))
    {
        // if buffer exists already and is large enough, just copy over and update size
        if ((m_DiscInfo != NULL) && (m_DiscInfoAvailableSize >= (ULONG)BufferSize))
        {
            m_DiscInfoSize = BufferSize;
            RtlCopyMemory(m_DiscInfo, Buffer, BufferSize);
        }
        else // allocate a new buffer and copy over the data
        {
            DISC_INFORMATION* tmpDiscInfo = (DISC_INFORMATION*)LocalAlloc(LPTR, BufferSize);
            if (tmpDiscInfo == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                RtlCopyMemory(tmpDiscInfo, Buffer, BufferSize);
                LocalFreeAndNull(m_DiscInfo);
                m_DiscInfo              = tmpDiscInfo;
                m_DiscInfoSize          = BufferSize;
                m_DiscInfoAvailableSize = BufferSize;
            }
        }
    }

    return hr;
}

HRESULT CDiscInformation::ValidateInitData(__in_bcount(BufferSize) BYTE* Buffer, __in LONG BufferSize)
{
    HRESULT hr = S_OK;

    // validate arguments
    if (Buffer == NULL)
    {
        hr = E_IMAPI_RECORDER_INVALID_RESPONSE_FROM_DEVICE;
    }
    else if (BufferSize < 2)
    {
        hr = E_IMAPI_RECORDER_INVALID_RESPONSE_FROM_DEVICE;
    }
    else if (BufferSize < DISC_INFORMATION_MINIMUM_SIZE)
    {
        hr = E_IMAPI_RECORDER_INVALID_RESPONSE_FROM_DEVICE;
    }

    if (SUCCEEDED(hr))
    {
        ULONG realBufferSize =
                            (Buffer[0] << (8*1)) |
                            (Buffer[1] << (8*0)) ;
        realBufferSize += 2; // sizeof ( Buffer[0..1] )
        if ((ULONG)BufferSize != realBufferSize)
        {
            hr = E_IMAPI_RECORDER_INVALID_RESPONSE_FROM_DEVICE;
        }
    }

    return hr;
}

ULONG CDiscInformation::get_DiscStatus()
{
    return m_DiscInfo->DiscStatus;
}

BOOLEAN CDiscInformation::get_Erasable()
{
    return (m_DiscInfo->Erasable != 0);
}


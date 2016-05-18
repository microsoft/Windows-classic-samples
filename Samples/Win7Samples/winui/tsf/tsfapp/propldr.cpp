/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation. All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          PropLdr.cpp

   Description:   CTSFPersistentPropertyLoader implementation.

**************************************************************************/

/**************************************************************************
	#include statements
**************************************************************************/

#include "PropLdr.h"

/**************************************************************************

	CTSFPersistentPropertyLoader::CTSFPersistentPropertyLoader()

**************************************************************************/

CTSFPersistentPropertyLoader::CTSFPersistentPropertyLoader(TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream *pStream)
{
    m_ObjRefCount = 1;
    m_hdr = *pHdr;
    m_pb = (BYTE *)GlobalAlloc(GPTR, pHdr->cb);
    if(m_pb)
    {
        pStream->Read(m_pb, pHdr->cb, NULL);
    }
}

/**************************************************************************

	CTSFPersistentPropertyLoader::~CTSFPersistentPropertyLoader()

**************************************************************************/

CTSFPersistentPropertyLoader::~CTSFPersistentPropertyLoader()
{
    if(m_pb)
    {
        GlobalFree(m_pb);
    }
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

/**************************************************************************

	CTSFPersistentPropertyLoader::QueryInterface()

**************************************************************************/

STDAPI CTSFPersistentPropertyLoader::QueryInterface(REFIID riid, void **ppvObj)
{
    if(ppvObj == NULL)
    {
        return E_POINTER;
    }

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfPersistentPropertyLoaderACP))
    {
        *ppvObj = (ITfPersistentPropertyLoaderACP*)this;
    }

    if (*ppvObj == NULL)
    {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

/**************************************************************************

	CTSFPersistentPropertyLoader::AddRef()

**************************************************************************/

STDAPI_(ULONG) CTSFPersistentPropertyLoader::AddRef()
{
    m_ObjRefCount++;
    return m_ObjRefCount;
}

/**************************************************************************

	CTSFPersistentPropertyLoader::Release()

**************************************************************************/

STDAPI_(ULONG) CTSFPersistentPropertyLoader::Release()
{
    if(--m_ObjRefCount == 0)
    {
        delete this;
        return 0;
    }
   
    return m_ObjRefCount;
}


///////////////////////////////////////////////////////////////////////////
//
// ITfPersistentPropertyLoaderACP Implementation
//
// we count on the given TF_PERSISTENT_PROPERTY_HEADER_ACP structure for the place
// to be loaded.

/**************************************************************************

	CTSFPersistentPropertyLoader::LoadProperty()

**************************************************************************/

STDMETHODIMP CTSFPersistentPropertyLoader::LoadProperty(const TF_PERSISTENT_PROPERTY_HEADER_ACP *pHdr, IStream **ppStream)
{
    HRESULT hr;
    IStream *pStream;

    *ppStream = NULL;

    if (m_pb == NULL)
        return E_FAIL; // failed initial mem alloc

    //create a stream to return
    hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    if(SUCCEEDED(hr))
    {
        //write the property data into the stream
        hr = pStream->Write(m_pb, m_hdr.cb, NULL);
        if(SUCCEEDED(hr))
        {
            LARGE_INTEGER   li;

            li.QuadPart = 0;
            hr = pStream->Seek(li, STREAM_SEEK_SET, NULL);
            if(SUCCEEDED(hr))
            {
                *ppStream = pStream;
            }
        }
    }

    return hr;
}


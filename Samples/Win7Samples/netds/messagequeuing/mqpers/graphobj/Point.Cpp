// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------

// Point.cpp : Implementation of CPoint
#include "stdafx.h"
#include "GraphObj.h"
#include "Point.h"

/////////////////////////////////////////////////////////////////////////////
// CPoint

STDMETHODIMP CPoint::IMyPersistStreamImpl_Load(LPSTREAM pIStream)
{
    ULONG           cb;
    HRESULT         hr;

    if (NULL==pIStream)
        return ResultFromScode(E_POINTER);
    //
    // Read an object from the stream.
    //
    hr=pIStream->Read(&m_Point, sizeof(POINT), &cb);
    if (FAILED(hr))
        return hr;
    if (sizeof(POINT) != cb)
        return E_FAIL;

	return NOERROR;
}



STDMETHODIMP CPoint::IMyPersistStreamImpl_Save(LPSTREAM pIStream, BOOL /*fClearDirty */)
{
    ULONG           cb;
    HRESULT         hr;

    if (NULL==pIStream)
        return ResultFromScode(E_POINTER);

    //
    // Write the object into the stream.
    //
    hr=pIStream->Write(&m_Point, (ULONG)sizeof(POINT), &cb);

    if (FAILED(hr) || sizeof(POINT)!=cb)
       return ResultFromScode(STG_E_WRITEFAULT);

	return NOERROR;
}


STDMETHODIMP CPoint::IMyPersistStreamImpl_GetSizeMax(ULARGE_INTEGER FAR* pcbSize)
{
	pcbSize->LowPart = (DWORD)(sizeof(POINT));
	pcbSize->HighPart = 0L;
	return NOERROR;
}


STDMETHODIMP CPoint::get_x(long * pVal)
{
	*pVal = m_Point.x;
	return S_OK;
}

STDMETHODIMP CPoint::put_x(long newVal)
{
	m_Point.x = newVal;
	return S_OK;
}

STDMETHODIMP CPoint::get_y(long * pVal)
{
	*pVal = m_Point.y;
	return S_OK;
}

STDMETHODIMP CPoint::put_y(long newVal)
{
	m_Point.y = newVal;
	return S_OK;
}

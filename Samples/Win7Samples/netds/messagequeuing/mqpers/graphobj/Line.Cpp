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

// Line.cpp : Implementation of CLine
#include "stdafx.h"
#include "GraphObj.h"
#include "Line.h"

/////////////////////////////////////////////////////////////////////////////
// CLine

STDMETHODIMP CLine::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ILine,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if ( ::InlineIsEqualGUID(*arr[i], riid) )

			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CLine::get_x1(long * pVal)
{
	*pVal = m_Line.x1;
	return S_OK;
}

STDMETHODIMP CLine::put_x1(long newVal)
{
	m_Line.x1 = newVal;

	return S_OK;
}

STDMETHODIMP CLine::get_y1(long * pVal)
{
	*pVal = m_Line.y1;

	return S_OK;
}

STDMETHODIMP CLine::put_y1(long newVal)
{
	m_Line.y1 = newVal;

	return S_OK;
}

STDMETHODIMP CLine::get_x2(long * pVal)
{
	*pVal = m_Line.x2;

	return S_OK;
}

STDMETHODIMP CLine::put_x2(long newVal)
{
	m_Line.x2 = newVal;

	return S_OK;
}

STDMETHODIMP CLine::get_y2(long * pVal)
{
	*pVal = m_Line.y2;

	return S_OK;
}

STDMETHODIMP CLine::put_y2(long newVal)
{
	m_Line.y2 = newVal;

	return S_OK;
}

STDMETHODIMP CLine::IMyPersistStreamImpl_Load(LPSTREAM pIStream)
{
    ULONG           cb;
    HRESULT         hr;

    if (NULL==pIStream)
        return ResultFromScode(E_POINTER);
    //
    // Read an object from the stream.
    //
    hr=pIStream->Read(&m_Line, sizeof(LINEDATA), &cb);
    if (FAILED(hr))
        return hr;
    if (sizeof(LINEDATA) != cb)
        return E_FAIL;

	return NOERROR;
}



STDMETHODIMP CLine::IMyPersistStreamImpl_Save(LPSTREAM pIStream, BOOL /* fClearDirty */)
{
    ULONG           cb;
    HRESULT         hr;

    if (NULL==pIStream)
        return ResultFromScode(E_POINTER);

    //
    // Write an object into the stream.
    //
    hr=pIStream->Write(&m_Line, (ULONG)sizeof(LINEDATA), &cb);

    if (FAILED(hr) || sizeof(LINEDATA)!=cb)
       return ResultFromScode(STG_E_WRITEFAULT);

	return NOERROR;
}


STDMETHODIMP CLine::IMyPersistStreamImpl_GetSizeMax(ULARGE_INTEGER FAR* pcbSize)
{
	pcbSize->LowPart = (DWORD)(sizeof(LINEDATA));
	pcbSize->HighPart = 0L;
	return NOERROR;
}


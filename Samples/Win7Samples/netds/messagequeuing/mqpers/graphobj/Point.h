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

// Point.h : Declaration of the CPoint

#ifndef __POINT_H_
#define __POINT_H_

#include "resource.h"       // main symbols
#include "IPersistStm.h"

/////////////////////////////////////////////////////////////////////////////
// CPoint
//
// This class implements a persistable point object.
//

class ATL_NO_VTABLE CPoint : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPoint, &CLSID_PointGO>,
	public IMyPersistStreamImpl<CPoint>,
	public IDispatchImpl<IPoint, &IID_IPoint, &LIBID_GRAPHOBJLib>
{
private:
	typedef struct {
		long x, y;
	} POINT;

	POINT	m_Point;

public:
	CPoint()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_POINT)
DECLARE_NOT_AGGREGATABLE(CPoint)

BEGIN_COM_MAP(CPoint)
	COM_INTERFACE_ENTRY(IPoint)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IPersistStream, IMyPersistStream)
END_COM_MAP()


// IMyPersistStream
	BOOL	m_bRequiresSave;
	STDMETHOD(IMyPersistStreamImpl_Load)(LPSTREAM pStm);
	STDMETHOD(IMyPersistStreamImpl_Save)(LPSTREAM pStm, BOOL fClearDirty);
	STDMETHOD(IMyPersistStreamImpl_GetSizeMax)(ULARGE_INTEGER FAR* pcbSize);


// IPoint
public:
	STDMETHOD(get_y)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_y)(/*[in]*/ long newVal);
	STDMETHOD(get_x)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_x)(/*[in]*/ long newVal);
};

#endif //__POINT_H_

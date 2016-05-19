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

// Line.h : Declaration of the CLine

#ifndef __LINE_H_
#define __LINE_H_

#include "resource.h"       // main symbols
#define _ATL_DEBUG_QI
#define _ATL_DEBUG_REFCOUNT
#include <atlcom.h>
#include <atlctl.h>
#include <olectl.h>
#include "IPersistStm.h"



/////////////////////////////////////////////////////////////////////////////
// CLine
//
// This class implements a persistable line object.
//


class /* ATL_NO_VTABLE */ CLine : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CLine, &CLSID_Line>,
	public IMyPersistStreamImpl<CLine>,
	public ISupportErrorInfo,
	public IDispatchImpl<ILine, &IID_ILine, &LIBID_GRAPHOBJLib>
{
private:
	typedef struct {
		long x1, y1;
		long x2, y2;
	} LINEDATA;

	LINEDATA m_Line;

public:
	CLine()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_LINE)
DECLARE_NOT_AGGREGATABLE(CLine)

BEGIN_COM_MAP(CLine)
	COM_INTERFACE_ENTRY(ILine)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IPersistStream, IMyPersistStream)
	// COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
END_COM_MAP()

// IMyPersistStream
	BOOL	m_bRequiresSave;
	STDMETHOD(IMyPersistStreamImpl_Load)(LPSTREAM pStm);
	STDMETHOD(IMyPersistStreamImpl_Save)(LPSTREAM pStm, BOOL fClearDirty);
	STDMETHOD(IMyPersistStreamImpl_GetSizeMax)(ULARGE_INTEGER FAR* pcbSize);
	
	// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// ILine
public:
	STDMETHOD(get_y2)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_y2)(/*[in]*/ long newVal);
	STDMETHOD(get_x2)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_x2)(/*[in]*/ long newVal);
	STDMETHOD(get_y1)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_y1)(/*[in]*/ long newVal);
	STDMETHOD(get_x1)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_x1)(/*[in]*/ long newVal);
};                          

#endif //__LINE_H_

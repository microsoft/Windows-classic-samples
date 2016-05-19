// Simple.h : Declaration of the CSimple


#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CATLSmpl

class CSimple : 
    public CComDualImpl<ISimple, &IID_ISimple, &LIBID_CATLSmpl>, 
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CSimple,&CLSID_CSimple>
{
public:
    CSimple();
BEGIN_COM_MAP(CSimple)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISimple)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()
//DECLARE_NOT_AGGREGATABLE(CSimple) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation.  The default is to support it

DECLARE_REGISTRY(CSimple, _T("IISSample.C++ATLSimple.1"), _T("IISSample.C++ATLSimple"), IDS_SIMPLE_DESC, THREADFLAGS_BOTH)
// ISupportsErrorInfo
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// ISimple
public:
	// for free-threaded marshalling
DECLARE_GET_CONTROLLING_UNKNOWN()
	HRESULT FinalCountruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p );
	}
	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

    STDMETHOD(get_myProperty)(BSTR* pbstrOutValue);
    STDMETHOD(put_myProperty)(BSTR bstrInValue);
    STDMETHOD(myMethod) (BSTR bstrIn, BSTR* pbstrOut);

private:
    CComBSTR			m_bstrMyProperty;
	CComPtr<IUnknown>	m_pUnkMarshaler;
};

// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OnAsync.h
//
// Description:
//	This file declares the CAsyncQuerySink class. It is the sink
//		object for ExecQueryAsync() calls.
// 
// Part of: WMI Tutorial #1.
//
// Used by: OnAsync().
//
// History:
//
// **************************************************************************

class CAsyncQuerySink : public IWbemObjectSink
{
public:
    CAsyncQuerySink(CListBox *output);
    ~CAsyncQuerySink();

    STDMETHOD_(SCODE, Indicate)(long lObjectCount,
								IWbemClassObject **pObjArray);

    STDMETHOD_(SCODE, SetStatus)(long lFlags,
									HRESULT hResult,
									BSTR strParam,
									IWbemClassObject *pObjParam);

    // IUnknown members
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    /* IDispatch methods */
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
		{return E_NOTIMPL;}
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
		{return E_NOTIMPL;}
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR** rgszNames, UINT cNames,
      LCID lcid, DISPID* rgdispid)
		{return E_NOTIMPL;}
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
      DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
      UINT* puArgErr)
		{return E_NOTIMPL;}

private:
    long m_lRef;
	CListBox *m_pOutputList;
	BOOL m_SetStatusCalled;
};

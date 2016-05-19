//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CLISTENER.CPP
//
//-----------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Includes
//												   
//////////////////////////////////////////////////////////////////////////////
#include "Headers.h"


//////////////////////////////////////////////////////////////////
// CListener
//
//////////////////////////////////////////////////////////////////
CListener::CListener()
{
	m_cRef = 1;
	m_hrReturn = S_OK;
}

//////////////////////////////////////////////////////////////////
// ~CListener
//
//////////////////////////////////////////////////////////////////
CListener::~CListener() 
{
}

//////////////////////////////////////////////////////////////////
// CListener::SetReturnValue
//
//////////////////////////////////////////////////////////////////
HRESULT CListener::SetReturnValue(HRESULT hrReturn)
{
	m_hrReturn = hrReturn;
	return S_OK;
}

//////////////////////////////////////////////////////////////////
// CListener::GetReturnValue
//
//////////////////////////////////////////////////////////////////
HRESULT CListener::GetReturnValue()
{
	return m_hrReturn;
}

//////////////////////////////////////////////////////////////////
// CListener::Advise
//
//////////////////////////////////////////////////////////////////
HRESULT CListener::Advise(CContainerBase* pCPoint, REFIID riid, DWORD* pdwCookie)
{
	HRESULT hr = S_OK;
	ASSERT(pCPoint);
	CComPtr<IConnectionPoint> spCP;

	if(pCPoint->m_pIConnectionPointContainer)
	{
		//Obtain the connection point 
		hr = pCPoint->FindConnectionPoint(riid, &spCP);
		if(SUCCEEDED(hr))
		{
			//Delegate
			TESTC(hr = Advise(spCP, pdwCookie));
		}
	}

CLEANUP:
	return hr;
}


//////////////////////////////////////////////////////////////////
// CListener::Advise
//
//////////////////////////////////////////////////////////////////
HRESULT CListener::Advise(IConnectionPoint* pIConnectionPoint, DWORD* pdwCookie)
{
	HRESULT hr = S_OK;

	if(pIConnectionPoint)
	{
		//Now we can advise the connection
		//TODO64: Note COM has not changed the Advise output cookie, so we still display it as 0x%08x instead of 0x%p
		XTEST(hr = pIConnectionPoint->Advise((IRowsetNotify*)this, pdwCookie));
		TESTC(TRACE_METHOD(hr, L"IConnectionPoint::Advise(0x%p, &0x%08x)", this, pdwCookie ? *pdwCookie : 0));
	}

CLEANUP:
	return hr;
}


//////////////////////////////////////////////////////////////////
// CListener::Unadvise
//
//////////////////////////////////////////////////////////////////
HRESULT CListener::Unadvise(CContainerBase* pCCPointBase, REFIID riid, DWORD* pdwCookie)
{
	HRESULT hr = S_OK;
	ASSERT(pCCPointBase);
	ASSERT(pdwCookie);
	CComPtr<IConnectionPoint> spCP;

	if(*pdwCookie && pCCPointBase->m_pIConnectionPointContainer)
	{
		//Obtain the connection point 
		hr = pCCPointBase->FindConnectionPoint(riid, &spCP);
		if(SUCCEEDED(hr))
		{
			//Delegate
			TESTC(hr = Unadvise(spCP, pdwCookie));
		}
	}

CLEANUP:
	return hr;
}


//////////////////////////////////////////////////////////////////
// CListener::Unadvise
//
//////////////////////////////////////////////////////////////////
HRESULT CListener::Unadvise(IConnectionPoint* pIConnectionPoint, DWORD* pdwCookie)
{
	HRESULT hr = S_OK;
	ASSERT(pdwCookie);

	if(pIConnectionPoint)
	{
		//Now we can Unadvise the connection
		XTEST(hr = pIConnectionPoint->Unadvise(*pdwCookie));
		TESTC(TRACE_METHOD(hr, L"IConnectionPoint::Unadvise(0x%08x)", *pdwCookie));
		*pdwCookie = 0;
	}

CLEANUP:
	return hr;
}


//////////////////////////////////////////////////////////////////
// CListener::AddRef
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CListener::AddRef()
{
	return ++m_cRef;
}

//////////////////////////////////////////////////////////////////
// CListener::Release
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CListener::Release()
{
	ASSERT(m_cRef);
	if(--m_cRef)
		return m_cRef;
	
	delete this;
	return 0;
}

//////////////////////////////////////////////////////////////////
// CListener::QueryInterface
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP CListener::QueryInterface(REFIID riid, LPVOID* ppv)
{
	if(!ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	if(riid == IID_IUnknown)
		*ppv = (IUnknown*)(IRowsetNotify*)this;
	else if(riid == IID_IRowsetNotify)
		*ppv = (IRowsetNotify*)this;
	else if(riid == IID_IDBAsynchNotify)
		*ppv = (IDBAsynchNotify*)this;
	else if(riid == IID_IRowPositionChange)
		*ppv = (IRowPositionChange*)this;
	
	if(*ppv)
	{
		((IUnknown*)*ppv)->AddRef();
		return S_OK;
	}	
	else
	{
		return E_NOINTERFACE;
	}

}


//////////////////////////////////////////////////////////////////
// CListener::OnFieldChange
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP CListener::OnFieldChange
( 
    /* [in] */ IRowset __RPC_FAR* pIRowset,
    /* [in] */ HROW hRow,
    /* [in] */ DBORDINAL cColumns,
    /* [size_is][in] */ DBORDINAL __RPC_FAR rgColumns[  ],
    /* [in] */ DBREASON eReason,
    /* [in] */ DBEVENTPHASE ePhase,
    /* [in] */ BOOL fCantDeny
)
{
	//Format the column list...
	CComWSTR cstrList; 
	cstrList += L"["; 
	for(DBORDINAL i=0; i<cColumns; i++)
		cstrList += StackFormat(L"%d%s", rgColumns[i], (i+1<cColumns) ? L"," : L"");
	cstrList += L"]"; 

	//Output Notification
	return TRACE_NOTIFICATION(NOTIFY_IROWSETNOTIFY, m_hrReturn, L"IRowsetNotify", L"OnFieldChange", L"(%s, %s, 0x%p, 0x%p, %lu, %s, %s)", GetReasonName(eReason), GetPhaseName(ePhase), pIRowset, hRow, cColumns, (WCHAR*)cstrList, fCantDeny ? L"TRUE" : L"FALSE");
}
        
//////////////////////////////////////////////////////////////////
// CListener::OnRowChange
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP CListener::OnRowChange
( 
    /* [in] */ IRowset __RPC_FAR* pIRowset,
    /* [in] */ DBCOUNTITEM cRows,
    /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
    /* [in] */ DBREASON eReason,
    /* [in] */ DBEVENTPHASE ePhase,
    /* [in] */ BOOL fCantDeny
)
{
	//Format the row handle list...
	CComWSTR cstrList; 
	cstrList += L"["; 
	for(DBCOUNTITEM i=0; i<cRows; i++)
		cstrList += StackFormat(L"%Id%s", rghRows[i], (i+1<cRows) ? L"," : L"");
	cstrList += L"]"; 

	//Output Notification
	return TRACE_NOTIFICATION(NOTIFY_IROWSETNOTIFY, m_hrReturn, L"IRowsetNotify", L"OnRowChange", L"(%s, %s, 0x%p, %Iu, %s, %s)", GetReasonName(eReason), GetPhaseName(ePhase), pIRowset, cRows, (WCHAR*)cstrList, fCantDeny ? L"TRUE" : L"FALSE");
}
        
//////////////////////////////////////////////////////////////////
// CListener::OnRowsetChange
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP CListener::OnRowsetChange
( 
    /* [in] */ IRowset __RPC_FAR* pIRowset,
    /* [in] */ DBREASON eReason,
    /* [in] */ DBEVENTPHASE ePhase,
    /* [in] */ BOOL fCantDeny)
{
	//Output Notification
	return TRACE_NOTIFICATION(NOTIFY_IROWSETNOTIFY, m_hrReturn, L"IRowsetNotify", L"OnRowsetChange", L"(%s, %s, 0x%p, %s)", GetReasonName(eReason), GetPhaseName(ePhase), pIRowset, fCantDeny ? L"TRUE" : L"FALSE");
}


//////////////////////////////////////////////////////////////////
// CListener::OnLowResource
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP CListener::OnLowResource
(
	DB_DWRESERVE dwReserved
)
{
	//Output Notification
	return TRACE_NOTIFICATION(NOTIFY_IDBASYNCHNOTIFY, m_hrReturn, L"IDBAsynchNotify", L"OnLowResource", L"(0x%p)", dwReserved);
}
        
//////////////////////////////////////////////////////////////////
// CListener::OnProgress
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP CListener::OnProgress
		( 
            HCHAPTER		hChapter,
            DBASYNCHOP		eOperation,
            DBCOUNTITEM		ulProgress,
            DBCOUNTITEM		ulProgressMax,
            DBASYNCHPHASE	eAsynchPhase,
            LPOLESTR		pwszStatusText
		)
{
	//Output Notification
	return TRACE_NOTIFICATION(NOTIFY_IDBASYNCHNOTIFY, m_hrReturn, L"IDBAsynchNotify", L"OnProgress", L"(0x%p, %s, %Iu, %Iu, %s, %s)", hChapter, GetAsynchReason(eOperation), ulProgress, ulProgressMax, GetAsynchPhase(eAsynchPhase), pwszStatusText);
}
        
//////////////////////////////////////////////////////////////////
// CListener::OnStop
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP CListener::OnStop
		( 
            HCHAPTER hChapter,
            ULONG    ulOperation,
           	HRESULT  hrStatus,
            LPOLESTR pwszStatusText
		)
{
	//Output Notification
	return TRACE_NOTIFICATION(NOTIFY_IDBASYNCHNOTIFY, m_hrReturn, L"IDBAsynchNotify", L"OnStop", L"(0x%p, %s, %S, %s)", hChapter, GetAsynchReason(ulOperation), GetErrorName(hrStatus), pwszStatusText);
}


//////////////////////////////////////////////////////////////////
// CListener::OnRowPositionChange
//
//////////////////////////////////////////////////////////////////
STDMETHODIMP CListener::OnRowPositionChange(
		DBREASON		eReason,
		DBEVENTPHASE	ePhase,
		BOOL			fCantDeny)
{
	//Output Notification
	return TRACE_NOTIFICATION(NOTIFY_IROWPOSITIONCHANGE, m_hrReturn, L"IRowPositionChange", L"OnRowPositionChange", L"(%s, %s, %s)", GetReasonName(eReason), GetPhaseName(ePhase), fCantDeny ? L"TRUE" : L"FALSE");
}



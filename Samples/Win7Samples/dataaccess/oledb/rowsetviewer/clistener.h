//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CLISTENER.H
//
//-----------------------------------------------------------------------------------

#ifndef _CLISTENER_H_
#define _CLISTENER_H_


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
						

/////////////////////////////////////////////////////////////////
// CListener 
//
/////////////////////////////////////////////////////////////////
class CListener :	public IRowsetNotify,		//Rowset Notifications
					public IDBAsynchNotify,		//Asynch Notifications
					public IRowPositionChange	//RowPosition Notifications
{
public:
	CListener();
	virtual ~CListener();
	
	virtual HRESULT			Advise(CContainerBase* pCCPointBase, REFIID riid, DWORD* pdwCookie);
	virtual HRESULT			Advise(IConnectionPoint* pIConnectionPoint, DWORD* pdwCookie);

	virtual HRESULT			Unadvise(CContainerBase* pCCPointBase, REFIID riid, DWORD* pdwCookie);
	virtual HRESULT			Unadvise(IConnectionPoint* pIConnectionPoint, DWORD* pdwCookie);

	virtual HRESULT			SetReturnValue(HRESULT hrReturn);
	virtual HRESULT			GetReturnValue();

	STDMETHODIMP_(ULONG)	AddRef(void);
	STDMETHODIMP_(ULONG)	Release(void);
	STDMETHODIMP			QueryInterface(REFIID riid, LPVOID *ppv);

	//IRowsetNotify
    STDMETHODIMP OnFieldChange
		( 
            IRowset* pIRowset,
            HROW hRow,
            DBORDINAL cColumns,
            DBORDINAL rgColumns[  ],
            DBREASON eReason,
            DBEVENTPHASE ePhase,
            BOOL fCantDeny
		);
        
    STDMETHODIMP OnRowChange
		( 
            IRowset* pIRowset,
            DBCOUNTITEM cRows,
            const HROW rghRows[  ],
            DBREASON eReason,
            DBEVENTPHASE ePhase,
            BOOL fCantDeny
		);
        
	STDMETHODIMP OnRowsetChange
		( 
			IRowset* pIRowset,
            DBREASON eReason,
            DBEVENTPHASE ePhase,
            BOOL fCantDeny
		);


	//IDBAsynchNotify
    STDMETHODIMP OnLowResource
		( 
            DB_DWRESERVE dwReserved
		);
        
    STDMETHODIMP OnProgress
		( 
            HCHAPTER		hChapter,
            DBASYNCHOP		eOperation,
            DBCOUNTITEM		ulProgress,
            DBCOUNTITEM		ulProgressMax,
            DBASYNCHPHASE	eAsynchPhase,
            LPOLESTR pwszStatusText
		);
        
	STDMETHODIMP OnStop
	   ( 
            HCHAPTER		hChapter,
            DBASYNCHOP		eOperation,
            HRESULT			hrStatus,
            LPOLESTR		pwszStatusText
		);

	//IRowPositionChange
	STDMETHODIMP OnRowPositionChange
		(
			DBREASON			eReason,
			DBEVENTPHASE		ePhase,
			BOOL				fCantDeny
		);



protected:
	//Data
	ULONG			m_cRef;			// reference count
	HRESULT			m_hrReturn;
};



#endif //_CLISTENER_H_

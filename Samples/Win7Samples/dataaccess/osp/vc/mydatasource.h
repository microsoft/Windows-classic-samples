//--------------------------------------------------------------------
// Microsoft OLE DB Sample OLEDB Simple Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation.  All Rights Reserved.
//
// module MyDataSource.h | MyDataSource object implementation
//
//
#ifndef _MyDataSource_H_
#define _MyDataSource_H_


////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////
#include "msdaosp.h"
#include "MyOSPObject.h"
#include "CExList.h"

////////////////////////////////////////////////////////
// MyDataSource
//
////////////////////////////////////////////////////////
class MyDataSource	: public IDataSource
{
public:
	//constructors
	MyDataSource();
	virtual ~MyDataSource();

	virtual HRESULT Init();

	//IUnknown
	virtual inline STDMETHODIMP_(ULONG)	AddRef()								
	{																
		InterlockedIncrement((LONG*)&m_cRef);						
		return m_cRef;												
	}																
	virtual inline STDMETHODIMP_(ULONG)	Release()								
	{																
		if(InterlockedDecrement((LONG*)&m_cRef))					
			return m_cRef;											
																	
		delete this;												
		return 0;													
	}																
	virtual STDMETHODIMP	QueryInterface(REFIID riid, void** ppv);

	//IDataSource methods
	virtual STDMETHODIMP	getDataMember(BSTR bstrDM, REFIID riid, IUnknown** ppUnk);
    virtual STDMETHODIMP	getDataMemberName(LONG lIndex, BSTR* pbstrDM);
    virtual STDMETHODIMP	getDataMemberCount(LONG* plCount);
        
	virtual STDMETHODIMP	addDataSourceListener(IDataSourceListener* pDSL);
    virtual STDMETHODIMP	removeDataSourceListener(IDataSourceListener* pDSL);

protected:
	//data
	MyOSPObject*					m_pMyOSPObject;
	CExList<IDataSourceListener*>	m_listListeners;

	//tables
	ULONG	m_cTables;
	WCHAR*	m_rgpwszTableName[1];

	//IUnknown
	ULONG m_cRef;
};

#endif //_MyDataSource_H_


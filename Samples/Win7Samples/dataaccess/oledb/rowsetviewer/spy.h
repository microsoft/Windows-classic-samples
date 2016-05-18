//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module SPY.H
//
//-----------------------------------------------------------------------------------

#ifndef _SPY_H_
#define _SPY_H_


/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////
#include "objidl.h" //IMallocSpy
#include "List.h"   //CList


/////////////////////////////////////////////////////////////////////////////
// C-Runtime hooks
//
/////////////////////////////////////////////////////////////////////////////
void EnableCRTReportHook(BOOL fEnable = TRUE);
void EnableCRTAllocHook(BOOL fEnable = TRUE);


/////////////////////////////////////////////////////////////////////////////
// CMallocSpy
//
/////////////////////////////////////////////////////////////////////////////
class CMallocSpy : public IMallocSpy
{
public:
    CMallocSpy();
	virtual ~CMallocSpy();

	//Interface
	virtual HRESULT AddToList(void* pv);
	virtual HRESULT RemoveFromList(void* pv);
	virtual HRESULT DumpLeaks();
	virtual void	Reset();

	//Registration
	virtual HRESULT Register();
	virtual HRESULT Unregister();

    // IUnknown methods
    virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppIUnknown);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

	// IMallocSpy methods

    //Alloc
	virtual STDMETHODIMP_(SIZE_T) PreAlloc(SIZE_T cbRequest);
    virtual STDMETHODIMP_(void*) PostAlloc(void *pActual);

    //Free
	virtual STDMETHODIMP_(void*) PreFree(void *pRequest, BOOL fSpyed);
    virtual STDMETHODIMP_(void ) PostFree(BOOL fSpyed);

    //Realloc
	virtual STDMETHODIMP_(SIZE_T) PreRealloc(void *pRequest, SIZE_T cbRequest, void **ppNewRequest, BOOL fSpyed);
    virtual STDMETHODIMP_(void*) PostRealloc(void *pActual, BOOL fSpyed);

    //GetSize
	virtual STDMETHODIMP_(void*) PreGetSize(void *pRequest, BOOL fSpyed);
    virtual STDMETHODIMP_(SIZE_T) PostGetSize(SIZE_T cbActual, BOOL fSpyed);

    //DidAlloc
	virtual STDMETHODIMP_(void*) PreDidAlloc(void *pRequest, BOOL fSpyed);
    virtual STDMETHODIMP_(BOOL)  PostDidAlloc(void *pRequest, BOOL fSpyed, BOOL fActual);

    //HeapMinimize
	virtual STDMETHODIMP_(void ) PreHeapMinimize();
    virtual STDMETHODIMP_(void ) PostHeapMinimize();


protected:
    ULONG    m_cRef;				//Reference count
    SIZE_T   m_cbRequest;			//Bytes requested
	ULONG	 m_cAllocations;		//Memory Allocations

	CList<void*, void*> CAllocList; //List to keep track of leaks
};


#endif // _SPY_H_

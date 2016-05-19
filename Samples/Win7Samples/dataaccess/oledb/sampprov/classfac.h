//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module CLASSFAC.H | Class Definitions for CClassFactory and 
// DLL Entry Points
//
//
#ifndef _CLASSFAC_H_
#define _CLASSFAC_H_



// Classes -------------------------------------------------------------------


//----------------------------------------------------------------------------
// @class CClassFactory 
//
class CClassFactory : public IClassFactory		//@base public | IClassFactory
{
	protected: //@access protected
		//@cmember Reference count
		DBREFCOUNT		m_cRef;
		CLSID			m_clsid;

	public: // @access public
		//@cmember Constructor
		CClassFactory(REFCLSID clsid);
		//@cmember Destructor
		~CClassFactory(void);

		//	IUnknown members
		//@cmember Request an Interface
		STDMETHODIMP				QueryInterface(REFIID, LPVOID *);
		//@cmember Increments the Reference count
		STDMETHODIMP_(DBREFCOUNT)	AddRef(void);
		//@cmember Decrements the Reference count
		STDMETHODIMP_(DBREFCOUNT)	Release(void);

		//	IClassFactory members
		//@cmember Instantiates an uninitialized instance of an object
		STDMETHODIMP			CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
	    //@cmember Lock Object so that it can not be unloaded
		STDMETHODIMP			LockServer(BOOL);
};

#endif


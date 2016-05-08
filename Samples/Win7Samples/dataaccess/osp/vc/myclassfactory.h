//--------------------------------------------------------------------
// Microsoft OLE DB Sample OLEDB Simple Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation.  All Rights Reserved.
//
// module MyClassFactory.h | Class Definitions for MyClassFactory and 
// DLL Entry Points
//
//

////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////
#include "MyDataSource.h"


////////////////////////////////////////////////////////
// MyClassFactory
//
////////////////////////////////////////////////////////
class MyClassFactory : public IClassFactory
{
	public: // @access public
		MyClassFactory(void);
		virtual ~MyClassFactory(void);

		//	IUnknown members
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

		//	IClassFactory members
		STDMETHODIMP	CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
		STDMETHODIMP	LockServer(BOOL);

		HRESULT			GetProviderCLSID();	

	protected: //@access protected
		ULONG			m_cRef;
		GUID			m_guidProv;
};



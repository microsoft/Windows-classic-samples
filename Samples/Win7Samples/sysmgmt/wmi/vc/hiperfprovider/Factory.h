////////////////////////////////////////////////////////////////////////

//

//	Factory.h

//

//	Module:	WMI high performance provider sample code

//

//	This is a standard class factory implementation for the CHiPerfProvider

//	object.  

//

//	

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
//
//	CClassFactory
//
//////////////////////////////////////////////////////////////

class CClassFactory : public IClassFactory
{
protected:
	long	m_lRef;

public:
	CClassFactory() : m_lRef(0) {}

	// Standard COM methods
	// ====================

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	// IClassFactory COM interfaces
	// ============================

	STDMETHODIMP CreateInstance(
		/* [in] */ IUnknown* pUnknownOuter, 
		/* [in] */ REFIID iid, 
		/* [out] */ LPVOID *ppv);	

	STDMETHODIMP LockServer(
		/* [in] */ BOOL bLock);
};
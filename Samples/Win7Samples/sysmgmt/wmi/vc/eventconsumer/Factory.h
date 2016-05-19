// **************************************************************************

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  factory.h
//
// Description: Event consumer provider class factory definition
//    
//
// History:
//
// **************************************************************************

#include <wbemcli.h>

class CProviderFactory : public IClassFactory
{
public:

	CProviderFactory(CListBox	*pOutputList);
	virtual ~CProviderFactory();

    // IUnknown members
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHOD_(SCODE, CreateInstance)(IUnknown * pUnkOuter, 
									REFIID riid, 
									void ** ppvObject);

    STDMETHOD_(SCODE, LockServer)(BOOL fLock);

private:
	LONG m_cRef;
	CListBox	*m_pOutputList;

};

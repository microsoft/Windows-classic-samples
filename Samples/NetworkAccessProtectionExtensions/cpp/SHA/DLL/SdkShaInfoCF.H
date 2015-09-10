#pragma once
#include "Unknwn.h"

class CSdkShaInfoCF :  public IClassFactory
{
public:
    CSdkShaInfoCF();

    ~CSdkShaInfoCF();

public:

    //IUnknown interface 

    IFACEMETHOD(QueryInterface)(
        /* [in] */ __RPC__in REFIID riid , 
        /* [out] */ __RPC__deref_out void **ppObj);

    IFACEMETHOD_(ULONG, AddRef)();

    IFACEMETHOD_(ULONG, Release)();

public:

    // IClassFactory interface

    IFACEMETHOD(CreateInstance)(
        /* [in] */ __RPC__in_opt IUnknown *pUnkOuter,
	    /* [in] */ __RPC__in REFIID riid,
	    /* [out] */ __RPC__deref_out void **ppvObject);

    IFACEMETHOD(LockServer)(
        /* [in] */ BOOL fLock) ; 

protected:

private:

    LONG m_nRefCount;
};

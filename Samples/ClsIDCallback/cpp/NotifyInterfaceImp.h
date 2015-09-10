#pragma once

#include <windows.h>
#include <tchar.h>
#include <bits.h>
#include <strsafe.h>
#include <shlobj.h>

#include "Bits3_0.h"

extern long g_lObjsInUse;
extern long g_lServerLocks;
extern DWORD g_dwMainThreadID;

// This is the root class of all classes the objects of which are referenced counted.
class CReferenceCountedObject : public IUnknown
{
  public :
    CReferenceCountedObject();
    virtual ~CReferenceCountedObject();

	// IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

  protected :
	LONG		m_cRef;
};



class CClassFactory : public IClassFactory
{
  public :
    CClassFactory();
	virtual ~CClassFactory();

	// IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	// IClassFactory methods.
    STDMETHODIMP CreateInstance
	( 
      IUnknown __RPC_FAR *pUnkOuter,
      REFIID riid,
      void __RPC_FAR *__RPC_FAR *ppvObject
	);
        
    STDMETHODIMP LockServer
	(
      BOOL fLock
	);

  protected :
	LONG		m_cRef;

  public :
	// Shut down the application.
	static void AttemptToTerminateServer();
};

class CNotifyInterfaceImp : public CReferenceCountedObject, public IBackgroundCopyCallback2
{
  public :
    CNotifyInterfaceImp();
    ~CNotifyInterfaceImp();

	// Overridden IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

  public :
    // IBackgroundCopyCallback2 interface impl.
	STDMETHOD(JobTransferred)(IBackgroundCopyJob* pJob);
	STDMETHOD(JobError)(IBackgroundCopyJob* pJob, IBackgroundCopyError* pError);
	STDMETHOD(JobModification)(IBackgroundCopyJob* pJob, DWORD dwReserved);
	STDMETHOD(FileTransferred)(IBackgroundCopyJob* pJob, IBackgroundCopyFile* pFile);

    // IDispatch interface impl.
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
    {
	  return E_NOTIMPL;
    }
	   
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
    {
	  return E_NOTIMPL;
    }
	   
    STDMETHOD(GetIDsOfNames)(REFIID riid, __in_ecount(cNames) LPOLESTR* rgszNames, UINT cNames,
		   LCID lcid, DISPID* rgdispid)
    {
	  return E_NOTIMPL;
    }
	   
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		   LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		   EXCEPINFO* pexcepinfo, UINT* puArgErr)
    {
	  return E_NOTIMPL;
    }

};

class CNotifyInterfaceImp_Factory : public CClassFactory
{
  public :
    CNotifyInterfaceImp_Factory();
	~CNotifyInterfaceImp_Factory();

	// IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	// IClassFactory methods.
    STDMETHODIMP CreateInstance
	( 
      IUnknown __RPC_FAR *pUnkOuter,
      REFIID riid,
      void __RPC_FAR *__RPC_FAR *ppvObject
	);
        
    STDMETHODIMP LockServer
	(
      BOOL fLock
	);
};


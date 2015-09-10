#include "StdAfx.h"
#include "NotifyInterfaceImp.h"

CReferenceCountedObject::CReferenceCountedObject() :
m_cRef(1)
{
	// Increment the global count of objects.
	InterlockedIncrement(&g_lObjsInUse);
}

CReferenceCountedObject::~CReferenceCountedObject()
{
	// Decrement the global count of objects.
	InterlockedDecrement(&g_lObjsInUse);

	CClassFactory::AttemptToTerminateServer();
}

STDMETHODIMP CReferenceCountedObject::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_IUnknown)
	{
		*ppvObject = (IUnknown*)this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CReferenceCountedObject::AddRef()
{
	InterlockedIncrement(&m_cRef);

	return m_cRef;
}

STDMETHODIMP_(ULONG) CReferenceCountedObject::Release()
{
	InterlockedDecrement(&m_cRef);

	if (m_cRef == 0)
	{	
		delete this;	
		return 0;
	}

	return m_cRef;
}

CClassFactory::CClassFactory() :
m_cRef(1)
{
}

CClassFactory::~CClassFactory()
{
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_IUnknown)
	{
		CReferenceCountedObject* pCReferenceCountedObject = (CReferenceCountedObject*)this;

		*ppvObject = (IUnknown*)pCReferenceCountedObject;
		((CReferenceCountedObject*)this) -> AddRef();
		return S_OK;
	}

	if (riid == IID_IClassFactory)
	{
		*ppvObject = (IClassFactory*)this;
		((CReferenceCountedObject*)this) -> AddRef();
		return S_OK;
	}

	*ppvObject = NULL;

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
	InterlockedIncrement(&m_cRef);

	return m_cRef;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
	InterlockedDecrement(&m_cRef);

	if (m_cRef == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}

// IClassFactory method.
STDMETHODIMP CClassFactory::CreateInstance
(
IUnknown __RPC_FAR *pUnkOuter,
REFIID riid,
void __RPC_FAR *__RPC_FAR *ppvObject
)
{
	return E_NOTIMPL;
}

// IClassFactory method.
STDMETHODIMP CClassFactory::LockServer
(
BOOL fLock
)
{
	if (fLock) 
	{
		::InterlockedIncrement(&g_lServerLocks) ; 
	}
	else
	{
		::InterlockedDecrement(&g_lServerLocks) ;
	}
	// If this is an out-of-proc server, check to see
	// whether we should shut down.
	AttemptToTerminateServer() ;  //@local

	return S_OK ;
}

// Shut down the application.
void CClassFactory::AttemptToTerminateServer()
{
	if ((g_lObjsInUse > 0) || (g_lServerLocks > 0))
	{
	}
	else
	{
		::PostThreadMessage(g_dwMainThreadID, WM_QUIT, 0, 0);
	}
}

CNotifyInterfaceImp::CNotifyInterfaceImp()
{
}

CNotifyInterfaceImp::~CNotifyInterfaceImp()
{
}

STDMETHODIMP CNotifyInterfaceImp::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_IUnknown || riid == __uuidof(IBackgroundCopyCallback2))
	{
		*ppvObject = (IBackgroundCopyCallback2*)this;
		((CReferenceCountedObject*)this) -> AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

// IUnknown method.
STDMETHODIMP_(ULONG) CNotifyInterfaceImp::AddRef()
{
	return CReferenceCountedObject::AddRef();
}

// IUnknown method.
STDMETHODIMP_(ULONG) CNotifyInterfaceImp::Release()
{
	return CReferenceCountedObject::Release();
}


STDMETHODIMP CNotifyInterfaceImp::JobTransferred(IBackgroundCopyJob* pJob)
{
	printf("Job Transfred\n");
	pJob->Complete();
	printf("It is OK to close the command window.\n");
	return S_OK;
}

STDMETHODIMP CNotifyInterfaceImp::JobError(IBackgroundCopyJob* pJob, IBackgroundCopyError* pError)
{
	printf("Job Failed\n");
	pJob->Cancel();
	printf("It is OK to close the command window.\n");
	return S_OK;
}

STDMETHODIMP CNotifyInterfaceImp::JobModification(IBackgroundCopyJob* pJob, DWORD dwReserved)
{
	return S_OK;
}

STDMETHODIMP CNotifyInterfaceImp::FileTransferred(IBackgroundCopyJob* pJob, IBackgroundCopyFile* pFile)
{
	return S_OK;
}

CNotifyInterfaceImp_Factory::CNotifyInterfaceImp_Factory()
{
}

CNotifyInterfaceImp_Factory::~CNotifyInterfaceImp_Factory()
{
}

// IUnknown method. Overridde QueryInterface() method.
STDMETHODIMP CNotifyInterfaceImp_Factory::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_IUnknown)
	{
		CReferenceCountedObject* pCReferenceCountedObject = (CReferenceCountedObject*)this;

		*ppvObject = (IUnknown*)pCReferenceCountedObject;
		((CReferenceCountedObject*)this) -> AddRef();
		return S_OK;
	}

	if (riid == IID_IClassFactory)
	{
		*ppvObject = (IClassFactory*)this;
		((CReferenceCountedObject*)this) -> AddRef();
		return S_OK;
	}

	*ppvObject = NULL;

	return E_NOINTERFACE;
}

// IUnknown method.
STDMETHODIMP_(ULONG) CNotifyInterfaceImp_Factory::AddRef()
{
	return CClassFactory::AddRef();
}

// IUnknown method.
STDMETHODIMP_(ULONG) CNotifyInterfaceImp_Factory::Release()
{
	return CClassFactory::Release();
}

// IClassFactory method.
STDMETHODIMP CNotifyInterfaceImp_Factory::CreateInstance
(
IUnknown __RPC_FAR *pUnkOuter,
REFIID riid,
void __RPC_FAR *__RPC_FAR *ppvObject
)
{
	CNotifyInterfaceImp* pCExeObj01 = NULL;

	// Initialise the receiver.
	*ppvObject = NULL;

	if (pUnkOuter != NULL)
	{
		return CLASS_E_NOAGGREGATION;
	}

	// Create an instance of the component.
	pCExeObj01 = new CNotifyInterfaceImp;

	if (pCExeObj01 == NULL)
	{
		return E_OUTOFMEMORY;
	}

	pCExeObj01 -> QueryInterface (riid, ppvObject);

	pCExeObj01 -> Release();

	printf("return a new callback instance\n");
	return S_OK;
}

// IClassFactory method.
STDMETHODIMP CNotifyInterfaceImp_Factory::LockServer
(
BOOL fLock
)
{
	return CClassFactory::LockServer(fLock);
}

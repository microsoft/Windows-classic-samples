//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CBASE.CPP
//
//-----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CBase::CBase
//
/////////////////////////////////////////////////////////////////
CBase::CBase(SOURCE eObjectType, CMainWindow* pCMainWindow, CMDIChild* pCMDIChild)
{
	//IUnknown
	m_cRef							= 1;

	//BackPointers
	ASSERT(pCMainWindow || pCMDIChild);
	m_pCMDIChild			=  pCMDIChild;
	m_pCMainWindow			=  pCMainWindow ? pCMainWindow : pCMDIChild->m_pCMainWindow;
	
	//Common OLE DB Interfaces
	m_pIUnknown						= NULL;		
	m_pISupportErrorInfo			= NULL;		
	m_pIAggregate					= NULL;		
	m_pIService						= NULL;

	//Data
	m_hTreeItem						= NULL;
	m_eObjectType					= eObjectType;
	m_eBaseClass					= eCBase;
	m_dwCLSCTX						= CLSCTX_INPROC_SERVER;

	//Parent Info
	m_pCParent						= NULL;
	m_guidSource					= GUID_NULL;
}


/////////////////////////////////////////////////////////////////
// CBase::~CBase
//
/////////////////////////////////////////////////////////////////
CBase::~CBase()
{
	//ReleaseObject should have already been called...
	//If you hit this ASSERT put a "ReleaseObject(0)" in the obhjects destructor...

	//NOTE:  We can't just call ReleaseObject here since its a virtual function.
	//Calling a virtual function would invoke the derived class, which when we are here
	//(in the base destructor) the derived class is already garbaged!
	ASSERT(m_pIUnknown == NULL);
	ASSERT(m_pCParent == NULL);

	//Make sure this item is removed from the tree...
	CObjTree* pCObjTree = m_pCMainWindow->m_pCMDIObjects->m_pCObjTree;
	if(pCObjTree && m_hTreeItem)
	{
		//NOTE: The object (after this desctructor) is no longer available,
		//so make sure that even if the node cannot be removed (child nodes),
		//we need to still remove the object reference so it doesn't try and access it anymore...
		pCObjTree->RemoveObject(this);
		pCObjTree->SetItemParam(m_hTreeItem, NULL);
	}
}


/////////////////////////////////////////////////////////////////
// CBase::AddRef
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CBase::AddRef()
{																
	//AddRef
	return ++m_cRef;
}																


/////////////////////////////////////////////////////////////////
// CBase::Release
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CBase::Release()
{
	//NOTE: The only thing that we have to be careful with is that our objects are refcounted,
	//but they contain a pointer to the window, which might have gone away or be going away.
	//So if the window is no longer valid, make sure we NULL out our window pointer...
	if(m_pCMDIChild && !m_pCMDIChild->m_hWnd)
		m_pCMDIChild = NULL;

	//Release
	if(--m_cRef)					
		return m_cRef;											
																
	delete this;												
	return 0;													
}																

/////////////////////////////////////////////////////////////////
// CBase::QueryInterface
//
/////////////////////////////////////////////////////////////////
STDMETHODIMP			CBase::QueryInterface(REFIID riid, LPVOID *ppv)
{
	if(!ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	//IUNKNOWN
	if(riid == IID_IUnknown)
		*ppv = this;
	
	if(*ppv)
	{
		((IUnknown*)(*ppv))->AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


/////////////////////////////////////////////////////////////////
// CBase::ObjectAddRef
//
/////////////////////////////////////////////////////////////////
ULONG CBase::ObjectAddRef()
{
	//Record the number of times the user has addref'd this object.
	//So we can know when to correctly NULL out the interface, so they can longer use it...
	AddRef();

	//IUnknown::AddRef
	return TRACE_ADDREF(m_pIUnknown, GetObjectName());
}


/////////////////////////////////////////////////////////////////
// CBase::ObjectRelease
//
/////////////////////////////////////////////////////////////////
ULONG CBase::ObjectRelease()
{
	IUnknown* pIUnknown = m_pIUnknown;
	ULONG ulRefCount = 0;

	//IUnknown::Release
	ulRefCount = CIntTrace::TraceRelease(&pIUnknown, GetObjectName());
	
	//We need to disable this interface once it hits zero so
	//its not used incorrectly after the object is released...
	if(ulRefCount == 0 || m_cRef<=1)
	{
		m_pIUnknown = NULL;
		ReleaseObject();
	}
	else
	{
		Release();
	}
	
	return ulRefCount;
}


/////////////////////////////////////////////////////////////////
// HRESULT CBase::SetInterface
//
/////////////////////////////////////////////////////////////////
HRESULT CBase::SetInterface(REFIID riid, IUnknown* pIUnknown)
{
	//We need to put the obtained interface in the appropiate member...
	if(pIUnknown)
	{
		//First obtain the correct interface member variable...
		IUnknown** ppInterfaceMember = GetInterfaceAddress(riid);
		if(!ppInterfaceMember)
			return E_NOINTERFACE;

		TRACE_RELEASE(*ppInterfaceMember, GetObjectName());
		TRACE_ADDREF(pIUnknown, GetObjectName());
		*ppInterfaceMember = pIUnknown;
	}

	return S_OK;
}

	
/////////////////////////////////////////////////////////////////
// HRESULT CBase::GetInterface
//
/////////////////////////////////////////////////////////////////
IUnknown* CBase::GetInterface(REFIID riid)
{
	//Delegate to the derived class...
	IUnknown** ppIUnknown = GetInterfaceAddress(riid);
	if(ppIUnknown)
		return *ppIUnknown;

	return NULL;
}


/////////////////////////////////////////////////////////////////
// HRESULT CBase::ObjectQI
//
/////////////////////////////////////////////////////////////////
HRESULT CBase::ObjectQI(REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT hr = S_OK;
	
	//IUnknown::QueryInterface
	XTESTC(hr = TRACE_QI(m_pIUnknown, riid, ppIUnknown, GetObjectName()));
	
	//We need to put the obtained interface in the appropiate member...
	if(ppIUnknown && *ppIUnknown)
		hr = SetInterface(riid, *ppIUnknown);
	
CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// CBase::ReleaseObject
//
/////////////////////////////////////////////////////////////////
HRESULT CBase::ReleaseObject(ULONG ulExpectedRefCount)
{
	//Release Derived interfaces first...
	AutoRelease();
	
	//IUnknown
	TRACE_RELEASE_(m_pIUnknown, L"IUnknown", ulExpectedRefCount);

	//Cleanup when there are no more references
	if(ulExpectedRefCount==0)
	{
		//Remove this object from the tree, unless there is a window associated with it
		m_pCMainWindow->m_pCMDIObjects->m_pCObjTree->RemoveObject(this);
		m_guidSource		= GUID_NULL;

		//Release Parent
		SAFE_RELEASE(m_pCParent);
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// CBase::ReleaseChildren
//
/////////////////////////////////////////////////////////////////
HRESULT CBase::ReleaseChildren()
{
	//Make our lives easier...
	CObjTree* pCObjTree = m_pCMainWindow->m_pCMDIObjects->m_pCObjTree;

	//No-op
	if(!m_hTreeItem || !pCObjTree->m_hWnd)
		return S_OK;
	BOOL bAllRemoved = TRUE;

	//Try to obtain the first child...
	HTREEITEM hChildItem = pCObjTree->GetChildItem(m_hTreeItem);
	while(hChildItem)
	{
		//NOTE: Before deleting this node of the tree, obtain the 
		//next sibling (if there is one...).  Since we can't do this after the node has been deleted!
		HTREEITEM hNextSibling = pCObjTree->GetNextItem(hChildItem);
		CBase* pCChild = (CBase*)pCObjTree->GetItemParam(hChildItem);
		
		if(pCChild)
		{
			//Make sure all its children are released (recurse)
			//If this child cannot be released, we still can continue on to the next child
			//(ie: release everything we can - not all or nothing...)
			if(SUCCEEDED(pCChild->ReleaseChildren()))
			{
				//Now we can release this child
				pCChild->ReleaseObject();
			}
		}
		
		//Go to the next sibling...
		hChildItem = hNextSibling;
	}

	return bAllRemoved ? S_OK : E_FAIL;
}


/////////////////////////////////////////////////////////////////
// HRESULT CBase::CreateObject
//
/////////////////////////////////////////////////////////////////
HRESULT CBase::CreateObject(CBase* pCSource, REFIID riid, IUnknown* pIUnkObject, DWORD dwCreateOpts)
{
	//No-op...
	if(!pIUnkObject)
		return E_INVALIDARG;
	
	//Use exsiting Connection
	IUnknown* pIUnknown = NULL;

	if(dwCreateOpts	== -1 /*Default*/)
		dwCreateOpts = GetOptions()->m_dwCreateOpts;

	HRESULT hr			= S_OK;

	//Store the Parent Object...
	SAFE_RELEASE(m_pCParent);
	SAFE_ADDREF(pCSource);
	m_pCParent = pCSource;

	//First we QI for IID_IUnknown the object passed in.
	//NOTE: We do this before we call ReleaseObject since the caller of this 
	//method could have passed in our own member variable (m_pIUnknown) to "recreate" the object.
	TRACE_QI(pIUnkObject, IID_IUnknown, &pIUnknown, GetObjectName());
	
	//Release all Previous interfaces...
	ReleaseObject(1);
	
	//[MANDATORY]
	//Used the AddRef'd input as our IUnknown, since all interfaces inherit from IUnknown.
	m_pIUnknown = pIUnknown;

	//Now set the pointer pased into the appropiate interface member
	//NOTE: We already handled IUnknown above...
	if(riid != IID_IUnknown)
		SetInterface(riid, pIUnkObject);

	//Now AutoQI for derived object interfaces...
	AutoQI(dwCreateOpts);
	
	//Add this object to the Objects Window
	m_pCMainWindow->m_pCMDIObjects->m_pCObjTree->AddObject(pCSource, this);
	
//CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CBase::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CBase::AutoRelease()
{
	//Common OLE DB Interface
	RELEASE_INTERFACE(ISupportErrorInfo);
	RELEASE_INTERFACE(IAggregate);
	RELEASE_INTERFACE(IService);
	return S_OK;
}


/////////////////////////////////////////////////////////////////
// HRESULT CBase::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CBase::AutoQI(DWORD dwCreateOpts)
{
	//[MANDATORY] Obtain [mandatory] interfaces
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
	}

	//[OPTIONAL]
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		OBTAIN_INTERFACE(ISupportErrorInfo);
		OBTAIN_INTERFACE(IAggregate);
		OBTAIN_INTERFACE(IService);
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// IUnknown** CBase::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CBase::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IUnknown);
	HANDLE_GETINTERFACE(ISupportErrorInfo);
	HANDLE_GETINTERFACE(IAggregate);
	HANDLE_GETINTERFACE(IService);

	return NULL;
}


/////////////////////////////////////////////////////////////////
// CBase::IsSameObject
//
/////////////////////////////////////////////////////////////////
BOOL CBase::IsSameObject(IUnknown* pIUnkObject)
{
	CComPtr<IUnknown> spUnknown = m_pIUnknown;
	return spUnknown.IsEqualObject(pIUnkObject);
}


/////////////////////////////////////////////////////////////////
// CBase::GetParent
//
/////////////////////////////////////////////////////////////////
CBase* CBase::GetParent(SOURCE eSource)
{
	CBase* pCParent = m_pCParent;
	
	//Try to find the requested parent object type
	while(pCParent)
	{
		//Do we have a match...
		if(pCParent->m_eObjectType == eSource)
			return pCParent;

		//Try the previous parent...
		pCParent = pCParent->m_pCParent;
	}

	return NULL;
}
	

////////////////////////////////////////////////////////////////
// CBase::SetObjectDesc
//
/////////////////////////////////////////////////////////////////
void	CBase::SetObjectDesc(WCHAR* pwszObjectDesc, BOOL fCopy) 
{ 
	//Optmization:  If the caller no longer needs the name, 
	//no sense in reallocing, copying, freeing, freeing.  Just reference it...
	if(fCopy)
	{
		m_strObjectDesc.CopyFrom(pwszObjectDesc);
	}
	else
	{
		m_strObjectDesc.Attach(pwszObjectDesc);
	}

	//Update the object in the tree...
	if(m_strObjectDesc && m_hTreeItem)
		m_pCMainWindow->m_pCMDIObjects->m_pCObjTree->AddObject(NULL, this);
}


////////////////////////////////////////////////////////////////
// CBase::OnDefOperation
//
/////////////////////////////////////////////////////////////////
void	CBase::OnDefOperation() 
{ 
	//The default implementation (unless its overridden)
	//Activate the MDIChild window assoicated with this object
	if(m_pCMDIChild && m_pCMDIChild->m_hWnd)
		m_pCMainWindow->MDIActivate(m_pCMDIChild->m_hWnd);
}


////////////////////////////////////////////////////////////////
// CBase::DisplayObject
//
/////////////////////////////////////////////////////////////////
HRESULT	CBase::DisplayObject()
{
	//Update the object in the tree...
	if(m_hTreeItem)
		m_pCMainWindow->m_pCMDIObjects->m_pCObjTree->AddObject(NULL, this);
	return S_OK;
}

	
////////////////////////////////////////////////////////////////
// CBase::GetOptions
//
/////////////////////////////////////////////////////////////////
COptionsSheet*	CBase::GetOptions()
{
	return m_pCMainWindow->GetOptions();
}


// {CB21F4D6-878D-11d1-9528-00C04FB66A50}
static const IID IID_IAggregate = 
{ 0xcb21f4d6, 0x878d, 0x11d1, { 0x95, 0x28, 0x0, 0xc0, 0x4f, 0xb6, 0x6a, 0x50 } };


///////////////////////////////////////////////////////////////////////////////
// CAggregate
// 
///////////////////////////////////////////////////////////////////////////////
CAggregate::CAggregate()
{
	m_cRef = 1;
}


///////////////////////////////////////////////////////////////////////////////
// ~CAggregate
// 
///////////////////////////////////////////////////////////////////////////////
CAggregate::~CAggregate() 
{
	//COM Aggregation rule #6
	//To free an inner pointer (other than IUnknown), the outer object calls its 
	//own outer unknown's AddRef followed by Release on the inner object's pointer
	//Currently we don't have this case, since we only have IUnknown
	//AddRef()
	//SAFE_RELEASE(m_pNonIUnknownInner);

	//Inner object free
	ReleaseInner();
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::SetInner
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CAggregate::SetInner(IUnknown* pIUnkInner)
{
	if(!pIUnkInner)
		return E_INVALIDARG;

	TRACE_RELEASE(m_spUnkInner.p, L"IUnknown");
	return TRACE_QI(pIUnkInner, IID_IUnknown, (IUnknown**)&m_spUnkInner);
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::ReleaseInner
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CAggregate::ReleaseInner()
{
	//Only release the inner if the RefCount of the outer has gone to its
	//orginal refcount
	if(m_cRef <= 1)
	{
		TRACE_RELEASE(m_spUnkInner.p, L"IUnknown");
		return S_OK;
	}
	
	return S_FALSE;
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::HandleAggregation
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CAggregate::HandleAggregation(REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT hr = S_OK;
	IUnknown* pIUnkInner = ppIUnknown ? *ppIUnknown : NULL;

	if(pIUnkInner)
	{
		//This would be a bug in the provider if aggregaiton succeeded
		//but the user didn't request IID_IUnknown.  
		if(riid != IID_IUnknown)
		{
			//NOTE:  We don't want to just continue here since this is dangerous.  
			//If the provider succeeds, this means the outer object was probably returned rather
			//than the inner non-delegating IUnknown, this will cause our outer controlling object
			//to have a circular QI problem when asked for an inner interface, and will have refcounting
			//problems as well.  Might as well let the use know, and don't further this provider bug...
			if(IDNO == wMessageBox
				(
					GetFocus(), 
					MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1, 
					wsz_ERROR, 
					L"Provider Bug: Allowed Aggregation and riid!=IID_IUnknown!\n\n"
					L"This may crash your Provider...\n"
					L"Do you wish to continue anyway?"
				))
			{
				//return a failure if the user wished not to continue...
				TESTC(hr = E_INVALIDARG);
			}
		}

		//We need to "hook" up our objects.  So that our outer controlling object
		//has a pointer to the inner for delegating QI calls.  This must be an non-delegating
		//inner IUnknown, (see below).  Also the caller of this function will ALWAYS
		//release their CAggregate object, and we return a the Aggregate object with our 
		//reference count.  (consistent with COM, calle never releases callers IUnknown, and
		//calle always addref's if they hold onto the callers object).
		if(SUCCEEDED(hr = SetInner(pIUnkInner)))
		{
			//SetInner 
			TRACE_RELEASE(pIUnkInner, L"IUnknown");
			hr = QueryInterface(IID_IUnknown, (void**)ppIUnknown);
		}
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::AddRef
//
/////////////////////////////////////////////////////////////////////////////
ULONG	CAggregate::AddRef(void)
{
	return ++m_cRef;
}

/////////////////////////////////////////////////////////////////////////////
//	CAggregate::Release
//
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP_(ULONG)	CAggregate::Release(void)
{
	ASSERT(m_cRef);
	if(--m_cRef)
		return m_cRef;

	//COM Aggregation rule #5
	//The outer object must protect its implementation of Release from 
	//reentrantcy with an artifical reference count arround its destruction code
	m_cRef++;

	//Delete this object
	delete this;
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//	CAggregate::QueryInterface
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CAggregate::QueryInterface(REFIID riid, LPVOID *ppv)
{
	HRESULT hr = S_OK;
	
	//TEST_ NULL
	if(ppv == NULL)
		return E_INVALIDARG;
	*ppv = NULL;

	//Support IID_IUnknown
	if(riid == IID_IUnknown)
	{
		*ppv = (IUnknown*)this;
		SAFE_ADDREF((IUnknown*)*ppv);
	}
	else if(riid == IID_IAggregate)
	{
		*ppv = (IUnknown*)this;
		SAFE_ADDREF((IUnknown*)*ppv);
	}
	else if(m_spUnkInner)
	{
		//Delegate the the Inner Object
		//This is not "circular" since this interface is the IID_IUnknown
		//interface only, which has its own non-delegating QI...
		hr = m_spUnkInner->QueryInterface(riid, ppv);
	}
	else
	{
		return E_NOINTERFACE;
	}

	return hr;
}




/////////////////////////////////////////////////////////////////
// CContainerBase::CContainerBase
//
/////////////////////////////////////////////////////////////////
CContainerBase::CContainerBase(SOURCE eObjectType, CMainWindow* pCMainWindow, CMDIChild* pCMDIChild)
	: CBase(eObjectType, pCMainWindow, pCMDIChild)
{
	//eBaseClass
	m_eBaseClass =	BASE_CLASS(m_eBaseClass | eCContainerBase);
	
	//Common OLE DB Interfaces
	m_pIConnectionPointContainer	= NULL;
}


/////////////////////////////////////////////////////////////////
// CContainerBase::~CContainerBase
//
/////////////////////////////////////////////////////////////////
CContainerBase::~CContainerBase()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// HRESULT CContainerBase::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CContainerBase::AutoRelease()
{
	//Common OLE DB Interface
	RELEASE_INTERFACE(IConnectionPointContainer);

	//Delegate
	return CBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CContainerBase::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CContainerBase::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CBase::AutoQI(dwCreateOpts);
	
	//[MANDATORY] Obtain [mandatory] interfaces
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
	}

	//[OPTIONAL]
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		OBTAIN_INTERFACE(IConnectionPointContainer);
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// IUnknown** CContainerBase::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CContainerBase::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IConnectionPointContainer);
	
	//Otherwise delegate
	return CBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// HRESULT CContainerBase::FindConnectionPoint
//
/////////////////////////////////////////////////////////////////
HRESULT CContainerBase::FindConnectionPoint(REFIID riid, IConnectionPoint** ppIConnectionPoint)
{
	HRESULT hr = S_OK;

	if(m_pIConnectionPointContainer)
	{
		//IConnectionPointContainer::FindConnectionPoint
		hr = m_pIConnectionPointContainer->FindConnectionPoint(riid, ppIConnectionPoint);
		TESTC(TRACE_METHOD(hr, L"IConnectionPointContainer::FindConnectionPoint(%s, &0x%p)", GetInterfaceName(riid), ppIConnectionPoint ? *ppIConnectionPoint : NULL));
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CContainerBase::AdviseListener
//
/////////////////////////////////////////////////////////////////
HRESULT CContainerBase::AdviseListener(REFIID riid, DWORD* pdwCookie)
{
	HRESULT hr = S_OK;
	
	if(m_pIConnectionPointContainer)
	{
		if(riid == IID_IDBAsynchNotify)
		{
			if(GetOptions()->m_dwNotifyOpts & NOTIFY_IDBASYNCHNOTIFY)
				hr = m_pCMainWindow->m_pCListener->Advise(this, riid, pdwCookie);
		}
		else if(riid == IID_IRowsetNotify)
		{
			if(GetOptions()->m_dwNotifyOpts & NOTIFY_IROWSETNOTIFY)
				hr = m_pCMainWindow->m_pCListener->Advise(this, riid, pdwCookie); 
		}
		else if(riid == IID_IRowPositionChange)
		{
			if(GetOptions()->m_dwNotifyOpts & NOTIFY_IROWPOSITIONCHANGE)
				hr = m_pCMainWindow->m_pCListener->Advise(this, riid, pdwCookie);
		}
		else
		{
			ASSERT(!"Unhandled Notification Type!");
		}
	}

	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CContainerBase::UnadviseListener
//
/////////////////////////////////////////////////////////////////
HRESULT CContainerBase::UnadviseListener(REFIID riid, DWORD* pdwCookie)
{
	ASSERT(pdwCookie);
	HRESULT hr = S_OK;

	if(m_pIConnectionPointContainer)
	{
		if(*pdwCookie)
			hr = m_pCMainWindow->m_pCListener->Unadvise(this, riid, pdwCookie);
	}

	return hr;
}





/////////////////////////////////////////////////////////////////
// CConnectionPoint::CConnectionPoint
//
/////////////////////////////////////////////////////////////////
CConnectionPoint::CConnectionPoint(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild)
	: CBase(eCConnectionPoint, pCMainWindow, pCMDIChild)
{
	//Common OLE DB Interfaces
	m_pIConnectionPoint				= NULL;		//Connection interface

	//Data
	m_dwCookie						= 0;
}


/////////////////////////////////////////////////////////////////
// CConnectionPoint::~CConnectionPoint
//
/////////////////////////////////////////////////////////////////
CConnectionPoint::~CConnectionPoint()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// HRESULT CConnectionPoint::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CConnectionPoint::AutoRelease()
{
	//Common OLE DB Interface
	RELEASE_INTERFACE(IConnectionPoint);

	//Delegate
	return CBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CConnectionPoint::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CConnectionPoint::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CBase::AutoQI(dwCreateOpts);
	
	//[MANDATORY] Obtain [mandatory] interfaces
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IConnectionPoint);
	}

	//[OPTIONAL]
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// IUnknown** CConnectionPoint::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CConnectionPoint::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IConnectionPoint);
	
	//Otherwise delegate
	return CBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////////////////
// CConnectionPoint::GetObjectDesc
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* CConnectionPoint::GetObjectDesc()
{
	if(!m_strObjectDesc && m_pIConnectionPoint)
	{
		IID iid;
		if(SUCCEEDED(GetConnectionInterface(&iid)))
			m_strObjectDesc.CopyFrom(GetInterfaceName(iid));
	}

	return m_strObjectDesc;
}


/////////////////////////////////////////////////////////////////
// HRESULT CConnectionPoint::GetConnectionInterface
//
/////////////////////////////////////////////////////////////////
HRESULT CConnectionPoint::GetConnectionInterface(IID* pIID)
{
	HRESULT hr = S_OK;

	if(m_pIConnectionPoint)
	{
		hr = m_pIConnectionPoint->GetConnectionInterface(pIID);
		TESTC(TRACE_METHOD(hr, L"IConnectionPoint::GetConnectionInterface(&%s)", GetInterfaceName(pIID ? *pIID : IID_NULL)));
	}

CLEANUP:
	return hr;
}

				

/////////////////////////////////////////////////////////////////
// CAsynchBase::CAsynchBase
//
/////////////////////////////////////////////////////////////////
CAsynchBase::CAsynchBase(SOURCE eObjectType, CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CContainerBase(eObjectType, pCMainWindow, pCMDIChild)
{
	//eBaseClass
	m_eBaseClass = BASE_CLASS(m_eBaseClass | eCAsynchBase);

	//OLE DB Interfaces
	m_pIDBInitialize			= NULL;		//OLE DB interface
	m_pIDBAsynchStatus			= NULL;		//OLE DB interface

	//Extra interfaces
	m_dwCookieAsynchNotify		= 0;

	//Data
	m_fInitialized				= FALSE;
}


/////////////////////////////////////////////////////////////////
// CAsynchBase::~CAsynchBase
//
/////////////////////////////////////////////////////////////////
CAsynchBase::~CAsynchBase()
{
}


/////////////////////////////////////////////////////////////////
// IUnknown** CAsynchBase::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CAsynchBase::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IDBInitialize);
	HANDLE_GETINTERFACE(IDBAsynchStatus);

	//Otherwise delegate
	return CContainerBase::GetInterfaceAddress(riid);
}


////////////////////////////////////////////////////////////////
// CAsynchBase::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CAsynchBase::AutoRelease()
{
	//UnadviseListeners
	UnadviseListener(IID_IDBAsynchNotify, &m_dwCookieAsynchNotify);

	//OLE DB interfaces
	RELEASE_INTERFACE(IDBInitialize);
	RELEASE_INTERFACE(IDBAsynchStatus);

	//Extra interfaces

	//Data
	m_fInitialized = FALSE;

	//Delegate
	return CContainerBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CAsynchBase::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CAsynchBase::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CContainerBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
	}

	//AutoQI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		//[OPTIONAL]
		OBTAIN_INTERFACE(IDBInitialize);
		OBTAIN_INTERFACE(IDBAsynchStatus);
	}

	//Listeners
	AdviseListener(IID_IDBAsynchNotify, &m_dwCookieAsynchNotify);
	return S_OK;
}


/////////////////////////////////////////////////////////////////
// HRESULT CAsynchBase::Initialize
//
/////////////////////////////////////////////////////////////////
HRESULT CAsynchBase::Initialize()
{
	HRESULT	hr = S_OK;
	DWORD dwCreateOpts = GetOptions()->m_dwCreateOpts;

	if(!m_pIDBInitialize)
		goto CLEANUP;

	//Initailize
	//NOTE: Expect S_OK or DB_E_CANCELED, since canceling the dialog always returns DB_E_CANCELED
	XTEST_(hr = m_pIDBInitialize->Initialize(), DB_E_CANCELED);
	TRACE_METHOD(hr, L"IDBInitialize::Initialize()");

	//Display any property errors...
	TESTC(hr = DisplayPropErrors(hr, IID_IDBProperties, m_pIDBInitialize));

	//We are now Initialized
	m_fInitialized = TRUE;

	//Obtain all interfaces, now that we are initialized
	TESTC(hr = AutoQI(dwCreateOpts));
	
	//Also "redraw" the object now that more interfaces are available for use
	//For Example: If the rowset was originally obtained Asynchronously and now interfaces
	//to obtain columns and data are available, display the object...
	TESTC(hr = DisplayObject());

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CAsynchBase::Uninitialize
//
/////////////////////////////////////////////////////////////////
HRESULT CAsynchBase::Uninitialize()
{
	HRESULT	hr = S_OK;

	if(!m_pIDBInitialize)
		goto CLEANUP;

	//Uninitailize
	XTEST(hr = m_pIDBInitialize->Uninitialize());
	TESTC(TRACE_METHOD(hr, L"IDBInitialize::Uninitialize()"));

	//We are now Uninitialized
	m_fInitialized = FALSE;

CLEANUP:
	return hr;
}



/////////////////////////////////////////////////////////////////
// HRESULT CAsynchBase::Abort
//
/////////////////////////////////////////////////////////////////
HRESULT CAsynchBase::Abort(HCHAPTER hChapter, DBASYNCHOP eOperation)
{
	HRESULT	hr = S_OK;

	if(!m_pIDBAsynchStatus)
		goto CLEANUP;

	//IDBAsynchStatus::Abort
	XTEST(hr = m_pIDBAsynchStatus->Abort(hChapter, eOperation));
	TRACE_METHOD(hr, L"IDBAsynchStatus::Abort(0x%p, %s)", hChapter, GetAsynchReason(eOperation));

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CAsynchBase::GetStatus
//
/////////////////////////////////////////////////////////////////
HRESULT CAsynchBase::GetStatus(HCHAPTER hChapter, DBASYNCHOP eOperation, DBCOUNTITEM* pulProgress, DBCOUNTITEM* pulProgressMax, DBASYNCHPHASE* peAsynchPhase, LPOLESTR* ppwszStatusText)
{
	HRESULT	hr = S_OK;

	if(!m_pIDBAsynchStatus)
		goto CLEANUP;

	//IDBAsynchStatus::GetStatus
	XTEST(hr = m_pIDBAsynchStatus->GetStatus(hChapter, eOperation, pulProgress, pulProgressMax, peAsynchPhase, ppwszStatusText));
	TRACE_METHOD(hr, L"IDBAsynchStatus::GetStatus(0x%p, %s, &%lu, &%lu, &%s, &\"%s\")", hChapter, GetAsynchReason(eOperation), pulProgress ? *pulProgress : 0, pulProgressMax ? *pulProgressMax : 0, GetAsynchPhase(peAsynchPhase ? *peAsynchPhase : 0), ppwszStatusText ? *ppwszStatusText : NULL);
	
CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// CPropertiesBase::CPropertiesBase
//
/////////////////////////////////////////////////////////////////
CPropertiesBase::CPropertiesBase(SOURCE eObjectType, CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CAsynchBase(eObjectType, pCMainWindow, pCMDIChild)
{
	//eBaseClass
	m_eBaseClass = BASE_CLASS(m_eBaseClass | eCPropertiesBase);

	//OLE DB Interfaces
	m_pIDBProperties			= NULL;		//OLE DB interface
}


/////////////////////////////////////////////////////////////////
// CPropertiesBase::~CPropertiesBase
//
/////////////////////////////////////////////////////////////////
CPropertiesBase::~CPropertiesBase()
{
}


/////////////////////////////////////////////////////////////////
// IUnknown** CPropertiesBase::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CPropertiesBase::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IDBProperties);

	//Otherwise delegate
	return CAsynchBase::GetInterfaceAddress(riid);
}


////////////////////////////////////////////////////////////////
// CPropertiesBase::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CPropertiesBase::AutoRelease()
{
	//OLE DB interfaces
	RELEASE_INTERFACE(IDBProperties);

	//Delegate
	return CAsynchBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CPropertiesBase::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CPropertiesBase::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have IConnectionPointContainer
	CAsynchBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IDBProperties);

		//NOTE: Since this class inherits from CAsynchBase which already has an IDBInitialize
		//pointer we will just use that, but the problem is that in the object its an optional
		//interface, and under the DSO its a required interface.  So we just do a QI again
		//if the pointer has not already been retrived...
		OBTAIN_INTERFACE(IDBInitialize);
	}

	//AutoQI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		//[OPTIONAL]
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// HRESULT CPropertiesBase::SetProperties
//
/////////////////////////////////////////////////////////////////
HRESULT CPropertiesBase::SetProperties(ULONG cPropSets, DBPROPSET* rgPropSets)
{
	HRESULT	hr = S_OK;

	if(m_pIDBProperties && cPropSets)
	{
		//SetProperties
		XTEST_(hr = m_pIDBProperties->SetProperties(cPropSets, rgPropSets),S_OK);
		TRACE_METHOD(hr, L"IDBProperties::SetProperties(%d, 0x%p)", cPropSets, rgPropSets);
		
		//Display any property errors...
		TESTC(hr = DisplayPropErrors(hr, cPropSets, rgPropSets));
	}

CLEANUP:
	return hr;
}





/////////////////////////////////////////////////////////////////
// GuidToSourceType
//
/////////////////////////////////////////////////////////////////
SOURCE GuidToSourceType(REFGUID guidType)
{
	if(guidType == DBGUID_ROWSET)
		return eCRowset;
	else if(guidType == DBGUID_ROW)
		return eCRow;
	else if(guidType == DBGUID_COMMAND)
		return eCCommand;
	else if(guidType == DBGUID_SESSION)
		return eCSession;
	else if(guidType == DBGUID_DSO)
		return eCDataSource;
	else if(guidType == DBGUID_STREAM)
		return eCStream;

	return eCUnknown;
}



////////////////////////////////////////////////////////////////
// DetermineObjectType
//
/////////////////////////////////////////////////////////////////
SOURCE DetermineObjectType(IUnknown* pIUnkObject, SOURCE eSource)
{
	//Don't rely upon the caller knowing exactly what the object type is...
	//Since many OLE DB methods can return different objects depending upon interface or properties
	//requested.  So use the "suggested" type as an optimizing starting point, and if not
	//then proceed the hard way to determine exactly what it is...

	//Do we need to figure out what type of object this is?
	if(!pIUnkObject)
		return eCUnknown;
	
	IUnknown* pIUnknown = NULL;
	HRESULT hr = E_NOINTERFACE;

	//See of the object is what the user's "guess" indicates...
	//If it is where done...
	switch(eSource)
	{
		case eCRow:
			hr = TRACE_QI(pIUnkObject, IID_IRow, &pIUnknown);
			break;

		case eCRowset:
			hr = TRACE_QI(pIUnkObject, IID_IRowset, &pIUnknown);
			break;

		case eCCommand:
			hr = TRACE_QI(pIUnkObject, IID_ICommand, &pIUnknown);
			break;

		case eCSession:
			hr = TRACE_QI(pIUnkObject, IID_IOpenRowset, &pIUnknown);
			break;

		case eCMultipleResults:
			hr = TRACE_QI(pIUnkObject, IID_IMultipleResults, &pIUnknown);
			break;

		case eCEnumerator:
			hr = TRACE_QI(pIUnkObject, IID_ISourcesRowset, &pIUnknown);
			break;

		case eCBinder:
			hr = TRACE_QI(pIUnkObject, IID_IBindResource, &pIUnknown);
			break;

		case eCDataSource:
			//IDBInitialize is no longer enough to fully indentify wither the object returned
			//is a DataSource or not.  Other objects also have these: ie: Enumerator, Binder, etc
			if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IDBInitialize, &pIUnknown)))
			{
				TRACE_RELEASE(pIUnknown, L"IDBInitialize");
				hr = TRACE_QI(pIUnkObject, IID_IPersist, &pIUnknown);
			}
			break;

		case eCServiceComp:
			hr = TRACE_QI(pIUnkObject, IID_IDataInitialize, &pIUnknown);
			break;

		case eCDataLinks:
			hr = TRACE_QI(pIUnkObject, IID_IDBPromptInitialize, &pIUnknown);
			break;

		case eCStream:
			//Some providers for some reason may not support the inherited ISequentialStream interface
			if(FAILED(hr = TRACE_QI(pIUnkObject, IID_ISequentialStream, &pIUnknown)))
				hr = TRACE_QI(pIUnkObject, IID_IStream, &pIUnknown);
			break;

		case eCDataset:
			hr = TRACE_QI(pIUnkObject, IID_IMDDataset, &pIUnknown);
			break;

		case eCRowPosition:
			hr = TRACE_QI(pIUnkObject, IID_IRowPosition, &pIUnknown);
			break;

		case eCTransaction:
			hr = TRACE_QI(pIUnkObject, IID_ITransaction, &pIUnknown);
			break;

		case eCTransactionOptions:
			hr = TRACE_QI(pIUnkObject, IID_ITransactionOptions, &pIUnknown);
			break;

		case eCError:
			hr = TRACE_QI(pIUnkObject, IID_IErrorInfo, &pIUnknown);
			break;

		case eCCustomError:
			hr = TRACE_QI(pIUnkObject, IID_ISQLErrorInfo, &pIUnknown);
			break;


		case eCConnectionPoint:
			hr = TRACE_QI(pIUnkObject, IID_IConnectionPoint, &pIUnknown);
			break;

		default:
			hr = E_NOINTERFACE;
			break;
	};
	
	//If the object doesn't match the users guess we will need to try and determine what it really is...
	if(FAILED(hr))
	{
		//Since many objects support the same interface, 
		//we need to actually QI for a "unique" interface on that object
		//to determine what type of object it really is...
		if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IRow, &pIUnknown)))
			eSource = eCRow;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IRowset, &pIUnknown)))
			eSource = eCRowset;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_ICommand, &pIUnknown)))
			eSource = eCCommand;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IOpenRowset, &pIUnknown)))
			eSource = eCSession;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IMultipleResults, &pIUnknown)))
			eSource = eCMultipleResults;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_ISourcesRowset, &pIUnknown)))
			eSource = eCEnumerator;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IBindResource, &pIUnknown)))
			eSource = eCBinder;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IDBProperties, &pIUnknown)))
			eSource = eCDataSource;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IDataInitialize, &pIUnknown)))
			eSource = eCServiceComp;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IDBPromptInitialize, &pIUnknown)))
			eSource = eCDataLinks;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_ISequentialStream, &pIUnknown)) || SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IStream, &pIUnknown)))
			eSource = eCStream;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IMDDataset, &pIUnknown)))
			eSource = eCDataset;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IRowPosition, &pIUnknown)))
			eSource = eCRowPosition;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_ITransaction, &pIUnknown)))
			eSource = eCTransaction;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_ITransactionOptions, &pIUnknown)))
			eSource = eCTransactionOptions;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IErrorInfo, &pIUnknown)))
			eSource = eCError;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_ISQLErrorInfo, &pIUnknown)))
			eSource = eCCustomError;
		else if(SUCCEEDED(hr = TRACE_QI(pIUnkObject, IID_IConnectionPoint, &pIUnknown)))
			eSource = eCConnectionPoint;
		else
			eSource = eCUnknown;
	}

	if(SUCCEEDED(hr))
		TRACE_RELEASE(pIUnknown, L"IUnknown");
	return eSource;
}




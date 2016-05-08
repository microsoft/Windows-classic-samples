//////////////////////////////////////////////////////////////////////

//

//	BasicHiPerf.h

//

//	Module:	WMI high performance provider sample code

//

//	This is the skeleton code implementation of a high performance 

//	provider.  This file includes the provider and refresher code

//

//	In this example, we are using a single object scenario.  The 

//	provider provides one type of object called Win32_BasicHiPerf.

//	There is a static number of instances available, specified by

//	NUM_INSTANCES and maintained by the CHiPerfProvider class.

//	When a refresher is created, it has two local caches: one

//	for the instances that it contains, and another for the 

//	enumerators.  Since there is only one class, only one enumerator 

//	should be necessary, but there can be up to MAX_ENUMERATORS 

//	enumerators added to a given refresher.  The members of a refresher

//	are given IDs.  The IDs for the enumerators are essentially array 

//	indecies into the enumerator array.  The IDs for the objects are

//	stored in an array that maps the unique ID to one of the cached 

//	objects.  The enumerator IDs are within the range of 0 to 

//	MAX_ENUMERATORS - 1, and the objects will have IDs equal or greater 

//	than MAX_ENUMERATORS.

//

//	The actual counter data is simulated by the CSampleDataSource 

//	which maintains a set of CSampleInstances.  Periodically, the 

//	CSampleInstance counter values are updated.  The objects in the 

//	refresher are only updated on a refresh.

//

//  History:

//	a-dcrews	12-Jan-99		Created

//	a-dcrews	10-Mar-99		Added data source simulation	

//

//	

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
/////////////////////////////////////////////////////////////////////

#ifndef _HIPERFPROV_H_
#define _HIPERFPROV_H_

#define UNICODE
#define _UNICODE

#include <tchar.h> 
#include <wbemprov.h>

class CRefresher;
class CRefCacheElement;

//////////////////////////////////////////////////////////////
//
//
//	Constants and globals
//
//	
//////////////////////////////////////////////////////////////

#define NUM_OBJECTS				1
#define NUM_INSTANCES			5
#define MAX_INSTANCES			128
#define MAX_ENUMERATORS			10

#define SAMPLE_CLASS			_T("Win32_BasicHiPerf")

#define PROP_DWORD	0x0001L
#define PROP_QWORD	0x0002L
#define PROP_VALUE	0x0004L

//	This enumeration is used to reference the counters.  It  
//	matchs the number of counters in the mof definition of
//	the Win32_BasicHiPerf object.

const enum CounterHandles
{
	ctr1 = 0,
	ctr2,
	ctr3,
	ctr4,
	ctr5,
	NumCtrs
};

//////////////////////////////////////////////////////////////
//
//	CCacheMapEl
//
//	The IDs assigned to objects added to the refresher are 
//	mapped to the actual object cache index using an array
//	of CCacheMapEls.  The generation of unique IDs is controlled
//	by the static member m_lGenID.
//
//////////////////////////////////////////////////////////////

class CCacheMapEl
{
	static long	m_lGenID;

	// The unique ID of the object passed back to the caller
	// =====================================================
	long	m_lUID;

	// The index of the object in the instance cache
	// =============================================
	long	m_lIndex;

public:
	CCacheMapEl(long lIndex) : m_lIndex(lIndex), m_lUID(m_lGenID++)
	{}

	long GetUID() {return m_lUID;}
};

//////////////////////////////////////////////////////////////
//
//	CSampleInstance
//
//	This class is a representation of a real data instance.  
//	The counter values correspond directly to WMI object 
//	parameters.
//
//////////////////////////////////////////////////////////////

class CSampleInstance
{
	// The counter values
	// ==================
	DWORD	m_aCounter[NumCtrs];

	// The lock mechanism
	// ==================
	CRITICAL_SECTION	CS;

public:
	CSampleInstance();
	CSampleInstance(const CSampleInstance &aInst);
	~CSampleInstance();

	DWORD	Lock(bool bLock);

	DWORD	SetCounter(long lCtr, DWORD dwVal);
	DWORD	GetCounter(long lCtr, DWORD* pdwVal);
};

//////////////////////////////////////////////////////////////
//
//	CSampleDataSource
//
//	This class is a representation of a real data source.  
//	Instance data counters are periodically updated, and their 
//	values may be retrieved to update the refresher.
//
//////////////////////////////////////////////////////////////

class CSampleDataSource
{
	// The IWbemObjectAccess counter handles
	// =====================================
	long	m_alHandle[NumCtrs];

	// The array of instances
	// ======================
	CSampleInstance m_aInstance[NUM_INSTANCES];

	// The data source control members (mocks changes in counter values)
	// =================================================================
	HANDLE	m_hThread;

	// The thread termination event
	// ============================
	HANDLE	m_hQuit;


	DWORD	SetHandles(IWbemClassObject* pSampleClass);

	DWORD	Simulate();

	static unsigned __stdcall CSampleDataSource::ThreadEntry(void* pArgs);

public:
	CSampleDataSource();
	virtual ~CSampleDataSource();

	DWORD Initialize(IWbemClassObject* pSampleClass);

	DWORD UpdateInstance(IWbemObjectAccess* pObj);
};

//////////////////////////////////////////////////////////////
//
//	CHiPerfProvider
//
//	The provider maintains a single IWbemClassObject to be used 
//	as a template to spawn instances for the Refresher as well
//	as QueryInstances.  It also maintains the static sample 
//	data source which provides all data to the instances.
//
//////////////////////////////////////////////////////////////

class CHiPerfProvider : public IWbemProviderInit, public IWbemHiPerfProvider
{
	long m_lRef;

	// Our mock data source
	// ====================

	static CSampleDataSource* m_pSampleDS;

	// An instance template (used for QueryInstances)
	// ==============================================

	IWbemClassObject*	m_pTemplate;

public:
	CHiPerfProvider();
	~CHiPerfProvider();

	// Standard COM methods
	// ====================

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	// IWbemProviderInit COM interface
	// ===============================

	STDMETHODIMP Initialize( 
		/* [unique][in] */ LPWSTR wszUser,
		/* [in] */ long lFlags,
		/* [in] */ LPWSTR wszNamespace,
		/* [unique][in] */ LPWSTR wszLocale,
		/* [in] */ IWbemServices __RPC_FAR *pNamespace,
		/* [in] */ IWbemContext __RPC_FAR *pCtx,
		/* [in] */ IWbemProviderInitSink __RPC_FAR *pInitSink );

	// IWbemHiPerfProvider COM interfaces
	// ==================================

	STDMETHODIMP QueryInstances( 
		/* [in] */ IWbemServices __RPC_FAR *pNamespace,
		/* [string][in] */ WCHAR __RPC_FAR *wszClass,
		/* [in] */ long lFlags,
		/* [in] */ IWbemContext __RPC_FAR *pCtx,
		/* [in] */ IWbemObjectSink __RPC_FAR *pSink );
    
	STDMETHODIMP CreateRefresher( 
		/* [in] */ IWbemServices __RPC_FAR *pNamespace,
		/* [in] */ long lFlags,
		/* [out] */ IWbemRefresher __RPC_FAR *__RPC_FAR *ppRefresher );
    
	STDMETHODIMP CreateRefreshableObject( 
		/* [in] */ IWbemServices __RPC_FAR *pNamespace,
		/* [in] */ IWbemObjectAccess __RPC_FAR *pTemplate,
		/* [in] */ IWbemRefresher __RPC_FAR *pRefresher,
		/* [in] */ long lFlags,
		/* [in] */ IWbemContext __RPC_FAR *pContext,
		/* [out] */ IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
		/* [out] */ long __RPC_FAR *plId );
    
	STDMETHODIMP StopRefreshing( 
		/* [in] */ IWbemRefresher __RPC_FAR *pRefresher,
		/* [in] */ long lId,
		/* [in] */ long lFlags );

	STDMETHODIMP CreateRefreshableEnum(
		/* [in] */ IWbemServices* pNamespace,
		/* [in, string] */ LPCWSTR wszClass,
		/* [in] */ IWbemRefresher* pRefresher,
		/* [in] */ long lFlags,
		/* [in] */ IWbemContext* pContext,
		/* [in] */ IWbemHiPerfEnum* pHiPerfEnum,
		/* [out] */ long* plId);

	STDMETHODIMP GetObjects(
        /* [in] */ IWbemServices* pNamespace,
		/* [in] */ long lNumObjects,
		/* [in,size_is(lNumObjects)] */ IWbemObjectAccess** apObj,
        /* [in] */ long lFlags,
        /* [in] */ IWbemContext* pContext);
};


//////////////////////////////////////////////////////////////
//
//	CRefresher
//
//	The refresher maintains an object and an enumerator cache.
//	When an enumerator is added to the refrehser, it is added 
//	to the enumerator cache, and the index of the array is
//	passed back as a unique ID.  The refresher creates a cache
//	of all instances during its initialization.  When an object 
//	is added to the refresher, a mapping to the object is 
//	created between the unique ID and the index of the object
//	in the cache.  This allows the objects to be reused and 
//	facilitates the management of objects that have been added 
//	multiple times.
//
//////////////////////////////////////////////////////////////

class CRefresher : public IWbemRefresher
{
	// COM reference counter
	// =====================

	long	m_lRef;

	// A pointer to the sample data source
	// ===================================

	CSampleDataSource*	m_pDS;

	// The enumerators that have been added to the refresher
	// =====================================================

	IWbemHiPerfEnum*	m_apEnumerator[MAX_ENUMERATORS];

	// The instances that have been added to the refresher
	// ===================================================

	CCacheMapEl*		m_apInstMap[MAX_INSTANCES];
	IWbemObjectAccess*	m_apInstances[NUM_INSTANCES];

	// The parent provider
	// ===================

	CHiPerfProvider* m_pProvider;

public:
	CRefresher(CHiPerfProvider* pProvider, CSampleDataSource* pDS);
	virtual ~CRefresher();

	DWORD Initialize(IWbemClassObject* pSampleClass);

	// Instance management functions
	// =============================

	DWORD AddObject(IWbemObjectAccess *pObj, IWbemObjectAccess **ppReturnObj, long *plId);
	DWORD RemoveObject(long lId);

	// Enumerator management functions
	// ===============================

	DWORD AddEnum(IWbemHiPerfEnum *pHiPerfEnum, long *plId);
	DWORD RemoveEnum(long lId);

	// COM methods
	// ===========

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP Refresh(/* [in] */ long lFlags);
};


#endif // _HIPERFPROV_H_

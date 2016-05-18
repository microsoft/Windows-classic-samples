////////////////////////////////////////////////////////////////////////

//

//	BasicHiPerf.cpp

//

//	Module:	WMI high performance provider sample code

//

//	This is the skeleton code implementation of a high performance 

//	provider.  This file includes the provider and refresher code.

//

//  History:

//

//	

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
////////////////////////////////////////////////////////////////////////
#define _UNICODE
#define UNICODE

#define STRSAFE_NO_DEPRECATE
//#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <objbase.h>
#include "BasicHiPerf.h"
#include <strsafe.h>


//////////////////////////////////////////////////////////////
//
//
//	Global, external and static variables
//
//
//////////////////////////////////////////////////////////////

// This structure quickly defines the properties and types of
// the counters.  It is used for simplicity; actual providers 
// should enumerate the property names and types
// ==========================================================

struct tagProperties
{
	_TCHAR	m_tcsPropertyName[128];
	DWORD	m_dwType;
} g_aCounterProps[] =
{
	_T("Counter1"),	PROP_DWORD,
	_T("Counter2"),	PROP_DWORD,
	_T("Counter3"),	PROP_DWORD,
	_T("Counter4"),	PROP_DWORD,
	_T("Counter5"),	PROP_DWORD
};

// The access handle for the object's ID
// =====================================

long g_hID;

// The COM object counter (declared in server.cpp)
// ===============================================

extern long g_lObjects;	

// The mock data source (declared as static in CHiPerfProvider)
// ============================================================

CSampleDataSource* CHiPerfProvider::m_pSampleDS = NULL;

// The unique ID generator in CCacheMap - values from 0 to 
// (MAX_ENUMERATORS - 1) are reserved for the enumerators
// =======================================================

long CCacheMapEl::m_lGenID = MAX_ENUMERATORS;


//////////////////////////////////////////////////////////////
//
//
//	Helper Functions
//
//
//////////////////////////////////////////////////////////////

HRESULT CloneAccess(IWbemObjectAccess* pOriginal, IWbemObjectAccess** ppClone)
//////////////////////////////////////////////////////////////
//
//	Returns a IWbemObjectAccess clone of an IWbemObjectAccess
//	object.
//
//	Parameters:
//		pOriginal	- the original object
//		ppClone		- the cloned object
//
//////////////////////////////////////////////////////////////
//ok
{
	HRESULT hRes = WBEM_NO_ERROR;

	IWbemClassObject*	pOurClassObj = NULL;		
	IWbemClassObject*	pClonedClassObj = NULL;
	IWbemObjectAccess*	pClonedAccessObj = NULL;

	// Get the IWbemClassObject interface for the object
	// =================================================

	hRes = pOriginal->QueryInterface(IID_IWbemClassObject, (PVOID*)&pOurClassObj);
	if (FAILED(hRes))
		return hRes;

	// Clone the object
	// ================

	hRes = pOurClassObj->Clone(&pClonedClassObj);
	pOurClassObj->Release();
	if (FAILED(hRes))
		return hRes;

	// Get the IWbemObjectAccess interface for the cloned object
	// =========================================================

	hRes = pClonedClassObj->QueryInterface(IID_IWbemObjectAccess, (PVOID*)&pClonedAccessObj);
	pClonedClassObj->Release();
	if (FAILED(hRes))
		return hRes;

	// Copy the cloned object into the returned parameter (refcount == 1)
	// ==================================================================

	*ppClone = pClonedAccessObj;

	return WBEM_NO_ERROR;
}




//////////////////////////////////////////////////////////////
// 
//	Name:	GetCurrentImpersonationLevel
//
//	
//  Description:
//
//			Get COM impersonation level of caller.	
//
//////////////////////////////////////////////////////////////

DWORD GetCurrentImpersonationLevel ()
{
	DWORD t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;

    HANDLE t_ThreadToken = NULL ;

    BOOL t_Status = OpenThreadToken (

		GetCurrentThread() ,
		TOKEN_QUERY,
		TRUE,
		&t_ThreadToken
	) ;

    if ( t_Status )
    {
		SECURITY_IMPERSONATION_LEVEL t_Level = SecurityAnonymous ;
		DWORD t_Returned = 0 ;

		t_Status = GetTokenInformation (

			t_ThreadToken ,
			TokenImpersonationLevel ,
			& t_Level ,
			sizeof ( SECURITY_IMPERSONATION_LEVEL ) ,
			& t_Returned
		) ;

		CloseHandle ( t_ThreadToken ) ;

		if ( t_Status == FALSE )
		{
			t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
		}
		else
		{
			switch ( t_Level )
			{
				case SecurityAnonymous:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
				}
				break ;

				case SecurityIdentification:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_IDENTIFY ;
				}
				break ;

				case SecurityImpersonation:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE ;
				}
				break ;

				case SecurityDelegation:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_DELEGATE ;
				}
				break ;

				default:
				{
					t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
				}
				break ;
			}
		}
	}
	else
	{
        ULONG t_LastError = GetLastError () ;

        if ( t_LastError == ERROR_NO_IMPERSONATION_TOKEN || t_LastError == ERROR_NO_TOKEN )
        {
            t_ImpersonationLevel = RPC_C_IMP_LEVEL_DELEGATE ;
        }
        else 
		{
			if ( t_LastError == ERROR_CANT_OPEN_ANONYMOUS )
			{
				t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
			}
			else
			{
				t_ImpersonationLevel = RPC_C_IMP_LEVEL_ANONYMOUS ;
			}
		}
    }

	return t_ImpersonationLevel ;
}



//////////////////////////////////////////////////////////////
//
//
//	CHiPerfProvider
//
//
//////////////////////////////////////////////////////////////

CHiPerfProvider::CHiPerfProvider() : m_lRef(0), m_pTemplate(NULL)
//////////////////////////////////////////////////////////////
//
//	Constructor
//
//////////////////////////////////////////////////////////////
//ok
{
	// Increment the global COM object counter
	// =======================================

	InterlockedIncrement(&g_lObjects);
}

CHiPerfProvider::~CHiPerfProvider()
//////////////////////////////////////////////////////////////
//
//	Destructor
//
//////////////////////////////////////////////////////////////
//ok
{
	long lObjCount = 0;

    // Release the template
	// ====================

	if (NULL != m_pTemplate)
            m_pTemplate->Release();

	// Decrement the global COM object counter
	// =======================================

	lObjCount = InterlockedDecrement(&g_lObjects);

	// If this is the last object, delete our data source
	// ==================================================

	if ((0 == lObjCount) && (NULL != m_pSampleDS))
	{
		delete m_pSampleDS;
		m_pSampleDS = NULL;
	}
}

//////////////////////////////////////////////////////////////
//
//					COM methods
//
//////////////////////////////////////////////////////////////

STDMETHODIMP CHiPerfProvider::QueryInterface(REFIID riid, void** ppv)
//////////////////////////////////////////////////////////////
//
//	Standard QueryInterface
//
//	Parameters:
//		riid	- the ID of the requested interface
//		ppv		- a pointer to the interface pointer
//
//////////////////////////////////////////////////////////////
//ok
{
    if(riid == IID_IUnknown)
        *ppv = (LPVOID)(IUnknown*)(IWbemProviderInit*)this;
    else if(riid == IID_IWbemProviderInit)
        *ppv = (LPVOID)(IWbemProviderInit*)this;
	else if (riid == IID_IWbemHiPerfProvider)
		*ppv = (LPVOID)(IWbemHiPerfProvider*)this;
	else return E_NOINTERFACE;

	((IUnknown*)*ppv)->AddRef();
	return WBEM_NO_ERROR;
}

STDMETHODIMP_(ULONG) CHiPerfProvider::AddRef()
//////////////////////////////////////////////////////////////
//
//	Standard COM AddRef
//
//////////////////////////////////////////////////////////////
//ok
{
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CHiPerfProvider::Release()
//////////////////////////////////////////////////////////////
//
//	Standard COM Release
//
//////////////////////////////////////////////////////////////
//ok
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;

    return lRef;
}

STDMETHODIMP CHiPerfProvider::Initialize( 
    /* [unique][in] */  LPWSTR wszUser,
    /* [in] */          long lFlags,
    /* [in] */          LPWSTR wszNamespace,
    /* [unique][in] */  LPWSTR wszLocale,
    /* [in] */          IWbemServices __RPC_FAR *pNamespace,
    /* [in] */          IWbemContext __RPC_FAR *pCtx,
    /* [in] */          IWbemProviderInitSink __RPC_FAR *pInitSink)
//////////////////////////////////////////////////////////////////////
//
//  Called once during startup for any one-time initialization.  The 
//	final call to Release() is for any cleanup.
//	
//	The parameters indicate to the provider which namespace it is being 
//	invoked for and which User.  It also supplies a back pointer to 
//	WINMGMT so that class definitions can be retrieved.
//
//	Initialize will create a single template object that can be used 
//	by the provider to spawn instances for QueryInstances.  It will 
//	also initialize our mock data source and set the global ID access 
//	handle.
//	
//	Parameters:
//		wszUser			- The current user.
//		lFlags			- Reserved.
//		wszNamespace	- The namespace for which we are being activated.
//		wszLocale		- The locale under which we are to be running.
//		pNamespace		- An active pointer back into the current namespace
//							from which we can retrieve schema objects.
//		pCtx			- The user's context object.  We simply reuse this
//							during any reentrant operations into WINMGMT.
//		pInitSink		- The sink to which we indicate our readiness.
//
//////////////////////////////////////////////////////////////////////
//ok
{
	HRESULT hRes = WBEM_NO_ERROR;

    IWbemObjectAccess *pAccess = NULL;		
	BSTR strObject = NULL;
	BSTR strPropName = NULL;

	if (wszNamespace == 0 || pNamespace == 0 || pInitSink == 0)
	{
	    pInitSink->SetStatus(WBEM_E_FAILED , 0);
	    return WBEM_NO_ERROR;
	}

    // Get a sample class object for initialization purposes
	// =====================================================

	strObject = SysAllocString(SAMPLE_CLASS);
	if (strObject == NULL)
	{
	    pInitSink->SetStatus(WBEM_E_OUT_OF_MEMORY , 0);
	    return WBEM_NO_ERROR;
	}

	hRes = pNamespace->GetObject(strObject, 0, pCtx, &m_pTemplate, 0);

	SysFreeString(strObject);

    if (FAILED(hRes))
	{
	    pInitSink->SetStatus(WBEM_E_FAILED , 0);
	    return WBEM_NO_ERROR;
	}


	// Initialize the datasource.  If the static member 
	// is not null, then it has already been setup.
	// ================================================

	if (NULL == m_pSampleDS)
	{
		m_pSampleDS = new CSampleDataSource;

		if (NULL == m_pSampleDS)
		{
			pInitSink->SetStatus(WBEM_E_FAILED , 0);
			return WBEM_NO_ERROR;
		}

		m_pSampleDS->Initialize(m_pTemplate);
	}

	// Initialize the global access handle for the object ID.  
	// If it is not null, then it has already been setup.
	// ======================================================

	if (NULL == g_hID)
	{
		// Get the IWbemObjectAccess interface to the object
		// =================================================

		hRes = m_pTemplate->QueryInterface(IID_IWbemObjectAccess, (PVOID*)&pAccess);

		if (FAILED(hRes))
			return hRes;
		
		// Get the name property access handle
		// ===================================

		strPropName = SysAllocString(L"Name");
		if (strPropName == NULL)
		{
			pInitSink->SetStatus(WBEM_E_OUT_OF_MEMORY , 0);
			return WBEM_NO_ERROR;
		}

		hRes = pAccess->GetPropertyHandle(strPropName, 0, &g_hID);

		pAccess->Release();
		SysFreeString(strPropName);

		if (FAILED(hRes))
		{
			pInitSink->SetStatus(WBEM_E_FAILED , 0);
			return WBEM_NO_ERROR;
		}
}

    // We now have all the instances ready to go and the name handle 
	// stored.  Tell WINMGMT that we're ready to start 'providing'
	// =============================================================
	
    pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);

    return WBEM_NO_ERROR;
}

STDMETHODIMP CHiPerfProvider::QueryInstances( 
    /* [in] */          IWbemServices __RPC_FAR *pNamespace,
    /* [string][in] */  WCHAR __RPC_FAR *wszClass,
    /* [in] */          long lFlags,
    /* [in] */          IWbemContext __RPC_FAR *pCtx,
    /* [in] */          IWbemObjectSink __RPC_FAR *pSink )
//////////////////////////////////////////////////////////////////////
//
//  Called whenever a complete, fresh list of instances for a given
//  class is required.   The objects are constructed and sent back to the
//  caller through the sink.  The sink can be used in-line as here, or
//  the call can return and a separate thread could be used to deliver
//  the instances to the sink.
//
//  Parameters:
//		pNamespace		- A pointer to the relevant namespace.  This
//							should not be AddRef'ed.
//		wszClass		- The class name for which instances are required.
//		lFlags			- Reserved.
//		pCtx			- The user-supplied context (not used here).
//		pSink			- The sink to which to deliver the objects.  The objects
//							can be delivered synchronously through the duration
//							of this call or asynchronously (assuming we
//							had a separate thread).  A IWbemObjectSink::SetStatus
//							call is required at the end of the sequence.
//
//////////////////////////////////////////////////////////////////////
//ok
{
	HRESULT hRes = WBEM_NO_ERROR;

	//Impersonate the client
	hRes = CoImpersonateClient () ;
	if ( FAILED ( hRes ) && hRes != RPC_E_CALL_COMPLETE)
	{
		return hRes ;
	}	

	//	Check to see if call is at lower than RPC_C_IMP_LEVEL_IMPERSONATE level. If that's the case,
	//  the provider will not be able to impersonate the client to access the protected resources.	

	DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
	if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
	{
		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		hRes = WBEM_E_ACCESS_DENIED;
		return hRes ;
	}

	IWbemClassObject*	pObjectCopy = NULL;
	IWbemObjectAccess*	pAccessCopy = NULL;

    if (pNamespace == 0 || wszClass == 0 || pSink == 0)
        return WBEM_E_INVALID_PARAMETER;

	// Loop through all instances, indicating an updated version
	// to the object sink
	// =========================================================

    for (int i = 0; i < NUM_INSTANCES; i++)
    {
		// Create a new instance from the template
		// =======================================

		hRes = m_pTemplate->SpawnInstance(0, &pObjectCopy);
		if (FAILED(hRes))
			break;

		// Obtain the IWbemObjectAccess interface
		// ======================================

		hRes = pObjectCopy->QueryInterface(IID_IWbemObjectAccess, (PVOID*)&pAccessCopy);
		if (FAILED(hRes))
		{
			pObjectCopy->Release();
			break;
		}

		// Set the ID
		// ==========

		WCHAR wszName[20];
		StringCbPrintfW(wszName, sizeof(wszName), L"%i", i);

		pAccessCopy->WritePropertyValue(g_hID, (long)(wcslen(wszName)+1)*sizeof(WCHAR), (byte *)wszName);

		// Initialize the counters
		// =======================

		hRes = m_pSampleDS->UpdateInstance(pAccessCopy);

		// Release the IWbemObjectAccess handle (values are maintained in IWbemClassObject interface)
		// ==========================================================================================

		pAccessCopy->Release();

		if (FAILED(hRes))
		{
			pObjectCopy->Release();
			break;
		}

        // Send a copy back to the caller
		// ==============================

        pSink->Indicate(1, &pObjectCopy);

        pObjectCopy->Release();    // Don't need this any more
    }
    
    // Tell WINMGMT we are all finished supplying objects
	// ==================================================

    pSink->SetStatus(0, WBEM_NO_ERROR, 0, 0);

    return WBEM_NO_ERROR;
}    

STDMETHODIMP CHiPerfProvider::CreateRefresher( 
     /* [in] */ IWbemServices __RPC_FAR *pNamespace,
     /* [in] */ long lFlags,
     /* [out] */ IWbemRefresher __RPC_FAR *__RPC_FAR *ppRefresher )
//////////////////////////////////////////////////////////////////////
//
//  Called whenever a new refresher is needed by the client.
//
//  Parameters:
//		pNamespace		- A pointer to the relevant namespace.  Not used.
//		lFlags			- Reserved.
//		ppRefresher		- Receives the requested refresher.
//
//////////////////////////////////////////////////////////////////////
//ok
{
	//Impersonate the client
	HRESULT hRes = CoImpersonateClient () ;
	if ( FAILED ( hRes ) && hRes != RPC_E_CALL_COMPLETE)
	{
		return hRes ;
	}	

	//	Check to see if call is at lower than RPC_C_IMP_LEVEL_IMPERSONATE level. If that's the case,
	//  the provider will not be able to impersonate the client to access the protected resources.	

	DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
	if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
	{
		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		hRes = WBEM_E_ACCESS_DENIED;
		return hRes ;
	}

    if (pNamespace == 0 || ppRefresher == 0)
        return WBEM_E_INVALID_PARAMETER;

    // Construct and initialize a new empty refresher
	// ==============================================

	CRefresher* pNewRefresher = new CRefresher(this, m_pSampleDS);
	if (pNewRefresher == NULL)
		return WBEM_E_OUT_OF_MEMORY;

	pNewRefresher->Initialize(m_pTemplate);

    // Follow COM rules and AddRef() the thing before sending it back
	// ==============================================================

    pNewRefresher->AddRef();
    *ppRefresher = pNewRefresher;
    
    return WBEM_NO_ERROR;
}

STDMETHODIMP CHiPerfProvider::CreateRefreshableObject( 
    /* [in] */ IWbemServices __RPC_FAR *pNamespace,
    /* [in] */ IWbemObjectAccess __RPC_FAR *pTemplate,
    /* [in] */ IWbemRefresher __RPC_FAR *pRefresher,
    /* [in] */ long lFlags,
    /* [in] */ IWbemContext __RPC_FAR *pContext,
    /* [out] */ IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
    /* [out] */ long __RPC_FAR *plId )
//////////////////////////////////////////////////////////////////////
//
//  Called whenever a user wants to include an object in a refresher.
//
//	Note that the object returned in ppRefreshable is a clone of the 
//	actual instance maintained by the provider.  If refreshers shared
//	a copy of the same instance, then a refresh call on one of the 
//	refreshers would impact the state of both refreshers.  This would 
//	break the refresher rules.	Instances in a refresher are only 
//	allowed to be updated when 'Refresh' is called.
//     
//  Parameters:
//		pNamespace		- A pointer to the relevant namespace in WINMGMT.
//		pTemplate		- A pointer to a copy of the object which is to be
//							added.  This object itself cannot be used, as
//							it not owned locally.        
//		pRefresher		- The refresher to which to add the object.
//		lFlags			- Not used.
//		pContext		- Not used here.
//		ppRefreshable	- A pointer to the internal object which was added
//							to the refresher.
//		plId			- The Object Id (for identification during removal).        
//
//////////////////////////////////////////////////////////////////////
//ok
{
   
	HRESULT hRes = WBEM_NO_ERROR;

	//Impersonate the client
	hRes = CoImpersonateClient () ;
	if ( FAILED ( hRes ) && hRes != RPC_E_CALL_COMPLETE)
	{
		return hRes ;
	}	

	//	Check to see if call is at lower than RPC_C_IMP_LEVEL_IMPERSONATE level. If that's the case,
	//  the provider will not be able to impersonate the client to access the protected resources.	

	DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
	if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
	{
		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		hRes = WBEM_E_ACCESS_DENIED;
		return hRes ;
	}

 // Find out which instance is being requested for addition
	// =======================================================

    if (pNamespace == 0 || pTemplate == 0 || pRefresher == 0)
        return WBEM_E_INVALID_PARAMETER;

    // The refresher being supplied by the caller is actually
    // one of our own refreshers, so a simple cast is convenient
    // so that we can access private members.
	// =========================================================

	CRefresher *pOurRefresher = (CRefresher *) pRefresher;

	// Add the object to the refresher. The ID is set by AddObject
	// ===========================================================

    hRes = pOurRefresher->AddObject(pTemplate, ppRefreshable, plId);

	if (FAILED(hRes))
		return hRes;

    return WBEM_NO_ERROR;
}
   
STDMETHODIMP CHiPerfProvider::CreateRefreshableEnum( 
	/* [in] */ IWbemServices* pNamespace,
	/* [in, string] */ LPCWSTR wszClass,
	/* [in] */ IWbemRefresher* pRefresher,
	/* [in] */ long lFlags,
	/* [in] */ IWbemContext* pContext,
	/* [in] */ IWbemHiPerfEnum* pHiPerfEnum,
	/* [out] */ long* plId )
//////////////////////////////////////////////////////////////////////
//
//  Called when an enumerator is being added to a refresher.  The 
//	enumerator will obtain a fresh set of instances of the specified 
//	class every time that refresh is called.
//     
//	Since this example provider only provides one class, then any 
//	enumerator in this refresher will only return instances from this 
//	class.  In the case where there is more than one class, then 
//	wszClass must be examined to determine which class the enumerator 
//	is being assigned.
//
//  Parameters:
//		pNamespace		- A pointer to the relevant namespace.  
//		wszClass		- The class name for the requested enumerator.
//		pRefresher		- The refresher object for which we will add 
//							the enumerator
//		lFlags			- Reserved.
//		pContext		- Not used here.
//		pHiPerfEnum		- The enumerator to add to the refresher.
//		plId			- A provider specified ID for the enumerator.
//
//////////////////////////////////////////////////////////////////////
//ok
{

	//Impersonate the client
	HRESULT hr = CoImpersonateClient () ;
	if ( FAILED ( hr ) && hr != RPC_E_CALL_COMPLETE)
	{
		return hr ;
	}	

	//	Check to see if call is at lower than RPC_C_IMP_LEVEL_IMPERSONATE level. If that's the case,
	//  the provider will not be able to impersonate the client to access the protected resources.	

	DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
	if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
	{
		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		hr = WBEM_E_ACCESS_DENIED;
		return hr ;
	}

    // The refresher being supplied by the caller is actually
    // one of our own refreshers, so a simple cast is convenient
    // so that we can access private members.

	CRefresher *pOurRefresher = (CRefresher *) pRefresher;

	// Add the enumerator to the refresher  
	// The ID is generated by AddEnum.
	// ===================================

	hr = pOurRefresher->AddEnum(pHiPerfEnum, plId);
	return hr; 
}

STDMETHODIMP CHiPerfProvider::StopRefreshing( 
    /* [in] */ IWbemRefresher __RPC_FAR *pRefresher,
    /* [in] */ long lId,
    /* [in] */ long lFlags )
//////////////////////////////////////////////////////////////////////
//
//  Called whenever a user wants to remove an object from a refresher.
//     
//  Parameters:
//		pRefresher			- The refresher object from which we are to 
//								remove the perf object.
//		lId					- The ID of the object.
//		lFlags				- Not used.
//  
//////////////////////////////////////////////////////////////////////
//ok
{
	HRESULT hRes = WBEM_NO_ERROR;

	//Impersonate the client
	hRes = CoImpersonateClient () ;
	if ( FAILED ( hRes ) && hRes != RPC_E_CALL_COMPLETE)
	{
		return hRes ;
	}	

	//	Check to see if call is at lower than RPC_C_IMP_LEVEL_IMPERSONATE level. If that's the case,
	//  the provider will not be able to impersonate the client to access the protected resources.	

	DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
	if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
	{
		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		hRes = WBEM_E_ACCESS_DENIED;
		return hRes ;
	}

    // The refresher being supplied by the caller is actually
    // one of our own refreshers, so a simple cast is convenient
    // so that we can access private members.
	// =========================================================

    CRefresher *pOurRefresher = (CRefresher *) pRefresher;

	// An ID that is less than MAX_ENUMERATORS is an enumerator ID.
	// Everything else is an object ID
	// ============================================================

	if (lId < MAX_ENUMERATORS)
		hRes = pOurRefresher->RemoveEnum(lId);
	else
		hRes = pOurRefresher->RemoveObject(lId);

	return hRes;
}

STDMETHODIMP CHiPerfProvider::GetObjects( 
    /* [in] */ IWbemServices* pNamespace,
	/* [in] */ long lNumObjects,
	/* [in,size_is(lNumObjects)] */ IWbemObjectAccess** apObj,
    /* [in] */ long lFlags,
    /* [in] */ IWbemContext* pContext)
//////////////////////////////////////////////////////////////////////
//
//  Called when a request is made to provide all instances currently 
//	being managed by the provider in the specified namespace.
//     
//  Parameters:
//		pNamespace		- A pointer to the relevant namespace.  
//		lNumObjects		- The number of instances being returned.
//		apObj			- The array of instances being returned.
//		lFlags			- Reserved.
//		pContext		- Not used here.
//
//////////////////////////////////////////////////////////////////////
//ok
{
	HRESULT hRes = WBEM_NO_ERROR;

	//Impersonate the client
	hRes = CoImpersonateClient () ;
	if ( FAILED ( hRes ) && hRes != RPC_E_CALL_COMPLETE)
	{
		return hRes ;
	}	

	//	Check to see if call is at lower than RPC_C_IMP_LEVEL_IMPERSONATE level. If that's the case,
	//  the provider will not be able to impersonate the client to access the protected resources.	

	DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
	if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
	{
		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		hRes = WBEM_E_ACCESS_DENIED;
		return hRes ;
	}

	int	nIndex = 0;
	DWORD dwID = 0;

	IWbemObjectAccess*	pCurrObj;

	// Cycle through the list of objects, updating each one
	// ====================================================

	for (nIndex = 0; nIndex < lNumObjects; nIndex++)
	{
		pCurrObj = apObj[nIndex];

		hRes = m_pSampleDS->UpdateInstance(pCurrObj);
		if (FAILED(hRes))
			break;
	}

	return hRes;
}


//////////////////////////////////////////////////////////////
//
//
//	CRefresher
//
//
//////////////////////////////////////////////////////////////

CRefresher::CRefresher(CHiPerfProvider* pProvider, CSampleDataSource* pDS) : m_lRef(0), m_pDS(pDS)
//////////////////////////////////////////////////////////////////////
//
//	Constructor
//
//////////////////////////////////////////////////////////////////////
//ok
{
	int nIndex = 0;

	// Retain a copy of the provider
	// =============================

	m_pProvider = pProvider;
	if (m_pProvider)
		m_pProvider->AddRef();

	// Initialize the instance map
	// ===========================

	for (nIndex = 0; nIndex < MAX_INSTANCES; nIndex++)
		m_apInstMap[nIndex] = NULL;

	// Initialize the instance cache
	// =============================

	for (nIndex = 0; nIndex < NUM_INSTANCES; nIndex++)
		m_apInstances[nIndex] = NULL;

	// Initialize the enumerator cache
	// ===============================

	for (nIndex = 0; nIndex < MAX_ENUMERATORS; nIndex++)
		m_apEnumerator[nIndex] = NULL;

	// Increment the global COM object counter
	// =======================================

	InterlockedIncrement(&g_lObjects);
}

CRefresher::~CRefresher()
//////////////////////////////////////////////////////////////////////
//
//	Destructor
//
//////////////////////////////////////////////////////////////////////
//ok
{
	int nIndex = 0;

	// Delete the instance cache
	// =========================

	for (nIndex = 0; nIndex < MAX_INSTANCES; nIndex++)
	{
		if (NULL != m_apInstMap[nIndex])
			delete m_apInstMap[nIndex];
	}

    // Release the cached IWbemObjectAccess instances
	// ==============================================

    for (nIndex = 0; nIndex < NUM_INSTANCES; nIndex++)
    {
        if (NULL != m_apInstances[nIndex])
            m_apInstances[nIndex]->Release();
    }            

	// Release all of the enumerators
	// ==============================

	for (nIndex = 0; nIndex < MAX_ENUMERATORS; nIndex++)
	{
		if (NULL != m_apEnumerator[nIndex])
			m_apEnumerator[nIndex]->Release();
	}

	// Release the provider
	// ====================

	if (m_pProvider)
		m_pProvider->Release();

	// Decrement the global COM object counter
	// =======================================

	InterlockedDecrement(&g_lObjects);
}

DWORD CRefresher::Initialize(IWbemClassObject* pSampleClass)
//////////////////////////////////////////////////////////////////////
//
//  Initializes the instances cache.
//     
//  Parameters:
//		pSampleClass	- A template of the WMI object from which we 
//							will create our instances
//
//////////////////////////////////////////////////////////////////////
//ok
{
	DWORD dwRes = WBEM_NO_ERROR;

	int nIndex = 0;

    // Precreate and initialize 5 instances of our sample class, and store 
	// them in the m_apInstances array as IWbemObjectAccess pointers
	// =====================================================================

    for (DWORD dwIndex = 0; dwIndex < NUM_INSTANCES; dwIndex++)
    {
		IWbemClassObject*	pClone = NULL;
		IWbemObjectAccess*	pAccess = NULL;

		// Create the instance
		// ===================

		dwRes = pSampleClass->SpawnInstance(0, &pClone);
		if (FAILED(dwRes))
			return dwRes;

		// Get the IWbemObjectAccess interface
		// ===================================

        dwRes = pClone->QueryInterface(IID_IWbemObjectAccess, (LPVOID *)&pAccess);
        pClone->Release();

		if (FAILED(dwRes))
			return dwRes;

		// Set the ID
		// ==========

		WCHAR wszName[20];
		StringCbPrintfW(wszName, sizeof(wszName), L"%i", dwIndex);
		dwRes = pAccess->WritePropertyValue(g_hID, (long)(wcslen(wszName)+1)*sizeof(WCHAR), (byte *)wszName);
		if (FAILED(dwRes))
		{
			pAccess->Release();
			return dwRes;
		}

		// Initialize the counters
		// =======================

		dwRes = m_pDS->UpdateInstance(pAccess);
		if (FAILED(dwRes))
		{
			pAccess->Release();
			return dwRes;
		}

		// Add to the instance array (refcount == 1)
		// =========================================

        m_apInstances[dwIndex] = pAccess;
    }

	return WBEM_NO_ERROR;
}

DWORD CRefresher::AddObject(IWbemObjectAccess *pObj, IWbemObjectAccess **ppReturnObj, long *plId)
//////////////////////////////////////////////////////////////////////
//
//  Adds an object to the refresher.   This is a private mechanism
//  used by CHiPerfProvider and not part of the COM interface.
//
//	All of the objects are maintained by the m_apInstances array.  This
//	is set up during a call to Initialize.  Multiple additions of the same
//	object to a refresher is legal, so to optimize the refresh, the 
//	same object is passed back but with a different ID.  The ID:object
//	relationships are maintained by m_apInstMap.
//
//	Parameters:
//		pObj		- the object to add
//		lID			- the ID
//
//////////////////////////////////////////////////////////////////////
//ok
{
	DWORD	dwRes	= WBEM_NO_ERROR;
	
	DWORD	dwID	= 0;
	int		nIndex	= 0;
	bool	bDone	= false;

	if (NULL == pObj)
        return DWORD(WBEM_E_INVALID_PARAMETER);

	// Grab the instance ID
	// ====================
	
	WCHAR wszName[20];
	long lNumBytes = 0;
	dwRes = pObj->ReadPropertyValue(g_hID, sizeof(wszName), &lNumBytes, (byte *)wszName);
	if (FAILED(dwRes))
		return dwRes;
	dwID = _wtoi(wszName);

	// Search the map array for an empty spot
	// ======================================

	for (nIndex = 0; (nIndex < MAX_INSTANCES) && !bDone; nIndex++)
	{
		if (NULL == m_apInstMap[nIndex])
		{
			// Create a new cache element
			// ==========================

			CCacheMapEl* pEl = new CCacheMapEl(dwID);
			if (pEl == NULL)
				return DWORD(WBEM_E_OUT_OF_MEMORY);

			// Set the return parameters
			// =========================

			*plId = pEl->GetUID();
			*ppReturnObj = m_apInstances[dwID];
			(*ppReturnObj)->AddRef();

			// Set the cache element
			// =====================

			m_apInstMap[nIndex] = pEl;

			bDone = true;
		}
	}

	// If array was full, report an error
	// ==================================

	if (!bDone)
		dwRes = DWORD(WBEM_E_OUT_OF_MEMORY);

	return dwRes;
}

DWORD CRefresher::RemoveObject(long lId)
//////////////////////////////////////////////////////////////////////
//
//  Removes an object from the refresher.  This is a private mechanism 
//	used by CHiPerfProvider and not part of the COM interface.
//
//  Removes an object from the refresher by ID.   Use a simple linear
//	search to locate the unique ID in the m_apInstMap.  Leave the 
//	cache instance intact until the refrehser is destroyed.
//
//	Parameters:
//		plID		- the unique ID
//
//////////////////////////////////////////////////////////////////////
//ok
{
	DWORD	dwRes = DWORD(WBEM_E_NOT_FOUND);

	int		nIndex = 0;
	bool	bDone = false;

	// Loop through the map array looking of the ID
	// ============================================

	for (nIndex = 0; (nIndex < MAX_INSTANCES) && !bDone; nIndex++)
	{
		CCacheMapEl* pEl = m_apInstMap[nIndex];

		if (NULL != pEl)
		{
			if (pEl->GetUID() == lId)
			{
				// Found!  So remove map element
				// =============================

				delete pEl;
				m_apInstMap[nIndex] = NULL;

				dwRes = WBEM_NO_ERROR;

				break;
			}
		}
	}

	return dwRes;
}

DWORD CRefresher::AddEnum(IWbemHiPerfEnum *pHiPerfEnum, long *plId)
//////////////////////////////////////////////////////////////////////
//
//  Adds an enumerator to the refresher.   This is a private mechanism
//  used by CHiPerfProvider and not part of the COM interface.
//
//  The ID we return for future identification is simply
//  the array index.
//
//	Note that the the refresher uses an array of enumerators.  For a 
//	hiperf provider that only supports one class, having more than one 
//	enumerator does not really make sense since a single enumerator will
//	manage all instances of a single class for a given refresher.  
//	NUM_ENUMERATORS may be set to reflect the number of classes if the 
//	proper class / enumerator management code is added.  The implementation
//	is up to the provider.
//
//	Parameters:
//		pHiPerfEnum	- the enumerator
//		plID		- the ID
//
//////////////////////////////////////////////////////////////////////
//ok
{
	DWORD dwRes = DWORD(WBEM_E_FAILED);

	if (NULL == pHiPerfEnum)
        return DWORD(WBEM_E_INVALID_PARAMETER);

	// Cycle through the enumerator array, and look for an empty slot
	// ==============================================================

	for (int nIndex = 0; nIndex < MAX_ENUMERATORS; nIndex++)
	{
		if (NULL == m_apEnumerator[nIndex])
		{
			// Add all the objects at once into the enumerator
			// ===============================================

			long alIDs[NUM_INSTANCES];

			for ( int nInst = 0; nInst < NUM_INSTANCES; nInst++ )
				alIDs[nInst] = nInst;

			dwRes = pHiPerfEnum->AddObjects( 0L, NUM_INSTANCES, alIDs, m_apInstances );
			if (FAILED(dwRes))
				return dwRes;

			// Add the enumerator to the refresher (released by the refresher's destructor)
			// ============================================================================

			m_apEnumerator[nIndex] = pHiPerfEnum;
			pHiPerfEnum->AddRef();

			*plId = nIndex;

			dwRes = WBEM_NO_ERROR;
			break;
		}
	}
	return dwRes;
}

DWORD CRefresher::RemoveEnum(long lId)
//////////////////////////////////////////////////////////////////////
//
//  Removes an enumerator from the refresher.  This is a private mechanism 
//	used by CHiPerfProvider and not part of the COM interface.
//
//  Removes an enumerator from the refresher by ID.   In our case, the ID
//  is actually the array index we used internally, so it is simple
//  to locate and remove the object.
//
//	Parameters:
//		plID		- the ID
//
//////////////////////////////////////////////////////////////////////
//ok
{
	// Verify the element
	// ==================

	if (NULL != m_apEnumerator[lId])
	{
		// And remove the enumerator
		// =========================

		m_apEnumerator[lId]->Release();
		m_apEnumerator[lId] = NULL;

		return WBEM_NO_ERROR;
	}
	return DWORD(WBEM_E_FAILED);
}

STDMETHODIMP CRefresher::QueryInterface(REFIID riid, void** ppv)
//////////////////////////////////////////////////////////////////////
//
//	Standard COM QueryInterface
//
//////////////////////////////////////////////////////////////////////
//ok
{
    if (riid == IID_IUnknown)
        *ppv = (LPVOID)(IUnknown*)this;
	else if (riid == IID_IWbemRefresher)
		*ppv = (LPVOID)(IWbemRefresher*)this;
    else return E_NOINTERFACE;

   	((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CRefresher::AddRef()
//////////////////////////////////////////////////////////////////////
//	
//	Standard COM AddRef
//
//////////////////////////////////////////////////////////////////////
//ok
{
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CRefresher::Release()
//////////////////////////////////////////////////////////////////////
//
//	Standard COM Release
//
//////////////////////////////////////////////////////////////////////
//ok
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;

    return lRef;
}

STDMETHODIMP CRefresher::Refresh(/* [in] */ long lFlags)
//////////////////////////////////////////////////////////////////////
//
//  Executed to refresh a set of instances bound to the particular 
//  refresher.
//
//	In most situations the instance data, such as counter values and 
//	the set of current instances within any existing enumerators, would 
//	be updated whenever Refresh was called.  For this example, we are 
//	going to or data source to update the counter data.  Since we are 
//	using a fixed set of instances, we are not adding or removing anything
//	from the enumerator.
//
//	Parameters:
//		lFlags	- not used
//
//////////////////////////////////////////////////////////////////////
//ok
{

	HRESULT	hRes = WBEM_NO_ERROR;
	
	//Impersonate the client
	hRes = CoImpersonateClient () ;
	if ( FAILED ( hRes ) && hRes != RPC_E_CALL_COMPLETE)
	{
		return hRes ;
	}	

	//	Check to see if call is at lower than RPC_C_IMP_LEVEL_IMPERSONATE level. If that's the case,
	//  the provider will not be able to impersonate the client to access the protected resources.	

	DWORD t_CurrentImpersonationLevel = GetCurrentImpersonationLevel () ;
	if ( t_CurrentImpersonationLevel < RPC_C_IMP_LEVEL_IMPERSONATE )
	{
		//	Revert before we perform any operations	
		CoRevertToSelf () ;

		hRes = WBEM_E_ACCESS_DENIED;
		return hRes ;
	}


	IWbemObjectAccess* pObj;
	DWORD	dwVal = 0;
	long	hCtr = 0;

	// A simple loop that iterates through all instances that have been
	// added to the refresher, and updates their counter values
	// ================================================================

	for (int nInst = 0; nInst < NUM_INSTANCES; nInst++)
	{
		pObj = m_apInstances[nInst];
		if (NULL != pObj)
		{
			hRes = m_pDS->UpdateInstance(pObj);
			if (FAILED(hRes))
				break;
		}
	}

	return hRes;
}

//////////////////////////////////////////////////////////////
//
//
//	CSampleDataSource
//
//
//////////////////////////////////////////////////////////////


CSampleDataSource::CSampleDataSource() : m_hThread(NULL)
//////////////////////////////////////////////////////////////////////
//
//	Constructor
//
//////////////////////////////////////////////////////////////////////
//ok
{
	m_hQuit = CreateEvent( NULL, TRUE, FALSE, NULL );
}

CSampleDataSource::~CSampleDataSource()
//////////////////////////////////////////////////////////////////////
//
//	Destructor
//
//////////////////////////////////////////////////////////////////////
//ok
{
	if (NULL != m_hThread)
	{
		// Signal the thread to terminate
		// ==============================

		SetEvent( m_hQuit );

		DWORD dwRet = WaitForSingleObject( m_hThread, 60000 );

		switch ( dwRet )
		{
		case WAIT_OBJECT_0:
			{
			}break;
		case WAIT_TIMEOUT:
			{
				// Something is wrong - handle appropriately
			}break;
		}

		m_hThread = NULL;
	}
}

DWORD CSampleDataSource::Initialize(IWbemClassObject* pSampleClass)
//////////////////////////////////////////////////////////////
//
//	Initializes the data source.
//
//	Set the access handles for the properties of the WMI objects
//	and create a thread that will independantly update the 
//	counter values. 
//
//	Parameters:
//		pSampleClass	- a template of the class for which
//							handles are required
//
//////////////////////////////////////////////////////////////
//ok
{
	DWORD dwRes = DWORD(WBEM_E_FAILED);
	unsigned uThread = 0;

	// Set the handles for the object (valid for all instances)
	// ========================================================

	dwRes = SetHandles(pSampleClass);
	if (FAILED(dwRes))
		return dwRes;

	// Create the simulation thread
	// ============================

	m_hThread = (PVOID)_beginthreadex(NULL, 0, CSampleDataSource::ThreadEntry, this, 0, &uThread); 

	if (0 == m_hThread)
		return DWORD(WBEM_E_FAILED);

	return WBEM_NO_ERROR;
}

DWORD CSampleDataSource::SetHandles(IWbemClassObject* pSampleClass)
//////////////////////////////////////////////////////////////
//
//	Called by the initialization method.
//
//	Get the access handles for the well-known properties in
//	the WMI object.  These handles are the same for all instances 
//	of the object.
//
//	If the method fails, the property handle values may be in 
//	an indeterminite state.
//
//	Parameters:
//		pSampleClass	- a template of the class for which
//							handles are required
//
//////////////////////////////////////////////////////////////
//ok
{
	DWORD dwRes = WBEM_NO_ERROR;

	IWbemObjectAccess *pAccess = NULL;
    BSTR strPropName	= NULL;

    if (NULL == pSampleClass)
        return DWORD(WBEM_E_FAILED);

	// Get the IWbemAccess interface to the class
	// ==========================================

	dwRes = pSampleClass->QueryInterface(IID_IWbemObjectAccess, (LPVOID *)&pAccess);

	if (FAILED(dwRes))
		return dwRes;

	// Get the counter properties access handles
	// =========================================

	for (int i = ctr1; i < NumCtrs; i++)
	{
		strPropName = SysAllocString(g_aCounterProps[i].m_tcsPropertyName);    
		if (!strPropName)
		{
			pAccess->Release();
			return DWORD(WBEM_E_OUT_OF_MEMORY);
		}
			

		dwRes = pAccess->GetPropertyHandle(strPropName, 0, &m_alHandle[i]);

		SysFreeString(strPropName);

		if (FAILED(dwRes))
		{
			pAccess->Release();
			return dwRes;
		}
	}

	pAccess->Release();

	return WBEM_NO_ERROR;
}


DWORD CSampleDataSource::UpdateInstance(IWbemObjectAccess* pAccess)
//////////////////////////////////////////////////////////////
//
//	Updates a given instance with the stored counter values.
//
//	Fetched the ID of the object, retrieves the corresponding
//	CSampleInstance object, and copies the counter values
//	into the object counter properties.
//
//	Parameters:
//		pAccess		- the object to be updated
//
//////////////////////////////////////////////////////////////
//ok
{
	DWORD dwRes = DWORD(WBEM_E_FAILED);

	DWORD dwID = 0;

	// Get the ID of the object
	// ========================

	WCHAR wszName[20];
	long lNumBytes = 0;
	dwRes = pAccess->ReadPropertyValue(g_hID, sizeof(wszName), &lNumBytes, (byte *)wszName);
	if (FAILED(dwRes))
		return dwRes;
	dwID = _wtoi(wszName);

	// Get the counter data
	// ====================

	CSampleInstance* pInst = &(m_aInstance[dwID]);

	// And set the counter properties
	// ==============================

	pInst->Lock(true);

	for (int nProp = ctr1; nProp < NumCtrs; nProp++)
	{
		DWORD dwVal = 0;
		dwRes = pInst->GetCounter(nProp, &dwVal);
		if (FAILED(dwRes))
			break;
		dwRes = pAccess->WriteDWORD(m_alHandle[nProp], dwVal);
		if (FAILED(dwRes))
			break;
	}

	pInst->Lock(false);

	return dwRes;
}

DWORD CSampleDataSource::Simulate()
//////////////////////////////////////////////////////////////
//
//	This will simulate real changes to the data set.  The 
//	thread will periodically cycle through the instances 
//	modifying their counter values.  The thread will be killed 
//	by the calling thread.
//
//////////////////////////////////////////////////////////////
//ok
{
	CSampleInstance* pCurrInst = NULL;
	DWORD dwVal = 0;

	// Loop until the thread is killed
	// ===============================

	while ( WaitForSingleObject( m_hQuit, 0 ) == WAIT_TIMEOUT )
	{
		// Loop through the instances
		// ==========================

		for (int nInst = 0; nInst < NUM_INSTANCES; nInst++)
		{
			// Get the current instance and lock it
			// ====================================

			pCurrInst = &(m_aInstance[nInst]);

			pCurrInst->Lock(true);

			// Update all of the counters
			// ==========================

			for (int nCtr = ctr1; nCtr < NumCtrs; nCtr++)
			{
				dwVal = GetTickCount() / (nCtr + 1);
				pCurrInst->SetCounter(nCtr, dwVal);
			}

			pCurrInst->Lock(false);
		}
		
		// Wait a while
		// ============

		Sleep(100);
	}

	return WBEM_NO_ERROR;
}

unsigned CSampleDataSource::ThreadEntry(void* pArgs)
//////////////////////////////////////////////////////////////
//
//	The entry point for the simulation.  Calls Simulate.
//
//	Parameters:
//		pArgs	- contains the this pointer of the calling 
//					object
//
//////////////////////////////////////////////////////////////
//ok
{
	CSampleDataSource* pThis = (CSampleDataSource*)pArgs;

	pThis->Simulate();

	return 0;
}

//////////////////////////////////////////////////////////////
//
//
//	CSampleInstance
//
//
//////////////////////////////////////////////////////////////

CSampleInstance::CSampleInstance()
//////////////////////////////////////////////////////////////
//
//	Constructor
//
//////////////////////////////////////////////////////////////
//ok
{
	InitializeCriticalSection(&CS);
}

CSampleInstance::CSampleInstance(const CSampleInstance &aInst)
//////////////////////////////////////////////////////////////
//
//	Copy Constructor
//
//////////////////////////////////////////////////////////////
//ok
{
	InitializeCriticalSection(&CS);

	for (int i = ctr1; i < NumCtrs; i++)
		m_aCounter[i] = aInst.m_aCounter[i];
}

CSampleInstance::~CSampleInstance()
//////////////////////////////////////////////////////////////
//
//	Destructor
//
//////////////////////////////////////////////////////////////
//ok
{
	DeleteCriticalSection(&CS);
}

DWORD CSampleInstance::Lock(bool bLock)
//////////////////////////////////////////////////////////////
//
//	Locks or unlocks the instance.    
//
//	Parameters:
//		bLock	- true = lock; false = unlock
//
//////////////////////////////////////////////////////////////
//ok
{
	if (bLock)
		EnterCriticalSection(&CS);
	else
		LeaveCriticalSection(&CS);

	return WBEM_NO_ERROR;
}

DWORD CSampleInstance::SetCounter(long lCtrIndex, DWORD dwVal)
//////////////////////////////////////////////////////////////
//
//	Sets the counter value for an indexed counter.
//
//	Parameters:
//		lCtrIndex	- the index of the counter
//		dwVal		- the value of the counter
//
//////////////////////////////////////////////////////////////
//ok
{
	DWORD dwRes = DWORD(WBEM_E_FAILED);

	// Verify counter index
	// ====================

	if ((lCtrIndex >= 0) && (lCtrIndex < NumCtrs))
	{
		// Update the counter value
		// ========================
		m_aCounter[lCtrIndex] = dwVal;

		dwRes = WBEM_NO_ERROR;
	}

	return dwRes;
}

DWORD CSampleInstance::GetCounter(long lCtrIndex, DWORD* pdwVal)
//////////////////////////////////////////////////////////////
//
//	Returns the counter value for an indexed counter.
//
//	Parameters:
//		lCtrIndex	- the index of the counter
//		dwVal		- the return parameter for the value
//
//////////////////////////////////////////////////////////////
//ok
{
	DWORD dwRes = DWORD(WBEM_E_FAILED);
	
	// Verify counter index
	// ====================

	if ((lCtrIndex >= 0) && (lCtrIndex < NumCtrs))
	{	
		// Set return values
		// =================

		*pdwVal = m_aCounter[lCtrIndex];
		dwRes = WBEM_NO_ERROR;
	}

	return dwRes;
}


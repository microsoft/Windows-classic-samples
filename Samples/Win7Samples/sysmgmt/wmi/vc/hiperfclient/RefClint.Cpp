// Copyright (c)  Microsoft Corporation, All Rights Reserved
///////////////////////////////////////////////////////////////////
//
//	RefClint.cpp : Refresher client implementation file
//
//	CRefClient encapsulates the basic functionality of using a 
//	WMI high performance refresher.
//
///////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>

#include "RefClint.h"
///////////////////////////////////////////////////////////////////
//
//	Counter data
//
//	These structures are used as a matter of convienience and 
//	clarity.  A real client should enumerate the properties
//	of an object in order to determine the number and names
//	of the counter properties.
//
///////////////////////////////////////////////////////////////////

// The number of counter properties for Win32_BasicHiPerf

enum 
{
	Ctr1,
	Ctr2,
	Ctr3,
	Ctr4,
	Ctr5,
	NumCtrs
};

// The names and handles (set by SetHandles()) for a Win32_BasicHiPerf object

struct CCounterData
{
	WCHAR	m_wcsName[256];
	long	m_lHandle;
} g_aCounters[] =
{
	L"Counter1", 0,
	L"Counter2", 0,
	L"Counter3", 0,
	L"Counter4", 0,
	L"Counter5", 0,
};


///////////////////////////////////////////////////////////////////
//
//	CRefClient
//
///////////////////////////////////////////////////////////////////


CRefClient::CRefClient() : 
	m_pNameSpace(NULL), m_pRefresher(NULL), m_pConfig(NULL)
{
}

CRefClient::~CRefClient()
{
	// Release the COM interfaces (if refresher members have not been manually
	// removed, the refresher destructor should do so automatically)
	// =======================================================================

	if (NULL != m_pNameSpace)
		m_pNameSpace->Release();

	if (NULL != m_pRefresher)
		m_pRefresher->Release();

	if (NULL != m_pConfig)
		m_pConfig->Release();
}

///////////////////////////////////////////////////////////////////
//
//	Helper methods
//
///////////////////////////////////////////////////////////////////

HRESULT CRefClient::SetHandles()
///////////////////////////////////////////////////////////////////
//
//	SetHandles will initialize the IWbemObjectAccess handle values 
//	in the counter array.
//
//	Returns a status code. Use the SUCCEEDED() and FAILED() macros
//	to interpret results.
//
///////////////////////////////////////////////////////////////////
//ok
{
	HRESULT hRes = WBEM_NO_ERROR;
	IWbemClassObject*	pObj = NULL;
	IWbemObjectAccess*	pAccess = NULL;
	long lIndex;

// Get an IWbemObjectAccess interface to one of the sample objects
// ===============================================================
	// Create a sample object 
	// ======================
	BSTR strObj = SysAllocString( cwcsObjectPath );
	if (strObj == NULL)
	{
		hRes = E_OUTOFMEMORY;
		goto cleanup;
	}

	hRes = m_pNameSpace->GetObject( strObj, 0, NULL, &pObj, NULL );

	SysFreeString( strObj );

	if ( FAILED( hRes ) ) 
		goto cleanup;

	// Get the alternate interface
	// ===========================

	hRes = pObj->QueryInterface( IID_IWbemObjectAccess, ( LPVOID* )&pAccess );

	if ( FAILED( hRes ) ) 
		goto cleanup;

// Set the access handles for all of the counter properties
// ========================================================

	for ( lIndex = Ctr1; lIndex < NumCtrs; lIndex++ )
	{	
		long lHandle;

		hRes = pAccess->GetPropertyHandle( g_aCounters[lIndex].m_wcsName, NULL, &lHandle );

		if ( FAILED( hRes ) )
			goto cleanup;

		g_aCounters[lIndex].m_lHandle = lHandle;
	}
// Cleanup the object pointers
// ===========================

cleanup:
	if ( NULL != pObj )
	{
		pObj->Release();
		pObj = NULL;
	}

	if ( NULL != pAccess )
	{
		pAccess->Release();
		pAccess = NULL;
	}

	if ( FAILED( hRes ) )
	{
		printf( "SetHandles() failed, 0x%x\n", hRes );
	}

	return hRes;
}

///////////////////////////////////////////////////////////////////
//
//	Class method API
//
///////////////////////////////////////////////////////////////////

HRESULT CRefClient::Initialize(IWbemServices* pNameSpace)
///////////////////////////////////////////////////////////////////
//
//	Initialize will create the refresher and configuration manager
//	and set the IWbemObjectAccess handles which are used to access
//	property values of the instances.
//
//	Returns a status code. Use the SUCCEEDED() and FAILED() macros
//	to interpret results.
//
///////////////////////////////////////////////////////////////////
{
	HRESULT hRes = WBEM_NO_ERROR;

	// Copy the namespace pointer
	// ==========================

	if ( NULL == pNameSpace )
		return WBEM_E_INVALID_PARAMETER;

	//Cleanup: in case this method nas been called more than once
	if (m_pNameSpace != NULL)
		m_pNameSpace->Release();
	if (m_pRefresher != NULL)
		m_pRefresher->Release();
	if (m_pConfig != NULL)
		m_pConfig->Release();

	m_pNameSpace = pNameSpace;
	m_pNameSpace->AddRef();

	// Create the refresher and refresher manager
	// ==========================================

	hRes = CoCreateInstance( CLSID_WbemRefresher, 
							 NULL, 
							 CLSCTX_INPROC_SERVER, 
							 IID_IWbemRefresher, 
							 (void**) &m_pRefresher );
	if ( FAILED( hRes ) )
		return hRes;

	hRes = m_pRefresher->QueryInterface( IID_IWbemConfigureRefresher, 
										 (void**) &m_pConfig );
	if ( FAILED( hRes ) )
	{
		m_pRefresher->Release();
		return hRes;
	}

	// Set the access handles
	// ======================

	hRes = SetHandles();

	return hRes;
}

HRESULT CRefClient::AddObjects()
///////////////////////////////////////////////////////////////////
//
//	AddObject will add a set of objects to the refresher.  The 
//	method will update m_aInstances with the instance data. 
//
//	Returns a status code. Use the SUCCEEDED() and FAILED() macros
//	to interpret results.
//
///////////////////////////////////////////////////////////////////
//ok
{
	HRESULT hRes = WBEM_NO_ERROR;

	long	lIndex = 0;
	WCHAR	wcsObjName[MAX_PATH + 50];

// Loop through all instances of Win32_BasicHiPerf and add them to the refresher
// =============================================================================

	for ( lIndex = 0; lIndex < clNumInstances; lIndex++ )
	{
		IWbemClassObject*	pObj = NULL;
		IWbemObjectAccess*	pAccess = NULL;
		long lID;

		// Set the object path (e.g. Win32_BasicHiPerf=1)
		// ==============================================

		StringCbPrintfW(wcsObjName, sizeof(wcsObjName), L"%s=%i", cwcsObjectPath, lIndex );
	
		// Add the object
		// ==============

		hRes = m_pConfig->AddObjectByPath( m_pNameSpace, wcsObjName, 0, NULL, &pObj, &lID );

		if ( FAILED( hRes ) )
		{
			printf( "AddObjectByPath() failed, 0x%x\n", hRes );
			return hRes;
		}

		// Save the IWbemObjectAccess interface
		// ====================================
		
		hRes = pObj->QueryInterface( IID_IWbemObjectAccess, (void**) &pAccess );

		pObj->Release();

		m_Instances[lIndex].Set(pAccess, lID);

		// Set does it's own AddRef()
		// ==========================

		pAccess->Release();
	}
	
	return hRes;
}

HRESULT CRefClient::RemoveObjects()
///////////////////////////////////////////////////////////////////
//
//	RemoveObjects calls Remove() on the refresher's configuration 
//	manager to remove all of the objects from the refresher.
//
//	Returns a status code. Use the SUCCEEDED() and FAILED() macros
//	to interpret results.
//
///////////////////////////////////////////////////////////////////
{
	HRESULT hRes = WBEM_NO_ERROR;

	long lInstance = 0;

	// Remove the instances
	// ====================

	if (m_pConfig)
	{
		for ( lInstance = 0; lInstance < clNumInstances; lInstance++ )
		{
			m_pConfig->Remove( m_Instances[lInstance].GetID(), 0L );
			m_Instances[lInstance].Reset();
		}
	}

	return hRes;
}

HRESULT CRefClient::ShowObjectData()
///////////////////////////////////////////////////////////////////
//
//	ShowInstanceData will output all of the counter data for all
//	of the instances within the refresher.
//
//	Returns a status code. Use the SUCCEEDED() and FAILED() macros
//	to interpret results.
//
///////////////////////////////////////////////////////////////////
{
	HRESULT hRes = WBEM_NO_ERROR;

	long lInstance;
	long lCounter;

// Cycle through all of the instances and print all the counter data
// =================================================================

	// Instance loop
	// =============

	for (lInstance = 0; lInstance < clNumInstances; lInstance++)
	{
		printf("  Instance %i:\n", lInstance);

		// Counter loop
		// ============

		for (lCounter = Ctr1; lCounter < NumCtrs; lCounter++)
		{
			DWORD dwVal;
			IWbemObjectAccess* pAccess = m_Instances[lInstance].GetMember();

			// Read the counter property value using the high performance IWbemObjectAccess->ReadDWORD().
			// NOTE: Remember to never to this while a refresh is in process!
			// ==========================================================================================

			hRes = pAccess->ReadDWORD( g_aCounters[lCounter].m_lHandle, &dwVal);
			if (FAILED(hRes))
			{
				pAccess->Release();

				printf("ShowInstanceData() failed, 0x%x", hRes);
				return hRes;
			}

			printf("    %i ", dwVal);

			pAccess->Release();
		}

		printf("\n");
	}

	return hRes;
}

HRESULT CRefClient::AddEnum()
///////////////////////////////////////////////////////////////////
//
//	AddEnum will add an enumerator to the refresher.  The 
//	function will return a status code.  The enumerator class member 
//	will updated.
//
//	Returns a status code. Use the SUCCEEDED() and FAILED() macros
//	to interpret results.
//
///////////////////////////////////////////////////////////////////
//ok
{
	HRESULT hRes = WBEM_NO_ERROR;

	IWbemHiPerfEnum*	pEnum = NULL;
	long lID;

// Add an enumerator to the refresher
// ==================================

	hRes = m_pConfig->AddEnum( m_pNameSpace, cwcsObjectPath, 0, NULL, &pEnum, &lID );

	if ( FAILED(hRes))
	{
		printf( "AddEnum() failed, 0x%x\n", hRes );
	}

	if (pEnum)
	{
		m_Enum.Set(pEnum, lID);
		// Set does it's own AddRef

		pEnum->Release();
	}
	

	return hRes;
}

HRESULT CRefClient::RemoveEnum()
///////////////////////////////////////////////////////////////////
//
//	RemoveEnum calls Remove() on the refresher's configuration 
//	manager to remove the enumerator from the refresher.
//
//	Returns a status code. Use the SUCCEEDED() and FAILED() macros
//	to interpret results.
//
///////////////////////////////////////////////////////////////////
{
	HRESULT hRes = WBEM_NO_ERROR;
	// Remove the enumerator
	// =====================
	if ( m_pConfig)
	{
		hRes = m_pConfig->Remove( m_Enum.GetID(), 0L );
	}
	m_Enum.Reset();


	return hRes;
}

HRESULT CRefClient::ShowEnumeratorData()
///////////////////////////////////////////////////////////////////
//
//	ShowEnumeratorData will output the number of instances within 
//	an enumerator.  Property values from the instances may obtained
//	using the standard WMI methods.
//
//	Returns a status code. Use the SUCCEEDED() and FAILED() macros
//	to interpret results.
/////////////////////////////////////////////////////////////////////
{
	HRESULT hRes = WBEM_NO_ERROR;

	DWORD	dwNumReturned = clNumInstances;
	DWORD	dwNumObjects = 0;

	IWbemObjectAccess**	apEnumAccess = NULL;

	IWbemHiPerfEnum*	pEnum = m_Enum.GetMember();
	if (pEnum == NULL)
	{
        return WBEM_E_FAILED;
	}
// Fetch the instances from the enumerator
// =======================================

	// Try to get the instances from the enumerator
	// ============================================

	hRes = pEnum->GetObjects( 0L, dwNumObjects, apEnumAccess, &dwNumReturned );

	// Is the buffer too small?
	// ========================

	if ( WBEM_E_BUFFER_TOO_SMALL == hRes )
	{
		// Increase the buffer size
		// ========================

		delete [] apEnumAccess;

		apEnumAccess = new IWbemObjectAccess*[dwNumReturned];
		dwNumObjects = dwNumReturned; 
		memset( apEnumAccess, 0, sizeof( apEnumAccess ));

		if ( NULL != apEnumAccess )
		{
			hRes = pEnum->GetObjects( 0L, dwNumObjects, apEnumAccess, &dwNumReturned );
		}
		else
		{
			hRes = WBEM_E_OUT_OF_MEMORY;
		}

	}	

	// Print the result
	// ================

	if ( SUCCEEDED( hRes ) )
	{
		printf( "\tThe enumerator returned %i instances\n", dwNumReturned );
	}
	else
	{
		printf( "ShowEnumeratorData failed, 0x%x\n", hRes );
	}

	// Release the objects from the enumerator's object array
	// ======================================================
	
	for ( DWORD nCtr = 0; nCtr < dwNumReturned; nCtr++ )
	{
		if (NULL != apEnumAccess[nCtr])
		{
			apEnumAccess[nCtr]->Release();
			apEnumAccess[nCtr] = NULL;
		}
	}

	if ( NULL != apEnumAccess )
		delete [] apEnumAccess;

	pEnum->Release();

	return hRes;
}

HRESULT CRefClient::Refresh()
///////////////////////////////////////////////////////////////////
//
//	Refresh simply calls the IWbemRefresher->Refresh() method to 
//	update all members of the refresher.
//
//	Returns a status code. Use the SUCCEEDED() and FAILED() macros
//	to interpret results.
//
///////////////////////////////////////////////////////////////////
{
	return m_pRefresher->Refresh( 0L );
}

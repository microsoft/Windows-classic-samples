//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module  CDataSourceObject Implementation Module | Implementation of base class for OLE DB Test Modules
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 10-05-95	Microsoft	Created <nl>
//	[02] 01-20-96	Microsoft	Changed inheritance on COLEDB <nl>
//	[03] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CDataSourceObject Elements|
//
// @subindex CDataSourceObject              
//
//---------------------------------------------------------------------------

#include "privstd.h"		// Private library common precompiled header
#include "coledb.hpp"
#include "miscfunc.h"		// For GetInitProps

//--------------------------------------------------------------------
// @mfunc CDataSourceObject
//
// @parm [IN] Test case name
//
//--------------------------------------------------------------------
CDataSourceObject::CDataSourceObject(WCHAR * pwszTestCaseName):COLEDB(pwszTestCaseName) 
{
	m_pIDBInitialize = NULL;
	m_fInitialized	 = FALSE;
}

//--------------------------------------------------------------------
// @mfunc ~CDataSourceObject
//
//--------------------------------------------------------------------
CDataSourceObject::~CDataSourceObject(void)
{
}

//--------------------------------------------------------------------
// @mfunc SetDataSourceObject
//
// This function is used to set the object's ITransactionDispenser interface
// pointer for the Data Source Object.  This function should be used when
// the Date Source was created by means other than CreateDataSourceObject.
// This function and COLEDB::CreateDataSourceObject are mutually exclusive 
// ways to set an IDBCreateCommand pointer for the object. <nl>
//
// NOTE:  An AddRef is done on this interface, so the caller is still responsible 
// for releasing the interface passed in, as well as calling ReleaseDataSourceObject.
//
//--------------------------------------------------------------------
HRESULT CDataSourceObject::SetDataSourceObject
(
	IUnknown*	pIUnknown,			// @parm [IN] Current IDBInitialize pointer
	BOOL		fAlreadyInitialized	// @parm [IN] Whether or not the DSO has been initialized already.
)	
{
	// This pointer should not be set if user is using class correctly
	ASSERT(m_pIDBInitialize == NULL);
	HRESULT hr = S_OK;

	//Obtain the IDBInitialize interface...
	if(!VerifyInterface(pIUnknown, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBInitialize))
	{
		return E_NOINTERFACE;
	}
	
	if (fAlreadyInitialized)
		m_fInitialized = TRUE;
	
	return hr;
}

//--------------------------------------------------------------------
// @mfunc ReleaseDataSourceObject
//
// Gets a Data Source Object for the current provider and returns
// an ITransactionDispenser interface on that object if ppITransactionDispenser
// is not null. <nl>
//
// NOTE:  The user must call the following function to be fully released
// from the provider:
//
//--------------------------------------------------------------------
HRESULT	CDataSourceObject::CreateDataSourceObject(void)
{
	HRESULT hr = S_OK;

	// If we don't have a Data Source yet	
	if(m_pIDBInitialize == NULL)
		CHECK(hr = GetModInfo()->CreateProvider(NULL, IID_IDBInitialize, (IUnknown**)&m_pIDBInitialize), S_OK);

	return hr;
}									   

//--------------------------------------------------------------------
// @mfunc InitializeDSO
//
// @parm [IN]  If the DSO is alreadyIf eReinitialize == REINITIALIZE_YES,
//
// Initializes the DSO.  
// the DSO is uninitialized and then reinitialized if it was already
// initialized when this method was called.  If eReinitialzie == 
// REINITIALIZE_NO, if the DSO has already been initialized, nothing is done,
// if the DSO has not been initialized, initialization is attempted.
//
//--------------------------------------------------------------------
HRESULT	CDataSourceObject::InitializeDSO(EREINITIALIZE eReinitialize, ULONG cExPropSets, DBPROPSET* rgExPropSets)
{
	HRESULT hr = E_FAIL;
	IDBProperties* pIDBProperties	= NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	// We must have IDBInitialize already
	if (!m_pIDBInitialize)
		goto CLEANUP;
	
	// Decide what to do if we are already initialized
	if (m_fInitialized)
	{
		// We stay in same state if the user doesn't want to reinitialize
		if (eReinitialize == REINITIALIZE_NO)
		{
			hr = S_OK;
			goto CLEANUP;
		}
		else // We uninitialize and go on
		{
			CHECK(UninitializeDSO(), S_OK);
		}
	}

	// Build our init options from string passed to us from LTM for this provider
	if(!GetInitProps(&cPropSets, &rgPropSets))
	{
		hr = E_FAIL;
		goto CLEANUP;
	}

	// Get IDBProperties Pointer
	if(!VerifyInterface(m_pIDBInitialize, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	//Set any Extra Properties the user wishes to set on the DataSource
	if(cExPropSets)
	{
		if(FAILED(hr = pIDBProperties->SetProperties(cExPropSets, rgExPropSets)))
			goto CLEANUP;
	}

	//Now, set the Init properties before we Initialize
	if(FAILED(hr = pIDBProperties->SetProperties(cPropSets, rgPropSets)))
		goto CLEANUP;

	// Try to Initialize and set flag accordingly
	if(FAILED(hr = m_pIDBInitialize->Initialize()))
		goto CLEANUP;

	m_fInitialized = TRUE;	

CLEANUP:
	// Only free if we got properties back	
	FreeProperties(&cPropSets, &rgPropSets);

	// Free pIDBProperties
	SAFE_RELEASE(pIDBProperties);
	return hr;	
}

//--------------------------------------------------------------------
// @mfunc UninitializeDSO
//
//--------------------------------------------------------------------
HRESULT	CDataSourceObject::UninitializeDSO(void)
{
	// We must have IDBInitialize already
	if (!m_pIDBInitialize)
		return E_FAIL;

	// Uninitialize and set the flag accordingly
	if (CHECK(m_hr = m_pIDBInitialize->Uninitialize(), S_OK))
		m_fInitialized = FALSE;
	
	return m_hr;
}

//--------------------------------------------------------------------
// @mfunc ReleaseDataSourceObject
//
//--------------------------------------------------------------------
void CDataSourceObject::ReleaseDataSourceObject(void)
{								   				
	// Release the interface asked for in CreateDataSourceObject
	if (m_pIDBInitialize)		
		if(m_pIDBInitialize->Release()==0)
			m_fInitialized = FALSE;

	m_pIDBInitialize = NULL;
}

//--------------------------------------------------------------------
// @mfunc GetDataSourceObject
//
//--------------------------------------------------------------------
HRESULT CDataSourceObject::GetDataSourceObject
(
	REFIID		riid,		// @parm [IN] IID of DSO pointer
	IUnknown**	ppIUnknown	// @parm [OUT] DSO pointer
)
{
	if(ppIUnknown)
		*ppIUnknown = NULL;
	
	if(m_pIDBInitialize)
	{
		if(!VerifyInterface(m_pIDBInitialize, riid, DATASOURCE_INTERFACE, ppIUnknown))
			return E_NOINTERFACE;

		return S_OK;
	}
	
	return E_FAIL;
}

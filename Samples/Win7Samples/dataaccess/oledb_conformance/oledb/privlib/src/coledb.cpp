//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module  COLEDB Implementation Module| Implementation of base class for OLE DB Test Modules
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 10-05-95	Microsoft	Created <nl>
//	[02] 01-20-96	Microsoft	Changed inheritance for COLEDB <nl>
//	[03] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 COLEDB Elements|
//
// @subindex COLEDB            
//
//---------------------------------------------------------------------------

#include "privstd.h"		//Private library common precompiled header
#include "coledb.hpp"


////////////////////////////////////////////////////////
// CBase::CBase
//
////////////////////////////////////////////////////////
CBase::CBase(CBase* pCBase,	LPUNKNOWN pUnkOuter)
{
	m_cRef		= 0;												
	m_pCBase	= pCBase;
	SAFE_ADDREF(m_pCBase);
	
	m_pUnkOuter = pUnkOuter ? pUnkOuter : this;

	//AddRef the parent object...
	//This is done since all Child objects in OLE DB have some way to 
	//get back to the parent, Session -> DataSource, Rowset -> Command.
	//We need to addref the parent so its still active whenever needed to get back to
	if(m_pCBase)
		SAFE_ADDREF(m_pCBase->m_pUnkOuter);
}

////////////////////////////////////////////////////////
// CBase::~CBase
//
////////////////////////////////////////////////////////
CBase::~CBase()
{
	//Release the parent object...
	//This is done since all Child objects in OLE DB have some way to 
	//get back to the parent, Session -> DataSource, Rowset -> Command.
	//We need to addref the parent so its still active whenever needed to get back to
	if(m_pCBase && m_pCBase->m_pUnkOuter)
		m_pCBase->m_pUnkOuter->Release();

	SAFE_RELEASE(m_pCBase);
}


////////////////////////////////////////////////////////
// CBase::QueryInterface
//
////////////////////////////////////////////////////////
STDMETHODIMP CBase::QueryInterface
    (
    REFIID riid,        //@parm IN | Interface ID of the interface being queried for.
    LPVOID * ppv        //@parm OUT | Pointer to interface that was instantiated
    )
{
	HRESULT hr = S_OK;
    
	if(!ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	//IUNKNOWN
	if(riid == IID_IUnknown)
	{
		*ppv = this;
	}
		
	//MANADATORY

	//OPTIONAL

	//NOTSUPPORTED
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	if(*ppv)
		((IUnknown*)*ppv)->AddRef();	
	return hr;
}


////////////////////////////////////////////////////////
// CBase::GetBaseObj
//
////////////////////////////////////////////////////////
IUnknown* CBase::GetBaseObj()
{
	if(m_pCBase && m_pCBase->m_pUnkOuter)
		return m_pCBase->m_pUnkOuter;

	return m_pCBase;
}


////////////////////////////////////////////////////////
// CBase::GetBaseInterface
//
////////////////////////////////////////////////////////
HRESULT CBase::GetBaseInterface(REFIID riid, IUnknown** ppIUnknown)
{
	if(ppIUnknown)
		*ppIUnknown = NULL;
	
	IUnknown* pBaseUnk = GetBaseObj();
	if(pBaseUnk)
		return pBaseUnk->QueryInterface(riid, (void**)ppIUnknown);

	return E_FAIL;
}




//--------------------------------------------------------------------
// @mfunc COLEDB  
//
// @parm [IN] Test case name
//
//--------------------------------------------------------------------
COLEDB::COLEDB(WCHAR * pwszTestCaseName) : CTestCases(pwszTestCaseName) 
{

	// Init to S_OK and NULL
	m_hr		= S_OK;
	m_pIMalloc	= NULL;
	m_pExtError = NULL;
	m_fLocalize	= FALSE;

	// If we can't get our memory allocator, we're in trouble anyway, so assert
	CoGetMalloc(MEMCTX_TASK, &m_pIMalloc);
	ASSERT(m_pIMalloc);

	//Record if we are called within a TC or standalone object
	m_fInsideTestCase	= TRUE;
	if(pwszTestCaseName == INVALID(WCHAR*))
		m_fInsideTestCase = FALSE;
}

//--------------------------------------------------------------------
// @mfunc ~COLEDB
//
//--------------------------------------------------------------------
COLEDB::~COLEDB()
{
	SAFE_RELEASE(m_pIMalloc);
}

//--------------------------------------------------------------------
// @mfunc Init
//
// Init used to create extended error object
//
//--------------------------------------------------------------------
BOOL COLEDB::Init()
{
	WCHAR* pwszValue = NULL;

	//If not called from LTM, setup CTestCases info...
	if(!m_fInsideTestCase)
		SetOwningMod(0, GetModInfo()->GetThisTestModule());

	// Check for the Localize keyword
	if(GetModInfo()->GetInitStringValue(L"LOCALIZE", &pwszValue))
		m_fLocalize = TRUE;

	// Create an object for checking extended errors, which will use
	// m_pError to increment the error count as needed.
	m_pExtError = new CExtError(m_pThisTestModule->m_ProviderClsid, m_pError, m_fLocalize);
	PROVIDER_FREE(pwszValue);

	return m_pExtError != NULL;
}

//--------------------------------------------------------------------
// @mfunc Init
//
// Terminate used to delete extended error object
//
//--------------------------------------------------------------------
BOOL COLEDB::Terminate()
{
	SAFE_DELETE(m_pExtError);
	return TRUE;
}

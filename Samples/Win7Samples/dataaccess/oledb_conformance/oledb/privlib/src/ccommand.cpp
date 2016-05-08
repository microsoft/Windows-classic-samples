//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CCommandObject Implementation Module | Implementation of base class for OLE DB Test Modules
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
// @head3 CCommandObject Elements|
//
// @subindex CCommandObject|
//
//---------------------------------------------------------------------------

#include "privstd.h"	// Private library common precompiled header
#include "coledb.hpp"

//--------------------------------------------------------------------
// Connection base class constructor.
//
// @mfunc CCommandObject
// @parm [IN] Test case name
//
//--------------------------------------------------------------------
CCommandObject::CCommandObject(WCHAR * pwszTestCaseName) : CSessionObject(pwszTestCaseName) 
{
	m_pICommand  = NULL;
	m_pIAccessor = NULL;
}

//--------------------------------------------------------------------
// Connection base class destructor.
//
// @mfunc ~CCommandObject
//
//--------------------------------------------------------------------
CCommandObject::~CCommandObject(void)
{
}

//--------------------------------------------------------------------
// This function is used to set the object's ICommand interface
// pointer for the Command Object.  This function should be used when
// the Command was created by means other than CreateCommandObject.
// This function and COLEDB::CreateCommandObject are mutually exclusive 
// ways to set an ICommand pointer for the object. <nl>
//
// NOTE:  An AddRef is done on this interface, so the caller is still responsible 
// for releasing the interface passed in, as well as calling ReleaseCommandObject.
//
// @mfunc SetCommandObject
//
// @parm [IN] Current ICommand pointer
//
//--------------------------------------------------------------------
void CCommandObject::SetCommandObject(ICommand * pICommand)	
{	
	IOpenRowset* pIOpenRowset = NULL;

	// This pointer should not be set if user is using class correctly
	ASSERT(m_pICommand == NULL);

	// Add Ref so the user can call ReleaseCommandObject -- this will
	// make SetCommandObject and CreateCommandObject require the same cleanup.
	m_pICommand = pICommand;
	m_pICommand->AddRef();

	// Set the session if we don't have one already
	if(m_pIOpenRowset == NULL)
	{	
		// Get the mandatory session interface
		if(CHECK(m_hr=m_pICommand->GetDBSession(IID_IOpenRowset, (IUnknown**)&pIOpenRowset),S_OK))
		{
			SetDBSession(pIOpenRowset);
		}
		else
		{		
			odtLog << wszNOSESSIONOBJECT;
			return;				
		}
	}

	SAFE_RELEASE(pIOpenRowset);
}

//--------------------------------------------------------------------
// Gets a Command Object for the current provider and returns an
// ICommand interface on that object if ppICommand is not null. <nl>
//
// NOTE:  The user must call the following functions to be fully released
// from the provider.  They may call a subset of these functions to release
// only the objects they choose:
//      ReleaseCommandObject
//		ReleaseDBSession
//		ReleaseDataSourceObject
//
// @mfunc CreateCommandObject
//
//--------------------------------------------------------------------
HRESULT	CCommandObject::CreateCommandObject(IUnknown* pIUnkOuter)
{
	HRESULT hr = S_OK;
	IDBCreateCommand* pIDBCreateCommand = NULL;

	// Get ICommand if we haven't done so already
	if(m_pICommand == NULL)
	{		
		// Get DB Session
		if(FAILED(hr = CreateDBSession(COMMAND_GENERATED)))
			goto CLEANUP;

		// See if Command Object is supported
		if(CHECK(hr = m_pIDBCreateCommand->CreateCommand(pIUnkOuter, IID_ICommand,(IUnknown **)&m_pICommand),S_OK))
		{
			// Make sure DB Session returned is correct
			CHECK(hr = m_pICommand->GetDBSession(IID_IDBCreateCommand, (IUnknown **)&pIDBCreateCommand),S_OK);
			COMPARE((pIDBCreateCommand != NULL), 1);
		}
	}

CLEANUP:
	SAFE_RELEASE(pIDBCreateCommand);
	return hr;
}					

//--------------------------------------------------------------------
// Releases all interfaces on the Command Object, DB Session and
// Data Source Object.
//
// @mfunc ReleaseCommandObject
//
//--------------------------------------------------------------------
void	CCommandObject::ReleaseCommandObject(
			ULONG ulRefCount	//@parm [IN] Expected Ref Count for object
								// after release is done.  Default is zero,
								// meaning the object is expected to be deleted.
)
{	
	// Release the interface asked for in CreateCommandObject
	if (m_pICommand)
	{

		//DO NOT expect the ref cnt to be zero after this release.
		if(ulRefCount)
			COMPARE(m_pICommand->Release(), ulRefCount);
		else
			m_pICommand->Release();

		m_pICommand = NULL;

		// We are Command driven as far as properties are
		// concerned since we use commands to generate rowsets, 
		// so get rid of properties at this level
		FreeProperties(&m_cPropSets, &m_rgPropSets);
	}
}

//--------------------------------------------------------------------
// Returns Command object
//
// @mfunc ReleaseCommandObject
//
//--------------------------------------------------------------------
HRESULT CCommandObject::GetCommandObject
(
	REFIID			riid,			// @parm [IN] iid of command pointer
	IUnknown**		ppIUnknown		// @parm [OUT] Session Pointer
)
{
	if(ppIUnknown)
		*ppIUnknown = NULL;

	//Obtain correct Command interface
	if(m_pICommand)
	{
		if(!VerifyInterface(m_pICommand, riid, COMMAND_INTERFACE, ppIUnknown))
			return E_NOINTERFACE;

		return S_OK;
	}
	
	return E_FAIL;
}

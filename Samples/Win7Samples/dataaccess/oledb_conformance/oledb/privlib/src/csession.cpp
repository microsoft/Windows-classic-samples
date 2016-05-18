//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module  CSessionObject Implementation Module| Implementation of base class for OLE DB Test Modules
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 06-30-95	Microsoft	Created <nl>
//	[02] 09-01-95	Microsoft	Code review update <nl>
//	[03] 10-01-95	Microsoft	Change to WCHAR * <nl>
//	[04] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CSessionObject Elements|
//
// @subindex CSessionObject|
//
//---------------------------------------------------------------------------

#include "privstd.h"	// Private library common precompiled header
#include "coledb.hpp"

//--------------------------------------------------------------------
// @mfunc CSessionObject
//
// @parm [IN] Test case name 
//
//--------------------------------------------------------------------
CSessionObject::CSessionObject(WCHAR * pwszTestCaseName) : CDataSourceObject(pwszTestCaseName) 
{
	// Null out Interface ptrs until we get valid ptrs from provider		
	m_pIDBCreateCommand = NULL;
	m_pIOpenRowset		= NULL;
	m_pColRowset		= NULL;
	m_pTable			= NULL;
	m_pTable2			= NULL;
	
	m_fDeleteTable	= DELETETABLE_YES;
	m_fDeleteTable2 = DELETETABLE_YES;
	
	m_cPropSets  = 0;			
	m_rgPropSets = NULL;		
}

//--------------------------------------------------------------------
// @mfunc ~CSessionObject
//
//--------------------------------------------------------------------
CSessionObject::~CSessionObject()
{
	// Since m_pTable lives on this object,
	// clean it up here if the user wants us to
	if (m_pTable && m_fDeleteTable == DELETETABLE_YES)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}
	
	if (m_pTable2 && m_fDeleteTable2==DELETETABLE_YES)
	{
		m_pTable2->DropTable();
		delete m_pTable2;
		m_pTable2=NULL;
	}
}

//--------------------------------------------------------------------
// This function is used to set the object's Session pointer
// This function should be used when the DBSession was created by means other 
// than CreateDBSession (ie, the ModuleGetDBSession function).  
//
// NOTE:  An AddRef is done on this interface, so the caller is still responsible 
// for releasing the interface passed in, as well as calling ReleaseDBSession.
//
// void CSessionObject::SetDBSession(IUnknown * pSession)
// @mfunc SetDBSession
//
// @parm [IN] Current Session Object
//
//--------------------------------------------------------------------
void CSessionObject::SetDBSession(IUnknown * pSession)
{
	// Both these pointers should not be set if user is using class correctly
	ASSERT(pSession);

	//Release the existing Session
	ReleaseDBSession();

	//Obtain IOpenRowset
	VerifyInterface(pSession, IID_IOpenRowset, SESSION_INTERFACE,	(IUnknown **)&m_pIOpenRowset);
	
	//Obtain IDBCreateCommand (if supported)
	VerifyInterface(pSession, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&m_pIDBCreateCommand);
}

//--------------------------------------------------------------------
// This function is used to set the object's CTable so that a
// new one is not created when needed by this object.  If fDeleteTable
// is set to DELETETABLE_YES, pTable is dropped and deleted in
// the RowsetObject destructor -- but not here so allows
// the same table to be used for multiple testcases.  
//
// This assumes that a client will have to NEW the CTable before calling
// this function and delete the CTable at some point after this function. <nl>
//
// NOTE:  An AddRef is done on this interface, so the caller is still responsible 
// for releasing the interface passed in, as well as calling ReleaseDataSourceObject.
//
// @mfunc SetTable
//
//--------------------------------------------------------------------
void CSessionObject::SetTable
(
	CTable * pTable,			// @parm [IN] Current Table object
	EDELETETABLE eDeleteTable	// @parm [IN] Should table be deleted
)
{
	// Set our data member to table user's passed us
	// Record if we should delete the object in our release function
	m_pTable = pTable;
	m_fDeleteTable = eDeleteTable;
}

//--------------------------------------------------------------------
// This function is used to set the object's CTable so that a
// new one is not created when needed by this object.  If fDeleteTable
// is set to DELETETABLE_YES, pTable is dropped and deleted in
// the RowsetObject destructor -- but not here so allows
// the same table to be used for multiple testcases.  
//
// This assumes that a client will have to NEW the CTable before calling
// this function and delete the CTable at some point after this function. <nl>
//
// NOTE:  An AddRef is done on this interface, so the caller is still responsible 
// for releasing the interface passed in, as well as calling ReleaseDataSourceObject.
//
// @mfunc SetTable
//
//--------------------------------------------------------------------
void CSessionObject::SetTable2
(
	CTable * pTable,			// @parm [IN] Current Table object
	EDELETETABLE eDeleteTable	// @parm [IN] Should table be deleted
)
{
	//Set our data member to table user's passed us
	//Record if we should delete the object in our release function
	m_pTable2 = pTable;
	m_fDeleteTable2 = eDeleteTable;
}

//--------------------------------------------------------------------
// Creates a Session for the current provider
//
// NOTE:  The user must call the following functions to be fully released
// from the provider.  They may call a subset of these functions to release
// only the objects they choose:
//		ReleaseDBSession
//		ReleaseDataSourceObject
//
// @mfunc CreateDBSession
//
//--------------------------------------------------------------------
HRESULT	CSessionObject::CreateDBSession(EROWSETGENERATED eRowsetGenerated)
{	
	HRESULT hr = NOERROR;
	IDBCreateSession*	pIDBCreateSession	= NULL;
	
	//Get IOpenRowset we haven't done so already
	if(m_pIOpenRowset == NULL)
	{
		// Get the DSO 
		if(!CHECK(hr = CreateDataSourceObject(), S_OK))
			return hr;
						
		// Now initialize DSO if we haven't done so already
		if(!CHECK(hr = InitializeDSO(REINITIALIZE_NO), S_OK))
			return hr;

		// Create the session object
		if(!VerifyInterface(m_pIDBInitialize, IID_IDBCreateSession, DATASOURCE_INTERFACE, (IUnknown **)&pIDBCreateSession))
			return E_NOINTERFACE;
		
		//Obtain IOpenRowset session
		if(!CHECK(hr = pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, (IUnknown **)&m_pIOpenRowset), S_OK))
			goto CLEANUP;
	}
	
	//Get IDBCreateCommand if we haven't done so already
	if(!GetCommandSupport())
	{
		//Try to get IDBCreateCommand
		if(!VerifyInterface(m_pIOpenRowset, IID_IDBCreateCommand, SESSION_INTERFACE,(IUnknown **)&m_pIDBCreateCommand)
			&& eRowsetGenerated == COMMAND_GENERATED)
		{
			hr = E_NOINTERFACE;
			goto CLEANUP;
		}
	}
	
CLEANUP:
	SAFE_RELEASE(pIDBCreateSession);			
	return hr;	
}

//--------------------------------------------------------------------
// Releases all interfaces to the provider on the DB Session and
// the Data Source.
//
// @mfunc ReleaseDBSession
//
//--------------------------------------------------------------------
void CSessionObject::ReleaseDBSession()
{	
	SAFE_RELEASE(m_pIOpenRowset);
	SAFE_RELEASE(m_pIDBCreateCommand);
	SAFE_RELEASE(m_pColRowset);
	
	//Free any Properties
	FreeProperties(&m_cPropSets, &m_rgPropSets);
}

//--------------------------------------------------------------------
// Returns pointer on Session object.
//
// @mfunc GetSessionObject
//
//--------------------------------------------------------------------
HRESULT CSessionObject::GetSessionObject
(
	REFIID			riid,			// @parm [IN] iid of Session Pointer
	IUnknown**		ppIUnknown		// @parm [OUT] Session Pointer
)
{
	if(ppIUnknown)
		*ppIUnknown = NULL;

	if(m_pIOpenRowset)
	{
		if(!VerifyInterface(m_pIOpenRowset, riid, SESSION_INTERFACE, ppIUnknown))
			return E_NOINTERFACE;

		return S_OK;
	}

	return E_FAIL;
}

//--------------------------------------------------------------------
//
// @cmember Get a column specific property on a give property set
//
// @rdesc TRUE or FALSE
//--------------------------------------------------------------------------
BOOL CSessionObject::GetColSpecProp(GUID guidPropertySet, DBPROPID *pPropID)
{
	BOOL			fResult				= FALSE;
	IDBProperties	*pIDBProperties		= NULL;
	IGetDataSource	*pIGetDataSource	= NULL;
	DBPROPIDSET		PropIDSet;
	DBPROPINFOSET	*rgPropInfoSet		= NULL;
	ULONG			cPropInfoSet		= 0;
	ULONG			i;

	TESTC(NULL != m_pIOpenRowset);

	// make sure the pointer is not null
	TESTC(NULL != pPropID);

	// get IDBProperties interface
	TESTC(VerifyInterface(m_pIOpenRowset, IID_IGetDataSource, 
		SESSION_INTERFACE, (IUnknown**)&pIGetDataSource));
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBProperties, (IUnknown**)&pIDBProperties), S_OK);
	TESTC(NULL != pIDBProperties);

	// get info about all rowset properties
	PropIDSet.rgPropertyIDs		= NULL;
	PropIDSet.cPropertyIDs		= 0;
	PropIDSet.guidPropertySet	= guidPropertySet;
	TESTC_PROVIDER(S_OK == (m_hr = pIDBProperties->GetPropertyInfo(1, &PropIDSet, &cPropInfoSet, &rgPropInfoSet, NULL)));
	TESTC(cPropInfoSet >= 1);
	TESTC(NULL != rgPropInfoSet && NULL != rgPropInfoSet[0].rgPropertyInfos);
	TESTC(guidPropertySet == rgPropInfoSet[0].guidPropertySet);
	
	// get a column specific property
	for (i=0; i<rgPropInfoSet[0].cPropertyInfos; i++)
	{
		// find a writeable boolean column specific property
		if (	(DBPROPFLAGS_COLUMNOK & rgPropInfoSet[0].rgPropertyInfos[i].dwFlags)
			&&	(DBPROPFLAGS_WRITE & rgPropInfoSet[0].rgPropertyInfos[i].dwFlags)
			&&	(DBTYPE_BOOL == rgPropInfoSet[0].rgPropertyInfos[i].vtType))
		{
			*pPropID = rgPropInfoSet[0].rgPropertyInfos[i].dwPropertyID;
			fResult = TRUE;
		}
	}

CLEANUP:
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBProperties);
	return fResult;
} //CSessionObject::GetColSpecProp


//--------------------------------------------------------------------
//
// @cmember Get a non column specific property on a property set
//
// @rdesc TRUE or FALSE
//--------------------------------------------------------------------------
BOOL CSessionObject::GetNonColSpecProp(GUID guidPropertySet, DBPROPID *pPropID)
{
	BOOL			fResult				= FALSE;
	IDBProperties	*pIDBProperties		= NULL;
	IGetDataSource	*pIGetDataSource	= NULL;
	DBPROPIDSET		PropIDSet;
	DBPROPINFOSET	*rgPropInfoSet		= NULL;
	ULONG			cPropInfoSet		= 0;
	ULONG			i;

	if (!m_pIOpenRowset)
		return FALSE;

	// make sure the pointer is not null
	TESTC(NULL != pPropID);

	// get IDBProperties interface
	TESTC(VerifyInterface(m_pIOpenRowset, IID_IGetDataSource, 
		SESSION_INTERFACE, (IUnknown**)&pIGetDataSource));
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBProperties, (IUnknown**)&pIDBProperties), S_OK);
	TESTC(NULL != pIDBProperties);

	// get info about all rowset properties
	PropIDSet.rgPropertyIDs		= NULL;
	PropIDSet.cPropertyIDs		= 0;
	PropIDSet.guidPropertySet	= guidPropertySet;
	TESTC_PROVIDER(S_OK == (m_hr = pIDBProperties->GetPropertyInfo(1, &PropIDSet, &cPropInfoSet, &rgPropInfoSet, NULL)));
	TESTC(cPropInfoSet >= 1);
	TESTC(NULL != rgPropInfoSet && NULL != rgPropInfoSet[0].rgPropertyInfos);
	TESTC(guidPropertySet == rgPropInfoSet[0].guidPropertySet);
	
	// get a column specific property
	for (i=0; i<rgPropInfoSet[0].cPropertyInfos; i++)
	{
		// find a writeable boolean non column specific property
		if (	!(DBPROPFLAGS_COLUMNOK & rgPropInfoSet[0].rgPropertyInfos[i].dwFlags)
			&&	(DBPROPFLAGS_WRITE & rgPropInfoSet[0].rgPropertyInfos[i].dwFlags)
			&&	(DBTYPE_BOOL == rgPropInfoSet[0].rgPropertyInfos[i].vtType))
		{
			*pPropID = rgPropInfoSet[0].rgPropertyInfos[i].dwPropertyID;
			fResult = TRUE;
		}
	}

CLEANUP:
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBProperties);
	return fResult;
} //CSessionObject::GetNonColSpecProp

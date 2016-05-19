//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CSESSION.CPP
//
//-----------------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CSession::CSession
//
/////////////////////////////////////////////////////////////////
CSession::CSession(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CAsynchBase(eCSession, pCMainWindow, pCMDIChild)
{
	//OLE DB Interfaces
	m_pIGetDataSource		= NULL;		//Session interface
	m_pIOpenRowset			= NULL;		//Session interface
	m_pISessionProperties	= NULL;		//Session interface
	m_pIDBCreateCommand		= NULL;		//Session interface
	m_pIDBSchemaRowset		= NULL;		//Session interface
	m_pIIndexDefinition		= NULL;		//Session interface
	m_pIAlterIndex			= NULL;		//Session interface
	m_pIAlterTable			= NULL;		//Session interface
	m_pITableDefinition		= NULL;		//Session interface
	m_pITableDefinitionWithConstraints	= NULL;		//Session interface

	m_pITransaction			= NULL;		//Session interface
	m_pITransactionLocal	= NULL;		//Session interface
	m_pITransactionJoin		= NULL;		//Session interface
	m_pITransactionObject	= NULL;		//Session interface

	m_pIBindResource		= NULL;		//Session interface
	m_pICreateRow			= NULL;		//Session interface

	m_cProvTypes			= 0;	// for native type names
	m_rgProvTypes			= NULL;
}


/////////////////////////////////////////////////////////////////
// CSession::~CSession
//
/////////////////////////////////////////////////////////////////
CSession::~CSession()
{
	ReleaseObject(0);

	//Objects
	m_listCPropSets.RemoveAll();
}


/////////////////////////////////////////////////////////////////
// IUnknown** CSession::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CSession::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IGetDataSource);
	HANDLE_GETINTERFACE(IOpenRowset);
	HANDLE_GETINTERFACE(ISessionProperties);
	HANDLE_GETINTERFACE(IDBCreateCommand);
	HANDLE_GETINTERFACE(IDBSchemaRowset);
	HANDLE_GETINTERFACE(IIndexDefinition);
	HANDLE_GETINTERFACE(IAlterIndex);
	HANDLE_GETINTERFACE(IAlterTable);
	HANDLE_GETINTERFACE(ITableDefinition);
	HANDLE_GETINTERFACE(ITableDefinitionWithConstraints);
	HANDLE_GETINTERFACE(ITransaction);
	HANDLE_GETINTERFACE(ITransactionLocal);
	HANDLE_GETINTERFACE(ITransactionJoin);
	HANDLE_GETINTERFACE(ITransactionObject);
	HANDLE_GETINTERFACE(IBindResource);
	HANDLE_GETINTERFACE(ICreateRow);

	//Otherwise delegate
	return CAsynchBase::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// HRESULT CSession::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CSession::AutoRelease()
{
	//Session
	RELEASE_INTERFACE(IGetDataSource);
	RELEASE_INTERFACE(IOpenRowset);
	RELEASE_INTERFACE(ISessionProperties);
	RELEASE_INTERFACE(IDBCreateCommand);
	RELEASE_INTERFACE(IDBSchemaRowset);
	RELEASE_INTERFACE(IIndexDefinition);
	RELEASE_INTERFACE(IAlterIndex);
	RELEASE_INTERFACE(IAlterTable);
	RELEASE_INTERFACE(ITableDefinition);
	RELEASE_INTERFACE(ITableDefinitionWithConstraints);
	RELEASE_INTERFACE(ITransaction);
	RELEASE_INTERFACE(ITransactionLocal);
	RELEASE_INTERFACE(ITransactionObject);
	RELEASE_INTERFACE(ITransactionJoin);

	RELEASE_INTERFACE(IBindResource);
	RELEASE_INTERFACE(ICreateRow);

	//Delegate
	return CAsynchBase::AutoRelease();
}


/////////////////////////////////////////////////////////////////
// HRESULT CSession::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CSession::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CAsynchBase::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IOpenRowset);
		OBTAIN_INTERFACE(IGetDataSource);
		OBTAIN_INTERFACE(ISessionProperties);
	}
	
	//AutoQI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{  	
		//[OPTIONAL]
		OBTAIN_INTERFACE(IDBCreateCommand);
		OBTAIN_INTERFACE(IDBSchemaRowset);
		OBTAIN_INTERFACE(IIndexDefinition);
		OBTAIN_INTERFACE(IAlterIndex);
		OBTAIN_INTERFACE(IAlterTable);
		OBTAIN_INTERFACE(ITableDefinition);
		OBTAIN_INTERFACE(ITableDefinitionWithConstraints);
		OBTAIN_INTERFACE(ITransaction);
		OBTAIN_INTERFACE(ITransactionLocal);
		OBTAIN_INTERFACE(ITransactionObject);
		OBTAIN_INTERFACE(ITransactionJoin);

		OBTAIN_INTERFACE(IBindResource);
		OBTAIN_INTERFACE(ICreateRow);
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// HRESULT CSession::CreateCommand
//
/////////////////////////////////////////////////////////////////
HRESULT CSession::CreateCommand(CAggregate* pCAggregate, REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT				hr = S_OK;

	//Obtain the IDBCreateCommand Interface
	IDBCreateCommand* pIDBCreateCommand = SOURCE_GETINTERFACE(this, IDBCreateCommand);
	if(pIDBCreateCommand)
	{
		//CreateCommand
		XTEST(hr = pIDBCreateCommand->CreateCommand(pCAggregate, riid, ppIUnknown));
		TESTC(TRACE_METHOD(hr, L"IDBCreateCommand::CreateCommand(0x%p, %s, &0x%p)", pCAggregate, GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL));

		//Handle Aggregation
		if(pCAggregate)
			TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CSession::GetDataSource
//
/////////////////////////////////////////////////////////////////
HRESULT CSession::GetDataSource(REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT		hr = S_OK;

	//This interface is required to continue...
	if(m_pIGetDataSource)
	{
		//IGetDataSource::GetDataSource
		XTEST(hr = m_pIGetDataSource->GetDataSource(riid, ppIUnknown));
		TESTC(TRACE_METHOD(hr, L"IGetDataSource::GetDataSource(%s, &0x%p)", GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL));
	}

CLEANUP:
	return hr;
}



/////////////////////////////////////////////////////////////////
// HRESULT CSession::OpenRowset
//
/////////////////////////////////////////////////////////////////
HRESULT CSession::OpenRowset(CAggregate* pCAggregate, DBID* pTableID, DBID* pIndexID, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown)
{
	HRESULT				hr = E_FAIL;			// HRESULT

	if(!m_pIOpenRowset)
		return E_FAIL;

	WCHAR wszTableName[MAX_QUERY_LEN+1];
	WCHAR wszIndexName[MAX_QUERY_LEN+1];

	// From IOpenRowset, get a rowset object
	DBIDToString(pTableID, wszTableName, MAX_QUERY_LEN);
	DBIDToString(pIndexID, wszIndexName, MAX_QUERY_LEN);
				
	XTEST_(hr = m_pIOpenRowset->OpenRowset(
							pCAggregate,		// pUnkOuter
							pTableID,			// pTableID
							pIndexID,			// pIndexID
							riid,				// refiid
							cPropSets,			// cProperties
							rgPropSets,			// rgProperties
							ppIUnknown),S_OK);	// IRowset pointer
	TRACE_METHOD(hr, L"IOpenRowset::OpenRowset(0x%p, %s, %s, %s, %d, 0x%p, &0x%p)", pCAggregate, wszTableName, wszIndexName, GetInterfaceName(riid), cPropSets, rgPropSets, ppIUnknown ? *ppIUnknown : NULL);

	//Display Errors (if occurred)
	TESTC(hr = DisplayPropErrors(hr, cPropSets, rgPropSets));

	//Handle Aggregation
	if(pCAggregate)
		TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));

CLEANUP:
	return hr;    
}

	
/////////////////////////////////////////////////////////////////
// HRESULT CSession::GetSchemaRowset
//
/////////////////////////////////////////////////////////////////
HRESULT CSession::GetSchemaRowset(CAggregate* pCAggregate, REFGUID guidSchema, ULONG cRestrictions, VARIANT* rgRestrictions, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown)
{
	HRESULT hr = S_OK;
	if(!m_pIDBSchemaRowset)
		return E_FAIL;
	
	//Schema Rowset
	WCHAR wszBuffer[MAX_NAME_LEN+1];
	WCHAR* pwszSchemaName = GetSchemaName(guidSchema);

	//Try to find the String Resprentation of the guidSchema
	if(pwszSchemaName == NULL)
	{
		StringFromGUID2(guidSchema, wszBuffer, MAX_NAME_LEN);
		pwszSchemaName = wszBuffer;
	}
	
	//GetSchema Rowset
	XTEST_(hr = m_pIDBSchemaRowset->GetRowset(
					pCAggregate,		// punkOuter
					guidSchema,			// schema IID
					cRestrictions,		// # of restrictions
					rgRestrictions,		// array of restrictions
					riid,				// rowset interface
					cPropSets,			// # of properties
					rgPropSets,			// properties
					ppIUnknown),S_OK);	// rowset pointer

	TRACE_METHOD(hr, L"IDBSchemaRowset::GetRowset(0x%p, %s, %d, 0x%p, %s, %d, 0x%p, &0x%p)", pCAggregate, pwszSchemaName, cRestrictions, rgRestrictions, GetInterfaceName(riid), cPropSets, rgPropSets, ppIUnknown ? *ppIUnknown : NULL);

	//Display Errors (if occurred)
	TESTC(hr = DisplayPropErrors(hr, cPropSets, rgPropSets));

	//Handle Aggregation
	if(pCAggregate)
		TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// HRESULT CSession::GetProviderTypes
//
////////////////////////////////////////////////////////////////
HRESULT CSession::GetProviderTypes()
{
	HRESULT			hr	= S_OK;

	HROW rghRows[MAX_BLOCK_SIZE];
	HROW* phRows = rghRows;
	DBCOUNTITEM cRowsObtained = 0;
	CComPtr<IUnknown>	spUnknown;
	CRowset cRowset(m_pCMainWindow);
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;

	//Setup the Bindings
	const static DBCOUNTITEM cBindings = 4;
	const static DBBINDING rgBindings[cBindings] = 
		{
			1,	 			
			offsetof(PROVTYPEINFO, wszTypeName),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			MAX_NAME_LEN,
			0, 				
			DBTYPE_WSTR,
			0,	
			0, 				

			2,	 			
			offsetof(PROVTYPEINFO, wType),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(DBTYPE),
			0, 				
			DBTYPE_UI2,
			0,	
			0, 				

			3,	 			
			offsetof(PROVTYPEINFO, ulColumnSize),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(DBLENGTH),	//TODO64: So is this schema going to be updated to allow for more than 4gig size?
			0, 				
			DBTYPE_UI4,			//TODO64: Sure would be nice to have defined type that toggels for the OS?
			0,	
			0, 				

			15,	 			
			offsetof(PROVTYPEINFO, iMaxScale),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(SHORT),
			0, 				
			DBTYPE_I2,
			0,	
			0,
		};

	//IDBSchemaRowset is optional
	//Only need to obtain this info once, so if already done, just exit
	if(m_pIDBSchemaRowset == NULL || m_cProvTypes != 0)
		goto CLEANUP;

	//PROVIDER_TYPES Rowset (is required if Schemas are supported)
	TESTC(hr = GetSchemaRowset(NULL, DBSCHEMA_PROVIDER_TYPES, 0, NULL, IID_IUnknown, 0, NULL, &spUnknown));
	TESTC(hr = cRowset.CreateObject(this, IID_IUnknown, spUnknown));
	
	//IAccessor::CreateAccessor
	TESTC(hr = cRowset.CreateAccessor(DBACCESSOR_ROWDATA, cBindings, (DBBINDING*)rgBindings, 0, &hAccessor));

	//Loop through the entire returned rowset
	while(TRUE)
	{
		TESTC(hr = cRowset.GetNextRows(0, MAX_BLOCK_SIZE, &cRowsObtained, &phRows));
		
		//ENDOFROWSET
		if(cRowsObtained==0) 
			break;
		
		//Alloc room for ProviderInfo (in chunks)
		SAFE_REALLOC(m_rgProvTypes, PROVTYPEINFO, m_cProvTypes + cRowsObtained);
		memset(&m_rgProvTypes[m_cProvTypes], 0, sizeof(PROVTYPEINFO) * (size_t)cRowsObtained);

		//Loop over rows obtained and get ProviderInfo
		for(ULONG i=0; i<cRowsObtained; i++) 
		{	
			//Get the Data
			TESTC(hr = cRowset.GetData(rghRows[i], hAccessor, (void*)&m_rgProvTypes[m_cProvTypes]));
			m_cProvTypes++;
		}
			
		//Release all the rows
		TESTC(hr = cRowset.ReleaseRows(cRowsObtained, rghRows));
	}

CLEANUP:
	cRowset.ReleaseAccessor(&hAccessor);
	return hr;
} 



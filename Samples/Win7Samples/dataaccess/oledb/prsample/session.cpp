//---------------------------------------------------------------------------
// Microsoft OLE DB Programmer's Reference Sample
// Copyright (C) 1998 By Microsoft Corporation.
//	  
// @doc
//												  
// @module SESSION.CPP
//
//---------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#include "prsample.h"		// Programmer's Reference Sample includes



/////////////////////////////////////////////////////////////////
// myCreateSession
//
//	Create an OLE DB Session object from the given DataSource
//	object. The IDBCreateSession interface is mandatory, so this
//	is a simple operation.
//
/////////////////////////////////////////////////////////////////
HRESULT	myCreateSession
	(
	IUnknown *				pUnkDataSource,
	IUnknown **				ppUnkSession
	)
{
	HRESULT					hr;
	IDBCreateSession *		pIDBCreateSession		= NULL;
	
	//Create a Session Object from a Data Source Object
	XCHECK_HR(hr = pUnkDataSource->QueryInterface(
				IID_IDBCreateSession, (void**)&pIDBCreateSession));
	XCHECK_HR(hr = pIDBCreateSession->CreateSession(
				NULL,				//pUnkOuter
				IID_IOpenRowset,	//riid
				ppUnkSession		//ppSession
				));

CLEANUP:
	if( pIDBCreateSession )
		pIDBCreateSession->Release();
	return hr;
}


/////////////////////////////////////////////////////////////////
// myCreateSchemaRowset
//
//	If the provider supports IDBSchemaRowset, this function will
//	obtain the tables schema rowset, will display this rowset to
//	the user, and will allow the user to select a row in the
//	rowset containing the name of a table of interest.
//
/////////////////////////////////////////////////////////////////
HRESULT	myCreateSchemaRowset
	(
	GUID					guidSchema, 
	IUnknown *				pUnkSession, 
	ULONG					cchBuffer, 
	LPWSTR					pwszBuffer
	)
{
	HRESULT					hr							= S_OK;
	IDBSchemaRowset *		pIDBSchemaRowset			= NULL;
	IUnknown *				pUnkRowset					= NULL;
	
	const ULONG				cProperties					= 2;
	DBPROP					rgProperties[cProperties];
	DBPROPSET				rgPropSets[1];

	// Attempt to obtain the IDBSchemaRowset interface on the Session object.
	// This is not a mandatory interface; if it is not supported, we are done
	CHECK_HR(pUnkSession->QueryInterface(
					IID_IDBSchemaRowset, (void**)&pIDBSchemaRowset));
	
	// Set properties on the rowset, to request additional functionality
	myAddRowsetProperties(rgPropSets, cProperties, rgProperties);

	// Get the requested schema rowset; if IDBSchemaRowset is supported,
	// the following schema rowsets are required to be supported:
	//	DBSCHEMA_TABLES, DBSCHEMA_COLUMNS, and DBSCHEMA_PROVIDERTYPES
	// We know that we will be asking for one of these, so it is not
	// necessary to call IDBSchemaRowset::GetSchemas in this case
	XCHECK_HR(hr = pIDBSchemaRowset->GetRowset(
				NULL,			//pUnkOuter
				guidSchema,		//guidSchema
				0,				//cRestrictions
				NULL,			//rgRestrictions
				IID_IRowset,	//riid
				1,				//cPropSets
				rgPropSets,		//rgPropSets
				&pUnkRowset		//ppRowset
				));

	// Display the rowset to the user; this will allow the user to
	// perform basic navigation of the rowset and will allow the user
	// to select a row containing a desired table name (taken from the
	// TABLE_NAME column)
	CHECK_HR(hr = myDisplayRowset(pUnkRowset, 
		L"TABLE_NAME", cchBuffer, pwszBuffer));

CLEANUP:
	if( pIDBSchemaRowset )
		pIDBSchemaRowset->Release();
	if( pUnkRowset )
		pUnkRowset->Release();
	return hr;
}



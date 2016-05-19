//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CRowsetObject Implementation Module| 	This module contains implementation information
// for rowset functions for the private library.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 01-20-95	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CRowsetObject Elements|
//
// @subindex CRowsetObject
//
//---------------------------------------------------------------------------

#include "privstd.h"
#include "coledb.hpp"

//--------------------------------------------------------------------
// Connection base class constructor.
//
// @mfunc CRowsetObject
//
// @parm [IN] Test case name
//
//--------------------------------------------------------------------
CRowsetObject::CRowsetObject(WCHAR * pwszTestCaseName) : CCommandObject(pwszTestCaseName)
{	
	m_rgTableColOrds	= NULL;	// Array of col ordinals for table
	m_cRowsetCols		= 0;		// Count of columns in rowset
	m_cRestrictions		= 0;	// number of restrictions to be used
	m_rgRestrictions	= NULL;	// Array of restrictions
}

//--------------------------------------------------------------------
// Connection base class destructor.
//
// @mfunc ~CRowsetObject
//
//--------------------------------------------------------------------
CRowsetObject::~CRowsetObject()
{
}

//--------------------------------------------------------------------
// This function is used to set the object's IAccessor interface
// pointer for the Rowset Object.  This function should be used when
// the Rowset was created by means other than CreateRowsetObject.
// This function and CRowsetTable::CreateRowsetObject are mutually exclusive 
// ways to set an IAccessor pointer for the object. <nl>
//
// NOTE:  An AddRef is done on this interface, so the caller is still responsible 
// for releasing the interface passed in, as well as calling ReleaseRowsetObject.
//
// @mfunc SetRowsetObject
//
//--------------------------------------------------------------------
BOOL CRowsetObject::SetRowsetObject
(
	IAccessor * pIAccessor		// @parm [IN] accessor on current rowset
)	
{
	HRESULT hr = S_OK;
	
	ICommand *			pICommand			= NULL;
	IOpenRowset *		pIOpenRowset		= NULL;
	IRowsetInfo *		pIRowsetInfo		= NULL;

	//Invalid check
	if(!pIAccessor)
		return FALSE;

	// Add Ref so the user can call ReleaseRowsetObject -- this will
	// make SetRowsetObject and CreateRowsetObject require the same cleanup.
	m_pIAccessor = pIAccessor;
	m_pIAccessor->AddRef();
	
	// Now we get as much information for the inherited classes as we can
	// note that the dso object can't currentl be retrieved
	// Get our DB Session, asking for the right interface
	// for generating rowsets
	if(!VerifyInterface(pIAccessor, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown **)&pIRowsetInfo))
		goto CLEANUP;

	// Try for command object
	hr = pIRowsetInfo->GetSpecification(IID_ICommand, (IUnknown **)&pICommand);

	//Command Object created the rowset
	if(hr == S_OK)
	{
		SetCommandObject(pICommand);
		goto CLEANUP; //done
	}

	//Command Object doesn't exist, so go get Session Object
	else
	{
		// Try for session object
		hr = pIRowsetInfo->GetSpecification(IID_IOpenRowset, (IUnknown **)&pIOpenRowset);

		//Found Session Object
		if(hr == S_OK)
		{
			//Since we found session object, note that we didn't find command object
			SetDBSession(pIOpenRowset);
			goto CLEANUP;
		}
		else
		{
			//Unable to find Specification
			//Create a new session
			if(FAILED(hr = CreateDBSession(EITHER_GENERATED)))
				goto CLEANUP;
		}
	}
	
CLEANUP:
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIOpenRowset);
	return hr==S_OK;
}

//--------------------------------------------------------------------
// Sets rowset properties desired for creation of rowset object
// when calling CreateRowsetObject. <nl>
// User must allocate memory for rgProperties, and then this object
// will be responsible for freeing it. <nl>
// To change properties, user simply calls this method again, specifying
// every property desired on the rowset. <nl>
// To reset properties to nothing, user calls this method with NULL,0
//
// @mfunc SetRowsetProperties
//
//--------------------------------------------------------------------
HRESULT CRowsetObject::SetRowsetProperties
(
	DBPROPSET	rgPropSets[],		//@parm [IN] Array of DBPROPSET structures.
	ULONG		cPropSets			//@parm [IN] Count of DBPROPSET structures.
)
{
	ULONG	i;

	// Make sure as far as we can tell that user passed parameters correctly
	if (cPropSets)
	{	ASSERT(rgPropSets);	}

	// Release previous array first
	FreeProperties(&m_cPropSets, &m_rgPropSets);

	// Allocate memory for our data member
	m_rgPropSets = (DBPROPSET *)PROVIDER_ALLOC(cPropSets * sizeof(DBPROPSET));
	
	if (!m_rgPropSets)
		return E_OUTOFMEMORY;

	// Record the new number of property sets
	m_cPropSets = cPropSets;

	// Copy the properties to our data members -- rgProperties
	// we'll alloc a new buffer for next, but for now just copy
	// the old address into m_rgPropSets[x].rgProperties.
	memcpy(m_rgPropSets, rgPropSets, cPropSets * sizeof(DBPROPSET));

	// Walk Set array, allocing and setting all rgProperties arrays we'll need
	for (i=0; i<m_cPropSets; i++)
	{
		m_rgPropSets[i].rgProperties = 
			(DBPROP *)PROVIDER_ALLOC(m_rgPropSets[i].cProperties * sizeof(DBPROP));

		if (!m_rgPropSets[i].rgProperties)
			return E_OUTOFMEMORY;

		// Copy data from passed rgProperties to our new buffer
		memcpy(m_rgPropSets[i].rgProperties, rgPropSets[i].rgProperties, 
			m_rgPropSets[i].cProperties * sizeof(DBPROP));
	}		

	return NOERROR;
}

//--------------------------------------------------------------------
// Releases all interfaces on the Rowset Object, Command object,
// DB Session and Data Source Object.
//
// @mfunc ReleaseRowsetObject
//
//--------------------------------------------------------------------
void CRowsetObject::ReleaseRowsetObject
(
	DBREFCOUNT ulRefCount	//@parm [IN] Expected Ref Count for object
						//			   after release is done.  Default is zero,
						//			   meaning the object is expected to be deleted.
)
{	
	// Release array generated in ExecuteCommand call
	PROVIDER_FREE(m_rgTableColOrds);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SAFE_RELEASE(m_pIAccessor);
	ReleaseRestrictions();
}

//--------------------------------------------------------------------
// Gets a Rowset object based on m_pTable, using the
// query passed in, and sets m_pIAccessor to the IAccessor interface for
// the rowset object.
// If the user wants specific properties for the rowset, they should set 
// them before hand by assigning calling SetRowsetProperties, and checking
// after execution m_rgPropertyErrors.
// If SetRowsetProperties is called, the user must call ClearPropertyErrors
// following any subsequent call to CreateRowsetObject, to clear the
// error array which is allocated when the properties are set during rowset creation.
//
// It is assumed that the table created in this function should be destroyed 
// when the rowset is released. <nl>
//
// NOTE:  The user must call the following functions to be fully released
// from the provider.  They may call a subset of these functions to release
// only the objects they choose:
//		ReleaseRowsetObject
//      ReleaseCommandObject
//		ReleaseDBSession
//		ReleaseDataSourceObject
//
// @mfunc CreateRowsetObject
//
//--------------------------------------------------------------------
HRESULT	CRowsetObject::CreateRowsetObject
(
	EQUERY		eQuery,		//@parm [IN] Query to generate rowset with
	REFIID		riid,		//@parm [IN] What riid to ask for 
	EEXECUTE	eExecute,	//@parm [IN] whether to always execute the command
	IUnknown**  ppIRowset
)
{
	HRESULT				hr				 = S_OK;
	IDBSchemaRowset *	pIDBSchemaRowset = NULL;
	IUnknown *			pRowsetIUnk		 = NULL;

	//Get Rowset object if we haven't done so already
	if(m_pIAccessor)
		goto CLEANUP;

	// Create the Session object
	QTESTC_(hr = CreateDBSession(OPENROWSET_GENERATED),S_OK);

	// Create table if we haven't already -- assumes that
	// if an object exists, CreateTable or SetExistingTable
	// has been done on it already
	if(!m_pTable)
	{
		m_pTable = new CTable(m_pIOpenRowset, m_pwszTestCaseName);
		if(!m_pTable)
		{				
			hr = E_OUTOFMEMORY;
			goto CLEANUP;
		}
		
		QTESTC_(hr = m_pTable->CreateTable(NUM_ROWS),S_OK);
	}
	
	//The user may have requested through the InitString exactly how to create the
	//rowset, so override any parameter passed in if this is the case...
	if(GetModInfo()->GetRowsetQuery() != NO_QUERY)
		eQuery = GetModInfo()->GetRowsetQuery();

	//Commands may not be supported...
	if(!m_pTable->GetCommandSupOnCTable() && (eQuery == SELECT_ALLFROMTBL || eQuery == SELECT_ORDERBYNUMERIC))
		eQuery = USE_SUPPORTED_SELECT_ALLFROMTBL;

	//Determine how the create the rowset...
	switch(eQuery)
	{
		case SELECT_DBSCHEMA_ASSERTIONS:
		case SELECT_DBSCHEMA_CATALOGS:
		case SELECT_DBSCHEMA_CHARACTER_SETS:
		case SELECT_DBSCHEMA_CHECK_CONSTRAINTS:
		case SELECT_DBSCHEMA_COLLATIONS:
		case SELECT_DBSCHEMA_COLUMN_DOMAIN_USAGE:
		case SELECT_DBSCHEMA_COLUMN_PRIVILEGES:
		case SELECT_DBSCHEMA_COLUMNS:
		case SELECT_DBSCHEMA_CONSTRAINT_COLUMN_USAGE:
		case SELECT_DBSCHEMA_CONSTRAINT_TABLE_USAGE:
		case SELECT_DBSCHEMA_FOREIGN_KEYS:
		case SELECT_DBSCHEMA_INDEXES:
		case SELECT_DBSCHEMA_KEY_COLUMN_USAGE:
		case SELECT_DBSCHEMA_PRIMARY_KEYS:
		case SELECT_DBSCHEMA_PROCEDURE_PARAMETERS:
		case SELECT_DBSCHEMA_PROCEDURES:
		case SELECT_DBSCHEMA_PROVIDER_TYPES:
		case SELECT_DBSCHEMA_REFERENTIAL_CONSTRAINTS:
		case SELECT_DBSCHEMA_SCHEMATA:
		case SELECT_DBSCHEMA_SQL_LANGUAGES:
		case SELECT_DBSCHEMA_STATISTICS:
		case SELECT_DBSCHEMA_TABLE_CONSTRAINTS:
		case SELECT_DBSCHEMA_TABLE_PRIVILEGES:
		case SELECT_DBSCHEMA_TABLE:
		case SELECT_DBSCHEMA_TRANSLATIONS:
		case SELECT_DBSCHEMA_USAGE_PRIVILEGES:
		case SELECT_DBSCHEMA_VIEW_COLUMN_USAGE:
		case SELECT_DBSCHEMA_VIEW_TABLE_USAGE:
		case SELECT_DBSCHEMA_VIEWS:

		case USE_ENUMERATOR:
		case USE_OPENROWSET:
		case USE_SUPPORTED_SELECT_ALLFROMTBL:
		case USE_COLUMNSROWSET:
		{
			//Use CursorEngine ( if requested )
			if(GetModInfo()->UseServiceComponents() & SERVICECOMP_CURSORENGINE)
				SetProperty(DBPROP_CLIENTCURSOR, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets, DBTYPE_BOOL, VARIANT_TRUE);

			//Delegate to our CreateRowset method
			QTESTC(SUCCEEDED(hr=m_pTable->CreateRowset(	eQuery,
														riid, 
														m_cPropSets, 
														m_rgPropSets, 
														(IUnknown**)&pRowsetIUnk, 
														NULL, 
														&m_cRowsetCols, 
														&m_rgTableColOrds, 
														m_cRestrictions, 
														m_rgRestrictions,
														m_pIOpenRowset)) );
			break;
		}

		default:
		{
			QTESTC_(hr = CreateDBSession(COMMAND_GENERATED),S_OK);
						
			// Create second table if needed and not created already
			if ((eQuery == SELECT_CROSSPRODUCT)  ||
				(eQuery == SELECT_LEFTOUTERJOIN) ||
				(eQuery == SELECT_RIGHTOUTERJOIN_ESC)||
				(eQuery == SELECT_LEFTOUTERJOIN_ESC) ||
				(eQuery == SELECT_RIGHTOUTERJOIN))
			{
				if (!m_pTable2)
				{
					m_pTable2 = new CTable(m_pIOpenRowset, m_pwszTestCaseName);
					if (!m_pTable2)
					{				
						hr = E_OUTOFMEMORY;
						goto CLEANUP;
					}
					
					QTESTC_(hr = m_pTable2->CreateTable(NUM_ROWS), S_OK);
				}
			}
			
			// Create command object if we haven't done so already
			QTESTC_(hr = CreateCommandObject(),S_OK);
			
			// Execute the query to generate a rowset object
			// Note, all allocated memory goes into data members, which the
			// destructor will clean up
			//
			// Second table is passed in regardless if needed, assume
			// ExecuteCommand won't do anything harmful with it
			QTESTC_(hr=m_pTable->ExecuteCommand(eQuery, riid,
				m_pTable2 ? m_pTable2->GetTableName() : NULL, NULL, 
				&m_cRowsetCols, &m_rgTableColOrds, eExecute, 
				m_cPropSets, m_rgPropSets, NULL,
				(IUnknown **)&pRowsetIUnk, &m_pICommand),S_OK);
			break;
		}
	}

	// Now set m_pIAccessor for this object, since
	// this is the one we want to keep.  We will
	// get rid of the first interface in CLEANUP,
	// which we requested just to ensure we got the type
	// of rowset we wanted (simple or regular)
	if(!VerifyInterface(pRowsetIUnk, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&m_pIAccessor))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP; 			
	}

CLEANUP:
	SAFE_RELEASE(pIDBSchemaRowset);
	if(ppIRowset)
		*ppIRowset = pRowsetIUnk;
	else
		SAFE_RELEASE(pRowsetIUnk);
	return hr;		
}

//--------------------------------------------------------------------
// Generates an IColumnsRowset based on rowset currently held in
// this class.Returns all optional columns in rowset.
//
// @mfunc CreateColumnRowset
//
//--------------------------------------------------------------------
HRESULT CRowsetObject::CreateColumnRowset(void)
{
	HRESULT				hr					= E_FAIL;
	IColumnsRowset *	pIColumnsRowset		= NULL;
	DBID *				rgOptionalColumns	= NULL;
	DBORDINAL			cOptionalColumns	= 0;
	
	// We must have an Accessor pointer, meaning there is a Rowset created
	if(!m_pIAccessor)
		return hr;

	// Release the current ColumnRowset if needed
	SAFE_RELEASE(m_pColRowset);

	// Obtain IColumnsRowset
	if (!VerifyInterface(m_pIAccessor, IID_IColumnsRowset, 
						ROWSET_INTERFACE, (IUnknown **)&pIColumnsRowset))
		return hr;

	// GetAvailableColumns
	if (FAILED(hr=pIColumnsRowset->GetAvailableColumns(
									&cOptionalColumns, &rgOptionalColumns)))
		goto CLEANUP;

	// GetColumnsRowset
	if (FAILED(hr=pIColumnsRowset->GetColumnsRowset(NULL, cOptionalColumns,
					rgOptionalColumns, IID_IRowset, 0, NULL, &m_pColRowset)))
			goto CLEANUP;

CLEANUP:
	SAFE_RELEASE(pIColumnsRowset);
	PROVIDER_FREE(rgOptionalColumns);
	return hr;
}


//--------------------------------------------------------------------
//
// @mfunc Releases the restrictions array
//
//--------------------------------------------------------------------
void CRowsetObject::ReleaseRestrictions()
{
	ULONG	cRestrictions;

	for (cRestrictions=0; cRestrictions < m_cRestrictions; cRestrictions++)
	{
		VariantClear(&m_rgRestrictions[cRestrictions]);
	}
	SAFE_FREE(m_rgRestrictions);
	m_cRestrictions		= 0;
	m_rgRestrictions	= NULL;
} // CRowsetObject::ReleaseRestrictions()




//--------------------------------------------------------------------
//	Function takes an array of restrictions and uses it fo schema rowsets
//
// @mfunc Set the restrictions to be used for a schema rowset
//
//--------------------------------------------------------------------
BOOL CRowsetObject::SetRestrictions(ULONG cRestrictions, VARIANT *rgRestrictions)
{
	BOOL	fRes = FALSE;

	ReleaseRestrictions();
	TESTC(0 < cRestrictions && NULL != rgRestrictions);
	SAFE_ALLOC(m_rgRestrictions, VARIANT, cRestrictions);
	for (m_cRestrictions = 0; m_cRestrictions < cRestrictions; m_cRestrictions++)
	{
		VariantInit(&m_rgRestrictions[m_cRestrictions]);
		TESTC_(VariantCopy(	&m_rgRestrictions[m_cRestrictions], 
							&rgRestrictions[m_cRestrictions]), S_OK);
	}
	m_cRestrictions = cRestrictions;
	fRes = TRUE;
CLEANUP:
	return fRes;
} // CRowsetObject::SetRestrictions

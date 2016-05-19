//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CTable Implementation Module | 	This module contains definition information
//						for CCol class for the private library.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 06-30-95	Microsoft	Created <nl>
//	[02] 10-10-95	Microsoft	Replace CStrings with WCHAR * <nl>
//  [03] 03-21-96   Microsoft	Added SELECT_CROSSPRODUCT case to CreateSQLStmt <nl>
//	[04] 01-20-96	Microsoft	Moved GetAccessorAndBindings to miscfunc.cpp <nl>	
//	[05] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CTable Elements|
//
// @subindex CTable
//
//---------------------------------------------------------------------------

#include "privstd.h"
#include "privcnst.h"
#include "miscfunc.h"
#include "ctable.hpp"
#include <winnls.h>
#include <locale.h>

#include <time.h>		//time routines

//---------------------------------------------------------------------------
// CTable::CTable		
// Constructor, initializes member variables. This constructor
// also runs CoGetMalloc and new CError. CError is set to HR_STRICT.
//		
//	@mfunc CTable	
//		
//---------------------------------------------------------------------------
CTable::CTable(
	IUnknown *	pSessionIUnknown,			// @parm [IN] Session object's IUnknown -- must be valid 
	WCHAR *		pwszModuleName,				// @parm [IN] Name of test module (Default = NULL)
	ENULL 		eNull						// @parm [IN] Will nulls be used where possible (DEFAULT=USENULLS)
) : CSchema(NULL, NULL, -1, eNull)
{
	ASSERT(pSessionIUnknown);
	HRESULT hr = S_OK;
	IGetDataSource* pIGetDataSource = NULL;

	m_pError = new CError(HR_STRICT);
	SetModuleName(pwszModuleName);
	
	m_pIDBInitialize	= NULL;
	m_pIOpenRowset		= NULL;
	m_pIDBCreateCommand = NULL;

	// Check to see if the Provider is readonly
	m_fProviderReadOnly = IsProviderReadOnly(pSessionIUnknown);

	//Obtain DataSource 
	VerifyInterface(pSessionIUnknown, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDataSource);
	pIGetDataSource->GetDataSource(IID_IDBInitialize, (IUnknown**)&m_pIDBInitialize);

	//Obtain IOpenRowset 
	VerifyInterface(pSessionIUnknown, IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&m_pIOpenRowset);
	
	//Obtain IDBCreateCommand if supported
	VerifyInterface(pSessionIUnknown, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&m_pIDBCreateCommand);

	m_ulNextRow				 = 1;
	m_ulRows				 = 0;
	m_pICommand				 = NULL;
	m_pwszViewName			 = NULL;
	m_pwszIndexName			 = NULL;

	//IDBInfo Literals
	m_cLiteralInfo			= 0;
	m_rgLiteralInfo			= NULL;
	m_pwszLiteralInfoBuffer	= NULL;

	m_pUnkOuter				= NULL;
	m_riid					= (struct _GUID*)&IID_IRowset;
	m_ppTableID				= NULL;
	m_ppRowset				= NULL;
	m_cPropertySets			= 0;
	m_rgPropertySets		= NULL;
	m_cColumnDesc			= 0;
	m_rgColumnDesc			= NULL;
	m_fInputTableID			= TRUE;	// use &m_TableID in ITableDefinition::CreateTable
	m_fBuildColumnDesc		= TRUE;

	//TableName
	m_TableID.uGuid.guid	 = GUID_NULL;
	m_TableID.eKind			 = DBKIND_NAME;
	m_TableID.uName.pwszName = NULL;

	//Setup IDBInfo (LiteralInfo) to be used by all methods...
	SetupLiteralInfo();
	
	SetHasAuto(FALSE);
	SetHasUnique(FALSE);
	SetHasCLSID(FALSE);
	SetHasDefault(FALSE);

	// Default it to UPPER
	m_ulIdentifierCase = DBPROPVAL_IC_UPPER;


	m_pSchemaCache = new CSchema(NULL, NULL, -1, eNull);

	//Cleanup
	SAFE_RELEASE(pIGetDataSource);
}

//---------------------------------------------------------------------------
// CTable::~CTable
// Destructor.  Frees m_pError
//	
// @mfunc	~CTable	
//	
//---------------------------------------------------------------------------
CTable::~CTable(void)
{
	POSITION pos;
	POSITION posSave;

	// Get top to list
	pos=m_ColList.GetHeadPosition();
	
	// While not at end of list
	while (pos!=NULL) 
	{		
		// Save the position for deleting and then get the next element
		posSave = pos;
		m_ColList.GetNext(pos);
		
		// Put column name in list
		m_ColList.RemoveAt(posSave);
	}

	m_ColList.RemoveAll();

	//Clear the current TableName
	SetTableName(NULL);
	
	PROVIDER_FREE(m_pwszViewName);
	PROVIDER_FREE(m_pwszIndexName);
	PROVIDER_FREE(m_pwszModuleName);

	//IDBInfo Literals
	SAFE_FREE(m_rgLiteralInfo);
	SAFE_FREE(m_pwszLiteralInfoBuffer);

	SAFE_RELEASE(m_pIDBInitialize);
	SAFE_RELEASE(m_pIDBCreateCommand);
	SAFE_RELEASE(m_pIOpenRowset);
	SAFE_RELEASE(m_pICommand);

	FreeProperties(&m_cPropertySets, &m_rgPropertySets);
	ReleaseColumnDesc(m_rgColumnDesc, m_cColumnDesc);

	SAFE_DELETE(m_pError);
	SAFE_DELETE(m_pSchemaCache);
}

//---------------------------------------------------------------------------
// CTable::SetTableName	
//	
// @mfunc	SetTableName
//	
//---------------------------------------------------------------------------
HRESULT CTable::SetTableName(WCHAR* pwszTableName)
{
	//We allow setting to NULL
	//If its been allocated before, first deallocate
	PROVIDER_FREE(m_TableID.uName.pwszName);
	m_TableID.uName.pwszName = wcsDuplicate(pwszTableName);
	
	return m_TableID.uName.pwszName ? S_OK : E_OUTOFMEMORY;
}


//---------------------------------------------------------------------------
// CTable::SetViewName	
//	
// @mfunc	SetViewName
//	
//---------------------------------------------------------------------------
HRESULT CTable::SetViewName(
	WCHAR * pViewName			// @parm [IN] View Name
)
{
	// Make sure view name is empty
	PROVIDER_FREE(m_pwszViewName);
	m_pwszViewName = wcsDuplicate(pViewName);

	return m_pwszViewName ? S_OK : E_OUTOFMEMORY;
}


//---------------------------------------------------------------------------
// CTable::ResetCreateCommand
//			
// This function should only be called within the same Data Source object.
// The intention of this method is for CTransaction class so that when 
// ExecuteCommand is called, a new pIDBCreateCommand is used for the new 
// transacion.		
//
// @mfunc ResetCreateCommand
// 	
//---------------------------------------------------------------------------
HRESULT	CTable::ResetCreateCommand(
	IDBCreateCommand*	pIDBCreateCommand	//@parm [IN] IDBCreateCommand pointer
)
{
	// Release the old DB Session object
	SAFE_RELEASE(m_pIDBCreateCommand);
	SAFE_RELEASE(m_pICommand);
	
	//Used the passed in IDBCreateCommand interface
	m_pIDBCreateCommand	= pIDBCreateCommand;
	SAFE_ADDREF(m_pIDBCreateCommand);

	return S_OK;
}

//-------------------------------------------------------
// Check to see if we can create non null columns or not.
// 	
// @mfunc IsNonNullColumnsAllowed
// 	
//---------------------------------------------------------------------------
BOOL CTable::IsNonNullColumnsAllowed()
{
	BOOL	bDone	= FALSE;
	VARIANT	vActualValue;

	VariantInit(&vActualValue);

	if ((GetProperty(DBPROP_COLUMNDEFINITION, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &vActualValue)) && 
		(DBTYPE_I4 == V_VT(&vActualValue)) &&
		(DBPROPVAL_CD_NOTNULL == V_I4(&vActualValue)))
		bDone = TRUE;

	VariantClear(&vActualValue);
	
	return bDone;
}

//-------------------------------------------------------
// Check the SQL Support of the Provider.
// 	
// @mfunc ProviderSQLSupport
// 	
//---------------------------------------------------------------------------
void CTable::ProviderSQLSupport()
{
	ULONG_PTR ulValue;

	// Initialize
	SetSQLSupport(0);

	// DBPROP_SQLSUPPORT
	if(GetProperty(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulValue))
		SetSQLSupport(ulValue);
}

//---------------------------------------------------------------------------
//	CTable::CreateTable	#1		
//
// CTable			|
// CreateTable		|
// Create Table with all data types in the Data Source. If ppwszTableName
// is NULL then this function generates a table name. If table is successfully
// created, check if index should be created and insert # of rows requested. 
// 
// 1) Set m_ColList
// 2) Goto QCreateTable
//
// @mfunc CreateTable
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT	CTable::CreateTable(	
	DBCOUNTITEM ulRowCount, 			// @parm [IN] # of Rows to insert into table, 1 based
	DBORDINAL ulIndex,				// @parm [IN] Column Number of index, 1 based (Default=1)
	WCHAR *pwszTableName,		// @parm [IN] TableName, if NULL call MakeTableName() (Default=NULL)
	EVALUE eValue,				// @parm [IN] Insert PRIMARY or SECONDARY data (Default=PRIMARY)
	BOOL fFirstUpdateable		// @parm [IN] TRUE means first column will be autoinc (Default=FALSE)
)		
{
	HRESULT hr = S_OK;

	// These are necessary until CreateColInfo takes pointers
	CList <WCHAR * ,WCHAR *> ListNativeTemp;
	CList <DBTYPE,DBTYPE> ListDataTypes;

	// Get the SQL Support of the Provider
	ProviderSQLSupport();

	// DBPROP_IDENTIFIERCASE
	GetProperty(DBPROP_IDENTIFIERCASE, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &m_ulIdentifierCase);

	// If the user passes in a TableName in the Init String, we will use 
	// SetExistingTable otherwise we will assume we can create a table
	if(GetModInfo()->GetTableName())
	{
		if(SUCCEEDED(hr = SetExistingTable(GetModInfo()->GetTableName())))
		{
			if(GetModInfo()->GetFileName())
			{
				hr = SetExistingCols(GetModInfo()->GetParseObject()->GetColList());
			}
		}
		goto CLEANUP;
	}

	//Get Table Name
	if(FAILED(hr = MakeTableName(pwszTableName)))		
		goto CLEANUP;

	// Can't pass nulls because last 2 params are references
	// if m_fBuildColumnDesc is TRUE, the table will be build from an array of DBCOLUMNDESC, so
	// there is no need to recreate m_ColList
	if(FAILED(hr=CreateColInfo(ListNativeTemp,ListDataTypes,ALLTYPES,fFirstUpdateable)))
		goto CLEANUP;
	
	// Create Table
	hr = QCreateTable(ulRowCount,ulIndex,eValue);
		
CLEANUP:
	//Make sure the returned table has at least the number of requested rows...
	if(SUCCEEDED(hr) && GetRowsOnCTable() < ulRowCount)
	{
		odtLog << L"Table contains " << GetRowsOnCTable() << " rows, but the test requires at least " << ulRowCount << " rows\n";
		hr = E_FAIL;
	}

	return hr;
}

//---------------------------------------------------------------------------
// CTable::CreateTable #2	
//
// CTable			|
// CreateTable		|
// Create Table with requested data types in the Data Source. If ppwszTableName
// is NULL then this function generates a table name. If table is successfully
// created, check if index should be created and insert # of rows requested. 
//	
// 1) Set m_ColList
// 2) Goto QCreateTable
//
// @mfunc CreateTable
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateTable(	
	CList 		<DBTYPE,DBTYPE>& DBProviderTypesList,	// @parm [IN] List of types of columns
	DBCOUNTITEM	ulRowCount, 							// @parm [IN] Number of rows to create, 1 based
	DBORDINAL	ulIndex,								// @parm [IN] Column ordinal to put index on (Default = 1)
	WCHAR *		pwszTableName,							// @parm [IN] Table name, (Default = NULL)
	EVALUE 		eValue,									// @parm [IN] Type of data for MakeData (Default = Primary)
	BOOL		fFirstUpdateable						// @parm [IN] TRUE means first column will be autoinc (Default=FALSE)
)
{
	HRESULT hr = S_OK;
	CList 	<WCHAR *,WCHAR*> ListNativeTemp;

	// Get the SQL Support of the Provider
	ProviderSQLSupport();

	// If the user passes in a TableName in the Init String, we will use 
	// SetExistingTable otherwise we will assume we can create a table
	if(GetModInfo()->GetTableName())
	{
		if(SUCCEEDED(hr = SetExistingTable(GetModInfo()->GetTableName())))
			if(GetModInfo()->GetFileName())
				hr = SetExistingCols(GetModInfo()->GetParseObject()->GetColList());
		goto CLEANUP;
	}

	//Get table name
	if(FAILED(hr = MakeTableName(pwszTableName)))
		goto CLEANUP;

	// Can't pass nulls because last 2 params are references
	if(FAILED(hr=CreateColInfo(ListNativeTemp,DBProviderTypesList,DBDATATYPES,fFirstUpdateable)))
		goto CLEANUP;

	// Create Table
	if(FAILED(hr=QCreateTable(ulRowCount,ulIndex,eValue)))
		goto CLEANUP;

CLEANUP:
	//Make sure the returned table has at least the number of requested rows...
	if(SUCCEEDED(hr) && GetRowsOnCTable() < ulRowCount)
	{
		odtLog << L"Table contains " << GetRowsOnCTable() << " rows, but the test requires at least " << ulRowCount << " rows\n";
		hr = E_FAIL;
	}
	return hr;	
}

//---------------------------------------------------------------------------
//	CTable::CreateTable #3	
//
//			CTable			|
//			CreateTable		|
//			Create Table with requested backend types in the Data Source. If ppwszTableName
//  is NULL then this function generates a table name. If table is successfully
//  created, check if index should be created and insert # of rows requested. 
//
//	1) Set m_ColList
//	2) Goto QCreateTable
//
//	@mfunc	CreateTable
// 	@rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateTable(	
	CList 		<WCHAR * ,WCHAR* >& NativeTypesList,	// @parm [IN] List of types
	DBCOUNTITEM	ulRowCount, 							// @parm [IN] Number of rows to create
	DBORDINAL	ulIndex,								// @parm [IN] Column ordinal to put index on (Default = 1)
	WCHAR * 	pwszTableName,							// @parm [IN] Table name (Default = NULL)
	EVALUE 		eValue,									// @parm [IN] Type of data for MakeData (Default = Primary)
	BOOL		fFirstUpdateable						// @parm [IN] TRUE means first column will be autoinc (Default=FALSE)
)
{
	HRESULT hr = S_OK;
	CList <DBTYPE,DBTYPE> ListDataTypes;

	// Get the SQL Support of the Provider
	ProviderSQLSupport();

	// If the user passes in a TableName in the Init String, we will use 
	// SetExistingTable otherwise we will assume we can create a table
	if(GetModInfo()->GetTableName())
	{
		if(SUCCEEDED(hr = SetExistingTable(GetModInfo()->GetTableName())))
			if(GetModInfo()->GetFileName())
				hr = SetExistingCols(GetModInfo()->GetParseObject()->GetColList());
		goto CLEANUP;
	}

	// Get Table Name
	if(FAILED(hr = MakeTableName(pwszTableName)))
		goto CLEANUP;

	// Can't pass nulls because last 2 params are references
	if(FAILED(hr=CreateColInfo(NativeTypesList,ListDataTypes,NATIVETYPES,fFirstUpdateable)))
		goto CLEANUP;

	if(FAILED(hr=QCreateTable(ulRowCount,ulIndex,eValue)))
		goto CLEANUP;

CLEANUP:
	//Make sure the returned table has at least the number of requested rows...
	if(SUCCEEDED(hr) && GetRowsOnCTable() < ulRowCount)
	{
		odtLog << L"Table contains " << GetRowsOnCTable() << " rows, but the test requires at least " << ulRowCount << " rows\n";
		hr = E_FAIL;
	}
	return hr;
}


//---------------------------------------------------------------------------
// CTable::CreateTypeColInfo 
//
// CTable				|
// CreateTypeColInfo	|
// Create column information, initializes m_ColList with columns of 
// specified types. Because temp table now puts bookmarks in the schema table,
// we have to get the bookmark column out the the DBCOLUMNINFO, before we call
// to get the bindings.
//
// @mfunc CreateTypeColInfo
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateTypeColInfo(
	CList<WCHAR *,WCHAR *>& NativeTypesList,	//  @parm [OUT] List of native data types 
	CList<DBTYPE,DBTYPE>& ProviderTypesList,	//  @parm [OUT] List of provider data types	
	EDATATYPES eDataTypes,						//  @parm [IN] Enum for column data type (Default = ALLTYPES)
	ULONG *pulAutoIncPrec						//  @parm [OUT] Precision of largest Auto Inc column (optional)

)
{
	HRESULT			hr = S_OK;			// Returned result
	IDBSchemaRowset	*pSchemaRowset=NULL;// To get Schema Types Rowset
	IRowset *		pIRowset=NULL;		// IRowset interface
	DBLENGTH		ulOffset;			// Offset into data buffer
	HACCESSOR		hAccessRead;		// HACCESSOR
	IAccessor *		pIAccessor=NULL;	// IAccessor interface
	IColumnsInfo * 	pIColumnsInfo=NULL;	// IColumnsInfo interface
	DBORDINAL		cColumns=0;			// Count of columns in ColumnsInfo
	DBCOLUMNINFO *  rgCOLUMNINFO=NULL;	// ColumnsInfo array
	WCHAR *			pStringsBuffer=NULL;// String values for IColumnsInfo
	DB_LORDINAL *	rgColsToBind=NULL;	// Array of Columns to Bind
	DBORDINAL		cColsToBind=0;		// Count of Columns to Bind
	DBORDINAL		index;				// Counter
	DBCOUNTITEM		ulcRowObt;			// Number of rows obtained from GetNextRows
	HROW *			rgHrow=NULL;		// HROW
	DBCOUNTITEM		uliRow;				// Row index
	WCHAR *			strElem=NULL;		// Element of the list of Native data types
	WCHAR 			strColName[MAXDATALEN];		// Column name
	WCHAR			*pdest;				// Pointer in the Dest String
	WCHAR *			pwszUnicodeInt=NULL;// Pointer to a Unicode string representing an integer
	POSITION		pos;				// Position in data types list
	INT_PTR			icList;				// Count of data types in the list
	int				icFoundList;		// Count of data types from list found

	DBTYPE *		rgList=NULL;		// array of dbtypes handled so far so don't repeat for DBDATATYPES
	int				cList=0;			// count of dbtypes handled so far
	
	DBCOUNTITEM		cBindings = 0;		// Count of columns in Schema TYPES Rowset
	DBBINDING *		rgBindings = NULL;	// Array of DBBINDINGs
	
	DBTYPE			dbProvElem;			// Element of the list of provider data types
	void*			pData = NULL;		// Data buffer for GetData
	UDWORD			udwColNum;			// Column number
	ULONG			ulAutoIncPrec = 0;	// Precision of largest auto increment column
	ULONG			cRefCounts=ULONG_MAX;// Init to a bogus count for testing purposes	
	WCHAR *			pwszInitValue = NULL; // Init string value for allowed types

	// See if the user has limited the data types via ALLOWED_TYPES keyword
	// Note: these must be provider type names.
	GetModInfo()->GetInitStringValue(L"ALLOWED_TYPES", &pwszInitValue);

	if(!VerifyInterface(m_pIOpenRowset, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pSchemaRowset))
	{
		// We should never be getting here.
		// If the provider doesn't support SchemaInfo, then you should be passing 
		// a TABLE=tablename to use, since we can't just guess on types...
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	// Cache Schema Information
	if( !CHECK(hr = m_pSchemaCache->PopulateTypeInfo(pSchemaRowset, m_lSQLSupport), S_OK) )
		goto CLEANUP;

	// Get Schema PROVIDER_TYPES Rowset data types
	if(!CHECK(hr = pSchemaRowset->GetRowset(NULL, DBSCHEMA_PROVIDER_TYPES, 0, NULL, 
			IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK))
			goto CLEANUP;

	// Get pointer to IColumnsInfo interface
	if (!VerifyInterface(pIRowset, IID_IColumnsInfo, ROWSET_INTERFACE, (IUnknown**)&pIColumnsInfo))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	// Get the ColumnInfo for DBSCHEMA_TYPES 
	if (!CHECK(hr=pIColumnsInfo->GetColumnInfo(&cColumns, 
								&rgCOLUMNINFO, &pStringsBuffer),S_OK))
		goto CLEANUP;

	// Get memory to hold array of all col numbers.  
	rgColsToBind = (DB_LORDINAL *)PROVIDER_ALLOC(cColumns * sizeof(DB_LORDINAL));

	if (!rgColsToBind)
	{
		hr = E_OUTOFMEMORY;
		goto CLEANUP;
	}

	// Loop from 1 to cDBCOLUMNINFO
	for(index=0; index < cColumns; index++)
	{
		if (rgCOLUMNINFO[index].iOrdinal)
		{
			rgColsToBind[cColsToBind] = rgCOLUMNINFO[index].iOrdinal;
			cColsToBind++;
		}
	}

	// Release the Interface and Free the Memory
	SAFE_RELEASE(pIColumnsInfo);	

	PROVIDER_FREE(rgCOLUMNINFO);
	PROVIDER_FREE(pStringsBuffer);

	// Get pointer to IAccessor interface
	if (!VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	// Create an Accessor and DBBINDINGs
	//NOTE:  We need to bind BLOB columns, so we are binding all columns
	//from the schema rowset, incase the provider returned IS_LONG 
	if (!CHECK(hr = GetAccessorAndBindings
	(
		pIRowset,			// IN: 	IRowset interface
		DBACCESSOR_ROWDATA,
		&hAccessRead,		// OUT:	Accessor from CreateAccessor
		&rgBindings,		// OUT:	Array of DBBINDINGs
		&cBindings,			// OUT: Count of columns
		&ulOffset,			// OUT: Length of the DBBINDINGs
  		DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY,
		FORWARD, 
		NO_COLS_BY_REF,
		NULL, 
		NULL,
		NULL, 
		DBTYPE_EMPTY, 
		cColsToBind, 
		rgColsToBind,
		NULL,					//rgColOrdering
		NO_COLS_OWNED_BY_PROV,	//eColsMemProvOwned
		DBPARAMIO_NOTPARAM,		//eParamIO
		BLOB_LONG				//dwBlobType
	), S_OK))
		goto CLEANUP;

	// Allocate memory for a single row
	SAFE_ALLOC(pData, BYTE, ulOffset);
	memset(pData, 0xCC, (size_t)ulOffset);

	// Make sure the CLIST is empty
	m_ColList.RemoveAll();

	// Get Provider-specific data type names user wants in the table
	if (NATIVETYPES == eDataTypes)
	{
		// Get count of types user wants in the table
		icList = NativeTypesList.GetCount();

		// If there are no elements then the user made a mistake
		if (0 == icList)
		{
			hr = E_FAIL;
			goto CLEANUP;
		}

		// Get the first element(type) out of the users list
		pos = NativeTypesList.GetHeadPosition();
	}

	// Get Provider data types from the user
	if (DBDATATYPES == eDataTypes)
	{
		// Get count of types user wants in the table
		icList = ProviderTypesList.GetCount();

		// If there are no elements then the user made a mistake
		if (0 == icList)
		{
			hr = E_FAIL;
			goto CLEANUP;
		}

		rgList = (DBTYPE *) PROVIDER_ALLOC(sizeof(DBTYPE) * icList);

		// Get the first element(type) out of the users list
		pos = ProviderTypesList.GetHeadPosition();
	}

	// Create the CCol list of data types the user wants in the table
	if ((NATIVETYPES == eDataTypes) || (DBDATATYPES == eDataTypes))
	{
		// Increment through every element in the users list
		while(pos)
		{
			CCol NewCol;

			// Get next element in the users list
			if (NATIVETYPES == eDataTypes)
			{
				LONG			lTemp=0;			// Count the # of matches by scanf
				WCHAR			*wszTypeName=NULL;

				// Set Provider-specific data type names
				strElem = NativeTypesList.GetNext(pos);

				// Check for fully qualified type name like number(38,4)
				if (wcsstr(strElem, L"("))
				{
					WCHAR* pwszQualType = wcsDuplicate(strElem);

					wszTypeName = (WCHAR *)PROVIDER_ALLOC((wcslen(strElem)+1)*sizeof(WCHAR));
					if (!pwszQualType || !wszTypeName)
					{
						hr = E_OUTOFMEMORY;
						goto CLEANUP;
					}
					
					// Check for zero or one or two parameters
					if (1  == (wcsstr(pwszQualType, L")") - wcsstr(pwszQualType, L"(")))
					{
						lTemp = swscanf(pwszQualType, L"%[^(]", wszTypeName);					
						ASSERT(lTemp==1);  // Improper string given to CreateColInfo
						NewCol.SetReqParams(0);
					}
					else if (wcsstr(pwszQualType, L","))
					{
						ULONG	ulPrecision = ULONG_MAX;
						ULONG	ulScale = ULONG_MAX;

						lTemp = swscanf(pwszQualType, L"%[^(](%u,%u)", wszTypeName, &ulPrecision, &ulScale);
						ASSERT(lTemp==3);  // Improper string given to CreateColInfo
						NewCol.SetReqParams(2);
						NewCol.SetPrecision((BYTE)ulPrecision);
						NewCol.SetScale((BYTE)ulScale);
					}
					else
					{
						DBLENGTH	ulColumnSize = 0;

						lTemp = swscanf(pwszQualType, L"%[^(](%u)", wszTypeName, &ulColumnSize);
						ASSERT(lTemp==2);  // Improper string given to CreateColInfo						
						NewCol.SetReqParams(1);
						NewCol.SetColumnSize(ulColumnSize);
					}

					NewCol.SetProviderTypeName(wszTypeName);					
					PROVIDER_FREE(pwszQualType);
					PROVIDER_FREE(wszTypeName);
				}
				else
				{
					NewCol.SetProviderTypeName((WCHAR *)strElem);
					NewCol.SetReqParams();	// set to default
				}
			}
			else
			{
				// Set Provider data type
				dbProvElem = ProviderTypesList.GetNext(pos);
				NewCol.SetProviderType((DBTYPE)(dbProvElem));
			}

			// Add the type to the CCol list
			m_ColList.AddTail(NewCol);
		}
	}

	// Initialize
	icFoundList = 0;
	rgHrow		= NULL;

	// Get sequential rows from Schema TYPES Rowset
	while(!FAILED(hr = pIRowset->GetNextRows(
			NULL,			// The chapter handle
			0,				// Count of rows to skip
			20,				// Number of rows to Fetch
			&ulcRowObt,		// Number of rows obtained
			&rgHrow))		// Handles of retrieved rows
	 && ulcRowObt != 0)
	{
		// Check the HResult an number of rows
		if (hr == S_OK)
			COMPARE(ulcRowObt, 20);
		else if (hr == DB_S_ENDOFROWSET)
			COMPARE(1, (ulcRowObt < 20));
		else
			goto CLEANUP;

		// Loop for each row obtained
		for(uliRow=0; uliRow < ulcRowObt; uliRow++)
		{
			// Get the data for the row
			if (CHECK(hr = pIRowset->GetData(rgHrow[uliRow], hAccessRead, pData),S_OK))
			{

				// See if this data type is an allowed type
				if (pwszInitValue)
				{
					BOOL fAllowedType = FALSE;
					WCHAR * pwszDataTypeName = NULL;
					WCHAR * pwszInitDupe = wcsDuplicate(pwszInitValue);
					CHECK_MEMORY(pwszInitDupe);
					pwszDataTypeName = SkipWhiteSpace(wcstok(pwszInitDupe, L","));
					while(pwszDataTypeName && !fAllowedType)
					{
						if (!wcscmp(pwszDataTypeName, (LPWSTR)&VALUE_BINDING(rgBindings[0], pData)))
							fAllowedType = TRUE;
						pwszDataTypeName = SkipWhiteSpace(wcstok(NULL, L","));
					}
					PROVIDER_FREE(pwszInitDupe);
					if (!fAllowedType)
						continue;
				}
				
				// If not all types, need to see if we want this,
				// If we do want this, we need to see if we have already handled it
				if ((NATIVETYPES == eDataTypes) || (DBDATATYPES == eDataTypes))
				{
					// If datatype was already found, go to next row
					if (DBDATATYPES == eDataTypes)
					{
						for(int i=0;i<cList;i++)
						{
							if (rgList[i] == (DBTYPE)VALUE_BINDING(rgBindings[1], pData))
								goto NEXTROW;
						}
					}
					
					// Loop until end of CCol list which now contains the types
					// the user passed in that the user wants in the table
					pos = m_ColList.GetHeadPosition();
					while(pos)
					{
						//True Reference 
						CCol& rCol = m_ColList.GetNext(pos);

						if ((DBDATATYPES == eDataTypes) && (rCol.GetProviderType() == (DBTYPE)VALUE_BINDING(rgBindings[1], pData))  ||
						(NATIVETYPES == eDataTypes && IsCompatibleType(cBindings, rgBindings, pData, rCol)))
						{
							// Put type in list so we don't have to go over it again
							if(DBDATATYPES==eDataTypes)
								rgList[cList++]= rCol.GetProviderType();
							
							// Call the function UpdateCCol to set all the values
							// from TYPES Rowset into the CCol list element for this column
							hr = UpdateCCol(cBindings, rgBindings, pData, rCol, eDataTypes, &ulAutoIncPrec, m_lSQLSupport);

							if (SUCCEEDED(hr))
							{
								// If we received the information from TYPES Rowset
								// for each data type the user passed in then there is
								// no need to go through the rest of the data types
								// from TYPES Rowset.
								if (++icFoundList == icList)
								{
									// Release the row handles
									if (!CHECK(hr = pIRowset->ReleaseRows(ulcRowObt, rgHrow, NULL, NULL, NULL),S_OK))
										goto CLEANUP;

									goto DONE;
								}
							}
							else // update failed
							{
								// If type matches but fails, no need to try it again
								if(DBDATATYPES == eDataTypes)
								{
									if (STATUS_BINDING(rgBindings[1], pData) == DBSTATUS_S_OK)
										PRVTRACE(L"%sCreateColInfo):DBTYPE %d failed in UpdateCCol()\n",wszPRIVLIBT, VALUE_BINDING(rgBindings[1], pData));

									goto NEXTROW;
								}
								else//native types
								{
									if (STATUS_BINDING(rgBindings[1], pData)==DBSTATUS_S_OK)
										PRVTRACE(L"%sCreateColInfo):Native Type %s failed in UpdateCCol()\n",wszPRIVLIBT,(WCHAR*)&VALUE_BINDING(rgBindings[0], pData));
									
									goto NEXTROW;
								}
							}
						}
					} // end of while for native and provider types
				}
				else // ALLTYPES
				{
					// Call the function UpdateCCol to set all the values
					// from TYPES Rowset into the CCol list element for
					// this column
					CCol NewCol(m_pSchemaCache);
					if (SUCCEEDED(UpdateCCol(cBindings, rgBindings, pData, NewCol, eDataTypes, &ulAutoIncPrec, m_lSQLSupport)))
						m_ColList.AddTail(NewCol);
				}

				NEXTROW:
				;
			}
		}

		// Release the row handles
		if (!CHECK(hr = pIRowset->ReleaseRows(ulcRowObt, rgHrow, NULL, NULL, NULL),S_OK))
			goto CLEANUP;
	}
DONE:
	PROVIDER_FREE(rgList);

	if (FAILED(hr))
		goto CLEANUP;

	// We want to fail if the user passed in a type that the server did not support
	if (((NATIVETYPES == eDataTypes) || (DBDATATYPES == eDataTypes)) 
			&& icFoundList != icList)
	{
		hr = E_FAIL;
		PRVTRACE(L"%sCreateColInfo):not all requested columns succeeded!!!\n",wszPRIVLIBT);
		goto CLEANUP;
	}


CLEANUP:
	//Mask hr from GetNextRows(20).
	if(hr==DB_S_ENDOFROWSET)
		hr = S_OK;

	// assigne ordinals
	if (pulAutoIncPrec)
		*pulAutoIncPrec = ulAutoIncPrec;

	udwColNum = 1;
	pos = m_ColList.GetHeadPosition();
	while(pos)
	{
		//True Reference
		CCol& rCol = m_ColList.GetNext(pos);
		
		// Format column name so it looks like "c02tiny"
		// Use different format if the International Data flag is set
		// Internation format is "<CHAR>02tiny" where <CHAR> is a locale-specific character
		if(GetModInfo()->GetLocaleInfo())
		{
			//Convert column name's to upper case...
			//TODO:  Temporary hack until we change the privlib completely to
			//correctly quote identifiers.  This gets arround the problem of 
			//generating lower case intl chars that when "unquoted" produce a upper
			//case backend column name, then trying to do quoted variations can't find
			//the column name on case sensitive providers (ie: Oracle)...
			//This is the same hack made for table names
			pwszUnicodeInt = GetModInfo()->GetLocaleInfo()->MakeUnicodeInt(udwColNum);

			swprintf(strColName,MAXDATALEN, L"%c%.2s%.4s", MapWCHAR(GetModInfo()->GetLocaleInfo()->MakeUnicodeChar(), LCMAP_UPPERCASE),
				pwszUnicodeInt,rCol.GetProviderTypeName());
		}
		else
		{						
			swprintf(strColName, MAXDATALEN, L"C%02lu%.4s", udwColNum, rCol.GetProviderTypeName());
			if( m_ulIdentifierCase == DBPROPVAL_IC_UPPER )
			_wcsupr(strColName);
			else if( m_ulIdentifierCase == DBPROPVAL_IC_LOWER )
				_wcslwr(strColName);
		}

		// If name has a blank at the end replace with an underscore
		if (pdest=wcsstr(strColName, L" "))
			*pdest = L'_';

		// Set the column name
		rCol.SetColName(strColName);
		rCol.SetColNum(udwColNum++);
		rCol.SetUpdateable(TRUE);
	}
	SAFE_RELEASE(pSchemaRowset)

	if(pIRowset && hAccessRead)
	{
		CHECK(pIAccessor->ReleaseAccessor(hAccessRead, &cRefCounts), S_OK);
		COMPARE(cRefCounts,0);
	}

	// Free the memory
	PROVIDER_FREE(rgHrow);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgColsToBind);
	PROVIDER_FREE(pwszUnicodeInt);
	PROVIDER_FREE(pwszInitValue);

	// Release the pointers
	SAFE_RELEASE(pIColumnsInfo);	
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);

	// Free the Accessor bindings
	FreeAccessorBindings(cBindings, rgBindings);
	return hr;	
} //CTable::CreateTypeColInfo





//---------------------------------------------------------------------------
// CTable::CreateColInfo 
//
// CTable			|
// CreateColInfo	|
// Create column information, initializes m_ColList with columns of 
// specified types. Because temp table now puts bookmarks in the schema table,
// we have to get the bookmark column out the the DBCOLUMNINFO, before we call
// to get the bindings.
//
// @mfunc CreateColInfo
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateColInfo(
	CList<WCHAR *,WCHAR *>& NativeTypesList,	//  @parm [OUT] List of native data types 
	CList<DBTYPE,DBTYPE>& ProviderTypesList,	//  @parm [OUT] List of provider data types
	EDATATYPES eDataTypes,						//  @parm [IN] Enum for column data type (Default = ALLTYPES)
	BOOL fFirstUpdateable						//	@parm [IN] TRUE means first column will not be autoinc (Default=FALSE)
	)
{
	HRESULT			hr = S_OK;			// Returned result
	WCHAR *			strElem=NULL;		// Element of the list of Native data types
	POSITION		pos;				// Position in data types list
	POSITION		posSave;			// Position in data types list
	UDWORD			udwColNum;			// Column number
	WCHAR 			strColName[MAXDATALEN];		// Column name
	WCHAR *			pdest;				// Pointer in the Dest String
	ULONG			ulAutoIncPrec = 0;	// Precision of largest auto increment column
	BOOL			fAutoIncDone;		// Has the auto increment column been found
	WCHAR *			pwszUnicodeInt = NULL; // Locale-dependent integer string
	
	//CreateTypeColInfo
	if(FAILED(hr = CreateTypeColInfo(NativeTypesList, ProviderTypesList, eDataTypes, &ulAutoIncPrec)))
		goto CLEANUP;

	// updatability of columns is unknown => set them to FALSE
	pos = m_ColList.GetHeadPosition();
	while(pos)
	{
		CCol& rCol = m_ColList.GetNext(pos);
		rCol.SetUpdateable(FALSE);
	}

	// We are only moving columns for the first column to be numeric, index when
	// the client has chosen that we build the column list, instead of the client
	// pass in a list of datatypes. If the client passes in a list of datatypes 
	// (native or provider), we assume they know what they are doing, and therefore
	// we just make their first column unique and indexed.
	if (ALLTYPES==eDataTypes)
	{
		// HACK: Delete the LONG RAW for ORACLE,
		// and sql_variant for Kagera.
		CLSID clsidKag = DB_NULLGUID;
		CLSID clsidConfProv = DB_NULLGUID;
		CLSIDFromProgID(L"MSDASQL", &clsidKag);
		CLSIDFromProgID(L"ConfProv", &clsidConfProv);

		pos = m_ColList.GetHeadPosition();
		while(pos)
		{
			posSave = pos;
			CCol& rCol = m_ColList.GetNext(pos);

			// Delete the log raw column from the list
			if (!(_wcsnicmp(L"long raw",rCol.GetProviderTypeName(),wcslen(L"long raw"))))
			{
				m_ColList.RemoveAt(posSave);
				break;
			}

			// Delete the sql_variant column from the list if provider is Kagera.
			if (!(_wcsnicmp(L"sql_variant",rCol.GetProviderTypeName(),wcslen(L"sql_variant"))) && 
				(GetModInfo()->GetProviderCLSID()==clsidKag || GetModInfo()->GetProviderCLSID()==clsidConfProv))
			{
				m_ColList.RemoveAt(posSave);
				break;
			}
		}

		//  Make the first column numeric for the index
		if (fFirstUpdateable==FALSE)
		{
			// Get first element in the CCol list
			pos = m_ColList.GetHeadPosition();
			CCol& rCol = m_ColList.GetNext(pos);

			// If first column is already numeric then were done unless there are
			// autoinc datatypes. If that is the case we want the largest autoinc
			// column as the first column.
			if ((!IsNumericType(rCol.GetProviderType())) || (rCol.GetPrecision() <= 4)
				|| ((ulAutoIncPrec > 0) && (ulAutoIncPrec != rCol.GetPrecision())
				&& (!rCol.GetAutoInc())))
			{
				// If first column needs to be numeric
				while(pos)
				{
					// Save the position for deleting and then get the next element
					posSave = pos;
					CCol& rCol = m_ColList.GetNext(pos);

					// If the there is an autoinc column in the table we want to
					// make the largest autoinc column the first column in the
					// table. If there is not an autoinc column we will take the
					// first numeric column with a precision greater than 4.
					if ((IsNumericType(rCol.GetProviderType())) &&
						(rCol.GetPrecision() > 4))
					{
						if ((ulAutoIncPrec == 0) || ((rCol.GetAutoInc()) &&
							(ulAutoIncPrec == rCol.GetPrecision())))
						{
							// Put the numeric column as the first element in the list
							m_ColList.AddHead(rCol);
							m_ColList.RemoveAt(posSave);
							break;
						}
					}
				}
			}
		}
		else // first column will be unique and updateable but not autoinc
		{
			// Get first element in the CCol list
			pos    = m_ColList.GetHeadPosition();
			CCol& rCol = m_ColList.GetNext(pos);

			// If first column is already numeric then were done unless there are
			// autoinc datatypes. If that is the case we want the largest autoinc
			// column as the first column.
			if ((!IsNumericType(rCol.GetProviderType())) || (rCol.GetPrecision() <= 4))
			{
				// If first column needs to be numeric
				while(pos)
				{
					// Save the position for deleting and then get the next element
					posSave = pos;
					CCol& rCol = m_ColList.GetNext(pos);

					// We will take the	first numeric column with a precision greater than 4
					// that is not autoinc
					if ((IsNumericType(rCol.GetProviderType())) &&
						(rCol.GetPrecision() > 4) &&
						(FALSE==rCol.GetAutoInc()))
					{
						rCol.SetNullable(0);
						
						// Put the numeric column as the first element in the list
						m_ColList.AddHead(rCol);
						
						// Delete column from where is was currently in the list
						m_ColList.RemoveAt(posSave);
						break;
					}
				}
			}
		}
	}

	// Initialize for filling in column number and name
	udwColNum = 1;
	posSave = pos;

	// Set to TRUE once the auto inc column with the correct precision is found
	fAutoIncDone = FALSE;

	// Fill in column number and name
	pos = m_ColList.GetHeadPosition();
	while(pos)
	{
		posSave = pos;
		CCol& rCol = m_ColList.GetNext(pos);

		// Remove all but the largest auto incrementing column
		if ((ALLTYPES == eDataTypes) && (rCol.GetAutoInc()))
		{
			//Remove the column if it is not the largest auto inc column
			//Or remove all autoinc's after the first one...
			if(ulAutoIncPrec != rCol.GetPrecision() || fAutoIncDone)
			{
				if ((!SupportedProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, m_pIDBInitialize,DATASOURCE_INTERFACE) &&
					!SettableProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, m_pIDBInitialize,DATASOURCE_INTERFACE)) ||
					!GetModInfo()->IsUsingITableDefinition())
				{
					// Remove the element from the CCol list
					m_ColList.RemoveAt(posSave);
					continue;
				}
				else
				{
					// COL_AUTOINCREMENT is a settable property; this implies than it is an optional column property
					// instead of removing this type, just make in non-autoincrementing
					rCol.SetAutoInc(0);
					rCol.SetUnique(FALSE);
				}
			}
			else
			{
				if (SettableProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, m_pIDBInitialize,DATASOURCE_INTERFACE) &&
					GetModInfo()->IsUsingITableDefinition())
				{
					// Add a non-autoinc flavor to the end
					// We'll come back to this type later
					m_ColList.AddTail(rCol);

					// Make sure we didn't add this one after the previous last one
					if (!pos)
						pos = m_ColList.GetTailPosition();
				}
					
				// We have our auto incrementing column we want to keep
				rCol.SetScale(0);		// Auto-Inc columns must have 0 scale
				rCol.SetNullable(0);	// Auto-Inc columns can never be NULL
				SetHasAuto(TRUE);		// Set flag to indicate that the table has an auto-inc col
				fAutoIncDone = TRUE;
			}
		}
		
		// Format column name so it looks like "c02tiny"
		// Use different format if the International Data flag is set
		// Internation format is "<CHAR>02tiny" where <CHAR> is a locale-specific character
		if (GetModInfo()->GetLocaleInfo())
		{
			//Convert column name's to upper case...
			//TODO:  Temporary hack until we change the privlib completely to
			//correctly quote identifiers.  This gets arround the problem of 
			//generating lower case intl chars that when "unquoted" produce a upper
			//case backend column name, then trying to do quoted variations can't find
			//the column name on case sensitive providers (ie: Oracle)...
			//This is the same hack made for table names
			pwszUnicodeInt = GetModInfo()->GetLocaleInfo()->MakeUnicodeInt(udwColNum);

			swprintf(strColName, MAXDATALEN, L"%c%.2s%.4s", MapWCHAR(GetModInfo()->GetLocaleInfo()->MakeUnicodeChar(), LCMAP_UPPERCASE),
				pwszUnicodeInt,rCol.GetProviderTypeName());
		}
		else
		{
			swprintf(strColName,  MAXDATALEN, L"C%02lu%.4s", udwColNum, rCol.GetProviderTypeName());
			if( m_ulIdentifierCase == DBPROPVAL_IC_UPPER )
			_wcsupr(strColName);
			else if( m_ulIdentifierCase == DBPROPVAL_IC_LOWER )
				_wcslwr(strColName);
		}

		// If name has a blank at the end replace with an underscore
		if (pdest=wcsstr(strColName, L" "))
			*pdest = L'_';

		// Set the column name
		rCol.SetColName(strColName);

		// Set the column number
		rCol.SetColNum(udwColNum++);

		if ((NATIVETYPES == eDataTypes) || (DBDATATYPES == eDataTypes)) 
		{
			if (SettableProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, m_pIDBInitialize,DATASOURCE_INTERFACE) &&
				GetModInfo()->IsUsingITableDefinition())
			{
				// If a list of nativetypes or dbtypes is given,
				// always use the non-autoinc flavor of the requested base type
				rCol.SetAutoInc(0);
				rCol.SetUnique(FALSE);
			}
		}
	}

CLEANUP:

	PROVIDER_FREE(pwszUnicodeInt);

	return hr;	
}


//---------------------------------------------------------------------------
// CTable::BuildColumnDesc
//
// CTable				|
// BuildColumnDesc		|
// Builds a DBCOLUMNDESC struc used by ITableDefinition::CreateTable, <nl>
// and ITableDefinition::AddColumn based on column info. <nl>
//
//---------------------------------------------------------------------------
HRESULT CTable::BuildColumnDesc(DBCOLUMNDESC *pColumnDesc, CCol& CurCol)
{	
	if (NULL == pColumnDesc)
		return E_FAIL;

	//initialize the structure
	memset(pColumnDesc, 0, sizeof(DBCOLUMNDESC));

	// Point our structure's pointers to the data members
	// containing column info.  Note, we don't own any
	// of this  memory, and are only accessing it.  Thus
	// this array is only valid while m_ColList is valid.
	pColumnDesc->pwszTypeName	= wcsDuplicate(CurCol.GetProviderTypeName());
	pColumnDesc->pTypeInfo		= NULL;
	if (GetHasCLSID() && GUID_NULL != CurCol.GetTypeGuid())
	{
		pColumnDesc->pclsid			= (GUID*)PROVIDER_ALLOC(sizeof(GUID));
		if (NULL != pColumnDesc->pclsid)
			memcpy(pColumnDesc->pclsid, &CurCol.GetTypeGuid(), sizeof(GUID));
	}
	else
		pColumnDesc->pclsid		= NULL;
	
	pColumnDesc->wType = CurCol.GetProviderType();
	
	// Copy the value of precision and scale into our structure
	pColumnDesc->bPrecision				= CurCol.GetPrecision();
	pColumnDesc->bScale					= CurCol.GetScale();
	pColumnDesc->ulColumnSize			= CurCol.GetMaxColumnSize();
	pColumnDesc->dbcid.eKind			= DBKIND_NAME;
	pColumnDesc->dbcid.uName.pwszName	= wcsDuplicate(CurCol.GetColName());
	
	// set properties 
	if ( CurCol.GetAutoInc() )
	{
		// if the column type is DBTYPE_NUMERIC or DBTYPE_DECIMAL, scale should be 0
		// if the column type is some other type and info was obtained from
		// schema rowset then scale should be 0, otherwise ~0 (for info got from 
		// IColumnsInfo::GetColumnsInfo()
		ASSERT(pColumnDesc->bScale == 0 || pColumnDesc->bScale == (BYTE)~0);

		SetProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, 
				&pColumnDesc->cPropertySets, &pColumnDesc->rgPropertySets,
				DBTYPE_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
	}
	else 
	{
		pColumnDesc->cPropertySets	= 0;
		pColumnDesc->rgPropertySets	= NULL;
	}

	// set default if it was asked
	if (CurCol.GetHasDefault())
	{
		VARIANT	vDefault = CurCol.GetDefaultValue();
		// set default property
		SetProperty(DBPROP_COL_DEFAULT, DBPROPSET_COLUMN, 
			&pColumnDesc->cPropertySets, &pColumnDesc->rgPropertySets,
			vDefault.vt, V_BSTR(&vDefault), DBPROPOPTIONS_REQUIRED, DB_NULLID);
	}

	// set unique if it was asked
	if (CurCol.GetUnique())
	{
		//check if DBPROP_COL_UNIQUE is supported
		//if not make the col uniqie with create index later
		//can't do it with index until the table is created
		if (SupportedProperty(DBPROP_COL_UNIQUE,DBPROPSET_COLUMN,this->m_pIDBInitialize))
		{
			SetProperty(DBPROP_COL_UNIQUE, DBPROPSET_COLUMN, 
				&pColumnDesc->cPropertySets, &pColumnDesc->rgPropertySets,
				VT_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		}
	}
	// set unique if it was asked
//	if( (m_eNull == NONULLS && IsNonNullColumnsAllowed()) || 
	if( NONULLS == m_eNull || (!CurCol.GetNullable() && IsNonNullColumnsAllowed()) )
	{
		SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
			&pColumnDesc->cPropertySets, &pColumnDesc->rgPropertySets,
			VT_BOOL, (ULONG_PTR)VARIANT_FALSE, DBPROPOPTIONS_REQUIRED, DB_NULLID);
		CurCol.SetNullable(0);
	}
	else
		SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
			&pColumnDesc->cPropertySets, &pColumnDesc->rgPropertySets,
			VT_BOOL, (ULONG_PTR)VARIANT_TRUE, DBPROPOPTIONS_REQUIRED, DB_NULLID);

	return S_OK;
} //CTable::BuildColumnDesc


//---------------------------------------------------------------------------
// CTable::BuildColumnDescs 
//
// CTable				|
// BuildColumnDescs		|
// Builds an array used by ITableDefinition::CreateTable, based on 
//
//---------------------------------------------------------------------------
HRESULT CTable::BuildColumnDescs(DBCOLUMNDESC** prgColumnDesc)
{
	POSITION pos = NULL;
	DBORDINAL	ColCount = 0;
	DBORDINAL	i = 0;
	DBORDINAL	nCol;

	// If we're done this once, don't bother to again, the info hasn't changed
	ASSERT(prgColumnDesc);
	*prgColumnDesc = NULL;

	if (m_ColList.IsEmpty())
		return E_FAIL;

	ColCount = CountColumnsOnTable();

	// This memory is freed in the destructor
	DBCOLUMNDESC* rgColumnDesc = (DBCOLUMNDESC *)PROVIDER_ALLOC(sizeof(DBCOLUMNDESC) * ColCount);
	memset(rgColumnDesc, 0, (size_t)(sizeof(DBCOLUMNDESC) * ColCount));
	if (!rgColumnDesc)
		return E_OUTOFMEMORY;

	pos = m_ColList.GetHeadPosition();
	for (i=0;  pos;  i++)
	{
		CCol& rCol = m_ColList.GetNext(pos);

		nCol = rCol.GetColNum(); // get ordinal position of the table
		if (0 == nCol) 
			continue;
		nCol--;	// get the 0 based position
	
		BuildColumnDesc(&rgColumnDesc[nCol], rCol);
	}
		
	*prgColumnDesc = rgColumnDesc;			
	return S_OK;
}


//---------------------------------------------------------------------------
// CTable::CreateIndex	 
//
// CTable				|
// CreateIndex			|
// Creates index on table, looks like: 
// "create [unique] index index_name on table_name (column_name)" , uses
// column ordinal number (m_ColNum). Index name is same as table name. Index
// is unique if EINDEXTYPE is UNIQUE, else not unique.
//
// @mfunc	CreateIndex
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem	    
// 	@flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateIndex(
	DBORDINAL	iOrdinal,		// @parm [IN] Ordinal number to put index on (Default = 1)
	EINDEXTYPE 	eIndexType,		// @parm [IN] Type of index (Default = UNIQUE)
	LPWSTR		pwszIndexName	// @parm [IN] Index name to use, default NULL will use table name.
)
{
	return CreateIndex(&iOrdinal, 1, eIndexType, pwszIndexName);
}


//---------------------------------------------------------------------------
// CTable::CreateIndex	 
//
// CTable				|
// CreateIndex			|
// Creates composite index on table, looks like: 
// "create [unique] index index_name on table_name (<COL_LIST>)" 
// Index name is same as table name. 
// Index is unique if EINDEXTYPE is UNIQUE, else not unique.
//
// @mfunc	CreateIndex
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem	    
// 	@flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateIndex(
	DBORDINAL*	rgulOrdinals,	// [IN] Array of ordinals
	DBORDINAL	cOrdinals,		// [IN] Count of ordinals
	EINDEXTYPE 	eIndexType,		// [IN] Type of index (Default = UNIQUE)
	LPWSTR		pwszIndexName	// [IN] Index name to use, default NULL will use table name.
)
{
	HRESULT		hrCreateIndex=S_OK;
	WCHAR * 	pwszSQLText=NULL;		// sql statement to execute
	WCHAR * 	pwszColList=NULL;		// Column List used for index
	size_t		cchColList=0;			// Keep track of ColList length
	DBORDINAL   iCol=0;

	IIndexDefinition*	pIIndexDefinition = NULL;
	DBINDEXCOLUMNDESC*	rgIndexColDesc = NULL;
	DBID				IndexID = DB_NULLID;
	BOOL				fValidColumnNames = TRUE;

	ASSERT(GetTableName());
	ASSERT(cOrdinals > 0);
	
	//Setup index name
	if (pwszIndexName == NULL)
		pwszIndexName = GetTableName();

	IndexID.eKind = DBKIND_NAME;
	IndexID.uName.pwszName = pwszIndexName;
	SAFE_ALLOC(rgIndexColDesc, DBINDEXCOLUMNDESC, cOrdinals);

	//Find Columns we're looking for
	for(iCol=0; iCol<cOrdinals; iCol++)
	{
		// Can't put index on bookmark column
		ASSERT(rgulOrdinals[iCol]!=0);
		CCol& col = GetColInfoForUpdate(rgulOrdinals[iCol]);

		//Note: We don't have to duplicate the DBIDs since were not saving them, were just
		//using them locally in this function, so a copy and no release is fine and much more effiencent
		rgIndexColDesc[iCol].pColumnID		 = col.GetColID();
		rgIndexColDesc[iCol].eIndexColOrder = DBINDEX_COL_ORDER_ASC;

		//Note: We can only use a command: if all the column names are names...
		if(!col.GetColName())
			fValidColumnNames = FALSE;
		cchColList += wcslen(col.GetColName());
	}

	//If commands are not supported or No SQL Support, use IIndexDefintion
	if(!GetCommandSupOnCTable() || !GetSQLSupport() || !fValidColumnNames)
	{
		// IIndexDefinition must be present
		if(VerifyInterface(m_pIOpenRowset, IID_IIndexDefinition, SESSION_INTERFACE, (IUnknown**)&pIIndexDefinition))
		{
			//IIndexDefinition::CreateIndex
			hrCreateIndex = pIIndexDefinition->CreateIndex(&m_TableID, &IndexID, 
				cOrdinals, rgIndexColDesc, 0, NULL, NULL);

			if (SUCCEEDED(hrCreateIndex))
				PRVTRACE(L"CreateIndex created index on table %s column %d using IIndexDefinition.\n",
				(m_TableID.eKind == DBKIND_NAME) ? m_TableID.uName.pwszName : L"PROPID or GUID", rgulOrdinals[0]);
			else
				PRVTRACE(L"CreateIndex using IIndexDefinition FAILED!\n");
		}
	}
	//Otherwise use commands
	else
	{
		// Alloc mem for the column names,commas, and null terminator
		SAFE_ALLOC(pwszColList, WCHAR, cchColList+cOrdinals);
		*pwszColList = L'\0';
		for(iCol=0; iCol < cOrdinals; iCol++)
		{
			CCol& col = GetColInfoForUpdate(rgulOrdinals[iCol]);
			
			if(iCol!=0)
				wcscat(pwszColList, wszCOMMA);
			
			//This should have already been checked above.
			ASSERT(col.GetColName());
			wcscat(pwszColList, col.GetColName());
		}

		//String layout "Create %s index %s on %s (%s)";
		pwszSQLText = (WCHAR *) PROVIDER_ALLOC(
			( sizeof(WCHAR) * 
				(wcslen(wszCREATE_INDEX) +
				wcslen(wszUNIQUE) +
				wcslen(pwszIndexName) +
				wcslen(wszON) +
				wcslen(GetTableName()) +
				wcslen(pwszColList) +
				+ sizeof(WCHAR))));
		TESTC(pwszSQLText != NULL);

		// Format SQL Statement
		// Statement like: "create [unique] index index_name on table_name (column_name)"    
		swprintf(pwszSQLText, wszCREATE_INDEX, wszUNIQUE, pwszIndexName, GetTableName(), pwszColList);

		// Execute statement
		hrCreateIndex =	BuildCommand(pwszSQLText,	//SQL text to execute
							IID_NULL,		//This won't be used
							EXECUTE_ALWAYS,	//Should always execute fine
							0, NULL,		//Set no properties
							NULL,			//No parmeters									
							NULL, 			//No RowsAffected info
							NULL,			//No rowset needs to be allocated
							&m_pICommand);	//Use this object's command to execute on

		if (SUCCEEDED(hrCreateIndex))
			PRVTRACE(L"%sCreateIndex): %s\n", wszPRIVLIBT, pwszSQLText);
		else
			PRVTRACE(L"%sCreateIndex FAILED ): %s\n", wszPRIVLIBT, pwszSQLText);
	}

	// If this table doesn't already have an index save this index
	// information.  We only save information for the first index
	// created.
	if (S_OK == hrCreateIndex && m_pwszIndexName == NULL)
	{
		m_pwszIndexName = wcsDuplicate(pwszIndexName); 
		m_ulIndex = rgulOrdinals[0];
	}

CLEANUP:
	SAFE_RELEASE(pIIndexDefinition);
	PROVIDER_FREE(rgIndexColDesc);
	PROVIDER_FREE(pwszColList);
	PROVIDER_FREE(pwszSQLText);
	return hrCreateIndex;
}


//---------------------------------------------------------------------------
// CTable::DoesIndexExist  	
//
// CTable			|
// DoesIndexExist	|
// If this index is on this table return true and fill in
// strIndexName and udwIndex, set fExists to true. If function runs correctly
// but doesn't find the table name, function will return S_OK, but fExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	Assumes table
// will have only 1 index.
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::DoesIndexExist(			
	BOOL *		pfExists				// @parm [OUT] TRUE if index exists (Default = NULL)
)
{
	HRESULT 			hr				= E_FAIL;
	BOOL				fReturn			= FALSE;
	BOOL				fFound			= FALSE;
	DBLENGTH			ulRowSize		= 0;		// size of row
	DBCOUNTITEM			cDBBINDING		= 0;		// count of bindings
	DBORDINAL			cDBCOLUMNINFO	= 0;		// count of columninfo
	DBCOUNTITEM			iRow			= 0;		// count of rows
	DBCOUNTITEM			cRowsObtained	= 0;		// number of rows returned, should be 1
	ULONG				cSchema			= 0;		// number of supported Schemas
	ULONG *				prgRestrictions	= 0;		// restrictions for each Schema
	GUID *				prgSchemas		= NULL;		// array of GUIDs
	HROW *				rghRows			= NULL;		// array of handles of rows
	IDBSchemaRowset *	pIDBSchemaRowset= NULL;		// interface pointer
	IRowset *			pTypesRowset	= NULL;		// returned rowset
	DBBINDING *			rgDBBINDING		= NULL;		// array of bindings
	DBCOLUMNINFO *		rgDBCOLUMNINFO	= NULL;		// array of columninfos
	WCHAR *				pStringsBuffer	= NULL;		// corresponding strings
	BYTE *				pRow			= NULL;		// pointter to data
	HACCESSOR 			hAccessor		= NULL;		// accessor
	DATA *				pColumn			= NULL;
	VARIANT				rgRestrictIndexes[5];
	
	// Check to see if IDBSchema is supported
	if(!VerifyInterface(m_pIOpenRowset, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset))
		return S_OK;

	// Check to see if the schema is supported
	CHECK(hr = pIDBSchemaRowset->GetSchemas(&cSchema, &prgSchemas, &prgRestrictions),S_OK);

	ULONG i;
	// Check to see if DBSCHEMA_INDEXES is supported
	for(i=0; i<cSchema; i++)
	{
		if(prgSchemas[i] == DBSCHEMA_INDEXES)
			break;
	}

	ULONG index;
	// Set restrictions
	for(index=0;index<5;index++)
		VariantInit(&rgRestrictIndexes[index]);

	rgRestrictIndexes[4].vt 	 = VT_BSTR;
	rgRestrictIndexes[4].bstrVal = SysAllocString(GetTableName());

	// if i is equal to cSchema it is not supported
	if(i == cSchema)
		goto CLEANUP;

	// Check to see if the Tablename is valid
	assert(GetTableName());

	if (!CHECK(hr = pIDBSchemaRowset->GetRowset(	
			NULL, 								// aggregation
			DBSCHEMA_INDEXES,					// REFGUID
			5,	 								// count of restrictions (1:types)
			rgRestrictIndexes,					// list of restrictions
			IID_IRowset,						// REFFID
			0,									// count of properties
			NULL,								// range of properties
			(IUnknown**)&pTypesRowset			// returned result set
		),S_OK))
			goto CLEANUP;

	if (!CHECK(hr = GetAccessorAndBindings(
		pTypesRowset,		
		DBACCESSOR_ROWDATA,
		&hAccessor,			
		&rgDBBINDING,		
		&cDBBINDING,
		&ulRowSize,	
		DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_BOUND,
		FORWARD,
		NO_COLS_BY_REF,
		&rgDBCOLUMNINFO,	// OUT: Array of DBCOLUMNINFOs
		&cDBCOLUMNINFO,		// OUT: Count of DBCOULMNINFOs
		&pStringsBuffer, 
		DBTYPE_EMPTY, 
		0, 						//cColsToBind
		NULL,					//rgColsToBind
		NULL,					//rgColOrdering
		NO_COLS_OWNED_BY_PROV,	//eColsMemProvOwned
		DBPARAMIO_NOTPARAM,		//eParamIO
		BLOB_LONG),S_OK))		//dwBlobType
		goto CLEANUP;

	pRow = new	BYTE[(size_t)ulRowSize];	//data

	while(!FAILED(hr=pTypesRowset->GetNextRows(0, 0, 10, &cRowsObtained, &rghRows)) && cRowsObtained !=0)
	{ 
		// Check the HResult an number of rows
		if (hr == S_OK)
			COMPARE(cRowsObtained, 10);
		else if (hr == DB_S_ENDOFROWSET)
			COMPARE(1, (cRowsObtained < 10));
		else
			goto CLEANUP;

		// Get data for each row
		for(iRow=0;iRow<cRowsObtained;iRow++)			 
		{
			// Get data for a row
			if (fFound)
				break;

			CHECK(hr=pTypesRowset->GetData(rghRows[iRow], hAccessor, pRow),S_OK);

			if (FAILED(hr))
				goto CLEANUP;

  			if (rgDBBINDING[0].iOrdinal == 0)
				pColumn = (DATA *)(pRow + rgDBBINDING[8].obStatus);
			else
				pColumn = (DATA *)(pRow + rgDBBINDING[7].obStatus);

			if (pColumn->sStatus == DBSTATUS_S_OK)
			{
				// Only interested in indexes that are unique (NOT clustered)
				if ((*(VARIANT_BOOL *)pColumn->bValue) != FALSE)
				{
					// Get index name
					if (rgDBCOLUMNINFO[0].iOrdinal ==0)
						pColumn = (DATA *)(pRow + rgDBBINDING[6].obStatus);
					else
  						pColumn = (DATA *)(pRow + rgDBBINDING[5].obStatus);

					if (pColumn->sStatus==DBSTATUS_S_OK)
					{
						PRVTRACE(L"Index Name = %s\n",m_pwszIndexName);

						*pfExists = TRUE;
						fFound = TRUE;
					}

					// Get index ordinal column
					if (rgDBCOLUMNINFO[0].iOrdinal ==0)
						pColumn = (DATA *)(pRow + rgDBBINDING[17].obStatus);
					else
  						pColumn = (DATA *)(pRow + rgDBBINDING[16].obStatus);

					if (pColumn->sStatus==DBSTATUS_S_OK)
					{
						m_ulIndex = *((unsigned int*)pColumn->bValue);
						PRVTRACE(L"Index Column = %u\n",m_ulIndex);
						fFound = TRUE;
					}
				}
			}
		}

		// need to release rows
		CHECK(hr=pTypesRowset->ReleaseRows
		(		 
			cRowsObtained,		// number of rows to release
			rghRows,	  		// array of row handles
			NULL,
			NULL,				// count of rows successfully released
			NULL				// there shouldn't be anymore references to these rows
		),S_OK);

		if(FAILED(hr))
			goto CLEANUP;
	}

	fReturn = TRUE;

CLEANUP:
	// Free the memory
	PROVIDER_FREE(prgRestrictions);
	PROVIDER_FREE(prgSchemas);

	SAFE_DELETE(pRow);

	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pTypesRowset);

	PROVIDER_FREE(rgDBBINDING);
	PROVIDER_FREE(rgDBCOLUMNINFO);
	PROVIDER_FREE(pStringsBuffer);
	PROVIDER_FREE(rghRows);

	for(index=0;index<5;index++)
		GCHECK(VariantClear(&(rgRestrictIndexes[index])),S_OK);
	
	if (fReturn == FALSE)
		return hr;

	return S_OK;
}


//---------------------------------------------------------------------------
// CTable::Select 	
//
// Builds a select statement on table, executes if requested.
// This function is an alternative to BuildComand and ExecuteCommand.
// It would be used when you know which singular row you want returned.
//
// @mfunc Select
// @rdesc HRESULT indicating success or failure
//  @flag S_OK		| Function ran without problem
//  @flag E_FAIL    
//
//---------------------------------------------------------------------------
HRESULT CTable::Select( 
		IUnknown*		pIUnkOuter,				// [IN]  Aggregate
		DBCOUNTITEM		ulRowNumber,			// [IN]  Row number to select
		REFIID 			riid,					// [IN]  Type of interface user wants back
		ULONG			cPropSets,				// [IN]	 Count of property sets.
		DBPROPSET*		rgPropSets,				// [IN]	 Array of DBPROPSET structures.
		DBROWCOUNT*		pcRowsAffected,			// [OUT] Pointer to memory in which to return the count
												//			of rows affected by a command that updates, deletes,
												//			or inserts rows.
		IUnknown **		ppIUnknown,				// [OUT] Interface pointer user wants back (Default=NULL)
		ICommand **		ppICommand				// [IN/OUT] If ppICommand == NULL, this function uses
													// its own command object and then frees it, caller does nothing.
													// <nl>
													// If *ppICommand is not null, this value is used as the
													// ICommand interface for all command operations in this function. 
													// Caller maintains responsiblity to release this interface.
													// <nl>
													// If *ppICommand is null, this function creates a new command object
													// and places the ICommand interface to this object in *ppICommand.
													// Caller assumes responsibility to release this interface.
)
{
	TBEGIN
	HRESULT hr = S_OK;
	WCHAR* pwszCommand = NULL;
	DBORDINAL cColumns = 0;

	//If using an INI File we cannot correctly generate the SQL Statements needed 
	//to select all rows, since their is currently only one statement in the INI File...
	if(GetModInfo()->GetFileName() || GetModInfo()->GetDefaultQuery())
		return DB_E_NOTSUPPORTED;

	//Obtain the SQL Statement for obtaining the requested row
	QTESTC_(hr = CreateSQLStmt(SELECT_ROW_WITH_LITERALS, 
								NULL,				//pwszTableName2
								&pwszCommand, 
								&cColumns,			
								NULL,				//prgColumns
								ulRowNumber
								),S_OK);

	//Now Execute the command
	QTESTC_(hr = BuildCommand(pwszCommand, 
								riid, 
								EXECUTE_IFNOERROR, 
								cPropSets, 
								rgPropSets, 
								NULL,				//pParams
								pcRowsAffected,
								ppIUnknown, 
								ppICommand,
								pIUnkOuter
								),S_OK);

CLEANUP:
	SAFE_FREE(pwszCommand);
	return hr;
}

//---------------------------------------------------------------------------
// CTable::Insert	
//
// CTable			|
// Insert			|
// Inserts 1 row into the table. If ulRowNumber == 0 then ulRowNumber
// == m_ulNextRow. Rownumbers start at 0 in the private library, regardless
// of how they are treated in the Data Source. 
// ppwszInsert should look like "Insert into table (col1,...) values (x, ...)".
//
// This function uses a command object and a SQL insert statement if supported, 
// else it uses IOpenRowset and IRowsetChange::InsertRow to insert a row into the table.
//
// @mfunc	Insert
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | No Columns Marked for use or Execution of statement failed
//
//---------------------------------------------------------------------------
HRESULT CTable::Insert(	
	DBCOUNTITEM	ulRowNumber,// @parm [IN]  Row to insert (Default = 0, meaning next row)
	EVALUE 		eValue,		// @parm [IN]  MakeData type to insert (Default = PRIMARY)
	BOOL 		fExecute,	// @parm [IN]  TRUE means execute statement (Default = TRUE)
	WCHAR **	ppwszInsert,// @parm [OUT] Memory to place SQL text, if commands are supported (Default = NULL)
	BOOL		fNULL,		// @parm [IN]  TRUE means insert NULL wherever possible (Default = FALSE)
	DBCOUNTITEM	cRowsToInsert
)
{
	POSITION 	posNum;				// position in ColNumList
	POSITION 	posm_ColList;				// position in m_ColList
	CList 		<DBORDINAL,DBORDINAL> ColNumList;	// list of column numbers marked
	HRESULT 	hr = S_OK;
	LONG_PTR	lIndex=0;
	WCHAR	 	pwszValuesClause[20000];	// values to insert into columns
	WCHAR * 	pwszSQLText=NULL;			// sql text to execute
	WCHAR * 	pwszValueData=NULL;			// value for where clause
	WCHAR	 	pwszColNames[20000];		// name of column
	WCHAR *		pwszSingleChar=NULL;
	IRowset*	pIRowset = NULL;			//IRowset ptr for ICommand::Execute
	DBCOUNTITEM	iRow = 0;
	WCHAR		pwszX[2];

	pwszX[0]=L'.';
	pwszX[1]=L'\0';
	// Check to see if the Provider is readonly
	if(m_fProviderReadOnly)
		return hr;

	//May want to use IRowsetChange::InsertRow...
	if(!GetCommandSupOnCTable() || !GetSQLSupport() || 
		GetModInfo()->GetInsert() == INSERT_ROWSETCHANGE)
	{
		if(fExecute)
			hr = Insert(eValue, ulRowNumber, cRowsToInsert);
		goto CLEANUP;
	}

	//May want to use ICommandWithParameters...
	if(GetModInfo()->GetInsert() == INSERT_WITHPARAMS)
	{
		hr = InsertWithParams(ulRowNumber, eValue, fExecute, &pwszSQLText, cRowsToInsert);
		goto CLEANUP;
	}

	// Get Marked Columns 
	if(FAILED(hr=GetColsUsed(ColNumList)))
		goto CLEANUP;

	if (ulRowNumber == 0)
		ulRowNumber = m_ulNextRow;

	
	//Loop through all the rows to insert
	for(iRow = 0; iRow <cRowsToInsert; iRow++)
	{
		pwszValuesClause[0] = L'\0';
		pwszColNames[0] = L'\0';

		// Get column names and values for column - there is a where clause
		if ((ColNumList.GetCount()) > 0)
		{
			// While not at end of columns used list
			posNum = ColNumList.GetHeadPosition();
			while(posNum)
			{
				// Get the next element from the column used list
				DBORDINAL iOrdinal = ColNumList.GetNext(posNum);
						
				// Get the beginning of the CCol list
				POSITION pos = m_ColList.GetHeadPosition(); 
				while(pos)
				{
					CCol& rCol = m_ColList.GetNext(pos);
			
					// Put into insert statement if column numbers are the same
					if(rCol.GetColNum()==iOrdinal)
					{
						// When building an insert statement and using an ini file MakeData is able
						// to return valid data for non-updateable cols and therefore doesn't
						// return DB_E_BADTYPE.  We want to allow that, so check here for updatable.
						if (!rCol.GetUpdateable())
						{
							pos = NULL;
							continue;
						}
						
						// MakeData returns a value in pwszValueData something like 'abc' or 123
						hr=GetLiteralAndValue(rCol,&pwszValueData,ulRowNumber + iRow,rCol.GetColNum(),eValue);
						
						// hack to allow inserting with literals with an .ini file
						// timestamp could get read back in differently than the literal that is 
						// used to create it from the .ini file
						if (rCol.GetProviderType()==DBTYPE_DBTIMESTAMP)
						{
							//get the scale
							WCHAR	*pwszDate	= wcsstr(pwszValueData,pwszX);
							WORD	wScale		= rCol.GetScale();

							//if there is a scale in this datetime
							if (pwszDate)
							{
								//if the amount of digits past the decimal point is more than the scale
								//trunc'em (subtract two for the decimal point and the end quote)
								if ((wcslen(pwszDate)-2)>wScale)
								{
									//assert if the to be turncated digits are not zero
									WCHAR	*pwszTrunc = wcsstr(pwszDate,pwszX);

									WORD i;
									//move past the good digits
									for (i = 0;i<wScale;i++,pwszTrunc++);

									i	= 0;
									while (pwszTrunc[i]!=L'\0')
									{
										i++;
										ASSERT(COMPARE(pwszTrunc[0],L'0'));
										pwszTrunc++;
									}

									//move past the decimal point
									pwszDate++;
									//put a ' and a \0 after the max number of scale digits
									pwszDate[wScale]=L'\'';
									pwszDate[wScale+1]=L'\0';
								}
							}
						}						
						
						if (FAILED(hr))
							goto CLEANUP;
							
						// Add column name 
						wcscat(pwszColNames,rCol.GetColName());
						wcscat(pwszColNames,wszCOMMA);

						// Add MakeData value
						if (fNULL && (1==rCol.GetNullable()) && (rCol.GetColNum() != GetIndexColumn()))
							// if NULL is required and supported!
							wcscat(pwszValuesClause, wszNULL);
						else
							wcscat(pwszValuesClause,pwszValueData);										
						wcscat(pwszValuesClause,wszCOMMA);
						
						// Free the MakeData value memory
						PROVIDER_FREE(pwszValueData);
					}
				}
			}
		}

		// StrColNames should look like	"col1,col2,"
		// StrValuesClause should look like	"makedata1, makedata2,"
		if ((ColNumList.GetCount())==0)
		{
			// Get beginning of list
			posm_ColList = m_ColList.GetHeadPosition();
			while(posm_ColList)
			{
				CCol& rCol = m_ColList.GetNext(posm_ColList);		

				// Returns in strWhereData something like 'abc' or 123
				CHECK(hr=GetLiteralAndValue(rCol,&pwszValueData,ulRowNumber + iRow,rCol.GetColNum(),eValue),S_OK);
				
				if (hr==DB_E_BADTYPE)
					break;
				
				if (FAILED(hr))
					goto CLEANUP;
					
				// Add Column name 
				wcscat(pwszColNames,rCol.GetColName());
				wcscat(pwszColNames,wszCOMMA);

				wcscat(pwszValuesClause,pwszValueData);										
				wcscat(pwszValuesClause,wszCOMMA);
					
				PROVIDER_FREE(pwszValueData);
				pwszValueData = NULL;
			}
		}
		
		if (*pwszValuesClause)
		{
			pwszSingleChar=wcsrchr(pwszValuesClause,(L','));
			lIndex = pwszSingleChar - pwszValuesClause;
			pwszValuesClause[lIndex]= L'\0';
		}
		
		if (*pwszColNames)
		{
			pwszSingleChar=wcsrchr(pwszColNames,(L','));
			lIndex = pwszSingleChar - pwszColNames ;
			pwszColNames[lIndex]= L'\0';
		}

		// Build sql statement 
		pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
						wcslen(wszINSERTINTO) + 
						wcslen(GetTableName()) +
						wcslen(wszLEFTPAREN) +
						wcslen(pwszColNames) +
						wcslen(wszRIGHTPAREN) +
						wcslen(wszVALUES) +
						wcslen(wszLEFTPAREN) +
						wcslen(pwszValuesClause) +
						wcslen(wszRIGHTPAREN))) +
						sizeof(WCHAR));

		wcscpy(pwszSQLText,wszINSERTINTO); 
		wcscat(pwszSQLText,GetTableName());
		wcscat(pwszSQLText,wszLEFTPAREN);
		wcscat(pwszSQLText,pwszColNames);
		wcscat(pwszSQLText,wszRIGHTPAREN);
		wcscat(pwszSQLText,wszVALUES);
		wcscat(pwszSQLText,wszLEFTPAREN);
		wcscat(pwszSQLText,pwszValuesClause);
		wcscat(pwszSQLText,wszRIGHTPAREN);

		PRVTRACE(L"%sInsert):%s\n",wszPRIVLIBT,pwszSQLText);

		// Execute statement
		if (fExecute==TRUE)
		{

			hr=	BuildCommand(pwszSQLText,	//SQL text to execute
							IID_NULL,		//This won't be used
							EXECUTE_ALWAYS,	//Should always execute fine
							0, NULL,		//Set no properties
							NULL,			//No parmeters									
							NULL, 			//No RowsAffected
							NULL,		    //No rowset needs to be allocated
							&m_pICommand);	//Use this object's command to execute on

			// release memory
			if (iRow+1 < cRowsToInsert)
				PROVIDER_FREE(pwszSQLText);
		
			if (FAILED(hr))
				goto CLEANUP;

			// # of rows in table
			m_ulRows++;
			m_ulNextRow++;
		}
	}

CLEANUP:
	//Return sql statement (if requested)
	if(SUCCEEDED(hr) && ppwszInsert)
		*ppwszInsert = pwszSQLText;
	else
		PROVIDER_FREE(pwszSQLText);
	return hr;
}


//---------------------------------------------------------------------------
// CTable::InsertWithUserLiterals	
//
// Inserts 1 row into the table using values that the user specifies
// Always uses commands with literals.
//
// @mfunc	Insert
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | No Columns Marked for use or Execution of statement failed
//
//---------------------------------------------------------------------------
HRESULT CTable::InsertWithUserLiterals(	
	DBORDINAL cCols,					// number of values to insert	
	DBORDINAL *pulColOrdinals,		// array of column ordinals
	WCHAR	**pwszLiteralValues	// values to insert
)
{
	BOOL		fReturn = FALSE;
	HRESULT 	hr=0;
	LONG_PTR	lIndex=0;
	DBORDINAL	i=0;
	WCHAR	 	pwszValuesClause[20000];	// values to insert into columns
	WCHAR * 	pwszSQLText=NULL;			// sql text to execute
	WCHAR * 	pwszValueData=NULL;			// value for where clause
	WCHAR	 	pwszColNames[20000];		// name of column
	WCHAR *		pwszSingleChar=NULL;
	
	pwszValuesClause[0] = L'\0';
	pwszColNames[0] = L'\0';
	
	// Get column names and values for column - there is a where clause
	for ( i = 0; i < cCols; i++ )
	{
		CCol col;
		GetColInfo(pulColOrdinals[0] ,col);
		pwszValueData = *(pwszLiteralValues + i);
								
		if (!col.GetUpdateable())
			return E_FAIL;

		// Add column name 
		wcscat(pwszColNames,col.GetColName());
		wcscat(pwszColNames,wszCOMMA);

		// add User Literals
		if(col.GetPrefix())
			wcscat(pwszValuesClause,col.GetPrefix());

		wcscat(pwszValuesClause,pwszValueData);										
		
		if(col.GetSuffix())
			wcscat(pwszValuesClause,col.GetSuffix());
	
		wcscat(pwszValuesClause,wszCOMMA);
	}

	
	if (*pwszValuesClause)
	{
		pwszSingleChar=wcsrchr(pwszValuesClause,(L','));
		lIndex = pwszSingleChar - pwszValuesClause;
		pwszValuesClause[lIndex]= L'\0';
	}
	
	if (*pwszColNames)
	{
		pwszSingleChar=wcsrchr(pwszColNames,(L','));
		lIndex = pwszSingleChar - pwszColNames ;
		pwszColNames[lIndex]= L'\0';
	}

	// Build sql statement 
	pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
					wcslen(wszINSERTINTO) + 
					wcslen(GetTableName()) +
					wcslen(wszLEFTPAREN) +
					wcslen(pwszColNames) +
					wcslen(wszRIGHTPAREN) +
					wcslen(wszVALUES) +
					wcslen(wszLEFTPAREN) +
					wcslen(pwszValuesClause) +
					wcslen(wszRIGHTPAREN))) +
					sizeof(WCHAR));

	wcscpy(pwszSQLText,wszINSERTINTO); 
	wcscat(pwszSQLText,GetTableName());
	wcscat(pwszSQLText,wszLEFTPAREN);
	wcscat(pwszSQLText,pwszColNames);
	wcscat(pwszSQLText,wszRIGHTPAREN);
	wcscat(pwszSQLText,wszVALUES);
	wcscat(pwszSQLText,wszLEFTPAREN);
	wcscat(pwszSQLText,pwszValuesClause);
	wcscat(pwszSQLText,wszRIGHTPAREN);

	PRVTRACE(L"%sInsert):%s\n",wszPRIVLIBT,pwszSQLText);

	hr=	BuildCommand(pwszSQLText,	//SQL text to execute
					IID_NULL,		//This won't be used
					EXECUTE_ALWAYS,	//Should always execute fine
					0, NULL,		//Set no properties
					NULL,			//No parmeters									
					NULL, 		//No RowsAffected
					NULL,		    //No rowset needs to be allocated
					NULL);	

	if (FAILED(hr))
	{
		PRVTRACE(L"%sInsert):%s\n",wszPRIVLIBT,wszFAILED);
		goto CLEANUP;
	}
	else
	{
		fReturn = TRUE;
		PRVTRACE(L"%sInsert):%s\n",wszPRIVLIBT,wszSUCCEEDED);
	}

CLEANUP:
	PROVIDER_FREE(pwszSQLText);
	
	if (fReturn == FALSE)
	{
		PRVTRACE(L"%sInsert):%s\n",wszPRIVLIBT,wszFUNCFAIL);
		return hr;
	}

	return S_OK;
}


//---------------------------------------------------------------------------
// CTable::InsertWithParams
//
// CTable				|
// InsertWithParams	|
// Inserts 1 row into the table. If ulRowNumber == 0 then ulRowNumber
// == m_ulNextRow. Rownumbers start at 0 in the private library, regardless
// of how they are treated in the Data Source. 
// ppwszInsert should look like "Insert into table (col1,...) values (?, ?, ...)".
// inserting only updateable columns.
//
// @mfunc	Insert
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | No Columns Marked for use or Execution of statement failed
//
//---------------------------------------------------------------------------
HRESULT CTable::InsertWithParams(	
	DBCOUNTITEM	ulRowNumber,	// @parm [IN]  Row to insert (Default = 0, meaning next row)
	EVALUE 		eValue,			// @parm [IN]  MakeData type to insert (Default = PRIMARY)
	BOOL 		fExecute,		// @parm [IN]  TRUE means execute statement (Default = TRUE)
	WCHAR **	ppwszInsert,	// @parm [OUT] Memory to place SQL text, if commands are supported (Default = NULL)
	DBCOUNTITEM	cRowsToInsert
)
{

	HRESULT 		hr				= E_FAIL;
	DBORDINAL *		rgColsToBind	= NULL;	
	ICommandWithParameters * pICmdWPar = NULL;
	HACCESSOR		hAccessor	= DB_NULL_HACCESSOR;
	DBORDINAL		idx			= 0;	
	DBCOUNTITEM		iRow = 0;
	DBCOUNTITEM		cBindings	= 0;
	DBBINDING *		rgBindings	= NULL;
	DBPARAMBINDINFO * pParamBindInfo = NULL;
	DBLENGTH		cbRowSize	= 0;
	void *			pData		= NULL;
	DB_LORDINAL *	rgColOrds	= NULL;
	DB_UPARAMS *	rgParamOrds = NULL;
	DBPARAMS		Params;
	POSITION		pos;
	DB_UPARAMS		cParams;
	DBPARAMINFO *	pParamInfo = NULL;

	// Declare our own memory for parameters passed to CreateSQLStmt, since 
	// these are required params but we don't want our caller to have to pass
	// these to us if they don't care about getting them back.
	WCHAR *	wszStmt=NULL;

	// If we don't support commands, call the private insert 
	// function, which knows how to insert without a command
	if(!GetCommandSupOnCTable() || !GetSQLSupport())
		return Insert(eValue, ulRowNumber);

	// Find row number to use on insert
	if (ulRowNumber == 0)
		ulRowNumber = m_ulNextRow;
	
	if (!CHECK(hr = CreateSQLStmt(INSERT_ALLWITHPARAMS, NULL, &wszStmt, NULL, NULL), S_OK))
		goto CLEANUP;
		
	if (fExecute)
	{
		// Get memory to hold array of all col numbers.  
		// This is the max possible, we won't necessarily use them all.
		rgColOrds = (DB_LORDINAL *)PROVIDER_ALLOC(m_ColList.GetCount() * sizeof(DB_LORDINAL));
		
		if (!rgColOrds)
		{
			hr = E_OUTOFMEMORY;
			goto CLEANUP;
		}

		rgParamOrds = (DB_UPARAMS *)PROVIDER_ALLOC(m_ColList.GetCount() * sizeof(DB_UPARAMS));
		
		if (!rgParamOrds)
		{
			hr = E_OUTOFMEMORY;
			goto CLEANUP;
		}

		if((m_ColList.GetCount()) == 0)
		{
			hr = E_FAIL;
			goto CLEANUP;
		}

		// While not at end of column list
		pos = m_ColList.GetHeadPosition();
		while(pos)
		{													
			// Get the next element from the column list
			CCol& rCol = m_ColList.GetNext(pos);
			
			// Record the column number in the array
			// if it is updateable
			if (rCol.GetUpdateable())
			{
				rgColOrds[idx] = rCol.GetColNum();	
				rgParamOrds[idx] = idx + 1;
				idx++;
			}
		}

		// Get the interface for m_pICommand.
		// Create a Update command with parameters
		if (!CHECK (ExecuteCommand(SELECT_UPDATEABLE, IID_IUnknown,
						NULL, NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, 
						NULL, NULL, &m_pICommand), S_OK))
			goto CLEANUP;
		
		
		// Get the Bindings for the Updateable columns only.
		// Passing an array containing the column ordinals for updateable columns only.
		if (!CHECK(GetAccessorAndBindings(m_pICommand, 
						DBACCESSOR_PARAMETERDATA,
						&hAccessor, &rgBindings, &cBindings, &cbRowSize,			
			  			DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
						UPDATEABLE_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, NULL,
						NULL, DBTYPE_EMPTY, idx, rgColOrds,	rgParamOrds,
						NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, BLOB_LONG), S_OK))
			goto CLEANUP;

		// Create associated DBPARAMBINDINFO structure for providers that can't derive 
		// parameter information, otherwise Execute result is "undefined" per spec.
		if(VerifyInterface(m_pICommand, IID_ICommandWithParameters, COMMAND_INTERFACE, (IUnknown**)&pICmdWPar))
		{
			// If GetParameterInfo returns DB_E_PARAMUNAVAILABLE the provider can't derive, otherwise
			// the only other expected hr here is S_OK indicating provider can derive
			if (DB_E_PARAMUNAVAILABLE == (hr = pICmdWPar->GetParameterInfo(&cParams, &pParamInfo, NULL)))
			{
				TESTC_(hr = GetDBPARAMBINDINFO(cBindings, rgBindings, rgColOrds, NULL, &pParamBindInfo), S_OK);
				TESTC_(hr = pICmdWPar->SetParameterInfo(cBindings, rgParamOrds, pParamBindInfo), S_OK);
			}
			else
				TESTC_(hr, S_OK);
		}

		// Get a buffer big enough for this accessor
		pData = (BYTE *)PROVIDER_ALLOC((size_t)cbRowSize);
		if(!pData)
		{
			hr = E_OUTOFMEMORY;
			goto CLEANUP;
		}
	
		//Loop over the number of rows to insert
		for(iRow=0; iRow<cRowsToInsert; iRow++)
		{
			// Fill buffer with appropriate data for insert of this row number
			if (!SUCCEEDED(FillInputBindings(this, 		
				DBACCESSOR_PARAMETERDATA, cBindings, rgBindings,	
				(BYTE**)&pData,	ulRowNumber + iRow, idx, rgColOrds))) 
				goto CLEANUP;
		
			// Create Param structure.
			Params.cParamSets = 1;
			Params.hAccessor = hAccessor;
			Params.pData = pData;
			
			// BuildCommand will accept NULL parameters from user,
			// in which case it just allocates the memory it needs to
			// perform the functionality and deallocates it before returning;
			// thus we don't have to declare our own temp vars to pass in.
			hr = BuildCommand(wszStmt, IID_NULL, EXECUTE_ALWAYS, 0, NULL,
					&Params, NULL, NULL, &m_pICommand);

			// We have to release the data before filling again or we'll have a leak
			ReleaseInputBindingsMemory(cBindings, rgBindings, (BYTE *)pData, FALSE); 

			if(FAILED(hr))
				goto CLEANUP;

			// # of rows in table
			m_ulRows++;
			m_ulNextRow++;
		}
	}

CLEANUP:

	// If we set parameter info we should reset in case of other executions
	if (pICmdWPar)
		CHECK(pICmdWPar->SetParameterInfo(0, NULL, NULL), S_OK);
	SAFE_RELEASE(pICmdWPar);

	// Free provider type names
	if (pParamBindInfo)
	{
		for (idx = 0; idx < cBindings; idx++)
			SAFE_FREE(pParamBindInfo[idx].pwszDataSourceType);
	}

	// Free DBPARAMBINDINFO array
	SAFE_FREE(pParamBindInfo);
	SAFE_FREE(pParamInfo);

	// Free stuff.
	SAFE_FREE(pData);
	FreeAccessorBindings(cBindings, rgBindings);

	PROVIDER_FREE (rgColOrds);
	PROVIDER_FREE (rgParamOrds);

	if (hAccessor != DB_NULL_HACCESSOR)
	{
		IAccessor *pIAccessor = NULL;
		if(VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE, (IUnknown**)&pIAccessor))
		{
			pIAccessor->ReleaseAccessor(hAccessor, NULL);
			SAFE_RELEASE(pIAccessor);
		}
	}

	if (SUCCEEDED(hr))
	{
		// Assign out parameter if caller wanted it from CreateSQLStmt
		if (ppwszInsert)						  
			*ppwszInsert = wszStmt;
		else
			PROVIDER_FREE(wszStmt);		
	}
	else
	{
		// Free what we've allocated and give user NULL back
		PROVIDER_FREE(wszStmt);

		if (ppwszInsert)
			*ppwszInsert = NULL;
	}
	
	return hr;
}

//---------------------------------------------------------------------------
// CTable::Insert
//
// CTable				|
// Insert				|
// Inserts 1 row in table using IRowsetChange::InsertRow. This function is called
// by the public Insert function.
//
// @mfunc	Insert
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | No Columns Marked for use or SetNewData failed
//
//---------------------------------------------------------------------------
HRESULT CTable::Insert(
			EVALUE 		eValue,			// @parm [IN]  Initial or second value of data 
			DBCOUNTITEM ulRowNumber,	// @parm [IN]  Row number to insert
			DBCOUNTITEM	cRowsToInsert
		)
{
	HRESULT				hr = E_FAIL;

	HACCESSOR			hAccessor = NULL;
	void*				pData = NULL;
	IRowsetChange*		pIRowsetChange = NULL;

	DBCOUNTITEM iRow = 0;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	
	// Find row number to use on insert
	if (ulRowNumber == 0)
		ulRowNumber = m_ulNextRow;

	//Use CursorEngine ( if requested )
	if(GetModInfo()->UseServiceComponents() & SERVICECOMP_CURSORENGINE)
	{
		SetProperty(DBPROP_CLIENTCURSOR, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_BOOL, VARIANT_TRUE);
	}
	else
	{
		//Need to set DBPROP_UPDATABILITY
		if(SettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, m_pIDBInitialize, DATASOURCE_INTERFACE))
			SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_I4, DBPROPVAL_UP_INSERT);
	}


	//Now get the rowset which we'll use to populate the table
	//NOTE:  We don't use CreateRowset here, since that will give us a "generic" 
	//rowset which depending upon InitString maybe another rowset source, here
	//we are inserting data into the table...
	QTESTC_(hr = m_pIOpenRowset->OpenRowset(
		NULL, 
		&m_TableID, 
		NULL,
		IID_IRowsetChange, 
		cPropSets, 
		rgPropSets, 
		(IUnknown **)&pIRowsetChange),S_OK);

	//Get an accessor with all the writeable columns
	QTESTC_(hr = GetAccessorAndBindings(pIRowsetChange, 
		DBACCESSOR_ROWDATA,
		&hAccessor, &rgBindings, &cBindings, NULL,
		DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL,
		NULL, NULL,	DBTYPE_EMPTY, 0, NULL, NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	//Insert the requested Number of rows...
	for(iRow=0; iRow<cRowsToInsert; iRow++)
	{
		// Fill buffer with appropriate data for insert of this row number
		QTESTC_(hr = FillInputBindings(
			this,
			DBACCESSOR_ROWDATA,		
			cBindings,
			rgBindings,	
			(BYTE**)&pData,		
			ulRowNumber + iRow,
			0,
			NULL),S_OK);

		QTESTC_(hr = pIRowsetChange->InsertRow(NULL, hAccessor, pData, NULL),S_OK);

		// We have successfully inserted if we got this far
		m_ulRows++;
		m_ulNextRow++;
	}

CLEANUP:
	if(hAccessor && pIRowsetChange)
	{
		IAccessor * pIAccessor;
		ULONG		cRefCounts=ULONG_MAX;

		VerifyInterface(pIRowsetChange, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor);
		if (pIAccessor)
		{
			pIAccessor->ReleaseAccessor(hAccessor, &cRefCounts);
			SAFE_RELEASE(pIAccessor);
			COMPARE(cRefCounts,0);
		}
	}

	if(pData)
		ReleaseInputBindingsMemory(cBindings, rgBindings, (BYTE*)pData, TRUE);
	FreeAccessorBindings(cBindings, rgBindings);
	SAFE_RELEASE(pIRowsetChange);
	FreeProperties(&cPropSets, &rgPropSets);
	return hr;
}

//---------------------------------------------------------------------------
// CTable::Update 
//
// CTable				|
// Update				|
// Updates 1 row in table. Client must have at least one column marked
// or an error will occur. eValue is the type of MakeData data you are going to
// update the column to (the end result).
//
// @mfunc	Updata
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | No Columns Marked for use or Execution of statement failed
//
//---------------------------------------------------------------------------
HRESULT CTable::Update(	
	DBCOUNTITEM	ulRowNumber,		// @parm [IN]  Row # passed to MakeData (Default = 0, meaning last row inserted into table)
	EVALUE 		eValue,				// @parm [IN]  Type of MakeData (Default=SECONDARY)
	BOOL 		fExecute,			// @parm [IN]  Execute pstrSelect (Default=TRUE)
	WCHAR		**ppwszUpdate,		// @parm [OUT] Update statement (Default = NULL)
	BOOL		fSameWhereValuesAsSet,//@parm [IN] Whether the same eValue is used for where clause as set clause
	DBCOUNTITEM	ulWhereRowNumber	//@param [IN] if not 0 then update table set ulRowNumber where ulWhereRowNumber
)
{
	POSITION 	posNum;				// position in ColNumList
	POSITION 	posm_ColList;				// position in m_ColList
	CList 		<DBORDINAL,DBORDINAL> ColNumList;	// list of column numbers marked
	EVALUE 		eOldValue;					// opposite of eValue
	BOOL		fReturn = FALSE;
	HRESULT 	hr=0;
	LONG_PTR	lIndex=0;
	DBORDINAL	iOrdinal=0;					// column number in m_ColList
	WCHAR *		pwszValueData=NULL;
	WCHAR *		pwszValueNewData=NULL;
	WCHAR *		pwszSetClause=NULL;			// name of column
	WCHAR *		pwszWhereClause=NULL;		// values to insert into columns
	WCHAR *		pwszTemp=NULL;
	WCHAR *		pwszSQLText=NULL;			// sql text to execute
	WCHAR *		pwszSingleChar=NULL;

	if (ulRowNumber == 0)
		ulRowNumber = m_ulRows;				// last row inserted into table
	
	ColNumList.RemoveAll();

	pwszSetClause = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) * 20000);
	pwszSetClause[0]='\0';

	pwszWhereClause = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) * 20000);
	pwszWhereClause[0]='\0';

	pwszTemp = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) * 20000);
	pwszTemp[0]='\0';
	
	// Get Marked Columns 
	if (FAILED(hr=GetColsUsed(ColNumList)))
		goto CLEANUP;

	// Get set and where clause
	// If want the where clause and set clause to match, causing in affect a no-op
	if (fSameWhereValuesAsSet)
	{
		// Make old value (where criteria) match the new value
		eOldValue = eValue;
	}
	else
	{
		// Need to switch to the other value for set clause, causing a real update
		if(eValue==PRIMARY)
			eOldValue=SECONDARY;
		else
			eOldValue=PRIMARY;
	}

	// Get beginning of both lists
	posNum = ColNumList.GetHeadPosition();

	// While not at end of ColNumList
	while(posNum!=NULL)
	{
		// Get next marked column number out of ColNumList
		iOrdinal=ColNumList.GetAt(posNum);

		// While not at end of m_ColList
		posm_ColList = m_ColList.GetHeadPosition();
		while(posm_ColList)
		{
			// Go to next object in m_ColList
			CCol& rCol = m_ColList.GetNext(posm_ColList);	
			
			if (rCol.GetColNum()==iOrdinal)
			{
				// When using ini file we can't depend on GetLiteralAndValue returning
				// DB_E_BADTYPE for nonupdateable cols.
				if (!rCol.GetUpdateable())
					break;

				//if ulWhereRowNumber is zero then ignore this param
				if (!ulWhereRowNumber)
				{
					// set clause will need the eValue passed to make data to get the NEW
					// value to update the field with
					hr=GetLiteralAndValue(rCol,&pwszValueNewData,ulRowNumber,rCol.GetColNum(),eValue);
				}
				else
				{
					// get To data
					hr=GetLiteralAndValue(rCol,&pwszValueNewData,ulRowNumber,rCol.GetColNum(),PRIMARY);
				}

				if (FAILED(hr))
					return hr;

				// Get set clause
				wcscat(pwszSetClause,rCol.GetColName());
				wcscat(pwszSetClause,wszEQUALS);
				wcscat(pwszSetClause,pwszValueNewData);
				wcscat(pwszSetClause,wszCOMMA);

				//if ulWhereRowNumber is zero then ignore this param
				if (!ulWhereRowNumber)
				{
					// Where clause will need the eNotValue passed to get the current value
					hr=GetLiteralAndValue(rCol,&pwszValueData,ulRowNumber,rCol.GetColNum(),eOldValue);
				}
				else
				{
					// get To data
					hr=GetLiteralAndValue(rCol,&pwszValueData,ulWhereRowNumber,rCol.GetColNum(),PRIMARY);
				}

				if (hr==DB_E_BADTYPE)
					break;

				if(FAILED(hr))
					goto CLEANUP;
					
				// Use right word in where clause
				if (rCol.GetSearchable() != DB_UNSEARCHABLE)
				{
					wcscat(pwszWhereClause,rCol.GetColName());

					// Get where clause
					if (rCol.GetSearchable()==DB_LIKE_ONLY)
					{
						if (wcslen(pwszValueData) > 254)
						{
							// SQL Server won't accept a literal string value longer than 255 bytes
							// in the where clause, so we truncate the search value and use a LIKE statement.
							// For international strings, we cut of at the 126th unicode char, so when converted
							// to ansi, the string will remain under the 255 byte limit.
							if ( GetModInfo()->GetLocaleInfo() )
							{
								pwszValueData[126] = L'\0';
							}
							else
							{
								pwszValueData[252]=L'\0';
							}

							wcscat(pwszValueData,wszEndOfLongData);
						}
						
						wcscat(pwszWhereClause,wszLIKE);
					}
					
					if ((rCol.GetSearchable()==DB_ALL_EXCEPT_LIKE) || 
						(rCol.GetSearchable()==DB_SEARCHABLE))
						wcscat(pwszWhereClause,wszEQUALS);

					wcscat(pwszWhereClause,pwszValueData);
					wcscat(pwszWhereClause,wszAND);
				}

				if (pwszValueData && pwszValueNewData)
					if (0==wcscmp(pwszValueData,pwszValueNewData))
						PRVTRACE(L"*** values are the same\n");

				// Memset(pwszValueData,L'T',wcslen(pwszValueData));
				PROVIDER_FREE(pwszValueData);
				pwszValueData = NULL;

				// Memset(pwszValueNewData,L'T',wcslen(pwszValueNewData));
				PROVIDER_FREE(pwszValueNewData);
				pwszValueNewData = NULL;

				// Skip to end of list
				break;
			}
		} // Go to next member of ColNumList
		ColNumList.GetNext(posNum);
	}

	// Get rid of last comma from strSetClause
	if (wcslen(pwszSetClause))
	{
			pwszSingleChar=wcsrchr(pwszSetClause,L',');
			lIndex = pwszSingleChar - pwszSetClause;
			pwszSetClause[lIndex]= L'\0';
	}

	// Get rid of last 'and' from strWhereClause
	if (wcslen(pwszWhereClause))
	{
			pwszSingleChar=wcsrchr(pwszWhereClause,L'A');
			lIndex = pwszSingleChar - pwszWhereClause;
			pwszWhereClause[lIndex]= L'\0';
	}

	// Build sql statement 
	pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
					wcslen(wszUPDATE_1ROW) +
					wcslen(GetTableName()) +
					wcslen(pwszSetClause) +
					wcslen(pwszWhereClause))) +
					sizeof(WCHAR));

	swprintf(pwszSQLText, wszUPDATE_1ROW,GetTableName(),pwszSetClause,pwszWhereClause);
	PRVTRACE(L"%s\n",pwszSQLText);

	// Execute statement
	if (fExecute==TRUE)
	{	
		hr=	BuildCommand(pwszSQLText,	//SQL text to execute
						IID_NULL,		//This won't be used
						EXECUTE_ALWAYS,	//Should always execute fine
						0, NULL,		//Set no properties
						NULL,			//No parmeters									
						NULL, 			//No RowsAffected info
						NULL,			//No rowset needs to be allocated
						&m_pICommand);	//Use this object's command to execute on

		if (FAILED(hr))
		{
			PRVTRACE(L"%sUpdate):%s\n",wszPRIVLIBT,wszFAILED);
			goto CLEANUP;
		}
		else
		{
			PRVTRACE(L"%sUpdate):%s\n",wszPRIVLIBT,wszSUCCEEDED);
			fReturn = TRUE;
		}
	}

	// Return sql statement
	if (ppwszUpdate)
	{
		(*ppwszUpdate) = wcsDuplicate(pwszSQLText);
		fReturn = TRUE;
	}

CLEANUP:
	
	PROVIDER_FREE(pwszWhereClause);
	PROVIDER_FREE(pwszTemp);
	PROVIDER_FREE(pwszSetClause);
	PROVIDER_FREE(pwszSQLText);

	if (fReturn == FALSE)
	{
		PRVTRACE(L"%sUpdate):%s",wszPRIVLIBT,wszFUNCFAIL);
		return hr;
	}

	return S_OK;
}       

//---------------------------------------------------------------------------
// CTable::Delete 
//
// CTable	|
// Delete	|
// Deletes 1 row or all rows. If pstrSelect is NULL, client will not
// be returned the sql text of the delete statement. eValue is the current
// value of the data to be deleted.
//
// @mfunc	Delete
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::Delete(	
	DBCOUNTITEM	ulRow,		// @parm [IN]  row # to pass to MakeData (Default = ALLROWS)
	EVALUE 		eValue,		// @parm [IN]  Type of MakeData (Default=PRIMARY)
	BOOL 		fExecute,	// @parm [IN]  Execute pstrSelect (Default=TRUE)
	WCHAR ** 	pwszDelete	// @parm [OUT] Delete statement (Default=NULL)
)
{
	POSITION 	posNum;				// position in ColNumList
	POSITION 	posm_ColList;				// position in m_ColList
	CList 		<DBORDINAL,DBORDINAL> ColNumList;	// list of column numbers marked
	BOOL		fReturn = FALSE;
	HRESULT 	hr=0;
	LONG_PTR	lIndex=0;
	DBORDINAL	iOrdinal=0;					// column number in ColNumList
	WCHAR *		pwszSQLText=NULL;			// sql text to execute
	WCHAR *		pwszWhereClause = NULL;		// should look like "col1=x and col2=y..."
	WCHAR *		pwszWhereData=NULL;
	WCHAR *		wszSingleChar=NULL;

	// Get Column Names or *
	if (ulRow==ALLROWS)
	{
		pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
						wcslen(wszDELETE_ALLROWS) + 
						wcslen(GetTableName()))) +
						sizeof(WCHAR));
		
		// Format SQL Statement
		swprintf(pwszSQLText, wszDELETE_ALLROWS, GetTableName());
		PRVTRACE(L"%sDelete):%s\n",wszPRIVLIBT,pwszSQLText);
	}	
	else
	{	
		// Arbitrary large amount
		pwszWhereClause = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) * 20000);
		
		if (!pwszWhereClause)
			goto CLEANUP;
		
		pwszWhereClause[0] = L'\0';
	
		// Get Marked Columns 
		if (FAILED(hr=GetColsUsed(ColNumList)))
			goto CLEANUP;
			
		// Quit if list is empty
		if (ColNumList.IsEmpty())
		{
			PRVTRACE(L"%sDelete):ColNumList is empty, %s\n",wszPRIVLIBT,wszFUNCFAIL);
			goto CLEANUP; 
		}
	
		// Get Column Names and Where Clause
		// Get beginning of both lists
		posNum = ColNumList.GetHeadPosition();

		// While not at end of ColNumList
		while(posNum!=NULL)
		{
			iOrdinal=ColNumList.GetAt(posNum);	
			posm_ColList = m_ColList.GetHeadPosition();
			
			// While not at end of m_ColList
			while(posm_ColList!=NULL)
			{
				CCol& rCol = m_ColList.GetNext(posm_ColList);
				
				if (rCol.GetColNum()==iOrdinal)
				{
					hr=GetLiteralAndValue(rCol,&pwszWhereData,ulRow,rCol.GetColNum(),eValue);
					if (hr==DB_E_BADTYPE)
						break;
					
					if (FAILED(hr))
						goto CLEANUP;
		
					// Use right word in where clause or don't use
					// in where clause at all if it is unsearchable
					if (rCol.GetSearchable() != DB_UNSEARCHABLE)
					{
						wcscat(pwszWhereClause,rCol.GetColName());

						// We have to handle NULL data specially
						if (wcscmp(pwszWhereData, L"NULL"))
						{
							
							// Get where clause
							if (rCol.GetSearchable()==DB_LIKE_ONLY)
							{
								if (wcslen(pwszWhereData) > 254)
								{
									// SQL Server won't accept a literal string value longer than 255 bytes
									// in the where clause, so we truncate the search value and use a LIKE statement.
									// For international strings, we cut of at the 126th unicode char, so when converted
									// to ansi, the string will remain under the 255 byte limit.
									if ( GetModInfo()->GetLocaleInfo() )
									{
										pwszWhereData[126] = L'\0';
									}
									else
									{
										pwszWhereData[252]=L'\0';
									}
									wcscat(pwszWhereData,wszEndOfLongData);
								}
								
								wcscat(pwszWhereClause,wszLIKE);
							}
							
							if ((rCol.GetSearchable()==DB_ALL_EXCEPT_LIKE) || 
								(rCol.GetSearchable()==DB_SEARCHABLE))
								wcscat(pwszWhereClause,wszEQUALS);
						}
						else
							wcscat(pwszWhereClause, L" IS ");

						wcscat(pwszWhereClause,pwszWhereData);
						wcscat(pwszWhereClause,wszAND);
					}

					PROVIDER_FREE(pwszWhereData);
					pwszWhereData = NULL;
					
					// Go to next member of ColNumList
					break;
				}
			}

			ColNumList.GetNext(posNum);
		}

		// Get rid of last 'and' from strWhereClause
		if (pwszWhereClause)
		{
			wszSingleChar=wcsrchr(pwszWhereClause,(L'A'));
			if( wszSingleChar )
				lIndex = wszSingleChar - pwszWhereClause;
			pwszWhereClause[lIndex]= L'\0';
		}

		// Build sql statement 
		pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
						wcslen(wszDELETE_1ROW) +
						wcslen(GetTableName()) + 
						wcslen(pwszWhereClause))) +
						sizeof(WCHAR));

		swprintf(pwszSQLText, wszDELETE_1ROW, GetTableName(),pwszWhereClause);
	}

	// Execute statement
	if (fExecute==TRUE)
	{
		hr=	BuildCommand(pwszSQLText,	//SQL text to execute
						IID_NULL,		//This won't be used
						EXECUTE_ALWAYS,	//Should always execute fine
						0, NULL,		//Set no properties
						NULL,			//No parmeters									
						NULL, 			//No RowsAffected info
						NULL,			//No rowset needs to be allocated
						&m_pICommand);	//Use this object's command to execute on

		if (FAILED(hr))
			goto CLEANUP;
		else
		{
			if (ulRow == ALLROWS)
			{
				PRVTRACE(L"%sDelete): All rows succeeded\n",wszPRIVLIBT);
				m_ulRows	= 0;
				m_ulNextRow = 1;
			}
			else
			{
				PRVTRACE(L"%sDelete): Row %d %s\n", wszPRIVLIBT,ulRow,wszSUCCEEDED);
				m_ulRows	-= 1;
				m_ulNextRow -= 1;
			}

			fReturn = TRUE;
		}
	}

	// Return sql statement
	if (pwszDelete)
	{ 
		(*pwszDelete) = wcsDuplicate(pwszSQLText);
		fReturn = TRUE;
	}

CLEANUP:

	PROVIDER_FREE(pwszSQLText);
	PROVIDER_FREE(pwszWhereClause);
	PROVIDER_FREE(pwszWhereData);
	
	if (fReturn == FALSE)
	{
		PRVTRACE(L"%sDelete):%s\n",wszPRIVLIBT,wszFUNCFAIL);
		return hr;
	}

	return S_OK;
}       
//---------------------------------------------------------------------------
// CTable::DeleteRows
//
// CTable				|
//
// @mfunc	DeleteRows
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | 
//
//---------------------------------------------------------------------------
HRESULT CTable::DeleteRows(			
		DBCOUNTITEM		ulRowNumber // @parm [IN]  row # to pass  (Default = ALLROWS)	
)
{
	HRESULT				hr = E_FAIL;
	
	DBCOUNTITEM			cRowsObtained=0;
	HROW*				rghRows=NULL;
	IRowset*			pIRowset	= NULL;
	IRowsetChange*		pIRowsetChange = NULL;
	
	ULONG				cPropSets = 0;
	DBPROPSET*			rgPropSets = NULL;
	
	// no-op
	DBCOUNTITEM cTotalRows = CountRowsOnTable();
	if(cTotalRows == 0)
		return S_OK;
	
	//Use CursorEngine ( if requested )
	if(GetModInfo()->UseServiceComponents() & SERVICECOMP_CURSORENGINE)
	{
		SetProperty(DBPROP_CLIENTCURSOR, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_BOOL, VARIANT_TRUE);
	}
	else if(SettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, m_pIDBInitialize, DATASOURCE_INTERFACE))
	{
		//Need to set DBPROP_UPDATABILITY
		SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_I4, DBPROPVAL_UP_DELETE);
	}

	// Now get the rowset which we'll use to populate the table
	if(!SUCCEEDED(CreateRowset(USE_OPENROWSET, IID_IRowsetChange, 
						cPropSets, rgPropSets, (IUnknown **)&pIRowsetChange)))
		goto CLEANUP;

	if(!VerifyInterface(pIRowsetChange,IID_IRowset,ROWSET_INTERFACE,(IUnknown**)&pIRowset))
		goto CLEANUP;

	if( ulRowNumber == ALLROWS)
	{
		//TODO add check for status
		while (SUCCEEDED(hr = pIRowset->GetNextRows(0,0,(DBROWCOUNT)cTotalRows,&cRowsObtained,&rghRows)))
		{
			if(cRowsObtained == 0)
				break;

			if (!SUCCEEDED(pIRowsetChange->DeleteRows(NULL,cRowsObtained,rghRows, NULL)))
				goto CLEANUP;


			if (!SUCCEEDED(pIRowset->ReleaseRows(cRowsObtained,rghRows, NULL,NULL,NULL)))
				goto CLEANUP;

			cTotalRows -= cRowsObtained;
			cRowsObtained = 0;
			PROVIDER_FREE(rghRows);
		}
	}
	else
	{
		if(SUCCEEDED(hr = pIRowset->GetNextRows(0,
												ulRowNumber-1,
												1,
												&cRowsObtained,
												&rghRows)) && cRowsObtained)
		{

			ASSERT(cRowsObtained == 1);

			if (!SUCCEEDED(pIRowsetChange->DeleteRows(NULL,cRowsObtained,rghRows, NULL)))
				goto CLEANUP;

			if (!SUCCEEDED(pIRowset->ReleaseRows(cRowsObtained,rghRows, NULL,NULL,NULL)))
				goto CLEANUP;

		}
	}
	
	if ( SUCCEEDED(hr))
	{
		if (ulRowNumber == ALLROWS)
		{
			PRVTRACE(L"%sDelete): All rows succeeded\n",wszPRIVLIBT);
			m_ulRows	= 0;
			m_ulNextRow = 1;
		}
		else
		{
			PRVTRACE(L"%sDelete): Row %d %s\n", wszPRIVLIBT,ulRowNumber,wszSUCCEEDED);
			m_ulRows	-= 1;
			m_ulNextRow -= 1;
		}
	}

CLEANUP:
	PROVIDER_FREE(rghRows);

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetChange);
	FreeProperties(&cPropSets, &rgPropSets);
	return hr;
}

//---------------------------------------------------------------------------
// CTable::ExecuteCommand
//
// CTable			|
// ExecuteCommand	|
//
// Client must release the following objects:
//		- If pICommand is not null user must release this
// Client must free the following memory:
//		- If prgPropertyErrors is not null user must free this
//		- If pcRowset is zero then user must free prgpRowset
//		- ppwszStatement
//		- prgColumns
//
// This function takes a mandatory SQL statement.
//
// @mfunc	ExecuteCommand
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Problem
//
//---------------------------------------------------------------------------
HRESULT CTable::ExecuteCommand(
	EQUERY					eSQLStmt,			// @parm [IN]  SQL statement to create 
	REFIID					riid,				// @parm [IN] Interface to return
	WCHAR * 				pwszTableName,		// @parm [IN]  Second TableName, NULL if no second table applies
	WCHAR **				ppwszStatement,		// @parm [IN/OUT] Pointer to memory to place
												// address of SQL statement allocated by the provider.
												// Caller is responsible for freeing *ppwszStatement,
												// unless ppwszStatement is passed as NULL.
	DBORDINAL *				pcColumns,			// @parm [IN/OUT] Pointer to memory to place 
												// Count of columns.  If caller does not
												// want the count, NULL may be passed.
	DB_LORDINAL **				prgColumns,			// @parm [IN/OUT] Pointer to memory to place
												// address of array of column numbers, which 
												// is allocated by the function. Caller is responsible
												// for freeing *prgColumns unless prgColumns is passed
												// as NULL.
	EEXECUTE				eExecute,			// @parm [IN] TRUE = execute SQL Statement
	ULONG					cPropSets,			// @parm [IN] Count of DBPROPSET structures.
	DBPROPSET				rgPropSets[],		// @parm [IN] Array of DBPROPSET structures.
	DBROWCOUNT *			pcRowsAffected,		// @parm [OUT] Pointer to memory in which to return the count
												// of rows affected by a command that updates, deletes,
												// or inserts rows.
	IUnknown **				ppRowset,			// @parm [IN/OUT] Pointer to the rowset pointer.  If caller doesn't
												// want an interface pointer back, they may pass NULL for this parameter. 
	ICommand **				ppICommand			// @parm [IN/OUT] If ppICommand == NULL, this function uses
												// its own command object and then frees it, caller does nothing.
												// <nl>
												// If *ppICommand is not null, this value is used as the
												// ICommand interface for all command operations in this function. 
												// Caller maintains responsiblity to release this interface.
												// <nl>
												// If *ppICommand is null, this function creates a new command object
												// and places the ICommand interface to this object in *ppICommand.
												// Caller assumes responsibility to release this interface.
)
{	
	// Declare our own memory for parameters passed to CreateSQLStmt, since 
	// these are required params but we don't want our caller to have to pass
	// these to us if they don't care about getting them back.
	WCHAR *	wszStmt = NULL;
	HRESULT hr		= S_OK;
	
	if(FAILED(hr = CreateSQLStmt(eSQLStmt,pwszTableName,&wszStmt, pcColumns, prgColumns)))
		goto CLEANUP;

	// BuildCommand will accept NULL parameters from user,
	// in which case it just allocates the memory it needs to
	// perform the functionality and deallocates it before returning;
	// thus we don't have to declare our own temp vars to pass in.
	hr = BuildCommand(wszStmt,riid,eExecute,cPropSets,rgPropSets,
				NULL,pcRowsAffected,ppRowset,ppICommand);

CLEANUP:

	// Assign out parameter if caller wanted it from CreateSQLStmt
	if (SUCCEEDED(hr))
	{
		if (ppwszStatement)						  
			*ppwszStatement = wszStmt;
		else
			PROVIDER_FREE(wszStmt);		
	}
	else
	{
		//Free what we've allocated and give user NULL back
		PROVIDER_FREE(wszStmt);

		if (ppwszStatement)
			*ppwszStatement = NULL;
	}
	
	return hr;
}



//---------------------------------------------------------------------------
// CTable::CreateRowset
//
// CTable			|
// CreateRowset	|
//
//
// @mfunc	CreateRowset
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Problem
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateRowset(
		EQUERY				eQuery,					// [IN]  Query to create 
		REFIID				riid,					// [IN]  Interface pointer to return
		ULONG				cPropSets,				// [IN]  Count of property sets.
		DBPROPSET*			rgPropSets,				// [IN]  Array of DBPROPSET structures.
		IUnknown **			ppIRowset,				// [OUT] Pointer to the rowset pointer.
		DBID*				pTableID,				// {IN]  TableID if needed (IOpenRowset)
		DBORDINAL *			pcColumns,				// [OUT] Count of columns
		DB_LORDINAL **		prgColumns,				// [OUT] Array of column numbers
		ULONG				cRestrictions,
		VARIANT*			rgRestrictions,
		IOpenRowset*		pIOpenRowset		 	// @parm [IN/OUT]
)
{	
	HRESULT hr = E_FAIL;
	IDBSchemaRowset* pIDBSchemaRowset = NULL;
	GUID guidSchema;
	ISourcesRowset* pISourcesRowset = NULL;
	IColumnsRowset* pIColumnsRowset = NULL;
	CLSID clsidEnumerator;

	//The user may have requested through the InitString exactly how to create the
	//rowset, so override any parameter passed in if this is the case...
	if(GetModInfo()->GetRowsetQuery() != NO_QUERY)
		eQuery = GetModInfo()->GetRowsetQuery();

	//Commands may not be supported...
	if(!GetCommandSupOnCTable() && (eQuery == SELECT_ALLFROMTBL || eQuery == SELECT_ORDERBYNUMERIC))
		eQuery = USE_SUPPORTED_SELECT_ALLFROMTBL;
	
	//Determine how the create the rowset...
	switch(eQuery)
	{
		case USE_OPENROWSET:
		case USE_SUPPORTED_SELECT_ALLFROMTBL:
		{
			//IOpenRowset::OpenRowset
			if(!pIOpenRowset)
				pIOpenRowset = m_pIOpenRowset;

			//IOpenRowset::OpenRowset
			QTESTC_(hr = pIOpenRowset->OpenRowset(NULL, pTableID ? pTableID : &m_TableID, NULL, riid,
													cPropSets, rgPropSets, ppIRowset),S_OK);

			//Need to setup everything that ExecuteCommand does
			//ie: m_cRowsetCols, m_rgTableColOrds
			if(pcColumns)
				TESTC(GetQueryInfo(SELECT_ALLFROMTBL, pcColumns, prgColumns, NULL, NULL, NULL, NULL));
			break;
		}

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
		{
			//IDBSchemaRowset::GetRowset
			if(!pIOpenRowset)
				pIOpenRowset = m_pIOpenRowset;
			
			//Obtain the IDBSchemaRowset interface
			if(!VerifyInterface(pIOpenRowset, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset))
			{
				hr = E_NOINTERFACE;
				goto CLEANUP;
			}
			QTESTC(GetSchemaGUID(eQuery, &guidSchema));

			//IDBSchemaRowset::GetRowset
			QTESTC_(hr = pIDBSchemaRowset->GetRowset(NULL, guidSchema, 
				cRestrictions, rgRestrictions, riid, cPropSets, rgPropSets, ppIRowset),S_OK);

			//Need to setup everything that ExecuteCommand does
			//ie: m_cRowsetCols, m_rgTableColOrds
			//NOTE: We only do this if CTable is a schema rowset
			if(pcColumns && GetModInfo()->GetRowsetQuery() != NO_QUERY)
				TESTC(GetQueryInfo(SELECT_ALLFROMTBL, pcColumns, prgColumns, NULL, NULL, NULL, NULL));
			break;
		}

		case USE_ENUMERATOR:
		{
			//Obtain an Instance to the Enumerator
			QTESTC_(hr = CLSIDFromProgID(GetModInfo()->GetEnumerator(), &clsidEnumerator),S_OK);
			QTESTC_(hr = GetModInfo()->CreateProvider(clsidEnumerator, NULL, IID_ISourcesRowset, (IUnknown**)&pISourcesRowset),S_OK);

			//ISourcesRowset::GetSourcesRowset
			QTESTC_(hr = pISourcesRowset->GetSourcesRowset(NULL, riid, cPropSets, rgPropSets, ppIRowset),S_OK);

			//Need to setup everything that ExecuteCommand does
			//ie: m_cRowsetCols, m_rgTableColOrds
			//NOTE: We only do this if CTable is a enumerator rowset
			if(pcColumns && GetModInfo()->GetRowsetQuery() != NO_QUERY)
				TESTC(GetQueryInfo(SELECT_ALLFROMTBL, pcColumns, prgColumns, NULL, NULL, NULL, NULL));
			break;
		}

		case USE_COLUMNSROWSET:
		{
			//IOpenRowset::OpenRowset
			if(!pIOpenRowset)
				pIOpenRowset = m_pIOpenRowset;

			//IOpenRowset::OpenRowset
			QTESTC_(hr = pIOpenRowset->OpenRowset(NULL, pTableID ? pTableID : &m_TableID, NULL, IID_IColumnsRowset,
													0, NULL, (IUnknown**)&pIColumnsRowset),S_OK);

			//Now obtain the ColumnsRowset
			QTESTC_(hr = pIColumnsRowset->GetColumnsRowset(NULL, 0, NULL, riid, cPropSets, rgPropSets, ppIRowset),S_OK);

			//Need to setup everything that ExecuteCommand does
			//ie: m_cRowsetCols, m_rgTableColOrds
			//NOTE: We only do this if CTable is a ColumnsRowset
			if(pcColumns && GetModInfo()->GetRowsetQuery() != NO_QUERY)
				TESTC(GetQueryInfo(SELECT_ALLFROMTBL, pcColumns, prgColumns, NULL, NULL, NULL, NULL));
			break;
		}
		
		default:
		{
			//ICommand::Execute
			QTESTC_(hr = ExecuteCommand(eQuery, riid, NULL, NULL, pcColumns, prgColumns, 
						EXECUTE_IFNOERROR, cPropSets, rgPropSets, NULL, ppIRowset, NULL),S_OK);
			break;
		}
	};

CLEANUP:
	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pISourcesRowset);
	SAFE_RELEASE(pIColumnsRowset);
	return hr;
}


//---------------------------------------------------------------------------
//	CTable::BuildCommand
//
//			CTable			|
//			BuildCommand	|
//
//	Client must release the following objects:
//		- If pICommand is not null user must release this
//	Client must free the following memory:
//		- If ppRowset is not null user must free this
//
//	This function takes a mandatory SQL statement.
//  This function will create a command object, set any properties passed in,
//   set the SQL statement, and execute the statement if fExecute = TRUE;
//  The user has the option of setting parameters, retrieving the count of
//   rowsets, retrieving the array of rowsets, retrieving the interface pointer
//   specified in the REFIID, and retrieving the command object.
//
//	@mfunc	BuildCommand
// 	@rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Problem
//
//---------------------------------------------------------------------------
HRESULT CTable::BuildCommand(
	LPCWSTR					pwszCommand,		// @parm [IN] SQL Statement to set
	REFIID					riid,				// @parm [IN] Interface pointer to return
	EEXECUTE				eExecute,			// @parm [IN] When to execute the SQL Statement
	ULONG 					cPropSets,			// @parm [IN] Count of property sets.
	DBPROPSET 				rgPropSets[],		// @parm [IN] Array of DBPROPSET structures.
	DBPARAMS * 				pParams,			// @parm [IN] Parameters to pass to ::Execute
	DBROWCOUNT *			pcRowsAffected,		// @parm [OUT] Pointer to memory in which to return the count
												// of rows affected by a command that updates, deletes,
												// or inserts rows.
	IUnknown **				ppRowset,			// @parm [IN/OUT] Pointer to the rowset pointer.  If caller doesn't
												// want an interface pointer back, they may pass NULL for this parameter. 
	ICommand **				ppICommand,			// @parm [IN/OUT] If ppICommand == NULL, this function uses
												// its own command object and then frees it, caller does nothing.
												// <nl>
												// If *ppICommand is not null, this value is used as the
												// ICommand interface for all command operations in this function. 
												// Caller maintains responsiblity to release this interface.
												// <nl>
												// If *ppICommand is null, this function creates a new command object
												// and places the ICommand interface to this object in *ppICommand.
												// Caller assumes responsibility to release this interface.
	IUnknown*				pIUnkOuter			// @parm [IN] Aggregate
)
{
	IUnknown	*			pRowset = NULL;
	HRESULT 				hr=0;	   					// Result code
	ICommand *				pICommand = NULL;			// ICommand object
	ICommandText *			pICommandText = NULL;		// ICommandText interface pointer
	ICommandProperties *	pICommandProperties = NULL;	// ICommandText interface pointer
	LPWSTR					pwszGetCmdText = NULL;		// Place to put existing text
	GUID					guidDialect = DBGUID_DEFAULT;// Dialect to get and set text as
														
	ASSERT(pwszCommand);

	// Init parameter
	if (ppRowset)
		*ppRowset = NULL;

	// Use the caller's command if they specified one
	if (ppICommand && *ppICommand)
		pICommand = *ppICommand;
	else
	{
		if (!GetCommandSupOnCTable())
			return E_FAIL;
		
		// Create a command object ourselves
		if (!CHECK(hr = m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,
				(IUnknown**)&pICommand),S_OK))
			return hr;
		
		// If user wants the command object back, set their memory to the interface we just got
		if (ppICommand)
			*ppICommand = pICommand;	
	}
	
	// Get an ICommandText object
	if (!VerifyInterface(pICommand, IID_ICommandText, COMMAND_INTERFACE, (IUnknown**)&pICommandText))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	// Check to see if we need to set new text
	hr = pICommandText->GetCommandText(&guidDialect, &pwszGetCmdText);

	//Only do SetCommandText if the intended text is different 
	//from what's set already, otherwise we'd get DB_E_OBJECTOPEN.
	if(hr==DB_E_NOCOMMAND || 
		pwszGetCmdText == NULL ||
		wcscmp(pwszGetCmdText, pwszCommand)!=0)
	{
		// Set the SQL statement.
		if (!CHECK(hr = pICommandText->SetCommandText(DBGUID_DBSQL, pwszCommand),S_OK))
			goto CLEANUP;
	}

	// Only SetProperties, if there are properties to set
	if (cPropSets)
	{
		if (!VerifyInterface(pICommand, IID_ICommandProperties, COMMAND_INTERFACE, (IUnknown**)&pICommandProperties))
		{
			hr = E_NOINTERFACE;
			goto CLEANUP;
		}

		// SetProperties may return DB_S/DB_E depending wieither 1 or all properties
		// were not settable.  Return if any property is not settable, since we
		// don't want to create a rowset without the required properties
		hr = pICommandProperties->SetProperties(cPropSets,rgPropSets);
		
		// If all the properties did not get set then we want to return.
		if (eExecute == EXECUTE_IFNOERROR && hr!=S_OK)
			goto CLEANUP;

		//Use CursorEngine ( if requested )
		if(GetModInfo()->UseServiceComponents() & SERVICECOMP_CURSORENGINE)
			hr = SetRowsetProperty(pICommand, DBPROPSET_ROWSET, DBPROP_CLIENTCURSOR, TRUE, DBPROPOPTIONS_REQUIRED, FALSE);
	}

	// Execute the SQL statement if eExecute 
	if (eExecute != EXECUTE_NEVER)
	{
		//Since the ICommand test is always directly pounding on 
		//ICommand::Execute, and since most consumers and apps will only use the 
		//ICommandText interface since they have to call SetCommandText and Execute,
		//and this allows the user to only have to obtain and keep 1 interface
		//arround, this will be the the common route of testing...
		
		// Execute the SQL Statement
		if(FAILED(hr = pICommandText->Execute(
				pIUnkOuter,			// [IN]		Aggregate
				riid,				// [IN]		REFIID
				pParams,			// [IN/OUT] DBPARAMS
				pcRowsAffected,		// [OUT]	Count of rows affected
				&pRowset)))			// [IN/OUT] Memory to put rowset interface
			goto CLEANUP;
  	}
			
	
CLEANUP:
	
	// Release this function's command pointer if the caller didn't want it back
	if (!ppICommand && pICommand)
		SAFE_RELEASE(pICommand);
		
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pICommandProperties);

	PROVIDER_FREE(pwszGetCmdText);

	// If user wanted rowset interface, pass it back
	if (ppRowset)
		*ppRowset = pRowset;
	else
		SAFE_RELEASE(pRowset);
	
	return hr;
}

//---------------------------------------------------------------------------
// CTable::CreateSQLStmt
//
// CTable			|
// CreateSQLStmt	|
//
// Client must free the following memory:
//		- ppwszStatement
//		- prgColumns
//
//	This function takes a one of the enum SQL Statements and creates the
//  SQL statement with the columns and the CTable.  The statement is then
//  passed back to the user along with an array of column numbers in the
//  order they will come back in the result set.
//
// @mfunc	CreateSQLStmt
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Problem
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateSQLStmt(
	EQUERY			eSQLStmt,		// @parm [IN]  SQL statement to create 
	WCHAR * 		pwszTableName2,	// @parm [IN]  Second TableName
	WCHAR **		ppwszStatement,	// @parm [OUT] Pointer to memory to place
									// address of generated SQL statement in.
									// This parameter must not be NULL.
	DBORDINAL *		pcColumns,		// @parm [IN\OUT] Pointer to memory to place
									// Count of columns in.  If this parameter is NULL, 
									// no count is returned.
	DB_LORDINAL **	prgColumns,		// @parm [IN\OUT] Pointer to memory to place address of
									// Array of column numbers in. User is responsible for
									// freeing this array, unless they pass this parameter as NULL.																		
	DBCOUNTITEM		ulRowNumber,	// @parm [IN]  Row number to use
	CTable *		pTable2,		// @parm [IN]  Second table object to use
	DBORDINAL		iOrdinal		// @parm [IN]  Col number to use, default 0 (all)
)
{
	HRESULT 	hr = S_OK; 			// Result code
	WCHAR *		pwszSQLText=NULL;	// SQL text to execute
	WCHAR *		pwszColList=NULL;	// List of column names
	WCHAR *		pwszParamColList = NULL;// List of column names with parameterized search criteria
	WCHAR *		pwszRevColList=NULL;// Reverse list of column names
	WCHAR *		pwszCol;			// Column name
	DBORDINAL	ulCountCol;			// Count of columns in the table
	WCHAR *		pwszRNum=NULL;		// Random Number
	DBORDINAL	cColumns = 0;
	DB_LORDINAL *	rgColumns = NULL;
	CCol		col;

	// Check Function Arguments
	ASSERT(ppwszStatement);
	// Not all statements require a table name.  Some have the name/sproc passed to the function.  Do not assert or fail for this
	// case.  For example, we assert here for the Drop Proc case if using a CTable object that we have not yet called CreateTable()
	// on.
//	ASSERT(GetTableName());   


	cColumns  = 0;
	rgColumns = NULL;

	//There is no reason we should be calling this function "CreateSQLStmt" if the provider
	//doesn't support commands...
	if(!GetCommandSupOnCTable())
	{
		hr = DB_E_NOTSUPPORTED;
		goto CLEANUP;
	}	
	
	// Switch on the enum passed in by user
	switch(eSQLStmt)
	{
		// "SELECT * FROM <tbl>"
		case USE_SUPPORTED_SELECT_ALLFROMTBL:
		case SELECT_ALLFROMTBL:
			// Create the column list and array of column numbers
			// Since the sql statement is "Select *" we can not guarantee
			// what the array of column numbers is correct
			
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			
			// Since * is being used the column list is not certain
			// Allocate memory for the sql statement. -2 is for %s
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_ALLFROMTBL) +	
							wcslen(GetTableName()) - 2)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_ALLFROMTBL, GetTableName());
			break;

		// "Select 'ABC', <col list> from <tbl>"
		case SELECT_ABCANDCOLLIST:
			// Create array of column numbers
			cColumns = m_ColList.GetCount() + 1;
			
			//Allocate our own memory so CreateColList doesn't do it for us,
			//adding room for ABC col
			rgColumns = (DB_LORDINAL *) PROVIDER_ALLOC(sizeof(DB_LORDINAL) * (cColumns));
			
			// -1 means that first column does not have column name or number
			(rgColumns)[0]=-1;
			
			// Create the column list and array of column numbers
			CreateColList(FORWARD, &pwszColList, &cColumns, &rgColumns);
			
			// Allocate memory for the sql statement. -4 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_ABCANDCOLLIST) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_ABCANDCOLLIST, pwszColList, GetTableName());
			break;

		// "Select DISTINCT <col list> from <tbl>"
		case SELECT_DISTINCTCOLLIST:
		{
			DBORDINAL iOrderByCol = 1;

			// Some providers require order by column appears in select list, so ensure that
			if (iOrdinal)
				iOrderByCol = iOrdinal;

			// Get column name of first column
			GetColInfo(iOrderByCol, col);
			pwszCol = col.GetColName();
			
			// Create the column list and array of column numbers
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns, ALL_COLS_IN_LIST,
				FALSE, FALSE, NULL, PRIMARY, 1, iOrdinal);
			
			// Allocate memory for the sql statement. -4 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_DISTINCTCOLLIST) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_DISTINCTCOLLIST, pwszColList, 
						GetTableName());
			break;
		}

		// "Select DISTINCT <col list> from <tbl> order by <col one> DESC"
		case SELECT_DISTINCTCOLLISTORDERBY:
		{
			DBORDINAL iOrderByCol = 1;

			// Some providers require order by column appears in select list, so ensure that
			if (iOrdinal)
				iOrderByCol = iOrdinal;

			// Get column name of first column
			GetColInfo(iOrderByCol, col);
			pwszCol = col.GetColName();
			
			// Create the column list and array of column numbers
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns, ALL_COLS_IN_LIST,
				FALSE, FALSE, NULL, PRIMARY, 1, iOrdinal);
			
			// Allocate memory for the sql statement. -6 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_DISTINCTCOLLISTORDERBY) +	
							wcslen(pwszColList) +	
							wcslen(pwszCol) +	
							wcslen(GetTableName()) - 6)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_DISTINCTCOLLISTORDERBY, pwszColList, 
						GetTableName(), pwszCol);
			break;
		}
		// "Select <reverse col list> from <tbl>"
		case SELECT_REVCOLLIST:
			CreateColList(REVERSE,&pwszRevColList, &cColumns, &rgColumns);
			
			// Allocate memory for the sql statement. -4 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_REVCOLLIST) +	
							wcslen(pwszRevColList) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_REVCOLLIST, pwszRevColList, GetTableName());
			break;

		// "Select <col one> from <tbl> GROUP BY <col one> HAVING <col one> is not null"
		case SELECT_COLLISTGROUPBY:
			// Get column one number and name
			GetColInfo(1, col);
			
			// This statement will only return
			cColumns = 1;
			rgColumns = (DB_LORDINAL *) PROVIDER_ALLOC(sizeof(DB_LORDINAL));
			(rgColumns)[0]=col.GetColNum();
			pwszCol = col.GetColName();
			
			// Allocate memory for the sql statement. -8 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTGROUPBY) +	
							wcslen(pwszCol) +	
							wcslen(pwszCol) +	
							wcslen(pwszCol) +	
							wcslen(GetTableName()) - 8)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTGROUPBY, pwszCol, 
						GetTableName(), pwszCol, pwszCol);
			break;

		case SELECT_INVALIDGROUPBY:
			// Get column one number and name
			GetColInfo(1, col);
			
			// This statement will only return
			cColumns = 1;
			rgColumns = (DB_LORDINAL *) PROVIDER_ALLOC(sizeof(DB_LORDINAL));
			(rgColumns)[0]=col.GetColNum();
			pwszCol = col.GetColName();
			
			// Allocate memory for the sql statement. -8 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_INVALIDGROUPBY) +	
							wcslen(pwszCol) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_INVALIDGROUPBY,	GetTableName(), pwszCol);
			break;

		// "Select <col list> from <tbl> where <last col> in (Select <last col> from <tbl>)"
		case SELECT_COLLISTWHERELASTCOLINSELECT:
			// Get information for last column in table
			ulCountCol = m_ColList.GetCount();
			GetColInfo(ulCountCol, col);
			pwszCol = col.GetColName();
			
			// Create the column list and array of column numbers
			// Allocate memory for the sql statement. -10 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTWHERELASTCOLINSELECT) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) +	
							wcslen(pwszCol) +	
							wcslen(pwszCol) +	
							wcslen(GetTableName()) - 10)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTWHERELASTCOLINSELECT, pwszColList, 
						GetTableName(), pwszCol, pwszCol, GetTableName());
			break;

		// "Select <reverse col list> from <view>"
		case SELECT_REVCOLLISTFROMVIEW:
			// Create the column list and array of column numbers
			CreateColList(REVERSE,&pwszRevColList, &cColumns, &rgColumns);

			// Create view if not already there
			if (!m_pwszViewName)
			{
				// Create view name based on table name
				m_pwszViewName = wcsDuplicate(GetTableName());
				wcsncpy(m_pwszViewName + (wcslen(m_pwszViewName) - 1), L"V", 1);

				// Allocate memory for the sql statement. -6 is for %s's
				pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
								(wcslen(wszCREATE_VIEW) +	
								wcslen(m_pwszViewName) +	
								wcslen(pwszRevColList) +	
								wcslen(GetTableName()) - 6)) +
								sizeof(WCHAR));

				// Format SQL Statement
				swprintf(pwszSQLText, wszCREATE_VIEW, m_pwszViewName, pwszRevColList,GetTableName());

				// Don't check the return code here because TableDump will create the view initially
				// and then when we run the test the view will exist, and create will fail.
				BuildCommand(pwszSQLText,	//SQL text to execute
								IID_NULL,		//This won't be used
								EXECUTE_ALWAYS,	//Should always execute fine
								0, NULL,		//Set no properties
								NULL,			//No parmeters									
								NULL, 			//No RowsAffected info
								NULL,		    //No rowset needs to be allocated
								&m_pICommand);	//Use this object's command to execute on				
			}

			// Allocate memory for the sql statement. -4 is for %s's
			PROVIDER_FREE(pwszSQLText);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_REVCOLLISTFROMVIEW) +	
							wcslen(pwszRevColList) +	
							wcslen(m_pwszViewName) - 4)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_REVCOLLISTFROMVIEW, pwszRevColList,
						m_pwszViewName);
			break;

		// "Select count(<col one>) from <tbl>"
		case SELECT_COUNT:
			// The result will not have a column name because it is the count
			// Create array of column numbers
			cColumns = 1;
			rgColumns = (DB_LORDINAL *)PROVIDER_ALLOC(sizeof(DB_LORDINAL) * (cColumns));
			rgColumns[0] = 1;

			GetColInfo(1, col);
			pwszCol = col.GetColName();

			// Allocate memory for the sql statement. -4 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COUNT) +	
							wcslen(pwszCol) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COUNT, pwszCol,	GetTableName());
			
			break;

		// "Select <col list> from <tbl>; Select <reverse col list> from <tbl>"
		case SELECT_COLLISTSELECTREVCOLLIST:
			// Create the column list and array of column numbers
			CreateColList(REVERSE,&pwszRevColList, &cColumns, &rgColumns);
			PROVIDER_FREE(rgColumns);

			// We want the forward column list since that is the first rowset
			// Allocate memory for the sql statement. -8 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTSELECTREVCOLLIST) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) +	
							wcslen(pwszRevColList) +	
							wcslen(GetTableName()) - 8)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTSELECTREVCOLLIST, pwszColList,
						GetTableName(), pwszRevColList, GetTableName());
			break;

		// "Select <col list> from <tbl> where 0=1"
		case SELECT_EMPTYROWSET:
			// Create the column list and array of column numbers
			// Allocate memory for the sql statement. -4 is for %s's
			TESTC_(hr = CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns), S_OK);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_EMPTYROWSET) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_EMPTYROWSET, pwszColList, GetTableName());
			break;

		// "Select <col list> from <tbl>"
		case SELECT_COLLISTFROMTBL:
			// Create the column list and array of column numbers
			// Allocate memory for the sql statement. -4 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTFROMTBL) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTFROMTBL, pwszColList, GetTableName());
			break;

		// "Select <col list> from <tbl> UNION select <col list> from <tbl>"
		case SELECT_COLLISTTBLUNIONTBL:
			// Create the column list and array of column numbers
			// Allocate memory for the sql statement. -8 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTTBLUNIONTBL) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) - 8)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTTBLUNIONTBL, pwszColList, 
						GetTableName(), pwszColList, GetTableName());
			break;

		// "Select <col list> from <tbl> ORDER BY <col one> COMPUTE SUM(<col one>) by <col one>"
		case SELECT_COLLISTORDERBYCOLONECOMPUTEBY:
			// Get column one name
			GetColInfo(1, col);
			pwszCol = col.GetColName();

			// Create the column list and array of column numbers
			// Allocate memory for the sql statement. -10 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTORDERBYCOLONECOMPUTEBY) +	
							wcslen(pwszColList) +	
							wcslen(pwszCol) +	
							wcslen(pwszCol) +	
							wcslen(pwszCol) +	
							wcslen(GetTableName()) - 10)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTORDERBYCOLONECOMPUTEBY, pwszColList, 
						GetTableName(), pwszCol, pwszCol, pwszCol);
			break;

		case SELECT_COLLISTORDERBYCOLONECOMPUTE:
			// Get column one name
			GetColInfo(1, col);
			pwszCol = col.GetColName();

			// Create the column list and array of column numbers
			// Allocate memory for the sql statement. -10 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTORDERBYCOLONECOMPUTE) +	
							wcslen(pwszColList) +	
							wcslen(pwszCol) +	
							wcslen(pwszCol) +	
							wcslen(GetTableName()) - 8)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTORDERBYCOLONECOMPUTE, pwszColList, 
						GetTableName(), pwszCol, pwszCol);
			break;

 		// "Select * from  <tbl1>, <tbl2>"
		case SELECT_CROSSPRODUCT:
			// We need a second table object to create this statement
			ASSERT(pwszTableName2);

			// Create the column list and array of column numbers
			// Since the sql statement is "Select *" we can not guarantee
			//  what the array of column numbers is correct
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);

			// Since * is being used the column list is not certain
			// Allocate memory for the sql statement. -2 is for %s
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_ALLFROMTBL) +	
							wcslen(GetTableName()) - 2)) +
							wcslen(pwszTableName2 - 2) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_ALLFROMTBL, GetTableName(),
														 wszCOMMA, pwszTableName2);
			break;

		// "Select * from { oj <tbl1> LEFT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>}"
		case SELECT_LEFTOUTERJOIN_ESC:
		// "Select * from { oj <tbl1> RIGHT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>}"
		case SELECT_RIGHTOUTERJOIN_ESC:		
		// "Select * from <tbl1> LEFT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>"
		case SELECT_LEFTOUTERJOIN:		
		// "Select * from <tbl1> RIGHT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>"
		case SELECT_RIGHTOUTERJOIN:
		{
			CCol col2;
			WCHAR * pwszCol2 = NULL;

			// We need a second table object to create this statement
			ASSERT(pwszTableName2);

			// Create the column list and array of column numbers
			// Since the sql statement is "Select *" we can not guarantee
			//  what the array of column numbers is correct
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			GetColInfo(1, col);
			pwszCol = col.GetColName();

			pwszCol2 = pwszCol;

			// If a second table object was passed in get the second column name
			// from that, since it may not have the same name.
			if (pTable2)
			{
				pTable2->GetColInfo(1, col2);
				pwszCol2 = col2.GetColName();
			}

			switch(eSQLStmt)
			{
				case SELECT_LEFTOUTERJOIN_ESC:
					// We need a second table object to create this statement
					ASSERT(pwszTableName2);

					// Allocate memory for the sql statement. -12 is for %s's
					pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
									(wcslen(wszSELECT_LEFTOUTERJOIN_ESC) +	
									wcslen(GetTableName()) +	
									wcslen(pwszTableName2) +	
									wcslen(GetTableName()) +	
									wcslen(pwszCol) +	
									wcslen(pwszCol2) +	
									wcslen(pwszTableName2) - 12)) +
									sizeof(WCHAR));
					// Format SQL Statement
					swprintf(pwszSQLText, wszSELECT_LEFTOUTERJOIN_ESC, GetTableName(),
								pwszTableName2, GetTableName(), pwszCol, pwszTableName2, pwszCol2);
					break;
				case SELECT_RIGHTOUTERJOIN_ESC:		
					// We need a second table object to create this statement
					ASSERT(pwszTableName2);

					// Allocate memory for the sql statement. -12 is for %s's
					pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
									(wcslen(wszSELECT_RIGHTOUTERJOIN_ESC) +	
									wcslen(GetTableName()) +	
									wcslen(pwszTableName2) +	
									wcslen(GetTableName()) +	
									wcslen(pwszCol) +	
									wcslen(pwszCol2) +	
									wcslen(pwszTableName2) - 12)) +
									sizeof(WCHAR));
					// Format SQL Statement
					swprintf(pwszSQLText, wszSELECT_RIGHTOUTERJOIN_ESC, GetTableName(),
								pwszTableName2, GetTableName(), pwszCol, pwszTableName2, pwszCol2);
					break;
				case SELECT_LEFTOUTERJOIN:		
					// We need a second table object to create this statement
					ASSERT(pwszTableName2);

					// Allocate memory for the sql statement. -12 is for %s's
					pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
									(wcslen(wszSELECT_LEFTOUTERJOIN) +	
									wcslen(GetTableName()) +	
									wcslen(pwszTableName2) +	
									wcslen(GetTableName()) +	
									wcslen(pwszCol) +	
									wcslen(pwszCol2) +	
									wcslen(pwszTableName2) - 12)) +
									sizeof(WCHAR));
					// Format SQL Statement
					swprintf(pwszSQLText, wszSELECT_LEFTOUTERJOIN, GetTableName(),
								pwszTableName2, GetTableName(), pwszCol, pwszTableName2, pwszCol2);
					break;
				case SELECT_RIGHTOUTERJOIN:
					// We need a second table object to create this statement
					ASSERT(pwszTableName2);

					// Allocate memory for the sql statement. -12 is for %s's
					pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
									(wcslen(wszSELECT_RIGHTOUTERJOIN) +	
									wcslen(GetTableName()) +	
									wcslen(pwszTableName2) +	
									wcslen(GetTableName()) +	
									wcslen(pwszCol) +	
									wcslen(pwszCol2) +	
									wcslen(pwszTableName2) - 12)) +
									sizeof(WCHAR));
					// Format SQL Statement
					swprintf(pwszSQLText, wszSELECT_RIGHTOUTERJOIN, GetTableName(),
								pwszTableName2, GetTableName(), pwszCol, pwszTableName2, pwszCol2);
					break;
			}
			break;
		}
		// "Select <col list> from <tbl> where <parm col list>"
		case SELECT_FROMTBLWITHPARAMS:		
		{
			DBORDINAL	cSearchableCols = 0;
			DB_LORDINAL * rgSearchableCols = NULL;
			
			// Create the column list and array of column numbers,
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns, ALL_COLS_IN_LIST, FALSE);
			PROVIDER_FREE(rgColumns);
			
			// Create the column list with parameterized search criteria			
			CreateColList(FORWARD,&pwszParamColList, &cSearchableCols, &rgSearchableCols, SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST, TRUE);
			
			//We must have at least one searchable column
			if(cSearchableCols)
			{
				// Allocate memory for the sql statement. -6 is for %s's
				pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTFROMTBLWHERE) +	
							wcslen(pwszColList) +	
							wcslen(pwszParamColList) +	
							wcslen(GetTableName()) - 6)) +
							sizeof(WCHAR));

				// Format SQL Statement
				swprintf(pwszSQLText, wszSELECT_COLLISTFROMTBLWHERE, pwszColList, GetTableName(), pwszParamColList);

				SAFE_FREE(rgSearchableCols);
			}
		
			break;
		}
		// "Select <out param list> from <tbl> where <parm col list>"
		case SELECT_ALL_BYINDEX_WITHPARAMS:		
		{
			DBORDINAL	cOutParams = 0;
			DB_LORDINAL * rgOutParamCols = NULL;
			LPWSTR	pwszOutParamList = NULL;
			DBORDINAL cSearchParams = 0;
			DB_LORDINAL * rgSearchParamCols = NULL;
			LPWSTR pwszSearchParamList = NULL;
			
			// Create the out param list and array of associated column numbers,
			TESTC_(hr = CreateList(LT_PARAM_OUT,&pwszOutParamList, &cOutParams, &rgOutParamCols, 
				ALL_COLS_IN_LIST, FORWARD), S_OK);
			
			// Create the search param list with parameterized search criteria			
			//We must have at least one searchable column
			if (CHECK(hr = CreateList(LT_PARAM_SEARCH,&pwszSearchParamList, &cSearchParams, &rgSearchParamCols, 
				INDEX_COL_IN_LIST, FORWARD), S_OK) && cSearchParams)
			{
				// Allocate memory for the sql statement. -6 is for %s's
				pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTFROMTBLWHERE) +	
							wcslen(pwszOutParamList) +	
							wcslen(pwszSearchParamList) +	
							wcslen(GetTableName()) - 6)) +
							sizeof(WCHAR));

				// Format SQL Statement
				swprintf(pwszSQLText, wszSELECT_COLLISTFROMTBLWHERE, pwszOutParamList, GetTableName(), pwszSearchParamList);

				// Create combined list of columns
				cColumns = cOutParams + cSearchParams;
				SAFE_ALLOC(rgColumns, DB_LORDINAL, cColumns);
				memcpy(rgColumns, rgOutParamCols, sizeof(DB_LORDINAL)*cOutParams);
				memcpy(rgColumns+cOutParams, rgSearchParamCols, sizeof(DB_LORDINAL)*cSearchParams);

			}

			SAFE_FREE(rgOutParamCols);
			SAFE_FREE(rgSearchParamCols);
			SAFE_FREE(pwszOutParamList);
			SAFE_FREE(pwszSearchParamList);
		
			break;
		}

		// "create procedure <name>(<rpc out param def list>) as begin select <rpc out param list> from <tbl> where <index param list>"
		case RPC_SELECT_ALL_BYINDEX_WITHPARAMS:		
		{
			DBORDINAL	cOutParamDef = 0;
			DB_LORDINAL * rgOutParamDef = NULL;
			LPWSTR pwszOutParamDef = NULL;
			DBORDINAL cInParamDef = 0;
			DB_LORDINAL * rgInParamDef = NULL;
			LPWSTR pwszInParamDef = NULL;
			LPWSTR pwszParamDef = NULL;
			DBORDINAL	cOutParams = 0;
			DB_LORDINAL * rgOutParamCols = NULL;
			LPWSTR	pwszOutParamList = NULL;
			DBORDINAL cSearchParams = 0;
			DB_LORDINAL * rgSearchParamCols = NULL;
			LPWSTR pwszSearchParamList = NULL;
			
			// Create the out param definition list and array of associated column numbers,
			TESTC_(hr = CreateList(LT_RPC_OUT_PARAM_DEF, &pwszOutParamDef, &cOutParamDef, &rgOutParamDef, 
				ALL_COLS_IN_LIST, FORWARD), S_OK);

			if (
			// Create the in param definition list and array of associated column numbers,
			CHECK(hr = CreateList(LT_RPC_PARAM_DEF, &pwszInParamDef, &cInParamDef, &rgInParamDef, 
				INDEX_COL_IN_LIST, FORWARD), S_OK) &&
			// Create the out param list and array of associated column numbers,
			CHECK(hr = CreateList(LT_RPC_PARAM_OUT, &pwszOutParamList, &cOutParams, &rgOutParamCols, 
				ALL_COLS_IN_LIST, FORWARD), S_OK) &&
			// Create the search param list with parameterized search criteria			
			CHECK(hr = CreateList(LT_RPC_PARAM_SEARCH,&pwszSearchParamList, &cSearchParams, &rgSearchParamCols, 
				INDEX_COL_IN_LIST, FORWARD), S_OK) && 
			//We must have at least one searchable column
				cSearchParams)
			{
				ULONG ulChars;
				LPWSTR pwszNameFmt = L"sp%s";
				LPWSTR pwszProcName = pwszTableName2;
				LPWSTR pwszSelect = NULL;

				// Create the combined parameter definition list
				SAFE_ALLOC(pwszParamDef, WCHAR, wcslen(pwszOutParamDef)+ wcslen(pwszInParamDef)+2); // "," and NULL terminator
				wcscpy(pwszParamDef, pwszOutParamDef);
				wcscat(pwszParamDef, L",");
				wcscat(pwszParamDef, pwszInParamDef);

				SAFE_FREE(pwszOutParamDef);
				SAFE_FREE(pwszInParamDef);

				ulChars = _scwprintf(wszSELECT_1ROW, pwszOutParamList, GetTableName(), pwszSearchParamList)+1;
				SAFE_ALLOC(pwszSelect, WCHAR, ulChars);
				swprintf(pwszSelect, wszSELECT_1ROW, pwszOutParamList, GetTableName(), pwszSearchParamList);

				if (!pwszProcName)
				{
					ulChars = _scwprintf(pwszNameFmt, GetTableName())+1;
					SAFE_ALLOC(pwszProcName, WCHAR, ulChars);
					swprintf(pwszProcName, pwszNameFmt, GetTableName());
				}

				ulChars = _scwprintf(wszCREATE_PROC_TEMPLATE1, pwszProcName, pwszParamDef, pwszSelect) + 1;
				SAFE_ALLOC(pwszSQLText, WCHAR, ulChars);

				// Format SQL Statement
				swprintf(pwszSQLText, wszCREATE_PROC_TEMPLATE1, pwszProcName, pwszParamDef, pwszSelect);
				SAFE_FREE(pwszSelect);

				// Create combined list of columns
				cColumns = cOutParams + cSearchParams;
				SAFE_ALLOC(rgColumns, DB_LORDINAL, cColumns);
				memcpy(rgColumns, rgOutParamCols, sizeof(DB_LORDINAL)*cOutParams);
				memcpy(rgColumns+cOutParams, rgSearchParamCols, sizeof(DB_LORDINAL)*cSearchParams);

			}

			SAFE_FREE(pwszParamDef);
			SAFE_FREE(pwszOutParamDef);
			SAFE_FREE(rgOutParamDef);
			SAFE_FREE(pwszInParamDef);
			SAFE_FREE(rgInParamDef);
			SAFE_FREE(pwszOutParamList);
			SAFE_FREE(rgOutParamCols);
			SAFE_FREE(pwszSearchParamList);
			SAFE_FREE(rgSearchParamCols);
			SAFE_FREE(rgOutParamCols);
			SAFE_FREE(rgSearchParamCols);
		
			break;
		}
		// "{call <sprocname>(?, ?, ...)}"
		case CALL_RPC:		
		{
			LPWSTR pwszNameFmt = L"sp%s";
			LPWSTR pwszProcName = pwszTableName2;
			size_t ulChars;
			DBORDINAL iParam;
			LPWSTR pwszParams = NULL;

			// Requires count of parameters to be passed in cColumns
			TESTC(pcColumns != NULL);

			if (!pwszProcName)
			{
				ulChars = _scwprintf(pwszNameFmt, GetTableName())+1;
				SAFE_ALLOC(pwszProcName, WCHAR, ulChars);
				swprintf(pwszProcName, pwszNameFmt, GetTableName());
			}

			ulChars = wcslen(pwszProcName) + *pcColumns*wcslen(wszQuestionMarkComma)+3;  // 1 NULL term, and 2 parens

			pwszParams = (LPWSTR)PROVIDER_ALLOC(sizeof(WCHAR) * ulChars);

			if (pwszParams)
				wcscpy(pwszParams, pwszProcName);
			else
			{
				SAFE_FREE(pwszProcName);
				goto CLEANUP;
			}

			if (*pcColumns)
			{
				cColumns = *pcColumns;
				wcscat(pwszParams, L"(");
				for (iParam = 0; iParam < *pcColumns; iParam++)
					wcscat(pwszParams, wszQuestionMarkComma);

				// Replace trailing comma with closing paren
				pwszParams[wcslen(pwszParams)-1] = L')';
			}
			if (prgColumns)
				rgColumns = *prgColumns;

			ulChars = _scwprintf(wszEXEC_PROC, pwszParams)+1;

			SAFE_ALLOC(pwszSQLText, WCHAR, ulChars);

			// Format SQL Statement
			swprintf(pwszSQLText, wszEXEC_PROC, pwszParams);

			// Free the proc name if we allocated it.  Do not free user's proc name passed in pwszTableName2.
			if (!pwszTableName2)
				SAFE_FREE(pwszProcName);
			SAFE_FREE(pwszParams);
			break;
		}		
		// "Select <col list> from <tbl> where <searchable and updateable literal col list>"
		case SELECT_ROW_WITH_LITERALS:		
		{
			DBORDINAL cCols2=0;
			DB_LORDINAL * rgCols2 = NULL;

			// Create the column list and array of column numbers using all columns
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns, ALL_COLS_IN_LIST/*UPDATEABLE_COLS_IN_LIST*/, FALSE);

			// Create the search criteria using literals
			CreateColList(FORWARD,&pwszParamColList, &cCols2, &rgCols2, 
				SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST, FALSE, TRUE, NULL, PRIMARY, ulRowNumber);
			PROVIDER_FREE(rgCols2);	

			//We must have at least one searchable column
			if(cCols2)
			{
				// Allocate memory for the sql statement. -6 is for %s's
				pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
								(wcslen(wszSELECT_COLLISTFROMTBLWHERE) +	
								wcslen(pwszColList) +	
								wcslen(pwszParamColList) +	
								wcslen(GetTableName()) - 6)) +
								sizeof(WCHAR));

				// Format SQL Statement
				swprintf(pwszSQLText, wszSELECT_COLLISTFROMTBLWHERE, pwszColList, GetTableName(), pwszParamColList);
			}
			break;
		}
		// "Select <col list> from <tbl> where <searchable and updateable parm col list>"
		case SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE:		
			// Create the column list and array of column numbers,
			//asking for only columns that are searchable
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns, SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST, FALSE);
			PROVIDER_FREE(rgColumns);

			// Create the column list with parameterized search criteria			
			CreateColList(FORWARD,&pwszParamColList, &cColumns, &rgColumns, 
				SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST, TRUE);

			//We must have at least one searchable column
			if(cColumns)
			{
				// Allocate memory for the sql statement. -6 is for %s's
				pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTFROMTBLWHERE) +	
							wcslen(pwszColList) +	
							wcslen(pwszParamColList) +	
							wcslen(GetTableName()) - 6)) +
							sizeof(WCHAR));

				// Format SQL Statement
				swprintf(pwszSQLText, wszSELECT_COLLISTFROMTBLWHERE, pwszColList, GetTableName(), pwszParamColList);
			}
			break;
		// "Delete from <tbl> where <searchable and updateable parm col list>"
		case DELETE_ALLWITHPARAMS:		
			
			// Create the column list with parameterized search criteria			
			CreateColList(FORWARD,&pwszParamColList, &cColumns, &rgColumns, 
				SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST, TRUE);
			SAFE_FREE(rgColumns);
			
			//We must have at least one searchable column
			if(cColumns)
			{
				// Allocate memory for the sql statement. -6 is for %s's
				pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszDELETE_1ROW) +	
							wcslen(pwszParamColList) +	
							wcslen(GetTableName()))) +
							sizeof(WCHAR));
				
				
				// Format SQL Statement
				swprintf(pwszSQLText, wszDELETE_1ROW, GetTableName(), pwszParamColList);
				
			}
			break;

		// "insert into table (<col list>) values (<
		case INSERT_1ROW:
			//Delegate to our insert function
			hr = Insert(ulRowNumber, PRIMARY, FALSE/*fExecute*/, &pwszSQLText, FALSE/*fNULL*/, 1/*cRowsToInsert*/);
			break;

		// "Insert into <tbl>(<col list>) values(value1, valu2, ...)
		case INSERT_ROW_WITH_LITERALS:
		{
			WCHAR * wszQuestionMarks = NULL;
			LPWSTR pwszLiterals = NULL;

			// Create the column list asking for only columns that are updateable
			TESTC_(hr = CreateList(LT_COLNAME,&pwszColList, &cColumns, &rgColumns, 
				UPDATEABLE_COLS_IN_LIST, FORWARD), S_OK);

			// Create a list of literals for each updatable column
			TESTC_(hr = CreateList(LT_LITERAL,&pwszLiterals, NULL, NULL, 
				UPDATEABLE_COLS_IN_LIST, FORWARD, PRIMARY, ulRowNumber), S_OK);

			//We must have at least one updateable column
			if(cColumns)		
			{
				PROVIDER_FREE(rgColumns);
			
				// Allocate memory for the sql statement. -6 is for %s's
				SAFE_ALLOC(pwszSQLText, WCHAR, wcslen(wszINSERT_1ROW) +
								wcslen(pwszColList) +	
								wcslen(pwszLiterals) +	
								wcslen(GetTableName()) - 6 +
								sizeof(WCHAR));

				// Format SQL Statement
				swprintf(pwszSQLText, wszINSERT_1ROW, GetTableName(), 
					pwszColList, pwszLiterals);

				// Free the memory.
				PROVIDER_FREE(pwszLiterals);
			}
			break;
		}

		// "Insert into <tbl>(<col list>) values(<?,?...?>)
		case INSERT_ALLWITHPARAMS:		
		{
			WCHAR * wszQuestionMarks = NULL;

			// Create the column list and array of column numbers,
			//asking for only columns that are updateable
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns, 
				UPDATEABLE_COLS_IN_LIST, FALSE);
	
			//We must have at least one updateable column
			if(cColumns)		
			{
				PROVIDER_FREE(rgColumns);
			
				//Allocate memory for our ?,?,?... string
				wszQuestionMarks = (LPWSTR)PROVIDER_ALLOC(cColumns * 
					((wcslen(wszQuestionMarkComma) * sizeof (WCHAR)) + sizeof(WCHAR)));
		
				//Build the ?,?,?... 
				wcscpy(wszQuestionMarks, L"?,");
				for (DBORDINAL i=1; i<cColumns; i++)
					wcscat(wszQuestionMarks, wszQuestionMarkComma);

				// Remove the last comma
				// Allocate memory for the sql statement. -6 is for %s's
				wcsncpy(wszQuestionMarks + (wcslen(wszQuestionMarks) - 1), L"\0", 1);
				pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
								(wcslen(wszINSERT_ALLWITHPARAMS) +	
								wcslen(pwszColList) +	
								wcslen(wszQuestionMarks) +	
								wcslen(GetTableName()) - 6)) +
								sizeof(WCHAR));

				// Format SQL Statement
				swprintf(pwszSQLText, wszINSERT_ALLWITHPARAMS, GetTableName(), 
					pwszColList, wszQuestionMarks);

				// Free the memory.
				PROVIDER_FREE(wszQuestionMarks);
			}
			break;
		}

		// "Insert INOT <tbl>(<col list>) values(<?,?...?>)
		case INSERT_INVALID_KEYWORD:		
		{
			WCHAR * wszQuestionMarks = NULL;
			LPWSTR pwszLiterals = NULL;

			// Create the column list asking for only columns that are updateable
			TESTC_(hr = CreateList(LT_COLNAME,&pwszColList, &cColumns, &rgColumns, 
				UPDATEABLE_COLS_IN_LIST, FORWARD), S_OK);

			// Create a list of literals for each updatable column
			TESTC_(hr = CreateList(LT_LITERAL,&pwszLiterals, NULL, NULL, 
				UPDATEABLE_COLS_IN_LIST, FORWARD), S_OK);

			//We must have at least one updateable column
			if(cColumns)		
			{
				PROVIDER_FREE(rgColumns);
			
				// Allocate memory for the sql statement. -6 is for %s's
				SAFE_ALLOC(pwszSQLText, WCHAR, wcslen(wszINSERT_INVALID_KEYWORD) +
								wcslen(pwszColList) +	
								wcslen(pwszLiterals) +	
								wcslen(GetTableName()) - 6 +
								sizeof(WCHAR));

				// Format SQL Statement
				swprintf(pwszSQLText, wszINSERT_INVALID_KEYWORD, GetTableName(), 
					pwszColList, pwszLiterals);

				// Free the memory.
				PROVIDER_FREE(pwszLiterals);
			}
			break;
		}

		// "Select oldname as newname,oldname2 as newname2... from <tbl>"
		// need to rebuild column list by rename each column 
		case SELECT_CHANGECOLNAME:
		{
			WCHAR * pwszOldList=NULL;
			WCHAR * pwszNewList=NULL;
			size_t cOldList=0;
			size_t cNewList=0;
			DBORDINAL iColumns=0;
			ULONG iFirstNew=0;
			ULONG iLastNew=0;
			ULONG iFirstOld=0;
			ULONG iLastOld=0;
			ULONG iCurrentCharOld=0;
			ULONG iCurrentCharNew=0;
			ULONG iChar=0;

			// Create the column list and array of column numbers
			// Since the sql statement is "Select *" we can not guarantee
			// what the array of column numbers is correct
			CreateColList(FORWARD,&pwszOldList, &cColumns, &rgColumns);

			cOldList = wcslen(pwszOldList) *sizeof(WCHAR);
			cOldList += sizeof(WCHAR);
			cNewList = 3 * cOldList;

			// Get memory for new col list
			pwszNewList = (WCHAR *) PROVIDER_ALLOC(cNewList);
			memset(pwszNewList,GARBAGE,cNewList);

			// For each column, copy new name, copy assignment operator, copy oldname
			// If not last column, copy comma
			for(iColumns=0;iColumns<cColumns;iColumns++)
			{
				iFirstOld = iCurrentCharOld;
			
				// Copy oldname in
				while((pwszOldList[iCurrentCharOld]!=',') && (pwszOldList[iCurrentCharOld]!='\0'))
				{
					iLastOld = iCurrentCharOld;
					pwszNewList[iCurrentCharNew++]=pwszOldList[iCurrentCharOld++];
				}

				// So now iCurrentChar
				pwszNewList[iCurrentCharNew++]=L' ';
				pwszNewList[iCurrentCharNew++]=L'A';
				pwszNewList[iCurrentCharNew++]=L'S';
				pwszNewList[iCurrentCharNew++]=L' ';
			
				iCurrentCharOld = iFirstOld;

				// Copy in new name
				for(iChar=0;iChar<((iLastOld-iFirstOld)+1);iChar++)
					pwszNewList[iCurrentCharNew++]=pwszOldList[iCurrentCharOld++];

				pwszNewList[iCurrentCharNew++]=L'X';
				pwszNewList[iCurrentCharNew++]=L',';
				iCurrentCharOld++;
			}

			pwszNewList[--iCurrentCharNew]=L'\0';

			// Allocate memory for the sql statement. -4 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTFROMTBL) +	
							wcslen(pwszNewList) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTFROMTBL, pwszNewList, GetTableName());
			PRVTRACE(L"%s\n",pwszSQLText);

			PROVIDER_FREE(pwszNewList);
			PROVIDER_FREE(pwszOldList);
			}
			break;

		// "Select <col list>,<col list>,<col list> from <tbl>"
		case SELECT_MAXCOLINQUERY:
			//TODO: Need to find out max columns in query 
			//For now just use the current column...
			
			// Create the column list and array of column numbers
			// Allocate memory for the sql statement. -4 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTFROMTBL) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTFROMTBL, pwszColList, GetTableName());
			break;

		// "Select <col list> , <col list> from tbl"
		case SELECT_DUPLICATECOLUMNS:
		{
			// Create the column list and array of column numbers
			DB_LORDINAL * rgTmpColumns = NULL;
			CreateColList(FORWARD, &pwszColList, &cColumns, &rgTmpColumns);
			cColumns += cColumns;
			rgColumns = (DB_LORDINAL *)PROVIDER_ALLOC(sizeof(DB_LORDINAL) * cColumns);
			for(DBORDINAL i=0; i<cColumns; i++)
				rgColumns[i] = rgTmpColumns[i%(cColumns/2)];
			PROVIDER_FREE(rgTmpColumns);

			// Allocate memory for the sql statement. -4 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_DUPLICATECOLUMNS) +	
							wcslen(pwszColList) +	
							wcslen(pwszColList) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));
	
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_DUPLICATECOLUMNS, pwszColList,pwszColList, GetTableName());
			break;
		}
		// "Select <reverse col list> , <reverse col list> from <tbl>
		case SELECT_REVERSEDUPLICATECOLUMNS:
		{
			// Create the column list and array of column numbers
			DB_LORDINAL * rgTmpColumns = NULL;
			CreateColList(REVERSE, &pwszRevColList, &cColumns, &rgTmpColumns);
			cColumns += cColumns;
			rgColumns = (DB_LORDINAL *)PROVIDER_ALLOC(sizeof(DB_LORDINAL) * cColumns);
			for(DBORDINAL i=0; i<cColumns; i++)
				rgColumns[i] = rgTmpColumns[i%(cColumns/2)];
			PROVIDER_FREE(rgTmpColumns);

			// Allocate memory for the sql statement. -4 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_DUPLICATECOLUMNS) +	
							wcslen(pwszRevColList) +	
							wcslen(pwszRevColList) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_DUPLICATECOLUMNS, pwszRevColList,pwszRevColList, GetTableName());
			break;
		}
		// "Select colx, colx+colx from <tbl>
		case SELECT_COMPUTEDCOLLIST:
		{
			DB_LORDINAL	*rgColumnsII	= NULL;
			DBORDINAL	cColumnsII		= 0;
			DBORDINAL	i				= 0;
			CCol		tempCol;

			CreateColList(FORWARD,&pwszColList, &cColumnsII, &rgColumnsII, UPDATEABLE_COLS_IN_LIST);

			// Find the numeric col
			if(FAILED(GetFirstNumericCol(&tempCol)))
				return E_FAIL;						

			// Set up array of ordinals, put the updatable columns
			// ordinal in  them, the last column in 
			// the result set is computed from the first numeric column
			rgColumns = (DB_LORDINAL *)PROVIDER_ALLOC((sizeof(DB_LORDINAL) * (1+cColumnsII)));
			cColumns=1+cColumnsII;

			for(i=0;i<cColumnsII;i++)
			{
				rgColumns[i] = rgColumnsII[i];
			}
			rgColumns[i] = tempCol.GetColNum();
			PROVIDER_FREE(rgColumnsII);

			// Allocate memory for the sql statement. -8 is for %s's
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COMPUTEDCOLLIST) +	
							(wcslen(tempCol.GetColName())*2) +	
							wcslen(pwszColList) +
							wcslen(GetTableName()) - 8)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COMPUTEDCOLLIST, pwszColList,
				tempCol.GetColName(), tempCol.GetColName(), GetTableName());
		}
			break;
		//"Select * from <tbl> Order By colx" where colx is first numeric col for providers that 
		//return in the order in an undeterministic manner
		case SELECT_VALIDATIONORDER:
		{
			// Find the numeric col
			CCol tempCol;
			if(FAILED(GetOrderByCol(&tempCol)))
				return E_FAIL;						

			// Create the array of column numbers, ignore the column list
			// since we don't need it with our "select *"
			// Allocate memory for the sql statement. -4 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_VALIDATIONORDER) +	
							(wcslen(tempCol.GetColName())) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_VALIDATIONORDER, GetTableName(), 
				tempCol.GetColName());
		}
			break;
		// "Select * from <tbl> Order By colx" where colx is first numeric col
		case SELECT_ORDERBYNUMERIC:
		{
			// Find the numeric col
			CCol tempCol;
			if(FAILED(GetOrderByCol(&tempCol)))
				return E_FAIL;						

			// Create the array of column numbers, ignore the column list
			// since we don't need it with our "select *"
			// Allocate memory for the sql statement. -4 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_ORDERBYNUMERIC) +	
							(wcslen(tempCol.GetColName())) +	
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_ORDERBYNUMERIC, GetTableName(), 
				tempCol.GetColName());
		}
			break;

		case SELECT_UPDATEABLEALLROWS:
		{					
			// Create the updateable column list and array of column numbers
			// Allocate memory for the sql statement. -4 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns, UPDATEABLE_COLS_IN_LIST);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_UPDATEABLEALLROWS) +	
							wcslen(pwszColList) +								
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_UPDATEABLEALLROWS, pwszColList, GetTableName());
		}
			break;

			// Uses command to execute "SELECT <searchable col list> from table
			case SELECT_SEARCHABLE:
		{		   
			// Create the updateable column list and array of column numbers
			// Allocate memory for the sql statement. -4 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns, SEARCHABLE_COLS_IN_LIST);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTFROMTBL) +	
							wcslen(pwszColList) +								
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));

			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTFROMTBL, pwszColList, GetTableName());
		}
			break;

		case SELECT_UPDATEABLE:	//Uses command to execute "SELECT <updateable col list> from table
		{
			// Create the updateable column list and array of column numbers
			// Allocate memory for the sql statement. -4 is for %s's
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns, UPDATEABLE_COLS_IN_LIST);
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_COLLISTFROMTBL) +	
							wcslen(pwszColList) +								
							wcslen(GetTableName()) - 4)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_COLLISTFROMTBL, pwszColList, GetTableName());
		}
			break;

		case SELECT_ALL_WITH_FOR_BROWSE:
		{
			// Create the column list and array of column numbers
			// Since the sql statement is "Select *" we can not guarantee
			//  what the array of column numbers is correct
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			
			// Since * is being used the column list is not certain
			// Allocate memory for the sql statement. -2 is for %s
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_ALL_WITH_FOR_BROWSE) +	
							wcslen(GetTableName()) - 2)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_ALL_WITH_FOR_BROWSE, GetTableName());
		}
			break;

		case SELECT_ALL_WITH_FOR_UPDATE:
		{
			// Create the column list and array of column numbers
			// Since the sql statement is "Select *" we can not guarantee
			//  what the array of column numbers is correct
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns);
			
			// Since * is being used the column list is not certain
			// Allocate memory for the sql statement. -2 is for %s
			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszSELECT_ALL_WITH_FOR_UPDATE) +	
							wcslen(GetTableName()) - 2)) +
							sizeof(WCHAR));
			
			// Format SQL Statement
			swprintf(pwszSQLText, wszSELECT_ALL_WITH_FOR_UPDATE, GetTableName());
		}
			break;

		case SELECT_ALL_WITH_BLOB_AT_END:
		{
			POSITION	pos;
			WCHAR *		pwszBLOBColName=NULL;
			DBORDINAL	cBLOBColName=0;
			DBORDINAL	cColumnOrdx=0;
			BOOL		fSucceed=TRUE;
			DB_LORDINAL	rgBLOBOrdinal[100];

			// Temp array of string for column names
			pwszBLOBColName = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR *) * 
							m_ColList.GetCount() * 2000) + sizeof(WCHAR));

			cColumns=m_ColList.GetCount();
			rgColumns=(DB_LORDINAL *) PROVIDER_ALLOC(sizeof(DB_LORDINAL) * m_ColList.GetCount());

			pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							m_ColList.GetCount() * 2000) + sizeof(WCHAR));

			pwszBLOBColName[0]=L'\0';
			pwszSQLText[0]=L'\0';

			// Begin Select statement
			wcscat(pwszSQLText,L"Select ");
			pos=m_ColList.GetHeadPosition();

			// While not at end of list
			while (pos!=NULL) 
			{		
				CCol& rCol = m_ColList.GetNext(pos);

				// put BLOB in it's own string
				if (rCol.GetIsLong())
				{
					wcscat(pwszBLOBColName,rCol.GetColName());
					wcscat(pwszBLOBColName, L",");
					rgBLOBOrdinal[cBLOBColName]=rCol.GetColNum();
					cBLOBColName++;
				}
				else // copy normal columns into normal string
				{	
					wcscat(pwszSQLText,rCol.GetColName());
					wcscat(pwszSQLText, L",");
					rgColumns[cColumnOrdx++]=rCol.GetColNum();
				}
			}

			// if no blob data, remove last comma
			if (!cBLOBColName)
				pwszSQLText[wcslen(pwszSQLText) - 1] = L'\0';
			else // copy in blob columns
			{
				// Copy in ordinals
				for(DBORDINAL i=0;i<cBLOBColName;i++)
				{
					rgColumns[cColumnOrdx] = rgBLOBOrdinal[i];
					cColumnOrdx++;
				}

				wcscat(pwszSQLText,pwszBLOBColName);

				// Take off last comma
				pwszSQLText[wcslen(pwszSQLText) - 1] = L'\0';
			}

			wcscat(pwszSQLText, L" FROM ");
			wcscat(pwszSQLText, GetTableName());
			wcscat(pwszSQLText, L" order by ");

			// GetFirstColumnName
			CCol colfirst;
			GetColInfo(1,colfirst);

			wcscat(pwszSQLText, colfirst.GetColName());
			wcscat(pwszSQLText, L" ASC ");
			PROVIDER_FREE(pwszBLOBColName);
			break;
		}

		case CREATE_VIEW:
		{
			// the name of the view is either passed as the second parameter of the CreateSQLStmt
			// or will be the table name, otherwise
			WCHAR *pwszViewName = wcsDuplicate(m_pwszViewName ? m_pwszViewName: GetTableName());
			
			// Change the first character of the name
			if(*pwszViewName && !m_pwszViewName)
			{
				*pwszViewName = L'V';
				SetViewName(pwszViewName);
			}
	
			// Allocate memory for the statement:	-6 for the 3 %s in wszCREATE_VIEW
			//										+2 for * and \0
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(wszCREATE_VIEW) +
																 wcslen(pwszViewName) +
																 wcslen(GetTableName()) -4));
			//wszCREATE_VIEW[] is L"Create view %s as Select %s from %s"
			swprintf(pwszSQLText, wszCREATE_VIEW, pwszViewName, L"*", GetTableName());
			PROVIDER_FREE(pwszViewName);
			break;
		}

		case DROP_VIEW:
		{
			// the name of the view is either passed as the second parameter of the CreateSQLStmt
			// or will be the table name, otherwise
			WCHAR *pwszViewName = wcsDuplicate(m_pwszViewName ? m_pwszViewName: GetTableName());
	
			// Change the first character of the name
			if(*pwszViewName && !m_pwszViewName)
			{
				*pwszViewName = L'V';
				SetViewName(pwszViewName);
			}
	
			// Allocate memory for the statement:	-2 for %s in wszDROP_VIEW
			//										+1 for \0
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(wszDROP_VIEW) +
																 wcslen(pwszViewName) -1));
			swprintf(pwszSQLText, wszDROP_VIEW, pwszViewName);
			PROVIDER_FREE(pwszViewName);
			break;
		}

		case CREATE_PROC:
		{
			// the name of the proc is either passed as the second parameter of the CreateSQLStmt
			// or will be the table name, otherwise
			WCHAR		*pwszProcName = pwszTableName2 ? pwszTableName2: GetTableName();	
	
			// Allocate memory for the statement:	-6 for the 3 %s in wszCREATE_PROC
			//										+2 for * and \0
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(wszCREATE_PROC) +
																 wcslen(pwszProcName) +
																 wcslen(GetTableName()) -4));
			//wszCREATE_PROC[] is L"Create procecure %s as Select %s from %s"
			swprintf(pwszSQLText, wszCREATE_PROC, pwszProcName, L"*", GetTableName());
			break;
		}

		case DROP_PROC:
		{
			// the name of the proc is either passed as the second parameter of the CreateSQLStmt
			// or will be the table name, otherwise
			WCHAR		*pwszProcName = pwszTableName2 ? pwszTableName2: GetTableName();	
	
			// Allocate memory for the statement:	-2 for %s in wszDROP_PROC
			//										+1 for \0
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(wszDROP_PROC) +
																 wcslen(pwszProcName) -1));
			swprintf(pwszSQLText, wszDROP_PROC, pwszProcName);
			break;
		}

		case EXEC_PROC:
		{
			// the name of the proc is either passed as the second parameter of the CreateSQLStmt
			// or will be the table name, otherwise
			WCHAR		*pwszProcName = pwszTableName2 ? pwszTableName2 : GetTableName();	
	
			// Allocate memory for the statement:	-2 for the %s in wszEXEC_PROC
			//										+1 for \0
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(wszEXEC_PROC) +
																 wcslen(pwszProcName) +
																 wcslen(GetTableName()) - 2 + 1));
			//wszEXEC_PROC[] is L"{call %s}"
			swprintf(pwszSQLText, wszEXEC_PROC, pwszProcName);
			break;
		}

		case DROP_TABLE:
		{
			WCHAR		*pwszName = pwszTableName2 ? pwszTableName2: GetTableName();	
	
			// Allocate memory for the statement:	-2 for %s in wszDROP_TABLE
			//										+1 for \0
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(wszDROP_TABLE) +
																 wcslen(pwszName) -1));
			swprintf(pwszSQLText, wszDROP_TABLE, pwszName);
			break;
		}

		case CREATE_INDEX:
		{
			WCHAR		*pwszIndexName = pwszTableName2 ? pwszTableName2 : GetTableName();	
			WCHAR		*pwszColNameList = NULL;
			ULONG_PTR	i, len;
			WCHAR		*pwszColName;

			//This statement requires the index columns to be passed in...
			if(!pcColumns || !prgColumns)
				break;

			// create the column name list
			// compute the length of the rezulting string, in characters
			for (len=i=0; i<*pcColumns; i++)
			{
				pwszColName = m_ColList.GetAt(m_ColList.FindIndex(*prgColumns[i])).GetColName();
				len += wcslen(pwszColName);
			}
			len += 2*(*pcColumns-1);
			pwszColNameList = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(len+1));
			pwszColName = m_ColList.GetAt(m_ColList.FindIndex(*prgColumns[0])).GetColName();
			wcscpy(pwszColNameList, pwszColName);
			for (i=1; i<*pcColumns; i++)
			{
				wcscat(pwszColNameList, L", ");
				pwszColName = m_ColList.GetAt(m_ColList.FindIndex(*prgColumns[i])).GetColName();
				wcscat(pwszColNameList, pwszColName);
			}
			// Allocate memory for the SQL Statement
			//	subtract -8 for 4 %s
			//	add 1 for \0
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(wszCREATE_INDEX) +
																 wcslen(pwszIndexName) +
																 wcslen(GetTableName()) + 
																 len -7));

			//String layout "Create %s index %s on %s (%s)";
			// Format SQL Statement
			// Statement like: "create [unique] index index_name on table_name (column_name)"    
			swprintf(pwszSQLText, wszCREATE_INDEX, L"", pwszIndexName, GetTableName(), pwszColNameList);
			PROVIDER_FREE(pwszColNameList);

			// pcColumns and prgColumns were input parameters!
			pcColumns	= NULL;
			prgColumns	= NULL;
			break;
		}

		// Create index in descending order.  By default CREATE_INDEX is in ascending order.
		case CREATE_INDEX_DESC:
		{
			WCHAR		*pwszIndexName = pwszTableName2 ? pwszTableName2 : GetTableName();	
			WCHAR		*pwszColNameList = NULL;
			ULONG_PTR	i, len;
			WCHAR		*pwszColName;

			//This statement requires the index columns to be passed in...
			if(!pcColumns || !prgColumns)
				break;

			// create the column name list
			// compute the length of the rezulting string, in characters
			for (len=i=0; i<*pcColumns; i++)
			{
				pwszColName = m_ColList.GetAt(m_ColList.FindIndex(*prgColumns[i])).GetColName();
				len += wcslen(pwszColName)+wcslen(L" DESC");
			}
			len += 2*(*pcColumns-1);
			pwszColNameList = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(len+1));
			pwszColName = m_ColList.GetAt(m_ColList.FindIndex(*prgColumns[0])).GetColName();
			wcscpy(pwszColNameList, pwszColName);
			wcscat(pwszColNameList, L" DESC");	// Add descending keyword for this column
			for (i=1; i<*pcColumns; i++)
			{
				wcscat(pwszColNameList, L", ");
				pwszColName = m_ColList.GetAt(m_ColList.FindIndex(*prgColumns[i])).GetColName();
				wcscat(pwszColNameList, pwszColName);
				wcscat(pwszColNameList, L" DESC");	// Add descending keyword for this column
			}
			// Allocate memory for the SQL Statement
			//	subtract -8 for 4 %s
			//	add 1 for \0
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(wszCREATE_INDEX) +
																 wcslen(pwszIndexName) +
																 wcslen(GetTableName()) + 
																 len -7));

			//String layout "Create %s index %s on %s (%s)";
			// Format SQL Statement
			// Statement like: "create [unique] index index_name on table_name (column_name)"    
			swprintf(pwszSQLText, wszCREATE_INDEX, L"", pwszIndexName, GetTableName(), pwszColNameList);
			PROVIDER_FREE(pwszColNameList);

			// pcColumns and prgColumns were input parameters!
			pcColumns	= NULL;
			prgColumns	= NULL;
			break;
		}


		case DROP_INDEX:
		{
			WCHAR		*pwszIndexName = pwszTableName2 ? pwszTableName2 : GetTableName();	

			// Allocate memory for the SQL Statement
			//	subtract -4 for 2 %s
			//	add 1 for \0
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(wszDROP_INDEX) +
																 wcslen(pwszIndexName) +
																 wcslen(GetTableName()) -3));

			//String layout "Create %s index %s on %s (%s)";
			// Format SQL Statement
			// Statement like: "create [unique] index index_name on table_name (column_name)"    
			swprintf(pwszSQLText, wszDROP_INDEX, GetTableName(), pwszIndexName);
			break;
		}

		case ALTER_TABLE_DROP_COLUMN:
		{
			//This statement requires the column to be droped to be passed in...
			if(!pcColumns || !prgColumns || *pcColumns!=1)
				break;

			size_t	ulStringSize = 0;
			DBORDINAL	nCol = (DBORDINAL)(*prgColumns)[0];
			
			CCol&	rCol = GetColInfoForUpdate(nCol);

			ASSERT(rCol.GetColNum() == (DBORDINAL)(nCol));
			if (!pwszTableName2)
				pwszTableName2 = GetTableName();
			
			// compute the string size
			ulStringSize =	wcslen(wszALTER_TABLE_DROP_COLUMN)	+ 
							wcslen(pwszTableName2)				+
							wcslen(rCol.GetColName()) - 3;
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*ulStringSize);
			//String layout "Alter Table %s Drop Column %s";
			// Format SQL Statement
			// Statement like: "Alter Table table_name Drop Column column_name"    
			swprintf(pwszSQLText, wszALTER_TABLE_DROP_COLUMN, pwszTableName2, rCol.GetColName());
			pcColumns	= NULL;
			prgColumns	= NULL;
			break;
		}

		case ALTER_TABLE_ADD:
		{
			// We need a second table object to create this statement
			ASSERT(pwszTableName2);
			size_t	ulStringSize;
			
			// compute the string size
			ulStringSize =	wcslen(wszALTER_TABLE_ADD)	+ 
							wcslen(pwszTableName2)		+
							wcslen(GetTableName()) - 3;
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*ulStringSize);
			//String layout "Alter Table %s add %s";
			// Format SQL Statement
			// Statement like: "Alter Table table_name add column_name type-name"    
			swprintf(pwszSQLText, wszALTER_TABLE_ADD, GetTableName(), pwszTableName2);
			break;
		}

		case ALTER_TABLE_ADD_EX:
		{
			// the column to be added is the tail of the column list
			// more properties can be added here: autoincrement, not nullable, etc
			size_t	ulStringSize;
			CCol&	rCol = m_ColList.GetTail();
			
			// compute the string size
			if(rCol.GetColName() && rCol.GetProviderTypeName() && GetTableName())
			{
				ulStringSize =	wcslen(wszALTER_TABLE_ADD_EX)		+ 
								wcslen(rCol.GetColName())			+
								wcslen(rCol.GetProviderTypeName())	+
								wcslen(GetTableName()) - 3;
				if (rCol.GetHasDefault())
					ulStringSize += wcslen(V_BSTR(&col.GetDefaultValue()))+wcslen(L" DEFAULT ");
				pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*ulStringSize);
				//String layout "Alter Table %s add %s";
				// Format SQL Statement
				// Statement like: "Alter Table table_name add column_name type-name"    
				swprintf(pwszSQLText, wszALTER_TABLE_ADD_EX, GetTableName(), 
						 rCol.GetColName(), rCol.GetProviderTypeName());
				if (rCol.GetHasDefault())
				{
					wcscat(pwszSQLText, L" DEFAULT ");
					wcscat(pwszSQLText, V_BSTR(&rCol.GetDefaultValue()));
				}
			}
			break;
		}
		// "update <table-name> set col1 = ?, col2 = ? ... where col1 = ? and col2 = ? ..."
		case UPDATE_WITH_PARAMS_WHERE:
		{
			
			DBORDINAL	cUpdatableCols = 0;
			DB_LORDINAL	* prgUpdatableCols = NULL;
			DBORDINAL	cSearchableCols = 0;
			DB_LORDINAL	* prgSearchableCols = NULL;

			// Create the updatable column list and array of column numbers,
			//asking for only columns that are updatable
			TESTC_(hr = CreateList(LT_PARAM_UPDATE,&pwszColList, &cUpdatableCols, &prgUpdatableCols, 
				UPDATEABLE_COLS_IN_LIST, FORWARD), S_OK);

			// Create the searchable column list with parameterized search criteria	
			// The columns must be updatable also otherwise we don't know what value to use
			// in the parameter.
			TESTC_(hr = CreateList(LT_PARAM_SEARCH,&pwszParamColList, &cSearchableCols, &prgSearchableCols, 
				SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST, FORWARD, PRIMARY, ulRowNumber), S_OK);

			//We must have at least one searchable column
			if(cSearchableCols)
			{
				// Compute total columns (parameters) for the statement
				cColumns = cUpdatableCols + cSearchableCols;

				// Allocate memory for total col list
				SAFE_ALLOC(rgColumns, DB_LORDINAL, cColumns);

				// Copy in the results for each set
				memcpy(rgColumns, prgUpdatableCols, (size_t)cUpdatableCols*sizeof(DB_LORDINAL));
				memcpy(rgColumns+cUpdatableCols, prgSearchableCols,
					(size_t)cSearchableCols*sizeof(DB_LORDINAL));

				// Allocate memory for the sql statement. -6 is for %s's
				pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
							(wcslen(wszUPDATE_1ROW) +	
							wcslen(pwszColList) +	
							wcslen(pwszParamColList) +	
							wcslen(GetTableName()) - 6)) +
							sizeof(WCHAR));

				// Format SQL Statement
				swprintf(pwszSQLText, wszUPDATE_1ROW, GetTableName(), pwszColList, pwszParamColList);
			}

			SAFE_FREE(prgUpdatableCols);
			SAFE_FREE(prgSearchableCols);

			break;
		}
		case SELECT_NO_TABLE:
			pwszSQLText = wcsDuplicate((WCHAR *)wszSELECT_STAR_FROM);
			break;
		
		case SELECT_INVALIDTBLNAME:
			pwszSQLText = wcsDuplicate((WCHAR *)wszSELECT_INVALIDTBLNAME);
			break;

		case INSERT_NO_TABLE:
			pwszSQLText = wcsDuplicate((WCHAR *)wszINSERTINTO);
			break;

		case NO_QUERY:
			pwszSQLText = wcsDuplicate(L"");
			break;

		case DEEP_SELECT_SUBTREE:
		case SHALLOW_SCOPED_SELECT:
			pwszSQLText = wcsDuplicate(GetModInfo()->GetRowScopedQuery());
			break;

		case CHANGE_CURRENT_CATALOG:
			ASSERT(pwszTableName2);	// this string is the catalog name
			size_t	ulStringSize;
			
			// compute the string size
			ulStringSize =	wcslen(wszCHANGE_CURRENT_CATALOG)	+ 
							wcslen(pwszTableName2)	- 1;
			pwszSQLText = (WCHAR*) PROVIDER_ALLOC(sizeof(WCHAR)*ulStringSize);
			//String layout "use %s ";
			// Format SQL Statement
			// Statement like: "use catalog-name"    
			swprintf(pwszSQLText, wszCHANGE_CURRENT_CATALOG, pwszTableName2);
			break;	
		default:
			ASSERT(FALSE);
			hr = E_FAIL;
	}

	
	//INI File Queries
	//Not all providers will support SQL Syntax.
	//The way we deal with this is if the user has specified either DEFAULTQUERY=
	//In the InitString or a INI File, we will try and use the QuerySyntax specified
	//Instead of using hard-coded SQL Syntax...
	if(GetModInfo()->GetFileName() || GetModInfo()->GetDefaultQuery())
	{
		WCHAR* pwszINIQuery = NULL;
		
		//If using ini file
		if(GetModInfo()->GetFileName())
		{
			if(eSQLStmt == USE_SUPPORTED_SELECT_ALLFROMTBL)
				eSQLStmt = SELECT_ALLFROMTBL;
			pwszINIQuery = GetModInfo()->GetParseObject()->GetQuery(eSQLStmt);
		}
		// Otherwise return DEFAULT query
		else
		{
			pwszINIQuery = GetModInfo()->GetDefaultQuery();
		}

		if(!pwszINIQuery)
		{
			//Someone removed one of the queries from the INI File, probably becuase this
			//particular provider does not have a concept of this particular query...
			hr = DB_E_NOTSUPPORTED;
			goto CLEANUP;
		}	

		// If the user didn't request a count of columns or an array of columns then
		// we can always satisfy the request from the ini file
		if (!pcColumns && !prgColumns)
		{
			//Free previous generated statements	
			SAFE_FREE(pwszSQLText);
			PROVIDER_FREE(pwszColList);	
			PROVIDER_FREE(rgColumns);
			cColumns = 0;
			
			//Use the INI File statement instead
			pwszSQLText = wcsDuplicate(pwszINIQuery);
		}

		//Since this is potentially provider specifc command syntax (not standard SQL), 
		//There is no way for me to know the order of the columns that will be produced from
		//the query, which I need inorder to compare later one.  For example:  The entire "table"
		//contains all columns, but the query returns a subset, which I need to know the "mapping"
		//of the columns that should be returned to the full data layout.

		//With the current INI File we cannot do this unless the query is SELECT_ALL* which
		//must match the order of the data in the INI File.  The only other approach is too
		//see if the query is standard ANSI SQL, and then we know the order, since we generated 
		//the statement.  This is the whole reason we do the INI File lookup AFTER we generate the 
		//normal ANSI SQL Statement, if they are the same, then we know the order, if not 
		//we cannot test your provider since we need to the know the mapping of the subset query
		//to the table layout. (something we don't yet support in INI files)
		if(!pwszSQLText || !(wcscmp(pwszINIQuery, pwszSQLText)==0))
		{
			switch(eSQLStmt)
			{
				case USE_SUPPORTED_SELECT_ALLFROMTBL:
				case SELECT_INVALIDTBLNAME:  
				case SELECT_VALIDATIONORDER: 
				case SELECT_ALLFROMTBL:
				case SELECT_COLLISTFROMTBL:
				case SELECT_ORDERBYNUMERIC: 
				case SELECT_DUPLICATECOLUMNS: 
				case SELECT_REVERSEDUPLICATECOLUMNS:
				case SELECT_REVCOLLIST: 
				case SELECT_CHANGECOLNAME: 
				case USE_OPENROWSET:
				case SELECT_EMPTYROWSET:
				{
					//The Query specified in the INI does not match the SQL Statement we generated
					//so we do not know the order of the columns, unless its one of the standard
					//statemenets that must match the INI data order.
				
					//All of these statemenets produce rowsets in the same order as the data
					//in the INI file...

					//Free previous generated statements	
					SAFE_FREE(pwszSQLText);
					PROVIDER_FREE(pwszColList);	
					PROVIDER_FREE(rgColumns);
					cColumns = 0;
					
					//Use the INI File statement instead
					pwszSQLText = wcsDuplicate(pwszINIQuery);

					//This statements matches the entire table, so just return all columns...
					if(m_ColList.GetCount())
					{
						CreateColList(FORWARD, &pwszColList, &cColumns, &rgColumns);
						if (eSQLStmt == SELECT_DUPLICATECOLUMNS || eSQLStmt == SELECT_REVERSEDUPLICATECOLUMNS)
						{
							cColumns+=cColumns;
						}
					}
					break;
				}
						
				default:
				{
					//The query does not match the entire table so we cannot 
					//correctly return the columns, since we don't know the columns
					//or order the query would produce.  Same error as if the provider
					//didn't support this query (removed from INI file)
					hr = DB_E_NOTSUPPORTED;
					break;
				}
			};
		}
	}
	 	

CLEANUP:
	if(FAILED(hr))
	{
		cColumns = 0;
		PROVIDER_FREE(rgColumns);
		PROVIDER_FREE(pwszSQLText);
	}
	else
	{
		//Set return values, if caller wants them
		if(pcColumns)
			*pcColumns = cColumns;

		// User doesn't want cols, so free them
		if(prgColumns)
			*prgColumns = rgColumns;
		else
			PROVIDER_FREE(rgColumns);

		//Set Return Values
		*ppwszStatement = pwszSQLText;
		if(pwszSQLText==NULL)
			hr = E_OUTOFMEMORY;
	}
	
	// Cleanup
	PROVIDER_FREE(pwszColList);	
	PROVIDER_FREE(pwszParamColList);
	PROVIDER_FREE(pwszRevColList);
	return hr;
} //CTable::CreateSQLStmt

//---------------------------------------------------------------------------
// CTable::CreateList
//
// CTable			|
// CreateList	|
//
// Client must release and/or free any object and/or memory returned;
//
// This function creates various lists from the CTable.  The column and 
// parameter lists will be comma separated.  Seach lists will be separated
// by " AND " and will return colname = <literal> | <param marker>.  
// This function also returns an array of column numbers.
//
//
// @mfunc	CreateList
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL, E_OUTOFMEMORY, DB_E_BADTYPE  | Problem
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateList(
	ELIST_TYPE		eListType,		// @parm [IN] Type of list to create	
	LPWSTR *		ppwszList,		// @parm [OUT] List generated
	DBORDINAL *		pcColumns,		// @parm [IN/OUT] In:  Maximum number of columns that can be 
									// in the Column array. Ignored if *prgColumns==NULL.
									// Out:  Actual number of columns in finished array.									
	DB_LORDINAL **	pprgColumns,	// @parm [IN/OUT] Array of column numbers, memory is allocated for caller
									// if *pprgColumns == NULL, else it is assums the array is *pcColumns
									// allocated by caller to hold all column numbers and the first column
									// is a constant column such as 'ABC'.
	ECOLS_IN_LIST	eColsInList,	// @parm [IN] Type of columns in list (Default = ALL_COLS_IN_LIST)
	ECOLUMNORDER	eColOrder,		// @parm [IN]  Column list order, default FORWARD
	EVALUE			eValue,			// @parm [IN] Type of makedata (Default = PRIMARY)
	DBCOUNTITEM		ulRowNum,		// @parm [IN] Row number to use for literals, default 1
	LPWSTR **		pprgColumnNames,	// @parm[OUT] Column names, if null are not returned, (Default = NULL)
									// Client must release each string and array of strings
	DBORDINAL *	prgColumns			// @parm [IN]   On input specifies desired columns,
									// over-riding eColsInList.  Default NULL.
)
{
	HRESULT hr = E_FAIL;
	BOOL fCandidate = FALSE;
	DBCOUNTITEM cCols = m_ColList.GetCount();
	DBCOUNTITEM iCol = 0;
	DBCOUNTITEM	idx = 0;
	DBORDINAL * prgCols = NULL;
	DBORDINAL cFinalCols = 0;
	LPWSTR * prgColNames = NULL;
	DBORDINAL iColNum;
	LPWSTR pwszCol = NULL;		// Column name
	LPWSTR pwszSearch = NULL;	// Equal sign
	LPWSTR pwszLit = NULL;		// Literal and value
	LPWSTR pwszDelim = NULL;	// comma or AND
	LPWSTR * prgColStr = NULL;	// Array of strings for each column
	LPWSTR pwszFinalStr = NULL;	// Final string to return
	LPWSTR pwszPrefix = L"";	// Prefix for in or inout param name definitions
	LPWSTR pwszOut = L" out";	// For output params

#if defined(_WIN64) || defined(_M_ALPHA)
	ULONGLONG ulStrSize = 0;		// Size of string needed for a column
	ULONGLONG ulFinalSize = 0;		// Size of final string
#else
	ULONG ulStrSize = 0;		// Size of string needed for a column
	ULONG ulFinalSize = 0;		// Size of final string
#endif

	POSITION	pos;			// position in list
	BOOL fNULLSearch = FALSE;	// Is this IS NULL rather than literal or param

	// Argument checking
	hr = E_INVALIDARG;
	TESTC(eListType >= LT_COLNAME && eListType < LT_LAST);
	TESTC(ppwszList != NULL);
	TESTC(pprgColumns == NULL || pcColumns != NULL);
	TESTC(eColsInList >= ALL_COLS_IN_LIST && eColsInList <= ECOLS_LAST); 
	TESTC(eColOrder == FORWARD || eColOrder == REVERSE);
	TESTC(eValue == PRIMARY || eValue == SECONDARY);
	TESTC(ulRowNum > 0);

	// Save input params
	if (pprgColumns && *pprgColumns)
	{
		prgCols = (DBORDINAL *)*pprgColumns;
		cCols = *pcColumns;
	}

	// Initialize output params
	*ppwszList = NULL;
	if (pprgColumns && !(*pprgColumns))
		*pcColumns = 0;

	// Allocate needed buffers and zero out
	hr = E_OUTOFMEMORY;
	SAFE_ALLOC(prgCols, DBORDINAL, cCols);
	memset(prgCols, 0, (size_t)cCols*sizeof(DB_LORDINAL));
	SAFE_ALLOC(prgColStr, LPWSTR, cCols);
	memset(prgColStr, 0, (size_t)cCols*sizeof(LPWSTR));

	if (pprgColumnNames)
	{
		SAFE_ALLOC(prgColNames, LPWSTR, cCols);
		memset(prgColNames, 0, (size_t)cCols*sizeof(LPWSTR));
	}

	hr = E_FAIL;

	// Determine delimiter between items
	switch(eListType)
	{
		case LT_COLNAME:
		case LT_PARAM:
		case LT_LITERAL:
		case LT_PARAM_UPDATE:
		case LT_PARAM_OUT:
		case LT_RPC_PARAM_DEF:
		case LT_RPC_OUT_PARAM_DEF:
		case LT_RPC_PARAM_OUT:
			pwszDelim = L", ";
			break;
		case LT_PARAM_SEARCH:
		case LT_LITERAL_SEARCH:
		case LT_RPC_PARAM_SEARCH:
			pwszDelim = L" AND ";
			break;
	}

	// Determine prefix
	switch(eListType)
	{
		case LT_RPC_PARAM_DEF:
		case LT_RPC_PARAM_SEARCH:
			pwszPrefix = L"@pi";
			break;
		case LT_RPC_OUT_PARAM_DEF:
		case LT_RPC_PARAM_OUT:
			pwszPrefix = L"@pio";
			break;
	}

	// While not at end of list
	pos=m_ColList.GetHeadPosition();
	while (pos!=NULL) 
	{
		fCandidate = FALSE;
		ulStrSize = 0;

		// Get the column info
		CCol& rCol = m_ColList.GetNext(pos);

		// Determine delimiter between items
		switch(eListType)
		{
			case LT_PARAM_SEARCH:
			case LT_LITERAL_SEARCH:
				pwszSearch = L" = ";
				if (rCol.GetSearchable() == DB_LIKE_ONLY)						
					pwszSearch = L" LIKE ";
		}
		
		iColNum = rCol.GetColNum();

		// See if it's a candidate column
		if (prgColumns)
		{
			// prgColumns takes priority
			for (iCol = 0; iCol < cCols && !fCandidate; iCol++)
			{
				hr = E_INVALIDARG;
				TESTC(prgColumns[iCol] > 0);
				if (prgColumns[iCol] == iColNum)
					fCandidate = TRUE;
			}
		}
		else
		{
			// We use one of the "canned" selections
			switch(eColsInList)
			{
				case ALL_COLS_IN_LIST:
					fCandidate = TRUE;
					// We can't build a literal for a non-updatable column
					if (eListType == LT_LITERAL_SEARCH && !rCol.GetUpdateable())
						fCandidate = FALSE;
					break;
				case UPDATEABLE_COLS_IN_LIST:
					if (rCol.GetUpdateable())
						fCandidate = TRUE;
					break;
				case SEARCHABLE_COLS_IN_LIST:
					if (DB_UNSEARCHABLE != rCol.GetSearchable())
						fCandidate = TRUE;
					// We can't build a literal for a non-updatable column
					if (eListType == LT_LITERAL_SEARCH && !rCol.GetUpdateable())
						fCandidate = FALSE;
					break;
				case SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST:
					if (rCol.GetUpdateable() &&
						DB_UNSEARCHABLE != rCol.GetSearchable())
						fCandidate = TRUE;
					break;
				case INDEX_COL_IN_LIST:
					if (rCol.GetColNum() == GetIndexColumn())
						fCandidate = TRUE;
					break;
			}

			switch(eListType)
			{
				case LT_PARAM_OUT:
				case LT_RPC_OUT_PARAM_DEF:
				case LT_RPC_PARAM_OUT:
					// No support for output params for legacy BLOBs
					if (rCol.GetIsLong() && !(rCol.GetIsNewLongType() || !wcscmp(rCol.GetProviderTypeName(), L"xml")))
						fCandidate = FALSE;
			}
		}


		if (fCandidate)
		{
			LPWSTR pwszStr = NULL;
			fNULLSearch = FALSE;

			// Make sure we don't write more values than we have room for
			if (idx >= cCols)
			{
				hr = E_FAIL;
				goto CLEANUP;
			}

			switch(eListType)
			{
				case LT_PARAM_SEARCH:
				case LT_PARAM_UPDATE:
				case LT_LITERAL_SEARCH:
				case LT_RPC_PARAM_SEARCH:
				{
					// GetLiteralAndValue handles NULL values, returning S_OK and "NULL". 
					// This is wrong for searches, need IS NULL.
					// Note this will fail for columns we can't build literals for, such
					// as autoinc or timestamp columns.
					pwszSearch = L" = ";

					// Determine delimiter between items
					switch(eListType)
					{
						case LT_PARAM_SEARCH:
						case LT_LITERAL_SEARCH:
							pwszSearch = L" = ";
							if (rCol.GetSearchable() == DB_LIKE_ONLY)						
								pwszSearch = L" LIKE ";
					}

					if (rCol.GetUpdateable())
					{
						TESTC_(GetLiteralAndValue(rCol, &pwszLit, ulRowNum, rCol.GetColNum(), eValue), S_OK);
						if (!wcscmp(pwszLit, L"NULL") && eListType != LT_PARAM_UPDATE)
						{
							// There is no literal for IS NULL.
							*pwszLit = L'\0';
							pwszSearch = L" IS NULL ";
							fNULLSearch = TRUE;
						}
					}

					// See if we need parameter markers
					if (eListType == LT_PARAM_SEARCH ||
						eListType == LT_PARAM_UPDATE ||
						eListType == LT_RPC_PARAM_SEARCH)
					{
						// We want a parameter marker in place of a literal
						if (!fNULLSearch)
						{
							SAFE_FREE(pwszLit);
							if (eListType == LT_RPC_PARAM_SEARCH)
							{
								SAFE_ALLOC(pwszLit, WCHAR, wcslen(pwszPrefix)+wcslen(rCol.GetColName())+1);
								wcscpy(pwszLit, pwszPrefix);
								wcscat(pwszLit, rCol.GetColName());
							}
							else
							pwszLit = wcsDuplicate(L"?");
						}
					}
					ulStrSize += wcslen(pwszLit);
				}
					// Fall through
				case LT_COLNAME:
					pwszCol = wcsDuplicate(rCol.GetColName());
					CHECK_MEMORY_HR(pwszCol);
					ulStrSize += wcslen(pwszCol);
					break;
				case LT_PARAM:
					pwszCol = wcsDuplicate(L"?");
					CHECK_MEMORY_HR(pwszCol);
					ulStrSize += wcslen(pwszCol);
					break;
				case LT_LITERAL:
					// GetLiteralAndValue handles NULL values, returning S_OK and "NULL". 
					// Note this will fail for columns we can't build literals for, such
					// as autoinc or timestamp columns.
					TESTC_(GetLiteralAndValue(rCol, &pwszCol, ulRowNum, rCol.GetColNum(), eValue), S_OK);
					ulStrSize += wcslen(pwszCol);
					break;
				case LT_PARAM_OUT:
					// ? = col
					SAFE_ALLOC(pwszCol, WCHAR, wcslen(L"? = ")+wcslen(rCol.GetColName())+1);
					CHECK_MEMORY_HR(pwszCol);
					wcscpy(pwszCol, L"? = ");
					wcscat(pwszCol, rCol.GetColName());
					ulStrSize += wcslen(pwszCol);
					break;
				case LT_RPC_PARAM_OUT:
				{
					// @pcol = col
					LPWSTR pwszFmt = L"%s%s = %s";
					ULONG ulCol = _scwprintf(pwszFmt, pwszPrefix, rCol.GetColName(), rCol.GetColName())+1;
					SAFE_ALLOC(pwszCol, WCHAR, ulCol);
					CHECK_MEMORY_HR(pwszCol);
					swprintf(pwszCol, pwszFmt, pwszPrefix, rCol.GetColName(), rCol.GetColName());
					ulStrSize += wcslen(pwszCol);
					break;
				}
				case LT_RPC_PARAM_DEF:
				case LT_RPC_OUT_PARAM_DEF:
				{
					ULONG ulCol;
					LPWSTR pwszFmt = L"%s%s %s";
					LPWSTR pwszColDef = NULL;
					LPWSTR pwszIdentity = NULL;
					rCol.CreateColDef(&pwszColDef);
					CHECK_MEMORY_HR(pwszColDef);

					// Hack for MSDASQL: Strip off "identity" returned at end of col def
					if (pwszIdentity = wcsstr(pwszColDef, L"identity"))
						*pwszIdentity = L'\0';
					
					ulCol = _scwprintf(pwszFmt, pwszPrefix, pwszColDef, pwszOut)+1;
				
					// @picol1 <type>(len) <out>
					SAFE_ALLOC(pwszCol, WCHAR, ulCol);
					CHECK_MEMORY_HR(pwszCol);

					if (eListType == LT_RPC_OUT_PARAM_DEF)
						swprintf(pwszCol, pwszFmt, pwszPrefix, pwszColDef, pwszOut);
					else
						swprintf(pwszCol, pwszFmt, pwszPrefix, pwszColDef, L"");

					ulStrSize += wcslen(pwszCol);
					SAFE_FREE(pwszColDef);

					break;
				}

			}

			// Include search clause in size for string if needed
			if (pwszSearch)
				ulStrSize += wcslen(pwszSearch);

			// If this column generates an IS NULL search, then don't include
			// in list as it's not really a literal or param.
			if (!fNULLSearch)
			{
				cFinalCols++;

				// Save col number
				prgCols[idx] = iColNum;

				// Save col name
				if (pprgColumnNames)
				{
					prgColNames[idx] = wcsDuplicate(rCol.GetColName());
					CHECK_MEMORY_HR(prgColNames[idx]);
				}
			}

			// Save string for this column
			hr = E_OUTOFMEMORY;
			SAFE_ALLOC(pwszStr, WCHAR, ulStrSize+1);
			*pwszStr = L'\0';
			if (pwszCol)
				wcscat(pwszStr, pwszCol);
			if (pwszSearch)
				wcscat(pwszStr, pwszSearch);
			if (pwszLit)
				wcscat(pwszStr, pwszLit);

			prgColStr[idx] = pwszStr;

			ulFinalSize+=ulStrSize;

			// Free column name and literal strings
			SAFE_FREE(pwszCol);
			SAFE_FREE(pwszLit);

			idx++;
		}
	}

	// Add on the number of delimiters required to the final size
	if (idx)
		ulFinalSize += (idx-1)*wcslen(pwszDelim);

	// Now we have arrays containing everything needed for the final string.
	hr = E_OUTOFMEMORY;
	SAFE_ALLOC(pwszFinalStr, WCHAR, ulFinalSize+1);
	*pwszFinalStr = L'\0';
	
	// Put required strings in final string, reversing if need be
	for (iCol = 0; iCol < idx; iCol++)
	{
		DBCOUNTITEM iStr = iCol;
		if (eColOrder == REVERSE)
		{
			iStr = idx - iCol - 1;
			
			// Switch values in output arrays
			if (iCol < iStr)
			{
				if (prgCols)
				{
					DBORDINAL ulTemp; 

					ulTemp = prgCols[iCol];
					prgCols[iCol]=prgCols[iStr];
					prgCols[iStr]=ulTemp;
				}
				if (prgColNames)
				{
					LPWSTR pwszTemp;

					pwszTemp = prgColNames[iCol];
					prgColNames[iCol]=prgColNames[iStr];
					prgColNames[iStr]=pwszTemp;
				}
			}
		}

		if (*pwszFinalStr)
			wcscat(pwszFinalStr, pwszDelim);
		wcscat(pwszFinalStr, prgColStr[iStr]);
	}

	hr = S_OK;
	
CLEANUP:

	// Release unneeded memory
	for (iCol = 0; iCol < idx; iCol++)
	{
		SAFE_FREE(prgColStr[iCol]);

		if (FAILED(hr) && prgColNames)
			SAFE_FREE(prgColNames[iCol]);
	}

	SAFE_FREE(prgColStr);
	// If we failed, release any memory left
	if (FAILED(hr))
	{		
		SAFE_FREE(prgColNames);
		SAFE_FREE(prgCols);
		SAFE_FREE(pwszFinalStr);
		idx = 0;
	}

	// Populate output params
	if (ppwszList)
		*ppwszList = pwszFinalStr;
	if (pcColumns)
		*pcColumns = cFinalCols;
	if (pprgColumns)
		*pprgColumns = (DB_LORDINAL *)prgCols;
	if (pprgColumnNames)
		*pprgColumnNames = prgColNames;

	return hr;
}

//---------------------------------------------------------------------------
// CTable::CreateColList
//
// CTable			|
// CreateColList	|
//
// Client must release and/or free any object and/or memory returned;
//
// This function creates a column list from the CTable.  The column list
// will be comma seperated.  This function also returns an array of
// column numbers.
//
// added last param, necessary to several test cases, needs to work
// regardless if this is the select part or the where part of the 
// sql clause
//
// @mfunc	CreateColList
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Problem
//
//---------------------------------------------------------------------------
HRESULT CTable::CreateColList(
	ECOLUMNORDER	eColOrder,		// @parm [IN]  Column list order, default FORWARD
	WCHAR **		ppwszColList,	// @parm [OUT] Column list generated
	DBORDINAL *		pcColumns,		// @parm [IN/OUT] In:  Maximum number of columns that can be 
									// in the Column array. Ignored if *prgColumns==NULL.
									// Out:  Actual number of columns in finished array.									
	DB_LORDINAL **	prgColumns,		// @parm [IN/OUT] Array of column numbers, memory is allocated for caller
									// if *prgColumns == NULL, else it is assums the array is *pcColumns
									// allocated by caller to hold all column numbers and the first column
									// is a constant column such as 'ABC'.
	ECOLS_IN_LIST	eColsInList,	// @parm [IN] Type of columns in list (Default = ALL_COLS_IN_LIST)
	BOOL			fAddParams,		// @parm [IN] TRUE if parameterized search criteria 
									//			  is added for each searchable column (Default = FALSE)
	BOOL			fAddData,		// @parm [IN] TRUE if data search criteria is
									//			  added for each searchable column (Default = FALSE)
	WCHAR ***		prgColumnNames,	// @parm[OUT] Column names, if null are not returned, (Default = NULL)
									// Client must release each string and array of strings
	EVALUE			eValue,			// @parm [IN] Type of makedata (Default = PRIMARY)
	DBCOUNTITEM		ulRowNumber,	// @parm [IN] Row number to use, default 1
	DBORDINAL		iOrdinal		// @parm [IN] Col number to use, default 0 (all)
)
{
	HRESULT		hr=E_FAIL;				// Returned HRESULT
	POSITION	pos;					// position in list
	WCHAR *		pwszSQLText=NULL;		// sql text to execute
	WCHAR *		pwszTempText=NULL;		// sql text to execute
	DBORDINAL	inc=0;					// Index
	WCHAR		szPredicate[10];		// Predicate holder
	WCHAR *		pwszMakeData = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR) * 2002);
	BOOL		fMayWrite=FALSE;

	WCHAR **	rgColNames=NULL;		// array of column names
	
	if (!pwszMakeData)
		return E_OUTOFMEMORY;

	ASSERT(pcColumns);
	ASSERT(!(fAddParams && fAddData));

	// Check the Column count
	if( !m_ColList.GetCount() )
	{
		PROVIDER_FREE(pwszMakeData);
		return S_OK;
	}
	
	pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
					m_ColList.GetCount() * 2000) + sizeof(WCHAR));
	
	// Check return
	if (!pwszSQLText)
		goto CLEANUP;

	pwszTempText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) *
					m_ColList.GetCount() * 60) + sizeof(WCHAR));

	// Check return
	if(!pwszTempText)
		goto CLEANUP;

	if(prgColumnNames)
	{
		rgColNames = (WCHAR **)PROVIDER_ALLOC(sizeof(WCHAR *) * m_ColList.GetCount());
		if (!rgColNames)
			goto CLEANUP;

		//Initialize Array
		memset(rgColNames, 0, (size_t)((sizeof(WCHAR *) * m_ColList.GetCount())));
	}

	pwszSQLText[0]=L'\0';
	pwszTempText[0]=L'\0';

	// Allocate memory for column number array if caller has not
	if(*prgColumns == NULL)
	{
		// Allocate memory for the array of column numbers, note this 
		// is the max, we may not use all of this memory
		*prgColumns = (DB_LORDINAL *) PROVIDER_ALLOC(sizeof(DB_LORDINAL) * m_ColList.GetCount());
		if (!(*prgColumns))
			goto CLEANUP;

		// Initialize to 0
		**prgColumns = 0;

		// Set counter to the order
		if (eColOrder == FORWARD)
			inc = 0;
		else
			inc = m_ColList.GetCount() - 1;
	
		// ReInitialize, we'll increment as we go to find actual number
		// of columns we put in the array
		*pcColumns = 0;
	}
	else
	{
		//Make sure the user alloced array is large enough
		ASSERT(*pcColumns >= m_ColList.GetCount());
		
		// Adjust for the first col being a constant ("ABC"), not in table.
		// we'll increment count as we add more columns to the table
		*pcColumns = 1;

		if (eColOrder == FORWARD)
			inc = 1;
		else
			inc = m_ColList.GetCount();
	}

	if (fAddData)
		CHECK(MayWrite(&fMayWrite),S_OK);

	// While not at end of list
	pos=m_ColList.GetHeadPosition();
	while (pos!=NULL) 
	{		
		// Put column name in list
		CCol& rCol = m_ColList.GetNext(pos);

		// Limit to specific column if required
		if (iOrdinal && rCol.GetColNum() != iOrdinal)
			continue;
		
		if (eColOrder == FORWARD)
		{
			// Check searchability if asked for
			if (eColsInList == SEARCHABLE_COLS_IN_LIST)
			{
				// Use equals if we can for parameterized search criteria
				if ((rCol.GetSearchable() == DB_SEARCHABLE) ||
						(rCol.GetSearchable() == DB_ALL_EXCEPT_LIKE))
				{
					if (fAddParams)	// = ?
						wcscpy(szPredicate, wszEQUAL_TO_QUESTION_MARK);
					else
						wcscpy(szPredicate, wszEQUALS);
				}
				else
					// Use Like if we can, otherwise we'll skip this column
					if (rCol.GetSearchable() == DB_LIKE_ONLY)						
					{
						if (fAddParams)// LIKE ?
							wcscpy(szPredicate, wszLIKE_QUESTION_MARK);
						else
							wcscpy(szPredicate, wszLIKE);
					}
					else						
						//We can't add this column to the list
						//since its not searchable, so skip it and move on
						continue;

				if (fAddParams)
				{
					// Column name
					// LIKE ? or = ?
					wcscat(pwszSQLText,rCol.GetColName());
					wcscat(pwszSQLText, szPredicate);
				}
				else
				{
					// Here we are creating the where part of select statement
					if (fAddData)
					{
						rCol.MakeData(pwszSQLText, 1, eValue);

						// If MakeData put a NULL into this row/column in the table
						if (hr == S_FALSE)
						{
							// Column name
							wcscat(pwszSQLText,rCol.GetColName());
							wcscat(pwszSQLText, wszISNULL);
						}
						else
						{
							// If the type is not updateable then skip it
							if (hr == DB_E_BADTYPE)
								continue;
							else
							{
								// Column name
								wcscat(pwszSQLText,rCol.GetColName());

								// "=" or "like"
								wcscat(pwszSQLText, szPredicate);
								
								if(rCol.GetPrefix())
									wcscat(pwszSQLText, rCol.GetPrefix());
								
								// "text" data type can not have more than 255 characters
								// in the where clause of the SQL Statement
								if ((DBTYPE_IUNKNOWN == rCol.GetProviderType()) && (rCol.GetColumnSize() > 255))
								{
									pwszMakeData[254] = L'%';
									pwszMakeData[255] = L'\0';
								}
								
								wcscat(pwszSQLText, pwszMakeData);
								
								if(rCol.GetSuffix())
									wcscat(pwszSQLText, rCol.GetSuffix());
							}
						}
					}
					else
						// Column name
						wcscat(pwszSQLText,rCol.GetColName());
				}
			}
			else if (eColsInList == SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST)
			{
				// Use equals if we can for parameterized search and updateable. criteria
				// We don't want long columns in where clause as most of the drivers do not support anyway.
				if ( rCol.GetIsLong() )
					continue;

				if ((rCol.GetUpdateable() && ((rCol.GetSearchable() == DB_SEARCHABLE) ||
						(rCol.GetSearchable() == DB_ALL_EXCEPT_LIKE))))
				{
					// = ?
					if (fAddParams)
						wcscpy(szPredicate, wszEQUAL_TO_QUESTION_MARK);
					else
						wcscpy(szPredicate, wszEQUALS);
				}
				else
					// Use Like if we can, otherwise we'll skip this column
					if ((rCol.GetSearchable() == DB_LIKE_ONLY ) && rCol.GetUpdateable())
					{
						// LIKE ?
						if (fAddParams)
							wcscpy(szPredicate, wszLIKE_QUESTION_MARK);
						else
							wcscpy(szPredicate, wszLIKE);
					}
					else						
						// We can't add this column to the list
						// since its not searchable, so skip it and move on
						continue;

				if (fAddParams)
				{
					// Column name
					// LIKE ? or = ?
					wcscat(pwszSQLText,rCol.GetColName());
					wcscat(pwszSQLText, szPredicate);
				}
				else
				{
					// Here we are creating the where part of select statement
					if (fAddData)
					{
						hr = rCol.MakeData(pwszMakeData, ulRowNumber, eValue, GetNull(), NULL, GetIndexColumn());

						// If MakeData put a NULL into this row/column in the table
						if(hr == S_FALSE)
						{
							// Column name
							wcscat(pwszSQLText,rCol.GetColName());
							wcscat(pwszSQLText, wszISNULL);
						}
						else
						{
							// If the type is not updateable then skip it
							if(hr == DB_E_BADTYPE || !rCol.GetUpdateable())
							{
								continue;
							}
							else
							{
								LPWSTR pwszLiteral = NULL;

								// Column name
								wcscat(pwszSQLText,rCol.GetColName());

								// "=" or "like"
								wcscat(pwszSQLText, szPredicate);

								CHECK(GetLiteralAndValue(rCol, &pwszLiteral, ulRowNumber, rCol.GetColNum()), S_OK);

								if (pwszLiteral)
									wcscat(pwszSQLText, pwszLiteral);

								SAFE_FREE(pwszLiteral);
							}
						}
					}
					else
						// Column name
						wcscat(pwszSQLText,rCol.GetColName());
				}
			}
			else
			{
				// If we only want updateable cols, 
				// skip this col if its not updateable
				if (eColsInList == UPDATEABLE_COLS_IN_LIST)
					if (!rCol.GetUpdateable())
						continue;

				// Now add column name
				if(rCol.GetColName())
					wcscat(pwszSQLText,rCol.GetColName());

				// making array of column names
				if (prgColumnNames)
					rgColNames[inc] = rCol.GetColName();
			}

			// Add AND or Comma
			if (fAddData || fAddParams)
				wcscat(pwszSQLText,wszAND);
			else
				wcscat(pwszSQLText,wszCOMMA);

			// Put column number in array
			// Increment number of columns in list
			(*prgColumns)[inc++]=rCol.GetColNum();			
			(*pcColumns)++;	
		}
		else // reverse col list
		{						
			pwszTempText[0]=L'\0';

			// Check searchability if asked for
			if (eColsInList == SEARCHABLE_COLS_IN_LIST)
			{
				// Use equals if we can for parameterized search criteria
				if ((rCol.GetSearchable() == DB_SEARCHABLE) ||
						(rCol.GetSearchable() == DB_ALL_EXCEPT_LIKE))
				{
					// Column name 
					wcscat(pwszTempText,rCol.GetColName());
					
					// = ?
					if (fAddParams)
						wcscat(pwszTempText, wszEQUAL_TO_QUESTION_MARK);
				}
				else

					// Use Like if we can, otherwise we'll skip this column
					if (rCol.GetSearchable() == DB_LIKE_ONLY)						
					{
						// Column name
						wcscat(pwszTempText,rCol.GetColName());
					
						// LIKE ?
						if (fAddParams)
							wcscat(pwszTempText, wszLIKE_QUESTION_MARK);
					}
					else						
						// We can't add this column to the list
						// since its not searchable, so skip it and move on
						continue;						
			}
			else
			{
				// If we only want updateable cols, 
				// skip this col if its not updateable
				if (eColsInList == UPDATEABLE_COLS_IN_LIST)
					if (!rCol.GetUpdateable())
						continue;

				// Now add column name
				if(rCol.GetColName())
					wcscat(pwszTempText,rCol.GetColName());

				// making array of column names
				if (prgColumnNames)
					rgColNames[inc] = rCol.GetColName();
			}

			// Add AND or Comma
			if (fAddData || fAddParams)
				wcscat(pwszTempText,wszAND);
			else
				wcscat(pwszTempText,wszCOMMA);

			// Now add this col to front of col list
			wcscat(pwszTempText,pwszSQLText);
			wcscpy(pwszSQLText,pwszTempText);

			// Put column number in array
			// Increment number of columns in list
			(*prgColumns)[inc--]=rCol.GetColNum();
			(*pcColumns)++;							
		}
	}
	
	
	// Remove last 'AND'
	// else Remove last comma if we have a col list string
	if (wcslen(pwszSQLText) >= 4)
	{
		if (fAddData || fAddParams)
			wcsncpy(pwszSQLText + (wcslen(pwszSQLText) - 4), L"\0", 1);
		else
			wcsncpy(pwszSQLText + (wcslen(pwszSQLText) - 1), L"\0", 1);
	}

	// Allocate memory for the string we're going to return to the caller
	*ppwszColList = wcsDuplicate(pwszSQLText);
	if (!*ppwszColList)
		goto CLEANUP;

	if(rgColNames)
		*prgColumnNames=rgColNames;

	hr = S_OK;

CLEANUP:

	if (hr==E_FAIL)
	{
		PROVIDER_FREE(*prgColumns);
		PROVIDER_FREE(*ppwszColList);
		PROVIDER_FREE(rgColNames);
	}
	
	PROVIDER_FREE(pwszSQLText);
	PROVIDER_FREE(pwszTempText);
	PROVIDER_FREE(pwszMakeData);

	return hr;
}
//---------------------------------------------------------------------------
// CTable::AllColumnsForUse
//	
// CTable				|
// AllColumnsForUse	|
// Sets all columns for use in where clause
//
// @mfunc	AllColumnsForUse
// @rdesc HRESULT indicating success or failure.
// 	@flag S_OK   | Function ran without problem.
// 	@flag E_FAIL    | Function ran with problems.
//
//---------------------------------------------------------------------------
HRESULT CTable::AllColumnsForUse(
	BOOL fValue  // @parm [IN] TRUE sets all cols, FALSE clears all cols (Default = FALSE)
)
{
	POSITION 	pos;			// position in m_ColList

	// While not at the end of m_ColList
	pos = m_ColList.GetHeadPosition();
	while(pos)
	{
		// Go to next position
		CCol& rCol = m_ColList.GetNext(pos);

		// Set m_fUseInSQL to fValue
		rCol.SetUseInSQL(fValue);
	}

	return S_OK;
}

//---------------------------------------------------------------------------
// CTable::AddColumn
//	
// CTable			|
// AddColumn
// Adds a column from the table.
//
// @mfunc	AddColumn
// @rdesc Adds a column to the table.
//
//---------------------------------------------------------------------------
HRESULT CTable::AddColumn(CCol& rCol) 
{ 
	HRESULT				hr					= E_FAIL;
	ITableDefinition	*pITableDefinition	= NULL;
	DBCOLUMNDESC		*pColumnDesc		= NULL;
	WCHAR				*pwszSQLText		= NULL;

	//See if ITableDefinition is supported...
	VerifyInterface(m_pIOpenRowset, IID_ITableDefinition, SESSION_INTERFACE, (IUnknown**)&pITableDefinition);
	
	if(GetModInfo()->IsUsingITableDefinition() && pITableDefinition)
	{
		//ITableDefinition::AddColumn
		SAFE_ALLOC(pColumnDesc, DBCOLUMNDESC, 1);
		BuildColumnDesc(pColumnDesc, rCol);
		QTESTC_(hr = pITableDefinition->AddColumn(&m_TableID, pColumnDesc, NULL),S_OK);
	}
	else if(GetCommandSupOnCTable())
	{
		//ICommand::Execute ("Alter Table ...")
		QTESTC_(hr = ExecuteCommand(ALTER_TABLE_ADD_EX, IID_NULL),S_OK);
	}
	else
	{
		//Both ITableDefinition and ICommand were not supported...
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	// Get new info about columns (this automatically add our new column if returned in ColInfo)
	// (e.g. the ordinals could have been changed)
	QTESTC_(hr = GetTableColumnInfo(&m_TableID),S_OK);
	
CLEANUP:
	SAFE_RELEASE(pITableDefinition);
	ReleaseColumnDesc(pColumnDesc, 1);
	return hr; 
}

//---------------------------------------------------------------------------
// CTable::DropColumn
//	
// CTable			|
// DropColumn
// Removes a column from the table.
//
// @mfunc	DropColumn
// @rdesc Removes a column from the table.
//
//---------------------------------------------------------------------------
HRESULT CTable::DropColumn(CCol& rCol) 
{ 
	POSITION			pos;				// Position in data types list
	POSITION			posSave;			// Position in data types list
	HRESULT				hr					= S_OK;
	ITableDefinition	*pITableDefinition	= NULL;
	DBID				*pColumnID			= rCol.GetColID();
	DB_LORDINAL			rgOrdinal[1]		= {rCol.GetColNum()};
	DBORDINAL			cOrdinal			= 1;
	DB_LORDINAL			*pOrdinal			= rgOrdinal;
	//If no table name or user supplied table return S_OK
	if(!GetTableName() || (GetModInfo()->GetTableName()))
		goto CLEANUP;

	//Check to see if the Provider is readonly
	if(m_fProviderReadOnly)
		goto CLEANUP;

	//If commands are not supported try using ITableDefintion
	if(!GetCommandSupOnCTable() || GetModInfo()->IsUsingITableDefinition())
	{
		// ITableDefinition must be present
		if(VerifyInterface(m_pIOpenRowset, IID_ITableDefinition, SESSION_INTERFACE, (IUnknown**)&pITableDefinition))
		{
			// Return code is checked below
			hr = pITableDefinition->DropColumn(&m_TableID, pColumnID);
		}
		else
		{
			//Unable to drop table, since both Commands and ITableDefinition
			//are not supported
			hr = E_FAIL;
		}

		goto CLEANUP;
	}

	// try to use commands
	hr = ExecuteCommand(ALTER_TABLE_DROP_COLUMN, IID_NULL, NULL, NULL, &cOrdinal, &pOrdinal);

CLEANUP:
	// Free memory
	SAFE_RELEASE(pITableDefinition);

	PROVIDER_FREE(m_pwszIndexName);

	if (!SUCCEEDED(hr))
		return hr;

	// drop the column from the list of columns
	pos = m_ColList.GetHeadPosition();
	while(pos) 
	{		
		// Save the position for deleting and then get the next element
		posSave = pos;
		CCol& rCol2 = m_ColList.GetNext(pos);
		
		if((DBORDINAL)rgOrdinal[0] < rCol.GetColNum())
			rCol2.SetColNum(rCol.GetColNum()-1);
		
		// for the time being compare column names
		// when operator == is available, will go for an == on columns
		if (CompareDBID(*pColumnID, *rCol2.GetColID()))
			m_ColList.RemoveAt(posSave);
	}
		
	return hr;
} //CTable::DropColumn


//---------------------------------------------------------------------------
// CTable::DropColumn
//	
// CTable			|
// DropColumn
// Removes a column from the table.
//
// @mfunc	DropColumn
// @rdesc Removes a column from the table.
//
//---------------------------------------------------------------------------
HRESULT CTable::DropColumn(DBORDINAL nOrdinalPos) 
{
	// get the column at with the ordinal position nOrdinalPos
	CCol& rCol = GetColInfoForUpdate(nOrdinalPos);
	
	// check the column was found
	if (nOrdinalPos != rCol.GetColNum())
		return E_FAIL;
	return DropColumn(rCol);
} //CTable::DropColumn


//---------------------------------------------------------------------------
// CTable::CountColumnsOnTable
//	
// CTable			|
// CountColumnsOnTable|
// Counts the columns in the table.
//
// @mfunc	CountColumnsOnTable
// @rdesc Total count of columns
//
//---------------------------------------------------------------------------
DBORDINAL CTable::CountColumnsOnTable(void)
{
	return (DBORDINAL) m_ColList.GetCount();  
}

//---------------------------------------------------------------------------
// CTable::CountRowsOnTable
//	
// CTable			|
// CountRowsOnTable |
// Counts the rows in the table by seeing how many GetRows it
// goes thru. Yes, this is expensive for large databases. If table name
// is not set, return E_FAIL. Alloc (pwszSQLText).
//
// @mfunc	CountRowsOnTable
// @rdesc Total rows found in the table
//
//---------------------------------------------------------------------------
DBCOUNTITEM CTable::CountRowsOnTable(IRowset *pIRowsetIn) 
{
	HRESULT 		hr				= 0;		// return result
	IRowset *		pIRowset		= NULL;		// rowset
	DBCOUNTITEM		cRowsObtained	= 0;		// count of rows
	HROW *			rghRows			= NULL;		// range of row handles
	DBROWCOUNT		cRowsAffected	= 0;		// count of rows

	// Init row count
	m_ulRows = 0;
	
	//If we passed rowset pointer in, count the rows in rowset
	if(pIRowsetIn)
	{
		pIRowset = pIRowsetIn;
		SAFE_ADDREF(pIRowset);

		//The rowset passed in may not have the NFP at the begining...
		hr = pIRowset->RestartPosition(NULL);
	}
	else
	{
		TESTC_(hr = CreateRowset(USE_OPENROWSET, IID_IRowset, 0, NULL, (IUnknown **)&pIRowset),S_OK);
	}
	
	//Loop over all rows...
	//NOTE: we don't use while(SUCCEEDED) as this will mask unexpected errors, and more than
	//likely not show any error, but just return 0 rows, which passes numerous tests!
	while(TRUE)
	{
		//IRowset->GetNextRows
		hr = pIRowset->GetNextRows(
				0,					// no chapters
				0,					// don't skip any rows
				1,					// number of rows to fetch
				&cRowsObtained,		// number of rows returned, should be 10	
				&rghRows);			// array of handles of rows, should be 10 element in array
		
		//Valid return codes (for success cases)
		TEST2C_(hr, S_OK, DB_S_ENDOFROWSET);

		//At end of rowset?
		if(hr == DB_S_ENDOFROWSET || cRowsObtained == 0)
		{
			TESTC_(hr, DB_S_ENDOFROWSET);
			TESTC(cRowsObtained == 0);
			goto CLEANUP;
		}
		
		TESTC(cRowsObtained == 1);
		m_ulRows++;
			
		//Release this row...
		TESTC_(hr = pIRowset->ReleaseRows(cRowsObtained,rghRows,NULL,NULL,NULL),S_OK);
	}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(rghRows);
	return m_ulRows;
}


/////////////////////////////////////////////////////////////////
// DBLITERALINFO* CTable::GetLiteralInfo
//
/////////////////////////////////////////////////////////////////
DBLITERALINFO* CTable::GetLiteralInfo(DBLITERAL dwLiteral)
{
	static DBLITERALINFO sLiteralInfo = { 0,0,0,0,0,0};
	
	//IDBInfo is an optional interface...
	//Just return a pointer (so not all method have to check the pointer),
	//But fSupported == FALSE...
	if(m_cLiteralInfo == 0 || dwLiteral>=m_cLiteralInfo)
		return &sLiteralInfo;

	//Our cahce of Literals is in the exact orders as the Literal value,
	//so you can directly index into, without have to serach for the id...
	return &m_rgLiteralInfo[dwLiteral];	
}


/////////////////////////////////////////////////////////////////
// HRESULT CTable::GetQuotedName
//
//	fFromCatalog - This flag controls whether the function assumes
//		the names are initially from an unquoted source (CTable)
//		or from a properly quoted source (catalog functions).  Once
//		privlib is changed to contain a properly quoted name then
//		this flag is no longer needed.
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::GetQuotedName(WCHAR* pwszName, WCHAR** ppwszQuoted, BOOL fFromCatalog)
{
	VARIANT		Variant;

	//no-op
	if(pwszName == NULL || ppwszQuoted == NULL)
		return E_INVALIDARG;

	WCHAR* pwszQuoted = NULL;
	*ppwszQuoted = NULL;

	DBLITERALINFO* pPrefixLiteral	= GetLiteralInfo(DBLITERAL_QUOTE_PREFIX);
	DBLITERALINFO* pSuffixLiteral	= GetLiteralInfo(DBLITERAL_QUOTE_SUFFIX);
	
	//Some providers may not have support for Suffix yet...
	if(!pSuffixLiteral->fSupported)
		pSuffixLiteral = pPrefixLiteral;

	size_t cPrefixLen	= pPrefixLiteral->fSupported ? wcslen(pPrefixLiteral->pwszLiteralValue) : 0;
	size_t cSuffixLen	= pSuffixLiteral->fSupported ? wcslen(pSuffixLiteral->pwszLiteralValue) : 0;
	size_t cPeriodLen	= 1;

	//Allocate enough room for the resultant quoted string...
	SAFE_ALLOC(pwszQuoted, WCHAR, cPrefixLen + wcslen(pwszName) + cSuffixLen +1);
	pwszQuoted[0] = L'\0';

	//Put on Prefix
	if(pPrefixLiteral->fSupported)
		wcscat(pwszQuoted, pPrefixLiteral->pwszLiteralValue);

	//Copy Name
	wcscat(pwszQuoted, pwszName);
	VariantInit(&Variant);
	if (	GetProperty(DBPROP_QUOTEDIDENTIFIERCASE, 
			DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &Variant)
		&&	(DBPROPVAL_IC_SENSITIVE == Variant.lVal))
	{
		VariantInit(&Variant);
		GetProperty(DBPROP_IDENTIFIERCASE, 
			DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &Variant);
		if (!fFromCatalog)
		{
			switch(Variant.lVal)
			{
				case DBPROPVAL_IC_UPPER:
					_wcsupr(pwszQuoted);
					break;
				case DBPROPVAL_IC_LOWER:
					_wcslwr(pwszQuoted);
					break;
			}
		}
	}

	//Put on Suffix
	if(pSuffixLiteral->fSupported)
		wcscat(pwszQuoted, pSuffixLiteral->pwszLiteralValue);


CLEANUP:
	*ppwszQuoted = pwszQuoted;
	return S_OK;
}


/////////////////////////////////////////////////////////////////
// HRESULT CTable::GetQualifiedName
//
//	fFromCatalog - This flag controls whether the function assumes
//		the names are initially from an unquoted source (CTable)
//		or from a properly quoted source (catalog functions).  Once
//		privlib is changed to contain a properly quoted name then
//		this flag is no longer needed.
//
/////////////////////////////////////////////////////////////////
HRESULT CTable::GetQualifiedName(WCHAR* pwszCatalogName, WCHAR* pwszSchemaName, WCHAR* pwszTableName, WCHAR** ppwszQualifiedName,
	BOOL fFromCatalog)
{
	//no-op
	if(pwszTableName == NULL || ppwszQualifiedName == NULL)
		return E_INVALIDARG;

	WCHAR* pwszQualifiedName = NULL;
	WCHAR* pwszItr = NULL;
	*ppwszQualifiedName = NULL;

	DBLITERALINFO* pCatalogLiteral	= GetLiteralInfo(DBLITERAL_CATALOG_SEPARATOR);
	
	WCHAR* pwszQuotedCatalog = NULL;
	WCHAR* pwszQuotedSchema  = NULL;
	WCHAR* pwszQuotedTable	 = NULL;
	GetQuotedName(pwszCatalogName, &pwszQuotedCatalog, fFromCatalog);
	GetQuotedName(pwszSchemaName, &pwszQuotedSchema, fFromCatalog);
	GetQuotedName(pwszTableName, &pwszQuotedTable, fFromCatalog);

	size_t cSepLen		= pCatalogLiteral->fSupported ? wcslen(pCatalogLiteral->pwszLiteralValue) : 0;
	size_t cPeriodLen	= 1;

	//Allocate enough room (max possible) for the resultant qualified string...
	size_t cTotalLen = cSepLen + cPeriodLen + wcslen(pwszQuotedTable);
	if(pwszQuotedCatalog)
		cTotalLen += wcslen(pwszQuotedCatalog);
	if(pwszQuotedSchema)
		cTotalLen += wcslen(pwszQuotedSchema);
	SAFE_ALLOC(pwszQualifiedName, WCHAR, cTotalLen+1);
	pwszQualifiedName[0] = L'\0';

	//QualifiedTableName syntax
	// #1.  TableName
	// #2.  Owner.TableName (always a ".")
	// #3.  Catalog[CatalogSeperator]TableName
	// #4.  Catalog[CatalogSeperator]Owner.TableName

	if(pwszQuotedCatalog)
	{
		wcscat(pwszQualifiedName, pwszQuotedCatalog);
		if(pCatalogLiteral->fSupported)
			wcscat(pwszQualifiedName, pCatalogLiteral->pwszLiteralValue);
	}
	if(pwszQuotedSchema)
	{
		wcscat(pwszQualifiedName, pwszQuotedSchema);
		wcscat(pwszQualifiedName, L".");
	}
	if(pwszQuotedTable)
	{
		wcscat(pwszQualifiedName, pwszQuotedTable);
	}


CLEANUP:
	*ppwszQualifiedName = pwszQualifiedName;
	SAFE_FREE(pwszQuotedCatalog);
	SAFE_FREE(pwszQuotedSchema);
	SAFE_FREE(pwszQuotedTable);
	return S_OK;
}


//---------------------------------------------------------------------------
// CTable					|
// SetupLiteralInfo
// Gets information about Literals used in DDL.
//
// @mfunc	SetupLiteralInfo
// @rdesc HRESULT indicating success or failure. 
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::SetupLiteralInfo()
{
	HRESULT		hr = S_OK;					// Result code
	IDBInfo*	pIDBInfo = NULL;			// IDBInfo interface pointer

	//Obtain IDBInfo interface
	if(!VerifyInterface(m_pIDBInitialize, IID_IDBInfo, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInfo))
		return E_NOINTERFACE;

	//What we want to do is cache all literals in an array, ordered exacly in the 
	//same order as the index so we can index into this array directly
	//IE:  rgLiterals[DBLITERAL_QUOTE_SUFFIX] = QuoteSuffixLiteral...

	const ULONG cLiterals = DBLITERAL_QUOTE_SUFFIX+1;		//Maximum Literals
	DBLITERAL rgLiterals[cLiterals];

	//Fill in the array in order, so we can index in order...
	for(ULONG i=0; i<cLiterals; i++)
		rgLiterals[i] = i;

	//IDBInfo::GetLiteralInfo
	SAFE_FREE(m_rgLiteralInfo);
	SAFE_FREE(m_pwszLiteralInfoBuffer);
	hr = pIDBInfo->GetLiteralInfo(cLiterals, rgLiterals, &m_cLiteralInfo, &m_rgLiteralInfo, &m_pwszLiteralInfoBuffer);

//CLEANUP:
	SAFE_RELEASE(pIDBInfo);
	return hr;
}

//---------------------------------------------------------------------------
// CTable::DoesTableExist
//
// CTable			|
// DoesTableExist	|
// Make sure table is in Data Source. Return E_FAIL if table name is empty.
// If table is found, set fExists to true. If function runs correctly
// but doesn't find the table name, function will return S_OK, but fExists
// will be FALSE. 
//
// @mfunc	DoesTableExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | No table name
//  @flag E_OUTOFMEMORY | Ran out of memory...
//
//---------------------------------------------------------------------------
HRESULT CTable::DoesTableExist
(
	WCHAR * 	pwszTableName,	// @parm [IN]  Table name to check (Default = NULL)
	BOOL *		pfExists		// @parm [OUT] TRUE if table exists (Default = NULL)
)
{
	DBID TableID;
	TableID.eKind		   = DBKIND_NAME;
	TableID.uName.pwszName = pwszTableName;
	
	//Delegate
	return DoesTableExist(&TableID, pfExists);
}


//---------------------------------------------------------------------------
// CTable::DoesTableExist
//
// CTable			|
// DoesTableExist	|
// Make sure table is in Data Source, E_FAIL if pointer is NULL or if DBID is empty.
// If table is found set fExists to true. If function runs correctly
// but doesn't find the table name, function will return S_OK, but fExists
// will be FALSE. 
//
// @mfunc	DoesTableExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | No table ID
//  @flag E_OUTOFMEMORY | Ran out of memory...
//
//---------------------------------------------------------------------------
HRESULT CTable::DoesTableExist
(
	DBID * 		pTableID,	// @parm [IN]  Pointer to Table ID to check (Default = NULL)
	BOOL *		pfExists		// @parm [OUT] TRUE if table exists (Default = NULL)
)
{
	IRowset*	pIRowset = NULL;		// Rowset
	HRESULT 	hr = S_OK;				// result code

 	//Initialize to FALSE until proven otherwise
	ASSERT(pfExists);
	*pfExists = FALSE;
	
	//Generically Obtain the rowset...
	QTESTC_(hr = CreateRowset(USE_OPENROWSET, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset, pTableID),S_OK);
	*pfExists = TRUE;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	if(hr==DB_E_NOTABLE)
		hr = S_OK;
	return hr;
}

//---------------------------------------------------------------------------
// CTable::DropIndex 
//	
// CTable			|
// DropIndex		|
// Drops the index on this table, should only be one
//
// @mfunc	DropIndex
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::DropIndex(void) 
{
	WCHAR * pwszSQLText=NULL;
	HRESULT hr=S_OK;

	// Check to see if the Provider is readonly
	if( (m_fProviderReadOnly) || (!m_pwszIndexName) )
		return hr;

	ASSERT(wcslen(GetTableName()));
	ASSERT(wcslen(m_pwszIndexName));
	
	// "Drop Index %s.%s";
	pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
					wcslen(wszDROP_INDEX)+
					wcslen(GetTableName()) +
					wcslen(m_pwszIndexName))) +
					sizeof(WCHAR));

	swprintf(pwszSQLText, wszDROP_INDEX, GetTableName(), m_pwszIndexName);
	PRVTRACE(L"%sDropIndex):%s\n",wszPRIVLIBT,pwszSQLText);

	hr=BuildCommand(pwszSQLText,		//SQL text to execute
						IID_NULL,		//This won't be used
						EXECUTE_ALWAYS,	//Should always execute fine
						0, NULL,		//Set no properties
						NULL,			//No parmeters									
						NULL, 			//No RowsAffected info
						NULL,			//No rowset needs to be allocated
						&m_pICommand);	//Use this object's command to execute on

	PROVIDER_FREE(pwszSQLText);

	if (FAILED(hr))
	{
		PRVTRACE(L"%sDropIndex):%s\n",wszPRIVLIBT,wszFUNCFAIL);
		return hr;
	}
	else
	{
		PROVIDER_FREE(m_pwszIndexName);
		return S_OK;
	}
}

//---------------------------------------------------------------------------
// CTable::DropTable
//	
// CTable			|
// DropTable		|
// Drops this table.
//
// @mfunc	DropTable
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::DropTable(BOOL fDropAlways)
{
	WCHAR *		pwszSQLText=NULL;
	POSITION	pos;				// Position in data types list
	POSITION	posSave;				// Position in data types list

	HRESULT		hr = S_OK;
	ITableDefinition* pITableDefinition = NULL;

	//If no table name, nothing to drop
	if(!GetTableName())
		goto CLEANUP;

	//If using a "static" INI File provided table then we can't drop it.
	//Since the user could have opened it numerous times.  
	//NOTE: If it is desired to drop an INI file table always (ie: automation), 
	//then ReleaseModInfo will do this functionality on shutdown if the init string keyword is specified.
	if(GetModInfo()->GetTableName() && !fDropAlways)
		goto CLEANUP;

	//Check to see if the Provider is readonly
	if(m_fProviderReadOnly)
		goto CLEANUP;

	//Drop the view on the table
	if (GetCommandSupOnCTable())
		DropView();

	// ITableDefinition must be present
	if(!VerifyInterface(m_pIOpenRowset, IID_ITableDefinition, SESSION_INTERFACE, (IUnknown**)&pITableDefinition))
		hr = E_FAIL;

	//If commands are not supported try using ITableDefintion
	if((GetModInfo()->IsUsingITableDefinition()) && (hr == S_OK))
	{
		// ITableDefinition must be present
		hr = pITableDefinition->DropTable(&m_TableID);
		goto CLEANUP;
	}

	if (!GetCommandSupOnCTable())
	{
		//Unable to drop table, ITableDefintion and Commands are not supported
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
					wcslen(wszDROP_TABLE) +
					wcslen(GetTableName()))) +
					sizeof(WCHAR));

	//"drop table tablename"
	swprintf(pwszSQLText, wszDROP_TABLE, GetTableName());
	PRVTRACE(L"%sDropTable):%s\n",wszPRIVLIBT,pwszSQLText);

	//Execute statement
	hr=	BuildCommand(pwszSQLText,	//SQL text to execute
					IID_NULL,		//This won't be used
					EXECUTE_ALWAYS,	//Should always execute fine
					0, NULL,		//Set no properties
					NULL,			//No parmeters									
					NULL, 			//No RowsAffected info
					NULL,			//No rowset needs to be allocated
					&m_pICommand);	//Use this object's command to execute on

CLEANUP:
	
	//Free memory
	SAFE_RELEASE(pITableDefinition);

	PROVIDER_FREE(pwszSQLText);
	PROVIDER_FREE(m_pwszIndexName);

	SetTableName(NULL);

	m_ulRows	= 0;
	m_ulNextRow = 1;
	m_ulIndex	= 0;

	//Get top to list
	pos=m_ColList.GetHeadPosition();
		
	//While not at end of list
	while (pos!=NULL) 
	{		
		//Save the position for deleting and then get the next element
		posSave = pos;
		m_ColList.GetNext(pos);
		
		//Put column name in list
		m_ColList.RemoveAt(posSave);
	}
	m_ColList.RemoveAll();
		
	return hr;
}

//---------------------------------------------------------------------------
// CTable::DropTable
//	
// CTable			|
// DropTable		|
// Drops this table.
//
// @mfunc	DropTable
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::DropTable(DBID *pTableID)
{
	POSITION	pos;				// Position in data types list
	POSITION	posSave;				// Position in data types list

	HRESULT		hr = S_OK;
	ITableDefinition* pITableDefinition = NULL;

	//If no table name or user supplied table return S_OK
	if(!pTableID)
		return S_OK;

	//Check to see if the Provider is readonly
	if(m_fProviderReadOnly)
		goto CLEANUP;

	//If commands are not supported try using ITableDefintion
	if (	GetModInfo()->IsUsingITableDefinition() 
		&&	VerifyInterface(m_pIOpenRowset, IID_ITableDefinition, SESSION_INTERFACE, 
			(IUnknown**)&pITableDefinition))
	{
		// ITableDefinition must be present
		hr = pITableDefinition->DropTable(pTableID);
	}
	else if (GetCommandSupOnCTable() && (DBKIND_NAME == pTableID->eKind))
	{
		//ICommand::Execute ("Drop Table ...")
		if (!GetTableName())
			SetTableName(pTableID->uName.pwszName); // just to avoid an assert in CTable::CreateSQLStmt
		QTESTC_(hr = ExecuteCommand(DROP_TABLE, IID_NULL, pTableID->uName.pwszName),S_OK);
	}
	else
	{
		//Unable to drop table, since both Commands and ITableDefinition
		//are not supported
		hr = E_FAIL;
	}

CLEANUP:
	// Free memory
	SAFE_RELEASE(pITableDefinition);

	SetTableName(NULL);

	m_ulRows	= 0;
	m_ulNextRow = 1;
	m_ulIndex	= 0;

	// Get top to list
	pos=m_ColList.GetHeadPosition();
		
	// While not at end of list
	while (pos!=NULL) 
	{		
		// Save the position for deleting and then get the next element
		posSave = pos;
		m_ColList.GetNext(pos);
		
		// Put column name in list
		m_ColList.RemoveAt(posSave);
	}
	m_ColList.RemoveAll();
		
	return hr;
}

//---------------------------------------------------------------------------
// CTable::DropView
//	
// CTable			|
// DropView		|
// Drops the view over the CTable.
//
// @mfunc	DropView
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::DropView(void)
{
	WCHAR *		pwszSQLText=NULL;
	HRESULT		hr=0;

	// If no view name return S_OK
	if (!m_pwszViewName)
		return S_OK;
	
	pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
					wcslen(wszDROP_VIEW) +
					wcslen(m_pwszViewName))) +
					sizeof(WCHAR));

	// " drop view viewname"
	swprintf(pwszSQLText, wszDROP_VIEW, m_pwszViewName);
	PRVTRACE(L"%sDropView):%s\n",wszPRIVLIBT,pwszSQLText);

	// Execute statement
	hr=	BuildCommand(pwszSQLText,	//SQL text to execute
					IID_NULL,		//This won't be used
					EXECUTE_ALWAYS,	//Should always execute fine
					0, NULL,		//Set no properties
					NULL,			//No parmeters									
					NULL, 			//No RowsAffected info
					NULL,			//No rowset needs to be allocated
					&m_pICommand);	//Use this object's command to execute on

	if (FAILED(hr))
	{
		PROVIDER_FREE(pwszSQLText);
		pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
						wcslen(wszDROP_TABLE) +
						wcslen(m_pwszViewName))) +
						sizeof(WCHAR));

		// " drop table viewname"
		swprintf(pwszSQLText, wszDROP_TABLE, m_pwszViewName);
		PRVTRACE(L"%sDropView):%s\n",wszPRIVLIBT,pwszSQLText);

		// Execute statement
		hr=	BuildCommand(pwszSQLText,	//SQL text to execute
						IID_NULL,		//This won't be used
						EXECUTE_ALWAYS,	//Should always execute fine
						0, NULL,		//Set no properties
						NULL,			//No parmeters									
						NULL, 			//No RowsAffected info
						NULL,			//No rowset needs to be allocated
						&m_pICommand);	//Use this object's command to execute on
	}

	// Free memory
	PROVIDER_FREE(pwszSQLText);
	PROVIDER_FREE(m_pwszViewName);
	return hr;
}

//---------------------------------------------------------------------------
// CTable::GetColsUsed 
//	
// CTable			|
// GetColsUsed		|
// Returns the list of column numbers that correspond to columns
// marked for selection criteria.
//
// @mfunc GetColsUsed
//
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function rans without problem
//  @flag E_FAIL    | m_ColList is empty
//
//---------------------------------------------------------------------------
HRESULT CTable::GetColsUsed(
	CList <DBORDINAL,DBORDINAL> &rColNumList	// @parm [OUT] list of column numbers marked
)
{
	// Make sure m_ColList is NOT empty
	if (m_ColList.IsEmpty())
	{
		PRVTRACE(L"%sGetColsUsed):m_ColList is empty,%s\n",wszPRIVLIBT,wszFUNCFAIL);
		return E_FAIL;
	}

	// Run Thru List Getting Cols
	// Make sure ColNumList is empty
	rColNumList.RemoveAll();

	// Get position of the head
	POSITION posm_ColList = m_ColList.GetHeadPosition();
	while(posm_ColList!=NULL)
	{
		// Move to next element in list
		CCol& rCol = m_ColList.GetNext(posm_ColList);

		// If it is true, then add current col# to ColNumList
		// Add element to list of marked columns and put value in that element
		if(rCol.GetUseInSQL())
			rColNumList.AddTail(rCol.GetColNum());
	}				   

	// Make sure m_ColList is NOT empty
	if (rColNumList.IsEmpty())
	{
		PRVTRACE(L"%sGetColsUsed):rColNumList is empty,%s\n",wszPRIVLIBT,wszFUNCFAIL);
		return E_FAIL;
	}
	else
		return S_OK;
}

//---------------------------------------------------------------------------
// CTable::GetLiteralAndValue
//	
// CTable				|
// GetLiteralAndValue	|
// Builds string containing literal prefix, MakeData, literal suffix.
// Client must free (*ppwszData).
//
// @mfunc GetLiteralAndValue
//
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   			| Function rans without problem
//  @flag E_FAIL			| Function ran with problems
//  @flag DB_E_NOTUPDATED	| Field can not be updated
//
//---------------------------------------------------------------------------
HRESULT CTable::GetLiteralAndValue(
	CCol &		col,				// @parm [IN]  Col to grab prefix and suffix from
	WCHAR ** 	ppwszData,			// @parm [OUT] prefix, data, suffix:(like 'zbc' or #222) (Default = NULL)
	DBCOUNTITEM	ulRow,				// @parm [IN]  row number to pass to MakeData (Default = 1)
	DBORDINAL	iOrdinal,			// @parm [IN]  column number to pass to MakeData (Default = 1)
	EVALUE 		eValue,				// @parm [IN]  Type of MakeData,PRIMARY or SECONDARY. (Default = PRIMARY)
	BOOL		fColData			// @parm [IN]  Whether to use col to make the data (Default = FALSE)
)
{
	HRESULT		hr = E_FAIL;
	WCHAR *		pwszPrefix = NULL;
	WCHAR *		pwszSuffix = NULL;
	WCHAR *		pwszMakeData = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) * MAXDATALEN);
	
	if (!pwszMakeData)
		return E_OUTOFMEMORY;
	
	pwszMakeData[0]=L'\0';
	
	// MakeData 
	if( fColData )
		hr=col.MakeData(pwszMakeData,ulRow,eValue);
	else
		hr=MakeData(pwszMakeData,ulRow,iOrdinal,eValue);

	// MakeData did not return S_OK
	if (FAILED(hr))
	{
		PROVIDER_FREE(pwszMakeData);
	}

	// Column value cannot be updated
	if (hr == DB_E_BADTYPE)
		return hr;

	// Column value needs to be NULL
	if ((hr)==S_FALSE)		
	{
		size_t iSize = 0;

		iSize = wcslen(wszSPACE);
		iSize *= 2;
		iSize += wcslen(wszNULL);
		iSize *= sizeof(WCHAR);
		iSize += sizeof(WCHAR);
		
		WCHAR * pwszFinalData = (WCHAR *) PROVIDER_ALLOC(iSize);
		pwszFinalData[0] = L'\0';
		wcscat(pwszFinalData,L"NULL");		
		(*ppwszData) = (WCHAR *) pwszFinalData;
		
		PROVIDER_FREE(pwszMakeData);
		return S_OK;
	}
	
	// Build sql statement 
	if (SUCCEEDED(hr))
	{
		if(wcslen(pwszMakeData) > 0)
		{
			size_t iSize = 0;

			//Special case Data Types...
			switch(col.GetProviderType()) 
			{
				case DBTYPE_BOOL:
				{
					//MakeData may return "0","1" or "False","True" (if using INI File)
					//Out INI File has it in the format from DBTYPE_BOOL -> DBTYPE_WSTR
					//Which should always be in "False","True" according to the spec
					//But since we maybe formulating Literals, they must be "0","1" for Boolean literals...
					if(wcscmp(pwszMakeData, L"True")==0)
						wcscpy(pwszMakeData, L"1");
					else if(wcscmp(pwszMakeData, L"False")==0)
						wcscpy(pwszMakeData, L"0");
					break;
				}
			}

			// Special case for SQL providers that expose VARIANT
			if( col.GetProviderType() == DBTYPE_VARIANT &&
				(DBPROPVAL_SQL_ESCAPECLAUSES & GetSQLSupport()) )
			{
				pwszPrefix = col.GetVariantPrefix(ulRow);
				pwszSuffix = col.GetVariantSuffix(ulRow);
			}
			else
			{
				// All other types
				pwszPrefix = col.GetPrefix();
				pwszSuffix = col.GetSuffix();
			}

			// If the literal prefix contains a single quote then we need to 
			// escape any single quotes in the data
			if (pwszPrefix && wcschr(pwszPrefix, L'\''))
				hr = EscapeChar(&pwszMakeData, L'\'');

			if (SUCCEEDED(hr))
			{
				iSize = wcslen(pwszMakeData);
				
				if (pwszPrefix)
					iSize += wcslen(pwszPrefix);

				if (pwszSuffix)
					iSize += wcslen(pwszSuffix);

				iSize *= sizeof(WCHAR);
				iSize += sizeof(WCHAR);

				WCHAR * pwszFinalData = (WCHAR *) PROVIDER_ALLOC(iSize);
				pwszFinalData[0] = L'\0';

				if(pwszPrefix)
					wcscat(pwszFinalData,pwszPrefix);

				wcscat(pwszFinalData,pwszMakeData);

				if(pwszSuffix)
					wcscat(pwszFinalData,pwszSuffix);

				// Special case for SQL providers that expose VARIANT
				if( col.GetProviderType() == DBTYPE_VARIANT &&
					(DBPROPVAL_SQL_ESCAPECLAUSES & GetSQLSupport()) )
				{
					CCol tempcol;

					hr = FormatVariantLiteral(col.GetVariantType(ulRow, tempcol), pwszFinalData, ppwszData);
					PROVIDER_FREE(pwszFinalData);
					if( FAILED(hr) )
					{
						PROVIDER_FREE(pwszMakeData);
						return hr;
					}				
				}
				else
				{
					(*ppwszData) = (WCHAR *) pwszFinalData;
				}
			}
			
			hr = S_OK;
		}


		PROVIDER_FREE(pwszMakeData);
	}

	return hr;	//ERROR;
}

//---------------------------------------------------------------------------
// CTable::FormatVariantLiteral
//	
// CTable				|
// FormatVariantLiteral	|
// Formats a variant literal using ODBC canonical convert functions
//
// @mfunc FormatVariantLiteral
//
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   			| Function rans without problem
//  @flag E_FAIL			| Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::FormatVariantLiteral
(
	DBTYPE		wVariantType,		// @parm [IN] variant sub type
	WCHAR *		pwszData,			// @parm [IN] input data
	WCHAR ** 	ppwszData			// @parm [OUT] formatted data
)
{
	HRESULT		hr = E_OUTOFMEMORY;
	DBTYPE		wType = DBTYPE_EMPTY;
	WCHAR *		pwszSQLType = NULL;
	WCHAR		wszFNConvert[] = L"{fn convert(%s, %s)}";
	WCHAR *		pwszFinalData = NULL;
	size_t		cbSize = 0;

	ASSERT(pwszData && ppwszData);

	pwszSQLType = VariantTypeToSQLType(wVariantType);

	cbSize = wcslen(wszFNConvert) + wcslen(pwszData) + wcslen(pwszSQLType) + 1;
	cbSize *= sizeof(WCHAR);

	pwszFinalData = (WCHAR *) PROVIDER_ALLOC(cbSize);
	TESTC(pwszFinalData != NULL);
	pwszFinalData[0] = L'\0';

	swprintf(pwszFinalData, wszFNConvert,  pwszData, pwszSQLType);
	hr = S_OK;

CLEANUP:

	*ppwszData = pwszFinalData;

	return hr;
}


//---------------------------------------------------------------------------
// Returns module name.
//
//	@mfunc	GetModuleName
//
// 	@rdesc WCHAR holding user name, currently hardcoded to L"OLEDB").
//
//---------------------------------------------------------------------------
WCHAR* CTable::GetModuleName(void)	
{
	return wcsDuplicate(m_pwszModuleName);
}

//---------------------------------------------------------------------------
// CTable::GetTime
// Returns current time.
//
// @mfunc	GetTime
//
// @rdesc WCHAR holding time, looks like "951201" for Dec 1, 1995
//
//---------------------------------------------------------------------------
WCHAR * CTable::GetTime(void)	
{
	WCHAR wszDateFound[9];
	WCHAR * wszDateReturned = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) * 7);

	_wstrdate(wszDateFound);
	
	// wstrdate always returns the buffer like "mm/dd/yy"
	// Need to move these around to "yymmdd"
	wszDateReturned[0]=wszDateFound[6];
	wszDateReturned[1]=wszDateFound[7];
	wszDateReturned[2]=wszDateFound[0];
	wszDateReturned[3]=wszDateFound[1];
	wszDateReturned[4]=wszDateFound[3];
	wszDateReturned[5]=wszDateFound[4];
	wszDateReturned[6]=L'\0';

	PRVTRACE(L"%sGetTime):Date is '%s'\n",wszPRIVLIBT,wszDateReturned);
	return wszDateReturned;	
}

//---------------------------------------------------------------------------
// CTable::MakeTableName	
// Creates table name based on user name, system time, 
// and random number, if this table exists ... try again.
//
// ISSUE: HOW DO I GET CURRENT LOGGED ON USER?
//
// @mfunc MakeTableName
//
// @rdesc HRESULT indicating success or failure
//  @flag S_OK	| Function ran without problem
//  @flag E_FAIL	| No table name
//  @flag OTHER		| IDBInfo->GetInfo failed,problems with DoesTableExist()
//
//---------------------------------------------------------------------------
HRESULT CTable::MakeTableName(WCHAR* pwszTableName)
{
	LONG			lTableLength=	0;			// length a table name can be
	WCHAR * 		pwszPID=		NULL;		// time
	WCHAR *			pwszModuleName=	NULL;		// Test Case Module Name
	WCHAR * 		pwszRandNumber=	NULL;		// random number generated
	WCHAR *			pCharBuffer =	NULL;		// buffer of characters

	BOOL 			fFound =		TRUE;		// Was the table found in the datasource
	HRESULT 		hr	=			S_OK;		// result

	//If we already have a "suggessted" TableName use that
	//otherwise we will have to generate one
	if(pwszTableName)
		return SetTableName(pwszTableName);

	pwszModuleName=GetModuleName();
	
	DBLITERALINFO* pTableLiteral = GetLiteralInfo(DBLITERAL_TABLE_NAME);
	DBLITERALINFO* pCatalogLiteral = GetLiteralInfo(DBLITERAL_CATALOG_SEPARATOR);
		
	// Default to 8, if length is 0, E_FAIL or if a \ is the Catalog Separator
	if( (!pTableLiteral->cchMaxLen) ||
		(((pTableLiteral->cchMaxLen <= 12) && (pTableLiteral->cchMaxLen >= 8)) && 
		 (pCatalogLiteral->fSupported) && (!wcscmp(pCatalogLiteral->pwszLiteralValue, L"\\"))) )
		lTableLength = TABLELENGTH1;
	else
		lTableLength = pTableLiteral->cchMaxLen;

	// While table is already in data source, 
	// Build new table name and check it
	// NOTE: We use a for loop so we don't loop infinitly if
	// a unique tablename cannot be generated...
	for(ULONG i=0; i<200; i++)
	{
		pwszTableName = MakeObjectName(pwszModuleName, lTableLength);

		// If table is found try again, FALSE means not found
		TESTC_(hr = DoesTableExist(pwszTableName, &fFound), S_OK);
		if(!fFound)
		{
			// Assign local Table Name to Member variable
			SetTableName(pwszTableName);
			goto CLEANUP;
		}

		//Cleanup here, incase we have to loop agian for another tablename...
		PROVIDER_FREE(pwszTableName);
	}

CLEANUP:
	PROVIDER_FREE(pwszTableName);
	PROVIDER_FREE(pwszModuleName);
	
	if(FAILED(hr))
		return hr;
	if(fFound)
	{
		odtLog << "ERROR: Unable to create unique TableName...\n";
		return DB_E_DUPLICATETABLEID;
	}
	return hr;
}

	

//---------------------------------------------------------------------------
// CTable::MayWrite
// Checks all columns for updatability. 
//	
// @mfunc	MayWrite
//	
// @rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function rans without problem
// 	@flag E_FAIL    | Subfunction ran with problems
//  @flag OTHER		| Problems with execution of statement,problems with subfunctions
//	
//---------------------------------------------------------------------------
HRESULT CTable::MayWrite(BOOL* pfMayWrite)
{
	HRESULT			hr = E_FAIL;
	
	DBORDINAL		i,cColumns = 0;
	DBCOLUMNINFO*	rgColumnInfo = NULL;
	WCHAR*			pStringsBuffer = NULL;
	IColumnsInfo* 	pIColumnsInfo = NULL;
	BOOL fMayWrite  = FALSE;
	BOOL fBookmark  = FALSE;

	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//DBPROP_IRowsetChange (if supported and settable)
	//Since WRITE | WRITEUNKNOWN are based upon weither SetData can be called
	//We should really set IRowsetChange if supported...
	if(SettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, m_pIDBInitialize))
		SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_BOOL, VARIANT_TRUE);

	//Obtain the rowset
	//NOTE:  If Commands are supported, we will reduce overhead and just ask for the columns...
	TESTC_(hr = CreateRowset(GetCommandSupOnCTable() ? SELECT_EMPTYROWSET : USE_OPENROWSET, IID_IColumnsInfo, cPropSets, rgPropSets, (IUnknown **)&pIColumnsInfo),S_OK);

	//IColumnsInfo::GetColumnInfo
	TESTC_(hr = pIColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &pStringsBuffer),S_OK);

	//Columns returned must be the same as expected, otherwise
	//we might not be adjusted all the columns, which is in error...
	fBookmark = rgColumnInfo[0].iOrdinal == 0;
	TESTC(cColumns - fBookmark == m_ColList.GetCount());
			
	//Loop over all columns returned
	for(i=0; i<cColumns; i++)
	{
		//Check to see if the Ordinal is 0 (BOOKMARK)
		if(rgColumnInfo[i].iOrdinal == 0 )
			continue;

		//Find the corresponding column
		//Its an error if ColumnsInfo returns a column we don't already know about!
		POSITION pos = m_ColList.FindIndex(rgColumnInfo[i].iOrdinal-1);
		if(pos == NULL)
		{
			hr = E_FAIL;
			goto CLEANUP;
		}

		//Get the corresponding column
		CCol& rCol = m_ColList.GetAt(pos);
				
		//DBCOLUMNFLAGS_WRITE | DBCOLUMNFLAGS_WRITEUNKNOWN
		if((rgColumnInfo[i].dwFlags & DBCOLUMNFLAGS_WRITE) || (rgColumnInfo[i].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))
		{
			PRVTRACE(L"%sMayWright):%s = TRUE\n",wszPRIVLIBT,rgColumnInfo[i].pwszName);
			if(pfMayWrite)
			fMayWrite = TRUE;	
			rCol.SetUpdateable(TRUE);
		}
		else
		{
			PRVTRACE(L"%sMayWright):%s = FALSE\n",wszPRIVLIBT,rgColumnInfo[i].pwszName);
			fMayWrite = FALSE;
			rCol.SetUpdateable(FALSE);
		}

		//Before we update the columninfo, make sure the values that we used to create
		//the table, are the same as those returned by the provider...
//		COMPARE(CompareStrings(rCol.GetColName(), rgColumnInfo[i].pwszName), TRUE);
//		COMPARE(rCol.GetTypeInfo(),				rgColumnInfo[i].pTypeInfo);
//		COMPARE(rCol.GetColNum(),				rgColumnInfo[i].iOrdinal);
//		COMPARE(rCol.GetFlags(),				rgColumnInfo[i].dwFlags);
//		COMPARE(rCol.GetColumnSize(),			rgColumnInfo[i].ulColumnSize);
//		COMPARE(rCol.GetProviderType(),			rgColumnInfo[i].wType);
//		COMPARE(rCol.GetPrecision(),			rgColumnInfo[i].bPrecision);
//		COMPARE(rCol.GetScale(),				rgColumnInfo[i].bScale);
//		COMPARE(CompareDBID(*rCol.GetColID(),	rgColumnInfo[i].columnid), TRUE);
		
		//Correctly update the Column Info, now that we have the table created.
		//NOTE:  The columnid returned by the provider, may be different than
		//the column name, provided in the statement...
		rCol.SetColInfo(&rgColumnInfo[i]);
	}

CLEANUP:
	if(pfMayWrite)
		*pfMayWrite = fMayWrite;
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIColumnsInfo);
	PROVIDER_FREE(rgColumnInfo);
	PROVIDER_FREE(pStringsBuffer);
	return hr;
}
		
//---------------------------------------------------------------------------
//	CTable::QCreateTable			
//
//	Called from one of the overloaded CreateTable functions.Passing zero
//  for ulIndex means do not but an Index on this table. eValue means
//  to create table with eValue kind of data.
//	
//	1) Get Table Name
//	2) Get Datatypes from Datasource
//	3) Determine if we'll use command with SQL or ITableDefinition
//	4) If using command, build sql/execute string for creating table
//	5) Create Table
//	4) Create Index, if requested
//	5) Fill table with rows, if requested
//	
//	create table SQL statement looks like:
//	"create table GetTableName() (col1 type,col2 type)"
//
//	@mfunc QCreateTable
// 	@rdesc HRESULT indicating success or failure
// 	@flag S_OK   | Function ran without problem
// 	@flag Other     | Failures in subfunction CreateIndex,
//					 Failures in execution of CreateTable statement,
//					 Failures in subfunction MayWrite,
//					 Failures in subfunction Insert,
//	
//---------------------------------------------------------------------------
HRESULT CTable::QCreateTable(
	DBCOUNTITEM	ulRowCount, 		// @parm [IN] # of Rows to insert into table, 1 based
	DBORDINAL	ulIndex,			// @parm [IN] Column Number of index, 1 based
	EVALUE 		eValue				// @parm [IN] insert PRIMARY or SECONDARY data
)
{	
	POSITION 	pos;				// position in CList
	BOOL		fMayWrite	=FALSE;
	HRESULT 	hr			=S_OK;	// result
	LONG 		lIndex		=0;		// index
	WCHAR *		pwszSingleChar=NULL;
	WCHAR *		pwszSQLText	=NULL;	// sql text to be executed
	WCHAR *		pwszColDef	=NULL;	// column definition
	WCHAR 		pwszColList[20000];	// list of columns 
	BOOL		fNotNull	= FALSE;// NOT NULL clause support
	ITableDefinition* pITableDefinition = NULL;
	BOOL		fExist;
	WCHAR*		pwszProviderName = NULL;
	ICommandWithParameters* pICommandWithParameters = NULL;

	// Initialize
	pwszColList[0]=L'\0';
	fNotNull = IsNonNullColumnsAllowed();

	// if data is already specified do not build it again
	if (!m_fBuildColumnDesc)
	{
		// the table will be constructed according to the specified column description array
		ColumnDesc2ColList(m_rgColumnDesc, m_cColumnDesc);
	}

	if(!VerifyInterface(m_pIOpenRowset, IID_ITableDefinition, SESSION_INTERFACE, (IUnknown**)&pITableDefinition))
		hr = E_FAIL;


	if(GetCommandSupOnCTable() && !m_pICommand && m_pIDBCreateCommand)
	{
		CHECK(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,
			(IUnknown**)&m_pICommand),S_OK);
	}

	if(!(GetModInfo()->IsUsingITableDefinition() && (hr == S_OK)))
	{
		hr = S_OK;	//again;
		if (!GetCommandSupOnCTable())
		{
			hr = E_FAIL;	// or any better error flag
			goto CLEANUP;
		}

		// Go thru list, get Col Defs
		// Get head of list
		pos=m_ColList.GetHeadPosition();

		// While not at the end of the list
		while(pos!=NULL)
		{
			// Get a ccol object 
			CCol& col = m_ColList.GetNext(pos);
			
			// Get column name, definition
			col.CreateColDef(&pwszColDef);
			wcscat(pwszColList,pwszColDef);

			// ask for a default value if user requests for it (CCol.m_fHasDefault == TRUE)
			if (col.GetHasDefault())
			{
				VARIANT	vDefault = col.GetDefaultValue();
				wcscat(pwszColList, L" DEFAULT ");
				wcscat(pwszColList, V_BSTR(&vDefault));
			}

			// Make columns non nullable if user requests for nonulls in his data.
			// ITableDefinition part of the if has not been modified for this change.
			if ((m_eNull == NONULLS && fNotNull) || (!col.GetNullable() && fNotNull))
			{
				col.SetNullable(0);
				wcscat(pwszColList, L" NOT NULL");
			}
			
			if (col.GetUnique())
				wcscat(pwszColList, L" UNIQUE");

			wcscat(pwszColList,wszCOMMA);
			PROVIDER_FREE(pwszColDef);
			pwszColDef = NULL;
		}
		
		if (pwszColList)
		{
			pwszSingleChar=wcsrchr(pwszColList,L',');
			
			// If we found a comma at the end, remove it 
			if (pwszSingleChar)
			{
				lIndex = (LONG)(pwszSingleChar - pwszColList);
				pwszColList[lIndex]= L'\0';
			}
		}

		// Build & execute SQL text
		pwszSQLText = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * (
						wcslen(wszCREATE_TABLE) + 
						wcslen(GetTableName()) +
						wcslen(pwszColList) +
						sizeof(WCHAR))));

		swprintf(pwszSQLText, wszCREATE_TABLE, GetTableName(), pwszColList);
		PRVTRACE(L"%sCreateTable): %s\n", wszPRIVLIBT, pwszSQLText);

		// Execute sql
		// Return code is checked below
		hr=	BuildCommand(pwszSQLText,	//SQL text to execute
						IID_NULL,		//This won't be used
						EXECUTE_ALWAYS,	//Should always execute fine
						0, NULL,		//Set no properties
						NULL,			//No parmeters									
						NULL, 			//No RowsAffected info
						NULL,			//No rowset needs to be allocated
						&m_pICommand);	//Use this object's command to execute on
		// to fit with MayWrite
		m_cColumnDesc = m_ColList.GetCount();
	}
	else
	{
		// Use ITableDefiniton since commands aren't supported
		// Set up column desc array for ITableDefinition 
		if (m_fBuildColumnDesc)
		{
			ReleaseColumnDesc(m_rgColumnDesc, m_cColumnDesc);
			m_cColumnDesc=0;
			if (!SUCCEEDED(hr = BuildColumnDescs(&m_rgColumnDesc)))
				goto CLEANUP;
			else
				m_cColumnDesc = CountColumnsOnTable();
		}

		// use m_fInputTableID to decide whether an input table IS is provided
		if (m_fInputTableID)
			hr = pITableDefinition->CreateTable(m_pUnkOuter, &m_TableID, m_cColumnDesc, 
				m_rgColumnDesc, *m_riid, m_cPropertySets, m_rgPropertySets, m_ppTableID, m_ppRowset);
		else
		{
			// set new DBID for m_TableID
			if (SUCCEEDED(hr = pITableDefinition->CreateTable(m_pUnkOuter, NULL, m_cColumnDesc,
					m_rgColumnDesc, *m_riid, m_cPropertySets, m_rgPropertySets, m_ppTableID, m_ppRowset)))
				SetTableID(**m_ppTableID);
		}
	}

	fExist = FALSE;
	if(FAILED(hr) || FAILED(hr = DoesTableExist(&GetTableID(), &fExist)) || !fExist)
	{
		hr = FAILED(hr) ? hr : E_FAIL;
		goto CLEANUP;
	}

	// Check MAYWRITE
	hr = MayWrite(&fMayWrite);
	if(FAILED(hr))
		goto CLEANUP;

	// Create Index
	if (ulIndex!=0)
	{
		if (FAILED(CreateIndex(ulIndex,UNIQUE)))
			PRVTRACE(L"%sQCreateTable):CreateIndex - %s\n",wszPRIVLIBT,wszFAILED);
	}		

	// Go thru list, get Col Defs
	// Get head of list
	pos=m_ColList.GetHeadPosition();

	// While not at the end of the list
	while(pos!=NULL)
	{
		// Get a ccol object 
		CCol& col = m_ColList.GetNext(pos);

		//set the index orindal info in CCol to Unique
		if(ulIndex==col.GetColNum())
		{
			col.SetUnique(TRUE);
		}
	}		

	// No rows to create? Then we're done.
	if(ulRowCount==0)
		goto CLEANUP;
	
	// HACK for Jolt
	//Jolt does not recognize "?" syntax, so just don't use 
	//InsertWithParams id on Jolt.
	pwszProviderName = GetProviderName(m_pIOpenRowset);
	if(pwszProviderName && wcscmp(pwszProviderName, L"MSJTOR35.DLL")==0)
	{
		QTESTC_(hr = Insert(0, PRIMARY, TRUE, NULL, FALSE, ulRowCount),S_OK);
	}
	else if((!GetModInfo()->IsUsingITableDefinition() ||
			(GetModInfo()->IsUsingITableDefinition() && !SupportedProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, m_pIDBInitialize,ROWSET_INTERFACE)) ||
			GetModInfo()->GetInsert()==INSERT_WITHPARAMS) &&
			GetModInfo()->GetInsert()!=INSERT_COMMAND &&
			VerifyInterface(m_pICommand, IID_ICommandWithParameters, COMMAND_INTERFACE, (IUnknown**)&pICommandWithParameters))
	{
		QTESTC_(hr = InsertWithParams(0, PRIMARY, TRUE, NULL, ulRowCount),S_OK);
	}
	else 
	{
		QTESTC_(hr = Insert(0, PRIMARY, TRUE, NULL, FALSE, ulRowCount),S_OK);
	}
		
CLEANUP:
	PROVIDER_FREE(pwszProviderName);
	SAFE_RELEASE(pITableDefinition);
	SAFE_RELEASE(pICommandWithParameters);
	PROVIDER_FREE(pwszSQLText);
	return hr;
}

//---------------------------------------------------------------------------
// CTable::SetColsUsed		
// Sets columns in current list to be used in selection criteria.
// If error occurs, all columns are set and E_Fail is returned.
//
// @mfunc SetColsUsed
//	
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//	
//---------------------------------------------------------------------------
HRESULT	CTable::SetColsUsed(
	CList <DBORDINAL,DBORDINAL> &rColNumList	// @parm [IN] list of columns requesting to set
)
{
	POSITION 	posNum;		//	position in ColNumList
	POSITION 	posm_ColList;		//	position in m_ColList
	DBORDINAL	iOrdinal=0;		//	column number in ColNumList
	BOOL 		fFound=FALSE;		//  column was found
	
	// Check Params
	if (rColNumList.IsEmpty())
	{
		AllColumnsForUse(TRUE);
		PRVTRACE(L"%sSetColsUsed):rColNumList is empty, %s",wszPRIVLIBT,wszFUNCFAIL);
		return E_FAIL;
	}

	// Make sure m_ColList is NOT empty
	ASSERT(m_ColList.IsEmpty()==FALSE);
	
	// Set all columns to not used as initialization
	AllColumnsForUse(FALSE);

	// Run Thru List Setting Cols
	// Set positions
	posNum=rColNumList.GetHeadPosition();
	
	// While ColNumList (passed in) is not empty
	while (posNum!=NULL) 
	{
		// Move to next element in ColNumList
		iOrdinal = rColNumList.GetAt(posNum);
		posm_ColList=m_ColList.GetHeadPosition();

		// While m_ColList (member variable) is not empty
		while(posm_ColList!=NULL) 
		{
			CCol& rCol = m_ColList.GetNext(posm_ColList);

			// If column # of both objects match
			if(rCol.GetColNum()==iOrdinal)
			{
				// Set column to be used in query
				rCol.SetUseInSQL(TRUE);
				fFound = TRUE;
				break;
			}
		}
		
		// If match was not made, then set all columns and return error
		if (fFound == FALSE)
		{
			AllColumnsForUse(TRUE);
			PRVTRACE(L"%sSetColsUsed):column not found, %s",wszPRIVLIBT,wszFUNCFAIL);
			return E_FAIL;
		}

		// Reset for next iteration
		fFound = FALSE;
		rColNumList.GetNext(posNum);
	}

	return S_OK;
}

//---------------------------------------------------------------------------
// CTable::SetExistingTable		
// Sets table object from existing table including:
// GetTableName()
// m_ColList
// m_udwRows
// m_udwNextRow
// m_pwszIndexName
// m_udwIndex
//
// @mfunc SetExistingTable
//
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::SetExistingTable(
	WCHAR * pwszTableName		// parm [IN] Table name, table name cannot be empty
)             
{
	HRESULT hr		= E_FAIL;	// result
	BOOL 	fFound	= FALSE;	// is object found

	ICommandText* pICommandText = NULL;
	IRowset* pIRowset = NULL;

	// Check to see if the Table is there
	TESTC_(hr = DoesTableExist(pwszTableName, &fFound),S_OK);
	if(fFound)
	{
		// Assign local Table Name to Member variable
		if (pwszTableName)
			SetTableName(pwszTableName);

		// If index doesn't exist
		// StrIndexName and udwIndex will come back empty
		if(FAILED(hr = DoesIndexExist(&fFound)))
			goto CLEANUP;

		// Set List of Columns (m_ColList)
		if(FAILED(hr = SetTableColumnInfo(pwszTableName)))
			goto CLEANUP;

		// Sets Count of Rows (m_ulRows)
		CountRowsOnTable();
		
		// Sets next row (m_udwNextRow)
		m_ulNextRow = m_ulRows+1;
	}

CLEANUP:
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pIRowset);
	
	if(!fFound)
		return DB_E_NOTABLE;
	return hr;
}




//---------------------------------------------------------------------------
//	CTable::AddInfoFromColumnsSchemaRowset
//
//	mfunc	HRESULT								|
//			CTable								|
//			AddInfoFromColumnsSchemaRowset
//			Updates column information about in m_ColList. It
//  grabs column information using IDBSchemaRowset::GetRowset. 
//
// 	rdesc HRESULT indicating success or failure
// 	
// 	flag S_OK   | Function ran without problem
// 	flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
// @cmember Updates elements in m_ColList based on values read from column schema rowset
HRESULT CTable::AddInfoFromColumnsSchemaRowset(
	IUnknown		*pIUnknown,				// [IN] session interface
	WCHAR			*pwszTableName/*=NULL*/,// [IN]	table name (if null go for current table)
	DBCOUNTITEM		*pcColsFound/*=NULL*/	// [OUT] Total number of columns found (Default=NULL)
)
{
	HRESULT				hr					= E_FAIL;
	IDBSchemaRowset		*pIDBSchemaRowset	= NULL;
	const ULONG			cRestrictColumns	= 4;
	VARIANT				rgRestrictColumns[cRestrictColumns];
	ULONG				i;

	// array of supported schemas (used in IDBSchemaRowset::GetSchemas
	ULONG				cSchema				= 0;		// number of supported Schemas
	GUID				*prgSchemas			= NULL;		// array of GUIDs
	ULONG				*prgRestrictions	= 0;		// restrictions for each Schema

	IRowset				*pColsRowset		= NULL;		// returned rowset
	const DBROWCOUNT	cRowsAsked			= 10;		// how many rows to return at once

	DBLENGTH			ulRowSize		= 0;		// size of row
	DBCOUNTITEM			cDBBINDING		= 0;		// count of bindings
	DBORDINAL			cDBCOLUMNINFO	= 0;		// count of columninfo
	DBCOUNTITEM			iRow			= 0;		// count of rows
	DBCOUNTITEM			cRowsObtained	= 0;		// number of rows returned, should be 1
	HROW *				rghRows			= NULL;		// array of handles of rows
	DBBINDING *			rgDBBINDING		= NULL;		// array of bindings
	DBCOLUMNINFO *		rgDBCOLUMNINFO	= NULL;		// array of columninfos
	WCHAR *				pStringsBuffer	= NULL;		// corresponding strings
	BYTE *				pRow			= NULL;		// pointter to data
	IAccessor			*pIAccessor		= NULL;
	HACCESSOR 			hAccessor		= NULL;		// accessor
	DATA *				pColumn			= NULL;

	WCHAR				*pwszColumnName;
	GUID				*pguidColumn;
	ULONG				*pulColumn;

	DBCOUNTITEM			cColumnName			= 3;
	DBCOUNTITEM			cColumnGUID			= 4;
	DBCOUNTITEM			cColumnPropID		= 5;
	DBCOUNTITEM			cOrdinalPosition	= 6;
	DBCOUNTITEM			cDescription		= 27;

	DBID				ColumnID;

	DBCOUNTITEM			cColsFound			= 0;
	DBREFCOUNT			cRefCounts;

	if (NULL == pIUnknown)
		return E_INVALIDARG;

	if (NULL == pwszTableName)
	{
		pwszTableName = GetTableName();
		if (NULL == pwszTableName)
			return E_INVALIDARG;
	}

	// get the IDBSchemaRowset interface
	if (!VerifyInterface(pIUnknown, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset))
		return E_FAIL;

	// Check to see if the schema (column rowset) is supported
	CHECK(hr = pIDBSchemaRowset->GetSchemas(&cSchema, &prgSchemas, &prgRestrictions),S_OK);

	// Check to see if DBSCHEMA_COLUMNS is supported
	for(i=0; i<cSchema; i++)
	{
		if(DBSCHEMA_COLUMNS == prgSchemas[i])
			break;
	}

	ULONG index;
	// Set restrictions
	for(ULONG index=0;index<cRestrictColumns;index++)
		VariantInit(&rgRestrictColumns[index]);

	rgRestrictColumns[2].vt 	 = VT_BSTR;
	rgRestrictColumns[2].bstrVal = SysAllocString(pwszTableName);

	// if i is equal to cSchema it is not supported
	if(i == cSchema)
		goto CLEANUP;

	// make sure table name restriction is supported
	TESTC_PROVIDER(0 != prgRestrictions[i] );

	if (!CHECK(hr = pIDBSchemaRowset->GetRowset(	
			NULL, 								// aggregation
			DBSCHEMA_COLUMNS,					// REFGUID
			cRestrictColumns,	 				// count of restrictions (1:types)
			rgRestrictColumns,					// list of restrictions
			IID_IRowset,						// REFFID
			0,									// count of properties
			NULL,								// range of properties
			(IUnknown**)&pColsRowset			// returned result set
		),S_OK))
			goto CLEANUP;

	COMPARE(VerifyInterface(pColsRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor), TRUE);
	
	if (!CHECK(hr = GetAccessorAndBindings(
		pColsRowset,		
		DBACCESSOR_ROWDATA,
		&hAccessor,			
		&rgDBBINDING,		
		&cDBBINDING,
		&ulRowSize,	
		DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_EXCEPTBOOKMARK,
		FORWARD,
		NO_COLS_BY_REF,
		&rgDBCOLUMNINFO,	// OUT: Array of DBCOLUMNINFOs
		&cDBCOLUMNINFO,		// OUT: Count of DBCOULMNINFOs
		&pStringsBuffer, 
		DBTYPE_EMPTY, 
		0, 						//cColsToBind
		NULL,					//rgColsToBind
		NULL,					//rgColOrdering
		NO_COLS_OWNED_BY_PROV,	//eColsMemProvOwned
		DBPARAMIO_NOTPARAM,		//eParamIO
		BLOB_LONG),S_OK))		//dwBlobType
		goto CLEANUP;

	pRow = new	BYTE[(size_t)ulRowSize];	//data

	while(!FAILED(hr=pColsRowset->GetNextRows(0, 0, cRowsAsked, &cRowsObtained, &rghRows)) && cRowsObtained !=0)
	{ 
		// Check the HResult an number of rows
		if (hr == S_OK)
			COMPARE(cRowsObtained, cRowsAsked);
		else if (hr == DB_S_ENDOFROWSET)
			COMPARE(1, (cRowsObtained < cRowsAsked));
		else
			goto CLEANUP;

		// Get data for each row
		for(iRow=0;iRow<cRowsObtained;iRow++, cColsFound++)			 
		{
			// Get data for a row
			CHECK(hr=pColsRowset->GetData(rghRows[iRow], hAccessor, pRow),S_OK);

			if (FAILED(hr))
				goto CLEANUP;

			pColumn = (DATA*)(pRow + rgDBBINDING[cOrdinalPosition].obStatus);

			if (DBSTATUS_S_OK == pColumn->sStatus)
			{
				//get the associated column in m_ColList
				CCol	&col = GetColInfoForUpdate(*(ULONG*)pColumn->bValue);

				pColumn = (DATA*)(pRow + rgDBBINDING[cDescription].obStatus);
				if (DBSTATUS_S_OK == pColumn->sStatus)
				{
					col.SetColDescription((WCHAR*)pColumn->bValue);
				}

				// get the name of the column
				pwszColumnName = NULL;
				pColumn = (DATA*)(pRow + rgDBBINDING[cColumnName].obStatus);

				if (DBSTATUS_S_OK == pColumn->sStatus)
				{
					pwszColumnName = (WCHAR*)pColumn->bValue;
				}
				else
					COMPARE(DBSTATUS_S_ISNULL == pColumn->sStatus, TRUE);

				// get the GUID of the column
				pguidColumn = NULL;
				pColumn = (DATA*)(pRow + rgDBBINDING[cColumnGUID].obStatus);

				if (DBSTATUS_S_OK == pColumn->sStatus)
				{
					pguidColumn = (GUID*)pColumn->bValue;
				}
				else
					COMPARE(DBSTATUS_S_ISNULL == pColumn->sStatus, TRUE);
			
				// get the PropID of the column
				pulColumn = NULL;
				pColumn = (DATA*)(pRow + rgDBBINDING[cColumnPropID].obStatus);

				if (DBSTATUS_S_OK == pColumn->sStatus)
				{
					pulColumn = (ULONG*)pColumn->bValue;
				}
				else
					COMPARE(DBSTATUS_S_ISNULL == pColumn->sStatus, TRUE);
			
				// set the DBID of the column
				BuildDBID(&ColumnID, pwszColumnName, pguidColumn, pulColumn);
				col.SetColID(&ColumnID);
			}
		}

		// need to release rows
		CHECK(hr=pColsRowset->ReleaseRows
		(		 
			cRowsObtained,		// number of rows to release
			rghRows,	  		// array of row handles
			NULL,
			NULL,				// count of rows successfully released
			NULL				// there shouldn't be anymore references to these rows
		),S_OK);

		if (FAILED(hr))
			goto CLEANUP;
	}

	hr = S_OK;

CLEANUP:
	if(pIAccessor && hAccessor)
	{
		CHECK(pIAccessor->ReleaseAccessor(hAccessor, &cRefCounts), S_OK);
		COMPARE(cRefCounts,0);
	}
	SAFE_RELEASE(pIAccessor);

	if (pcColsFound)
		*pcColsFound = cColsFound;

	PROVIDER_FREE(prgRestrictions);
	PROVIDER_FREE(prgSchemas);

	SAFE_DELETE(pRow);

	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pColsRowset);

	PROVIDER_FREE(rgDBBINDING);
	PROVIDER_FREE(rgDBCOLUMNINFO);
	PROVIDER_FREE(pStringsBuffer);
	PROVIDER_FREE(rghRows);

	for(index=0;index<cRestrictColumns;index++)
		GCHECK(VariantClear(&(rgRestrictColumns[index])),S_OK);
	
	return hr;
} //CTable::AddInfoFromColumnsSchemaRowset





//---------------------------------------------------------------------------
//	CTable::GetFromColumnsRowset
//
//	mfunc	HRESULT								|
//			CTable								|
//			GetFromColumnsRowset				|
//			Very similar to SetFromColumnsRowset. Puts column information about 
//  pwszTableName table in m_ColList.Grab column information using 
//  IColumnsRowset::GetColumnsRowset.	If no columns found
//  return E_FAIL. If pIRowset == NULL, E_FAIL.
//
//  Mapping of CCol private member variable to DBCOLUMNINFO struct members:
//  -----------------------------------------------
//
//	m_fAutoInc			->	DBCOLUMN_ISAUTOINCREMENT
//	m_fSearchable		->	DBCOLUMN_ISSEARCHABLE
// 
// 	rdesc HRESULT indicating success or failure
// 	
// 	flag S_OK   | Function ran without problem
// 	flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CTable::GetFromColumnsRowset(
	IColumnsRowset * pIColumnsRowset,	// @parm ||[IN]	 IColumnsRowset pointer
	DBORDINAL *		 pcColsFound			// @parm ||[OUT] Count of columns found
)
{
	HRESULT 			hr				= E_FAIL;
	BOOL 				fResult			= FALSE;
	BOOL				fFound			= FALSE;
	POSITION 			pos;						// position in m_ColList
	CCol 				col;						// column in m_ColList
	HACCESSOR 			hAccessor;					// accessor
	DBCOUNTITEM			cRowsObtained	= 0;		// number of rows returned, should be 1
	DBLENGTH			ulRowSize		= 0;		// size of row of data
	HROW *				rghRows			= NULL;		// array of handles of rows
	DATA *				pColumn			= NULL;
	BYTE *				pRow			= NULL;		// pointter to data
	IRowset *			pColRowset		= NULL;		// Rowset interface pointer
	DBCOUNTITEM			iDBBINDING		= 0;		// index of rgDBBINDING
	DBCOUNTITEM			cDBBINDING		= 0;		// count of bindings
	DBBINDING *			rgDBBINDING		= NULL;		// pointer to array of bindings
	DBORDINAL			cDBCOLUMNINFO	= 0;		// count of column info
	DBCOLUMNINFO *		rgDBCOLUMNINFO	= NULL;		// pointer to array of columninfos
	WCHAR *				rgStringsBuffer	= NULL;		// corresponding strings
	DBORDINAL			cOptionalColumns= 0;		// count of optional columns
	DBID *				rgOptionalColumns=NULL;		// array of optional columns
	DBCOUNTITEM			iRow			= 0;		// row index
	DBCOUNTITEM			ulTotalRows		= 0;
	int					fDefault;
	DBCOUNTITEM			i;

	// column ordinals for some optional columns
	DBCOUNTITEM			iAutoincrement	= 0;
	DBCOUNTITEM			iUnique			= 0;
	DBCOUNTITEM			iHasDefault		= 0;		
	DBCOUNTITEM			iDefaultValue	= 0;
	DBCOUNTITEM			iCLSID			= 0;
	DBCOUNTITEM			iLCID			= 0;
	DBCOUNTITEM			iBaseColName	= 0;

	TESTC(NULL != pcColsFound);
	*pcColsFound = 0;

	if (!CHECK(hr = pIColumnsRowset->GetAvailableColumns(&cOptionalColumns,&rgOptionalColumns),S_OK))
		goto CLEANUP;

	if (!CHECK(hr = pIColumnsRowset->GetColumnsRowset(NULL,cOptionalColumns,
		rgOptionalColumns,IID_IRowset,0,NULL,(IUnknown **) &pColRowset),S_OK))
		goto CLEANUP;

	if (!CHECK(hr = GetAccessorAndBindings(
		pColRowset,			// IN: 	Rowset to get info from
		DBACCESSOR_ROWDATA,
		&hAccessor,			// OUT:	accessor from create accessor
		&rgDBBINDING,		// OUT:	Array of DBBINDINGs
		&cDBBINDING,
		&ulRowSize,			// OUT: offset from DBBINDING struct
		DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_EXCEPTBOOKMARK,
		FORWARD,
		NO_COLS_BY_REF,
		&rgDBCOLUMNINFO,	// OUT: Array of DBCOLUMNINFOs
		&cDBCOLUMNINFO,		// OUT: Count of DBCOULMNINFOs
		&rgStringsBuffer, 
		DBTYPE_EMPTY, 
		0, 						//cColsToBind
		NULL,					//rgColsToBind
		NULL,					//rgColOrdering
		NO_COLS_OWNED_BY_PROV,	//eColsMemProvOwned
		DBPARAMIO_NOTPARAM,		//eParamIO
		BLOB_LONG),S_OK))		//dwBlobType
		goto CLEANUP;

	// set the properties that can be get for the columns (m_fColProps)
	fDefault = 0;
	SetHasUnique(FALSE);
	SetHasCLSID(FALSE);
	SetHasAuto(FALSE);

	for (i=0; i< cDBBINDING; i++)
	{
		if (CompareDBID(rgDBCOLUMNINFO[i].columnid, DBCOLUMN_ISAUTOINCREMENT))
		{
			m_fColProps |= READ_DBPROP_COL_AUTOINCREMENT;
			iAutoincrement = i;
			SetHasAuto(TRUE);
		}
			
		if (CompareDBID(rgDBCOLUMNINFO[i].columnid, DBCOLUMN_ISUNIQUE))
		{
			m_fColProps |= READ_DBPROP_COL_UNIQUE;
			iUnique = i;
			SetHasUnique(TRUE);
		}
			
		if (CompareDBID(rgDBCOLUMNINFO[i].columnid, DBCOLUMN_CLSID))
		{
			m_fColProps |= READ_COL_CLSID;
			iCLSID = i;
			SetHasCLSID(TRUE);
		}
			
		if (CompareDBID(rgDBCOLUMNINFO[i].columnid, DBCOLUMN_HASDEFAULT))
		{
			fDefault++;
			iHasDefault = i;
		}

		if (CompareDBID(rgDBCOLUMNINFO[i].columnid, DBCOLUMN_DEFAULTVALUE))
		{
			fDefault++;
			iDefaultValue = i;
		}

		if (CompareDBID(rgDBCOLUMNINFO[i].columnid, DBCOLUMN_COLLATINGSEQUENCE))
		{
			iLCID = i;
		}

		if (CompareDBID(rgDBCOLUMNINFO[i].columnid, DBCOLUMN_BASECOLUMNNAME))
		{
			iBaseColName = i;
		}
	} // for i
	
	if (fDefault == 2)
	{
		m_fColProps |= READ_DBPROP_COL_DEFAULT;
		SetHasDefault(TRUE);
	}
	else
		SetHasDefault(FALSE);
	
	pRow = new	BYTE[(size_t)ulRowSize];	//data

	while(!FAILED(hr=pColRowset->GetNextRows
	(
		0,					// no chapters
		0,					// don't skip any rows
		10,					// total number of rows requesting
		&cRowsObtained,		// number of rows returned, this will be the number of
							//  columns in the table if there are 10 or less
		&rghRows			// array of handles of rows
	)) && cRowsObtained !=0)
	{ 
		// Check the HResult an number of rows
		if(hr == S_OK)
			COMPARE(cRowsObtained, 10);
		else if (hr == DB_S_ENDOFROWSET)
			COMPARE(1, (cRowsObtained < 10));
		else
			goto CLEANUP;

		// Get data for each row
		for(iRow=0;iRow<cRowsObtained;iRow++)			 
		{
			// Get data for a row
			CHECK(hr=pColRowset->GetData(		 
				rghRows[iRow],		// hrow
				hAccessor,	  		// handle of accessor to use
				pRow				// actual row of data returned 
			),S_OK);

			if (FAILED(hr))
				goto CLEANUP;

			// positioning for DBCOLUMN_NUMBER
			pColumn = (DATA *) (pRow + rgDBBINDING[4].obStatus);

			if (DBSTATUS_S_OK != pColumn->sStatus)
				continue;

			// for each column in our CList
			pos = m_ColList.GetHeadPosition();
			while(pos)
			{
				CCol& rCol = m_ColList.GetNext(pos);

				// Match this rows DBCOLUMN_NUMBER
				if((*(unsigned int*)pColumn->bValue) != rCol.GetColNum())
					continue;

				// Get Data for columns of interest
				if (iAutoincrement)
				{
					pColumn = (DATA *) (pRow + rgDBBINDING[iAutoincrement].obStatus);
					if (pColumn->sStatus==DBSTATUS_S_OK)
					{
						if (*(VARIANT_BOOL*) pColumn->bValue==0)
							rCol.SetAutoInc(FALSE);
						else
							rCol.SetAutoInc(TRUE);
							
						PRVTRACE(L"COLROWSET AutoInc:%d\n", *(VARIANT_BOOL*) pColumn->bValue);
						fFound = TRUE;
					}
				} // if (iAutoincrement)
				
				if (iUnique)
				{
					pColumn = (DATA *) (pRow + rgDBBINDING[iUnique].obStatus);
					if (pColumn->sStatus==DBSTATUS_S_OK)
					{
						if (*(VARIANT_BOOL*) pColumn->bValue==0)
							rCol.SetUnique(FALSE);
						else
							rCol.SetUnique(TRUE);
							
						PRVTRACE(L"COLROWSET Unique:%d\n", *(VARIANT_BOOL*) pColumn->bValue);
						fFound = TRUE;
					}
				} // if (iUnique)
				
				if (iCLSID)
				{
					pColumn = (DATA *) (pRow + rgDBBINDING[iCLSID].obStatus);
					if (pColumn->sStatus==DBSTATUS_S_OK)
					{
						rCol.SetTypeGuid( * ((GUID*)pColumn->bValue) );
						PRVTRACE(L"COLROWSET CLSID:%d\n", *(GUID*) pColumn->bValue);
						fFound = TRUE;
					}
				} // if (iCLSID)
				
				if (iHasDefault && iDefaultValue)
				{
					pColumn = (DATA *) (pRow + rgDBBINDING[iHasDefault].obStatus);
					if (pColumn->sStatus==DBSTATUS_S_OK)
					{
						if (*(VARIANT_BOOL*) pColumn->bValue == VARIANT_TRUE)
						{
							pColumn = (DATA *) (pRow + rgDBBINDING[iDefaultValue].obStatus);
							if (pColumn->sStatus == DBSTATUS_S_OK)
							{
								rCol.SetDefaultValue(* (VARIANT *) pColumn->bValue);
								fFound = TRUE;
							}
						}
						else
							rCol.SetHasDefault(FALSE);
							
						PRVTRACE(L"COLROWSET HasDefault:%d\n", *(VARIANT_BOOL*) pColumn->bValue);
					}
				} // if (iAutoincrement)

				if (iLCID)
				{
					pColumn = (DATA *)(pRow + rgDBBINDING[iLCID].obStatus);
					if (pColumn->sStatus==DBSTATUS_S_OK)
					{
						rCol.SetLCID(*(LONG *)(pColumn->bValue));
						PRVTRACE(L"COLROWSET LCID:%d\n", *(LONG*) pColumn->bValue);
						fFound = TRUE;
					}
				} // if (iLCID)
				
				if (iBaseColName)
				{
					pColumn = (DATA *)(pRow + rgDBBINDING[iBaseColName].obStatus);
					if (pColumn->sStatus==DBSTATUS_S_OK)
					{
						DBID	ColumnID;

						ColumnID.eKind = DBKIND_NAME;
						ColumnID.uName.pwszName = (WCHAR*)(pColumn->bValue);
						PRVTRACE(L"COLROWSET BASECOLUMNNAME:%s\n", ColumnID.uName.pwszName);
						rCol.SetColID(&ColumnID);
						fFound = TRUE;
					}
				} // if (iLCID)
				
				if(fFound)
				{
					(*pcColsFound)++;					
					fFound = FALSE;
					goto NEXTROW;
				}
			} 
NEXTROW: 		
			ulTotalRows++;
		}

		// Need to release rows
		CHECK(hr=pColRowset->ReleaseRows
		(		 
			cRowsObtained,	// number of rows to release
			rghRows,	  	// array of row handles
			NULL,
			NULL,			// count of rows successfully released
			NULL			// there shouldn't be anymore references to these rows
		),S_OK);

		if(FAILED(hr))
			goto CLEANUP;
	}

	fResult	= TRUE;

CLEANUP:
	
	SAFE_DELETE(pRow);
	PROVIDER_FREE(rgOptionalColumns);
	SAFE_RELEASE(pColRowset);

	FreeAccessorBindings(cDBBINDING, rgDBBINDING);
	PROVIDER_FREE(rgDBCOLUMNINFO);
	PROVIDER_FREE(rgStringsBuffer);
	PROVIDER_FREE(rghRows);

	if (fResult == FALSE)
		return hr;

	return S_OK;
}


//---------------------------------------------------------------------------
// Sets module name by taking first 8 characters of strModuleName
// and then removing any non-alpha/numeric letters and replacing them with
// underscores.
//
//	@mfunc SetModuleName
//---------------------------------------------------------------------------
HRESULT CTable::SetModuleName(
	WCHAR * pwszModuleName		// @parm [IN] Module name
)
{
	if (pwszModuleName)
	{
		m_pwszModuleName = (WCHAR *)PROVIDER_ALLOC((sizeof(WCHAR) * MODULENAME) + sizeof(WCHAR));
		wcsncpy(m_pwszModuleName,pwszModuleName,MODULENAME);

		// Module name passed in, have to make sure it is first 8 chars are alpha-numerics
		for(int iIndex=0;iIndex<MODULENAME;iIndex++)
		{
			if ( (iswalnum(m_pwszModuleName[iIndex])) ==  FALSE)
				m_pwszModuleName[iIndex]=L'_';
		}	

		m_pwszModuleName[MODULENAME] = L'\0';
	}
	else	
	{
		// No module name passed in, so it is "DEFAULT", assign to member variable
		m_pwszModuleName = wcsDuplicate((WCHAR*)wszDEFAULT);
	}

	return S_OK;
}

//---------------------------------------------------------------------------
//	CTable::SetTableColumnInfo
//
//	mfunc	HRESULT					|
//			CTable					|
//			SetTableColumnInfo		|
//			Puts column information about pwszTableName table in m_ColList.
//
//	Called after you are sure table is in data source,
//	Grabs column information using IColumnsRowset::GetColumnsRowset,
//  IColumnsInfo::GetColumnsInfo, and IDBSchemaRowsets::GetSchemaRowset
//  for Types.
//
// 	rdesc HRESULT indicating success or failure
//
// 	flag S_OK   | Function ran without problem
// 	flag E_FAIL    | pwszTableName is empty or function ran with problems
//---------------------------------------------------------------------------
HRESULT CTable::SetTableColumnInfo(
	WCHAR * pwszTableName,
	IUnknown *pIRowset// parm ||[IN] Table name
)
{
	HRESULT 		hr				= 0;
	BOOL 			fResult			= FALSE;
	IColumnsInfo*	pIColumnsInfo	= NULL;

	//We need either need IOpenRowset to generate a rowset and ColInfo from
	//Or we need a passed in rowset to obtain the ColInfo from
	if(m_pIOpenRowset==NULL && pIRowset==NULL)
		return E_FAIL;

	//Set TableID
	if(pwszTableName && FAILED(SetTableName(pwszTableName)))
		goto CLEANUP;

	if(pIRowset)
	{
		if(!VerifyInterface(pIRowset, IID_IColumnsInfo, ROWSET_INTERFACE, (IUnknown**)&pIColumnsInfo))
		{
			hr = E_NOINTERFACE;
			goto CLEANUP;
		}
	}
	else
	{
		// Now get the rowset which we'll use to populate the table
		TESTC_(hr = CreateRowset(USE_OPENROWSET, IID_IColumnsInfo, 0, NULL, (IUnknown **)&pIColumnsInfo),S_OK);
	}
	
	TESTC_(hr = InitSchema(pIColumnsInfo),S_OK);

CLEANUP:

	SAFE_RELEASE(pIColumnsInfo);
	return hr;
}


//---------------------------------------------------------------------------
//	CTable::GetTableColumnInfo
//
//	mfunc	HRESULT					|
//			CTable					|
//			GetTableColumnInfo		|
//			Puts column information about pwszTableName table in m_ColList.
//
//	Called after you are sure table is in data source,
//	Grabs column information using IColumnsRowset::GetColumnsRowset,
//  IColumnsInfo::GetColumnsInfo. Very much alike SetTableColumnsInfo, 
//	but preserves the info read!
//
// 	rdesc HRESULT indicating success or failure
//
// 	flag S_OK   | Function ran without problem
// 	flag E_FAIL    | pwszTableName is empty or function ran with problems
//---------------------------------------------------------------------------
HRESULT CTable::GetTableColumnInfo(
	DBID * pTableID,
	IUnknown *pIRowset// parm ||[IN] Table name
)
{
	HRESULT 		hr				= 0;
	DBORDINAL		index			= 0;
	BOOL 			fResult			= FALSE;
	IColumnsRowset* pIColumnsRowset = NULL;
	IColumnsInfo *	pIColumnsInfo	= NULL;
	DBORDINAL		cDBCOLUMNINFO	= 0;
	DBCOLUMNINFO *	rgDBCOLUMNINFO	= NULL;
	WCHAR *			pStringsBuffer	= NULL;

	DBORDINAL 		cColsColRowset	= 0;	// count of columns
	DBORDINAL		cColsColInfo	= 0;	// count of column objects in GetColumnsInfo 
	DBROWCOUNT		cRowsAffected	= 0;	// Count of rowsets
	CCol			col;

	// say nothing is known about the optional columns in IColumnsRowset
	SetHasAuto(FALSE);
	SetHasUnique(FALSE);
	SetHasCLSID(FALSE);
	SetHasDefault(FALSE);

	//We need either need IOpenRowset to generate a rowset and ColInfo from
	//Or we need a passed in rowset to obtain the ColInfo from
	if(m_pIOpenRowset==NULL && pIRowset==NULL)
		return E_FAIL;

	if(pIRowset)
	{
		if(!VerifyInterface(pIRowset, IID_IColumnsInfo, ROWSET_INTERFACE, (IUnknown**)&pIColumnsInfo))
		{
			hr = E_NOINTERFACE;
			goto CLEANUP;
		}
	}
	else
	{
		const ULONG		cProps		= 1;
		const ULONG		cPropSets	= 1;
		DBPROP			rgProps[cProps];
		DBPROPSET		rgPropSets[cPropSets];

		rgPropSets[0].cProperties		= cProps;
		rgPropSets[0].rgProperties		= rgProps;
		rgPropSets[0].guidPropertySet	= DBPROPSET_ROWSET;
		memset(rgProps, 0, sizeof(DBPROP));
		rgProps[0].dwPropertyID			= DBPROP_CANHOLDROWS;
		rgProps[0].dwOptions			= DBPROPOPTIONS_OPTIONAL;
		rgProps[0].vValue.vt			= VT_BOOL;
		V_BOOL(&rgProps[0].vValue)		= VARIANT_TRUE;

		// Now get the rowset which we'll use to populate the table
		if(FAILED(hr = CreateRowset(USE_OPENROWSET, IID_IColumnsInfo, cPropSets, rgPropSets, (IUnknown **)&pIColumnsInfo, pTableID)))
			goto CLEANUP;
	}
	
	//GetColumnInfo
	if (!CHECK(pIColumnsInfo->GetColumnInfo(&cDBCOLUMNINFO,&rgDBCOLUMNINFO,&pStringsBuffer),S_OK))
		goto CLEANUP;

	// you wouldn't like to add to the old list, right?
	m_ColList.RemoveAll();
	
	for(index=0;index<cDBCOLUMNINFO;index++)
	{
		if (rgDBCOLUMNINFO[index].iOrdinal!=0)
		{
			//Set the ColumnInfo into this CCol struct
			col.SetColInfo(&rgDBCOLUMNINFO[index]);

			//Add it to the list
			m_ColList.AddTail(col);
			cColsColInfo++;
		}
	}

	m_fColProps = READ_DBPROP_COL_FIXEDLENGTH | READ_DBPROP_COL_NULLABLE;

	if(VerifyInterface(pIColumnsInfo, IID_IColumnsRowset, ROWSET_INTERFACE, (IUnknown**)&pIColumnsRowset))
	{
		if (!CHECK(hr=GetFromColumnsRowset(pIColumnsRowset, &cColsColRowset),S_OK))
			goto CLEANUP;		

		if (cColsColInfo!= cColsColRowset)
		{
			hr = E_FAIL;
			goto CLEANUP;
		}
	}

	fResult = TRUE;

CLEANUP:
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIColumnsRowset);

	PROVIDER_FREE(rgDBCOLUMNINFO);
	PROVIDER_FREE(pStringsBuffer);

	return fResult ? S_OK : E_FAIL;
}


//---------------------------------------------------------------------------
//	CTable::GetQueryInfo
//
//
//
// 	rdesc HRESULT indicating success or failure
//
// 	flag S_OK   | Function ran without problem
// 	flag E_FAIL    | pwszTableName is empty or function ran with problems
//---------------------------------------------------------------------------

BOOL CTable::GetQueryInfo
(
	EQUERY			sqlStmt,				// @parm [in] SQL statement
	DBORDINAL *		pcColumns1,				// @parm [out] 1.Count of columns
	DB_LORDINAL **	prgColumns1,			// @parm [out] 1.Array of Base Table Column Ordinals
	WCHAR ***		prgColumnNames1,		// @parm [out] 1.Column names
	DBORDINAL *		pcColumns2,				// @parm [out] 2.Count of columns
	DB_LORDINAL **	prgColumns2,			// @parm [out] 2.Array of Base Table Column ordinals
	WCHAR ***		prgColumnNames2			// @parm [out] 2.Column names
)
{
	DBORDINAL	cColumns=0;					// 1. count of columns in query, 50 = max # of columns to return
	DB_LORDINAL *	rgColumns=NULL;				// 1. array of column ordinals (ULONG cause it's that way in CCol)

	DBORDINAL	cColumns2=0;				// 2. count of columns in query 2, 50 = max # of columns to return
	DB_LORDINAL *	rgColumns2=NULL;			// 2. array of column ordinals

	WCHAR **	rgColumnNames=NULL;			// array of column names 1
	WCHAR **	rgColumnNames2=NULL;		// array of column names 2

	WCHAR *		pwszColList=NULL;			// string of column names, comes back comma separated
	CCol		col;						// temporary column object

	DBORDINAL	cNewCol=0;					// 1. count of columns in query
	DB_LORDINAL *	rgNewColOrd=NULL;			// 1. array of column ordinals (ULONG cause it's that way in CCol)
	WCHAR **	rgNewColName=NULL;			// array of column names 1
	
	switch(sqlStmt)
	{
		// Forward, all columns
		case USE_OPENROWSET:							// IOpenRowset
		case USE_SUPPORTED_SELECT_ALLFROMTBL:			// IOpenRowset or "SELECT * FROM <tbl>"
		case SELECT_ALLFROMTBL:							// "SELECT * FROM <tbl>"
		case SELECT_DISTINCTCOLLISTORDERBY:				// "Select DISTINCT <col list> from <tbl> order by <col one> DESC"
		case SELECT_COLLISTWHERELASTCOLINSELECT:		// "Select <col list> from <tbl> where <last col> in (Select <last col> from <tbl>)"
		case SELECT_EMPTYROWSET:						// "Select <col list> from <tbl> where 0=1"
		case SELECT_COLLISTFROMTBL:						// "Select <col list> from <tbl>"
		case SELECT_COLLISTTBLUNIONTBL:					// "Select <col list> from <tbl> UNION select <col list> from <tbl>"
 		case SELECT_COLLISTORDERBYCOLONECOMPUTE:		// "Select <col list> from <tbl> ORDER BY <col one> COMPUTE SUM(<col one>)"
 		case SELECT_COLLISTORDERBYCOLONECOMPUTEBY:		// "Select <col list> from <tbl> ORDER BY <col one> COMPUTE SUM(<col one>) by <col one>"
		case SELECT_ORDERBYNUMERIC:						
			CreateColList(FORWARD,&pwszColList,&cColumns,&rgColumns,ALL_COLS_IN_LIST,FALSE,FALSE,&rgColumnNames);
			break;
		
		// Reverse, all columns
		case SELECT_REVCOLLIST:							// "Select <reverse col list> from <tbl>"
		case SELECT_REVCOLLISTFROMVIEW:					//"Select <reverse col list> from <view>"
			CreateColList(REVERSE,&pwszColList, &cColumns, &rgColumns,ALL_COLS_IN_LIST, FALSE,FALSE,&rgColumnNames);
			break;

		// "Select 'ABC', <col list> from <tbl>"
		// Result set's first column doesn't have a name
		case SELECT_ABCANDCOLLIST:
			// Create array of column numbers
			cColumns = m_ColList.GetCount() + 1;

			//Allocate our own memory so CreateColList doesn't do it for us,
			//adding room for ABC col
			rgColumns = (DB_LORDINAL *) PROVIDER_ALLOC(sizeof(DB_LORDINAL) *(cColumns));
			ASSERT(rgColumns);

			// -1 means that first column does not have column name or number
			(rgColumns)[0] = -1;
			
			// Create the column list and array of column numbers
			CreateColList(REVERSE,&pwszColList, &cColumns, &rgColumns,ALL_COLS_IN_LIST,
				FALSE,FALSE,&rgColumnNames);
			// NOTE: Since the first column will not have a column name, I don't
			// add a name here but instead leave it as null. 
			break;

		// column 1 only
		case SELECT_COLLISTGROUPBY:// "Select <col one> from <tbl> GROUP BY <col one> HAVING <col one> is not null"
			// Get column one number and name
			GetColInfo(1, col);
			
			// column count
			cColumns = 1;
			
			// column ordinal
			rgColumns = (DB_LORDINAL *) PROVIDER_ALLOC(sizeof(DB_LORDINAL) * cColumns);
			(rgColumns)[0] = col.GetColNum();
			
			// column name
			rgColumnNames = (WCHAR **) PROVIDER_ALLOC(sizeof(WCHAR *) * cColumns);
			rgColumnNames[0] = col.GetColName();
			break;

		// 1 column, no name
		case SELECT_COUNT:// "Select count(<col one>) from <tbl>" Doesn't name the single column of result set

			cColumns = 1;
			rgColumns = (DB_LORDINAL *) PROVIDER_ALLOC(sizeof(DB_LORDINAL));
			rgColumns[0] = -1;
			rgColumnNames = (WCHAR **) PROVIDER_ALLOC(cColumns * sizeof(WCHAR *));
			// return array of 1 element, whose value is null
			rgColumnNames[0]=NULL;
			
			cColumns2=0;
			break;

		// "Select <col list> from <tbl>; Select <reverse col list> from <tbl>"
		case SELECT_COLLISTSELECTREVCOLLIST:

			// Create the column list and array of column numbers
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns,ALL_COLS_IN_LIST,	FALSE, FALSE, &rgColumnNames);
			PROVIDER_FREE(pwszColList);
			
			// We want the forward column list since that is the first rowset
			// TODO: Check that this works
			CreateColList(REVERSE,&pwszColList, &cColumns2, &rgColumns2,ALL_COLS_IN_LIST, FALSE, FALSE, &rgColumnNames2);
			break;

		case SELECT_LEFTOUTERJOIN: // "Select * from <tbl1> LEFT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>"
		case SELECT_RIGHTOUTERJOIN:// "Select * from <tbl1> RIGHT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>"
		case SELECT_LEFTOUTERJOIN_ESC: // "Select * from { oj <tbl1> LEFT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>}"
		case SELECT_RIGHTOUTERJOIN_ESC:// "Select * from { oj <tbl1> RIGHT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>}"
			PRVTRACE(L"** Privlib(CTable::GetQueryInfo):Select * query, can't guarantee column ordering\n");
			return FALSE;

		// "Select c1=col1,c1=col2... from <tbl>"
		// need to rebuild column list by renaming each column 
		case SELECT_CHANGECOLNAME:
		{
			WCHAR * pwszOldList=NULL;
			WCHAR * pwszNewList=NULL;
			size_t cOldListCharacters=0;
			size_t cOldList=0;
			size_t cNewList=0;
			ULONG iOldIndex=0;
			ULONG iNewIndex=0;
			ULONG iNameCopyStart=0;
			ULONG iNameCopyIndex=0;

			// Create the column list and array of column numbers
			CreateColList(FORWARD,&pwszOldList, &cColumns, &rgColumns,
				ALL_COLS_IN_LIST, FALSE, FALSE, &rgColumnNames);

			cOldListCharacters = wcslen(pwszOldList);
			
			cOldList = cOldListCharacters *sizeof(WCHAR);
			cOldList += sizeof(WCHAR);

			cNewList = 3 * cOldList;


			// Get memory for new col list
			pwszNewList = (WCHAR *) PROVIDER_ALLOC(cNewList);
			memset(pwszNewList,GARBAGE,cNewList);

			// Copy col list over
			for(iNewIndex=0,iOldIndex=0,iNameCopyStart=0; iOldIndex < cOldListCharacters ; iOldIndex++)
			{
				if(pwszOldList[iOldIndex]!=L',')
					pwszNewList[iNewIndex++]=pwszOldList[iOldIndex];

				if(pwszOldList[iOldIndex]==L',')
				{
					pwszNewList[iNewIndex++]=L'X';
					pwszNewList[iNewIndex++]=L'=';

					for(iNameCopyIndex=iNameCopyStart;iNameCopyIndex<iOldIndex;iNameCopyIndex++)
					{
						pwszNewList[iNewIndex++]=pwszOldList[iNameCopyIndex];
						iNameCopyStart++;
					}

					iNameCopyStart=iOldIndex+1;
					pwszNewList[iNewIndex++]=pwszOldList[iOldIndex];
				}
			}

			pwszNewList[iNewIndex++]=L'X';
			pwszNewList[iNewIndex++]=L'=';

			for(iNameCopyIndex=iNameCopyStart;iNameCopyIndex<(iOldIndex+1);iNameCopyIndex++)
			{
				pwszNewList[iNewIndex++]=pwszOldList[iNameCopyIndex];
				iNameCopyStart++;
			}

			pwszNewList[iNewIndex]=L'\0';

			PROVIDER_FREE(pwszNewList);
			PROVIDER_FREE(pwszOldList);
		}
		break;

		// "Select <col list> , <col list> from tbl"
		case SELECT_DUPLICATECOLUMNS:
			
			// Create the column list and array of column numbers
			CreateColList(FORWARD,&pwszColList, &cColumns, &rgColumns,ALL_COLS_IN_LIST,
				FALSE,FALSE,&rgColumnNames);

			cNewCol = cColumns * 2;
			rgNewColOrd	= (DB_LORDINAL *) PROVIDER_ALLOC(cNewCol * sizeof(DB_LORDINAL));
			rgNewColName = (WCHAR **) PROVIDER_ALLOC(cNewCol * sizeof(WCHAR *));

			{
				DBORDINAL i;
				for(i=0;i<cColumns;i++)
				{
					rgNewColOrd[i]=rgColumns[i];
					rgNewColName[i]=rgColumnNames[i];
				}

				for(i=cColumns;i<cNewCol;i++)
				{
					rgNewColOrd[i]=rgColumns[i-cColumns];
					rgNewColName[i]=rgColumnNames[i-cColumns];
				}
			}

			cColumns = cNewCol;
			PROVIDER_FREE(rgColumns);
			PROVIDER_FREE(rgColumnNames);
			rgColumns = rgNewColOrd;
			rgColumnNames = rgNewColName;
			break;

		// "Select <reverse col list> , <reverse col list> from <tbl>
		case SELECT_REVERSEDUPLICATECOLUMNS:

			// Create the column list and array of column numbers
			CreateColList(REVERSE,&pwszColList, &cColumns, &rgColumns,ALL_COLS_IN_LIST,
				FALSE,FALSE,&rgColumnNames);
			
			cNewCol = cColumns * 2;
			rgNewColOrd	= (DB_LORDINAL *) PROVIDER_ALLOC(cNewCol * sizeof(DB_LORDINAL));
			rgNewColName = (WCHAR **) PROVIDER_ALLOC(cNewCol * sizeof(WCHAR *));

			{
				DBORDINAL i;
				for(i=0;i<cColumns;i++)
				{
					rgNewColOrd[i]=rgColumns[i];
					rgNewColName[i]=rgColumnNames[i];
				}
				for(i=cColumns;i<cNewCol;i++)
				{
					rgNewColOrd[i]=rgColumns[i-cColumns];
					rgNewColName[i]=rgColumnNames[i-cColumns];
				}
			}

			cColumns = cNewCol;
			PROVIDER_FREE(rgColumns);
			PROVIDER_FREE(rgColumnNames);
			rgColumns = rgNewColOrd;
			rgColumnNames = rgNewColName;
			break;

		default:
			ASSERT(0);
				return FALSE;// Compiler needs this

	}
	PROVIDER_FREE(pwszColList);

	if(pcColumns1)
		*pcColumns1 = cColumns;
	if(prgColumnNames1)
		*prgColumnNames1 = rgColumnNames;
	else
		PROVIDER_FREE(rgColumnNames);
	if(prgColumns1)
		*prgColumns1 = rgColumns;
	else
		PROVIDER_FREE(rgColumns);

	if(pcColumns2)
		*pcColumns2 = cColumns2;
	if(prgColumns2)
		*prgColumns2 = rgColumns2;
	else
		PROVIDER_FREE(rgColumns2);
	if(prgColumnNames2)
		*prgColumnNames2 = rgColumnNames2;
	else
		PROVIDER_FREE(rgColumnNames2);

	return TRUE;
}

//---------------------------------------------------------------------------
//	CTable::GetFirstNumericCol
//
// 	rdesc HRESULT
//
// 	flag S_OK		| Function ran without problem
// 	flag E_FAIL			| No numeric columns available
// 	flag E_INVALIDARG	| pCCol was NULL
//---------------------------------------------------------------------------

HRESULT CTable::GetFirstNumericCol(
				CCol * pCCol	// @parm [in/out] Pointer to CCol object to copy 
								//first numeric column object into
)
{
	if (!pCCol)
		return E_INVALIDARG;
	
	// While not at end of list
	POSITION pos = m_ColList.GetHeadPosition();
	while(pos) 
	{		
		// Put column name in list
		CCol& rCol = m_ColList.GetNext(pos);	

		switch(rCol.GetProviderType())
		{
			//For now we'll consider only ints to be numeric
			//so we don't have to worry about rounding if
			//we add two together.  We assume that all providers
			//will support one of these types
			case DBTYPE_UI4:			
			case DBTYPE_I4:			
			case DBTYPE_UI8:
			case DBTYPE_I8:
			case DBTYPE_NUMERIC:
			case DBTYPE_R4:
			case DBTYPE_R8: 
			case DBTYPE_CY:
				*pCCol = rCol;
				return S_OK;
			break;

			default:
			break;
		}
	}

	//If we got to end without finding a numeric, fail
	return E_FAIL;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	PrintTable
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CTable::PrintCColInfo()
{
	DBORDINAL i=1;

	// While not at end of list
	POSITION pos = m_ColList.GetHeadPosition();
	while (pos!=NULL) 
	{		
		// Put column name in list
		CCol& rCol = m_ColList.GetNext(pos);

		if(rCol.GetColName())
			PRVTRACE(L"[%d].m_pwszColName=%s\n",i,rCol.GetColName());
		else
			PRVTRACE(L"[%d].m_pwszColName=<null>\n");

		if(rCol.GetProviderTypeName())
			PRVTRACE(L"[%d].m_pwszProviderTypeName=%s\n",i,rCol.GetProviderTypeName());
		else
			PRVTRACE(L"[%d].m_pwszProviderTypeName=<null>\n");

		if(rCol.GetPrefix())
			PRVTRACE(L"[%d].m_pwszPrefix=%s\n",i,rCol.GetPrefix());
		else
			PRVTRACE(L"[%d].m_pwszPrefix=<null>\n",i);

		if(rCol.GetSuffix())
			PRVTRACE(L"[%d].m_pwszSuffix=%s\n",i,rCol.GetSuffix());
		else
			PRVTRACE(L"[%d].m_pwszSuffix=<null>\n",i);

		if(rCol.GetCreateParams())
			PRVTRACE(L"[%d].m_pwszCreateParams=%s\n",i,rCol.GetCreateParams());
		else
			PRVTRACE(L"[%d].m_pwszCreateParams=<null>\n",i);

		PRVTRACE(L"[%d].m_wProviderType=%d\n",i,rCol.GetProviderType());
		PRVTRACE(L"[%d].m_lPrecision=%d\n",i,rCol.GetPrecision());
		PRVTRACE(L"[%d].m_ulColumnSize=%d\n",i,rCol.GetColumnSize());
		PRVTRACE(L"[%d].iOrdinal=%d\n",i,rCol.GetColNum());
		PRVTRACE(L"[%d].m_sNullable=%d\n",i,rCol.GetNullable());
		PRVTRACE(L"[%d].m_fUnsigned=%d\n",i,rCol.GetUnsigned());
		PRVTRACE(L"[%d].m_sScale=%d\n",i,rCol.GetScale());
		PRVTRACE(L"[%d].m_fAutoInc=%d\n",i,rCol.GetAutoInc());
		PRVTRACE(L"[%d].m_fUpdateable=%d\n",i,rCol.GetUpdateable());
		PRVTRACE(L"[%d].m_ulSearchable=%d\n",i,rCol.GetSearchable());
		PRVTRACE(L"[%d].m_fUseInSQL=%d\n",i,rCol.GetUseInSQL());
		PRVTRACE(L"[%d].m_fIsLong=%d\n",i,rCol.GetIsLong());
		i++;
	}
}


//---------------------------------------------------------------------------
// CTable::SetExistingCols
//	
// @mfunc	SetExistingCols
//	
//---------------------------------------------------------------------------
HRESULT CTable::SetExistingCols(
	CList<CCol,CCol&>  &InColList
)
{
	m_ColList.RemoveAll();

	POSITION pos = InColList.GetHeadPosition();
	while(pos)
	{
		CCol& rCol = InColList.GetNext(pos);
		m_ColList.AddTail(rCol);
	}
	
	ASSERT(m_ColList.GetCount() == InColList.GetCount());
	return S_OK;
}


//---------------------------------------------------------------------------
//	CTable::GetOrderByCol
//
// 	rdesc HRESULT
//
// 	flag S_OK		| Function ran without problem
// 	flag E_FAIL			| No numeric columns available
// 	flag E_INVALIDARG	| pCCol was NULL
//---------------------------------------------------------------------------

HRESULT CTable::GetOrderByCol(
				CCol * pCCol	// @parm [in/out] Pointer to CCol object to copy 
								//first numeric column object into
)
{
	DBORDINAL	iOrdinal  =1;

	if (!pCCol)
		return E_INVALIDARG;
	
	// ok! the fix lies in the way we generate data ( NULLS diagonally)and the way we
	// create a unique index - always on the first column.( so we are guaranteed only one null in this column)
	//  if the default null collation is at the low order of the list
	// we are safe, seed for the lowest row matches the position of NULL in the rowset.
	POSITION pos = m_ColList.GetHeadPosition();
	while(pos)
	{
		// Get next object in list
		CCol& rCol = m_ColList.GetNext(pos);

		// Check for col num
		if(rCol.GetColNum() == iOrdinal)
		{
			*pCCol = rCol;
			return S_OK;
		}
	}
	
	//TODO remove this - just leaving it  here if the above solution fails 
	// Get top of list
	
	// While not at end of list
	pos = m_ColList.GetHeadPosition();
	while (pos) 
	{		
		// Put column name in list
		CCol& rCol = m_ColList.GetNext(pos);	

		// this will gauarntee no ordering with respect to the newly inserted rows.
		// they will get appended to the end - even if you generate data using a seed
		// for a "hole" in the table but this is what most of the rowset tests expect
		if (!rCol.GetUpdateable() && !rCol.GetNullable())
		{
			*pCCol = rCol;
			return S_OK;
		}
	}
	
	// Get top of list
	pos = m_ColList.GetHeadPosition();
	while (pos) 
	{		
		// Put column name in list
		CCol& rCol = m_ColList.GetNext(pos);	

		// try an build an order by clause on a fixed length, non nullable column
		if ( rCol.GetIsFixedLength() && !rCol.GetNullable())
		{
			*pCCol = rCol;
			return S_OK;
		}
	}
	
	
	//If we got to end without finding a numeric, fail
	return E_FAIL;
}


// @cmember Sets pointer to the return table DBID (in ITableDefinition::CreateTable)
DBID	**CTable::SetDBID(DBID **ppTableID) 
{
	if (m_ppTableID) 
		delete m_ppTableID;
	return m_ppTableID = ppTableID; 
}

// @cmember Gets pointer to the return table DBID created by ITableDefinition::CreateTable
inline DBID **CTable::GetDBID(void) 
{
	return m_ppTableID; 
}

// @cmember Sets pointer to the rowset interface to be created by ITableDefinition::CreateTable
IUnknown **CTable::SetRowset(IUnknown **ppRowset) 
{
	if (m_ppRowset) 
	{
		if (*m_ppRowset)
			(*m_ppRowset)->Release();
	}
	return m_ppRowset = ppRowset; 
}

// @cmember,mfunc Sets TableID
void CTable::SetTableID(DBID dbid)
{
	ReleaseDBID(&m_TableID, FALSE);	// release its content only
	DuplicateDBID(dbid, &m_TableID);
}	

// @cmember Gets the pointer to the rowset interface created by ITableDefinition::CreateTable
inline IUnknown **CTable::GetRowset(void) 
{
	return m_ppRowset; 
}


// @cmember Sets the property sets
DBPROPSET *CTable::SetPropertySets
(
   DBPROPSET *rgPropertySets,	// [in] array of rowset properties
   ULONG	cPropertySets		// [in]	array size (default 0)
) 
{
	if (rgPropertySets)
	{
		// release the current allocated memory
		FreeProperties(&m_cPropertySets, &m_rgPropertySets);
		m_cPropertySets = cPropertySets;
		return m_rgPropertySets = rgPropertySets; 
	}
	else
	{
		// using NULL as input parameter saves the memory from being released
		// usefull 
		rgPropertySets		= m_rgPropertySets;
		m_rgPropertySets	= NULL;
		m_cPropertySets		= 0;
		return rgPropertySets;
	}
}
	

// @cmember Sets the column description array
DBCOLUMNDESC *CTable::SetColumnDesc(DBCOLUMNDESC *rgColumnDesc)
{
	if (rgColumnDesc)
	{
		// release the old column descriptions array 
		ReleaseColumnDesc(m_rgColumnDesc, m_cColumnDesc);
		return m_rgColumnDesc= rgColumnDesc; 
	}
	else
	{
		// the function was called just to get the memory used by m_rgColumnDesc
		// so that it won't be released next time when m_rgColumnDesc is set
		rgColumnDesc	= m_rgColumnDesc;
		m_rgColumnDesc	= NULL;
		return rgColumnDesc;
	}
}
	

// @cmember Sets the column description array
DBCOLUMNDESC *CTable::SetColumnDesc(DBCOLUMNDESC *rgColumnDesc, DBORDINAL cColumnDesc)
{
	if (rgColumnDesc)
	{
		// release the old column descriptions array 
		ReleaseColumnDesc(m_rgColumnDesc, m_cColumnDesc);
		m_cColumnDesc	= cColumnDesc;
		return m_rgColumnDesc= rgColumnDesc; 
	}
	else
	{
		// the function was called just to get the memory used by m_rgColumnDesc
		// so that it won't be released next time when m_rgColumnDesc is set
		m_cColumnDesc	= 0;
		rgColumnDesc	= m_rgColumnDesc;
		m_rgColumnDesc	= NULL;
		return rgColumnDesc;
	}
}
	

// @cmember Duplicate the column list, it's user responsability to free it
CList <CCol, CCol&>	&CTable::DuplicateColList(CList <CCol, CCol&> &ColList)
{
	ColList.RemoveAll();
	POSITION pos = m_ColList.GetHeadPosition();
	while (pos)
	{
		CCol& rCol = m_ColList.GetNext(pos);
		ColList.AddTail(rCol);
	}
	
	return ColList;
}

// @cmember Set the column list, it's user responsability to free it
CList <CCol, CCol&>	&CTable::SetColList(CList <CCol, CCol&> &ColList)
{
	m_ColList.RemoveAll();
	POSITION pos = ColList.GetHeadPosition();
	while (pos)
	{
		CCol& rCol = ColList.GetNext(pos);
		m_ColList.AddTail(rCol);
	}
	return m_ColList;
}


// @cmember Build an array of ColumnDesc from m_ColList
BOOL CTable::ColList2ColumnDesc(
	DBCOLUMNDESC	**prgColumnDesc,		// [OUT] An array of ColumnDesc build from m_ColList
	DBORDINAL		*pcColumnDesc
)
{
	TBEGIN
	POSITION		pos;
	DBORDINAL		cColumns;
	ULONG			cProps = 0;
	DBCOLUMNDESC	*rgColumnDesc;
	DBCOLUMNDESC	*pColumnDesc;

	TESTC(prgColumnDesc && pcColumnDesc);

	if (*prgColumnDesc)
		ReleaseColumnDesc(*prgColumnDesc, *pcColumnDesc);

	*prgColumnDesc	= NULL;
	*pcColumnDesc	= 0;

	SAFE_ALLOC(rgColumnDesc, DBCOLUMNDESC, CountColumnsOnTable());
	
	cColumns = 0;
	for (pColumnDesc = rgColumnDesc, pos = m_ColList.GetHeadPosition(); pos; )
	{
		CCol& rCol = m_ColList.GetNext(pos);

		if (0 == rCol.GetColNum())
		{
			continue;
		}

		// build the col accordingly
		pColumnDesc->pwszTypeName	= wcsDuplicate(rCol.GetProviderTypeName());
		pColumnDesc->pTypeInfo		= rCol.GetTypeInfo();
		if (GetHasCLSID() && GUID_NULL != rCol.GetTypeGuid())
		{
			pColumnDesc->pclsid			= (GUID*)PROVIDER_ALLOC(sizeof(GUID));
			if (NULL != pColumnDesc->pclsid)
				memcpy(pColumnDesc->pclsid, &rCol.GetTypeGuid(), sizeof(GUID));
		}
		else
			pColumnDesc->pclsid			= NULL;
		pColumnDesc->ulColumnSize	= rCol.GetColumnSize();
		pColumnDesc->wType			= rCol.GetProviderType();
		pColumnDesc->bPrecision		= rCol.GetPrecision();
		pColumnDesc->bScale			= rCol.GetScale();
		memcpy(&pColumnDesc->dbcid, rCol.GetColID(), sizeof(DBID));
		switch (rCol.GetColID()->eKind)
		{
		case DBKIND_GUID_NAME:
		case DBKIND_PGUID_NAME:
		case DBKIND_NAME:
			pColumnDesc->dbcid.uName.pwszName = wcsDuplicate(rCol.GetColName());
			break;
		default:
			ASSERT(FALSE);
		}
		// set the preoperties!
		pColumnDesc->cPropertySets	= 1;
		pColumnDesc->rgPropertySets	= (DBPROPSET*) PROVIDER_ALLOC(sizeof(DBPROPSET));
		pColumnDesc->rgPropertySets[0].guidPropertySet	= DBPROPSET_COLUMN;
		pColumnDesc->rgPropertySets[0].cProperties	= rCol.GetHasDefault() ? 6 : 5;
		// alloc memory for properties
		SAFE_ALLOC(pColumnDesc->rgPropertySets[0].rgProperties, DBPROP, pColumnDesc->rgPropertySets[0].cProperties);
		memset(pColumnDesc->rgPropertySets[0].rgProperties, 0, pColumnDesc->rgPropertySets[0].cProperties * sizeof(DBPROP));

		cProps = 0;

		pColumnDesc->rgPropertySets[0].rgProperties[cProps].dwPropertyID	= DBPROP_COL_AUTOINCREMENT;
		pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue.vt = VT_BOOL;
		V_BOOL(&pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue) = rCol.GetAutoInc()? VARIANT_TRUE: VARIANT_FALSE;
		cProps++;

		pColumnDesc->rgPropertySets[0].rgProperties[cProps].dwPropertyID	= DBPROP_COL_FIXEDLENGTH;
		pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue.vt = VT_BOOL;
		V_BOOL(&pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue) = rCol.GetIsFixedLength()? VARIANT_TRUE: VARIANT_FALSE;
		cProps++;
		
		pColumnDesc->rgPropertySets[0].rgProperties[cProps].dwPropertyID	= DBPROP_COL_NULLABLE;
		pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue.vt = VT_BOOL;
		V_BOOL(&pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue)= rCol.GetNullable()? VARIANT_TRUE: VARIANT_FALSE;
		cProps++;
		
		pColumnDesc->rgPropertySets[0].rgProperties[cProps].dwPropertyID	= DBPROP_COL_UNIQUE;
		pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue.vt = VT_BOOL;
		V_BOOL(&pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue) = rCol.GetUnique()? VARIANT_TRUE: VARIANT_FALSE;
		cProps++;
		
		if (SupportedProperty(DBPROP_COL_DESCRIPTION, DBPROPSET_COLUMN, m_pIDBInitialize))
		{
			pColumnDesc->rgPropertySets[0].rgProperties[cProps].dwPropertyID	= DBPROP_COL_DESCRIPTION;
			pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue.vt = rCol.GetColDescription()? VT_BSTR: VT_NULL;
			V_BSTR(&pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue) = SysAllocString(rCol.GetColDescription());
			cProps++;
		}

		if (rCol.GetHasDefault())
		{
			VARIANT	*pV = DBTYPE2VARIANT(&rCol.GetDefaultValue(), rCol.GetSubType());
			pColumnDesc->rgPropertySets[0].rgProperties[cProps].dwPropertyID	= DBPROP_COL_DEFAULT;
			VariantInit(&pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue);
			VariantCopy(&pColumnDesc->rgPropertySets[0].rgProperties[cProps].vValue, pV);
			VariantClear(pV);
			PROVIDER_FREE(pV);
			cProps++;
		}
		pColumnDesc->rgPropertySets[0].cProperties	= cProps;

		cColumns++;
		pColumnDesc++;
	}

	*pcColumnDesc = cColumns;
	*prgColumnDesc = rgColumnDesc;

CLEANUP:
	TRETURN
} //CTable::ColList2ColumnDesc



// @cmember Fill m_ColList with information about current table
// This function is intended for the situation when the array of columndesc is build by the user
// and m_colList have to be actualyzed
BOOL CTable::ColumnDesc2ColList(
	DBCOLUMNDESC	*rgColumnDesc,		// [IN] An array of ColumnDesc 
	DBORDINAL		cColumnDesc			// [IN] number of elements in ColumnDesc array
)
{
	CList <CCol, CCol&>	ColTypes;			// the list of provider types 
	POSITION			pos;
	CCol				typeCol;
	CCol				col;
	DBORDINAL			i;
	BOOL				fNotFound = TRUE;

	// update m_ColList to whatever is in m_rgColumnDesc
	// These are necessary until CreateColInfo takes pointers
	CList <WCHAR * ,WCHAR *>	NativeTypesList;
	CList <DBTYPE,DBTYPE>		ProviderTypesList;
	ULONG						ulAutoIncPrec;

	if(FAILED(CreateTypeColInfo(NativeTypesList, ProviderTypesList, ALLTYPES, &ulAutoIncPrec)))
		goto CLEANUP;
	DuplicateColList(ColTypes);

	m_ColList.RemoveAll();
	for (i=0; i<cColumnDesc; i++)
	{
		fNotFound = TRUE;

		// try to retrieve the CCol, using the type name
		if (rgColumnDesc[i].pwszTypeName)
			// but,  what if the type name is not caught?
			for (	pos = ColTypes.GetHeadPosition(), typeCol=ColTypes.GetNext(pos);
					(fNotFound=wcscmp(typeCol.GetProviderTypeName(), rgColumnDesc[i].pwszTypeName)) && pos;
					typeCol=ColTypes.GetNext(pos)
				);
		// give IT another chance, to get the type after its wType
		if (fNotFound)
		{
			// find the type closest to the one sought
			// same wType and min ulColumnsSize > the one asked
			for ( pos = ColTypes.GetHeadPosition(); pos;)
			{
				col = ColTypes.GetNext(pos);
				if (col.GetProviderType() == rgColumnDesc[i].wType)
				{
					if (	fNotFound 
						||	(col.GetColumnSize() > rgColumnDesc[i].ulColumnSize
							&& col.GetColumnSize() < typeCol.GetColumnSize()))
					{
						typeCol = col;
					}
					fNotFound = FALSE;
				}
			}
		}
		if (fNotFound)
			// give up; guy's a looney!
			continue;
		typeCol.SetColName(rgColumnDesc[i].dbcid.uName.pwszName);
		typeCol.SetColumnSize(rgColumnDesc[i].ulColumnSize);
		typeCol.SetPrecision(rgColumnDesc[i].bPrecision);
		typeCol.SetScale(rgColumnDesc[i].bScale);
		typeCol.SetColNum(i+1);
		typeCol.SetUpdateable(TRUE);
		typeCol.SetAutoInc(FALSE);
		typeCol.SetColDescription(NULL);

		// go for properties
		if (NULL != rgColumnDesc[i].rgPropertySets)
		{
			for (ULONG cPropSets=0; cPropSets<rgColumnDesc[i].cPropertySets; cPropSets++)
			{
				if (	NULL == rgColumnDesc[i].rgPropertySets[cPropSets].rgProperties
					||	DBPROPSET_COLUMN != rgColumnDesc[i].rgPropertySets[cPropSets].guidPropertySet)
					continue;
				for (ULONG cProp=0; cProp<rgColumnDesc[i].rgPropertySets[cPropSets].cProperties; cProp++)
				{
					DBPROP	*pProp = &rgColumnDesc[i].rgPropertySets[cPropSets].rgProperties[cProp];
					
					switch (pProp->dwPropertyID)
					{
						case DBPROP_COL_DEFAULT:
							typeCol.SetHasDefault(TRUE);
							typeCol.SetDefaultValue(pProp->vValue);
							break;
						case DBPROP_COL_AUTOINCREMENT:
							{
								BOOL	fSetAutoInc;

								if (VT_BOOL == pProp->vValue.vt)
									fSetAutoInc = VARIANT_TRUE == V_BOOL(&pProp->vValue);
								typeCol.SetAutoInc(fSetAutoInc);
								if (fSetAutoInc)
									typeCol.SetNullable(0);
							}
							break;
						case DBPROP_COL_UNIQUE:
							typeCol.SetUnique(VARIANT_TRUE == V_BOOL(&pProp->vValue));
							break;
						case DBPROP_COL_NULLABLE:
							typeCol.SetNullable(VARIANT_TRUE == V_BOOL(&pProp->vValue)? 1: 0);
							break;
						case DBPROP_COL_DESCRIPTION:
							typeCol.SetColDescription(V_BSTR(&pProp->vValue));
							break;
					}
				}
			}
		}

		// if necessary, extend to the other members
		m_ColList.AddTail(typeCol);
	}

CLEANUP:
	return TRUE;
} //CTable::ColumnDesc2ColList


// @cmember Sets the column description array
BOOL CTable::IsCompatibleType(
	DBCOUNTITEM		cBindings,
	DBBINDING*		rgBindings,
	void*			pData,
	CCol &			NewCol			// @parm [IN] Element from the CCol List
	)
{
	ASSERT(NewCol.GetReqParams()==ULONG_MAX || NewCol.GetReqParams() <= 2); // need additional supports if we encounter a types with 3 or more col def params

	if(_wcsicmp((WCHAR*)&VALUE_BINDING(rgBindings[0], pData), NewCol.GetProviderTypeName()))
		return FALSE;

	if (NewCol.GetReqParams()==ULONG_MAX)
		return TRUE;	// just return the first type name that matches.

	if (NewCol.GetReqParams()==0 && (STATUS_BINDING(rgBindings[5], pData) == DBSTATUS_S_ISNULL))
		return TRUE;

	if (STATUS_BINDING(rgBindings[5], pData) != DBSTATUS_S_ISNULL && 
		wcsstr((WCHAR *)(&VALUE_BINDING(rgBindings[5], pData)), L",") && 
		NewCol.GetReqParams()==2)  // comma means two params
		return TRUE;

	if (STATUS_BINDING(rgBindings[5], pData) != DBSTATUS_S_ISNULL && 
		NULL == wcsstr((WCHAR *)(&VALUE_BINDING(rgBindings[5], pData)), L",") && 
		NewCol.GetReqParams()==1)
		return TRUE;	

	return FALSE;
}


//-----------------------------------------------------------------------------
// @cmember Builds a variant which can represent the default value of a type
//
//-----------------------------------------------------------------------------
BOOL CTable::BuildDefaultValue(
	CCol&	rCol,				// [in]		used to indicate the column
	DBCOUNTITEM	cRow,				// [in]		value used to build the default value
	VARIANT	*pvDefault/*=NULL*/	// [out]	pointer to a variant to be built
)
{
	//		col is good as input parameter because there are multiple ways to indicate the type
	// on which you want to build a default value: type name, type descriptor (DBTYPE), a
	// column (wherefrom to grab the previous kind of info), etc...
	//		one can call the most convenient variant of GetColInfo prior to calling this method 
	// and get the col of the table
	WCHAR	*pwszDefault	= NULL;
	BOOL	fRes			= FALSE;

	TESTC(NULL != pvDefault);

	// build a value for this column
	if (S_OK != GetLiteralAndValue(rCol, &pwszDefault, cRow, rCol.GetColNum(), PRIMARY))
	{
		goto CLEANUP;
	}

	// set the variant
	VariantInit(pvDefault);
	pvDefault->vt		= DBTYPE_BSTR;
	V_BSTR(pvDefault)	= SysAllocString(pwszDefault);
	fRes				= TRUE;

CLEANUP:
	SAFE_FREE(pwszDefault);
	return fRes;
} //CTable::BuildDefaultValue



//-----------------------------------------------------------------------------
// @cmember Sets the default value of a column
//
//-----------------------------------------------------------------------------
BOOL CTable::SetDefaultValue(
	CCol&	rCol,				// [in]		used to indicate the column
	DBCOUNTITEM	cRow				// [in]		value used to build the default value
)
{
	VARIANT		vDefault;
	BOOL		fRes			= FALSE;

	VariantInit(&vDefault);
	// build a value for this column
	if (!BuildDefaultValue(rCol, cRow, &vDefault))
		goto CLEANUP;

	rCol.SetDefaultValue(vDefault);
	rCol.SetHasDefault(TRUE);
	fRes = TRUE;
CLEANUP:
	VariantClear(&vDefault);
	return fRes;
} //CTable::BuildDefaultValue



//-----------------------------------------------------------------------------
// @cmember Sets the has default value of a column
//
//-----------------------------------------------------------------------------
BOOL CTable::SetHasDefaultValue(CCol& rCol, BOOL fHasDefaultValue)
{
	BOOL		fRes = FALSE;

	rCol.SetHasDefault(fHasDefaultValue);
	fRes = TRUE;
	return fRes;
} //CTable::SetHasDefaultValue


//-----------------------------------------------------------------------------
// @cmember Gets a column that satisfies several criteria
//
//-----------------------------------------------------------------------------
BOOL CTable::GetColWithAttr(ULONG cCond, ULONG *rgCond, DBORDINAL *pcSelectedColumn)
{
	ULONG		iCond	= 0;
	BOOL		fFound	= TRUE;
	BOOL		fNotCondition;
	ULONG		curCond;

	if (	(NULL == pcSelectedColumn)
		||	(NULL == rgCond && 0 != cCond))
		return FALSE;

	POSITION pos = m_ColList.GetHeadPosition();
	while(pos)
	{
		CCol& rCol = m_ColList.GetNext(pos);
		fFound	= TRUE;		
		for (iCond=0; iCond<cCond && fFound; iCond++)
		{
			//does this condition have the COL_NOT_COND bits?
			fNotCondition	= rgCond[iCond] & COL_NOT_COND;
			
			//yank out the COL_NOT_COND bits if they are there
			curCond	= rgCond[iCond] & (~COL_NOT_COND);
		
			switch (curCond)
			{
				case COL_COND_NULL:
					//check if the Condition was asked to be allowed and the Condition was not available on the column
					//		if the Condition was asked not to be allowed and the Condition was available on the column or
					//then this column does not meet the needs
					if	(	(!fNotCondition && !rCol.GetNullable())	||
							(fNotCondition && rCol.GetNullable())
						)
					{
						fFound = FALSE;
					}
					break;
				case COL_COND_DEFAULT:
					//check if the Condition was asked to be allowed and the Condition was not available on the column
					//		if the Condition was asked not to be allowed and the Condition was available on the column or
					//then this column does not meet the needs
					if	(	(!fNotCondition && !rCol.GetHasDefault())	||
							(fNotCondition && rCol.GetHasDefault())
						)
					{
						fFound = FALSE;
					}
					break;
				case COL_COND_UNIQUE:
					//check if the Condition was asked to be allowed and the Condition was not available on the column
					//		if the Condition was asked not to be allowed and the Condition was available on the column or
					//then this column does not meet the needs
					if	(	(!fNotCondition && !rCol.GetUnique())	||
							(fNotCondition && rCol.GetUnique())
						)
					{
						fFound = FALSE;
					}
					break;
				case COL_COND_UPDATEABLE:
					//check if the Condition was asked to be allowed and the Condition was not available on the column
					//		if the Condition was asked not to be allowed and the Condition was available on the column or
					//then this column does not meet the needs
					if	(	(!fNotCondition && !rCol.GetUpdateable())	||
							(fNotCondition && rCol.GetUpdateable())
						)
					{
						fFound = FALSE;
					}
					break;
				case COL_COND_AUTOINC:
					//check if the Condition was asked to be allowed and the Condition was not available on the column
					//		if the Condition was asked not to be allowed and the Condition was available on the column or
					//then this column does not meet the needs
					if	(	(!fNotCondition && !rCol.GetAutoInc())	||
							(fNotCondition && rCol.GetAutoInc())
						)
					{
						fFound = FALSE;
					}
					break;
			}
		}
		if (fFound)
		{
			*pcSelectedColumn = rCol.GetColNum();
			return TRUE;
		}
	}

	return FALSE;
} //CTable::GetColWithAttr

//-----------------------------------------------------------------------------
// @cmember Creates DBPARAMBINDINFO and rgParamOrdinal array given desired 
// bindings, column ordinals.  User must free pParamOrdinals, pParamBindInfo.
//
// Note: pParamOrdinals is optional as the user may have this from GetAccessorAndBindings.
//-----------------------------------------------------------------------------
HRESULT CTable::GetDBPARAMBINDINFO(
	DBCOUNTITEM		cBindings,
	DBBINDING *		rgBindings,
	DB_LORDINAL	*	rgColOrds,
	DB_UPARAMS **	ppParamOrdinals,
	DBPARAMBINDINFO ** ppParamBindInfo
)
{
	HRESULT hr = E_OUTOFMEMORY;
	CCol TempCol;
	DB_UPARAMS * pParamOrdinals = NULL;
	DBPARAMBINDINFO * pParamBindInfo = NULL;
	DB_UPARAMS iBind;
	DBTYPE wType;
	BYTE bPrecision;
	BYTE bScale;
	DBPARAMIO eParamIO;
	WCHAR * pwszStrip = NULL;

	// Check args
	// We allow user to not specify ppParamOrdinals
	if (cBindings == 0 || !rgBindings || !rgColOrds || !ppParamBindInfo)
		return E_INVALIDARG;

	// Initialize out params
	if (ppParamOrdinals)
		*ppParamOrdinals = NULL;
	*ppParamBindInfo = NULL;

	// Allocate space for param ordinal array
	SAFE_ALLOC(pParamOrdinals, DB_UPARAMS, cBindings * sizeof(DB_UPARAMS));
	memset(pParamOrdinals, 0, (size_t)(cBindings * sizeof(DB_UPARAMS)));

	// Allocate space for DBPARAMBINDINFO array
	SAFE_ALLOC(pParamBindInfo, DBPARAMBINDINFO, cBindings * sizeof(DBPARAMBINDINFO));
	memset(pParamBindInfo, 0, (size_t)(cBindings * sizeof(DBPARAMBINDINFO)));

	// For each binding passed in
	for (iBind = 0; iBind < cBindings; iBind++)
	{
		// Get the associated column information
		TESTC_(hr = GetColInfo(rgColOrds[iBind], TempCol), S_OK);

		pParamOrdinals[iBind] = iBind+1;

		wType = TempCol.GetProviderType();
		bPrecision = (BYTE) TempCol.GetPrecision();
		bScale = (BYTE)TempCol.GetScale();
		eParamIO = rgBindings[iBind].eParamIO;

		// The CCol information matches GetColumnInfo, which is different for bScale than the spec
		// for bScale for GetParameterInfo.  Fix it up here.
		if (IsNumericType(wType))
		{
			switch(wType)
			{
				case DBTYPE_DECIMAL:
				case DBTYPE_NUMERIC:
				case DBTYPE_VARNUMERIC:
				case DBTYPE_R4:
				case DBTYPE_R8:
					// CCol value is correct
					break;
				case DBTYPE_CY:
					// Scale for money is always 4 per spec
					bScale = 4;
					break;
				default:
					// All other numeric types are integers with scale 0.  CCol matches
					// GetColInfo, which doesn't match spec for GetParameterInfo.
					bScale = 0;
					break;
			}
		}

		pParamBindInfo[iBind].pwszDataSourceType = wcsDuplicate(TempCol.GetProviderTypeName());

		// Sql Server specific: If the type name contains a left paren, then strip it off
		if (pwszStrip = wcsstr(pParamBindInfo[iBind].pwszDataSourceType, L"("))
			*pwszStrip = L'\0';

		// Sql Server specific: If the type name contains "identity, then strip it off
		if (pwszStrip = wcsstr(pParamBindInfo[iBind].pwszDataSourceType, L"identity"))
			*pwszStrip = L'\0';

		pParamBindInfo[iBind].pwszName = NULL;
		pParamBindInfo[iBind].ulParamSize = TempCol.GetColumnSize();

		// Set the appropriate flags
		pParamBindInfo[iBind].dwFlags=0;
		if (eParamIO & DBPARAMIO_OUTPUT)
			pParamBindInfo[iBind].dwFlags |= DBPARAMFLAGS_ISOUTPUT;
		if (eParamIO & DBPARAMIO_INPUT)
			pParamBindInfo[iBind].dwFlags |= DBPARAMFLAGS_ISINPUT;
		if (TempCol.GetIsLong())
			pParamBindInfo[iBind].dwFlags |= DBPARAMFLAGS_ISLONG;
		if (TempCol.GetNullable())
			pParamBindInfo[iBind].dwFlags |= DBPARAMFLAGS_ISNULLABLE;
		if (!TempCol.GetUnsigned())
			pParamBindInfo[iBind].dwFlags |= DBPARAMFLAGS_ISSIGNED;

		pParamBindInfo[iBind].bPrecision = bPrecision;
		pParamBindInfo[iBind].bScale = bScale;
	}

	// Return out params
	if (ppParamOrdinals)
		*ppParamOrdinals = pParamOrdinals;
	*ppParamBindInfo = pParamBindInfo;

	hr = S_OK;

CLEANUP:

	if (!ppParamOrdinals)
		SAFE_FREE(pParamOrdinals);

	return hr;
}



//---------------------------------------------------------------------------
//		
//---------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////
// CLocaleInfo::CLocaleInfo
//
// Constructor, initializes member variables. 
// Takes a LOCALE ID as its only argument and initializes member variables
// accordingly.
// 		
//	@mfunc CLocaleInfo
////////////////////////////////////////////////////////////////////////
CLocaleInfo::CLocaleInfo(LCID lcid)	
{
	static unsigned char szUSAnsiChars[] = {'A', 'B', 'C'};
	static WCHAR szUSUnicodeChars[]		 = {L'A', L'B', L'C'};

	static unsigned char szGermanAnsiChars[] = 
	{	0xFC,
		'b',
		0xF6,
		'u',
		'a',
		0xE4,
		'Z',
		0xDF,
		'A',
		'B',
		'C',
		0xC4,
		'b',
		'o',
		0xDC,
		'O',
		'U',
		0xD6,
		'v',
		'r',
		'h',
		0xC3,
		0xE3,
		'\0'
	};

	static WCHAR szGermanUnicodeChars[] = 
	{	0x00FC,
		L'b',
		0x00F6,
		L'u',
		L'a',
		0x00E4,
		L'Z',
		0x00DF,
		L'A',
		L'B',
		L'C',
		0x00C4,
		L'b',
		L'o',
		0x00DC,
		L'O',
		L'U',
		0x00D6,
		L'v',
		L'r',
		L'h',
		0x00C3,
		0x00E3,
		L'\0',
	};
	
	static unsigned char szFrenchAnsiChars[] = 
	{	0xC7,	// capital Cedilla
		0xE7,	// lowercase cedilla
		0xC0,	// A (grave)
		0xC2,	// A ( circumflex)
		0xC4,	// A (diaeresis)
		'A',
		'B',
		'C',
		0xC8,	// E (grave)
		0xC9,	// E (acute)
		0xCA,	// E (circumflex)
		0xCB,	// E (diaersis)
		'a',
		'e',
		'i',
		0xCE,	// I (circumflex)
		0xCF,	// I (diaersis)
		0xD4,	// O (circumflex)
		0xD6,	// O (diaersis)
		'E',
		'I',
		'O',
		'U',
		0xD9,	// U (grave)
		0xDB,	// U (circumflex)
		0xDC,	// U (diaersis)
		'o',
		'u',
		0xE6,	// ae ligature
		0xC6,	// AE ligature
		0x9C,	// oe ligature
		0x8C,	// OE ligaure
		0xE0,	// a (grave)
		0xE2,	// a (circumflex)
		0xE6,	// a (diaersis)
		0xE8,	// e (acute)
		0xE9,	// e (grave)
		0xEA,	// e (circumflex)
		0xEB,	// e (diaersis)'
		0xEE,	// i (circumflex)
		0xEF,	// i (diaersis)
		0xF4,	// o (circumflex)
		0xF6,	// o (diaersis)
		0xF9,	// u (grave)
		0xFB,	// u (circumflex)
		0xFC,	// u (diaersis)
		'\0'
	};

	static WCHAR szFrenchUnicodeChars[] = 
	{	0x00C7,	// capital Cedilla
		0x00E7,	// lowercase cedilla
		0x00C0,	// A (grave)
		0x00C2,	// A ( circumflex)
		0x00C4,	// A (diaeresis)
		L'A',
		L'B',
		L'C',
		0x00C8,	// E (grave)
		0x00C9,	// E (acute)
		0x00CA,	// E (circumflex)
		0x00CB,	// E (diaersis)
		L'a',
		L'e',
		L'i',
		0x00CE,	// I (circumflex)
		0x00CF,	// I (diaersis)
		0x00D4,	// O (circumflex)
		0x00D6,	// O (diaersis)
		L'E',
		L'I',
		L'O',
		L'U',
		0x00D9,	// U (grave)
		0x00DB,	// U (circumflex)
		0x00DC,	// U (diaersis)
		L'o',
		L'u',
		0x00E6,	// ae ligature
		0x00C6,	// AE ligature
		0x0153,	// oe ligature
		0x0152,	// OE ligaure
		0x00E0,	// a (grave)
		0x00E2,	// a (circumflex)
		0x00E6,	// a (diaersis)
		0x00E8,	// e (acute)
		0x00E9,	// e (grave)
		0x00EA,	// e (circumflex)
		0x00EB,	// e (diaersis)'
		0x00EE,	// i (circumflex)
		0x00EF,	// i (diaersis)
		0x00F4,	// o (circumflex)
		0x00F6,	// o (diaersis)
		0x00F9,	// u (grave)
		0x00FB,	// u (circumflex)
		0x00FC,	// u (diaersis)
		L'\0'
	};

	static unsigned char szSpanishAnsiChars[] = 
	{	0xD1,	// N (tilde)
		0xF1,	// n (tilde)
		'A',
		'B',
		'C',
		0xD3,	// O (acute)
		0xF3,	// o (acute)
		'N',
		'U',
		'I',
		'E',
		0xC1,	// A (acute)
		0xE1,	// a (acute)
		0xC9,	// E (acute)
		0xE9,	// e (acute)
		0xCD,	// I (acute)
		0xED,	// i (acute)
		0xDA,	// U (acute)
		0xFA,	// u (acute)
		'v',
		'r',
		'n',
		'a',
		'e',
		'i',
		'o',
		'u',
		'\0'
	};

	static WCHAR szSpanishUnicodeChars[] = 
	{	0x00D1,	// N (tilde)
		0x00F1,	// n (tilde)
		L'A',
		L'B',
		L'C',
		0x00D3,	// O (acute)
		0x00F3,	// o (acute)
		L'N',
		L'U',
		L'I',
		L'E',
		0x00C1,	// A (acute)
		0x00E1,	// a (acute)
		0x00C9,	// E (acute)
		0x00E9,	// e (acute)
		0x00CD,	// I (acute)
		0x00ED,	// i (acute)
		0x00DA,	// U (acute)
		0x00FA,	// u (acute)
		L'v',
		L'r',
		L'n',
		L'a',
		L'e',
		L'i',
		L'o',
		L'u',
		L'\0'
	};
	
	static unsigned char szItalianAnsiChars[] = 
	{	0xC0,	// A (grave)	
		0xC8,	// E (grave)
		0xCC,	// I (grave)
		0xD9,	// U (grave)
		0xE0,	// a (grave)
		0xE9,	// e (grave)
		0xEC,	// i (grave)
		0xF9,	// u (grave)
		'A',
		'B',
		'C',
		'N',
		'U',
		'I',
		'E',
		0xC9,	// E (acute)
		0xE9,	// e (acute)
		'v',
		'r',
		'n',
		'a',
		'e',
		'i',
		'o',
		'u',
		0xD2,	// O (grave)
		0xF2,	// o (grave)
		'\0'
	};

	static WCHAR szItalianUnicodeChars[] = 
	{	0x00C0,	// A (grave)	
		0x00C8,	// E (grave)
		0x00CC,	// I (grave)
		0x00D9,	// U (grave)
		0x00E0,	// a (grave)
		0x00E9,	// e (grave)
		0x00EC,	// i (grave)
		0x00F9,	// u (grave)
		L'A',
		L'B',
		L'C',
		L'N',
		L'U',
		L'I',
		L'E',
		0x00C9,	// E (acute)
		0x00E9,	// e (acute)
		L'v',
		L'r',
		L'n',
		L'a',
		L'e',
		L'i',
		L'o',
		L'u',
		0x00D2,	// O (grave)
		0x00F2,	// o (grave)
		L'\0'
	};

	static unsigned char szDutchAnsiChars[] = 
	{
		0xCA,	// E (circumflex)
		0xEA,	// e (circumflex)
		0xCB,	// E (diaersis)
		0xEB,	// e (diaerisi)
		'U',
		'I',
		'E',
		0xC9,	// E (acute)
		0xE9,	// e (acute)
		'e',
		'i',
		'o',
		'u',
		0xCE,	// I (circumflex)
		0xEE,	// i (circumflex)
		0xCF,	// I (diaersis)
		0xEF,	// i (diaersis)
		0xD6,	// O (diaersis)
		0xF6,	// o (diaersis)
		0xDC,	// U (diaersis)
		0xEC,	// u (diaersis)		
		'\0'
	};

	static WCHAR szDutchUnicodeChars[] = 
	{	
		0x00CA,	// E (circumflex)
		0x00EA,	// e (circumflex)
		0x00CB,	// E (diaersis)
		0x00EB,	// e (diaerisi)
		L'U',
		L'I',
		L'E',
		0x00C9,	// E (acute)
		0x00E9,	// e (acute)
		L'e',
		L'i',
		L'o',
		L'u',
		0x00CE,	// I (circumflex)
		0x00EE,	// i (circumflex)
		0x00CF,	// I (diaersis)
		0x00EF,	// i (diaersis)
		0x00D6,	// O (diaersis)
		0x00F6,	// o (diaersis)
		0x00DC,	// U (diaersis)
		0x00EC,	// u (diaersis)
		L'\0'
	};


	static unsigned char szJPNAnsiChars[] = 
	{	0xA6,			// single byte katakana
		0x83,0x40,
		0x7A,			// US 'z'
		0x97,0x7E,
		0x83,0x4E,
		0x83,0x4F,
		0x83,0x5B,
		0x83,0x5C,
		0x83,0x5D,
		0x83,0x5E,
		0x83,0x5F,
		0x83,0x60,
		0x83,0x6E,
		0x83,0x6F,
		0x83,0x7B,
		0x83,0x7C,
		0x83,0x7D,
		0x83,0x7E,
		0x83,0x5C,
		0xDD,			// single byte katakana, n
		0xFC,0x4B,
		0x82,0x9A,
		0xC8,			// single byte katakana, ne
		0xE1,0x5C,
		0xE0,0xFC,
		0xFA,0x40,
		0xEE,0xEF,
		0xE3,0x7E,
		0x9F,0x65,
		0x99,0xB7,
		0x98,0x9F,
		0x97,0x7E,
		0x97,0xEF,
		0x41,			// US 'A'
		0x88,0x9F,
		0x84,0x60,
		0x84,0x76,
		0x8B,0xE3,
		0x84,0x91,
		0x82,0x50,
		0x82,0x9F,
		0x82,0xA0,
		0x82,0xF1,
		0x82,0xF2,
		0x82,0xD7,
		0x82,0xD8,
		0x82,0xD0,
		0x82,0xD1,
		0x82,0xD2,
		0x00,0x00	};

	static WCHAR szJPNUnicodeChars[] = 
	{	0xFF66,	// single byte katakana
		0x30A1,
		0x007A,	// US 'z'
		0x30AF,
		0x30B0,
		0x30BC,
		0x30BD,
		0xFF9D,	// single byte katakana
		0x30BE,
		0x30BF,
		0x30BC,
		0x30C1,
		0x30CF,
		0x30D0,
		0x30DC,
		0x30DD,
		0x30DE,
		0x30DF,
		0x30BD, 
		0x9ED1,
		0xFF5A,	// Double-width 'z'
		0xFF88,	//single byte katakana, ne
		0x755A,
		0x73F1,
		0x7E37,
		0x6B79,
		0x531A,
		0x5F0C,
		0x6B32,
		0x66A6,
		0x0041,	// US 'A'
		0x4E9C,
		0x042F,
		0x0451,
		0x4E5D,
		0x044F,
		0xFF41,
		0x3041,
		0x3042,
		0x3092,
		0x3093,
		0x3079,
		0x307A,
		0x3072,
		0x3073,
		0x3074,
		0x0000	};


	static unsigned char szTCAnsiChars[] = 
	{	0xA3,0x5C,
		0xA4,0x40,
		0xA8,0x7D,
		0xA4,0x7E,
		0xA4,0x5D,
		0xA6,0x72,
		0xC1,0x7B,
		0xF9,0xD5,
		0xB8,0x7D,
		0xA5,0x41,
		0xA4,0xC5,
		0xA4,0xE5,
		0xA4,0xDE,	
		0xA4,0xDF,
		0xA4,0xFD,
		0x00,0x00	};

	static WCHAR szTCUnicodeChars[] = 
	{	0x03B1,
		0x4E00, 
		0x826F, 
		0x624D, 
		0x4E5F, 
		0x5B57,
		0x81E8, 
		0x9F98, 
		0x8173, 
		0x4E15, 
		0x52FF, 
		0x6587,
		0x5F15, 
		0x5FC3, 
		0x738B, 
		0x0000	};

	static unsigned char szArabicAnsiChars[] = 
	{
		0x81,		// Arabic Letter PEH
		0x8D,
		0x8E,
		0x90,
		0xC2,
		0xC3,
		0xC4,
		0xC5,
		0xC6,
		0xC7,
		0xC8,
		0xD1,
		0xD2,
		0xD3,
		0xD4,
		0xD5,
		0xD6,
		0xDA,
		0xE1,
		0xF0,
		0xFA,
		'\0'
	};

	static WCHAR szArabicUnicodeChars[] =
	{
		0x067E,
		0x0686,
		0x0698,
		0x06AF,
		0x0622,
		0x0623,
		0x0624,
		0x0625,		
		0x0626,		
		0x0627,		
		0x0628,		
		0x0631,
		0x0632,
		0x0633,
		0x0634,
		0x0635,
		0x0636,
		0x0639,
		0x0644,
		0x064B,
		0x0652,
		L'\0'
	};

	static WCHAR wszSurrogateChars[] =
	{
		0xD840, 0xDD7A,
		0xD841, 0xDEFF,
		0xD850, 0xDF08,
		0xD840, 0xDD52,
		0xD845, 0xDEAB,
		0xD864, 0xDCFC,
		0xD861, 0xDF33,
		0x0000, 0x0000
	};

	LANGID lang_id = LANGIDFROMLCID(lcid);
	BYTE primary_lang = PRIMARYLANGID( lang_id );
	BYTE sublang = SUBLANGID( lang_id );
	
	m_wszSurrogateChars = wszSurrogateChars;

	switch ( primary_lang ) 
	{
		// US
		case LANG_ENGLISH:
			ASSERT ( GetACP() == 1252 );
			m_szAnsiChars = szUSAnsiChars;
			// set to Japanese Unicode characters.
			// Even on an English client, this is okay, but the target DBMS must support Unicode.
			m_wszUnicodeChars = szJPNUnicodeChars; 
			m_nCharMaxWidth = 1;
			break;

		//JPN
		case LANG_JAPANESE:
			ASSERT ( GetACP() == 932 );
			m_szAnsiChars = szJPNAnsiChars;
			m_wszUnicodeChars = szJPNUnicodeChars;
			m_nCharMaxWidth = 2;
			break;
		
		//Traditional Chinese
		case LANG_CHINESE:
			if ( sublang == SUBLANG_CHINESE_TRADITIONAL )
			{
				ASSERT( GetACP() == 950 );
				m_szAnsiChars = szTCAnsiChars;
				m_wszUnicodeChars = szTCUnicodeChars;
				m_nCharMaxWidth = 2;
			}
			else
			{
				ASSERT(!"No support for this sublang");
			}
			break;

		// Korean
		case LANG_KOREAN:
			ASSERT( GetACP() == 949 );
			ASSERT(!"No support yet");
			break;

		// German
		case LANG_GERMAN:
			ASSERT( GetACP() == 1252 );
			if ( sublang == SUBLANG_GERMAN )
			{
				m_szAnsiChars = szGermanAnsiChars;
				m_wszUnicodeChars = szGermanUnicodeChars;
			
				m_nCharMaxWidth = 1;
			}
			else
			{
				ASSERT(!"No support for this sublang");
			}
			break;

		// French
		case LANG_FRENCH:
			ASSERT( GetACP() == 1252 );
			if ( sublang == SUBLANG_FRENCH )
			{
				m_szAnsiChars = szFrenchAnsiChars;
				m_wszUnicodeChars = szFrenchUnicodeChars;

				m_nCharMaxWidth = 1;
			}
			else
			{
				ASSERT(!"No support for this sublang");
			}
			break;

		// Spanish (Castillan only)
		case LANG_SPANISH:
			ASSERT( GetACP() == 1252 );
		
			m_szAnsiChars = szSpanishAnsiChars;
			m_wszUnicodeChars = szSpanishUnicodeChars;

			m_nCharMaxWidth = 1;
			
			break;

		case LANG_ITALIAN:
			ASSERT( GetACP() == 1252 );
			if ( sublang == SUBLANG_ITALIAN )
			{
				m_szAnsiChars = szItalianAnsiChars;
				m_wszUnicodeChars = szItalianUnicodeChars;

				m_nCharMaxWidth = 1;
			}
			else
			{
				ASSERT(!"No support for this sublang");
			}
			break;

		case LANG_PORTUGUESE:			
				ASSERT(!"No support for this Portuguese");			
			break;

			
		case LANG_DUTCH:
			ASSERT( GetACP() == 1252 );
			if ( sublang == SUBLANG_DUTCH )
			{
				m_szAnsiChars = szDutchAnsiChars;
				m_wszUnicodeChars = szDutchUnicodeChars;

				m_nCharMaxWidth = 1;
			}
			else
			{
				ASSERT(!"No support for this sublang");
			}
			break;

		case LANG_ARABIC:
			ASSERT( GetACP() == 1256 );
			m_szAnsiChars = szArabicAnsiChars;
			m_wszUnicodeChars = szArabicUnicodeChars;
			m_nCharMaxWidth = 1;
			break;

		default:
			ASSERT(!"No support for this language");
			m_szAnsiChars = NULL;
			m_wszUnicodeChars = NULL;
			break;
	}

	if ( !m_szAnsiChars )
		m_szAnsiChars = szUSAnsiChars;
	if (!m_wszUnicodeChars )
		m_wszUnicodeChars = szUSUnicodeChars;

	int i = 0;
	m_cchUnicode = m_cchAnsi = 0;
	
	while ( m_wszUnicodeChars[i] != '\0' ) 
	{
		i++;
		m_cchUnicode++;
	}

	char *szTmp = (char *)m_szAnsiChars;
	while ( *szTmp ) 
	{
		szTmp = CharNextExA(CP_ACP, szTmp, 0);
		m_cchAnsi++;
	}

	m_lcid = lcid;
	m_seedUnicode = m_seedAnsi =0;
}


////////////////////////////////////////////////////////////////////////
// CLocaleInfo::~CLocaleInfo
//
////////////////////////////////////////////////////////////////////////
CLocaleInfo::~CLocaleInfo()	
{
}


////////////////////////////////////////////////////////////////////////
// CLocaleInfo::MakeUnicodeIntlString
//
// 2nd argument indicates number of characters to use.
////////////////////////////////////////////////////////////////////////
BOOL CLocaleInfo::MakeUnicodeIntlString(WCHAR *wsz, int len)
{
	ASSERT( len > 0 );

	for(LONG i=0; i<len; i++) 
	{
		wsz[i] = m_wszUnicodeChars[m_seedUnicode];
		m_seedUnicode = ++m_seedUnicode % (m_cchUnicode);
	}

	wsz[len-1] = L'\0';
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// CLocaleInfo::MakeUnicodeIntlData
// Differs from MakeUnicodeIntlString in that MakeUnicodeIntlString
// must return characters that are valid identifiers.
//
// 2nd argument indicates number of characters to use.
////////////////////////////////////////////////////////////////////////
BOOL CLocaleInfo::MakeUnicodeIntlData(WCHAR *wsz, int len)
{
	return MakeUnicodeIntlString(wsz, len);

#if 0

	ASSERT( len > 0 );

	int i = 0;
	WCHAR * pwszSurrogatePosition = (WCHAR *)m_wszSurrogateChars;

	while ( i < (len-1) ) 
	{
		if ( i == len-2 )
		{
			wsz[i] = m_wszUnicodeChars[m_seedUnicode];
			m_seedUnicode = ++m_seedUnicode % (m_cchUnicode);
			break;
		}
	
		// Add the surrogate characters
		ASSERT((*pwszSurrogatePosition & 0xD800) == 0xD800);
		wsz[i++] = *pwszSurrogatePosition++;

		ASSERT((*pwszSurrogatePosition & 0xDC00) == 0xDC00);
		wsz[i++] = *pwszSurrogatePosition++;

		if ( *pwszSurrogatePosition == L'\0' )
			pwszSurrogatePosition = (WCHAR *)m_wszSurrogateChars;
	}

	wsz[len-1] = '\0';
	return TRUE;
#endif
}


////////////////////////////////////////////////////////////////////////
// CLocaleInfo::MakeAnsiIntlString
//
// 2nd argument indicates number of bytes to use
////////////////////////////////////////////////////////////////////////
BOOL CLocaleInfo::MakeAnsiIntlString(char *sz, int len)
{
	int i = 0;
	char *szPosition = (char *)m_szAnsiChars;

	ASSERT( len > 0 );

	for ( int iPos = 1; iPos <= m_seedAnsi; iPos++)
		szPosition = CharNextExA(CP_ACP, szPosition, 0);

	while ( i < (len-1) ) 
	{
		BOOL fIsDBCSChar = IsDBCSLeadByte(*szPosition);

		if ( i == len-2 && fIsDBCSChar)
		{
			sz[i] = 'A';
			break;
		}
	
		sz[i] = *szPosition;
		i++;

		if ( fIsDBCSChar )
		{
			sz[i] = *(szPosition+1);
			i++;
		}
			
		szPosition = CharNextExA(CP_ACP, szPosition, 0);
		if ( *szPosition == '\0' )
			szPosition = (char *)m_szAnsiChars;

		m_seedAnsi++;
		m_seedAnsi = m_seedAnsi % (m_cchAnsi);
	}

	sz[len-1] = '\0';
	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// CLocaleInfo::MakeUnicodeChar
//
////////////////////////////////////////////////////////////////////////
WCHAR CLocaleInfo::MakeUnicodeChar()
{
	int prev_seed;

	prev_seed = m_seedUnicode;
	m_seedUnicode++;
	m_seedUnicode = m_seedUnicode % (m_cchUnicode);
	return m_wszUnicodeChars[prev_seed];
}


////////////////////////////////////////////////////////////////////////
// CLocaleInfo::MakeAnsiChar
//
////////////////////////////////////////////////////////////////////////
void CLocaleInfo::MakeAnsiChar(char *cLead, char *cTrail)
{
	int prev_seed = m_seedAnsi;
	char *szPosition = (char *)m_szAnsiChars;
	
	for ( int iPos = 1; iPos <= m_seedAnsi; iPos++)
		szPosition = CharNextExA(CP_ACP, szPosition, 0);
	
	*cLead = *szPosition;
	if ( IsDBCSLeadByte(*szPosition) )
		*cTrail = *(szPosition+1);
	else
		*cTrail = '\0';
		
	szPosition = CharNextExA(CP_ACP, szPosition, 0);
	
	if ( *szPosition == '\0' )
		szPosition = (char *)m_szAnsiChars;

	m_seedAnsi++;
	m_seedAnsi = m_seedAnsi % m_cchAnsi;

	return;
}

////////////////////////////////////////////////////////////////////////
// CLocaleInfo::MakeUnicodeInt
//
////////////////////////////////////////////////////////////////////////
WCHAR* CLocaleInfo::MakeUnicodeInt(int val)
{
	WCHAR* pwszIntBuf = (WCHAR *)PROVIDER_ALLOC(MAXBUFLEN*sizeof(WCHAR));

	if (pwszIntBuf)
	{
		_itow(val, pwszIntBuf, 10);

		// For Chinese, Japanese, or Korean,
		// Map the integer to its Fullwidth equivalent
		if (m_lcid == 0x0404 || m_lcid == 0x0411 || m_lcid == 0x0412)
			W95LCMapString(pwszIntBuf, LCMAP_FULLWIDTH);
	}

	return pwszIntBuf;
}

////////////////////////////////////////////////////////////////////////
// CLocaleInfo::SetAnsiSeed
//
////////////////////////////////////////////////////////////////////////
BOOL CLocaleInfo::SetAnsiSeed(int val)
{
	m_seedAnsi = val % (m_cchAnsi);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CLocaleInfo::SetUnicodeSeed
//
////////////////////////////////////////////////////////////////////////
BOOL CLocaleInfo::SetUnicodeSeed(int val)
{
	m_seedUnicode = val % (m_cchUnicode);
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// CLocaleInfo::LocalizeString
//
////////////////////////////////////////////////////////////////////////
HRESULT	CLocaleInfo::LocalizeString(WCHAR* pwszString, BSTR* pbstrLocalizedString)
{
	//invalid arguments
	ASSERT(pbstrLocalizedString);
	*pbstrLocalizedString = NULL;

	//no-op
	if(pwszString == NULL)
		return S_OK;

	//Initlize Temp Variant
	VARIANT vVariant;
	VariantInit(&vVariant);
	
	//Setup Passed in string into the Variant
	V_VT(&vVariant) = VT_BSTR;
	V_BSTR(&vVariant) = SysAllocString(pwszString);

	//Convert to Localized Version (in place conversion)
	HRESULT hr = VariantChangeTypeEx(&vVariant, &vVariant, m_lcid, 0, VT_BSTR);

	//Return output
	*pbstrLocalizedString = NULL;
	if(SUCCEEDED(hr))
		*pbstrLocalizedString = V_BSTR(&vVariant);
	return hr;
}

////////////////////////////////////////////////////////////////////////
// CLocaleInfo::LocalizeVariant
//
////////////////////////////////////////////////////////////////////////
HRESULT	CLocaleInfo::LocalizeVariant(VARIANT* pVariant, VARIANT* pVarLocalized)
{
	//invalid arguments
	if(pVariant == NULL || pVarLocalized == NULL)
		return E_FAIL;

	//Initlized Target Variant
	VariantInit(pVarLocalized);

	//Use the LCID specified for this class
	return VariantChangeTypeEx(pVarLocalized, pVariant, m_lcid, 0, V_VT(pVariant));
}


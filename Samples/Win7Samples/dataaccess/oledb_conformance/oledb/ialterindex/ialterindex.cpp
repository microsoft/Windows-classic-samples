//*-----------------------------------------------------------------------
//
//   This is the Test Module for IAlterIndex interface, which is an 
//	 optional interface on Session objects.
//
//   WARNING:
//          PLEASE USE THE TEST CASE WIZARD TO ADD/DELETE TESTS AND VARIATIONS!
//
//
//   Copyright (C) 1994-1997 Microsoft Corporation
//*-----------------------------------------------------------------------

#include "MODStandard.hpp"
#include "IAlterIndex.h"
#include "ExtraLib.h"


//*-----------------------------------------------------------------------
// Global vars
//*-----------------------------------------------------------------------
BOOL g_fBadInitialSize = FALSE;	// Used to suppress duplicate errors

//*-----------------------------------------------------------------------
// Module Values
//*-----------------------------------------------------------------------
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x6522f900, 0x46d5, 0x11d3, { 0x89, 0x3d, 0x00, 0x60, 0x08, 0x9f, 0xc4, 0x66} };
DECLARE_MODULE_NAME("IAlterIndex");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Testsuite for IAlterIndex interface.");
DECLARE_MODULE_VERSION(1);
// TCW_WizardVersion(2)
// TCW_Automation(FALSE)
// }} TCW_MODULE_GLOBALS_END


//*-----------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	TBEGIN
	IAlterIndex*	pIAlterIndex = NULL;

	g_fBadInitialSize = FALSE;

	//Create the session
	TESTC(ModuleCreateDBSession(pThisTestModule))
	
	TESTC_PROVIDER(VerifyInterface(pThisTestModule->m_pIUnknown2, 
		IID_IAlterIndex, SESSION_INTERFACE, (IUnknown**)&pIAlterIndex))

	//g_pIDBInitialize needed for some Extralib functions.
	TESTC(VerifyInterface(pThisTestModule->m_pIUnknown, 
		IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)
		&g_pIDBInitialize))

CLEANUP:
	SAFE_RELEASE(pIAlterIndex);
	TRETURN
}

//*-----------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	SAFE_RELEASE(g_pIDBInitialize);
	return ModuleReleaseDBSession(pThisTestModule);
}

class CVARIANT : public VARIANT
{

public:

	CVARIANT(VARTYPE vt, VARIANT_BOOL bValue)
	{
		VariantInit(this);
		this->vt = vt;
		this->boolVal = bValue;
	}
};


class CDBID : public DBID
{

public:

	CDBID(DBKIND eKindNew, LPWSTR pwszNameNew)
	{
		uGuid.guid = GUID_NULL;
		uGuid.pguid = NULL;
		eKind = eKindNew;
		uName.pwszName = pwszNameNew;
	}
};

class CIndexInfo
{
private:
	DBID 				m_TableID;
	DBID 				m_IndexID;
	DBID *				m_pTableID;
	DBID *				m_pIndexID;
	LPBYTE				m_pData;
	DBBINDING *			m_pBinding;
	DBLENGTH			m_ulRowSize;
	ULONG				m_cIndexCols;
	IDBSchemaRowset *	m_pIDBSchemaRowset;
	IOpenRowset *		m_pIOpenRowset;

	void ReleaseAll(void);

	HRESULT GetIndexInfo(void);

	HRESULT	RefreshIndexInfo(void);

	DBLENGTH  GetIndexValueLength(ULONG iIndexCol, enum EINDEXSCHEMA iIndexValue);

public:

	CIndexInfo(void);

	~CIndexInfo(void);

	BOOL Init(IUnknown * pSessionUnknown, DBID *pTableID, DBID *pIndexID);

	BOOL IsIndexValueValid(ULONG iIndexCol, enum EINDEXSCHEMA iIndexValue);

	LPBYTE GetIndexValuePtr(ULONG iIndexCol, enum EINDEXSCHEMA iIndexValue);
};

CIndexInfo::CIndexInfo(void)
{
	m_pTableID			= &m_TableID;
	m_pIndexID			= &m_IndexID;
	memset(m_pTableID, 0, sizeof(DBID));
	memset(m_pIndexID, 0, sizeof(DBID));
	m_pData				= NULL;
	m_pBinding			= NULL;
	m_ulRowSize			= 0;
	m_cIndexCols		= 0;
	m_pIDBSchemaRowset	= NULL;
	m_pIOpenRowset		= NULL;
}

CIndexInfo::~CIndexInfo(void)
{
	// Release/free member vars
	ReleaseAll();
}

void CIndexInfo::ReleaseAll(void)
{
	if (m_pTableID)
		ReleaseDBID(m_pTableID, FALSE);
	if (m_pIndexID)
		ReleaseDBID(m_pIndexID, FALSE);
	SAFE_FREE(m_pData);
	SAFE_FREE(m_pBinding);
	SAFE_RELEASE(m_pIDBSchemaRowset);
	SAFE_RELEASE(m_pIOpenRowset);
}

BOOL CIndexInfo::Init(IUnknown * pSessionUnknown, DBID *pTableID, DBID *pIndexID)
{
	// Validate args
	if (!pSessionUnknown || !pTableID)
		return FALSE;

	// Release/free member vars
	ReleaseAll();

	// Save table and index ID
	TESTC_(DuplicateDBID(*pTableID, m_pTableID), S_OK);

	if (pIndexID)
	{
		TESTC_(DuplicateDBID(*pIndexID, m_pIndexID), S_OK);
	}
	else
		m_pIndexID = NULL;
	
	// Get IDBSchemaRowset interface.  We assume all providers will support this as
	// otherwise there's no way to know what indexes have been created.
	if (!VerifyInterface(pSessionUnknown, 
		IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&m_pIDBSchemaRowset))
		return FALSE;

	if (!VerifyInterface(pSessionUnknown, 
		IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&m_pIOpenRowset))
		return FALSE;

	return (S_OK == GetIndexInfo());

CLEANUP:

	return FALSE;
}

HRESULT	CIndexInfo::RefreshIndexInfo(void)
{
	SAFE_FREE(m_pData);
	SAFE_FREE(m_pBinding);

	return GetIndexInfo();
}

DBLENGTH  CIndexInfo::GetIndexValueLength(ULONG iIndexCol, enum EINDEXSCHEMA iIndexValue)
{
	if (!IsIndexValueValid(iIndexCol, iIndexValue))
		return 0;

	if (!LENGTH_IS_BOUND(m_pBinding[iIndexValue]))
		return 0;

	return LENGTH_BINDING(m_pBinding[iIndexValue], m_pData+iIndexCol*m_ulRowSize);
}

BOOL CIndexInfo::IsIndexValueValid(ULONG iIndexCol, enum EINDEXSCHEMA iIndexValue)
{
	if (!m_pData || !m_pBinding)
		return FALSE;

	if (iIndexCol >= m_cIndexCols)
		return FALSE;

	if (iIndexCol > 0 && m_ulRowSize == 0)
		return 0;

	if (!STATUS_IS_BOUND(m_pBinding[iIndexValue]))
		return 0;

	if (!VALUE_IS_BOUND(m_pBinding[iIndexValue]))
		return 0;

	return (DBSTATUS_S_OK == STATUS_BINDING(m_pBinding[iIndexValue], m_pData+iIndexCol*m_ulRowSize));
}

LPBYTE CIndexInfo::GetIndexValuePtr(ULONG iIndexCol, enum EINDEXSCHEMA iIndexValue)
{
	if (!IsIndexValueValid(iIndexCol, iIndexValue))
		return NULL;

	return (LPBYTE)&VALUE_BINDING(m_pBinding[iIndexValue], m_pData+iIndexCol*m_ulRowSize);
}

HRESULT CIndexInfo::GetIndexInfo(void)
{
	HRESULT					hr						= E_FAIL;
	IRowsetIndex *			pIRowsetIndex			= NULL;
	DBLENGTH				ulBufferSize			= 0;
	ULONG					cRows					= 0;
	LPBYTE					pRow					= NULL;
	DBORDINAL				cKeyColumns				= 0;
	DBINDEXCOLUMNDESC *		rgIndexColumnDesc		= NULL;
	ULONG					cIndexPropSets			= 0;
	DBPROPSET *				rgIndexPropSets			= NULL;
	

	BOOL				fReturn			= FALSE;
	BOOL				fProps			= TRUE;
	BOOL				bIsSchemaSupported;
	ULONG 				ulRowSize		= 0;		// size of row
	DBCOUNTITEM			cDBBINDING		= 0;		// count of bindings
	ULONG 				iRow			= 0;		// count of rows
	DBCOUNTITEM			cRowsObtained	= 0;		// number of rows returned, should be 1
	ULONG				cSchema			= 0;		// number of supported Schemas
	ULONG				*prgRestrictions= 0;		// restrictions for each Schema
	GUID				*prgSchemas		= NULL;		// array of GUIDs
	HROW				hRow;						// handler of rows
	HROW				*phRow = &hRow;				// pointer to handler
	IRowset				*pIndexRowset	= NULL;		// returned rowset
	DBBINDING			*rgDBBINDING	= NULL;		// array of bindings
	HACCESSOR 			hAccessor		= NULL;		// accessor
	BOOL				*rgColPresent = NULL;
	const int			cRest = 5;
	VARIANT				rgRestrictIndexes[cRest];
	ULONG				index, i;

	// column entries in index schema rowset and their presence flags
	LPWSTR				pwszIndexName		= NULL;
	BOOL				fIndexName			= FALSE;
	LPWSTR				pwszTableName		= NULL;
	BOOL				fTableName			= FALSE;

	// We need IDBSchemaRowset for verification.
	if (!m_pIDBSchemaRowset)
		return E_FAIL;

	if (m_pIndexID)
	{
		// Try to get IRowsetIndex from integrated index
		TEST3C_(hr = m_pIOpenRowset->OpenRowset(
			NULL,
			m_pTableID,
			m_pIndexID, 
			IID_IRowsetIndex,
			0,
			NULL,
			(IUnknown**)&pIRowsetIndex
		), S_OK, E_NOINTERFACE, DB_E_NOINDEX);

		if (FAILED(hr))
		{
			// Try to get IRowsetIndex from separate index
			TEST3C_(hr = m_pIOpenRowset->OpenRowset(
				NULL,
				NULL,
				m_pIndexID, 
				IID_IRowsetIndex,
				0,
				NULL,
				(IUnknown**)&pIRowsetIndex
			), S_OK, E_NOINTERFACE, DB_E_NOINDEX);

			if (SUCCEEDED(hr))
				TESTC(pIRowsetIndex != NULL);
		}
	}

	if (pIRowsetIndex)
	{
		TESTC_(pIRowsetIndex->GetIndexInfo(
				&cKeyColumns,
				&rgIndexColumnDesc,
				&cIndexPropSets,
				&rgIndexPropSets), S_OK);
	}

	// Initialize restrictions
	for(index=0;index<cRest;index++)
		VariantInit(&rgRestrictIndexes[index]);

	// Check to see if the schema is supported
	TESTC_(hr = m_pIDBSchemaRowset->GetSchemas(&cSchema, &prgSchemas, &prgRestrictions),S_OK);

	if (m_pIndexID)
	{
		if (DBKIND_NAME == m_pIndexID->eKind)
		{
			pwszIndexName = wcsDuplicate(m_pIndexID->uName.pwszName);
			fIndexName = TRUE;
		}
	}

	if (DBKIND_NAME == m_pTableID->eKind)
	{
		pwszTableName = wcsDuplicate(m_pTableID->uName.pwszName);
		fTableName = TRUE;
	}

	// Check to see if DBSCHEMA_INDEXES is supported
	for(i=0, bIsSchemaSupported=FALSE; i<cSchema && !bIsSchemaSupported;)
	{
		if(prgSchemas[i] == DBSCHEMA_INDEXES)
			bIsSchemaSupported = TRUE;
		else 
			i++;
	}

	if(!bIsSchemaSupported || !(prgRestrictions[i] & 0x4) || !(prgRestrictions[i] & 0x10))
	{
		odtLog << "Index Schema Rowset or the required constraints are not supported\n";
		goto CLEANUP;
	}

	if (m_pIndexID)
	{
		rgRestrictIndexes[2].vt 	 = VT_BSTR;
		rgRestrictIndexes[2].bstrVal = SysAllocString(m_pIndexID->uName.pwszName);
	}
	rgRestrictIndexes[4].vt 	 = VT_BSTR;
	rgRestrictIndexes[4].bstrVal = SysAllocString(m_pTableID->uName.pwszName);

	TESTC_(hr = m_pIDBSchemaRowset->GetRowset(	
			NULL, 								// aggregation
			DBSCHEMA_INDEXES,					// REFGUID
			cRest,	 							// count of restrictions (1:types)
			rgRestrictIndexes,					// list of restrictions
			IID_IRowset,						// REFFID
			0,									// count of properties
			NULL,								// range of properties
			(IUnknown**)&pIndexRowset			// returned result set
		),S_OK);

	TESTC_(hr = GetAccessorAndBindings(
						pIndexRowset, DBACCESSOR_ROWDATA, &hAccessor,	&m_pBinding,		
						&cDBBINDING, &m_ulRowSize, DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
						ALL_COLS_EXCEPTBOOKMARK, FORWARD, NO_COLS_BY_REF,
						NULL,					// OUT: Array of DBCOLUMNINFOs
						0,						// OUT: Count of DBCOULMNINFOs
						NULL,					//&pStringsBuffer, 
						DBTYPE_EMPTY, 
						0, NULL),S_OK);


	// read the first row, check local data and initialize variables
	// Read all the rows and verify the columns that were indexed and
	// their collation, ordinal position, etc.
	// Also verify integrated based on how openrowset performs.

	hr = S_OK;

	ASSERT(m_pData == NULL);

	while (hr == S_OK)
	{
		TEST2C_(hr=pIndexRowset->GetNextRows(0, 0, 1, &cRowsObtained, &phRow), S_OK, DB_S_ENDOFROWSET);

		if (hr == DB_S_ENDOFROWSET)
			break;

		TESTC(cRowsObtained == 1);

		// Allocate buffer for row of data
		ulBufferSize = m_ulRowSize*(cRows+1);
		SAFE_REALLOC(m_pData, BYTE, ulBufferSize);

		pRow = m_pData+cRows*m_ulRowSize;

		// Retrieve the row
		TESTC_(hr=pIndexRowset->GetData(hRow, hAccessor, pRow),S_OK);


		TESTC_(pIndexRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);\

		// Now we have one more row in our buffer
		cRows++;

	}

	// Make sure we got end of rowset
	TESTC_(hr, DB_S_ENDOFROWSET);

	// Make sure we got at least one row from the index
	TESTC(cRows > 0);

	m_cIndexCols = cRows;

	// If we have both types of information then compare them
	if (pIRowsetIndex && m_pIDBSchemaRowset)
	{
		// Compare schema information from IRowsetIndex and DBSCHEMA_INDEXES

		// Count of rows from schema rowset should match number of key colummns
		if (COMPARE(cKeyColumns, cRows))
		{
			// Each row should match appropriate value from IRowset::GetIndexInfo
			for (ULONG iRow=0; iRow < cRows; iRow++)
			{
				DBID * pColumnID = rgIndexColumnDesc[iRow].pColumnID;
				DBINDEX_COL_ORDER eIndexColOrder = rgIndexColumnDesc[iRow].eIndexColOrder;
				BOOL fIndexPropSet = FALSE;

				// IRowsetIndex::GetIndexInfo doesn't return information about catalog, schema, and
				// table, but those must have matched or we won't find the index.  We could match
				// if index id is fully qualified, or else if we use current catalog.

				// If DBID has name, we can compare with name in schema rowset
				if (pColumnID && DBIDHasName(*pColumnID) && IsIndexValueValid(iRow, IS_COLUMN_NAME))
					COMPARE(wcscmp(pColumnID->uName.pwszName, (LPWSTR)GetIndexValuePtr(iRow, IS_COLUMN_NAME)), 0);

				// TODO: Compare propid, guid, etc.  But at this time no providers support those.

				// The index order should match.  Note values differ, schema rowset is 1 based
				// while DBINDEX_COL_ORDER is 0 based.  Also, DBINDEX_COL_ORDER is ULONG, while
				// COLLATION is SHORT.
				if (IsIndexValueValid(iRow, IS_COLLATION))
					COMPARE(eIndexColOrder, (ULONG)(*(SHORT *)GetIndexValuePtr(iRow, IS_COLLATION))-1);

				// The ordinal should match
				if (IsIndexValueValid(iRow, IS_ORDINAL_POSITION))
					COMPARE(iRow+1, (*(ULONG *)GetIndexValuePtr(iRow, IS_ORDINAL_POSITION)));

				// Compare all the properties
				for (ULONG iPropSet = 0; iPropSet < cIndexPropSets; iPropSet++)
				{
					// We can't compare any provider specific index prop sets
					if (rgIndexPropSets[iPropSet].guidPropertySet != DBPROPSET_INDEX)
						continue;

					fIndexPropSet = TRUE;

					for (ULONG iProp = 0; iProp < rgIndexPropSets[iPropSet].cProperties; iProp++)
					{
						ULONG iMap;
						// Find this prop in the mapping
						for (iMap = 0; iMap < NUMELEM(IndexProperties); iMap++)
							if (rgIndexPropSets[iPropSet].rgProperties[iProp].dwPropertyID ==
								IndexProperties[iMap].dwProp)
								break;

						// We use the enum value of CINDEXFIELDS to indicate no corresponding field in INDEXES rowset
						// We can't compare values with no corresponding INDEXES field.
						if (IndexProperties[iMap].eSchemaField == CINDEXFIELDS)
							break;

						// We must find the prop
						if (COMPARE(iMap < NUMELEM(IndexProperties), TRUE))
						{
							// Extract the values
							LPBYTE pPropVal = &rgIndexPropSets[iPropSet].rgProperties[iProp].vValue.bVal;
							LPBYTE pSchemaVal = GetIndexValuePtr(iRow, IndexProperties[iMap].eSchemaField);
							DBLENGTH ulLength = GetIndexValueLength(iRow, IndexProperties[iMap].eSchemaField);

							// If we got valid data for each value we can compare them
							if (COMPARE(pPropVal != NULL, TRUE) &&
								COMPARE(pSchemaVal != NULL, TRUE) &&
								IsIndexValueValid(iRow, IndexProperties[iMap].eSchemaField))
							{
								// Perform compare, suppress duplicate failures for INITIALSIZE
								if (rgIndexPropSets[iPropSet].rgProperties[iProp].dwPropertyID == 
									DBPROP_INDEX_INITIALSIZE)
								{
									if (!g_fBadInitialSize && !COMPARE(memcmp(pPropVal, pSchemaVal, (size_t)ulLength) == 0, TRUE))
									{
										odtLog << L"Property " << IndexProperties[iMap].pwszPropVal << L" had an invalid value.\n\n";
										g_fBadInitialSize = TRUE;
									}
								}
								else
								{
									// Perform compare
									if (!COMPARE(memcmp(pPropVal, pSchemaVal, (size_t)ulLength) == 0, TRUE))
										odtLog << L"Property " << IndexProperties[iMap].pwszPropVal << L" had an invalid value.\n\n";
								}

							}

						}

					}
				}
				
				// We have to find the index propset in the property sets
				COMPARE(fIndexPropSet, TRUE);
			}
		}
	}
	
	// Reset to S_OK
	hr = S_OK;

CLEANUP:

	// Free the memory
	SAFE_FREE(pwszIndexName);
	SAFE_FREE(pwszTableName);
	
	SAFE_FREE(rgColPresent);

	SAFE_FREE(prgRestrictions);
	SAFE_FREE(prgSchemas);

	SAFE_RELEASE(pIndexRowset);

	SAFE_RELEASE(pIRowsetIndex);

	for(index=0;index<cRest;index++)
		GCHECK(VariantClear(&(rgRestrictIndexes[index])),S_OK);
	
	return hr;
}



////////////////////////////////////////////////////////////////////////
// TCBase  -  Class for reusing Test Cases.
// This is one of the base classes from which all the Test Case
// classes will inherit. It is used to duplicate test cases, yet
// maintain some sort of distinct identity for each.
//
////////////////////////////////////////////////////////////////////////
class TCBase
{
public:
	//constructor
	TCBase() { SetTestCaseParam(TC_SingleColNoProps); }

	virtual void SetTestCaseParam(ETESTCASE eTestCase)
	{
		m_eTestCase = eTestCase;

		switch(eTestCase)
		{
		case TC_SingleColNoProps:
			break;

		case TC_SingleColProps:
			break;

		case TC_MultipleColsNoProps:
			break;

		case TC_MultipleColsProps:
			break;
		
		case TC_PropSingCol:
			break;

		case TC_PropMultCol:
			break;

		default:
			ASSERT(!L"Unhandled Type...");
			break;
		};
	}

	//data
	ETESTCASE	m_eTestCase;
};


////////////////////////////////////////////////////////////////////////
//  CAlterIndex  - Class for IAlterIndex Test Cases.
//
////////////////////////////////////////////////////////////////////////
class CAlterIndex : public CSessionObject, public TCBase
{
public:
	//constructors
	CAlterIndex(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~CAlterIndex();

	//methods
	virtual BOOL		Init();
	virtual BOOL		Terminate();

protected:

//VARIABLES...

	HRESULT				m_hr;
	ULONG				m_cMaxTableName, m_cMaxIndexName, m_cMaxColumnName;
	LPOLESTR			m_pwszInvalidTableChars;
	LPOLESTR			m_pwszInvalidTableStartingChars;
	LPOLESTR			m_pwszInvalidIndexChars;
	LPOLESTR			m_pwszInvalidIndexStartingChars;
	LPOLESTR			m_pwszInvalidColumnChars;
	ULONG				m_cPropSets;
	DBPROPSET*			m_rgPropSets;

	BOOL				m_ulIRowsetIndex;
	BOOL				m_ulOpenIndexSupport;
	DBCOLUMNDESC*		m_rgColumnDesc;
	DBORDINAL			m_cColumnDesc;
	DBID*				m_pIndexID;
	DBINDEXCOLUMNDESC*	m_pIndexColumnDesc;
	DBORDINAL			m_cIndexColumnDesc;
	BOOL				m_fSetOnlyDefault;
	CTable	*			m_pNoNullTable;

//INTERFACES...

	IAlterIndex*		m_pIAlterIndex;
	IIndexDefinition*	m_pIIndexDef;
	IDBSchemaRowset*	m_pIDBSchemaRowset;

//METHODS...

	//Wrapper for corresponding method of IAlterIndex. The table
	//ID used here is m_pTableID. If you want to pass in another
	//table ID (like in boundary cases), call method directly.
	HRESULT	AlterIndex(		
		   DBID *	pIndexID,
		   DBID *	pNewIndexID,
		   BOOL		fValidIndex = TRUE
			);

	HRESULT	AlterIndex(	
			DBID *      pTableID,
			DBID *      pIndexID,
			DBID *      pNewIndexID,
			BOOL		fValidIndex = TRUE
			);

	HRESULT	GetLiteralInfo();

	void SetIndexCreationProps(ETESTCASE eTestCase);

	WCHAR* BuildValidName(size_t length, WCHAR* pattern);

	WCHAR* BuildInvalidName(size_t length, WCHAR* pattern, WCHAR* invchars);

	BOOL	CheckProperty(const GUID guidPropSet, DBPROP *rgProp, HRESULT hr);

	// @cmember Check whether all the columns specified appear in index
	// if a IRowsetIndex could be open on the index, it goes for CheckIndex2
	// otherwise tries to use IDBSchemaRowset
	BOOL	CheckIndex(
			DBID				*pTableID,				// the index of the table
			DBID				*pIndexDBID,			// the index to be checked
			ULONG				cPropertySets = 0,		// number of property sets
			DBPROPSET			*rgPropertySets = NULL	// the array of property sets
		);

	// @cmember Check whether all the columns specified appear in index
	// opens an IRowsetIndex and checks everything
	BOOL	CheckIndex2(
			DBID				*pTableID,				// the index of the table
			DBID				*pIndexDBID,			// the index to be checked
			ULONG				cPropertySets = 0,		// number of property sets
			DBPROPSET			*rgPropertySets = NULL	// the array of property sets
		);

	//Read the value of a column in the indexe schema rowset
	//RETURNS FALSE if the value was read and differ than the original one
	BOOL	GetIndexValue(
		LPVOID		pVariable,		// [OUT]	value read
		BOOL		*pfSet,			// [OUT]	if the value is set
		DBBINDING	*rgDBBINDING,	// [IN]		binding array
		ULONG		cColumn,		// [IN]		column to be read
		ULONG		ulDBTYPE,		// [IN]		type of property variant
		BYTE		*pData,			// [IN]		pointer to read DATA stru
		WCHAR		*lpwszMesaj		// [IN]		message text for error
	);

	//Read the value of a column in the indexe schema rowset
	//RETURNS FALSE if the value was read and differ than the original one
	BOOL GetIndexValueFromFirstRow(
		LPVOID		pVariable,		// [OUT]	value read
		BOOL		*pfSet,			// [OUT]	if the value is set
		DBBINDING	*rgDBBINDING,	// [IN]		binding array
		ULONG		cColumn,		// [IN]		column to be read
		ULONG		ulDBTYPE,		// [IN]		type of property variant
		BYTE		*pData,			// [IN]		pointer to read DATA stru
		DBPROPID	PropID,			// [IN]		property that is being read
		ULONG		cPropSets,		// [IN]		number of property sets
		DBPROPSET	*rgPropSets,	// [IN]		array of property sets
		WCHAR		*lpwszMesaj		// [IN]		message text for error
	);

	HRESULT		DoesIndexExist(	
		DBID		*pTableID,				// @parm [IN]	Table ID										  
		DBID		*pIndexID,				// @parm [IN]	Index ID
		BOOL 		*pfExists				// @parm [OUT] TRUE if index exists
	);

	HRESULT		DoesIndexExistInIndexSchemaRowset(	
		DBID		*pTableID,				// @parm [IN]	Table ID										  
		DBID		*pIndexID,				// @parm [IN]	Index ID
		BOOL 		*pfExists				// @parm [OUT]	TRUE if index exists
	);

	HRESULT DoesIndexExistRowsetIndex(	
		DBID		*pTableID,				// @parm [IN]	Table ID										  
		DBID		*IndexID,				// @parm [IN]	Index ID
		BOOL 		*pfExists				// @parm [OUT]	TRUE if index exists
	);

	HRESULT CreateIndex(
		DBID		*pTableID,
		DBID		*pIndexID,
		DBORDINAL	cCols,
		DBINDEXCOLUMNDESC * pIndexColumnDescs,
		ULONG		cPropertySets,
		DBPROPSET	rgPropertySets[],
		DBID **     ppIndexID,
		BOOL		fNextIndex = FALSE,
		BOOL		fSetDefaultOrder = TRUE,
		DBINDEX_COL_ORDER eIndexOrderDefault = DBINDEX_COL_ORDER_ASC
	);

	HRESULT CreateAlterableIndex(
		DBID		*pTableID,
		DBID		*pIndexID,
		DBORDINAL	cCols,
		DBINDEXCOLUMNDESC * pIndexColumnDescs,
		ULONG		cPropertySets,
		DBPROPSET	rgPropertySets[],
		DBID **     ppIndexID,
		BOOL		fNextIndex = FALSE,
		BOOL		fSetDefaultOrder = TRUE,
		DBINDEX_COL_ORDER eIndexOrderDefault = DBINDEX_COL_ORDER_ASC
	);

	BOOL IncrementIndex(
		DBORDINAL cCols,
		ULONG * pIndexCols,
		DBORDINAL cColumnDesc
	);

	BOOL FindRow(
		IRowset* pIRowset,
		HACCESSOR hAccessor,
		LPBYTE pData,
		DBCOUNTITEM cBindings,
		DBBINDING * pBindings,
		DBLENGTH ulRowSize,
		HROW * phRow,
		ULONG * pulRowNum = NULL
	);

	void VerifyNulls(
		HRESULT hr,
		CTable * pTable,
		DBID * pIndexID,
		ULONG ulIndexNullVal,
		DBORDINAL cIndexCols,
		DBINDEXCOLUMNDESC *	pIndexColDesc
		);

	void VerifyUnique
	(
		HRESULT hr,
		CTable * pTable,
		DBID * pIndexID,
		VARIANT_BOOL vbValue,
		DBORDINAL cIndexCols,
		DBINDEXCOLUMNDESC *	pIndexColDesc
	);

	void VerifyPrimaryKey
	(
		HRESULT hr,
		CTable * pTable,
		DBID * pIndexID,
		VARIANT_BOOL vbValue,
		DBORDINAL cIndexCols,
		DBINDEXCOLUMNDESC *	pIndexColDesc
	);

	void VerifyNullCollation
	(
		HRESULT hr,
		CTable * pTable,
		DBID * pIndexID,
		ULONG ulIndexNullVal,
		DBORDINAL cIndexCols,
		DBINDEXCOLUMNDESC *	pIndexColDesc
	);

	BOOL GetQualifierNames(
		IUnknown * pSessionIUnknown,// [in]		IUnknown off session object
		LPWSTR	pwszTableName,		// [in]		the name of the table
		LPWSTR	*ppwszCatalogName,	// [out]	catalog name
		LPWSTR	*ppwszSchemaName	// [out]	schema name
	);

	HRESULT CleanUpIndex(
		DBID		*pTableID,
		DBID		**ppIndexID,
		DBINDEXCOLUMNDESC * pIndexColumnDescs,
		ULONG *		pcPropertySets,
		DBPROPSET ** ppPropertySets
	);

	void FreeIndexColumnDesc(DBORDINAL * pcColDesc, DBINDEXCOLUMNDESC ** ppIndexColumnDesc, BOOL fFreeBuf);

	//Thread Methods
	static ULONG WINAPI Thread_VerifyAlterIndex(LPVOID pv);
};



//----------------------------------------------------------------------
// CAlterIndex::CAlterIndex
//
CAlterIndex::CAlterIndex(WCHAR * wstrTestCaseName)	: CSessionObject(wstrTestCaseName) 
{
	m_ulIRowsetIndex	= OIS_NONE;
	m_ulOpenIndexSupport= OIS_NONE;
	m_cColumnDesc		= 0;
	m_rgColumnDesc		= NULL;
	m_cPropSets			= 0;
	m_rgPropSets		= NULL;
	m_pIndexID			= NULL;

	m_pwszInvalidTableChars				= NULL;
	m_pwszInvalidTableStartingChars		= NULL;
 	m_pwszInvalidIndexChars				= NULL;
	m_pwszInvalidIndexStartingChars		= NULL;
	m_pwszInvalidColumnChars			= NULL;

	m_pIAlterIndex		= NULL;
	m_pIIndexDef		= NULL;
	m_pIDBSchemaRowset	= NULL;
	m_pIndexColumnDesc	= NULL;
	m_cIndexColumnDesc	= 0;
	m_fSetOnlyDefault	= FALSE;
	m_pNoNullTable		= NULL;
}

//----------------------------------------------------------------------
// CAlterIndex::~CAlterIndex
//
CAlterIndex::~CAlterIndex(void)
{
	if (m_pNoNullTable)
	{
		CHECK(m_pNoNullTable->DropTable(), S_OK);
		SAFE_DELETE(m_pNoNullTable);
	}
}

//----------------------------------------------------------------------
// CAlterIndex::Init
//
BOOL CAlterIndex::Init()
{
	TBEGIN
	HRESULT					hr;
	ULONG					i=0;
	DBID					*pIndexID = NULL;
	CList <WCHAR*, WCHAR*>	ListNativeTemp;
	CList <DBTYPE, DBTYPE>	ListDataTypes;
	ULONG					cSchema = 0;
	DBCOUNTITEM				iRow = 0;
	ULONG*					prgRestrictions = NULL;
	GUID*					prgSchemas = NULL;
	IRowsetIndex*			pIRowsetIndex = NULL;
	LPWSTR					pwszProviderFileName = NULL;

	// These get init'd in the constructor, but if test is run twice constructor
	// isn't called again...
	m_pIndexColumnDesc		= NULL;
	m_cIndexColumnDesc		= 0;

	TESTC(CSessionObject::Init())

	SetDataSourceObject(m_pThisTestModule->m_pIUnknown, TRUE);
	SetDBSession(m_pThisTestModule->m_pIUnknown2);
	GetLiteralInfo();

	TESTC(VerifyInterface(GetModInfo()->GetThisTestModule()->m_pIUnknown2, 
		IID_IAlterIndex, SESSION_INTERFACE, (IUnknown**)&m_pIAlterIndex))

	//If IAlterIndex is supported, IIndexDefinition also has
	//to be supported.
	TESTC(VerifyInterface(m_pIAlterIndex, IID_IIndexDefinition, 
		SESSION_INTERFACE, (IUnknown**)&m_pIIndexDef))

	//Check if IDBSchemaRowset is supported.
	if(!VerifyInterface(m_pIAlterIndex, IID_IDBSchemaRowset, 
		SESSION_INTERFACE, (IUnknown**)&m_pIDBSchemaRowset))
	{
		TESTC(!m_pIDBSchemaRowset)
		odtLog<<L"INFO: IDBSchemaRowset is not supported.\n";
	}

	if(m_pIDBSchemaRowset)
	{
		//Check whether it supports INDEXES Schema Rowset and its restrictions.
		//If not then might as well release m_pIDBSchemaRowset
		//to indicate it.
		TESTC_(m_hr = m_pIDBSchemaRowset->GetSchemas(&cSchema, &prgSchemas, &prgRestrictions), S_OK)

		for(i=0; i<cSchema; i++)
		{
			if(prgSchemas[i] == DBSCHEMA_INDEXES)
				break;
		}
		if(	   (i >= cSchema)
			|| !(prgRestrictions[i] & 0x4)
			|| !(prgRestrictions[i] & 0x10))
		{
			SAFE_RELEASE(m_pIDBSchemaRowset);
			odtLog<<L"INFO: INDEXES Schema Rowset or table and index restrictions are not supported.\n";
		}
	}

	// create a table and get info about all the data types
	GetModInfo()->UseITableDefinition(TRUE);

	// We have to create a table with no nulls or some index properties can't be set.
	// Many providers require no nulls for the unique index column, and there is a 
	// separate DBPROP_INDEX_NULLS we want to test.  We can add null values later.
	// But if we want to have no nulls but insert nulls later we have to fool privlib
	m_pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName, USENULLS);
	TESTC(NULL != m_pTable);
	
	// To test PRIMARYKEY prop we need a table that doesn't allow nulls in columns
	m_pNoNullTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName, NONULLS);
	TESTC(NULL != m_pNoNullTable);

	// get the provider types list
	m_pTable->CreateColInfo(ListNativeTemp, ListDataTypes, ALLTYPES);
	// get the column description array
	TESTC_(m_pTable->BuildColumnDescs(&m_rgColumnDesc), S_OK);
	m_cColumnDesc=m_pTable->CountColumnsOnTable();

	m_pTable->SetColumnDesc(m_rgColumnDesc, m_cColumnDesc);
	m_pTable->SetBuildColumnDesc(FALSE);

	// Create table with no rows but nullable columns
	TESTC(SUCCEEDED(m_hr = m_pTable->CreateTable(0, 0))); 

	// Create the no-null table with no index
	TESTC(SUCCEEDED(m_hr = m_pNoNullTable->CreateTable(MIN_TABLE_ROWS, 0))); 

	if (GetModInfo()->GetFileName())
	{
		// If running with ini file we don't get to specify table props (NONULLS)
		// so make sure they're what we need.  The problem is there's no good way to
		// do this. We want nullable columns but no nulls in the table.  NONULLS makes
		// non-nullable columns with no nulls.  USENULLS makes NULLABLE columns and uses
		// nulls.  At this time just skip.
		TESTC_PROVIDER(FALSE);
	}
	else
	{
		// Change the NONULLS flag to prevent nulls being inserted into table
		m_pTable->SetNull(NONULLS);

		// Insert as many rows as we want.  The table may already have rows if using ini file
		for (iRow = m_pTable->GetNextRowNumber(); iRow <= MIN_TABLE_ROWS; iRow++)
		{
			TESTC_(m_pTable->Insert(iRow),S_OK);
		}
	}

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(m_pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cColumnDesc);
	memset(m_pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cColumnDesc));

	//The INIT will vary depending on type of Test Case. Different
	//cases will differ in the type of Index being created.

	// Set properties needed for index creation
	SetIndexCreationProps(m_eTestCase);

	// Create index with required props and cols.  We'll skip if provider
	// can't support the number of cols or prop requested.
	TESTC_PROVIDER(S_OK == CreateIndex(&(m_pTable->GetTableID()), NULL, m_cIndexColumnDesc,
		m_pIndexColumnDesc, m_cPropSets, m_rgPropSets, &m_pIndexID));

	// We should have created an index
	TESTC(m_pIndexID != NULL);

	//Check if IRowsetIndex is supported. Set m_ulIRowsetIndex.
	//Providers can support separate indexes, integrated indexes, or both.
	m_ulIRowsetIndex = OIS_NONE;
	m_ulOpenIndexSupport = OIS_NONE;

	// Try to open a rowset with integrated index
	hr = m_pIOpenRowset->OpenRowset(NULL, &(m_pTable->GetTableID()),
		m_pIndexID, IID_IRowsetIndex, 0, NULL, (IUnknown**)
		&pIRowsetIndex); 

	// Release integrated index/table rowset
	SAFE_RELEASE(pIRowsetIndex);

	// Provider supports all	-> S_OK
	// No IRowsetIndex support	-> E_NOINTERFACE
	// No open index support	-> DB_E_NOINDEX
	TEST3C_(hr, S_OK, DB_E_NOINDEX, E_NOINTERFACE);

	if (S_OK == hr)
	{
		m_ulIRowsetIndex	|= OIS_INTEGRATED;
		m_ulOpenIndexSupport|= OIS_INTEGRATED;
	}
	else if (E_NOINTERFACE == hr)
	{
		IRowset * pIRowset = NULL;

		hr = m_pIOpenRowset->OpenRowset(NULL, &(m_pTable->GetTableID()),
			m_pIndexID, IID_IRowset, 0, NULL, (IUnknown**)
			&pIRowset); 

		SAFE_RELEASE(pIRowset);

		if (S_OK == hr)
			m_ulOpenIndexSupport|= OIS_INTEGRATED;
	}

	// Try to open a rowset with separate index
	hr = m_pIOpenRowset->OpenRowset(NULL, NULL,
		m_pIndexID, IID_IRowsetIndex, 0, NULL, (IUnknown**)
		&pIRowsetIndex); 

	SAFE_RELEASE(pIRowsetIndex);

	TEST3C_(hr, S_OK, DB_E_NOINDEX, E_NOINTERFACE);

	if (S_OK == hr)
	{
		m_ulIRowsetIndex	|= OIS_ROWSET;
		m_ulOpenIndexSupport|= OIS_ROWSET;
	}
	else if (E_NOINTERFACE == hr)
	{
		IRowset * pIRowset = NULL;

		hr = m_pIOpenRowset->OpenRowset(NULL, NULL,
			m_pIndexID, IID_IRowset, 0, NULL, (IUnknown**)
			&pIRowset); 

		SAFE_RELEASE(pIRowset);

		if (S_OK == hr)
			m_ulOpenIndexSupport|= OIS_ROWSET;
	}


	// Determine whether provider only supports setting props to default (current)
	// value via AlterIndex.
	m_fSetOnlyDefault = FALSE;
	if (GetProperty(DBPROP_PROVIDERFILENAME, DBPROPSET_DATASOURCEINFO, 
		m_pThisTestModule->m_pIUnknown, &pwszProviderFileName) &&
		COMPARE(pwszProviderFileName != NULL, TRUE))
	{
		for (i=0; i < NUMELEM(ProviderSupportsDefaultOnly); i++)
		{
			if (!wcscmp(pwszProviderFileName, ProviderSupportsDefaultOnly[i]))
				m_fSetOnlyDefault = TRUE;
		}
	}

CLEANUP:
	if(pIndexID)
		ReleaseDBID(pIndexID);
	SAFE_FREE(pwszProviderFileName);
	SAFE_FREE(prgSchemas);
	SAFE_FREE(prgRestrictions);
	SAFE_RELEASE(pIRowsetIndex);
	TRETURN
} //Init

//----------------------------------------------------------------------
// CAlterIndex::Terminate
//
BOOL CAlterIndex::Terminate()
{
	m_pTable->SetColumnDesc(NULL);
	ReleaseColumnDesc(m_rgColumnDesc, m_cColumnDesc);
	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	if(m_cPropSets && m_rgPropSets)
		FreeProperties(&m_cPropSets, &m_rgPropSets);

	if(m_pIndexID)
	{
		ReleaseDBID(m_pIndexID); //Also frees the DBID memory.
		m_pIndexID = NULL;
	}
	SAFE_FREE(m_pIndexColumnDesc);
	SAFE_FREE(m_pwszInvalidTableChars);
	SAFE_FREE(m_pwszInvalidTableStartingChars);
 	SAFE_FREE(m_pwszInvalidIndexChars);
	SAFE_FREE(m_pwszInvalidIndexStartingChars);
	SAFE_FREE(m_pwszInvalidColumnChars);

	SAFE_RELEASE(m_pIAlterIndex);
	SAFE_RELEASE(m_pIOpenRowset);
	SAFE_RELEASE(m_pIIndexDef);
	SAFE_RELEASE(m_pIDBSchemaRowset);

	ReleaseDBSession();
	ReleaseDataSourceObject();
	return CSessionObject::Terminate();
} //Terminate

//-------------------------------------------------------------------------
// CAlterIndex::AlterIndex
// Wrapper for corresponding method of IAlterIndex.
//
HRESULT	CAlterIndex::AlterIndex(DBID* pIndexID, DBID* pNewIndexID, BOOL	fValidIndex)
{
	return AlterIndex(&(m_pTable->GetTableID()), pIndexID, pNewIndexID, fValidIndex);
}

//-------------------------------------------------------------------------
// CAlterIndex::AlterIndex
// Wrapper for corresponding method of IAlterIndex.
//
HRESULT	CAlterIndex::AlterIndex(DBID * pTableID, DBID* pIndexID, DBID* pNewIndexID, BOOL fValidIndex)
{
	HRESULT		hr = E_FAIL;
	BOOL		fExists = FALSE;
	BOOL		fNewIndex = TRUE;

	if(!m_pIAlterIndex)
		return E_FAIL;

	hr = m_pIAlterIndex->AlterIndex(pTableID, pIndexID, 
		pNewIndexID, m_cPropSets, m_rgPropSets);

	// See if this is a new index being created
	if ((pIndexID && !pNewIndexID) ||
		(pIndexID == pNewIndexID) ||
		CompareDBID(*pIndexID, *pNewIndexID, m_pThisTestModule->m_pIUnknown))
		fNewIndex = FALSE;

	// If pNewIndexID is the same as pIndexID or NULL then the index
	// is unchanged. But since we always verify on pNewIndex ID we 
	// have to set it.
	if (!pNewIndexID)
		pNewIndexID = pIndexID;

	// If AlterIndex was successful or this was a duplicate index make sure
	// the index actually exists
	if(hr==S_OK || hr==DB_S_ERRORSOCCURRED || hr == DB_E_DUPLICATEINDEXID)
	{
		if(S_OK == DoesIndexExist(pTableID, 
			pNewIndexID, &fExists) && COMPARE(fExists, TRUE))
		{
			// Check values in index.  For DB_E_DUPLICATEINDEXID the props
			// should remain as before.
			// TODO: Retain previous props and pass to this function for verification
			// when DB_E_DUPLICATEINDEXID is returned
			COMPARE(CheckIndex(pTableID, pNewIndexID,
				(SUCCEEDED(hr)) ? m_cPropSets : 0, m_rgPropSets), TRUE);
		}

		// We were successful, any old index must not exist
		if(SUCCEEDED(hr) && fNewIndex && S_OK == DoesIndexExist(pTableID, 
			pIndexID, &fExists))
			COMPARE(fExists, FALSE);
	}
	else
	{
		// An error occurred, any new index must not exist
		if(fNewIndex && S_OK == DoesIndexExist(pTableID, 
			pNewIndexID, &fExists))
			COMPARE(fExists, FALSE);

		// But the previous index should always still exist if there was one
		if(fValidIndex && pIndexID && S_OK == DoesIndexExist(pTableID, 
			pIndexID, &fExists) && COMPARE(fExists, TRUE))
		{
			// Check values in index.
			// TODO: Retain previous props and pass to this function for verification
			// when an error is returned
			COMPARE(CheckIndex(pTableID, pIndexID,
				(SUCCEEDED(hr)) ? m_cPropSets : 0, m_rgPropSets), TRUE);
		}

	}

	// If S_OK is returned all props must have DBPROPSTATUS_OK.
	// If DB_E/S_ERRORSOCCURRED was returned one of the props must have status set
	COMPARE(VerifyProperties(hr, m_cPropSets, m_rgPropSets), TRUE);

	return hr;
} //AlterIndex

BOOL CAlterIndex::IncrementIndex(DBORDINAL cCols, ULONG * pIndexCols, DBORDINAL cColumnDesc)
{
	if (!cCols)
		return FALSE;

	ASSERT(*pIndexCols < cColumnDesc);

	if (*pIndexCols < cColumnDesc)
	{
		(*pIndexCols)++;
		if (*pIndexCols == cColumnDesc)
		{
			*pIndexCols = 0;
			return IncrementIndex(--cCols, ++pIndexCols, cColumnDesc);
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CAlterIndex::FindRow
(
	IRowset* pIRowset,
	HACCESSOR hAccessor,
	LPBYTE pData,
	DBCOUNTITEM cBindings,
	DBBINDING * pBindings,
	DBLENGTH ulRowSize,
	HROW * phRow,
	ULONG * pulRowNum
)
{
	LPBYTE pFindData			= NULL;
	DBCOUNTITEM cRowsObtained	= 0;
	ULONG iRow					= 0;
	BOOL fFound					= FALSE;
	HRESULT hr					= E_FAIL;

	// Validate args
	TESTC(pIRowset && pData && cBindings &&
		pBindings && ulRowSize && phRow);

	*phRow = DB_NULL_HROW;

	SAFE_ALLOC(pFindData, BYTE, ulRowSize);

	TESTC_(pIRowset->RestartPosition(NULL), S_OK);

	while(S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &phRow)))
	{
		iRow++;

		// Retrieve the data for the row
		TESTC_(pIRowset->GetData(*phRow, hAccessor, pFindData), S_OK);

		// See if the row matches
		if (CompareBuffer(pFindData, pData, cBindings, pBindings, NULL,
			TRUE, FALSE, COMPARE_ONLY))
		{
			// This is our row
			fFound = TRUE;
			break;
		}

		CHECK(pIRowset->ReleaseRows(1, phRow, NULL, NULL, NULL), S_OK);
		*phRow = DB_NULL_HROW;

		CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pFindData, FALSE), S_OK);
	}

	// S_OK - We found the row, no need to search farther
	// DB_S_ENDOFROWSET - Didn't find our row
	TEST2C_(hr, S_OK, DB_S_ENDOFROWSET);

CLEANUP:

	CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pFindData, TRUE), S_OK);

	if (pulRowNum)
		*pulRowNum = iRow;

	return fFound;
}

//-------------------------------------------------------------------------
// CAlterIndex::CreateAlterableIndex
// Create an index that can be altered to the desire props.
//
HRESULT CAlterIndex::CreateAlterableIndex(
	DBID		*pTableID,
	DBID		*pIndexID,
	DBORDINAL	cCols,
	DBINDEXCOLUMNDESC * pIndexColumnDescs,
	ULONG		cPropertySets,
	DBPROPSET	rgPropertySets[],
	DBID **     ppIndexID,
	BOOL		fNextIndex,
	BOOL		fSetDefaultOrder,
	DBINDEX_COL_ORDER eIndexOrderDefault
)
{
	HRESULT hr = E_FAIL;

	TESTC(pTableID != NULL);
	TESTC(ppIndexID != NULL);

	// Create an index with the desired props so we can find the columns
	// that we can later alter.
	hr = CreateIndex(pTableID, pIndexID, cCols,
		pIndexColumnDescs, cPropertySets, rgPropertySets, ppIndexID);

	// If the provider doesn't allow setting props at AlterIndex time then
	// leave the index created with props so we can alter to "default" values.
	if (!m_fSetOnlyDefault)
	{
		// Drop and release the index that has the props set
		CHECK(m_pIIndexDef->DropIndex(pTableID, *ppIndexID), S_OK); 
		ReleaseDBID(*ppIndexID);

		// If we created an index with the desired props
		if (S_OK == hr)
		{
			// Create an index to use that has no props set using the same columns.
			TESTC_(hr = CreateIndex(pTableID, pIndexID, cCols,
				pIndexColumnDescs, 0, NULL, ppIndexID), S_OK);
		}
	}

CLEANUP:

	return hr;

}

//-------------------------------------------------------------------------
// CAlterIndex::CreateIndex
// Wrapper for corresponding method of IIndexDefinition.
//
HRESULT CAlterIndex::CreateIndex(
	DBID		*pTableID,
	DBID		*pIndexID,
	DBORDINAL	cCols,
	DBINDEXCOLUMNDESC * pIndexColumnDescs,
	ULONG		cPropertySets,
	DBPROPSET	rgPropertySets[],
	DBID **     ppIndexID,
	BOOL		fNextIndex,
	BOOL		fSetDefaultOrder,
	DBINDEX_COL_ORDER eIndexOrderDefault
)
{
	ULONG iCol;
	BOOL * pfSetDBID = NULL;
	BOOL fNewDBID = TRUE;
	HRESULT hr = DB_E_ERRORSOCCURRED;
	DBID * pNewIndexID = NULL;
	ULONG * rgIndexCols = NULL;

	SAFE_ALLOC(pfSetDBID, BOOL, cCols);
	memset(pfSetDBID, 0, (size_t)(cCols*sizeof(BOOL)));

	SAFE_ALLOC(rgIndexCols, ULONG, cCols);
	memset(rgIndexCols, 0, (size_t)(cCols*sizeof(ULONG)));

	// Set up initial conditions
	// Iterate over all desired columns
	for (iCol = 0; iCol < cCols; iCol++)
	{
		// Set the default index order if requested
		if (fSetDefaultOrder)
			pIndexColumnDescs[iCol].eIndexColOrder = eIndexOrderDefault;

		// If no DBID was specified for the column then start with the first one
		// by default (index 0, set above with memset)
		if (!pIndexColumnDescs[iCol].pColumnID)
		{
			// Remember which ones to set
			pfSetDBID[iCol] = TRUE;
		}
		else
		{
			// Find the matching index
			for (; rgIndexCols[iCol] < m_cColumnDesc; rgIndexCols[iCol]++)
				if (pIndexColumnDescs[iCol].pColumnID ==
					&m_rgColumnDesc[rgIndexCols[iCol]].dbcid)
					break;
		}
	}

	// Increment to next possible index if requested
	if (fNextIndex)
		fNewDBID = IncrementIndex(cCols, rgIndexCols, m_cColumnDesc);

	// While we have a new set of DBID's we haven't tried
	while (fNewDBID)
	{
		// Set the dbid for each index column desc
		for (iCol=0; iCol <  cCols; iCol++)
			pIndexColumnDescs[iCol].pColumnID = &m_rgColumnDesc[rgIndexCols[iCol]].dbcid;

		// Now we have a set of DBID's as a candidate for an index so try to create
		if (SUCCEEDED(hr = m_pIIndexDef->CreateIndex(pTableID, pIndexID,
			cCols, pIndexColumnDescs, cPropertySets, rgPropertySets,
			&pNewIndexID)))
		{
			BOOL fExists = FALSE;

			SAFE_ALLOC(*ppIndexID, DBID, 1);
			memset(*ppIndexID, 0, sizeof(DBID));
			DuplicateDBID(*pNewIndexID, *ppIndexID);
			fNewDBID = FALSE;

			// Just because CreateIndex returned S_OK doesn't mean the index exists,
			// so check to make sure
			TESTC_(DoesIndexExist(pTableID, pNewIndexID, &fExists), S_OK);

			TESTC(fExists == TRUE);

			break;
		}

		fNewDBID = IncrementIndex(cCols, rgIndexCols, m_cColumnDesc);
	}

CLEANUP:

	if (pNewIndexID)
	{
		ReleaseDBID(pNewIndexID);
		pNewIndexID = NULL;
	}

	SAFE_FREE(pfSetDBID);
	SAFE_FREE(rgIndexCols);

	return hr;

} //CreateIndex


//-------------------------------------------------------------------------
// CAlterIndex::VerifyNulls
//
//		Verify table/index null handling matches expected based on setting
//
void CAlterIndex::VerifyNulls
(
	HRESULT hr,
	CTable * pTable,
	DBID * pIndexID,
	ULONG ulIndexNullVal,
	DBORDINAL cIndexCols,
	DBINDEXCOLUMNDESC *	pIndexColDesc
)
{
	HRESULT hrNullInsert		= E_FAIL;
	BOOL	fFindInsertedNull	= FALSE;
	DBORDINAL cStates			= 2 << (cIndexCols-1);
	ULONG	iState				= 0;
	ULONG	iBind				= 0;
	ULONG	cInsertProps		= 0;
	DBPROPSET * prgInsertProps	= NULL;
	IRowsetChange *	pIRowsetChange = NULL;
	IRowset *	pIRowset		= NULL;
	HACCESSOR hAccessor			= DB_NULL_HACCESSOR;
	DBCOUNTITEM cBindings		= 0;
	DBBINDING *	pBindings		= NULL;
	DBLENGTH ulRowSize			= 0;
	HROW hRow					= DB_NULL_HROW;
	HROW * phRow				= &hRow;
	LPBYTE pData				= NULL;
	DBORDINAL cCols				= pTable->CountColumnsOnTable();
	CCol TempCol;


	// If we didn't set successfully, then no need to verify
	if (FAILED(hr))
		return;

	// We have to know the table and index
	TESTC(pTable != NULL);

	TESTC(pIndexID != NULL);

	switch(ulIndexNullVal)
	{
		case DBPROPVAL_IN_ALLOWNULL:
			hrNullInsert = S_OK;		// Inserting NULL key will succeed
			fFindInsertedNull = TRUE;	// We will see the NULL key with OpenRowset
			break;
		case DBPROPVAL_IN_DISALLOWNULL:
			hrNullInsert = DB_E_INTEGRITYVIOLATION; // Inserting NULL key will fail
			fFindInsertedNull = FALSE;	// We will not see the NULL key with OpenRowset
			break;
		case DBPROPVAL_IN_IGNORENULL:
			hrNullInsert = S_OK;		// Inserting NULL key will succeed 
			fFindInsertedNull = FALSE;	// We will not see the NULL key with OpenRowset
			break;
		case DBPROPVAL_IN_IGNOREANYNULL:
			hrNullInsert = S_OK;		// Inserting NULL key will succeed
			fFindInsertedNull = FALSE;	// We will not see the NULL key with OpenRowset
			break;
		default:
			TESTC(FALSE);
			break;
	}

	// Set props to allow inserts
	TESTC(SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, &cInsertProps, &prgInsertProps, (void*)(DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE), DBTYPE_I4, DBPROPOPTIONS_REQUIRED));

	// OpenRowset on table using no index requesting insert
	TESTC_(m_pIOpenRowset->OpenRowset(NULL, &pTable->GetTableID(),
		NULL, IID_IRowsetChange, cInsertProps, prgInsertProps, (IUnknown**)
		&pIRowsetChange), S_OK); 

	TESTC(VerifyInterface(pIRowsetChange, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pIRowset));
	
	// GetAccessorAndBindings for all rows, no bookmark
	TESTC_(hr = GetAccessorAndBindings(
						pIRowsetChange, DBACCESSOR_ROWDATA, &hAccessor,	&pBindings,		
						&cBindings, &ulRowSize, DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
						ALL_COLS_EXCEPTBOOKMARK, FORWARD, NO_COLS_BY_REF,
						NULL,					// OUT: Array of DBCOLUMNINFOs
						0,						// OUT: Count of DBCOULMNINFOs
						NULL,					//&pStringsBuffer, 
						DBTYPE_EMPTY, 
						0, NULL, NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG),S_OK); // NO_BLOB_COLS);
	
	// For each state
	// States = 2^N, where N == numer of key columns, i.e. col is null or not null.
	for (iState = 0; iState < cStates; iState++)
	{
		BOOL fNULLKey = FALSE;
		ULONG cBitsSet = 0;

		// Fill input bindings with data for new row
		TESTC_(FillInputBindings(pTable,  
			DBACCESSOR_ROWDATA, cBindings, pBindings, &pData, pTable->GetNextRowNumber(), 
			0, NULL, PRIMARY), S_OK);

		// Insert NULL in key field for each bit set
		for(ULONG iBit = 0; iBit < cIndexCols; iBit++)
		{
			// If the bit is set in this state
			if (iState & (1 << iBit))
			{
				// Find this index column in the cTable
				for (iBind = 0; iBind < cBindings; iBind++)
				{
					// Retrieve colinfo for this column
					TESTC_(pTable->GetColInfo(pBindings[iBind].iOrdinal, TempCol), S_OK);

					// If the DBID's match this is the right column
					if (CompareDBID(*pIndexColDesc[iBit].pColumnID,
						*TempCol.GetColID(), m_pThisTestModule->m_pIUnknown))
						break;
				}
				
				// Make sure we found the index col
				TESTC(iBind < cBindings);

				// If the column is nullable insert a NULL.  Some columns aren't nullable,
				// such as autoinc columns.
				if (TempCol.GetNullable())
				{
					// Insert NULL in this (iBit) key field
					STATUS_BINDING(pBindings[iBind], pData) = DBSTATUS_S_ISNULL;

					cBitsSet++;

					ASSERT(cBitsSet <= cIndexCols);
				}
			}
		} // Next bit

		// fNULLKey = cBitsSet == n
		fNULLKey = (cIndexCols == cBitsSet);

		// Attempt to insert row.  If NULL key expect hrNullInsert, otherwise S_OK.
		hr = pIRowsetChange->InsertRow(NULL, hAccessor, pData, NULL);

		// For NULL key we can have either either DB_E_INTEGRITYVIOLATION or
		// DB_E_ERRORSOCCURRED when failure is expected
		if (fNULLKey && FAILED(hrNullInsert))
		{
			if (hr == DB_E_ERRORSOCCURRED)
			{
				// Status must be DBSTATUS_E_INTEGRITYVIOLATION for key columns
				for(ULONG iBit = 0; iBit < cIndexCols; iBit++)
				{
					// If the bit is set in this state
					if (iState & (1 << iBit))
					{
						// Find this index column in the cTable
						for (iBind = 0; iBind < cBindings; iBind++)
						{
							// Retrieve colinfo for this column
							TESTC_(pTable->GetColInfo(pBindings[iBind].iOrdinal, TempCol), S_OK);

							// If the DBID's match this is the right column
							if (CompareDBID(*pIndexColDesc[iBit].pColumnID,
								*TempCol.GetColID(), m_pThisTestModule->m_pIUnknown))
								break;
						}
						
						// Make sure we found the index col
						TESTC(iBind < cBindings);

						// Check the status
						COMPARE(STATUS_BINDING(pBindings[iBind], pData), DBSTATUS_E_INTEGRITYVIOLATION);
					}
					else
						// Check the status
						COMPARE(STATUS_BINDING(pBindings[iBind], pData), DBSTATUS_E_UNAVAILABLE);
				}
			}
			else
				CHECK(hr, hrNullInsert);
		}
		else
			CHECK(hr, S_OK);

		// If insertion was successful
		if (SUCCEEDED(hr))
		{
			IRowset * pIRowsetWithIndex = NULL;
			IAccessor * pIAccessorIndex = NULL;
			HROW hRowPos = DB_NULL_HROW;
			HRESULT hrGetNextRows = S_OK;
			ULONG iRow = 0;
			HACCESSOR hIndexAccessor = DB_NULL_HACCESSOR;

			// OpenRowset using index and find the key value expecting fFindInsertedNull.
				// Note this will succeed only on providers supporting opening indexes via
				// OpenRowset (i.e. Jolt).

			if (m_ulOpenIndexSupport & OIS_INTEGRATED)
			{
				// Open the rowset with no props but using the index
				TESTC_(m_pIOpenRowset->OpenRowset(NULL, &pTable->GetTableID(),
					pIndexID, IID_IRowset, 0, NULL, (IUnknown**)
					&pIRowsetWithIndex), S_OK); 

				TESTC(VerifyInterface(pIRowsetWithIndex, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessorIndex));

				// Create an accessor off this rowset using our known bindings
				TESTC_(hr = pIAccessorIndex->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, pBindings,
					0, &hIndexAccessor, NULL), S_OK);

				// Find our inserted row
				COMPARE(FindRow(pIRowsetWithIndex, hAccessor, pData, cBindings, pBindings, ulRowSize, &hRowPos, NULL), fNULLKey ? fFindInsertedNull : TRUE);

				if (hRowPos != DB_NULL_HROW)
					CHECK(pIRowsetWithIndex->ReleaseRows(1, &hRowPos, NULL, NULL, NULL), S_OK);

				SAFE_RELEASE_ACCESSOR(pIAccessorIndex, hIndexAccessor);
				SAFE_RELEASE(pIAccessorIndex);
				SAFE_RELEASE(pIRowsetWithIndex);
			}
			else if (m_ulOpenIndexSupport & OIS_ROWSET)
			{
				TESTC(FALSE);
				odtLog << L"Provider only supports non-integrated index, need more code.\n";
			}

			// Now delete the inserted row
			if (COMPARE(FindRow(pIRowset, hAccessor, pData, cBindings, pBindings, ulRowSize, phRow), TRUE))
			{
				TESTC_(pIRowsetChange->DeleteRows(NULL, 1, &hRow, NULL), S_OK);

				CHECK(pIRowset->ReleaseRows(1, phRow, NULL, NULL, NULL), S_OK);
			}
		}

	}  // Next state

CLEANUP:

	// Release binding memory and pData buffer
	FreeProperties(&cInsertProps, &prgInsertProps);
	CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData, TRUE), S_OK);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowset);

	return;
}

//-------------------------------------------------------------------------
// CAlterIndex::VerifyUnique
//
//		Verify unique handling matches expected based on setting
//
void CAlterIndex::VerifyUnique
(
	HRESULT hr,
	CTable * pTable,
	DBID * pIndexID,
	VARIANT_BOOL vbValue,
	DBORDINAL cIndexCols,
	DBINDEXCOLUMNDESC *	pIndexColDesc
)
{
	HRESULT hrInsertNonUnique	= E_FAIL;
	BOOL	fFindInserted		= FALSE;
	DBORDINAL cStates			= 2 << (cIndexCols-1);
	ULONG	iState				= 0;
	ULONG	iBind				= 0;
	ULONG	cInsertPropSets		= 0;
	DBPROPSET * prgInsertPropSets= NULL;
	IRowsetChange *	pIRowsetChange = NULL;
	IRowset *	pIRowset		= NULL;
	IAccessor * pIAccessor		= NULL;
	HACCESSOR hAccessor			= DB_NULL_HACCESSOR;
	DBCOUNTITEM cBindings		= 0;
	DBBINDING *	pBindings		= NULL;
	DBLENGTH ulRowSize			= 0;
	HROW hRow					= DB_NULL_HROW;
	HROW * phRow				= &hRow;
	LPBYTE pData				= NULL;
	DBORDINAL cCols				= pTable->CountColumnsOnTable();
	CCol TempCol;


	// If we didn't set successfully, then no need to verify
	if (FAILED(hr))
		return;

	// We have to know the table and index
	TESTC(pTable != NULL);

	TESTC(pIndexID != NULL);

	switch(vbValue)
	{
		case VARIANT_TRUE:
			hrInsertNonUnique = DB_E_INTEGRITYVIOLATION;// Inserting non unique key will fail
			fFindInserted = FALSE;						// We will not see the unique value with OpenRowset
			break;
		case VARIANT_FALSE:
			hrInsertNonUnique = S_OK;	// Inserting non unique key will succeed
			fFindInserted = TRUE;		// We will see the unique value with OpenRowset
			break;
		default:
			TESTC(FALSE);
			break;
	}

	// Set props to allow inserts
	TESTC(SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, &cInsertPropSets, &prgInsertPropSets, (void*)(DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE), DBTYPE_I4, DBPROPOPTIONS_REQUIRED));

	// For each state
	// States = 2^N, where N == numer of key columns, i.e. col is null or not null.
	for (iState = 0; iState < cStates; iState++)
	{
		BOOL fDupeKey = FALSE;
		ULONG cBitsSet = 0;

		if (!pIRowsetChange)
		{
			// OpenRowset on table using no index requesting insert
			TESTC_(m_pIOpenRowset->OpenRowset(NULL, &pTable->GetTableID(),
				NULL, IID_IRowsetChange, cInsertPropSets, prgInsertPropSets, (IUnknown**)
				&pIRowsetChange), S_OK); 

			TESTC(VerifyInterface(pIRowsetChange, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pIRowset));

			TESTC(VerifyInterface(pIRowsetChange, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor));

			if (!cBindings)
			{	
				ASSERT(pBindings == NULL);

				// GetAccessorAndBindings for all rows, no bookmark
				TESTC_(hr = GetAccessorAndBindings(
									pIRowsetChange, DBACCESSOR_ROWDATA, &hAccessor,	&pBindings,		
									&cBindings, &ulRowSize, DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
									ALL_COLS_EXCEPTBOOKMARK, FORWARD, NO_COLS_BY_REF,
									NULL,					// OUT: Array of DBCOLUMNINFOs
									0,						// OUT: Count of DBCOULMNINFOs
									NULL,					//&pStringsBuffer, 
									DBTYPE_EMPTY, 
									0, NULL, NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG),S_OK); // NO_BLOB_COLS);
			}
			else
				TESTC_(hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, pBindings,
					0, &hAccessor, NULL), S_OK);
		}
	
		// Fill input bindings with data for new row.  Since specifying PRIMARY row seed
		// can result in duplicate data in a given row we use SECONDARY data.
		TESTC_(FillInputBindings(pTable,  
			DBACCESSOR_ROWDATA, cBindings, pBindings, &pData, pTable->GetNextRowNumber(), 
			0, NULL, SECONDARY), S_OK);

		// Insert non-unique in key field for each bit set
		for(ULONG iBit = 0; iBit < cIndexCols; iBit++)
		{
			// If the bit is set in this state
			if (iState & (1 << iBit))
			{
				// Find this index column in the cTable
				for (iBind = 0; iBind < cBindings; iBind++)
				{
					// Retrieve colinfo for this column
					TESTC_(pTable->GetColInfo(pBindings[iBind].iOrdinal, TempCol), S_OK);

					// If the DBID's match this is the right column
					if (CompareDBID(*pIndexColDesc[iBit].pColumnID,
						*TempCol.GetColID(), m_pThisTestModule->m_pIUnknown))
						break;
				}
				
				// Make sure we found the index col
				TESTC(iBind < cBindings);

				// If the column is nullable insert a NULL.  Some columns aren't nullable,
				// such as autoinc columns.
				if (TempCol.GetUpdateable())
				{
					// Insert duplicate in this (iBit) key field
					// This changes the data to duplicate the last row in this column
					CHECK(ReleaseInputBindingsMemory(1, &pBindings[iBind], pData, FALSE), S_OK);
					TESTC_(FillInputBindings(pTable,  
						DBACCESSOR_ROWDATA, 1, &pBindings[iBind], &pData, pTable->GetNextRowNumber()-1, 
						0, NULL, PRIMARY), S_OK);

					cBitsSet++;

					ASSERT(cBitsSet <= cIndexCols);
				}
			}
		} // Next bit

		// fDupeKey = cBitsSet == n
		fDupeKey = (cIndexCols == cBitsSet);

		// Attempt to insert row.  If NULL key expect hrNullInsert, otherwise S_OK.
		if (1)
		{
			CHECK(hr = pIRowsetChange->InsertRow(NULL, hAccessor, pData, NULL),
				fDupeKey ? hrInsertNonUnique : S_OK);
		}
//		hr = E_FAIL;

		// If insertion was successful
		if (SUCCEEDED(hr))
		{
			IRowset * pIRowsetWithIndex = NULL;
			IAccessor * pIAccessorIndex = NULL;
			HROW hRowPos = DB_NULL_HROW;
			HRESULT hrGetNextRows = S_OK;
			ULONG iRow = 0;
			HACCESSOR hIndexAccessor = DB_NULL_HACCESSOR;

			// OpenRowset using index and find the key value expecting fFindInsertedNull.
				// Note this will succeed only on providers supporting opening indexes via
				// OpenRowset (i.e. Jolt).

			if (m_ulOpenIndexSupport & OIS_INTEGRATED)
			{
				// Open the rowset with no props but using the index
				TESTC_(m_pIOpenRowset->OpenRowset(NULL, &pTable->GetTableID(),
					pIndexID, IID_IRowset, 0, NULL, (IUnknown**)
					&pIRowsetWithIndex), S_OK); 

				TESTC(VerifyInterface(pIRowsetWithIndex, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessorIndex));

				// Create an accessor off this rowset using our known bindings
				TESTC_(hr = pIAccessorIndex->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, pBindings,
					0, &hIndexAccessor, NULL), S_OK);

				// Find our inserted row
				COMPARE(FindRow(pIRowsetWithIndex, hAccessor, pData, cBindings, pBindings, ulRowSize, &hRowPos, NULL), fDupeKey ? fFindInserted : TRUE);

				if (hRowPos != DB_NULL_HROW)
					CHECK(pIRowsetWithIndex->ReleaseRows(1, &hRowPos, NULL, NULL, NULL), S_OK);

				SAFE_RELEASE_ACCESSOR(pIAccessorIndex, hIndexAccessor);
				SAFE_RELEASE(pIAccessorIndex);
				SAFE_RELEASE(pIRowsetWithIndex);
			}
			else if (m_ulOpenIndexSupport & OIS_ROWSET)
			{
				TESTC(FALSE);
				odtLog << L"Provider only supports non-integrated index, need more code.\n";
			}
				
			// Delete the row to prevent impact on other variations

			// Because some providers don't allow CHANGEINSERTEDROWS without a unique index
			// we need to open a rowset and search through the rows ourselves to delete the
			// new row.
			if (COMPARE(FindRow(pIRowset, hAccessor, pData, cBindings, pBindings, ulRowSize, phRow), TRUE))
			{
				TESTC_(pIRowsetChange->DeleteRows(NULL, 1, &hRow, NULL), S_OK);

				CHECK(pIRowset->ReleaseRows(1, phRow, NULL, NULL, NULL), S_OK);
			}
		}

		CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData, FALSE), S_OK);

	}  // Next state

CLEANUP:

	// Release binding memory and pData buffer
	FreeProperties(&cInsertPropSets, &prgInsertPropSets);
	CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData, TRUE), S_OK);
	SAFE_FREE(pBindings);
	SAFE_RELEASE_ACCESSOR(pIAccessor, hAccessor);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowset);

	return;
}

//-------------------------------------------------------------------------
// CAlterIndex::VerifyPrimaryKey
//
//		Verify unique handling matches expected based on setting
//
void CAlterIndex::VerifyPrimaryKey
(
	HRESULT hr,
	CTable * pTable,
	DBID * pIndexID,
	VARIANT_BOOL vbValue,
	DBORDINAL cIndexCols,
	DBINDEXCOLUMNDESC *	pIndexColDesc
)
{
	// Primary keys are always unique
	VerifyUnique(hr, pTable, pIndexID, vbValue, cIndexCols, pIndexColDesc);

	// They also disallow nulls
	if (vbValue == VARIANT_TRUE)
		VerifyNulls(hr, pTable, pIndexID, DBPROPVAL_IN_DISALLOWNULL, cIndexCols, pIndexColDesc);
	else
		VerifyNulls(hr, pTable, pIndexID, DBPROPVAL_IN_ALLOWNULL, cIndexCols, pIndexColDesc);

}


//-------------------------------------------------------------------------
// CAlterIndex::VerifyNullCollation
//
//		Verify table/index null handling matches expected based on setting
//
void CAlterIndex::VerifyNullCollation
(
	HRESULT hr,
	CTable * pTable,
	DBID * pIndexID,
	ULONG ulIndexNullVal,
	DBORDINAL cIndexCols,
	DBINDEXCOLUMNDESC *	pIndexColDesc
)
{
	BOOL	fFindNullAtEnd		= FALSE;
	ULONG iBit					= 0;
	ULONG	iState				= 0;
	ULONG	iBind				= 0;
	ULONG	cInsertProps		= 0;
	DBPROPSET * prgInsertProps	= NULL;
	IRowsetChange *	pIRowsetChange = NULL;
	IRowset *	pIRowset		= NULL;
	HACCESSOR hAccessor			= DB_NULL_HACCESSOR;
	DBCOUNTITEM cBindings		= 0;
	DBBINDING *	pBindings		= NULL;
	DBLENGTH ulRowSize			= 0;
	HROW hRow					= DB_NULL_HROW;
	HROW * phRow				= &hRow;
	LPBYTE pData				= NULL;
	BOOL fNULLKey				= FALSE;
	ULONG cBitsSet				= 0;
	DBORDINAL cCols;
	CCol TempCol;
	DBINDEX_COL_ORDER eCollationFirstKey;

	// If we didn't set successfully, then no need to verify
	if (FAILED(hr))
		return;

	// We have to know the table and index
	TESTC(pTable != NULL);

	TESTC(pIndexID != NULL);

	TESTC(pIndexColDesc != NULL);

	cCols	= pTable->CountColumnsOnTable();
	eCollationFirstKey = pIndexColDesc[0].eIndexColOrder;

	switch(ulIndexNullVal)
	{
		case DBPROPVAL_NC_END:
			fFindNullAtEnd = TRUE;		// We will see the NULL key at the end of the rowset
			break;
		case DBPROPVAL_NC_START:
			fFindNullAtEnd = FALSE;		// We will see the NULL key at the start of the rowset
			break;
		case DBPROPVAL_NC_HIGH:
			if (eCollationFirstKey == DBINDEX_COL_ORDER_ASC)
				fFindNullAtEnd = TRUE;	// We will see the NULL key at the end of the rowset
			else
				fFindNullAtEnd = FALSE;	// We will see the NULL key at the start of the rowset
			break;
		case DBPROPVAL_NC_LOW:
			if (eCollationFirstKey == DBINDEX_COL_ORDER_DESC)
				fFindNullAtEnd = TRUE;	// We will see the NULL key at the end of the rowset
			else
				fFindNullAtEnd = FALSE;	// We will see the NULL key at the start of the rowset
			break;
		default:
			TESTC(FALSE);
			break;
	}

	// Set props to allow inserts
	TESTC(SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, &cInsertProps, &prgInsertProps, (void*)(DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE), DBTYPE_I4, DBPROPOPTIONS_REQUIRED));

	// OpenRowset on table using no index requesting insert
	TESTC_(m_pIOpenRowset->OpenRowset(NULL, &pTable->GetTableID(),
		NULL, IID_IRowsetChange, cInsertProps, prgInsertProps, (IUnknown**)
		&pIRowsetChange), S_OK); 

	TESTC(VerifyInterface(pIRowsetChange, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pIRowset));
	
	// GetAccessorAndBindings for all rows, no bookmark
	TESTC_(hr = GetAccessorAndBindings(
						pIRowsetChange, DBACCESSOR_ROWDATA, &hAccessor,	&pBindings,		
						&cBindings, &ulRowSize, DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
						ALL_COLS_EXCEPTBOOKMARK, FORWARD, NO_COLS_BY_REF,
						NULL,					// OUT: Array of DBCOLUMNINFOs
						0,						// OUT: Count of DBCOULMNINFOs
						NULL,					//&pStringsBuffer, 
						DBTYPE_EMPTY, 
						0, NULL, NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG),S_OK); // NO_BLOB_COLS);
	
	// For each state
	// States = 2^N, where N == numer of key columns, i.e. col is null or not null.
	// The problem here is that we end up inserting keys where for example the first key part
	// is non-null and the next is null, then we have to validate where these show up in the
	// rowset based on data in the first column and index null collation in the second key 
	// column.  At this point I don't intend to test multipart null collation inserts the
	// second or greater key value appropriately, it's just too much work for the gain.
	iState = ~0; // All bits set

	// Fill input bindings with data for new row
	TESTC_(FillInputBindings(pTable,  
		DBACCESSOR_ROWDATA, cBindings, pBindings, &pData, pTable->GetNextRowNumber(), 
		0, NULL, PRIMARY), S_OK);

	// Insert NULL in key field for each bit set
	for(iBit = 0; iBit < cIndexCols; iBit++)
	{
		// If the bit is set in this state
		if (iState & (1 << iBit))
		{
			// Find this index column in the cTable
			for (iBind = 0; iBind < cBindings; iBind++)
			{
				// Retrieve colinfo for this column
				TESTC_(pTable->GetColInfo(pBindings[iBind].iOrdinal, TempCol), S_OK);

				// If the DBID's match this is the right column
				if (CompareDBID(*pIndexColDesc[iBit].pColumnID,
					*TempCol.GetColID(), m_pThisTestModule->m_pIUnknown))
					break;
			}
			
			// Make sure we found the index col
			TESTC(iBind < cBindings);

			// If the column is nullable insert a NULL.  Some columns aren't nullable,
			// such as autoinc columns.
			if (TempCol.GetNullable())
			{
				// Insert NULL in this (iBit) key field
				STATUS_BINDING(pBindings[iBind], pData) = DBSTATUS_S_ISNULL;

				cBitsSet++;

				ASSERT(cBitsSet <= cIndexCols);
			}
		}
	} // Next bit

	// fNULLKey = cBitsSet == n
	fNULLKey = (cIndexCols == cBitsSet);

	// Attempt to insert row.  
	CHECK(hr = pIRowsetChange->InsertRow(NULL, hAccessor, pData, NULL), S_OK);

	// If insertion was successful
	if (SUCCEEDED(hr))
	{
		IRowset * pIRowsetWithIndex = NULL;
		IAccessor * pIAccessorIndex = NULL;
		HROW hRowPos = DB_NULL_HROW;
		HRESULT hrGetNextRows = S_OK;
		ULONG iRow = 0;
		HACCESSOR hIndexAccessor = DB_NULL_HACCESSOR;

		// OpenRowset using index and find the key value expecting proper location.
			// Note this will succeed only on providers supporting opening indexes via
			// OpenRowset (i.e. Jolt).

		if (m_ulOpenIndexSupport & OIS_INTEGRATED)
		{
			// Open the rowset with no props but using the index
			TESTC_(m_pIOpenRowset->OpenRowset(NULL, &pTable->GetTableID(),
				pIndexID, IID_IRowset, 0, NULL, (IUnknown**)
				&pIRowsetWithIndex), S_OK); 

			TESTC(VerifyInterface(pIRowsetWithIndex, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessorIndex));

			// Create an accessor off this rowset using our known bindings
			TESTC_(hr = pIAccessorIndex->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, pBindings,
				0, &hIndexAccessor, NULL), S_OK);

			// Find our inserted row
			if (COMPARE(FindRow(pIRowsetWithIndex, hAccessor, pData, cBindings, pBindings, ulRowSize, phRow, &iRow), TRUE))
			{
				// Release the row handle we got with FindRow
				CHECK(pIRowsetWithIndex->ReleaseRows(1, phRow, NULL, NULL, NULL), S_OK);
				*phRow = DB_NULL_HROW;		
				
				// See if the row came back in the right spot
				if (fFindNullAtEnd)
					// Must be the last row in the table
					COMPARE(iRow, pTable->CountRowsOnTable());
				else
					// Otherwise, must be the first row
					COMPARE(iRow, 1);
			}

			SAFE_RELEASE_ACCESSOR(pIAccessorIndex, hIndexAccessor);
			SAFE_RELEASE(pIAccessorIndex);
			SAFE_RELEASE(pIRowsetWithIndex);
		}
		else if (m_ulOpenIndexSupport & OIS_ROWSET)
		{
			TESTC(FALSE);
			odtLog << L"Provider only supports non-integrated index, need more code.\n";
		}

		if (COMPARE(FindRow(pIRowset, hAccessor, pData, cBindings, pBindings, ulRowSize, phRow), TRUE))
		{
			TESTC_(pIRowsetChange->DeleteRows(NULL, 1, &hRow, NULL), S_OK);

			CHECK(pIRowset->ReleaseRows(1, phRow, NULL, NULL, NULL), S_OK);
		}
	}

CLEANUP:

	// Release binding memory and pData buffer
	FreeProperties(&cInsertProps, &prgInsertProps);
	CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData, TRUE), S_OK);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowset);

	return;
}

//-------------------------------------------------------------------------
// CAlterIndex::SetIndexCreationProps
//
void CAlterIndex::SetIndexCreationProps(ETESTCASE eTestCase)
{
	// We assume no props are set at this point
	ASSERT(!m_cPropSets);

	switch(eTestCase)
	{
		//Create Index with only one column and no props.
		case TC_SingleColNoProps:
		case TC_PropSingCol:
		{
			m_cIndexColumnDesc = 1;
			break;
		}

		//Create Index with only one column and some props.
		case TC_SingleColProps:
		{
			m_cIndexColumnDesc = 1;
			//Set one Index prop (required)
			SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
			break;
		}

		//Create Index with multiple columns and no props.
		case TC_MultipleColsNoProps:
		case TC_PropMultCol:
		{
			m_cIndexColumnDesc = MAX_INDEX_COLS;
			break;
		}

		//Create Index with multiple columns some props.
		case TC_MultipleColsProps:
		{
			m_cIndexColumnDesc = MAX_INDEX_COLS;
			//Set one Index prop (required)
			SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
			break;
		}
		
		default:
			ASSERT(!L"Unhandled Type...");
			break;
	};
}

ULONG CAlterIndex::Thread_VerifyAlterIndex(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables 
	IAlterIndex * pIAlterIndex = (IAlterIndex *)THREAD_FUNC;
	DBID * pTableID		= (DBID *)THREAD_ARG1;
	DBID * pIndexID		= (DBID *)THREAD_ARG2;
	DBID * pNewID		= (DBID *)THREAD_ARG3;
	HRESULT * pHR		= (HRESULT *)THREAD_ARG4;
	ULONG cPropSets		= (ULONG)(ULONG_PTR)THREAD_ARG5;
	DBPROPSET * pPropSets = (DBPROPSET *)THREAD_ARG6;

	ASSERT(pIAlterIndex);
	ASSERT(pTableID);
	ASSERT(pIndexID);
	ASSERT(pNewID);

	ThreadSwitch(); //Let the other thread(s) catch up

	//Call AlterIndex
	*pHR = pIAlterIndex->AlterIndex(pTableID, pIndexID, pNewID, cPropSets, pPropSets);

	ThreadSwitch(); //Let the other thread(s) catch up

	THREAD_RETURN
}



//-------------------------------------------------------------------------
// CAlterIndex::GetLiteralInfo
//
HRESULT	CAlterIndex::GetLiteralInfo()
{
	IDBInfo*		pInterface = NULL;
	const int		nLiteral=3;
	DBLITERAL		rgLiteral[nLiteral]={DBLITERAL_TABLE_NAME, DBLITERAL_INDEX_NAME, DBLITERAL_COLUMN_NAME};
	DBLITERALINFO*	rgLiteralInfo = NULL;
	ULONG			cLiteralInfo, i;
	OLECHAR*		pCharBuffer = NULL;
	IGetDataSource*	pIGetDataSource=NULL;	// IGetDataSource interface pointer

	if(!VerifyInterface(m_pIOpenRowset, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDataSource))
		return E_FAIL;

	m_hr=pIGetDataSource->GetDataSource(IID_IDBInfo,(IUnknown**)&pInterface);
	SAFE_RELEASE(pIGetDataSource);

	TESTC_(m_hr = pInterface->GetLiteralInfo(	nLiteral, rgLiteral, &cLiteralInfo, 
												&rgLiteralInfo, &pCharBuffer), S_OK);

	for (i=0; i< cLiteralInfo; i++)
	{
		switch (rgLiteralInfo[i].lt)
		{
			case DBLITERAL_TABLE_NAME:
				// get the maximum size of a valid table name and the invalid chars for a table name
				m_cMaxTableName = rgLiteralInfo[i].cchMaxLen;
				SAFE_FREE(m_pwszInvalidTableChars);
				m_pwszInvalidTableChars = wcsDuplicate(rgLiteralInfo[i].pwszInvalidChars?
					rgLiteralInfo[i].pwszInvalidChars: L"");
				SAFE_FREE(m_pwszInvalidTableStartingChars);
				m_pwszInvalidTableStartingChars = wcsDuplicate(rgLiteralInfo[i].pwszInvalidStartingChars?
					rgLiteralInfo[i].pwszInvalidStartingChars: L"");
				break;
			case DBLITERAL_INDEX_NAME:
				m_cMaxIndexName = rgLiteralInfo[i].cchMaxLen;
				SAFE_FREE(m_pwszInvalidIndexChars);
				m_pwszInvalidIndexChars = wcsDuplicate(rgLiteralInfo[i].pwszInvalidChars?
					rgLiteralInfo[i].pwszInvalidChars: L"");
				SAFE_FREE(m_pwszInvalidIndexStartingChars);
				m_pwszInvalidIndexStartingChars = wcsDuplicate(rgLiteralInfo[i].pwszInvalidStartingChars?
					rgLiteralInfo[i].pwszInvalidStartingChars: L"");
				break;
			case DBLITERAL_COLUMN_NAME:
				// get the maximum size of a valid column name and the invalid chars for a table name
				m_cMaxColumnName = rgLiteralInfo[i].cchMaxLen;
				SAFE_FREE(m_pwszInvalidColumnChars);
				m_pwszInvalidColumnChars = wcsDuplicate(rgLiteralInfo[i].pwszInvalidChars?
					rgLiteralInfo[i].pwszInvalidChars: L"");
				break;
			default:
				break;
		}
	}

CLEANUP:
	SAFE_RELEASE(pInterface);
	SAFE_FREE(pCharBuffer);
	SAFE_FREE(rgLiteralInfo);
	return m_hr;
} //GetLiteralInfo

//-------------------------------------------------------------------------
// CAlterIndex::BuildValidName
// Build a valid table name of a certain length
//
WCHAR* CAlterIndex::BuildValidName(size_t length, WCHAR* pattern)
{
	WCHAR*	pwszBuffer;
	size_t	cLen = wcslen(pattern), i;

	i=length+1;
	SAFE_ALLOC(pwszBuffer, WCHAR, i);
	memset(pwszBuffer, 0, i*sizeof(WCHAR));
	for (i=0; i< length - cLen; i+=cLen)
		wcscat(pwszBuffer, pattern);
	// manage the rest of the characters
	wcsncat(pwszBuffer, pattern, length-i);
CLEANUP:
	return pwszBuffer;
} //BuildValidName

//-------------------------------------------------------------------------
// CAlterIndex::BuildInvalidName
// Build an invalid table name of a certain length the pattern 
// is supposed to be shorter than the string to be build.
//
WCHAR* CAlterIndex::BuildInvalidName(size_t length, WCHAR* pattern, WCHAR* invchars)
{
	WCHAR*	pwszBuffer;
	size_t	cLen = wcslen(pattern), i, cInvLen=wcslen(invchars);

	SAFE_ALLOC(pwszBuffer, WCHAR, length+1);
	if (length > cInvLen+cLen)
	{
		wcscpy(pwszBuffer, pattern);
		wcscat(pwszBuffer, invchars);
	}
	else
	{
		wcscpy(pwszBuffer, L"aa");
		wcsncat(pwszBuffer, invchars, length-wcslen(pwszBuffer));
		return pwszBuffer;
	}
	for (i=wcslen(pwszBuffer); i< length - cLen; i+=cLen)
		wcscat(pwszBuffer, pattern);
	// manage the rest of the characters
	for (; i<length; i++)
		wcscat(pwszBuffer, L"a");
CLEANUP:
	return pwszBuffer;
} //BuildInvalidName

//--------------------------------------------------------------------------
// CAlterIndex::CheckProperty
// Check the result and property status
//
BOOL CAlterIndex::CheckProperty(
	const GUID	guidPropertySet,// [in]	property set guid (should be DBPROPSET_INDEX
	DBPROP		*pProp,			// [in]	property
	HRESULT		hr				// [in] the result
)
{
	BOOL		fRes = TRUE;
	BOOL		fSupported;
	BOOL		fSettable;

	if (!pProp)
		return TRUE;
	
	// general checking
	if (DBPROPOPTIONS_REQUIRED == pProp->dwOptions && DBPROPSTATUS_OK != pProp->dwStatus)
		fRes = CHECK(FAILED(hr), TRUE);

	if (!(guidPropertySet == DBPROPSET_INDEX))
	{
		// check a couple of things and returns
		return COMPARE(pProp->dwStatus, DBPROPSTATUS_NOTSUPPORTED) && COMPARE(S_OK != hr, TRUE);
	}

	fSupported	= SupportedProperty(pProp->dwPropertyID, guidPropertySet);
	fSettable	= SettableProperty(pProp->dwPropertyID, guidPropertySet);

	if (!fSupported && !CHECK(S_OK != hr, TRUE))
		fRes = FALSE;
		
	// status driven checking
	switch (pProp->dwStatus)
	{
		case DBPROPSTATUS_OK:
			fRes = COMPARE(fSupported != 0, TRUE);
			break;
		case DBPROPSTATUS_NOTSUPPORTED:
			fRes = COMPARE(fSupported == 0, TRUE);
			break;
		case DBPROPSTATUS_NOTSETTABLE:
			fRes = COMPARE(fSupported != 0, TRUE) && COMPARE(fSettable == 0, TRUE);
			break;
		case DBPROPSTATUS_BADOPTION:
			fRes =	(pProp->dwOptions != DBPROPOPTIONS_OPTIONAL) 
				&&	COMPARE(DBPROPOPTIONS_REQUIRED != pProp->dwOptions, TRUE);
			break;
		case DBPROPSTATUS_CONFLICTING:
			fRes = COMPARE(fSettable != 0, TRUE);
			break;
		case DBPROPSTATUS_BADVALUE:
			fRes = COMPARE(fSupported != 0, TRUE);
			break;
		default:
			break;
	}

	// DBPROPSTATUS_BADVALUE can be certainly got only if type of the prop is wrong
	// info about supported discret values of the prop cannot be generacally obtained 
	// before creating an index
	if (	GetPropInfoType(pProp->dwPropertyID, guidPropertySet) != pProp->vValue.vt
		&&	!COMPARE(DBPROPSTATUS_OK == pProp->dwStatus, FALSE))
	{
			odtLog << "ERROR: bad propstatus on improper type value on property" << pProp->dwPropertyID << "\n";
			fRes = FALSE;
	}

	return fRes;
} //CheckProperty

//-------------------------------------------------------------------------
// CAlterIndex::CheckIndex
// Check whether all the columns specified appear in index
// and the properties are properly set
// returns	TRUE - everything was set ok
//			FALSE - problems
//
BOOL CAlterIndex::CheckIndex(
		DBID				*pTableID,				// the index of the table
		DBID				*pIndexID,				// the index to be checked
		ULONG				cPropertySets,			// number of property sets
		DBPROPSET			*rgPropertySets			// the array of property sets
)
{
	HRESULT 			hr				= E_FAIL;
	BOOL				fReturn			= FALSE;
	BOOL				fProps			= TRUE;
	BOOL				bIsSchemaSupported;
	DBLENGTH			ulRowSize		= 0;		// size of row
	DBCOUNTITEM			cDBBINDING		= 0;		// count of bindings
	ULONG 				iRow			= 0;		// count of rows
	DBCOUNTITEM 		cRowsObtained	= 0;		// number of rows returned, should be 1
	ULONG				cSchema			= 0;		// number of supported Schemas
	ULONG				*prgRestrictions= 0;		// restrictions for each Schema
	GUID				*prgSchemas		= NULL;		// array of GUIDs
	HROW				hRow;						// handler of rows
	HROW				*phRow = &hRow;				// pointer to handler
	IRowset				*pIndexRowset	= NULL;		// returned rowset
	DBBINDING			*rgDBBINDING	= NULL;		// array of bindings
	BYTE				*pData			= NULL;		// pointer to data
	HACCESSOR 			hAccessor		= NULL;		// accessor
	BOOL				*rgColPresent = NULL;
	const int			cRest = 5;
	VARIANT				rgRestrictIndexes[cRest];
	ULONG				i, index;
	ULONG				nColPresent=0;
	ULONG				nTABLE_NAME			= 2;
	ULONG				nINDEX_NAME			= 5;
	ULONG				nPRIMARY_KEY		= 6;
	ULONG				nUNIQUE				= 7;
	ULONG				nCLUSTERED			= 8;
	ULONG				nTYPE				= 9;
	ULONG				nFILL_FACTOR		= 10;
	ULONG				nINITIAL_SIZE		= 11;
	ULONG				nNULLS				= 12;
	ULONG				nSORT_BOOKMARKS		= 13;
	ULONG				nAUTO_UPDATE		= 14;
	ULONG				nNULL_COLLATION		= 15;
	ULONG				nORDINAL_POSITION	= 16;
	ULONG				nCOLUMN_NAME		= 17;
	ULONG				nCOLLATION			= 20;

	// column entries in index schema rowset and their presence flags
	LPWSTR				pwszIndexName		= NULL;
	BOOL				fIndexName			= FALSE;
	LPWSTR				pwszTableName		= NULL;
	BOOL				fTableName			= FALSE;
	VARIANT_BOOL		vbAutoUpdate;
	BOOL				fAutoUpdate			= FALSE;
	VARIANT_BOOL		vbClustered;
	BOOL				fClustered			= FALSE;
	LONG				lFillFactor;
	BOOL				fFillFactor			= FALSE;
	LONG				lInitialSize;
	BOOL				fInitialSize		= FALSE;
	LONG				lNullCollation;
	BOOL				fNullCollation		= FALSE;
	BOOL				fCollation			= FALSE;
	LPWSTR				pwszColumnName		= NULL;
	BOOL				fColumnName			= FALSE;
	LONG				lNulls;
	BOOL				fNulls				= FALSE;
	VARIANT_BOOL		vbPrimaryKey;
	BOOL				fPrimaryKey			= FALSE;
	VARIANT_BOOL		vbSortBookmarks;
	BOOL				fSortBookmarks		= FALSE;
	unsigned short		usType;
	BOOL				fType				= FALSE;
	VARIANT_BOOL		vbUnique;
	BOOL				fUnique				= FALSE;

	// Set restrictions
	for(index=0;index<cRest;index++)
		VariantInit(&rgRestrictIndexes[index]);

	if (m_ulIRowsetIndex)
		TESTC(CheckIndex2(pTableID, pIndexID, cPropertySets, 
			rgPropertySets));

	if (NULL == m_pIDBSchemaRowset)
	{
		fReturn = TRUE;
		goto CLEANUP;
	}

	// Check to see if the schema is supported
	TESTC_(hr = m_pIDBSchemaRowset->GetSchemas(&cSchema, &prgSchemas, &prgRestrictions),S_OK);

	if (DBKIND_NAME == pIndexID->eKind)
	{
		pwszIndexName = wcsDuplicate(pIndexID->uName.pwszName);
		fIndexName = TRUE;
	}

	if (DBKIND_NAME == pTableID->eKind)
	{
		pwszTableName = wcsDuplicate(pTableID->uName.pwszName);
		fTableName = TRUE;
	}

	// Check to see if DBSCHEMA_INDEXES is supported
	for(i=0, bIsSchemaSupported=FALSE; i<cSchema && !bIsSchemaSupported;)
	{
		if(prgSchemas[i] == DBSCHEMA_INDEXES)
			bIsSchemaSupported = TRUE;
		else 
			i++;
	}

	if(!bIsSchemaSupported || !(prgRestrictions[i] & 0x4) || !(prgRestrictions[i] & 0x10))
	{
		odtLog << "Index Schema Rowset or the required constraints are not supported\n";
		goto CLEANUP;
	}

	rgRestrictIndexes[2].vt 	 = VT_BSTR;
	rgRestrictIndexes[2].bstrVal = SysAllocString(pIndexID->uName.pwszName);
	rgRestrictIndexes[4].vt 	 = VT_BSTR;
	rgRestrictIndexes[4].bstrVal = SysAllocString(pTableID->uName.pwszName);

	TESTC_(hr = m_pIDBSchemaRowset->GetRowset(	
			NULL, 								// aggregation
			DBSCHEMA_INDEXES,					// REFGUID
			cRest,	 							// count of restrictions (1:types)
			rgRestrictIndexes,					// list of restrictions
			IID_IRowset,						// REFFID
			0,									// count of properties
			NULL,								// range of properties
			(IUnknown**)&pIndexRowset			// returned result set
		),S_OK);

	TESTC_(hr = GetAccessorAndBindings(
						pIndexRowset, DBACCESSOR_ROWDATA, &hAccessor,	&rgDBBINDING,		
						&cDBBINDING, &ulRowSize, DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
						ALL_COLS_EXCEPTBOOKMARK, FORWARD, NO_COLS_BY_REF,
						NULL,					// OUT: Array of DBCOLUMNINFOs
						0,						// OUT: Count of DBCOULMNINFOs
						NULL,					//&pStringsBuffer, 
						DBTYPE_EMPTY, 
						0, NULL),S_OK);
	SAFE_ALLOC(pData, BYTE, ulRowSize);	//data

	// read the first row, check local data and initialize variables
	// TODO: Read all the rows and verify the columns that were indexed and
	// their collation, ordinal position, etc.
	// Also verify integrated based on how openrowset performs.
	TESTC_(hr=pIndexRowset->GetNextRows(0, 0, 1, &cRowsObtained, &phRow), S_OK);
	TESTC_((long)cRowsObtained, 1);
	TESTC_(hr=pIndexRowset->GetData(hRow, hAccessor, pData),S_OK);

	// get the first values and verifications
	fProps = fProps && GetIndexValue(&pwszIndexName, &fIndexName, 
		rgDBBINDING, nINDEX_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad index name\n");

	fProps = fProps && GetIndexValue(&pwszTableName, &fTableName, 
		rgDBBINDING, nTABLE_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad table name\n");

	if (cPropertySets > 0 && rgPropertySets)
	{
		fProps = fProps && GetIndexValueFromFirstRow(&vbAutoUpdate, &fAutoUpdate, 
			rgDBBINDING, nAUTO_UPDATE, DBTYPE_BOOL, pData, DBPROP_INDEX_AUTOUPDATE, 
			cPropertySets, rgPropertySets, L"ERROR: Auto update\n");

		fProps = fProps && GetIndexValueFromFirstRow(&vbClustered, &fClustered, 
			rgDBBINDING, nCLUSTERED, DBTYPE_BOOL, pData, DBPROP_INDEX_CLUSTERED, 
			cPropertySets, rgPropertySets, L"ERROR: Clustered\n");

		fProps = fProps && GetIndexValueFromFirstRow(&lFillFactor, &fFillFactor, 
			rgDBBINDING, nFILL_FACTOR, DBTYPE_I4, pData, DBPROP_INDEX_FILLFACTOR, 
			cPropertySets, rgPropertySets, L"ERROR: Fill factor\n");

		fProps = fProps && GetIndexValueFromFirstRow(&lInitialSize, &fInitialSize, 
			rgDBBINDING, nINITIAL_SIZE, DBTYPE_I4, pData, DBPROP_INDEX_INITIALSIZE, 
			cPropertySets, rgPropertySets, L"ERROR: Initial Size\n");

		fProps = fProps && GetIndexValueFromFirstRow(&lNullCollation, &fNullCollation, 
			rgDBBINDING, nNULL_COLLATION, DBTYPE_I4, pData, DBPROP_INDEX_NULLCOLLATION,
			cPropertySets, rgPropertySets, L"ERROR: Null Collation\n");

		fProps = fProps && GetIndexValueFromFirstRow(&lNulls, &fNulls, 
			rgDBBINDING, nNULLS, DBTYPE_I4, pData, DBPROP_INDEX_NULLS,
			cPropertySets, rgPropertySets, L"ERROR: Nulls\n");

		fProps = fProps && GetIndexValueFromFirstRow(&vbPrimaryKey, &fPrimaryKey, 
			rgDBBINDING, nPRIMARY_KEY, DBTYPE_BOOL, pData, DBPROP_INDEX_PRIMARYKEY,
			cPropertySets, rgPropertySets, L"ERROR: Primary Key\n");

		fProps = fProps && GetIndexValueFromFirstRow(&vbSortBookmarks, &fSortBookmarks, 
			rgDBBINDING, nSORT_BOOKMARKS, DBTYPE_BOOL, pData, DBPROP_INDEX_SORTBOOKMARKS,
			cPropertySets, rgPropertySets, L"ERROR: SortBookmarks\n");

		fProps = fProps && GetIndexValueFromFirstRow(&usType, &fType, 
			rgDBBINDING, nTYPE, DBTYPE_UI2, pData, DBPROP_INDEX_TYPE,
			cPropertySets, rgPropertySets, L"ERROR: Type\n");

		fProps = fProps && GetIndexValueFromFirstRow(&vbUnique, &fUnique, 
			rgDBBINDING, nUNIQUE, DBTYPE_BOOL, pData, DBPROP_INDEX_UNIQUE,
			cPropertySets, rgPropertySets, L"ERROR: Unique\n");
	}

	TESTC_(pIndexRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);

	fReturn = fProps;

CLEANUP:
	// Free the memory
	SAFE_FREE(pwszIndexName);
	SAFE_FREE(pwszTableName);
	
	SAFE_FREE(rgColPresent);

	SAFE_FREE(prgRestrictions);
	SAFE_FREE(prgSchemas);

	SAFE_FREE(pData);

	SAFE_RELEASE(pIndexRowset);

	SAFE_FREE(rgDBBINDING);

	for(index=0;index<cRest;index++)
		GCHECK(VariantClear(&(rgRestrictIndexes[index])),S_OK);
	
	return fReturn;
} //CheckIndex

//-------------------------------------------------------------------------
// CAlterIndex::CheckIndex2
// Check whether all the columns specified appear in index
// and whether the properties are properly set
// uses IRowsetIndex
//-------------------------------------------------------------------------
BOOL CAlterIndex::CheckIndex2(
		DBID				*pTableID,				// the index of the table
		DBID				*pIndexID,				// the index to be checked
		ULONG				cPropertySets,			// number of property sets
		DBPROPSET			*rgPropertySets			// the array of property sets
)
{
	IRowsetIndex		*pIRowsetIndex = NULL;
	DBORDINAL			cColumns=0;
	ULONG				cIndexPropSets=0, nPropSet, nProp;
	DBINDEXCOLUMNDESC	*rgOutIndexColumnDesc = NULL;
	DBPROPSET			*rgIndexPropSets = NULL;
	BOOL				fResult = FALSE, fProp=TRUE;
	DBPROP				*pProp=NULL;
	DBPROPSTATUS		dwStatus;
	VARIANT				*pvValue;
	DBPROPID			dwPropID;


	// Use IRowsetIndex->GetIndexInfo to check how the index was set
	TESTC_(m_hr = m_pIOpenRowset->OpenRowset(
		NULL,
		m_ulIRowsetIndex & OIS_INTEGRATED ? pTableID : NULL,
		pIndexID, 
		IID_IRowsetIndex,
		0,
		NULL,
		(IUnknown**)&pIRowsetIndex
	), S_OK);


	TESTC_(pIRowsetIndex->GetIndexInfo(&cColumns, &rgOutIndexColumnDesc, &cIndexPropSets, &rgIndexPropSets), S_OK);

	if (!rgPropertySets)
	{
		fResult = TRUE;
		goto CLEANUP;
	}

	for (nPropSet=0; nPropSet<cPropertySets; nPropSet++)
	{
		if (DBPROPSET_INDEX != rgPropertySets[nPropSet].guidPropertySet)
		{
			for (nProp=0; nProp<rgPropertySets[nPropSet].cProperties; nProp++)
			{
				if (!COMPARE(rgPropertySets[nPropSet].rgProperties[nProp].dwStatus, DBPROPSTATUS_NOTSUPPORTED))
					fProp = FALSE;
			}
			continue;
		}
		if (!rgPropertySets[nPropSet].rgProperties)
			continue;
		for (nProp=0; nProp<rgPropertySets[nPropSet].cProperties; nProp++)
		{
			dwStatus	= rgPropertySets[nPropSet].rgProperties[nProp].dwStatus;
			dwPropID	= rgPropertySets[nPropSet].rgProperties[nProp].dwPropertyID;
			pvValue		= &rgPropertySets[nPropSet].rgProperties[nProp].vValue;

			// try to find coresponding property got from IRowsetIndex
			if (!FindProperty(	dwPropID,
								rgPropertySets[nPropSet].guidPropertySet,
								cIndexPropSets, 
								rgIndexPropSets, 
								&pProp))	
			{
				fProp = fProp && COMPARE(dwStatus, DBPROPSTATUS_NOTSUPPORTED);
				continue;
			}

			// prop status were check in CreateAndCheckIndex by calling CheckProperty
			if (DBPROPSTATUS_OK == dwStatus)
				fProp = fProp && COMPARE(CompareVariant(&pProp->vValue, pvValue), TRUE);
		}
	}

	fResult = fProp;

CLEANUP:
	SAFE_RELEASE(pIRowsetIndex);
	FreeProperties(&cIndexPropSets, &rgIndexPropSets);
	FreeIndexColumnDesc(&cColumns, &rgOutIndexColumnDesc, TRUE);
	return fResult;
} //CheckIndex2

//--------------------------------------------------------------------------
// CAlterIndex::GetIndexValue
// Read the value of a column in the index schema rowset
//	if *pfSet is TRUE then there is a coparison value for the value read,
// otherwise the new value will be used for further comparisons => save
// it in *pVariable and set *pfSet to TRUE
//	RETURNS FALSE if the value was read and differ than the original one
//
BOOL CAlterIndex::GetIndexValue(
	LPVOID		pVariable,		// [OUT]	value read
	BOOL		*pfSet,			// [OUT]	if the value is set
	DBBINDING	*rgDBBINDING,	// [IN]		binding array
	ULONG		cColumn,		// [IN]		column to be read
	ULONG		ulDBTYPE,		// [IN]		type of property variant
	BYTE		*pData,			// [IN]		pointer to read DATA stru
	WCHAR		*lpwszMesaj		// [IN]		message text for error
)
{
	BOOL	fRes		= FALSE;
	BOOL	fWStrComp	= FALSE;
	DATA	*pColumn	= NULL;

	TESTC(NULL != pData);
	TESTC(NULL != rgDBBINDING);
	TESTC(NULL != pVariable);
	TESTC(NULL != pfSet);
	fRes = TRUE;

	// get the status of the read property value
	pColumn = (DATA*) (pData+rgDBBINDING[cColumn].obStatus);	
	if (DBSTATUS_S_OK == pColumn->sStatus)							
	{
		// get the value in pVariable
		switch (ulDBTYPE)
		{
			case DBTYPE_I4:
				if (!*pfSet)
				{
					*(LONG*)pVariable = *(LONG*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(LONG*)(&(pColumn->bValue)) == *(LONG*)pVariable);	
				break;

			case DBTYPE_UI2:
				if (!*pfSet)
				{
					*(unsigned short*)pVariable = *(unsigned short*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(unsigned short*)(&(pColumn->bValue)) == *(unsigned short*)pVariable);
				break;

			case DBTYPE_I2:
				if (!*pfSet)
				{
					*(SHORT*)pVariable = *(SHORT*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(SHORT*)(&(pColumn->bValue)) == *(SHORT*)pVariable);
				break;

			case DBTYPE_BOOL:
				if (!*pfSet)
				{
					*(VARIANT_BOOL*)pVariable = *(VARIANT_BOOL*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(VARIANT_BOOL*)(&(pColumn->bValue)) == *(VARIANT_BOOL*)pVariable);
				break;

			case DBTYPE_WSTR:
				if (!*pfSet)
				{
					TESTC(NULL != *(WCHAR**)pVariable);
					// release the memory is wrongfully passed
					if (!*(WCHAR*)pVariable)
						SAFE_FREE(*(WCHAR**)pVariable);
					// set the variable 
					*(WCHAR**)pVariable = wcsDuplicate(*(WCHAR**)(&(pColumn->bValue)));						
					*pfSet = TRUE;
				}
				else
					// compare
					fRes =		(NULL != *(WCHAR**)pVariable) 
							&&	S_OK == CompareID(&fWStrComp, *(WCHAR**)pVariable, (WCHAR*)(&(pColumn->bValue)), m_pIDBInitialize)
							&& fWStrComp;
				break;
			default:
				ASSERT(FALSE);
				break;
		}
		if (!fRes)
			odtLog << lpwszMesaj;											
	}

CLEANUP:
	return fRes;
} //GetIndexValue

//--------------------------------------------------------------------------
// CAlterIndex::GetIndexValueFromFirstRow
// Read the value of a column in the indexe schema rowset
//	RETURNS FALSE if the value was read and differ than the original one
//
BOOL CAlterIndex::GetIndexValueFromFirstRow(
	LPVOID		pVariable,		// [OUT]	value read
	BOOL		*pfSet,			// [OUT]	if the value is set
	DBBINDING	*rgDBBINDING,	// [IN]		binding array
	ULONG		cColumn,		// [IN]		column to be read
	ULONG		ulDBTYPE,		// [IN]		type of property variant
	BYTE		*pData,			// [IN]		pointer to read DATA stru
	DBPROPID	PropID,			// [IN]		property that is being read
	ULONG		cPropSets,		// [IN]		number of property sets
	DBPROPSET	*rgPropSets,	// [IN]		array of property sets
	WCHAR		*lpwszMesaj		// [IN]		message text for error
)
{
	BOOL	fRes = TRUE;
	DBPROP	*pProp = NULL;

	TESTC(NULL != pfSet);

	*pfSet = FALSE;
	FindProperty(PropID, DBPROPSET_INDEX, cPropSets, rgPropSets, &pProp);
	if (!pProp)
		goto CLEANUP;

	// get the variable value
	switch (ulDBTYPE)
	{
		case DBTYPE_BOOL:
			*(VARIANT_BOOL*)pVariable = V_BOOL(&pProp->vValue);
			break;

		case DBTYPE_I4:
			*(LONG*)pVariable = V_I4(&pProp->vValue);
			break;

		case DBTYPE_UI2:
			*(unsigned short*)pVariable = V_UI2(&pProp->vValue);
			break;

		case DBTYPE_I2:
			*(SHORT*)pVariable = V_I2(&pProp->vValue);
			break;
		default:
			TESTC(FALSE);
	}
	*pfSet = (DBPROPSTATUS_OK == pProp->dwStatus);

CLEANUP:
	fRes = GetIndexValue(pVariable, pfSet, rgDBBINDING, cColumn, ulDBTYPE, pData, lpwszMesaj);
	return fRes;
} //GetIndexValueFromFirstRow

//---------------------------------------------------------------------------
// CAlterIndex::DoesIndexExistInIndexSchemaRowset
//
// DoesIndexExist	|
// If this index is on this table return true. If function runs correctly
// but doesn't find the table name, function will return S_OK, but fExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	The index is sought in 
// index schema rowset.
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CAlterIndex::DoesIndexExistInIndexSchemaRowset(	
	DBID		*pTableID,				// @parm [IN]	Table ID										  
	DBID		*pIndexID,				// @parm [IN]	Index ID
	BOOL 		*pfExists				// @parm [OUT]	TRUE if index exists
)
{
	HRESULT 			hr				= E_FAIL;
	DBCOUNTITEM			cRowsObtained	= 0;			// number of rows returned, should be 1
	HROW				*rghRows		= NULL;			// array of handles of rows
	IRowset				*pIRowset		= NULL;			// returned rowset
	const int			cRest = 5;						// restrictions on INDEXES Schema Rowset
	VARIANT				rgRestrictIndexes[cRest];
	ULONG				index;

	// Initialize out param
	*pfExists = FALSE;

	// Set restrictions
	for(index=0;index<cRest;index++)
		VariantInit(&rgRestrictIndexes[index]);

	TESTC(NULL != m_pIDBSchemaRowset);
	TESTC(NULL != pTableID);

	// At this time we don't have any way to determine if an index exists if 
	// we don't have the index name and table name, so we'll return S_OK and
	// set *pfExists to false
	// TODO: Track indexes so we can make sure a bogus index didn't get created
	// either on success or error cases. 
	if (!pIndexID || !DBIDHasName(*pIndexID) || !DBIDHasName(*pTableID))
		return S_OK;

	// Note that the format for restrictions is not specified by OLEDB spec so provider
	// is free to do whatever they want here.

	// The restrictions passed to GetRowset must be unquoted for every provider I've tried.
	// TODO: Remove any quotes.

	if (DBIDHasName(*pIndexID))
	{
		rgRestrictIndexes[2].vt 	 = VT_BSTR;
		rgRestrictIndexes[2].bstrVal = SysAllocString(pIndexID->uName.pwszName);
	}

	if (DBIDHasName(*pTableID))
	{
		rgRestrictIndexes[4].vt 	 = VT_BSTR;
		rgRestrictIndexes[4].bstrVal = SysAllocString(pTableID->uName.pwszName);
	}



	if (!CHECK(hr = m_pIDBSchemaRowset->GetRowset(	
			NULL, 								// aggregation
			DBSCHEMA_INDEXES,					// REFGUID
			cRest,	 							// count of restrictions (1:types)
			rgRestrictIndexes,					// list of restrictions
			IID_IRowset,						// REFFID
			0,									// count of properties
			NULL,								// range of properties
			(IUnknown**)&pIRowset				// returned result set
		),S_OK))
			goto CLEANUP;

	// Only do this once, if there is a rowset then 
	// there is an index on this table in the data source
	hr=pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &rghRows);
	if (pfExists)
		*pfExists = (1 == cRowsObtained);	
	CHECK(hr=pIRowset->ReleaseRows(cRowsObtained,rghRows,NULL,NULL,NULL),S_OK);

	if(FAILED(hr))
			goto CLEANUP;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(rghRows);

	for(index=0;index<cRest;index++)
		GCHECK(VariantClear(&(rgRestrictIndexes[index])),S_OK);
	
	return hr;
} //DoesIndexExistInIndexSchemaRowset


//---------------------------------------------------------------------------
// CAlterIndex::DoesIndexExistRowsetIndex
//
// DoesIndexExist	|
// If this index is on this table return true. If function runs correctly
// but doesn't find the table name, function will return S_OK, but fExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	The index is sought in 
// index schema rowset.
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CAlterIndex::DoesIndexExistRowsetIndex(	
	DBID		*pTableID,				// @parm [IN]	Table ID										  
	DBID		*pIndexID,				// @parm [IN]	Index ID
	BOOL 		*pfExist					// @parm [OUT] TRUE if index exists
)
{
	HRESULT			hr;
	IRowsetIndex	*pRowsetIndex=NULL;

	hr = m_pIOpenRowset->OpenRowset(NULL, pTableID, pIndexID, IID_IRowsetIndex, 0, NULL, 
			(IUnknown**)&pRowsetIndex);
	if (pfExist)
		*pfExist = (S_OK == hr)? TRUE: FALSE;
	SAFE_RELEASE(pRowsetIndex);
	return S_OK;
} //DoesIndexExistRowsetIndex



//---------------------------------------------------------------------------
// CAlterIndex::DoesIndexExist  	
//
// DoesIndexExist	|
// If this index is on this table return true If function runs correctly
// but doesn't find the table name, function will return S_OK, but fExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CAlterIndex::DoesIndexExist(	
	DBID		*pTableID,				// @parm [IN]	Table ID										  
	DBID		*pIndexID,				// @parm [IN]	Index ID
	BOOL		*pfExists				// @parm [OUT]	TRUE if index exists
)
{
	BOOL	fIRowsetIndex = FALSE;
	HRESULT	hr = E_FAIL;

	if (!pfExists)
		return E_INVALIDARG;

	//Cannot do any verification if none of the two exist.
	if(!m_ulIRowsetIndex && !m_pIDBSchemaRowset)
		return S_FALSE;

	if (m_ulIRowsetIndex)
	{
		TESTC_(DoesIndexExistRowsetIndex(pTableID, pIndexID, pfExists), S_OK)
		fIRowsetIndex = *pfExists;
	}

	if (m_pIDBSchemaRowset)
	{
		TESTC_(DoesIndexExistInIndexSchemaRowset(pTableID, pIndexID, pfExists), S_OK)
		if(m_ulIRowsetIndex)
			COMPARE(*pfExists, fIRowsetIndex);
	}

	hr = S_OK;

CLEANUP:
	return hr;
} //DoesIndexExist

BOOL CAlterIndex::GetQualifierNames(
	IUnknown * pSessionIUnknown,// [in]		IUnknown off session object
	LPWSTR	pwszTableName,		// [in]		the name of the table
	LPWSTR	*ppwszCatalogName,	// [out]	catalog name
	LPWSTR	*ppwszSchemaName	// [out]	schema name
)
{
    IRowset		*pIRowset = NULL;
	IDBSchemaRowset * pIDBSchmr = NULL;
	ULONG	cSchemas = 0;
	GUID * pSchemas = NULL;
	ULONG * pRestrictionSupport = NULL;
	ULONG iSchema, iRestrict;
	LONG_PTR rgColsToBind[TABLES_COLS] = {1, 2, 3};
	BOOL fTablesRowset = FALSE;
	BOOL fTableRestrict = FALSE;
	VARIANT rgRestrictions[RESTRICTION_COUNT];
	BYTE * pData = NULL;
	HACCESSOR hAccessor;
	DBBINDING * pBinding = NULL;
	DBCOUNTITEM cBinding = 0;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM cRows = 0;
	HROW	*	pRow = NULL;
	BOOL	fTableFound = FALSE;
	ULONG_PTR ulIdentCase = DBPROPVAL_IC_UPPER;
	ULONG_PTR ulQuotedIdentCase = DBPROPVAL_IC_UPPER;

	// Check args
	TESTC(NULL != pwszTableName);
	TESTC(NULL != ppwszCatalogName);
	TESTC(NULL != ppwszSchemaName);

	// See if we can get an IDBSchemaRowset interface
	if (!VerifyInterface(pSessionIUnknown, IID_IDBSchemaRowset, SESSION_INTERFACE,
		(IUnknown **)&pIDBSchmr))
	{
		// Then the best we can do is get the catalog name from DBPROP_CURRENTCATALOG
		GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, pSessionIUnknown, ppwszCatalogName);
		goto CLEANUP;
	}

	// we've got to either remove
	// the quotes or convert to proper case if DBPROP_IDENTIFIERCASE happens to 
	// be different than DBPROP_QUOTEDIDENTIFIERCASE and the QUOTEDIDENTIFIERCASE
	// is DBPROPVAL_IC_SENSITIVE.
	GetProperty(DBPROP_IDENTIFIERCASE, DBPROPSET_DATASOURCEINFO, 
							m_pThisTestModule->m_pIUnknown,&ulIdentCase);

	GetProperty(DBPROP_QUOTEDIDENTIFIERCASE, DBPROPSET_DATASOURCEINFO, 
						m_pThisTestModule->m_pIUnknown,&ulQuotedIdentCase);

	// Need to convert identifier to upper case or lower case
	if (ulIdentCase == DBPROPVAL_IC_UPPER)
		_wcsupr(pwszTableName);
	else if (ulIdentCase == DBPROPVAL_IC_LOWER)
		_wcslwr(pwszTableName);

	// Find out of the TABLES rowset is supported
	TESTC_(pIDBSchmr->GetSchemas(&cSchemas, &pSchemas, &pRestrictionSupport), S_OK);

	for (iSchema = 0; iSchema < cSchemas; iSchema++)
	{
		if (pSchemas[iSchema] == DBSCHEMA_TABLES)
		{
			fTablesRowset = TRUE;

			// See if the tablename restriction is supported
			if (pRestrictionSupport[iSchema] & TABLE_RESTRICT)
				fTableRestrict = TRUE;

			break;
		}
	}

	if (!fTablesRowset)
	{
		// Then the best we can do is get the catalog name from DBPROP_CURRENTCATALOG
		GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, pSessionIUnknown, ppwszCatalogName);
		goto CLEANUP;
	}

	// Initialize restrictions
	for (iRestrict = 0; iRestrict < RESTRICTION_COUNT; iRestrict++)
		VariantInit(&rgRestrictions[iRestrict]);

	// Set the table restriction, if supported
	if (fTableRestrict)
	{
		V_VT(&rgRestrictions[TABLE_RESTRICT-1]) = VT_BSTR;
		V_BSTR(&rgRestrictions[TABLE_RESTRICT-1]) = SysAllocString(pwszTableName);
	}

	//Obtain Schema TABLES Rowset
	TESTC_(pIDBSchmr->GetRowset(NULL, DBSCHEMA_TABLES, RESTRICTION_COUNT, rgRestrictions,
		IID_IRowset, 0, NULL, (IUnknown **)&pIRowset), S_OK);


	TESTC_(GetAccessorAndBindings(
		pIRowset,									// @parm [IN]  Rowset or command to create Accessor for
		DBACCESSOR_ROWDATA,							// @parm [IN]  Properties of the Accessor
		&hAccessor,									// @parm [OUT] Accessor created
		&pBinding,									// @parm [OUT] Array of DBBINDINGS
		&cBinding,									// @parm [OUT] Count of bindings
		&cbRowSize,									// @parm [OUT] length of a row	
		DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,	// @parm [IN]  Types of binding to do (Value, Status, and/or Length)	
		USE_COLS_TO_BIND_ARRAY,						// @parm [IN]  Which columns will be used in the bindings
		FORWARD,									// @parm [IN]  Order to bind columns in accessor												
		NO_COLS_BY_REF,								// @parm [IN]  Which columns to bind by reference (fixed, variable, all or none)
		NULL,										// @parm [OUT] Array of DBCOLUMNINFO
		NULL,										// @parm [OUT] Count of Columns, also count of ColInfo elements
		NULL,										// @parm [OUT] ppStringsBuffer				
		DBTYPE_EMPTY,								// @parm [IN] Modifier to be OR'd with each binding type.
		TABLES_COLS,								// @parm [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY
		rgColsToBind								// @parm [IN]  Used only if eColsToBind = USE_COLS_TO_BIND_ARRAY												 
	), S_OK);

	// Allocate a buffer to hold the results
	SAFE_ALLOC(pData, BYTE, cbRowSize);
	
	//Try to find the specified row with this table name
	while(S_OK == pIRowset->GetNextRows(NULL, 0, 1, &cRows, &pRow))
	{
		DATA * pCol = (DATA *)(pData + pBinding[2].obStatus);
		
		TESTC(cRows == 1);

		//GetData for this row
		TESTC_(pIRowset->GetData(*pRow, hAccessor, pData),S_OK);

		// If the table name isn't NULL or an error
		if(pCol->sStatus ==DBSTATUS_S_OK)
		{
			// See if it matches
			if(!wcscmp(pwszTableName, (LPWSTR)pCol->bValue))
			{
				DATA * pCatalogName = (DATA *)(pData + pBinding[0].obStatus);
				DATA * pSchemaName = (DATA *)(pData + pBinding[1].obStatus);

				fTableFound = TRUE;

				//Catalog Name
				if(pCatalogName->sStatus ==DBSTATUS_S_OK)
					*ppwszCatalogName = wcsDuplicate((LPWSTR)pCatalogName->bValue);
				//Schema Name
				if(pSchemaName->sStatus ==DBSTATUS_S_OK)
					*ppwszSchemaName = wcsDuplicate((LPWSTR)pSchemaName->bValue);

				break;
			}
		}

		TESTC_(pIRowset->ReleaseRows(cRows, pRow, NULL, NULL, NULL), S_OK);
	}

	COMPARE(fTableFound, TRUE);

CLEANUP:

	CHECK(pIRowset->ReleaseRows(cRows, pRow, NULL, NULL, NULL), S_OK);

	if (fTableRestrict)
		VariantClear(&rgRestrictions[TABLE_RESTRICT-1]);

	PROVIDER_FREE(pSchemas);
	PROVIDER_FREE(pRestrictionSupport);
	PROVIDER_FREE(pRow);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pBinding);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBSchmr);

	return fTableFound;
} 

HRESULT CAlterIndex::CleanUpIndex(
	DBID		*pTableID,
	DBID		**ppIndexID,
	DBINDEXCOLUMNDESC * pIndexColumnDescs,
	ULONG *		pcPropertySets,
	DBPROPSET ** ppPropertySets
)
{
	HRESULT hr = S_OK;
		
	// Drop the index we created
	if (pTableID && ppIndexID && *ppIndexID)
		CHECK(hr = m_pIIndexDef->DropIndex(pTableID, *ppIndexID), S_OK); 

	if (ppIndexID)
	{
		ReleaseDBID(*ppIndexID);
		*ppIndexID= NULL;
	}

	// Free props
	if (pcPropertySets && ppPropertySets)
		FreeProperties(pcPropertySets, ppPropertySets);

	// Note the DBIDs in this array just point to the appropriate column
	// in CCol and so we shouldn't free the DBIDs.
	SAFE_FREE(pIndexColumnDescs);

	return hr;
}

void CAlterIndex::FreeIndexColumnDesc(DBORDINAL * pcColDesc, DBINDEXCOLUMNDESC ** ppIndexColumnDesc, BOOL fFreeBuf)
{
	ULONG iColDesc;

	ASSERT(pcColDesc && ppIndexColumnDesc);

	if (*ppIndexColumnDesc)
	{
		for (iColDesc = 0; iColDesc < *pcColDesc; iColDesc++)
		{
			ReleaseDBID((*ppIndexColumnDesc)[iColDesc].pColumnID);
		}

		if (fFreeBuf)
			SAFE_FREE(*ppIndexColumnDesc);
	}
	*pcColDesc = 0;
}

//*-----------------------------------------------------------------------
// Test Case Section
//*-----------------------------------------------------------------------

// {{ TCW_TEST_CASE_MAP(TCAI_SingCol)
//*-----------------------------------------------------------------------
// @class Test case for AlterIndex.
//
class TCAI_SingCol : public CAlterIndex { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAI_SingCol,CAlterIndex);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Verify Session object
	int Variation_1();
	// @cmember General - Change index name to names of various lengths.
	int Variation_2();
	// @cmember General - Change index name to a name of maximum length.
	int Variation_3();
	// @cmember General - Make names from wierd (valid) chars
	int Variation_4();
	// @cmember General - Change index name to DBKIND_GUID_NAME
	int Variation_5();
	// @cmember General - Change index name to DBKIND_GUID_PROPID
	int Variation_6();
	// @cmember General - Change index name to DBKIND_PGUID_NAME
	int Variation_7();
	// @cmember General - Change index name to DBKIND_PGUID_PROPID
	int Variation_8();
	// @cmember General - Change index name to DBKIND_PROPID
	int Variation_9();
	// @cmember General - Change index name to DBKIND_GUID
	int Variation_10();
	// @cmember General - pNewIndexID same as pIndexID (and no props)
	int Variation_11();
	// @cmember General - Change index name and one prop
	int Variation_12();
	// @cmember General - Change index name and several props
	int Variation_13();
	// @cmember General - Change index name and props (OPTIONAL)
	int Variation_14();
	// @cmember General - Change index name and props (SETIFCHEAP)
	int Variation_15();
	// @cmember General - try to set some index and some non-index props.
	int Variation_16();
	// @cmember General - pNewIndexID is NULL, set some props
	int Variation_17();
	// @cmember General - pNewIndexID is same as pIndexID, set some props
	int Variation_18();
	// @cmember General - pNewIndexID is NULL, no props
	int Variation_19();
	// @cmember General - AlterIndex for all data types that can have indexes
	int Variation_20();
	// @cmember General - Create several indexes and alter each
	int Variation_21();
	// @cmember General - AlterIndex on multiple threads, same index
	int Variation_22();
	// @cmember General - AlterIndex on multiple threads, different indexes
	int Variation_23();
	// @cmember General - AlterIndex on multiple threads, two tables to same index name
	int Variation_24();
	// @cmember General - AlterIndex on multiple threads, name on one, props on another
	int Variation_25();
	// @cmember General - AlterIndex on index created by command
	int Variation_26();
	// @cmember General - AlterIndex with quoted name
	int Variation_27();
	// @cmember General - AlterIndex with qualified and quoted name
	int Variation_28();
	// @cmember General - AlterIndex to same name as PrimaryKey constraint
	int Variation_29();
	// @cmember General - AlterIndex on temp table
	int Variation_30();
	// @cmember General - pIndexID is DBKIND_GUID_NAME
	int Variation_31();
	// @cmember General - pIndexID is DBKIND_GUID_PROPID
	int Variation_32();
	// @cmember General - pIndexID is DBKIND_PGUID_NAME
	int Variation_33();
	// @cmember General - pIndexID is DBKIND_PGUID_PROPID
	int Variation_34();
	// @cmember General - pIndexID is DBKIND_PROPID
	int Variation_35();
	// @cmember General - pIndexID is DBKIND_GUID
	int Variation_36();
	// @cmember General - AlterIndex with rowset open on different table
	int Variation_37();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCAI_SingCol)
#define THE_CLASS TCAI_SingCol
BEG_TEST_CASE(TCAI_SingCol, CAlterIndex, L"Test case for AlterIndex.")
	TEST_VARIATION(1, 		L"General - Verify Session object")
	TEST_VARIATION(2, 		L"General - Change index name to names of various lengths.")
	TEST_VARIATION(3, 		L"General - Change index name to a name of maximum length.")
	TEST_VARIATION(4, 		L"General - Make names from wierd (valid) chars")
	TEST_VARIATION(5, 		L"General - Change index name to DBKIND_GUID_NAME")
	TEST_VARIATION(6, 		L"General - Change index name to DBKIND_GUID_PROPID")
	TEST_VARIATION(7, 		L"General - Change index name to DBKIND_PGUID_NAME")
	TEST_VARIATION(8, 		L"General - Change index name to DBKIND_PGUID_PROPID")
	TEST_VARIATION(9, 		L"General - Change index name to DBKIND_PROPID")
	TEST_VARIATION(10, 		L"General - Change index name to DBKIND_GUID")
	TEST_VARIATION(11, 		L"General - pNewIndexID same as pIndexID (and no props)")
	TEST_VARIATION(12, 		L"General - Change index name and one prop")
	TEST_VARIATION(13, 		L"General - Change index name and several props")
	TEST_VARIATION(14, 		L"General - Change index name and props (OPTIONAL)")
	TEST_VARIATION(15, 		L"General - Change index name and props (SETIFCHEAP)")
	TEST_VARIATION(16, 		L"General - try to set some index and some non-index props.")
	TEST_VARIATION(17, 		L"General - pNewIndexID is NULL, set some props")
	TEST_VARIATION(18, 		L"General - pNewIndexID is same as pIndexID, set some props")
	TEST_VARIATION(19, 		L"General - pNewIndexID is NULL, no props")
	TEST_VARIATION(20, 		L"General - AlterIndex for all data types that can have indexes")
	TEST_VARIATION(21, 		L"General - Create several indexes and alter each")
	TEST_VARIATION(22, 		L"General - AlterIndex on multiple threads, same index")
	TEST_VARIATION(23, 		L"General - AlterIndex on multiple threads, different indexes")
	TEST_VARIATION(24, 		L"General - AlterIndex on multiple threads, two tables to same index name")
	TEST_VARIATION(25, 		L"General - AlterIndex on multiple threads, name on one, props on another")
	TEST_VARIATION(26, 		L"General - AlterIndex on index created by command")
	TEST_VARIATION(27, 		L"General - AlterIndex with quoted name")
	TEST_VARIATION(28, 		L"General - AlterIndex with qualified and quoted name")
	TEST_VARIATION(29, 		L"General - AlterIndex to same name as PrimaryKey constraint")
	TEST_VARIATION(30, 		L"General - AlterIndex on temp table")
	TEST_VARIATION(31, 		L"General - pIndexID is DBKIND_GUID_NAME")
	TEST_VARIATION(32, 		L"General - pIndexID is DBKIND_GUID_PROPID")
	TEST_VARIATION(33, 		L"General - pIndexID is DBKIND_PGUID_NAME")
	TEST_VARIATION(34, 		L"General - pIndexID is DBKIND_PGUID_PROPID")
	TEST_VARIATION(35, 		L"General - pIndexID is DBKIND_PROPID")
	TEST_VARIATION(36, 		L"General - pIndexID is DBKIND_GUID")
	TEST_VARIATION(37, 		L"General - AlterIndex with rowset open on different table")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCBoundary_SingCol)
//*-----------------------------------------------------------------------
// @class Boundary cases
//
class TCBoundary_SingCol : public CAlterIndex { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBoundary_SingCol,CAlterIndex);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG - pTableID = NULL
	int Variation_1();
	// @cmember E_INVALIDARG - pIndexID = NULL
	int Variation_2();
	// @cmember E_INVALIDARG - pTableID = NULL and pIndexID = NULL
	int Variation_3();
	// @cmember E_INVALIDARG - cPropSets > 0 and rgPropSets NULL
	int Variation_4();
	// @cmember E_INVALIDARG - cProperties != 0 and rgProperties NULL
	int Variation_5();
	// @cmember DB_E_NOTABLE - *pTableID == DB_NULLID
	int Variation_6();
	// @cmember DB_E_NOTABLE - pTableID->uName is NULL
	int Variation_7();
	// @cmember DB_E_NOTABLE - pTableID->uName is empty string
	int Variation_8();
	// @cmember DB_E_NOTABLE - Table name invalid
	int Variation_9();
	// @cmember DB_E_NOTABLE - Table name exceeds max length
	int Variation_10();
	// @cmember DB_E_NOTABLE - Table does not exist
	int Variation_11();
	// @cmember DB_E_NOINDEX - Index does not exist
	int Variation_12();
	// @cmember DB_E_NOINDEX - *pIndexID == DB_NULLID
	int Variation_13();
	// @cmember DB_E_NOINDEX - pIndexID->uName is NULL
	int Variation_14();
	// @cmember DB_E_NOINDEX - pIndexID->uName is empty string
	int Variation_15();
	// @cmember DB_E_NOINDEX - pIndexID name is invalid
	int Variation_16();
	// @cmember DB_E_NOINDEX - pIndexID name exceeds max name length
	int Variation_17();
	// @cmember DB_E_DUPLICATEINDEXID - The new index already exists
	int Variation_18();
	// @cmember DBSEC_E_PERMISSIONDENIED - Insufficient permissions to alter index
	int Variation_19();
	// @cmember DB_E_BADINDEXID - pNewIndexID == DB_NULLID
	int Variation_20();
	// @cmember DB_E_BADINDEXID - pNewIndexID->uName is NULL
	int Variation_21();
	// @cmember DB_E_BADINDEXID - pNewIndexID->uName is empty string
	int Variation_22();
	// @cmember DB_E_BADINDEXID - Specify an invalid new index ID
	int Variation_23();
	// @cmember DB_E_BADINDEXID - pNewIndexID name exceeds max name length
	int Variation_24();
	// @cmember DB_E_INDEXINUSE - AlterIndex with index in use
	int Variation_25();
	// @cmember DB_E_TABLEINUSE - AlterIndex with table in use
	int Variation_26();
	// @cmember S_OK - cPropSets == 0, rgPropSets is ignored
	int Variation_27();
	// @cmember DB_E_NOINDEX - AlterIndex on a dropped index
	int Variation_28();
	// @cmember DB_E_NOTABLE - Pass procedure name as existing table name
	int Variation_29();
	// @cmember DB_E_NOINDEX - Pass procedure name as existing index name
	int Variation_30();
	// @cmember DB_E_NOINDEX - Pass valid index from different table as existing index name
	int Variation_31();
	// @cmember DB_E_ERRORSOCCURRED - Alter second index on table to be duplicate primary key
	int Variation_32();
	// @cmember DB_S_ERRORSOCCURRED - Alter second index on table to be duplicate primary key
	int Variation_33();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBoundary_SingCol)
#define THE_CLASS TCBoundary_SingCol
BEG_TEST_CASE(TCBoundary_SingCol, CAlterIndex, L"Boundary cases")
	TEST_VARIATION(1, 		L"E_INVALIDARG - pTableID = NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG - pIndexID = NULL")
	TEST_VARIATION(3, 		L"E_INVALIDARG - pTableID = NULL and pIndexID = NULL")
	TEST_VARIATION(4, 		L"E_INVALIDARG - cPropSets > 0 and rgPropSets NULL")
	TEST_VARIATION(5, 		L"E_INVALIDARG - cProperties != 0 and rgProperties NULL")
	TEST_VARIATION(6, 		L"DB_E_NOTABLE - *pTableID == DB_NULLID")
	TEST_VARIATION(7, 		L"DB_E_NOTABLE - pTableID->uName is NULL")
	TEST_VARIATION(8, 		L"DB_E_NOTABLE - pTableID->uName is empty string")
	TEST_VARIATION(9, 		L"DB_E_NOTABLE - Table name invalid")
	TEST_VARIATION(10, 		L"DB_E_NOTABLE - Table name exceeds max length")
	TEST_VARIATION(11, 		L"DB_E_NOTABLE - Table does not exist")
	TEST_VARIATION(12, 		L"DB_E_NOINDEX - Index does not exist")
	TEST_VARIATION(13, 		L"DB_E_NOINDEX - *pIndexID == DB_NULLID")
	TEST_VARIATION(14, 		L"DB_E_NOINDEX - pIndexID->uName is NULL")
	TEST_VARIATION(15, 		L"DB_E_NOINDEX - pIndexID->uName is empty string")
	TEST_VARIATION(16, 		L"DB_E_NOINDEX - pIndexID name is invalid")
	TEST_VARIATION(17, 		L"DB_E_NOINDEX - pIndexID name exceeds max name length")
	TEST_VARIATION(18, 		L"DB_E_DUPLICATEINDEXID - The new index already exists")
	TEST_VARIATION(19, 		L"DBSEC_E_PERMISSIONDENIED - Insufficient permissions to alter index")
	TEST_VARIATION(20, 		L"DB_E_BADINDEXID - pNewIndexID == DB_NULLID")
	TEST_VARIATION(21, 		L"DB_E_BADINDEXID - pNewIndexID->uName is NULL")
	TEST_VARIATION(22, 		L"DB_E_BADINDEXID - pNewIndexID->uName is empty string")
	TEST_VARIATION(23, 		L"DB_E_BADINDEXID - Specify an invalid new index ID")
	TEST_VARIATION(24, 		L"DB_E_BADINDEXID - pNewIndexID name exceeds max name length")
	TEST_VARIATION(25, 		L"DB_E_INDEXINUSE - AlterIndex with index in use")
	TEST_VARIATION(26, 		L"DB_E_TABLEINUSE - AlterIndex with table in use")
	TEST_VARIATION(27, 		L"S_OK - cPropSets == 0, rgPropSets is ignored")
	TEST_VARIATION(28, 		L"DB_E_NOINDEX - AlterIndex on a dropped index")
	TEST_VARIATION(29, 		L"DB_E_NOTABLE - Pass procedure name as existing table name")
	TEST_VARIATION(30, 		L"DB_E_NOINDEX - Pass procedure name as existing index name")
	TEST_VARIATION(31, 		L"DB_E_NOINDEX - Pass valid index from different table as existing index name")
	TEST_VARIATION(32, 		L"DB_E_ERRORSOCCURRED - Alter second index on table to be duplicate primary key")
	TEST_VARIATION(33, 		L"DB_S_ERRORSOCCURRED - Alter second index on table to be duplicate primary key")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCProp_SingCol)
//*-----------------------------------------------------------------------
// @class Property related tests
//
class TCProp_SingCol : public CAlterIndex { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCProp_SingCol,CAlterIndex);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember DBPROP_INDEX_AUTOUPDATE
	int Variation_1();
	// @cmember DBPROP_INDEX_CLUSTERED
	int Variation_2();
	// @cmember DBPROP_INDEX_NULLS
	int Variation_3();
	// @cmember DBPROP_INDEX_PRIMARYKEY
	int Variation_4();
	// @cmember DBPROP_INDEX_SORTBOOKMARKS
	int Variation_5();
	// @cmember DBPROP_INDEX_TEMPINDEX
	int Variation_6();
	// @cmember DBPROP_INDEX_TYPE
	int Variation_7();
	// @cmember DBPROP_INDEX_UNIQUE
	int Variation_8();
	// @cmember DBPROP_INDEX_FILLFACTOR
	int Variation_9();
	// @cmember DBPROP_INDEX_INITIALSIZE
	int Variation_10();
	// @cmember DBPROP_INDEX_NULLCOLLATION
	int Variation_11();
	// @cmember DBPROP_INDEX_SORTBOOKMARKS and DBPROP_INDEX_UNIQUE
	int Variation_12();
	// @cmember DBPROP_INDEX_UNIQUE and DBPROP_INDEX_PRIMARYKEY
	int Variation_13();
	// @cmember DBPROP_INDEX_UNIQUE and DBPROP_INDEX_NULLS
	int Variation_14();
	// @cmember DBPROP_INDEX_NULLS and DBPROP_INDEX_PRIMARYKEY
	int Variation_15();
	// @cmember DB_E_ERRORSOCCURRED - Non index property with DBPROP_REQUIRED
	int Variation_16();
	// @cmember DB_S_ERRORSOCCURRED - Non index property with DBPROP_SETIFCHEAP
	int Variation_17();
	// @cmember Invalid values for properties (unexpected type)
	int Variation_18();
	// @cmember Specify a property twice
	int Variation_19();
	// @cmember Set invalid value for prop (unexpected value)
	int Variation_20();
	// @cmember Set colid for prop
	int Variation_21();
	// @cmember Set non-index prop in DBPROPSET_INDEX
	int Variation_22();
	// @cmember Set array of propsets, one with 0 properties
	int Variation_23();
	// @cmember Set props with static property sets and properties
	int Variation_24();
	// @cmember Set invalid properties REQUIRED and change name, verify new name doesn't exist
	int Variation_25();
	// @cmember Set invalid properties OPTIONAL and change name, verify new name does exist
	int Variation_26();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCProp_SingCol)
#define THE_CLASS TCProp_SingCol
BEG_TEST_CASE(TCProp_SingCol, CAlterIndex, L"Property related tests")
	TEST_VARIATION(1, 		L"DBPROP_INDEX_AUTOUPDATE")
	TEST_VARIATION(2, 		L"DBPROP_INDEX_CLUSTERED")
	TEST_VARIATION(3, 		L"DBPROP_INDEX_NULLS")
	TEST_VARIATION(4, 		L"DBPROP_INDEX_PRIMARYKEY")
	TEST_VARIATION(5, 		L"DBPROP_INDEX_SORTBOOKMARKS")
	TEST_VARIATION(6, 		L"DBPROP_INDEX_TEMPINDEX")
	TEST_VARIATION(7, 		L"DBPROP_INDEX_TYPE")
	TEST_VARIATION(8, 		L"DBPROP_INDEX_UNIQUE")
	TEST_VARIATION(9, 		L"DBPROP_INDEX_FILLFACTOR")
	TEST_VARIATION(10, 		L"DBPROP_INDEX_INITIALSIZE")
	TEST_VARIATION(11, 		L"DBPROP_INDEX_NULLCOLLATION")
	TEST_VARIATION(12, 		L"DBPROP_INDEX_SORTBOOKMARKS and DBPROP_INDEX_UNIQUE")
	TEST_VARIATION(13, 		L"DBPROP_INDEX_UNIQUE and DBPROP_INDEX_PRIMARYKEY")
	TEST_VARIATION(14, 		L"DBPROP_INDEX_UNIQUE and DBPROP_INDEX_NULLS")
	TEST_VARIATION(15, 		L"DBPROP_INDEX_NULLS and DBPROP_INDEX_PRIMARYKEY")
	TEST_VARIATION(16, 		L"DB_E_ERRORSOCCURRED - Non index property with DBPROP_REQUIRED")
	TEST_VARIATION(17, 		L"DB_S_ERRORSOCCURRED - Non index property with DBPROP_SETIFCHEAP")
	TEST_VARIATION(18, 		L"Invalid values for properties (unexpected type)")
	TEST_VARIATION(19, 		L"Specify a property twice")
	TEST_VARIATION(20, 		L"Set invalid value for prop (unexpected value)")
	TEST_VARIATION(21, 		L"Set colid for prop")
	TEST_VARIATION(22, 		L"Set non-index prop in DBPROPSET_INDEX")
	TEST_VARIATION(23, 		L"Set array of propsets, one with 0 properties")
	TEST_VARIATION(24, 		L"Set props with static property sets and properties")
	TEST_VARIATION(25, 		L"Set invalid properties REQUIRED and change name, verify new name doesn't exist")
	TEST_VARIATION(26, 		L"Set invalid properties OPTIONAL and change name, verify new name does exist")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCTransact)
//*-----------------------------------------------------------------------
// @class Commit/abort behavior for AlterIndex
//
class TCTransact : public CAlterIndex { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	CTransaction * m_pTransact;

	ULONG_PTR m_ulSupportedTxnDDL;

	HRESULT	m_hrTxn;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCTransact,CAlterIndex);
	// }} TCW_DECLARE_FUNCS_END

	TCTransact(void);
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	int TestTxn(ETXN eTxn,BOOL fRetaining, BOOL fCreateInTxn = FALSE);

	// {{ TCW_TESTVARS()
	// @cmember Commit Retaining
	int Variation_1();
	// @cmember Commit Non Retaining
	int Variation_2();
	// @cmember Abort Retaining
	int Variation_3();
	// @cmember Abort Non Retaining
	int Variation_4();
	// @cmember Commit Retaining, create index in txn
	int Variation_5();
	// @cmember Commit Non Retaining, create index in txn
	int Variation_6();
	// @cmember Abort Retaining, create index in txn
	int Variation_7();
	// @cmember Abort Non Retaining, create index in txn
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCTransact)
#define THE_CLASS TCTransact
BEG_TEST_CASE(TCTransact, CAlterIndex, L"Commit/abort behavior for AlterIndex")
	TEST_VARIATION(1, 		L"Commit Retaining")
	TEST_VARIATION(2, 		L"Commit Non Retaining")
	TEST_VARIATION(3, 		L"Abort Retaining")
	TEST_VARIATION(4, 		L"Abort Non Retaining")
	TEST_VARIATION(5, 		L"Commit Retaining, create index in txn")
	TEST_VARIATION(6, 		L"Commit Non Retaining, create index in txn")
	TEST_VARIATION(7, 		L"Abort Retaining, create index in txn")
	TEST_VARIATION(8, 		L"Abort Non Retaining, create index in txn")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()


////////////////////////////////////////////////////////////////////////
// Copying Test Cases to make duplicate ones.
//
////////////////////////////////////////////////////////////////////////
#define COPY_TEST_CASE(theClass, baseClass)						\
	class theClass : public baseClass							\
	{															\
	public:														\
		static const WCHAR		m_wszTestCaseName[];			\
		DECLARE_TEST_CASE_FUNCS(theClass, baseClass);			\
	};															\
const WCHAR		theClass::m_wszTestCaseName[] = { L#theClass };	\


#define TEST_CASE_WITH_PARAM(iCase, theClass, param)			\
    case iCase:													\
		pCTestCase = new theClass(NULL);						\
		((theClass*)pCTestCase)->SetTestCaseParam(param);		\
		pCTestCase->SetOwningMod(iCase-1, pCThisTestModule);	\
		return pCTestCase;

//Make a copy of the test cases.

COPY_TEST_CASE(TCAI_SingColWithProp, TCAI_SingCol)

COPY_TEST_CASE(TCAI_MultCol, TCAI_SingCol)
COPY_TEST_CASE(TCBoundary_MultCol, TCBoundary_SingCol)

COPY_TEST_CASE(TCAI_MultColWithProp, TCAI_SingCol)
COPY_TEST_CASE(TCBoundary_MultColWithProp, TCBoundary_SingCol)

COPY_TEST_CASE(TCProp_MultCol, TCProp_SingCol)

//NOTE: The #ifdef block below is only for test wizard.  TestWizard has too many 
//strict rules in the parsing code and requires a 1:1 correspondence between
//testcases and the map.  What the #else section is doing is basically "reusing"
//existing testcases by just passing in a parameter which changes the behvior.
//So we make LTM think there are several cases in here with different names, but in
//reality we only have to maintain code for the unique cases. 

#if 0
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(4, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCAI_SingCol)
	TEST_CASE(2, TCBoundary_SingCol)
	TEST_CASE(3, TCProp_SingCol)
	TEST_CASE(4, TCTransact)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(10, ThisModule, gwszModuleDescrip)
	//1
	TEST_CASE(1, TCAI_SingCol)
	TEST_CASE(2, TCBoundary_SingCol)

	//2
	TEST_CASE_WITH_PARAM(3, TCAI_SingColWithProp, TC_SingleColProps)

	//3
	TEST_CASE_WITH_PARAM(4, TCAI_MultCol, TC_MultipleColsNoProps)
	TEST_CASE_WITH_PARAM(5, TCBoundary_MultCol, TC_MultipleColsNoProps)

	//4
	TEST_CASE_WITH_PARAM(6, TCAI_MultColWithProp, TC_MultipleColsProps)
	TEST_CASE_WITH_PARAM(7, TCBoundary_MultColWithProp, TC_MultipleColsProps)

	//5
	TEST_CASE_WITH_PARAM(8, TCProp_SingCol, TC_PropSingCol)
	TEST_CASE_WITH_PARAM(9, TCProp_MultCol, TC_PropMultCol)

	TEST_CASE(10, TCTransact)

END_TEST_MODULE()
#endif



// {{ TCW_TC_PROTOTYPE(TCAI_SingCol)
//*-----------------------------------------------------------------------
//| Test Case:		TCAI_SingCol - Test case for AlterIndex.
//| Created:  	7/30/1999
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAI_SingCol::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	return CAlterIndex::Init();
	// }}
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Verify Session object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_1()
{ 
	TBEGIN

	TESTC(DefaultObjectTesting(m_pIAlterIndex, SESSION_INTERFACE))

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name to names of various lengths.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_2()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, L"X");

	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

	Refresh_m_pIndexID(hr, dbidNew);
	dbidNew.uName.pwszName = BuildValidName(2, L"xy");
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	dbidNew.uName.pwszName = BuildValidName(15, L"a1b2c3");
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	dbidNew.uName.pwszName = BuildValidName(16, L"IAlterIndex");
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

	if(m_cMaxIndexName<63)
		goto CLEANUP;

	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	dbidNew.uName.pwszName = BuildValidName(63, L"TheQuickBrownFoxJumpedOverTheLazyDog");
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

	if(m_cMaxIndexName<64)
		goto CLEANUP;

	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	dbidNew.uName.pwszName = BuildValidName(64, L"ThisIsATestStringXYZ");
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

	if(m_cMaxIndexName<65)
		goto CLEANUP;

	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	dbidNew.uName.pwszName = BuildValidName(65, L"Howareyou1234567890");
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

	if(m_cMaxIndexName<127)
		goto CLEANUP;

	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	dbidNew.uName.pwszName = BuildValidName(127, L"TheQuickBrownFoxJumpedOverTheLazyDog1234567890");
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

	if(m_cMaxIndexName<128)
		goto CLEANUP;

	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	dbidNew.uName.pwszName = BuildValidName(128, L"TheQuickBrownFoxJumpedOverTheLazyDog1234567890");
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

	if(m_cMaxIndexName<129)
		goto CLEANUP;

	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	dbidNew.uName.pwszName = BuildValidName(129, L"TheQuickBrownFox0123456789JumpedOverTheLazyDog");
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name to a name of maximum length.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_3()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, BuildValidName(m_cMaxIndexName, L"XYZ"));

	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_FREE(dbidNew.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - Make names from wierd (valid) chars
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_4()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME, wcsDuplicate(m_pIndexID->uName.pwszName));
	size_t		iChar;
	HRESULT		hr = E_FAIL;
	size_t		cchName = wcslen(m_pIndexID->uName.pwszName);
	WCHAR		wszValidChar[2] = L"A";
	WCHAR		*pwszValidChars = L"`1234567890-=~!@#$%^&*()_+[]\\;',./{}|:\"<>?";
	size_t		cchValid = wcslen(pwszValidChars);

	// There must be at least one character in the existing index
	TESTC(cchName > 0);

	// Replace the last character in the valid name with wierd valid ones
	for (iChar = 0; iChar < cchValid; iChar++)
	{
		// Make sure this character doesn't appear in the invalid list
		if (wcschr(m_pwszInvalidIndexChars, pwszValidChars[iChar]))
			continue;

		dbidNew.uName.pwszName[cchName-1] = pwszValidChars[iChar];

		if (!CHECK(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK))
		{
			wszValidChar[0] = pwszValidChars[iChar];
			odtLog << L"Valid character # " << iChar << " '" << wszValidChar << L"' was invalid.\n\n";
		}

		Refresh_m_pIndexID(hr, dbidNew);

	}

	// Reset back to the valid name
	wcscpy(dbidNew.uName.pwszName, m_pIndexID->uName.pwszName);

	// Now try as starting characters

	// Replace the first character in the valid name with invalid ones
	for (iChar = 0; iChar < cchValid; iChar++)
	{
		// Make sure this character doesn't appear in the invalid list
		if (wcschr(m_pwszInvalidIndexStartingChars, pwszValidChars[iChar]))
			continue;

		dbidNew.uName.pwszName[0] = pwszValidChars[iChar];

		if (!CHECK(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK))
		{
			wszValidChar[0] = pwszValidChars[iChar];
			odtLog << L"Valid starting character # " << iChar << " '" << wszValidChar << L"' was invalid.\n\n";
		}

		Refresh_m_pIndexID(hr, dbidNew);

	}

CLEANUP:
	
	SAFE_FREE(dbidNew.uName.pwszName);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name to DBKIND_GUID_NAME
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_5()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_GUID_NAME, L"abc123");

	dbidNew.uGuid.guid = DBGUID_DSO;
	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_E_BADINDEXID)

	if(DB_E_BADINDEXID == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_GUID_NAME are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name to DBKIND_GUID_PROPID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_6()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_GUID_PROPID, NULL);

	dbidNew.uName.ulPropid = 666;
	dbidNew.uGuid.guid = DBGUID_DSO;
	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_E_BADINDEXID)

	if(DB_E_BADINDEXID == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_GUID_PROPID are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name to DBKIND_PGUID_NAME
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_7()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_PGUID_NAME, L"abc123");
	GUID		guid = DBGUID_ROW;

	dbidNew.uGuid.pguid = &guid;
	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_E_BADINDEXID)

	if(DB_E_BADINDEXID == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_PGUID_NAME are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name to DBKIND_PGUID_PROPID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_8()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_PGUID_PROPID, NULL);
	GUID		guid = DBGUID_ROW;

	dbidNew.uName.ulPropid = 666;
	dbidNew.uGuid.pguid = &guid;
	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_E_BADINDEXID)

	if(DB_E_BADINDEXID == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_PGUID_PROPID are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name to DBKIND_PROPID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_9()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_PROPID, NULL);

	dbidNew.uName.ulPropid = 666;
	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_E_BADINDEXID)

	if(DB_E_BADINDEXID == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_PROPID are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name to DBKIND_GUID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_10()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_GUID, NULL);

	dbidNew.uGuid.guid = DBGUID_DSO;
	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_E_BADINDEXID)

	if(DB_E_BADINDEXID == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_GUID are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc General - pNewIndexID same as pIndexID (and no props)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_11()
{ 
	TBEGIN
	HRESULT		hr;
	DBID		dbidNew = DB_NULLID;

	DuplicateDBID(*m_pIndexID, &dbidNew);
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);

	ReleaseDBID(&dbidNew, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name and one prop
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_12()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, L"JamesBond007");

	//Set one Index prop (optional)
	SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);

	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_S_ERRORSOCCURRED)

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name and several props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_13()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, L"SupermanSSS");

	//Set 2 props as optional. Also set a 3rd prop if it is
	//settable.
	SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)50, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetSettableProperty(DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets); //VARIANT_TRUE, REQUIRED.

	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_S_ERRORSOCCURRED)

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name and props (OPTIONAL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_14()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, L"LoisLaneYYY");

	//Set 2 props as optional. Also set a 3rd prop if it is settable.
	//Set a non-index property optional.
	SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)50, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetSettableProperty(DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets); //VARIANT_TRUE, REQUIRED.
	SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);

	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), DB_S_ERRORSOCCURRED)

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc General - Change index name and props (SETIFCHEAP)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_15()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, L"LoisLaneZZZ");

	//Set 2 props as optional. Also set a 3rd prop if it is settable.
	//Set a non-index property SETIFCHEAP.

	// Note that DBPROPOPTIONS_OPTIONAL and DBPROPOPTIONS_SETIFCHEAP are numerically identical
	TESTC(DBPROPOPTIONS_OPTIONAL == DBPROPOPTIONS_SETIFCHEAP);
	SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)50, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetSettableProperty(DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets); //VARIANT_TRUE, REQUIRED.
	SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_SETIFCHEAP);

	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), DB_S_ERRORSOCCURRED)

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc General - try to set some index and some non-index props.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_16()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, L"SupermanSSS");

	//Set 2 props as optional. Also set a 3rd prop if it is settable.
	//Set a non-index property.
	SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetSettableProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)50, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetSettableProperty(DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets); //VARIANT_TRUE, REQUIRED.
	SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets); //VARIANT_TRUE, REQUIRED.

	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), DB_E_ERRORSOCCURRED)

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc General - pNewIndexID is NULL, set some props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_17()
{ 
	TBEGIN
	HRESULT		hr;

	//Set 2 props as optional. Also set a 3rd prop if it is
	//settable.
	SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetSettableProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)50, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetSettableProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_IN_DISALLOWNULL, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetSettableProperty(DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets); //VARIANT_TRUE, REQUIRED.

	TESTC_(hr = AlterIndex(m_pIndexID, NULL), S_OK)

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc General - pNewIndexID is same as pIndexID, set some props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_18()
{ 
	TBEGIN
	HRESULT		hr;

	//Set 2 props as optional. Also set a 3rd prop if it is settable.
	SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)50, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	SetSettableProperty(DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets); //VARIANT_TRUE, REQUIRED.

	TESTC_(hr = AlterIndex(m_pIndexID, m_pIndexID), S_OK)

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc General - pNewIndexID is NULL, no props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_19()
{ 
	TBEGIN
	HRESULT		hr;

	TESTC_(hr = AlterIndex(m_pIndexID, NULL), S_OK)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex for all data types that can have indexes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_20()
{ 
	TBEGIN
	HRESULT				hr, hrCreateIndex;
	CDBID				dbidNew(DBKIND_NAME, L"SingCol");
	DBID				*pIndexID = NULL;
	DBORDINAL			cCols = 1;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBPROPSET*			prgPropSets = NULL;
	DBID				*pIndexWithProp = NULL;
	HRESULT				hrExpected = E_FAIL;


	// TODO: Create unique index name?

	cCols = m_cIndexColumnDesc;

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, cCols);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*cCols));

	// There should be at least one column that can obtain an index with no props
	TESTC_(hrCreateIndex = CreateIndex(&(m_pTable->GetTableID()), NULL, cCols,
		pIndexColumnDesc, 0, NULL, &pIndexID), S_OK);

	while(hrCreateIndex == S_OK)
	{
		// Find out if the property requested is valid for this index
		// by attempting to create with the prop.
		hrExpected = m_pIIndexDef->CreateIndex(&(m_pTable->GetTableID()), NULL, cCols,
			pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexWithProp);

		// We should get S_OK or DB_E_ERRORSOCCURRED for required prop
		if (hrExpected != DB_E_ERRORSOCCURRED)
			CHECK(hrExpected, S_OK);

		// Drop the property index
		CHECK(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), pIndexWithProp), S_OK); 
		ReleaseDBID(pIndexWithProp);
		pIndexWithProp = NULL;

		// Now alter the index to obtain a new name and the prop desired.
		hr = AlterIndex(pIndexID, &dbidNew);

		CHECK(hr, hrExpected);

		if (SUCCEEDED(hr))
		{
			// Drop the altered index
			CHECK(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), &dbidNew), S_OK); 
		}
		else
		{
			// Drop the previous index
			CHECK(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), pIndexID), S_OK); 
		}

		// Now drop both indexes again in case the provider lied about the success of
		// AlterIndex (one does).  Don't check return code in case it didn't lie.
		m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), &dbidNew);
		m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), pIndexID);

		ReleaseDBID(pIndexID);
		pIndexID = NULL;

		// Get the next possible index
		hrCreateIndex = CreateIndex(&(m_pTable->GetTableID()), NULL, cCols,
			pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID, TRUE);
	}

CLEANUP:

	CleanUpIndex(&(m_pTable->GetTableID()), &pIndexID, pIndexColumnDesc, &m_cPropSets, &m_rgPropSets);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc General - Create several indexes and alter each
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_21()
{ 
	TBEGIN
	HRESULT				hr, hrCreateIndex;
	DBID				*pIndexID = NULL;
	ULONG				iCol;
	DBORDINAL			cCols = 1;
	DBORDINAL			cTableCols = 1;
	DBORDINAL			cIndexes = 1;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBPROPSET*			prgPropSets = NULL;
	DBID				*pIndexWithProp = NULL;
	HRESULT				hrExpected = S_OK;
	DBID *				pIndexIDs = NULL;
	WCHAR				wszName[]=L"AI00000000";
	CDBID				dbidNew(DBKIND_NAME, (LPWSTR)wszName);

	// Count number of columns in the table
	cTableCols = m_pTable->CountColumnsOnTable();

	cCols = m_cIndexColumnDesc;

	// Combinations (or permutations?) of N columns taken L at at time is
	// N!/(N-L)!
	for (iCol = 0; iCol < cCols; iCol++)
		cIndexes *= cTableCols - iCol;

	// Allocate space for a DBID for each column to hold the created index id
	SAFE_ALLOC(pIndexIDs, DBID, cIndexes);

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, cCols);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*cCols));

	// Reset to 0 indexes
	cIndexes = 0;

	// There should be at least one column that can obtain an index with no props
	TESTC_PROVIDER(S_OK == (hrCreateIndex = CreateIndex(&(m_pTable->GetTableID()), NULL, cCols,
		pIndexColumnDesc, 0, NULL, &pIndexID)));

	while(hrCreateIndex == S_OK)
	{
		// Save this index id 
		DuplicateDBID(*pIndexID, &pIndexIDs[cIndexes]);

		if (m_cPropSets)
		{
			// Find out if the property requested is valid for this index
			// by attempting to create with the prop.
			hrExpected = m_pIIndexDef->CreateIndex(&(m_pTable->GetTableID()), NULL, cCols,
				pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexWithProp);

			// We should get S_OK or DB_E_ERRORSOCCURRED for required prop
			if (hrExpected == E_FAIL)
				// Some providers will return E_FAIL if too many indexes on one table,
				// usually >250, but since we don't know if this is valid we need to
				// warn.  We will arbitrarily pick a point at which to warn.
				if (cIndexes > 30)
					CHECKW(hrExpected, S_OK);
				else
					CHECK(hrExpected, S_OK);
			else if (hrExpected == DB_E_ERRORSOCCURRED)
				// It was DB_E_ERRORSOCCURRED, must have at least one prop
				COMPARE(m_cPropSets > 0, TRUE);
			else
				CHECK(hrExpected, S_OK);

			// Drop the property index
			if (SUCCEEDED(hrExpected))
			{
				CHECK(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), pIndexWithProp), S_OK); 
				ReleaseDBID(pIndexWithProp);
				pIndexWithProp = NULL;
			}
		}

		// Make a unique new name
		swprintf(dbidNew.uName.pwszName+sizeof(L"AI"), L"%u", cIndexes);

		// Now alter the index to obtain a new name and the prop desired.
		CHECK(hr = AlterIndex(pIndexID, &dbidNew), hrExpected);

		ReleaseDBID(pIndexID);
		pIndexID = NULL;

		if (SUCCEEDED(hr))
		{
			ReleaseDBID(&pIndexIDs[cIndexes], FALSE);
			DuplicateDBID(dbidNew, &pIndexIDs[cIndexes]);
		}

		// Get the next possible index with no props
		hrCreateIndex = CreateIndex(&(m_pTable->GetTableID()), NULL, cCols,
			pIndexColumnDesc, 0, NULL, &pIndexID, TRUE);

		cIndexes++;
	}

	odtLog << L"Created " << cIndexes << L" indexes.\n";

CLEANUP:

	// Drop all the indexes created
	for (ULONG iIndex = 0; iIndex < cIndexes; iIndex++)
	{
		CHECK(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), &pIndexIDs[iIndex]), S_OK); 
		ReleaseDBID(&pIndexIDs[iIndex], FALSE);
	}
	SAFE_FREE(pIndexIDs);
	if (pIndexID)
	{
		CHECK(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), pIndexID), S_OK); 
		ReleaseDBID(pIndexID);
	}
	SAFE_FREE(pIndexColumnDesc);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex on multiple threads, same index
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_22()
{ 
	// Call AlterIndex from multiple threads on the same index. One should 
	// succeed, the others should fail.

	ULONG iThrd, iWon;
	BOOL fExists = FALSE;
	HRESULT hrThreads[MAX_THREADS];
	ULONG cWon = 0;

	// We'll have a different index name for each thread.
	LPWSTR pwszIndexNameRoot = L"Idx00000";

	TBEGIN
	INIT_THREADS(MAX_THREADS);
	
	//Setup Thread Arguments
	THREADARG6 TArgs[MAX_THREADS];
	
	// Create the index names to be used
	memset(TArgs, 0, MAX_THREADS*sizeof(THREADARG6));
	for (iThrd = 0; iThrd < MAX_THREADS; iThrd++)
	{
		DBID * pIndexID = NULL;

		// Allocate a DBID buffer
		SAFE_ALLOC(pIndexID, DBID, 1);

		(*pIndexID).eKind = DBKIND_NAME;
		(*pIndexID).uName.pwszName = wcsDuplicate(pwszIndexNameRoot);
		TESTC((*pIndexID).uName.pwszName != NULL);

		// Change this name to a unique one
		_ltow(iThrd, (*pIndexID).uName.pwszName+wcslen(L"Idx"), 10);

		// Set the hresult for this thread to failure
		hrThreads[iThrd] = E_FAIL;

		TArgs[iThrd].pFunc = m_pIAlterIndex;			// IAlterIndex interface
		TArgs[iThrd].pArg1 = &(m_pTable->GetTableIDRef()); // Table to alter
		TArgs[iThrd].pArg2 = m_pIndexID;				// Index to alter
		TArgs[iThrd].pArg3 = pIndexID;					// New name
		TArgs[iThrd].pArg4 = &hrThreads[iThrd];			// Return code
		TArgs[iThrd].pArg5 = 0;							// Count of props to set
		TArgs[iThrd].pArg6 = NULL;						// Props to set

		//Create the thread
		CREATE_THREAD(iThrd, Thread_VerifyAlterIndex, &TArgs[iThrd]);
	}

	START_THREADS();
	END_THREADS();	

	// See which thread won
	for (iThrd = 0; iThrd < MAX_THREADS; iThrd++)
	{

		if (hrThreads[iThrd] == S_OK)
		{
			cWon++;			
			iWon = iThrd;
		}

		// Should succeed, or else if another thread beat it then DB_E_NOINDEX.
		TEST2C_(hrThreads[iThrd], S_OK, DB_E_NOINDEX);

	}

	// Only one thread should have won
	TESTC(cWon == 1);

	// And the appropriate index name should exist
	TESTC_(DoesIndexExist(&(m_pTable->GetTableID()), (DBID *)(TArgs[iWon].pArg3), &fExists), S_OK);
	TESTC(fExists);

	// Update the current index id
	Refresh_m_pIndexID(S_OK, *(DBID *)(TArgs[iWon].pArg3));

CLEANUP:

	// Release DBID memory
	for (iThrd=0; iThrd < MAX_THREADS; iThrd++)
		ReleaseDBID((DBID *)(TArgs[iThrd].pArg3));

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex on multiple threads, different indexes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_23()
{ 
	// Call AlterIndex from multiple threads on different indexes. All should
	// succeed.

	ULONG iThrd;
	BOOL fExists = FALSE;
	HRESULT hrThreads[MAX_THREADS];
	ULONG cWon = 0;
	ULONG				cCols = 1;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;

	// We'll have a different index name for each thread.
	LPWSTR pwszIndexNameRoot = L"Idx00000";

	TBEGIN
	INIT_THREADS(MAX_THREADS);
	
	//Setup Thread Arguments
	THREADARG6 TArgs[MAX_THREADS];

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, cCols);
	memset(pIndexColumnDesc, 0, sizeof(DBINDEXCOLUMNDESC)*cCols);

	// Create the index names to be used
	memset(TArgs, 0, MAX_THREADS*sizeof(THREADARG6));
	for (iThrd = 0; iThrd < MAX_THREADS; iThrd++)
	{
		DBID * pIndexID = NULL;
		DBID * pNewIndexID = NULL;

		TESTC_(CreateIndex(&(m_pTable->GetTableID()), NULL, cCols,
			pIndexColumnDesc, 0, NULL, &pIndexID), S_OK);

		// Allocate a DBID buffer
		SAFE_ALLOC(pNewIndexID, DBID, 1);

		(*pNewIndexID).eKind = DBKIND_NAME;
		(*pNewIndexID).uName.pwszName = wcsDuplicate(pwszIndexNameRoot);
		TESTC((*pNewIndexID).uName.pwszName != NULL);

		// Change this name to a unique one
		_ltow(iThrd, (*pNewIndexID).uName.pwszName+wcslen(L"Idx"), 10);

		// Set the hresult for this thread to failure
		hrThreads[iThrd] = E_FAIL;

		TArgs[iThrd].pFunc = m_pIAlterIndex;			// IAlterIndex interface
		TArgs[iThrd].pArg1 = &(m_pTable->GetTableIDRef()); // Table to alter
		TArgs[iThrd].pArg2 = pIndexID;					// Index to alter
		TArgs[iThrd].pArg3 = pNewIndexID;				// New name
		TArgs[iThrd].pArg4 = &hrThreads[iThrd];			// Return code
		TArgs[iThrd].pArg5 = 0;							// Count of props to set
		TArgs[iThrd].pArg6 = NULL;						// Props to set

		//Create the thread
		CREATE_THREAD(iThrd, Thread_VerifyAlterIndex, &TArgs[iThrd]);
	}

	START_THREADS();
	END_THREADS();	

	// All threads should succeed
	for (iThrd = 0; iThrd < MAX_THREADS; iThrd++)
	{
		// Should succeed, or else if another thread beat it then DB_E_NOINDEX.
		TESTC_(hrThreads[iThrd], S_OK);

		// And the appropriate index name should exist
		TESTC_(DoesIndexExist(&(m_pTable->GetTableID()), (DBID *)(TArgs[iThrd].pArg3), &fExists), S_OK);
		TESTC(fExists);

		// Drop the index
		TESTC_(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), (DBID *)(TArgs[iThrd].pArg3)), S_OK);
	}


CLEANUP:

	// Release DBID memory
	for (iThrd=0; iThrd < MAX_THREADS; iThrd++)
	{
		ReleaseDBID((DBID *)(TArgs[iThrd].pArg2));
		ReleaseDBID((DBID *)(TArgs[iThrd].pArg3));
	}
	SAFE_FREE(pIndexColumnDesc);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex on multiple threads, two tables to same index name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_24()
{ 
	// Both should succeed

	ULONG iThrd, iTime;
	HRESULT hrThreads[TWO_THREADS];
	ULONG cCols = 1;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBID * pTableFirst = &(m_pTable->GetTableIDRef());
	DBID * pTableSecond = &(m_pNoNullTable->GetTableIDRef());
	DBID * pIndexIDFirst = NULL;
	DBID * pIndexIDSecond = NULL;
	CDBID dbid(DBKIND_NAME, L"Idx00001");

	TBEGIN
	INIT_THREADS(TWO_THREADS);
	
	//Setup Thread Arguments
	THREADARG6 TArgs[TWO_THREADS];

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, cCols);
	memset(pIndexColumnDesc, 0, sizeof(DBINDEXCOLUMNDESC)*cCols);

	for (iTime = 0; iTime < 10; iTime++)
	{
		// Create the indexes to be altered
		TESTC_(CreateIndex(pTableFirst, NULL, cCols,
			pIndexColumnDesc, 0, NULL, &pIndexIDFirst), S_OK);

		TESTC_(CreateIndex(pTableSecond, NULL, cCols,
			pIndexColumnDesc, 0, NULL, &pIndexIDSecond), S_OK);

		// Set up the first thread args
		hrThreads[0] = E_FAIL;
		TArgs[0].pFunc = m_pIAlterIndex;			// IAlterIndex interface
		TArgs[0].pArg1 = pTableFirst;				// Table to alter
		TArgs[0].pArg2 = pIndexIDFirst;				// Index to alter
		TArgs[0].pArg3 = &dbid;						// New name
		TArgs[0].pArg4 = &hrThreads[0];				// Return code
		TArgs[0].pArg5 = 0;							// Count of props to set
		TArgs[0].pArg6 = NULL;						// Props to set

		//Create the thread
		CREATE_THREAD(0, Thread_VerifyAlterIndex, &TArgs[0]);

		// Set up the second thread args
		hrThreads[1] = E_FAIL;
		TArgs[1].pFunc = m_pIAlterIndex;			// IAlterIndex interface
		TArgs[1].pArg1 = pTableSecond;				// Table to alter
		TArgs[1].pArg2 = pIndexIDSecond;			// Index to alter
		TArgs[1].pArg3 = &dbid;						// New name
		TArgs[1].pArg4 = &hrThreads[1];				// Return code
		TArgs[1].pArg5 = 0;							// Count of props to set
		TArgs[1].pArg6 = NULL;						// Props to set

		//Create the thread
		CREATE_THREAD(1, Thread_VerifyAlterIndex, &TArgs[1]);

		START_THREADS();
		END_THREADS();	

		// See whether the threads succeeded or failed
		for (iThrd = 0; iThrd < TWO_THREADS; iThrd++)
		{
			BOOL fExists = FALSE;

			// Should succeed
			TESTC_(hrThreads[iThrd], S_OK);

			TESTC_(DoesIndexExist((DBID *)(TArgs[iThrd].pArg1), (DBID *)(TArgs[iThrd].pArg3), &fExists), S_OK);

			// And the appropriate index name should exist
			if (hrThreads[iThrd] == S_OK)
			{
				TESTC(fExists);

				// Drop the new index
				TESTC_(m_pIIndexDef->DropIndex((DBID *)(TArgs[iThrd].pArg1), (DBID *)(TArgs[iThrd].pArg3)), S_OK);
			}
			else
			{
				TESTC(!fExists);

				// Drop the old index
				TESTC_(m_pIIndexDef->DropIndex((DBID *)(TArgs[iThrd].pArg1), (DBID *)(TArgs[iThrd].pArg2)), S_OK);
			}

		}

		ReleaseDBID(pIndexIDFirst);
		pIndexIDFirst= NULL;

		ReleaseDBID(pIndexIDSecond);
		pIndexIDSecond = NULL;

	} // Next time

CLEANUP:

	odtLog << L"Times: " << iTime << L"\n";

	SAFE_FREE(pIndexColumnDesc);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex on multiple threads, name on one, props on another
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_25()
{ 
	// Both should succeed, or, props should fail if beaten by name change

	ULONG iThrd;
	HRESULT hrThreads[TWO_THREADS];
	ULONG cCols = 1;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBID * pTable = &(m_pNoNullTable->GetTableIDRef());
	DBID * pIndexID = NULL;
	CDBID dbid(DBKIND_NAME, L"Idx00001");

	TBEGIN
	INIT_THREADS(TWO_THREADS);
	
	//Setup Thread Arguments
	THREADARG6 TArgs[TWO_THREADS];

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, cCols);
	memset(pIndexColumnDesc, 0, sizeof(DBINDEXCOLUMNDESC)*cCols);

	// Set Primary Key prop as it's the most prevalent
	TESTC(m_cPropSets == 0);
	SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	// Create the index to be altered
	TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTable, NULL, m_cIndexColumnDesc,
		pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	// Set up the first thread args
	hrThreads[0] = E_FAIL;
	TArgs[0].pFunc = m_pIAlterIndex;			// IAlterIndex interface
	TArgs[0].pArg1 = pTable;					// Table to alter
	TArgs[0].pArg2 = pIndexID;					// Index to alter
	TArgs[0].pArg3 = pIndexID;					// New name same as old name
	TArgs[0].pArg4 = &hrThreads[0];				// Return code
	TArgs[0].pArg5 = (LPVOID)(ULONG_PTR)m_cPropSets;// Count of props to set
	TArgs[0].pArg6 = (LPVOID)m_rgPropSets;		// Props to set

	//Create the thread
	CREATE_THREAD(0, Thread_VerifyAlterIndex, &TArgs[0]);

	// Set up the second thread args
	hrThreads[1] = E_FAIL;
	TArgs[1].pFunc = m_pIAlterIndex;			// IAlterIndex interface
	TArgs[1].pArg1 = pTable;					// Table to alter
	TArgs[1].pArg2 = pIndexID;					// Index to alter
	TArgs[1].pArg3 = &dbid;						// New name
	TArgs[1].pArg4 = &hrThreads[1];				// Return code
	TArgs[1].pArg5 = (LPVOID)0;					// Count of props to set
	TArgs[1].pArg6 = NULL;						// Props to set

	//Create the thread
	CREATE_THREAD(1, Thread_VerifyAlterIndex, &TArgs[1]);

	START_THREADS();
	END_THREADS();	

	// See whether the threads succeeded or failed
	for (iThrd = 0; iThrd < TWO_THREADS; iThrd++)
	{
		BOOL fExists = FALSE;

		// Prop thread may succeed, or fail if beaten by name change
		if (iThrd == 0)
		{
			TEST2C_(hrThreads[iThrd], S_OK, DB_E_NOINDEX);
		}
		else
		{
			// Name change must always succeed
			TESTC_(hrThreads[iThrd], S_OK);
		}

		TESTC_(DoesIndexExist((DBID *)(TArgs[iThrd].pArg1), (DBID *)(TArgs[iThrd].pArg3), &fExists), S_OK);

		// And the appropriate index name should exist
		if (iThrd == 1 && hrThreads[iThrd] == S_OK)
		{
			TESTC(fExists);

			// Drop the new index
			TESTC_(m_pIIndexDef->DropIndex((DBID *)(TArgs[iThrd].pArg1), (DBID *)(TArgs[iThrd].pArg3)), S_OK);
		}
		else
		{
			// The old index cannot exist
			TESTC(!fExists);
		}

	}


CLEANUP:

	
	ReleaseDBID(pIndexID);
	pIndexID= NULL;

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	SAFE_FREE(pIndexColumnDesc);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex on index created by command
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_26()
{ 
	TBEGIN
	HRESULT		hr;
	ULONG		iCol, iCCol;
	ULONG		rgIndexCols[MAX_INDEX_COLS];
	ULONG *		prgIndexCols = rgIndexCols;
	CDBID		dbidNew(DBKIND_NAME,  BuildValidName(m_cMaxIndexName, L"ABC"));
	CDBID		dbidOld(DBKIND_NAME,  BuildValidName(m_cMaxIndexName, L"XYZ"));

	// Get the column numbers for the index
	for (iCol = 0; iCol < m_cIndexColumnDesc; iCol++)
	{
		// Find the matching CCol DBID
		for (iCCol = 1; iCCol <= m_pTable->CountColumnsOnTable(); iCCol++)
		{
			CCol		TempCol;

			TESTC_(m_pTable->GetColInfo(iCCol, TempCol), S_OK);

			// if this DBID matched the index col DBID then this is the right col
			if (CompareDBID(*(m_pIndexColumnDesc[iCol].pColumnID),
						*TempCol.GetColID(), m_pThisTestModule->m_pIUnknown))
				break;
		}

		// We *must* find the matching DBID
		TESTC(iCCol < m_pTable->CountColumnsOnTable());

		// Insert column number in array
		rgIndexCols[iCol] = iCCol;
	}

	// Create an index with a command.  Just to be different use descending index.
	TESTC_(m_pTable->ExecuteCommand(CREATE_INDEX_DESC, IID_NULL, dbidOld.uName.pwszName,
		NULL, &m_cIndexColumnDesc, (DB_LORDINAL **)&prgIndexCols), S_OK);

	// Alter the index
	TESTC_(hr = AlterIndex(&dbidOld, &dbidNew), S_OK);

	// Drop the index
	TESTC_(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), &dbidNew), S_OK);

CLEANUP:

	SAFE_FREE(dbidOld.uName.pwszName);
	SAFE_FREE(dbidNew.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex with quoted name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_27()
{ 
	TBEGIN
	HRESULT		hr;
	LPWSTR		pwszNewIndexName = BuildValidName(m_cMaxIndexName, L"XYZ");
	LPWSTR		pwszTableName = (&(m_pTable->GetTableID()))->uName.pwszName;
	LPWSTR		pwszIndexName = m_pIndexID->uName.pwszName;
	LPWSTR		pwszQuotedTableName = NULL;
	LPWSTR		pwszQuotedIndexName = NULL;
	LPWSTR		pwszQuotedNewIndexName = NULL;

	// Get quoted table name
	TESTC_(m_pTable->GetQuotedName(pwszTableName,&pwszQuotedTableName), S_OK);

	// Get quoted current index name
	TESTC_(m_pTable->GetQuotedName(pwszIndexName,&pwszQuotedIndexName), S_OK);

	// Get quoted new index name
	TESTC_(m_pTable->GetQuotedName(pwszNewIndexName,&pwszQuotedNewIndexName), S_OK);

	// Set the names into the dbid's
	{
		CDBID		dbidNew(DBKIND_NAME, pwszQuotedNewIndexName);
		CDBID		dbidNewUnquoted(DBKIND_NAME, pwszNewIndexName);
		CDBID		dbidOld(DBKIND_NAME, pwszQuotedIndexName);
		CDBID		dbidTable(DBKIND_NAME, pwszQuotedTableName);
		BOOL		fExists = FALSE;

		// Call AlterIndex.  Note our helper function can't handle quoted or qualified
		// names, so we have to call directly.
		TESTC_(hr = m_pIAlterIndex->AlterIndex(&dbidTable, &dbidOld, &dbidNew, 0, NULL), S_OK)

		// Since we didn't use our helper we have to check index existence ourself.
		TESTC_(DoesIndexExist(&(m_pTable->GetTableID()), &dbidNewUnquoted, &fExists), S_OK);
		TESTC(fExists == TRUE);

		Refresh_m_pIndexID(hr, dbidNewUnquoted);

		TESTC_(hr, S_OK);
	}

CLEANUP:

	SAFE_FREE(pwszNewIndexName);
	SAFE_FREE(pwszQuotedTableName);
	SAFE_FREE(pwszQuotedIndexName);
	SAFE_FREE(pwszQuotedNewIndexName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex with qualified and quoted name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_28()
{ 
	TBEGIN
	HRESULT		hr;
	LPWSTR		pwszNewIndexName = BuildValidName(m_cMaxIndexName, L"XYZ");
	LPWSTR		pwszTableName = (&(m_pTable->GetTableID()))->uName.pwszName;
	LPWSTR		pwszIndexName = m_pIndexID->uName.pwszName;
	LPWSTR		pwszCatalogName = NULL;
	LPWSTR		pwszSchemaName = NULL;
	LPWSTR		pwszQualifiedTableName = NULL;
	LPWSTR		pwszQualifiedIndexName = NULL;
	LPWSTR		pwszQualifiedNewIndexName = NULL;

	TESTC(GetQualifierNames(m_pThisTestModule->m_pIUnknown2, pwszTableName,
		&pwszCatalogName, &pwszSchemaName));
	
	// Get Qualified table name
	TESTC_(m_pTable->GetQualifiedName(pwszCatalogName, pwszSchemaName, 
		pwszTableName,&pwszQualifiedTableName), S_OK);

	// Get Qualified current index name
	TESTC_(m_pTable->GetQualifiedName(pwszCatalogName, pwszSchemaName, 
		pwszIndexName,&pwszQualifiedIndexName), S_OK);

	// Get Qualified new index name
	TESTC_(m_pTable->GetQualifiedName(pwszCatalogName, pwszSchemaName, 
		pwszNewIndexName,&pwszQualifiedNewIndexName), S_OK);

	// Set the names into the dbid's
	{
		CDBID		dbidNew(DBKIND_NAME, pwszQualifiedNewIndexName);
		CDBID		dbidNewUnQualified(DBKIND_NAME, pwszNewIndexName);
		CDBID		dbidOld(DBKIND_NAME, pwszQualifiedIndexName);
		CDBID		dbidTable(DBKIND_NAME, pwszQualifiedTableName);
		BOOL		fExists = FALSE;

		// Call AlterIndex.  Note our helper function can't handle Qualified or qualified
		// names, so we have to call directly.
		TESTC_(hr = m_pIAlterIndex->AlterIndex(&dbidTable, &dbidOld, &dbidNew, 0, NULL), S_OK)

		// Since we didn't use our helper we have to check index existence ourself.
		TESTC_(DoesIndexExist(&(m_pTable->GetTableID()), &dbidNewUnQualified, &fExists), S_OK);
		TESTC(fExists == TRUE);

		Refresh_m_pIndexID(hr, dbidNewUnQualified);

		TESTC_(hr, S_OK);
	}

CLEANUP:

	SAFE_FREE(pwszNewIndexName);
	SAFE_FREE(pwszQualifiedTableName);
	SAFE_FREE(pwszQualifiedIndexName);
	SAFE_FREE(pwszQualifiedNewIndexName);
	SAFE_FREE(pwszCatalogName);
	SAFE_FREE(pwszSchemaName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex to same name as PrimaryKey constraint
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_29()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, NULL);
	DBID * pTableID = &(m_pNoNullTable->GetTableIDRef());
	DBCOLUMNDESC * pColumnDesc = NULL;
	DBPROPSET * pColPropSets = NULL;
	ULONG cColPropSets = 0;
	DBORDINAL cCols = m_pTable->CountColumnsOnTable();	
	ULONG iCol;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBID * pIndexID = NULL;
	CTable NewTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName, NONULLS);
	CIndexInfo  IndexInfo;
	LPWSTR pwszPK = NULL;

	// Note we could just AlterTable on the current automaketable to create a
	// PrimaryKey constraint, but at the time this was written AlterTable did not
	// properly support this.  

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, cCols);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*cCols));

	// Get the DBCOLUMNDESC information from the automaketable
	TESTC_(m_pTable->BuildColumnDescs(&pColumnDesc), S_OK);

	// Create a primary key index on the automaketable so we can tell which column
	// will support a primary key.
	TESTC(m_cPropSets == 0);
	SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	// Create the index to be altered
	TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
		pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	// Drop the index on the automaketable - we only wanted the column descs
	TESTC_(m_pIIndexDef->DropIndex(pTableID, pIndexID), S_OK); 
	ReleaseDBID(pIndexID);

	// Add a PrimaryKey prop for this column for the new table
	for (iCol = 0; iCol < cCols; iCol++)
	{
		if (CompareDBID(*pIndexColumnDesc[iCol].pColumnID, pColumnDesc[iCol].dbcid))
		{
			SetProperty(DBPROP_COL_PRIMARYKEY, DBPROPSET_COLUMN,
				&pColumnDesc[iCol].cPropertySets, &pColumnDesc[iCol].rgPropertySets,
				(void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
			break;
		}
	}

	TESTC(iCol < cCols);

	// Set the column desc info for the new table
	TESTC(NewTable.SetColumnDesc(pColumnDesc, cCols) != NULL);
	NewTable.SetBuildColumnDesc(FALSE);

	// Create the new table
	TESTC_(NewTable.CreateTable(MIN_TABLE_ROWS, 0), S_OK);

	// Get information about the index if the provider created an index to support
	// the primary key constraint
	TESTC_PROVIDER(IndexInfo.Init(m_pIOpenRowset, &(NewTable.GetTableID()), NULL));

	// Get the name of the PK index (the only index on the table at this point)
	pwszPK = (LPWSTR)IndexInfo.GetIndexValuePtr(0, IS_INDEX_NAME);

	TESTC(pwszPK != NULL);

	// Set the name the same as the PK name
	dbidNew.uName.pwszName = pwszPK;

	// Free the props so we're not making a duplicate PK
	FreeProperties(&m_cPropSets, &m_rgPropSets);

	// Create an index on the new table, using the same column as above.
	TESTC_PROVIDER(S_OK == CreateIndex(&(NewTable.GetTableID()),
		NULL, m_cIndexColumnDesc,pIndexColumnDesc, 0, NULL,
		&pIndexID));

	// Call AlterIndex to set this name.
	TESTC_(hr = AlterIndex(&(NewTable.GetTableID()), pIndexID, &dbidNew), DB_E_DUPLICATEINDEXID)

CLEANUP:

	ReleaseDBID(pIndexID);

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	SAFE_FREE(pIndexColumnDesc);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex on temp table
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_30()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, BuildValidName(m_cMaxTableName+1, L"IAlterIndex"));
	DBID * pTableID = NULL; 
	DBCOLUMNDESC * pColumnDesc = NULL;
	DBPROPSET * pColPropSets = NULL;
	ULONG cColPropSets = 0;
	DBORDINAL cCols = m_pTable->CountColumnsOnTable();	
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBID * pIndexID = NULL;
	CTable NewTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName, NONULLS);
	DBPROPSET * pTablePropSets = NULL;
	ULONG cTablePropSets = 0;

	// Note we could just AlterTable on the current automaketable to create a
	// PrimaryKey constraint, but at the time this was written AlterTable did not
	// properly support this.  

	// Set primary key prop
	TESTC(m_cPropSets == 0);
	SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, cCols);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*cCols));

	// Get the DBCOLUMNDESC information from the automaketable
	TESTC_(m_pTable->BuildColumnDescs(&pColumnDesc), S_OK);

	// Change this table to be a temp table.  Not all providers will support this
	TESTC(SetProperty(DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, &cTablePropSets, &pTablePropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED));
	NewTable.SetPropertySets(pTablePropSets, cTablePropSets);

	// Set the column desc info for the new table
	TESTC(NewTable.SetColumnDesc(pColumnDesc, cCols) != NULL);
	NewTable.SetDBID(&pTableID);
	NewTable.SetBuildColumnDesc(FALSE);
	NewTable.ResetInputTableID();

	// Create the temp table
	TESTC_PROVIDER(S_OK == NewTable.CreateTable(MIN_TABLE_ROWS, 0));

	// Create an index on the new table, using the same column as above.
	TESTC_PROVIDER(S_OK == CreateAlterableIndex(&(NewTable.GetTableID()),
		NULL, m_cIndexColumnDesc, pIndexColumnDesc, m_cPropSets, m_rgPropSets,
		&pIndexID));

	// Call AlterIndex to set this name and props
	TESTC_(hr = AlterIndex(&(NewTable.GetTableID()), pIndexID, &dbidNew), S_OK);

CLEANUP:

	// We don't drop the index 'cause it's dropped when the table is dropped.
	// Ditto for table props
	FreeProperties(&m_cPropSets, &m_rgPropSets);

	SAFE_FREE(pIndexColumnDesc);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc General - pIndexID is DBKIND_GUID_NAME
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_31()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidOld(DBKIND_GUID_NAME, m_pIndexID->uName.pwszName);
	CDBID		dbidNew(DBKIND_NAME, L"abc123");

	// At this time no providers support DBKIND_GUID_NAME

	dbidOld.uGuid.guid = DBGUID_DSO;
	TEST2C_(hr = AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX, S_OK)

	if(DB_E_NOINDEX == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_GUID_NAME are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc General - pIndexID is DBKIND_GUID_PROPID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_32()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidOld(DBKIND_GUID_PROPID, NULL);
	CDBID		dbidNew(DBKIND_NAME, L"abc123");

	// At this time no providers support DBKIND_GUID_PROPID

	dbidOld.uName.ulPropid = 666;
	dbidOld.uGuid.guid = DBGUID_DSO;
	TEST2C_(hr = AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX, S_OK)

	if(DB_E_NOINDEX == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_GUID_PROPID are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc General - pIndexID is DBKIND_PGUID_NAME
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_33()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidOld(DBKIND_PGUID_NAME, m_pIndexID->uName.pwszName);
	CDBID		dbidNew(DBKIND_NAME, L"abc123");
	GUID		guid = DBGUID_ROW;

	// At this time no providers support DBKIND_PGUID_NAME

	dbidOld.uGuid.pguid = &guid;
	TEST2C_(hr = AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX, S_OK)

	if(DB_E_NOINDEX == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_PGUID_NAME are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc General - pIndexID is DBKIND_PGUID_PROPID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_34()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidOld(DBKIND_PGUID_PROPID, NULL);
	CDBID		dbidNew(DBKIND_NAME, L"abc123");
	GUID		guid = DBGUID_ROW;

	// At this time no providers support DBKIND_PGUID_PROPID

	dbidOld.uName.ulPropid = 666;
	dbidOld.uGuid.pguid = &guid;
	TEST2C_(hr = AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX, S_OK)

	if(DB_E_NOINDEX == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_PGUID_PROPID are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc General - pIndexID is DBKIND_PROPID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_35()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidOld(DBKIND_PROPID, NULL);
	CDBID		dbidNew(DBKIND_NAME, L"abc123");

	// At this time no providers support DBKIND_PROPID

	dbidOld.uName.ulPropid = 666;
	TEST2C_(hr = AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX, S_OK)

	if(DB_E_NOINDEX == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_PROPID are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc General - pIndexID is DBKIND_GUID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_36()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidOld(DBKIND_GUID, NULL);
	CDBID		dbidNew(DBKIND_NAME, L"abc123");

	// At this time no providers support DBKIND_GUID

	dbidOld.uGuid.guid = DBGUID_DSO;
	TEST2C_(hr = AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX, S_OK)

	if(DB_E_NOINDEX == hr)
		odtLog<<L"INFO: Index IDs of DBKIND_GUID are considered invalid.\n";

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc General - AlterIndex with rowset open on different table
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAI_SingCol::Variation_37()
{ 
	TBEGIN
	CDBID			dbidNew(DBKIND_NAME,  L"MtRainier");
	DBID *			pdbidTable		= &(m_pNoNullTable->GetTableIDRef());
	IRowset *		pIRowset = NULL;
	HRESULT			hr = E_FAIL;
	
	// Open a rowset on our other table
	TESTC_(m_pIOpenRowset->OpenRowset(NULL, pdbidTable,
		NULL, IID_IRowset, 0, NULL, (IUnknown**)
		&pIRowset), S_OK); 

	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK);

CLEANUP:

	SAFE_RELEASE(pIRowset);

	Refresh_m_pIndexID(hr, dbidNew);


	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCAI_SingCol::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAlterIndex::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCBoundary_SingCol)
//*-----------------------------------------------------------------------
//| Test Case:		TCBoundary_SingCol - Boundary cases
//| Created:  	8/2/1999
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBoundary_SingCol::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	return CAlterIndex::Init();
	// }}
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - pTableID = NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_1()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"GoldFinger");

	TESTC_(m_pIAlterIndex->AlterIndex(NULL, m_pIndexID, &dbidNew, 0, NULL), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - pIndexID = NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_2()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"GoldFinger2");

	TESTC_(m_pIAlterIndex->AlterIndex(&(m_pTable->GetTableID()), NULL, &dbidNew, 0, NULL), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - pTableID = NULL and pIndexID = NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_3()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"GoldFinger3");

	TESTC_(m_pIAlterIndex->AlterIndex(NULL, NULL, &dbidNew, 0, NULL), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - cPropSets > 0 and rgPropSets NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_4()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"DrEvil");

	TESTC_(m_pIAlterIndex->AlterIndex(NULL, NULL, &dbidNew, 3, NULL), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - cProperties != 0 and rgProperties NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_5()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"MrBigglesworth");
	DBPROPSET * prgPropSets=NULL;
	ULONG cPropSets = 0;

	SetProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX, &cPropSets, &prgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	SAFE_FREE(prgPropSets[0].rgProperties);

	TESTC_(m_pIAlterIndex->AlterIndex(NULL, NULL, &dbidNew, cPropSets, prgPropSets), E_INVALIDARG)

CLEANUP:

	FreeProperties(&cPropSets, &prgPropSets);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTABLE - *pTableID == DB_NULLID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_6()
{ 
	TBEGIN
	DBID		dbTableID = DB_NULLID;
	CDBID		dbidNew(DBKIND_NAME,  L"Cello");

	TESTC_(AlterIndex(&dbTableID, m_pIndexID, &dbidNew, FALSE), DB_E_NOTABLE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTABLE - pTableID->uName is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_7()
{ 
	TBEGIN
	CDBID		dbTableID(DBKIND_NAME, NULL);
	CDBID		dbidNew(DBKIND_NAME,  L"Cello2");

	TESTC_(AlterIndex(&dbTableID, m_pIndexID, &dbidNew, FALSE), DB_E_NOTABLE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTABLE - pTableID->uName is empty string
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_8()
{ 
	TBEGIN
	CDBID		dbTableID(DBKIND_NAME, L"");
	CDBID		dbidNew(DBKIND_NAME,  L"Trumpet");

	TESTC_(AlterIndex(&dbTableID, m_pIndexID, &dbidNew, FALSE), DB_E_NOTABLE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTABLE - Table name invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_9()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"Clarinet");
	CDBID		dbidTable(DBKIND_NAME,  wcsDuplicate(m_pTable->GetTableID().uName.pwszName));
	size_t		iChar;
	HRESULT		hr = E_FAIL;
	size_t		cchInvalid = wcslen(m_pwszInvalidTableChars);
	size_t		cchName = wcslen(m_pTable->GetTableID().uName.pwszName);
	WCHAR		wszInvalidChar[2] = L"A";

	// We must have at least one invalid character to test this
	TESTC_PROVIDER(cchInvalid > 0);

	// There must be at least two characters in the existing table
	TESTC(cchName > 1);

	// Replace the last character in the valid name with invalid ones
	for (iChar = 0; iChar < cchInvalid; iChar++)
	{
		dbidTable.uName.pwszName[cchName-1] = m_pwszInvalidTableChars[iChar];

		if (!CHECK(hr = AlterIndex(&dbidTable, m_pIndexID, &dbidNew, FALSE), DB_E_NOTABLE))
		{
			wszInvalidChar[0] = m_pwszInvalidTableChars[iChar];
			odtLog << L"Invalid character # " << iChar << " '" << wszInvalidChar << L"' failed.\n\n";
		}

		Refresh_m_pIndexID(hr, dbidNew);

	}

	// Reset back to the valid name
	wcscpy(dbidTable.uName.pwszName, m_pTable->GetTableID().uName.pwszName);

	// Now try invalid starting characters
	cchInvalid = wcslen(m_pwszInvalidTableStartingChars);

	// Replace the first character in the valid name with invalid ones
	for (iChar = 0; iChar < cchInvalid; iChar++)
	{
		dbidTable.uName.pwszName[0] = m_pwszInvalidTableStartingChars[iChar];

		if (!CHECK(hr = AlterIndex(&dbidTable, m_pIndexID, &dbidNew, FALSE), DB_E_NOTABLE))
		{
			wszInvalidChar[0] = m_pwszInvalidTableStartingChars[iChar];
			odtLog << L"Invalid starting character # " << iChar << " '" << wszInvalidChar << L"' failed.\n\n";
		}

		Refresh_m_pIndexID(hr, dbidNew);

	}

CLEANUP:
	
	SAFE_FREE(dbidTable.uName.pwszName);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTABLE - Table name exceeds max length
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_10()
{ 
	TBEGIN
	CDBID		dbTableID(DBKIND_NAME,  BuildValidName(m_cMaxTableName+1, L"IAlterIndex"));
	CDBID		dbidNew(DBKIND_NAME,  L"Bass");

	// TODO:  It would really be better here to create an actual table of max
	// length, and then use that name plus one char to see if provider will
	// truncate name and then use the valid table.  But since this doesn't
	// work for ini files it's kinda useless.  We need to create a separate max
	// name length table here if not using an ini file and create an index on it.

	TESTC_(AlterIndex(&dbTableID, m_pIndexID, &dbidNew, FALSE), DB_E_NOTABLE);

CLEANUP:

	ReleaseDBID(&dbTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTABLE - Table does not exist
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_11()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"GoldFinger4");
	CDBID		dbidTable(DBKIND_NAME,  L"BogusX1");

	TESTC_(m_pIAlterIndex->AlterIndex(&dbidTable, m_pIndexID, &dbidNew, 0, NULL), DB_E_NOTABLE)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOINDEX - Index does not exist
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_12()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"GoldFinger5");
	CDBID		dbidOld(DBKIND_NAME,  L"BogusX1");

	TESTC_(AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOINDEX - *pIndexID == DB_NULLID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_13()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"BogusX1");
	DBID		dbidOld = DB_NULLID;

	TESTC_(AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOINDEX - pIndexID->uName is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_14()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"BogusX1");
	CDBID		dbidOld(DBKIND_NAME,  NULL);

	TESTC_(AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOINDEX - pIndexID->uName is empty string
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_15()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"BogusX1");
	CDBID		dbidOld(DBKIND_NAME,  L"");

	TESTC_(AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOINDEX - pIndexID name is invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_16()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"Trombone");
	CDBID		dbidOld(DBKIND_NAME,  wcsDuplicate(m_pIndexID->uName.pwszName));
	size_t		iChar;
	HRESULT		hr = E_FAIL;
	size_t		cchInvalid = wcslen(m_pwszInvalidIndexChars);
	size_t		cchName = wcslen(m_pIndexID->uName.pwszName);
	WCHAR		wszInvalidChar[2] = L"A";

	// We must have at least one invalid character to test this
	TESTC_PROVIDER(cchInvalid > 0);

	// There must be at least two characters in the existing index
	TESTC(cchName > 1);

	// Replace the last character in the valid name with invalid ones
	for (iChar = 0; iChar < cchInvalid; iChar++)
	{
		dbidOld.uName.pwszName[cchName-1] = m_pwszInvalidIndexChars[iChar];

		if (!CHECK(hr = AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX))
		{
			wszInvalidChar[0] = m_pwszInvalidIndexChars[iChar];
			odtLog << L"Invalid character # " << iChar << " '" << wszInvalidChar << L"' failed.\n\n";
		}

		Refresh_m_pIndexID(hr, dbidNew);

	}

	// Reset back to the valid name
	wcscpy(dbidOld.uName.pwszName, m_pIndexID->uName.pwszName);

	// Now try invalid starting characters
	cchInvalid = wcslen(m_pwszInvalidIndexStartingChars);

	// Replace the first character in the valid name with invalid ones
	for (iChar = 0; iChar < cchInvalid; iChar++)
	{
		dbidOld.uName.pwszName[0] = m_pwszInvalidIndexStartingChars[iChar];

		if (!CHECK(hr = AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX))
		{
			wszInvalidChar[0] = m_pwszInvalidIndexStartingChars[iChar];
			odtLog << L"Invalid starting character # " << iChar << " '" << wszInvalidChar << L"' failed.\n\n";
		}

		Refresh_m_pIndexID(hr, dbidNew);

	}

CLEANUP:
	
	SAFE_FREE(dbidOld.uName.pwszName);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOINDEX - pIndexID name exceeds max name length
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_17()
{ 
	TBEGIN
	CDBID		dbidOld(DBKIND_NAME,  BuildValidName(m_cMaxIndexName+1, L"IAlterIndex"));
	CDBID		dbidNew(DBKIND_NAME,  L"Bass");

	// TODO:  It would really be better here to create an actual index of max
	// length to see if the provider will find the max length one instead.

	TESTC_(AlterIndex(&dbidOld, &dbidNew, FALSE), DB_E_NOINDEX);

CLEANUP:

	ReleaseDBID(&dbidOld, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_E_DUPLICATEINDEXID - The new index already exists
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_18()
{ 
	TBEGIN
	DBID		* pdbidExisting = NULL;
	DBINDEXCOLUMNDESC rgIndexColumnDesc[1];

	// Create another index on the table so we can ensure there is a duplicate
	// index with a name that doesn't match the current index name.
	memset(rgIndexColumnDesc, 0, sizeof(rgIndexColumnDesc));
	TESTC_(CreateIndex(&(m_pTable->GetTableID()), NULL, 1,
		rgIndexColumnDesc, 0, NULL, &pdbidExisting), S_OK);

	TESTC_(AlterIndex(m_pIndexID, pdbidExisting), DB_E_DUPLICATEINDEXID);

CLEANUP:

	// Drop the duplicate index
	if (pdbidExisting)
		CHECK(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), pdbidExisting), S_OK); 

	ReleaseDBID(pdbidExisting);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc DBSEC_E_PERMISSIONDENIED - Insufficient permissions to alter index
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_19()
{ 
	TBEGIN
	ULONG	cExPropSets = 0;
	DBPROPSET * prgExPropSets = NULL;
	CDBID		dbidNew(DBKIND_NAME,  L"GoldRush");
	IAlterIndex * pIAlterIndex = NULL;
	IDBCreateSession * pIDBCreateSession = NULL;
	CSessionObject SessionObject(L"TCBoundary_SingCol");

	// Save the IAlterIndex object we use off our normal session
	pIAlterIndex = m_pIAlterIndex;
	m_pIAlterIndex = NULL;

	// Create a new session with DBPROP_INIT_MODE DB_MODE_READ

	// Set mode prop
	TESTC(SetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, &cExPropSets, &prgExPropSets, (void*)DB_MODE_READ, DBTYPE_I4, DBPROPOPTIONS_REQUIRED));

	// Create DSO
	TESTC_(SessionObject.CreateDataSourceObject(), S_OK);
						
	// Initialize the DSO with these props
	TESTC_PROVIDER(S_OK == SessionObject.InitializeDSO(REINITIALIZE_YES, cExPropSets, prgExPropSets));

	// Create the session object
	TESTC(VerifyInterface(SessionObject.m_pIDBInitialize, IID_IDBCreateSession, DATASOURCE_INTERFACE, (IUnknown **)&pIDBCreateSession));
	
	// Get the new IAlterIndex
	TESTC_(pIDBCreateSession->CreateSession(NULL, IID_IAlterIndex, (IUnknown **)&m_pIAlterIndex), S_OK);

	TESTC_(AlterIndex(m_pIndexID, &dbidNew), DB_SEC_E_PERMISSIONDENIED);

CLEANUP:
	// Free the init props
	FreeProperties(&cExPropSets, &prgExPropSets);

	// Release the second session's IAlterIndex
	SAFE_RELEASE(m_pIAlterIndex);

	// Release the second session's pIDBCreateSession
	SAFE_RELEASE(pIDBCreateSession);

	// Put the AlterIndex interface back to normal
	m_pIAlterIndex = pIAlterIndex;

	// Release the r/o session object
	SessionObject.ReleaseDBSession();
	SessionObject.ReleaseDataSourceObject();

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADINDEXID - pNewIndexID == DB_NULLID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_20()
{ 
	TBEGIN
	DBID		dbidNew = DB_NULLID;

	TESTC_(AlterIndex(m_pIndexID, &dbidNew), DB_E_BADINDEXID);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADINDEXID - pNewIndexID->uName is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_21()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  NULL);

	TESTC_(AlterIndex(m_pIndexID, &dbidNew), DB_E_BADINDEXID);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADINDEXID - pNewIndexID->uName is empty string
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_22()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"");

	TESTC_(AlterIndex(m_pIndexID, &dbidNew), DB_E_BADINDEXID);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADINDEXID - Specify an invalid new index ID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_23()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  wcsDuplicate(m_pIndexID->uName.pwszName));
	size_t		iChar;
	HRESULT		hr = E_FAIL;
	size_t		cchInvalid = wcslen(m_pwszInvalidIndexChars);
	size_t		cchName = wcslen(m_pIndexID->uName.pwszName);
	WCHAR		wszInvalidChar[2] = L"A";

	// We must have at least one invalid character to test this
	TESTC_PROVIDER(cchInvalid > 0);

	// There must be at least two characters in the existing index
	TESTC(cchName > 1);

	// Replace the last character in the valid name with invalid ones
	for (iChar = 0; iChar < cchInvalid; iChar++)
	{
		dbidNew.uName.pwszName[cchName-1] = m_pwszInvalidIndexChars[iChar];

		if (!CHECK(hr = AlterIndex(m_pIndexID, &dbidNew), DB_E_BADINDEXID))
		{
			wszInvalidChar[0] = m_pwszInvalidIndexChars[iChar];
			odtLog << L"Invalid character # " << iChar << " '" << wszInvalidChar << L"' failed.\n\n";
		}

		Refresh_m_pIndexID(hr, dbidNew);

	}

	// Reset back to the valid name
	wcscpy(dbidNew.uName.pwszName, m_pIndexID->uName.pwszName);

	// Now try invalid starting characters
	cchInvalid = wcslen(m_pwszInvalidIndexStartingChars);

	// Replace the first character in the valid name with invalid ones
	for (iChar = 0; iChar < cchInvalid; iChar++)
	{
		dbidNew.uName.pwszName[0] = m_pwszInvalidIndexStartingChars[iChar];

		if (!CHECK(hr = AlterIndex(m_pIndexID, &dbidNew), DB_E_BADINDEXID))
		{
			wszInvalidChar[0] = m_pwszInvalidIndexStartingChars[iChar];
			odtLog << L"Invalid starting character # " << iChar << " '" << wszInvalidChar << L"' failed.\n\n";
		}

		Refresh_m_pIndexID(hr, dbidNew);

	}

CLEANUP:
	
	SAFE_FREE(dbidNew.uName.pwszName);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADINDEXID - pNewIndexID name exceeds max name length
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_24()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  BuildValidName(m_cMaxIndexName+1, L"IAlterIndex"));

	TESTC_(AlterIndex(m_pIndexID, &dbidNew), DB_E_BADINDEXID);

CLEANUP:

	ReleaseDBID(&dbidNew, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc DB_E_INDEXINUSE - AlterIndex with index in use
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_25()
{ 
	TBEGIN
	CDBID			dbidNew(DBKIND_NAME,  L"OddJobHat");
	DBID		*	pdbidTable		= &(m_pTable->GetTableIDRef());
	IRowsetIndex *	pIRowsetIndex	= NULL;
	
	// If neither integrated nor separate indexes are supported we
	// don't have a way to ensure the index is in use.
	TESTC_PROVIDER(m_ulIRowsetIndex != OIS_NONE);
	
	// If integrated indexes are supported we can pass the table id
	// otherwise table ID must be NULL;
	if (m_ulIRowsetIndex & OIS_ROWSET)
		pdbidTable = NULL;

	// Open a rowset using this index
	TESTC_(m_pIOpenRowset->OpenRowset(NULL, pdbidTable,
		m_pIndexID, IID_IRowsetIndex, 0, NULL, (IUnknown**)
		&pIRowsetIndex), S_OK); 

	if (m_ulIRowsetIndex & OIS_ROWSET)
	{
		TESTC_(AlterIndex(m_pIndexID, &dbidNew), DB_E_INDEXINUSE);
	}
	else
	{
		TEST2C_(AlterIndex(m_pIndexID, &dbidNew), DB_E_INDEXINUSE, DB_E_TABLEINUSE);
	}

CLEANUP:

	SAFE_RELEASE(pIRowsetIndex);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc DB_E_TABLEINUSE - AlterIndex with table in use
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_26()
{ 
	TBEGIN
	CDBID			dbidNew(DBKIND_NAME,  L"OddJobHat");
	DBID		*	pdbidTable		= &(m_pTable->GetTableIDRef());
	IRowset		*	pIRowset		= NULL;
	HRESULT			hr				= E_FAIL;

	// Open a rowset using this index
	TESTC_(m_pIOpenRowset->OpenRowset(NULL, pdbidTable,
		NULL, IID_IRowset, 0, NULL, (IUnknown**)
		&pIRowset), S_OK); 

	// Some providers may allow indexes to be altered while the table is open
	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_E_TABLEINUSE);

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	SAFE_RELEASE(pIRowset);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc S_OK - cPropSets == 0, rgPropSets is ignored
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_27()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME,  L"Oboe");

	m_cPropSets = 0;
	m_rgPropSets = INVALID(DBPROPSET *);

	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK)

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	m_rgPropSets = NULL;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOINDEX - AlterIndex on a dropped index
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_28()
{ 
	TBEGIN
	DBID		* pdbidDropped = NULL;
	CDBID		dbidNew(DBKIND_NAME,  L"Baritone");
	DBINDEXCOLUMNDESC rgIndexColumnDesc[1];

	// Create another index on the table so we can have one to drop safely
	memset(rgIndexColumnDesc, 0, sizeof(rgIndexColumnDesc));
	TESTC_(CreateIndex(&(m_pTable->GetTableID()), NULL, 1,
		rgIndexColumnDesc, 0, NULL, &pdbidDropped), S_OK);

	// Drop the duplicate index
	if (pdbidDropped)
		CHECK(m_pIIndexDef->DropIndex(&(m_pTable->GetTableID()), pdbidDropped), S_OK); 

	TESTC_(AlterIndex(pdbidDropped, &dbidNew, FALSE), DB_E_NOINDEX);

CLEANUP:

	ReleaseDBID(pdbidDropped);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTABLE - Pass procedure name as existing table name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_29()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"Cello");
	LPWSTR		pwszProcName = MakeObjectName(L"IAlterIn", m_cMaxTableName);

	// Create a stored proc with a different name than the table
	TESTC_PROVIDER(S_OK == m_pTable->ExecuteCommand(CREATE_PROC, IID_NULL,
		pwszProcName,NULL,NULL, NULL, EXECUTE_IFNOERROR, 0, NULL, 
		NULL, NULL, NULL));

	{
		CDBID dbTableID(DBKIND_NAME,  pwszProcName);

		TESTC_(AlterIndex(&dbTableID, m_pIndexID, &dbidNew, FALSE), DB_E_NOTABLE);
	}

CLEANUP:

	// Drop the stored proc we created above. Note it might not exist.
	m_pTable->ExecuteCommand(DROP_PROC, IID_NULL,
		pwszProcName,NULL,NULL, NULL, EXECUTE_IFNOERROR, 0, NULL, 
		NULL, NULL, NULL);

	SAFE_FREE(pwszProcName);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOINDEX - Pass procedure name as existing index name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_30()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"Cello");
	LPWSTR		pwszProcName = MakeObjectName(L"IAlterIn", m_cMaxTableName);

	// Create a stored proc with a different name than the table
	TESTC_PROVIDER(S_OK == m_pTable->ExecuteCommand(CREATE_PROC, IID_NULL,
		pwszProcName,NULL,NULL, NULL, EXECUTE_IFNOERROR, 0, NULL, 
		NULL, NULL, NULL));

	{
		CDBID dbIndexID(DBKIND_NAME,  pwszProcName);

		TESTC_(AlterIndex(&dbIndexID, &dbidNew, FALSE), DB_E_NOINDEX);
	}

CLEANUP:

	// Drop the stored proc we created above. Note it might not exist.
	m_pTable->ExecuteCommand(DROP_PROC, IID_NULL,
		pwszProcName,NULL,NULL, NULL, EXECUTE_IFNOERROR, 0, NULL, 
		NULL, NULL, NULL);

	SAFE_FREE(pwszProcName);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOINDEX - Pass valid index from different table as existing index name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_31()
{ 
	TBEGIN
	CDBID		dbidNew(DBKIND_NAME,  L"Cello");
	DBID *		pIndexID = NULL;

	// Create an index on a different table
	TESTC_(CreateIndex(&(m_pNoNullTable->GetTableID()),
		NULL, m_cIndexColumnDesc,m_pIndexColumnDesc, 0, NULL,
		&pIndexID), S_OK);

	TESTC_(AlterIndex(pIndexID, &dbidNew, FALSE), DB_E_NOINDEX);

CLEANUP:

	// Drop the index on the other table
	TESTC_(m_pIIndexDef->DropIndex(&(m_pNoNullTable->GetTableID()), pIndexID), S_OK); 

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED - Alter second index on table to be duplicate primary key
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_32()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, BuildValidName(m_cMaxIndexName, L"XYZ"));
	DBID * pTableID = &(m_pNoNullTable->GetTableIDRef());
	DBCOLUMNDESC * pColumnDesc = NULL;
	DBPROPSET * pColPropSets = NULL;
	ULONG cColPropSets = 0;
	DBORDINAL cCols = m_pTable->CountColumnsOnTable();	
	ULONG iCol;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBID * pIndexID = NULL;
	CTable NewTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName, NONULLS);

	// Note we could just AlterTable on the current automaketable to create a
	// PrimaryKey constraint, but at the time this was written AlterTable did not
	// properly support this.  

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, cCols);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*cCols));

	// Get the DBCOLUMNDESC information from the automaketable
	TESTC_(m_pTable->BuildColumnDescs(&pColumnDesc), S_OK);

	// Create a primary key index on the automaketable so we can tell which column
	// will support a primary key.
	TESTC(m_cPropSets == 0);
	SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	// Create the index to be altered
	TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
		pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	// Drop the index on the automaketable - we only wanted the column descs
	TESTC_(m_pIIndexDef->DropIndex(pTableID, pIndexID), S_OK); 

	// Add a PrimaryKey prop for this column for the new table
	for (iCol = 0; iCol < cCols; iCol++)
	{
		if (CompareDBID(*pIndexColumnDesc[iCol].pColumnID, pColumnDesc[iCol].dbcid))
		{
			SetProperty(DBPROP_COL_PRIMARYKEY, DBPROPSET_COLUMN,
				&pColumnDesc[iCol].cPropertySets, &pColumnDesc[iCol].rgPropertySets,
				(void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
			break;
		}
	}

	TESTC(iCol < cCols);

	// Set the column desc info for the new table
	TESTC(NewTable.SetColumnDesc(pColumnDesc, cCols) != NULL);
	NewTable.SetBuildColumnDesc(FALSE);

	// Create the new table
	TESTC_(NewTable.CreateTable(MIN_TABLE_ROWS, 0), S_OK);

	// Create an index on the new table, using the same column as above.
	TESTC_PROVIDER(S_OK == CreateIndex(&(NewTable.GetTableID()),
		NULL, m_cIndexColumnDesc,pIndexColumnDesc, 0, NULL,
		&pIndexID));

	// Call AlterIndex to set this property.
	// Since this is a duplicate PK it should fail.
	TESTC_(hr = AlterIndex(&(NewTable.GetTableID()), pIndexID, &dbidNew), DB_E_ERRORSOCCURRED)

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);

	SAFE_FREE(dbidNew.uName.pwszName);
	SAFE_FREE(pIndexColumnDesc);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED - Alter second index on table to be duplicate primary key
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBoundary_SingCol::Variation_33()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME, BuildValidName(m_cMaxIndexName, L"XYZ"));
	DBID * pTableID = &(m_pNoNullTable->GetTableIDRef());
	DBCOLUMNDESC * pColumnDesc = NULL;
	DBPROPSET * pColPropSets = NULL;
	ULONG cColPropSets = 0;
	DBORDINAL cCols = m_pTable->CountColumnsOnTable();	
	ULONG iCol;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBID * pIndexID = NULL;
	CTable NewTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName, NONULLS);

	// Note we could just AlterTable on the current automaketable to create a
	// PrimaryKey constraint, but at the time this was written AlterTable did not
	// properly support this.  

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, cCols);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*cCols));

	// Get the DBCOLUMNDESC information from the automaketable
	TESTC_(m_pTable->BuildColumnDescs(&pColumnDesc), S_OK);

	// Create a primary key index on the automaketable so we can tell which column
	// will support a primary key.
	TESTC(m_cPropSets == 0);
	SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	// Create the index to be altered
	TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
		pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	// Drop the index on the automaketable - we only wanted the column descs
	TESTC_(m_pIIndexDef->DropIndex(pTableID, pIndexID), S_OK); 

	// Add a PrimaryKey prop for this column for the new table
	for (iCol = 0; iCol < cCols; iCol++)
	{
		if (CompareDBID(*pIndexColumnDesc[iCol].pColumnID, pColumnDesc[iCol].dbcid))
		{
			SetProperty(DBPROP_COL_PRIMARYKEY, DBPROPSET_COLUMN,
				&pColumnDesc[iCol].cPropertySets, &pColumnDesc[iCol].rgPropertySets,
				(void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
			break;
		}
	}

	TESTC(iCol < cCols);

	// Set the column desc info for the new table
	TESTC(NewTable.SetColumnDesc(pColumnDesc, cCols) != NULL);
	NewTable.SetBuildColumnDesc(FALSE);

	// Create the new table
	TESTC_(NewTable.CreateTable(MIN_TABLE_ROWS, 0), S_OK);

	// Reset the prop to optional
	m_rgPropSets[0].rgProperties[0].dwOptions = DBPROPOPTIONS_OPTIONAL;

	// Create an index on the new table, using the same column as above.
	TESTC_PROVIDER(S_OK == CreateIndex(&(NewTable.GetTableID()),
		NULL, m_cIndexColumnDesc,pIndexColumnDesc, 0, NULL,
		&pIndexID));

	// Call AlterIndex to set this property.
	// Since this is a duplicate PK it should fail.
	TESTC_(hr = AlterIndex(&(NewTable.GetTableID()), pIndexID, &dbidNew), DB_S_ERRORSOCCURRED)

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);

	SAFE_FREE(dbidNew.uName.pwszName);
	SAFE_FREE(pIndexColumnDesc);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBoundary_SingCol::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAlterIndex::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

// {{ TCW_TC_PROTOTYPE(TCProp_SingCol)
//*-----------------------------------------------------------------------
//| Test Case:		TCProp_SingCol - Property related tests
//| Created:  	8/25/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCProp_SingCol::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAlterIndex::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_AUTOUPDATE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_1()
{ 
	TBEGIN
	HRESULT		hr;
	CIndexInfo  IndexInfo;
	VARIANT_BOOL vbAutoUpdate = 100;
	ULONG iVal;

	// Get information about this index.
	TESTC(IndexInfo.Init(m_pIOpenRowset, &(m_pTable->GetTableID()), m_pIndexID));

	// Read current autoupdate value for first column of index (0 based).
	vbAutoUpdate = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_AUTO_UPDATE);

	for (iVal = 0; iVal < NUMELEM(VariantBoolVals); iVal++)
	{
		//Set prop.
		SetProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

		// If this val matches the initial value it should always succeed
		if (VariantBoolVals[iVal] == vbAutoUpdate)
		{
			// Try to alter with this prop 
			TESTC_(hr = AlterIndex(m_pIndexID, NULL), S_OK)
		}
		else
		{
			// Try to alter with this prop 
			TEST2C_(hr = AlterIndex(m_pIndexID, NULL), S_OK, DB_E_ERRORSOCCURRED)
		}

		// Verify the prop status based on settability, etc.
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX, DBPROPSTATUS_OK))

		// If prop is VARIANT_TRUE, verify the index is auto-updatable
		// VerifyAutoUpdate(hr, m_pTableID, m_pIndexID);

		FreeProperties(&m_cPropSets, &m_rgPropSets);
	}

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_CLUSTERED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_2()
{ 
	TBEGIN
	HRESULT		hr;
	CIndexInfo  IndexInfo;
	VARIANT_BOOL vbClustered = 100;
	ULONG iVal;

	// Get information about this index.
	TESTC(IndexInfo.Init(m_pIOpenRowset, &(m_pTable->GetTableID()), m_pIndexID));

	// Read current autoupdate value for first column of index (0 based).
	vbClustered = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_CLUSTERED);

	for (iVal = 0; iVal < NUMELEM(VariantBoolVals); iVal++)
	{
		//Set prop.
		SetProperty(DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

		// If this val matches the initial value it should always succeed
		if (VariantBoolVals[iVal] == vbClustered)
		{
			// Try to alter with this prop 
			TESTC_(hr = AlterIndex(m_pIndexID, NULL), S_OK)
		}
		else
		{
			// Try to alter with this prop 
			TEST2C_(hr = AlterIndex(m_pIndexID, NULL), S_OK, DB_E_ERRORSOCCURRED)
		}

		// Verify the prop status based on settability, etc.
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_CLUSTERED, DBPROPSET_INDEX, DBPROPSTATUS_OK))

		// Verify the index is clustered.  Since verification is very provider-specific we
		// won't attempt at this time.
		// VerifyClustered(hr, m_pTableID, m_pIndexID);


		FreeProperties(&m_cPropSets, &m_rgPropSets);
	}


CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_NULLS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_3()
{ 
	TBEGIN
	HRESULT		hr, hrCreateIndex = S_OK;
	ULONG		iVal, iCreateVal;
	ULONG		ulCurrentValue = DBPROPVAL_IN_ALLOWNULL;
	ULONG		cPropSets = 0;
	DBPROPSET * pPropSets = NULL;
	DBPROPSTATUS stExpected = DBPROPSTATUS_OK;
	DBID *		pTableID = &(m_pTable->GetTableIDRef());
	CIndexInfo  IndexInfo;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	// Because of Jolt behavior wrt setting only to default value with
	// AlterIndex we need to loop through all values of the property and attempt
	// to create an index with that property value first.
	for (iCreateVal = 0; iCreateVal < NUMELEM(IndexNullVals); iCreateVal++)
	{
		// Set the property values for index creation
		SetProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, &cPropSets, &pPropSets, (void *)(ULONG_PTR)IndexNullVals[iCreateVal].ulPropVal, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

		// Create an index with this property set to the proper value.  We assume
		// failure to support the property value is valid and tested in IIndexDef
		// test, so just allow failure.
		hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
			pIndexColumnDesc, cPropSets, pPropSets, &m_pIndexID);

		while(hrCreateIndex == S_OK)
		{

			// Get information about this index.
			TESTC(IndexInfo.Init(m_pIOpenRowset, pTableID, m_pIndexID));

			// Read current INDEX_NULLS value for first column of index (0 based).
			ulCurrentValue = *(ULONG *)IndexInfo.GetIndexValuePtr(0, IS_NULLS);

			// If index creation succeeded we'd better have the value we asked for
			TESTC(ulCurrentValue == IndexNullVals[iCreateVal].ulPropVal);
			
			for (iVal = 0; iVal < NUMELEM(IndexNullVals); iVal++)
			{
				// For some providers AlterIndex can only set a property to it's current value
				// so set expected prop status appropriately
				if (ulCurrentValue != IndexNullVals[iVal].ulPropVal && m_fSetOnlyDefault)
					stExpected = DBPROPSTATUS_NOTSETTABLE;
				else
					stExpected = DBPROPSTATUS_OK;

				//Set prop as required.
				SetProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)(ULONG_PTR)IndexNullVals[iVal].ulPropVal, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

				// Try to alter with this prop 
				hr = AlterIndex(m_pIndexID, NULL);

				// Allow DB_E_ERRORSOCCURRED or S_OK
				if (hr != DB_E_ERRORSOCCURRED)
					CHECK(hr, S_OK);

				// Verify the prop status based on settability, etc.
				COMPARE(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_NULLS, DBPROPSET_INDEX, stExpected), TRUE);

				// Verify the index null handling
				VerifyNulls(hr,
							m_pTable,
							m_pIndexID,
							IndexNullVals[iVal].ulPropVal,
							m_cIndexColumnDesc,
							pIndexColumnDesc
				);

				FreeProperties(&m_cPropSets, &m_rgPropSets);

			} // Next alteration value

			// Drop the index we created
			CHECK(m_pIIndexDef->DropIndex(pTableID, m_pIndexID), S_OK); 

			// Create the next possible index
			hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
				pIndexColumnDesc, cPropSets, pPropSets, &m_pIndexID, TRUE);

		} // Next supported index column set

		// Make sure we got a valid error return from CreateIndex
		// If the provider doesn't support property -> DB_E_ERRORSOCCURRED
		// If other error occurs (duplicate column in index) -> E_FAIL
		// The IIndexDef test should be checking for these.
		if (hrCreateIndex != DB_E_ERRORSOCCURRED)
			CHECK(hrCreateIndex, E_FAIL);

		// Reset hrCreateIndex for next prop val
		hrCreateIndex = S_OK;

		// Free the previous prop values
		FreeProperties(&cPropSets, &pPropSets);

		// Reset the index col DBIDs. These are pointers to the CCol DBID
		// so we don't want to release them.
		memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	} // Next index creation value

CLEANUP:
	
	SAFE_FREE(pIndexColumnDesc);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_PRIMARYKEY
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_4()
{ 
	TBEGIN
	HRESULT		hr, hrCreateIndex = S_OK;
	ULONG		iVal, iCreateVal;
	ULONG		cIndexes = 0;
	VARIANT_BOOL vbCurrentValue = VARIANT_FALSE;
	ULONG		cPropSets = 0;
	DBPROPSET * pPropSets = NULL;
	DBPROPSTATUS stExpected = DBPROPSTATUS_OK;
	DBID *		pTableID = &(m_pNoNullTable->GetTableIDRef());
	DBID 		IndexID = DB_NULLID;
	DBID *		pIndexID = &IndexID;
	CIndexInfo  IndexInfo;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	// Because of provider behavior wrt setting only to default value with
	// AlterIndex we need to loop through all values of the property and attempt
	// to create an index with that property value first.
	for (iCreateVal = 0; iCreateVal < NUMELEM(VariantBoolVals); iCreateVal++)
	{
		// Set the property values for index creation
		SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &cPropSets, &pPropSets, (void*)VariantBoolVals[iCreateVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

		// Create an index with this property set to the proper value.  We assume
		// failure to support the property value is valid and tested in IIndexDef
		// test, so just allow failure.
		hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
			pIndexColumnDesc, cPropSets, pPropSets, &pIndexID);

		cIndexes = 0;

		while(hrCreateIndex == S_OK)
		{
			cIndexes++;

			// Get information about this index.
			TESTC(IndexInfo.Init(m_pIOpenRowset, pTableID, pIndexID));

			// Read current UNIQUE value for first column of index (0 based).
			vbCurrentValue = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_PRIMARY_KEY);

			// If index creation succeeded we'd better have the value we asked for
			TESTC(vbCurrentValue == VariantBoolVals[iCreateVal]);
			
			for (iVal = 0; iVal < NUMELEM(VariantBoolVals); iVal++)
			{
				// For some providers AlterIndex can only set a property to it's current value
				// so set expected prop status appropriately
				if (vbCurrentValue != VariantBoolVals[iVal] && m_fSetOnlyDefault)
					stExpected = DBPROPSTATUS_NOTSETTABLE;
				else
					stExpected = DBPROPSTATUS_OK;

				//Set prop as required.
				SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

				// Try to alter with this prop 
				hr = AlterIndex(pTableID, pIndexID, (DBID *)NULL);

				// Allow DB_E_ERRORSOCCURRED or S_OK
				if (hr != DB_E_ERRORSOCCURRED)
					CHECK(hr, S_OK);

				// Verify the prop status based on settability, etc.
				COMPARE(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, stExpected), TRUE);

				// Verify the primary key
				VerifyPrimaryKey(hr,
							m_pNoNullTable,
							pIndexID,
							VariantBoolVals[iVal],
							m_cIndexColumnDesc,
							pIndexColumnDesc
				);

				FreeProperties(&m_cPropSets, &m_rgPropSets);

			} // Next alteration value

			// Drop the index we created
			CHECK(m_pIIndexDef->DropIndex(pTableID, pIndexID), S_OK); 

			// Only test the maximum index count to speed up testing when using multipart
			// keys
			if (cIndexes >= MAX_INDEX_COUNT)
			{
				hrCreateIndex = E_FAIL;
				break;
			}

			ReleaseDBID(pIndexID, TRUE);

			// Create the next possible index
			hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
				pIndexColumnDesc, cPropSets, pPropSets, &pIndexID, TRUE);

		} // Next supported index column set

		// Make sure we got a valid error return from CreateIndex
		// If the provider doesn't support property -> DB_E_ERRORSOCCURRED
		// If other error occurs (duplicate column in index) -> E_FAIL
		// The IIndexDef test should be checking for these.
		if (hrCreateIndex != DB_E_ERRORSOCCURRED)
			CHECK(hrCreateIndex, E_FAIL);

		// Reset hrCreateIndex for next prop val
		hrCreateIndex = S_OK;

		FreeProperties(&cPropSets, &pPropSets);

		// Reset the index col DBIDs. These are pointers to the CCol DBID
		// so we don't want to release them.
		memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	} // Next index creation value

CLEANUP:

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	SAFE_FREE(pIndexColumnDesc);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_SORTBOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_5()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME,  MakeObjectName(L"IAlterIn", m_cMaxIndexName));
	
	CHECK_MEMORY(dbidNew.uName.pwszName);

	//Set prop as required.
	SetProperty(DBPROP_INDEX_SORTBOOKMARKS, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_SORTBOOKMARKS, DBPROPSET_INDEX, DBPROPSTATUS_OK))

	// Verify the index bookmark behavior.  At this time I don't see a lot of utility
	// in verifying this for the amount of work required.
	// VerifySortBookmarks(hr, m_pTableID, m_pIndexID);


CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	ReleaseDBID(&dbidNew, FALSE);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_TEMPINDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_6()
{ 
	TBEGIN
	HRESULT		hr, hrCreateIndex = S_OK;
	ULONG		iVal, iCreateVal;
	ULONG		cIndexes = 0;
	VARIANT_BOOL vbCurrentValue = VARIANT_FALSE;
	ULONG		cPropSets = 0;
	DBPROPSET * pPropSets = NULL;
	DBPROPSTATUS stExpected = DBPROPSTATUS_OK;
	DBID *		pTableID = &(m_pTable->GetTableIDRef());
	CIndexInfo  IndexInfo;
	IUnknown * pSessionIUnknown = NULL;
	BOOL fExists = FALSE;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	// Save the existing IAlterIndex object.  Note session object for IIndexDefinition
	// is unaltered, thus index is created by a different session than we alter it with.
	pSessionIUnknown = m_pIAlterIndex;
	m_pIAlterIndex = NULL;

	// Because of Jolt behavior wrt setting only to default value with
	// AlterIndex we need to loop through all values of the property and attempt
	// to create an index with that property value first.
	for (iCreateVal = 0; iCreateVal < NUMELEM(VariantBoolVals); iCreateVal++)
	{
		// Set the property values for index creation
		SetProperty(DBPROP_INDEX_TEMPINDEX, DBPROPSET_INDEX, &cPropSets, &pPropSets, (void*)VariantBoolVals[iCreateVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

		// Create an index with this property set to the proper value.  We assume
		// failure to support the property value is valid and tested in IIndexDef
		// test, so just allow failure.
		hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
			pIndexColumnDesc, cPropSets, pPropSets, &m_pIndexID);

		cIndexes = 0;

		while(hrCreateIndex == S_OK)
		{
			cIndexes++;

			// Get information about this index.
			TESTC(IndexInfo.Init(m_pIOpenRowset, pTableID, m_pIndexID));

			// Set current TEMPINDEX value
			vbCurrentValue = VariantBoolVals[iCreateVal];

			for (iVal = 0; iVal < NUMELEM(VariantBoolVals); iVal++)
			{
				// Create a new session object we can release below
				if (!m_pIAlterIndex)
					TESTC_(GetSessionObject(IID_IAlterIndex, (IUnknown **)&m_pIAlterIndex), S_OK);

				// For some providers AlterIndex can only set a property to it's current value
				// so set expected prop status appropriately
				if (vbCurrentValue != VariantBoolVals[iVal] && m_fSetOnlyDefault)
					stExpected = DBPROPSTATUS_NOTSETTABLE;
				else
					stExpected = DBPROPSTATUS_OK;

				//Set prop as required.
				SetProperty(DBPROP_INDEX_TEMPINDEX, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

				// Try to alter with this prop 
				hr = AlterIndex(m_pIndexID, NULL);

				// Allow DB_E_ERRORSOCCURRED or S_OK
				if (hr != DB_E_ERRORSOCCURRED)
					CHECK(hr, S_OK);

				// Verify the prop status based on settability, etc.
				COMPARE(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_TEMPINDEX, DBPROPSET_INDEX, stExpected), TRUE);

				// This should drop the temp index if it's a temp index
				SAFE_RELEASE(m_pIAlterIndex);

				// Find out if the index exists
				TESTC_(DoesIndexExist(pTableID, m_pIndexID, &fExists), S_OK);

				// Verify temp index
				if (SUCCEEDED(hr) && VariantBoolVals[iVal] == VARIANT_TRUE)
				{
					if (COMPARE(fExists, FALSE))
					{
						// We must recreate index for next alteration value
						hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
							pIndexColumnDesc, cPropSets, pPropSets, &m_pIndexID);

						// This must always succeed, since it succeeded previously
						TESTC_(hrCreateIndex, S_OK);
					}
				}
				else
					// IAlterIndex failed, or the index is not a temp index, must exist
					COMPARE(fExists, TRUE);

				FreeProperties(&m_cPropSets, &m_rgPropSets);

			} // Next alteration value

			// Drop the index we created
			CHECK(m_pIIndexDef->DropIndex(pTableID, m_pIndexID), S_OK); 

			// Only test the maximum index count to speed up testing when using multipart
			// keys
			if (cIndexes >= MAX_INDEX_COUNT)
			{
				hrCreateIndex = E_FAIL;
				break;
			}

			// Create the next possible index
			hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
				pIndexColumnDesc, cPropSets, pPropSets, &m_pIndexID, TRUE);

		} // Next supported index column set

		// Make sure we got a valid error return from CreateIndex
		// If the provider doesn't support property -> DB_E_ERRORSOCCURRED
		// If other error occurs (duplicate column in index) -> E_FAIL
		// The IIndexDef test should be checking for these.
		if (hrCreateIndex != DB_E_ERRORSOCCURRED)
			CHECK(hrCreateIndex, E_FAIL);

		// Reset hrCreateIndex for next prop val
		hrCreateIndex = S_OK;

		FreeProperties(&cPropSets, &pPropSets);

		// Reset the index col DBIDs. These are pointers to the CCol DBID
		// so we don't want to release them.
		memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	} // Next index creation value

CLEANUP:

	SAFE_RELEASE(m_pIAlterIndex);
	m_pIAlterIndex = (IAlterIndex *)pSessionIUnknown;

	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_TYPE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_7()
{ 
	TBEGIN
	HRESULT		hr;
	ULONG		iVal;

	// TODO: Get the current value of the prop so we can determine setting r/o
	// prop to current value is S_OK.  Actually, due to behavior of some providers,
	// we need to attempt to create an index of each type and then alter to every
	// other type.

	for (iVal = 0; iVal < NUMELEM(IndexTypeVals); iVal++)
	{
		//Set prop as required.
		SetProperty(DBPROP_INDEX_TYPE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)(ULONG_PTR)IndexTypeVals[iVal].ulPropVal, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

		// Try to alter with this prop 
		hr = AlterIndex(m_pIndexID, NULL);

		// Allow DB_E_ERRORSOCCURRED or S_OK
		if (hr != DB_E_ERRORSOCCURRED)
			CHECK(hr, S_OK);

		if (S_OK == hr)
			odtLog << IndexTypeVals[iVal].pwszPropVal << ":\tSUPPORTED \n";
		else
			odtLog << IndexTypeVals[iVal].pwszPropVal << ":\tUNSUPPORTED \n";

		// Verify the prop status based on settability, etc.
		COMPARE(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_TYPE, DBPROPSET_INDEX, DBPROPSTATUS_OK), TRUE);

		// Verify the type.  This looks pretty provider-specific, so we won't test here.

		// Free props for the next time
		FreeProperties(&m_cPropSets, &m_rgPropSets);
	}

	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_UNIQUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_8()
{ 
	TBEGIN
	HRESULT		hr, hrCreateIndex = S_OK;
	ULONG		iVal, iCreateVal;
	ULONG		cIndexes = 0;
	VARIANT_BOOL vbCurrentValue = VARIANT_FALSE;
	ULONG		cPropSets = 0;
	DBPROPSET * pPropSets = NULL;
	DBPROPSTATUS stExpected = DBPROPSTATUS_OK;
	DBID *		pNewIndexID = NULL;
	DBID *		pTableID = &(m_pTable->GetTableIDRef());
	CIndexInfo  IndexInfo;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	// Because of Jolt behavior wrt setting only to default value with
	// AlterIndex we need to loop through all values of the property and attempt
	// to create an index with that property value first.
	for (iCreateVal = 0; iCreateVal < NUMELEM(VariantBoolVals); iCreateVal++)
	{
		// Set the property values for index creation
		SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &cPropSets, &pPropSets, (void*)VariantBoolVals[iCreateVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

		// Create an index with this property set to the proper value.  We assume
		// failure to support the property value is valid and tested in IIndexDef
		// test, so just allow failure.
		hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
			pIndexColumnDesc, cPropSets, pPropSets, &pNewIndexID);

		cIndexes = 0;

		while(hrCreateIndex == S_OK)
		{
			cIndexes++;

			// Get information about this index.
			TESTC(IndexInfo.Init(m_pIOpenRowset, pTableID, pNewIndexID));

			// Read current UNIQUE value for first column of index (0 based).
			vbCurrentValue = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_UNIQUE);

			// If index creation succeeded we'd better have the value we asked for
			TESTC(vbCurrentValue == VariantBoolVals[iCreateVal]);
			
			for (iVal = 0; iVal < NUMELEM(VariantBoolVals); iVal++)
			{
				// For some providers AlterIndex can only set a property to it's current value
				// so set expected prop status appropriately
				if (vbCurrentValue != VariantBoolVals[iVal] && m_fSetOnlyDefault)
					stExpected = DBPROPSTATUS_NOTSETTABLE;
				else
					stExpected = DBPROPSTATUS_OK;

				//Set prop as required.
				SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

				// Try to alter with this prop 
				hr = AlterIndex(pNewIndexID, NULL);

				// Allow DB_E_ERRORSOCCURRED or S_OK
				if (hr != DB_E_ERRORSOCCURRED)
					CHECK(hr, S_OK);

				// Verify the prop status based on settability, etc.
				COMPARE(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, stExpected), TRUE);

				// Verify the unique handling
				VerifyUnique(hr,
							m_pTable,
							pNewIndexID,
							VariantBoolVals[iVal],
							m_cIndexColumnDesc,
							pIndexColumnDesc
				);

				FreeProperties(&m_cPropSets, &m_rgPropSets);

			} // Next alteration value

			// Drop the index we created
			CHECK(m_pIIndexDef->DropIndex(pTableID, pNewIndexID), S_OK); 

			ReleaseDBID(pNewIndexID, TRUE);

			pNewIndexID = NULL;

			// Only test the maximum index count to speed up testing when using multipart
			// keys
			if (cIndexes >= MAX_INDEX_COUNT)
			{
				hrCreateIndex = E_FAIL;
				break;
			}

			// Create the next possible index
			hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
				pIndexColumnDesc, cPropSets, pPropSets, &pNewIndexID, TRUE);

		} // Next supported index column set

		// Make sure we got a valid error return from CreateIndex
		// If the provider doesn't support property -> DB_E_ERRORSOCCURRED
		// If other error occurs (duplicate column in index) -> E_FAIL
		// The IIndexDef test should be checking for these.
		if (hrCreateIndex != DB_E_ERRORSOCCURRED)
			CHECK(hrCreateIndex, E_FAIL);

		// Reset hrCreateIndex for next prop val
		hrCreateIndex = S_OK;

		FreeProperties(&cPropSets, &pPropSets);

		// Reset the index col DBIDs. These are pointers to the CCol DBID
		// so we don't want to release them.
		memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	} // Next index creation value

CLEANUP:

	SAFE_FREE(pIndexColumnDesc);
	FreeProperties(&cPropSets, &pPropSets);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	ReleaseDBID(pNewIndexID, TRUE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_FILLFACTOR
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_9()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME,  MakeObjectName(L"IAlterIn", m_cMaxIndexName));
	DBPROPSTATUS dbpropstat = DBPROPSTATUS_OK;
	
	CHECK_MEMORY(dbidNew.uName.pwszName);

	//Set prop as required.
	//TODO: Note this prop has valid values 1-100 for B+ tree index.  Need to test 
	//boundary cases also.  And for linear hash index we're not sure what values are
	//valid, needs more investigation.

	// Set to max (100)
	SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)100, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TEST2C_(hr = AlterIndex(m_pIndexID, &dbidNew), S_OK, DB_E_ERRORSOCCURRED)

	Refresh_m_pIndexID(hr, dbidNew);

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, DBPROPSTATUS_OK))

	// Looks like verifying fill factor will be very provider-specific and so we won't
	// attempt here.

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	// Set to min (1)
	SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)1, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TEST2C_(hr = AlterIndex(m_pIndexID, NULL), S_OK, DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, DBPROPSTATUS_OK))

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	// If we successfully set the prop above, then it's supported, therefore we should get
	// BADVALUE for the following.
	if (S_OK == hr)
		dbpropstat = DBPROPSTATUS_BADVALUE;

	// Set to invalid (101)
	SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)101, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TESTC_(hr = AlterIndex(m_pIndexID, NULL), DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, dbpropstat))

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	// Set to invalid (0)
	SetProperty(DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)0, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TESTC_(hr = AlterIndex(m_pIndexID, NULL), DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_FILLFACTOR, DBPROPSET_INDEX, dbpropstat))

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	ReleaseDBID(&dbidNew, FALSE);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_INITIALSIZE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_10()
{ 
	TBEGIN
	HRESULT		hr;
	DBPROPSTATUS dbpropstat = DBPROPSTATUS_OK;

	//Set prop as required.  INITIALSIZE is the number of bytes allocated for the index
	// structure.  We should try 0, 1, and some large size.  This is very provider
	// specific.

	// Set to max (10K)???  TODO:  How do I know what the max size is?  Do I really want
	// to set a "max" size, as there may be limits such as it has to fit in available memory
	// or disk space. I chose 10K as a "moderate" size likely to be supported.
	SetProperty(DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)10000, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TEST2C_(hr = AlterIndex(m_pIndexID, NULL), S_OK, DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX, DBPROPSTATUS_OK))

	// Looks like verifying initialsize will be very provider-specific and so we won't
	// attempt here.

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	// If we successfully set the prop above, then it's supported, therefore we should get
	// BADVALUE for the following.
	if (S_OK == hr)
		dbpropstat = DBPROPSTATUS_BADVALUE;

	// Set to min (0).  May or may not be supported.
	SetProperty(DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)0, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TEST2C_(hr = AlterIndex(m_pIndexID, NULL), S_OK, DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX, dbpropstat))

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	// Set to value (1). May or may not be supported.
	SetProperty(DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)1, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TESTC_(hr = AlterIndex(m_pIndexID, NULL), DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX, dbpropstat))

	FreeProperties(&m_cPropSets, &m_rgPropSets);

	// Set to invalid (ULONG_MAX)
	SetProperty(DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)(ULONG_PTR)ULONG_MAX, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TESTC_(hr = AlterIndex(m_pIndexID, NULL), DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_INITIALSIZE, DBPROPSET_INDEX, dbpropstat))

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_NULLCOLLATION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_11()
{ 
	TBEGIN
	HRESULT		hr, hrCreateIndex = S_OK;
	ULONG		iVal, iCreateVal, iOrder, cCreate=0;
	ULONG		ulCurrentValue = DBPROPVAL_NC_END;
	ULONG		cPropSets = 0;
	DBPROPSET * pPropSets = NULL;
	DBPROPSTATUS stExpected = DBPROPSTATUS_OK;
	DBID *		pTableID = &(m_pTable->GetTableIDRef());
	CIndexInfo  IndexInfo;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	// Because of Jolt behavior wrt setting only to default value with
	// AlterIndex we need to loop through all values of the property and attempt
	// to create an index with that property value first.
	for (iCreateVal = 0; iCreateVal < NUMELEM(NullCollationVals); iCreateVal++)
	{
		// Set the property values for index creation
		SetProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, &cPropSets, &pPropSets, (void*)DBPROPVAL_IN_ALLOWNULL, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);
		SetProperty(DBPROP_INDEX_NULLCOLLATION, DBPROPSET_INDEX, &cPropSets, &pPropSets, (void*)(ULONG_PTR)NullCollationVals[iCreateVal].ulPropVal, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

		// For each possible index column ordering
		for (iOrder = 0; iOrder < NUMELEM(IndexOrderVals); iOrder++)
		{
		
			// Create an index with this property set to the proper value.  We assume
			// failure to support the property value is valid and tested in IIndexDef
			// test, so just allow failure.
			hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
				pIndexColumnDesc, cPropSets, pPropSets, &m_pIndexID, FALSE, TRUE,
				IndexOrderVals[iOrder]);

			while(hrCreateIndex == S_OK)
			{
				cCreate++;

				// Get information about this index.
				TESTC(IndexInfo.Init(m_pIOpenRowset, pTableID, m_pIndexID));

				// Read current INDEX_NULLS value for first column of index (0 based).
				ulCurrentValue = *(ULONG *)IndexInfo.GetIndexValuePtr(0, IS_NULL_COLLATION);

				// If index creation succeeded we'd better have the value we asked for
				TESTC(ulCurrentValue == NullCollationVals[iCreateVal].ulPropVal);
				
				for (iVal = 0; iVal < NUMELEM(IndexNullVals); iVal++)
				{
					// For some providers AlterIndex can only set a property to it's current value
					// so set expected prop status appropriately
					if (ulCurrentValue != NullCollationVals[iVal].ulPropVal && m_fSetOnlyDefault)
						stExpected = DBPROPSTATUS_NOTSETTABLE;
					else
						stExpected = DBPROPSTATUS_OK;

					//Set prop as required.
					SetProperty(DBPROP_INDEX_NULLCOLLATION, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)(ULONG_PTR)NullCollationVals[iVal].ulPropVal, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

					// Try to alter with this prop 
					hr = AlterIndex(m_pIndexID, NULL);

					// Allow DB_E_ERRORSOCCURRED or S_OK
					if (hr != DB_E_ERRORSOCCURRED)
						CHECK(hr, S_OK);

					// Verify the prop status based on settability, etc.
					COMPARE(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_NULLCOLLATION, DBPROPSET_INDEX, stExpected), TRUE);

					// Verify the index null handling
					VerifyNullCollation(hr,
								m_pTable,
								m_pIndexID,
								NullCollationVals[iVal].ulPropVal,
								m_cIndexColumnDesc,
								pIndexColumnDesc
					);

					FreeProperties(&m_cPropSets, &m_rgPropSets);

				} // Next alteration value

				// Drop the index we created
				CHECK(m_pIIndexDef->DropIndex(pTableID, m_pIndexID), S_OK); 

				// Create the next possible index
				hrCreateIndex = CreateIndex(pTableID, NULL, m_cIndexColumnDesc,
					pIndexColumnDesc, cPropSets, pPropSets, &m_pIndexID, TRUE,
					TRUE, IndexOrderVals[iOrder]);

			} // Next supported index column set

		} // Next col order value

		// Make sure we got a valid error return from CreateIndex
		// If the provider doesn't support property -> DB_E_ERRORSOCCURRED
		// If other error occurs (duplicate column in index) -> E_FAIL
		// The IIndexDef test should be checking for these.
		if (hrCreateIndex != DB_E_ERRORSOCCURRED)
			CHECK(hrCreateIndex, E_FAIL);

		// Reset hrCreateIndex for next prop val
		hrCreateIndex = S_OK;

		FreeProperties(&cPropSets, &pPropSets);

		// Reset the index col DBIDs. These are pointers to the CCol DBID
		// so we don't want to release them.
		memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	} // Next index creation value

CLEANUP:

	if (!cCreate)
	{
		odtLog << L"Unable to create any indexes to test this property.\n";
		TESTB = TEST_SKIPPED;
	}

	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_SORTBOOKMARKS and DBPROP_INDEX_UNIQUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_12()
{ 
	TBEGIN
	HRESULT		hr;
	CIndexInfo  IndexInfo;
	VARIANT_BOOL vbUnique = 100;
	VARIANT_BOOL vbSort = 100;
	ULONG iUniqueVal, iSortVal;
	DBPROPSTATUS stUnique, stSort;

	// Get information about this index.
	TESTC(IndexInfo.Init(m_pIOpenRowset, &(m_pTable->GetTableID()), m_pIndexID));

	// Read current autoupdate value for first column of index (0 based).
	vbUnique = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_UNIQUE);
	vbSort = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_SORT_BOOKMARKS);

	for (iUniqueVal = 0; iUniqueVal < NUMELEM(VariantBoolVals); iUniqueVal++)
	{
		for (iSortVal = 0; iSortVal < NUMELEM(VariantBoolVals); iSortVal++)
		{
			// For some providers AlterIndex can only set a property to it's current value
			// so set expected prop status appropriately
			if (vbUnique != VariantBoolVals[iUniqueVal] && m_fSetOnlyDefault)
				stUnique = DBPROPSTATUS_NOTSETTABLE;
			else
				stUnique = DBPROPSTATUS_OK;

			if (vbSort != VariantBoolVals[iSortVal] && m_fSetOnlyDefault)
				stSort = DBPROPSTATUS_NOTSETTABLE;
			else
				stSort = DBPROPSTATUS_OK;

			//Set prop.
			SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iUniqueVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
			SetProperty(DBPROP_INDEX_SORTBOOKMARKS, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iSortVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

			// If this val matches the initial values it should always succeed
			if (VariantBoolVals[iUniqueVal] == vbUnique &&
				VariantBoolVals[iSortVal] == vbSort)
			{
				// Try to alter with this prop 
				TESTC_(hr = AlterIndex(m_pIndexID, NULL), S_OK)
			}
			else
			{
				// Try to alter with this prop 
				TEST2C_(hr = AlterIndex(m_pIndexID, NULL), S_OK, DB_E_ERRORSOCCURRED)
			}

			// Verify the prop status based on settability, etc.
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, stUnique))
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_SORTBOOKMARKS, DBPROPSET_INDEX, stSort))

			// If prop is VARIANT_TRUE, verify the index works as requested.

			// Since we already verify this for the single prop case and it's kinda
			// slow we'll leave commented out until code review time.  I'd like to
			// remove this if we agree it's redundant.

			// VerifyUnique(hr, m_pTableID, m_pIndexID);
			// VerifySortBookmarks(hr, m_pTable, m_pIndexID); 

			FreeProperties(&m_cPropSets, &m_rgPropSets);
		}
	}

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_UNIQUE and DBPROP_INDEX_PRIMARYKEY
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_13()
{ 
	TBEGIN
	HRESULT		hr;
	CIndexInfo  IndexInfo;
	VARIANT_BOOL vbUnique = 100;
	VARIANT_BOOL vbPrimary = 100;
	ULONG iUniqueVal, iPrimaryVal;
	DBPROPSTATUS stUnique, stPrimary;

	// Get information about this index.
	TESTC(IndexInfo.Init(m_pIOpenRowset, &(m_pTable->GetTableID()), m_pIndexID));

	// Read current autoupdate value for first column of index (0 based).
	vbUnique = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_UNIQUE);
	vbPrimary = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_PRIMARY_KEY);

	for (iUniqueVal = 0; iUniqueVal < NUMELEM(VariantBoolVals); iUniqueVal++)
	{
		for (iPrimaryVal = 0; iPrimaryVal < NUMELEM(VariantBoolVals); iPrimaryVal++)
		{
			// For some providers AlterIndex can only set a property to it's current value
			// so set expected prop status appropriately
			if (vbUnique != VariantBoolVals[iUniqueVal] && m_fSetOnlyDefault)
				stUnique = DBPROPSTATUS_NOTSETTABLE;
			else
				stUnique = DBPROPSTATUS_OK;

			if (vbPrimary != VariantBoolVals[iPrimaryVal] && m_fSetOnlyDefault)
				stPrimary = DBPROPSTATUS_NOTSETTABLE;
			else
				stPrimary = DBPROPSTATUS_OK;

			//Set prop.
			SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iUniqueVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
			SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iPrimaryVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

			// If this val matches the initial values it should always succeed
			if (VariantBoolVals[iUniqueVal] == vbUnique &&
				VariantBoolVals[iPrimaryVal] == vbPrimary)
			{
				// Try to alter with this prop 
				TESTC_(hr = AlterIndex(m_pIndexID, NULL), S_OK)
			}
			else
			{
				// Try to alter with this prop 
				TEST2C_(hr = AlterIndex(m_pIndexID, NULL), S_OK, DB_E_ERRORSOCCURRED)
			}

			// Verify the prop status based on settability, etc.
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, stUnique))
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, stPrimary))

			// If prop is VARIANT_TRUE, verify the index works as requested.

			// Since we already verify this for the single prop case and it's kinda
			// slow we'll leave commented out until code review time.  I'd like to
			// remove this if we agree it's redundant.

			// VerifyUnique(hr, m_pTable, m_pIndexID);
			// VerifyPrimaryKey(hr, m_pTable, m_pIndexID);

			FreeProperties(&m_cPropSets, &m_rgPropSets);
		}
	}

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_UNIQUE and DBPROP_INDEX_NULLS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_14()
{ 
	TBEGIN
	HRESULT		hr;
	CIndexInfo  IndexInfo;
	VARIANT_BOOL vbUnique = 100;
	ULONG ulNull = 100;
	ULONG iUniqueVal, iNullVal;
	DBPROPSTATUS stUnique, stNull;

	// Get information about this index.
	TESTC(IndexInfo.Init(m_pIOpenRowset, &(m_pTable->GetTableID()), m_pIndexID));

	// Read current UNIQUE and NULLS value for first column of index (0 based).
	TESTC(IndexInfo.IsIndexValueValid(0, IS_UNIQUE));
	vbUnique = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_UNIQUE);
	TESTC(IndexInfo.IsIndexValueValid(0, IS_NULLS));
	ulNull = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_NULLS);

	for (iUniqueVal = 0; iUniqueVal < NUMELEM(VariantBoolVals); iUniqueVal++)
	{
		for (iNullVal = 0; iNullVal < NUMELEM(IndexNullVals); iNullVal++)
		{
			// For some providers AlterIndex can only set a property to it's current value
			// so set expected prop status appropriately
			if (vbUnique != VariantBoolVals[iUniqueVal] && m_fSetOnlyDefault)
				stUnique = DBPROPSTATUS_NOTSETTABLE;
			else
				stUnique = DBPROPSTATUS_OK;

			if (ulNull != IndexNullVals[iNullVal].ulPropVal && m_fSetOnlyDefault)
				stNull = DBPROPSTATUS_NOTSETTABLE;
			else
				stNull = DBPROPSTATUS_OK;

			//Set prop.
			SetProperty(DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iUniqueVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
			SetProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)(ULONG_PTR)IndexNullVals[iNullVal].ulPropVal, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

			// If this val matches the initial values it should always succeed
			if (VariantBoolVals[iUniqueVal] == vbUnique &&
				IndexNullVals[iNullVal].ulPropVal == ulNull)
			{
				// Try to alter with this prop 
				TESTC_(hr = AlterIndex(m_pIndexID, NULL), S_OK)
			}
			else
			{
				// Try to alter with this prop 
				TEST2C_(hr = AlterIndex(m_pIndexID, NULL), S_OK, DB_E_ERRORSOCCURRED)
			}

			// Verify the prop status based on settability, etc.
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_UNIQUE, DBPROPSET_INDEX, stUnique))
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_NULLS, DBPROPSET_INDEX, stNull))

			// If prop is VARIANT_TRUE, verify the index works as requested.

			// Since we already verify this for the single prop case and it's kinda
			// slow we'll leave commented out until code review time.  I'd like to
			// remove this if we agree it's redundant.

			// VerifyUnique(hr, m_pTable, m_pIndexID);
			// VerifyNulls(hr, m_pTable, m_pIndexID);

			FreeProperties(&m_cPropSets, &m_rgPropSets);
		}
	}

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INDEX_NULLS and DBPROP_INDEX_PRIMARYKEY
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_15()
{ 
	TBEGIN
	HRESULT		hr;
	CIndexInfo  IndexInfo;
	VARIANT_BOOL vbPrimary = 100;
	ULONG ulNull = 100;
	ULONG iPrimaryVal, iNullVal;
	DBPROPSTATUS stPrimary, stNull;

	// Get information about this index.
	TESTC(IndexInfo.Init(m_pIOpenRowset, &(m_pTable->GetTableID()), m_pIndexID));

	// Read current autoupdate value for first column of index (0 based).
	TESTC(IndexInfo.IsIndexValueValid(0, IS_PRIMARY_KEY));
	vbPrimary = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_PRIMARY_KEY);
	TESTC(IndexInfo.IsIndexValueValid(0, IS_NULLS));
	ulNull = *(VARIANT_BOOL *)IndexInfo.GetIndexValuePtr(0, IS_NULLS);

	for (iPrimaryVal = 0; iPrimaryVal < NUMELEM(VariantBoolVals); iPrimaryVal++)
	{
		for (iNullVal = 0; iNullVal < NUMELEM(IndexNullVals); iNullVal++)
		{
			// For some providers AlterIndex can only set a property to it's current value
			// so set expected prop status appropriately
			if (vbPrimary != VariantBoolVals[iPrimaryVal] && m_fSetOnlyDefault)
				stPrimary = DBPROPSTATUS_NOTSETTABLE;
			else
				stPrimary = DBPROPSTATUS_OK;

			if (ulNull != IndexNullVals[iNullVal].ulPropVal && m_fSetOnlyDefault)
				stNull = DBPROPSTATUS_NOTSETTABLE;
			else
				stNull = DBPROPSTATUS_OK;

			//Set prop.
			SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VariantBoolVals[iPrimaryVal], DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
			SetProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)(ULONG_PTR)IndexNullVals[iNullVal].ulPropVal, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

			// If this val matches the initial values it should always succeed
			if (VariantBoolVals[iPrimaryVal] == vbPrimary &&
				IndexNullVals[iNullVal].ulPropVal == ulNull)
			{
				// Try to alter with this prop 
				TESTC_(hr = AlterIndex(m_pIndexID, NULL), S_OK)
			}
			else
			{
				// Try to alter with this prop 
				TEST2C_(hr = AlterIndex(m_pIndexID, NULL), S_OK, DB_E_ERRORSOCCURRED)
			}

			// Verify the prop status based on settability, etc.
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, stPrimary))
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_NULLS, DBPROPSET_INDEX, stNull))

			// If prop is VARIANT_TRUE, verify the index works as requested.
			// VerifyPrimaryKey(hr, m_pTable, m_pIndexID);
			// VerifyNulls(hr, m_pTable, m_pIndexID);

			FreeProperties(&m_cPropSets, &m_rgPropSets);
		}
	}

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED - Non index property with DBPROP_REQUIRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_16()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME,  MakeObjectName(L"IAlterIn", m_cMaxIndexName));

	CHECK_MEMORY(dbidNew.uName.pwszName);

	//Set a non-index property required.
	SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_BOOKMARKS, DBPROPSET_ROWSET, DBPROPSTATUS_NOTSUPPORTED))

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	ReleaseDBID(&dbidNew, FALSE);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED - Non index property with DBPROP_SETIFCHEAP
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_17()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID		dbidNew(DBKIND_NAME,  MakeObjectName(L"IAlterIn", m_cMaxIndexName));

	CHECK_MEMORY(dbidNew.uName.pwszName);

	// Note that DBPROPOPTIONS_OPTIONAL and DBPROPOPTIONS_SETIFCHEAP are numerically identical
	TESTC(DBPROPOPTIONS_OPTIONAL == DBPROPOPTIONS_SETIFCHEAP);

	//Set a non-index property SETIFCHEAP.
	SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_SETIFCHEAP);

	// Try to alter with this prop 
	TESTC_(hr = AlterIndex(m_pIndexID, &dbidNew), DB_S_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_BOOKMARKS, DBPROPSET_ROWSET, DBPROPSTATUS_NOTSUPPORTED))

CLEANUP:
	Refresh_m_pIndexID(hr, dbidNew);
	ReleaseDBID(&dbidNew, FALSE);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Invalid values for properties (unexpected type)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_18()
{ 
	TBEGIN
	HRESULT		hr;
	ULONG		iProp;

	// Try for all index props
	for (iProp = 0; iProp < NUMELEM(IndexProperties); iProp++)
	{
		//Set prop as required using DBTYPE_I2, which doesn't match any index props.
		SetProperty(IndexProperties[iProp].dwProp, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)-1, DBTYPE_I2, DBPROPOPTIONS_REQUIRED);

		// Try to alter with this prop 
		hr = AlterIndex(m_pIndexID, NULL);

		// Allow DB_E_ERRORSOCCURRED
		CHECK(hr, DB_E_ERRORSOCCURRED);

		// Verify the prop status based on settability, etc.
		COMPARE(VerifyPropStatus(m_cPropSets, m_rgPropSets, IndexProperties[iProp].dwProp, DBPROPSET_INDEX, DBPROPSTATUS_BADVALUE), TRUE);

		// Free props for the next time
		FreeProperties(&m_cPropSets, &m_rgPropSets);
	}

	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Specify a property twice
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_19()
{ 
	TBEGIN
	ULONG	iPropSet, iProp;
	LONG	lCurrentVal = 0, lDefaultVal = 0;
	DBID *	pTableID = &(m_pTable->GetTableIDRef());

	CIndexInfo  IndexInfo;

	// Get information about this index.
	TESTC(IndexInfo.Init(m_pIOpenRowset, pTableID, m_pIndexID));

	// See if the property is available
	TESTC(IndexInfo.IsIndexValueValid(0, IS_NULLS));

	// Get the default value of this property
	lDefaultVal = *(LONG *)IndexInfo.GetIndexValuePtr(0, IS_NULLS);

	// Set prop twice with conflicting values
	SetProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_IN_ALLOWNULL, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);
	SetProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_IN_DISALLOWNULL, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TESTC_(AlterIndex(m_pIndexID, NULL), DB_E_ERRORSOCCURRED);

	// See if the property is available
	TESTC(IndexInfo.IsIndexValueValid(0, IS_NULLS));

	// Get the current value of this property
	lCurrentVal = *(LONG *)IndexInfo.GetIndexValuePtr(0, IS_NULLS);

	// VerifyPropStatus only verifies one prop of the two, so we've got to spin through
	// the props ourselves.  Note since we know the propset/props we set we don't bother
	// to validate propset or propid.
	for (iPropSet = 0; iPropSet < m_cPropSets; iPropSet++)
	{
		for (iProp = 0; iProp < m_rgPropSets[iPropSet].cProperties; iProp++)
		{
			// See if the property status matches expected
			if (m_rgPropSets[iPropSet].rgProperties[iProp].dwStatus == DBPROPSTATUS_OK)
				// Must match currently set value
				COMPARE(lCurrentVal, V_I4(&m_rgPropSets[iPropSet].rgProperties[iProp].vValue));
			else
			{
				// If error status is returned this prop must not match current value
				COMPARE(lCurrentVal != V_I4(&m_rgPropSets[iPropSet].rgProperties[iProp].vValue), TRUE);

				// And we expect conflicting status or perhaps not settable for r/o prop
				// not being set to default value.
				if (m_rgPropSets[iPropSet].rgProperties[iProp].dwStatus == DBPROPSTATUS_NOTSETTABLE)
				{
					// This prop value must not match default value
					COMPARE(lDefaultVal != V_I4(&m_rgPropSets[iPropSet].rgProperties[iProp].vValue), TRUE);
					// Must not be a settable prop, or provider must not allow setting props
					// via AlterIndex even if allowed on CreateIndex
					COMPARE
					(
						!SettableProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) ||
						m_fSetOnlyDefault,
						TRUE
					);
				}
				else
					COMPARE(m_rgPropSets[iPropSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_CONFLICTING);
			}
		}
	}

CLEANUP:
	// Free props for the next time
	FreeProperties(&m_cPropSets, &m_rgPropSets);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Set invalid value for prop (unexpected value)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_20()
{ 
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBID * pTableID = &(m_pTable->GetTableIDRef());
	DBID * pIndexID = NULL;
	DBPROPSTATUS stExpected = DBPROPSTATUS_BADVALUE;

	TBEGIN

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	// Set the prop to valid value so we can get an alterable index
	TESTC_PROVIDER(SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED));

	TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
		pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	//Set prop as required using using a value of 1 (not VARIANT_TRUE or VARIANT_FALSE).
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)1, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TESTC_(AlterIndex(m_pIndexID, NULL), DB_E_ERRORSOCCURRED);

	// If the provider doesn't allow setting the prop except to default then we'll get
	// NOTSETTABLE here.
	if (m_fSetOnlyDefault)
		stExpected = DBPROPSTATUS_NOTSETTABLE;

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, stExpected));

CLEANUP:
	
	CleanUpIndex(pTableID, &pIndexID, pIndexColumnDesc, &m_cPropSets, &m_rgPropSets);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Set colid for prop
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_21()
{ 
	TBEGIN
	HRESULT		hr;
	CDBID colid(DBKIND_NAME,  MakeObjectName(L"IAlterIn", m_cMaxIndexName));

	// Per spec DBPROP_INDEX_PRIMARYKEY doesn't take a colid.  Make sure this is so.
	TESTC(!(DBPROPFLAGS_COLUMNOK & GetPropInfoFlags(DBPROP_INDEX_PRIMARYKEY,
		DBPROPSET_INDEX, m_pThisTestModule->m_pIUnknown, DATASOURCE_INTERFACE)));

	TESTC(m_cPropSets == 0);
	TESTC_PROVIDER(SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED, colid));

	// Try to alter with this prop.  Per spec colid should be ignored if no applicable. 
	TESTC_(hr = AlterIndex(m_pIndexID, NULL), S_OK)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, DBPROPSTATUS_OK))

CLEANUP:
	ReleaseDBID(&colid, FALSE);
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Set non-index prop in DBPROPSET_INDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_22()
{ 
	TBEGIN
	HRESULT		hr;

	TESTC(m_cPropSets == 0);
	TESTC(SetProperty(DBPROP_BOOKMARKS, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED));

	// Try to alter with this prop 
	TESTC_(hr = AlterIndex(m_pIndexID, NULL), DB_E_ERRORSOCCURRED)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_BOOKMARKS, DBPROPSET_INDEX, DBPROPSTATUS_NOTSUPPORTED))

CLEANUP:
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Set array of propsets, one with 0 properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_23()
{ 
	TBEGIN
	HRESULT		hr;
	DBID * pTableID = &(m_pNoNullTable->GetTableIDRef());
	DBID * pIndexID = NULL;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	TESTC(m_cPropSets == 0);
	TESTC_PROVIDER(SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED));

	// Create an index we can alter with DBPROP_INDEX_PRIMARYKEY
	TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
		pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	// Create another propset entry with 0 props
	TESTC_PROVIDER(SetSettableProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED));

	VariantClear(&(m_rgPropSets[1].rgProperties[0].vValue));
	m_rgPropSets[1].cProperties = 0;
	m_rgPropSets[1].guidPropertySet = DBPROPSET_INDEX;

	// Try to alter with this prop 
	TESTC_(hr = AlterIndex(pTableID, pIndexID, (DBID *)NULL), S_OK)

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, DBPROPSTATUS_OK))

CLEANUP:

	CleanUpIndex(pTableID, &pIndexID, pIndexColumnDesc, &m_cPropSets, &m_rgPropSets);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Set props with static property sets and properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_24()
{ 
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	DBID * pTableID = &(m_pTable->GetTableIDRef());
	DBID * pIndexID = NULL;
	CVARIANT vTrue(VT_BOOL, VARIANT_TRUE); 
	ULONG cPropSets = 1;
	DBPROP rgProperties[1];
	DBPROPSET rgPropSets[1];

	rgProperties[0].dwPropertyID	= DBPROP_INDEX_PRIMARYKEY;
	rgProperties[0].dwOptions		= DBPROPOPTIONS_REQUIRED;
	rgProperties[0].dwStatus		= DBPROPSTATUS_OK;
	rgProperties[0].colid			= DB_NULLID;
	rgProperties[0].vValue			= vTrue;

	rgPropSets[0].rgProperties		= rgProperties;
	rgPropSets[0].cProperties		= 1;
	rgPropSets[0].guidPropertySet	= DBPROPSET_INDEX;

	TBEGIN

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
		pIndexColumnDesc, cPropSets, rgPropSets, &pIndexID));

	// Try to alter with this prop 
	TESTC_(AlterIndex(pIndexID, NULL), S_OK);

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(cPropSets, rgPropSets, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, DBPROPSTATUS_OK));

CLEANUP:

	CleanUpIndex(pTableID, &pIndexID, pIndexColumnDesc, &m_cPropSets, &m_rgPropSets);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Set invalid properties REQUIRED and change name, verify new name doesn't exist
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_25()
{ 
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	CDBID				dbidNew(DBKIND_NAME,  MakeObjectName(L"IAlterIn", m_cMaxIndexName));
	DBID * pTableID = &(m_pNoNullTable->GetTableIDRef());
	DBID * pIndexID = NULL;
	DBPROPSTATUS stExpected = DBPROPSTATUS_BADVALUE;

	TBEGIN

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	// Set the prop to valid value so we can get an alterable index
	TESTC_PROVIDER(SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED));

	TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
		pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	//Set prop as required using using a value of 1 (not VARIANT_TRUE or VARIANT_FALSE).
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)1, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	// Try to alter with this prop 
	TESTC_(AlterIndex(pTableID, pIndexID, &dbidNew), DB_E_ERRORSOCCURRED);

	// If the provider doesn't allow setting the prop except to default then we'll get
	// NOTSETTABLE here.
	if (m_fSetOnlyDefault)
		stExpected = DBPROPSTATUS_NOTSETTABLE;

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, stExpected));

CLEANUP:

	CleanUpIndex(pTableID, &pIndexID, pIndexColumnDesc, &m_cPropSets, &m_rgPropSets);
	ReleaseDBID(&dbidNew, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Set invalid properties OPTIONAL and change name, verify new name does exist
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProp_SingCol::Variation_26()
{ 
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;
	CDBID				dbidNew(DBKIND_NAME,  MakeObjectName(L"IAlterIn", m_cMaxIndexName));
	DBID * pTableID = &(m_pNoNullTable->GetTableIDRef());
	DBID * pIndexID = NULL;
	DBPROPSTATUS stExpected = DBPROPSTATUS_BADVALUE;

	TBEGIN

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cIndexColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cIndexColumnDesc));

	// Set the prop to valid value so we can get an alterable index
	TESTC_PROVIDER(SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED));

	TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
		pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	//Set prop as optional using using a value of 1 (not VARIANT_TRUE or VARIANT_FALSE).
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)1, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);

	// Try to alter with this prop 
	TESTC_(AlterIndex(pTableID, pIndexID, &dbidNew), DB_S_ERRORSOCCURRED);

	// If the provider doesn't allow setting the prop except to default then we'll get
	// NOTSETTABLE here.
	if (m_fSetOnlyDefault)
		stExpected = DBPROPSTATUS_NOTSETTABLE;

	// Verify the prop status based on settability, etc.
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, stExpected));

CLEANUP:

	CleanUpIndex(pTableID, &pIndexID, pIndexColumnDesc, &m_cPropSets, &m_rgPropSets);
	ReleaseDBID(&dbidNew, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCProp_SingCol::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAlterIndex::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCTransact)
//*-----------------------------------------------------------------------
//| Test Case:		TCTransact - Commit/abort behavior for AlterIndex
//| Created:  	9/22/99
//*-----------------------------------------------------------------------

TCTransact::TCTransact(void)
{
	m_pTransact			= NULL;
	m_ulSupportedTxnDDL = DBPROPVAL_TC_NONE;
	m_hrTxn				= E_FAIL;
}

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCTransact::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CAlterIndex::Init())
	// }}
	{ 
		// Create a transaction object
		m_pTransact = new CTransaction(L"TCTransact");
		TESTC(m_pTransact != NULL);

		// We have to use a different table here than the 
		// one we're calling AlterIndex on, otherwise when the transaction
		// is opened on the table and AlterIndex is called we'll get
		// DB_E_TABLEINUSE.  Use our nullable table (m_pTable).  This will
		// fail with ini file as we really end up with only one table.
		TESTC_PROVIDER(!GetModInfo()->GetFileName());

		// Initialize transact object 
		TESTC_PROVIDER(m_pTransact->Init(this, m_pTable));

		// Get the provider's TXN support for index changes
		GetProperty(DBPROP_SUPPORTEDTXNDDL, DBPROPSET_DATASOURCEINFO, 
			m_pThisTestModule->m_pIUnknown, &m_ulSupportedTxnDDL);

		switch(m_ulSupportedTxnDDL)
		{
			case DBPROPVAL_TC_NONE:
				odtLog << L"\tDBPROP_SUPPORTEDTXNDDL: DBPROPVAL_TC_NONE\n";
				// No transaction support, thus starting transaction should fail
				m_hrTxn = S_OK;		
				break;
			case DBPROPVAL_TC_DML:
				odtLog << L"\tDBPROP_SUPPORTEDTXNDDL: DBPROPVAL_TC_DML\n";
				// Only DML support, thus starting altering index should fail
				m_hrTxn = XACT_E_XTIONEXISTS;		
				break;
			case DBPROPVAL_TC_DDL_COMMIT:
				odtLog << L"\tDBPROP_SUPPORTEDTXNDDL: DBPROPVAL_TC_DDL_COMMIT\n";
				// Only DML support but DDL commits transaction, thus starting altering
				// index should succeed but the transaction will be committed.
				m_hrTxn = S_OK;		
				break;
			case DBPROPVAL_TC_DDL_IGNORE:
				odtLog << L"\tDBPROP_SUPPORTEDTXNDDL: DBPROPVAL_TC_DDL_IGNORE\n";
				// Only DML support but DDL is ignored, thus starting altering
				// index should succeed but the index will not be altered.
				m_hrTxn = S_OK;		
				break;
			case DBPROPVAL_TC_DDL_LOCK:
				odtLog << L"\tDBPROP_SUPPORTEDTXNDDL: DBPROPVAL_TC_DDL_LOCK\n";
				// DDL support but table or index is locked until end of transaction
				m_hrTxn = S_OK;		
				break;
			case DBPROPVAL_TC_ALL:
				odtLog << L"\tDBPROP_SUPPORTEDTXNDDL: DBPROPVAL_TC_ALL\n";
				// DDL support
				m_hrTxn = S_OK;		
				break;
			default:
			{
				TESTC(FALSE);
				m_ulSupportedTxnDDL = DBPROPVAL_TC_NONE;
			}
		}

		// See if we can start a transaction and register the interface
		if(m_pTransact->RegisterInterface(SESSION_INTERFACE, IID_IAlterIndex))
			return TRUE;
		else
			// No transaction support
			return TEST_SKIPPED;
	} 

CLEANUP:

	return FALSE;
} 

int TCTransact::TestTxn
(						 
	ETXN eTxn,
	BOOL fRetaining,
	BOOL fCreateInTxn
)
{
	
	BOOL				fSuccess = FALSE;
	ULONG				index=0;
	CDBID				dbidNew(DBKIND_NAME,  MakeObjectName(L"IAlterIn", m_cMaxIndexName));
	DBID *				pIndexID = NULL;
	DBID *				pTableID = &(m_pNoNullTable->GetTableIDRef());
	HRESULT				hr = E_FAIL;
	ULONG				cPropSets = 0;
	DBPROPSET *			prgPropSets = NULL;
	DBINDEXCOLUMNDESC	*pIndexColumnDesc = NULL;	

	CHECK_MEMORY(dbidNew.uName.pwszName);

	// Initialize pIndexColumnDesc
	SAFE_ALLOC(pIndexColumnDesc, DBINDEXCOLUMNDESC, m_cColumnDesc);
	memset(pIndexColumnDesc, 0, (size_t)(sizeof(DBINDEXCOLUMNDESC)*m_cColumnDesc));

	// Set some index props also as no props were set in base class.
	TESTC(m_cPropSets == 0);
	SetSettableProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
	SetSettableProperty(DBPROP_INDEX_NULLCOLLATION, DBPROPSET_INDEX, &m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_NC_START, VT_I4, DBPROPOPTIONS_REQUIRED);

	// Create an index that we can later alter.
	if (!fCreateInTxn)
		TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
			pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	// Set a property to force non-firehose cursor.  Note this is a *very*
	// provider-specific HACK.  Don't check return code as it might not be
	// supported. Why do I have to do this?  Because:
	// 1) StartTransaction opens a rowset.  
	// 2) AlterIndex helper function opens IDBSchemaRowset.
	// 3) Can't create a new connection inside transaction.
	// Since the CTransaction object has it's own command I really don't need
	// to set the prop back.
	SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, &cPropSets, &prgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
	
	TESTC(m_pTransact->StartTransaction(SELECT_ALLFROMTBL, NULL, 
						cPropSets, prgPropSets, NULL, ISOLATIONLEVEL_READUNCOMMITTED));

	// Create an index that we can later alter.
	if (fCreateInTxn)
		TESTC_PROVIDER(S_OK == CreateAlterableIndex(pTableID, NULL, m_cIndexColumnDesc,
			pIndexColumnDesc, m_cPropSets, m_rgPropSets, &pIndexID));

	//Try AlterIndex in the middle of the transaction, expect appropriate result
	TESTC_(hr = AlterIndex(pTableID, pIndexID, &dbidNew), m_hrTxn);

	// If provider commits transaction when DDL is used we need to start again.
	if (m_ulSupportedTxnDDL == DBPROPVAL_TC_DDL_COMMIT)
	{
		// Are we in a transaction??  Spec is unclear, spec bug opened.  Specifically
		// what is the fRetaining behavior here.
		m_pTransact->CleanUpTransaction(S_OK);

		// Start another transaction
		TESTC(m_pTransact->StartTransaction(SELECT_ALLFROMTBL, NULL, 
							0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED));
	}

	// If provider locks index until end of transaction we should not be able to alter
	// again, right?  But we need a new session to verify this.
	if (m_ulSupportedTxnDDL == DBPROPVAL_TC_DDL_LOCK)
	{
		CDBID dbidLock(DBKIND_NAME, MakeObjectName(L"IAlterIn", m_cMaxIndexName));
		HRESULT hrLock = E_FAIL;
		IAlterIndex * pIAlterIndex = m_pIAlterIndex;
		IDBCreateSession * pIDBCreateSession = (IDBCreateSession *)m_pThisTestModule->m_pIUnknown;

		CHECK_MEMORY(dbidLock.uName.pwszName);

		// Get a new session object
		TESTC_(pIDBCreateSession->CreateSession(NULL, IID_IAlterIndex, (IUnknown**)&m_pIAlterIndex), S_OK);

		// What error do I expect from this???  DB_E_TABLEINUSE seems reasonable
		CHECK(hrLock = AlterIndex(pTableID, &dbidNew, &dbidLock), DB_E_TABLEINUSE);

		// Release the new sesion object and put the old one back
		SAFE_RELEASE(m_pIAlterIndex);
		m_pIAlterIndex = pIAlterIndex;

		// If it did succeed we need to fix things up
		if (SUCCEEDED(hrLock))
		{
			SAFE_FREE(dbidNew.uName.pwszName);
			dbidNew.uName.pwszName = dbidLock.uName.pwszName;
		}
	}

	if (eTxn == ETXN_COMMIT)
	{
		//Commit the transaction, with retention as specified
		TESTC(m_pTransact->GetCommit(fRetaining));
	}
	else
	{
		//Abort the transaction, with retention as specified
		TESTC(m_pTransact->GetAbort(fRetaining));
	}

	// If we commited the transaction, then the new name is the right one,
	// otherwise it's still the previous one.
	if ((eTxn == ETXN_COMMIT || m_ulSupportedTxnDDL == DBPROPVAL_TC_DDL_COMMIT) &&
		m_ulSupportedTxnDDL != DBPROPVAL_TC_DDL_IGNORE)
	{
		// Make sure the new index exists and the props are right.
		// Note our helper does this immediately after altering, but the
		// commit may have a bug and actually abort, so we need to check
		// again.
		COMPARE(CheckIndex(pTableID, &dbidNew, m_cPropSets, m_rgPropSets), TRUE);

		// Release the old index name
		ReleaseDBID(pIndexID, FALSE);
		TESTC_(DuplicateDBID(dbidNew, pIndexID), S_OK);

		// Create another new index name
		SAFE_FREE(dbidNew.uName.pwszName);
		dbidNew.uName.pwszName = MakeObjectName(L"IAlterIn", m_cMaxIndexName);
		CHECK_MEMORY(dbidNew.uName.pwszName);
	}

	// Re-commit or re-abort the transaction because CheckIndex ends up starting
	// another transaction because CheckIndex2 opens another rowset on the table if
	// using integrated indexes.
	if (eTxn == ETXN_COMMIT)
	{
		//Commit the transaction, with retention as specified
		TESTC(m_pTransact->GetCommit(fRetaining));
	}
	else
	{
		//Abort the transaction, with retention as specified
		TESTC(m_pTransact->GetAbort(fRetaining));
	}

	// If the index creation was aborted
	if (fCreateInTxn && eTxn == ETXN_ABORT && 
		m_ulSupportedTxnDDL != DBPROPVAL_TC_DDL_COMMIT)
	{
		BOOL fExists = FALSE;

		// The index creation was aborted as well, make sure it doesn't exist
		if(S_OK == DoesIndexExist(pTableID, 
			pIndexID, &fExists))
			TESTC(!fExists);
	}
	else
		// The index is available, make sure everything still works after
		// commit or abort
		TESTC_(hr = AlterIndex(pTableID, pIndexID, &dbidNew), S_OK);

	fSuccess = TRUE;

CLEANUP:

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	if (fRetaining)
	{	
		BOOL fExists = FALSE;

		// We abort the transaction, thus the index will not
		// retain the new value.
		m_pTransact->CleanUpTransaction(S_OK);

		// The new index should not exist after abort
		if(S_OK == DoesIndexExist(pTableID, 
			&dbidNew, &fExists))
			COMPARE(fExists, FALSE);

		if (!fExists)
			// Since the new index doesn't exist dropping should return NOINDEX.
			CHECK(m_pIIndexDef->DropIndex(pTableID, &dbidNew), DB_E_NOINDEX); 

	}
	else
	{
		// We weren't in a transaction on the last AlterIndex,
		// thus the index will have the new value.  Make it so.
		// Release the old index name
		ReleaseDBID(pIndexID, FALSE);
		CHECK(DuplicateDBID(dbidNew, pIndexID), S_OK);

		m_pTransact->CleanUpTransaction(XACT_E_NOTRANSACTION);
	}

	// Drop the altered index, otherwise we may end up with dupicate PrimaryKey index
	// which is not allowed.
	CleanUpIndex(pTableID, &pIndexID, pIndexColumnDesc, &m_cPropSets, &m_rgPropSets);

	ReleaseDBID(&dbidNew, FALSE);

	FreeProperties(&cPropSets, &prgPropSets);

	return fSuccess;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit Retaining
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransact::Variation_1()
{ 
	return TestTxn(ETXN_COMMIT, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit Non Retaining
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransact::Variation_2()
{ 
	return TestTxn(ETXN_COMMIT, FALSE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort Retaining
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransact::Variation_3()
{ 
	return TestTxn(ETXN_ABORT, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort Non Retaining
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransact::Variation_4()
{ 
	return TestTxn(ETXN_ABORT, FALSE);
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Commit Retaining, create index in txn
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransact::Variation_5()
{ 
	return TestTxn(ETXN_COMMIT, TRUE, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Commit Non Retaining, create index in txn
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransact::Variation_6()
{ 
	return TestTxn(ETXN_COMMIT, FALSE, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Abort Retaining, create index in txn
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransact::Variation_7()
{ 
	return TestTxn(ETXN_ABORT, TRUE, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Abort Non Retaining, create index in txn
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransact::Variation_8()
{ 
	return TestTxn(ETXN_ABORT, FALSE, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCTransact::Terminate()
{ 
	SAFE_DELETE(m_pTransact);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CAlterIndex::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END




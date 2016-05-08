//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module IALTERTABLE.CPP | IALTERTABLE source file for all test modules.
//

#include "MODStandard.hpp"		// Standard headers			
#include "IAlterTable.h"		// IAlterTable header
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xe81d1260, 0x4c52, 0x11d3, { 0xaf, 0x74, 0x00, 0xc0, 0x4f, 0x78, 0x29, 0x26} };
DECLARE_MODULE_NAME("IAlterTable");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IAlterTable Interface Test");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

// Limit new column length to 200
#define NEWCOLUMNLEN	200

// Classification of DataTypes
enum ETYPECLASS
{
	CLASS_SIGNEDINT,	// signed integer
	CLASS_UNSIGNEDINT,	// unsigned integer
	CLASS_APPROXNUM,	// approximate numeric e.g. float, real
	CLASS_SCALEDINT,	// scaled exact integer
	CLASS_DATE,			// date type
	CLASS_STRING,		// string
	CLASS_SPECIAL,		// no like type e.g. uniqueidentifier, variant
};


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable
//
////////////////////////////////////////////////////////////////////////////
class TCIAlterTable : public CSessionObject
{
public:
	//constructors
	TCIAlterTable(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~TCIAlterTable();

	//methods
	virtual BOOL		Init();
	virtual BOOL		Terminate();

	virtual HRESULT		AlterTable
						(
							DBID *		pTableID,
							DBID *		pNewTableID,
							ULONG		cPropertySets = 0,
							DBPROPSET *	rgPropertySets = NULL
						);
	
	virtual HRESULT		AlterColumn
						(
							DBID *				pTableID = NULL,
							DBID *				pColumnID = NULL,
							DBCOLUMNDESCFLAGS	ColumnDescFlags = 0,
							DBCOLUMNDESC *		pColumnDesc = NULL
						);

	virtual BOOL		SetProperty
						(
							DBPROPID		dwPropertyID,
							GUID			guidPropertySet = DBPROPSET_TABLE, 
							void *			pValue = (void *)VARIANT_TRUE, 
							DBTYPE			wType = DBTYPE_BOOL,
							DBPROPOPTIONS	dwOptions = DBPROPOPTIONS_REQUIRED, 
							DBID			colid = DB_NULLID
						);

	virtual BOOL		CheckUnsupportedProp
						(
							ULONG			cPropSets,
							DBPROPSET *		rgPropSets
						);

	virtual BOOL		VerifyPropStatus
						(
							ULONG			cPropSets,
							DBPROPSET *		rgPropSets, 
							DBPROPID		dwPropertyID,
							GUID			guidPropertySet,
							DBPROPSTATUS	 dwInputStatus
						);

	virtual BOOL		VerifyAlterTable
						(
							DBID *		pTableID,
							DBID *		pNewTableID
						);

	virtual	BOOL		CheckAlterTableID
						(
							DBID *		pTableID,
							DBID *		pNewTableID
						);

	virtual BOOL		VerifyAlterTableProperties
						(
							DBID *			pTableID,
							DBPROPID		dbPropID,
							GUID			guidPropertySet,
							void *			pValue,
							DBTYPE			wType,
							DBPROPOPTIONS	dwOptions
						);

	virtual BOOL		VerifyAlterInvalidTableProperties
						(
							DBID *			pTableID,
							DBPROPSTATUS	dwExpectedStatus,
							DBPROPID		dbPropID,
							GUID			guidPropertySet,
							void *			pValue,
							DBTYPE			wType,
							DBPROPOPTIONS	dwOptions,
							DBID			colid = DB_NULLID
						);

	virtual BOOL		VerifyAlterColumn
						(
							DBORDINAL			ulColumnOrdinal,
							DBCOLUMNDESCFLAGS	ColumnDescFlags,
							DBCOLUMNDESC *		pColumnDesc,
							CTable *			pTable = NULL,
							HRESULT *			phr = NULL
						);

	virtual BOOL		VerifyAlterColumnName
						(
							DBORDINAL			ulColumnOrdinal,
							DBID *				pNewColumnID
						);

	virtual BOOL		VerifyAlterColumnSize
						(
							DBORDINAL			ulColumnOrdinal,
							DBLENGTH			ulNewColSize,
							CTable *			pTable = NULL
						);

	virtual BOOL		VerifyAlterColumnPrecision
						(
							DBORDINAL			ulColumnOrdinal,
							BYTE				bNewPrecision
						);

	virtual BOOL		VerifyAlterColumnScale
						(
							DBORDINAL			ulColumnOrdinal,
							BYTE				bNewScale
						);

	virtual BOOL		VerifyAlterColumnCLSID
						(
							DBORDINAL			ulColumnOrdinal,
							CLSID *				pclsid
						);

	virtual BOOL		VerifyAlterColumnType
						(
							DBORDINAL			ulColumnOrdinal,
							DBTYPE				wType,
							CTable *			pTable
						);

	virtual BOOL		VerifyAlterColumnTypeName
						(
							DBORDINAL			ulColumnOrdinal,
							WCHAR *				pwszTypeName,
							CTable *			pTable
						);

	virtual BOOL		VerifyAlterColumnProperty
						(
							DBORDINAL		ulColumnOrdinal,
							DBPROPID		dbPropID,
							void *			pValue,
							DBTYPE			wType,
							DBPROPOPTIONS	dwOptions = DBPROPOPTIONS_REQUIRED
						);


	virtual BOOL		VerifyAlterInvalidColumnProperties
						(
							CTable *		pTable,
							DBPROPSTATUS	dwExpectedStatus,
							DBPROPID		dbPropID,
							GUID			guidPropertySet,
							void *			pValue,
							DBTYPE			wType,
							DBPROPOPTIONS	dwOptions,
							DBID			colid = DB_NULLID
						);

	virtual BOOL		CompareColumnMetaData
						(
							DBORDINAL			ulColumnOrdinal,
							DBCOLUMNDESCFLAGS	ColumnDescFlags,
							DBCOLUMNDESC *		pColumnDesc,
							CTable *			pTable = NULL
						);

	virtual BOOL		CacheLiteralInfo();

	virtual	HRESULT		MakeNewID
						(
							DBID *		pDBID,
							DBKINDENUM	eKind,
							size_t		ulLen
						);

	virtual	HRESULT		MakeNewTableID
						(
							DBID *		pDBID,
							DBKINDENUM	eKind,
							size_t		ulLen
						);

	virtual HRESULT		MakeNewColumnID
						(
							DBID *		pNewColumnID,
							DBKINDENUM	eKind,
							size_t		ulLen
						);

	virtual BOOL		RestoreTableID
						(
							DBID *		pCurrentID
						);

	virtual BOOL		RestoreColumnID
						(
							DBORDINAL			ulColumnOrdinal,
							DBID *				pCurrentID,
							DBCOLUMNDESCFLAGS	ColumnDescFlags
						);

	virtual HRESULT		CreateTable
						(
							CTable **		ppTable
						);

	virtual HRESULT		CreateTable
						(
							CTable **		ppTable,
							DBPROPID		dbPropID,
							void *			pValue,
							DBPROPOPTIONS	dwOptions
						);

	virtual HRESULT		CreateOneColTableWithProps
						(
							CTable **		ppTable,
							CCol *			pCol,
							ULONG			cPropSets,
							DBPROPSET *		rgPropSets
						);

	virtual BOOL		VerifyNonTableProps
						(
							DBPROPOPTIONS	dwOptions
						);

	virtual HRESULT		SetChangeColInfo
						(
							CCol *			pCol
						);

	virtual BOOL		GetReadOnlySession
						(
							IUnknown **		ppUnkReadSession
						);

	virtual BOOL 		AlterPropertyAndVerifyRowset
						(
							DBPROPID		dbPropID
						);

	virtual BOOL		VerifyRowsetValues
						(
							CTable *	pTable,
							DBPROPID	dbPropID
						);

	virtual BOOL		UpgradeType
						(
							CCol *		pCurrentCol,
							CCol *		pNewCol,
							BOOL		fAllowLong = FALSE
						);

	virtual BOOL		IsTypeGreater
						(
							CCol *	pCurCol,
							CCol *	pNewCol
						);

	virtual ULONG		GetTypeClass
						(
							DBTYPE	wType
						);

	virtual ULONG		GetTypeRank
						(
							DBTYPE	wType
						);

	virtual BOOL		MakeDefaultData
						(
							CCol *		pCol,
							void **		ppData,
							DBTYPE *	pwType,
							CTable *	pTable
						);

	void				ReleaseDefaultData
						(
							void *		pDefaultData,
							DBTYPE		wDefaultType
						);

	virtual	BOOL		VerifyTableDefault
						(
							CTable *	pTable,
							BOOL		fDefaultExists
						);

	virtual BOOL		VerifyTableUnique
						(
							CTable *	pTable,
							BOOL		fUniqueExists
						);

	virtual BOOL		AlterStringCol2BLOB
						(
							DBTYPE	wTargetType,
							BOOL	fIsFixed
						);

	virtual BOOL		TCAlterCompareDBID
						(
							DBID * pdbid1,			
							DBID * pdbid2
						);

	static ULONG WINAPI Thread_AlterTable(void* pv);

	static ULONG WINAPI Thread_AlterColumn(void* pv);

	//Data
	IAlterTable *		m_pIAlterTable;

	CTable	*			m_pSchema;
	CSchema *			m_pFullSchemaInfo;
	CCol				m_Col;

	ULONG				m_cchMaxTableName;
	ULONG				m_cchMaxColName;
	WCHAR *				m_pwszInvalidTableChars;
	WCHAR *				m_pwszInvalidStartingTableChars;
	WCHAR *				m_pwszInvalidColChars;
	WCHAR *				m_pwszInvalidStartingColChars;

	DBKINDENUM			m_lDBIDType;

	DBCOLUMNDESCFLAGS	m_lAlterColumnSupport;

	DBID				m_TableID;
	DBID				m_ColumnID;

	CCol *				m_pChangedCol;
};




////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::TCIAlterTable
//
////////////////////////////////////////////////////////////////////////////
TCIAlterTable::TCIAlterTable(WCHAR * wstrTestCaseName)	: CSessionObject(wstrTestCaseName) 
{
	m_pIAlterTable		= NULL;
	m_pSchema			= NULL;
	m_pFullSchemaInfo	= NULL;

	m_cchMaxTableName	= ~0;
	m_cchMaxColName		= ~0;

	m_pwszInvalidTableChars			= NULL;
	m_pwszInvalidStartingTableChars = NULL;
	m_pwszInvalidColChars			= NULL;
	m_pwszInvalidStartingColChars	= NULL;

	m_lDBIDType = DBKIND_NAME;

	m_lAlterColumnSupport = 0;

	memset(&m_TableID, 0, sizeof(DBID));
	memset(&m_ColumnID, 0, sizeof(DBID));

	m_pChangedCol	= NULL;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::~TCIAlterTable
//
////////////////////////////////////////////////////////////////////////////
TCIAlterTable::~TCIAlterTable()
{
	ReleaseDBID(&m_TableID, FALSE);
	ReleaseDBID(&m_ColumnID, FALSE);

	SAFE_FREE(m_pwszInvalidTableChars);
	SAFE_FREE(m_pwszInvalidStartingTableChars);
	SAFE_FREE(m_pwszInvalidColChars);
	SAFE_FREE(m_pwszInvalidStartingColChars);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::Init
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::Init()
{
	BOOL		fPass = TEST_FAIL;
	LONG_PTR	lSQLSupport = 0;

    //Init baseclass
	TESTC(CSessionObject::Init())
	
	//Use the global DSO created in Module init
	SetDataSourceObject(g_pIDBInitialize); 

	//Use the global DB Rowset created in Module init
	SetDBSession(g_pIOpenRowset); 

	m_pTable = new CTable(g_pIOpenRowset, L"IALTTAB");
	TESTC(m_pTable != NULL)
	TESTC_(m_pTable->CreateTable(0,COL_ONE,NULL,PRIMARY,TRUE),S_OK);

	TESTC_(DuplicateDBID(m_pTable->GetTableID(), &m_TableID), S_OK);
	m_lDBIDType = (DBKINDENUM)m_TableID.eKind;

	TESTC(CacheLiteralInfo());
    		
	TESTC(VerifyInterface(m_pIOpenRowset, IID_IAlterTable, SESSION_INTERFACE,
		(IUnknown**)&m_pIAlterTable));

	m_pFullSchemaInfo = new CSchema();
	TESTC(m_pFullSchemaInfo != NULL);

	GetProperty(DBPROP_SQLSUPPORT, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, (ULONG_PTR *)&lSQLSupport);
	
	TESTC(SUCCEEDED(m_pFullSchemaInfo->PopulateTypeInfo(m_pIOpenRowset,
		lSQLSupport)));

	fPass = TEST_PASS;

CLEANUP:
	
	return fPass;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::Terminate
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::Terminate()
{	
	if( m_pTable )
	{
		m_pTable->DropTable();
		SAFE_DELETE(m_pTable);
	}

	SAFE_DELETE(m_pFullSchemaInfo);

	SAFE_RELEASE(m_pIAlterTable);

	SAFE_FREE(m_pwszInvalidTableChars);
	SAFE_FREE(m_pwszInvalidStartingTableChars);
	SAFE_FREE(m_pwszInvalidColChars);
	SAFE_FREE(m_pwszInvalidStartingColChars);
	
    ReleaseDBSession();
	ReleaseDataSourceObject();

	return CSessionObject::Terminate();
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::CacheLiteralInfo
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::CacheLiteralInfo()
{	
	BOOL			fSuccess = FALSE;
	IDBInfo *		pIDBInfo = NULL;
	DBLITERAL		rgDBLiterals[] = {DBLITERAL_TABLE_NAME, DBLITERAL_COLUMN_NAME};
	ULONG			cLiteralInfo = 0;
	DBLITERALINFO *	rgLiteralInfo = NULL;
	WCHAR *			pCharBuffer = NULL;
	DWORDLONG		dwl = 0;

	
	TESTC(GetProperty(DBPROP_ALTERCOLUMN, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, (ULONG_PTR *)&dwl));
	m_lAlterColumnSupport = (DBCOLUMNDESCFLAGS)dwl;

	if(VerifyInterface(m_pIDBInitialize, IID_IDBInfo, DATASOURCE_INTERFACE, 
					(IUnknown **)&pIDBInfo))
	{
		TESTC_(pIDBInfo->GetLiteralInfo(NUMELEM(rgDBLiterals), rgDBLiterals, 
				&cLiteralInfo, &rgLiteralInfo, &pCharBuffer), S_OK);
		TESTC(cLiteralInfo == 2);

		if( rgLiteralInfo[0].fSupported )
		{
			m_cchMaxTableName = rgLiteralInfo[0].cchMaxLen;
			m_pwszInvalidTableChars = wcsDuplicate(rgLiteralInfo[0].pwszInvalidChars);
			m_pwszInvalidStartingTableChars = wcsDuplicate(rgLiteralInfo[0].pwszInvalidStartingChars);
		}
		
		if( rgLiteralInfo[1].fSupported )
		{
			m_cchMaxColName = rgLiteralInfo[1].cchMaxLen;
			m_pwszInvalidColChars = wcsDuplicate(rgLiteralInfo[1].pwszInvalidChars);
			m_pwszInvalidStartingColChars = wcsDuplicate(rgLiteralInfo[1].pwszInvalidStartingChars);
		}
	}

	fSuccess = TRUE;

CLEANUP:
	
	SAFE_FREE(pCharBuffer);
	SAFE_FREE(rgLiteralInfo);

	SAFE_RELEASE(pIDBInfo);

	return fSuccess;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::AlterTable
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIAlterTable::AlterTable
(
	DBID *		pTableID,
	DBID *		pNewTableID,
	ULONG		cPropertySets,
	DBPROPSET *	rgPropertySets
)
{	
	return m_pIAlterTable->AlterTable(pTableID, pNewTableID, cPropertySets, rgPropertySets);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::AlterColumn
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIAlterTable::AlterColumn
(
	DBID *				pTableID,
	DBID *				pColumnID,
	DBCOLUMNDESCFLAGS	ColumnDescFlags,
	DBCOLUMNDESC *		pColumnDesc

)
{	
	return m_pIAlterTable->AlterColumn(pTableID, pColumnID, ColumnDescFlags, pColumnDesc);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::SetProperty
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::SetProperty
(
	DBPROPID		dwPropertyID,
	GUID			guidPropertySet, 
	void *			pValue, 
	DBTYPE			wType,
	DBPROPOPTIONS	dwOptions, 
	DBID			colid
)
{
	return ::SetProperty(dwPropertyID, guidPropertySet, &m_cPropSets,
		&m_rgPropSets, pValue, wType, dwOptions, colid);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::CheckUnsupportedProp
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::CheckUnsupportedProp
(
	ULONG			cPropSets,
	DBPROPSET *		rgPropSets
)
{
	TBEGIN

	for(ULONG iPropSet=0; iPropSet<cPropSets && rgPropSets; iPropSet++)
	{
		DBPROPSET * pPropSet = &rgPropSets[iPropSet];
		
		//Loop over all the properties of this set
		for(ULONG iProp=0; iProp<pPropSet->cProperties && pPropSet->rgProperties; iProp++)
		{	
			DBPROP * pProp = &pPropSet->rgProperties[iProp];	

			if( pProp->dwStatus != DBPROPSTATUS_OK )
			{
				if( pProp->dwOptions == DBPROPOPTIONS_OPTIONAL )
				{
					COMPARE(pProp->dwStatus, DBPROPSTATUS_NOTSET);
				}
				else
				{
					// This method should only be called with valid parameters
					// Hence, it should only fail if a property is not supported.
					TESTC(pProp->dwStatus == DBPROPSTATUS_NOTSUPPORTED);
				}
			}				
		}
	}

CLEANUP:

	TRETURN
}

////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyPropStatus
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyPropStatus
(
	ULONG			cPropSets,
	DBPROPSET *		rgPropSets, 
	DBPROPID		dwPropertyID,
	GUID			guidPropertySet,
	DBPROPSTATUS	 dwInputStatus
)
{
	ASSERT(cPropSets && rgPropSets);
	TBEGIN
	DBPROP * pProp = NULL;
		
	//Need to find the property within the PropSets
	DBPROPFLAGS dwFlags = GetPropInfoFlags(dwPropertyID, guidPropertySet);
	TESTC(FindProperty(dwPropertyID, guidPropertySet, cPropSets, rgPropSets, &pProp))

	// Check for UNSUPPORTED property
	if(pProp->dwStatus == DBPROPSTATUS_NOTSUPPORTED)
	{
		TOUTPUT("Status returned: " << GetPropStatusName(pProp->dwStatus) << " For property: " << GetPropertyName(pProp->dwPropertyID, guidPropertySet));
		goto CLEANUP;
	}

	// Otherwise delegate to Extralib's VerifyPropStatus	
	TESTC(::VerifyPropStatus(cPropSets, rgPropSets, dwPropertyID, guidPropertySet, dwInputStatus));
	
CLEANUP:	
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterTable
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterTable
(
	DBID *		pTableID,
	DBID *		pNewTableID
)
{
	TBEGIN
	
	TESTC( SUCCEEDED(AlterTable(pTableID, pNewTableID)) );
	
	TESTC(CheckAlterTableID(pTableID, pNewTableID));

CLEANUP:

	TRETURN
}

////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::CheckAlterTableID
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::CheckAlterTableID
(
	DBID *		pTableID,
	DBID *		pNewTableID
)
{
	TBEGIN
	BOOL	fExists;

	// pNewTableID is NULL or pNewTableID is the same as pTableID, 
	// then the table ID should not have changed
	if( !pNewTableID || CompareDBID(*pTableID, *pNewTableID) )
	{
		TESTC_(m_pTable->DoesTableExist(pTableID, &fExists), S_OK);
		TESTC(fExists == TRUE);
	}
	else
	{
		
		TESTC_(m_pTable->DoesTableExist(pTableID, &fExists), S_OK);
		TESTC(fExists == FALSE);

		TESTC_(m_pTable->DoesTableExist(pNewTableID, &fExists), S_OK);
		TESTC(fExists == TRUE);
	}

CLEANUP:

	TRETURN
}

////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterTableProperties
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterTableProperties
(
	DBID *			pTableID,
	DBPROPID		dbPropID,
	GUID			guidPropertySet,
	void *			pValue,
	DBTYPE			wType,
	DBPROPOPTIONS	dwOptions
)
{
	HRESULT		hr;
	ULONG		cPropSets = 0;
	DBPROPSET *	rgPropSets = NULL;
	BOOL		fSupported;
	BOOL		fSettable;
	DBPROP *	pProp = NULL;
		
	fSupported	= SupportedProperty(dbPropID, DBPROPSET_TABLE);
	fSettable	= SettableProperty(dbPropID, DBPROPSET_TABLE);

	::SetProperty(dbPropID, guidPropertySet, &cPropSets, &rgPropSets, 
		pValue, wType, dwOptions);

	pProp = rgPropSets[0].rgProperties;

	hr = AlterTable(pTableID, NULL, cPropSets, rgPropSets);

	if( hr == DB_E_ERRORSOCCURRED && pProp->dwStatus == DBPROPSTATUS_NOTSUPPORTED)
	{
		TESTC(dwOptions == DBPROPOPTIONS_REQUIRED);
		odtLog << " Temp Table property was not supported with IAlterTable. " << ENDL;
	}
	else if( hr == DB_S_ERRORSOCCURRED && (pProp->dwStatus == DBPROPSTATUS_NOTSET || pProp->dwStatus == DBPROPSTATUS_NOTSUPPORTED))
	{
	
		TESTC(dwOptions == DBPROPOPTIONS_OPTIONAL);
		odtLog << " Temp Table property was not supported with IAlterTable. " << ENDL;
	}	 
	else
	{	
		if( fSupported && fSettable )
		{
			TESTC_(hr, S_OK);

			// Verify TEMPTABLE property was added/removed
			if( dbPropID == DBPROP_TBL_TEMPTABLE )
			{

			}
		}
		else
		{
			// Only other reason to fail is that the property is read only
			TESTC_(hr, DB_E_ERRORSOCCURRED);
			TESTC(pProp->dwStatus == DBPROPSTATUS_NOTSETTABLE);			
		}
	}

CLEANUP:

	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterInvalidTableProperties
//
//	Test invalid property conditions
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterInvalidTableProperties
(
	DBID *			pTableID,
	DBPROPSTATUS	dwExpectedStatus,
	DBPROPID		dbPropID,
	GUID			guidPropertySet,
	void *			pValue,
	DBTYPE			wType,
	DBPROPOPTIONS	dwOptions,
	DBID			colid
)
{
	HRESULT		hrExpected;
	ULONG		cPropSets = 0;
	DBPROPSET *	rgPropSets = NULL;

	::SetProperty(dbPropID, guidPropertySet, &cPropSets, &rgPropSets, 
		pValue, wType, dwOptions, colid);

	hrExpected = dwOptions == DBPROPOPTIONS_OPTIONAL ? DB_S_ERRORSOCCURRED : DB_E_ERRORSOCCURRED;
	TESTC_(AlterTable(pTableID, NULL, cPropSets, rgPropSets), hrExpected);

	TESTC(VerifyPropStatus(cPropSets, rgPropSets, dbPropID,	guidPropertySet, dwExpectedStatus))

CLEANUP:

	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
}

////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterColumn
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterColumn
(
	DBORDINAL			ulColumnOrdinal,
	DBCOLUMNDESCFLAGS	ColumnDescFlags,
	DBCOLUMNDESC *		pColumnDesc,
	CTable *			pTable,
	HRESULT	*			phr
)
{
	TBEGIN
	HRESULT		hr;
	CCol		TempCol;
	CTable *	pAlterTable = NULL;

	if( pTable )
	{
		pAlterTable = pTable;
	}
	else
	{
		ASSERT(m_pTable != NULL);
		pAlterTable = m_pTable;
	}

	TESTC_(pAlterTable->GetColInfo(ulColumnOrdinal, TempCol), S_OK);
	
	hr = AlterColumn(&pAlterTable->GetTableID(), TempCol.GetColID(),
		ColumnDescFlags, pColumnDesc);

	//On Error, we need to check for NOT_SUPPORTED property errors
	//Loop over all property sets
	if( hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED )
	{
		// DBCOLUMNDESCFLAGS_PROPERTIES must be requested 
		// for DB_E_ERROR*/DB_S_ERROR*
		TESTC((ColumnDescFlags & DBCOLUMNDESCFLAGS_PROPERTIES) != 0);
		TESTC(CheckUnsupportedProp(pColumnDesc->cPropertySets, pColumnDesc->rgPropertySets));
	}
	else
		TESTC_(hr, S_OK);

	if( SUCCEEDED(hr) )
	{
		TESTC(CompareColumnMetaData(ulColumnOrdinal, ColumnDescFlags, pColumnDesc, pTable));
	}

CLEANUP:

	if( phr )
		*phr = hr;

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterColumnName
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterColumnName
(
	DBORDINAL	ulColumnOrdinal,
	DBID *		pNewColumnID
)
{
	DBCOLUMNDESC	ColumnDesc;

	memset(&ColumnDesc, 0xCA, sizeof(DBCOLUMNDESC));
	memcpy(&ColumnDesc.dbcid, pNewColumnID, sizeof(DBID));
	
	return VerifyAlterColumn(ulColumnOrdinal, DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterColumnSize
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterColumnSize
(
	DBORDINAL			ulColumnOrdinal,
	DBLENGTH			ulNewColSize,
	CTable *			pTable
)
{
	DBCOLUMNDESC	ColumnDesc;

	memset(&ColumnDesc, 0xCA, sizeof(DBCOLUMNDESC));	
	ColumnDesc.ulColumnSize = ulNewColSize;
	
	return VerifyAlterColumn(ulColumnOrdinal, DBCOLUMNDESCFLAGS_COLSIZE, &ColumnDesc, pTable);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterColumnPrecision
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterColumnPrecision
(
	DBORDINAL			ulColumnOrdinal,
	BYTE				bNewPrecision
)
{
	DBCOLUMNDESC	ColumnDesc;

	memset(&ColumnDesc, 0xCA, sizeof(DBCOLUMNDESC));	
	ColumnDesc.bPrecision = bNewPrecision;
	
	return VerifyAlterColumn(ulColumnOrdinal, DBCOLUMNDESCFLAGS_PRECISION, &ColumnDesc);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterColumnScale
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterColumnScale
(
	DBORDINAL			ulColumnOrdinal,
	BYTE				bNewScale
)
{
	DBCOLUMNDESC	ColumnDesc;

	memset(&ColumnDesc, 0xCA, sizeof(DBCOLUMNDESC));	
	ColumnDesc.bScale = bNewScale;
	
	return VerifyAlterColumn(ulColumnOrdinal, DBCOLUMNDESCFLAGS_SCALE, &ColumnDesc);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterColumnType
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterColumnType
(
	DBORDINAL			ulColumnOrdinal,
	DBTYPE				wType,
	CTable *			pTable
)
{
	DBCOLUMNDESC	ColumnDesc;

	memset(&ColumnDesc, 0xCA, sizeof(DBCOLUMNDESC));	
	ColumnDesc.wType = wType;
	
	return VerifyAlterColumn(ulColumnOrdinal, DBCOLUMNDESCFLAGS_WTYPE, &ColumnDesc, pTable);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterColumnTypeName
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterColumnTypeName
(
	DBORDINAL			ulColumnOrdinal,
	WCHAR *				pwszTypeName,
	CTable *			pTable
)
{
	DBCOLUMNDESC	ColumnDesc;

	memset(&ColumnDesc, 0xCA, sizeof(DBCOLUMNDESC));	
	ColumnDesc.pwszTypeName = pwszTypeName;
	
	return VerifyAlterColumn(ulColumnOrdinal, DBCOLUMNDESCFLAGS_TYPENAME, &ColumnDesc, pTable);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterColumnCLSID
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterColumnCLSID
(
	DBORDINAL			ulColumnOrdinal,
	CLSID *				pclsid
)
{
	DBCOLUMNDESC	ColumnDesc;

	memset(&ColumnDesc, 0xCA, sizeof(DBCOLUMNDESC));	
	ColumnDesc.pclsid = pclsid;
	
	return VerifyAlterColumn(ulColumnOrdinal, DBCOLUMNDESCFLAGS_CLSID, &ColumnDesc);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterColumnProperty
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterColumnProperty
(
	DBORDINAL		ulColumnOrdinal,
	DBPROPID		dbPropID,
	void *			pValue,
	DBTYPE			wType,
	DBPROPOPTIONS	dwOptions
)
{
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;
	ULONG			cPropSets = 0;
	DBPROPSET *		rgPropSets = NULL;

	::SetProperty(dbPropID, DBPROPSET_COLUMN, &cPropSets, &rgPropSets, 
		pValue, wType, dwOptions);

	memset(&ColumnDesc, 0xCA, sizeof(DBCOLUMNDESC));	
	ColumnDesc.cPropertySets = cPropSets;
	ColumnDesc.rgPropertySets = rgPropSets;
	
	TESTC(VerifyAlterColumn(ulColumnOrdinal, DBCOLUMNDESCFLAGS_PROPERTIES, &ColumnDesc));
	
CLEANUP:

	::FreeProperties(&cPropSets, &rgPropSets);

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyAlterInvalidColumnProperties
//
//	Test invalid property conditions
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyAlterInvalidColumnProperties
(
	CTable *		pTable,
	DBPROPSTATUS	dwExpectedStatus,
	DBPROPID		dbPropID,
	GUID			guidPropertySet,
	void *			pValue,
	DBTYPE			wType,
	DBPROPOPTIONS	dwOptions,
	DBID			colid
)
{
	HRESULT			hrExpected;
	DBCOLUMNDESC	ColumnDesc;
	CCol			col;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_PROPERTIES) == 0 )
		return TEST_SKIPPED;

	memset(&ColumnDesc, 0, sizeof(DBCOLUMNDESC));

	TESTC_(pTable->GetColInfo(1, col), S_OK);

	::SetProperty(dbPropID, guidPropertySet, &ColumnDesc.cPropertySets, &ColumnDesc.rgPropertySets, 
		pValue, wType, dwOptions, colid);

	hrExpected = dwOptions == DBPROPOPTIONS_OPTIONAL ? DB_S_ERRORSOCCURRED : DB_E_ERRORSOCCURRED;
	
	TESTC_(AlterColumn(&pTable->GetTableID(), col.GetColID(), DBCOLUMNDESCFLAGS_PROPERTIES, &ColumnDesc), hrExpected);

	TESTC(VerifyPropStatus(ColumnDesc.cPropertySets, ColumnDesc.rgPropertySets, dbPropID, guidPropertySet, dwExpectedStatus))

CLEANUP:

	::FreeProperties(&ColumnDesc.cPropertySets, &ColumnDesc.rgPropertySets);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::CompareColumnMetaData
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::CompareColumnMetaData
(
	DBORDINAL			ulColumnOrdinal,
	DBCOLUMNDESCFLAGS	ColumnDescFlags,
	DBCOLUMNDESC *		pColumnDesc,
	CTable *			pTable
)
{
	TBEGIN
	BOOL			fTypeChanged = FALSE;
	CTable *		pAlterTable = NULL;
	CTable			CurrentSchema(g_pIOpenRowset);
	CTable *		pPrevSchema = NULL;
	IColumnsInfo *	pIColumnsInfo = NULL;
	CCol			CachedColInfo;
	CCol			CurrentColInfo;

	if( pTable )
	{
		pAlterTable = pTable;
		pPrevSchema = pTable;
	}
	else
	{
		ASSERT(m_pTable != NULL);
		pAlterTable = m_pTable;
		pPrevSchema = m_pSchema;
	}
		
	// Now get the rowset which we'll use to populate the table
	TESTC_(pAlterTable->CreateRowset(USE_OPENROWSET, IID_IColumnsInfo, 0, 
		NULL, (IUnknown **)&pIColumnsInfo),S_OK);
	TESTC_(CurrentSchema.GetTableColumnInfo(&pAlterTable->GetTableID(), pIColumnsInfo), S_OK);
	TESTC_(CurrentSchema.AddInfoFromColumnsSchemaRowset(g_pIOpenRowset, pAlterTable->GetTableName()), S_OK);
	
	TESTC( CurrentSchema.CountColumnsOnSchema() == pPrevSchema->CountColumnsOnSchema() );

	
	// Verify that the appropriate change has been made
	TESTC_(pPrevSchema->GetColInfo(ulColumnOrdinal, CachedColInfo), S_OK);	
	TESTC_(CurrentSchema.GetColInfo(ulColumnOrdinal, CurrentColInfo), S_OK);

	if( ColumnDescFlags & DBCOLUMNDESCFLAGS_DBCID )
	{
		TESTC(TCAlterCompareDBID(CurrentColInfo.GetColID(), &pColumnDesc->dbcid));
	}
	else
	{
		TESTC(TCAlterCompareDBID(CurrentColInfo.GetColID(), CachedColInfo.GetColID()));
	}

	// If the column type changes, ColumnSize, Precision, and Scale will all be affected
	fTypeChanged = ColumnDescFlags & (DBCOLUMNDESCFLAGS_WTYPE | DBCOLUMNDESCFLAGS_TYPENAME);
	if( fTypeChanged )
	{
		// When changing a column type, the test variation must call SetChangeColInfo
		// to indicate the new type information
		ASSERT(m_pChangedCol != NULL);
	}

	if( ColumnDescFlags & DBCOLUMNDESCFLAGS_WTYPE )
	{
		TESTC(CurrentColInfo.GetProviderType() == pColumnDesc->wType);
	}
	else
	{
		if( fTypeChanged )
		{
			TESTC(CurrentColInfo.GetProviderType() == m_pChangedCol->GetProviderType());
		}
		else
		{
			TESTC(CurrentColInfo.GetProviderType() == CachedColInfo.GetProviderType());
		}
	}

	if( ColumnDescFlags & DBCOLUMNDESCFLAGS_TYPENAME )
	{
		// There is no way to derive a column's underlying typename once it is created.
		// So, check the wType, FixedLength, and IsLong attributes
		TESTC(CurrentColInfo.GetProviderType() == m_pChangedCol->GetProviderType());
		TESTC(CurrentColInfo.GetIsLong() == m_pChangedCol->GetIsLong());
		TESTC(CurrentColInfo.GetIsFixedLength() == m_pChangedCol->GetIsFixedLength());
	}

	// Column Size should only be applicable to non-BLOB STRING and BINARY types
	if( (CurrentColInfo.GetProviderType() == DBTYPE_STR ||
		CurrentColInfo.GetProviderType() == DBTYPE_WSTR ||
		CurrentColInfo.GetProviderType() == DBTYPE_BYTES) &&
		ColumnDescFlags & DBCOLUMNDESCFLAGS_COLSIZE )
	{
		TESTC(CurrentColInfo.GetColumnSize() == pColumnDesc->ulColumnSize);
	}
	else
	{
		if( !fTypeChanged )
		{
			TESTC(CurrentColInfo.GetColumnSize() == CachedColInfo.GetColumnSize());
		}
		else
		{
			if( CurrentColInfo.GetProviderType() == DBTYPE_STR ||
				CurrentColInfo.GetProviderType() == DBTYPE_WSTR ||
				CurrentColInfo.GetProviderType() == DBTYPE_BYTES )
			{
				TESTC(CurrentColInfo.GetColumnSize() == CachedColInfo.GetColumnSize());
			}
			else
			{
				TESTC(CurrentColInfo.GetColumnSize() == m_pChangedCol->GetColumnSize());
			}
		}
	}

	// Precision should only be applicable to NUMERIC and VARNUMERIC types
	if( (CurrentColInfo.GetProviderType() == DBTYPE_NUMERIC ||
		CurrentColInfo.GetProviderType() == DBTYPE_VARNUMERIC) &&
		(ColumnDescFlags & DBCOLUMNDESCFLAGS_PRECISION) )
	{
		TESTC(CurrentColInfo.GetPrecision() == pColumnDesc->bPrecision);
	}
	else
	{
		if( !fTypeChanged )
		{
			TESTC(CurrentColInfo.GetPrecision() == CachedColInfo.GetPrecision());
		}
	}

	// Scale should only be applicable to NUMERIC, VARNUMERIC, and DECIMAL types
	if( (CurrentColInfo.GetProviderType() == DBTYPE_NUMERIC ||
		CurrentColInfo.GetProviderType() == DBTYPE_VARNUMERIC) &&
		(ColumnDescFlags & DBCOLUMNDESCFLAGS_SCALE) )
	{
		TESTC(CurrentColInfo.GetScale() == pColumnDesc->bScale);
	}
	else
	{
		if( !fTypeChanged )
		{
			TESTC(CurrentColInfo.GetScale() == CachedColInfo.GetScale());
		}
	}	


	// The various properties
	if( ColumnDescFlags & DBCOLUMNDESCFLAGS_PROPERTIES )
	{
		if( m_pChangedCol )
		{
			// DBPROP_COL_ISLONG
			TESTC(CurrentColInfo.GetIsLong() == m_pChangedCol->GetIsLong());

			// DBPROP_COL_FIXEDLENGTH
			TESTC(CurrentColInfo.GetIsFixedLength() == m_pChangedCol->GetIsFixedLength());

			// DBPROP_COL_NULLABLE
			TESTC(CurrentColInfo.GetNullable() == m_pChangedCol->GetNullable());

			// DBPROP_COL_DESCRIPTION
			if(CurrentColInfo.GetColDescription(), m_pChangedCol->GetColDescription())
			{
				TESTC(0 == wcscmp(CurrentColInfo.GetColDescription(), m_pChangedCol->GetColDescription()));
			}
			else
			{
				TESTC(	CurrentColInfo.GetColDescription() == NULL &&
						m_pChangedCol->GetColDescription() == NULL);
			}

			// DBPROP_COL_COLUMNNLCID
			TESTC(CurrentColInfo.GetLCID() == m_pChangedCol->GetLCID());

			// DBPROP_COL_UNIQUE
			// Uniqueness cannot be checked thru schema information
			// Must actually try to violate unique constraint.
		}
		else
			TERROR("No change col info.");
	}
	else
	{
		if( !fTypeChanged )
		{
			// DBPROP_COL_ISLONG
			TESTC(CurrentColInfo.GetIsLong() == CachedColInfo.GetIsLong());

			// DBPROP_COL_FIXEDLENGTH
			TESTC(CurrentColInfo.GetIsFixedLength() == CachedColInfo.GetIsFixedLength());

			// DBPROP_COL_NULLABLE
			TESTC(CurrentColInfo.GetNullable() == CachedColInfo.GetNullable());

			// DBPROP_COL_DESCRIPTION
			if(CurrentColInfo.GetColDescription(), CachedColInfo.GetColDescription())
			{
				TESTC(0 == wcscmp(CurrentColInfo.GetColDescription(), CachedColInfo.GetColDescription()));
			}
			else
			{
				TESTC(	CurrentColInfo.GetColDescription() == NULL &&
						CachedColInfo.GetColDescription() == NULL);
			}

			// DBPROP_COL_UNIQUE
			// Uniqueness cannot be checked thru schema information
			// Must actually try to violate unique constraint.
		}
	}

CLEANUP:

	SAFE_RELEASE(pIColumnsInfo);

	if( pTable == NULL )
	{
		// Only restore column if the using m_pTable which must be re-used for
		// variations
		RestoreColumnID(ulColumnOrdinal, CurrentColInfo.GetColID(), ColumnDescFlags);
	}

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::MakeNewID
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIAlterTable::MakeNewID
(
	DBID *		pDBID,
	DBKINDENUM	eKind,
	size_t		ulLen
)
{	
	ASSERT(pDBID);

	WCHAR *	pwszName = NULL;
	size_t	cchName = 0;
	size_t	cIter = 0;

	memset(pDBID, 0, sizeof(DBID));
	pDBID->eKind = eKind;

	if( eKind == DBKIND_NAME )
	{
		pwszName = MakeObjectName(m_pTable->GetModuleName(), ulLen);
		if( !pwszName )
			return E_OUTOFMEMORY;

		cchName = wcslen(pwszName);
		if( cchName < ulLen)
		{
			WCHAR * pwszNewName = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(ulLen+1));
			
			if( pwszNewName == NULL)
			{
				PROVIDER_FREE(pwszName);
				return E_OUTOFMEMORY;
			}

			pwszNewName[0] = L'\0';
							
			for ( cIter = 0; cIter < ulLen - cchName; cIter += cchName)
				wcscat(pwszNewName, pwszName);
	
			wcsncat(pwszNewName, pwszName, ulLen-cIter);
			PROVIDER_FREE(pwszName);
			pwszName = pwszNewName;
			
		}
		else if( cchName > ulLen )
		{
			ASSERT(!"MakeObjectName did not honor ulLen");
		}
		
		pDBID->uName.pwszName = pwszName;

		return S_OK;
	}
	else
		return DB_E_NOTSUPPORTED;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::MakeNewTableID
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIAlterTable::MakeNewTableID
(
	DBID *		pTableID,
	DBKINDENUM	eKind,
	size_t		ulLen
)
{	
	HRESULT hr;
	BOOL	fExists = TRUE;

	TESTC_(hr = MakeNewID(pTableID, eKind, ulLen), S_OK);

	// Verify that the table does not already exist.
	TESTC_(hr = m_pTable->DoesTableExist(pTableID, &fExists), S_OK);

	// Couldn't create a new DBID
	if( fExists )
		return S_FALSE;

CLEANUP:

	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::MakeNewColumnID
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIAlterTable::MakeNewColumnID
(
	DBID *		pNewColumnID,
	DBKINDENUM	eKind,
	size_t		ulLen
)
{	
	HRESULT hr;
	BOOL	fExists = TRUE;
	ULONG	cIter;
	CCol	TempCol;

	TESTC_(hr = MakeNewID(pNewColumnID, eKind, ulLen), S_OK);

	// Verify that the table does not already have this columnID
	for ( cIter = 1; cIter <= m_pSchema->CountColumnsOnSchema(); cIter++)
	{
		TESTC_(m_pSchema->GetColInfo(cIter, TempCol), S_OK);
		
		if( CompareDBID(*TempCol.GetColID(), *pNewColumnID) )
		{
			// Couldn't create a new DBID
			hr = S_FALSE;
			goto CLEANUP;
		}
	}

	hr = S_OK;

CLEANUP:

	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::RestoreTableID
//
//	To be called to restore the table name, back to its orginal state
//  E.g.  A test variation changes the table ID to pCurrentID.
//  Then the variation should clean up after itself by changing pCurrentID
//	back to its original form, m_pTableID
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::RestoreTableID
(
	DBID *	pCurrentID 
)
{	
	HRESULT	hr;

	// Try calling AlterTable
	hr = AlterTable(pCurrentID, &m_TableID);

	// If AlterTable fails, log a failure
	COMPARE( SUCCEEDED(hr), TRUE);
	if( FAILED(hr) )
	{
		hr = E_FAIL;

		// Must be more drastic
		// Try to delete the whole table.
		m_pTable->DropTable();
		SAFE_DELETE(m_pTable);
		ReleaseDBID(&m_TableID, FALSE);
		
		m_pTable = new CTable(g_pIOpenRowset, L"IALTTAB");
		TESTC(m_pTable != NULL)
		TESTC_(hr = m_pTable->CreateTable(0,COL_ONE,NULL,PRIMARY,TRUE),S_OK);

		TESTC_(hr = DuplicateDBID(m_pTable->GetTableID(), &m_TableID), S_OK);
		m_lDBIDType = (DBKINDENUM)m_TableID.eKind;
	}
	
CLEANUP:

	return SUCCEEDED(hr);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::RestoreColumnID
//
//	To be called to restore a Column back to its original state
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::RestoreColumnID
(
	DBORDINAL			ulColumnOrdinal,
	DBID *				pCurrentID,
	DBCOLUMNDESCFLAGS	ColumnDescFlags
)
{
	TBEGIN
	HRESULT			hr;
	CCol			CachedCol;
	DBCOLUMNDESC	ColumnDesc;
	IColumnsInfo *	pIColumnsInfo = NULL;

	// Try calling AlterColumn
	TESTC_(m_pTable->GetColInfo(ulColumnOrdinal, CachedCol), S_OK);
	TESTC_(m_pTable->BuildColumnDesc(&ColumnDesc, CachedCol), S_OK);
	hr = AlterColumn(&m_pTable->GetTableID(), pCurrentID, ColumnDescFlags, &ColumnDesc);

	// If AlterColumn fails, log a failure
	COMPARE( SUCCEEDED(hr), TRUE);
	if( FAILED(hr) )
	{
		// Must be more drastic
		// Try to delete the whole table.
		m_pTable->DropTable();
		SAFE_DELETE(m_pTable);
		ReleaseDBID(&m_TableID, FALSE);
		
		m_pTable = new CTable(g_pIOpenRowset, L"IALTTAB");
		TESTC(m_pTable != NULL)
		TESTC_(m_pTable->CreateTable(0,COL_ONE,NULL,PRIMARY,TRUE),S_OK);

		TESTC_(DuplicateDBID(m_pTable->GetTableID(), &m_TableID), S_OK);
		m_lDBIDType = (DBKINDENUM)m_TableID.eKind;

		// Re-Cache column metadata using CSchema object
		SAFE_DELETE(m_pSchema);
		m_pSchema = new CTable(g_pIOpenRowset);

		// Now get the rowset which we'll use to populate the table
		TESTC_(m_pTable->CreateRowset(USE_OPENROWSET, IID_IColumnsInfo, 0, 
			NULL, (IUnknown **)&pIColumnsInfo),S_OK);
		TESTC_(m_pSchema->GetTableColumnInfo(&m_pTable->GetTableID(), pIColumnsInfo), S_OK);
	}
	
CLEANUP:

	ReleaseColumnDesc(&ColumnDesc, 1, FALSE);
	SAFE_RELEASE(pIColumnsInfo);

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::CreateTable
//
////////////////////////////////////////////////////////////////////////////
HRESULT	TCIAlterTable::CreateTable
(
	CTable **		ppTable
)
{
	HRESULT		hr = E_FAIL;
	CTable *	pTable = NULL;

	ASSERT(ppTable);

	*ppTable = NULL;

	pTable = new CTable(g_pIOpenRowset, L"IALTTAB");
	TESTC(pTable != NULL);

	TESTC_(hr = pTable->CreateTable(SIZEOF_TABLE,COL_ONE,NULL,PRIMARY,TRUE), S_OK);

CLEANUP:

	if( SUCCEEDED(hr) )
	{
		*ppTable = pTable;
	}
	else
	{
		SAFE_DELETE(pTable);
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::CreateTable
//
//	Creates a Table using ITableDefinition
//	with the requested table property
//
////////////////////////////////////////////////////////////////////////////
HRESULT	TCIAlterTable::CreateTable
(
	CTable **		ppTable,
	DBPROPID		dbPropID,
	void *			pValue,
	DBPROPOPTIONS	dwOptions
)
{
	HRESULT		hr = E_FAIL;
	CTable *	pTable = NULL;
	DBID		dbid;
	DBID *		pdbid = &dbid;
	ULONG		cPropSets = NULL;
	DBPROPSET *	rgPropSets = NULL;

	ASSERT(ppTable);

	*ppTable = NULL;

	::SetProperty(dbPropID, DBPROPSET_TABLE, &cPropSets, &rgPropSets, 
		pValue, DBTYPE_BOOL, dwOptions);

	pTable = new CTable(g_pIOpenRowset, L"IALTTAB");
	TESTC(pTable != NULL);

	pTable->SetPropertySets(rgPropSets, cPropSets);
	pTable->ResetInputTableID();
	pTable->SetDBID(&pdbid); 
	TEST2C_(hr = pTable->CreateTable(SIZEOF_TABLE,COL_ONE,NULL,PRIMARY,TRUE), S_OK, DB_E_ERRORSOCCURRED);

	// If requested property was not supported,
	// return S_FALSE to indication that the variation should not continue
	if( hr == DB_E_ERRORSOCCURRED )
	{
		hr = S_FALSE;
		SAFE_DELETE(pTable);
		goto CLEANUP;
	}

CLEANUP:

	if( pTable )
	{
		*ppTable = pTable;
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::CreateOneColTableWithProps
//
//	Creates a Table that has one column (based on the info in pCol)
//	with the requested column properties
//
////////////////////////////////////////////////////////////////////////////
HRESULT	TCIAlterTable::CreateOneColTableWithProps
(
	CTable **		ppTable,
	CCol *			pCol,
	ULONG			cPropSets,
	DBPROPSET *		rgPropSets
)
{
	HRESULT			hr = E_FAIL;
	DBCOLUMNDESC *	pColumnDesc = NULL;
	CTable *		pTable = NULL;

	ASSERT(ppTable);

	*ppTable = NULL;

	pTable = new CTable(g_pIOpenRowset, L"IALTTAB");
	TESTC(pTable != NULL);

	// Based new column attributes on pCol
	pColumnDesc = (DBCOLUMNDESC *)PROVIDER_ALLOC(sizeof(DBCOLUMNDESC));
	TESTC(pColumnDesc != NULL);
	TESTC_(hr = pTable->BuildColumnDesc(pColumnDesc, *pCol), S_OK);

	// Discard any properties and replace with the user specified properties
	::FreeProperties(&pColumnDesc->cPropertySets, &pColumnDesc->rgPropertySets);
	TESTC(DuplicatePropertySets(cPropSets, rgPropSets,
		&pColumnDesc->cPropertySets, &pColumnDesc->rgPropertySets));

	TESTC_(hr = MakeNewColumnID(&pColumnDesc->dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);

	pTable->SetBuildColumnDesc(FALSE);
	pTable->SetColumnDesc(pColumnDesc, 1);

	TESTC_(hr = pTable->CreateTable(0,0,NULL,PRIMARY,TRUE), S_OK);		

CLEANUP:

	if( pTable )
	{
		*ppTable = pTable;
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyNonTableProps
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyNonTableProps
(
	DBPROPOPTIONS	dwOptions
)
{
	TBEGIN
	HRESULT			hr;
	DBPROPSTATUS	dwExpectedStatus;
	CTable *		pTable = NULL;

	dwExpectedStatus = dwOptions == DBPROPOPTIONS_REQUIRED ? DBPROPSTATUS_NOTSUPPORTED : DBPROPSTATUS_NOTSET;

	TESTC_(hr = CreateTable(&pTable), S_OK);

	// Column Property
	TESTC(VerifyAlterInvalidTableProperties(&pTable->GetTableID(), dwExpectedStatus, 
		DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, (void *)VARIANT_TRUE, DBTYPE_BOOL, dwOptions));

	// DataSource Property
	TESTC(VerifyAlterInvalidTableProperties(&pTable->GetTableID(), dwExpectedStatus, 
		DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, (void *)VARIANT_TRUE, DBTYPE_BOOL, dwOptions));
	
	// DataSource Info Property
	TESTC(VerifyAlterInvalidTableProperties(&pTable->GetTableID(), dwExpectedStatus, DBPROP_ALTERCOLUMN, 
		DBPROPSET_DATASOURCEINFO, (void *)LONG_MAX, DBTYPE_I4, dwOptions));

	// Index Property
	TESTC(VerifyAlterInvalidTableProperties(&pTable->GetTableID(), dwExpectedStatus, DBPROP_INDEX_UNIQUE, 
		DBPROPSET_INDEX, (void *)VARIANT_FALSE, DBTYPE_BOOL, dwOptions));

	// Initialization Property
	TESTC(VerifyAlterInvalidTableProperties(&pTable->GetTableID(), dwExpectedStatus, DBPROP_AUTH_PASSWORD, 
		DBPROPSET_DBINIT, (void *)L"FOO", DBTYPE_BSTR, dwOptions));

	// Rowset Property
	TESTC(VerifyAlterInvalidTableProperties(&pTable->GetTableID(), dwExpectedStatus, DBPROP_IRowset, 
		DBPROPSET_ROWSET, (void *)VARIANT_TRUE, DBTYPE_BOOL, dwOptions));

	// Session Property
	TESTC(VerifyAlterInvalidTableProperties(&pTable->GetTableID(), dwExpectedStatus, DBPROP_SESS_AUTOCOMMITISOLEVELS, 
		DBPROPSET_SESSION, (void *)0, DBTYPE_I4, dwOptions));

	// View Property
	TESTC(VerifyAlterInvalidTableProperties(&pTable->GetTableID(), dwExpectedStatus, DBPROP_MAXORSINFILTER, 
		DBPROPSET_VIEW, (void *)1, DBTYPE_I4, dwOptions));

	// Table Property with Bad PROPSET GUID
	TESTC(VerifyAlterInvalidTableProperties(&pTable->GetTableID(), dwExpectedStatus, DBPROP_TBL_TEMPTABLE, 
		DBPROPSET_COLUMN, (void *)VARIANT_TRUE, DBTYPE_BOOL, dwOptions));

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::SetChangeColInfo
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIAlterTable::SetChangeColInfo
(
	CCol *			pCol
)
{
	m_pChangedCol = pCol;

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::GetReadOnlySession
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::GetReadOnlySession
(
	IUnknown **		ppUnkReadSession
)
{
	TBEGIN
	ULONG				cPropSets;
	DBPROPSET *			rgPropSets = NULL;
	IDBInitialize *		pIDBInitialize = NULL;
	IDBProperties *		pIDBProperties = NULL;

	ASSERT(ppUnkReadSession != NULL);

	*ppUnkReadSession = NULL;
	
	//Obtain a new uninitialized DSO
	TESTC_(CreateNewDSO(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_NONE), S_OK);

	TESTC(VerifyInterface(pIDBInitialize, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown **)&pIDBProperties));

	//Build our init options from string passed to us from the LTM InitString
	GetInitProps(&cPropSets,&rgPropSets);

	// Add Property DBPROP_INIT_MODE
	::SetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, &cPropSets,
		&rgPropSets, (void *)DB_MODE_READ, DBTYPE_I4, DBPROPOPTIONS_REQUIRED);

	TESTC_(QI(pIDBInitialize, IID_IDBProperties, (void**)&pIDBProperties), S_OK)
	QTESTC_(pIDBProperties->SetProperties(cPropSets,rgPropSets), S_OK)

	QTESTC_(pIDBInitialize->Initialize(),S_OK)

	//Obtain IAlterTable
	TESTC_(CreateNewSession(pIDBInitialize, IID_IUnknown, ppUnkReadSession), S_OK);
	
CLEANUP:

	::FreeProperties(&cPropSets, &rgPropSets);

	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIDBProperties);

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::AlterPropertyAndVerifyRowset
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::AlterPropertyAndVerifyRowset
(
	DBPROPID		dbPropID
)
{
	TBEGIN
	HRESULT				hr;
	DBORDINAL			ulOrdinal = 0;
	CCol				col;
	CCol				onetablecol;
	DBCOLUMNDESCFLAGS	ColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES;
	CTable *			pTable = NULL;

	ASSERT(dbPropID == DBPROP_COL_UNIQUE || dbPropID == DBPROP_COL_PRIMARYKEY ||
		dbPropID == DBPROP_COL_DEFAULT || dbPropID == DBPROP_COL_AUTOINCREMENT );

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_PROPERTIES) == 0 )
		return TEST_SKIPPED;

	// For each column in our Table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		DBCOLUMNDESC *	pColumnDesc = NULL;
		DBCOLUMNDESC	AlterColumnDesc;
		CCol			onetablecol;

		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);

		if( dbPropID == DBPROP_COL_AUTOINCREMENT )
		{
			if( !col.CanAutoInc() )
				continue;
		}
		else if( dbPropID == DBPROP_COL_DEFAULT )
		{
			if( !col.GetUpdateable() )
				continue;
		}
		else
		{
			if( col.GetIsLong() || col.GetAutoInc() || !col.GetUpdateable() || col.GetProviderType() == DBTYPE_BOOL )
				continue;
		}

		odtLog << "Changing Property: " << GetPropertyName(dbPropID, DBPROPSET_COLUMN) << " on type: "<< col.GetProviderTypeName() << ENDL;

		pColumnDesc = (DBCOLUMNDESC *)PROVIDER_ALLOC(sizeof(DBCOLUMNDESC));
		TESTC(pColumnDesc != NULL);
		TESTC_(m_pTable->BuildColumnDesc(pColumnDesc, col), S_OK);

		TESTC_(MakeNewColumnID(&pColumnDesc->dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);

		// Create a fresh table for this variation
		pTable = new CTable(g_pIOpenRowset, L"IALTTAB");
		TESTC(pTable != NULL);		

		pColumnDesc->ulColumnSize = 1;

		if( dbPropID == DBPROP_COL_PRIMARYKEY )
		{
			::SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, &pColumnDesc->cPropertySets, 
				&pColumnDesc->rgPropertySets, (void *)VARIANT_FALSE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
		}

		pTable->SetBuildColumnDesc(FALSE);
		pTable->SetColumnDesc(pColumnDesc, 1);

		TESTC_(pTable->CreateTable(0,0,NULL,PRIMARY,TRUE), S_OK);		
		TESTC_(pTable->GetColInfo(1, onetablecol), S_OK);

		memset(&AlterColumnDesc, 0, sizeof(DBCOLUMNDESC));	

		// Set Appropriate Property
		if( dbPropID == DBPROP_COL_DEFAULT )
		{
			DBTYPE		wType;
			void *		pData = NULL;
	
			TESTC(MakeDefaultData(&onetablecol, &pData, &wType, pTable));
	
			::SetProperty(DBPROP_COL_DEFAULT, DBPROPSET_COLUMN, &AlterColumnDesc.cPropertySets, 
				&AlterColumnDesc.rgPropertySets, pData, wType, DBPROPOPTIONS_REQUIRED);

			SAFE_FREE(pData);
		}
		else
		{
			::SetProperty(dbPropID, DBPROPSET_COLUMN, &AlterColumnDesc.cPropertySets, 
				&AlterColumnDesc.rgPropertySets, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
		}

		hr = AlterColumn(&pTable->GetTableID(), onetablecol.GetColID(),
					ColumnDescFlags, &AlterColumnDesc);

		if( hr == S_OK )
		{
			if( dbPropID == DBPROP_COL_UNIQUE || dbPropID == DBPROP_COL_PRIMARYKEY )
			{
				// Insert a row using IRowsetChange::InsertRow
				if(CHECK(pTable->Insert(PRIMARY, 1), S_OK))
				{
					// Insert the same row again to violate UNIQUE constraint.
					hr = pTable->Insert(PRIMARY, 1);
					COMPARE(hr == DB_E_INTEGRITYVIOLATION || hr == DB_E_ERRORSOCCURRED, TRUE);
				}
			}
			else
			{
				COMPARE(VerifyRowsetValues(pTable, dbPropID), TRUE);			
			}
		}
		else
		{
			CHECK(hr, DB_E_ERRORSOCCURRED);
			COMPARE(CheckUnsupportedProp(AlterColumnDesc.cPropertySets, AlterColumnDesc.rgPropertySets), TRUE);
		}

		if( pTable )
		{
			pTable->DropTable();
			SAFE_DELETE(pTable);
		}
	}
	
CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyRowsetValues
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyRowsetValues
(
	CTable *	pTable,
	DBPROPID	dbPropID
)
{
	TBEGIN
	HRESULT			hr;
	const ULONG		cAutoInc = 2;
	CRowsetChange	Rowset;	
	HACCESSOR		hAccessor;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING *		rgBindings = NULL;
	BYTE *			pData = NULL;
	HROW 			rghRow[cAutoInc];
	void *			rgpData[cAutoInc];
	ULONG			cIter;

	memset(rgpData, 0, sizeof(rgpData));

	Rowset.SetTable(pTable, DELETETABLE_NO);
	Rowset.SetSettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET);

	TESTC_(Rowset.CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, pTable, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, BLOB_LONG), S_OK);

	TESTC_(Rowset.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS,
		&cBindings,	&rgBindings, NULL, BLOB_LONG, NULL, ALL_COLS_BOUND | NOBOOKMARK_COLS_BOUND,
		NO_COLS_BY_REF, DBTYPE_EMPTY), S_OK);
	TESTC(cBindings == 1);
	TESTC_(hr = FillInputBindings(pTable, DBACCESSOR_ROWDATA, cBindings, rgBindings,	
		&pData,	1, 0, NULL),S_OK);

	if( dbPropID == DBPROP_COL_DEFAULT )
	{
		*(DBSTATUS *)(pData+rgBindings[0].obStatus) = DBSTATUS_S_DEFAULT;
		TESTC_(Rowset.pIRowsetChange()->InsertRow(NULL, hAccessor, pData, NULL), S_OK);
	}
	else if( dbPropID == DBPROP_COL_AUTOINCREMENT )
	{
		*(DBSTATUS *)(pData+rgBindings[0].obStatus) = DBSTATUS_S_IGNORE;

		// Insert the autoinc values
		for( cIter = 0; cIter < cAutoInc; cIter++ )
		{
			TESTC_(Rowset.pIRowsetChange()->InsertRow(NULL, hAccessor, pData, NULL), S_OK);
		}
	}

	TESTC(SUCCEEDED(Rowset.pIRowset()->RestartPosition(NULL)));

	if( dbPropID == DBPROP_COL_DEFAULT )
	{
		TESTC(Rowset.VerifyAllRows());
	}
	else if( dbPropID == DBPROP_COL_AUTOINCREMENT )
	{
		TESTC_(Rowset.GetNextRows(cAutoInc, rghRow), S_OK);
		TESTC_(Rowset.GetRowData(cAutoInc, rghRow, rgpData), S_OK);

		// Assume seed to 1 and increment to be 1 by default.
		LONG	lAutoIncVal = 1;
		LONG	lInc = 1;

		for( cIter = 0; cIter < cAutoInc; cIter++, lAutoIncVal += lInc )
		{
			WCHAR	wszAutoInc[80];
			void *	pBackEndData = NULL;

			_ltow(lAutoIncVal, wszAutoInc, 10);
			pBackEndData = WSTR2DBTYPE(wszAutoInc, rgBindings[0].wType, NULL);
			TESTC(pBackEndData != NULL);

			TESTC(*(DBSTATUS *)((BYTE *)rgpData[cIter]+rgBindings[0].obStatus) == DBSTATUS_S_OK);

			TESTC(CompareDBTypeData((BYTE *)rgpData[cIter]+rgBindings[0].obValue, pBackEndData, rgBindings[0].wType,
				0, rgBindings[0].bPrecision, rgBindings[0].bScale,	NULL));
		}
	}

CLEANUP:

	SAFE_FREE(rgBindings);
	Rowset.ReleaseRowData(pData, hAccessor);

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::UpgradeType
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::UpgradeType
(
	CCol *		pCurrentCol,
	CCol *		pNewCol,
	BOOL		fAllowLong
)
{
	TBEGIN
	DBORDINAL	ulOrdinal;
	CCol		schemacol;
	CCol		tablecol;

	ASSERT(pCurrentCol && pNewCol);

	for( ulOrdinal = 1; ulOrdinal <= m_pSchema->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pSchema->GetColInfo(ulOrdinal, schemacol), S_OK);

		if( IsTypeGreater(pCurrentCol, &schemacol) )
		{
			if( fAllowLong || !schemacol.GetIsLong() )
			{
				// Copy the new information
				*pNewCol = schemacol;

				TESTC_(m_pTable->GetColInfo(ulOrdinal, tablecol), S_OK);

				pNewCol->SetProviderTypeName(tablecol.GetProviderTypeName());
				return TRUE;
			}
		}
	}

CLEANUP:

	// no compatible type found
	return FALSE;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::IsTypeGreater
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::IsTypeGreater
(
	CCol *	pCurCol,
	CCol *	pNewCol
)
{
	TBEGIN
	ASSERT(pCurCol && pNewCol);

	ULONG	CurColClass = GetTypeClass(pCurCol->GetProviderType());
	ULONG	NewColClass = GetTypeClass(pNewCol->GetProviderType());

	if( CurColClass == CLASS_SPECIAL )
		return FALSE;

	if( CurColClass == NewColClass )
	{
		ULONG	CurColRank = GetTypeRank(pCurCol->GetProviderType());
		ULONG	NewColRank = GetTypeRank(pNewCol->GetProviderType());

		if( CurColRank > NewColRank )
			return FALSE;

		if( CurColRank == NewColRank )
		{
			if( pCurCol->GetProviderType() == pNewCol->GetProviderType() &&
				pCurCol->GetIsFixedLength() == pNewCol->GetIsFixedLength() &&
				pCurCol->GetIsLong() == pNewCol->GetIsLong() )
			{
				return FALSE;
			}

			if( pCurCol->GetProviderType() == DBTYPE_CY || pCurCol->GetProviderType() == DBTYPE_DBTIMESTAMP )
			{
				if( pCurCol->GetPrecision() <= pNewCol->GetPrecision() )
					return TRUE;
				else
					return FALSE;
			}
			else
			{
				return TRUE;
			}
		}

		return TRUE;
	}
	else
		return FALSE;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::GetTypeClass
//
////////////////////////////////////////////////////////////////////////////
ULONG TCIAlterTable::GetTypeClass
(
	DBTYPE	wType
)
{
	switch( wType )
	{
	case DBTYPE_I1:
	case DBTYPE_I2:
	case DBTYPE_I4:
	case DBTYPE_I8:
		return CLASS_SIGNEDINT;
	case DBTYPE_UI1:
	case DBTYPE_UI2:
	case DBTYPE_UI4:
	case DBTYPE_UI8:
		return CLASS_UNSIGNEDINT;
	case DBTYPE_R4:
	case DBTYPE_R8:
		return CLASS_APPROXNUM;
	case DBTYPE_CY:
	case DBTYPE_DECIMAL:
	case DBTYPE_NUMERIC:
	case DBTYPE_VARNUMERIC:
		return CLASS_SCALEDINT;
	case DBTYPE_DBTIME:
	case DBTYPE_DBDATE:
	case DBTYPE_DATE:
	case DBTYPE_DBTIMESTAMP:
		return CLASS_DATE;
	case DBTYPE_STR:
	case DBTYPE_WSTR:
	case DBTYPE_BSTR:
		return CLASS_STRING;
	default:
		return CLASS_SPECIAL;
	}
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::GetTypeRank
//
////////////////////////////////////////////////////////////////////////////
ULONG TCIAlterTable::GetTypeRank
(
	DBTYPE	wType
)
{
	switch( wType )
	{
	// signed int
	case DBTYPE_I1:
		return 1;
	case DBTYPE_I2:
		return 2;
	case DBTYPE_I4:
		return 3;
	case DBTYPE_I8:
		return 4;

	// unsigned int
	case DBTYPE_UI1:
		return 1;
	case DBTYPE_UI2:
		return 2;
	case DBTYPE_UI4:
		return 3;
	case DBTYPE_UI8:
		return 4;

	// approx num
	case DBTYPE_R4:
		return 1;
	case DBTYPE_R8:
		return 2;

	// scaled int
	case DBTYPE_CY:
		return 1;
	case DBTYPE_DECIMAL:
		return 2;
	case DBTYPE_NUMERIC:
		return 3;
	case DBTYPE_VARNUMERIC:
		return 4;
	
	// date
	case DBTYPE_DBTIME:
		return 1;
	case DBTYPE_DBDATE:
		return 2;
	case DBTYPE_DATE:
		return 3;
	case DBTYPE_DBTIMESTAMP:
		return 4;

	// string
	case DBTYPE_STR:
		return 1;
	case DBTYPE_WSTR:
		return 2;
	case DBTYPE_BSTR:
		return 3;

	default:
		return 0;
	}
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::MakeDefaultData
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::MakeDefaultData
(
	CCol *		pCol,
	void **		ppData,
	DBTYPE *	pwType,
	CTable *	pTable
)
{
	TBEGIN
	HRESULT		hr;
	DBTYPE		wType;
	void *		pData = NULL;
	VARIANT *	pVar = NULL;
	DBLENGTH	cb = 0;
	WCHAR		wszData[(MAX_COL_SIZE+1)*sizeof(WCHAR)];
	DBTYPE		vt;

	ASSERT(pCol && ppData && pwType && pTable);

	*ppData = NULL;
	*pwType = INVALID_DBTYPE;

	TESTC_(pCol->MakeData(wszData, 1, PRIMARY, USENULLS, &vt, 0), S_OK);

	if( pCol->GetProviderType() == DBTYPE_VARIANT )
	{
		pData = WSTR2DBTYPE_EX(wszData, vt, &cb);			
		hr = MapDBTYPE2VARIANT(pData, vt, cb, &pVar);	
	}
	else
	{
		pData = WSTR2DBTYPE_EX(wszData, pCol->GetProviderType(), &cb);			
		hr = MapDBTYPE2VARIANT(pData, pCol->GetProviderType(), cb, &pVar);	
	}	

	if( S_OK == hr )
	{
		SAFE_FREE(pData);
		pData = pVar;

		wType = DBTYPE_VARIANT;
	}
	else
	{
		WCHAR *	pwszFormattedData = NULL;

		TESTC_(hr, S_FALSE);

		TESTC_(pTable->GetLiteralAndValue(*pCol, &pwszFormattedData, 1, pCol->GetColNum(), PRIMARY, TRUE), S_OK);

		pData = wcsDuplicate(pwszFormattedData);
		wType = DBTYPE_BSTR;
	}

	if( pData )
	{
		*ppData = pData;
		*pwType = wType;
	}

CLEANUP:

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::ReleaseDefaultData
//
////////////////////////////////////////////////////////////////////////////
void TCIAlterTable::ReleaseDefaultData
(
	void *		pDefaultData,
	DBTYPE		wDefaultType
)
{
	if( pDefaultData && wDefaultType == DBTYPE_VARIANT )
	{
		VariantClear((VARIANT *)pDefaultData);
	}

	SAFE_FREE(pDefaultData);
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyTableDefault
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyTableDefault
(
	CTable *	pTable,
	BOOL		fDefaultExists
)
{
	TBEGIN
	HRESULT			hr;
	BOOL			fPass = FALSE;
	BOOL			fCheckDefault = FALSE;
	CRowsetChange	Rowset;	
	HACCESSOR		hAccessor;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING *		rgBindings = NULL;
	BYTE *			pData = NULL;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW 			rghRow[1];

	Rowset.SetTable(pTable, DELETETABLE_NO);
	Rowset.SetSettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET);

	TESTC_(Rowset.CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, pTable, DBACCESSOR_ROWDATA,
		DBPART_ALL, ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, BLOB_LONG), S_OK);

	TESTC_(Rowset.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS,
		&cBindings,	&rgBindings, NULL, BLOB_LONG, NULL, ALL_COLS_BOUND | NOBOOKMARK_COLS_BOUND,
		NO_COLS_BY_REF, DBTYPE_EMPTY), S_OK);

	TESTC(cBindings == 1);
	TESTC_(hr = FillInputBindings(pTable, DBACCESSOR_ROWDATA, cBindings, rgBindings,	
		&pData,	1, 0, NULL),S_OK);

	*(DBSTATUS *)(pData+rgBindings[0].obStatus) = DBSTATUS_S_DEFAULT;	

	hr = Rowset.pIRowsetChange()->InsertRow(NULL, hAccessor, pData, NULL);
	if( FAILED(hr) )
	{
		// InsertRow could fail if default value does not exist
		if( fDefaultExists == FALSE )
			fPass = TRUE;

		// Verification complete
		goto CLEANUP;
	}

	TESTC(SUCCEEDED(Rowset.pIRowset()->RestartPosition(NULL)));

	SAFE_FREE(pData);

	TESTC_(Rowset.GetNextRows(1, rghRow), S_OK);
	TESTC_(Rowset.GetRowData(1, rghRow, (void **)&pData), S_OK);

	fCheckDefault = CompareData(1, NULL, 1, pData, cBindings, rgBindings, pTable, NULL, PRIMARY, COMPARE_ONLY);

	fPass = (fCheckDefault == fDefaultExists);

CLEANUP:

	SAFE_FREE(rgBindings)
	Rowset.ReleaseRowData(pData, hAccessor);
	Rowset.ReleaseAccessor(hAccessor);

	//Rowset.DropRowset();

	CHECK(pTable->DeleteRows(ALLROWS), S_OK);
	
	return fPass;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::VerifyTableUnique
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::VerifyTableUnique
(
	CTable *	pTable,
	BOOL		fUniqueExists
)
{
	HRESULT	hr;
	BOOL	fInsertFailed;

	ASSERT(pTable);

	// Insert a row using IRowsetChange::InsertRow
	if( FAILED(pTable->Insert(PRIMARY, 1)) )
		return FALSE;

	// Insert the same row again 
	hr = pTable->Insert(PRIMARY, 1);
	fInsertFailed = FAILED(hr);

	return (fInsertFailed == fUniqueExists);
}

BOOL TCIAlterTable::AlterStringCol2BLOB
(
	DBTYPE	wTargetType,
	BOOL	fIsFixed
)
{
	TBEGIN
	DBORDINAL			ulOrdinal;
	DBORDINAL			ulNonLongStrCol = 0;
	DBORDINAL			ulLongStrCol = 0;
	CCol				schemacol;
	CCol				schemacolBLOB;
	CTable *			pTable = NULL;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_WTYPE | DBCOLUMNDESCFLAGS_PROPERTIES;

	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;

	memset(&ColumnDesc, 0, sizeof(DBCOLUMNDESC));

	for( ulOrdinal = 1; ulOrdinal <= m_pSchema->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pSchema->GetColInfo(ulOrdinal, schemacol), S_OK);

		if( schemacol.GetProviderType() == wTargetType &&
			schemacol.GetIsFixedLength() == fIsFixed &&
			!schemacol.GetIsLong() )
		{
			ulNonLongStrCol = ulOrdinal;
			break;
		}
	}

	if( ulNonLongStrCol == 0 )
	{
		odtLog << "Non blob version of " << GetDBTypeName(wTargetType) << " was not found." << ENDL;
		return TEST_SKIPPED;
	}

	for( ulOrdinal = 1; ulOrdinal <= m_pSchema->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pSchema->GetColInfo(ulOrdinal, schemacolBLOB), S_OK);

		if( schemacolBLOB.GetProviderType() == wTargetType &&
			schemacolBLOB.GetIsLong() )
		{
			ulLongStrCol = ulOrdinal;
			break;
		}
	}

	if( ulLongStrCol == 0 )
	{
		odtLog << "Blob version of " << GetDBTypeName(wTargetType) << " was not found." << ENDL;
		return TEST_SKIPPED;
	}

	// Create a table based on schemacol
	TESTC_(CreateOneColTableWithProps(&pTable, &schemacol, 0, NULL), S_OK);		

	TESTC_(SetChangeColInfo(&schemacolBLOB), S_OK);
	
	ColumnDesc.wType = schemacolBLOB.GetProviderType();
	
	// Set FixedLength FALSE 
	// Set IsLong TRUE
	::SetProperty(DBPROP_COL_FIXEDLENGTH, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
		&ColumnDesc.rgPropertySets, (void *)(VARIANT_FALSE),
		DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	::SetProperty(DBPROP_COL_ISLONG, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
		&ColumnDesc.rgPropertySets, (void *)(VARIANT_TRUE),
		DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
	
	COMPARE(VerifyAlterColumn(1, dwColumnDescFlags, &ColumnDesc, pTable), TRUE);

CLEANUP:

	FreeProperties(&ColumnDesc.cPropertySets, &ColumnDesc.rgPropertySets);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::TCAlterCompareDBID
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIAlterTable::TCAlterCompareDBID
(
	DBID * pdbid1,			
	DBID * pdbid2			
)
{
	BOOL	fRes = FALSE;

	ASSERT( pdbid1 && pdbid2 );

	if( (pdbid1->eKind == DBKIND_GUID_NAME ||
		pdbid1->eKind == DBKIND_PGUID_NAME ||
		pdbid1->eKind == DBKIND_NAME) &&
		(pdbid2->eKind == DBKIND_GUID_NAME ||
		pdbid2->eKind == DBKIND_PGUID_NAME ||
		pdbid2->eKind == DBKIND_NAME) )
	{		
		// just compare the name portion
		TESTC_(CompareID(&fRes, (pdbid1->uName.pwszName), 
			(pdbid2->uName.pwszName), NULL), S_OK);
				return fRes;
	}
	else
	{
		fRes = CompareDBID(*pdbid1, *pdbid2);
	}

CLEANUP:
	return fRes;
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::Thread_AlterTable
//
////////////////////////////////////////////////////////////////////////////
ULONG TCIAlterTable::Thread_AlterTable(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	TCIAlterTable *	pThis = (TCIAlterTable*)THREAD_FUNC;
	HRESULT *		phrres = (HRESULT *)THREAD_ARG1;
	DBID *			pTableID = (DBID *)THREAD_ARG2;
	DBID *			pNewTableID = (DBID *)THREAD_ARG3;

	ASSERT(pThis && phrres && pTableID && pNewTableID);

	ThreadSwitch(); //Let the other thread(s) catch up

	*phrres = pThis->AlterTable(pTableID, pNewTableID);
	
	ThreadSwitch(); //Let the other thread(s) catch up

	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIAlterTable::Thread_AlterColumn
//
////////////////////////////////////////////////////////////////////////////
ULONG TCIAlterTable::Thread_AlterColumn(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	TCIAlterTable *	pThis = (TCIAlterTable*)THREAD_FUNC;
	DBORDINAL		ulOrdinal = (DBORDINAL)THREAD_ARG1;
	DBID *			pColumnID = (DBID *)THREAD_ARG2;

	ASSERT(pThis && pColumnID);

	ThreadSwitch(); //Let the other thread(s) catch up

	COMPARE(pThis->VerifyAlterColumnName(ulOrdinal, pColumnID), TRUE);
	
	ThreadSwitch(); //Let the other thread(s) catch up

	THREAD_RETURN
}


// {{ TCW_TEST_CASE_MAP(TCAlterTable)
//*-----------------------------------------------------------------------
// @class TestCases for AlterTable method
//
class TCAlterTable : public TCIAlterTable { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAlterTable,TCIAlterTable);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Connect with DBPROP_INIT_MODE requesting read only access.  Verify AlterTable return DB_SEC_E_PERMISSIONDENIED
	int Variation_1();
	// @cmember Use AlterTable to change name of view
	int Variation_2();
	// @cmember Use Alter Table to change name of non-table types. Try data type and column names
	int Variation_3();
	// @cmember Use AlterTable when command are active and there are open rowsets, though not on the table to be altered.
	int Variation_4();
	// @cmember Call AlterTable when an updatable rowset is open on the table to be altered
	int Variation_5();
	// @cmember Call AlterTable when a FO/RO rowset is open on the table to be altered
	int Variation_6();
	// @cmember pTableID is NULL.  Verify E_INVALIDARG
	int Variation_7();
	// @cmember cPropertySets = 1 and rgPropertySet is NULL.  Verify E_INVALIDARG
	int Variation_8();
	// @cmember In rgPropertySets, cProperties = 1 and rgProperties is NULL.  Verify E_INVALIDARG
	int Variation_9();
	// @cmember pTableID does not exist.  Verify DB_E_NOTABLE
	int Variation_10();
	// @cmember pTableID is a null string.  Verify DB_E_NOTABLE
	int Variation_11();
	// @cmember pTableID is an empty string.  Verify DB_E_NOTABLE
	int Variation_12();
	// @cmember pTableID has an invalid DBKIND.  Verify DB_E_NOTABLE
	int Variation_13();
	// @cmember pNewTableID already exists.   Verify DB_E_DUPLICATETABLEID.
	int Variation_14();
	// @cmember pNewTableID is a null string.  Verify DB_E_BADTABLEID.
	int Variation_15();
	// @cmember pNewTableID is an empty string.  Verify DB_E_BADTABLEID
	int Variation_16();
	// @cmember pNewTableID contains invalid characters for a table id.  Verify DB_E_BADTABLEID.
	int Variation_17();
	// @cmember valid pTableID.  pNewTableID is NULL, cPropertySets =0, rgPropertySets is NULL.  Verify S_OK and no-op.
	int Variation_18();
	// @cmember valid pTableID, pNewTableID = pTableID, cPropertySets=0, rgPropertySets=NULL.  Verify S_OK.
	int Variation_19();
	// @cmember Use fully qualified pTableID
	int Variation_20();
	// @cmember Use quoted pTableID
	int Variation_21();
	// @cmember valid pTableID, valid pNewTableID, cPropertySets=0, rgPropertySets=NULL.  Verify pTableID no longer exists and pNewTableID exists.
	int Variation_22();
	// @cmember Create temp table.  Alter Table to optionally unset property
	int Variation_23();
	// @cmember Create temp table.  Alter table to unset that property using REQUIRED option
	int Variation_24();
	// @cmember Create normal table.  Alter table to set TEMPTABLE property using REQUIRED option.
	int Variation_25();
	// @cmember Create normal table.  Alter table to set TEMPTABLE property using OPTIONAL option.
	int Variation_26();
	// @cmember Request a non TABLE property group with OPTIONAL flag.  Verify DB_S_ERRORSOCCURRED.
	int Variation_27();
	// @cmember Request a non TABLE property group with REQUIRED flag.  Verify DB_E_ERRORSOCCURRED.
	int Variation_28();
	// @cmember Request a property OPTIONAL where the type in vValue of the DBPROP structure did not match the type of the property.  Verify DB_S_ERRORSOCCURRED.
	int Variation_29();
	// @cmember Request a property REQUIRED where the type in vValue of the DBPROP structure did not match the type of the property.  Verify DB_E_ERRORSOCCURRED.
	int Variation_30();
	// @cmember Request DBPROP_TBL_TEMPTABLE with vValue type = VT_EMPTY.  Verify success.
	int Variation_31();
	// @cmember cPropertySets = 0, rgPropertySets not null.  Verify rgPropertySets is ignored.
	int Variation_32();
	// @cmember pNewTableID contains invalid starting characters for a tableid.  Verify DB_E_BADTABLEID
	int Variation_33();
	// @cmember pNewTableID exceeds maximum table name length.  Verify DB_E_BADTABLEID.
	int Variation_34();
	// @cmember Request bad property option.  Verify DB_E_ERRORSOCCURRED
	int Variation_35();
	// @cmember Request, optional, an invalid value of TEMPTABLE
	int Variation_36();
	// @cmember Request, required, an invalid value of TEMPTABLE
	int Variation_37();
	// @cmember Optional prop, colid was not DB_NULLID.  Verify DB_S_ERRORSOCCURRED
	int Variation_38();
	// @cmember Required prop, colid was not DB_NULLID.  Verify DB_E_ERRORSOCCURRED
	int Variation_39();
	// @cmember Use AlterTable to change table name.  Verify using TABLES schema rowset
	int Variation_40();
	// @cmember Mult Threads changing table id.  Verify that only one thread succeeds.
	int Variation_43();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCAlterTable)
#define THE_CLASS TCAlterTable
BEG_TEST_CASE(TCAlterTable, TCIAlterTable, L"TestCases for AlterTable method")
	TEST_VARIATION(1, 		L"Connect with DBPROP_INIT_MODE requesting read only access.  Verify AlterTable return DB_SEC_E_PERMISSIONDENIED")
	TEST_VARIATION(2, 		L"Use AlterTable to change name of view")
	TEST_VARIATION(3, 		L"Use Alter Table to change name of non-table types. Try data type and column names")
	TEST_VARIATION(4, 		L"Use AlterTable when command are active and there are open rowsets, though not on the table to be altered.")
	TEST_VARIATION(5, 		L"Call AlterTable when an updatable rowset is open on the table to be altered")
	TEST_VARIATION(6, 		L"Call AlterTable when a FO/RO rowset is open on the table to be altered")
	TEST_VARIATION(7, 		L"pTableID is NULL.  Verify E_INVALIDARG")
	TEST_VARIATION(8, 		L"cPropertySets = 1 and rgPropertySet is NULL.  Verify E_INVALIDARG")
	TEST_VARIATION(9, 		L"In rgPropertySets, cProperties = 1 and rgProperties is NULL.  Verify E_INVALIDARG")
	TEST_VARIATION(10, 		L"pTableID does not exist.  Verify DB_E_NOTABLE")
	TEST_VARIATION(11, 		L"pTableID is a null string.  Verify DB_E_NOTABLE")
	TEST_VARIATION(12, 		L"pTableID is an empty string.  Verify DB_E_NOTABLE")
	TEST_VARIATION(13, 		L"pTableID has an invalid DBKIND.  Verify DB_E_NOTABLE")
	TEST_VARIATION(14, 		L"pNewTableID already exists.   Verify DB_E_DUPLICATETABLEID.")
	TEST_VARIATION(15, 		L"pNewTableID is a null string.  Verify DB_E_BADTABLEID.")
	TEST_VARIATION(16, 		L"pNewTableID is an empty string.  Verify DB_E_BADTABLEID")
	TEST_VARIATION(17, 		L"pNewTableID contains invalid characters for a table id.  Verify DB_E_BADTABLEID.")
	TEST_VARIATION(18, 		L"valid pTableID.  pNewTableID is NULL, cPropertySets =0, rgPropertySets is NULL.  Verify S_OK and no-op.")
	TEST_VARIATION(19, 		L"valid pTableID, pNewTableID = pTableID, cPropertySets=0, rgPropertySets=NULL.  Verify S_OK.")
	TEST_VARIATION(20, 		L"Use fully qualified pTableID")
	TEST_VARIATION(21, 		L"Use quoted pTableID")
	TEST_VARIATION(22, 		L"valid pTableID, valid pNewTableID, cPropertySets=0, rgPropertySets=NULL.  Verify pTableID no longer exists and pNewTableID exists.")
	TEST_VARIATION(23, 		L"Create temp table.  Alter Table to optionally unset property")
	TEST_VARIATION(24, 		L"Create temp table.  Alter table to unset that property using REQUIRED option")
	TEST_VARIATION(25, 		L"Create normal table.  Alter table to set TEMPTABLE property using REQUIRED option.")
	TEST_VARIATION(26, 		L"Create normal table.  Alter table to set TEMPTABLE property using OPTIONAL option.")
	TEST_VARIATION(27, 		L"Request a non TABLE property group with OPTIONAL flag.  Verify DB_S_ERRORSOCCURRED.")
	TEST_VARIATION(28, 		L"Request a non TABLE property group with REQUIRED flag.  Verify DB_E_ERRORSOCCURRED.")
	TEST_VARIATION(29, 		L"Request a property OPTIONAL where the type in vValue of the DBPROP structure did not match the type of the property.  Verify DB_S_ERRORSOCCURRED.")
	TEST_VARIATION(30, 		L"Request a property REQUIRED where the type in vValue of the DBPROP structure did not match the type of the property.  Verify DB_E_ERRORSOCCURRED.")
	TEST_VARIATION(31, 		L"Request DBPROP_TBL_TEMPTABLE with vValue type = VT_EMPTY.  Verify success.")
	TEST_VARIATION(32, 		L"cPropertySets = 0, rgPropertySets not null.  Verify rgPropertySets is ignored.")
	TEST_VARIATION(33, 		L"pNewTableID contains invalid starting characters for a tableid.  Verify DB_E_BADTABLEID")
	TEST_VARIATION(34, 		L"pNewTableID exceeds maximum table name length.  Verify DB_E_BADTABLEID.")
	TEST_VARIATION(35, 		L"Request bad property option.  Verify DB_E_ERRORSOCCURRED")
	TEST_VARIATION(36, 		L"Request, optional, an invalid value of TEMPTABLE")
	TEST_VARIATION(37, 		L"Request, required, an invalid value of TEMPTABLE")
	TEST_VARIATION(38, 		L"Optional prop, colid was not DB_NULLID.  Verify DB_S_ERRORSOCCURRED")
	TEST_VARIATION(39, 		L"Required prop, colid was not DB_NULLID.  Verify DB_E_ERRORSOCCURRED")
	TEST_VARIATION(40, 		L"Use AlterTable to change table name.  Verify using TABLES schema rowset")
	TEST_VARIATION(43, 		L"Mult Threads changing table id.  Verify that only one thread succeeds.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCAlterColumn)
//*-----------------------------------------------------------------------
// @class Test Cases for AlterColumn method
//
class TCAlterColumn : public TCIAlterTable { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAlterColumn,TCIAlterTable);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Connect with DBPROP_INIT_MODE read only access.  Verify DB_SEC_E_PERMISSIONDENIED when altering a column
	int Variation_1();
	// @cmember Try to use AlterColumn to change a table name. Verify failure
	int Variation_2();
	// @cmember Call AlterColumn with multiple commands active which have open rowsets, though not on the table to be altered.
	int Variation_3();
	// @cmember Use AlterColumn when a rowset is open on the table to be altered.  Verify DB_E_TABLEINUSE, or if successful, verify DB_S_COLUMNSCHANGE on restart position
	int Variation_4();
	// @cmember Use AlterColumn when a FO/RO rowset is open on the table to be altered.  Verify DB_E_TABLEINUSE, or if successful, verify DB_S_COLUMNSCHANGE on restart position
	int Variation_5();
	// @cmember pTableID is NULL.  Verify E_INVALIDARG.
	int Variation_6();
	// @cmember pColumnID is NULL. Verify E_INVALIDARG
	int Variation_7();
	// @cmember pColumnDesc is NULL.  Verify E_INVALIDARG.
	int Variation_8();
	// @cmember pTableID does not exist.  Verify DB_E_NOTABLE
	int Variation_9();
	// @cmember pColumnID does not exist.  Verify DB_E_NOCOLUMN
	int Variation_10();
	// @cmember pTableID is a null string.  Verify DB_E_NOTABLE
	int Variation_11();
	// @cmember pTableID is an empty string.  Verify DB_E_NOTABLE
	int Variation_12();
	// @cmember pTableID has invalid DBKIND.  Verify DB_E_NOTABLE
	int Variation_13();
	// @cmember pColumnID is a null string.  Verify DB_E_NOCOLUMN
	int Variation_14();
	// @cmember pColumnID is an empty string.  Verify DB_E_NOCOLUMN.
	int Variation_15();
	// @cmember pColumnID has invalid DBKIND.  Verify DB_E_NOCOLUMN.
	int Variation_16();
	// @cmember ColumnDescFlags was not supported.  Verify DB_E_NOTSUPPORTED
	int Variation_17();
	// @cmember Invalid ColumnDescFlags. Verify DB_E_NOTSUPPORTED.
	int Variation_18();
	// @cmember pColumnDesc contains an existing column id.  Verify DB_E_DUPLICATECOLUMNID.
	int Variation_19();
	// @cmember pColumnDesc contains invalid DBKIND.  Verify DB_E_BADCOLUMNID.
	int Variation_20();
	// @cmember pColumnDesc contains empty string.  Verify DB_E_BADCOLUMNID.
	int Variation_21();
	// @cmember pColumnDesc contains NULL string.  Verify DB_E_BADCOLUMNID
	int Variation_22();
	// @cmember pColumnDesc contains an invalid precision, overflow and underflow.  Verify DB_E_BADPRECISION
	int Variation_23();
	// @cmember pColumnDesc contains an invalid scale, both overflow and underflow.  Verify DB_E_BADSCALE.
	int Variation_24();
	// @cmember pColumnDesc contains an invalid type name.  Verify DB_E_BADTYPE
	int Variation_25();
	// @cmember pColumnDesc contains an invalid DBTYPE
	int Variation_26();
	// @cmember In pColumnDesc, cPropertySets > 0 and rgPropertySets is null.  Verify E_INVALIDARG.
	int Variation_27();
	// @cmember DBCOLUMNDESCFLAG_DBCID.  Change ID of columns
	int Variation_28();
	// @cmember DBCOLUMNDESCFLAGS_COLSIZE.  Change column size of columns
	int Variation_29();
	// @cmember DBCOLUMNDESCFLAGS_PRECISION.  Change precision of columns.
	int Variation_30();
	// @cmember DBCOLUMNDESCFLAGS_SCALE.  Change scale of columns
	int Variation_31();
	// @cmember DBCOLUMNDESCFLAG_CLSID.  Change clsid of columns.
	int Variation_32();
	// @cmember DBCOLUMNDESCFLAGS_WTYPE.  Change dbtype of columns.
	int Variation_33();
	// @cmember DBCOLUMNDESC_TYPENAME.  Change typename of columns
	int Variation_34();
	// @cmember DBCOLUMNDESC_ITYPEINFO.  Change type info of columns.
	int Variation_35();
	// @cmember DBPROP_COL_FIXEDLENGTH
	int Variation_36();
	// @cmember DBPROP_COL_ISLONG
	int Variation_37();
	// @cmember DBPROP_COL_NULLABLE
	int Variation_38();
	// @cmember DBPROP_COL_DESCRIPTION
	int Variation_39();
	// @cmember DBPROP_COL_UNIQUE
	int Variation_40();
	// @cmember DBPROP_COLUMNLCID
	int Variation_41();
	// @cmember DBPROP_COL_PRIMARYKEY
	int Variation_42();
	// @cmember DBPROP_COL_DEFAULT
	int Variation_43();
	// @cmember DBPROP_COL_AUTOINCREMENT
	int Variation_44();
	// @cmember Multiple attribute change
	int Variation_45();
	// @cmember Test atomicity.  Valid new ID but one other invalid attribute
	int Variation_46();
	// @cmember Test atomicity. Drop UNIQUE constraint and change to invalid type
	int Variation_47();
	// @cmember In an element of rgPropertySets, cProperties was > 0 but rgProperties was not null
	int Variation_48();
	// @cmember Drop multiple constraints
	int Variation_49();
	// @cmember Add multiple constraints
	int Variation_50();
	// @cmember Test atomicity.  Alter column id along with unsupported ColumnDescFlag
	int Variation_51();
	// @cmember Request BLOB using DBPROP_COL_ISLONG,FIXEDLENGTH
	int Variation_52();
	// @cmember Request DBPROP_COL_NULLABLE on non-nullable type. Verify error
	int Variation_53();
	// @cmember Request PRIMARYKEY and UNIQUE constraints - verify error
	int Variation_54();
	// @cmember Request AutoInc on non AUTO_UNIQUE_VALUE. Verify error
	int Variation_55();
	// @cmember Request ISLONG with FIXEDLENGTH true.  Verify Error
	int Variation_56();
	// @cmember Threads.  Test one thread altering a distinct col.  Verify each thread succeeds.
	int Variation_57();
	// @cmember ColumnDescFlags is 0. Verify no-op
	int Variation_58();
	// @cmember Change Column ID and use new Column ID in select stmt
	int Variation_59();
	// @cmember Change column id to invalid keyword. Verify Error
	int Variation_60();
	// @cmember Combination E_INVALIDARG conditiions
	int Variation_61();
	// @cmember WTYPE and TYPENAME mismatch.  Verify DB_E_BADTYPE
	int Variation_62();
	// @cmember SupportedTXNDDL
	int Variation_63();
	// @cmember Bad prop option
	int Variation_64();
	// @cmember Non column prop set  REQUIRED
	int Variation_65();
	// @cmember Non column prop set OPTIONAL
	int Variation_66();
	// @cmember Mismatch in property vt type. REQUIRED
	int Variation_67();
	// @cmember Mismatch in property vt type. OPTIONAL
	int Variation_68();
	// @cmember Invalid property value. REQUIRED
	int Variation_69();
	// @cmember Invalid property value. OPTIONAL
	int Variation_70();
	// @cmember colid was not DB_NULLID.  OPTIONAL
	int Variation_71();
	// @cmember colid was not DB_NULLID.  REQUIRED
	int Variation_72();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCAlterColumn)
#define THE_CLASS TCAlterColumn
BEG_TEST_CASE(TCAlterColumn, TCIAlterTable, L"Test Cases for AlterColumn method")
	TEST_VARIATION(1, 		L"Connect with DBPROP_INIT_MODE read only access.  Verify DB_SEC_E_PERMISSIONDENIED when altering a column")
	TEST_VARIATION(2, 		L"Try to use AlterColumn to change a table name. Verify failure")
	TEST_VARIATION(3, 		L"Call AlterColumn with multiple commands active which have open rowsets, though not on the table to be altered.")
	TEST_VARIATION(4, 		L"Use AlterColumn when a rowset is open on the table to be altered.  Verify DB_E_TABLEINUSE, or if successful, verify DB_S_COLUMNSCHANGE on restart position")
	TEST_VARIATION(5, 		L"Use AlterColumn when a FO/RO rowset is open on the table to be altered.  Verify DB_E_TABLEINUSE, or if successful, verify DB_S_COLUMNSCHANGE on restart position")
	TEST_VARIATION(6, 		L"pTableID is NULL.  Verify E_INVALIDARG.")
	TEST_VARIATION(7, 		L"pColumnID is NULL. Verify E_INVALIDARG")
	TEST_VARIATION(8, 		L"pColumnDesc is NULL.  Verify E_INVALIDARG.")
	TEST_VARIATION(9, 		L"pTableID does not exist.  Verify DB_E_NOTABLE")
	TEST_VARIATION(10, 		L"pColumnID does not exist.  Verify DB_E_NOCOLUMN")
	TEST_VARIATION(11, 		L"pTableID is a null string.  Verify DB_E_NOTABLE")
	TEST_VARIATION(12, 		L"pTableID is an empty string.  Verify DB_E_NOTABLE")
	TEST_VARIATION(13, 		L"pTableID has invalid DBKIND.  Verify DB_E_NOTABLE")
	TEST_VARIATION(14, 		L"pColumnID is a null string.  Verify DB_E_NOCOLUMN")
	TEST_VARIATION(15, 		L"pColumnID is an empty string.  Verify DB_E_NOCOLUMN.")
	TEST_VARIATION(16, 		L"pColumnID has invalid DBKIND.  Verify DB_E_NOCOLUMN.")
	TEST_VARIATION(17, 		L"ColumnDescFlags was not supported.  Verify DB_E_NOTSUPPORTED")
	TEST_VARIATION(18, 		L"Invalid ColumnDescFlags. Verify DB_E_NOTSUPPORTED.")
	TEST_VARIATION(19, 		L"pColumnDesc contains an existing column id.  Verify DB_E_DUPLICATECOLUMNID.")
	TEST_VARIATION(20, 		L"pColumnDesc contains invalid DBKIND.  Verify DB_E_BADCOLUMNID.")
	TEST_VARIATION(21, 		L"pColumnDesc contains empty string.  Verify DB_E_BADCOLUMNID.")
	TEST_VARIATION(22, 		L"pColumnDesc contains NULL string.  Verify DB_E_BADCOLUMNID")
	TEST_VARIATION(23, 		L"pColumnDesc contains an invalid precision, overflow and underflow.  Verify DB_E_BADPRECISION")
	TEST_VARIATION(24, 		L"pColumnDesc contains an invalid scale, both overflow and underflow.  Verify DB_E_BADSCALE.")
	TEST_VARIATION(25, 		L"pColumnDesc contains an invalid type name.  Verify DB_E_BADTYPE")
	TEST_VARIATION(26, 		L"pColumnDesc contains an invalid DBTYPE")
	TEST_VARIATION(27, 		L"In pColumnDesc, cPropertySets > 0 and rgPropertySets is null.  Verify E_INVALIDARG.")
	TEST_VARIATION(28, 		L"DBCOLUMNDESCFLAG_DBCID.  Change ID of columns")
	TEST_VARIATION(29, 		L"DBCOLUMNDESCFLAGS_COLSIZE.  Change column size of columns")
	TEST_VARIATION(30, 		L"DBCOLUMNDESCFLAGS_PRECISION.  Change precision of columns.")
	TEST_VARIATION(31, 		L"DBCOLUMNDESCFLAGS_SCALE.  Change scale of columns")
	TEST_VARIATION(32, 		L"DBCOLUMNDESCFLAG_CLSID.  Change clsid of columns.")
	TEST_VARIATION(33, 		L"DBCOLUMNDESCFLAGS_WTYPE.  Change dbtype of columns.")
	TEST_VARIATION(34, 		L"DBCOLUMNDESC_TYPENAME.  Change typename of columns")
	TEST_VARIATION(35, 		L"DBCOLUMNDESC_ITYPEINFO.  Change type info of columns.")
	TEST_VARIATION(36, 		L"DBPROP_COL_FIXEDLENGTH")
	TEST_VARIATION(37, 		L"DBPROP_COL_ISLONG")
	TEST_VARIATION(38, 		L"DBPROP_COL_NULLABLE")
	TEST_VARIATION(39, 		L"DBPROP_COL_DESCRIPTION")
	TEST_VARIATION(40, 		L"DBPROP_COL_UNIQUE")
	TEST_VARIATION(41, 		L"DBPROP_COLUMNLCID")
	TEST_VARIATION(42, 		L"DBPROP_COL_PRIMARYKEY")
	TEST_VARIATION(43, 		L"DBPROP_COL_DEFAULT")
	TEST_VARIATION(44, 		L"DBPROP_COL_AUTOINCREMENT")
	TEST_VARIATION(45, 		L"Multiple attribute change")
	TEST_VARIATION(46, 		L"Test atomicity.  Valid new ID but one other invalid attribute")
	TEST_VARIATION(47, 		L"Test atomicity. Drop UNIQUE constraint and change to invalid type")
	TEST_VARIATION(48, 		L"In an element of rgPropertySets, cProperties was > 0 but rgProperties was not null")
	TEST_VARIATION(49, 		L"Drop multiple constraints")
	TEST_VARIATION(50, 		L"Add multiple constraints")
	TEST_VARIATION(51, 		L"Test atomicity.  Alter column id along with unsupported ColumnDescFlag")
	TEST_VARIATION(52, 		L"Request BLOB using DBPROP_COL_ISLONG,FIXEDLENGTH")
	TEST_VARIATION(53, 		L"Request DBPROP_COL_NULLABLE on non-nullable type. Verify error")
	TEST_VARIATION(54, 		L"Request PRIMARYKEY and UNIQUE constraints - verify error")
	TEST_VARIATION(55, 		L"Request AutoInc on non AUTO_UNIQUE_VALUE. Verify error")
	TEST_VARIATION(56, 		L"Request ISLONG with FIXEDLENGTH true.  Verify Error")
	TEST_VARIATION(57, 		L"Threads.  Test one thread altering a distinct col.  Verify each thread succeeds.")
	TEST_VARIATION(58, 		L"ColumnDescFlags is 0. Verify no-op")
	TEST_VARIATION(59, 		L"Change Column ID and use new Column ID in select stmt")
	TEST_VARIATION(60, 		L"Change column id to invalid keyword. Verify Error")
	TEST_VARIATION(61, 		L"Combination E_INVALIDARG conditiions")
	TEST_VARIATION(62, 		L"WTYPE and TYPENAME mismatch.  Verify DB_E_BADTYPE")
	TEST_VARIATION(63, 		L"SupportedTXNDDL")
	TEST_VARIATION(64, 		L"Bad prop option")
	TEST_VARIATION(65, 		L"Non column prop set  REQUIRED")
	TEST_VARIATION(66, 		L"Non column prop set OPTIONAL")
	TEST_VARIATION(67, 		L"Mismatch in property vt type. REQUIRED")
	TEST_VARIATION(68, 		L"Mismatch in property vt type. OPTIONAL")
	TEST_VARIATION(69, 		L"Invalid property value. REQUIRED")
	TEST_VARIATION(70, 		L"Invalid property value. OPTIONAL")
	TEST_VARIATION(71, 		L"colid was not DB_NULLID.  OPTIONAL")
	TEST_VARIATION(72, 		L"colid was not DB_NULLID.  REQUIRED")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCZombie)
//*-----------------------------------------------------------------------
// @class Test the Zombie states of IAlterTable
//
class TCZombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember ABORT with fRetaining TRUE
	int Variation_1();
	// @cmember ABORT with fRetaining FALSE
	int Variation_2();
	// @cmember COMMIT with fRetaining TRUE
	int Variation_3();
	// @cmember COMMIT with fRetaining FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCZombie)
#define THE_CLASS TCZombie
BEG_TEST_CASE(TCZombie, CTransaction, L"Test the Zombie states of IAlterTable")
	TEST_VARIATION(1, 		L"ABORT with fRetaining TRUE")
	TEST_VARIATION(2, 		L"ABORT with fRetaining FALSE")
	TEST_VARIATION(3, 		L"COMMIT with fRetaining TRUE")
	TEST_VARIATION(4, 		L"COMMIT with fRetaining FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	return CommonModuleInit(pThisTestModule, IID_IAlterTable);
}	
  
//--------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
    return CommonModuleTerminate(pThisTestModule);
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(3, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCAlterTable)
	TEST_CASE(2, TCAlterColumn)
	TEST_CASE(3, TCZombie)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END

// {{ TCW_TC_PROTOTYPE(TCAlterTable)
//*-----------------------------------------------------------------------
//| Test Case:		TCAlterTable - TestCases for AlterTable method
//| Created:  	8/6/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAlterTable::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIAlterTable::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Connect with DBPROP_INIT_MODE requesting read only access.  Verify AlterTable return DB_SEC_E_PERMISSIONDENIED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_1()
{ 
	TBEGIN
	DBID			NewTableID;
	IUnknown *		pUnkReadSession = NULL;
	IAlterTable *	pIAlterTable = NULL;

	// Try to obtain a read only session
	if( !GetReadOnlySession(&pUnkReadSession) )
		return TEST_SKIPPED;

	TESTC(VerifyInterface(pUnkReadSession, IID_IAlterTable, SESSION_INTERFACE,
		(IUnknown**)&pIAlterTable));

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	
	TESTC_(pIAlterTable->AlterTable(&(m_pTable->GetTableID()), &NewTableID, 0 , NULL), DB_SEC_E_PERMISSIONDENIED);

CLEANUP:
	
	SAFE_RELEASE(pUnkReadSession);
	SAFE_RELEASE(pIAlterTable);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use AlterTable to change name of view
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_2()
{ 
	TBEGIN
	DBID	ViewID;
	DBID	NewTableID;
	HRESULT	hr;

	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	TESTC_(m_pTable->ExecuteCommand(CREATE_VIEW, IID_NULL), S_OK);

	ViewID.eKind = DBKIND_NAME;
	ViewID.uName.pwszName = wcsDuplicate(m_pTable->GetViewName());
	
	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

	TESTC(hr = VerifyAlterTable(&ViewID, &NewTableID));

	if( SUCCEEDED(hr) )
	{
		m_pTable->SetViewName(NewTableID.uName.pwszName);
	}

CLEANUP:

	TESTC_(m_pTable->ExecuteCommand(DROP_VIEW, IID_NULL), S_OK);

	ReleaseDBID(&ViewID, FALSE);
	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Use Alter Table to change name of non-table types. Try data type and column names
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_3()
{ 
	TBEGIN
	DBID	NewTableID;
	CCol	TempCol;

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TESTC_(m_pTable->GetColInfo(1, TempCol), S_OK);
	
	TESTC_(AlterTable(TempCol.GetColID(), &NewTableID), DB_E_NOTABLE);		

CLEANUP:

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Use AlterTable when command are active and there are open rowsets, though not on the table to be altered.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_4()
{ 
	TBEGIN
	CRowset			RowsetA;
	CRowset			RowsetB;
	IRowset *		pIRowsetA = NULL;
	IRowset *		pIRowsetB = NULL;
	DBID			NewTableID;
	
	//Create a couple rowsets using ICommand::Execute
	TESTC_(RowsetA.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, 0,
				NULL, (IUnknown**)&pIRowsetA), S_OK);
	TESTC_(RowsetB.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, 0,
				NULL, (IUnknown**)&pIRowsetB), S_OK);

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TESTC(VerifyAlterTable(&(m_pTable->GetTableID()), &NewTableID));

CLEANUP:

	SAFE_RELEASE(pIRowsetA);
	SAFE_RELEASE(pIRowsetB);

	RestoreTableID(&NewTableID);
	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Call AlterTable when an updatable rowset is open on the table to be altered
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_5()
{ 
	TBEGIN
	HRESULT			hr;
	CRowset			RowsetA;
	IRowset *		pIRowset = NULL;
	DBID			NewTableID;
	
	//Create an updatable rowset using ICommand::Execute
	RowsetA.SetTable(m_pTable, DELETETABLE_NO);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET);
	RowsetA.SetSettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET);

	TESTC_(RowsetA.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, 0,
				NULL, (IUnknown**)&pIRowset), S_OK);
	TESTC(DefaultObjectTesting(pIRowset, ROWSET_INTERFACE));	

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TEST2C_(hr = AlterTable(&(m_pTable->GetTableID()), &NewTableID), S_OK, DB_E_TABLEINUSE);
	if( S_OK == hr )
	{
		TESTC(CheckAlterTableID(&m_pTable->GetTableID(), &NewTableID));
	}

CLEANUP:

	SAFE_RELEASE(pIRowset);
	
	if( S_OK == hr )
	{
		RestoreTableID(&NewTableID);
	}

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Call AlterTable when a FO/RO rowset is open on the table to be altered
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_6()
{ 
	TBEGIN
	HRESULT			hr;
	CRowset			RowsetA;
	IRowset *		pIRowset = NULL;
	DBID			NewTableID;
	
	//Create a rowset using ICommand::Execute
	RowsetA.SetTable(m_pTable, DELETETABLE_NO);
	TESTC_(RowsetA.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, 0,
				NULL, (IUnknown**)&pIRowset), S_OK);

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TEST2C_(hr = AlterTable(&(m_pTable->GetTableID()), &NewTableID), S_OK, DB_E_TABLEINUSE);
	if( S_OK == hr )
	{
		TESTC(CheckAlterTableID(&m_pTable->GetTableID(), &NewTableID));
	}
CLEANUP:

	SAFE_RELEASE(pIRowset);
	
	if( S_OK == hr )
	{
		RestoreTableID(&NewTableID);
	}

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc pTableID is NULL.  Verify E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_7()
{ 
	return CHECK(AlterTable(NULL, NULL), E_INVALIDARG);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc cPropertySets = 1 and rgPropertySet is NULL.  Verify E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_8()
{ 
	TBEGIN
	DBID	NewTableID;

	TESTC_(DuplicateDBID(m_pTable->GetTableID(), &NewTableID), S_OK);
	TESTC_(AlterTable(&(m_pTable->GetTableID()), &NewTableID, 1, NULL), E_INVALIDARG);

CLEANUP:

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc In rgPropertySets, cProperties = 1 and rgProperties is NULL.  Verify E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_9()
{ 
	TBEGIN
	DBID		NewTableID;
	DBPROPSET	PropertySet;

	PropertySet.cProperties = 1;
	PropertySet.guidPropertySet = DBPROPSET_TABLE;
	PropertySet.rgProperties = NULL;

	TESTC_(DuplicateDBID(m_pTable->GetTableID(), &NewTableID), S_OK);
	TESTC_(AlterTable(&(m_pTable->GetTableID()), &NewTableID, 1, &PropertySet), E_INVALIDARG);

CLEANUP:

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc pTableID does not exist.  Verify DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_10()
{ 
	TBEGIN
	DBID	NonExistentTableID;
	DBID	NewTableID;

	TESTC_(MakeNewTableID(&NonExistentTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

	TESTC_(AlterTable(&NonExistentTableID, &NewTableID), DB_E_NOTABLE);

CLEANUP:

	ReleaseDBID(&NonExistentTableID, FALSE);
	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc pTableID is a null string.  Verify DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_11()
{ 
	TBEGIN
	DBID	NullID;
	DBID	NewTableID;

	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&NullID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

	SAFE_FREE(NullID.uName.pwszName); 
	
	TESTC_(AlterTable(&NullID, &NewTableID), DB_E_NOTABLE);

CLEANUP:

	ReleaseDBID(&NullID, FALSE);
	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc pTableID is an empty string.  Verify DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_12()
{ 
	TBEGIN
	DBID	EmptyID;
	DBID	NewTableID;

	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&EmptyID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

	if( EmptyID.uName.pwszName )
		EmptyID.uName.pwszName[0] = L'\0';
	
	TESTC_(AlterTable(&EmptyID, &NewTableID), DB_E_NOTABLE);

CLEANUP:

	ReleaseDBID(&EmptyID, FALSE);
	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc pTableID has an invalid DBKIND.  Verify DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_13()
{ 
	TBEGIN
	LONG		eKind;
	DBID		TableID;
	DBID		NewTableID;

	TableID = m_pTable->GetTableID();
	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

	for( eKind = DBKIND_GUID_NAME; eKind <= DBKIND_GUID+1; eKind++ )
	{
		if( eKind == m_lDBIDType )
			continue;

		TableID.eKind =  eKind;
		TESTC_(AlterTable(&TableID, &NewTableID), DB_E_NOTABLE);
	}

CLEANUP:

	NewTableID.eKind = m_lDBIDType;
	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc pNewTableID already exists.   Verify DB_E_DUPLICATETABLEID.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_14()
{ 
	CHECK(AlterTable(&m_pTable->GetTableID(), &g_pTable->GetTableID()), DB_E_DUPLICATETABLEID);
	CHECK(AlterTable(&m_pTable->GetTableID(), &g_pEmptyTable->GetTableID()), DB_E_DUPLICATETABLEID);
	CHECK(AlterTable(&m_pTable->GetTableID(), &g_p1RowTable->GetTableID()), DB_E_DUPLICATETABLEID);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc pNewTableID is a null string.  Verify DB_E_BADTABLEID.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_15()
{ 
	TBEGIN
	DBID		NewTableID;

	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	SAFE_FREE(NewTableID.uName.pwszName);

	TESTC_(AlterTable(&m_pTable->GetTableID(), &NewTableID), DB_E_BADTABLEID);

CLEANUP:

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc pNewTableID is an empty string.  Verify DB_E_BADTABLEID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_16()
{ 
	TBEGIN
	DBID		NewTableID;

	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

	if( NewTableID.uName.pwszName )
		NewTableID.uName.pwszName[0] = L'\0';

	TESTC_(AlterTable(&m_pTable->GetTableID(), &NewTableID), DB_E_BADTABLEID);

CLEANUP:

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc pNewTableID contains invalid characters for a table id.  Verify DB_E_BADTABLEID.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_17()
{ 
	TBEGIN
	ULONG	cIter = 0;
	DBID	NewTableID;
	HRESULT	hr;
	WCHAR *	pwszNewTableName = NULL;

	if( m_lDBIDType != DBKIND_NAME || !m_pwszInvalidTableChars )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

	pwszNewTableName = wcsDuplicate(NewTableID.uName.pwszName);
	TESTC(pwszNewTableName != NULL);
	SAFE_FREE(NewTableID.uName.pwszName);

	for( cIter = 0; cIter < wcslen(m_pwszInvalidTableChars); cIter++ )
	{
		NewTableID.uName.pwszName = wcsDuplicate(pwszNewTableName);
		NewTableID.uName.pwszName[ cIter % wcslen(pwszNewTableName) ] = m_pwszInvalidTableChars[cIter];

		hr = AlterTable(&m_pTable->GetTableID(), &NewTableID);
		if( SUCCEEDED(hr) )
		{
			RestoreTableID(&NewTableID);
		}
		TESTC_(hr, DB_E_BADTABLEID);

		SAFE_FREE(NewTableID.uName.pwszName);
	}

CLEANUP:

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc valid pTableID.  pNewTableID is NULL, cPropertySets =0, rgPropertySets is NULL.  Verify S_OK and no-op.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_18()
{ 
	return VerifyAlterTable(&(m_pTable->GetTableID()), NULL);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc valid pTableID, pNewTableID = pTableID, cPropertySets=0, rgPropertySets=NULL.  Verify S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_19()
{ 
	TBEGIN
	DBID	NewTableID;

	TESTC_(DuplicateDBID(m_pTable->GetTableID(), &NewTableID), S_OK);
	TESTC(VerifyAlterTable(&(m_pTable->GetTableID()), &NewTableID));

CLEANUP:

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Use fully qualified pTableID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_20()
{ 
	TBEGIN
	BOOL	fTableFound = FALSE;
	WCHAR *	pwszTableName = NULL;
	WCHAR * pwszQualifiedTableName = NULL;
	WCHAR *	pwszCatalogName = NULL;
	WCHAR *	pwszSchemaName = NULL;
	DBID	QualifiedTableID;
	DBID	NewTableID;
	
	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TESTC_(DuplicateDBID(m_pTable->GetTableID(), &QualifiedTableID), S_OK);
	pwszTableName = QualifiedTableID.uName.pwszName;

	// Tables schema is not necessarily supported
	if( !GetTableSchemaInfo(m_pTable, pwszTableName ,&fTableFound, &pwszCatalogName, &pwszSchemaName) )
		return TEST_SKIPPED;

	TESTC( fTableFound == TRUE );

	//Construct a fully Qualified TableName...
	TESTC_(m_pTable->GetQualifiedName(pwszCatalogName, pwszSchemaName,
		pwszTableName, &pwszQualifiedTableName), S_OK);
	SAFE_FREE(QualifiedTableID.uName.pwszName);
	QualifiedTableID.uName.pwszName = pwszQualifiedTableName;
	
	TESTC(VerifyAlterTable(&QualifiedTableID, &NewTableID));

CLEANUP:

	SAFE_FREE(pwszCatalogName);
	SAFE_FREE(pwszSchemaName);

	ReleaseDBID(&NewTableID, FALSE);
	ReleaseDBID(&QualifiedTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Use quoted pTableID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_21()
{ 
	TBEGIN
	DBID	NewTableID;
	WCHAR *	pwszQuotedName = NULL;

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

	if( NewTableID.eKind == DBKIND_NAME )
	{

		pwszQuotedName = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR *)*(wcslen(NewTableID.uName.pwszName)+1));
		TESTC(pwszQuotedName != NULL);

		swprintf(pwszQuotedName, L"\"%s\"", NewTableID.uName.pwszName);

		SAFE_FREE(NewTableID.uName.pwszName);
		NewTableID.uName.pwszName = pwszQuotedName;
	}

	TESTC(VerifyAlterTable(&(m_pTable->GetTableID()), &NewTableID));

CLEANUP:

	RestoreTableID(&NewTableID);
	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc valid pTableID, valid pNewTableID, cPropertySets=0, rgPropertySets=NULL.  Verify pTableID no longer exists and pNewTableID exists.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_22()
{ 
	TBEGIN
	DBID	TableID;
	DBID 	NewTableID;
	size_t	rgTableLengths[] = 
							{
								1,						// one char table name
								2,						// two char table name
								m_cchMaxTableName/2,	// medium length
								m_cchMaxTableName - 1,	// max length - 1
								m_cchMaxTableName		// max length
							};
	HRESULT	hr;
	BOOL	fExists = FALSE;
	ULONG	cIter = 0;

	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	if( m_cchMaxTableName == ~0 )
	{
		// Unknown maximum length
		size_t	cchLen = wcslen(m_pTable->GetTableName());

		rgTableLengths[2] = cchLen/2;
		rgTableLengths[3] = cchLen-1;
		rgTableLengths[4] = cchLen;		
	}

	TESTC_(DuplicateDBID(m_pTable->GetTableID(), &TableID), S_OK);

	// Try altering the table name to several interesting lengths.
	for( cIter = 0; cIter < NUMELEM(rgTableLengths); cIter++ )
	{
		if( rgTableLengths[cIter] > m_cchMaxTableName )
			continue;
		
		hr = MakeNewTableID(&NewTableID, DBKIND_NAME, rgTableLengths[cIter]);
		if( S_FALSE == hr )
			continue;

		TESTC_(hr, S_OK);
		
		TEST(VerifyAlterTable(&TableID, &NewTableID));
		
		SAFE_FREE(TableID.uName.pwszName);
		TableID.uName.pwszName = wcsDuplicate(NewTableID.uName.pwszName);

		ReleaseDBID(&NewTableID, FALSE);
	}

CLEANUP:

	RestoreTableID(&TableID);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Create temp table.  Alter Table to optionally unset property
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_23()
{ 
	TBEGIN
	HRESULT		hr;
	CTable *	pTable = NULL;

	TEST2C_(hr = CreateTable(&pTable, DBPROP_TBL_TEMPTABLE, (void *)VARIANT_TRUE,
		DBPROPOPTIONS_REQUIRED), S_OK, S_FALSE);

	if( hr == S_FALSE )
		return TEST_SKIPPED;

	TESTC(VerifyAlterTableProperties(&pTable->GetTableID(), DBPROP_TBL_TEMPTABLE, 
		DBPROPSET_TABLE, (void *)VARIANT_FALSE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL));

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Create temp table.  Alter table to unset that property using REQUIRED option
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_24()
{ 
	TBEGIN
	HRESULT		hr;
	CTable *	pTable = NULL;

	TEST2C_(hr = CreateTable(&pTable, DBPROP_TBL_TEMPTABLE, (void *)VARIANT_TRUE,
		DBPROPOPTIONS_REQUIRED), S_OK, S_FALSE);

	if( hr == S_FALSE )
		return TEST_SKIPPED;

	TESTC(VerifyAlterTableProperties(&pTable->GetTableID(), DBPROP_TBL_TEMPTABLE, 
		DBPROPSET_TABLE, (void *)VARIANT_FALSE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED));

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Create normal table.  Alter table to set TEMPTABLE property using REQUIRED option.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_25()
{ 
	TBEGIN
	CTable *	pTable = NULL;

	TESTC_(CreateTable(&pTable), S_OK);

	TESTC(VerifyAlterTableProperties(&pTable->GetTableID(), DBPROP_TBL_TEMPTABLE, 
		DBPROPSET_TABLE, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED));

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Create normal table.  Alter table to set TEMPTABLE property using OPTIONAL option.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_26()
{ 
	TBEGIN
	CTable *	pTable = NULL;

	TESTC_(CreateTable(&pTable), S_OK);

	TESTC(VerifyAlterTableProperties(&pTable->GetTableID(), DBPROP_TBL_TEMPTABLE, 
		DBPROPSET_TABLE, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL));

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Request a non TABLE property group with OPTIONAL flag.  Verify DB_S_ERRORSOCCURRED.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_27()
{ 
	return VerifyNonTableProps(DBPROPOPTIONS_OPTIONAL);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Request a non TABLE property group with REQUIRED flag.  Verify DB_E_ERRORSOCCURRED.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_28()
{ 
	return VerifyNonTableProps(DBPROPOPTIONS_REQUIRED);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Request a property OPTIONAL where the type in vValue of the DBPROP structure did not match the type of the property.  Verify DB_S_ERRORSOCCURRED.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_29()
{ 
	return VerifyAlterInvalidTableProperties(&m_pTable->GetTableID(), DBPROPSTATUS_NOTSET, 
		DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, (void *)1, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Request a property REQUIRED where the type in vValue of the DBPROP structure did not match the type of the property.  Verify DB_E_ERRORSOCCURRED.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_30()
{ 
	return VerifyAlterInvalidTableProperties(&m_pTable->GetTableID(), DBPROPSTATUS_BADVALUE, 
		DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, (void *)L"Foo", DBTYPE_BSTR, DBPROPOPTIONS_REQUIRED);	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Request DBPROP_TBL_TEMPTABLE with vValue type = VT_EMPTY.  Verify success.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_31()
{ 
	TBEGIN

	TESTC(VerifyAlterTableProperties(&m_pTable->GetTableID(), DBPROP_TBL_TEMPTABLE, 
		DBPROPSET_TABLE, NULL, DBTYPE_EMPTY, DBPROPOPTIONS_REQUIRED));

	TESTC(VerifyAlterTableProperties(&m_pTable->GetTableID(), DBPROP_TBL_TEMPTABLE, 
		DBPROPSET_TABLE, NULL, DBTYPE_EMPTY, DBPROPOPTIONS_OPTIONAL));

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc cPropertySets = 0, rgPropertySets not null.  Verify rgPropertySets is ignored.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_32()
{ 
	TBEGIN
	TESTC_(AlterTable(&m_pTable->GetTableID(), NULL, 0, INVALID(DBPROPSET *)), S_OK);

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc pNewTableID contains invalid starting characters for a tableid.  Verify DB_E_BADTABLEID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_33()
{ 
	TBEGIN
	HRESULT hr;
	ULONG	cIter = 0;
	DBID	NewTableID;
	WCHAR *	pwszNewTableName = NULL;

	if( m_lDBIDType != DBKIND_NAME || !m_pwszInvalidStartingTableChars )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

	pwszNewTableName = wcsDuplicate(NewTableID.uName.pwszName);
	TESTC(pwszNewTableName != NULL);
	SAFE_FREE(NewTableID.uName.pwszName);

	for( cIter = 0; cIter < wcslen(m_pwszInvalidStartingTableChars); cIter++ )
	{
		NewTableID.uName.pwszName = wcsDuplicate(pwszNewTableName);
		NewTableID.uName.pwszName[0] = m_pwszInvalidStartingTableChars[cIter];

		hr = AlterTable(&m_pTable->GetTableID(), &NewTableID);
		if( SUCCEEDED(hr) )
		{
			RestoreTableID(&NewTableID);
		}
		TESTC_(hr, DB_E_BADTABLEID);

		SAFE_FREE(NewTableID.uName.pwszName);
	}

CLEANUP:

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc pNewTableID exceeds maximum table name length.  Verify DB_E_BADTABLEID.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_34()
{ 
	TBEGIN
	DBID	NewTableID;

	if( m_lDBIDType != DBKIND_NAME || !m_pwszInvalidStartingTableChars )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, m_cchMaxColName+1), S_OK);
	TESTC_(AlterTable(&m_pTable->GetTableID(), &NewTableID), DB_E_BADTABLEID);

CLEANUP:

	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Request bad property option.  Verify DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_35()
{ 
	return VerifyAlterInvalidTableProperties(&m_pTable->GetTableID(), DBPROPSTATUS_BADOPTION, 
		DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL+1);	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Request, optional, an invalid value of TEMPTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_36()
{ 
	return VerifyAlterInvalidTableProperties(&m_pTable->GetTableID(), DBPROPSTATUS_NOTSET, 
		DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, (void *)1, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Request, required, an invalid value of TEMPTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_37()
{ 
	return VerifyAlterInvalidTableProperties(&m_pTable->GetTableID(), DBPROPSTATUS_BADVALUE, 
		DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, (void *)(USHRT_MAX-1), DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Optional prop, colid was not DB_NULLID.  Verify DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_38()
{ 
	return VerifyAlterInvalidTableProperties(&m_pTable->GetTableID(), DBPROPSTATUS_NOTSET, 
		DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED, m_pTable->GetTableID());	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Required prop, colid was not DB_NULLID.  Verify DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_39()
{ 
	return VerifyAlterInvalidTableProperties(&m_pTable->GetTableID(), DBPROPSTATUS_BADCOLUMN, 
		DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED, m_pTable->GetTableID());	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Use AlterTable to change table name.  Verify using TABLES schema rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_40()
{ 
	TBEGIN
	BOOL	fTableFound;
	DBID	TableID;
	DBID	NewTableID;

	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	TableID = m_pTable->GetTableID();

	// Verify that TABLES SCHEMA is supported
	if( !GetTableSchemaInfo(m_pTable, TableID.uName.pwszName, &fTableFound) )
		return TEST_SKIPPED;

	TESTC(fTableFound == TRUE);

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	
	TESTC_(AlterTable(&(m_pTable->GetTableID()), &NewTableID), S_OK);

	// Verify new table exists
	TESTC(GetTableSchemaInfo(m_pTable, NewTableID.uName.pwszName ,&fTableFound));
	TESTC(fTableFound == TRUE);

	// Verify previous table no longer exists
	TESTC(GetTableSchemaInfo(m_pTable, TableID.uName.pwszName ,&fTableFound));
	TESTC(fTableFound == FALSE);
	
CLEANUP:

	RestoreTableID(&NewTableID);
	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Mult Threads changing table id.  Verify that only one thread succeeds.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterTable::Variation_43()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	

	ULONG		cIter = 0;
	BOOL		fExists = FALSE;
	BOOL		fAlteredTableFound = FALSE;
	HRESULT		rghr[MAX_THREADS];
	THREADARG	rgTArg[MAX_THREADS];

	memset(rghr, 0, sizeof(rghr));

	//Setup Thread Arguments and Create Threads
	for( cIter = 0; cIter < MAX_THREADS; cIter++ )
	{
		DBID	NewTableID;

		TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);

		rgTArg[cIter].pFunc = (void *)this;
		rgTArg[cIter].pArg1 = (void *)&rghr[cIter];
		rgTArg[cIter].pArg2 = (void *)&m_pTable->GetTableIDRef();
		rgTArg[cIter].pArg3 = (void *)&NewTableID;
			
		CREATE_THREAD(cIter, Thread_AlterTable, &rgTArg[cIter]);
	}

	START_THREADS();
	END_THREADS();	

	// 
	for( cIter = 0; cIter < MAX_THREADS; cIter++ )
	{
		DBID *	pNewTableID = (DBID *)rgTArg[cIter].pArg3;

		if( rghr[cIter] == S_OK )
		{
			if( fAlteredTableFound )
			{
				TERROR("Error. Two threads succeeded in altering the table.");
			}

			fAlteredTableFound = TRUE;

			TESTC_(m_pTable->DoesTableExist(pNewTableID, &fExists), S_OK);
			TESTC(fExists == TRUE);

			RestoreTableID(pNewTableID);
		}
		else
		{
			// Some other thread must have changed the table name
			CHECK(rghr[cIter], DB_E_NOTABLE);

			TESTC_(m_pTable->DoesTableExist(pNewTableID, &fExists), S_OK);
			TESTC(fExists == FALSE);
		}

		ReleaseDBID(pNewTableID, FALSE);
	}

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCAlterTable::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIAlterTable::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCAlterColumn)
//*-----------------------------------------------------------------------
//| Test Case:		TCAlterColumn - Test Cases for AlterColumn method
//| Created:  	8/6/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAlterColumn::Init()
{ 
	TBEGIN
	IColumnsInfo *	pIColumnsInfo = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIAlterTable::Init())
	// }}
	{ 
		// Cache column metadata using CSchema object
		m_pSchema = new CTable(g_pIOpenRowset);

		// Now get the rowset which we'll use to populate the table
		TESTC_(m_pTable->CreateRowset(USE_OPENROWSET, IID_IColumnsInfo, 0, 
			NULL, (IUnknown **)&pIColumnsInfo),S_OK);
		TESTC_(m_pSchema->GetTableColumnInfo(&m_pTable->GetTableID(), pIColumnsInfo), S_OK);
		TESTC_(m_pSchema->AddInfoFromColumnsSchemaRowset(g_pIOpenRowset, m_pTable->GetTableName()), S_OK);

		TESTC_(m_pSchema->GetColInfo(1, m_Col), S_OK);
		TESTC_(DuplicateDBID(*m_Col.GetColID(), &m_ColumnID), S_OK);
	} 

CLEANUP:

	SAFE_RELEASE(pIColumnsInfo);

	TRETURN
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Connect with DBPROP_INIT_MODE read only access.  Verify DB_SEC_E_PERMISSIONDENIED when altering a column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_1()
{ 
	TBEGIN
	IUnknown *			pUnkReadSession = NULL;
	IAlterTable *		pIAlterTable = NULL;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_DBCID;

	// Try to obtain a read only session
	if( !GetReadOnlySession(&pUnkReadSession) )
		return TEST_SKIPPED;

	TESTC(VerifyInterface(pUnkReadSession, IID_IAlterTable, SESSION_INTERFACE,
		(IUnknown**)&pIAlterTable));
	
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);
	TESTC_(pIAlterTable->AlterColumn(&(m_pTable->GetTableID()), m_Col.GetColID(), dwColumnDescFlags,
		&ColumnDesc), DB_SEC_E_PERMISSIONDENIED);

CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	
	SAFE_RELEASE(pUnkReadSession);
	SAFE_RELEASE(pIAlterTable);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Try to use AlterColumn to change a table name. Verify failure
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_2()
{ 
	TBEGIN;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_DBCID;

	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);
	TESTC_(m_pIAlterTable->AlterColumn(&m_pTable->GetTableID(), &m_pTable->GetTableID(), dwColumnDescFlags,
		&ColumnDesc), DB_E_NOCOLUMN);

CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Call AlterColumn with multiple commands active which have open rowsets, though not on the table to be altered.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_3()
{ 
	TBEGIN
	CRowset			RowsetA;
	CRowset			RowsetB;
	CRowset			RowsetC;
	IRowset *		pIRowsetA = NULL;
	IRowset *		pIRowsetB = NULL;
	IRowset *		pIRowsetC = NULL;
	DBID 			NewColumnID;
	
	//Create a couple rowsets using ICommand::Execute
	TESTC_(RowsetA.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, 0,
				NULL, (IUnknown**)&pIRowsetA), S_OK);
	TESTC_(RowsetB.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, 0,
				NULL, (IUnknown**)&pIRowsetB), S_OK);
	TESTC_(RowsetC.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, 0,
				NULL, (IUnknown**)&pIRowsetC), S_OK);
	
	TESTC_(MakeNewColumnID(&NewColumnID, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);
			
	TESTC(VerifyAlterColumnName(1, &NewColumnID));

CLEANUP:

	SAFE_RELEASE(pIRowsetA);
	SAFE_RELEASE(pIRowsetB);
	SAFE_RELEASE(pIRowsetC);
	
	ReleaseDBID(&NewColumnID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Use AlterColumn when a rowset is open on the table to be altered.  Verify DB_E_TABLEINUSE, or if successful, verify DB_S_COLUMNSCHANGE on restart position
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_4()
{ 
	TBEGIN
	HRESULT			hr;
	CRowset			RowsetA;
	IRowset *		pIRowset = NULL;
	DBCOLUMNDESC	ColumnDesc;
	
	//Create an updatable rowset using ICommand::Execute
	RowsetA.SetTable(m_pTable, DELETETABLE_NO);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET);
	RowsetA.SetSettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET);

	TESTC_(RowsetA.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, 0,
				NULL, (IUnknown**)&pIRowset), S_OK);
	TESTC(DefaultObjectTesting(pIRowset, ROWSET_INTERFACE));	
	
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);			
	TEST2C_(hr = AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(),
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), S_OK, DB_E_TABLEINUSE);
	
	if( S_OK == hr )
	{
		TESTC(CompareColumnMetaData(1, DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc));
	}

CLEANUP:

	SAFE_RELEASE(pIRowset);
	
	ReleaseDBID(&ColumnDesc.dbcid, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Use AlterColumn when a FO/RO rowset is open on the table to be altered.  Verify DB_E_TABLEINUSE, or if successful, verify DB_S_COLUMNSCHANGE on restart position
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_5()
{ 
	TBEGIN
	HRESULT			hr;
	CRowset			RowsetA;
	IRowset *		pIRowset = NULL;
	DBCOLUMNDESC	ColumnDesc;
	
	//Create a rowset using ICommand::Execute
	RowsetA.SetTable(m_pTable, DELETETABLE_NO);
	TESTC_(RowsetA.pTable()->CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, 0,
				NULL, (IUnknown**)&pIRowset), S_OK);

	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);			
	TEST2C_(hr = AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(),
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), S_OK, DB_E_TABLEINUSE);
	
	if( S_OK == hr )
	{
		TESTC(CompareColumnMetaData(1, DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc));
	}


CLEANUP:

	SAFE_RELEASE(pIRowset);	

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc pTableID is NULL.  Verify E_INVALIDARG.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_6()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;
	
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);
	TESTC_(AlterColumn(NULL, &m_ColumnID, DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), E_INVALIDARG);
	
CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc pColumnID is NULL. Verify E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_7()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;
	
	TESTC_(AlterColumn(&m_pTable->GetTableID(), NULL, 
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), E_INVALIDARG);
	
CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc pColumnDesc is NULL.  Verify E_INVALIDARG.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_8()
{ 
	TBEGIN
	
	TESTC_(AlterColumn(&m_pTable->GetTableID(), &m_ColumnID, 
		DBCOLUMNDESCFLAGS_DBCID, NULL), E_INVALIDARG);
	
CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc pTableID does not exist.  Verify DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_9()
{ 
	TBEGIN		
	DBID			NonExistentTableID;
	DBCOLUMNDESC	ColumnDesc;
	
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);	
	TESTC_(MakeNewTableID(&NonExistentTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	
	TESTC_(AlterColumn(&NonExistentTableID, &m_ColumnID, 
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_NOTABLE);
	
CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	ReleaseDBID(&NonExistentTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc pColumnID does not exist.  Verify DB_E_NOCOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_10()
{ 
	TBEGIN		
	DBCOLUMNDESC	ColumnDesc;
	DBID			NonExistentColumnID;
		
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);	
	TESTC_(MakeNewColumnID(&NonExistentColumnID, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);
	
	TESTC_(AlterColumn(&m_pTable->GetTableID(), &NonExistentColumnID, 
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_NOCOLUMN);
	
CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	ReleaseDBID(&NonExistentColumnID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc pTableID is a null string.  Verify DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_11()
{ 
	TBEGIN
	DBID			NullTableID;
	DBCOLUMNDESC	ColumnDesc;
	
	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&NullTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);

	SAFE_FREE(NullTableID.uName.pwszName); 
	
	TESTC_(AlterColumn(&NullTableID, &m_ColumnID, 
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_NOTABLE);

CLEANUP:

	ReleaseDBID(&NullTableID, FALSE);
	ReleaseDBID(&ColumnDesc.dbcid, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc pTableID is an empty string.  Verify DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_12()
{ 
	TBEGIN
	DBID			EmptyTableID;
	DBCOLUMNDESC	ColumnDesc;
	
	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	TESTC_(MakeNewTableID(&EmptyTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);

	EmptyTableID.uName.pwszName[0] = L'\0';
	
	TESTC_(AlterColumn(&EmptyTableID, &m_ColumnID, 
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_NOTABLE);

CLEANUP:

	ReleaseDBID(&EmptyTableID, FALSE);
	ReleaseDBID(&ColumnDesc.dbcid, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc pTableID has invalid DBKIND.  Verify DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_13()
{ 
	TBEGIN
	LONG			eKind;
	DBID			TableID;
	DBCOLUMNDESC	ColumnDesc;

	ColumnDesc.cPropertySets = 0;
	ColumnDesc.rgPropertySets = NULL;

	DuplicateDBID(m_pTable->GetTableID(), &TableID);

	for( eKind = DBKIND_GUID_NAME; eKind <= DBKIND_GUID+1; eKind++ )
	{
		if( eKind == m_lDBIDType )
			continue;

		TableID.eKind = eKind;
		TESTC_(AlterColumn(&TableID, &m_ColumnID, 
			DBCOLUMNDESCFLAGS_PROPERTIES, &ColumnDesc), DB_E_NOTABLE);
	}

CLEANUP:

	TableID.eKind = m_lDBIDType;
	ReleaseDBID(&TableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc pColumnID is a null string.  Verify DB_E_NOCOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_14()
{ 
	TBEGIN
	DBID			ColumnID;
	DBCOLUMNDESC	ColumnDesc;
	
	if( m_lDBIDType != DBKIND_NAME )	
		return TEST_SKIPPED;

	ColumnDesc.cPropertySets = 0;
	ColumnDesc.rgPropertySets = NULL;

	TESTC_(DuplicateDBID(m_ColumnID, &ColumnID), S_OK);
	SAFE_FREE(ColumnID.uName.pwszName);
	
	TESTC_(AlterColumn(&m_pTable->GetTableID(), &ColumnID, 
		DBCOLUMNDESCFLAGS_PROPERTIES, &ColumnDesc), DB_E_NOCOLUMN);

CLEANUP:

	ReleaseDBID(&ColumnID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc pColumnID is an empty string.  Verify DB_E_NOCOLUMN.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_15()
{ 
	TBEGIN
	DBID			ColumnID;
	DBCOLUMNDESC	ColumnDesc;
	
	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	ColumnDesc.cPropertySets = 0;
	ColumnDesc.rgPropertySets = NULL;

	TESTC_(DuplicateDBID(m_ColumnID, &ColumnID), S_OK);
	ColumnID.uName.pwszName[0] = L'\0';
	
	TESTC_(AlterColumn(&m_pTable->GetTableID(), &ColumnID, 
		DBCOLUMNDESCFLAGS_PROPERTIES, &ColumnDesc), DB_E_NOCOLUMN);

CLEANUP:

	ReleaseDBID(&ColumnID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc pColumnID has invalid DBKIND.  Verify DB_E_NOCOLUMN.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_16()
{ 
	TBEGIN
	LONG			eKind;
	DBID			ColumnID;
	DBCOLUMNDESC	ColumnDesc;

	ColumnDesc.cPropertySets = 0;
	ColumnDesc.rgPropertySets = NULL;

	DuplicateDBID(m_ColumnID, &ColumnID);

	for( eKind = DBKIND_GUID_NAME; eKind <= DBKIND_GUID+1; eKind++ )
	{
		if( eKind == m_lDBIDType )
			continue;

		if( (eKind == DBKIND_GUID_NAME ||
			eKind == DBKIND_PGUID_NAME ||
			eKind == DBKIND_NAME) &&
			(m_lDBIDType == DBKIND_GUID_NAME ||
			m_lDBIDType == DBKIND_PGUID_NAME ||
			m_lDBIDType == DBKIND_NAME) )
			continue;

		ColumnID.eKind = eKind;
		TESTC_(AlterColumn(&m_pTable->GetTableID(), &ColumnID, 
			DBCOLUMNDESCFLAGS_PROPERTIES, &ColumnDesc), DB_E_NOCOLUMN);
	}

CLEANUP:

	ColumnID.eKind = m_lDBIDType;
	ReleaseDBID(&ColumnID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ColumnDescFlags was not supported.  Verify DB_E_NOTSUPPORTED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_17()
{ 
	TBEGIN
	CCol				col;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwFlags = 0;
	
	TESTC_(m_pTable->GetColInfo(1, col), S_OK);
	TESTC_(m_pTable->BuildColumnDesc(&ColumnDesc, col), S_OK);

	for( dwFlags = DBCOLUMNDESCFLAGS_TYPENAME; dwFlags <= DBCOLUMNDESCFLAGS_SCALE; dwFlags <<= 1)
	{
		if( (m_lAlterColumnSupport & dwFlags) == 0 )
		{
			CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
				dwFlags, &ColumnDesc), DB_E_NOTSUPPORTED);
		}
	}

	CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
			~m_lAlterColumnSupport, &ColumnDesc), DB_E_NOTSUPPORTED);

CLEANUP:

	ReleaseColumnDesc(&ColumnDesc, 1, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Invalid ColumnDescFlags. Verify DB_E_NOTSUPPORTED.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_18()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;
	
	TESTC_(AlterColumn(&m_pTable->GetTableID(), 
		&m_ColumnID, DBCOLUMNDESCFLAGS_SCALE << 1, &ColumnDesc), DB_E_NOTSUPPORTED);

	TESTC_(AlterColumn(&m_pTable->GetTableID(), 
		&m_ColumnID, LONG_MAX, &ColumnDesc), DB_E_NOTSUPPORTED);

	TESTC_(AlterColumn(&m_pTable->GetTableID(), 
		&m_ColumnID, LONG_MIN, &ColumnDesc), DB_E_NOTSUPPORTED);
	
CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc pColumnDesc contains an existing column id.  Verify DB_E_DUPLICATECOLUMNID.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_19()
{ 
	TBEGIN
	DBID			SourceColumnID;
	DBCOLUMNDESC	ColumnDesc;
	ULONG			i,j;
	CCol			sourceCol;
	CCol			DestCol;

	memset(&ColumnDesc, 0, sizeof(DBID));

	for( i=1; i <= m_pTable->CountColumnsOnSchema(); i++ )
	{
		TESTC_(m_pTable->GetColInfo(i, sourceCol), S_OK);
		TESTC_(DuplicateDBID(*sourceCol.GetColID(), &SourceColumnID), S_OK);

		for( j=1; j <= m_pTable->CountColumnsOnSchema(); j++ )
		{
			if( i == j )
				continue;

			TESTC_(m_pTable->GetColInfo(j, DestCol), S_OK);
			TESTC_(DuplicateDBID(*DestCol.GetColID(), &ColumnDesc.dbcid), S_OK);

			TESTC_(AlterColumn(&m_pTable->GetTableID(), &SourceColumnID, 
				DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_DUPLICATECOLUMNID);

			ReleaseDBID(&ColumnDesc.dbcid, FALSE);
		}

		ReleaseDBID(&SourceColumnID, FALSE);
	}

CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	ReleaseDBID(&SourceColumnID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc pColumnDesc contains invalid DBKIND.  Verify DB_E_BADCOLUMNID.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_20()
{ 
	TBEGIN
	LONG			eKind;
	DBCOLUMNDESC	ColumnDesc;

	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);	

	for( eKind = DBKIND_GUID_NAME; eKind <= DBKIND_GUID+1; eKind++ )
	{
		if( eKind == m_lDBIDType )
			continue;

		ColumnDesc.dbcid.eKind = eKind;
		TESTC_(AlterColumn(&m_pTable->GetTableID(), &m_ColumnID, 
			DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_BADCOLUMNID);
	}

CLEANUP:

	ColumnDesc.dbcid.eKind = m_lDBIDType;
	ReleaseDBID(&ColumnDesc.dbcid, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc pColumnDesc contains empty string.  Verify DB_E_BADCOLUMNID.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_21()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;

	if( m_lDBIDType != DBKIND_NAME )	
		return TEST_SKIPPED;

	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);	
	ColumnDesc.dbcid.uName.pwszName[0] = L'\0';

	TESTC_(AlterColumn(&m_pTable->GetTableID(), &m_ColumnID, 
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_BADCOLUMNID);

CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc pColumnDesc contains NULL string.  Verify DB_E_BADCOLUMNID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_22()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;

	if( m_lDBIDType != DBKIND_NAME )	
		return TEST_SKIPPED;

	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);	
	SAFE_FREE(ColumnDesc.dbcid.uName.pwszName);

	TESTC_(AlterColumn(&m_pTable->GetTableID(), &m_ColumnID, 
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_BADCOLUMNID);

CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc pColumnDesc contains an invalid precision, overflow and underflow.  Verify DB_E_BADPRECISION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_23()
{ 
	TBEGIN
	HRESULT				hr;
 	ULONG				cIter = 0;
	ULONG				ulOrdinal = 0;
	CCol				col;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags =	DBCOLUMNDESCFLAGS_PRECISION;

	if( (m_lAlterColumnSupport & dwColumnDescFlags) == 0 )
		return TEST_SKIPPED;

	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);

		if( col.GetProviderType() != DBTYPE_NUMERIC && 
			col.GetProviderType() != DBTYPE_VARNUMERIC )
			continue;	

		BYTE	rgbPrecision[] = 
							{
								0,
								col.GetPrecision()+1,
							};

		for( cIter = 0; cIter < NUMELEM(rgbPrecision); cIter++ )
		{
			ColumnDesc.bPrecision = rgbPrecision[cIter];

			hr = AlterColumn(&m_pTable->GetTableID(), col.GetColID(), 
				dwColumnDescFlags, &ColumnDesc);
			CHECK(hr, DB_E_BADPRECISION);
		}
	}

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc pColumnDesc contains an invalid scale, both overflow and underflow.  Verify DB_E_BADSCALE.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_24()
{ 
	TBEGIN
	HRESULT				hr;
 	ULONG				cIter = 0;
	ULONG				ulOrdinal = 0;
	CCol				col;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags =	DBCOLUMNDESCFLAGS_SCALE;

	if( (m_lAlterColumnSupport & dwColumnDescFlags) == 0 )
		return TEST_SKIPPED;

	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);

		if( col.GetProviderType() != DBTYPE_NUMERIC && 
			col.GetProviderType() != DBTYPE_VARNUMERIC &&
			col.GetProviderType() != DBTYPE_DECIMAL )
			continue;	

		BYTE	rgbScale[] = 
							{
								col.GetMinScale()-1,
								col.GetPrecision()+1,
								col.GetMaxScale()+1,
							};

		for( cIter = 0; cIter < NUMELEM(rgbScale); cIter++ )
		{
			ColumnDesc.bScale = rgbScale[cIter];

			hr = AlterColumn(&m_pTable->GetTableID(), col.GetColID(), 
				dwColumnDescFlags, &ColumnDesc);
			CHECK(hr, DB_E_BADSCALE);
		}

		// one last case with Scale > Precision
		ColumnDesc.bPrecision = col.GetPrecision()/2;
		ColumnDesc.bScale = ColumnDesc.bPrecision+1;

		hr = AlterColumn(&m_pTable->GetTableID(), col.GetColID(),
			DBCOLUMNDESCFLAGS_PRECISION | DBCOLUMNDESCFLAGS_SCALE, &ColumnDesc);
		CHECK(hr, DB_E_BADPRECISION);
	}

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc pColumnDesc contains an invalid type name.  Verify DB_E_BADTYPE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_25()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_TYPENAME) == 0 )
		return TEST_SKIPPED;

	memset(&ColumnDesc, 0xCC, sizeof(DBCOLUMNDESC));

	ColumnDesc.pwszTypeName = NULL;
	CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
			DBCOLUMNDESCFLAGS_TYPENAME, &ColumnDesc), DB_E_BADTYPE);

	ColumnDesc.pwszTypeName = L"";
	CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
			DBCOLUMNDESCFLAGS_TYPENAME, &ColumnDesc), DB_E_BADTYPE);

	ColumnDesc.pwszTypeName = L"NonExistentTypeName";
	CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
			DBCOLUMNDESCFLAGS_TYPENAME, &ColumnDesc), DB_E_BADTYPE);

	if( GetModInfo()->GetLocaleInfo() )
	{
		const ULONG	cchTypeLen = 10;
		WCHAR		wszIntlString[cchTypeLen+1];

		GetModInfo()->GetLocaleInfo()->MakeUnicodeIntlString(wszIntlString, cchTypeLen);

		ColumnDesc.pwszTypeName = wszIntlString;
		CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
			DBCOLUMNDESCFLAGS_TYPENAME, &ColumnDesc), DB_E_BADTYPE);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc pColumnDesc contains an invalid DBTYPE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_26()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_WTYPE) == 0 )
		return TEST_SKIPPED;

	memset(&ColumnDesc, 0xFF, sizeof(DBCOLUMNDESC));

	// max type
	ColumnDesc.wType = DBTYPE_DBTIMESTAMP + 1;
	CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
			DBCOLUMNDESCFLAGS_WTYPE, &ColumnDesc), DB_E_BADTYPE);

	// in valid range, but invalid
	ColumnDesc.wType = DBTYPE_DECIMAL + 1;
	CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
			DBCOLUMNDESCFLAGS_WTYPE, &ColumnDesc), DB_E_BADTYPE);

	// max possible value
	ColumnDesc.wType = USHRT_MAX;
	CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
			DBCOLUMNDESCFLAGS_WTYPE, &ColumnDesc), DB_E_BADTYPE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc In pColumnDesc, cPropertySets > 0 and rgPropertySets is null.  Verify E_INVALIDARG.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_27()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;

	ColumnDesc.cPropertySets = 1;
	ColumnDesc.rgPropertySets = NULL;

	TESTC_(AlterColumn(&m_pTable->GetTableID(), &m_ColumnID, DBCOLUMNDESCFLAGS_PROPERTIES,
					&ColumnDesc), E_INVALIDARG);
CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc DBCOLUMNDESCFLAG_DBCID.  Change ID of columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_28()
{ 
	TBEGIN
	DBID 	NewColumnID;
	size_t	rgColumnLengths[] = 
							{
								1,						// one char table name
								2,						// two char table name
								m_cchMaxColName/2,	// medium length
								m_cchMaxColName - 1,	// max length - 1
								m_cchMaxColName		// max length
							};
	HRESULT	hr;
	ULONG	ulOrdinal = 0;
	ULONG	cIter = 0;

	if( m_lDBIDType != DBKIND_NAME )
		return TEST_SKIPPED;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_DBCID) == 0 )
		return TEST_SKIPPED;

	if( m_cchMaxColName == ~0 )
	{
		// Unknown maximum length
		size_t	cchLen = wcslen(m_pTable->GetTableName());

		rgColumnLengths[2] = cchLen/2;
		rgColumnLengths[3] = cchLen-1;
		rgColumnLengths[4] = cchLen;		
	}

	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{

		odtLog << "Altering column ID for column " << ulOrdinal << ENDL;

		// Try altering the column name to several interesting lengths.
		for( cIter = 0; cIter < NUMELEM(rgColumnLengths); cIter++ )
		{
			if( rgColumnLengths[cIter] > m_cchMaxColName )
				continue;
			
			hr = MakeNewColumnID(&NewColumnID, DBKIND_NAME, rgColumnLengths[cIter]);
			if( S_FALSE == hr )
				continue;

			TESTC_(hr, S_OK);
			
			TESTC(VerifyAlterColumnName(ulOrdinal, &NewColumnID));
		}
	}

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc DBCOLUMNDESCFLAGS_COLSIZE.  Change column size of columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_29()
{ 
	TBEGIN
	ULONG				cIter = 0;
	ULONG				ulOrdinal = 0;
	CCol				col;
	CCol				onetablecol;
	DBCOLUMNDESCFLAGS	ColumnDescFlags = DBCOLUMNDESCFLAGS_COLSIZE;
	CTable *			pTable = NULL;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_COLSIZE) == 0 )
		return TEST_SKIPPED;

	// For each value in the schema rowset
	for( ulOrdinal = 1; ulOrdinal <= m_pFullSchemaInfo->CountColumnsOnSchema(); ulOrdinal++)
	{
		DBCOLUMNDESC *	pColumnDesc = NULL;

		TESTC_(m_pFullSchemaInfo->GetColInfo(ulOrdinal, col), S_OK);

		// DBCOLUMNDESCFLAGS_COLSIZE applies only to STR/WSTR/BYTES
		if( col.GetProviderType() != DBTYPE_STR && 
			col.GetProviderType() != DBTYPE_WSTR &&
			col.GetProviderType() != DBTYPE_BYTES )
			continue;	

		if( col.GetCreateParams() == NULL)
			continue;

		pColumnDesc = (DBCOLUMNDESC *)PROVIDER_ALLOC(sizeof(DBCOLUMNDESC));
		TESTC(pColumnDesc != NULL);
		TESTC_(m_pTable->BuildColumnDesc(pColumnDesc, col), S_OK);

		TESTC_(MakeNewColumnID(&pColumnDesc->dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);
		pColumnDesc->ulColumnSize = 1;

		// Need to build a one column table to try this out.
		// Important since Datasources often limit a row size
		// to a page size.
		// PrivLib table creation logic often tries to create
		// row sizes near its limit.
		// Trying to increase a column size on such a table
		// will probably violate its row limit size.
		// With a new one column table, we will be able to
		// alter the size up to its column size limits
		// as reported in the PROVIDER_TYPES schema rowset
		pTable = new CTable(g_pIOpenRowset, L"IALTTAB");
		TESTC(pTable != NULL);		

		pTable->SetBuildColumnDesc(FALSE);
		pTable->SetColumnDesc(pColumnDesc, 1);

		TESTC_(pTable->CreateTable(0,0,NULL,PRIMARY,TRUE), S_OK);

		TESTC_(pTable->GetColInfo(1, onetablecol), S_OK);
		DBLENGTH rgulColSize[] = 
							{
								col.GetColumnSize(),
								col.GetColumnSize()-1,
								col.GetColumnSize()/2,
								2,		
								1,																															
							};

		odtLog << "Altering column of type: " << onetablecol.GetProviderTypeName() << " to column size : ";
		for( cIter = 0; cIter < NUMELEM(rgulColSize); cIter++ )
		{
			odtLog << ENDL << rgulColSize[cIter] << L".";
			COMPARE(VerifyAlterColumnSize(1, rgulColSize[cIter], pTable), TRUE);
		}
		odtLog << ENDL;

		if( pTable )
		{
			pTable->DropTable();
			SAFE_DELETE(pTable);
		}
	}

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc DBCOLUMNDESCFLAGS_PRECISION.  Change precision of columns.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_30()
{ 
	TBEGIN
	ULONG	cIter = 0;
	ULONG	ulOrdinal = 0;
	CCol	col;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_PRECISION) == 0 )
		return TEST_SKIPPED;

	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);

		// DBCOLUMNDESCFLAGS_PRECISION only applies to NUMERIC and VARNUMERIC
		if( col.GetProviderType() != DBTYPE_NUMERIC && 
			col.GetProviderType() != DBTYPE_VARNUMERIC )
			continue;	

		BYTE	rgbPrecision[] = 
							{
								1,						
								2,		
								col.GetScale(),
								col.GetPrecision()/2,	
								col.GetPrecision() - 1,	
								col.GetPrecision()
							};

		for( cIter = 0; cIter < NUMELEM(rgbPrecision); cIter++ )
		{
			if( col.GetProviderType() == DBTYPE_NUMERIC && rgbPrecision[cIter] < col.GetScale() )
				continue;

			TESTC(VerifyAlterColumnPrecision(ulOrdinal, rgbPrecision[cIter]));
		}
	}

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc DBCOLUMNDESCFLAGS_SCALE.  Change scale of columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_31()
{ 
	TBEGIN
	ULONG	cIter = 0;
	ULONG	ulOrdinal = 0;
	CCol	col;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_SCALE) == 0 )
		return TEST_SKIPPED;

	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);

		// DBCOLUMNDESCFLAGS_SCALE only applies to NUMERIC,VARNUMERIC, and DECIMAL
		if( col.GetProviderType() != DBTYPE_NUMERIC && 
			col.GetProviderType() != DBTYPE_VARNUMERIC &&
			col.GetProviderType() != DBTYPE_DECIMAL )
			continue;	

		BYTE	rgbScale[] = 
							{
								0,						
								1,		
								col.GetScale(),
								col.GetPrecision()/2,
								col.GetPrecision() - 1,	
								col.GetPrecision()
							};

		for( cIter = 0; cIter < NUMELEM(rgbScale); cIter++ )
		{
			if( rgbScale[cIter] < col.GetMinScale() ||
				rgbScale[cIter] > col.GetMaxScale() )
				continue;

			TESTC(VerifyAlterColumnScale(ulOrdinal, rgbScale[cIter]));
		}
	}

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc DBCOLUMNDESCFLAG_CLSID.  Change clsid of columns.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_32()
{ 
	TBEGIN
	CLSID 			clsid;
	ULONG			ulOrdinal = 0;

	memcpy(&clsid, &IID_IUnknown, sizeof(CLSID));
	
	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_CLSID) == 0 )
	{
		DBCOLUMNDESC	ColumnDesc;

		ColumnDesc.pclsid = &clsid;

		TESTC_(AlterColumn(&m_pTable->GetTableID(), 
			&m_ColumnID, DBCOLUMNDESCFLAGS_CLSID, &ColumnDesc), DB_E_NOTSUPPORTED);
	}
	else
	{
		// For each column in the table, try altering the CLSID of the column			
		for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
		{		
			TESTC(VerifyAlterColumnCLSID(ulOrdinal, &clsid));		
		}
	}

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc DBCOLUMNDESCFLAGS_WTYPE.  Change dbtype of columns.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_33()
{ 
	TBEGIN
	ULONG			cTypes = 0;
	ULONG			ulOrdinal = 0;
	CCol			tablecol;
	CCol			typecol;
	CTable *		pTable = NULL;
	
	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_WTYPE) == 0 )
		return TEST_SKIPPED;

	TESTC_(CreateTable(&pTable), S_OK);
	
	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(pTable->GetColInfo(ulOrdinal, tablecol), S_OK);
		if( tablecol.GetUnique() || tablecol.GetAutoInc() || tablecol.GetIsLong() || !tablecol.GetUpdateable() )
				continue;

		if( UpgradeType(&tablecol, &typecol) )
		{
			odtLog << "Altering column: " << ulOrdinal << " of type: " << tablecol.GetProviderTypeName() << " to type: " << typecol.GetProviderTypeName() << "." << ENDL;
			TESTC_(SetChangeColInfo(&typecol), S_OK);
			
			COMPARE(VerifyAlterColumnType(ulOrdinal, typecol.GetProviderType(), pTable), TRUE);	
		}
		else
		{
			odtLog << "Couldn't find like type for column: " << ulOrdinal << " of type: " << tablecol.GetProviderTypeName() << ENDL;
		}
	}

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc DBCOLUMNDESC_TYPENAME.  Change typename of columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_34()
{ 
	BOOL			fPass = FALSE;
	ULONG			cTypes = 0;
	ULONG			ulOrdinal = 0;
	CCol			tablecol;
	CCol			typecol;
	CTable *		pTable = NULL;
	
	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_TYPENAME) == 0 )
		return TEST_SKIPPED;

	TESTC_(CreateTable(&pTable), S_OK);
	
	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(pTable->GetColInfo(ulOrdinal, tablecol), S_OK);
		if( tablecol.GetUnique() || tablecol.GetAutoInc() || tablecol.GetIsLong() || !tablecol.GetUpdateable() )
				continue;

		if( UpgradeType(&tablecol, &typecol) )
		{
			odtLog << "Altering column: " << ulOrdinal << " of type: " << tablecol.GetProviderTypeName() << " to type: " << typecol.GetProviderTypeName() << "." << ENDL;
			TESTC_(SetChangeColInfo(&typecol), S_OK);

			fPass = VerifyAlterColumnTypeName(ulOrdinal, typecol.GetProviderTypeName(), pTable);		
			COMPARE(fPass, TRUE);	
		}
		else
		{
			odtLog << "Couldn't find like type for column: " << ulOrdinal << " of type: " << tablecol.GetProviderTypeName() << ENDL;
		}
	}

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc DBCOLUMNDESC_ITYPEINFO.  Change type info of columns.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_35()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;
	
	ColumnDesc.pTypeInfo = INVALID(ITypeInfo *);
	
	// DBCOLUMNDESCFLAGS_ITYPEINFO is reserved for future use and should not be supported.
	TESTC( (DBCOLUMNDESCFLAGS_ITYPEINFO & m_lAlterColumnSupport) == 0 );
	
	TESTC_(AlterColumn(&m_pTable->GetTableID(), 
		&m_ColumnID, DBCOLUMNDESCFLAGS_ITYPEINFO, &ColumnDesc), DB_E_NOTSUPPORTED);

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COL_FIXEDLENGTH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_36()
{ 
	TBEGIN
	ULONG				cTypes = 0;
	ULONG				ulOrdinal = 0;
	CCol				tablecol;
	CCol				typecol;
	BOOL				fFixed;
	CTable *			pTable = NULL;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES | DBCOLUMNDESCFLAGS_WTYPE;
	DBCOLUMNDESC		ColumnDesc;
	
	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;
	
	TESTC_(CreateTable(&pTable), S_OK);

	memset(&ColumnDesc, 0, sizeof(DBCOLUMNDESC));
	
	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= pTable->CountColumnsOnSchema(); ulOrdinal++ )
	{
		TESTC_(pTable->GetColInfo(ulOrdinal, tablecol), S_OK);

		if( !tablecol.GetUpdateable() || tablecol.GetIsLong() )
				continue;

		if( tablecol.GetProviderType() == DBTYPE_WSTR  ||		
			tablecol.GetProviderType() == DBTYPE_STR  ||
			tablecol.GetProviderType() == DBTYPE_BYTES  )
		{			
			if( UpgradeType(&tablecol, &typecol) )
			{
				odtLog << "Altering column: " << ulOrdinal << " of type: " << tablecol.GetProviderTypeName() << " to type: " << typecol.GetProviderTypeName() << "." << ENDL;
				TESTC_(SetChangeColInfo(&typecol), S_OK);

				// Get the FixedLength attribute of the new column
				fFixed = typecol.GetIsFixedLength();

				TESTC_(SetChangeColInfo(&typecol), S_OK);
				
				ColumnDesc.wType = tablecol.GetProviderType();
				
				::SetProperty(DBPROP_COL_FIXEDLENGTH, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
					&ColumnDesc.rgPropertySets, (void *)(fFixed ? VARIANT_TRUE : VARIANT_FALSE),
					DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
				
				COMPARE(VerifyAlterColumn(ulOrdinal, dwColumnDescFlags, &ColumnDesc, pTable), TRUE);

				FreeProperties(&ColumnDesc.cPropertySets, &ColumnDesc.rgPropertySets);
			}
		}		
	}

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COL_ISLONG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_37()
{ 
	TBEGIN
	ULONG				cTypes = 0;
	ULONG				ulOrdinal = 0;
	CCol				tablecol;
	CCol				typecol;
	BOOL				fIsLong;
	CTable *			pTable = NULL;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES | DBCOLUMNDESCFLAGS_WTYPE;
	DBCOLUMNDESC		ColumnDesc;
	
	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;
	
	TESTC_(CreateTable(&pTable), S_OK);

	memset(&ColumnDesc, 0, sizeof(DBCOLUMNDESC));
	
	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= pTable->CountColumnsOnSchema(); ulOrdinal++ )
	{
		TESTC_(pTable->GetColInfo(ulOrdinal, tablecol), S_OK);

		if( !tablecol.GetUpdateable() )
				continue;

		if( tablecol.GetProviderType() == DBTYPE_WSTR  ||		
			tablecol.GetProviderType() == DBTYPE_STR  ||
			tablecol.GetProviderType() == DBTYPE_BYTES  )
		{			
			if( UpgradeType(&tablecol, &typecol, TRUE) )
			{
				odtLog << "Altering column: " << ulOrdinal << " of type: " << tablecol.GetProviderTypeName() << " to type: " << typecol.GetProviderTypeName() << "." << ENDL;
				TESTC_(SetChangeColInfo(&typecol), S_OK);

				// Get the ISLONG attribute of the new column
				fIsLong = typecol.GetIsLong();

				TESTC_(SetChangeColInfo(&typecol), S_OK);
				
				ColumnDesc.wType = tablecol.GetProviderType();
				
				::SetProperty(DBPROP_COL_ISLONG, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
					&ColumnDesc.rgPropertySets, (void *)(fIsLong ? VARIANT_TRUE : VARIANT_FALSE),
					DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
				
				COMPARE(VerifyAlterColumn(ulOrdinal, dwColumnDescFlags, &ColumnDesc, pTable), TRUE);

				FreeProperties(&ColumnDesc.cPropertySets, &ColumnDesc.rgPropertySets);
			}
		}		
	}

CLEANUP:

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COL_NULLABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_38()
{ 
	TBEGIN
	BOOL			fNullable;
	ULONG			cTypes = 0;
	ULONG			ulOrdinal = 0;
	CCol			tablecol;
	CCol			changecol;
	
	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_PROPERTIES) == 0 )
		return TEST_SKIPPED;
	
	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pTable->GetColInfo(ulOrdinal, tablecol), S_OK);

		if( tablecol.GetUnique() || tablecol.GetAutoInc() || !tablecol.GetUpdateable() )
				continue;

		// flip the NULLABLE attribute of the column
		fNullable = !tablecol.GetNullable();
		changecol = tablecol;
		changecol.SetNullable(fNullable);

		TESTC_(SetChangeColInfo(&changecol), S_OK);
			
		TESTC(VerifyAlterColumnProperty(ulOrdinal, DBPROP_COL_NULLABLE, (void *)(fNullable ? VARIANT_TRUE : VARIANT_FALSE), DBTYPE_BOOL));		
	}

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COL_DESCRIPTION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_39()
{ 
	TBEGIN
	ULONG			ulOrdinal = 0;
	CCol			tablecol;
	CCol			changecol;
	WCHAR *			pwszDescription = NULL;
	
	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_PROPERTIES) == 0 )
		return TEST_SKIPPED;

	pwszDescription = MakeObjectName(L"IALTTAB", MAXBUFLEN);
	TESTC(pwszDescription != NULL);
	
	// For each column in the table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pTable->GetColInfo(ulOrdinal, tablecol), S_OK);

		changecol = tablecol;
		changecol.SetColDescription(pwszDescription);

		odtLog << "Changing DESCRIPTION property on " << tablecol.GetColName() << ENDL;

		if( CHECK(SetChangeColInfo(&changecol), S_OK) )
		{
			COMPARE(VerifyAlterColumnProperty(ulOrdinal, DBPROP_COL_DESCRIPTION, (void *)pwszDescription, DBTYPE_BSTR), TRUE);		
		}
	}

CLEANUP:

	SAFE_FREE(pwszDescription);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COL_UNIQUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_40()
{ 
	return AlterPropertyAndVerifyRowset(DBPROP_COL_UNIQUE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COLUMNLCID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_41()
{ 
	TBEGIN
	ULONG				cIter = 0;
	ULONG				ulOrdinal = 0;
	CCol				col;
	CCol				onetablecol;
	CCol				changecol;
	CTable *			pTable = NULL;
	LONG				lLCID = 0;
	IColumnsInfo *		pIColumnsInfo = NULL;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_PROPERTIES) == 0 )
		return TEST_SKIPPED;

	// For each value in the schema rowset
	for( ulOrdinal = 1; ulOrdinal <= m_pFullSchemaInfo->CountColumnsOnSchema(); ulOrdinal++)
	{
		DBCOLUMNDESC *	pColumnDesc = NULL;

		TESTC_(m_pFullSchemaInfo->GetColInfo(ulOrdinal, col), S_OK);

		// DBPROP_COL_COLUMNLCID applies only to STR/WSTR/BYTES
		if( col.GetProviderType() != DBTYPE_STR && 
			col.GetProviderType() != DBTYPE_WSTR &&
			col.GetProviderType() != DBTYPE_BYTES )
			continue;	

		if( col.GetCreateParams() == NULL)
			continue;

		pColumnDesc = (DBCOLUMNDESC *)PROVIDER_ALLOC(sizeof(DBCOLUMNDESC));
		TESTC(pColumnDesc != NULL);
		TESTC_(m_pTable->BuildColumnDesc(pColumnDesc, col), S_OK);

		TESTC_(MakeNewColumnID(&pColumnDesc->dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);

		// Create fresh table
		pTable = new CTable(g_pIOpenRowset, L"IALTTAB");
		TESTC(pTable != NULL);		

		pTable->SetBuildColumnDesc(FALSE);
		pTable->SetColumnDesc(pColumnDesc, 1);

		TESTC_(pTable->CreateTable(0,0,NULL,PRIMARY,TRUE), S_OK);

		TESTC_(pTable->CreateRowset(USE_OPENROWSET, IID_IColumnsInfo, 0, 
				NULL, (IUnknown **)&pIColumnsInfo),S_OK);

		TESTC_(pTable->GetTableColumnInfo(&pTable->GetTableID(), pIColumnsInfo), S_OK);

		SAFE_RELEASE(pIColumnsInfo);


		TESTC_(pTable->GetColInfo(1, onetablecol), S_OK);

		if( onetablecol.GetLCID() == 0 )
		{
			// unable to determine column LCID
			TWARNING("Will not be able to verify column LCID.");
		}
		else
		{
			if( GetUserDefaultLCID() == LOCALE_ENGLISH_US )
			{
				if( onetablecol.GetLCID() == LOCALE_ENGLISH_US )
				{
					// Try German LCID since that is at least the same code page
					lLCID = MAKELCID(MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN), SORT_DEFAULT);
				}
				else
				{
					lLCID = LOCALE_ENGLISH_US;
				}
			}
			else
			{
				if( onetablecol.GetLCID() == GetUserDefaultLCID() )
					lLCID = LOCALE_ENGLISH_US;
				else
					lLCID = GetUserDefaultLCID();
			}
		}

		changecol = onetablecol;
		changecol.SetLCID(lLCID);

		TESTC_(SetChangeColInfo(&changecol), S_OK);
			
		COMPARE(VerifyAlterColumnProperty(ulOrdinal, DBPROP_COLUMNLCID, (void *)(LONG_PTR)lLCID, DBTYPE_I4), TRUE);		
					
		if( pTable )
		{
			pTable->DropTable();
			SAFE_DELETE(pTable);
		}
	}

CLEANUP:

	SAFE_FREE(pIColumnsInfo);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COL_PRIMARYKEY
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_42()
{ 
	return AlterPropertyAndVerifyRowset(DBPROP_COL_PRIMARYKEY);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COL_DEFAULT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_43()
{ 
	return AlterPropertyAndVerifyRowset(DBPROP_COL_DEFAULT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_COL_AUTOINCREMENT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_44()
{ 
	return AlterPropertyAndVerifyRowset(DBPROP_COL_AUTOINCREMENT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Multiple attribute change
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_45()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Test atomicity.  Valid new ID but one other invalid attribute
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_46()
{ 
	ULONG				ulOrdinal;
	DBCOLUMNDESC *		pColumnDesc = NULL;
	DBCOLUMNDESC		AlterColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_DBCID | DBCOLUMNDESCFLAGS_TYPENAME;
	CCol				col;

	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;

	memset(&AlterColumnDesc, 0, sizeof(DBCOLUMNDESC));

	// Make new column name and set invalid type name
	TESTC_(MakeNewColumnID(&AlterColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);
	AlterColumnDesc.pwszTypeName = L"GARBAGE";

	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);

		if( col.GetIsLong() || !col.GetUpdateable() )
				continue;

		TESTC_(AlterColumn(&m_pTable->GetTableID(), col.GetColID(),
					dwColumnDescFlags, &AlterColumnDesc), DB_E_BADTYPE);

		// Verify that column hasn't changed
		TESTC(CompareColumnMetaData(ulOrdinal, 0, NULL));
	}

CLEANUP:

	ReleaseDBID(&AlterColumnDesc.dbcid, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Test atomicity. Drop UNIQUE constraint and change to invalid type
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_47()
{ 
	ULONG				ulOrdinal;
	DBCOLUMNDESC *		pColumnDesc = NULL;
	DBCOLUMNDESC		AlterColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES|DBCOLUMNDESCFLAGS_WTYPE;
	CCol				col;

	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;

	memset(&AlterColumnDesc, 0, sizeof(DBCOLUMNDESC));

	// Look for a UNIQUE column
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);
	
		if( !col.GetUnique() )
			continue;			
		
		::SetProperty(DBPROP_COL_UNIQUE, DBPROPSET_COLUMN, &AlterColumnDesc.cPropertySets, 
			&AlterColumnDesc.rgPropertySets, (void *)VARIANT_FALSE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

		// Set invalid TYPE
		AlterColumnDesc.wType = DBTYPE_DBTIMESTAMP + 1;

		TESTC_(AlterColumn(&m_pTable->GetTableID(), col.GetColID(),
					dwColumnDescFlags, &AlterColumnDesc), DB_E_BADTYPE);

		// Verify that column hasn't changed
		TESTC(CompareColumnMetaData(ulOrdinal, 0, NULL));
	}

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc In an element of rgPropertySets, cProperties was > 0 but rgProperties was not null
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_48()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;
	DBPROPSET		PropSet;

	ColumnDesc.cPropertySets = 1;
	ColumnDesc.rgPropertySets = &PropSet;

	PropSet.guidPropertySet = DBPROPSET_COLUMN;
	PropSet.cProperties = 1;
	PropSet.rgProperties = NULL;

	TESTC_(AlterColumn(&m_pTable->GetTableID(), &m_ColumnID, DBCOLUMNDESCFLAGS_PROPERTIES,
					&ColumnDesc), E_INVALIDARG);
CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Drop multiple constraints
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_49()
{ 
	TBEGIN
	HRESULT				hr;
	ULONG				ulOrdinal = 0;
	CCol				col;
	CCol				onetablecol;
	DBCOLUMNDESCFLAGS	ColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES;
	CTable *			pTable = NULL;
	void *				pDefaultData = NULL;
	ULONG				cPropSets = 0;
	DBPROPSET *			rgPropSets = NULL;
	DBCOLUMNDESC		AlterColumnDesc;
	DBTYPE				wDefaultType;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_PROPERTIES) == 0 )
		return TEST_SKIPPED;

	memset(&AlterColumnDesc, 0, sizeof(DBCOLUMNDESC));

	// Create a normal table.
	// Add a PRIMARY KEY and a DEFAULT value

	// For each column in our Table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		CCol			onetablecol;

		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);

		if( col.GetIsLong() || col.GetAutoInc() || col.GetProviderType() == DBTYPE_BOOL || !col.GetUpdateable())
			continue;

		odtLog << "Dropping constraints on column: " << ulOrdinal << " of type: " << col.GetColName() << ENDL ;
		
		// Add a unique constraint
		::SetProperty(DBPROP_COL_UNIQUE, DBPROPSET_COLUMN, &cPropSets,
			&rgPropSets, (void *)VARIANT_TRUE, VT_BOOL, DBPROPOPTIONS_REQUIRED);

		TESTC(MakeDefaultData(&col, &pDefaultData, &wDefaultType, m_pTable));
	
		// Add a default value
		::SetProperty(DBPROP_COL_DEFAULT, DBPROPSET_COLUMN, &cPropSets, 
				&rgPropSets, pDefaultData, wDefaultType, DBPROPOPTIONS_REQUIRED);
		
		CHECK(hr = CreateOneColTableWithProps(&pTable, &col, cPropSets, rgPropSets), S_OK);
		if( S_OK == hr )
		{
			TESTC_(pTable->GetColInfo(1, onetablecol), S_OK);

			// The new table should have a default value
			// Cannot proceed if the Columnmetadata does not have default value info
			TESTC(VerifyTableDefault(pTable, TRUE));		

			// Now remove the UNIQUE constraint
			::SetProperty(DBPROP_COL_UNIQUE, DBPROPSET_COLUMN, &AlterColumnDesc.cPropertySets,
				&AlterColumnDesc.rgPropertySets, (void *)VARIANT_FALSE, VT_BOOL, DBPROPOPTIONS_REQUIRED);
		
			// Remove default value
			::SetProperty(DBPROP_COL_DEFAULT, DBPROPSET_COLUMN, &AlterColumnDesc.cPropertySets, 
					&AlterColumnDesc.rgPropertySets, (void *)NULL, VT_EMPTY, DBPROPOPTIONS_REQUIRED);

			TESTC_(hr = AlterColumn(&pTable->GetTableID(), onetablecol.GetColID(),
						ColumnDescFlags, &AlterColumnDesc), S_OK);
			
			// Verification of DEFAULT
			COMPARE(VerifyTableDefault(pTable, FALSE), TRUE);

			// Verification of UNIQUE
			COMPARE(VerifyTableUnique(pTable, FALSE), TRUE);
		}

		if( pTable )
		{
			pTable->DropTable();
			SAFE_DELETE(pTable);
		}

		ReleaseDefaultData(pDefaultData, wDefaultType);

		::FreeProperties(&cPropSets, &rgPropSets);
	}
	
CLEANUP:

	::FreeProperties(&cPropSets, &rgPropSets);
	::FreeProperties(&AlterColumnDesc.cPropertySets, &AlterColumnDesc.rgPropertySets);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Add multiple constraints
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_50()
{ 
	TBEGIN
	HRESULT				hr;
	ULONG				ulOrdinal = 0;
	CCol				col;
	CCol				onetablecol;
	DBCOLUMNDESCFLAGS	ColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES;
	CTable *			pTable = NULL;
	void *				pDefaultData = NULL;
	DBCOLUMNDESC		AlterColumnDesc;
	DBTYPE				wDefaultType;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_PROPERTIES) == 0 )
		return TEST_SKIPPED;

	memset(&AlterColumnDesc, 0, sizeof(DBCOLUMNDESC));

	// Create a normal table.
	// Add a PRIMARY KEY and a DEFAULT value

	// For each column in our Table
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		CCol			onetablecol;

		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);

		if( col.GetIsLong() || col.GetAutoInc() || col.GetProviderType() == DBTYPE_BOOL || !col.GetUpdateable())
			continue;

		odtLog << "Adding constraints on column: " << ulOrdinal << " of type: " << col.GetColName() << ENDL ;
		
		TESTC_(CreateOneColTableWithProps(&pTable, &col, 0, 0), S_OK);
		TESTC_(pTable->GetColInfo(1, onetablecol), S_OK);

		// The new table should not have a default value
		// Cannot proceed if the Columnmetadata does not have default value info
		TESTC(VerifyTableDefault(pTable, FALSE));

		// Add a primary key
		::SetProperty(DBPROP_COL_PRIMARYKEY, DBPROPSET_COLUMN, &AlterColumnDesc.cPropertySets,
			&AlterColumnDesc.rgPropertySets, (void *)VARIANT_FALSE, VT_BOOL, DBPROPOPTIONS_REQUIRED);

		TESTC(MakeDefaultData(&col, &pDefaultData, &wDefaultType, m_pTable));
	
		// Add a default value
		::SetProperty(DBPROP_COL_DEFAULT, DBPROPSET_COLUMN, &AlterColumnDesc.cPropertySets, 
				&AlterColumnDesc.rgPropertySets, pDefaultData, wDefaultType, DBPROPOPTIONS_REQUIRED);
		
		TESTC_(hr = AlterColumn(&pTable->GetTableID(), onetablecol.GetColID(),
					ColumnDescFlags, &AlterColumnDesc), S_OK);
		
		// Verification of DEFAULT
		COMPARE(VerifyTableDefault(pTable, TRUE), TRUE);

		// Verification of UNIQUE
		COMPARE(VerifyTableUnique(pTable, TRUE), TRUE);

		if( pTable )
		{
			pTable->DropTable();
			SAFE_DELETE(pTable);
		}

		ReleaseDefaultData(pDefaultData, wDefaultType);

		::FreeProperties(&AlterColumnDesc.cPropertySets, &AlterColumnDesc.rgPropertySets);
	}
	
CLEANUP:

	::FreeProperties(&AlterColumnDesc.cPropertySets, &AlterColumnDesc.rgPropertySets);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Test atomicity.  Alter column id along with unsupported ColumnDescFlag
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_51()
{ 
	TBEGIN
	CCol				col;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColFlag = 0;

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_DBCID) == 0 )
		return TEST_SKIPPED;
	
	TESTC_(m_pTable->GetColInfo(1, col), S_OK);
	TESTC_(m_pTable->BuildColumnDesc(&ColumnDesc, col), S_OK);

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);

	for( dwColFlag = DBCOLUMNDESCFLAGS_TYPENAME; dwColFlag <= DBCOLUMNDESCFLAGS_SCALE; dwColFlag <<= 1)
	{
		if( (m_lAlterColumnSupport & dwColFlag) == 0 )
		{
			if(CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
				dwColFlag | DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_NOTSUPPORTED))
			{
				// Verify no changes
				COMPARE(CompareColumnMetaData(1, 0, NULL), TRUE);
			}
		}
	}

	CHECK(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
			(~m_lAlterColumnSupport) | DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_NOTSUPPORTED);

CLEANUP:

	ReleaseColumnDesc(&ColumnDesc, 1, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Request BLOB using DBPROP_COL_ISLONG,FIXEDLENGTH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_52()
{ 
	AlterStringCol2BLOB(DBTYPE_STR, TRUE);
	AlterStringCol2BLOB(DBTYPE_STR, FALSE);
	AlterStringCol2BLOB(DBTYPE_WSTR, TRUE);
	AlterStringCol2BLOB(DBTYPE_WSTR, FALSE);
	AlterStringCol2BLOB(DBTYPE_BYTES, TRUE);
	AlterStringCol2BLOB(DBTYPE_BYTES, FALSE);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Request DBPROP_COL_NULLABLE on non-nullable type. Verify error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_53()
{ 
	TBEGIN
	HRESULT				hr;
	ULONG				ulOrdinal;
	ULONG				ulNonNullableCol = 0;
	CCol				schemacol;
	CCol				col;
	CTable *			pTable = NULL;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES;

	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;

	memset(&ColumnDesc, 0, sizeof(ColumnDesc));

	// Find a non-nullable column
	for( ulOrdinal = 1; ulOrdinal <= m_pFullSchemaInfo->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pFullSchemaInfo->GetColInfo(ulOrdinal, schemacol), S_OK);

		if( schemacol.GetNullable() == FALSE )
		{
			ulNonNullableCol = ulOrdinal;
			break;
		}
	}

	if( ulNonNullableCol == 0 )
	{
		odtLog << "Nonnullable column was not found" << ENDL;
		return TEST_SKIPPED;
	}

	// Create a table based on schemacol
	TESTC_(CreateOneColTableWithProps(&pTable, &schemacol, 0, NULL), S_OK);		
	
	// Set Nullable FALSE
	::SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
		&ColumnDesc.rgPropertySets, (void *)(VARIANT_TRUE),
		DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	TESTC_(pTable->GetColInfo(1, col), S_OK);
	
	hr = AlterColumn(&pTable->GetTableID(), col.GetColID(),
		dwColumnDescFlags, &ColumnDesc);

	TESTC_(hr, DB_E_ERRORSOCCURRED);
	TESTC(ColumnDesc.rgPropertySets[0].rgProperties[0].dwStatus == DBPROPSTATUS_BADVALUE ||
		ColumnDesc.rgPropertySets[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSUPPORTED);

CLEANUP:

	FreeProperties(&ColumnDesc.cPropertySets, &ColumnDesc.rgPropertySets);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Request PRIMARYKEY and UNIQUE constraints - verify error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_54()
{ 
	TBEGIN
	ULONG				ulOrdinal;
	CCol				schemacol;
	CTable *			pTable = NULL;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES;
	CCol				TempCol;

	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;

	memset(&ColumnDesc, 0, sizeof(ColumnDesc));

	// Find a non-nullable column
	for( ulOrdinal = 1; ulOrdinal <= m_pSchema->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pSchema->GetColInfo(ulOrdinal, schemacol), S_OK);

		if( !schemacol.GetUnique() && !schemacol.GetAutoInc() &&! schemacol.GetIsLong() &&
			schemacol.GetUpdateable() )
			break;
	}

	if( ulOrdinal == m_pSchema->CountColumnsOnSchema() )
	{
		return TEST_SKIPPED;
	}

	// Create a table based on schemacol
	TESTC_(CreateOneColTableWithProps(&pTable, &schemacol, 0, NULL), S_OK);		
	
	// Set Unique and Primary key to TRUE
	::SetProperty(DBPROP_COL_UNIQUE, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
		&ColumnDesc.rgPropertySets, (void *)(VARIANT_TRUE),
		DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	::SetProperty(DBPROP_COL_PRIMARYKEY, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
		&ColumnDesc.rgPropertySets, (void *)(VARIANT_TRUE),
		DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	TESTC_(pTable->GetColInfo(1, TempCol), S_OK);
	
	TESTC_(AlterColumn(&pTable->GetTableID(), TempCol.GetColID(),
			dwColumnDescFlags, &ColumnDesc), DB_E_ERRORSOCCURRED);

	//DBPROP_COL_UNIQUE
	TESTC(VerifyPropStatus(ColumnDesc.cPropertySets, ColumnDesc.rgPropertySets, DBPROP_COL_UNIQUE,	DBPROPSET_COLUMN, DBPROPSTATUS_OK));

	//DBPROP_COL_PRIMARYKEY
	TESTC(VerifyPropStatus(ColumnDesc.cPropertySets, ColumnDesc.rgPropertySets, DBPROP_COL_PRIMARYKEY,	DBPROPSET_COLUMN, DBPROPSTATUS_CONFLICTING));

CLEANUP:

	FreeProperties(&ColumnDesc.cPropertySets, &ColumnDesc.rgPropertySets);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Request AutoInc on non AUTO_UNIQUE_VALUE. Verify error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_55()
{ 
	TBEGIN
	ULONG				ulOrdinal;
	ULONG				ulNonAutoIncCol = 0;
	CCol				schemacol;
	CCol				schemacolBLOB;
	CTable *			pTable = NULL;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES;

	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;

	memset(&ColumnDesc, 0, sizeof(ColumnDesc));

	// Find a non-nullable column
	for( ulOrdinal = 1; ulOrdinal <= m_pSchema->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pSchema->GetColInfo(ulOrdinal, schemacol), S_OK);

		if( schemacol.CanAutoInc() == FALSE )
		{
			ulNonAutoIncCol = ulOrdinal;
			break;
		}
	}

	if( ulNonAutoIncCol == 0 )
	{
		odtLog << "Non auto inc column was not found" << ENDL;
		return TEST_SKIPPED;
	}

	// Create a table based on schemacol
	TESTC_(CreateOneColTableWithProps(&pTable, &schemacol, 0, NULL), S_OK);		
	
	// Set AUTOINC True
	::SetProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
		&ColumnDesc.rgPropertySets, (void *)(VARIANT_TRUE),
		DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
	
	COMPARE(VerifyAlterColumn(1, dwColumnDescFlags, &ColumnDesc, pTable), TRUE);

CLEANUP:

	FreeProperties(&ColumnDesc.cPropertySets, &ColumnDesc.rgPropertySets);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Request ISLONG with FIXEDLENGTH true.  Verify Error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_56()
{ 
	TBEGIN
	ULONG				ulOrdinal;
	CCol				schemacol;
	CTable *			pTable = NULL;
	DBCOLUMNDESC		ColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_PROPERTIES;
	CCol				TempCol;
	DBPROP *			pProp = NULL;

	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;

	memset(&ColumnDesc, 0, sizeof(ColumnDesc));

	// Find a non-nullable column
	for( ulOrdinal = 1; ulOrdinal <= m_pSchema->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pSchema->GetColInfo(ulOrdinal, schemacol), S_OK);

		if( !schemacol.GetUnique() && !schemacol.GetAutoInc() && !schemacol.GetIsLong() &&
			schemacol.GetUpdateable() && schemacol.GetProviderType() == DBTYPE_STR )
			break;
	}

	if( ulOrdinal == m_pSchema->CountColumnsOnSchema() )
	{
		return TEST_SKIPPED;
	}

	// Create a table based on schemacol
	TESTC_(CreateOneColTableWithProps(&pTable, &schemacol, 0, NULL), S_OK);		
	
	// Set ISLONG and FIXED properties to TRUE
	::SetProperty(DBPROP_COL_ISLONG, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
		&ColumnDesc.rgPropertySets, (void *)(VARIANT_TRUE),
		DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	::SetProperty(DBPROP_COL_FIXEDLENGTH, DBPROPSET_COLUMN, &ColumnDesc.cPropertySets, 
		&ColumnDesc.rgPropertySets, (void *)(VARIANT_TRUE),
		DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);

	TESTC_(pTable->GetColInfo(1, TempCol), S_OK);
	
	TESTC_(AlterColumn(&pTable->GetTableID(), TempCol.GetColID(),
			dwColumnDescFlags, &ColumnDesc), DB_E_ERRORSOCCURRED);

	//DBPROP_COL_ISLONG
	TESTC(VerifyPropStatus(ColumnDesc.cPropertySets, ColumnDesc.rgPropertySets, DBPROP_COL_ISLONG, DBPROPSET_COLUMN, DBPROPSTATUS_OK));

	TESTC(FindProperty(DBPROP_COL_ISLONG, DBPROPSET_COLUMN, ColumnDesc.cPropertySets,  ColumnDesc.rgPropertySets, &pProp))


	if( pProp->dwStatus == DBPROPSTATUS_NOTSUPPORTED )
	{		
		TESTC(VerifyPropStatus(ColumnDesc.cPropertySets, ColumnDesc.rgPropertySets, DBPROP_COL_FIXEDLENGTH,	DBPROPSET_COLUMN, DBPROPSTATUS_OK));
	}
	else
	{
		TESTC(VerifyPropStatus(ColumnDesc.cPropertySets, ColumnDesc.rgPropertySets, DBPROP_COL_FIXEDLENGTH,	DBPROPSET_COLUMN, DBPROPSTATUS_CONFLICTING));
	}

CLEANUP:

	FreeProperties(&ColumnDesc.cPropertySets, &ColumnDesc.rgPropertySets);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc Threads.  Test one thread altering a distinct col.  Verify each thread succeeds.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_57()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	

	ULONG		cIter = 0;
	DBORDINAL	ulColOrdinal = 0;
	DBID		rgColumnID[MAX_THREADS];
	THREADARG	rgTArg[MAX_THREADS];

	if( (m_lAlterColumnSupport & DBCOLUMNDESCFLAGS_DBCID) == 0 )
		return TEST_SKIPPED;

	//Setup Thread Arguments and Create Threads
	for( cIter = 0; cIter < MAX_THREADS; cIter++ )
	{
		ulColOrdinal = cIter + 1;
		// Create a thread to alter each column up to MAX_THREADS.
		if( ulColOrdinal > m_pSchema->CountColumnsOnSchema() )
			break;

		TESTC_(MakeNewColumnID(&rgColumnID[cIter], m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);

		rgTArg[cIter].pFunc = (void *)this;
		rgTArg[cIter].pArg1 = (void *)ulColOrdinal;
		rgTArg[cIter].pArg2 = (void *)&rgColumnID[cIter];
			
		CREATE_THREAD(cIter, Thread_AlterColumn, &rgTArg[cIter]);
	}

	START_THREADS();
	END_THREADS();		

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc ColumnDescFlags is 0. Verify no-op
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_58()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;
	DBCOLUMNDESC	ColumnDescCheck;
	
	memset(&ColumnDesc, 0xFF, sizeof(DBCOLUMNDESC));
	memset(&ColumnDescCheck, 0xFF, sizeof(DBCOLUMNDESC));

	TESTC_(AlterColumn(&m_pTable->GetTableID(), 
		&m_ColumnID, 0, &ColumnDesc), S_OK);

	TESTC(0 == memcmp(&ColumnDesc, &ColumnDescCheck, sizeof(DBCOLUMNDESC)));
	
CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Change Column ID and use new Column ID in select stmt
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_59()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;
	CTable *		pTable = NULL;
	CRowset			Rowset;

	if( m_lDBIDType != DBKIND_NAME )	
		return TEST_SKIPPED;

	if( m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTC_(CreateTable(&pTable), S_OK);

	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);	

	{
		CCol &NewCol = pTable->GetColInfoForUpdate(1);

		TESTC_(AlterColumn(&pTable->GetTableID(), NewCol.GetColID(), 
			DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), S_OK);

		NewCol.SetColName( ColumnDesc.dbcid.uName.pwszName );
		NewCol.SetColID( &ColumnDesc.dbcid );

		TESTC_(Rowset.CreateRowset(SELECT_COLLISTFROMTBL, IID_IRowset, pTable), S_OK);
		TESTC(Rowset.VerifyAllRows());
	}


CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Change column id to invalid keyword. Verify Error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_60()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;

	if( m_lDBIDType != DBKIND_NAME )	
		return TEST_SKIPPED;

	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);	
	SAFE_FREE(ColumnDesc.dbcid.uName.pwszName);

	ColumnDesc.dbcid.uName.pwszName = L"select";

	TESTC_(AlterColumn(&m_pTable->GetTableID(), m_Col.GetColID(), 
		DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), DB_E_BADCOLUMNID);

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Combination E_INVALIDARG conditiions
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_61()
{ 
	TBEGIN
	DBCOLUMNDESC	ColumnDesc;
	
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);

	TESTC_(AlterColumn(&m_pTable->GetTableID(), NULL, DBCOLUMNDESCFLAGS_DBCID, NULL), E_INVALIDARG);
	TESTC_(AlterColumn(NULL, m_Col.GetColID(), DBCOLUMNDESCFLAGS_DBCID, NULL), E_INVALIDARG);
	TESTC_(AlterColumn(NULL, NULL, DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), E_INVALIDARG);
	TESTC_(AlterColumn(NULL, NULL, DBCOLUMNDESCFLAGS_DBCID, NULL), E_INVALIDARG);
	
CLEANUP:

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc WTYPE and TYPENAME mismatch.  Verify DB_E_BADTYPE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_62()
{ 
	ULONG				ulOrdinal;
	DBCOLUMNDESC		AlterColumnDesc;
	DBCOLUMNDESCFLAGS	dwColumnDescFlags = DBCOLUMNDESCFLAGS_WTYPE | DBCOLUMNDESCFLAGS_TYPENAME;
	CCol				col;

	if( ~m_lAlterColumnSupport & dwColumnDescFlags )
		return TEST_SKIPPED;

	memset(&AlterColumnDesc, 0, sizeof(DBCOLUMNDESC));


	// Find an numeric type
	for( ulOrdinal = 1; ulOrdinal <= m_pTable->CountColumnsOnSchema(); ulOrdinal++)
	{
		TESTC_(m_pTable->GetColInfo(ulOrdinal, col), S_OK);

		if( col.GetUpdateable() && DBTYPE_I4 == col.GetProviderType() )
				break;
	}

	if( ulOrdinal > m_pTable->CountColumnsOnSchema() )
		return TEST_SKIPPED;

	AlterColumnDesc.pwszTypeName = col.GetProviderTypeName();
	AlterColumnDesc.wType = DBTYPE_NUMERIC;

	TESTC_(AlterColumn(&m_pTable->GetTableID(), col.GetColID(),
				dwColumnDescFlags, &AlterColumnDesc), DB_E_BADTYPE);

	// Verify that column hasn't changed
	TESTC(CompareColumnMetaData(ulOrdinal, 0, NULL));

CLEANUP:

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc SupportedTXNDDL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_63()
{ 
	TBEGIN

	CTable *			pTable = NULL;
	IAlterTable *		pIAlterTable = NULL;
	ITransactionLocal *	pITransactionLocal		= NULL;
	CCol				col;
	BOOL				fExists;
	DBID				NewTableID;
	DBCOLUMNDESC		ColumnDesc;
	VARIANT				vSupportedTxnDDL;

	TESTC_PROVIDER(VerifyInterface(m_pIAlterTable, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));

	VariantInit(&vSupportedTxnDDL);
	TESTC_PROVIDER(GetProperty(DBPROP_SUPPORTEDTXNDDL, DBPROPSET_DATASOURCEINFO, 
			g_pIDBInitialize, &vSupportedTxnDDL));
	
	TESTC(VT_I4 == vSupportedTxnDDL.vt);
	TESTC_PROVIDER(DBPROPVAL_TC_NONE != vSupportedTxnDDL.lVal);

	// Create a separate session
	TESTC_(CreateNewSession(g_pIDBInitialize, IID_IAlterTable, (IUnknown **)&pIAlterTable), S_OK);
	
	// create a table and populate it
	TESTC_(CreateTable(&pTable), S_OK);
	TESTC_(pTable->GetColInfo(1, col), S_OK);

	TESTC_(MakeNewTableID(&NewTableID, m_lDBIDType, wcslen(m_pTable->GetTableName())), S_OK);
	TESTC_(MakeNewColumnID(&ColumnDesc.dbcid, m_lDBIDType, wcslen(m_Col.GetColName())), S_OK);

	// start the transaction
	TESTC_(pITransactionLocal->StartTransaction(ISOLATIONLEVEL_READUNCOMMITTED, 0, NULL, NULL), S_OK);
	

	if( V_I4(&vSupportedTxnDDL) == DBPROPVAL_TC_DML )
	{
		//check for XACT_E_XTIONEXISTS
		TESTC_(m_pIAlterTable->AlterColumn(&pTable->GetTableID(), col.GetColID(), DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), XACT_E_XTIONEXISTS);
		TESTC_(m_pIAlterTable->AlterTable(&pTable->GetTableID(), &NewTableID, 0, NULL), XACT_E_XTIONEXISTS);
	}
	else if ( V_I4(&vSupportedTxnDDL) == DBPROPVAL_TC_DDL_IGNORE || 
			V_I4(&vSupportedTxnDDL) == DBPROPVAL_TC_ALL ||
			V_I4(&vSupportedTxnDDL) == DBPROPVAL_TC_DDL_COMMIT)
	{
		// change table and column IDs
		TESTC_(m_pIAlterTable->AlterColumn(&pTable->GetTableID(), col.GetColID(), DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), S_OK);
		TESTC_(m_pIAlterTable->AlterTable(&pTable->GetTableID(), &NewTableID, 0, NULL), S_OK);		
	}
	else if( V_I4(&vSupportedTxnDDL) == DBPROPVAL_TC_DDL_LOCK )
	{
		// Change the Column ID - this should lock the table
		TESTC_(m_pIAlterTable->AlterColumn(&pTable->GetTableID(), col.GetColID(), DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc), S_OK);

		// Check that the table is locked from a different session
		TESTC_(pIAlterTable->AlterTable(&pTable->GetTableID(), &m_pTable->GetTableID(), 0, NULL), DB_E_TABLEINUSE);

		// Commit
		TESTC_(pITransactionLocal->Commit(TRUE, 0, 0), S_OK); 

		// Change the Table ID - this should lock the table
		TESTC_(m_pIAlterTable->AlterTable(&pTable->GetTableID(), &NewTableID, 0, NULL), S_OK);

		// Check that the table is locked from a different session
		TESTC_(pIAlterTable->AlterTable(&pTable->GetTableID(), &NewTableID, 0, NULL), DB_E_TABLEINUSE);		
	}
			
	// Commit the transaction and don't retain
	if (DBPROPVAL_TC_DDL_COMMIT != vSupportedTxnDDL.lVal)
	{
		TESTC_(pITransactionLocal->Commit(FALSE, 0, 0), S_OK); 
	}
	else
	{
		TESTC_(pITransactionLocal->Commit(FALSE, 0, 0), XACT_E_NOTRANSACTION ); 
	}


	if( V_I4(&vSupportedTxnDDL) == DBPROPVAL_TC_DML ||
		V_I4(&vSupportedTxnDDL) == DBPROPVAL_TC_DDL_IGNORE )
	{	
		// Check that the table and column names are not changed
		TESTC_(pTable->DoesTableExist(&NewTableID, &fExists), S_OK);
		TESTC(fExists == FALSE);
		TESTC(CompareColumnMetaData(1, 0, NULL, pTable));
	}
	else
	{
		
		// check that the table and column names have changed
		TESTC_(pTable->DoesTableExist(&NewTableID, &fExists), S_OK);
		TESTC(fExists == TRUE);

		// Restore the table ID
		TESTC_(AlterTable(&NewTableID, &pTable->GetTableID()), S_OK);

		TESTC(CompareColumnMetaData(1, DBCOLUMNDESCFLAGS_DBCID, &ColumnDesc, pTable));
	}

CLEANUP:

	SAFE_RELEASE(pIAlterTable);
	SAFE_RELEASE(pITransactionLocal);

	if( pTable )
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}

	ReleaseDBID(&ColumnDesc.dbcid, FALSE);
	ReleaseDBID(&NewTableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Bad prop option
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_64()
{ 
	return VerifyAlterInvalidColumnProperties(m_pTable, DBPROPSTATUS_BADOPTION, 
		DBPROP_COL_SEED, DBPROPSET_COLUMN, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL+1);		
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Non column prop set  REQUIRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_65()
{ 
	return VerifyAlterInvalidColumnProperties(m_pTable, DBPROPSTATUS_NOTSUPPORTED, 
		DBPROP_TBL_TEMPTABLE, DBPROPSET_TABLE, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Non column prop set OPTIONAL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_66()
{ 
	return VerifyAlterInvalidColumnProperties(m_pTable, DBPROPSTATUS_NOTSUPPORTED, 
		DBPROP_COL_ISLONG, DBPROPSET_SESSION, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Mismatch in property vt type. REQUIRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_67()
{ 
	return VerifyAlterInvalidColumnProperties(m_pTable, DBPROPSTATUS_BADVALUE, 
		DBPROP_COL_UNIQUE, DBPROPSET_COLUMN, (void *)L"No", DBTYPE_BSTR, DBPROPOPTIONS_REQUIRED);	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc Mismatch in property vt type. OPTIONAL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_68()
{ 
	return VerifyAlterInvalidColumnProperties(m_pTable, DBPROPSTATUS_NOTSET, 
		DBPROP_COL_DESCRIPTION, DBPROPSET_COLUMN, (void *)LONG_MAX, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc Invalid property value. REQUIRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_69()
{ 
	return VerifyAlterInvalidColumnProperties(m_pTable, DBPROPSTATUS_BADVALUE, 
		DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, (void *)1, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc Invalid property value. OPTIONAL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_70()
{ 
	VARIANT vt;

	// Invalid default value
	VariantInit(&vt);
	V_VT(&vt) = VT_VARIANT;

	return VerifyAlterInvalidColumnProperties(m_pTable, DBPROPSTATUS_NOTSET, 
		DBPROP_COL_DEFAULT, DBPROPSET_COLUMN, (void *)&vt, DBTYPE_VARIANT, DBPROPOPTIONS_OPTIONAL);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc colid was not DB_NULLID.  OPTIONAL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_71()
{ 
	return VerifyAlterInvalidColumnProperties(m_pTable, DBPROPSTATUS_BADCOLUMN, 
		DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED, m_pTable->GetTableID());	
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc colid was not DB_NULLID.  REQUIRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAlterColumn::Variation_72()
{ 
	return VerifyAlterInvalidColumnProperties(m_pTable, DBPROPSTATUS_NOTSET, 
		DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, (void *)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL, m_pTable->GetTableID());	
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCAlterColumn::Terminate()
{ 
	SAFE_DELETE(m_pSchema);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIAlterTable::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END






// {{ TCW_TC_PROTOTYPE(TCZombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCZombie - Test the Zombie states of IAlterTable
//| Created:  	  09-01-99 
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombie::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		//register interface to be tested                                         
   		if(RegisterInterface(SESSION_INTERFACE, IID_IAlterTable))
			return TRUE;
	}

	//Not all providers have to support transactions
	//If a required interface, an error would ahve been posted by VerifyInterface
	TEST_PROVIDER(m_pITransactionLocal != NULL);
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ABORT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCZombie::Variation_1()
{ 
	TBEGIN
	IAlterTable *	pIAlterTable = NULL;

	//Start the Transaction
	//And obtain the IAlterTable interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIAlterTable))
	TESTC(pIAlterTable != NULL)

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE))
	
	//Verify we still can use IAlterTable after an ABORT			
	TESTC_(pIAlterTable->AlterTable(&m_pCTable->GetTableID(), NULL, 0, NULL), S_OK);

CLEANUP:

	SAFE_RELEASE(pIAlterTable);

	CleanUpTransaction(S_OK);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ABORT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCZombie::Variation_2()
{ 
	TBEGIN
	IAlterTable *	pIAlterTable = NULL;

	//Start the Transaction
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIAlterTable))
	TESTC(pIAlterTable!=NULL)

	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE))
	
	//Verify we still can use IAlterTable after an ABORT			
	TESTC_(pIAlterTable->AlterTable(&m_pCTable->GetTableID(), NULL, 0, NULL), S_OK);

CLEANUP:

	SAFE_RELEASE(pIAlterTable);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc COMMIT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCZombie::Variation_3()
{ 
	TBEGIN
	IAlterTable *	pIAlterTable = NULL;

	//Start the Transaction
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIAlterTable))
	TESTC(pIAlterTable != NULL)

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE))
	
	//Verify we still can use IAlterTable after an ABORT			
	TESTC_(pIAlterTable->AlterTable(&m_pCTable->GetTableID(), NULL, 0, NULL), S_OK);

CLEANUP:

	SAFE_RELEASE(pIAlterTable);
	CleanUpTransaction(S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc COMMIT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCZombie::Variation_4()
{ 
	TBEGIN
	IAlterTable *	pIAlterTable = NULL;

	//Start the Transaction
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIAlterTable))
	TESTC(pIAlterTable != NULL)

	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE))
	
	//Verify we still can use IAlterTable after an ABORT			
	TESTC_(pIAlterTable->AlterTable(&m_pCTable->GetTableID(), NULL, 0, NULL), S_OK);

CLEANUP:

	SAFE_RELEASE(pIAlterTable);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCZombie::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


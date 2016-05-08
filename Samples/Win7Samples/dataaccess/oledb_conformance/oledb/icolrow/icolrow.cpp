//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module IColRow.CPP 
//
//--------------------------------------------------------------------

#include "modstandard.hpp"

#define DBINITCONSTANTS		// Must be defined to initialize constants in oledb.h
#define INITGUID			// For IID_ITransactionOptions, etc.

#include "IColRow.h"
#include "msdasql.h"

#define INIT if(!Init_Var()) return TEST_FAIL;
#define FREE Free_Var();

// Globals
BOOL g_fKagera=FALSE;
BOOL g_fOracle=FALSE;
BOOL g_fCmd;			// IColumnsRowset supported on command object.
BOOL g_fRowset;			// IColumnsRowset supported on rowset object.

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xe74eb522, 0x7229, 0x11cf, { 0xaa, 0x61, 0x00, 0xa0, 0xc9, 0x05, 0x41, 0xce }};
DECLARE_MODULE_NAME("IColumnsRowset");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module for IColumnRowset Interface");
DECLARE_MODULE_VERSION(837735774);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	LPWSTR				wszProviderName=NULL;
	IColumnsRowset *	pIColRow = NULL;
	HRESULT				hr = E_FAIL;

	// Get connection and session objects
	if( ModuleCreateDBSession(pThisTestModule) )
	{
		IOpenRowset * pIOpenRowset				= (IOpenRowset *)pThisTestModule->m_pIUnknown2;
		ICommand * pICommand					= NULL;
		IDBCreateCommand * pIDBCreateCommand	= NULL;
		CTable * pTable							= NULL;

		// Check to see if we are using MSDASQL or MSDAORA
		g_fKagera=FALSE;
		g_fOracle = FALSE;

		// Assume no support on either rowset or command object
		g_fCmd		= FALSE;
		g_fRowset	= FALSE;
		
		if(GetProperty(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO, 
								pThisTestModule->m_pIUnknown, &wszProviderName))
		{
			if (!wcscmp((LPWSTR)wszProviderName, L"MSDASQL.DLL")) 
				g_fKagera=TRUE;
			if (!wcsncmp((LPWSTR)wszProviderName, L"MSDAORA.DLL", 7)) 
				g_fOracle=TRUE;
		}

		PROVIDER_FREE(wszProviderName);

		// Create a table we'll use for the whole test module,
		// store it in pVoid for now
		pThisTestModule->m_pVoid = new CTable(
			(IUnknown *)pThisTestModule->m_pIUnknown2,
			(LPWSTR)gwszModuleName);

		pTable = (CTable *)pThisTestModule->m_pVoid;

		if( !pTable )
		{
			odtLog << wszMemoryAllocationError;
			return FALSE;
		}

		// Start with a table with 10 rows								 
		if(!CHECK(pTable->CreateTable(10), S_OK))
			return FALSE;

		// Check for IColumnsRowset support on rowset
		hr = pIOpenRowset->OpenRowset(NULL,	&pTable->GetTableID(),
			NULL,IID_IColumnsRowset,0,NULL,	(IUnknown **)&pIColRow);

		if (E_NOINTERFACE == hr)
			odtLog << L"IColummnsRowset is not Supported by the Provider on rowset objects.\n";
		else if (!CHECK(hr, S_OK))
			odtLog << L"Provider claims IColumnsRowset is supported on rowsets but failed to provide the interface.\n";
		else
			g_fRowset = TRUE;

		// Now check for support on command objects
		SAFE_RELEASE(pIColRow);
		if (VerifyInterface(pIOpenRowset, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown **)&pIDBCreateCommand) &&
			(S_OK == (hr = pIDBCreateCommand->CreateCommand(NULL, IID_IColumnsRowset, (IUnknown **)&pIColRow))))
			g_fCmd = TRUE;
		else
		{
			if (!pIDBCreateCommand || CHECK(hr, E_NOINTERFACE))
				odtLog << L"IColummnsRowset is not Supported by the Provider on command objects.\n";
			else
				odtLog << L"Provider claims IColumnsRowset is supported on commands but failed to provide the interface.\n";
		}

		SAFE_RELEASE(pIColRow);
		SAFE_RELEASE(pIDBCreateCommand);
		
		if (!g_fRowset && !g_fCmd)
			return TEST_SKIPPED;

		// While not required in the spec we've set a precedent that both must be supported.
		COMPARE(g_fRowset, g_fCmd);

		// If we made it this far, everything has succeeded
		return TRUE;
	}
	
	return FALSE;
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
	// We still own the table since all of our testcases
	// have only used it and not deleted it.
	if( pThisTestModule->m_pVoid )
	{
		// Remove table from database & Delete CTable object
		((CTable *)pThisTestModule->m_pVoid)->DropTable();
		delete (CTable*)pThisTestModule->m_pVoid;
		pThisTestModule->m_pVoid = NULL;
	}
	
	return ModuleReleaseDBSession(pThisTestModule);
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class IColRow : public CSessionObject{

protected:

	// @cmember Count of DBCOLUMNINFO structs for original rowset
	DBORDINAL		m_cOriginalColumns;
	// @cmember Array of DBCOLUMNINFO structs for original rowset
	DBCOLUMNINFO *	m_pOriginalColumnsInfo;
	// @cmember Strings buffer for DBCOLUMNINFO structs for original rowset
	WCHAR *			m_pOriginalStringsBuffer;
	// @cmember Count of DBPROPSET structs for original rowset
	ULONG			m_cOriginalPropertySets;
	// @cmember Array of DBPROPSET structs for original rowset
	DBPROPSET *		m_pOriginalPropertySets;
	// @cmember Flag to indicate IColumnsRowset called on command object
	BOOL			m_fColRowsetOnCommand;

	// Original, unqualified table name
	WCHAR *			m_pwszBareTableName;
	// Catalog name, NULL if not available
	WCHAR * m_pwszCatalogName;
	// Schema name, NULL if not available
	WCHAR * m_pwszSchemaName;
	// Provider name
	WCHAR *			m_pwszProviderName;
	// DBMS name
	WCHAR *			m_pwszDBMSName;
	// Catalog support (0 for none)
	ULONG_PTR		m_ulCatalogSupport;
	// Schema support (0 for none)
	ULONG_PTR		m_ulSchemaSupport;
	// Driver ODBC version, only valid if provider is MSDASQL.DLL
	ULONG			m_ulDriverODBCVer;
	// Driver name, only valid if provider is MSDASQL.DLL
	WCHAR *			m_pwszDriverName;

public:

	//////////////////
	// Static
	//////////////////
	BOOL m_fOriginalRowsetHasBookmark;
	BOOL m_fColumnsRowsetHasBookmark;
	BOOL m_fColumnCacheDeferred;
	BOOL m_fColumnMayDefer;
	BOOL m_fHasOrdinalZero;
	
	// Which HRESULT should I expect 
	PREPARATION m_ePREPARATION;

	// Check column in rowset, only want to do it once.
	BOOL m_fCheckColumnInRowset;

	// @cmember Sql statement on command
	EQUERY	m_SQLSTMT;

	// @cmember Property on command
	ULONG	m_prop;

	// @cmember Count of Property Sets
	ULONG	m_cDBPROPSET;

	// @cmember Count of rows affected in ICommand::Execute
	DBROWCOUNT	m_cRowsAffected;

	// @cmember Array of Property Sets
	DBPROP  m_rgProperties[3];

	// @cmember Array of Property Sets
	DBPROPSET 	m_rgDBPROPSET[3];

	// @cmember Count of DBCOLUMNINFO structs
	DBORDINAL	m_cDBCOLUMNINFO;

	// @cmember Count of DBID
	DBORDINAL m_cDBID;

	// GetColumnsRowset
 	IID m_riid;

	//////////////////
	// Dynamic
	//////////////////

	// @cmember Rowset
	IRowset *			m_pIRowset;
	
	// Interface pointers
	ICommand *			m_pICommand;
	IColumnsInfo *		m_pIColumnsInfo;
	IColumnsRowset *	m_pIColumnsRowset;

	IRowset *			m_pIRowsetReturned;
	IRowset *			m_pIRowsetReturnedSelf;

	// @cmember Array of DBCOLUMNINFO structs
	DBCOLUMNINFO *	m_rgDBCOLUMNINFO;

	// @cmember Strings Buffer
	WCHAR *			m_pStringsBuffer;

	// @cmember Array of DBIDs
	DBID *	m_rgDBID;
		
	// Supported Property Sets and their properties
	ULONG			m_cPropertyInfoSets;
	DBPROPINFOSET *	m_rgPropertyInfoSets;
	WCHAR*			m_pDescBuffer;

	//@cmember Constructor
	IColRow(LPWSTR wszTestCaseName) : CSessionObject (wszTestCaseName)
		{
			m_cPropertyInfoSets=0;
			m_rgPropertyInfoSets=NULL;
			m_pDescBuffer=NULL;


			m_ePREPARATION=PREP_UNKNOWN;
			m_fCheckColumnInRowset=FALSE;
			m_SQLSTMT=SELECT_ALLFROMTBL;
			m_prop=0;
			m_cRowsAffected=0;
			m_cDBCOLUMNINFO=0;
			m_cDBID=0;
			m_riid=IID_NULL;
 			m_cDBPROPSET=1;
			m_pIRowset=NULL;
			m_pICommand=NULL;
			m_pIColumnsInfo=NULL;
			m_pIColumnsRowset=NULL;
			m_pIRowsetReturned=NULL;
			m_pIRowsetReturnedSelf=NULL;
			m_rgDBCOLUMNINFO=NULL;
			m_pStringsBuffer=NULL;
			m_rgDBID=NULL;
			m_fHasOrdinalZero=FALSE;
			m_pwszBareTableName = NULL;
			m_pwszCatalogName=NULL;
			m_pwszSchemaName=NULL;
			m_cOriginalColumns = 0;
			m_pOriginalColumnsInfo = NULL;
			m_pOriginalStringsBuffer = NULL;
			m_cOriginalPropertySets = 0;
			m_pOriginalPropertySets = NULL;
			m_fColRowsetOnCommand = TRUE;
			m_pwszProviderName = NULL;
			m_pwszDBMSName = NULL;
			m_ulCatalogSupport = 0;
			m_ulSchemaSupport = 0;
			m_ulDriverODBCVer = 0;
			m_pwszDriverName = NULL;
			VariantInit(&m_vValue);
			m_dwProp = 0;
		};
	
	//@cmember Destructor
	~IColRow(void)
		{
		ASSERT(!m_pIRowset);
		ASSERT(!m_pICommand);
		ASSERT(!m_pIColumnsInfo);
		ASSERT(!m_pIColumnsRowset);
		ASSERT(!m_rgDBID);
		ASSERT(!m_rgDBCOLUMNINFO);
		ASSERT(!m_pStringsBuffer);
		ASSERT(!m_pIRowsetReturned);
		ASSERT(!m_pIRowsetReturnedSelf);
		};


	// @cmember Initializes member variables for each variation
	BOOL Init_Var(void);

	//@cmember Common base class initialization
	BOOL Init();

	//@cmember Common base class termination
	BOOL Terminate();
	BOOL Free_Var();

	//@cmember ArrangeOptionalColumns
	BOOL ArrangeOptionalColumns
	(
		OPTCOLUMNS	eArrangeOption,				// [in] Enum for Optional DBID's
		HRESULT hrGetAvailableColumns = S_OK	// [in] hr to expect for GetAvailableCols
	);

	BOOL FreeOptionalColumns();

	// @cmember Maps to GetAvailableColumns
	HRESULT GetAvailableColumns(
		DBORDINAL * cOptColumns,					// [IN/OUT] Count of optional columns
		DBID **	rgOptColumns,						// [IN/OUT] Array of optional columns
		STATEMENTKIND stmt = eSELECT,				// [IN] kind of statement
		EQUERY query = SELECT_COLLISTFROMTBL,		// [IN] sql statement
		WCHAR * pSQL = NULL,						// [IN] client's choice for sql statement
		EPREPARE prepare = PREPARE,					// [IN] prepared state
		ULONG count = 1,							// [IN] run's prepared for
		DBPROPID property = 0						// [IN] property set
	);

	// @cmember GetColumnsRowset
	HRESULT GetAvailableColumns_row(
		DBORDINAL *cOptColumns,						// [IN/OUT] Count of optional columns
		DBID **	rgOptColumns,						// [IN/OUT] Array of optional columns
		STATEMENTKIND stmt = eSELECT,				// [IN] kind of statement
		EQUERY query = SELECT_COLLISTFROMTBL,		// [IN] sql statement
		WCHAR * pSQL = NULL,						// [IN] client's choice for sql statement
		EPREPARE prepare = NEITHER,					// [IN] prepared state
		ULONG count = 0,							// [IN] run's prepared for
		DBPROPID property = 0						// [IN] property set
	);
	// @cmember GetColumnsRowset
	HRESULT GetColumnsRowset(
		BOOL fCountOptColumns,						// [IN] Pass Count of Optional Columns? or NULL						
		BOOL fOptColumns,							// [IN] Pass Array of Optional Columns? or NULL
		IID riid = IID_IRowset,						// [IN] kind of rowset to return
		BOOL fRowsetReturned=TRUE,					// [IN/OUT] rowset returned
		OPTCOLUMNS columns = ALLDBID,				// [IN] which optional columns 
		STATEMENTKIND stmt = eSELECT,				// [IN] kind of statement
		EQUERY query = SELECT_COLLISTFROMTBL,		// [IN] sql statement
		WCHAR * pSQL = NULL,						// [IN] client's choice for sql statement
		EPREPARE prepare = PREPARE,					// [IN] prepared state
		ULONG count = 1,							// [IN] run's prepared for
		PROPVALS ePropStructure = VALIDROWSET,		// [IN] Property structure
		PROPOPTION ePropOption = ISOPTIONAL,		// [IN] Property dwOption
		DBPROPID property = 0,						// [IN] property set
		IRowset ** pIRowset = NULL,					// [IN] pIRowset
		BOOL fSkipPropertyCheck=FALSE,				// [IN] if not rowset property, skip it
		HRESULT	hrExpect = S_OK,					// [IN] hr to expect from GetColumnsRowset
		EEXECUTE eExecute = EXECUTE_NEVER,			// [IN]	whether to execute the statement to generate the rowset
		EINTERFACE eInterface = COMMAND_INTERFACE,	// [IN] Call GetColumns rowset on COMMAND_INTERFACE or ROWSET_INTERFACE
		BOOL fPassProps = TRUE,						// [IN] Pass property to GetColumnsRowset 
		enum AGGREGATION eAggregate = NONE,			// [IN] How to aggregate result
		IID	riidExec = IID_IRowset					// [IN] iid for Execute call
	);

	// @cmember GetColumnsRowset
	HRESULT GetColumnsRowset_row(
		BOOL fCountOptColumns,						// [IN] Pass Count of Optional Columns? or NULL						
		BOOL fOptColumns,							// [IN] Pass Array of Optional Columns? or NULL
		IID riid = IID_IRowset,						// [IN] kind of rowset to return
		BOOL fRowsetReturned=TRUE,					// [IN/OUT] rowset returned
		OPTCOLUMNS columns = ALLDBID,				// [IN] which optional columns 
		STATEMENTKIND stmt = eSELECT,				// [IN] kind of statement
		EQUERY query = SELECT_COLLISTFROMTBL,		// [IN] sql statement
		WCHAR * pSQL = NULL,						// [IN] client's choice for sql statement
		EPREPARE prepare = PREPARE,					// [IN] prepared state
		ULONG count = 1,							// [IN] run's prepared for
		PROPVALS ePropStructure = VALIDROWSET,		// [IN] Property structure
		PROPOPTION ePropOption = ISOPTIONAL,		// [IN] Property dwOption
		DBPROPID property = 0,						// [IN] property set
		IRowset ** pIRowset = NULL,					// [IN] pIRowset
		BOOL fSkipPropertyCheck=FALSE				// [IN] if not rowset property, skip it

	);

	// @cmember GetColumnsRowset
	HRESULT GetColumnsRowset_selfrow(
		BOOL fCountOptColumns,						// [IN] Pass Count of Optional Columns? or NULL						
		BOOL fOptColumns,							// [IN] Pass Array of Optional Columns? or NULL
		IID riid = IID_IRowset,						// [IN] kind of rowset to return
		BOOL fRowsetReturned=TRUE,					// [IN/OUT] rowset returned
		OPTCOLUMNS columns = ALLDBID,				// [IN] which optional columns 
		STATEMENTKIND stmt = eSELECT,				// [IN] kind of statement
		EQUERY query = SELECT_COLLISTFROMTBL,		// [IN] sql statement
		WCHAR * pSQL = NULL,						// [IN] client's choice for sql statement
		EPREPARE prepare = PREPARE,					// [IN] prepared state
		ULONG count = 1,							// [IN] run's prepared for
		DBPROPID property = 0,						// [IN] property set
		IRowset ** pIRowset = NULL					// [IN] pIRowset
	);

	//@cmember GetQueryInfo
	// How to handle more than 1 query column list
	BOOL GetQueryInfo(
		ULONG *			cCol1,
		ULONG *			cCol2,
		ULONG **		rgOrd1,
		ULONG **		rgOrd2,
		WCHAR ***		rgName1,
		WCHAR ***		rgName2
	);

	//@cmember Check_GetAvailableColumns
	BOOL Check_GetAvailableColumns(
		HRESULT			expectedHR,		// [IN]
		HRESULT			hr,				// [IN]
		DBORDINAL		cOptColumns,	
		DBID *			rgOptColumns);

	//@cmember Check_GetColumnsRowset
	BOOL Check_GetColumnsRowset(
		HRESULT			expectedHR,		// [IN]
		HRESULT			hr,				// [IN]
		DBORDINAL		cOptColumns,	// [IN]
		DBID *			rgOptColumns,	// [IN]
		IUnknown *		pColRowset,		// [IN]
		IID				iid,
		STATEMENTKIND	stmt = eSELECT	// [IN] kind of statement
	);

	//@cmember QIRowset
	BOOL IsSQLServer();
	BOOL QIRowset(IUnknown * pColRowset);
	BOOL GetPreparation();	
	BOOL IsPropertySupported(GUID propset, DBPROPID propid);
	BOOL IsPropertySet(GUID propset, DBPROPID propid, IUnknown * pIUnknown, LONG lValue);
	VOID FillPropertyStructure(PROPVALS ePropStructure, 
							   PROPOPTION ePropOption, DBPROPID property);
	
	// These are for checking the IColumnsInfo
	VOID AdjustStringLengths(COLROWINFO * g_rgColRowInfo);
	BOOL CheckIColumnsInfo(DBPROPSET * pColRowPropSets);
	BOOL Compare_pwszName(COLROWINFO s_rgColRowInfo, DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_pTypeInfo(DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_iOrdinal(ULONG ulOrdinal, DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_ulColumnSize(COLROWINFO s_rgColRowInfo, DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_dwType(COLROWINFO s_rgColRowInfo, DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_bPrecision(COLROWINFO s_rgColRowInfo, DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_bScale(COLROWINFO s_rgColRowInfo, DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_Columnid(COLROWINFO s_rgColRowInfo, DBCOLUMNINFO dbColumnInfo);
	
	BOOL Compare_DBCOLUMNFLAGS_ISBOOKMARK(DBORDINAL iOrdinal, WCHAR * pwszName, LONG dwFlags, BOOL fExpBookmarks);
	BOOL Compare_DBCOLUMNFLAGS_CACHEDEFERRED(WCHAR * pwszName, LONG dwFlags);
	BOOL Compare_DBCOLUMNFLAGS_ISFIXEDLENGTH(DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_DBCOLUMNFLAGS_ISLONG(DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_DBCOLUMNFLAGS_ISNULLABLE(COLROWINFO s_rgColRowInfo, DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_DBCOLUMNFLAGS_ISROWID(DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_DBCOLUMNFLAGS_ISROWVER(DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_DBCOLUMNFLAGS_MAYBENULLABLE(COLROWINFO s_rgColRowInfo, DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_DBCOLUMNFLAGS_MAYDEFER
	(
		WCHAR * pwszName,
		LONG dwFlags,
		DBORDINAL iRow,
		DBCOLUMNINFO * pColumnsInfo,
		DBPROPSET * pPropertySets
	);

	BOOL Compare_DBCOLUMNFLAGS_WRITE(DBCOLUMNINFO dbColumnInfo);
	BOOL Compare_DBCOLUMNFLAGS_WRITEUNKNOWN(DBCOLUMNINFO dbColumnInfo);

	// These are for checking the IColumnsInfo
	BOOL VerifyRowset(IUnknown * pIUnknown, IID iid, STATEMENTKIND	stmt);	
	BOOL VerifyRow(ULONG iRow, DBBINDING * rgBindings, DBCOUNTITEM cBindings, BYTE * pData);
	BOOL If_CHAR(DBTYPE dbtype);
	BOOL If_CHAR_Or_BYTE(DBTYPE dbtype);
	BOOL IsScaleUsed(DBTYPE dbtype);
	BOOL If_DATETIME(DBTYPE dbtype);
	BOOL CheckStatusInfo(DATA * pColumn);
	BOOL Compare_StatusInfo(CCol col, DBSTATUS dbStatus, DATA * pColumn, WCHAR * wszDsc,
		enum STATUS_ENUM eStatus = NO_NULLS_ALLOWED);
	BOOL Compare_DBCOLUMN_NAME(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_NUMBER(CCol col, ULONG iRow, DATA * pColumn);
	BOOL Compare_DBCOLUMN_TYPE(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_COLUMNSIZE(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_PRECISION(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_SCALE(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_BASECATALOGNAME(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_BASESCHEMANAME(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_TABLENAME(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_DATETIMEPRECISION(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_ISAUTOINCREMENT(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_ISSEARCHABLE(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_ISUNIQUE(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_OCTETLENGTH(CCol col, DATA * pColumn);
	BOOL Compare_DBCOLUMN_ISCASESENSITIVE(CCol col, DATA * pColumn);
	BOOL Compare_BOOL(CCol col, DATA * pColumn, WCHAR * wszDsc,
		STATUS_ENUM eStatus = NO_NULLS_ALLOWED);

protected:
	DBPROPID m_dwProp;
	VARIANT m_vValue;
	ULONG_PTR m_ulHiddenColumns;

	BOOL GetRowsetInfo(IUnknown * pIUnknown);
	void DumpCommandProps(IUnknown * pIUnknown, BOOL fPropertiesInError = FALSE);
	DBPROPID FindColumnProperty(IUnknown * pIDataSourceUnknown);
	BOOL ProviderSupports(enum FEATURE eFeature);
	BOOL GetSignificantProperties(void);
	void GetQualifierNames(	IUnknown * pSessionIUnknown,// [in]		IUnknown off session object
		LPWSTR	pwszTableName,							// [in]		the name of the table
		LPWSTR	*ppwszCatalogName,						// [out]	catalog name
		LPWSTR	*ppwszSchemaName);						// [out]	schema name

};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Init()
{
	m_cOriginalColumns=0;
	WCHAR * pwszQualifiedName = NULL;
	ULONG_PTR ulIdentCase = DBPROPVAL_IC_UPPER;
	ULONG_PTR ulQuotedIdentCase = DBPROPVAL_IC_UPPER;

	// Asserts will fire if someone calls init without calling terminate after
	// allocating buffers.
	ASSERT(m_pOriginalColumnsInfo==NULL);
	ASSERT(m_pOriginalStringsBuffer==NULL);

	m_pOriginalColumnsInfo=NULL;
	m_pOriginalStringsBuffer=NULL;

	// Initialize the COLEDB Class
	if (COLEDB::Init())
	{	
		// Setup all pointers
		if (m_pThisTestModule->m_pIUnknown)
		{
			m_pIDBInitialize = (IDBInitialize *)m_pThisTestModule->m_pIUnknown;
			m_pIDBInitialize->AddRef();
		}

		SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);	

		// GetQualifiedName returns a QUOTED name!  So we've got to either remove
		// the quotes or convert to proper case if DBPROP_IDENTIFIERCASE happens to 
		// be different than DBPROP_QUOTEDIDENTIFIERCASE and the QUOTEDIDENTIFIERCASE
		// is DBPROPVAL_IC_SENSITIVE.
		GetProperty(DBPROP_IDENTIFIERCASE, DBPROPSET_DATASOURCEINFO, 
								m_pThisTestModule->m_pIUnknown,&ulIdentCase);

		GetProperty(DBPROP_QUOTEDIDENTIFIERCASE, DBPROPSET_DATASOURCEINFO, 
							m_pThisTestModule->m_pIUnknown,&ulQuotedIdentCase);

		// Save the unqualified table name we used for validation with
		// BASETABLENAME.
		m_pwszBareTableName = wcsDuplicate(m_pTable->GetTableName());

		// Need to convert identifier to upper case or lower case
		if (ulIdentCase == DBPROPVAL_IC_UPPER)
			_wcsupr(m_pwszBareTableName);
		else if (ulIdentCase == DBPROPVAL_IC_LOWER)
			_wcslwr(m_pwszBareTableName);

		// Get significant properties for this provider
		GetSignificantProperties();

		// Change the table name to use a partially qualified name
		// Note: This is the only way to populate BASESCHEMANAME on Sql Server.
		// If this fails we'll just use the non-qualified name, but errors will occur
		// when validating BASESCHEMANAME.
		if (CHECK(m_pTable->GetQualifiedName(NULL, m_pwszSchemaName, 
								m_pwszBareTableName,&pwszQualifiedName), S_OK))
		{
			// GetQualifiedName returns a QUOTED name!  So we've got to either remove
			// the quotes or convert to proper case if DBPROP_IDENTIFIERCASE happens to 
			// be different than DBPROP_QUOTEDIDENTIFIERCASE and the QUOTEDIDENTIFIERCASE
			// is DBPROPVAL_IC_SENSITIVE.
			if (ulQuotedIdentCase != ulIdentCase)
			{
				// Need to convert identifier to upper case or lower case
				if (ulIdentCase == DBPROPVAL_IC_UPPER)
					_wcsupr(pwszQualifiedName);
				else if (ulIdentCase == DBPROPVAL_IC_LOWER)
					_wcslwr(pwszQualifiedName);
			}

			m_pTable->SetTableName(pwszQualifiedName);
			PROVIDER_FREE(pwszQualifiedName);
		}

		return TRUE;
	}  
	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Terminate
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Terminate()
{
	// Set the table name back to the bare name
	if (m_pwszBareTableName)
		m_pTable->SetTableName(m_pwszBareTableName);

	PROVIDER_FREE(m_pOriginalColumnsInfo);
	PROVIDER_FREE(m_pOriginalStringsBuffer);
	PROVIDER_FREE(m_pwszProviderName);
	PROVIDER_FREE(m_pwszDBMSName);
	PROVIDER_FREE(m_pwszCatalogName);
	PROVIDER_FREE(m_pwszSchemaName);
	PROVIDER_FREE(m_pwszBareTableName);
	PROVIDER_FREE(m_pwszDriverName);

	// Release the DSO
	SAFE_RELEASE(m_pIDBInitialize);
	ReleaseDBSession();

	return(COLEDB::Terminate());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Init_Var
//
// Initialize member variable
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Init_Var(void)
{
	// Initialize member variable
	m_prop			= 0;
	m_cRowsAffected	= 0;
  	m_cDBCOLUMNINFO	= 0;
	m_cDBCOLUMNINFO	= 0;
	m_cDBID			= 0;
	m_cDBPROPSET	= 0;

	m_fOriginalRowsetHasBookmark = FALSE;

	// command object from ctable
	if( !m_pIDBCreateCommand || 
		 FAILED(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,(IUnknown **)&m_pICommand)) )		
		return FALSE;

	// Check to see if ICommandPrepare is supported
	if( !GetPreparation() )
		return FALSE;

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Free_Var
//
// Clean-up member variable
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Free_Var()
{
	// 	Release the Objects and Free Memory
	SAFE_RELEASE(m_pIColumnsRowset);
	SAFE_RELEASE(m_pIColumnsInfo);
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetReturned);
	SAFE_RELEASE(m_pIRowsetReturnedSelf);
	PROVIDER_FREE(m_rgDBCOLUMNINFO);
	PROVIDER_FREE(m_pStringsBuffer);
	PROVIDER_FREE(m_rgDBID);
	PROVIDER_FREE(m_pOriginalColumnsInfo);
	PROVIDER_FREE(m_pOriginalStringsBuffer);
	FreeProperties(&m_cOriginalPropertySets, &m_pOriginalPropertySets);

	// If we set a property on now reset it
	if (V_VT(&m_vValue) != VT_EMPTY)
		CHECK(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, 
									m_dwProp, V_BOOL(&m_vValue), DBPROPOPTIONS_OPTIONAL), S_OK);

	SAFE_RELEASE(m_pICommand);

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// IsPropertySupported
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::IsPropertySupported(GUID propset, DBPROPID propid)
{
	HRESULT			hr					= E_FAIL;
	BOOL			fSucceed			= FALSE;
	IDBProperties *	pIDBProperties		= NULL;
	DBPROPIDSET		rgPropertyIDSet[1];
	DBPROPID *		rgPropertyIDs;

	// Make sure the DSO object is valid
	if( !m_pIDBInitialize )
		goto CLEANUP;

	// Get a IDBProperties pointer
	if( !CHECK(hr=m_pIDBInitialize->QueryInterface(IID_IDBProperties,
												(void**)&pIDBProperties),S_OK) )
		goto CLEANUP;

	// Init for IDBProperties
	rgPropertyIDs = &propid;
	rgPropertyIDSet->guidPropertySet = propset;

	rgPropertyIDSet->cPropertyIDs = 1;
	rgPropertyIDSet->rgPropertyIDs = rgPropertyIDs;

	// GetPropertyInfo
	if( FAILED(hr=pIDBProperties->GetPropertyInfo(1, rgPropertyIDSet,
					&m_cPropertyInfoSets, &m_rgPropertyInfoSets, &m_pDescBuffer)) )
		goto CLEANUP;

	// Check to see if the Property is Supported
	if((m_rgPropertyInfoSets->rgPropertyInfos->dwFlags == DBPROPFLAGS_NOTSUPPORTED))
		goto CLEANUP;

	PRVTRACE(L"[desc=%s],[propid=%d],[dwflags=%d],[vartype=%d]\n",
			m_rgPropertyInfoSets->rgPropertyInfos->pwszDescription,
			m_rgPropertyInfoSets->rgPropertyInfos->dwPropertyID,
			m_rgPropertyInfoSets->rgPropertyInfos->dwFlags,
			m_rgPropertyInfoSets->rgPropertyInfos->vtType);


	if (SettableProperty(propid, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) ||
		GetProperty(propid, DBPROPSET_ROWSET, m_pICommand, VARIANT_TRUE))
		fSucceed = TRUE;

CLEANUP:

	// Release the IDBProperty pointer	
	SAFE_RELEASE(pIDBProperties);

	// Clear the Property memory
	for(ULONG i=0; i<m_cPropertyInfoSets; i++)
	{
		for(ULONG j=0; j<m_rgPropertyInfoSets[i].cPropertyInfos; j++)
			VariantClear(&(m_rgPropertyInfoSets[i].rgPropertyInfos[j].vValues));

		if( m_rgPropertyInfoSets[i].rgPropertyInfos )
			PROVIDER_FREE(m_rgPropertyInfoSets[i].rgPropertyInfos);
	}

	// Free the IDBProperty Sets Memory
	if( m_rgPropertyInfoSets )
	{
		PROVIDER_FREE(m_rgPropertyInfoSets);
		m_rgPropertyInfoSets=NULL;
	}

	// Free the IDBProperty Description Memory
	if( m_pDescBuffer )
	{
		PROVIDER_FREE(m_pDescBuffer);
		m_pDescBuffer=NULL;
	}

	return fSucceed;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// IsPropertySet
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::IsPropertySet(GUID propset, DBPROPID propid, IUnknown * pIUnknown, LONG lValue)
{
	HRESULT				hr					= E_FAIL;
	BOOL				fSucceed			= FALSE;
	ICommandProperties *pICmdProperties		= NULL;
	IDBProperties *		pIDBProperties		= NULL;
	IRowsetInfo *		pIRowsetInfo		= NULL;
	DBPROPIDSET			rgPropertyIDSet[1];
	DBPROPID *			rgPropertyIDs;
	ULONG				cPropertySets;
	DBPROPSET *			rgPropertySets;

	// Init for Properties
	rgPropertyIDs = &propid;
	rgPropertyIDSet->guidPropertySet = propset;

	rgPropertyIDSet->cPropertyIDs = 1;
	rgPropertyIDSet->rgPropertyIDs = rgPropertyIDs;

	// Check to set what DBPROPSET is passed in
	if( !(memcmp(&propset, &(DBPROPSET_DATASOURCEINFO), sizeof(GUID))) )
	{
		// Make sure the DSO object is valid
		if( !m_pIDBInitialize )
			goto CLEANUP;

		// Get a IDBProperties pointer
		if( !CHECK(hr=m_pIDBInitialize->QueryInterface(IID_IDBProperties,
													(void**)&pIDBProperties),S_OK) )
			goto CLEANUP;

		// GetProperties
		hr=pIDBProperties->GetProperties(1, rgPropertyIDSet,
										  &cPropertySets, &rgPropertySets);
	}
	else if( (!(memcmp(&propset, &(DBPROPSET_ROWSET), sizeof(GUID)))) && (!pIUnknown) )
	{
		// Make sure the Command object is valid
		if( !m_pICommand )
			goto CLEANUP;

		// Get a ICommandProperties pointer
		if( !CHECK(hr=m_pICommand->QueryInterface(IID_ICommandProperties,
													(void**)&pICmdProperties),S_OK) )
			goto CLEANUP;

		// GetProperties
		hr=pICmdProperties->GetProperties(1, rgPropertyIDSet,
										  &cPropertySets, &rgPropertySets);
	}
	else if( (!(memcmp(&propset, &(DBPROPSET_ROWSET), sizeof(GUID)))) && (pIUnknown) )
	{
		// Get a IRosetInfo pointer
		if( !CHECK(hr=pIUnknown->QueryInterface(IID_IRowsetInfo,
													(void**)&pIRowsetInfo),S_OK) )
			goto CLEANUP;

		// GetProperties
		hr=pIRowsetInfo->GetProperties(1, rgPropertyIDSet,
									   &cPropertySets, &rgPropertySets);
	}
	else
		ASSERT(!("Please add DBPROPSET to if statement"));

	// Check to see if the Property is Supported and set to TRUE
	if( rgPropertySets->rgProperties->vValue.vt == VT_BOOL )
	{
		if( (hr == S_OK) && (rgPropertySets->rgProperties->dwStatus == DBPROPSTATUS_OK) &&
			((V_BOOL(&rgPropertySets->rgProperties->vValue)) == VARIANT_TRUE) )
			fSucceed = TRUE;
	}
	else
		if( (hr == S_OK) && (rgPropertySets->rgProperties->dwStatus == DBPROPSTATUS_OK) &&
			((V_I4(&rgPropertySets->rgProperties->vValue)) == lValue) )
			fSucceed = TRUE;

	PRVTRACE(L"[Propid=%d],[dwOptions=%d],[dwStatus=%d]\n",
			rgPropertySets->rgProperties->dwPropertyID,
			rgPropertySets->rgProperties->dwOptions,
			rgPropertySets->rgProperties->dwStatus);

CLEANUP:

	// Release the Properties pointer	
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICmdProperties);
	SAFE_RELEASE(pIDBProperties);

	// Clear the Property memory
	for(ULONG i=0; i<cPropertySets; i++)
	{
		VariantClear(&(rgPropertySets[i].rgProperties->vValue));
		PROVIDER_FREE(rgPropertySets[i].rgProperties);
	}

	// Free the IDBProperty Sets Memory
	if( rgPropertySets )
		PROVIDER_FREE(rgPropertySets);

	return fSucceed;
}
	
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Check_GetAvailableColumns
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
VOID IColRow::FillPropertyStructure		// Enum for property
(
	PROPVALS	ePropStructure,
	PROPOPTION	ePropOption,
	DBPROPID	property
)
{
	// Initialize the Properties
	m_cDBPROPSET = 1;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPROPSET[0].cProperties = 1;
	m_rgDBPROPSET[0].rgProperties = m_rgProperties;

	m_rgDBPROPSET[0].rgProperties[0].dwPropertyID = property;
	m_rgDBPROPSET[0].rgProperties[0].dwOptions = DBPROPOPTIONS_OPTIONAL;
	m_rgDBPROPSET[0].rgProperties[0].colid = DB_NULLID;
	m_rgDBPROPSET[0].rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&m_rgDBPROPSET[0].rgProperties[0].vValue) = VARIANT_TRUE;

	// Swith on the ERROR condition
	switch( ePropStructure )
	{
		case BADCOLID:
			memset(&m_rgDBPROPSET[0].rgProperties[0].colid, 1, sizeof(DBID));
			break;

		case BADTYPE:
			m_rgDBPROPSET[0].rgProperties[0].vValue.vt	= VT_DATE;
			break;

		case BADVALUE:
			V_BOOL(&m_rgDBPROPSET[0].rgProperties[0].vValue) = 666;
			break;
	}

	// Swith on the dwOption condition
	switch( ePropOption )
	{
		case REQUIRED:
			m_rgDBPROPSET[0].rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
			break;

		case ISOPTIONAL:
			m_rgDBPROPSET[0].rgProperties[0].dwOptions = DBPROPOPTIONS_OPTIONAL;
			break;

		case BADOPTION:
			m_rgDBPROPSET[0].rgProperties[0].dwOptions = MAX_ULONG;
			break;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	GetPreparation
//
//  See if the Provider supports ICommandPrepare.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::GetPreparation()
{
	HRESULT				hr				 = E_FAIL;
	ICommandPrepare*	pICommandPrepare = NULL;

	// Initialize the member
	m_ePREPARATION = NOTSUPPORTED;
	
	// If there is no ICommand Fail
	if( !m_pICommand )
		return FALSE;

	// Try to get a ICommandPrepare Object
	hr = m_pICommand->QueryInterface(IID_ICommandPrepare,
									(void **)&pICommandPrepare);

	// Check to see if Prepare is Supported
	if( hr == S_OK )
		m_ePREPARATION = SUPPORTED;
	else if( hr == E_NOINTERFACE )
		m_ePREPARATION = NOTSUPPORTED;
	else
		m_ePREPARATION = NOTSUPPORTED;

	// Release the ICommandPrepare Object
	SAFE_RELEASE(pICommandPrepare);

	// Return TRUE if it is supported or not supported
	if( (hr == S_OK) || (hr == E_NOINTERFACE) )
		return TRUE;
	else
		return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Find a writable DBPROPSET_ROWSET property with DBPROPFLAGS_COLUMNOK
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DBPROPID IColRow::FindColumnProperty(IUnknown * pIDataSourceUnknown)
{
	DBPROPIDSET rgPropertyIDSets[1];
	IDBProperties * pIDBProp = NULL;
	ULONG cPropertyInfoSets = 0;
	DBPROPINFOSET * pPropertyInfoSets = NULL;
	WCHAR * pDescBuffer = NULL;
	HRESULT hrGetProp = E_FAIL;
	DBPROPID dwColPropID = 0;
	ULONG iProp;

	rgPropertyIDSets[0].rgPropertyIDs = NULL;
	rgPropertyIDSets[0].cPropertyIDs = 0;
	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_ROWSET;

	TESTC(VerifyInterface(pIDataSourceUnknown, IID_IDBProperties, DATASOURCE_INTERFACE,
		(IUnknown **)&pIDBProp));

	TESTC_(pIDBProp->GetPropertyInfo(1, rgPropertyIDSets, &cPropertyInfoSets, &pPropertyInfoSets, NULL), S_OK);

	for (iProp=0; iProp < pPropertyInfoSets[0].cPropertyInfos; iProp++)
	{
		if ((pPropertyInfoSets[0].rgPropertyInfos[iProp].dwFlags & DBPROPFLAGS_COLUMNOK) &&
			(pPropertyInfoSets[0].rgPropertyInfos[iProp].dwFlags & DBPROPFLAGS_WRITE))
		{
			dwColPropID = pPropertyInfoSets[0].rgPropertyInfos[iProp].dwPropertyID;
			break;
		}
	}



CLEANUP:

	SAFE_RELEASE(pIDBProp);
	FreeProperties(&cPropertyInfoSets, &pPropertyInfoSets, &pDescBuffer);
	
	return dwColPropID;
}

// @cmember Gets the catalog and schema name for a given table
void IColRow::GetQualifierNames(
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
} 

// Retrieve properties of interest for this provider
BOOL IColRow::GetSignificantProperties(void)
{
	IRowset * pIRowset = NULL;
	WCHAR * pwszName = NULL;

	// Provider name
	GetProperty(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO, m_pThisTestModule->m_pIUnknown, &m_pwszProviderName);

	// DBMS Name
	GetProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, m_pThisTestModule->m_pIUnknown, &m_pwszDBMSName);

	// Catalog support
	GetProperty(DBPROP_CATALOGUSAGE, DBPROPSET_DATASOURCEINFO, m_pThisTestModule->m_pIUnknown, &m_ulCatalogSupport);

	// Schema support
	GetProperty(DBPROP_SCHEMAUSAGE, DBPROPSET_DATASOURCEINFO, m_pThisTestModule->m_pIUnknown, &m_ulSchemaSupport);

	// ODBC version, valid only if using Kagera, otherwise zero
	if (m_pwszProviderName && !wcscmp(m_pwszProviderName, L"MSDASQL.DLL"))
	{
		// Get driver version
		if (GetProperty(KAGPROP_DRIVERODBCVER, DBPROPSET_PROVIDERDATASOURCEINFO, 
				m_pThisTestModule->m_pIUnknown, &pwszName))
			m_ulDriverODBCVer = _wtoi(pwszName);

		// Get driver name KAGPROP_DRIVERNAME
		GetProperty(KAGPROP_DRIVERNAME, DBPROPSET_PROVIDERDATASOURCEINFO, m_pThisTestModule->m_pIUnknown,
			&m_pwszDriverName);

	}

	// Schema name and catalog name
	// This can fail if the provider doesn't support schema rowsets or the CURRENTCATALOG
	// property, or if memory allocation fails.
	GetQualifierNames(m_pThisTestModule->m_pIUnknown2, m_pwszBareTableName,
		&m_pwszCatalogName, &m_pwszSchemaName);

	SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(pwszName);

	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Determine if provider supports a given feature
//		Currently only works for CATALOG and SCHEMA features
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::ProviderSupports(enum FEATURE eFeature)
{
	BOOL fSupported = FALSE;
	ULONG ulBitMask = 0;

	switch(eFeature)
	{
		case CATALOGNAME:
			// We assume the provider supports catalogs if it supports
			// properties or literals associated with catalogs.
			if (m_pwszCatalogName && m_ulCatalogSupport)
				fSupported = TRUE;
			break;
		case SCHEMANAME:
			// We assume the provider supports schemas if it supports
			// properties or literals associated with schemas.
			if (m_pwszSchemaName && m_ulSchemaSupport)
				fSupported = TRUE;
			break;
	}		

	// Special note for Sql Server: Catalog name and schema name are only
	// available for server cursors, NULL for firehose cursor. This also
	// implies catalog and schema name will never be available from command
	// object.

	// Special note for Sockeye: DBPROP_SERVERCURSOR not correct on rowset
	if (fSupported && m_pwszDBMSName &&
		!wcscmp(m_pwszDBMSName, L"Microsoft SQL Server"))
	{

		// We're running against SQL Server and we think the feature is supported
		if (m_fColRowsetOnCommand)
			fSupported = FALSE;
		else
		{
			// Go through the rowset properties to find DBPROP_SERVERCURSOR
			for (ULONG iProp = 0; iProp < m_pOriginalPropertySets[0].cProperties; iProp++)
			{
				if ((m_pOriginalPropertySets[0].rgProperties[iProp].dwPropertyID == DBPROP_SERVERCURSOR) &&
					(m_pOriginalPropertySets[0].rgProperties[iProp].dwStatus != DBPROPSTATUS_NOTSUPPORTED))
				{
					// We found the property and it's supported
					if (V_VT(&m_pOriginalPropertySets[0].rgProperties[iProp].vValue) != VT_EMPTY &&
						V_BOOL(&m_pOriginalPropertySets[0].rgProperties[iProp].vValue) == VARIANT_FALSE)
					{
						// The value was FALSE, so the cursor is not a server cursor
						fSupported = FALSE;
					}
					break;
				}

			}
		}
			

	}

	// Special note for Sockeye: 2.x ODBC drivers cannot return BASECATALOG/SCHEMA name even
	// if they support catalogs or schemas.
	if (fSupported && m_pwszProviderName &&
		!wcscmp(m_pwszProviderName, L"MSDASQL.DLL"))
	{
		if (m_ulDriverODBCVer < 3)
		{
			// The driver was a 2.x driver
			fSupported = FALSE;
		}
	}


	return fSupported;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Print REQUIRED TRUE command props
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void IColRow::DumpCommandProps(IUnknown * pIUnknown, BOOL fPropertiesInError)
{
	DBPROPIDSET rgPropertyIDSets[1];
	ICommandProperties * pICmdProp = NULL;
	IRowsetInfo * pIRowsetInfo = NULL;
	ULONG cPropertySets = 0;
	DBPROPSET * pPropertySets = NULL;
	HRESULT hrGetProp = E_FAIL;

	rgPropertyIDSets[0].rgPropertyIDs = NULL;
	rgPropertyIDSets[0].cPropertyIDs = 0;
	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;
	if (SUCCEEDED(pIUnknown->QueryInterface(IID_ICommandProperties, (void **)&pICmdProp)))
	{
		ULONG cPropZero = 0;
		ULONG cPropZeroNULL = 0;

		if (!fPropertiesInError)
		{
			odtLog << L"Dumping command properties:\n";
			pICmdProp->GetProperties(0, NULL, &cPropertySets, &pPropertySets);
		}
		else if (fPropertiesInError)
		{
			odtLog << L"Dumping command properties in error:\n";
			pICmdProp->GetProperties(1, rgPropertyIDSets, &cPropertySets, &pPropertySets);
		}
		else
			odtLog << L"Invalid option:\n";
	}
	else if (SUCCEEDED(pIUnknown->QueryInterface(IID_IRowsetInfo, (void **)&pIRowsetInfo)))
	{
		pIRowsetInfo->GetProperties(0, NULL, &cPropertySets, &pPropertySets);
		odtLog << L"Dumping rowset properties:\n";
	}
	else
		odtLog << L"Not a command or rowset interface.\n";

	if (pPropertySets)
	{
		for (ULONG iPropSet = 0; iPropSet < cPropertySets; iPropSet++)
		{
			for (ULONG iProp = 0; iProp < pPropertySets[iPropSet].cProperties; iProp++)
			{
				if (V_BOOL(&pPropertySets[iPropSet].rgProperties[iProp].vValue) == VARIANT_TRUE)
				{
					DBPROPINFO * pPropInfo = NULL;

					pPropInfo = GetPropInfo(pPropertySets[iPropSet].rgProperties[iProp].dwPropertyID, pPropertySets[iPropSet].guidPropertySet,
						m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE);

					if (pPropInfo && pPropInfo->pwszDescription)
					{
						odtLog << L"\tProperty " << pPropInfo->pwszDescription << L" is on ";
						PROVIDER_FREE(pPropInfo->pwszDescription);
					}
					else
						odtLog << L"\tProperty " << pPropertySets[iPropSet].rgProperties[iProp].dwPropertyID << L" is on ";

					if (pPropertySets[iPropSet].rgProperties[iProp].dwOptions == DBPROPOPTIONS_REQUIRED)
						odtLog << L"REQUIRED.\n";
					else
						odtLog << L"OPTIONAL.\n";

					PROVIDER_FREE(pPropInfo);
				}

			}
			odtLog << L"\n";
		}

		FreeProperties(&cPropertySets, &pPropertySets);
	}
	SAFE_RELEASE(pICmdProp);
	SAFE_RELEASE(pIRowsetInfo);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Get the ColumnsInfo and Rowset properties needed from original rowset
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::GetRowsetInfo(IUnknown * pIUnknown)
{
	BOOL fResult = FALSE;
	IRowsetInfo *	pIRowsetInfo = NULL;
	ICommandProperties *	pICommandProperties = NULL;
	IColumnsInfo * pIColumnsInfo = NULL;
	DBPROPIDSET		rgPropertyIDSets;
	EINTERFACE		eInterface = ROWSET_INTERFACE;

	rgPropertyIDSets.rgPropertyIDs = NULL;
	rgPropertyIDSets.cPropertyIDs = 0;
	rgPropertyIDSets.guidPropertySet = DBPROPSET_ROWSET;

	if (!pIUnknown)
		return FALSE;

	// Find out if we're getting rowset or command properties here
	if (S_OK == pIUnknown->QueryInterface(IID_IRowsetInfo, (void **)&pIRowsetInfo))
	{
		// Get rowset properties
		TESTC_(pIRowsetInfo->GetProperties(1, &rgPropertyIDSets, &m_cOriginalPropertySets,
			&m_pOriginalPropertySets), S_OK);
	}
	else if (S_OK == pIUnknown->QueryInterface(IID_ICommandProperties, (void **)&pICommandProperties))
	{
		// Get command props
		TESTC_(pICommandProperties->GetProperties(1, &rgPropertyIDSets, &m_cOriginalPropertySets,
			&m_pOriginalPropertySets), S_OK);

		eInterface = COMMAND_INTERFACE;
	}
	else
		// Neither rowset nor command
		goto CLEANUP;

	if (!GetProperty(DBPROP_HIDDENCOLUMNS, 
				   DBPROPSET_ROWSET,pIUnknown, &m_ulHiddenColumns))
		   m_ulHiddenColumns = 0;

	// Get the IColumnsInfo for the original rowset.  There are some values not available in the CCol struct
	TESTC(VerifyInterface(pIUnknown, IID_IColumnsInfo, eInterface,
		(IUnknown **)&pIColumnsInfo));

	// Get the columns info for the original rowset.  If this was a non-rowreturning
	// statement off a command interface then m_pOriginalColumnsInfo is NULL.
	TESTC_(pIColumnsInfo->GetColumnInfo(&m_cOriginalColumns, &m_pOriginalColumnsInfo,
			&m_pOriginalStringsBuffer), S_OK);

	// See if Bookmarks are on turned on
	m_fOriginalRowsetHasBookmark=FALSE;
	if(m_pOriginalColumnsInfo && m_pOriginalColumnsInfo[0].iOrdinal == 0)
		m_fOriginalRowsetHasBookmark=TRUE;
	
	fResult = TRUE;

CLEANUP:

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICommandProperties);
	SAFE_RELEASE(pIColumnsInfo);

	return fResult;

}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	GetAvailableColumns on the Command
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT IColRow::GetAvailableColumns
(
	DBORDINAL *		cOptColumns,
	DBID **			rgOptColumns,
	STATEMENTKIND	stmt,				// [IN] kind of statement
	EQUERY			query,				// [IN] sql statement
	WCHAR *			pSQL,				// [IN] client's choice for sql statement
	EPREPARE		prepare,			// [IN] prepared state
	ULONG			count,				// [IN] run's prepared for
	DBPROPID		property			// [IN] property set
)
{
	HRESULT Exphr	= S_OK;
	HRESULT hr		= E_FAIL;

	// SetCommandText
	if( FAILED(hr=SetCommandText(m_pIMalloc, m_pICommand, 
								 m_pTable, NULL, stmt, query, pSQL)) )
		return hr;

	// Try to turn on the property.  Even if it's supposedly not supported we
	// try to turn it on to make sure nothing bad happens.
	if (!IsPropertySupported(DBPROPSET_ROWSET, property))
		Exphr = DB_E_ERRORSOCCURRED;

	CHECK(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, property), Exphr);

	// Prepare
	if( FAILED(hr=PrepareCommand(m_pICommand, prepare, count)) )
		return hr;

	if( FAILED(hr=m_pICommand->QueryInterface(IID_IColumnsRowset,
											 (void **)&m_pIColumnsRowset)) )
		return hr;
	
	// Get the Optional Columns
	hr = m_pIColumnsRowset->GetAvailableColumns(cOptColumns, rgOptColumns);

	return hr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	GetAvailableColumns on the Rowset
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT IColRow::GetAvailableColumns_row
(
	DBORDINAL *		cOptColumns,
	DBID **			rgOptColumns,
	STATEMENTKIND	stmt,				// [IN] kind of statement
	EQUERY			query,				// [IN] sql statement
	WCHAR *			pSQL,				// [IN] client's choice for sql statement
	EPREPARE		prepare,			// [IN] prepared state
	ULONG			count,				// [IN] run's prepared for
	DBPROPID		property			// [IN] property set
)
{
	HRESULT Exphr	= S_OK;
	HRESULT hr		= E_FAIL;

	// SetCommandText
	if( FAILED(hr=SetCommandText(m_pIMalloc, m_pICommand, 
								 m_pTable, NULL, stmt, query, pSQL)) )
		return hr;

	// Request IColumnsRowset property
	if( FAILED(hr=SetRowsetProperty(m_pICommand, 
									DBPROPSET_ROWSET, DBPROP_IColumnsRowset)) )
		return hr;

	// Check to see if the property is not supported
	if( (!property) || !IsPropertySupported(DBPROPSET_ROWSET, property) )
		Exphr = DB_E_ERRORSOCCURRED;

	CHECK(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, property), Exphr);

	// Prepare
	if( FAILED(hr=PrepareCommand(m_pICommand, prepare, count)) )
		return hr;

	// Execute the ICommandText
	hr=m_pICommand->Execute(NULL, IID_IRowset, NULL, 
									&m_cRowsAffected, (IUnknown **)&m_pIRowset);

	if (FAILED(hr))
		return hr;

	// If no Rowset return E_FAIL
	if( !m_pIRowset )
		return E_FAIL;
	
	if( FAILED(hr=m_pIRowset->QueryInterface(IID_IColumnsRowset,
											(void **)&m_pIColumnsRowset)) )
		return hr;
	
	// Get the Optional Columns
	hr = m_pIColumnsRowset->GetAvailableColumns(cOptColumns, rgOptColumns);

	return hr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Check_GetAvailableColumns
//
// This function needs to check the following:
//
// 1) Verify that columns returned are not mandatory columns
// 2) Verify that if count is returned as zero, that array is null
// 3) Verify that if error, count is returned as zero
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Check_GetAvailableColumns
(
	HRESULT	expectedHR,
	HRESULT	hr,
	DBORDINAL cOptColumns,
	DBID *	rgOptColumns
)
{
	BOOL fSucceed = TRUE;

	fSucceed &= CHECK(hr, expectedHR);

	// If the IColumnsRowset Succeeded
	if( SUCCEEDED(hr) )
	{
		// If the count of columns is 0 then the array should be NULL
		if( ((!cOptColumns) && (rgOptColumns)) ||
			((cOptColumns) && (!rgOptColumns)) )
		{
			odtLog << "Check_GetAvailableColumns: cOptColumns or rgOptColumns are incorrect\n";
			fSucceed=FALSE;
		}
		
		// Check if a mandatory column was returned
		for(ULONG iOpt=0; iOpt<cOptColumns; iOpt++)
		{
			for(ULONG iMand=1 ;iMand<MANCOL+1; iMand++)
			{
				if(memcmp(&(rgOptColumns[iOpt]),g_rgColRowInfo[iMand].columnid,sizeof(DBID)) == 0)
				{
					odtLog << L"Check_GetAvailableColumns:Mandatory Column found at " << iOpt<< ENDL;
					fSucceed = FALSE;
				}
			}
		}
	}
	else
	{
		// The count of optional Columns should be 0
		if( cOptColumns )
		{
			odtLog << L"Check_GetAvailableColumns:hr = E_xx, count != 0\n";
			fSucceed = FALSE;
		}

		// The array of optional Columns should be NULL
		if( rgOptColumns )
		{
			odtLog << L"Check_GetAvailableColumns:hr = E_xx, array != NULL\n";
			fSucceed = FALSE;
		}
	}

	return fSucceed;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	GetColumnsRowset
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT IColRow::GetColumnsRowset
(
	BOOL			fCountOptColumns,							
	BOOL			fOptColumns,
	IID				riidColRow,			// [IN] kind of rowset to return
	BOOL			fRowsetReturned,	// [IN/OUT] rowset returned
	OPTCOLUMNS		columns,			// [IN] which optional columns 
	STATEMENTKIND	stmt,				// [IN] kind of statement
	EQUERY			query,				// [IN] sql statement
	WCHAR *			pSQL,				// [IN] client's choice for sql statement
	EPREPARE		prepare,			// [IN] prepared state
	ULONG			count,				// [IN] run's prepared for
	PROPVALS		ePropStructure,		// [IN] Property structure
	PROPOPTION		ePropOption,		// [IN] Property dwOption
	DBPROPID		property,			// [IN] property set
	IRowset **		ppIRowset,			// [IN] rowset pointer
	BOOL			fSkipPropertyCheck,
	HRESULT			hrExpect,			// [IN] hr to expect from GetColumnsRowset
	EEXECUTE		eExecute,
	EINTERFACE		eInterface,			// [IN] Call GetColumns rowset on COMMAND_INTERFACE or ROWSET_INTERFACE
	BOOL			fPassProps,
	enum AGGREGATION eAggregate,		// [IN] How to aggregate result
	IID				riidExec			// [IN] iid for Execute call
)
{
	HRESULT			Exphr			= S_OK;
	HRESULT			hr				= E_FAIL;
	HRESULT			hrSetProp		= E_FAIL;
	DBORDINAL		cOptColumns		= 0;
	DBID *			rgOptColumns	= NULL;
	ULONG			index			= 0;
	IRowset *		pIOriginalRowset=NULL;
	HROW *			prghRows=NULL;
	ICommandPrepare * pICmdPrep = NULL;
	ICommandProperties * pICmdProp = NULL;
	IUnknown *		pIUnknown = NULL;
	IUnknown **		ppIUnknown = &pIUnknown;
	ULONG cRowsObtained = 0;
	IID riid = riidColRow;
	CAggregate * pAggregate = NULL;

	ASSERT(eInterface == COMMAND_INTERFACE || eInterface == ROWSET_INTERFACE);

	// For E_INVALIDARG case we have to force a NULL ppIUnknown.
	if (!ppIRowset)
		ppIUnknown = NULL;

	// Set the command Text
	TESTC_(SetCommandText(m_pIMalloc, m_pICommand,
								 m_pTable, NULL, stmt, query, pSQL), S_OK);

	// Get ICommandProperties
	TESTC(VerifyInterface(m_pICommand, IID_ICommandProperties, 
					COMMAND_INTERFACE, (IUnknown **)&pICmdProp));

	// Initialize the Properties
	if( property )
	{
		// Save the property we set so later we can reset.
		m_dwProp = property;
		FillPropertyStructure(ePropStructure, ePropOption, property);
	}

	if(property && !fSkipPropertyCheck)
	{
		// Get the current value of the property.   We put it back if setting actually succeeded.
		GetProperty(property, DBPROPSET_ROWSET, (IUnknown *)pICmdProp, &m_vValue);

		// If the prop isn't supported or isn't settable the provider should catch that
		if(!IsPropertySupported(DBPROPSET_ROWSET, property) ||
			(!SettableProperty(property, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,
				SESSION_INTERFACE) && V_BOOL(&m_vValue) == VARIANT_FALSE) ||
			ePropOption == BADOPTION ||
			ePropStructure != VALIDROWSET && ePropStructure != BADCOLID)
			CHECK(hrSetProp = pICmdProp->SetProperties(m_cDBPROPSET, m_rgDBPROPSET), DB_E_ERRORSOCCURRED);
		else if (ePropStructure == BADCOLID)
		{
			// Providers are allowed to validate colid at Execute time, so this could
			// return S_OK or DB_E_ERRORSOCCURRED.
			hrSetProp = pICmdProp->SetProperties(m_cDBPROPSET, m_rgDBPROPSET);

			// We'd really prefer DB_E_ERRORSOCCURRED
			if (hrSetProp != S_OK && hrSetProp != DB_E_ERRORSOCCURRED)
				CHECK(hrSetProp, DB_E_ERRORSOCCURRED);
		}
		else
		{
			// We used to validate this was set successfully, but some providers return the property is supported
			// but then fail with unsupported status when attempting to set. 
			hrSetProp = pICmdProp->SetProperties(m_cDBPROPSET, m_rgDBPROPSET);

			if (FAILED(hrSetProp))
			{
				if (CHECK(hrSetProp, DB_E_ERRORSOCCURRED))
				{
					if (COMPARE(m_rgDBPROPSET[0].rgProperties[0].dwStatus, DBPROPSTATUS_NOTSUPPORTED))
						// This is a valid failure, reset prop status for later checking
						m_rgDBPROPSET[0].rgProperties[0].dwStatus = DBPROPSTATUS_OK;
				}

			}
			else
				CHECK(hrSetProp, S_OK);
		}

		if (eExecute == EXECUTE_IFNOERROR && !SUCCEEDED(hr))
			goto CLEANUP;
	}

	// Set hr to expect from prepare
	if (stmt == eNOCOMMAND)
		Exphr = DB_E_NOCOMMAND;
	else
		Exphr = S_OK;

	// See if Prepare is supported.  If not then setting command text is sufficient
	// to allow GetColumnsInfo.
	VerifyInterface(m_pICommand, IID_ICommandPrepare, 
						COMMAND_INTERFACE, (IUnknown **)&pICmdPrep);

	// Prepare
	TESTC_(PrepareCommand(m_pICommand, prepare, count), Exphr);

	// Execute if requred
	if (eExecute != EXECUTE_NEVER)
	{
		if (eInterface == ROWSET_INTERFACE)
		{
			// Make sure we require IColumnsRowset on the rowset
			TESTC_(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, 
				DBPROP_IColumnsRowset), S_OK);
		}

		hr = m_pICommand->Execute(NULL, riidExec, NULL, NULL, (IUnknown **)&pIOriginalRowset);

		// If we asked for an unsupported interface here then we'll get E_NOINTERFACE
		if (E_NOINTERFACE == hr)
			odtLog << L"Interface not supported on Execute.\n";

		if (FAILED(hr))
			goto CLEANUP;

		TESTC(DefaultInterfaceTesting(pIOriginalRowset, ROWSET_INTERFACE, riidExec));

		TESTC(GetRowsetInfo(pIOriginalRowset));

		// Retrieve the first row
//		TESTC_(pIOriginalRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &prghRows), S_OK);
	}
	// We can't get columns info if not prepared when provider supports prepare
	else if (Exphr == S_OK && (prepare == PREPARE || !pICmdPrep))
		TESTC(GetRowsetInfo(m_pICommand));

	if (eInterface == COMMAND_INTERFACE)
	{
		// QI for IColumnsRowset off the Command
		TESTC_(hr=m_pICommand->QueryInterface(IID_IColumnsRowset, 
											 (void **)&m_pIColumnsRowset), S_OK);
		m_fColRowsetOnCommand = TRUE;
	}	
	else if (eInterface == ROWSET_INTERFACE && pIOriginalRowset)
	{
		// QI for IColumnsRowset off the Rowset
		TESTC_(hr=pIOriginalRowset->QueryInterface(IID_IColumnsRowset, 
											 (void **)&m_pIColumnsRowset), S_OK);
		m_fColRowsetOnCommand = FALSE;
	}
	else
	{
		hr = E_FAIL;
		goto CLEANUP;
	}

	
	// Setup Optional columns to the IColumnsRowset
	if(ArrangeOptionalColumns(columns, hrExpect) )
	{
		// Set Optional column count
		if( fCountOptColumns )
			cOptColumns = m_cDBID;

		// Set Optional column DBID's
		if( fOptColumns )
			rgOptColumns = m_rgDBID;
	}

	// Set up for aggregation
	if (eAggregate != NONE)
	{
		pAggregate = new CAggregate;

		TESTC(pAggregate != NULL);

		if (eAggregate == NOAGGREGATION)
		{
			riidColRow = IID_IRowset;
			pIUnknown = (IUnknown *)0x12345678;
		}
		else if (eAggregate == AGGREGATE)
			riidColRow = IID_IUnknown;
		else
			ASSERT(!L"Unknown aggregation request.");
	}

	// Get the ColumnsRowset
	hr = m_pIColumnsRowset->GetColumnsRowset(pAggregate, cOptColumns, rgOptColumns,
		riidColRow, (fPassProps) ? m_cDBPROPSET : 0, m_rgDBPROPSET, ppIUnknown);

	// Validate returned arguments
	if (FAILED(hr))
	{
		// Since we set pIUnknown to garbage we need to NULL if it's not, but
		// post a failure
		if (!COMPARE(pIUnknown == NULL, TRUE))
			pIUnknown = NULL;
	}
	else
		TESTC(pIUnknown != NULL);

	// Check aggregation results
	if (eAggregate != NONE)
	{
		if (riidColRow == IID_IUnknown)
		{
			// S_OK case

			// Set up inner unknown
			pAggregate->SetUnkInner(pIUnknown);

			// We're done with the inner unknown now
			SAFE_RELEASE(pIUnknown);

			// VerifyAggregationQI doesn't handle E_NOINTERFACE so don't call
			// it if the interface isn't supported on the object.
			// If GetColumnsRowset failed we still want to VerifyAggregation.
			if (FAILED(hr) || VerifyInterface(pAggregate, riid, 
				ROWSET_INTERFACE, (IUnknown **)&pIUnknown))
			{			
				// Aggregation should succeed for IID_IUnknown
				TESTC(pAggregate->VerifyAggregationQI(hr, riid));
			}
			else
				hr = E_NOINTERFACE;
		}
		else
			// DB_E_NOAGGREGATION
			TESTC(!pAggregate->VerifyAggregationQI(hr, riid));
	}

	// Populate output param and NULL out pIUnknown so it doesn't get released
	if (ppIRowset)
	{
		*ppIRowset = (IRowset *)pIUnknown;
		pIUnknown = NULL;
	}

	if (SUCCEEDED(hr))
		TESTC(DefaultInterfaceTesting(*ppIRowset, ROWSET_INTERFACE, riid));

	// If we asked for an unsupported interface here then we'll get E_NOINTERFACE
	if (E_NOINTERFACE == hr)
		odtLog << L"Interface not supported on IColumnsRowset.\n";

CLEANUP:

	// Release Objects
	if (prghRows)
	{
		pIOriginalRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL);
		PROVIDER_FREE(prghRows);
	}
	SAFE_RELEASE(pIOriginalRowset);

	// If setting the property was not successful, then reset the variant value so later we won't try
	// to reset the property.  We can't reset here because the columns rowset might be open.
	if (S_OK != hrSetProp)
		VariantClear(&m_vValue);

	SAFE_RELEASE(pICmdProp);
	SAFE_RELEASE(pICmdPrep);
	SAFE_RELEASE(pAggregate);
	SAFE_RELEASE(pIUnknown);

	// Free the Optional columns
	FreeOptionalColumns();

	CHECK(hr, hrExpect);

	return hr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//	GetColumnsRowset_row
//		Note: This function is now obsolete, you should use GetColumnsRowset.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
HRESULT IColRow::GetColumnsRowset_row
(
	BOOL			fCountOptColumns,							
	BOOL			fOptColumns,
	IID				riid,				// [IN] kind of rowset to return
	BOOL			fRowsetReturned,	// [IN/OUT] rowset returned
	OPTCOLUMNS		columns,			// [IN] which optional columns 
	STATEMENTKIND	stmt,				// [IN] kind of statement
	EQUERY			query,				// [IN] sql statement
	WCHAR *			pSQL,				// [IN] client's choice for sql statement
	EPREPARE		prepare,			// [IN] prepared state
	ULONG			count,				// [IN] run's prepared for
	PROPVALS		ePropStructure,		// [IN] Property structure
	PROPOPTION		ePropOption,		// [IN] Property dwOption
	DBPROPID		property,			// [IN] property set
	IRowset **		ppIRowset,			// [IN] rowset pointer
	BOOL			fSkipPropertyCheck	// [IN] if not rowset property, skip it

)
{
	HRESULT			Exphr			= S_OK;
	HRESULT			hr				= E_FAIL;
	DBORDINAL		cOptColumns		= 0;
	DBID *			rgOptColumns	= NULL;

	// Set the command Text
	if( FAILED(hr=SetCommandText(m_pIMalloc, m_pICommand,
								 m_pTable, NULL, stmt, query, pSQL)) )
		return hr;


	if(!fSkipPropertyCheck)
	{
		// Check to see if the property is not supported
		if( (!property) || !IsPropertySupported(DBPROPSET_ROWSET, property) || ePropStructure == BADCOLID )
			Exphr = DB_E_ERRORSOCCURRED;

		hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, property);

		// If we expect and got failure then proceed.
		// If we expect failure but succeeded then proceed but post a failure
		if (!CHECK(hr, Exphr) && FAILED(hr))
			goto CLEANUP;

	}
	else
		Exphr = S_OK;

	// Prepare
	if( FAILED(hr=PrepareCommand(m_pICommand, prepare, count)) )
		return hr;

	// Check to see if the Property is not supported
	if( !IsPropertySupported(DBPROPSET_ROWSET, DBPROP_IColumnsRowset) )
		Exphr = DB_E_ERRORSOCCURRED;
	else
		Exphr = S_OK;


	CHECK(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, 
											DBPROP_IColumnsRowset), Exphr);

	// Execute the command to get a Rowset
	if(!CHECK(hr=m_pICommand->Execute(NULL, IID_IRowset, NULL, 
							&m_cRowsAffected, (IUnknown **)&m_pIRowset), S_OK) )
		return hr;

	if( !m_pIRowset )
		return E_FAIL;

	TESTC(GetRowsetInfo(m_pIRowset));

	// QI for IColumnsRowset off of the Rowset
	if( FAILED(hr=m_pIRowset->QueryInterface(IID_IColumnsRowset,(void **) &m_pIColumnsRowset)) )
		return hr;

	// Setup Optional columns to the IColumnsRowset
	if( !ArrangeOptionalColumns(columns) )
		return E_FAIL;

	// Set Optional column count
	if( fCountOptColumns )
		cOptColumns = m_cDBID;

	// Set Optional column DBID's
	if( fOptColumns )
		rgOptColumns = m_rgDBID;

	// Initialize the Properties
	if( property )
		FillPropertyStructure(ePropStructure, ePropOption, property);

	// Get the ColumnsRowset
	hr = m_pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns,
				rgOptColumns, riid, m_cDBPROPSET, m_rgDBPROPSET, PPI ppIRowset);

CLEANUP:
	
	// Free the Optional columns
	FreeOptionalColumns();

	return hr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	GetColumnsRowset_selfrow
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT IColRow::GetColumnsRowset_selfrow
(
	BOOL			fCountOptColumns,							
	BOOL			fOptColumns,
	IID				riid,				// [IN] kind of rowset to return
	BOOL			fRowsetReturned,	// [IN/OUT] rowset returned
	OPTCOLUMNS		columns,			// [IN] which optional columns 
	STATEMENTKIND	stmt,				// [IN] kind of statement
	EQUERY			query,				// [IN] sql statement
	WCHAR *			pSQL,				// [IN] client's choice for sql statement
	EPREPARE		prepare,			// [IN] prepared state
	ULONG			count,				// [IN] run's prepared for
	DBPROPID		property,			// [IN] property set
	IRowset **		ppIRowset			// [IN] rowset pointer
)
{
	HRESULT			Exphr			= S_OK;
	HRESULT			hr				= E_FAIL;
	DBORDINAL		cOptColumns		= 0;
	DBID *			rgOptColumns	= NULL;

	// Set the command Text
	if( FAILED(hr=SetCommandText(m_pIMalloc, m_pICommand,
								 m_pTable, NULL, stmt, query, pSQL)) )
		return hr;

	// Prepare
	if( FAILED(hr=PrepareCommand(m_pICommand, prepare, count)) )
		return hr;

	// Check to see if the Property is not supported
	if( !IsPropertySupported(DBPROPSET_ROWSET, DBPROP_IColumnsRowset) )
		Exphr = DB_E_ERRORSOCCURRED;

	CHECK(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, 
											DBPROP_IColumnsRowset), Exphr);

	// Execute the command to get a Rowset
	if(!CHECK(hr=m_pICommand->Execute(NULL, IID_IRowset, NULL, 
							&m_cRowsAffected, (IUnknown **)&m_pIRowset), S_OK) )
		return hr;

	if( !m_pIRowset )
		return E_FAIL;

	// QI for IColumnsRowset off of the Rowset
	if( FAILED(hr=m_pIRowset->QueryInterface(IID_IColumnsRowset,(void **) &m_pIColumnsRowset)) )
		return hr;

	// Setup Optional columns to the IColumnsRowset
	if( !ArrangeOptionalColumns(columns) )
		return E_FAIL;

	// Set Optional column count
	if( fCountOptColumns )
		cOptColumns = m_cDBID;

	// Set Optional column DBID's
	if( fOptColumns )
		rgOptColumns = m_rgDBID;

	// Initialize the Properties
	if( property )
		FillPropertyStructure(VALIDROWSET, ISOPTIONAL, property);

	// Set property if Supported
	if( (!property) || 
		((property) && IsPropertySupported(DBPROPSET_ROWSET, property)) )
	{
		// Get IColumnsRowset
		if(FAILED(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, 
				rgOptColumns, riid, m_cDBPROPSET, m_rgDBPROPSET, PPI &m_pIRowsetReturned)))
			return hr;
	}
	else
		CHECK(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, rgOptColumns, 
				riid, m_cDBPROPSET, m_rgDBPROPSET, PPI &m_pIRowsetReturned),DB_S_ERRORSOCCURRED);

	TESTC(GetRowsetInfo(m_pIRowsetReturned));

	// Free the Optional columns
	FreeOptionalColumns();

	// Release the IColumnsRowset Object
	SAFE_RELEASE(m_pIColumnsRowset);

	// QI for IColumnsRowset off of the Rowset
	if( FAILED(hr=m_pIRowsetReturned->QueryInterface(IID_IColumnsRowset,
													(void **) &m_pIColumnsRowset)) )
		return hr;

	// Setup Optional columns to the IColumnsRowset
	if( !ArrangeOptionalColumns(columns) )
		return E_FAIL;

	// Set Optional column count
	if( fCountOptColumns )
		cOptColumns = m_cDBID;

	// Set Optional column DBID's
	if( fOptColumns )
		rgOptColumns = m_rgDBID;

	// Initialize the Properties
	if( property )
		FillPropertyStructure(VALIDROWSET, ISOPTIONAL, property);

	// Set property if Supported
	if( (!property) || 
		((property) && IsPropertySupported(DBPROPSET_ROWSET, property)) )
	{
		// Get IColumnsRowset
		if(FAILED(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, 
				rgOptColumns, riid, m_cDBPROPSET, m_rgDBPROPSET, PPI ppIRowset)))
			return hr;
	}
	else
		CHECK(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, rgOptColumns, 
				riid, m_cDBPROPSET, m_rgDBPROPSET, PPI ppIRowset),DB_S_ERRORSOCCURRED);

	SAFE_RELEASE(m_pIRowset); 
	SAFE_RELEASE(m_pIRowsetReturned);
	
CLEANUP:
	
	// Free the Optional columns
	FreeOptionalColumns();

	return hr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Check_GetColumnsRowset()
//
// This function needs to check the following based on rowset or simple rowset:
// 
// 1) if succeeded, QI for mandatory interfaces on object
// 2) if succeeded, When called on itself, check column name
// 3) if succeeded, Verify Maxlength, Precision, and Scale for non-applicable datatypes
// 4) if succeeded, Check ComputeMode
// 5) if succeeded, Order of rows is order of columns in query
// 6) if FAILED,    pColRowset == NULL
// 
// TODO: What happens for Invalid DBIDs
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Check_GetColumnsRowset
(
	HRESULT			expectedHR,
	HRESULT			hr,
	DBORDINAL		cOptColumns,
	DBID *			rgOptColumns,
	IUnknown *		pColRowset,
	IID				iid,
	STATEMENTKIND	stmt
)
{
	BOOL fSucceed = FALSE;

	// Check the HRESULT of the call
	if( SUCCEEDED(hr) )
	{
		// Make sure all DBPROP structures have dwStatus of DBPROPSTATUS_OK
		if( S_OK==hr && m_cDBPROPSET && m_rgDBPROPSET )
		{
			for(ULONG i=0; i<m_cDBPROPSET; i++)
			{
				for(ULONG j=0 ;j<m_rgDBPROPSET[i].cProperties ;j++)
				{
					if( m_rgDBPROPSET[i].rgProperties[j].dwStatus != DBPROPSTATUS_OK )
						odtLog << L"S_OK: not all DBPROP[x].dwStatus == DBPROPSTATUS_OK\n";
  
				}
			}
		}

		// Check QI for all mandatory Interfaces
		if( pColRowset && QIRowset(pColRowset) && COMPARE(VerifyRowset(pColRowset,iid,stmt), TRUE))
			fSucceed = TRUE;
	}
	else
	{
		// The Rowset pointer should be NULL on an E_Code
		if( !pColRowset )
			fSucceed = TRUE;
		else
			odtLog << L"Check_GetColumnsRowset:hr = E_xx, *ppColRowset !=NULL\n";
	}

	fSucceed &= COMPARE(expectedHR, hr);

	return fSucceed;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	ArrangeOptionalColumns
//  
//	Arrange DBID's in requested order and return in same params
//
//	const ULONG ALLDBID =			0;
//	const ULONG SOMEDBID =			1;
//	const ULONG REVERSEDBID =		2;
//	const ULONG DUPLICATEDBID =		3;
//	const ULONG INVALIDDBID =		4;
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::ArrangeOptionalColumns
(
	OPTCOLUMNS	eArrangeOption,		// [in] Enum for Optional DBID's
	HRESULT hrGetAvailableColumns	// [in] hr to expect for GetAvailableCols
)
{
	HRESULT	hr			= E_FAIL;
	BOOL	fSucceed	= FALSE;
 	DBORDINAL index		= 0;
	DBORDINAL givenIndex= 0;

 	DBORDINAL cOriginalColumns  = 0;
	DBID *	rgOriginalColumns = NULL;
	
	// Clear the count & array of DBID's
	FreeOptionalColumns();
 
	// Make sure we have interface
	if(!m_pIColumnsRowset)
		goto CLEANUP;

	// Get the list of Optional Columns
	hr=m_pIColumnsRowset->GetAvailableColumns(&cOriginalColumns,
											  &rgOriginalColumns);

	// GetAvailableColumns is allowed to succeed with no command object since
	// some providers have a fixed set of optional columns they can return
	// without being prepared or without a command object.
	if(hr != S_OK)
	{
		// If GetAvailableColumns failed it better be for the proper reason
		CHECK(hr, hrGetAvailableColumns);
		goto CLEANUP;
	}

	// Make sure optional columns came back
	if(!cOriginalColumns)
		goto CLEANUP;

	// Switch on the enum
	switch(eArrangeOption)
	{
		// Return all DBID's
		case ALLDBID:
			
			// Initialize member var's
			fSucceed = FALSE;
			m_cDBID	 = cOriginalColumns;
			m_rgDBID = (DBID *) PROVIDER_ALLOC(sizeof(DBID)*m_cDBID);
			
			// Copy optional DBID's into member var
			if(m_rgDBID)
			{
				memcpy(m_rgDBID,rgOriginalColumns,(size_t)(sizeof(DBID)*m_cDBID));
				fSucceed = TRUE;
			}

			break;

		// Return every other DBID starting with the second element of the array
		case SOMEDBID:

			// Initialize member var's
			fSucceed = FALSE;
			m_cDBID  = (cOriginalColumns / 2) - 1;
			m_rgDBID = (DBID *) PROVIDER_ALLOC(m_cDBID * sizeof(DBID));

			// Copy optional DBID's into member var
			if(m_rgDBID)
			{
				for(givenIndex=index=0; index<m_cDBID; index++)
				{
					memcpy(&(m_rgDBID[index]), &(rgOriginalColumns[givenIndex]), sizeof(DBID));
					givenIndex = givenIndex + 2;
				}
				fSucceed = TRUE;
			}

			break;

		// Return every DBID in reversed order
		case REVERSEDBID:

			// Initialize member var's
			fSucceed = FALSE;
			m_cDBID  = cOriginalColumns;
			m_rgDBID = (DBID *) PROVIDER_ALLOC(m_cDBID * sizeof(DBID));

			// Copy optional DBID's into member var
			if(m_rgDBID)
			{
				for(index=0; index<m_cDBID; index++)
					memcpy(&(m_rgDBID[index]), &(rgOriginalColumns[(m_cDBID-1)-index]), sizeof(DBID));
  
				fSucceed = TRUE;
			}

			break;

		// Return every DBID twice in the list
		case DUPLICATEDBID:

			// Initialize member var's
			fSucceed = FALSE;
			m_cDBID  = cOriginalColumns * 2;
			m_rgDBID = (DBID *) PROVIDER_ALLOC(m_cDBID * sizeof(DBID));

			// Copy optional DBID's into member var
			if(m_rgDBID)
			{
				// First half of array
				for(index=0; index<cOriginalColumns; index++)
					memcpy(&(m_rgDBID[index]), &(rgOriginalColumns[index]), sizeof(DBID));
				
				// Second half of array
				for(index=cOriginalColumns; index<m_cDBID; index++)
					memcpy(&(m_rgDBID[index]), &(rgOriginalColumns[index-cOriginalColumns]), (size_t)sizeof(DBID));

				fSucceed = TRUE;
			}
		
			break;

		// Return invalid DBID's
		case INVALIDDBID:
			
			// Initialize member var's
			fSucceed = FALSE;
			m_cDBID  = cOriginalColumns;
			m_rgDBID = (DBID *) PROVIDER_ALLOC(m_cDBID * sizeof(DBID));

			// Copy optional DBID's into member var
			if(m_rgDBID)
			{
				for(index=0; index<cOriginalColumns; index++)
					memcpy(&(m_rgDBID[index]), &(rgOriginalColumns[index]), sizeof(DBID));

				for(index=0; index<m_cDBID-1; index++)
					m_rgDBID[index++].eKind = 9857;

				fSucceed = TRUE;
			}

			break;

		// Return no DBID's
		case NONEDBID:

			// Initialize member var's
			m_rgDBID = NULL;
			m_cDBID  = 0;
			fSucceed = TRUE;

			break;

		default:
			ASSERT(!"BAD Enum for ArangeOptionalColumns");
			break;
	}

CLEANUP:
	
	// Cleanup memory
	PROVIDER_FREE(rgOriginalColumns);
	return fSucceed;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	FreeOptionalColumns
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::FreeOptionalColumns()
{
	// Clear the count & array of DBID's
	m_cDBID = 0;
	PROVIDER_FREE(m_rgDBID);
	return TRUE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	QIRowset
//
//	Try all rowset Interfaces on the ColumnsRowset
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::QIRowset
(
	IUnknown * pColRowset
)
{
	ULONG		cErrors	   = 0;
	IUnknown*	pInterface = NULL;

	// Check the in Rowset pointer
	if(!pColRowset)
		return FALSE;
	
	// Loop thru the mandatory Rowset Interfaces
	for(ULONG ulIndex=0; ulIndex < NUMELEM(g_rgIIDRowset); ulIndex++)
	{
		// Get the IID_IUnknown Interface off of the IColumnsRowset Object
		if(!VerifyInterface(pColRowset, *g_rgIIDRowset[ulIndex], 
							ROWSET_INTERFACE, (IUnknown **)&pInterface))
			cErrors++;

		SAFE_RELEASE(pInterface);
	}

	// If no errors return TRUE
	return !cErrors;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// VerifyRowset, verify rowset and row order
//
// 1) Verify Maxlength, Precision, and Scale for non-applicable datatypes
// 2) Check ComputeMode
// 3) Order of rows is order of columns in query
//
// ----------------------------------------------
// 1. IColumnsInfo::GetColumnInfo, build binding (from GetAccessorAndBindings)
// 2. IAccessor::CreateAccessor (from GetAccessorAndBindings)
// Loop
//    3. IRowset::GetNextRows
//    4. IRowset::GetData
//    5. IRowset::ReleaseRows
// End Loop
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::VerifyRowset
(
	IUnknown * pIUnknown, 
	IID iid,
	STATEMENTKIND stmt
)
{
	HRESULT			hr				= E_FAIL;	// HRESULT
	BOOL			fSucceed		= FALSE;	// success of function
	ULONG			i				= 0;
	
	HACCESSOR		hAccessor		= NULL;		// handle to accessor
	DBLENGTH		cbRowSize		= 0;		// size of row
	ULONG			iRow			= 0;		// current row
	DBCOUNTITEM		cRowsObtained	= 0;		// total rows obtained from getnextrows
	ULONG			cTotalRows		= 0;		// total row count
	BYTE *			pRow			= NULL;		// row of Data
	HROW *			rghRows			= NULL;		// array of hrows
	IRowset *		pIRowset		= NULL;
	IColumnsInfo *	pIColumnsInfo	= NULL;
	IRowsetInfo *	pIRowsetInfo	= NULL;
	IAccessor *		pIAccessor		= NULL;

	ULONG			iBind			= 0;		// current binding
	DBCOUNTITEM		cBinding		= 0;		// count of bindings
	DBBINDING *		rgBinding		= NULL;		// array of bindings
	DBPROPSTATUS	propStatus		= 0;
	ULONG			cRowErrors		= 0;		// count of errors from VerifyRow
	ULONG			rgRefCounts[30];
	DBROWSTATUS		rgRowStatus[30];
	ULONG			cColRowPropSets = 0;
	DBPROPSET *		pColRowPropSets = NULL;

	DBPROPIDSET rgPropertyIDSets;
	
	rgPropertyIDSets.rgPropertyIDs = NULL;
	rgPropertyIDSets.cPropertyIDs = 0;
	rgPropertyIDSets.guidPropertySet = DBPROPSET_ROWSET;

	// Initialize
	m_fColumnsRowsetHasBookmark = FALSE;
	m_fColumnCacheDeferred		= FALSE;
	m_fColumnMayDefer			= FALSE;

	// Check the IColumnsRowset to see if it has Bookmarks set
	if( IsPropertySet(DBPROPSET_ROWSET, DBPROP_BOOKMARKS, pIUnknown, 0) )
		m_fColumnsRowsetHasBookmark = TRUE;

	// Check the IColumnsRowset to see if the column is CacheDeferred
	if( IsPropertySet(DBPROPSET_ROWSET, DBPROP_CACHEDEFERRED, pIUnknown, 0) )
		m_fColumnCacheDeferred = TRUE;

	// Check the IColumnsRowset to see if the column is Deferred
	if( IsPropertySet(DBPROPSET_ROWSET, DBPROP_DEFERRED, pIUnknown, 0) )
		m_fColumnMayDefer = TRUE;

	// Get the rowset properties for the IColumnsRowset
	TESTC(VerifyInterface(pIUnknown, IID_IRowsetInfo, ROWSET_INTERFACE,
		(IUnknown **)&pIRowsetInfo));

	// Get property info for original rowset
	TESTC_(pIRowsetInfo->GetProperties(1, &rgPropertyIDSets, &cColRowPropSets,
		&pColRowPropSets), S_OK);

	// Get bindings and column info
	if( !CHECK(hr=GetAccessorAndBindings(
		pIUnknown,									// @parm [IN]  Rowset to create Accessor for
		DBACCESSOR_ROWDATA,							// @parm [IN]  Properties of the Accessor
		&hAccessor,									// @parmopt [OUT] Accessor created
		&rgBinding,									// @parmopt [OUT] Array of DBBINDINGS
		&cBinding,									// @parmopt [OUT] Count of bindings
		&cbRowSize,									// @parmopt [OUT] Length of a row, DATA	
		DBPART_VALUE|DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_BOUND,								// @parmopt [IN] Which columns will be used in the bindings
		FORWARD,									// @parmopt [IN]  Order to bind columns in accessor												
		NO_COLS_BY_REF,								// @parmopt [IN]  Which column types to bind by reference
		&m_rgDBCOLUMNINFO,							// @parmopt [OUT] Count of Columns, also count of ColInfo elements
		&m_cDBCOLUMNINFO,
		&m_pStringsBuffer), S_OK) )
		goto CLEANUP;

	// This might be set from the previous test variation
	m_fHasOrdinalZero = FALSE;
	
	// Check the IColumnsInfo on the IColumnsRowset
	CheckIColumnsInfo(pColRowPropSets);

	// Set the member var back  to FALSE
	m_fHasOrdinalZero = FALSE;

	// QI for a IRowset pointer
	if( FAILED(pIUnknown->QueryInterface(IID_IRowset,(void **) &pIRowset)) )
		goto CLEANUP;

	// Validate objects
	if( (!cBinding) || (!m_cDBCOLUMNINFO) || (!rgBinding) || (!m_rgDBCOLUMNINFO) )
		goto CLEANUP;

	// Create space for row of data
	pRow = (BYTE *) PROVIDER_ALLOC(cbRowSize);
	if( !pRow )
		goto CLEANUP;
 
	// Process all the rows, NUMROWS_CHUNK rows at a time
	while(1)
	{
		// Get rows to process 30 is an arbitrary number of rows to fetch
		if( FAILED(hr=pIRowset->GetNextRows(0, 0, 30, &cRowsObtained, &rghRows)) )
			goto CLEANUP;

		// Check to see if it is an Insert Statement 0 rows
		if( stmt == eINSERT && !cRowsObtained )
			fSucceed = TRUE;

		// Verify that we have rows to process
		if( !cRowsObtained )
			goto CLEANUP;

		// Display the number of rows and columns
		PRVTRACE(L"Columns Returned [%d]", m_cDBCOLUMNINFO);
		PRVTRACE(L" and Rows Returned [%d]\n", cRowsObtained);;

		// Loop over rows obtained, getting data for each
		for(iRow=0; iRow<cRowsObtained; iRow++)
		{
			memset(pRow, 0, (size_t)cbRowSize);

			// Get the row
			if( !CHECK(m_hr=pIRowset->GetData(rghRows[iRow], hAccessor, pRow),S_OK) )
				goto CLEANUP;

			// Make sure we got the row
			if( !pRow )
				goto CLEANUP;

 			// Do something with row
			if (!VerifyRow(cTotalRows, rgBinding, cBinding, pRow))
				cRowErrors++;

			cTotalRows++;
		}

		// Release the HRows
		if( !CHECK(m_hr=pIRowset->ReleaseRows(cRowsObtained, rghRows,
												NULL, rgRefCounts, rgRowStatus),S_OK) )
			goto CLEANUP;

		//Set the TRUE if we atleast get 1 row
		fSucceed = TRUE;
	}

CLEANUP:

	// Release objects
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);

	PROVIDER_FREE(m_rgDBCOLUMNINFO);
	PROVIDER_FREE(m_pStringsBuffer);

	PROVIDER_FREE(pRow);
	PROVIDER_FREE(rghRows);
	
	FreeAccessorBindings(cBinding, rgBinding);
	FreeProperties(&cColRowPropSets, &pColRowPropSets);

	// Return result
	return fSucceed && !cRowErrors;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	VerifyColumn
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::VerifyRow
(
	ULONG		iRow,
	DBBINDING *	rgBindings,
	DBCOUNTITEM	cBindings,
	BYTE *		pData
)
{
	ULONG 	iBind			= 0;				// Binding Count
    DATA *	pColumn			= NULL;				// Data Structure
	CCol	col;
	BOOL	fDBTYPEisDecimal= FALSE;
	BOOL	fSucceed		= TRUE;
	BOOL	fHidden			= FALSE;			// Not a hidden column

	// Check to see if the Original Rowset has BOOKMARKS
	if( m_fOriginalRowsetHasBookmark && !iRow)
	{
		// If the original rowset has a bookmark then we expect the first row in the
		// columns rowset to be for the bookmark and have ordinal 0.
		if(COMPARE((ULONG_PTR)VALUE_BINDING(rgBindings[4], pData), 0))
		{
			m_fHasOrdinalZero = TRUE;
			return TRUE;
		}
		odtLog << L"Columns rowset did not return bookmark column from original rowset!\n\n";
	}

	// Param checking
	if( !rgBindings )
		return FALSE;

	// We can't check the value of hidden columns.  The best we can do is compare
	// to IColumnsInfo.
	// Actually, per spec we should be able to do some minimal checking.
	//		Verify extra column count matches expected from DBPROP_HIDDENCOLUMNS.
	//		Must have DBID DBKIND_GUID_PROPID, etc.
	//		Must be or point to DBCOL_SPECIALCOL.
	if (iRow+1 > m_pTable->CountColumnsOnTable() + m_fHasOrdinalZero)
	{
		// We expect this is a hidden column
		WCHAR * pName = L"<error>";

		fHidden = TRUE;

		if (STATUS_BINDING(rgBindings[m_fColumnsRowsetHasBookmark+3], pData) == DBSTATUS_S_ISNULL)
			pName = L"<null>";
		if (STATUS_BINDING(rgBindings[m_fColumnsRowsetHasBookmark+3], pData) == DBSTATUS_S_OK)
			pName = (WCHAR *)&VALUE_BINDING(rgBindings[m_fColumnsRowsetHasBookmark+3], pData);

		// Print the name of the extra column
		odtLog << L"GetColumnsRowset returned extra column: " << pName << L"\n";

		// The extra column ordinal should be greater than the number of columns in the table and should be <= number of 
		// cols in table + hidden columns.
		if (!COMPARE(iRow+1 <=  m_pTable->CountColumnsOnTable() + m_fHasOrdinalZero + m_ulHiddenColumns, TRUE))
		{
			odtLog << L"Extra column was not reflected in DBPROP_HIDDENCOLUMNS.\n";
			fSucceed = FALSE;
		}

		//	Must have DBID DBKIND_GUID_PROPID, etc.

		//	Must be or point to DBCOL_SPECIALCOL, therefore cannot be NULL
		if (!COMPARE(STATUS_BINDING(rgBindings[m_fColumnsRowsetHasBookmark+1], pData), DBSTATUS_S_OK))
		{
			odtLog << L"Status for extra column was not DBSTATUS_S_OK.\n";
			fSucceed = FALSE;
		}
		else
		{
			GUID * pGUID = (GUID *)&VALUE_BINDING(rgBindings[m_fColumnsRowsetHasBookmark+1], pData);
			if (!COMPARE(*pGUID == DBCOL_SPECIALCOL, TRUE))
			{
				odtLog << L"Error: DBCOLUMN_GUID did not match DBCOL_SPECIALCOL.\n";
				fSucceed = FALSE;
			}
		}

		// If DBCOLUMN_KEYCOLUMN is included in the optional columns it must be VARIANT_TRUE
		// Note we don't necessarily know where DBCOLUMN_KEYCOLUMN will be found.
		for (iBind = MANCOL+ m_fColumnsRowsetHasBookmark; iBind < cBindings; iBind++)
		{
			if (CompareDBID( m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_KEYCOLUMN))
			{
				VARIANT_BOOL bKeyCol = (VARIANT_BOOL)VALUE_BINDING(rgBindings[iBind], pData);
				if (!COMPARE(bKeyCol, VARIANT_TRUE))
				{
					odtLog << L"Error: DBCOLUMN_KEYCOLUMN was not VARIANT_TRUE for extra column.\n";
					fSucceed = FALSE;
				}
			}
		}
	}

	// Get Query Col that this row should match
	if(!fHidden && FAILED(m_pTable->GetColInfo(iRow + 1 - m_fHasOrdinalZero, col)) )
	{
		odtLog << L"VerifyRowset:Couldn't get matching column\n";
		return FALSE;
	}

	// Check each column we bound
	for(iBind=0; iBind < cBindings; iBind++)
	{
		// Grab column
		PRVTRACE(L"Row[%lu],Ordinal[%lu]:%s", iRow, m_rgDBCOLUMNINFO[iBind].iOrdinal, m_rgDBCOLUMNINFO[iBind].pwszName);
		pColumn = (DATA *) (pData + rgBindings[iBind].obStatus);
		LPWSTR pwszColName = NULL;

		// Mandatory columns
		if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_IDNAME) )
		{
			if (fHidden)
			{
				if( ( col.GetColID()->eKind == DBKIND_NAME ) ||
					( col.GetColID()->eKind == DBKIND_GUID_NAME ) ||
					( col.GetColID()->eKind == DBKIND_PGUID_NAME ) )
				{
					fSucceed &= Compare_DBCOLUMN_NAME(col, pColumn);
					fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_IDNAME");
				}
				else
					fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_IDNAME", ALLOW_NULLS);
			}
		}
		else if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_GUID) )
		{
			if( ( col.GetColID()->eKind == DBKIND_NAME ) ||
				( col.GetColID()->eKind == DBKIND_PROPID ))
				fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_GUID", ALLOW_NULLS);
			else
				fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_GUID");
		}
		else if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_PROPID) )
		{
			if( ( col.GetColID()->eKind == DBKIND_PROPID ) ||
				( col.GetColID()->eKind == DBKIND_GUID_PROPID ) ||
				( col.GetColID()->eKind == DBKIND_PGUID_PROPID ) )
				fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_PROPID");
			else
				fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_PROPID", ALLOW_NULLS);
		}
		else if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_NAME) )
		{
			fSucceed &= Compare_DBCOLUMN_NAME(col, pColumn);
		}
		else if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_NUMBER) )
			fSucceed &= Compare_DBCOLUMN_NUMBER(col, iRow+!m_fHasOrdinalZero, pColumn);
		else if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_TYPE) )
			fSucceed &= Compare_DBCOLUMN_TYPE(col, pColumn);
		else if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_TYPEINFO) )
			fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_TYPEINFO", ALLOW_NULLS);
		else if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_COLUMNSIZE) )
			fSucceed &= Compare_DBCOLUMN_COLUMNSIZE(col, pColumn);
		else if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_PRECISION) )
			fSucceed &= Compare_DBCOLUMN_PRECISION(col, pColumn);
		else if( CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_SCALE) )
			fSucceed &= Compare_DBCOLUMN_SCALE(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_FLAGS))
		{
			fSucceed &= Compare_DBCOLUMNFLAGS_CACHEDEFERRED(col.GetColName(), *((LONG *)pColumn->bValue));
			fSucceed &= Compare_DBCOLUMNFLAGS_ISBOOKMARK((iRow + !m_fHasOrdinalZero), col.GetColName(), *((LONG *)pColumn->bValue), m_fOriginalRowsetHasBookmark);
		//Compare_DBCOLUMNFLAGS_ISFIXEDLENGTH(m_rgDBCOLUMNINFO[ulIndex1]);
		//Compare_DBCOLUMNFLAGS_ISLONG(m_rgDBCOLUMNINFO[ulIndex1]);
		//Compare_DBCOLUMNFLAGS_ISNULLABLE(s_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndex1]);
		//Compare_DBCOLUMNFLAGS_ISROWID(m_rgDBCOLUMNINFO[ulIndex1]);
		//Compare_DBCOLUMNFLAGS_ISROWVER(m_rgDBCOLUMNINFO[ulIndex1]);
		//Compare_DBCOLUMNFLAGS_MAYBENULLABLE(s_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndex1]);
		//fSucceed &= Compare_DBCOLUMNFLAGS_MAYDEFER(col.GetColName(), *((LONG *)pColumn->bValue), iRow+1,m_pOriginalColumnsInfo, m_pOriginalPropertySets);
		//Compare_DBCOLUMNFLAGS_WRITE(m_rgDBCOLUMNINFO[ulIndex1]);
		//Compare_DBCOLUMNFLAGS_WRITEUNKNOWN(m_rgDBCOLUMNINFO[ulIndex1]);
		}
		// optional columns
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_BASECATALOGNAME))
			fSucceed &= Compare_DBCOLUMN_BASECATALOGNAME(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_BASECOLUMNNAME))
		{
			fSucceed &= Compare_DBCOLUMN_NAME(col, pColumn);
		}
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_BASESCHEMANAME))
			fSucceed &= Compare_DBCOLUMN_BASESCHEMANAME(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_BASETABLENAME))
			fSucceed &= Compare_DBCOLUMN_TABLENAME(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_CLSID))
			fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_CLSID", ALLOW_NULLS);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_COLLATINGSEQUENCE))
			// Spec does not indicate COLLATINGSEQUENCE can contain a NULL, but since it only applies to char cols
			// it seems reasonable to allow NULLs, and some providers do return NULLs. Additionally, the provider
			// may not know the collating sequence of the underlying DBMS (e.g. it can be changed on Oracle during
			// server boot time).
			fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_COLLATINGSEQUENCE", ALLOW_NULLS);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_COMPUTEMODE))
			fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_COMPUTEMODE");
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_DATETIMEPRECISION))
			fSucceed &= Compare_DBCOLUMN_DATETIMEPRECISION(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_DEFAULTVALUE))
			fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_DEFAULTVALUE", ALLOW_NULLS);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_DOMAINCATALOG))
			fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_DOMAINCATALOG", ALLOW_NULLS);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_DOMAINSCHEMA))
			fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_DOMAINSCHEMA", ALLOW_NULLS);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_DOMAINNAME))
			fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_DOMAINNAME", ALLOW_NULLS);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid, DBCOLUMN_HASDEFAULT))
			fSucceed &= Compare_BOOL(col, pColumn, L"DBCOLUMN_HASDEFAULT", ALLOW_NULLS);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid,DBCOLUMN_ISAUTOINCREMENT))
			fSucceed &= Compare_DBCOLUMN_ISAUTOINCREMENT(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid,DBCOLUMN_ISCASESENSITIVE))
			fSucceed &= Compare_DBCOLUMN_ISCASESENSITIVE(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid,DBCOLUMN_ISSEARCHABLE))
			fSucceed &= Compare_DBCOLUMN_ISSEARCHABLE(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid,DBCOLUMN_ISUNIQUE))
			fSucceed &= Compare_DBCOLUMN_ISUNIQUE(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid,DBCOLUMN_MAYSORT))
			fSucceed &= Compare_BOOL(col, pColumn, L"DBCOLUMN_MAYSORT");
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid,DBCOLUMN_OCTETLENGTH))
			fSucceed &= Compare_DBCOLUMN_OCTETLENGTH(col, pColumn);
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid,DBCOLUMN_KEYCOLUMN))
			fSucceed &= Compare_BOOL(col, pColumn, L"DBCOLUMN_KEYCOLUMN");
		else if(CompareDBID(m_rgDBCOLUMNINFO[iBind].columnid,DBCOLUMN_BASETABLEVERSION))
			fSucceed &= Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_BASETABLEVERSION");
		else if( (m_rgDBCOLUMNINFO[iBind].columnid.eKind == DBKIND_GUID_PROPID ) && 
				 (m_rgDBCOLUMNINFO[iBind].columnid.uGuid.guid == DBCOL_SPECIALCOL) &&
				 (m_rgDBCOLUMNINFO[iBind].columnid.uName.ulPropid == 2) )
		{
			if( !COMPARE(iRow+1, *((ULONG *)pColumn->bValue)) )
			{
				fSucceed = FALSE;
				odtLog  << L"Bookmark should be " <<iRow+1 
						<<L" returned " <<*((ULONG *)pColumn->bValue) << ENDL;
			}
		}
	}
   
	return fSucceed;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CheckStatusInfo
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::CheckStatusInfo(DATA * pColumn)
{
	// Check to see if the column has info
	if( pColumn->sStatus == DBSTATUS_S_OK )
		return TRUE;
	else
	{
		PRVTRACE(L"The STATUS of the COLUMN is ");
		return RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_StatusInfo
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_StatusInfo(CCol col, DBSTATUS dbStatus, DATA * pColumn, WCHAR * wszDsc,
	enum STATUS_ENUM eStatus)
{
	// Check to see if the column has info
	if(COMPARE((DBSTATUS)pColumn->sStatus == dbStatus ||
		eStatus == ALLOW_NULLS && (DBSTATUS)pColumn->sStatus == DBSTATUS_S_ISNULL, TRUE))
		return TRUE;
	else
	{
		odtLog << L"[ " << col.GetColName() << L" ] "
				  L" The Status for " << wszDsc << " is unexpected: " << (DBSTATUS)pColumn->sStatus
				  << ENDL;
		return RowsetBindingStatus((DBSTATUSENUM)pColumn->sStatus);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_NAME
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_NAME(CCol col, DATA * pColumn)
{
	BOOL fSuccess = FALSE;

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_NAME", ALLOW_NULLS) )
	{
		// See if both values are valid. DBCOLUMN_NAME and DBCOLUMN_BASECOLUMNNAME are both
		// allowed to be NULL.
		if (pColumn->sStatus == DBSTATUS_S_ISNULL)
			fSuccess = TRUE;
		else if(pColumn->sStatus == DBSTATUS_S_OK && ((WCHAR *)pColumn->bValue) && col.GetColName() )
		{
			if( !(wcscmp(col.GetColName(), (WCHAR*)pColumn->bValue)) )
				fSuccess = TRUE;
			else 
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The column name should be "<< col.GetColName()
					  << L", returned "<< (WCHAR*)pColumn->bValue<< ENDL;
		}
	}

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_CATALOGNAME
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_BASECATALOGNAME(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;

	// If the provider supports catalogs then we should have a success status
	if (ProviderSupports(CATALOGNAME) &&
		!Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_BASECATALOGNAME"))
		return FALSE;

	// If the status is OK
	if((DBSTATUS)pColumn->sStatus == DBSTATUS_S_OK)
	{
		if (COMPARE(m_pwszCatalogName != NULL, TRUE))
		{
			if(COMPARE(!wcscmp(m_pwszCatalogName, (WCHAR*)pColumn->bValue), TRUE))
			{
				fSuccess = TRUE;
			}
			else
			{
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The Catalog name should be "<< m_pwszCatalogName
					  << L", returned "<< (WCHAR*)pColumn->bValue<< ENDL;
			}
		}
	}
	else if (COMPARE((DBSTATUS)pColumn->sStatus, DBSTATUS_S_ISNULL))
		fSuccess = TRUE;

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_NUMBER
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_NUMBER(CCol col, ULONG iRow, DATA * pColumn)
{
	BOOL fRowNumber = FALSE;
	BOOL fColumnNum = FALSE;

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_NUMBER") )
	{
		// Compare row number to column number, should be the same
		if( *((ULONG*)pColumn->bValue) == iRow)
			fRowNumber = TRUE;
		else
			odtLog<< L"[ " << col.GetColName() << L" ] "
				  << L"ERROR The column number should be "<< iRow
				  << L", returned "<< *((ULONG*)pColumn->bValue)<< ENDL;

		// Compare to base table column number
		if( *((ULONG*)pColumn->bValue) == col.GetColNum() )
			fColumnNum = TRUE;
		else
			odtLog<< L"[ " << col.GetColName() << L" ] "
				  << L"ERROR The column number should be "<< col.GetColNum()
				  << L", returned "<< *((ULONG*)pColumn->bValue)<< ENDL;
	}

	return (fRowNumber&fColumnNum);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_SCHEMANAME
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_BASESCHEMANAME(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;

	// If the provider supports catalogs then we should have a success status
	if (ProviderSupports(SCHEMANAME) &&
		!Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_BASESCHEMANAME"))
		return FALSE;

	// Check the Status bits
	if((DBSTATUS)pColumn->sStatus == DBSTATUS_S_OK)
	{
		// This should test the SCHEMANAME
		if (COMPARE(m_pwszSchemaName != NULL, TRUE))
		{
			if(COMPARE(!wcscmp(m_pwszSchemaName, (WCHAR*)pColumn->bValue), TRUE))
			{
				fSuccess = TRUE;
			}
			else
			{
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The Schema name should be " << m_pwszSchemaName
					  << L", returned "<< (WCHAR*)pColumn->bValue<< ENDL;
			}
		}
	}
	else if (COMPARE((DBSTATUS)pColumn->sStatus, DBSTATUS_S_ISNULL))
		fSuccess = TRUE;

	return fSuccess;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_TYPE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_TYPE(CCol col, DATA * pColumn)
{
	BOOL fSuccess = FALSE;

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_TYPE") )
	{
		// See if both values a valid
		if( *((USHORT*)pColumn->bValue) == col.GetProviderType() )
			fSuccess = TRUE;
		else 
			odtLog<< L"[ " << col.GetColName() << L" ] "
				  << L"ERROR The column type should be "<< col.GetProviderType()
				  << L", returned "<< *((USHORT*)pColumn->bValue)<< ENDL;
	}

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_COLUMNSIZE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_COLUMNSIZE(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;
	DBLENGTH ulHolder = 0;

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_COLUMNSIZE") )
	{
		// Check to see if it is variable length
		if(!IsFixedLength(col.GetProviderType()) )
		{
			ulHolder = col.GetColumnSize();

			// Adjust for the VARNUMERIC
			if (col.GetProviderType() == DBTYPE_VARNUMERIC)
				ulHolder = 20;

			// Check the values
			if( *((ULONG*)pColumn->bValue) == ulHolder )
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The column size for the data type should be "<< ulHolder
					  << L", returned "<< *((ULONG*)pColumn->bValue)<< ENDL;
		}
		else
		{
			// Check the values
			if( *((ULONG*)pColumn->bValue) == (ULONG)GetDBTypeSize(col.GetProviderType()) )
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The column size for fixed type should be "<< GetDBTypeSize(col.GetProviderType())
					  << L", returned "<< *((ULONG*)pColumn->bValue)<< ENDL;
		}
	}

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_PRECISION
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_PRECISION(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;
	ULONG	ulHolder = 0;

	// Check to see if it's a numeric column
	if( IsNumericType(col.GetProviderType()) || col.GetProviderType() == DBTYPE_DBTIMESTAMP)
	{
		// Check the Status bits
		if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_PRECISION") )
		{
			// This should cover NUMERIC types
			if( (IsNumericType(col.GetProviderType()) && 
				 !col.GetCreateParams() && (*(USHORT*)pColumn->bValue) == 0) ||
				((*(USHORT*)pColumn->bValue) == col.GetPrecision()) )
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The column precision for the data type should be "<< col.GetPrecision()
					  << L", returned "<< *((USHORT*)pColumn->bValue)<< ENDL;
		}
	}
	else
		fSuccess = Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_PRECISION", ALLOW_NULLS);

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_SCALE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_SCALE(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;
	ULONG	ulHolder = 0;

	// This should cover NUMERIC, VARNUMERIC, DECIMAL, and TIMESTAMP
	if( IsScaleUsed(col.GetProviderType()) )
	{
		// Check the Status bits
		if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_SCALE") )
		{
			// CCol struct has wrong value for scale By Design, so compare to ColumnsInfo
			if( (*(SHORT*)pColumn->bValue) == 
				m_pOriginalColumnsInfo[col.GetColNum()-m_pOriginalColumnsInfo[0].iOrdinal].bScale)
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The column scale for NUMERIC or DECIMAL should be "<< col.GetScale()
					  << L", returned "<< *((SHORT*)pColumn->bValue)<< ENDL;
		}
	}
	else
		fSuccess = Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_SCALE", ALLOW_NULLS);

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_TABLENAME
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_TABLENAME(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;

	// Provider-specific hack for particular ODBC drivers (Access, Sql Server, Oracle).  Fail
	// them if they return NULL BASETABLENAME.
	if (m_pwszDriverName && (!wcscmp(m_pwszDriverName, L"odbcjt32.dll") ||
		!wcscmp(m_pwszDriverName, L"SQLSRV32.DLL")))
	{
		if (!COMPARE(pColumn->sStatus, DBSTATUS_S_OK))
		{
			odtLog << L"The Status for DBCOLUMN_BASETABLENAME is invalid: " << pColumn->sStatus << ENDL;
			return FALSE;
		}
	}

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_BASETABLENAME", ALLOW_NULLS) )
	{
		// This should test the TABLENAME
		if (pColumn->sStatus == DBSTATUS_S_ISNULL)
			fSuccess = TRUE;
		else if( m_pTable->GetTableName() && *(WCHAR*)pColumn->bValue )
		{
			if( !(wcscmp((WCHAR*)pColumn->bValue, m_pwszBareTableName)) )
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The base table name should be "<< m_pwszBareTableName
					  << L", returned "<< (WCHAR*)pColumn->bValue<< ENDL;
		}
	}

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_DATETIMEPRECISION
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_DATETIMEPRECISION(CCol col, DATA * pColumn)
{
	BOOL	fSuccess   = FALSE;
	ULONG	ulTmpDTVal = 0;

	// This should test the DATETIMEPRECISION
	if( If_DATETIME(col.GetProviderType()))
	{
		// Figure out the Scale
		if( col.GetPrecision() > 20 )
			ulTmpDTVal = (col.GetPrecision()%20);

		if( (*(ULONG*)pColumn->bValue) == ulTmpDTVal )
			fSuccess = TRUE;
		else
			odtLog<< L"[ " << col.GetColName() << L" ] "
				  << L"ERROR The datetime precision should be "<< col.GetScale()
				  << L", returned "<< (*(ULONG*)pColumn->bValue) << ENDL;
	}
	else
	{
		if( Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_DATETIMEPRECISION", ALLOW_NULLS) )
			fSuccess = TRUE;
		else
			odtLog<< L"[ " << col.GetColName() << L" ] "
				  << L"ERROR The datetime precision should have a status of NULL" << ENDL;
	}

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_ISAUTOINCREMENT
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_ISAUTOINCREMENT(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_ISAUTOINCREMENT") )
	{
		// This should test if the column is AutoIncrement
		if( col.GetAutoInc() )
		{
			if( (*(VARIANT_BOOL*)pColumn->bValue) == VARIANT_TRUE )
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The column autoinc should be "<< col.GetAutoInc()
					  << L", returned "<< (*(VARIANT_BOOL*)pColumn->bValue) << ENDL;
		}
		else
		{
			if( (*(VARIANT_BOOL*)pColumn->bValue) == VARIANT_FALSE )
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The column autoinc should be "<< col.GetAutoInc()
					  << L", returned "<< (*(VARIANT_BOOL*)pColumn->bValue) << ENDL;
		}
	}

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_ISCASESENSITIVE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_ISCASESENSITIVE(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_ISCASESENSITIVE") )
	{
		// Check to see if it is character and case sensitive
		// The case sensitivity value was derived from the Provider Types rowset.
		if(col.GetCaseSensitive())
		{
			// This should test if the column is CaseSensitive
			if( (*(VARIANT_BOOL*)pColumn->bValue) == VARIANT_TRUE )
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The column case sensitivity should be VARIANT_TRUE"
					  << L", returned "<< (*(VARIANT_BOOL*)pColumn->bValue) << ENDL;
		}
		else
		{
			// This should test if the column is not CaseSensitive
			if( (*(VARIANT_BOOL*)pColumn->bValue) == VARIANT_FALSE )
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The column case sensitivity should be VARIANT_FALSE"
					  << L", returned "<< (*(VARIANT_BOOL*)pColumn->bValue) << ENDL;
		}
	}

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_BOOL
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_BOOL(CCol col, DATA * pColumn, WCHAR * wszDsc, STATUS_ENUM eStatus)
{
	BOOL	fSuccess = FALSE;

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, wszDsc, eStatus))
	{
		// Check to see if it's VARIANT_TRUE or VARIANT_FALSE
		if( (*(VARIANT_BOOL*)pColumn->bValue) == VARIANT_TRUE ||
			(*(VARIANT_BOOL*)pColumn->bValue) == VARIANT_FALSE)
			fSuccess = TRUE;
		else
			odtLog<< L"[ " << col.GetColName() << L" ] "
				  << L"ERROR The "<< wszDsc << L" column value should be VARIANT_TRUE or VARIANT_FALSE"
				  << L", returned "<< (*(VARIANT_BOOL*)pColumn->bValue) << ENDL;
	}

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_ISSEARCHABLE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_ISSEARCHABLE(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_ISSEARCHABLE") )
	{
		// This should test the ISSEARCHABLE
		if( (col.GetSearchable() == (*(ULONG*)pColumn->bValue)) )
			fSuccess = TRUE;
		else
			odtLog<< L"[ " << col.GetColName() << L" ] "
				  << L"ERROR The column searchablity should be "<< col.GetSearchable()
				  << L", returned "<< (*(ULONG*)pColumn->bValue) << ENDL;
	}

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_ISUNIQUE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_ISUNIQUE(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;

	// Check the Status bits
	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_ISUNIQUE") )
		fSuccess = TRUE;

	return fSuccess;

//	if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_ISUNIQUE") )
//	{
//		// This should test if the column is AutoIncrement
//		if( (*(VARIANT_BOOL*)pColumn->bValue) == VARIANT_FALSE )
//			fSuccess = TRUE;
//		else
//			odtLog<< L"[ " << col.GetColName() << L" ] "
//				  << L"ERROR The column unique should be FALSE"
//				  << L", returned "<< (*(VARIANT_BOOL*)pColumn->bValue) << ENDL;
//	}
//	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Compare_DBCOLUMN_OCTETLENGTH
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::Compare_DBCOLUMN_OCTETLENGTH(CCol col, DATA * pColumn)
{
	BOOL	fSuccess = FALSE;
	DBLENGTH ulHolder = 0;

	// Check to see if it is variable length
	if( If_CHAR_Or_BYTE(col.GetProviderType()) )
	{
		// Check the Status bits
		if( Compare_StatusInfo(col, DBSTATUS_S_OK, pColumn, L"DBCOLUMN_OCTETLENGTH") )
		{
			ulHolder = col.GetColumnSize();

			// Check the values
			if( col.GetProviderType() == DBTYPE_WSTR )
				ulHolder = (ulHolder * sizeof(WCHAR));

			if( *((ULONG*)pColumn->bValue) == ulHolder )
				fSuccess = TRUE;
			else
				odtLog<< L"[ " << col.GetColName() << L" ] "
					  << L"ERROR The Octet length for the data type should be "<< ulHolder
					  << L", returned "<< *((ULONG*)pColumn->bValue)<< ENDL;
		}
	}
	else
		fSuccess = Compare_StatusInfo(col, DBSTATUS_S_ISNULL, pColumn, L"DBCOLUMN_OCTETLENGTH", ALLOW_NULLS);


	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CheckIColumnsInfo
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::CheckIColumnsInfo(DBPROPSET * pColRowPropSets)
{
	// Initialize
	DBORDINAL ulIndex  = 0;
	ULONG ulIndexColInfo = 0;
	ULONG ulcnt    = 0;


	// Compare the number of columns asked for is the same as returned
	if( m_cDBID ) 
		COMPARE(m_cDBCOLUMNINFO-MANCOL, m_cDBID);

	// Figure out the index value for the lookup table
	for(ulIndexColInfo = 0; ulIndexColInfo < m_cDBCOLUMNINFO; ulIndexColInfo++)
	{
		// Check if Mandatory or figure index
		if( m_rgDBCOLUMNINFO[ulIndexColInfo].iOrdinal <= MANCOL )
			ulIndex = m_rgDBCOLUMNINFO[ulIndexColInfo].iOrdinal;
		else
		{
			// Loop thru the static structure
			for(ulcnt = 0; ulcnt < (sizeof(g_rgColRowInfo)/sizeof(COLROWINFO)); ulcnt++)
			{
				if( !memcmp(g_rgColRowInfo[ulcnt].columnid, &(m_rgDBCOLUMNINFO[ulIndexColInfo].columnid), sizeof(DBID)) )
				{
					ulIndex = ulcnt;
					break;
				}
			}

			// If it did not break, it could not match the DBID
			if( ulcnt == (sizeof(g_rgColRowInfo)/sizeof(COLROWINFO)) )
			{
				PRVTRACE(L"ERROR The column is a user defined column, please validate info.\n");
				PRVTRACE(L"iOrdinal = %d" ,m_rgDBCOLUMNINFO[ulIndexColInfo].iOrdinal);
				PRVTRACE(L", pwszName = %s", m_rgDBCOLUMNINFO[ulIndexColInfo].pwszName);
				PRVTRACE(L", wType = %d\n", m_rgDBCOLUMNINFO[ulIndexColInfo].wType);
				PRVTRACE(L"ulColumnSize = %d", m_rgDBCOLUMNINFO[ulIndexColInfo].ulColumnSize);
				PRVTRACE(L", bPrecision = %d", m_rgDBCOLUMNINFO[ulIndexColInfo].bPrecision);
				PRVTRACE(L", bScale = %d", m_rgDBCOLUMNINFO[ulIndexColInfo].bScale);
				PRVTRACE(L", dwFlags = %d", m_rgDBCOLUMNINFO[ulIndexColInfo].dwFlags);
				continue;
			}
		}

		// Check all of the IColumnsInfo values
		Compare_pwszName(g_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_pTypeInfo(m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_iOrdinal((m_fColumnsRowsetHasBookmark) ? ulIndexColInfo : ulIndexColInfo+1, m_rgDBCOLUMNINFO[ulIndexColInfo]);
		AdjustStringLengths(&g_rgColRowInfo[ulIndex]);
		Compare_ulColumnSize(g_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_dwType(g_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_bPrecision(g_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_bScale(g_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_Columnid(g_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndexColInfo]);

		Compare_DBCOLUMNFLAGS_CACHEDEFERRED(m_rgDBCOLUMNINFO[ulIndexColInfo].pwszName, m_rgDBCOLUMNINFO[ulIndexColInfo].dwFlags);
		Compare_DBCOLUMNFLAGS_ISBOOKMARK(m_rgDBCOLUMNINFO[ulIndexColInfo].iOrdinal, m_rgDBCOLUMNINFO[ulIndexColInfo].pwszName, m_rgDBCOLUMNINFO[ulIndexColInfo].dwFlags, m_fColumnsRowsetHasBookmark);
		Compare_DBCOLUMNFLAGS_ISFIXEDLENGTH(m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_DBCOLUMNFLAGS_ISLONG(m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_DBCOLUMNFLAGS_ISNULLABLE(g_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_DBCOLUMNFLAGS_ISROWID(m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_DBCOLUMNFLAGS_ISROWVER(m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_DBCOLUMNFLAGS_MAYBENULLABLE(g_rgColRowInfo[ulIndex], m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_DBCOLUMNFLAGS_MAYDEFER(m_rgDBCOLUMNINFO[ulIndexColInfo].pwszName,
			m_rgDBCOLUMNINFO[ulIndexColInfo].dwFlags,
			m_rgDBCOLUMNINFO[ulIndexColInfo].iOrdinal-m_rgDBCOLUMNINFO[0].iOrdinal+1, 
			m_rgDBCOLUMNINFO, pColRowPropSets);
		Compare_DBCOLUMNFLAGS_WRITE(m_rgDBCOLUMNINFO[ulIndexColInfo]);
		Compare_DBCOLUMNFLAGS_WRITEUNKNOWN(m_rgDBCOLUMNINFO[ulIndexColInfo]);
	}

	return TRUE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.pwszName to the 
// column name in s_rgColRowInfo.pwszName
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_pwszName
(
	COLROWINFO s_rgColRowInfo,
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Check the string to see if they are valid
	if( (!s_rgColRowInfo.pwszName) || (!dbColumnInfo.pwszName) )
		return fSuccess;

	// Compare the 2 strings
	if( wcscmp(s_rgColRowInfo.pwszName, dbColumnInfo.pwszName) == 0 )
		fSuccess = TRUE;
	else 
		odtLog<< L"[ " << dbColumnInfo.pwszName << L" ] "
			  << L"ERROR The column name should be "<< s_rgColRowInfo.pwszName
			  << L", returned "<< dbColumnInfo.pwszName<< ENDL;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.pTypeInfo to NULL.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_pTypeInfo
(
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Check the memory to make sure it is NULL
	if( !dbColumnInfo.pTypeInfo )
		fSuccess = TRUE;
	else 
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The column is reserved it should be NULL, returned valid pointer"<< ENDL;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.iOrdinal to the column Ordinal.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_iOrdinal
(
	ULONG ulOrdinal,
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Compare the ordinals of the columns returned
	if( ulOrdinal == dbColumnInfo.iOrdinal )
		fSuccess = TRUE;
	else 
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The Ordinal expected " << ulOrdinal
			   << L", returned "<< dbColumnInfo.iOrdinal << ENDL;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.ulColumnSize
// to the column size.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_ulColumnSize
(
	COLROWINFO s_rgColRowInfo,
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Campare the column size of the columns returned
	if( s_rgColRowInfo.ulColumnSize == dbColumnInfo.ulColumnSize )
		fSuccess = TRUE;
	else 
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The ColumnSize should be " << s_rgColRowInfo.ulColumnSize
			   << L", returned "<< dbColumnInfo.ulColumnSize << ENDL;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.wType to the 
// columns DBTYPE.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_dwType
(
	COLROWINFO s_rgColRowInfo,
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Campare the DBTYPE of the columns returned
	if( s_rgColRowInfo.dbtype == dbColumnInfo.wType )
		fSuccess = TRUE;
	else 
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The DBTYPE should be " << s_rgColRowInfo.dbtype
			   << L", returned " << dbColumnInfo.wType << ENDL;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.bPrecision to the 
// column precision.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_bPrecision
(
	COLROWINFO s_rgColRowInfo,
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Campare the precision of the columns returned
	if( s_rgColRowInfo.bPrecision == dbColumnInfo.bPrecision )
		fSuccess = TRUE;
	else 
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The precision should be " << s_rgColRowInfo.bPrecision
			   << L", returned " << dbColumnInfo.bPrecision << ENDL;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.bScale to the 
// column scale.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_bScale
(
	COLROWINFO s_rgColRowInfo,
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Campare the scale of the columns returned
	if( s_rgColRowInfo.bScale == dbColumnInfo.bScale )
		fSuccess = TRUE;
	else 
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The scale should be " << s_rgColRowInfo.bScale
			   << L", returned " << dbColumnInfo.bScale << ENDL;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.bScale to the 
// column scale.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_Columnid
(
	COLROWINFO s_rgColRowInfo,
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Campare the columnid of the columns returned
	if( dbColumnInfo.iOrdinal == 0 )
	{
		if( (dbColumnInfo.columnid.eKind == DBKIND_GUID_PROPID ) && 
			(dbColumnInfo.columnid.uGuid.guid == DBCOL_SPECIALCOL) &&
			(dbColumnInfo.columnid.uName.ulPropid == 2) )
			fSuccess = TRUE;
		else 
			odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
				   << L"ERROR The columnid is not correct." << ENDL;
	}
	else
	{
		// NON-BOOKMARK columns
		if( !memcmp(s_rgColRowInfo.columnid, &(dbColumnInfo.columnid), sizeof(DBID)) )
			fSuccess = TRUE;
		else 
			odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
				   << L"ERROR The columnid is not correct." << ENDL;
	}

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_CACHEDEFERRED.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_CACHEDEFERRED
(
	WCHAR * pwszName,
	LONG dwFlags
)
{
	BOOL fSuccess  = FALSE;

	// If the column is CacheDeferred
	if( m_fColumnCacheDeferred )
	{
		if( dwFlags & DBCOLUMNFLAGS_CACHEDEFERRED )
			fSuccess = TRUE;
		else
			odtLog << L"[ " << pwszName << L" ] "
				   << L"ERROR The column is Cache Deferred, DBCOLUMNFLAGS_CACHEDEFERRED is not present."<< ENDL;
	}
	else
	{
		if( dwFlags & DBCOLUMNFLAGS_CACHEDEFERRED )
			odtLog << L"[ " << pwszName << L" ] "
				   << L"ERROR The Cache Deferred Property was not set, DBCOLUMNFLAGS_CACHEDEFERRED is present."<< ENDL;
		else
			fSuccess = TRUE;
	}
		
	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_ISBOOKMARK to 
// to the iOrdinal of zero
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_ISBOOKMARK
(
	DBORDINAL iOrdinal,
	WCHAR * pwszName,
	LONG	dwFlags,
	BOOL	fExpBookmarks 
)
{
	BOOL fOrdinal	= FALSE;
	BOOL fProperty	= FALSE;

	// If the Ordinal is 0, then it is the bookmark column
	if( iOrdinal == 0 )
	{
		m_fHasOrdinalZero = TRUE;
		
		// Check the Flag from IColumnsInfo
		if( dwFlags & DBCOLUMNFLAGS_ISBOOKMARK )
			fOrdinal = TRUE;
		else
			odtLog << L"[ " << pwszName << L" ] "
				   << L"ERROR This is ordinal 0, DBCOLUMNFLAGS_ISBOOKMARK is not present."<< ENDL;
	}
	else
	{
		if( dwFlags & DBCOLUMNFLAGS_ISBOOKMARK )
			odtLog << L"[ " << pwszName << L" ] "
				   << L"ERROR This not ordinal 0, DBCOLUMNFLAGS_ISBOOKMARK is present."<< ENDL;
		else
			fOrdinal = TRUE;
	}

	// Check the Property off of IRowsetInfo
	if( m_fHasOrdinalZero )
	{
		if( fExpBookmarks )
			fProperty = TRUE;
		else
			odtLog << L"[ " << pwszName << L" ] "
				   << L"ERROR IRowsetInfo expected bookmark, DBPROP_BOOKMARKS is not VARIANT_TRUE."<< ENDL;
	}
	else
	{
		if( fExpBookmarks )
			odtLog << L"[ " << pwszName << L" ] "
				   << L"ERROR IRowsetInfo expected bookmark, DBPROP_BOOKMARKS is not VARIANT_FALSE."<< ENDL;
		else
			fProperty = TRUE;
	}

	return (fOrdinal&fProperty);
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_ISFIXEDLENGTH.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_ISFIXEDLENGTH
(
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// All NON WSTR Columns are Fixed Length
	if( dbColumnInfo.wType == DBTYPE_WSTR )
	{
		if( dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH )
			odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
				   << L"ERROR The column is not a fixed length type, DBCOLUMNFLAGS_ISFIXEDLENGTH is present."<< ENDL;
		else
			fSuccess = TRUE;
	}
	else
	{
		if( dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH )
			fSuccess = TRUE;
		else
			odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
				   << L"ERROR The column is a fixed length type, DBCOLUMNFLAGS_ISFIXEDLENGTH is not present."<< ENDL;
	}

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_ISLONG.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_ISLONG
(
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess  = FALSE;

	// All IColumnsRowset Columns are long columns
	if( dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISLONG )
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The column is not a long type, DBCOLUMNFLAGS_ISLONG is present."<< ENDL;
	else
		fSuccess = TRUE;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_ISNULLABLE.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_ISNULLABLE
(
	COLROWINFO s_rgColRowInfo,
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Check the Status Flag to see if it is NULLABLE (SCHEMA's are READONLY)
	if( dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISNULLABLE )
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The column is not Nullable, DBCOLUMNFLAGS_ISNULLABLE is present."<< ENDL;
	else 
		fSuccess = TRUE;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_ISROWID.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_ISROWID
(
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess  = FALSE;

	// All IColumnsRowset Columns are plain columns
	if( dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISROWID )
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The column is not a Row ID column, DBCOLUMNFLAGS_ISROWID is present."<< ENDL;
	else
		fSuccess = TRUE;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_ISROWVER.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_ISROWVER
(
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess  = FALSE;

	// All IColumnsRowset Columns are plain columns
	if( dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISROWVER )
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The column is not a RowVer column, DBCOLUMNFLAGS_ISROWVER is present."<< ENDL;
	else
		fSuccess = TRUE;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_MAYBENULL.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_MAYBENULLABLE
(
	COLROWINFO s_rgColRowInfo,
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess = FALSE;

	// Check the Static Flag to see if it is NULLABLE
	if( s_rgColRowInfo.dwFlags & DBCOLUMNFLAGS_ISNULLABLE )
	{
		if(COMPARE(!!(dbColumnInfo.dwFlags & DBCOLUMNFLAGS_MAYBENULL), TRUE))
			fSuccess = TRUE;
		else 
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The column may be NULL, DBCOLUMNFLAGS_MAYBENULL is not present."<< ENDL;
	}
	else
	{
		if(COMPARE(!!(dbColumnInfo.dwFlags & DBCOLUMNFLAGS_MAYBENULL), FALSE))
			fSuccess = TRUE;
		else
			odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
				   << L"ERROR The column is not Nullable, DBCOLUMNFLAGS_MAYBENULL is present."<< ENDL;
	}
	
	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_MAYDEFER.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_MAYDEFER
(
	WCHAR * pwszName,
	LONG dwFlags,
	DBORDINAL iCol,
	DBCOLUMNINFO * pColumnsInfo,
	DBPROPSET * pPropertySets
)
{
	BOOL fSuccess  = FALSE;
	BOOL fMayDefer = FALSE;
	ULONG iProp;

	// BLOB columns are deferred by default
	if (pColumnsInfo[iCol-1].dwFlags & DBCOLUMNFLAGS_ISLONG)
		fMayDefer = TRUE;

	// See if DBPROP_DEFERRED is set on for this column
	for (iProp = 0; iProp < pPropertySets[0].cProperties; iProp++)
	{
		if ((pPropertySets[0].rgProperties[iProp].dwPropertyID == DBPROP_DEFERRED) &&
			(pPropertySets[0].rgProperties[iProp].dwStatus != DBPROPSTATUS_NOTSUPPORTED))
		{
			// We found the property and it's supported
			if (CompareDBID(pPropertySets[0].rgProperties[iProp].colid, DB_NULLID) ||
				CompareDBID(pPropertySets[0].rgProperties[iProp].colid, pColumnsInfo[iCol-1].columnid))
			{
				// The property either applies to all columns or it was for this specific column
				if (V_VT(&pPropertySets[0].rgProperties[iProp].vValue) != VT_EMPTY &&
					V_BOOL(&pPropertySets[0].rgProperties[iProp].vValue) == VARIANT_FALSE)
				{
					// The value was FALSE, so the column is not deferred.
					fMayDefer = FALSE;
				}
				break;
			}
		}
	}

	// Since the MAYDEFER flag isn't dependent on column type, and the default may be different
	// for different providers, just verify with IColumsInfo value
	if( (!!(dwFlags & DBCOLUMNFLAGS_MAYDEFER)) == fMayDefer)
		fSuccess = TRUE;
	else
		odtLog << L"[ " << pwszName << L" ] "
			   << L"ERROR DBCOLUMNFLAGS_MAYDEFER value expected " << fMayDefer << L" received " << !!(dwFlags & DBCOLUMNFLAGS_MAYDEFER) << ENDL;
		
	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_WRITE.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_WRITE
(
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess  = FALSE;

	// All IColumnsRowset Columns are ReadOnly
	if( dbColumnInfo.dwFlags & DBCOLUMNFLAGS_WRITE )
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The column is ReadOnly, DBCOLUMNFLAGS_WRITE is present."<< ENDL;
	else
		fSuccess = TRUE;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNFLAGS_WRITEUNKNOWN.
//
// @rdesc TRUE or FALSE
//
BOOL IColRow::Compare_DBCOLUMNFLAGS_WRITEUNKNOWN
(
	DBCOLUMNINFO dbColumnInfo
)
{
	BOOL fSuccess  = FALSE;

	// All IColumnsRowset Columns are ReadOnly
	if( dbColumnInfo.dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN )
		odtLog << L"[ " << dbColumnInfo.pwszName << L" ] "
			   << L"ERROR The column is ReadOnly, DBCOLUMNFLAGS_WRITEUNKNOWN is present."<< ENDL;
	else
		fSuccess = TRUE;

	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	AdjustStringLengths
//
//	Change all of the String lengths for ulColumnSize.
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void IColRow::AdjustStringLengths
(
	COLROWINFO * s_rgColRowInfo
)
{
	HRESULT			hr				= E_FAIL;
	IDBInfo *		pIDBInfo		= NULL;
	
	ULONG			cLiteralInfo	= 0;
	DBLITERAL		rgLiterals[1];
	DBLITERALINFO *	prgLiteralInfo	= NULL;
	OLECHAR *		pCharBuffer		= NULL;

	// Check the in Rowset pointer
	if( (!m_pIDBInitialize) || (s_rgColRowInfo->dbtype != DBTYPE_WSTR) )
		return;
	
	// Get the IID_IDBInfo Interface off of the IDBInitialize pointer
	if( !VerifyInterface(m_pIDBInitialize, IID_IDBInfo, 
						DATASOURCE_INTERFACE, (IUnknown **)&pIDBInfo) )
		return;

	// Switch on the ordinals of the column
	switch( s_rgColRowInfo->ulOrdinal )
	{
		// Column name length
		case 1:
		case 4:
		case 13:
			rgLiterals[0] = DBLITERAL_COLUMN_NAME;
			break;

		// Catalog name length
		case 12:
		case 21:
			rgLiterals[0] = DBLITERAL_CATALOG_NAME;
			break;

		// Schema name length
		case 14:
		case 22:
			rgLiterals[0] = DBLITERAL_SCHEMA_NAME;
			break;

		// Table name length
		case 15:
		case 23:
			rgLiterals[0] = DBLITERAL_TABLE_NAME;
			break;
	}
	
	hr = pIDBInfo->GetLiteralInfo(1, rgLiterals, 
							 &cLiteralInfo, &prgLiteralInfo, &pCharBuffer);

	// Check the HRESULT
	if( prgLiteralInfo->fSupported )
		COMPARE(hr, S_OK);
	else 
		COMPARE(hr, DB_E_ERRORSOCCURRED);

	// Copy the value into the Static structure
	if( (prgLiteralInfo->fSupported) &&
		(prgLiteralInfo->cchMaxLen)  && (prgLiteralInfo->cchMaxLen != (ULONG)~0) )
		s_rgColRowInfo->ulColumnSize = prgLiteralInfo->cchMaxLen;

	// Release the IDBInfo pointer
	SAFE_RELEASE(pIDBInfo);
	PROVIDER_FREE(prgLiteralInfo);
	PROVIDER_FREE(pCharBuffer);

	return;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	If_DATETIME
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::If_DATETIME(DBTYPE dbtype)
{
 	if( (DBTYPE_DATE   == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY) & dbtype)) ||
		(DBTYPE_DBDATE == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY) & dbtype)) ||
		(DBTYPE_DBTIME == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY) & dbtype)) ||
		(DBTYPE_DBTIMESTAMP == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY) & dbtype)) )
		return TRUE;

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	If_CHAR_BYTE
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::If_CHAR_Or_BYTE(DBTYPE dbtype)
{
 	if( (DBTYPE_BYTES == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY)  & dbtype)) ||
		(DBTYPE_STR   == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY)  & dbtype)) ||
		(DBTYPE_WSTR  == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY)  & dbtype)) )
		return TRUE;

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	If_CHAR
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::If_CHAR(DBTYPE dbtype)
{
 	if( (DBTYPE_STR  == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY|DBTYPE_RESERVED)  & dbtype)) ||
		(DBTYPE_WSTR == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY|DBTYPE_RESERVED)  & dbtype)) ||
		(DBTYPE_BSTR == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY|DBTYPE_RESERVED)  & dbtype)) )
		return TRUE;

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	IsScaleUsed
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::IsScaleUsed(DBTYPE dbtype)
{
 	if( (DBTYPE_NUMERIC     == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY|DBTYPE_RESERVED)  & dbtype)) ||
		(DBTYPE_DECIMAL     == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY|DBTYPE_RESERVED)  & dbtype)) ||
		(DBTYPE_VARNUMERIC  == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY|DBTYPE_RESERVED)  & dbtype)) ||
		(DBTYPE_DBTIMESTAMP == (~(DBTYPE_BYREF|DBTYPE_VECTOR|DBTYPE_ARRAY|DBTYPE_RESERVED)  & dbtype)) )
		return TRUE;

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	IsSQLServer
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColRow::IsSQLServer()
{
	// Properties
	BOOL fSQLServer = FALSE;
	WCHAR* pwszDBMSName = NULL;
	
	GetProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO,
		m_pThisTestModule->m_pIUnknown, &pwszDBMSName);
	
	if(pwszDBMSName && wcscmp(pwszDBMSName, L"Microsoft SQL Server")==0)
		fSQLServer = TRUE;
	
	PROVIDER_FREE(pwszDBMSName);
	return fSQLServer;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(Cmd_GetAvailableColumns)
//--------------------------------------------------------------------
// @class Test Case for GetAvailableColumns on Command Object
//
class Cmd_GetAvailableColumns : public IColRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Cmd_GetAvailableColumns,IColRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK: Query Set, Valid Params
	int Variation_1();
	// @cmember DB_E_NOCOMMAND: Command text not set
	int Variation_2();
	// @cmember DB_E_NOTPREPARED: Not Prepared with Command text set
	int Variation_3();
	// @cmember S_OK: Prepared Query
	int Variation_4();
	// @cmember S_OK: Bookmark Property
	int Variation_5();
	// @cmember S_OK: IRowsetLocate Property
	int Variation_6();
	// @cmember S_OK: IRowsetChange Property
	int Variation_7();
	// @cmember S_OK: Empty Table
	int Variation_8();
	// @cmember S_OK: Zero Rowset -> From Insert Statement
	int Variation_9();
	// @cmember E_INVALIDARG: cOptColumns == NULL
	int Variation_10();
	// @cmember E_INVALIDARG: rgOptColumns == NULL
	int Variation_11();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Cmd_GetAvailableColumns)
#define THE_CLASS Cmd_GetAvailableColumns
BEG_TEST_CASE(Cmd_GetAvailableColumns, IColRow, L"Test Case for GetAvailableColumns on Command Object")
	TEST_VARIATION(1, 		L"S_OK: Query Set, Valid Params")
	TEST_VARIATION(2, 		L"DB_E_NOCOMMAND: Command text not set")
	TEST_VARIATION(3, 		L"DB_E_NOTPREPARED: Not Prepared with Command text set")
	TEST_VARIATION(4, 		L"S_OK: Prepared Query")
	TEST_VARIATION(5, 		L"S_OK: Bookmark Property")
	TEST_VARIATION(6, 		L"S_OK: IRowsetLocate Property")
	TEST_VARIATION(7, 		L"S_OK: IRowsetChange Property")
	TEST_VARIATION(8, 		L"S_OK: Empty Table")
	TEST_VARIATION(9, 		L"S_OK: Zero Rowset -> From Insert Statement")
	TEST_VARIATION(10, 		L"E_INVALIDARG: cOptColumns == NULL")
	TEST_VARIATION(11, 		L"E_INVALIDARG: rgOptColumns == NULL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Cmd_GetColumnsRowset)
//--------------------------------------------------------------------
// @class Test case for GetColumnsRowset on Command object
//
class Cmd_GetColumnsRowset : public IColRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Cmd_GetColumnsRowset,IColRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK: Valid params, Query set
	int Variation_1();
	// @cmember DB_E_NOCOMMAND: no text set
	int Variation_2();
	// @cmember DB_E_NOTPREPARED: Text set, not prepared
	int Variation_3();
	// @cmember S_OK: Query set, count==0, array == NULL, valid ppColRowset
	int Variation_4();
	// @cmember S_OK: Prepared Query
	int Variation_5();
	// @cmember S_OK: Bookmark Property
	int Variation_6();
	// @cmember S_OK: IRowsetLocate
	int Variation_7();
	// @cmember S_OK: Empty Table
	int Variation_8();
	// @cmember S_OK: Zero Rowset (from Insert Statement
	int Variation_9();
	// @cmember E_INVALIDARG: ppColRowset==NULL
	int Variation_10();
	// @cmember E_INVALIDARG - NULL rgProperySets and cPropertySets != 0
	int Variation_11();
	// @cmember E_INVALIDARG - NULL rgProperies anc cProperties != 0
	int Variation_12();
	// @cmember E_INVALIDARG - NULL rgProperies anc cProperties != 0 in 2nd rgPropertySet
	int Variation_13();
	// @cmember E_NOINTERFACE: IID_ICommand for the riid
	int Variation_14();
	// @cmember S_OK: Pass some DBIDs
	int Variation_15();
	// @cmember S_OK: Pass DBIDs in reverse order
	int Variation_16();
	// @cmember S_OK: Pass Duplicate DBIDs
	int Variation_17();
	// @cmember DB_E_BADCOLUMNID: Invalid DBID
	int Variation_18();
	// @cmember Pass thru to SQL Col Attributes
	int Variation_19();
	// @cmember DB_S_ERRORSOCCURRED: requested IRowsetChange
	int Variation_20();
	// @cmember DB_S_ERRORSOCCURRED: requested DBPROP_ACTIVESESSIONS as property
	int Variation_21();
	// @cmember DB_S_ERRORSOCCURRED: requested a Property with a Bad colid
	int Variation_22();
	// @cmember DB_E_ERRORSOCCURRED: requested a Property with a Bad dwOption
	int Variation_23();
	// @cmember DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue Type
	int Variation_24();
	// @cmember DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue value
	int Variation_25();
	// @cmember DB_E_ERRORSOCCURRED: required IRowsetChange
	int Variation_26();
	// @cmember DB_E_ERRORSOCCURRED: required DBPROP_ACTIVESESSIONS as property
	int Variation_27();
	// @cmember DB_E_ERRORSOCCURRED: required a Property with a Bad colid
	int Variation_28();
	// @cmember DB_E_ERRORSOCCURRED: required a Property with a Bad vValue Type
	int Variation_29();
	// @cmember DB_E_ERRORSOCCURRED: required a Property with a Bad vValue value
	int Variation_30();
	// @cmember S_OK: SQL server specific optional columns - FOR BROWSE
	int Variation_31();
	// @cmember S_OK: SQL server specific optional columns, COMPUTE BY
	int Variation_32();
	// @cmember S_OK: Bookmark Property with an Insert Statement
	int Variation_33();
	// @cmember S_OK: Oracle Specific test - FOR UPDATE
	int Variation_34();
	// @cmember S_OK: DBPROP_HIDDENCOLUMNS
	int Variation_35();
	// @cmember S_OK: Aggregated columns rowset on command
	int Variation_36();
	// @cmember DB_E_NOAGGREGATION - Request aggregation but non-IUnknown iid
	int Variation_37();
	// @cmember S_OK: Aggregated columns rowset on command with IID_IRowsetFind
	int Variation_38();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Cmd_GetColumnsRowset)
#define THE_CLASS Cmd_GetColumnsRowset
BEG_TEST_CASE(Cmd_GetColumnsRowset, IColRow, L"Test case for GetColumnsRowset on Command object")
	TEST_VARIATION(1, 		L"S_OK: Valid params, Query set")
	TEST_VARIATION(2, 		L"DB_E_NOCOMMAND: no text set")
	TEST_VARIATION(3, 		L"DB_E_NOTPREPARED: Text set, not prepared")
	TEST_VARIATION(4, 		L"S_OK: Query set, count==0, array == NULL, valid ppColRowset")
	TEST_VARIATION(5, 		L"S_OK: Prepared Query")
	TEST_VARIATION(6, 		L"S_OK: Bookmark Property")
	TEST_VARIATION(7, 		L"S_OK: IRowsetLocate")
	TEST_VARIATION(8, 		L"S_OK: Empty Table")
	TEST_VARIATION(9, 		L"S_OK: Zero Rowset (from Insert Statement")
	TEST_VARIATION(10, 		L"E_INVALIDARG: ppColRowset==NULL")
	TEST_VARIATION(11, 		L"E_INVALIDARG - NULL rgProperySets and cPropertySets != 0")
	TEST_VARIATION(12, 		L"E_INVALIDARG - NULL rgProperies anc cProperties != 0")
	TEST_VARIATION(13, 		L"E_INVALIDARG - NULL rgProperies anc cProperties != 0 in 2nd rgPropertySet")
	TEST_VARIATION(14, 		L"E_NOINTERFACE: IID_ICommand for the riid")
	TEST_VARIATION(15, 		L"S_OK: Pass some DBIDs")
	TEST_VARIATION(16, 		L"S_OK: Pass DBIDs in reverse order")
	TEST_VARIATION(17, 		L"S_OK: Pass Duplicate DBIDs")
	TEST_VARIATION(18, 		L"DB_E_BADCOLUMNID: Invalid DBID")
	TEST_VARIATION(19, 		L"Pass thru to SQL Col Attributes")
	TEST_VARIATION(20, 		L"DB_S_ERRORSOCCURRED: requested IRowsetChange")
	TEST_VARIATION(21, 		L"DB_S_ERRORSOCCURRED: requested DBPROP_ACTIVESESSIONS as property")
	TEST_VARIATION(22, 		L"DB_S_ERRORSOCCURRED: requested a Property with a Bad colid")
	TEST_VARIATION(23, 		L"DB_E_ERRORSOCCURRED: requested a Property with a Bad dwOption")
	TEST_VARIATION(24, 		L"DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue Type")
	TEST_VARIATION(25, 		L"DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue value")
	TEST_VARIATION(26, 		L"DB_E_ERRORSOCCURRED: required IRowsetChange")
	TEST_VARIATION(27, 		L"DB_E_ERRORSOCCURRED: required DBPROP_ACTIVESESSIONS as property")
	TEST_VARIATION(28, 		L"DB_E_ERRORSOCCURRED: required a Property with a Bad colid")
	TEST_VARIATION(29, 		L"DB_E_ERRORSOCCURRED: required a Property with a Bad vValue Type")
	TEST_VARIATION(30, 		L"DB_E_ERRORSOCCURRED: required a Property with a Bad vValue value")
	TEST_VARIATION(31, 		L"S_OK: SQL server specific optional columns - FOR BROWSE")
	TEST_VARIATION(32, 		L"S_OK: SQL server specific optional columns, COMPUTE BY")
	TEST_VARIATION(33, 		L"S_OK: Bookmark Property with an Insert Statement")
	TEST_VARIATION(34, 		L"S_OK: Oracle Specific test - FOR UPDATE")
	TEST_VARIATION(35, 		L"S_OK: DBPROP_HIDDENCOLUMNS")
	TEST_VARIATION(36, 		L"S_OK: Aggregated columns rowset on command")
	TEST_VARIATION(37, 		L"DB_E_NOAGGREGATION - Request aggregation but non-IUnknown iid")
	TEST_VARIATION(38, 		L"S_OK: Aggregated columns rowset on command with IID_IRowsetFind")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Row_GetAvailableColumns)
//--------------------------------------------------------------------
// @class Test case for GetAvailableColumns on Rowset
//
class Row_GetAvailableColumns : public IColRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Row_GetAvailableColumns,IColRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK: Valid Params
	int Variation_1();
	// @cmember S_OK: Prepared Query
	int Variation_2();
	// @cmember S_OK: Prepared and Unprepared Query
	int Variation_3();
	// @cmember S_OK: Bookmark Property
	int Variation_4();
	// @cmember S_OK: IRowsetLocate Property
	int Variation_5();
	// @cmember S_OK: IRowsetChange Property
	int Variation_6();
	// @cmember S_OK: Empty Table
	int Variation_7();
	// @cmember E_INVALIDARG: cOptColumns == NULL
	int Variation_8();
	// @cmember E_INVALIDARG: rgOptColumns == NULL
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Row_GetAvailableColumns)
#define THE_CLASS Row_GetAvailableColumns
BEG_TEST_CASE(Row_GetAvailableColumns, IColRow, L"Test case for GetAvailableColumns on Rowset")
	TEST_VARIATION(1, 		L"S_OK: Valid Params")
	TEST_VARIATION(2, 		L"S_OK: Prepared Query")
	TEST_VARIATION(3, 		L"S_OK: Prepared and Unprepared Query")
	TEST_VARIATION(4, 		L"S_OK: Bookmark Property")
	TEST_VARIATION(5, 		L"S_OK: IRowsetLocate Property")
	TEST_VARIATION(6, 		L"S_OK: IRowsetChange Property")
	TEST_VARIATION(7, 		L"S_OK: Empty Table")
	TEST_VARIATION(8, 		L"E_INVALIDARG: cOptColumns == NULL")
	TEST_VARIATION(9, 		L"E_INVALIDARG: rgOptColumns == NULL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Row_GetColumnsRowset)
//--------------------------------------------------------------------
// @class Test case for GetColumnsRowset on Rowset
//
class Row_GetColumnsRowset : public IColRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Row_GetColumnsRowset,IColRow);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK: Valid params
	int Variation_1();
	// @cmember S_OK: Count==0, array == NULL, valid ppColRowset
	int Variation_2();
	// @cmember S_OK: Prepared Query
	int Variation_3();
	// @cmember S_OK: Prepared and Unprepared Query
	int Variation_4();
	// @cmember S_OK: Bookmark Property
	int Variation_5();
	// @cmember S_OK: IRowsetLocate
	int Variation_6();
	// @cmember S_OK: Empty Table
	int Variation_7();
	// @cmember E_INVALIDARG: ppColRowset==NULL
	int Variation_8();
	// @cmember E_INVALIDARG - NULL rgProperySets and cPropertySets != 0
	int Variation_9();
	// @cmember E_INVALIDARG - NULL rgProperies anc cProperties != 0
	int Variation_10();
	// @cmember E_INVALIDARG - NULL rgProperies anc cProperties != 0 in 2nd rgPropertySet
	int Variation_11();
	// @cmember E_NOINTERFACE: IID_ICommand for the riid
	int Variation_12();
	// @cmember S_OK: Pass some DBIDs
	int Variation_13();
	// @cmember S_OK: Pass DBIDs in reverse order
	int Variation_14();
	// @cmember S_OK: Pass Duplicate DBIDS
	int Variation_15();
	// @cmember DB_E_BADCOLUMNID: Invalid DBID
	int Variation_16();
	// @cmember S_OK: Call on itself with Optional Columns passed in
	int Variation_17();
	// @cmember S_OK: Call on itself with Optional Columns NOT passed in
	int Variation_18();
	// @cmember DB_S_ERRORSOCCURRED: requested IRowsetChange
	int Variation_19();
	// @cmember DB_S_ERRORSOCCURRED: requested DBPROP_ACTIVESESSIONS as property
	int Variation_20();
	// @cmember DB_S_ERRORSOCCURRED: requested a Property with a Bad colid
	int Variation_21();
	// @cmember DB_E_ERRORSOCCURRED: requested a Property with a Bad dwOption
	int Variation_22();
	// @cmember DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue Type
	int Variation_23();
	// @cmember DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue value
	int Variation_24();
	// @cmember DB_E_ERRORSOCCURRED: required IRowsetChange
	int Variation_25();
	// @cmember DB_E_ERRORSOCCURRED: required DBPROP_ACTIVESESSIONS as property
	int Variation_26();
	// @cmember DB_E_ERRORSOCCURRED: required a Property with a Bad colid
	int Variation_27();
	// @cmember DB_E_ERRORSOCCURRED: required a Property with a Bad vValue Type
	int Variation_28();
	// @cmember DB_E_ERRORSOCCURRED: required a Property with a Bad vValue value
	int Variation_29();
	// @cmember S_OK: SQL server specific optional columns - FOR BROWSE
	int Variation_30();
	// @cmember S_OK: SQL server specific optional columns, COMPUTE BY
	int Variation_31();
	// @cmember S_OK: Oracle Specific test - FOR UPDATE
	int Variation_32();
	// @cmember S_OK: DBPROP_HIDDENCOLUMNS
	int Variation_33();
	// @cmember S_OK: Aggregated columns rowset on rowset
	int Variation_34();
	// @cmember DB_E_NOAGGREGATION - Request aggregation but non-IUnknown iid
	int Variation_35();
	// @cmember S_OK: Aggregated columns rowset on rowset with IID_IRowsetFind
	int Variation_36();
	// @cmember DB_E_NOAGGREGATION: Execute with IID_IRowsetFind, aggregate GetColumnsRowset with IID_IRowset
	int Variation_37();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Row_GetColumnsRowset)
#define THE_CLASS Row_GetColumnsRowset
BEG_TEST_CASE(Row_GetColumnsRowset, IColRow, L"Test case for GetColumnsRowset on Rowset")
	TEST_VARIATION(1, 		L"S_OK: Valid params")
	TEST_VARIATION(2, 		L"S_OK: Count==0, array == NULL, valid ppColRowset")
	TEST_VARIATION(3, 		L"S_OK: Prepared Query")
	TEST_VARIATION(4, 		L"S_OK: Prepared and Unprepared Query")
	TEST_VARIATION(5, 		L"S_OK: Bookmark Property")
	TEST_VARIATION(6, 		L"S_OK: IRowsetLocate")
	TEST_VARIATION(7, 		L"S_OK: Empty Table")
	TEST_VARIATION(8, 		L"E_INVALIDARG: ppColRowset==NULL")
	TEST_VARIATION(9, 		L"E_INVALIDARG - NULL rgProperySets and cPropertySets != 0")
	TEST_VARIATION(10, 		L"E_INVALIDARG - NULL rgProperies anc cProperties != 0")
	TEST_VARIATION(11, 		L"E_INVALIDARG - NULL rgProperies anc cProperties != 0 in 2nd rgPropertySet")
	TEST_VARIATION(12, 		L"E_NOINTERFACE: IID_ICommand for the riid")
	TEST_VARIATION(13, 		L"S_OK: Pass some DBIDs")
	TEST_VARIATION(14, 		L"S_OK: Pass DBIDs in reverse order")
	TEST_VARIATION(15, 		L"S_OK: Pass Duplicate DBIDS")
	TEST_VARIATION(16, 		L"DB_E_BADCOLUMNID: Invalid DBID")
	TEST_VARIATION(17, 		L"S_OK: Call on itself with Optional Columns passed in")
	TEST_VARIATION(18, 		L"S_OK: Call on itself with Optional Columns NOT passed in")
	TEST_VARIATION(19, 		L"DB_S_ERRORSOCCURRED: requested IRowsetChange")
	TEST_VARIATION(20, 		L"DB_S_ERRORSOCCURRED: requested DBPROP_ACTIVESESSIONS as property")
	TEST_VARIATION(21, 		L"DB_S_ERRORSOCCURRED: requested a Property with a Bad colid")
	TEST_VARIATION(22, 		L"DB_E_ERRORSOCCURRED: requested a Property with a Bad dwOption")
	TEST_VARIATION(23, 		L"DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue Type")
	TEST_VARIATION(24, 		L"DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue value")
	TEST_VARIATION(25, 		L"DB_E_ERRORSOCCURRED: required IRowsetChange")
	TEST_VARIATION(26, 		L"DB_E_ERRORSOCCURRED: required DBPROP_ACTIVESESSIONS as property")
	TEST_VARIATION(27, 		L"DB_E_ERRORSOCCURRED: required a Property with a Bad colid")
	TEST_VARIATION(28, 		L"DB_E_ERRORSOCCURRED: required a Property with a Bad vValue Type")
	TEST_VARIATION(29, 		L"DB_E_ERRORSOCCURRED: required a Property with a Bad vValue value")
	TEST_VARIATION(30, 		L"S_OK: SQL server specific optional columns - FOR BROWSE")
	TEST_VARIATION(31, 		L"S_OK: SQL server specific optional columns, COMPUTE BY")
	TEST_VARIATION(32, 		L"S_OK: Oracle Specific test - FOR UPDATE")
	TEST_VARIATION(33, 		L"S_OK: DBPROP_HIDDENCOLUMNS")
	TEST_VARIATION(34, 		L"S_OK: Aggregated columns rowset on rowset")
	TEST_VARIATION(35, 		L"DB_E_NOAGGREGATION - Request aggregation but non-IUnknown iid")
	TEST_VARIATION(36, 		L"S_OK: Aggregated columns rowset on rowset with IID_IRowsetFind")
	TEST_VARIATION(37, 		L"DB_E_NOAGGREGATION: Execute with IID_IRowsetFind, aggregate GetColumnsRowset with IID_IRowset")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CZombie)
//--------------------------------------------------------------------
// @class Induce zombie states on the Command
//
class CZombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CZombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	// @mfunc Does the dirty work
	int TestTxn(EMETHOD eMethod, ETXN eTxn, BOOL fRetaining);

	// @mfunc See if the statement remains prepared
	void PrepareBehavior();

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	//@cmember	Interface for IDBProperties
	IDBProperties *	m_pIDBProperties;
	//@cmember Prepare Commit Behavior
	BOOL m_fPrepCommitPreserve;
	//@cmember Prepare Abort Behavior
	BOOL m_fPrepAbortPreserve;

	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=FALSE
	int Variation_2();
	// @cmember S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=TRUE
	int Variation_3();
	// @cmember S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=FALSE
	int Variation_4();
	// @cmember S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=TRUE
	int Variation_5();
	// @cmember S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=FALSE
	int Variation_6();
	// @cmember S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=TRUE
	int Variation_7();
	// @cmember S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=FALSE
	int Variation_8();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(CZombie)
#define THE_CLASS CZombie
BEG_TEST_CASE(CZombie, CTransaction, L"Induce zombie states on the Command")
	TEST_VARIATION(1, 		L"S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=FALSE")
	TEST_VARIATION(3, 		L"S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=TRUE")
	TEST_VARIATION(4, 		L"S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=FALSE")
	TEST_VARIATION(5, 		L"S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=TRUE")
	TEST_VARIATION(6, 		L"S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=FALSE")
	TEST_VARIATION(7, 		L"S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=TRUE")
	TEST_VARIATION(8, 		L"S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(RZombie)
//--------------------------------------------------------------------
// @class Induce zombie states on the Rowset
//
class RZombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RZombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	// Does the dirty work
	int TestTxn(EMETHOD eMethod, ETXN eTxn, BOOL fRetaining);

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	//@cmember	Set og Properties
	DBPROPSET m_rgPropertySets;

	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=FALSE
	int Variation_2();
	// @cmember S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=TRUE
	int Variation_3();
	// @cmember S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=FALSE
	int Variation_4();
	// @cmember S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=TRUE
	int Variation_5();
	// @cmember S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=FALSE
	int Variation_6();
	// @cmember S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=TRUE
	int Variation_7();
	// @cmember S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=FALSE
	int Variation_8();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(RZombie)
#define THE_CLASS RZombie
BEG_TEST_CASE(RZombie, CTransaction, L"Induce zombie states on the Rowset")
	TEST_VARIATION(1, 		L"S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=FALSE")
	TEST_VARIATION(3, 		L"S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=TRUE")
	TEST_VARIATION(4, 		L"S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=FALSE")
	TEST_VARIATION(5, 		L"S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=TRUE")
	TEST_VARIATION(6, 		L"S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=FALSE")
	TEST_VARIATION(7, 		L"S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=TRUE")
	TEST_VARIATION(8, 		L"S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(ExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class ExtendedErrors : public IColRow { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ExtendedErrors,IColRow);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid Cmd_IColumnsRowset calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid Cmd_IColumnsRowset calls with previous error object existing
	int Variation_2();
	// @cmember Invalid Cmd_IColumnsRowset calls with no previous error object existing
	int Variation_3();
	// @cmember Valid Row_IColumnsRowset calls with previous error object existing.
	int Variation_4();
	// @cmember Invalid Row_IColumnsRowset calls with previous error object existing
	int Variation_5();
	// @cmember Invalid Row_IColumnsRowset calls with no previous error object existing
	int Variation_6();
	// @cmember DB_E_NOCOMMAND or DB_E_NOTPREPARED Row_IColumnsRowset calls with no previous error object existing
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(ExtendedErrors)
#define THE_CLASS ExtendedErrors
BEG_TEST_CASE(ExtendedErrors, IColRow, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid Cmd_IColumnsRowset calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid Cmd_IColumnsRowset calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid Cmd_IColumnsRowset calls with no previous error object existing")
	TEST_VARIATION(4, 		L"Valid Row_IColumnsRowset calls with previous error object existing.")
	TEST_VARIATION(5, 		L"Invalid Row_IColumnsRowset calls with previous error object existing")
	TEST_VARIATION(6, 		L"Invalid Row_IColumnsRowset calls with no previous error object existing")
	TEST_VARIATION(7, 		L"DB_E_NOCOMMAND or DB_E_NOTPREPARED Row_IColumnsRowset calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(7, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, Cmd_GetAvailableColumns)
	TEST_CASE(2, Cmd_GetColumnsRowset)
	TEST_CASE(3, Row_GetAvailableColumns)
	TEST_CASE(4, Row_GetColumnsRowset)
	TEST_CASE(5, CZombie)
	TEST_CASE(6, RZombie)
	TEST_CASE(7, ExtendedErrors)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(Cmd_GetAvailableColumns)
//*-----------------------------------------------------------------------
//| Test Case:		Cmd_GetAvailableColumns - Test Case for GetAvailableColumns on Command Object
//|	Created:			02/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Cmd_GetAvailableColumns::Init()
{
	if (!g_fCmd)
	{
		odtLog << L"No IColumnsRowset support on commands\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(IColRow::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Query Set, Valid Params
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_1()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	ULONG	iCol;
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	BOOL	fFound;

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1), S_OK);									// [IN] property set

	// Display the optional DBCOLUMNs
	odtLog << L"Supported OPTIONAL columns:\n";
	for(ULONG index=0; index<m_cDBID; index++)
	{
		fFound = FALSE;
		for (iCol = MANCOL+1; iCol < NUMELEM(g_rgColRowInfo); iCol++)
		{
			if (CompareDBID(m_rgDBID[index],*g_rgColRowInfo[iCol].columnid))
			{
				odtLog << L"\t" << g_rgColRowInfo[iCol].pwszName << L"\n";
				fFound = TRUE;
				break;
			}
		}

		if (!fFound)
			odtLog << L"The DBID is Provider Specific\n";
	}

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: Command text not set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_2()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// The mandatory columns are fixed and known to the provider before setting
	// command text or preparing.  It's possible the optional columns are also,
	// so some providers can return S_OK here.
	hr = GetAvailableColumns(
		&m_cDBID,
		&m_rgDBID,
		eNOCOMMAND,									// [IN] kind of statement
		NO_QUERY,									// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1);											// [IN] property set

	// Verify results
	if (S_OK == hr &&
		Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID))
		fSucceed = TEST_PASS;
	else if (Check_GetAvailableColumns(DB_E_NOCOMMAND, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTPREPARED: Not Prepared with Command text set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_3()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// The mandatory columns are fixed and known to the provider before setting
	// command text or preparing.  It's possible the optional columns are also,
	// so some providers can return S_OK here.
	hr = GetAvailableColumns(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_ALLFROMTBL,							// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1);

	// Verify results
	if (S_OK == hr &&
		Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID))
		fSucceed = TEST_PASS;
	else if( Check_GetAvailableColumns(DB_E_NOTPREPARED, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Prepared Query
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_4()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1),S_OK);									// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Bookmark Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_5()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,
		DBPROP_BOOKMARKS), S_OK);					// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: IRowsetLocate Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_6()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
		
	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,
		DBPROP_IRowsetLocate), S_OK);				// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: IRowsetChange Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_7()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
		
	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,
		DBPROP_IRowsetChange), S_OK);				// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Empty Table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_8()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	ULONG	index		= 0;

	INIT;

	// Check to see if the DSO is ReadOnly
	if( !IsPropertySet(DBPROPSET_DATASOURCEINFO, DBPROP_DATASOURCEREADONLY, NULL, 0) )
	{
		// Delete all rows in the table.
		if(!CHECK(m_pTable->Delete(),S_OK))
			goto CLEANUP;
	}
	else
	{
		// Drop the table and create an empty one
		if(!CHECK(m_pTable->DropTable(),S_OK))
			goto CLEANUP;

		if(!SUCCEEDED(m_pTable->CreateTable(0,1,NULL,PRIMARY,TRUE)))
			goto CLEANUP;
	}

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1), S_OK);									// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	// Reinsert all the rows in the table
	for(index=1; index < 30; index++)
		CHECK(m_pTable->Insert(), S_OK);

CLEANUP:

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Zero Rowset -> From Insert Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_9()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns(
		&m_cDBID,
		&m_rgDBID,
		eINSERT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1), S_OK);									// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cOptColumns == NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_10()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns(
		NULL,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1), E_INVALIDARG);

	// Verify results
	if( Check_GetAvailableColumns(E_INVALIDARG, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rgOptColumns == NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetAvailableColumns::Variation_11()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns(
		&m_cDBID,
		NULL,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1), E_INVALIDARG);

	// verify results
	if( Check_GetAvailableColumns(E_INVALIDARG, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Cmd_GetAvailableColumns::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Cmd_GetColumnsRowset)
//*-----------------------------------------------------------------------
//| Test Case:		Cmd_GetColumnsRowset - Test case for GetColumnsRowset on Command object
//|	Created:			02/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Cmd_GetColumnsRowset::Init()
{
	if (!g_fCmd)
	{
		odtLog << L"No IColumnsRowset support on commands\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(IColRow::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Valid params, Query set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_1()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		S_OK,							// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS					// Some providers require an executed state
	);

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: no text set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_2()
{
 	HRESULT	hr			= E_FAIL;			// HRESULT
	BOOL	fSucceed	= TEST_FAIL;		// indicates variation success

	INIT;

	// hr = GetColumnsRowset
	hr = GetColumnsRowset(
		0,
		NULL,
		IID_IRowset,							// [IN] which optional columns 
		TRUE,
		ALLDBID,								// [IN] which optional columns 
		eNOCOMMAND,								// [IN] kind of statement
		NO_QUERY,								// [IN] sql statement
		NULL,									// [IN] client's choice for sql statement
		BOTH,									// [IN] prepared state
		1,										// [IN] run's prepared for
		VALIDROWSET,							// [IN] Property structure
		ISOPTIONAL,								// [IN] Property dwOption
		0,										// [IN]	Prpoerty ID
		&m_pIRowsetReturned,					// [IN] pIRowset
		FALSE,
		DB_E_NOCOMMAND);

	// Verify results
	if( Check_GetColumnsRowset(DB_E_NOCOMMAND,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTPREPARED: Text set, not prepared
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_3()
{
 	HRESULT	hr			= E_FAIL;			// HRESULT
 	HRESULT	Exphr		= DB_E_NOTPREPARED;	// Expected HRESULT
	BOOL	fSucceed	= TEST_FAIL;		// indicates variation success

	INIT;

	// If ICommandPrepare is not supported
	if( NOTSUPPORTED == m_ePREPARATION )
 		Exphr = S_OK;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		0,
		NULL,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		BOTH,							// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,
		Exphr);	

	// Verify results
	if( Check_GetColumnsRowset(Exphr,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Query set, count==0, array == NULL, valid ppColRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_4()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		0,
		NULL,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned				// [IN] pIRowset
	);	

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Prepared Query
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_5()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned				// [IN] pIRowset
	);	

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Bookmark Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_6()
{
 	HRESULT	ExpHR	 = S_OK;		// Expect HResult
	HRESULT	hr		 = E_FAIL;		// HRESULT
	BOOL	fSucceed = TEST_FAIL;	// indicates variation success

	INIT;

	// Check to see if BOOKMARKS are supported or ReadOnly and Variant False
	if( (!SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE)) ||
		(!SettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) &&
		 GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommand, VARIANT_FALSE)) )
		ExpHR = DB_E_ERRORSOCCURRED;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		REQUIRED,						// [IN] Property dwOption
		DBPROP_BOOKMARKS,				// [IN] property set
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] Skip property check
		ExpHR							// [IN] Expected HResult
	);	

	// Verify results
	if( Check_GetColumnsRowset(ExpHR,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: IRowsetLocate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_7()
{
 	HRESULT	ExpHR	 = S_OK;		// Expect HResult
 	HRESULT	hr		 = E_FAIL;		// HRESULT
	BOOL	fSucceed = TEST_FAIL;	// indicates variation success

	INIT;

	// Check to see if IRowsetLocate is supported or ReadOnly and Variant False
	if( (!SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE)) ||
		(!SettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) &&
		 GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pICommand, VARIANT_FALSE)) )
		ExpHR = DB_E_ERRORSOCCURRED;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		REQUIRED,						// [IN] Property dwOption
		DBPROP_IRowsetLocate,			// [IN] property set
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] Skip property check
		ExpHR							// [IN] Expected HResult
	);	

	// Verify results
	if( Check_GetColumnsRowset(ExpHR,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Empty Table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_8()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	ULONG	index		= 0;

	INIT;

	// Check to see if the DSO is ReadOnly
	if( !IsPropertySet(DBPROPSET_DATASOURCEINFO, DBPROP_DATASOURCEREADONLY, NULL, 0) )
	{
		// Delete all rows in the table.
		if(!CHECK(m_pTable->Delete(),S_OK))
			goto CLEANUP;
	}
	else
	{
		// Drop the table and create an empty one
		if(!CHECK(m_pTable->DropTable(),S_OK))
			goto CLEANUP;

		if(!SUCCEEDED(m_pTable->CreateTable(0,1,NULL,PRIMARY,TRUE)))
			goto CLEANUP;
	}

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned				// [IN] pIRowset
	);	

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	// Insert all the rows in the table
	for(index=1;index<11;index++)
		m_pTable->Insert(index);

CLEANUP:

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Zero Rowset (from Insert Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_9()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eINSERT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned				// [IN] pIRowset
	);	

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset,eINSERT) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: ppColRowset==NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_10()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		FALSE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN] Property ID
		NULL,							// [IN] pIRowset
		FALSE,
		E_INVALIDARG);			

	// Verify results
	if( Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL rgProperySets and cPropertySets != 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_11()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Initialize the Properties
	m_cDBPROPSET = 1;

	// set text
	if(FAILED(hr = SetCommandText(m_pIMalloc, m_pICommand, m_pTable, 
									NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL)))
		goto CLEANUP;

	// prepare
	if(FAILED(hr = PrepareCommand(m_pICommand, PREPARE, 1)))
		goto CLEANUP;

	if(FAILED(hr=m_pICommand->QueryInterface(IID_IColumnsRowset,(void **)&m_pIColumnsRowset)))
		goto CLEANUP;

	CHECK(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, 0, NULL, 
			IID_IRowset, m_cDBPROPSET, NULL, PPI m_pIRowsetReturned),E_INVALIDARG);

	// Verify results
	if( Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

CLEANUP:

	FREE

  return fSucceed;
	
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL rgProperies anc cProperties != 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_12()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Initialize the Properties
	m_cDBPROPSET = 1;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPROPSET[0].cProperties = 1;
	m_rgDBPROPSET[0].rgProperties = NULL;

	// Get ColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,							// [IN] which optional columns 
		FALSE,
		ALLDBID,								// [IN] which optional columns 
		eSELECT,								// [IN] kind of statement
		SELECT_COLLISTFROMTBL,					// [IN] sql statement
		NULL,									// [IN] client's choice for sql statement
		PREPARE,								// [IN] prepared state
		1,										// [IN] run's prepared for
		VALIDROWSET,							// [IN] Property structure
		ISOPTIONAL,								// [IN] Property dwOption
		0,										// [IN]	Prpoerty ID
		&m_pIRowsetReturned,					// [IN] pIRowset
		FALSE,
		E_INVALIDARG);	

	// Verify results
	if( Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL rgProperies anc cProperties != 0 in 2nd rgPropertySet
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_13()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Initialize the Properties
	m_cDBPROPSET = 2;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPROPSET[0].cProperties = 0;
	m_rgDBPROPSET[0].rgProperties = NULL;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[1].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPROPSET[1].cProperties = 1;
	m_rgDBPROPSET[1].rgProperties = NULL;

	// Get IColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,							// [IN] which optional columns 
		FALSE,
		ALLDBID,								// [IN] which optional columns 
		eSELECT,								// [IN] kind of statement
		SELECT_COLLISTFROMTBL,					// [IN] sql statement
		NULL,									// [IN] client's choice for sql statement
		PREPARE,								// [IN] prepared state
		1,										// [IN] run's prepared for
		VALIDROWSET,							// [IN] Property structure
		ISOPTIONAL,								// [IN] Property dwOption
		0,										// [IN]	Prpoerty ID
		&m_pIRowsetReturned,					// [IN] pIRowset
		FALSE,
		E_INVALIDARG);	

	// Verify results
	if( Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE: IID_ICommand for the riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_14()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_ICommand,							// [IN] which optional columns 
		TRUE,
		ALLDBID,								// [IN] which optional columns 
		eSELECT,								// [IN] kind of statement
		SELECT_COLLISTFROMTBL,					// [IN] sql statement
		NULL,									// [IN] client's choice for sql statement
		PREPARE,								// [IN] prepared state
		1,										// [IN] run's prepared for
		VALIDROWSET,							// [IN] Property structure
		ISOPTIONAL,								// [IN] Property dwOption
		0,										// [IN] property set
		&m_pIRowsetReturned,					// [IN] pIRowset
		FALSE,
		E_NOINTERFACE);	
	
	// Verify results
	if( Check_GetColumnsRowset(E_NOINTERFACE,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_ICommand) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Pass some DBIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_15()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		SOMEDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned				// [IN] pIRowset
	);	

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Pass DBIDs in reverse order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_16()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		REVERSEDBID,					// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned				// [IN] pIRowset
	);	

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Pass Duplicate DBIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_17()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		DUPLICATEDBID,					// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned				// [IN] pIRowset
	);							

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCOLUMNID: Invalid DBID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_18()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,							// [IN] which optional columns 
		TRUE,
		INVALIDDBID,							// [IN] which optional columns 
		eSELECT,								// [IN] kind of statement
		SELECT_COLLISTFROMTBL,					// [IN] sql statement
		NULL,									// [IN] client's choice for sql statement
		PREPARE,								// [IN] prepared state
		1,										// [IN] run's prepared for
		VALIDROWSET,							// [IN] Property structure
		ISOPTIONAL,								// [IN] Property dwOption
		0,										// [IN]	Prpoerty ID
		&m_pIRowsetReturned,					// [IN] pIRowset
		FALSE,
		DB_E_BADCOLUMNID);

	// Verify results
	if( Check_GetColumnsRowset(DB_E_BADCOLUMNID,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Pass thru to SQL Col Attributes
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_19()
{
 	HRESULT		ExpHR		= S_OK;
	HRESULT		hr			= E_FAIL;		// HRESULT
	BOOL		fSucceed	= TEST_FAIL;	// indicates variation success
	ULONG		index		= 0;
	ULONG		cErrors		= 0;
	IRowset *	pIRowset	= NULL;
	DBID		rgDBID[1];

	// This test variation is only supported on Kagera.
	if (!g_fKagera)
		ExpHR = DB_E_BADCOLUMNID;
	
	for(index=0; index<COLATTRIB_MAX; index++)
	{
		INIT;

		// Set the Command Text
		TESTC_(hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
									 NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL), S_OK);

		// Prepare
		TESTC_(hr=PrepareCommand(m_pICommand,PREPARE,1), S_OK);

		TESTC(GetRowsetInfo(m_pICommand));


		// Set the DBID contents
		rgDBID[0].uGuid.guid = GUID_NULL;
		rgDBID[0].eKind = DBKIND_GUID_PROPID;
		rgDBID[0].uName.ulPropid = g_rgCOLATTRIB[index];

		TESTC_(hr=m_pICommand->QueryInterface(IID_IColumnsRowset,(void **)&m_pIColumnsRowset), S_OK);

		// Set property if Supported
		if( IsPropertySupported(DBPROPSET_ROWSET, DBPROP_IColumnsRowset) )
		{
			// Get ColumnsRowset
			hr = m_pIColumnsRowset->GetColumnsRowset(
				NULL, 
				1, 
				rgDBID, 
				IID_IRowset, 
				0, 
				NULL, 
				(IUnknown**) &pIRowset);

			// Against non-Kagera providers we really expect this to fail
			// with DB_E_BADCOLUMNID, but since other providers may run on
			// top of Kagera it could succeed.  Don't fail them for this.
			if (SUCCEEDED(hr) && !g_fKagera)
			{
				CHECKW(hr, ExpHR);
				ExpHR = S_OK;	
			}

			// Verify results, 
			if(!Check_GetColumnsRowset(ExpHR,hr,1,rgDBID,pIRowset,IID_IRowset) )
				cErrors++;
		}
		else
			goto CLEANUP;

CLEANUP:

		// Release the rowset
		SAFE_RELEASE(pIRowset);
		FREE;
	}
	
	// Return results
	if( cErrors )
		return TEST_FAIL;
	else
		return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested IRowsetChange
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_20()
{
 	HRESULT	hr		 = E_FAIL;		// HRESULT
	BOOL	fSucceed = TEST_FAIL;	// indicates variation success

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		DBPROP_IRowsetChange,						// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [IN] pIRowset
		FALSE,
		DB_S_ERRORSOCCURRED);	

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested DBPROP_ACTIVESESSIONS as property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_21()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Initialize the Properties
	m_cDBPROPSET = 1;

	m_rgProperties[0].dwPropertyID = DBPROP_ACTIVESESSIONS;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_OPTIONAL;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&m_rgProperties[0].vValue) = VARIANT_TRUE;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	m_rgDBPROPSET[0].cProperties = 1;
	m_rgDBPROPSET[0].rgProperties = m_rgProperties;

	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		DBPROP_ACTIVESESSIONS,						// [IN]	Prpoerty ID
		&m_pIRowsetReturned,
		TRUE,										// [in] skip prop check 'cause it's a DS prop
		DB_S_ERRORSOCCURRED);				

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested a Property with a Bad colid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_22()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	DBPROPID dwColPropID = 0;			// Rowset prop with COLUMNOK

	if (!(dwColPropID = FindColumnProperty(m_pThisTestModule->m_pIUnknown)))
	{
		odtLog << L"Provider does not support any DBPROPSET_ROWSET properties with DBPROPFLAGS_COLUMNOK.\n";
		return TEST_SKIPPED;
	}

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		BADCOLID,									// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		dwColPropID,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [IN] pIRowset
		FALSE,
		DB_S_ERRORSOCCURRED);	

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: requested a Property with a Bad dwOption
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_23()
{
 	PROPOPTION	dwOption = BADOPTION;	// Expected Option
 	HRESULT		hr		 = E_FAIL;		// HRESULT
	BOOL		fSucceed = TEST_FAIL;	// indicates variation success

	// Check to see if BOOKMARKS are supported
	if( !SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) )
		dwOption = REQUIRED;

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		dwOption,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [IN] pIRowset
		FALSE,
		DB_E_ERRORSOCCURRED);	

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue Type
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_24()
{
 	PROPVALS	dwValue	= BADTYPE;		// Expected Option
 	HRESULT		hr		= E_FAIL;		// HRESULT
	BOOL		fSucceed= TEST_FAIL;	// indicates variation success

	// Check to see if BOOKMARKS are supported
	if( !SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) )
		dwValue = VALIDROWSET;

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		dwValue,									// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [IN] pIRowset
		FALSE,
		DB_S_ERRORSOCCURRED);	

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue value
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_25()
{
 	PROPVALS	dwValue	= BADTYPE;		// Expected Option
 	HRESULT		hr		= E_FAIL;		// HRESULT
	BOOL		fSucceed= TEST_FAIL;	// indicates variation success

	// Check to see if BOOKMARKS are supported
	if( !SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) )
		dwValue = VALIDROWSET;

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		dwValue,									// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [IN] pIRowset
		FALSE,
		DB_S_ERRORSOCCURRED);	

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required IRowsetChange
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_26()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		DBPROP_IRowsetChange,						// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [IN] pIRowset
		FALSE,
		DB_E_ERRORSOCCURRED);	

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required DBPROP_ACTIVESESSIONS as property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_27()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
	
	// Initialize the Properties
	m_cDBPROPSET = 1;

	m_rgProperties[0].dwPropertyID = DBPROP_ACTIVESESSIONS;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&m_rgProperties[0].vValue) = VARIANT_TRUE;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	m_rgDBPROPSET[0].cProperties = 1;
	m_rgDBPROPSET[0].rgProperties = m_rgProperties;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		DBPROP_ACTIVESESSIONS,						// [IN]	Prpoerty ID
		&m_pIRowsetReturned,
		TRUE,										// [in] skip prop check 'cause it's a DS prop
		DB_E_ERRORSOCCURRED);				

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required a Property with a Bad colid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_28()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	DBPROPID dwColPropID = 0;			// Rowset prop with COLUMNOK

	if (!(dwColPropID = FindColumnProperty(m_pThisTestModule->m_pIUnknown)))
	{
		odtLog << L"Provider does not support any DBPROPSET_ROWSET properties with DBPROPFLAGS_COLUMNOK.\n";
		return TEST_SKIPPED;
	}

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		BADCOLID,									// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		dwColPropID,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [IN] pIRowset
		FALSE,
		DB_E_ERRORSOCCURRED);	

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required a Property with a Bad vValue Type
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_29()
{
 	PROPVALS	dwValue	= BADTYPE;		// Expected Option
 	HRESULT		hr		= E_FAIL;		// HRESULT
	BOOL		fSucceed= TEST_FAIL;	// indicates variation success

	// Check to see if BOOKMARKS are supported
	if( !SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) )
		dwValue = VALIDROWSET;

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		dwValue,									// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [IN] pIRowset
		FALSE,
		DB_E_ERRORSOCCURRED);	

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required a Property with a Bad vValue value
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_30()
{
 	PROPVALS	dwValue	= BADTYPE;		// Expected Option
 	HRESULT		hr		= E_FAIL;		// HRESULT
	BOOL		fSucceed= TEST_FAIL;	// indicates variation success

	// Check to see if BOOKMARKS are supported
	if( !SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) )
		dwValue = VALIDROWSET;

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		dwValue,									// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [IN] pIRowset
		FALSE,
		DB_E_ERRORSOCCURRED);	

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SQL server specific optional columns - FOR BROWSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_31()
{
 	HRESULT		ExpHR		= S_OK;
 	HRESULT		hr			= E_FAIL;		// HRESULT
	ULONG		index		= 0;
	IRowset *	pIRowset	= NULL;
	DBID		rgDBID[1];
	ULONG		i=0;
	ULONG		j=0;

	TBEGIN;

	// Check to see if the Backend is SQL Server AND using SQL Server driver.
	if( !IsSQLServer() || !g_fKagera )
		ExpHR = DB_E_BADCOLUMNID;

	INIT;

	// Set the Command Text
	hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL,eSELECT,SELECT_ALL_WITH_FOR_BROWSE,NULL);

	// If stmt generation returned DB_E_NOTSUPPORTED then skip test.  Note since
	// we require the catalog name, if supported, then this will return NOTSUPPORTED
	// when using an ini file.
	TESTC_PROVIDER(hr != DB_E_NOTSUPPORTED);

	// Now make sure it was S_OK
	TESTC_(hr, S_OK);

	// Prepare
	TESTC_PROVIDER(SUCCEEDED(hr=PrepareCommand(m_pICommand,PREPARE,1)));

	TESTC(GetRowsetInfo(m_pICommand));
	
	TESTC_(hr=m_pICommand->QueryInterface(IID_IColumnsRowset,(void **)&m_pIColumnsRowset), S_OK);


	// Set property if Supported
	if( IsPropertySupported(DBPROPSET_ROWSET, DBPROP_IColumnsRowset) )
	{
		for(index=0; index<MAX_SQL_CA_SS; index++)
		{
			// Set the DBID contents
			rgDBID[0].uGuid.guid = GUID_NULL;
			rgDBID[0].eKind = DBKIND_GUID_PROPID;
			rgDBID[0].uName.ulPropid = g_rgCOLATTRIB_SS[index];

			// Get ColumnsRowset
			hr = m_pIColumnsRowset->GetColumnsRowset(
				NULL, 
				1, 
				rgDBID, 
				IID_IRowset, 
				0, 
				NULL, 
				(IUnknown**) &pIRowset);

			// Against non-Kagera providers we really expect this to fail
			// with DB_E_BADCOLUMNID, but since other providers may run on
			// top of Kagera it could succeed.  Don't fail them for this.
			if (SUCCEEDED(hr) && !g_fKagera)
			{
				CHECKW(hr, ExpHR);
				ExpHR = S_OK;	
			}

			// Verify results, 
			if(!COMPARE(Check_GetColumnsRowset(ExpHR,hr,1,rgDBID,pIRowset,IID_IRowset), TRUE))
				odtLog << g_rgCOLATTRIB_SS[index] << L" failed\n";

			// Release the rowset
			SAFE_RELEASE(pIRowset);
		}
	}


CLEANUP:

	FREE;

	TRETURN;
}
// }}



// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SQL server specific optional columns, COMPUTE BY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_32()
{
 	HRESULT		ExpHR		= S_OK;
 	HRESULT		hr			= E_FAIL;		// HRESULT
	BOOL		fSucceed	= TEST_FAIL;	// indicates variation success
	ULONG		index		= 0;
	IRowset *	pIRowset	= NULL;
	DBID		rgDBID[1];
	ULONG		i=0;
	ULONG		j=0;

	TBEGIN;

	// Check to see if the Backend is SQL Server AND using SQL Server driver.
	if( !IsSQLServer() || !g_fKagera )
		ExpHR = DB_E_BADCOLUMNID;

	INIT;

	// Set the Command Text
	hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL,eSELECT,SELECT_COLLISTORDERBYCOLONECOMPUTEBY,NULL);

	// If stmt generation returned DB_E_NOTSUPPORTED then skip test.  Note since
	// we require the catalog name, if supported, then this will return NOTSUPPORTED
	// when using an ini file.
	TESTC_PROVIDER(hr != DB_E_NOTSUPPORTED);

	// Now make sure it was S_OK
	TESTC_(hr, S_OK);

	// Prepare
	TESTC_PROVIDER(SUCCEEDED(hr=PrepareCommand(m_pICommand,PREPARE,1)));

	TESTC(GetRowsetInfo(m_pICommand));
	
	TESTC_(hr=m_pICommand->QueryInterface(IID_IColumnsRowset,(void **)&m_pIColumnsRowset), S_OK);

	// Set property if Supported
	if( IsPropertySupported(DBPROPSET_ROWSET, DBPROP_IColumnsRowset) )
	{
		// Set the DBID contents
		rgDBID[0].uGuid.guid = GUID_NULL;
		rgDBID[0].eKind = DBKIND_GUID_PROPID;
		rgDBID[0].uName.ulPropid = SQL_CA_SS_NUM_COMPUTES;

		// Get ColumnsRowset
		hr=m_pIColumnsRowset->GetColumnsRowset(
			NULL, 
			1, 
			rgDBID, 
			IID_IRowset, 
			0, 
			NULL, 
			(IUnknown**) &pIRowset);

		// Against non-Kagera providers we really expect this to fail
		// with DB_E_BADCOLUMNID, but since other providers may run on
		// top of Kagera it could succeed.  Don't fail them for this.
		if (SUCCEEDED(hr) && !g_fKagera)
		{
			CHECKW(hr, ExpHR);
			ExpHR = S_OK;	
		}

		// Verify results, 
		if(!COMPARE(Check_GetColumnsRowset(ExpHR,hr,1,rgDBID,pIRowset,IID_IRowset), TRUE))
			odtLog << g_rgCOLATTRIB_SS_COMPUTE[index] << L" failed\n";

		// Release the rowset
		SAFE_RELEASE(pIRowset);
	}

CLEANUP:

	FREE;

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Bookmark Property with an Insert Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Cmd_GetColumnsRowset::Variation_33()
{
 	HRESULT	ExpHR	 = S_OK;		// Expect HResult
 	HRESULT	hr		 = E_FAIL;		// HRESULT
	BOOL	fSucceed = TEST_FAIL;	// indicates variation success

	INIT;

	// Check to see if BOOKMARKS are supported or ReadOnly and Variant False
	if( (!SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE)) ||
		(!SettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) &&
		 GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommand, VARIANT_FALSE)) )
		ExpHR = DB_E_ERRORSOCCURRED;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eINSERT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		REQUIRED,						// [IN] Property dwOption
		DBPROP_BOOKMARKS,				// [IN]	Prpoerty ID
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] Skip property check
		ExpHR							// [IN] Expected HResult
	);	

	// Verify results
	if( Check_GetColumnsRowset(ExpHR,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset,eINSERT) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Oracle Specific test - FOR UPDATE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Cmd_GetColumnsRowset::Variation_34()
{ 
 	HRESULT		ExpHR		= S_OK;
 	HRESULT		hr			= E_FAIL;		// HRESULT
	ULONG		cErrors		= 0;
	IRowset *	pIRowset	= NULL;
	ITransactionLocal * pITrnLocal = NULL;

	if (!g_fOracle)
	{
		odtLog << L"FOR UPDATE is only supported against Oracle Provider.\n";
		return TEST_SKIPPED;
	}

	INIT;

	// Oracle 8 servers only allow this within a transaction
	if (!VerifyInterface(m_pThisTestModule->m_pIUnknown2, IID_ITransactionLocal, SESSION_INTERFACE,
		(IUnknown **)&pITrnLocal))
	{
		odtLog << L"FOR UPDATE needs a transaction started against Oracle 8 Servers.\n";
		return TEST_SKIPPED;
	}

	// Start a local transaction
	CHECK(pITrnLocal->StartTransaction(ISOLATIONLEVEL_READUNCOMMITTED, 0, NULL, NULL), S_OK);


	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_ALL_WITH_FOR_UPDATE,		// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		S_OK,							// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS					// Some providers require an executed state
	);

	// Verify results
	if(!Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset))
		cErrors++;

	// Release the rowset
	SAFE_RELEASE(pIRowset);

	// Commit local transaction
	CHECK(pITrnLocal->Commit(FALSE, XACTTC_SYNC_PHASETWO, 0), S_OK);

	FREE;

	SAFE_RELEASE(pITrnLocal);

	// Return results
	if( cErrors )
		return TEST_FAIL;
	else
		return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc S_OK: DBPROP_HIDDENCOLUMNS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Cmd_GetColumnsRowset::Variation_35()
{ 
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		REQUIRED,						// [IN] Property dwOption
		DBPROP_UNIQUEROWS,			// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		S_OK,							// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS,					// Some providers require an executed state
		COMMAND_INTERFACE,
		FALSE	// Don't pass property to GetColumnsRowset
	);

	if (!m_ulHiddenColumns)
		odtLog << L"Provider did not return any hidden columns.\n";

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Aggregated columns rowset on command
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Cmd_GetColumnsRowset::Variation_36()
{ 
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		S_OK,							// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS,					// Some providers require an executed state
		COMMAND_INTERFACE,				// Call IColumnsRowset off command interface
		TRUE,							// Pass props to GetColumnsRowset
		AGGREGATE						// Aggregate the result
	);

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOAGGREGATION - Request aggregation but non-IUnknown iid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Cmd_GetColumnsRowset::Variation_37()
{ 
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		DB_E_NOAGGREGATION,				// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS,					// Some providers require an executed state
		COMMAND_INTERFACE,				// Call IColumnsRowset off command interface
		TRUE,							// Pass props to GetColumnsRowset
		NOAGGREGATION					// Cause no aggregation
	);

	// Verify results
	if( Check_GetColumnsRowset(DB_E_NOAGGREGATION,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Aggregated columns rowset on command with IID_IRowsetFind
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Cmd_GetColumnsRowset::Variation_38()
{ 
 	HRESULT	ExpHR	 = S_OK;		// Expect HResult
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Check to see if IRowsetFind is supported
	if( (!SupportedProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE)) ||
		(!SettableProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) &&
		 GetProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, m_pICommand, VARIANT_FALSE)) )
		ExpHR = E_NOINTERFACE;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowsetFind,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		ExpHR,							// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS,					// Some providers require an executed state
		COMMAND_INTERFACE,				// Call IColumnsRowset off command interface
		TRUE,							// Pass props to GetColumnsRowset
		AGGREGATE						// Aggregate the result
	);

	// Verify results
	if( Check_GetColumnsRowset(ExpHR,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;

} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Cmd_GetColumnsRowset::Terminate()
{

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Row_GetAvailableColumns)
//*-----------------------------------------------------------------------
//| Test Case:		Row_GetAvailableColumns - Test case for GetAvailableColumns on Rowset
//|	Created:			02/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Row_GetAvailableColumns::Init()
{
	if (!g_fRowset)
	{
		odtLog << L"No IColumnsRowset support on rowsets\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(IColRow::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Valid Params
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetAvailableColumns::Variation_1()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	ULONG	iCol;
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	BOOL	fFound;

	INIT;

	// 

	CHECK(hr = GetAvailableColumns_row(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1), S_OK);									// [IN] property set

	// Display the optional DBCOLUMNs
	odtLog << L"Supported OPTIONAL columns:\n";
	for(ULONG index=0; index<m_cDBID; index++)
	{
		fFound = FALSE;
		for (iCol = MANCOL+1; iCol < NUMELEM(g_rgColRowInfo); iCol++)
		{
			if (CompareDBID(m_rgDBID[index],*g_rgColRowInfo[iCol].columnid))
			{
				odtLog << L"\t" << g_rgColRowInfo[iCol].pwszName << L"\n";
				fFound = TRUE;
				break;
			}
		}

		if (!fFound)
			odtLog << L"The DBID is Provider Specific\n";
	}

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Prepared Query
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetAvailableColumns::Variation_2()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns_row(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1), S_OK);									// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Prepared and Unprepared Query
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetAvailableColumns::Variation_3()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns_row(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		BOTH,										// [IN] prepared state
		1), S_OK);									// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Bookmark Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetAvailableColumns::Variation_4()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns_row(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1,
		DBPROP_BOOKMARKS), S_OK);					// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: IRowsetLocate Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetAvailableColumns::Variation_5()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns_row(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1,
		DBPROP_IRowsetLocate), S_OK);				// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: IRowsetChange Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetAvailableColumns::Variation_6()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns_row(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1,
		DBPROP_IRowsetChange), S_OK);				// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Empty Table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetAvailableColumns::Variation_7()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	ULONG	index		= 0;

	INIT;

	// Check to see if the DSO is ReadOnly
	if( !IsPropertySet(DBPROPSET_DATASOURCEINFO, DBPROP_DATASOURCEREADONLY, NULL, 0) )
	{
		// Delete all rows in the table.
		if(!CHECK(m_pTable->Delete(),S_OK))
			goto CLEANUP;
	}
	else
	{
		// Drop the table and create an empty one
		if(!CHECK(m_pTable->DropTable(),S_OK))
			goto CLEANUP;

		if(!SUCCEEDED(m_pTable->CreateTable(0,1,NULL,PRIMARY,TRUE)))
			goto CLEANUP;
	}

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns_row(
		&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1), S_OK);									// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(S_OK, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	// Cleanup the objects before the Insert
	FREE;

	// Insert all the rows in the table
	for(index=1;index<11;index++)
		m_pTable->Insert(index);

CLEANUP:

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cOptColumns == NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetAvailableColumns::Variation_8()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns_row(
		NULL,//&m_cDBID,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1), E_INVALIDARG);							// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(E_INVALIDARG, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rgOptColumns == NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetAvailableColumns::Variation_9()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetAvailableColumns_row(
		&m_cDBID,
		NULL,//&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1), E_INVALIDARG);							// [IN] property set

	// Verify results
	if( Check_GetAvailableColumns(E_INVALIDARG, hr, m_cDBID, m_rgDBID) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Row_GetAvailableColumns::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColRow::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Row_GetColumnsRowset)
//*-----------------------------------------------------------------------
//| Test Case:		Row_GetColumnsRowset - Test case for GetColumnsRowset on Rowset
//|	Created:			02/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Row_GetColumnsRowset::Init()
{
	if (!g_fRowset)
	{
		odtLog << L"No IColumnsRowset support on rowsets\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(IColRow::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Valid params
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_1()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		0,											// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [OUT] pIRowset
		FALSE,
		S_OK,
		EXECUTE_ALWAYS,
		ROWSET_INTERFACE);	

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Count==0, array == NULL, valid ppColRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_2()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		0,
		NULL,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned), S_OK);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Prepared Query
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_3()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned), S_OK);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Prepared and Unprepared Query
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_4()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		0,
		NULL,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		BOTH,							// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned), S_OK);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Bookmark Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_5()
{
 	HRESULT	ExpHR	 = S_OK;		// Expect HResult
 	HRESULT	hr		 = E_FAIL;		// HRESULT
	BOOL	fSucceed = TEST_FAIL;	// indicates variation success

	INIT;

	// Check to see if BOOKMARKS are supported or ReadOnly and Variant False
	if( (!SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE)) ||
		(!SettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) &&
		 GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommand, VARIANT_FALSE)) )
		ExpHR = DB_E_ERRORSOCCURRED;

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		REQUIRED,						// [IN] Property dwOption
		DBPROP_BOOKMARKS,				// [IN] property set
		&m_pIRowsetReturned), ExpHR);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(ExpHR,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: IRowsetLocate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_6()
{
 	HRESULT	ExpHR	 = S_OK;		// Expect HResult
 	HRESULT	hr		 = E_FAIL;		// HRESULT
	BOOL	fSucceed = TEST_FAIL;	// indicates variation success

	INIT;

	// Check to see if IRowsetLocate is supported or ReadOnly and Variant False
	if( (!SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE)) ||
		(!SettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) &&
		 GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pICommand, VARIANT_FALSE)) )
		ExpHR = DB_E_ERRORSOCCURRED;

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		REQUIRED,						// [IN] Property dwOption
		DBPROP_IRowsetLocate,			// [IN] property set
		&m_pIRowsetReturned), ExpHR);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(ExpHR,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Empty Table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_7()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	ULONG	index		= 0;

	INIT;

	// Check to see if the DSO is ReadOnly
	if( !IsPropertySet(DBPROPSET_DATASOURCEINFO, DBPROP_DATASOURCEREADONLY, NULL, 0) )
	{
		// Delete all rows in the table.
		if(!CHECK(m_pTable->Delete(),S_OK))
			goto CLEANUP;
	}
	else
	{
		// Drop the table and create an empty one
		if(!CHECK(m_pTable->DropTable(),S_OK))
			goto CLEANUP;

		if(!SUCCEEDED(m_pTable->CreateTable(0,1,NULL,PRIMARY,TRUE)))
			goto CLEANUP;
	}

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned), S_OK);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	// Cleanup the objects before the Insert
	FREE;

	// Insert all the rows in the table
	for(index=1; index<11; index++)
		m_pTable->Insert(index);

CLEANUP:

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: ppColRowset==NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_8()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		FALSE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN] Property ID
		NULL), E_INVALIDARG);			// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL rgProperySets and cPropertySets != 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_9()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Initialize the Properties
	m_cDBPROPSET = 1;

	// Set the Command Text
	if( FAILED(hr = SetCommandText(m_pIMalloc, m_pICommand, m_pTable, 
									NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL)) )
		goto CLEANUP;

	// Prepare
	if( FAILED(hr = PrepareCommand(m_pICommand, PREPARE, 1)) )
		goto CLEANUP;

	if( FAILED(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_IColumnsRowset)) )
		goto CLEANUP;

	if( !CHECK(hr=m_pICommand->Execute(NULL, IID_IRowset, 
						NULL, &m_cRowsAffected, (IUnknown **) &m_pIRowset),S_OK) )
		goto CLEANUP;

	if( !m_pIRowset )
		goto CLEANUP;

	// Get IColumnsRowset Interface off Rowset
	SAFE_RELEASE(m_pIColumnsRowset);

	if( FAILED(hr=m_pIRowset->QueryInterface(IID_IColumnsRowset, 
											(void **) &m_pIColumnsRowset)) )
		goto CLEANUP;

	CHECK(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, 0, NULL, 
			IID_IRowset, m_cDBPROPSET, NULL, PPI m_pIRowsetReturned), E_INVALIDARG);

	// Verify results
	if( Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

CLEANUP:

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL rgProperies anc cProperties != 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_10()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT

	// Initialize the Properties
	m_cDBPROPSET = 1;

	// status not set but have to check when it comes back
	m_rgDBPROPSET[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPROPSET[0].cProperties = 1;
	m_rgDBPROPSET[0].rgProperties = NULL;

	// Get IColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,							// [IN] which optional columns 
		FALSE,
		ALLDBID,								// [IN] which optional columns 
		eSELECT,								// [IN] kind of statement
		SELECT_COLLISTFROMTBL,					// [IN] sql statement
		NULL,									// [IN] client's choice for sql statement
		PREPARE,								// [IN] prepared state
		1,										// [IN] run's prepared for
		VALIDROWSET,							// [IN] Property structure
		ISOPTIONAL,								// [IN] Property dwOption
		0,										// [IN]	Prpoerty ID
		&m_pIRowsetReturned), E_INVALIDARG);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG - NULL rgProperies anc cProperties != 0 in 2nd rgPropertySet
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_11()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Initialize the Properties
	m_cDBPROPSET = 2;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPROPSET[0].cProperties = 0;
	m_rgDBPROPSET[0].rgProperties = NULL;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[1].guidPropertySet = DBPROPSET_ROWSET;
	m_rgDBPROPSET[1].cProperties = 1;
	m_rgDBPROPSET[1].rgProperties = NULL;

	// Get ColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,							// [IN] which optional columns 
		FALSE,
		ALLDBID,								// [IN] which optional columns 
		eSELECT,								// [IN] kind of statement
		SELECT_COLLISTFROMTBL,					// [IN] sql statement
		NULL,									// [IN] client's choice for sql statement
		PREPARE,								// [IN] prepared state
		1,										// [IN] run's prepared for
		VALIDROWSET,							// [IN] Property structure
		ISOPTIONAL,								// [IN] Property dwOption
		0,										// [IN]	Prpoerty ID
		&m_pIRowsetReturned), E_INVALIDARG);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE: IID_ICommand for the riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_12()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_ICommand,							// [IN] which optional columns 
		TRUE,
		ALLDBID,								// [IN] which optional columns 
		eSELECT,								// [IN] kind of statement
		SELECT_COLLISTFROMTBL,					// [IN] sql statement
		NULL,									// [IN] client's choice for sql statement
		PREPARE,								// [IN] prepared state
		1,										// [IN] run's prepared for
		VALIDROWSET,							// [IN] Property structure
		ISOPTIONAL,								// [IN] Property dwOption
		0,										// [IN] property set
		&m_pIRowsetReturned), E_NOINTERFACE);	// [IN] pIRowset
	
	// Verify results
	if( Check_GetColumnsRowset(E_NOINTERFACE,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_ICommand) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Pass some DBIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_13()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		SOMEDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned), S_OK);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Pass DBIDs in reverse order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_14()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		REVERSEDBID,					// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned), S_OK);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Pass Duplicate DBIDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_15()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		DUPLICATEDBID,					// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned), S_OK);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCOLUMNID: Invalid DBID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_16()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetAvailableColumns
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		INVALIDDBID,								// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		0,											// [IN]	Prpoerty ID
		&m_pIRowsetReturned), DB_E_BADCOLUMNID);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(DB_E_BADCOLUMNID,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Call on itself with Optional Columns passed in
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_17()
{
	
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_selfrow(
		TRUE,
		TRUE,
		IID_IRowset,						// [IN] which optional columns 
		TRUE,
		ALLDBID,							// [IN] which optional columns 
		eSELECT,							// [IN] kind of statement
		SELECT_COLLISTFROMTBL,				// [IN] sql statement
		NULL,								// [IN] client's choice for sql statement
		PREPARE,							// [IN] prepared state
		1,									// [IN] run's prepared for
		0,									// [IN]	Prpoerty ID
		&m_pIRowsetReturnedSelf), S_OK);	// [IN] pIRowset

	// Verify results
//	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturnedSelf,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Call on itself with Optional Columns NOT passed in
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_18()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_selfrow(
		TRUE,
		TRUE,
		IID_IRowset,						// [IN] which optional columns 
		TRUE,
		NONEDBID,							// [IN] which optional columns 
		eSELECT,							// [IN] kind of statement
		SELECT_COLLISTFROMTBL,				// [IN] sql statement
		NULL,								// [IN] client's choice for sql statement
		PREPARE,							// [IN] prepared state
		1,									// [IN] run's prepared for
		0,									// [IN]	Prpoerty ID
		&m_pIRowsetReturnedSelf), S_OK);	// [IN] pIRowset

	// Verify results
//	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturnedSelf,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested IRowsetChange
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_19()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		DBPROP_IRowsetChange,						// [IN] property set
		&m_pIRowsetReturned), DB_S_ERRORSOCCURRED);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested DBPROP_ACTIVESESSIONS as property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_20()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Initialize the Properties
	m_cDBPROPSET = 1;

	m_rgProperties[0].dwPropertyID = DBPROP_ACTIVESESSIONS;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_OPTIONAL;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&m_rgProperties[0].vValue) = VARIANT_TRUE;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	m_rgDBPROPSET[0].cProperties = 1;
	m_rgDBPROPSET[0].rgProperties = m_rgProperties;

	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		DBPROP_ACTIVESESSIONS,						// [IN]	Prpoerty ID
		&m_pIRowsetReturned,
		TRUE), DB_S_ERRORSOCCURRED);				// [in] skip prop check 'cause it's a DS prop

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested a Property with a Bad colid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_21()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	DBPROPID dwColPropID = 0;			// Rowset prop with COLUMNOK

	if (!(dwColPropID = FindColumnProperty(m_pThisTestModule->m_pIUnknown)))
	{
		odtLog << L"Provider does not support any DBPROPSET_ROWSET properties with DBPROPFLAGS_COLUMNOK.\n";
		return TEST_SKIPPED;
	}

	INIT;
	
	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		BADCOLID,									// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		dwColPropID,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [OUT] pIRowset
		FALSE,
		DB_S_ERRORSOCCURRED,
		EXECUTE_ALWAYS,
		ROWSET_INTERFACE);	

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: requested a Property with a Bad dwOption
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_22()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		BADOPTION,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned), DB_E_ERRORSOCCURRED);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue Type
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_23()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		BADTYPE,									// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned), DB_S_ERRORSOCCURRED);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: requested a Property with a Bad vValue value
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_24()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		BADVALUE,									// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned), DB_S_ERRORSOCCURRED);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(DB_S_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required IRowsetChange
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_25()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		DBPROP_IRowsetChange,						// [IN]	Prpoerty ID
		&m_pIRowsetReturned), DB_E_ERRORSOCCURRED);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required DBPROP_ACTIVESESSIONS as property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_26()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Initialize the Properties
	m_cDBPROPSET = 1;

	m_rgProperties[0].dwPropertyID = DBPROP_ACTIVESESSIONS;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&m_rgProperties[0].vValue) = VARIANT_TRUE;

	// Status not set but have to check when it comes back
	m_rgDBPROPSET[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	m_rgDBPROPSET[0].cProperties = 1;
	m_rgDBPROPSET[0].rgProperties = m_rgProperties;

	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		DBPROP_ACTIVESESSIONS,						// [IN]	Prpoerty ID
		&m_pIRowsetReturned,
		TRUE), DB_E_ERRORSOCCURRED);				// [in] skip prop check 'cause it's a DS prop

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required a Property with a Bad colid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_27()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	DBPROPID dwColPropID = 0;			// Rowset prop with COLUMNOK

	if (!(dwColPropID = FindColumnProperty(m_pThisTestModule->m_pIUnknown)))
	{
		odtLog << L"Provider does not support any DBPROPSET_ROWSET properties with DBPROPFLAGS_COLUMNOK.\n";
		return TEST_SKIPPED;
	}

	INIT;
	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		BADCOLID,									// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		dwColPropID,								// [IN]	Prpoerty ID
		&m_pIRowsetReturned), DB_E_ERRORSOCCURRED);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required a Property with a Bad vValue Type
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_28()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		BADTYPE,									// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned), DB_E_ERRORSOCCURRED);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: required a Property with a Bad vValue value
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_29()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;
	
	// GetColumnsRowset
	CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		BADVALUE,									// [IN] Property structure
		REQUIRED,									// [IN] Property dwOption
		DBPROP_BOOKMARKS,							// [IN]	Prpoerty ID
		&m_pIRowsetReturned), DB_E_ERRORSOCCURRED);	// [IN] pIRowset

	// Verify results
	if( Check_GetColumnsRowset(DB_E_ERRORSOCCURRED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SQL server specific optional columns - FOR BROWSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_30()
{
 	HRESULT		ExpHR		= S_OK;
 	HRESULT		hr			= E_FAIL;		// HRESULT
	BOOL		fSucceed	= TEST_FAIL;	// indicates variation success
	ULONG		index		= 0;
	IRowset *	pIRowset	= NULL;
	IColumnsRowset *	pIColumnsRowset=NULL;
	DBID		rgDBID[1];
	ULONG		i=0;
	ULONG		j=0;

	TBEGIN;

	// Check to see if the Backend is SQL Server AND using SQL Server driver.
	if( !IsSQLServer() || !g_fKagera )
		ExpHR = DB_E_BADCOLUMNID;

	INIT;

	// Set the Command Text
	hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL,eSELECT,SELECT_ALL_WITH_FOR_BROWSE,NULL);

	// If stmt generation returned DB_E_NOTSUPPORTED then skip test.  Note since
	// we require the catalog name, if supported, then this will return NOTSUPPORTED
	// when using an ini file.
	TESTC_PROVIDER(hr != DB_E_NOTSUPPORTED);

	// Now make sure it was S_OK
	TESTC_(hr, S_OK);

	TESTC_(hr=SetRowsetProperty(m_pICommand, 
									DBPROPSET_ROWSET, DBPROP_IColumnsRowset), S_OK);

		// If stmt generation returned DB_E_NOTSUPPORTED then skip test.  Note since
	// we require the catalog name, if supported, then this will return NOTSUPPORTED
	// when using an ini file.
	TESTC_PROVIDER(hr != DB_E_NOTSUPPORTED);

	// Now make sure it was S_OK
	TESTC_(hr, S_OK);

	// Server cursors cannot be used to execute SELECT statements that contain COMPUTE,
	// COMPUTE BY, FOR BROWSE, or INTO clauses.
	// Remove IRowsetChange property to allow firehose cursor.
	if (SettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE))
		TESTC_(hr=SetRowsetProperty(m_pICommand, 
									DBPROPSET_ROWSET, DBPROP_IRowsetChange, FALSE), S_OK);

	// Prepare
	if (FAILED(hr=PrepareCommand(m_pICommand,PREPARE,1)) ||
		FAILED(hr=m_pICommand->Execute(NULL,IID_IColumnsRowset,NULL,NULL,(IUnknown**)&pIColumnsRowset)))
	{
		odtLog << L"Couldn't Prepare or Execute required FOR BROWSE statement.\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	TESTC(GetRowsetInfo(pIColumnsRowset));


	for(index=0; index<MAX_SQL_CA_SS; index++)
	{
		// Set the DBID contents
		rgDBID[0].uGuid.guid = GUID_NULL;
		rgDBID[0].eKind = DBKIND_GUID_PROPID;
		rgDBID[0].uName.ulPropid = g_rgCOLATTRIB_SS[index];

		// Get ColumnsRowset
		if( !CHECK(hr = pIColumnsRowset->GetColumnsRowset(
			NULL, 
			1, 
			rgDBID, 
			IID_IRowset, 
			0, 
			NULL, 
			(IUnknown**) &pIRowset), ExpHR))
				odtLog << g_rgCOLATTRIB_SS[index] << L" failed\n";
		else
			// Verify results
			TESTC(Check_GetColumnsRowset(ExpHR,hr,1,rgDBID,pIRowset,IID_IRowset));

		// Release the rowset
		SAFE_RELEASE(pIRowset);

	}

CLEANUP:

	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pIRowset);

	FREE;

	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SQL server specific optional columns, COMPUTE BY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Row_GetColumnsRowset::Variation_31()
{
 	HRESULT		ExpHR		= S_OK;
 	HRESULT		hr			= E_FAIL;		// HRESULT
	BOOL		fSucceed	= TEST_FAIL;		// indicates variation success
	ULONG		index		= 0;
	IRowset *	pIRowset	= NULL;
	IColumnsRowset * pIColumnsRowset=NULL;
	IMultipleResults * pIMultResults=NULL;
	DBID		rgDBID[1];
	ULONG		i=0;
	ULONG		j=0;

	TBEGIN;
	
	// Check to see if the Backend is SQL Server AND using SQL Server driver.
	if( !IsSQLServer() || !g_fKagera )
		ExpHR = DB_E_BADCOLUMNID;

	INIT

	// Set the Command Text
	hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL,eSELECT,SELECT_COLLISTORDERBYCOLONECOMPUTEBY,NULL);

	// If stmt generation returned DB_E_NOTSUPPORTED then skip test.  Note since
	// we require the catalog name, if supported, then this will return NOTSUPPORTED
	// when using an ini file.
	TESTC_PROVIDER(hr != DB_E_NOTSUPPORTED);

	// Now make sure it was S_OK
	TESTC_(hr, S_OK);

	TESTC_(hr=SetRowsetProperty(m_pICommand, 
									DBPROPSET_ROWSET, DBPROP_IColumnsRowset), S_OK);

	// Server cursors cannot be used to execute SELECT statements that contain COMPUTE,
	// COMPUTE BY, FOR BROWSE, or INTO clauses.
	// Remove IRowsetChange property to allow firehose cursor.
	if (SettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE))
		TESTC_(hr=SetRowsetProperty(m_pICommand, 
									DBPROPSET_ROWSET, DBPROP_IRowsetChange, FALSE), S_OK);

	// Prepare
	if (FAILED(hr=PrepareCommand(m_pICommand,PREPARE,1)) ||
		FAILED(hr=m_pICommand->Execute(NULL,IID_IMultipleResults,NULL,NULL,(IUnknown**)&pIMultResults)))
	{
		odtLog << L"Couldn't Prepare or Execute required COMPUTE BY statement.\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	// Get the initial rowset
	TESTC_(hr=pIMultResults->GetResult(NULL, 0, IID_IColumnsRowset, NULL, (IUnknown **)&pIColumnsRowset), S_OK);

	// Release the rowset
	SAFE_RELEASE(pIColumnsRowset);

	// Get the compute by rowset
	TESTC_(hr=pIMultResults->GetResult(NULL, 0, IID_IColumnsRowset, NULL, (IUnknown **)&pIColumnsRowset), S_OK);

	TESTC(GetRowsetInfo(pIColumnsRowset));

	for(index=0; index<MAX_SQL_CA_SS_COMPUTE; index++)
	{
		// Set the DBID contents
		rgDBID[0].uGuid.guid = GUID_NULL;
		rgDBID[0].eKind = DBKIND_GUID_PROPID;
		rgDBID[0].uName.ulPropid = g_rgCOLATTRIB_SS_COMPUTE[index];

		// Get ColumnsRowset
		if( !CHECK(hr = pIColumnsRowset->GetColumnsRowset(
			NULL, 
			1, 
			rgDBID, 
			IID_IRowset, 
			0, 
			NULL, 
			(IUnknown**) &pIRowset), ExpHR))
				odtLog << g_rgCOLATTRIB_SS_COMPUTE[index] << L" failed\n";

		// TODO: Verify the results of the COMPUTE BY clause.  It doesn't match typical
		// expected values and so needs a custom verify routine.

		// Release the rowset
		SAFE_RELEASE(pIRowset);

	}

CLEANUP:

	SAFE_RELEASE(pIMultResults);
	SAFE_RELEASE(pIColumnsRowset);

	FREE;

	// Return results
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Oracle Specific test - FOR UPDATE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Row_GetColumnsRowset::Variation_32()
{ 
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	if (!g_fOracle)
	{
		odtLog << L"FOR UPDATE is only supported against Oracle Provider.\n";
		return TEST_SKIPPED;
	}

	INIT;

	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_ALL_WITH_FOR_UPDATE,					// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		0,											// [IN]	Prpoerty ID
		&m_pIRowsetReturned,						// [OUT] pIRowset
		FALSE,
		S_OK,
		EXECUTE_ALWAYS,
		ROWSET_INTERFACE);	

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc S_OK: DBPROP_HIDDENCOLUMNS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Row_GetColumnsRowset::Variation_33()
{ 
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		REQUIRED,						// [IN] Property dwOption
		DBPROP_UNIQUEROWS,			// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		S_OK,							// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS,					// Some providers require an executed state
		ROWSET_INTERFACE,
		FALSE	// Don't pass property to GetColumnsRowset
	);

	if (!m_ulHiddenColumns)
		odtLog << L"Provider did not return any hidden columns.\n";

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Aggregated columns rowset on rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Row_GetColumnsRowset::Variation_34()
{ 
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] interface on GetRowset
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		S_OK,							// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS,					// Some providers require an executed state
		ROWSET_INTERFACE,				// Call IColumnsRowset off rowset interface
		TRUE,							// Pass props to GetColumnsRowset
		AGGREGATE,						// Aggregate the result
		IID_IRowset						// interface on Execute
	);

	// Verify results
	if( Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOAGGREGATION - Request aggregation but non-IUnknown iid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Row_GetColumnsRowset::Variation_35()
{ 
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowsetFind,				// [IN] which optional columns 
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		DB_E_NOAGGREGATION,				// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS,					// Some providers require an executed state
		ROWSET_INTERFACE,				// Call IColumnsRowset off rowset interface
		TRUE,							// Pass props to GetColumnsRowset
		NOAGGREGATION					// Cause no aggregation
	);

	// Verify results
	if( Check_GetColumnsRowset(DB_E_NOAGGREGATION,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Aggregated columns rowset on rowset with IID_IRowsetFind
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Row_GetColumnsRowset::Variation_36()
{ 
 	HRESULT	ExpHR	 = S_OK;		// Expect HResult
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Check to see if BOOKMARKS are supported or ReadOnly and Variant False
	if( (!SupportedProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE)) ||
		(!SettableProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) &&
		 GetProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, m_pICommand, VARIANT_FALSE)) )
		ExpHR = E_NOINTERFACE;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowsetFind,				// [IN] interface on GetRowset
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		ExpHR,							// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS,					// Some providers require an executed state
		ROWSET_INTERFACE,				// Call IColumnsRowset off rowset interface
		TRUE,							// Pass props to GetColumnsRowset
		AGGREGATE,						// Aggregate the result
		IID_IRowsetFind					// interface on Execute
	);

	// Verify results
	if( Check_GetColumnsRowset(ExpHR,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOAGGREGATION: Execute with IID_IRowsetFind, aggregate GetColumnsRowset with IID_IRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Row_GetColumnsRowset::Variation_37()
{ 
 	HRESULT	ExpHR		= DB_E_NOAGGREGATION;		// Expect HResult
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success

	INIT;

	// Check to see if BOOKMARKS are supported or ReadOnly and Variant False
	if( (!SupportedProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE)) ||
		(!SettableProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE) &&
		 GetProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, m_pICommand, VARIANT_FALSE)) )
		ExpHR = E_NOINTERFACE;

	// GetColumnsRowset
	hr = GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,					// [IN] interface on GetRowset
		TRUE,
		ALLDBID,						// [IN] which optional columns 
		eSELECT,						// [IN] kind of statement
		SELECT_COLLISTFROMTBL,			// [IN] sql statement
		NULL,							// [IN] client's choice for sql statement
		PREPARE,						// [IN] prepared state
		1,								// [IN] run's prepared for
		VALIDROWSET,					// [IN] Property structure
		ISOPTIONAL,						// [IN] Property dwOption
		0,								// [IN]	Prpoerty ID.  
		&m_pIRowsetReturned,			// [IN] pIRowset
		FALSE,							// [IN] if not rowset property, skip it
		ExpHR,							// [IN] hr to expect from GetColumnsRowset
		EXECUTE_ALWAYS,					// Some providers require an executed state
		ROWSET_INTERFACE,				// Call IColumnsRowset off rowset interface
		TRUE,							// Pass props to GetColumnsRowset
		NOAGGREGATION,					// Aggregate with non-IUknown
		IID_IRowsetFind					// interface on Execute
	);

	// Verify results
	if( Check_GetColumnsRowset(ExpHR,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
		fSucceed = TEST_PASS;

	FREE;

	return fSucceed;

} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Row_GetColumnsRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColRow::Terminate());
}	// }}
// }}
// }}


// }}
// {{ TCW_TC_PROTOTYPE(CZombie)
//*-----------------------------------------------------------------------
//| Test Case:		CZombie - Induce zombie states on the Command
//|	Created:			02/02/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CZombie::Init()
{
	if (!g_fCmd)
	{
		odtLog << L"No IColumnsRowset support on commands\n";
		return TEST_SKIPPED;
	}

	// Initialize the Transaction Class
	if( !CTransaction::Init() )
		return TEST_SKIPPED;
	
	//This is a optional interface, it should always be checked
	return RegisterInterface(COMMAND_INTERFACE, IID_IColumnsRowset, 0, NULL);
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_1()
{
	return TestTxn(EMETHOD_AVAILCOL, ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_2()
{
	return TestTxn(EMETHOD_AVAILCOL, ETXN_ABORT, FALSE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_3()
{
	return TestTxn(EMETHOD_AVAILCOL, ETXN_COMMIT, TRUE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_4()
{
	return TestTxn(EMETHOD_AVAILCOL, ETXN_COMMIT, FALSE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_5()
{
	return TestTxn(EMETHOD_COLROWSET, ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_6()
{
	return TestTxn(EMETHOD_COLROWSET, ETXN_ABORT, FALSE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_7()
{
	return TestTxn(EMETHOD_COLROWSET, ETXN_COMMIT, TRUE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::Variation_8()
{
	return TestTxn(EMETHOD_COLROWSET, ETXN_COMMIT, FALSE);
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CZombie::Terminate()
{
	// Cleanup the Transaction Class
	return(CTransaction::Terminate());
}	// }}


//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort with respect to IColumnsRowset on commands
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CZombie::TestTxn
(
	EMETHOD eMethod,
	ETXN	eTxn,
	BOOL	fRetaining
)
{
	BOOL			fSuccess		= FALSE;
	HRESULT			hr				= E_FAIL;
	IColumnsRowset* pIColumnsRowset	= NULL;
	DBORDINAL		cOptColumns		= 0;
	DBID *			rgOptColumns	= NULL;
	const IID		iid				= IID_IRowset;
	IRowset *		pIRowset		= NULL;
	DBORDINAL		cRowsObtained	= 0;
	HROW *			rghRows			= NULL;		
	
	// Start a Transaction
	if( !StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIColumnsRowset, 
							0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE) )
		goto CLEANUP;
		
	// Set the Prepare behavior flags
	PrepareBehavior();

	// Commit or Abort the Transaction
	if( eTxn == ETXN_COMMIT )
	{
		if( !GetCommit(fRetaining) )
			goto CLEANUP;
	}
	else
	{
		if( !GetAbort(fRetaining) )
			goto CLEANUP;
	}

	// Switch on the 2 methods for IColumnsRowset
	// Since we're on a command object PrepCommitPreserve and
	// PrepAbortPreserve is significant, CommitPreserve and
	// AbortPreserve play no role for command objects.
	if( eMethod == EMETHOD_AVAILCOL )
	{
		// Commit or Abort can affect the State of the Command
		if( (eTxn == ETXN_COMMIT && !m_fPrepCommitPreserve) ||
			(eTxn == ETXN_ABORT && !m_fPrepAbortPreserve) )
		{
			CHECK(pIColumnsRowset->GetAvailableColumns(&cOptColumns, 
										&rgOptColumns), DB_E_NOTPREPARED);
			COMPARE(cOptColumns, 0);
			COMPARE(rgOptColumns, NULL);
		}
		else
		{
			CHECK(pIColumnsRowset->GetAvailableColumns(
									&cOptColumns,&rgOptColumns), S_OK);
		}
	}
	else
	{
		// Commit or Abort can affect the State of the Command
		if((eTxn == ETXN_COMMIT && !m_fPrepCommitPreserve) ||
			(eTxn == ETXN_ABORT && !m_fPrepAbortPreserve))
		{
			CHECK(pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, rgOptColumns,
							iid, 0, NULL, (IUnknown **) &pIRowset), DB_E_NOTPREPARED);
			
			COMPARE(cOptColumns, 0);
			COMPARE(rgOptColumns, NULL);
			COMPARE(pIRowset, NULL);
		}
		else
		{
			CHECK(pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, 
					rgOptColumns, iid, 0, NULL, (IUnknown **) &pIRowset), S_OK);

			// Check the Rowset that was generated
			if( pIRowset )
				hr = pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows);
			
			// Check the HRESULT and cRowsObtained
			if(hr == S_OK)
			{
				COMPARE(cRowsObtained, 1);
				CHECK(m_hr=pIRowset->ReleaseRows(cRowsObtained,rghRows,NULL,NULL,NULL),S_OK);
			}
			else if(hr == DB_S_ENDOFROWSET)
				COMPARE(cRowsObtained, 0);
			else
				goto CLEANUP;
		}
	}
	
	// Everything worked correctly
	fSuccess = TRUE;

CLEANUP:

	// Release the Rowsets
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIColumnsRowset);

	if( rgOptColumns )
		PROVIDER_FREE(rgOptColumns);

	if( rghRows )
		PROVIDER_FREE(rghRows);

	// Return code of Commit/Abort will vary depending on whether
	// or not we have an open txn, so adjust accordingly
	if( fRetaining )
		CleanUpTransaction(S_OK);
	else
		CleanUpTransaction(XACT_E_NOTRANSACTION);
	
	if( fSuccess )
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc PrepareBehavior
// Figues out if the command is left prepared after a commit or an abort
//
// @rdesc TEST_PASS or TEST_FAIL
//
void CZombie::PrepareBehavior()
{
	ULONG		ulIndex				= 0;
	ULONG		cPropertyIDSets		= 0;
	DBPROPIDSET	rgPropertyIDSets[1];
	DBPROPID 	rgPropertyIDs		= NULL;
	ULONG 		cPropertySets		= 0;
	DBPROPSET *	rgPropertySets		= NULL;

	// Set the defualts
	m_fPrepAbortPreserve =  FALSE;
	m_fPrepCommitPreserve = FALSE;

	// If m_pIDBCreateSession is NULL, can't get a IDBProperties pointer
	if( !m_pIDBCreateSession )
		goto DEFAULT;

	// Queryinterface for IDBProperties pointer
	if( !CHECK(m_pIDBCreateSession->QueryInterface(IID_IDBProperties, 
		(LPVOID *)&m_pIDBProperties),S_OK) )
		goto DEFAULT;

	// Ask for DBPROP_PREPAREABORTBEHAVIOR
	cPropertyIDSets = 1;
	rgPropertyIDs = DBPROP_PREPAREABORTBEHAVIOR;
	rgPropertyIDSets[0].rgPropertyIDs = &rgPropertyIDs;
	rgPropertyIDSets[0].cPropertyIDs = 1;
	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;

	if(!CHECK(m_pIDBProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets,
		&cPropertySets, &rgPropertySets),S_OK))
		goto DEFAULT;

	if( rgPropertySets->rgProperties->vValue.lVal == DBPROPVAL_CB_PRESERVE )
		m_fPrepAbortPreserve = TRUE;

	// Free rgPropertySets
	if( rgPropertySets )
	{
		//Clean up our variants we used in the init
		for(ulIndex=0; ulIndex<rgPropertySets[0].cProperties; ulIndex++)
			VariantClear(&rgPropertySets[0].rgProperties[ulIndex].vValue);		
	
		PROVIDER_FREE(rgPropertySets[0].rgProperties);	
		PROVIDER_FREE(rgPropertySets);	
		rgPropertySets = NULL;
	}

  	// Ask for DBPROP_PREPARECOMMITBEHAVIOR
	cPropertyIDSets = 1;
	rgPropertyIDs = DBPROP_PREPARECOMMITBEHAVIOR;
	rgPropertyIDSets[0].rgPropertyIDs = &rgPropertyIDs;
	rgPropertyIDSets[0].cPropertyIDs = 1;
	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;

	if( !CHECK(m_pIDBProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets,
		&cPropertySets, &rgPropertySets),S_OK) )
		goto DEFAULT;

	if( rgPropertySets->rgProperties->vValue.lVal == DBPROPVAL_CB_PRESERVE )
		m_fPrepCommitPreserve = TRUE;

	// Free rgPropertySets
	if( rgPropertySets )
	{
		// Clean up our variants we used in the init
		for(ulIndex=0; ulIndex<rgPropertySets[0].cProperties; ulIndex++)
			VariantClear(&rgPropertySets[0].rgProperties[ulIndex].vValue);		
	
		PROVIDER_FREE(rgPropertySets[0].rgProperties);	
		PROVIDER_FREE(rgPropertySets);	
		rgPropertySets = NULL;
	}

DEFAULT:
	
	// Release the Property Object
	SAFE_RELEASE(m_pIDBProperties);
}


// {{ TCW_TC_PROTOTYPE(RZombie)
//*-----------------------------------------------------------------------
//| Test Case:		RZombie - Induce zombie states on the Rowset
//|	Created:		02/02/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RZombie::Init()
{
	// Initialize vars
	memset(&m_rgPropertySets, 0, sizeof(DBPROPSET));

	// Check to see if supported on the rowset
	if (!g_fRowset)
	{
		odtLog << L"No IColumnsRowset support on rowsets\n";
		return TEST_SKIPPED;
	}

	// Initialize the Transaction Class
	if(!CTransaction::Init())
		return TEST_SKIPPED;

	// Set DBPROP_IColumnsRowset Property
	m_rgPropertySets.guidPropertySet=DBPROPSET_ROWSET;
	m_rgPropertySets.cProperties=1;
	m_rgPropertySets.rgProperties=(DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP));

	m_rgPropertySets.rgProperties[0].dwPropertyID=DBPROP_IColumnsRowset;
   	m_rgPropertySets.rgProperties[0].dwOptions=DBPROPOPTIONS_REQUIRED;                                              
   	m_rgPropertySets.rgProperties[0].vValue.vt=VT_BOOL;                                        
   	V_BOOL(&m_rgPropertySets.rgProperties[0].vValue)=VARIANT_TRUE; 
		
	//This is a optional interface, it should always be checked
	return RegisterInterface(ROWSET_INTERFACE, IID_IColumnsRowset, 1, &m_rgPropertySets);
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RZombie::Variation_1()
{
	return TestTxn(EMETHOD_AVAILCOL, ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IColumnsRowset::GetAvailableColumns with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RZombie::Variation_2()
{
	return TestTxn(EMETHOD_AVAILCOL, ETXN_ABORT, FALSE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RZombie::Variation_3()
{
	return TestTxn(EMETHOD_AVAILCOL, ETXN_COMMIT, TRUE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IColumnsRowset::GetAvailableColumns with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RZombie::Variation_4()
{
	return TestTxn(EMETHOD_AVAILCOL, ETXN_COMMIT, FALSE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RZombie::Variation_5()
{
	return TestTxn(EMETHOD_COLROWSET, ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IColumnsRowset::GetColumnsRowset with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RZombie::Variation_6()
{
	return TestTxn(EMETHOD_COLROWSET, ETXN_ABORT, FALSE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RZombie::Variation_7()
{
	return TestTxn(EMETHOD_COLROWSET, ETXN_COMMIT, TRUE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IColumnsRowset::GetColumnsRowset with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RZombie::Variation_8()
{
	return TestTxn(EMETHOD_COLROWSET, ETXN_COMMIT, FALSE);
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL RZombie::Terminate()
{
	// Free
	PROVIDER_FREE(m_rgPropertySets.rgProperties);

	// Cleanup the Transaction Class
	return(CTransaction::Terminate());
}	// }}


//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort with respect to IColumnsRowset on commands
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RZombie::TestTxn
(
	EMETHOD eMethod,
	ETXN	eTxn,
	BOOL	fRetaining
)
{
	BOOL			fSuccess		= FALSE;
	HRESULT			hr				= E_FAIL;
	IColumnsRowset*	pIColumnsRowset	= NULL;
	DBORDINAL		cOptColumns		= 0;
	DBID *			rgOptColumns	= NULL;
	const IID		iid				= IID_IRowset;
	IRowset *		pIRowset		= NULL;
	ULONG 			cRowsObtained	= 0;
	HROW *			rghRows			= NULL;		
	
	// Start the Transaction
	if( !StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIColumnsRowset,
														1, &m_rgPropertySets) )
		goto CLEANUP;
		
	// Commit or Abort the Transaction
	if( eTxn == ETXN_COMMIT )
	{
		if( !GetCommit(fRetaining) )
			goto CLEANUP;
	}
	else
	{
		if( !GetAbort(fRetaining) )
			goto CLEANUP;
	}

	// Make sure everything still works after commit or abort
	// Since we're on a rowset object the rowset CommitPreserve and
	// AbortPreserve are significant, the PrepCommitPreserve and
	// PrepAbortPreserve play no role.

	// Switch on the 2 methods for IColumnsRowset
	if( eMethod == EMETHOD_AVAILCOL )
	{
		if( (eTxn == ETXN_COMMIT && m_fCommitPreserve) ||
			(eTxn == ETXN_ABORT && m_fAbortPreserve) )
		{	
			CHECK(pIColumnsRowset->GetAvailableColumns(
									&cOptColumns,&rgOptColumns), S_OK);
		}
		else
		{
			CHECK(pIColumnsRowset->GetAvailableColumns(
									&cOptColumns,&rgOptColumns), E_UNEXPECTED);
			COMPARE(cOptColumns, 0);
			COMPARE(rgOptColumns, NULL);
		}

	}
	else
	{
		if( (eTxn == ETXN_COMMIT && m_fCommitPreserve) ||
			(eTxn == ETXN_ABORT && m_fAbortPreserve) )
		{	
			CHECK(pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, 
					rgOptColumns, iid, 0, NULL, (IUnknown **) &pIRowset), S_OK);
		}
		else
		{
			CHECK(pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, 
					rgOptColumns, iid, 0, NULL, (IUnknown **) &pIRowset), E_UNEXPECTED);

			COMPARE(cOptColumns, 0);
			COMPARE(rgOptColumns, NULL);
			COMPARE(pIRowset, NULL);
		}
	}
	
	// Everything worked correctly
	fSuccess = TRUE;

CLEANUP:

	// Release the Rowsets
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIColumnsRowset);

	if( rgOptColumns )
		PROVIDER_FREE(rgOptColumns);

	// Return code of Commit/Abort will vary depending on whether
	// or not we have an open txn, so adjust accordingly
	if( fRetaining )
		CleanUpTransaction(S_OK);
	else
		CleanUpTransaction(XACT_E_NOTRANSACTION);
	
	if( fSuccess )
		return TEST_PASS;
	else
		return TEST_FAIL;
}


// {{ TCW_TC_PROTOTYPE(ExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		ExtendedErrors - Extended Errors
//|	Created:			07/10/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL ExtendedErrors::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(IColRow::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid Cmd_IColumnsRowset calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_1()
{
   	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_FAIL;	// indicates variation success
	DBORDINAL cOptColumns	= 0;
	DBID *	rgOptColumns= NULL;

   	// For each method of the interface, first create an error object on
	// the current thread, then try get S_OK from the Cmd_IColumnsRowset method.
	// We then check extended errors to verify nothing is set since an 
	// error object shouldn't exist following a successful call.

	if (!g_fCmd)
	{
		odtLog << L"No IColumnsRowset support on commands\n";
		return TEST_SKIPPED;
	}

	INIT;

	// Set the Command Text
	if( FAILED(hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL)) )
		return TEST_FAIL;

	// Prepare the Command
	if( FAILED(hr=PrepareCommand(m_pICommand, PREPARE, 1)) )
		return TEST_FAIL;

	TESTC(GetRowsetInfo(m_pICommand));

	// Make sure it is alive
	if( !m_pIColumnsRowset )
	{
		if( FAILED(hr=m_pICommand->QueryInterface(IID_IColumnsRowset,
												 (void **)&m_pIColumnsRowset)) )
			return TEST_FAIL;
	}

	// Create an error object
	m_pExtError->CauseError();
	
	// GetAvailableColumns
	if( CHECK(hr = m_pIColumnsRowset->GetAvailableColumns(&m_cDBID, &m_rgDBID), S_OK) )

	// Do extended check following GetAvailableColumns
	fSucceed = XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);
	
	// Verify results
	Check_GetAvailableColumns(S_OK,hr,m_cDBID,m_rgDBID);

	FREE;
	
	INIT;

	// Set the Command Text
	if( FAILED(hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable, 
								 NULL, eSELECT, SELECT_COLLISTFROMTBL,NULL)) )
		return TEST_FAIL;

	// Prepare the Command
	if( FAILED(hr=PrepareCommand(m_pICommand,PREPARE,1)) )
		return TEST_FAIL;

	TESTC(GetRowsetInfo(m_pICommand));

	// Make sure it is alive
	if( !m_pIColumnsRowset )
	{
		if( FAILED(hr=m_pICommand->QueryInterface(IID_IColumnsRowset,
												 (void **)&m_pIColumnsRowset)) )
			return TEST_FAIL;
	}
	
	// Arrange option columns
	if( !ArrangeOptionalColumns(ALLDBID) )
		return TEST_FAIL;

	cOptColumns  = m_cDBID;
	rgOptColumns = m_rgDBID;

	// Create an error object
	m_pExtError->CauseError();
	
	// Get ColumnsRowset
	if( CHECK(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, rgOptColumns,
				IID_IRowset, m_cDBPROPSET, m_rgDBPROPSET, PPI &m_pIRowsetReturned), S_OK) )
		fSucceed &= XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);	
	
	FreeOptionalColumns();

	// Verify results
	Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset);

CLEANUP:

	FREE;

	// Return the results
	if( fSucceed )
		return TEST_PASS;
	else
		return TEST_FAIL; 
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid Cmd_IColumnsRowset calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_2()
{
	HRESULT	hr			= E_FAIL;								
	BOOL	fSucceed	= TEST_FAIL;
	DBORDINAL cOptColumns = 0;
	DBID *	rgOptColumns= NULL;

   	// For each method of the interface, first create an error object on
	// the current thread, then try get a failure from the Cmd_IColumnsRowset method.
	// We then check extended errors to verify the right extended error behavior.

	if (!g_fCmd)
	{
		odtLog << L"No IColumnsRowset support on commands\n";
		return TEST_SKIPPED;
	}

	INIT;

	// Set the Command Text
	if( FAILED(hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL)) )
		return TEST_FAIL;

	// Prepare the Command
	if( FAILED(hr=PrepareCommand(m_pICommand, PREPARE, 1)) )
		return TEST_FAIL;

	TESTC(GetRowsetInfo(m_pICommand));

	// Make sure it is alive
	if( !m_pIColumnsRowset )
	{
		if( FAILED(hr=m_pICommand->QueryInterface(IID_IColumnsRowset,
												 (void **)&m_pIColumnsRowset)) )
			return TEST_FAIL;
	}

	// Create an error object
	m_pExtError->CauseError();
	
	// GetAvailableColumns
	if( CHECK(hr = m_pIColumnsRowset->GetAvailableColumns(NULL, &m_rgDBID), E_INVALIDARG) )
		fSucceed = XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);
	
	// Verify results
	Check_GetAvailableColumns(E_INVALIDARG, hr, m_cDBID, m_rgDBID);
	
	FREE;
	
	INIT;

	// Set the Command Text
	if( FAILED(hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL)) )
		return TEST_FAIL;

	// Prepare the Command
	if( FAILED(hr=PrepareCommand(m_pICommand, PREPARE, 1)) )
		return TEST_FAIL;

	TESTC(GetRowsetInfo(m_pICommand));

	// Make sure it is alive
	if( !m_pIColumnsRowset )
	{
		if( FAILED(hr=m_pICommand->QueryInterface(IID_IColumnsRowset,
												 (void **)&m_pIColumnsRowset)) )
			return TEST_FAIL;
	}
	
	// Arrange option columns
	if( !ArrangeOptionalColumns(ALLDBID) )
		return TEST_FAIL;

	cOptColumns  = m_cDBID;
	rgOptColumns = m_rgDBID;

	// Create an error object
	m_pExtError->CauseError();
	
	// Get ColumnsRowset
	if( CHECK(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, rgOptColumns, 
						IID_IRowset, m_cDBPROPSET, m_rgDBPROPSET, NULL), E_INVALIDARG) )
		fSucceed &= XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);	
	
	FreeOptionalColumns();
	
	// Verify results
	Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset);

CLEANUP:

	FREE;

	// Return the results
	if( fSucceed )
		return TEST_PASS;
	else
		return TEST_FAIL; 
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid Cmd_IColumnsRowset calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_3()
{
	HRESULT	hr			= E_FAIL;					
	BOOL	fSucceed	= TEST_FAIL;
   
	// For each method of the interface, with no error object on
	// the current thread, try get a failure from the Cmd_IColumnsRowset method.
	// We then check extended errors to verify the right extended error behavior.

	if (!g_fCmd)
	{
		odtLog << L"No IColumnsRowset support on commands\n";
		return TEST_SKIPPED;
	}

	INIT;

	if( !CHECK(hr = GetAvailableColumns(
		NULL,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1), E_INVALIDARG) )							// [IN] property set
	{
		FREE;
		goto CLEANUP;
	}

	// Do extended check following GetAvailableColumns
	fSucceed = XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);
	
	// Verify results
	Check_GetAvailableColumns(E_INVALIDARG, hr, m_cDBID, m_rgDBID);

	FREE;
	
	INIT;

	// GetColumnsRowset
	if( !GetColumnsRowset(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		FALSE,
		ALLDBID,									// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		0,											// [IN]	Prpoerty ID
		NULL,										// [IN] pIRowset
		FALSE,
		E_INVALIDARG) )						
	{
		FREE;
		goto CLEANUP;
	}
	
	// Do extended check following GetColumnsRowset
	fSucceed &= XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);	
	
	// Verify results
	Check_GetColumnsRowset(E_INVALIDARG,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset);

	FREE;

CLEANUP:

	// Return the results
	if( fSucceed )
		return TEST_PASS;
	else
		return TEST_FAIL; 
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid Row_IColumnsRowset calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_4()
{
   	HRESULT	hr;						// HRESULT
	BOOL		fSucceed=FALSE;		// indicates variation success
	DBORDINAL	cOptColumns=0;
	DBID *		rgOptColumns=NULL;

   	// For each method of the interface, first create an error object on
	// the current thread, then try get S_OK from the Row_IColumnsRowset method.
	// We then check extended errors to verify nothing is set since an 
	// error object shouldn't exist following a successful call.

	if (!g_fRowset)
	{
		odtLog << L"No IColumnsRowset support on rowsets\n";
		return TEST_SKIPPED;
	}

	INIT;
														
	// Set the Command Text
	TESTC_(hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL), S_OK);

	TESTC_(hr=SetRowsetProperty(m_pICommand, 
									DBPROPSET_ROWSET, DBPROP_IColumnsRowset), S_OK);

	// Prepare the Command
	TESTC_(hr=PrepareCommand(m_pICommand, NEITHER, 1), S_OK);

	TESTC_(hr=m_pICommand->Execute(NULL, IID_IRowset, NULL, 
									   &m_cRowsAffected, (IUnknown **)&m_pIRowset), S_OK);

	TESTC(GetRowsetInfo(m_pIRowset));

	// Get IColumnsRowset Interface off Rowset
	SAFE_RELEASE(m_pIColumnsRowset);
	
	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsRowset, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsRowset));

	// Create an error object
	m_pExtError->CauseError();

	// Get ColumnsRowset
	TESTC_(hr = m_pIColumnsRowset->GetAvailableColumns(&m_cDBID, &m_rgDBID), S_OK);

	TESTC(XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr));

	// Verify results
	TESTC(Check_GetAvailableColumns(S_OK,hr,m_cDBID,m_rgDBID));

	FREE;
	
	INIT;

	// Set the Command Text
	TESTC_(hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL), S_OK);

	// Prepare the Command
	TESTC_(hr=PrepareCommand(m_pICommand, PREPARE, 1), S_OK);

	// See if Bookmarks are on turned on
	if( IsPropertySet(DBPROPSET_ROWSET, DBPROP_BOOKMARKS, NULL, 0) )
		m_fOriginalRowsetHasBookmark=TRUE;

	// Set property if Supported
	if(IsPropertySupported(DBPROPSET_ROWSET, DBPROP_IColumnsRowset))
	{
		TESTC_(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_IColumnsRowset), S_OK);
	}
	else
		TESTC_(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, 
									DBPROP_IColumnsRowset),DB_S_ERRORSOCCURRED);

	TESTC_(hr=m_pICommand->Execute(NULL, IID_IRowset, 
						NULL, &m_cRowsAffected, (IUnknown **) &m_pIRowset),S_OK);

	TESTC(GetRowsetInfo(m_pIRowset));

	// Get IColumnsRowset Interface off Rowset
	SAFE_RELEASE(m_pIColumnsRowset);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsRowset, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsRowset));

	// Arrange option columns
	TESTC(ArrangeOptionalColumns(ALLDBID));

	cOptColumns  = m_cDBID;
	rgOptColumns = m_rgDBID;

	// Create an error object
	m_pExtError->CauseError();
	
	// Get ColumnsRowset
	TESTC_(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, 
			rgOptColumns, IID_IRowset, m_cDBPROPSET, m_rgDBPROPSET, PPI &m_pIRowsetReturned), S_OK);

	TESTC(XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr));	
	
	FreeOptionalColumns();

	// Verify results
	TESTC(Check_GetColumnsRowset(S_OK,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset));

	fSucceed = TRUE;

CLEANUP:

	FREE;

	// Return the results
	return fSucceed ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Invalid Row_IColumnsRowset calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_5()
{
	HRESULT	hr;						// HRESULT
	BOOL	fSucceed=FALSE;		// indicates variation success
	DBORDINAL	cOptColumns=0;
	DBID *		rgOptColumns=NULL;

   	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the Row_IColumnsRowset method.
	//We then check extended errors to verify the right extended error behavior.

	if (!g_fRowset)
	{
		odtLog << L"No IColumnsRowset support on rowsets\n";
		return TEST_SKIPPED;
	}

	INIT;

	// Set the Command Text
	TESTC_(hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL), S_OK);

	// Request IColumnsRowset property
	TESTC_(hr=SetRowsetProperty(m_pICommand, 
										DBPROPSET_ROWSET, DBPROP_IColumnsRowset), S_OK);

	// Prepare the Command
	TESTC_(hr=PrepareCommand(m_pICommand, NEITHER, 1), S_OK);

	// See if Bookmarks are on turned on
	if( IsPropertySet(DBPROPSET_ROWSET, DBPROP_BOOKMARKS, NULL, 0) )
		m_fOriginalRowsetHasBookmark=TRUE;

	TESTC_(hr=m_pICommand->Execute(NULL, IID_IRowset, NULL, 
									   &m_cRowsAffected, (IUnknown **)&m_pIRowset), S_OK);
	TESTC(GetRowsetInfo(m_pIRowset));
	
	// Get IColumnsRowset Interface off Rowset
	SAFE_RELEASE(m_pIColumnsRowset);
	
	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsRowset, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsRowset));

	// Create an error object
	m_pExtError->CauseError();

	// Get ColumnsRowset
	TESTC_(hr = m_pIColumnsRowset->GetAvailableColumns(NULL, &m_rgDBID), E_INVALIDARG);

	TESTC(XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr));

	// Verify results
	TESTC(Check_GetAvailableColumns(E_INVALIDARG, hr, m_cDBID, m_rgDBID));

	FREE;
	
	INIT;

	// Set the Command Text
	TESTC_(hr=SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
								 NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL), S_OK);

	// Prepare the Command
	TESTC_(hr=PrepareCommand(m_pICommand,PREPARE,1), S_OK);

	// See if Bookmarks are on turned on
	if( IsPropertySet(DBPROPSET_ROWSET, DBPROP_BOOKMARKS, NULL, 0) )
		m_fOriginalRowsetHasBookmark=TRUE;

	// Set property if Supported
	if( IsPropertySupported(DBPROPSET_ROWSET, DBPROP_IColumnsRowset) )
	{
		TESTC_(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_IColumnsRowset), S_OK);
	}
	else
		TESTC_(hr=SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, 
									DBPROP_IColumnsRowset),DB_S_ERRORSOCCURRED);

	TESTC_(hr=m_pICommand->Execute(NULL, IID_IRowset, 
						NULL, &m_cRowsAffected, (IUnknown **) &m_pIRowset), S_OK);

	TESTC(GetRowsetInfo(m_pIRowset));

	// Get IColumnsRowset Interface off Rowset
	SAFE_RELEASE(m_pIColumnsRowset);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsRowset, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsRowset));

	TESTC(ArrangeOptionalColumns(INVALIDDBID));

	cOptColumns  = m_cDBID;
	rgOptColumns = m_rgDBID;
	
	// Create an error object
	m_pExtError->CauseError();
	
	// Get ColumnsRowset
	TESTC_(hr = m_pIColumnsRowset->GetColumnsRowset(NULL, cOptColumns, 
			rgOptColumns, IID_IRowset, m_cDBPROPSET, m_rgDBPROPSET, PPI &m_pIRowsetReturned), DB_E_BADCOLUMNID);

	TESTC(XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr));	

	FreeOptionalColumns();

	// Verify results
	TESTC(Check_GetColumnsRowset(DB_E_BADCOLUMNID,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset));

	fSucceed = TRUE;

CLEANUP:

	FREE;

	// Return the results
	return fSucceed ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid Row_IColumnsRowset calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_6()
{
  	HRESULT	hr			= E_FAIL;			// HRESULT
	BOOL	fSucceed	= TEST_FAIL;		// indicates variation success

 	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the Row_IColumnsRowset method.
	//We then check extended errors to verify the right extended error behavior.

	if (!g_fRowset)
	{
		odtLog << L"No IColumnsRowset support on rowsets\n";
		return TEST_SKIPPED;
	}

	INIT;

	// GetAvailableColumns
	if( !CHECK(hr = GetAvailableColumns_row(
		NULL,
		&m_rgDBID,
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		NEITHER,									// [IN] prepared state
		1),E_INVALIDARG) )							// [IN] property set
	{
		FREE;
		goto CLEANUP;
	}
	
	// Do extended check following GetAvailableColumns
	fSucceed = XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);

	// Verify results
	Check_GetAvailableColumns(E_INVALIDARG, hr, m_cDBID, m_rgDBID);

	FREE;
	
	INIT;

	if( !CHECK(hr = GetColumnsRowset_row(
		TRUE,
		TRUE,
		IID_IRowset,								// [IN] which optional columns 
		TRUE,
		INVALIDDBID,								// [IN] which optional columns 
		eSELECT,									// [IN] kind of statement
		SELECT_COLLISTFROMTBL,						// [IN] sql statement
		NULL,										// [IN] client's choice for sql statement
		PREPARE,									// [IN] prepared state
		1,											// [IN] run's prepared for
		VALIDROWSET,								// [IN] Property structure
		ISOPTIONAL,									// [IN] Property dwOption
		0,											// [IN]	Prpoerty ID
		&m_pIRowsetReturned), DB_E_BADCOLUMNID) )	// [IN] pIRowset
	{
		FREE;
		goto CLEANUP;
	}
	
	// Do extended check following GetColumnsRowset
	fSucceed &= XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);	
	
	// Verify results
	Check_GetColumnsRowset(DB_E_BADCOLUMNID,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset);

	FREE;

CLEANUP:
	
	// Return the results
	if( fSucceed )
		return TEST_PASS;
	else	
		return TEST_FAIL; 
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND or DB_E_NOTPREPARED Row_IColumnsRowset calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_7()
{
 	HRESULT	hr			= E_FAIL;		// HRESULT
	BOOL	fSucceed	= TEST_PASS;	// indicates variation success

 	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the Row_IColumnsRowset method.
	//We then check extended errors to verify the right extended error behavior.

	if (!g_fRowset)
	{
		odtLog << L"No IColumnsRowset support on rowsets\n";
		return TEST_SKIPPED;
	}

	INIT;

	if( NOTSUPPORTED == m_ePREPARATION )
	{
		// GetColumnsRowset
		if(hr = GetColumnsRowset(
			TRUE,
			TRUE,
			IID_IRowset,								// [IN] which optional columns 
			TRUE,
			ALLDBID,									// [IN] which optional columns 
			eNOCOMMAND,									// [IN] kind of statement
			NO_QUERY,									// [IN] sql statement
			NULL,										// [IN] client's choice for sql statement
			PREPARE,									// [IN] prepared state
			1,											// [IN] run's prepared for
			VALIDROWSET,								// [IN] Property structure
			ISOPTIONAL,									// [IN] Property dwOption
			0,											// [IN]	Prpoerty ID
			&m_pIRowsetReturned,						// [IN] pIRowset
			FALSE,
			DB_E_NOCOMMAND) )		
		fSucceed &= XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);	

		// Verify results
		if( Check_GetAvailableColumns(DB_E_NOCOMMAND,hr,m_cDBID,m_rgDBID) )
			fSucceed &= TEST_PASS;
	}
	else
	{
		// GetColumnsRowset
		if(hr = GetColumnsRowset(
			0,
			NULL,
			IID_IRowset,								// [IN] which optional columns 
			TRUE,
			ALLDBID,									// [IN] which optional columns 
			eSELECT,									// [IN] kind of statement
			SELECT_COLLISTFROMTBL,						// [IN] sql statement
			NULL,										// [IN] client's choice for sql statement
			BOTH,										// [IN] prepared state
			1,											// [IN] run's prepared for
			VALIDROWSET,								// [IN] Property structure
			ISOPTIONAL,									// [IN] Property dwOption
			0,											// [IN]	Prpoerty ID
			&m_pIRowsetReturned,						// [IN] pIRowset
			FALSE,
			DB_E_NOTPREPARED) )	
		fSucceed &= XCHECK(m_pIColumnsRowset, IID_IColumnsRowset, hr);	

		// Verify results
		if( Check_GetColumnsRowset(DB_E_NOTPREPARED,hr,m_cDBID,m_rgDBID,m_pIRowsetReturned,IID_IRowset) )
			fSucceed &= TEST_PASS;
	}

	FREE;

	// Return the results
	return fSucceed;
}
//}}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ExtendedErrors::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColRow::Terminate());
}	// }}

// }}

//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2002 Microsoft Corporation
//
// @doc 
//
// @module ORAPROC.CPP | ORAPROC source file for all test modules.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "ORAPROC.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xeed32d2b, 0x8368, 0x47c5, { 0xbd, 0x35, 0x11, 0x1d, 0x34, 0x92, 0xb1, 0x7c }};

DECLARE_MODULE_NAME("Oraproc");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Oracle Procedure Test");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals and Enums
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define DBPROP_MSDAORA_DETERMINEKEYCOLUMNS	1
const GUID DBPROPSET_MSDAORA_ROWSET = {0xE8CC4CBD,0xFDFF,0x11D0,{0xB8,0x65,0x00,0xA0,0xC9,0x08,0x1C,0x1D}};

// Oracle Proc Flags
#define	STRONG_TYPE			0x1  
#define USE_OPENFOR			0x2
#define USE_DIRECTASSIGN	0x4
#define EXTRAPARAM_FIRST	0x8
#define EXTRAPARAM_LAST		0x10
#define EXTRALITERAL_FIRST  0x20
#define EXTRALITERAL_LAST	0x40
#define PREFETCHED_CURS		0x80
#define EXTRATWOPARAM_LAST  0x100
#define EXTRATWOLITERAL_LAST  0x200

//STRONG_TYPE		Strongly typed cursor
//USE_OPENFOR		created using "OPEN FOR..."
//USE_DIRECTASSIGN	create using assignment := op

// enum ETABLE
enum ETABLE
{
	NO_BLOB_TABLE,		// A table with no BLOB columns
	LONG_TABLE,			// A table with one BLOB column of type LONG
	LONGRAW_TABLE,		// A table with one BLOB column of type LONG RAW
	MANYCOLUMNS_TABLE,	// A table with at least 26 columns
	MANY_50_COLUMNS_TABLE, // A table with at more than 50 columns
	
};

WCHAR wszPKG[] = L"Pkg";
WCHAR wszSP[] = L"sp";

BOOL g_fKagera;

DBPROP		g_FindPropOpt = {DBPROP_IRowsetFind, DBPROPOPTIONS_OPTIONAL, 0, {DB_NULLGUID, 0, (LPOLESTR)0}, {VT_BOOL, 0, 0, 0, VARIANT_TRUE}};
DBPROP		g_rgVariousProps[] = 
	{
		{DBPROP_CANHOLDROWS, DBPROPOPTIONS_OPTIONAL, 0, {DB_NULLGUID, 0, (LPOLESTR)0}, {VT_BOOL, 0, 0, 0, VARIANT_TRUE}},
		{DBPROP_CANFETCHBACKWARDS, DBPROPOPTIONS_OPTIONAL, 0, {DB_NULLGUID, 0, (LPOLESTR)0}, {VT_BOOL, 0, 0, 0, VARIANT_TRUE}},
		{DBPROP_CANSCROLLBACKWARDS, DBPROPOPTIONS_OPTIONAL, 0, {DB_NULLGUID, 0, (LPOLESTR)0}, {VT_BOOL, 0, 0, 0, VARIANT_TRUE}},
	};

ULONG g_ulDBMSVer;

// Ref Cursor proc information
static WCHAR* rgwszNOBLOBTypesList[] = 
									{
									L"float", 
									L"varchar2",
									L"char", 
									L"number(38,38)",
									L"number(38,0)",
									L"number(9,0)",
									L"number(9,4)",
									L"number()",
									L"date"
									};

static WCHAR* rgwszLONGBLOBTypesList[] = 
									{
									L"float", 
									L"varchar2",
									L"char", 
									L"number(38,38)",
									L"number(38,0)",
									L"long",
									L"number(5,0)",
									L"number(5,5)",
									L"number()",
									L"date"
									};

static WCHAR* rgwszLONGRAWBLOBTypesList[] = 
									{
									L"float", 
									L"varchar2",
									L"char", 
									L"number(38,4)",
									L"number(3,0)",
									L"number(5,0)",
									L"number(5,5)",
									L"number()",
									L"date",
									L"long raw"
									};

static WCHAR* rgwszMANYCOLUMNSTypesList[] =
	{L"float", L"number(1,0)", L"number(3,0)", L"date", L"char", L"number(38,0)", L"number", 
	L"number(5,0)", L"number(7,0)", L"number(2,0)", L"number(4,0)", L"number(6,0)", L"number(19,4)",
	L"number(10,0)", L"number(19,0)", L"number(20,0)", L"number(28,0)", L"number(5,2)", 
	L"number(21,3)", L"number(14,0)", L"number(25,6)", L"number(38,10)", L"number(23,0)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(21,1)"};

static WCHAR* rgwszMANY_50_COLUMNSTypesList[] =
	{L"float", L"number(1,0)", L"number(3,0)", L"date", L"char", L"number(38,0)", L"number", 
	L"number(5,0)", L"number(7,0)", L"number(2,0)", L"number(4,0)", L"number(6,0)", L"number(19,4)",
	L"number(10,0)", L"number(19,0)", L"number(20,0)", L"number(28,0)", L"number(5,2)", 
	L"number(21,3)", L"number(14,0)", L"number(25,6)", L"number(38,10)", L"number(23,0)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(21,1)"
	L"number(5,0)", L"number(7,0)", L"number(2,0)", L"number(4,0)", L"number(6,0)", L"number(19,4)",
	L"number(10,0)", L"number(19,0)", L"number(20,0)", L"number(28,0)", L"number(5,2)", 
	L"number(21,3)", L"number(14,0)", L"number(25,6)", L"number(38,10)", L"number(23,0)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(21,1)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(30,0)",
	L"number(5,0)", L"number(7,0)", L"number(2,0)", L"number(4,0)", L"number(6,0)", L"number(19,4)",
	L"number(10,0)", L"number(19,0)", L"number(20,0)", L"number(28,0)", L"number(5,2)", 
	L"number(21,3)", L"number(14,0)", L"number(25,6)", L"number(38,10)", L"number(23,0)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(21,1)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(30,0)",
	L"number(5,0)", L"number(7,0)", L"number(2,0)", L"number(4,0)", L"number(6,0)", L"number(19,4)",
	L"number(10,0)", L"number(19,0)", L"number(20,0)", L"number(28,0)", L"number(5,2)", 
	L"number(21,3)", L"number(14,0)", L"number(25,6)", L"number(38,10)", L"number(23,0)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(21,1)"
	L"number(5,0)", L"number(7,0)", L"number(2,0)", L"number(4,0)", L"number(6,0)", L"number(19,4)",
	L"number(10,0)", L"number(19,0)", L"number(20,0)", L"number(28,0)", L"number(5,2)", 
	L"number(21,3)", L"number(14,0)", L"number(25,6)", L"number(38,10)", L"number(23,0)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(21,1)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(30,0)",
	L"number(5,0)", L"number(7,0)", L"number(2,0)", L"number(4,0)", L"number(6,0)", L"number(19,4)",
	L"number(10,0)", L"number(19,0)", L"number(20,0)", L"number(28,0)", L"number(5,2)", 
	L"number(21,3)", L"number(14,0)", L"number(25,6)", L"number(38,10)", L"number(23,0)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(21,1)",
	L"number(12,4)", L"number(25,5)", L"number(30,0)", L"number(30,0)"
	};


// Backend type is of type wType with Precision ulParamSize
// calculate buffer needed to bind to WCHAR
static DBLENGTH FindMaxcbLen(DBLENGTH ulParamSize, DBTYPE wType)
{
	DBLENGTH cbLength;

	switch ( wType )
	{
	case DBTYPE_WSTR:
		cbLength = (ulParamSize+1)*sizeof(WCHAR);
		break;
	case DBTYPE_STR:
		cbLength = (ulParamSize+1)*sizeof(WCHAR);
		break;
	case DBTYPE_BYTES:
		cbLength = ((ulParamSize*2)+1)*sizeof(WCHAR);
		break;
	default:
		cbLength = 1000;
		break;
	}

	return cbLength;
}

static DBLENGTH GetDataLength(void *pData, DBTYPE wColType, DBLENGTH cbBytesLen)
{
	DBLENGTH cbLength;

	switch ( wColType )
	{
	case DBTYPE_WSTR:
		cbLength = (wcslen((WCHAR *)pData)+1)*sizeof(WCHAR);
		break;
	case DBTYPE_STR:
		cbLength = strlen((char *)pData)+sizeof(char);
		break;
	case DBTYPE_BYTES:
		cbLength = cbBytesLen;
		break;
	default:
		cbLength = GetDBTypeSize(wColType);
		break;
	}

	return cbLength;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class Oraproc
// @base public | CCommandObject
//
class COraproc : public CCommandObject{
public:
	
	// @cmember ICommand ptr object, around for lifetime of test case
	ICommand*				m_pICommand;
	// @cmember ICommandText Interface pointer
	ICommandText*			m_pICommandText;
	// @cmember ICommandPrepare Interface pointer
	ICommandPrepare*		m_pICommandPrepare;
	// @cmember ICommandWithParameters Interface pointer
	ICommandWithParameters*	m_pICmdWParams;
	// @cmember IDBIfo Interface pointer
	IDBInfo*				m_pIDBInfo;
	
	// @cmember Is ICommandPrepare Supported
	BOOL			m_PrepareSupport;
	// @cmember Should command object now be in prepared state (PREPARE or UNPREPARE)
	EPREPARE		m_PrepareState;
	
	// Result from method
	HRESULT			m_hr;
	// Result from variation
	BOOL			m_fResult;

	// data buffer for parameter data
	void*			m_pData;
	// HACCESSOR
	HACCESSOR		m_hAccessor;
	// array of pData's
	void*			m_pppvData[MAX_BINDINGS][MAX_PARAMSETS];

	// Array of DBBINDING structs
	DBBINDING		m_rgBindings[MAX_BINDINGS];
	// Keeps track of elements in m_pBindings
	DBCOUNTITEM		m_cBindings;

	// Array of DBPARAMBINDINFP structs
	DBPARAMBINDINFO	m_rgParamBindInfo[MAX_BINDINGS];
	// Keep track of elements in m_pParamBindInfo
	DBCOUNTITEM		m_cParamBindings;

	// Keep track of input statuses
	DBSTATUS		m_rgStatus[MAX_BINDINGS];

	// Keeps tracks of columns in the automaketable generated by test cases
	DBORDINAL		m_iNumCols;
	// Keeps tracks of parameter sets
	DB_UPARAMS		m_cParamSets;
	// Count of bytes size of OUTPUT parameter buffer
	DBLENGTH		m_cbBufferSize;
	// Used in rare occasions when we retrieve a pre-fetched cursor
	DBCOUNTITEM		m_ulVerifyRowStart;	

	CTable*			m_pAutoMakeTable;

	CTable*			m_pRefCursorTable;

	// flag to determine whether a Provider can determine parameter info
	BOOL			m_fCanDeriveParamInfo;

	WCHAR*			m_pwszPackageName;
	WCHAR*			m_pwszRefProcName;			
	WCHAR*			m_pwszCallRefProc;	

	DBORDINAL		m_rgParamColOrd[MAX_BINDINGS];
	DBORDINAL		m_rgParamLongColOrd[MAX_BINDINGS];

	ETABLE			m_eTableType;

	// @cmember Constructor
	COraproc(LPWSTR wszTestCaseName) : CCommandObject (wszTestCaseName)
	{
		m_pICommandText				= NULL;
		m_pICommand					= NULL;				
		m_pICommandPrepare			= NULL;				
		m_pICmdWParams				= NULL;				
		m_pIDBInfo					= NULL;
		m_pData						= NULL;
		m_PrepareSupport			= FALSE;
		m_PrepareState				= UNPREPARE;
		m_fResult					= FALSE;
		m_hr						= E_FAIL;
		m_cBindings					= 0;
		m_cParamBindings			= 0;
		m_pAutoMakeTable			= NULL;
		m_pRefCursorTable			= NULL;
		m_fCanDeriveParamInfo		= 0;
		m_pwszPackageName			= NULL;
		m_pwszRefProcName			= NULL;
		m_pwszCallRefProc			= NULL;
	};

	// @cmember Constructor
	~COraproc()
	{
		ASSERT(!m_pAutoMakeTable);
		ASSERT(!m_pRefCursorTable);
		ASSERT(!m_pICommandText);
		ASSERT(!m_pICmdWParams);
		ASSERT(!m_pICommandPrepare);	
		ASSERT(!m_pICommand);				
		ASSERT(!m_pIDBInfo);				
	};

	//@cmember Init
	BOOL Init();
	//@cmember Terminate
	BOOL Terminate();

	BOOL CreateCommand();

	BOOL	MakeBinding
		(
		DBORDINAL	iOrdinal,
		DBPARAMIO	eParamIO,
		DBTYPE		wBindingType,
		void		*pData,						
		BYTE		bPrecision,
		BYTE		bScale,
		WCHAR		*pwszDataSourceType,
		DBTYPE		wColType,
		DBLENGTH	ulParamSize,
		DWORD		dwPart = DBPART_STATUS | DBPART_LENGTH | DBPART_VALUE,
		DBSTATUS	dbsStatus = DBSTATUS_S_OK,
		DB_UPARAMS	cParamSets = 1,
		WCHAR		*pwszParameterName = NULL
		);
	
	BOOL	ClearAllBindings();

	HRESULT	ExecuteProc
		(
		WCHAR *			wszCommand,
		BOOL			fPrepare,
		BOOL			fSetParamInfo,
		REFIID			riid,
		DBROWCOUNT *	pcRowsAffected,
		IUnknown **		ppRowset
		);

	HRESULT Execute(WCHAR *wszCommand, BOOL fPrepare = FALSE);

	BOOL	CheckBindingValue(ULONG iOrdinal, WCHAR *wszExpectedValue, ULONG cParamSet = 1);

	BOOL	CreateNumericProc1();

	BOOL	CreateNumericProc2();

	BOOL	CreateDateFunc1();

	BOOL	CreateCharProc1();

	BOOL    CreateProcLongRaw1();

	BOOL	CreatePackage1();

	BOOL	CreatePackage2();

	BOOL	CreatePkg1LargeResults();

	BOOL	CreatePkg1DateProc();

	BOOL	CreatePkg1RawProc();

	BOOL	CreatePkg1SimilarCols();

	BOOL	CreateVarNumPackage(BOOL fNoScale);

	BOOL	Fetch_Pkg1Proc1(IRowset *pIRowset);

	BOOL	Fetch_Pkg1ProcDate(IRowset *pIRowset);

	BOOL	Fetch_Pkg1ProcRaw(IRowset *pIRowset);

	BOOL	Fetch_Pkg1ProcMultRes(IMultipleResults *pIMultRes);

	BOOL	Fetch_Pkg1ProcLargeRes(IRowset *pIRowset);

	BOOL	Fetch_VarPkg1(IRowset* pIRowset, BOOL fNoScale);

	BOOL	CheckBindingStatus(ULONG ulOrdinal, DBSTATUS dbsExpectedStatus, ULONG cParamSets=1);
	
	BOOL	CheckBindingLength(ULONG ulOrdinal, ULONG ulExpectedLength, ULONG cParamSets=1);

	BOOL	TestNumberArray(BOOL bPrepare, BOOL bSetParamInfo);

	BOOL	TestCharTypeProc1(BOOL bPrepare, BOOL bDefaultParam1);

	BOOL	TestLargeResultsArray(BOOL bPrepare, BOOL bSetParamInfo);

	BOOL	TestMultResArray(BOOL bPrepare, BOOL bSetParamInfo);

	BOOL	TestDateArray(BOOL bPrepare, BOOL bSetParamInfo);

	BOOL	TestRawArray(BOOL bPrepare, BOOL bSetParamInfo);

	BOOL	TestVarNumArray(BOOL bPrepare, BOOL bSetParamInfo, BOOL bNoScale);

	BOOL	TestNumericTypesProc2(BOOL bPrepare, BOOL bDefaultParam1);

	BOOL	TestNumericTypesProcScenario(BOOL bDefaultParam1, BOOL fGetParamInfo);

	BOOL	TestLongRawType1(BOOL bPrepare, BOOL bSetParamInfo);

	BOOL	TestDateTypeFunc1(BOOL bPrepare, BOOL bSetParamInfo);

	BOOL	TestDateTypeFunc2(BOOL bPrepare, BOOL bSetParamInfo);

	HRESULT	CreateRefCursorTable(ETABLE eTableType);

	HRESULT DropRefCursorTable();

	HRESULT CreateRefCursorProc(DWORD dwProcFlags);

	HRESULT	DropRefCursorProc();

	HRESULT CreatePackageBody(DWORD dwProcFlags, WCHAR* pwszParameterList, WCHAR** ppwszPackageBodyCreate);

	BOOL TestRefCursorProc(BOOL fPrepare, IID riid, ULONG cProps = 0, DBPROP* rgPropIDs = NULL, BOOL fSetParamInfo = FALSE);

	BOOL VerifyColumnsInfo(IColumnsInfo* pIColumnsInfo);

	BOOL VerifyRowset(IRowset* pIRowset);

	void SetRefCursorCallText(WCHAR* pwszCallProc);

	BOOL TestRefCursWParam(DWORD dwProcFlags, BOOL fPrepare, BOOL fSetParamInfo, ULONG ulSeed);

	BOOL VerifyProcColumns();

	BOOL VerifyColumnsSchema(WCHAR * pwszTableName, BOOL fSynonym);

	BOOL CreateArrayPkgFromTable
		(
			CTable *	pTable,
			WCHAR **	ppwszCreatePkg,
			WCHAR **	ppwszCreatePkgBody,
			WCHAR **	ppwszCallProc,
			WCHAR **	ppwszDropPkg
		);

	BOOL CheckNativeError
		(
			HRESULT		hr,
			IID			iid,
			DBCOUNTITEM	cRecordsExpected
		);
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL COraproc::Init()
{
  	if (COLEDB::Init())	
	{
		// QI for IDBInfo Optional Interface
		m_pThisTestModule->m_pIUnknown->QueryInterface(
											IID_IDBInfo, (void **)&m_pIDBInfo);

		// Set needed pointers
		SetDataSourceObject(m_pThisTestModule->m_pIUnknown, TRUE);
		SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);

		// Get a Command object
		if (!CreateCommand())
		{
			odtLog << wszINITCASEfailed;
			return FALSE;
		}
		
		m_ulVerifyRowStart = 1;

		m_pAutoMakeTable =  new CTable(m_pThisTestModule->m_pIUnknown2, (WCHAR *)g_wszModuleName, USENULLS);		
		if(!m_pAutoMakeTable || !SUCCEEDED(m_pAutoMakeTable->CreateTable(1,1,NULL,PRIMARY,TRUE)))
			return FALSE;

		return TRUE;
	}  

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember Terminate
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL COraproc::Terminate()
{
	// Release objects
	SAFE_RELEASE(m_pICommand);
	SAFE_RELEASE(m_pIAccessor);
	SAFE_RELEASE(m_pICommandText);
	SAFE_RELEASE(m_pICommandPrepare);
	SAFE_RELEASE(m_pICmdWParams)
	SAFE_RELEASE(m_pIDBInfo);

	// Drop the AutomakeTable
	if (m_pAutoMakeTable)
	{
		m_pAutoMakeTable->DropTable();
		SAFE_DELETE(m_pAutoMakeTable);
	}
	
	ReleaseDBSession();
	ReleaseDataSourceObject();
	
	return(COLEDB::Terminate());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember - CreateCommand - creates a new set of commands
//			sometimes used to reset command properties
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL COraproc::CreateCommand()
{
	BOOL fSuccess = TRUE;

	SAFE_RELEASE(m_pICmdWParams);
	SAFE_RELEASE(m_pICommand);
	SAFE_RELEASE(m_pIAccessor);	
	SAFE_RELEASE(m_pICommandText);
	SAFE_RELEASE(m_pICommandPrepare);

	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandWithParameters, (IUnknown **)&m_pICmdWParams), S_OK);
		
	TESTC(VerifyInterface(m_pICmdWParams, IID_ICommand, COMMAND_INTERFACE, (IUnknown **)&m_pICommand));
	TESTC(VerifyInterface(m_pICmdWParams, IID_IAccessor, COMMAND_INTERFACE, (IUnknown **)&m_pIAccessor));
	TESTC(VerifyInterface(m_pICmdWParams, IID_ICommandText, COMMAND_INTERFACE, (IUnknown **)&m_pICommandText));
	
	// Check for Prepare Support
	if(VerifyInterface(m_pICommand,IID_ICommandPrepare,	COMMAND_INTERFACE,(IUnknown **)&m_pICommandPrepare))
	{
		m_PrepareSupport = TRUE;
	}
	fSuccess = TRUE;

CLEANUP:
	return fSuccess;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @cmember MakeBinding - maps ODBC SQLBindParameter calls
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL COraproc::MakeBinding
(
	DBORDINAL	iOrdinal,
	DBPARAMIO	eParamIO,
	DBTYPE		wBindingType,
	void *		pData,
	BYTE		bPrecision,
	BYTE		bScale,
	WCHAR *		pwszDataSourceType,
	DBTYPE		wColType,
	DBLENGTH	ulParamSize,
	DWORD		dwPart,
	DBSTATUS	dbsStatus,
	DB_UPARAMS	cParamSets,
	WCHAR		*pwszParameterName
)
{
	BOOL		fReplace = FALSE;
	DWORD		dwFlags = 0;
	DBLENGTH	cbMaxLen = 0;
	DBORDINAL	ulTargetOrdinal = m_cBindings;
	DBOBJECT	dbObject;

	ASSERT(cParamSets == 1); // No support for multiple parameter sets

	// BUG BUG - need to create a separate member to set # of param sets
	m_cParamSets = cParamSets;

	for ( ULONG i = 0; i < m_cBindings; i++ )
	{
		// Should we add or replace a binding?
		if (m_rgBindings[i].iOrdinal == iOrdinal)
		{
			fReplace = TRUE;
			ulTargetOrdinal = i;
		}
	}

	if ( !fReplace && (m_cBindings == MAX_BINDINGS || m_cParamBindings == MAX_BINDINGS) )
	{
		odtLog << L"You created too many bindings!" << ENDL;
		return FALSE;
	}

	memset(&(m_rgBindings[ulTargetOrdinal]), 0, sizeof(DBBINDING));
	memset(&(m_rgParamBindInfo[ulTargetOrdinal]), 0, sizeof(DBPARAMBINDINFO));

	m_rgBindings[ulTargetOrdinal].iOrdinal = iOrdinal;
	m_rgBindings[ulTargetOrdinal].dwPart = dwPart;
	m_rgBindings[ulTargetOrdinal].eParamIO = eParamIO;	
	m_rgBindings[ulTargetOrdinal].pTypeInfo = NULL;
	m_rgBindings[ulTargetOrdinal].obValue = offsetof(ValueInfo, pValue);
	m_rgBindings[ulTargetOrdinal].cbMaxLen = FindMaxcbLen(ulParamSize, wColType);
	m_rgBindings[ulTargetOrdinal].obLength = offsetof(ValueInfo, cbLength);
	m_rgBindings[ulTargetOrdinal].obStatus = offsetof(ValueInfo, dbsStatus);
	m_rgBindings[ulTargetOrdinal].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	m_rgBindings[ulTargetOrdinal].wType = wBindingType;
	m_rgBindings[ulTargetOrdinal].pBindExt = NULL;
	m_rgBindings[ulTargetOrdinal].bPrecision = bPrecision;
	m_rgBindings[ulTargetOrdinal].bScale = bScale;

	if (wBindingType == DBTYPE_IUNKNOWN)
	{
		dbObject.dwFlags = STGM_READ;
		dbObject.iid = IID_ISequentialStream;
		m_rgBindings[ulTargetOrdinal].pObject = &dbObject;
	}

	m_rgStatus[ulTargetOrdinal] = dbsStatus;

	if ( eParamIO & DBPARAMIO_INPUT )
		dwFlags |= DBPARAMFLAGS_ISINPUT;
	if ( eParamIO & DBPARAMIO_OUTPUT )
		dwFlags |= DBPARAMIO_OUTPUT;

	m_rgParamBindInfo[ulTargetOrdinal].pwszDataSourceType = pwszDataSourceType;
	m_rgParamBindInfo[ulTargetOrdinal].pwszName = pwszParameterName;
	m_rgParamBindInfo[ulTargetOrdinal].ulParamSize = ulParamSize;
	m_rgParamBindInfo[ulTargetOrdinal].bPrecision = bPrecision;
	m_rgParamBindInfo[ulTargetOrdinal].bScale = bScale;
	m_rgParamBindInfo[ulTargetOrdinal].dwFlags = dwFlags;
	
	m_pppvData[ulTargetOrdinal][cParamSets-1] = pData;

	if ( !fReplace )
	{
		m_cBindings++;
		m_cParamBindings++;
	}

	return TRUE;
}

BOOL	COraproc::ClearAllBindings()
{
	HRESULT hr;

	if ( !m_pIAccessor || !m_pICmdWParams )
		return FALSE;

	PROVIDER_FREE(m_pData);
	m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL);
	CHECK(hr = m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	m_cBindings = m_cParamBindings = 0;
	
	return SUCCEEDED(hr);
}


//------------------------------------------------------------------------
// Function:	ExecuteProc
// Purpose: 	Execute a stored proc and returns a rowset interface if 
//				any
//
//	Output:	HRESULT E_FAIL - on failure
//					S_FALSE - provider didn't support a needed feature 
//					S_OK - everything worked
//------------------------------------------------------------------------
HRESULT COraproc::ExecuteProc
(
	WCHAR *			wszCommand,
	BOOL			fPrepare,
	BOOL			fSetParamInfo, 
	REFIID			riid,
	DBROWCOUNT *	pcRowsAffected,
	IUnknown **		ppRowset
)
{
	HRESULT			hr;
	DBROWCOUNT		cRows = 0;
	IUnknown *		pIUnknown = NULL;
	DBPARAMS		Params;
	DBBINDSTATUS	rgBindStatus[MAX_BINDINGS];
	DBLENGTH		cbBuffer = 0, cbDataLength = 0, cbNullTerm = 0;

	if ( !pcRowsAffected )
		pcRowsAffected = &cRows;

	if ( !ppRowset )
		ppRowset = &pIUnknown;

	if ( !m_pICommandText )
		return E_FAIL;

	if (!COMPARE(SUCCEEDED(hr = m_pICommandText->SetCommandText(DBGUID_DEFAULT, wszCommand)), TRUE))
		return hr;

	if ( fPrepare )
	{
		if ( !m_PrepareSupport )			
			return S_FALSE;
		else
		{
			if(!m_pICommandPrepare)
				return E_FAIL;
			if (!COMPARE(SUCCEEDED(hr = m_pICommandPrepare->Prepare(0)),TRUE))
				return hr;
		}
	}

	//Create Accessor and call SetparameterInfo before

	if ( m_cBindings != 0 )
	{
		DBCOUNTITEM i;

		for ( i = 0; i < m_cBindings; i++ )
		{
			m_rgBindings[i].obStatus = cbBuffer;
			cbBuffer += sizeof(DBSTATUS);
			m_rgBindings[i].obLength = cbBuffer;
			cbBuffer += sizeof(ULONG);
			if (m_rgBindings[i].dwPart & DBPART_VALUE)
			{
				m_rgBindings[i].obValue = cbBuffer;
				cbBuffer += m_rgBindings[i].cbMaxLen;
			}
		}

		m_cbBufferSize = cbBuffer;
		m_pData = (BYTE *)PROVIDER_ALLOC(m_cbBufferSize*m_cParamSets);
		
		for (DB_UPARAMS cParamSets = 0; cParamSets < m_cParamSets; cParamSets++)
		{
			for ( DBCOUNTITEM cBindings = 0; cBindings < m_cBindings; cBindings++ )
			{
				// Set the status even if parameter is OUTPUT only
				// because some variations set the status to keep the provider
				// honest
				*(DBSTATUS *)((BYTE *)m_pData+m_rgBindings[cBindings].obStatus+(cParamSets*cbBuffer)) = m_rgStatus[cBindings];					
				
				if ( m_rgBindings[cBindings].eParamIO & DBPARAMIO_INPUT )
				{					
					if (m_rgBindings[cBindings].dwPart & DBPART_VALUE)
					{
						cbDataLength = GetDataLength((m_pppvData[cBindings][cParamSets]), m_rgBindings[cBindings].wType, m_rgBindings[cBindings].cbMaxLen);
						if (m_rgBindings[cBindings].wType == DBTYPE_STR)
							cbNullTerm = sizeof(char);
						else if (m_rgBindings[cBindings].wType == DBTYPE_WSTR)
							cbNullTerm = sizeof(WCHAR);
						else
							cbNullTerm = 0;
						
						*(DBLENGTH *)((BYTE *)m_pData+m_rgBindings[cBindings].obLength+(cParamSets*cbBuffer)) = cbDataLength - cbNullTerm;

						memcpy((BYTE *)m_pData+m_rgBindings[cBindings].obValue+(cParamSets*cbBuffer), (m_pppvData[cBindings][cParamSets]), cbDataLength);
					}
				}
			}
		}

		if ( !CHECK(hr =  m_pIAccessor->CreateAccessor(
								DBACCESSOR_PARAMETERDATA,
								m_cBindings, 
								m_rgBindings,
								m_cbBufferSize,
								&m_hAccessor,
								rgBindStatus), S_OK) )
				return hr;

		Params.pData = m_pData;
		Params.cParamSets = m_cParamSets;
		Params.hAccessor = m_hAccessor;

		DBORDINAL rgParamOrdinals[MAX_BINDINGS];

		if (fSetParamInfo)
		{
			for ( i = 0; i < m_cBindings; i++ )
				rgParamOrdinals[i] = m_rgBindings[i].iOrdinal;

			if ( !COMPARE(SUCCEEDED(hr=m_pICmdWParams->SetParameterInfo(m_cParamBindings, rgParamOrdinals, m_rgParamBindInfo)), TRUE)) 
			{
				return hr;		
			}
			
			m_fCanDeriveParamInfo =  ( hr == DB_S_TYPEINFOOVERRIDDEN );
		}
	}
	else
	{
		Params.cParamSets = 0;
		Params.pData = NULL;
	}
	
	hr = m_pICommandText->Execute(NULL, riid, &Params, pcRowsAffected, ppRowset);

	return hr;
}

//------------------------------------------------------------------------
// Function:	Execute
// Purpose: 	Mapping for ODBC SQLExecDirect calls
//				
//
//	Output:	HRESULT E_FAIL - on failure
//					S_OK - everything worked
//					other - returns hr from the Provider
//------------------------------------------------------------------------
HRESULT COraproc::Execute(WCHAR *wszCommand, BOOL fPrepare)
{
	HRESULT hr;
	
	if ( !m_pICommandText )
		return E_FAIL;

	if (!SUCCEEDED(hr = m_pICommandText->SetCommandText(DBGUID_DEFAULT, wszCommand)))
		return hr;

	if ( fPrepare )
	{
		if ( !m_PrepareSupport || !m_pICommandPrepare )
			return E_FAIL;
		else
			if (!SUCCEEDED(hr = m_pICommandPrepare->Prepare(0)) )
				return hr;
	}
	
	return m_pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL);
}


//------------------------------------------------------------------------
// Function:	CreateNumericProc1
// Purpose: 	create on Oracle procedure with (in, out) parameters
//
//	Output:	Returns TRUE if the procedure was created successfully
//------------------------------------------------------------------------
BOOL  COraproc::CreateNumericProc1 ()
{
	HRESULT hr = E_FAIL;
	// Drop table just in case.
	// Don't check HResult since table might not exist
	Execute(L"Drop Table oratst_sp1tab");
   
	TESTC_(hr=Execute(L"create table oratst_sp1tab( col1 int ,col2 char (5) ) "), S_OK);
    
	TESTC_(hr=Execute(L"insert into oratst_sp1tab values ( 1, 'aaaaa' )"), S_OK);

	TESTC_(hr=Execute(L"insert into oratst_sp1tab values ( 2, 'bbbbb' )"), S_OK);

	TESTC_(hr=Execute(L"create or replace procedure oratst_sp1( par_in_num IN NUMBER , par_out_num  OUT  NUMBER )  as"
	                               L" begin   "
									L"	 select count(*) into par_out_num from  oratst_sp1tab;   "
								    L"end;" ), S_OK);	

CLEANUP:
	return (hr == S_OK);
}


//------------------------------------------------------------------------
// Function:	CreateNumericProc2
// Purpose: 	Create an Oracle procedure with numeric 
//				 ( in default 100, in out) parameter
//
//	Output:	Returns TRUE if the procedure was created successfully
//------------------------------------------------------------------------
BOOL  COraproc::CreateNumericProc2()
{
	HRESULT	hr;

	TESTC_(hr=Execute(	L"create or replace procedure oratst_spInOut1( par_in_num IN number := 100, par_out_num IN OUT number )  as"
						L" begin   "
						L"	 par_out_num := par_in_num;   "
						L" end;"), S_OK);	

CLEANUP:
	return (hr == S_OK);
}

//------------------------------------------------------------------------
// Function:	CreateDateFunc1
// Purpose: 	Create a function which returns birthdate of employee.
//
//	Output:	Returns TRUE if the procedure was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreateDateFunc1()
{
	HRESULT		hr;

	Execute(L"Drop Table oratst_birthdays");

    //create oratst_birhtdays table 
	TESTC_(hr=Execute(L"create table oratst_birthdays ( empid  int, dob date null ) "), S_OK);

	TESTC_(hr=Execute(L"insert into oratst_birthdays values ( 1, {ts '1969-10-10 11:11:11'} ) "), S_OK);
	TESTC_(hr=Execute(L"insert into oratst_birthdays values (2, {d  '1970-01-22'} ) "), S_OK);
	TESTC_(hr=Execute(L"insert into oratst_birthdays values (3, {d  '1960-11-12'} ) "), S_OK);

	TESTC_(hr=Execute(	L"Create or replace function oratst_func_date1  ( id  IN oratst_birthdays.empid%type DEFAULT 1 )  "
						L" RETURN DATE   IS  "
						L"	bday  DATE; "
						L"  BEGIN  "
						L"		select dob into bday from  oratst_birthdays where empid = id; "
						L"       RETURN(bday);   "
						L"   END;  "), S_OK);


	TESTC_(hr=Execute(	L"Create or replace function oratst_func_date2  ( i_dob IN oratst_birthdays.dob%type )  "
						L" RETURN INT   IS  "
						L"	eid int; "
						L"  BEGIN  "
						L"		select empid into eid from  oratst_birthdays where dob = i_dob ; "
						L"       RETURN(eid);   "
						L"   END;  "), S_OK);

CLEANUP:
	return (hr == S_OK);
}


//------------------------------------------------------------------------
// Function:	CreateCharProc1
// Purpose: 	create on Oracle procedure with (in, out) parameters
//
//	Output:	Returns TRUE if the procedure was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreateCharProc1()
{
	HRESULT	hr = E_FAIL;

	// Drop table just in case.
	// Don't check HResult since table might not exist
	Execute(L"Drop Table oratst_weather");
 
	TESTC_(hr=Execute(	L"Create table oratst_weather  (city  char(30),temperature number(3), humidity number(3),"
						L" condition  varchar2(40))"), S_OK);
	
	TESTC_(hr=Execute(L"insert into oratst_weather values ( 'PARIS', 55, 20, 'MILD' )") , S_OK);

	TESTC_(hr=Execute(L"insert into oratst_weather values ('PHOENIX', 75, 30, 'WARM' )"), S_OK);
    
	TESTC_(hr=Execute(L"insert into oratst_weather values ('ROME', 105, 20, 'HOT' )"), S_OK);
	
	
	TESTC_(hr=Execute(	L"create or replace procedure sp_oratst_weather( p1 IN  oratst_weather.city%type DEFAULT 'ROME',p2 OUT varchar2) as "
						L"  BEGIN  "
						L"		SELECT condition into p2 from oratst_weather where city = p1;  "
						L" END;"), S_OK);

CLEANUP:
	return (hr == S_OK);
}


//------------------------------------------------------------------------
// Function:	CreateProcLongRaw1
// Purpose: 	create on Oracle procedure with (in, out) parameters
//
//	Output:	Returns TRUE if the procedure was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreateProcLongRaw1()
{
	HRESULT	hr;

	Execute(L"drop table oratst_tab_longraw");

	TESTC_(hr=Execute(L"create table oratst_tab_longraw( id int primary key, c_longraw long raw )"), S_OK);

	TESTC_(hr=Execute(L"insert into oratst_tab_longraw values ( 1, '62626262')"), S_OK);

	TESTC_(hr=Execute(	L"create or replace procedure oratst_proc_longraw ( i_id IN oratst_tab_longraw.id%type , o_longraw OUT oratst_tab_longraw.c_longraw%type ) AS   "
						L"	BEGIN "
						L"		SELECT  c_longraw into o_longraw from oratst_tab_longraw where id = i_id;  "
						L"	END;"), S_OK);
CLEANUP:
	return (hr == S_OK);
}


//------------------------------------------------------------------------
// Function: CreatePackage1   
// Purpose:	Create a Package with PL/SQL Table type which is returned
//			as an output parameter from an Oracle Procedure and Function
//           
// Output:	Returns TRUE if the package was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreatePackage1()
{
	HRESULT	hr;

   	TESTC_(hr=Execute(	L"CREATE OR REPLACE PACKAGE oratst_pkg1 AS "
						L" TYPE t_Id IS TABLE OF NUMBER(5) INDEX BY BINARY_INTEGER;"
						L" PROCEDURE proc1(o_id OUT t_Id);"    
						L" END oratst_pkg1; "), S_OK);

   	TESTC_(hr=Execute(	L"Create OR replace PACKAGE BODY oratst_pkg1 AS "
						L"	PROCEDURE proc1(o_id OUT t_Id)"
						L"		AS BEGIN" 
						L"		o_id(1) := 200; o_id(2) := 201; o_id(3) := 202;"
						L"		END proc1;"
						L"END oratst_pkg1;"), S_OK);
CLEANUP:
   return (hr == S_OK);
}


//------------------------------------------------------------------------
//
// Function: CreatePackage2  
// Purpose: Create a Package with 2 PL/SQL Table type which is returned
//			as an output parameter from an Oracle Procedure and Function
//           
// Output:	Returns TRUE of the package was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreatePackage2 ()
{
	HRESULT		hr;

    TESTC_(hr=Execute(	L"CREATE OR REPLACE PACKAGE oratst_pkg2 AS"
						L" TYPE t_Id IS TABLE OF NUMBER(5) INDEX BY BINARY_INTEGER;"
						L" TYPE t_Course IS TABLE OF VARCHAR2(10) INDEX BY BINARY_INTEGER;"
						L" PROCEDURE proc2(o_id OUT t_Id, o_course OUT t_Course);"
						L"END oratst_pkg2;"), S_OK);

	

   	TESTC_(hr=Execute(	L"CREATE OR REPLACE PACKAGE BODY oratst_pkg2 AS"
						L"	PROCEDURE proc2(o_id OUT t_Id, o_course OUT t_Course)  AS "
						L"		BEGIN "
						L"			o_id(1) := 200; o_id(2) := 201; o_id(3) := 202;"
						L"			o_course(1) := 'CS101'; o_course(2) := 'EE350'; o_course(3) := 'MATH750';"
						L"		END proc2;"
						L"END oratst_pkg2;"), S_OK);

CLEANUP:
   return (hr == S_OK);
}


//------------------------------------------------------------------------
//
// Function: CreatePkg1LargeResults
// Purpose: Create a Package with a proc that returns a lagre resultset,
//			the size of the result set is determined by the first param
//           
// Output:	Returns TRUE of the package was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreatePkg1LargeResults()
{
	HRESULT		hr;	

    TESTC_(hr=Execute(	L"CREATE OR REPLACE PACKAGE oratst_pkg1 AS "
						L" TYPE t_varchar is TABLE OF VARCHAR2(2000) INDEX BY BINARY_INTEGER;"
						L" procedure proc_large_results( i_nitems NUMBER , o_items OUT  t_varchar );"
						L"END oratst_pkg1;"), S_OK);

   	TESTC_(hr=Execute(	L"Create OR replace PACKAGE BODY oratst_pkg1 AS "
						L"	procedure proc_large_results ( i_nitems NUMBER , o_items OUT t_varchar ) as "
						L"		i NUMBER; tempC VARCHAR2(2000); "
						L"	Begin "
						L"		for i in 1..i_nitems loop "
						L"			 tempC := 'a'; "
						L"			 o_items(i) :=  RPAD(tempC, 2000, to_char(i) );"
						L"		end loop; "
						L"	end;"
						L"	END oratst_pkg1;"), S_OK);

CLEANUP:
   return (hr == S_OK);
}


//------------------------------------------------------------------------
// Function: CreatePkg1DateProc   
// Purpose:	Create a Package with DATE Table type which is returned
//			as an output parameter from an Oracle Procedure and Function
//           
// Output:	Returns TRUE if the package was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreatePkg1DateProc()
{
   	HRESULT	hr;	

   	TESTC_(hr=Execute(	L"CREATE OR REPLACE PACKAGE oratst_pkg1 AS "
						L" TYPE t_ColDate IS TABLE OF DATE INDEX BY BINARY_INTEGER;  "
						L" PROCEDURE proc_date ( o_date OUT t_ColDate  ); "
						L"END oratst_pkg1;"), S_OK);

   	TESTC_(hr=Execute(	L"Create OR replace PACKAGE BODY oratst_pkg1 AS "
						L"	PROCEDURE proc_date  ( o_date OUT t_ColDate ) AS "
						L"		BEGIN  "
						L"			o_date(1) := to_date('1991-04-01','YYYY-MM-DD'); "
						L"			o_date(2) := to_date('1990-01-02','YYYY-MM-DD'); "
						L"		END;"
						L"END oratst_pkg1;"), S_OK);
CLEANUP:
   return (hr == S_OK);
}


//------------------------------------------------------------------------
// Function: CreatePkg1RawProc   
// Purpose:	Create a Package with RAW Table type which is returned
//			as an output parameter from an Oracle Procedure and Function
//           
// Output:	Returns TRUE if the package was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreatePkg1RawProc()
{
   	HRESULT	hr;	

   	TESTC_(hr=Execute(	L"CREATE OR REPLACE PACKAGE oratst_pkg1 AS "
						L" TYPE t_ColRaw  IS TABLE OF RAW(255) INDEX BY BINARY_INTEGER; "
						L" PROCEDURE proc_raw ( o_raw OUT t_ColRaw ); "
						L"END oratst_pkg1;"), S_OK);

   	TESTC_(hr=Execute(	L"Create OR replace PACKAGE BODY oratst_pkg1 AS "
						L"	PROCEDURE proc_raw  ( o_raw OUT t_ColRaw )  AS "
						L"		BEGIN  "
						L"			o_raw(1) := '62626262';"
						L"			o_raw(2) := '63636363';"
						L"		END;"
						L"END oratst_pkg1;"), S_OK);
CLEANUP:
   return (hr == S_OK);
}


//------------------------------------------------------------------------
// Function: CreatePkg1SimilarCols  
// Purpose:	Create a Package with similar table names 
//			Idea is to test the parser
//           
// Output:	Returns TRUE if the package was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreatePkg1SimilarCols()
{
   	HRESULT	hr;	

   	TESTC_(hr=Execute(	L"CREATE OR REPLACE PACKAGE oratst_pkg1 AS "
						L" TYPE t_varchar is TABLE OF VARCHAR2(2000) INDEX BY BINARY_INTEGER;"
						L" procedure proc_similar_cols ( ResCode OUT t_varchar, Res OUT t_varchar ); "
						L"END oratst_pkg1;"), S_OK);

   	TESTC_(hr=Execute(	L"Create OR replace PACKAGE BODY oratst_pkg1 AS "
						L"	procedure proc_similar_cols ( ResCode OUT t_varchar, Res OUT t_varchar ) as"
						L"		BEGIN  "
						L"			Rescode(1) := 'a';"
						L"			Res(1) := 'b';"
						L"		END;"
						L"END oratst_pkg1;"), S_OK);
CLEANUP:
   return (hr == S_OK);
}


//------------------------------------------------------------------------
// Function: CreateVarNumPackage
// Purpose:	Create a Package with PL/SQL Table type which is returned
//			as an output parameter from an Oracle Procedure and Function
//           
// Output:	Returns TRUE if the package was created successfully
//------------------------------------------------------------------------
BOOL COraproc::CreateVarNumPackage(BOOL fNoScale)
{
	HRESULT	hr;

	if( fNoScale )
	{
		TESTC_(hr=Execute(	L"CREATE OR REPLACE PACKAGE oratst_varpkg1 AS "
							L" TYPE t_Id IS TABLE OF NUMBER(40) INDEX BY BINARY_INTEGER;"
							L" PROCEDURE proc1(o_id OUT t_Id);"    
							L" END oratst_varpkg1; "), S_OK);

   		TESTC_(hr=Execute(	L"Create OR replace PACKAGE BODY oratst_varpkg1 AS "
							L"	PROCEDURE proc1(o_id OUT t_Id)"
							L"		AS BEGIN" 
							L"		o_id(1) := 9.99e+39; "
							L"		o_id(2) := -9.99e+39; "
							L"		o_id(3) := 0; "
							L"		o_id(4) := 1; "
							L"		o_id(5) := -1; "
							L"		END proc1;"
							L"END oratst_varpkg1;"), S_OK);
	}
	else
	{
   		TESTC_(hr=Execute(	L"CREATE OR REPLACE PACKAGE oratst_varpkg1 AS "
							L" TYPE t_Id IS TABLE OF NUMBER(38,-84) INDEX BY BINARY_INTEGER;"
							L" PROCEDURE proc1(o_id OUT t_Id);"    
							L" END oratst_varpkg1; "), S_OK);

   		TESTC_(hr=Execute(	L"Create OR replace PACKAGE BODY oratst_varpkg1 AS "
							L"	PROCEDURE proc1(o_id OUT t_Id)"
							L"		AS BEGIN" 
							L"		o_id(1) := 1e+84; "
							L"		o_id(2) := 1e+121; "
							L"		o_id(3) := 99e+120; "
							L"		o_id(4) := 0; "
							L"		o_id(5) := -99e+120; "
							L"		END proc1;"
							L"END oratst_varpkg1;"), S_OK);
	}
CLEANUP:
   return (hr == S_OK);
}


//------------------------------------------------------------------------
//
// Function: FetchPkg1Proc1
// Purpose: Verifies result of executing the proc created by
//			CreatePackage1
//
// Input:   Rowset Interface from package1's stored proc       
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL	COraproc::Fetch_Pkg1Proc1(IRowset *pIRowset)
{
	IAccessor *		pIAccessor = NULL;
	HACCESSOR		hAccessor;
	DBBINDSTATUS	BindStatus = DBSTATUS_S_OK;
	DBLENGTH		cRowSize = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW			hrow = DB_NULL_HROW;
	HROW *			pHRow = &hrow;
	LONG			lValue = 0;
	BOOL			fPass = TEST_FAIL;
	HRESULT			hr;

	int				cCurRow = 0;
	static int		arrPkg1Proc1Data1[] = { 200, 201, 202 };


	VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor);	

	// Set up Binding
	DBBINDING Binding;

	Binding.iOrdinal = 1;
	Binding.dwPart = DBPART_VALUE;
	Binding.eParamIO = DBPARAMIO_NOTPARAM;	
	Binding.pTypeInfo = NULL;
	Binding.cbMaxLen = 0;
	Binding.obValue = 0;
	Binding.obLength = ULONG_MAX;
	Binding.obStatus = ULONG_MAX;
	Binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	Binding.wType = DBTYPE_I4;
	Binding.pBindExt = NULL;
	Binding.bPrecision = 0;
	Binding.bScale = 0;
	

	TESTC_(pIAccessor->CreateAccessor(
								DBACCESSOR_ROWDATA,
								1,
								&Binding,
								NULL,
								&hAccessor,
								&BindStatus
								), S_OK);

	while ( S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow)) )
	{
		TESTC(COMPARE(cRowsObtained, 1));
		TESTC_(pIRowset->GetData(pHRow[0], hAccessor, &lValue), S_OK);

		COMPARE(lValue, arrPkg1Proc1Data1[cCurRow++]); 			

		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL), S_OK);			
	}				
	
	TESTC(cCurRow == 3);
	TESTC_(hr, DB_S_ENDOFROWSET);

	fPass = TEST_PASS;

CLEANUP:
	if (pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor,NULL);
	SAFE_RELEASE(pIAccessor);

	return fPass;
}


//------------------------------------------------------------------------
//
// Function: FetchPkg1ProcDate
// Purpose: Verifies result of executing the proc created by
//			CreatePkg1DateProc
//
// Input:   Rowset Interface from package1's stored proc       
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL COraproc::Fetch_Pkg1ProcDate(IRowset *pIRowset)
{
	IAccessor *		pIAccessor = NULL;
	HACCESSOR		hAccessor;
	DBBINDSTATUS	BindStatus = DBSTATUS_S_OK;
	DBLENGTH		cRowSize = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW			hrow = DB_NULL_HROW;
	HROW *			pHRow = &hrow;
	BOOL			fPass = TEST_FAIL;
	HRESULT			hr;
	DBCOUNTITEM		cCurRow = 0;
	static char *	arrPkg1ProcDate[]  = { "1991-04-01 00:00:00" , "1990-01-02 00:00:00"};
	char			szTimestamp[100];


	VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor);	

	// Set up Binding
	DBBINDING Binding;

	Binding.iOrdinal = 1;
	Binding.dwPart = DBPART_VALUE;
	Binding.eParamIO = DBPARAMIO_NOTPARAM;	
	Binding.pTypeInfo = NULL;
	Binding.cbMaxLen = sizeof(szTimestamp);
	Binding.obValue = 0;
	Binding.obLength = ULONG_MAX;
	Binding.obStatus = ULONG_MAX;
	Binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	Binding.wType = DBTYPE_STR;
	Binding.pBindExt = NULL;
	Binding.bPrecision = 0;
	Binding.bScale = 0;
	
	TESTC_(pIAccessor->CreateAccessor(
								DBACCESSOR_ROWDATA,
								1,
								&Binding,
								NULL,
								&hAccessor,
								&BindStatus
								), S_OK);

	while ( S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow)) )
	{
		TESTC(COMPARE(cRowsObtained, 1));
		TESTC_(pIRowset->GetData(pHRow[0], hAccessor, szTimestamp), S_OK);

		COMPARE(0, strcmp(szTimestamp,arrPkg1ProcDate[cCurRow++])); 			

		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL), S_OK);			
	}				
	
	TESTC(cCurRow == 2);
	TESTC_(hr, DB_S_ENDOFROWSET);

	fPass = TEST_PASS;

CLEANUP:
	if (pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor,NULL);
	SAFE_RELEASE(pIAccessor);

	return fPass;
}


//------------------------------------------------------------------------
//
// Function: FetchPkg1ProcRaw
// Purpose: Verifies result of executing the proc created by
//			CreatePkg1RawProc
//
// Input:   Rowset Interface from package1's stored proc       
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL COraproc::Fetch_Pkg1ProcRaw(IRowset *pIRowset)
{
	IAccessor *		pIAccessor = NULL;
	HACCESSOR		hAccessor;
	DBBINDSTATUS	BindStatus = DBSTATUS_S_OK;
	DBLENGTH		cRowSize = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW			hrow = DB_NULL_HROW;
	HROW *			pHRow = &hrow;
	BOOL			fPass = TEST_FAIL;
	HRESULT			hr;
	DBCOUNTITEM		cCurRow = 0;
	static			BYTE bytes1[] = {0x62, 0x62, 0x62, 0x62};
	static			BYTE bytes2[] = {0x63, 0x63, 0x63, 0x63};
	static			BYTE *arrPkg1ProcRaw[] = { bytes1, bytes2};
	BYTE			rgBytes[8];
	DBBINDING		Binding;

	VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor);	

	// Set up Binding
	Binding.iOrdinal = 1;
	Binding.dwPart = DBPART_VALUE | DBPART_LENGTH;
	Binding.eParamIO = DBPARAMIO_NOTPARAM;	
	Binding.pTypeInfo = NULL;
	Binding.cbMaxLen = 4;
	Binding.obValue = 4;
	Binding.obLength = 0;
	Binding.obStatus = ULONG_MAX;
	Binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	Binding.wType = DBTYPE_BYTES;
	Binding.pBindExt = NULL;
	Binding.bPrecision = 0;
	Binding.bScale = 0;
	
	TESTC_(pIAccessor->CreateAccessor(
								DBACCESSOR_ROWDATA,
								1,
								&Binding,
								NULL,
								&hAccessor,
								&BindStatus
								), S_OK);

	while ( S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow)) )
	{
		TESTC(COMPARE(cRowsObtained, 1));
		TESTC_(pIRowset->GetData(pHRow[0], hAccessor, rgBytes), S_OK);

		COMPARE(*(ULONG *)(rgBytes+Binding.obLength), 4);
		COMPARE(0, memcmp(rgBytes+Binding.obValue, arrPkg1ProcRaw[cCurRow++], 4)); 			

		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL), S_OK);			
	}				
	
	TESTC(cCurRow == 2);
	TESTC_(hr, DB_S_ENDOFROWSET);

	fPass = TEST_PASS;

CLEANUP:
	if (pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor,NULL);
	SAFE_RELEASE(pIAccessor);

	return fPass;
}


//------------------------------------------------------------------------
//
// Function: FetchPkg1ProcMultRes
// Purpose: Verifies result of executing the proc created by
//			CreatePkg2
//
// Input:   IMultipleResults Interface from package1's stored proc       
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL COraproc::Fetch_Pkg1ProcMultRes(IMultipleResults *pIMultRes)
{
	IRowset	*			pIRowset = NULL;
	IAccessor *			pIAccessor = NULL;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBBINDSTATUS		BindStatus;
	HRESULT				hr = S_OK;
	HROW				hrow = DB_NULL_HROW;
	HROW *				pHRow = &hrow;
	DBCOUNTITEM			cResultSet = 0;
	DBCOUNTITEM			cRowsObtained = 0;
	DBCOUNTITEM			cCurRow = 0;

	DBBINDING			rgBinding[2] =
	{
		{1, 0, 0, 0, NULL, NULL, NULL, DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_I4, 0, 0},
		{2, 0, 500, 0, NULL, NULL, NULL, DBPART_LENGTH | DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 500, 0, DBTYPE_STR, 0, 0}
	};

	LONG				lValue = 0;
	BYTE				rgBytes[504];			
	BOOL				fPass = TEST_FAIL;
	
	static int  arrPkg1Proc1Data1[] = { 200, 201, 202 };
	static char* arrPkg2Proc2Data2[] = {"CS101","EE350" ,"MATH750"};

	while ( hr != DB_S_NORESULT )
	{
		if (SUCCEEDED(hr = pIMultRes->GetResult(NULL, 0, IID_IRowset, NULL, (IUnknown **)pIRowset)))
		{

			VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor);	
					
			TESTC_(pIAccessor->CreateAccessor(
										DBACCESSOR_ROWDATA,
										1,
										&rgBinding[cResultSet],
										NULL,
										&hAccessor,
										&BindStatus
										), S_OK);

			while ( S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow)) )
			{
				TESTC(COMPARE(cRowsObtained, 1));

				if (cResultSet == 0)
				{
					TESTC_(pIRowset->GetData(pHRow[0], hAccessor, &lValue), S_OK);
					COMPARE(lValue, arrPkg1Proc1Data1[cCurRow++]); 			
				}
				else
				{
					TESTC_(pIRowset->GetData(pHRow[0], hAccessor, rgBytes), S_OK);
					COMPARE(0, strcmp((char *)rgBytes, arrPkg2Proc2Data2[cCurRow++])); 			
				}

				TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL), S_OK);			
			}				
					
			TESTC(cCurRow == 3);
			TESTC_(hr, DB_S_ENDOFROWSET);

			pIAccessor->ReleaseAccessor(hAccessor,NULL);
			SAFE_RELEASE(pIAccessor);
			SAFE_RELEASE(pIRowset);
			
			cResultSet++;
		}
	}

	fPass = TEST_PASS;
CLEANUP:	
	return fPass;
}


//------------------------------------------------------------------------
//
// Function: FetchPkg1ProcLargeRes
// Purpose: Verifies result of executing the proc created by
//			CreatePkg1LargeResults
//
// Input:   IRowset Interface from package1's stored proc       
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL COraproc::Fetch_Pkg1ProcLargeRes(IRowset *pIRowset)
{
	BOOL			fPass = TEST_FAIL;
	IAccessor *		pIAccessor = NULL;
	HACCESSOR		hAccessor;
	DBBINDSTATUS	BindStatus = DBSTATUS_S_OK;
	DBLENGTH		cRowSize = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW			hrow = DB_NULL_HROW;
	HROW *			pHRow = &hrow;
	HRESULT			hr;
	DBCOUNTITEM		cCurRow = 0;
	char			szData[2001], szExpected[2010], szCurRow[10];

	VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor);	

	// Set up Binding
	DBBINDING Binding;

	Binding.iOrdinal = 1;
	Binding.dwPart = DBPART_VALUE;;
	Binding.eParamIO = DBPARAMIO_NOTPARAM;	
	Binding.pTypeInfo = NULL;
	Binding.cbMaxLen = 2001;
	Binding.obValue = 0;
	Binding.obLength = ULONG_MAX;
	Binding.obStatus = ULONG_MAX;
	Binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	Binding.wType = DBTYPE_STR;
	Binding.pBindExt = NULL;
	Binding.bPrecision = 0;
	Binding.bScale = 0;
	
	TESTC_(pIAccessor->CreateAccessor(
								DBACCESSOR_ROWDATA,
								1,
								&Binding,
								NULL,
								&hAccessor,
								&BindStatus
								), S_OK);

	while ( S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow)) )
	{
		cCurRow++;

		TESTC(COMPARE(cRowsObtained, 1));
		TESTC_(pIRowset->GetData(pHRow[0], hAccessor, szData), S_OK);

		strcpy(szExpected, "a");
		_i64toa(cCurRow, szCurRow, 10);
		while( strlen(szExpected) < 2000)
			strcat(szExpected, szCurRow);
		szExpected[2000] = '\0';
		
		COMPARE(0, strcmp(szData, szExpected)); 			

		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL), S_OK);			
	}				
	
	TESTC(cCurRow == 1000);
	TESTC_(hr, DB_S_ENDOFROWSET);

	if (pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor,NULL);
	SAFE_RELEASE(pIAccessor);

	// If we've made it this far, everything's OK.
	fPass = TEST_PASS;

CLEANUP:
	return fPass;
}


//------------------------------------------------------------------------
//
// Function: Fetch_VarPkg1
// Purpose: Verifies result of executing the proc created by
//			CreateVarNumPackage
//
// Input:   Rowset Interface from package1's stored proc       
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL	COraproc::Fetch_VarPkg1(IRowset* pIRowset, BOOL fNoScale)
{
	IAccessor *		pIAccessor = NULL;
	IColumnsInfo *	pIColumnsInfo = NULL;
	HACCESSOR		hAccessor;
	DBBINDSTATUS	BindStatus = DBSTATUS_S_OK;
	DBLENGTH		cRowSize = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	HROW			hrow = DB_NULL_HROW;
	HROW *			pHRow = &hrow;
	BOOL			fPass = TEST_FAIL;
	DBBINDING		Binding;
	HRESULT			hr;
	DBORDINAL		cCols = 0;
	DBCOLUMNINFO *	rgColInfo = NULL;
	WCHAR*			pwszNames = NULL;
	DBCOUNTITEM		cCurRow = 0;
	double			dblVal = 0;
	static double	rgdbldataNegScale[] = { 1e+84, 1e+121, 99e+120, 0, -99e+120 };
	static double	rgdbldataNoScale[] = { 9.99e+39, -9.99e+39, 0, 1, -1 };
	double *		rgdbldata = ( fNoScale ? rgdbldataNoScale : rgdbldataNegScale );

	// First verify some column metadata
	TESTC(VerifyInterface(pIRowset, IID_IColumnsInfo, ROWSET_INTERFACE, (IUnknown**)&pIColumnsInfo));	
	TESTC_(pIColumnsInfo->GetColumnInfo(&cCols, &rgColInfo, &pwszNames),S_OK);
	TESTC(cCols == 1);
	TESTC(rgColInfo[0].iOrdinal == 1);
	TESTC(rgColInfo[0].pTypeInfo == NULL);
	TESTC(rgColInfo[0].wType == DBTYPE_VARNUMERIC);
	TESTC(rgColInfo[0].ulColumnSize == 20);
	TESTC((rgColInfo[0].dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH) == 0);
	if( fNoScale == FALSE )
	{
		TESTC((rgColInfo[0].dwFlags & DBCOLUMNFLAGS_SCALEISNEGATIVE) == DBCOLUMNFLAGS_SCALEISNEGATIVE);
	}

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor));	

	// Set up Binding
	Binding.iOrdinal = 1;
	Binding.dwPart = DBPART_VALUE;
	Binding.eParamIO = DBPARAMIO_NOTPARAM;	
	Binding.pTypeInfo = NULL;
	Binding.cbMaxLen = 0;
	Binding.obValue = 0;
	Binding.obLength = ULONG_MAX;
	Binding.obStatus = ULONG_MAX;
	Binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	Binding.wType = DBTYPE_R8;
	Binding.pBindExt = NULL;
	Binding.bPrecision = 0;
	Binding.bScale = 0;
	

	TESTC_(pIAccessor->CreateAccessor(
								DBACCESSOR_ROWDATA,
								1,
								&Binding,
								NULL,
								&hAccessor,
								&BindStatus
								), S_OK);

	while ( S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow)) )
	{
		TESTC(COMPARE(cRowsObtained, 1));
		TESTC_(pIRowset->GetData(pHRow[0], hAccessor, &dblVal), S_OK);

		TESTC(fabs(dblVal - rgdbldata[cCurRow]) <= fabs(dblVal*1.7763568394e-15));

		cCurRow++;
		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL), S_OK);			
	}				
	
	TESTC(cCurRow == 5);
	TESTC_(hr, DB_S_ENDOFROWSET);

	fPass = TEST_PASS;

CLEANUP:
	if (pIAccessor)
	{
		pIAccessor->ReleaseAccessor(hAccessor,NULL);
		SAFE_RELEASE(pIAccessor);
	}
	
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(pwszNames);
	SAFE_FREE(rgColInfo);

	return fPass;
}


BOOL	COraproc::CheckBindingValue(ULONG ulOrdinal, WCHAR *wszExpectedValue, ULONG cParamSet)
{
	void *	pValue = NULL;
	void *	pExpected = NULL;
	USHORT	usExpectedsize = 0;
	USHORT  usActualSize = 0;
	BOOL	fSuccess = FALSE;

	ASSERT(ulOrdinal>0 && ulOrdinal <= m_cBindings);
	ASSERT(wszExpectedValue);
	ASSERT(cParamSet>0 && cParamSet <= m_cParamSets);
	ASSERT(m_pData);
	ASSERT(m_rgBindings[ulOrdinal].dwPart & DBPART_VALUE);


	pValue = (BYTE *)m_pData + (m_rgBindings[ulOrdinal-1].obValue + ((cParamSet-1)*m_cbBufferSize));


	if (m_rgBindings[ulOrdinal-1].dwPart & DBPART_LENGTH)
		usActualSize = *(USHORT *)((BYTE *)m_pData + (m_rgBindings[ulOrdinal-1].obLength + ((cParamSet-1)*m_cbBufferSize)));
	else
	{
		if (m_rgBindings[ulOrdinal-1].wType == DBTYPE_VARNUMERIC || m_rgBindings[ulOrdinal-1].wType == DBTYPE_BYTES)
		{
			ASSERT("Cannot compare output Varnumeric or Bytes data without a length binding!");
			return FALSE;
		}
		else
			usActualSize = 0;
	}

	pExpected = WSTR2DBTYPE(wszExpectedValue, m_rgBindings[ulOrdinal-1].wType, &usExpectedsize);

	if (!COMPARE(fSuccess = CompareDBTypeData(
									pValue,
									pExpected,		
									m_rgBindings[ulOrdinal-1].wType,	
									usActualSize,		
									m_rgBindings[ulOrdinal-1].bPrecision,		
									m_rgBindings[ulOrdinal-1].bScale,
									NULL,
									TRUE,
									DBTYPE_EMPTY,
									usExpectedsize), TRUE))
		return FALSE;
	else
		return TRUE;
}


//------------------------------------------------------------------------
//
// Function: TestNumberArray
// Purpose: Verifies result of executing the proc created by
//			CreatePackage1
//
// Input:   BOOL bPrepare - determines whether proc invocation is prepared
//			BOOL bSetParamInfo - determines whether SetParamInfo is called
//
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL	COraproc::TestNumberArray(BOOL bPrepare, BOOL bSetParamInfo)
{
	BOOL	fPass = TEST_FAIL;
	IRowset	*pIRowset = NULL;

	TESTC(CreatePackage1());
	TESTC_(ExecuteProc(L"{call oratst_pkg1.proc1( {resultset 3, o_id} ) }", bPrepare, bSetParamInfo, IID_IRowset, NULL, (IUnknown **)&pIRowset),S_OK);

	fPass = Fetch_Pkg1Proc1(pIRowset);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	return fPass;
}


//------------------------------------------------------------------------
//
// Function: TestLargeResultsArray
// Purpose: Verifies result of executing the proc created by
//			CreatePkg1LargeResults
//
// Input:   BOOL bPrepare - determines whether proc invocation is prepared
//			BOOL bSetParamInfo - determines whether SetParamInfo is called
//
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL	COraproc::TestLargeResultsArray(BOOL bPrepare, BOOL bSetParamInfo)
{
	BOOL			fPass = TEST_FAIL;
	IRowset			*pIRowset = NULL;

	TESTC(CreatePkg1LargeResults());
	TESTC_(ExecuteProc(L"{call oratst_pkg1.proc_large_results( 1000, {resultset 1000, o_items })}", bPrepare, bSetParamInfo, IID_IRowset, NULL, (IUnknown **)&pIRowset),S_OK);

	fPass = Fetch_Pkg1ProcLargeRes(pIRowset);
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	return fPass;
}


//------------------------------------------------------------------------
//
// Function: TestMultResArray
// Purpose: Verifies result of executing the proc created by
//			CreatePkg2
//
// Input:   BOOL bPrepare - determines whether proc invocation is prepared
//			BOOL bSetParamInfo - determines whether SetParamInfo is called
//
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL	COraproc::TestMultResArray(BOOL bPrepare, BOOL bSetParamInfo)
{
	IMultipleResults	*pIMultRes = NULL;
	BOOL				fPass = TEST_FAIL;
	HRESULT				hr;

	TESTC(CreatePackage2());
	hr = ExecuteProc(L"{call oratst_pkg2.proc2( {resultset 3, o_id},{resultset 3, o_course} ) }", bPrepare, bSetParamInfo, IID_IMultipleResults, NULL, (IUnknown **)&pIMultRes);

	if (hr == E_NOINTERFACE)
		return TEST_SKIPPED;

	fPass = Fetch_Pkg1ProcMultRes(pIMultRes);

CLEANUP:

	SAFE_RELEASE(pIMultRes);	
	return fPass;
}


//------------------------------------------------------------------------
//
// Function: TestDateArray
// Purpose: Verifies result of executing the proc created by
//			CreatePkg1DateProc
//
// Input:   BOOL bPrepare - determines whether proc invocation is prepared
//			BOOL bSetParamInfo - determines whether SetParamInfo is called
//
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL	COraproc::TestDateArray(BOOL bPrepare, BOOL bSetParamInfo)
{

	BOOL	fPass = TEST_FAIL;
	IRowset	*pIRowset = NULL;

	TESTC(CreatePkg1DateProc());
	TESTC_(ExecuteProc(L"{ call oratst_pkg1.proc_date( {resultset 3, o_date } ) }", bPrepare, bSetParamInfo, IID_IRowset, NULL, (IUnknown **)&pIRowset),S_OK);

	fPass = Fetch_Pkg1ProcDate(pIRowset);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	return fPass;
}


//------------------------------------------------------------------------
//
// Function: TestRawArray
// Purpose: Verifies result of executing the proc created by
//			CreatePkg1DateProc
//
// Input:   BOOL bPrepare - determines whether proc invocation is prepared
//			BOOL bSetParamInfo - determines whether SetParamInfo is called
//
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL	COraproc::TestRawArray(BOOL bPrepare, BOOL bSetParamInfo)
{

	BOOL	fPass = TEST_FAIL;
	IRowset	*pIRowset = NULL;

	TESTC(CreatePkg1RawProc());
	TESTC_(ExecuteProc(L"{call oratst_pkg1.proc_raw({resultset 3, o_raw})}", bPrepare, bSetParamInfo, IID_IRowset, NULL, (IUnknown **)&pIRowset),S_OK);

	fPass = Fetch_Pkg1ProcRaw(pIRowset);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	return fPass;
}


//------------------------------------------------------------------------
//
// Function: TestVarNumArray
// Purpose: Verifies result of executing the proc created by
//			CreateVarNumPackage
//
// Input:   BOOL bPrepare - determines whether proc invocation is prepared
//			BOOL bSetParamInfo - determines whether SetParamInfo is called
//
// Output:	Returns TRUE if the results are correct
//------------------------------------------------------------------------
BOOL	COraproc::TestVarNumArray(BOOL bPrepare, BOOL bSetParamInfo, BOOL bNoScale)
{
	BOOL		fPass = TEST_FAIL;
	IRowset*	pIRowset = NULL;

	TESTC(CreateVarNumPackage(bNoScale));
	TESTC_(ExecuteProc(L"{call oratst_varpkg1.proc1( {resultset 5, o_id} ) }", bPrepare, bSetParamInfo, IID_IRowset, NULL, (IUnknown **)&pIRowset),S_OK);

	fPass = Fetch_VarPkg1(pIRowset, bNoScale);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	return fPass;
}


BOOL	COraproc::CheckBindingStatus(ULONG ulOrdinal, DBSTATUS dbsExpectedStatus, ULONG cParamSet)
{
	DBSTATUS	dbsReturnedStatus;

	ASSERT(ulOrdinal>0 && ulOrdinal <= m_cBindings);
	ASSERT(cParamSet>0 && cParamSet <= m_cParamSets);
	ASSERT(m_pData);
	ASSERT(m_rgBindings[ulOrdinal-1].dwPart & DBPART_STATUS);

	dbsReturnedStatus = *(DBSTATUS *)((BYTE *)m_pData + (m_rgBindings[ulOrdinal-1].obStatus + ((cParamSet-1)*m_cbBufferSize)));

	return COMPARE(dbsReturnedStatus,dbsExpectedStatus);
}

BOOL	COraproc::CheckBindingLength(ULONG ulOrdinal, ULONG ulExpectedLength, ULONG cParamSet)
{
	ULONG	ulReturnedLength;

	ASSERT(ulOrdinal>0 && ulOrdinal <= m_cBindings);
	ASSERT(cParamSet>0 && cParamSet <= m_cParamSets);
	ASSERT(m_pData);
	ASSERT(m_rgBindings[ulOrdinal-1].dwPart & DBPART_LENGTH);

	ulReturnedLength = *(ULONG *)((BYTE *)m_pData + (m_rgBindings[ulOrdinal-1].obLength + ((cParamSet-1)*m_cbBufferSize)));

	return COMPARE(ulReturnedLength, ulExpectedLength);
}


//------------------------------------------------------------------------
// Function:	TestCharTypeProc1
// Purpose: 	tests proc created by CreateCharProc1
//
// Input:		bPrepare - Flag to determine whether cmd is prepared
//				bDefaultParam - Flag to determine use of DBSTATUS_S_DEFAULT
//------------------------------------------------------------------------      
BOOL COraproc::TestCharTypeProc1(BOOL bPrepare, BOOL bDefaultParam1)
{	
	//make first parameter default parameter  if bDefaultParam1 flag is set 	
	WCHAR		*wszValue1 = (bDefaultParam1 ? NULL : L"ROME");
	DBSTATUS	dbsInput1 = (bDefaultParam1 ? DBSTATUS_S_DEFAULT : DBSTATUS_S_OK);
	DWORD		dwBindingPart = (bDefaultParam1 ? DBPART_STATUS | DBPART_LENGTH : DBPART_STATUS | DBPART_LENGTH | DBPART_VALUE);

	WCHAR		wszDummy[50] = { 0 };
	BOOL		fPass = TEST_FAIL;

	// Make sure to include formal parameter names (Canoe requires this)
	MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_WSTR, (void *)wszValue1, 0, 0, L"CHAR", DBTYPE_STR, 30, dwBindingPart , dbsInput1, 1, L"p1");
	MakeBinding(2, DBPARAMIO_OUTPUT, DBTYPE_WSTR, NULL, 0, 0, L"CHAR", DBTYPE_STR, 3, DBPART_STATUS | DBPART_LENGTH | DBPART_VALUE, DBSTATUS_S_OK, 1, L"p2");

	TESTC_(ExecuteProc(L"{Call sp_oratst_weather(?,?) }", bPrepare, NOSETPARAMINFO, IID_NULL, NULL, NULL),S_OK);

	COMPARE(CheckBindingValue(2, L"HOT"), TRUE);
	ClearAllBindings();


	 // Set output parameter info type to "DBTYPE_VARCHAR"
	MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_WSTR, (void *)wszValue1, 0, 0, L"DBTYPE_VARCHAR", DBTYPE_STR, 30, dwBindingPart , dbsInput1, 1, L"p1");
	MakeBinding(2, DBPARAMIO_OUTPUT, DBTYPE_WSTR, NULL, 0, 0, L"DBTYPE_VARCHAR", DBTYPE_STR, 3, DBPART_STATUS | DBPART_LENGTH | DBPART_VALUE, DBSTATUS_S_OK, 1, L"p2");

 	TESTC_(ExecuteProc(L"{Call sp_oratst_weather(?,?) }", bPrepare, SETPARAMINFO, IID_NULL, NULL, NULL),S_OK);

	COMPARE(CheckBindingValue(2, L"HOT"), TRUE);
	ClearAllBindings();

	// Set output parameter info type to "DBTYPE_VARCHAR" and param type to INPUT/OUTPUT
	MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_WSTR, (void *)wszValue1, 0, 0, L"DBTYPE_CHAR", DBTYPE_STR, 30, dwBindingPart , dbsInput1, 1, L"p1");
	MakeBinding(2, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT, DBTYPE_WSTR, (void *)wszDummy, 0, 0, L"DBTYPE_VARCHAR", DBTYPE_STR, 3, DBPART_STATUS | DBPART_LENGTH | DBPART_VALUE, DBSTATUS_S_OK, 1, L"p2");

	TESTC_(ExecuteProc(L"{Call sp_oratst_weather(?,?) }", bPrepare, SETPARAMINFO, IID_NULL, NULL, NULL),S_OK);

	COMPARE(CheckBindingValue(2, L"HOT"), TRUE);
	ClearAllBindings();   

	fPass = TEST_PASS;

CLEANUP:
	return fPass;
}


//-----------------------------------------------------------------------------------
// TestNumericTypesProc2:  
// Test combinatins for numeric types for NumericProc2 : ortst_spInOut()
//
//  Parameters: 
//					bPrepare  - indicate if stmt should be prepared TRUE = prepare, FALSE = executedirect
//				    bDefaultParam1 - flag if param1 is SQL_DEFAUT_PARAM
//
//-----------------------------------------------------------------------------------
BOOL COraproc::TestNumericTypesProc2(BOOL bPrepare, BOOL bDefaultParam1)
{
	BOOL		fPass = TEST_FAIL;
	WCHAR		*wszValue1 = L"1111111111";
	WCHAR		*wszValue2 = L"000";
	
	DBSTATUS	dbsInput1 =  (bDefaultParam1 ? DBSTATUS_S_DEFAULT: DBSTATUS_S_OK);
    DWORD		dwBindingPart = DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH; 
	
	MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_WSTR, (void *)wszValue1, 0, 0, L"number", DBTYPE_STR, 30, dwBindingPart , dbsInput1, 1, L"par_in_num");
	MakeBinding(2, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT, DBTYPE_WSTR, wszValue2, 0, 0, L"number", DBTYPE_STR, 30, dwBindingPart, DBSTATUS_S_OK, 1, L"par_out_num");

	TESTC_(ExecuteProc(L"{call oratst_spInOut1(?,?) }", bPrepare, NOSETPARAMINFO, IID_NULL, NULL, NULL),S_OK);

	if ( bDefaultParam1 )
		fPass = COMPARE(CheckBindingValue(2, L"100"), TRUE);
	else
		fPass = COMPARE(CheckBindingValue(2, wszValue1), TRUE);
	
CLEANUP:

	ClearAllBindings();
	return fPass;
}


//-----------------------------------------------------------------------------------
// TestNumericTypesProcScenario
// Same proc as in TestNumericTypesProc2
// but set up the proc execution differently
// And pay particularly close attention to customer scenarios
//
//  Parameters: 
//					bPrepare  - indicate if stmt should be prepared TRUE = prepare, FALSE = executedirect
//				    bDefaultParam1 - flag if param1 is SQL_DEFAUT_PARAM
//
//-----------------------------------------------------------------------------------
BOOL COraproc::TestNumericTypesProcScenario(BOOL bDefaultParam1, BOOL fGetParamInfo)
{
	BOOL		fPass = TEST_FAIL;
	WCHAR*		wszValue1 = L"1111111111";
	WCHAR*		wszValue2 = L"000";
	WCHAR		wszCallProc[] = L"{call oratst_spInOut1 (?,?)}";

	DB_UPARAMS		cParams = 0;
	DBPARAMINFO*	rgParamInfo = NULL;
	WCHAR*			pwszNames = NULL;
	
	DBSTATUS	dbsInput1 =  (bDefaultParam1 ? DBSTATUS_S_DEFAULT: DBSTATUS_S_OK);
    DWORD		dwBindingPart = DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH; 

	if (fGetParamInfo)
	{
		// Prepare the command and the parameter information
		TESTC(SUCCEEDED(m_pICommandText->SetCommandText(DBGUID_DEFAULT, wszCallProc)));
		
		if(m_pICommandPrepare)
		{
			TESTC(SUCCEEDED(m_pICommandPrepare->Prepare(0)));
			if (m_pICmdWParams)
			{
				TESTC_(m_pICmdWParams->GetParameterInfo(&cParams, &rgParamInfo, &pwszNames),S_OK);

				TESTC(cParams == 2);

				COMPARE(rgParamInfo[0].iOrdinal, 1);
				COMPARE(rgParamInfo[1].iOrdinal, 2);

				COMPARE(rgParamInfo[0].pTypeInfo, NULL);
				COMPARE(rgParamInfo[1].pTypeInfo, NULL);

				COMPARE(rgParamInfo[0].dwFlags & DBPARAMFLAGS_ISINPUT, DBPARAMFLAGS_ISINPUT);
				COMPARE(rgParamInfo[0].dwFlags & DBPARAMFLAGS_ISNULLABLE, DBPARAMFLAGS_ISNULLABLE);

				COMPARE(rgParamInfo[1].dwFlags & DBPARAMFLAGS_ISOUTPUT, DBPARAMFLAGS_ISOUTPUT);
				COMPARE(rgParamInfo[1].dwFlags & DBPARAMFLAGS_ISNULLABLE, DBPARAMFLAGS_ISNULLABLE);

				if (g_fKagera)
				{
					COMPARE(rgParamInfo[0].pwszName, NULL);
					COMPARE(rgParamInfo[1].pwszName, NULL);

					COMPARE(rgParamInfo[0].ulParamSize, 20);
					COMPARE(rgParamInfo[1].ulParamSize, 20);

					COMPARE(rgParamInfo[0].wType, DBTYPE_VARNUMERIC);
					COMPARE(rgParamInfo[1].wType, DBTYPE_VARNUMERIC);

					COMPARE(rgParamInfo[0].bPrecision, 0);
					COMPARE(rgParamInfo[1].bPrecision, 0);
					COMPARE(rgParamInfo[0].bScale, 0);
					COMPARE(rgParamInfo[1].bScale, 0);
				}
				else
				{
					COMPARE(_wcsicmp(rgParamInfo[0].pwszName, L"PAR_IN_NUM"), 0);
					COMPARE(_wcsicmp(rgParamInfo[1].pwszName, L"PAR_OUT_NUM"), 0);

					COMPARE(rgParamInfo[0].ulParamSize, 131);
					COMPARE(rgParamInfo[1].ulParamSize, 131);

					COMPARE(rgParamInfo[0].wType, DBTYPE_STR);
					COMPARE(rgParamInfo[1].wType, DBTYPE_STR);

					COMPARE(rgParamInfo[0].bPrecision, 255);
					COMPARE(rgParamInfo[1].bPrecision, 255);
					COMPARE(rgParamInfo[0].bScale, 255);
					COMPARE(rgParamInfo[1].bScale, 255);
				}
			}
		}
	}
	
	MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_WSTR, (void *)wszValue1, 0, 0, L"DBTYPE_VARNUMERIC", DBTYPE_STR, 30, dwBindingPart , dbsInput1, 1, L"par_in_num");
	MakeBinding(2, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT, DBTYPE_WSTR, wszValue2, 0, 0, L"DBTYPE_VARNUMERIC", DBTYPE_STR, 30, dwBindingPart, DBSTATUS_S_OK, 1, L"par_out_num");

	TESTC_(ExecuteProc(wszCallProc, PREPARE, SETPARAMINFO, IID_NULL, NULL, NULL),S_OK);

	if ( bDefaultParam1 )
		fPass = COMPARE(CheckBindingValue(2, L"100"), TRUE);
	else
		fPass = COMPARE(CheckBindingValue(2, wszValue1), TRUE);
	
CLEANUP:

	ClearAllBindings();

	PROVIDER_FREE(pwszNames);
	PROVIDER_FREE(rgParamInfo);
	return fPass;
}


//-----------------------------------------------------------------------------------
// TestLongRawType1
// Test combinatoins for Long Raw parameters
//
//  Parameters: 
//					bPrepare  - indicate if stmt should be prepared TRUE = prepare, FALSE = executedirect
//				    bDefaultParam1 - flag if param1 is SQL_DEFAUT_PARAM
//
//-----------------------------------------------------------------------------------
BOOL COraproc::TestLongRawType1(BOOL bPrepare, BOOL bSetParamInfo)
{
	
	BOOL		fPass = TEST_FAIL;
	LONG		lValue = 1;

	TESTC(CreateProcLongRaw1());

	MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_I4, (void *)&lValue, 4, 0, L"number", DBTYPE_I4, 4);
	MakeBinding(2, DBPARAMIO_OUTPUT, DBTYPE_BYTES, NULL, 40, 0, L"long raw", DBTYPE_BYTES, 40);

	TESTC_(ExecuteProc(L"{call oratst_proc_longraw(?,?)}", bPrepare, bSetParamInfo, IID_NULL, NULL, NULL),S_OK);

	fPass = COMPARE(CheckBindingValue(2, L"62626262"), TRUE);
		
CLEANUP:

	ClearAllBindings();
	return fPass;
}


//-----------------------------------------------------------------------------------
// TestDateTypeFunc1
// Test date type output parameter
//
//  Parameters: 
//					bPrepare  - indicate if stmt should be prepared TRUE = prepare, FALSE = executedirect
//				    bDefaultParam1 - flag if param1 is SQL_DEFAUT_PARAM
//
//-----------------------------------------------------------------------------------
BOOL COraproc::TestDateTypeFunc1(BOOL bPrepare, BOOL bSetParamInfo)
{
	LONG	lValue = -999999;  // should be ignored because of default status
	BOOL	fPass = TEST_FAIL;

	// The first binding is output only, but set the status to DBSTATUS_S_DEFAULT to make sure provider ignores it
	MakeBinding(1, DBPARAMIO_OUTPUT, DBTYPE_WSTR, NULL, 23, 0, L"DBTYPE_DBTIMESTAMP", DBTYPE_DBTIMESTAMP, 50, DBPART_STATUS | DBPART_VALUE | DBPART_LENGTH, DBSTATUS_S_DEFAULT, 1, L"RETURN_VALUE"); 
	MakeBinding(2, DBPARAMIO_INPUT, DBTYPE_I4, &lValue, 0, 0, L"number", DBTYPE_VARNUMERIC, 30, DBPART_STATUS | DBPART_VALUE , DBSTATUS_S_DEFAULT, 1, L"id");

	TESTC_(ExecuteProc(L"{ ? = call oratst_func_date1(?) }", bPrepare, bSetParamInfo, IID_NULL, NULL, NULL),S_OK);

	fPass = COMPARE(CheckBindingValue(1, L"1969-10-10 11:11:11"), TRUE);	
	
CLEANUP:

	ClearAllBindings();
	return fPass;
}


//-----------------------------------------------------------------------------------
// TestDateTypeFunc2
// Test int type output parameter
//
//  Parameters: 
//					bPrepare  - indicate if stmt should be prepared TRUE = prepare, FALSE = executedirect
//				    bDefaultParam1 - flag if param1 is SQL_DEFAUT_PARAM
//
//-----------------------------------------------------------------------------------
BOOL COraproc::TestDateTypeFunc2(BOOL bPrepare, BOOL bSetParamInfo)
{
	LONG	lValue = 4;
	DATE	date = 0;
	BOOL	fPass = TEST_FAIL;

	TESTC_(VarDateFromStr(L"2000-02-29", 0, 0, &date), S_OK);
	
	MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_I4, &lValue, 0, 0, L"number", DBTYPE_VARNUMERIC, 30, DBPART_STATUS | DBPART_VALUE);
	MakeBinding(2, DBPARAMIO_INPUT, DBTYPE_DATE, &date, 0, 0, L"DBTYPE_DATE", DBTYPE_DATE, sizeof(DATE), DBPART_STATUS | DBPART_VALUE | DBPART_LENGTH); 	

	TESTC_(ExecuteProc(L"insert into oratst_birthdays values (?, ?)", bPrepare, bSetParamInfo, IID_NULL, NULL, NULL),S_OK);

	ClearAllBindings();

	MakeBinding(1, DBPARAMIO_OUTPUT, DBTYPE_I4, &lValue, 0, 0, L"number", DBTYPE_VARNUMERIC, 30, DBPART_STATUS | DBPART_VALUE);
	MakeBinding(2, DBPARAMIO_INPUT, DBTYPE_DATE, &date, 0, 0, L"DBTYPE_DATE", DBTYPE_DATE, sizeof(DATE), DBPART_STATUS | DBPART_VALUE | DBPART_LENGTH); 

	TESTC_(ExecuteProc(L"{ ? = call oratst_func_date2(?) }", bPrepare, bSetParamInfo, IID_NULL, NULL, NULL),S_OK);

	fPass = COMPARE(CheckBindingValue(1, L"4"), TRUE);	
	
CLEANUP:

	ClearAllBindings();
	return fPass;
}

HRESULT	COraproc::CreateRefCursorTable(ETABLE eTableType)
{
	WCHAR**					ppwszTypesList = NULL;
	CList <WCHAR* ,WCHAR*>	TypesList;
	ULONG					cTypes = 0;
	ULONG					cIter = 0;
	HRESULT					hr = E_FAIL;

	ASSERT(m_pRefCursorTable == NULL);

	m_eTableType = eTableType;

	switch (eTableType)
	{
	case NO_BLOB_TABLE:
		// A table with no BLOB columns
		ppwszTypesList = rgwszNOBLOBTypesList;
		cTypes = NUMELEM(rgwszNOBLOBTypesList);
		break;
	case LONG_TABLE:
		// A table with one BLOB column of type LONG
		ppwszTypesList = rgwszLONGBLOBTypesList;
		cTypes = NUMELEM(rgwszLONGBLOBTypesList);
		break;
	case LONGRAW_TABLE:
		// A table with one BLOB column of type LONG RAW
		ppwszTypesList = rgwszLONGRAWBLOBTypesList;
		cTypes = NUMELEM(rgwszLONGRAWBLOBTypesList);
		break;
	case MANYCOLUMNS_TABLE:
		// A table with at least 26 columns
		ppwszTypesList = rgwszMANYCOLUMNSTypesList;
		cTypes = NUMELEM(rgwszMANYCOLUMNSTypesList);
		break;
	case MANY_50_COLUMNS_TABLE:
		// A table with at least 50 columns
		ppwszTypesList = rgwszMANY_50_COLUMNSTypesList;
		cTypes = NUMELEM(rgwszMANY_50_COLUMNSTypesList);
		break;
	default:
		ASSERT(!"Unknown table type");
		return E_FAIL;
	}

	for (cIter=0; cIter < cTypes; cIter++)
		TypesList.AddTail(ppwszTypesList[cIter]);

	m_pRefCursorTable = new CTable(m_pThisTestModule->m_pIUnknown2, (WCHAR *)g_wszModuleName, USENULLS);		
	TESTC(m_pRefCursorTable != NULL);

	// Create a 20 row table with an index on the first column
	TESTC_(hr = m_pRefCursorTable->CreateTable(TypesList, 20, 1),S_OK);


CLEANUP:
	TypesList.RemoveAll();
	return hr;
}

HRESULT COraproc::CreateRefCursorProc(DWORD dwProcFlags)
{
	ASSERT(m_pRefCursorTable);  // This table must exist

	static WCHAR s_wszWeakCursPkgFormat[] = 
									L"CREATE OR REPLACE PACKAGE %s AS"	// pkg decl
									L"	TYPE t_cursor IS REF CURSOR;"	// weak cursor decl
									L"	PROCEDURE %s (%s);"				// proc decl
									L"END %s;";							// end

	static WCHAR s_wszStrongCursPkgFormat[] = 
									L"CREATE OR REPLACE PACKAGE %s AS"	// pkg decl
									L"	TYPE t_cursor IS REF CURSOR RETURN %s%%rowtype;"	// strong cursor decl
									L"	PROCEDURE %s (%s);"				// proc decl
									L"END %s;";							// end

	WCHAR*		pwszParameterList = NULL;
	WCHAR*		pwszBuf = NULL;
	WCHAR*		pwszPkgCreate = NULL;
	WCHAR*		pwszPackageBodyCreate = NULL;
	HRESULT		hr = E_OUTOFMEMORY;

	// Detect conflicting FLAGS
	ASSERT((dwProcFlags & (USE_OPENFOR | USE_DIRECTASSIGN)) != 0); 

	// This functions save the package name, the procedure name, and the invocation string
	m_pwszPackageName = (WCHAR *)PROVIDER_ALLOC((wcslen(m_pRefCursorTable->GetTableName())+wcslen(wszPKG)+1)*sizeof(WCHAR));
	m_pwszRefProcName = (WCHAR *)PROVIDER_ALLOC((wcslen(m_pRefCursorTable->GetTableName())+wcslen(wszSP)+1)*sizeof(WCHAR));
	m_pwszCallRefProc = (WCHAR *)PROVIDER_ALLOC((wcslen(m_pRefCursorTable->GetTableName())+MAXBUFLEN)*sizeof(WCHAR));

	TESTC(m_pwszPackageName != NULL && m_pwszRefProcName != NULL && m_pwszCallRefProc);

	wcscpy(m_pwszPackageName, m_pRefCursorTable->GetTableName());
	wcscat(m_pwszPackageName, wszPKG);

	wcscpy(m_pwszRefProcName, m_pRefCursorTable->GetTableName());
	wcscat(m_pwszRefProcName, wszSP);

	swprintf(m_pwszCallRefProc, L"{call %s.%s({resultset 0, io_cursor})}", m_pwszPackageName, m_pwszRefProcName);
	
	pwszParameterList = (WCHAR *)PROVIDER_ALLOC(MAXDATALEN);
	pwszBuf = (WCHAR *)PROVIDER_ALLOC(MAXDATALEN);

	if (dwProcFlags & EXTRAPARAM_FIRST)
		wcscpy(pwszParameterList, L"io_Num IN OUT number, ");
	else if (dwProcFlags & EXTRALITERAL_FIRST)
		wcscpy(pwszParameterList, L"i_Num IN number, ");
	else
		wcscpy(pwszParameterList, L"\0");

	wcscat(pwszParameterList, L"io_cursor IN OUT t_cursor");

	if (dwProcFlags & EXTRAPARAM_LAST)
		wcscat(pwszParameterList, L", io_Num IN OUT number");
	else if (dwProcFlags & EXTRALITERAL_LAST)
		wcscat(pwszParameterList, L", i_Num IN number");
	else if (dwProcFlags & EXTRATWOPARAM_LAST)
		wcscat(pwszParameterList, L", i_Num IN number, o_Long OUT long");
	else if (dwProcFlags & EXTRATWOLITERAL_LAST)
		wcscat(pwszParameterList, L", i_Num IN number, o_Long IN long");
	
	// Create the package
	// The cursor type (either strong or weak must be declared here)
	// Any extra parameters must also be decided here
	pwszPkgCreate = (WCHAR *)PROVIDER_ALLOC(MAXDATALEN);
	TESTC(pwszPkgCreate != NULL);

	// Strong Cursor
	if (dwProcFlags & STRONG_TYPE)
	{
		// Strongly typed cursors are anchored to a table row type
		swprintf(
				pwszPkgCreate, 
				s_wszStrongCursPkgFormat,
				m_pwszPackageName, 
				m_pRefCursorTable->GetTableName(),
				m_pwszRefProcName,
				pwszParameterList,
				m_pwszPackageName
				);
	}
	else
	{
		swprintf(
				pwszPkgCreate, 
				s_wszWeakCursPkgFormat,
				m_pwszPackageName, 
				m_pwszRefProcName,
				pwszParameterList,
				m_pwszPackageName
				);
	}

	// Create the package
	TESTC_(hr = Execute(pwszPkgCreate), S_OK);

	// Create the procedure
	// Need to take into account how the procedure is created ("OPEN FOR" vs direct assignment");
	TESTC_(hr = CreatePackageBody(dwProcFlags, pwszParameterList, &pwszPackageBodyCreate), S_OK);
	TESTC_(hr = Execute(pwszPackageBodyCreate), S_OK);

CLEANUP:
	PROVIDER_FREE(pwszPkgCreate);
	PROVIDER_FREE(pwszPackageBodyCreate);
	PROVIDER_FREE(pwszParameterList);
	PROVIDER_FREE(pwszBuf);

	return hr;
}

HRESULT	COraproc::DropRefCursorProc()
{
	WCHAR	wszDropPkg[MAXDATALEN];

	wcscpy(wszDropPkg, L"Drop package ");
	wcscat(wszDropPkg, m_pwszPackageName);

	CHECK(Execute(wszDropPkg), S_OK);
	
	SAFE_FREE(m_pwszPackageName);
	SAFE_FREE(m_pwszRefProcName);
	SAFE_FREE(m_pwszCallRefProc);

	ClearAllBindings();

	return S_OK;
}

HRESULT COraproc::DropRefCursorTable()
{
	HRESULT hr = S_OK;

	if (m_pRefCursorTable)
	{
		TESTC_(hr = m_pRefCursorTable->DropTable(),S_OK);
		SAFE_DELETE(m_pRefCursorTable);
	}

CLEANUP:
	return hr;
}

HRESULT COraproc::CreatePackageBody(DWORD dwProcFlags, WCHAR* pwszParameterList, WCHAR** ppwszPackageBodyCreate)
{
	static WCHAR s_wszOpenForCursPkgBodyFmt[] = 
								L"CREATE OR REPLACE PACKAGE BODY %s AS"	
								L"	PROCEDURE %s (%s) IS "
								L"	BEGIN "
								L"		OPEN io_cursor FOR select * from %s;"
								L"	END %s; "
								L"END %s; ";
								
	static WCHAR s_wszDirectCursPkgBodyFmt[] = 
								L"CREATE OR REPLACE PACKAGE BODY %s AS"	
								L"	PROCEDURE %s (%s) IS "
								L"	temp_cursor t_cursor; "
								L"	BEGIN "
								L"		OPEN temp_cursor FOR select * from %s;"
								L"		io_cursor := temp_cursor;"
								L"	END %s; "
								L"END %s; ";

	static WCHAR s_wszExtraParamOpenForPkgBodyFmt[] = 
								L"CREATE OR REPLACE PACKAGE BODY %s AS"	
								L"	PROCEDURE %s (%s) IS "
								L"	BEGIN "
								L"		select %s into io_Num from %s where %s = io_Num;"
								L"		OPEN io_cursor FOR select * from %s;"
								L"	END %s; "
								L"END %s; ";

	static WCHAR s_wszExtraParamDirectPkgBodyFmt[] = 
								L"CREATE OR REPLACE PACKAGE BODY %s AS"	
								L"	PROCEDURE %s (%s) IS "
								L"	temp_cursor t_cursor; "
								L"	BEGIN "
								L"		select %s into io_Num from %s where %s = io_Num;"
								L"		OPEN temp_cursor FOR select * from %s;"
								L"		io_cursor := temp_cursor;"
								L"	END %s; "
								L"END %s; ";

	static WCHAR s_wszPreFetchOpenForPkgBodyFmt[] = 
								L"CREATE OR REPLACE PACKAGE BODY %s AS"	
								L"	PROCEDURE %s (%s) IS "
								L"	temp_var %s%%rowtype; "
								L"	BEGIN "
								L"		OPEN io_cursor FOR select * from %s;"
								L"		FETCH io_cursor into temp_var;"
								L"	END %s; "
								L"END %s; ";

	static WCHAR s_wszExtraTwoParamOpenForPkgBodyFmt[] = 
								L"CREATE OR REPLACE PACKAGE BODY %s AS"	
								L"	PROCEDURE %s (%s) IS "
								L"	BEGIN "
								L"		select %s into o_Long from %s where %s = i_Num;"
								L"		OPEN io_cursor FOR select * from %s;"
								L"	END %s; "
								L"END %s; ";


	// Don't need a format string to EXTRALITERAL_FIRST, EXTRALITERAL_LAST, or EXTRATWOLITERAL_LAST
	// Just ignore them in the body and use s_wszOpenForCursPkgBodyFmt or s_wszDirectCursPkgBodyFmt

	WCHAR *		pwszProcBody = NULL;
	WCHAR *		rgpwszColumns[2] = {NULL, NULL};
	WCHAR *		rgpwszLongColumns[1] = {NULL};
	DBORDINAL	cColFound = 0;
	DBORDINAL	cLongFound = 0;
	DBORDINAL	cIter = 0;
	CCol		TempCol;
	HRESULT		hr = S_OK;

	ASSERT(pwszParameterList && ppwszPackageBodyCreate);

	*ppwszPackageBodyCreate = (WCHAR *)PROVIDER_ALLOC(MAXDATALEN*2);
	if (!*ppwszPackageBodyCreate)
		return E_OUTOFMEMORY;

	// If we need an extra argument, search for 2 numeric columns
	if (dwProcFlags & (EXTRAPARAM_FIRST | EXTRAPARAM_LAST | EXTRATWOPARAM_LAST))
	{
		ULONG	cNumNeeded = (dwProcFlags & (EXTRAPARAM_FIRST | EXTRAPARAM_LAST)) ? 2 : 1;
		ULONG	cLongNeeded = (dwProcFlags & EXTRATWOPARAM_LAST) ? 1 : 0;
		
		// Find a couple of numeric columns
		for (cIter=1; cIter <= m_pRefCursorTable->CountColumnsOnTable(); cIter++)
		{
			TESTC_(m_pRefCursorTable->GetColInfo(cIter, TempCol), S_OK); 
			
			if( IsNumericType(TempCol.GetProviderType()) &&
				cColFound < cNumNeeded )
			{
				rgpwszColumns[cColFound] = wcsDuplicate(TempCol.GetColName());
				m_rgParamColOrd[cColFound++] = TempCol.GetColNum();

			}
			else if( 0 == _wcsicmp(TempCol.GetProviderTypeName(), L"long") &&
					cLongFound < cLongNeeded )
			{
				rgpwszLongColumns[cLongFound] = wcsDuplicate(TempCol.GetColName());
				m_rgParamLongColOrd[cLongFound++] = TempCol.GetColNum();
			}

			if( cColFound >= cNumNeeded && cLongFound >= cLongNeeded )
				break;
		}
		
		if( cColFound < cNumNeeded || cLongFound < cLongNeeded )
		{
			odtLog << "Couldn't find the columns needed to run test." << ENDL;
			hr = E_FAIL;
			goto CLEANUP;
		}
	}

	if (dwProcFlags & PREFETCHED_CURS)
	// PreFetch takes precedence
	{
		// Tell our rowset verification routine that we expect the first row to be the 2 row in the table
		m_ulVerifyRowStart = 2;

		swprintf(
				*ppwszPackageBodyCreate,
				s_wszPreFetchOpenForPkgBodyFmt,
				m_pwszPackageName,
				m_pwszRefProcName,
				pwszParameterList,				
				m_pRefCursorTable->GetTableName(),
				m_pRefCursorTable->GetTableName(),
				m_pwszRefProcName,
				m_pwszPackageName
				);		
	}
	else if (dwProcFlags & USE_DIRECTASSIGN)
	// Use direct assignment to a REF CURSOR
	{
		if (dwProcFlags & (EXTRAPARAM_FIRST | EXTRAPARAM_LAST))
		{
			swprintf(
					*ppwszPackageBodyCreate,
					s_wszExtraParamDirectPkgBodyFmt,
					m_pwszPackageName,
					m_pwszRefProcName,
					pwszParameterList,
					rgpwszColumns[1],
					m_pRefCursorTable->GetTableName(),
					rgpwszColumns[0],
					m_pRefCursorTable->GetTableName(),
					m_pwszRefProcName,
					m_pwszPackageName
					);
		}
		else
			swprintf(
					*ppwszPackageBodyCreate, 
					s_wszDirectCursPkgBodyFmt,
					m_pwszPackageName,
					m_pwszRefProcName,
					pwszParameterList,
					m_pRefCursorTable->GetTableName(),
					m_pwszRefProcName,
					m_pwszPackageName
					);
	}
	else
	// Use OPEN FOR ... to open cursor
	{
		if (dwProcFlags & (EXTRAPARAM_FIRST | EXTRAPARAM_LAST))
		{
			swprintf(
					*ppwszPackageBodyCreate,
					s_wszExtraParamDirectPkgBodyFmt,
					m_pwszPackageName,
					m_pwszRefProcName,
					pwszParameterList,
					rgpwszColumns[1],
					m_pRefCursorTable->GetTableName(),
					rgpwszColumns[0],
					m_pRefCursorTable->GetTableName(),
					m_pwszRefProcName,
					m_pwszPackageName
					);
		}
		else if (dwProcFlags & EXTRATWOPARAM_LAST)
		{
			swprintf(
					*ppwszPackageBodyCreate,
					s_wszExtraTwoParamOpenForPkgBodyFmt,
					m_pwszPackageName,
					m_pwszRefProcName,
					pwszParameterList,
					rgpwszLongColumns[0],
					m_pRefCursorTable->GetTableName(),
					rgpwszColumns[0],
					m_pRefCursorTable->GetTableName(),
					m_pwszRefProcName,
					m_pwszPackageName
					);
		}
		else
			swprintf(
					*ppwszPackageBodyCreate, 
					s_wszDirectCursPkgBodyFmt,
					m_pwszPackageName,
					m_pwszRefProcName,
					pwszParameterList,
					m_pRefCursorTable->GetTableName(),
					m_pwszRefProcName,
					m_pwszPackageName
					);
	}

CLEANUP:
	PROVIDER_FREE(rgpwszColumns[0]);
	PROVIDER_FREE(rgpwszColumns[1]);
	PROVIDER_FREE(rgpwszLongColumns[0]);
	PROVIDER_FREE(pwszProcBody);

	return hr;
}


BOOL COraproc::TestRefCursorProc(BOOL fPrepare, IID riid, ULONG cProps, DBPROP* rgPropIDs, BOOL fSetParamInfo)
{
	HRESULT				hr = E_NOINTERFACE;
	DBROWCOUNT			cRowsAffected = 0;
	IUnknown*			pIUnknown = NULL;
	IColumnsInfo*		pIColumnsInfo = NULL;
	IColumnsRowset*		pIColumnsRowset = NULL;
	ICommandProperties*	pICommandProperties = NULL;
	IRowset*			pIRowset = NULL;

	if (cProps)
	{
		DBPROPSET	dbPropSet;

		dbPropSet.cProperties = cProps;
		dbPropSet.rgProperties = rgPropIDs;
		dbPropSet.guidPropertySet = DBPROPSET_ROWSET;
		
		TESTC(VerifyInterface(m_pICommand, IID_ICommandProperties, COMMAND_INTERFACE, (IUnknown**)&pICommandProperties));		

		// Don't care if this works; props may not be settable in some configurations
		TEST2C_(pICommandProperties->SetProperties(1, &dbPropSet), S_OK, DB_E_ERRORSOCCURRED);
	}

	// Execute the Proc
	TESTC_(hr = ExecuteProc(m_pwszCallRefProc, fPrepare, fSetParamInfo, riid, &cRowsAffected, &pIUnknown),S_OK);
	TESTC(pIUnknown != NULL);

	// Test QI/AddRef/Release
	TESTC(DefaultObjectTesting(pIUnknown, ROWSET_INTERFACE));

	// Verify Column Metadata
	TESTC(VerifyInterface(pIUnknown, IID_IColumnsInfo, ROWSET_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC(VerifyColumnsInfo(pIColumnsInfo));

	if(VerifyInterface(pIUnknown, IID_IColumnsRowset, ROWSET_INTERFACE, (IUnknown**)&pIColumnsRowset))
	{
		TESTC(DefaultObjectTesting(pIColumnsRowset, ROWSET_INTERFACE));
	}

	// Traverse Rowset and verify data
	TESTC(VerifyInterface(pIUnknown, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pIRowset));
	TESTC(VerifyRowset(pIRowset));

CLEANUP:

	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pICommandProperties);
	SAFE_RELEASE(pIRowset);
	
	if (cProps)
	{
		// Release old command object and create a new one
		// This is the only way to reset command properties
		CreateCommand();
	}

	return SUCCEEDED(hr);
}

BOOL COraproc::VerifyColumnsInfo(IColumnsInfo* pIColumnsInfo)
{
	HRESULT			hr;
	BOOL			fSuccess = FALSE;
	DBCOUNTITEM		cColumns = 0;
	DBCOUNTITEM		ulCount = 0;
	DBCOUNTITEM		ulIndex = 0;
	DBCOLUMNINFO*	rgInfo = NULL;
	WCHAR*			pStringsBuffer = NULL;
	CCol			ccol;

	TESTC_(hr=pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer),S_OK);
	for(ulIndex=0;ulIndex<cColumns;ulIndex++)
	{
		// Check for Ordinal 0
		if (rgInfo[ulIndex].iOrdinal==0)
		{
			COMPARE(rgInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK, DBCOLUMNFLAGS_ISBOOKMARK);
			COMPARE(2, (rgInfo[ulIndex].columnid).uName.ulPropid);
			COMPARE((rgInfo[0].columnid).uGuid.guid == DBCOL_SPECIALCOL, TRUE);
		}
		else
		{	
			// Get column info and increment ulCount
			TESTC_(m_pRefCursorTable->GetColInfo(++ulCount,ccol), S_OK)
			
			if(ccol.GetColName())
				COMPARE(wcscmp(rgInfo[ulIndex].pwszName,ccol.GetColName()), 0);

			COMPARE(rgInfo[ulIndex].pTypeInfo, NULL);
			COMPARE(rgInfo[ulIndex].iOrdinal, ulCount);
			COMPARE(rgInfo[ulIndex].ulColumnSize, ccol.GetColumnSize());
			COMPARE(rgInfo[ulIndex].wType, ccol.GetProviderType());
			COMPARE(rgInfo[ulIndex].bPrecision, ccol.GetPrecision());
			COMPARE(rgInfo[ulIndex].bScale, ccol.GetScale());
			COMPARE(CompareDBID(rgInfo[ulIndex].columnid, *(ccol.GetColID())), TRUE);
		}
	}

	fSuccess = TRUE;

CLEANUP:
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	return fSuccess;
}

BOOL COraproc::VerifyRowset(IRowset* pIRowset)
{
	BOOL		fSuccess = FALSE;
	HRESULT		hr;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW *		rghRows = NULL;
	HACCESSOR	hAccessor;
	DBBINDING * rgBindings = NULL;
	DBCOUNTITEM	cBindings = 0;
	DBLENGTH	cRowSize = 0;
	DBCOUNTITEM	i = 0;
	void *		pData = NULL;

	DBROWSTATUS ulRowStatus = DBROWSTATUS_S_OK;

	DBCOUNTITEM	ulRowCount = m_pRefCursorTable->CountRowsOnTable();

	hr = GetAccessorAndBindings(pIRowset, DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cRowSize, DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY,
		0, NULL, NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG);
	TESTC_(hr, S_OK);
	TESTC((pData = PROVIDER_ALLOC(cRowSize)) != NULL);

	//loop through the rowset
	for(i=m_ulVerifyRowStart; i<=ulRowCount; i++)
	{
		//GetNextRow 
		TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//VerifyRowHandle
		TESTC(cRowsObtained == 1);
		TESTC_(pIRowset->GetData(rghRows[0], hAccessor, pData),S_OK);		
		TESTC(CompareData(0, NULL, i, pData, cBindings, rgBindings, m_pRefCursorTable, NULL, PRIMARY, COMPARE_ONLY));
		
		//release the row handle
		CHECK(pIRowset->ReleaseRows(1, rghRows, NULL, NULL, &ulRowStatus), S_OK);
		COMPARE(ulRowStatus, DBROWSTATUS_S_OK);
		SAFE_FREE(rghRows);
	}

 	//Verify the cursor is at the end of the rowset
	TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	PROVIDER_FREE(rghRows);

	//Restart Position - should always be S_OK for ref cursor rowsets
	TEST2C_(pIRowset->RestartPosition(NULL), S_OK, DB_S_COMMANDREEXECUTED);
	
	// Verify that the special looping rules apply
	if( GetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, pIRowset) )
	{
		TESTC_(pIRowset->GetNextRows(NULL, 0, -1, &cRowsObtained, &rghRows), S_OK);
		TESTC(cRowsObtained==1);
	}

	fSuccess = TRUE;

CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	PROVIDER_FREE(rghRows);	
	PROVIDER_FREE(pData);
	
	return fSuccess;
}

void COraproc::SetRefCursorCallText(WCHAR* pwszCallProc)
{
	if (m_pwszCallRefProc)
		SAFE_FREE(m_pwszCallRefProc);
	m_pwszCallRefProc = wcsDuplicate(pwszCallProc);
}

BOOL COraproc::TestRefCursWParam(DWORD dwProcFlags, BOOL fPrepare, BOOL fSetParamInfo, ULONG ulSeed)
{
	BOOL  fPass = TEST_FAIL;
	WCHAR wszNumericBuf[MAXDATALEN];
	WCHAR wszExpectedNumeric[MAXDATALEN];
	WCHAR wszLongBuf[MAXDATALEN];
	WCHAR wszExpectedLongBuf[MAXDATALEN];
	WCHAR wszParamFirstInvokeFmt[] = L"{call %s.%s (?, {resultset 0, io_cursor})}";
	WCHAR wszParamLastInvokeFmt[] = L"{call %s.%s ({resultset 0, io_cursor}, ?)}";
	WCHAR wszTwoParamLastInvokeFmt[] = L"{call %s.%s ({resultset 0, io_cursor}, ?, ?)}";
	WCHAR* pwszParamInvokeFmt = NULL;
	WCHAR wszCallProc[MAXDATALEN];

	TESTC_(CreateRefCursorProc(dwProcFlags),S_OK);

	TESTC_(m_pRefCursorTable->MakeData(wszNumericBuf, ulSeed, m_rgParamColOrd[0], PRIMARY), S_OK);

	if (dwProcFlags & EXTRATWOPARAM_LAST)
	{
		pwszParamInvokeFmt = wszTwoParamLastInvokeFmt;
		TESTC_(m_pRefCursorTable->MakeData(wszExpectedLongBuf, ulSeed, m_rgParamLongColOrd[0], PRIMARY), S_OK);
	}
	else
	{
		TESTC_(m_pRefCursorTable->MakeData(wszExpectedNumeric, ulSeed, m_rgParamColOrd[1], PRIMARY), S_OK);

		if (dwProcFlags & EXTRAPARAM_FIRST)
			pwszParamInvokeFmt = wszParamFirstInvokeFmt;
		else if (dwProcFlags & EXTRAPARAM_LAST)
			pwszParamInvokeFmt = wszParamLastInvokeFmt;
	}

	swprintf(wszCallProc, pwszParamInvokeFmt, m_pwszPackageName, m_pwszRefProcName);
	SetRefCursorCallText(wszCallProc);

	if( dwProcFlags & EXTRATWOPARAM_LAST )
	{
		MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_WSTR,
				(void *)wszNumericBuf, 0, 0, L"number", DBTYPE_VARNUMERIC, 20);

		MakeBinding(2, DBPARAMIO_OUTPUT, DBTYPE_WSTR,
				(void *)wszLongBuf, 0, 0, L"long", DBTYPE_STR, 32760);
	}
	else
	{
		MakeBinding(1, DBPARAMIO_INPUT | DBPARAMIO_OUTPUT, DBTYPE_WSTR,
				(void *)wszNumericBuf, 0, 0, L"number", DBTYPE_VARNUMERIC, 20);
	}

	TESTC(TestRefCursorProc(fPrepare, IID_IRowset));

	// Check the output parameter
	if( dwProcFlags & EXTRATWOPARAM_LAST )
	{
		TESTC(CompareWCHARData
			(
				(BYTE *)m_pData + m_rgBindings[1].obValue,
				wszExpectedLongBuf, 
				DBTYPE_STR, 
				sizeof(WCHAR)*wcslen(wszExpectedLongBuf),
				sizeof(WCHAR)*wcslen((WCHAR *)((BYTE *)m_pData + m_rgBindings[1].obValue))							
			));
	}
	else
	{
		TESTC(CompareWCHARData
				(
					(BYTE *)m_pData + m_rgBindings[0].obValue,
					wszExpectedNumeric, 
					DBTYPE_VARNUMERIC, 
					sizeof(WCHAR)*wcslen(wszExpectedNumeric),
					sizeof(WCHAR)*wcslen((WCHAR *)((BYTE *)m_pData + m_rgBindings[0].obValue))							
				));
	}
	fPass = TEST_PASS;

CLEANUP:
	DropRefCursorProc();
	return fPass;
}

BOOL COraproc::VerifyProcColumns()
{
	BOOL				fPass = FALSE;
	HRESULT				hr;
	IDBSchemaRowset	*	pIDBSchemaRowset = NULL;
	IAccessor *			pIAccessor = NULL;
	IRowset *			pIRowset = NULL;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBCOUNTITEM			cRowsObtained = 0;
	DBCOUNTITEM			cCurRow = 0;
	DBORDINAL			ulOrdinal = 0;
	HROW *				pHRow = NULL;
	CCol				TempCol;
	VARIANT				rgRestrict[3];
	BYTE				pData[200];

	DBBINDSTATUS		rgBindStatus[7];
	DBBINDING			rgBinding[7] =
	{
		// COLUMN_NAME
		{4, 0, 0, 0, NULL, NULL, NULL, DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 50, 0, DBTYPE_WSTR, 0, 0}, 
		// ORDINAL POSITION
		{8, 52, 0, 0, NULL, NULL, NULL, DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI4, 0, 0}, 
		// DATA_TYPE
		{10, 56, 0, 0, NULL, NULL, NULL, DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI4, 0, 0}, 
		// CHARACTER_MAXIMUM_LENGTH
		{12, 68, 0, 64, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI4, 0, 0}, 
		// CHARACTER_OCTET_LENGTH
		{13, 76, 0, 72, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI4, 0, 0}, 
		// NUMERIC_PRECISION
		{14, 84, 0, 80, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_I4, 0, 0},
		// NUMERIC_SCALE
		{15, 92, 0, 88, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_I4, 0, 0}
	};

	GetSessionObject(IID_IDBSchemaRowset, (IUnknown **)&pIDBSchemaRowset);
	TESTC(pIDBSchemaRowset != NULL);

	VariantInit(&(rgRestrict[0]));
	VariantInit(&(rgRestrict[1]));
	VariantInit(&(rgRestrict[2]));

	V_VT(&(rgRestrict[2])) = VT_BSTR;
	V_BSTR(&(rgRestrict[2])) = SysAllocStringLen(NULL, (UINT)(wcslen(m_pwszPackageName)+wcslen(m_pwszRefProcName)+1+1) );
	wcscpy(V_BSTR(&(rgRestrict[2])), m_pwszPackageName);
	wcscat(V_BSTR(&(rgRestrict[2])), wszPERIOD);
	wcscat(V_BSTR(&(rgRestrict[2])), m_pwszRefProcName);

	TESTC_(hr=pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_PROCEDURE_COLUMNS, 3, rgRestrict, IID_IRowset, 0, NULL, (IUnknown **) &pIRowset),S_OK);
	TESTC(VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor));
	TESTC_(pIAccessor->CreateAccessor
						(
							DBACCESSOR_ROWDATA,
							NUMELEM(rgBinding),
							rgBinding,
							NULL,
							&hAccessor,
							rgBindStatus
						), S_OK);

	while ( S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow)) )
	{
		TESTC(COMPARE(cRowsObtained, 1));
		TESTC_(pIRowset->GetData(pHRow[0], hAccessor, (void *)pData), S_OK);
		
		TESTC(DBSTATUS_S_ISNULL != *(DBSTATUS *)(pData+rgBinding[1].obStatus));
		ulOrdinal = *(LONG *)(pData+rgBinding[1].obValue);
		TESTC(ulOrdinal > 0 && ulOrdinal <= m_pRefCursorTable->CountColumnsOnTable());

		TESTC_(m_pRefCursorTable->GetColInfo(ulOrdinal, TempCol),S_OK);

		// Check COLUMN_NAME
		COMPARE(0, wcscmp(TempCol.GetColName(), (WCHAR *)(pData+rgBinding[0].obValue)));

		// Check DATA_TYPE
		COMPARE(TempCol.GetProviderType(), *(DBTYPE *)(pData+rgBinding[2].obValue));

		// Check CHARACTER_MAXIMUM_LENGTH
		if (DBSTATUS_S_ISNULL != *(DBSTATUS *)(pData+rgBinding[3].obStatus))
			COMPARE(TempCol.GetColumnSize(), *(ULONG *)(pData+rgBinding[3].obValue));

		// Check CHARACTER_OCTET_LENGTH
		if (DBSTATUS_S_ISNULL != *(DBSTATUS *)(pData+rgBinding[4].obStatus))
			COMPARE(TempCol.GetColumnSize(), *(ULONG *)(pData+rgBinding[4].obValue));

		// Check NUMERIC PRECISION
		if( IsNumericType(TempCol.GetProviderType()) )
		{
			COMPARE(DBSTATUS_S_OK, *(DBSTATUS *)(pData+rgBinding[5].obStatus));
			COMPARE(TempCol.GetPrecision(), *(LONG *)(pData+rgBinding[5].obValue));
		}
		else 
			COMPARE(DBSTATUS_S_ISNULL, *(DBSTATUS *)(pData+rgBinding[5].obStatus));

		// Check NUMERIC SCALE
		if(	TempCol.GetProviderType() == DBTYPE_NUMERIC ||
			TempCol.GetProviderType() == DBTYPE_DECIMAL ||
			(TempCol.GetProviderType() == DBTYPE_VARNUMERIC &&
			 TempCol.GetScale() != 0) )
		{
			COMPARE(DBSTATUS_S_OK, *(DBSTATUS *)(pData+rgBinding[6].obStatus));		
			COMPARE(TempCol.GetScale(), *(LONG *)(pData+rgBinding[6].obValue));			
		}
		else
		{
			if( TempCol.GetProviderType() == DBTYPE_VARNUMERIC )
			{
				COMPARE(DBSTATUS_S_OK, *(DBSTATUS *)(pData+rgBinding[6].obStatus));		
				COMPARE(0,  *(LONG *)(pData+rgBinding[6].obValue));
			}
			else
				COMPARE(DBSTATUS_S_ISNULL, *(DBSTATUS *)(pData+rgBinding[6].obStatus));		
		}
		 		
		cCurRow++;
		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL), S_OK);			
	}			
	
	TESTC(cCurRow == m_pRefCursorTable->CountColumnsOnTable());
	fPass = TRUE;

CLEANUP:

	VariantClear(&(rgRestrict[2]));
	if (pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor,NULL);

	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBSchemaRowset);
	
	PROVIDER_FREE(pHRow);

	return fPass;
}

BOOL COraproc::VerifyColumnsSchema
(
	WCHAR *	pwszTableName,
	BOOL	fSynonym
)
{
	BOOL				fPass = FALSE;	
	HRESULT				hr;
	IRowset *			pIRowset = NULL;
	IAccessor *			pIAccessor = NULL;
	IDBSchemaRowset	*	pIDBSchemaRowset = NULL;

	typedef struct tagColInfo
	{
		 char *			szName;			// column name
		 ULONG			ulColumnFlags;	// column flags
		 VARIANT_BOOL	fNullable;		// nullability
		 DBTYPE			wType;			// column type
		 ULONG			ulCharMax;		// character maximum length
		 ULONG			ulOctetLen;		// character octet length
		 USHORT			usNumPrec;		// numeric precision
		 SHORT			sNumScale;		// numeric scale
		 ULONG			ulDateTimePrec;	// datetime precision
	} COLINFO;

	DBCOLUMNFLAGS	dbDefaultFlags = DBCOLUMNFLAGS_WRITEUNKNOWN |  DBCOLUMNFLAGS_ISNULLABLE | DBCOLUMNFLAGS_MAYBENULL;

    COLINFO arr_colinfo[] = 
		{
			// empty data for zeroth ordinal
			"", 0, VARIANT_FALSE, DBTYPE_EMPTY, 0, 0, 0, 0, 0,
			"I_C", dbDefaultFlags | DBCOLUMNFLAGS_ISFIXEDLENGTH, VARIANT_TRUE, DBTYPE_STR, 255, 255, 0, 0, 0,
			"I_V", dbDefaultFlags, VARIANT_TRUE, DBTYPE_STR, 255, 255, 0, 0, 0,
			"I_R", dbDefaultFlags, VARIANT_TRUE, DBTYPE_BYTES, 255, 255, 0, 0, 0,
			"I_L", dbDefaultFlags | DBCOLUMNFLAGS_ISLONG, VARIANT_TRUE, DBTYPE_STR, 2147483647, 2147483647, 0, 0, 0,
			"I_D", dbDefaultFlags | DBCOLUMNFLAGS_ISFIXEDLENGTH, VARIANT_TRUE, DBTYPE_DBTIMESTAMP, 0, 0, 0, 0, 0,
			"I_N", dbDefaultFlags, VARIANT_TRUE, DBTYPE_VARNUMERIC, 0, 0, 38, 0, 0,
			 // float is type double so precision and scale are 38,40
			"I_F", dbDefaultFlags | DBCOLUMNFLAGS_ISFIXEDLENGTH, VARIANT_TRUE, DBTYPE_R8, 0, 0, 15, 0, 0,
			 //int is of float type since it does not have scale
			"I_I", dbDefaultFlags | DBCOLUMNFLAGS_ISFIXEDLENGTH, VARIANT_TRUE, DBTYPE_NUMERIC, 0, 0, 38, 0, 0,
			"O_C", dbDefaultFlags | DBCOLUMNFLAGS_ISFIXEDLENGTH, VARIANT_TRUE, DBTYPE_STR, 255, 255, 0, 0, 0,
			"I_ROWID", dbDefaultFlags | DBCOLUMNFLAGS_ISFIXEDLENGTH | DBCOLUMNFLAGS_ISROWID, VARIANT_TRUE, DBTYPE_STR, 18, 18, 0, 0, 0,
		};

	DBBINDING rgBinding[10] =
	{
		// column name
		{4, 0, 0, 0, NULL, NULL, NULL, DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 50, 0, DBTYPE_STR, 0, 0}, 
		// ordinal pos.
		{7, 56, 0, 52, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_I4, 0, 0}, 
		// column flags
		{10, 60, 0, 0, NULL, NULL, NULL, DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI4, 0, 0}, 
		// nullability
		{11, 68, 0, 64, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_BOOL, 0, 0},
		// data type
		{12, 76, 0, 72, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI2, 0, 0}, 
		// char maximum length
		{14, 84, 0, 80, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI4, 0, 0}, 
		// char octet length
		{15, 92, 0, 88, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI4, 0, 0}, 
		// numeric precision
		{16, 100, 0, 96, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI2, 0, 0}, 
		// numeric scale
		{17, 108, 0, 104, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_I2, 0, 0},
		// datetime precision
		{18, 116, 0, 112, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_UI4, 0, 0} 
	};
	      
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBBINDSTATUS	rgBindStatus[10];
	DBCOUNTITEM		cRowsObtained = 0;
	DBCOUNTITEM		cCurRow = 1;
	DB_LORDINAL		lParamOrdinal = 0;
	HROW			hrow = DB_NULL_HROW;
	HROW *			pHRow = &hrow;
	BYTE			pData[200];
	VARIANT			rgRestrict[3];

	VariantInit(&(rgRestrict[0]));
	VariantInit(&(rgRestrict[1]));
	VariantInit(&(rgRestrict[2]));

	if( fSynonym )
		V_VT(&(rgRestrict[1])) = VT_NULL;
	V_VT(&(rgRestrict[2])) = VT_BSTR;
	V_BSTR(&(rgRestrict[2])) = SysAllocString(pwszTableName);

	GetSessionObject(IID_IDBSchemaRowset, (IUnknown **)&pIDBSchemaRowset);
	TESTC_(hr=pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_COLUMNS, 3, rgRestrict, IID_IRowset,
					0, NULL, (IUnknown **)&pIRowset),S_OK);

	VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor);	

	TESTC_(pIAccessor->CreateAccessor(
						DBACCESSOR_ROWDATA,
						NUMELEM(rgBinding),
						rgBinding,
						NULL,
						&hAccessor,
						rgBindStatus
						), S_OK);

	while ( S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow)) )
	{
		TESTC(COMPARE(cRowsObtained, 1));
		TESTC_(pIRowset->GetData(pHRow[0], hAccessor, (void *)pData), S_OK);
		
		// Is the ordinal position NULL?
		// then assume parameters are returned sequentially.
		if (DBSTATUS_S_ISNULL == *(DBSTATUS *)(pData+rgBinding[1].obStatus))
			lParamOrdinal = cCurRow;
		else
			lParamOrdinal = *(LONG *)(pData+rgBinding[1].obValue);
		
		// check name and type
		COMPARE(0, strcmp(arr_colinfo[lParamOrdinal].szName, (char *)(pData+rgBinding[0].obValue)));
		COMPARE(arr_colinfo[lParamOrdinal].wType, *(DBTYPE *)(pData+rgBinding[4].obValue));

		COMPARE(arr_colinfo[lParamOrdinal].ulColumnFlags, *(ULONG *)(pData+rgBinding[2].obValue));
		COMPARE(arr_colinfo[lParamOrdinal].fNullable, *(VARIANT_BOOL *)(pData+rgBinding[3].obValue));

		if( DBTYPE_STR == arr_colinfo[lParamOrdinal].wType || 
			DBTYPE_WSTR == arr_colinfo[lParamOrdinal].wType ||
			DBTYPE_BYTES == arr_colinfo[lParamOrdinal].wType )
		{
			COMPARE(DBSTATUS_S_OK, *(DBSTATUS *)(pData+rgBinding[5].obStatus));
			COMPARE(DBSTATUS_S_OK, *(DBSTATUS *)(pData+rgBinding[6].obStatus));
			COMPARE(arr_colinfo[lParamOrdinal].ulCharMax, *(ULONG *)(pData+rgBinding[5].obValue));
			COMPARE(arr_colinfo[lParamOrdinal].ulOctetLen, *(ULONG *)(pData+rgBinding[6].obValue));
		}
		else
		{
			COMPARE(DBSTATUS_S_ISNULL, *(DBSTATUS *)(pData+rgBinding[5].obStatus));
			COMPARE(DBSTATUS_S_ISNULL, *(DBSTATUS *)(pData+rgBinding[6].obStatus));
		}

		if( DBTYPE_NUMERIC == arr_colinfo[lParamOrdinal].wType || 
			DBTYPE_VARNUMERIC == arr_colinfo[lParamOrdinal].wType ||
			DBTYPE_R8 == arr_colinfo[lParamOrdinal].wType )
		{
			COMPARE(DBSTATUS_S_OK, *(DBSTATUS *)(pData+rgBinding[7].obStatus));
			COMPARE(arr_colinfo[lParamOrdinal].usNumPrec, *(USHORT *)(pData+rgBinding[7].obValue));


			if( DBTYPE_R8 == arr_colinfo[lParamOrdinal].wType )
			{
				// Scale only applies to NUMERIC/DECIMAL
				COMPARE(DBSTATUS_S_ISNULL, *(DBSTATUS *)(pData+rgBinding[8].obStatus));			
			}
			else
			{
				COMPARE(DBSTATUS_S_OK, *(DBSTATUS *)(pData+rgBinding[8].obStatus));			
				COMPARE(arr_colinfo[lParamOrdinal].sNumScale, *(SHORT *)(pData+rgBinding[8].obValue));
			}
			
		}
		else
		{
			COMPARE(DBSTATUS_S_ISNULL, *(DBSTATUS *)(pData+rgBinding[7].obStatus));
			COMPARE(DBSTATUS_S_ISNULL, *(DBSTATUS *)(pData+rgBinding[8].obStatus));
		}

		if( DBTYPE_DBTIMESTAMP == arr_colinfo[lParamOrdinal].wType )
		{
			COMPARE(DBSTATUS_S_OK, *(DBSTATUS *)(pData+rgBinding[9].obStatus));
			COMPARE(arr_colinfo[lParamOrdinal].ulDateTimePrec, *(ULONG *)(pData+rgBinding[9].obValue));
		}
		else
			COMPARE(DBSTATUS_S_ISNULL, *(DBSTATUS *)(pData+rgBinding[9].obStatus));

		 		
		cCurRow++;
		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL), S_OK);			
	}
	
	//  arr_colinfo is one-based
	TESTC(cCurRow == NUMELEM(arr_colinfo));
	fPass = TRUE;

CLEANUP:

	VariantClear(&(rgRestrict[2]));

	if (pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor,NULL);

	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBSchemaRowset);

	return fPass;
}

BOOL COraproc::CreateArrayPkgFromTable
(
	CTable *	pTable,
	WCHAR **	ppwszCreatePkg,
	WCHAR **	ppwszCreatePkgBody,
	WCHAR **	ppwszCallProc,
	WCHAR **	ppwszDropPkg
)
{
	static WCHAR s_wszCreate_PKGFMT[] =
		L"CREATE OR REPLACE PACKAGE %s AS \n %s "
		L"PROCEDURE %s(%s); \n"
		L"END %s; \n";

	static WCHAR s_wszCreate_PKGBODYFMT[] =
		L"Create OR replace PACKAGE BODY %s AS \n"
		L" PROCEDURE %s(%s) \n"
        L" IS \n"
		L" CURSOR test_cur IS SELECT * from %s; \n"
        L" test_count NUMBER DEFAULT 1; \n"
		L" BEGIN \n"
		L" FOR test_rec in test_cur \n"
		L"  LOOP \n"
		L"   %s \n"
		L"   test_count := test_count + 1; \n"
		L"  END LOOP; \n"
		L" END %s; \n "
		L"END %s; \n"; 
	
	static WCHAR s_wszCALLFMT[] = L"{call %s.%s( {resultset %d, %s} ) }";

	static WCHAR s_wszTypeFMT[] = L"TYPE t_%d is TABLE of %s INDEX BY BINARY_INTEGER;\n";
	static WCHAR s_wszParamFMT[] = L"%s OUT t_%d";
    static WCHAR s_wszAssignFMT[] = L"%s(test_count) := test_rec.%s;\n";
	WCHAR *	pwszPkgName = NULL;
	WCHAR *	pwszProcName = NULL;
	WCHAR *	pwszParamList = NULL;
	WCHAR *	pwszTypeList = NULL;
	WCHAR *	pwszAssignList = NULL;
	WCHAR * pwszArgList = NULL;
	WCHAR *	pwszTypeDef = NULL;
	WCHAR *	pwszTypeParams = NULL;

	ULONG	cIter = 0;
	CCol	TempCol;
	HRESULT	hr = E_FAIL;

	ASSERT(pTable && ppwszCreatePkg && ppwszCreatePkgBody && ppwszDropPkg);

	*ppwszCreatePkg = NULL;
	*ppwszCreatePkgBody = NULL;
	*ppwszDropPkg = NULL;

	pwszPkgName = MakeObjectName(pTable->GetModuleName(), ORAMAX_TABLELEN);
	pwszProcName = MakeObjectName(pTable->GetModuleName(), ORAMAX_TABLELEN);
	TESTC(pwszPkgName != NULL && pwszProcName != NULL);

	pwszTypeList = (WCHAR *)PROVIDER_ALLOC((wcslen(s_wszTypeFMT) + ORAMAX_COLLEN + 10) * sizeof(WCHAR) * pTable->CountColumnsOnSchema());
	pwszParamList = (WCHAR *)PROVIDER_ALLOC((wcslen(s_wszParamFMT) + ORAMAX_COLLEN + 10) * sizeof(WCHAR) * pTable->CountColumnsOnSchema());
	pwszAssignList = (WCHAR *)PROVIDER_ALLOC((wcslen(s_wszParamFMT) + 2*ORAMAX_COLLEN) * sizeof(WCHAR) * pTable->CountColumnsOnSchema());
	pwszArgList = (WCHAR *)PROVIDER_ALLOC((ORAMAX_COLLEN+1)* sizeof(WCHAR) * pTable->CountColumnsOnSchema());
	
	TESTC(pwszTypeList && pwszParamList && pwszAssignList);

	*pwszTypeList = L'\0';
	*pwszParamList = L'\0';
	*pwszAssignList = L'\0';
	*pwszArgList = L'\0';

	for (cIter=1; cIter<=pTable->CountColumnsOnSchema(); cIter++)
	{
		WCHAR	wszTypeItem[MAXBUFLEN];
		WCHAR	wszParamItem[MAXBUFLEN];
		WCHAR	wszTableType[MAXBUFLEN];
		WCHAR	wszAssignItem[MAXBUFLEN];
		
		TESTC(SUCCEEDED(pTable->GetColInfo(cIter, TempCol)));

		wcscpy(wszTableType, TempCol.GetProviderTypeName());
		TempCol.CreateColDef(&pwszTypeDef);
		if( pwszTypeDef != NULL )
		{
			pwszTypeParams = wcsstr(pwszTypeDef, L"(");
			if( pwszTypeParams )
				wcscat(wszTableType, pwszTypeParams);
		}

		swprintf(wszTypeItem, s_wszTypeFMT, cIter, wszTableType);
		swprintf(wszParamItem, s_wszParamFMT, TempCol.GetColName(), cIter);
		swprintf(wszAssignItem, s_wszAssignFMT, TempCol.GetColName(), TempCol.GetColName());

		wcscat(pwszTypeList, wszTypeItem);
		wcscat(pwszParamList, wszParamItem);
		wcscat(pwszAssignList, wszAssignItem);
		wcscat(pwszArgList, TempCol.GetColName());

		if(cIter != pTable->CountColumnsOnSchema())
		{
			wcscat(pwszParamList, L",");
			wcscat(pwszArgList, L",");
		}
	}

	*ppwszCreatePkg = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(s_wszCreate_PKGFMT)+2*wcslen(pwszPkgName)+wcslen(pwszProcName)+wcslen(pwszTypeList)+wcslen(pwszParamList)+1));
	*ppwszCreatePkgBody = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(s_wszCreate_PKGBODYFMT)+2*wcslen(pwszPkgName)+2*wcslen(pwszProcName)+wcslen(pTable->GetTableName())+wcslen(pwszParamList)+wcslen(pwszAssignList)+1));
	*ppwszCallProc = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(s_wszCALLFMT)+wcslen(pwszPkgName)+wcslen(pwszProcName)+wcslen(pwszArgList)+10));
	*ppwszDropPkg = (WCHAR *)PROVIDER_ALLOC(MAXBUFLEN);

	TESTC(*ppwszCreatePkg && *ppwszCreatePkgBody && *ppwszDropPkg);

	swprintf(*ppwszCreatePkg, s_wszCreate_PKGFMT, pwszPkgName, pwszTypeList, pwszProcName, pwszParamList, pwszPkgName);
	swprintf(*ppwszCreatePkgBody, s_wszCreate_PKGBODYFMT, pwszPkgName, pwszProcName, pwszParamList,
		pTable->GetTableName(), pwszAssignList, pwszProcName, pwszPkgName);

	swprintf(*ppwszCallProc, s_wszCALLFMT, pwszPkgName, pwszProcName, pTable->CountRowsOnTable(), pwszArgList);
	
	wcscpy(*ppwszDropPkg, L"drop package ");
	wcscat(*ppwszDropPkg, pwszPkgName);

	hr = S_OK;

CLEANUP:

	SAFE_FREE(pwszTypeDef);
	SAFE_FREE(pwszTypeList);
	SAFE_FREE(pwszParamList);
	SAFE_FREE(pwszAssignList);

	if( FAILED(hr) )
	{
		SAFE_FREE(*ppwszCreatePkg);
		SAFE_FREE(*ppwszCreatePkgBody);
		SAFE_FREE(*ppwszCallProc);
		SAFE_FREE(*ppwszDropPkg);
	}

	return SUCCEEDED(hr);
}


BOOL COraproc::CheckNativeError
(
	HRESULT		hr,
	IID			iid,
	DBCOUNTITEM cRecordsExpected
)
{
	IErrorInfo *	pIErrorInfo = NULL;
	IErrorInfo *	pRecordIErrorInfo = NULL;
	IErrorRecords *	pIErrorRecords = NULL;
	BSTR			bstrDescription = NULL;
	GUID			guid;
	ULONG			cRecords = 0;
	ULONG			cIter = 0;
	ERRORINFO		ErrorInfo;
	DWORD			dwMinor = 0;
	BOOL			fPass = TEST_FAIL;

	TESTC_(GetErrorInfo(0, &pIErrorInfo), S_OK);
	TESTC(pIErrorInfo != NULL);

	TESTC(VerifyInterface(pIErrorInfo, IID_IErrorRecords, ERROR_INTERFACE,
			(IUnknown **)&pIErrorRecords));

	TESTC_(pIErrorRecords->GetRecordCount(&cRecords), S_OK);
	TESTC(cRecordsExpected == cRecords);

	for( cIter = 0; cIter < cRecords; cIter++ )
	{
		TESTC_(pIErrorRecords->GetErrorInfo(cIter, GetUserDefaultLCID(), &pRecordIErrorInfo), S_OK);
		TESTC_(pRecordIErrorInfo->GetDescription(&bstrDescription), S_OK);
		TESTC_(pRecordIErrorInfo->GetGUID(&guid), S_OK);
		TESTC(guid == iid);

		TESTC_(pIErrorRecords->GetBasicErrorInfo(cIter, &ErrorInfo), S_OK);
		TESTC(ErrorInfo.hrError == hr);
		TESTC(ErrorInfo.iid == iid);

		// Extract the native oracle error from bstrDescription
		// and compare against the posted native error
		TESTC(1 == swscanf(bstrDescription, L"ORA-%ud", &dwMinor));
		TESTC(dwMinor == ErrorInfo.dwMinor);

		SAFE_RELEASE(pRecordIErrorInfo);
		SAFE_SYSFREE(bstrDescription);	
	}

	fPass = TEST_PASS;

CLEANUP:

	SAFE_SYSFREE(bstrDescription);

	SAFE_RELEASE(pIErrorInfo);
	SAFE_RELEASE(pRecordIErrorInfo);
	SAFE_RELEASE(pIErrorRecords);

	return fPass;
}


// {{ TCW_TEST_CASE_MAP(TestNulls)
//*-----------------------------------------------------------------------
// @class Test unbound accessor parts
//
class TestNulls : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TestNulls,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Only Length and Status bound
	int Variation_1();
	// @cmember Only Length and Status bound for INPUT
	int Variation_2();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TestNulls)
#define THE_CLASS TestNulls
BEG_TEST_CASE(TestNulls, COraproc, L"Test unbound accessor parts")
	TEST_VARIATION(1, 		L"Only Length and Status bound")
	TEST_VARIATION(2, 		L"Only Length and Status bound for INPUT")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TestNullStatus)
//*-----------------------------------------------------------------------
// @class Use DBSTATUS_S_ISNULL status
//
class TestNullStatus : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TestNullStatus,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Use DBSTATUS_S_ISNULL for numeric parameter
	int Variation_1();
	// @cmember Use DBSTATUS_S_ISNULL for char parameter
	int Variation_2();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TestNullStatus)
#define THE_CLASS TestNullStatus
BEG_TEST_CASE(TestNullStatus, COraproc, L"Use DBSTATUS_S_ISNULL status")
	TEST_VARIATION(1, 		L"Use DBSTATUS_S_ISNULL for numeric parameter")
	TEST_VARIATION(2, 		L"Use DBSTATUS_S_ISNULL for char parameter")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(DefaultParam)
//*-----------------------------------------------------------------------
// @class Use DBSTATUS_S_DEFAULT
//
class DefaultParam : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DefaultParam,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Character Proc, Prepare, don't use DBSTATUS_S_DEFAULT
	int Variation_1();
	// @cmember Character proc, Prepare, use DBSTATUS_S_DEFAULT
	int Variation_2();
	// @cmember Char proc, no prepare, don't use DEFAULT status
	int Variation_3();
	// @cmember Char proc, no prepare, use DEFAULT status
	int Variation_4();
	// @cmember Numeric, prepare, no default
	int Variation_5();
	// @cmember Numeric, prepare, use default
	int Variation_6();
	// @cmember Numeric, no prepare, no default
	int Variation_7();
	// @cmember Numeric, no prepare, use default
	int Variation_8();
	// @cmember Date, no prepare, no setparaminfo
	int Variation_9();
	// @cmember Date, prepared, no setparaminfo
	int Variation_10();
	// @cmember Date, no prepare, setparaminfo
	int Variation_11();
	// @cmember Date, prepared, setparaminfo
	int Variation_12();
	// @cmember Varnumeric, no default, no getparaminfo
	int Variation_13();
	// @cmember Varnumeric, no default, getparaminfo
	int Variation_14();
	// @cmember Varnumeric, default, no getparaminfo
	int Variation_15();
	// @cmember Varnumeric, default, getparaminfo
	int Variation_16();
	// @cmember  use DBTYPE_DATE binding, no prepare, NOsetparaminfo
	int Variation_17();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(DefaultParam)
#define THE_CLASS DefaultParam
BEG_TEST_CASE(DefaultParam, COraproc, L"Use DBSTATUS_S_DEFAULT")
	TEST_VARIATION(1, 		L"Character Proc, Prepare, don't use DBSTATUS_S_DEFAULT")
	TEST_VARIATION(2, 		L"Character proc, Prepare, use DBSTATUS_S_DEFAULT")
	TEST_VARIATION(3, 		L"Char proc, no prepare, don't use DEFAULT status")
	TEST_VARIATION(4, 		L"Char proc, no prepare, use DEFAULT status")
	TEST_VARIATION(5, 		L"Numeric, prepare, no default")
	TEST_VARIATION(6, 		L"Numeric, prepare, use default")
	TEST_VARIATION(7, 		L"Numeric, no prepare, no default")
	TEST_VARIATION(8, 		L"Numeric, no prepare, use default")
	TEST_VARIATION(9, 		L"Date, no prepare, no setparaminfo")
	TEST_VARIATION(10, 		L"Date, prepared, no setparaminfo")
	TEST_VARIATION(11, 		L"Date, no prepare, setparaminfo")
	TEST_VARIATION(12, 		L"Date, prepared, setparaminfo")
	TEST_VARIATION(13, 		L"Varnumeric, no default, no getparaminfo")
	TEST_VARIATION(14, 		L"Varnumeric, no default, getparaminfo")
	TEST_VARIATION(15, 		L"Varnumeric, default, no getparaminfo")
	TEST_VARIATION(16, 		L"Varnumeric, default, getparaminfo")
	TEST_VARIATION(17, 		L"use DBTYPE_DATE binding, no prepare, NOsetparaminfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(ParamLen)
//--------------------------------------------------------------------
// @class Use param len from PROCEDURE PARAMETERS schema
//
class ParamLen : public COraproc {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	WCHAR	m_wszDropFunc[MAXBUFLEN];
	WCHAR	m_wszDropTable[MAXBUFLEN];
	WCHAR	m_wszTableName[MAXBUFLEN];
	WCHAR	m_wszFunctionName[MAXBUFLEN];

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ParamLen,COraproc);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Verify PROCEDURE_PARAMETERS schema
	int Variation_1();
	// @cmember Verify COLUMNS schema
	int Variation_2();
	// @cmember Verify COLUMNS schema with SYNONYM
	int Variation_3();
	// }}
} ;
// {{ TCW_TESTCASE(ParamLen)
#define THE_CLASS ParamLen
BEG_TEST_CASE(ParamLen, COraproc, L"Use param len from PROCEDURE PARAMETERS schema")
	TEST_VARIATION(1,		L"Verify PROCEDURE_PARAMETERS schema")
	TEST_VARIATION(2,		L"Verify COLUMNS schema")
	TEST_VARIATION(3,		L"Verify COLUMNS schema with SYNONYM")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Scalar)
//*-----------------------------------------------------------------------
// @class Use scalar values
//
class Scalar : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Scalar,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember LONGRAW, no prepare, no setparaminfo
	int Variation_1();
	// @cmember LONGRAW, prepared, no setparaminfo
	int Variation_2();
	// @cmember LONGRAW, no prepare, setparaminfo
	int Variation_3();
	// @cmember LONGRAW,  prepared, setparaminfo
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Scalar)
#define THE_CLASS Scalar
BEG_TEST_CASE(Scalar, COraproc, L"Use scalar values")
	TEST_VARIATION(1, 		L"LONGRAW, no prepare, no setparaminfo")
	TEST_VARIATION(2, 		L"LONGRAW, prepared, no setparaminfo")
	TEST_VARIATION(3, 		L"LONGRAW, no prepare, setparaminfo")
	TEST_VARIATION(4, 		L"LONGRAW,  prepared, setparaminfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(ArrayParameters)
//*-----------------------------------------------------------------------
// @class Use PLSQL Parameter arrays
//
class ArrayParameters : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ArrayParameters,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Fetch NUMBER array, no prepare, no setparaminfo
	int Variation_1();
	// @cmember Fetch NUMBER array, prepared, no setparaminfo
	int Variation_4();
	// @cmember Fetch NUMBER array, no prepare, setparaminfo
	int Variation_5();
	// @cmember Fetch NUMBER array, prepared, setparaminfo
	int Variation_6();
	// @cmember Fetch Large RS, no prepare, no setparaminfo
	int Variation_7();
	// @cmember Fetch Large RS, prepared, no setparaminfo
	int Variation_8();
	// @cmember Fetch Large RS, no prepare, setparaminfo
	int Variation_9();
	// @cmember Fetch Large RS, prepared, setparaminfo
	int Variation_10();
	// @cmember Basic Mult Res, no prepare, no setparaminfo
	int Variation_11();
	// @cmember Basic Mult Res, prepared, no setparaminfo
	int Variation_12();
	// @cmember Basic Mult Res, no prepare, setparaminfo
	int Variation_13();
	// @cmember Basic Mult Res,  prepared, setparaminfo
	int Variation_14();
	// @cmember Fetch DATE array, no prepare, no setparaminfo
	int Variation_15();
	// @cmember Fetch DATE array, prepared, no setparaminfo
	int Variation_16();
	// @cmember Fetch DATE array, no prepare, setparaminfo
	int Variation_17();
	// @cmember Fetch DATE array, prepared, setparaminfo
	int Variation_18();
	// @cmember Fetch RAW array, no prepare, no setparaminfo
	int Variation_19();
	// @cmember Fetch RAW array, prepared, no setparaminfo
	int Variation_20();
	// @cmember Fetch RAW array, no prepare, setparaminfo
	int Variation_21();
	// @cmember Fetch RAW array, prepared, setparaminfo
	int Variation_22();
	// @cmember Fetch VARNUM array, no prepare, no setparaminfo, negscale
	int Variation_23();
	// @cmember Fetch VARNUM array, prepared, no setparaminfo, negscale
	int Variation_24();
	// @cmember Fetch VARNUM array, no prepare, setparaminfo, negscale
	int Variation_25();
	// @cmember Fetch VARNUM array, prepared, setparaminfo, negscale
	int Variation_26();
	// @cmember Fetch VARNUM array, no prepare, no setparaminfo, noscale
	int Variation_27();
	// @cmember Fetch VARNUM array, prepared, no setparaminfo, noscale
	int Variation_28();
	// @cmember Fetch VARNUM array, no prepare, setparaminfo, noscale
	int Variation_29();
	// @cmember Fetch VARNUM array, prepared, setparaminfo, noscale
	int Variation_30();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(ArrayParameters)
#define THE_CLASS ArrayParameters
BEG_TEST_CASE(ArrayParameters, COraproc, L"Use PLSQL Parameter arrays")
	TEST_VARIATION(1, 		L"Fetch NUMBER array, no prepare, no setparaminfo")
	TEST_VARIATION(4, 		L"Fetch NUMBER array, prepared, no setparaminfo")
	TEST_VARIATION(5, 		L"Fetch NUMBER array, no prepare, setparaminfo")
	TEST_VARIATION(6, 		L"Fetch NUMBER array, prepared, setparaminfo")
	TEST_VARIATION(7, 		L"Fetch Large RS, no prepare, no setparaminfo")
	TEST_VARIATION(8, 		L"Fetch Large RS, prepared, no setparaminfo")
	TEST_VARIATION(9, 		L"Fetch Large RS, no prepare, setparaminfo")
	TEST_VARIATION(10, 		L"Fetch Large RS, prepared, setparaminfo")
	TEST_VARIATION(11, 		L"Basic Mult Res, no prepare, no setparaminfo")
	TEST_VARIATION(12, 		L"Basic Mult Res, prepared, no setparaminfo")
	TEST_VARIATION(13, 		L"Basic Mult Res, no prepare, setparaminfo")
	TEST_VARIATION(14, 		L"Basic Mult Res,  prepared, setparaminfo")
	TEST_VARIATION(15, 		L"Fetch DATE array, no prepare, no setparaminfo")
	TEST_VARIATION(16, 		L"Fetch DATE array, prepared, no setparaminfo")
	TEST_VARIATION(17, 		L"Fetch DATE array, no prepare, setparaminfo")
	TEST_VARIATION(18, 		L"Fetch DATE array, prepared, setparaminfo")
	TEST_VARIATION(19, 		L"Fetch RAW array, no prepare, no setparaminfo")
	TEST_VARIATION(20, 		L"Fetch RAW array, prepared, no setparaminfo")
	TEST_VARIATION(21, 		L"Fetch RAW array, no prepare, setparaminfo")
	TEST_VARIATION(22, 		L"Fetch RAW array, prepared, setparaminfo")
	TEST_VARIATION(23, 		L"Fetch VARNUM array, no prepare, no setparaminfo, negscale")
	TEST_VARIATION(24, 		L"Fetch VARNUM array, prepared, no setparaminfo, negscale")
	TEST_VARIATION(25, 		L"Fetch VARNUM array, no prepare, setparaminfo, negscale")
	TEST_VARIATION(26, 		L"Fetch VARNUM array, prepared, setparaminfo, negscale")
	TEST_VARIATION(27, 		L"Fetch VARNUM array, no prepare, no setparaminfo, noscale")
	TEST_VARIATION(28, 		L"Fetch VARNUM array, prepared, no setparaminfo, noscale")
	TEST_VARIATION(29, 		L"Fetch VARNUM array, no prepare, setparaminfo, noscale")
	TEST_VARIATION(30, 		L"Fetch VARNUM array, prepared, setparaminfo, noscale")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(PLSQL_Types)
//*-----------------------------------------------------------------------
// @class Test all PL/SQL Types
//
class PLSQL_Types : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(PLSQL_Types,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Char Output parameter, 2k size, no prepare, no setparaminfo
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(PLSQL_Types)
#define THE_CLASS PLSQL_Types
BEG_TEST_CASE(PLSQL_Types, COraproc, L"Test all PL/SQL Types")
	TEST_VARIATION(1, 		L"Char Output parameter, 2k size, no prepare, no setparaminfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Properties)
//*-----------------------------------------------------------------------
// @class Properties
//
class Properties : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Properties,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Key Column Property
	int Variation_1();
	// @cmember Key Column Property based on UNIQUE index
	int Variation_2();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Properties)
#define THE_CLASS Properties
BEG_TEST_CASE(Properties, COraproc, L"Properties")
	TEST_VARIATION(1, 		L"Key Column Property")
	TEST_VARIATION(2, 		L"Key Column Property based on UNIQUE index")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(Misc)
//*-----------------------------------------------------------------------
// @class Misc
//
class Misc : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Misc,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember LONG parameter info in proc
	int Variation_1();
	// @cmember Long error message
	int Variation_2();
	// @cmember SCALEISNEGATIVE check for parameters
	int Variation_3();
	// @cmember Native error on DB_E_ERRORSINCOMMAND
	int Variation_4();
	// @cmember Native error on E_FAIL
	int Variation_5();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(Misc)
#define THE_CLASS Misc
BEG_TEST_CASE(Misc, COraproc, L"Misc")
	TEST_VARIATION(1, 		L"LONG parameter info in proc")
	TEST_VARIATION(2, 		L"Long error message")
	TEST_VARIATION(3, 		L"SCALEISNEGATIVE check for parameters")
	TEST_VARIATION(4, 		L"Native error on DB_E_ERRORSINCOMMAND")
	TEST_VARIATION(5, 		L"Native error on E_FAIL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(RefCursor_Misc)
//*-----------------------------------------------------------------------
// @class Miscellaneous REF CURSOR cases
//
class RefCursor_Misc : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RefCursor_Misc,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Weak Cursor, Param Before, No prepare, No setparaminfo
	int Variation_1();
	// @cmember Weak Cursor, Param After, No prepare, No setparaminfo
	int Variation_2();
	// @cmember Invoke ref cursor with a literal before escape clause
	int Variation_3();
	// @cmember Invoke ref cursor with a literal after escape clause
	int Variation_4();
	// @cmember Strong Cursor, Param Before, No prepare, No setparaminfo
	int Variation_5();
	// @cmember Strong Cursor, Param After, No prepare, No setparaminfo
	int Variation_6();
	// @cmember Weak Cursor, Param Before, Prepare, No setparaminfo
	int Variation_7();
	// @cmember Weak Cursor, Param After, Prepare, No setparaminfo
	int Variation_8();
	// @cmember Strong Cursor, Param Before, Prepare, No setparaminfo
	int Variation_9();
	// @cmember Strong Cursor, Param After, Prepare, No setparaminfo
	int Variation_10();
	// @cmember Weak Cursor, Param Before, No prepare, Setparaminfo
	int Variation_11();
	// @cmember Weak Cursor, Param After, No prepare, Setparaminfo
	int Variation_12();
	// @cmember Strong Cursor, Param Before, No prepare, Setparaminfo
	int Variation_13();
	// @cmember Strong Cursor, Param After, No prepare, Setparaminfo
	int Variation_14();
	// @cmember Weak Cursor, Param Before, Prepare, Setparaminfo
	int Variation_15();
	// @cmember Weak Cursor, Param After, Prepare, Setparaminfo
	int Variation_16();
	// @cmember Strong Cursor, Param Before, Prepare, Setparaminfo
	int Variation_17();
	// @cmember Strong Cursor, Param After, Prepare, Setparaminfo
	int Variation_18();
	// @cmember Retrieve partially fetched cursor
	int Variation_19();

	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(RefCursor_Misc)
#define THE_CLASS RefCursor_Misc
BEG_TEST_CASE(RefCursor_Misc, COraproc, L"Miscellaneous REF CURSOR cases")
	TEST_VARIATION(1, 		L"Weak Cursor, Param Before, No prepare, No setparaminfo")
	TEST_VARIATION(2, 		L"Weak Cursor, Param After, No prepare, No setparaminfo")
	TEST_VARIATION(3, 		L"Invoke ref cursor with a literal before escape clause")
	TEST_VARIATION(4, 		L"Invoke ref cursor with a literal after escape clause")
	TEST_VARIATION(5, 		L"Strong Cursor, Param Before, No prepare, No setparaminfo")
	TEST_VARIATION(6, 		L"Strong Cursor, Param After, No prepare, No setparaminfo")
	TEST_VARIATION(7, 		L"Weak Cursor, Param Before, Prepare, No setparaminfo")
	TEST_VARIATION(8, 		L"Weak Cursor, Param After, Prepare, No setparaminfo")
	TEST_VARIATION(9, 		L"Strong Cursor, Param Before, Prepare, No setparaminfo")
	TEST_VARIATION(10, 		L"Strong Cursor, Param After, Prepare, No setparaminfo")
	TEST_VARIATION(11, 		L"Weak Cursor, Param Before, No prepare, Setparaminfo")
	TEST_VARIATION(12, 		L"Weak Cursor, Param After, No prepare, Setparaminfo")
	TEST_VARIATION(13, 		L"Strong Cursor, Param Before, No prepare, Setparaminfo")
	TEST_VARIATION(14, 		L"Strong Cursor, Param After, No prepare, Setparaminfo")
	TEST_VARIATION(15, 		L"Weak Cursor, Param Before, Prepare, Setparaminfo")
	TEST_VARIATION(16, 		L"Weak Cursor, Param After, Prepare, Setparaminfo")
	TEST_VARIATION(17, 		L"Strong Cursor, Param Before, Prepare, Setparaminfo")
	TEST_VARIATION(18, 		L"Strong Cursor, Param After, Prepare, Setparaminfo")
	TEST_VARIATION(19, 		L"Retrieve partially fetched cursor")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(RefCursor_MiscLONG)
//*-----------------------------------------------------------------------
// @class Miscellaneous REF CURSOR cases
//
class RefCursor_MiscLONG : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RefCursor_MiscLONG,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Invoke ref cursor with 2 parameters after resultset clause case1
	int Variation_1();
	// @cmember Invoke ref cursor with 2 parameters after resultset clause case2
	int Variation_2();
	// @cmember Invoke ref cursor with 2 parameters after resultset clause case3
	int Variation_3();
	// @cmember Invoke ref cursor with 2 parameters after resultset clause case4
	int Variation_4();
	// @cmember Invoke ref cursor with 2 literals after resultset clause
	int Variation_5();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(RefCursor_MiscLONG)
#define THE_CLASS RefCursor_MiscLONG
BEG_TEST_CASE(RefCursor_MiscLONG, COraproc, L"Miscellaneous REF CURSOR cases")
	TEST_VARIATION(1, 		L"Invoke ref cursor with 2 parameters after resultset clause case1")
	TEST_VARIATION(2, 		L"Invoke ref cursor with 2 parameters after resultset clause case2")
	TEST_VARIATION(3, 		L"Invoke ref cursor with 2 parameters after resultset clause case3")
	TEST_VARIATION(4, 		L"Invoke ref cursor with 2 parameters after resultset clause case4")
	TEST_VARIATION(5, 		L"Invoke ref cursor with 2 literals after resultset clause")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(RefCursor_Invalid)
//*-----------------------------------------------------------------------
// @class Error cases involving Ref Cursors
//
class RefCursor_Invalid : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RefCursor_Invalid,COraproc);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Use non-zero row count in resultset clause
	int Variation_1();
	// @cmember Extra whitespace
	int Variation_2();
	// @cmember Missing comma
	int Variation_3();
	// @cmember Dangling comma
	int Variation_4();
	// @cmember Misspelled Resultset
	int Variation_5();
	// @cmember Non-number 2nd arg
	int Variation_6();
	// @cmember Only 2 args
	int Variation_7();
	// @cmember Badly qualified call
	int Variation_8();
	// @cmember Unclosed last brace
	int Variation_9();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(RefCursor_Invalid)
#define THE_CLASS RefCursor_Invalid
BEG_TEST_CASE(RefCursor_Invalid, COraproc, L"Error cases involving Ref Cursors")
	TEST_VARIATION(1, 		L"Use non-zero row count in resultset clause")
	TEST_VARIATION(2, 		L"Extra whitespace")
	TEST_VARIATION(3, 		L"Missing comma")
	TEST_VARIATION(4, 		L"Dangling comma")
	TEST_VARIATION(5, 		L"Misspelled Resultset")
	TEST_VARIATION(6, 		L"Non-number 2nd arg")
	TEST_VARIATION(7, 		L"Only 2 args")
	TEST_VARIATION(8, 		L"Badly qualified call")
	TEST_VARIATION(9, 		L"Unclosed last brace")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//*-----------------------------------------------------------------------
// @class Generic ref cursor testing on a rowset
//
class RefCursor : public COraproc { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	~RefCursor (void) {};								
    RefCursor ( wchar_t* pwszTestCaseName) : COraproc(pwszTestCaseName) { };	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// @cmember Strong cursor, OPEN FOR
	int Variation_1();
	// @cmember Strong cursor, Direct Assignment
	int Variation_2();
	// @cmember Weak Cursor, OPEN FOR
	int Variation_3();
	// @cmember Weak Cursor, Direct Assignment
	int Variation_4();
	// @cmember Strong Cursor, OPEN FOR, optional IRowsetFind
	int Variation_5();
	// @cmember Strong cursor, Direct Assignment, optional IRowsetFind
	int Variation_6();
	// @cmember Weak cursor, OPEN FOR, optional IRowsetFind
	int Variation_7();
	// @cmember Weak cursor, Direct Assignment, optional IRowsetFind
	int Variation_8();
	// @cmember Strong cursor, OPEN FOR, various props
	int Variation_9();
	// @cmember Strong cursor, Direct Assignment, various props
	int Variation_10();
	// @cmember Weak cursor, OPEN FOR, various props
	int Variation_11();
	// @cmember Weak cursor, Direct assignment, various props
	int Variation_12();
	// @cmember Verify PROCEDURE COLUMNS schema rowset
	int Variation_13();
	// @cmember Use array parameters
	int Variation_14();
} ;

// {{ TCW_TEST_CASE_MAP(RefCursor_NoBlobs)
//*-----------------------------------------------------------------------
// @class Generic ref cursor testing on a rowset that has no BLOBs
//
class RefCursor_NoBlobs : public RefCursor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RefCursor_NoBlobs,RefCursor);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Strong cursor, OPEN FOR
	//int Variation_1();
	// @cmember Strong cursor, Direct Assignment
	//int Variation_2();
	// @cmember Weak Cursor, OPEN FOR
	//int Variation_3();
	// @cmember Weak Cursor, Direct Assignment
	//int Variation_4();
	// @cmember Strong Cursor, OPEN FOR, optional IRowsetFind
	//int Variation_5();
	// @cmember Strong cursor, Direct Assignment, optional IRowsetFind
	//int Variation_6();
	// @cmember Weak cursor, OPEN FOR, optional IRowsetFind
	//int Variation_7();
	// @cmember Weak cursor, Direct Assignment, optional IRowsetFind
	//int Variation_8();
	// @cmember Strong cursor, OPEN FOR, various props
	//int Variation_9();
	// @cmember Strong cursor, Direct Assignment, various props
	//int Variation_10();
	// @cmember Weak cursor, OPEN FOR, various props
	//int Variation_11();
	// @cmember Weak cursor, Direct assignment, various props
	//int Variation_12();
	// @cmember Verify PROCEDURE COLUMNS schema rowset
	//int Variation 13()
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(RefCursor_NoBlobs)
#define THE_CLASS RefCursor_NoBlobs
BEG_TEST_CASE(RefCursor_NoBlobs, RefCursor, L"Generic ref cursor testing on a rowset that has no BLOBs")
	TEST_VARIATION(1, 		L"Strong cursor, OPEN FOR")
	TEST_VARIATION(2, 		L"Strong cursor, Direct Assignment")
	TEST_VARIATION(3, 		L"Weak Cursor, OPEN FOR")
	TEST_VARIATION(4, 		L"Weak Cursor, Direct Assignment")
	TEST_VARIATION(5, 		L"Strong Cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(6, 		L"Strong cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(7, 		L"Weak cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(8, 		L"Weak cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(9, 		L"Strong cursor, OPEN FOR, various props")
	TEST_VARIATION(10, 		L"Strong cursor, Direct Assignment, various props")
	TEST_VARIATION(11, 		L"Weak cursor, OPEN FOR, various props")
	TEST_VARIATION(12, 		L"Weak cursor, Direct assignment, various props")
	TEST_VARIATION(13, 		L"Verify PROCEDURE COLUMNS schema rowset")
	//TEST_VARIATION(14, 		L"Use array parameters")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(RefCursor_LONGBlobs)
//*-----------------------------------------------------------------------
// @class Generic ref cursor testing on a rowset containg a LONG BLOB
//
class RefCursor_LONGBlobs : public RefCursor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RefCursor_LONGBlobs,RefCursor);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Strong cursor, OPEN FOR
	//int Variation_1();
	// @cmember Strong cursor, Direct Assignment
	//int Variation_2();
	// @cmember Weak Cursor, OPEN FOR
	//int Variation_3();
	// @cmember Weak Cursor, Direct Assignment
	//int Variation_4();
	// @cmember Strong Cursor, OPEN FOR, optional IRowsetFind
	//int Variation_5();
	// @cmember Strong cursor, Direct Assignment, optional IRowsetFind
	//int Variation_6();
	// @cmember Weak cursor, OPEN FOR, optional IRowsetFind
	//int Variation_7();
	// @cmember Weak cursor, Direct Assignment, optional IRowsetFind
	//int Variation_8();
	// @cmember Strong cursor, OPEN FOR, various props
	//int Variation_9();
	// @cmember Strong cursor, Direct Assignment, various props
	//int Variation_10();
	// @cmember Weak cursor, OPEN FOR, various props
	//int Variation_11();
	// @cmember Weak cursor, Direct assignment, various props
	//int Variation_12();
	// @cmember Verify PROCEDURE COLUMNS schema rowset
	//int Variation 13()

	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(RefCursor_LONGBlobs)
#define THE_CLASS RefCursor_LONGBlobs
BEG_TEST_CASE(RefCursor_LONGBlobs, RefCursor, L"Generic ref cursor testing on a rowset that has no BLOBs")
	TEST_VARIATION(1, 		L"Strong cursor, OPEN FOR")
	TEST_VARIATION(2, 		L"Strong cursor, Direct Assignment")
	TEST_VARIATION(3, 		L"Weak Cursor, OPEN FOR")
	TEST_VARIATION(4, 		L"Weak Cursor, Direct Assignment")
	TEST_VARIATION(5, 		L"Strong Cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(6, 		L"Strong cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(7, 		L"Weak cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(8, 		L"Weak cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(9, 		L"Strong cursor, OPEN FOR, various props")
	TEST_VARIATION(10, 		L"Strong cursor, Direct Assignment, various props")
	TEST_VARIATION(11, 		L"Weak cursor, OPEN FOR, various props")
	TEST_VARIATION(12, 		L"Weak cursor, Direct assignment, various props")
	TEST_VARIATION(13, 		L"Verify PROCEDURE COLUMNS schema rowset")
	//TEST_VARIATION(14, 		L"Use array parameters")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(RefCursor_LONGRAWBlobs)
//*-----------------------------------------------------------------------
// @class Generic ref cursor testing on a rowset containg a LONG RAW BLOB
//
class RefCursor_LONGRAWBlobs : public RefCursor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RefCursor_LONGRAWBlobs,RefCursor);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Strong cursor, OPEN FOR
	//int Variation_1();
	// @cmember Strong cursor, Direct Assignment
	//int Variation_2();
	// @cmember Weak Cursor, OPEN FOR
	//int Variation_3();
	// @cmember Weak Cursor, Direct Assignment
	//int Variation_4();
	// @cmember Strong Cursor, OPEN FOR, optional IRowsetFind
	//int Variation_5();
	// @cmember Strong cursor, Direct Assignment, optional IRowsetFind
	//int Variation_6();
	// @cmember Weak cursor, OPEN FOR, optional IRowsetFind
	//int Variation_7();
	// @cmember Weak cursor, Direct Assignment, optional IRowsetFind
	//int Variation_8();
	// @cmember Strong cursor, OPEN FOR, various props
	//int Variation_9();
	// @cmember Strong cursor, Direct Assignment, various props
	//int Variation_10();
	// @cmember Weak cursor, OPEN FOR, various props
	//int Variation_11();
	// @cmember Weak cursor, Direct assignment, various props
	//int Variation_12();
	// @cmember Verify PROCEDURE COLUMNS schema rowset
	//int Variation 13()
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(RefCursor_LONGRAWBlobs)
#define THE_CLASS RefCursor_LONGRAWBlobs
BEG_TEST_CASE(RefCursor_LONGRAWBlobs, RefCursor, L"Generic ref cursor testing on a rowset that has no BLOBs")
	TEST_VARIATION(1, 		L"Strong cursor, OPEN FOR")
	TEST_VARIATION(2, 		L"Strong cursor, Direct Assignment")
	TEST_VARIATION(3, 		L"Weak Cursor, OPEN FOR")
	TEST_VARIATION(4, 		L"Weak Cursor, Direct Assignment")
	TEST_VARIATION(5, 		L"Strong Cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(6, 		L"Strong cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(7, 		L"Weak cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(8, 		L"Weak cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(9, 		L"Strong cursor, OPEN FOR, various props")
	TEST_VARIATION(10, 		L"Strong cursor, Direct Assignment, various props")
	TEST_VARIATION(11, 		L"Weak cursor, OPEN FOR, various props")
	TEST_VARIATION(12, 		L"Weak cursor, Direct assignment, various props")
	TEST_VARIATION(13, 		L"Verify PROCEDURE COLUMNS schema rowset")
	//TEST_VARIATION(14, 		L"Use array parameters")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(RefCursor_ManyColumns)
//*-----------------------------------------------------------------------
// @class Generic ref cursor testing on a rowset containg a LONG BLOB
//
class RefCursor_ManyColumns : public RefCursor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RefCursor_ManyColumns,RefCursor);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Strong cursor, OPEN FOR
	//int Variation_1();
	// @cmember Strong cursor, Direct Assignment
	//int Variation_2();
	// @cmember Weak Cursor, OPEN FOR
	//int Variation_3();
	// @cmember Weak Cursor, Direct Assignment
	//int Variation_4();
	// @cmember Strong Cursor, OPEN FOR, optional IRowsetFind
	//int Variation_5();
	// @cmember Strong cursor, Direct Assignment, optional IRowsetFind
	//int Variation_6();
	// @cmember Weak cursor, OPEN FOR, optional IRowsetFind
	//int Variation_7();
	// @cmember Weak cursor, Direct Assignment, optional IRowsetFind
	//int Variation_8();
	// @cmember Strong cursor, OPEN FOR, various props
	//int Variation_9();
	// @cmember Strong cursor, Direct Assignment, various props
	//int Variation_10();
	// @cmember Weak cursor, OPEN FOR, various props
	//int Variation_11();
	// @cmember Weak cursor, Direct assignment, various props
	//int Variation_12();
	// @cmember Verify PROCEDURE COLUMNS schema rowset
	//int Variation_13();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(RefCursor_ManyColumns)
#define THE_CLASS RefCursor_ManyColumns
BEG_TEST_CASE(RefCursor_ManyColumns, RefCursor, L"Generic ref cursor testing on a rowset that has no BLOBs")
	TEST_VARIATION(1, 		L"Strong cursor, OPEN FOR")
	TEST_VARIATION(2, 		L"Strong cursor, Direct Assignment")
	TEST_VARIATION(3, 		L"Weak Cursor, OPEN FOR")
	TEST_VARIATION(4, 		L"Weak Cursor, Direct Assignment")
	TEST_VARIATION(5, 		L"Strong Cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(6, 		L"Strong cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(7, 		L"Weak cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(8, 		L"Weak cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(9, 		L"Strong cursor, OPEN FOR, various props")
	TEST_VARIATION(10, 		L"Strong cursor, Direct Assignment, various props")
	TEST_VARIATION(11, 		L"Weak cursor, OPEN FOR, various props")
	TEST_VARIATION(12, 		L"Weak cursor, Direct assignment, various props")
	TEST_VARIATION(13, 		L"Verify PROCEDURE COLUMNS schema rowset")
	TEST_VARIATION(14, 		L"Use array parameters")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(RefCursor_Many_50_Columns)
//*-----------------------------------------------------------------------
// @class Generic ref cursor testing on a rowset containg 50 columns
//
class RefCursor_Many_50_Columns : public RefCursor { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RefCursor_Many_50_Columns,RefCursor);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Strong cursor, OPEN FOR
	//int Variation_1();
	// @cmember Strong cursor, Direct Assignment
	//int Variation_2();
	// @cmember Weak Cursor, OPEN FOR
	//int Variation_3();
	// @cmember Weak Cursor, Direct Assignment
	//int Variation_4();
	// @cmember Strong Cursor, OPEN FOR, optional IRowsetFind
	//int Variation_5();
	// @cmember Strong cursor, Direct Assignment, optional IRowsetFind
	//int Variation_6();
	// @cmember Weak cursor, OPEN FOR, optional IRowsetFind
	//int Variation_7();
	// @cmember Weak cursor, Direct Assignment, optional IRowsetFind
	//int Variation_8();
	// @cmember Strong cursor, OPEN FOR, various props
	//int Variation_9();
	// @cmember Strong cursor, Direct Assignment, various props
	//int Variation_10();
	// @cmember Weak cursor, OPEN FOR, various props
	//int Variation_11();
	// @cmember Weak cursor, Direct assignment, various props
	//int Variation_12();
	// @cmember Verify PROCEDURE COLUMNS schema rowset
	//int Variation_13();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(RefCursor_Many_50_Columns)
#define THE_CLASS RefCursor_Many_50_Columns
BEG_TEST_CASE(RefCursor_Many_50_Columns, RefCursor, L"Generic ref cursor testing on a rowset that has 50 columns")
	TEST_VARIATION(1, 		L"Strong cursor, OPEN FOR")
	TEST_VARIATION(2, 		L"Strong cursor, Direct Assignment")
	TEST_VARIATION(3, 		L"Weak Cursor, OPEN FOR")
	TEST_VARIATION(4, 		L"Weak Cursor, Direct Assignment")
	TEST_VARIATION(5, 		L"Strong Cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(6, 		L"Strong cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(7, 		L"Weak cursor, OPEN FOR, optional IRowsetFind")
	TEST_VARIATION(8, 		L"Weak cursor, Direct Assignment, optional IRowsetFind")
	TEST_VARIATION(9, 		L"Strong cursor, OPEN FOR, various props")
	TEST_VARIATION(10, 		L"Strong cursor, Direct Assignment, various props")
	TEST_VARIATION(11, 		L"Weak cursor, OPEN FOR, various props")
	TEST_VARIATION(12, 		L"Weak cursor, Direct assignment, various props")
	TEST_VARIATION(13, 		L"Verify PROCEDURE COLUMNS schema rowset")
	TEST_VARIATION(14, 		L"Use array parameters")
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
	HRESULT				hr = E_FAIL;
	IDBCreateCommand	*pIDBCreateCommand = NULL;
	IDBProperties		*pIDBProperties = NULL;

	g_fKagera = FALSE;

	// Get connection and session objects
	if (ModuleCreateDBSession(pThisTestModule))
	{
		// IDBCreateCommand
		if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, 
								SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
		{
			odtLog << L"Commands are not supported." << ENDL;
			return TEST_SKIPPED;
		}
		else
		{
			ICommandWithParameters * pICmdWPar = NULL;

			hr = pIDBCreateCommand->CreateCommand(NULL, IID_ICommandWithParameters, (IUnknown **)&pICmdWPar);

			SAFE_RELEASE(pIDBCreateCommand);
			SAFE_RELEASE(pICmdWPar);

			if (hr == E_NOINTERFACE)
			{
				odtLog << L"ICommandWithParameters is not supported.\n";
				return TEST_SKIPPED;
			}

			// Fail if we got an error.
			if (!pThisTestModule->m_pError->Validate(hr,	
								LONGSTRING(__FILE__), __LINE__, S_OK))
				return FALSE;
		}

		pThisTestModule->m_pIUnknown->QueryInterface(IID_IDBProperties, (void **)&pIDBProperties);		

		ULONG cPropertyIDSets = 1;
		ULONG cPropertySets = 0;
		DBPROPSET *rgPropertySets=NULL;
		DBPROPIDSET rgPropertyIDSets[1];
		DBPROPID dbPropId[4] =	{
								DBPROP_SQLSUPPORT,
								DBPROP_DBMSNAME,
								DBPROP_PROVIDERFILENAME,
								DBPROP_DBMSVER
								};

		rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;
		rgPropertyIDSets[0].cPropertyIDs = NUMELEM(dbPropId);
		rgPropertyIDSets[0].rgPropertyIDs = dbPropId;

		hr = pIDBProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropertySets, &rgPropertySets);
		SAFE_RELEASE(pIDBProperties);		

		if ( !(DBPROPVAL_SQL_ESCAPECLAUSES & rgPropertySets[0].rgProperties[0].vValue.iVal) )
		{
			odtLog << L"This provider doesn't report support for canonical SQL ESCAPE CLAUSES!" << ENDL;
			FreeProperties(&cPropertySets, &rgPropertySets);
			return TEST_SKIPPED;
		}
		
		if ( 0 != wcscmp(L"Oracle", rgPropertySets->rgProperties[1].vValue.bstrVal ) )
		{
			odtLog << L"This test can only run against the Oracle BackEnd!" << ENDL;
			FreeProperties(&cPropertySets, &rgPropertySets);
			return TEST_SKIPPED;
		}

		if ( 0 == wcscmp(L"MSDASQL.DLL", rgPropertySets->rgProperties[2].vValue.bstrVal ) )
		{
			g_fKagera = TRUE;
		}

		g_ulDBMSVer = _wtol(rgPropertySets->rgProperties[3].vValue.bstrVal);
		odtLog << L"Oracle Backend Version: " << g_ulDBMSVer << ENDL;

		FreeProperties(&cPropertySets, &rgPropertySets);

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
	return ModuleReleaseDBSession(pThisTestModule);
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(17, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TestNulls)
	TEST_CASE(2, TestNullStatus)
	TEST_CASE(3, DefaultParam)
	TEST_CASE(4, ParamLen)
	TEST_CASE(5, Scalar)
	TEST_CASE(6, ArrayParameters)
	TEST_CASE(7, PLSQL_Types)
	TEST_CASE(8, Properties)
	TEST_CASE(9, Misc)
	TEST_CASE(10, RefCursor_Misc)
	TEST_CASE(11, RefCursor_MiscLONG)
	TEST_CASE(12, RefCursor_Invalid)
	TEST_CASE(13, RefCursor_NoBlobs)
	TEST_CASE(14, RefCursor_LONGBlobs)
	TEST_CASE(15, RefCursor_LONGRAWBlobs)
	TEST_CASE(16, RefCursor_ManyColumns)
	TEST_CASE(17, RefCursor_Many_50_Columns)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END

// {{ TCW_TC_PROTOTYPE(TestNulls)
//*-----------------------------------------------------------------------
//| Test Case:		TestNulls - Test unbound accessor parts
//| Created:  	7-15-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TestNulls::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		if (CreateNumericProc1())
			return TRUE;
		else
		{
			odtLog << "Creating tables and stored proc failed!" << ENDL;
			return FALSE;
		}
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Only Length and Status bound
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TestNulls::Variation_1()
{ 
	int		Pass = TEST_FAIL;

	//var2 both pcbValue and rgbValue are null for  output  parameter
	MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_STR, (void *)"2", 2, 5, L"NUMBER", DBTYPE_NUMERIC, 4);
	MakeBinding(2, DBPARAMIO_OUTPUT, DBTYPE_STR, NULL, 38, 0, L"NUMBER", DBTYPE_NUMERIC, 0, DBPART_LENGTH | DBPART_STATUS);
    
	TESTC_(ExecuteProc(L"{Call  oratst_sp1( ? , ? )  }", NO_PREPARE, NOSETPARAMINFO, IID_NULL, NULL, NULL),S_OK);

	Pass = CheckBindingStatus(2, DBSTATUS_S_OK) && CheckBindingLength(2, 1);


CLEANUP:
	
	ClearAllBindings();
	return Pass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Only Length and Status bound for INPUT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TestNulls::Variation_2()
{ 
	int		Pass = TEST_FAIL;
	HRESULT	hr;

	//var2 both pcbValue and rgbValue are null for  output  parameter
	MakeBinding(1, DBPARAMIO_INPUT, DBTYPE_STR, NULL, 2, 5, L"NUMBER", DBTYPE_NUMERIC, 4, DBPART_LENGTH | DBPART_STATUS);
	MakeBinding(2, DBPARAMIO_OUTPUT, DBTYPE_STR, NULL, 38, 0, L"NUMBER", DBTYPE_NUMERIC, 0);
    
	// should fail because input binding for first parameter doesn't have value bound and status is OK
	hr = ExecuteProc(L"{Call  oratst_sp1( ? , ? )  }", NO_PREPARE, NOSETPARAMINFO, IID_NULL, NULL, NULL);

	if (FAILED(hr))
		Pass = TEST_PASS;

	ClearAllBindings();
	return Pass;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TestNulls::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TestNullStatus)
//*-----------------------------------------------------------------------
//| Test Case:		TestNullStatus - Use DBSTATUS_S_ISNULL status
//| Created:  	7-15-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TestNullStatus::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Use DBSTATUS_S_ISNULL for numeric parameter
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TestNullStatus::Variation_1()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use DBSTATUS_S_ISNULL for char parameter
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TestNullStatus::Variation_2()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TestNullStatus::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(DefaultParam)
//*-----------------------------------------------------------------------
//| Test Case:		DefaultParam - Use DBSTATUS_S_DEFAULT
//| Created:  	7-15-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DefaultParam::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		if (CreateCharProc1() && CreateNumericProc2() && CreateDateFunc1())
			return TRUE;
		else
		{
			odtLog << "Creating tables and stored proc failed!" << ENDL;
			return FALSE;
		}
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Character Proc, Prepare, don't use DBSTATUS_S_DEFAULT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_1()
{ 
	return TestCharTypeProc1(PREPARE, NO_DEFAULT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Character proc, Prepare, use DBSTATUS_S_DEFAULT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_2()
{ 
	return TestCharTypeProc1(PREPARE, DEFAULT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Char proc, no prepare, don't use DEFAULT status
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_3()
{ 
	return TestCharTypeProc1(NO_PREPARE, NO_DEFAULT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Char proc, no prepare, use DEFAULT status
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_4()
{ 
	return TestCharTypeProc1(NO_PREPARE, DEFAULT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Numeric, prepare, no default
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_5()
{ 
	return TestNumericTypesProc2(PREPARE, NO_DEFAULT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Numeric, prepare, use default
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_6()
{ 
	return TestNumericTypesProc2(PREPARE, DEFAULT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Numeric, no prepare, no default
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_7()
{ 
	return TestNumericTypesProc2(NO_PREPARE, NO_DEFAULT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Numeric, no prepare, use default
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_8()
{ 
	return TestNumericTypesProc2(NO_PREPARE, DEFAULT);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Date, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_9()
{ 
	return TestDateTypeFunc1(NO_PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Date, prepared, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_10()
{ 
	return TestDateTypeFunc1(PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Date, no prepare, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_11()
{ 
	return TestDateTypeFunc1(NO_PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Date, prepared, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_12()
{ 
	return TestDateTypeFunc1(PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Varnumeric, no default, no getparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_13()
{ 
	return TestNumericTypesProcScenario(NO_DEFAULT, NOGETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Varnumeric, no default, getparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_14()
{ 
	return TestNumericTypesProcScenario(NO_DEFAULT, GETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Varnumeric, default, no getparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_15()
{ 
	return TestNumericTypesProcScenario(DEFAULT, NOGETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Varnumeric, default, getparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_16()
{ 
	return TestNumericTypesProcScenario(DEFAULT, GETPARAMINFO);;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc use DBTYPE_DATE binding, no prepare, NOsetparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultParam::Variation_17()
{ 
	return TestDateTypeFunc2(NO_PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL DefaultParam::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(ParamLen)
//*-----------------------------------------------------------------------
//|	Test Case:		ParamLen - Use param len from PROCEDURE PARAMETERS schema
//| Created:  	7-15-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ParamLen::Init()
{ 
	WCHAR	wszTableAllTypesFormat[] =
		L"CREATE TABLE %s ( i_c  char(255), i_v varchar2(255), i_r raw(255), i_l long, i_d date, i_n number, i_f float, i_i int, o_c char(255), i_rowid rowid ) ";
	WCHAR	wszFuncAllTypesFormat[] = 
		L"CREATE OR REPLACE FUNCTION %s ( i_c  char, i_v varchar2, i_r raw, i_l long, i_lr long raw,  i_d date, i_n number, i_f float, i_i int, o_c OUT char, i_rowid rowid ) "
		L" RETURN NUMBER IS "
		L"  nret NUMBER; "
		L" BEGIN "  
        L" nret := 1;  RETURN(nret); "
		L" END;" ;

	WCHAR	wszTableAllTypes[1000];
	WCHAR	wszFuncAllTypes[1000];
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		WCHAR * pwszTbl = NULL;
		
		pwszTbl = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_TABLELEN);
		if( !pwszTbl )
			return FALSE;

		wcscpy(m_wszTableName, pwszTbl);		
		PROVIDER_FREE(pwszTbl);

		swprintf(wszTableAllTypes, wszTableAllTypesFormat, m_wszTableName);

		wcscpy(m_wszFunctionName, L"fnall_");
		wcsncat(m_wszFunctionName, m_wszTableName, 12);		
		swprintf(wszFuncAllTypes, wszFuncAllTypesFormat, m_wszFunctionName);

		wcscpy(m_wszDropTable, L"drop table ");
		wcscat(m_wszDropTable, m_wszTableName);

		wcscpy(m_wszDropFunc, L"drop function ");
		wcscat(m_wszDropFunc, m_wszFunctionName);

		TESTC_(Execute(wszTableAllTypes), S_OK);
		TESTC_(Execute(wszFuncAllTypes), S_OK);

		return TRUE;
	} 

CLEANUP:
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Verify PROCEDURE_PARAMETERS schema
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ParamLen::Variation_1()
{ 
	HRESULT				hr;
	IDBSchemaRowset	*	pIDBSchemaRowset = NULL;
	IAccessor *			pIAccessor = NULL;
	IRowset *			pIRowset = NULL;

	typedef struct tagProcCols 
	{
		 char		*szName;
		 LONG		nType ;   // 1 in ,4 out ,5  return
		 LONG		cbColDef;  //precision
		 LONG		nLength;   //length
		 LONG		nOrdinalPos; //ordinal position
	} PROCCOLS;

	
    PROCCOLS arr_proc_colsOra7[] = {
								"RETURN_VALUE", DBPARAMTYPE_RETURNVALUE, 38,40,1,
								"I_C", DBPARAMTYPE_INPUT, 255,255,2,
								"I_V", DBPARAMTYPE_INPUT, 2000, 2000,3,
								"I_R", DBPARAMTYPE_INPUT, 255, 255,4,
								"I_L", DBPARAMTYPE_INPUT ,  2147483647, 2147483647,5,
								"I_LR", DBPARAMTYPE_INPUT, 2147483647, 2147483647,6,
								"I_D", DBPARAMTYPE_INPUT, 19, 16, 7,
								"I_N", DBPARAMTYPE_INPUT, 38,40, 8,
								"I_F", DBPARAMTYPE_INPUT, 38, 40, 9,  // float is type double so precision and scale are 38,40
								"I_I", DBPARAMTYPE_INPUT, 15,8, 10,  //int is of float type since it does not have scale
								"O_C", DBPARAMTYPE_OUTPUT, 255,255, 11,
								"I_ROWID", DBPARAMTYPE_INPUT, 18, 18, 12
									};
	
	PROCCOLS arr_proc_colsOra8[] = 	{
								"RETURN_VALUE", DBPARAMTYPE_RETURNVALUE, 38,40,1,
		                        "I_C", DBPARAMTYPE_INPUT, 2000,2000,2,
								"I_V", DBPARAMTYPE_INPUT, 4000, 4000,3,
								"I_R", DBPARAMTYPE_INPUT, 2000, 2000,4,
								"I_L", DBPARAMTYPE_INPUT ,  2147483647, 2147483647,5,
								"I_LR", DBPARAMTYPE_INPUT, 2147483647, 2147483647,6,
								"I_D", DBPARAMTYPE_INPUT, 19, 16, 7,
								"I_N", DBPARAMTYPE_INPUT, 38,40, 8,
								"I_F", DBPARAMTYPE_INPUT, 38, 40, 9,  // float is type double so precision and scale are 38,40
								"I_I", DBPARAMTYPE_INPUT, 15,8, 10,  //int is of float type since it does not have scale
								"O_C", DBPARAMTYPE_OUTPUT, 2000,2000, 11,
								"I_ROWID", DBPARAMTYPE_INPUT, 18, 18, 12
									};
	PROCCOLS *	arr_proc_cols = ( g_ulDBMSVer > 7 ? arr_proc_colsOra8 : arr_proc_colsOra7 );


	VARIANT	rgRestrict[3];

	DBBINDING			rgBinding[5] =
	{
		//parameter name
		{4, 0, 0, 0, NULL, NULL, NULL, DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 50, 0, DBTYPE_STR, 0, 0}, 
		// ordinal pos.
		{5, 56, 0, 52, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_I4, 0, 0}, 
		// param type (input/output)
		{6, 60, 0, 0, NULL, NULL, NULL, DBPART_VALUE, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_I4, 0, 0}, 
		// prec
		{11, 68, 0, 64, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_I4, 0, 0},
		// param size
		{13, 76, 0, 72, NULL, NULL, NULL, DBPART_VALUE | DBPART_STATUS, DBMEMOWNER_CLIENTOWNED, DBPARAMIO_NOTPARAM, 0, 0, DBTYPE_I4, 0, 0} 
	};
	      
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBBINDSTATUS	rgBindStatus[5];
	DBCOUNTITEM		cRowsObtained = 0, cCurRow =0;
	DB_LORDINAL		lParamOrdinal = 0;
	HROW			hrow = DB_NULL_HROW;
	HROW *			pHRow = &hrow;
	BYTE			pData[100];
  
	VariantInit(&(rgRestrict[0]));
	VariantInit(&(rgRestrict[1]));
	VariantInit(&(rgRestrict[2]));

	V_VT(&(rgRestrict[2])) = VT_BSTR;
	V_BSTR(&(rgRestrict[2])) = SysAllocString(m_wszFunctionName);

	GetSessionObject(IID_IDBSchemaRowset, (IUnknown **)&pIDBSchemaRowset);
	TESTC_(hr=pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_PROCEDURE_PARAMETERS, 3, rgRestrict, IID_IRowset, 0, NULL, (IUnknown **) &pIRowset),S_OK);

	VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor);	

	TESTC_(pIAccessor->CreateAccessor(
						DBACCESSOR_ROWDATA,
						NUMELEM(rgBinding),
						rgBinding,
						NULL,
						&hAccessor,
						rgBindStatus
						), S_OK);

	while ( S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &pHRow)) )
	{
		TESTC(COMPARE(cRowsObtained, 1));
		TESTC_(pIRowset->GetData(pHRow[0], hAccessor, (void *)pData), S_OK);
		
		// Is the ordinal position NULL?
		// then assume parameters are returned sequentially.
		if (DBSTATUS_S_ISNULL == *(DBSTATUS *)(pData+rgBinding[1].obStatus))
			lParamOrdinal = cCurRow;
		else
			lParamOrdinal = *(LONG *)(pData+rgBinding[1].obValue);
		
		COMPARE(0, strcmp(arr_proc_cols[lParamOrdinal].szName, (char *)(pData+rgBinding[0].obValue)));
		COMPARE(arr_proc_cols[lParamOrdinal].nType, *(LONG *)(pData+rgBinding[2].obValue));

		if (DBSTATUS_S_ISNULL != *(DBSTATUS *)(pData+rgBinding[3].obStatus))
			COMPARE(arr_proc_cols[lParamOrdinal].nLength, *(LONG *)(pData+rgBinding[3].obValue));
		/* see bug #28970
		if (DBSTATUS_S_ISNULL != *(DBSTATUS *)(pData+rgBinding[4].obStatus))
			COMPARE(arr_proc_cols[lParamOrdinal].cbColDef, *(LONG *)(pData+rgBinding[4].obValue));
		*/
		 		
		cCurRow++;
		TESTC_(pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL), S_OK);			
	}			
	TESTC(cCurRow == NUMELEM(arr_proc_colsOra8));

CLEANUP:
	

	VariantClear(&(rgRestrict[2]));

	if (pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor,NULL);

	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBSchemaRowset);

	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc Verify COLUMNS schema
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ParamLen::Variation_2()
{
	return VerifyColumnsSchema(m_wszTableName, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc Verify COLUMNS schema with SYNONYM
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ParamLen::Variation_3()
{
	BOOL	fPass;
	WCHAR *	pwszUser = NULL;
	WCHAR	wszSynonym[MAXBUFLEN];
	WCHAR	wszCreateSynonym[MAXBUFLEN];
	WCHAR	wszDropSynonym[MAXBUFLEN];

	ASSERT(m_pIDBInfo && m_wszTableName);

	TESTC(GetProperty(DBPROP_AUTH_USERID, DBPROPSET_DBINIT,
				m_pIDBInfo, &pwszUser));

	wcscpy(wszSynonym, L"syn_");
	wcsncat(wszSynonym, m_wszTableName, 15);		
	
	wcscpy(wszCreateSynonym, L"create public synonym ");
	wcscat(wszCreateSynonym, wszSynonym);
	wcscat(wszCreateSynonym, L" for ");
	if( pwszUser )
		wcscat(wszCreateSynonym, pwszUser);
	else
		wcscat(wszCreateSynonym, L"scott");
	wcscat(wszCreateSynonym, L".");
	wcscat(wszCreateSynonym, m_wszTableName);

	wcscpy(wszDropSynonym, L"drop public synonym ");
	wcscat(wszDropSynonym, wszSynonym);

	TESTC_(Execute(wszCreateSynonym), S_OK);

	fPass = VerifyColumnsSchema(wszSynonym, TRUE);

CLEANUP:
	Execute(wszDropSynonym);

	PROVIDER_FREE(pwszUser);

	return fPass;
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL ParamLen::Terminate()
{ 
	Execute(m_wszDropTable);
	Execute(m_wszDropFunc);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(Scalar)
//*-----------------------------------------------------------------------
//| Test Case:		Scalar - Use scalar values
//| Created:  	7-15-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scalar::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc LONGRAW, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Scalar::Variation_1()
{ 
	return	TestLongRawType1(NO_PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc LONGRAW, prepared, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Scalar::Variation_2()
{ 
	return	TestLongRawType1(PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LONGRAW, no prepare, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Scalar::Variation_3()
{ 
	return	TestLongRawType1(NO_PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc LONGRAW,  prepared, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Scalar::Variation_4()
{ 
	return	TestLongRawType1(PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Scalar::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(ArrayParameters)
//*-----------------------------------------------------------------------
//| Test Case:		ArrayParameters - Use PLSQL Parameter arrays
//| Created:  	7-15-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ArrayParameters::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Fetch NUMBER array, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_1()
{ 
	return TestNumberArray(NO_PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Fetch NUMBER array, prepared, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_4()
{ 
	return TestNumberArray(PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Fetch NUMBER array, no prepare, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_5()
{ 
	return TestNumberArray(NO_PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Fetch NUMBER array, prepared, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_6()
{ 
	return TestNumberArray(PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Fetch Large RS, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_7()
{ 
	return TestLargeResultsArray(NO_PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Fetch Large RS, prepared, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_8()
{ 
	return TestLargeResultsArray(PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Fetch Large RS, no prepare, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_9()
{ 
	return TestLargeResultsArray(NO_PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Fetch Large RS, prepared, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_10()
{ 
	return TestLargeResultsArray(PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Basic Mult Res, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_11()
{ 	
	return TestMultResArray(NO_PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Basic Mult Res, prepared, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_12()
{ 
	return TestMultResArray(PREPARE, NOSETPARAMINFO);;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Basic Mult Res, no prepare, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_13()
{ 
	return TestMultResArray(NO_PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Basic Mult Res,  prepared, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_14()
{ 
	return TestMultResArray(PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Fetch DATE array, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_15()
{ 
	return TestDateArray(NO_PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Fetch DATE array, prepared, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_16()
{ 
	return TestDateArray(PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Fetch DATE array, no prepare, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_17()
{ 
	return TestDateArray(NO_PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Fetch DATE array, prepared, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_18()
{ 
	return TestDateArray(PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Fetch RAW array, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_19()
{ 
	return TestRawArray(NO_PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Fetch RAW array, prepared, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_20()
{ 
	return TestRawArray(PREPARE, NOSETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Fetch RAW array, no prepare, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_21()
{ 
	return TestRawArray(NO_PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Fetch RAW array, prepared, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_22()
{ 
	return TestRawArray(PREPARE, SETPARAMINFO);
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Fetch VARNUM array, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_23()
{ 
	return TestVarNumArray(NO_PREPARE, NOSETPARAMINFO, NEGSCALE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Fetch VARNUM array, prepared, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_24()
{ 
	return TestVarNumArray(PREPARE, NOSETPARAMINFO, NEGSCALE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Fetch VARNUM array, no prepare, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_25()
{ 
	return TestVarNumArray(NO_PREPARE, SETPARAMINFO, NEGSCALE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Fetch VARNUM array, prepared, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_26()
{ 
	return TestVarNumArray(PREPARE, SETPARAMINFO, NEGSCALE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Fetch VARNUM array, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_27()
{ 
	return TestVarNumArray(NO_PREPARE, NOSETPARAMINFO, NOSCALE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Fetch VARNUM array, prepared, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_28()
{ 
	return TestVarNumArray(PREPARE, NOSETPARAMINFO, NOSCALE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Fetch VARNUM array, no prepare, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_29()
{ 
	return TestVarNumArray(NO_PREPARE, SETPARAMINFO, NOSCALE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Fetch VARNUM array, prepared, setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ArrayParameters::Variation_30()
{ 
	return TestVarNumArray(PREPARE, SETPARAMINFO, NOSCALE);
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL ArrayParameters::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(PLSQL_Types)
//*-----------------------------------------------------------------------
//| Test Case:		PLSQL_Types - Test all PL/SQL Types
//| Created:  	7-23-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL PLSQL_Types::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Char Output parameter, 2k size, no prepare, no setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int PLSQL_Types::Variation_1()
{ 
	const int maxCHARsize = 2000;
	HRESULT	hr;
	WCHAR	wszExpected[maxCHARsize + 1];
	int	i;


	TESTC_(hr=Execute(	L"create or replace procedure sp_bigchar( par_out_bigchar OUT CHAR)  as"
						L"		tempC VARCHAR2(2000); "
	                    L" begin   "
						L"	tempC := 'a';"
						L"	par_out_bigchar :=  RPAD(tempC, 2000, '1' ); "
						L"end;" ), S_OK);

	MakeBinding(1, DBPARAMIO_OUTPUT, DBTYPE_WSTR, NULL, 0, 0, L"CHAR", DBTYPE_STR, maxCHARsize);

	TESTC_(ExecuteProc(L"{call sp_bigchar(?)}", NO_PREPARE, NOSETPARAMINFO, IID_NULL, NULL, NULL),S_OK);

	wszExpected[0] = L'a';
	for (i=1; i<maxCHARsize; i++)
		wszExpected[i] = L'1';
	wszExpected[maxCHARsize] = L'\0';

	COMPARE(CheckBindingValue(1, wszExpected), TRUE);

CLEANUP:

	ClearAllBindings();
	return (hr == S_OK);
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL PLSQL_Types::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(Properties)
//*-----------------------------------------------------------------------
//| Test Case:		Properties - Properties
//| Created:  	7-23-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Properties::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Key Column Property
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Properties::Variation_1()
{ 
	WCHAR *		pwszCreateTable = NULL;
	WCHAR *		pwszDropTable = NULL;
	WCHAR *		pwszQuery = NULL;
	WCHAR *		pwszTableName = NULL;
	WCHAR *		pwszColumnNameNum = NULL;
	WCHAR *		pwszColumnNameRowId = NULL;
	WCHAR *		pStringBuf = NULL;
	BYTE *		pData = NULL;
	HROW *		rghRows = NULL;
	DBORDINAL	cCols = 0;
	DBCOUNTITEM	cRowsObtained = 0;

	DBPROPSET	rgPropSet[2];
	DBPROP		PropUniqueRows = 
					{
					DBPROP_UNIQUEROWS, 
					DBPROPOPTIONS_REQUIRED, 
					DBPROPSTATUS_OK, 
					{DB_NULLGUID, 0, (LPOLESTR)0},
					{VT_BOOL, 0, 0, 0, VARIANT_TRUE}
					};

	DBPROP		PropOraKeyColumn = 
					{
					DBPROP_MSDAORA_DETERMINEKEYCOLUMNS,
					DBPROPOPTIONS_REQUIRED,
					DBPROPSTATUS_OK,
					{DB_NULLGUID, 0, (LPOLESTR)0}, 
					{VT_BOOL, 0, 0, 0, VARIANT_TRUE}
					};

	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBROWSTATUS		ulRowStatus;
	DBLENGTH		cRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING *		rgBindings = NULL;
	DBCOLUMNINFO *	rgColInfo = NULL;
	IRowset *		pColRowset = NULL;
	IColumnsInfo *	pIColumnsInfo = NULL;
	IColumnsRowset * pIColumnsRowset = NULL;
	ICommandProperties * pICmdProp = NULL;

	static WCHAR s_wszCreateTable[] = L"create table %s (%s number(38,-84), %s rowid, primary key(%s,%s))";

	// This var tests an MSDAORA specific property
	if( g_fKagera )
		return TEST_SKIPPED;

	pwszTableName = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_TABLELEN);
	pwszColumnNameNum = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_COLLEN);
	pwszColumnNameRowId = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_COLLEN);

	TESTC(pwszTableName && pwszColumnNameNum && pwszColumnNameRowId);

	pwszCreateTable = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(pwszTableName)+2*wcslen(pwszColumnNameNum)+2*wcslen(pwszColumnNameRowId)+wcslen(s_wszCreateTable)+1));
	pwszDropTable = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(pwszTableName)+wcslen(wszDROP_TABLE)+1));
	pwszQuery = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(pwszTableName)+wcslen(pwszColumnNameNum)+wcslen(wszSELECT_ALLFROMTBL)+1));

	TESTC(pwszCreateTable && pwszDropTable && pwszQuery);

	swprintf(pwszCreateTable, s_wszCreateTable, pwszTableName, pwszColumnNameNum, pwszColumnNameRowId, pwszColumnNameNum, pwszColumnNameRowId);
	swprintf(pwszDropTable, wszDROP_TABLE, pwszTableName);
	swprintf(pwszQuery, L"select %s from %s", pwszColumnNameNum, pwszTableName);

	TESTC_(Execute(pwszCreateTable), S_OK);

	TESTC(VerifyInterface(m_pICommandText, IID_ICommandProperties, ROWSET_INTERFACE, (IUnknown**)&pICmdProp));

	rgPropSet[0].cProperties = 1;
	rgPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	rgPropSet[0].rgProperties = &PropUniqueRows;
	rgPropSet[1].cProperties = 1;
	rgPropSet[1].guidPropertySet = DBPROPSET_MSDAORA_ROWSET;
	rgPropSet[1].rgProperties = &PropOraKeyColumn;

	TESTC_(pICmdProp->SetProperties(2, rgPropSet), S_OK);

	TESTC_(ExecuteProc(pwszQuery, NO_PREPARE, NOSETPARAMINFO, IID_IColumnsInfo, NULL, (IUnknown **)&pIColumnsInfo), S_OK);

	TESTC(DefaultObjectTesting(pIColumnsInfo, ROWSET_INTERFACE));

	// verify column metadata
	TESTC_(pIColumnsInfo->GetColumnInfo(&cCols, &rgColInfo, &pStringBuf), S_OK);
	TESTC(cCols == 1 && rgColInfo && pStringBuf);
	TESTC(rgColInfo[0].wType == DBTYPE_VARNUMERIC);
	TESTC(rgColInfo[0].iOrdinal == 1);
	TESTC(rgColInfo[0].bPrecision == 38);
	TESTC(rgColInfo[0].bScale == 84);
	TESTC(rgColInfo[0].ulColumnSize == 20);
	TESTC(rgColInfo[0].dwFlags == (DBCOLUMNFLAGS_WRITEUNKNOWN | DBCOLUMNFLAGS_SCALEISNEGATIVE))
	TESTC(0 == _wcsicmp(rgColInfo[0].pwszName, pwszColumnNameNum));
	
	TESTC(VerifyInterface(pIColumnsInfo, IID_IColumnsRowset, ROWSET_INTERFACE, (IUnknown**)&pIColumnsRowset));
	TESTC_(pIColumnsRowset->GetColumnsRowset(NULL, 1, &DBCOLUMN_KEYCOLUMN, IID_IRowset, 0, NULL, (IUnknown **)&pColRowset), S_OK);

	TESTC_(GetAccessorAndBindings(pColRowset, DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cRowSize, DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY,
		0, NULL, NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM), S_OK);

	TESTC((pData = (BYTE *)PROVIDER_ALLOC(cRowSize)) != NULL);

	//GetNextRow for the column in the select list
	TESTC_(pColRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
		
	//VerifyRowHandle
	TESTC(cRowsObtained == 1);
	TESTC_(pColRowset->GetData(rghRows[0], hAccessor, pData),S_OK);		
	
	// Test the column metadata
	TESTC(STATUS_BINDING(rgBindings[0], pData) == DBSTATUS_S_OK);
	TESTC(0 == _wcsicmp((WCHAR*)&VALUE_BINDING(rgBindings[0], pData), pwszColumnNameNum));
	TESTC(STATUS_BINDING(rgBindings[4], pData) == DBSTATUS_S_OK);
	TESTC(*(ULONG *)&VALUE_BINDING(rgBindings[4], pData) == 1);
	TESTC(STATUS_BINDING(rgBindings[cBindings-1], pData) == DBSTATUS_S_OK);
	TESTC(*(VARIANT_BOOL *)&VALUE_BINDING(rgBindings[cBindings-1], pData) == VARIANT_TRUE);

	//GetNextRow for the hidden column
	CHECK(pColRowset->ReleaseRows(1, rghRows, NULL, NULL, &ulRowStatus), S_OK);
	SAFE_FREE(rghRows);
	TESTC_(pColRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
		
	//VerifyRowHandle
	TESTC(cRowsObtained == 1);
	TESTC_(pColRowset->GetData(rghRows[0], hAccessor, pData),S_OK);		
	
	// Test the column metadata
	TESTC(STATUS_BINDING(rgBindings[0], pData) == DBSTATUS_S_OK);
	TESTC(0 == _wcsicmp((WCHAR*)&VALUE_BINDING(rgBindings[0], pData), pwszColumnNameRowId));
	TESTC(STATUS_BINDING(rgBindings[4], pData) == DBSTATUS_S_OK);
	TESTC(*(ULONG *)&VALUE_BINDING(rgBindings[4], pData) == 2);
	TESTC(STATUS_BINDING(rgBindings[cBindings-1], pData) == DBSTATUS_S_OK);
	TESTC(*(VARIANT_BOOL *)&VALUE_BINDING(rgBindings[cBindings-1], pData) == VARIANT_TRUE);


	//release the row handle
	CHECK(pColRowset->ReleaseRows(1, rghRows, NULL, NULL, &ulRowStatus), S_OK);
	TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	SAFE_FREE(rghRows);

	// There should only be one row in the columns rowset
	TESTC_(pColRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);


CLEANUP:

	SAFE_FREE(rghRows);
	SAFE_FREE(pData);
	SAFE_FREE(rgBindings);
	SAFE_FREE(pwszQuery);
	SAFE_FREE(pwszCreateTable);
	SAFE_FREE(pwszDropTable);
	SAFE_FREE(pwszTableName);
	SAFE_FREE(pwszColumnNameNum);
	SAFE_FREE(pwszColumnNameRowId);
	SAFE_FREE(rgColInfo);
	SAFE_FREE(pStringBuf);

	SAFE_RELEASE(pICmdProp);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pColRowset);

	if( pwszDropTable )
		Execute(pwszDropTable);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Key Column Property with UNIQUE index
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Properties::Variation_2()
{ 
	WCHAR *		pwszCreateTable = NULL;
	WCHAR *		pwszCreateIndex = NULL;
	WCHAR *		pwszDropTable = NULL;
	WCHAR *		pwszQuery = NULL;
	WCHAR *		pwszTableName = NULL;
	WCHAR *		pStringBuf = NULL;
	BYTE *		pData = NULL;
	HROW *		rghRows = NULL;
	DBORDINAL	cCols = 0;
	DBCOUNTITEM	cRowsObtained = 0;
	ULONG		cPropSets = 0;
	DBPROPSET *	rgPropSet = NULL;
	
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBROWSTATUS		ulRowStatus;
	DBLENGTH		cRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING *		rgBindings = NULL;
	DBCOLUMNINFO *	rgColInfo = NULL;
	IRowset *		pColRowset = NULL;
	IColumnsInfo *	pIColumnsInfo = NULL;
	IColumnsRowset * pIColumnsRowset = NULL;
	ICommandProperties * pICmdProp = NULL;

	static WCHAR s_wszCreateTable[] = L"create table %s (c1 number, c2 date, c3 long)";
	static WCHAR s_wszCreateIndex[] = L"create unique index %s on %s(c2, c1)";

	// This var tests an MSDAORA specific property
	if( g_fKagera )
		return TEST_SKIPPED;

	pwszTableName = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_TABLELEN);

	TESTC(pwszTableName != NULL);

	pwszCreateTable = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(pwszTableName)+wcslen(s_wszCreateTable)+1));
	pwszDropTable = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(pwszTableName)+wcslen(wszDROP_TABLE)+1));
	pwszQuery = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(wcslen(pwszTableName)+wcslen(wszSELECT_ALLFROMTBL)+1));
	pwszCreateIndex = (WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR)*(2*wcslen(pwszTableName)+wcslen(s_wszCreateIndex)+1));

	TESTC(pwszCreateTable && pwszDropTable && pwszQuery && pwszCreateIndex);

	swprintf(pwszCreateTable, s_wszCreateTable, pwszTableName);
	swprintf(pwszCreateIndex, s_wszCreateIndex, pwszTableName, pwszTableName);
	swprintf(pwszDropTable, wszDROP_TABLE, pwszTableName);
	swprintf(pwszQuery, L"select c1 from %s", pwszTableName);

	TESTC_(Execute(pwszCreateTable), S_OK);
	TESTC_(Execute(pwszCreateIndex), S_OK);

	TESTC(VerifyInterface(m_pICommandText, IID_ICommandProperties, ROWSET_INTERFACE, (IUnknown**)&pICmdProp));

	::SetProperty(DBPROP_UNIQUEROWS, DBPROPSET_ROWSET, &cPropSets,
		&rgPropSet, DBTYPE_BOOL, (ULONG_PTR)VARIANT_TRUE, DBPROPOPTIONS_REQUIRED);

	::SetProperty(DBPROP_MSDAORA_DETERMINEKEYCOLUMNS, DBPROPSET_MSDAORA_ROWSET, &cPropSets,
		&rgPropSet, DBTYPE_BOOL, (ULONG_PTR)VARIANT_TRUE, DBPROPOPTIONS_REQUIRED);

	TESTC_(pICmdProp->SetProperties(cPropSets, rgPropSet), S_OK);

	TESTC_(ExecuteProc(pwszQuery, NO_PREPARE, NOSETPARAMINFO, IID_IColumnsInfo, NULL, (IUnknown **)&pIColumnsInfo), S_OK);

	TESTC(DefaultObjectTesting(pIColumnsInfo, ROWSET_INTERFACE));

	// verify column metadata
	TESTC_(pIColumnsInfo->GetColumnInfo(&cCols, &rgColInfo, &pStringBuf), S_OK);
	TESTC(cCols == 1 && rgColInfo && pStringBuf);
	TESTC(rgColInfo[0].wType == DBTYPE_VARNUMERIC);
	
	TESTC(VerifyInterface(pIColumnsInfo, IID_IColumnsRowset, ROWSET_INTERFACE, (IUnknown**)&pIColumnsRowset));
	TESTC_(pIColumnsRowset->GetColumnsRowset(NULL, 1, &DBCOLUMN_KEYCOLUMN, IID_IRowset, 0, NULL, (IUnknown **)&pColRowset), S_OK);

	TESTC_(GetAccessorAndBindings(pColRowset, DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cRowSize, DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY,
		0, NULL, NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM), S_OK);

	TESTC((pData = (BYTE *)PROVIDER_ALLOC(cRowSize)) != NULL);

	//GetNextRow for the column in the select list
	TESTC_(pColRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
		
	//VerifyRowHandle
	TESTC(cRowsObtained == 1);
	TESTC_(pColRowset->GetData(rghRows[0], hAccessor, pData),S_OK);		
	
	// Test the column metadata
	TESTC(STATUS_BINDING(rgBindings[0], pData) == DBSTATUS_S_OK);
	TESTC(0 == _wcsicmp((WCHAR*)&VALUE_BINDING(rgBindings[0], pData), L"c1"));
	TESTC(STATUS_BINDING(rgBindings[4], pData) == DBSTATUS_S_OK);
	TESTC(*(ULONG *)&VALUE_BINDING(rgBindings[4], pData) == 1);
	TESTC(STATUS_BINDING(rgBindings[cBindings-1], pData) == DBSTATUS_S_OK);
	TESTC(*(VARIANT_BOOL *)&VALUE_BINDING(rgBindings[cBindings-1], pData) == VARIANT_TRUE);

	//GetNextRow for the hidden column
	CHECK(pColRowset->ReleaseRows(1, rghRows, NULL, NULL, &ulRowStatus), S_OK);
	SAFE_FREE(rghRows);
	TESTC_(pColRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
		
	//VerifyRowHandle
	TESTC(cRowsObtained == 1);
	TESTC_(pColRowset->GetData(rghRows[0], hAccessor, pData),S_OK);		
	
	// Test the column metadata
	TESTC(STATUS_BINDING(rgBindings[0], pData) == DBSTATUS_S_OK);
	TESTC(0 == _wcsicmp((WCHAR*)&VALUE_BINDING(rgBindings[0], pData), L"c2"));
	TESTC(STATUS_BINDING(rgBindings[4], pData) == DBSTATUS_S_OK);
	TESTC(*(ULONG *)&VALUE_BINDING(rgBindings[4], pData) == 2);
	TESTC(STATUS_BINDING(rgBindings[cBindings-1], pData) == DBSTATUS_S_OK);
	TESTC(*(VARIANT_BOOL *)&VALUE_BINDING(rgBindings[cBindings-1], pData) == VARIANT_TRUE);

	//release the row handle
	CHECK(pColRowset->ReleaseRows(1, rghRows, NULL, NULL, &ulRowStatus), S_OK);
	TESTC(ulRowStatus == DBROWSTATUS_S_OK);
	SAFE_FREE(rghRows);

	// There should only be one row in the columns rowset
	TESTC_(pColRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);


CLEANUP:

	::FreeProperties(&cPropSets, &rgPropSet);

	SAFE_FREE(rghRows);
	SAFE_FREE(pData);
	SAFE_FREE(rgBindings);
	SAFE_FREE(pwszQuery);
	SAFE_FREE(pwszTableName);
	SAFE_FREE(pwszCreateIndex);
	SAFE_FREE(pwszCreateTable);
	SAFE_FREE(pwszDropTable);
	SAFE_FREE(rgColInfo);
	SAFE_FREE(pStringBuf);
	SAFE_RELEASE(pICmdProp);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pColRowset);

	if( pwszDropTable )
		Execute(pwszDropTable);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Properties::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(Misc)
//*-----------------------------------------------------------------------
//| Test Case:	Misc - Misc
//| Created:  	7-23-98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Misc::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Long parameter info in proc
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Misc::Variation_1()
{ 
	WCHAR			wszFuncCreate[] = 
		L"CREATE OR REPLACE PROCEDURE %s (o_l OUT long, o_lr OUT long raw) as"
		L" BEGIN "  
        L"  o_l := 'a'; "
		L" END;" ;

	WCHAR			wszFuncCall[] = L"{call %s(?, ?)}";

	WCHAR *			pwszFunc = NULL;
	WCHAR			wszCreateFunc[MAXBUFLEN];
	WCHAR			wszCallFunc[MAXBUFLEN];
	WCHAR			wszDropFunc[MAXBUFLEN];
	DB_UPARAMS		cParams;
	DBPARAMINFO *	rgParamInfo = NULL;
	WCHAR *			pwszNames = NULL;
	
	pwszFunc = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_TABLELEN);
	if( !pwszFunc )
		return FALSE;

	swprintf(wszCreateFunc, wszFuncCreate, pwszFunc);
	swprintf(wszCallFunc, wszFuncCall, pwszFunc);

	wcscpy(wszDropFunc, L"drop procedure ");
	wcscat(wszDropFunc, pwszFunc);

	TESTC_(Execute(wszCreateFunc), S_OK);


	// Prepare the command and the parameter information
	TESTC(SUCCEEDED(m_pICommandText->SetCommandText(DBGUID_DEFAULT, wszCallFunc)));
		
	if( m_pICommandPrepare )
	{
		TESTC(SUCCEEDED(m_pICommandPrepare->Prepare(0)));
		if( m_pICmdWParams )
		{
			TESTC_(m_pICmdWParams->GetParameterInfo(&cParams, &rgParamInfo, &pwszNames),S_OK);

			TESTC(cParams == 2);

			COMPARE(rgParamInfo[0].iOrdinal, 1);
			COMPARE(rgParamInfo[1].iOrdinal, 2);

			COMPARE(rgParamInfo[0].pTypeInfo, NULL);
			COMPARE(rgParamInfo[1].pTypeInfo, NULL);

			COMPARE(rgParamInfo[0].ulParamSize, 32760 );
			COMPARE(rgParamInfo[1].ulParamSize, 32760 );

			COMPARE(rgParamInfo[0].wType, DBTYPE_STR);
			COMPARE(rgParamInfo[1].wType, DBTYPE_BYTES);
		}
	}

CLEANUP:

	PROVIDER_FREE(pwszNames);
	PROVIDER_FREE(rgParamInfo);

	Execute(wszDropFunc, FALSE);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Long error message
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Misc::Variation_2()
{ 
	WCHAR	wszErrProc[] =
						L"CREATE OR REPLACE PROCEDURE %s AS "
						L"output_err_msg VARCHAR2(1000); "
						L"raised_error_string VARCHAR2(2); "
						L"BEGIN "
							L"raised_error_string := 'This assignement will raise an exeption'; "
							L"EXCEPTION "
							L"WHEN OTHERS THEN "
							 L"output_err_msg := '%s' || SQLERRM; "
							 L"raise_application_error(-20250, output_err_msg);"
						L"END;";

	WCHAR	wszErrorBody[] =
							L"Exception thrown by user:- This exception message "
							L"contains more than 255 characters Exception thrown by user:- This exception "
							L"message contains more than 255 characters Exception thrown by user:- This "
							L"exception message contains more than 255 characters Exception thrown by "
							L"user:- This exception message contains more than 255 characters "
							L"Exception thrown by user:- This exception message contains more than 255 "
							L"characters Exception thrown by user:- This exception message contains more "
							L"than 255 characters";

	WCHAR	wszCreateErrProc[sizeof(wszErrProc)+sizeof(wszErrorBody)+MAXBUFLEN];

	WCHAR	wszProcCall[] = L"{call %s}";
	WCHAR *	pwszProc = NULL;
	WCHAR	wszCallErrProc[MAXBUFLEN];
	WCHAR	wszDropProc[MAXBUFLEN];
	HRESULT	hr;
	BSTR	bstrDescription = NULL;
	IErrorInfo * pIErrorInfo = NULL;
	
	pwszProc = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_TABLELEN);
	if( !pwszProc )
		return FALSE;

	swprintf(wszCreateErrProc, wszErrProc, pwszProc, wszErrorBody);
	swprintf(wszCallErrProc, wszProcCall, pwszProc);

	wcscpy(wszDropProc, L"drop procedure ");
	wcscat(wszDropProc, pwszProc);

	TESTC_(Execute(wszCreateErrProc), S_OK);


	hr = ExecuteProc(wszCallErrProc, NO_PREPARE, NOSETPARAMINFO, IID_IUnknown, NULL, NULL);
	TESTC_(hr, E_FAIL);

	TESTC_(GetErrorInfo(0, &pIErrorInfo), S_OK);
	TESTC_(pIErrorInfo->GetDescription(&bstrDescription), S_OK);

	if( g_fKagera )
	{
		// ODBC error messages limited by SQL_MAX_MESSSAGE_LENGTH = 512
		TESTC(wcslen(bstrDescription) == 511 );
		TESTC( NULL != wcsstr(bstrDescription, L"Exception thrown by user:- This exception message contains more than 255 characters"));
	}
	else
	{
		TESTC( NULL != wcsstr(bstrDescription, wszErrorBody));

	}


CLEANUP:

	SAFE_RELEASE(pIErrorInfo);
	SysFreeString(bstrDescription);

	Execute(wszDropProc, FALSE);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc SCALEISNEGATIVE check for parameters
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Misc::Variation_3()
{ 
	WCHAR *			pwszTable = NULL;
	WCHAR			wszCreateTable[MAXBUFLEN];
	WCHAR			wszDropTable[MAXBUFLEN];
	WCHAR			wszInsert[MAXBUFLEN];
	DB_UPARAMS		cParams;
	DBPARAMINFO *	rgParamInfo = NULL;
	WCHAR *			pwszNames = NULL;

	static WCHAR s_wszCreateTable[] = L"create table %s (c1 number(38,-1), c2 number(5, -84), c3 number(9, -40))";
	static WCHAR s_wszInsert[] = L"insert into %s(c1, c2, c3) values (?, ?, ?)";
	
	// Only relevant for Kagera
	// since this depends on parameter derivation for non PL/SQL blocks.
	if( !g_fKagera )
		return TEST_SKIPPED;

	pwszTable = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_TABLELEN);
	if( !pwszTable )
		return FALSE;

	swprintf(wszCreateTable, s_wszCreateTable, pwszTable);
	swprintf(wszInsert, s_wszInsert, pwszTable);

	wcscpy(wszDropTable, L"drop table ");
	wcscat(wszDropTable, pwszTable);

	TESTC_(Execute(wszCreateTable), S_OK);


	// Prepare the command and the parameter information
	TESTC(SUCCEEDED(m_pICommandText->SetCommandText(DBGUID_DEFAULT, wszInsert)));
		
	if( m_pICommandPrepare )
	{
		TESTC(SUCCEEDED(m_pICommandPrepare->Prepare(0)));
		if( m_pICmdWParams )
		{
			TESTC_(m_pICmdWParams->GetParameterInfo(&cParams, &rgParamInfo, &pwszNames),S_OK);

			TESTC(cParams == 3);

			COMPARE(rgParamInfo[0].iOrdinal, 1);
			COMPARE(rgParamInfo[1].iOrdinal, 2);
			COMPARE(rgParamInfo[2].iOrdinal, 3);

			COMPARE(rgParamInfo[0].dwFlags & DBPARAMFLAGS_SCALEISNEGATIVE , DBPARAMFLAGS_SCALEISNEGATIVE );
			COMPARE(rgParamInfo[1].dwFlags & DBPARAMFLAGS_SCALEISNEGATIVE , DBPARAMFLAGS_SCALEISNEGATIVE );
			COMPARE(rgParamInfo[2].dwFlags & DBPARAMFLAGS_SCALEISNEGATIVE , DBPARAMFLAGS_SCALEISNEGATIVE );
			
			COMPARE(rgParamInfo[0].bPrecision, 38);
			COMPARE(rgParamInfo[1].bPrecision, 5);
			COMPARE(rgParamInfo[2].bPrecision, 9);

			COMPARE(rgParamInfo[0].bScale, 1);
			COMPARE(rgParamInfo[1].bScale, 84);
			COMPARE(rgParamInfo[2].bScale, 40);
		}
	}

CLEANUP:

	SAFE_FREE(pwszTable);

	PROVIDER_FREE(pwszNames);
	PROVIDER_FREE(rgParamInfo);

	Execute(wszDropTable, FALSE);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Native Error on DB_E_ERRORSINCOMMAND
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Misc::Variation_4()
{ 
	HRESULT			hr;
	
	// Only relevant for Oracle provider
	if( g_fKagera )
		return TEST_SKIPPED;

	hr = Execute(L"asdasodjas");
	if(!CHECK(hr, DB_E_ERRORSINCOMMAND))
		return TEST_FAIL;
	
	return CheckNativeError(hr, IID_ICommand, 1);
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Native Error on E_FAIL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int Misc::Variation_5()
{ 
	WCHAR 	wszDropPKTable[MAXTMPBUF];
	WCHAR 	wszDropFKTable[MAXTMPBUF];
	WCHAR 	wszCreatePKTable[MAXTMPBUF];
	WCHAR 	wszCreateFKTable[MAXTMPBUF];
	WCHAR	wszInsert[MAXTMPBUF];
	WCHAR *	pwszPKTable = NULL;
	WCHAR *	pwszFKTable = NULL;
	HRESULT	hr;

	WCHAR	s_wszCreatePKTable[] = L"create table %s (c1 number primary key)";
	WCHAR	s_wszCreateFKTable[] = L"create table %s (c1 number, foreign key (c1) references %s)";
	WCHAR	s_wszInsert[] = L"insert into %s values (1)";

	// Only relevant for Oracle provider
	if( g_fKagera )
		return TEST_SKIPPED;

	pwszPKTable = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_TABLELEN);
	pwszFKTable = MakeObjectName(m_pAutoMakeTable->GetModuleName(), ORAMAX_TABLELEN);

	swprintf(wszCreatePKTable, s_wszCreatePKTable, pwszPKTable);
	swprintf(wszCreateFKTable, s_wszCreateFKTable, pwszFKTable, pwszPKTable);
	swprintf(wszInsert, s_wszInsert, pwszFKTable);

	wcscpy(wszDropPKTable, L"drop table ");
	wcscat(wszDropPKTable, pwszPKTable);

	wcscpy(wszDropFKTable, L"drop table ");
	wcscat(wszDropFKTable, pwszFKTable);

	TESTC_(Execute(wszCreatePKTable), S_OK);
	TESTC_(Execute(wszCreateFKTable), S_OK);

	// Insert a value into the FK that doesn't correlate to a value in the PK table.
	hr = Execute(wszInsert);
	if(!CHECK(hr, E_FAIL))
		return TEST_FAIL;

	TESTC(CheckNativeError(hr, IID_ICommand, 1));

CLEANUP:

	SAFE_FREE(pwszPKTable);
	SAFE_FREE(pwszFKTable);

	Execute(wszDropPKTable);
	Execute(wszDropFKTable);
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL Misc::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END




// {{ TCW_TC_PROTOTYPE(RefCursor_Misc)
//*-----------------------------------------------------------------------
//| Test Case:		RefCursor_Misc - Miscellaneous REF CURSOR cases
//| Created:  	12/8/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RefCursor_Misc::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if (g_fKagera)
		return TEST_SKIPPED;

	if(COraproc::Init())
	// }}
	{ 
		TESTC_(CreateRefCursorTable(NO_BLOB_TABLE),S_OK);		
		return TRUE;
	} 
CLEANUP:
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Invoke ref cursor with a param before escape clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_1()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRAPARAM_FIRST, FALSE, FALSE, 3);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invoke ref cursor with a param after escape clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_2()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRAPARAM_LAST, FALSE, FALSE, 5);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invoke ref cursor with a literal before escape clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_3()
{ 
	BOOL  fPass = TEST_FAIL;
	WCHAR wszParamInvokeFmt[] = L"{call %s.%s (22.0, {resultset 0, io_cursor})}";
	WCHAR wszCallProc[MAXDATALEN];

	TESTC_(CreateRefCursorProc(USE_OPENFOR | EXTRALITERAL_FIRST),S_OK);

	swprintf(wszCallProc, wszParamInvokeFmt, m_pwszPackageName, m_pwszRefProcName);
	SetRefCursorCallText(wszCallProc);
	
	TESTC(TestRefCursorProc(FALSE, IID_IRowset));
	fPass = TEST_PASS;

CLEANUP:
	DropRefCursorProc();
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Invoke ref cursor with a literal after escape clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_4()
{ 
	BOOL  fPass = TEST_FAIL;
	WCHAR wszParamInvokeFmt[] = L"{call %s.%s ({resultset 0, io_cursor}, -1)}";
	WCHAR wszCallProc[MAXDATALEN];

	TESTC_(CreateRefCursorProc(USE_OPENFOR | EXTRALITERAL_LAST),S_OK);

	swprintf(wszCallProc, wszParamInvokeFmt, m_pwszPackageName, m_pwszRefProcName);
	SetRefCursorCallText(wszCallProc);
	
	TESTC(TestRefCursorProc(FALSE, IID_IRowset));
	fPass = TEST_PASS;

CLEANUP:
	DropRefCursorProc();
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Strong Cursor, Param Before, No prepare, No setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_5()
{ 
	return TestRefCursWParam(STRONG_TYPE | USE_DIRECTASSIGN | EXTRAPARAM_FIRST, FALSE, FALSE, 3);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Strong Cursor, Param After, No prepare, No setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_6()
{ 
	return TestRefCursWParam(STRONG_TYPE | USE_DIRECTASSIGN | EXTRAPARAM_LAST, FALSE, FALSE, 5);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Weak Cursor, Param Before, Prepare, No setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_7()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRAPARAM_FIRST, TRUE, FALSE, 6);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Weak Cursor, Param After, Prepare, No setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_8()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRAPARAM_LAST, TRUE, FALSE, 7);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Strong Cursor, Param Before, Prepare, No setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_9()
{ 
	return TestRefCursWParam(STRONG_TYPE | USE_OPENFOR | EXTRAPARAM_FIRST, TRUE, FALSE, 8);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Strong Cursor, Param After, Prepare, No setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_10()
{ 
	return TestRefCursWParam(STRONG_TYPE | USE_DIRECTASSIGN | EXTRAPARAM_LAST, TRUE, FALSE, 9);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Weak Cursor, Param Before, No Prepare, Setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_11()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRAPARAM_FIRST, FALSE, TRUE, 10);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Weak Cursor, Param After, No Prepare, Setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_12()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRAPARAM_LAST, FALSE, TRUE, 3);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Strong Cursor, Param Before, No prepare, Setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_13()
{ 
	return TestRefCursWParam(STRONG_TYPE | USE_OPENFOR | EXTRAPARAM_FIRST, FALSE, TRUE, 2);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Strong Cursor, Param After, No prepare, Setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_14()
{ 
	return TestRefCursWParam(STRONG_TYPE | USE_OPENFOR | EXTRAPARAM_LAST, FALSE, TRUE, 11);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Weak Cursor, Param Before, Prepare, Setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_15()
{ 
	return TestRefCursWParam(USE_DIRECTASSIGN | EXTRAPARAM_FIRST, TRUE, TRUE, 12);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Weak Cursor, Param After, Prepare, Setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_16()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRAPARAM_LAST, TRUE, TRUE, 1);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Strong Cursor, Param Before, Prepare, Setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_17()
{ 
	return TestRefCursWParam(STRONG_TYPE | USE_OPENFOR | EXTRAPARAM_FIRST, TRUE, TRUE, 13);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Strong Cursor, Param After, Prepare, Setparaminfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_18()
{ 
	return TestRefCursWParam(STRONG_TYPE | USE_DIRECTASSIGN | EXTRAPARAM_LAST, TRUE, TRUE, 14);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Retrieve partially fetched cursor
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Misc::Variation_19()
{ 
	BOOL  fPass = TEST_FAIL;

	TESTC_(CreateRefCursorProc(PREFETCHED_CURS | USE_OPENFOR),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset));

	fPass = TEST_PASS;

CLEANUP:
	DropRefCursorProc();
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL RefCursor_Misc::Terminate()
{ 
	CHECK(DropRefCursorTable(),S_OK);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(RefCursor_MiscLONG)
//*-----------------------------------------------------------------------
//| Test Case:		RefCursor_MiscLONG - Miscellaneous REF CURSOR cases 
//| Created:  	12/8/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RefCursor_MiscLONG::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if (g_fKagera)
		return TEST_SKIPPED;

	if(COraproc::Init())
	// }}
	{ 
		TESTC_(CreateRefCursorTable(LONG_TABLE),S_OK);		
		return TRUE;
	} 
CLEANUP:
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Invoke ref cursor with 2 parameters after resultset clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_MiscLONG::Variation_1()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRATWOPARAM_LAST, FALSE, FALSE, 3);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invoke ref cursor with 2 parameters after resultset clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_MiscLONG::Variation_2()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRATWOPARAM_LAST, TRUE, FALSE, 1);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invoke ref cursor with 2 parameters after resultset clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_MiscLONG::Variation_3()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRATWOPARAM_LAST, FALSE, TRUE, 2);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Invoke ref cursor with 2 parameters after resultset clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_MiscLONG::Variation_4()
{ 
	return TestRefCursWParam(USE_OPENFOR | EXTRATWOPARAM_LAST, TRUE, TRUE, 4);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Invoke ref cursor with 2 literals after resultset clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_MiscLONG::Variation_5()
{ 
	BOOL  fPass = TEST_FAIL;
	WCHAR wszParamInvokeFmt[] = L"{call %s.%s ({resultset 0, io_cursor}, 1, 'a')}";
	WCHAR wszCallProc[MAXDATALEN];

	TESTC_(CreateRefCursorProc(USE_OPENFOR | EXTRATWOLITERAL_LAST),S_OK);

	swprintf(wszCallProc, wszParamInvokeFmt, m_pwszPackageName, m_pwszRefProcName);
	SetRefCursorCallText(wszCallProc);
	
	TESTC(TestRefCursorProc(FALSE, IID_IRowset));
	fPass = TEST_PASS;

CLEANUP:
	DropRefCursorProc();
	return fPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL RefCursor_MiscLONG::Terminate()
{ 
	CHECK(DropRefCursorTable(),S_OK);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(RefCursor_Invalid)
//*-----------------------------------------------------------------------
//| Test Case:		RefCursor_Invalid - Error cases involving Ref Cursors
//| Created:  	12/8/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RefCursor_Invalid::Init()
{ 
	if (g_fKagera)
		return TEST_SKIPPED;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 
		TESTC_(CreateRefCursorTable(NO_BLOB_TABLE),S_OK);		
		return TRUE;
	} 
CLEANUP:
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Use non-zero row count in resultset clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Invalid::Variation_1()
{ 
	WCHAR wszBoundaryCall[] = L"{call %s.%s ({resultset 1, io_cursor})}";
	WCHAR wszCall[MAXDATALEN];

	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_OPENFOR),S_OK);
	swprintf(wszCall, wszBoundaryCall, m_pwszPackageName, m_pwszRefProcName);
	TEST2C_(Execute(wszCall), DB_E_ERRORSINCOMMAND, E_FAIL);

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Extra whitespace
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Invalid::Variation_2()
{ 
	WCHAR wszBoundaryCall[] = L" {  call  %s.%s ( {  resultset  0,  io_cursor } ) } ";
	WCHAR	wszCall[MAXDATALEN];

	TESTC_(CreateRefCursorProc(USE_OPENFOR),S_OK);
	swprintf(wszCall, wszBoundaryCall, m_pwszPackageName, m_pwszRefProcName);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Missing Comma
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Invalid::Variation_3()
{ 
	WCHAR wszBoundaryCall[] = L"{call %s.%s({resultset 0 io_cursor})}";
	WCHAR	wszCall[MAXDATALEN];

	TESTC_(CreateRefCursorProc(USE_OPENFOR),S_OK);
	swprintf(wszCall, wszBoundaryCall, m_pwszPackageName, m_pwszRefProcName);
	TESTC_(Execute(wszCall), DB_E_ERRORSINCOMMAND);

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Dangling Comma
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Invalid::Variation_4()
{ 
	WCHAR wszBoundaryCall[] = L"{call %s.%s({resultset 0, })}";
	WCHAR	wszCall[MAXDATALEN];

	TESTC_(CreateRefCursorProc(USE_OPENFOR),S_OK);
	swprintf(wszCall, wszBoundaryCall, m_pwszPackageName, m_pwszRefProcName);
	TESTC_(Execute(wszCall), DB_E_ERRORSINCOMMAND);

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Misspelled Resultset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Invalid::Variation_5()
{ 
	WCHAR wszBoundaryCall[] = L"{call %s.%s({resulset 0, })}";
	WCHAR	wszCall[MAXDATALEN];

	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_DIRECTASSIGN),S_OK);
	swprintf(wszCall, wszBoundaryCall, m_pwszPackageName, m_pwszRefProcName);
	TESTC_(Execute(wszCall), DB_E_ERRORSINCOMMAND);

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Non-number 2nd arg
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Invalid::Variation_6()
{ 
	WCHAR wszBoundaryCall[] = L"{call %s.%s({resultset IO_CURSOR, 0})}";
	WCHAR	wszCall[MAXDATALEN];

	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_DIRECTASSIGN),S_OK);
	swprintf(wszCall, wszBoundaryCall, m_pwszPackageName, m_pwszRefProcName);
	TESTC_(Execute(wszCall), DB_E_ERRORSINCOMMAND);

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Only 2 args
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Invalid::Variation_7()
{ 
	WCHAR wszBoundaryCall[] = L"{call %s.%s({resultset 0})}";
	WCHAR wszCall[MAXDATALEN];

	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_DIRECTASSIGN),S_OK);
	swprintf(wszCall, wszBoundaryCall, m_pwszPackageName, m_pwszRefProcName);
	TESTC_(Execute(wszCall), DB_E_ERRORSINCOMMAND);

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Badly qualified call
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Invalid::Variation_8()
{ 
	WCHAR wszBoundaryCall[] = L"{call .%s.%s({resultset 0, io_cursor})}";
	WCHAR wszCall[MAXDATALEN];

	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_DIRECTASSIGN),S_OK);
	swprintf(wszCall, wszBoundaryCall, m_pwszPackageName, m_pwszRefProcName);
	TESTC_(Execute(wszCall), DB_E_ERRORSINCOMMAND);

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Unclosed last brace
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor_Invalid::Variation_9()
{ 
	WCHAR wszBoundaryCall[] = L"{call %s.%s({resultset 0, io_cursor})";
	WCHAR wszCall[MAXDATALEN];

	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_DIRECTASSIGN),S_OK);
	swprintf(wszCall, wszBoundaryCall, m_pwszPackageName, m_pwszRefProcName);
	TEST2C_(Execute(wszCall), DB_E_ERRORSINCOMMAND, E_FAIL);

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL RefCursor_Invalid::Terminate()
{ 
	CHECK(DropRefCursorTable(),S_OK);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(RefCursor_NoBlobs)
//*-----------------------------------------------------------------------
//| Test Case:		RefCursor_NoBlobs - Generic ref cursor testing on a rowset that has no BLOBs
//| Created:  	12/9/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RefCursor::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COraproc::Init())
	// }}
	{ 		
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Strong cursor, OPEN FOR
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_1()
{ 
	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_OPENFOR),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Strong cursor, Direct Assignment
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_2()
{ 
	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_DIRECTASSIGN),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IUnknown));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Weak Cursor, OPEN FOR
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_3()
{ 
	TESTC_(CreateRefCursorProc(USE_OPENFOR),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowsetInfo));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Weak Cursor, Direct Assignment
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_4()
{ 
	TESTC_(CreateRefCursorProc(USE_DIRECTASSIGN),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IColumnsInfo));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Strong Cursor, OPEN FOR, optional IRowsetFind
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_5()
{ 
	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_OPENFOR),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset, 1, &g_FindPropOpt));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Strong cursor, Direct Assignment, optional IRowsetFind
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_6()
{ 
	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_OPENFOR),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset, 1, &g_FindPropOpt));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Weak cursor, OPEN FOR, optional IRowsetFind
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_7()
{ 
	TESTC_(CreateRefCursorProc(USE_OPENFOR),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset, 1, &g_FindPropOpt));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Weak cursor, Direct Assignment, optional IRowsetFind
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_8()
{ 
	TESTC_(CreateRefCursorProc(USE_DIRECTASSIGN),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset, 1, &g_FindPropOpt));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Strong cursor, OPEN FOR, various props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_9()
{ 
	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_OPENFOR),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset, NUMELEM(g_rgVariousProps), g_rgVariousProps));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Strong cursor, Direct Assignment, various props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_10()
{ 
	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_DIRECTASSIGN),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset, NUMELEM(g_rgVariousProps), g_rgVariousProps));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Weak cursor, OPEN FOR, various props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_11()
{ 
	TESTC_(CreateRefCursorProc(USE_OPENFOR),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset, NUMELEM(g_rgVariousProps), g_rgVariousProps));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Weak cursor, Direct assignment, various props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_12()
{ 
	TESTC_(CreateRefCursorProc(USE_DIRECTASSIGN),S_OK);
	TESTC(TestRefCursorProc(FALSE, IID_IRowset, NUMELEM(g_rgVariousProps), g_rgVariousProps));

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Verify PROCEDURE COLUMNS schema rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_13()
{ 
	TESTC_(CreateRefCursorProc(STRONG_TYPE | USE_DIRECTASSIGN),S_OK);
	TESTC(VerifyProcColumns());

CLEANUP:
	DropRefCursorProc();
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Use array parameters
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int RefCursor::Variation_14()
{ 
	WCHAR *		pwszCreatePkg = NULL;
	WCHAR *		pwszCreatePkgBody = NULL;
	WCHAR *		pwszCallProc = NULL;
	WCHAR *		pwszDropPkg = NULL;
	IUnknown *	pUnk = NULL;
	IRowset *	pRowset = NULL;

	TESTC(CreateArrayPkgFromTable(m_pRefCursorTable, &pwszCreatePkg, &pwszCreatePkgBody,
		&pwszCallProc, &pwszDropPkg));

	TESTC_(Execute(pwszCreatePkg), S_OK);
	TESTC_(Execute(pwszCreatePkgBody), S_OK);
	
	SetRefCursorCallText(pwszCallProc);
	
	// Execute the Proc
	TESTC_(ExecuteProc(m_pwszCallRefProc, NO_PREPARE, NOSETPARAMINFO, IID_IUnknown, NULL, &pUnk),S_OK);
	TESTC(pUnk != NULL);
	TESTC(DefaultObjectTesting(pUnk, ROWSET_INTERFACE));

	// Traverse Rowset and verify data
	TESTC(VerifyInterface(pUnk, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pRowset));
	TESTC(VerifyRowset(pRowset));

CLEANUP:

	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pUnk);

	if(pwszDropPkg)
		Execute(pwszDropPkg);

	// Drop package and procedure.
	SAFE_FREE(pwszCreatePkg);
	SAFE_FREE(pwszCreatePkgBody);
	SAFE_FREE(pwszCallProc);
	SAFE_FREE(pwszDropPkg);

	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL RefCursor::Terminate()
{ 
	CHECK(DropRefCursorTable(),S_OK);

// {{ TCW_TERM_BASECLASS_CHECK2
	return(COraproc::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

//*-----------------------------------------------------------------------
// RefCursor_NoBlobs SubTestCase
BOOL RefCursor_NoBlobs::Init()
{ 
	if (g_fKagera)
		return TEST_SKIPPED;

	if(RefCursor::Init())
	{ 
		TESTC_(CreateRefCursorTable(NO_BLOB_TABLE),S_OK);		
		return TRUE;
	} 
CLEANUP:
	return FALSE;
} 

BOOL RefCursor_NoBlobs::Terminate()
{ 
	return(RefCursor::Terminate());
}

//*-----------------------------------------------------------------------
// RefCursor_LONGBlobs SubTestCase
BOOL RefCursor_LONGBlobs::Init()
{ 
	if (g_fKagera)
		return TEST_SKIPPED;

	if(RefCursor::Init())
	{ 
		TESTC_(CreateRefCursorTable(LONG_TABLE),S_OK);		
		return TRUE;
	} 
CLEANUP:
	return FALSE;
} 

BOOL RefCursor_LONGBlobs::Terminate()
{ 
	return(RefCursor::Terminate());
}

//*-----------------------------------------------------------------------
// RefCursor_LONGRAWBlobs SubTestCase
BOOL RefCursor_LONGRAWBlobs::Init()
{ 
	if (g_fKagera)
		return TEST_SKIPPED;

	if(RefCursor::Init())
	{ 
		TESTC_(CreateRefCursorTable(LONGRAW_TABLE),S_OK);		
		return TRUE;
	} 
CLEANUP:
	return FALSE;
} 

BOOL RefCursor_LONGRAWBlobs::Terminate()
{ 
	return(RefCursor::Terminate());
}


//*-----------------------------------------------------------------------
// Many columns in the Rowset
BOOL RefCursor_ManyColumns::Init()
{ 
	if (g_fKagera)
		return TEST_SKIPPED;

	if(RefCursor::Init())
	{ 
		TESTC_(CreateRefCursorTable(MANYCOLUMNS_TABLE),S_OK);		
		return TRUE;
	} 
CLEANUP:
	return FALSE;
} 


BOOL RefCursor_ManyColumns::Terminate()
{ 
	return(RefCursor::Terminate());
}


//*-----------------------------------------------------------------------
// Many more columns in the Rowset
BOOL RefCursor_Many_50_Columns::Init()
{ 
	if (g_fKagera)
		return TEST_SKIPPED;

	if(RefCursor::Init())
	{ 
		TESTC_(CreateRefCursorTable(MANY_50_COLUMNS_TABLE),S_OK);		
		return TRUE;
	} 
CLEANUP:
	return FALSE;
} 


BOOL RefCursor_Many_50_Columns::Terminate()
{ 
	return(RefCursor::Terminate());
}



//--------------------------------------------------------------------
// Microsoft OLE DB Test
// 
// Copyright 1996-2000 Microsoft Corporation.
//
// @doc 
//
// @module ICmdWParams.CPP | ICmdWParams source file
//
//--------------------------------------------------------------------


#include "MODStandard.hpp"

#define DBINITCONSTANTS		// Must be defined to initialize constants in oledb.h
#define INITGUID			// For IID_ITransactionOptions, etc.

#include "ICmdWPar.h"
#include "msdasql.h"
#include "extralib.h" 
#include <time.h>
#include <malloc.h>	// _heapchk()

#define FREE_DATA(pData)	PROVIDER_FREE(pData)

#define TEST_CHECK(exp, hres)	{ if (!CHECK ((exp), (hres))) { goto CLEANUP; } }

#define TEST_COMPARE(exp, val)	{ if (!COMPARE ((exp), (val))) { goto CLEANUP; } }

#define RELEASE(ptr)	SAFE_RELEASE(ptr)

#define TEST_PTR(ptr)	{if (!ptr) {odtLog << L"Out of memory!\n"; goto CLEANUP;}}

#define ABORT_PTR(ptr)	{if (!ptr) {odtLog << L"Out of memory!\n"; fResult = FALSE; goto CLEANUP;}}

#define TEST_ALLOC(ptype, ptr, fill, cb)	{ ptr = (ptype *)PROVIDER_ALLOC(cb); TEST_PTR(ptr); memset(ptr, fill, cb); }

#define ABORT_ALLOC(ptype, ptr, fill, cb)	{ ptr = (ptype *)PROVIDER_ALLOC(cb); ABORT_PTR(ptr); memset(ptr, fill, cb); }

#define FAIL_VAR(exp, hres) if (!(exp == hres)) fResult = FALSE

#define FAIL_CHECK(exp, hres) if (!CHECK((exp), (hres))) fResult = FALSE

#define IF_CHECK(exp, hres) if (!CHECK((exp), (hres))) fResult = FALSE; if ((exp) == (hres))

#define FAIL_COMPARE(exp, val) if (!COMPARE((exp), (val))) fResult = FALSE

#define IF_COMPARE(exp, val) if (!COMPARE((exp), (val))) fResult = FALSE; if ((exp) == (val))

#define ABORT_CHECK(exp, hres)	{ if (!CHECK ((exp), (hres))) { fResult = FALSE; goto CLEANUP; } }

#define ABORT_COMPARE(exp, val)	{ if (!COMPARE ((exp), (val))) { fResult = FALSE; goto CLEANUP; } }

#define SAFE_RELEASE_ACCESSOR(pIAcc, hAcc) {if ((pIAcc) && (hAcc) && \
	CHECK((pIAcc)->ReleaseAccessor((hAcc), NULL), S_OK)) (hAcc) = DB_NULL_HACCESSOR;}

#define DEBUGVAR	odtLog << L"***********************************************************************\n";\
					odtLog << L"Currently using customized code to track down bugs.\n";\
					odtLog << L"Remove debugging code.\n";\
					odtLog << L"***********************************************************************\n";

const CLSID		CLSID_ConfProv		= {0xb2a233c1, 0x5b20, 0x11d0, {0x84, 0x18, 0x0, 0xaa, 0x00, 0x3f, 0xd, 0xd4}};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x26638211, 0x7c75, 0x11cf, { 0xac, 0xa3, 0x00, 0xaa, 0x00, 0x4a, 0x99, 0xe0 }};
DECLARE_MODULE_NAME("ICommandWithParameters");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test for ICommandWithParameters");
DECLARE_MODULE_VERSION(831765099);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END


// Globals
typedef struct tagParamStruct 
{
	DBORDINAL ulColIndex;
	DBPARAMIO eParamIO;
	WCHAR * pwszParamName;
	WCHAR wszDataSourceType[SP_MAX_PARAMNAME_LENGTH];
	enum COMPARE_OP eCompareOp;
} ParamStruct;

// Valid storage flags, plus one invalid one
#define STGM_INVALID	0x80000000L		// Not currently valid
struct tagStorageModes {
	DWORD dwMode;
	WCHAR wszMode[30];
};
typedef struct tagStorageModes StorageModes;

const StorageModes g_StorageFlags[]={
	EXPAND(STGM_READ),					
	EXPAND(STGM_WRITE),
	EXPAND(STGM_READWRITE),
	EXPAND(STGM_SHARE_DENY_NONE),
	EXPAND(STGM_SHARE_DENY_READ),
	EXPAND(STGM_SHARE_DENY_WRITE),
	EXPAND(STGM_SHARE_EXCLUSIVE),
//	EXPAND(STGM_DIRECT),		// Same as STGM_READ!
	EXPAND(STGM_TRANSACTED),
	EXPAND(STGM_CREATE),
	EXPAND(STGM_CONVERT),
//	EXPAND(STGM_FAILIFTHERE),	// Same as STGM_READ!
	EXPAND(STGM_PRIORITY),
	EXPAND(STGM_DELETEONRELEASE),
	EXPAND(STGM_INVALID)
};

WCHAR *		g_pwszProcedureName;
WCHAR *		g_pwszProcedureName2;
BOOL		g_bMultipleParamSets;
BOOL		g_bSqlServer;
BOOL		g_bKagera;
BOOL		g_bOracle;
BOOL		g_bLuxor;
BOOL		g_bConfProv;
ULONG_PTR	g_ulOutParamsSupported;
BOOL		g_fRowsetTest;
BOOL		g_fRowObj;

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	g_pwszProcedureName = NULL;
	g_pwszProcedureName2 = NULL;
	g_bMultipleParamSets = FALSE;
	g_bSqlServer = FALSE;
	g_bKagera = FALSE;
	g_bOracle = FALSE;
	g_bLuxor = FALSE;
	g_bConfProv = FALSE;
	g_ulOutParamsSupported = DBPROPVAL_OA_NOTSUPPORTED;
	g_fRowsetTest = TRUE;
	g_fRowObj = FALSE;

	if (ModuleCreateDBSession(pThisTestModule))
	{
		HRESULT				hr=E_FAIL;
		IDBCreateCommand	*pIDBCreateCommand=NULL;
		IDBInitialize *		pIDBInitialize = NULL;
		CTable * pTable = NULL;
		LPWSTR pwszDBMSName = NULL;
		LPWSTR pwszName = NULL;

		// Fail gracefully and quit module if we don't support Commands
		if (SUCCEEDED(hr = pThisTestModule->m_pIUnknown2->QueryInterface(
			IID_IDBCreateCommand, (void **)&pIDBCreateCommand)))
		{
			ICommandWithParameters * pICmdWPar = NULL;

			hr = pIDBCreateCommand->CreateCommand(NULL, IID_ICommandWithParameters, (IUnknown **)&pICmdWPar);

			pIDBCreateCommand->Release();
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
		else
		{
			// Make sure we returned E_NOINTERFACE if we've failed
			if (pThisTestModule->m_pError->Validate(hr,	
								LONGSTRING(__FILE__), __LINE__, E_NOINTERFACE))
				odtLog <<L"Commands are not supported.\n";

			return TEST_SKIPPED;
		}

		// This test doesn't support using an ini file, make sure we're not 
		if(GetModInfo()->GetFileName())
		{
			odtLog << L"INFO: Test does not support using fixed table from ini file, resetting...\n";
			GetModInfo()->ResetIniFile();
		}

		// This test doesn't support using INSERT_ROWSETCHANGE because it needs the command text back
		if (GetModInfo()->GetInsert() == INSERT_ROWSETCHANGE)
		{
			odtLog << L"INFO: Test does not support using INSERT_ROWSETCHANGE, resetting to INSERT_COMMAND\n";
			GetModInfo()->SetInsert(INSERT_COMMAND);
		}

		//Create a table we'll use for the whole test module,
		//store it in pVoid for now
		if (!(pTable = new CTable((IUnknown *)pThisTestModule->m_pIUnknown2, 
			(LPWSTR)gwszModuleName)))
//			(LPWSTR)gwszModuleName, NONULLS)))
		{
			PRVTRACE (wszMemoryAllocationError);
			return FALSE;
		}

		// Save the table
		pThisTestModule->m_pVoid = pTable;			
		
		if(!CHECK(pTable->CreateTable(TOTAL_NUMBER_OF_ROWS), S_OK))
			return FALSE;

		// Create procedure names for default procedures
		g_pwszProcedureName = MakeObjectName(L"ICmdProc", wcslen(pTable->GetTableName()));
		g_pwszProcedureName2 = MakeObjectName(L"ICmdProc", wcslen(pTable->GetTableName()));

		if (!g_pwszProcedureName || !g_pwszProcedureName2)
			return FALSE;

		if (!VerifyInterface(pThisTestModule->m_pIUnknown, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInitialize))
			return FALSE;

		// Get output param support
		GetProperty(DBPROP_OUTPUTPARAMETERAVAILABILITY, 
					   DBPROPSET_DATASOURCEINFO,pIDBInitialize, &g_ulOutParamsSupported);

		// Get multiple paramsets support
		g_bMultipleParamSets = GetProperty(DBPROP_MULTIPLEPARAMSETS, 
					   DBPROPSET_DATASOURCEINFO,pIDBInitialize, VARIANT_TRUE);

		if (pThisTestModule->m_ProviderClsid == CLSID_ConfProv)
				g_bConfProv = TRUE;

		if(GetProperty(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO, pIDBInitialize, &pwszName))
		{
			if (!wcscmp(pwszName, L"MSDASQL.DLL"))
				g_bKagera = TRUE;
			if (!wcscmp(pwszName, L"sqloledb.dll"))
				g_bLuxor = TRUE;
			SAFE_FREE(pwszName);
		}


		if(GetProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, pIDBInitialize, &pwszDBMSName))
		{
			if (!wcscmp(pwszDBMSName, L"Microsoft SQL Server"))
				g_bSqlServer = TRUE;
			if (!wcscmp(pwszDBMSName, L"Oracle"))
				g_bOracle = TRUE;
		}

		// Hack to keep from pumping out the same failures over and over for known issues.
		// Access doesn't support output params or multiple paramsets, but Kagera doesn't know that
		// Remoting doesn't support output params or multiple paramsets either, but doesn't report correctly
		if (!wcscmp (pwszDBMSName, L"ACCESS"))
		{
			if (!COMPARE(g_ulOutParamsSupported, DBPROPVAL_OA_NOTSUPPORTED))
			{
				odtLog << L"Provider doesn't support output params, resetting to DBPROPVAL_OA_NOTSUPPORTED\n";
				g_ulOutParamsSupported = DBPROPVAL_OA_NOTSUPPORTED;
			}
			else
				odtLog << L"Provider now returns proper output param support, remove fixup code.\n";
		}

		if (!wcscmp (pwszDBMSName, L"ACCESS") || GetModInfo()->GetClassContext() == CLSCTX_LOCAL_SERVER)
		{
			if (!COMPARE(g_bMultipleParamSets, FALSE))
			{
				odtLog << L"Provider does not support multiple paramsets, resetting to FALSE.\n";
				g_bMultipleParamSets = FALSE;
			}
			else
				odtLog << L"Provider now returns proper multipleparamset support, remove fixup code.\n";
		}

		if (GetModInfo()->GetClassContext() == CLSCTX_LOCAL_SERVER)
		{
			odtLog << L"Provider does not allow passing valid ppRowset to Execute for output params, IID_NULL will be used.\n";
			g_fRowsetTest = FALSE;		// Interface Remoting does not yet support a rowset pointer if output params
		}

		// See if row objects are supported
		if (SupportedProperty(DBPROP_IRow, DBPROPSET_ROWSET, pIDBInitialize, ROWSET_INTERFACE))
			g_fRowObj = TRUE;

		SAFE_RELEASE(pIDBInitialize);
		SAFE_FREE(pwszDBMSName);

		//If we made it this far, everything has succeeded
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
	SAFE_FREE(g_pwszProcedureName);
	SAFE_FREE(g_pwszProcedureName2);

	if (pThisTestModule->m_pVoid)
	{
		((CTable *)pThisTestModule->m_pVoid)->DropTable();
		delete (CTable *)pThisTestModule->m_pVoid;
		pThisTestModule->m_pVoid = NULL;
	}

	return ModuleReleaseDBSession(pThisTestModule);
}	

ULONG wcschcount(LPWSTR pwsz, WCHAR ch);
BOOL IsErrorStatus(DBSTATUS sStatus);
DBLENGTH DisplaySize(CCol ColInfo);
size_t FormatString(WCHAR ** ppwszDest, WCHAR * pwszFmt, ULONG cArgs, ...);
size_t FormatStringFromArray(WCHAR ** ppwszDest, WCHAR * pwszFmt, ULONG cArgs, WCHAR ** ppInsertArray);
BOOL AddParam(
	DB_UPARAMS iParam,
	DBORDINAL iCol,
	DBPARAMIO eParamIO,
	WCHAR * pwszParamName,
	BOOL fBindByName,
	DBLENGTH * pcbRowSize,
	DBBINDING * pDBBINDINFO,
	DBPARAMBINDINFO * pDBPARAMBINDINFO,
	ParamStruct * pParamAll,
	CTable * pTable,
	BOOL fReturnParam = FALSE
);
void FreeParameterNames(DBCOUNTITEM cParams, ParamStruct * pParamInfo);




// Returns the size of a type, including variable length types, since
// WSTR2DBTYE ignores length output for fixed length types.
DBLENGTH GetDataSize(DBTYPE wType, DBLENGTH cbData)
{
	if (IsFixedLength(wType))
		return GetDBTypeSize(wType);
	else 
		return cbData;
}

LPWSTR GetStandardTypeName(DBTYPE wType)
{
	for (ULONG iTypeName=0; iTypeName < g_cStdParams; iTypeName++)
	{
		if (g_rgStdParamBindInfo[iTypeName].wType == wType)
			return g_rgStdParamBindInfo[iTypeName].wszStdTypeName;
	}
	return NULL;
}


//--------------------------------------------------------------------------
// AddParam
//   Add a parameter to the bind information arrays
//	
// Return parameter is assumed if pParamInfo is NULL, in which case wType
// and cbParam are used.  If pParamInfo is not NULL then this information
// is obtained from the associated table column.
//--------------------------------------------------------------------------
BOOL AddParam(
	DB_UPARAMS iParam,
	DBORDINAL iCol,
	DBPARAMIO eParamIO,
	WCHAR * pwszParamName,
	BOOL fBindByName,
	DBLENGTH * pcbRowSize,
	DBBINDING * pDBBINDINFO,
	DBPARAMBINDINFO * pDBPARAMBINDINFO,
	ParamStruct * pParamAll,
	CTable * pTable,
	BOOL fReturnParam
)
{
	CCol TempCol;
	BOOL fIsVariableLength = FALSE;
	DBTYPE wType;
	DBLENGTH cbParam;
	DBLENGTH cbOutParam;
	DBLENGTH cbRowSize=0;
	WCHAR * pTypeName=NULL;
	BYTE bPrecision;
	BYTE bScale;
	WCHAR * pwszStrip = NULL;

	ASSERT(eParamIO != 0);

	if (!pcbRowSize)
		pcbRowSize = &cbRowSize;

	// Get the associated column information
	if (!CHECK(pTable->GetColInfo(iCol, TempCol), S_OK))
		return FALSE;
	
	// Always use the native type
	wType = TempCol.GetProviderType();

	bPrecision = (BYTE) TempCol.GetPrecision();
	bScale = (BYTE)TempCol.GetScale();

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

	cbParam = TempCol.GetColumnSize();

	// For string params we need to have one more space in our bindings for the NULL term.
	cbOutParam = TempCol.GetMaxColumnSize();
	if (wType == DBTYPE_STR)
		cbOutParam+=sizeof(CHAR);
	if (wType == DBTYPE_WSTR)
		cbOutParam= 2 * cbOutParam + sizeof(WCHAR);

	// Save the data source type until SetParameterInfo time
	pTypeName=TempCol.GetProviderTypeName();
	if (!pTypeName || !pTypeName[0])
	{
		// We need to use a standard type name
		if (!(pTypeName=GetStandardTypeName(wType)))
			return FALSE;
	}

	// Create the DBBINDING info
	if (pDBBINDINFO)
	{
		pDBBINDINFO[iParam].iOrdinal=iParam+1;
		pDBBINDINFO[iParam].obLength=*pcbRowSize;
		*pcbRowSize+=sizeof(DBLENGTH);
		pDBBINDINFO[iParam].obStatus=*pcbRowSize;
		*pcbRowSize+=sizeof(DBSTATUS);

		// Adjust value for proper alignment
		*pcbRowSize = ROUND_UP(*pcbRowSize, ROUND_UP_AMOUNT);

		pDBBINDINFO[iParam].obValue=*pcbRowSize;
		pDBBINDINFO[iParam].cbMaxLen = cbOutParam;
		*pcbRowSize+=cbOutParam;

		// Adjust row size for proper alignment
		*pcbRowSize = ROUND_UP(*pcbRowSize, ROUND_UP_AMOUNT);

		pDBBINDINFO[iParam].pTypeInfo=NULL;
		pDBBINDINFO[iParam].pObject=NULL;			// We don't handle stream objects here
		pDBBINDINFO[iParam].pBindExt=NULL;
		pDBBINDINFO[iParam].dwPart=DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
		pDBBINDINFO[iParam].dwMemOwner=DBMEMOWNER_CLIENTOWNED;
		pDBBINDINFO[iParam].eParamIO = eParamIO;
		pDBBINDINFO[iParam].dwFlags = 0;			// TODO: Handle DBBINDFLAG_HTML?
		pDBBINDINFO[iParam].wType = wType;
		pDBBINDINFO[iParam].bPrecision = bPrecision;
		pDBBINDINFO[iParam].bScale = bScale;
	}

	// Populate the ParamStruct information for this clause to the one for the whole stmt
	if (pParamAll)
	{
		pParamAll[iParam].ulColIndex = iCol;
		pParamAll[iParam].eParamIO = eParamIO;
		pParamAll[iParam].pwszParamName = pwszParamName;

		wcscpy(pParamAll[iParam].wszDataSourceType, pTypeName);

		// Sql Server specific: If the type name contains a left paren, then strip it off
		if (pwszStrip = wcsstr(pParamAll[iParam].wszDataSourceType, L"("))
			*pwszStrip = L'\0';

		// Sql Server specific: If the type name contains "identity, then strip it off
		if (pwszStrip = wcsstr(pParamAll[iParam].wszDataSourceType, L"identity"))
			*pwszStrip = L'\0';

		// We assume the comparison operator will be '=' for this parameter
		pParamAll[iParam].eCompareOp = CP_EQ;
	}

	// Create the DBPARAMBINDINFO information
	if (pDBPARAMBINDINFO)
	{
		pDBPARAMBINDINFO[iParam].pwszDataSourceType = pParamAll[iParam].wszDataSourceType;
		pDBPARAMBINDINFO[iParam].pwszName = (fBindByName) ? pwszParamName : NULL;
		pDBPARAMBINDINFO[iParam].ulParamSize = cbParam;

		// Set the appropriate flags
		pDBPARAMBINDINFO[iParam].dwFlags=0;
		if (eParamIO & DBPARAMIO_OUTPUT)
			pDBPARAMBINDINFO[iParam].dwFlags |= DBPARAMFLAGS_ISOUTPUT;
		if (eParamIO & DBPARAMIO_INPUT)
			pDBPARAMBINDINFO[iParam].dwFlags |= DBPARAMFLAGS_ISINPUT;
		if (TempCol.GetIsLong())
			pDBPARAMBINDINFO[iParam].dwFlags |= DBPARAMFLAGS_ISLONG;
		// For SQL Server, return params are not nullable, otherwise expect nullability
		// to match the associated column
		if (TempCol.GetNullable())
		{
			if (!fReturnParam || !g_bSqlServer)
				pDBPARAMBINDINFO[iParam].dwFlags |= DBPARAMFLAGS_ISNULLABLE;
		}
		if (!TempCol.GetUnsigned())
			pDBPARAMBINDINFO[iParam].dwFlags |= DBPARAMFLAGS_ISSIGNED;

		pDBPARAMBINDINFO[iParam].bPrecision = bPrecision;
		pDBPARAMBINDINFO[iParam].bScale = bScale;
	}

	return TRUE;
}

// Adjust obStatus, obLength, and obValue due to changes in binding structure.
// Recompute cbRowSize also.  
void Repack(DBCOUNTITEM cBind, DBBINDING * pBIND, DBLENGTH * pcbRowSize)
{
	ULONG iBind =0;
	DBLENGTH ulOffset = 0;

	for (iBind=0; iBind < cBind; iBind++)
	{
		pBIND[iBind].obStatus = ulOffset;
		ulOffset+=sizeof(DBSTATUS);

		// Adjust for alignment of Length
		ulOffset = ROUND_UP(ulOffset,ROUND_UP_AMOUNT);

		pBIND[iBind].obLength = ulOffset;
		ulOffset+=sizeof(DBLENGTH);
		pBIND[iBind].obValue = ulOffset;
		// Note we assume cbMaxLen is already set appropriately for variable length types.
		pBIND[iBind].cbMaxLen = GetDataSize(pBIND[iBind].wType, pBIND[iBind].cbMaxLen);
		ulOffset+=GetDataSize(pBIND[iBind].wType, pBIND[iBind].cbMaxLen);

		// Adjust for alignment
		ulOffset = ROUND_UP(ulOffset,ROUND_UP_AMOUNT);
	}

	if (pcbRowSize)
		*pcbRowSize = ulOffset;
}


void FreeParameterNames(DBCOUNTITEM cParams, ParamStruct * pParamInfo)
{
	if (pParamInfo)
	{
		for (ULONG iParam = 0; iParam < cParams; iParam++)
			SAFE_FREE(pParamInfo[iParam].pwszParamName);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CErrorCache
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define CCHECK(ErrObj, hrAct, hrExp, ulErr, pwszMsg, fWarn)  (ErrObj.CCheck((hrAct), \
	(hrExp), (ulErr), (pwszMsg), (fWarn), LONGSTRING(__FILE__), (__LINE__)))
#define CCOMPARE(ErrObj, fResult, ulErr, pwszMsg, fWarn)  (ErrObj.CCompare((fResult), \
	(ulErr), (pwszMsg), (fWarn), LONGSTRING(__FILE__), (__LINE__)))

enum ERROR_CACHE_ENUM
{
	// GetParameterInfo cached errors
	EC_INVALID_PRECISION = 1,
	EC_INVALID_SCALE,
	EC_INVALID_ISSIGNED,
	EC_INVALID_ISNULLABLE,
	EC_INVALID_ISINPUT,
	EC_MISSING_ISINPUT_AND_ISOUTPUT_FLAGS,
	EC_UNEXPECTED_S_OK,
	EC_INVALID_PARAM_NAME,
	EC_MAX_ERROR_NUMBER			// Must always be last enum
};

LPWSTR g_ppwszErrorStrings[] = 
{
	// GetParameterInfo cached errors
	L"EC_INVALID_PRECISION",
	L"EC_INVALID_SCALE",
	L"EC_INVALID_ISSIGNED",
	L"EC_INVALID_ISNULLABLE",
	L"EC_INVALID_ISINPUT",
	L"EC_MISSING_ISINPUT_AND_ISOUTPUT_FLAGS",
	L"EC_UNEXPECTED_S_OK",
	L"EC_INVALID_PARAM_NAME",
};


class CErrorCache
{

private:

	ULONG m_ulDebugMode;
	ULONG m_ulMaxCachedErrors;
	ULONG * m_pulErrorCache;
	ULONG m_cErrorsCached;

	BOOL IsErrorCached(ULONG ulError);

public:

	CErrorCache(void);

	~CErrorCache(void);

	BOOL Init(ULONG ulDebugMode);

	BOOL CCheck(HRESULT hrActual, HRESULT hrExpected, ULONG ulError = 0,
		LPWSTR pwszMessage = NULL, BOOL fWarning = FALSE, LPWSTR pwszFile = NULL,
		ULONG ulLine = 0);
	
	BOOL CCompare(BOOL fResult, ULONG ulError = 0, LPWSTR pwszMessage = NULL,
		BOOL fWarning = FALSE, LPWSTR pwszFile = NULL, ULONG ulLine = 0);
};

CErrorCache::CErrorCache(void)
{
	m_ulDebugMode = 0;
	m_ulMaxCachedErrors = EC_MAX_ERROR_NUMBER-1;
	m_pulErrorCache = NULL;
	m_cErrorsCached = 0;
}

CErrorCache::~CErrorCache(void)
{
	// Print summary of errors cached if there is at least one
	// error cached.
	if (m_cErrorsCached)
	{
		odtLog << L"\n\nThe following duplicate errors were supressed:\n\n";

		for (ULONG iErr = 0; iErr < m_ulMaxCachedErrors; iErr++)
			if (m_pulErrorCache[iErr])
				odtLog << L"\t" << g_ppwszErrorStrings[iErr] <<
				L"\t\t" << m_pulErrorCache[iErr] << L"\n";

		odtLog << L"\n\n";
		odtLog << L"To see all error details instead of this summary please add 'DEBUGMODE=FULL;' to the init string.\n\n";

	}

	SAFE_FREE(m_pulErrorCache);
}

BOOL CErrorCache::Init(ULONG ulDebugMode)
{
	BOOL fResult = FALSE;

	m_ulDebugMode = ulDebugMode;

	// Allocate memory for error cache.  Since we don't expect this to be more than
	// a few 10's of items just use a static array.
	SAFE_ALLOC(m_pulErrorCache, ULONG, m_ulMaxCachedErrors);

	// Init all cache locations to 0, no error cached
	memset(m_pulErrorCache, 0, m_ulMaxCachedErrors*sizeof(ULONG));

	fResult = TRUE;

CLEANUP:

	return fResult;
}

BOOL CErrorCache::IsErrorCached(ULONG ulError)
{
	ASSERT(ulError <= m_ulMaxCachedErrors);

	// In case this gets called before init.
	if (!m_pulErrorCache)
		return FALSE;

	// If the cache has a value other than 0 then it's been cached
	return m_pulErrorCache[ulError-1];
}

BOOL CErrorCache::CCheck(HRESULT hrActual, HRESULT hrExpected, ULONG ulError,
		LPWSTR pwszMessage, BOOL fWarning, LPWSTR pwszFile,
		ULONG ulLine)
{
	BOOL fReturn = hrActual == hrExpected;

	// Check for valid error number
	if (ulError == 0 || ulError > m_ulMaxCachedErrors)
		return FALSE;

	// If this error is already cached then we allow it to pass unless debugmode is full.
	if (IsErrorCached(ulError) && !(m_ulDebugMode & DEBUGMODE_FULL))
	{
		if (!fReturn)
			// Update the cache
			m_pulErrorCache[ulError-1]++;
		return fReturn;
	}

	// Otherwise we have to perform the comparison.  Note we can't just return the
	// value from PrivLibValidate because on warning it always returns TRUE even on
	// a miscompare
	PrivlibValidate(hrActual, hrExpected, fWarning, pwszFile, ulLine);

	// If the comparison failed, then print failure message
	if (!fReturn)
	{
		// Update the cache
		if (!(m_ulDebugMode & DEBUGMODE_FULL))
		{
			m_pulErrorCache[ulError-1]++;
			m_cErrorsCached++;
		}

		if (pwszMessage)
			odtLog << pwszMessage << L"\n";
	}

	return fReturn;
}

BOOL CErrorCache::CCompare(BOOL fResult, ULONG ulError, LPWSTR pwszMessage,
		BOOL fWarning, LPWSTR pwszFile, ULONG ulLine)
{

	// Check for valid error number
	if (ulError == 0 || ulError > m_ulMaxCachedErrors)
		return FALSE;

	// If this error is already cached then we allow it to pass unless debugmode is full.
	if (IsErrorCached(ulError) && !(m_ulDebugMode & DEBUGMODE_FULL))
	{
		if (!fResult)
			// Update the cache
			m_pulErrorCache[ulError-1]++;
		return fResult;
	}

	// Otherwise we have to perform the comparison.  Note we can't just return the
	// value from PrivLibValidate because on warning it always returns TRUE even on
	// a miscompare
	PrivlibCompare(fResult, fWarning, pwszFile, ulLine);

	// If the comparison failed, then print failure message
	if (!fResult)
	{
		if (!(m_ulDebugMode & DEBUGMODE_FULL))
		{
			m_pulErrorCache[ulError-1]++;
			m_cErrorsCached++;
		}

		if (pwszMessage)
			odtLog << pwszMessage << L"\n";
	}

	return fResult;
}

class ICmdWParSequentialStream : public ISequentialStream {

private:
	ULONG m_ulRefCount;
	ULONG m_ulBufSize;
	ULONG m_ulReadSize;
	ULONG m_ulBytesLeft;
	ULONG m_ulReadPos;
	BYTE * m_pSrcData;
	BYTE * m_pReadPtr;
	BOOL m_fWasRead;
	BOOL m_fFree;

public:

	ICmdWParSequentialStream(void)
	{
		m_ulRefCount=1;
		m_ulBufSize=0;
		m_ulReadSize=0;
		m_ulBytesLeft=0;
		m_ulReadPos=0;
		m_pSrcData=NULL;
		m_pReadPtr=NULL;
		m_fWasRead = FALSE;
		m_fFree = FALSE;
	}

	~ICmdWParSequentialStream(void)
	{
		// If user requested stream object to free the source buffer
		if (m_fFree)
			SAFE_FREE(m_pSrcData);
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) {return ++m_ulRefCount;}

	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		--m_ulRefCount;
		if (m_ulRefCount == 0)
		{
			delete this;
			return 0;
		}
		return m_ulRefCount;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObj)
	{
		HRESULT hr=E_FAIL;

		if (!ppvObj)
			return E_INVALIDARG;
		else
			*ppvObj = NULL;
		
		if (riid != IID_ISequentialStream && riid != IID_IUnknown)
			return E_NOINTERFACE;
		
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	HRESULT Init(const void * pSrcData, const ULONG ulBufSize, const ULONG ulReadSize, BOOL fFree = FALSE) 
	{
		// Must have a source 
		if (NULL == pSrcData)
			return E_INVALIDARG;

		// Data length must be non-zero
		if (0 == ulBufSize)
			return E_INVALIDARG;

		m_ulBufSize=ulBufSize;
		m_ulReadSize=ulReadSize;
		m_pSrcData = (BYTE *)pSrcData;
		m_pReadPtr = m_pSrcData;
		m_ulBytesLeft=m_ulReadSize;
		m_ulReadPos=0;
		m_fWasRead = FALSE;
		m_fFree = fFree;

		return S_OK;
	}
			
	virtual HRESULT STDMETHODCALLTYPE Write(const void * pv, ULONG cb, ULONG * pcbWritten)
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Read(void * pv, ULONG cb, ULONG * pcbRead)
	{
		ULONG ulBytesWritten=0;
		ULONG ulCBToWrite=cb;
		ULONG ulCBToCopy;
		BYTE * pvb = (BYTE *)pv;

		m_fWasRead = TRUE;

		if (NULL == m_pSrcData)
			return E_FAIL;

		if (NULL == pv)
			return STG_E_INVALIDPOINTER;

		while (ulBytesWritten < ulCBToWrite && m_ulBytesLeft)
		{
			// Make sure we don't write more than our max read size or the size they asked for
			ulCBToCopy=min(m_ulBytesLeft, cb);

			// Make sure we don't read past the end of the internal buffer
			ulCBToCopy=min(m_ulBufSize-m_ulReadPos, ulCBToCopy);

			memcpy(pvb, m_pReadPtr+m_ulReadPos, ulCBToCopy);
			pvb+=ulCBToCopy;
			ulBytesWritten+=ulCBToCopy;
			m_ulBytesLeft-=ulCBToCopy;
			cb-=ulCBToCopy;

			// Wrap reads around the src buffer
			m_ulReadPos+=ulCBToCopy;
			if (m_ulReadPos >= m_ulBufSize)
				m_ulReadPos = 0;

		}

		if (pcbRead)
			*pcbRead=ulBytesWritten;

		return S_OK;
	}

	BOOL WasRead(void)
	{
		return m_fWasRead;
	}

	LPBYTE GetSrcBuffer(void)
	{
		return m_pSrcData;
	}

	ULONG GetSrcBufferSize(void)
	{
		return m_ulBufSize;
	}

	ULONG GetReadSize(void)
	{
		return m_ulReadSize;
	}

	void Rewind(void)
	{
		m_pReadPtr = m_pSrcData;
		m_ulBytesLeft=m_ulReadSize;
		m_ulReadPos=0;
	}
};

//--------------------------------------------------------------------
//	Utility class to return provider specific syntax for procedures
//
class CSyntax {

private:
	WCHAR * m_pwszProviderName;
	WCHAR * m_pwszDBMSName;
	WCHAR * m_pwszProcName;
	enum PROVIDER_ENUM m_eProvider;			// The provider
	enum DBMS_ENUM m_eDBMS;					// The DBMS
	enum DIALECT_ENUM m_eDialect;			// The dialect
	const ProviderList * m_pProviderList;	// List of providers
	const DBMSList * m_pDBMSList;			// List of DBMS's
	ULONG m_cDialectTokens;			// Total dialect tokens in list
	const DialectTokens * m_pDialectTokens;	// List of dialect tokens
	const Dialects * m_pDialectList;		// List of dialects given provider and DBMS
	CTable * m_pTable;						// Table to use to build column name, parm name lists from
	DBBINDING * m_pDBBINDING;				// User's bindings
	DBCOUNTITEM m_cParams;					// Count of parameters
	ParamStruct * m_pColmap;				// Mapping of parameters to columns
	ULONG m_iCurrentParam;					// Current parameter to build syntax for
	DBCOUNTITEM m_iCurrentRow;				// Current table row to build syntax for
	BOOL m_fColCounts;						// Whether column count and mappings are needed for this syntax element
	BOOL m_fHasReturnParam;					// Whether procedure has a return parameter
	ULONG m_ulMaxParamName;					// Maximum length of a parameter for boundary testing.
	ULONG m_ulCreateFlags;					// Flags to control parameter creation.
	enum TOKEN_ENUM m_eProcType;

	// Given the provider name find the provider in our known provider list
	void SetProvider(void)
	{	
		for (ULONG idx=0; idx < UNKNOWN_PROVIDER; idx++)
		{
			if (!wcscmp(m_pwszProviderName, m_pProviderList[idx].wszProviderName))
			{
				m_eProvider=m_pProviderList[idx].eProvider;
				break;
			}
		}

	}

	// Given the DBMS name find the DBMS in our known DBMS list
	void SetDBMS(void)
	{
		for (ULONG idx=0; idx < UNKNOWN_DBMS; idx++)
		{
			if (!wcscmp(m_pwszDBMSName, m_pDBMSList[idx].wszDBMSName))
			{
				m_eDBMS=m_pDBMSList[idx].eDBMS;
				break;
			}
		}
	}

	// Given the Provider and DBMS set the dialect used
	void SetDialect(void)
	{
		for (ULONG idx=0;
			m_pDialectList[idx].eProvider != UNKNOWN_PROVIDER || m_pDialectList[idx].eDBMS != UNKNOWN_DBMS;
			idx++)
		{
			if ((m_pDialectList[idx].eProvider == m_eProvider && m_pDialectList[idx].eDBMS == m_eDBMS) ||
				(m_pDialectList[idx].eProvider == UNKNOWN_PROVIDER && m_pDialectList[idx].eDBMS == UNKNOWN_DBMS))
			{
				// We found the provider/dbms combo or they're unknown, set the dialect
				m_eDialect = m_pDialectList[idx].eDialect;
				break;
			}
		}
	}

	void SetCurrentParam(ULONG iParam)
	{
		m_iCurrentParam = iParam;
	}

	// Creates a parameter definition string for a given parameter
	WCHAR * MakeParamDef(ULONG iParam, BOOL fDefaultVal)
	{
		CCol TempCol;
		enum TOKEN_ENUM eParamDef=T_PARM_DEF_IN;
		WCHAR * pwszParamDef = NULL;

		// Get the information about the column
		if (FAILED(m_pTable->GetColInfo(m_pColmap[iParam].ulColIndex, TempCol)))
			goto CLEANUP;

		if ((m_pColmap[iParam].eParamIO & DBPARAMIO_INPUT) &&
			(m_pColmap[iParam].eParamIO & DBPARAMIO_OUTPUT))
			eParamDef = T_PARM_DEF_INOUT;			
		else if (m_pColmap[iParam].eParamIO & DBPARAMIO_INPUT)
			eParamDef = T_PARM_DEF_IN;			
		else if (m_pColmap[iParam].eParamIO & DBPARAMIO_OUTPUT)
			eParamDef = T_PARM_DEF_OUT;			
		else
			goto CLEANUP;

		// Tell the syntax builder what parameter we're working with
		SetCurrentParam(iParam);

		// Retrieve parameter definition format
		pwszParamDef = GetSyntax(eParamDef);

CLEANUP:

		return pwszParamDef;

	}

	// Builds special token strings based on table/param info
	WCHAR * BuildTokenString(enum TOKEN_ENUM eToken, DBORDINAL * pcColsAdded, DB_LORDINAL ** prgColsAdded)
	{
		CCol TempCol;
		ULONG iCol, iStartCol;
		DBORDINAL cCols;
		WCHAR * pwszStr=NULL;
		WCHAR * pwszName=NULL;
		WCHAR * pwszParamName = NULL;
		WCHAR * pwszParamNameFmt = NULL;
		WCHAR * pwszParamDef = NULL;
		WCHAR * pwszColDef = NULL;
		WCHAR * pwszSize = NULL;
		WCHAR * pwszParamMarker = NULL;
		WCHAR * pwszStrip = NULL;
		ULONG iParam=0;
		BOOL fError=TRUE;
		DBORDINAL cColsAdded = 0;  
		DB_LORDINAL * pColsAdded = NULL;
		const ULONG ulColNameMax = m_ulMaxParamName;

		cCols = m_pTable->CountColumnsOnTable();

		TEST_ALLOC(DB_LORDINAL, pColsAdded, 0, (size_t)(cCols * sizeof(DB_LORDINAL)));

		// We know how to build certain strings from the table and/or col map
		switch(eToken)
		{
			case T_ORDER_COL:
				// Find the numeric col
				if (SUCCEEDED(m_pTable->GetOrderByCol(&TempCol)))
					pwszStr = wcsDuplicate(TempCol.GetColName());
				break;
			case T_FIRST_PARM:
				// We MUST have information about the number of params and 
				// we can't make a name for a non-existent param
				if (!m_cParams || !m_pColmap || m_iCurrentParam+1 > m_cParams)
					goto CLEANUP;

				pwszStr = wcsDuplicate(m_pColmap[0].pwszParamName);
				break;
			case T_PARM_NAME:
				// We MUST have information about the number of params and 
				// we can't make a name for a non-existent param
				if (!m_cParams || !m_pColmap || m_iCurrentParam+1 > m_cParams)
					goto CLEANUP;

				pwszStr = wcsDuplicate(m_pColmap[m_iCurrentParam].pwszParamName);
				break;

			case T_RET_TYPE:
				// We MUST have information about the number of params and 
				// we can't make a name for a non-existent param
				if (!m_cParams || !m_pColmap)
					goto CLEANUP;

				pwszStr = wcsDuplicate(m_pColmap[0].wszDataSourceType);
				break;
				
			case T_RET_DEF:
				// We MUST have information about the mapping of the parameter to column
				// so we can look up the proper create params based on the column.
				if (!m_cParams || !m_pColmap)
					goto CLEANUP;

				// Return param is always the first parameter
				pwszStr = MakeParamDef(0, FALSE);
				break;

			case T_RET_VAL:
					// Return parameter is always the first one
					m_iCurrentParam = 0;
					// Fall through
			case T_DATA_VAL:
				{
					WCHAR * pwszPrefix = NULL;
					WCHAR * pwszSuffix = NULL;
					WCHAR wszData[DATA_SIZE] = L"";
					size_t ulLiteral = 0;
					HRESULT hr;


					// We always just return a literal that matches the data type requested
					// for the return value.  There must be a column of that type in the table
					// for output value verification.  

					// We MUST have information about the number of params and 
					// we can't make a literal for a non-existent param
					if (!m_cParams || !m_pColmap || m_iCurrentParam+1 > m_cParams || !m_iCurrentRow)
						goto CLEANUP;

					// Get the information about the associated column.
					if (FAILED(m_pTable->GetColInfo(m_pColmap[m_iCurrentParam].ulColIndex, TempCol)))
						goto CLEANUP;

					// Get the literal prefix
					if (pwszPrefix = TempCol.GetPrefix())
						ulLiteral+=wcslen(pwszPrefix);

					// Get the literal suffix
					if (pwszSuffix = TempCol.GetSuffix())
						ulLiteral+=wcslen(pwszSuffix);

					// Create a wchar data value
					hr=m_pTable->MakeData(wszData, m_iCurrentRow, m_pColmap[m_iCurrentParam].ulColIndex, PRIMARY, TRUE);

					// If makedata failed we're hosed
					if (FAILED(hr))
						goto CLEANUP;

					// If the data is supposed to be NULL, then that's the literal
					if (S_FALSE == hr)
						pwszStr = wcsDuplicate(L"NULL");
					else
					{
						//Special case Data Types...
						switch(TempCol.GetProviderType()) 
						{
							case DBTYPE_BOOL:
							{
								//MakeData may return "0","1" or "False","True" (if using INI File)
								//Out INI File has it in the format from DBTYPE_BOOL -> DBTYPE_WSTR
								//Which should always be in "False","True" according to the spec
								//But since we maybe formulating Literals, they must be "0","1" for Boolean literals...
								if(wcscmp(wszData, L"True")==0)
									wcscpy(wszData, L"1");
								else if(wcscmp(wszData, L"False")==0)
									wcscpy(wszData, L"0");
								break;
							}
						}

						// Need to add one more for null term
						ulLiteral+=wcslen(wszData)+1;

						// Allocate space for the literal from the combined size of each
						TEST_ALLOC(WCHAR, pwszStr, 0, ulLiteral*sizeof(WCHAR));

						pwszStr[0] = L'\0';

						// Copy the pieces into the literal
						if (pwszPrefix)
							wcscat(pwszStr, pwszPrefix);
						wcscat(pwszStr, wszData);
						if (pwszSuffix)
							wcscat(pwszStr, pwszSuffix);
					}
				}
				break;

			case T_PARM_SIZE:
				// We MUST have information about the number of params and 
				// we can't make a name for a non-existent param
				if (!m_cParams || !m_pColmap || m_iCurrentParam+1 > m_cParams)
					goto CLEANUP;

				// Get the information about the column
				if (FAILED(m_pTable->GetColInfo(m_pColmap[m_iCurrentParam].ulColIndex, TempCol)))
					goto CLEANUP;

				TempCol.CreateColDef(&pwszColDef);

				// Sql Server specific: If the col def contains "identity", then strip it off
				if (pwszColDef && (pwszStrip = wcsstr(pwszColDef, L"identity")))
					*pwszStrip = L'\0';

				// If we have a size specification return it
				if (pwszColDef && (pwszSize = wcsstr(pwszColDef, L"(")))
					pwszStr = wcsDuplicate(pwszSize);
				else
					// Otherwise return an empty string
					pwszStr = wcsDuplicate(L"");

				PROVIDER_FREE(pwszColDef);

				break;
			case T_PARM_TYPE:
				// We MUST have information about the number of params and 
				// we can't make a name for a non-existent param
				if (!m_cParams || !m_pColmap || m_iCurrentParam+1 > m_cParams)
					goto CLEANUP;

				pwszStr = wcsDuplicate(m_pColmap[m_iCurrentParam].wszDataSourceType);
				break;
				
			case T_OFFSET_NO_LONG_PARM_MARKER_EQ_LIST:
				if (!(pwszParamMarker = GetSyntax(T_PARM_MARKER)))
					goto CLEANUP;

				// Fall through
			case T_OFFSET_NO_LONG_PARM_EQ_LIST:
				// In this case the number of params in the limit clause should match
				// the number in the output list.

				// We MUST have information about the number of params
				if (!m_cParams || !m_pColmap)
					goto CLEANUP;

				// We assume space for column name=param name+comma+space
				// will never be greater than 2 + 2*columnsize.
				TEST_ALLOC(WCHAR, pwszStr, 0, (size_t)m_cParams * (3 + 2*ulColNameMax) * sizeof(WCHAR));

				// Go through each parameter and add to our lists
				for (iParam=0; iParam < m_cParams; iParam++)
				{
					// We'll get the column name from the next parameter, not this one
					ULONG iColParam = iParam+1;

					// Wrap back to first param
					if (iParam == m_cParams-1)
						iColParam = 0;

					// Get the information about the column
					if (FAILED(m_pTable->GetColInfo(m_pColmap[iColParam].ulColIndex, TempCol)))
						goto CLEANUP;

					if (pwszStr[0])
						wcscat(pwszStr, L",");
					
					if (eToken == T_OFFSET_NO_LONG_PARM_EQ_LIST)
						wcscat(pwszStr, m_pColmap[iParam].pwszParamName);
					else
						wcscat(pwszStr, pwszParamMarker);
					wcscat(pwszStr, L"=");
					wcscat(pwszStr, TempCol.GetColName());

				}
				PROVIDER_FREE(pwszParamMarker);

				break;
			case T_PARM_MARKER_LIST:
			case T_PARM_MARKER_LIST_DFLT:
			case T_PARM_MARKER_LIST_RET:
				// We MUST have information about the number of params
				if (!m_cParams || !m_pColmap)
					goto CLEANUP;
			
				if (!(pwszParamMarker = GetSyntax(T_PARM_MARKER)))
					goto CLEANUP;

				// Allocate space for L"?, " for each param (marker list)
				TEST_ALLOC(WCHAR, pwszStr, 0, (size_t)m_cParams * (wcslen(pwszParamMarker) + sizeof(L",")));

				iParam = 0;

				// Skip creation of the first marker character for these types of marker lists
				if (eToken == T_PARM_MARKER_LIST_RET ||
					eToken == T_PARM_MARKER_LIST_DFLT)
					iParam++;
				
				for (; iParam < m_cParams; iParam++)
				{
					if (!((m_pColmap[iParam].eCompareOp == CP_ISNULL ||
						m_pColmap[iParam].eCompareOp == CP_ISNOTNULL) &&
						!(m_pColmap[iParam].eParamIO & DBPARAMIO_OUTPUT)))
					{
						if (pwszStr[0])
							wcscat(pwszStr, L",");

						wcscat(pwszStr, pwszParamMarker);
					}
				}

				break;

			case T_NO_LONG_PARM_DEF_LIST:
			case T_UPDATABLE_PARM_DEF_LIST:
			case T_NO_LONG_PARM_DEF_LIST_DFLT:
				{
					BOOL fDefault = FALSE;  // We only create a default value for the first param
					DBCOUNTITEM cbMem	= m_cParams * (2 + 3*ulColNameMax) * sizeof(WCHAR);

					if (eToken == T_NO_LONG_PARM_DEF_LIST_DFLT)
						fDefault = TRUE;

					// parm1 inout type1(createparams)

					// We MUST have information about the mapping of the parameter to column
					// so we can look up the proper create params based on the column.
					if (!m_cParams || !m_pColmap)
						goto CLEANUP;
					
					// We assume space for column/param name+space+inputoutput+space+typenamemax+(p, s)
					// will never be greater than 2 + 3*columnsize.
					TEST_ALLOC(WCHAR, pwszStr, 0, (size_t)cbMem);

					// Go through each parameter and add to our lists
					for (iParam=(m_fHasReturnParam ? 1 : 0); iParam < m_cParams; iParam++)
					{
						BOOL fAdd = TRUE;

						// If the parameter is using the IS NULL syntax it's not really
						// a parameter, so leave it off
						if (m_pColmap[iParam].eCompareOp == CP_ISNULL ||
							m_pColmap[iParam].eCompareOp == CP_ISNOTNULL)
							fAdd = FALSE;

						// But output only params need to be left in
						if (m_pColmap[iParam].eParamIO & DBPARAMIO_OUTPUT)
							fAdd = TRUE;

						// And we need to add it anyway if it's an insert sproc
						if (m_eProcType == T_EXEC_PROC_INSERT_INPUT)
							fAdd = TRUE;

						// If the parameter is using the IS NULL syntax it's not really
						// a parameter, so leave it off
/*
						if (!((m_pColmap[iParam].eCompareOp == CP_ISNULL ||
							m_pColmap[iParam].eCompareOp == CP_ISNOTNULL) &&
							!(m_pColmap[iParam].eParamIO & DBPARAMIO_OUTPUT)))
*/
						if (fAdd)
						{
							if (!(pwszParamDef = MakeParamDef(iParam, fDefault)))
								goto CLEANUP;

							// Need space for pwszParamDef, ", ", and null terminator.
							if ((wcslen(pwszStr)+wcslen(pwszParamDef)+3)*sizeof(WCHAR) > cbMem)
							{
								// Realloc twice as much as needed to allow fewer reallocations
								cbMem= (wcslen(pwszStr)+wcslen(pwszParamDef)+3)*sizeof(WCHAR)*2;
								SAFE_REALLOC(pwszStr, WCHAR, cbMem)
							}
							
							if (pwszStr[0])
 								wcscat(pwszStr, L", ");
							
							wcscat(pwszStr, pwszParamDef);

							PROVIDER_FREE(pwszParamDef);

							fDefault = FALSE;
						}
					}
				}

				break;

			case T_UNIQUE_SEARCHABLE_COL_EQ_PARM_MARKER:
			case T_SEARCHABLE_COL_EQ_PARM_MARKER:
			case T_NO_LONG_PARM_MARKER_EQ_LIST:
				if (!(pwszParamMarker = GetSyntax(T_PARM_MARKER)))
					goto CLEANUP;

				// Fall through

			case T_SEARCHABLE_COL_EQ_PARM:
			case T_OFFSET_SEARCHABLE_COL_EQ_PARM:
			case T_UNIQUE_SEARCHABLE_COL_EQ_PARM:
			case T_NO_LONG_PARM_LIST:
			case T_NO_LONG_PARM_EQ_LIST:
			case T_NULL_PARM_EQ_LIST:
			case T_FIRST_COL_EQ_PARM:
			case T_LOOKUP_PARM_EQ_LIST:
			case T_UPDATABLE_PARM_LIST:
				// Retrieve the syntax for parameter names
				if (!(pwszParamNameFmt = GetSyntax(T_PARM_NAME_FMT)))
					goto CLEANUP;

				// We need to know the param mapping
				ASSERT(m_cParams && m_pColmap);

				// Fall through
			case T_SEARCHABLE_COL_EQ:
			case T_UPDATABLE_COL_LIST:
			case T_NO_LONG_COL_LIST:
			case T_NO_LONG_COL_ORDER_LIST:
			case T_OFFSET_NO_LONG_COL_LIST:
			case T_COL_LIST:
			case T_SECOND_COL:
			case T_NULL_COL_LIST:
				// L"colname, colname, ..." or L"parmname, parmname..." or L"parmname=colname
				TEST_ALLOC(WCHAR, pwszStr, 0, (size_t)(cCols * (3 + 2 * ulColNameMax) * sizeof(WCHAR)));

				// For creating an order list for the order by clause we're stuck with a limit
				// on the number of columns.  For some providers this is 16, others may have
				// a different limit.  We'll use the min we are aware of.
				if (eToken == T_NO_LONG_COL_ORDER_LIST && cCols > MAX_ORDER_LIST)
					cCols = MAX_ORDER_LIST;

				iStartCol = 1;

				// For lookup syntax only one column is desired, the second one
				if (eToken == T_SECOND_COL)
				{
					iStartCol = 2;
					cCols = 2;
				}

				// For lookup syntax only one param is desired, the first one
				if (eToken == T_FIRST_COL_EQ_PARM || eToken == T_LOOKUP_PARM_EQ_LIST)
					cCols = 1;

				// Go through each column in the table and add to our lists
				for (iCol=iStartCol; iCol<=cCols; iCol++)
				{
					ULONG iNewCol = iCol;
					WCHAR wchFirst;

					// Adjust retrieved column if making offset col list
					if (eToken == T_OFFSET_NO_LONG_COL_LIST)
					{
						iNewCol++;
						if (iNewCol > cCols)
							iNewCol = 1;
					}

					// Get the information about the column
					if (FAILED(m_pTable->GetColInfo(iNewCol, TempCol)))
						goto CLEANUP;

					// If the column is wanted add to the list
					if (bAddColumn(eToken, TempCol))
					{
						ULONG fIsNull = FALSE;

						pColsAdded[cColsAdded++] = iNewCol;

						if (eToken == T_NULL_COL_LIST)
							pwszName = wcsDuplicate(L"NULL");
						else
							pwszName = wcsDuplicate(TempCol.GetColName());

						if (!pwszName)
							goto CLEANUP;

						// For most tokens we need to add a comma separator
						if (eToken != T_SEARCHABLE_COL_EQ_PARM && 
								eToken != T_OFFSET_SEARCHABLE_COL_EQ_PARM && 
								eToken != T_UNIQUE_SEARCHABLE_COL_EQ_PARM && 
								eToken != T_SEARCHABLE_COL_EQ_PARM_MARKER &&
								eToken != T_UNIQUE_SEARCHABLE_COL_EQ_PARM_MARKER &&
								eToken != T_FIRST_COL_EQ_PARM &&
								pwszStr[0])
							wcscat(pwszStr, L", ");

						if (eToken == T_NO_LONG_PARM_LIST ||
							eToken == T_SEARCHABLE_COL_EQ_PARM ||
							eToken == T_OFFSET_SEARCHABLE_COL_EQ_PARM ||
							eToken == T_UNIQUE_SEARCHABLE_COL_EQ_PARM ||
							eToken == T_SEARCHABLE_COL_EQ_PARM_MARKER ||
							eToken == T_UNIQUE_SEARCHABLE_COL_EQ_PARM_MARKER ||
							eToken == T_NO_LONG_PARM_EQ_LIST ||
							eToken == T_NULL_PARM_EQ_LIST ||
							eToken == T_NO_LONG_PARM_MARKER_EQ_LIST ||
							eToken == T_FIRST_COL_EQ_PARM ||
							eToken == T_LOOKUP_PARM_EQ_LIST ||
							eToken == T_UPDATABLE_PARM_LIST)
						{
							// Build param name
							wchFirst = L'P';

							if (eToken == T_SEARCHABLE_COL_EQ_PARM || 
								eToken == T_UNIQUE_SEARCHABLE_COL_EQ_PARM ||
								eToken == T_OFFSET_SEARCHABLE_COL_EQ_PARM ||
								eToken == T_SEARCHABLE_COL_EQ_PARM_MARKER ||
								eToken == T_UNIQUE_SEARCHABLE_COL_EQ_PARM_MARKER ||
								eToken == T_FIRST_COL_EQ_PARM)
							{
								// When building the offset list for in/out params we need the
								// param name in the limit clause to match the one in the output
								// list.
								if (eToken != T_OFFSET_SEARCHABLE_COL_EQ_PARM && 
									eToken != T_UNIQUE_SEARCHABLE_COL_EQ_PARM &&
									eToken != T_UNIQUE_SEARCHABLE_COL_EQ_PARM_MARKER &&
									eToken != T_FIRST_COL_EQ_PARM)
									// Change the param name to start with 'L' for the limit clause
									wchFirst = L'L';

								if (pwszStr[0])
									wcscat(pwszStr, L" and ");

								// Tack the column name onto the string
								wcscat(pwszStr, TempCol.GetColName());

								if (!(pwszParamName = MakeParamName(TempCol, wchFirst)))
									goto CLEANUP;

								for (iParam = 0; iParam < m_cParams; iParam++)
									if (!wcscmp(m_pColmap[iParam].pwszParamName, pwszParamName))
										break;

								// We should always find a matching parameter name.
								ASSERT(iParam < m_cParams);
								
								// Now put in the operator based on the param info
								switch (m_pColmap[iParam].eCompareOp)
								{
									case CP_EQ:
										wcscat(pwszStr, L"=");
										break;
									case CP_GT:
										wcscat(pwszStr, L">");
										break;
									case CP_LT:
										wcscat(pwszStr, L"<");
										break;
									case CP_ISNULL:
										wcscat(pwszStr, L" IS NULL");
										fIsNull = TRUE;
										break;
									case CP_ISNOTNULL:
										wcscat(pwszStr, L" IS NOT NULL");
										fIsNull = TRUE;
										break;
									default:
										ASSERT(!L"Unknown operator found.");
								}
							}

							if (!pwszParamName && !(pwszParamName = MakeParamName(TempCol, wchFirst)))
								goto CLEANUP;
							PROVIDER_FREE(pwszName);
							pwszName = pwszParamName;
							pwszParamName = NULL;

							if (eToken == T_SEARCHABLE_COL_EQ_PARM_MARKER ||
								eToken == T_UNIQUE_SEARCHABLE_COL_EQ_PARM_MARKER ||
								eToken == T_NO_LONG_PARM_MARKER_EQ_LIST)
								wcscpy(pwszName, pwszParamMarker);

						}

						// We always want to tack on the parameter name unless
						// using IS NULL or IS NOT NULL syntax.
						if (!fIsNull)
							wcscat(pwszStr, pwszName);

						if (eToken == T_NO_LONG_PARM_EQ_LIST ||
							eToken == T_NULL_PARM_EQ_LIST ||	
							eToken == T_NO_LONG_PARM_MARKER_EQ_LIST ||
							eToken == T_LOOKUP_PARM_EQ_LIST)
						{
							wcscat(pwszStr, L"=");
							if (eToken == T_NULL_PARM_EQ_LIST)
								wcscat(pwszStr, L"NULL");
							else
								wcscat(pwszStr, TempCol.GetColName());
						}

						PROVIDER_FREE(pwszName);
					}
				}

				break;
			case T_TABLE_NAME:
				// We just duplicate the table name
				if (!(pwszStr = wcsDuplicate(m_pTable->GetTableName())))
					goto CLEANUP;
				break;
			case T_PROC_NAME:
				if (!m_pwszProcName)
				{
					// We make a unique proc name by calling MakeObjectName
					if (!(m_pwszProcName = MakeObjectName(L"ICmdProc", wcslen(m_pTable->GetTableName())+sizeof(WCHAR))))
						goto CLEANUP;
				}

				// Return a copy of the name
				pwszStr = wcsDuplicate(m_pwszProcName);
				break;
			case T_SEARCHABLE_PARM_MARKER_LIST:
			case T_UPDATABLE_PARM_MARKER_LIST:
			case T_NO_LONG_PARM_MARKER_LIST:
				// L"?,?,?"

				if (!(pwszParamMarker = GetSyntax(T_PARM_MARKER)))
					goto CLEANUP;

				TEST_ALLOC(WCHAR, pwszStr, 0, (size_t)(cCols * (ulColNameMax+wcslen(pwszParamMarker)+sizeof(L","))));

				// Go through each column in the table and add to our lists
				for (iCol=1; iCol<=cCols; iCol++)
				{
					// Get the information about the column
					if (FAILED(m_pTable->GetColInfo(iCol, TempCol)))
						goto CLEANUP;

					// If the column is wanted add to the list
					if (bAddColumn(eToken, TempCol))
					{
						if (pwszStr[0])
							wcscat(pwszStr, L", ");

						wcscat(pwszStr, pwszParamMarker);

						PROVIDER_FREE(pwszName);
					}
				}

				break;
			default:
				return NULL;

		}

		fError=FALSE;

CLEANUP:

		if (fError)
			PROVIDER_FREE(pwszStr);

		PROVIDER_FREE(pwszParamMarker);
		PROVIDER_FREE(pwszName)
		PROVIDER_FREE(pwszParamName);
		PROVIDER_FREE(pwszParamNameFmt);

		if (pcColsAdded)
			*pcColsAdded = cColsAdded;
		if (prgColsAdded)
			*prgColsAdded = pColsAdded;
		else
			PROVIDER_FREE(pColsAdded);

		return pwszStr;

	}


public:

	// Constructor
	CSyntax(void)
	{
		m_pwszProviderName = NULL;
		m_pwszDBMSName = NULL;
		m_cDialectTokens=0;
		m_pDialectTokens=NULL;
		m_pProviderList=NULL;
		m_pDBMSList=NULL;
		m_pDialectList = NULL;
		m_pTable = NULL;
		m_eProvider=UNKNOWN_PROVIDER;
		m_eDBMS=UNKNOWN_DBMS;
		m_eDialect=ORA_SQL;
		m_pDBBINDING = NULL;
		m_cParams=0;
		m_pColmap = NULL;
		m_pwszProcName = NULL;
		m_iCurrentParam = 0;
		m_iCurrentRow = 0;
		m_fColCounts = FALSE;
		m_fHasReturnParam = FALSE;
		m_ulMaxParamName = SP_MAX_PARAMNAME_LENGTH;
		m_ulCreateFlags = 0;
		m_eProcType = T_EXEC_PROC_SELECT_OUT;
	}

	void SetProcType(enum TOKEN_ENUM eProcType)
	{
		m_eProcType = eProcType;
	}

	BOOL Init(WCHAR * pwszProviderName,
			WCHAR * pwszDBMSName,
			ULONG cDialectTokens,
			const DialectTokens * pDialectTokens,
			const ProviderList * pProviderList,
			const DBMSList * pDBMSList,
			const Dialects * pDialectList,
			CTable * pTable,
			ULONG ulMaxParamName)
	{
		m_pwszProviderName = pwszProviderName;
		m_pwszDBMSName = pwszDBMSName;
		m_cDialectTokens=cDialectTokens;
		m_pDialectTokens=pDialectTokens;
		m_pProviderList=pProviderList;
		m_pDBMSList=pDBMSList;
		m_pDialectList = pDialectList;
		m_eDialect=ORA_SQL;
		m_pTable = pTable;
		m_ulMaxParamName = ulMaxParamName;

		// Validate member vars.  We need all these.
		// m_pTable might be NULL if we never need to build specialized strings
		// from the table
		if (!m_pwszProviderName	||
			!m_pwszDBMSName		||
			!m_cDialectTokens	||
			!m_pDialectTokens	||
			!m_pProviderList	||
			!m_pDBMSList		||
			!m_pDialectList)		
			return FALSE;

		SetProvider();
		SetDBMS();
		SetDialect();

		return TRUE;
	}

	BOOL bHasColCounts(enum TOKEN_ENUM eToken, ULONG * pcColCounts, ULONG ** prgColCounts)
	{
		ASSERT(pcColCounts);
		ASSERT(prgColCounts);

		*pcColCounts = 0;
		*prgColCounts = NULL;

		switch (eToken)
		{
			case T_SELECT_IN:
				// Only one token returns col counts
				TEST_ALLOC(ULONG, *prgColCounts, 0, sizeof(ULONG));
				(*prgColCounts)[0] = 2;	// Token 3 needs to return col counts
				*pcColCounts = 1;
				return TRUE;
				break;
			default:
				return FALSE;
		}

CLEANUP:
		
		return FALSE;
	}

	BOOL bNeedsLimitClause(enum TOKEN_ENUM eToken)
	{
		switch(eToken)
		{
			case T_EXEC_PROC_SELECT_IN:
			case T_EXEC_PROC_SELECT_INOUT:
			case T_EXEC_PROC_SELECT_INOUT_RET:
			case T_EXEC_PROC_SELECT_OUT:
			case T_EXEC_PROC_SELECT_OUT_DFLT:
			case T_EXEC_PROC_SELECT_OUT_NULL:
			case T_EXEC_PROC_SELECT_OUT_RET:
			case T_EXEC_PROC_UPDATE_INPUT:
			case T_EXEC_PROC_UPDATE_INPUT_RET:
				return TRUE;
			case T_EXEC_PROC_INSERT_INPUT:
			case T_EXEC_PROC_INSERT_INPUT_RET:
			default:
				return FALSE;
		}
	}

	BOOL bAddColumn(enum TOKEN_ENUM eToken, CCol ColInfo)
	{
		// We want certain columns from the table based on the proc type
		switch(eToken)
		{
			case T_SEARCHABLE_COL_EQ:
			case T_SEARCHABLE_COL_EQ_PARM:
			case T_SEARCHABLE_PARM_MARKER_LIST:
			case T_SEARCHABLE_COL_EQ_PARM_MARKER:
			case T_UNIQUE_SEARCHABLE_COL_EQ_PARM_MARKER:
			case T_EXEC_PROC_SELECT_INOUT:
			case T_EXEC_PROC_SELECT_INOUT_RET:
			case T_EXEC_PROC_SELECT_OUT:
			case T_EXEC_PROC_SELECT_OUT_DFLT:
			case T_EXEC_PROC_SELECT_OUT_NULL:
			case T_EXEC_PROC_SELECT_OUT_RET:
			case T_NO_LONG_PARM_EQ_LIST:
			case T_NULL_PARM_EQ_LIST:
			case T_NO_LONG_PARM_MARKER_EQ_LIST:
			case T_OFFSET_NO_LONG_PARM_EQ_LIST:
			case T_OFFSET_NO_LONG_COL_LIST:
			case T_OFFSET_NO_LONG_PARM_MARKER_EQ_LIST:
			case T_OFFSET_SEARCHABLE_COL_EQ_PARM:
			case T_UNIQUE_SEARCHABLE_COL_EQ_PARM:
			case T_NO_LONG_COL_LIST:
			case T_NULL_COL_LIST:
			case T_NO_LONG_COL_ORDER_LIST:
				return (ColInfo.GetSearchable() &&		// Must be a searchable column for use in where
						!ColInfo.GetIsLong() &&			// Long columns require 'LIKE', which we don't support yet
						ColInfo.GetUpdateable() &&		// If we didn't insert the value we don't know it
						!ColInfo.GetAutoInc());			// Even if updatable we never insert autoincs
				break;
			case T_UPDATABLE_COL_LIST:
			case T_UPDATABLE_PARM_MARKER_LIST:
			case T_EXEC_PROC_UPDATE_INPUT:
			case T_EXEC_PROC_UPDATE_INPUT_RET:
			case T_EXEC_PROC_INSERT_INPUT:
			case T_EXEC_PROC_INSERT_INPUT_RET:
			case T_UPDATABLE_PARM_LIST:
				return ColInfo.GetUpdateable();
				break;
			case T_NO_LONG_PARM_LIST:
			case T_NO_LONG_PARM_MARKER_LIST:
				return !ColInfo.GetIsLong();
				break;
			case T_COL_LIST:
			case T_SECOND_COL:
			case T_FIRST_COL_EQ_PARM:
			case T_LOOKUP_PARM_EQ_LIST:
				return TRUE;
			default:
				// The delete and select in proc will hit the default and not add any columns except
				// in the limit clause.
				return FALSE;
		}
	}

	void SetProcName(WCHAR * pwszProcName) 
	{
		m_pwszProcName = pwszProcName;
	}

	void SetCreateFlags(ULONG ulCreateFlags) 
	{
		m_ulCreateFlags = ulCreateFlags;
	}

	void SetCurrentRow(DBCOUNTITEM iRow)
	{
		m_iCurrentRow = iRow;
	}

	void SetReturnParam(BOOL fRetParam)
	{
		m_fHasReturnParam = fRetParam;
	}

	void SetMaxParamName(ULONG ulMaxParamName)
	{
		m_ulMaxParamName = ulMaxParamName;
	}

	// Build the string for a given syntax
	WCHAR * GetSyntax(enum TOKEN_ENUM eToken, DBORDINAL * pcColsWanted = NULL, DB_LORDINAL ** prgColsWanted = NULL)
	{
		ULONG iSyntax=0;
		ULONG iANSISyntax=0;
		ULONG iToken=0;
		ULONG cTokens=0;
		BOOL fFoundANSISyntax = FALSE;
		BOOL fFoundProviderSyntax = FALSE;
		WCHAR ** ppwszTokenStrings = NULL;
		WCHAR * pwszSyntax = NULL;
		ULONG crgColCounts = 0;
		ULONG * prgColCounts = NULL;

		// Find the syntax for this token
		for (iToken = 0; iToken < m_cDialectTokens; iToken++)
		{
			// Look for provider syntax
			if (m_pDialectTokens[iToken].eToken == eToken &&
				m_pDialectTokens[iToken].eDialect == m_eDialect)
			{
				// We found the provider's syntax
				fFoundProviderSyntax = TRUE;
				iSyntax = iToken;
				break;
			}
			// Look for default syntax (Oracle, since Oracle seems more close
			// to ANSI.
			if (m_pDialectTokens[iToken].eToken == eToken &&
				m_pDialectTokens[iToken].eDialect == ORA_SQL)
			{
				iANSISyntax=iToken;
				fFoundANSISyntax = TRUE;
			}
		}

		// Make sure we actually found the provider syntax otherwise use ANSI
		if (!fFoundProviderSyntax)
		{
			if (fFoundANSISyntax)
				iSyntax = iANSISyntax;
			else
			{
				// We didn't find the provider syntax or the ansi syntax
				odtLog << L"Couldn't find proper syntax to use for this token.\n";
				goto CLEANUP;
			}
		}

		cTokens = m_pDialectTokens[iSyntax].cTokens;

		// Find out if this token is expected to receive column count information
		// prgColCounts is an array of token indexes that should return count info.
		if (bHasColCounts(eToken, &crgColCounts, &prgColCounts))
			m_fColCounts=TRUE;

		// We need new code to sum the col counts and reallocate the array
		// if this is > 1.
		ASSERT(crgColCounts < 2);

		// Check to see if it's a root token and return the string
		if (m_pDialectTokens[iSyntax].rgeTokens[0] == T_ROOT)
		{
			// It claims to be a root token, but if the token count isn't zero it's bogus
			if (cTokens)
			{
				cTokens = 0;
				goto CLEANUP;
			}

			// If it's a special root token that's one we think we know how to build then the string
			// pointer is NULL;
			if (!m_pDialectTokens[iSyntax].pwszTokenString)
			{
				if (!(pwszSyntax = BuildTokenString(eToken, pcColsWanted, prgColsWanted))) 
					goto CLEANUP;
			}
			// Otherwise just return the string
			else if (!(pwszSyntax = wcsDuplicate(m_pDialectTokens[iSyntax].pwszTokenString)))
				goto CLEANUP;
		}
		else
		{
			// Not a root token, we have to assemble the token from the pieces specified

			// Allocate space for a pointer to each piece of the string
			TEST_ALLOC(LPWSTR, ppwszTokenStrings, 0, cTokens * sizeof(LPWSTR));

			// Find out the size of each piece and get the pointer to each
			for (iToken = 0; iToken < cTokens; iToken++)
			{
				ULONG iColCount = 0;
				DBORDINAL cCols = 0;
				DB_LORDINAL * rgCols = NULL;
				DBORDINAL * pcCols = &cCols;
				DB_LORDINAL ** prgCols = &rgCols;

				// Get the syntax for this token
				if (!(ppwszTokenStrings[iToken]=GetSyntax(m_pDialectTokens[iSyntax].rgeTokens[iToken], pcCols, prgCols)))
					goto CLEANUP;

				// If this token needs to return col counts set up for it.
				// Currently only coded for 1 col count being returned so only the first one is
				if ((m_fColCounts && iToken == prgColCounts[iColCount]) ||
					(!m_fColCounts && pcColsWanted && !*pcColsWanted))
				{
					if (pcColsWanted && prgColsWanted)
					{
						*pcColsWanted = cCols;
						SAFE_FREE(*prgColsWanted);
						*prgColsWanted = rgCols;
					}
					iColCount++;
					if (iColCount = crgColCounts)
					{
						PROVIDER_FREE(prgColCounts);
						m_fColCounts = FALSE;
					}
				}
			}

			// Now assemble the combined string from the pieces.  First piece is the format.
			if (!FormatStringFromArray(&pwszSyntax, ppwszTokenStrings[0], cTokens-1, &ppwszTokenStrings[1]))
				goto CLEANUP;
		}

CLEANUP:

		// Free any temporary strings we allocated
		for (iToken = 0; iToken < cTokens; iToken++)
			PROVIDER_FREE(ppwszTokenStrings[iToken]);

		PROVIDER_FREE(ppwszTokenStrings);

		ASSERT(pwszSyntax);

		return pwszSyntax;
	}


	WCHAR * MakeParamName(CCol ColInfo, WCHAR wchFirst)
	{
		WCHAR * pwszParamName = NULL;
		WCHAR * pwszParamRoot = NULL;
		WCHAR * pwszNameFmt = NULL;
		ULONG ulMaxParamName = m_ulMaxParamName;

		if (!(pwszParamRoot = wcsDuplicate(ColInfo.GetColName())))
			goto CLEANUP;

		// Make the last character of the param name different so it doesn't match col name.
		// Use last char so it doesn't conflict with localization testing.
		pwszParamRoot[wcslen(pwszParamRoot)-1] = wchFirst;

		if (!(pwszNameFmt = GetSyntax(T_PARM_NAME_FMT)))
			goto CLEANUP;

		if (!FormatString(&pwszParamName, pwszNameFmt, 1, pwszParamRoot))
			goto CLEANUP;

		// If we're making long names, make name one larger than max
		if (m_ulCreateFlags & CREATE_LONG_NAMES)
			ulMaxParamName++;

		// If we're testing boundary on max name size, reallocate and fill name
		if (m_ulCreateFlags & CREATE_LONG_NAMES || m_ulCreateFlags & CREATE_MAX_NAMES)
		{
			SAFE_REALLOC(pwszParamName, WCHAR, ulMaxParamName+1);

			if (GetModInfo()->GetLocaleInfo())
			{
				// We have to create a consistent value for parameter name each time, so we must set the
				// Unicode Seed consistently.  Use column number.
				GetModInfo()->GetLocaleInfo()->SetUnicodeSeed((INT)ColInfo.GetColNum());
				if (!GetModInfo()->GetLocaleInfo()->MakeUnicodeIntlString(pwszParamName+wcslen(pwszParamName), 
					ulMaxParamName-(ULONG)wcslen(pwszParamName)+1))
					goto CLEANUP;
			}
			else
			{
				for (size_t ichar=wcslen(pwszParamName); ichar < ulMaxParamName; ichar++)
					pwszParamName[ichar] = L'A';
				pwszParamName[ulMaxParamName] = L'\0';
			}
		}

	CLEANUP:
		PROVIDER_FREE(pwszParamRoot);
		PROVIDER_FREE(pwszNameFmt);

		return pwszParamName;

	}

	inline BOOL	IsKnown(void) {return m_eProvider != UNKNOWN_PROVIDER && m_eDBMS != UNKNOWN_DBMS;}

	inline void SetBinding(DBBINDING * pBinding) {m_pDBBINDING = pBinding;}

	inline void SetColMap(DBCOUNTITEM cParams, ParamStruct * pColMap)
	{
		m_cParams = cParams;
		m_pColmap = pColMap;
	}

	inline void SetTable(CTable * pTable)
	{
		m_pTable = pTable;
	}

	inline enum DBMS_ENUM GetDBMSType(void) {return m_eDBMS;}

};


//--------------------------------------------------------------------
//	Utility class to allow printing a custom string when an error occurs
//
class CMyError {

public:

	//@cmember	Checks the value of fEqual increments error count if it's FALSE
	inline BOOL	Compare(
		BOOL fEqual, 
		wchar_t * strFile, 
		UDWORD udwLine,
		CError * m_pCError,
		WCHAR * wszMsg=NULL)
	{
		ASSERT(m_pCError);
		if (wszMsg && !fEqual)
			odtLog << wszMsg;
		return m_pCError->Compare(fEqual, strFile, udwLine);
	}
};

#define MYCOMPARE(obj1,obj2,msg) m_MyError.Compare((obj1)==(obj2), LONGSTRING(__FILE__), __LINE__, m_pError, msg)

class CIVerifyRow : public CSessionObject
{
private:
	IID m_iidExec;
	BOOL m_fLiteralSelect;

public:
	//@cmember CTOR
	CIVerifyRow(LPWSTR wszTestCaseName)
		:CSessionObject(wszTestCaseName)
	{
		m_iidExec = IID_IRowset;
		m_fLiteralSelect = FALSE;
	}

	//@cmember DTOR
	~CIVerifyRow(){	}

	//@cmember function to find whether a row exists in the table or not.
	BOOL FindRow (DBCOUNTITEM ulRowNum, CTable *pTable=NULL, ICommand * pICommand = NULL, IRowset ** ppIRowset=NULL, HROW ** pphRows=NULL,
		DBORDINAL * pcCols = NULL, DB_LORDINAL ** ppCols = NULL, BOOL fFetchRow = TRUE);

	LPBYTE GetUpdatableCols(DBCOUNTITEM ulRowNum, DBCOUNTITEM cBindings, DBBINDING * pBinding,
		DBLENGTH cbRowSize, ICommand * pICommand, CTable *pTable = NULL);

	void SetIID(REFIID riid) {m_iidExec = riid;}

	CMyError m_MyError;
};


//--------------------------------------------------------------------
// @mfunc FindRow
//	Checks to see whether a specified row exists in the table or not.
//  uses a parameterized query to (select) to verify.
//--------------------------------------------------------------------
BOOL
CIVerifyRow::FindRow(DBCOUNTITEM ulRowNum, CTable *pTable, ICommand * pICommand, IRowset ** ppIRowset, HROW ** pphRows,
	DBORDINAL * pcCols, DB_LORDINAL ** ppCols, BOOL fFetchRow)
{
	const ULONG	cRows = 1;					// Should be 1 because we are expecting 1 row back from select.
	HROW *		phRows = NULL;
	BOOL		fFound = FALSE;
	IRowset *	pIRowset = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	DB_LORDINAL *	rgSearchableCols = NULL;
	WCHAR *		pwszSqlStmt = NULL;
	ICommandText * pICommandText = NULL;
	DBORDINAL iCol, cCols = 0;
	DB_LORDINAL * pCols = NULL;
	IAccessor * pCmdIAccessor = NULL;			
	DBBINDING *	rgBindings = NULL;
	DBCOUNTITEM	cBindings = 0;
	DBLENGTH	cbRowSize = 0;
	DBPARAMS 	Param;
	DBPARAMS *	pParam = NULL;
	BYTE *		pData = NULL;
	HACCESSOR 	hExecAccessor=DB_NULL_HACCESSOR;
	DBPARAMBINDINFO * pParamBindInfo = NULL;
	ParamStruct * pParamStruct = NULL;
	DB_UPARAMS * prgParamOrdinals = NULL;
	ICommandWithParameters * pICmdWParams = NULL;

	// Init out params
	if (pcCols)
		*pcCols = 0;
	if (ppCols)
		*ppCols = NULL;

	if (ppIRowset)
		*ppIRowset = NULL;

	if (pphRows)
		*pphRows = NULL;

	SAFE_ALLOC(phRows, HROW, sizeof(HROW) * cRows);

	// if we are passed a table pointer set the table to that one.
	if (!pTable) 
		pTable = m_pTable;

	// Get a command object to do a sql query.
	if (!pICommand)
	{
		TEST_CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
			(IUnknown **)&pICommand), S_OK);
	}
	else
		pICommand->AddRef();

	TEST_COMPARE(VerifyInterface(pICommand, IID_ICommandText, COMMAND_INTERFACE, (IUnknown**)&pICommandText),TRUE);

	//	While it is better to use literals to validate the inserted row, there are cases where literals will not match
	//	the inserted data, mainly in the case of variant date data, because the date inside a variant does not contain
	//	a fractional seconds component.  So this is now changed to use parameters instead for most cases.
	if (m_fLiteralSelect)
	{
		TEST_CHECK(pTable->CreateSQLStmt(SELECT_ROW_WITH_LITERALS, NULL, &pwszSqlStmt,&cCols, &pCols, ulRowNum), S_OK);

		// Set command text
		TEST_CHECK(pICommandText->SetCommandText(DBGUID_DBSQL , pwszSqlStmt), S_OK);
	}
	else
	{
		HRESULT hrSetParam = E_FAIL;

		TEST_CHECK(pTable->CreateSQLStmt(SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE, NULL, &pwszSqlStmt,&cCols, &pCols, ulRowNum), S_OK);

		// Set command text
		TEST_CHECK(pICommandText->SetCommandText(DBGUID_DBSQL , pwszSqlStmt), S_OK);

		TEST_COMPARE(VerifyInterface(pICommand, IID_IAccessor, COMMAND_INTERFACE, (IUnknown **)&pCmdIAccessor), TRUE)

		//  Create the accessor.
		TEST_CHECK(GetAccessorAndBindings(pCmdIAccessor, DBACCESSOR_PARAMETERDATA,
			&hExecAccessor, &rgBindings, &cBindings, &cbRowSize,			
  			DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH, ALL_COLS_BOUND, 
			FORWARD, NO_COLS_BY_REF, NULL, NULL,  NULL, DBTYPE_EMPTY, 
			0, NULL, NULL, NO_COLS_OWNED_BY_PROV,
			DBPARAMIO_INPUT, BLOB_LONG), S_OK);

		//Set up parameter input values for selecting row 1
		TEST_CHECK (FillInputBindings(pTable, DBACCESSOR_PARAMETERDATA, cBindings,
			rgBindings, &pData, ulRowNum, cCols, pCols, PRIMARY), S_OK);
 
		// Allocate information for SetParameterInfo
		TEST_ALLOC(DBPARAMBINDINFO, pParamBindInfo, 0, (size_t)(sizeof(DBPARAMBINDINFO) * cCols));
		TEST_ALLOC(ParamStruct, pParamStruct, 0, (size_t)(sizeof(ParamStruct) * cCols));
		TEST_ALLOC(DB_UPARAMS, prgParamOrdinals, 0, (size_t)(sizeof(DB_UPARAMS) * cCols));

		// Compare the given Parameter information against the columns information 
		// gotten by ExecuteCommand. In this case cParams should match cColumns.
		for (iCol = 0; iCol < cBindings; iCol++)
		{
			prgParamOrdinals[iCol] = iCol+1;
			AddParam(iCol, pCols[iCol], DBPARAMIO_INPUT, NULL, FALSE, &cbRowSize, NULL, pParamBindInfo,  pParamStruct, pTable);
		}

		TEST_COMPARE(VerifyInterface(pICommand, IID_ICommandWithParameters,
				COMMAND_INTERFACE,(IUnknown **)&pICmdWParams), TRUE);

		// Remove any old parameter information that might be hanging around.
		TEST_CHECK(pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

		pParam = &Param;
		Param.cParamSets = 1;
		Param.hAccessor = hExecAccessor;
		Param.pData = pData;

		// Now reset the statement to retrieve all columns using parameters derived from the
		// searchable and updatable columns
		cCols = 0;
		SAFE_FREE(pCols);
		SAFE_FREE(pwszSqlStmt);
		TEST_CHECK(pTable->CreateSQLStmt(SELECT_FROMTBLWITHPARAMS, NULL, &pwszSqlStmt,&cCols, &pCols, ulRowNum), S_OK);
		TEST_CHECK(pICommandText->SetCommandText(DBGUID_DBSQL , pwszSqlStmt), S_OK);
		hrSetParam = pICmdWParams->SetParameterInfo(cBindings, prgParamOrdinals, pParamBindInfo);

		if (hrSetParam != DB_S_TYPEINFOOVERRIDDEN)
			TEST_CHECK(hrSetParam, S_OK);

	}

	TEST_CHECK(pICommand->Execute(NULL, m_iidExec, pParam, NULL, (IUnknown **)&pIRowset), S_OK);
	
	if (m_iidExec == IID_IRowset && fFetchRow)
	{
		TEST_CHECK (pIRowset->GetNextRows(NULL, 0, cRows, &cRowsObtained, &phRows), S_OK);
		TEST_COMPARE(cRowsObtained, 1);
		TEST_COMPARE(phRows != NULL, TRUE);
	}
	fFound = TRUE;
	
CLEANUP:

	if (fFound)
	{
		if (pcCols)
			*pcCols = cCols;
		if (ppCols)
			*ppCols = pCols;
		else
			SAFE_FREE(pCols);
	}

	// If the rowset and hrows were requested, pass them back
	if (pphRows && fFound)
		*pphRows = phRows;
	else
	{
		//Free all the rows for this GetNextRows call
		if (pIRowset && phRows && m_iidExec == IID_IRowset)
			CHECK(pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);
		SAFE_FREE(phRows);
	}

	if (ppIRowset && fFound)
		*ppIRowset = pIRowset;
	else 
		SAFE_RELEASE (pIRowset);

	SAFE_RELEASE_ACCESSOR(pCmdIAccessor, hExecAccessor);
	SAFE_RELEASE (pICommand);
	SAFE_RELEASE (pICommandText);
	SAFE_RELEASE (pCmdIAccessor);
	SAFE_RELEASE (pICmdWParams);

	if (pData)
		CHECK(ReleaseInputBindingsMemory(cBindings, rgBindings, pData, TRUE), S_OK);

	SAFE_FREE(prgParamOrdinals);
	SAFE_FREE(pParamBindInfo);
	FreeParameterNames(cBindings, pParamStruct);
	SAFE_FREE(pParamStruct);
	SAFE_FREE(rgBindings);
	SAFE_FREE(pwszSqlStmt);

	return fFound;
	
}

//--------------------------------------------------------------------
// @mfunc FindRow
//	Checks to see whether a specified row exists in the table or not.
//  and returns a pData buffer with the updatable columns (user must free)
//--------------------------------------------------------------------
LPBYTE CIVerifyRow::GetUpdatableCols(DBCOUNTITEM ulRowNum, DBCOUNTITEM cBindings, DBBINDING * pBinding,
		DBLENGTH cbRowSize, ICommand * pICommand, CTable *pTable)
{
	LPBYTE pData			= NULL;
	IRowset * pIRowset		= NULL;
	IAccessor * pIAccessor	= NULL;
	HROW * phRows			= NULL;
	HACCESSOR hAccessor		= DB_NULL_HACCESSOR;
	DBCOUNTITEM cBindRowset	= 0;
	DBLENGTH cbRowsizeRowset= 0;
	ULONG iBind;
	DBBINDING * pBindRowset = NULL;
	DBBINDING *	pBindNew	= NULL;

	// Since we're munging the bindings and therefore need IAccessor we can't use a row
	// object here;
	IID iid = m_iidExec;
	m_iidExec = IID_IRowset;

	if (!pTable)
		pTable = m_pTable;

	SAFE_ALLOC(pBindNew, DBBINDING, cBindings);
	memcpy(pBindNew, pBinding, (size_t)(cBindings*sizeof(DBBINDING)));

	TESTC(pICommand != NULL);

	TESTC(FindRow(ulRowNum, pTable, pICommand, &pIRowset, &phRows));

	TESTC(pIRowset!=NULL);
	TESTC(phRows!=NULL);

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown **)&pIAccessor));


	TESTC_(GetAccessorAndBindings(pIRowset, DBACCESSOR_ROWDATA,
				&hAccessor, &pBindRowset, &cBindRowset, &cbRowsizeRowset,			
				DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
				UPDATEABLE_COLS_BOUND, FORWARD, NO_COLS_BY_REF, 
				NULL, NULL,
				NULL, DBTYPE_EMPTY, 0, NULL,
				NULL ,NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG ), S_OK);

	TESTC(cBindings == cBindRowset);

	// We really only wanted the ordinal values required for the updatable cols
	// so we can retrieve the right columns.
	SAFE_RELEASE_ACCESSOR(pIAccessor, hAccessor);

	for (iBind = 0; iBind < cBindings; iBind++)
		pBindNew[iBind].iOrdinal = pBindRowset[iBind].iOrdinal;

	SAFE_FREE(pBindRowset);

	// Now create an accessor with bindings matching the parameter data
	TEST_CHECK(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings,
		pBindNew, cbRowSize, &hAccessor, NULL), S_OK);

	SAFE_ALLOC(pData, BYTE, cbRowSize);

	// Retrieve the row
	TESTC_(pIRowset->GetData(*phRows, hAccessor, pData), S_OK);

	TESTC_(pIRowset->ReleaseRows(1, phRows, NULL, NULL, NULL), S_OK);

CLEANUP:

	// Restore the proper iid
	m_iidExec = iid;
	m_fLiteralSelect = FALSE;	

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_FREE(pBindRowset);
	SAFE_FREE(phRows);
	
	return pData;

}


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//Base Class definition for ICommandWithParameters.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class CICmdWParams: Base class for all ICommandWithParameter test cases.
class CICmdWParams : public CSessionObject
{
public:
	//@cmember Constructor
	CICmdWParams(LPWSTR wszTestCaseName);

	virtual void SetTestCaseParam(ETESTCASE eTestCase = TC_Rowset)
	{
		m_eTestCase = eTestCase;

		switch(eTestCase)
		{
		case TC_Rowset:
			break;
		case TC_Row:
			break;
		default:
			ASSERT(!L"Unhandled Type...");
			break;
		};
	}

	//data
	ETESTCASE	m_eTestCase;
	//@cmember CompareDbParamInfo
	//Compares Data information in rgParamInfo against rgColInfo.
	//Assumes that rgColInfo is initialized to the column information 
	HRESULT CompareDBParamInfo( 
		DBCOUNTITEM		cCols,			// [in]	 Number of columns in rgColumnsInfo.  This is expected to match cParams
		DBORDINAL *		prgParamColMap,	// [in]  Parameter to column mapping array
		DB_UPARAMS		cParams, 		// [in]  Number of parameter information to compare
		DBPARAMINFO *	rgParamInfo,	// [in]  Array of DBPARAMINFO containing parameter info
		LPOLESTR pNamesBuffer			// [in]  Names buffer from GetParameterInfo.
		);

	void DumpCommandProps(IUnknown * pIUnknown, BOOL fPropertiesInError,
			ULONG cPropSets=0, DBPROPSET * pPropSets=NULL);

	// @cmember copies CTestCase public data to local objects
	void TestCaseInfoToObjs(void);

	// @cmember Create pData for both commands.
	BOOL MakeDataForCommand(DBCOUNTITEM cRowNum);

	// @cmember MakeTempTableName to create a table name based on existing table name.
	WCHAR * MakeTempTableName(WCHAR wch);

	// @cmember Generate a row number to be inserted.
	DBCOUNTITEM NextInsertRowNum();

	// @cmember Release pdata for both commands.
	void ReleaseDataForCommand();

	// @cmember True if a provider type is DateTime type.
	BOOL IsColDateTime(DBTYPE ProviderType);

	// @cmember TRUE if a provider type is Numeric.
	BOOL IsColNumeric	( DBTYPE ProviderType );

	// @cmember TRUE if a provider type is I2
	BOOL IsColI2	( DBTYPE ProviderType );

	// Gets all the relevent property values (Sqlserver, Outparameters, multipleparameteres)
	BOOL GetReleventProperties();
	
	// @cmember Release rowdata information.
	inline void ReleaseRowsetPtr(IRowset ** ppRowset) 
	{
		if (*ppRowset)
		{
			(*ppRowset)->Release();
			(*ppRowset) = NULL;
		}
	}

	//@cmember Execute after calling Setparameterinfo.
	// For different combination of variable length columns.
	BOOL ExecuteWithSetParamInfo(
		LPOLESTR pwszDataSourceType, 	// SetParamInfo with this Data type.
		DBTYPE wType					// DbBind for Execute with this Data type.
	);

	//@cmember Create the Sql statement for the wType.
	BOOL GetSqlBindInfo(
		LPOLESTR pwszDataSourceType,			// SetParamInfo with this data type
		DBTYPE wType, 							// DbBind for Execute with this data type
		WCHAR *pwszCreateStatement,				// Create table statement
		WCHAR *pwszDropStatement,				// Drop table statement
		WCHAR *pwszInsertStatement, 			// Insert row statement
		DBBINDING **prgDbBindInfo,				// DbBinding structure generated.
		DBPARAMBINDINFO **prgDbParamBindInfo,	// ParamBindInfo generated for SetParam.
		DBORDINAL **rgParamColumnMap, 
		HACCESSOR *hAccessor
	);

	//@cmember Fillinputbindings for multiple parameter sets.
	//			most of the parameters are for FillInputBindings call(MiscFunc.cpp).
	HRESULT FillInputBindingsForArray(
		CTable *		pTable,					
		DBACCESSORFLAGS dwFlags,				
		DBLENGTH		cbRowSize,
		DBCOUNTITEM		cBindings,
		DBBINDING *		rgBindings,
		BYTE **			ppData,
		ULONG 			cNumRows,		// Number of Rows for which data has to be generated
		DBCOUNTITEM *	rgRowNums,		// Array of rownumbers for which data has to be generated.
		DBORDINAL		cParamColMap,	
		DB_LORDINAL *	rgParamColMap	// Array of Column number mappings for the bindings.
	);
 
 	//@cmember ReleaseInputBindings for multiple parameter sets.
	//			Most of the parameters are for ReleaseInputBindins call (MiscFunc.cpp).
	HRESULT ReleaseInputBindingsForArray(
		ULONG cNumRows,
		DBLENGTH cbRowSize,
		DBCOUNTITEM cBindings,
		DBBINDING *rgBindings,
		BYTE *pData);
	
	BOOL bHasReturnParam(enum TOKEN_ENUM eProcType);

	HRESULT CreateStoredProc(ICommandText * pICommandText, WCHAR * pwszProcName, WCHAR * pwszCreateStmt, BOOL fIsFunction = FALSE);

	HRESULT SetParameterInfoIfNeeded(DB_UPARAMS cParams, DB_UPARAMS * pParamOrdinals, DBPARAMBINDINFO * pParamBindInfo, 
		ICommand * pICommand = NULL);

	HRESULT PrepareForExecute(WCHAR * pwszExecuteStmt, ULONG cParams, DB_UPARAMS * pParamOrdinals,
		DBPARAMBINDINFO * pPARAMBIND, BOOL * pfCanDerive, WCHAR * pwszProcName=NULL,
		WCHAR * pwszCreateProcStmt=NULL, BOOL fIsFunction = FALSE);

	HRESULT ValidateGetParameterInfo(ULONG cExpParams, ULONG cParamSets, DBCOUNTITEM ulRowNum, DB_UPARAMS * pExpOrdinals,
		DBPARAMBINDINFO * pExpParamBind, DBBINDING * pBINDING, DBLENGTH cbRowSize, ParamStruct * pParamAll,
		BYTE * pData, enum ROWSET_ENUM eRowset, DBORDINAL cColumns, DB_LORDINAL * rgColumns,BOOL fCanDerive,
		enum VERIFY_ENUM eVerifyMethod = VERIFY_USE_TABLE, ICommand * pICommand = NULL);

	HRESULT ExecuteAndVerify(ULONG cParams, ULONG cParamSets, ParamStruct * pParamAll,
			DBCOUNTITEM ulRowNum,	DBBINDING * pBINDING, DBLENGTH cbRowSize, BYTE * pData, enum ROWSET_ENUM eRowset,
			DBORDINAL cColumns, DB_LORDINAL * rgColumns, enum VERIFY_ENUM eVerifyMethod, BOOL fRelease,
			ICommand * pICommand = NULL, HRESULT hrExpected = S_OK, DBBINDING * pBindMatch=NULL,
			BYTE * pDataMatch = NULL, DBCOUNTITEM * pcRowsExpected = NULL);

	HRESULT VerifyObj(REFIID riidRowset, IUnknown * pUnkRowset, DBCOUNTITEM ulStartingRow,
		DBORDINAL cRowsetCols, DB_LORDINAL * rgTableColOrds, BOOL fRelease, BOOL fRestart = FALSE, CTable * pTable = NULL,
		DBCOUNTITEM * pcRows = NULL);

	HRESULT VerifyRowObj(REFIID riidRow, IUnknown * pUnkRow, DBCOUNTITEM ulStartingRow,
		DBORDINAL cRowsetCols, DB_LORDINAL * rgTableColOrds, BOOL fRelease, CTable * pTable);

	HRESULT VerifyRowset(REFIID riidRowset, IUnknown * pUnkRowset, DBCOUNTITEM ulStartingRow,
		DBORDINAL cRowsetCols, DB_LORDINAL * rgTableColOrds, BOOL fRelease, BOOL fRestart = FALSE,
		CTable * pTable = NULL, DBCOUNTITEM * pcRowsExpected=NULL);

	HRESULT VerifyOutParams(ULONG cParams, ULONG cParamSets, DBCOUNTITEM ulRowNum, DBBINDING * pBinding,
		DBLENGTH cbRowSize, BYTE * pData, ParamStruct * pParamAll, enum VERIFY_ENUM eVerifyMethod, 
		DBBINDING * pBindMatch=NULL, BYTE * pDataMatch = NULL);

	BOOL CreateProcBindings(
		enum TOKEN_ENUM eProcType,			// [IN]  Proc type, regular proc or function (has return value)
		BOOL fBindByName,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		ULONG cParamSets,					// [IN]	 Number of sets of parameters to be created
		DBTYPE wReturnType,					// [IN]  Return parameter type
		DBCOUNTITEM ulTableRow,				// [IN]  Row number in table to select, insert, or update
		ULONG * pcParams,					// [OUT] Count of params created
		DBLENGTH * pcbRowSize,				// [OUT] Count of bytes for a single row of parameters
		DBBINDING ** ppDBBINDINFO,			// [OUT] Binding array for CreateAccessor
		DB_UPARAMS ** ppParamOrdinals,		// [OUT] Array of cParams ordinals
		DBPARAMBINDINFO ** ppDBPARAMBINDINFO,// [OUT] rgParamBindInfo for SetParameterInfo
		WCHAR ** ppwszCreateStmt,			// [OUT] SQL stmt to create the stored proc
		WCHAR ** ppwszExecProcStmt,			// [OUT] SQL stmt to execute the stored proc
		WCHAR ** ppwszExecStmt,				// [OUT] SQL stmt to execute without stored proc
		WCHAR ** ppwszProcName,				// [OUT] Name of stored proc created
		BYTE ** ppData,						// [OUT] Pointer to data for the parameters
		ParamStruct ** ppParamAll,			// [OUT] Pointer to array of param structs to save name, column
		DBORDINAL * pcColumns = NULL,		// [OUT] Column count for rowset returned by proc or exec
		DB_LORDINAL ** prgColumnsOrd = NULL,// [OUT] Array of mappings of rowset columns to underlying table columns
		DB_LORDINAL ** prgParamColOrd = NULL,// [OUT] Array of mappings of parameters to underlying table columns
		ULONG ulCreateFlags = 0				// [IN]  Flags used to control creation of parameters
	);

	//@cmember  Executes the stored procedure.
	HRESULT ExecuteStoredProcedure(ICommandText *pICommandText, DBPARAMS *pDbParams, 
		DBCOUNTITEM ulRowNum, ULONG DataValues, ULONG BindingType);
	
	// Drop the procedure created.
	BOOL DropStoredProcedure(ICommandText *pICommandText, WCHAR * pwszProcedureName=NULL, BOOL fIsFunction = FALSE);

	// @cmember Speciality functions to store the Byref pointers to free them later.
	BOOL StorePassByRefPointers(DB_UPARAMS cRows, BYTE * pData);
	void ReleaseInputBindingsMemoryByRef(ULONG cRows, BYTE * pData, BOOL fFreeProviderMemory);

	// @cmember Swap the ordinal information for a parameter
	void SwapOrdinal(ULONG ulBindIndex);

	//@cmemmber Destructor
	~CICmdWParams(){};

	CErrorCache		m_EC;	// Error cache to suppress duplicate errors

	// @cmember Command object for select statement with parameters for 
	// columns only.
	ICommand *m_pICommand;

	// @cmember Command object for no particulare command.  Basicaly a scratch command object.
	ICommand *m_pIEmptyCommand;

	// @cmember Rownumber for which to generate pData for using FillInputBindings.
	ULONG m_cSDVRowNum;

	// @cmember Class  to verify the insertion of the row.
	CIVerifyRow cRow;

	// @cmember Rownumber used to create the stored procedure.
	ULONG m_ulSPRowNum;

	// @cmember Rownumber guarenteed to be there. (usually the first row number).
	ULONG m_ulFixedRowNum;

protected:

	IID m_iidExec;

	//@cmember Common base class initialization
	BOOL Init();

	//@cmember Common base class termination
	BOOL Terminate();

	//@cmember Interface pointer for ICommandWithParameters(select for ).
	ICommandWithParameters *m_pICmdWParams;

	//@cmember Interface pointer for ICommandWithParameters(Select for ).
	ICommandPrepare *m_pICommandPrepare;

	//@cmember Interface pointer for ICommandWithParameters(Select for ).
	ICommandText *m_pICommandText;
	
	//@cmember Handle to a created valid  accessor.
	HACCESSOR	m_hAccessor;

	//@cmember Handle to a create valid accessor for the stored procedures.
	HACCESSOR m_hStoredProcAccessor;

	//@cmember pData size for the stored proc.
	ULONG m_cbStoredProcRowSize;

	// Type information for the stored precedure parameters.
	DBCOLUMNINFO *m_rgStoredProcColInfo;

	// General accessors for error checking.
	HACCESSOR m_hGeneralAccessor;
	HACCESSOR m_hNullAccessor;
	HACCESSOR m_hRowdataAccessor;
	HACCESSOR m_hDupParamAccessor;
	
	//@cmember IAccessor on Command Object
	IAccessor *	m_pCmdIAccessor;
	IAccessor * m_pGeneralCmdIAccessor;

	// @cmember Rowset pointer for Stored procedure
	IRowset *m_pIStoredProcRowset;

	//@cmember Count of bindings in m_rgBindings.
	DBCOUNTITEM m_cBindings;

	//@cmember Array of DBBINDINGS used to create an accessor.
	DBBINDING * m_rgBindings;

	//@cmember Count of bindings in m_rgBindings.
	ULONG		m_cStoredProcBindings;

	//@cmember Array of DBBINDINGS used to create an accessor.
	DBBINDING * m_rgStoredProcBindings;

	//@cmember Array of DBCOLUMNSINFO got from GetAccessorAndBinding call
	//			for  columns.
	DBCOLUMNINFO * m_rgColInfo;

	//@cmember count of elements in DBPARAMBINDINFO structure for SetParameterInfo.
	DB_UPARAMS	m_cDbParamBindInfo;
	
	//@cmember Array of BSTR for DBPARAMBINDINFO for corresponding column types.
	ParamStruct *m_pParamStruct;

	// @cmember array of DBPARAMBINDINFO structures.
	DBPARAMBINDINFO *m_rgDbParamBindInfo;

	// @cmember array of DBPARAMBINDINFO structures for stored proc parameters.
	DBPARAMBINDINFO *m_rgStoredProcParamBindInfo;

	//@cmember Array of BSTR for DBPARAMBINDINFO for corresponding data types in Stored proc
	//parameters.
	WCHAR **m_rgwszStoredProcDataSourceTypes;

	// @cmember array of DBPARAMBINDINFO structures.
	DB_UPARAMS *m_rgParamOrdinals;

	// @cmember number of rows affected by the stored procedure query.
	DBROWCOUNT m_cStoredProcRowsAffected;

	// @cmember Number of parameters in the stored procedure
	ULONG m_cStoredProcParamColMap;

	// @cmember Range of parameter to COlumn mapping.
	DB_LORDINAL *m_rgStoredProcParamColMap;

	// @cmember Range of parameter ordinals for the stored procedure.
	ULONG *m_rgStoredProcParamOrdinals;

	// @cmember BOOL to specify whether SetParameterInfo was set or not.
	BOOL m_bSetParameterInfo;

	// @cmember BOOL to Specify if default values for columns are supported (DBPROP_COL_DEFFAULT)
	BOOL	m_fDefaultVal;

	// @cmember BOOL to Specify case of identifiers
	ULONG_PTR m_ulIdentifierCase;

	// @cmember BOOL to Specify case of quoted identifiers
	ULONG_PTR m_ulQuotedIdentifierCase;

	//@cmember Row size used to allocate buffer pointed to by m_pData.
	DBLENGTH m_cbRowSize;

	//@cmember Pointer to consumers's buffer.
	BYTE *		m_pData;

	//@cmember DBPARAMINFO structure for 
	DBPARAMS	m_DbParamsAll;

	//@cmember Count of  columns in rowset, used for 
	//@cmember for parameterized queries.
	ULONG	m_cParamColMap;

	//@cmember Array of ordinals of  columns in rowset, used
	//@cmember for parameterized queries.
	DBORDINAL *	m_rgParamColMap;

	//@cmember Sql statement for Select with parameters.
	WCHAR *m_pwszSqlInsertAllWithParams;

	// @cmember Table pointer for an extra table.
	CTable *m_pExtraTable;

	// @cmember Number of Parameters returned by GetParameterInfo
	DB_UPARAMS m_cParams;

	// @cmember Array of parameter information.
	DBPARAMINFO *m_rgParamInfo;

	// @cmember Buffer to store name information for m_rgParamInfo.
	WCHAR *m_pNamesBuffer;

	// @cmember Create procedure string
	WCHAR *m_pwszCreateProcedureString;

	// @cmember Execute procedure string
	WCHAR *m_pwszExecuteProcedureString;
	
	// @cmember Buffer containing parameter names
	WCHAR ** m_pwszParameterNames;

	// @cmember Base addresss of buffer containing parameter names
	LPOLESTR * m_prgwszParameterNamesBase; 

	// Range of pointers allocated by FillInputBindings for ByRef types.
	// We have to store them because kagera overwrites them when it generates 
	// Output parameter values.
	void **m_rgvpByRefPointers;

	void FreeDescParams();

	CSyntax m_Syntax;

	HRESULT VerifyParamInfo(DBCOUNTITEM cParamsExp, DB_UPARAMS * rgParamOrdinals, DBPARAMBINDINFO * rgDbParamBindInfo,
		DB_UPARAMS cParams, DBPARAMINFO * rgParamInfo, LPOLESTR pNamesBuffer, ULONG idxStart=0, BOOL fDerived = FALSE);
	
	BOOL ReverseArray(void * rgArray, DBCOUNTITEM cElements, ULONG ulElementSize);

	BOOL ScrambleArray(void * rgArray, DB_UPARAMS cElements, ULONG ulElementSize);

	WCHAR ** CreateParameterNames(WCHAR ** ppwszParameterNames, enum NAME_ENUM eNameType, DB_UPARAMS * pcParamNames=NULL, 
		ULONG fColTypes = NO_LONG_COLS);

	void FreeParameterNames(WCHAR ** pwszParameterNames);

	void SetParameterNames(WCHAR ** pwszParameterNames);

	HRESULT SetRowsetPropertyDefault(DBPROPID DBPropID, ICommand * pICommand = NULL);

	CMyError m_MyError;

	DWORD m_dwDriverODBCVer;

	DB_UPARAMS * m_prgParamOrdinals;

	WCHAR ** m_prgwszParameterNames;

	static DBCOUNTITEM m_cInsertRowNum;

	BOOL m_fProcedureSupport;

	// @cmember Result of test
	TESTRESULT m_TestResult;

private:

};

DBCOUNTITEM CICmdWParams::m_cInsertRowNum = TOTAL_NUMBER_OF_ROWS + 1;

//---------------------------------------------------------------------
//@mfunc Constructor for the base class;
//---------------------------------------------------------------------
CICmdWParams::CICmdWParams(LPWSTR wszTestCaseName)
	:	CSessionObject (wszTestCaseName),
		cRow(wszTestCaseName)
		
{
	m_hAccessor = DB_NULL_HACCESSOR;
	m_pICommand = NULL;
	m_pIEmptyCommand = NULL;

	m_hGeneralAccessor = DB_NULL_HACCESSOR;
	m_hNullAccessor = DB_NULL_HACCESSOR;
	m_hRowdataAccessor = DB_NULL_HACCESSOR;
	m_hDupParamAccessor = DB_NULL_HACCESSOR;
	m_hStoredProcAccessor = DB_NULL_HACCESSOR;
	m_cbStoredProcRowSize = 0;
	m_rgStoredProcColInfo = NULL;	

	m_cBindings = 0;
	m_cStoredProcRowsAffected = 0;
	m_cStoredProcParamColMap = 0;
	m_rgBindings = NULL;
	m_rgStoredProcParamColMap = NULL;
	m_rgStoredProcParamOrdinals = NULL;
	m_bSetParameterInfo = FALSE;
	m_ulIdentifierCase = DBPROPVAL_IC_UPPER;
	m_ulQuotedIdentifierCase = DBPROPVAL_IC_UPPER;
	m_cStoredProcBindings = 0;
	m_rgStoredProcBindings = 0;
	m_cDbParamBindInfo = 0;
	m_rgDbParamBindInfo = NULL;
	m_rgParamOrdinals = NULL;
	m_pParamStruct = NULL;
	m_rgColInfo = NULL;
	m_pData = NULL;
	m_cSDVRowNum = 0;
	m_iidExec = IID_IRowset;

	// Since So many rows were inserted into the table starting
	// from row 1.  This numbered row would have been inserted.
	m_ulFixedRowNum = TOTAL_NUMBER_OF_ROWS - 1;

	m_cbRowSize = 0;
	
	m_pCmdIAccessor = NULL;
	m_pGeneralCmdIAccessor = NULL;
	m_pICommandPrepare = NULL;
	m_pICmdWParams = NULL;
	m_pICommandText = NULL;
	m_pIStoredProcRowset = NULL;


	m_cParamColMap = 0;
	m_rgParamColMap = NULL;

	m_rgStoredProcParamBindInfo = NULL;

	m_rgwszStoredProcDataSourceTypes = NULL;


	m_pwszSqlInsertAllWithParams = NULL;
	
	m_pwszCreateProcedureString = NULL;
	m_pwszExecuteProcedureString = NULL;
	m_pExtraTable = NULL;

	m_cParams = 0;
	m_rgParamInfo = NULL;
	m_pNamesBuffer = NULL;
	m_rgvpByRefPointers = NULL;
	m_prgParamOrdinals		= NULL;
	m_prgwszParameterNames	= NULL;
	m_pwszParameterNames	= NULL;
	m_fProcedureSupport=FALSE;		// We assume no stored proc support

	// By default we're testing rowsets being returned from Execute
	SetTestCaseParam(TC_Rowset);
}

void CICmdWParams::DumpCommandProps(IUnknown * pIUnknown, BOOL fPropertiesInError,
	ULONG cPropSets, DBPROPSET * pPropSets)
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

	if (cPropSets && pPropSets)
	{
		cPropertySets = cPropSets;
		pPropertySets = pPropSets;
	}
	else
	{
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
	}

	if (pPropertySets)
	{
		for (ULONG iPropSet = 0; iPropSet < cPropertySets; iPropSet++)
		{
			for (ULONG iProp = 0; iProp < pPropertySets[iPropSet].cProperties; iProp++)
			{
				const LPWSTR pwszTrue = L"VARIANT_TRUE";
				const LPWSTR pwszFalse = L"VARIANT_FALSE";
				const LPWSTR pwszEmpty = L"VT_EMPTY";
				const LPWSTR pwszUnexpected = L"UNEXPECTED";
				WCHAR wszBuff[30] = L"";
				const LPWSTR ppwszStatus[] = {
					L"DBPROPSTATUS_OK",
					L"DBPROPSTATUS_NOTSUPPORTED",
					L"DBPROPSTATUS_BADVALUE",
					L"DBPROPSTATUS_BADOPTION",
					L"DBPROPSTATUS_BADCOLUMN",
					L"DBPROPSTATUS_NOTALLSETTABLE",
					L"DBPROPSTATUS_NOTSETTABLE",
					L"DBPROPSTATUS_NOTSET",
					L"DBPROPSTATUS_CONFLICTING",
					L"DBPROPSTATUS_NOTAVAILABLE",
				};

				LPWSTR pwszValue = pwszUnexpected;
				switch(V_VT(&pPropertySets[iPropSet].rgProperties[iProp].vValue))
				{
					case VT_EMPTY:
						pwszValue = pwszEmpty;
						break;
					case VT_BOOL:
						if (V_BOOL(&pPropertySets[iPropSet].rgProperties[iProp].vValue) == VARIANT_TRUE)
							pwszValue = pwszTrue;
						else if (V_BOOL(&pPropertySets[iPropSet].rgProperties[iProp].vValue) == VARIANT_FALSE)
							pwszValue = pwszFalse;
						break;
					case VT_I4:
						swprintf(wszBuff, L"%d", V_I4(&pPropertySets[iPropSet].rgProperties[iProp].vValue));
						pwszValue = (LPWSTR)wszBuff;
						break;
					case VT_BSTR:
						pwszValue = V_BSTR(&pPropertySets[iPropSet].rgProperties[iProp].vValue);
						break;
				}


				{
					DBPROPINFO * pPropInfo = NULL;

					pPropInfo = GetPropInfo(pPropertySets[iPropSet].rgProperties[iProp].dwPropertyID, pPropertySets[iPropSet].guidPropertySet,
						m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE);

					if (pPropInfo && pPropInfo->pwszDescription)
					{
						odtLog << L"\t" << iPropSet << L" " << iProp << L" Property " << pPropInfo->pwszDescription << L" " << pwszValue;
						PROVIDER_FREE(pPropInfo->pwszDescription);
					}
					else
						odtLog << L"\t" << iPropSet << L" " << iProp << L" Property " << pPropertySets[iPropSet].rgProperties[iProp].dwPropertyID << L" " << pwszValue;

					if (pPropertySets[iPropSet].rgProperties[iProp].dwOptions == DBPROPOPTIONS_REQUIRED)
						odtLog << L" REQUIRED";
					else
						odtLog << L" OPTIONAL";

					if (pPropInfo->dwFlags & DBPROPFLAGS_READ)
						odtLog << L" READ";

					if (pPropInfo->dwFlags & DBPROPFLAGS_WRITE)
						odtLog << L" WRITE";

					odtLog << L" " << ppwszStatus[pPropertySets[iPropSet].rgProperties[iProp].dwStatus];

					odtLog << L"\n";

					PROVIDER_FREE(pPropInfo);
				}

			}
			odtLog << L"\n";
		}

		// Only free the properties if we obtained them.
		if (!(cPropSets && pPropSets))
			FreeProperties(&cPropertySets, &pPropertySets);
	}
	SAFE_RELEASE(pICmdProp);
	SAFE_RELEASE(pIRowsetInfo);
}


//---------------------------------------------------------------------
//@mfunc  Copy testcase information from the base class to member
//			command objects.  As all use common test case information.
//
void
CICmdWParams::TestCaseInfoToObjs(void)
{
	cRow.SetOwningMod(0, this->m_pThisTestModule);
}

//--------------------------------------------------------------------
//@mfunc Routine to Free parameters for GetParameterInfo.
//@rdesc void
//
void
CICmdWParams::FreeDescParams()
{
	FREE_DATA (m_pNamesBuffer);
	FREE_DATA (m_rgParamInfo);
}	

//--------------------------------------------------------------------
//@mfunc Gets relevent properties and sets member variables.
BOOL
CICmdWParams::GetReleventProperties()
{
	WCHAR *			pwszDriverODBCVer;

	// Needed for testing case of named parameters
	if (!GetProperty(DBPROP_IDENTIFIERCASE, 
				   DBPROPSET_DATASOURCEINFO,m_pIDBInitialize, &m_ulIdentifierCase))
		m_ulIdentifierCase = DBPROPVAL_IC_UPPER;

	// Needed for testing case of named parameters
	if (!GetProperty(DBPROP_QUOTEDIDENTIFIERCASE, 
				   DBPROPSET_DATASOURCEINFO,m_pIDBInitialize, &m_ulQuotedIdentifierCase))
		m_ulQuotedIdentifierCase = DBPROPVAL_IC_UPPER;

	if (GetProperty(KAGPROP_DRIVERODBCVER, DBPROPSET_PROVIDERDATASOURCEINFO, 
				   m_pIDBInitialize,&pwszDriverODBCVer))
		m_dwDriverODBCVer=_wtoi(pwszDriverODBCVer);
	else
		m_dwDriverODBCVer=0;	// Unknown/not Kagera provider

	m_fDefaultVal = SupportedProperty(DBPROP_COL_DEFAULT, DBPROPSET_COLUMN, m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE);

	// Needed for DBPROP_DBMSNAME
	ULONG			cPropertyIDSets = 1;
	DBPROPIDSET		rgPropertyIDSets[1];
	DBPROPID		rgPropertyIDs[]=
	{
		DBPROP_DBMSNAME,
		DBPROP_PROVIDERNAME,
		DBPROP_PROCEDURETERM
	};
	ULONG			cPropertySets  = 0;
	ULONG			ulMaxParamNameLen = SP_MAX_PARAMNAME_LENGTH;
	DBPROPSET *		rgPropertySets = NULL;
	IDBProperties *	pIDBProp = NULL;
	IDBInfo *		pIDBInfo = NULL;
	DBLITERALINFO * pLiteralInfo = NULL;
	ULONG			cLiterals;
	WCHAR *			pwszCharBuffer = NULL;
	DBLITERAL		dbliteral = DBLITERAL_COLUMN_NAME;

	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	rgPropertyIDSets[0].cPropertyIDs = sizeof(rgPropertyIDs)/sizeof(DBPROPID);
	rgPropertyIDSets[0].rgPropertyIDs=&rgPropertyIDs[0];
	
	if(FAILED(m_pIDBInitialize->QueryInterface(IID_IDBProperties,(void **)&pIDBProp)))
		return FALSE;

	if (SUCCEEDED(m_pIDBInitialize->QueryInterface(IID_IDBInfo,(void **)&pIDBInfo)))
	{
		if (S_OK == pIDBInfo->GetLiteralInfo(1, &dbliteral, &cLiterals, &pLiteralInfo, &pwszCharBuffer))
		{
			ulMaxParamNameLen = pLiteralInfo->cchMaxLen;

			// Per spec the value may have no maximum or the max may be unknown, so use our default value.
			if (ulMaxParamNameLen == ~0)
				ulMaxParamNameLen = SP_MAX_PARAMNAME_LENGTH;

			// Set the maximum parameter name length to 30 for now due to spec issues.
			ulMaxParamNameLen = 29;
			odtLog << L"Maximum parameter name length hard-coded to 29 due to spec issues.\n";

			SAFE_FREE(pLiteralInfo);
			SAFE_FREE(pwszCharBuffer);

		}
	}

	if (pIDBProp->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropertySets, &rgPropertySets) == S_OK)
	{
		// Initialize string lookups
		m_Syntax.Init(V_BSTR(&(rgPropertySets[0].rgProperties[1].vValue)),	// Provider name
			V_BSTR(&(rgPropertySets[0].rgProperties[0].vValue)),			// DBMS name
			NUMELEM(g_DialectTokens),										// Size of dialect list
			g_DialectTokens,												// List of dialect tokens
			g_ProviderList,													// List of providers
			g_DBMSList,														// List of DBMS's
			g_Dialects,														// List of dialects	
			m_pTable,														// Table to use
			ulMaxParamNameLen);												// Maximum size of a parameter name

		// If the DBPROP_PROCEDURETERM isn't NULL, then the provider/dbms supports procedures
		if (V_BSTR(&(rgPropertySets[0].rgProperties[2].vValue)))
		{
			m_fProcedureSupport=TRUE;
			if (!m_Syntax.IsKnown())
			{
				odtLog <<L"Procedures are supported but the dialect is unknown.  Assuming ANSI SQL. \n";
			}
		}
	}

	//Free rgPropertySets
	SAFE_RELEASE(pIDBProp);
	SAFE_RELEASE(pIDBInfo);
	FreeProperties(&cPropertySets, &rgPropertySets);

	return TRUE;
}


//--------------------------------------------------------------------
//@mfunc FillinputBindings variant to fill pData for a number of rows.
//		Assumes the *ppData is already allocated.
//--------------------------------------------------------------------
HRESULT  
CICmdWParams::FillInputBindingsForArray(
	CTable *pTable,
	DBACCESSORFLAGS dwFlags,
	DBLENGTH cbRowSize,
	DBCOUNTITEM cBindings,
	DBBINDING *rgBindings,
	BYTE **ppData,
	ULONG cNumRows,	
	DBCOUNTITEM *rgRowNums,
	DBORDINAL cParamColMap,
	DB_LORDINAL *rgParamColMap
	)
{
	ULONG i = 0;
	HRESULT hResult = S_OK;
	BYTE *pDataPtr=NULL;

	ASSERT (*ppData);
	for (i = 0; i < cNumRows; i++)
	{
		// Get to the next row.
		pDataPtr = *ppData + (i * cbRowSize);
		hResult = FillInputBindings(pTable, dwFlags, cBindings, 
			rgBindings, &pDataPtr, rgRowNums[i], cParamColMap, rgParamColMap); 
		if (FAILED(hResult)) // If we fail return from here. no need to do any further.
			break;
	}
	return hResult;

}


//--------------------------------------------------------------------
//@mfunc ReleaseInputBindings variant to release pData allocated by 
//			FillInputBindingsForArray.
//			Calls ReleaseInputBindings and Frees memory pointed by pData;
//--------------------------------------------------------------------
HRESULT 
CICmdWParams::ReleaseInputBindingsForArray(
	ULONG cNumRows,
	DBLENGTH cbRowSize,
	DBCOUNTITEM cBindings,
	DBBINDING *rgBindings,
	BYTE *pData)
{
	ULONG i = 0;
	BYTE *pDataPtr = NULL;
	HRESULT hResult = S_OK;

	if (!pData)
		return E_FAIL;

	for (i = 0; i < cNumRows; i++)
	{
		// Get to the next row.
		pDataPtr = pData + ( i * cbRowSize );
		hResult = ReleaseInputBindingsMemory(cBindings, rgBindings, pDataPtr, FALSE);
	}
	PROVIDER_FREE(pData);
	return hResult;

}


//--------------------------------------------------------------------
//@mfunc Base class Initialization Routine
//@rdesc TRUE or FALSE
//
BOOL
CICmdWParams::Init()
{
	BOOL fResult = FALSE;
	CCol	TempCol(m_pIMalloc);
	DB_UPARAMS	cParams;
	DBPARAMINFO *rgLocalParamInfo = NULL;
	DBORDINAL	cColsOut;
	UINT	i=0;
	BOOL fLocalPrepare = FALSE;
	IRowset *pIRowset= NULL;

	// Initialize error cache
	m_EC.Init(GetModInfo()->GetDebugMode());

	if (COLEDB::Init())
	{
		if (m_pThisTestModule && m_pThisTestModule->m_pVoid)
			m_cInsertRowNum = ((CTable *)(m_pThisTestModule->m_pVoid))->CountRowsOnTable()+1;

		// Get the IDBInitialize interface pointer.
		if (!VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBInitialize,
				DATASOURCE_INTERFACE,(IUnknown **)&m_pIDBInitialize))
			goto CLEANUP;

		//Copy the IDBCreateCommand pointer we got at the module level
		//down to the testcase level.  Note, this increments the ref count, 
		//So we call ReleaseDbSession in the Terminate, but the DbSession does
		//not go away until ModuleTerminate time.
		SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);

		// Set the session for the CRow object.
		cRow.SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);
		
		//Have this testcase use the table but don't
		//let table be deleted, since we'll use it for next test case.
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);
		cRow.SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);

		// Gets the required property values.
		GetReleventProperties();

		// Create another table.
		m_pExtraTable = new CTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
			(LPWSTR)gwszModuleName);
			
		if (!m_pExtraTable)
		{
			PRVTRACE (wszMemoryAllocationError);
			return FALSE;
		}

		TestCaseInfoToObjs();

		//	Opening a rowset on Existing command object to stay in FireHose mode.
		//	and Creating the other command so that they will be generated on a new hdbc.	
		TEST_CHECK (m_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset, NULL, NULL, 
			NULL, NULL,  EXECUTE_ALWAYS, 0, NULL, NULL, (IUnknown **)&pIRowset, &m_pTable->m_pICommand), S_OK);

		// Create Two Command Objects.  
		if(FAILED(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&m_pICommand)))
		{
			odtLog << "Create command on m_pICommand Failed\n";
			goto CLEANUP;
		}

		if(FAILED(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&m_pIEmptyCommand)))
		{
			odtLog << "Create command on m_pIEmptyCommand Failed\n";
			goto CLEANUP;
		}

		// Now we are done with the use of rowset. so release it.
		// Since the our command objects are created with a new hdbc because of the existing 
		// firehose mode. We can release the rowset now.
		if (pIRowset) ReleaseRowsetPtr(&pIRowset);

		/////////////////////////////////////////////////////////
		// Build array containing col ordinals for all able
		// cols so we can use them in parameterized queries
		/////////////////////////////////////////////////////////

		//Get memory to hold array of all col numbers.  NOTE:  This 
		//is the max possible, we won't necessarily use them all.

		// Mapping array to be used by GetAccessorAndBindings and FillInputBinding
		m_rgParamColMap = (DBORDINAL *)m_pIMalloc->Alloc(m_pTable->CountColumnsOnTable() * sizeof(DBORDINAL));
		if (m_rgParamColMap == NULL)
		{
			odtLog << wszMemoryAllocationError;
			goto CLEANUP;
		}
		
		m_rgParamOrdinals = (DB_UPARAMS *)m_pIMalloc->Alloc(m_pTable->CountColumnsOnTable() * sizeof(DB_UPARAMS));
		if (m_rgParamOrdinals == NULL)
		{
			odtLog << wszMemoryAllocationError;
			goto CLEANUP;
		}
		
		// Data source type for use in SetParameterInfo.
		m_pParamStruct = (ParamStruct *)m_pIMalloc->Alloc(m_pTable->CountColumnsOnTable() * sizeof(ParamStruct));
		if (m_pParamStruct == NULL)
		{
			odtLog << wszMemoryAllocationError;
			goto CLEANUP;
		}
		
		// Parameter Bind information for use in SetParameterInfo.
		m_rgDbParamBindInfo = (DBPARAMBINDINFO *)m_pIMalloc->Alloc(m_pTable->CountColumnsOnTable() * sizeof(DBPARAMBINDINFO));
		if (m_rgDbParamBindInfo == NULL)
		{
			odtLog << wszMemoryAllocationError;
			goto CLEANUP;
		}

		// Set all array entries to NULL so terminate can free the pointers if non-NULL.
		memset(m_rgDbParamBindInfo, 0, (size_t)(m_pTable->CountColumnsOnTable() * sizeof(DBPARAMBINDINFO)));
		
		//We'll use this count as the index to the array as we build it
		m_cParamColMap = 0;
		m_cDbParamBindInfo = 0;

		for (i = 1; i <= m_pTable->CountColumnsOnTable(); i++)
		{
			CHECK(m_pTable->GetColInfo(i, TempCol), S_OK);

			//Record the column number in the array if it is  updateable.
			if (TempCol.GetUpdateable() )
			{
				m_rgParamColMap[m_cParamColMap] = TempCol.GetColNum();				
				m_rgParamOrdinals[m_cDbParamBindInfo] = m_cDbParamBindInfo+1;  // Parameter ordinal number.
				AddParam(m_cDbParamBindInfo, i, DBPARAMIO_INPUT, NULL, FALSE, NULL, NULL, m_rgDbParamBindInfo, m_pParamStruct, m_pTable);

				m_cParamColMap++;
				m_cDbParamBindInfo++;

			
			}
			
		}


		//  Get the interface pointer for Accessor.
		if (!VerifyInterface(m_pICommand, IID_IAccessor,
				COMMAND_INTERFACE,(IUnknown **)&m_pCmdIAccessor))
		{
			goto CLEANUP;
		}
		
		// Get the Interface for a Command Text Object.
		if (!VerifyInterface(m_pICommand, IID_ICommandText,
				COMMAND_INTERFACE,(IUnknown **)&m_pICommandText))
		{
			goto CLEANUP;
		}
		
		// Get the Interface pointer for ICommandWithparameters Object.
		if (!VerifyInterface(m_pICommand, IID_ICommandWithParameters,
				COMMAND_INTERFACE,(IUnknown **)&m_pICmdWParams))
		{
			goto CLEANUP;
		}

		// Get the Interface pointer for ICommandPrepare Object.
		if (!VerifyInterface(m_pICommand, IID_ICommandPrepare,
				COMMAND_INTERFACE,(IUnknown **)&m_pICommandPrepare))
		{
			// Continue;
			goto CLEANUP;
		}
		
		////////////////////////////////////////////////////////////////////////////////
		//Now build the accessor using only the columns in m_rgUpdateableCols
		////////////////////////////////////////////////////////////////////////////////
		
		//Create a select for only the updatable columns
		TEST_CHECK (m_pTable->ExecuteCommand(SELECT_UPDATEABLE, IID_IUnknown, NULL, NULL, 
			NULL, NULL,  EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);
		
		
		// Get the Bindings for the Updateable columns only.
		// Passing an array containing the column ordinals for updateable columns only.
		TEST_CHECK(GetAccessorAndBindings(m_pCmdIAccessor, 
						DBACCESSOR_PARAMETERDATA, &m_hAccessor, &m_rgBindings, &m_cBindings, 
						&m_cbRowSize, DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH, UPDATEABLE_COLS_BOUND, 
						FORWARD, NO_COLS_BY_REF,  &m_rgColInfo, &cColsOut, NULL, DBTYPE_EMPTY, m_cParamColMap,  
						(LONG_PTR *)m_rgParamColMap,  (DBORDINAL *)m_rgParamOrdinals ,NO_COLS_OWNED_BY_PROV,
						DBPARAMIO_INPUT, BLOB_LONG ), S_OK);


		// Lets restore the text object to insert command.
		TEST_CHECK(m_pTable->ExecuteCommand(INSERT_ALLWITHPARAMS, IID_IUnknown, NULL, 
			&m_pwszSqlInsertAllWithParams, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);
		
		// Now test GetParameterInfo and if it fails use SetParameterInfo to Build Initial
		// list.
		m_hr = m_pICmdWParams->GetParameterInfo(&cParams, &rgLocalParamInfo, NULL);
		if((m_hr == DB_E_NOTPREPARED))
		{
			OLECHAR * pNamesBuffer=NULL;
			
			//Have to prepare the statement first.
			fLocalPrepare = TRUE;
			TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
			m_hr = m_pICmdWParams->GetParameterInfo(&cParams, &rgLocalParamInfo, &pNamesBuffer);
			PROVIDER_FREE(pNamesBuffer);
		} 

		if (m_hr == DB_E_PARAMUNAVAILABLE || (SUCCEEDED(m_hr) && (cParams == 0 || !rgLocalParamInfo)))
		{
			if (!CHECK(m_hr, DB_E_PARAMUNAVAILABLE))
			{
				// This is a bug in the provider, but I don't want to block all testing, so default to calling
				// SetParameterInfo but flag as a failure
				odtLog << L"Error: Provider claims to derive parameter information but doesn't, defaulting " \
					L"to using SetParameterInfo. This may cause errors in some variations. \n\n";
			}

			m_hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
			if (m_hr != S_OK && m_hr != DB_S_TYPEINFOOVERRIDDEN )
			{
				CHECK(m_hr, S_OK);
				goto CLEANUP;
			}

			// SetParameterInfo called successfully.
			// Means that provider could not describe parameters and we have to use SetParameterInfo.
			m_bSetParameterInfo = TRUE;
		}
		else if (m_hr != S_OK)
		{
			// GetParameterInfo returned an unexpected error, fResult is FALSE.
			goto CLEANUP;
		}
		if (fLocalPrepare)
		{
			fLocalPrepare = FALSE;
			TEST_CHECK (m_pICommandPrepare->Unprepare(), S_OK);
		}

		// Set the IID to use for data retrieval
		if (m_eTestCase == TC_Rowset)
			m_iidExec = IID_IRowset;
		else
			m_iidExec = IID_IRow;

		// Set the iid for the cRow object to use to match.
		cRow.SetIID(m_iidExec);

		fResult = TRUE;
	}

CLEANUP:
	// Cleanup before returning
	if (pIRowset)
		ReleaseRowsetPtr(&pIRowset);
	if (rgLocalParamInfo)
		m_pIMalloc->Free(rgLocalParamInfo);

	return fResult;
}


//----------------------------------------------
//@mfunc Generate the next row number to insert.
//----------------------------------------------
DBCOUNTITEM CICmdWParams::NextInsertRowNum()
{
	return (m_cInsertRowNum++);
}

//-------------------------------------------------------------------------------
//@mfunc Prepare parameter structure for Execute command (parameter information).
//	release if previously allocated.
//-------------------------------------------------------------------------------
BOOL
CICmdWParams::MakeDataForCommand(DBCOUNTITEM ulRowNum)
{
	//  Use the Bindings generated in Init and create RowData.
	ReleaseDataForCommand();

	if (!CHECK(FillInputBindings((CTable *)m_pThisTestModule->m_pVoid, DBACCESSOR_PARAMETERDATA,
				m_cBindings, m_rgBindings, &m_pData, ulRowNum, m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK))
	{
		return FALSE;
	}

	m_DbParamsAll.cParamSets = 1;
	m_DbParamsAll.hAccessor = m_hAccessor;
	m_DbParamsAll.pData = m_pData;

	return TRUE;
}
//-----------------------------------------------------------------------------
//@mfunc MakeTempTableName (Replace first character of the existing table name.)
//------------------------------------------------------------------------------
WCHAR *
CICmdWParams::MakeTempTableName(WCHAR wch)
{
	WCHAR *pwszTableName;

	pwszTableName = (WCHAR *)m_pIMalloc->Alloc(wcslen(m_pTable->GetTableName())*sizeof (WCHAR) + sizeof (WCHAR));
	
	// if not null process else return null.
	if (pwszTableName)
	{

		// Now copy table name.
		wcscpy(pwszTableName, m_pTable->GetTableName());

		// Replace first character with parameter.
		pwszTableName[0] = wch;
	}

	return pwszTableName;
}
//------------------------------------------------------------------------------
//@mfunc Release allocated memory for the parameter structure.
//---------------------------------------------------------------------
void
CICmdWParams::ReleaseDataForCommand()
{
	if (m_pData) 
	{
		ReleaseInputBindingsMemory( m_cBindings, m_rgBindings, m_pData, TRUE);
		m_pData = NULL;
	}

}

enum {
	SUBTYPE_LONGVARCHAR,
	SUBTYPE_VARCHAR,
	SUBTYPE_CHAR,
	SUBTYPE_WLONGVARCHAR ,
	SUBTYPE_WVARCHAR ,
	SUBTYPE_WCHAR,
	SUBTYPE_LONGVARBINARY ,
	SUBTYPE_VARBINARY,
	SUBTYPE_BINARY
};
//--------------------------------------------------------------------
// @mfunc Create sql statements, Binding information, and accessor for
//			the types specified by input arguements.
//--------------------------------------------------------------------
BOOL
CICmdWParams::GetSqlBindInfo(
	LPOLESTR pwszDataSourceType,			//	[In]
	DBTYPE	wType,							//	[In]
	WCHAR *pwszCreateStatement,				//	[Out]
	WCHAR *pwszInsertStatement,				//	[Out]
	WCHAR *pwszDropStatement,
	DBBINDING ** prgDbBindInfo,				//	[Out]
	DBPARAMBINDINFO ** prgDbParamBindInfo, 	//	[Out]
	DBORDINAL ** prgParamColumnMap, 			//	[Out]
	HACCESSOR * phLocalAccessor)				//	[Out]
{
	BOOL fResult = FALSE, fFound = FALSE;
	ULONG ulSubType = 0, i = 0;
	CCol	TempCol;
	WCHAR  *pwszTableName=NULL;
	WCHAR	wszCreateFormat[] = L"Create table %s ( %s)";
	WCHAR	wszInsertFormat[] = L"Insert into %s ( %s ) values ( ? )";
	WCHAR	wszDropFormat[] = L"Drop table %s";
	WCHAR	*wszColDef = NULL;
	
	// Initialize the output strings.
	pwszCreateStatement[0] = L'\0';
	pwszInsertStatement[0] = L'\0';
	pwszDropStatement[0] = L'\0';
	
	
	TEST_ALLOC(DBBINDING, *prgDbBindInfo, 0, sizeof(DBBINDING));
	TEST_ALLOC(DBPARAMBINDINFO, *prgDbParamBindInfo, 0, sizeof(DBPARAMBINDINFO));
	TEST_ALLOC(DBORDINAL, *prgParamColumnMap, 0, sizeof(DBORDINAL));

	switch (wType)
	{
		case DBTYPE_STR:
			if (! wcscmp ((pwszDataSourceType), L"DBTYPE_LONGVARCHAR"))
				ulSubType = SUBTYPE_LONGVARCHAR;
			else if (! wcscmp ((pwszDataSourceType), L"DBTYPE_VARCHAR"))
				ulSubType = SUBTYPE_VARCHAR;
			else
				ulSubType = SUBTYPE_CHAR;
			break;
		case DBTYPE_WSTR:
			if (! wcscmp ((pwszDataSourceType), L"DBTYPE_WLONGVARCHAR"))
				ulSubType = SUBTYPE_WLONGVARCHAR;
			else if (! wcscmp ((pwszDataSourceType), L"DBTYPE_WVARCHAR"))
				ulSubType = SUBTYPE_WVARCHAR;
			else
				ulSubType = SUBTYPE_WCHAR;
			break;
		case DBTYPE_BYTES:
			if (! wcscmp ((pwszDataSourceType), L"DBTYPE_LONGVARBINARY"))
				ulSubType = SUBTYPE_LONGVARBINARY;
			else if (! wcscmp ((pwszDataSourceType), L"DBTYPE_VARBINARY"))
				ulSubType = SUBTYPE_VARBINARY;
			else
				ulSubType = SUBTYPE_BINARY;
			break;
		default:
			// Code might have to be added if it comes here.
			ASSERT (TRUE);
			break;
	}

	(*prgDbParamBindInfo)->pwszDataSourceType = NULL;
	for (i = 1; i <= m_pTable->CountColumnsOnTable(); i++)
	{
		if (FAILED(m_pTable->GetColInfo(i, TempCol)))
			goto CLEANUP;

		// 
		if (!(TempCol.GetProviderType() == DBTYPE_STR ||
			TempCol.GetProviderType() == DBTYPE_WSTR ||
			TempCol.GetProviderType() == DBTYPE_BYTES) ||
			!TempCol.GetUpdateable())
			continue;

		switch(ulSubType)
		{
			case SUBTYPE_LONGVARCHAR:
				// Has to be long and Not fixed length type
				if ((TempCol.GetProviderType() == DBTYPE_STR) && TempCol.GetIsLong()	)
				{
					LPOLESTR ptr;
					ptr = WCSDUP(m_pIMalloc, L"DBTYPE_LONGVARCHAR");
					(*prgDbParamBindInfo)->pwszDataSourceType =  ptr;
					(*prgDbBindInfo)->wType = DBTYPE_STR;
				//	paramBindInfo.pwszDataSourceType = ptr;
					fFound = TRUE;
				}
				else
					continue;
				break;
			case SUBTYPE_VARCHAR:
				if ((TempCol.GetProviderType() == DBTYPE_STR) &&
					!(TempCol.GetIsLong()) &&
					!(TempCol.GetIsFixedLength())
					)
				{
					(*prgDbParamBindInfo)->pwszDataSourceType =  WCSDUP(m_pIMalloc, L"DBTYPE_VARCHAR");
					(*prgDbBindInfo)->wType = DBTYPE_STR;
					fFound = TRUE;
				}
				else
					continue;
				break;
			case SUBTYPE_CHAR:
				if ((TempCol.GetProviderType() == DBTYPE_STR) &&
					!(TempCol.GetIsLong()) &&
					TempCol.GetIsFixedLength()
					)
				{
					(*prgDbParamBindInfo)->pwszDataSourceType =  WCSDUP(m_pIMalloc, L"DBTYPE_CHAR");
					(*prgDbBindInfo)->wType = DBTYPE_STR;
					fFound = TRUE;
				}
				else
					continue;
				break;

			case SUBTYPE_WLONGVARCHAR:
				// Has to be long and Not fixed length type
				if ((TempCol.GetProviderType() == DBTYPE_WSTR) && TempCol.GetIsLong() )
				{
					(*prgDbParamBindInfo)->pwszDataSourceType =  WCSDUP(m_pIMalloc, L"DBTYPE_WLONGVARCHAR");
					(*prgDbBindInfo)->wType = DBTYPE_WSTR;
					fFound = TRUE;
				}
				else
					continue;
				break;
			case SUBTYPE_WVARCHAR:
				if ((TempCol.GetProviderType() == DBTYPE_WSTR) &&
					!(TempCol.GetIsLong()) &&
					!(TempCol.GetIsFixedLength())
					)
				{
					(*prgDbParamBindInfo)->pwszDataSourceType =  WCSDUP(m_pIMalloc, L"DBTYPE_WVARCHAR");
					(*prgDbBindInfo)->wType = DBTYPE_WSTR;
					fFound = TRUE;
				}
				else
					continue;
				break;
			case SUBTYPE_WCHAR:
				if ((TempCol.GetProviderType() == DBTYPE_STR) &&
					!(TempCol.GetIsLong()) &&
					TempCol.GetIsFixedLength()
					)
				{
					(*prgDbParamBindInfo)->pwszDataSourceType =  WCSDUP(m_pIMalloc, L"DBTYPE_WCHAR");
					(*prgDbBindInfo)->wType = DBTYPE_WSTR;
					fFound = TRUE;
				}
				else
					continue;
				break;
			case SUBTYPE_LONGVARBINARY:
				// Has to be long and Not fixed length type
				if ((TempCol.GetProviderType() == DBTYPE_BYTES) && TempCol.GetIsLong() )
				{
					(*prgDbParamBindInfo)->pwszDataSourceType =  WCSDUP(m_pIMalloc, L"DBTYPE_LONGVARBINARY");
					(*prgDbBindInfo)->wType = DBTYPE_BYTES;
					fFound = TRUE;
				}
				else
					continue;
				break;
			case SUBTYPE_VARBINARY:
				if ((TempCol.GetProviderType() == DBTYPE_BYTES) &&
					!(TempCol.GetIsLong()) &&
					!(TempCol.GetIsFixedLength())
					)
				{
					(*prgDbParamBindInfo)->pwszDataSourceType =  WCSDUP(m_pIMalloc, L"DBTYPE_VARBINARY");
					(*prgDbBindInfo)->wType = DBTYPE_BYTES;
					fFound = TRUE;
				}
				else
					continue;
				break;
			case SUBTYPE_BINARY:
				if ((TempCol.GetProviderType() == DBTYPE_BYTES) &&
					!(TempCol.GetIsLong()) &&
					TempCol.GetIsFixedLength()
					)
				{
					(*prgDbParamBindInfo)->pwszDataSourceType =  WCSDUP(m_pIMalloc, L"DBTYPE_BINARY");
					(*prgDbBindInfo)->wType = DBTYPE_BYTES;
					fFound = TRUE;
				}
				else
					continue;
				break;
			default:
				ASSERT (!L"Need to Add more code");
				break;
				

		}

		if (fFound)
			break;
	}
	

	if (fFound)
	{
		TempCol.CreateColDef( &wszColDef); 

		pwszTableName = MakeTempTableName(L'Z');
		// Found the type.  Now form the statement and break;
		swprintf(pwszCreateStatement, wszCreateFormat, pwszTableName, wszColDef);
		swprintf(pwszInsertStatement, wszInsertFormat, pwszTableName, TempCol.GetColName());
		swprintf(pwszDropStatement, wszDropFormat, pwszTableName);
		// Free wszColDef.
		FREE_DATA (wszColDef);
		FREE_DATA (pwszTableName);

		// ParamBindinfo.
		
		// Build Information for SetParameterInfo.
		
		
		(*prgDbParamBindInfo)->pwszName = NULL;
		(*prgDbParamBindInfo)->dwFlags = DBPARAMFLAGS_ISINPUT;// | DBPARAMFLAGS_ISOUTPUT;
		if (TempCol.GetNullable() == TRUE )
			(*prgDbParamBindInfo)->dwFlags |= DBPARAMFLAGS_ISNULLABLE;

		(*prgDbParamBindInfo)->ulParamSize = TempCol.GetPrecision();
		(*prgDbParamBindInfo)->bScale = (BYTE)TempCol.GetScale();
		(*prgDbParamBindInfo)->bPrecision = (BYTE)TempCol.GetPrecision();

		// DbBinding info.
		(*prgDbBindInfo)->dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
		(*prgDbBindInfo)->eParamIO = DBPARAMIO_INPUT ;
		(*prgDbBindInfo)->iOrdinal = 1;
		(*prgDbBindInfo)->pTypeInfo = NULL;
		(*prgDbBindInfo)->obValue = offsetof(DATA, bValue);
		(*prgDbBindInfo)->cbMaxLen = TempCol.GetMaxSize();
		(*prgDbBindInfo)->obLength = offsetof(DATA, ulLength);
		(*prgDbBindInfo)->obStatus = offsetof (DATA, sStatus);
		
		(*prgDbBindInfo)->dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		(*prgDbBindInfo)->pBindExt = NULL;
		(*prgDbBindInfo)->bPrecision = 0;
		(*prgDbBindInfo)->bScale = 0;
		// Now Set the column mapping with original table for FillInputBindings.
		(*prgParamColumnMap)[0] = TempCol.GetColNum();

			// Lets create the accessor.
		TEST_CHECK (m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, 1, *prgDbBindInfo, 5000, 
			phLocalAccessor, NULL), S_OK);
		// Everything is OK.
		fResult = TRUE;
	}


CLEANUP:

	return fResult;
}

//---------------------------------------------------------------------
// @mfunc Use SetParameterInfo to Set typeinfo and then execute .
//			for all possible variations of DBTYPE_STR, DBTYPE_WSTR, DBTYPE_BYTES
//---------------------------------------------------------------------
BOOL
CICmdWParams::ExecuteWithSetParamInfo(
	LPOLESTR pwszDataSourceType,
	DBTYPE wType
	)
{
	WCHAR *wszCreateStatement = NULL;
	WCHAR *wszInsertStatement = NULL;
	WCHAR *wszDropStatement = NULL;
	DB_UPARAMS	cParams=1;
	HACCESSOR	hLocalAccessor = DB_NULL_HACCESSOR;
	DB_UPARAMS	rgParamOrdinals[] = { 1 };
	DBBINDING	*rgDbBindInfo = NULL;
	DBPARAMBINDINFO *rgDbParamBindInfo = NULL;
	DB_LORDINAL	*rgParamColumnMap = NULL;
	DBROWCOUNT	cRowsAffected = 0;
	void		*pData = NULL;
	HRESULT		hr;
	DBCOUNTITEM	ulRowNum=0;
	DBPARAMS		DbParams;
	BOOL		fResult = FALSE;
	DBLENGTH	ulLength = 0;

	// Initialize.
	DbParams.pData = NULL;
	wszCreateStatement = (WCHAR *)m_pIMalloc->Alloc(SP_TEXT_BLOCK_SIZE);
	if (!wszCreateStatement)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}			
	

	wszInsertStatement = (WCHAR *)m_pIMalloc->Alloc(SP_TEXT_BLOCK_SIZE);

	if (!wszInsertStatement)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}			

	wszDropStatement = (WCHAR *)m_pIMalloc->Alloc(SP_TEXT_BLOCK_SIZE);
	if (!wszDropStatement)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}			


	wszCreateStatement[0] = NULL;
	wszInsertStatement[0] = NULL;
	wszDropStatement[0] = NULL;

	// First Create the Sql statement for the wType.
	if (!GetSqlBindInfo(pwszDataSourceType, wType, wszCreateStatement, wszInsertStatement, wszDropStatement,
			(DBBINDING **)&rgDbBindInfo, (DBPARAMBINDINFO **)&rgDbParamBindInfo, 
			(DBORDINAL **)&rgParamColumnMap, &hLocalAccessor))
	{
		odtLog << L"Couldn't find a matching type in the table for :" << pwszDataSourceType << L" " << wType << L"\n" ;
		// Let's not fail the test for that reason.
		fResult = TRUE;
		goto CLEANUP;
	}

	// First let's create the table.
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL, wszCreateStatement), S_OK);
	
	TEST_CHECK (m_pICommandText->Execute (NULL, IID_NULL, NULL, NULL, NULL), S_OK);


	hr = m_pICmdWParams->SetParameterInfo (cParams, rgParamOrdinals, rgDbParamBindInfo);
	
	if (! SUCCEEDED(hr))
		goto CLEANUP;

	// Use fill inputbindings to generate the data for insert.
	ulRowNum = NextInsertRowNum();

	// Fill input bindings.
	// We are using m_pTable pointer just to fill pData.  The actual table we are using
	// different.
	TEST_CHECK((hr = FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cParams,
			rgDbBindInfo, (BYTE **)&pData, ulRowNum, cParams, rgParamColumnMap)), S_OK);

	// First  Set the Insert command.
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL, wszInsertStatement), S_OK);

	DbParams.hAccessor = hLocalAccessor;
	DbParams.cParamSets = 1;
	DbParams.pData = pData;

	TESTC(	rgDbBindInfo->wType == DBTYPE_STR ||
			rgDbBindInfo->wType == DBTYPE_WSTR ||
			rgDbBindInfo->wType == DBTYPE_BYTES);

	switch(rgDbBindInfo->wType)
	{
		case DBTYPE_BYTES:
			ulLength = 20;
			break;
		case DBTYPE_STR:
		{
			DBLENGTH cch = 0;
			LPSTR pszStr = (LPSTR)((BYTE *)pData+rgDbBindInfo->obValue);
			ulLength = 0;

			while(pszStr[ulLength] && cch < 20)
			{
				if (!IsDBCSLeadByte (pszStr[ulLength]))
					ulLength++;
				else
					ulLength+=2;

				cch++;
			}

			break;
		}
		case DBTYPE_WSTR:
			ulLength = 20*sizeof(WCHAR);
			break;
	}


	
	// Override the data generated by Private library and insert only 10 chars.
	*((DBLENGTH *)((BYTE *)pData + rgDbBindInfo->obLength)) = ulLength;
	TEST_CHECK (m_pICommandText->Execute (NULL, IID_NULL, &DbParams, &cRowsAffected, NULL), S_OK);


	fResult = TRUE;
CLEANUP:

	// First let's drop the table.
	if (wszDropStatement && wszDropStatement[0])
	{
		CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL, wszDropStatement), S_OK);
	
		CHECK (m_pICommandText->Execute (NULL, IID_NULL, NULL, NULL, NULL), S_OK);
	}

	if (DbParams.pData)
		ReleaseInputBindingsMemory(1, rgDbBindInfo, (BYTE *)DbParams.pData, TRUE);
	
	FREE_DATA (rgDbParamBindInfo->pwszDataSourceType);
	FREE_DATA (rgDbParamBindInfo);
	FREE_DATA (rgParamColumnMap);
	FREE_DATA (rgDbBindInfo);
	FREE_DATA (wszCreateStatement);
	FREE_DATA (wszInsertStatement);
	FREE_DATA (wszDropStatement);

	return fResult;

}
//---------------------------------------------------------------------
//@mfunc For GetParameterInfo.  Compares information returned by
//	GetParameterInfo with that of column information.  
//---------------------------------------------------------------------

HRESULT CICmdWParams::CompareDBParamInfo(
	DBCOUNTITEM		cCols,			// [in]	 Number of columns in rgColumnsInfo.  This is expected to match cParams
	DBORDINAL *		prgParamColMap,	// [in]  Parameter to column mapping array
	DB_UPARAMS		cParams, 		// [in]  Number of parameter information to compare
	DBPARAMINFO *	rgParamInfo,	// [in]  Array of DBPARAMINFO containing parameter info
	LPOLESTR pNamesBuffer			// [in]  Names buffer from GetParameterInfo.
	)
{
	ULONG iCol; 
	DBLENGTH cbRowSize = 0;
	BOOL bResult = FALSE;
	DBBINDING * pBinding = NULL;
	DBPARAMBINDINFO * pParamBindInfo = NULL;
	ParamStruct * pParamStruct = NULL;
	DB_UPARAMS * prgParamOrdinals = NULL;
	HRESULT hr = E_FAIL;

	// If no columns are expected, verify null out params
	if (!cCols)
	{
		TESTC(cParams == 0);
		TESTC(rgParamInfo == NULL);
		TESTC(pNamesBuffer == NULL);
		return S_OK;
	}

	TEST_ALLOC(DBBINDING, pBinding, 0, (size_t)(sizeof(DBBINDING) * cCols));
	TEST_ALLOC(DBPARAMBINDINFO, pParamBindInfo, 0, (size_t)(sizeof(DBPARAMBINDINFO) * cCols));
	TEST_ALLOC(ParamStruct, pParamStruct, 0, (size_t)(sizeof(ParamStruct) * cCols));
	TEST_ALLOC(DB_UPARAMS, prgParamOrdinals, 0, (size_t)(sizeof(DB_UPARAMS) * cCols));

	// Compare the given Parameter information against the columns information 
	// gotten by ExecuteCommand. In this case cParams should match cColumns.
	for (iCol = 0; iCol < cCols; iCol++)
	{
		prgParamOrdinals[iCol] = iCol+1;
		TESTC(AddParam(iCol, prgParamColMap[iCol], DBPARAMIO_INPUT, NULL, FALSE, &cbRowSize, pBinding, pParamBindInfo,  pParamStruct, m_pTable))
	}

	// Note that VerifyParamInfo posts it's own failures, so posting a failure here will cause redundant postings
	hr = S_OK;	// If we made it to here we need to return S_OK since VerifyParamInfo now posts it's own failures
	VerifyParamInfo(cCols, prgParamOrdinals, pParamBindInfo, cParams, rgParamInfo, pNamesBuffer);

CLEANUP:

	PROVIDER_FREE(pBinding);
	PROVIDER_FREE(pParamBindInfo);
	PROVIDER_FREE(pParamStruct);
	PROVIDER_FREE(prgParamOrdinals);

	return hr;
}

//---------------------------------------------------------------------------
//	CCommand::IsColDateTime 
//
//	@mfunc	BOOL			|
//			CCommand		|
//			IsColDateTime	|
//			Can the data type hold DateTime values?
//
//
//---------------------------------------------------------------------------
BOOL CICmdWParams::IsColDateTime			
(
	DBTYPE ProviderType		// @parm [IN] provider data type
)
{
	if (ProviderType & DBTYPE_BYREF)
		ProviderType &= ~DBTYPE_BYREF;

	switch(ProviderType)
	{
			case DBTYPE_DATE:			// OLE Auto. Date
			case DBTYPE_DBDATE:			// Date
			case DBTYPE_DBTIME:			// Time
			case DBTYPE_DBTIMESTAMP:	// TimeStamp
				return TRUE;
		default:
				return FALSE;	// Compiler needs this
	}
}
//---------------------------------------------------------------------------
//	CCommand::IsColI2 
//
//	@mfunc	BOOL			|
//			CCommand		|
//			IsColI2	|
//			Can the data type hold numeric values?
//
//
//---------------------------------------------------------------------------
BOOL CICmdWParams::IsColI2			
(
	DBTYPE ProviderType		// @parm [IN] provider data type
)
{
	if (ProviderType & DBTYPE_BYREF)
		ProviderType &= ~DBTYPE_BYREF;

	switch(ProviderType)
	{
		case DBTYPE_I2:	//I2
				return TRUE;
		default:
				return FALSE;	// Compiler needs this
	}
}

//---------------------------------------------------------------------------
//	CCommand::IsColNumeric 
//
//	@mfunc	BOOL			|
//			CCommand		|
//			IsColNumeric	|
//			Can the data type hold numeric values?
//
//
//---------------------------------------------------------------------------
BOOL CICmdWParams::IsColNumeric			
(
	DBTYPE ProviderType		// @parm [IN] provider data type
)
{
	if (ProviderType & DBTYPE_BYREF)
		ProviderType &= ~DBTYPE_BYREF;

	switch(ProviderType)
	{
		case DBTYPE_NUMERIC:	// Numeric, Decimal
				return TRUE;
		default:
				return FALSE;	// Compiler needs this
	}
}

// Returns count of character in string
ULONG wcschcount(LPWSTR pwsz, WCHAR ch)
{
	ULONG cch = 0;
	
	if (!pwsz)
		return 0;

	for (; *pwsz; pwsz++)
		if (*pwsz == ch)
			cch++;

	return cch;
}

BOOL IsErrorStatus(DBSTATUS sStatus)
{
	switch (sStatus)
	{
		case S_OK:
		case DBSTATUS_S_ISNULL:
		case DBSTATUS_S_TRUNCATED:
		case DBSTATUS_S_DEFAULT:
			return FALSE;
	}

	return TRUE;
}

// Returns the number of bytes required to display the given type
// as a WSTR.
DBLENGTH DisplaySize(CCol ColInfo)
{
	DBLENGTH ulDispSize = ColInfo.GetMaxSize();
	switch(ColInfo.GetProviderType())
	{
		case DBTYPE_DBTIMESTAMP:
			ulDispSize = (ulDispSize+6)*sizeof(WCHAR);
			break;
		case DBTYPE_BYTES:
			ulDispSize *= 2*sizeof(WCHAR);
			break;
		case DBTYPE_R4:
			// Precision plus sign plus decimal point plus 'E'+ sign + 2 dig exponent
			ulDispSize = (ulDispSize+6) * sizeof(WCHAR);
			break;
		case DBTYPE_CY:
		case DBTYPE_R8:
		case DBTYPE_NUMERIC:
		case DBTYPE_DECIMAL:
			// Precision plus sign plus decimal point plus 'E'+ sign + 3 dig exponent
			ulDispSize = (ulDispSize+7) * sizeof(WCHAR);
			break;
		case DBTYPE_VARNUMERIC:
			// Max precision plus sign plus decimal point plus 'E'+ sign + 3 dig exponent
			ulDispSize = (255+7) * sizeof(WCHAR);
			break;
		case DBTYPE_I1:
		case DBTYPE_I2:
		case DBTYPE_I4:
		case DBTYPE_I8:
			// Precision plus sign
			ulDispSize = (ulDispSize+1) * sizeof(WCHAR);
			break;
		case DBTYPE_BOOL:
			// Per spec Appendix
			ulDispSize = wcslen(L"FALSE")*sizeof(WCHAR);
			break;
		case DBTYPE_GUID:
			// Per spec Appendix
			ulDispSize = 38 * sizeof(WCHAR);
			break;
		case DBTYPE_DATE:
			// Per spec Appendix
			ulDispSize = 20 * sizeof(WCHAR);
			break;
		case DBTYPE_DBDATE:
			// Per spec Appendix
			ulDispSize = 10 * sizeof(WCHAR);
			break;
		case DBTYPE_FILETIME:
			// Per spec Appendix
			ulDispSize = 24 * sizeof(WCHAR);
			break;
		case DBTYPE_DBTIME:
			// Per spec Appendix
			ulDispSize = 8 * sizeof(WCHAR);
			break;
		default:
			ulDispSize *= sizeof(WCHAR);
	}

	return ulDispSize;
}

// Format a string using replacable %1 parameters, allocating memory required
size_t FormatString(WCHAR ** ppwszDest, WCHAR * pwszFmt, ULONG cArgs, ...)
{
	size_t ulStringLen=0;
	ULONG iArg;
	WCHAR ** ppwszParamArray=NULL;

	va_list varargs;

	va_start(varargs, cArgs);

	if (!pwszFmt || !ppwszDest)
		return 0;
	
	if (cArgs)
	{
		// Allocate an array of wide chars large enough to hold the variable arguments
		TEST_ALLOC(WCHAR *, ppwszParamArray, 0, cArgs * sizeof(WCHAR *));

		for (iArg = 0; iArg < cArgs; iArg++)
			ppwszParamArray[iArg] = va_arg(varargs, WCHAR *);
		va_end(varargs);
	}

	ulStringLen = FormatStringFromArray(ppwszDest, pwszFmt, cArgs, ppwszParamArray);

CLEANUP:

	PROVIDER_FREE(ppwszParamArray);

	return ulStringLen;
}

// Format a string using replacable %1 parameters, allocating memory required
size_t FormatStringFromArray(WCHAR ** ppwszDest, WCHAR * pwszFmt, ULONG cArgs, WCHAR ** ppInsertArray)
{
	size_t ulStringLen = 0, cch=0;
	ULONG cInserts=0, iIns, ulInsNo;
	WCHAR * pwszIns = NULL;
	WCHAR * pwszPart = pwszFmt;

	ASSERT(ppwszDest && pwszFmt);
	ASSERT(cArgs == 0 || (cArgs > 0 && ppInsertArray));

	*ppwszDest = NULL;

	if (!pwszFmt )
		goto CLEANUP;

	pwszIns = wcsstr(pwszFmt, L"%");

	// If no arguments or no %n params then we just return the format string
	if (cArgs == 0 || !pwszIns)
	{
		*ppwszDest = wcsDuplicate(pwszFmt);
		if (*ppwszDest)
			ulStringLen = wcslen(pwszFmt)*sizeof(WCHAR);
		return ulStringLen;
	}

	// Find out space needed for destination string
	cch = wcslen(pwszFmt)+1;
	for (iIns=0; iIns < cArgs; iIns++)
	{
		ASSERT(ppInsertArray[iIns]);
		cch += wcslen(ppInsertArray[iIns]);
	}

	// Allocate space
	SAFE_ALLOC(*ppwszDest, WCHAR, cch*sizeof(WCHAR));

	// Start with an empty string
	*ppwszDest[0] = L'\0';

	for (iIns = 0; iIns < cArgs && pwszIns; iIns++)
	{
		// Found a % character, replace with a null term
		*pwszIns = L'\0';

		// Get the number associated with the %
		ulInsNo = _wtol(pwszIns+1);

		// If it wasn't a number after the % or it has a leading 0 then ignore it
		if (ulInsNo && *(pwszIns+1) != L'0')
		{
			WCHAR wszInsNo[MAX_LTOW];

			cInserts++;

			// We try to be nicer than FormatMessage and make sure the count of inserts matches cArgs
			ASSERT(cInserts <= cArgs);

			if (cInserts > cArgs)
				goto CLEANUP;

			// Can't have an insert number greater than number of insert array elements
			if (ulInsNo > cArgs)
				goto CLEANUP;

			_ltow(ulInsNo, wszInsNo, 10);

			// Copy the part from the format string
			wcscat(*ppwszDest, pwszPart);

			// Copy the insert string
			wcscat(*ppwszDest, ppInsertArray[ulInsNo-1]);

			pwszPart = pwszIns+1+wcslen(wszInsNo);

		}

		// Put back the % to avoid side effects
		*pwszIns = L'%';

		// Find the next insert point
		pwszIns = wcsstr(pwszIns+1, L"%");
	}

	// Copy any remaining part of the format string
	wcscat(*ppwszDest, pwszPart);

	ulStringLen = wcslen(*ppwszDest) * sizeof(WCHAR);

CLEANUP:

	if (pwszIns)
		*pwszIns = L'%';

	if (!ulStringLen)
		SAFE_FREE(*ppwszDest);

	return ulStringLen;
}

BOOL CICmdWParams::bHasReturnParam(enum TOKEN_ENUM eProcType)
{
	switch (eProcType)
	{
		case T_EXEC_PROC_SELECT_OUT_RET:
		case T_EXEC_PROC_SELECT_INOUT_RET:
		case T_EXEC_PROC_UPDATE_INPUT_RET:
		case T_EXEC_PROC_INSERT_INPUT_RET:
		case T_EXEC_PROC_DELETE_RET:
			return TRUE;
	}

	return FALSE;
}


HRESULT CICmdWParams::CreateStoredProc(ICommandText * pICommandText, WCHAR * pwszProcName, WCHAR * pwszCreateStmt, BOOL fIsFunction)
{
	HRESULT hr = S_OK;
	CHAR * pszCreateStmt = NULL;

	// Drop the proc if we were passed the name
	if (pwszProcName)
		DropStoredProcedure(pICommandText, pwszProcName, fIsFunction);

	if (CHECK(hr = pICommandText->SetCommandText(DBGUID_DBSQL, pwszCreateStmt), S_OK))
		hr = pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL);

	return hr;
}

HRESULT CICmdWParams::SetParameterInfoIfNeeded(DB_UPARAMS cParams, DB_UPARAMS * pParamOrdinals,
	DBPARAMBINDINFO * pParamBindInfo, ICommand * pICommand)
{
	ICommandWithParameters * pICmdWParams = NULL;
	DBPARAMINFO * pParamInfo = NULL;
	DB_UPARAMS cParamsDerived = 0;
	WCHAR * pNamesBuffer = NULL;
	HRESULT hr = E_FAIL;

	// Use the member ICommand if we didn't specify one
	if (!pICommand)
		pICommand = m_pICommand;

	// Get the Interface pointer for ICommandWithparameters Object.
	TEST_COMPARE(VerifyInterface(pICommand, IID_ICommandWithParameters,
			COMMAND_INTERFACE,(IUnknown **)&pICmdWParams), TRUE);

	// Remove any old parameter information that might be hanging around.
	TEST_CHECK(pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Call GetParameterInfo
	hr = pICmdWParams->GetParameterInfo(&cParamsDerived, &pParamInfo, &pNamesBuffer);

	if (hr == S_OK)
		goto CLEANUP;
	else if (hr == DB_E_PARAMUNAVAILABLE)
		hr = pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pParamBindInfo);
	else
		CHECK(hr, S_OK);

CLEANUP:

	PROVIDER_FREE(pParamInfo);
	PROVIDER_FREE(pNamesBuffer);
	SAFE_RELEASE(pICmdWParams);

	return hr;
}

HRESULT CICmdWParams::PrepareForExecute(WCHAR * pwszExecuteStmt, ULONG cParams, DB_UPARAMS * pParamOrdinals,
	DBPARAMBINDINFO * pPARAMBIND, BOOL * pfCanDerive, WCHAR * pwszProcName, WCHAR * pwszCreateProcStmt,
	BOOL fIsFunction)
{
	HRESULT hr;
	BOOL fResult = TRUE;
	BOOL fCanDerive = FALSE;

	if (!pfCanDerive)
		pfCanDerive = &fCanDerive;

	if (pwszCreateProcStmt)
		// Create the stored procedure
		ABORT_CHECK(hr = CreateStoredProc(m_pICommandText, pwszProcName, pwszCreateProcStmt, fIsFunction), S_OK);

	// Remove any old parameter information
	FAIL_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set the command text to execute the stored proc
	ABORT_CHECK(hr = m_pICommandText->SetCommandText(DBGUID_DBSQL, pwszExecuteStmt), S_OK);

	// Prepare if supported
	if (m_pICommandPrepare)
	{
		CHECK(hr = m_pICommandPrepare->Prepare(1), S_OK);
		if (FAILED(hr))
		{
			fResult = FALSE;
			goto CLEANUP;
		}
	}

	// Providers that can derive parameter information return DB_S_TYPEINFOOVERRIDDEN here
	// Note that some providers can derive information for some types of statements but no
	// others, so we check each time.
	hr = m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND);

	// If the provider returns DB_S_TYPEINFOOVERRIDDEN then we assume he can derive.
	// We also assume he can derive for no parameters at all, since it's a no-op.
	if (hr == DB_S_TYPEINFOOVERRIDDEN || !cParams)
		*pfCanDerive = TRUE;
	else
	{
		*pfCanDerive = FALSE;
		FAIL_CHECK(hr, S_OK);
	}

	// Now reset the parameter information again so we can let the provider derive (or fail)
	FAIL_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);


CLEANUP:

	return fResult ? S_OK : E_FAIL;

}

HRESULT CICmdWParams::ValidateGetParameterInfo(ULONG cExpParams, ULONG cParamSets, DBCOUNTITEM ulRowNum, DB_UPARAMS * pExpOrdinals,
	DBPARAMBINDINFO * pExpParamBind, DBBINDING * pBINDING, DBLENGTH cbRowSize, ParamStruct * pParamAll,
	BYTE * pData, enum ROWSET_ENUM eRowset, DBORDINAL cColumns, DB_LORDINAL * rgColumns, BOOL fCanDerive, 
	enum VERIFY_ENUM eVerifyMethod,	ICommand * pICommand)
{
	HRESULT hrGet = E_FAIL;
	HRESULT hrSet = S_OK;
	HRESULT hr = E_FAIL;
	DB_UPARAMS cParams = 0;
	DBPARAMINFO * pParamInfo = NULL;
	WCHAR * pNamesBuffer = NULL;
	BOOL fResult = TRUE;
	ICommandWithParameters * pICmdWParams = NULL;
	HRESULT hrGetExpected = DB_E_PARAMUNAVAILABLE;	// Assuming provider can't derive parameter info
	BYTE * pDataCopy = NULL;

	if (fCanDerive)
		hrGetExpected = S_OK;

	// Use the member command object if we didn't pass in one to use
	if (!pICommand)
		pICommand = m_pICommand;

	// Get the Interface pointer for ICommandWithparameters Object.
	ABORT_COMPARE(VerifyInterface(pICommand, IID_ICommandWithParameters,
			COMMAND_INTERFACE,(IUnknown **)&pICmdWParams), TRUE);

	// Some providers can derive parameter information for stored procs but not statements
	// so this call could succeed even though we think parameter derivation isn't supported.
	hrGet = pICmdWParams->GetParameterInfo(&cParams, &pParamInfo, &pNamesBuffer);

	FAIL_CHECK(hrGet, hrGetExpected);
	
	if (SUCCEEDED(hrGet))
	{
		// We were able to derive parameter information, see if it looks right
		FAIL_VAR(VerifyParamInfo(cExpParams, pExpOrdinals, pExpParamBind,
			cParams, pParamInfo, pNamesBuffer, 0, TRUE), S_OK);

		// Free the buffers we got from GetParameterInfo
		SAFE_FREE(pParamInfo);
		SAFE_FREE(pNamesBuffer);

		// Save the values in pData so we can replace them after output params are verified.
		// For the case of I/O params we expect a second Execute to succeed using the pData
		// we passed in, but the Output has overwritten the input values.  Also, for non-I/O
		// params this puts back the pData values that have the output param buffers NULL'd.
		TEST_ALLOC(BYTE, pDataCopy, 0, (size_t)(cbRowSize*cParamSets*sizeof(BYTE)));
		memcpy(pDataCopy, pData, (size_t)(cbRowSize*cParamSets*sizeof(BYTE)));

		// The final proof is that we can execute with these values
		FAIL_VAR(ExecuteAndVerify(cExpParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, eRowset, 
			cColumns, rgColumns, eVerifyMethod, TRUE, pICommand), S_OK);

		// Get it again so to make sure the bogus values got set
		ABORT_CHECK(pICmdWParams->GetParameterInfo(&cParams, &pParamInfo, &pNamesBuffer), S_OK);

		// Check the param info after execute 
		FAIL_VAR(VerifyParamInfo(cExpParams, pExpOrdinals, pExpParamBind,
			cParams, pParamInfo, pNamesBuffer, 0, TRUE), S_OK);

		// Put pData back the way it was for further Execute calls
		if (eVerifyMethod != VERIFY_NONE)
			memcpy(pData, pDataCopy, (size_t)(cbRowSize*cParamSets*sizeof(BYTE)));

		PROVIDER_FREE(pDataCopy);

		// Free the buffers we got from GetParameterInfo
		PROVIDER_FREE(pParamInfo);
		PROVIDER_FREE(pNamesBuffer);

		// If there are parameters, then the provider will return DB_S_TYPEINFOOVERRIDDEN
		// if we set param info.
		if (cExpParams)
			hrSet = DB_S_TYPEINFOOVERRIDDEN;

	}

	if (FAILED(hrGet))
	{
		odtLog << L"Warning: Provider was not able to derive parameter information.\n\n";

		hrSet = S_OK;
	}

	SAFE_FREE(pParamInfo);
	SAFE_FREE(pNamesBuffer);

	if (cExpParams > 1)
	{
		HRESULT hr = E_FAIL;

		// Set the parameter information to something different so we can verify we can over-ride

		// Note some providers goof the DB_S_TYPEINFOOVERRIDDEN and instead return S_OK here.  
		hr = pICmdWParams->SetParameterInfo(1, &pExpOrdinals[0], &pExpParamBind[1]);

		// For this success case we will supress duplicate failures
		if (hrSet == DB_S_TYPEINFOOVERRIDDEN && hr == S_OK)
		{
			CCHECK(m_EC, hr, hrSet, 
						EC_UNEXPECTED_S_OK,
						L"Provider returned S_OK rather than required DB_S_TYPEINFOOVERRIDDEN",
						FALSE);
		}
		// Otherwise we will report a non-suppressed error
		else
		{
			FAIL_CHECK(hr, hrSet);
		}

		// Get it again so to make sure the bogus values got set
		ABORT_CHECK(pICmdWParams->GetParameterInfo(&cParams, &pParamInfo, &pNamesBuffer), S_OK);

		// Verify the bogus information
		// Note that due to spec change providers are allowed to return all parameters even though
		// I set only one, so don't fail them for that.

		if (COMPARE(cParams > 0, TRUE))
			FAIL_VAR(VerifyParamInfo(1, &pExpOrdinals[0], &pExpParamBind[1],
				1, pParamInfo, pNamesBuffer), S_OK);

		if (cParams > 1)
			FAIL_VAR(VerifyParamInfo(cExpParams-1, &pExpOrdinals[1], &pExpParamBind[1],
				cParams-1, &pParamInfo[1], pNamesBuffer, 1), S_OK);

		// Since we called SetParameterInfo once above this is now the expected return from any
		// other SetParameterInfo calls.
		hrSet = DB_S_TYPEINFOOVERRIDDEN;

		// Free the buffers we got from GetParameterInfo
		PROVIDER_FREE(pParamInfo);
		PROVIDER_FREE(pNamesBuffer);
	}

	// Now set the parameter information correctly
	ABORT_CHECK(pICmdWParams->SetParameterInfo(cExpParams, pExpOrdinals, pExpParamBind), hrSet);

	// Get it again so we can validate we get back what we set.
	ABORT_CHECK(pICmdWParams->GetParameterInfo(&cParams, &pParamInfo, &pNamesBuffer), S_OK);

	// Verify results.  If we didn't get back what was set it might not be a failure
	FAIL_VAR(VerifyParamInfo(cExpParams, pExpOrdinals, pExpParamBind,
			cParams, pParamInfo, pNamesBuffer), S_OK);

	SAFE_FREE(pParamInfo);
	SAFE_FREE(pNamesBuffer);

	// The final proof is that we can execute with these values
	FAIL_VAR(ExecuteAndVerify(cExpParams, cParamSets, pParamAll, ulRowNum, pBINDING, cbRowSize, pData, eRowset, 
		cColumns, rgColumns, eVerifyMethod, TRUE, pICommand), S_OK);

	// Get it again so we can validate we get back what we set.
	ABORT_CHECK(pICmdWParams->GetParameterInfo(&cParams, &pParamInfo, &pNamesBuffer), S_OK);

	// Check the param info after execute 
	FAIL_VAR(VerifyParamInfo(cExpParams, pExpOrdinals, pExpParamBind,
		cParams, pParamInfo, pNamesBuffer), S_OK);

CLEANUP:

	SAFE_FREE(pParamInfo);
	SAFE_FREE(pNamesBuffer);
	SAFE_RELEASE(pICmdWParams);

	return fResult ? S_OK : E_FAIL;
}

HRESULT CICmdWParams::VerifyOutParams(ULONG cParams, ULONG cParamSets, DBCOUNTITEM ulRowNum, DBBINDING * pBinding,
	DBLENGTH cbRowSize, BYTE * pData, ParamStruct * pParamAll, enum VERIFY_ENUM eVerifyMethod, DBBINDING * pBindMatch,
	BYTE * pDataMatch)
{
	BOOL fResult = TRUE;
	ULONG iParam, cOutParams = 0; 
	DBCOUNTITEM iRow;
	DBORDINAL cCols = 0;
	DB_LORDINAL * rgColumns = NULL;
	DBBINDING * pOutBind = NULL;

	// Currently don't support multiple paramsets on output params
//	FAIL_COMPARE(cParamSets, 1);

	// Now depending on the comparison method we verify the data
	if (eVerifyMethod == VERIFY_USE_TABLE)
	{
		fResult = FALSE;	// In case of allocation failure

		// If allocation fails we'll return E_FAIL
		TEST_ALLOC(DB_LORDINAL, rgColumns, 0, sizeof(DB_LORDINAL)*cParams);
		TEST_ALLOC(DBBINDING, pOutBind, 0, sizeof(DBBINDING)*cParams);

		// From now on assume success
		fResult = TRUE;

		// Go through the parameter mapping array and find all the output params
		for (iParam = 0; iParam < cParams; iParam++)
		{
			if (pParamAll[iParam].eParamIO & DBPARAMIO_OUTPUT)
			{
				// It's an output param, copy the binding information 
				memcpy(&pOutBind[cOutParams], &pBinding[iParam], sizeof(DBBINDING));
				// Copy the column map for CompareData
				if (pBinding[iParam].iOrdinal > cCols)
					cCols = pBinding[iParam].iOrdinal;
				rgColumns[pBinding[iParam].iOrdinal-1]=pParamAll[iParam].ulColIndex;
				cOutParams++;
			}
		}

		// If there are no output params then we just return pass
		if (!cOutParams)
			goto CLEANUP;

		// The output param rows match data in the table
		for (iRow = ulRowNum; iRow < ulRowNum + cParamSets; iRow++)
		{
			FAIL_COMPARE(CompareData(
							cCols,
							rgColumns,
							iRow,
							pData+(iRow - ulRowNum)*cbRowSize,
							cOutParams,
							pOutBind,
							m_pTable,
							m_pIMalloc,
							PRIMARY,
							COMPARE_ONLY,
							COMPARE_ALL,
							TRUE
						), TRUE);

		}

	}
	else if (eVerifyMethod == VERIFY_USE_PDATA)
	{
		// By default we assume the second row of data in pData contains the
		// expected values and the bindings are identical.
		if (!pBindMatch)
			pBindMatch = pBinding;
		if (!pDataMatch)
			pDataMatch = pData+cbRowSize;

		// The output param rows match the second row in pdata
		for (iParam = 0; iParam < cParams; iParam++)
		{
			BOOL fCompareData = FALSE;

			DBSTATUS ulStatusOut = DBSTATUS_E_BADSTATUS;
			DBSTATUS ulStatusExp = DBSTATUS_E_BADSTATUS;

			DBLENGTH ulLengthOut = 0;
			DBLENGTH ulLengthExp = 0;

			BYTE * pValueOut = NULL;
			BYTE * pValueExp = NULL;

			// Extract status, length, and value
			if (pBinding[iParam].dwPart & DBPART_STATUS)
				ulStatusOut = *(DBSTATUS *)(pData+pBinding[iParam].obStatus);
			if (pBindMatch[iParam].dwPart & DBPART_STATUS)
				ulStatusExp = *(DBSTATUS *)(pDataMatch+pBindMatch[iParam].obStatus);

			if (pBinding[iParam].dwPart & DBPART_LENGTH)
				ulLengthOut = *(DBLENGTH *)(pData+pBinding[iParam].obLength);
			if (pBindMatch[iParam].dwPart & DBPART_LENGTH)
				ulLengthExp = *(DBLENGTH *)(pDataMatch+pBindMatch[iParam].obLength);
	
			if (pBinding[iParam].dwPart & DBPART_VALUE)
				pValueOut = pData+pBinding[iParam].obValue;
			if (pBindMatch[iParam].dwPart & DBPART_VALUE)
				pValueExp = pDataMatch+pBindMatch[iParam].obValue;

			// First make sure the status matches if bound
			if ((pBinding[iParam].dwPart & DBPART_STATUS) &&
				(pBindMatch[iParam].dwPart & DBPART_STATUS))
				FAIL_COMPARE(ulStatusOut, ulStatusExp);
			
			// If either status is bound we can compare the data.  If not, we'll fall back
			// and use the eCompareOp to decide if it's NULL.
			if ((pBinding[iParam].dwPart & DBPART_STATUS) ||
				(pBindMatch[iParam].dwPart & DBPART_STATUS))
			{
				if (((pBinding[iParam].dwPart & DBPART_STATUS) && ulStatusOut == DBSTATUS_S_OK) ||
					((pBindMatch[iParam].dwPart & DBPART_STATUS) && ulStatusExp == DBSTATUS_S_OK))
						fCompareData = TRUE;
			}
			else if (pParamAll[iParam].eCompareOp != CP_ISNULL)
				fCompareData = TRUE;

			if (fCompareData)
			{
				// We can compare the length, but note length for input
				// only params should be ignored.
				if ((pBinding[iParam].dwPart & DBPART_LENGTH) &&
					(pBindMatch[iParam].dwPart & DBPART_LENGTH) &&
					pBinding[iParam].eParamIO & DBPARAMIO_OUTPUT)
					FAIL_COMPARE(ulLengthOut, ulLengthExp);

				// If the value part was bound then we should be able to compare it.
				// If length wasn't bound then we can only compare fixed-length or char 
				// types.
				if ((pBinding[iParam].dwPart & DBPART_VALUE) &&
					(pBindMatch[iParam].dwPart & DBPART_VALUE))
				{
					DBLENGTH ulLength = 0;
					DBTYPE wType = pBinding[iParam].wType;

					// If we've got a BYREF binding then pValue is a pointer to the value, 
					// not the value itself.
					if (wType & DBTYPE_BYREF)
					{
						wType &= ~DBTYPE_BYREF;
						pValueOut = (BYTE *)*(LPVOID *)pValueOut;
					}
					if (pBindMatch[iParam].wType & DBTYPE_BYREF)
						pValueExp = (BYTE *)*(LPVOID *)pValueExp;

					if (pBindMatch[iParam].dwPart & DBPART_LENGTH)
						ulLength = ulLengthExp;
					else if (pBinding[iParam].dwPart & DBPART_LENGTH)
						ulLength = ulLengthOut;
					else if (IsFixedLength(pBinding[iParam].wType))
						ulLength = GetDBTypeSize(pBinding[iParam].wType);
					else if (pBinding[iParam].wType == DBTYPE_STR)
						ulLength = strlen((CHAR *)pValueExp);
					else if (pBinding[iParam].wType == DBTYPE_WSTR)
						ulLength = wcslen((WCHAR *)pValueExp);

					// If we know the length, compare the value.  We should always know
					// the length.
					ASSERT(ulLength);

					if (ulLength)
						FAIL_COMPARE(RelativeCompare(pValueOut, pValueExp, wType, (USHORT)ulLength, 
							pBinding[iParam].bPrecision, pBinding[iParam].bScale, (ULONG)ulLength), 0);
				}
			}
		}
	}
	else if (eVerifyMethod == VERIFY_NULL)
	{
		for (iParam = 0; iParam < cParams; iParam++)
		{
			if ((pBinding[iParam].eParamIO & DBPARAMIO_OUTPUT) && (pBinding[iParam].dwPart & DBPART_STATUS))
			{
				DBSTATUS ulStatusOut = *(DBSTATUS *)(pData+pBinding[iParam].obStatus);
				COMPARE(ulStatusOut, DBSTATUS_S_ISNULL);
			}
		}
	}
	else if (eVerifyMethod == VERIFY_NONE)
		NULL;	// No verification
	else
		ASSERT(!L"Invalid verify method in VerifyOutParams.");

CLEANUP:

	PROVIDER_FREE(rgColumns);
	PROVIDER_FREE(pOutBind);

	return fResult ? S_OK : E_FAIL;
}

HRESULT CICmdWParams::ExecuteAndVerify(ULONG cParams, ULONG cParamSets, ParamStruct * pParamAll,
		DBCOUNTITEM ulRowNum,	DBBINDING * pBINDING, DBLENGTH cbRowSize, BYTE * pData, enum ROWSET_ENUM eRowset,
		DBORDINAL cColumns, DB_LORDINAL * rgColumns, enum VERIFY_ENUM eVerifyMethod, BOOL fRelease, 
		ICommand * pICommand, HRESULT hrExpected,DBBINDING * pBindMatch, BYTE * pDataMatch,
		DBCOUNTITEM * pcRowsExpected)
{
	BOOL fResult = TRUE;
	HACCESSOR hParamAccessor = DB_INVALID_HACCESSOR;
	DBPARAMS ExecDbParams;
	DBPARAMS * pParams = NULL;
	DBROWCOUNT cRowsAffected;
	IRowset * pRowset = NULL;
	IAccessor * pCmdIAccessor = NULL;
	HRESULT hr = E_FAIL;
	IRowset ** ppIRowset = &pRowset;
	IID iid = m_iidExec;

	// Null out DBPARAMS
	memset(&ExecDbParams, 0, sizeof(DBPARAMS));

	if (!g_fRowsetTest && eRowset == ROWSET_NONE)
	{
		ppIRowset = NULL;
		iid = IID_NULL;
	}

	// Use the member command object if we didn't pass in one to use
	if (!pICommand)
		pICommand = m_pICommand;

	if (cParams)
	{
		//  Get the interface pointer for Accessor.
		if (!VerifyInterface(pICommand, IID_IAccessor,
				COMMAND_INTERFACE,(IUnknown **)&pCmdIAccessor))
		{
			goto CLEANUP;
		}

		// Now create the parameter accessor
		ABORT_CHECK (pCmdIAccessor->CreateAccessor( DBACCESSOR_PARAMETERDATA,
			cParams, pBINDING, cbRowSize, &hParamAccessor, NULL), S_OK);

		// Set up our param accessor
		ExecDbParams.hAccessor = hParamAccessor;

		pParams = &ExecDbParams;
	}
	
	// The rest of these should be ignored
	ExecDbParams.cParamSets = cParamSets;
	ExecDbParams.pData = pData;

	hr = pICommand->Execute(NULL, iid, pParams, 
		&cRowsAffected, (IUnknown **)ppIRowset);

	if (hr != hrExpected && (hr == DB_E_ERRORSOCCURRED || hr == DB_S_ERRORSOCCURRED))
	{
		// Dump current command props
		DumpCommandProps(pICommand, FALSE);
		// Dump properties in error
		DumpCommandProps(pICommand, TRUE);
	}

	// Make sure we got the result we expect, but don't fail a provider if they can execute
	// and get the data we expect (bonus functionality)
	if (FAILED(hr))
	{
		// If we failed, then exit.  For success codes this will allow
		// us to validate the data returned in case it's only one parameter failing.
		CHECK(hr, hrExpected);
		goto CLEANUP;
	}
	else
		CHECK(hr, S_OK);

	// Validate the rowset pointer returned
	switch (eRowset)
	{
		case ROWSET_MAYBE:
			if (!pRowset)
				odtLog << L"Warning: This command did not produce a rowset.  Some providers cannot produce rowsets here.\n\n";
			break;
		case ROWSET_NONE:
			FAIL_COMPARE(pRowset, NULL);
			break;
		case ROWSET_ALWAYS:
			FAIL_COMPARE(pRowset != NULL, TRUE);
			break;
		default:
			ASSERT(!L"Unknown rowset enum value.");
	}

	// If the provider claims output params are available immediately after Execute verify them here
	if (g_ulOutParamsSupported == DBPROPVAL_OA_ATEXECUTE)
		FAIL_VAR(VerifyOutParams(cParams, cParamSets, ulRowNum, pBINDING, cbRowSize, pData, pParamAll,
			eVerifyMethod, pBindMatch, pDataMatch), S_OK);

	// If a rowset was returned validate the data
	if (pRowset)
		FAIL_VAR(VerifyObj(m_iidExec, pRowset, ulRowNum, cColumns, rgColumns, fRelease, FALSE,
			NULL, pcRowsExpected), S_OK);

	if (g_ulOutParamsSupported == DBPROPVAL_OA_ATROWRELEASE)
		FAIL_VAR(VerifyOutParams(cParams, cParamSets, ulRowNum, pBINDING, cbRowSize, pData, pParamAll,
			eVerifyMethod, pBindMatch, pDataMatch), S_OK);

CLEANUP:

	if (hParamAccessor != DB_INVALID_HACCESSOR)
		FAIL_CHECK(pCmdIAccessor->ReleaseAccessor(hParamAccessor, NULL), S_OK);

	SAFE_RELEASE(pCmdIAccessor);

	return fResult ? S_OK : E_FAIL;
}

HRESULT CICmdWParams::VerifyObj(REFIID riidRowset, IUnknown * pUnkRowset, DBCOUNTITEM ulStartingRow,
	DBORDINAL cRowsetCols, DB_LORDINAL * rgTableColOrds, BOOL fRelease, BOOL fRestart, CTable * pTable,
	DBCOUNTITEM * pcRows)
{
	if (!pTable)
		pTable = m_pTable;

	if (riidRowset == IID_IRow)
		return VerifyRowObj(riidRowset, pUnkRowset, ulStartingRow,
			cRowsetCols, rgTableColOrds, fRelease, pTable);
	else
		return VerifyRowset(riidRowset, pUnkRowset, ulStartingRow,
			cRowsetCols, rgTableColOrds, fRelease, fRestart, pTable,
			pcRows);
}

HRESULT CICmdWParams::VerifyRowObj(REFIID riidRow, IUnknown * pUnkRow, DBCOUNTITEM ulStartingRow,
	DBORDINAL cRowsetCols, DB_LORDINAL * rgTableColOrds, BOOL fRelease, CTable * pTable)
{
	BOOL fResult = FALSE;
	CRowObject*	pCRow = NULL;

	pCRow = new CRowObject();
	TESTC(pCRow != NULL)
	TESTC_(pCRow->SetRowObject(pUnkRow), S_OK)

	//Verify the GetColumns method.
	TESTC(pCRow->VerifyGetColumns(ulStartingRow, pTable, ALL_COLS_BOUND, BLOB_LONG, FORWARD,
		NO_COLS_BY_REF, DBTYPE_EMPTY, (DBORDINAL)cRowsetCols, (DBORDINAL *)rgTableColOrds))

	fResult = TRUE;

CLEANUP:
	SAFE_DELETE(pCRow);
	if (fRelease)
		SAFE_RELEASE(pUnkRow);
	return fResult ? S_OK : E_FAIL;
}

HRESULT CICmdWParams::VerifyRowset(REFIID riidRowset, IUnknown * pUnkRowset, DBCOUNTITEM ulStartingRow,
	DBORDINAL cRowsetCols, DB_LORDINAL * rgTableColOrds, BOOL fRelease, BOOL fRestart, CTable * pTable,
	DBCOUNTITEM * pcRows)
{
	BOOL fResult = TRUE;
	IRowset * pIRowset = NULL;
	IAccessor * pIAccessor = NULL;
	HRESULT hr = E_FAIL;
	DBCOUNTITEM cRowsObtained=0;
	HROW * prghRow = NULL;
	HACCESSOR hAccessor = DB_INVALID_HACCESSOR;
	DBBINDING * pBindings = NULL;
	DBCOUNTITEM cBindings = 0;
	DBLENGTH cbRowSize = 0;
	BYTE * pData = NULL;
	DBCOUNTITEM cRowsInRowset = 0;

	// If we didn't get passed the mapping of rowset cols to table cols we can't compare data
//	ABORT_COMPARE(cRowsetCols && rgTableColOrds && pUnkRowset, TRUE);

	if (!pTable)
		pTable = m_pTable;
	
	// Make sure we can get a rowset interface
	ABORT_COMPARE(VerifyInterface(pUnkRowset, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown **)&pIRowset), TRUE);

	// If we were passed in a rowset interface, use the one passed in
	if (riidRowset == IID_IRowset)
	{
		SAFE_RELEASE(pIRowset);
		pIRowset = (IRowset *)pUnkRowset;
	}

	// Make sure we're at the top of the rowset
	if (fRestart)
	{
		hr = pIRowset->RestartPosition(NULL);

		if (hr != DB_S_COMMANDREEXECUTED)
			ABORT_CHECK(hr, S_OK);
	}

	// Make sure we can get an accessor interface
	ABORT_COMPARE(VerifyInterface(pUnkRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown **)&pIAccessor), TRUE);

	ABORT_CHECK(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
		&hAccessor, &pBindings, &cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	TEST_ALLOC(BYTE, pData, 0, (size_t)(cbRowSize * sizeof(BYTE)));

	// TODO: Use IMultipleResults here if supported in case multiple results are returned??
	while (S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &prghRow)))
	{
		ABORT_COMPARE(cRowsObtained, 1);

		cRowsInRowset+=cRowsObtained;

		FAIL_CHECK(hr = pIRowset->GetData(*prghRow, hAccessor, pData), S_OK);

		if (SUCCEEDED(hr))
		{
			FAIL_COMPARE(CompareData(
							cRowsetCols,
							rgTableColOrds,
							ulStartingRow,
							pData,
							cBindings,
							pBindings,
							pTable,
							m_pIMalloc,
							PRIMARY,
							COMPARE_ONLY,
							COMPARE_ALL,
							TRUE
						), TRUE);

		
			FAIL_CHECK(pIRowset->ReleaseRows(cRowsObtained, prghRow, NULL, NULL, NULL), S_OK);

			ulStartingRow++;
		}
		else
			// If this GetData failed it's doubtful further GetData's are of much use
			goto CLEANUP;
	}

	FAIL_CHECK(hr, DB_S_ENDOFROWSET);

	if (pcRows)
	{
		if (*pcRows)
			FAIL_COMPARE(cRowsInRowset, *pcRows);

		*pcRows = cRowsInRowset;
	}

CLEANUP:

	if (hAccessor != DB_INVALID_HACCESSOR)
		FAIL_CHECK(pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);

	SAFE_RELEASE(pIAccessor);
	
	// If we passed in a rowset pointer then it gets release below if requested
	// otherwise release here.
	if (riidRowset != IID_IRowset)
		SAFE_RELEASE(pIRowset);

	if (fRelease)
		SAFE_RELEASE(pUnkRowset);

	PROVIDER_FREE(pData);
	PROVIDER_FREE(prghRow);

	return fResult ? S_OK : E_FAIL;
}


//--------------------------------------------------------------------------
// CreateProcBindings
//   Create a stored procedure and set up the member variables necessary 
//			for executing it
//	
// TODO: add support for scalar functions.  
// TODO: add support for other limit clauses besides "where" (select, etc.)
// TODO: add support for other comparison types ("in", "between",
//  "like", "<", ">")
// TODO: Handle DBBINDFLAG_HTML?
//--------------------------------------------------------------------------
BOOL CICmdWParams::CreateProcBindings(
	enum TOKEN_ENUM eProcType,			// [IN]  Proc type, regular proc or function (has return value)
	BOOL fBindByName,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
	ULONG cParamSets,					// [IN]	 Number of sets of parameters to be created
	DBTYPE wReturnType,					// [IN]  Return parameter type
	DBCOUNTITEM ulTableRow,					// [IN]  Row number in table or rowset to select, insert, or update
	ULONG * pcParams,					// [OUT] Count of params created
	DBLENGTH * pcbRowSize,				// [OUT] Count of bytes for a single row of parameters
	DBBINDING ** ppDBBINDINFO,			// [OUT] Binding array for CreateAccessor
	DB_UPARAMS ** ppParamOrdinals,		// [OUT] Array of cParams ordinals
	DBPARAMBINDINFO ** ppDBPARAMBINDINFO,// [OUT] rgParamBindInfo for SetParameterInfo
	WCHAR ** ppwszCreateStmt,			// [OUT] SQL stmt to create the stored proc
	WCHAR ** ppwszExecProcStmt,			// [OUT] SQL stmt to execute the stored proc
	WCHAR ** ppwszExecStmt,				// [OUT] SQL stmt to execute without stored proc
	WCHAR ** ppwszProcName,				// [OUT] Name of stored proc created
	BYTE ** ppData,						// [OUT] Pointer to data for the parameters
	ParamStruct ** ppParamAll,			// [OUT] Pointer to array of param structs to save name, column
	DBORDINAL * pcColumns,				// [OUT] Column count for rowset returned by proc or exec
	DB_LORDINAL ** prgColumnsOrd,		// [OUT] Array of mappings of rowset columns to underlying table columns
	DB_LORDINAL ** prgParamColOrd,		// [OUT] Array of mappings of parameters to underlying table columns
	ULONG ulCreateFlags					// [IN]  Flags used to control creation of parameters
)
{
	ULONG iCol, cParams=0, cColList=0, cParamList=0, cLimit=0, iParam, iParamList=0,
		iLimit=0, iParamRow, iDestParam, cFinalParams, iDefaultParam;
	DBLENGTH cbRowSize=0, ulMaxDispSize=0; 
	DBORDINAL cCols;
	CCol TempCol;
	BOOL fReturn = FALSE;
	enum TOKEN_ENUM eCreateProc, eExecStmt;
	WCHAR * pwszSelect = NULL;
	IRowset * pIRowset = NULL;
	BYTE * pData = NULL;
	DBCOUNTITEM cRowsObtained = 0;
	DBROWCOUNT cRowsAffected = 0;
	HROW * phRows = NULL;
	HACCESSOR hAccessor;
	LPWSTR pwszParamMarker = NULL;

	ParamStruct * pColList=NULL;
	ParamStruct * pParamList=NULL;
	ParamStruct * pLimit=NULL;
	ParamStruct * pReturn=NULL;
	ParamStruct * pParamAll=NULL;

	ParamStruct ReturnParamStruct;

	DB_LORDINAL * pulColOrds=NULL;

	if (pcParams)
		*pcParams=0;
	if (pcbRowSize)
		*pcbRowSize=0;
	if (ppDBBINDINFO)
		*ppDBBINDINFO=NULL;
	if (ppParamOrdinals)
		*ppParamOrdinals=NULL;
	if (ppDBPARAMBINDINFO)
		*ppDBPARAMBINDINFO=NULL;
	if (ppwszCreateStmt)
		*ppwszCreateStmt=NULL;
	if (ppwszExecProcStmt)
		*ppwszExecProcStmt=NULL;
	if (ppwszExecStmt)
		*ppwszExecStmt=NULL;
	if (ppData)
		*ppData=NULL;
	if (ppParamAll)
		*ppParamAll=NULL;

	// It's always an error to not include these output params
	if (!pcParams || !pcbRowSize || !ppDBBINDINFO || !ppDBPARAMBINDINFO ||
		!ppwszProcName || !ppParamAll || !ppParamOrdinals)
		goto CLEANUP;

	// Validate the proc type
	switch(eProcType)
	{
		// We only understand this type right now
		case T_EXEC_PROC_SELECT_NO_PARM:
		case T_EXEC_PROC_SELECT_IN:
		case T_EXEC_PROC_SELECT_OUT:
		case T_EXEC_PROC_SELECT_OUT_DFLT:
		case T_EXEC_PROC_SELECT_OUT_RET:
		case T_EXEC_PROC_SELECT_INOUT:
		case T_EXEC_PROC_SELECT_OUT_NULL:
		case T_EXEC_PROC_INSERT_INPUT:
			break;
		default:
			goto CLEANUP;
	}

	// It's an error to ask for multiple parameter sets on a select?
	if ((eProcType == T_EXEC_PROC_SELECT_OUT_DFLT ||
		eProcType == T_EXEC_PROC_SELECT_OUT_RET)
		&& cParamSets > 1)
		goto CLEANUP;

	// If we passed in a proc name to use then set it for the syntax routine
	m_Syntax.SetProcName(*ppwszProcName);
	m_Syntax.SetCreateFlags(ulCreateFlags);

	// If we have a return parameter increment the count of params immediately
	// and tell the syntax object
	if (bHasReturnParam(eProcType))
	{
		cParams++;
		m_Syntax.SetReturnParam(TRUE);
	}
	else
		m_Syntax.SetReturnParam(FALSE);

	// Find out how many columns are in the table
	cCols = m_pTable->CountColumnsOnTable();

	// Allocate space for arrays of indexes mapping items to columns.  We assume there will be no more
	// than one entry per table column maximum
	TEST_ALLOC(ParamStruct, pColList,	0, (size_t)(cCols*sizeof(ParamStruct)));
	TEST_ALLOC(ParamStruct, pParamList,	0, (size_t)(cCols*sizeof(ParamStruct)));
	TEST_ALLOC(ParamStruct, pLimit,		0, (size_t)(cCols*sizeof(ParamStruct)));

	// Go through each column in the table and add to our lists
	for (iCol=1; iCol<=cCols; iCol++)
	{
		// Get the information about the column
		if (!CHECK(m_pTable->GetColInfo(iCol, TempCol), S_OK))
			goto CLEANUP;

		// If the proc needs a limit clause and the column is useful in a limit clause add it
		if (m_Syntax.bNeedsLimitClause(eProcType) && m_Syntax.bAddColumn(T_SEARCHABLE_COL_EQ, TempCol))
		{
			WCHAR wszDataVal[SP_TEXT_BLOCK_SIZE] = L"\0";

			// By default the limit clause has param names starting with L so they
			// don't conflict with any output params.
			WCHAR wchFirst = L'L';
			if (eProcType == T_EXEC_PROC_SELECT_INOUT ||
				eProcType == T_EXEC_PROC_SELECT_INOUT_RET)
				// For these procs the param names must match
				wchFirst = L'P';

			// Record the max display size for later use if binding to WCHAR
			ulMaxDispSize = max(DisplaySize(TempCol), ulMaxDispSize);

			// Create a param name for the limit clause
			if (!(pLimit[cLimit].pwszParamName = m_Syntax.MakeParamName(TempCol, wchFirst)))
				goto CLEANUP;

			// Record the column index and I/O for this limit parameter
			pLimit[cLimit].eParamIO=DBPARAMIO_INPUT;
			pLimit[cLimit].ulColIndex=iCol;
			cLimit++;
		}

		// Now we build the column list and parameter lists.  Either may be empty depending on the stmt type
		// For updates or inserts we want only the updatable columns, for output params we can't have LONG cols.
		if (m_Syntax.bAddColumn(eProcType, TempCol))
		{
			// We need a column that matches the return type requested.  This parameter doesn't have
			// a naturally associated column, but we can pick one to use, otherwise we can't match
			// the data.
			if (bHasReturnParam(eProcType) && !pReturn && wReturnType == TempCol.GetProviderType())
			{
				ReturnParamStruct.ulColIndex=iCol;
				ReturnParamStruct.eParamIO=DBPARAMIO_OUTPUT;
				// Note the provider type name is populated later

				pReturn = &ReturnParamStruct;

				/*
				// Return parameter name will start with 'R'
				if (!(pReturn[0].pwszParamName = m_Syntax.MakeParamName(TempCol, 'R')))
					goto CLEANUP;
				*/

				// Due to provider-specific behavior here we will simply look up the return param
				// name to use
				if (!(pReturn[0].pwszParamName = m_Syntax.GetSyntax(T_RET_NAME)))
					goto CLEANUP;
			}

			// Other parameter names will start with 'P'
			if (!(pParamList[cParamList].pwszParamName = m_Syntax.MakeParamName(TempCol, 'P')))
				goto CLEANUP;

			// Record the column index and I/O
			pParamList[cParamList].ulColIndex=iCol;

			// Depending on the type of proc we're creating set the I/O
			switch (eProcType)
			{
				case T_EXEC_PROC_SELECT_OUT:
				case T_EXEC_PROC_SELECT_OUT_DFLT:
				case T_EXEC_PROC_SELECT_OUT_NULL:
				case T_EXEC_PROC_SELECT_OUT_RET:
					pParamList[cParamList].eParamIO=DBPARAMIO_OUTPUT;
					break;
				case T_EXEC_PROC_SELECT_INOUT:
				case T_EXEC_PROC_SELECT_INOUT_RET:
					pParamList[cParamList].eParamIO=DBPARAMIO_OUTPUT | DBPARAMIO_INPUT;
					break;
				default:
					pParamList[cParamList].eParamIO=DBPARAMIO_INPUT;
			}

			cParamList++;
		}

	} // for (icol = 1; ...

	// Now that we've gone through all the columns we have generated
	// 1) The limit clause parameters
	// 2) The output parameter list
	// 3) The return parameter info if needed

	// For IN/OUT params we want to just discard the limit clause infomation since it should match
	// the output params
	if (eProcType == T_EXEC_PROC_SELECT_INOUT ||
		eProcType == T_EXEC_PROC_SELECT_INOUT_RET)
		cLimit = 0;

	// The total number of parameters is the number in the parameter list plus the limit clause, plus any return
	// parameter
	cParams+=cParamList+cLimit;

	// Allocate space for DBBINDINFO and DBPARAMBINDINFO
	TEST_ALLOC(DBBINDING, *ppDBBINDINFO, 0, cParams*sizeof(DBBINDING));
	TEST_ALLOC(DBPARAMBINDINFO, *ppDBPARAMBINDINFO, 0, cParams*sizeof(DBPARAMBINDINFO));
	TEST_ALLOC(ParamStruct, pParamAll,	0, cParams*sizeof(ParamStruct));
	TEST_ALLOC(DB_LORDINAL, pulColOrds,0, cParams*sizeof(DB_LORDINAL));
	TEST_ALLOC(DB_UPARAMS, *ppParamOrdinals, 0, cParams*sizeof(DB_UPARAMS));

	// Now go through each parameter and build the DBBINDING and DBPARAMBINDINFO
	for (iParam=0; iParam < cParams; iParam++)
	{
		(*ppParamOrdinals)[iParam] = iParam+1;

		// The first parameter is the return parameter if it exists.  
		if (bHasReturnParam(eProcType) && !iParam)
		{
			// It's a programming error to not populate pReturn at this point.
			ASSERT (pReturn);

			if (!pReturn)
				goto CLEANUP;

			if (!AddParam(iParam, pReturn->ulColIndex, pReturn->eParamIO, pReturn->pwszParamName, fBindByName,
			&cbRowSize, *ppDBBINDINFO, *ppDBPARAMBINDINFO, pParamAll, m_pTable, TRUE))
				goto CLEANUP;

			pulColOrds[iParam]=pReturn->ulColIndex;

		}
		// The next cParamList parameters are the statement parameters
		else if (cParamList)
		{
			if (!AddParam(iParam,  pParamList[iParamList].ulColIndex, pParamList[iParamList].eParamIO,
				pParamList[iParamList].pwszParamName, fBindByName, &cbRowSize, *ppDBBINDINFO, *ppDBPARAMBINDINFO, pParamAll, m_pTable))
					goto CLEANUP;

			pulColOrds[iParam]=pParamList[iParamList++].ulColIndex;

			cParamList--;
		}

		// The last cLimit parameters are for the limit clause
		else if (cLimit)
		{
			if (!AddParam(iParam, pLimit[iLimit].ulColIndex, pLimit[iLimit].eParamIO, 
				pLimit[iLimit].pwszParamName, fBindByName,  &cbRowSize, *ppDBBINDINFO, *ppDBPARAMBINDINFO, pParamAll, m_pTable))
					goto CLEANUP;

			pulColOrds[iParam]=pLimit[iLimit++].ulColIndex;

			cLimit--;
		}

	}

	// If creating the indirect select to test in/out params we need to set the comparison
	// operator to use in the where clause to select the proper row.  By selecting DISTINCT
	// using an order by clause we assume we can end up with the first row less than the second 
	// row such that with a select with second row data values as inputs we can retrieve the first
	// row.  That way our output params hopefully have different values than the input params.

	// This will be absolutely true using automaketable without NULLs, but may not be true if 
	// using an ini file or if NULLS are included.
	if (eProcType == T_EXEC_PROC_SELECT_INOUT)
	{
		BOOL fSingletonSelect = FALSE;
		BYTE * pPrev = NULL;
		DBLENGTH cbPrev = 0;
		DBSTATUS ulStPrev = DBSTATUS_E_BADSTATUS;

		// Get the syntax for doing the same select without params
		TEST_COMPARE(((pwszSelect = m_Syntax.GetSyntax(T_SELECT_UNIQUE_ALL)) != NULL), TRUE);

		TEST_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL, pwszSelect), S_OK);

		TEST_CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cParams, *ppDBBINDINFO,
			cbRowSize, &hAccessor, NULL), S_OK);

		TEST_CHECK(m_pICommand->Execute(NULL, IID_IRowset,
				NULL, &cRowsAffected, (IUnknown **)&pIRowset), S_OK);

		// Allocate enough memory to hold first three rows
		TEST_ALLOC(BYTE, pData, 0, (size_t)(3 * cbRowSize));

		// Row number 0 is illegal
		TEST_COMPARE(ulTableRow > 0, TRUE);

		// Retrieve the row handles.  For the first row desired we retrieve rows 1-3 and ignore row 3.
		// We use rows 1 & 2 only.
		// If row desired > 1 then we retrieve the row before and after also.  The row before is used to
		// decide if the select will return too many rows.
		TEST_CHECK(pIRowset->GetNextRows(NULL, (ulTableRow == 1) ? ulTableRow-1 : ulTableRow-2, 3, &cRowsObtained, &phRows), S_OK);

		// Make sure we got 3 rows
		TEST_COMPARE(cRowsObtained, 3);

		// Retrieve the data from the first three rows
		// If everything goes right the row at pData+cbRowSize will match the output
		// params, while the row at pData will be the input/output params.
		// We use the row at pData + 2 * cbRowSize when ulTableRow is > 1 to verify the select
		// will only retrieve one row.

		if (ulTableRow == 1)
		{
			// We want to retrieve the first row, so we use row 2 data in the input params
			// The third row is useless to us.
			TEST_CHECK(pIRowset->GetData(phRows[0], hAccessor, pData+cbRowSize), S_OK);		// Row 1 data for validation
			TEST_CHECK(pIRowset->GetData(phRows[1], hAccessor, pData), S_OK);				// Input/output params (row 2)
			TEST_CHECK(pIRowset->GetData(phRows[2], hAccessor, pData+2*cbRowSize), S_OK);	// Ignored
			fSingletonSelect = TRUE;
		}
		else
		{
			TEST_CHECK(pIRowset->GetData(phRows[1], hAccessor, pData+cbRowSize), S_OK);		// Row N data for validation
			TEST_CHECK(pIRowset->GetData(phRows[2], hAccessor, pData), S_OK);				// Input/output params (row N+1)
			TEST_CHECK(pIRowset->GetData(phRows[0], hAccessor, pData+2*cbRowSize), S_OK);	// Row N-1
		}

		// Release the rows and rowset now that we've got the data
		CHECK(pIRowset->ReleaseRows(3, phRows, NULL, NULL, NULL), S_OK);

		// Our input/output parameter data will be the first row in pData.
		if (ppData)
			*ppData = pData;
		
		// For each column (parameter) in the result set compare the first row's
		// data with the second and set the where operator appropriately (<, = , >).
		for (iParam=0; iParam < cParams; iParam++)
		{
			LONG lCompResult;
			BYTE * pFirst, * pSecond;
			DBLENGTH cbFirst, cbSecond;
			DBSTATUS ulStFirst, ulStSecond;
			DBLENGTH * pLengthFirst = (DBLENGTH *)(pData+cbRowSize+(*ppDBBINDINFO)[iParam].obLength);
			DBLENGTH * pLengthSecond = (DBLENGTH *)(pData+(*ppDBBINDINFO)[iParam].obLength);

			// For STR and WSTR fields, since they were retrieved by GetData, the 
			// length value includes the null terminator.  We want to use this for
			// sending parameter data so we can't include the null terminator.
			// Oops, no the spec says null terminator is not included in the length
			/*
			if ((*ppDBBINDINFO)[iParam].wType == DBTYPE_STR)
				(*pLengthFirst)--;

			if ((*ppDBBINDINFO)[iParam].wType == DBTYPE_WSTR)
				(*pLengthFirst)-=sizeof(WCHAR);
			*/
			pFirst		= pData+cbRowSize+(*ppDBBINDINFO)[iParam].obValue;
			pSecond		= pData+(*ppDBBINDINFO)[iParam].obValue;
			cbFirst		= *pLengthFirst;
			cbSecond	= *pLengthSecond;
			ulStFirst	= *(DBSTATUS *)(pData+cbRowSize+(*ppDBBINDINFO)[iParam].obStatus);
			ulStSecond	= *(DBSTATUS *)(pData+(*ppDBBINDINFO)[iParam].obStatus);

			// Get the previous row info (only used for ulTableRow > 1)
			pPrev		= pData+2*cbRowSize+(*ppDBBINDINFO)[iParam].obValue;
			ulStPrev	= *(DBSTATUS *)(pData+2*cbRowSize+(*ppDBBINDINFO)[iParam].obStatus);
			cbPrev		= *(DBLENGTH *)(pData+2*cbRowSize+(*ppDBBINDINFO)[iParam].obLength);

			// Either of the fields may be NULL.
			if (ulStFirst == DBSTATUS_S_ISNULL ||
				ulStSecond == DBSTATUS_S_ISNULL ||
				RELCMP_NULL_VARIANT == (lCompResult = RelativeCompare(pFirst, pSecond, (*ppDBBINDINFO)[iParam].wType, (USHORT)cbSecond, 
					(*ppDBBINDINFO)[iParam].bPrecision, (*ppDBBINDINFO)[iParam].bScale, (ULONG)cbFirst)))
			{
				// If either is NULL we can't verify input/output param because we 
				// have to use the IS NULL construct instead of a parameter.
				odtLog << L"Couldn't verify input/output for parameter: " << 
					pParamAll[iParam].pwszParamName << L" because one entry was NULL.\n";

				// If the second row is NULL and the first isn't we can use IS NOT NULL construct
				// to retrieve first row.  
				if (ulStSecond == DBSTATUS_S_ISNULL)
					pParamAll[iParam].eCompareOp = CP_ISNOTNULL;
				
				// If the first row is actually NULL then we must use IS NULL to retrieve,
				// nothing else is possible.
				if (ulStFirst == DBSTATUS_S_ISNULL)
					pParamAll[iParam].eCompareOp = CP_ISNULL;

				(*ppDBBINDINFO)[iParam].eParamIO &= ~DBPARAMIO_INPUT;
				pParamAll[iParam].eParamIO &= ~DBPARAMIO_INPUT;
				(*ppDBPARAMBINDINFO)[iParam].dwFlags &= ~DBPARAMFLAGS_ISINPUT;

			}
			else
			{
				// Hack for Oracle DBMS.  Oracle uses a binary comparison, not a linguistic comparison,
				// so need to revert to wcscmp/strcmp for that case.
				if (g_bOracle)
				{
					switch((*ppDBBINDINFO)[iParam].wType)
					{	
						case DBTYPE_STR:
							lCompResult = strcmp((LPSTR)pFirst, (LPSTR)pSecond);
							break;
						case DBTYPE_WSTR:
							lCompResult = wcscmp((LPWSTR)pFirst, (LPWSTR)pSecond);
							break;
					}
				}

				// Neither field was NULL
				switch(lCompResult)
				{
					case 1:
						pParamAll[iParam].eCompareOp = CP_GT;
						break;
					case -1:
						pParamAll[iParam].eCompareOp = CP_LT;
						break;
					default:
						// '=' is the default, however, we didn't verify i/o param if =.
						odtLog << L"Couldn't verify input/output for parameter: " << 
							pParamAll[iParam].pwszParamName << L" because the values were equal.\n";
				}
			}

			// If requesting a row > 1 make sure we have a singleton select
			if (!fSingletonSelect)
			{
				if (ulStFirst != ulStPrev)
					fSingletonSelect = TRUE;
				else
				{
					if (ulStFirst == DBSTATUS_S_OK && lCompResult != RelativeCompare(pPrev, pSecond,
						(*ppDBBINDINFO)[iParam].wType, (USHORT)cbPrev, (*ppDBBINDINFO)[iParam].bPrecision,
						(*ppDBBINDINFO)[iParam].bScale, (ULONG)cbFirst))
						fSingletonSelect = TRUE;
				}
			}
		}

		// We must have a singleton select for this case.  If we're asking for the first
		// row then this is guaranteed due to the "select distinct"
		TEST_COMPARE(fSingletonSelect, TRUE);

	}

	// Fill the parameter information for each row if requested. 
	// Note it's very important to fill the bindings before the syntax is created,
	// since for NULL values the syntax is different (IS NULL must be used).
	else if (ppData)
	{
		TEST_ALLOC(BYTE, *ppData, 0, (size_t)(cbRowSize*cParamSets));

		for (iParamRow=0; iParamRow < cParamSets; iParamRow++)
		{
			BYTE * pData=(*ppData+cbRowSize*iParamRow);

			TEST_CHECK (FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cParams,
				*ppDBBINDINFO, &pData, ulTableRow+iParamRow, cParams, pulColOrds, PRIMARY), S_OK);

			// For output only bindings we want to zero out the buffer so we verify if it's been used
			for (iParam=0; iParam < cParams; iParam++)
			{
				DBSTATUS * pParamSt = (DBSTATUS *)(pData+(*ppDBBINDINFO)[iParam].obStatus);

				// If the data for the parameter will be NULL then in order to select it
				// we need to use IS NULL in the where clause
				if ((*ppDBBINDINFO)[iParam].eParamIO == DBPARAMIO_INPUT &&
					*pParamSt == DBSTATUS_S_ISNULL &&
					eProcType != T_EXEC_PROC_INSERT_INPUT) // For insert/updates statements we need to leave as-is
				{
					pParamAll[iParam].eCompareOp = CP_ISNULL;
					(*ppDBBINDINFO)[iParam].eParamIO &= ~DBPARAMIO_INPUT;
					pParamAll[iParam].eParamIO &= ~DBPARAMIO_INPUT;
					(*ppDBPARAMBINDINFO)[iParam].dwFlags &= ~DBPARAMFLAGS_ISINPUT;
				}

				// For output params, set initial values to bogus.
				if ((*ppDBBINDINFO)[iParam].eParamIO == DBPARAMIO_OUTPUT)
				{
					BYTE * pParamBuf = pData+(*ppDBBINDINFO)[iParam].obValue;
					DBLENGTH ulParamLen = (*ppDBBINDINFO)[iParam].cbMaxLen;
					memset(pParamBuf, 0xCA, (size_t)ulParamLen);

					STATUS_BINDING((*ppDBBINDINFO)[iParam], pData) = OUT_PARAM_STATUS_INVALID; 
					LENGTH_BINDING((*ppDBBINDINFO)[iParam], pData) = OUT_PARAM_LENGTH_INVALID; 
				}

			}
		}
	}


	// Give the parameter struct info to the syntax lookups
	m_Syntax.SetColMap(cParams, pParamAll);
	m_Syntax.SetCurrentRow(ulTableRow);
	m_Syntax.SetProcType(eProcType);

	switch (eProcType)
	{
		case T_EXEC_PROC_SELECT_OUT:
			eCreateProc = T_CREATE_PROC_SELECT_OUT;
			eExecStmt = T_SELECT_OUT_MARKER;
			break;
		case T_EXEC_PROC_SELECT_OUT_DFLT:
			eCreateProc = T_CREATE_PROC_SELECT_OUT_DFLT;
			eExecStmt = T_NONE;
			break;
		case T_EXEC_PROC_SELECT_OUT_RET:
			eCreateProc = T_CREATE_PROC_SELECT_OUT_RET;
			eExecStmt = T_NONE;
			break;
		case T_EXEC_PROC_SELECT_IN:
			eCreateProc = T_CREATE_PROC_SELECT_IN;
			eExecStmt = T_SELECT_IN_MARKER;
			break;
		case T_EXEC_PROC_SELECT_NO_PARM:
			eCreateProc = T_CREATE_PROC_SELECT_NO_PARM;
			eExecStmt = T_SELECT_NO_PARM;
			break;
		case T_EXEC_PROC_SELECT_IN_RET:
			eCreateProc = T_CREATE_PROC_SELECT_IN_RET;
			eExecStmt = T_NONE;
			break;
		case T_EXEC_PROC_SELECT_INOUT:
			eCreateProc = T_CREATE_PROC_SELECT_INOUT;
			eExecStmt = T_NONE;
			break;
		case T_EXEC_PROC_SELECT_OUT_NULL:
			eCreateProc = T_CREATE_PROC_SELECT_OUT_NULL;
			eExecStmt = T_SELECT_OUT_NULL_MARKER;
			break;
		case T_EXEC_PROC_INSERT_INPUT:
			eCreateProc = T_CREATE_PROC_INSERT_IN;
			eExecStmt = T_INSERT_IN_MARKER;
			break;
	}

	// Get the statement required to create the proc
	if (ppwszCreateStmt && !(*ppwszCreateStmt = m_Syntax.GetSyntax(eCreateProc, pcColumns, prgColumnsOrd)))
		goto CLEANUP;
	
	// Get the statement required to execute the proc
	if (ppwszExecProcStmt && !(*ppwszExecProcStmt = m_Syntax.GetSyntax(eProcType)))
		goto CLEANUP;

	// Get the statement required to execute the statement with parameter markers
	if (ppwszExecStmt && eExecStmt != T_NONE && !(*ppwszExecStmt = m_Syntax.GetSyntax(eExecStmt)))
		goto CLEANUP;

	// Get the proc name
	if (!*ppwszProcName && !(*ppwszProcName = m_Syntax.GetSyntax(T_PROC_NAME)))
		goto CLEANUP;

	// Can't strip out any ISNULL or ISNOTNULL params for an insert sproc
	if (eProcType != T_EXEC_PROC_INSERT_INPUT) // For insert/updates statements we need to leave as-is
	{

		// Go back through the binding information and remove any that are using IS NULL or IS NOT NULL
		// syntax for input params.  They're not really input parameters.  Also, when using a default
		// param we remove the first input parameter from the final bindings.
		cFinalParams = cParams;
		iDestParam = 0;
		iDefaultParam = ULONG_MAX;
		for(iParam=0; iParam < cParams; iParam++)
		{
			if ((pParamAll[iParam].eCompareOp == CP_ISNULL ||
				pParamAll[iParam].eCompareOp == CP_ISNOTNULL) ||
				(eProcType == T_EXEC_PROC_SELECT_OUT_DFLT &&
				iDefaultParam == ULONG_MAX))
			{
				if (!(pParamAll[iParam].eParamIO & DBPARAMIO_OUTPUT))
				{
					// It's not an output param, so we can remove it entirely
					cFinalParams--;
					// Can't adjust cbRowSize because we filled with all params and offsets.
					//				cbRowSize-=(*ppDBBINDINFO)[iParam].cbMaxLen;  
					PROVIDER_FREE(pParamAll[iParam].pwszParamName);

					if (pParamAll[iParam].eCompareOp != CP_ISNULL &&
						pParamAll[iParam].eCompareOp != CP_ISNOTNULL)
						iDefaultParam = iParam;
				}
				else
					iDestParam++;
			}
			else
			{
				if (iParam > iDestParam)
				{
					memcpy(&pParamAll[iDestParam], &pParamAll[iParam], sizeof(ParamStruct));
					memcpy(&(*ppDBBINDINFO)[iDestParam], &(*ppDBBINDINFO)[iParam], sizeof(DBBINDING));
					memcpy(&(*ppParamOrdinals)[iDestParam], &(*ppParamOrdinals)[iParam], sizeof(DB_UPARAMS));
					memcpy(&pulColOrds[iDestParam], &pulColOrds[iParam], sizeof(DB_LORDINAL));
					memcpy(&(*ppDBPARAMBINDINFO)[iDestParam], &(*ppDBPARAMBINDINFO)[iParam], sizeof(DBPARAMBINDINFO));
					(*ppDBPARAMBINDINFO)[iDestParam].pwszDataSourceType = pParamAll[iDestParam].wszDataSourceType;
					(*ppParamOrdinals)[iDestParam] = iDestParam+1;
					(*ppDBBINDINFO)[iDestParam].iOrdinal = iDestParam+1;
				}
				iDestParam++;
			}
		}
	}

	// Set the output parameters if not set yet
	pwszParamMarker = m_Syntax.GetSyntax(T_PARM_MARKER);
	*pcParams=wcschcount(*ppwszExecProcStmt, *pwszParamMarker); // cFinalParams;
	*pcbRowSize=cbRowSize;
	*ppParamAll=pParamAll;

	if (prgParamColOrd)
		*prgParamColOrd = pulColOrds;

	fReturn = TRUE;

CLEANUP:

	if (pIRowset)
		pIRowset->ReleaseRows(2, phRows, NULL, NULL, NULL);

	SAFE_RELEASE(pIRowset);

	PROVIDER_FREE(phRows);
	PROVIDER_FREE(pColList);
	PROVIDER_FREE(pParamList);
	PROVIDER_FREE(pLimit);
	if (!prgParamColOrd)
		PROVIDER_FREE(pulColOrds);
	PROVIDER_FREE(pwszSelect);
	PROVIDER_FREE(pwszParamMarker);

	return fReturn;

}

//--------------------------------------------------------------------------
// @mfunc Drop stored procedure
// @desc  Drop the stored procedure created so that a new one can be added.
//	
//--------------------------------------------------------------------------
BOOL 
CICmdWParams::DropStoredProcedure(ICommandText *pICommandText, WCHAR * pwszProcedureName, BOOL fIsFunction)
{
	WCHAR * pwszDropProc = NULL;
	WCHAR * pwszDropString=NULL;
	BOOL fSuccess = FALSE;
	ULONG i=0;
	enum TOKEN_ENUM eDropType = T_DROP;

	// First free the rowset if exists.
	if (m_pIStoredProcRowset)
	{
		m_pIStoredProcRowset->Release();
		m_pIStoredProcRowset = NULL;
	}

	// Retrieve syntax for dropping stored proc
	if (fIsFunction)
		eDropType = T_DROP_FUN;

	if (!(pwszDropProc = m_Syntax.GetSyntax(eDropType)))
		goto CLEANUP;

	// If we are passed a stored procedure name use it.
	if (pwszProcedureName)
		FormatString(&pwszDropString, pwszDropProc, 1, pwszProcedureName);
	else
		FormatString(&pwszDropString, pwszDropProc, 1, g_pwszProcedureName2);
	
	pICommandText->SetCommandText(DBGUID_DBSQL , pwszDropString);

	// If it exists drop it.  No need to check return code.
	pICommandText->Execute(NULL, IID_NULL,	NULL, NULL, NULL);

	if (m_hStoredProcAccessor != DB_NULL_HACCESSOR)
	{
		m_pCmdIAccessor->ReleaseAccessor (m_hStoredProcAccessor, NULL);
		m_hStoredProcAccessor = DB_NULL_HACCESSOR;
	}

	for (i =0; i < m_cStoredProcParamColMap;i++)
	{
		// This was allocated by wcsDuplicate in privlib.
		if (m_rgwszStoredProcDataSourceTypes[i])
			PROVIDER_FREE(m_rgwszStoredProcDataSourceTypes[i]);
	}

	fSuccess = TRUE;


CLEANUP:
	FREE_DATA (m_rgStoredProcColInfo);
	FREE_DATA(m_rgStoredProcBindings);
	FREE_DATA (m_rgStoredProcParamColMap);
	FREE_DATA(m_rgStoredProcParamOrdinals);
	FREE_DATA(m_rgStoredProcParamBindInfo);
	FREE_DATA(m_rgwszStoredProcDataSourceTypes);
	PROVIDER_FREE(pwszDropProc);
	PROVIDER_FREE(pwszDropString);

	// Reset the counts.
	m_cStoredProcParamColMap = 0;
	m_cStoredProcBindings = 0;

	return fSuccess;
}


//--------------------------------------------------------------------------
// @mfunc ExecuteStoredProcedure
// @desc  Execute the stored procedure for which text has already been set.
//			generate the required data and execute the stored procedure.
//	
//--------------------------------------------------------------------------
HRESULT 
CICmdWParams::ExecuteStoredProcedure(ICommandText *pICommandText, DBPARAMS *pDbParams, DBCOUNTITEM ulRowNum, ULONG DataValues,  ULONG BindingType)
{
	BYTE *pData = NULL;
	HRESULT hr = S_OK;
	ULONG i = 0;
	
	if (m_pIStoredProcRowset) ReleaseRowsetPtr(&m_pIStoredProcRowset);
	m_cStoredProcRowsAffected = 0;
	
	// Allocate pData.
	pData = (BYTE *)m_pIMalloc->Alloc(m_cbStoredProcRowSize);
	if (!pData)
	{
		return E_OUTOFMEMORY;
	}

	pDbParams->pData = pData;
	// Fill input bindings.
	if (!CHECK((hr = FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, m_cStoredProcBindings, m_rgStoredProcBindings,
				(BYTE **)&pData, (ulRowNum-1),  m_cStoredProcParamColMap, m_rgStoredProcParamColMap)), S_OK))
	{
		return hr;
	}

		
	if (DataValues == NULL_IN_VALID_OUT_DATA || BindingType == NULL_STATUS_ONLY)
	{
		// Setup the status bit as required.
		for (i = 0; i < m_cStoredProcBindings; i++)
		{
			// Check to see if status was bound.
			if (m_rgStoredProcBindings[i].dwPart & DBPART_STATUS)
			{
				*((DBSTATUS *)((BYTE *)pData + m_rgStoredProcBindings[i].obStatus)) = DBSTATUS_S_ISNULL;
			}
			else
			{
				// since we are expecting status to be bound return E_FAIL.
				return E_FAIL;
			}
		}
	}

	if (BindingType == PASS_BYREF)
	{
		COMPARE (StorePassByRefPointers(pDbParams->cParamSets, pData), TRUE);
	}

	return pICommandText->Execute(NULL, IID_IRowset,	pDbParams, &m_cStoredProcRowsAffected,
		(IUnknown **)&m_pIStoredProcRowset);

}



BOOL
CICmdWParams::StorePassByRefPointers(DB_UPARAMS cRows, BYTE * pData)
{
	ULONG i=0;
	
	if (cRows > 1)
	{
		odtLog << L"Function to be updated for multiple parameter sets\n";
		COMPARE(0, 1);
		return FALSE;
	}

	// Allocate storage for atleast m_cStoredProcBindings

	m_rgvpByRefPointers = (void **)m_pIMalloc->Alloc(m_cStoredProcBindings  * sizeof (void *));

	if (!m_rgvpByRefPointers)
	{
		odtLog << wszMemoryAllocationError;
		return FALSE;
	}

	// This function 
	for (i = 0; i < m_cStoredProcBindings; i++)
	{
		// No need to worry about fixed length columns.
		// Null out the pointer so that we know.
		if (IsFixedLength(m_rgStoredProcBindings[i].wType))
		{
			// Now get the pointer and store it.
			m_rgvpByRefPointers[i] = (void *)NULL;
		}
		else
		{
			m_rgvpByRefPointers[i] = (*(void **)(pData + m_rgStoredProcBindings[i].obValue));
		}
	}
	return TRUE;

}

void 
CICmdWParams::ReleaseInputBindingsMemoryByRef(ULONG cRows, BYTE * pData, BOOL fFreeProviderMemory)
{
	ULONG i = 0;
	void *ptr = NULL; 

	if (cRows > 1)
	{
		odtLog << L"Function to be updated for multiple parameter sets\n";
		COMPARE(0, 1);
		return;
	}


	if (!pData)
	{
		COMPARE(0, 1);
		odtLog << L"Null pData to free\n";
	}
	for (i = 0; i < m_cStoredProcBindings; i++)
	{
		if (IsFixedLength(m_rgStoredProcBindings[i].wType))
			continue;

		//Rest we have to free.
		if (m_rgvpByRefPointers[i])
			m_pIMalloc->Free (m_rgvpByRefPointers[i]);

		// If the data is valid, then we need to free it
		if (*(DBSTATUS *)(pData + m_rgStoredProcBindings[i].obStatus) == DBSTATUS_S_OK)
		{
			ptr = (void *)(*(void **)(pData + m_rgStoredProcBindings[i].obValue));

			// Now free the provider's memory.
			if (fFreeProviderMemory)
				m_pIMalloc->Free(ptr);
		}
	}

	// Now we have to free pData.
	FREE_DATA (pData);
	FREE_DATA (m_rgvpByRefPointers);
}

//--------------------------------------------------------------------------
// @mfunc SwapOrdinal
// @desc  Utility function to swap ordinal information for parameters
//--------------------------------------------------------------------------
void CICmdWParams::SwapOrdinal(ULONG ulBindIndex)
{
	ULONG ulSwapIndex=m_cStoredProcBindings-ulBindIndex-1;

	ASSERT(m_cStoredProcBindings > ulBindIndex);

	if(ulBindIndex < ulSwapIndex)
	{
		// Swap the binding values for this and corresponding opposite parameter
		DBORDINAL ulOrdinal;
		ulOrdinal=m_rgStoredProcBindings[ulBindIndex].iOrdinal;
		m_rgStoredProcBindings[ulBindIndex].iOrdinal=m_rgStoredProcBindings[ulSwapIndex].iOrdinal;
		m_rgStoredProcBindings[ulSwapIndex].iOrdinal=ulOrdinal;
	}
}

//--------------------------------------------------------------------------
// @mfunc ReverseArray
// @desc  Utility function to reverse the contents of an array
//--------------------------------------------------------------------------
BOOL CICmdWParams::ReverseArray(void * rgArray, DBCOUNTITEM cElements, ULONG ulElementSize)
{
	BYTE * pTemp=new BYTE[ulElementSize];
	BYTE * prgStart=(BYTE *)rgArray;
	BYTE * prgEnd=prgStart+(cElements-1)*ulElementSize;

	if (cElements < 2)
		return TRUE; 

	if (!pTemp)  
		return FALSE;

	while (prgStart < prgEnd)
	{
		memcpy(pTemp, prgStart, ulElementSize);
		memcpy(prgStart, prgEnd, ulElementSize);
		memcpy(prgEnd, pTemp, ulElementSize);
		prgStart+=ulElementSize;
		prgEnd-=ulElementSize;
	}

	if (pTemp)
		delete[] pTemp;

	return TRUE;
}

//--------------------------------------------------------------------------
// @mfunc ScrambleArray
// @desc  Utility function to scramble the contents of an array
//--------------------------------------------------------------------------
BOOL CICmdWParams::ScrambleArray(void * rgArray, DB_UPARAMS cElements, ULONG ulElementSize)
{
	BYTE * pTemp=new BYTE[ulElementSize];
	BYTE * prgStart=(BYTE *)rgArray;
	BYTE * prgNext=prgStart+ulElementSize;

	if (cElements < 2)
		return TRUE; 

	if (!pTemp)  
		return FALSE;

	for (ULONG idx=0; idx < cElements-1; idx+=2)
	{
		memcpy(pTemp, prgStart, ulElementSize);
		memcpy(prgStart, prgNext, ulElementSize);
		memcpy(prgNext, pTemp, ulElementSize);
		prgStart+=2*ulElementSize;
		prgNext+=2*ulElementSize;
	}

	if (pTemp)
		delete[] pTemp;

	return TRUE;
}


//--------------------------------------------------------------------------
// @mfunc CreateParameterNames
// @desc  Utility function to populate parameter names in DBPARAMBINDINFO array
//	ALL_VALID_NAMES,
//	ALL_INVALID_NAMES,	// Control character
//	SOME_INVALID_NAMES,	// Odd names are invalid
//	ALL_NULL_NAMES,
//	ALL_EMPTY_STRING_NAMES
//--------------------------------------------------------------------------
WCHAR ** CICmdWParams::CreateParameterNames(WCHAR ** ppwszParameterNames, enum NAME_ENUM eNameType,
	DB_UPARAMS * pcParamNames, ULONG fColTypes)
{

	CCol  		TempCol;
	ULONG		i=0;
	ULONG		iParamName=0;

	if (!(ppwszParameterNames))
	{
		// Allocate one more than required and set to zero to simplify freeing.
		ppwszParameterNames = (LPOLESTR *)m_pIMalloc->Alloc ((m_pTable->CountColumnsOnTable()+1) * sizeof (LPOLESTR));

		if (!(ppwszParameterNames))
			goto CLEANUP;

		memset(ppwszParameterNames, 0, (size_t)(m_pTable->CountColumnsOnTable()+1) * sizeof (LPOLESTR));
	}

	for (i = 0; i < m_pTable->CountColumnsOnTable(); i++)
	{
		CHECK(m_pTable->GetColInfo(i+1, TempCol), S_OK);

		// We want only the updatable non-long columns
		if (TempCol.GetUpdateable())
		{
			if (!TempCol.GetIsLong() || fColTypes == INCLUDE_LONG_COLS)
			{

				if (!ppwszParameterNames[iParamName])
				{
					ppwszParameterNames[iParamName] = (LPOLESTR)m_pIMalloc->Alloc( sizeof (WCHAR) * SP_MAX_PARAMNAME_LENGTH + 1);
					if (!ppwszParameterNames[iParamName])
						goto CLEANUP;
				}

				// Create a parameter name from the column name appropriate for the
				// provider/dbms combo.
				switch (eNameType)
				{
					case ALL_VALID_NAMES:
						// Use the provider required format if known
						if (!(ppwszParameterNames[iParamName] = m_Syntax.MakeParamName(TempCol, 'P')))
							goto CLEANUP;
						break;
					case SOME_INVALID_NAMES:
					case ALL_INVALID_NAMES:
						// Use the provider required format if known
						if (!(ppwszParameterNames[iParamName] = m_Syntax.MakeParamName(TempCol, 'P')))
							goto CLEANUP;
						if (eNameType == ALL_INVALID_NAMES || iParamName % 2)
						{
							// Set first character of name to an invalid value
							memset(ppwszParameterNames[iParamName], 1, 1);
						}
						break;
					case ALL_NULL_NAMES:
						if (ppwszParameterNames[iParamName])
							FREE_DATA(ppwszParameterNames[iParamName]);
						ppwszParameterNames[iParamName]=NULL;
						break;
					case ALL_EMPTY_STRING_NAMES:
						wcscpy(ppwszParameterNames[iParamName], L"");
						break;

				}

				iParamName++;
			}

		}
	}

	// Fill in the count of parameters if requested
	if (pcParamNames)
		*pcParamNames=iParamName;

	return ppwszParameterNames;

CLEANUP:

	FreeParameterNames(ppwszParameterNames);
	return NULL;
}

//--------------------------------------------------------------------------
// @mfunc FreeParameterNames
// @desc  Utility function to free all parameter names
//--------------------------------------------------------------------------
void CICmdWParams::FreeParameterNames(WCHAR ** ppwszParameterNames)
{
	if (ppwszParameterNames)
	{
		for (ULONG idx=0; ppwszParameterNames[idx]; idx++)
			FREE_DATA(ppwszParameterNames[idx]);
		FREE_DATA(ppwszParameterNames);
	}
}

void CICmdWParams::SetParameterNames(WCHAR ** pwszParameterNames)
{
	WCHAR * pwszParameterName=NULL;

	for (ULONG idx=0; idx < m_cDbParamBindInfo; idx++)
	{
		if (pwszParameterNames)
			pwszParameterName=pwszParameterNames[idx];
		m_rgDbParamBindInfo[idx].pwszName=pwszParameterName;
	}

}

HRESULT CICmdWParams::VerifyParamInfo(DBCOUNTITEM cParamsExp, DB_UPARAMS * rgParamOrdinals, DBPARAMBINDINFO * rgDbParamBindInfo,
	DB_UPARAMS cParams, DBPARAMINFO * rgParamInfo, LPOLESTR pNamesBuffer, ULONG idxStart, BOOL fDerived)
{
	CCol TempCol;
	DBORDINAL iOrdinal;
	ULONG idxMatch;
	BOOL bMatch=FALSE, bVerify=TRUE, bType=FALSE;
	DBTYPE wType;
	ULONG cFailures=0, cWarnings=0;
	ULONG iFlag, ulAllFlags=0;
	WCHAR wszMsg[MAX_MSG_LEN];
	WCHAR * pwszParamName;
	DBPARAMFLAGS dwFlags;
	LPWSTR pwszRetName = m_Syntax.GetSyntax(T_RET_NAME);

	// First make sure the count of params returned is accurate
	if (!MYCOMPARE(cParamsExp, cParams, L"Count of parameters did not match.\n"))
		return E_FAIL;

	if (cParamsExp == 0)
	{
		if (!COMPARE(rgParamInfo, NULL))
			return E_FAIL;

		if (!COMPARE(pNamesBuffer, NULL))
			return E_FAIL;
	}
	
	// Compare all values.  Note that OLE DB doesn't guarantee you get back the values you set.
	// When the driver can derive type information, the GetParameterInfo will return the 
	// "correct" values, not what was set.  Even if the provider can't derive parameter information
	// then the value may reflect the "best fit" parameter type for the DBTYPE specified in
	// SetParameterInfo so the values may not match for DBTYPE, Precision, Scale, etc.
	for (ULONG idx=0; idx < cParams; idx++)	
	{
		iOrdinal = rgParamInfo[idx].iOrdinal;

		// Ordinal values start at 1 and should be sequential
		if (!MYCOMPARE(iOrdinal,idx+idxStart+1,L"rgParamInfo.iOrdinal values are not sequential.\n"))
			cFailures++;

		// Locate the entry in rgParamOrdinals, since the order doesn't necessarily match
		bMatch=FALSE;
		for (idxMatch=0; idxMatch < cParams; idxMatch++)
		{
			if (rgParamOrdinals[idxMatch] == iOrdinal)
			{
				bMatch=TRUE;
				break;
			}
		}

		// If no matching value found print a failure
		if (!MYCOMPARE(bMatch, TRUE, L"Matching value not found in rgParamOrdinals.\n"))
		{
			cFailures++;
			continue;
		}

		// Compare the other entries

		// dwFlags
		// Sql Server always sets the ISINPUT flag even if not requested
		// when deriving parameter info except for return parameter.
		dwFlags = rgDbParamBindInfo[idxMatch].dwFlags;
		if (g_bSqlServer && fDerived &&
			wcscmp(rgDbParamBindInfo[idxMatch].pwszName, pwszRetName) &&
			dwFlags & DBPARAMFLAGS_ISOUTPUT &&
			!(dwFlags & DBPARAMFLAGS_ISINPUT))
			dwFlags |= DBPARAMFLAGS_ISINPUT;

		// Sql Server always set the NULLABLE flag for sysname data type for parameters even though
		// a sysname column is not nullable.  The parameter is nullable. 
		if (g_bSqlServer && fDerived &&
			!wcscmp(rgDbParamBindInfo[idxMatch].pwszDataSourceType, L"sysname") &&
			!(dwFlags & DBPARAMFLAGS_ISNULLABLE))
			dwFlags |= DBPARAMFLAGS_ISNULLABLE;
		
		// Set up param name to be used if mismatch
		if (rgDbParamBindInfo[idxMatch].pwszName)
			pwszParamName = rgDbParamBindInfo[idxMatch].pwszName;
		else
			pwszParamName = L"<null>";

		// Make sure at least one of DBPARAMFLAGS_ISINPUT or DBPARAMFLAGS_ISOUTPUT are set
		if (!(rgParamInfo[idx].dwFlags & DBPARAMFLAGS_ISINPUT) &&
			!(rgParamInfo[idx].dwFlags & DBPARAMFLAGS_ISOUTPUT))
			swprintf(wszMsg, L"Neither DBPARAMFLAGS_ISINPUT nor DBPARAMFLAGS_ISOUTPUT were set for parameter %d type %s.\n",
				idx+1, rgDbParamBindInfo[idxMatch].pwszDataSourceType);

		CCOMPARE(m_EC, ((rgParamInfo[idx].dwFlags & (DBPARAMFLAGS_ISINPUT | DBPARAMFLAGS_ISOUTPUT)) > 0) == TRUE, 
			EC_MISSING_ISINPUT_AND_ISOUTPUT_FLAGS,
			wszMsg,
			FALSE);

		// Check the setting of all known flags
		for (iFlag = 0; iFlag < NUMELEM(g_rgParamFlags); iFlag++)
		{

			ulAllFlags |= g_rgParamFlags[iFlag].dwFlag;

			// Set up message in case flag value did not match expected
			if (rgParamInfo[idx].dwFlags & g_rgParamFlags[iFlag].dwFlag)
				swprintf(wszMsg, L"%s was set and should have been clear for parameter %d type %s.\n",
					g_rgParamFlags[iFlag].pwszFlagName, idx+1, rgDbParamBindInfo[idxMatch].pwszDataSourceType);
			else
				swprintf(wszMsg, L"%s was clear and should have been set for parameter %d type %s.\n",
					g_rgParamFlags[iFlag].pwszFlagName, idx+1, rgDbParamBindInfo[idxMatch].pwszDataSourceType);

			switch(g_rgParamFlags[iFlag].dwFlag)
			{
				case DBPARAMFLAGS_ISSIGNED:
					CCOMPARE(m_EC, (rgParamInfo[idx].dwFlags & g_rgParamFlags[iFlag].dwFlag)==(dwFlags & g_rgParamFlags[iFlag].dwFlag), 
						EC_INVALID_ISSIGNED,
						wszMsg,
						FALSE);
					break;
				
				case DBPARAMFLAGS_ISNULLABLE:
					CCOMPARE(m_EC, (rgParamInfo[idx].dwFlags & g_rgParamFlags[iFlag].dwFlag)==(dwFlags & g_rgParamFlags[iFlag].dwFlag), 
						EC_INVALID_ISNULLABLE,
						wszMsg,
						FALSE);
					break;

				case DBPARAMFLAGS_ISINPUT:
					CCOMPARE(m_EC, (rgParamInfo[idx].dwFlags & g_rgParamFlags[iFlag].dwFlag)==(dwFlags & g_rgParamFlags[iFlag].dwFlag), 
						EC_INVALID_ISINPUT,
						wszMsg,
						FALSE);

					break;
				default:
					if (!MYCOMPARE(rgParamInfo[idx].dwFlags & g_rgParamFlags[iFlag].dwFlag, 
						dwFlags & g_rgParamFlags[iFlag].dwFlag, wszMsg))
						cFailures++;
			}
		}

		// Make sure no unknown flags were set
		swprintf(wszMsg, L"An unknown flag was set for parameter %d type %s.\n",
			idx+1, rgDbParamBindInfo[idxMatch].pwszDataSourceType);

		if (!MYCOMPARE(rgParamInfo[idx].dwFlags & ~ulAllFlags, 0, wszMsg))
			cFailures++;

		// pwszName
		//
		// Note that we assume names are supported if pNamesBuffer is not NULL or the pwszName is not NULL
		//
		// TODO: Check that pwszName in rgParamInfo points into pNamesNamesBuffer.
		if (pNamesBuffer || rgParamInfo[idx].pwszName)
		{
			if (rgDbParamBindInfo[idxMatch].pwszName && rgParamInfo[idx].pwszName)
			{	
				// We put in a name and got one back, make sure they're the same

				// Need to convert our internal names to upper or lower case before compare if provider does so
				if (m_ulIdentifierCase == DBPROPVAL_IC_UPPER)
					_wcsupr(rgDbParamBindInfo[idxMatch].pwszName);

				if (m_ulIdentifierCase == DBPROPVAL_IC_LOWER)
					_wcslwr(rgDbParamBindInfo[idxMatch].pwszName);

				CCOMPARE(m_EC, wcscmp(rgParamInfo[idx].pwszName, rgDbParamBindInfo[idxMatch].pwszName) == 0, 
					EC_INVALID_PARAM_NAME,
					L"Parameter names did not match.\n",
					FALSE);
			}
			else
			{
				// We either didn't put in a name (NULL), or put one in and didn't get one back
				// In either case it's an error if they don't match
				if (!MYCOMPARE(rgParamInfo[idx].pwszName, rgDbParamBindInfo[idxMatch].pwszName, L"Parameter names did not match.\n"))
					cFailures++;
			}
		}

		// pTypeInfo
		if (!MYCOMPARE(rgParamInfo[idx].pTypeInfo, NULL, L"pTypeInfo mismatch.\n"))  
			cFailures++;
		
		// wType, we need to find the corresponding provider type name so we can get the DBTYPE
		CCol TempCol;
		bMatch=FALSE;
		for (ULONG iCol=1; iCol<=m_pTable->CountColumnsOnTable(); iCol++)
		{
			CHECK(m_pTable->GetColInfo(iCol, TempCol), S_OK);
			if (!wcscmp(TempCol.GetProviderTypeName(), rgDbParamBindInfo[idxMatch].pwszDataSourceType))
			{
				bMatch=TRUE;
				wType=TempCol.GetProviderType();
				break;
			}
		}
		if (!MYCOMPARE(bMatch, TRUE, L"Provider type name not found.\n"))
			cFailures++;
		else if (!(bType = (wType == rgParamInfo[idx].wType)))
		{
/*
			odtLog << L"Warning: Type identifier didn't match for parameter " << idx+1 << L"\n";
			odtLog << L"Expected: " << wType << L" Received: " << rgParamInfo[idx].wType << L"\n";
			odtLog << L"Per spec this isn't a failure since providers are allowed to return the 'best fit' type.\n";
			odtLog << L"Skipping comparison of ulParamSize, bPrecision, and bScale.\n\n";
*/
			cWarnings++;
		}

		if (bType && !g_bOracle)
		{
			// ulParamSize - if the types didn't compare then the size likely won't anyway.
			if (!MYCOMPARE(rgParamInfo[idx].ulParamSize, rgDbParamBindInfo[idxMatch].ulParamSize, L"Parameter size did not match.\n"))  
			{
				cFailures++;
				odtLog << L"Parameter " << rgParamInfo[idx].iOrdinal << L" type " << rgDbParamBindInfo[idxMatch].pwszDataSourceType
					<< L" returned length " << rgParamInfo[idx].ulParamSize << L", expected " << (rgDbParamBindInfo[idxMatch]).ulParamSize
					<< L"\n";
			}

			// bPrecision
			CCOMPARE(m_EC, rgParamInfo[idx].bPrecision == rgDbParamBindInfo[idxMatch].bPrecision, 
				EC_INVALID_PRECISION,
				L"Precision did not match expected value",
				FALSE);


			// bScale
			CCOMPARE(m_EC, rgParamInfo[idx].bScale==rgDbParamBindInfo[idxMatch].bScale, 
				EC_INVALID_SCALE,
				L"Scale did not match expected value",
				FALSE);
		}
	}

	SAFE_FREE(pwszRetName);

	return cFailures ? E_FAIL  : S_OK;
}


// CICmdWParams::SetRowsetPropertyDefault ------------------------------------
//
// Sets the given rowset property using ICommandProperties to the default value
//
//
HRESULT CICmdWParams::SetRowsetPropertyDefault(DBPROPID DBPropID, ICommand * pICommand)
{
	ICommandProperties * pICmdProps = NULL;
	DBPROPSET	DBPropSet;
	DBPROP		DBProp;
	HRESULT		hr = E_FAIL;

	//Set up the rowset property structure to use the ID passed in
	DBPropSet.rgProperties = &DBProp;
	DBPropSet.cProperties = 1;
	DBPropSet.guidPropertySet = DBPROPSET_ROWSET;

	DBProp.dwPropertyID = DBPropID;
	DBProp.dwOptions = DBPROPOPTIONS_OPTIONAL;
	DBProp.colid = DB_NULLID;
	DBProp.vValue.vt = VT_EMPTY;	//Causes default to be set

	if (!pICommand)
		pICommand = m_pICommand;

	ASSERT(pICommand);
	
	TESTC_(pICommand->QueryInterface(IID_ICommandProperties, 
		(void **)&pICmdProps), S_OK);
		
	TESTC_(pICmdProps->SetProperties(1, &DBPropSet), S_OK);

	hr = S_OK;

CLEANUP:

	SAFE_RELEASE(pICmdProps);
	
	return hr;
}


//-----------------------------------------
//@mfunc Terminate function for base class;
//-----------------------------------------
BOOL
CICmdWParams::Terminate()
{

	// Free allocated memory.
	ReleaseDataForCommand();

	FREE_DATA(m_rgParamColMap);
	FREE_DATA(m_pwszSqlInsertAllWithParams);
	FREE_DATA(m_rgBindings);
	FREE_DATA(m_rgColInfo);
	FREE_DATA(m_rgParamOrdinals);


	if (m_pExtraTable)
	{
		((CTable *)m_pExtraTable)->DropTable();
		delete (CTable *)m_pExtraTable;
		m_pExtraTable = NULL;
	}


	PROVIDER_FREE(m_pParamStruct);
	FREE_DATA(m_rgDbParamBindInfo);

	// Release interface pointers.
	RELEASE (m_pICmdWParams);
	RELEASE (m_pGeneralCmdIAccessor);
	RELEASE (m_pICommandPrepare);
	RELEASE (m_pICommandText);

	// Accessor being released.
	if (m_pCmdIAccessor)
		m_pCmdIAccessor->ReleaseAccessor(m_hAccessor, NULL);	
	
	RELEASE (m_pCmdIAccessor);
	RELEASE (m_pICommand);
	RELEASE (m_pIEmptyCommand);

	cRow.ReleaseDBSession();
	cRow.ReleaseDataSourceObject();

	ReleaseDBSession();
	ReleaseDataSourceObject();

	return (COLEDB::Terminate());

}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCGetParameterInfo_Rowset)
//--------------------------------------------------------------------
// @class Test case for GetParameterInfo.
//
class TCGetParameterInfo_Rowset : public CICmdWParams { 
protected:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetParameterInfo_Rowset,CICmdWParams);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Call GetParameterInfo after SetCommandText.  Delete text and call GetParameterInfo again.
	int Variation_1();
	// @cmember Call GetParameterInfo after ICommandPrepare.
	int Variation_2();
	// @cmember Call GetParameterInfo after execute and release
	int Variation_3();
	// @cmember Valid SQL with no parameters.
	int Variation_4();
	// @cmember No command object.
	int Variation_5();
	// @cmember Call GetParameterInfo twice.
	int Variation_6();
	// @cmember Parameters (NULL, NULL, Valid
	int Variation_7();
	// @cmember Parameters(NULL, Valid, Valid
	int Variation_8();
	// @cmember Parameters (Valid, NULL, Valid
	int Variation_9();
	// @cmember Parameters (NULL, NULL, NULL
	int Variation_10();
	// @cmember Parameters (Valid, Valid, NULL
	int Variation_11();
	// @cmember Call GetParameterInfo after SetCommandText for stored proc with INPUT params
	int Variation_12();
	// @cmember Call GetParameterInfo after calling SetCommandText with a stored procedure with OUTPUT params
	int Variation_13();
	// @cmember Call GetParamInfo after calling SetCommandText with a stored procedure with IN/OUT params.
	int Variation_14();
	// @cmember Call GetParameterInfo after SetCommandText with StoredProc with VeryLongParamNames
	int Variation_15();
	// @cmember Call GetParamInfo with a stored procedure which is returning a value.
	int Variation_16();
	// @cmember GetParameterInfo with text set to different types of SQL statments with funny characters to test parsing code of Dev.
	int Variation_17();
	// @cmember Stored procedure execution on two different command objects.
	int Variation_18();
	// @cmember Maximum length parameter names
	int Variation_19();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCGetParameterInfo_Rowset)
#define THE_CLASS TCGetParameterInfo_Rowset
BEG_TEST_CASE(TCGetParameterInfo_Rowset, CICmdWParams, L"Test case for GetParameterInfo.")
	TEST_VARIATION(1, 		L"Call GetParameterInfo after SetCommandText.  Delete text and call GetParameterInfo again.")
	TEST_VARIATION(2, 		L"Call GetParameterInfo after ICommandPrepare.")
	TEST_VARIATION(3, 		L"Call GetParameterInfo after execute and release")
	TEST_VARIATION(4, 		L"Valid SQL with no parameters.")
	TEST_VARIATION(5, 		L"No command object.")
	TEST_VARIATION(6, 		L"Call GetParameterInfo twice.")
	TEST_VARIATION(7, 		L"Parameters (NULL, NULL, Valid")
	TEST_VARIATION(8, 		L"Parameters(NULL, Valid, Valid")
	TEST_VARIATION(9, 		L"Parameters (Valid, NULL, Valid")
	TEST_VARIATION(10, 		L"Parameters (NULL, NULL, NULL")
	TEST_VARIATION(11, 		L"Parameters (Valid, Valid, NULL")
	TEST_VARIATION(12, 		L"Call GetParameterInfo after SetCommandText for stored proc with INPUT params")
	TEST_VARIATION(13, 		L"Call GetParameterInfo after calling SetCommandText with a stored procedure with OUTPUT params")
	TEST_VARIATION(14, 		L"Call GetParamInfo after calling SetCommandText with a stored procedure with IN/OUT params.")
	TEST_VARIATION(15, 		L"Call GetParameterInfo after SetCommandText with StoredProc with VeryLongParamNames")
	TEST_VARIATION(16, 		L"Call GetParamInfo with a stored procedure which is returning a value.")
	TEST_VARIATION(17, 		L"GetParameterInfo with text set to different types of SQL statments with funny characters to test parsing code of Dev.")
	TEST_VARIATION(18, 		L"Stored procedure execution on two different command objects.")
	TEST_VARIATION(19, 		L"Maximum length parameter names")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCMapParameterNames)
//--------------------------------------------------------------------
// @class Test case for MapParameterNames.
//
class TCMapParameterNames : public CICmdWParams { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

protected:
	DBPARAMBINDINFO *	m_pParamBindInfo;
	DB_UPARAMS			m_iNotTouched;
	HRESULT				m_hrExpect;
	DB_UPARAMS			m_cParamNames;
	ULONG				m_fColTypes;
	BOOL				m_fUseProc;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMapParameterNames,CICmdWParams);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Constructor
	TCMapParameterNames(void);
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cParamNames = 0 : returns S_OK
	int Variation_1();
	// @cmember cParamNames > 0 and NULL pointer for rgParamNames: returns E_INVALIDARG
	int Variation_2();
	// @cmember rgParamOrdinals with NULL : returns E_INVALIDARG
	int Variation_3();
	// @cmember (0, NULL, NULL
	int Variation_4();
	// @cmember Map Parameter Names without prepared command: DB_E_NOTPREPARED
	int Variation_5();
	// @cmember MapParameterNames with all invalid parameter names: DB_E_ERRORSOCCURRED
	int Variation_6();
	// @cmember MapParameterNames with some invalid parameter names: DB_S_ERRORSOCCURRED
	int Variation_7();
	// @cmember MapParameterNames with all valid names: S_OK
	int Variation_8();
	// @cmember MapParameterNames with fewer names than set
	int Variation_9();
	// @cmember Random name order: S_OK
	int Variation_10();
	// @cmember Overridden with SetParameterInfo: S_OK or DB_E_ERRORSOCCURRED
	int Variation_11();
	// @cmember No command text set: DB_E_NOCOMMAND
	int Variation_12();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCMapParameterNames)
#define THE_CLASS TCMapParameterNames
BEG_TEST_CASE(TCMapParameterNames, CICmdWParams, L"Test case for MapParameterNames.")
	TEST_VARIATION(1, 		L"cParamNames = 0 : returns S_OK")
	TEST_VARIATION(2, 		L"cParamNames > 0 and NULL pointer for rgParamNames: returns E_INVALIDARG")
	TEST_VARIATION(3, 		L"rgParamOrdinals with NULL : returns E_INVALIDARG")
	TEST_VARIATION(4, 		L"(0, NULL, NULL")
	TEST_VARIATION(5, 		L"Map Parameter Names without prepared command: DB_E_NOTPREPARED")
	TEST_VARIATION(6, 		L"MapParameterNames with all invalid parameter names: DB_E_ERRORSOCCURRED")
	TEST_VARIATION(7, 		L"MapParameterNames with some invalid parameter names: DB_S_ERRORSOCCURRED")
	TEST_VARIATION(8, 		L"MapParameterNames with all valid names: S_OK")
	TEST_VARIATION(9, 		L"MapParameterNames with fewer names than set")
	TEST_VARIATION(10, 		L"Random name order: S_OK")
	TEST_VARIATION(11, 		L"Overridden with SetParameterInfo: S_OK or DB_E_ERRORSOCCURRED")
	TEST_VARIATION(12, 		L"No command text set: DB_E_NOCOMMAND")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCCommandExecute_Rowset)
//--------------------------------------------------------------------
// @class Test case for ICommand::Execute (For Parameter related test cases
//
class TCCommandExecute_Rowset : public CICmdWParams { 
protected:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

private:
	// @cmember Stored procedure command text.
	ICommandText *m_pSPICommandText;

	
	// @cmember Stored procedure command prepare.
	ICommandPrepare *m_pSPICommandPrepare;

	// @cmember Pointer to parameter struct of mapping info
	ParamStruct * m_pSPParamAll;

	// @cmember number of Bindings for SP.
	ULONG m_cSPBindings;

	// @cmember range of Bindings for SP
	DBBINDING *m_rgSPBindings;
	DBPARAMBINDINFO * m_rgSPParamBind;

	// @cmember Stored procedure ordinals and Colmap array.
	ULONG m_cSPParamColMap;
	DB_LORDINAL *m_rgSPParamColMap;
	DB_UPARAMS *m_rgSPParamColOrdinals;

	// @cmember Accessor interface for Stored procedure command
	ULONG m_cbReadRowSize;
	DBLENGTH m_cbSPRowSize;
	IAccessor *m_pSPIAccessor;
	HACCESSOR m_hReadAccessor;
	ULONG		m_cReadBindings;
	DBBINDING * m_rgReadBindings;


	// @cmember DBPARAMS data for execute statement
	DBPARAMS m_SPDBParams;

	// @cmember Rownumber used to create the stored procedure. (For Comapare data)
	DBCOUNTITEM m_ulLocalSPRowNum;

	//  @cmember Pointer to the rowset generated by the stored procedure.
	IRowset *m_pSPRowset;

	// @cmember Number of rows affected by the stored procedure.
	DBROWCOUNT m_cSPRowsAffected;

	// @cmember CreateSPText with  out parameters.
	BOOL CreateSPText();

	// @cmember CreateSP with out parameters.
	BOOL CreateSP();

	// @cmember Drop the stored procedure.
	BOOL DropSP();

	// @cmember Execute the stored procedure.
	HRESULT ExecuteSP(DBPARAMS *pDbParams, DBCOUNTITEM ulRowNum);

	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCommandExecute_Rowset,CICmdWParams);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember An invalid parameter in pParams, returns DB_E_INVALIDARG
	int Variation_1();
	// @cmember Supply too many parameter info than required.  Should return S_OK.
	int Variation_2();
	// @cmember Supply too few parameters than required, returns DB_E_PARAMNOTOPTIONAL
	int Variation_3();
	// @cmember Supply more than one value for the same parameter.  Returns DB_E_DUPLICATEPARAM
	int Variation_4();
	// @cmember Supply a parameter value outside the domain for that parameter, returns DB_E_OVERFLOW
	int Variation_5();
	// @cmember Accessor with PARAMETER_DATA | ROWDATA | READWRITE
	int Variation_6();
	// @cmember Accessor with ROWDATA with [in] and [in/out] parameters
	int Variation_7();
	// @cmember Accessor with only status binding.
	int Variation_8();
	// @cmember Accessor with only (value and length
	int Variation_9();
	// @cmember Verify Stored procedure (in/out
	int Variation_10();
	// @cmember DBPARAM invalid conditions.
	int Variation_11();
	// @cmember Supply a value which could not be co-erced, returns DB_E_CANTCONVERTVALUE.
	int Variation_12();
	// @cmember hAccessor in the DBPARAMS structure was invalid (DB_E_BADACCESSORHANDLE
	int Variation_13();
	// @cmember TEST Stuff
	int Variation_14();
	// @cmember Stored procedure with Valid Input/Output data.
	int Variation_15();
	// @cmember Stored Procedure with Valid OUTPUT only parameters.
	int Variation_16();
	// @cmember Stored procedure with NULL [out] and Valid [input] parameters.
	int Variation_17();
	// @cmember Stored procedure with NULL Output only parameters.
	int Variation_18();
	// @cmember Stored procedure with NULL input and Valid Output parameters.
	int Variation_19();
	// @cmember Input/Output parameters with only Status bound to NULL
	int Variation_20();
	// @cmember Output only parameters with Status only binding to NULL
	int Variation_21();
	// @cmember Input/output with status only binding to S_OK
	int Variation_22();
	// @cmember Output only with status only binding to S_OK
	int Variation_23();
	// @cmember Input/Output with Length and Value only bound.
	int Variation_24();
	// @cmember Output only with Length and Value bound
	int Variation_25();
	// @cmember Input/Output with Value and Status bound
	int Variation_26();
	// @cmember output only with Value and Status bound
	int Variation_27();
	// @cmember Input/output with only Value bound
	int Variation_28();
	// @cmember output only with Value only bound.
	int Variation_29();
	// @cmember INPUT/OUTPUT with BY_REF bindings
	int Variation_30();
	// @cmember OUTPUT_ONLY with BY_REF bindings.
	int Variation_31();
	// @cmember Return value for a stored procedure.
	int Variation_32();
	// @cmember Bind same output parameters multiple number of times.
	int Variation_33();
	// @cmember 1 parameter Set.
	int Variation_34();
	// @cmember 5 parameter sets
	int Variation_35();
	// @cmember 105 parameter sets.
	int Variation_36();
	// @cmember Different types of Good and Bad parameter Sets.
	int Variation_37();
	// @cmember Having a table such that Variable length columns make up first and last columns
	int Variation_38();
	// @cmember Having BLOB as first and last columns
	int Variation_39();
	// @cmember 5 parameter sets with different size of Variable length parameters.
	int Variation_40();
	// @cmember 5 parameters sets followed by 1 parameter set.
	int Variation_41();
	// @cmember BYREF accessor with PROVIDER_OWNED memory should fail.
	int Variation_42();
	// @cmember RestartPosition should either return S_OK or DB_E_CANNOTRESTART
	int Variation_43();
	// @cmember Named Parameters: Sproc parm count != bound parm count
	int Variation_44();
	// @cmember Named Parameters: Sproc parm order != binding order
	int Variation_45();
	// @cmember Named Parameters: Use all parameter names
	int Variation_46();
	// @cmember Named Parameters: Retrieve names and use without setting
	int Variation_47();
	// @cmember Named Parameters: Named return parameter from sproc
	int Variation_48();
	// @cmember Inline binding with more than 64K and embedded null term
	int Variation_49();
	// @cmember Insert with BSTR bindings to strings
	int Variation_50();
	// @cmember OpenRowset on a stored proc with params
	int Variation_51();
	// @cmember DBSTATUS_S_DEFAULT - Default values for param
	int Variation_52();
	// @cmember Multiple procs - execute two procs without SetParamInfo or Prepare
	int Variation_53();
	// @cmember Multiple procs - execute two procs with partial SetParamInfo and Prepare
	int Variation_54();
	// @cmember DBSTATUS_S_IGNORE - Not legal for params (DBSTATUS_E_BADSTATUS)
	int Variation_55();
	// @cmember OpenRowset on stored proc without params
	int Variation_56();
	// @cmember Select with DBSTATUS_S_ISNULL input params
	int Variation_57();
	// @cmember S_OK - Send less data than max for parameter
	int Variation_58();
	// @cmember S_OK - Insert proc with CANHOLDROWS OPTIONAL
	int Variation_59();
	// @cmember Insert with BSTR bindings to strings to stored proc
	int Variation_60();
	// @cmember S_OK: Input and output params with cParamSets > 1
	int Variation_61();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCCommandExecute_Rowset)
#define THE_CLASS TCCommandExecute_Rowset
BEG_TEST_CASE(TCCommandExecute_Rowset, CICmdWParams, L"Test case for ICommand::Execute (For Parameter related test cases")
	TEST_VARIATION(1, 		L"An invalid parameter in pParams, returns DB_E_INVALIDARG")
	TEST_VARIATION(2, 		L"Supply too many parameter info than required.  Should return S_OK.")
	TEST_VARIATION(3, 		L"Supply too few parameters than required, returns DB_E_PARAMNOTOPTIONAL")
	TEST_VARIATION(4, 		L"Supply more than one value for the same parameter.  Returns DB_E_DUPLICATEPARAM")
	TEST_VARIATION(5, 		L"Supply a parameter value outside the domain for that parameter, returns DB_E_OVERFLOW")
	TEST_VARIATION(6, 		L"Accessor with PARAMETER_DATA | ROWDATA | READWRITE")
	TEST_VARIATION(7, 		L"Accessor with ROWDATA with [in] and [in/out] parameters")
	TEST_VARIATION(8, 		L"Accessor with only status binding.")
	TEST_VARIATION(9, 		L"Accessor with only (value and length")
	TEST_VARIATION(10, 		L"Verify Stored procedure (in/out")
	TEST_VARIATION(11, 		L"DBPARAM invalid conditions.")
	TEST_VARIATION(12, 		L"Supply a value which could not be co-erced, returns DB_E_CANTCONVERTVALUE.")
	TEST_VARIATION(13, 		L"hAccessor in the DBPARAMS structure was invalid (DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(14, 		L"TEST Stuff")
	TEST_VARIATION(15, 		L"Stored procedure with Valid Input/Output data.")
	TEST_VARIATION(16, 		L"Stored Procedure with Valid OUTPUT only parameters.")
	TEST_VARIATION(17, 		L"Stored procedure with NULL [out] and Valid [input] parameters.")
	TEST_VARIATION(18, 		L"Stored procedure with NULL Output only parameters.")
	TEST_VARIATION(19, 		L"Stored procedure with NULL input and Valid Output parameters.")
	TEST_VARIATION(20, 		L"Input/Output parameters with only Status bound to NULL")
	TEST_VARIATION(21, 		L"Output only parameters with Status only binding to NULL")
	TEST_VARIATION(22, 		L"Input/output with status only binding to S_OK")
	TEST_VARIATION(23, 		L"Output only with status only binding to S_OK")
	TEST_VARIATION(24, 		L"Input/Output with Length and Value only bound.")
	TEST_VARIATION(25, 		L"Output only with Length and Value bound")
	TEST_VARIATION(26, 		L"Input/Output with Value and Status bound")
	TEST_VARIATION(27, 		L"output only with Value and Status bound")
	TEST_VARIATION(28, 		L"Input/output with only Value bound")
	TEST_VARIATION(29, 		L"output only with Value only bound.")
	TEST_VARIATION(30, 		L"INPUT/OUTPUT with BY_REF bindings")
	TEST_VARIATION(31, 		L"OUTPUT_ONLY with BY_REF bindings.")
	TEST_VARIATION(32, 		L"Return value for a stored procedure.")
	TEST_VARIATION(33, 		L"Bind same output parameters multiple number of times.")
	TEST_VARIATION(34, 		L"1 parameter Set.")
	TEST_VARIATION(35, 		L"5 parameter sets")
	TEST_VARIATION(36, 		L"105 parameter sets.")
	TEST_VARIATION(37, 		L"Different types of Good and Bad parameter Sets.")
	TEST_VARIATION(38, 		L"Having a table such that Variable length columns make up first and last columns")
	TEST_VARIATION(39, 		L"Having BLOB as first and last columns")
	TEST_VARIATION(40, 		L"5 parameter sets with different size of Variable length parameters.")
	TEST_VARIATION(41, 		L"5 parameters sets followed by 1 parameter set.")
	TEST_VARIATION(42, 		L"BYREF accessor with PROVIDER_OWNED memory should fail.")
	TEST_VARIATION(43, 		L"RestartPosition should either return S_OK or DB_E_CANNOTRESTART")
	TEST_VARIATION(44, 		L"Named Parameters: Sproc parm count != bound parm count")
	TEST_VARIATION(45, 		L"Named Parameters: Sproc parm order != binding order")
	TEST_VARIATION(46, 		L"Named Parameters: Use all parameter names")
	TEST_VARIATION(47, 		L"Named Parameters: Retrieve names and use without setting")
	TEST_VARIATION(48, 		L"Named Parameters: Named return parameter from sproc")
	TEST_VARIATION(49, 		L"Inline binding with more than 64K and embedded null term")
	TEST_VARIATION(50, 		L"Insert with BSTR bindings to strings")
	TEST_VARIATION(51, 		L"OpenRowset on a stored proc with params")
	TEST_VARIATION(52, 		L"DBSTATUS_S_DEFAULT - Default values for param")
	TEST_VARIATION(53, 		L"Multiple procs - execute two procs without SetParamInfo or Prepare")
	TEST_VARIATION(54, 		L"Multiple procs - execute two procs with partial SetParamInfo and Prepare")
	TEST_VARIATION(55, 		L"DBSTATUS_S_IGNORE - Not legal for params (DBSTATUS_E_BADSTATUS)")
	TEST_VARIATION(56, 		L"OpenRowset on stored proc without params")
	TEST_VARIATION(57, 		L"Select with DBSTATUS_S_ISNULL input params")
	TEST_VARIATION(58, 		L"S_OK - Send less data than max for parameter")
	TEST_VARIATION(59, 		L"S_OK - Insert proc with CANHOLDROWS OPTIONAL")
	TEST_VARIATION(60, 		L"Insert with BSTR bindings to strings to stored proc")
	TEST_VARIATION(61, 		L"S_OK: Input and output params with cParamSets > 1")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCSetParameterInfo_Rowset)
//--------------------------------------------------------------------
// @class Test case for SetParameterInfo
//
class TCSetParameterInfo_Rowset : public CICmdWParams { 
protected:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSetParameterInfo_Rowset,CICmdWParams);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Call SetParameterInfo.  GetParameterInfo. Should match.
	int Variation_1();
	// @cmember SetParameterInfo.  Execute.  S_OK
	int Variation_2();
	// @cmember SetParameterInfo,	Execute.  Use Rowset.  DescribeParams.
	int Variation_3();
	// @cmember SetParameterInfo.  Remove SetParameterInfo.  GetParameterInfo. return original or fail
	int Variation_4();
	// @cmember SetParameterInfo for all.  Delete a few.  Override a few.  Call GetParameterInfo and verify.
	int Variation_5();
	// @cmember (valid, NULL, NULL
	int Variation_6();
	// @cmember Bstr = NULL.  What happens?
	int Variation_7();
	// @cmember SetParameterInfo with Illegal Coersions.  Execute should fail.
	int Variation_8();
	// @cmember rgParamOrdinals[] = ULONG_MAX.  what happens.
	int Variation_9();
	// @cmember Call execute with params before SetParameterInfo
	int Variation_10();
	// @cmember Wrong SetParameterInfo.  Verify DP returns the same.
	int Variation_11();
	// @cmember Delete first parameter, last parameter.  Verify.
	int Variation_12();
	// @cmember Call SetParameterInfo for Output Parameters.
	int Variation_13();
	// @cmember Call SetParameterInfo for Input/Output parameters.
	int Variation_14();
	// @cmember Call SetParameterInfo for All DBTYPES in the SPEC and Verify.
	int Variation_15();
	// @cmember Call SetParameterInfo for Native types and Verify.
	int Variation_16();
	// @cmember Open Rowset , and then call SetParameterInfo returns DB_E_OBJECTOPEN.
	int Variation_17();
	// @cmember Set parameter for variable length char fields and execute
	int Variation_18();
	// @cmember SetParameterInfo with reverse rgParamOrdinals
	int Variation_19();
	// @cmember SetParameterInfo with random rgParamOrdinals
	int Variation_20();
	// @cmember SetParameterInfo with duplicate parameter name
	int Variation_21();
	// @cmember SetParameterInfo with invalid parameter name
	int Variation_22();
	// @cmember Default Conversion: pwszDataSourceType == NULL
	int Variation_23();
	// @cmember Verify error on DBTYPE_NUMERIC with no precision
	int Variation_24();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSetParameterInfo_Rowset)
#define THE_CLASS TCSetParameterInfo_Rowset
BEG_TEST_CASE(TCSetParameterInfo_Rowset, CICmdWParams, L"Test case for SetParameterInfo")
	TEST_VARIATION(1, 		L"Call SetParameterInfo.  GetParameterInfo. Should match.")
	TEST_VARIATION(2, 		L"SetParameterInfo.  Execute.  S_OK")
	TEST_VARIATION(3, 		L"SetParameterInfo,	Execute.  Use Rowset.  DescribeParams.")
	TEST_VARIATION(4, 		L"SetParameterInfo.  Remove SetParameterInfo.  GetParameterInfo. return original or fail")
	TEST_VARIATION(5, 		L"SetParameterInfo for all.  Delete a few.  Override a few.  Call GetParameterInfo and verify.")
	TEST_VARIATION(6, 		L"(valid, NULL, NULL")
	TEST_VARIATION(7, 		L"Bstr = NULL.  What happens?")
	TEST_VARIATION(8, 		L"SetParameterInfo with Illegal Coersions.  Execute should fail.")
	TEST_VARIATION(9, 		L"rgParamOrdinals[] = ULONG_MAX.  what happens.")
	TEST_VARIATION(10, 		L"Call execute with params before SetParameterInfo")
	TEST_VARIATION(11, 		L"Wrong SetParameterInfo.  Verify DP returns the same.")
	TEST_VARIATION(12, 		L"Delete first parameter, last parameter.  Verify.")
	TEST_VARIATION(13, 		L"Call SetParameterInfo for Output Parameters.")
	TEST_VARIATION(14, 		L"Call SetParameterInfo for Input/Output parameters.")
	TEST_VARIATION(15, 		L"Call SetParameterInfo for All DBTYPES in the SPEC and Verify.")
	TEST_VARIATION(16, 		L"Call SetParameterInfo for Native types and Verify.")
	TEST_VARIATION(17, 		L"Open Rowset , and then call SetParameterInfo returns DB_E_OBJECTOPEN.")
	TEST_VARIATION(18, 		L"Set parameter for variable length char fields and execute")
	TEST_VARIATION(19, 		L"SetParameterInfo with reverse rgParamOrdinals")
	TEST_VARIATION(20, 		L"SetParameterInfo with random rgParamOrdinals")
	TEST_VARIATION(21, 		L"SetParameterInfo with duplicate parameter name")
	TEST_VARIATION(22, 		L"SetParameterInfo with invalid parameter name")
	TEST_VARIATION(23, 		L"Default Conversion: pwszDataSourceType == NULL")
	TEST_VARIATION(24, 		L"Verify error on DBTYPE_NUMERIC with no precision")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public CICmdWParams { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,CICmdWParams);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid GetParameterInfo calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid GetParameterInfo calls with previous error object existing
	int Variation_2();
	// @cmember Invalid GetParameterInfo calls with no previous error object existing
	int Variation_3();
	// @cmember Valid SetParameterInfo calls with previous error object existing.
	int Variation_4();
	// @cmember Invalid SetParameterInfo calls with previous error object existing
	int Variation_5();
	// @cmember Invalid SetParameterInfo calls with no previous error object existing
	int Variation_6();
	// @cmember Valid MapParameterNames calls with previous error object existing.
	int Variation_7();
	// @cmember Invalid MapParameterNames calls with previous error object existing
	int Variation_8();
	// @cmember Invalid MapParameterNames calls with no previous error object existing
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, CICmdWParams, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid GetParameterInfo calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid GetParameterInfo calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid GetParameterInfo calls with no previous error object existing")
	TEST_VARIATION(4, 		L"Valid SetParameterInfo calls with previous error object existing.")
	TEST_VARIATION(5, 		L"Invalid SetParameterInfo calls with previous error object existing")
	TEST_VARIATION(6, 		L"Invalid SetParameterInfo calls with no previous error object existing")
	TEST_VARIATION(7, 		L"Valid MapParameterNames calls with previous error object existing.")
	TEST_VARIATION(8, 		L"Invalid MapParameterNames calls with previous error object existing")
	TEST_VARIATION(9, 		L"Invalid MapParameterNames calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCBugRegressions)
//--------------------------------------------------------------------
// @class Test case to hold the regressions for bugs reported by other groups.
//
class TCBugRegressions : public CICmdWParams { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBugRegressions,CICmdWParams);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Cmd that executes SP with param type adText returns truncation error if string > 255 chars
	int Variation_1();
	// @cmember Prepare, Execute, Prepare, Execute w/o rebinding
	int Variation_2();
	// @cmember Retrieve a row after failing to retrieve with DBSTATUS_S_ISNULL
	int Variation_3();
	// @cmember Use int64 as input and output param binding in stored proc
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCBugRegressions)
#define THE_CLASS TCBugRegressions
BEG_TEST_CASE(TCBugRegressions, CICmdWParams, L"Test case to hold the regressions for bugs reported by other groups.")
	TEST_VARIATION(1, 		L"Cmd that executes SP with param type adText returns truncation error if string > 255 chars")
	TEST_VARIATION(2, 		L"Prepare, Execute, Prepare, Execute w/o rebinding")
	TEST_VARIATION(3, 		L"Retrieve a row after failing to retrieve with DBSTATUS_S_ISNULL")
	TEST_VARIATION(4, 		L"Use int64 as input and output param binding in stored proc")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCIUnknown)
//--------------------------------------------------------------------
// @class Test parameter bindings to IUnknown
//
class TCIUnknown : public COLEDB { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	// @cmember Default table object
	CTable * m_pTable;

	// @cmember Supported structured storage mask
	ULONG_PTR m_ulStorageSupport;

	// @cmember Defined storage flags
	DWORD m_lStorageFlags;

	// @cmember Whether provider is Kagera or not
	BOOL m_fKagera;

	// @cmember Result of test
	TESTRESULT m_TestResult;

	// @cmember HRESULT used for intermediate results
	HRESULT	m_hr;

	// @cmember Expected HRESULT
	HRESULT m_hrExpect;

	// @cmember Flag for interface support
	BOOL m_fSupportInterface;

	// @cmember Bind status array
	DBBINDSTATUS m_rgStatus[1];

	// @cmember Command object pointer
	ICommand * m_pICommand;

	// @cmember Command text object pointer
	ICommandText * m_pICommandText;

	// @cmember Command parameter object pointer
	ICommandWithParameters * m_pICommandWithParameters;

	// @cmember Properties interface pointer
	IDBProperties * m_pIDBProperties;

	// @cmember IAccessor object pointer
	IAccessor * m_pIAccessor;

	// @cmember Local implementation of ISequentialStream
	IUnknown * m_pIUnkVerifyObj;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIUnknown,COLEDB);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// @cmember Returns a pointer to a data item for a column
	LPVOID MakeData(CCol ColInfo, ULONG * pcbDataSize, ULONG ulSeed, enum EVALUE eValue = SECONDARY);

	// @cmember IUnknown conversion check
	HRESULT CanConvertIUnknown(IUnknown * pIUnknown, DBTYPE wType, DBCONVERTFLAGS dwConvertFlags);

	// @cmember Returns TRUE if a type is a BLOB type
	BOOL IsBLOB(DBTYPE wType);

	// @cmember Creates the desired storage object for test or verification
	HRESULT CreateStorageObject(REFIID iidObject, IUnknown ** ppIObject);

	// @cmember Tests whether last byte to be sent is a lead byte.
	BOOL IsLastByteLeadByte(LPVOID pvSrcData, ULONG cbSrcData, ULONG cbReadSize);

	// @cmember Worker function to encapsulate common active error test code
	TESTRESULT TestActiveError(enum ACTIVE_ERROR eTest, REFIID iidParamObject);

	// @cmember Worker function to encapsulate common storage object test code
	TESTRESULT TestStorageObject(enum PARAM_OBJECT_TEST	eTest, REFIID iidParamObject,
		REFIID	iidVerifyObject, DB_UPARAMS cParamSets = 1);

	BOOL MakeUnknownBinding(
		DBCOUNTITEM iBind,
		DBBINDING * pBind,
		DB_UPARAMS * pParamOrdinals,
		DBPARAMBINDINFO * pParamBind,
		LPBYTE pData,
		DBOBJECT * pParamStorageObjects,
		REFIID iidParamObject,
		DWORD dwFlags,
		DBLENGTH cbRowSize,
		DB_UPARAMS cParamSets,
		ULONG ulUpdateRow,
		CCol ColInfo,
		enum PARAM_OBJECT_TEST eTest
	);

	// {{ TCW_TESTVARS()
	// @cmember ISequentialStream
	int Variation_1();
	// @cmember ILockBytes
	int Variation_2();
	// @cmember IStorage
	int Variation_3();
	// @cmember IStream
	int Variation_4();
	// @cmember DBPROP_MULTIPLESTORAGEOBJECTS
	int Variation_5();
	// @cmember DBPROP_BLOCKINGSTORAGEOBJECTS
	int Variation_6();
	// @cmember NULL Storage Object - S_OK
	int Variation_7();
	// @cmember Empty storage object - S_OK
	int Variation_8();
	// @cmember DBSTATUS_S_ISNULL
	int Variation_9();
	// @cmember Verify streams active after error
	int Variation_10();
	// @cmember Verify streams released after error
	int Variation_11();
	// @cmember Multiple Paramsets
	int Variation_12();
	// @cmember Multiple Storage Objects
	int Variation_13();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCIUnknown)
#define THE_CLASS TCIUnknown
BEG_TEST_CASE(TCIUnknown, COLEDB, L"Test parameter bindings to IUnknown")
	TEST_VARIATION(1, 		L"ISequentialStream")
	TEST_VARIATION(2, 		L"ILockBytes")
	TEST_VARIATION(3, 		L"IStorage")
	TEST_VARIATION(4, 		L"IStream")
	TEST_VARIATION(5, 		L"DBPROP_MULTIPLESTORAGEOBJECTS")
	TEST_VARIATION(6, 		L"DBPROP_BLOCKINGSTORAGEOBJECTS")
	TEST_VARIATION(7, 		L"NULL Storage Object - S_OK")
	TEST_VARIATION(8, 		L"Empty storage object - S_OK")
	TEST_VARIATION(9, 		L"DBSTATUS_S_ISNULL")
	TEST_VARIATION(10, 		L"Verify streams active after error")
	TEST_VARIATION(11, 		L"Verify streams released after error")
	TEST_VARIATION(12, 		L"Multiple Paramsets")
	TEST_VARIATION(13, 		L"Multiple Storage Objects")
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

COPY_TEST_CASE(TCGetParameterInfo_Row,	TCGetParameterInfo_Rowset)
COPY_TEST_CASE(TCCommandExecute_Row,	TCCommandExecute_Rowset)
COPY_TEST_CASE(TCSetParameterInfo_Row,	TCSetParameterInfo_Rowset)

#if 0
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(7, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCGetParameterInfo_Rowset)
	TEST_CASE(2, TCMapParameterNames)
	TEST_CASE(3, TCCommandExecute_Rowset)
	TEST_CASE(4, TCSetParameterInfo_Rowset)
	TEST_CASE(5, TCExtendedErrors)
	TEST_CASE(6, TCBugRegressions)
	TEST_CASE(7, TCIUnknown)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(10, ThisModule, gwszModuleDescrip)
	TEST_CASE_WITH_PARAM(1, TCGetParameterInfo_Rowset, TC_Rowset)
	TEST_CASE_WITH_PARAM(2, TCGetParameterInfo_Row, TC_Row)

	TEST_CASE(3, TCMapParameterNames)

	TEST_CASE_WITH_PARAM(4, TCCommandExecute_Rowset, TC_Rowset)
	TEST_CASE_WITH_PARAM(5, TCCommandExecute_Row, TC_Row)

	TEST_CASE_WITH_PARAM(6, TCSetParameterInfo_Rowset, TC_Rowset)
	TEST_CASE_WITH_PARAM(7, TCSetParameterInfo_Row, TC_Row)

	TEST_CASE(8, TCExtendedErrors)
	TEST_CASE(9, TCBugRegressions)
	TEST_CASE(10, TCIUnknown)
END_TEST_MODULE()
#endif

// {{ TCW_TC_PROTOTYPE(TCGetParameterInfo_Rowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetParameterInfo_Rowset - Test case for GetParameterInfo.
//|	Created:			03/30/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetParameterInfo_Rowset::Init()
{
	if (m_eTestCase == TC_Row && !g_fRowObj)
	{
		odtLog << L"Row objects not supported.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CICmdWParams::Init())
	// }}
	{
		// Now proceed.
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Call GetParameterInfo after SetCommandText.  Delete text and call GetParameterInfo again.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_1()
{
	BOOL fResult = TRUE;

	// Call ICommandPrepare.
	ABORT_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	ABORT_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);
	
	// Compare parameter information.
	FAIL_VAR(CompareDBParamInfo(m_cBindings, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer), S_OK);
	
	// Now set command text to empty.
	ABORT_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , L""), S_OK);

	// Free stuff for second call.
	FreeDescParams();

	if (m_bSetParameterInfo)
	{
		// Since parameter information is already set GetParameterInfo returns S_OK.  
		// Since return values have already been verified. No need to Check now.
		ABORT_CHECK(m_pICmdWParams->GetParameterInfo(
			&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

		FAIL_VAR(CompareDBParamInfo(m_cBindings, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer), S_OK);
	}
	else
	{
		HRESULT hrGet = DB_E_NOCOMMAND;
		HRESULT hr = E_FAIL;

		if (m_pICommandPrepare)
			hrGet = DB_E_NOTPREPARED;

		hr = m_pICmdWParams->GetParameterInfo(
			&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);

		ABORT_CHECK(hr, hrGet);

		FAIL_VAR(CompareDBParamInfo(0, NULL, NULL, m_rgParamInfo, m_pNamesBuffer), S_OK);
	}
	
CLEANUP:
	
	// Free stuff.
	FreeDescParams();		

	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Call GetParameterInfo after ICommandPrepare.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_2()
{
	BOOL fResult = FALSE;

	// Set command text
	TEST_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	TEST_CHECK(m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

	CompareDBParamInfo(m_cBindings,  m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer);
	
	// Call ICommandPrepare.
	TEST_CHECK(m_pICommandPrepare->Unprepare(), S_OK);

	// Now call Describe Parameters.
	// Free stuff for second call.
	FreeDescParams();

	
	if (m_bSetParameterInfo)
	{
		// Since parameter information is already set GetParameterInfo returns S_OK.  
		// Since return values have already been verified. No need to Check now.
		TEST_CHECK(m_pICmdWParams->GetParameterInfo(
			&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);
	}
	else
	{
		TEST_CHECK(m_pICmdWParams->GetParameterInfo(
			&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), DB_E_NOTPREPARED);
	}
	
	fResult = TRUE;
	
CLEANUP:

	FreeDescParams();

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Call GetParameterInfo after execute and release
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_3()
{
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	IRowset * pRowset = NULL;
	IRowset * pIRowset = NULL;
	DBCOUNTITEM ulRowNum;
	ULONG i;

	// To cover some possible values lets do execute in a loop.

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	for (i = 0; i < 10 ; i++ )
	{
		DBCOUNTITEM cRowsExpected = 1;

		MakeDataForCommand((ulRowNum = NextInsertRowNum()));
		
		// Now execute ICommand.
		TEST_CHECK(m_pICommand->Execute(NULL, IID_IUnknown,
				&m_DbParamsAll, &cRowsAffected, (IUnknown **)&pRowset), S_OK);

		// Verify If we have inserted the row properly.
		TEST_COMPARE(cRow.FindRow(ulRowNum, m_pTable, NULL, &pIRowset, NULL,
			0, NULL, FALSE), TRUE);

		// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
		TEST_COMPARE(VerifyObj(m_iidExec, pIRowset, ulRowNum, 0, NULL,
			FALSE, FALSE, m_pTable, &cRowsExpected), S_OK);

		SAFE_RELEASE(pIRowset);
		SAFE_RELEASE(pRowset);

	}
	TEST_CHECK(m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

	CompareDBParamInfo(m_cBindings,  m_rgParamColMap,	m_cParams, m_rgParamInfo, m_pNamesBuffer);
	
	if (pRowset) ReleaseRowsetPtr (&pRowset);
	FreeDescParams();

	TEST_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);
	

	fResult = TRUE;
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pRowset);
	FreeDescParams();
	ReleaseDataForCommand();
	if (pRowset)
		ReleaseRowsetPtr(&pRowset);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid SQL with no parameters.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_4()
{
	WCHAR *pwszSqlSelectAllFromTbl = NULL;
	BOOL fResult = FALSE;

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);

	// Call ICommandPrepare.
	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	FreeDescParams();
	TEST_CHECK(m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

	
	CompareDBParamInfo(m_cBindings, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer);


	//Replace text in CommandObject With select statment with no parameters.
	TEST_CHECK(m_hr = m_pTable->CreateSQLStmt(SELECT_COLLISTFROMTBL, NULL, &pwszSqlSelectAllFromTbl, NULL, NULL ), S_OK);

	// Set command text
	TEST_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , pwszSqlSelectAllFromTbl), S_OK);

	// Call ICommandPrepare.
	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	FreeDescParams();
	TEST_CHECK(m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);


	if (m_bSetParameterInfo)
	{
		// In case we did SetParameterInfo, expect those parameters.
		TEST_COMPARE (m_cParams, m_cDbParamBindInfo );
	}
	else
	{
		// We have not called SetParameterInfo.
		// Text is prepared with no parameters expect 0.
		TEST_COMPARE (m_cParams, 0);
	}

	fResult = TRUE;

CLEANUP:

			
	// Restore original text.
	if (!CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK))
		fResult = FALSE;

	FREE_DATA (pwszSqlSelectAllFromTbl);
	FreeDescParams();

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc No command object.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_5()
{
	
	BOOL fResult = FALSE;
	ICommandWithParameters *pICommandWithParameters = NULL;
	ICommandText *pLocalICommandText = NULL;
	HRESULT hr = E_FAIL;
	
	
	if (!VerifyInterface(m_pIEmptyCommand, IID_ICommandWithParameters, 
		COMMAND_INTERFACE, (IUnknown **)&pICommandWithParameters))
	{
		goto CLEANUP;
	}

	// For m_pIEmptyCommand No command text interface is present.
	// Describe parameters should return DB_E_NOTPREPARED.

	hr = pICommandWithParameters->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);

	if (m_bSetParameterInfo)
	{
		// Provider doesn't derive and requires setting parameter information.
		TEST_CHECK(hr, DB_E_PARAMUNAVAILABLE);
	
	}
	else
	{
		// Provider can derive
		// Some providers may (legally) return DB_E_NOTPREPARED since this was in the spec for this 
		// condition under GetParameterInfo.  But the front matter command state table shows 
		// DB_E_NOCOMMAND.  So we will warn for DB_E_NOTPREPARED.
		if (hr == DB_E_NOTPREPARED)
			TESTW_(hr, DB_E_NOCOMMAND);
		else
		{
			TEST_CHECK(hr, DB_E_NOCOMMAND);
		}
	}


	// Now get text interface but do not set text.
	// DB_E_NOTPREPARED should be returned.
	
	if (!VerifyInterface(m_pIEmptyCommand, IID_ICommandText, 
		COMMAND_INTERFACE, (IUnknown **)&pLocalICommandText))
	{
		goto CLEANUP;   
	}

	hr = pICommandWithParameters->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);

	if (m_bSetParameterInfo)
	{
		// Provider doesn't derive and requires setting parameter information.
		TEST_CHECK(hr, DB_E_PARAMUNAVAILABLE);
	
	}
	else
	{
		// Provider can derive
		// Some providers may (legally) return DB_E_NOTPREPARED since this was in the spec for this 
		// condition under GetParameterInfo.  But the front matter command state table shows 
		// DB_E_NOCOMMAND.  So we will warn for DB_E_NOTPREPARED.
		if (hr == DB_E_NOTPREPARED)
			TESTW_(hr, DB_E_NOCOMMAND);
		else
		{
			TEST_CHECK(hr, DB_E_NOCOMMAND);
		}
	}
	fResult = TRUE;

CLEANUP:
	if (pICommandWithParameters)
		pICommandWithParameters->Release();

	if (pLocalICommandText)
		pLocalICommandText->Release();

	return (fResult) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Call GetParameterInfo twice.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_6()
{

	BOOL fResult = FALSE;
	
	// Just to be sure set the appropriate text. Restore original text.
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);

	// Call ICommandPrepare.
	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	TEST_CHECK(m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

	CompareDBParamInfo(m_cBindings, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer);
	
	FreeDescParams();
	TEST_CHECK(m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

	// Second call should yield the same results.
	CompareDBParamInfo(m_cBindings, m_rgParamColMap,	m_cParams, m_rgParamInfo, m_pNamesBuffer);

	fResult = TRUE;
CLEANUP:

	FreeDescParams();

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Parameters (NULL, NULL, Valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_7()
{
	// 
	FreeDescParams();
	if (!CHECK(m_pICmdWParams->GetParameterInfo(NULL, NULL, &m_pNamesBuffer),
		E_INVALIDARG))
	{
		FreeDescParams();		
		return TEST_FAIL;
	}

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Parameters(NULL, Valid, Valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_8()
{
	FreeDescParams();
	if (!CHECK(m_pICmdWParams->GetParameterInfo(NULL, &m_rgParamInfo, &m_pNamesBuffer),
		E_INVALIDARG))
	{
		FreeDescParams();		
		return TEST_FAIL;
	}

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Parameters (Valid, NULL, Valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_9()
{
	FreeDescParams();
	if (!CHECK(m_pICmdWParams->GetParameterInfo(&m_cParams, NULL, &m_pNamesBuffer),
		E_INVALIDARG))
	{
		FreeDescParams();		
		return TEST_FAIL;
	}

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Parameters (NULL, NULL, NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_10()
{
	FreeDescParams();
	if (!CHECK(m_pICmdWParams->GetParameterInfo(NULL, NULL, NULL),
		E_INVALIDARG))
	{
		FreeDescParams();		
		return TEST_FAIL;
	}

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Parameters (Valid, Valid, NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_11()
{

	FreeDescParams();
	if (!CHECK(m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, NULL),
		S_OK))
	{
		FreeDescParams();		
		return TEST_FAIL;
	}
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//--------------------------------------------------------------------
// @mfunc Call GetParameterInfo after SetCommandText for stored proc with INPUT params
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_12()
{
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSetProp = E_FAIL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	// Oracle cannot return a rowset from within a stored proc at this time.
	if (g_bOracle)
	{
		odtLog << L"Oracle cannot return a rowset from a stored proc.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with input parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_IN,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd
	), TRUE);

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on
	if (SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);
	
	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_MAYBE, cColumns, prgColumnsOrd, fCanDerive), S_OK);

CLEANUP:

	// If we set RANDOM REQUIRED above we need to set back to default
	if (hrSetProp == S_OK)
		CHECK(SetRowsetPropertyDefault(DBPROP_ACCESSORDER), S_OK);

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Call GetParameterInfo after calling SetCommandText with a stored procedure with OUTPUT params
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_13()
{

	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 2;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_bOracle)
	{
		odtLog << "Skipping comparison of ulParamSize, bPrecision, and bScale for Oracle DBMS.\n";
	}


	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_NONE, cColumns, prgColumnsOrd, fCanDerive), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Call GetParamInfo after calling SetCommandText with a stored procedure with IN/OUT params.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_14()
{

	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_bOracle)
	{
		odtLog << "Skipping comparison of ulParamSize, bPrecision, and bScale for Oracle DBMS.\n";
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_NONE, cColumns, prgColumnsOrd, fCanDerive, 
		VERIFY_USE_PDATA), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Call GetParameterInfo after SetCommandText with StoredProc with VeryLongParamNames
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_15()
{

	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hr = E_FAIL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_bOracle)
	{
		odtLog << "Skipping comparison of ulParamSize, bPrecision, and bScale for Oracle DBMS.\n";
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		NULL,
		NULL,
		NULL,
		CREATE_LONG_NAMES
	), TRUE);

	// Set up to execute the stored proc
	hr = CreateStoredProc(m_pICommandText, pwszProcName, pwszCreateProcStmt, FALSE);

	if (FAILED(hr))
	{
		fResult = CHECK(hr, DB_E_ERRORSINCOMMAND);
		goto CLEANUP;
	}

	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_NONE, cColumns, prgColumnsOrd, fCanDerive, 
		VERIFY_USE_PDATA), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Call GetParamInfo with a stored procedure which is returning a value.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_16()
{
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	WCHAR * pwszRetTypeName = NULL;
	DBTYPE wRetType = DBTYPE_EMPTY;
	ULONG iType;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_bOracle)
	{
		odtLog << "Skipping comparison of ulParamSize, bPrecision, and bScale for Oracle DBMS.\n";
	}

	// It is provider specific what types can be returned from a sproc.  Retrieve a valid
	// type name for this provider.
	if (!(pwszRetTypeName = m_Syntax.GetSyntax(T_RET_TYPE_NAME)))
		goto CLEANUP;

	// Find the actual type in the standard type name list
	for (iType = 0; iType < NUMELEM(g_rgStdParamBindInfo); iType++)
	{
		if (!wcscmp(pwszRetTypeName, g_rgStdParamBindInfo[iType].wszStdTypeName))
		{
			wRetType = g_rgStdParamBindInfo[iType].wType;
			break;
		}
	}

	ABORT_COMPARE(wRetType != DBTYPE_EMPTY, TRUE);

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT_RET,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		wRetType,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt, TRUE), S_OK);

	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_NONE, cColumns, prgColumnsOrd, fCanDerive), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName, TRUE);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	PROVIDER_FREE(pwszRetTypeName);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc GetParameterInfo with text set to different types of SQL statments with funny characters to test parsing code of Dev.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_17()
{
	//@todo  More case of strange procedure names can be added if Execute(Params)chokes on this one.

	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcNameRoot=L"\"''Stra# _* & % n  ";
	WCHAR * pwszProcName = NULL;
	WCHAR * pwszRootName = NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	DBLITERALINFO* pTableLiteral = m_pTable->GetLiteralInfo(DBLITERAL_TABLE_NAME);


	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_bOracle)
	{
		odtLog << "Skipping comparison of ulParamSize, bPrecision, and bScale for Oracle DBMS.\n";
		pwszProcNameRoot = L"\"Stra# _* & % n  ";
	}

	// Allocate a block for the final proc name
	SAFE_ALLOC(pwszProcName, WCHAR, pTableLiteral->cchMaxLen);

	// Create a name containing the funny characters, but also a unique portion to prevent
	// duplicate names when two tests running at once
	pwszRootName = MakeObjectName(pwszProcNameRoot, pTableLiteral->cchMaxLen-1); // -1 to allow for closing quote
	TEST_PTR(pwszRootName);

	// In Intl scenarios privlib will ignore the pwszProcNameRoot and instead create a name
	// containing int'l chars.  This is not quite what we want here
	if ( GetModInfo()->GetLocaleInfo() && GetModInfo()->GetUseIntlIdentifier() )
	{
		wcsncpy(pwszProcName, pwszProcNameRoot, pTableLiteral->cchMaxLen);
		wcsncpy(pwszProcName+wcslen(pwszProcNameRoot), pwszRootName, pTableLiteral->cchMaxLen -  wcslen(pwszProcNameRoot));
	}
	else
	{
		// Copy the root name
		wcscpy(pwszProcName, pwszRootName);
	}

	// Add trailing double quote
	wcscat(pwszProcName,L"\"");

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I4,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_NONE, cColumns, prgColumnsOrd, fCanDerive), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);

	SAFE_FREE(pwszProcName);
	SAFE_FREE(pwszRootName);
	
	return fResult ? TEST_PASS : TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Stored procedure execution on two different command objects.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetParameterInfo_Rowset::Variation_18()
{
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	ICommand * pICommand2 = NULL;
	ICommandText * pICommandText2 = NULL;
	ICommandPrepare * pICommandPrepare2 = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_bOracle)
	{
		odtLog << "Skipping comparison of ulParamSize, bPrecision, and bScale for Oracle DBMS.\n";
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I4,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_NONE, cColumns, prgColumnsOrd, fCanDerive), S_OK);

	// Create a second command object.
	ABORT_CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&pICommand2), S_OK);

	// Get the Interface for a Command Text Object.
	ABORT_COMPARE(VerifyInterface(pICommand2, IID_ICommandText,
			COMMAND_INTERFACE,(IUnknown **)&pICommandText2), TRUE);

	VerifyInterface(pICommand2, IID_ICommandPrepare,
		COMMAND_INTERFACE,(IUnknown **)&pICommandPrepare2);

	ABORT_CHECK(pICommandText2->SetCommandText(DBGUID_DBSQL, pwszExecProcStmt), S_OK);

		// Prepare if supported
	if (pICommandPrepare2)
		ABORT_CHECK(pICommandPrepare2->Prepare(1), S_OK);
	
	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_NONE, cColumns, prgColumnsOrd, fCanDerive,
		VERIFY_USE_TABLE, pICommand2), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);

	SAFE_RELEASE(pICommandPrepare2);
	SAFE_RELEASE(pICommandText2);
	SAFE_RELEASE(pICommand2);
	
	return (fResult) ? TEST_PASS : TEST_FAIL;

}
// }}




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Maximum length parameter names
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetParameterInfo_Rowset::Variation_19()
{ 

	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_bOracle)
	{
		odtLog << "Skipping comparison of ulParamSize, bPrecision, and bScale for Oracle DBMS.\n";
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		NULL,
		NULL,
		NULL,
		CREATE_MAX_NAMES
	), TRUE);

	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_NONE, cColumns, prgColumnsOrd, fCanDerive, 
		VERIFY_USE_PDATA), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetParameterInfo_Rowset::Terminate()
{
	if (m_pICommandPrepare)
		m_pICommandPrepare->Unprepare();
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CICmdWParams::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCMapParameterNames)
//*-----------------------------------------------------------------------
//| Test Case:		TCMapParameterNames - Test case for MapParameterNames.
//|	Created:			03/30/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMapParameterNames::Init()
{
	BOOL fResult = TRUE;

	ULONG cParams, cParamSets = 1 ;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	DBPARAMBINDINFO * pParamBindInfo = NULL;
	
	
	DBPARAMFLAGS *rgDwParamFlags = NULL;
	HRESULT hr;
	
	// Assume we'll include LONG columns in the parameters
	m_fColTypes = INCLUDE_LONG_COLS;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CICmdWParams::Init())
	// }}
	{
		m_hrExpect=S_OK;	

		// For Kagera and 2.x drivers expect not implemented.
		if (m_dwDriverODBCVer > 0 && m_dwDriverODBCVer < 3)
			m_hrExpect=E_NOTIMPL;
		
		memset(&m_iNotTouched, NOT_TOUCHED_ORD, sizeof(DB_UPARAMS));

		m_fUseProc = (m_fProcedureSupport && g_ulOutParamsSupported != DBPROPVAL_OA_NOTSUPPORTED && m_Syntax.IsKnown());

		// Create a stored procedure and set the command text if we know the syntax for doing so.  Otherwise
		// we'll have to use the command text from the base class init (insert statement).
		if (m_fUseProc)
		{
			// We're assuming parameter names aren't available at this time
			ASSERT(!m_prgwszParameterNames);

			// Create the syntax and binding for a stored proc with output parameters
			ABORT_COMPARE(CreateProcBindings(
				T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
				TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
				1,						// [IN]	 Number of sets of parameters to be created
				DBTYPE_I2,				// [IN]  Return parameter type
				1,						// [IN]  Row number in table to select, insert, or update
				&cParams,				// [OUT] Count of params created
				&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
				&pBINDING,				// [OUT] Binding array for CreateAccessor
				&pParamOrdinals,
				&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
				&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
				&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
				&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
				&pwszProcName,			// [OUT] Name of procedure
				&pData,					// [OUT] Pointer to data for the parameters
				&pParamAll
			), TRUE);

			// Set up to execute the stored proc
			ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
				&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

			TEST_ALLOC(WCHAR *, m_prgwszParameterNames, 0, (cParams+1) * sizeof(WCHAR *));

			for (ULONG iParam = 0; iParam < cParams; iParam++)
			{
				if (!(m_prgwszParameterNames[iParam] = wcsDuplicate(pPARAMBIND[iParam].pwszName)))
					goto CLEANUP;
			}

			m_cParamNames = cParams;
			m_prgParamOrdinals = pParamOrdinals;
			m_pParamBindInfo = pPARAMBIND;

			// If we can't derive we need to call SetParameterInfo
			if (!fCanDerive)
			{
				if (!CHECK(hr = m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND),
					S_OK))	// We set reset parameter info in PrepareForExecute.
					return FALSE;
			}

			return TRUE;
		}
		else
		{
			if (!m_bSetParameterInfo && m_pICommandPrepare)
				TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

			m_cParamNames = m_cDbParamBindInfo;
			m_pParamBindInfo = m_rgDbParamBindInfo;
		}


		// If we didn't get parameter names from the CreateStoredProcedure we need to create them here
		if (!m_prgwszParameterNames && !(m_prgwszParameterNames=CreateParameterNames(NULL, ALL_VALID_NAMES, &m_cParamNames, m_fColTypes)))
			return FALSE;

		if (!m_prgParamOrdinals && !(m_prgParamOrdinals=(DB_UPARAMS *)PROVIDER_ALLOC(m_cParamNames*sizeof(DB_UPARAMS))))
			return FALSE;

		// Copy the ordinals we had in base class init.
		memcpy(m_prgParamOrdinals, m_rgParamOrdinals, (size_t)(m_cParamNames * sizeof(DB_UPARAMS)));

		// For providers that support command preparation and can derive parameter information we need to prepare here
		// in case they check for NOT_PREPARED before other error conditions in the following variations.  For providers
		// that can't derive parameter information we need to be prepared before we call SetParameterInfo below.
		if (!m_bSetParameterInfo && m_pICommandPrepare && !CHECK(m_pICommandPrepare->Prepare(1), S_OK))
			return FALSE;

		// For providers that can't derive parameter information we need to set parameter names before MapParameterNames
		// can work.  Note we didn't set them in the base class init, we left them NULL in case the provider doesn't
		// support actually setting the names.  
		// We also need to call SetParameterInfo if not using a stored proc so that parameter names will be set where
		// they are not defined in the proc.
		if (m_bSetParameterInfo || !m_fUseProc)
		{
			// We're assuming the count of the names we created above matches the parameter count in base class init
			ASSERT(m_cDbParamBindInfo == m_cParamNames);
			SetParameterNames(m_prgwszParameterNames);
			CHECK(hr = m_pICmdWParams->SetParameterInfo(m_cParamNames, m_rgParamOrdinals, m_rgDbParamBindInfo),
				DB_S_TYPEINFOOVERRIDDEN);	// We set it with NULL names in base class init.

			// Even if we got a warning we can proceed
			if (!SUCCEEDED(hr))
				return FALSE;
		}


		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cParamNames = 0 : returns S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_1()
{
	LONG cParamNames = 3;
	WCHAR *rgParamNames[] = {L"ONE", L"TWO", L"THREE" };
	DB_LPARAMS *rgParamOrdinals = NULL;
	BOOL fSuccess = FALSE;

	if ((rgParamOrdinals = (DB_LPARAMS *)m_pIMalloc->Alloc(cParamNames *sizeof (DB_LPARAMS))) == NULL)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}

	TEST_CHECK (m_pICmdWParams->MapParameterNames(0, (const WCHAR **)rgParamNames, rgParamOrdinals), S_OK);
	fSuccess = TRUE;
CLEANUP:
	if (rgParamOrdinals)
		m_pIMalloc->Free(rgParamOrdinals);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cParamNames > 0 and NULL pointer for rgParamNames: returns E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_2()
{
	LONG cParamNames = 3;
	DB_LPARAMS *rgParamOrdinals = NULL;
	BOOL fSuccess = FALSE;

	if ((rgParamOrdinals = (DB_LPARAMS *)m_pIMalloc->Alloc(cParamNames *sizeof (DB_LPARAMS))) == NULL)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}

	for (LONG i = 0; i < cParamNames; i++ )
		rgParamOrdinals[i] = i+1;

	TEST_CHECK (m_pICmdWParams->MapParameterNames(cParamNames, NULL, rgParamOrdinals), E_INVALIDARG);
	
	fSuccess = TRUE;
CLEANUP:
	if (rgParamOrdinals)
		m_pIMalloc->Free(rgParamOrdinals);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc rgParamOrdinals with NULL : returns E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_3()
{
	LONG cParamNames = 3;
	WCHAR *rgParamNames[] = {L"ONE", L"TWO", L"THREE" };

	if (CHECK (m_pICmdWParams->MapParameterNames(cParamNames, (const WCHAR **)rgParamNames, NULL), E_INVALIDARG))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc (0, NULL, NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_4()
{
	
	if (CHECK (m_pICmdWParams->MapParameterNames (0, NULL, NULL), S_OK))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Map Parameter Names without prepared command: DB_E_NOTPREPARED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_5()
{
	BOOL fResult=TRUE;
	LONG cParamNames = 3;
	WCHAR *rgParamNames[] = {L"ONE", L"TWO", L"THREE" };
	DB_LPARAMS rgParamOrdinals[] = { LONG_MAX, LONG_MAX, LONG_MAX };
	HRESULT hr = E_FAIL;

	if (!m_pICommandPrepare)
	{
		odtLog << L"Provider doesn't support ICommandPrepare.\n\n";
		return TEST_SKIPPED;
	}

	// If we Prepared in the init, then we need to unprepare here
	if (!CHECK(m_pICommandPrepare->Unprepare(), S_OK))
		return TEST_FAIL;

	// If we've set parameter info unset it here.  We always set parameter info when not using a stored proc.
	if (!m_fUseProc)
		m_pICmdWParams->SetParameterInfo(0, NULL, NULL);

	// If the provider is capable of deriving error information, then expect DB_E_NOTPREPARED
	// here, else spec is unclear but since the interface is supported then all methods should be supported.
	// Some providers return E_NOTIMPL, and a review of the ole docs show that an existing method is 
	// allowed to return E_NOTIMPL if it doesn't support all semantics of a method.
	hr = m_pICmdWParams->MapParameterNames(cParamNames, (const WCHAR **)rgParamNames, rgParamOrdinals);

	if (hr == E_NOTIMPL)
	{
		odtLog << L"MapParameterNames not implemented.\n";
		return TEST_SKIPPED;
	}

	fResult = CHECK (hr, DB_E_NOTPREPARED);

	// Need to reprepare for the rest of the variations.
	if (!CHECK(m_pICommandPrepare->Prepare(1), S_OK))
		fResult = FALSE;

	return (fResult) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc MapParameterNames with all invalid parameter names: DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_6()
{
	TESTRESULT testresult=TEST_FAIL;
	LONG iOrdinal=0;
	WCHAR ** prgwszParameterNames;
	DB_LPARAMS * pParamOrdinals = NULL;
	ULONG iParam;
	HRESULT hr = E_FAIL;

	TEST_ALLOC(DB_LPARAMS, pParamOrdinals, NOT_TOUCHED_ORD, (size_t)(m_cParamNames*sizeof(DB_LPARAMS)));

	// Copy the name array
	TEST_ALLOC(WCHAR *, prgwszParameterNames, 0, (size_t)(m_cParamNames*sizeof(WCHAR *)));
	memcpy(prgwszParameterNames, m_prgwszParameterNames, (size_t)m_cParamNames*sizeof(WCHAR *));

	// Set the names to NULL
	for (iParam = 0; iParam < m_cParamNames; iParam++)
		prgwszParameterNames[iParam] = NULL;

	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	hr = m_pICmdWParams->MapParameterNames(m_cParamNames, (const WCHAR **)prgwszParameterNames, (DB_LPARAMS *)pParamOrdinals);

	if (hr == E_NOTIMPL)
	{
		odtLog << L"MapParameterNames not implemented.\n";
		testresult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TEST_CHECK (hr, DB_E_ERRORSOCCURRED);

	// Per spec on failure all ordinal values are set to 0
	for (iParam = 0; iParam < m_cParamNames; iParam++)
		TEST_COMPARE(pParamOrdinals[iParam], 0);

	// If we made it to here everything succeeded
	testresult=TEST_PASS;

CLEANUP:	

	PROVIDER_FREE(prgwszParameterNames); 
	PROVIDER_FREE(pParamOrdinals); 

	return testresult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc MapParameterNames with some invalid parameter names: DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_7()
{
	TESTRESULT testresult=TEST_FAIL;
	LONG iOrdinal=0;
	WCHAR ** prgwszParameterNames;
	DB_UPARAMS * pParamOrdinals = NULL;
	ULONG iParam;
	HRESULT hr = E_FAIL;

	TEST_ALLOC(DB_UPARAMS, pParamOrdinals, NOT_TOUCHED_ORD, (size_t)m_cParamNames*sizeof(DB_UPARAMS));

	// Copy the name array
	TEST_ALLOC(WCHAR *, prgwszParameterNames, 0, (size_t)m_cParamNames*sizeof(WCHAR *));
	memcpy(prgwszParameterNames, m_prgwszParameterNames, (size_t)m_cParamNames*sizeof(WCHAR *));

	// If not using stored proc we must call SetParameterInfo to set the names even if a provider
	// can derive.
	if (!m_fUseProc)
	{
		hr = m_pICmdWParams->SetParameterInfo(m_cParamNames, m_rgParamOrdinals, m_rgDbParamBindInfo);

		if (hr != DB_S_TYPEINFOOVERRIDDEN)
			CHECK(hr, S_OK);
	}

	// Set some of the names to NULL
	for (iParam = 0; iParam < m_cParamNames; iParam++)
	{
		if (iParam % 2)
			prgwszParameterNames[iParam] = NULL;
	}

	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	hr = m_pICmdWParams->MapParameterNames(m_cParamNames, (const WCHAR **)prgwszParameterNames, (DB_LPARAMS *)pParamOrdinals);

	if (hr == E_NOTIMPL)
	{
		odtLog << L"MapParameterNames not implemented.\n";
		testresult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TEST_CHECK (hr, DB_S_ERRORSOCCURRED);


	// Per spec on failure all ordinal values are set to 0
	for (iParam = 0; iParam < m_cParamNames; iParam++)
	{
		if (iParam % 2)
		{
			TEST_COMPARE(pParamOrdinals[iParam], 0);
		}
		else
		{
			TEST_COMPARE(pParamOrdinals[iParam], m_prgParamOrdinals[iParam]);
		}
	}

	// If we made it to here everything succeeded
	testresult=TEST_PASS;

CLEANUP:	

	PROVIDER_FREE(prgwszParameterNames); 
	PROVIDER_FREE(pParamOrdinals); 

	return testresult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc MapParameterNames with all valid names: S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_8()
{
	TESTRESULT testresult=TEST_FAIL;
	LONG iOrdinal=0;
	DB_UPARAMS * pParamOrdinals = NULL;
	ULONG iParam;
	HRESULT hr = E_FAIL;

	TEST_ALLOC(DB_UPARAMS, pParamOrdinals, NOT_TOUCHED_ORD, (size_t)m_cParamNames*sizeof(DB_UPARAMS));

	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	hr = m_pICmdWParams->MapParameterNames(m_cParamNames, (const WCHAR **)m_prgwszParameterNames, (DB_LPARAMS *)pParamOrdinals);

	if (hr == E_NOTIMPL)
	{
		odtLog << L"MapParameterNames not implemented.\n";
		testresult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TEST_CHECK (hr, S_OK);

	// Per spec on failure all ordinal values are set to 0
	for (iParam = 0; iParam < m_cParamNames; iParam++)
			TEST_COMPARE(pParamOrdinals[iParam], m_prgParamOrdinals[iParam]);

	// If we made it to here everything succeeded
	testresult=TEST_PASS;

CLEANUP:	

	PROVIDER_FREE(pParamOrdinals); 

	return testresult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc MapParameterNames with fewer names than set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_9()
{
	TESTRESULT testresult=TEST_FAIL;
	ULONG iOrdinal=0;
	DB_UPARAMS * pParamOrdinals = NULL;
	ULONG ulMaxNames=1;
	HRESULT hr;

	// Set all ordinal values to a known value
	TEST_ALLOC(DB_UPARAMS, pParamOrdinals, NOT_TOUCHED_ORD, (size_t)m_cParamNames*sizeof(DB_UPARAMS));

	if (!CHECK (m_pICommandPrepare->Prepare(1), S_OK))
		goto CLEANUP;

	// Ask for the last parameters to be mapped only, rest should be 0
	hr=m_pICmdWParams->MapParameterNames(ulMaxNames, (const WCHAR **)(&m_prgwszParameterNames[m_cParamNames-ulMaxNames]),
		(DB_LPARAMS *)pParamOrdinals);

	if (hr == E_NOTIMPL)
	{
		odtLog << L"MapParameterNames not implemented.\n";
		testresult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TEST_CHECK(hr, S_OK);

	// Now check that the ordinal values are correct
	for (iOrdinal=0; iOrdinal < m_cParamNames; iOrdinal++)
	{
		// For the requested parameters the correct ordinal should be returned
		if (iOrdinal < ulMaxNames)
		{
			TEST_COMPARE(pParamOrdinals[iOrdinal], m_prgParamOrdinals[iOrdinal+m_cParamNames-ulMaxNames]);
		}
		else
		{
			// Otherwise the "not touched" value
			TEST_COMPARE(pParamOrdinals[iOrdinal], m_iNotTouched);
		}

	}

	// If we made it to here everything succeeded
	testresult=TEST_PASS;

CLEANUP:
	
	PROVIDER_FREE(pParamOrdinals); 

	return testresult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Random name order: S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_10()
{
	TESTRESULT testresult=TEST_FAIL;
	LONG iOrdinal=0, iOrd2;
	HRESULT hr;
	DB_LPARAMS * pParamOrdinals = NULL;
	WCHAR ** prgwszParameterNames = NULL;

	TEST_ALLOC(DB_LPARAMS, pParamOrdinals, NOT_TOUCHED_ORD, (size_t)m_cParamNames*sizeof(DB_LPARAMS));

	// Allocate space for duplicate of param names array
	prgwszParameterNames = new WCHAR *[(ULONG)m_cParamNames];

	if (!prgwszParameterNames)
		return TEST_FAIL;

	// Copy the names from the original array
	memcpy(prgwszParameterNames, m_prgwszParameterNames, (size_t)m_cParamNames * sizeof(WCHAR *));

	ScrambleArray(prgwszParameterNames, m_cParamNames, sizeof(WCHAR *));

	if (!CHECK (m_pICommandPrepare->Prepare(1), S_OK))
		goto CLEANUP;

	hr=m_pICmdWParams->MapParameterNames(m_cParamNames, (const WCHAR **)prgwszParameterNames, pParamOrdinals);

	if (hr == E_NOTIMPL)
	{
		odtLog << L"MapParameterNames not implemented.\n";
		testresult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TEST_CHECK(hr, S_OK);

	// Now check that the ordinal values are correct
	for (iOrdinal=0; iOrdinal < (LONG)m_cParamNames; iOrdinal++)
	{
		// Locate the name in the properly ordered list
		for (iOrd2=0; iOrd2 < (LONG)m_cParamNames; iOrd2++)
		{
			if (!wcscmp(prgwszParameterNames[iOrdinal], m_prgwszParameterNames[iOrd2]))
				break;
		}

		if (!MYCOMPARE(pParamOrdinals[iOrdinal], iOrd2+1,L"Incorrect ordinal value returned."))
			goto CLEANUP;
	}

	// If we made it to here everything succeeded
	testresult=TEST_PASS;

CLEANUP:	

	delete[] prgwszParameterNames;
	PROVIDER_FREE(pParamOrdinals);
	return testresult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Overridden with SetParameterInfo: S_OK or DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_11()
{
	TESTRESULT testresult=TEST_FAIL;
	DB_UPARAMS iOrdinal=0;
	HRESULT hr;
	DB_LPARAMS * pParamOrdinals = NULL;
	WCHAR ** rgwszParamNames=NULL;
	DBPARAMBINDINFO * pParamBindInfo = NULL;
	WCHAR * pwszParamNameFmt = NULL;
	WCHAR * pwszName1 = NULL;
	WCHAR * pwszName2 = NULL;

	WCHAR NameOverRide[][SP_MAX_PARAMNAME_LENGTH] = {
		L"ABCD", 
		L"xyz"
	};

	// Providers have different syntax for parameter names.  We need to match with the 
	// overridden names.  Retrieve syntax.
	if (!(pwszParamNameFmt = m_Syntax.GetSyntax(T_PARM_NAME_FMT)))
		goto CLEANUP;

	if (!FormatString(&pwszName1, pwszParamNameFmt, 1, NameOverRide[0]))
		goto CLEANUP;

	if (!FormatString(&pwszName2, pwszParamNameFmt, 1, NameOverRide[1]))
		goto CLEANUP;

	TEST_ALLOC(DB_LPARAMS, pParamOrdinals, NOT_TOUCHED_ORD, (size_t)m_cParamNames*sizeof(DB_LPARAMS));
	TEST_ALLOC(DBPARAMBINDINFO, pParamBindInfo, NOT_TOUCHED_ORD, (size_t)m_cParamNames*sizeof(DBPARAMBINDINFO));
	memcpy(pParamBindInfo, m_pParamBindInfo, (size_t)m_cParamNames*sizeof(DBPARAMBINDINFO));

	// Initialize GetParameterInfo buffers to NULL
	m_pNamesBuffer=NULL;
	m_rgParamInfo=NULL;

	// Must be prepared
	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	// Make an array to store the parameter names
	TEST_ALLOC(WCHAR *, rgwszParamNames, 0, (size_t)m_cParamNames*sizeof(WCHAR *));
	memcpy(rgwszParamNames, m_prgwszParameterNames, (size_t)m_cParamNames * sizeof(WCHAR *));

	// Set the parameter names, over-riding first and last name
	pParamBindInfo[0].pwszName=pwszName1;
	rgwszParamNames[0]=pwszName1;
	pParamBindInfo[m_cParamNames-1].pwszName=pwszName2;
	rgwszParamNames[m_cParamNames-1]=pwszName2;

	// Expect DB_S_TYPEINFOOVERRIDDEN here because we either set it in the init or the provider
	// can derive.
	TEST_CHECK(hr = m_pICmdWParams->SetParameterInfo(m_cParamNames, m_prgParamOrdinals, pParamBindInfo),
		DB_S_TYPEINFOOVERRIDDEN);

	hr=m_pICmdWParams->MapParameterNames(m_cParamNames, (const OLECHAR **)rgwszParamNames, pParamOrdinals);

	if (hr == E_NOTIMPL)
	{
		odtLog << L"MapParameterNames not implemented.\n";
		testresult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TEST_CHECK(hr, S_OK);

	// Now check that the ordinal values are correct
	for (iOrdinal=0; iOrdinal < m_cParamNames; iOrdinal++)
	{
		if (!MYCOMPARE(m_prgParamOrdinals[iOrdinal], iOrdinal+1,L"Incorrect ordinal value returned."))
			goto CLEANUP;
	}

	// If we made it to here everything succeeded
	testresult=TEST_PASS;

CLEANUP:
	
	// Set the parameter information back for further variations
	if (!CHECK(hr = m_pICmdWParams->SetParameterInfo(m_cParamNames, m_prgParamOrdinals, m_pParamBindInfo),
		DB_S_TYPEINFOOVERRIDDEN))
		testresult = TEST_FAIL;

	PROVIDER_FREE(rgwszParamNames);
	PROVIDER_FREE(pParamOrdinals); 
	PROVIDER_FREE(pParamBindInfo); 
	PROVIDER_FREE(pwszName1); 
	PROVIDER_FREE(pwszName2); 
	PROVIDER_FREE(pwszParamNameFmt); 

	return testresult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc No command text set: DB_E_NOCOMMAND
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMapParameterNames::Variation_12()
{
	WCHAR * wszExecuteString;
	TESTRESULT testresult=TEST_FAIL;
	GUID guidSQL=DBGUID_DBSQL;
	DB_LPARAMS * pParamOrdinals = NULL;
	HRESULT hr = E_FAIL;

	// We need to remove any parameter info set previously, since the provider is allowed
	// to use any set info for MapParameterNames
	TEST_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	TEST_ALLOC(DB_LPARAMS, pParamOrdinals, NOT_TOUCHED_ORD, (size_t)m_cParamNames * sizeof(DB_LPARAMS));

	// Save the command text so we can put it back
	TEST_CHECK (m_pICommandText->GetCommandText(&guidSQL, &wszExecuteString), S_OK);

	// Make sure we have no command text set
	TEST_CHECK (m_pICommandText->SetCommandText(guidSQL, NULL), S_OK);

	hr = m_pICmdWParams->MapParameterNames(m_cParamNames, (const WCHAR **)m_prgwszParameterNames, pParamOrdinals); 

	if (hr == E_NOTIMPL)
	{
		odtLog << L"MapParameterNames not implemented.\n";
		testresult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TEST_CHECK(hr, DB_E_NOCOMMAND);

	testresult = TEST_PASS;

CLEANUP:

	// Reset param info
	if (!COMPARE(SUCCEEDED(m_pICmdWParams->SetParameterInfo(m_cParamNames, m_prgParamOrdinals, m_pParamBindInfo)),
		TRUE))
		testresult = TEST_FAIL;

	// Reset the command text for other variations
	if (!CHECK (m_pICommandText->SetCommandText(guidSQL, wszExecuteString), S_OK))
		testresult = TEST_FAIL;

	PROVIDER_FREE(pParamOrdinals);
	
	return testresult;
}

// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMapParameterNames::Terminate()
{
	PROVIDER_FREE(m_prgParamOrdinals);
	
	FreeParameterNames(m_prgwszParameterNames); 

	DropStoredProcedure(m_pICommandText);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CICmdWParams::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCCommandExecute_Rowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCCommandExecute_Rowset - Test case for ICommand::Execute (For Parameter related test cases
//|	Created:			04/10/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//--------------------------------------------------------------------
BOOL TCCommandExecute_Rowset::Init()
{
	WCHAR *pwszSelectAllFromTbl = NULL;

	// Initialize values.
	m_cSPParamColMap = NULL;
	m_rgSPParamColOrdinals = NULL;
	m_rgSPBindings = NULL;
	m_rgSPParamBind = NULL;
	m_rgSPParamColMap = NULL;
	m_cSPBindings = 0;
	m_pSPParamAll = NULL;

	// USe the Empty command it is already created anyway. 
	m_pSPICommandText = NULL;
	m_pSPICommandPrepare = NULL;
	m_pSPIAccessor = NULL;
	m_pSPRowset = NULL;
	m_hReadAccessor = DB_NULL_HACCESSOR;
	m_cReadBindings = 0;
	m_cbReadRowSize = 0;
	m_cbSPRowSize = 0;
	m_rgReadBindings = NULL;

	// If we're using an ini file then skip the test case.  Privlib crashes
	// due to the unavailability of the underlying provider type name information
	// needed for CreateColDef when using an ini file.
	// This restriction can be removed when TableDump can provide the provider type
	// name and create params.
	// TODO: Move the stored proc test variations to their own test case so all other
	// variations can still be run.
	if (GetModInfo()->GetFileName())
	{
		odtLog << L"Currently can't run this test case using an ini file.\n";
		return TEST_SKIPPED;
	}

	if (m_eTestCase == TC_Row && !g_fRowObj)
	{
		odtLog << L"Row objects not supported.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CICmdWParams::Init())
	// }}
	{
		// If we know the SQL syntax for the provider and output parameters are supported then 
		// create a stored procedure and accessor.
		if (g_ulOutParamsSupported != DBPROPVAL_OA_NOTSUPPORTED)
		{

			// Get Command text interface.
			if (!VerifyInterface(m_pIEmptyCommand,
					IID_ICommandText, COMMAND_INTERFACE, (IUnknown **)&m_pSPICommandText))
			{
				return FALSE;
			}

			// Get accessor interface.
			if (!VerifyInterface(m_pIEmptyCommand,
					IID_IAccessor, COMMAND_INTERFACE, (IUnknown **)&m_pSPIAccessor))
			{
				return FALSE;
			}
			
			if (!VerifyInterface(m_pIEmptyCommand,
					IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown **)&m_pSPICommandPrepare))
			{
				return FALSE;
			}

			if (!COMPARE (CreateSP(), TRUE))
				return FALSE;

		}
		return TRUE;
	}
	return TEST_SKIPPED;
}

//--------------------------------------------------------------------
// @mfunc Drop temporary stored procedure.
//
// @rdesc TRUE always.
//--------------------------------------------------------------------
BOOL 
TCCommandExecute_Rowset::DropSP()
{
	WCHAR * pwszDropString = NULL;
	WCHAR * pwszDropProc = NULL;
	BOOL fSuccess = FALSE;

	if (m_pwszCreateProcedureString && !(pwszDropProc = m_Syntax.GetSyntax(T_DROP)))
		goto CLEANUP;

	FormatString(&pwszDropString, pwszDropProc, 1, &g_pwszProcedureName[0]);

	if (m_pSPICommandText)
		m_pSPICommandText->SetCommandText(DBGUID_DBSQL , pwszDropString);

	// If it exists drop it.  No need to check return code.
	if (m_pIEmptyCommand)
		m_pIEmptyCommand->Execute(NULL, IID_NULL, NULL, NULL, NULL);

	// Free the strings pointed to be CreateProcedure and execute procedure.
	FREE_DATA (m_pwszCreateProcedureString);
	FREE_DATA (m_pwszExecuteProcedureString);

	fSuccess = TRUE;

CLEANUP:

	PROVIDER_FREE(pwszDropString);
	PROVIDER_FREE(pwszDropProc);

	return fSuccess;
}


//--------------------------------------------------------------------
// @mfunc Create text for temporary stored procedure.
//
// @rdesc TRUE or FALSE
//--------------------------------------------------------------------
BOOL TCCommandExecute_Rowset::CreateSPText()
{
	// Creates both the stored procedure text and the execute procedure text and bindings
	BOOL fSuccess = FALSE;

	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=wcsDuplicate((WCHAR *)g_pwszProcedureName);
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	ULONG iParam;

	// Initialize Local data.
	m_ulLocalSPRowNum = m_pTable->GetRowsOnCTable();
	m_rgSPParamColMap = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return FALSE;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return FALSE;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		NULL,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Allocate space for param to column map
	TEST_ALLOC(DB_LORDINAL, m_rgSPParamColMap, 0, cParams * sizeof(DB_LORDINAL));

	// Fill it
	for (iParam = 0; iParam < cParams; iParam++)
		m_rgSPParamColMap[iParam] = pParamAll[iParam].ulColIndex;

	fSuccess = TRUE;

CLEANUP:

	m_cSPBindings = cParams;
	m_cbSPRowSize = cbRowSize;
	m_cSPParamColMap = cParams;
	m_rgSPParamColOrdinals = pParamOrdinals;
	m_rgSPBindings = pBINDING;
	m_rgSPParamBind = pPARAMBIND;
	m_pwszCreateProcedureString = pwszCreateProcStmt;
	m_pwszExecuteProcedureString = pwszExecProcStmt;
	m_pSPParamAll = pParamAll;

	SAFE_FREE(pwszProcName);

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Create the stored procedure in the backend
//
// @rdesc TRUE or FALSE.
//--------------------------------------------------------------------
BOOL
TCCommandExecute_Rowset::CreateSP()
{

	// In case it exists we should try to drop it first
	DropSP();

	CreateSPText();

	TEST_CHECK(m_pSPICommandText->SetCommandText(DBGUID_DBSQL , m_pwszCreateProcedureString), S_OK);

	TEST_CHECK(m_pIEmptyCommand->Execute(NULL, IID_NULL, NULL, NULL, NULL), S_OK);

	return TRUE;
	
CLEANUP:

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Execute temporary stored procedure.
//
// @rdesc HRESULT
//--------------------------------------------------------------------
HRESULT 
TCCommandExecute_Rowset::ExecuteSP(DBPARAMS *pDbParams, DBCOUNTITEM ulRowNum)
{
	BYTE *pData = NULL;
	HRESULT hr = S_OK;
	
	hr = m_pSPICommandText->SetCommandText(DBGUID_DBSQL , m_pwszExecuteProcedureString);

	if (! CHECK (hr, S_OK))
		return hr;

	if (!CHECK ((hr = m_pSPICommandPrepare->Prepare(1)), S_OK))
	{
		return hr;
	}

	if (m_pSPRowset) ReleaseRowsetPtr(&m_pSPRowset);
	m_cSPRowsAffected = 0;
	// Allocate pData.
	pData = (BYTE *)m_pIMalloc->Alloc(m_cbSPRowSize);
	if (!pData)
	{
		return E_OUTOFMEMORY;
	}

	pDbParams->pData = pData;
	// Fill input bindings.
	if (!CHECK((hr = FillInputBindings(m_pTable,
				DBACCESSOR_PARAMETERDATA, m_cSPBindings, m_rgSPBindings,
				(BYTE **)&pData, (ulRowNum-1),  m_cSPParamColMap, m_rgSPParamColMap)), S_OK))
	{
		return hr;
	}
	return m_pIEmptyCommand->Execute(NULL, IID_IRowset,
			pDbParams, &m_cSPRowsAffected, (IUnknown **)&m_pSPRowset);
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc An invalid parameter in pParams, returns DB_E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_1()
{
	DBPARAMS sInvalidDbParams;
	DBROWCOUNT cRowsAffected;
	IRowset * pRowset = NULL;
	
	sInvalidDbParams.cParamSets = 1;
	sInvalidDbParams.hAccessor = m_hAccessor;
	sInvalidDbParams.pData  = NULL;


	if (!CHECK(m_pICommand->Execute(NULL, IID_NULL, &sInvalidDbParams, &cRowsAffected,
		 (IUnknown **)&pRowset),  E_INVALIDARG))
	{
		return TEST_FAIL;
	}

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Supply too many parameter info than required.  Should return S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_2()
{
	DBCOUNTITEM cBindings;
	DBBINDING *rgBindings = NULL;
	DB_LORDINAL *rgParamColMap = NULL;
	BOOL fResult = FALSE;
	void *pData = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBPARAMS ExecDbParams;
	DBBYTEOFFSET ulOffset;
	DBROWCOUNT cRowsAffected = 0;
	IRowset * pRowset = NULL;
	HRESULT hr = E_FAIL;
	UINT i = 0;
	DBCOUNTITEM ulRowNum;


	// One more than number of bindings we need.
	cBindings = m_cBindings + 1;

	// Allocate memory for the array.
	rgBindings = (DBBINDING *)m_pIMalloc->Alloc(cBindings * sizeof(DBBINDING));
	
	if (rgBindings == NULL)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}

	rgParamColMap = (DB_LORDINAL *)m_pIMalloc->Alloc(cBindings * sizeof(DB_LORDINAL));
	if (rgParamColMap == NULL)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}
		
	
	// Copy all the bindings.
	memcpy(rgBindings, m_rgBindings, (size_t)m_cBindings*sizeof(DBBINDING));
	memcpy(rgParamColMap, m_rgParamColMap, (size_t)m_cBindings*sizeof (DB_LORDINAL));

	// Now initialize the last binding.
	rgBindings[cBindings -1] = rgBindings[cBindings -2];
	
	// Also initialize the column array. (We have sufficient space in the column array)
	rgParamColMap[cBindings - 1] = rgParamColMap[cBindings - 2];

	
	// Get to the end.
	ulOffset = rgBindings[cBindings -1].obValue + sizeof(DBLENGTH);

	//If we bound the value, we need to compensate for any extra room 
	//the value may have taken over the bValue size allocated in the struct
	
	if (rgBindings[cBindings -1].dwPart & DBPART_VALUE)
		if (rgBindings[cBindings - 1].cbMaxLen > sizeof(DBLENGTH))
			ulOffset += rgBindings[cBindings - 1].cbMaxLen - sizeof(DBLENGTH);

	//Make sure our structure begins on a correct byte alignment
	ulOffset = ROUND_UP(ulOffset,ROUND_UP_AMOUNT);
	
	if (rgBindings[cBindings - 1].dwPart & DBPART_VALUE)
	{
		rgBindings[cBindings - 1].obValue = 	ulOffset + offsetof(DATA,bValue);
	}

	rgBindings[cBindings - 1].pObject = NULL;				

	//Only set length offset if we're binding length
	if (rgBindings[cBindings - 1].dwPart & DBPART_LENGTH)
	{
		rgBindings[cBindings - 1].obLength = ulOffset + offsetof(DATA,ulLength);

	}

	//Only set status offset if we're binding status
	if (rgBindings[cBindings - 1].dwPart & DBPART_STATUS)
	{
		rgBindings[cBindings - 1].obStatus = ulOffset + offsetof(DATA,sStatus);
	
	}

	// Now create data for these accessors.
	ulRowNum = NextInsertRowNum();
	if (!CHECK(FillInputBindings((CTable *)m_pThisTestModule->m_pVoid, DBACCESSOR_PARAMETERDATA, 
		cBindings, rgBindings, (BYTE **)&pData, ulRowNum,  cBindings, rgParamColMap), S_OK))
	{
		goto CLEANUP;
	}
	
	// change parameter ordinal.
	rgBindings[cBindings -1].iOrdinal++;

	for (i = 0; i < cBindings; i++)
	{
		rgBindings[i].dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
		rgBindings[i].eParamIO = DBPARAMIO_INPUT;
		rgBindings[i].dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
		rgBindings[i].dwFlags = 0;
	}

	// Now create the parameter accessor with the extra ordinal.
	if (!CHECK (m_pCmdIAccessor->CreateAccessor( DBACCESSOR_PARAMETERDATA,
		cBindings, rgBindings, m_cbRowSize, &hAccessor, NULL), S_OK))
	{
		goto CLEANUP;
	}

	
	ExecDbParams.cParamSets = 1;
	ExecDbParams.hAccessor = hAccessor;
	ExecDbParams.pData = pData;

	// Too many parameters supplied.
	// Should return DB_E_ERRORSOCCURRED or DB_E_BADORDINAL
	hr = m_pICommand->Execute(NULL, IID_IUnknown, &ExecDbParams, &cRowsAffected, (IUnknown **)&pRowset);

	if (FAILED(hr))
		// Since the row wasn't inserted, decrement next insert row number incremented above.
		m_cInsertRowNum--;

	if (hr == DB_E_ERRORSOCCURRED)
	{
		// Check to see if status is set for the Last arguement.  if not test fail.
		TEST_COMPARE ((*(DBSTATUS *)((BYTE *)pData + rgBindings[cBindings - 1].obStatus)), DBSTATUS_E_BADACCESSOR);
	}
	else if (hr != DB_E_BADORDINAL) 
	{
		CHECK(hr, DB_E_ERRORSOCCURRED);  // To force an error in the log
		goto CLEANUP;
	}
	
	fResult = TRUE;

CLEANUP:

	ReleaseDataForCommand();

	
	if (pData)
	{
		ReleaseInputBindingsMemory(cBindings, rgBindings, (BYTE *)pData, TRUE);
		pData = NULL;
	}

	FREE_DATA (rgBindings);
	FREE_DATA (rgParamColMap);

	if (hAccessor)
		CHECK(m_pCmdIAccessor->ReleaseAccessor (hAccessor, NULL), S_OK);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Supply too few parameters than required, returns DB_E_PARAMNOTOPTIONAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_3()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBCOUNTITEM cBindings = m_cBindings - m_cBindings/2;
	DBPARAMS ExecDbParams;
	void *pData = NULL;
	DBROWCOUNT cRowsAffected;
	IRowset * pRowset = NULL;
	BOOL fResult = FALSE;
	DBCOUNTITEM ulRowNum = 0;
	HRESULT hr = E_FAIL;
	
	
	// Now create the parameter accessor with less bindings.
	if (!CHECK (m_pCmdIAccessor->CreateAccessor( 
		DBACCESSOR_PARAMETERDATA,
		cBindings, m_rgBindings, m_cbRowSize,
		&hAccessor, NULL), S_OK))
	{
		goto CLEANUP;
	}
	ulRowNum = NextInsertRowNum();
	if (!CHECK(FillInputBindings((CTable *)m_pThisTestModule->m_pVoid,
				DBACCESSOR_PARAMETERDATA,
				cBindings, m_rgBindings,
				(BYTE **)&pData, ulRowNum,  m_cParamColMap,(DB_LORDINAL *)m_rgParamColMap), S_OK))
	{
		goto CLEANUP;
	}

	ExecDbParams.cParamSets = 1;
	ExecDbParams.hAccessor = hAccessor;
	ExecDbParams.pData = pData;

	// Now call Execute.  It should fail with DB_E_PARAMNOTOPTIONAL;
	// 
	hr = m_pICommand->Execute(NULL, IID_IUnknown, &ExecDbParams, &cRowsAffected, 
		(IUnknown **)&pRowset);

	if (FAILED(hr))
		// Since the row wasn't inserted, decrement next insert row number incremented above.
		m_cInsertRowNum--;
	
	
	if (!CHECK(hr, DB_E_PARAMNOTOPTIONAL))	
	{
		goto CLEANUP;
	}

	fResult = TRUE;

CLEANUP:

	if (pData)
	{
		ReleaseInputBindingsMemory(cBindings, m_rgBindings, (BYTE *)pData, TRUE);
		pData = NULL;
	}

	if (hAccessor)
		CHECK(m_pCmdIAccessor->ReleaseAccessor (hAccessor, NULL), S_OK);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Supply more than one value for the same parameter.  Returns DB_E_DUPLICATEPARAM
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_4()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBCOUNTITEM cBindings = m_cBindings - m_cBindings/2;
	IRowset * pRowset = NULL;
	BOOL fResult = FALSE;
	DBBINDSTATUS *rgStatus = NULL;
	
	DBORDINAL cSaveCol;
	
	if (m_cBindings < 3)
	{
		odtLog << L"Too few columns in this table for this variation\n";
		return TEST_SKIPPED;
	}
	// Reassigning the column number to create a duplicate parameter.
	cSaveCol = m_rgBindings[1].iOrdinal;
	m_rgBindings[1].iOrdinal = m_rgBindings[2].iOrdinal;

	rgStatus = (DBBINDSTATUS *)m_pIMalloc->Alloc(m_cBindings*sizeof(DBBINDSTATUS));
	if (!rgStatus)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}

	// Now create the parameter accessor with the extra ordinal.
	// Create accessor will fail with the duplicate ordinal.
	// This test gets completed here.  (Changes in M08-M09)
	if (!CHECK (m_pCmdIAccessor->CreateAccessor(  DBACCESSOR_PARAMETERDATA, m_cBindings, 
		m_rgBindings, m_cbRowSize, &hAccessor, rgStatus), DB_E_ERRORSOCCURRED))
	{
		goto CLEANUP;
	}

	if (!COMPARE (rgStatus[2], DBBINDSTATUS_BADBINDINFO))
		goto CLEANUP;
	
	fResult = TRUE;

CLEANUP:
	m_rgBindings[1].iOrdinal = cSaveCol;
	
	FREE_DATA (rgStatus);

	if (hAccessor)
		CHECK(m_pCmdIAccessor->ReleaseAccessor (hAccessor, NULL), S_OK);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Supply a parameter value outside the domain for that parameter, returns DB_E_OVERFLOW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_5()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommandPrepare Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	ICommandWithParameters* pICmdWPar	= NULL;		// ICommandWithParameters.
	ICommandText *		pICommandText	= NULL;
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				count			= 0;		// Loop counter
	ULONG				ColPrec			= 0;		// Numerical column precision.
	CList				<DBTYPE, DBTYPE> DBTypeList;
	CCol				NewCol(m_pIMalloc);			// Class CCol
	IAccessor *			pLocalAccessor = NULL;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBPARAMS			Params;
	void *				pData = NULL;				// pdata for parameters.
	LONGLONG			cllMax = _I64_MAX;			// Max longlong value.
	ULONG				cParams = 1;
	DB_UPARAMS			rgParamOrdinals[1] = { 1 };
	WCHAR				wszDataSourceType[15] = L"DBTYPE_I4";
	DBPARAMBINDINFO		rgParamBindInfo[1] = { &wszDataSourceType[0], NULL, 2, DBPARAMFLAGS_ISINPUT|DBPARAMFLAGS_ISSIGNED, (BYTE)~255, (BYTE)~255};

	// Creates a column list from the Ctable
	pcColumns = m_pTable->CountColumnsOnTable();

	// Loop thru column types
	for( count=1; count <= pcColumns; count++)
	{
		m_pTable->GetColInfo(count, NewCol);
		
		// When we find an updatable I4, I2 or I1 column we're done
		if ((NewCol.GetProviderType() == DBTYPE_I4 ||
			NewCol.GetProviderType() == DBTYPE_I2 ||
			NewCol.GetProviderType() == DBTYPE_I1)
			&& NewCol.GetUpdateable())
				break;
	}

	// Make sure we found the type we need
	if (count >= pcColumns)
	{
		odtLog << L"Couldn't find a DBTYPE_I1, DBTYPE_I2, or I4 column needed for this test variation.\n";
		return TEST_SKIPPED;
	}

	// If we found an I1 column we have to fix up rgparamBindInfo
	if (NewCol.GetProviderType() == DBTYPE_I2)
		wcscpy(rgParamBindInfo[0].pwszDataSourceType, L"DBTYPE_I2");
	if (NewCol.GetProviderType() == DBTYPE_I1)
		wcscpy(rgParamBindInfo[0].pwszDataSourceType, L"DBTYPE_I1");

	DBTypeList.AddHead(NewCol.GetProviderType());

	// Create a table
	if(!CHECK(m_pExtraTable->CreateTable(DBTypeList,
									1,			// Number of rows to insert
									0,			// Column to put index on
									NULL,		// Table name
									PRIMARY),	// Primary or secondary values
									S_OK))
	{
		// Free memory in the list
		DBTypeList.RemoveAll();
		return TEST_FAIL;
	}

	// Get the name of the table just created
	pTableName = m_pExtraTable->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( SP_TEXT_BLOCK_SIZE);

	// Format SQL Statement
	swprintf(pwszSQLStmt, g_wszInsertInvalidValue, pTableName);


	//  Command to return a ICommand with Text Set

	if( !CHECK(m_pExtraTable->BuildCommand(pwszSQLStmt,		// SQL STMT
			IID_IRowset,		// IID
			EXECUTE_NEVER,		// EXECUTE
			NULL,				// # Prop's
			NULL,				// Prop's
			NULL,				// Params
			NULL,				// # Rowset
			NULL,				// Rowsets
			&pICommand),		// ICommand
			S_OK) )
			goto CLEANUP;


	// QI for ICommandPrepare
	if( !CHECK(pICommand->QueryInterface(IID_ICommandPrepare, (void **)&pICommandPrep), S_OK) )
			goto CLEANUP;


	// Setup the parameter sturcture.

	// 1. Create the accessor.

	// Verify The Interface for a Command Accessor Object.
	if (!VerifyInterface(pICommand, IID_IAccessor, COMMAND_INTERFACE, (IUnknown **)&pLocalAccessor))
	{
		//ICommandAccessor is not supported
		goto CLEANUP;
	}

	// Set the text first.
	if (!VerifyInterface(pICommand, IID_ICommandText, COMMAND_INTERFACE,(IUnknown **)&pICommandText))
	{
		//ICommandText is not supported.
		goto CLEANUP;
	}
		
	// set text.
	TEST_CHECK (pICommandText->SetCommandText (DBGUID_DBSQL, pwszSQLStmt), S_OK );


	// Create the bindings and the create handle to accessor.
	DBBINDING DbBinding;

	// Set entire dbbinding structure to 0 to prevent interface remoting problems.
	memset(&DbBinding, 0, sizeof(DBBINDING));

	DbBinding.dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
	DbBinding.eParamIO = DBPARAMIO_INPUT;
	DbBinding.iOrdinal = 1;
	DbBinding.pTypeInfo = NULL;
	DbBinding.obValue = offsetof(DATA, bValue);
	DbBinding.cbMaxLen = sizeof (LONGLONG);
	DbBinding.obLength = offsetof(DATA, ulLength);
	DbBinding.obStatus = offsetof (DATA, sStatus);
	DbBinding.wType = DBTYPE_I8; // Column is I4, I2, or I1.  I8 will overflow.
	DbBinding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	DbBinding.pBindExt = NULL;
	DbBinding.bPrecision = 0;
	DbBinding.bScale = 0;


	// Call create accessor.
	TEST_CHECK (pLocalAccessor->CreateAccessor( 
			DBACCESSOR_PARAMETERDATA, 1, &DbBinding, m_cbRowSize,
			&hAccessor, NULL), S_OK);

	// Allocate memory
	pData = (void *)m_pIMalloc->Alloc( ( 2 * sizeof (LONGLONG) + DbBinding.cbMaxLen ) );
	if (pData == NULL)
	{
		goto CLEANUP;
	}

	// 2. Create the data.
	memcpy((void *)((BYTE *)pData + DbBinding.obValue), &cllMax, sizeof(LONGLONG));
	*((DBLENGTH *)((BYTE *)pData + DbBinding.obLength)) = sizeof (LONGLONG);
	*((DBSTATUS *)((BYTE *)pData + DbBinding.obStatus)) = DBSTATUS_S_OK;

	// 3. Set the ParamData.
	Params.cParamSets = 1;
	Params.hAccessor = hAccessor;
	Params.pData = pData;

	if (!VerifyInterface(pICommand, IID_ICommandWithParameters,
			COMMAND_INTERFACE,(IUnknown **)&pICmdWPar))
	{
		//ICommandWithParameters is not supported
		goto CLEANUP;
	}

	// Lets set the parameter info.
	hr = pICmdWPar->SetParameterInfo (cParams, rgParamOrdinals, rgParamBindInfo);

	if (!(hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN ))
		goto CLEANUP;

	// Execute here, don't Prepare since Prepare doesn't set status if it fails.
	TEST_CHECK(pICommand->Execute(NULL, IID_NULL,
		&Params, NULL, NULL), DB_E_ERRORSOCCURRED);

	// Check for Overflow status
	TEST_COMPARE (*((DBSTATUS *)((BYTE *)pData + DbBinding.obStatus)), DBSTATUS_E_DATAOVERFLOW);

	fSuccess = TRUE;


CLEANUP:
	// Drop the table
	m_pExtraTable->DropTable();

	// Free memory in the list
	DBTypeList.RemoveAll();


	// Free Memory
	FREE_DATA ( pwszSQLStmt );
	FREE_DATA ( pData );

	// Release Objects
	if (pLocalAccessor) pLocalAccessor->ReleaseAccessor (hAccessor, NULL);

	RELEASE (pLocalAccessor);
	RELEASE (pICommandText);
	RELEASE (pICommandPrep);
	RELEASE (pICmdWPar);
	RELEASE (pICommand);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Accessor with PARAMETER_DATA | ROWDATA | READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL
//
// Verify the same accessor can be used as both a parameter and rowdata
// accessor.
int TCCommandExecute_Rowset::Variation_6()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBPARAMS ExecDbParams;
	DBROWCOUNT cRowsAffected;
	DBCOUNTITEM cRowsObtained = 0;
	IRowset *pRowset = NULL;
	IRowset *pIRowset = NULL;
	BOOL fResult = FALSE;
	DBCOUNTITEM ulRowNum = 0;
	HRESULT hr= S_OK;
	HROW * prghRows = NULL;
	HRESULT hrSetProp = E_FAIL;
	DBCOUNTITEM cRowsExpected = 1;
	
	if (m_eTestCase == TC_Row)
	{
		odtLog << L"Row objects can't share an accessor with a command object.\n";
		return TEST_SKIPPED;
	}

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on
	if (SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	// Now create the parameter accessor
	// Change the bindings. to input/output bindings.
	ExecDbParams.pData = NULL;

	// Create a shared parameter and row accessor.  Note eParamIO ignored for row accessors
	// so we can leave it as DBPARAMIO_INPUT.
	TEST_CHECK (m_pCmdIAccessor->CreateAccessor( DBACCESSOR_ROWDATA | DBACCESSOR_PARAMETERDATA, 
		m_cBindings, m_rgBindings, m_cbRowSize, &hAccessor, NULL), S_OK);
	
	// Create data.
	TEST_CHECK(FillInputBindings((CTable *)m_pThisTestModule->m_pVoid,	DBACCESSOR_PARAMETERDATA,
		m_cBindings, m_rgBindings, (BYTE **)&ExecDbParams.pData, (ulRowNum = NextInsertRowNum()),  
		m_cParamColMap,(DB_LORDINAL *)m_rgParamColMap), S_OK);

	ExecDbParams.cParamSets = 1;
	ExecDbParams.hAccessor = hAccessor;
	
	// Now call Execute and expect success as this is an insert statement with params
	// This is a fatal error if it doesn't succeed since we can't verify shared accessors work
	TEST_CHECK(m_pICommand->Execute(NULL, IID_IUnknown, &ExecDbParams, &cRowsAffected, (IUnknown **)&pRowset), S_OK);

	// Make sure the row really was inserted	
	TEST_COMPARE(cRow.FindRow(ulRowNum, m_pTable, NULL, &pIRowset, NULL,
		0, NULL, FALSE), TRUE);

	// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
	TEST_COMPARE(VerifyObj(m_iidExec, pIRowset, ulRowNum, 0, NULL,
		FALSE, FALSE, m_pTable, &cRowsExpected), S_OK);

	// Successful insert or failure shouldn't create a rowset
	if (!COMPARE(pRowset, NULL))
		SAFE_RELEASE(pRowset);
	
	// Now try to use the same accessor to retrieve data

	//Create a select for only the updatable columns
	TEST_CHECK (m_pTable->ExecuteCommand(SELECT_UPDATEABLE, IID_IUnknown, NULL, NULL, 
		NULL, NULL,  EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);

	// Since we've already got the accessor and data buffers created just try to execute with the new command text
	TEST_CHECK(m_pICommand->Execute(NULL, IID_IRowset, &ExecDbParams, &cRowsAffected, (IUnknown **)&pRowset), S_OK);

	// Make sure a rowset was opened this time
	TEST_COMPARE(pRowset != NULL, TRUE);

	// Rowsets don't return cRowsAffected, that is it's undefined, so we can't verify it

	// Note we don't use VerifyRowset because we want to make sure we use the same accessor
	// and the same bindings, whereas VerifyRowset creates the accessor and bindings for you

	// Now fetch a row and compare data using the same accessor
	TEST_CHECK(pRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &prghRows), S_OK);

	// We asked for only one row, we'd better get it
	TEST_COMPARE(cRowsObtained, 1);

	// Now actually retrieve the data using the same accessor
	CHECK(hr = pRowset->GetData(*prghRows, hAccessor, ExecDbParams.pData), S_OK);
	
	if (SUCCEEDED(hr))
	{
		// Make sure the data was retrieved properly
		TEST_COMPARE(CompareData(
						m_cBindings,
						(DB_LORDINAL *)m_rgParamColMap,
						1,
						ExecDbParams.pData,
						m_cBindings,
						m_rgBindings,
						m_pTable,
						m_pIMalloc,
						PRIMARY,
						COMPARE_ONLY,
						COMPARE_ALL,
						TRUE
					), TRUE);
	}
	

	fResult = TRUE;

	
CLEANUP:
	
	if (prghRows)
	{
		CHECK(pRowset->ReleaseRows(1, prghRows, NULL, NULL, NULL), S_OK);
		PROVIDER_FREE(prghRows);
	}

	SAFE_RELEASE(pRowset);
	SAFE_RELEASE(pIRowset);

	if (ExecDbParams.pData)
	{
		ReleaseInputBindingsMemory(m_cBindings, m_rgBindings, (BYTE *) ExecDbParams.pData, TRUE);
		ExecDbParams.pData = NULL;
	}

	if (hAccessor != DB_NULL_HACCESSOR)
		CHECK(m_pCmdIAccessor->ReleaseAccessor (hAccessor, NULL), S_OK);

	// If we set RANDOM REQUIRED above we need to set back OPTIONAL
	if (hrSetProp == S_OK)
		CHECK(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM, DBPROPOPTIONS_OPTIONAL), S_OK);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Accessor with ROWDATA with [in] and [in/out] parameters
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_7()
{

	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HACCESSOR hSPAccessor = DB_NULL_HACCESSOR;
	DBPARAMS ExecDbParams;
	DBPARAMS ExecSPParams;
	void *pData = NULL;
	DBROWCOUNT cRowsAffected;
	IRowset *pRowset = NULL;
	BOOL fResult = FALSE;
	DBCOUNTITEM ulRowNum = 0;
	HRESULT hr = E_FAIL;

	ExecDbParams.pData = NULL;
	ExecSPParams.pData = NULL;
	
	// Reset command text since another variation may have altered it.
	TEST_CHECK(m_pTable->ExecuteCommand(INSERT_ALLWITHPARAMS, IID_IUnknown, NULL, 
		NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);

	// Now create a row accessor
	TEST_CHECK (m_pCmdIAccessor->CreateAccessor( DBACCESSOR_ROWDATA,
		m_cBindings, m_rgBindings, m_cbRowSize, &hAccessor, NULL), S_OK);
	
	// Create data for the bindings
	TEST_CHECK(FillInputBindings((CTable *)m_pThisTestModule->m_pVoid,
				DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings,
				(BYTE **)&pData,(ulRowNum = NextInsertRowNum()),  m_cParamColMap,(DB_LORDINAL *)m_rgParamColMap), S_OK);

	ExecDbParams.cParamSets = 1;
	ExecDbParams.hAccessor = hAccessor;
	ExecDbParams.pData = pData;

	// Now call Execute and expect failure since we used a rowdata accessor for parameters.
	hr = m_pICommand->Execute(NULL, IID_IUnknown, &ExecDbParams, 
		&cRowsAffected, (IUnknown **)&pRowset);

	if (FAILED(hr))
		// Since the row wasn't inserted, decrement next insert row number incremented above.
		m_cInsertRowNum--;

	TEST_CHECK(hr, DB_E_BADACCESSORTYPE);

	// Initialize it to Null first.
	if (g_ulOutParamsSupported != DBPROPVAL_OA_NOTSUPPORTED)
	{
		// Now create the accessor  for in out parameters.
		TEST_CHECK (m_pSPIAccessor->CreateAccessor( DBACCESSOR_ROWDATA ,
			m_cSPBindings, m_rgSPBindings, m_cbSPRowSize, &hSPAccessor, NULL), S_OK);
				
		ExecSPParams.cParamSets = 1;
		ExecSPParams.hAccessor = hSPAccessor;
		ExecSPParams.pData = NULL;

		// Now call Execute. It should Fail.  Since there are in/out and out parameters..
		if (!CHECK (ExecuteSP(&ExecSPParams, m_ulLocalSPRowNum), DB_E_BADACCESSORTYPE))
		{
			odtLog << L"Should have failed with rowdata accessor.(Stored procedure with input and out parameters)\n";
			goto CLEANUP;
		}
		
	}
	fResult = TRUE;

CLEANUP:

	
	if (ExecDbParams.pData)
		ReleaseInputBindingsMemory(m_cBindings, m_rgBindings, (BYTE *) ExecDbParams.pData, TRUE);

	if (ExecSPParams.pData)
		ReleaseInputBindingsMemory(m_cSPBindings, m_rgSPBindings, (BYTE *) ExecSPParams.pData, TRUE);

	ReleaseRowsetPtr(&pRowset);
	
	if (hAccessor)
		CHECK(m_pCmdIAccessor->ReleaseAccessor (hAccessor, NULL), S_OK);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Accessor with only status binding.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_8()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBPARAMS ExecDbParams;
	void *pData = NULL;
	DBROWCOUNT cRowsAffected;
	IRowset *pRowset=NULL;
	BOOL fResult = FALSE;
	UINT iIndex = 0, i = 0;
	CCol TempCol;
	DBCOUNTITEM ulRowNum = 0;
			
	if( (m_pExtraTable->CreateTable(TOTAL_NUMBER_OF_ROWS)) != S_OK)
		return FALSE;
	
	// Lets restore the text object to insert command.
	TEST_CHECK(m_pExtraTable->ExecuteCommand(INSERT_ALLWITHPARAMS, IID_NULL, NULL, 
			NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);

	// Fill in regular values.
	TEST_CHECK(FillInputBindings(m_pExtraTable, DBACCESSOR_PARAMETERDATA,  m_cBindings,
		 m_rgBindings, (BYTE **)&pData, ulRowNum,  m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);
	
	// Now set the bindings of nullable columns to DBPART_STATUS and set the status bit appropriately for them.
	iIndex = 0;
	for (i = 1; i <= m_pExtraTable->CountColumnsOnTable(); i++)
	{
		CHECK(m_pExtraTable->GetColInfo(i, TempCol), S_OK);

		//Record the column number in the array
		//if it is 
		
		if (TempCol.GetUpdateable() )
		{
			ASSERT(iIndex < m_cBindings); 

			if (TempCol.GetNullable() == TRUE )
			{
				m_rgBindings[iIndex].dwPart = 0| DBPART_STATUS;	
				// Set pData to DBSTATUS_ISNULL;
				*((DBSTATUS *)((BYTE *)pData + m_rgBindings[iIndex].obStatus)) = DBSTATUS_S_ISNULL;

			}
			iIndex++; // For updateable Column.
		}
		
	}


	// Now create the parameter accessor 
	if (!CHECK (m_pCmdIAccessor->CreateAccessor(  DBACCESSOR_ROWDATA |  DBACCESSOR_PARAMETERDATA,
		m_cBindings, m_rgBindings, m_cbRowSize, &hAccessor, NULL), S_OK))
	{
		goto CLEANUP;
	}
	

	ExecDbParams.cParamSets = 1;
	ExecDbParams.hAccessor = hAccessor;
	ExecDbParams.pData = pData;

	// Now call Execute.  
	TEST_CHECK (m_pICommand->Execute(NULL, IID_NULL, &ExecDbParams, &cRowsAffected, 
		 (IUnknown **)&pRowset), S_OK);
	
	fResult = TRUE;

CLEANUP:

	// Drop the table
	m_pExtraTable->DropTable();

	if (ExecDbParams.pData)
		ReleaseInputBindingsMemory (m_cBindings, m_rgBindings, (BYTE *)pData, TRUE);

	ReleaseRowsetPtr(&pRowset);
	
	// Restoring the columnpart bindings.
	for (iIndex = 0; iIndex < m_cBindings; iIndex++)
	{
		m_rgBindings[iIndex].dwPart = DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS;
	}
	if (hAccessor)
		CHECK(m_pCmdIAccessor->ReleaseAccessor (hAccessor, NULL), S_OK);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Accessor with only (value and length
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_9()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBPARAMS ExecDbParams;
	void *pData = NULL;
	DBROWCOUNT cRowsAffected = 0;
	IRowset *pRowset = NULL;
	BOOL fResult = FALSE;
	UINT iIndex;
	DBCOUNTITEM ulRowNum = 0;

	// Reset command text since another variation may have altered it.
	TEST_CHECK(m_pTable->ExecuteCommand(INSERT_ALLWITHPARAMS, IID_IUnknown, NULL, 
		NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);

	// changing all bindings to VALUE and length only bindings.
	for (iIndex = 0; iIndex < m_cBindings; iIndex++)
	{
		m_rgBindings[iIndex].dwPart = 0| DBPART_VALUE | DBPART_LENGTH;
	}
	
	// Now create the parameter accessor with Value only bindings.
	TEST_CHECK (m_pCmdIAccessor->CreateAccessor( DBACCESSOR_PARAMETERDATA,
		m_cBindings, m_rgBindings, m_cbRowSize, &hAccessor, NULL), S_OK);
	

	TEST_CHECK(FillInputBindings((CTable *)m_pThisTestModule->m_pVoid, DBACCESSOR_PARAMETERDATA,
		m_cBindings, m_rgBindings, (BYTE **)&pData, (ulRowNum = NextInsertRowNum()),  m_cParamColMap, 
		(DB_LORDINAL *)m_rgParamColMap), S_OK);
	
	
	ExecDbParams.cParamSets = 1;
	ExecDbParams.hAccessor = hAccessor;
	ExecDbParams.pData = pData;

	// Now call Execute. 
	TEST_CHECK (m_pICommand->Execute(NULL, IID_NULL, &ExecDbParams, &cRowsAffected, (IUnknown **)&pRowset), S_OK);
	
	fResult = TRUE;

CLEANUP:

	ReleaseInputBindingsMemory (m_cBindings, m_rgBindings, (BYTE *)pData, TRUE);

	ReleaseRowsetPtr(&pRowset);
	
	// Restoring the columnpart bindings.
	for (iIndex = 0; iIndex < m_cBindings; iIndex++)
	{
		m_rgBindings[iIndex].dwPart = DBPART_LENGTH | DBPART_VALUE | DBPART_STATUS;
	}
	if (hAccessor)
		CHECK(m_pCmdIAccessor->ReleaseAccessor (hAccessor, NULL), S_OK);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Verify Stored procedure (in/out
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_10()
{
	BOOL fResult = FALSE;
	BYTE * pData = NULL;

	// Note, the row number here MUST match the row number used when creating the 
	// stored proc due to different SQL created for different rows if any of the
	// columns are NULL.
	DBCOUNTITEM ulRowNum = 1;

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Stored procedure syntax unknown or output Parameters are  not supported \n";
		return TEST_SKIPPED;
	}

	// The proc was created in the init, prepare to execute it
	TEST_CHECK(PrepareForExecute(m_pwszExecuteProcedureString, m_cSPBindings, m_rgSPParamColOrdinals,
		m_rgSPParamBind, NULL, (WCHAR *)g_pwszProcedureName, NULL), S_OK);

	// Fill input bindings.
	TEST_CHECK(FillInputBindings(m_pTable,
				DBACCESSOR_PARAMETERDATA, m_cSPBindings, m_rgSPBindings,
				(BYTE **)&pData, ulRowNum,  m_cSPParamColMap,m_rgSPParamColMap), S_OK);

	TEST_CHECK(ExecuteAndVerify(m_cSPBindings, 1, m_pSPParamAll, ulRowNum, m_rgSPBindings, m_cbSPRowSize,
		pData, ROWSET_NONE, 0, NULL, VERIFY_USE_TABLE, TRUE), S_OK);

	fResult = TRUE;

CLEANUP:

	if (pData)
	{
		ReleaseInputBindingsMemory(m_cSPBindings, m_rgSPBindings, pData, TRUE);
		pData = NULL;
	}

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DBPARAM invalid conditions.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_11()
{

	HACCESSOR hAccessor = DB_NULL_HACCESSOR;

	DBPARAMS ExecDbParams;
	DBROWCOUNT cRowsAffected;
	IRowset * pRowset = NULL;
	BOOL fResult = FALSE;
	WCHAR *pwszSqlSelectAllFromTbl = NULL;
	DBCOUNTITEM ulRowNum=0;

	TEST_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	//Replace text in CommandObject With select statment with no parameters.
	TEST_CHECK(m_hr = m_pTable->CreateSQLStmt(SELECT_COLLISTFROMTBL, NULL, &pwszSqlSelectAllFromTbl, NULL, NULL ), S_OK);

	// Now create the parameter accessor with less bindings.
	if (!CHECK (m_pCmdIAccessor->CreateAccessor(  DBACCESSOR_PARAMETERDATA, m_cBindings,
		m_rgBindings, m_cbRowSize, &hAccessor, NULL), S_OK))
	{
		goto CLEANUP;
	}

	ExecDbParams.cParamSets = 1;
	ExecDbParams.hAccessor = hAccessor;
	ExecDbParams.pData = NULL;

	// Should return E_INVALIDARG
	if (!CHECK(m_pICommand->Execute(NULL, IID_IUnknown, &ExecDbParams,
		 &cRowsAffected, (IUnknown **)&pRowset),  E_INVALIDARG))
	{
		goto CLEANUP;
	}
	

	MakeDataForCommand(1);


	// For a command executing with parameters it should return either E_INVALIDARG
	ExecDbParams.cParamSets = 0;

	// Should return E_INVALIDARG
	if (!CHECK(m_pICommand->Execute(NULL, IID_IUnknown, &ExecDbParams, &cRowsAffected,
		 (IUnknown **)&pRowset),  E_INVALIDARG))
	{
		goto CLEANUP;
	}

	if (!CHECK (m_pICommand->Execute (NULL, IID_IUnknown, NULL, &cRowsAffected, (IUnknown **)&pRowset), 
		DB_E_PARAMNOTOPTIONAL))
	{
		goto CLEANUP;
	}


	//Set Command text with no parameters.
	TEST_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , pwszSqlSelectAllFromTbl), S_OK);

	// It should execute fine. (with ExecDbParams.cParamSets = 0 and pData = NULL )
	// For a command without parameter it should execute fine.
	if (!CHECK(m_pICommand->Execute(NULL, IID_IUnknown, &ExecDbParams, &cRowsAffected, 
		(IUnknown **)&pRowset),  S_OK))
	{
		goto CLEANUP;
	}


	fResult = TRUE;

CLEANUP:

	ReleaseRowsetPtr(&pRowset);
	ReleaseDataForCommand();

	if (hAccessor)
		CHECK(m_pCmdIAccessor->ReleaseAccessor (hAccessor, NULL), S_OK);

	// Restore original text.
	CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	FREE_DATA (pwszSqlSelectAllFromTbl);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Supply a value which could not be co-erced, returns DB_E_CANTCONVERTVALUE.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_12()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommandPrepare Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pwszNumeric		= NULL;		// Numeric value
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				count			= 0;		// Loop counter
	CList				<DBTYPE, DBTYPE> DBTypeList;
	CCol				NewCol(m_pIMalloc);			// Class CCol
	IAccessor *			pLocalAccessor = NULL;
	HACCESSOR			hAccessor = DB_NULL_HACCESSOR;
	DBPARAMS			Params;
	WCHAR				wcTmpVal = 'C'; // Some temporary value.
	void *				pData = NULL;
	
	
	// Creates a column list from the Ctable
	pcColumns = m_pTable->CountColumnsOnTable();

	// Loop thru column types
	for( count=1; count <= pcColumns; count++)
	{
		m_pTable->GetColInfo(count, NewCol);
		
		// If first column is already numeric then were done
		if( IsColDateTime(NewCol.GetProviderType()) )
			break;
	}

	if (count > pcColumns)
	{
		odtLog << L"Couldn't find a datetime column for this variation.\n";
		return TEST_SKIPPED;
	}

	DBTypeList.AddHead(NewCol.GetProviderType());

	// Create a table
	if(!CHECK(m_pExtraTable->CreateTable(DBTypeList,
									1,			// Number of rows to insert
									0,			// Column to put index on
									NULL,		// Table name
									PRIMARY),	// Primary or secondary values
									S_OK))
	{
		// Free memory in the list
		DBTypeList.RemoveAll();
		return TEST_FAIL;
	}

	// Get the name of the table just created
	pTableName = m_pExtraTable->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(g_wszInsertInvalidChar) + wcslen(pTableName))) );

	// Format SQL Statement
	swprintf(pwszSQLStmt, g_wszInsertInvalidChar, pTableName);

	
	//  Command to return a ICommand with Text Set
	if( !CHECK(m_pExtraTable->BuildCommand(pwszSQLStmt,		// SQL STMT
									m_iidExec,			// IID
									EXECUTE_NEVER,		// EXECUTE
									NULL,				// # Prop's
									NULL,				// Prop's
									&Params,			// Params
									NULL,				// # Rowset
									NULL,				// Rowsets
									&pICommand),		// ICommand
									S_OK) )
		goto CLEANUP;

	// Setup the parameter sturcture.

	// 1. Create the accessor.

	// Verify The Interface for a Command Accessor Object.
	if (!VerifyInterface(pICommand, IID_IAccessor,
			COMMAND_INTERFACE,(IUnknown **)&pLocalAccessor))
	{
		//ICommandAccessor is not supported
		goto CLEANUP;
	}
	// Create the bindings and the create handle to accessor.
	DBBINDING DbBinding;
	memset(&DbBinding, 0, sizeof(DBBINDING));

	DbBinding.dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
	DbBinding.eParamIO = DBPARAMIO_INPUT;
	DbBinding.iOrdinal = 1;
	DbBinding.pTypeInfo = NULL;
	DbBinding.obValue = offsetof(DATA, bValue);
	DbBinding.cbMaxLen = sizeof (WCHAR);
	DbBinding.obLength = offsetof(DATA, ulLength);
	DbBinding.obStatus = offsetof (DATA, sStatus);
	DbBinding.wType = DBTYPE_WSTR;
	DbBinding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
	DbBinding.pBindExt = NULL;
	DbBinding.bPrecision = 0;
	DbBinding.bScale = 0;


	// Call create accessor.
	TEST_CHECK (pLocalAccessor->CreateAccessor( 
		DBACCESSOR_PARAMETERDATA, 1, &DbBinding, m_cbRowSize,
		&hAccessor, NULL), S_OK);

	// Allocate memory
	pData = (void *)m_pIMalloc->Alloc(sizeof(DATA) + DbBinding.cbMaxLen + sizeof(WCHAR));
	if (pData == NULL)
	{
		goto CLEANUP;
	}

	// 2. Create the data.
	memcpy((WCHAR *)((BYTE *)pData + DbBinding.obValue), &wcTmpVal, sizeof (WCHAR));
	LENGTH_BINDING(DbBinding, pData) = sizeof (WCHAR);
	STATUS_BINDING(DbBinding, pData) = DBSTATUS_S_OK ;

	// 3. Set the ParamData.
	Params.cParamSets = 1;
	Params.hAccessor = hAccessor;
	Params.pData = pData;

	// Need to call SetParameterInfo here else Execute result is undefined for providers that can't derive


	hr = pICommand->Execute(NULL, IID_NULL, &Params, NULL, NULL);

	if (hr != DB_E_CANTCONVERTVALUE)
	{
		TEST_CHECK (hr, DB_E_ERRORSOCCURRED);
		TEST_COMPARE (STATUS_BINDING(DbBinding, pData), DBSTATUS_E_CANTCONVERTVALUE);
	}


	fSuccess = TRUE;

CLEANUP:
	// Drop the table
	m_pExtraTable->DropTable();

	// Free memory in the list
	DBTypeList.RemoveAll();

	// Free Memory
	FREE_DATA ( pwszSQLStmt );
	FREE_DATA ( pData);
			
	if (pLocalAccessor) pLocalAccessor->ReleaseAccessor (hAccessor, NULL);

	RELEASE (pLocalAccessor);
	RELEASE ( pICommand );

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc hAccessor in the DBPARAMS structure was invalid (DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_13()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBPARAMS ExecDbParams;
	void *pData = NULL;
	DBROWCOUNT cRowsAffected;
	IRowset *pRowset = NULL;
	BOOL fResult = FALSE;
	DBCOUNTITEM ulRowNum = 0;
	HRESULT hr = E_FAIL;
	
	// Create data.
	TEST_CHECK(FillInputBindings((CTable *)m_pThisTestModule->m_pVoid,
				DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings,
				(BYTE **)&pData,(ulRowNum = NextInsertRowNum()),  m_cParamColMap,
				(DB_LORDINAL *)m_rgParamColMap), S_OK);

	ExecDbParams.cParamSets = 1;
	ExecDbParams.hAccessor = hAccessor;
	ExecDbParams.pData = pData;
	
	hr = m_pICommand->Execute(NULL, IID_IUnknown, &ExecDbParams, 
		&cRowsAffected, (IUnknown **)&pRowset);

	if (FAILED(hr))
		// Since the row wasn't inserted, decrement next insert row number incremented above.
		m_cInsertRowNum--;

	TEST_CHECK(hr, DB_E_BADACCESSORHANDLE);

	fResult = TRUE;

CLEANUP:

	if (ExecDbParams.pData)
		ReleaseInputBindingsMemory(m_cBindings, m_rgBindings, (BYTE *) ExecDbParams.pData, TRUE);
	
	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc TEST Stuff
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_14()
{

	odtLog << L"This variation only duplicates more robust testing in other variations and was removed.\n";
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Stored procedure with Valid Input/Output data.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_15()
{
	// Need to test with all paramter names set to NULL so we can verify
	// names aren't required.
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	ULONG iParam;
	HRESULT hrSet = DB_S_TYPEINFOOVERRIDDEN;
	DB_UPARAMS cActParams = 0;
	WCHAR * pNamesBuffer = NULL;
	DBPARAMINFO * pParamInfo = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set all the parameter names NULL
	for (iParam=0; iParam < cParams; iParam++)
		pPARAMBIND[iParam].pwszName = NULL;

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	if (!fCanDerive)
		hrSet = S_OK;

	// Now set the parameter information correctly
	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), hrSet);

	// Get it again so we can validate we get back what we set.
	ABORT_CHECK(m_pICmdWParams->GetParameterInfo(&cActParams, &pParamInfo, &pNamesBuffer), S_OK);

	// Verify results.  If we didn't get back what was set it might not be a failure
	FAIL_VAR(VerifyParamInfo(cParams, pParamOrdinals, pPARAMBIND,
			cActParams, pParamInfo, pNamesBuffer), S_OK);

	// The final proof is that we can execute with these values
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll, ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_PDATA, TRUE), S_OK);


CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pParamInfo);
	PROVIDER_FREE(pNamesBuffer);
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Stored Procedure with Valid OUTPUT only parameters.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_16()
{
		
	// Need to test with all paramter names set to NULL so we can verify
	// names aren't required.
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize;
	DBCOUNTITEM ulRowNum = 3;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	ULONG iParam;
	HRESULT hrSet = DB_S_TYPEINFOOVERRIDDEN;
	DB_UPARAMS cActParams = 0;
	WCHAR * pNamesBuffer = NULL;
	DBPARAMINFO * pParamInfo = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set all the parameter names NULL
	for (iParam=0; iParam < cParams; iParam++)
		pPARAMBIND[iParam].pwszName = NULL;

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	if (!fCanDerive)
		hrSet = S_OK;

	// Now set the parameter information correctly
	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), hrSet);

	// Get it again so we can validate we get back what we set.
	ABORT_CHECK(m_pICmdWParams->GetParameterInfo(&cActParams, &pParamInfo, &pNamesBuffer), S_OK);

	// Verify results.  If we didn't get back what was set it might not be a failure
	FAIL_VAR(VerifyParamInfo(cParams, pParamOrdinals, pPARAMBIND,
			cActParams, pParamInfo, pNamesBuffer), S_OK);

	// The final proof is that we can execute with these values
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll, ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE), S_OK);


CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pParamInfo);
	PROVIDER_FREE(pNamesBuffer);
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Stored procedure with NULL [out] and Valid [input] parameters.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_17()
{
	BOOL fResult = TRUE;
	HRESULT hr = E_FAIL;
	ULONG iCol; 
	DBCOUNTITEM ulRowNum;
	DBCOUNTITEM cBinding = 0;
	DBLENGTH cbRowSize = 0;
	CCol TempCol;
	HACCESSOR hParamAccessor = DB_NULL_HACCESSOR;
	DBBINDING * pBinding = NULL;
	DBPARAMBINDINFO rgParamBindInfo[2];
	DB_LPARAMS rgParamOrdinals[2] = {1, 2};
	DBPARAMS 	Param;
	ParamStruct rgParamAll[2];
	IAccessor * pCmdIAccessor = NULL;
	BYTE * pData = NULL;
	WCHAR * pTableStringsBuffer = NULL;
	WCHAR * pwszCreateProc = NULL;
	WCHAR * pwszExecProc = NULL;
	WCHAR * pwszDropProc = NULL;
	WCHAR * pwszProcName = NULL;
	CList <WCHAR *, WCHAR *> DBTypeList;
	// New table, no NULLS.
	CTable DupColTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName, NULLABLE);

	// Due to the nature of this variation it needs to create a table with two columns
	// of identical types, looping through all the data types.  This will NOT work when
	// using an ini file or against a read-only provider 'cause we can't create a new table.

	// I/O params only supported with procs
	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	// If no output params, then no I/O params.
	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	pwszProcName = MakeObjectName(L"ICMDWPAR", wcslen(m_pTable->GetTableName()));
	TEST_PTR(pwszProcName);

	// Tell the syntax builder to use a temporary proc name
	m_Syntax.SetProcName(pwszProcName);

	//Get accessor interface on the command object on which we will do the execute
	ABORT_COMPARE(VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE, (IUnknown **)&pCmdIAccessor), TRUE);

	// For simplicity, use the default table as the source of the column info for the new table
	for (iCol = 1; iCol <= m_pTable->CountColumnsOnTable(); iCol++)
	{
		// Get the column information
		ABORT_CHECK(m_pTable->GetColInfo(iCol, TempCol), S_OK);

		// If the column is not updatable or is not nullable or is LONG then it's useless to us
		if (!TempCol.GetUpdateable() || !TempCol.GetNullable() || TempCol.GetIsLong())
			continue;

		// Now we create a table with two columns of this type
		DBTypeList.AddTail(TempCol.GetProviderTypeName());
		DBTypeList.AddTail(TempCol.GetProviderTypeName());

		// If table creation succeeded we'd like to go ahead with the rest of the
		// testing and at least drop the table
		hr = DupColTable.CreateTable(DBTypeList,0,0);
		IF_CHECK(hr, S_OK)
		{
			CCol DupCol;

			// Insert 5 non-NULL rows
			FAIL_CHECK(DupColTable.InsertWithParams(0, PRIMARY, TRUE, NULL, 5), S_OK);

			// Get the column information
			ABORT_CHECK(DupColTable.GetColInfo(1, DupCol), S_OK);

			// Create a parameter name for this table
			rgParamAll[0].pwszParamName = m_Syntax.MakeParamName(DupCol, 'P');
			rgParamAll[1].pwszParamName = m_Syntax.MakeParamName(DupCol, 'P');

			// Create a statement and corresponding accessor to insert another row
			hr = DupColTable.ExecuteCommand(SELECT_UPDATEABLE, IID_IUnknown,
					NULL, NULL, NULL, NULL, 
					EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand);
			IF_CHECK(hr, S_OK)
			{

				// If prepare is supported we must prepare before we can get 
				// the columns info
				if (m_pICommandPrepare)
				{
					FAIL_CHECK(m_pICommandPrepare->Prepare(1), S_OK);
				}

				// Create param info for optional SetParameterInfo call
				AddParam(0, 1, DBPARAMIO_INPUT, rgParamAll[0].pwszParamName, TRUE, NULL, NULL, rgParamBindInfo,
					rgParamAll, &DupColTable);
				AddParam(1, 2, DBPARAMIO_INPUT, rgParamAll[1].pwszParamName, TRUE, NULL, NULL, rgParamBindInfo,
					rgParamAll, &DupColTable);

				// Call SetParameterInfo if needed
				// We have to call SetParameterInfo if the provider can't derive
				// otherwise Execute behavior is "undefined" per spec.
				SetParameterInfoIfNeeded(2, (DB_UPARAMS *)rgParamOrdinals, rgParamBindInfo, m_pICommand);

				// Create a parameter accessor
				hr = GetAccessorAndBindings(pCmdIAccessor, 
					DBACCESSOR_PARAMETERDATA,&hParamAccessor, &pBinding, &cBinding, &cbRowSize,			
  					DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
					ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
					NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
					NO_COLS_OWNED_BY_PROV,	DBPARAMIO_INPUT,
					BLOB_LONG);

				IF_CHECK(hr, S_OK)
				{
					ulRowNum = DupColTable.GetNextRowNumber();

					// Fill the bindings with normal data for the next available row
					hr = FillInputBindings(&DupColTable, DBACCESSOR_ROWDATA, cBinding,
						pBinding, &pData, ulRowNum, 0, NULL, PRIMARY);
					IF_CHECK(hr, S_OK)
					{
						// Set the second column to be NULL.  We use the second column 'cause the
						// first one might be a primary key that can't be NULL.
						*(DBSTATUS *)(pData+pBinding[1].obStatus) = DBSTATUS_S_ISNULL;

						// Set the command text to insert with params
						hr = DupColTable.ExecuteCommand(INSERT_ALLWITHPARAMS, IID_IUnknown,
								NULL, NULL, NULL, NULL, 
								EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand);
						IF_CHECK(hr, S_OK)
						{
							DBROWCOUNT cRowsAffected = 0;

							// Set the Execute params.
							Param.cParamSets = 1;
							Param.hAccessor = hParamAccessor;
							Param.pData = pData;

							// Now insert the row with the NULL
							hr = m_pICommand->Execute(NULL, IID_NULL, &Param, 
								&cRowsAffected, NULL);

							// Release the accessor now that we're done with the row
							SAFE_RELEASE_ACCESSOR(pCmdIAccessor, hParamAccessor);

							IF_CHECK(hr,S_OK)
							{
								// Make sure we have one row assuming the provider knows
								IF_COMPARE(cRowsAffected == 1 || cRowsAffected == DB_COUNTUNAVAILABLE, TRUE)
								{

									// Change the first parameter to Input/Output
									pBinding[0].eParamIO = DBPARAMIO_INPUT | DBPARAMIO_OUTPUT;
									rgParamBindInfo[0].dwFlags |= DBPARAMFLAGS_ISINPUT | DBPARAMFLAGS_ISOUTPUT;
									rgParamAll[0].eParamIO = pBinding[0].eParamIO;

									// Give required information to the syntax builder
									m_Syntax.SetColMap(cBinding-1, rgParamAll);
									m_Syntax.SetCurrentRow(ulRowNum);
									m_Syntax.SetTable(&DupColTable);

									// Now we can actually create the stored proc and execute it
									// select c2 into p1 where c1 = p1
									pwszCreateProc = m_Syntax.GetSyntax(T_CREATE_PROC_SELECT_LOOKUP);
									pwszExecProc = m_Syntax.GetSyntax(T_EXEC_PROC_SELECT_LOOKUP);
									pwszDropProc = m_Syntax.GetSyntax(T_DROP_PROC);

									// Make sure we got the pointers back
									ABORT_PTR(pwszCreateProc);
									ABORT_PTR(pwszExecProc);
									ABORT_PTR(pwszDropProc);

									// Drop the stored proc in case it exists due to a crashed test
									// Normally will not exist, so don't check return code.
									DupColTable.BuildCommand(pwszDropProc, IID_NULL, EXECUTE_ALWAYS, 0, NULL,
										NULL, NULL, NULL, &m_pICommand);

									// Create the stored proc using the syntax required
									hr = DupColTable.BuildCommand(pwszCreateProc, IID_NULL, EXECUTE_ALWAYS, 0, NULL,
										NULL, NULL, NULL, &m_pICommand);
									IF_CHECK(hr, S_OK)
									{
										
										// Set the parameter accessor and parameters
										ABORT_CHECK(pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA,
											cBinding-1,
											pBinding,
											cbRowSize,
											&hParamAccessor,
											NULL), S_OK);

										Param.cParamSets = 1;
										Param.hAccessor = hParamAccessor;
										Param.pData = pData;

										// Set the command text only
										ABORT_CHECK(DupColTable.BuildCommand(pwszExecProc, IID_NULL, EXECUTE_NEVER, 0, NULL,
											NULL, NULL, NULL, &m_pICommand), S_OK);

										// We need to prepare the command before we can desribe params, if we can at all
										if (m_pICommandPrepare)
											ABORT_CHECK(hr = m_pICommandPrepare->Prepare(1), S_OK);

										ABORT_CHECK(SetParameterInfoIfNeeded(1, (DB_UPARAMS *)rgParamOrdinals, rgParamBindInfo, m_pICommand), S_OK);

										// Execute the stored proc
										hr = DupColTable.BuildCommand(pwszExecProc, IID_NULL, EXECUTE_ALWAYS, 0, NULL,
											&Param, NULL, NULL, &m_pICommand);
										IF_CHECK(hr, S_OK)
										{
											// Validate the results.  The status must be NULL.
											FAIL_COMPARE(*(DBSTATUS *)(pData+pBinding[0].obStatus), DBSTATUS_S_ISNULL);
										}

										SAFE_RELEASE_ACCESSOR(pCmdIAccessor, hParamAccessor);

										// Drop the stored proc.  Can't do anything about failure
										ABORT_CHECK(DupColTable.BuildCommand(pwszDropProc, IID_NULL, EXECUTE_ALWAYS, 0, NULL,
											NULL, NULL, NULL, &m_pICommand), S_OK);
									}


									SAFE_FREE(pwszCreateProc);
									SAFE_FREE(pwszExecProc);
									SAFE_FREE(pwszDropProc);
								}
							}
						}

						SAFE_FREE(pData);
					}
				}

				// Release the accessor
				SAFE_RELEASE_ACCESSOR(pCmdIAccessor, hParamAccessor);
			}

			// Drop the table
			DupColTable.DropTable();
		}

		DBTypeList.RemoveAll();

		SAFE_FREE(pBinding);
	}

CLEANUP:

	m_Syntax.SetTable(m_pTable);
	SAFE_FREE(pwszProcName);
	SAFE_FREE(pwszCreateProc);
	SAFE_FREE(pwszExecProc);
	SAFE_FREE(pwszDropProc);
	SAFE_FREE(pData);
	SAFE_FREE(pBinding);
	SAFE_FREE(pTableStringsBuffer);

	SAFE_RELEASE_ACCESSOR(pCmdIAccessor, hParamAccessor);

	// Drop the table
	DupColTable.DropTable();

	SAFE_RELEASE(pCmdIAccessor);

	return fResult;
		
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Stored procedure with NULL Output only parameters.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_18()
{
	odtLog << L"Testing that NULL output params can be returned was done in Variation_17.\n";
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Stored procedure with NULL input and Valid Output parameters.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_19()
{
	// NULL input param is illegal in the sense it will never return any rows.
	// This might be tested using provider-specific batch SQL:
	//	insert into table values @p1; select @p1=col2 where col1 IS NULL.
	odtLog << L"This variation is not a valid test for SQL providers and is provider specific, therefore was removed.\n";
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Input/Output parameters with only Status bound to NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_20()
{
	/*
	NULL input param is illegal in the sense it will never return any rows and therefore will not
	populate the output param.  

	We should add a test case for populating output params when the search clause doesn't return any rows.  In the
	meantime this variation can be skipped as it doesn't provide any
	valid testing, at least for SQL based providers. We could consider just skipping the variation if the provider
	is SQL based but leaving it for non-SQL providers.  However, it would be provider-specific whether they support
	retrieving NULL data by binding to DBSTATUS_S_ISNULL.
	*/

	odtLog << L"This variation is not a valid test for SQL providers and is provider specific, therefore was removed.\n";
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Output only parameters with Status only binding to NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_21()
{
	// "binding to NULL" has no meaning for output params, however, a NULL can be
	// returned to an output param with status-only binding.
	// Use "select NULL, NULL, NULL, ... into p1, p2, p3, ... from table where
	// col1 = l1 and col2 = l2 ...

	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 2;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT_NULL,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Reset the binding status to only DBPART_STATUS for the output params
	for (iParam=0; iParam < cParams; iParam++)
		if (pBINDING[iParam].eParamIO == DBPARAMIO_OUTPUT)
			pBINDING[iParam].dwPart = DBPART_STATUS;

	// Validate the parameters and execute results
	FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
		pBINDING, cbRowSize, pParamAll, pData, ROWSET_NONE, cColumns, prgColumnsOrd, fCanDerive,
		VERIFY_NULL), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Input/output with status only binding to S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_22()
{
	// Error case (DB_E_ERRORSOCCURRED) for status-only binding on input.  No output
	// params are populated, all status set to DBSTATUS_E_UNAVAILABLE.

	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Reset the binding status to only DBPART_STATUS and status to S_OK (as it might be DBSTATUS_S_ISNULL)
	for (iParam=0; iParam < cParams; iParam++)
	{
		if (pBINDING[iParam].eParamIO == (DBPARAMIO_OUTPUT | DBPARAMIO_INPUT))
		{
			pBINDING[iParam].dwPart = DBPART_STATUS;
			pBINDING[iParam].cbMaxLen = ULONG_MAX;	// cbMaxLen should be ignored if no value
			*(DBSTATUS *)(pData+pBINDING[iParam].obStatus) = DBSTATUS_S_OK;
			*(DBSTATUS *)(pData+pBINDING[iParam].obValue) = 0;
			*(DBSTATUS *)(pData+pBINDING[iParam].obLength) = 0;
		}
	}

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);
	
	// Create param accessor and call Execute expecting DB_E_ERRORSOCCURRED
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_PDATA, TRUE, m_pICommand, DB_E_ERRORSOCCURRED), S_OK);

	// Validate the status values returned.  
	for (iParam=0; iParam < cParams; iParam++)
	{
		if (pBINDING[iParam].eParamIO == (DBPARAMIO_OUTPUT | DBPARAMIO_INPUT))
		{
			FAIL_COMPARE(*(DBSTATUS *)(pData+pBINDING[iParam].obStatus), DBSTATUS_E_UNAVAILABLE);
		}
	}

	
CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Output only with status only binding to S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_23()
{
	// This should be DBSTATUS_S_ISNULL for NULL columns or DBSTATUS_S_OK for non-NULL columns.
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam, cParamsChanged = 0;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 2;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrExp = DB_S_ERRORSOCCURRED;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Reset the binding status to only DBPART_STATUS for the output params
	for (iParam=0; iParam < cParams; iParam++)
	{
		if (pBINDING[iParam].eParamIO == DBPARAMIO_OUTPUT)
		{
			pBINDING[iParam].dwPart = DBPART_STATUS;
			*(DBSTATUS *)(pData+pBINDING[iParam].obStatus) = ULONG_MAX;
			cParamsChanged++;
		}
	}

	if (cParamsChanged == cParams)
		hrExp = DB_E_ERRORSOCCURRED;

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);

	// Create param accessor and call Execute expecting DB_S_ERRORSOCCURRED
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE, m_pICommand, hrExp), S_OK);

	// Validate the status values returned.  
	for (iParam=0; iParam < cParams; iParam++)
	{
		if (pBINDING[iParam].eParamIO == DBPARAMIO_OUTPUT)
		{	
			WCHAR wszData[MAXDATALEN] = L"";
			HRESULT hrMakeData = m_pTable->MakeData(wszData, ulRowNum, pParamAll[iParam].ulColIndex, PRIMARY, TRUE);

			switch(hrMakeData)
			{
				case S_OK:
					FAIL_COMPARE(*(DBSTATUS *)(pData+pBINDING[iParam].obStatus), DBSTATUS_S_OK);
					break;
				case S_FALSE:
					FAIL_COMPARE(*(DBSTATUS *)(pData+pBINDING[iParam].obStatus), DBSTATUS_S_ISNULL);
					break;
				default:
					ASSERT(!L"MakeData failure!!");
			}
		}
	}


CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Input/Output with Length and Value only bound.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_24()
{
	// On input, provider assumes DBSTATUS_S_OK, so for a valid success case we must
	// have non-NULL data for input params.
	// S_OK for non-NULL ouput data, DB_S_ERRORSOCCURRED for some NULL output data.
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Reset the binding status to only DBPART_STATUS and status to S_OK (as it might be DBSTATUS_S_ISNULL)
	for (iParam=0; iParam < cParams; iParam++)
	{
		if (pBINDING[iParam].eParamIO == (DBPARAMIO_OUTPUT | DBPARAMIO_INPUT))
		{
			pBINDING[iParam].dwPart = DBPART_LENGTH | DBPART_VALUE;
			*(DBSTATUS *)(pData+pBINDING[iParam].obStatus) = DBSTATUS_S_OK;
		}
	}

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);

	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_PDATA, TRUE, m_pICommand, S_OK), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Output only with Length and Value bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_25()
{
	// S_OK for non-NULL ouput data, DB_S_ERRORSOCCURRED for some NULL output data.
	// On input, provider assumes DBSTATUS_S_OK.
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 3;
	DBCOUNTITEM cRowsObtained=0;
	HROW * prghRow = NULL;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	BYTE * pDataSt = NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Reset the binding status to only DBPART_STATUS and status to a bad value
	for (iParam=0; iParam < cParams; iParam++)
	{
		if (pBINDING[iParam].eParamIO == DBPARAMIO_OUTPUT)
		{
			pBINDING[iParam].dwPart = DBPART_LENGTH | DBPART_VALUE;
			*(DBSTATUS *)(pData+pBINDING[iParam].obStatus) = DBSTATUS_E_BADSTATUS;
		}
	}

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);
	
	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE, m_pICommand, S_OK), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
	
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Input/Output with Value and Status bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_26()
{
	// S_OK for fixed-length input data, output data.  For DBTYPE_BYTES data and DBSTATUS_S_OK	
	// the provider assumes the length value is cbMaxLen from the binding.  Error for populating
	// output params.  For strings we can assume NULL terminated if the length isn't bound.
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 2;
	DBCOUNTITEM cRowsObtained=0;
	HROW * prghRow = NULL;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	BYTE * pDataSt = NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Reset the binding status to only DBPART_STATUS and DBPART_VALUE
	for (iParam=0; iParam < cParams; iParam++)
	{
		if (pBINDING[iParam].eParamIO == (DBPARAMIO_OUTPUT | DBPARAMIO_INPUT) &&
			(IsFixedLength(pBINDING[iParam].wType) ||
			pBINDING[iParam].wType == DBTYPE_STR ||
			pBINDING[iParam].wType == DBTYPE_WSTR))

		{
			pBINDING[iParam].dwPart = DBPART_VALUE | DBPART_STATUS;
			*(DBSTATUS *)(pData+pBINDING[iParam].obStatus) = DBSTATUS_S_OK;
		}
	}

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);
	
	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_PDATA, TRUE, m_pICommand, S_OK), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc output only with Value and Status bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_27()
{

	// S_OK for fixed-length input data, output data.  For DBTYPE_BYTES data and DBSTATUS_S_OK	
	// the provider assumes the length value is cbMaxLen from the binding.  Error for populating
	// output params.  For strings we can assume NULL terminated if the length isn't bound.
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 3;
	DBCOUNTITEM cRowsObtained=0;
	HROW * prghRow = NULL;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	BYTE * pDataSt = NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Reset the binding status to only DBPART_STATUS and DBPART_VALUE
	for (iParam=0; iParam < cParams; iParam++)
	{
		
		if (pBINDING[iParam].eParamIO == DBPARAMIO_INPUT)
		{
			pBINDING[iParam].dwPart = DBPART_VALUE | DBPART_STATUS;
			*(DBSTATUS *)(pData+pBINDING[iParam].obStatus) = DBSTATUS_S_OK;
			*(DBSTATUS *)(pData+pBINDING[iParam].obLength) = 0;
		}
		if (pBINDING[iParam].eParamIO == DBPARAMIO_OUTPUT &&
			(IsFixedLength(pBINDING[iParam].wType) ||
			pBINDING[iParam].wType == DBTYPE_STR ||
			pBINDING[iParam].wType == DBTYPE_WSTR))

		{
			pBINDING[iParam].dwPart = DBPART_VALUE | DBPART_STATUS;
			*(DBSTATUS *)(pData+pBINDING[iParam].obStatus) = DBSTATUS_S_OK;
			*(DBSTATUS *)(pData+pBINDING[iParam].obLength) = 0;
		}
	}

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);
	
	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE, m_pICommand, S_OK), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Input/output with only Value bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_28()
{
	// Error for NULL data, S_OK for input param, error for variable length binary output data
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 3;
	DBCOUNTITEM cRowsObtained=0;
	HROW * prghRow = NULL;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	BYTE * pDataSt = NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Reset the binding status to only DBPART_VALUE.  We don't reset for NULL values,
	// since that's an error case and we want a success case.  Variable length non null
	// terminated data is also an error case since we don't know the length.
	for (iParam=0; iParam < cParams; iParam++)
	{
		if (pBINDING[iParam].eParamIO == (DBPARAMIO_OUTPUT | DBPARAMIO_INPUT) &&
			(IsFixedLength(pBINDING[iParam].wType) ||
			pBINDING[iParam].wType == DBTYPE_STR ||
			pBINDING[iParam].wType == DBTYPE_WSTR) &&
			STATUS_BINDING(pBINDING[iParam], pData) != DBSTATUS_S_ISNULL)
		{
			pBINDING[iParam].dwPart = DBPART_VALUE;
			pBINDING[iParam].obLength = 0;
			pBINDING[iParam].obStatus = 0;
		}
	}

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);
	
	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_PDATA, TRUE, m_pICommand, S_OK), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc output only with Value only bound.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_29()
{
	// Error for NULL data, S_OK for input param, error for variable length binary output data
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 3;
	DBCOUNTITEM cRowsObtained=0;
	HROW * prghRow = NULL;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	BYTE * pDataSt = NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Reset the binding status to only DBPART_VALUE.  We don't reset for NULL values,
	// since that's an error case and we want a success case.  Variable length non null
	// terminated data is also an error case since we don't know the length.
	for (iParam=0; iParam < cParams; iParam++)
	{
		if ((IsFixedLength(pBINDING[iParam].wType) ||
			pBINDING[iParam].wType == DBTYPE_STR ||
			pBINDING[iParam].wType == DBTYPE_WSTR) &&
			STATUS_BINDING(pBINDING[iParam], pData) != DBSTATUS_S_ISNULL)
		{
			pBINDING[iParam].dwPart = DBPART_VALUE;
			pBINDING[iParam].obLength = 0;
			pBINDING[iParam].obStatus = 0;
		}
	}

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);
	
	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE, m_pICommand, S_OK), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;


}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc INPUT/OUTPUT with BY_REF bindings
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_30()
{
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 3, cbByRef = 0, ulOffSet=0;
	DBCOUNTITEM cRowsObtained=0;
	HROW * prghRow = NULL;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	BYTE * pDataBYREF=NULL;
	DBBINDING * pBINDINGBYREF=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	IConvertType * pIConvertType = NULL;
	DBTYPE wType;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);


	// Get an IConverType interface from the command
	VerifyInterface (m_pICommand, IID_IConvertType, COMMAND_INTERFACE, 	(IUnknown **)&pIConvertType);

	// By default CreateProcBindings creates in-line bindings, not BYREF.  We need to
	// go through all the bindings and set them to BYREF.  Can't use FillInputBindings
	// because for input/output params we don't know the underlying row number.
	
	// Find out how big our data buffer will be when set for BYREF.
	// It is provider-specific whether fixed-len types can be bound BYREF, so
	// we can't include those.
	for(iParam=0; iParam < cParams; iParam++)
	{
		wType = pBINDING[iParam].wType;

		cbByRef+=sizeof(DBSTATUS)+sizeof(DBLENGTH);

		if(pIConvertType && (S_OK == pIConvertType->CanConvert(wType, wType|DBTYPE_BYREF, DBCONVERTFLAGS_PARAMETER)) ||
			!IsFixedLength(wType))
			cbByRef+=sizeof(LPVOID);
		else
			cbByRef+=GetDBTypeSize(wType);
	}
	
	// Copy the DBBINDING array since we need it to use in the comparison
	TEST_ALLOC(DBBINDING, pBINDINGBYREF, 0, cParams * sizeof(DBBINDING));
	memcpy(pBINDINGBYREF, pBINDING, cParams * sizeof(DBBINDING));

	// Go through the old binding structure and convert to BYREF.  Note we
	// can't copy the old pData and convert in place because some data types
	// may be smaller than sizeof(LPVOID).
	for(iParam=0; iParam < cParams; iParam++)
	{
		wType = pBINDING[iParam].wType;

		// Reset obValue, obLength, obStatus
		pBINDINGBYREF[iParam].obStatus = ulOffSet;
		ulOffSet+=sizeof(DBSTATUS);
		ulOffSet = ROUND_UP(ulOffSet, ROUND_UP_AMOUNT);
		pBINDINGBYREF[iParam].obLength = ulOffSet;
		ulOffSet+=sizeof(DBLENGTH);
		pBINDINGBYREF[iParam].obValue = ulOffSet;

		if(pIConvertType && (S_OK == pIConvertType->CanConvert(wType, wType|DBTYPE_BYREF, DBCONVERTFLAGS_PARAMETER)) ||
			!IsFixedLength(wType))
		{
			// Change the binding to BYREF.
			pBINDINGBYREF[iParam].wType |= DBTYPE_BYREF;
			pBINDINGBYREF[iParam].cbMaxLen = sizeof(LPVOID);
		}

		ulOffSet+=pBINDINGBYREF[iParam].cbMaxLen;
		ulOffSet = ROUND_UP(ulOffSet, ROUND_UP_AMOUNT);
	}

	// Create a new pDataBYREF buffer to use for the BYREF bindings.  We'll just
	// point our new params to the old pData since it already has all the right
	// values.  
	TEST_ALLOC(BYTE, pDataBYREF, 0, (size_t)ulOffSet);

	for(iParam=0; iParam < cParams; iParam++)
	{
		wType = pBINDING[iParam].wType;

		// Copy the status
		STATUS_BINDING(pBINDINGBYREF[iParam], pDataBYREF) = STATUS_BINDING(pBINDING[iParam], pData);
		// Copy the length
		LENGTH_BINDING(pBINDINGBYREF[iParam], pDataBYREF) = LENGTH_BINDING(pBINDING[iParam], pData);

		if(pIConvertType && (S_OK == pIConvertType->CanConvert(wType, wType|DBTYPE_BYREF, DBCONVERTFLAGS_PARAMETER)) ||
			!IsFixedLength(wType))
		{
			// Set the value to point to the old pData's value
			(LPVOID)VALUE_BINDING(pBINDINGBYREF[iParam], pDataBYREF) = &VALUE_BINDING(pBINDING[iParam], pData);
			// If the data for the parameter is NULL, make the pointer NULL.
			if (STATUS_BINDING(pBINDINGBYREF[iParam], pData) == DBSTATUS_S_ISNULL)
				VALUE_BINDING(pBINDINGBYREF[iParam], pDataBYREF) = NULL;
		}
		else
		{
			pBINDINGBYREF[iParam].cbMaxLen = pBINDING[iParam].cbMaxLen;
			// Copy the data
			memcpy(&VALUE_BINDING(pBINDINGBYREF[iParam], pDataBYREF), 
				&VALUE_BINDING(pBINDING[iParam], pData), (size_t)pBINDINGBYREF[iParam].cbMaxLen); 
		}

	}

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);
	
	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDINGBYREF, cbByRef, pDataBYREF, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_PDATA, TRUE, m_pICommand, S_OK, pBINDING, pData+cbRowSize), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	SAFE_RELEASE(pIConvertType);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pBINDINGBYREF);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pDataBYREF);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc OUTPUT_ONLY with BY_REF bindings.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_31()
{
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1, iParam;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 3, cbByRef = 0, ulOffSet=0;
	DBCOUNTITEM cRowsObtained=0;
	HROW * prghRow = NULL;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	BYTE * pDataBYREF=NULL;
	DBBINDING * pBINDINGBYREF=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	IConvertType * pIConvertType = NULL;
	DBTYPE wType;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);


	// Get an IConverType interface from the command
	VerifyInterface (m_pICommand, IID_IConvertType, COMMAND_INTERFACE, 	(IUnknown **)&pIConvertType);

	// By default CreateProcBindings creates in-line bindings, not BYREF.  We need to
	// go through all the bindings and set them to BYREF.  Can't use FillInputBindings
	// because for input/output params we don't know the underlying row number.
	
	// Copy the DBBINDING array since we need it to use in the comparison
	TEST_ALLOC(DBBINDING, pBINDINGBYREF, 0, cParams * sizeof(DBBINDING));
	memcpy(pBINDINGBYREF, pBINDING, cParams * sizeof(DBBINDING));

	// Go through the old binding structure and convert to BYREF.  Note we
	// can't copy the old pData and convert in place because some data types
	// may be smaller than sizeof(LPVOID).
	for(iParam=0; iParam < cParams; iParam++)
	{
		wType = pBINDING[iParam].wType;

		// Reset obValue, obLength, obStatus
		pBINDINGBYREF[iParam].obStatus = ulOffSet;
		ulOffSet+=sizeof(DBSTATUS);
		ulOffSet = ROUND_UP(ulOffSet, ROUND_UP_AMOUNT);
		pBINDINGBYREF[iParam].obLength = ulOffSet;
		ulOffSet+=sizeof(DBLENGTH);
		pBINDINGBYREF[iParam].obValue = ulOffSet;

		if(pIConvertType && (S_OK == pIConvertType->CanConvert(wType, wType|DBTYPE_BYREF, DBCONVERTFLAGS_PARAMETER)) ||
			!IsFixedLength(wType))
		{
			// Change the binding to BYREF.
			pBINDINGBYREF[iParam].wType |= DBTYPE_BYREF;
			pBINDINGBYREF[iParam].cbMaxLen = sizeof(LPVOID);
		}

		ulOffSet+=pBINDINGBYREF[iParam].cbMaxLen;
		ulOffSet = ROUND_UP(ulOffSet, ROUND_UP_AMOUNT);
	}

	// Create a new pDataBYREF buffer to use for the BYREF bindings.  We'll just
	// point our new params to the old pData since it already has all the right
	// values.  
	TEST_ALLOC(BYTE, pDataBYREF, 0, (size_t)ulOffSet);

	for(iParam=0; iParam < cParams; iParam++)
	{
		wType = pBINDING[iParam].wType;

		// Copy the status
		STATUS_BINDING(pBINDINGBYREF[iParam], pDataBYREF) = STATUS_BINDING(pBINDING[iParam], pData);
		// Copy the length
		LENGTH_BINDING(pBINDINGBYREF[iParam], pDataBYREF) = LENGTH_BINDING(pBINDING[iParam], pData);

		if(pIConvertType && (S_OK == pIConvertType->CanConvert(wType, wType|DBTYPE_BYREF, DBCONVERTFLAGS_PARAMETER)) ||
			!IsFixedLength(wType))
		{
			// Set the value to point to the old pData's value
			(LPVOID)VALUE_BINDING(pBINDINGBYREF[iParam], pDataBYREF) = &VALUE_BINDING(pBINDING[iParam], pData);
			// If the data for the parameter is NULL, make the pointer NULL.
			if (STATUS_BINDING(pBINDINGBYREF[iParam], pData) == DBSTATUS_S_ISNULL)
				VALUE_BINDING(pBINDINGBYREF[iParam], pDataBYREF) = NULL;
		}
		else
		{
			pBINDINGBYREF[iParam].cbMaxLen = pBINDING[iParam].cbMaxLen;
			// Copy the data
			memcpy(&VALUE_BINDING(pBINDINGBYREF[iParam], pDataBYREF), 
				&VALUE_BINDING(pBINDING[iParam], pData), (size_t)pBINDINGBYREF[iParam].cbMaxLen); 
		}

	}

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Set parameter info if provider can't derive
	if (!fCanDerive)
		ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);
	
	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDINGBYREF, ulOffSet, pDataBYREF, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE, m_pICommand, S_OK), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	SAFE_RELEASE(pIConvertType);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pBINDINGBYREF);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pDataBYREF);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Return value for a stored procedure.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_32()
{
//	return TestStoredProcBindings (RETURN_VALUE_TYPE, VALID_IN_OUT_DATA, LENGTH_VALUE_STATUS, REGULAR_PARAMNAMES);

	// TODO: Since TCGetParameterInfo_Rowset::Variation_16 only tests one return type, use this variation
	//		 to test other return data types if supported by the provider (Oracle does).	

	odtLog << L"This is already tested in TCGetParameterInfo_Rowset::Variation_16.\n";

	return TEST_SKIPPED;

}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Bind same output parameters multiple number of times.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_33()
{
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	DBBINDING * pMultiBinding=NULL;
	DBPARAMBINDINFO * pMultiParamBindInfo = NULL;
	ParamStruct * pMultiParamAll = NULL;
	DB_UPARAMS * pMultiParamOrdinals=NULL;
	DB_LORDINAL * pMultiColMap = NULL;
	ULONG cMultiParams = 0;
	DBLENGTH cbMultiRowSize = 0;
	ULONG iParam;
	HRESULT hrSet = S_OK;


	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		NULL,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// The bindings for the output parameters above are native bindings by default.
	// Create a binding to WCHAR for each output parameter since that's the only
	// supported/required conversion for every data type.
	cMultiParams = cParams * 2;
	TEST_ALLOC(DBBINDING, pMultiBinding, 0, cMultiParams*sizeof(DBBINDING));
	TEST_ALLOC(DBPARAMBINDINFO, pMultiParamBindInfo, 0, cMultiParams*sizeof(DBPARAMBINDINFO));
	TEST_ALLOC(DB_UPARAMS, pMultiParamOrdinals, 0, cMultiParams*sizeof(DB_UPARAMS));
	TEST_ALLOC(ParamStruct, pMultiParamAll, 0, cMultiParams*sizeof(ParamStruct));
	TEST_ALLOC(DB_LORDINAL, pMultiColMap, 0, cMultiParams*sizeof(DB_LORDINAL));

	// Copy the binding information one-by-one, but for each output param create an additional binding
	// to WCHAR.
	cMultiParams = 0;
	for (iParam = 0; iParam < cParams; iParam++)
	{
		// Copy the original binding info
		memcpy(&pMultiBinding[cMultiParams], &pBINDING[iParam], sizeof(DBBINDING));

		// Adjust offsets
		pMultiBinding[cMultiParams].obStatus = cbMultiRowSize;
		cbMultiRowSize += sizeof(DBSTATUS);
		cbMultiRowSize = ROUND_UP(cbMultiRowSize, ROUND_UP_AMOUNT);
		pMultiBinding[cMultiParams].obLength = cbMultiRowSize;
		cbMultiRowSize += sizeof(DBLENGTH);
		pMultiBinding[cMultiParams].obValue = cbMultiRowSize;
		cbMultiRowSize += pMultiBinding[cMultiParams].cbMaxLen;
		cbMultiRowSize = ROUND_UP(cbMultiRowSize, ROUND_UP_AMOUNT);

		memcpy(&pMultiParamBindInfo[cMultiParams], &pPARAMBIND[iParam], sizeof(DBPARAMBINDINFO));
		pMultiParamOrdinals[cMultiParams] = pParamOrdinals[iParam];
		memcpy(&pMultiParamAll[cMultiParams], &pParamAll[iParam], sizeof(ParamStruct));
		pMultiColMap[cMultiParams] = pParamAll[iParam].ulColIndex;
		cMultiParams++;
	
		// Create the WCHAR binding also if it's an output only parameter.  
		if (pBINDING[iParam].eParamIO == DBPARAMIO_OUTPUT)
		{
			CCol TempCol;

			TEST_CHECK(m_pTable->GetColInfo(pParamAll[iParam].ulColIndex, TempCol), S_OK);

			// Copy the original binding info
			memcpy(&pMultiBinding[cMultiParams], &pBINDING[iParam], sizeof(DBBINDING));

			// Adjust data type and cbMaxLen for WCHAR value
			pMultiBinding[cMultiParams].wType = DBTYPE_WSTR;
			pMultiBinding[cMultiParams].cbMaxLen = DisplaySize(TempCol)+sizeof(WCHAR);

			// Adjust offsets
			pMultiBinding[cMultiParams].obStatus = cbMultiRowSize;
			cbMultiRowSize += sizeof(DBSTATUS);
			cbMultiRowSize = ROUND_UP(cbMultiRowSize, ROUND_UP_AMOUNT);
			pMultiBinding[cMultiParams].obLength = cbMultiRowSize;
			cbMultiRowSize += sizeof(DBLENGTH);
			pMultiBinding[cMultiParams].obValue = cbMultiRowSize;
			cbMultiRowSize += pMultiBinding[cMultiParams].cbMaxLen;
			cbMultiRowSize = ROUND_UP(cbMultiRowSize, ROUND_UP_AMOUNT);

			// Copy the other associated info
			memcpy(&pMultiParamBindInfo[cMultiParams], &pPARAMBIND[iParam], sizeof(DBPARAMBINDINFO));
			pMultiParamOrdinals[cMultiParams] = pParamOrdinals[iParam];
			memcpy(&pMultiParamAll[cMultiParams], &pParamAll[iParam], sizeof(ParamStruct));
			pMultiColMap[cMultiParams] = pParamAll[iParam].ulColIndex;

			cMultiParams++;
		}
	}

	// Fill the parameters
	ABORT_CHECK (FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cMultiParams,
		pMultiBinding, &pData, ulRowNum, cMultiParams, pMultiColMap, PRIMARY), S_OK);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cMultiParams, pMultiParamOrdinals, pMultiParamBindInfo, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Since the count of parametr bindings doesn't match the count of parameter markers in the statement
	// we ALWAYS have to call SetParameterInfo
	if (fCanDerive)
		hrSet = DB_S_TYPEINFOOVERRIDDEN;

	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cMultiParams, pMultiParamOrdinals, pMultiParamBindInfo),
		hrSet);

	FAIL_VAR(ExecuteAndVerify(cMultiParams, cParamSets, pMultiParamAll, ulRowNum, pMultiBinding, cbMultiRowSize,
		pData, ROWSET_NONE, cMultiParams, pMultiColMap, VERIFY_USE_TABLE, TRUE), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	PROVIDER_FREE(pMultiBinding);
	PROVIDER_FREE(pMultiParamBindInfo);
	PROVIDER_FREE(pMultiParamOrdinals);
	PROVIDER_FREE(pMultiParamAll);
	PROVIDER_FREE(pMultiColMap);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc 1 parameter Set.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_34()
{
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	DBCOUNTITEM ulRowNum;
	DBCOUNTITEM cRowsExpected = 1;
	IRowset * pIRowset = NULL;

	// Remove any old parameter info
	TEST_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	//This sets it to one row of data.
	MakeDataForCommand((ulRowNum = NextInsertRowNum()));
		
	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Make sure the row really was inserted	
	TEST_COMPARE(cRow.FindRow(ulRowNum, m_pTable, NULL, &pIRowset, NULL,
		0, NULL, FALSE), TRUE);

	// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
	TEST_COMPARE(VerifyObj(m_iidExec, pIRowset, ulRowNum, 0, NULL,
		FALSE, FALSE, m_pTable, &cRowsExpected), S_OK);

	fResult = TRUE;
CLEANUP:

	SAFE_RELEASE(pIRowset);
	ReleaseDataForCommand();

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc 5 parameter sets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_35()
{

	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	ULONG cNumRows, i;
	DBCOUNTITEM *rgRowNums=NULL;
	BYTE *pData = NULL;
	IRowset * pIRowset = NULL;
	DBCOUNTITEM cRowsExpected = 1;
	
	if (!g_bMultipleParamSets)
	{
		odtLog << "Multiple parameter sets are not supported \n";
		return TEST_SKIPPED;
	}

	cNumRows = 5;  // For now

	TEST_ALLOC(DBCOUNTITEM, rgRowNums, 0, cNumRows * sizeof (DBCOUNTITEM));

	for (i = 0; i < cNumRows ; i++)
		rgRowNums[i] = NextInsertRowNum();

	// Remove any old parameter info
	TEST_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	TEST_ALLOC(BYTE, pData, 0, (size_t)(cNumRows * m_cbRowSize * sizeof (BYTE)));

	// Create the Parameter array.
	TEST_CHECK (FillInputBindingsForArray (m_pTable, DBACCESSOR_PARAMETERDATA, m_cbRowSize, m_cBindings, m_rgBindings,
		(BYTE **)&pData, cNumRows, rgRowNums, m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);

	m_DbParamsAll.cParamSets = cNumRows;
	m_DbParamsAll.hAccessor = m_hAccessor;
	m_DbParamsAll.pData = pData;

	TEST_CHECK(SetParameterInfoIfNeeded(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), S_OK);
	
	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Verify If we have inserted the row properly.
	for (i=0; i < cNumRows; i++)
	{
		// Verify If we have inserted the row properly.
		TEST_COMPARE(cRow.FindRow(rgRowNums[i], m_pTable, NULL, &pIRowset, NULL,
			0, NULL, FALSE), TRUE);

		// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
		TEST_COMPARE(VerifyObj(m_iidExec, pIRowset, rgRowNums[i], 0, NULL,
			FALSE, FALSE, m_pTable, &cRowsExpected), S_OK);

		SAFE_RELEASE(pIRowset);
	}

	fResult = TRUE;

CLEANUP:

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(rgRowNums);
	PROVIDER_FREE(pData);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc 105 parameter sets.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_36()
{
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	ULONG cNumRows, i;
	DBCOUNTITEM *rgRowNums=NULL;
	BYTE *pData = NULL;
	IRowset * pIRowset = NULL;
	DBCOUNTITEM cRowsExpected = 1;
	
	if (!g_bMultipleParamSets)
	{
		odtLog << "Multiple parameter sets are not supported \n";
		return TEST_SKIPPED;
	}

	cNumRows = 105;

	TEST_ALLOC(DBCOUNTITEM, rgRowNums, 0, cNumRows * sizeof (DBCOUNTITEM));

	for (i = 0; i < cNumRows ; i++)
		rgRowNums[i] = NextInsertRowNum();

	// Remove any old parameter info
	TEST_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	TEST_ALLOC(BYTE, pData, 0, (size_t)(cNumRows * m_cbRowSize * sizeof (BYTE)));

	// Create the Parameter array.
	TEST_CHECK (FillInputBindingsForArray (m_pTable, DBACCESSOR_PARAMETERDATA, m_cbRowSize, m_cBindings, m_rgBindings,
		(BYTE **)&pData, cNumRows, rgRowNums, m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);
	
	m_DbParamsAll.cParamSets = cNumRows;
	m_DbParamsAll.hAccessor = m_hAccessor;
	m_DbParamsAll.pData = pData;

	TEST_CHECK(SetParameterInfoIfNeeded(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), S_OK);

	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Verify If we have inserted the row properly.
	for (i=0; i < cNumRows; i++)
	{
		// Verify If we have inserted the row properly.
		TEST_COMPARE(cRow.FindRow(rgRowNums[i], m_pTable, NULL, &pIRowset, NULL,
			0, NULL, FALSE), TRUE);

		// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
		TEST_COMPARE(VerifyObj(m_iidExec, pIRowset, rgRowNums[i], 0, NULL,
			FALSE, FALSE, m_pTable, &cRowsExpected), S_OK);

		SAFE_RELEASE(pIRowset);
	}

	fResult = TRUE;
CLEANUP:

	SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(rgRowNums);
	PROVIDER_FREE(pData);


	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Different types of Good and Bad parameter Sets.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_37()
{

	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	ULONG cNumRows, i;
	DBCOUNTITEM *rgRowNums=NULL;
	void *pData = NULL;
	void * pInt = NULL;
	BYTE *pRowData = NULL;
	const BYTE cTinyInt25 = 25;
	const SHORT cShortMax = SHRT_MAX;
	const LONG cLongMax = LONG_MAX;
	const LONGLONG cInt64Max = _I64_MAX;
	DBLENGTH cbMaxLen = 0;
	DBLENGTH cbRowSize=m_cbRowSize;
	ULONG j = 0;
	LONG lSaveIndex = -1;
	DBTYPE dbSaveType;
	HACCESSOR hLocalAccessor = DB_NULL_HACCESSOR;
	ULONG iBadParam = 0;
	HRESULT hr = E_FAIL;
	DBBINDING * pBind = NULL;

	if (!g_bMultipleParamSets)
	{
		odtLog << "Multiple parameter sets are not supported \n";
		return TEST_SKIPPED;
	}

	// Make a copy of our bindings that we can change
	TEST_ALLOC(DBBINDING, pBind, 0, m_cBindings * sizeof(DBBINDING));
	memcpy(pBind, m_rgBindings, m_cBindings * sizeof(DBBINDING));

	// Find a column to insert data into
	for (i=0; i < m_cBindings; i++)
	{
		// We assume most providers will support one of these integer types
		if	(lSaveIndex == -1 && 
			(pBind[i].wType == DBTYPE_I1 ||
			pBind[i].wType == DBTYPE_UI1 ||
			pBind[i].wType == DBTYPE_I2 ||
			pBind[i].wType == DBTYPE_UI2 ||
			pBind[i].wType == DBTYPE_I4 ||
			pBind[i].wType == DBTYPE_UI4)) 
		{
			// Change the binding to the next larger integer type
			lSaveIndex = i;
			dbSaveType = pBind[i].wType;
			cbMaxLen = pBind[i].cbMaxLen;

			// The next type is always twice as large
			pBind[i].cbMaxLen *= 2;

			// Adjust the row size accordingly
			cbRowSize-=cbMaxLen;
			cbRowSize+=pBind[i].cbMaxLen;

			switch (pBind[i].wType)
			{
				case DBTYPE_I1:
				case DBTYPE_UI1:
					pBind[i].wType = DBTYPE_I2;
					pInt = (void *)&cShortMax;
					break;
				case DBTYPE_I2:
				case DBTYPE_UI2:
					pBind[i].wType = DBTYPE_I4;
					pInt = (void *)&cLongMax;
					break;
				case DBTYPE_I4:
				case DBTYPE_UI4:
					pBind[i].wType = DBTYPE_I8;
					pInt = (void *)&cInt64Max;
					break;
			}
		}
	}

	// Make sure we actually found one of the integer types we want
	if (lSaveIndex == -1)
	{
		odtLog << L"Couldn't find an integer type needed for this variation.\n";
		return TEST_SKIPPED;
	}

	// Since we changed the type and size for one of the bindings we need to reset the offsets
	Repack(m_cBindings, pBind, &cbRowSize);

	cNumRows = 5;  // For now
	TEST_ALLOC(DBCOUNTITEM, rgRowNums, 0, cNumRows * sizeof (DBCOUNTITEM));

	for (i = 0; i < cNumRows ; i++)
	{
		rgRowNums[i] = NextInsertRowNum();
	} 

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	TEST_ALLOC(BYTE, pData, 0, (size_t)(cNumRows * cbRowSize * sizeof (BYTE)));

	// Create the Parameter array.  
	TEST_CHECK (FillInputBindingsForArray (m_pTable, DBACCESSOR_PARAMETERDATA, cbRowSize, m_cBindings, pBind,
		(BYTE **)&pData, cNumRows, rgRowNums, m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);
	
	// Modify pData to Throw an error on the odd rows.
	for (j = 1; j < cNumRows; j+=2)
	{
		// Create an overflow scenario.
		pRowData = (BYTE *)pData + j*cbRowSize;

		*((DBLENGTH *)(((BYTE *)pRowData) + pBind[lSaveIndex].obLength)) = pBind[lSaveIndex].cbMaxLen;
		memcpy((void *)(((BYTE *)pRowData) + pBind[lSaveIndex].obValue), pInt, (size_t)pBind[lSaveIndex].cbMaxLen);
	}

	// Make sure the first row and the correct rows have small enough number not to 
	// create overflows.
	for (j = 0; j < cNumRows; j+=2)
	{
		pRowData = (BYTE *)pData + j*cbRowSize;
		*((DBLENGTH *)(((BYTE *)pRowData) + pBind[lSaveIndex].obLength)) = sizeof (BYTE);
		memcpy((void *)(((BYTE *)pRowData) + pBind[lSaveIndex].obValue), &cTinyInt25, sizeof(BYTE));
	}

	// We Will have to create a new accessor.
	TEST_CHECK (m_pCmdIAccessor->CreateAccessor ( DBACCESSOR_PARAMETERDATA, m_cBindings, pBind, 
		cbRowSize, &hLocalAccessor, NULL), S_OK);

	m_DbParamsAll.cParamSets = cNumRows;
	m_DbParamsAll.hAccessor = hLocalAccessor;
	m_DbParamsAll.pData = pData;

	// Now execute ICommand.
	hr = m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL);

	if (FAILED(hr))
		// Since the rows weren't inserted, decrement next insert row number incremented above.
		m_cInsertRowNum -= cNumRows;

	TEST_CHECK(hr, DB_E_ERRORSOCCURRED);
	
	// Verify If we have inserted the row properly.
	fResult = TRUE;
	for (j=0; j < cNumRows; j++)
	{
		for (i=0; i < m_cBindings; i++)
		{
			// The status will be DBSTATUS_E_DATAOVERFLOW for the rows & cols we set above, except providers 
			// are allowed to set DBSTATUS_E_UNAVAILABLE for all except the one that failed.
			DBSTATUS ulExpectedStatus;
			DBSTATUS ulStatus = *((DBSTATUS *)(((BYTE *)pData + j*cbRowSize) + pBind[i].obStatus));
			BOOL fComp = TRUE;

			if ((LONG)i == lSaveIndex && (j % 2))
			{
				// This is one of the parameters we set to overflow
				ulExpectedStatus = DBSTATUS_E_DATAOVERFLOW;

				// The first bad parameter should always have the correct status but the others
				// might be DBSTATUS_E_UNAVAILABLE
				if (!iBadParam || (ulStatus != ulExpectedStatus && ulStatus != DBSTATUS_E_UNAVAILABLE))
					fComp = COMPARE(ulStatus,ulExpectedStatus);

				iBadParam++;
			}
			else
			{
				// This parameter had correct data and should not overflow, but also might be
				// UNAVAILABLE
				ulExpectedStatus = DBSTATUS_S_OK;

				if (ulStatus != ulExpectedStatus && ulStatus != DBSTATUS_E_UNAVAILABLE)
					fComp = COMPARE(ulStatus,ulExpectedStatus);
			}

			if (!fComp)
			{
				odtLog << "( " << i+1 << " )th Parameter in Row ( " << j+1 << " ) has bad status ( " << 
					ulStatus << " )\n";
				fResult=FALSE;
			}
		}

	}

CLEANUP:


	if(hLocalAccessor != DB_NULL_HACCESSOR)
		m_pCmdIAccessor->ReleaseAccessor (hLocalAccessor, NULL);

	PROVIDER_FREE(pBind);
	PROVIDER_FREE(rgRowNums);
	PROVIDER_FREE(pData);

	return (fResult) ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Having a table such that Variable length columns make up first and last columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_38()
{

	CList				<WCHAR *, WCHAR *> DBTypeList;
	CList				<WCHAR *, WCHAR *> DBVarLengthList;
	POSITION			ListPosition, HeadPosition;
	DBORDINAL			cSaveTableCols = 0;
	DBORDINAL			cColumns = 0; 
	ULONG				count;
	CTable *			pSaveTable= NULL;
	BOOL				fResult = FALSE;
	LONG i;
	WCHAR				 *pwszSqlStatement = NULL;
	ULONG				cDbParamBindInfo=0;
	DB_UPARAMS			*rgParamOrdinals = NULL;
	DB_LORDINAL			*rgUpdateableCols = NULL;
	ULONG				cUpdateableCols = 0;
	CCol				TempCol;
	void				*pData = NULL;
	DBROWCOUNT			cRowsAffected = 0;
	DBPARAMS			DbParamsAll;	
	HACCESSOR			hLocalAccessor = DB_NULL_HACCESSOR;
	DBBINDING			*rgLocalBindings = NULL;
	DBCOUNTITEM			cBindings = 0;
	DBLENGTH			cbRowSize = 0;
	ULONG				cColsOut = 0;
	ULONG				cRowsOnTable = 1;
	WCHAR				*pwszSqlSelectAllFromTbl = NULL;
	WCHAR				*pwszInsertAllWithParams = NULL;
	ULONG				ulRowNum = 0;		
	LONG				cNumRows = 5;
	DBCOUNTITEM			*rgRowNums=NULL;
	CCol				NewCol;
	WCHAR **			ppwszProviderTypeNames=NULL;
	DBPARAMBINDINFO *	pParamBindInfo = NULL;
	ParamStruct *		pParamStruct = NULL;
	WCHAR *				pStringsBuffer = NULL;

	if (!g_bMultipleParamSets)
	{
		cNumRows = 1;
		odtLog << L"Multiple parameter sets are not supported; using 1 parameter set \n";
	}
	else
		odtLog << L"Using " << cNumRows << L" parameter sets.\n";

	TEST_ALLOC(DBCOUNTITEM, rgRowNums, 0, cNumRows * sizeof (DBCOUNTITEM));

	for (i = 0; i < cNumRows ; i++)
	{
		rgRowNums[i] = cRowsOnTable+i+1;
	}

	// Creates a column list from the Ctable
	cColumns = m_pTable->CountColumnsOnTable();

	// Allocate space for array of provider type names and set them to NULL
	TEST_ALLOC(WCHAR *, ppwszProviderTypeNames, 0, (size_t)cColumns * sizeof(WCHAR *));

	// Loop thru column types
	for( count=1; count <= cColumns; count++)
	{
		m_pTable->GetColInfo(count, NewCol);

		// Allocate a buffer and copy the type name string
		if (!(ppwszProviderTypeNames[count-1]=wcsDuplicate(NewCol.GetProviderTypeName())))
			goto CLEANUP;

		// We want variable length non-BLOB columns.  Next variation uses BLOB cols.
		if (!IsFixedLength(NewCol.GetProviderType()) && !NewCol.GetIsLong())
				DBVarLengthList.AddTail(ppwszProviderTypeNames[count-1]);
		else
				DBTypeList.AddTail(ppwszProviderTypeNames[count-1]);
			
	}

	// Now from the VarList append some in beginint and Add some at the end.
	ListPosition = DBVarLengthList.GetHeadPosition();

	// Add this element to the head of types list.
	if (ListPosition)
	{
		HeadPosition = DBTypeList.GetHeadPosition();

		DBTypeList.InsertBefore(HeadPosition, DBVarLengthList.GetNext(ListPosition));
		// Rest add at the tail.
		while (ListPosition != NULL)
		{
			DBTypeList.AddTail(DBVarLengthList.GetNext(ListPosition));
		}
	}
	// We have constructed the list.  Now create the table.
	pSaveTable = 	new CTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
			(LPWSTR)gwszModuleName, NONULLS);

	if (!pSaveTable)
		goto CLEANUP;

	// Now Create the table.  Some providers (Kagera) require an index and
	// can't create an index on a 
	if(!CHECK(pSaveTable->CreateTable(DBTypeList, cRowsOnTable), S_OK))
		goto CLEANUP;

	//Replace text in CommandObject With select statment with no parameters.
	TEST_CHECK(m_hr = pSaveTable->CreateSQLStmt(SELECT_UPDATEABLE, NULL, &pwszSqlSelectAllFromTbl, NULL, NULL ), S_OK);
	
	//Create a Update command with parameters
	// Set command text
	TEST_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , pwszSqlSelectAllFromTbl), S_OK);
	
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	
	// Create the accessor and get the binding information.
	TEST_CHECK(GetAccessorAndBindings(m_pCmdIAccessor, DBACCESSOR_PARAMETERDATA,
				&hLocalAccessor, &rgLocalBindings, &cBindings, &cbRowSize,			
				DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
				UPDATEABLE_COLS_BOUND, FORWARD, NO_COLS_BY_REF, 
				NULL, NULL,
				&pStringsBuffer, DBTYPE_EMPTY, cUpdateableCols, rgUpdateableCols ,
				(DBORDINAL *)rgParamOrdinals ,NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, BLOB_LONG ), S_OK);

	cSaveTableCols = pSaveTable->CountColumnsOnTable();

	TEST_ALLOC(DB_UPARAMS, rgParamOrdinals, 0, (size_t)(cSaveTableCols * sizeof(DB_UPARAMS)));
	TEST_ALLOC(DB_LORDINAL, rgUpdateableCols, 0, (size_t)(cSaveTableCols * sizeof(DB_LORDINAL)));
	TEST_ALLOC(DBPARAMBINDINFO, pParamBindInfo, 0, (size_t)(cSaveTableCols * sizeof(DBPARAMBINDINFO)));
	TEST_ALLOC(ParamStruct, pParamStruct, 0, (size_t)(cSaveTableCols * sizeof(ParamStruct)));

	//We'll use this count as the index to the array as we build it
	cDbParamBindInfo = 0;

	for (i = 1; i <= (LONG)cSaveTableCols; i++)
	{
		TEST_CHECK(pSaveTable->GetColInfo(i, TempCol), S_OK);

		//Record the column number in the array
		//if it is 
		if (TempCol.GetUpdateable() )
		{
			// Build Information for SetParameterInfo.
			AddParam(cDbParamBindInfo, i, DBPARAMIO_INPUT, NULL, FALSE, NULL, NULL, pParamBindInfo,
				pParamStruct, pSaveTable);
			rgUpdateableCols[cUpdateableCols++] = TempCol.GetColNum();  // Column number which is the next Updateable col.
			rgParamOrdinals[cDbParamBindInfo] = cDbParamBindInfo + 1;  // Parameter ordinal number.
			cDbParamBindInfo++;
		
		}
		
	}

	
	// Lets restore the text object to insert command.
	TEST_CHECK(pSaveTable->ExecuteCommand(INSERT_ALLWITHPARAMS, IID_IUnknown,
			NULL, &pwszInsertAllWithParams, NULL, NULL, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);
	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , pwszInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	TEST_CHECK(SetParameterInfoIfNeeded(cDbParamBindInfo, rgParamOrdinals, pParamBindInfo), S_OK);
	
	TEST_ALLOC(BYTE, pData, 0, (size_t)(cNumRows * cbRowSize * sizeof (BYTE)));

	// Now call execute.
	// Create the Parameter array.
	TEST_CHECK (FillInputBindingsForArray (pSaveTable, DBACCESSOR_PARAMETERDATA, cbRowSize, cBindings, rgLocalBindings,
		(BYTE **)&pData, cNumRows, rgRowNums, cUpdateableCols, rgUpdateableCols), S_OK);

	DbParamsAll.cParamSets = cNumRows;
	DbParamsAll.hAccessor = hLocalAccessor;
	DbParamsAll.pData = pData;
	
	// Now execute ICommand.
	TEST_CHECK(m_pICommandText->Execute(NULL, IID_NULL,
			&DbParamsAll, &cRowsAffected, NULL), S_OK);

	TEST_COMPARE (cRowsAffected, cNumRows);

	fResult = TRUE;

CLEANUP:

	// Free the provider type name array
	for (count=0; count < cColumns; count++)
		PROVIDER_FREE(ppwszProviderTypeNames[count]);
	PROVIDER_FREE(ppwszProviderTypeNames);
	
	PROVIDER_FREE(rgRowNums);
	PROVIDER_FREE(pData);

	PROVIDER_FREE(rgParamOrdinals );
	PROVIDER_FREE(pParamStruct );

	PROVIDER_FREE(pwszSqlSelectAllFromTbl) ;
	PROVIDER_FREE(pwszInsertAllWithParams) ;
	PROVIDER_FREE(rgUpdateableCols) ;
	PROVIDER_FREE(rgLocalBindings) ;
	PROVIDER_FREE(pStringsBuffer);

	DBTypeList.RemoveAll();
	DBVarLengthList.RemoveAll();
	if (pSaveTable)
	{
		pSaveTable->DropTable();
		delete pSaveTable;
	}

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Having BLOB as first and last columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_39()
{

	CList				<WCHAR *, WCHAR *> DBTypeList;
	CList				<WCHAR *, WCHAR *> DBBlobList;
	POSITION			ListPosition, HeadPosition;
	DBORDINAL			cColumns = 0;
	ULONG				count;
	CTable *			pSaveTable= NULL;
	BOOL				fResult = FALSE;
	LONG i;
	WCHAR				 *pwszSqlStatement = NULL;
	ULONG				cDbParamBindInfo=0;
	DB_UPARAMS			*rgParamOrdinals = NULL;
	DB_LORDINAL			*rgUpdateableCols = NULL;
	ULONG				cUpdateableCols = 0;
	CCol				TempCol;
	void				*pData = NULL;
	DBROWCOUNT			cRowsAffected = 0;
	IRowset				*pRowset = NULL;
	DBPARAMS			DbParamsAll;	
	HACCESSOR			hLocalAccessor = DB_NULL_HACCESSOR;
	DBBINDING			*rgLocalBindings = NULL;
	DBCOUNTITEM			cBindings = 0;
	DBLENGTH			cbRowSize = 0;
	ULONG				cColsOut = 0;
	ULONG				cRowsOnTable=1;
	WCHAR				*pwszSqlSelectAllFromTbl = NULL;
	WCHAR				*pwszInsertAllWithParams = NULL;
	ULONG				ulRowNum = 0;		
	LONG				cNumRows = 5;
	DBCOUNTITEM			*rgRowNums=NULL;
	CCol				NewCol;
	WCHAR **			ppwszProviderTypeNames=NULL;
	DBPARAMBINDINFO *	pParamBindInfo = NULL;
	ParamStruct *		pParamStruct = NULL;
	DBORDINAL			cSaveTableCols;
	IRowset *			pIRowset = NULL;
	DBCOUNTITEM			cRowsExpected = 1;

	if (!g_bMultipleParamSets)
	{
		cNumRows = 1;
		odtLog << L"Multiple parameter sets are not supported; using 1 parameter set \n";
	}
	else
		odtLog << L"Using " << cNumRows << L" parameter sets.\n";

	TEST_ALLOC(DBCOUNTITEM, rgRowNums, 0, cNumRows * sizeof (DBCOUNTITEM));
	for (i = 0; i < cNumRows ; i++)
		rgRowNums[i] = cRowsOnTable+i+1;

	// Creates a column list from the Ctable
	cColumns = m_pTable->CountColumnsOnTable();

	// Allocate space for array of provider type names
	if (!(ppwszProviderTypeNames = (WCHAR **)PROVIDER_ALLOC(cColumns * sizeof(WCHAR *))))
		goto CLEANUP;

	// Initialize all the pointers to NULL
	memset(ppwszProviderTypeNames, 0, (size_t)(cColumns * sizeof(WCHAR *)));

	// Loop thru column types
	for( count=1; count <= cColumns; count++)
	{
		m_pTable->GetColInfo(count, NewCol);

		// Allocate a buffer and copy the type name string
		if (!(ppwszProviderTypeNames[count-1]=wcsDuplicate(NewCol.GetProviderTypeName())))
			goto CLEANUP;

		if (NewCol.GetIsLong())
			DBBlobList.AddTail(ppwszProviderTypeNames[count-1]);
		else
			DBTypeList.AddTail(ppwszProviderTypeNames[count-1]);
			
	}

	// Now from the VarList append some in begining and Add some at the end.
	ListPosition = DBBlobList.GetHeadPosition();

	if (ListPosition)
	{
		HeadPosition = DBTypeList.GetHeadPosition();

		DBTypeList.InsertBefore(HeadPosition, DBBlobList.GetNext(ListPosition));
		
		// Rest add at the tail.
		while (ListPosition != NULL)
		{
			DBTypeList.AddTail(DBBlobList.GetNext(ListPosition));
		}
	}
	// We have constructed the list.  Now create the table.

	pSaveTable = 	new CTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
			(LPWSTR)gwszModuleName, NONULLS);
	if (!pSaveTable)
		goto CLEANUP;

	// Create a table with one row.  Since some providers require indexes
	// and can't have an index on a BLOB col we make the index col 2.
	if(!CHECK(pSaveTable->CreateTable(DBTypeList, cRowsOnTable, 2), S_OK))
	{
		// Free memory in the list
		DBTypeList.RemoveAll();
		goto CLEANUP;
	}


	cSaveTableCols = pSaveTable->CountColumnsOnTable();
	
	TEST_ALLOC(DB_UPARAMS, rgParamOrdinals, 0, (size_t)(cSaveTableCols * sizeof(DB_UPARAMS)));
	TEST_ALLOC(DB_LORDINAL, rgUpdateableCols, 0, (size_t)(cSaveTableCols * sizeof(DB_LORDINAL)));
	TEST_ALLOC(DBPARAMBINDINFO, pParamBindInfo, 0, (size_t)(cSaveTableCols * sizeof(DBPARAMBINDINFO)));
	TEST_ALLOC(ParamStruct, pParamStruct, 0, (size_t)(cSaveTableCols * sizeof(ParamStruct)));

	//We'll use this count as the index to the array as we build it
	cDbParamBindInfo = 0;

	for (i = 1; i <= (LONG)pSaveTable->CountColumnsOnTable(); i++)
	{
		TEST_CHECK(pSaveTable->GetColInfo(i, TempCol), S_OK);

		//Record the column number in the array if it is 
		if (TempCol.GetUpdateable() )
		{
			// Build Information for SetParameterInfo.
			AddParam(cDbParamBindInfo, i, DBPARAMIO_INPUT, NULL, FALSE, NULL, NULL, pParamBindInfo, pParamStruct, pSaveTable);
			rgUpdateableCols[cUpdateableCols++] = TempCol.GetColNum();  // Column number which is the next Updateable col.
			rgParamOrdinals[cDbParamBindInfo] = cDbParamBindInfo + 1;  // Parameter ordinal number.
			cDbParamBindInfo++;
		
		}
		
	}

	//Replace text in CommandObject With select statment with no parameters.
	TEST_CHECK(m_hr = pSaveTable->CreateSQLStmt(SELECT_UPDATEABLE, NULL, &pwszSqlSelectAllFromTbl, NULL, NULL ), S_OK);
	
	//Create a Update command with parameters
	// Set command text
	TEST_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , pwszSqlSelectAllFromTbl), S_OK);
	
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	TEST_CHECK(SetParameterInfoIfNeeded(cDbParamBindInfo, rgParamOrdinals, pParamBindInfo), S_OK);
	
	// Create the accessor and get the binding information.
	TEST_CHECK(GetAccessorAndBindings(m_pCmdIAccessor, DBACCESSOR_PARAMETERDATA,
				&hLocalAccessor, &rgLocalBindings, &cBindings, &cbRowSize,			
				DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
				UPDATEABLE_COLS_BOUND, FORWARD, NO_COLS_BY_REF, 
				NULL, NULL,
				NULL, DBTYPE_EMPTY, cUpdateableCols, rgUpdateableCols ,
				(DBORDINAL *)rgParamOrdinals ,NO_COLS_OWNED_BY_PROV, DBPARAMIO_INPUT, BLOB_LONG ), S_OK);

	// Lets restore the text object to insert command.
	TEST_CHECK(pSaveTable->ExecuteCommand(INSERT_ALLWITHPARAMS, IID_IUnknown,
			NULL, &pwszInsertAllWithParams, NULL, NULL, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);
	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , pwszInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	pData = (BYTE *)m_pIMalloc->Alloc (cNumRows * cbRowSize * sizeof (BYTE));
	if (!pData) goto CLEANUP;
	// Now call execute.
	// Create the Parameter array.
	TEST_CHECK (FillInputBindingsForArray (pSaveTable, DBACCESSOR_PARAMETERDATA, cbRowSize, cBindings, rgLocalBindings,
		(BYTE **)&pData, cNumRows, rgRowNums, cUpdateableCols, rgUpdateableCols), S_OK);

	DbParamsAll.cParamSets = cNumRows;
	DbParamsAll.hAccessor = hLocalAccessor;
	DbParamsAll.pData = pData;
	
	// Now execute ICommand.
	TEST_CHECK(m_pICommandText->Execute(NULL, IID_NULL,
			&DbParamsAll, &cRowsAffected, NULL), S_OK);

	TEST_COMPARE (cRowsAffected, cNumRows);

	// Verify If we have inserted the row properly.
	for (i=0; i < cNumRows; i++)
	{
		// Verify If we have inserted the row properly.
		TEST_COMPARE(cRow.FindRow(rgRowNums[i], pSaveTable, NULL, &pIRowset, NULL,
			0, NULL, FALSE), TRUE);

		// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
		TEST_COMPARE(VerifyObj(m_iidExec, pIRowset, rgRowNums[i], 0, NULL,
			FALSE, FALSE, pSaveTable, &cRowsExpected), S_OK);

		SAFE_RELEASE(pIRowset);
	}

	fResult = TRUE;

CLEANUP:

	SAFE_RELEASE(pIRowset);

	// Free the provider type name array
	for (count=0; count < cColumns; count++)
		PROVIDER_FREE(ppwszProviderTypeNames[count]);
	PROVIDER_FREE(ppwszProviderTypeNames);

 	ReleaseInputBindingsForArray(cNumRows, cbRowSize, cBindings, rgLocalBindings, (BYTE *)pData);

	if (pRowset) ReleaseRowsetPtr(&pRowset);

	FREE_DATA (rgRowNums);
	FREE_DATA (rgParamOrdinals );
	FREE_DATA (pwszSqlSelectAllFromTbl);
	FREE_DATA (pwszInsertAllWithParams);
	FREE_DATA (rgUpdateableCols);
	FREE_DATA (rgLocalBindings);

	DBTypeList.RemoveAll();
	DBBlobList.RemoveAll();
	if (pSaveTable)
	{
		pSaveTable->DropTable();
		delete pSaveTable;
	}

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc 5 parameter sets with different size of Variable length parameters.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_40()
{
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	IRowset * pRowset = NULL;
	TESTRESULT	testresult = TEST_FAIL;
	LONG cNumRows = 5;
	LONG i;
	DBCOUNTITEM *rgRowNums=NULL;
	void *pData = NULL;
	LONG VarColumn=0;
	DBLENGTH * pLength = NULL;
	ULONG ulNum[5] = {1, 7, 4, 8, 3};

	if (!g_bMultipleParamSets)
	{
		cNumRows = 1;
		odtLog << L"Multiple parameter sets are not supported; using 1 parameter set \n";
	}
	else
		odtLog << L"Using " << cNumRows << L" parameter sets.\n";

	TEST_ALLOC(DBCOUNTITEM, rgRowNums, 0, cNumRows * sizeof (DBCOUNTITEM));
	for (i = 0; i < cNumRows ; i++)
		rgRowNums[i] = NextInsertRowNum();

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	TEST_CHECK(SetParameterInfoIfNeeded(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), S_OK);

	TEST_ALLOC(BYTE, pData, 0, (size_t)(cNumRows * m_cbRowSize * sizeof (BYTE)));
	
	// Create the Parameter array.
	TEST_CHECK (FillInputBindingsForArray (m_pTable, DBACCESSOR_PARAMETERDATA, m_cbRowSize, m_cBindings, m_rgBindings,
		(BYTE **)&pData, cNumRows, rgRowNums, m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);
	
	m_DbParamsAll.cParamSets = cNumRows;
	m_DbParamsAll.hAccessor = m_hAccessor;
	m_DbParamsAll.pData = pData;

	// Now Lets Modify length and Value buffers of pData.
	// For Testing purpose.  1st Buffer smallest(1/8th of Orig).  2nd Buffer biggest(7/8th).  
	// 3rd buffer(4/8), 4th buffer ( 8/8) 5th buffer (3/8)

	// First get the index of a Variable length column.
	VarColumn = -1;
	for (i=0; i < (LONG)m_cBindings && VarColumn == -1; i++)
	{
		if (!IsFixedLength(m_rgBindings[i].wType))
				VarColumn = i;	// Just need the index.
	}

	if (VarColumn == -1)
	{
		// Couldn't get a variable column.
		odtLog << "No variable size column in the table\n";
		testresult = TEST_SKIPPED;
		goto CLEANUP;
	}

	// Now lets change pData.
	// First Array.

	// Change How much of Data you are putting in.
	for (i=0; i < cNumRows; i++)
	{
		pLength = ((DBLENGTH *)((BYTE *)pData + (i*m_cbRowSize) + m_rgBindings[VarColumn].obLength));
		*pLength = *pLength*ulNum[i]/8;
		// If it's a unicode string we must put in an even number.
		if (m_rgBindings[VarColumn].wType == DBTYPE_WSTR &&	*pLength % 2)
			(*pLength)++;
	}

	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	TEST_COMPARE (cRowsAffected, cNumRows);

	// Some how verify How much data actually got put in.
	// @todo.

	// At this point our table contains perverted data.  We must either delete these
	// extra rows or fix them up to match expected pattern, or later verifications will
	// fail.

	// Due to the difficulty in deleting only the rows inserted above we will delete all
	// rows and reinsert
	m_pTable->Delete();
	m_pTable->Insert(1, PRIMARY, TRUE, NULL, FALSE, TOTAL_NUMBER_OF_ROWS);	// 0 means next row
	m_cInsertRowNum = TOTAL_NUMBER_OF_ROWS+1;

	testresult = TEST_PASS;


CLEANUP:

	PROVIDER_FREE(pData);	
	PROVIDER_FREE(rgRowNums);

	return testresult;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc 5 parameters sets followed by 1 parameter set.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_41()
{

	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	ULONG cNumRows, i;
	DBCOUNTITEM *rgRowNums=NULL;
	BYTE *pData = NULL;
	
	if (!g_bMultipleParamSets)
	{
		odtLog << "Multiple parameter sets are not supported \n";
		return TEST_SKIPPED;
	}

	cNumRows = 5;  // For now
	TEST_ALLOC(DBCOUNTITEM, rgRowNums, 0, cNumRows * sizeof (DBCOUNTITEM));
	for (i = 0; i < cNumRows ; i++)
		rgRowNums[i] = NextInsertRowNum();

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	TEST_CHECK(SetParameterInfoIfNeeded(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), S_OK);

	TEST_ALLOC(BYTE, pData, 0, (size_t)(cNumRows * m_cbRowSize * sizeof (BYTE)));

	// Create the Parameter array.
	TEST_CHECK (FillInputBindingsForArray (m_pTable, DBACCESSOR_PARAMETERDATA, m_cbRowSize, m_cBindings, m_rgBindings,
		(BYTE **)&pData, cNumRows, rgRowNums, m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);
	
	m_DbParamsAll.cParamSets = cNumRows;
	m_DbParamsAll.hAccessor = m_hAccessor;
	m_DbParamsAll.pData = pData;

	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Verify If we have inserted the row properly.
	for (i=0; i < cNumRows; i++)
	{
		TEST_COMPARE(cRow.FindRow(rgRowNums[i], m_pTable), TRUE);
		// Tell the CTable object we have another row
		m_pTable->AddRow();
	}

	cNumRows = 1;  // For now
	for (i = 0; i < cNumRows ; i++)
		rgRowNums[i] = NextInsertRowNum();

	// Create the Parameter array.
	TEST_CHECK (FillInputBindingsForArray (m_pTable, DBACCESSOR_PARAMETERDATA, m_cbRowSize, m_cBindings, m_rgBindings,
		(BYTE **)&pData, cNumRows, rgRowNums, m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);
	
	m_DbParamsAll.cParamSets = cNumRows;
	m_DbParamsAll.hAccessor = m_hAccessor;
	m_DbParamsAll.pData = pData;

	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Verify If we have inserted the row properly.
	for (i=0; i < cNumRows; i++)
	{
		TEST_COMPARE(cRow.FindRow(rgRowNums[i], m_pTable), TRUE);
		// Tell the CTable object we have another row
		m_pTable->AddRow();
	}

	fResult = TRUE;

CLEANUP:

	PROVIDER_FREE(rgRowNums);
	PROVIDER_FREE(pData);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc BYREF accessor with PROVIDER_OWNED memory should fail.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_42()
{
	DBCOUNTITEM cBindings=1;
	DBBINDING DbBindings;
	HACCESSOR hLocalAccessor = DB_NULL_HACCESSOR;


	DbBindings.iOrdinal = 1;
	DbBindings.obValue = offsetof(DATA, bValue) + sizeof (DATA);
	DbBindings.obLength = offsetof(DATA, ulLength) + sizeof (DATA);
	DbBindings.obStatus = offsetof (DATA, sStatus) + sizeof (DATA);
	DbBindings.pTypeInfo = NULL;
	DbBindings.pObject = NULL;
	DbBindings.pBindExt = NULL;
	DbBindings.dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
	DbBindings.dwMemOwner = DBMEMOWNER_PROVIDEROWNED;
	DbBindings.eParamIO = DBPARAMIO_OUTPUT;
	DbBindings.cbMaxLen = sizeof (LONG);
	DbBindings.dwFlags = 0;
	DbBindings.wType = DBTYPE_STR | DBTYPE_BYREF;
	DbBindings.bPrecision = 0;
	DbBindings.bScale = 0;

	// We Will have to create a new accessor.
	if (!CHECK (m_pCmdIAccessor->CreateAccessor ( DBACCESSOR_PARAMETERDATA, cBindings, &DbBindings, 
		m_cbRowSize, &hLocalAccessor, NULL), DB_E_ERRORSOCCURRED))
	{
		return TEST_FAIL;
	}
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc RestartPosition should either return S_OK or DB_E_CANNOTRESTART
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_43()
{
	BOOL fResult = TRUE;
	IRowset *	pIRowset = NULL;
	BYTE *		pData = NULL;
	DBBINDING *	rgBindings = NULL;
	HACCESSOR 	hExecAccessor=DB_NULL_HACCESSOR;
	ULONG 		cSearchableCols = 0;
	ULONG		i = 0;
	DBLENGTH	cbRowSize = 0;
	DBCOUNTITEM	cBindings = 0;
	DB_LORDINAL *	rgSearchableCols = NULL;
	DBORDINAL	rgParamOrds[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25};
	DBROWCOUNT	cRowsAffected = 0;
	DBPARAMS 	Param;
	CCol  		TempCol;


	if (m_eTestCase == TC_Row)
	{
		odtLog << L"RestartPosition not supported for row objects.\n";
		return TEST_SKIPPED;
	}

	// SetParameter Info information should be deleted.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	//Get memory to hold array of all col numbers.  NOTE:  This 
	//is the max possible, we won't necessarily use them all.
	rgSearchableCols = (DB_LORDINAL *)m_pIMalloc->Alloc(m_pTable->CountColumnsOnTable() * sizeof(DB_LORDINAL));
	if (!rgSearchableCols)
		return FALSE;
				
	//We'll use this count as the index to the array as we build it
	cSearchableCols = 0;

	for (i=1; i<=m_pTable->CountColumnsOnTable(); i++)
	{
		CHECK(m_pTable->GetColInfo(i, TempCol), S_OK);

		//Record the column number in the array, if it is searchable
		if (TempCol.GetIsLong())
			continue;

		if ( (TempCol.GetSearchable() != DB_UNSEARCHABLE) && TempCol.GetUpdateable() )
		{
			rgSearchableCols[cSearchableCols] = TempCol.GetColNum();				
			cSearchableCols++;
		}
															
	}

	//Just set the command so we can do IColumnsInfo to generate accessor
	if (!CHECK(m_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset, NULL, NULL, NULL, NULL, 
		EXECUTE_NEVER,  0, NULL, NULL, NULL, &m_pICommand), S_OK))
		goto CLEANUP;
	
	
	//  Create the accessor.
	if (!CHECK(GetAccessorAndBindings(m_pCmdIAccessor, DBACCESSOR_PARAMETERDATA,
		&hExecAccessor, &rgBindings, &cBindings, &cbRowSize,			
  		DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH, USE_COLS_TO_BIND_ARRAY, 
		FORWARD, NO_COLS_BY_REF, NULL, NULL,  NULL, DBTYPE_EMPTY, 
		cSearchableCols, rgSearchableCols, rgParamOrds, NO_COLS_OWNED_BY_PROV,
		DBPARAMIO_INPUT, BLOB_LONG), S_OK))
		goto CLEANUP;
	

	// Set command text for a select query using prameters.
	if (FAILED(m_hr = m_pTable->ExecuteCommand(SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE, IID_IUnknown,
		NULL,NULL,NULL, NULL, EXECUTE_NEVER, 0, NULL,  NULL, NULL, &m_pICommand)))
		goto CLEANUP;

	//Alloc enough memory to hold a row of parameter data
	pData = (BYTE *)m_pIMalloc->Alloc(cbRowSize);	

	if (!pData)
	{
		PRVTRACE (wszMemoryAllocationError);
		goto CLEANUP;
	}
	memset(pData, 0, (size_t)cbRowSize);
	
	//Set up parameter input values for selecting row 1
	// m_ulFixedRowNum contains the row number guarenteed to be in the table.
	TEST_CHECK(FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cBindings,
		rgBindings, &pData, m_ulFixedRowNum, cSearchableCols, rgSearchableCols, PRIMARY), S_OK);
 
	Param.cParamSets = 1;
	Param.hAccessor = hExecAccessor;
	Param.pData = pData;

	// Call SetParameterInfo if the provider can't derive.
	// Need to use the proper paraminfo here.
//	TEST_CHECK(SetParameterInfoIfNeeded(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), S_OK);
	
	// Execute Command statement.
	if (!CHECK(m_hr = m_pICommand->Execute(NULL, IID_IRowset, &Param, 
		&cRowsAffected, (IUnknown **)&pIRowset),S_OK))	
		goto CLEANUP;
	
	// Expecting a rowset (with atleast one row)
	if (!pIRowset)
		goto CLEANUP;
	
	m_hr = pIRowset->RestartPosition(NULL);

	// Should return S_OK or DB_E_CANNOTRESTART
	if (m_hr != S_OK)
	{
		// If we did not succeed in doing restart position it should give this error.
		TEST_CHECK (m_hr, DB_E_CANNOTRESTART);
	}

	fResult = TRUE;
CLEANUP:

	//  Release pData before we set it again.
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	FREE_DATA (pData);

	if ((hExecAccessor != DB_NULL_HACCESSOR ) && m_pCmdIAccessor)
	{
		CHECK(m_pCmdIAccessor->ReleaseAccessor(hExecAccessor, NULL), S_OK);
		hExecAccessor = DB_NULL_HACCESSOR;
	}

	// Release Bindings.
	FREE_DATA (rgBindings);
	FREE_DATA (rgSearchableCols);
	RELEASE (pIRowset);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Named Parameters: Sproc parm count != bound parm count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_44()
{
	// The stored proc contains one parameter with a default value, so the count of parameters 
	// in the proc don't match those bound. We bind to a parameter by name after the first one such that
	// if the name isn't used to resolve the parameter, then a conversion error will result.  If no
	// bindings can be made that would result in a conversion error, then the test is inconclusive unless
	// we validate the rowset returned (that the parm values were resolved correctly).  This might happen
	// if the provider only supports char type, for instance.
	// 
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 2;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	DBPARAMINFO * pParamInfo = NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	WCHAR * pNamesBuffer = NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSet = DB_S_TYPEINFOOVERRIDDEN;
	enum TOKEN_ENUM eProcType = T_EXEC_PROC_SELECT_OUT_DFLT;
	CHAR * pszStmt = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output params not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		eProcType,				// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	if (!fCanDerive)
		hrSet = S_OK;

	// Set parameter info
	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), hrSet);

	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE, m_pICommand, S_OK), S_OK);


CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pParamInfo);
	PROVIDER_FREE(pNamesBuffer);
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Named Parameters: Sproc parm order != binding order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_45()
{
	// The parameters are bound in the reverse order of the parameters found in the
	// sql stmt, and are resolved via parameter name only.  We need to validate the rowset
	// returned in case a provider only suports char type and all parameter types match the
	// bindings even when reversed.

	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 2;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	DBPARAMINFO * pParamInfo = NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	WCHAR * pNamesBuffer = NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSet = DB_S_TYPEINFOOVERRIDDEN;
	enum TOKEN_ENUM eProcType = T_EXEC_PROC_SELECT_OUT;
	enum ROWSET_ENUM eRowset = ROWSET_NONE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		eProcType = T_EXEC_PROC_SELECT_IN;
		eRowset = ROWSET_ALWAYS;
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		eProcType,				// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	if (!fCanDerive)
		hrSet = S_OK;

	// Set parameter info
	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), hrSet);

	ReverseArray(pBINDING, cParams, sizeof(DBBINDING));
	ReverseArray(pParamAll, cParams, sizeof(ParamStruct));

	// Create param accessor and call Execute expecting S_OK
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, eRowset, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE, m_pICommand, S_OK), S_OK);


CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pParamInfo);
	PROVIDER_FREE(pNamesBuffer);
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Named Parameters: Use all parameter names
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_46()
{
	// Tests with all named parameters.  Since this uses the column name as the parameter name, when the
	// column name contains international characters the parameter will also, but must be run on an
	// international OS (code page).  Parameter bindings match parameter markers in Sql stmt.


//	return TestStoredProcBindings (INPUT_OUTPUT, VALID_IN_OUT_DATA, LENGTH_VALUE_STATUS, REGULAR_PARAMNAMES, TRUE);
	odtLog << L"This is already tested in other variations now, for example, TCGetParameterInfo_Rowset::Variation_13 and 14.\n";


	return TEST_SKIPPED;

}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Named Parameters: Retrieve names and use without setting
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_47()
{
	odtLog << L"Not yet implemented.\n";
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Named Parameters: Named return parameter from sproc
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandExecute_Rowset::Variation_48()
{
	odtLog << L"Not yet implemented.\n";
	return TEST_PASS;
}

// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Inline binding with more than 64K and embedded null term
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_49()
{ 
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	DBCOUNTITEM ulRowNum;
	CCol  					ColInfo;
	ULONG					iBind=0;
	ULONG					iCol=0;
	DBBINDING *				pSaveBinding = NULL;
	ULONG *					pulBigBind = NULL;
	ULONG					cBigBind = 0;
	DBLENGTH				ulColumnSize = 0;
	DBLENGTH				cbRowSize = 0;
	BYTE *					pData = NULL;
	HRESULT					hrSetProp = E_FAIL;
	HRESULT					hrIns = E_FAIL;
	
	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on.
	if (SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);
	
	// Make a copy of the bindings so we can increase the size appropriately
	SAFE_ALLOC(pSaveBinding, DBBINDING, m_cBindings * sizeof(DBBINDING));
	memcpy(pSaveBinding, m_rgBindings, (size_t)(m_cBindings * sizeof(DBBINDING)));

	SAFE_ALLOC(pulBigBind, ULONG, m_cBindings * sizeof(ULONG));

	// Make sure we have a column that allows more than 64K data
	for (iCol = 1; iCol <= m_pTable->CountColumnsOnTable(); iCol++)
	{
		// Look up information for the column
		ABORT_CHECK(m_pTable->GetColInfo(iCol, ColInfo), S_OK);

		// We only want the uptable columns
		if (!ColInfo.GetUpdateable())
			continue;
			
		ulColumnSize = ColInfo.GetColumnSize();

		// If the size is more than our boundary value we're happy
		if (ulColumnSize > BOUNDARY_VALUE)
		{
			// Note the column may not set DBCOLUMNFLAGS_ISLONG
			m_rgBindings[iBind].cbMaxLen = min(MAX_LONG_VALUE, ulColumnSize);

			// WCHAR columns must be even
			if ((m_rgBindings[iBind].wType == DBTYPE_WSTR) && (m_rgBindings[iBind].cbMaxLen %2))
				m_rgBindings[iBind].cbMaxLen--;

			pulBigBind[cBigBind++]=iBind;
		}
		
		// Reset the obValue, obLength, obStatus
		if (iBind)
		{
			m_rgBindings[iBind].obValue = m_rgBindings[iBind-1].obValue+m_rgBindings[iBind-1].cbMaxLen
				+sizeof(DBSTATUS)+sizeof(DBLENGTH);
			m_rgBindings[iBind].obLength = m_rgBindings[iBind-1].obLength+m_rgBindings[iBind-1].cbMaxLen
				+sizeof(DBSTATUS)+sizeof(DBLENGTH);
			m_rgBindings[iBind].obStatus = m_rgBindings[iBind-1].obStatus+m_rgBindings[iBind-1].cbMaxLen
				+sizeof(DBSTATUS)+sizeof(DBLENGTH);
		}

		cbRowSize+=sizeof(DBLENGTH)+sizeof(DBSTATUS)+m_rgBindings[iBind].cbMaxLen;
		iBind++;
	}

	if (!cBigBind)
	{
		odtLog << L"No columns with size > " << BOUNDARY_VALUE << " to test with.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	Repack(m_cBindings, m_rgBindings, &cbRowSize);
	
	ReleaseDataForCommand();
	SAFE_ALLOC(m_pData, BYTE, cbRowSize);

	// Set memory to a known value
	memset(m_pData, 'S', (size_t)cbRowSize);

	// Remove any old parameter info
	TESTC_(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set command text 
	TESTC_(m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TESTC_(m_pICommandPrepare->Prepare(1), S_OK);

	//This sets it to one row of data.
	if (!CHECK(FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA,
				m_cBindings, m_rgBindings, &m_pData, ulRowNum = m_pTable->GetNextRowNumber(), m_cParamColMap,
				(DB_LORDINAL *)m_rgParamColMap), S_OK))
	{
		return FALSE;
	}

	m_DbParamsAll.cParamSets = 1;
	m_DbParamsAll.pData = m_pData;

	// Reset the accessor to reflect the new bindings
	TESTC_(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA | DBACCESSOR_ROWDATA,		
		m_cBindings,
		m_rgBindings,
		cbRowSize,
		&m_DbParamsAll.hAccessor,
		NULL), S_OK);

	// Now reset the length binding for the big columns
	// Note the data value filled for the big columns does not fill the space available, but
	// there is some random data there anyway.  We'll use that for validation.  String data will
	// have an embedded null terminator, which is legal.
	for (iBind = 0; iBind < cBigBind; iBind++)
	{
		LENGTH_BINDING(m_rgBindings[pulBigBind[iBind]], m_pData)=m_rgBindings[pulBigBind[iBind]].cbMaxLen;

		// For CHAR and WCHAR we must allow space for NULL terminator.  This is not required for 
		// sending data but for retrieving, otherwise we'll truncate
		if (m_rgBindings[pulBigBind[iBind]].wType == DBTYPE_WSTR)
			LENGTH_BINDING(m_rgBindings[pulBigBind[iBind]], m_pData) -= sizeof(WCHAR);
		if (m_rgBindings[pulBigBind[iBind]].wType == DBTYPE_STR)
			LENGTH_BINDING(m_rgBindings[pulBigBind[iBind]], m_pData) -= sizeof(CHAR);

	}

	// Now execute ICommand.
	TESTC_(hrIns = m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Tell the CTable object we have another row
	m_pTable->AddRow();

	// Verify If we have inserted the row properly.
	TESTC((pData = cRow.GetUpdatableCols(ulRowNum, m_cBindings, m_rgBindings, 
		cbRowSize, m_pICommand)) != NULL);

	// Validate information retrieved
	TESTC(CompareBuffer(pData, m_pData, m_cBindings, m_rgBindings, NULL, TRUE, FALSE, COMPARE_ONLY));

	fResult = TRUE;

CLEANUP:

	// Delete the row we added above because it does not match the expected values and may cause
	// subsequent data validation to fail.
//	if (SUCCEEDED(hrIns))
//		m_pTable->DeleteRows(ulRowNum);


	ReleaseDataForCommand();

	SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, m_DbParamsAll.hAccessor);

	// Put the bindings back
	if (pSaveBinding)
		memcpy(m_rgBindings, pSaveBinding, (size_t)(m_cBindings * sizeof(DBBINDING)));

	SAFE_FREE(pSaveBinding);
	SAFE_FREE(pulBigBind);
	SAFE_FREE(pData);

	// If we set RANDOM REQUIRED above we need to set back OPTIONAL
	if (hrSetProp == S_OK)
		CHECK(SetRowsetPropertyDefault(DBPROP_ACCESSORDER), S_OK);
	
	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Insert with BSTR bindings to strings
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_50()
{ 
	BOOL fResult = TEST_FAIL;
	DBROWCOUNT cRowsAffected = 0;

	IConvertType * pIConvertType = NULL;
	DBBINDING * pBindings = NULL;
	ULONG iBind;
	HACCESSOR hParamAccessor = m_hAccessor;
	BOOL fConvertSTR = FALSE;
	BOOL fConvertWSTR = FALSE;
	HRESULT hrConv = E_FAIL;
	HRESULT	hrSet = DB_S_TYPEINFOOVERRIDDEN;

	// Make sure the provider claims it can support conversion from BSTR -> STR
	// or BSTR -> WSTR.
	TESTC(VerifyInterface(m_pICommand, IID_IConvertType,
				COMMAND_INTERFACE,(IUnknown **)&pIConvertType));

	hrConv = pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_STR, DBCONVERTFLAGS_PARAMETER);

	if (S_OK == hrConv)
		fConvertSTR = TRUE;
	else
		CHECK(hrConv, S_FALSE);

	hrConv = pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR, DBCONVERTFLAGS_PARAMETER);

	if (S_OK == hrConv)
		fConvertWSTR = TRUE;
	else
		CHECK(hrConv, S_FALSE);

	if (!fConvertSTR && !fConvertWSTR)
	{
		odtLog << L"Provider doesn't support conversions from BSTR to string types.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	
	// Remove any old parameter info
	TEST_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Now set back to "default" values for the insert statement we got in the base class init
	if (m_bSetParameterInfo)
		hrSet = S_OK;
	FAIL_CHECK(m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), hrSet);

	// Reset str and wstr bindings to BSTR.
	SAFE_ALLOC(pBindings, DBBINDING, m_cBindings);
	memcpy(pBindings, m_rgBindings, (size_t)(m_cBindings*sizeof(DBBINDING)));
	
	// Release any previous binding memory
	ReleaseDataForCommand();

	// Allocate a block of memory to hold the bindings
	// Note we allocate up front instead of letting FillInputBindings do it.  This is
	// because FillInputBindings will allocate a minimum sized block of mem, while
	// we have extra space in the binding array for the str types.
	SAFE_ALLOC(m_pData, BYTE, m_cbRowSize);

	for (iBind=0; iBind < m_cBindings; iBind++)
	{
		if (m_rgBindings[iBind].wType == DBTYPE_STR && fConvertSTR)
			m_rgBindings[iBind].wType = DBTYPE_BSTR;

		if (m_rgBindings[iBind].wType == DBTYPE_WSTR && fConvertWSTR)
			m_rgBindings[iBind].wType = DBTYPE_BSTR;
	}


	TEST_CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings, m_cbRowSize,
		&m_hAccessor, NULL), S_OK);

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA,
		m_cBindings, m_rgBindings, &m_pData, m_pTable->GetNextRowNumber(), m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);


	m_DbParamsAll.cParamSets = 1;
	m_DbParamsAll.hAccessor = m_hAccessor;
	m_DbParamsAll.pData = m_pData;
		
	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);



	// Verify If we have inserted the row properly.
	TEST_COMPARE(cRow.FindRow(m_pTable->GetNextRowNumber(), m_pTable), TRUE);

	// Tell the CTable object we have another row
	m_pTable->AddRow();



	fResult = TEST_PASS;

CLEANUP:

	ReleaseDataForCommand();
	memcpy(m_rgBindings, pBindings, (size_t)(m_cBindings * sizeof(DBBINDING)));
	SAFE_FREE(pBindings);
	SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, m_hAccessor);
	SAFE_RELEASE(pIConvertType);
	m_hAccessor = hParamAccessor;

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc OpenRowset on a stored proc with params
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_51()
{ 
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	IOpenRowset * pIOpenRowset = NULL;
	IRowset * pIRowset = NULL;
	DBID dbidProc = DB_NULLID;
	HRESULT hr = E_FAIL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Get an OpenRowset interface
	TEST_COMPARE(VerifyInterface(m_pThisTestModule->m_pIUnknown2, 
		IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&pIOpenRowset),TRUE);

	// Create a DBID from the proc name
	dbidProc.eKind = DBKIND_NAME;
	dbidProc.uName.pwszName = pwszProcName;

	// Call OpenRowset on the stored proc
	hr = pIOpenRowset->OpenRowset(NULL, &dbidProc, NULL, m_iidExec, 
		0, NULL, (IUnknown **)&pIRowset);
		
	// Allow DB_E_PARAMNOTOPTIONAL or DB_E_NOTABLE (proc isn't a table).
	TEST2C_(hr, DB_E_NOTABLE, DB_E_PARAMNOTOPTIONAL);

	fResult = TEST_PASS;

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	SAFE_RELEASE(pIOpenRowset);
	
	return fResult ? TEST_PASS : TEST_FAIL;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc DBSTATUS_S_DEFAULT - Default values for param
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_52()
{ 
	/*
	We have to create a new table here because we can't create a default value for a non-updatable column, and
	the only way we know if a column is updatable is to have the table already created.  Therefore we use the
	original table to tell us the updatability.  Notice we create the new table with the same column ordering.
	*/

	ULONG			cDefaultCols =0;
	BOOL			fResult = TEST_FAIL;
	CTable *		pDefaultTable = NULL;
	ULONG			iCol;
	DBORDINAL		cCols;
	DBCOLUMNDESC *	prgColumnDesc = NULL;
	DBROWCOUNT		cRowsAffected = 0;
	ULONG			ulRowNum = TOTAL_NUMBER_OF_ROWS+1;
	ULONG			iBind = 0;
	IRowset *		pIRowset = NULL;
	WCHAR *			pwszSqlInsertAllWithParams = NULL;
	HRESULT			hrSetProp = E_FAIL;
	DB_LORDINAL *	pCols = NULL;
	DBCOUNTITEM		cRowsExpected = 1;

	// Make sure the provider supports default params in statements
	// Oracle doesn't support default params in statements, only PL/SQL.
	// In ODBC, SQL_DEFAULT_PARAM is not supported for statements, per ODBC spec.
	if (!m_fDefaultVal)
	{
		odtLog << L"Provider doesn't support default values for columns.\n";
		return TEST_SKIPPED;
	}

	// Initialize our accessor
	m_DbParamsAll.hAccessor = DB_NULL_HACCESSOR;

	// We have to jump through some hoops to get default values for each column
	// These are necessary until CreateColInfo takes pointers
	CList <WCHAR * ,WCHAR *> ListNativeTemp;
	CList <DBTYPE,DBTYPE> ListDataTypes;

	if (!(pDefaultTable = new CTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
		(LPWSTR)gwszModuleName, NONULLS)))
	{
		PRVTRACE (wszMemoryAllocationError);
		goto CLEANUP;
	}

	// Determine the SQL Support
	pDefaultTable->ProviderSQLSupport();

	// Create the colinfo for the table
	TESTC_(pDefaultTable->CreateColInfo(ListNativeTemp,ListDataTypes,ALLTYPES,FALSE), S_OK);

	cCols = pDefaultTable->CountColumnsOnTable();

	// Make sure the count of columns is the same
	TESTC(cCols == m_pTable->CountColumnsOnTable());

	// Set the unique index flag to match the original table
	pDefaultTable->SetIndexColumn(m_pTable->GetIndexColumn());

	// Now go through the colinfo and update to force a default value for each column
	for (iCol = 1; iCol <= cCols; iCol++)
	{
		// Get colinfo for column in default table for update
		CCol& DefaultCol=pDefaultTable->GetColInfoForUpdate(iCol);
		CCol OrigCol;

		TESTC_(m_pTable->GetColInfo(iCol, OrigCol), S_OK);

		// Make sure the columns appear to match
//		TESTC(!_wcsicmp(DefaultCol.GetColName(), OrigCol.GetColName()));

		// Set the updatability to match the original table
		DefaultCol.SetUpdateable(OrigCol.GetUpdateable());

		// We can only have defaults for updatable cols
		if (!DefaultCol.GetUpdateable())
			continue;

		// Now create the default value and set flag indicating there's a default
		if (pDefaultTable->SetDefaultValue(DefaultCol, ulRowNum))
			cDefaultCols++;

	}

	// Make sure we actually have some default cols
	if (!cDefaultCols)
	{
		odtLog << L"No default columns available.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	TESTC_(pDefaultTable->BuildColumnDescs(&prgColumnDesc), S_OK);
	pDefaultTable->SetColumnDesc(prgColumnDesc, pDefaultTable->CountColumnsOnTable());
	pDefaultTable->SetBuildColumnDesc(FALSE);

	// Now actually create the table containing default values
	TESTC_(pDefaultTable->CreateTable(ulRowNum-1), S_OK);

	// Remove any old parameter info
	TEST_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	TESTC_(pDefaultTable->ExecuteCommand(INSERT_ALLWITHPARAMS, IID_IUnknown, NULL, 
		&pwszSqlInsertAllWithParams, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , pwszSqlInsertAllWithParams), S_OK);

	// Just to be different don't call Prepare

	// This sets it to one row of data.  Note it actually uses the original table
	// to get the data values, but since they match it's OK.
	TESTC_(FillInputBindings(pDefaultTable, DBACCESSOR_PARAMETERDATA,
				m_cBindings, m_rgBindings, &m_pData, ulRowNum, m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);
		
	// For each column with a default value, set obValue and obLength to bogus values
	// as provider must ignore them.
	for (iCol = 1; iCol <= cCols; iCol++)
	{
		CCol DefaultCol;

		TESTC_(pDefaultTable->GetColInfo(iCol, DefaultCol), S_OK);

		// We only want the uptable columns
		if (!DefaultCol.GetUpdateable())
			continue;

		if (DefaultCol.GetHasDefault())
		{
			// The column has a default value, set the value and length to bogus values.
			// Also set status to DBSTATUS_S_DEFAULT.
			STATUS_BINDING(m_rgBindings[iBind], m_pData) = DBSTATUS_S_DEFAULT;
			VALUE_BINDING(m_rgBindings[iBind], m_pData) = 0;
			LENGTH_BINDING(m_rgBindings[iBind], m_pData) = ULONG_MAX;
		}

		iBind++;
	}

	m_DbParamsAll.cParamSets = 1;
	m_DbParamsAll.pData = m_pData;

	// Reset the accessor to reflect the new bindings
	TEST_CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA | DBACCESSOR_ROWDATA,		
		m_cBindings,
		m_rgBindings,
		m_cbRowSize,
		&m_DbParamsAll.hAccessor,
		NULL), S_OK);

	// Perform insert.  We use a row seed larger than number of rows table was created
	// with to avoid duplicate rows.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on.
	if (SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	// Verify If we have inserted the row properly.
	TEST_COMPARE(cRow.FindRow(ulRowNum, pDefaultTable, m_pICommand, &pIRowset, NULL,
		&cCols, &pCols, FALSE), TRUE);

	// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
	TEST_COMPARE(VerifyObj(m_iidExec, pIRowset, ulRowNum, cCols, pCols,
		FALSE, FALSE, pDefaultTable, &cRowsExpected), S_OK);

	fResult = TEST_PASS;

CLEANUP:

	// Release the rowset
	SAFE_RELEASE(pIRowset);

	// If we set RANDOM REQUIRED above we need to set back to default
	if (hrSetProp == S_OK)
		CHECK(SetRowsetPropertyDefault(DBPROP_ACCESSORDER), S_OK);

	ReleaseDataForCommand();

	SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, m_DbParamsAll.hAccessor);

	SAFE_FREE(pwszSqlInsertAllWithParams);
	SAFE_FREE(pCols);

	ReleaseInputBindingsMemory(m_cBindings, m_rgBindings, m_pData, TRUE);

	if (pDefaultTable)
	{
		pDefaultTable->DropTable();
		delete pDefaultTable;
	}

	return fResult;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Multiple procs - execute two procs without SetParamInfo or Prepare
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_53()
{ 
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSet = DB_S_TYPEINFOOVERRIDDEN;
	DB_UPARAMS cActParams = 0;
	WCHAR * pNamesBuffer = NULL;
	DBPARAMINFO * pParamInfo = NULL;

	// Variables needed for second proc
	ULONG cParams2, cParamSets2 = 1;
	DBLENGTH cbRowSize2;
	DBBINDING * pBINDING2 = NULL;
	DB_UPARAMS * pParamOrdinals2 = NULL;
	DBPARAMBINDINFO * pPARAMBIND2=NULL;
	WCHAR * pwszCreateProcStmt2=NULL;
	WCHAR * pwszExecProcStmt2=NULL;
	WCHAR * pwszExecStmt2=NULL;
	WCHAR * pwszProcName2=NULL;
	BYTE * pData2=NULL;
	ParamStruct * pParamAll2=NULL;
	HRESULT hr = E_FAIL;
	
	DB_UPARAMS cParamsOut=0;
	DBPARAMINFO * pParamInfoOut = NULL;
	WCHAR * pNamesBufferOut = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with input and output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,					// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Create the syntax and binding for a second stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		2,						// [IN]  Row number in table to select, insert, or update
		&cParams2,				// [OUT] Count of params created
		&cbRowSize2,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING2,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals2,
		&pPARAMBIND2,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt2,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt2,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt2,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName2,			// [OUT] Name of procedure
		&pData2,					// [OUT] Pointer to data for the parameters
		&pParamAll2
	), TRUE);

	// Remove any old parameter info
	FAIL_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	ABORT_CHECK(hr = CreateStoredProc(m_pICommandText, pwszProcName, pwszCreateProcStmt, FALSE), S_OK);

	ABORT_CHECK(hr = CreateStoredProc(m_pICommandText, pwszProcName2, pwszCreateProcStmt2, FALSE), S_OK);
	
	// Set the command text to execute the stored proc
	ABORT_CHECK(hr = m_pICommandText->SetCommandText(DBGUID_DBSQL, pwszExecProcStmt), S_OK);

	if (m_pICommandPrepare)
		ABORT_CHECK(hr = m_pICommandPrepare->Prepare(1), S_OK);

	ABORT_CHECK(m_pICmdWParams->GetParameterInfo(&cParamsOut, &pParamInfoOut, &pNamesBufferOut), S_OK);

	// The final proof is that we can execute with these values
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll, ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_PDATA, TRUE), S_OK);

	// Now set up to execute the second stored proc
	// Set the command text to execute the stored proc
	ABORT_CHECK(hr = m_pICommandText->SetCommandText(DBGUID_DBSQL, pwszExecProcStmt2), S_OK);

	// The final proof is that we can execute with these values
	FAIL_VAR(ExecuteAndVerify(cParams2, cParamSets2, pParamAll2, 2, pBINDING2, cbRowSize2, pData2, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE), S_OK);


CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);
	DropStoredProcedure(m_pICommandText, pwszProcName2);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	PROVIDER_FREE(pBINDING2);
	PROVIDER_FREE(pParamOrdinals2);
	PROVIDER_FREE(pPARAMBIND2);
	PROVIDER_FREE(pwszCreateProcStmt2);
	PROVIDER_FREE(pwszExecProcStmt2);
	PROVIDER_FREE(pwszExecStmt2);
	PROVIDER_FREE(pwszProcName2);
	PROVIDER_FREE(pData2);
	PROVIDER_FREE(pParamAll2);

	return fResult ? TEST_PASS : TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Multiple procs - execute two procs with partial SetParamInfo and Prepare
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_54()
{ 
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSet = DB_S_TYPEINFOOVERRIDDEN;
	DB_UPARAMS cActParams = 0;
	WCHAR * pNamesBuffer = NULL;
	DBPARAMINFO * pParamInfo = NULL;

	// Variables needed for second proc
	ULONG cParams2, cParamSets2 = 1;
	DBLENGTH cbRowSize2 = 0;
	DBBINDING * pBINDING2 = NULL;
	DB_UPARAMS * pParamOrdinals2 = NULL;
	DBPARAMBINDINFO * pPARAMBIND2=NULL;
	WCHAR * pwszCreateProcStmt2=NULL;
	WCHAR * pwszExecProcStmt2=NULL;
	WCHAR * pwszExecStmt2=NULL;
	WCHAR * pwszProcName2=NULL;
	BYTE * pData2=NULL;
	ParamStruct * pParamAll2=NULL;
	HRESULT hr = E_FAIL;

	// For extra parameter validation after setting first param
	DB_UPARAMS cParamsOut=0;
	DBPARAMINFO * pParamInfoOut = NULL;
	WCHAR * pNamesBufferOut = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with input and output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,					// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Create the syntax and binding for a second stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		2,						// [IN]  Row number in table to select, insert, or update
		&cParams2,				// [OUT] Count of params created
		&cbRowSize2,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING2,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals2,
		&pPARAMBIND2,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt2,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt2,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt2,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName2,			// [OUT] Name of procedure
		&pData2,					// [OUT] Pointer to data for the parameters
		&pParamAll2
	), TRUE);

	// Create the second stored proc first
	ABORT_CHECK(hr = CreateStoredProc(m_pICommandText, pwszProcName2, pwszCreateProcStmt2, FALSE), S_OK);

	// Remove any old parameter info
	FAIL_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set up to execute the first stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Call SetParameterInfo for the first param
	hr = m_pICmdWParams->SetParameterInfo(1, pParamOrdinals, pPARAMBIND);

	ABORT_CHECK(m_pICmdWParams->GetParameterInfo(&cParamsOut, &pParamInfoOut, &pNamesBufferOut), S_OK);

	// Per spec it is provider-specific whether a provider returns information for all parameters or only
	// for the one I set above.  So if 1 is not returned then cParams should be
	if (cParamsOut != 1)
		FAIL_COMPARE(cParamsOut, cParams);

	// Check the parameter information returned for the proper number of params
	FAIL_VAR(VerifyParamInfo(cParamsOut, pParamOrdinals, pPARAMBIND,
			cParamsOut, pParamInfoOut, pNamesBufferOut), S_OK);

	SAFE_FREE(pParamInfoOut);
	SAFE_FREE(pNamesBufferOut);

	// If the provider returns information about all the params then we should be able to Execute.
	// Otherwise we don't have information about all params, and result of Execute is undefined.
	if (cParamsOut == cParams)
	{
		FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll, ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
			cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE), S_OK);
	}

	// Now set up to execute the second stored proc
	ABORT_CHECK(hr = m_pICommandText->SetCommandText(DBGUID_DBSQL, pwszExecProcStmt2), S_OK);

	if (m_pICommandPrepare)
		ABORT_CHECK(hr = m_pICommandPrepare->Prepare(1), S_OK);

	// Now get parameter info again
	ABORT_CHECK(m_pICmdWParams->GetParameterInfo(&cParamsOut, &pParamInfoOut, &pNamesBufferOut), S_OK);

	// Per spec it is provider-specific whether a provider returns information for all parameters or only
	// for the one I set above.  So if 1 is not returned then cParams should be
	if (cParamsOut != 1)
		FAIL_COMPARE(cParamsOut, cParams2);

	// Since we called SetParameterInfo for only one param it should match what we set
	FAIL_VAR(VerifyParamInfo(1, pParamOrdinals, pPARAMBIND,
			cParamsOut, pParamInfoOut, pNamesBufferOut), S_OK);

	// If any other were returned they should match the second stored proc
	if (cParamsOut > 1)
	{
		FAIL_VAR(VerifyParamInfo(cParamsOut-1, &pParamOrdinals2[1], &pPARAMBIND2[1],
				cParamsOut-1, &pParamInfoOut[1], pNamesBufferOut, 1), S_OK);
	}

	// We don't expect execution to succeed now that our parameter information is incorrect.
	// Since the parameter is really I/O but we set it Output only in SetParameterInfo then
	// there's no input parameter.  Expected return code may be DB_E_ERRORSOCCURRED or DB_E_PARAMNOTOPTIONAL.
	// However, since we claim in the spec "Result of executing with wrong parameter information is undefined"
	// then we could even succeed and return bogus data, or could crash.  So don't bother to test it.
//	FAIL_VAR(ExecuteAndVerify(cParams2, cParamSets2, pParamAll2, ulRowNum, pBINDING2, cbRowSize2, pData2, ROWSET_NONE, 
//		cColumns, prgColumnsOrd, VERIFY_USE_PDATA, TRUE, NULL, DB_E_ERRORSOCCURRED), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);
	DropStoredProcedure(m_pICommandText, pwszProcName2);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pNamesBufferOut);
	PROVIDER_FREE(pParamInfoOut);

	// Free other buffers
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	PROVIDER_FREE(pBINDING2);
	PROVIDER_FREE(pParamOrdinals2);
	PROVIDER_FREE(pPARAMBIND2);
	PROVIDER_FREE(pwszCreateProcStmt2);
	PROVIDER_FREE(pwszExecProcStmt2);
	PROVIDER_FREE(pwszExecStmt2);
	PROVIDER_FREE(pwszProcName2);
	PROVIDER_FREE(pData2);
	PROVIDER_FREE(pParamAll2);

	return fResult ? TEST_PASS : TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc DBSTATUS_S_IGNORE - Not legal for params (DBSTATUS_E_BADSTATUS)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_55()
{ 
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	ULONG iParam;
	DBCOUNTITEM ulRowNum;
	HRESULT hr = E_FAIL;

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	// No need to check return code, checked inside function
	if (FAILED(SetParameterInfoIfNeeded(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo)))
		goto CLEANUP;

	//This sets it to one row of data.
	MakeDataForCommand((ulRowNum = NextInsertRowNum()));

	// Set each param status to DBSTATUS_S_IGNORE in turn, Execute should fail.
	// We have to set them individually because the provider might fail to catch
	// for some data types.
	for (iParam = 0; iParam < m_cBindings; iParam++)
	{
		// We assume the initial status was DBSTATUS_S_OK
		if (iParam && STATUS_BINDING(m_rgBindings[iParam-1], m_pData) == DBSTATUS_S_IGNORE)
			STATUS_BINDING(m_rgBindings[iParam-1], m_pData) = DBSTATUS_S_OK;
	
		if (STATUS_BINDING(m_rgBindings[iParam], m_pData) == DBSTATUS_S_OK)
		{
			STATUS_BINDING(m_rgBindings[iParam], m_pData) = DBSTATUS_S_IGNORE;

			// Now execute ICommand.
			hr = m_pICommand->Execute(NULL, IID_NULL, &m_DbParamsAll, &cRowsAffected, NULL);

			if (SUCCEEDED(hr))
				m_pTable->AddRow();

			TESTC_(hr, DB_E_ERRORSOCCURRED);

			// Validate the status value is DBSTATUS_E_BADSTATUS
			TESTC(STATUS_BINDING(m_rgBindings[iParam], m_pData) == DBSTATUS_E_BADSTATUS);
		}
	}

	fResult = TRUE;
CLEANUP:

	ReleaseDataForCommand();

	return (fResult) ? TEST_PASS : TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc OpenRowset on stored proc without params
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_56()
{ 
	// Check for support.  If provider supports DBPROP_OPENROWSETSUPPORT and
	// DBPROPVAL_ORS_STOREDPROC, then OpenRowset will open a stored proc that
	// has zero params.

	ULONG_PTR ulOpenRowsetSupport = 0;
	BOOL fResult = TRUE;
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSetProp = E_FAIL;
	DBID dbidProc = DB_NULLID;
	IOpenRowset * pIOpenRowset = NULL;
	IRowset * pIRowset = NULL;
	HRESULT hrOpen = E_FAIL;
	HRESULT hrExpected = S_OK;

	if (!(GetProperty(DBPROP_OPENROWSETSUPPORT, 
				   DBPROPSET_DATASOURCEINFO,m_pIDBInitialize, &ulOpenRowsetSupport) &&
		ulOpenRowsetSupport & DBPROPVAL_ORS_STOREDPROC))
	{
		odtLog << L"Provider doesn't support OpenRowset on a stored proc.\n";
		hrExpected = DB_E_NOTABLE;
	}

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	// Oracle cannot return a rowset from within a stored proc at this time.
	if (g_bOracle)
	{
		odtLog << L"Oracle cannot return a rowset from a stored proc.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with input parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_NO_PARM,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd
	), TRUE);

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on.  However, other providers
	// can't retrieve a row object with ACCESSORDER RANDOM.  So we won't set it when retrieving row object.
	if (m_iidExec == IID_IRowset && SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);
	
	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Can't validate data at this time for row objects w/o ini file (if any extra row object columns)
	if (m_iidExec == IID_IRow && g_bConfProv && !GetModInfo()->GetFileName())
		odtLog << L"Can't verify row object data w/o ini file at this time.  Data compasison skipped. \n";
	else
	{
		FAIL_VAR(ValidateGetParameterInfo(cParams, cParamSets, ulRowNum, pParamOrdinals, pPARAMBIND,
			pBINDING, cbRowSize, pParamAll, pData, ROWSET_MAYBE, cColumns, prgColumnsOrd, fCanDerive), S_OK);
	}

	// Now try to "open" the stored proc via OpenRowset

	// Get an OpenRowset interface
	TEST_COMPARE(VerifyInterface(m_pThisTestModule->m_pIUnknown2, 
		IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&pIOpenRowset),TRUE);

	// Create a DBID from the proc name
	dbidProc.eKind = DBKIND_NAME;
	dbidProc.uName.pwszName = pwszProcName;

	// Call OpenRowset on the stored proc
	hrOpen = pIOpenRowset->OpenRowset(NULL, &dbidProc, NULL, m_iidExec, 
		0, NULL, (IUnknown **)&pIRowset);

	TEST_CHECK(hrOpen, hrExpected);

	// Validate the rowset returned
	if (hrOpen == S_OK)
		FAIL_VAR(VerifyObj(m_iidExec, pIRowset, ulRowNum, cColumns, prgColumnsOrd, TRUE), S_OK);

CLEANUP:

	// If we set RANDOM REQUIRED above we need to set back OPTIONAL
	if (hrSetProp == S_OK)
		CHECK(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM, DBPROPOPTIONS_OPTIONAL), S_OK);

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	SAFE_RELEASE(pIOpenRowset);
	
	return fResult ? TEST_PASS : TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc Select with DBSTATUS_S_ISNULL input params
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_57()
{ 
	BOOL fResult = TEST_PASS;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	IRowset * pIRowset = NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	DB_LORDINAL * prgParamColOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSetProp = E_FAIL;
	HRESULT hrGetNextRows = E_FAIL;
	DBROWCOUNT cRowsAffected = 0;
	DBCOUNTITEM cRowsObtained = 0;
	DBCOUNTITEM cRowsInRowset = 0;
	HROW * prghRows = NULL;
	ULONG iParam, iRow;
	ULONG ulNullRow = 0;
	ULONG ulNullParam = 0;
	ULONG ulNonNullRow = 0;
	HRESULT hrExpect = S_OK;

	// If we're asking for a row object then a select with NULL params will likely return DB_E_NOTFOUND
	if (m_iidExec == IID_IRow)
		hrExpect = DB_E_NOTFOUND;

	// Temporarily set the CTable object not to create NULL data, otherwise columns containing
	// NULL will not have a parameter.
	m_pTable->SetNull(NONULLS);

	// Create the syntax and binding for a stored proc and statement using input parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_IN,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd,
		&prgParamColOrd
	), TRUE);

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on
	if (SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);
	
	// Set up to execute the statement
	ABORT_CHECK(PrepareForExecute(pwszExecStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, NULL), S_OK);

	// Set the parameter info if we need to
	ABORT_CHECK(SetParameterInfoIfNeeded(cParams, pParamOrdinals, pPARAMBIND), S_OK);

	// Find a row with a NULL in one of the parameter columns
	m_pTable->SetNull(USENULLS);
	for (iParam = 0; iParam < cParams && !ulNullRow; iParam++)
	{
		HRESULT hrNULL = E_FAIL;

		ulNullRow = 0;

		for (iRow = 1; iRow <= m_pTable->GetRowsOnCTable() && !ulNullRow; iRow++)
		{
			WCHAR wszData[MAXDATALEN] = L"";

			// If this row/column data is NULL
			hrNULL = m_pTable->MakeData(wszData, iRow, pParamAll[iParam].ulColIndex, PRIMARY, TRUE);

			if (hrNULL == S_FALSE)
			{
				if (!ulNullRow)
				{
					ulNullRow = iRow;
					ulNullParam = iParam;
				}
			}
		}
	}

	// Find a row without a NULL in any of the parameter columns
	for (iRow = 1; iRow <= m_pTable->GetRowsOnCTable() && !ulNonNullRow; iRow++)
	{
		HRESULT hrNULL = E_FAIL;

		ulNonNullRow = iRow;

		for (iParam = 0; iParam < cParams; iParam++)
		{
			WCHAR wszData[MAXDATALEN] = L"";

			// If this row/column data is NULL
			hrNULL = m_pTable->MakeData(wszData, iRow, pParamAll[iParam].ulColIndex, PRIMARY, TRUE);

			if (hrNULL == S_FALSE)
			{
				ulNonNullRow = 0;
				break;
			}
		}
	}


	if (!ulNullRow)
	{
		odtLog << L"No row containing a NULL we can use as a param.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	if (!ulNonNullRow)
	{
		odtLog << L"No row with all non-NULL data.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	// Now refill the parameter data buffer because we filled it with non-null data
	ABORT_CHECK (FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cParams,
		pBINDING, &pData, ulNullRow, cParams, prgParamColOrd, PRIMARY), S_OK);

	// Execute and make sure results are either 0 rows or valid row.
	ABORT_CHECK(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulNullRow, pBINDING,
		cbRowSize, pData, ROWSET_ALWAYS, cColumns, prgColumnsOrd, VERIFY_USE_TABLE,
		TRUE, NULL, hrExpect, NULL, NULL, &cRowsInRowset), S_OK);

	// Typically we will get 0 rows in the rowset, but don't fail a provider if it returned
	// one row with valid data.
	if (cRowsInRowset)
		FAIL_COMPARE(cRowsInRowset, 1);

	// Now we have to validate we can get a rowset back properly when NOT asking for
	// DBSTATUS_S_ISNULL.

	// If set the row number a non-NULL row for this param
	// Now refill the parameter data buffer because we filled it with non-null data
	ABORT_CHECK (FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cParams,
		pBINDING, &pData, ulNonNullRow, cParams, prgParamColOrd, PRIMARY), S_OK);

	// Check GetParameterInfo result and Execute result for this statement
	cRowsInRowset = 1;	// We expect only one row back, 0 or >1 is an error.
	ABORT_CHECK(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulNonNullRow, pBINDING,
		cbRowSize, pData, ROWSET_ALWAYS, cColumns, prgColumnsOrd, VERIFY_USE_TABLE,
		TRUE, NULL, S_OK, NULL, NULL, &cRowsInRowset), S_OK);

CLEANUP:

	// Remove parameter info
	CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Make sure we set back to USENULLS in case of abort.
	m_pTable->SetNull(USENULLS);

	// If we set RANDOM REQUIRED above we need to set back OPTIONAL
	if (hrSetProp == S_OK)
		CHECK(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM, DBPROPOPTIONS_OPTIONAL), S_OK);

	SAFE_RELEASE(pIRowset);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	PROVIDER_FREE(prgColumnsOrd);
	
	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Send less data than max for parameter
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_58()
{ 
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	DBCOUNTITEM ulRowNum;
	ULONG iBind;
	
	ULONG cParamSets = 1; 
	ULONG cParams;
	DBCOUNTITEM cRowsObtained = 100;
	DBLENGTH cbRowSize = 0;
	HROW * prghRows = NULL;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSetProp = E_FAIL;
	HACCESSOR hParamAccessor = DB_INVALID_HACCESSOR;
	DBPARAMS ExecDbParams;
	IRowset * pIRowset = NULL;
	IAccessor * pIAccessor = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBBINDING * pBindings = NULL;
	DBCOUNTITEM cBindings = 0;
	DBLENGTH cbRowSizeRowset = 0;
	BYTE * pDataRowset = NULL;
	DBCOLUMNINFO * pColInfo = NULL;
	LPWSTR pStringsBuffer = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	// Oracle cannot return a rowset from within a stored proc at this time.
	if (g_bOracle)
	{
		odtLog << L"Oracle cannot return a rowset from a stored proc.\n";
		return TEST_SKIPPED;
	}

	if (m_iidExec == IID_IRow)
	{
		odtLog << L"Can't run this var for row objects at this time because accessors are not supported.  \n";
		return TEST_SKIPPED;
	}

	// Remove any old parameter info
	TEST_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	//This sets it to one row of data.
	MakeDataForCommand(ulRowNum = m_pTable->GetNextRowNumber());

	// Use local names
	pData = m_pData;
	pBINDING = &m_rgBindings[0];
	cParams = (ULONG)m_cBindings;


	// We created normal data above, now we have to change it
	for (iBind = 0; iBind < cParams; iBind++)
	{
		DBBINDING * pBind = &pBINDING[iBind];
		LPBYTE pCol = (LPBYTE)&VALUE_BINDING(*pBind, pData);
		DBTYPE wType = pBind->wType;
		DBLENGTH cbMaxLen = pBind->cbMaxLen;
		DBLENGTH ulDataSize = LENGTH_BINDING(*pBind, pData);
		DBSTATUS sStatus = STATUS_BINDING(*pBind, pData);

		// Don't try to fix up NULL status values
		if (sStatus != DBSTATUS_S_OK)
			continue;

		// Since FillInputBindings uses bogus length values for fixed length
		// types we won't be able to compare the lengths unless we fix them up.
		if (IsFixedLength(wType))
			ulDataSize = GetDBTypeSize(wType);

		if (DBTYPE_WSTR == wType)
			ulDataSize = sizeof(WCHAR);
		else if (wType == DBTYPE_STR ||
			wType == DBTYPE_BYTES)
		{
			// Assume 1 byte
			ulDataSize = 1;

			// Can't split a lead byte
			if (DBTYPE_STR == wType && IsDBCSLeadByte(*pCol))
				ulDataSize++;
		}

		// Now adjust the data
		LENGTH_BINDING(*pBind, pData) = ulDataSize;
	}

	// Reset vars
	pData = NULL;
	pBINDING = NULL;
	cParams = 0;
	cbRowSize = 0;

	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Now we have a row of one char/byte values, let's see if we can retrieve it
	// using a stored proc

	// Create the syntax and binding for a stored proc with input parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_IN,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd
	), TRUE);

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on
	if (SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);
	
	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on.
	if (SupportedProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		CHECK(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_IRowsetChange, TRUE), S_OK);
	if (SupportedProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		CHECK(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_UPDATABILITY, (LONG_PTR)DBPROPVAL_UP_DELETE), S_OK);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Go through the params and set to the same size we did previously
	for (iBind = 0; iBind < cParams; iBind++)
	{
		DBBINDING * pBind = &pBINDING[iBind];
		LPBYTE pCol = (LPBYTE)&VALUE_BINDING(*pBind, pData);
		DBTYPE wType = pBind->wType;
		DBLENGTH ulDataSize = LENGTH_BINDING(*pBind, pData);
		DBSTATUS sStatus = STATUS_BINDING(*pBind, pData);

		// Don't try to fix up NULL status values
		if (sStatus != DBSTATUS_S_OK)
			continue;

		if (DBTYPE_WSTR == wType)
			ulDataSize = sizeof(WCHAR);
		else if (wType == DBTYPE_STR ||
			wType == DBTYPE_BYTES)
		{
			// Assume 1 byte
			ulDataSize = 1;

			// Can't split a lead byte
			if (DBTYPE_STR == wType && IsDBCSLeadByte(*pCol))
				ulDataSize++;
		}

		// Now adjust the data
		LENGTH_BINDING(*pBind, pData) = ulDataSize;

	}

	// Set up call to execute stored proc

	// Null out DBPARAMS
	memset(&ExecDbParams, 0, sizeof(DBPARAMS));
	ABORT_CHECK (m_pCmdIAccessor->CreateAccessor( DBACCESSOR_PARAMETERDATA,
		cParams, pBINDING, cbRowSize, &hParamAccessor, NULL), S_OK);

	// Set up our param accessor
	ExecDbParams.hAccessor = hParamAccessor;
	ExecDbParams.cParamSets = cParamSets;
	ExecDbParams.pData = pData;

	// Execute the stored proc, should get one row back
	ABORT_CHECK(m_pICommand->Execute(NULL, m_iidExec, &ExecDbParams, 
		&cRowsAffected, (IUnknown **)&pIRowset), S_OK);

	// Make sure we can get an accessor interface
	// TESTBUG: Row object does not support IAccessor interface.  Needs work.
	TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown **)&pIAccessor));

	TESTC_(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
		&hAccessor, &pBindings, &cBindings, &cbRowSizeRowset,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		UPDATEABLE_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		&pColInfo, NULL, &pStringsBuffer, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	SAFE_ALLOC(pDataRowset, BYTE, cbRowSizeRowset);

	// Now validate the data
	TESTC_(pIRowset->GetNextRows(NULL, 0, 100, &cRowsObtained, &prghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained == 1);
	TESTC(prghRows != NULL);

	TESTC_(pIRowset->GetData(*prghRows, hAccessor, pDataRowset), S_OK);

	TESTC(cBindings == m_cBindings);

	// STR, WSTR, and BTYPES columns that return DBCOLUMNFLAGS_ISFIXEDLENGTH will be
	// padded with spaces or NULLs out to the size of the column.  This will cause
	// comparebuffer to fail unless we fix this up also.
	for (iBind = 0; iBind < cBindings; iBind++)
	{
		DBTYPE wType = m_rgBindings[iBind].wType;
		DBSTATUS sStatus = STATUS_BINDING(m_rgBindings[iBind], m_pData);

		// We have to get the ordinal from pBindings because that matches the rowset
		// returned and therefore the colinfo.
		DBCOLUMNFLAGS dwFlags = pColInfo[pBindings[iBind].iOrdinal - !!pColInfo[0].iOrdinal].dwFlags;

		ASSERT(pBindings[iBind].iOrdinal == pColInfo[pBindings[iBind].iOrdinal - !!pColInfo[0].iOrdinal].iOrdinal);

		if ((IS_BASE_TYPE(wType, DBTYPE_STR) ||
			IS_BASE_TYPE(wType, DBTYPE_WSTR) ||
			IS_BASE_TYPE(wType, DBTYPE_BYTES)) &&
			dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH &&
			sStatus == DBSTATUS_S_OK)
		{
			// We need to fix up the size of our inserted data buffer
			DBLENGTH ulLength = LENGTH_BINDING(m_rgBindings[iBind], m_pData);
			DBLENGTH ulNewLength;
			BYTE * pFill = (BYTE *)&VALUE_BINDING(m_rgBindings[iBind], m_pData)+ulLength;
			
			// Fill with spaces, wide spaces, or 0's
			if (IS_BASE_TYPE(wType, DBTYPE_STR))
			{
				ulNewLength = m_rgBindings[iBind].cbMaxLen-sizeof(CHAR);
				_strnset((LPSTR)pFill, ' ', (size_t)(ulNewLength-ulLength));
			}
			else if (IS_BASE_TYPE(wType, DBTYPE_WSTR))
			{
				ulNewLength = m_rgBindings[iBind].cbMaxLen-sizeof(WCHAR);
				_wcsnset((LPWSTR)pFill, L' ', (size_t)((ulNewLength-ulLength)/sizeof(WCHAR)));
			}
			else if (IS_BASE_TYPE(wType, DBTYPE_BYTES))
			{
				ulNewLength = m_rgBindings[iBind].cbMaxLen;
				memset(pFill, 0, (size_t)(ulNewLength-ulLength));
			}

			LENGTH_BINDING(m_rgBindings[iBind], m_pData) = ulNewLength;
		}
	}

	TESTC(CompareBuffer(pDataRowset, m_pData, cBindings, pBindings, NULL, TRUE, FALSE, COMPARE_ONLY,
		FALSE, m_cBindings, m_rgBindings));

	fResult = TRUE;

CLEANUP:

	ReleaseDataForCommand();

	// We have a goofy row in the table which we should remove.  Use the row handle.  Note if 
	// the provider has a bug where we don't actually get the row handle then we may fail to
	// remove the row.
	if (prghRows)
	{
		IRowsetChange * pIRowsetChange = NULL;
		COMPARE(VerifyInterface (pIRowset, IID_IRowsetChange, ROWSET_INTERFACE, 
			(IUnknown **)&pIRowsetChange), TRUE);
		if (pIRowsetChange)
			CHECK(pIRowsetChange->DeleteRows(NULL, 1, prghRows, NULL), S_OK);
		SAFE_RELEASE(pIRowsetChange);
	}

	SAFE_RELEASE_ACCESSOR(pIAccessor, hAccessor);
	SAFE_RELEASE(pIAccessor);
	SAFE_FREE(pBindings);
	SAFE_FREE(pColInfo);
	SAFE_FREE(pStringsBuffer);
	SAFE_FREE(pDataRowset);
	if (pIRowset && prghRows)
		CHECK(pIRowset->ReleaseRows(1, prghRows, NULL, NULL, NULL), S_OK);
	SAFE_FREE(prghRows);
	SAFE_RELEASE(pIRowset);

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Set props back to default
	CHECK(SetRowsetPropertyDefault(DBPROP_UPDATABILITY), S_OK);
	CHECK(SetRowsetPropertyDefault(DBPROP_IRowsetChange), S_OK);
	CHECK(SetRowsetPropertyDefault(DBPROP_ACCESSORDER), S_OK);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Insert proc with CANHOLDROWS OPTIONAL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_59()
{ 
	BOOL fResult = TEST_FAIL;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSetProp = E_FAIL;
	IRowset * pIRowset = NULL;
	DBCOUNTITEM cRows = 1;	// We expect one row to be retrieved, the one we insert.
	DBORDINAL cCols = 0;
	DB_LORDINAL * pCols = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (m_iidExec == IID_IRow && g_bConfProv && !GetModInfo()->GetFileName())
	{
		odtLog << L"Can't verify row object data w/o ini file at this time.  \n";
		return TEST_SKIPPED;
	}

	// Get a valid row we can insert
	ulRowNum = m_pTable->GetNextRowNumber();

	// Create the syntax and binding for a stored proc with input parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_INSERT_INPUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd
	), TRUE);

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on
	if (m_iidExec != IID_IRow || !g_bLuxor &&
		SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	// Set CANHOLDROWS
	TESTC_(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_CANHOLDROWS, TRUE, DBPROPOPTIONS_OPTIONAL), S_OK);
	
	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	// Check GetParameterInfo and execute
	TESTC_(ExecuteAndVerify(cParams, cParamSets, pParamAll, ulRowNum, pBINDING, cbRowSize,
		pData, ROWSET_NONE, cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE, NULL, S_OK), S_OK);

	// Tell the table object there's one more row.  Note we really need to
	// split up the ExecuteAndVerify above so we know Execute itself succeeded.
	m_pTable->AddRow();

	// Set CANHOLDROWS to default for verification in case it can't be supported.
	CHECK(SetRowsetPropertyDefault(DBPROP_CANHOLDROWS), S_OK);

	// See if the row was properly inserted
	// Verify If we have inserted the row properly.
	TESTC(cRow.FindRow(ulRowNum, m_pTable, m_pICommand, &pIRowset, NULL,
		&cCols, &pCols, FALSE));

	// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
	TESTC_(VerifyObj(m_iidExec, pIRowset, ulRowNum, cCols, pCols, FALSE, FALSE, m_pTable, &cRows), S_OK);

	fResult = TEST_PASS;

CLEANUP:

	// Free the rowset
	SAFE_RELEASE(pIRowset);

	// Set CANHOLDROWS to default
	CHECK(SetRowsetPropertyDefault(DBPROP_CANHOLDROWS), S_OK);

	// Set RANDOM back to default
	CHECK(SetRowsetPropertyDefault(DBPROP_ACCESSORDER), S_OK);

	DropStoredProcedure(m_pICommandText, pwszProcName);

	SAFE_FREE(pCols);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Insert with BSTR bindings to strings to stored proc
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_60()
{ 
	BOOL fResult = TEST_FAIL;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSetProp = E_FAIL;
	IRowset * pIRowset = NULL;
	DBCOUNTITEM cRows = 1;	// We expect one row to be retrieved, the one we insert.
	BOOL fConvertSTR = FALSE;
	BOOL fConvertWSTR = FALSE;
	HRESULT hrConv = E_FAIL;
	ULONG iBind;
	IConvertType * pIConvertType = NULL;
	DB_LORDINAL * prgParamColOrd = NULL;
	DBORDINAL cCols = 0;
	DB_LORDINAL * pCols = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	// Get a valid row we can insert
	ulRowNum = m_pTable->GetNextRowNumber();

	// Create the syntax and binding for a stored proc with input parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_INSERT_INPUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd,
		&prgParamColOrd
	), TRUE);

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on
	if (SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	TESTC_(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	if (!fCanDerive)
		TESTC_(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), S_OK);

	// Make sure the provider claims it can support conversion from BSTR -> STR
	// or BSTR -> WSTR.
	TESTC(VerifyInterface(m_pICommand, IID_IConvertType,
				COMMAND_INTERFACE,(IUnknown **)&pIConvertType));

	hrConv = pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_STR, DBCONVERTFLAGS_PARAMETER);

	if (S_OK == hrConv)
		fConvertSTR = TRUE;
	else
		CHECK(hrConv, S_FALSE);

	hrConv = pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR, DBCONVERTFLAGS_PARAMETER);

	if (S_OK == hrConv)
		fConvertWSTR = TRUE;
	else
		CHECK(hrConv, S_FALSE);

	if (!fConvertSTR && !fConvertWSTR)
	{
		odtLog << L"Provider doesn't support conversions from BSTR to string types.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	
	// Release the previous parameter set
	ReleaseInputBindingsMemory(cParams, pBINDING, pData, TRUE);
	pData = NULL; // Freed above.

	// Change str and wstr types to bstr
	for (iBind=0; iBind < cParams; iBind++)
	{
		if (pBINDING[iBind].wType == DBTYPE_STR && fConvertSTR)
			pBINDING[iBind].wType = DBTYPE_BSTR;

		if (pBINDING[iBind].wType == DBTYPE_WSTR && fConvertWSTR)
			pBINDING[iBind].wType = DBTYPE_BSTR;
	}

	// Repack the obValue, obLength, and obStatus based on the new bindings
	Repack(cParams, pBINDING, &cbRowSize);

	// Create a new pData based on the new bindings
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cParams,
		pBINDING, &pData, ulRowNum, cParams, prgParamColOrd, PRIMARY), S_OK);

	// Execute
	TESTC_(ExecuteAndVerify(cParams, cParamSets, pParamAll, ulRowNum, pBINDING, cbRowSize,
		pData, ROWSET_NONE, cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE, NULL, S_OK), S_OK);

	// Tell the table object there's one more row.  Note we really need to
	// split up the ExecuteAndVerify above so we know Execute itself succeeded.
	m_pTable->AddRow();

	// See if the row was properly inserted
	// Verify If we have inserted the row properly.
	TESTC(cRow.FindRow(ulRowNum, m_pTable, m_pICommand, &pIRowset, NULL,
		&cCols, &pCols, FALSE));

	// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
	TESTC_(VerifyObj(m_iidExec, pIRowset, ulRowNum, cCols, pCols, FALSE, FALSE, m_pTable, &cRows), S_OK);

	fResult = TEST_PASS;

CLEANUP:

	// Free the rowset
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIConvertType);

	// Set RANDOM back to default
	CHECK(SetRowsetPropertyDefault(DBPROP_ACCESSORDER), S_OK);

	DropStoredProcedure(m_pICommandText, pwszProcName);

	SAFE_FREE(pCols);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VARIATION_DELETED *************** DELETED *************** DELETED *************** DELETED
//*-----------------------------------------------------------------------
// @mfunc Test variation - remove
//
// @rdesc TEST_PASS or TEST_FAIL 
//
/*
{ 
	BOOL fResult = TEST_FAIL;
	
	ULONG cParams, cParamSets = 1, cbRowSize;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSetProp = E_FAIL;
	IRowset * pIRowset = NULL;
	ULONG cRows = 1;	// We expect one row to be retrieved, the one we insert.
	BOOL fConvertSTR = FALSE;
	BOOL fConvertWSTR = FALSE;
	HRESULT hrConv = E_FAIL;
	ULONG iBind;
	IConvertType * pIConvertType = NULL;
	DB_LORDINAL * prgParamColOrd = NULL;
	WCHAR wszProcCall[] = L"{call sp_tables(?)}";

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	// Get a valid row we can insert
	ulRowNum = m_pTable->GetNextRowNumber();

	// Create the syntax and binding for a stored proc with input parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_INSERT_INPUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd,
		&prgParamColOrd
	), TRUE);

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on
	if (SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	// Set up to execute the stored proc
	if (pwszCreateProcStmt)
		// Create the stored procedure
		TESTC_(CreateStoredProc(m_pICommandText, pwszProcName, pwszCreateProcStmt, FALSE), S_OK);

	pwszExecProcStmt = (LPWSTR)wszProcCall;

	// Set the command text to execute the stored proc
	TESTC_(m_pICommandText->SetCommandText(DBGUID_DBSQL, pwszExecProcStmt), S_OK);

	// Make sure the provider claims it can support conversion from BSTR -> STR
	// or BSTR -> WSTR.
	TESTC(VerifyInterface(m_pICommand, IID_IConvertType,
				COMMAND_INTERFACE,(IUnknown **)&pIConvertType));

	hrConv = pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_STR, DBCONVERTFLAGS_PARAMETER);

	if (S_OK == hrConv)
		fConvertSTR = TRUE;
	else
		CHECK(hrConv, S_FALSE);

	hrConv = pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR, DBCONVERTFLAGS_PARAMETER);

	if (S_OK == hrConv)
		fConvertWSTR = TRUE;
	else
		CHECK(hrConv, S_FALSE);

	if (!fConvertSTR && !fConvertWSTR)
	{
		odtLog << L"Provider doesn't support conversions from BSTR to string types.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	
	// Release the previous parameter set
	ReleaseInputBindingsMemory(cParams, pBINDING, pData, TRUE);
	pData = NULL; // Freed above.

	// Change str and wstr types to bstr
	for (iBind=0; iBind < cParams; iBind++)
	{
		if (pBINDING[iBind].wType == DBTYPE_WSTR && fConvertWSTR)
			pBINDING[iBind].wType = DBTYPE_BSTR;
		break;
	}

	cParams = 1;

	// Repack the obValue, obLength, and obStatus based on the new bindings
	Repack(cParams, &pBINDING[iBind], &cbRowSize);


	// Create a new pData based on the new bindings
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cParams,
		&pBINDING[iBind], &pData, ulRowNum, cParams, (LONG *)&prgParamColOrd[iBind], PRIMARY), S_OK);

	// Now copy in a valid table name
	wcscpy((LPWSTR)&VALUE_BINDING(pBINDING[iBind], pData), L"sysusers");

	// Execute
	TESTC_(ExecuteAndVerify(cParams, cParamSets, &pParamAll[iBind], ulRowNum, &pBINDING[iBind], cbRowSize,
		pData, ROWSET_NONE, 1, &prgColumnsOrd[iBind], VERIFY_USE_TABLE, TRUE, NULL, S_OK), S_OK);

	// See if the row was properly inserted
	// Verify If we have inserted the row properly.
	TESTC(cRow.FindRow(ulRowNum, m_pTable, m_pICommand, &pIRowset));

	// FindRow merely finds the appropriate row.  We need to validate the inserted data for the default bindings.
	TESTC_(VerifyObj(m_iidExec, pIRowset, ulRowNum, cColumns, prgColumnsOrd, FALSE, TRUE, m_pTable, &cRows), S_OK);

	fResult = TEST_PASS;

CLEANUP:

	// Free the rowset
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIConvertType);

	// Set CANHOLDROWS to default
	TESTC_(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_CANHOLDROWS, VT_EMPTY), S_OK);

	// Set RANDOM back to default
	CHECK(SetRowsetPropertyDefault(DBPROP_ACCESSORDER), S_OK);

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult;
} 
*/
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Input and output params with cParamSets > 1
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCommandExecute_Rowset::Variation_61()
{ 
		
	// Need to test with all paramter names set to NULL so we can verify
	// names aren't required.
	BOOL fResult = TRUE;
	
	ULONG cParams = 0, cParamSets = 5;
	DBLENGTH cbRowSize;
	DBCOUNTITEM iRow, ulRowNum = 3;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	ULONG iParam;
	HRESULT hrSet = DB_S_TYPEINFOOVERRIDDEN;
	DB_UPARAMS cActParams = 0;
	WCHAR * pNamesBuffer = NULL;
	DBPARAMINFO * pParamInfo = NULL;
	BOOL fTableHadNulls = FALSE;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	if (!g_bMultipleParamSets)
	{
		odtLog << "Multiple parameter sets are not supported \n";
		return TEST_SKIPPED;
	}

	// This variation cannot work if one of the values in one of the table columns is NULL
	// unless the whole column is NULL, because the syntax used in the 'where' clause
	// has to use the IS NULL construct rather than using a parameter.  This would require
	// different syntax for each param set, meaning a different statement for each param
	// set.  Since this isn't possible we will have to insert cParamSets rows that do not
	// contain any NULLs.  Since these violate the underlying table's information we will 
	// have to remove them afterwards.
	if (m_pTable->GetNull() == USENULLS)
	{
		// Record that table did have nulls
		fTableHadNulls = TRUE;

		// Make the table think it doesn't have NULLS
		m_pTable->SetNull(NONULLS);

		// Record the starting number of the new rows
		ulRowNum = m_pTable->GetNextRowNumber();

		// Insert cParamSets new rows
		TESTC_(m_pTable->Insert(ulRowNum, PRIMARY, TRUE, NULL, FALSE, cParamSets), S_OK);	
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set all the parameter names NULL
	for (iParam=0; iParam < cParams; iParam++)
		pPARAMBIND[iParam].pwszName = NULL;

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	if (!fCanDerive)
		hrSet = S_OK;

	// Now set the parameter information correctly
	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), hrSet);

	// Get it again so we can validate we get back what we set.
	ABORT_CHECK(m_pICmdWParams->GetParameterInfo(&cActParams, &pParamInfo, &pNamesBuffer), S_OK);

	// Verify results.  If we didn't get back what was set it might not be a failure
	FAIL_VAR(VerifyParamInfo(cParams, pParamOrdinals, pPARAMBIND,
			cActParams, pParamInfo, pNamesBuffer), S_OK);

	// The final proof is that we can execute with these values
	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll, ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		cColumns, prgColumnsOrd, VERIFY_USE_TABLE, TRUE), S_OK);


CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pParamInfo);
	PROVIDER_FREE(pNamesBuffer);
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);

	// Reset table parameters and delete any added rows
	if (fTableHadNulls)
	{
		// Delete the extra non-null rows we added
		for (iRow = ulRowNum; iRow < ulRowNum + cParamSets; iRow++)
			CHECK(m_pTable->Delete(iRow), S_OK);

		m_pTable->SetNull(USENULLS);
	}
	
	return fResult ? TEST_PASS : TEST_FAIL;

} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCommandExecute_Rowset::Terminate()
{

	if (g_ulOutParamsSupported != DBPROPVAL_OA_NOTSUPPORTED)
	{
		if (m_pSPIAccessor)
			m_pSPIAccessor->ReleaseAccessor(m_hReadAccessor, NULL);

		DropSP();
		RELEASE (m_pSPICommandText);
		RELEASE (m_pSPICommandPrepare);

		SAFE_RELEASE(m_pSPIAccessor);
		PROVIDER_FREE(m_rgSPBindings);
		PROVIDER_FREE(m_rgSPParamBind);
		PROVIDER_FREE(m_rgSPParamColOrdinals);
		PROVIDER_FREE(m_rgSPParamColMap);
		PROVIDER_FREE(m_rgReadBindings);
		::FreeParameterNames(m_cSPBindings, m_pSPParamAll);
		PROVIDER_FREE(m_pSPParamAll);
	}

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CICmdWParams::Terminate());

}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCSetParameterInfo_Rowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCSetParameterInfo_Rowset - Test case for SetParameterInfo
//|	Created:			05/03/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSetParameterInfo_Rowset::Init()
{
	HRESULT hr;
	BOOL fResult = FALSE;

	if (m_eTestCase == TC_Row && !g_fRowObj)
	{
		odtLog << L"Row objects not supported.\n";
		return TEST_SKIPPED;
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	if(CICmdWParams::Init())
	// }}
	{
		if (!m_bSetParameterInfo)
		{
			// IF SetParameterInfo is not set.  set it anyway.  for testing SetParameterInfo.
			if (! CHECK (m_pICommandPrepare->Prepare(1), S_OK))
				goto CLEANUP;

			hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
			if (hr != DB_S_TYPEINFOOVERRIDDEN)
			{
				if (!CHECK(hr, S_OK))
					goto CLEANUP;
			}

		}

		// Create an array of parameter names
		m_pwszParameterNames=CreateParameterNames(NULL,  ALL_VALID_NAMES, NULL, INCLUDE_LONG_COLS);
		if (!m_pwszParameterNames)
			goto CLEANUP;

		fResult = TRUE;
	}
CLEANUP:

	return fResult;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Call SetParameterInfo.  GetParameterInfo. Should match.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_1()
{
	BOOL fResult = TRUE;

	// Note: SetParameterInfo called in init for insert statement with params
	
	FreeDescParams();

	ABORT_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);
	
	FAIL_VAR(CompareDBParamInfo(m_cDbParamBindInfo, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer), S_OK);

CLEANUP:

	FreeDescParams();

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc SetParameterInfo.  Execute.  S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_2()
{
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	DBCOUNTITEM ulRowNum = 0;
	HRESULT hr = S_OK;

	// Note: SetParameterInfo called in init for insert statement with params
	
	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	MakeDataForCommand((ulRowNum = NextInsertRowNum()));
	// Now execute ICommand.

	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	TEST_COMPARE(cRow.FindRow(ulRowNum, m_pTable), TRUE);

	MakeDataForCommand((ulRowNum = NextInsertRowNum()));

	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	TEST_COMPARE(cRow.FindRow(ulRowNum, m_pTable), TRUE);
	fResult = TRUE;
CLEANUP:

	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	ReleaseDataForCommand();

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc SetParameterInfo,	Execute.  Use Rowset.  DescribeParams.
//		Over-rides parameter information from non-parameterized query
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_3()
{

	UINT i;
	WCHAR *pwszSqlStatement = NULL;
	ULONG	cDbParamBindInfo=0;
	DBPARAMBINDINFO *rgDbParamBindInfo = NULL;
	DB_UPARAMS *rgParamOrdinals = NULL;
	DBORDINAL *rgUpdateableCols = NULL;
	ULONG cUpdateableCols = 0;
	CCol	TempCol;
	void *pData = NULL;
	DBROWCOUNT cRowsAffected = 0;
	DBPARAMS DbParamsAll;	
	HACCESSOR hLocalAccessor = DB_NULL_HACCESSOR;
	DBBINDING *rgLocalBindings = NULL;
	DBCOLUMNINFO *rgColInfo = NULL;
	DBCOUNTITEM cBindings = 0;
	DBLENGTH cbRowSize = 0;
	DBORDINAL cColsOut = 0;
	HRESULT hr;
	BOOL fResult = TRUE;
	WCHAR *pwszSqlSelectAllFromTbl = NULL;
	DBCOUNTITEM ulRowNum = 0;
	DBORDINAL cTableCols = m_pTable->CountColumnsOnTable();
	ParamStruct * pParamStruct = NULL;
	IRowset * pRowset = NULL;

	// Remove any old parameter info
	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set command text 
	ABORT_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	if (m_pICommandPrepare)
		ABORT_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	// Set the parameter info if the provider requires it
	ABORT_CHECK(SetParameterInfoIfNeeded(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), S_OK);
	
	MakeDataForCommand((ulRowNum = NextInsertRowNum()));

	// Now execute ICommand.
	ABORT_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// No rowset to use.  IRowset with parameters not supported currently.
	ABORT_ALLOC(DB_UPARAMS, rgParamOrdinals, 0, (size_t)(cTableCols * sizeof(DB_UPARAMS)));
	ABORT_ALLOC(DBORDINAL, rgUpdateableCols, 0, (size_t)(cTableCols * sizeof(DBORDINAL)));
	ABORT_ALLOC(DBPARAMBINDINFO, rgDbParamBindInfo, 0, (size_t)(cTableCols * sizeof(DBPARAMBINDINFO)));
	ABORT_ALLOC(DBBINDING, rgLocalBindings, 0, (size_t)(cTableCols * sizeof(DBBINDING)));
	ABORT_ALLOC(ParamStruct, pParamStruct, 0, (size_t)(cTableCols * sizeof(ParamStruct)));

	//We'll use this count as the index to the array as we build it
	for (i = 1; i <= m_pTable->CountColumnsOnTable(); i++)
	{
		ABORT_CHECK(m_pTable->GetColInfo(i, TempCol), S_OK);

		//Record the column number in the array
		if (TempCol.GetUpdateable())
		{
			// Build Information for SetParameterInfo.
			rgUpdateableCols[cUpdateableCols++] = TempCol.GetColNum();  // Column number which is the next Updateable col.
			rgParamOrdinals[cDbParamBindInfo] = cDbParamBindInfo + 1;  // Parameter ordinal number.
			AddParam(cDbParamBindInfo++, i, DBPARAMIO_INPUT, NULL, FALSE, &cbRowSize, rgLocalBindings, rgDbParamBindInfo, pParamStruct, m_pTable);
		}
		
	}

	// Create the accessor
	ABORT_CHECK(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, cDbParamBindInfo, rgLocalBindings,
		cbRowSize, &hLocalAccessor, NULL), S_OK);

	//Replace text in CommandObject With select statment with no parameters.
	ABORT_CHECK(m_hr = m_pTable->CreateSQLStmt(SELECT_UPDATEABLE, NULL, &pwszSqlSelectAllFromTbl, NULL, NULL ), S_OK);
	ABORT_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , pwszSqlSelectAllFromTbl), S_OK);
	
	// Call ICommandPrepare.
	if (m_pICommandPrepare)
		ABORT_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	// Now override the previous settings and call SetParameterInfo.
	FAIL_CHECK(m_pICmdWParams->SetParameterInfo(cDbParamBindInfo, rgParamOrdinals, rgDbParamBindInfo), DB_S_TYPEINFOOVERRIDDEN);

	// Set command text for parameters.
	ABORT_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	if (m_pICommandPrepare)
		ABORT_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	// Now generate data for execute
	ABORT_CHECK(FillInputBindings((CTable *)m_pTable,
				DBACCESSOR_PARAMETERDATA, cDbParamBindInfo, rgLocalBindings,
				(BYTE **)&pData, (ulRowNum = NextInsertRowNum()) , cUpdateableCols, (DB_LORDINAL *)rgUpdateableCols), S_OK);

	DbParamsAll.cParamSets = 1;
	DbParamsAll.hAccessor = hLocalAccessor;
	DbParamsAll.pData = pData;

	ABORT_CHECK(m_pICommand->Execute(NULL, m_iidExec,
			&DbParamsAll, &cRowsAffected, (IUnknown **)&pRowset), S_OK);
	
	FAIL_COMPARE(cRowsAffected, 1);
	FAIL_COMPARE(pRowset, NULL);
	

CLEANUP:

	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	if (hLocalAccessor != DB_NULL_HACCESSOR)
		m_pCmdIAccessor->ReleaseAccessor(hLocalAccessor, NULL);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(rgParamOrdinals );
	PROVIDER_FREE(rgDbParamBindInfo) ;
	PROVIDER_FREE(pwszSqlSelectAllFromTbl);
	PROVIDER_FREE(rgUpdateableCols);
	PROVIDER_FREE(rgLocalBindings);
	PROVIDER_FREE(rgColInfo );
	PROVIDER_FREE(pParamStruct );

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc SetParameterInfo.  Remove SetParameterInfo.  GetParameterInfo. return original or fail
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_4()
{
	BOOL fResult = FALSE;
	HRESULT hr;
	LPWSTR wszTypeInfo[] = { L"DBTYPE_I1", L"DBTYPE_I2" };
	DBPARAMBINDINFO rgParamBindInfo[] = { { NULL, NULL,  0, 0, 0, 0 }, { NULL, NULL, 0, 0, 0, 0 } };
	DB_UPARAMS rgParamOrdinals[2] = { 1, 2 };

	rgParamBindInfo[0].pwszDataSourceType = wszTypeInfo[0];
	rgParamBindInfo[1].pwszDataSourceType = wszTypeInfo[1];

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Now override the previous settings and call SetParameterInfo.
	// Should return DB_S_TYPEINFOOVERRIDDEN because we called it once in Init.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo),DB_S_TYPEINFOOVERRIDDEN);
	
	FreeDescParams();
	TEST_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);
	
	CompareDBParamInfo(m_cDbParamBindInfo, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer);

	// Now remove SetParameterInfo.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo (0, NULL, NULL), S_OK);

	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	FreeDescParams();
	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);

	if (m_bSetParameterInfo)
	{
		TEST_COMPARE (m_cParams, 0);
		TEST_CHECK (hr, DB_E_PARAMUNAVAILABLE);
	}
	else
	{
		TEST_COMPARE (m_cParams, m_cDbParamBindInfo);
		TEST_CHECK (hr, S_OK);
	}
	

	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	
	if (m_bSetParameterInfo)
	{
		
		// Provider couldn't describe.
		// Since TypeInfo was removed we should be able to set.
			TEST_CHECK (hr, S_OK);
	}
	else
	{
		// Provider could describe but SetParameterInfo overrode.
		TEST_CHECK (hr, DB_S_TYPEINFOOVERRIDDEN);
	}

	// Try other combinations for removal.(0, Valid, NULL)
	// Now remove SetParameterInfo.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo (0, rgParamOrdinals, NULL), S_OK);

	FreeDescParams();
	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);

	if (m_bSetParameterInfo)
	{
		TEST_CHECK (hr, DB_E_PARAMUNAVAILABLE);
	}
	else
	{
		TEST_CHECK (hr, S_OK);
	}

	// TEST_COMPARE (m_cParams, 0);
	

	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (m_bSetParameterInfo)
	{
		// Since TypeInfo was removed we should be able to set.
		TEST_CHECK (hr, S_OK);
	}
	else
	{
		// Provider could describe but SetParameterInfo overrode.
		TEST_CHECK (hr, DB_S_TYPEINFOOVERRIDDEN);
	}

	// Try other combinations for removal.(0, NULL, Valid)
	// Now remove SetParameterInfo.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo (0, NULL, rgParamBindInfo), S_OK);

	FreeDescParams();
	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);
	
	if (m_bSetParameterInfo)
	{
		TEST_COMPARE (m_cParams, 0);
		TEST_CHECK (hr, DB_E_PARAMUNAVAILABLE);
	}
	else
	{
		TEST_CHECK (hr, S_OK);
	}

	// IN M08, we will have to prepare the statement here.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (m_bSetParameterInfo)
	{
		// Since TypeInfo was removed we should be able to set.
		TEST_CHECK (hr, S_OK);
	}
	else
	{
		// Provider could describe but SetParameterInfo overrode.
		TEST_CHECK (hr, DB_S_TYPEINFOOVERRIDDEN);
	}

	
	// Try other combinations for removal.(0, Valid, Valid)
	// Now remove SetParameterInfo.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo (0, rgParamOrdinals, rgParamBindInfo), S_OK);

	FreeDescParams();
	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);
	
	if (m_bSetParameterInfo)
	{
		TEST_COMPARE (m_cParams, 0);
		TEST_CHECK (hr, DB_E_PARAMUNAVAILABLE);
	}
	else
	{
		TEST_COMPARE (m_cParams, m_cDbParamBindInfo);
		TEST_CHECK (hr, S_OK);
	}

	// IN M08, we will have to prepare the statement here.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (m_bSetParameterInfo)
	{
		// Since TypeInfo was removed we should be able to set.
		TEST_CHECK (hr, S_OK);
	}
	else
	{
		// Provider could describe but SetParameterInfo overrode.
		TEST_CHECK (hr, DB_S_TYPEINFOOVERRIDDEN);
	}

	fResult = TRUE;
CLEANUP:

	FreeDescParams();
	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	
	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc SetParameterInfo for all.  Delete a few.  Override a few.  Call GetParameterInfo and verify.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_5()
{
	BOOL fResult = FALSE;
	DB_UPARAMS cParams = 0;
	DB_UPARAMS *rgParamOrdinals = NULL;
	HRESULT hr;
	DB_UPARAMS i;
	
	// Now override the previous settings and call SetParameterInfo.
	// Should return DB_S_TYPEINFOOVERRIDDEN because we called it once in Init.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo),DB_S_TYPEINFOOVERRIDDEN);
	
	FreeDescParams();
	TEST_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);
	
	CompareDBParamInfo(m_cDbParamBindInfo, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer);

	// Now remove SetParameterInfo.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo (0, NULL, NULL), S_OK);

	TEST_CHECK (m_pICmdWParams->SetParameterInfo (0, NULL, NULL), S_OK);

	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	FreeDescParams();
	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);
	
	if (m_bSetParameterInfo)
	{
		
		TEST_COMPARE (m_cParams, 0);
		TEST_CHECK (hr, DB_E_PARAMUNAVAILABLE);
	}
	else
	{
		// Only derived information is available.
		TEST_COMPARE (m_cParams, m_cDbParamBindInfo);
		TEST_CHECK (hr, S_OK);
		CompareDBParamInfo(m_cDbParamBindInfo, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer);
	}


	FreeDescParams();

	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
	{
		fResult = FALSE;
		goto CLEANUP;
	}
	
	// Now verify.
	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);
	
	TEST_CHECK (hr, S_OK);
	
	CompareDBParamInfo(m_cDbParamBindInfo, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer);

 	// Now lets prepare to remove the last half and verify.
	cParams = m_cDbParamBindInfo/2;
	rgParamOrdinals = (DB_UPARAMS *)m_pIMalloc->Alloc((m_cDbParamBindInfo - cParams)* sizeof(DB_UPARAMS));
	if (rgParamOrdinals == NULL)
	{
		odtLog << wszMemoryAllocationError;
		goto CLEANUP;
	}

	// Initialize.the ordinal array.
	for (i =cParams; i < m_cDbParamBindInfo; i++)
		rgParamOrdinals[i-cParams] = m_rgParamOrdinals[i];

	// Now remove the upper half of the params, leaving cParams left.  
	hr = m_pICmdWParams->SetParameterInfo((m_cDbParamBindInfo - cParams), rgParamOrdinals, NULL);
	if (hr != DB_S_TYPEINFOOVERRIDDEN)
	{
		if (hr != S_OK)
			goto CLEANUP;
	}
	
	FreeDescParams();
	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);
	TEST_CHECK (hr, S_OK);

	// Spec change: Providers that can derive parameter info are allowed to return information for all params.
	if (m_bSetParameterInfo)
		// Provider couldn't derive, so we expect some params to be removed
		CompareDBParamInfo(cParams, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer);
	else
	{
		// Provider can derive, params may be removed or may not be removed.  Derived information should be
		// available for params that were removed above if they are returned anyway.
		COMPARE(m_cParams == m_cDbParamBindInfo || m_cParams == cParams, TRUE);
		CompareDBParamInfo(m_cParams, m_rgParamColMap, m_cParams, m_rgParamInfo, m_pNamesBuffer);
	}
	
	fResult = TRUE;
CLEANUP:

	FreeDescParams();

	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	if (rgParamOrdinals)
		FREE_DATA(rgParamOrdinals);
	
	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc (valid, NULL, NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_6()
{

	ULONG cParams = 2;
	LPWSTR wszTypeInfo[] = { L"DBTYPE_I1", L"DBTYPE_I2" };
	HRESULT hr = S_OK;

	DBPARAMBINDINFO rgParamBindInfo[] = { { NULL, NULL,  0, 0, 0, 0 }, { NULL, NULL, 0, 0, 0, 0 } };
	BOOL fResult = FALSE;

	rgParamBindInfo[0].pwszDataSourceType = wszTypeInfo[0];
	rgParamBindInfo[1].pwszDataSourceType = wszTypeInfo[1];

	TEST_CHECK (m_pICmdWParams->SetParameterInfo (cParams, NULL, rgParamBindInfo), E_INVALIDARG);

	TEST_CHECK (m_pICmdWParams->SetParameterInfo (cParams, NULL, NULL), E_INVALIDARG);

	fResult = TRUE;
CLEANUP:

	
	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Bstr = NULL.  What happens?
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_7()
{
	
	ULONG cParams = 2;
	DB_UPARAMS rgParamOrdinals[2] = { 1, 2 };
	LPWSTR wszTypeInfo[] = { L"DBTYPE_I1", L"DBTYPE_I2" };
	DBPARAMBINDINFO rgParamBindInfo[] = { { NULL, NULL,  0, 0, 0, 0 }, { NULL, NULL, 0, 0, 0, 0 } };
	HRESULT hr;
	
	BOOL fResult = FALSE;
	// rgParamBindInfo[0].pwszDataSourceType is already initialized to NULL.
	rgParamBindInfo[1].pwszDataSourceType = wszTypeInfo[1];

	TEST_CHECK (m_pICmdWParams->SetParameterInfo (cParams, rgParamOrdinals, rgParamBindInfo), E_INVALIDARG);

	fResult = TRUE;
CLEANUP:

	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;


	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc SetParameterInfo with Illegal Coersions.  Execute should fail.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_8()
{
	HRESULT hr;
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	IRowset *pRowset = NULL;
	DB_UPARAMS *rgParamOrdinals = NULL;
	UINT i;
	BOOL fValidStatus = FALSE;
	ULONG iParam;


	// First Set type info incorrectly.
	// reverse the ordinal array. 
	rgParamOrdinals = (DB_UPARAMS *)m_pIMalloc->Alloc (m_cDbParamBindInfo*sizeof (DB_UPARAMS));
	
	if (m_cDbParamBindInfo > 1)
	{
		//swap;
		for (i = 0; i < m_cDbParamBindInfo; i++ )
			rgParamOrdinals[i] = m_rgParamOrdinals[m_cDbParamBindInfo - i -1];
	}
	else
	{
		// Only One column.  return TRUE
		return TEST_PASS;
	}

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	MakeDataForCommand(NextInsertRowNum());
	
	// Now override the previous settings and call SetParameterInfo.
	// Should return DB_S_TYPEINFOOVERRIDDEN because we called it once in Init.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, rgParamOrdinals, m_rgDbParamBindInfo),DB_S_TYPEINFOOVERRIDDEN);
	
	// Now execute ICommand.
	hr = m_pICommand->Execute(NULL, IID_IUnknown, &m_DbParamsAll, &cRowsAffected, (IUnknown **)&pRowset);

	if (FAILED(hr))
		// Since the row wasn't inserted, decrement next insert row number incremented above.
		m_cInsertRowNum--;

	switch (hr)
	{
		case DB_E_ERRORSOCCURRED:
			// Status value should be set to something informative, NOT DBSTATUS_E_BADACCESSOR
			// for at least one of the parameters.
			for (iParam=0; iParam < m_cBindings; iParam++)
				if (IsErrorStatus(STATUS_BINDING(m_rgBindings[iParam], m_pData)) && 
					STATUS_BINDING(m_rgBindings[iParam], m_pData) != DBSTATUS_E_BADACCESSOR)
					fValidStatus = TRUE;
			break;
		case DB_E_UNSUPPORTEDCONVERSION:
			// Status value should be set to DBSTATUS_E_BADACCESSOR for at least one of the parameters
			for (iParam=0; iParam < m_cBindings; iParam++)
				if (STATUS_BINDING(m_rgBindings[iParam], m_pData) == DBSTATUS_E_BADACCESSOR)
					fValidStatus = TRUE;
			break;
		default:
			TEST_CHECK(hr, DB_E_ERRORSOCCURRED);

	}

	TEST_COMPARE(fValidStatus, TRUE);

	fResult = TRUE;

CLEANUP:
	
	FreeDescParams();
	ReleaseDataForCommand();
	ReleaseRowsetPtr(&pRowset);
	FREE_DATA (rgParamOrdinals);
	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;


	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc rgParamOrdinals[] = ULONG_MAX.  what happens.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_9()
{
	ULONG cParams = 2;
	DB_UPARAMS ulNumParams=0;
	HRESULT hr = E_FAIL, hrSet = E_FAIL;
	DB_UPARAMS rgParamOrdinals[2] = { 1, ULONG_MAX };
	LPWSTR wszTypeInfo[] = { L"DBTYPE_I1", L"DBTYPE_I2" };
	WCHAR *pwszSqlSelectAllFromTbl=NULL;

	DBPARAMBINDINFO rgParamBindInfo[] = { { NULL, NULL,  0, 0, 0, 0 }, { NULL, NULL, 0, 0, 0, 0 } };
	BOOL fResult = FALSE;

	rgParamBindInfo[0].pwszDataSourceType = wszTypeInfo[0];
	rgParamBindInfo[1].pwszDataSourceType = wszTypeInfo[1];

	// Set command text 
	FAIL_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);

	// Make sure we're prepared in case a previous variation failed the prepare
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	// First remove all.
	hr = m_pICmdWParams->SetParameterInfo(0, NULL, NULL);

	// Verify.
	FreeDescParams();
	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);
	
	if (m_bSetParameterInfo)
	{
		// Since we delete all we should get Unavailable;
		TEST_CHECK (hr, DB_E_PARAMUNAVAILABLE);
		TEST_COMPARE(m_cParams, 0);
		TEST_COMPARE(m_rgParamInfo, NULL);
		TEST_COMPARE(m_pNamesBuffer, NULL);
	}
	else
	{
		TEST_CHECK (hr, S_OK);
		ulNumParams = m_cParams;
	}
	
	// Now unprepare and delete command text to keep the provider from knowing ULONG_MAX is not a
	// valid ordinal if they can derive
	if (! CHECK (m_pICommandPrepare->Unprepare(), S_OK))
			goto CLEANUP;

	// Set command text to none
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , NULL), S_OK);

	// Now set the ULONG_MAX ordinal.  We're mainly concerned the provider has some
	// valid behavior for this case, doesn't crash or return an unexpected error.
	// Note 1: Some providers allocate memory for each parameter up to the max (ULONG_MAX)
	//		and will run out of memory, which is not really a failure, so pass that behavior
	// Note 2: Some providers don't allow ULONG_MAX iOrdinals and have an internal limit.  In
	//		that case they will return an error.  Again, this is valid behavior.
	hrSet = m_pICmdWParams->SetParameterInfo (cParams, rgParamOrdinals, rgParamBindInfo);

	switch(hrSet)
	{
		case DB_S_TYPEINFOOVERRIDDEN:
		case E_OUTOFMEMORY:
		case DB_E_ERRORSOCCURRED:
			break;
		default:
			TEST_CHECK(hrSet, S_OK);
	}

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);

	// Prepare again
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	// Verify with get parameter info.
	FreeDescParams();

	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);

	// If the set was successful
	if (SUCCEEDED(hrSet))
	{
		TEST_CHECK(hr, S_OK);

		TEST_COMPARE (m_cParams, cParams);

		// Verify 2nd ordinal is ULongMax.
		TEST_COMPARE (m_rgParamInfo[cParams-1].iOrdinal, ULONG_MAX);
	}
	// If set failed but the provider can derive ULONG_MAX was not added
	else if (!m_bSetParameterInfo)
	{
		TEST_CHECK(hr, S_OK);
		TEST_COMPARE (m_cParams, ulNumParams);
		TEST_COMPARE (m_rgParamInfo[ulNumParams-1].iOrdinal != ULONG_MAX, TRUE);
	}
	// Set failed and provider can't derive
	else 
	{
		TEST_CHECK (hr, DB_E_PARAMUNAVAILABLE);
		TEST_COMPARE(m_cParams, 0);
		TEST_COMPARE(m_rgParamInfo, NULL);
		TEST_COMPARE(m_pNamesBuffer, NULL);
	}

	
	fResult = TRUE;

CLEANUP:

	FreeDescParams();

	// Set command text 
	FAIL_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);

	// RESET THE TYPEINFO correctly.
	FAIL_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Call execute with params before SetParameterInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_10()
{
	ICommand *pICommand = NULL;
	ICommandPrepare *pICommandPrep = NULL;
	IAccessor *pLocalIAccessor = NULL;
	ICommandWithParameters *pICmdWPar = NULL;
	BOOL fResult = FALSE;
	HACCESSOR hLocalAccessor = DB_NULL_HACCESSOR;
	void *pData = NULL;
	DBPARAMS ExecDbParams;
	DBROWCOUNT cRowsAffected;
	IRowset * pRowset = NULL;
	DBCOUNTITEM ulRowNum = 0;
	HRESULT hr = S_OK;
	//  Command to return a ICommand with Text Set
	if( !CHECK(m_pTable->BuildCommand(m_pwszSqlInsertAllWithParams,		// SQL STMT
			IID_ICommand,		// IID  no more that IID_ICommand is needed.
			EXECUTE_NEVER,		// EXECUTE
			NULL,				// # Prop's
			NULL,				// Prop's
			NULL,				// Params
			NULL,				// # Rowset
			NULL,				// Rowsets
			&pICommand),		// ICommand
			S_OK) )
		goto CLEANUP;

	// QI for ICommandPrepare
	if (!VerifyInterface (pICommand, IID_ICommandPrepare, COMMAND_INTERFACE, 	(IUnknown **)&pICommandPrep)) 
		goto CLEANUP;
	
	// Verify The Interface for a Command Accessor Object.
	if (!VerifyInterface(pICommand, IID_IAccessor, COMMAND_INTERFACE,(IUnknown **)&pLocalIAccessor))
	{
		//ICommandAccessor is not supported
		goto CLEANUP;
	}

	// We might need to set the property for this or we need not.  REVISIT and CHECK.
	if (!VerifyInterface(pICommand, IID_ICommandWithParameters, COMMAND_INTERFACE, (IUnknown **)&pICmdWPar))
		goto CLEANUP;

	// 1. Create accessor.
	// Get accessor.
	TEST_CHECK (pLocalIAccessor->CreateAccessor ( DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings, 
		m_cbRowSize, &hLocalAccessor, NULL), S_OK);

	TEST_CHECK (FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, m_cBindings, m_rgBindings,
		(BYTE **)&pData, (ulRowNum = NextInsertRowNum()), m_cParamColMap, (DB_LORDINAL *)m_rgParamColMap), S_OK);

	ExecDbParams.cParamSets = 1;
	ExecDbParams.hAccessor = hLocalAccessor;
	ExecDbParams.pData = pData;
	//ExecDbParams.cbParamSetSize = 0;

	// Now call Execute. 
	// Since SetParameterInfo is not called, execute should try to coerce the paraemters based
	// on accessor type and succeed.  Should return S_OK.
	TEST_CHECK(pICommand->Execute(NULL, IID_IUnknown, &ExecDbParams, 
		&cRowsAffected, (IUnknown **)&pRowset), S_OK);


	fResult = TRUE;
CLEANUP:
	
	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	ReleaseInputBindingsMemory(m_cBindings, m_rgBindings, (BYTE *)ExecDbParams.pData, TRUE);

	// Release Objects
	pLocalIAccessor->ReleaseAccessor(hLocalAccessor, NULL);

	RELEASE (pLocalIAccessor);
	RELEASE( pICommandPrep );
	RELEASE( pICmdWPar);
	RELEASE( pICommand );

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Wrong SetParameterInfo.  Verify DP returns the same.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_11()
{

	BOOL fResult = FALSE;
	HRESULT hr;
	UINT i =0;
	DBPARAMBINDINFO * pParamBindInfo = NULL;

	//  Delete all the information.
	TEST_CHECK(m_pICmdWParams->SetParameterInfo (0, NULL, NULL), S_OK);

	// Make a copy of the "correct" info
	TEST_ALLOC(DBPARAMBINDINFO, pParamBindInfo, 0, (size_t)(m_cBindings * sizeof(DBPARAMBINDINFO)));
	memcpy(pParamBindInfo, m_rgDbParamBindInfo, (size_t)(m_cBindings *  sizeof(DBPARAMBINDINFO)));

	// Make it incorrect
	ReverseArray(pParamBindInfo, m_cBindings, sizeof(DBPARAMBINDINFO));
	
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);

	if (m_pICommandPrepare)
		TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	// Set it with some type so that we can verify back again.
	hr = m_pICmdWParams->SetParameterInfo (m_cBindings, m_rgParamOrdinals, pParamBindInfo);
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		goto CLEANUP;

	// Compare contents. with set values.
	// First call GetParameterInfo.
	FreeDescParams();
	TEST_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

	// Compare GetParameterInfo.
	TEST_CHECK(VerifyParamInfo(m_cBindings, m_rgParamOrdinals, pParamBindInfo, m_cParams,
		m_rgParamInfo, m_pNamesBuffer), S_OK);

	fResult = TRUE;	

CLEANUP:
		// Set command text 
	FreeDescParams();
	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Delete first parameter, last parameter.  Verify.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_12()
{
	
	ULONG cDeleteParams = 2;
	DB_UPARAMS rgDeleteOrdinals[2];
	BOOL fResult = FALSE;
	HRESULT hr;

	//  Delete all the information.
	TEST_CHECK(m_pICmdWParams->SetParameterInfo (0, NULL, NULL), S_OK);

	// Prepare if needed
	if (m_pICommandPrepare)
		TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	// Set it with some type so that we can verify back again.
	hr = m_pICmdWParams->SetParameterInfo (m_cBindings, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		goto CLEANUP;

	// Set the delete ordinals
	rgDeleteOrdinals[0] = m_rgParamOrdinals[0];
	rgDeleteOrdinals[1] = m_rgParamOrdinals[m_cBindings-1];

	// Delete the First and last.  Only the 2nd should remain.
	hr = m_pICmdWParams->SetParameterInfo(cDeleteParams, rgDeleteOrdinals, NULL);
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		goto CLEANUP;


	// Compare contents. with set values.
	// First call GetParameterInfo.
	FreeDescParams();
	TEST_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

	TEST_CHECK(VerifyParamInfo(m_cBindings-cDeleteParams, &m_rgParamOrdinals[1], &m_rgDbParamBindInfo[1], m_cParams,
		m_rgParamInfo, m_pNamesBuffer, 1), S_OK);

	fResult = TRUE;	

CLEANUP:
	FreeDescParams();

	// Set command text 
	if (!CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK))
		fResult = FALSE;

	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Call SetParameterInfo for Output Parameters.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_13()
{
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSet = S_OK;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	if (fCanDerive)
		hrSet = DB_S_TYPEINFOOVERRIDDEN;

	// Call SetParameterInfo without having called GetParameterInfo first
	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), hrSet);

	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		0, NULL, VERIFY_USE_TABLE, TRUE), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Call SetParameterInfo for Input/Output parameters.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_14()
{
	BOOL fResult = TRUE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	ULONG cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HRESULT hrSet = S_OK;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TEST_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_INOUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		1,						// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		1,						// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll
	), TRUE);

	// Set up to execute the stored proc
	ABORT_CHECK(PrepareForExecute(pwszExecProcStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, pwszProcName, pwszCreateProcStmt), S_OK);

	if (fCanDerive)
		hrSet = DB_S_TYPEINFOOVERRIDDEN;

	// Call SetParameterInfo without having called GetParameterInfo first
	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND), hrSet);

	FAIL_VAR(ExecuteAndVerify(cParams, cParamSets, pParamAll,ulRowNum, pBINDING, cbRowSize, pData, ROWSET_NONE, 
		0, NULL, VERIFY_USE_PDATA, TRUE), S_OK);

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Call SetParameterInfo for All DBTYPES in the SPEC and Verify.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_15()
{

	BOOL fResult = TRUE;
	HRESULT hr;
	ULONG iStdType;
	DB_UPARAMS rgParamOrdinals[1] = {1};

	//  Delete all the information. DELETING Should not return DB_S_TYPEINFOOVERRIDDEN
	TEST_CHECK(m_pICmdWParams->SetParameterInfo (0, NULL, NULL), S_OK);

	TEST_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);

	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	// For each standard type in the list attempt to set parameter information
	for (iStdType =0; iStdType < g_cStdParams; iStdType++)
	{
		// Set the DBPARAMBINDINFO pointer to the proper type name
		g_rgStdParamBindInfo[iStdType].rgParamBindInfo[0].pwszDataSourceType = g_rgStdParamBindInfo[iStdType].wszStdTypeName;
	
		//  Delete all the information. DELETING Should not return DB_S_TYPEINFOOVERRIDDEN
		TEST_CHECK(m_pICmdWParams->SetParameterInfo (0, NULL, NULL), S_OK);

		// Set it with the type so that we can verify back again.
		hr = m_pICmdWParams->SetParameterInfo (1, rgParamOrdinals, g_rgStdParamBindInfo[iStdType].rgParamBindInfo);

		// If we didn't succeed setting the name it should be because the type name wasn't supported
		if (!SUCCEEDED(hr))
		{
			fResult &= CHECK(hr, DB_E_BADTYPENAME);
			odtLog << L"Couldn't set standard type name for type: " << g_rgStdParamBindInfo[iStdType].wszStdTypeName << "\n\n";

			// TODO: If the name wasn't set verify it's not a type in the provider types rowset
		}
		else
		{
			// Providers that can derive type information may return DB_S_TYPEINFOOVERRIDDEN here
			if (!m_bSetParameterInfo)
				CHECK(hr, DB_S_TYPEINFOOVERRIDDEN);
			else
				// Since we deleted parameter information above this should never be DB_S_TYPEINFOOVERRIDDEN
				fResult &= CHECK(hr, S_OK);

			// Compare contents. with set values.
			// First call GetParameterInfo.
			FreeDescParams();
			TEST_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

			// Per spec it's valid for providers to return the "best fit" type in GetParameterInfo.  So if
			// the types don't match it's not a failure, and we can't verify precision, scale, etc.
			if (m_rgParamInfo [0].wType ==  g_rgStdParamBindInfo[iStdType].wType)
			{
				fResult &= COMPARE (m_rgParamInfo [0].bPrecision, g_rgStdParamBindInfo[iStdType].rgParamBindInfo[0].bPrecision);
				fResult &= COMPARE (m_rgParamInfo [0].ulParamSize, g_rgStdParamBindInfo[iStdType].rgParamBindInfo[0].ulParamSize);
				fResult &= COMPARE (m_rgParamInfo [0].bScale, g_rgStdParamBindInfo[iStdType].rgParamBindInfo[0].bScale);
			}
			else
			{
				odtLog << L"Warning: Type identifier didn't match for type " << g_rgStdParamBindInfo[iStdType].wszStdTypeName 
					<< L"\n";
				odtLog << L"Skipping comparison of ulParamSize, bPrecision, and bScale.\n";
				odtLog << L"Per spec this is not a failure because providers are allowed to return the 'best fit' type.\n\n";
			}

			// But we expect the flags to match regardless
			fResult &= COMPARE (m_rgParamInfo [0].dwFlags, g_rgStdParamBindInfo[iStdType].rgParamBindInfo[0].dwFlags);

			// TODO: Actually execute with standard type names set and verify data is retrieved properly.
		}
	}


CLEANUP:

	// Set command text again
	if (!CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK))
		fResult = FALSE;
	FreeDescParams();
	// RESET THE TYPEINFO correctly.
	CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Call SetParameterInfo for Native types and Verify.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_16()
{
	ULONG cParams = 1;
	HRESULT hr;
	DB_UPARAMS rgParamOrdinals[1] = { 1 };
	LPWSTR wszTypeInfo[] = { L"bit"};
	DBTYPE dbTypeValues[] = { DBTYPE_BOOL };
	WCHAR *pwszSqlSelectAllFromTbl=NULL;

	odtLog << L"This variation only duplicates more complete testing done in many other variations.\n";
	return TEST_SKIPPED;


	DBPARAMBINDINFO rgParamBindInfo[] = { { NULL, NULL,  0, 0, 0, 0 } };
	BOOL fResult = FALSE;

	rgParamBindInfo[0].pwszDataSourceType = wszTypeInfo[0];
	
	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);

	// Prepare
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	// First remove all.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Now set the Native types.
	hr = m_pICmdWParams->SetParameterInfo (cParams, rgParamOrdinals, rgParamBindInfo);
	if (hr != DB_S_TYPEINFOOVERRIDDEN)
	{
		if (hr != S_OK)
			goto CLEANUP;
	}

	// Verify with get parameter info.
	FreeDescParams();
	
	hr = m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer);
	TEST_CHECK (hr, S_OK);
	TEST_COMPARE (m_cParams, 1);
	
	fResult = TRUE;

CLEANUP:
	FreeDescParams();

	// Set command text 
	if (!CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK))
		fResult = FALSE;
	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Open Rowset , and then call SetParameterInfo returns DB_E_OBJECTOPEN.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_17()
{
	BOOL fResult = FALSE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 1;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	HACCESSOR hParamAccessor = DB_NULL_HACCESSOR;
	DBPARAMS ExecParams;
	IRowset * pIRowset = NULL;
	WCHAR * pwszStmt = NULL;
	DBROWCOUNT cRowsAffected=0;

	// Note: This variation previously used a stored proc, so it didn't work on providers that didn't
	// support stored procs.  Rewritten to use statement.

	ABORT_COMPARE(CreateProcBindings(
		T_EXEC_PROC_SELECT_IN,// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd
	), TRUE);

	// Set up to execute the statement
	ABORT_CHECK(PrepareForExecute(pwszExecStmt, cParams, pParamOrdinals, pPARAMBIND, 
		&fCanDerive, NULL, NULL), S_OK);

	// Create the parameter accessor
	ABORT_CHECK (m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, cParams,
		pBINDING, cbRowSize, &hParamAccessor, NULL), S_OK);

	// Set the parameter info properly. This might return DB_S_TYPEINFOOVERRIDDEN or S_OK.
	if (!fCanDerive)
		ABORT_COMPARE(FAILED(m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, pPARAMBIND)), FALSE);
	
	ExecParams.hAccessor = hParamAccessor;
	ExecParams.cParamSets = 1;
	ExecParams.pData = pData;

	// Execute the command, opening a rowset
	ABORT_CHECK(m_pICommand->Execute(NULL, m_iidExec, &ExecParams, &cRowsAffected, (IUnknown **)&pIRowset), S_OK);
	
	ABORT_CHECK (m_pICmdWParams->SetParameterInfo(cParams, pParamOrdinals, 
			pPARAMBIND), DB_E_OBJECTOPEN);

	// If we got here then success
	fResult = TRUE;

CLEANUP:

	if (hParamAccessor != DB_NULL_HACCESSOR)
		m_pCmdIAccessor->ReleaseAccessor(hParamAccessor, NULL);
	SAFE_RELEASE(pIRowset);

	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Set parameter for variable length char fields and execute
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_18()
{
	BOOL fResult = FALSE;

	// Regression case for 3146 (oledb).

	fResult = ExecuteWithSetParamInfo(L"DBTYPE_LONGVARCHAR", DBTYPE_STR);
	fResult = ExecuteWithSetParamInfo(L"DBTYPE_VARCHAR", DBTYPE_STR);
	fResult = ExecuteWithSetParamInfo(L"DBTYPE_CHAR", DBTYPE_STR);

	fResult = ExecuteWithSetParamInfo(L"DBTYPE_LONGVARBINARY", DBTYPE_BYTES);
	fResult = ExecuteWithSetParamInfo(L"DBTYPE_VARBINARY", DBTYPE_BYTES);
	fResult = ExecuteWithSetParamInfo(L"DBTYPE_BINARY", DBTYPE_BYTES);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc SetParameterInfo with reverse rgParamOrdinals
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_19()
{
	BOOL fResult = FALSE;
	HRESULT hr = E_FAIL;

	TEST_CHECK (m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Reset command text since another variation may have altered it.
	TEST_CHECK(m_pTable->ExecuteCommand(INSERT_ALLWITHPARAMS, IID_IUnknown, NULL, 
		NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &m_pICommand), S_OK);

	// Prepare if supported
	if (m_pICommandPrepare)
		TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	// Populate the name value in m_rgDbParamBindInfo
	SetParameterNames(m_pwszParameterNames);

	ReverseArray(m_rgParamOrdinals, m_cDbParamBindInfo, sizeof(DB_UPARAMS));
	ReverseArray(m_rgDbParamBindInfo, m_cDbParamBindInfo, sizeof(DBPARAMBINDINFO));

	// Now set the reversed parameter info
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);

	if (hr != S_OK && hr != DB_S_TYPEINFOOVERRIDDEN)
		TEST_CHECK(hr, S_OK);

	
	TEST_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

	TEST_CHECK(VerifyParamInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo, m_cParams,
		m_rgParamInfo, m_pNamesBuffer), S_OK);

	fResult = TRUE;
CLEANUP:

	FreeDescParams();

	// Reset everything  
	ReverseArray(m_rgParamOrdinals, m_cDbParamBindInfo, sizeof(DB_UPARAMS));
	ReverseArray(m_rgDbParamBindInfo, m_cDbParamBindInfo, sizeof(DBPARAMBINDINFO));

	SetParameterNames(NULL);

	CHECK(m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), DB_S_TYPEINFOOVERRIDDEN);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc SetParameterInfo with random rgParamOrdinals
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_20()
{
	BOOL fResult = FALSE;

	// Populate the name value in m_rgDbParamBindInfo
	SetParameterNames(m_pwszParameterNames);

	ScrambleArray(m_rgParamOrdinals, m_cDbParamBindInfo, sizeof(DB_UPARAMS));
	ScrambleArray(m_rgDbParamBindInfo, m_cDbParamBindInfo, sizeof(DBPARAMBINDINFO));
	
	// Now override the previous settings and call SetParameterInfo.
	// Will return DB_S_TYPEINFOOVERRIDDEN because we called it once in Init.
	TEST_CHECK (m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo),DB_S_TYPEINFOOVERRIDDEN);
	
	TEST_CHECK (m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);

	TEST_CHECK(VerifyParamInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo, m_cParams,
		m_rgParamInfo, m_pNamesBuffer), S_OK);

	fResult = TRUE;
CLEANUP:

	FreeDescParams();

	// Reset everything  
	ScrambleArray(m_rgParamOrdinals, m_cDbParamBindInfo, sizeof(DB_UPARAMS));
	ScrambleArray(m_rgDbParamBindInfo, m_cDbParamBindInfo, sizeof(DBPARAMBINDINFO));

	// Populate the name value in m_rgDbParamBindInfo
	SetParameterNames(NULL);
	CHECK(m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), DB_S_TYPEINFOOVERRIDDEN);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc SetParameterInfo with duplicate parameter name
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_21()
{
	BOOL fResult = TRUE;
	ULONG idxDupSource=0;	// First parameter is the source of the duplicate name
	DB_UPARAMS idxDupTarget=m_cDbParamBindInfo-1;	// Last parameter is the duplicate
	LPOLESTR pwszSavedName;
	OLECHAR wszDupName[SP_MAX_PARAMNAME_LENGTH + 1];
	HRESULT hr;

	// Populate the name value in m_rgDbParamBindInfo
	SetParameterNames(m_pwszParameterNames);
	pwszSavedName=m_rgDbParamBindInfo[idxDupTarget].pwszName;

	// There are two ways to duplicate a parameter name: 1) Pointer is to same name; 2) Pointer is different, but buffer
	// contains the same name.  Both are errors.
	
	// Duplicate pointer
	m_rgDbParamBindInfo[idxDupTarget].pwszName=m_rgDbParamBindInfo[idxDupSource].pwszName;
	hr=m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (DB_E_BADPARAMETERNAME != hr)
	{
		if (!CHECK (hr,DB_S_TYPEINFOOVERRIDDEN))
			fResult=FALSE;
	}
	
	// Duplicate buffer
	wcscpy(wszDupName, m_rgDbParamBindInfo[idxDupSource].pwszName);
	m_rgDbParamBindInfo[idxDupTarget].pwszName=(LPOLESTR)&wszDupName;
	hr=m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (DB_E_BADPARAMETERNAME != hr)
	{
		if (!CHECK (hr,DB_S_TYPEINFOOVERRIDDEN))
			fResult=FALSE;
	}
	
	// Reset everything  
	SetParameterNames(NULL);
	CHECK(m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), DB_S_TYPEINFOOVERRIDDEN);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc SetParameterInfo with invalid parameter name
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetParameterInfo_Rowset::Variation_22()
{

	BOOL fResult = TRUE;
	DB_UPARAMS idxBadParam=m_cDbParamBindInfo-1;	// We'll use the last parameter
	LPOLESTR pwszSavedName;
	OLECHAR wszBadName[SP_MAX_PARAMNAME_LENGTH + 1];
	HRESULT hr;

	// Some providers might not be able to validate parameter names, so they could return S_OK instead of DB_E_BADPARAMETERNAME.
	SetParameterNames(m_pwszParameterNames);
	pwszSavedName=m_rgDbParamBindInfo[idxBadParam].pwszName;

	// Set one of the parameters to NULL.  It's an error to have one unnamed parameter.
	m_rgDbParamBindInfo[idxBadParam].pwszName=NULL;
	hr=m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (DB_E_BADPARAMETERNAME != hr)
	{
		if (!CHECK (hr,DB_S_TYPEINFOOVERRIDDEN))
			fResult=FALSE;
	}
	
	// Set one of the parameters to an empty string.  I'm assuming this isn't a valid name.
	wcscpy(wszBadName, L"");
	m_rgDbParamBindInfo[idxBadParam].pwszName=(LPOLESTR)&wszBadName;
	hr=m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (DB_E_BADPARAMETERNAME != hr)
	{
		if (!CHECK (hr,DB_S_TYPEINFOOVERRIDDEN))
			fResult=FALSE;
	}

	// Set one of the parameters to a value containing a control character. I'm assuming this isn't a valid name.
	wcscpy(wszBadName, pwszSavedName);
	memset(wszBadName, 1, 1);	// Set first character to Control-A
	hr=m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	if (DB_E_BADPARAMETERNAME != hr)
	{
		if (!CHECK (hr,DB_S_TYPEINFOOVERRIDDEN))
			fResult=FALSE;
	}

	// Reset everything 
	SetParameterNames(NULL);
	CHECK(m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo), DB_S_TYPEINFOOVERRIDDEN);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Default Conversion: pwszDataSourceType == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetParameterInfo_Rowset::Variation_23()
{ 

	UINT i;
	DBCOUNTITEM ulRowNum;
	DBPARAMBINDINFO *rgDbParamBindInfo = NULL;
	DBROWCOUNT cRowsAffected = 0;
	HRESULT hr;
	BOOL fResult = TEST_FAIL;
	IRowset * pRowset = NULL;

	// Remove any old parameter info
	ABORT_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set command text 
	ABORT_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);

	// Make a copy of the parameter info
	SAFE_ALLOC(rgDbParamBindInfo, DBPARAMBINDINFO, m_cDbParamBindInfo * sizeof(DBPARAMBINDINFO));
	memcpy(rgDbParamBindInfo, m_rgDbParamBindInfo,  (size_t)(m_cDbParamBindInfo * sizeof(DBPARAMBINDINFO)));
	
	// Set all pwszDataSourceType's to NULL.  This will return E_INVALIDARG if provider doesn't support
	// default conversions.
	for (i=0; i < m_cDbParamBindInfo; i++)
		rgDbParamBindInfo[i].pwszDataSourceType = NULL;

	// Set the parameter info 
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, rgDbParamBindInfo);

	if (E_INVALIDARG == hr)
	{
		odtLog << L"This provider doesn't support default parameter conversions.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	// If the provider can derive we'll get DB_S_TYPEINFOOVERRIDDEN
	if (DB_S_TYPEINFOOVERRIDDEN != hr)
		ABORT_CHECK(hr, S_OK);

	// Call ICommandPrepare.
	if (m_pICommandPrepare)
		ABORT_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	MakeDataForCommand((ulRowNum = NextInsertRowNum()));

	// Now execute ICommand.
	ABORT_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	
	FAIL_COMPARE(cRowsAffected, 1);
	FAIL_COMPARE(pRowset, NULL);

	// Make sure the row was inserted.  This provides data validation for updatable non-LONG columns
	TEST_COMPARE(cRow.FindRow(ulRowNum, m_pTable), TRUE);

	// Currently no providers actually support this feature, all return E_INVALIDARG above.

	// TODO: Validate data for row so LONG columns are verified.

	// TODO: Call GetParameterInfo and verify valid type names are returned.

	fResult = TEST_PASS;

CLEANUP:

	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult = FALSE;

	ReleaseDataForCommand();
	PROVIDER_FREE(rgDbParamBindInfo) ;

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Verify error on DBTYPE_NUMERIC with no precision
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetParameterInfo_Rowset::Variation_24()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSetParameterInfo_Rowset::Terminate()
{
	HRESULT hr;
	
	// RESET THE TYPEINFO correctly.
	if (m_pICmdWParams)
	{
		// We might have set typeinfo above or not depending upon where we jump from.
		hr=m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
		if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		{
			// Free parameter names array
			FreeParameterNames(m_pwszParameterNames);
			m_pwszParameterNames=NULL;

			CICmdWParams::Terminate();
			return FALSE;
		}
	}

	// Free parameter names array
	FreeParameterNames(m_pwszParameterNames);
	m_pwszParameterNames=NULL;

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CICmdWParams::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:			07/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL TCExtendedErrors::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CICmdWParams::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid GetParameterInfo calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	BOOL fResult = FALSE;
	HRESULT hr;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ICommandWithParameters method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
	
	// Set command text
	TEST_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK(m_pICommandPrepare->Prepare(1), S_OK);

	//create an error object
	m_pExtError->CauseError();

	TEST_CHECK(hr=m_pICmdWParams->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer), S_OK);
		
	//do extended error check following the GetParameterInfo
	fResult = XCHECK(m_pICmdWParams, IID_ICommandWithParameters, hr);	


CLEANUP:
	FreeDescParams();
	m_pICommandPrepare->Unprepare();

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetParameterInfo calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	HRESULT		hr;							
	BOOL		fResult	= FALSE;

 	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ICommandWithParameters method.
	//We then check extended errors to verify the right extended error behavior.
	
	FreeDescParams();

	//create an error object
	m_pExtError->CauseError();
  
	if (CHECK(hr=m_pICmdWParams->GetParameterInfo(NULL, NULL, &m_pNamesBuffer),
		E_INVALIDARG))
		//Do extended check following GetParameterInfo
		fResult = XCHECK(m_pICmdWParams, IID_ICommandWithParameters, hr);	

	FreeDescParams();		
	m_pICommandPrepare->Unprepare();

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetParameterInfo calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	HRESULT hr;
	BOOL fResult = FALSE;
	ICommandWithParameters *pICommandWithParameters = NULL;
	
	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the ICommandWithParameters method.
	//We then check extended errors to verify the right extended error behavior.
	
	if (!VerifyInterface(m_pIEmptyCommand, IID_ICommandWithParameters, COMMAND_INTERFACE, (IUnknown **)&pICommandWithParameters))
	{
		goto CLEANUP;
	}

	
	if (m_bSetParameterInfo)
	{
		TEST_CHECK(hr=pICommandWithParameters->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer),
			DB_E_PARAMUNAVAILABLE);
	}
	else
	{
		TEST_CHECK(hr=pICommandWithParameters->GetParameterInfo(&m_cParams, &m_rgParamInfo, &m_pNamesBuffer),
			DB_E_NOTPREPARED);
	}
	//Do extended check following GetParameterInfo
	fResult = XCHECK(m_pICmdWParams, IID_ICommandWithParameters, hr);	

CLEANUP:
	if (pICommandWithParameters)
		pICommandWithParameters->Release();
	m_pICommandPrepare->Unprepare();

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid SetParameterInfo calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_4()
{
	BOOL fResult = FALSE;
	HRESULT hr;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ICommandWithParameters method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.
	
	if (!m_bSetParameterInfo)
	{
		// IF SetParameterInfo is not set.  set it anyway.  for testing SetParameterInfo.
		if (! CHECK (m_pICommandPrepare->Prepare(1), S_OK))
			goto CLEANUP;
		hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
		if (hr != DB_S_TYPEINFOOVERRIDDEN)
			TEST_CHECK(hr, S_OK);
	}

	FreeDescParams();

	//create an error object	
	m_pExtError->CauseError();

	// Now remove SetParameterInfo.
	TEST_CHECK (hr=m_pICmdWParams->SetParameterInfo (0, NULL, NULL), S_OK);
	
	//Do extended check following SetParameterInfo
	fResult = XCHECK(m_pICmdWParams, IID_ICommandWithParameters, hr);	

	fResult = TRUE;
CLEANUP:

	FreeDescParams();
	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		fResult &= FALSE;

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
//}}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Invalid SetParameterInfo calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_5()
{
	DBPARAMBINDINFO rgParamBindInfo[] = { { NULL, NULL,  0, 0, 0, 0 }, { NULL, NULL, 0, 0, 0, 0 } };
	BOOL fResult = FALSE;
	HRESULT hr;
   	LPWSTR wszTypeInfo[] = { L"DBTYPE_I1", L"DBTYPE_I2" };
	ULONG cParams = 2;

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ICommandWithParameters method.
	//We then check extended errors to verify the right extended error behavior.
	
	rgParamBindInfo[0].pwszDataSourceType = wszTypeInfo[0];
	rgParamBindInfo[1].pwszDataSourceType = wszTypeInfo[1];

	//create an error object
	m_pExtError->CauseError();
  	
	TEST_CHECK (hr=m_pICmdWParams->SetParameterInfo (cParams, NULL, rgParamBindInfo), E_INVALIDARG);
	//Do extended check following SetParameterInfo
	fResult = XCHECK(m_pICmdWParams, IID_ICommandWithParameters, hr);	
	
	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (hr != DB_S_TYPEINFOOVERRIDDEN)
	{
		if (!CHECK(hr, S_OK))
			fResult &= FALSE;
	}

CLEANUP:
	FreeDescParams();		

	return (fResult) ? TEST_PASS : TEST_FAIL;
}

// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid SetParameterInfo calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_6()
{
	DBPARAMBINDINFO rgParamBindInfo[] = { { NULL, NULL,  0, 0, 0, 0 }, { NULL, NULL, 0, 0, 0, 0 } };
	BOOL fResult = FALSE;
	HRESULT hr;
   	LPWSTR wszTypeInfo[] = { L"DBTYPE_I1", L"DBTYPE_I2" };
	ULONG cParams = 2;

	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the ICommandWithParameters method.
	//We then check extended errors to verify the right extended error behavior.
	
	rgParamBindInfo[0].pwszDataSourceType = wszTypeInfo[0];
	rgParamBindInfo[1].pwszDataSourceType = wszTypeInfo[1];

	TEST_CHECK (hr=m_pICmdWParams->SetParameterInfo (cParams, NULL, rgParamBindInfo), E_INVALIDARG);
		//Do extended check following SetParameterInfo
		fResult = XCHECK(m_pICmdWParams, IID_ICommandWithParameters, hr);	
	
	// RESET THE TYPEINFO correctly.
	hr = m_pICmdWParams->SetParameterInfo(m_cDbParamBindInfo, m_rgParamOrdinals, m_rgDbParamBindInfo);
	// We might have set typeinfo above or not depending upon where we jump from.
	if (hr != DB_S_TYPEINFOOVERRIDDEN)
	{
		if (!CHECK(hr, S_OK))
			fResult &= FALSE;
	}

CLEANUP:
	FreeDescParams();		

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Valid MapParameterNames calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_7()
{
	LONG cParamNames = 3;
	WCHAR *rgParamNames[] = {L"ONE", L"TWO", L"THREE" };
	DB_LPARAMS *rgParamOrdinals = NULL;
	BOOL fResult = FALSE;
	HRESULT  hr;

	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ICommandWithParameters method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.

	if ((rgParamOrdinals = (DB_LPARAMS *)m_pIMalloc->Alloc(cParamNames *sizeof (DB_LPARAMS))) == NULL)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}

	// May need to be prepared here
	if (m_pICommandPrepare)
		m_pICommandPrepare->Prepare(1);

	//create an error object
	m_pExtError->CauseError();
	
	TEST_CHECK (hr=m_pICmdWParams->MapParameterNames(0, (const WCHAR **)rgParamNames, rgParamOrdinals), S_OK);
	//do extended error check following the MapParameterNames
	fResult = XCHECK(m_pICmdWParams, IID_ICommandWithParameters, hr);	

CLEANUP:
	if (rgParamOrdinals)
		m_pIMalloc->Free(rgParamOrdinals);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Invalid MapParameterNames calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_8()
{
	LONG cParamNames = 3;
	DB_LPARAMS *rgParamOrdinals = NULL;
	BOOL fResult = FALSE;
	HRESULT hr;

 	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ICommandWithParameters method.
	//We then check extended errors to verify the right extended error behavior.
	
	if ((rgParamOrdinals = (DB_LPARAMS *)m_pIMalloc->Alloc(cParamNames *sizeof (DB_LPARAMS))) == NULL)
	{
		odtLog << wszMemoryAllocationError;
		return TEST_FAIL;
	}

	for (LONG i = 0; i < cParamNames; i++ )
		rgParamOrdinals[i] = i+1;

	// May need to be prepared here
	if (m_pICommandPrepare)
		m_pICommandPrepare->Prepare(1);

	//create an error object
	m_pExtError->CauseError();

	TEST_CHECK (hr=m_pICmdWParams->MapParameterNames(cParamNames, NULL, rgParamOrdinals), E_INVALIDARG);
	//Do extended check following MapParameterNames
	fResult = XCHECK(m_pICmdWParams, IID_ICommandWithParameters, hr);	

CLEANUP:
	if (rgParamOrdinals)
		m_pIMalloc->Free(rgParamOrdinals);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Invalid MapParameterNames calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_9()
{
	BOOL fResult = FALSE;
	HRESULT  hr;
	LONG cParamNames = 3;
	WCHAR *rgParamNames[] = {L"ONE", L"TWO", L"THREE" };
	
	//For each method of the interface, with no error object on
	//the current thread, then try get a failure from the ICommandWithParameters method.
	//We then check extended errors to verify the right extended error behavior.
	
	// May need to be prepared here
	if (m_pICommandPrepare)
		m_pICommandPrepare->Prepare(1);

	if (CHECK (hr=m_pICmdWParams->MapParameterNames(cParamNames, (const WCHAR **)rgParamNames, NULL), E_INVALIDARG))
		//Do extended check following MapParameterNames
		fResult = XCHECK(m_pICmdWParams, IID_ICommandWithParameters, hr);	
	
	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CICmdWParams::Terminate());
	// }}
}
// }}
// }}
 

// {{ TCW_TC_PROTOTYPE(TCBugRegressions)
//*-----------------------------------------------------------------------
//| Test Case:		TCBugRegressions - Test case to hold the regressions for bugs reported by other groups.
//|	Created:			11/05/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBugRegressions::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CICmdWParams::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Cmd that executes SP with param type adText returns truncation error if string > 255 chars
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCBugRegressions::Variation_1()
{
	ULONG cParams = 1;
	DB_UPARAMS rgParamOrdinals[] = { 1 };
	LPWSTR wszTypeInfo[] = { L"DBTYPE_LONGVARCHAR" };
	ULONG cTypes=2;
	DBPARAMBINDINFO rgParamBindInfo[] = { 
		{ NULL, NULL, LONG_MAX, DBPARAMFLAGS_ISNULLABLE | DBPARAMFLAGS_ISINPUT, (BYTE)~0, (BYTE)~0 }  };
	BOOL fResult = FALSE;
	ULONG i =0;
	DBBINDING DbBindings[1];
	void *pData = NULL;
	DBPARAMS Params;
	HRESULT hr;
	VARIANT vBstrValue;
	BSTR wszInputStr = L"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;

	rgParamBindInfo[0].pwszDataSourceType = wszTypeInfo[0];
	
	if (! (g_bSqlServer  && ( g_ulOutParamsSupported != DBPROPVAL_OA_NOTSUPPORTED) ))
	{
		odtLog << g_wszNotSqlServerOrNoOutParams;
		return TRUE;
	}

	// Init array to 0
	memset(DbBindings, 0, sizeof(DBBINDING));

	// First Drop the sp if exists.
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL, L"Drop Procedure CmdWParamBug26SP;"), S_OK);
	m_pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL);

	// Now create the Stored procedure.
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL,
		L"Create Procedure CmdWParamBug26SP (@ADOInParam text) AS  RETURN(1)"), S_OK);
	TEST_CHECK (m_pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL), S_OK);

	// Now set the execute SP statement.
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL,	L"{ call CmdWParamBug26SP (? ) }"), S_OK);

	if (!SUCCEEDED(m_pICmdWParams->SetParameterInfo(0, NULL, NULL)))
		goto CLEANUP;

	hr = m_pICmdWParams->SetParameterInfo (cParams, rgParamOrdinals, rgParamBindInfo);
	if (! (hr == S_OK || hr == DB_S_TYPEINFOOVERRIDDEN))
		goto CLEANUP;
	
	for (i = 0; i < cParams; i++)
	{
		DbBindings[i].dwPart = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
		DbBindings[i].eParamIO = DBPARAMIO_INPUT;
		DbBindings[i].iOrdinal = i+1;		// Parameter Ordinal.
		DbBindings[i].pTypeInfo = NULL;
		DbBindings[i].cbMaxLen = LONG_MAX;
		DbBindings[i].wType = DBTYPE_VARIANT;
		DbBindings[i].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		DbBindings[i].pBindExt = NULL;
		DbBindings[i].bPrecision = 0;
		DbBindings[i].bScale = 0;

	}

	VariantClear(&vBstrValue);
	vBstrValue.vt = VT_BSTR;
	V_BSTR(&vBstrValue) = wszInputStr;
	
	// Change first parameter to input only.
	DbBindings[0].eParamIO |= DBPARAMIO_INPUT;
	DbBindings[0].obValue = offsetof(DATA, bValue) ;
	DbBindings[0].obLength = offsetof(DATA, ulLength); 
	DbBindings[0].obStatus = offsetof (DATA, sStatus);

	// Call create accessor.
	TEST_CHECK (m_pCmdIAccessor->CreateAccessor( 
		DBACCESSOR_PARAMETERDATA, cParams, DbBindings, m_cbRowSize,
		&hAccessor, NULL), S_OK);

	// Allocate memory
	pData = (void *)m_pIMalloc->Alloc(m_cbRowSize);
	if (pData == NULL)
	{
		goto CLEANUP;
	}

	// 2. Create the data.
	for (i=0; i < cParams; i++ )
	{
		memcpy((void *)((BYTE *)pData + DbBindings[i].obValue), (void *)&vBstrValue, sizeof (VARIANT));
//		wcscpy (((WCHAR *)((BYTE *)pData + DbBindings[i].obValue)), wszInputStr);
		*((DBLENGTH *)((BYTE *)pData + DbBindings[i].obLength)) = sizeof (VARIANT);
		*((DBSTATUS *)((BYTE *)pData + DbBindings[i].obStatus)) = DBSTATUS_S_OK ;
	}


	Params.cParamSets = 1;
	Params.hAccessor = hAccessor;
	Params.pData = pData;

	
	TEST_CHECK(m_pICommandText->Execute(NULL, IID_NULL,
			&Params, NULL, NULL), S_OK);
	fResult = TRUE;
//TEST_CHECK ( GetAccessorAndBindingsFromTypes(cParams, rgParamBindInfo, &hLocalAccessor, &pData), S_OK);

CLEANUP:

	// Drop the stored proc we created above
	if (!CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL, L"Drop Procedure CmdWParamBug26SP;"), S_OK))
		fResult = FALSE;

	m_pICommandText->Execute(NULL, IID_NULL, NULL, NULL, NULL);

	FREE_DATA (pData);

	return (fResult) ? TEST_PASS : TEST_FAIL;
}
// }}




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Prepare, Execute, Prepare, Execute w/o rebinding
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBugRegressions::Variation_2()
{ 
	BOOL fResult = FALSE;
	DBROWCOUNT cRowsAffected = 0;
	DBCOUNTITEM ulRowNum;

	// Remove any old parameter info
	TEST_CHECK(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	// Set command text 
	TEST_CHECK (m_pICommandText->SetCommandText(DBGUID_DBSQL , m_pwszSqlInsertAllWithParams), S_OK);
	
	// Call ICommandPrepare.
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);
	
	//This sets it to one row of data.
	MakeDataForCommand((ulRowNum = NextInsertRowNum()));
		
	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Verify If we have inserted the row properly.
	TEST_COMPARE(cRow.FindRow(ulRowNum, m_pTable), TRUE);

	// Prepare again
	TEST_CHECK (m_pICommandPrepare->Prepare(1), S_OK);

	// Make data for the next valid row
	MakeDataForCommand((ulRowNum = NextInsertRowNum()));

	// Now execute ICommand.
	TEST_CHECK(m_pICommand->Execute(NULL, IID_NULL,
			&m_DbParamsAll, &cRowsAffected, NULL), S_OK);

	// Verify If we have inserted the row properly.
	TEST_COMPARE(cRow.FindRow(ulRowNum, m_pTable), TRUE);

	fResult = TRUE;
CLEANUP:

	ReleaseDataForCommand();

	return (fResult) ? TEST_PASS : TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Retrieve a row after failing to retrieve with DBSTATUS_S_ISNULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBugRegressions::Variation_3()
{ 

	BOOL fResult = FALSE;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 2;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	ULONG_PTR ulSqlSupport = DBPROPVAL_SQL_NONE;
	DBSTATUS dbStatus = DBSTATUS_E_BADSTATUS;
	DBPARAMS Param;
	HACCESSOR hExecAccessor;
	DBROWCOUNT cRowsAffected = 0;
	DBCOUNTITEM cRowsObtained = 0;
	IRowset * pIRowset = NULL;
	DBCOUNTITEM cRowsInRowset = 0;
	HRESULT	hrSetProp = E_FAIL;

	// Create the syntax and binding for a stored proc with input parameters
	TESTC(CreateProcBindings(
		T_EXEC_PROC_SELECT_IN,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd
	));

	// Find out if we're a SQL provider
	TESTC(GetProperty(DBPROP_SQLSUPPORT, 
				   DBPROPSET_DATASOURCEINFO,m_pIDBInitialize, &ulSqlSupport));

	TESTC_(m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	TESTC_(m_pICommandText->SetCommandText(DBGUID_DEFAULT, pwszExecStmt), S_OK);

	// Set the status value for the first binding to DBSTATUS_S_ISNULL so query shouldn't retrieve any rows
	dbStatus = STATUS_BINDING(pBINDING[0], pData);
	STATUS_BINDING(pBINDING[0], pData) = DBSTATUS_S_ISNULL;

	// Create a parameter accessor
	TESTC_(m_pCmdIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, cParams, pBINDING, cbRowSize,
		&hExecAccessor, NULL), S_OK);

	Param.cParamSets = 1;
	Param.hAccessor = hExecAccessor;
	Param.pData = pData;
	
	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on
	if (SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	// Execute Command statement.
	TESTC_(m_pICommand->Execute(NULL, IID_IRowset, &Param, 
		&cRowsAffected, (IUnknown **)&pIRowset),S_OK);	

	// If we made it to here we have a (possibly empty) rowset
	TESTC_(VerifyObj(m_iidExec, pIRowset, ulRowNum, cColumns, prgColumnsOrd,
		FALSE, TRUE, NULL, &cRowsInRowset), S_OK);

	// Release the rowset pointer
	SAFE_RELEASE(pIRowset);

	// For SQL providers we expect this to return 0 rows if there were NULLS in the bindings,
	// but it's not a failure if they succeed.
	if (ulSqlSupport != DBPROPVAL_SQL_NONE)
		TESTW(cRowsInRowset == 0);
	else
		TESTC(cRowsInRowset > 0);

	// Now reset the binding correctly to see if we get the desired row
	STATUS_BINDING(pBINDING[0], pData) = dbStatus;

	// Execute again
	TESTC_(m_pICommand->Execute(NULL, IID_IRowset, &Param, 
		&cRowsAffected, (IUnknown **)&pIRowset),S_OK);	

	// We should have the row
	TESTC_(VerifyObj(m_iidExec, pIRowset, ulRowNum, cColumns, prgColumnsOrd,
		FALSE, TRUE), S_OK);

	fResult = TEST_PASS;

CLEANUP:
	
	SAFE_RELEASE_ACCESSOR(m_pCmdIAccessor, hExecAccessor);

	CHECK(ReleaseInputBindingsMemory(cParams, pBINDING, pData, FALSE), S_OK);
	SAFE_RELEASE(pIRowset);

	// If we set RANDOM REQUIRED above we need to set back OPTIONAL
	if (hrSetProp == S_OK)
		CHECK(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM, DBPROPOPTIONS_OPTIONAL), S_OK);

	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	PROVIDER_FREE(prgColumnsOrd);
	
	return fResult;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Use int64 as input and output param binding in stored proc
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBugRegressions::Variation_4()
{ 

	BOOL fResult = TEST_FAIL;
	
	ULONG cParams, cParamSets = 1;
	DBLENGTH cbRowSize = 0;
	DBCOUNTITEM ulRowNum = 2;
	DBBINDING * pBINDING=NULL;
	DB_UPARAMS * pParamOrdinals=NULL;
	DBPARAMBINDINFO * pPARAMBIND=NULL;
	WCHAR * pwszCreateProcStmt=NULL;
	WCHAR * pwszExecProcStmt=NULL;
	WCHAR * pwszExecStmt=NULL;
	WCHAR * pwszProcName=NULL;
	BYTE * pData=NULL;
	ParamStruct * pParamAll=NULL;
	DBORDINAL cColumns = 0;
	DB_LORDINAL * prgColumnsOrd = NULL;
	BOOL fCanDerive = FALSE;
	IConvertType * pIConvType = NULL;
	ULONG iParam;
	DB_LORDINAL * pParamColOrd = NULL;
	BYTE * pData64 = NULL;
	BYTE * pData64Match = NULL;
	DBBINDING * pBINDING64 = NULL;
	DBLENGTH cbRowSize64 = 0;
	ULONG ci64 = 0;
	USHORT uSrcData = 0;
	WCHAR wszData[MAXDATALEN] = L"";
	HRESULT hrMakeData = E_FAIL;
	HRESULT hrSetParam = E_FAIL;
	HRESULT hrExec = S_OK;
	void * pvData = NULL;
	DB_UPARAMS cDerivedParams = 0;
	DBPARAMINFO * pParamInfo = NULL;

	if (!m_fProcedureSupport)
	{
		odtLog << "Procedures not supported.\n";
		return TEST_SKIPPED;
	}

	if (g_ulOutParamsSupported == DBPROPVAL_OA_NOTSUPPORTED)
	{
		odtLog << "Output parameters not supported.\n";
		return TEST_SKIPPED;
	}

	// Create the syntax and binding for a stored proc with output parameters
	TESTC(CreateProcBindings(
		T_EXEC_PROC_SELECT_OUT,	// [IN]  Proc type, regular proc or function (has return value)
		TRUE,					// [IN]  If TRUE then we add parameter names to the rgParamBindInfo
		cParamSets,				// [IN]	 Number of sets of parameters to be created
		DBTYPE_I2,				// [IN]  Return parameter type
		ulRowNum,				// [IN]  Row number in table to select, insert, or update
		&cParams,				// [OUT] Count of params created
		&cbRowSize,				// [OUT] Count of bytes for a single row of parameters
		&pBINDING,				// [OUT] Binding array for CreateAccessor
		&pParamOrdinals,
		&pPARAMBIND,			// [OUT] rgParamBindInfo for SetParameterInfo
		&pwszCreateProcStmt,	// [OUT] SQL stmt to create the stored proc
		&pwszExecProcStmt,		// [OUT] SQL stmt to execute the stored proc
		&pwszExecStmt,			// [OUT] SQL stmt to execute without stored proc
		&pwszProcName,			// [OUT] Name of procedure
		&pData,					// [OUT] Pointer to data for the parameters
		&pParamAll,
		&cColumns,
		&prgColumnsOrd,
		&pParamColOrd
	));

	// Make a copy of the original DBBINDING structure
	SAFE_ALLOC(pBINDING64, DBBINDING, cParams);
	memcpy(pBINDING64, pBINDING, cParams*sizeof(DBBINDING));

	// We need to convert all eligible consumer bindings to I8, so get an IConvertType pointer.
	TESTC(VerifyInterface(m_pICommand, IID_IConvertType, COMMAND_INTERFACE, (IUnknown**)&pIConvType));

	// Go through each of the parameters and see if it will convert from I8 to native type.
	cbRowSize64 = cbRowSize;
	for (iParam = 0; iParam < cParams; iParam++)
	{
		HRESULT hrConv = E_FAIL;

		if (pBINDING[iParam].eParamIO == DBPARAMIO_INPUT)
			hrConv = pIConvType->CanConvert(DBTYPE_I8, pBINDING[iParam].wType, DBCONVERTFLAGS_PARAMETER);
		else
			hrConv = pIConvType->CanConvert(pBINDING[iParam].wType, DBTYPE_I8, DBCONVERTFLAGS_PARAMETER);

		TEST2C_(hrConv, S_OK, S_FALSE);

		// If we can convert, then this is an eligible binding
		if (S_OK == hrConv)
		{
			// The conversion is supported, but not necessarily for every value.
			// For example 'H123' won't convert to I2, but '123' will, etc.  So we need
			// to ensure the data value in the table will convert to I8.
			WCHAR wszData64[MAXDATALEN] = L"";
			BOOL fConv = FALSE;

			// Get a string copy of the data.
			hrMakeData = m_pTable->MakeData(wszData, ulRowNum, pParamAll[iParam].ulColIndex, PRIMARY);

			TEST2C_(hrMakeData, S_OK, S_FALSE);

			if (S_OK == hrMakeData)
			{
				// See if it will convert to I8
				pvData = WSTR2DBTYPE(wszData, DBTYPE_I8, &uSrcData);

				if (pvData)
				{
					// It's also important that we didn't truncate fractional digits or otherwise change
					// the data during the conversion.  To see if this is so, convert the I8 to a wstr,
					// it should match the original data string.
					_i64tow(*(LONGLONG *)pvData, wszData64, 10);

					if (!wcscmp(wszData, wszData64))
					{
						// The values matched, we didn't lop off decimals, etc.
						fConv = TRUE;
						SAFE_FREE(pvData);
					}
				}
			}
			else
				// Must be NULL.  We can always retrieve a NULL into an int64.
				fConv = TRUE;
			
			if (fConv)
			{
				// Change the data type in the binding
				pBINDING64[iParam].wType = DBTYPE_I8;
				ci64++;
			}
		}
	}

	// See if there's at least one eligible binding
	if (!ci64)
	{
		odtLog << L"No data types available that contain i64 convertible data.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	else
		odtLog << L"There are now " << ci64 << " int64 bindings.\n";

	// Repack the obValue, obLength, and obStatus based on the new bindings
	Repack(cParams, pBINDING64, &cbRowSize64);

	// Create a new pData based on the new bindings
	TESTC_(FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cParams,
		pBINDING64, &pData64, ulRowNum, cParams, pParamColOrd, PRIMARY), S_OK);

	// Make a copy of the param data that will contain valid data for out params to compare
	// with.
	SAFE_ALLOC(pData64Match, BYTE, cbRowSize64);
	memcpy(pData64Match, pData64, (size_t)cbRowSize64);

	// Now replace with converted data 
	for (iParam = 0; iParam < cParams; iParam++)
	{
		// We have an I8 binding, convert the data
		if (pBINDING64[iParam].wType == DBTYPE_I8)
		{
			// Just in case, free any other data in this binding
			ReleaseInputBindingsMemory(1, &pBINDING64[iParam], pData64, FALSE);

			// Get a string copy of the data.
			hrMakeData = m_pTable->MakeData(wszData, ulRowNum, pParamAll[iParam].ulColIndex, PRIMARY);

			if (S_OK == hrMakeData)
			{
				// Convert to I8
				pvData = WSTR2DBTYPE(wszData, DBTYPE_I8, &uSrcData);

				TESTC(pvData != NULL);

				// Put valid data in both match and param buffers
				*(LONGLONG *)&VALUE_BINDING(pBINDING64[iParam], pData64Match) = *(LONGLONG *)pvData;
				STATUS_BINDING(pBINDING64[iParam], pData64Match) = DBSTATUS_S_OK;
				LENGTH_BINDING(pBINDING64[iParam], pData64Match) = GetDataSize(pBINDING64[iParam].wType,
					uSrcData);
				*(LONGLONG *)&VALUE_BINDING(pBINDING64[iParam], pData64) = *(LONGLONG *)pvData;

				SAFE_FREE(pvData);
			}
		}
		else
			// Set valid length for other fixed length types
			LENGTH_BINDING(pBINDING64[iParam], pData64Match) = GetDataSize(pBINDING64[iParam].wType,
				LENGTH_BINDING(pBINDING64[iParam], pData64Match));

		// Set output only params to garbage
		if (pBINDING64[iParam].eParamIO == DBPARAMIO_OUTPUT)
		{
			STATUS_BINDING(pBINDING64[iParam], pData64) = OUT_PARAM_STATUS_INVALID;
			LENGTH_BINDING(pBINDING64[iParam], pData64) = OUT_PARAM_LENGTH_INVALID;

			memset(&VALUE_BINDING(pBINDING64[iParam], pData64), 
				0xCA,
				(size_t)GetDataSize(pBINDING64[iParam].wType, pBINDING64[iParam].cbMaxLen));
		}
	}

	// In order to test this you must do a partial SetParameterInfo.
	// If you set all or none the bug won't repro.  Note if we do a partial
	// SetParameterInfo we can't set a parameter name because then we'd end up
	// with some that have names and some that don't - error condition.
	TESTC_(hrSetParam = m_pICmdWParams->SetParameterInfo(0, NULL, NULL), S_OK);

	pPARAMBIND[0].pwszName = NULL;
	hrSetParam = m_pICmdWParams->SetParameterInfo(1, &pParamOrdinals[0], &pPARAMBIND[0]);

	TEST2C_(hrSetParam, S_OK, DB_S_TYPEINFOOVERRIDDEN);

	if (hrSetParam == S_OK)
		fCanDerive = FALSE;
	else
		fCanDerive = TRUE;

	// Set up to execute the stored proc
	if (pwszCreateProcStmt)
		// Create the stored procedure
		TESTC_(CreateStoredProc(m_pICommandText, pwszProcName, pwszCreateProcStmt, FALSE), S_OK);

	// Set the command text to execute the stored proc
	TESTC_(m_pICommandText->SetCommandText(DBGUID_DBSQL, pwszExecProcStmt), S_OK);

	// If the provider can derive, then it's permissible for them to derive the additional
	// parameter information.  However, if they don't then we'll fail executing the proc.
	if (fCanDerive)
	{
		TESTC_(m_pICmdWParams->GetParameterInfo(&cDerivedParams, &pParamInfo, NULL), S_OK);

		// The provider shouldn't give me more params than there are.
		COMPARE(cDerivedParams <= cParams, TRUE);

		// But if there are less, then there must be only 1.
		if (cDerivedParams < cParams)
		{
			COMPARE(cDerivedParams, 1);
			// And Execute will fail because we don't know all the params.
			// While I would prefer DB_E_ERRORSOCCURRED because we have invalid input
			// param value, the spec doesn't seem to require this.
			hrExec = E_FAIL;
		}
	}

	TESTC_(ExecuteAndVerify(cParams, cParamSets, pParamAll, ulRowNum, pBINDING64, cbRowSize64,
		pData64, ROWSET_NONE, cColumns, prgColumnsOrd, VERIFY_USE_PDATA, TRUE, NULL, hrExec,
		NULL, pData64Match), S_OK);

	fResult = TEST_PASS;

CLEANUP:

	DropStoredProcedure(m_pICommandText, pwszProcName);

	SAFE_RELEASE(pIConvType);

	// Free the buffers we got from GetParameterInfo
	PROVIDER_FREE(pParamInfo);
	PROVIDER_FREE(pvData);
	PROVIDER_FREE(pBINDING64);
	PROVIDER_FREE(pData64);
	PROVIDER_FREE(pData64Match);
	PROVIDER_FREE(pParamColOrd);
	PROVIDER_FREE(pBINDING);
	PROVIDER_FREE(pParamOrdinals);
	PROVIDER_FREE(pPARAMBIND);
	PROVIDER_FREE(pwszCreateProcStmt);
	PROVIDER_FREE(pwszExecProcStmt);
	PROVIDER_FREE(pwszExecStmt);
	PROVIDER_FREE(pwszProcName);
	PROVIDER_FREE(pData);
	::FreeParameterNames(cParams, pParamAll);
	PROVIDER_FREE(pParamAll);
	
	return fResult;;

} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBugRegressions::Terminate()
{

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CICmdWParams::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCIUnknown)
//*-----------------------------------------------------------------------
//| Test Case:		TCIUnknown - Test parameter bindings to IUnknown
//|	Created:			06/23/97
//*-----------------------------------------------------------------------

// Determines whether desireed IUnknown conversion should be successful for the given column
HRESULT TCIUnknown::CanConvertIUnknown(IUnknown * pIUnknown, DBTYPE wType, DBCONVERTFLAGS dwConvertFlags)
{
	HRESULT			hr=S_FALSE;
	ULONG			cColumns=0;
	IConvertType *	pIConvertType=NULL;
	IColumnsInfo *	pIColumnsInfo=NULL;
	DBCOLUMNINFO *	prgInfo=NULL;
	LPWSTR			pStringsBuffer=NULL;

	// Interface could be a command or rowset interface
	pIUnknown->QueryInterface(IID_IConvertType, (void **)&pIConvertType);
	if (!pIConvertType)
		return E_NOINTERFACE;

	pIUnknown->QueryInterface(IID_IColumnsInfo, (void **)&pIColumnsInfo);	
	if (!pIColumnsInfo)
		return E_NOINTERFACE;

	switch (dwConvertFlags)
	{
		case DBCONVERTFLAGS_PARAMETER:

			hr = pIConvertType->CanConvert(DBTYPE_IUNKNOWN, wType, dwConvertFlags);

			break;
		case DBCONVERTFLAGS_COLUMN:

			hr = pIConvertType->CanConvert(wType, DBTYPE_IUNKNOWN, dwConvertFlags);

			break;

		default:
			hr = E_FAIL;
	}

	pIConvertType->Release();
	pIColumnsInfo->Release();
	FREE_DATA(prgInfo);
	FREE_DATA(pStringsBuffer);
	return hr;
}

BOOL TCIUnknown::IsBLOB(DBTYPE wType)
{
	switch(wType)
	{
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		case DBTYPE_BYTES:
			return TRUE;
		default:
			return FALSE;
	}
}

LPVOID TCIUnknown::MakeData(CCol ColInfo, ULONG * pcbDataSize, ULONG ulSeed, enum EVALUE eValue)
{
	LPWSTR pwszDataItem=NULL;
	LPVOID pvData=NULL;
	DBTYPE wType = ColInfo.GetProviderType();
	ULONG cbSrcData=0;
	USHORT uSrcData=0;

	// Allocate a WCHAR buffer large enough to hold a data item
	TEST_ALLOC(WCHAR, pwszDataItem, 0, (size_t)(DisplaySize(ColInfo)+sizeof(WCHAR)));

	// Make a data item for this column
	TEST_CHECK(m_pTable->MakeData(pwszDataItem, ulSeed, ColInfo.GetColNum(), eValue, wType, TRUE), S_OK);
	
	// Convert the char item to the appropriate type for this column.
	// Note uSrcData only used for DBTYPE_BYTES
	pvData = WSTR2DBTYPE(pwszDataItem, wType, &uSrcData);

	switch(wType)
	{
		case DBTYPE_BYTES:
		case DBTYPE_VARNUMERIC:
			cbSrcData = uSrcData;
			break;
		case DBTYPE_STR:
			cbSrcData = (ULONG)strlen((CHAR *)pvData);
			break;
		case DBTYPE_WSTR:
			cbSrcData = (ULONG)wcslen(pwszDataItem)*sizeof(WCHAR);
			break;
		default:
			cbSrcData = GetDBTypeSize(wType);
			break;
	}

CLEANUP:

	FREE_DATA(pwszDataItem);

	if (pcbDataSize)
		*pcbDataSize = cbSrcData;

	return pvData;
}


HRESULT TCIUnknown::CreateStorageObject(REFIID iidObject, IUnknown ** ppIObject)
{
	if (!ppIObject)
		goto CLEANUP;

	if (IID_ISequentialStream == iidObject)
		*ppIObject = new ICmdWParSequentialStream();
	else
	{
		*ppIObject = NULL;
		goto CLEANUP;
	}

	TEST_PTR(*ppIObject);

	return S_OK;

CLEANUP:

	return E_FAIL;
	
}

BOOL TCIUnknown::IsLastByteLeadByte(LPVOID pvSrcData, ULONG cbSrcData, ULONG cbReadSize)
{
	BYTE * pbSrcData = (BYTE *)pvSrcData;
	ULONG iLast, iByte = 0;

	// Adjust for read size greater or less than source buffer.
	if (cbReadSize > cbSrcData)
		iLast = cbReadSize % cbSrcData - 1;
	else
		iLast = cbReadSize - 1;

	// Wrap to source buffer size
	if (!(iLast + 1))
		iLast+=cbSrcData;
	
	while (iByte <= iLast)
	{
		if (IsDBCSLeadByte(pbSrcData[iByte]))
		{
			if (iByte == iLast)
				return TRUE;
			// Skip the trail byte
			iByte++;
		}
		iByte++;
	}

	return FALSE;
}

TESTRESULT TCIUnknown::TestActiveError(enum ACTIVE_ERROR eTest, REFIID iidParamObject)
{
	BOOL					fResult = TRUE;
	CCol  					ColInfo;
	const WCHAR				wszUpdateFormat[] = L"update %s set %s = ?";
	const WCHAR				wszSelectFormat[] = L"select %s from %s";
	WCHAR					wszCommand[MAX_COMMAND_BUF] = L"";
	ULONG					cbSrcData;
	LPVOID					pvSrcData=NULL;
	ULONG					iCol, cRefCount, cParams = 1, iRow;
	DBROWCOUNT				cRowsAffected;
	IRowset *				pIRowset=NULL;
	IRowset *				pIRowsetOpenObject=NULL;
	IUnknown *				pIParamObject=NULL;
	HACCESSOR				hParamAccessor;
	IAccessor *				pIAccessor=NULL;
	ULONG_PTR				ulStorageInterface;
	DBPROPID				ColumnStorageProperty;
	DBACCESSORFLAGS			dwAccessorFlags = DBACCESSOR_PARAMETERDATA;
	IUnknown *				pUnkOuter = NULL;
	HRESULT					hrExp = S_OK;
	DBSTATUS				ulInputStatus;
	ULONG					cRef = 0;
	BOOL					fRelease = TRUE;
	DBLENGTH				ulParamOffset = 0;

	// If the provider supports IMultipleResults then we can't test this one
	if (eTest == INVALIDARG_MULTRES)
	{
		// Actually we don't support this one yet because the framework expects
		// to use an update and we need a select for this test.
		// Testing not implemented yet.
		return TEST_PASS;
	}

	if (eTest == NOAGGREGATION_OLEDB1)
	{
		// If this isn't an OLEDB 1.0 or 1.1 provider can't test
		// For now, just return passed because we don't test
		// 1.0 or 1.1 providers
		// Testing not implemented yet.
		return TEST_PASS;
	}

	if (eTest == OBJECTOPEN)
	{
		// If this provider does not support DBPROP_MULTIPLECONNECTIONS,
		// then it will not return DB_E_OBJECTOPEN on Execute. Need to 
		// check the property and expect Execute to succeed.  Otherwise
		// if the prop is supported then set off.  Need to add code to
		// do that.
		// Testing not implemented yet.
		return TEST_PASS;
	}

	// If the provider supports IMultipleResults then we can't test this one
	if (eTest == BADBINDINFO)
	{
		// Testing not implemented yet.
		return TEST_PASS;
	}

	// Structure for parameter and row buffers for IUnknown and in-line data
	struct tagParameterData {
		DBLENGTH			ulDataSize;
		DBSTATUS			ulStatus;
		union {
			IUnknown *			pIUnknown;
			BYTE				bData[MAX_DATA_BUF+1];
		};
	} *pParameterData;

	// Storage object info
	DBOBJECT				ParamStorageObject;

	// Common client parameter binding info
	DBBINDING				rgParamBind[1];
	rgParamBind[0].iOrdinal	=1;
	if (eTest == BADORDINAL)
		rgParamBind[0].iOrdinal	=100;
	rgParamBind[0].obLength	=ulParamOffset;
	ulParamOffset += sizeof(DBLENGTH);
	rgParamBind[0].obStatus	=ulParamOffset;
	ulParamOffset += sizeof(DBSTATUS);
	ulParamOffset = ROUND_UP(ulParamOffset, ROUND_UP_AMOUNT);
	rgParamBind[0].obValue	=ulParamOffset;
	rgParamBind[0].pTypeInfo=NULL;
	rgParamBind[0].pObject	=&ParamStorageObject;
	rgParamBind[0].pBindExt	=NULL;
	rgParamBind[0].dwPart	=DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
	rgParamBind[0].dwMemOwner=DBMEMOWNER_CLIENTOWNED;
	rgParamBind[0].eParamIO	=DBPARAMIO_INPUT;
	rgParamBind[0].cbMaxLen	=0;  
	rgParamBind[0].dwFlags	=0;
	rgParamBind[0].wType	=DBTYPE_IUNKNOWN;
	rgParamBind[0].bPrecision=0;
	rgParamBind[0].bScale	=0;

	// Backend parameter binding info
	DB_UPARAMS rgParamOrdinals[1]={1};
	DBPARAMBINDINFO			rgParamBindInfo[1];
	rgParamBindInfo[0].pwszDataSourceType = NULL;
	rgParamBindInfo[0].pwszName = NULL;
	rgParamBindInfo[0].ulParamSize = 0;
	rgParamBindInfo[0].dwFlags = DBPARAMFLAGS_ISINPUT;
	rgParamBindInfo[0].bPrecision = 0;
	rgParamBindInfo[0].bScale = 0;

	SAFE_ALLOC(pParameterData, struct tagParameterData, 1);

	// Assume failure until proven otherwise
	m_TestResult=TEST_PASS;

	// Set the property values we need to use
	if (iidParamObject==IID_ISequentialStream)
	{
		ulStorageInterface=DBPROPVAL_SS_ISEQUENTIALSTREAM;
		ColumnStorageProperty = DBPROP_ISequentialStream;
	}
	else if (iidParamObject==IID_ILockBytes)
	{
		ulStorageInterface=DBPROPVAL_SS_ILOCKBYTES;
		ColumnStorageProperty = DBPROP_ILockBytes;
	}
	else if (iidParamObject==IID_IStorage)
	{
		ulStorageInterface=DBPROPVAL_SS_ISTORAGE;
		ColumnStorageProperty = DBPROP_IStorage;
	}
	else if (iidParamObject==IID_IStream)
	{
		ulStorageInterface=DBPROPVAL_SS_ISTREAM;
		ColumnStorageProperty = DBPROP_IStream;
	}
	else
	{
		// Not an object we know how to test
		return TEST_FAIL;
	}

	// Make sure this property (interface) is supported by the provider (data source)
	if (!(m_fSupportInterface = (m_ulStorageSupport & ulStorageInterface) > 0))
		odtLog << L"Storage object is not supported.\n";

	// Variation specific information
	ParamStorageObject.iid = iidParamObject;
	// Set the iid to an invalid value
	if (eTest == NOINTERFACE)
		ParamStorageObject.iid = IID_IRowset;

	ParamStorageObject.dwFlags	= STGM_READ;
	if (eTest == BADSTORAGEFLAGS_ERR)
		ParamStorageObject.dwFlags	= STGM_INVALID;

	// Create a parameter accessor for this parameter
	if (eTest == BADACCESSORTYPE)
		dwAccessorFlags = DBACCESSOR_ROWDATA;

	m_hr = m_pIAccessor->CreateAccessor(dwAccessorFlags, 1, rgParamBind, sizeof(struct tagParameterData),
		&hParamAccessor, m_rgStatus);

	// If the provider caught the error at CreateAccessor time we can't test Execute
	if (FAILED(m_hr))
	{
		fResult = TEST_PASS;
		goto CLEANUP;
	}

	// Create a DBPARAMS structure to pass to execute later
	DBPARAMS Params;
	Params.pData=pParameterData;
	Params.cParamSets=1;
	Params.hAccessor=hParamAccessor;

	// Pick a column to be used for this test
	// Note that a column or parameter need not be specified as DBCOLUMNFLAGS_ISLONG
	// to allow the use of structured storage.
	for (iCol=1; iCol <= m_pTable->CountColumnsOnTable(); iCol++)
	{
		ULONG cbReadSize;

		// Free the data buffers for this column
		FREE_DATA(pvSrcData);

		// Retrieve the information about this column
		ABORT_CHECK(m_pTable->GetColInfo(iCol, ColInfo), S_OK);

		// Pick the appropriate column
		if (!ColInfo.GetUpdateable())
			continue;

		// If this is a key column then we can't do an update of the whole column to the same value.
		if (iCol == m_pTable->GetIndexColumn())
			continue;

		// For UNSUPPORTEDCONVERVERSION pick a column that won't convert from IUnknown
		if (eTest == UNSUPPORTEDCONVERSION)
		{
			if (ColInfo.GetIsLong())
				continue;

			if (S_OK == CanConvertIUnknown(m_pICommand, ColInfo.GetProviderType(),
										DBCONVERTFLAGS_PARAMETER))
				continue;
		}
		else if (!ColInfo.GetIsLong())
			continue;

		// Make a data item appropriate for this column
		pvSrcData = MakeData(ColInfo, &cbSrcData, 1);
		TEST_PTR(pvSrcData);

		// Set the read size

		// Use the data size, except for long data use MAX_LONG_VALUE
		// This way we can test sending large parameters in pieces via our ISequentialStream
		// implementation.
 		cbReadSize =  cbSrcData; 
		if (ColInfo.GetColumnSize() > MAX_LONG_VALUE)
		{
			cbReadSize = MAX_LONG_VALUE;  // Limit to something practical.

			// For WSTR columns we must have an even amount
			if (ColInfo.GetProviderType() == DBTYPE_WSTR)
			{
				if (cbReadSize % 2)
					cbReadSize--;
			}
			// For STR columns we cannot split a DBCS character
			if (ColInfo.GetProviderType() == DBTYPE_STR)
			{
				// See if this size would split a DBCS character and adjust if required.
				// We will split a DBCS character if the last byte to be sent is a lead byte.
				if (IsLastByteLeadByte(pvSrcData, cbSrcData, cbReadSize))
					cbReadSize--;
			}

		}

		// Get the storage object pointer.  This will be addrefed, released, and therefore 
		// deleted by the provider during Execute.  After Execute the pointer is not valid.
		// Reference count must be 2 or greater to keep the object around after Execute

		// Create the storage object
		if (!SUCCEEDED(CreateStorageObject(iidParamObject, (IUnknown **)&pIParamObject)))
		{
			odtLog << L"Unable to create parameter storage object.\n";
			goto CLEANUP;
		}

		// Initialize the storage object
		if (IID_ISequentialStream == iidParamObject)
			((ICmdWParSequentialStream *)pIParamObject)->Init(pvSrcData, cbSrcData, cbReadSize);
		else
			// Not a parameter object we know how to initialize
			goto CLEANUP;

		// Set the storage object pointer appropriately
		pParameterData->pIUnknown = (IUnknown *)pIParamObject;

		// Set the length of the data
		pParameterData->ulDataSize = cbReadSize;

		// Set the status on input
		pParameterData->ulStatus = DBSTATUS_S_OK;

		// update %s set %s = ?
		swprintf(wszCommand, wszUpdateFormat, m_pTable->GetTableName(), ColInfo.GetColName());

		// Set command text
		ABORT_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL, wszCommand), S_OK);

		// Set the parameter information for this parameter
		rgParamBindInfo[0].pwszDataSourceType = ColInfo.GetProviderTypeName();

		// ulParamSize is cch for character types.
		if (ColInfo.GetProviderType() == DBTYPE_WSTR)
			rgParamBindInfo[0].ulParamSize = cbReadSize/sizeof(WCHAR);
		else
			rgParamBindInfo[0].ulParamSize = cbReadSize;

		rgParamBindInfo[0].bPrecision = (BYTE)ColInfo.GetPrecision();
		rgParamBindInfo[0].bScale = (BYTE)ColInfo.GetScale();

		if (!SUCCEEDED(m_pICommandWithParameters->SetParameterInfo(1, rgParamOrdinals, rgParamBindInfo)))
			goto CLEANUP;

		// Reference count must be 2 or greater to keep the object around after Execute
		// for non-NULL data.  
		COMPARE(pIParamObject->AddRef(), 2);

		// Set common error conditions where possible
		switch (eTest)
		{
			case INVALIDARG_NULL_PDATA:
				Params.pData=NULL;
				hrExp = E_INVALIDARG;
				break;
			case INVALIDARG_CPARAMSETS_ZERO:
				Params.cParamSets=0;
				hrExp = E_INVALIDARG;
				break;
			case BADACCESSORHANDLE:
				Params.hAccessor=DB_NULL_HACCESSOR;
				hrExp = DB_E_BADACCESSORHANDLE;
				break;
			case BADACCESSORTYPE:
				// Nothing to do, rowdata accessor created above
				hrExp = DB_E_BADACCESSORTYPE;
				break;
			case ERRORSOCCURRED:
				// Set the status to a bad value
				// TODO: We need more than one stream parameter and verify
				// released for DBSTATUS_S_OK and not released otherwise.
				odtLog << L"DB_E_ERRORSOCCURRED case not yet fully tested.\n";
				pParameterData->ulStatus = DBSTATUS_E_BADSTATUS;
				hrExp = DB_E_ERRORSOCCURRED;
				break;
			case NOINTERFACE:
				// Nothing to do, invalid iid set above
				hrExp = E_NOINTERFACE;
				break;
			case BADORDINAL:
				// Nothing to do, ordinal 100 set above
				hrExp = DB_E_BADORDINAL;
				// For at least one case we know we will cause a crash on dll shutdown
				// using this scenario.  The workaround is to not release the storage
				// object for this case, but post a failure.
				if (GetModInfo()->GetClassContext() == CLSCTX_LOCAL_SERVER)
					fRelease = FALSE;
				break;
			case BADSTORAGEFLAGS_ERR:
				// Nothing to do, STGM_INVALID set above
				hrExp = DB_E_BADSTORAGEFLAGS;
				break;
			case UNSUPPORTEDCONVERSION:
				// Nothing to do, unsupported column picked above
				hrExp = DB_E_UNSUPPORTEDCONVERSION;
				break;
			case NOAGGREGATION_NOT_IUNKOWN:
				pUnkOuter=pIParamObject;
				hrExp = DB_E_NOAGGREGATION;
				break;
			case NOCOMMAND:
				ABORT_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL, NULL), S_OK);
				hrExp = DB_E_NOCOMMAND;
				break;
			case OBJECTOPEN:
				// Open a rowset on the command
				ABORT_CHECK (m_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset, NULL, NULL, 
					NULL, NULL,  EXECUTE_ALWAYS, 0, NULL, NULL, (IUnknown **)&pIRowsetOpenObject, &m_pICommand), S_OK);
				hrExp = DB_E_OBJECTOPEN;
				break;
			case BADBINDINFO:
				hrExp = DB_E_BADBINDINFO;
			default:
				// We don't understand any other tests
				fResult = FALSE;
				ABORT_COMPARE(TRUE, FALSE);
		};

		// Save the param input status
		ulInputStatus = pParameterData->ulStatus;

		// Perform the update.
		// At this time we don't have a provider supporting anything but ISequentialStream, so
		// we won't write consumer implementations of anything else.  If a provider DOES support
		// another interface, and DOESN'T bother to QI before using the interface passed in, then
		// we will crash here.  Otherwise QI will fail, and Execute will fail.
		m_hr = m_pICommand->Execute(pUnkOuter, IID_IRowset, &Params, &cRowsAffected, (IUnknown **)&pIRowset);

		// This is an error test and therefore should not succeed.
		ABORT_CHECK(m_hr, hrExp);

		// These are fatal errors and we don't expect our stream object 
		// to have been read
		if (FAILED(m_hr))
		{
		FAIL_COMPARE(((ICmdWParSequentialStream *)pIParamObject)->WasRead(), FALSE);
		}


		// If the provider released the storage object, then after we release cRef == 0
		// If provider didn't release then cRef == 1 after we release here.
		if (fRelease)
			cRef = pIParamObject->Release();
		else
			cRef = 0;

		// If the ref count is 0 then don't attempt to delete object below
		if (cRef == 0)
			pIParamObject = NULL;

		// Make sure the ref count is what we expect. Should be 1.
		// Provider should not release if
		//		m_hr != DB_E_ERRORSOCCURRED or
		//		m_hr == DB_E_ERRORSOCCURRED and input status != DBSTATUS_S_OK
		if (FAILED(m_hr) && ((m_hr == DB_E_ERRORSOCCURRED && ulInputStatus != DBSTATUS_S_OK) || m_hr != DB_E_ERRORSOCCURRED)
			&& !COMPARE(cRef, 1))
		{
			odtLog << L"Provider released storage object.\n";
			pIParamObject = NULL;	// So we don't release again below
		}
		else
			// Delete the storage object
			SAFE_DELETE(pIParamObject);

		// An update shouldn't open a rowset
		COMPARE(pIRowset, NULL);
		SAFE_RELEASE(pIRowset);

		// Validate the return code.  In some cases providers may return DB_E_ERRROSOCCURRED
		// rather than the more descriptive error we might expect, for example DB_E_BADORDINAL,
		// so we have to allow both.  
		switch(hrExp)
		{
			case E_NOINTERFACE:
			case DB_E_BADBINDINFO:
			case DB_E_BADORDINAL:
			case DB_E_BADSTORAGEFLAGS:
			case DB_E_UNSUPPORTEDCONVERSION:
				if (m_hr == DB_E_ERRORSOCCURRED)
					break;
			default:
				ABORT_CHECK(m_hr, hrExp);
		}

		// At this point we've done the test, break out of the column search loop
		break;	

	} // Next iCol

CLEANUP:

	// Reset the table to contain standard data by deleting all rows and reinserting
	if (CHECK(m_pTable->Delete(ALLROWS), S_OK))
	{
		for (iRow = 0; iRow < TOTAL_NUMBER_OF_ROWS; iRow++)
		{
			CHECK(m_pTable->Insert(iRow+1), S_OK);
		}
	}


	FREE_DATA(pvSrcData);

	SAFE_RELEASE(pIRowsetOpenObject);
	SAFE_RELEASE(pIRowset);
	SAFE_DELETE(pIParamObject);
	SAFE_FREE(pParameterData);
	
	// Release the accessors now
	m_pIAccessor->ReleaseAccessor(hParamAccessor, &cRefCount);
	FAIL_COMPARE(cRefCount, 0);

	return fResult ? TEST_PASS : TEST_FAIL;

}


BOOL TCIUnknown::MakeUnknownBinding(
	DBCOUNTITEM iBind,
	DBBINDING * pBind,
	DB_UPARAMS * pParamOrdinals,
	DBPARAMBINDINFO * pParamBindInfo,
	LPBYTE pData,
	DBOBJECT * pParamStorageObjects,
	REFIID iidParamObject,
	DWORD dwFlags,
	DBLENGTH cbRowSize,
	DB_UPARAMS cParamSets,
	ULONG ulUpdateRow,
	CCol ColInfo,
	enum PARAM_OBJECT_TEST eTest
	)
{
	BOOL fResult = FALSE;
	DB_UPARAMS iParamSet;
	DBTYPE wBaseType = ColInfo.GetProviderType();
	ICmdWParSequentialStream *	pIParamObject=NULL;
	LPVOID pvSrcData;
	ULONG cbSrcData, cbReadSize = 0;
	DBSTATUS sStatus = DBSTATUS_S_OK;

	// At this time we only know how to deal with ISequentialStream objects
	TESTC(IID_ISequentialStream == iidParamObject);

	// Set the pObject in the binding structure
	pBind[iBind].pObject = &pParamStorageObjects[iBind];

	// Adjust the cbMaxLen
	pBind[iBind].cbMaxLen = sizeof(IUnknown *);

	// Set as DBTYPE_IUnknown
	pBind[iBind].wType = DBTYPE_IUNKNOWN;

	// Fill in the pObject itself
	pParamStorageObjects[iBind].dwFlags = dwFlags;
	pParamStorageObjects[iBind].iid = iidParamObject;

	// Set the storage object pointer in the data structure
	for (iParamSet = 0; iParamSet < cParamSets; iParamSet++)
	{
		// Make a data item appropriate for this column and row
		pvSrcData = MakeData(ColInfo, &cbSrcData, ulUpdateRow + (ULONG)iParamSet, SECONDARY);
		TEST_PTR(pvSrcData);

		// Compute the read size
		if (eTest == EMPTY_STORAGE_OBJECT)
			cbReadSize = 0;
		else
		{
			// Use the data size, except for long data use MAX_LONG_VALUE
			// This way we can test sending large parameters in pieces via our ISequentialStream
			// implementation.
 			cbReadSize =  cbSrcData; 
			if (ColInfo.GetColumnSize() > MAX_LONG_VALUE)
			{
				cbReadSize = MAX_LONG_VALUE;  // Limit to something practical.

				// For WSTR columns we must have an even amount
				if (wBaseType == DBTYPE_WSTR)
				{
					if (cbReadSize % 2)
						cbReadSize--;
				}
				// For STR columns we cannot split a DBCS character
				if (wBaseType == DBTYPE_STR)
				{
					// See if this size would split a DBCS character and adjust if required.
					// We will split a DBCS character if the last byte to be sent is a lead byte.
					if (IsLastByteLeadByte(pvSrcData, cbSrcData, cbReadSize))
						cbReadSize--;
				}


				odtLog << L"         Insert size set to " << cbReadSize << L" bytes.\n";
			}
		}

		// A NULL storage object is legal, so don't create one for that case
		if (NULL_STORAGE_OBJECT != eTest)
		{
			// Create the storage object itself
			TESTC_(CreateStorageObject(iidParamObject, (IUnknown **)&pIParamObject), S_OK);

			// Addref the param object so it doesn't get deleted by Execute later.
			// Reference count must be 2 or greater to keep the object around after Execute
			// for non-NULL data.  For NULL data, the provider isn't allowed release the 
			// storage object
			if (NULL_DATA != eTest)
			{
				// AddRef once so the object won't get destructed when the provider releases it
				pIParamObject->AddRef();
				// AddRef again so when we call Release on the object after Execute it won't
				// destruct itself
				pIParamObject->AddRef();
			}

			// Initialize this stream object to use this particular source buffer and
			// allow the stream object to free the source buffer.
			TESTC_(pIParamObject->Init(pvSrcData, cbSrcData, cbReadSize, TRUE), S_OK);
		}
		else
			// Need to free the source data buffer because we can't free later
			SAFE_FREE(pvSrcData);

		// Set the status on input
		if (eTest == NULL_DATA)
			sStatus = DBSTATUS_S_ISNULL;
		else if (eTest == DEFAULT_DATA)
			sStatus = DBSTATUS_S_DEFAULT;

		STATUS_BINDING(pBind[iBind], pData+iParamSet*cbRowSize) = sStatus;

		// Set the length to the read size
		LENGTH_BINDING(pBind[iBind], pData+iParamSet*cbRowSize) = cbReadSize;

		// Set the value to the storage object
		*(IUnknown **)&VALUE_BINDING(pBind[iBind], pData+iParamSet*cbRowSize) = pIParamObject;

	}

	// Fill in the parameter ordinal
	pParamOrdinals[iBind] = iBind+1;

	// Fill in the DBPARAMBINDINFO information

	// ulParamSize is cch for character types.
	if (ColInfo.GetProviderType() == DBTYPE_WSTR)
		pParamBindInfo[iBind].ulParamSize = cbReadSize/sizeof(WCHAR);
	else
		pParamBindInfo[iBind].ulParamSize = cbReadSize;

	pParamBindInfo[iBind].bPrecision = (BYTE)ColInfo.GetPrecision();
	pParamBindInfo[iBind].bScale = (BYTE)ColInfo.GetScale();

	fResult = TRUE;

CLEANUP:

	return fResult;

}

TESTRESULT TCIUnknown::TestStorageObject(enum PARAM_OBJECT_TEST	eTest, REFIID iidParamObject,
	REFIID	iidVerifyObject, DB_UPARAMS cParamSets)
{
	BOOL					fResult = FALSE;
	CCol  					ColInfo;
	ULONG					cbSrcData = 0;
	LPVOID					pvSrcData=NULL;
	LPVOID					pvPrimaryData = NULL;
	LPVOID					pvReadBuf=NULL;
	LPVOID					pvCmpBuf=NULL; // Because we want to compare long data in pieces
	DBORDINAL				iCol;
	ULONG cRefCount, cbReadProv, cbReadSrc;
	DBCOUNTITEM				cRowsObtained;
	DBROWCOUNT				cRowsAffected;
	IRowset *				pIRowset=NULL;
	ISequentialStream *		pISeqStreamRow=NULL;
	ICmdWParSequentialStream *	pIParamObject=NULL;
	ICmdWParSequentialStream *	pIParamObjectCompare=NULL;
	HACCESSOR				hRowAccessor = DB_NULL_HACCESSOR,
							hParamAccessor = DB_NULL_HACCESSOR,
							hRowAccessorInLine = DB_NULL_HACCESSOR;
	HROW					rghRows[1];
	HROW *					prghRows=rghRows;
	UWORD					iFlag;
	DBSTATUS				ulExpStatus;	
	IAccessor *				pIAccessor=NULL;
	HRESULT					hrObject, hrInLine;
	ULONG					ulStorageInterface;
	DBPROPID				ColumnStorageProperty;
	LPWSTR					pwszDataItem = NULL;
	ULONG					ulUpdateRow = 1;
	CCol  					WhereColInfo;
	ULONG					cStorageObjects = 0;
	DBORDINAL				cParams = 0;
	ULONG					cUpdateParams = 0, iRow; 
	LPWSTR					pwszUpdateStmt = NULL;
	LPWSTR					pwszSelectStmt = NULL;
	DB_LORDINAL *			pParamColMap = NULL;
	DBBINDING *				pParamBindBase = NULL;
	DBBINDING *				pParamBind = NULL;
	DBCOUNTITEM				iBind, cParamBind = 0;// cUpdatableCols, cSearchableCols;
	DBLENGTH				cbParamRowSize = 0;
	DB_UPARAMS				iSet, iParam;
	DB_UPARAMS *			pParamOrdinals = NULL;
	DBPARAMBINDINFO *		pParamBindInfo = NULL;
	DBOBJECT *				pParamStorageObjects = NULL;
	LPBYTE					pData = NULL;
	ParamStruct *			pParamAll = NULL;
	DBOBJECT				VerifyStorageObject;
	BOOL					fTableHadNulls = FALSE;
	IRowset **				ppRowset = &pIRowset;
	ULONG					cStorage = 0;
	DBLENGTH				ulRowOffset = 0;

	VerifyStorageObject.iid = iidVerifyObject;
	VerifyStorageObject.dwFlags=STGM_READ;

	// Row binding info
	DBBINDING rgRowBind[1];
	rgRowBind[0].iOrdinal	=1;
	rgRowBind[0].obLength	=ulRowOffset;
	ulRowOffset += sizeof(DBLENGTH);
	rgRowBind[0].obStatus	=ulRowOffset;
	ulRowOffset += sizeof(DBSTATUS);
	ulRowOffset = ROUND_UP(ulRowOffset, ROUND_UP_AMOUNT);
	rgRowBind[0].obValue	=ulRowOffset;
	rgRowBind[0].pTypeInfo	=NULL;
	rgRowBind[0].pBindExt	=NULL;
	rgRowBind[0].dwPart		=DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
	rgRowBind[0].dwMemOwner	=DBMEMOWNER_CLIENTOWNED;
	rgRowBind[0].dwFlags	=0;
	rgRowBind[0].wType		=DBTYPE_IUNKNOWN;
	rgRowBind[0].bPrecision	=0;
	rgRowBind[0].bScale		=0;
	rgRowBind[0].pObject	=&VerifyStorageObject;
	rgRowBind[0].eParamIO	=DBPARAMIO_NOTPARAM;
	rgRowBind[0].cbMaxLen	=MAX_DATA_BUF+1;

	struct tagParameterData {
		DBLENGTH			ulDataSize;
		DBSTATUS			ulStatus;
		union {
			IUnknown *			pIUnknown;
			BYTE				bData[MAX_DATA_BUF+1];
		};
	} * pRowData;

	SAFE_ALLOC(pRowData, struct tagParameterData, 1);

	if (cParamSets > 1 && !g_bMultipleParamSets)
	{
		odtLog << "Multiple parameter sets are not supported \n";
		return TEST_SKIPPED;
	}

	// For cParamSets > 1 the table cannot have NULLS
	if (m_pTable->GetNull() == USENULLS && cParamSets > 1)
	{
		// Record that table did have nulls
		fTableHadNulls = TRUE;

		// Make the table think it doesn't have NULLS
		m_pTable->SetNull(NONULLS);

		TESTC_(m_pTable->Delete(ALLROWS), S_OK);
		for (iRow = 0; iRow < TOTAL_NUMBER_OF_ROWS; iRow++)
		{
			TESTC_(m_pTable->Insert(iRow+1), S_OK);
		}
	}

	// For Remoting and Kagera, must set DBPROP_ACCESSORDER if multiple BLOB columns or
	// only the BLOB at the end will be read properly.
	if (g_bKagera && GetModInfo()->GetClassContext() == CLSCTX_LOCAL_SERVER)
	{
		TESTC_(SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM), S_OK);
	}


	// We will be inserting SECONDARY data into the updatable columns.  This is fine for
	// those parameters that are for the column list of updatable cols, but we can't use
	// SECONDARY data in the 'where' clause.  To avoid this we need to limit the params used
	// to exclude those in the 'where' clause, which basically means we use the first N 
	// updatable columns.  Compute count of updatable columns.
	for (iCol = 1; iCol <= m_pTable->CountColumnsOnTable(); iCol++)
	{

		TESTC_(m_pTable->GetColInfo(iCol, ColInfo), S_OK);

		if (ColInfo.GetUpdateable())
			cUpdateParams++;
	}

	TESTC_(m_pTable->CreateSQLStmt(UPDATE_WITH_PARAMS_WHERE, NULL, &pwszUpdateStmt,&cParams, &pParamColMap, ulUpdateRow), S_OK);
	TESTC_(m_pTable->CreateSQLStmt(SELECT_VALIDATIONORDER, NULL, &pwszSelectStmt,NULL, NULL), S_OK);

	// Allocate a base binding array that does not use IUknown bindings
	SAFE_ALLOC(pParamBindBase, DBBINDING, cParams);

	// Allocate a duplicate binding array that we can modify
	SAFE_ALLOC(pParamBind, DBBINDING, cParams);

	// Allocate an array of ordinals
	SAFE_ALLOC(pParamOrdinals, DB_UPARAMS, cParams);

	// Allocate DBPARAMBINDINFO
	SAFE_ALLOC(pParamBindInfo, DBPARAMBINDINFO, cParams);

	// Allocate ParamStruct that holds provider type name and other info
	SAFE_ALLOC(pParamAll, ParamStruct, cParams);

	// Allocate a buffer of DBOBJECTS to use
	SAFE_ALLOC(pParamStorageObjects, DBOBJECT, cParams);

	for (iParam = 0; iParam < cParams; iParam++)
	{
		TESTC(AddParam(iParam, pParamColMap[iParam], DBPARAMIO_INPUT, NULL, FALSE,
			&cbParamRowSize, pParamBindBase, pParamBindInfo,  pParamAll, m_pTable));
		
		// Since later we will attempt to bind this parameter as a storage object we have 
		// to ensure there is adequate space for the IUnknown pointer.
		if (pParamBindBase[iParam].cbMaxLen < sizeof(IUnknown *))
		{
			// Need to adjust space
			cbParamRowSize += sizeof(IUnknown *) - pParamBindBase[iParam].cbMaxLen;
			pParamBindBase[iParam].cbMaxLen = sizeof(IUnknown *);
		}

		pParamOrdinals[iParam] = iParam+1;
	}

	// Adjust for alignment
//	cbParamRowSize = ROUND_UP(cbParamRowSize, ROUND_UP_AMOUNT);
	Repack(cParams, pParamBindBase, &cbParamRowSize);

	// Allocate a pData array of the appropriate size
	SAFE_ALLOC(pData, BYTE, cbParamRowSize * cParamSets);

	// Set the property values we need to use
	if (iidParamObject==IID_ISequentialStream)
	{
		ulStorageInterface=DBPROPVAL_SS_ISEQUENTIALSTREAM;
		ColumnStorageProperty = DBPROP_ISequentialStream;
	}
	else if (iidParamObject==IID_ILockBytes)
	{
		ulStorageInterface=DBPROPVAL_SS_ILOCKBYTES;
		ColumnStorageProperty = DBPROP_ILockBytes;
	}
	else if (iidParamObject==IID_IStorage)
	{
		ulStorageInterface=DBPROPVAL_SS_ISTORAGE;
		ColumnStorageProperty = DBPROP_IStorage;
	}
	else if (iidParamObject==IID_IStream)
	{
		ulStorageInterface=DBPROPVAL_SS_ISTREAM;
		ColumnStorageProperty = DBPROP_IStream;
	}
	else
	{
		// Not an object we know how to test
		return TEST_FAIL;
	}

	// Make sure this property (interface) is supported by the provider (data source)
	if (!(m_fSupportInterface = (m_ulStorageSupport & ulStorageInterface) > 0))
		odtLog << L"Storage object is not supported.\n";

	// Get a pointer to the verification object
	if (!SUCCEEDED(CreateStorageObject(iidVerifyObject, (IUnknown **)&m_pIUnkVerifyObj)))
	{
		odtLog << L"Unable to create verify storage object.\n";
		goto CLEANUP;
	}

	// Try all possible storage mode flags
	for (iFlag=0; iFlag < sizeof(g_StorageFlags)/sizeof(g_StorageFlags[0]); iFlag++)
	{
		// ISequentialStream doesn't use the Storage Flags so we only need to test
		// the data for STGM_READ if using ISequentialStream. There's no sense in doing
		// the same thing numerous times.
		if (iidParamObject == IID_ISequentialStream && g_StorageFlags[iFlag].dwMode != STGM_READ)
			continue;
		
		odtLog << g_StorageFlags[iFlag].wszMode << L"\n";

		// Create a DBPARAMS structure to pass to execute later
		DBPARAMS Params;

		// Row binding, copy param and modify
		if (eTest == MULTIPLE_OBJECTS)
		{
			// Make a copy of the base binding structure to modify
			memcpy(pParamBind, pParamBindBase, (size_t)(sizeof(DBBINDING) * cParams));

			// Fill the bindings with valid data based on normal bindings
			for (iSet = 0; iSet < cParamSets; iSet++)
			{
				LPBYTE pRow = pData+iSet*cbParamRowSize;

				TESTC_(FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cParams,
						pParamBindBase, (BYTE **)&pRow,
						ulUpdateRow + iSet, cParams, pParamColMap), S_OK);

			}
		}


		// We need to verify for all data types (columns) provider supports
		// Note that a column or parameter need not be specified as DBCOLUMNFLAGS_ISLONG
		// to allow the use of structured storage, but many providers will require that
		// anyway.
		for (iBind=0; iBind < cUpdateParams; iBind++)
		{
			iCol = pParamColMap[iBind];

			// Retrieve the information about this column
			ABORT_CHECK(m_pTable->GetColInfo(iCol, ColInfo), S_OK);

			// For multiple object testing we want to only include columns that will result
			// in a successful stream binding, meaning BLOB columns, which some providers 
			// use as a synonym for LONG columns.
			if (eTest == MULTIPLE_OBJECTS)
			{
				// Bind all BLOBS as storage
				if (iBind < cUpdateParams-1)
				{
					// Must have a BLOB column
					if (!ColInfo.GetIsLong())
						continue;

					odtLog << L"      " << L" - " << ColInfo.GetProviderTypeName() << L"\n";

					TESTC(MakeUnknownBinding(
							iBind,
							pParamBind,
							pParamOrdinals,
							pParamBindInfo,
							pData,
							pParamStorageObjects,
							iidParamObject,
							g_StorageFlags[iFlag].dwMode,
							cbParamRowSize,
							cParamSets,
							ulUpdateRow,
							ColInfo,
							eTest));

					cStorage++;

					continue;
				}

			}
			else
			{
				// Make a copy of the base binding structure to modify
				memcpy(pParamBind, pParamBindBase, (size_t)(sizeof(DBBINDING) * cParams));

				// Fill the bindings with valid data based on normal bindings
				for (iSet = 0; iSet < cParamSets; iSet++)
				{
					LPBYTE pRow = pData+iSet*cbParamRowSize;

					TESTC_(FillInputBindings(m_pTable, DBACCESSOR_PARAMETERDATA, cParams,
							pParamBindBase, (BYTE **)&pRow,
							ulUpdateRow + iSet, cParams, pParamColMap), S_OK);
				}

				odtLog << L"      " << L" - " << ColInfo.GetProviderTypeName() << L"\n";

				// Change this binding to be a storage object
				TESTC(MakeUnknownBinding(
						iBind,
						pParamBind,
						pParamOrdinals,
						pParamBindInfo,
						pData,
						pParamStorageObjects,
						iidParamObject,
						g_StorageFlags[iFlag].dwMode,
						cbParamRowSize,
						cParamSets,
						ulUpdateRow,
						ColInfo,
						eTest));
			}

			if (eTest == MULTIPLE_OBJECTS && cStorage <= 1)
			{
				odtLog << L"Only one BLOB column available for storage object.\n";
				fResult = TEST_SKIPPED;
				goto CLEANUP;
			}

			m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_PARAMETERDATA, cParams, pParamBind, cbParamRowSize,
				&hParamAccessor, NULL);

			// If requested interface is not supported then accessor validation may fail.
			// This may be delayed until the accessor is used.
			switch(m_hr)
			{
				case E_NOINTERFACE:
					COMPARE(m_fSupportInterface, FALSE);
					break;
				case DB_E_ERRORSOCCURRED:
					if (COMPARE(m_rgStatus[iBind],DBBINDSTATUS_BADSTORAGEFLAGS))
						odtLog << L"      Storage flag " << g_StorageFlags[iFlag].wszMode << L" is not supported for parameter accessors.\n";
					break;
				default:
					CHECK(m_hr, S_OK);
			}

			// Set command text to update statement
			TESTC_(m_pICommandText->SetCommandText(DBGUID_DBSQL, pwszUpdateStmt), S_OK);

			m_hr = m_pICommandWithParameters->SetParameterInfo(cParams, pParamOrdinals, pParamBindInfo);

			if (DB_S_TYPEINFOOVERRIDDEN != m_hr)
				TESTC_(m_hr, S_OK);

			// Set the params structure
			Params.pData=pData;
			Params.cParamSets=cParamSets;
			Params.hAccessor=hParamAccessor;

			// Perform the update.
			// At this time we don't have a provider supporting anything but ISequentialStream, so
			// we won't write consumer implementations of anything else.  If a provider DOES support
			// another interface, and DOESN'T bother to QI before using the interface passed in, then
			// we will crash here.  Otherwise QI will fail, and Execute will fail.
			m_hr = m_pICommand->Execute(NULL, IID_IRowset, &Params, &cRowsAffected, (IUnknown **)ppRowset);

			// An update shouldn't open a rowset
			COMPARE(pIRowset, NULL);
			SAFE_RELEASE(pIRowset);

			// Validate the return code
			switch(m_hr)
			{
				case E_NOINTERFACE:
					// Interface is not supported
					FAIL_COMPARE(m_fSupportInterface, FALSE);
					break;
				case DB_E_UNSUPPORTEDCONVERSION:
					// The conversion wasn't valid
					FAIL_COMPARE(S_OK != CanConvertIUnknown(m_pICommand, ColInfo.GetProviderType(),
							DBCONVERTFLAGS_PARAMETER), TRUE);
					break;
				case DB_E_BADSTORAGEFLAGS:
					// The Storage Mode flag was not supported or is invalid
					odtLog << L"      Storage flag " << g_StorageFlags[iFlag].wszMode << 
						L" is not supported for parameter accessors.\n";
					break;
				case DB_E_ERRORSOCCURRED:
					if (eTest == NULL_DATA && !ColInfo.GetNullable())
					{
						// The column wasn't nullable, check status
						FAIL_COMPARE(STATUS_BINDING(pParamBind[iBind], pData), DBSTATUS_E_INTEGRITYVIOLATION);
					}
					else if (!IsBLOB(ColInfo.GetProviderType()))
					{
						// HACK!!
						// Luxor reports conversion is supported for this case because they have a hack
						// to support 0-length conversions "BY DESIGN".
						if (!g_bLuxor || !(ColInfo.GetProviderType() == DBTYPE_VARIANT))
						{
							// The conversion wasn't valid so expect CanConvert to reflect that
							FAIL_COMPARE(S_OK != CanConvertIUnknown(m_pICommand, ColInfo.GetProviderType(),
									DBCONVERTFLAGS_PARAMETER), TRUE);
							// We will allow fixed length types to error here
							FAIL_COMPARE(STATUS_BINDING(pParamBind[iBind], pData), DBSTATUS_E_CANTCONVERTVALUE);
						}
					}
					else
					{
						FAIL_CHECK(m_hr, S_OK);
					}

					break;
				case E_FAIL:
					// If the provider doesn't support reading non-LONG data via stream objects then
					// this is the likely return code.  Don't fail provider for this case because it's
					// provider-specific.
					if (!ColInfo.GetIsLong())
						break;
					// Fall through
				default:
					FAIL_CHECK(m_hr, S_OK);
			}


			// Free storage objects

			// Release the accessor
			SAFE_RELEASE_ACCESSOR(m_pIAccessor, hParamAccessor);

			if (SUCCEEDED(m_hr))
			{

				ULONG iBind;
				CCol ColInfo;

				
				// We should have affected cParamSets rows
				COMPARE(cRowsAffected, (DBROWCOUNT)cParamSets);

				// Go through the bindings looking for the IUnknown ones
				for (iBind=0; iBind < cUpdateParams; iBind++)
				{
					ULONG cbReadSize = 0;

					// We will only verify the IUknown columns we updated at this time
					if (pParamBind[iBind].wType != DBTYPE_IUNKNOWN)
						continue;

					// Validate data for this binding
					iCol = pParamColMap[iBind];

					// Retrieve the information about this column
					ABORT_CHECK(m_pTable->GetColInfo(iCol, ColInfo), S_OK);

					// Get the parameter stream object for this column
					if (pParamBind[iBind].wType == DBTYPE_IUNKNOWN && 
						STATUS_BINDING(pParamBind[iBind], pData) == DBSTATUS_S_OK)
						pIParamObject = *(ICmdWParSequentialStream **)&VALUE_BINDING(pParamBind[iBind], pData);
					else
						pIParamObject = NULL;

					if (NULL_STORAGE_OBJECT == eTest)
					{
						// We don't have a stream to get the expected cbSrcData, etc. from, but since this
						// is the NULL storage object test we can just create it here.  This
						// will be padded with 0's or blanks if needed below.
						pvSrcData = MakeData(ColInfo, &cbSrcData, ulUpdateRow);
						TEST_PTR(pvSrcData);

						TESTC_(CreateStorageObject(iidParamObject, (IUnknown **)&pIParamObject), S_OK);

						if (NULL_DATA != eTest)
						{
							// AddRef once so the object won't get destructed when we release it
							pIParamObject->AddRef();
						}

						// If the type will be padded by the provider we have to
						// expect to read the full size
						if (ColInfo.GetIsFixedLength())
							cbReadSize = cbSrcData;

						// Initialize this stream object to use this particular source buffer and
						// allow the stream object to free the source buffer.
						TESTC_(pIParamObject->Init(pvSrcData, cbSrcData, cbReadSize, TRUE), S_OK);
					}

 					if (pIParamObject)
					{
						// Make sure the ref count is what we expect.  
						if (!COMPARE(pIParamObject->Release(), 1))
						{

							odtLog << L"Provider did not release storage object ";
							if (FAILED(m_hr))
								odtLog << L"on error.\n";
							else
								odtLog << L"on success.\n";
							
						}

						cbSrcData = pIParamObject->GetSrcBufferSize();
						cbReadSize = pIParamObject->GetReadSize();
						pvSrcData = pIParamObject->GetSrcBuffer();

						// Rewind our source ISequentialStream to point to beginning
						pIParamObject->Rewind();
					}

					// Allocate buffers to do reads and comparisons
					// Use one more byte than needed to make sure we don't write past required length
					TEST_ALLOC(BYTE, pvReadBuf, FILL_VALUE, cbSrcData+1);
					TEST_ALLOC(BYTE, pvCmpBuf, FILL_VALUE, cbSrcData+1);

					// Set the proper ordinal for the updated column
					rgRowBind[0].iOrdinal = iCol;
					rgRowBind[0].wType=DBTYPE_IUNKNOWN;
					rgRowBind[0].bPrecision=0;
					rgRowBind[0].bScale=0;

					// Release any previously existing row accessor
					SAFE_RELEASE_ACCESSOR(m_pIAccessor, hRowAccessor);

					// Create a row accessor for this row
					m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, rgRowBind, 0,
						&hRowAccessor, m_rgStatus);

					// If requested interface is not supported then accessor validation may fail.
					// This may be delayed until the accessor is used.
					switch(m_hr)
					{
						case E_NOINTERFACE:
							COMPARE(m_fSupportInterface, FALSE);
							break;
						case DB_E_ERRORSOCCURRED:
							if (COMPARE(m_rgStatus[0],DBBINDSTATUS_BADSTORAGEFLAGS))
								odtLog << L"      Storage flag " << g_StorageFlags[iFlag].wszMode << L" is not supported for row accessors.\n";
							break;
						default:
							CHECK(m_hr, S_OK);
					}


					// Verify the inserted data
					ABORT_CHECK(m_pICommandText->SetCommandText(DBGUID_DBSQL, pwszSelectStmt), S_OK);
					ABORT_CHECK(m_pICommand->Execute(NULL, IID_IRowset, NULL, &cRowsAffected, (IUnknown **)&pIRowset), S_OK);
					ABORT_CHECK(pIRowset->GetNextRows(NULL, ulUpdateRow-1, 1, &cRowsObtained, &prghRows), S_OK);

					// Set Inline result so we know whether to release inline objects
					hrInLine=E_FAIL;

					// Try to get the data using the IUnknown accessor
					hrObject=pIRowset->GetData(*prghRows, hRowAccessor, (void *)pRowData);

					// Check the return code from GetData
					switch(hrObject)
					{
						case DB_E_BADACCESSORHANDLE:
							// Create accessor failed for IUnknown row accessor
							FAIL_COMPARE(hRowAccessor, DB_NULL_HACCESSOR);
							break;
						case E_NOINTERFACE:
							// Interface is not supported
							FAIL_COMPARE(m_fSupportInterface, FALSE);
							break;
						case DB_E_UNSUPPORTEDCONVERSION:
							// The conversion wasn't valid
							FAIL_COMPARE(S_OK != CanConvertIUnknown(pIRowset, ColInfo.GetProviderType(),
									DBCONVERTFLAGS_COLUMN), TRUE);
							break;
						case DB_E_BADSTORAGEFLAGS:
							// The Storage Mode flag was not supported or is invalid
							odtLog << L"      Storage flag " << g_StorageFlags[iFlag].wszMode << 
								L" is not supported for row accessors.\n";
							break;
						default:
							FAIL_CHECK(hrObject, S_OK);
					}

					if (!SUCCEEDED(hrObject))
					{
						// Reading the data as an IUnknown object failed, read in-line instead
						pRowData->ulStatus = S_OK;

						// Update the inline binding data type
						rgRowBind[0].wType=ColInfo.GetProviderType();
						rgRowBind[0].bPrecision=ColInfo.GetPrecision();
						rgRowBind[0].bScale=ColInfo.GetScale();

						// Create a row accessor for this row
						// This must be on the rowset object, not the command object
						pIRowset->QueryInterface(IID_IAccessor, (void **)&pIAccessor);
						if (!pIAccessor)
							goto CLEANUP;

						ABORT_CHECK(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, rgRowBind, 0,
							&hRowAccessorInLine, m_rgStatus), S_OK);

						// We have to read the data as in-memory data all at once
						hrInLine=pIRowset->GetData(*prghRows, hRowAccessorInLine, (void *)pRowData);

					}

					// If neither of the read methods succeeded then we can't verify the data
					if (SUCCEEDED(hrObject) || SUCCEEDED(hrInLine))
					{
						// Check the status code
						switch (eTest)
						{
							case NULL_DATA:
								ulExpStatus = DBSTATUS_S_ISNULL;
								break;
							case NULL_STORAGE_OBJECT:
							case EMPTY_STORAGE_OBJECT:
								// We should only be able to read 0 bytes from the provider for
								// variable length data types.  Fixed length types are padded.
								if (!ColInfo.GetIsFixedLength())
									cbReadSize = 0;
								else
								{
									// The backend data should be nulls or zeros, so set source buffer
									// the same so we can compare
									switch (ColInfo.GetProviderType())
									{
										case DBTYPE_BYTES:
											// Padded with NULLs
											memset(pvSrcData, 0x00, cbSrcData);
											break;
										case DBTYPE_STR:
											// Padded with spaces
											memset(pvSrcData, 0x20, cbSrcData);
											break;
										case DBTYPE_WSTR:
											// Padded with wide spaces
											for (ULONG iChar=0; iChar < cbSrcData/sizeof(WCHAR); iChar++)
												*((WCHAR *)pvSrcData+iChar) = L' ';
											break;
									}
									// Since fixed length types were padded we should have data to read
									cbReadSize = cbSrcData;
								}
								// Fall through for expected status
							default:
								ulExpStatus = DBSTATUS_S_OK;
						}

						// If the status is what we exect we can verify the data
						// Note that some providers will fail here for NULL/empty storage objects because
						// they cannot create a 0 length column and instead either have one byte or show
						// it as DBSTATUS_S_ISNULL.
						if (COMPARE(pRowData->ulStatus, ulExpStatus))
						{
							switch(pRowData->ulStatus)
							{
								case DBSTATUS_S_ISNULL:
									// Status value compared above, nothing to do
									break;
								case DBSTATUS_S_TRUNCATED:
									// We can only compare the amount actually read
									cbReadSize = (ULONG)pRowData->ulDataSize;
									// Fall through
								case DBSTATUS_S_OK:
									if (hrObject == S_OK)
									{
										ULONG iRead = 0;

										cbReadSrc = 0;

										// Verify using storage object
										// RowData buffer is a pointer to an ISequentialStream instantiated by the provider over the column
										pISeqStreamRow = (ISequentialStream *)pRowData->pIUnknown;

										// Use the read method to get the data into our buffer
										do
										{
											HRESULT hrRead;

											// Set both read and compare buff to fill value as debugging aid for
											// partial reads
											memset(pvReadBuf, FILL_VALUE, cbSrcData+1);
											memset(pvCmpBuf, FILL_VALUE, cbSrcData+1);

											iRead++;

											// Read a buffer full of data from the provider
											hrRead = pISeqStreamRow->Read(pvReadBuf, cbSrcData, &cbReadProv);

											// Make sure the hresult is valid.  We really expect S_OK even when
											// no more data, but implementations are allowed to return S_FALSE 
											// for this condition.
											if (!(hrRead == S_OK || (hrRead == S_FALSE && cbReadProv < cbSrcData)))
												ABORT_CHECK(hrRead, S_OK);

											// Read data from our local copy in the size chunk we expect
											if (pIParamObject)
												ABORT_CHECK(pIParamObject->Read(pvCmpBuf, cbSrcData, &cbReadSrc), S_OK);
										
											// The read size should match
											if (COMPARE(cbReadProv, cbReadSrc))
											{
												// Compare this buffer with what was written.  Can't use CompareDBTypeData
												// because strings aren't null terminated (theoretically).
												FAIL_COMPARE(memcmp(pvCmpBuf, pvReadBuf, cbReadSrc), 0);

												// Make sure the fill value was not overwritten
												FAIL_COMPARE(*((BYTE *)pvReadBuf+cbReadSrc), FILL_VALUE);
											}
											else
												m_TestResult=TEST_FAIL;

										}
										while (cbReadSrc > 0 && cbReadProv == cbReadSrc);

										FAIL_COMPARE(pISeqStreamRow->Release(), 0);
										pISeqStreamRow=NULL;
									}
									else
									{
										// Fixed length data types will be padded so won't have zero length
										if (!ColInfo.GetIsFixedLength() &&
											(eTest == NULL_STORAGE_OBJECT ||
											eTest == EMPTY_STORAGE_OBJECT))
										{
											// We already compared status above, size must be zero.  Note that
											// CompareDBTypeData uses strcmp, which fails for 0 length data
											FAIL_COMPARE(pRowData->ulDataSize, 0);
										}
										else
										{
											// Have to verify using in-line data. 
											// Note: SQL based providers will fail here because they will pad
											// the data if fixed length char/binary.  But non-SQL providers
											// will most likely not follow this behavior.
											if (COMPARE(pRowData->ulDataSize, cbReadSize))
											{
												BYTE * pvData = pRowData->bData;

												// Rewind our source ISequentialStream to point to beginning
												if (pIParamObject)
													pIParamObject->Rewind();

												do
												{
													// Read data from our local copy in the size chunk we expect
													if (pIParamObject)
														ABORT_CHECK(pIParamObject->Read(pvCmpBuf, cbSrcData, &cbReadSrc), S_OK);

													FAIL_COMPARE(memcmp(pvCmpBuf, pvData, cbReadSrc), 0);

													pvData+=cbReadSrc;
												}
												while (cbReadSrc > 0);

												// String data must be null terminated
												if (ColInfo.GetProviderType() == DBTYPE_WSTR)
												{
													FAIL_COMPARE(((WCHAR *)pRowData->bData)[cbReadSize/sizeof(WCHAR)], L'\0');
												}
												if (ColInfo.GetProviderType() == DBTYPE_STR)
												{
													FAIL_COMPARE(((CHAR *)pRowData->bData)[cbReadSize], '\0');
												}
											}
											else
												m_TestResult = TEST_FAIL;

										}
									}
									break;
								default:
									ASSERT(!L"Need more case statements");
							}
						}
						else
							m_TestResult = TEST_FAIL;

						// If we retrieved in-line data then release it
						if (SUCCEEDED(hrInLine))
						{
							// We're done verifying, release the accessor now
							pIAccessor->ReleaseAccessor(hRowAccessorInLine, &cRefCount);
							FAIL_COMPARE(cRefCount, 0);
							FAIL_COMPARE(pIAccessor->Release(), 1);  // Rowset interface should be all that's left
							pIAccessor = NULL;
						}

					}
					else
					{
						// Neither of the read methods succeeded, unable to verify data
						// Force an error to be posted
						FAIL_COMPARE(hrInLine, S_OK);
					}

					ABORT_CHECK(pIRowset->ReleaseRows(1, rghRows, NULL, NULL, NULL), S_OK);

					RELEASE(pIRowset);

					// Release the parameter stream object
					SAFE_RELEASE(pIParamObject);

				} // Next binding

				// Reset the table to contain standard data by deleting all rows and reinserting
				TESTC_(m_pTable->Delete(ALLROWS), S_OK);
				for (iRow = 0; iRow < TOTAL_NUMBER_OF_ROWS; iRow++)
				{
					TESTC_(m_pTable->Insert(iRow+1), S_OK);
				}
			}

			// Free binding memory
			TESTC_(ReleaseInputBindingsMemory(cParams, pParamBind, pData, FALSE), S_OK);

		} // Next iCol

	} // Next iFlag

	fResult = TRUE;

CLEANUP:


//	FREE_DATA(pvSrcData); // Stream's data buffer is released by destructor
	SAFE_FREE(pRowData);
	FREE_DATA(pvPrimaryData);
	FREE_DATA(pvReadBuf);
	FREE_DATA(pvCmpBuf);

	RELEASE(pISeqStreamRow);
	RELEASE(pIRowset);
	RELEASE(m_pIUnkVerifyObj);
	
	// Release the accessors now
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, hParamAccessor);
	SAFE_RELEASE_ACCESSOR(m_pIAccessor, hRowAccessor);

	// Reset table parameters and delete any added rows
	if (fTableHadNulls)
	{
		m_pTable->SetNull(USENULLS);

		// Reset the table to contain standard data by deleting all rows and reinserting
		CHECK(m_pTable->Delete(ALLROWS), S_OK);
		for (iRow = 0; iRow < TOTAL_NUMBER_OF_ROWS; iRow++)
		{
			CHECK(m_pTable->Insert(iRow+1), S_OK);
		}
	}

	switch (fResult)
	{
		case TEST_SKIPPED:
			return TEST_SKIPPED;
		case TRUE:
			return TEST_PASS;
		default:
			return TEST_FAIL;
	}

}

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIUnknown::Init()
{
	BOOL InitSuccess = FALSE;
	CCol ColInfo;
	ULONG_PTR ulOLEMask = 0;
	IDBCreateCommand * pIDBCreateCommand=NULL;
	IUnknown * pDSO = m_pThisTestModule->m_pIUnknown;
	LPWSTR wszProviderName=NULL;

	m_pICommand=NULL;
	m_pICommandText=NULL;
	m_pICommandWithParameters=NULL;
	m_pIDBProperties=NULL;
	m_pIAccessor=NULL;
	m_fKagera=FALSE;
	m_pTable = NULL;
	m_pIUnkVerifyObj = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(COLEDB::Init())
	// }}
	{
		TEST_COMPARE(VerifyInterface(pDSO, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBProperties),TRUE);

		TEST_COMPARE(VerifyInterface(m_pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand),TRUE);

		// Get a command object
		TEST_CHECK(pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, 
			(IUnknown **)&m_pICommand), S_OK);

		RELEASE(pIDBCreateCommand);

		TEST_COMPARE(VerifyInterface(m_pICommand, IID_ICommandText, COMMAND_INTERFACE, (IUnknown**)&m_pICommandText),TRUE);

		TEST_COMPARE(VerifyInterface(m_pICommand, IID_ICommandWithParameters, COMMAND_INTERFACE, (IUnknown**)&m_pICommandWithParameters),TRUE);

		TEST_COMPARE(VerifyInterface(m_pICommand, IID_IAccessor, COMMAND_INTERFACE, (IUnknown**)&m_pIAccessor),TRUE);

		TEST_COMPARE(GetProperty(DBPROP_STRUCTUREDSTORAGE, DBPROPSET_DATASOURCEINFO, m_pIDBProperties, &m_ulStorageSupport),TRUE);

		GetProperty(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO, m_pIDBProperties, &wszProviderName);
		if (!wcscmp((LPWSTR)wszProviderName, L"MSDASQL.DLL"))
			m_fKagera=TRUE;
		PROVIDER_FREE(wszProviderName);

		if (GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, m_pIDBProperties, &ulOLEMask))
		{
			if (ulOLEMask & DBPROPVAL_OO_BLOB)
			{
				//Create a table just for this test case
				m_pTable = new CTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, 
					(LPWSTR)gwszModuleName);

				TEST_PTR(m_pTable);

				TEST_CHECK(m_pTable->CreateTable(TOTAL_NUMBER_OF_ROWS), S_OK)

				InitSuccess = TRUE;
			}
			else
				odtLog << L"Provider does not support structured storage objects, test case skipped.\n";
		}
		else
			odtLog << L"Unable to retrieve value of DBPROP_OLEOBJECTS.\n";
	}

CLEANUP:


	return InitSuccess;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ISequentialStream
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIUnknown::Variation_1()
{
	return TestStorageObject(VALID_INPUT_DATA, IID_ISequentialStream, IID_ISequentialStream);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ILockBytes
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIUnknown::Variation_2()
{
	odtLog << L"Not yet implemented.\n";
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IStorage
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIUnknown::Variation_3()
{
	odtLog << L"Not yet implemented.\n";
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IStream
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIUnknown::Variation_4()
{
	odtLog << L"Not yet implemented.\n";
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_MULTIPLESTORAGEOBJECTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIUnknown::Variation_5()
{
	odtLog << L"Not yet implemented.\n";
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_BLOCKINGSTORAGEOBJECTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIUnknown::Variation_6()
{
	odtLog << L"Not yet implemented.\n";
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc NULL Storage Object - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIUnknown::Variation_7()
{
	return TestStorageObject(NULL_STORAGE_OBJECT, IID_ISequentialStream, IID_ISequentialStream);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty storage object - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIUnknown::Variation_8()
{
	return TestStorageObject(EMPTY_STORAGE_OBJECT, IID_ISequentialStream, IID_ISequentialStream);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DBSTATUS_S_ISNULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIUnknown::Variation_9()
{
	return TestStorageObject(NULL_DATA, IID_ISequentialStream, IID_ISequentialStream);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Verify streams active after error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIUnknown::Variation_10()
{ 
	ULONG iActiveError;
	BOOL fResult = TEST_PASS;

	for (iActiveError = INVALIDARG_MULTRES;
			iActiveError < LAST_ACTIVE_ERROR;
			iActiveError++)
	{

		fResult &= TestActiveError((enum ACTIVE_ERROR)iActiveError, IID_ISequentialStream);
	}

	return fResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Verify streams released after error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIUnknown::Variation_11()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Multiple Paramsets
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIUnknown::Variation_12()
{ 
	return TestStorageObject(VALID_INPUT_DATA, IID_ISequentialStream, IID_ISequentialStream, 3);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Multiple Storage Objects
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCIUnknown::Variation_13()
{ 
	return TestStorageObject(MULTIPLE_OBJECTS, IID_ISequentialStream, IID_ISequentialStream);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIUnknown::Terminate()
{
	RELEASE(m_pIDBProperties);
	RELEASE(m_pICommand);
	RELEASE(m_pICommandWithParameters);
	RELEASE(m_pICommandText);
	RELEASE(m_pIAccessor);

	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
	}

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(COLEDB::Terminate());
}	// }}
// }}
// }}

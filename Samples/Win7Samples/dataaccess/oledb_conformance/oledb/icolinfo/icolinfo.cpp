//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module ICOLINFO.CPP | OLE DB IColumnsInfo tests for Provider, 
//

#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "modstandard.hpp"

#include "icolinfo.h"
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xf17d6f60, 0xd740, 0x11ce, { 0x88, 0xe7, 0x00, 0xaa, 0x00, 0xb5, 0xa9, 0x1b }};
DECLARE_MODULE_NAME("IColumnsInfo");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("The test module for IColumnsInfo interface.");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(False)
// }} TCW_MODULE_GLOBALS_END

//Check whether the provider is read only
BOOL g_fReadOnlyProvider = FALSE;
LPCOLESTR	g_pwszRowURL=NULL;
LPCOLESTR	g_pwszRowsetURL=NULL;
GUID		guidMod = { 0xf17d6f60, 0xd740, 0x11ce, { 0x88, 0xe7, 0x00, 0xaa, 0x00, 0xb5, 0xa9, 0x1b }};

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	
	// Get connection and session objects
	if (ModuleCreateDBSession(pThisTestModule))
	{
		//Check to see if the DSO is ReadOnly
		g_fReadOnlyProvider = GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, pThisTestModule->m_pIUnknown);
		g_pwszRowURL = NULL;
		g_pwszRowsetURL = NULL;

		// Create a table we'll use for the whole test module,
		// store it in pVoid for now
		pThisTestModule->m_pVoid = new CTable(
				(IUnknown *)pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

		if (!pThisTestModule->m_pVoid)
		{
			odtLog << wszMemoryAllocationError;
			return FALSE;
		}

		// Start with a table with 5 rows								 
		TERM(FAILED(((CTable *)pThisTestModule->m_pVoid)->CreateTable(5,1,NULL,PRIMARY,TRUE)));
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
	if (pThisTestModule->m_pVoid)
	{
		// Remove table from database
		((CTable *)pThisTestModule->m_pVoid)->DropTable();

		// Delete CTable object
		delete (CTable*)pThisTestModule->m_pVoid;
		pThisTestModule->m_pVoid = NULL;
	} 
	
	return ModuleReleaseDBSession(pThisTestModule);
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
	TCBase() { SetTestCaseParam(TC_Rowset); }

	//Set the m_fWarning and m_fBinder flags.
	virtual void SetTestCaseParam(ETESTCASE eTestCase = TC_Rowset)
	{
		m_eTestCase = eTestCase;

		switch(eTestCase)
		{
		case TC_Rowset:
			break;
		case TC_Cmd:
			break;
		case TC_OpenRW:
			break;
		case TC_Bind:
			break;
		case TC_IColInfo2:
			break;
		case TC_SingSel:
			break;
		default:
			ASSERT(!L"Unhandled Type...");
			break;
		};
	}

	//data
	ETESTCASE	m_eTestCase;
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class IColInfo : public CSessionObject, public TCBase
{
	public:
		// @cmember Which HRESULT should I expect 
		PREPARATION		m_ePreparation;
		// @cmember What method
		METHOD_CHOICE	m_Method;
		// @cmember Whether I want to check the structures coming back
		BOOL			m_fCheckIColumnsInfo;
		// @cmember Supported Properties
		DBPROPID		m_DBPROPID;

		// @cmember Count of rows affected, based on non-select statement
		DBROWCOUNT		m_cRowsAffected;

		// @cmember Interface pointers
		IDBInitialize *	m_pMyIDBInitialize;
		IColumnsInfo *	m_pIColumnsInfo;
		IColumnsInfo2 *	m_pIColumnsInfo2;
		ICommand *		m_pICommand;
		IRowset *		m_pIRowset;
		
		// @cmember GetColumnInfo params
		DBORDINAL		m_cColumns;
		DBCOLUMNINFO *	m_rgInfo;
		WCHAR *			m_pStringsBuffer;

		// @cmember IColumnsInfo2 params
		DBORDINAL		m_cColumns2;
		DBCOLUMNINFO*	m_rgInfo2;
		DBID *			m_rgColumnIDs2;

		DBORDINAL		m_cMasks;
		DBID*			m_rgMasks;

		BOOL			m_fDocSource;
		
		// @cmember MapColumnIDs params
		DBORDINAL		m_cColumnIDs;
		DBID *			m_rgColumnIDs;
		DBORDINAL*		m_rgColumns;

		// @cmember	Pointer to Row Object
		CRowObject*		m_pCRowObj;
		CRowset*		m_pCRowset;

		// @cmember CheckProperty
		IUnknown *		m_pIUnknown;

		// @cmember Ordinals of the SQL Statement
		DBORDINAL		m_cOrdTbl1;
		DB_LORDINAL*	m_prgOrdTbl1;
		DBORDINAL		m_cOrdTbl2;
		DB_LORDINAL*	m_prgOrdTbl2;

		// @member HResult
		HRESULT			m_hr;
		
		// @cmember Constructor
		IColInfo(const LPWSTR wszTestCaseName): CSessionObject(wszTestCaseName)
		{
			// Interface pointer
			m_pIColumnsInfo		= NULL;
			m_pIColumnsInfo2	= NULL;
			m_pICommand			= NULL;
			m_pIRowset			= NULL;
			m_pMyIDBInitialize	= NULL;
			
			m_Method			= INVALID_METHOD;
			m_fDocSource		= FALSE;
			m_hr				= E_FAIL;

			// GetColumnInfo
			m_cColumns			= 0;
			m_rgInfo			= NULL;
			m_pStringsBuffer	= NULL;

			m_cColumns2			= 0;
			m_rgInfo2			= NULL;
			m_rgColumnIDs2		= NULL;
			m_cMasks			= 0;
			m_rgMasks			= NULL;
			m_pCRowObj			= NULL;
			m_pCRowset			= NULL;
			
			// MapColumnIDs
			m_cColumnIDs		= 0;
			m_rgColumnIDs		= NULL;
			m_rgColumns			= NULL;
			
			m_pIUnknown			= NULL;
			m_cOrdTbl1			= 0;
			m_prgOrdTbl1		= NULL;
			m_cOrdTbl2			= 0;
			m_prgOrdTbl2		= NULL;
		};

		// @cmember Destructor
		virtual ~IColInfo()
		{
			// Interface pointer
			ASSERT(!m_pIColumnsInfo);
			ASSERT(!m_pIColumnsInfo2);
			ASSERT(!m_pICommand);
			ASSERT(!m_pIRowset);
			
			// GetColumnInfo
			ASSERT(!m_rgInfo);
			ASSERT(!m_pStringsBuffer);

			ASSERT(!m_rgInfo2);
			ASSERT(!m_rgColumnIDs2);
			ASSERT(!m_rgMasks);
			ASSERT(!m_pCRowObj);
			ASSERT(!m_pCRowset);
			
			// MapColumnIDs
			ASSERT(!m_rgColumnIDs);
			ASSERT(!m_rgColumns);
		};

		// @cmember Test case Init
		BOOL Init(EINTERFACE eInterface);

		// @cmember Test case Terminate
		BOOL Terminate();

		// @cmember GetDBIDs copies dbids form DBCOLUMNINFO structure and makes
		// param to pass to MapColIDs
		HRESULT GetDBIDs(
			BOOL			fcColumnIDs=TRUE,
			BOOL			frgColumnIDs=TRUE,
			BOOL			frgColumns=TRUE,
			ULONG			cExpectedRuns=1,
			BOOL			fPrep = TRUE
		);
		
		// @cmember MakeDBIDArrays copies dbids form DBCOLUMNINFO structure and makes
		// param to pass to MapColIDs
		HRESULT MakeDBIDArrays(
			BOOL			fcColumnIDs=TRUE,
			BOOL			frgColumnIDs=TRUE,
			BOOL			frgColumns=TRUE
		);

		// @cmember Compare Ordinal to CCol
		BOOL CheckOrdinal(HRESULT hr, BOOL bCheckOrder=TRUE);

		// @cmember Check HResult
		BOOL IfErrorParmsShouldBeNULL();
		
		// @cmember Check Correctness of each column
		void CheckEachColumn(HRESULT hr,DBPROPID PropID=0,EQUERY eSQLStmt=NO_QUERY);

		// @cmember Variation-specific initialization
		BOOL Init_Var();

		// @cmember Variation-specific initialization
		BOOL Free(BOOL fFreeIColInfo=TRUE);

		// @cmember Free the IColumnsInfo buffers
		BOOL FreeColumnInfo(DBORDINAL* pcColumns,DBCOLUMNINFO** prgInfo, OLECHAR** pStringsBuffer);

		// @cmember Executes MapColumnIDs on a rowset object
		HRESULT ExecuteMethod_row(
			BOOL			fcColumnIDs,
			BOOL			frgColumnIDs,
			BOOL			frgColumns,
			STATEMENTKIND	StmtKd,
			EQUERY			sqlStmt,
			WCHAR *			pStmt,
			EPREPARE		ePrepare,
			ULONG			cExpectedRuns=1,
			DBPROPID		prop=0
		);

		// @cmember Executes MapClumnIDs on a command object
		HRESULT ExecuteMethod_cmd(
			BOOL			fcColumnIDs,
			BOOL			frgColumnIDs,
			BOOL			frgColumns,
			STATEMENTKIND	StmtKd,
			EQUERY			sqlStmt,
			WCHAR *			pStmt,
			EPREPARE		ePrepare,
			ULONG			cExpectedRuns=1,
			DBPROPID		prop=0,
			BOOL			fSingSel=FALSE
		);

		// @cmember Obtains a row object from a rowset.
		BOOL	GetRowFromRowset(BOOL bColInfo2);

		// @cmember Obtains a row object from a command.
		BOOL	GetRowFromCommand(BOOL bColInfo2);

		// @cmember Obtains a row object from OpenRowset.
		BOOL	GetRowFromOpenRW(BOOL bColInfo2);

		// @cmember Obtains a row object by direct binding.
		BOOL	GetRowFromBind(BOOL bColInfo2);

		//Get IColumnsInfo interface on Row object
		BOOL	GetRowIColumnsInfo(ICommand* pIC, IUnknown** pICI);

		// @cmember Calls a method on the row object in m_pCRowObj.
		HRESULT	CallMethodOnRowObj(BOOL bColInfo2, METHOD_CHOICE method=INVALID_METHOD);

		// @cmember Checks the returned values of output params.
		BOOL	CheckParams(HRESULT hr, METHOD_CHOICE method);

		// @cmember Checks the row specific columns returned by calls
		// on a Row object.
		BOOL	CheckRowSpecificColumns(DBORDINAL cCols, DBCOLUMNINFO* rgInfo, DBORDINAL ulFirstOrd);

		// @cmember Checks the returned columns to see if the restrictions
		//were applied.
		BOOL	VerifyRestrictions();

		// compare the DBCOLUMNINFO struct except for:
		BOOL Compare_pwszName(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex,EQUERY eSQLStmt);
		BOOL Compare_pTypeInfo(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_iOrdinal(DBORDINAL iOrdinal,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_ulColumnSize(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_dwType(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_bPrecision(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_bScale(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_Columnid(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex,EQUERY eSQLStmt);

		BOOL Compare_DBCOLUMNFLAGS_CACHEDEFERRED(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_DBCOLUMNFLAGS_ISBOOKMARK(DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_DBCOLUMNFLAGS_ISFIXEDLENGTH(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_DBCOLUMNFLAGS_ISLONG(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_DBCOLUMNFLAGS_ISNULLABLE(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_DBCOLUMNFLAGS_ISROWID(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_DBCOLUMNFLAGS_ISROWVER(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_DBCOLUMNFLAGS_MAYBENULLABLE(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex,EQUERY eSQLStmt);
		BOOL Compare_DBCOLUMNFLAGS_MAYDEFER(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);
		BOOL Compare_DBCOLUMNFLAGS_WRITE(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex);

		void PrintInfo(DBCOLUMNINFO * dbColumnInfo, DBORDINAL ulIndex);
		BOOL GetPreparation();
		void SetRowsetProperties(GUID propset, DBPROPID propid);
};

void IColInfo::SetRowsetProperties(GUID propset, DBPROPID propid)
{
	//Set properties for rowsets
	DBPROP * rgDBProps;
	rgDBProps = (DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * 1);
	rgDBProps->dwPropertyID = propid;
	rgDBProps->dwOptions = DBPROPOPTIONS_REQUIRED;
	rgDBProps->vValue.vt = VT_BOOL;
	V_BOOL(&(rgDBProps->vValue)) = VARIANT_TRUE;	
	
	//Build the Set struct to set all our rowset properties
	m_cPropSets = 1;
	m_rgPropSets = (DBPROPSET *)PROVIDER_ALLOC(m_cPropSets * sizeof(DBPROPSET));
	m_rgPropSets->rgProperties = rgDBProps;
	m_rgPropSets->rgProperties->colid = DB_NULLID;
	m_rgPropSets->cProperties = 1;
	m_rgPropSets->guidPropertySet = propset;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	GetPreparation
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::GetPreparation()
{
	BOOL				fSucceed		 = FALSE;
	ICommand *			pICommand		 = NULL;
	ICommandPrepare *	pICommandPrepare = NULL;

	// Initialize
	m_ePreparation = NOTSUPPORTED;

	// Check for Command Support
	if (!m_pIDBCreateCommand)
		return TRUE;

	if (FAILED(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,(IUnknown **)&pICommand)))
		return FALSE;

	if (SUCCEEDED(pICommand->QueryInterface(IID_ICommandPrepare,(void **)&pICommandPrepare)))
		m_ePreparation = SUPPORTED;

	fSucceed = TRUE;

	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pICommandPrepare);

	return fSucceed;
}
		
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	PrintInfo
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void IColInfo::PrintInfo(DBCOLUMNINFO * dbColumnInfo, DBORDINAL ulIndex)
{		
	odtLog <<L"[" <<ulIndex <<L"] -> pwszName = " <<dbColumnInfo[ulIndex].pwszName <<ENDL;
	odtLog <<L"[" <<ulIndex <<L"] -> iOrdinal = " <<dbColumnInfo[ulIndex].iOrdinal <<ENDL;
	odtLog <<L"[" <<ulIndex <<L"] -> ulColumnSize = " <<dbColumnInfo[ulIndex].ulColumnSize <<ENDL;
	odtLog <<L"[" <<ulIndex <<L"] -> wType = " <<dbColumnInfo[ulIndex].wType <<ENDL;
	odtLog <<L"[" <<ulIndex <<L"] -> bPrecision = " <<dbColumnInfo[ulIndex].bPrecision <<ENDL;
	odtLog <<L"[" <<ulIndex <<L"] -> bScale = " <<dbColumnInfo[ulIndex].bScale <<ENDL;
	odtLog <<L"Flag that are supported are : " <<ENDL;
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_CACHEDEFERRED) odtLog <<L"CACHEDEFERRED ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK)	 odtLog <<L"ISBOOKMARK ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH) odtLog <<L"ISFIXEDLENGTH ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISLONG)		 odtLog <<L"ISLONG ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISNULLABLE)	 odtLog <<L"ISNULLABLE ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISROWID)		 odtLog <<L"ISROWID ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ISROWVER)		 odtLog <<L"ISROWVER ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_MAYBENULL)	 odtLog <<L"MAYBENULL ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_MAYDEFER)		 odtLog <<L"MAYDEFER ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_WRITE)		 odtLog <<L"WRITE ";
	if (dbColumnInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)	 odtLog <<L"WRITEUNKNOWN";
	odtLog <<ENDL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Init
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::Init(EINTERFACE eInterface)
{
	WCHAR *	pwszColList=NULL;

	if (COLEDB::Init())	
	{	
		// Have this testcase use the table created in ModuleInit, but don't
		// let table be deleted, since we'll use it for next test case
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		

		// Create a second table for the Join variations
		m_pTable2 = new CTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
		if (!SUCCEEDED(m_pTable2->CreateTable(0,1,NULL,PRIMARY,TRUE)))
			return FALSE;

		m_cOrdTbl2 = m_pTable2->CountColumnsOnTable();
		if(m_cOrdTbl2)
		{
			m_pTable2->CreateColList(FORWARD, &pwszColList, &m_cOrdTbl2, &m_prgOrdTbl2);
			PROVIDER_FREE(pwszColList);
		}

		// Set DSO pointer
		if (m_pThisTestModule->m_pIUnknown)
		{
			if(!VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBInitialize))
				return FALSE;
		}
		
		if (m_pIDBInitialize)
			m_pMyIDBInitialize = m_pIDBInitialize;

		// Create Data Source Object, Initialize, and get a IOpenRowset pointer.
		SetDBSession(m_pThisTestModule->m_pIUnknown2);

		// Fill in the Ordinals
		if (eInterface == ROWSET_INTERFACE)
		{
			m_cOrdTbl1 = m_pTable->CountColumnsOnTable();
			if(m_cOrdTbl1)
			{
				m_pTable->CreateColList(FORWARD, &pwszColList, &m_cOrdTbl1, &m_prgOrdTbl1);
				PROVIDER_FREE(pwszColList);
			}
		}

		// Make sure base classes are working
		if ((!m_pIDBCreateCommand) && (eInterface == COMMAND_INTERFACE))
		{
			odtLog << L"Commands not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}

		if (!GetPreparation())
			return FALSE;

		return TRUE;
	}  

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Terminate
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::Terminate()
{
	COLEDB::Terminate();
	
	SAFE_RELEASE(m_pIDBInitialize);
	PROVIDER_FREE(m_prgOrdTbl1);
	PROVIDER_FREE(m_prgOrdTbl2);
	m_cOrdTbl1 = 0;
	m_cOrdTbl2 = 0;

	// Drop the second table
	if(m_pTable2)
		CHECK(m_pTable2->DropTable(),S_OK);
	SAFE_DELETE(m_pTable2);

	// Release session object
	ReleaseDBSession();

	SAFE_DELETE(m_pCRowObj);
	SAFE_DELETE(m_pCRowset);

	return(CTestCases::Terminate());
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Init, inorder to clear properties, need to get rid of ICommand object
//  and get new ICommand Object 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::Init_Var()
{
	// Don't check columninfo by default
	m_fCheckIColumnsInfo = TRUE;	
	
	// Reset Params
	m_cRowsAffected = 0;
	m_cColumns		= 0;
	m_DBPROPID		= 0;
	m_cColumnIDs	= 0;
	m_cPropSets		= 0;
	m_rgPropSets	= NULL;
	m_hr			= E_FAIL;

	// Check for Commands
	if (m_pIDBCreateCommand)
		if (FAILED(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,(IUnknown **)&m_pICommand)))
			return FALSE;

	// Initialize to the Command Object
	m_pIUnknown	= m_pICommand;
	
	return TRUE;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Free_
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::Free(BOOL fFreeIColInfo)
{
	// Release 
	if(fFreeIColInfo)
	{
		SAFE_RELEASE(m_pIColumnsInfo);
		SAFE_RELEASE(m_pIColumnsInfo2);
	}
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pICommand);

	if (m_rgInfo == INVALID(DBCOLUMNINFO*))
		m_rgInfo = NULL;

	if (m_rgInfo2 == INVALID(DBCOLUMNINFO*))
		m_rgInfo2 = NULL;

	if (m_pStringsBuffer == INVALID(WCHAR*))
		m_pStringsBuffer = NULL;
			
	if (m_rgColumnIDs2!=INVALID(DBID*))
		SAFE_FREE(m_rgColumnIDs2)
	else
		m_rgColumnIDs2 = NULL;

	if (m_rgColumns!=INVALID(DBORDINAL*))
		SAFE_FREE(m_rgColumns)
	else
		m_rgColumns = NULL;

	if (m_rgColumnIDs!=INVALID(DBID*))
		SAFE_FREE(m_rgColumnIDs)
	else
		m_rgColumnIDs = NULL;

	if (m_rgMasks!=INVALID(DBID*))
		SAFE_FREE(m_rgMasks)
	else
		m_rgMasks = NULL;
	
	FreeColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer);
	FreeColumnInfo(&m_cColumns2,&m_rgInfo2,&m_pStringsBuffer);
	
	FreeProperties(&m_cPropSets, &m_rgPropSets);
	return TRUE;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Free_
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::FreeColumnInfo(DBORDINAL* pcColumns,DBCOLUMNINFO** prgInfo, OLECHAR** pStringsBuffer)
{
	TBEGIN;

	// Check to see if we need to free the buffer
	if( prgInfo )
		SAFE_FREE(*prgInfo);

	if( pStringsBuffer )
		SAFE_FREE(*pStringsBuffer);

	return TRUE;
}

/// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	ExecuteMethod_row
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT IColInfo::ExecuteMethod_row
(
	BOOL			fcColumnIDs,
	BOOL			frgColumnIDs,
	BOOL			frgColumns,
	STATEMENTKIND	StmtKd,
	EQUERY			sqlStmt,
	WCHAR *			pStmt,
	EPREPARE		ePrepare,
	ULONG			cExpectedRuns,
	DBPROPID		prop

)
{
	HRESULT hr	 = E_FAIL;
	IID		riid = IID_IRowset;

	// Make sure we cleanup
	SAFE_RELEASE(m_pIColumnsInfo);
	PROVIDER_FREE(m_prgOrdTbl1);
	m_cOrdTbl1 = 0;

	// SetText
	if (FAILED(hr=SetCommandText(m_pIMalloc,m_pICommand,m_pTable,m_pTable2->GetTableName(),
							StmtKd,sqlStmt,pStmt,&m_cOrdTbl1,&m_prgOrdTbl1)))
		goto CLEANUP;

	// Set property if writable
	if (SettableProperty(prop, DBPROPSET_ROWSET, m_pIDBInitialize))
		CHECK(hr=SetRowsetProperty(m_pICommand,DBPROPSET_ROWSET,prop),S_OK);
	else
	{
		// If the Property is ReadOnly and the value is the same exp. S_OK
		if (GetProperty(prop, DBPROPSET_ROWSET, m_pICommand))
			CHECK(hr=SetRowsetProperty(m_pICommand,DBPROPSET_ROWSET,prop),S_OK);
		else
		{
			hr=SetRowsetProperty(m_pICommand,DBPROPSET_ROWSET,prop);
			if(SUCCEEDED(hr))
			{
				CHECKW(hr, DB_E_ERRORSOCCURRED);
				odtLog <<wszPropertySet;
			}
			else
				CHECK(hr, DB_E_ERRORSOCCURRED);
		}
	}

	if (m_Method == MAPCOLID)
	{
		if (FAILED(hr=GetDBIDs(fcColumnIDs,frgColumnIDs,frgColumns,cExpectedRuns)))
			goto CLEANUP;
	}

	// Prepare the Statement
	if ((m_ePreparation == SUPPORTED) &&
		(FAILED(hr=PrepareCommand(m_pICommand,PREPARE,cExpectedRuns))))
		goto CLEANUP;

	if (FAILED(hr=m_pICommand->Execute(NULL,riid,NULL,
								&m_cRowsAffected,(IUnknown **) &m_pIRowset)))
		goto CLEANUP;

	if (FAILED(hr=m_pIRowset->QueryInterface(IID_IColumnsInfo,(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	if (m_Method == GETCOLINFO)
		return hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns,&m_rgInfo,&m_pStringsBuffer);
	else if (m_Method == MAPCOLID)
		return hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs,m_rgColumnIDs,m_rgColumns);

CLEANUP:

	odtLog << wszTestFailure;
	return hr;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	ExecuteMethod_cmd
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT IColInfo::ExecuteMethod_cmd
(
	BOOL			fcColumnIDs,
	BOOL			frgColumnIDs,
	BOOL			frgColumns,
	STATEMENTKIND	StmtKd,
	EQUERY			sqlStmt,
	WCHAR *			pStmt,
	EPREPARE		ePrepare,
	ULONG			cExpectedRuns,
	DBPROPID		prop,
	BOOL			fSingSel
)
{
	HRESULT		hr	 = E_FAIL;

	// Make sure we cleanup
	SAFE_RELEASE(m_pIColumnsInfo);
	PROVIDER_FREE(m_prgOrdTbl1);
	m_cOrdTbl1 = 0;

	// ICommandText::SetCommandText
	if (FAILED(hr=SetCommandText(m_pIMalloc,m_pICommand,m_pTable,m_pTable2->GetTableName(),
							StmtKd,sqlStmt,pStmt,&m_cOrdTbl1,&m_prgOrdTbl1)))
		goto CLEANUP;

	// Set property if writable
	if(prop)
	{
		if (SettableProperty(prop, DBPROPSET_ROWSET, m_pIDBInitialize))
			CHECK(hr=SetRowsetProperty(m_pICommand,DBPROPSET_ROWSET,prop),S_OK);
		else
		{
			// If the Property is ReadOnly and the value is the same exp. S_OK
			if (GetProperty(prop, DBPROPSET_ROWSET, m_pICommand))
				CHECK(hr=SetRowsetProperty(m_pICommand,DBPROPSET_ROWSET,prop),S_OK);
			else
			{
				hr=SetRowsetProperty(m_pICommand,DBPROPSET_ROWSET,prop);
				if(SUCCEEDED(hr))
				{
					CHECKW(hr, DB_E_ERRORSOCCURRED);
					odtLog <<wszPropertySet;
				}
				else
					CHECK(hr, DB_E_ERRORSOCCURRED);
			}
		}
	}

	// Get an IColumnsInfo pointer
	if(fSingSel)
	{
		TESTC(GetRowIColumnsInfo(m_pICommand, (IUnknown**)&m_pIColumnsInfo))
	}
	else
	{
		//Get IColumnsInfo off the command object
		if (FAILED(hr=m_pICommand->QueryInterface(IID_IColumnsInfo,(void **) &m_pIColumnsInfo)))
			goto CLEANUP;
	}

	if (m_Method == MAPCOLID)
	{
		if (FAILED(hr=GetDBIDs(fcColumnIDs,frgColumnIDs,frgColumns,cExpectedRuns, !fSingSel)))
			goto CLEANUP;
	}

	// Prepare the Statement
	if ((m_ePreparation == SUPPORTED) && (!fSingSel) &&
		(FAILED(hr=PrepareCommand(m_pICommand,ePrepare,cExpectedRuns))))
		goto CLEANUP;
	
	if (m_Method == GETCOLINFO)
	{
		hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns,&m_rgInfo,&m_pStringsBuffer);
		
		if(SUCCEEDED(hr))
		{
			// Adjust the column count for Joins
			if((sqlStmt == SELECT_LEFTOUTERJOIN) || (sqlStmt == SELECT_RIGHTOUTERJOIN))
				COMPARE(m_cColumns, DBORDINAL((m_cOrdTbl1 + m_cOrdTbl2) + (m_cColumns ? !m_rgInfo[0].iOrdinal : 0)));
			else
				COMPARE(m_cColumns, DBORDINAL(m_cOrdTbl1 + (m_cColumns ? !m_rgInfo[0].iOrdinal : 0)));
		}
	}
	else if (m_Method == MAPCOLID)
		hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs,m_rgColumnIDs,m_rgColumns);
	else
		hr=E_FAIL;

CLEANUP:
	return hr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GetRowFromRowset
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::GetRowFromRowset(BOOL bColInfo2)
{
	TBEGIN
	HRESULT		hr = E_FAIL;
	BOOL		bRowSup = FALSE;
	ULONG_PTR	ulOleObj = 0;
	ULONG_PTR	ulDSType = 0;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*		rghRows = NULL;

	//Required for using some ExtraLib functions.
	g_pIDBInitialize = m_pIDBInitialize;
	g_pIOpenRowset = m_pIOpenRowset;

	//Check if provider supports ROW Objects. If not, then SKIP
	//this test case.
	if(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &ulOleObj))
	{
		if((ulOleObj & DBPROPVAL_OO_ROWOBJECT) == DBPROPVAL_OO_ROWOBJECT)
			bRowSup = TRUE;
	}

	if(GetProperty(DBPROP_DATASOURCE_TYPE, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &ulDSType))
	{
		if(ulDSType & DBPROPVAL_DST_DOCSOURCE)
			m_fDocSource = TRUE;
	}

	m_pCRowset = new CRowset();

	//Create the parent Rowset of the row objects used for testing 
	//in the variations.
	m_pCRowset->SetProperty(DBPROP_CANHOLDROWS);
	m_pCRowset->SetProperty(DBPROP_IRowsetIdentity);
	m_pCRowset->SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(m_pCRowset->CreateRowset(USE_OPENROWSET, IID_IRowset, m_pTable),S_OK);
	TESTC_(m_pCRowset->RestartPosition(),S_OK)
	TESTC_(m_pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK)

	m_pCRowObj = new CRowObject();

	TEST3C_(hr = m_pCRowObj->CreateRowObject(m_pCRowset->pIRowset(), 
		rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS, E_NOINTERFACE)

	if(bRowSup)
		TEST2C_(hr, S_OK, DB_S_NOROWSPECIFICCOLUMNS)
	else
	{
		if(E_NOINTERFACE == hr)
			TESTC_PROVIDER(FALSE) // ROW objects are not supported.
		else
			COMPAREW(FAILED(hr), TRUE);
	}

	if(bColInfo2)
		TESTC_PROVIDER(VerifyInterface(m_pCRowObj->pIRow(), IID_IColumnsInfo2,
			ROW_INTERFACE, (IUnknown**)&m_pIColumnsInfo2))
	else
		TESTC(VerifyInterface(m_pCRowObj->pIRow(), IID_IColumnsInfo,
			ROW_INTERFACE, (IUnknown**)&m_pIColumnsInfo))

	TESTC(VerifyInterface(m_pCRowset->pIRowset(), IID_IUnknown,
		ROWSET_INTERFACE, (IUnknown**)&m_pIUnknown))

CLEANUP:
	SAFE_FREE(rghRows);
	TRETURN
} //GetRowFromRowset

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GetRowFromCommand
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::GetRowFromCommand(BOOL bColInfo2)
{
	TBEGIN
	HRESULT		hr = E_FAIL;
	BOOL		bRowSup = FALSE;
	ULONG_PTR	ulOleObj = 0;
	ULONG_PTR	ulDSType = 0;
	DBROWCOUNT	cRowsAffected = 0;
	IRow*		pIRow = NULL;

	//Required for using some ExtraLib functions.
	g_pIDBInitialize = m_pIDBInitialize;
	g_pIOpenRowset = m_pIOpenRowset;

	//Check if provider supports ROW Objects. If not, then SKIP
	//this test case.
	if(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &ulOleObj))
	{
		if((ulOleObj & DBPROPVAL_OO_SINGLETON) == DBPROPVAL_OO_SINGLETON)
			bRowSup = TRUE;
	}

	if(GetProperty(DBPROP_DATASOURCE_TYPE, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &ulDSType))
	{
		if(ulDSType & DBPROPVAL_DST_DOCSOURCE)
			m_fDocSource = TRUE;
	}

	TESTC_PROVIDER(m_pIDBCreateCommand)
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,
		(IUnknown **)&m_pICommand), S_OK)

	// SetText
	TESTC_(SetCommandText(m_pIMalloc,m_pICommand,m_pTable,m_pTable2->GetTableName(),
		eSELECT,SELECT_ALLFROMTBL,NULL,&m_cOrdTbl1,&m_prgOrdTbl1), S_OK)

	// Prepare the Statement
	if (m_ePreparation == SUPPORTED)
		TESTC_(PrepareCommand(m_pICommand,PREPARE,1), S_OK)

	TEST3C_(hr = m_pICommand->Execute(NULL,IID_IRow,NULL, &cRowsAffected,
		(IUnknown **) &pIRow), S_OK, DB_S_NOTSINGLETON, E_NOINTERFACE)
	if(bRowSup)
		TEST2C_(hr, S_OK, DB_S_NOTSINGLETON)
	else
	{
		if(E_NOINTERFACE == hr)
			TESTC_PROVIDER(FALSE) // ROW objects are not supported.
		else
			COMPAREW(FAILED(hr), TRUE);
	}

	m_pCRowObj = new CRowObject();
	TESTC_(m_pCRowObj->SetRowObject(pIRow), S_OK)

	if(bColInfo2)
		TESTC_PROVIDER(VerifyInterface(m_pCRowObj->pIRow(), IID_IColumnsInfo2,
			ROW_INTERFACE, (IUnknown**)&m_pIColumnsInfo2))
	else
		TESTC(VerifyInterface(m_pCRowObj->pIRow(), IID_IColumnsInfo,
			ROW_INTERFACE, (IUnknown**)&m_pIColumnsInfo))

	TESTC(VerifyInterface(m_pICommand, IID_IUnknown,
		COMMAND_INTERFACE, (IUnknown**)&m_pIUnknown))

CLEANUP:
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(m_pICommand);
	TRETURN
} //GetRowFromCommand


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GetRowFromOpenRW
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::GetRowFromOpenRW(BOOL bColInfo2)
{
	TBEGIN
	HRESULT		hr = E_FAIL;
	BOOL		bRowSup = FALSE;
	ULONG_PTR	ulOleObj = 0;
	ULONG_PTR	ulDSType = 0;
	DBROWCOUNT	cRowsAffected = 0;
	IRow*		pIRow = NULL;

	//Required for using some ExtraLib functions.
	g_pIDBInitialize = m_pIDBInitialize;
	g_pIOpenRowset = m_pIOpenRowset;

	//Check if provider supports ROW Objects. If not, then SKIP
	//this test case.
	if(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &ulOleObj))
	{
		if((ulOleObj & DBPROPVAL_OO_SINGLETON) == DBPROPVAL_OO_SINGLETON)
			bRowSup = TRUE;
	}

	if(GetProperty(DBPROP_DATASOURCE_TYPE, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &ulDSType))
	{
		if(ulDSType & DBPROPVAL_DST_DOCSOURCE)
			m_fDocSource = TRUE;
	}

	TEST3C_(hr = m_pIOpenRowset->OpenRowset(NULL, &(m_pTable->GetTableID()), 
		NULL, IID_IRow, 0, NULL, (IUnknown **) &pIRow), S_OK, DB_S_NOTSINGLETON, E_NOINTERFACE)

	if(bRowSup)
		TEST2C_(hr, S_OK, DB_S_NOTSINGLETON)
	else
	{
		if(E_NOINTERFACE == hr)
			TESTC_PROVIDER(FALSE) // ROW objects are not supported.
		else
			COMPAREW(FAILED(hr), TRUE);
	}

	m_pCRowObj = new CRowObject();
	TESTC_(m_pCRowObj->SetRowObject(pIRow), S_OK)

	if(bColInfo2)
		TESTC_PROVIDER(VerifyInterface(m_pCRowObj->pIRow(), IID_IColumnsInfo2,
			ROW_INTERFACE, (IUnknown**)&m_pIColumnsInfo2))
	else
		TESTC(VerifyInterface(m_pCRowObj->pIRow(), IID_IColumnsInfo,
			ROW_INTERFACE, (IUnknown**)&m_pIColumnsInfo))

	TESTC_(hr = m_pIOpenRowset->OpenRowset(NULL, &(m_pTable->GetTableID()), 
		NULL, IID_IRowset, 0, NULL, (IUnknown **) &m_pIUnknown), S_OK)

CLEANUP:
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(m_pICommand);
	TRETURN
} //GetRowFromOpenRW


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// GetRowFromBind
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::GetRowFromBind(BOOL bColInfo2)
{
	TBEGIN
	HRESULT			hr = E_FAIL;
	BOOL			bRowSup = FALSE;
	ULONG_PTR		ulOleObj = 0;
	ULONG_PTR		ulDSType = 0;
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	DBROWCOUNT		cRowsAffected = 0;
	IRow*			pIRow = NULL;
	IBindResource*	pIBR = NULL;
	IDBBinderProperties*	pIDBBP = NULL;

	//Required for using some ExtraLib functions.
	g_pIDBInitialize = m_pIDBInitialize;
	g_pIOpenRowset = m_pIOpenRowset;

	//Check if provider supports direct binding.
	TESTC_PROVIDER(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &ulOleObj) &&
		((ulOleObj & DBPROPVAL_OO_DIRECTBIND) == DBPROPVAL_OO_DIRECTBIND))

	//Check if provider supports ROW Objects. If not, then SKIP
	//this test case.
	if((ulOleObj & DBPROPVAL_OO_ROWOBJECT) == DBPROPVAL_OO_ROWOBJECT)
		bRowSup = TRUE;

	if(GetProperty(DBPROP_DATASOURCE_TYPE, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &ulDSType))
	{
		if(ulDSType & DBPROPVAL_DST_DOCSOURCE)
			m_fDocSource = TRUE;
	}

	if(GetModInfo()->GetRootBinder())
	{
		TESTC_PROVIDER(VerifyInterface(GetModInfo()->GetRootBinder(), IID_IBindResource,
			BINDER_INTERFACE, (IUnknown**)&pIBR))
		TESTC(VerifyInterface(pIBR, IID_IDBBinderProperties,
			BINDER_INTERFACE, (IUnknown**)&pIDBBP))

		TESTC(GetInitProps(&cPropSets, &rgPropSets))
		TESTC_(pIDBBP->SetProperties(cPropSets, rgPropSets), S_OK)
	}
	else
	{
		TESTC_PROVIDER(VerifyInterface(m_pIOpenRowset, IID_IBindResource,
			SESSION_INTERFACE, (IUnknown**)&pIBR))
	}

	if(!g_pwszRowURL)
	{
		g_pwszRowURL = GetModInfo()->GetParseObject()->GetURL(ROW_INTERFACE);
		if(!g_pwszRowURL)
			g_pwszRowURL = GetModInfo()->GetRootURL();
	}

	if(!g_pwszRowsetURL)
	{
		g_pwszRowsetURL = GetModInfo()->GetParseObject()->GetURL(ROWSET_INTERFACE);
		if(!g_pwszRowsetURL)
			g_pwszRowsetURL = GetModInfo()->GetRootURL();
	}

	TESTC_PROVIDER(g_pwszRowsetURL && wcslen(g_pwszRowsetURL)>3)

	TEST2C_(hr = pIBR->Bind(NULL, g_pwszRowURL, DBBINDURLFLAG_READ, DBGUID_ROW,
		IID_IRow, NULL, NULL, NULL, (IUnknown**)&pIRow), S_OK, DB_E_NOTSUPPORTED)
	if(bRowSup)
	{
		TESTC_(hr, S_OK)
		TESTC(pIRow != NULL)
	}
	else
	{
		if(DB_E_NOTSUPPORTED == hr)
			TESTC_PROVIDER(FALSE) // ROW objects are not supported.
		else
			COMPAREW(FAILED(hr), TRUE);
	}

	TESTC_(pIBR->Bind(NULL, g_pwszRowsetURL, DBBINDURLFLAG_READ, DBGUID_ROWSET,
		IID_IUnknown, NULL, NULL, NULL, (IUnknown**)&m_pIUnknown), S_OK)
	TESTC(m_pIUnknown != NULL)

	m_pCRowObj = new CRowObject();
	TESTC_(m_pCRowObj->SetRowObject(pIRow), S_OK)

	if(bColInfo2)
		TESTC_PROVIDER(VerifyInterface(m_pCRowObj->pIRow(), IID_IColumnsInfo2,
			ROW_INTERFACE, (IUnknown**)&m_pIColumnsInfo2))
	else
		TESTC(VerifyInterface(m_pCRowObj->pIRow(), IID_IColumnsInfo,
			ROW_INTERFACE, (IUnknown**)&m_pIColumnsInfo))

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIDBBP);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIBR);
	TRETURN
} //GetRowFromBind

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CallMethodOnRowObj
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::GetRowIColumnsInfo(ICommand* pIC, IUnknown** ppICI)
{
	TBEGIN

	IRow*	pIRow = NULL;

	TESTC(pIC && ppICI)

	//Get IColumnsInfo off the row object
	if(m_ePreparation == SUPPORTED)
		TESTC(SUCCEEDED(PrepareCommand(pIC,PREPARE,0)))

	TESTC(SUCCEEDED(pIC->Execute(NULL, IID_IRow, NULL, NULL, (IUnknown**)&pIRow)))

	TESTC(VerifyInterface(pIRow, IID_IColumnsInfo, ROW_INTERFACE, (IUnknown**)ppICI))

CLEANUP:
	SAFE_RELEASE(pIRow);
	TRETURN
} //GetRowIColumnsInfo

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CallMethodOnRowObj
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT	IColInfo::CallMethodOnRowObj(BOOL bColInfo2, METHOD_CHOICE method)
{
	HRESULT			hr = E_FAIL;
	METHOD_CHOICE	methodused = INVALID_METHOD;

	TESTC(m_pCRowObj != NULL)

	if(method==INVALID_METHOD)
		methodused = m_Method;
	else
		methodused = method;

	switch (methodused)
	{
		case GETCOLINFO:
			if(bColInfo2)
				hr = m_pIColumnsInfo2->GetColumnInfo(&m_cColumns2,&m_rgInfo2,&m_pStringsBuffer);
			else
				hr = m_pIColumnsInfo->GetColumnInfo(&m_cColumns2,&m_rgInfo2,&m_pStringsBuffer);
			break;
		case MAPCOLID:
			if(bColInfo2)
				hr = m_pIColumnsInfo2->MapColumnIDs(m_cColumnIDs,m_rgColumnIDs,m_rgColumns);
			else
				hr = m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs,m_rgColumnIDs,m_rgColumns);
			break;
		case GETRESCOLINFO:
			hr = m_pIColumnsInfo2->GetRestrictedColumnInfo(m_cMasks, m_rgMasks, 0, &m_cColumns2,&m_rgColumnIDs2,&m_rgInfo2,&m_pStringsBuffer);
			if(hr == DB_E_NOCOLUMN)
				odtLog<<L"INFO: No column matched the given mask.\n";
			break;
		default:
			TESTC(methodused==GETCOLINFO)
			break;
	} //switch

	TESTC(CheckParams(hr, methodused))

	if(GETRESCOLINFO==methodused && S_OK==hr && (m_rgInfo2!=NULL) && (m_rgColumnIDs2!=NULL))
		for(DBORDINAL ulIndex=0; ulIndex<m_cColumns2; ulIndex++)
			COMPARE(CompareDBID(m_rgColumnIDs2[ulIndex], m_rgInfo2[ulIndex].columnid), TRUE);

CLEANUP:
	return hr;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CheckParams
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::CheckParams(HRESULT hr, METHOD_CHOICE method)
{
	TBEGIN

	switch (method)
	{
		case GETCOLINFO:
			if(S_OK==hr)
			{
				if(m_cColumns2)
					TESTC(m_rgInfo2!=NULL && m_pStringsBuffer!=NULL)
				else
				{
					TESTC(!m_rgInfo2 && !m_pStringsBuffer)
					odtLog<<L"WARNING: No columns were returned by GetColumnInfo.\n";
				}
			}
			else //if FAILED(hr)
			{
				TESTC(!m_cColumns2 && !m_rgInfo2 && !m_pStringsBuffer)
			}
			break;
		case MAPCOLID:
			break;
		case GETRESCOLINFO:
			if(S_OK == hr)  //This is the only success code now.
			{
				TESTC(m_cColumns2>0 && m_pStringsBuffer!=NULL)
				TESTC(m_rgColumnIDs2!=NULL || m_rgInfo2!=NULL)
			}
			else
				TESTC(!m_cColumns2 && !m_rgColumnIDs2 && !m_rgInfo2 && !m_pStringsBuffer);
			break;
		default:
			TESTC(m_Method==GETCOLINFO)
			break;
	} //switch

CLEANUP:
	TRETURN
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// CheckRowSpecificColumns
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::CheckRowSpecificColumns(DBORDINAL cCols, DBCOLUMNINFO* rgInfo, DBORDINAL ulFirstOrd)
{
	TBEGIN
	DBORDINAL		ulIndex=0;

	if(cCols==0)
		goto CLEANUP;
	else
		TESTC(cCols>0 && rgInfo!=NULL)

	for(ulIndex=0; ulIndex<cCols; ulIndex++)
	{
		COMPARE(rgInfo[ulIndex].iOrdinal, ulFirstOrd+ulIndex);
		COMPARE(rgInfo[ulIndex].columnid.eKind == DBKIND_GUID_NAME ||
			rgInfo[ulIndex].columnid.eKind == DBKIND_GUID_PROPID ||
			rgInfo[ulIndex].columnid.eKind == DBKIND_NAME ||
			rgInfo[ulIndex].columnid.eKind == DBKIND_PROPID ||
			rgInfo[ulIndex].columnid.eKind == DBKIND_GUID , TRUE);
		COMPARE(rgInfo[ulIndex].dwFlags & DBCOLUMNFLAGS_ROWSPECIFICCOLUMN, DBCOLUMNFLAGS_ROWSPECIFICCOLUMN);
	}

CLEANUP:
	TRETURN
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// VerifyRestrictions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::VerifyRestrictions()
{
	TBEGIN
	DBORDINAL		iCol=0;

	if(!m_cMasks && !m_rgMasks)
		return TRUE;
	else
		TESTC(m_cMasks>0 && m_rgMasks!=NULL)

	if(!m_rgInfo2 && !m_rgColumnIDs2)
	{
		TESTC(!m_pStringsBuffer)
		odtLog<<L"INFO: No columns matched the restriction(s) provided.\n";
		return TRUE;
	}
	else
	{
		TESTC(m_cColumns2>0 && m_pStringsBuffer!=NULL)
		TESTC(m_rgColumnIDs2!=NULL || m_rgInfo2!=NULL)
	}

	for(iCol=0; iCol<m_cColumns2; iCol++)
	{
		BOOL	bMatch = FALSE;

		//if(m_rgInfo2[iCol].pwszName)
		//	COMPARE(wcscmp(m_rgInfo2[iCol].pwszName, m_rgInfo2[iCol].columnid.uName.pwszName), 0);
		if((iCol>0) && (m_rgInfo2 != NULL))
			COMPARE(m_rgInfo2[iCol].iOrdinal > m_rgInfo2[iCol-1].iOrdinal, TRUE);
	
		for(DBORDINAL iMask=0; iMask<m_cMasks && !bMatch; iMask++)
		{
			WCHAR*	pwszTemp = NULL;
			const DBID* pColumnID	= &(m_rgInfo2[iCol].columnid);
			if(m_rgInfo2)
				pColumnID = &(m_rgInfo2[iCol].columnid);
			else
				pColumnID = &(m_rgColumnIDs2[iCol]);
			const DBID* pColumnMask	= &m_rgMasks[iMask];
			DBKIND eKind = pColumnID->eKind;

			//Types ofcourse must match...
			if(eKind != pColumnMask->eKind)
				continue;

			switch(eKind)
			{
			case DBKIND_GUID_NAME:
				if((pColumnID->uGuid.guid == pColumnMask->uGuid.guid) &&
					(pColumnID->uName.pwszName))
				{
					pwszTemp = wcsstr(pColumnID->uName.pwszName, pColumnMask->uName.pwszName);
					if(pwszTemp && (wcscmp(pwszTemp, pColumnID->uName.pwszName)==0))
						bMatch = TRUE;
				}
				break;
			case DBKIND_GUID_PROPID:
				if((pColumnID->uGuid.guid == pColumnMask->uGuid.guid) &&
					(pColumnID->uName.ulPropid == pColumnMask->uName.ulPropid))
					bMatch = TRUE;
				break;
			case DBKIND_NAME:
				if(pColumnID->uName.pwszName)
				{
					pwszTemp = wcsstr(pColumnID->uName.pwszName, pColumnMask->uName.pwszName);
					if(pwszTemp && (wcscmp(pwszTemp, pColumnID->uName.pwszName)==0))
						bMatch = TRUE;
				}
				break;
			case DBKIND_PGUID_NAME:
				if((!pColumnID->uGuid.pguid && pColumnMask->uGuid.pguid) ||
					(pColumnID->uGuid.pguid && !pColumnMask->uGuid.pguid) )
					continue;
				if(pColumnID->uGuid.pguid && pColumnMask->uGuid.pguid &&
				   *pColumnID->uGuid.pguid != *pColumnMask->uGuid.pguid)
					continue;
				if(pColumnID->uName.pwszName)
				{
					pwszTemp = wcsstr(pColumnID->uName.pwszName, pColumnMask->uName.pwszName);
					if(pwszTemp && (wcscmp(pwszTemp, pColumnID->uName.pwszName)==0))
						bMatch = TRUE;
				}
				break;
			case DBKIND_PGUID_PROPID:
				if((!pColumnID->uGuid.pguid && pColumnMask->uGuid.pguid) ||
					(pColumnID->uGuid.pguid && !pColumnMask->uGuid.pguid) )
					continue;
				if(pColumnID->uGuid.pguid && pColumnMask->uGuid.pguid &&
				   *pColumnID->uGuid.pguid != *pColumnMask->uGuid.pguid)
					continue;
				if(pColumnID->uName.ulPropid == pColumnMask->uName.ulPropid)
					bMatch = TRUE;
				break;
			case DBKIND_PROPID:
				if(pColumnID->uName.ulPropid == pColumnMask->uName.ulPropid)
					bMatch = TRUE;
				break;
			case DBKIND_GUID:
				if(pColumnID->uGuid.guid == pColumnMask->uGuid.guid)
					bMatch = TRUE;
				break;
			default:
				//should not get here.
				COMPARE(TRUE, FALSE);
				break;
			}//switch
		}
		//Make sure we found a match for every column returned.
		TESTC(bMatch)
	}

CLEANUP:
	TRETURN
} //VerifyRestrictions


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	GetDBIDs
//
//	Take DBID part of DBCOLUMNINFO and build array of DBIDs
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT IColInfo::GetDBIDs(			
	BOOL	fcColumnIDs,
	BOOL	frgColumnIDs,
	BOOL	frgColumns,
	ULONG	cExpectedRuns,
	BOOL	fPrep
)
{
	HRESULT			hr			  = E_FAIL;
	IColumnsInfo *	pIColumnsInfo = NULL;

	if (!m_pICommand)
		return FALSE;

	if (FAILED(hr=m_pICommand->QueryInterface(IID_IColumnsInfo,
												(void **)&pIColumnsInfo)))
		return hr;

	// Prepare the Statement
	if ((m_ePreparation == SUPPORTED) && (fPrep) &&
		(FAILED(hr=PrepareCommand(m_pICommand,PREPARE,cExpectedRuns))))
		goto CLEANUP;

	if (!m_cColumns)
		if (FAILED(hr=pIColumnsInfo->GetColumnInfo(&m_cColumns, 
												&m_rgInfo, &m_pStringsBuffer)))
			goto CLEANUP;

	// Make array of DBIDs 
	if (FAILED(hr=MakeDBIDArrays(fcColumnIDs,frgColumnIDs,frgColumns)))
		goto CLEANUP;

	// Prepare the Statement
	if (m_ePreparation==SUPPORTED && fPrep)
		hr=PrepareCommand(m_pICommand,UNPREPARE,cExpectedRuns);

CLEANUP:

	SAFE_RELEASE(pIColumnsInfo);

	return hr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	MakeDBIDArrays
//
//	Take DBID part of DBCOLUMNINFO and build array of DBIDs
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
HRESULT IColInfo::MakeDBIDArrays(			
	BOOL	fcColumnIDs,
	BOOL	frgColumnIDs,
	BOOL	frgColumns
)
{
	// Make array of DBIDs 
	if (m_cColumns)
	{
		if (frgColumnIDs)
		{
			// Allocate memory for the array of DBID's
			m_rgColumnIDs = (DBID *) PROVIDER_ALLOC(sizeof(DBID) * m_cColumns);
			if (!m_rgColumnIDs) goto CLEANUP;

			// Copy values into members
			for(DBORDINAL ulIndex=0; ulIndex<m_cColumns; ulIndex++)
				memcpy(&(m_rgColumnIDs[ulIndex]),&(m_rgInfo[ulIndex].columnid),sizeof(DBID));
		}

		if (frgColumns)
		{
			// Allocate memory for the array of DBID's
			m_rgColumns = (DBORDINAL *) PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cColumns);
			if (!m_rgColumns) goto CLEANUP;

			// Copy values into members
			for(DBORDINAL ulIndex=0; ulIndex<m_cColumns; ulIndex++)
				m_rgColumns[ulIndex] = INVALID(DBORDINAL);
		}

		// Set count of DBID's
		if (fcColumnIDs)
			m_cColumnIDs = m_cColumns;

		return NOERROR;
	}

CLEANUP:
	
	return E_FAIL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	CheckOrdinal
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL IColInfo::CheckOrdinal(HRESULT hr, BOOL bCheckOrder)
{
	ULONG cErrors = 0;
	DBORDINAL cUnInit = 0;

	// Loop thru the IDs to see if there is an invalid one
	for(DBORDINAL ulIndex = 0; ulIndex < m_cColumnIDs; ulIndex++) 
	{
		// Set to the count on E_INVALIDARG
		if( !m_rgColumns )
		{
			cUnInit = m_cColumnIDs;
			break;
		}

		if( m_rgColumns[ulIndex] == DB_INVALIDCOLUMN )
			cErrors++;

		if( m_rgColumns[ulIndex] == INVALID(DBORDINAL) )
			cUnInit++;
	}

	if((S_OK==hr) && bCheckOrder) {
		for(DBORDINAL ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++) 
		{
			DBORDINAL ulFirst;
			// Get the Oridinal
			for(ulFirst=0; ulFirst<m_cColumns; ulFirst++) 
			{
				if( CompareDBID(m_rgColumnIDs[ulIndex],
								m_rgInfo[ulFirst].columnid) )
					break;
			}
			
			// Compare the ordinal
			if( m_rgColumns[ulIndex] )
				COMPARE(m_rgColumns[ulIndex], m_rgInfo[ulFirst].iOrdinal);
		}
	}

	// Check the ReturnCode
	switch( hr )
	{
		case S_OK:
			TESTC(!cErrors);
			TESTC(!cUnInit);
			break;

		case DB_S_ERRORSOCCURRED:
			TESTC(cErrors > 0);
			TESTC(!cUnInit);
			TESTC(cErrors < m_cColumnIDs);
			break;

		case DB_E_ERRORSOCCURRED:
			TESTC(cErrors > 0);
			TESTC(!cUnInit);
			TESTC(cErrors == m_cColumnIDs);
			break;

		default:
			TESTC(!cErrors);
			TESTC(cUnInit > 0);
			TESTC(cUnInit == m_cColumnIDs);
			break;
	}

	return TRUE;

CLEANUP:

	return FALSE;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @mfunc General Compare routine that handles each part of the 
// DBCOLUMNINFO struct and the return code
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void IColInfo::CheckEachColumn
(
	HRESULT			hr,
	DBPROPID		PropID,
	EQUERY			eSQLStmt
)
{
	CCol			ccol;
	CTable *		pLocalCTable = NULL;
	DBORDINAL 		cLocalOrd    = 0;
	DB_LORDINAL*	pLocalrgOrd  = NULL;
	DBORDINAL		ulCount	  = 0;
	
	// If Success with a positive column count
	if (SUCCEEDED(hr) && m_cColumns)
	{
		// If on the Rowset change to the Rowset Object
		if (m_pIRowset)
			m_pIUnknown = m_pIRowset;

		//TO DO - if PropID==IRowsetLocate OR BOOKMARKS, then first
		//ordinal==0.

		// If bookmarks are present expect a 0 ordinal
		if (GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIUnknown))
			if(!COMPARE(m_rgInfo[0].iOrdinal, 0))
				odtLog<<L"Bookmarks are present but there is no Ordinal 0"<<ENDL;
			
		for(DBORDINAL ulIndex=0;ulIndex<m_cColumns;ulIndex++)
		{
			// Check for Ordinal 0
			if (m_rgInfo[ulIndex].iOrdinal==0)
			{
				COMPARE(Compare_DBCOLUMNFLAGS_ISBOOKMARK(m_rgInfo[ulIndex], ulIndex),TRUE);
				Compare_Columnid(ccol,m_rgInfo[ulIndex],ulIndex,eSQLStmt);
			}
			else
			{
				// Change the table for joins
				pLocalCTable = (CTable*)((ulCount < m_cOrdTbl1) ? m_pTable : m_pTable2);
				pLocalrgOrd = ((ulCount < m_cOrdTbl1) ? m_prgOrdTbl1 : m_prgOrdTbl2);
				cLocalOrd = ((ulCount < m_cOrdTbl1) ? m_cOrdTbl1 : m_cOrdTbl2);
				
				// Check the IColumnsInfo data
				if ((cLocalOrd) && (eSQLStmt != SELECT_COUNT) &&
					(SUCCEEDED(pLocalCTable->GetColInfo(pLocalrgOrd[ulCount%cLocalOrd],ccol))) )
				{
					Compare_pwszName(ccol,m_rgInfo[ulIndex],ulIndex,eSQLStmt);
					Compare_pTypeInfo(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_iOrdinal((DBORDINAL)ulIndex+(m_rgInfo[0].iOrdinal),m_rgInfo[ulIndex],ulIndex);
					Compare_ulColumnSize(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_dwType(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_bPrecision(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_bScale(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_Columnid(ccol,m_rgInfo[ulIndex],ulIndex,eSQLStmt);
			
					Compare_DBCOLUMNFLAGS_CACHEDEFERRED(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_DBCOLUMNFLAGS_ISBOOKMARK(m_rgInfo[ulIndex],ulIndex);
					Compare_DBCOLUMNFLAGS_ISFIXEDLENGTH(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_DBCOLUMNFLAGS_ISLONG(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_DBCOLUMNFLAGS_ISNULLABLE(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_DBCOLUMNFLAGS_ISROWID(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_DBCOLUMNFLAGS_ISROWVER(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_DBCOLUMNFLAGS_MAYBENULLABLE(ccol,m_rgInfo[ulIndex],ulIndex,eSQLStmt);
					Compare_DBCOLUMNFLAGS_MAYDEFER(ccol,m_rgInfo[ulIndex],ulIndex);
					Compare_DBCOLUMNFLAGS_WRITE(ccol,m_rgInfo[ulIndex],ulIndex);
				}
				else
				{
					odtLog<< L"This is a Calculated column.\n";
					PrintInfo(m_rgInfo, ulIndex);
				}
				
				// Incement the count
				ulCount++;
			}
		}
	}
	else if (SUCCEEDED(hr) && (!m_cColumns))
	{
		// If cColumns is 0, the buffers are NULL
		if (m_rgInfo || m_pStringsBuffer)
		{
			odtLog<< L"ERROR: The rgInfo pointer or pStringsBuffer was not NULL where cColumns was 0.\n";
			COMPARE(1, NULL);
		}
	}
	else
		COMPARE(IfErrorParmsShouldBeNULL(), TRUE);
}

//--------------------------------------------------------------------
// @mfunc Utility function that checks that the params of 
// IColumnsInfo::GetColumnInfo are correctly null if there is 
// an error as the return code
//
// hr must be a failure code before coming into this function
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::IfErrorParmsShouldBeNULL()
{
	// Check the pointers on ERROR
	if (!m_rgInfo && !m_pStringsBuffer && !m_cColumns)
		return TRUE;
	
	odtLog<< L"ERROR: The rgInfo pointer or pStringsBuffer was not NULL on a Error.\n";
	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.pwszName to the 
// column name in CCol.m_pwszColName
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_pwszName(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex,EQUERY eSQLStmt)
{
	// Get the Column name
	WCHAR wszColumnName[100] = L"";
	
	if( col.GetColName() )
		wcscpy(wszColumnName,col.GetColName());

	// Check to see if the Column names have been changed
	if( eSQLStmt == SELECT_CHANGECOLNAME )
		wcscat(wszColumnName,L"X");

	// Compare two strings, regarding upper and lower case
	if( ((wszColumnName) && (dbColumnInfo.pwszName) && 
		 (wcscmp(wszColumnName,dbColumnInfo.pwszName) == 0)) ||
		(!(*wszColumnName) && !(dbColumnInfo.pwszName)) )
		return TRUE;
	else 
		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->NAME]: expected "
			<<(*wszColumnName ? wszColumnName : L"<EMPTY>")  <<L", returned "<<dbColumnInfo.pwszName <<ENDL;

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares pTypeInfo
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_pTypeInfo(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	// Reserved
	if( COMPARE(dbColumnInfo.pTypeInfo, NULL) )
		return TRUE;
	else 
		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->ITYPEINFO]: expected NULL." <<ENDL;

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.iOrdinal to the index.
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_iOrdinal(DBORDINAL iOrdinal, DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	// Compare the columns Ordinals
	if( COMPARE(iOrdinal, dbColumnInfo.iOrdinal) )
		return TRUE;
	else 
		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->NUMBER]: expected "
			<<iOrdinal <<L", returned " <<dbColumnInfo.iOrdinal <<ENDL;

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.wType to the 
// column name in CCol.m_wProviderType
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_dwType(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	// Compare the Types
	if( COMPARE(col.GetProviderType(), dbColumnInfo.wType) )
		return TRUE;
	else 
		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->TYPE]: expected "
			<<col.GetProviderType() <<L", returned " <<dbColumnInfo.wType <<ENDL;

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.ulColumnSize to the 
// column name in CCol.m_lPrecisionUsed
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_ulColumnSize(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	DBLENGTH ulHolder = ~0;

	// Compare ulColumnsize as though it were count of objects
	if( IsFixedLength(col.GetProviderType()) )
		ulHolder=GetDBTypeSize(col.GetProviderType());
	else
		ulHolder=col.GetColumnSize();

	// Adjust for the VARNUMERIC
	if( col.GetProviderType() == DBTYPE_VARNUMERIC )
		ulHolder = 20;

	// Check the size
	if( COMPARE(ulHolder, dbColumnInfo.ulColumnSize) )
		return TRUE;
	else
		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType 
			<<L" ->COLUMNSIZE]: length of column expected "
			<<ulHolder <<L", received " <<dbColumnInfo.ulColumnSize <<ENDL;

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.bPrecision to the 
// column name in CCol.m_lPrecision
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_bPrecision(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	BYTE bNumericPrec = ~0;

	// Get the Precision of the Column
	if( col.GetProviderType() != DBTYPE_DBTIMESTAMP )
		bNumericPrec=bNumericPrecision(col.GetProviderType(), col);
	else
		bNumericPrec=col.GetPrecision();

	if( COMPARE(dbColumnInfo.bPrecision, bNumericPrec) )
		return TRUE;
	else
		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType 
			<<L" ->PRECISION]: precision of column expected " <<bNumericPrec
			<<L", received " <<dbColumnInfo.bPrecision <<ENDL;

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.bScale to the 
// scale in CCol.m_sScale
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_bScale(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	BYTE bNumericScale = ~0;

	// Get the Precision of the Column
	if( IsScaleType(col.GetProviderType()) )
		bNumericScale=col.GetScale();

	if( COMPARE(bNumericScale, dbColumnInfo.bScale) )
		return TRUE;
	else
		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType 
			<<L" ->SCALE]: expects "
			<<col.GetScale() <<L", returned " 
			<<dbColumnInfo.bScale <<ENDL;

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.columnid to the 
// column name in CCol.
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_Columnid(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex,EQUERY eSQLStmt)
{
	BOOL	fSuccess = FALSE;
	WCHAR	wszColumnName[100] = L"";
	DBORDINAL	ulOrdinal = 0;
	ULONG_PTR	ulIDType = 0;
	DBID *	rgColumnIDs = NULL;

	// Get the property value
	GetProperty(DBPROP_PERSISTENTIDTYPE, DBPROPSET_DATASOURCEINFO, m_pMyIDBInitialize, &ulIDType);

	// Get the Column name
	if (col.GetColName())
		wcscpy(wszColumnName,col.GetColName());

	// This should cover DBKIND_GUID_NAME
	if ((dbColumnInfo.columnid).eKind == DBKIND_GUID_NAME)
	{
		// Check the property value being returned
		if (!ulIDType)
			odtLog << L"Should return DBPROPVAL_PT_GUID_NAME for DBPROP_PERSISTENTIDTYPE" <<ENDL;
		else
			COMPARE(ulIDType, DBPROPVAL_PT_GUID_NAME);

		// Check to see if the Column names have been changed
		if (eSQLStmt == SELECT_CHANGECOLNAME)
			wcscat(wszColumnName,L"X");

		// Compare two strings, regarding upper and lower case
		if (((wszColumnName) && ((dbColumnInfo.columnid).uName.pwszName) && 
			 (wcscmp(wszColumnName,(dbColumnInfo.columnid).uName.pwszName) == 0)) ||
			(!(*wszColumnName) && !((dbColumnInfo.columnid).uName.pwszName)))
			return TRUE;
		else 
		{
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->COLUMNID]: expected "
				<<(*wszColumnName ? wszColumnName : L"<EMPTY>") <<L", returned "<<(dbColumnInfo.columnid).uName.pwszName <<ENDL;
			return FALSE;
		}
	}
	// This should cover DBKIND_GUID_PROPID
	else if ((dbColumnInfo.columnid).eKind == DBKIND_GUID_PROPID)
	{
		// Check the property value being returned
		if (!ulIDType)
			odtLog << L"Should return DBPROPVAL_PT_GUID_PROPID for DBPROP_PERSISTENTIDTYPE" <<ENDL;
		else if (m_rgInfo[ulIndex].iOrdinal)
			COMPARE(ulIDType, DBPROPVAL_PT_GUID_PROPID);

		// If Bookmark column
		if (!m_rgInfo[ulIndex].iOrdinal)
		{
			// Compare two iOrdinal and ulPropid
			if ((2 == (dbColumnInfo.columnid).uName.ulPropid) &&
				((dbColumnInfo.columnid).uGuid.guid == DBCOL_SPECIALCOL))
				return TRUE;
			else 
			{
				odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
					<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
					<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->COLUMNID]: expected "
					<<m_rgInfo[ulIndex].iOrdinal <<L", returned "<<(dbColumnInfo.columnid).uName.ulPropid <<ENDL;
				return FALSE;
			}
		}
		else
		{
			// Get the Ordinal back from MapColumnIDs
			rgColumnIDs = &dbColumnInfo.columnid;
			if(m_pIColumnsInfo)
				CHECK(m_pIColumnsInfo->MapColumnIDs(1,rgColumnIDs,&ulOrdinal), S_OK);
			else if(m_pIColumnsInfo2)
				CHECK(m_pIColumnsInfo2->MapColumnIDs(1,rgColumnIDs,&ulOrdinal), S_OK);
			rgColumnIDs=NULL;

			// Compare two iOrdinal and ulPropid
			if (ulOrdinal == m_rgInfo[ulIndex].iOrdinal)
				return TRUE;
			else 
			{
				odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
					<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
					<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->COLUMNID]: expected "
					<<m_rgInfo[ulIndex].iOrdinal <<L", returned "<<(dbColumnInfo.columnid).uName.ulPropid <<ENDL;
				return FALSE;
			}
		}
	}
	// This should cover DBKIND_NAME
	else if ((dbColumnInfo.columnid).eKind == DBKIND_NAME)
	{
		// Check the property value being returned
		if (!ulIDType)
			odtLog << L"Should return DBPROPVAL_PT_NAME for DBPROP_PERSISTENTIDTYPE" <<ENDL;
		else
			COMPARE(ulIDType, DBPROPVAL_PT_NAME);

		// Check to see if the Column names have been changed
		if (eSQLStmt == SELECT_CHANGECOLNAME)
			wcscat(wszColumnName,L"X");

		// Compare two strings, regarding upper and lower case
		if (((wszColumnName) && ((dbColumnInfo.columnid).uName.pwszName) && 
			 (wcscmp(wszColumnName,(dbColumnInfo.columnid).uName.pwszName) == 0)) ||
			(!(*wszColumnName) && !((dbColumnInfo.columnid).uName.pwszName)))
			return TRUE;
		else 
		{
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->COLUMNID]: expected "
				<<(*wszColumnName ? wszColumnName : L"<EMPTY>") <<L", returned "<<(dbColumnInfo.columnid).uName.pwszName <<ENDL;
			return FALSE;
		}
	}
	// This should cover DBKIND_PGUID_NAME
	else if ((dbColumnInfo.columnid).eKind == DBKIND_PGUID_NAME)
	{
		// Check the property value being returned
		if (!ulIDType)
			odtLog << L"Should return DBPROPVAL_PT_PGUID_NAME for DBPROP_PERSISTENTIDTYPE" <<ENDL;
		else
			COMPARE(ulIDType, DBPROPVAL_PT_PGUID_NAME);

		// Check to see if the Column names have been changed
		if (eSQLStmt == SELECT_CHANGECOLNAME)
			wcscat(wszColumnName,L"X");

		// Compare two strings, regarding upper and lower case
		if (((wszColumnName) && ((dbColumnInfo.columnid).uName.pwszName) && 
			 (wcscmp(wszColumnName,(dbColumnInfo.columnid).uName.pwszName) == 0)) ||
			(!(*wszColumnName) && !((dbColumnInfo.columnid).uName.pwszName)))
			return TRUE;
		else 
		{
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->COLUMNID]: expected "
				<<(*wszColumnName ? wszColumnName : L"<EMPTY>") <<L", returned "<<(dbColumnInfo.columnid).uName.pwszName <<ENDL;
			return FALSE;
		}
	}
	// This should cover DBKIND_PGUID_PROPID
	else if ((dbColumnInfo.columnid).eKind == DBKIND_PGUID_PROPID)
	{
		// Check the property value being returned
		if (!ulIDType)
			odtLog << L"Should return DBPROPVAL_PT_PGUID_PROPID for DBPROP_PERSISTENTIDTYPE" <<ENDL;
		else
			COMPARE(ulIDType, DBPROPVAL_PT_PGUID_PROPID);

		// Get the Ordinal back from MapColumnIDs
		rgColumnIDs = &dbColumnInfo.columnid;
		if(m_pIColumnsInfo)
			CHECK(m_pIColumnsInfo->MapColumnIDs(1,rgColumnIDs,&ulOrdinal), S_OK);
		else if(m_pIColumnsInfo2)
			CHECK(m_pIColumnsInfo2->MapColumnIDs(1,rgColumnIDs,&ulOrdinal), S_OK);
		rgColumnIDs=NULL;

		// Compare two iOrdinal and ulPropid
		if (ulOrdinal == m_rgInfo[ulIndex].iOrdinal)
			return TRUE;
		else 
		{
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->COLUMNID]: expected "
				<<m_rgInfo[ulIndex].iOrdinal <<L", returned "<<(dbColumnInfo.columnid).uName.ulPropid <<ENDL;
			return FALSE;
		}
	}
	// This should cover DBKIND_PROPID
	else if ((dbColumnInfo.columnid).eKind == DBKIND_PROPID)
	{
		// Check the property value being returned
		if (!ulIDType)
			odtLog << L"Should return DBPROPVAL_PT_PROPID for DBPROP_PERSISTENTIDTYPE" <<ENDL;
		else
			COMPARE(ulIDType, DBPROPVAL_PT_PROPID);

		// Get the Ordinal back from MapColumnIDs
		rgColumnIDs = &dbColumnInfo.columnid;
		if(m_pIColumnsInfo)
			CHECK(m_pIColumnsInfo->MapColumnIDs(1,rgColumnIDs,&ulOrdinal), S_OK);
		else if(m_pIColumnsInfo2)
			CHECK(m_pIColumnsInfo2->MapColumnIDs(1,rgColumnIDs,&ulOrdinal), S_OK);
		rgColumnIDs=NULL;

		// Compare two iOrdinal and ulPropid
		if (ulOrdinal == m_rgInfo[ulIndex].iOrdinal)
			return TRUE;
		else 
		{
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->COLUMNID]: expected "
				<<m_rgInfo[ulIndex].iOrdinal <<L", returned "<<(dbColumnInfo.columnid).uName.ulPropid <<ENDL;
			return FALSE;
		}
	}
	// This should cover DBKIND_GUID
	else if ((dbColumnInfo.columnid).eKind == DBKIND_GUID)
	{
		// Check the property value being returned
		if (!ulIDType)
			odtLog << L"Should return DBPROPVAL_PT_GUID for DBPROP_PERSISTENTIDTYPE" <<ENDL;
		else
			COMPARE(ulIDType, DBPROPVAL_PT_GUID);

		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->DBKIND_GUID]: Not tested. " <<ENDL;
	}
	else
		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - "
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType <<L" ->COLUMNID]: IS BAD " <<ENDL;

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_CACHEDEFERRED
//
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_CACHEDEFERRED(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	BOOL	fSuccess = FALSE;

	// Check the DBPROP_CACHEDEFERRED Property
	if (GetProperty(DBPROP_CACHEDEFERRED, DBPROPSET_ROWSET, m_pIUnknown))
	{
		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_CACHEDEFERRED)
			fSuccess = TRUE;
		else
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->CACHEDEFERRED]: expected, but isn't present."<<ENDL;
	}
	else
	{
		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_CACHEDEFERRED)
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->CACHEDEFERRED]: was not expected, but was present."<<ENDL;
		else
			fSuccess = TRUE;
	}

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_ISBOOKMARK to 
// to the iOrdinal of zero.
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_ISBOOKMARK(DBCOLUMNINFO dbColumnInfo, DBORDINAL ulIndex)
{
	BOOL	fSuccess = FALSE;
	
	// Check the Ordinal of the column
	if (dbColumnInfo.iOrdinal == 0)
	{
		// Check for the Bookmark Property and then the FLAG
		if ((!GetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pIUnknown)) &&
			(!GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIUnknown)) )
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<"Bookmark" <<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->ISBOOKMARK]: expected bookmark property to be set, but isn't set."<< ENDL;
		else if (!(dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISBOOKMARK))
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<"Bookmark" <<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->ISBOOKMARK]: expected bookmark, but isn't present."<< ENDL;
		else
			fSuccess = TRUE;
	}
	else
	{
		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISBOOKMARK)
				odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
					<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
					<<L" - " <<m_rgInfo[ulIndex].wType 
					<<L" ->ISBOOKMARK]: didn't expect the bookmark, but it is present."<< ENDL;
		else
			fSuccess = TRUE;
	}

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_ISFIXEDLENGTH
//
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_ISFIXEDLENGTH(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	BOOL fSuccess = FALSE;

	// Check to see if the type is FixedLength
	if ((IsFixedLength(col.GetProviderType())) || (col.GetIsFixedLength())) 
	{
		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH)
			fSuccess = TRUE;
		else
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->FIXEDLENGTH]: type is fixedlen but flag not set."<<ENDL;
	}
	else		
	{
		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH)
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->FIXEDLENGTH]: type is not fixedlen but flag was set."<<ENDL;
		else
			fSuccess = TRUE;
	}

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_ISLONG
//
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_ISLONG(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	BOOL fSuccess = FALSE;

	// Check to see if the column is Long
	if (col.GetIsLong())
	{
		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISLONG)
			fSuccess = TRUE;
		else
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->ISLONG]: type is Long but flag not set."<<ENDL;
	}
	else
	{
		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISLONG)
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>") 
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->ISLONG]: type is not Long but flag was set."<< ENDL;
		else
			fSuccess = TRUE;
	}

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_ISNULLABLE
//
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_ISNULLABLE(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	BOOL fSuccess = FALSE;

	// See if the Column is NULLABLE
	if (col.GetNullable())
	{
		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISNULLABLE)
			fSuccess = TRUE;
		else 
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->ISNULLABLE]: expected, but isn't present."<<ENDL;
	}
	else
	{
 		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISNULLABLE)
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->ISNULLABLE]: is not expected, but is present."<<ENDL;
		else 
			fSuccess = TRUE;
	}

	return fSuccess;
}
//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_ISROWID
//
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_ISROWID(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISROWID)
		odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
			<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
			<<L" - " <<m_rgInfo[ulIndex].wType 
			<<L" ->ISROWID]: returned for this column."<<ENDL;
	
	return TRUE;
}
//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_ISROWVER
//
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_ISROWVER(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISROWVER)
	{
		// Check to see if the column is nullable or updatable
		if( (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_ISNULLABLE) || (col.GetNullable()) || 
			(dbColumnInfo.dwFlags & DBCOLUMNFLAGS_WRITE) || (col.GetUpdateable()) )
		{
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->ISROWVER]: returned for this column and the column is NULLABLE or WRITEABLE."<<ENDL;
			return FALSE;

		}
	}

	return TRUE;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_MAYBENULL
//
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_MAYBENULLABLE(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex,EQUERY eSQLStmt)
{
	BOOL fSuccess = FALSE;

	// See if the Column is MAYBENULL (JOINS 
	if (((col.GetNullable()) && (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_MAYBENULL)) ||
		(dbColumnInfo.dwFlags & DBCOLUMNFLAGS_MAYBENULL))
		fSuccess = TRUE;
	else
	{
		if((eSQLStmt != SELECT_LEFTOUTERJOIN) && (eSQLStmt != SELECT_RIGHTOUTERJOIN))
			fSuccess = TRUE;
		else
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->MAYBENULL]: expected, but isn't present."<<ENDL;
	}

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_MAYDEFER
//
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_MAYDEFER(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	BOOL	fSuccess = FALSE;

	// Check the DBPROP_DEFERRED Property
	if (GetProperty(DBPROP_DEFERRED, DBPROPSET_ROWSET, m_pIUnknown))
	{
		if ((dbColumnInfo.dwFlags & DBCOLUMNFLAGS_MAYDEFER) || 
			(!(col.GetIsLong())) )
			fSuccess = TRUE;
		else
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->MAYDEFER]: expected, but isn't present."<<ENDL;
	}
	else
	{
		if (dbColumnInfo.dwFlags & DBCOLUMNFLAGS_MAYDEFER)
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->MAYDEFER]: was not expected, but was present."<<ENDL;
		else
			fSuccess = TRUE;
	}

	return fSuccess;
}

//--------------------------------------------------------------------
// @mfunc Utility function that compares DBCOLUMNINFO.DBCOLUMNFLAGS_WRITE
//
//
// @rdesc TRUE or FALSE
//
BOOL IColInfo::Compare_DBCOLUMNFLAGS_WRITE(CCol col,DBCOLUMNINFO dbColumnInfo,DBORDINAL ulIndex)
{
	BOOL fSuccess = FALSE;

	// Check to see if the column is readonly
	if (col.GetUpdateable())
	{
		if (((dbColumnInfo.dwFlags & DBCOLUMNFLAGS_WRITE) && 
			 (!(dbColumnInfo.dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))) ||
			((dbColumnInfo.dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN) && 
			 (!(dbColumnInfo.dwFlags & DBCOLUMNFLAGS_WRITE))))
			fSuccess = TRUE;
		else
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->WRITE & WRITEUNKNOWN]: not expected, but both are present."<<ENDL;
	}
	else
	{
		if ((dbColumnInfo.dwFlags & DBCOLUMNFLAGS_WRITE) ||
			(dbColumnInfo.dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))
			odtLog<<L"[ " <<m_rgInfo[ulIndex].iOrdinal <<L" - " 
				<<(m_rgInfo[ulIndex].pwszName ? m_rgInfo[ulIndex].pwszName : L"<EMPTY>")
				<<L" - " <<m_rgInfo[ulIndex].wType 
				<<L" ->WRITE or WRITEUNKNOWN]: is not expected, but is present."<<ENDL;
		else
			fSuccess = TRUE;
	}

	return fSuccess;
}

//--------------------------------------------------------------------
// @class zombie on CTransaction
//
class Zombie : public CTransaction
{			 
public:
	Zombie(const LPWSTR wszTestCaseName): CTransaction(wszTestCaseName){};
	int TestTxnCmd(ETXN eTxn, BOOL fRetaining);
	int TestTxnRowset(ETXN eTxn, BOOL fRetaining,BOOL fMultRowsets=FALSE);
	int TestTxnRow(ETXN eTxn, BOOL fRetaining);
};

//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort with respect to IAccessor on commands
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::TestTxnCmd(ETXN eTxn,BOOL fRetaining)
{
	BOOL			fSuccess		= FALSE;
	HRESULT			ExpectedHr		= E_UNEXPECTED;
	HRESULT			ExpColInfoHr	= DB_E_NOTPREPARED;
	DBORDINAL		index			= 0;
	DBCOUNTITEM		cRowsObtained	= 0;
	HROW *			rghRows			= NULL;
	DBORDINAL		cColumnIDs		= 0;
	DBID *			rgColumnIDs		= NULL;
	DBORDINAL *		rgColumns		= NULL;
	DBORDINAL		cColumns		= 0;
	DBCOLUMNINFO *	rgInfo			= NULL;
	WCHAR *			pStringsBuffer	= NULL;
	IColumnsInfo *	pIColumnsInfo	= NULL;

	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIColumnsInfo, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto CLEANUP;

	//Make sure everything still works after commit or abort
	CHECK(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer),S_OK);

	// build array of DBIDs
	if(cColumns)
	{
		cColumnIDs = cColumns;
		rgColumnIDs = (DBID *) PROVIDER_ALLOC(sizeof(DBID) * cColumns);
		rgColumns = (DBORDINAL *) PROVIDER_ALLOC(sizeof(DBORDINAL) * cColumns);

		for(index=0;index<cColumns;index++)
			DuplicateDBID(rgInfo[index].columnid, &rgColumnIDs[index]);
	}
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	if (eTxn == ETXN_COMMIT)
	{
		// Commit the transaction, with retention as specified
		if (!GetCommit(fRetaining))
			goto CLEANUP;
	}
	else
	{
		// Abort the transaction, with retention as specified
		if (!GetAbort(fRetaining))
			goto CLEANUP;
	}

	// Make sure everything still works after commit or abort
	if ((eTxn == ETXN_COMMIT && m_fCommitPreserve) ||
		(eTxn == ETXN_ABORT && m_fAbortPreserve))
		ExpectedHr = S_OK;

	// Test zombie
	CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);
		
	// Make sure everything still works after commit or abort
	if ((eTxn == ETXN_COMMIT && m_fPrepareCommitPreserve) ||
		(eTxn == ETXN_ABORT && m_fPrepareAbortPreserve))
		ExpColInfoHr = S_OK;

	// Transaction object prepares so I have to check for S_OK
	// Make sure everything still works after commit or abort
	CHECK(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer), ExpColInfoHr);
	{
		// Transaction object doesn't prepare so I have to check for S_OK
		fSuccess = CHECK(pIColumnsInfo->MapColumnIDs(cColumnIDs,rgColumnIDs,rgColumns),ExpColInfoHr);
	}
		
CLEANUP:

	// Release the objects and cleanup memory
	SAFE_RELEASE(pIColumnsInfo);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	// Free the memory allocated for MapColIDs
	for(index=0;index<cColumnIDs;index++)
		ReleaseDBID(&rgColumnIDs[index],FALSE);
	PROVIDER_FREE(rgColumnIDs);
	PROVIDER_FREE(rgColumns);

	// Release the row handle on the 1st rowset
	if (rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(rghRows);
	}

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	if (fRetaining)
		ExpectedHr = S_OK;
	else
		ExpectedHr = XACT_E_NOTRANSACTION;

	CleanUpTransaction(ExpectedHr);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort with respect to IAccessor on commands
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::TestTxnRowset(ETXN eTxn,BOOL fRetaining,BOOL fMultRowsets)
{
	BOOL			fSuccess		= FALSE;
	HRESULT			ExpectedHr		= E_UNEXPECTED;
	DBORDINAL		index			= 0;
	DBCOUNTITEM		cRowsObtained	= 0;
	HROW *			rghRows			= NULL;
	DBORDINAL		cColumnIDs		= 0;
	DBID *			rgColumnIDs		= NULL;
	DBORDINAL *		rgColumns		= NULL;
	DBORDINAL		cColumns		= 0;
	DBCOLUMNINFO *	rgInfo			= NULL;
	WCHAR *			pStringsBuffer	= NULL;
	IColumnsInfo *	pIColumnsInfo	= NULL;
	ULONG			cPropSets		= 0;
	DBPROPSET *		rgPropSets		= NULL;
	
	IColumnsInfo *	pIRowset1		= NULL;
	DBORDINAL		cColumns1		= 0;
	DBCOLUMNINFO *	rgColumns1		= NULL;
	OLECHAR*		pStringsBuffer1	= NULL;

	IColumnsInfo *	pIRowset2		= NULL;
	DBORDINAL		cColumns2		= 0;
	DBCOLUMNINFO *	rgColumns2		= NULL;
	OLECHAR*		pStringsBuffer2	= NULL;

	IColumnsInfo *	pIRowset3		= NULL;	
	DBORDINAL		cColumns3		= 0;
	DBCOLUMNINFO *	rgColumns3		= NULL;
	OLECHAR*		pStringsBuffer3	= NULL;

	if( (SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBCreateSession)) &&
		(SettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBCreateSession)) )
		SetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,
					&cPropSets,&rgPropSets,DBTYPE_BOOL,VARIANT_TRUE,DBPROPOPTIONS_REQUIRED);

	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIColumnsInfo, 
						cPropSets, rgPropSets, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto CLEANUP;

	if (fMultRowsets)
	{
		if (m_pICommand)
		{
			CHECK(m_pICommand->Execute(NULL,IID_IColumnsInfo,NULL,NULL,(IUnknown**)&pIRowset1),S_OK);
			CHECK(m_pICommand->Execute(NULL,IID_IColumnsInfo,NULL,NULL,(IUnknown**)&pIRowset2),S_OK);
			CHECK(m_pICommand->Execute(NULL,IID_IColumnsInfo,NULL,NULL,(IUnknown**)&pIRowset3),S_OK);
		}
	}

	//Make sure everything still works after commit or abort
	CHECK(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer),S_OK);

	// build array of DBIDs
	if(cColumns)
	{
		cColumnIDs = cColumns;
		rgColumnIDs = (DBID *) PROVIDER_ALLOC(sizeof(DBID) * cColumns);
		rgColumns = (DBORDINAL *) PROVIDER_ALLOC(sizeof(DBORDINAL) * cColumns);

		for(index=0;index<cColumns;index++)
			DuplicateDBID(rgInfo[index].columnid, &rgColumnIDs[index]);
	}
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	if (eTxn == ETXN_COMMIT)
	{
		//Commit the transaction, with retention as specified
		if(!GetCommit(fRetaining))
			goto CLEANUP;
	}
	else
	{
		//Abort the transaction, with retention as specified
		if(!GetAbort(fRetaining))
			goto CLEANUP;
	}

	// Make sure everything still works after commit or abort
	if ((eTxn == ETXN_COMMIT && m_fCommitPreserve) ||
		(eTxn == ETXN_ABORT && m_fAbortPreserve))
		ExpectedHr = S_OK;

	// Test zombie
	CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);
		
	//Make sure everything still works after commit or abort
	if (CHECK(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer),ExpectedHr))
	{
		fSuccess = CHECK(pIColumnsInfo->MapColumnIDs(cColumnIDs,rgColumnIDs,rgColumns),ExpectedHr);
		
		// make sure rowset1-3 are still alive
		if (pIRowset1)
			CHECK(pIRowset1->GetColumnInfo(&cColumns1,&rgColumns1,&pStringsBuffer1),ExpectedHr);

		if (pIRowset2)
			CHECK(pIRowset2->GetColumnInfo(&cColumns2,&rgColumns2,&pStringsBuffer2),ExpectedHr);

		if (pIRowset3)
			CHECK(pIRowset3->GetColumnInfo(&cColumns3,&rgColumns3,&pStringsBuffer3),ExpectedHr);
	}
		
CLEANUP:

	// Release the row handle on the 1st rowset
	if (rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(rghRows);
	}

	// Release the objects and cleanup memory
	SAFE_RELEASE(pIColumnsInfo);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);
	PROVIDER_FREE(pStringsBuffer1);
	PROVIDER_FREE(pStringsBuffer2);
	PROVIDER_FREE(pStringsBuffer3);

	// Free the memory allocated for MapColIDs
	for(index=0;index<cColumnIDs;index++)
		ReleaseDBID(&rgColumnIDs[index],FALSE);
	PROVIDER_FREE(rgColumnIDs);

	PROVIDER_FREE(rgColumns);
	PROVIDER_FREE(rgColumns1);
	PROVIDER_FREE(rgColumns2);
	PROVIDER_FREE(rgColumns3);
	PROVIDER_FREE(rghRows);

	SAFE_RELEASE(pIRowset1);
	SAFE_RELEASE(pIRowset2);
	SAFE_RELEASE(pIRowset3);

	FreeProperties(&cPropSets, &rgPropSets);

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	if (fRetaining)
		ExpectedHr = S_OK;
	else
		ExpectedHr = XACT_E_NOTRANSACTION;

	CleanUpTransaction(ExpectedHr);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort with respect to IAccessor on commands
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::TestTxnRow(ETXN eTxn,BOOL fRetaining)
{
	BOOL			fSuccess		= FALSE;
	HRESULT			ExpectedHr		= E_UNEXPECTED;
	DBORDINAL		index			= 0;
	DBCOUNTITEM		cRowsObtained	= 0;
	HROW *			rghRows			= NULL;
	DBORDINAL		cColumnIDs		= 0;
	DBID *			rgColumnIDs		= NULL;
	DBORDINAL *		rgColumns		= NULL;
	DBORDINAL		cColumns		= 0;
	DBCOLUMNINFO *	rgInfo			= NULL;
	WCHAR *			pStringsBuffer	= NULL;
	IColumnsInfo *	pIColumnsInfo	= NULL;
	ULONG			cPropSets		= 0;
	DBPROPSET *		rgPropSets		= NULL;

	if( (SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBCreateSession)) &&
		(SettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBCreateSession)) )
		SetProperty(DBPROP_BOOKMARKS,DBPROPSET_ROWSET,
					&cPropSets,&rgPropSets,DBTYPE_BOOL,VARIANT_TRUE,DBPROPOPTIONS_REQUIRED);

	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pIColumnsInfo, 
						cPropSets, rgPropSets, NULL, ISOLATIONLEVEL_READUNCOMMITTED, TRUE))
		goto CLEANUP;

	//Make sure everything still works after commit or abort
	CHECK(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer),S_OK);

	// build array of DBIDs
	if(cColumns)
	{
		cColumnIDs = cColumns;
		rgColumnIDs = (DBID *) PROVIDER_ALLOC(sizeof(DBID) * cColumns);
		rgColumns = (DBORDINAL *) PROVIDER_ALLOC(sizeof(DBORDINAL) * cColumns);

		for(index=0;index<cColumns;index++)
			DuplicateDBID(rgInfo[index].columnid, &rgColumnIDs[index]);
	}
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	if (eTxn == ETXN_COMMIT)
	{
		//Commit the transaction, with retention as specified
		if(!GetCommit(fRetaining))
			goto CLEANUP;
	}
	else
	{
		//Abort the transaction, with retention as specified
		if(!GetAbort(fRetaining))
			goto CLEANUP;
	}

	// Make sure everything still works after commit or abort
	if ((eTxn == ETXN_COMMIT && m_fCommitPreserve) ||
		(eTxn == ETXN_ABORT && m_fAbortPreserve))
		ExpectedHr = S_OK;

	// Test zombie
	CHECK(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);
		
	//Make sure everything still works after commit or abort
	// Per spec, E_UNEXPECTED will only be returned for Rowsets.  
	if (CHECK(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&pStringsBuffer),S_OK))
	{
		fSuccess = CHECK(pIColumnsInfo->MapColumnIDs(cColumnIDs,rgColumnIDs,rgColumns),S_OK);
	}
		
CLEANUP:

	// Release the row handle on the 1st rowset
	if (rghRows)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);
		PROVIDER_FREE(rghRows);
	}

	// Release the objects and cleanup memory
	SAFE_RELEASE(pIColumnsInfo);
	PROVIDER_FREE(rgInfo);
	PROVIDER_FREE(pStringsBuffer);

	// Free the memory allocated for MapColIDs
	for(index=0;index<cColumnIDs;index++)
		ReleaseDBID(&rgColumnIDs[index],FALSE);
	PROVIDER_FREE(rgColumnIDs);

	PROVIDER_FREE(rgColumns);
	PROVIDER_FREE(rghRows);

	FreeProperties(&cPropSets, &rgPropSets);

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	if (fRetaining)
		ExpectedHr = S_OK;
	else
		ExpectedHr = XACT_E_NOTRANSACTION;

	CleanUpTransaction(ExpectedHr);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(GetColInfo_Command)
//--------------------------------------------------------------------
// @class IColumnsInfo on a command object
//
class GetColInfo_Command : public IColInfo { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetColInfo_Command,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG: cColumns is NULL
	int Variation_1();
	// @cmember E_INVALIDARG: rgInfo is NULL
	int Variation_2();
	// @cmember E_INVALIDARG: pStringsBuffer is NULL
	int Variation_3();
	// @cmember DB_E_NOCOMMAND: CommandText is not Set
	int Variation_4();
	// @cmember DB_E_NOCOMMAND: CommandText is Set to Empty
	int Variation_5();
	// @cmember DB_E_NOCOMMAND: CommandText is Set to NULL
	int Variation_6();
	// @cmember DB_E_NOTPREPARED/S_OK: CommandText is Set, Prepared, and Unprepared
	int Variation_7();
	// @cmember DB_E_NOTPREPARED/S_OK: CommandText is Set, but not Prepared
	int Variation_8();
	// @cmember S_OK: CommandText is Set and Prepared
	int Variation_9();
	// @cmember S_OK: CommandText includes duplicate columns
	int Variation_10();
	// @cmember S_OK: CommandText is Set, then change the Text
	int Variation_11();
	// @cmember S_OK: CommandText contains a count(*)
	int Variation_12();
	// @cmember S_OK: CommandText contains different column names
	int Variation_13();
	// @cmember S_OK: CommandText contains a insert statement
	int Variation_14();
	// @cmember S_OK: CommandText contains a select from an Empty Rowset
	int Variation_15();
	// @cmember S_OK: Base table contains no rows
	int Variation_16();
	// @cmember S_OK: CommandText contains a update statement
	int Variation_17();
	// @cmember S_OK: CommandText contains a delete statement
	int Variation_18();
	// @cmember S_OK: Ask for DBPROP_IRowsetLocate on a Select Statement
	int Variation_19();
	// @cmember S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement
	int Variation_20();
	// @cmember S_OK: Ask for DBPROP_BOOKMARKS on a Insert Statement
	int Variation_21();
	// @cmember S_OK: Ask for DBPROP_DEFERRED on a Select Statement
	int Variation_22();
	// @cmember S_OK: Ask for DBPROP_CACHEDEFERRED on a Select Statement
	int Variation_23();
	// @cmember S_OK: Ask for DBPROP_MAYWRITECOLUMN on a Select Statement
	int Variation_24();
	// @cmember S_OK: CommandText contains a select from view
	int Variation_25();
	// @cmember S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement with Column List
	int Variation_26();
	// @cmember S_OK: CommandText contains a LEFT OUTER JOIN
	int Variation_27();
	// @cmember S_OK: CommandText contains a RIGHT OUTER JOIN
	int Variation_28();
	// @cmember DB_E_NOTABLE: Call with a Table that doesn't exist
	int Variation_29();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(GetColInfo_Command)
#define THE_CLASS GetColInfo_Command
BEG_TEST_CASE(GetColInfo_Command, IColInfo, L"IColumnsInfo on a command object")
	TEST_VARIATION(1, 		L"E_INVALIDARG: cColumns is NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG: rgInfo is NULL")
	TEST_VARIATION(3, 		L"E_INVALIDARG: pStringsBuffer is NULL")
	TEST_VARIATION(4, 		L"DB_E_NOCOMMAND: CommandText is not Set")
	TEST_VARIATION(5, 		L"DB_E_NOCOMMAND: CommandText is Set to Empty")
	TEST_VARIATION(6, 		L"DB_E_NOCOMMAND: CommandText is Set to NULL")
	TEST_VARIATION(7, 		L"DB_E_NOTPREPARED/S_OK: CommandText is Set, Prepared, and Unprepared")
	TEST_VARIATION(8, 		L"DB_E_NOTPREPARED/S_OK: CommandText is Set, but not Prepared")
	TEST_VARIATION(9, 		L"S_OK: CommandText is Set and Prepared")
	TEST_VARIATION(10, 		L"S_OK: CommandText includes duplicate columns")
	TEST_VARIATION(11, 		L"S_OK: CommandText is Set, then change the Text")
	TEST_VARIATION(12, 		L"S_OK: CommandText contains a count(*)")
	TEST_VARIATION(13, 		L"S_OK: CommandText contains different column names")
	TEST_VARIATION(14, 		L"S_OK: CommandText contains a insert statement")
	TEST_VARIATION(15, 		L"S_OK: CommandText contains a select from an Empty Rowset")
	TEST_VARIATION(16, 		L"S_OK: Base table contains no rows")
	TEST_VARIATION(17, 		L"S_OK: CommandText contains a update statement")
	TEST_VARIATION(18, 		L"S_OK: CommandText contains a delete statement")
	TEST_VARIATION(19, 		L"S_OK: Ask for DBPROP_IRowsetLocate on a Select Statement")
	TEST_VARIATION(20, 		L"S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement")
	TEST_VARIATION(21, 		L"S_OK: Ask for DBPROP_BOOKMARKS on a Insert Statement")
	TEST_VARIATION(22, 		L"S_OK: Ask for DBPROP_DEFERRED on a Select Statement")
	TEST_VARIATION(23, 		L"S_OK: Ask for DBPROP_CACHEDEFERRED on a Select Statement")
	TEST_VARIATION(24, 		L"S_OK: Ask for DBPROP_MAYWRITECOLUMN on a Select Statement")
	TEST_VARIATION(25, 		L"S_OK: CommandText contains a select from view")
	TEST_VARIATION(26, 		L"S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement with Column List")
	TEST_VARIATION(27, 		L"S_OK: CommandText contains a LEFT OUTER JOIN")
	TEST_VARIATION(28, 		L"S_OK: CommandText contains a RIGHT OUTER JOIN")
	TEST_VARIATION(29, 		L"DB_E_NOTABLE: Call with a Table that doesn't exist")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(GetColInfo_ExecuteRowset)
//--------------------------------------------------------------------
// @class IColumnsInfo on a rowset object
//
class GetColInfo_ExecuteRowset : public IColInfo { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetColInfo_ExecuteRowset,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG: cColumns is NULL
	int Variation_1();
	// @cmember E_INVALIDARG: rgInfo is NULL
	int Variation_2();
	// @cmember E_INVALIDARG: pStringsBuffer is NULL
	int Variation_3();
	// @cmember S_OK: CommandText is Set and Executed
	int Variation_4();
	// @cmember S_OK: CommandText is Set, Prepared, Unprepared, and Executed
	int Variation_5();
	// @cmember S_OK: CommandText is Set, Prepared, and Executed
	int Variation_6();
	// @cmember S_OK: CommandText that includes duplicate columns
	int Variation_7();
	// @cmember S_OK: CommandText is Set, then change the Text
	int Variation_8();
	// @cmember S_OK: CommandText contains a count(*)
	int Variation_9();
	// @cmember S_OK: CommandText contains different column names
	int Variation_10();
	// @cmember S_OK: CommandText contains a select from an Empty Rowset
	int Variation_11();
	// @cmember S_OK: Base table contains no rows
	int Variation_12();
	// @cmember S_OK: Ask for DBPROP_IRowsetLocate on a Select Statement
	int Variation_13();
	// @cmember S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement
	int Variation_14();
	// @cmember S_OK: Ask for DBPROP_DEFERRED on a Select Statement
	int Variation_15();
	// @cmember S_OK: Ask for DBPROP_CACHEDEFERRED on a Select Statement
	int Variation_16();
	// @cmember S_OK: Ask for DBPROP_MAYWRITECOLUMN on a Select Statement
	int Variation_17();
	// @cmember S_OK: CommandText contains a select from view
	int Variation_18();
	// @cmember S_OK: CommandText contains a LEFT OUTER JOIN
	int Variation_19();
	// @cmember S_OK: CommandText contains a RIGHT OUTER JOIN
	int Variation_20();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(GetColInfo_ExecuteRowset)
#define THE_CLASS GetColInfo_ExecuteRowset
BEG_TEST_CASE(GetColInfo_ExecuteRowset, IColInfo, L"IColumnsInfo on a rowset object")
	TEST_VARIATION(1, 		L"E_INVALIDARG: cColumns is NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG: rgInfo is NULL")
	TEST_VARIATION(3, 		L"E_INVALIDARG: pStringsBuffer is NULL")
	TEST_VARIATION(4, 		L"S_OK: CommandText is Set and Executed")
	TEST_VARIATION(5, 		L"S_OK: CommandText is Set, Prepared, Unprepared, and Executed")
	TEST_VARIATION(6, 		L"S_OK: CommandText is Set, Prepared, and Executed")
	TEST_VARIATION(7, 		L"S_OK: CommandText that includes duplicate columns")
	TEST_VARIATION(8, 		L"S_OK: CommandText is Set, then change the Text")
	TEST_VARIATION(9, 		L"S_OK: CommandText contains a count(*)")
	TEST_VARIATION(10, 		L"S_OK: CommandText contains different column names")
	TEST_VARIATION(11, 		L"S_OK: CommandText contains a select from an Empty Rowset")
	TEST_VARIATION(12, 		L"S_OK: Base table contains no rows")
	TEST_VARIATION(13, 		L"S_OK: Ask for DBPROP_IRowsetLocate on a Select Statement")
	TEST_VARIATION(14, 		L"S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement")
	TEST_VARIATION(15, 		L"S_OK: Ask for DBPROP_DEFERRED on a Select Statement")
	TEST_VARIATION(16, 		L"S_OK: Ask for DBPROP_CACHEDEFERRED on a Select Statement")
	TEST_VARIATION(17, 		L"S_OK: Ask for DBPROP_MAYWRITECOLUMN on a Select Statement")
	TEST_VARIATION(18, 		L"S_OK: CommandText contains a select from view")
	TEST_VARIATION(19, 		L"S_OK: CommandText contains a LEFT OUTER JOIN")
	TEST_VARIATION(20, 		L"S_OK: CommandText contains a RIGHT OUTER JOIN")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(GetColInfo_OpenRowset)
//--------------------------------------------------------------------
// @class IColumnsInfo on a rowset object
//
class GetColInfo_OpenRowset : public IColInfo { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetColInfo_OpenRowset,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG: cColumns is NULL
	int Variation_1();
	// @cmember E_INVALIDARG: rgInfo is NULL
	int Variation_2();
	// @cmember E_INVALIDARG: pStringsBuffer is NULL
	int Variation_3();
	// @cmember S_OK: Basic OpenRowset
	int Variation_4();
	// @cmember S_OK: Base table contains no rows
	int Variation_5();
	// @cmember S_OK: Ask for DBPROP_IRowsetLocate on a Select Statement
	int Variation_6();
	// @cmember S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement
	int Variation_7();
	// @cmember S_OK: Ask for DBPROP_DEFERRED on a Select Statement
	int Variation_8();
	// @cmember S_OK: Ask for DBPROP_CACHEDEFERRED on a Select Statement
	int Variation_9();
	// @cmember S_OK: Ask for DBPROP_MAYWRITECOLUMN on a Select Statement
	int Variation_10();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(GetColInfo_OpenRowset)
#define THE_CLASS GetColInfo_OpenRowset
BEG_TEST_CASE(GetColInfo_OpenRowset, IColInfo, L"IColumnsInfo on a rowset object")
	TEST_VARIATION(1, 		L"E_INVALIDARG: cColumns is NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG: rgInfo is NULL")
	TEST_VARIATION(3, 		L"E_INVALIDARG: pStringsBuffer is NULL")
	TEST_VARIATION(4, 		L"S_OK: Basic OpenRowset")
	TEST_VARIATION(5, 		L"S_OK: Base table contains no rows")
	TEST_VARIATION(6, 		L"S_OK: Ask for DBPROP_IRowsetLocate on a Select Statement")
	TEST_VARIATION(7, 		L"S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement")
	TEST_VARIATION(8, 		L"S_OK: Ask for DBPROP_DEFERRED on a Select Statement")
	TEST_VARIATION(9, 		L"S_OK: Ask for DBPROP_CACHEDEFERRED on a Select Statement")
	TEST_VARIATION(10, 		L"S_OK: Ask for DBPROP_MAYWRITECOLUMN on a Select Statement")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(GetColInfoRow_Rowset)
//*-----------------------------------------------------------------------
// @class GetColumnInfo on a Row object
//
class GetColInfoRow_Rowset : public IColInfo { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetColInfoRow_Rowset,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember S_OK: Basic
	int Variation_1();
	// @cmember E_INVALIDARG: pcColumns is NULL
	int Variation_2();
	// @cmember E_INVALIDARG: prgInfo is NULL
	int Variation_3();
	// @cmember E_INVALIDARG: ppStringsBuffer is NULL
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(GetColInfoRow_Rowset)
#define THE_CLASS GetColInfoRow_Rowset
BEG_TEST_CASE(GetColInfoRow_Rowset, IColInfo, L"GetColumnInfo on a Row object")
	TEST_VARIATION(1, 		L"S_OK: Basic")
	TEST_VARIATION(2, 		L"E_INVALIDARG: pcColumns is NULL")
	TEST_VARIATION(3, 		L"E_INVALIDARG: prgInfo is NULL")
	TEST_VARIATION(4, 		L"E_INVALIDARG: ppStringsBuffer is NULL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(GetResColInfoRow_Rowset)
//*-----------------------------------------------------------------------
// @class GetRestrictedColumnInfo on a Row object
//
class GetResColInfoRow_Rowset : public IColInfo { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetResColInfoRow_Rowset,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember S_OK: No Masks
	int Variation_1();
	// @cmember S_OK: 1 name mask
	int Variation_2();
	// @cmember S_OK: Multiple name masks
	int Variation_3();
	// @cmember S_OK: Existing Column as mask
	int Variation_4();
	// @cmember S_OK: GUID_PROPID mask
	int Variation_5();
	// @cmember S_OK: Guid_Name mask
	int Variation_6();
	// @cmember S_OK: Pguid_Name mask
	int Variation_7();
	// @cmember S_OK: PGuid_Propid mask
	int Variation_8();
	// @cmember S_OK: Propid mask
	int Variation_9();
	// @cmember S_OK: Guid mask
	int Variation_10();
	// @cmember S_OK: prgColumnIDs=NULL but prgColumnInfo != NULL
	int Variation_11();
	// @cmember S_OK: prgColumnInfo=NULL but prgColumnIDs != NULL
	int Variation_12();
	// @cmember DB_E_BADCOLUMNID: invalid DBID for rgColumnIDMasks
	int Variation_13();
	// @cmember DB_E_NOCOLUMN: Non-existent name for mask
	int Variation_14();
	// @cmember E_INVALIDARG: cColumnIDMasks=1 and rgColumnIDMasks=NULL
	int Variation_15();
	// @cmember E_INVALIDARG: pcColumns=NULL
	int Variation_16();
	// @cmember E_INVALIDARG: ppStringsBuffer=NULL
	int Variation_17();
	// @cmember E_INVALIDARG: prgColumnIDs=NULL and prgColumnInfo=NULL
	int Variation_18();
	// @cmember E_INVALIDARG: ppStringsBuffer=NULL & either prgColumnIDs or prgColumnInfo = NULL
	int Variation_19();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(GetResColInfoRow_Rowset)
#define THE_CLASS GetResColInfoRow_Rowset
BEG_TEST_CASE(GetResColInfoRow_Rowset, IColInfo, L"GetRestrictedColumnInfo on a Row object")
	TEST_VARIATION(1, 		L"S_OK: No Masks")
	TEST_VARIATION(2, 		L"S_OK: 1 name mask")
	TEST_VARIATION(3, 		L"S_OK: Multiple name masks")
	TEST_VARIATION(4, 		L"S_OK: Existing Column as mask")
	TEST_VARIATION(5, 		L"S_OK: GUID_PROPID mask")
	TEST_VARIATION(6, 		L"S_OK: Guid_Name mask")
	TEST_VARIATION(7, 		L"S_OK: Pguid_Name mask")
	TEST_VARIATION(8, 		L"S_OK: PGuid_Propid mask")
	TEST_VARIATION(9, 		L"S_OK: Propid mask")
	TEST_VARIATION(10, 		L"S_OK: Guid mask")
	TEST_VARIATION(11, 		L"S_OK: prgColumnIDs=NULL but prgColumnInfo != NULL")
	TEST_VARIATION(12, 		L"S_OK: prgColumnInfo=NULL but prgColumnIDs != NULL")
	TEST_VARIATION(13, 		L"DB_E_BADCOLUMNID: invalid DBID for rgColumnIDMasks")
	TEST_VARIATION(14, 		L"DB_E_NOCOLUMN: Non-existent name for mask")
	TEST_VARIATION(15, 		L"E_INVALIDARG: cColumnIDMasks=1 and rgColumnIDMasks=NULL")
	TEST_VARIATION(16, 		L"E_INVALIDARG: pcColumns=NULL")
	TEST_VARIATION(17, 		L"E_INVALIDARG: ppStringsBuffer=NULL")
	TEST_VARIATION(18, 		L"E_INVALIDARG: prgColumnIDs=NULL and prgColumnInfo=NULL")
	TEST_VARIATION(19, 		L"E_INVALIDARG: ppStringsBuffer=NULL & either prgColumnIDs or prgColumnInfo = NULL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(MapColID_Command)
//--------------------------------------------------------------------
// @class MapColumnIDs on a command object
//
class MapColID_Command : public IColInfo { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MapColID_Command,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG: cColumnIDs != 0, range = NULL
	int Variation_1();
	// @cmember E_INVALIDARG: rgColumns is NULL
	int Variation_2();
	// @cmember DB_E_NOCOMMAND: CommandText is not Set
	int Variation_3();
	// @cmember DB_E_NOCOMMAND: CommandText is Set to Empty
	int Variation_4();
	// @cmember DB_E_NOCOMMAND: CommandText is Set to NULL
	int Variation_5();
	// @cmember DB_E_NOTPREPARED/S_OK: CommandText is Set, but not Prepared
	int Variation_6();
	// @cmember DB_E_NOTPREPARED/S_OK: CommandText is Set, Prepared, and Unprepared
	int Variation_7();
	// @cmember DB_E_ERRORSOCCURRED: All ColumnIDs are invalid
	int Variation_8();
	// @cmember DB_S_ERRORSOCCURRED: One ColumnID is invalid
	int Variation_9();
	// @cmember S_OK: Count of ColumnIDs is 0
	int Variation_10();
	// @cmember S_OK: Valid count and valid range of ColumnIDs
	int Variation_11();
	// @cmember S_OK: Ask for DBPROP_BOOKMARKS
	int Variation_12();
	// @cmember S_OK: Duplicate ColumnIDs
	int Variation_13();
	// @cmember S_OK: ColumnIDs in reverse order
	int Variation_14();
	// @cmember S_OK: ColumnIDs from a view
	int Variation_15();
	// @cmember S_OK: ColumnIDs from a count(*)
	int Variation_16();
	// @cmember DB_E_ERRORSOCCURRED: Change the DBKIND in the ColumnIDs
	int Variation_17();
	// @cmember DB_E_ERRORSOCCURRED: Ask for Column 0 with DBKIND_PROPID
	int Variation_18();
	// @cmember DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs
	int Variation_19();
	// @cmember DB_E_ERRORSOCCURRED: Change the GUID in the DBID
	int Variation_20();
	// @cmember DB_S_ERRORSOCCURRED: ColumnID one greater than the last ordinal
	int Variation_21();
	// @cmember DB_E_ERRORSOCCURRED: Ask for Column 0
	int Variation_22();
	// @cmember DB_E_NOTABLE: Call with a Table that doesn't exist
	int Variation_23();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(MapColID_Command)
#define THE_CLASS MapColID_Command
BEG_TEST_CASE(MapColID_Command, IColInfo, L"MapColumnIDs on a command object")
	TEST_VARIATION(1, 		L"E_INVALIDARG: cColumnIDs != 0, range = NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG: rgColumns is NULL")
	TEST_VARIATION(3, 		L"DB_E_NOCOMMAND: CommandText is not Set")
	TEST_VARIATION(4, 		L"DB_E_NOCOMMAND: CommandText is Set to Empty")
	TEST_VARIATION(5, 		L"DB_E_NOCOMMAND: CommandText is Set to NULL")
	TEST_VARIATION(6, 		L"DB_E_NOTPREPARED/S_OK: CommandText is Set, but not Prepared")
	TEST_VARIATION(7, 		L"DB_E_NOTPREPARED/S_OK: CommandText is Set, Prepared, and Unprepared")
	TEST_VARIATION(8, 		L"DB_E_ERRORSOCCURRED: All ColumnIDs are invalid")
	TEST_VARIATION(9, 		L"DB_S_ERRORSOCCURRED: One ColumnID is invalid")
	TEST_VARIATION(10, 		L"S_OK: Count of ColumnIDs is 0")
	TEST_VARIATION(11, 		L"S_OK: Valid count and valid range of ColumnIDs")
	TEST_VARIATION(12, 		L"S_OK: Ask for DBPROP_BOOKMARKS")
	TEST_VARIATION(13, 		L"S_OK: Duplicate ColumnIDs")
	TEST_VARIATION(14, 		L"S_OK: ColumnIDs in reverse order")
	TEST_VARIATION(15, 		L"S_OK: ColumnIDs from a view")
	TEST_VARIATION(16, 		L"S_OK: ColumnIDs from a count(*)")
	TEST_VARIATION(17, 		L"DB_E_ERRORSOCCURRED: Change the DBKIND in the ColumnIDs")
	TEST_VARIATION(18, 		L"DB_E_ERRORSOCCURRED: Ask for Column 0 with DBKIND_PROPID")
	TEST_VARIATION(19, 		L"DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs")
	TEST_VARIATION(20, 		L"DB_E_ERRORSOCCURRED: Change the GUID in the DBID")
	TEST_VARIATION(21, 		L"DB_S_ERRORSOCCURRED: ColumnID one greater than the last ordinal")
	TEST_VARIATION(22, 		L"DB_E_ERRORSOCCURRED: Ask for Column 0")
	TEST_VARIATION(23, 		L"DB_E_NOTABLE: Call with a Table that doesn't exist")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(MapColID_ExecuteRowset)
//--------------------------------------------------------------------
// @class MapColumnIDs on a rowset
//
class MapColID_ExecuteRowset : public IColInfo { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MapColID_ExecuteRowset,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG: cColumnIDs != 0, range = NULL
	int Variation_1();
	// @cmember E_INVALIDARG: rgColumns is NULL
	int Variation_2();
	// @cmember DB_E_ERRORSOCCURRED: All ColumnIDs are invalid
	int Variation_3();
	// @cmember DB_S_ERRORSOCCURRED: One ColumnID is invalid
	int Variation_4();
	// @cmember S_OK: Count of ColumnIDs is 0
	int Variation_5();
	// @cmember S_OK: Valid count and valid range of ColumnIDs
	int Variation_6();
	// @cmember S_OK: Ask for DBPROP_BOOKMARKS
	int Variation_7();
	// @cmember S_OK: Duplicate ColumnIDs
	int Variation_8();
	// @cmember S_OK: ColumnIDs in reverse order
	int Variation_9();
	// @cmember S_OK: ColumnIDs from a view
	int Variation_10();
	// @cmember S_OK: MapColumnIDs on optional columns in IColumnsRowset
	int Variation_11();
	// @cmember S_OK: ColumnIDs from a count(*)
	int Variation_12();
	// @cmember DB_E_ERRORSOCCURRED: Change the DBKIND in the ColumnIDs
	int Variation_13();
	// @cmember DB_E_ERRORSOCCURRED: Ask for Column 0 with DBKIND_PROPID
	int Variation_14();
	// @cmember DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs
	int Variation_15();
	// @cmember DB_E_ERRORSOCCURRED: Change the GUID in the DBID
	int Variation_16();
	// @cmember DB_S_ERRORSOCCURRED: ColumnID one greater than the last ordinal
	int Variation_17();
	// @cmember DB_E_ERRORSOCCURRED: Ask for Column 0
	int Variation_18();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(MapColID_ExecuteRowset)
#define THE_CLASS MapColID_ExecuteRowset
BEG_TEST_CASE(MapColID_ExecuteRowset, IColInfo, L"MapColumnIDs on a rowset")
	TEST_VARIATION(1, 		L"E_INVALIDARG: cColumnIDs != 0, range = NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG: rgColumns is NULL")
	TEST_VARIATION(3, 		L"DB_E_ERRORSOCCURRED: All ColumnIDs are invalid")
	TEST_VARIATION(4, 		L"DB_S_ERRORSOCCURRED: One ColumnID is invalid")
	TEST_VARIATION(5, 		L"S_OK: Count of ColumnIDs is 0")
	TEST_VARIATION(6, 		L"S_OK: Valid count and valid range of ColumnIDs")
	TEST_VARIATION(7, 		L"S_OK: Ask for DBPROP_BOOKMARKS")
	TEST_VARIATION(8, 		L"S_OK: Duplicate ColumnIDs")
	TEST_VARIATION(9, 		L"S_OK: ColumnIDs in reverse order")
	TEST_VARIATION(10, 		L"S_OK: ColumnIDs from a view")
	TEST_VARIATION(11, 		L"S_OK: MapColumnIDs on optional columns in IColumnsRowset")
	TEST_VARIATION(12, 		L"S_OK: ColumnIDs from a count(*)")
	TEST_VARIATION(13, 		L"DB_E_ERRORSOCCURRED: Change the DBKIND in the ColumnIDs")
	TEST_VARIATION(14, 		L"DB_E_ERRORSOCCURRED: Ask for Column 0 with DBKIND_PROPID")
	TEST_VARIATION(15, 		L"DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs")
	TEST_VARIATION(16, 		L"DB_E_ERRORSOCCURRED: Change the GUID in the DBID")
	TEST_VARIATION(17, 		L"DB_S_ERRORSOCCURRED: ColumnID one greater than the last ordinal")
	TEST_VARIATION(18, 		L"DB_E_ERRORSOCCURRED: Ask for Column 0")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(MapColID_OpenRowset)
//--------------------------------------------------------------------
// @class MapColumnIDs on a rowset
//
class MapColID_OpenRowset : public IColInfo { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MapColID_OpenRowset,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_INVALIDARG: cColumnIDs != 0, range = NULL
	int Variation_1();
	// @cmember E_INVALIDARG: rgColumns is NULL
	int Variation_2();
	// @cmember DB_E_ERRORSOCCURRED: All ColumnIDs are invalid
	int Variation_3();
	// @cmember DB_S_ERRORSOCCURRED: One ColumnID is invalid
	int Variation_4();
	// @cmember S_OK: Count of ColumnIDs is 0
	int Variation_5();
	// @cmember S_OK: Valid count and valid range of ColumnIDs
	int Variation_6();
	// @cmember S_OK: Ask for DBPROP_BOOKMARKS
	int Variation_7();
	// @cmember S_OK: MapColumnIDs on optional columns in IColumnsRowset
	int Variation_8();
	// @cmember DB_E_ERRORSOCCURRED: Change the DBKIND in the ColumnIDs
	int Variation_9();
	// @cmember DB_E_ERRORSOCCURRED: Ask for Column 0 with DBKIND_PROPID
	int Variation_10();
	// @cmember DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs
	int Variation_11();
	// @cmember DB_E_ERRORSOCCURRED: Change the GUID in the DBID
	int Variation_12();
	// @cmember DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs with Bookmarks
	int Variation_13();
	// @cmember DB_S_ERRORSOCCURRED: ColumnID one greater than the last ordinal
	int Variation_14();
	// @cmember DB_E_ERRORSOCCURRED: Ask for Column 0
	int Variation_15();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(MapColID_OpenRowset)
#define THE_CLASS MapColID_OpenRowset
BEG_TEST_CASE(MapColID_OpenRowset, IColInfo, L"MapColumnIDs on a rowset")
	TEST_VARIATION(1, 		L"E_INVALIDARG: cColumnIDs != 0, range = NULL")
	TEST_VARIATION(2, 		L"E_INVALIDARG: rgColumns is NULL")
	TEST_VARIATION(3, 		L"DB_E_ERRORSOCCURRED: All ColumnIDs are invalid")
	TEST_VARIATION(4, 		L"DB_S_ERRORSOCCURRED: One ColumnID is invalid")
	TEST_VARIATION(5, 		L"S_OK: Count of ColumnIDs is 0")
	TEST_VARIATION(6, 		L"S_OK: Valid count and valid range of ColumnIDs")
	TEST_VARIATION(7, 		L"S_OK: Ask for DBPROP_BOOKMARKS")
	TEST_VARIATION(8, 		L"S_OK: MapColumnIDs on optional columns in IColumnsRowset")
	TEST_VARIATION(9, 		L"DB_E_ERRORSOCCURRED: Change the DBKIND in the ColumnIDs")
	TEST_VARIATION(10, 		L"DB_E_ERRORSOCCURRED: Ask for Column 0 with DBKIND_PROPID")
	TEST_VARIATION(11, 		L"DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs")
	TEST_VARIATION(12, 		L"DB_E_ERRORSOCCURRED: Change the GUID in the DBID")
	TEST_VARIATION(13, 		L"DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs with Bookmarks")
	TEST_VARIATION(14, 		L"DB_S_ERRORSOCCURRED: ColumnID one greater than the last ordinal")
	TEST_VARIATION(15, 		L"DB_E_ERRORSOCCURRED: Ask for Column 0")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(MapColIDRow_Rowset)
//*-----------------------------------------------------------------------
// @class MapColumnIDs on a Row object
//
class MapColIDRow_Rowset : public IColInfo { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MapColIDRow_Rowset,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember S_OK: cColumnIDs = 0
	int Variation_1();
	// @cmember S_OK: All columns returned by GetColInfo
	int Variation_2();
	// @cmember S_OK: Subset of columns returned by GetColInfo
	int Variation_3();
	// @cmember S_OK: Columns out of order
	int Variation_4();
	// @cmember DB_S_ERRORSOCCURRED: One ColumnID is invalid
	int Variation_5();
	// @cmember DB_E_ERRORSOCCURRED: All ColumnIDs are invalid
	int Variation_6();
	// @cmember E_INVALIDARG: cColumnIDs=1, rgColumnIDs=NULL
	int Variation_7();
	// @cmember E_INVALIDARG: rgColumns is NULL
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(MapColIDRow_Rowset)
#define THE_CLASS MapColIDRow_Rowset
BEG_TEST_CASE(MapColIDRow_Rowset, IColInfo, L"MapColumnIDs on a Row object")
	TEST_VARIATION(1, 		L"S_OK: cColumnIDs = 0")
	TEST_VARIATION(2, 		L"S_OK: All columns returned by GetColInfo")
	TEST_VARIATION(3, 		L"S_OK: Subset of columns returned by GetColInfo")
	TEST_VARIATION(4, 		L"S_OK: Columns out of order")
	TEST_VARIATION(5, 		L"DB_S_ERRORSOCCURRED: One ColumnID is invalid")
	TEST_VARIATION(6, 		L"DB_E_ERRORSOCCURRED: All ColumnIDs are invalid")
	TEST_VARIATION(7, 		L"E_INVALIDARG: cColumnIDs=1, rgColumnIDs=NULL")
	TEST_VARIATION(8, 		L"E_INVALIDARG: rgColumns is NULL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(ZombieClassCmd)
//--------------------------------------------------------------------
// @class zombie on command
//
class ZombieClassCmd : public Zombie { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ZombieClassCmd,Zombie);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Abort with fRetaining set to TRUE
	int Variation_1();
	// @cmember Abort with fRetaining set to FALSE
	int Variation_2();
	// @cmember Commit with fRetaining set to TRUE
	int Variation_3();
	// @cmember Commit with fRetaining set to FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ZombieClassCmd)
#define THE_CLASS ZombieClassCmd
BEG_TEST_CASE(ZombieClassCmd, Zombie, L"zombie on command")
	TEST_VARIATION(1, 		L"Abort with fRetaining set to TRUE")
	TEST_VARIATION(2, 		L"Abort with fRetaining set to FALSE")
	TEST_VARIATION(3, 		L"Commit with fRetaining set to TRUE")
	TEST_VARIATION(4, 		L"Commit with fRetaining set to FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ZombieClassRowset)
//--------------------------------------------------------------------
// @class zombie on a rowset
//
class ZombieClassRowset : public Zombie { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ZombieClassRowset,Zombie);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Abort with fRetaining set to TRUE
	int Variation_1();
	// @cmember Abort with fRetaining set to FALSE
	int Variation_2();
	// @cmember Commit with fRetaining set to TRUE
	int Variation_3();
	// @cmember Commit with fRetaining set to FALSE
	int Variation_4();
	// @cmember Abort with fRetaining set to TRUE for Multiple Rowsets
	int Variation_5();
	// @cmember Abort with fRetaining set to FALSE for Multiple Rowsets
	int Variation_6();
	// @cmember Commit with fRetaining set to TRUE for Multiple Rowsets
	int Variation_7();
	// @cmember Commit with fRetaining set to FALSE for Multiple Rowsets
	int Variation_8();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ZombieClassRowset)
#define THE_CLASS ZombieClassRowset
BEG_TEST_CASE(ZombieClassRowset, Zombie, L"zombie on a rowset")
	TEST_VARIATION(1, 		L"Abort with fRetaining set to TRUE")
	TEST_VARIATION(2, 		L"Abort with fRetaining set to FALSE")
	TEST_VARIATION(3, 		L"Commit with fRetaining set to TRUE")
	TEST_VARIATION(4, 		L"Commit with fRetaining set to FALSE")
	TEST_VARIATION(5, 		L"Abort with fRetaining set to TRUE for Multiple Rowsets")
	TEST_VARIATION(6, 		L"Abort with fRetaining set to FALSE for Multiple Rowsets")
	TEST_VARIATION(7, 		L"Commit with fRetaining set to TRUE for Multiple Rowsets")
	TEST_VARIATION(8, 		L"Commit with fRetaining set to FALSE for Multiple Rowsets")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(ZombieClassRow)
//*-----------------------------------------------------------------------
// @class zombie on a row
//
class ZombieClassRow : public Zombie { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ZombieClassRow,Zombie);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Abort with fRetaining set to TRUE
	int Variation_1();
	// @cmember Abort with fRetaining set to FALSE
	int Variation_2();
	// @cmember Commit with fRetaining set to TRUE
	int Variation_3();
	// @cmember Commit with fRetaining set to FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(ZombieClassRow)
#define THE_CLASS ZombieClassRow
BEG_TEST_CASE(ZombieClassRow, Zombie, L"zombie on a row")
	TEST_VARIATION(1, 		L"Abort with fRetaining set to TRUE")
	TEST_VARIATION(2, 		L"Abort with fRetaining set to FALSE")
	TEST_VARIATION(3, 		L"Commit with fRetaining set to TRUE")
	TEST_VARIATION(4, 		L"Commit with fRetaining set to FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ExtendedErrors)
//--------------------------------------------------------------------
// @class extended error tests
//
class ExtendedErrors : public IColInfo { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ExtendedErrors,IColInfo);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid IColumnsInfo calls on the Command with previous error object existing
	int Variation_1();
	// @cmember Invalid IColumnsInfo calls on the Command with previous eror object existing
	int Variation_2();
	// @cmember Invalid IColumnsInfo calls on the Command with no previous error object existing
	int Variation_3();
	// @cmember Valid IColumnsInfo calls on the Rowset with previous error object existing
	int Variation_4();
	// @cmember Invalid IColumnsInfo calls on the Rowset with previous error object exisiting
	int Variation_5();
	// @cmember Invalid IColumnsInfo calls on the Rowset with no previous error object exisiting
	int Variation_6();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(ExtendedErrors)
#define THE_CLASS ExtendedErrors
BEG_TEST_CASE(ExtendedErrors, IColInfo, L"extended error tests")
	TEST_VARIATION(1, 		L"Valid IColumnsInfo calls on the Command with previous error object existing")
	TEST_VARIATION(2, 		L"Invalid IColumnsInfo calls on the Command with previous eror object existing")
	TEST_VARIATION(3, 		L"Invalid IColumnsInfo calls on the Command with no previous error object existing")
	TEST_VARIATION(4, 		L"Valid IColumnsInfo calls on the Rowset with previous error object existing")
	TEST_VARIATION(5, 		L"Invalid IColumnsInfo calls on the Rowset with previous error object exisiting")
	TEST_VARIATION(6, 		L"Invalid IColumnsInfo calls on the Rowset with no previous error object exisiting")
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

//2
COPY_TEST_CASE(GetColInfoRow_Cmd, GetColInfoRow_Rowset)
COPY_TEST_CASE(MapColIDRow_Cmd, MapColIDRow_Rowset)
COPY_TEST_CASE(GetResColInfoRow_Cmd, GetResColInfoRow_Rowset)

//3
COPY_TEST_CASE(GetColInfoRow_OpenRW, GetColInfoRow_Rowset)
COPY_TEST_CASE(MapColIDRow_OpenRW, MapColIDRow_Rowset)
COPY_TEST_CASE(GetResColInfoRow_OpenRW, GetResColInfoRow_Rowset)

//4
COPY_TEST_CASE(GetColInfoRow_Bind, GetColInfoRow_Rowset)
COPY_TEST_CASE(MapColIDRow_Bind, MapColIDRow_Rowset)
COPY_TEST_CASE(GetResColInfoRow_Bind, GetResColInfoRow_Rowset)

//5
COPY_TEST_CASE(GetColInfoRow_IColInfo2, GetColInfoRow_Rowset)
COPY_TEST_CASE(MapColIDRow_IColInfo2, MapColIDRow_Rowset)

COPY_TEST_CASE(GetColInfo_SingSel, GetColInfo_Command)
COPY_TEST_CASE(MapColID_SingSel, MapColID_Command)

//NOTE: The #ifdef block below is only for test wizard.  TestWizard has too many 
//strict rules in the parsing code and requires a 1:1 correspondence between
//testcases and the map.  What the #else section is doing is basically "reusing"
//existing testcases by just passing in a parameter which changes the behvior.
//So we make LTM think there are various cases in here with different names, but in
//reality we only have to maintain code for the unique cases. 

#if 0
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(13, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, GetColInfo_Command)
	TEST_CASE(2, GetColInfo_ExecuteRowset)
	TEST_CASE(3, GetColInfo_OpenRowset)
	TEST_CASE(4, GetColInfoRow_Rowset)
	TEST_CASE(5, GetResColInfoRow_Rowset)
	TEST_CASE(6, MapColID_Command)
	TEST_CASE(7, MapColID_ExecuteRowset)
	TEST_CASE(8, MapColID_OpenRowset)
	TEST_CASE(9, MapColIDRow_Rowset)
	TEST_CASE(10, ZombieClassCmd)
	TEST_CASE(11, ZombieClassRowset)
	TEST_CASE(12, ZombieClassRow)
	TEST_CASE(13, ExtendedErrors)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(26, ThisModule, gwszModuleDescrip)
	TEST_CASE_WITH_PARAM(1, GetColInfo_Command, TC_Cmd)
	TEST_CASE_WITH_PARAM(2, GetColInfo_SingSel, TC_SingSel)

	TEST_CASE(3, GetColInfo_ExecuteRowset)

	TEST_CASE(4, GetColInfo_OpenRowset)

	TEST_CASE(5, GetColInfoRow_Rowset)
	TEST_CASE_WITH_PARAM(6, GetColInfoRow_Cmd, TC_Cmd)
	TEST_CASE_WITH_PARAM(7, GetColInfoRow_OpenRW, TC_OpenRW)
	TEST_CASE_WITH_PARAM(8, GetColInfoRow_Bind, TC_Bind)
	TEST_CASE_WITH_PARAM(9, GetColInfoRow_IColInfo2, TC_IColInfo2)

	TEST_CASE(10, GetResColInfoRow_Rowset)
	TEST_CASE_WITH_PARAM(11, GetResColInfoRow_Cmd, TC_Cmd)
	TEST_CASE_WITH_PARAM(12, GetResColInfoRow_OpenRW, TC_OpenRW)
	TEST_CASE_WITH_PARAM(13, GetResColInfoRow_Bind, TC_Bind)

	TEST_CASE_WITH_PARAM(14, MapColID_Command, TC_Cmd)
	TEST_CASE_WITH_PARAM(15, MapColID_SingSel, TC_SingSel)

	TEST_CASE(16, MapColID_ExecuteRowset)

	TEST_CASE(17, MapColID_OpenRowset)

	TEST_CASE(18, MapColIDRow_Rowset)
	TEST_CASE_WITH_PARAM(19, MapColIDRow_Cmd, TC_Cmd)
	TEST_CASE_WITH_PARAM(20, MapColIDRow_OpenRW, TC_OpenRW)
	TEST_CASE_WITH_PARAM(21, MapColIDRow_Bind, TC_Bind)
	TEST_CASE_WITH_PARAM(22, MapColIDRow_IColInfo2, TC_IColInfo2)

	TEST_CASE(23, ZombieClassCmd)

	TEST_CASE(24, ZombieClassRowset)

	TEST_CASE(25, ZombieClassRow)

	TEST_CASE(26, ExtendedErrors)

END_TEST_MODULE()
#endif


// {{ TCW_TC_PROTOTYPE(GetColInfo_Command)
//*-----------------------------------------------------------------------
//| Test Case:		GetColInfo_Command - IColumnsInfo on a command object
//|	Created:		09/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetColInfo_Command::Init()
{
	// Initialize for IColumnsInfo::GetColumnInfo
	m_Method = GETCOLINFO;
	ULONG_PTR ulOleObj = 0;

	if(!GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		GetModInfo()->GetThisTestModule()->m_pIUnknown, &ulOleObj) ||
		!(ulOleObj & DBPROPVAL_OO_SINGLETON))
	{
		if(m_eTestCase == TC_SingSel)
		{
			odtLog<<L"INFO: Obtaining row objects directly from commands is not supported.\n";
			return TEST_SKIPPED;
		}
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	return IColInfo::Init(COMMAND_INTERFACE);
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cColumns is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_1()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	// Set text on command
	TESTC_(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							  NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL),S_OK);

	// Prepare the Statement
	if( m_ePreparation == SUPPORTED ) {
		TESTC_(m_hr=PrepareCommand(m_pICommand,PREPARE,1),S_OK);
	}

	if(m_eTestCase == TC_SingSel)
	{
		TESTC(GetRowIColumnsInfo(m_pICommand, (IUnknown**)&m_pIColumnsInfo))
	}
	else
	{
		TESTC(VerifyInterface(m_pICommand, IID_IColumnsInfo, 
			COMMAND_INTERFACE, (IUnknown **)&m_pIColumnsInfo));
	}

	// Dirty the output params
	m_rgInfo		 = INVALID(DBCOLUMNINFO*);
	m_pStringsBuffer = INVALID(WCHAR*);

	// Run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(NULL, &m_rgInfo, 
											&m_pStringsBuffer), E_INVALIDARG);
		
	// Validate the columns
	CheckEachColumn(m_hr);
	
CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rgInfo is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_2()
{
	TBEGIN;
	
	// Initialize the variation
	INIT;

	// Set text on command
	TESTC_(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							  NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL),S_OK);

	// Prepare the Statement
	if( m_ePreparation == SUPPORTED ) {
		TESTC_(m_hr=PrepareCommand(m_pICommand,PREPARE,1),S_OK);
	}

	if(m_eTestCase == TC_SingSel)
	{
		TESTC(GetRowIColumnsInfo(m_pICommand, (IUnknown**)&m_pIColumnsInfo))
	}
	else
	{
		TESTC(VerifyInterface(m_pICommand, IID_IColumnsInfo, 
			COMMAND_INTERFACE, (IUnknown **)&m_pIColumnsInfo));
	}

	// Dirty the output params
	m_cColumns		 = 99;
	m_pStringsBuffer = INVALID(WCHAR*);
	
	// Run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns,
									NULL, &m_pStringsBuffer), E_INVALIDARG);
		
	// Validate the columns
	CheckEachColumn(m_hr);
	
CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: pStringsBuffer is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_3()
{
	TBEGIN;
	
	// Initialize the variation
	INIT;
	
	// Set text on command
	TESTC_(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							  NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL),S_OK);

	// Prepare the Statement
	if( m_ePreparation == SUPPORTED ) {
		TESTC_(m_hr=PrepareCommand(m_pICommand,PREPARE,1),S_OK);
	}

	if(m_eTestCase == TC_SingSel)
	{
		TESTC(GetRowIColumnsInfo(m_pICommand, (IUnknown**)&m_pIColumnsInfo))
	}
	else
	{
		TESTC(VerifyInterface(m_pICommand, IID_IColumnsInfo, 
			COMMAND_INTERFACE, (IUnknown **)&m_pIColumnsInfo));
	}

	// Dirty the output params
	m_cColumns	= 99;
	m_rgInfo	= INVALID(DBCOLUMNINFO*);

	// Run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
										&m_rgInfo, NULL), E_INVALIDARG);
		
	// Validate the columns
	CheckEachColumn(m_hr);
	
CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: CommandText is not Set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_4()
{
	TBEGIN;
	
	// Initialize the variation
	INIT;
	
	if(m_eTestCase == TC_SingSel)
		goto CLEANUP;  //skip test.

	TESTC(VerifyInterface(m_pICommand, IID_IColumnsInfo, 
		COMMAND_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Dirty the output params
	m_cColumns		= 99;
	m_rgInfo		= INVALID(DBCOLUMNINFO*);
	m_pStringsBuffer= INVALID(WCHAR*);

	// Validate return code
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
						&m_rgInfo, &m_pStringsBuffer), DB_E_NOCOMMAND);
	
	// Validate the columns
	CheckEachColumn(m_hr);

CLEANUP:
	
	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: CommandText is Set to Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_5()
{
	TBEGIN;
	WCHAR * pSQLSet = NULL;
	
	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	// Initialize the variation
	INIT;

	// Dirty the output params
	m_cColumns		= 99;
	m_rgInfo		= INVALID(DBCOLUMNINFO*);
	m_pStringsBuffer= INVALID(WCHAR*);

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSQL,
					SELECT_ALLFROMTBL, pSQLSet, NEITHER, 1), DB_E_NOCOMMAND);
		
	// Validate the columns
	CheckEachColumn(m_hr);

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: CommandText is Set to NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_6()
{
	TBEGIN;
	
	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	// Initialize the variation
	INIT;

	// Dirty the output params
	m_cColumns		= 99;
	m_rgInfo		= INVALID(DBCOLUMNINFO*);
	m_pStringsBuffer= INVALID(WCHAR*);

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSQL,
							SELECT_ALLFROMTBL, NULL, NEITHER, 1), DB_E_NOCOMMAND);
		
	// Validate the columns
	CheckEachColumn(m_hr);

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTPREPARED/S_OK: CommandText is Set, Prepared, and Unprepared
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_7()
{
	TBEGIN;

	HRESULT Exphr = S_OK;
	
	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	// Initialize the variation
	INIT;

	// Dirty the output params
	m_cColumns		= 99;
	m_rgInfo		= INVALID(DBCOLUMNINFO*);
	m_pStringsBuffer= INVALID(WCHAR*);

	// Check to see if the Provider supports ICommandPrepare
	if( m_ePreparation == SUPPORTED )
		Exphr = DB_E_NOTPREPARED;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
							SELECT_COLLISTFROMTBL, NULL, BOTH, 1), Exphr);
		
	// Validate the columns
	CheckEachColumn(m_hr);

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTPREPARED/S_OK: CommandText is Set, but not Prepared
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_8()
{
	TBEGIN;

	HRESULT Exphr = S_OK;

	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	// Initialize the variation
	INIT;

	// Dirty the output params
	m_cColumns		= 99;
	m_rgInfo		= INVALID(DBCOLUMNINFO*);
	m_pStringsBuffer= INVALID(WCHAR*);

	// Check to see if the Provider supports ICommandPrepare
	if( m_ePreparation == SUPPORTED )
		Exphr = DB_E_NOTPREPARED;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
							SELECT_COLLISTFROMTBL, NULL, NEITHER, 1), Exphr);
		
	// Validate the columns
	CheckEachColumn(m_hr);

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText is Set and Prepared
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_9()
{
	TBEGIN;
	
	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_COLLISTFROMTBL, NULL, PREPARE, 1,
								0, (m_eTestCase==TC_SingSel)), S_OK);
		
	// Validate the columns
	CheckEachColumn(m_hr);

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText includes duplicate columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_10()
{
	TBEGIN;
	
	// Initialize the variation
	INIT;

	// Run testing interface, validate params
	TEST2C_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
							SELECT_DUPLICATECOLUMNS, NULL, PREPARE,1,
							0,(m_eTestCase==TC_SingSel)), S_OK, DB_E_NOTSUPPORTED);

	//Skip variation if query is not supported.
	TESTC_PROVIDER(m_hr != DB_E_NOTSUPPORTED)
		
	// Validate the columns
	CheckEachColumn(m_hr);

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText is Set, then change the Text
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_11()
{
	TBEGIN;
	HRESULT	hr=E_FAIL;
	
	// Initialize the variation
	INIT;

	// Set text on command
	TESTC_(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							  NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL),S_OK);

	// Prepare the Statement
	if( m_ePreparation == SUPPORTED ) {
		TESTC_(m_hr=PrepareCommand(m_pICommand,PREPARE,1),S_OK);
	}

	// Set text on command
	TEST2C_(hr = SetCommandText(m_pIMalloc, m_pICommand, m_pTable, 
							  NULL, eSELECT, SELECT_REVCOLLIST, NULL),S_OK, DB_E_NOTSUPPORTED);

	//Skip variation if query is not supported.
	TESTC_PROVIDER(hr != DB_E_NOTSUPPORTED)

	// Prepare the Statement
	if( m_ePreparation == SUPPORTED ) {
		TESTC_(m_hr=PrepareCommand(m_pICommand,PREPARE,1),S_OK);
	}

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_ALLFROMTBL, NULL, PREPARE,1,
								0,(m_eTestCase==TC_SingSel)), S_OK);
		
	// Validate the columns
	CheckEachColumn(m_hr);
	
CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a count(*)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_12()
{
	TBEGIN;
	
	// Initialize the variation
	INIT;

	TEST2C_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
									SELECT_COUNT, NULL, PREPARE,1,
									0,(m_eTestCase==TC_SingSel)), S_OK, DB_E_NOTSUPPORTED);

	//Skip variation if query is not supported.
	TESTC_PROVIDER(m_hr != DB_E_NOTSUPPORTED);

	// Validate the columns
	CheckEachColumn(m_hr,0,SELECT_COUNT);

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains different column names
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_13()
{
	TBEGIN;
	
	// Initialize the variation
	INIT;

	TEST2C_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
							SELECT_CHANGECOLNAME, NULL, PREPARE,1,
							0,(m_eTestCase==TC_SingSel)), S_OK, DB_E_NOTSUPPORTED);

	//Skip variation if query is not supported.
	TESTC_PROVIDER(m_hr != DB_E_NOTSUPPORTED)

	// Validate the columns
	CheckEachColumn(m_hr,0,SELECT_CHANGECOLNAME);

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a insert statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_14()
{
	TBEGIN;

	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	//If the provider is read only, skip the variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
								INSERT_1ROW, NULL, PREPARE), S_OK);

	// Validate the columns
	CheckEachColumn(m_hr);

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a select from an Empty Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_15()
{
	TBEGIN;
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	// Initialize the variation
	INIT;

	if(m_eTestCase==TC_SingSel)
	{
		IRow*	pIRow = NULL;
		hr=SetCommandText(m_pIMalloc,m_pICommand,m_pTable,m_pTable2->GetTableName(),
							eSELECT,SELECT_EMPTYROWSET,NULL,NULL,NULL);

		//Skip variation if query is not supported.
		TESTC_PROVIDER(hr != DB_E_NOTSUPPORTED)

		CHECK(hr, S_OK);

		if(m_ePreparation == SUPPORTED)
			COMPARE(SUCCEEDED(PrepareCommand(m_pICommand,PREPARE,0)), TRUE);

		CHECK(m_pICommand->Execute(NULL, IID_IRow, NULL, NULL, (IUnknown**)&pIRow), DB_E_NOTFOUND);
		SAFE_RELEASE(pIRow);
		FREE;
		return TEST_PASS;
	}

	TEST2C_(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_EMPTYROWSET,	NULL, PREPARE), S_OK, DB_E_NOTSUPPORTED)

	//Skip variation if query is not supported.
	TESTC_PROVIDER(hr != DB_E_NOTSUPPORTED)

	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:
	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Base table contains no rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_16()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	ULONG	cRows	= 0;
	
	// Initialize the variation
	INIT;

	//Check to see if the DSO is ReadOnly
	//Commands are required for Delete
	if(!g_fReadOnlyProvider && m_pTable->GetSQLSupport() != DBPROPVAL_SQL_NONE)
	{
		// Delete all rows in the table.
		if (!CHECK(m_pTable->Delete(),S_OK))
			goto CLEANUP;
	}
	else
	{
		// Drop the table and create an empty one
		if (!CHECK(m_pTable->DropTable(),S_OK))
			goto CLEANUP;

		if (!SUCCEEDED(m_pTable->CreateTable(0,1,NULL,PRIMARY,TRUE)))
			goto CLEANUP;
	}

	if(m_eTestCase==TC_SingSel)
	{
		IRow*	pIRow = NULL;
		TEST2C_(hr=SetCommandText(m_pIMalloc,m_pICommand,m_pTable,m_pTable2->GetTableName(),
							eSELECT,SELECT_EMPTYROWSET,NULL,NULL,NULL), S_OK, DB_E_NOTSUPPORTED);

		//Skip variation if query is not supported.
		if(hr == DB_E_NOTSUPPORTED)
		{
			FREE;
			return TEST_SKIPPED;
		}

		if(m_ePreparation == SUPPORTED)
			COMPARE(SUCCEEDED(PrepareCommand(m_pICommand,PREPARE,0)), TRUE);

		CHECK(m_pICommand->Execute(NULL, IID_IRow, NULL, NULL, (IUnknown**)&pIRow), DB_E_NOTFOUND);
		SAFE_RELEASE(pIRow);
		FREE;
		return TEST_PASS;
	}

	if (CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, SELECT_COLLISTFROMTBL, NULL, PREPARE), S_OK))
		fResult = TEST_PASS;
		
	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:
	for(cRows=0;cRows<10;cRows++)
		m_pTable->Insert();

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a update statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_17()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	//If the provider is read only, skip the variation.
	if(g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE)
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eUPDATE, 
								SELECT_EMPTYROWSET,	NULL, PREPARE), S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a delete statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_18()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	//If the provider is read only, skip the variation.
	if(g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE)
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eDELETE, 
							SELECT_EMPTYROWSET,	NULL, PREPARE);
	if (CHECK(hr, S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_IRowsetLocate on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_19()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	if (!SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"IRowsetLocate is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
			SELECT_ALLFROMTBL, NULL, PREPARE, 1, DBPROP_IRowsetLocate), S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_IRowsetLocate);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_20()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;

	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;
	
	if (!SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Bookmarks are not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
				SELECT_ALLFROMTBL,	NULL, PREPARE, 1, DBPROP_BOOKMARKS), S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr,DBPROP_BOOKMARKS);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_BOOKMARKS on a Insert Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_21()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	if (!SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Bookmarks are not supported.\n";
		return TEST_SKIPPED;
	}

	//If the provider is read only, skip the variation.
	if(g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE)
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
				INSERT_1ROW, NULL, PREPARE, 1, DBPROP_BOOKMARKS);
	
	if (CHECK(hr, S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr,DBPROP_BOOKMARKS);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_DEFERRED on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_22()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;

	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	if (!SupportedProperty(DBPROP_DEFERRED, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Deferred is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
				SELECT_ALLFROMTBL, NULL, PREPARE, 1, DBPROP_DEFERRED), S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr,DBPROP_DEFERRED);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_CACHEDEFERRED on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_23()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;

	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	if (!SupportedProperty(DBPROP_CACHEDEFERRED, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Cache Deferred is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
			SELECT_ALLFROMTBL, NULL, PREPARE, 1, DBPROP_CACHEDEFERRED), S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr,DBPROP_CACHEDEFERRED);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_MAYWRITECOLUMN on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_24()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;

	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	if (!SupportedProperty(DBPROP_MAYWRITECOLUMN, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"May Write Column is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT, 
				SELECT_ALLFROMTBL, NULL, PREPARE, 1, DBPROP_MAYWRITECOLUMN), S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr,DBPROP_MAYWRITECOLUMN);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a select from view
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_25()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	//If the provider is read only, skip the variation.
	if(g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE)
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_REVCOLLISTFROMVIEW,	NULL, PREPARE);

	
	// Check to see if the create view failed
	if (FAILED(hr)) 
	{
		odtLog<<L"Create view not supported" <<ENDL;
		fResult = TEST_PASS;
	}
	else
	{
		// Check to see if everything worked
		if(CHECK(hr, S_OK))
			fResult = TEST_PASS;
	}

	// Validate the columns
	CheckEachColumn(hr);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement with Column List
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_26()
{
	TBEGIN;
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	if (!SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Bookmarks are not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	TEST2C_(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
								SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE,	
								NULL, PREPARE, 1, DBPROP_BOOKMARKS), S_OK, DB_E_NOTSUPPORTED)

	//Skip variation if query is not supported.
	TESTC_PROVIDER(hr != DB_E_NOTSUPPORTED)

	// Validate the columns
	CheckEachColumn(hr,DBPROP_BOOKMARKS);

CLEANUP:
	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a LEFT OUTER JOIN
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_27()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	// Check the SQL Support of the Provider
	if( !(m_pTable->GetSQLSupport() & DBPROPVAL_SQL_ODBC_CORE) )
	{
		odtLog << L"Provider does not support outer joins." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_LEFTOUTERJOIN,	NULL, PREPARE);

	
	// Check to see if the left outer joins work
	if (FAILED(hr)) 
	{
		odtLog<<L"Left Outer Joins not supported" <<ENDL;
		fResult = TEST_PASS;
	}
	else
	{
		// Check to see if everything worked
		if(CHECK(hr, S_OK))
			fResult = TEST_PASS;
	}

	// Validate the columns
	CheckEachColumn(hr, 0, SELECT_LEFTOUTERJOIN);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a RIGHT OUTER JOIN
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_28()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	// Check the SQL Support of the Provider
	if( !(m_pTable->GetSQLSupport() & DBPROPVAL_SQL_ODBC_CORE) )
	{
		odtLog << L"Provider does not support outer joins." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if(m_eTestCase==TC_SingSel)
	{
		IRow*	pIRow = NULL;
		CHECK(hr=SetCommandText(m_pIMalloc,m_pICommand,m_pTable,m_pTable2->GetTableName(),
							eSELECT,SELECT_RIGHTOUTERJOIN,NULL,NULL,NULL), S_OK);
		if(m_ePreparation == SUPPORTED)
			COMPARE(SUCCEEDED(PrepareCommand(m_pICommand,PREPARE,0)), TRUE);

		hr = m_pICommand->Execute(NULL, IID_IRow, NULL, NULL, (IUnknown**)&pIRow);
		COMPARE((hr==DB_E_NOTFOUND) || (hr==DB_E_ERRORSINCOMMAND), TRUE);
		SAFE_RELEASE(pIRow);
		FREE;
		return TEST_PASS;
	}

	hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_RIGHTOUTERJOIN,	NULL, PREPARE);

	
	// Check to see if the right outer joins work
	if (FAILED(hr)) 
	{
		odtLog<<L"Right Outer Joins not supported" <<ENDL;
		fResult = TEST_PASS;
	}
	else
	{
		// Check to see if everything worked
		if(CHECK(hr, S_OK))
			fResult = TEST_PASS;
	}

	// Validate the columns
	CheckEachColumn(hr, 0, SELECT_RIGHTOUTERJOIN);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTABLE: Call with a Table that doesn't exist
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_Command::Variation_29()
{
	TBEGIN;

	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	// Initialize the variation
	INIT;

	TEST3C_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
			SELECT_NO_TABLE, NULL, PREPARE), DB_E_NOTABLE, DB_E_ERRORSINCOMMAND, DB_E_NOTSUPPORTED);

	//Skip variation if query is not supported.
	TESTC_PROVIDER(m_hr != DB_E_NOTSUPPORTED)

	// Validate the columns
	CheckEachColumn(m_hr, 0, SELECT_NO_TABLE);

CLEANUP:
	
	FREE;
	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetColInfo_Command::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(GetColInfo_ExecuteRowset)
//*-----------------------------------------------------------------------
//| Test Case:		GetColInfo_ExecuteRowset - IColumnsInfo on a rowset object
//|	Created:		09/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetColInfo_ExecuteRowset::Init()
{

	// Initialize for IColumnsInfo::GetColumnInfo
	m_Method = GETCOLINFO;

	// {{ TCW_INIT_BASECLASS_CHECK
	return IColInfo::Init(COMMAND_INTERFACE);
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cColumns is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_1()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	// Initialize the variation
	INIT;

	// SetText on command
	if (FAILED(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL)))
		goto CLEANUP;

	if (!CHECK(hr=m_pICommand->Execute(NULL, iid, NULL, &m_cRowsAffected,
										(IUnknown **) &m_pIRowset),S_OK))
		goto CLEANUP;

	if (!m_pIRowset)
		goto CLEANUP;

	// Release the IColumnsInfo Pointer
	SAFE_RELEASE(m_pIColumnsInfo);

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
										(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// Dirty the output params
	m_rgInfo		= INVALID(DBCOLUMNINFO*);
	m_pStringsBuffer= INVALID(WCHAR*);

	// Run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(NULL, &m_rgInfo,
									&m_pStringsBuffer), E_INVALIDARG))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rgInfo is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_2()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	// Initialize the variation
	INIT;

	// SetText on command
	if (FAILED(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL)))
		goto CLEANUP;

	if (!CHECK(hr=m_pICommand->Execute(NULL, iid, NULL, &m_cRowsAffected,
										(IUnknown **) &m_pIRowset),S_OK))
		goto CLEANUP;

	if (!m_pIRowset)
		goto CLEANUP;

	// Release the ColumnsInfo Pointer
	SAFE_RELEASE(m_pIColumnsInfo);

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
										(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// Dirty the output params
	m_cColumns		= 99;
	m_pStringsBuffer= INVALID(WCHAR*);

	// Run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, NULL,
									&m_pStringsBuffer),E_INVALIDARG))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: pStringsBuffer is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_3()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	// Initialize the variation
	INIT;

	// SetText on command
	if (FAILED(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL)))
		goto CLEANUP;

	if (!CHECK(hr=m_pICommand->Execute(NULL, iid, NULL, &m_cRowsAffected,
										(IUnknown **) &m_pIRowset),S_OK))
		goto CLEANUP;

	if (!m_pIRowset)
		goto CLEANUP;

	// Release the ColumnsInfo Pointer
	SAFE_RELEASE(m_pIColumnsInfo);

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
									(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// Dirty the output params
	m_cColumns	= 99;
	m_rgInfo	= INVALID(DBCOLUMNINFO*);

	// run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													NULL),E_INVALIDARG))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText is Set and Executed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_4()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
								SELECT_ALLFROMTBL, NULL, NEITHER, 0),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText is Set, Prepared, Unprepared, and Executed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_5()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
								SELECT_ALLFROMTBL, NULL, BOTH, 0),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText is Set, Prepared, and Executed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_6()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
								SELECT_ALLFROMTBL, NULL, PREPARE, 1),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText that includes duplicate columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_7()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
							SELECT_DUPLICATECOLUMNS, NULL, NEITHER, 1),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText is Set, then change the Text
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_8()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	// Initialize the variation
	INIT;

	if (!CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
							SELECT_DUPLICATECOLUMNS, NULL, NEITHER, 1),S_OK))
		goto CLEANUP;

	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIColumnsInfo);

	FreeColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer);
	PROVIDER_FREE(m_prgOrdTbl1);

	m_cOrdTbl1 = 0;

	// SetText on Command
	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
							SELECT_REVCOLLIST, NULL, PREPARE, 1),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

	if (CHECK(hr,S_OK))
		fResult = TEST_PASS;
		
CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a count(*)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_9()
{
	BOOL	fResult	= TEST_FAIL;

	// Initialize the variation
	INIT;

	TEST2C_(m_hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
								SELECT_COUNT, NULL, NEITHER, 1),S_OK,DB_E_NOTSUPPORTED);
		
	//Skip variation if query is not supported.
	if(m_hr == DB_E_NOTSUPPORTED)
	{
		fResult	= TEST_SKIPPED;
		goto CLEANUP;
	}
	
	// Validate the columns
	CheckEachColumn(m_hr,0,SELECT_COUNT);

	fResult = TEST_PASS;

CLEANUP:
	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains different column names
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_10()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
								SELECT_CHANGECOLNAME, NULL, NEITHER, 1),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr,0,SELECT_CHANGECOLNAME);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a select from an Empty Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_11()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	// Initialize the variation
	INIT;

	TEST2C_(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
									SELECT_EMPTYROWSET, NULL, NEITHER, 1),S_OK,DB_E_NOTSUPPORTED);
	//Skip variation if query is not supported.
	if (hr == DB_E_NOTSUPPORTED)
	{
		odtLog<<L"Statement is not supported" <<ENDL;
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}
	
	fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:
	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Base table contains no rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_12()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	ULONG	count	= 0;

	// Initialize the variation
	INIT;

	// Check to see if the DSO is ReadOnly
	if(!g_fReadOnlyProvider && m_pTable->GetSQLSupport() != DBPROPVAL_SQL_NONE)
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

	if(CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
								SELECT_COLLISTFROMTBL, NULL, NEITHER),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

	for(count=0;count<10;count++)
		m_pTable->Insert();

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_IRowsetLocate on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_13()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	if (!SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"IRowsetLocate is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
				SELECT_COLLISTFROMTBL, NULL, NEITHER, 1, DBPROP_IRowsetLocate),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_IRowsetLocate);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_14()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	if (!SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Bookmarks are not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, NEITHER, 1, DBPROP_BOOKMARKS),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_BOOKMARKS);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_DEFERRED on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_15()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	if (!SupportedProperty(DBPROP_DEFERRED, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Deferred is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, NEITHER, 1, DBPROP_DEFERRED),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_DEFERRED);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_CACHEDEFERRED on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_16()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	if (!SupportedProperty(DBPROP_CACHEDEFERRED, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Cache Deferred is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if (CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
				SELECT_COLLISTFROMTBL, NULL, NEITHER, 1, DBPROP_CACHEDEFERRED),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_CACHEDEFERRED);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_MAYWRITECOLUMN on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_17()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	if (!SupportedProperty(DBPROP_MAYWRITECOLUMN, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"May Write Column is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	if(CHECK(hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
			SELECT_COLLISTFROMTBL, NULL, NEITHER, 1, DBPROP_MAYWRITECOLUMN),S_OK))
		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_MAYWRITECOLUMN);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a select from view
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_18()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;

	//If the provider is read only, skip the variation.
	if(g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE)
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
								SELECT_REVCOLLISTFROMVIEW, NULL, NEITHER, 1);

	// Check to see if the create view failed
	if (FAILED(hr)) 
	{
		odtLog<<L"Create view not supported" <<ENDL;
		fResult = TEST_PASS;
	}
	else
	{
		// Check to see if everything worked
		if(CHECK(hr, S_OK))
			fResult = TEST_PASS;
	}

	// Validate the columns
	CheckEachColumn(hr);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a LEFT OUTER JOIN
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_19()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	// Initialize the variation
	INIT;

	hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_LEFTOUTERJOIN,	NULL, PREPARE);

	
	// Check to see if the left outer joins work
	if (FAILED(hr)) 
	{
		odtLog<<L"Left Outer Joins not supported" <<ENDL;
		fResult = TEST_PASS;
	}
	else
	{
		// Check to see if everything worked
		if(CHECK(hr, S_OK))
			fResult = TEST_PASS;
	}

	// Validate the columns
	CheckEachColumn(hr, 0, SELECT_LEFTOUTERJOIN);

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CommandText contains a RIGHT OUTER JOIN
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_ExecuteRowset::Variation_20()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult = TEST_FAIL;
	
	// Initialize the variation
	INIT;

	hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_RIGHTOUTERJOIN,	NULL, PREPARE);

	
	// Check to see if the right outer joins work
	if (FAILED(hr)) 
	{
		odtLog<<L"Right Outer Joins not supported" <<ENDL;
		fResult = TEST_PASS;
	}
	else
	{
		// Check to see if everything worked
		if(CHECK(hr, S_OK))
			fResult = TEST_PASS;
	}

	// Validate the columns
	CheckEachColumn(hr, 0, SELECT_RIGHTOUTERJOIN);

	FREE;
	return fResult;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetColInfo_ExecuteRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(GetColInfo_OpenRowset)
//*-----------------------------------------------------------------------
//| Test Case:		GetColInfo_OpenRowset - IColumnsInfo on a rowset object
//|	Created:		09/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetColInfo_OpenRowset::Init()
{

	// Initialize for IColumnsInfo::GetColumnInfo
	m_Method = GETCOLINFO;

	// {{ TCW_INIT_BASECLASS_CHECK
	return IColInfo::Init(ROWSET_INTERFACE);
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cColumns is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_1()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	// Initialize the variation
	INIT;

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
		NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
		goto CLEANUP;

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
										(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// Dirty the output params
	m_rgInfo		= INVALID(DBCOLUMNINFO*);
	m_pStringsBuffer= INVALID(WCHAR*);

	// Run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(NULL, &m_rgInfo,
									&m_pStringsBuffer), E_INVALIDARG))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rgInfo is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_2()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	// Initialize the variation
	INIT;

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
		goto CLEANUP;

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
										(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// Dirty the output params
	m_cColumns		= 99;
	m_pStringsBuffer= INVALID(WCHAR*);

	// Run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, NULL,
									&m_pStringsBuffer),E_INVALIDARG))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: pStringsBuffer is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_3()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	// Initialize the variation
	INIT;

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
		goto CLEANUP;

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
									(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// Dirty the output params
	m_cColumns	= 99;
	m_rgInfo	= INVALID(DBCOLUMNINFO*);

	// run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													NULL),E_INVALIDARG))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Basic OpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_4()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	// Initialize the variation
	INIT;

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
		goto CLEANUP;

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
									(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Base table contains no rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_5()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	ULONG	count	= 0;
	IID		iid		= IID_IRowset;

	// Initialize the variation
	INIT;

	// Check to see if the DSO is ReadOnly
	if(!g_fReadOnlyProvider && m_pTable->GetSQLSupport() != DBPROPVAL_SQL_NONE)
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

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
		goto CLEANUP;

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
									(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr);

	for(count=0;count<10;count++)
		m_pTable->Insert();

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_IRowsetLocate on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_6()
{
	HRESULT	hr		= E_FAIL;
	HRESULT	Exphr	= S_OK;
	BOOL	fResult	= TEST_FAIL;
 	IID		iid		= IID_IRowset;

	if (!SupportedProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"IRowsetLocate is not supported.\n";
		return TEST_SKIPPED;
	}
	
	// Initialize the variation
	INIT;

	// Set Property
	SetRowsetProperties(DBPROPSET_ROWSET,DBPROP_IRowsetLocate);

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
	{
		if(CHECK(DB_E_ERRORSOCCURRED, hr) && (!m_pIRowset) && (m_rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSETTABLE))
			fResult = TEST_PASS;

		goto CLEANUP;
	}

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
									(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_IRowsetLocate);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_BOOKMARKS on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_7()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	if (!SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Bookmarks are not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	// Set Property
	SetRowsetProperties(DBPROPSET_ROWSET,DBPROP_BOOKMARKS);

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
	{
		if(CHECK(DB_E_ERRORSOCCURRED, hr) && (!m_pIRowset) && (m_rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSETTABLE))
			fResult = TEST_PASS;

		goto CLEANUP;
	}

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
									(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_BOOKMARKS);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_DEFERRED on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_8()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;
	
	if (!SupportedProperty(DBPROP_DEFERRED, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Deferred is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	// Set Property
	SetRowsetProperties(DBPROPSET_ROWSET,DBPROP_DEFERRED);

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
	{
		if(CHECK(DB_E_ERRORSOCCURRED, hr) && (!m_pIRowset) && (m_rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSETTABLE))
			fResult = TEST_PASS;

		goto CLEANUP;
	}

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
									(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_DEFERRED);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_CACHEDEFERRED on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_9()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	if (!SupportedProperty(DBPROP_CACHEDEFERRED, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"Cache Deferred is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	// Set Property
	SetRowsetProperties(DBPROPSET_ROWSET,DBPROP_CACHEDEFERRED);

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
	{
		if(CHECK(DB_E_ERRORSOCCURRED, hr) && (!m_pIRowset) && (m_rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSETTABLE))
			fResult = TEST_PASS;

		goto CLEANUP;
	}

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
									(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_DEFERRED);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_MAYWRITECOLUMN on a Select Statement
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetColInfo_OpenRowset::Variation_10()
{
	HRESULT	hr		= E_FAIL;
	BOOL	fResult	= TEST_FAIL;
	IID		iid		= IID_IRowset;

	if (!SupportedProperty(DBPROP_MAYWRITECOLUMN, DBPROPSET_ROWSET, m_pIDBInitialize))
	{												
		odtLog << L"May Write Column is not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	// Set Property
	SetRowsetProperties(DBPROPSET_ROWSET,DBPROP_MAYWRITECOLUMN);

	if (FAILED(hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)))
	{
		if(CHECK(DB_E_ERRORSOCCURRED, hr) && (!m_pIRowset) && (m_rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSETTABLE))
			fResult = TEST_PASS;

		goto CLEANUP;
	}

	if (!m_pIRowset)
		goto CLEANUP;

	if (FAILED(m_pIRowset->QueryInterface(IID_IColumnsInfo, 
									(void **)&m_pIColumnsInfo)))
		goto CLEANUP;

	// run testing interface, validate params
	if (CHECK(hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK))
 		fResult = TEST_PASS;

	// Validate the columns
	CheckEachColumn(hr, DBPROP_MAYWRITECOLUMN);

CLEANUP:

	FREE;
	return fResult;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetColInfo_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
}	// }}
// }}
// }}



// {{ TCW_TC_PROTOTYPE(GetColInfoRow_Rowset)
//*-----------------------------------------------------------------------
//| Test Case:		GetColInfoRow_Rowset - GetColumnInfo on a Row object
//| Created:  	10/12/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetColInfoRow_Rowset::Init()
{ 
	EINTERFACE	eIntf = ROWSET_INTERFACE;

	if(m_eTestCase == TC_Cmd)
		eIntf = COMMAND_INTERFACE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(IColInfo::Init(eIntf))
	// }}
	{ 
		m_Method = GETCOLINFO;
		switch(m_eTestCase)
		{
		case TC_Rowset:
			return GetRowFromRowset(FALSE);
		case TC_Cmd:
			return GetRowFromCommand(FALSE);
		case TC_OpenRW:
			return GetRowFromOpenRW(FALSE);
		case TC_Bind:
			return GetRowFromBind(FALSE);
		case TC_IColInfo2:
			return GetRowFromRowset(TRUE);
		default:
			ASSERT(!L"Unhandled Type...");
		}
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Basic
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetColInfoRow_Rowset::Variation_1()
{ 
	TBEGIN

	TESTC_(m_hr=CallMethodOnRowObj(FALSE), S_OK)

	m_cColumns = m_cOrdTbl1 + !(m_rgInfo2[0].iOrdinal) ;
	m_rgInfo = m_rgInfo2;
	CheckEachColumn(m_hr);

	TESTC(m_cColumns2 >= m_cColumns)
	TESTC(CheckRowSpecificColumns(m_cColumns2-m_cColumns, 
		&(m_rgInfo2[m_cColumns]), m_rgInfo2[m_cColumns-1].iOrdinal+1))

CLEANUP:
	CLEAR
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: pcColumns is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetColInfoRow_Rowset::Variation_2()
{ 
	TBEGIN

	if(m_pIColumnsInfo)
		TESTC_(m_pIColumnsInfo->GetColumnInfo(NULL,&m_rgInfo2,&m_pStringsBuffer), E_INVALIDARG)
	else
		TESTC_(m_pIColumnsInfo2->GetColumnInfo(NULL,&m_rgInfo2,&m_pStringsBuffer), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: prgInfo is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetColInfoRow_Rowset::Variation_3()
{ 
	TBEGIN

	if(m_pIColumnsInfo)
		TESTC_(m_pIColumnsInfo->GetColumnInfo(&m_cColumns2,NULL,&m_pStringsBuffer), E_INVALIDARG)
	else
		TESTC_(m_pIColumnsInfo2->GetColumnInfo(&m_cColumns2,NULL,&m_pStringsBuffer), E_INVALIDARG)

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: ppStringsBuffer is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetColInfoRow_Rowset::Variation_4()
{ 
	TBEGIN

	if(m_pIColumnsInfo)
		TESTC_(m_pIColumnsInfo->GetColumnInfo(&m_cColumns2,&m_rgInfo2,NULL), E_INVALIDARG)
	else
		TESTC_(m_pIColumnsInfo2->GetColumnInfo(&m_cColumns2,&m_rgInfo2,NULL), E_INVALIDARG)

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
BOOL GetColInfoRow_Rowset::Terminate()
{ 
	SAFE_RELEASE(m_pIUnknown);
	FREE;

// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(GetResColInfoRow_Rowset)
//*-----------------------------------------------------------------------
//| Test Case:		GetResColInfoRow_Rowset - GetRestrictedColumnInfo on a Row object
//| Created:  	10/12/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetResColInfoRow_Rowset::Init()
{ 
	EINTERFACE	eIntf = ROWSET_INTERFACE;

	if(m_eTestCase == TC_Cmd)
		eIntf = COMMAND_INTERFACE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(IColInfo::Init(eIntf))
	// }}
	{ 
		m_Method = GETRESCOLINFO;
		switch(m_eTestCase)
		{
		case TC_Rowset:
			return GetRowFromRowset(TRUE);
		case TC_Cmd:
			return GetRowFromCommand(TRUE);
		case TC_OpenRW:
			return GetRowFromOpenRW(TRUE);
		case TC_Bind:
			return GetRowFromBind(TRUE);
		default:
			ASSERT(!L"Unhandled Type...");
		}
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: No Masks
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_1()
{ 
	TBEGIN

	//NO MASKS.
	m_cMasks = 0;
	m_rgMasks = NULL;

	TESTC_(m_hr=CallMethodOnRowObj(TRUE), S_OK)

	m_cColumns = m_cOrdTbl1 + !(m_rgInfo2[0].iOrdinal) ;
	m_rgInfo = m_rgInfo2;
	CheckEachColumn(m_hr);

	TESTC(m_cColumns2 >= m_cColumns)
	TESTC(CheckRowSpecificColumns(m_cColumns2-m_cColumns, 
		&(m_rgInfo2[m_cColumns]), m_rgInfo2[m_cColumns-1].iOrdinal+1))

CLEANUP:
	CLEAR
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: 1 name mask
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_2()
{ 
	TBEGIN

	//ONE NAME MASK
	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);
	m_rgMasks[0].eKind = DBKIND_NAME;
	m_rgMasks[0].uName.pwszName = L"C";

	TEST2C_(m_hr=CallMethodOnRowObj(TRUE), S_OK, DB_E_NOCOLUMN)

	if(S_OK == m_hr)
		TESTC(VerifyRestrictions());

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Multiple name masks
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_3()
{ 
	TBEGIN
	DBORDINAL	iMask;
	DBORDINAL	cMasks = 8;

	//6 NAME MASKS
	m_cMasks = cMasks;
	SAFE_ALLOC(m_rgMasks, DBID, cMasks);

	for(iMask=0; iMask<cMasks; iMask++)
		m_rgMasks[iMask].eKind = DBKIND_NAME;
	m_rgMasks[0].uName.pwszName = L"C";
	m_rgMasks[1].uName.pwszName = L"c";
	m_rgMasks[2].uName.pwszName = L"R";
	m_rgMasks[3].uName.pwszName = L"r";
	m_rgMasks[4].uName.pwszName = L"S";
	m_rgMasks[5].uName.pwszName = L"s";
	m_rgMasks[6].uName.pwszName = L"D";
	m_rgMasks[7].uName.pwszName = L"h";

	TEST2C_(m_hr=CallMethodOnRowObj(TRUE), S_OK, DB_E_NOCOLUMN)

	if(S_OK == m_hr)
		TESTC(VerifyRestrictions());

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Existing Column as mask
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_4()
{ 
	TBEGIN
	DBORDINAL	cCol = 0;

	TESTC_(m_hr=CallMethodOnRowObj(TRUE, GETCOLINFO), S_OK)

	TESTC(m_cColumns2>0 && m_rgInfo2)
	if(m_cColumns2>1)
		cCol = 1;

	//ONE NAME MASK
	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);
	TESTC_(DuplicateDBID(m_rgInfo2[cCol].columnid, m_rgMasks), S_OK)

	FreeColumnInfo(&m_cColumns2,&m_rgInfo2,&m_pStringsBuffer);

	TESTC_(m_hr=CallMethodOnRowObj(TRUE), S_OK)

	TESTC(VerifyRestrictions());

CLEANUP:
	ReleaseDBID(m_rgMasks, FALSE);
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: GUID_PROPID mask
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_5()
{ 
	TBEGIN
	ULONG_PTR	ulDocSrc=0;
	BOOL	bDocSrc=FALSE;

	//Get the value of DBPROP_DATASOURCE_TYPE property.
	if(GetProperty(DBPROP_DATASOURCE_TYPE, DBPROPSET_DATASOURCEINFO,
		m_pIDBInitialize, &ulDocSrc))
	{
		if((ulDocSrc & DBPROPVAL_DST_DOCSOURCE) == DBPROPVAL_DST_DOCSOURCE)
			bDocSrc = TRUE;
	}

	//6 GUID MASKS
	m_cMasks = 6;
	SAFE_ALLOC(m_rgMasks, DBID, 6);

	m_rgMasks[0] = DBROWCOL_PARSENAME;
	m_rgMasks[1] = DBROWCOL_PARENTNAME;
	m_rgMasks[2] = DBROWCOL_ABSOLUTEPARSENAME;
	m_rgMasks[3] = DBROWCOL_ISHIDDEN;
	m_rgMasks[4] = DBROWCOL_ISREADONLY;
	m_rgMasks[5] = DBROWCOL_CONTENTTYPE;

	TEST2C_(m_hr=CallMethodOnRowObj(TRUE), S_OK, DB_E_NOCOLUMN)

	if(S_OK == m_hr)
	{
		TESTC(VerifyRestrictions());

		//Since the resource rowset columns were returned,
		//verify the value of DBPROP_DATASOURCE_TYPE property.
		TESTC(bDocSrc);
		odtLog<<L"INFO: This provider supports direct URL binding and is a document source provider.\n";
	}
	else
	{
		TESTC(!bDocSrc);
		odtLog<<L"INFO: This provider is NOT a document source provider.\n";
	}

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Guid_Name mask
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_6()
{ 
	TBEGIN

	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);

	m_rgMasks[0].eKind = DBKIND_GUID_NAME;
	m_rgMasks[0].uGuid.guid = guidMod;
	m_rgMasks[0].uName.pwszName = L"C";

	TEST2C_(m_hr=CallMethodOnRowObj(TRUE), S_OK, DB_E_NOCOLUMN)

	if(S_OK == m_hr)
		TESTC(VerifyRestrictions());

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Pguid_Name mask
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_7()
{ 
	TBEGIN

	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);

	m_rgMasks[0].eKind = DBKIND_PGUID_NAME;
	m_rgMasks[0].uGuid.pguid = &guidMod;
	m_rgMasks[0].uName.pwszName = L"C";

	TEST2C_(m_hr=CallMethodOnRowObj(TRUE), S_OK, DB_E_NOCOLUMN)

	if(S_OK == m_hr)
		TESTC(VerifyRestrictions());

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: PGuid_Propid mask
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_8()
{ 
	TBEGIN

	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);

	m_rgMasks[0].eKind = DBKIND_PGUID_PROPID;
	m_rgMasks[0].uGuid.pguid = &guidMod;
	m_rgMasks[0].uName.ulPropid = 2;

	TEST2C_(m_hr=CallMethodOnRowObj(TRUE), S_OK, DB_E_NOCOLUMN)

	if(S_OK == m_hr)
		TESTC(VerifyRestrictions());

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Propid mask
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_9()
{ 
	TBEGIN

	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);

	m_rgMasks[0].eKind = DBKIND_PROPID;
	m_rgMasks[0].uName.ulPropid = 2;

	TEST2C_(m_hr=CallMethodOnRowObj(TRUE), S_OK, DB_E_NOCOLUMN)

	if(S_OK == m_hr)
		TESTC(VerifyRestrictions());

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Guid mask
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_10()
{ 
	TBEGIN

	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);

	m_rgMasks[0].eKind = DBKIND_GUID;
	m_rgMasks[0].uGuid.guid = guidMod;

	TEST2C_(m_hr=CallMethodOnRowObj(TRUE), S_OK, DB_E_NOCOLUMN)

	if(S_OK == m_hr)
		TESTC(VerifyRestrictions());

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK: prgColumnIDs=NULL but prgColumnInfo != NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_11()
{ 
	TBEGIN
	DBORDINAL	cCol = 0;

	TESTC_(m_hr=CallMethodOnRowObj(TRUE, GETCOLINFO), S_OK)

	TESTC(m_cColumns2>0 && m_rgInfo2)
	if(m_cColumns2>1)
		cCol = 1;

	//ONE NAME MASK
	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);
	TESTC_(DuplicateDBID(m_rgInfo2[cCol].columnid, m_rgMasks), S_OK)

	FreeColumnInfo(&m_cColumns2,&m_rgInfo2,&m_pStringsBuffer);

	TESTC_(m_hr = m_pIColumnsInfo2->GetRestrictedColumnInfo(m_cMasks, 
		m_rgMasks, 0, &m_cColumns2,NULL,&m_rgInfo2,&m_pStringsBuffer), S_OK)

	TESTC(CheckParams(m_hr, GETRESCOLINFO))
	TESTC(VerifyRestrictions());

CLEANUP:
	ReleaseDBID(m_rgMasks, FALSE);
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK: prgColumnInfo=NULL but prgColumnIDs != NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_12()
{ 
	TBEGIN
	DBORDINAL	cCol = 0;

	TESTC_(m_hr=CallMethodOnRowObj(TRUE, GETCOLINFO), S_OK)

	TESTC(m_cColumns2>0 && m_rgInfo2)
	if(m_cColumns2>1)
		cCol = 1;

	//ONE NAME MASK
	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);
	TESTC_(DuplicateDBID(m_rgInfo2[cCol].columnid, m_rgMasks), S_OK)

	FreeColumnInfo(&m_cColumns2,&m_rgInfo2,&m_pStringsBuffer);

	TESTC_(m_hr = m_pIColumnsInfo2->GetRestrictedColumnInfo(m_cMasks, 
		m_rgMasks, 0, &m_cColumns2,&m_rgColumnIDs2,NULL,&m_pStringsBuffer), S_OK)

	TESTC(CheckParams(m_hr, GETRESCOLINFO))
	TESTC(VerifyRestrictions());

CLEANUP:
	ReleaseDBID(m_rgMasks, FALSE);
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCOLUMNID: invalid DBID for rgColumnIDMasks
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_13()
{ 
	TBEGIN

	//ONE NAME MASK
	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);
	m_rgMasks[0].eKind = DBKIND_GUID + 100;  //Some large number.
	m_rgMasks[0].uName.pwszName = L"n";

	TESTC_(m_hr=CallMethodOnRowObj(TRUE), DB_E_BADCOLUMNID)

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOLUMN: Non-existent name for mask
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_14()
{ 
	TBEGIN
	DBORDINAL	iMask;

	//6 NAME MASKS
	m_cMasks = 6;
	SAFE_ALLOC(m_rgMasks, DBID, 6);

	for(iMask=0; iMask<6; iMask++)
		m_rgMasks[iMask].eKind = DBKIND_NAME;
	m_rgMasks[0].uName.pwszName = L"NonExistentColumn1";
	m_rgMasks[1].uName.pwszName = L"NonExistentColumn2";
	m_rgMasks[2].uName.pwszName = L"NonExistentColumn3";
	m_rgMasks[3].uName.pwszName = L"NonExistentColumn4";
	m_rgMasks[4].uName.pwszName = L"NonExistentColumn5";
	m_rgMasks[5].uName.pwszName = L"NonExistentColumn6";

	TESTC_(m_hr=CallMethodOnRowObj(TRUE), DB_E_NOCOLUMN)

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cColumnIDMasks=1 and rgColumnIDMasks=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_15()
{ 
	TBEGIN

	TESTC_(m_pIColumnsInfo2->GetRestrictedColumnInfo(1, NULL, 0, &m_cColumns2,&m_rgColumnIDs2,&m_rgInfo2,&m_pStringsBuffer), E_INVALIDARG)

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: pcColumns=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_16()
{ 
	TBEGIN

	//ONE NAME MASK
	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);
	m_rgMasks[0].eKind = DBKIND_NAME;
	m_rgMasks[0].uName.pwszName = L"n";

	TESTC_(m_pIColumnsInfo2->GetRestrictedColumnInfo(m_cMasks, m_rgMasks, 0, NULL,&m_rgColumnIDs2,&m_rgInfo2,&m_pStringsBuffer), E_INVALIDARG)

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: ppStringsBuffer=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_17()
{ 
	TBEGIN

	//ONE NAME MASK
	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);
	m_rgMasks[0].eKind = DBKIND_NAME;
	m_rgMasks[0].uName.pwszName = L"n";

	TESTC_(m_pIColumnsInfo2->GetRestrictedColumnInfo(m_cMasks, m_rgMasks, 0, &m_cColumns2,&m_rgColumnIDs2,&m_rgInfo2,NULL), E_INVALIDARG)

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: prgColumnIDs=NULL and prgColumnInfo=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_18()
{ 
	TBEGIN

	//ONE NAME MASK
	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);
	m_rgMasks[0].eKind = DBKIND_NAME;
	m_rgMasks[0].uName.pwszName = L"n";

	TESTC_(m_pIColumnsInfo2->GetRestrictedColumnInfo(m_cMasks, m_rgMasks, 
		0, &m_cColumns2,NULL,NULL,&m_pStringsBuffer), E_INVALIDARG)

	TESTC_(m_pIColumnsInfo2->GetRestrictedColumnInfo(0, m_rgMasks, 0, 
		&m_cColumns2,NULL,NULL,NULL), E_INVALIDARG)

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: ppStringsBuffer=NULL & either prgColumnIDs or prgColumnInfo = NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int GetResColInfoRow_Rowset::Variation_19()
{ 
	TBEGIN

	//ONE NAME MASK
	m_cMasks = 1;
	SAFE_ALLOC(m_rgMasks, DBID, 1);
	m_rgMasks[0].eKind = DBKIND_NAME;
	m_rgMasks[0].uName.pwszName = L"n";

	CHECK(m_pIColumnsInfo2->GetRestrictedColumnInfo(m_cMasks, m_rgMasks, 
		0, &m_cColumns2,NULL, &m_rgInfo2, NULL), E_INVALIDARG);

	CHECK(m_pIColumnsInfo2->GetRestrictedColumnInfo(m_cMasks, m_rgMasks, 
		0, &m_cColumns2,&m_rgColumnIDs2,NULL,NULL), E_INVALIDARG);

	CHECK(m_pIColumnsInfo2->GetRestrictedColumnInfo(0, m_rgMasks, 
		0, &m_cColumns2,NULL,&m_rgInfo2,NULL), E_INVALIDARG);

	CHECK(m_pIColumnsInfo2->GetRestrictedColumnInfo(0, m_rgMasks, 
		0, &m_cColumns2,&m_rgColumnIDs2,NULL,NULL), E_INVALIDARG);

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL GetResColInfoRow_Rowset::Terminate()
{ 
	SAFE_RELEASE(m_pIUnknown);
	FREE;

// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(MapColID_Command)
//*-----------------------------------------------------------------------
//| Test Case:		MapColID_Command - MapColumnIDs on a command object
//|	Created:		09/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MapColID_Command::Init()
{
	// Initialize for IColumnsInfo::MapColumnIDs
	m_Method = MAPCOLID;

	ULONG_PTR ulOleObj = 0;

	if(!GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		GetModInfo()->GetThisTestModule()->m_pIUnknown, &ulOleObj) ||
		!(ulOleObj & DBPROPVAL_OO_SINGLETON))
	{
		if(m_eTestCase == TC_SingSel)
		{
			odtLog<<L"INFO: Obtaining row objects directly from commands is not supported.\n";
			return TEST_SKIPPED;
		}
	}

	// {{ TCW_INIT_BASECLASS_CHECK
	return IColInfo::Init(COMMAND_INTERFACE);
	// }}
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cColumnIDs != 0, range = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_1()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, FALSE, TRUE, eSELECT,
				SELECT_COLLISTFROMTBL, NULL, PREPARE),E_INVALIDARG);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rgColumns is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_2()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE),E_INVALIDARG);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: CommandText is not Set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_3()
{
	TBEGIN;

	DBORDINAL ulIndex = 0;

	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	// Initialize the variation
	INIT;
	
	// Initialize to Params
	m_cColumnIDs = 1;
	m_rgColumnIDs = (DBID *) PROVIDER_ALLOC(sizeof(DBID) * m_cColumnIDs);
	m_rgColumns = (DBORDINAL *) PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cColumnIDs);
	
	// Check the pointers and memset to invalid signature
	TESTC(m_rgColumnIDs != NULL);
	TESTC(m_rgColumns != NULL);

	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	TESTC(VerifyInterface(m_pICommand, IID_IColumnsInfo, 
						COMMAND_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Validate return code
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs,m_rgColumnIDs,
											m_rgColumns), DB_E_NOCOMMAND);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: CommandText is Set to Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_4()
{
	TBEGIN;
	DBORDINAL	ulIndex = 0;
	WCHAR * pSQLSet = NULL;
	
	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	// Initialize the variation
	INIT;

	// Fillout the Columns
	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// SetText on command
	TESTC_(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							NULL, eSQL, SELECT_COLLISTFROMTBL, pSQLSet),S_OK);
	
	// Reset the rgColumns array
	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs,m_rgColumnIDs,
											m_rgColumns), DB_E_NOCOMMAND);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: CommandText is Set to NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_5()
{
	TBEGIN;
	DBORDINAL	ulIndex = 0;

	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	// Initialize the variation
	INIT;

	// Fillout the Columns
	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// SetText on command
	TESTC_(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							NULL, eSQL, SELECT_COLLISTFROMTBL, NULL),S_OK);

	// Reset the rgColumns array
	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs,m_rgColumnIDs,
										m_rgColumns), DB_E_NOCOMMAND);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTPREPARED/S_OK: CommandText is Set, but not Prepared
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_6()
{
	TBEGIN;

	HRESULT Exphr = S_OK;

	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	// Initialize the variation
	INIT;

	// Check to see if the Provider supports ICommandPrepare
	if( m_ePreparation == SUPPORTED )
		Exphr = DB_E_NOTPREPARED;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
							SELECT_REVCOLLIST, NULL, NEITHER, 1), Exphr);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTPREPARED/S_OK: CommandText is Set, Prepared, and Unprepared
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_7()
{
	TBEGIN;

	HRESULT Exphr = S_OK;

	if(m_eTestCase==TC_SingSel)
		goto CLEANUP;

	// Initialize the variation
	INIT;

	// Check to see if the Provider supports ICommandPrepare
	if( m_ePreparation == SUPPORTED )
		Exphr = DB_E_NOTPREPARED;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
							SELECT_COLLISTFROMTBL, NULL, BOTH, 1), Exphr);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: All ColumnIDs are invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_8()
{
	TBEGIN;
	DBORDINAL ulIndex = 0;

	// Initialize the variation
	INIT;

	// Initialize to Params
	m_cColumnIDs = 5;
	m_rgColumnIDs = (DBID *) PROVIDER_ALLOC(sizeof(DBID) * m_cColumnIDs);
	m_rgColumns = (DBORDINAL *) PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cColumnIDs);
	
	TESTC(m_rgColumnIDs != NULL);
	TESTC(m_rgColumns != NULL);

	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		m_rgColumns[ulIndex] = 55;

		m_rgColumnIDs[ulIndex].uGuid.guid = GUID_NULL;
		m_rgColumnIDs[ulIndex].eKind = 1000;
		m_rgColumnIDs[ulIndex].uName.ulPropid = 1000;
	}

	TESTC_(m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)),DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: One ColumnID is invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_9()
{
	TBEGIN;
	HRESULT Exphr	= S_OK;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( !ulIndex )
		{
			// Check to see if there is only one column
			Exphr = DB_E_ERRORSOCCURRED;

			m_rgColumnIDs[ulIndex].uGuid.guid = GUID_NULL;
			m_rgColumnIDs[ulIndex].eKind = 1000;
			m_rgColumnIDs[ulIndex].uName.ulPropid = 1000;
			continue;
		}

		// Check to see if there is more than one column
		Exphr = DB_S_ERRORSOCCURRED;
	}

	TESTC_(m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
						0,(m_eTestCase==TC_SingSel)),Exphr);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Count of ColumnIDs is 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_10()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(FALSE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
						0,(m_eTestCase==TC_SingSel)),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Valid count and valid range of ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_11()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
						0,(m_eTestCase==TC_SingSel)),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_BOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_12()
{
	TBEGIN;

	if( !SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize) )
	{												
		odtLog << L"Bookmarks are not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE, 1, DBPROP_BOOKMARKS,(m_eTestCase==TC_SingSel)),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Duplicate ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_13()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_DUPLICATECOLUMNS, NULL, PREPARE,1,
						0,(m_eTestCase==TC_SingSel)),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK: ColumnIDs in reverse order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_14()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_REVCOLLIST, NULL, PREPARE,1,
						0,(m_eTestCase==TC_SingSel)),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK: ColumnIDs from a view
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_15()
{
	TBEGIN;

	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	//If the provider is read only, skip the variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_REVCOLLISTFROMVIEW, NULL, PREPARE);

	// Check to see if the create view failed
	if( FAILED(m_hr) )
		odtLog<<L"Create view not supported" <<ENDL;
	else {
		TESTC_(m_hr, S_OK);
	}

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK: ColumnIDs from a count(*)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_16()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TEST2C_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COUNT, NULL, PREPARE,1,
						0,(m_eTestCase==TC_SingSel)),S_OK,DB_E_NOTSUPPORTED);
	
			//Skip variation if query is not supported.
	TESTC_PROVIDER(m_hr != DB_E_NOTSUPPORTED);
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Change the DBKIND in the ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_17()
{
	TBEGIN;
	DBORDINAL	ulIndex	 = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( (m_rgColumnIDs[ulIndex].eKind == DBKIND_NAME) ||
			(m_rgColumnIDs[ulIndex].eKind == DBKIND_GUID_NAME) ||
			(m_rgColumnIDs[ulIndex].eKind == DBKIND_PGUID_NAME) )
		{
			m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
			m_rgColumnIDs[ulIndex].uName.ulPropid = 1;
		}
		else
		{
			m_rgColumnIDs[ulIndex].eKind = DBKIND_NAME;
			m_rgColumnIDs[ulIndex].uName.pwszName = L"Bogus";
		}
	}

	TESTC_(m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)), DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Ask for Column 0 with DBKIND_PROPID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_18()
{
	TBEGIN;
	DBORDINAL	ulIndex	 = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
		m_rgColumnIDs[ulIndex].uName.ulPropid = 0;
	}

	TESTC_(m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)), DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_19()
{
	TBEGIN;
	DBORDINAL	ulIndex	 = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		switch(m_rgColumnIDs[ulIndex].eKind)
		{
			case DBKIND_NAME:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_NAME;
				break;
			case DBKIND_GUID_NAME:
			case DBKIND_PGUID_NAME:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_NAME;
				break;
			case DBKIND_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_PROPID;
				break;
			case DBKIND_GUID_PROPID:
			case DBKIND_PGUID_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
				break;
			case DBKIND_GUID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_PROPID;
				break;
			default:
				assert(FALSE);
		}
	}

	CHECKW(m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)), DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr, FALSE));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Change the GUID in the DBID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_20()
{
	TBEGIN;
	DBORDINAL	ulIndex	 = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		switch(m_rgColumnIDs[ulIndex].eKind)
		{
			case DBKIND_PGUID_NAME:
			case DBKIND_PGUID_PROPID:
				*m_rgColumnIDs[ulIndex].uGuid.pguid = DBPROPSET_ROWSET;
				break;

			case DBKIND_NAME:
			case DBKIND_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID;
			case DBKIND_GUID:
			case DBKIND_GUID_NAME:
			case DBKIND_GUID_PROPID:
				m_rgColumnIDs[ulIndex].uGuid.guid = DBPROPSET_ROWSET;
				break;
		}
	}

	TESTC_(m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)), DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: ColumnID one greater than the last ordinal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_21()
{
	TBEGIN;
	HRESULT Exphr	= S_OK;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( !ulIndex )
		{ 
			// Check to see if there is only one column
			Exphr = DB_E_ERRORSOCCURRED;

			if( m_rgColumnIDs[ulIndex].eKind != DBKIND_PROPID &&
				m_rgColumnIDs[ulIndex].eKind != DBKIND_GUID_PROPID &&
				m_rgColumnIDs[ulIndex].eKind != DBKIND_PGUID_PROPID )
				m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
		
			m_rgColumnIDs[ulIndex].uName.ulPropid = ULONG(m_cColumnIDs+2);
			continue;
		}

		// Check to see if there is more than one column
		Exphr = DB_S_ERRORSOCCURRED;
	}


	TESTC_(m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)),Exphr);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Ask for Column 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_22()
{
	TBEGIN;
	DBORDINAL	ulIndex	 = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
					0,(m_eTestCase==TC_SingSel)),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( m_rgColumnIDs[ulIndex].eKind != DBKIND_PROPID &&
			m_rgColumnIDs[ulIndex].eKind != DBKIND_GUID_PROPID &&
			m_rgColumnIDs[ulIndex].eKind != DBKIND_PGUID_PROPID )
			m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;

		m_rgColumnIDs[ulIndex].uName.ulPropid = 0;
	}

	m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
							SELECT_COLLISTFROMTBL, NULL, PREPARE,1,
							0,(m_eTestCase==TC_SingSel));
	
	// May or may not find the bookmark column
	if( GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommand) ) {
		TEST2C_(m_hr, DB_E_ERRORSOCCURRED, DB_S_ERRORSOCCURRED);
	}
	else {
		TESTC_(m_hr, DB_E_ERRORSOCCURRED);
	}

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOTABLE: Call with a Table that doesn't exist
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_Command::Variation_23()
{
	TBEGIN;
	DBORDINAL ulIndex = 0;

	if(m_eTestCase==TC_SingSel)
		return TEST_SKIPPED;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
								SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// Reset the rgColumns array
	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	TEST3C_(m_hr=ExecuteMethod_cmd(FALSE, FALSE, FALSE, eSELECT,
			SELECT_NO_TABLE, NULL, PREPARE), DB_E_NOTABLE, DB_E_ERRORSINCOMMAND,DB_E_NOTSUPPORTED);
	
		//Skip variation if query is not supported.
	TESTC_PROVIDER(m_hr != DB_E_NOTSUPPORTED)
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MapColID_Command::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(MapColID_ExecuteRowset)
//*-----------------------------------------------------------------------
//| Test Case:		MapColID_ExecuteRowset - MapColumnIDs on a rowset
//|	Created:		09/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MapColID_ExecuteRowset::Init()
{		
	// Initialize for IColumnsInfo::MapColumnIDs
	m_Method = MAPCOLID;

	// {{ TCW_INIT_BASECLASS_CHECK
	return IColInfo::Init(COMMAND_INTERFACE);
	// }}
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cColumnIDs != 0, range = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_1()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, FALSE, TRUE, eSELECT,
				SELECT_COLLISTFROMTBL, NULL, NEITHER),E_INVALIDARG);

	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rgColumns is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_2()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, FALSE, eSELECT,
				SELECT_COLLISTFROMTBL, NULL, NEITHER),E_INVALIDARG);

	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: All ColumnIDs are invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_3()
{
	TBEGIN;
	DBORDINAL ulIndex = 0;

	// Initialize the variation
	INIT;

	// Initialize to Params
	m_cColumnIDs = 5;
	m_rgColumnIDs = (DBID *) PROVIDER_ALLOC(sizeof(DBID) * m_cColumnIDs);
	m_rgColumns = (DBORDINAL *) PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cColumnIDs);
	
	TESTC(m_rgColumnIDs != NULL);
	TESTC(m_rgColumns != NULL);

	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		m_rgColumns[ulIndex] = 55;

		m_rgColumnIDs[ulIndex].uGuid.guid = GUID_NULL;
		m_rgColumnIDs[ulIndex].eKind = 1000;
		m_rgColumnIDs[ulIndex].uName.ulPropid = 1000;
	}

	TESTC_(m_hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, NEITHER),DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: One ColumnID is invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_4()
{
	TBEGIN;
	HRESULT Exphr	= S_OK;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// Release the Rowset created
	SAFE_RELEASE(m_pIRowset);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( !ulIndex )
		{
			// Check to see if there is only one column
			Exphr = DB_E_ERRORSOCCURRED;

			m_rgColumnIDs[ulIndex].uGuid.guid = GUID_NULL;
			m_rgColumnIDs[ulIndex].eKind = 1000;
			m_rgColumnIDs[ulIndex].uName.ulPropid = 1000;
			continue;
		}

		// Check to see if there is more than one column
		Exphr = DB_S_ERRORSOCCURRED;
	}

	// Check to see if there is only one column
	if( m_cColumnIDs == 1 )
		Exphr = DB_E_ERRORSOCCURRED;

	TESTC_(m_hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, NEITHER),Exphr);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Count of ColumnIDs is 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_5()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(FALSE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, NEITHER),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Valid count and valid range of ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_6()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, NEITHER),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_BOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_7()
{
	TBEGIN;

	if( !SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize) )
	{												
		odtLog << L"Bookmarks are not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, NEITHER,1, DBPROP_BOOKMARKS),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Duplicate ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_8()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
						SELECT_DUPLICATECOLUMNS, NULL, NEITHER),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: ColumnIDs in reverse order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_9()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
						SELECT_REVCOLLIST, NULL, NEITHER),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: ColumnIDs from a view
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_10()
{
	TBEGIN;

	//If the provider is read only, skip the variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
	{
		odtLog << L"Provider is ReadOnly." << ENDL;
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
						SELECT_REVCOLLISTFROMVIEW, NULL, PREPARE);

	// Check to see if the create view failed
	if( FAILED(m_hr) )
		odtLog<<L"Create view not supported" <<ENDL;
	else {
		TESTC_(m_hr, S_OK);
	}

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK: MapColumnIDs on optional columns in IColumnsRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_11()
{
	TBEGIN;
	DBORDINAL		ulOptColOrd		= 99;
	DBORDINAL		cOptColumns		= 0;
	DBID*			rgOptColumns	= NULL;
	IColumnsRowset*	pIColumnsRowset	= NULL;
	IRowset*		pIRowsetColumns	= NULL;

	// Initialize the variation
	INIT;

	// SetText on command and get a Rowset
	TESTC_(SetCommandText(m_pIMalloc, m_pICommand, m_pTable,
							NULL, eSELECT, SELECT_COLLISTFROMTBL, NULL),S_OK);

	TESTC_(m_pICommand->Execute(NULL,IID_IRowset,NULL,
									NULL,(IUnknown **)&m_pIRowset),S_OK);

	// Check to see if IColumnsRowset is supported
	TESTC_PROVIDER(VerifyInterface(m_pIRowset, IID_IColumnsRowset, 
						ROWSET_INTERFACE, (IUnknown **)&pIColumnsRowset));

	TESTC_(pIColumnsRowset->GetAvailableColumns(&cOptColumns,
													&rgOptColumns),S_OK);
	// Check for an optional column
	TESTC_PROVIDER(cOptColumns && rgOptColumns);

	// Pass first optional column
	TESTC_(pIColumnsRowset->GetColumnsRowset(NULL, 1, rgOptColumns,
					IID_IRowset, 0, NULL, (IUnknown**)&pIRowsetColumns),S_OK);

	// Get a IID_IColumnsInfo pointer
	TESTC(VerifyInterface(pIRowsetColumns, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
									&m_rgInfo, &m_pStringsBuffer), S_OK);

	TESTC_(m_pIColumnsInfo->MapColumnIDs(1, rgOptColumns, 
											&ulOptColOrd),S_OK);
	
	TESTC(m_rgInfo[m_cColumns-1].iOrdinal == ulOptColOrd);
	
CLEANUP:
	
	PROVIDER_FREE(rgOptColumns);

	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pIRowsetColumns);

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK: ColumnIDs from a count(*)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_12()
{
	TBEGIN;

	// Initialize the variation
	INIT;

	TEST2C_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COUNT, NULL, NEITHER),S_OK,DB_E_NOTSUPPORTED);
		//Skip variation if query is not supported.
	TESTC_PROVIDER(m_hr != DB_E_NOTSUPPORTED)
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Change the DBKIND in the ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_13()
{
	TBEGIN;
	DBORDINAL ulIndex  = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// Release the Rowset created
	SAFE_RELEASE(m_pIRowset);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( (m_rgColumnIDs[ulIndex].eKind == DBKIND_NAME) ||
			(m_rgColumnIDs[ulIndex].eKind == DBKIND_GUID_NAME) ||
			(m_rgColumnIDs[ulIndex].eKind == DBKIND_PGUID_NAME) )
		{
			m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
			m_rgColumnIDs[ulIndex].uName.ulPropid = 1;
		}
		else
		{
			m_rgColumnIDs[ulIndex].eKind = DBKIND_NAME;
			m_rgColumnIDs[ulIndex].uName.pwszName = L"Bogus";
		}
	}

	TESTC_(m_hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, NEITHER), DB_E_ERRORSOCCURRED);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Ask for Column 0 with DBKIND_PROPID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_14()
{
	TBEGIN;
	DBORDINAL ulIndex  = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// Release the Rowset created
	SAFE_RELEASE(m_pIRowset);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
		m_rgColumnIDs[ulIndex].uName.ulPropid = 0;
	}

	TESTC_(m_hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, NEITHER), DB_E_ERRORSOCCURRED);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_15()
{
	TBEGIN;
	DBORDINAL ulIndex  = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// Release the Rowset created
	SAFE_RELEASE(m_pIRowset);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		switch(m_rgColumnIDs[ulIndex].eKind)
		{
			case DBKIND_NAME:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_NAME;
				break;
			case DBKIND_GUID_NAME:
			case DBKIND_PGUID_NAME:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_NAME;
				break;
			case DBKIND_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_PROPID;
				break;
			case DBKIND_GUID_PROPID:
			case DBKIND_PGUID_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
				break;
			case DBKIND_GUID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_PROPID;
				break;
			default:
				assert(FALSE);
		}
	}

	CHECKW(m_hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, NEITHER), DB_E_ERRORSOCCURRED);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr, FALSE));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Change the GUID in the DBID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_16()
{
	TBEGIN;
	DBORDINAL ulIndex  = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// Release the Rowset created
	SAFE_RELEASE(m_pIRowset);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		switch(m_rgColumnIDs[ulIndex].eKind)
		{
			case DBKIND_PGUID_NAME:
			case DBKIND_PGUID_PROPID:
				*m_rgColumnIDs[ulIndex].uGuid.pguid = DBPROPSET_ROWSET;
				break;

			case DBKIND_NAME:
			case DBKIND_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID;
			case DBKIND_GUID:
			case DBKIND_GUID_NAME:
			case DBKIND_GUID_PROPID:
				m_rgColumnIDs[ulIndex].uGuid.guid = DBPROPSET_ROWSET;
				break;
		}
	}

	TESTC_(m_hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, NEITHER), DB_E_ERRORSOCCURRED);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: ColumnID one greater than the last ordinal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_17()
{
	TBEGIN;
	HRESULT Exphr	= S_OK;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// Release the Rowset created
	SAFE_RELEASE(m_pIRowset);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( !ulIndex )
		{ 
			// Check to see if there is only one column
			Exphr = DB_E_ERRORSOCCURRED;

			if( m_rgColumnIDs[ulIndex].eKind != DBKIND_PROPID &&
				m_rgColumnIDs[ulIndex].eKind != DBKIND_GUID_PROPID &&
				m_rgColumnIDs[ulIndex].eKind != DBKIND_PGUID_PROPID )
				m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
			
			m_rgColumnIDs[ulIndex].uName.ulPropid = ULONG(m_cColumnIDs+2);
			continue;
		}

		// Check to see if there is more than one column
		Exphr = DB_S_ERRORSOCCURRED;
	}

	// Check to see if there is only one column
	if( m_cColumnIDs == 1 )
		Exphr = DB_E_ERRORSOCCURRED;

	TESTC_(m_hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE),Exphr);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Ask for Column 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_ExecuteRowset::Variation_18()
{
	TBEGIN;
	DBORDINAL ulIndex  = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
					SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK);

	// Release the Rowset created
	SAFE_RELEASE(m_pIRowset);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( m_rgColumnIDs[ulIndex].eKind != DBKIND_PROPID &&
			m_rgColumnIDs[ulIndex].eKind != DBKIND_GUID_PROPID &&
			m_rgColumnIDs[ulIndex].eKind != DBKIND_PGUID_PROPID )
			m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
		
		m_rgColumnIDs[ulIndex].uName.ulPropid = 0;
	}

	m_hr=ExecuteMethod_row(FALSE, FALSE, FALSE, eSELECT,
							SELECT_COLLISTFROMTBL, NULL, PREPARE);
	
	// May or may not find the bookmark column
	if( GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pICommand) ) {
		TEST2C_(m_hr, DB_E_ERRORSOCCURRED, DB_S_ERRORSOCCURRED);
	}
	else {
		TESTC_(m_hr, DB_E_ERRORSOCCURRED);
	}

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MapColID_ExecuteRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(MapColID_OpenRowset)
//*-----------------------------------------------------------------------
//| Test Case:		MapColID_OpenRowset - MapColumnIDs on a rowset
//|	Created:		09/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MapColID_OpenRowset::Init()
{		
	// Initialize for IColumnsInfo::MapColumnIDs
	m_Method = MAPCOLID;

	// {{ TCW_INIT_BASECLASS_CHECK
	return IColInfo::Init(ROWSET_INTERFACE);
	// }}
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cColumnIDs != 0, range = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_1()
{
	TBEGIN;

	IID	iid	= IID_IRowset;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,FALSE,TRUE),S_OK);

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns),E_INVALIDARG);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rgColumns is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_2()
{
	TBEGIN;
	
	IID	iid	= IID_IRowset;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,FALSE),S_OK);

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns),E_INVALIDARG);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: All ColumnIDs are invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_3()
{
	TBEGIN;
	IID	  iid	  = IID_IRowset;
	DBORDINAL ulIndex = 0;

	// Initialize the variation
	INIT;

	// Initialize to Params
	m_cColumnIDs = 5;
	m_rgColumnIDs = (DBID *) PROVIDER_ALLOC(sizeof(DBID) * m_cColumnIDs);
	m_rgColumns = (DBORDINAL *) PROVIDER_ALLOC(sizeof(DBORDINAL) * m_cColumnIDs);
	
	TESTC(m_rgColumnIDs != NULL);
	TESTC(m_rgColumns != NULL);

	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		m_rgColumns[ulIndex] = 55;

		m_rgColumnIDs[ulIndex].uGuid.guid = GUID_NULL;
		m_rgColumnIDs[ulIndex].eKind = 1000;
		m_rgColumnIDs[ulIndex].uName.ulPropid = 1000;
	}

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns),DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: One ColumnID is invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_4()
{
	TBEGIN;
	HRESULT Exphr	= S_OK;
	IID		iid		= IID_IRowset;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,TRUE),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( !ulIndex )
		{
			// Check to see if there is only one column
			Exphr = DB_E_ERRORSOCCURRED;

			m_rgColumnIDs[ulIndex].uGuid.guid = GUID_NULL;
			m_rgColumnIDs[ulIndex].eKind = 1000;
			m_rgColumnIDs[ulIndex].uName.ulPropid = 1000;
			continue;
		}

		// Check to see if there is more than one column
		Exphr = DB_S_ERRORSOCCURRED;
	}

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns),Exphr);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Count of ColumnIDs is 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_5()
{
	TBEGIN;

	IID iid	= IID_IRowset;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(FALSE,TRUE,TRUE),S_OK);

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Valid count and valid range of ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_6()
{
	TBEGIN;

	IID	iid	= IID_IRowset;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,TRUE),S_OK);

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for DBPROP_BOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_7()
{
	TBEGIN;

	IID	iid	= IID_IRowset;

	if( !SupportedProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize) )
	{												
		odtLog << L"Bookmarks are not supported.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	// Set Property
	SetRowsetProperties(DBPROPSET_ROWSET,DBPROP_BOOKMARKS);

	if( FAILED(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset)) )
	{
		TESTC_(DB_E_ERRORSOCCURRED, m_hr);
		TESTC(!m_pIRowset);
		TESTC(m_rgPropSets->rgProperties->dwStatus == DBPROPSTATUS_NOTSETTABLE);
		goto CLEANUP;
	}

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(FALSE,TRUE,TRUE),S_OK);

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns),S_OK);

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: MapColumnIDs on optional columns in IColumnsRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_8()
{
	TBEGIN;
	DBORDINAL		ulOptColOrd		= 99;
	DBORDINAL		cOptColumns		= 0;
	DBID*			rgOptColumns	= NULL;
	IColumnsRowset*	pIColumnsRowset	= NULL;
	IRowset*		pIRowsetColumns	= NULL;
	IID				iid				= IID_IRowset;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	// Check to see if IColumnsRowset is supported
	TESTC_PROVIDER(VerifyInterface(m_pIRowset, IID_IColumnsRowset, 
						ROWSET_INTERFACE, (IUnknown **)&pIColumnsRowset));

	TESTC_(pIColumnsRowset->GetAvailableColumns(&cOptColumns,
													&rgOptColumns),S_OK);

	// Check for an optional column
	TESTC_PROVIDER(cOptColumns && rgOptColumns);

	// Pass first optional column
	TESTC_(pIColumnsRowset->GetColumnsRowset(NULL, 1, rgOptColumns,
					IID_IRowset, 0, NULL, (IUnknown**)&pIRowsetColumns),S_OK);

	// Get a IID_IColumnsInfo pointer
	TESTC(VerifyInterface(pIRowsetColumns, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
									&m_rgInfo, &m_pStringsBuffer), S_OK);

	TESTC_(m_pIColumnsInfo->MapColumnIDs(1, rgOptColumns, 
											&ulOptColOrd),S_OK);
	
	TESTC(m_rgInfo[m_cColumns-1].iOrdinal == ulOptColOrd);
	
CLEANUP:
	
	PROVIDER_FREE(rgOptColumns);

	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pIRowsetColumns);

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Change the DBKIND in the ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_9()
{
	TBEGIN;
	IID	  iid     = IID_IRowset;
	DBORDINAL ulIndex = 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,TRUE),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( (m_rgColumnIDs[ulIndex].eKind == DBKIND_NAME) ||
			(m_rgColumnIDs[ulIndex].eKind == DBKIND_GUID_NAME) ||
			(m_rgColumnIDs[ulIndex].eKind == DBKIND_PGUID_NAME) )
		{
			m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
			m_rgColumnIDs[ulIndex].uName.ulPropid = 1;
		}
		else
		{
			m_rgColumnIDs[ulIndex].eKind = DBKIND_NAME;
			m_rgColumnIDs[ulIndex].uName.pwszName = L"Bogus";
		}
	}

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns), DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Ask for Column 0 with DBKIND_PROPID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_10()
{
	TBEGIN;
	IID		iid		= IID_IRowset;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,TRUE),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
		m_rgColumnIDs[ulIndex].uName.ulPropid = 0;
	}

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns), DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_11()
{
	TBEGIN;
	IID		iid		= IID_IRowset;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,TRUE),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		switch(m_rgColumnIDs[ulIndex].eKind)
		{
			case DBKIND_NAME:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_NAME;
				break;
			case DBKIND_GUID_NAME:
			case DBKIND_PGUID_NAME:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_NAME;
				break;
			case DBKIND_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_PROPID;
				break;
			case DBKIND_GUID_PROPID:
				if(m_rgColumnIDs[ulIndex].uGuid.guid == DBCOL_SPECIALCOL)
					m_rgColumnIDs[ulIndex].uName.ulPropid = 0;

				m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
				break;
			case DBKIND_PGUID_PROPID:
					m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
				break;
			case DBKIND_GUID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_PROPID;
				break;
			default:
				assert(FALSE);
		}
	}

	// run testing interface, validate params
	CHECKW(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns), DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr, FALSE));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Change the GUID in the DBID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_12()
{
	TBEGIN;
	IID		iid		= IID_IRowset;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,TRUE),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		switch(m_rgColumnIDs[ulIndex].eKind)
		{
			case DBKIND_PGUID_NAME:
			case DBKIND_PGUID_PROPID:
				*m_rgColumnIDs[ulIndex].uGuid.pguid = DBPROPSET_ROWSET;
				break;

			case DBKIND_NAME:
			case DBKIND_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID;
			case DBKIND_GUID:
			case DBKIND_GUID_NAME:
			case DBKIND_GUID_PROPID:
				m_rgColumnIDs[ulIndex].uGuid.guid = DBPROPSET_ROWSET;
				break;
		}
	}

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns), DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Switch the DBKIND in the ColumnIDs with Bookmarks
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_13()
{
	TBEGIN;
	IID		iid		= IID_IRowset;
	DBORDINAL	ulIndex	= 0;

	// Set Property
	if( !SettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIDBInitialize) )
	{												
		odtLog << L"Bookmarks are not settable.\n";
		return TEST_SKIPPED;
	}

	// Initialize the variation
	INIT;

	// Set Property
	SetRowsetProperties(DBPROPSET_ROWSET,DBPROP_BOOKMARKS);

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,TRUE),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		switch(m_rgColumnIDs[ulIndex].eKind)
		{
			case DBKIND_NAME:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_NAME;
				break;
			case DBKIND_GUID_NAME:
			case DBKIND_PGUID_NAME:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_NAME;
				break;
			case DBKIND_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_PROPID;
				break;
			case DBKIND_GUID_PROPID:
			case DBKIND_PGUID_PROPID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
				break;
			case DBKIND_GUID:
				m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_PROPID;
				break;
			default:
				assert(FALSE);
		}
	}

	// run testing interface, validate params
	CHECKW(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns), DB_E_ERRORSOCCURRED);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr, FALSE));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: ColumnID one greater than the last ordinal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_14()
{
	TBEGIN;
	HRESULT Exphr	= S_OK;
	IID		iid		= IID_IRowset;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,TRUE),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if (!ulIndex) 
		{ 
			// Check to see if there is only one column
			Exphr = DB_E_ERRORSOCCURRED;

			if( m_rgColumnIDs[ulIndex].eKind != DBKIND_PROPID &&
				m_rgColumnIDs[ulIndex].eKind != DBKIND_GUID_PROPID &&
				m_rgColumnIDs[ulIndex].eKind != DBKIND_PGUID_PROPID )
				m_rgColumnIDs[ulIndex].eKind = DBKIND_PROPID;
			
			m_rgColumnIDs[ulIndex].uName.ulPropid = ULONG(m_cColumnIDs+2);
			continue;
		}

		// Check to see if there is more than one column
		Exphr = DB_S_ERRORSOCCURRED;
	}

	// run testing interface, validate params
	TESTC_(m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
											m_rgColumnIDs, m_rgColumns), Exphr);
	
	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: Ask for Column 0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MapColID_OpenRowset::Variation_15()
{
	TBEGIN;
	IID		iid		= IID_IRowset;
	DBORDINAL	ulIndex	= 0;

	// Initialize the variation
	INIT;

	TESTC_(m_hr=m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(),
			NULL, iid, m_cPropSets, m_rgPropSets, (IUnknown **)&m_pIRowset),S_OK);

	TESTC(m_pIRowset != NULL);

	TESTC(VerifyInterface(m_pIRowset, IID_IColumnsInfo, 
						ROWSET_INTERFACE, (IUnknown **)&m_pIColumnsInfo));

	// Get the Column DBIDs
	TESTC_(m_hr=m_pIColumnsInfo->GetColumnInfo(&m_cColumns, &m_rgInfo,
													&m_pStringsBuffer),S_OK);

	// Make array of DBIDs 
	TESTC_(m_hr=MakeDBIDArrays(TRUE,TRUE,TRUE),S_OK);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if( m_rgColumnIDs[ulIndex].eKind != DBKIND_PROPID &&
			m_rgColumnIDs[ulIndex].eKind != DBKIND_GUID_PROPID &&
			m_rgColumnIDs[ulIndex].eKind != DBKIND_PGUID_PROPID )
			m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID_PROPID;
		
		m_rgColumnIDs[ulIndex].uName.ulPropid = 0;
	}

	// run testing interface, validate params
	m_hr=m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, m_rgColumnIDs, m_rgColumns);

	// May or may not find the bookmark column
	if( GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, m_pIRowset) ) {
		TEST2C_(m_hr, DB_E_ERRORSOCCURRED, DB_S_ERRORSOCCURRED);
	}
	else {
		TESTC_(m_hr, DB_E_ERRORSOCCURRED);
	}

	// Validate the columns
	QTESTC(CheckOrdinal(m_hr));

CLEANUP:

	FREE;
	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MapColID_OpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
}	// }}
// }}
// }}



// {{ TCW_TC_PROTOTYPE(MapColIDRow_Rowset)
//*-----------------------------------------------------------------------
//| Test Case:		MapColIDRow_Rowset - MapColumnIDs on a Row object
//| Created:  	10/12/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MapColIDRow_Rowset::Init()
{ 
	EINTERFACE	eIntf = ROWSET_INTERFACE;

	if(m_eTestCase == TC_Cmd)
		eIntf = COMMAND_INTERFACE;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(IColInfo::Init(eIntf))
	// }}
	{ 
		m_Method = MAPCOLID;
		switch(m_eTestCase)
		{
		case TC_Rowset:
			return GetRowFromRowset(FALSE);
		case TC_Cmd:
			return GetRowFromCommand(FALSE);
		case TC_OpenRW:
			return GetRowFromOpenRW(FALSE);
		case TC_Bind:
			return GetRowFromBind(FALSE);
		case TC_IColInfo2:
			return GetRowFromRowset(TRUE);
		default:
			ASSERT(!L"Unhandled Type...");
		}
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: cColumnIDs = 0
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int MapColIDRow_Rowset::Variation_1()
{ 
	TBEGIN

	m_cColumnIDs = 0;
	SAFE_ALLOC(m_rgColumnIDs, DBID, 1);
	SAFE_ALLOC(m_rgColumns, DBORDINAL, 1);

	TESTC_(m_hr=CallMethodOnRowObj(FALSE), S_OK)

	TESTC(m_rgColumnIDs != NULL)
	TESTC(m_rgColumns != NULL)

CLEANUP:
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: All columns returned by GetColInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int MapColIDRow_Rowset::Variation_2()
{ 
	TBEGIN

	TESTC_(m_hr=CallMethodOnRowObj(FALSE, GETCOLINFO), S_OK)

	m_cColumns = m_cColumns2;
	m_rgInfo = m_rgInfo2;

	TESTC_(MakeDBIDArrays(TRUE,TRUE,TRUE), S_OK)

	TESTC_(m_hr=CallMethodOnRowObj(FALSE), S_OK)

	QTESTC(CheckOrdinal(m_hr))

CLEANUP:
	CLEAR
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Subset of columns returned by GetColInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int MapColIDRow_Rowset::Variation_3()
{ 
	TBEGIN

	TESTC_(m_hr=CallMethodOnRowObj(FALSE, GETCOLINFO), S_OK)

	TESTC_PROVIDER(m_cColumns2>2)
	m_cColumns = m_cColumns2 - 2;
	m_rgInfo = &(m_rgInfo2[2]);

	TESTC_(MakeDBIDArrays(TRUE,TRUE,TRUE), S_OK)

	TESTC_(m_hr=CallMethodOnRowObj(FALSE), S_OK)

	QTESTC(CheckOrdinal(m_hr))

	TESTC(m_rgColumns[0] == m_rgInfo2[2].iOrdinal)

CLEANUP:
	CLEAR
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Columns out of order
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int MapColIDRow_Rowset::Variation_4()
{ 
	TBEGIN
	DBORDINAL ulIndex = 0;

	TESTC_(m_hr=CallMethodOnRowObj(FALSE, GETCOLINFO), S_OK)

	TESTC_PROVIDER(m_cColumns2>2)
	m_cColumns = m_cColumns2;
	m_rgInfo = m_rgInfo2;

	m_cColumnIDs = 2;

	// Allocate memory for the array of DBID's
	SAFE_ALLOC(m_rgColumnIDs, DBID, sizeof(DBID) * m_cColumnIDs);

	// Copy DBIDs using DuplicateDBID, not memcpy
	DuplicateDBID(m_rgInfo2[2].columnid, &(m_rgColumnIDs[0]));
	DuplicateDBID(m_rgInfo2[0].columnid, &(m_rgColumnIDs[1]));

	// Allocate memory for the array of DBID's
	SAFE_ALLOC(m_rgColumns, DBORDINAL, sizeof(DBORDINAL) * m_cColumnIDs);

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = DB_INVALIDCOLUMN;

	TESTC_(m_hr=CallMethodOnRowObj(FALSE), S_OK)

	QTESTC(CheckOrdinal(m_hr))

	TESTC(m_rgColumns[0] == m_rgInfo2[2].iOrdinal)
	TESTC(m_rgColumns[1] == m_rgInfo2[0].iOrdinal)

CLEANUP:
	CLEAR
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED: One ColumnID is invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int MapColIDRow_Rowset::Variation_5()
{ 
	TBEGIN
	DBORDINAL	ulIndex;

	TESTC_(m_hr=CallMethodOnRowObj(FALSE, GETCOLINFO), S_OK)

	m_cColumns = m_cColumns2;
	m_rgInfo = m_rgInfo2;

	TESTC_(MakeDBIDArrays(TRUE,TRUE,TRUE), S_OK)

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		if (!ulIndex) 
		{
			m_rgColumnIDs[ulIndex].uGuid.guid = GUID_NULL;
			m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID + 100;
			m_rgColumnIDs[ulIndex].uName.ulPropid = 1000;
		}
	}

	TEST2C_(m_hr=CallMethodOnRowObj(FALSE), DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED)

	// Check to see if there is only one column
	if (m_cColumnIDs == 1)
		TESTC_(m_hr, DB_E_ERRORSOCCURRED);

	QTESTC(CheckOrdinal(m_hr))

CLEANUP:
	CLEAR
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED: All ColumnIDs are invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int MapColIDRow_Rowset::Variation_6()
{ 
	TBEGIN
	DBORDINAL	ulIndex;

	TESTC_(m_hr=CallMethodOnRowObj(FALSE, GETCOLINFO), S_OK)

	m_cColumns = m_cColumns2;
	m_rgInfo = m_rgInfo2;

	TESTC_(MakeDBIDArrays(TRUE,TRUE,TRUE), S_OK)

	// Copy values into members
	for(ulIndex=0; ulIndex<m_cColumnIDs; ulIndex++)
	{
		m_rgColumnIDs[ulIndex].uGuid.guid = GUID_NULL;
		m_rgColumnIDs[ulIndex].eKind = DBKIND_GUID + 100;
		m_rgColumnIDs[ulIndex].uName.ulPropid = 1000;
	}

	TESTC_(m_hr=CallMethodOnRowObj(FALSE), DB_E_ERRORSOCCURRED)

	QTESTC(CheckOrdinal(m_hr))

CLEANUP:
	CLEAR
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cColumnIDs=1, rgColumnIDs=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int MapColIDRow_Rowset::Variation_7()
{ 
	TBEGIN

	if(m_pIColumnsInfo)
		TESTC_(m_pIColumnsInfo->MapColumnIDs(1,NULL,m_rgColumns), E_INVALIDARG)
	else
		TESTC_(m_pIColumnsInfo2->MapColumnIDs(1,NULL,m_rgColumns), E_INVALIDARG)

CLEANUP:
	CLEAR
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: rgColumns is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int MapColIDRow_Rowset::Variation_8()
{ 
	TBEGIN

	TESTC_(m_hr=CallMethodOnRowObj(FALSE, GETCOLINFO), S_OK)

	m_cColumns = m_cColumns2;
	m_rgInfo = m_rgInfo2;

	TESTC_(MakeDBIDArrays(TRUE,TRUE,TRUE), S_OK)

	if(m_pIColumnsInfo)
		TESTC_(m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs,m_rgColumnIDs,NULL), E_INVALIDARG)
	else
		TESTC_(m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs,m_rgColumnIDs,NULL), E_INVALIDARG)

CLEANUP:
	CLEAR
	Free(FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL MapColIDRow_Rowset::Terminate()
{ 
	SAFE_RELEASE(m_pIUnknown);
	FREE;

// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(ZombieClassCmd)
//*-----------------------------------------------------------------------
//| Test Case:		ZombieClassCmd - zombie on command
//|	Created:		09/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ZombieClassCmd::Init()
{
	BOOL fReturn = FALSE;

	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	m_pIDBCreateCommand = INVALID(IDBCreateCommand*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(Zombie::Init())
	// }}
	{
		//This is a mandatory interface, it should always succeed
		if(RegisterInterface(COMMAND_INTERFACE, IID_IColumnsInfo, 0, NULL))
			return TRUE;
	}

	// Check to see if ITransaction or Commands are supported
    if( (!m_pITransactionLocal) || (!m_pIDBCreateCommand) )
		fReturn = TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pIDBCreateCommand == INVALID(IDBCreateCommand*))
		m_pIDBCreateCommand = NULL;

	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;

	return fReturn;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassCmd::Variation_1()
{
	return TestTxnCmd(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassCmd::Variation_2()
{
	return TestTxnCmd(ETXN_ABORT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassCmd::Variation_3()
{
	return TestTxnCmd(ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassCmd::Variation_4()
{
	return TestTxnCmd(ETXN_COMMIT, FALSE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ZombieClassCmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Zombie::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ZombieClassRowset)
//*-----------------------------------------------------------------------
//| Test Case:		ZombieClassRowset - zombie on a rowset
//|	Created:		09/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ZombieClassRowset::Init()
{
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(Zombie::Init())
	// }}
	{
		//This is a mandatory interface, it should always succeed
		if(RegisterInterface(ROWSET_INTERFACE, IID_IColumnsInfo, 0, NULL))
			return TRUE;
	}

	// Check to see if ITransaction is supported
    if(!m_pITransactionLocal)
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassRowset::Variation_1()
{
	return TestTxnRowset(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassRowset::Variation_2()
{
	return TestTxnRowset(ETXN_ABORT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassRowset::Variation_3()
{
	return TestTxnRowset(ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassRowset::Variation_4()
{
	return TestTxnRowset(ETXN_COMMIT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to TRUE for Multiple Rowsets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassRowset::Variation_5()
{
	return TestTxnRowset(ETXN_ABORT, TRUE, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to FALSE for Multiple Rowsets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassRowset::Variation_6()
{
	return TestTxnRowset(ETXN_ABORT, FALSE, TRUE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to TRUE for Multiple Rowsets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassRowset::Variation_7()
{
	return TestTxnRowset(ETXN_COMMIT, TRUE, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to FALSE for Multiple Rowsets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ZombieClassRowset::Variation_8()
{
	return TestTxnRowset(ETXN_COMMIT, FALSE, TRUE);
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ZombieClassRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(Zombie::Terminate());
}	// }}
// }}
// }}



// {{ TCW_TC_PROTOTYPE(ZombieClassRow)
//*-----------------------------------------------------------------------
//| Test Case:		ZombieClassRow - zombie on a row
//| Created:  	1/28/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ZombieClassRow::Init()
{ 
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(Zombie::Init())
	// }}
	{
		//This is a mandatory interface, it should always succeed
		if(RegisterInterface(ROW_INTERFACE, IID_IColumnsInfo, 0, NULL))
			return TRUE;
		
		//Check to see if the interface is supported (IGetRow off the Rowset)
		if( m_hr == E_NOINTERFACE )
			return TEST_SKIPPED;
	}

	// Check to see if ITransaction is supported
    if(!m_pITransactionLocal)
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;

	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ZombieClassRow::Variation_1()
{ 
	return TestTxnRow(ETXN_ABORT, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Abort with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ZombieClassRow::Variation_2()
{ 
	return TestTxnRow(ETXN_ABORT, FALSE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ZombieClassRow::Variation_3()
{ 
	return TestTxnRow(ETXN_COMMIT, TRUE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Commit with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int ZombieClassRow::Variation_4()
{ 
	return TestTxnRow(ETXN_COMMIT, FALSE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL ZombieClassRow::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(Zombie::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(ExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		ExtendedErrors - extended error tests
//|	Created:		09/20/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ExtendedErrors::Init()
{
	BOOL fResult = TEST_SKIPPED;
	ISupportErrorInfo *pISupportErrorInfo = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(IColInfo::Init(COMMAND_INTERFACE))
	// }}
	{
		INIT;

		if(!m_pICommand)
			goto CLEANUP;

		if((m_pICommand->QueryInterface(IID_ISupportErrorInfo, (void **)&pISupportErrorInfo))== E_NOINTERFACE)
		{
			odtLog<<L"ISupportErrorInfo is not supported."<<ENDL;
			goto CLEANUP;
		}
		
		fResult = TRUE;
	}
	else
		return FALSE;
CLEANUP:
	
	SAFE_RELEASE(pISupportErrorInfo);

	FREE;
	return fResult;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IColumnsInfo calls on the Command with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_1()
{
	BOOL	fResult = TEST_FAIL;
	HRESULT hr		= E_FAIL;

	// Initialize the variation
	INIT;
	m_Method = GETCOLINFO;

	if (!CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_COLLISTFROMTBL, NULL, PREPARE, 1), S_OK))
		goto CLEANUP;
	
	// Free the memory
	FreeColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer);
		
	// Cause an error
	m_pExtError->CauseError();

	// Do extended check following GetColumnInfo
	if(CHECK(hr = m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
										&m_rgInfo, &m_pStringsBuffer), S_OK))
		fResult = XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);

	// Validate the columns
	CheckEachColumn(hr);
	FREE;
	
	// Initialize the variation
	INIT;
	m_Method = MAPCOLID;

	if (!CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK))
		goto CLEANUP;

	// Cause an error
	m_pExtError->CauseError();

	// Do extended check following MapColumnIDs
	if(CHECK(hr = m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
								m_rgColumnIDs, m_rgColumns), S_OK))
		fResult &= XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);	
	
	// Validate the columns
	QTESTC(CheckOrdinal(hr));

CLEANUP:
	
	FREE;

	if (fResult)
		return TEST_PASS;
	else
		return TEST_FAIL; 
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid IColumnsInfo calls on the Command with previous eror object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_2()
{
	BOOL	fResult = TEST_FAIL;
	HRESULT hr		= E_FAIL;
	DBORDINAL	ulIndex = 0;

	// Initialize the variation
	INIT;
	m_Method = GETCOLINFO;

	if (!CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_COLLISTFROMTBL, NULL, PREPARE, 1), S_OK))
		goto CLEANUP;
	
	// Free the memory
	FreeColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer);
		
	// Cause an error
	m_pExtError->CauseError();

	// Do extended check following GetColumnInfo
	if(CHECK(hr = m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
										&m_rgInfo, NULL), E_INVALIDARG))
		fResult = XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);

	// Validate the columns
	CheckEachColumn(hr);
	FREE;
	
	// Initialize the variation
	INIT;
	m_Method = MAPCOLID;

	if (!CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK))
		goto CLEANUP;

	// Cause an error
	m_pExtError->CauseError();

	// Reset the rgColumns array
	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	// Do extended check following MapColumnIDs
	if(CHECK(hr = m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
								m_rgColumnIDs, NULL), E_INVALIDARG))
		fResult &= XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);	
	
	// Validate the columns
	QTESTC(CheckOrdinal(hr));

CLEANUP:
	
	FREE;

	if (fResult)
		return TEST_PASS;
	else
		return TEST_FAIL; 
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid IColumnsInfo calls on the Command with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_3()
{
	BOOL	fResult = TEST_FAIL;
	HRESULT hr		= E_FAIL;
	DBORDINAL	ulIndex = 0;

	// Initialize the variation
	INIT;
	m_Method = GETCOLINFO;

	if (!CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_COLLISTFROMTBL, NULL, PREPARE, 1), S_OK))
		goto CLEANUP;
	
	// Free the memory
	FreeColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer);
		
	// Do extended check following GetColumnInfo
	if(CHECK(hr = m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
										&m_rgInfo, NULL), E_INVALIDARG))
		fResult = XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);

	// Validate the columns
	CheckEachColumn(hr);
	FREE;
	
	// Initialize the variation
	INIT;
	m_Method = MAPCOLID;

	if (!CHECK(hr=ExecuteMethod_cmd(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK))
		goto CLEANUP;

	// Reset the rgColumns array
	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	// Do extended check following MapColumnIDs
	if(CHECK(hr = m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
								m_rgColumnIDs, NULL), E_INVALIDARG))
		fResult &= XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);	
	
	// Validate the columns
	QTESTC(CheckOrdinal(hr));

CLEANUP:
	
	FREE;

	if (fResult)
		return TEST_PASS;
	else
		return TEST_FAIL; 
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Valid IColumnsInfo calls on the Rowset with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_4()
{
	BOOL	fResult = TEST_FAIL;
	HRESULT hr		= E_FAIL;

	// Initialize the variation
	INIT;
	m_Method = GETCOLINFO;

	if (!CHECK(hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_COLLISTFROMTBL, NULL, PREPARE, 1), S_OK))
		goto CLEANUP;
	
	// Free the memory
	FreeColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer);
		
	// Cause an error
	m_pExtError->CauseError();

	// Do extended check following GetColumnInfo
	if(CHECK(hr = m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
										&m_rgInfo, &m_pStringsBuffer), S_OK))
		fResult = XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);

	// Validate the columns
	CheckEachColumn(hr);
	FREE;
	
	// Initialize the variation
	INIT;
	m_Method = MAPCOLID;

	if (!CHECK(hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK))
		goto CLEANUP;

	// Cause an error
	m_pExtError->CauseError();

	// Do extended check following MapColumnIDs
	if(CHECK(hr = m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
								m_rgColumnIDs, m_rgColumns), S_OK))
		fResult &= XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);	
	
	// Validate the columns
	QTESTC(CheckOrdinal(hr));

CLEANUP:
	
	FREE;
	
	if (fResult)
		return TEST_PASS;
	else
		return TEST_FAIL; 
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Invalid IColumnsInfo calls on the Rowset with previous error object exisiting
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_5()
{
	BOOL	fResult = TEST_FAIL;
	HRESULT hr		= E_FAIL;
	DBORDINAL	ulIndex = 0;

	// Initialize the variation
	INIT;
	m_Method = GETCOLINFO;

	if (!CHECK(hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_COLLISTFROMTBL, NULL, PREPARE, 1), S_OK))
		goto CLEANUP;
	
	// Free the memory
	FreeColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer);
		
	// Cause an error
	m_pExtError->CauseError();

	// Reset the rgColumns array
	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	// Do extended check following GetColumnInfo
	if(CHECK(hr = m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
										&m_rgInfo, NULL), E_INVALIDARG))
		fResult = XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);

	// Validate the columns
	CheckEachColumn(hr);
	FREE;
	
	// Initialize the variation
	INIT;
	m_Method = MAPCOLID;

	if (!CHECK(hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK))
		goto CLEANUP;

	// Cause an error
	m_pExtError->CauseError();

	// Reset the rgColumns array
	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	// Do extended check following MapColumnIDs
	if(CHECK(hr = m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
								m_rgColumnIDs, NULL), E_INVALIDARG))
		fResult &= XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);	
	
	// Validate the columns
	QTESTC(CheckOrdinal(hr));

CLEANUP:
	
	FREE;

	if (fResult)
		return TEST_PASS;
	else
		return TEST_FAIL; 
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid IColumnsInfo calls on the Rowset with no previous error object exisiting
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_6()
{
	BOOL	fResult = TEST_FAIL;
	HRESULT hr		= E_FAIL;
	DBORDINAL	ulIndex = 0;

	// Initialize the variation
	INIT;
	m_Method = GETCOLINFO;

	if (!CHECK(hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT, 
								SELECT_COLLISTFROMTBL, NULL, PREPARE, 1), S_OK))
		goto CLEANUP;
	
	// Free the memory
	FreeColumnInfo(&m_cColumns, &m_rgInfo, &m_pStringsBuffer);
		
	// Do extended check following GetColumnInfo
	if(CHECK(hr = m_pIColumnsInfo->GetColumnInfo(&m_cColumns, 
										&m_rgInfo, NULL), E_INVALIDARG))
		fResult = XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);

	// Validate the columns
	CheckEachColumn(hr);
	FREE;
	
	// Initialize the variation
	INIT;
	m_Method = MAPCOLID;

	if (!CHECK(hr=ExecuteMethod_row(TRUE, TRUE, TRUE, eSELECT,
						SELECT_COLLISTFROMTBL, NULL, PREPARE),S_OK))
		goto CLEANUP;

	// Reset the rgColumns array
	for(ulIndex=0; ulIndex < m_cColumnIDs; ulIndex++)
		m_rgColumns[ulIndex] = INVALID(DBORDINAL);

	// Do extended check following MapColumnIDs
	if(CHECK(hr = m_pIColumnsInfo->MapColumnIDs(m_cColumnIDs, 
								m_rgColumnIDs, NULL), E_INVALIDARG))
		fResult &= XCHECK(m_pIColumnsInfo, IID_IColumnsInfo, hr);	
	
	// Validate the columns
	QTESTC(CheckOrdinal(hr));

CLEANUP:
	
	FREE;

	if (fResult)
		return TEST_PASS;
	else
		return TEST_FAIL; 
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ExtendedErrors::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(IColInfo::Terminate());
}	// }}
// }}
// }}

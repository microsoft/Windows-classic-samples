//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module icommand.CPP | icommand source file
//
//--------------------------------------------------------------------


#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID

#include "modstandard.hpp"
#include "icommand.h"
#include "msdasql.h" // for kagera specific properties
#include "extralib.h"
#include <time.h>

enum ETXN	{ETXN_COMMIT, ETXN_ABORT};

CTable * g_Table2=NULL;
CTable * g_Table3=NULL;


#define CLEANUP(x) if(x) goto CLEANUP;
#define TERMINATE(x) if(x) return FALSE;
#define CONTINUE(x) if(x) return TRUE;

#define	PI	(IUnknown *)
#define PPI (IUnknown **)
#define PPPI (IUnknown ***)
#define WC (WCHAR *)

#define TESTBEGIN BOOL TESTB = TEST_FAIL;
#define TPASS TESTB = TEST_PASS;
#define	TSKIPPED TESTB = TEST_SKIPPED;goto CLEANUP;

enum {THRD_ONE=0, THRD_TWO, THRD_THREE, THRD_FOUR}; 
enum {ONE_THRD=1,TWO_THRDS,THREE_THRDS, FOUR_THRDS};

//This simply the usage of threads down to 3 lines of code, CREATE/START/END
#define INIT_THRDS(cTh)  const ULONG cThreads = cTh; HANDLE rghThread[cThreads]; DWORD rgThreadID[cThreads]

//#define CREATE_THRDS(pFunc,pArgs)         CreateThreads(pFunc,pArgs,cThreads,rghThread,rgThreadID)
#define CREATE_THRD(iThread,pFunc,pArg)   CreateThreads(pFunc,pArg,ONE_THRD,&rghThread[iThread],&rgThreadID[iThread])

#define START_THRDS()                     ScreenLogging(FALSE); StartThreads(cThreads,rghThread)       
//#define START_THRD(iThread)               StartThread(&rghThread[iThread])       

#define END_THRDS()                       EndThreads(cThreads,rghThread);ScreenLogging(TRUE);
//#define END_THRD(iThread)                 EndThread(&rghThread[iThread])           

#define GET_THRDCODE(iThread)				GetThreadCode(&rghThread[iThread])

#define THRD_FUNC ((THRDARG*)pv)->pFunc
#define THRD_ARG1 ((THRDARG*)pv)->pArg1
#define THRD_ARG2 ((THRDARG*)pv)->pArg2
#define THRD_ARG4 ((THRDARG*)pv)->pArg4
#define THRD_ARG3 ((THRDARG*)pv)->pArg3
#define THRD_ARG5 ((THRDARG*)pv)->pArg5

struct THRDARG 
{
	LPVOID pFunc;
	LPVOID pArg1; 
	LPVOID pArg2;
	LPVOID pArg3;
	LPVOID pArg4;
	LPVOID pArg5;
};

#define ThreadSwitch() Sleep(0) 
					
//CQTEST Macros, similar to TEST except they are "QUITE" no error messages
#define CQTEST_(exp,hr)   { HRESULT acthr; if(!CQCHECK(acthr=exp,hr)) { HRTRACE(acthr,hr); TRETURN } } 
#define CQTESTC_(exp,hr)  { HRESULT acthr; if(!CQCHECK(acthr=exp,hr)) { HRTRACE(acthr,hr); goto CLEANUP; } }
#define CQTEST(exp)	     { CQTEST_(exp,TRUE)  }
#define CQTESTC(exp)	     { CQTESTC_(exp,TRUE) }

#define CQCHECK(exp,hr) (TESTB = ((exp)==(hr)))
#define CQCOMPARE(x,y)  (TESTB = ((x)==(y)))

#define HRTRACE(acthr,hr) PRVTRACE(L"***HRTRACE Actual: %x, Expected: %x, File: %s, Line: %d\n",ResultFromScode(acthr),ResultFromScode(hr),LONGSTRING(__FILE__),__LINE__); 

// @todo Replace other macros with these ones.
#define TEST_CHECK(exp, hres)	{ if (!CHECK ((exp), (hres))) { goto CLEANUP; } }
#define TEST_COMPARE(exp, val)	{ if (!COMPARE ((exp), (val))) { goto CLEANUP; } }
#define TEST_SUPPORTED(exp, hres){ TESTB = TEST_PASS; if((exp) == DB_E_NOTSUPPORTED) { TOUTPUT_LINE(L"NotSupported by Provider, skipping Variation"); TESTB = TEST_SKIPPED; goto CLEANUP;} else if (!CHECK ((exp), (hres))) {TESTB = TEST_FAIL; goto CLEANUP;}}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x2fd3c861, 0x9156, 0x11cf, { 0xaa, 0x86, 0x00, 0xa0, 0xc9, 0x05, 0x41, 0xce }};
DECLARE_MODULE_NAME("ICommand");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test module for ICommand methods");
DECLARE_MODULE_VERSION(829867051);
// TCW_WizardVersion(2)
// TCW_Automation(False)
// }} TCW_MODULE_GLOBALS_END

//---------------------------------------------------------------------------------------------------
// @func ScreenLogging
//			Turns the screen logging to litmus window off.
//---------------------------------------------------------------------------------------------------
inline void
ScreenLogging(BOOL fFlag)
{
	odtLog.ScreenLogging(fFlag);
}

//---------------------------------------------------------------------------------------------------
// @Global
//---------------------------------------------------------------------------------------------------

//Check whether the provider is read only.
BOOL	g_fReadOnlyProvider;
BOOL	g_fKagera;
BOOL	g_fLuxor;
BOOL	g_fSqlServer;

CRITICAL_SECTION g_csExtendedError;

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	BOOL fSuccess=FALSE;
	HRESULT	hr=E_FAIL;
	IDBCreateCommand *pIDBCreateCommand=NULL;
	CTable * pTable = NULL;
	DBCOUNTITEM iRow = 0;

	InitializeCriticalSection(&g_csExtendedError);

	//Get connection and session objects
	if(ModuleCreateDBSession(pThisTestModule))
	{
		LPWSTR				wszProviderName=NULL;

		// Initialize globals
		g_fKagera = FALSE;
		g_fLuxor = FALSE;
		g_fSqlServer = FALSE;

		// Check whether provider is read only
		g_fReadOnlyProvider = IsProviderReadOnly((IUnknown *)pThisTestModule->m_pIUnknown2);
		
		// Fail gracefully and quit module if we don't support Commands
		if (SUCCEEDED(hr = pThisTestModule->m_pIUnknown2->QueryInterface(
			IID_IDBCreateCommand, (void **)&pIDBCreateCommand)))
			pIDBCreateCommand->Release();
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
			odtLog << L"WARNING: Test does not support using fixed table from ini file.\n";

			// Read only providers must use ini file, so don't reset
			if (!g_fReadOnlyProvider)
			{
				odtLog << L"\tResetting to ignore ini file and use tables created by test.\n";
				GetModInfo()->ResetIniFile();
			}
		}

		// This test doesn't support using INSERT_ROWSETCHANGE because it needs the command text back
		if (GetModInfo()->GetInsert() == INSERT_ROWSETCHANGE)
		{
			odtLog << L"WARNING: Test does not support using INSERT_ROWSETCHANGE\n";

			// Read only providers must use ini file, so don't reset
			if (!g_fReadOnlyProvider)
			{
				odtLog << L"\tResetting to INSERT_COMMAND\n";
				GetModInfo()->SetInsert(INSERT_COMMAND);
			}
		}

		// Get the name of the provider
		GetProperty(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO, pThisTestModule->m_pIUnknown, &wszProviderName);
		if (wszProviderName && !wcscmp((LPWSTR)wszProviderName, L"MSDASQL.DLL"))
			g_fKagera=TRUE;
		if (wszProviderName && !wcscmp((LPWSTR)wszProviderName, L"sqloledb.dll"))
			g_fLuxor=TRUE;
		PROVIDER_FREE(wszProviderName);

		GetProperty(DBPROP_DBMSNAME, DBPROPSET_DATASOURCEINFO, pThisTestModule->m_pIUnknown, &wszProviderName);
		if (wszProviderName && !wcscmp((LPWSTR)wszProviderName, L"Microsoft SQL Server"))
			g_fSqlServer = TRUE;
		PROVIDER_FREE(wszProviderName);

		//If the provider is not read only,
		//create a table we'll use for the whole test module,
		//store it in pVoid for now
		pThisTestModule->m_pVoid = new CTable((IUnknown *)pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
		pTable = (CTable *)pThisTestModule->m_pVoid;

		g_Table2 = new CTable((IUnknown *)pThisTestModule->m_pIUnknown2,(LPWSTR)gwszModuleName);
		g_Table3 = new CTable((IUnknown *)pThisTestModule->m_pIUnknown2,(LPWSTR)gwszModuleName);

		if (!pTable || !g_Table2 || !g_Table3)
		{
			odtLog << wszMemoryAllocationError;
			goto CLEANUP;
		}

		// Hack for Kagera.  We have to force the insert to use commands otherwise we
		// end up getting into QBU mode which can't insert BLOB columns.
		if (g_fKagera && !GetModInfo()->GetInsert())
			GetModInfo()->SetInsert(INSERT_COMMAND);

		//Start with a table with 15 rows and no index
		TESTC_(pTable->CreateTable(0, 0), S_OK);

		// Lie to the CTable object and tell it we've got an index on column 1
		// This will force data to be made in sequential order so an order by 
		// clause will sort the data properly for verification.
		pTable->SetIndexColumn(1);

		// Insert MAX_ROWS rows into the table if no rows exist already
		if (!pTable->GetRowsOnCTable())
		{
			for (iRow=pTable->GetNextRowNumber(); iRow <= MAX_ROWS; iRow++)
			{
				// Force insert 
				TESTC_(pTable->Insert(iRow), S_OK);
			}
		}

		// Don't use autoincrement column for index column

		TESTC_(g_Table2->CreateTable(15), S_OK);

		fSuccess = TRUE;
	}

CLEANUP:
	
	// No need to cleanup as ModuleTerminate cleans them up.
	return fSuccess;
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
	DeleteCriticalSection(&g_csExtendedError);

	//If we created the table before,
	//we still own the table since all of our testcases
	//have only used it and not deleted it.
	if (pThisTestModule->m_pVoid)
	{
		((CTable *)pThisTestModule->m_pVoid)->DropTable();
		delete (CTable*)pThisTestModule->m_pVoid;
		pThisTestModule->m_pVoid = NULL;
	}

	if (g_Table2)
	{
		g_Table2->DropTable();
		delete g_Table2;
		g_Table2 = NULL;
	}

	if (g_Table3)
	{
		delete g_Table3;
		g_Table3 = NULL;
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
		case TC_Row:
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
// @class CCmd Base Class for all ICommand Testcases
class CCmd : public CSessionObject, public TCBase
{
	public:
		// @cmember Result 
		HRESULT		m_hr;

		static BOOL	m_fValidateExtended;

		// @cmember DSO pointer
		IDBInitialize * m_pMyIDBInitialize;

		// @cmember Constructor
		CCmd(LPWSTR wszTestCaseName) : CSessionObject(wszTestCaseName)
		{
			m_hr=E_FAIL;
			m_pMyIDBInitialize=NULL;
		};

		// @cmember Destructor
		~CCmd()
		{
			ASSERT(!m_pMyIDBInitialize);
		};

		// @cmember Common base class initialization
		virtual BOOL Init();

		// @cmember Common base class termination
		virtual BOOL Terminate();

		//Helper function for calling pICmd->Execute.
		HRESULT	Execute(
			ICommand*		pICmd,					//[IN]
			IUnknown **		ppRowset,				//[OUT]
			DBROWCOUNT *	pcRowsAffected=NULL,	//[OUT]
			IUnknown *		pUnkOuter=NULL,			//[IN]
			DBPARAMS *		pParams=NULL,			//[IN]
			IID				iid=IID_IDBInfo);		//[IN]

		// @cmember CreateSession
		// must Release *pIUnknown
		BOOL CreateSession( REFIID iid, IUnknown** pIUnknown);

		// @cmember Gets command ptr from session object based on IID
		BOOL GetCommand(REFIID iid,	IUnknown ** pIUnknownCommand, IUnknown * pIUnknownSession
		);
		
		// @cmember Validate Session Object
		BOOL ValidateSessionObject(IUnknown * pIUnknown);

		// @cmember Returns true if column is datetime
		BOOL IsColDateTime(DBTYPE ProviderType);

		// @cmember Returns true if columns is character
		BOOL IsColCharacter(DBTYPE ProviderType);

		// @cmember Returns true if column is numeric
		BOOL IsColNumeric(DBTYPE ProviderType);

		// @mfunc Can the data type hold Numeric with a Scale		
		BOOL IsColNumWithScale (
				DBTYPE ProviderType, // @parm [IN] Data type
				ULONG  Scale		 // @parm [IN] precision for the data type
			);

		////////////////////////
		// Multi-threading
		////////////////////////
		static ULONG WINAPI Execute(LPVOID pv);
		static ULONG WINAPI Cancel(LPVOID pv);

		// Thread routines from Thread test case (copied here)
		BOOL CreateThreads(LPTHREAD_START_ROUTINE pFunc,void* pArgs,ULONG cThreads,HANDLE* rghThread,DWORD* rgThreadID);
		BOOL StartThreads(ULONG cThreads, LPHANDLE rghThread);
		BOOL EndThreads(ULONG cThreads, LPHANDLE rghThread);

		HRESULT GetCommandTextAndPrintOut(ICommand * pICommand);
		BOOL PropertiesInError(IUnknown * pIUnknown, HRESULT hrExecute);
		void DumpCommandProps(IUnknown * pIUnknown, BOOL fPropertiesInError,
			ULONG cPropSets=0, DBPROPSET * pPropSets=NULL);

	protected:
		HRESULT SetRowsetPropertyDefault(DBPROPID DBPropID, ICommand * pICommand = NULL);
		void GetQualifierNames(
			IUnknown * pSessionIUnknown,// [in]		IUnknown off session object
			LPWSTR	pwszTableName,		// [in]		the name of the table
			LPWSTR	*ppwszCatalogName,	// [out]	catalog name
			LPWSTR	*ppwszSchemaName	// [out]	schema name
		);

		BOOL VerifyAndRemoveRows(CTable * pTable, DBCOUNTITEM ulRowNum, DBCOUNTITEM cRows);

		BOOL VerifyThreadingModel(DWORD dwCoInit);

		BOOL SetPropertyAndExecute(
			ICommand * pICmd,
			DBPROPINFOSET * pPropertyInfoSets,
			ULONG iSet,
			ULONG iProp,
			enum SET_OPTION_ENUM eSetOption,
			WCHAR * pwszDescription1,
			BOOL * pfPropFound);

		HRESULT VerifyRowset(REFIID riidRowset, IUnknown * pUnkRowset, ULONG ulStartingRow,
			DBORDINAL cRowsetCols, DBORDINAL * rgTableColOrds, BOOL fRelease);

		HRESULT VerifyRowObj(REFIID riidRow, IUnknown * pUnkRow, ULONG ulStartingRow,
			DBORDINAL cRowsetCols, DBORDINAL * rgTableColOrds, BOOL fRelease);
};


// Initialize static
BOOL	CCmd::m_fValidateExtended = FALSE;


BOOL CCmd::PropertiesInError(IUnknown * pIUnknown, HRESULT hrExecute)
{
	BOOL fPropertiesInError = FALSE;
	DBPROPIDSET rgPropertyIDSets[1];
	ICommandProperties * pICmdProp = NULL;
	ULONG cPropertySets = 0;
	DBPROPSET * pPropertySets = NULL;
	HRESULT hrGetProp = E_FAIL;

	rgPropertyIDSets[0].rgPropertyIDs = NULL;
	rgPropertyIDSets[0].cPropertyIDs = 0;
	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	TESTC(VerifyInterface(pIUnknown, IID_ICommandProperties, COMMAND_INTERFACE,
		(IUnknown **)&pICmdProp));

	TESTC_(pICmdProp->GetProperties(1, rgPropertyIDSets, &cPropertySets, &pPropertySets), S_OK);

	if (cPropertySets)
	{
		TESTC(pPropertySets != NULL);
	}
	else
	{
		TESTC(pPropertySets == NULL);
	}

	if (pPropertySets)
	{
		for (ULONG iPropSet = 0; iPropSet < cPropertySets; iPropSet++)
		{
			if (pPropertySets[iPropSet].cProperties)
				fPropertiesInError = TRUE;

			for (ULONG iProp = 0; iProp < pPropertySets[iPropSet].cProperties; iProp++)
			{
				const LPWSTR pwszTrue = L"VARIANT_TRUE";
				const LPWSTR pwszFalse = L"VARIANT_FALSE";
				const LPWSTR pwszEmpty = L"VT_EMPTY";
				const LPWSTR pwszUnexpected = L"UNEXPECTED";
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

//				COMPARE(pPropertySets[iPropSet].rgProperties[iProp].dwStatus != DBPROPSTATUS_OK, TRUE);

				if (hrExecute == DB_E_ERRORSOCCURRED)
					if (!COMPARE(pPropertySets[iPropSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_CONFLICTING))
						odtLog << L"Expected status DBPROPSTATUS_CONFLICTING, received " << 
						ppwszStatus[pPropertySets[iPropSet].rgProperties[iProp].dwStatus] << ".\n";

				if (hrExecute == DB_S_ERRORSOCCURRED)
					if (!COMPARE(pPropertySets[iPropSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_NOTSET))
						odtLog << L"Expected status DBPROPSTATUS_NOTSET, received " << 
						ppwszStatus[pPropertySets[iPropSet].rgProperties[iProp].dwStatus] << ".\n";

				if (V_VT(&pPropertySets[iPropSet].rgProperties[iProp].vValue) == VT_EMPTY)
					pwszValue = pwszEmpty;
				else if (V_BOOL(&pPropertySets[iPropSet].rgProperties[iProp].vValue) == VARIANT_TRUE)
					pwszValue = pwszTrue;
				else if (V_BOOL(&pPropertySets[iPropSet].rgProperties[iProp].vValue) == VARIANT_FALSE)
					pwszValue = pwszFalse;

				{
					DBPROPINFO * pPropInfo = NULL;

					pPropInfo = GetPropInfo(pPropertySets[iPropSet].rgProperties[iProp].dwPropertyID, pPropertySets[iPropSet].guidPropertySet,
						m_pThisTestModule->m_pIUnknown, SESSION_INTERFACE);

					if (pPropInfo && pPropInfo->pwszDescription)
					{
						odtLog << L"\t\t" << iPropSet << L" " << iProp << L" Property " << pPropInfo->pwszDescription << L" " << pwszValue;
						PROVIDER_FREE(pPropInfo->pwszDescription);
					}
					else
						odtLog << L"\t\t" << iPropSet << L" " << iProp << L" Property " << pPropertySets[iPropSet].rgProperties[iProp].dwPropertyID << L" " << pwszValue;

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
		}

	}

CLEANUP:

	if (cPropertySets && pPropertySets)
		FreeProperties(&cPropertySets, &pPropertySets);

	SAFE_RELEASE(pICmdProp);

	if (!fPropertiesInError)
		odtLog << L"No properties in error were found!\n";

	return fPropertiesInError;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Print REQUIRED TRUE command props
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CCmd::DumpCommandProps(IUnknown * pIUnknown, BOOL fPropertiesInError,
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


void CCmd::GetQualifierNames(
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

	if (pIRowset && pRow)
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

//---------------------------------------------------------------------------
//	GetCommandTextAndPrintOut
//---------------------------------------------------------------------------
HRESULT CCmd::GetCommandTextAndPrintOut(ICommand * pICommand)
{
	HRESULT hr=E_FAIL;
	ICommandText * pICommandText=NULL;
	WCHAR * sql = NULL;

	// make sure everything is alive
	if(!pICommand)
		return hr;

	if(FAILED(hr=pICommand->QueryInterface(IID_ICommandText,(void **)&pICommandText)))
		goto CLEANUP;

	if(FAILED(hr=pICommandText->GetCommandText(NULL,&sql)))
		goto CLEANUP;

	odtLog << sql << ENDL;


CLEANUP:
	
	SAFE_RELEASE(pICommandText);
	PROVIDER_FREE(sql);
	
	return hr;
}
//---------------------------------------------------------------------------
//	CCommand::IsColNumWithScale
//
//	@mfunc	BOOL				|
//			CCommand			|
//			IsColNumWithScale	|
//			Can the data type hold Numeric values with Scale?
//
//
//---------------------------------------------------------------------------
BOOL CCmd::IsColNumWithScale
(
	DBTYPE ProviderType,	// @parm [IN] provider data type
	ULONG  Scale			// @parm [IN] precision for the data type
)
{
	if (ProviderType & DBTYPE_BYREF)
		ProviderType &= ~DBTYPE_BYREF;

	switch(ProviderType)
	{
			case DBTYPE_NUMERIC:	// Numeric, Decimal
			case DBTYPE_DECIMAL:
				if( Scale )
					return TRUE;
				else
					return FALSE;
		default:
				return FALSE;	// Compiler needs this
	}
}

//--------------------------------------------------------------------
// @mfunc Base class Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCmd::Init()
{


  	if(COLEDB::Init())	
	{	
		if( m_pThisTestModule->m_pIUnknown )
		{
			m_pIDBInitialize = (IDBInitialize *)m_pThisTestModule->m_pIUnknown;
			m_pIDBInitialize->AddRef();
		}

		if(m_pThisTestModule->m_pIUnknown)
			m_pMyIDBInitialize = (IDBInitialize *)m_pThisTestModule->m_pIUnknown;
		
		SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);
		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);		

		return TRUE;
	}  
	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Base Case Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCmd::Terminate()
{
	// NULL out the Local pointer
	m_pMyIDBInitialize = NULL;

	// Release the objects
	SAFE_RELEASE(m_pIDBInitialize)
	ReleaseDBSession();

	return(COLEDB::Terminate());
}

//--------------------------------------------------------------------
// @mfunc Helper function for ICommand::Execute
//
// @rdesc HRESULT
//
HRESULT	CCmd::Execute(ICommand* pICmd,IUnknown **	ppRowset,DBROWCOUNT *	pcRowsAffected,IUnknown * pUnkOuter,DBPARAMS * pParams,IID iid)
{
	HRESULT		hr = E_FAIL;

	TESTC(pICmd != NULL)

	if(iid == IID_IDBInfo)
	{
		if(IFROWSET)
			iid = IID_IRowset;
		else
			iid = IID_IRow;
	}

	hr = pICmd->Execute(pUnkOuter, iid, pParams, pcRowsAffected, ppRowset);

	if(SUCCEEDED(hr))
	{
		if(ppRowset)
			COMPARE(*ppRowset==INVALID(IRowset*), FALSE);
	}

	if(FAILED(hr))
	{
		if(ppRowset)
			COMPARE(*ppRowset, NULL);
	}

CLEANUP:
	return hr;
}



//--------------------------------------------------------------------
// CreateThreads
//
//This simply the usage of threads down to 3 lines of code, CREATE/START/END
//--------------------------------------------------------------------
inline BOOL CCmd::CreateThreads
(
	LPTHREAD_START_ROUTINE pFunc,
	void* pArgs,
	ULONG cThreads,
	HANDLE* rghThread,
	DWORD* rgThreadID
)
{
	ASSERT(pFunc && pArgs && rghThread && rgThreadID);

	for(ULONG i=0; i<cThreads; i++)
	{
		rghThread[i] = (void*)_beginthreadex(
			NULL,
			0,
			(UINT(WINAPI*)(void*))pFunc,
			pArgs,
			CREATE_SUSPENDED,
			(UINT*)&rgThreadID[i]);
	}

// NOTE: CreateThread (WinAPI) is bad, it doesn't work well with the CrtLib,
//       Supposed to use, _beginthreadex...  CreateThread is much nicer, and doesn't
//       require all the "casting", but _begin thread actually delegates out to eventually...
//		rghThread[i] = CreateThread(NULL,0,pFunc,pArgs,CREATE_SUSPENDED,&rgThreadID[i]);

	return TEST_PASS;
}

//--------------------------------------------------------------------
// StartThreads
//
//--------------------------------------------------------------------
inline BOOL CCmd::StartThreads(ULONG cThreads, LPHANDLE rghThread)
{
	ASSERT(cThreads && rghThread);

	for(ULONG i=0; i<cThreads; i++)
		ResumeThread(rghThread[i]);
	return TEST_PASS;
}

//--------------------------------------------------------------------
// EndThreads
//
//--------------------------------------------------------------------
inline BOOL CCmd::EndThreads(ULONG cThreads, LPHANDLE rghThread)
{
	ASSERT(cThreads && rghThread);

   WaitForMultipleObjects(cThreads, rghThread, TRUE, INFINITE); 
   
   for(ULONG i=0; i<cThreads; i++) 
	   CloseHandle(rghThread[i]);

	return TEST_PASS;
}

// Hack to define COINIT value
#define COINIT_MULTITHREADED	0x0

//--------------------------------------------------------------------
// Execute
//
//--------------------------------------------------------------------
ULONG CCmd::Execute(LPVOID pv)
{
	BOOL TESTB = TEST_FAIL;
	DBROWCOUNT cRowsAffected = -111;
	HRESULT hReturnResult = E_FAIL;
	HRESULT hrCoInit = E_FAIL;

	ASSERT(pv);

	ASSERT(THRD_FUNC);

	//Thread Stack Variables
	CCmd*  pThis  = (CCmd*)THRD_FUNC;
	
	ASSERT(THRD_ARG2);
	
	// expected result
	HRESULT hr = *(HRESULT *)THRD_ARG2;

	ASSERT(THRD_ARG3);

	// Type of sql statement used.
	STATEMENTKIND eStatementKind = *(STATEMENTKIND *)THRD_ARG3;

	ASSERT(THRD_ARG4);

	// Type of sql statement used.
	HRESULT hOptionalResult = *(HRESULT *)THRD_ARG4;

	//Local Variables
	ICommand * pICommand = (ICommand *)THRD_ARG1;
	IUnknown* pUnk = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up
	
	TESTC_(hrCoInit = CoInit(COINIT_MULTITHREADED), S_OK);

	hReturnResult = pThis->Execute(pICommand, PPI &pUnk, &cRowsAffected);

	if (hReturnResult != hr &&
		hReturnResult != hOptionalResult &&
		hReturnResult != DB_S_STOPLIMITREACHED)
	{
		// Kagera returns E_FAIL here intermittently, which is a known issue that causes
		// problems with the automation runs, therefore we will just print a message so
		// we don't get a failures all the time.  See Note 3.
		if (g_fKagera && hReturnResult == E_FAIL)
			odtLog << L"Unexpected return code E_FAIL.\n";
		else if (!pThis->m_pError->Validate(hReturnResult, LONGSTRING(__FILE__), __LINE__, hr))
				goto CLEANUP;
	}

	if (m_fValidateExtended)
	{
		EnterCriticalSection(&g_csExtendedError);
		BOOL fSuccess = TRUE;
		
		// It is legal for providers to return DB_S_STOPLIMITREACHED here
//		odtLog << L"hReturnResult before ValidateExtended: " << hReturnResult << "\n";
		if (hReturnResult != DB_S_STOPLIMITREACHED)
			fSuccess = pThis->m_pExtError->ValidateExtended(hReturnResult, pICommand, IID_ICommand,
			LONGSTRING(__FILE__), __LINE__);

//		if (!fSuccess)
//			odtLog << L"hReturnResult: " << hReturnResult << "\n";
			
		pThis->m_pError->Compare(fSuccess == TRUE, LONGSTRING(__FILE__), __LINE__);

		LeaveCriticalSection(&g_csExtendedError);
	}

	if (eStatementKind == eINSERT)
	{
		// Assuming that the insert statement is just one
		if (hReturnResult == S_OK)
		{
			// Providers are allowed to return DB_COUNTUNAVAILABLE if they don't know
			if (cRowsAffected == DB_COUNTUNAVAILABLE)
			{
				// Since it is legal for providers to returen DB_COUNTUNAVAILABLE we will no longer
				// warn here.  The warning was just so we knew it was returned, in case a particular
				// provider wasn't supposed to return it.  But it causes too much noise in the tests.
				// COMPAREW(cRowsAffected, 1);
				// odtLog << L"Provider did not return cRowsAffected on insert and returned DB_COUNTUNAVAILABLE instead.\n";
			}
			else if (!pThis->m_pError->Compare(cRowsAffected==1,LONGSTRING(__FILE__), __LINE__) )
				goto CLEANUP;
		}
		else if (hReturnResult == DB_E_CANCELED)
		{
			if (! pThis->m_pError->Compare(cRowsAffected != 1, LONGSTRING(__FILE__), __LINE__) )
				goto CLEANUP;
		}
	
	}
	else if (eStatementKind == eSELECT)
	{
		if (hReturnResult == S_OK)
		{
			if (g_fKagera)
			{
				// Kagera has an intermittent bug here where they return S_OK but a NULL rowset pointer.
				// This causes all sorts of problems with automation runs, that is, failures that show
				// up sometimes but not others.  Since they have refused to fix this we will merely print
				// a message indicating the problem.  See Note 2.
				if (pUnk == NULL)
					odtLog << L"Provider returned S_OK but did not return a valid rowset pointer.\n";
			}
			else if (! pThis->m_pError->Compare((pUnk != NULL), LONGSTRING(__FILE__), __LINE__) )
				goto CLEANUP;
		}
		else if (hReturnResult == DB_E_CANCELED)
		{
			if (! pThis->m_pError->Compare((pUnk == NULL), LONGSTRING(__FILE__), __LINE__) )
				goto CLEANUP;
		}
	}
	else
	{
		// Currently not prepare to handle any other types.
		ASSERT(!L"NEED MORE CODE");
	}

	ThreadSwitch(); //Let the other thread(s) catch up

	TPASS;
CLEANUP:
	
	SAFE_RELEASE(pUnk);

	/*
	if (eStatementKind == eINSERT)
		PRVTRACE("Insert thread exited with status: %d\n", TESTB);
	else
		PRVTRACE("Select thread exited with status: %d\n", TESTB);
	*/

	if (SUCCEEDED(hrCoInit))
		CoUninitialize();
	_endthreadex(TESTB);

	TRETURN
}
//--------------------------------------------------------------------
// Cancel
//
//--------------------------------------------------------------------
ULONG CCmd::Cancel(LPVOID pv)
{
	BOOL TESTB = TEST_FAIL;
	HRESULT hReturnResult;
	HRESULT hrCoInit = E_FAIL;

	ASSERT(pv);

	//Thread Stack Variables
	ASSERT(THRD_FUNC);
	CCmd*  pThis  = (CCmd*)THRD_FUNC;

	ASSERT(THRD_ARG1);
	ICommand * pICommand = (ICommand *)THRD_ARG1;

	ASSERT(THRD_ARG2);
	HRESULT hr = *(HRESULT *)THRD_ARG2;

	ASSERT(THRD_ARG3);

	// Type of sql statement used.
	STATEMENTKIND eStatementKind = *(STATEMENTKIND *)THRD_ARG3;

	ASSERT(THRD_ARG4);

	// Type of sql statement used.
	HRESULT hOptionalResult = *(HRESULT *)THRD_ARG4;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(hrCoInit = CoInit(COINIT_MULTITHREADED), S_OK);

	// (! (pThis->m_pError->Validate(pICommand->Cancel(), 	LONGSTRING(__FILE__), __LINE__,	hr)))
	hReturnResult=pICommand->Cancel();
	
	if (! pThis->m_pError->Compare((hReturnResult == hr || hReturnResult == hOptionalResult)==TRUE, LONGSTRING(__FILE__), __LINE__) )
		goto CLEANUP;
	
	ThreadSwitch(); //Let the other thread(s) catch up
	
	TPASS
CLEANUP:

	if (SUCCEEDED(hrCoInit))
		CoUninitialize(); 
	_endthreadex(TESTB);
	TRETURN
}

//---------------------------------------------------------------------------
//	CCmd::IsColNumeric 
//
//	@mfunc	BOOL			|
//			CCmd		|
//			IsColNumeric	|
//			Can the data type hold numeric values?
//
//
//---------------------------------------------------------------------------
BOOL CCmd::IsColNumeric			
(
	DBTYPE ProviderType		// @parm [IN] provider data type
)
{
	if (ProviderType & DBTYPE_BYREF)
		ProviderType &= ~DBTYPE_BYREF;

	switch(ProviderType)
	{
			case DBTYPE_NUMERIC:	// Numeric, Decimal
			case DBTYPE_I4:			// Integer
			case DBTYPE_I2:			// Smallint
			case DBTYPE_I1:			// Tinyint
			case DBTYPE_R8:			// Float, Double
			case DBTYPE_R4:			// Real
			case DBTYPE_UI2:
			case DBTYPE_UI4:
			case DBTYPE_I8:
			case DBTYPE_UI8:
				return TRUE;
		default:
				return FALSE;	// Compiler needs this
	}
}

//---------------------------------------------------------------------------
//	CCmd::IsColCharacter 
//
//	@mfunc	BOOL			|
//			CCmd		|
//			IsColCharacter	|
//			Can the data type hold string values?
//
//
//---------------------------------------------------------------------------
BOOL CCmd::IsColCharacter
(
	DBTYPE ProviderType	// @parm [IN] provider data type
)
{
	if (ProviderType & DBTYPE_BYREF)
		ProviderType &= ~DBTYPE_BYREF;

	switch(ProviderType)
	{
			case DBTYPE_STR:	// Character
			case DBTYPE_WSTR:
				return TRUE;
		default:
				return FALSE;	// Compiler needs this
	}
}

//---------------------------------------------------------------------------
//	CCmd::IsColDateTime 
//
//	@mfunc	BOOL			|
//			CCmd		|
//			IsColDateTime	|
//			Can the data type hold DateTime values?
//
//
//---------------------------------------------------------------------------
BOOL CCmd::IsColDateTime			
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

//--------------------------------------------------------------------
// CreateSession
//--------------------------------------------------------------------
BOOL CCmd::CreateSession
(
	REFIID	iid,
	IUnknown ** pIUnknown
)
{
	BOOL fResult = FALSE;
	IDBCreateSession * pIDBCreateSession = NULL;

	if(!pIUnknown)
		return FALSE;

	if(m_pMyIDBInitialize)
	{
		if(!VerifyInterface(m_pMyIDBInitialize, IID_IDBCreateSession,
						DATASOURCE_INTERFACE, (IUnknown**)&pIDBCreateSession))
			goto CLEANUP;

		if(SUCCEEDED(pIDBCreateSession->CreateSession(NULL,iid,pIUnknown)))
			fResult = TRUE;
	}
	
CLEANUP:
	
	SAFE_RELEASE(pIDBCreateSession);
	return fResult;
}

//-------------------------------------------------------------------
// GetCommand
//--------------------------------------------------------------------
BOOL CCmd::GetCommand
(
	REFIID iid,						// [OUT] IID for command
	IUnknown ** pIUnknownCommand,	// [OUT] command pointer
	IUnknown *	pIUnknownSession	// [IN] session object
)
{
	BOOL fResult = FALSE;
	IDBCreateCommand * pSession = NULL;

	if(!pIUnknownCommand)
		return FALSE;

	// Get session objects just gets the pointer.  No need to release pSession2.
	if(pIUnknownSession) {
		CLEANUP(!VerifyInterface(pIUnknownSession, IID_IDBCreateCommand,
									SESSION_INTERFACE, (IUnknown**)&pSession));
	}
	else {
		CLEANUP(GetSessionObject(IID_IDBCreateCommand, (IUnknown**)&pSession));
	}

	if(SUCCEEDED(pSession->CreateCommand(NULL,iid,pIUnknownCommand)))
		fResult = TRUE;

CLEANUP:
	
	SAFE_RELEASE(pSession);
	return fResult;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	ValidateSessionObject
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmd::ValidateSessionObject
(
	IUnknown * pIUnknown
)
{
	BOOL		fResult=FALSE;
	IUnknown *	pISession=NULL;

	if(!pIUnknown)
		return FALSE;
	
	// Get Session that created command
	TEST_CHECK(GetSessionObject(IID_IUnknown, &pISession), S_OK);
			
	if(COMPARE(pISession, pIUnknown))
		fResult = TRUE;

CLEANUP:
	
	SAFE_RELEASE(pISession);
	return fResult;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	SetPropertyAndExecute
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BOOL CCmd::SetPropertyAndExecute(
	ICommand * pICmd,
	DBPROPINFOSET * pPropertyInfoSets,
	ULONG iSet,
	ULONG iProp,
	enum SET_OPTION_ENUM eSetOption,
	WCHAR * pwszDescription1,
	BOOL * pfPropFound)
{
	IRowset * pIRowset = NULL;
	IUnknown* pIUnk = NULL;
	BOOL fResult = TRUE;
	HRESULT hrSetProp = E_FAIL;
	HRESULT hr = E_FAIL;
	VARIANT vValue;

	VariantInit(&vValue);

	odtLog << L"Property: " << pPropertyInfoSets[iSet].rgPropertyInfos[iProp].pwszDescription << L"\n";

	// If the property is not supported or not writable just skip it
	if (pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwFlags == DBPROPFLAGS_NOTSUPPORTED	||
		!(pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwFlags & DBPROPFLAGS_WRITE))
	{
		if (pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwFlags == DBPROPFLAGS_NOTSUPPORTED)
			odtLog << L"\tNot supported.\n";
		if (!(pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwFlags & DBPROPFLAGS_WRITE))
			odtLog << L"\tNot writable.\n";
		return TRUE;
	}

	// Record the current property value so we can reset it to a successful value later
	COMPARE(GetProperty(pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID, 
		pPropertyInfoSets[iSet].guidPropertySet, pICmd, &vValue), TRUE);

	// Set the property on
	hrSetProp = SetRowsetProperty(pICmd, pPropertyInfoSets[iSet].guidPropertySet,
		pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID, TRUE, 
		DBPROPOPTIONS_REQUIRED, FALSE);

	hr = Execute(pICmd,PPI &pIUnk);

	SAFE_RELEASE(pIUnk);

	// If this caused DB_E_ERRORSOCCURRED when REQUIRED it's one we want
	if (DB_E_ERRORSOCCURRED == hr)
	{
		odtLog << L"\tDB_E_ERRORSOCCURRED\n";

//		DumpCommandProps(pICmd, FALSE);

		// Make sure there are properties in error.
		COMPARE(PropertiesInError(pICmd, hr), TRUE);
		
		if (pfPropFound)
			*pfPropFound = TRUE;

		// Now set optional
		SetRowsetProperty(pICmd, pPropertyInfoSets[iSet].guidPropertySet,
			pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID,
			TRUE, DBPROPOPTIONS_OPTIONAL, FALSE);

		if (!CHECK(hr = Execute(pICmd,PPI &pIUnk),DB_S_ERRORSOCCURRED))
		{
  			WCHAR * pwszDescription = L"NULL property description!!";
			WCHAR * pwszDescription2 = L"NULL property description!!";
			if (pwszDescription1)
				pwszDescription = pwszDescription1;
			if (pPropertyInfoSets[iSet].rgPropertyInfos[iProp].pwszDescription)
				pwszDescription2 = pPropertyInfoSets[iSet].rgPropertyInfos[iProp].pwszDescription;

			odtLog << L"Execute did not return DB_S_ERRORSOCCURRED for OPTIONAL property that causes DB_E_ERRORSOCCURRED when REQUIRED.\n";

			if (eSetOption == PAIRED_PROP)
				odtLog << L"Property description (REQUIRED): " << pwszDescription1 <<
				L"\n Paired with \n";

			odtLog << L"Property description (OPTIONAL): " << pwszDescription2 << L"\n\n";

			fResult = FALSE;

		}
		else
		{
			odtLog << L"\tDB_S_ERRORSOCCURRED\n";
			// There should be at least one property in error
			COMPARE(PropertiesInError(pICmd, hr), TRUE);
		}


		SAFE_RELEASE(pIRowset);
	}
	else
	{
		HRESULT ExpHr = S_OK;

		// At least two properties (DBPROP_IMultipleResults, IRow, IRowset) will cause E_NOINTERFACE, since this causes
		// Execute to return an IMultipleResults obj or Row obj, not a rowset object.
		if (pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID == DBPROP_IMultipleResults ||
			pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID == DBPROP_IRowChange ||
			pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID == DBPROP_IRowSchemaChange ||
			pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID == DBPROP_IRowset ||
			pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID == DBPROP_IRow)
		{
			enum DBPROPOPTIONSENUM dwOption = DBPROPOPTIONS_REQUIRED;

			if(hrSetProp == DB_E_ERRORSOCCURRED)
			{
				if(IFROWSET)
					ExpHr = S_OK;
				else
					ExpHr = E_NOINTERFACE;
			}

			// Some providers can't roll the IRowset prop if this is set REQUIRED
			if (IFROW && pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID == DBPROP_IRowset)
				dwOption = DBPROPOPTIONS_OPTIONAL;

			// Turn the property back off so we don't ALWAYS have an IMultipleResults (or Row) object from 
			// now on.
			SetRowsetProperty(pICmd, pPropertyInfoSets[iSet].guidPropertySet,
				pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID, VARIANT_FALSE, 
				dwOption);

		}

		if (hr != E_NOINTERFACE && hr != DB_E_ERRORSOCCURRED && !CHECK(hr, S_OK))
		{
			WCHAR * pwszDescription = L"NULL property description!!";
			if (pPropertyInfoSets[iSet].rgPropertyInfos[iProp].pwszDescription)
				pwszDescription = pPropertyInfoSets[iSet].rgPropertyInfos[iProp].pwszDescription;

			odtLog << L"Execute did not return S_OK, E_NOINTERFACE, or DB_E_ERRORSOCCURRED for REQUIRED property.\n" <<
			L"Property description: " << pwszDescription <<  L"\n\n";

			fResult = FALSE;

		}
	}

	VariantClear(&vValue);
	SAFE_RELEASE(pIUnk);

	return fResult;
}


HRESULT CCmd::VerifyRowset(REFIID riidRowset, IUnknown * pUnkRowset, ULONG ulStartingRow,
	DBORDINAL cRowsetCols, DBORDINAL * rgTableColOrds, BOOL fRelease)
{
	BOOL fResult = FALSE;
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

	// If we didn't get passed the mapping of rowset cols to table cols we can't compare data
	if (!cRowsetCols || !rgTableColOrds)
		return E_FAIL;
	
	// Make sure we can get a rowset interface
	TESTC(VerifyInterface(pUnkRowset, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown **)&pIRowset));

	// If we were passed in a rowset interface, use the one passed in
	if (riidRowset == IID_IRowset)
	{
		SAFE_RELEASE(pIRowset);
		pIRowset = (IRowset *)pUnkRowset;
	}

	// Make sure we can get an accessor interface
	TESTC(VerifyInterface(pUnkRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown **)&pIAccessor));

	TESTC_(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
		&hAccessor, &pBindings, &cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

	SAFE_ALLOC(pData, BYTE, cbRowSize * sizeof(BYTE));

	// TODO: Use IMultipleResults here if supported in case multiple results are returned??
	while (S_OK == (hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &prghRow)))
	{
		TESTC(cRowsObtained == 1);

		CHECK(hr = pIRowset->GetData(*prghRow, hAccessor, pData), S_OK);

		if (SUCCEEDED(hr))
		{
				TESTC(CompareData(
							cRowsetCols,
							(DB_LORDINAL *)rgTableColOrds,
							ulStartingRow,
							pData,
							cBindings,
							pBindings,
							m_pTable,
							m_pIMalloc,
							PRIMARY,
							COMPARE_ONLY,
							COMPARE_ALL,
							TRUE
						));

		
			TESTC_(pIRowset->ReleaseRows(cRowsObtained, prghRow, NULL, NULL, NULL), S_OK);

			ulStartingRow++;
		}
		else
			// If this GetData failed it's doubtful further GetData's are of much use
			goto CLEANUP;
	}

	TESTC_(hr, DB_S_ENDOFROWSET);

	COMPARE(ulStartingRow, m_pTable->GetRowsOnCTable() +1);

	fResult = TRUE;

CLEANUP:

	if ((hAccessor != DB_INVALID_HACCESSOR) && 
		!CHECK(pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK))
		fResult = FALSE;
		

	SAFE_RELEASE(pIAccessor);
	
	// If we passed in a rowset pointer then it gets release below if requested
	// otherwise release here.
	if (riidRowset != IID_IRowset)
		SAFE_RELEASE(pIRowset);

	if (fRelease)
		SAFE_RELEASE(pUnkRowset);

	FreeAccessorBindings(cBindings, pBindings);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(prghRow);

	return fResult ? S_OK : E_FAIL;
}

HRESULT CCmd::VerifyRowObj(REFIID riidRow, IUnknown * pUnkRow, ULONG ulStartingRow,
	DBORDINAL cRowsetCols, DBORDINAL * rgTableColOrds, BOOL fRelease)
{
	BOOL fResult = FALSE;
	CRowObject*	pCRow = NULL;
	
	pCRow = new CRowObject();
	TESTC(pCRow != NULL)
	TESTC_(pCRow->SetRowObject(pUnkRow), S_OK)

	//Veify the GetColumns method.
	TESTC(pCRow->VerifyGetColumns(ulStartingRow, m_pTable))

	fResult = TRUE;

CLEANUP:
	SAFE_DELETE(pCRow);
	return fResult ? S_OK : E_FAIL;
}

// Verify the specified row number appears in the table cRows times.  The
// row number is based on the insert seed number.
BOOL CCmd::VerifyAndRemoveRows(CTable * pTable, DBCOUNTITEM ulRowNum, DBCOUNTITEM cRows)
{
	ICommandText * pICommandText = NULL;
	ICommand * pICommand = NULL;
	IRowset * pIRowset = NULL;
	DBORDINAL cCols;
	DBCOUNTITEM cRowsObtained;
	DB_LORDINAL * pCols = NULL;
	HROW * phRows = NULL;
	BOOL fResult = FALSE;
	WCHAR * pwszSqlStmt = NULL;

	TESTC((pICommand = pTable->m_pICommand) != NULL);

	// Set command text for a select query using literals.
	TESTC_(pTable->CreateSQLStmt(SELECT_ROW_WITH_LITERALS, NULL, &pwszSqlStmt,&cCols,&pCols, ulRowNum), S_OK);

	TESTC(VerifyInterface(pICommand, IID_ICommandText,
						COMMAND_INTERFACE, (IUnknown**)&pICommandText));

	TESTC_(pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSqlStmt), S_OK);

	// Retrieve all rows that match the requested row.
	TESTC_(pICommand->Execute(NULL, IID_IRowset, NULL, 
		NULL, (IUnknown **)&pIRowset),S_OK);
	
	// There should be only the number of rows we expect so asking for one more will be ENDOFROWSET
	TESTC_(pIRowset->GetNextRows(NULL, 0, cRows+1, &cRowsObtained, &phRows), DB_S_ENDOFROWSET);

	if (phRows)
	{
		pIRowset->ReleaseRows(cRows, phRows, NULL, NULL, NULL);
		SAFE_FREE(phRows);
	}

	SAFE_RELEASE(pIRowset);

	// Make sure not fewer rows
	fResult = COMPARE(cRowsObtained, cRows);

	fResult &= CHECK(pTable->Delete(ulRowNum,PRIMARY,TRUE,NULL), S_OK);

CLEANUP:

	if (phRows)
		pIRowset->ReleaseRows(cRows, phRows, NULL, NULL, NULL);

	SAFE_FREE(phRows);
	SAFE_FREE(pCols);
	SAFE_FREE(pwszSqlStmt);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pIRowset);

	return fResult;
}

BOOL CCmd::VerifyThreadingModel(DWORD dwCoInit)
{
	ULONG_PTR ulThreadModel;
	BOOL fValidThreadModel = FALSE;
	HRESULT hr = E_FAIL;

	// Get the provider's thread model
	if (m_pIDBInitialize)
	{
		// See if our base thread is in free threaded. 
		if (SUCCEEDED(hr = CoInit(COINIT_MULTITHREADED)))
			CoUninitialize(); 

		switch(hr)
		{
			case S_FALSE:
				// Base thread is multi-threaded apartment model.
				odtLog << L"Base thread (LTM) is free threaded.\n";
				fValidThreadModel = TRUE;
				break;
			case E_INVALIDARG:
			case RPC_E_CHANGED_MODE:
				// The user (LTM) might have called CoInitialize to set apartment model, and the two API's don't
				// seem to work together properly to return S_FALSE.
				if (SUCCEEDED(hr = CoInitialize(NULL)))
					CoUninitialize(); 

				if (S_FALSE == hr)
					// Base thread is apartment model.
					odtLog << L"Base thread (LTM) is apartment model, not currently supported by test.\n";
				else
					fValidThreadModel = TRUE;
				break;
			default:
				odtLog << L"Unrecognized base threading model.\n";
		}


		// Warn that message loop needed for apartment threading
		// We don't currently work with apartment threading
		if (dwCoInit & COINIT_APARTMENTTHREADED)
		{
			odtLog << L"Aparment model requested but not currently supported..\n";
			fValidThreadModel = FALSE;
		}

		// If the provider doesn't support the thread model property we
		// assume single threaded.
		if (!GetProperty(DBPROP_DSOTHREADMODEL, DBPROPSET_DATASOURCEINFO,
			m_pIDBInitialize, &ulThreadModel))
			ulThreadModel = DBPROPVAL_RT_SINGLETHREAD;

		// Warn if the requested model is not supported by the provider.
		if ((dwCoInit & COINIT_APARTMENTTHREADED &&
			!(ulThreadModel & DBPROPVAL_RT_APTMTTHREAD)) ||
			(dwCoInit & COINIT_MULTITHREADED && 
			!(ulThreadModel & DBPROPVAL_RT_FREETHREAD)))
		{
			odtLog << L"Requested threading model doesn't match provider's supported value.\n";
			fValidThreadModel = FALSE;
		}

	}

	return fValidThreadModel;

}

// CCmd::SetRowsetPropertyDefault ------------------------------------
//
// Sets the given rowset property using ICommandProperties to the default value
//
//
HRESULT CCmd::SetRowsetPropertyDefault(DBPROPID DBPropID, ICommand * pICommand)
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

	TESTC(pICommand != NULL);
	
	TESTC_(pICommand->QueryInterface(IID_ICommandProperties, 
		(void **)&pICmdProps), S_OK);
		
	TESTC_(pICmdProps->SetProperties(1, &DBPropSet), S_OK);

	hr = S_OK;

CLEANUP:

	SAFE_RELEASE(pICmdProps);
	
	return hr;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_TEST_CASE_MAP(CExecute_Rowset)
//--------------------------------------------------------------------
// @class general valid variations
//
class CExecute_Rowset : public CCmd { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CExecute_Rowset,CCmd);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK: Multiple Executions on same command object (Select)
	int Variation_1();
	// @cmember S_OK:Multiple Executions on same command object (Insert)
	int Variation_2();
	// @cmember S_OK:Prepared command
	int Variation_3();
	// @cmember S_OK:Prepared and Unprepared Command
	int Variation_4();
	// @cmember S_OK:Multiple Sessions Open
	int Variation_5();
	// @cmember S_OK:Multiple Commands Open
	int Variation_6();
	// @cmember S_OK: 1 cmd obj (3 inserts on their own threads
	int Variation_7();
	// @cmember S_OK: Query set only
	int Variation_8();
	// @cmember DB_E_NOCOMMAND: no command set
	int Variation_9();
	// @cmember DB_E_ERRORSINCOMMAND - (Select)
	int Variation_10();
	// @cmember DB_E_ERRORSINCOMMAND - (Insert)
	int Variation_11();
	// @cmember DB_INVALIDARG - cParamSets > 1 and ppRowset was not a null pointer
	int Variation_12();
	// @cmember DB_E_MULTIPLESTATEMENTS - command contained multiple statements
	int Variation_13();
	// @cmember DB_S_ERRORSOCCURED - property  was not set
	int Variation_14();
	// @cmember S_OK: Insert, riid=IID_NULL
	int Variation_15();
	// @cmember S_OK: Insert, riid=IID_IRowset
	int Variation_16();
	// @cmember S_OK: Insert, riid=IID_IRowsetInfo
	int Variation_17();
	// @cmember S_OK: Select, riid=IID_IRowset
	int Variation_18();
	// @cmember S_OK: Select, riid=IID_IRowsetInfo
	int Variation_19();
	// @cmember E_NOINTERACE: dso iid
	int Variation_20();
	// @cmember S_OK:valid pcRowsAffected, *pc=0, valid pIRowset (Select)
	int Variation_21();
	// @cmember S_OK:valid pcRowsAffected, *pc=1, valid pIRowset (Select)
	int Variation_22();
	// @cmember S_OK:valid pcRowsAffected, *pc=0, valid pIRowset (Insert)
	int Variation_23();
	// @cmember S_OK:valid pcRowsAffected, *pc=1, valid pIRowset (Insert)
	int Variation_24();
	// @cmember S_OK:NULL pcRowsAffected, valid pIRowset (Select)
	int Variation_25();
	// @cmember E_INVALIDARG:NULL pcRowsAffected, NULL pIRowset (Select)
	int Variation_26();
	// @cmember S_OK:NULL pcRowsAffected, valid pIRowset (Insert)
	int Variation_27();
	// @cmember S_OK:NULL pcRowsAffected, NULL pIRowset (Insert)
	int Variation_28();
	// @cmember S_OK:valid pcRowsAffected, *pc=0, valid pIRowsetLocate (Select)
	int Variation_29();
	// @cmember S_OK:valid pcRowsAffected, *pc=1, valid pIRowsetLocate (Select)
	int Variation_30();
	// @cmember S_OK:valid pcRowsAffected, *pc=0, valid pIRowsetLocate (Insert)
	int Variation_31();
	// @cmember S_OK:NULL pcRowsAffected, valid pIRowsetLocate (Select)
	int Variation_32();
	// @cmember E_INVALIDARG:NULL pcRowsAffected, NULL pIRowsetLocate (Select)
	int Variation_33();
	// @cmember S_OK:NULL pcRowsAffected, valid pIRowsetLocate (Insert)
	int Variation_34();
	// @cmember S_OK:NULL pcRowsAffected, NULL pIRowsetLocate (Insert)
	int Variation_35();
	// @cmember S_OK:valid pcRowsAffected, *pc=0, valid pIColumnsInfo (Select)
	int Variation_36();
	// @cmember S_OK:valid pcRowsAffected, *pc=1, valid pIColumnsInfo (Select)
	int Variation_37();
	// @cmember S_OK:valid pcRowsAffected, *pc=0, valid pIColumnsInfo (Insert)
	int Variation_38();
	// @cmember S_OK:valid pcRowsAffected, *pc=1, valid pIColumnsInfo (Insert)
	int Variation_39();
	// @cmember S_OK:NULL pcRowsAffected, valid pIColumnsInfo (Select)
	int Variation_40();
	// @cmember E_INVALIDARG:NULL pcRowsAffected, NULL pIColumnsInfo (Select)
	int Variation_41();
	// @cmember S_OK:NULL pcRowsAffected, valid pIColumnsInfo (Insert)
	int Variation_42();
	// @cmember S_OK:NULL pcRowsAffected, NULL pIColumnsInfo (Insert)
	int Variation_43();
	// @cmember E_INVALIDARG:riid != IID_NULL, ppRowset==NULL
	int Variation_44();
	// @cmember DB_E_CANTCONVERTVALUE: string (22005)
	int Variation_45();
	// @cmember DB_E_CANTCONVERTVALUE: implicit (22005)
	int Variation_46();
	// @cmember DB_E_CANTCONVERTVALUE: implicit (22008)
	int Variation_47();
	// @cmember DB_E_DATAOVERFLOW: math (22003)
	int Variation_48();
	// @cmember DB_E_DATAOVERFLOW: string right (22001
	int Variation_49();
	// @cmember DB_E_ERRORSINCOMMAND - Create Table statement on a table that already exists (S0001)
	int Variation_50();
	// @cmember DB_E_ERRORSINCOMMAND - Create View statement on a view that already exists (S0001)
	int Variation_51();
	// @cmember DB_E_ERRORSINCOMMAND - Select statement with invalid table name (S0002)
	int Variation_52();
	// @cmember DB_E_ERRORSINCOMMAND - Drop Table statement on a table that does not exist (S0002)
	int Variation_53();
	// @cmember DB_E_ERRORSINCOMMAND - Drop View statement on a view that does not exist (S0002)
	int Variation_54();
	// @cmember DB_E_ERRORSINCOMMAND - Select statement with invalid column name (S0022)
	int Variation_55();
	// @cmember S_OK - Numeric Truncation (01004)
	int Variation_56();
	// @cmember DB_E_ERRORSINCOMMAND - Invalid nodes in a command (37000)
	int Variation_57();
	// @cmember S_OK: riid = IUnknown
	int Variation_58();
	// @cmember S_OK: cParam=0
	int Variation_59();
	// @cmember S_OK: select * from sp_tables
	int Variation_60();
	// @cmember DB_E_ERRORSINCOMMAND: create current exisiting table
	int Variation_61();
	// @cmember Query TimeOut
	int Variation_62();
	// @cmember Query TimeOut
	int Variation_63();
	// @cmember Firehose mode
	int Variation_64();
	// @cmember Aggregation - Open a rowset on aggregated command
	int Variation_65();
	// @cmember S_OK: Select, IID_NULL, ppRowset NULL
	int Variation_66();
	// @cmember Test all IID's of object
	int Variation_67();
	// @cmember E_NOINTERFACE - Ask for unsupported interface with ppRowset == NULL
	int Variation_68();
	// @cmember DB_E_INTEGRITYVIOLATION - Insert a duplicate key value
	int Variation_69();
	// @cmember S_OK - select with qualified name
	int Variation_70();
	// @cmember S_OK - select with quoted name
	int Variation_71();
	// @cmember S_OK - select distinct
	int Variation_72();
	// @cmember DB_E_ERRORSOCCURRED - select distinct with DBPROP_UNIQUEROWS REQUIRED
	int Variation_73();
	// @cmember DB_E_ERRORSINCOMMAND - select with invalid group by clause
	int Variation_74();
	// @cmember DB_S_ERRORSOCCURRED - select distinct with DBPROP_UNIQUEROWS OPTIONAL
	int Variation_75();
	// @cmember DB_E_ERRORSINCOMMAND - (Insert with mis-spelled keyword)
	int Variation_76();
	// @cmember S_OK: Multiple commands, 2 initial and 1 later
	int Variation_77();
	// @cmember S_OK: Create index in descending order
	int Variation_78();
	// @cmember S_OK: All rows with singleton select
	int Variation_79();
	// @cmember S_OK: Select ABC and Col List
	int Variation_80();
	// @cmember S_OK: Select duplicate columns
	int Variation_81();
	// @cmember S_OK: Select Count
	int Variation_82();
	// @cmember S_OK: Select empty rowset
	int Variation_83();
	// @cmember S_OK: Select Change col name
	int Variation_84();
	// @cmember S_OK: Aggregated rowset
	int Variation_85();
	// @cmember Last Variation - Final Verification
	int Variation_86();
	// }} TCW_TESTVARS_END
};
// }}
// }}

// {{ TCW_TESTCASE(CExecute_Rowset)
#define THE_CLASS CExecute_Rowset
BEG_TEST_CASE(CExecute_Rowset, CCmd, L"general valid variations")
	TEST_VARIATION(1, 		L"S_OK: Multiple Executions on same command object (Select)")
	TEST_VARIATION(2, 		L"S_OK:Multiple Executions on same command object (Insert)")
	TEST_VARIATION(3, 		L"S_OK:Prepared command")
	TEST_VARIATION(4, 		L"S_OK:Prepared and Unprepared Command")
	TEST_VARIATION(5, 		L"S_OK:Multiple Sessions Open")
	TEST_VARIATION(6, 		L"S_OK:Multiple Commands Open")
	TEST_VARIATION(7, 		L"S_OK: 1 cmd obj (3 inserts on their own threads")
	TEST_VARIATION(8, 		L"S_OK: Query set only")
	TEST_VARIATION(9, 		L"DB_E_NOCOMMAND: no command set")
	TEST_VARIATION(10, 		L"DB_E_ERRORSINCOMMAND - (Select)")
	TEST_VARIATION(11, 		L"DB_E_ERRORSINCOMMAND - (Insert)")
	TEST_VARIATION(12, 		L"DB_INVALIDARG - cParamSets > 1 and ppRowset was not a null pointer")
	TEST_VARIATION(13, 		L"DB_E_MULTIPLESTATEMENTS - command contained multiple statements")
	TEST_VARIATION(14, 		L"DB_S_ERRORSOCCURED - property  was not set")
	TEST_VARIATION(15, 		L"S_OK: Insert, riid=IID_NULL")
	TEST_VARIATION(16, 		L"S_OK: Insert, riid=IID_IRowset")
	TEST_VARIATION(17, 		L"S_OK: Insert, riid=IID_IRowsetInfo")
	TEST_VARIATION(18, 		L"S_OK: Select, riid=IID_IRowset")
	TEST_VARIATION(19, 		L"S_OK: Select, riid=IID_IRowsetInfo")
	TEST_VARIATION(20, 		L"E_NOINTERACE: dso iid")
	TEST_VARIATION(21, 		L"S_OK:valid pcRowsAffected, *pc=0, valid pIRowset (Select)")
	TEST_VARIATION(22, 		L"S_OK:valid pcRowsAffected, *pc=1, valid pIRowset (Select)")
	TEST_VARIATION(23, 		L"S_OK:valid pcRowsAffected, *pc=0, valid pIRowset (Insert)")
	TEST_VARIATION(24, 		L"S_OK:valid pcRowsAffected, *pc=1, valid pIRowset (Insert)")
	TEST_VARIATION(25, 		L"S_OK:NULL pcRowsAffected, valid pIRowset (Select)")
	TEST_VARIATION(26, 		L"E_INVALIDARG:NULL pcRowsAffected, NULL pIRowset (Select)")
	TEST_VARIATION(27, 		L"S_OK:NULL pcRowsAffected, valid pIRowset (Insert)")
	TEST_VARIATION(28, 		L"S_OK:NULL pcRowsAffected, NULL pIRowset (Insert)")
	TEST_VARIATION(29, 		L"S_OK:valid pcRowsAffected, *pc=0, valid pIRowsetLocate (Select)")
	TEST_VARIATION(30, 		L"S_OK:valid pcRowsAffected, *pc=1, valid pIRowsetLocate (Select)")
	TEST_VARIATION(31, 		L"S_OK:valid pcRowsAffected, *pc=0, valid pIRowsetLocate (Insert)")
	TEST_VARIATION(32, 		L"S_OK:NULL pcRowsAffected, valid pIRowsetLocate (Select)")
	TEST_VARIATION(33, 		L"E_INVALIDARG:NULL pcRowsAffected, NULL pIRowsetLocate (Select)")
	TEST_VARIATION(34, 		L"S_OK:NULL pcRowsAffected, valid pIRowsetLocate (Insert)")
	TEST_VARIATION(35, 		L"S_OK:NULL pcRowsAffected, NULL pIRowsetLocate (Insert)")
	TEST_VARIATION(36, 		L"S_OK:valid pcRowsAffected, *pc=0, valid pIColumnsInfo (Select)")
	TEST_VARIATION(37, 		L"S_OK:valid pcRowsAffected, *pc=1, valid pIColumnsInfo (Select)")
	TEST_VARIATION(38, 		L"S_OK:valid pcRowsAffected, *pc=0, valid pIColumnsInfo (Insert)")
	TEST_VARIATION(39, 		L"S_OK:valid pcRowsAffected, *pc=1, valid pIColumnsInfo (Insert)")
	TEST_VARIATION(40, 		L"S_OK:NULL pcRowsAffected, valid pIColumnsInfo (Select)")
	TEST_VARIATION(41, 		L"E_INVALIDARG:NULL pcRowsAffected, NULL pIColumnsInfo (Select)")
	TEST_VARIATION(42, 		L"S_OK:NULL pcRowsAffected, valid pIColumnsInfo (Insert)")
	TEST_VARIATION(43, 		L"S_OK:NULL pcRowsAffected, NULL pIColumnsInfo (Insert)")
	TEST_VARIATION(44, 		L"E_INVALIDARG:riid != IID_NULL, ppRowset==NULL")
	TEST_VARIATION(45, 		L"DB_E_CANTCONVERTVALUE: string (22005)")
	TEST_VARIATION(46, 		L"DB_E_CANTCONVERTVALUE: implicit (22005)")
	TEST_VARIATION(47, 		L"DB_E_CANTCONVERTVALUE: implicit (22008)")
	TEST_VARIATION(48, 		L"DB_E_DATAOVERFLOW: math (22003)")
	TEST_VARIATION(49, 		L"DB_E_DATAOVERFLOW: string right (22001")
	TEST_VARIATION(50, 		L"DB_E_ERRORSINCOMMAND - Create Table statement on a table that already exists (S0001)")
	TEST_VARIATION(51, 		L"DB_E_ERRORSINCOMMAND - Create View statement on a view that already exists (S0001)")
	TEST_VARIATION(52, 		L"DB_E_ERRORSINCOMMAND - Select statement with invalid table name (S0002)")
	TEST_VARIATION(53, 		L"DB_E_ERRORSINCOMMAND - Drop Table statement on a table that does not exist (S0002)")
	TEST_VARIATION(54, 		L"DB_E_ERRORSINCOMMAND - Drop View statement on a view that does not exist (S0002)")
	TEST_VARIATION(55, 		L"DB_E_ERRORSINCOMMAND - Select statement with invalid column name (S0022)")
	TEST_VARIATION(56, 		L"S_OK - Numeric Truncation (01004)")
	TEST_VARIATION(57, 		L"DB_E_ERRORSINCOMMAND - Invalid nodes in a command (37000)")
	TEST_VARIATION(58, 		L"S_OK: riid = IUnknown")
	TEST_VARIATION(59, 		L"S_OK: cParam=0")
	TEST_VARIATION(60, 		L"S_OK: select * from sp_tables")
	TEST_VARIATION(61, 		L"DB_E_ERRORSINCOMMAND: create current exisiting table")
	TEST_VARIATION(62, 		L"Query TimeOut")
	TEST_VARIATION(63, 		L"Query TimeOut")
	TEST_VARIATION(64, 		L"Firehose mode")
	TEST_VARIATION(65, 		L"Aggregation - Open a rowset on aggregated command")
	TEST_VARIATION(66, 		L"S_OK: Select, IID_NULL, ppRowset NULL")
	TEST_VARIATION(67, 		L"Test all IID's of object")
	TEST_VARIATION(68, 		L"E_NOINTERFACE - Ask for unsupported interface with ppRowset == NULL")
	TEST_VARIATION(69, 		L"DB_E_INTEGRITYVIOLATION - Insert a duplicate key value")
	TEST_VARIATION(70, 		L"S_OK - select with qualified name")
	TEST_VARIATION(71, 		L"S_OK - select with quoted name")
	TEST_VARIATION(72, 		L"S_OK - select distinct")
	TEST_VARIATION(73, 		L"DB_E_ERRORSOCCURRED - select distinct with DBPROP_UNIQUEROWS REQUIRED")
	TEST_VARIATION(74, 		L"DB_E_ERRORSINCOMMAND - select with invalid group by clause")
	TEST_VARIATION(75, 		L"DB_S_ERRORSOCCURRED - select distinct with DBPROP_UNIQUEROWS OPTIONAL")
	TEST_VARIATION(76, 		L"DB_E_ERRORSINCOMMAND - (Insert with mis-spelled keyword)")
	TEST_VARIATION(77, 		L"S_OK: Multiple commands, 2 initial and 1 later")
	TEST_VARIATION(78, 		L"S_OK: Create index in descending order")
	TEST_VARIATION(79, 		L"S_OK: All rows with singleton select")
	TEST_VARIATION(80, 		L"S_OK: Select ABC and Col List")
	TEST_VARIATION(81, 		L"S_OK: Select duplicate columns")
	TEST_VARIATION(82, 		L"S_OK: Select Count")
	TEST_VARIATION(83, 		L"S_OK: Select empty rowset")
	TEST_VARIATION(84, 		L"S_OK: Select Change col name")
	TEST_VARIATION(85, 		L"S_OK: Aggregated rowset")
	TEST_VARIATION(86, 		L"Last Variation - Final Verification")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// {{ TCW_TEST_CASE_MAP(CDBSession)
//--------------------------------------------------------------------
// @class general cases
//
class CDBSession : public CCmd { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CDBSession,CCmd);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK: IRowsetInfo::GetSpecification, get same IUnknown
	int Variation_1();
	// @cmember S_OK: Don't execute command, get same IUnknown
	int Variation_2();
	// @cmember E_NOINTERFACE: dso iid, valid ptr
	int Variation_4();
	// @cmember E_NOINTERFACE: iid_null, valid ptr
	int Variation_5();
	// @cmember E_INVALIDARG: valid session id, ptr==NULL
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(CDBSession)
#define THE_CLASS CDBSession
BEG_TEST_CASE(CDBSession, CCmd, L"general cases")
	TEST_VARIATION(1, 		L"S_OK: IRowsetInfo::GetSpecification, get same IUnknown")
	TEST_VARIATION(2, 		L"S_OK: Don't execute command, get same IUnknown")
	TEST_VARIATION(4, 		L"E_NOINTERFACE: dso iid, valid ptr")
	TEST_VARIATION(5, 		L"E_NOINTERFACE: iid_null, valid ptr")
	TEST_VARIATION(6, 		L"E_INVALIDARG: valid session id, ptr==NULL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// {{ TCW_TEST_CASE_MAP(Zombie)
//--------------------------------------------------------------------
// @class Induce zombie states
//
class Zombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Zombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	int Zombie::TestTxn(ETXN eTxn,BOOL fRetaining);


	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember State 1:Abort with fRetaining set to TRUE
	int Variation_1();
	// @cmember State 2:Abort with fRetaining set to FALSE
	int Variation_2();
	// @cmember State 3:Commit with fRetaining set to TRUE
	int Variation_3();
	// @cmember State 4:Commit with fRetaining set to FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Zombie)
#define THE_CLASS Zombie
BEG_TEST_CASE(Zombie, CTransaction, L"Induce zombie states")
	TEST_VARIATION(1, 		L"State 1:Abort with fRetaining set to TRUE")
	TEST_VARIATION(2, 		L"State 2:Abort with fRetaining set to FALSE")
	TEST_VARIATION(3, 		L"State 3:Commit with fRetaining set to TRUE")
	TEST_VARIATION(4, 		L"State 4:Commit with fRetaining set to FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(CCancel)
//--------------------------------------------------------------------
// @class general cases
//
class CCancel : public CCmd { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CCancel,CCmd);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember S_OK,S_OK: 2 cmd obj (Select & Insert), cancel each during execution
	int Variation_1();
	// @cmember S_OK,S_OK: 2 cmd obj (Select & Insert), cancel 1 during, cancel 1 after execution
	int Variation_2();
	// @cmember S_OK,S_OK: 2 cmd obj (Select & Insert), cancel 1 before, cancel 1 during execution
	int Variation_3();
	// @cmember S_OK,S_OK: 2 cmd obj, cancel 1 before, cancel 1 after
	int Variation_4();
	// @cmember S_OK: 1 cmd obj (Select), cancel before execution
	int Variation_5();
	// @cmember S_OK: 1 cmd obj (Select), cancel after execution
	int Variation_6();
	// @cmember S_OK: 1 cmd obj (Select), execute, cancel cancel
	int Variation_7();
	// @cmember S_OK: 1 cmd obj (Insert), cancel before execution
	int Variation_8();
	// @cmember S_OK: 1 cmd obj (Insert), cancel after execution
	int Variation_9();
	// @cmember S_OK: 1 cmd obj (Insert), execute, cancel cancel
	int Variation_10();
	// @cmember No Query Set, 1 Cancel per Thread, Before Execution
	int Variation_11();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(CCancel)
#define THE_CLASS CCancel
BEG_TEST_CASE(CCancel, CCmd, L"general cases")
	TEST_VARIATION(1, 		L"S_OK,S_OK: 2 cmd obj (Select & Insert), cancel each during execution")
	TEST_VARIATION(2, 		L"S_OK,S_OK: 2 cmd obj (Select & Insert), cancel 1 during, cancel 1 after execution")
	TEST_VARIATION(3, 		L"S_OK,S_OK: 2 cmd obj (Select & Insert), cancel 1 before, cancel 1 during execution")
	TEST_VARIATION(4, 		L"S_OK,S_OK: 2 cmd obj, cancel 1 before, cancel 1 after")
	TEST_VARIATION(5, 		L"S_OK: 1 cmd obj (Select), cancel before execution")
	TEST_VARIATION(6, 		L"S_OK: 1 cmd obj (Select), cancel after execution")
	TEST_VARIATION(7, 		L"S_OK: 1 cmd obj (Select), execute, cancel cancel")
	TEST_VARIATION(8, 		L"S_OK: 1 cmd obj (Insert), cancel before execution")
	TEST_VARIATION(9, 		L"S_OK: 1 cmd obj (Insert), cancel after execution")
	TEST_VARIATION(10, 		L"S_OK: 1 cmd obj (Insert), execute, cancel cancel")
	TEST_VARIATION(11, 		L"No Query Set, 1 Cancel per Thread, Before Execution")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public CCmd { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
	//@cmember Extended error object
	CExtError * m_pExtError;
	

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,CCmd);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid ICommand call with previous error object existing.
	int Variation_1();
	// @cmember Invalid Execute call with previous error object existing
	int Variation_2();
	// @cmember Invalid GetDBSession calls with previous error object existing
	int Variation_3();
	// @cmember Invalid GetDBSession calls with no previous error object existing
	int Variation_4();
	// @cmember E_NOINTERFACE Execute call with no previous error object existing
	int Variation_5();
	// @cmember DB_E_NOCOMMAND Execute call with no previous error object existing
	int Variation_6();
	// @cmember DB_E_CANCELED Check extended error info when Execute is canceled
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, CCmd, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid ICommand call with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid Execute call with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid GetDBSession calls with previous error object existing")
	TEST_VARIATION(4, 		L"Invalid GetDBSession calls with no previous error object existing")
	TEST_VARIATION(5, 		L"E_NOINTERFACE Execute call with no previous error object existing")
	TEST_VARIATION(6, 		L"DB_E_NOCOMMAND Execute call with no previous error object existing")
	TEST_VARIATION(7, 		L"DB_E_CANCELED Check extended error info when Execute is canceled")
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

COPY_TEST_CASE(CExecute_Row, CExecute_Rowset)


#if 0
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(5, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, CExecute_Rowset)
	TEST_CASE(2, CDBSession)
	TEST_CASE(3, Zombie)
	TEST_CASE(4, CCancel)
	TEST_CASE(5, TCExtendedErrors)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(6, ThisModule, gwszModuleDescrip)

	TEST_CASE_WITH_PARAM(1, CExecute_Rowset, TC_Rowset)
	TEST_CASE_WITH_PARAM(2, CExecute_Row, TC_Row)

	TEST_CASE(3, CDBSession)
	TEST_CASE(4, Zombie)
	TEST_CASE(5, CCancel)
	TEST_CASE(6, TCExtendedErrors)
END_TEST_MODULE()
#endif

// {{ TCW_TC_PROTOTYPE(CExecute_Rowset)
//*-----------------------------------------------------------------------
//| Test Case:		CExecute_Rowset - general valid variations
//|	Created:			04/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CExecute_Rowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCmd::Init())
	// }}
	{
		ULONG_PTR ulOleObj = 0;

		if(!GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
			GetModInfo()->GetThisTestModule()->m_pIUnknown, &ulOleObj) ||
			!(ulOleObj & DBPROPVAL_OO_SINGLETON))
		{
			if(IFROW)
			{
				odtLog<<L"INFO: Obtaining row objects directly from commands is not supported.\n";
				return TEST_SKIPPED;
			}
		}

		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Multiple Executions on same command object (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_1()
{
	ICommand *	pICmd1   = NULL;
	IUnknown*	pIUnk1 = NULL;
	IUnknown*	pIUnk2 = NULL;
	IUnknown*	pIUnk3 = NULL;
	
  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));

	CLEANUP(!CHECK(m_hr = Execute(pICmd1, PPI &pIUnk1),S_OK));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1, PPI &pIUnk2),S_OK));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1, PPI &pIUnk3),S_OK));

	TESTC(pIUnk1 && pIUnk2 && pIUnk3)

CLEANUP:
	SAFE_RELEASE(pIUnk1);
	SAFE_RELEASE(pIUnk2);
	SAFE_RELEASE(pIUnk3);
	SAFE_RELEASE(pICmd1);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK:Multiple Executions on same command object (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_2()
{
	ICommand*	pICmd1=NULL;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL,NULL,NULL,NULL,IID_NULL),S_OK));

	m_pTable->AddRow();

	// Have to reset the command text to make sure the new row will be unique
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL,NULL,NULL,NULL,IID_NULL),S_OK));			

	m_pTable->AddRow();
	
	// Have to reset the command text to make sure the new row will be unique
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL,NULL,NULL,NULL,IID_NULL),S_OK));

	m_pTable->AddRow();

	// Set text again and execute again.
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL,NULL,NULL,NULL,IID_NULL),S_OK));

	m_pTable->AddRow();

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK:Prepared command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_3()
{
	ICommand*	pICmd1=NULL;
	DBROWCOUNT cRowsAffected=0;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(FAILED(PrepareCommand(pICmd1,PREPARE,1)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL,NULL,NULL,NULL,IID_NULL),S_OK));
	m_pTable->AddRow();

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK:Prepared and Unprepared Command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_4()
{	
	DBROWCOUNT cRowsAffected=0;
	ICommand*	pICmd1=NULL;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))

	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(FAILED(PrepareCommand(pICmd1,BOTH,1)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL,NULL,NULL,NULL,IID_NULL),S_OK));
	m_pTable->AddRow();

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK:Multiple Sessions Open
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_5()
{
	TESTBEGIN

	ULONG_PTR	ulMaxSessions=0;
	IUnknown *	pUnk1=NULL;
	IUnknown *	pUnk3=NULL;

	// first session object is still in coledb classes
	IDBCreateCommand *	pCreateCmd1=NULL;
	IDBCreateCommand *	pCreateCmd2=NULL;
	IDBCreateCommand *	pCreateCmd3=NULL;

	//first command object is still in m_pICommand
	ICommand *	pICmd1=NULL;
	ICommand *	pICmd2=NULL;
	ICommand *	pICmd3=NULL;

	// Check the Session Limit
	GetProperty(DBPROP_ACTIVESESSIONS, 
				DBPROPSET_DATASOURCEINFO, m_pMyIDBInitialize, &ulMaxSessions);
	
	// Need atleast 4 sessions to run the variation
	if( ulMaxSessions && ulMaxSessions < 4 )
		return TEST_SKIPPED;

	// Get Session Pointers
	CLEANUP(!CreateSession(IID_IDBCreateCommand,PPI &pCreateCmd1));
	CLEANUP(!CreateSession(IID_IDBCreateCommand,PPI &pCreateCmd2));
	CLEANUP(!CreateSession(IID_IDBCreateCommand,PPI &pCreateCmd3));

	// Get Command Pointers
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,pCreateCmd1));
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd2,pCreateCmd2));
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd3,pCreateCmd3))

	// First session object's command
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,PPI &pUnk1),S_OK));

	//If the provider is read only or does not understand SQL, skip variation.
	if( !g_fReadOnlyProvider && m_pTable->GetSQLSupport() != DBPROPVAL_SQL_NONE )
	{
		// Second session object's command
		CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
		CLEANUP(!CHECK(m_hr = Execute(pICmd2,NULL,NULL,NULL,NULL,IID_NULL),S_OK));
		m_pTable->AddRow();
	}

	// Third session object's command
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd3,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd3,PPI &pUnk3),S_OK));

	TPASS

CLEANUP:	
	SAFE_RELEASE(pCreateCmd1);
	SAFE_RELEASE(pCreateCmd2);
	SAFE_RELEASE(pCreateCmd3);
	SAFE_RELEASE(pICmd1);		
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pICmd3);
	SAFE_RELEASE(pUnk1);
	SAFE_RELEASE(pUnk3);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK:Multiple Commands Open
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_6()
{
	TESTBEGIN		

	IUnknown *	pUnk1=NULL;
	IUnknown *	pUnk3=NULL;
	ICommand * pICmd1=NULL;
	ICommand * pICmd2=NULL;
	ICommand * pICmd3=NULL;

 
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd2,NULL));
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd3,NULL));

	if(!pICmd2 || !pICmd2 || !pICmd3)
		goto CLEANUP;

	// First session object's command
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,PPI &pUnk1),S_OK));

	//If the provider is read only or does not understand SQL, skip variation.
	if( !g_fReadOnlyProvider && m_pTable->GetSQLSupport() != DBPROPVAL_SQL_NONE )
	{
		// Second session object's command
		CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
		CLEANUP(!CHECK(m_hr = Execute(pICmd2,NULL,NULL,NULL,NULL,IID_NULL),S_OK));
		m_pTable->AddRow();
	}

	// Third session object's command
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd3,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd3,PPI &pUnk3),S_OK));

	TPASS;

CLEANUP:	

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pICmd3);

	SAFE_RELEASE(pUnk1);
	SAFE_RELEASE(pUnk3);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: 1 cmd obj (3 inserts on their own threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_7()
{
	ICommand *	pICmd1=NULL;
	HRESULT hrExecute1=S_OK;
	HRESULT hrExecuteOr=S_OK;
	STATEMENTKIND eStatementKind = eINSERT;
	UWORD ThreadNum=0;
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	// Make sure the threading model is correct for this variation.  We only
	// support Free threaded.
	if (!VerifyThreadingModel(COINIT_MULTITHREADED))
		return TEST_SKIPPED;

 	TESTBEGIN
	INIT_THRDS(THREE_THRDS);

	// Get a command object.
	CLEANUP(!GetCommand(IID_ICommand, PPI &pICmd1,NULL));

	// Initialize the thread function argument.
	{
		THRDARG ExecuteFirstCmd = { this, pICmd1, &hrExecute1, &eStatementKind, &hrExecuteOr };

		// Set text in command object
		CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)))				
		//Create Thread
		CREATE_THRD(THRD_ONE,Execute,&ExecuteFirstCmd);
		CREATE_THRD(THRD_TWO,Execute,&ExecuteFirstCmd);
		CREATE_THRD(THRD_THREE,Execute,&ExecuteFirstCmd);
	}

	//Start Thread
	START_THRDS();
	// End Thread
	END_THRDS();

	// Verify the rows got inserted.  Note they violate the automaketable row definitions since they're
	// all the same.
	TESTC(VerifyAndRemoveRows(m_pTable, m_pTable->GetNextRowNumber(), THREE_THRDS));

	// Since Delete thinks it removed one row from the table but the insert never added one 
	// we need to adjust here
	m_pTable->AddRow();

	TPASS;
	
CLEANUP:
	
	SAFE_RELEASE(pICmd1);

	TRETURN;

}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Query set only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_8()
{
	ICommand*	pICmd1=NULL;
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL,NULL,NULL,NULL,IID_NULL),S_OK));
	m_pTable->AddRow();

	TPASS;

CLEANUP:

	SAFE_RELEASE(pICmd1);
			
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: no command set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_9()
{
	ICommand*	pICmd1=NULL;
	IUnknown *	pUnk=NULL;
	
  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,PPI &pUnk),DB_E_NOCOMMAND));
	
	TPASS;
	
CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_10()
{
	
	ICommand*	pICmd1=NULL;
	IUnknown *	pUnk=NULL;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	// Set command text to select statement with no table name
	TEST_SUPPORTED(m_hr = m_pTable->ExecuteCommand(SELECT_NO_TABLE, IID_IUnknown,
		NULL,NULL,NULL, NULL, EXECUTE_NEVER, 0, NULL, 
		NULL, NULL, &pICmd1), S_OK);

	TEST2C_(m_hr = Execute(pICmd1,PPI &pUnk),DB_E_ERRORSINCOMMAND, DB_E_NOTABLE);
	
	TPASS
	
CLEANUP:
	
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pUnk);
	
	TRETURN;
}
// }}



// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_11()
{
	
	ICommand*	pICmd1=NULL;
	IUnknown *	pUnk=NULL;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	// Set command text to select statement with no table name
	TEST_SUPPORTED(m_hr = m_pTable->ExecuteCommand(INSERT_NO_TABLE, IID_IUnknown,
		NULL,NULL,NULL, NULL, EXECUTE_NEVER, 0, NULL, 
		NULL, NULL, &pICmd1), S_OK);

	TEST2C_(m_hr = Execute(pICmd1,PPI &pUnk),DB_E_ERRORSINCOMMAND, DB_E_NOTABLE);
	
	TPASS
	
CLEANUP:
	
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pUnk);
	
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(12)
//--------------------------------------------------------------------
// @mfunc DB_INVALIDARG - cParamSets > 1 and ppRowset was not a null pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_12()
{
	odtLog << L"No longer necessary due to multiple result sets\n";
	// Delete this variation.
	return TEST_PASS;
}	

// {{ TCW_VAR_PROTOTYPE(13)
//--------------------------------------------------------------------
// @mfunc DB_E_MULTIPLESTATEMENTS - command contained multiple statements
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_13()
{
  	TESTBEGIN;

	ICommand* pICmd1=NULL;
	ICommandText * pICmdTxt = NULL;
	IRowset * pRowset=INVALID(IRowset *);
	IRow * pRow=INVALID(IRow *);

	WCHAR * pTempSQL=NULL;
	WCHAR * pSQL=NULL;
	HRESULT Exphr=S_OK; 
	ULONG_PTR ulMultipleResults=0;
	DBORDINAL cRowsetCols;
	DBORDINAL * rgTableColOrds = NULL;
	HRESULT hrSetProp = E_FAIL;

	// Check for IMultipleResults support
	GetProperty(DBPROP_MULTIPLERESULTS, DBPROPSET_DATASOURCEINFO, 
										m_pMyIDBInitialize, &ulMultipleResults);
	

	CLEANUP(FAILED(m_hr=m_pTable->CreateSQLStmt(SELECT_VALIDATIONORDER,NULL,&pTempSQL,&cRowsetCols,(DB_LORDINAL **)&rgTableColOrds)));
	pSQL = (WCHAR *) m_pIMalloc->Alloc(((wcslen(pTempSQL) + wcslen(pTempSQL) + 1) * sizeof(WCHAR)) + sizeof(WCHAR));
	CLEANUP(!pSQL);

	wcscpy(pSQL, pTempSQL);
	wcscat(pSQL, wszSemicolon);
	wcscat(pSQL, pTempSQL);
	
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSQL,NO_QUERY,pSQL)));

	TESTC(VerifyInterface(pICmd1, IID_ICommandText,
					COMMAND_INTERFACE, (IUnknown**)&pICmdTxt));

	// If the provider supports the DBPROP_IMultipleResults property make sure it's off
	SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_IMultipleResults, FALSE);

	// If the property above is not settable then it might still be on.
	if (GetProperty(DBPROP_IMultipleResults, DBPROPSET_ROWSET, pICmd1))
	{
		odtLog << L"Couldn't turn off DBPROP_IMultipleResults property and so can't generate error.\n";
		TSKIPPED;
		goto CLEANUP;
	}

	// Can't validate data at this time for row objects w/o ini file (if any extra row object columns)
	if (IFROW && !g_fLuxor && !GetModInfo()->GetFileName())
	{
		odtLog << L"Can't verify row object data w/o ini file at this time.\n";
		TSKIPPED;
		goto CLEANUP;
	}

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on.  Can't turn on IRowsetLocate
	// for batch SQL as Sql Server driver will turn back off when it detects batch stmt.
	// If the property is not supported then this may fail, but there's nothing we can do about that
	if (g_fKagera && SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	// Spec change: Providers are allowed to return DB_E_ERRORSINCOMMAND rather than DB_E_MULTIPLESTATEMENTS here
	// Spec change: Providers are allowed to return S_OK and the first result set, discarding
	// any further results.
	m_hr = pICmd1->Execute(NULL,IID_IRowset,NULL,NULL,PPI &pRowset);

	// Allow S_OK 
	if (S_OK == m_hr)
	{
		// Make sure we have a rowset
		if (!COMPARE(pRowset != NULL, TRUE))
			goto CLEANUP;
		
		// Validate data
		TESTC_(VerifyRowset(IID_IRowset, (IUnknown *)pRowset, 1, 
			cRowsetCols, rgTableColOrds, FALSE), S_OK);
		SAFE_RELEASE(pRowset);
	}
	else
	{
		// Allow DB_E_ERRORSINCOMMAND or DB_E_NOTABLE
		TEST2C_(m_hr, DB_E_ERRORSINCOMMAND, DB_E_NOTABLE);
		COMPARE(pRowset, NULL);
		goto CLEANUP;
	}

	if(IFROW)
	{
//		m_hr = pICmd1->Execute(NULL,IID_IRow,NULL,NULL,PPI &pRow);
		m_hr = pICmdTxt->Execute(NULL,IID_IRow,NULL,NULL,PPI &pRow);

		// Allow S_OK 
		if (S_OK == m_hr)
		{
			// Make sure we have a rowset
			if (!COMPARE(pRow != NULL, TRUE))
				goto CLEANUP;
			
			// Validate data
			// Note this will fail if not using an ini file if there are additional
			// row object columns beyond the last one.  In that case just limit the
			// compare to those that we can match w/o ini file.
			TESTC_(VerifyRowObj(IID_IRow, (IUnknown *)pRow, 1, 
				cRowsetCols, rgTableColOrds, FALSE), S_OK);
			SAFE_RELEASE(pRow);

		}
		else
		{
			// Allow DB_E_ERRORSINCOMMAND or DB_E_NOTABLE
			TEST2C_(m_hr, DB_E_ERRORSINCOMMAND, DB_E_NOTABLE);
			COMPARE(pRow, NULL);
			goto CLEANUP;
		}
	}

	TPASS;
	
CLEANUP:

	if(pRowset != INVALID(IRowset *))
		SAFE_RELEASE(pRowset);
	if(pRow != INVALID(IRow *))
		SAFE_RELEASE(pRow);

	if (hrSetProp == S_OK)
		CHECK(SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM, DBPROPOPTIONS_OPTIONAL), S_OK);

	PROVIDER_FREE(pSQL);
	PROVIDER_FREE(pTempSQL);
	PROVIDER_FREE(rgTableColOrds);

	SAFE_RELEASE(pICmdTxt);
	SAFE_RELEASE(pICmd1);
		
	
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(14)
//--------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURED - property  was not set
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_14()
{
	TESTRESULT fResult = TEST_FAIL;
	ICommand*	pICmd1=NULL;
	IRowset *	pIRowset=NULL;
	IUnknown*	pIUnk = NULL;
	IRowsetInfo * pIRowsetInfo=NULL;
	IDBProperties * pIDBProperties = NULL;
	ICommandProperties * pICommandProperties = NULL;
	DBPROPINFO * pPropInfo = NULL;
	DBPROPIDSET rgPropertyIDSets[1];
	DBPROPINFOSET * pPropertyInfoSets = NULL;
	DBPROPSET * pPropertySets = NULL;
	DBPROPSET * pPropertySetsReq = NULL;
	DBPROPSET * pPropertySetsReqCopy = NULL;
	WCHAR *	pDescBuffer = NULL;
	ULONG cPropertyInfoSets = 0;
	ULONG cPropertySetsReq = 0;
	ULONG cPropertySets = 0;
	ULONG iProp, iProp2;
	ULONG iSet, iSet2;
	BOOL fPropFound = FALSE;
	HRESULT hr = E_FAIL;
	
	rgPropertyIDSets[0].rgPropertyIDs = NULL;
	rgPropertyIDSets[0].cPropertyIDs = NULL;
	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_ROWSETALL;

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	// Get an IDBProperties interface
	TESTC(VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBProperties,
					DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties));

	// Get an ICommandProperties interface
	TESTC(VerifyInterface(pICmd1, IID_ICommandProperties,
					COMMAND_INTERFACE, (IUnknown**)&pICommandProperties));

	// Save the existing command properties so we can put them back
	TESTC_(pICommandProperties->GetProperties(0, NULL, &cPropertySets, &pPropertySets), S_OK);

	// Set the command text
	TESTC_(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL), S_OK);

//	odtLog << L"\n\nBefore successful Execute call:\n";
//	DumpCommandProps(pICmd1, FALSE, cPropertySets, pPropertySets);

	TESTC_(hr = pICmd1->Execute(NULL,IID_IRowsetInfo,NULL,NULL,PPI &pIRowsetInfo), S_OK);

//	odtLog << L"\n\nAfter successful Execute call:\n";
//	DumpCommandProps(pICmd1, FALSE, cPropertySets, pPropertySets);

	// Get a copy of the existing properties we can modify
	TESTC_(pIRowsetInfo->GetProperties(0, NULL, &cPropertySetsReq, &pPropertySetsReq), S_OK);

//	odtLog << L"\n\nFrom IRowsetInfo:\n";
//	DumpCommandProps(pICmd1, FALSE, cPropertySetsReq, pPropertySetsReq);

	SAFE_RELEASE(pIRowsetInfo);

	// Make a copy of the prop array from the rowset.  We have to remove nonsettable props
	// or change them to their default values.  Set to default.
	for (iSet = 0; iSet < cPropertySetsReq; iSet++)
	{
		ULONG cProps = pPropertySetsReq[iSet].cProperties;

		for (iProp=0; iProp < cProps ; iProp++)
		{

			// Set all settable props required except DBPROP_IRow and IMultipleResults
			// since those props create a non-rowset object.
			/*
			if (pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID != DBPROP_IRow &&
				pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID != DBPROP_IMultipleResults &&
				SettableProperty(pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID,
				pPropertySetsReq[iSet].guidPropertySet, m_pIDBInitialize))
			*/

//			if (pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID == DBPROP_IRowset ||
//				pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID == DBPROP_IRow)
//				odtLog << L"Props found\n";

			// If we want a row object back, make sure we don't require IRowset TRUE, as this will
			// conflict with IID_IRow
			if (IFROW && pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID == DBPROP_IRowset)
				V_BOOL(&pPropertySetsReq[iSet].rgProperties[iProp].vValue) = VARIANT_FALSE;

			// If we want a row object back, make sure we require IRow TRUE, otherwise if it is 
			// FALSE by default then we'll end up with it REQUIRED FALSE and be unable to get
			// a row object, since some providers have IRowset non settable TRUE as the default
			if (IFROW && pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID == DBPROP_IRow)
				V_BOOL(&pPropertySetsReq[iSet].rgProperties[iProp].vValue) = VARIANT_TRUE;

			// If we want a rowset object back, make sure we don't require IRow TRUE, as this will
			// conflict with IID_IRowset
			if (!IFROW && pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID == DBPROP_IRow)
				V_BOOL(&pPropertySetsReq[iSet].rgProperties[iProp].vValue) = VARIANT_FALSE;

			// Luxor needs a server cursor for a row object if access order is random.  Other providers
			// may not have this limitation
			if (IFROW && g_fLuxor && pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID == DBPROP_SERVERCURSOR)
				V_BOOL(&pPropertySetsReq[iSet].rgProperties[iProp].vValue) = VARIANT_TRUE;
				
			if (SettableProperty(pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID,
				pPropertySetsReq[iSet].guidPropertySet, m_pIDBInitialize))
			{
				// Make REQUIRED
				pPropertySetsReq[iSet].rgProperties[iProp].dwOptions = DBPROPOPTIONS_REQUIRED;
			}
			else
			{
				// Set to VT_EMPTY if not settable to force default
				VariantClear(&pPropertySetsReq[iSet].rgProperties[iProp].vValue);

				// Have to set this property OPTIONAL because the default value set as REQUIRED may
				// preclude successfully getting a ROW object, since the provider won't be able to
				// roll the prop
				pPropertySetsReq[iSet].rgProperties[iProp].dwOptions = DBPROPOPTIONS_OPTIONAL;
			}
		}
	}

//	odtLog << L"Modified props\n";
//	DumpCommandProps(pICmd1, FALSE, cPropertySetsReq, pPropertySetsReq);

	// Make another copy of the prop array, because some providers muck with my array
	SAFE_ALLOC(pPropertySetsReqCopy, DBPROPSET, cPropertySetsReq * sizeof(DBPROPSET));
	memset(pPropertySetsReqCopy, 0, cPropertySetsReq * sizeof(DBPROPSET));
	for (iSet = 0; iSet < cPropertySetsReq; iSet++)
	{
		pPropertySetsReqCopy[iSet].guidPropertySet = pPropertySetsReq[iSet].guidPropertySet;
		pPropertySetsReqCopy[iSet].cProperties = pPropertySetsReq[iSet].cProperties;
		SAFE_ALLOC(pPropertySetsReqCopy[iSet].rgProperties , DBPROP, 
			pPropertySetsReq[iSet].cProperties * sizeof(DBPROP));
		memcpy(pPropertySetsReqCopy[iSet].rgProperties, pPropertySetsReq[iSet].rgProperties,
			pPropertySetsReq[iSet].cProperties * sizeof(DBPROP));
	}

	// Retrieve all rowset properties
	TESTC_(pIDBProperties->GetPropertyInfo(1, rgPropertyIDSets, &cPropertyInfoSets, &pPropertyInfoSets,
		&pDescBuffer), S_OK);

	hr = pICommandProperties->SetProperties(cPropertySetsReq, pPropertySetsReq);

	if (S_OK != hr)
		DumpCommandProps(pICmd1, FALSE, cPropertySetsReq, pPropertySetsReq);
	
	TESTC_(hr, S_OK);

	// Now make sure the provider didn't muck with the properties
	for (iSet = 0; iSet < cPropertySetsReq; iSet++)
	{	
		for (iProp=0; iProp < pPropertySetsReq[iSet].cProperties; iProp++)
		{
			BOOL fCmp = TRUE;
			LONG lCmp;

			// Compare the option
			fCmp &= COMPARE(pPropertySetsReq[iSet].rgProperties[iProp].dwOptions,
				pPropertySetsReqCopy[iSet].rgProperties[iProp].dwOptions);

			// Compare the propid
			fCmp &= COMPARE(pPropertySetsReq[iSet].rgProperties[iProp].dwPropertyID,
				pPropertySetsReqCopy[iSet].rgProperties[iProp].dwPropertyID);

			// Compare the value
			lCmp = memcmp(&pPropertySetsReq[iSet].rgProperties[iProp].vValue,
				&pPropertySetsReqCopy[iSet].rgProperties[iProp].vValue, sizeof(VARIANT));

			fCmp &= COMPARE(lCmp, 0);

			if (!fCmp)
				odtLog << L"Error: Provider changed properties passed to SetProperties!\n";		
		}
	}

	// Make sure I can still open a rowset or row (props should have no effect on row obj).
	hr = Execute(pICmd1,PPI &pIUnk);

	if (S_OK != hr)
	{
		// Dump current command props
		DumpCommandProps(pICmd1, FALSE);
		// Dump properties in error
		DumpCommandProps(pICmd1, TRUE);
	}

	TESTC_(hr, S_OK);

	SAFE_RELEASE(pIUnk);

	// All the setup code worked.  Now assume success until we know otherwise
	fResult = TEST_PASS;

	// Go through properties until we find one
	// that returns DB_E_ERRORSOCCURRED when set REQUIRED
	for (iSet = 0; iSet < cPropertyInfoSets && !fPropFound; iSet++)
	{
		for (iProp=0; iProp < pPropertyInfoSets[iSet].cPropertyInfos && !fPropFound; iProp++)
		{
			if (!SetPropertyAndExecute(pICmd1, pPropertyInfoSets, iSet, iProp,
				SINGLE_PROP, NULL, &fPropFound))
				fResult = TEST_FAIL;

			if (fPropFound)
				// Set the property back off since we set it on in SetPropertyAndExecute
				SetRowsetProperty(pICmd1,pPropertyInfoSets[iSet].guidPropertySet,
					pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID, FALSE,
					DBPROPOPTIONS_REQUIRED, FALSE);
		}
	}

	if (!fPropFound)
	{
		odtLog << L"Couldn't find a set of properties that will return DB_S_ERRORSOCCURRED.\n";
		fResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	// If we made it this far we know that there is at least one combination of properties that will result
	// in DB_S_ERRORSOCCURRED, otherwise we tried setting them all TRUE REQUIRED and didn't find any that will.
	// Now we check for other combinations

	// Set the original properties required again.  We must first set all the props
	// to their default values before attempting to set our blob of props due to
	// provider-specific order of processing, that is, the provider may return the props
	// to us in any order and may find a prop conflicting with this set even though the set 
	// itself is consistent and valid.

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICommandProperties);

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	// Get an ICommandProperties interface
	TESTC(VerifyInterface(pICmd1, IID_ICommandProperties,
					COMMAND_INTERFACE, (IUnknown**)&pICommandProperties));

	// Set the command text again
	TESTC_(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL), S_OK);

	// Set the original properties required again.
	TESTC_(pICommandProperties->SetProperties(cPropertySetsReq, pPropertySetsReq), S_OK);

	// Now set additional properties one at a time
	for (iSet = 0; iSet < cPropertyInfoSets; iSet++)
	{
		for (iProp=0; iProp < pPropertyInfoSets[iSet].cPropertyInfos; iProp++)
		{
			TESTC_(pICommandProperties->SetProperties(cPropertySetsReq, pPropertySetsReq), S_OK);

			// If the property is already on then skip it
			if (GetProperty(pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID,
				pPropertyInfoSets[iSet].guidPropertySet, pICommandProperties))
				continue;

			if (!SetPropertyAndExecute(pICmd1, pPropertyInfoSets, iSet, iProp,
				SINGLE_PROP, NULL, NULL))
				fResult = TEST_FAIL;
		}
	}

	//End variation here if its a row object we are testing.
	if(IFROW)
	{
		fResult = TEST_PASS;
		goto CLEANUP;
	}

	// Now set them in pairs to find a combination that gives DB_E_ERRORSOCCURRED.
	for (iSet = 0; iSet < cPropertyInfoSets; iSet++)
	{
		for (iProp=0; iProp < pPropertyInfoSets[iSet].cPropertyInfos; iProp++)
		{
			// Put the original properties back
			TESTC_(pICommandProperties->SetProperties(cPropertySetsReq, pPropertySetsReq), S_OK);

			// If the property is not supported or not writable just skip it
			if (pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwFlags == DBPROPFLAGS_NOTSUPPORTED	||
				!(pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwFlags & DBPROPFLAGS_WRITE))
				continue;

			// Set property on
			SetRowsetProperty(pICmd1,pPropertyInfoSets[iSet].guidPropertySet,
				pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID, TRUE,
				DBPROPOPTIONS_REQUIRED, FALSE);

			// If this property causes Execute to fail then we don't want it
			hr = pICmd1->Execute(NULL,IID_IRowset,NULL,NULL,PPI &pIRowset);

			SAFE_RELEASE(pIRowset);

			// Set property back off
			SetRowsetProperty(pICmd1,pPropertyInfoSets[iSet].guidPropertySet,
				pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID, FALSE,
				DBPROPOPTIONS_REQUIRED, FALSE);

			if (S_OK != hr)
			{
				// This is the expected failure if required property can't be supported on rowset
				// All single property values were tested previously and errors were posted above.
				// Don't post them again
				// CHECK(hr, DB_E_ERRORSOCCURRED);
				continue;
			}

			for (iSet2 = 0; iSet2 < cPropertyInfoSets; iSet2++)
			{
				for (iProp2=0; iProp2 < pPropertyInfoSets[iSet2].cPropertyInfos; iProp2++)
				{
					// Don't bother if it's the same property.  Some providers will return
					// DB_S_ERRORSOCCURRED at Execute any time the same property is set twice.
					if (iSet == iSet2 && iProp == iProp2)
						continue;

					// Put the original properties back REQUIRED
					TESTC_(pICommandProperties->SetProperties(cPropertySetsReq, pPropertySetsReq), S_OK);

					// If the property is not supported or not writable just skip it
					if (pPropertyInfoSets[iSet2].rgPropertyInfos[iProp2].dwFlags == DBPROPFLAGS_NOTSUPPORTED	||
						!(pPropertyInfoSets[iSet2].rgPropertyInfos[iProp2].dwFlags & DBPROPFLAGS_WRITE))
						continue;

					// If the second property is already set on then skip it
					if (GetProperty(pPropertyInfoSets[iSet2].rgPropertyInfos[iProp2].dwPropertyID,
						pPropertyInfoSets[iSet2].guidPropertySet, pICommandProperties))
						continue;

					// Set second property on
					SetRowsetProperty(pICmd1,pPropertyInfoSets[iSet2].guidPropertySet,
						pPropertyInfoSets[iSet2].rgPropertyInfos[iProp2].dwPropertyID, TRUE,
						DBPROPOPTIONS_REQUIRED, FALSE);

					// If this property causes Execute to fail then we don't want it
					hr = pICmd1->Execute(NULL,IID_IRowset,NULL,NULL,PPI &pIRowset);

					SAFE_RELEASE(pIRowset);

					if (S_OK != hr)
					{
						// This is the expected failure if required property can't be supported on rowset
						// All single property values were tested previously and errors were posted above.
						// Don't post them again
						// CHECK(hr, DB_E_ERRORSOCCURRED);

						// Set property back off
						SetRowsetProperty(pICmd1,pPropertyInfoSets[iSet2].guidPropertySet,
							pPropertyInfoSets[iSet2].rgPropertyInfos[iProp2].dwPropertyID, FALSE,
							DBPROPOPTIONS_REQUIRED, FALSE);

						continue;
					}
					
					// Put the original properties back REQUIRED
					TESTC_(pICommandProperties->SetProperties(cPropertySetsReq, pPropertySetsReq), S_OK);

					// Set one property on
					SetRowsetProperty(pICmd1,pPropertyInfoSets[iSet].guidPropertySet,
						pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID, TRUE,
						DBPROPOPTIONS_REQUIRED, FALSE);

					if (!SetPropertyAndExecute(pICmd1, pPropertyInfoSets, iSet2, iProp2,
						PAIRED_PROP, pPropertyInfoSets[iSet].rgPropertyInfos[iProp].pwszDescription, NULL))
						fResult = TEST_FAIL;

					// Set properties back off REQUIRED
					SetRowsetProperty(pICmd1,pPropertyInfoSets[iSet].guidPropertySet,
						pPropertyInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID, FALSE,
						DBPROPOPTIONS_REQUIRED, FALSE);

					SetRowsetProperty(pICmd1,pPropertyInfoSets[iSet2].guidPropertySet,
						pPropertyInfoSets[iSet2].rgPropertyInfos[iProp2].dwPropertyID, FALSE,
						DBPROPOPTIONS_REQUIRED, FALSE);

				}
			}

		}

	}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pIRowsetInfo);

	// Put the properties back
	CHECK(pICommandProperties->SetProperties(cPropertySets, pPropertySets), S_OK);

	// Free the copies of the properties.  We can't call FreeProperties because we didn't 
	// make a copy of bstr values, they're the same as the original
	for (iSet = 0; iSet < cPropertySetsReq; iSet++)
		SAFE_FREE(pPropertySetsReqCopy[iSet].rgProperties);
	SAFE_FREE(pPropertySetsReqCopy);

	// Free the property sets
	FreeProperties(&cPropertyInfoSets, &pPropertyInfoSets, &pDescBuffer);
	FreeProperties(&cPropertySetsReq, &pPropertySetsReq);
	FreeProperties(&cPropertySets, &pPropertySets);

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pICommandProperties);
		
	return fResult;
}
// }}

// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Insert, riid=IID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_15()
{
	DBROWCOUNT cRowsAffected=0;
	ICommand*	pICmd1=NULL;
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;
  	
	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL,&cRowsAffected,NULL,NULL,IID_NULL),S_OK));
	TEST_COMPARE(cRowsAffected, 1);
	m_pTable->AddRow();

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Insert, riid=IID_IRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_16()
{
	DBROWCOUNT	cRowsAffected=0;
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnk=INVALID(IUnknown *);

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1, PPI &pIUnk, &cRowsAffected),S_OK));
	TEST_COMPARE(cRowsAffected, 1);
	// Rowset pointer should be nulled out.
	TEST_COMPARE(pIUnk, NULL);
	m_pTable->AddRow();

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	if(pIUnk != INVALID(IUnknown *))
		SAFE_RELEASE(pIUnk);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Insert, riid=IID_IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_17()
{
	DBROWCOUNT	cRowsAffected=0;
	ICommand*	pICmd1=NULL;
	IRowsetInfo *pIRowsetInfo=INVALID(IRowsetInfo *);

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	ONLYROWSETVAR;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IRowsetInfo,NULL,&cRowsAffected, (IUnknown **)&pIRowsetInfo),S_OK));
	
	m_pTable->AddRow();
	TEST_COMPARE(cRowsAffected, 1);

	// Is pIRowsetInfo nulled out.
	if (!pIRowsetInfo)
		TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	if(pIRowsetInfo != INVALID(IRowsetInfo *))
		SAFE_RELEASE(pIRowsetInfo);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Select, riid=IID_IRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_18()
{
	ICommand*	pICmd1=NULL;
	IRowset* pIRowset=NULL;
	DBORDINAL cRowsetCols;
	DBORDINAL * rgTableColOrds = NULL;
	
	ONLYROWSETVAR;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC_(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset, NULL,
		NULL, &cRowsetCols, (DB_LORDINAL **)&rgTableColOrds, EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIRowset,
		&pICmd1), S_OK);
	
	TESTC(pIRowset != NULL);

	TESTC_(VerifyRowset(IID_IRowset, (IUnknown *)pIRowset, 1, 
		cRowsetCols, rgTableColOrds, FALSE), S_OK);

	TPASS;

CLEANUP:
	SAFE_FREE(rgTableColOrds);
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIRowset);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Select, riid=IID_IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_19()
{
	ICommand*	pICmd1=NULL;
	IRowsetInfo* pIRowsetInfo=NULL;

	ONLYROWSETVAR;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	TEST_CHECK(pICmd1->Execute(NULL,IID_IRowsetInfo,NULL,NULL,PPI &pIRowsetInfo),S_OK);

	// We do expect a rowset back.
	if (pIRowsetInfo)
		TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIRowsetInfo);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(20)
//--------------------------------------------------------------------
// @mfunc E_NOINTERACE: dso iid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_20()
{
	ICommand* pICmd1=NULL;
	IUnknown*  pIUnk=INVALID(IUnknown *);

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	TEST_CHECK(pICmd1->Execute(NULL,IID_IDBProperties,NULL,NULL,PPI &pIUnk),E_NOINTERFACE);

	if(!pIUnk)
		TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
    if(pIUnk != INVALID(IUnknown*))
		SAFE_RELEASE(pIUnk);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(21)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=0, valid pIRowset (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_21()
{
	DBROWCOUNT cRowsAffected=0;
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnk=NULL;
	
  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1, PPI &pIUnk, &cRowsAffected),S_OK));

	COMPARE(cRowsAffected==DB_COUNTUNAVAILABLE ||
		(ULONG)cRowsAffected==m_pTable->GetRowsOnCTable(), TRUE);

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnk);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=1, valid pIRowset (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_22()
{
	DBROWCOUNT cRowsAffected=1;
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnk=NULL;
	
  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1, PPI &pIUnk, &cRowsAffected),S_OK));

	COMPARE(cRowsAffected==DB_COUNTUNAVAILABLE ||
		(ULONG)cRowsAffected==m_pTable->GetRowsOnCTable(), TRUE);

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnk);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(23)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=0, valid pIRowset (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_23()
{
	DBROWCOUNT cRowsAffected=0;
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnk=INVALID(IUnknown *);
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1, PPI &pIUnk, &cRowsAffected),S_OK));
	m_pTable->AddRow();

	TEST_COMPARE(cRowsAffected, 1);

	if(!pIUnk)
		TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	if(pIUnk != INVALID(IUnknown *))
		SAFE_RELEASE(pIUnk);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(24)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=1, valid pIRowset (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_24()
{
	DBROWCOUNT cRowsAffected=1;
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnk=INVALID(IUnknown *);
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1, PPI &pIUnk, &cRowsAffected),S_OK));
	m_pTable->AddRow();

	TEST_COMPARE(cRowsAffected, 1);

	if(!pIUnk)
		TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	if(pIUnk != INVALID(IUnknown *))
		SAFE_RELEASE(pIUnk);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(25)
//--------------------------------------------------------------------
// @mfunc S_OK:NULL pcRowsAffected, valid pIRowset (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_25()
{
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnk=NULL;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1, PPI &pIUnk),S_OK));

	if (pIUnk)
	{
		TPASS;
	}

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnk);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(26)
//--------------------------------------------------------------------
// @mfunc E_INVALIDARG:NULL pcRowsAffected, NULL pIRowset (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_26()
{
	ICommand*	pICmd1=NULL;
	
  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1, NULL),E_INVALIDARG));

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(27)
//--------------------------------------------------------------------
// @mfunc S_OK:NULL pcRowsAffected, valid pIRowset (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_27()
{
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnk=NULL;
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,PPI &pIUnk),S_OK));
	m_pTable->AddRow();

	if (!pIUnk)
		TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnk);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(28)
//--------------------------------------------------------------------
// @mfunc S_OK:NULL pcRowsAffected, NULL pIRowset (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_28()
{
	ICommand*	pICmd1=NULL;
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL,NULL,NULL,NULL,IID_NULL),S_OK));
	m_pTable->AddRow();

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(29)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=0, valid pIRowsetLocate (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_29()
{
	DBROWCOUNT cRowsAffected=0;
	ICommand*	pICmd1=NULL;
	IRowsetLocate * pIRowsetLocate=NULL;

	ONLYROWSETVAR;

 	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));

	// Make sure the property is supported and settable before proceeding
	if(FAILED(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetLocate, TRUE, DBPROPOPTIONS_REQUIRED, FALSE)))
	{
		odtLog << L"IRowsetLocate is not supported by this provider.\n";
  		TSKIPPED;
	}

	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IRowsetLocate,NULL,&cRowsAffected,PPI &pIRowsetLocate),S_OK));

	if (pIRowsetLocate)
		TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIRowsetLocate);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(30)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=1, valid pIRowsetLocate (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_30()
{
	DBROWCOUNT cRowsAffected=1;
	ICommand*	pICmd1=NULL;
	IRowsetLocate * pIRowsetLocate=NULL;
	
	ONLYROWSETVAR;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));

	// Make sure the property is supported and settable before proceeding
	if(FAILED(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetLocate, TRUE, DBPROPOPTIONS_REQUIRED, FALSE)))
	{
		odtLog << L"IRowsetLocate is not supported by this provider.\n";
  		TSKIPPED;
	}

	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IRowsetLocate,NULL,&cRowsAffected,PPI &pIRowsetLocate),S_OK));

	if (pIRowsetLocate)
		TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIRowsetLocate);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=0, valid pIRowsetLocate (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_31()
{
	DBROWCOUNT cRowsAffected=1;
	ICommand*	pICmd1=NULL;
	IRowsetLocate * pIRowsetLocate=INVALID(IRowsetLocate *);

	ONLYROWSETVAR;

  	TESTBEGIN;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	// Make sure the property is supported and settable before proceeding
	if(FAILED(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetLocate, TRUE, DBPROPOPTIONS_REQUIRED, FALSE)))
	{
		odtLog << L"IRowsetLocate is not supported by this provider.\n";
  		TSKIPPED;
	}

	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IRowsetLocate,NULL,&cRowsAffected,PPI &pIRowsetLocate),S_OK));
	m_pTable->AddRow();

	// Expecting a NULL pIRowsetLocate for an Insert statement.
	if (!pIRowsetLocate)
		TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	// Since we set to a bogus value don't free it if it's still bogus
	if (pIRowsetLocate != INVALID(IRowsetLocate *))
		SAFE_RELEASE(pIRowsetLocate);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(32)
//--------------------------------------------------------------------
// @mfunc S_OK:NULL pcRowsAffected, valid pIRowsetLocate (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_32()
{
	ICommand*	pICmd1=NULL;
	IRowsetLocate * pIRowsetLocate=NULL;
	
	ONLYROWSETVAR;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));

	// Make sure the property is supported and settable before proceeding
	if(FAILED(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetLocate, TRUE, DBPROPOPTIONS_REQUIRED, FALSE)))
	{
		odtLog << L"IRowsetLocate is not supported by this provider.\n";
  		TSKIPPED;
	}

	CLEANUP(!CHECK(pICmd1->Execute(NULL,IID_IRowsetLocate,NULL,NULL,PPI &pIRowsetLocate),S_OK));

	if (pIRowsetLocate)
		TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIRowsetLocate);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(33)
//--------------------------------------------------------------------
// @mfunc E_INVALIDARG:NULL pcRowsAffected, NULL pIRowsetLocate (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_33()
{
	ICommand*	pICmd1=NULL;
	
  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));

	// Make sure the property is supported and settable before proceeding
	if(FAILED(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetLocate, TRUE, DBPROPOPTIONS_REQUIRED, FALSE)))
	{
		odtLog << L"IRowsetLocate is not supported by this provider.\n";
  		TSKIPPED;
	}

	m_hr = pICmd1->Execute(NULL,IID_IRowsetLocate,NULL,NULL,NULL);

	if(IFROWSET)
		TESTC_(m_hr, E_INVALIDARG)
	if(IFROW)
		TEST2C_(m_hr, E_INVALIDARG, E_NOINTERFACE)

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(34)
//--------------------------------------------------------------------
// @mfunc S_OK:NULL pcRowsAffected, valid pIRowsetLocate (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_34()
{
	ICommand*	pICmd1=NULL;
	IRowsetLocate* pIRowsetLocate=NULL;
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	ONLYROWSETVAR;

 	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	// Make sure the property is supported and settable before proceeding
	if(FAILED(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetLocate, TRUE, DBPROPOPTIONS_REQUIRED, FALSE)))
	{
		odtLog << L"IRowsetLocate is not supported by this provider.\n";
  		TSKIPPED;
	}

	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IRowsetLocate,NULL,NULL,PPI &pIRowsetLocate),S_OK));
	m_pTable->AddRow();

	if(!pIRowsetLocate)
		TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIRowsetLocate)
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(35)
//--------------------------------------------------------------------
// @mfunc S_OK:NULL pcRowsAffected, NULL pIRowsetLocate (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_35()
{
	// @todo :: Delete this variation.
	PRVTRACE (L"Bogus Variation\n");
	return TEST_PASS;
}
// }}

// {{ TCW_VAR_PROTOTYPE(36)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=0, valid pIColumnsInfo (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_36()
{
	ICommand*	pICmd1=NULL;
	IColumnsInfo* pIColumnsInfo=NULL;
	DBROWCOUNT cRowsAffected=0;
	EINTERFACE	eIF = ROWSET_INTERFACE;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));

	if(IFROW)
	{
		eIF = ROW_INTERFACE;
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);
	}

	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IColumnsInfo,NULL,&cRowsAffected,PPI &pIColumnsInfo),S_OK));

	TESTC(DefaultObjectTesting(pIColumnsInfo, eIF))

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(37)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=1, valid pIColumnsInfo (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_37()
{
	ICommand*	pICmd1=NULL;
	IColumnsInfo* pIColumnsInfo=NULL;
	DBROWCOUNT cRowsAffected=1;
	EINTERFACE	eIF = ROWSET_INTERFACE;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));

	if(IFROW)
	{
		eIF = ROW_INTERFACE;
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);
	}

	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IColumnsInfo,NULL,&cRowsAffected,PPI &pIColumnsInfo),S_OK));

	TESTC(DefaultObjectTesting(pIColumnsInfo, eIF))

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(38)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=0, valid pIColumnsInfo (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_38()
{
	ICommand*	pICmd1=NULL;
	IColumnsInfo* pIColumnsInfo=NULL;
	DBROWCOUNT cRowsAffected=0;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))

	if(IFROW)
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);

	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IColumnsInfo,NULL,&cRowsAffected,PPI &pIColumnsInfo),S_OK));
	m_pTable->AddRow();

	COMPARE(cRowsAffected, 1);

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(39)
//--------------------------------------------------------------------
// @mfunc S_OK:valid pcRowsAffected, *pc=1, valid pIColumnsInfo (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_39()
{
	ICommand*	pICmd1=NULL;
	IColumnsInfo* pIColumnsInfo=NULL;
	DBROWCOUNT cRowsAffected=1;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))

	if(IFROW)
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);

	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IColumnsInfo,NULL,&cRowsAffected,PPI &pIColumnsInfo),S_OK));
	m_pTable->AddRow();

	COMPARE(cRowsAffected, 1);

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(40)
//--------------------------------------------------------------------
// @mfunc S_OK:NULL pcRowsAffected, valid pIColumnsInfo (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_40()
{
	ICommand*	pICmd1=NULL;
	IColumnsInfo* pIColumnsInfo=NULL;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));

	if(IFROW)
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);

	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IColumnsInfo,NULL,NULL,PPI &pIColumnsInfo),S_OK));

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(41)
//--------------------------------------------------------------------
// @mfunc E_INVALIDARG:NULL pcRowsAffected, NULL pIColumnsInfo (Select)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_41()
{
	ICommand*	pICmd1=NULL;

	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));

	if(IFROW)
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);

	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IColumnsInfo,NULL,NULL,NULL),E_INVALIDARG));

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(42)
//--------------------------------------------------------------------
// @mfunc S_OK:NULL pcRowsAffected, valid pIColumnsInfo (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_42()
{
	ICommand*	pICmd1=NULL;
	IColumnsInfo* pIColumnsInfo=NULL;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	if(IFROW)
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);

	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IColumnsInfo,NULL,NULL,PPI &pIColumnsInfo),S_OK));
	COMPARE(pIColumnsInfo, NULL);
	m_pTable->AddRow();

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(43)
//--------------------------------------------------------------------
// @mfunc S_OK:NULL pcRowsAffected, NULL pIColumnsInfo (Insert)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_43()
{
	ICommand*	pICmd1=NULL;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));
	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK));
	m_pTable->AddRow();

	TPASS

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	TRETURN
}
// }}

	
// {{ TCW_VAR_PROTOTYPE(44)
//--------------------------------------------------------------------
// @mfunc E_INVALIDARG:riid != IID_NULL, ppRowset==NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_44()
{
	ICommand*	pICmd1=NULL;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	if(IFROW)
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);

	CLEANUP(!CHECK(m_hr = Execute(pICmd1,NULL),E_INVALIDARG));

	TPASS

CLEANUP:
	if (SUCCEEDED(m_hr))
		m_pTable->AddRow();
	SAFE_RELEASE(pICmd1);
	TRETURN
}
// {{ TCW_VAR_PROTOTYPE(45)
//--------------------------------------------------------------------
// @mfunc DB_E_CANTCONVERTVALUE: string (22005)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_45()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pwszValue		= NULL;		// String value
	DBORDINAL			pcColumns		= 0;		// Count of columns
	DBLENGTH			ColSize			= 0;		// Column Size
	ULONG				count			= 0;		// Loop counter
	CList				<WCHAR* ,WCHAR*> NativeTypesList;
	CCol				NewCol(m_pIMalloc);			// Class CCol
	IUnknown *			pIUnk=NULL;


	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	// Creates a column list from the Ctable
	pcColumns = m_pTable->CountColumnsOnTable();

	// Loop thru column types
	for( count=1; count <= pcColumns; count++)
	{
		m_pTable->GetColInfo(count, NewCol);
		
		// We want a non-long character column
		if( IsColCharacter(NewCol.GetProviderType()) && !NewCol.GetIsLong())
			break;
	}

	if (count > pcColumns)
	{
		odtLog << L"Couldn't find a non-Long char column for this test.\n";
		return TEST_SKIPPED;
	}

	NativeTypesList.AddHead(NewCol.GetProviderTypeName());

	// Create a table
	if(!CHECK(g_Table3->CreateTable(NativeTypesList,
									1,			// Number of rows to insert
									0,			// Column to put index on
									NULL,		// Table name
									PRIMARY),	// Primary or secondary values
									S_OK))
	{
		// Free memory in the list
		NativeTypesList.RemoveAll();
		return TEST_FAIL;
	}

	// Get the name of the table just created
	pTableName = g_Table3->GetTableName();

	// Get Column Size
	ColSize = NewCol.GetMaxSize();

	// Alloc Memory
	pwszValue= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + 
											 (sizeof(WCHAR)*(ColSize+1)) );

	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidCharValue) + wcslen(pTableName) + ColSize+1)) );

	// Create String value out of range
	wcscpy(pwszValue, L"\0");
	for(count=1; count <= ColSize+1; count++)
		wcscat(pwszValue, L"A");

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidCharValue, pTableName, pwszValue);

	//  Command to return a ICommand with Text Set
	if( !CHECK(g_Table3->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK) )
		goto END;

	// Compare the HRESULT
	if( CHECK(Execute(pICommand, PPI &pIUnk), DB_E_DATAOVERFLOW) )
		fSuccess = TRUE;

END:
	// Free memory in the list
	NativeTypesList.RemoveAll();

	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pwszValue);

	// Drop the table
	g_Table3->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// {{ TCW_VAR_PROTOTYPE(46)
//--------------------------------------------------------------------
// @mfunc DB_E_CANTCONVERTVALUE: implicit (22005)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_46()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pwszNumeric		= NULL;		// Numeric value
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				count			= 0;		// Loop counter
	CList				<DBTYPE, DBTYPE> DBTypeList;
	CCol				NewCol(m_pIMalloc);			// Class CCol
	
	IUnknown *			pIUnk=NULL;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	// Creates a column list from the Ctable
	pcColumns = m_pTable->CountColumnsOnTable();

	// Loop thru column types
	for( count=1; count <= pcColumns; count++)
	{
		m_pTable->GetColInfo(count, NewCol);
		
		// We need an updatable numeric column
		if( IsNumericType(NewCol.GetProviderType()) && NewCol.GetUpdateable())
			break;
	}

	if (count > pcColumns)
	{
		odtLog << L"Couldn't find an updatable numeric column for this test.\n";
		return TEST_SKIPPED;
	}

	DBTypeList.AddHead(NewCol.GetProviderType());

	// Create a table
	if(!CHECK(g_Table3->CreateTable(DBTypeList,
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
	pTableName = g_Table3->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidChar) + wcslen(pTableName))) );

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidChar, pTableName);

	//  Command to return a ICommand with Text Set
	if( !CHECK(g_Table3->BuildCommand(pwszSQLStmt, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK) )
		goto END;

	if( CHECK(Execute(pICommand,PPI &pIUnk),DB_E_CANTCONVERTVALUE) )
		fSuccess = TRUE;

END:
	// Free memory in the list
	DBTypeList.RemoveAll();

	// Free Memory
	if( pwszSQLStmt )
		m_pIMalloc->Free(pwszSQLStmt);

	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE_(pICommand);

	// Drop the table
	g_Table3->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// {{ TCW_VAR_PROTOTYPE(47)
//--------------------------------------------------------------------
// @mfunc DB_E_CANTCONVERTVALUE: implicit (22008)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_47()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pPrefix			= NULL;		// Prefix of DataType
	WCHAR *				pSuffix			= NULL;		// Suffix of DataType
	WCHAR *				pwszDateTime	= NULL;		// DateTime value
	ULONG				ColPrec			= 0;		// Column Precision
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				count			= 0;		// Loop counter
	CList				<DBTYPE, DBTYPE> DBTypeList;
	CCol				NewCol(m_pIMalloc);			// Class CCol
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	// Creates a column list from the Ctable
	pcColumns = m_pTable->CountColumnsOnTable();

	// Loop thru column types
	for( count=1; count <= pcColumns; count++)
	{
		m_pTable->GetColInfo(count, NewCol);
		
		// If first column is already datetime then were done
		if( IsColDateTime(NewCol.GetProviderType()) )
			break;
	}

	if (count > pcColumns)
	{
		odtLog << L"Couldn't find a datetime column for this test.\n";
		return TEST_SKIPPED;
	}

	DBTypeList.AddHead(NewCol.GetProviderType());

	// Create a table
	if(!CHECK(g_Table3->CreateTable(DBTypeList,
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
	pTableName = g_Table3->GetTableName();

	// Get Numeric Precision
	ColPrec = NewCol.GetPrecision();

	// Get DataType Prefix
	pPrefix = NewCol.GetPrefix();

	// Get DataType Suffix
	pSuffix = NewCol.GetSuffix();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + 
							(sizeof(WCHAR) * (wcslen(wszInsertInvalidDateValue) +
											  wcslen(pTableName) + 
											  wcslen(pPrefix) +
											  wcslen(pSuffix) +
											  wcslen(wszInvalidDateTime))) );

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidDateValue, pTableName, pPrefix, wszInvalidDateTime, pSuffix);

	//  Command to return a ICommand with Text Set
	if( CHECK(g_Table3->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_CANTCONVERTVALUE) )
		fSuccess = TRUE;

	// Free memory in the list
	DBTypeList.RemoveAll();

	SAFE_RELEASE_(pICommand)
	PROVIDER_FREE(pwszSQLStmt);
	g_Table3->DropTable();
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// {{ TCW_VAR_PROTOTYPE(48)
//--------------------------------------------------------------------
// @mfunc DB_E_DATAOVERFLOW: math (22003)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_48()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pwszNumeric		= NULL;		// Numeric value
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				ColPrec			= 0;		// Column Precision
	ULONG				count			= 0;		// Loop counter
	CList				<DBTYPE, DBTYPE> DBTypeList;
	CCol				NewCol(m_pIMalloc);			// Class CCol
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	pcColumns = m_pTable->CountColumnsOnTable();

	// Loop thru column types
	for( count=1; count <= pcColumns; count++)
	{
		m_pTable->GetColInfo(count, NewCol);
		
		// If first column is already numeric then were done
		if( (IsColNumWithScale(NewCol.GetProviderType(),NewCol.GetScale())) &&
			(NewCol.GetUpdateable()) )
			break;
	}

	if (count > pcColumns)
	{
		odtLog << L"Couldn't find a numeric column with scale.\n";
		return TEST_SKIPPED;
	}

	DBTypeList.AddHead(NewCol.GetProviderType());

	// Create a table
	if(!CHECK(g_Table3->CreateTable(DBTypeList,
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
	pTableName = g_Table3->GetTableName();

	// Get Numeric Precision
	ColPrec = NewCol.GetPrecision();

	// Alloc Memory
	pwszNumeric= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + 
											 (sizeof(WCHAR)*(ColPrec+1)) );

	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidValue) + wcslen(pTableName) + ColPrec+1)) );

	// Create Numeric Valus out of range
	wcscpy(pwszNumeric, L"\0");
	for(count=1; count <= ColPrec+1; count++)
		wcscat(pwszNumeric, L"9");

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidValue, pTableName, pwszNumeric);

	//  Command to return a ICommand with Text Set
	if( CHECK(g_Table3->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_DATAOVERFLOW) )
		fSuccess = TRUE;
	
	// Free memory in the list
	DBTypeList.RemoveAll();

	SAFE_RELEASE_(pICommand);
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pwszNumeric)
	g_Table3->DropTable();
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// {{ TCW_VAR_PROTOTYPE(49)
//--------------------------------------------------------------------
// @mfunc DB_E_DATAOVERFLOW: string right (22001
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_49()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	ICommandPrepare*	pICommandPrep	= NULL;		// ICommandPrepare Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pTableNCpy		= NULL;		// Copy of the Table Name
	WCHAR *				pwszValue		= NULL;		// String value
	IUnknown *			pIUnk=NULL;
	DBORDINAL			pcColumns		= 0;		// Count of columns
	DBLENGTH			ColSize			= 0;		// Column Size
	ULONG				count			= 0;		// Loop counter
	CCol				NewCol(m_pIMalloc);			// Class CCol
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || g_Table3->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	// should fail on buckhorn (sql 6.0) due
	// to driver limitation

	// if this fails on access, it is a driver bug


	if( !CHECK(g_Table3->CreateTable(1,			// Number of rows to insert
									  1,			// Column to put index on
									  NULL,			// Table name
									  PRIMARY),		// Primary or secondary val
									  S_OK) )
		return TEST_FAIL;


	// Creates a column list from the Ctable
	pcColumns = g_Table3->CountColumnsOnTable();


	// Loop thru column types to find a char column with create params
	// When the single column table is created below we assume create params (length)
	// and use (length-1) to avoid the SQL Server invalid error code when char(255) is used.
	for( count=1; count <= pcColumns; count++)
	{
		g_Table3->GetColInfo(count, NewCol);
		
		// If first column is already non-long character with create params then were done
		if( IsColCharacter(NewCol.GetProviderType()) && 
			NewCol.GetCreateParams() &&
			!NewCol.GetIsLong())
			break;
	}
	
	if (count > pcColumns)
	{
		odtLog << L"Couldn't find a non-Long char column for this test.\n";
		return TEST_SKIPPED;
	}

	// Get the name of the table just created
	pTableName = g_Table3->GetTableName();


	// Get a copy of the table name
	pTableNCpy = (WCHAR *) m_pIMalloc->Alloc((wcslen(pTableName) *
					sizeof(WCHAR)) + sizeof(WCHAR));

	wcscpy(pTableNCpy, pTableName);

	// Get Column Size
	ColSize = NewCol.GetMaxSize();


	// Create a table
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszCreateStringTable) + wcslen(pTableNCpy) + 
				   wcslen(NewCol.GetProviderTypeName()) + 3)) );


	// Format SQL Statement
	swprintf(pwszSQLStmt, wszCreateStringTable, pTableNCpy, NewCol.GetProviderTypeName(), ColSize-1);

	// Drop the table, also deletes the table name
	g_Table3->DropTable();
	
	//  Command to return a ICommand with Text Set
	if( !CHECK(g_Table3->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK) )
		goto END;

	// Set the TableName
	g_Table3->SetTableName(pTableNCpy);

	// Alloc Memory
	pwszValue= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + 
											 (sizeof(WCHAR)*(ColSize)) );

	PROVIDER_FREE(pwszSQLStmt);

	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidCharValue) + wcslen(pTableNCpy) + ColSize)) );

	// Create String value out of range
	wcscpy(pwszValue, L"\0");
	for(count=1; count <= ColSize; count++)
		wcscat(pwszValue, L"A");

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidCharValue, pTableNCpy, pwszValue);

	//  Command to return a ICommand with Text Set
	if( CHECK(g_Table3->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_DATAOVERFLOW) )
		fSuccess = TRUE;

END:
	// Release Objects
	SAFE_RELEASE(pICommandPrep);
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pTableNCpy);
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pwszValue);

	// Drop the table
	g_Table3->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Create Table statement on a table that already exists (S0001)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_50()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT

	CTable cTable((IUnknown *)m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName); 

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
	{
		odtLog << L"Can't create a table on a read-only provider.\n";
		return TEST_SKIPPED;
	}

	// Save the values we may change to force command usage
	BOOL fITableDef = GetModInfo()->IsUsingITableDefinition();

	// Set the values to force command usage
	GetModInfo()->UseITableDefinition(FALSE);

	// Save the existing table name since CreateTable will release it
	hr = cTable.CreateTable((DBCOUNTITEM)0, (DBORDINAL)0, m_pTable->GetTableName());
	GetModInfo()->UseITableDefinition(fITableDef);

	TESTC_(hr, DB_E_ERRORSINCOMMAND);

	fSuccess = TRUE;

CLEANUP:
	return fSuccess;
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Create View statement on a view that already exists (S0001)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_51()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR*				pTableName		= NULL;		// Name of the table
	WCHAR*				pViewName		= NULL;		// Name of the view
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
	{
		odtLog << L"Can't create a view on a read-only provider.\n";
		return TEST_SKIPPED;
	}

	// Get the name of the table just created
	pTableName = m_pTable->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( (sizeof(WCHAR) * 
				  (wcslen(wszCreateView) + wcslen(pTableName) + 
 				   wcslen(pTableName) + 1)) + sizeof(WCHAR) );

	pViewName = (WCHAR *) m_pIMalloc->Alloc( (sizeof(WCHAR) * 
				(wcslen(pTableName) + 1)) + sizeof(WCHAR) );

	// Make a view name
	wcscpy(pViewName, pTableName);
	wcscat(pViewName, L"v");

	// Put the SQL statement together for Create View
	swprintf(pwszSQLStmt, wszCreateView, pViewName, pTableName);

	// Create View
	hr = m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);

	// check to see if the create view failed
	if(FAILED(hr))
	{
		odtLog<<L"Create view not supported" <<ENDL;
		fSuccess = TRUE;
		goto END;
	}

	//  Command to return a ICommand with Text Set
	if( CHECK(m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_ERRORSINCOMMAND) )
		fSuccess = TRUE;

END:
	// Put the SQL statement together for Drop View
	swprintf(pwszSQLStmt, wszDropView, pViewName);

	// Drop View
	hr = m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);
	if( hr != S_OK)
	{
		// Access Driver HACK (Drop Table on the View)
		swprintf(pwszSQLStmt, wszDropTable, pViewName);
		hr = m_pTable->BuildCommand(pwszSQLStmt, iid, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);
	}

	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pViewName);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Select statement with invalid table name (S0002)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_52()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	WCHAR*				pszStartTblName = NULL;
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	// Create a SQL Stmt and Set the Command
	m_pTable->CreateSQLStmt(SELECT_INVALIDTBLNAME, 
							NULL, &pwszSQLStmt, NULL, NULL);

	//  Command to return a ICommand with Text Set
	if( CHECK(m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_NOTABLE) )
		fSuccess = TRUE;

	SAFE_RELEASE_(pICommand);
	PROVIDER_FREE(pwszSQLStmt);
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Drop Table statement on a table that does not exist (S0002)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_53()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR*				pTableName		= NULL;		// Name of the table
	WCHAR*				pTableNCpy		= NULL;		// Name of the table
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || g_Table3->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	if( !CHECK(g_Table3->CreateTable(1,			// Number of rows to insert
									  1,			// Column to put index on
									  NULL,			// Table name
									  PRIMARY),		// Primary or secondary val
									  S_OK) )
		return TEST_FAIL;

	// Get the name of the table just created
	pTableName = g_Table3->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( (sizeof(WCHAR) * 
				  (wcslen(wszDropTable) + wcslen(pTableName))) + sizeof(WCHAR) );

	// Get a copy of the table name
	pTableNCpy = (WCHAR *) m_pIMalloc->Alloc((wcslen(pTableName) *
					sizeof(WCHAR)) + sizeof(WCHAR));
	wcscpy(pTableNCpy, pTableName);

	// Drop the table, also deletes the table name
	g_Table3->DropTable();

	// Put the SQL statement together
	swprintf(pwszSQLStmt, wszDropTable, pTableNCpy);

	//  Command to return a ICommand with Text Set
	if( CHECK(m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_NOTABLE) )
		fSuccess = TRUE;

	SAFE_RELEASE_(pICommand);
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pTableNCpy);
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Drop View statement on a view that does not exist (S0002)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_54()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR*				pTableName		= NULL;		// Name of the table
	WCHAR*				pViewName		= NULL;		// Name of the view
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	// Get the name of the table just created
	pTableName = m_pTable->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( (sizeof(WCHAR) * 
				  (wcslen(wszCreateView) + wcslen(pTableName) + 
				   wcslen(pTableName) + 1)) + sizeof(WCHAR) );

	pViewName = (WCHAR *) m_pIMalloc->Alloc( (sizeof(WCHAR) * 
				(wcslen(pTableName) + 1)) + sizeof(WCHAR) );

	// Make a view name
	wcscpy(pViewName, pTableName);
	wcscat(pViewName, L"v");

	// Put the SQL statement together for Create View
	swprintf(pwszSQLStmt, wszCreateView, pViewName, pTableName);

	// Create View
	hr = m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);

	// check to see if the create view failed
	if(FAILED(hr))
	{
		odtLog<<L"Create view not supported" <<ENDL;
		fSuccess = TRUE;
		goto END;
	}

	// Put the SQL statement together for Drop View
	swprintf(pwszSQLStmt, wszDropView, pViewName);

	// Drop View
	hr = m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand);
	if( hr != S_OK)
	{
		// Access Driver HACK (Drop Table on the View)
		swprintf(pwszSQLStmt, wszDropTable, pViewName);
		if( !CHECK(m_pTable->BuildCommand(pwszSQLStmt, iid, 
				EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK) )
			goto END;
	}

	//  Command to return a ICommand with Text Set
	if( CHECK(m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_NOTABLE) )
		fSuccess = TRUE;

END:
	SAFE_RELEASE_(pICommand);
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pViewName);
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Select statement with invalid column name (S0022)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_55()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR*				pTableName		= NULL;		// Name of the table
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	// if this fails on access, it is a driver bug

	// Get the name of the table just created
	pTableName = m_pTable->GetTableName();

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( (sizeof(WCHAR) * 
				  (wcslen(wszSelectBadColName) + wcslen(pTableName))) + sizeof(WCHAR) );

	// Put the SQL statement together
	swprintf(pwszSQLStmt, wszSelectBadColName, pTableName);

	//  Command to return a ICommand with Text Set

	if( CHECK(m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_ERRORSINCOMMAND) )
		fSuccess = TRUE;

	// Cleanup 
	
	SAFE_RELEASE_(pICommand);

	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Numeric Truncation (01004)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_56()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	ICommand*			pICommand		= NULL;		// ICommand Object
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pwszNumeric		= NULL;		// Numeric value
	DBORDINAL			pcColumns		= 0;		// Count of columns
	ULONG				ColScale		= 0;		// Column Scale
	ULONG				count			= 0;		// Loop counter
	CList				<WCHAR* ,WCHAR*> NativeTypesList;
	CCol				NewCol(m_pIMalloc);			// Class CCol
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	// Creates a column list from the Ctable
	pcColumns = m_pTable->CountColumnsOnTable();

	// Loop thru column types
	for( count=1; count <= pcColumns; count++)
	{
		m_pTable->GetColInfo(count, NewCol);
		
		// If first column is already numeric then were done
		if( IsColNumWithScale(NewCol.GetProviderType(),
							  NewCol.GetScale()) )
			break;
	}

	if (count > pcColumns)
	{
		odtLog << L"Couldn't find a numeric column for this test.\n";
		return TEST_SKIPPED;
	}

	NativeTypesList.AddHead(NewCol.GetProviderTypeName());

	// Create a table
	if(!CHECK(g_Table3->CreateTable(NativeTypesList,
									1,			// Number of rows to insert
									0,			// Column to put index on
									NULL,		// Table name
									PRIMARY),	// Primary or secondary values
									S_OK))
	{
		// Free memory in the list
		NativeTypesList.RemoveAll();
		return TEST_FAIL;
	}

	// Get the name of the table just created
	pTableName = g_Table3->GetTableName();

	// Get Numeric Scale
	ColScale = NewCol.GetScale();

	// Alloc Memory
	pwszNumeric = (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + 
											 (sizeof(WCHAR)*(ColScale+3)) );

	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( sizeof(WCHAR) + (sizeof(WCHAR) * 
				  (wcslen(wszInsertInvalidValue) + wcslen(pTableName) + (ColScale+3))) );

	// Create Numeric Valus out of range
	wcscpy(pwszNumeric, L"1.");
	for(count=1; count <= ColScale+1; count++)
		wcscat(pwszNumeric, L"1");

	// Format SQL Statement
	swprintf(pwszSQLStmt, wszInsertInvalidValue, pTableName, pwszNumeric);

	//  Command to return a ICommand with Text Set
	if( CHECK(g_Table3->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), S_OK) )
		fSuccess = TRUE;

	// Free memory in the list
	NativeTypesList.RemoveAll();

	// Cleanup 
	SAFE_RELEASE_(pICommand);
	
	// Free Memory
	PROVIDER_FREE(pwszSQLStmt);
	PROVIDER_FREE(pwszNumeric);

	// Drop the table
	g_Table3->DropTable();

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - Invalid nodes in a command (37000)
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_57()
{
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	HRESULT				hr				= E_FAIL;	// HRESULT
	WCHAR *				pTableName		= NULL;		// Name of the table
	WCHAR *				pwszSQLStmt		= NULL;		// SQL Statement
	ICommand*			pICommand		= NULL;		// ICommand Object
	IID					iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	// Alloc Memory
	pwszSQLStmt	= (WCHAR *) m_pIMalloc->Alloc( (sizeof(WCHAR) * 
				  (wcslen(wszSelectBadSelect))) + sizeof(WCHAR) );

	// Put the SQL statement together
	wcscpy(pwszSQLStmt, wszSelectBadSelect);

	//  Command to return a ICommand with Text Set
	TEST2C_(m_pTable->BuildCommand(pwszSQLStmt, iid, 
			EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, &pICommand), DB_E_ERRORSINCOMMAND, DB_E_NOTABLE);

	fSuccess = TRUE;

CLEANUP:
	SAFE_RELEASE_(pICommand);
	PROVIDER_FREE(pwszSQLStmt);
	return fSuccess;
}
// }}

// {{ TCW_VAR_PROTOTYPE(58)
//--------------------------------------------------------------------
// @mfunc S_OK: riid = IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_58()
{
	ICommand*	pICmd1=NULL;
	IUnknown * pIUnknown=NULL;
	EINTERFACE	eI = ROWSET_INTERFACE;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))

	if(IFROW)
	{
		eI = ROW_INTERFACE;
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);
	}

	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IUnknown,NULL,NULL,PPI &pIUnknown),S_OK));
	COMPARE(DefaultObjectTesting(pIUnknown, eI), TRUE);

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnknown);
	TRETURN
}
// }}

// }}

// {{ TCW_VAR_PROTOTYPE(59)
//--------------------------------------------------------------------
// @mfunc S_OK: cParam=0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_59()
{
	ICommand*	pICmd1=NULL;
	IUnknown * pIUnknown=NULL;
	DBPARAMS	pParams;
	HRESULT		hr = NOERROR;
	EINTERFACE	eI = ROWSET_INTERFACE;

	if(IFROW)
		eI = ROW_INTERFACE;

	pParams.pData=NULL;
	pParams.cParamSets=0;
	pParams.hAccessor = DB_NULL_HACCESSOR;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	if (FAILED(hr=PrepareCommand(pICmd1,PREPARE,1)))
		CLEANUP(!CHECK(hr, E_NOINTERFACE));	
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,PPI &pIUnknown,NULL,NULL,&pParams),S_OK));
	COMPARE(DefaultObjectTesting(pIUnknown, eI), TRUE);

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnknown);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(60)
//--------------------------------------------------------------------
// @mfunc S_OK: select * from sp_tables
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_60()
{
	ICommand*	pICmd1=NULL;
	IUnknown *	pIUnk=NULL;
	HRESULT		hr = E_FAIL;

	WCHAR		wszSQL[]=L"sp_tables";	
	
  	TESTBEGIN

	// Due to the sp_tables proc used in the select this is a SQL Server specific test.
	if (!g_fSqlServer)
	{
		odtLog << L"This test variation is only valid against SQL Server.\n";
		TSKIPPED;
	}


	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSQL,SELECT_ALLFROMTBL,wszSQL)));

	//since the property setting is logged in SetRowsetProperty, we don't have to check here
	//Also, we don't want to get bunch of errors if they are not supported by the provider
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_BOOKMARKS,TRUE, DBPROPOPTIONS_SETIFCHEAP, TRUE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetLocate,TRUE,DBPROPOPTIONS_SETIFCHEAP, TRUE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_CANSCROLLBACKWARDS,TRUE,DBPROPOPTIONS_SETIFCHEAP, TRUE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_CANFETCHBACKWARDS,TRUE,DBPROPOPTIONS_SETIFCHEAP, TRUE);
	// currently not supported
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetScroll,TRUE,DBPROPOPTIONS_SETIFCHEAP, TRUE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_OTHERUPDATEDELETE,TRUE,DBPROPOPTIONS_SETIFCHEAP, TRUE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetResynch,TRUE,DBPROPOPTIONS_SETIFCHEAP, TRUE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_OWNINSERT,TRUE,DBPROPOPTIONS_SETIFCHEAP, TRUE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_OWNUPDATEDELETE,TRUE,DBPROPOPTIONS_SETIFCHEAP, TRUE);

	// Only supported against Kagera
	if (g_fKagera)
		SetRowsetProperty(pICmd1,DBPROPSET_PROVIDERROWSET,KAGPROP_POSITIONONNEWROW,TRUE,
			DBPROPOPTIONS_SETIFCHEAP, TRUE);

	// This may return DB_S_ERRORSOCCURRED because it wasn't cheap to set the properties
	hr = Execute(pICmd1, PPI &pIUnk);

	// Rowset and row object creation may return DB_S_ERRORSOCCURRED if the props above were not set
	TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED);

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnk);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(61)
//--------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND: create current exisiting table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_61()
{
	ICommand*	pICmd1=NULL;
	IUnknown *	pIUnknown=NULL;

	WCHAR		wszSQL[]=L"create table %s (col1 str)";	
	WCHAR *		pwszSQL=NULL;		

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

  	TESTBEGIN

	// plus 1 for the null terminator
	pwszSQL = (WCHAR *) m_pIMalloc->Alloc(
		(wcslen(wszSQL) + 
		wcslen(m_pTable->GetTableName() + 1)) 
		* sizeof(WCHAR *));

	if(!pwszSQL)
		goto CLEANUP;

	swprintf(pwszSQL,wszSQL,m_pTable->GetTableName());

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSQL,SELECT_ALLFROMTBL,pwszSQL)));

	//CLEANUP(FAILED(PrepareCommand(pICmd1,PREPARE,1)));
	CLEANUP(!CHECK(m_hr = Execute(pICmd1,PPI &pIUnknown),DB_E_ERRORSINCOMMAND));

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnknown);
	PROVIDER_FREE(pwszSQL);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(62)
//--------------------------------------------------------------------
// @mfunc Query TimeOut
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_62()
{

/*
Problem is that command time out is being reset
ADO said they request COMMAND_TIMEOUT as ifcheap
1)Open a rowset in fire-house mode. Keep the rowset around
2)On a command, set text and set time out
	Execution of command should time out
3)On another command, set text
4) Re-execute command from 2, should still timeout
	but wasn't (so that was the bug)
	
Not all properties are supported by access, so I'm making
it a sql server test only
*/
	ICommand*	pICmdRowset=NULL;
	IUnknown *	pIUnknownCmdRowset=NULL;

	ICommand*	pICmd1=NULL;
	ICommand*	pICmd2=NULL;
	IUnknown *	pIUnknown1=NULL;
	IUnknown *	pIUnknown2=NULL;

	ICommandProperties *	pICommandProperties=NULL;
	DBPROPSET				dbPropSet[1];
	DBPROP					dbProp[1];

	ULONG		i=0;
	ULONG		j=0;
	HRESULT		hr = S_OK;
	LPWSTR pwszCmd = NULL;
	LPWSTR pwszText = NULL;
	LPWSTR pwszWaitFor = L"waitfor delay '000:00:20';";  // Wait 20s


	BOOL		bSqlServer=FALSE;
	DBPROPIDSET	rgDBPROPIDSET[1];
	DBPROPID	rgDBPROPID[1];
	ULONG		cDBPROPSET=0;
	DBPROPSET*	rgDBPROPSET=NULL;
	// properties

	IDBProperties* pIDBProp=NULL;

	rgDBPROPIDSET[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	rgDBPROPIDSET[0].cPropertyIDs = 1;
	rgDBPROPIDSET[0].rgPropertyIDs = &rgDBPROPID[0];

	rgDBPROPID[0] = DBPROP_DBMSNAME;

	// only test if we are on a sql server database

	TESTBEGIN

	if(!m_pIDBInitialize)
	{
		odtLog << L"m_pIDBInitialize is null\n";
		return TEST_FAIL;
	}

	if(!CHECK(m_pIDBInitialize->QueryInterface(
		IID_IDBProperties,
		(void **)&pIDBProp),S_OK))
		goto CLEANUP;

	//if DBPROP_DBMSNAME is not supported skip the test
	hr=pIDBProp->GetProperties(
		1, rgDBPROPIDSET, &cDBPROPSET, &rgDBPROPSET);
	if (hr==DB_E_ERRORSOCCURRED)
	{
		if(rgDBPROPSET[0].rgProperties[0].dwStatus!=DBPROPSTATUS_NOTSUPPORTED)
			goto CLEANUP;
		else
		{
			TPASS
			goto CLEANUP;
		}
	}

	for ( i = 0; i < cDBPROPSET; i++ )
	{
		if (rgDBPROPSET[0].guidPropertySet == DBPROPSET_DATASOURCEINFO )
		{
			for (ULONG j = 0 ; j < rgDBPROPSET[i].cProperties; j++ )
			{
				if (rgDBPROPSET[i].rgProperties[j].dwPropertyID == DBPROP_DBMSNAME)
				{
					if (!wcscmp (V_BSTR(&(rgDBPROPSET[i].rgProperties[j].vValue)), L"Microsoft SQL Server"))
					{
						bSqlServer = TRUE;
					}
				
				}
			}
		}
	}

	if(!bSqlServer)
	{
		odtLog << L"Not a sql server\n";
		TPASS

		goto CLEANUP;
	}


	//get rowset in firehose mode and keep it around
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmdRowset,NULL)) 
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmdRowset,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = pICmdRowset->Execute(NULL,IID_IUnknown,NULL,NULL,PPI &pIUnknownCmdRowset),S_OK));

	//set text, then time out, then execute on this command
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL)) 
	// Modify the commmand text for SQL Server to add a wait statement and force the timeout
	if (CHECK(m_pTable->CreateSQLStmt(SELECT_LEFTOUTERJOIN, g_Table2->GetTableName(), &pwszText, NULL, NULL, 1, g_Table2), S_OK))
	{
		SAFE_ALLOC(pwszCmd, WCHAR, wcslen(pwszWaitFor) + wcslen(pwszText)+1);

		if (pwszCmd)
		{
			wcscpy(pwszCmd, pwszWaitFor);
			wcscat(pwszCmd, pwszText);
		}

		hr = m_pTable->BuildCommand(pwszCmd, IID_IRowset, 
				EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICmd1);
	}

	TESTC_(hr, S_OK);

	CLEANUP(!CHECK(pICmd1->QueryInterface(IID_ICommandProperties,(void**)&pICommandProperties),S_OK));

	// status not set but have to check when it comes back
	VariantInit(&(dbProp[0].vValue));
	dbProp[0].dwPropertyID= DBPROP_COMMANDTIMEOUT;
	dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
	dbProp[0].colid = DB_NULLID;
	dbProp[0].vValue.vt = VT_I4;
	V_I4(&dbProp[0].vValue) = 5;// set to 2 in ADO's bug
	
	dbPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;

	if(!CHECK(pICommandProperties->SetProperties(1,dbPropSet),S_OK))
	{
		odtLog << L"Property ["
			<< dbProp[0].dwPropertyID
			<< L"] failure: " 
			<< dbProp[0].dwStatus 
			<< ENDL;
	}
	VariantClear(&(dbProp[0].vValue));
	
	//Look for return code DB_E_ABORTLIMITREACHED 
	m_hr = Execute(pICmd1,PPI &pIUnknown1);

	// This may actually succeed if a fast processor or server
	if (SUCCEEDED(m_hr))
	{
		TESTW_(m_hr, DB_E_ABORTLIMITREACHED);
		TESTC(pIUnknown1 != NULL);
		SAFE_RELEASE(pIUnknown1);
	}
	else
		TESTC_(m_hr, DB_E_ABORTLIMITREACHED);

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd2,NULL))
	hr = m_pTable->BuildCommand(pwszCmd, IID_IRowset, 
			EXECUTE_NEVER, 0, NULL, NULL, NULL, NULL, &pICmd2);

	TESTC_(hr, S_OK);
	
	m_hr = Execute(pICmd1,PPI &pIUnknown1);

	// This may actually succeed if a fast processor or server
	if (SUCCEEDED(m_hr))
	{
		TESTW_(m_hr, DB_E_ABORTLIMITREACHED);
		TESTC(pIUnknown1 != NULL);
		SAFE_RELEASE(pIUnknown1);
	}
	else
		TESTC_(m_hr, DB_E_ABORTLIMITREACHED);

	TPASS

CLEANUP:
	SAFE_FREE(pwszCmd);
	SAFE_FREE(pwszText);
	SAFE_RELEASE(pIDBProp);
	FreeProperties(&cDBPROPSET, &rgDBPROPSET);
	SAFE_RELEASE(pIUnknown1);
	SAFE_RELEASE(pIUnknown2);
	SAFE_RELEASE(pIUnknownCmdRowset);
	SAFE_RELEASE(pICmdRowset);
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pICommandProperties);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(63)
//--------------------------------------------------------------------
// @mfunc Query TimeOut
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_63()
{

/*
Problem is that timeout return code is being returned as
DB_E_ERRORSINCOMMAND instead of DB_E_ABORTLIMITREACHED

I asked ADO if the previous variation would cover
this bug and he said no, so that is why it is here.
	
ADO said all this bug is about is setting time out,
and making sure the right error return code is
set back.

He said they request COMMAND_TIMEOUT as ifcheap

Not all properties are supported by access, so I'm making
it a sql server test only
*/
	ICommand*	pICmd1=NULL;
	IUnknown *	pIUnknown1=NULL;

	ICommandProperties *	pICommandProperties=NULL;
	DBPROPSET				dbPropSet[1];
	DBPROP					dbProp[1];

	TESTBEGIN

	ULONG		i=0;
	ULONG		j=0;
	HRESULT		hr = S_OK;

	LPWSTR pwszCmd = NULL;
	LPWSTR pwszText = NULL;
	LPWSTR pwszWaitFor = L"waitfor delay '000:00:20';";  // Wait 20s
	BOOL		bSqlServer=FALSE;
	DBPROPIDSET	rgDBPROPIDSET[1];
	DBPROPID	rgDBPROPID[1];
	ULONG		cDBPROPSET=0;
	DBPROPSET*	rgDBPROPSET=NULL;
	time_t		tstart = 0;
	time_t		tend = 0;
	LONG		timeout = 5;  // Set timeout to 5s, rather than 2s, to allow better resolution.

	// properties

	IDBProperties* pIDBProp=NULL;

	rgDBPROPIDSET[0].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	rgDBPROPIDSET[0].cPropertyIDs = 1;
	rgDBPROPIDSET[0].rgPropertyIDs = rgDBPROPID;

	rgDBPROPID[0] = DBPROP_DBMSNAME;

	// only test if we are on a sql server database


	if(!m_pIDBInitialize)
	{
		odtLog << L"m_pIDBInitialize is null\n";
		return TEST_FAIL;
	}

	if(!CHECK(m_pIDBInitialize->QueryInterface(
		IID_IDBProperties,
		(void **)&pIDBProp),S_OK))
		goto CLEANUP;

	//if DBPROP_DBMSNAME is not supported skip the test
	hr=pIDBProp->GetProperties(
		1, rgDBPROPIDSET, &cDBPROPSET, &rgDBPROPSET);
	if (hr==DB_E_ERRORSOCCURRED)
	{
		if(rgDBPROPSET[0].rgProperties[0].dwStatus!=DBPROPSTATUS_NOTSUPPORTED)
			goto CLEANUP;
		else
		{
			TPASS
			goto CLEANUP;
		}
	}
	
	for ( i = 0; i < cDBPROPSET; i++ )
	{
		if (rgDBPROPSET[0].guidPropertySet == DBPROPSET_DATASOURCEINFO )
		{
			for (ULONG j = 0 ; j < rgDBPROPSET[i].cProperties; j++ )
			{
				if (rgDBPROPSET[i].rgProperties[j].dwPropertyID == DBPROP_DBMSNAME)
				{
					if (!wcscmp (V_BSTR(&(rgDBPROPSET[i].rgProperties[j].vValue)), L"Microsoft SQL Server"))
					{
						bSqlServer = TRUE;
					}
				
				}
			}
		}
	}

	if(!bSqlServer)
	{
		odtLog << L"Not a sql server\n";
		TSKIPPED;

		goto CLEANUP;
	}
	//set text, then time out, then execute on this command
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL)) 
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,g_Table2->GetTableName(),
		eSELECT,SELECT_LEFTOUTERJOIN,NULL,NULL,NULL,g_Table2)));
	CLEANUP(!CHECK(pICmd1->QueryInterface(IID_ICommandProperties,(void**)&pICommandProperties),S_OK));

	// status not set but have to check when it comes back
	VariantInit(&(dbProp[0].vValue));
	dbProp[0].dwPropertyID= DBPROP_COMMANDTIMEOUT;
	dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
	dbProp[0].colid = DB_NULLID;
	dbProp[0].vValue.vt = VT_I4;
	V_I4(&dbProp[0].vValue) = timeout;// set to 2 in ADO's bug
	
	dbPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;

	if(!CHECK(pICommandProperties->SetProperties(1,dbPropSet),S_OK))
	{
		odtLog << L"Property ["
			<< dbProp[0].dwPropertyID
			<< L"] failure: " 
			<< dbProp[0].dwStatus 
			<< ENDL;
	}
	VariantClear(&(dbProp[0].vValue));
	
	// Get time
	time(&tstart);
	m_hr = Execute(pICmd1,PPI &pIUnknown1);
	// Get time
	time(&tend);

	// Make sure time diff is not greater than timeout
	TESTC(tend-tstart <= timeout+1);

	// Look for return code DB_E_ABORTLIMITREACHED 

	// This may actually succeed if a fast processor or server
	if (SUCCEEDED(m_hr))
	{
		TESTW_(m_hr, DB_E_ABORTLIMITREACHED);
		TESTC(pIUnknown1 != NULL);
	}
	else
		TESTC_(m_hr, DB_E_ABORTLIMITREACHED);

	TPASS

CLEANUP:
	SAFE_FREE(pwszCmd);
	SAFE_FREE(pwszText);
	SAFE_RELEASE(pIDBProp);
	FreeProperties(&cDBPROPSET, &rgDBPROPSET);
	SAFE_RELEASE(pIUnknown1);
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICommandProperties);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(64)
//--------------------------------------------------------------------
// @mfunc Firehose mode
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CExecute_Rowset::Variation_64()
{

/*
	Verify that when the first statement 
	is opened in one cursor mode that other statements
	can be opened in other cursor modes

Sub Bug_FieldEnumeratorError2()
  
    Dim rs As New Recordset, rs2 As New Recordset
    Dim conn As New Connection
 
    On Error GoTo errfound
    conn.Open "adotsql65", "adolab", "adolab"
    
    rs.Open "select * from azpt1dao", conn		// Forward Only/Read Only
    
    rs2.Open "select * from azpt1dao", conn, adOpenStatic, adLockOptimistic  
    rs2.Close
    
    rs2.Open "select * from azpt1dao", conn, adOpenStatic, adLockOptimistic '<-- error here
    rs2.Close
        
Exit Sub


1) rs is opened in firehose mode
2) rs2 causes another connection to be opened and 
	the first rs2.Open statement succeeds
3) rs2 is closed and the connection is freed
4)rs2.open (second case) - for some reason 
	Kagera isn't detecting that the first statement 
	is opened in firehose mode and tries to open the 
	cursor on the first connection
	

adOpenStatic:
	OWNINSERT
	OWNUPDATEDELETE
	IRowsetLocate
	CANSCROLLBACKWARDS
	CANHOLDROWS
	IRowsetResynch
	REMOVEDELETED
	KAGPROP_POSITIONONNEWROW

adLockOptimistic:
	IRowsetChange
	UPDATABILITY (
		DBPROPVAL_UP_INSERT | 
		DBPROPVAL_UP_DELETE |
		DBPROPVAL_UP_CHANGE)
	CONCURRENCY (
		KAGPROP_CONCUR_VALUES | 
		KAGPROP_CONCUR_ROWVER)

every ADO command gets MAXROWS==1, and COMMANDTIMEOUT==???
really don't know which props are supposed to be
	required and which setifcheap	

Not all properties are supported by access, so I'm making
it a sql server test only
*/
	ICommand*	pICmd1=NULL;
	IUnknown *	pIUnknown1=NULL;
	ICommand*	pICmd2=NULL;
	IUnknown *	pIUnknown2=NULL;
	ICommandProperties *	pICommandProperties1=NULL;
	ICommandProperties *	pICommandProperties2=NULL;
	DBPROPSET*	rgDBPROPSET=NULL;
	IDBProperties* pIDBProp=NULL;
	
	HRESULT hr=E_FAIL;
	DBPROPSET				dbPropSet[1];
	DBPROP					dbProp[1];
	ULONG		i=0;
	ULONG		j=0;

	ULONG		cDBPROPSET=0;

	TESTBEGIN

	//set text, then time out, then execute on this command
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL)) 
	CLEANUP(FAILED(hr=SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(hr=pICmd1->QueryInterface(IID_ICommandProperties,(void**)&pICommandProperties1),S_OK));

	// status not set but have to check when it comes back
	VariantInit(&(dbProp[0].vValue));
	dbProp[0].dwPropertyID= DBPROP_COMMANDTIMEOUT;
	dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
	dbProp[0].colid = DB_NULLID;
	dbProp[0].vValue.vt = VT_I4;
	V_I4(&dbProp[0].vValue) = 100;// set to 2 in ADO's bug

	dbPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;

	// Not all providers support command timeout.  It's not critical if not supported.
	if(FAILED(hr=pICommandProperties1->SetProperties(1,dbPropSet)))
	{														  
		CHECK(hr,DB_E_ERRORSOCCURRED);

		if (dbProp[0].dwStatus != DBPROPSTATUS_NOTSUPPORTED)
		{
			odtLog << L"Property ["
				<< dbProp[0].dwPropertyID
				<< L"] failure: " 
				<< dbProp[0].dwStatus 
				<< ENDL;
		}
	}
	VariantClear(&(dbProp[0].vValue));
	
	// status not set but have to check when it comes back
	VariantInit(&(dbProp[0].vValue));
	dbProp[0].dwPropertyID=DBPROP_MAXROWS; 
	dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
	dbProp[0].colid = DB_NULLID;
	dbProp[0].vValue.vt = VT_I4;
	V_I4(&dbProp[0].vValue) = 1; // set to 1 in ADO's bug

	dbPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;

	// We don't really care if this property gets set if it's not supported
	if(FAILED(hr=pICommandProperties1->SetProperties(1,dbPropSet)))
	{
		CHECK(hr,DB_E_ERRORSOCCURRED);

		if (dbProp[0].dwStatus != DBPROPSTATUS_NOTSUPPORTED &&
			dbProp[0].dwStatus != DBPROPSTATUS_NOTSETTABLE)
		{
			odtLog << L"Property ["
				<< dbProp[0].dwPropertyID
				<< L"] failure: " 
				<< dbProp[0].dwStatus 
				<< ENDL;
		}
	}
	VariantClear(&(dbProp[0].vValue));

	// status not set but have to check when it comes back
	VariantInit(&(dbProp[0].vValue));
	dbProp[0].dwPropertyID=DBPROP_UPDATABILITY; 
	dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
	dbProp[0].colid = DB_NULLID;
	dbProp[0].vValue.vt = VT_I4;
	V_I4(&dbProp[0].vValue) = 0;	// Not updatable

	dbPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;


	// Set the updatability property. This will fail if not supported, but we don't care
	pICommandProperties1->SetProperties(1,dbPropSet);
	VariantClear(&(dbProp[0].vValue));

	if (g_fKagera)
	{
		// status not set but have to check when it comes back
		VariantInit(&(dbProp[0].vValue));
		dbProp[0].dwPropertyID=KAGPROP_CONCURRENCY; 
		dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
		dbProp[0].colid = DB_NULLID;
		dbProp[0].vValue.vt = VT_I4;
		V_I4(&dbProp[0].vValue) = KAGPROPVAL_CONCUR_READ_ONLY;

		dbPropSet[0].guidPropertySet = DBPROPSET_PROVIDERROWSET;
		dbPropSet[0].cProperties = 1;
		dbPropSet[0].rgProperties = dbProp;


		if(FAILED(hr=pICommandProperties1->SetProperties(1,dbPropSet)))
		{
			CHECK(hr,S_OK);

			odtLog << L"Property ["
				<< dbProp[0].dwPropertyID
				<< L"] failure: " 
				<< dbProp[0].dwStatus 
				<< ENDL;
		}
		VariantClear(&(dbProp[0].vValue));
	}

	// We want to set a bunch of properties that will likely force a FORWARD ONLY/READ ONLY cursor, but we need to
	// igore the return code in case one of the properties isn't supported.
//	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_OWNINSERT, FALSE, DBPROPOPTIONS_REQUIRED, FALSE);
//	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_OWNUPDATEDELETE,FALSE, DBPROPOPTIONS_REQUIRED, FALSE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetLocate,FALSE, DBPROPOPTIONS_REQUIRED, FALSE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_CANSCROLLBACKWARDS,FALSE, DBPROPOPTIONS_REQUIRED, FALSE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_CANHOLDROWS,FALSE, DBPROPOPTIONS_REQUIRED, FALSE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetResynch,FALSE, DBPROPOPTIONS_REQUIRED, FALSE);
//	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_REMOVEDELETED,FALSE, DBPROPOPTIONS_REQUIRED, FALSE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_IRowsetChange,FALSE, DBPROPOPTIONS_REQUIRED, FALSE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_BOOKMARKS,FALSE, DBPROPOPTIONS_REQUIRED, FALSE);
	SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,DBPROP_CANFETCHBACKWARDS,FALSE, DBPROPOPTIONS_REQUIRED, FALSE);

	if (g_fKagera)
		CHECK(hr=SetRowsetProperty(pICmd1,DBPROPSET_PROVIDERROWSET,KAGPROP_POSITIONONNEWROW,FALSE, DBPROPOPTIONS_REQUIRED, TRUE),S_OK);

	// We require a FO/RO cursor here, so if Execute fails we can't proceed with the test
	hr = Execute(pICmd1,PPI &pIUnknown1);

	if(IFROW)
		TESTC_(hr, S_OK);

	// A generic provider may not support FO/RO. This is DB_E/S_ERRORSOCCURRED.
	if (hr == DB_E_ERRORSOCCURRED || hr == DB_S_ERRORSOCCURRED)
	{
		odtLog << L"Couldn't obtain Forward Only/Read Only rowset against this provider.\n";
		TSKIPPED;
	}

	// Should be successful
	TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED);

	//set text, then time out, then execute on this command
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd2,NULL)) 
	CLEANUP(FAILED(hr=SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(hr=pICmd2->QueryInterface(IID_ICommandProperties,(void**)&pICommandProperties2),S_OK));

	// status not set but have to check when it comes back
	VariantInit(&(dbProp[0].vValue));
	dbProp[0].dwPropertyID=DBPROP_COMMANDTIMEOUT; 
	dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
	dbProp[0].colid = DB_NULLID;
	dbProp[0].vValue.vt = VT_I4;
	V_I4(&dbProp[0].vValue) = 100;

	dbPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;


	if(FAILED(hr=pICommandProperties2->SetProperties(1,dbPropSet)))
	{														  
		CHECK(hr,DB_E_ERRORSOCCURRED);

		if (dbProp[0].dwStatus != DBPROPSTATUS_NOTSUPPORTED)
		{
			odtLog << L"Property ["
				<< dbProp[0].dwPropertyID
				<< L"] failure: " 
				<< dbProp[0].dwStatus 
				<< ENDL;
		}
	}
	VariantClear(&(dbProp[0].vValue));
	
	// status not set but have to check when it comes back
	VariantInit(&(dbProp[0].vValue));
	dbProp[0].dwPropertyID=DBPROP_MAXROWS; 
	dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
	dbProp[0].colid = DB_NULLID;
	dbProp[0].vValue.vt = VT_I4;
	V_I4(&dbProp[0].vValue) = 1;

	dbPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;

	// We don't really care if this property gets set if it's not supported
	if(FAILED(hr=pICommandProperties2->SetProperties(1,dbPropSet)))
	{
		CHECK(hr,DB_E_ERRORSOCCURRED);

		if (dbProp[0].dwStatus != DBPROPSTATUS_NOTSUPPORTED &&
			dbProp[0].dwStatus != DBPROPSTATUS_NOTSETTABLE)
		{
			odtLog << L"Property ["
				<< dbProp[0].dwPropertyID
				<< L"] failure: " 
				<< dbProp[0].dwStatus 
				<< ENDL;
		}
	}
	VariantClear(&(dbProp[0].vValue));

	// status not set but have to check when it comes back
	VariantInit(&(dbProp[0].vValue));
	dbProp[0].dwPropertyID=DBPROP_UPDATABILITY; 
	dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
	dbProp[0].colid = DB_NULLID;
	dbProp[0].vValue.vt = VT_I4;
	V_I4(&dbProp[0].vValue) = 
		DBPROPVAL_UP_INSERT | 
		DBPROPVAL_UP_DELETE |
		DBPROPVAL_UP_CHANGE;

	dbPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;


	pICommandProperties2->SetProperties(1,dbPropSet);
	VariantClear(&(dbProp[0].vValue));

	if (g_fKagera)
	{
		// status not set but have to check when it comes back
		VariantInit(&(dbProp[0].vValue));
		dbProp[0].dwPropertyID=KAGPROP_CONCURRENCY; 
		dbProp[0].dwOptions = DBPROPOPTIONS_SETIFCHEAP;
		dbProp[0].colid = DB_NULLID;
		dbProp[0].vValue.vt = VT_I4;
		V_I4(&dbProp[0].vValue) = 
			KAGPROPVAL_CONCUR_VALUES | 
			KAGPROPVAL_CONCUR_ROWVER;

		dbPropSet[0].guidPropertySet = DBPROPSET_PROVIDERROWSET;
		dbPropSet[0].cProperties = 1;
		dbPropSet[0].rgProperties = dbProp;



		if(FAILED(hr=pICommandProperties2->SetProperties(1,dbPropSet)))
		{
			CHECK(hr,S_OK);
			
			odtLog << L"Property ["
				<< dbProp[0].dwPropertyID
				<< L"] failure: " 
				<< dbProp[0].dwStatus 
				<< ENDL;
		}
		VariantClear(&(dbProp[0].vValue));
	}


	// We want to set a property that will likely force a non-FORWARD ONLY/non-READ ONLY cursor, but we need to
	// igore the return code in case one of the properties isn't supported.
	hr = SetRowsetProperty(pICmd2,DBPROPSET_ROWSET,DBPROP_CANSCROLLBACKWARDS,TRUE, DBPROPOPTIONS_REQUIRED, FALSE);
	if (!SUCCEEDED(hr))
		hr = SetRowsetProperty(pICmd2,DBPROPSET_ROWSET,DBPROP_OWNUPDATEDELETE,TRUE, DBPROPOPTIONS_REQUIRED, FALSE);
	if (!SUCCEEDED(hr))
		hr = SetRowsetProperty(pICmd2,DBPROPSET_ROWSET,DBPROP_IRowsetLocate,TRUE, DBPROPOPTIONS_REQUIRED, FALSE);
	if (!SUCCEEDED(hr))
		hr = SetRowsetProperty(pICmd2,DBPROPSET_ROWSET,DBPROP_OWNINSERT,TRUE, DBPROPOPTIONS_REQUIRED, FALSE);
	if (!SUCCEEDED(hr))
		hr = SetRowsetProperty(pICmd2,DBPROPSET_ROWSET,DBPROP_CANHOLDROWS,TRUE, DBPROPOPTIONS_REQUIRED, FALSE);
	if (!SUCCEEDED(hr))
		hr = SetRowsetProperty(pICmd2,DBPROPSET_ROWSET,DBPROP_IRowsetResynch,TRUE, DBPROPOPTIONS_REQUIRED, FALSE);
	if (!SUCCEEDED(hr))
		hr = SetRowsetProperty(pICmd2,DBPROPSET_ROWSET,DBPROP_REMOVEDELETED,TRUE, DBPROPOPTIONS_REQUIRED, FALSE);
	if (!SUCCEEDED(hr))
		hr = SetRowsetProperty(pICmd2,DBPROPSET_ROWSET,DBPROP_IRowsetChange,TRUE, DBPROPOPTIONS_REQUIRED, FALSE);

	if (g_fKagera && !SUCCEEDED(hr))
		CHECK(hr=SetRowsetProperty(pICmd2,DBPROPSET_PROVIDERROWSET,KAGPROP_POSITIONONNEWROW,TRUE,
			DBPROPOPTIONS_REQUIRED, TRUE),S_OK);

	CLEANUP(!CHECK(hr = Execute(pICmd2,PPI &pIUnknown2),S_OK ));
	COMPARE(DefaultObjectTesting(pIUnknown2, IFROW ? ROW_INTERFACE : ROWSET_INTERFACE), TRUE);
	SAFE_RELEASE(pIUnknown2);

	// Failure should repro here if it's going to
	CLEANUP(!CHECK(hr = Execute(pICmd2,PPI &pIUnknown2),S_OK ));

	TPASS

CLEANUP:

	FreeProperties(&cDBPROPSET, &rgDBPROPSET);

	SAFE_RELEASE(pIDBProp);
	SAFE_RELEASE(pIUnknown1);
	SAFE_RELEASE(pIUnknown2);
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pICommandProperties1);
	SAFE_RELEASE(pICommandProperties2);

	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Open a rowset on aggregated command
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_65()
{ 
	ICommand * pIAggCommand = NULL;
	IRowset * pIRowset = NULL;
	TESTRESULT		testresult = TEST_FAIL;
	HRESULT			hr = E_FAIL, hrSetProp = E_FAIL;
	DBORDINAL		cRowsetCols;
	DBORDINAL * rgTableColOrds = NULL;
	IUnknown * pIAggUnknown = NULL;
	IUnknown*	pIUnk = NULL;
	IID			iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	CAggregate Aggregate;

	// Create a new ICommandText object aggregated from IDBCreateCommand object.
	hr = m_pIDBCreateCommand->CreateCommand(&Aggregate, IID_IUnknown, 
		(IUnknown **)&pIAggUnknown);
	Aggregate.SetUnkInner(pIAggUnknown);

	TESTC(Aggregate.VerifyAggregationQI(hr, IID_ICommand));

	// See if we can get ICommandText off our aggregated object
	TESTC_(Aggregate.QueryInterface(IID_ICommand, (void **)&pIAggCommand), S_OK);

	// Kagera can't retrieve BLOB data without IRowsetLocate on.
//	if (g_fKagera)
//		CHECK(hrSetProp = SetRowsetProperty(pIAggCommand, DBPROPSET_ROWSET, DBPROP_IRowsetLocate, TRUE), S_OK);
	if (g_fKagera && SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(pIAggCommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	// Set command text and execute
	TESTC_(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, iid, NULL,
		NULL, &cRowsetCols, (DB_LORDINAL **)&rgTableColOrds, EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIUnk,
		&pIAggCommand), S_OK);

	if(IFROWSET)
		TESTC_(VerifyRowset(IID_IRowset, (IUnknown *)pIUnk, 1, 
			cRowsetCols, rgTableColOrds, FALSE), S_OK);

	if(IFROW)
		TESTC_(VerifyRowObj(IID_IRow, (IUnknown *)pIUnk, 1, 
			cRowsetCols, rgTableColOrds, FALSE), S_OK);

	// Now release our aggregated command text object and IUnknown
	// We expect the aggregated object to go away when the last ref is released.
	SAFE_RELEASE(pIUnk);

	// Reset ACCESSORDER
	if (hrSetProp == S_OK)
		CHECK(SetRowsetProperty(pIAggCommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM, DBPROPOPTIONS_OPTIONAL), S_OK);

	SAFE_RELEASE(pIAggCommand);
	TESTC(Aggregate.GetRefCount()==1);	// Object gone!

	testresult = TEST_PASS;

CLEANUP:
	SAFE_FREE(rgTableColOrds);
	SAFE_RELEASE(pIUnk);
	// Reset again just in case of failure above.
	if (hrSetProp == S_OK && pIAggCommand)
		CHECK(SetRowsetProperty(pIAggCommand, DBPROPSET_ROWSET, DBPROP_IRowsetLocate, TRUE, DBPROPOPTIONS_OPTIONAL), S_OK);
	SAFE_RELEASE(pIAggUnknown);
	SAFE_RELEASE(pIAggCommand);
	return testresult;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Select, IID_NULL, ppRowset NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_66()
{ 
	ICommand*	pICmd1=NULL;
	DBROWCOUNT cRowsAffected=0;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(FAILED(PrepareCommand(pICmd1,PREPARE,1)));
	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_NULL,NULL,&cRowsAffected,NULL),S_OK));

	// Note for a select stmt cRowsAffected will be "undefined" per spec, so we don't test it.

	TPASS;

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	TRETURN;

} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Test all IID's of object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_67()
{ 
    IUnknown* pIUnknown = NULL;
	ICommand* pICmd1    = NULL;
	EINTERFACE	eI = ROWSET_INTERFACE;

	if(IFROW)
		eI = ROW_INTERFACE;

	TESTBEGIN;

	//Obtain the Rowset IIDs
	ULONG i, cRowsetIIDs = 0;
	INTERFACEMAP* rgRowsetIIDs = NULL;
	TESTC(GetInterfaceArray(eI, &cRowsetIIDs, &rgRowsetIIDs));

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	if(IFROW)
	{
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);
	}

    //Loop through all rowset IIDs...
	for(i=0; i<cRowsetIIDs; i++)
	{
		odtLog << rgRowsetIIDs[i].pwszName << L"\n";

		//Asking for IID_I* is requesting a rowset that supports this interface
		//This is implicilty like requesting DBPROP_I* ahead of time...
		TESTC_(SetCommandText(m_pIMalloc,pICmd1,g_Table2,NULL,eSELECT,SELECT_ALLFROMTBL,NULL), S_OK);
		TEST2C_(m_hr=pICmd1->Execute(NULL,*rgRowsetIIDs[i].pIID,NULL,NULL,&pIUnknown),S_OK, E_NOINTERFACE);
	
		//Success, verify this interface...
		if(m_hr == S_OK)
		{
			if(!ValidInterface(*rgRowsetIIDs[i].pIID, pIUnknown))
				TERROR(L"Interface Incorrect for " << GetInterfaceName(*rgRowsetIIDs[i].pIID) << "\n");

			TESTC(DefaultObjectTesting(pIUnknown, eI));
		}
		else
		{
			//Make sure this is allowed to not be required
			TCOMPARE_(!rgRowsetIIDs[i].fMandatory);
		}
		
	    SAFE_RELEASE(pIUnknown);
	}

	TPASS;

CLEANUP:
	SAFE_RELEASE(pICmd1);
    SAFE_RELEASE(pIUnknown);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE - Ask for unsupported interface with ppRowset == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_68()
{ 
	ICommand* pICmd1=NULL;
	HRESULT hrExec = E_FAIL;

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	hrExec = pICmd1->Execute(NULL,IID_IDBProperties,NULL,NULL,NULL);
	
	// Depending on checking order, provider may return either failure
	TEST2C_(hrExec, E_NOINTERFACE, E_INVALIDARG);

	TPASS

CLEANUP:
	SAFE_RELEASE(pICmd1);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc DB_E_INTEGRITYVIOLATION - Insert a duplicate key value
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_69()
{ 
	DBROWCOUNT cRowsAffected=0;
	ICommand*	pICmd1=NULL;
	CCol ColIndex;
	DBORDINAL ulIndexCol = 1;
	HRESULT hrCreateIndex = E_FAIL;
	
	TESTBEGIN;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
	{
		TSKIPPED;
	}

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	// If the index column happens to be an autoincrement or non-updatable column
	// then the insert query below will not contain that column and the integrity
	// violation will not occur. To prevent this create another index.
	ulIndexCol = g_Table2->GetIndexColumn();

	TESTC_(g_Table2->GetColInfo(ulIndexCol, ColIndex), S_OK);

	if (ColIndex.GetAutoInc())
	{
		ULONG iNewIndex;
		for (iNewIndex = 1; iNewIndex <= g_Table2->CountColumnsOnTable();
			iNewIndex++)
		{
			if (iNewIndex == ulIndexCol)
				continue;

			TESTC_(g_Table2->GetColInfo(iNewIndex, ColIndex), S_OK);

			if (!ColIndex.GetAutoInc() && ColIndex.GetUpdateable())
				break;
		}

		TESTC(iNewIndex <= g_Table2->CountColumnsOnTable());

		// Since there's already an index with the default name we need to 
		// create one with a new name.
		TESTC_(g_Table2->GetColInfo(iNewIndex, ColIndex), S_OK);
		hrCreateIndex = g_Table2->CreateIndex(iNewIndex, UNIQUE, ColIndex.GetColName());
		TESTC_(hrCreateIndex, S_OK);
	}

	// Cause the CTable object to create a duplicate row. Note g_Table2 has an 
	// index so we can't insert a dupicate row.
	g_Table2->SubtractRow();

	if(S_OK == (m_hr = SetCommandText(m_pIMalloc,pICmd1,g_Table2,NULL,eINSERT,NO_QUERY,NULL)))
		m_hr = pICmd1->Execute(NULL,IID_NULL,NULL,&cRowsAffected,NULL);

	// Set CTable to have proper row count
	g_Table2->AddRow();

	TESTC_(m_hr, DB_E_INTEGRITYVIOLATION);

	if (cRowsAffected != DB_COUNTUNAVAILABLE)
		TESTC(cRowsAffected == 0);

	TPASS;

CLEANUP:
	// The extra index if will be dropped when the table is dropped
	SAFE_RELEASE(pICmd1);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc S_OK - select with qualified name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_70()
{ 
	ICommand*	pICmd1=NULL;
	LPWSTR pwszTableName = NULL;
	LPWSTR pwszQualifiedName = NULL;
	LPWSTR pwszCatalogName = NULL;
	LPWSTR pwszSchemaName = NULL;
	DBORDINAL cRowsetCols;
	DBORDINAL * rgTableColOrds = NULL;
	IUnknown * pIUnk = NULL;
	IID			iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTBEGIN;

	pwszTableName = wcsDuplicate(m_pTable->GetTableName());

	TESTC(pwszTableName != NULL);

	GetQualifierNames(m_pThisTestModule->m_pIUnknown2, pwszTableName,
		&pwszCatalogName, &pwszSchemaName);

	TESTC_(m_pTable->GetQualifiedName(pwszCatalogName, pwszSchemaName, 
		pwszTableName,&pwszQualifiedName), S_OK);

	m_pTable->SetTableName(pwszQualifiedName);

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC_(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, iid, NULL,
		NULL, &cRowsetCols, (DB_LORDINAL **)&rgTableColOrds, EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIUnk,
		&pICmd1), S_OK);

	if(IFROWSET)
		TESTC_(VerifyRowset(IID_IRowset, (IUnknown *)pIUnk, 1, 
			cRowsetCols, rgTableColOrds, FALSE), S_OK);
	if(IFROW)
		TESTC_(VerifyRowObj(IID_IRow, (IUnknown *)pIUnk, 1, 
			cRowsetCols, rgTableColOrds, FALSE), S_OK);

	TPASS;

CLEANUP:
	m_pTable->SetTableName(pwszTableName);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pICmd1);
	SAFE_FREE(pwszTableName);
	SAFE_FREE(pwszQualifiedName);
	SAFE_FREE(pwszCatalogName);
	SAFE_FREE(pwszSchemaName);
	SAFE_FREE(rgTableColOrds);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc S_OK - select with quoted name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_71()
{ 
	ICommand*	pICmd1=NULL;
	LPWSTR pwszTableName = NULL;
	LPWSTR pwszQuotedName = NULL;
	DBORDINAL	cRowsetCols;
	DBORDINAL * rgTableColOrds = NULL;
	IUnknown * pIUnk = NULL;
	IID			iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTBEGIN;

	pwszTableName = wcsDuplicate(m_pTable->GetTableName());

	TESTC(pwszTableName != NULL);

	TESTC_(m_pTable->GetQuotedName(pwszTableName,&pwszQuotedName), S_OK);

	m_pTable->SetTableName(pwszQuotedName);

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC_(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, iid, NULL,
		NULL, &cRowsetCols, (DB_LORDINAL **)&rgTableColOrds, EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIUnk,
		&pICmd1), S_OK);

	if(IFROWSET)
		TESTC_(VerifyRowset(IID_IRowset, (IUnknown *)pIUnk, 1, 
			cRowsetCols, rgTableColOrds, FALSE), S_OK);
	if(IFROW)
		TESTC_(VerifyRowObj(IID_IRow, (IUnknown *)pIUnk, 1, 
			cRowsetCols, rgTableColOrds, FALSE), S_OK);

	TPASS;

CLEANUP:
	m_pTable->SetTableName(pwszTableName);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pICmd1);
	SAFE_FREE(pwszTableName);
	SAFE_FREE(pwszQuotedName);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc S_OK - select distinct
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_72()
{ 
	ICommand*	pICmd1=NULL;
	ICommandText * pICommandText = NULL;
	IAccessor * pIAccessor = NULL;
	DBORDINAL	cCols;
	DBCOUNTITEM cRowsObtained;
	ULONG iCol;
	LONG cRows = LONG_MAX;
	HROW * phRows = NULL;
	DB_LORDINAL * pCols = NULL;
	IRowset * pIRowset = NULL;
	WCHAR * pwszSqlStmt = NULL;
	HRESULT hr, hrExec;
	ULONG iRow, jRow;
	BOOL fDup = FALSE;
	BYTE * pData1 = NULL;
	BYTE * pData2 = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBBINDING * pBindings = NULL;
	DBCOUNTITEM cBindings;
	DBLENGTH cbRowSize;
	ULONG ulHiddenColumns = 0;

	TESTBEGIN;

	ONLYROWSETVAR;

	// If the provider doesn't understand SQL then skip test if not using ini file
	if(m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE &&
		!GetModInfo()->GetFileName())
	{
		odtLog << L"Provider doesn't support SQL and not using ini file.\n";
		return TEST_SKIPPED;
	}

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC(VerifyInterface(pICmd1, IID_ICommandText,
						COMMAND_INTERFACE, (IUnknown**)&pICommandText));

	// Try to insert a duplicate row.  This will fail for r/o providers.
	if (!g_fReadOnlyProvider)
		TESTC_(m_pTable->Insert(m_pTable->GetNextRowNumber()-1), S_OK);

	// Since providers will likely try to allocate cRows * sizeof(HROW) for this case,
	// and max allocation size if LONG_MAX (release), or less than LONG_MAX (debug)
	cRows = LONG_MAX/(sizeof(HROW)*100);	

	// Set CANHOLDROWS on so we can hold more than one row on some providers (prop is required)
	TESTC_(SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_CANHOLDROWS, TRUE), S_OK);

	for (iCol = 1; iCol <= g_Table2->CountColumnsOnTable(); iCol++)
	{
		CCol TempCol;
		HRESULT hrCreateStmt = E_FAIL;

		TESTC_(m_pTable->GetColInfo(iCol, TempCol), S_OK);

		// Some providers can't do an order-by on a LONG column, so allow an error
		if (TempCol.GetIsLong())
			hrExec = DB_E_ERRORSINCOMMAND;
		else
			hrExec = S_OK;

		// Set command text for a select distinct query.
		hrCreateStmt = m_pTable->CreateSQLStmt(SELECT_DISTINCTCOLLISTORDERBY, NULL, &pwszSqlStmt,&cCols,&pCols, 0, NULL, iCol);

		TESTC_PROVIDER(hrCreateStmt == S_OK);

		TESTC_(pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSqlStmt), S_OK);

		// Retrieve all rows that match the requested row.
		hr = pICmd1->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown **)&pIRowset);

		if (FAILED(hr))
			CHECK(hr, hrExec);

		// If we succeeded then validate results
		if (S_OK == hr)
		{

			TESTC(VerifyInterface(pIRowset, IID_IAccessor,
								ROWSET_INTERFACE, (IUnknown**)&pIAccessor));

			TESTC_(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
				&hAccessor, &pBindings, &cBindings, &cbRowSize,			
  				DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
				ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
				NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

			// Allocate a couple of row buffers for comparisons
			SAFE_ALLOC(pData1, BYTE, cbRowSize);
			SAFE_ALLOC(pData2, BYTE, cbRowSize);
			
			// Retrieve all the rows and validate no duplicates
			while (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
			{
				TESTC_(hr = pIRowset->GetNextRows(NULL, 0, cRows, &cRowsObtained, &phRows), DB_S_ENDOFROWSET);

				TESTC(phRows != NULL);

				TESTC(cRowsObtained > 0);

				if (phRows)
				{
					for (iRow = 0; iRow < cRowsObtained; iRow++)
					{
						TESTC_(pIRowset->GetData(phRows[iRow], hAccessor, pData1), S_OK);

						for (jRow = 0; jRow < cRowsObtained; jRow++)
						{
							TESTC_(pIRowset->GetData(phRows[jRow], hAccessor, pData2), S_OK);

							// Of course we expect corresponding rows to match
							if (iRow == jRow)
							{

								TESTC(CompareBuffer(pData1, pData2, cBindings, pBindings, NULL, FALSE, FALSE, COMPARE_ONLY));
							}
							else
							{
								BOOL fCmp = CompareBuffer(pData1, pData2, cBindings, pBindings, NULL, FALSE, FALSE, COMPARE_ONLY, FALSE, 0, NULL, TRUE);
								if (fCmp)
									odtLog << L"Data compared when it should not have, row " << iRow << " and row " << jRow << "\n";
								TESTC(!fCmp);
							}

							TESTC_(ReleaseInputBindingsMemory(cBindings, pBindings, pData2), S_OK);
						}

						TESTC_(ReleaseInputBindingsMemory(cBindings, pBindings, pData1), S_OK);
					}

					TESTC_(pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);
					SAFE_FREE(phRows);
				}
			}

			if (hAccessor != DB_NULL_HACCESSOR)
			{
				pIAccessor->ReleaseAccessor(hAccessor, NULL);
				hAccessor = DB_NULL_HACCESSOR;
			}
			SAFE_RELEASE(pIAccessor);
			SAFE_RELEASE(pIRowset);
			SAFE_FREE(pBindings);
			SAFE_FREE(pData1);
			SAFE_FREE(pData2);
		}

		SAFE_FREE(pwszSqlStmt);
		SAFE_FREE(pCols);
	}

	TPASS;

CLEANUP:

	if(!g_fReadOnlyProvider)
	{
		// Adjust table object's row count 
		m_pTable->SubtractRow();
		// Delete the duplicate row (deletes both rows)
		CHECK(m_pTable->Delete(m_pTable->GetNextRowNumber()-1), S_OK);
		// Now reinsert one row
		CHECK(m_pTable->Insert(m_pTable->GetNextRowNumber()), S_OK);
	}

	if (phRows && pIRowset)
		pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL);

	if (pData1)
		CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData1), S_OK);
	if (pData2)
		CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData2), S_OK);

	if (hAccessor != DB_NULL_HACCESSOR)
	{
		pIAccessor->ReleaseAccessor(hAccessor, NULL);
		hAccessor = DB_NULL_HACCESSOR;
	}
	SAFE_FREE(pBindings);
	SAFE_FREE(pData1);
	SAFE_FREE(pData2);
	SAFE_FREE(phRows);
	SAFE_FREE(pCols);
	SAFE_FREE(pwszSqlStmt);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pICmd1);
	
	TRETURN;

} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED - select distinct with DBPROP_UNIQUEROWS REQUIRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_73()
{ 
	ICommand*	pICmd1=NULL;
	ICommandText * pICommandText = NULL;
	IAccessor * pIAccessor = NULL;
	DBORDINAL cCols;
	DBCOUNTITEM cRowsObtained;
	ULONG iCol;
	LONG cRows = LONG_MAX;
	HROW * phRows = NULL;
	DB_LORDINAL * pCols = NULL;
	IRowset * pIRowset = NULL;
	WCHAR * pwszSqlStmt = NULL;
	HRESULT hr, hrExec, hrExecLong, hrUnique, hrCreateStmt;
	ULONG iRow, jRow;
	BOOL fDup = FALSE;
	BYTE * pData1 = NULL;
	BYTE * pData2 = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBBINDING * pBindings = NULL;
	DBCOUNTITEM cBindings;
	DBLENGTH cbRowSize;
	ULONG_PTR ulHiddenColumns = 0;

	TESTBEGIN;

	ONLYROWSETVAR;

	// If the provider doesn't understand SQL then skip test if not using ini file
	if(m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE &&
		!GetModInfo()->GetFileName())
	{
		odtLog << L"Provider doesn't support SQL and not using ini file.\n";
		return TEST_SKIPPED;
	}


	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC(VerifyInterface(pICmd1, IID_ICommandText,
						COMMAND_INTERFACE, (IUnknown**)&pICommandText));

	// Try to insert a duplicate row.  This will fail for r/o providers.
	if (!g_fReadOnlyProvider)
		TESTC_(m_pTable->Insert(m_pTable->GetNextRowNumber()-1), S_OK);

	// Since providers will likely try to allocate cRows * sizeof(HROW) for this case,
	// and max allocation size if LONG_MAX (release), or less than LONG_MAX (debug)
	cRows = LONG_MAX/(sizeof(HROW)*100);	

	// Set CANHOLDROWS on so we can hold more than one row on some providers (prop is required)
	TESTC_(SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_CANHOLDROWS, TRUE), S_OK);

	// Set UNIQUEROWS on because some providers fail to retrieve "distinct" values when on
	if (S_OK == SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_UNIQUEROWS, TRUE))
		hrUnique = DB_E_ERRORSOCCURRED;
	else
		hrUnique = S_OK;

	for (iCol = 1; iCol <= g_Table2->CountColumnsOnTable(); iCol++)
	{
		CCol TempCol;

		TESTC_(m_pTable->GetColInfo(iCol, TempCol), S_OK);

		// Some providers can't do an order-by on a LONG column, so allow an error
		if (TempCol.GetIsLong())
			hrExecLong = DB_E_ERRORSINCOMMAND;
		else
			hrExecLong = S_OK;

		// Set command text for a select distinct query.
		hrCreateStmt = m_pTable->CreateSQLStmt(SELECT_DISTINCTCOLLISTORDERBY, NULL, &pwszSqlStmt,&cCols,&pCols, 0, NULL, iCol);

		TESTC_PROVIDER(hrCreateStmt == S_OK);

		TESTC_(pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSqlStmt), S_OK);

		// Retrieve all rows that match the requested row.
		hr = pICmd1->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown **)&pIRowset);

		// If there are no hidden columns then the fact the property was set is immaterial
		if (pIRowset && S_OK == GetProperty(DBPROP_HIDDENCOLUMNS, DBPROPSET_ROWSET, pIRowset, &ulHiddenColumns) &&
			!ulHiddenColumns)
			hrUnique = S_OK;
		
		// If the property was successfully set, then we really expect Execute can't
		// support UNIQUEROWS and distinct.
		hrExec = hrUnique;

		// But if we think Execute will succeed we still need to take into account
		// the column may be long
		if (hrExec == S_OK)
			hrExec = hrExecLong;

		CHECK(hr, hrExec);

		// If we got DB_E_ERRORSOCCURRED then we expect properties in error
		if (hr == DB_E_ERRORSOCCURRED)
			COMPARE(PropertiesInError(pICmd1, hr), TRUE);			

		// If we succeeded then validate results
		if (S_OK == hr)
		{

			TESTC(VerifyInterface(pIRowset, IID_IAccessor,
								ROWSET_INTERFACE, (IUnknown**)&pIAccessor));

			TESTC_(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
				&hAccessor, &pBindings, &cBindings, &cbRowSize,			
  				DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
				ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
				NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

			// Allocate a couple of row buffers for comparisons
			SAFE_ALLOC(pData1, BYTE, cbRowSize);
			SAFE_ALLOC(pData2, BYTE, cbRowSize);
			
			// Retrieve all the rows and validate no duplicates
			while (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
			{
				TESTC_(hr = pIRowset->GetNextRows(NULL, 0, cRows, &cRowsObtained, &phRows), DB_S_ENDOFROWSET);

				TESTC(phRows != NULL);

				TESTC(cRowsObtained > 0);

				if (phRows)
				{
					for (iRow = 0; iRow < cRowsObtained; iRow++)
					{
						TESTC_(pIRowset->GetData(phRows[iRow], hAccessor, pData1), S_OK);

						for (jRow = 0; jRow < cRowsObtained; jRow++)
						{
							TESTC_(pIRowset->GetData(phRows[jRow], hAccessor, pData2), S_OK);

							// Of course we expect corresponding rows to match
							if (iRow == jRow)
							{
								TESTC(CompareBuffer(pData1, pData2, cBindings, pBindings, NULL, 0, 0, COMPARE_ONLY));
							}
							else
							{
								// TESTC(!CompareBuffer(pData1, pData2, cBindings, pBindings, NULL, 0, 0, COMPARE_ONLY));
								BOOL fCmp = CompareBuffer(pData1, pData2, cBindings, pBindings, NULL, FALSE, FALSE, COMPARE_ONLY, FALSE, 0, NULL, TRUE);
								if (fCmp)
									odtLog << L"Data compared when it should not have, row " << iRow << " and row " << jRow << "\n";
								TESTC(!fCmp);
							}

							TESTC_(ReleaseInputBindingsMemory(cBindings, pBindings, pData2), S_OK);
						}

						TESTC_(ReleaseInputBindingsMemory(cBindings, pBindings, pData1), S_OK);
					}

					TESTC_(pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);
					SAFE_FREE(phRows);
				}
			}

			if (hAccessor != DB_NULL_HACCESSOR)
			{
				pIAccessor->ReleaseAccessor(hAccessor, NULL);
				hAccessor = DB_NULL_HACCESSOR;
			}
			SAFE_RELEASE(pIAccessor);
			SAFE_RELEASE(pIRowset);
			SAFE_FREE(pBindings);
			SAFE_FREE(pData1);
			SAFE_FREE(pData2);
		}
		SAFE_FREE(pwszSqlStmt);
		SAFE_FREE(pCols);
	}

	TPASS;

CLEANUP:

	if(!g_fReadOnlyProvider)
	{
		// Adjust table object's row count 
		m_pTable->SubtractRow();
		// Delete the duplicate row (deletes both rows)
		CHECK(m_pTable->Delete(m_pTable->GetNextRowNumber()-1), S_OK);
		// Now reinsert one row
		CHECK(m_pTable->Insert(m_pTable->GetNextRowNumber()), S_OK);
	}

	if (phRows && pIRowset)
		pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL);

	if (pData1)
		CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData1), S_OK);
	if (pData2)
		CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData2), S_OK);

	if (hAccessor != DB_NULL_HACCESSOR)
	{
		pIAccessor->ReleaseAccessor(hAccessor, NULL);
		hAccessor = DB_NULL_HACCESSOR;
	}
	SAFE_FREE(pBindings);
	SAFE_FREE(pData1);
	SAFE_FREE(pData2);
	SAFE_FREE(phRows);
	SAFE_FREE(pCols);
	SAFE_FREE(pwszSqlStmt);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pICmd1);
	
	TRETURN;

} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - select with invalid group by clause
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_74()
{ 
	ICommand*	pICmd1=NULL;
	DBORDINAL	cRowsetCols;
	LONG * rgTableColOrds = NULL;
	IUnknown * pIUnk = NULL;
	IID		iid = IID_IRowset;
	HRESULT hrExec = E_FAIL;

	if(IFROW)
		iid = IID_IRow;

	// If the provider doesn't understand SQL then skip test if not using ini file
	if(m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE &&
		!GetModInfo()->GetFileName())
	{
		odtLog << L"Provider doesn't support SQL and not using ini file.\n";
		return TEST_SKIPPED;
	}

	TESTBEGIN;

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	hrExec = m_pTable->ExecuteCommand(SELECT_INVALIDGROUPBY, iid, NULL,
		NULL, &cRowsetCols, (DB_LORDINAL **)&rgTableColOrds, EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIUnk,
		&pICmd1);

	TESTC_PROVIDER(hrExec != DB_E_NOTSUPPORTED);

	TESTC_(hrExec, DB_E_ERRORSINCOMMAND);

	TPASS;

CLEANUP:
	SAFE_FREE(rgTableColOrds);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pICmd1);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ERRORSOCCURRED - select distinct with DBPROP_UNIQUEROWS OPTIONAL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_75()
{ 
	ICommand*	pICmd1=NULL;
	ICommandText * pICommandText = NULL;
	IAccessor * pIAccessor = NULL;
	DBORDINAL cCols;
	DBCOUNTITEM cRowsObtained;
	ULONG iCol;
	LONG cRows = LONG_MAX;
	HROW * phRows = NULL;
	DB_LORDINAL * pCols = NULL;
	IRowset * pIRowset = NULL;
	WCHAR * pwszSqlStmt = NULL;
	HRESULT hr, hrExec, hrCreateStmt;
	ULONG iRow, jRow;
	BOOL fDup = FALSE;
	BYTE * pData1 = NULL;
	BYTE * pData2 = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBBINDING * pBindings = NULL;
	DBCOUNTITEM cBindings;
	DBLENGTH cbRowSize;
	ULONG_PTR ulHiddenColumns = 0;
	BOOL fUniqueRows = FALSE;	// Assume we couldn't set the property on

	ONLYROWSETVAR;

	// If the provider doesn't understand SQL then skip test if not using ini file
	if(m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE &&
		!GetModInfo()->GetFileName())
	{
		odtLog << L"Provider doesn't support SQL and not using ini file.\n";
		return TEST_SKIPPED;
	}

	TESTBEGIN;

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC(VerifyInterface(pICmd1, IID_ICommandText,
						COMMAND_INTERFACE, (IUnknown**)&pICommandText));

	// Try to insert a duplicate row.  This will fail for r/o providers.
	if (!g_fReadOnlyProvider)
		TESTC_(m_pTable->Insert(m_pTable->GetNextRowNumber()-1), S_OK);

	// Since providers will likely try to allocate cRows * sizeof(HROW) for this case,
	// and max allocation size if LONG_MAX (release), or less than LONG_MAX (debug)
	cRows = LONG_MAX/(sizeof(HROW)*100);	

	// Set CANHOLDROWS on so we can hold more than one row on some providers (prop is required)
	TESTC_(SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_CANHOLDROWS, TRUE), S_OK);

	// Set UNIQUEROWS on because some providers fail to retrieve "distinct" values when on
	if (S_OK == SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_UNIQUEROWS,
		TRUE, DBPROPOPTIONS_OPTIONAL))
		fUniqueRows = TRUE;

	for (iCol = 1; iCol <= g_Table2->CountColumnsOnTable(); iCol++)
	{
		CCol TempCol;

		// If we were able to turn on UNIQUEROWS then we expect DB_S_ERRORSOCCURRED
		if (fUniqueRows)
			hrExec = DB_S_ERRORSOCCURRED;
		else
			hrExec = S_OK;

		// Set command text for a select distinct query.
		hrCreateStmt = m_pTable->CreateSQLStmt(SELECT_DISTINCTCOLLISTORDERBY, NULL, &pwszSqlStmt,&cCols,&pCols, 0, NULL, iCol);

		TESTC_PROVIDER(hrCreateStmt == S_OK);

		TESTC_(pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSqlStmt), S_OK);

		// Retrieve all rows that match the requested row.
		hr = pICmd1->Execute(NULL, IID_IRowset, NULL, NULL, (IUnknown **)&pIRowset);

		if (FAILED(hr))
		{
			// Providers generally can't do an order-by on a LONG column,
			// so allow an error to pass
			TESTC_(m_pTable->GetColInfo(iCol, TempCol), S_OK);
			if (TempCol.GetIsLong())
				hrExec = DB_E_ERRORSINCOMMAND;
		}
		else if (DB_S_ERRORSOCCURRED != hr)
		{
			// If there are no hidden columns then the fact the property was set is immaterial
			// Some providers will succeed since they actually don't have hidden columns and
			// so will not need to disallow the UNIQUEROWS property
			if (pIRowset && S_OK == GetProperty(DBPROP_HIDDENCOLUMNS, DBPROPSET_ROWSET, pIRowset, &ulHiddenColumns) &&
				!ulHiddenColumns)
				hrExec = S_OK;
		}
		
		CHECK(hr, hrExec);

		// If we got DB_S_ERRORSOCCURRED then we expect properties in error
		if (hr == DB_S_ERRORSOCCURRED)
			COMPARE(PropertiesInError(pICmd1, hr), TRUE);			

		// If we succeeded then validate results
		if (SUCCEEDED(hr))
		{

			TESTC(VerifyInterface(pIRowset, IID_IAccessor,
								ROWSET_INTERFACE, (IUnknown**)&pIAccessor));

			TESTC_(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
				&hAccessor, &pBindings, &cBindings, &cbRowSize,			
  				DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
				ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
				NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, NULL, 
				NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, BLOB_LONG), S_OK);

			// Allocate a couple of row buffers for comparisons
			SAFE_ALLOC(pData1, BYTE, cbRowSize);
			SAFE_ALLOC(pData2, BYTE, cbRowSize);
			
			// Retrieve all the rows and validate no duplicates
			while (SUCCEEDED(hr) && hr != DB_S_ENDOFROWSET)
			{
				TESTC_(hr = pIRowset->GetNextRows(NULL, 0, cRows, &cRowsObtained, &phRows), DB_S_ENDOFROWSET);

				TESTC(phRows != NULL);

				TESTC(cRowsObtained > 0);

				if (phRows)
				{
					for (iRow = 0; iRow < cRowsObtained; iRow++)
					{
						TESTC_(pIRowset->GetData(phRows[iRow], hAccessor, pData1), S_OK);

						for (jRow = 0; jRow < cRowsObtained; jRow++)
						{
							TESTC_(pIRowset->GetData(phRows[jRow], hAccessor, pData2), S_OK);

							// Of course we expect corresponding rows to match
							if (iRow == jRow)
							{
								TESTC(CompareBuffer(pData1, pData2, cBindings, pBindings, NULL));
							}
							else
							{
								TESTC(!CompareBuffer(pData1, pData2, cBindings, pBindings, NULL));
							}

							TESTC_(ReleaseInputBindingsMemory(cBindings, pBindings, pData2), S_OK);
						}

						TESTC_(ReleaseInputBindingsMemory(cBindings, pBindings, pData1), S_OK);
					}

					TESTC_(pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);
					SAFE_FREE(phRows);
				}
			}

			if (hAccessor != DB_NULL_HACCESSOR)
			{
				pIAccessor->ReleaseAccessor(hAccessor, NULL);
				hAccessor = DB_NULL_HACCESSOR;
			}
			SAFE_RELEASE(pIAccessor);
			SAFE_RELEASE(pIRowset);
			SAFE_FREE(pBindings);
			SAFE_FREE(pData1);
			SAFE_FREE(pData2);
		}
		SAFE_FREE(pwszSqlStmt);
		SAFE_FREE(pCols);
	}

	TPASS;

CLEANUP:

	if(!g_fReadOnlyProvider)
	{
		// Adjust table object's row count 
		m_pTable->SubtractRow();
		// Delete the duplicate row (deletes both rows)
		CHECK(m_pTable->Delete(m_pTable->GetNextRowNumber()-1), S_OK);
		// Now reinsert one row
		CHECK(m_pTable->Insert(m_pTable->GetNextRowNumber()), S_OK);
	}

	if (phRows && pIRowset)
		pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL);

	if (pData1)
		CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData1), S_OK);
	if (pData2)
		CHECK(ReleaseInputBindingsMemory(cBindings, pBindings, pData2), S_OK);

	if (hAccessor != DB_NULL_HACCESSOR)
	{
		pIAccessor->ReleaseAccessor(hAccessor, NULL);
		hAccessor = DB_NULL_HACCESSOR;
	}
	SAFE_FREE(pBindings);
	SAFE_FREE(pData1);
	SAFE_FREE(pData2);
	SAFE_FREE(phRows);
	SAFE_FREE(pCols);
	SAFE_FREE(pwszSqlStmt);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pICmd1);
	
	TRETURN;

} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(76)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND - (Insert with mis-spelled keyword)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_76()
{ 
	ICommand*	pICmd1=NULL;
	IRowset * pRowset=INVALID(IRowset *);
	IID		iid = IID_IRowset;

	if(IFROW)
		iid = IID_IRow;

	// If the provider doesn't understand SQL then skip test if not using ini file
	if(m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE &&
		!GetModInfo()->GetFileName())
	{
		odtLog << L"Provider doesn't support SQL and not using ini file.\n";
		return TEST_SKIPPED;
	}

  	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	// Set command text to insert statement with invalid keyword (inot instead of into)
	m_hr = m_pTable->ExecuteCommand(INSERT_INVALID_KEYWORD, IID_IUnknown,
		NULL,NULL,NULL, NULL, EXECUTE_NEVER, 0, NULL, 
		NULL, NULL, &pICmd1);

	// Make sure there's support for this statement
	TEST_SUPPORTED(m_hr, S_OK);

	// Now execute expecting DB_E_ERRORSINCOMMAND
	TESTC_(pICmd1->Execute(NULL,iid,NULL,NULL,PPI &pRowset),DB_E_ERRORSINCOMMAND);

	TESTC(pRowset == NULL);
	
	TPASS
	
CLEANUP:
	SAFE_RELEASE(pICmd1);
	if (pRowset != INVALID(IRowset *))
		SAFE_RELEASE(pRowset);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(77)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Multiple commands, 2 initial and 1 later
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_77()
{ 
	// Some providers cannot support two open rowsets on the same command, which may cause a
	// bug where the third command object will fail also.  We allow the second command object to
	// fail execute in order to not unfairly penalize providers here.
	TBEGIN;

	HRESULT		hr = E_FAIL;
	HRESULT		hrExp = S_OK;
	IMultipleResults *	pMultRes1=NULL;
	IUnknown *	pUnk=NULL;
	DBROWCOUNT	cRowsAffected2=0;
	LPWSTR		pwszSqlStmt = NULL;
	LPWSTR		pwszSqlMult = NULL;
	ULONG_PTR	ulMultipleResults = 0;
	ICommand *	pICmd1=NULL;
	ICommand *	pICmd2=NULL;
	ICommand *	pICmd3=NULL;

	// Get a pointer to the session object.  There are no commands active at this time
	IDBCreateCommand * pIDBCreateCommand = (IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2;

	// Make sure we have a valid interface
	TESTC(pIDBCreateCommand != NULL);

	// Since this variation requires a multiple results object check for support
	if (!GetProperty(DBPROP_MULTIPLERESULTS, DBPROPSET_DATASOURCEINFO, 
										m_pMyIDBInitialize, &ulMultipleResults) ||
		ulMultipleResults == DBPROPVAL_MR_NOTSUPPORTED)
	{
		odtLog << L"Multiple results objects are not supported.\n";
		hrExp = E_NOINTERFACE;
	}

	// We don't know the syntax for non Sql Server multiple results objects
	if (!g_fSqlServer)
	{
		odtLog << L"Syntax for multiple results is only known for Sql Server.\n";
		TSKIPPED;
	}

	// Get two command objects off the session
	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,pIDBCreateCommand));
	TESTC(GetCommand(IID_ICommand,PPI &pICmd2,pIDBCreateCommand));

	TESTC(pICmd1 != NULL && pICmd2 != NULL);

	// Get statement with multiple results
	TESTC_(m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, NULL, &pwszSqlStmt,NULL,NULL), S_OK);

	// Allocate space for two copies of stmt plus a ';' and a null terminator
	SAFE_ALLOC(pwszSqlMult, WCHAR, wcslen(pwszSqlStmt)*2 + 2*sizeof(WCHAR));

	// Build the final statement
	wcscpy(pwszSqlMult, pwszSqlStmt);
	wcscat(pwszSqlMult, L";");
	wcscat(pwszSqlMult, pwszSqlStmt);

	// First session object's first command
	TESTC_(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSQL,SELECT_ALLFROMTBL,pwszSqlMult), S_OK);
	TESTC_(pICmd1->Execute(NULL,IID_IMultipleResults,NULL,NULL,PPI &pMultRes1),hrExp);

	// First session object's second command
	TESTC_(SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL), S_OK);
	hr = Execute(pICmd2,PPI &pUnk);

	// Don't check the hr for Execute, because at least one provider can't support a second open rowset on
	// the same session, but if a new command object is created then it can.  Nice implementation.

	// Third command object on same session
	TESTC(GetCommand(IID_ICommand,PPI &pICmd3,pIDBCreateCommand));

	TESTC(pICmd3 != NULL);

	// Third session object's command
	TESTC_(SetCommandText(m_pIMalloc,pICmd3,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL), S_OK);
	TESTC_(pICmd3->Execute(NULL,IID_NULL,NULL,&cRowsAffected2,NULL), S_OK);

	TPASS;

CLEANUP:	
	SAFE_FREE(pwszSqlStmt);
	SAFE_FREE(pwszSqlMult);
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pICmd3);
	SAFE_RELEASE(pMultRes1);
	SAFE_RELEASE(pUnk);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END

// {{ TCW_VAR_PROTOTYPE(78)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Create index in descending order
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_78()
{ 
	BOOL				fSuccess		= FALSE;	// Variation passed	or failed
	WCHAR*				pwszSQLStmt		= NULL;		// SQL Statement
	HRESULT				hr				= E_FAIL;	// HRESULT
	LPWSTR				pszStartTblName = NULL;
	LPWSTR				pwszIndexSuffix	= L"IdxDesc";
	WCHAR				wszIndexName[100] = L"";
	DBORDINAL			cIndexCols		=1;
	DB_LORDINAL			rgIndexCols[1];
	DB_LORDINAL *		prgIndexCols = &rgIndexCols[0];
	ULONG				iCol;
	DBORDINAL			cTableCols;
	CCol				TempCol;

	// If the provider doesn't understand SQL then skip test if not using ini file
	if(m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE &&
		!GetModInfo()->GetFileName())
	{
		odtLog << L"Provider doesn't support SQL and not using ini file.\n";
		return TEST_SKIPPED;
	}

	TBEGIN;

	// Find a column that will qualify for an index.  We'll use the first numeric column.
	cTableCols = m_pTable->CountColumnsOnTable();
	for (iCol = 1; iCol <= cTableCols; iCol++)
	{
		TESTC_(m_pTable->GetColInfo(iCol, TempCol), S_OK);

		if (IsNumericType(TempCol.GetProviderType()))
		{
			// Column index needed by CREATE_INDEX and CREATE_INDEX_DESC is 0 based.
			rgIndexCols[0] = iCol-1;	
			break;
		}
	}

	// Create the index name.  This will be the table name and the suffix, since we may already have
	// an index using the table name, and we need a unique value that won't collide with other index
	// names from other tests running simultaneously.
	wcscpy(wszIndexName, m_pTable->GetTableName());
	wcscat(wszIndexName, pwszIndexSuffix);

	hr = m_pTable->ExecuteCommand(CREATE_INDEX_DESC, IID_NULL, wszIndexName, NULL, &cIndexCols, &prgIndexCols);

	if (hr == DB_E_NOTSUPPORTED)
	{
		TESTC_PROVIDER(hr == S_OK);
	}

	TESTC_(hr, S_OK);

	TPASS;

CLEANUP:

	// Try to drop the index created above.  Some providers (Access/Jolt) have different syntax
	// for dropping indexes so don't check return code
	m_pTable->ExecuteCommand(DROP_INDEX, IID_NULL, wszIndexName, NULL, NULL, NULL);

	TRETURN;

} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(79)
//*-----------------------------------------------------------------------
// @mfunc S_OK: All rows with singleton select
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_79()
{ 
	TBEGIN
	IID				iid = IID_IRowset;
	ULONG			ulIndex = 0;
	DBCOUNTITEM		cRows = g_Table2->GetRowsOnCTable();
	BYTE*			pData = NULL;
	HROW*			rghRows = NULL;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBORDINAL		cColumns = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBCOUNTITEM		cTableBindings, iBind;
	DBORDINAL		cTableCols;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	HACCESSOR*		phAccessor = NULL;
	CRowObject*		pCRow = NULL;
	IUnknown*		pIUnk=NULL;
	HRESULT			hrCreateStmt = E_FAIL;
	WCHAR *			pwszSqlStmt = NULL;

	if(IFROW)
		iid = IID_IRow;

	hrCreateStmt = m_pTable->CreateSQLStmt(SELECT_ROW_WITH_LITERALS, NULL, &pwszSqlStmt,&cColumns,&rgColumnsOrd, 0, NULL, 1);

	SAFE_FREE(pwszSqlStmt);
	SAFE_FREE(rgColumnsOrd);

	TESTC_PROVIDER(hrCreateStmt == S_OK);

	TESTC(g_Table2->GetQueryInfo(SELECT_ALLFROMTBL, &cColumns, 
		&rgColumnsOrd, NULL,NULL,NULL,NULL))

	for(ulIndex=0; ulIndex<cRows; ulIndex++)
//	for(ulIndex=0; ulIndex<1; ulIndex++)
	{
		BLOBTYPE dwBlobType = NO_BLOB_COLS | BLOB_IID_IUNKNOWN;
		TESTC_(g_Table2->Select(NULL, ulIndex+1, iid, 0, NULL, NULL, PPI &pIUnk), S_OK)

		if(IFROW)
		{
			phAccessor = NULL;
			// Do not request IUnknown UDT for row object use, not supported
			dwBlobType |= BLOB_BIND_UDT_NO_IUNKNOWN;
		}
		else
			phAccessor = &hAccessor;

		TESTC_(GetAccessorAndBindings(pIUnk, 
			DBACCESSOR_ROWDATA,phAccessor, &rgBindings,
			&cBindings, &cbRowSize,	DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
			ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
			NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
			NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
			dwBlobType, 
			NULL),S_OK)

		//Allocate a new data buffer
		SAFE_ALLOC(pData, BYTE, cbRowSize);

		if(IFROWSET)
		{
			TESTC_(((IRowset*)pIUnk)->GetNextRows(NULL, 0, 2, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET)

			COMPARE(cRowsObtained, 1);

			TESTC(cRowsObtained > 0);
			//Initialize pData and call GetData for a row.
			memset(pData, 0, (size_t) cbRowSize);
			//We are sure that pIUnk is IRowset. Hence the
			//cast used below is safe.
			TESTC_(((IRowset*)pIUnk)->GetData(*rghRows, hAccessor, pData),S_OK)
			CHECK(((IRowset*)pIUnk)->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
		}

		if(IFROW)
		{
			pCRow = new CRowObject();
			TESTC(pCRow != NULL)
			TESTC_(pCRow->SetRowObject(pIUnk), S_OK)

			TESTC_(pCRow->GetColumns(cBindings, rgBindings, pData), S_OK)

		}

		// The first N columns in the rowset are columns from the table, for 
		// which we must use the proper ordinals in m_rgCompOrds.  Columns
		// N+1 to cBindings are extra row object columns if this is a row
		// object.  We cannot use m_rgCompOrds for these columns as they 
		// have no relation to the "base" columns in the table.
	
		cTableCols = g_Table2->CountColumnsOnTable();
		cTableBindings = cBindings;

		// Find the last binding within the table
		for (iBind = 0; iBind < cBindings; iBind++)
		{
			if (rgBindings[iBind].iOrdinal > cTableCols)
			{
				cTableBindings = iBind;
				break;
			}
		}
		
		//Verify data value, length and status are what is expected
		if (!COMPARE(CompareData(cTableCols, 
							rgColumnsOrd, 
							ulIndex+1, 
							pData, 
							cTableBindings, 
							rgBindings,
							g_Table2, 
							m_pIMalloc, 
							PRIMARY,
							COMPARE_ONLY), TRUE))			
		{		
			m_hr = E_FAIL;
			break;
		}

		// If there are extra bindings, then compare them without m_rgCompOrds
		// This is currently not possible with the existing test design, because
		// the test requires two tables.  With two tables, one cannot use an ini
		// file, and without using an ini file one cannot compare the row object's
		// extra columns.  So just print a warning until we can restructure the test.
		if (cBindings > cTableBindings)
		{
			odtLog << L"Extra row object columns not compared.\n";

			COMPAREW(0, 1);

			/*
			//Verify data value, length and status are what is expected
			if (!COMPARE(CompareData(0, 
								NULL, 
								ROW_SEED+i, 
								pData, 
								cBindings - cTableBindings, 
								rgBindings + cTableBindings,
								m_pSelectTable, 
								CMultResults::s_pIMalloc, 
								PRIMARY), TRUE))			
			{		
				m_hr = E_FAIL;
				break;
			}
			*/
		}

/*
		TESTC(CompareData(cColumns, rgColumnsOrd, ulIndex+1,
			pData, cBindings, rgBindings, g_Table2,
			m_pIMalloc, PRIMARY, COMPARE_ONLY))
*/

		CHECK(ReleaseInputBindingsMemory(cBindings, rgBindings, pData), S_OK);
		FreeAccessorBindings(cBindings, rgBindings);
		rgBindings = NULL;
		SAFE_FREE(pData);
		SAFE_FREE(rghRows);
		SAFE_DELETE(pCRow);
		SAFE_RELEASE(pIUnk);
//		pIUnk->Release();
	}

CLEANUP:
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	if (rgBindings)
		FreeAccessorBindings(cBindings, rgBindings);
	SAFE_FREE(rgColumnsOrd);
	SAFE_FREE(pData);
	SAFE_DELETE(pCRow);
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(80)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Select ABC and Col List
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_80()
{ 
	TBEGIN
	HRESULT			hr = E_FAIL;
	LONG_PTR		rgColsToBind[1];
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	HACCESSOR*		phAccessor = &hAccessor;
	DBCOUNTITEM		cRowsObtained = 0;
	DBCOUNTITEM		cGotRows = 0;
	HROW*			rghRows = NULL;
	BYTE*			pData = NULL;
	CRowObject*		pCRow = NULL;
	ICommand*		pICmd1=NULL;
	IUnknown*		pIUnk=NULL;

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	if(IFROW)
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);
	
	TESTC_(SetCommandText(m_pIMalloc,pICmd1,g_Table2,NULL,eSELECT,SELECT_ABCANDCOLLIST,NULL), S_OK)

	TESTC_(m_hr = Execute(pICmd1,PPI &pIUnk),S_OK);
	TESTC(pIUnk != NULL);

	rgColsToBind[0] = 1;

	if(IFROW)
		phAccessor = NULL;

	TESTC_(GetAccessorAndBindings(pIUnk, 
		DBACCESSOR_ROWDATA,phAccessor, &rgBindings,
		&cBindings, &cbRowSize,	DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 1, rgColsToBind, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	COMPARE(rgBindings[0].wType == DBTYPE_WSTR ||
		rgBindings[0].wType == DBTYPE_STR, TRUE);

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize);

	if(IFROWSET)
	{
		while(S_OK==(((IRowset*)pIUnk)->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows)))
		{
			COMPARE(cRowsObtained, 1);
			//Initialize pData and call GetData for a row.
			memset(pData, 0, (size_t) cbRowSize);
			//We are sure that pIUnk is IRowset. Hence the
			//cast used below is safe.
			TESTC_(((IRowset*)pIUnk)->GetData(*rghRows, hAccessor, pData),S_OK)
			CHECK(((IRowset*)pIUnk)->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);

			if(rgBindings[0].wType == DBTYPE_WSTR)
				TESTC(wcscmp((WCHAR*)((BYTE *)pData+rgBindings[0].obValue), L"ABC") == 0)
			else if(rgBindings[0].wType == DBTYPE_STR)
				TESTC(strcmp((CHAR*)((BYTE *)pData+rgBindings[0].obValue), "ABC") == 0)

			CHECK(ReleaseInputBindingsMemory(cBindings, rgBindings, pData), S_OK);
			SAFE_FREE(rghRows);
			cGotRows++;
		}

		COMPARE(cGotRows, g_Table2->GetRowsOnCTable());
	}

	if(IFROW)
	{
		pCRow = new CRowObject();
		TESTC(pCRow != NULL)
		TESTC_(pCRow->SetRowObject(pIUnk), S_OK)

		TESTC_(pCRow->GetColumns(cBindings, rgBindings, pData), S_OK)

		if(rgBindings[0].wType == DBTYPE_WSTR)
			TESTC(wcscmp((WCHAR*)((BYTE *)pData+rgBindings[0].obValue), L"ABC") == 0)
		else if(rgBindings[0].wType == DBTYPE_STR)
			TESTC(strcmp((CHAR*)((BYTE *)pData+rgBindings[0].obValue), "ABC") == 0)
	}

CLEANUP:
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	FreeAccessorBindings(cBindings, rgBindings);
	SAFE_FREE(pData);
	SAFE_DELETE(pCRow);
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(81)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Select duplicate columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_81()
{ 
	TBEGIN
	IID				iid = IID_IRowset;
	DBORDINAL		ulCol = 0;
	BYTE*			pData = NULL;
	HROW*			rghRows = NULL;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBORDINAL		cColumns = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBCOUNTITEM		cBindingsOrg = 0;
	DBBINDING*		rgBindings = NULL;
	DBBINDING*		rgBindingsOrg = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	HACCESSOR*		phAccessor = NULL;
	CRowObject*		pCRow = NULL;
	IUnknown*		pIUnk=NULL;
	ICommand*		pICmd1=NULL;
	BLOBTYPE dwBlobType = NO_BLOB_COLS;

	if(IFROW)
	{
		iid = IID_IRow;
		// Do not request IUnknown UDT for row object use, not supported
		dwBlobType |= BLOB_BIND_UDT_NO_IUNKNOWN;
	}

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cColumns, 
		&rgColumnsOrd, NULL,NULL,NULL,NULL))

	TESTC_(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_DUPLICATECOLUMNS,NULL), S_OK)

	TESTC_(m_hr = Execute(pICmd1,PPI &pIUnk),S_OK);

	if(IFROW)
		phAccessor = NULL;
	else
		phAccessor = &hAccessor;

	TESTC_(GetAccessorAndBindings(pIUnk, 
		DBACCESSOR_ROWDATA,phAccessor, &rgBindingsOrg,
		&cBindingsOrg, &cbRowSize,	DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		dwBlobType, NULL),S_OK)

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize);

	if(IFROWSET)
	{
		TESTC_(((IRowset*)pIUnk)->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows), S_OK)

		COMPARE(cRowsObtained, 1);
		//Initialize pData and call GetData for a row.
		memset(pData, 0, (size_t) cbRowSize);
		//We are sure that pIUnk is IRowset. Hence the
		//cast used below is safe.
		TESTC_(((IRowset*)pIUnk)->GetData(*rghRows, hAccessor, pData),S_OK)
		CHECK(((IRowset*)pIUnk)->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
	}

	if(IFROW)
	{
		pCRow = new CRowObject();
		TESTC(pCRow != NULL)
		TESTC_(pCRow->SetRowObject(pIUnk), S_OK)

		TESTC_(pCRow->GetColumns(cBindingsOrg, rgBindingsOrg, pData), S_OK)
	}

	cBindings = cBindingsOrg/2;
	rgBindings = rgBindingsOrg;

	TESTC(CompareData(cColumns, rgColumnsOrd, 1,
		pData, cBindings, rgBindings, m_pTable,
		m_pIMalloc, PRIMARY, COMPARE_ONLY))

	rgBindings = &(rgBindingsOrg[cBindings]);
	for(ulCol=0; ulCol<cBindings; ulCol++)
		rgBindings[ulCol].iOrdinal = rgBindingsOrg[ulCol].iOrdinal;

	TESTC(CompareData(cColumns, rgColumnsOrd, 1,
		pData, cBindings, rgBindings, m_pTable,
		m_pIMalloc, PRIMARY, COMPARE_ONLY))

CLEANUP:
	ReleaseInputBindingsMemory(cBindingsOrg, rgBindingsOrg, pData);
	FreeAccessorBindings(cBindingsOrg, rgBindingsOrg);
	SAFE_FREE(pData);
	SAFE_FREE(rghRows);
	SAFE_DELETE(pCRow);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pICmd1);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(82)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Select Count
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_82()
{ 
	TBEGIN
	HRESULT			hr = E_FAIL;
	LONG_PTR		rgColsToBind[1];
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBCOUNTITEM		cRowsObtained = 0;
	DBCOUNTITEM		cGotRows = 0;
	HROW*			rghRows = NULL;
	BYTE*			pData = NULL;
	CRowObject*		pCRow = NULL;
	ICommand*		pICmd1=NULL;
	IUnknown*		pIUnk=NULL;
	IAccessor*		pIA = NULL;

	//If the provider does not understand SQL, skip variation.
	if(g_Table2->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	if(IFROW)
		CHECK(SetRowsetProperty(pICmd1,DBPROPSET_ROWSET,
			DBPROP_IRow,TRUE,DBPROPOPTIONS_REQUIRED), S_OK);
	
	TESTC_(SetCommandText(m_pIMalloc,pICmd1,g_Table2,NULL,eSELECT,SELECT_COUNT,NULL), S_OK)

	TESTC_(m_hr = Execute(pICmd1,PPI &pIUnk),S_OK);
	TESTC(pIUnk != NULL);

	rgColsToBind[0] = 1;

	TESTC_(GetAccessorAndBindings(pIUnk, 
		DBACCESSOR_ROWDATA,NULL, &rgBindings,
		&cBindings, &cbRowSize,	DBPART_VALUE,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 1, rgColsToBind, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	if(IFROWSET)
	{
		TESTC(VerifyInterface(pIUnk, IID_IAccessor, ROWSET_INTERFACE, PPI &pIA))
		if(rgBindings[0].wType != DBTYPE_I4)
			rgBindings[0].wType = DBTYPE_WSTR;
		TESTC_(pIA->CreateAccessor(DBACCESSOR_ROWDATA, 
			cBindings, rgBindings, cbRowSize, &hAccessor,NULL), S_OK)
	}

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize);

	if(IFROWSET)
	{
		TESTC_(((IRowset*)pIUnk)->GetNextRows(NULL, 0, 2, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET)

		COMPARE(cRowsObtained, 1);
		//Initialize pData and call GetData for a row.
		memset(pData, 0, (size_t) cbRowSize);
		//We are sure that pIUnk is IRowset. Hence the
		//cast used below is safe.
		TESTC_(((IRowset*)pIUnk)->GetData(*rghRows, hAccessor, pData),S_OK)
		CHECK(((IRowset*)pIUnk)->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
	}

	if(IFROW)
	{
		pCRow = new CRowObject();
		TESTC(pCRow != NULL)
		TESTC_(pCRow->SetRowObject(pIUnk), S_OK)

		TESTC_(pCRow->GetColumns(cBindings, rgBindings, pData), S_OK)
	}

	if(rgBindings[0].wType == DBTYPE_I4)
		COMPARE(*(ULONG*)((BYTE *)pData+rgBindings[0].obValue), g_Table2->GetRowsOnCTable());
	else
		COMPARE((ULONG)_wtoi((WCHAR*)((BYTE *)pData+rgBindings[0].obValue)), g_Table2->GetRowsOnCTable());

CLEANUP:
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	FreeAccessorBindings(cBindings, rgBindings);
	SAFE_FREE(rghRows);
	SAFE_FREE(pData);
	SAFE_DELETE(pCRow);
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIA);
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(83)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Select empty rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_83()
{ 
	TBEGIN
	HROW*			rghRows = NULL;
	DBCOUNTITEM		cRowsObtained = 0;
	ICommand*		pICmd1=NULL;
	IUnknown*		pIUnk=NULL;

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC_(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_EMPTYROWSET,NULL), S_OK)

	TEST2C_(m_hr = Execute(pICmd1,PPI &pIUnk),S_OK, DB_E_NOTFOUND);

	if(IFROW)
	{
		TESTC_(m_hr, DB_E_NOTFOUND)
		goto CLEANUP;
	}

	TESTC_(m_hr, S_OK)
	TESTC(pIUnk != NULL)

	TESTC_(((IRowset*)pIUnk)->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET)
	COMPARE(cRowsObtained, 0);

CLEANUP:
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(84)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Select Change col name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_84()
{ 
	TBEGIN
	IID				iid = IID_IRowset;
	BYTE*			pData = NULL;
	HROW*			rghRows = NULL;
	DB_LORDINAL*	rgColumnsOrd = NULL;
	DBORDINAL		cColumns = 0;
	DBCOUNTITEM		cRowsObtained = 0;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	HACCESSOR*		phAccessor = NULL;
	CRowObject*		pCRow = NULL;
	IUnknown*		pIUnk=NULL;
	ICommand*		pICmd1=NULL;

	if(IFROW)
		iid = IID_IRow;

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC(m_pTable->GetQueryInfo(SELECT_ALLFROMTBL, &cColumns, 
		&rgColumnsOrd, NULL,NULL,NULL,NULL))


	// Renaming the columns destroys our ability to look up UDT columns by column name.  So we will create our accessor using
	// non-renamed columns and just use it again after renaming the columns.  It should still work...
	TESTC_(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL), S_OK)

	TESTC_(m_hr = Execute(pICmd1,PPI &pIUnk),S_OK);

	if(IFROW)
		phAccessor = NULL;
	else
		phAccessor = &hAccessor;

	SAFE_RELEASE(pIUnk);

	TESTC_(GetAccessorAndBindings(pICmd1, 
		DBACCESSOR_ROWDATA,phAccessor, &rgBindings,
		&cBindings, &cbRowSize,	DBPART_VALUE,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF,
		NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
		NULL,NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM,
		NO_BLOB_COLS, NULL),S_OK)

	TESTC_(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_CHANGECOLNAME,NULL), S_OK)

	TESTC_(m_hr = Execute(pICmd1,PPI &pIUnk),S_OK);

	//Allocate a new data buffer
	SAFE_ALLOC(pData, BYTE, cbRowSize);

	if(IFROWSET)
	{
		TESTC_(((IRowset*)pIUnk)->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows), S_OK)

		COMPARE(cRowsObtained, 1);
		//Initialize pData and call GetData for a row.
		memset(pData, 0, (size_t) cbRowSize);
		//We are sure that pIUnk is IRowset. Hence the
		//cast used below is safe.
		TESTC_(((IRowset*)pIUnk)->GetData(*rghRows, hAccessor, pData),S_OK)
		CHECK(((IRowset*)pIUnk)->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
	}

	if(IFROW)
	{
		pCRow = new CRowObject();
		TESTC(pCRow != NULL)
		TESTC_(pCRow->SetRowObject(pIUnk), S_OK)

		TESTC_(pCRow->BindingsToColAccess(cBindings, rgBindings, &pCRow->m_pData, &pCRow->m_cColAccess, &pCRow->m_rgColAccess), S_OK);

		TESTC_(pCRow->GetColumns(cBindings, rgBindings, pData), S_OK)
	}

	TESTC(CompareData(cColumns, rgColumnsOrd, 1,
		pData, cBindings, rgBindings, m_pTable,
		m_pIMalloc, PRIMARY, COMPARE_ONLY))

CLEANUP:
	ReleaseInputBindingsMemory(cBindings, rgBindings, pData);
	FreeAccessorBindings(cBindings, rgBindings);
	SAFE_FREE(pData);
	SAFE_FREE(rghRows);
	SAFE_DELETE(pCRow);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pICmd1);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(85)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Aggregated rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_85()
{ 
	TESTRESULT		testresult = TEST_FAIL;
	HRESULT			hr = E_FAIL, hrSetProp = E_FAIL;
	DBORDINAL		cRowsetCols;
	DBORDINAL *		rgTableColOrds = NULL;
	IUnknown *		pIUnkInner = NULL;
	IUnknown *		pIUnk;
	IID				iid = IID_IRowset;

	if(IFROW)
	{
		iid = IID_IRow;

		// For aggregation case, since we must specify IID_IUnknown, we have to set DBPROP_IRow
		CHECK(SetRowsetProperty(m_pTable->m_pICommand, DBPROPSET_ROWSET, DBPROP_IRow, TRUE), S_OK);
	}

	CAggregate Aggregate(m_pTable->m_pICommand);

	// Kagera can't retrieve BLOB data without ACCESSORDER RANDOM.
	if (g_fKagera && SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(m_pTable->m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	// Set the command text to execute
	TESTC_(m_pTable->ExecuteCommand(SELECT_VALIDATIONORDER, IID_IRowset, NULL,
		NULL, &cRowsetCols, (DB_LORDINAL **)&rgTableColOrds, EXECUTE_NEVER, 0, NULL, NULL, (IUnknown**)&pIUnk, &m_pTable->m_pICommand), S_OK);

	TESTC_(hr = m_pTable->m_pICommand->Execute(&Aggregate, IID_IUnknown, NULL, NULL, &pIUnkInner), S_OK);

	Aggregate.SetUnkInner(pIUnkInner);
	TESTC(Aggregate.VerifyAggregationQI(hr, iid));

	// Get IRowset (or IRow) from the aggregated object
	TESTC_(Aggregate.QueryInterface(iid, (void **)&pIUnk), S_OK);

	if(IFROWSET)
		TESTC_(VerifyRowset(IID_IRowset, (IUnknown *)pIUnk, 1, 
			cRowsetCols, rgTableColOrds, FALSE), S_OK);

	if(IFROW)
		TESTC_(VerifyRowObj(IID_IRow, (IUnknown *)pIUnk, 1, 
			cRowsetCols, rgTableColOrds, FALSE), S_OK);

	// Now release our aggregated rowset object and IUnknown
	// We expect the aggregated object to go away when the last ref is released.
	SAFE_RELEASE(pIUnkInner);
	SAFE_RELEASE(pIUnk);

	TESTC(Aggregate.GetRefCount()==1);

	testresult = TEST_PASS;

CLEANUP:

	SAFE_RELEASE(pIUnkInner);
	SAFE_RELEASE(pIUnk);

	Aggregate.ReleaseInner();

	SAFE_FREE(rgTableColOrds);
	// Reset again just in case of failure above.
	if (hrSetProp == S_OK)
		CHECK(SetRowsetProperty(m_pTable->m_pICommand, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM, DBPROPOPTIONS_OPTIONAL), S_OK);

	// Set DBROP_IRow back to default value.  
	if(IFROW)
		CHECK(SetRowsetPropertyDefault(DBPROP_IRow, m_pTable->m_pICommand), S_OK);
	return testresult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(86)
//*-----------------------------------------------------------------------
// @mfunc Last Variation - Final Verification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int CExecute_Rowset::Variation_86()
{ 
	TBEGIN
	ICommand*	pICmd1=NULL;
	IUnknown*	pIUnk=NULL;
	WCHAR *		pSQL=NULL;
	DBORDINAL	cRowsetCols=0;
	DBORDINAL *		rgTableColOrds = NULL;
	HRESULT		hrSetProp = E_FAIL;

	TESTC(GetCommand(IID_ICommand,PPI &pICmd1,NULL));

	TESTC_(m_pTable->CreateSQLStmt(SELECT_VALIDATIONORDER,NULL,&pSQL,&cRowsetCols,(DB_LORDINAL **)&rgTableColOrds), S_OK)
	
	TESTC_(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSQL,NO_QUERY,pSQL), S_OK)

	// Some providers can't retrieve BLOB data without this property or IRowsetLocate on.  Can't turn on IRowsetLocate
	// for batch SQL as Sql Server driver will turn back off when it detects batch stmt.
	// If the property is not supported then this may fail, but there's nothing we can do about that
	if (g_fKagera && SupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, m_pThisTestModule->m_pIUnknown,SESSION_INTERFACE))
		hrSetProp = SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM);

	TESTC_(m_hr = Execute(pICmd1,PPI &pIUnk, NULL, NULL, NULL, IID_IRowset),S_OK);

	TESTC(pIUnk != NULL);

	//Make sure the rowset has correct number of rows,
	//and data.
	TESTC_(VerifyRowset(IID_IRowset, (IUnknown *)pIUnk, 1, 
		cRowsetCols, rgTableColOrds, FALSE), S_OK);

CLEANUP:

	SAFE_FREE(pSQL);
	SAFE_FREE(rgTableColOrds);
	SAFE_RELEASE(pIUnk);

	if (hrSetProp == S_OK)
		CHECK(SetRowsetProperty(pICmd1, DBPROPSET_ROWSET, DBPROP_ACCESSORDER, (LONG_PTR)DBPROPVAL_AO_RANDOM, DBPROPOPTIONS_OPTIONAL), S_OK);

	SAFE_RELEASE(pICmd1);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CExecute_Rowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCmd::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CDBSession)
//*-----------------------------------------------------------------------
//| Test Case:		CDBSession - general cases
//|	Created:			04/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CDBSession::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCmd::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: IRowsetInfo::GetSpecification, get same IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CDBSession::Variation_1()
{
	ICommand*	 pICmd1 = NULL;
	IRowsetInfo* pIRowsetInfo = NULL;
	ICommand*	 pICmd2 = NULL;
	IUnknown*	 pIUnknown = NULL;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = pICmd1->Execute(NULL,IID_IRowsetInfo,NULL,NULL,PPI &pIRowsetInfo),S_OK));
	CLEANUP(!CHECK(m_hr = pIRowsetInfo->GetSpecification(IID_ICommand,PPI &pICmd2),S_OK));
	CLEANUP(!CHECK(m_hr=pICmd2->GetDBSession(IID_IUnknown,&pIUnknown),S_OK));
	
	if(COMPARE(ValidateSessionObject(pIUnknown),TRUE))
		TPASS

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIUnknown);
	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Don't execute command, get same IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CDBSession::Variation_2()
{
	ICommand* pICmd1 = NULL;
	IUnknown* pIUnknown = NULL;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(!CHECK(m_hr=pICmd1->GetDBSession(IID_IUnknown,&pIUnknown),S_OK));
	
	if(COMPARE(ValidateSessionObject(pIUnknown),TRUE))
		TPASS

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pIUnknown);
	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE: dso iid, valid ptr
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CDBSession::Variation_4()
{
	ICommand*	   pICmd1 = NULL;
	IDBInitialize* pIDBInitialize = INVALID(IDBInitialize *);
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr=pICmd1->GetDBSession(IID_IDBInitialize,PPI &pIDBInitialize),E_NOINTERFACE));
	
	if(!pIDBInitialize)
		TPASS

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	if(pIDBInitialize != INVALID(IDBInitialize *))
		SAFE_RELEASE(pIDBInitialize);
	
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE: iid_null, valid ptr
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CDBSession::Variation_5()
{
	ICommand*		  pICmd1 = NULL;
	IDBCreateCommand* pIDBCreateCommand = INVALID(IDBCreateCommand *);
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr=pICmd1->GetDBSession(IID_NULL,PPI &pIDBCreateCommand),E_NOINTERFACE));
	
	if(!pIDBCreateCommand)
		TPASS

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	if(pIDBCreateCommand != INVALID(IDBCreateCommand *))
		SAFE_RELEASE(pIDBCreateCommand);
	
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: valid session id, ptr==NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CDBSession::Variation_6()
{
	ICommand * pICmd1 = NULL;
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECKW(m_hr=pICmd1->GetDBSession(IID_IDBCreateCommand,NULL),E_INVALIDARG));
	
	TPASS

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CDBSession::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCmd::Terminate());
}	// }}
// }}
// }}





// {{ TCW_TC_PROTOTYPE(Zombie)
//*-----------------------------------------------------------------------
//| Test Case:		Zombie - Induce zombie states
//|	Created:			02/02/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Zombie::Init()
{
	if(!CTransaction::Init())
		return TEST_SKIPPED;
	
	odtLog << L"CTransaction::Init succeeded\n";

	//This is a mandatory interface, it should always succeed
	return COMPARE(RegisterInterface(COMMAND_INTERFACE, IID_ICommand, 0, NULL), TRUE);
}
//*-----------------------------------------------------------------------
// @mfunc TestTxn
// Tests commit/abort with respect to IAccessor on commands
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::TestTxn
(						 
	ETXN eTxn,
	BOOL fRetaining
)
{
	
	BOOL				fSuccess = FALSE;
	ULONG				index=0;
	ICommandText *		pICommand=NULL;
	IRowset *			pIRowset=NULL;
	IUnknown *			pIDBSession=NULL;
	
	
	if (!StartTransaction(SELECT_ALLFROMTBL, (IUnknown **)&pICommand, 
						0, NULL, NULL, ISOLATIONLEVEL_READUNCOMMITTED))
		goto CLEANUP;

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

	//Make sure everything still works after commit or abort
	if (!CHECK(pICommand->Execute(NULL,IID_IRowset,NULL,NULL,PPI &pIRowset),S_OK))
		goto CLEANUP;

	if(!CHECK(pICommand->Cancel(),S_OK))
		goto CLEANUP;
		
	fSuccess = CHECK(pICommand->GetDBSession(IID_IDBCreateCommand,&pIDBSession),S_OK);

		

CLEANUP:

	SAFE_RELEASE(pIDBSession);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIRowset);

	//Return code of Commit/Abort will vary depending on whether
	//or not we have an open txn, so adjust accordingly
	if (fRetaining)
		CleanUpTransaction(S_OK);
	else
		CleanUpTransaction(XACT_E_NOTRANSACTION);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;

}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc State 1:Abort with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_1()
{
	return TestTxn(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc State 2:Abort with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_2()
{
	return TestTxn(ETXN_ABORT, FALSE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc State 3:Commit with fRetaining set to TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_3()
{
	return TestTxn(ETXN_COMMIT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc State 4:Commit with fRetaining set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Zombie::Variation_4()
{
	return TestTxn(ETXN_COMMIT, FALSE);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Zombie::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
}	// }}



// {{ TCW_TC_PROTOTYPE(CCancel)
//*-----------------------------------------------------------------------
//| Test Case:		CCancel - general cases
//|	Created:			04/08/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCancel::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCmd::Init())
	// }}
	{
		return TRUE;
	}

	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK,S_OK: 2 cmd obj (Select & Insert), cancel each during execution
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_1()
{
   	
	ICommand *	pICmd1=NULL;
	ICommand *  pICmd2=NULL;
	IRowset *	pIRowset = NULL;
	HRESULT		hr = NOERROR;
	ULONG iTime = 0;
	
	// Make sure the threading model is correct for this variation.  We only
	// support Free threaded.
	if (!VerifyThreadingModel(COINIT_MULTITHREADED))
		return TEST_SKIPPED;

 	TESTBEGIN;
	INIT_THRDS(FOUR_THRDS);

		
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd2,NULL));
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL)))
	
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL);
	else
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eINSERT,NO_QUERY,NULL);
	CLEANUP(FAILED(hr))
	{
		HRESULT hrExecute=DB_E_CANCELED;
		HRESULT hrExecuteOr = S_OK;		// Expect either hrExecute or hrExecuteOr result from Execute command.
		HRESULT hrCancel=S_OK;
		HRESULT hrCancelOr=DB_E_CANTCANCEL;
		STATEMENTKIND eStatementKind1 = eSELECT;
		STATEMENTKIND eStatementKind2 = eINSERT;
		
		if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
			eStatementKind2 = eSELECT;

		THRDARG ExecuteFirstCmd = { this, pICmd1,&hrExecute, &eStatementKind1, &hrExecuteOr};
		THRDARG CancelFirstCmd =  { this, pICmd1,&hrCancelOr, &eStatementKind1, &hrCancel};

		THRDARG ExecuteSecondCmd ={ this, pICmd2,&hrExecute, &eStatementKind2, &hrExecuteOr};
		THRDARG CancelSecondCmd = { this, pICmd2,&hrCancelOr, &eStatementKind2, &hrCancel};
		
		// Try 100 times to see if thread timing impacts results.
		for (iTime = 0; iTime < 100; iTime++)
		{
//			odtLog << L"Time # " << iTime+1 << L"\n";
			//Create Thread
			CREATE_THRD(THRD_ONE,Execute,&ExecuteFirstCmd);
			CREATE_THRD(THRD_TWO,Cancel,&CancelFirstCmd);

			CREATE_THRD(THRD_THREE,Execute,&ExecuteSecondCmd);
			CREATE_THRD(THRD_FOUR,Cancel,&CancelSecondCmd);

			//Start Thread
			START_THRDS();
			
			// End Thread
			END_THRDS();
		}
	}

	TPASS;

CLEANUP:

	odtLog << L"Times: " << iTime << L"\n";

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pIRowset);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK,S_OK: 2 cmd obj (Select & Insert), cancel 1 during, cancel 1 after execution
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_2()
{
	ICommand *	pICmd1=NULL;
	ICommand *  pICmd2=NULL;
	HRESULT		hr = NOERROR;
	
	// Make sure the threading model is correct for this variation.  We only
	// support Free threaded.
	if (!VerifyThreadingModel(COINIT_MULTITHREADED))
		return TEST_SKIPPED;

 	TESTBEGIN;
	INIT_THRDS(TWO_THRDS);

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd2,NULL))

	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL)));
	
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL);
	else
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eINSERT,NO_QUERY,NULL);
	
	CLEANUP(FAILED(hr))
	{ // First cmd object

		HRESULT hrExecute1=DB_E_CANCELED;
		HRESULT hrExecuteOr = S_OK; // Expect either Canceled or S_OK
		HRESULT hrCancel1=S_OK;
		HRESULT hrCancel1Or=DB_E_CANTCANCEL;
		STATEMENTKIND eStatementKind = eSELECT;

		THRDARG ExecuteFirstCmd = { this, pICmd1,&hrExecute1, &eStatementKind, &hrExecuteOr};
		THRDARG CancelFirstCmd =  { this, pICmd1,&hrCancel1, &eStatementKind, &hrCancel1Or};


		//Create Thread
		CREATE_THRD(THRD_ONE,Execute,&ExecuteFirstCmd);
		CREATE_THRD(THRD_TWO,Cancel,&CancelFirstCmd);
		//Start Thread
		START_THRDS();
		// End Thread
		END_THRDS();
		
	}

	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICmd2->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	hr = pICmd2->Cancel();
	if(!(hr==S_OK)&&!(hr==DB_E_CANTCANCEL))
		goto CLEANUP;

	TPASS;

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);	
	
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK,S_OK: 2 cmd obj (Select & Insert), cancel 1 before, cancel 1 during execution
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_3()
{
	ICommand *	pICmd1=NULL;
	ICommand *  pICmd2=NULL;
	HRESULT		hr = NOERROR;
	
	// Make sure the threading model is correct for this variation.  We only
	// support Free threaded.
	if (!VerifyThreadingModel(COINIT_MULTITHREADED))
		return TEST_SKIPPED;

	TESTBEGIN;
	INIT_THRDS(TWO_THRDS);

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd2,NULL));
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL)));
	
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL);
	else
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eINSERT,NO_QUERY,NULL);
	
	CLEANUP(FAILED(hr));
	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICmd2->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	hr = pICmd2->Cancel();
	if(!(hr==S_OK)&&!(hr==DB_E_CANTCANCEL))
		goto CLEANUP;

	{ // First cmd object

		HRESULT hrExecute1=DB_E_CANCELED;
		HRESULT hrExecuteOr=S_OK; // Expect DB_E_CANCELED or S_OK.
		HRESULT hrCancel1=S_OK;
		HRESULT hrCancel1Or=DB_E_CANTCANCEL;
		STATEMENTKIND eStatementKind1 = eSELECT;
		
		THRDARG ExecuteFirstCmd = { this, pICmd1,&hrExecute1, &eStatementKind1, &hrExecuteOr};
		THRDARG CancelFirstCmd =  { this, pICmd1,&hrCancel1, &eStatementKind1, &hrCancel1Or};


		//Create Thread
		CREATE_THRD(THRD_ONE,Execute,&ExecuteFirstCmd);
		CREATE_THRD(THRD_TWO,Cancel,&CancelFirstCmd);
		//Start Thread
		START_THRDS();
		// End Thread
		END_THRDS();
		
	}

	TPASS;

CLEANUP:
	
	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK,S_OK: 2 cmd obj, cancel 1 before, cancel 1 after
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_4()
{
	ICommand *	pICmd1=NULL;
	ICommand *  pICmd2=NULL;
	HRESULT		hr = NOERROR;
	
 	TESTBEGIN;
	
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd2,NULL))
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL)))

	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL);
	else
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eINSERT,NO_QUERY,NULL);

	CLEANUP(FAILED(hr));

	if(!CHECK(pICmd1->Cancel(),S_OK))
		goto CLEANUP;

	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
		goto CLEANUP;


	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICmd2->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	hr = pICmd2->Cancel();
	if(!(hr==S_OK)&&!(hr==DB_E_CANTCANCEL))
		goto CLEANUP;

	TPASS;

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: 1 cmd obj (Select), cancel before execution
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_5()
{
	ICommand *	pICmd1=NULL;

	
	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL)))
	
	if(!CHECK(pICmd1->Cancel(),S_OK))
		goto CLEANUP;

	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	TPASS;

CLEANUP:
	
	SAFE_RELEASE(pICmd1);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: 1 cmd obj (Select), cancel after execution
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_6()
{
	ICommand *	pICmd1=NULL;
	HRESULT hr;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL)))
	
	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	hr = pICmd1->Cancel();
	if(!(hr==S_OK)&&!(hr==DB_E_CANTCANCEL))
		goto CLEANUP;

	TPASS;

CLEANUP:
	
	SAFE_RELEASE(pICmd1);

	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: 1 cmd obj (Select), execute, cancel cancel
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_7()
{
	ICommand *	pICmd1=NULL;
	
	// Make sure the threading model is correct for this variation.  We only
	// support Free threaded.
	if (!VerifyThreadingModel(COINIT_MULTITHREADED))
		return TEST_SKIPPED;

 	TESTBEGIN;
	INIT_THRDS(THREE_THRDS);

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL))
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL)))
	
	{ // First cmd object

		HRESULT hrExecute1=DB_E_CANCELED;
		HRESULT hrExecuteOr=S_OK; // Expect S_OK or DB_E_CANCELED.
		HRESULT hrCancel1=S_OK;
		HRESULT hrCancel1Or=DB_E_CANTCANCEL;
		HRESULT hrCancel2 = S_OK;
		HRESULT hrCancel2Or=DB_E_CANTCANCEL;
		STATEMENTKIND eStatementKind = eSELECT;

		THRDARG ExecuteFirstCmd = { this, pICmd1,&hrExecute1, &eStatementKind, &hrExecuteOr};
		THRDARG CancelFirstCmd =  { this, pICmd1,&hrCancel1, &eStatementKind, &hrCancel1Or};
		THRDARG CancelFirstCmd2 = { this, pICmd1,&hrCancel2, &eStatementKind, &hrCancel2Or}; 


		//Create Thread
		CREATE_THRD(THRD_ONE,Execute,&ExecuteFirstCmd);
		CREATE_THRD(THRD_TWO,Cancel,&CancelFirstCmd);
		CREATE_THRD(THRD_THREE,Cancel,&CancelFirstCmd2);

		//Start Thread
		START_THRDS();

		// End Thread
		END_THRDS();
		
	}

	TPASS;

CLEANUP:
	
	SAFE_RELEASE(pICmd1);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: 1 cmd obj (Insert), cancel before execution
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_8()
{
	ICommand *	pICmd1=NULL;
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	if(!CHECK(pICmd1->Cancel(),S_OK))
		goto CLEANUP;

	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	TPASS;

CLEANUP:
	
	SAFE_RELEASE(pICmd1);
	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: 1 cmd obj (Insert), cancel after execution
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_9()
{
	ICommand *	pICmd1=NULL;
	HRESULT hr;
	
	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),S_OK))
		goto CLEANUP;

	hr = pICmd1->Cancel();
	if((hr!=S_OK) && (hr!=DB_E_CANTCANCEL))
	{
		// We need to post an error here
		CHECK(hr, S_OK);
		goto CLEANUP;
	}

	TPASS;

CLEANUP:
	
	SAFE_RELEASE(pICmd1);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: 1 cmd obj (Insert), execute, cancel cancel
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_10()
{
	ICommand *	pICmd1=NULL;

	//If the provider is read only or does not understand SQL, skip variation.
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		return TEST_SKIPPED;

	// Make sure the threading model is correct for this variation.  We only
	// support Free threaded.
	if (!VerifyThreadingModel(COINIT_MULTITHREADED))
		return TEST_SKIPPED;

 	TESTBEGIN;
	INIT_THRDS(THREE_THRDS);

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eINSERT,NO_QUERY,NULL)));

	{ // First cmd object

		HRESULT hrExecute1=DB_E_CANCELED;
		HRESULT hrExecuteOr=S_OK;
		HRESULT hrCancel1=S_OK;
		HRESULT hrCancel2 = S_OK;
		HRESULT hrCancel1Or = DB_E_CANTCANCEL;
		HRESULT hrCancel2Or = DB_E_CANTCANCEL;
		STATEMENTKIND eStatementKind = eINSERT;

		THRDARG ExecuteFirstCmd = { this, pICmd1,&hrExecute1, &eStatementKind, &hrExecuteOr};
		THRDARG CancelFirstCmd =  { this, pICmd1,&hrCancel1, &eStatementKind, &hrCancel1Or};
		THRDARG CancelFirstCmd2 = { this, pICmd1,&hrCancel2, &eStatementKind, &hrCancel2Or}; 

		//Create Thread
		CREATE_THRD(THRD_ONE,Execute,&ExecuteFirstCmd);
		CREATE_THRD(THRD_TWO,Cancel,&CancelFirstCmd);
		CREATE_THRD(THRD_THREE,Cancel,&CancelFirstCmd2);

		//Start Thread
		START_THRDS();

		// End Thread
		END_THRDS();
		
	}

	TPASS;

CLEANUP:

	SAFE_RELEASE(pICmd1);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc No Query Set, 1 Cancel per Thread, Before Execution
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CCancel::Variation_11()
{
	ICommand *	pICmd1=NULL;
	
	TESTBEGIN;

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	
	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICmd1->Cancel(),S_OK))
		goto CLEANUP;

	if(!CHECK(pICmd1->Execute(NULL,IID_NULL,NULL,NULL,NULL),DB_E_NOCOMMAND))
		goto CLEANUP;

	TPASS;

CLEANUP:
	
	SAFE_RELEASE(pICmd1);

	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CCancel::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCmd::Terminate());
}	// }}
// }}
// }}



// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:			08/04/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL TCExtendedErrors::Init()
{
	ICommand			*pICommand = NULL;
	ISupportErrorInfo	*pISupportErrorInfo = NULL;
	HRESULT				hr = NOERROR;
	BOOL				fResult = TEST_FAIL;

 	//Create an object for checking extended errors, which will use
	//m_pError to increment the error count as needed.
	m_pExtError = new CExtError(m_pThisTestModule->m_ProviderClsid, m_pError);
	
	if (!m_pExtError)
		return FALSE;
	
 	CLEANUP(!CCmd::Init());
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICommand,NULL));
	CLEANUP(FAILED(hr=pICommand->QueryInterface(IID_ISupportErrorInfo, (void **)&pISupportErrorInfo) ));	
	fResult = TEST_PASS;

CLEANUP:
	if(hr==E_NOINTERFACE)
	{
		odtLog<<L"ISupportErrorInfo is not supported."<<ENDL;
		fResult = TEST_SKIPPED;
	}
	
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pISupportErrorInfo);

	return fResult;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid ICommand call with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	BOOL				fSuccess = FALSE;
	//ULONG				index=0;
	ICommand *			pICommand=NULL;
	IRowset *			pIRowset=NULL;
	IUnknown *			pIDBSession=NULL;
	
  	//For each method of the interface, first create an error object on
	//the current thread, then try get S_OK from the ICommand method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.

	TESTBEGIN
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICommand,NULL))

	//create an error object
	m_pExtError->CauseError();

	// First session object's command
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICommand,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	CLEANUP(!CHECK(m_hr = pICommand->Execute(NULL,IID_IRowset,NULL,NULL,PPI &pIRowset),S_OK));
		//Do extended check following Execute
		fSuccess = XCHECK(pICommand, IID_ICommand, m_hr);	

	//create an error object
	m_pExtError->CauseError();

	if(CHECK(m_hr=pICommand->Cancel(),S_OK))
		//Do extended check following Cancel
		fSuccess &= XCHECK(pICommand, IID_ICommand, m_hr);	

	//create an error object
	m_pExtError->CauseError();
	
	if(CHECK(m_hr=pICommand->GetDBSession(IID_IDBCreateCommand,&pIDBSession),S_OK))
		//Do extended check following GetDBSession
		fSuccess &= XCHECK(pICommand, IID_ICommand, m_hr);	

CLEANUP:

	SAFE_RELEASE(pIDBSession);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIRowset);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid Execute call with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	BOOL			fSuccess=TEST_FAIL;		
	ICommand*		pICommand=NULL;
	IRowset *		pRowset=NULL;
	WCHAR			wszSQL[]=L"Select * from";

	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ICommand method.
	//We then check extended errors to verify the right extended error behavior.
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICommand,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICommand,m_pTable,NULL,eSQL,NO_QUERY,(WCHAR *) wszSQL)));
	
	//create an error object
	m_pExtError->CauseError();
	
	//Command text contains error
	if(CHECK(m_hr = pICommand->Execute(NULL,IID_IRowset,NULL,NULL,PPI &pRowset),DB_E_ERRORSINCOMMAND))
	{
		//Do extended check following Execute
		fSuccess = XCHECK(pICommand, IID_ICommand, m_hr);	
	}
   	if(!pRowset)
		fSuccess &= TRUE;

CLEANUP:

	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pRowset);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}



// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetDBSession calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	BOOL			fSuccess=TEST_FAIL;		
	ICommand*		pICommand=NULL;
	IDBInitialize*	pIDBInitialize=NULL;

	
	//For each method of the interface, first create an error object on
	//the current thread, then try get a failure from the ICommand method.
	//We then check extended errors to verify the right extended error behavior.
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICommand,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICommand,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	
	//create an error object
	m_pExtError->CauseError();
	
	if(CHECK(m_hr=pICommand->GetDBSession(IID_IDBInitialize,PPI &pIDBInitialize),E_NOINTERFACE))
		//Do extended check following GetDBSession
		fSuccess = XCHECK(pICommand, IID_ICommand, m_hr);	
	
	if(!pIDBInitialize)
		fSuccess &= TRUE;

CLEANUP:

	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIDBInitialize);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Invalid GetDBSession calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_4()
{
	BOOL			fSuccess=TEST_FAIL;		
	ICommand*		pICommand=NULL;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ICommand method.
	//We then check extended errors to verify the right extended error behavior.
	
  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICommand,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICommand,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	if(CHECK(m_hr=pICommand->GetDBSession(IID_IDBCreateCommand,NULL),E_INVALIDARG))
		//Do extended check following GetDBSession
		fSuccess = XCHECK(pICommand, IID_ICommand, m_hr);	

CLEANUP:

	SAFE_RELEASE(pICommand);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE Execute call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_5()
{
	BOOL			fSuccess=TEST_FAIL;		
	ICommand*		pICommand=NULL;
	IRowset*		pIRowset=NULL;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ICommand method.
	//We then check extended errors to verify the right extended error behavior.

  	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICommand,NULL))
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICommand,m_pTable,NULL,eSELECT,SELECT_ALLFROMTBL,NULL)));
	if(CHECK(m_hr = pICommand->Execute(NULL,IID_IDBInitialize,NULL,NULL,PPI &pIRowset),E_NOINTERFACE))
		//Do extended check following Execute
		fSuccess = XCHECK(pICommand, IID_ICommand, m_hr);	

	if(!pIRowset)
		fSuccess &= TRUE;	

CLEANUP:

	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIRowset);
	
	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND Execute call with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_6()
{
	BOOL			fSuccess=TEST_FAIL;		
	ICommand*		pICommand=NULL;
	IRowset*		pIRowset=NULL;

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the ICommand method.
	//We then check extended errors to verify the right extended error behavior.
	
	TESTBEGIN

	CLEANUP(!GetCommand(IID_ICommand,PPI &pICommand,NULL))
	
	// Second cmd object
	// cRowsAffected is null on purpose
	if(!CHECK(pICommand->Cancel(),S_OK))
		goto CLEANUP;

	if(CHECK(m_hr=pICommand->Execute(NULL,IID_NULL,NULL,NULL,NULL),DB_E_NOCOMMAND))
	{
		//Do extended check following Execute
		fSuccess = XCHECK(pICommand, IID_ICommand, m_hr);	
	}


CLEANUP:
	
	SAFE_RELEASE(pICommand);

	if (fSuccess)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
//}}





// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANCELED Check extended error info when Execute is canceled
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCExtendedErrors::Variation_7()
{ 
   	
	ICommand *	pICmd1=NULL;
	ICommand *  pICmd2=NULL;
	IRowset *	pIRowset = NULL;
	HRESULT		hr = NOERROR;
	
	// Make sure the threading model is correct for this variation.  We only
	// support Free threaded.
	if (!VerifyThreadingModel(COINIT_MULTITHREADED))
		return TEST_SKIPPED;

	// In this variation we want to validate extended error information
	m_fValidateExtended = TRUE;

 	TESTBEGIN;
	INIT_THRDS(FOUR_THRDS);

		
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd1,NULL));
	CLEANUP(!GetCommand(IID_ICommand,PPI &pICmd2,NULL));
	
	// Set text in command object
	CLEANUP(FAILED(SetCommandText(m_pIMalloc,pICmd1,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL)))
	
	if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eSELECT,SELECT_COLLISTFROMTBL,NULL);
	else
		hr = SetCommandText(m_pIMalloc,pICmd2,m_pTable,NULL,eINSERT,NO_QUERY,NULL);
	CLEANUP(FAILED(hr))
	{
		HRESULT hrExecute=DB_E_CANCELED;
		HRESULT hrExecuteOr = S_OK;		// Expect either hrExecute or hrExecuteOr result from Execute command.
		HRESULT hrCancel=S_OK;
		HRESULT hrCancelOr=DB_E_CANTCANCEL;
		STATEMENTKIND eStatementKind1 = eSELECT;
		STATEMENTKIND eStatementKind2 = eINSERT;
		
		if( g_fReadOnlyProvider || m_pTable->GetSQLSupport() == DBPROPVAL_SQL_NONE )
			eStatementKind2 = eSELECT;

		THRDARG ExecuteFirstCmd = { this, pICmd1,&hrExecute, &eStatementKind1, &hrExecuteOr};
		THRDARG CancelFirstCmd =  { this, pICmd1,&hrCancelOr, &eStatementKind1, &hrCancel};

		THRDARG ExecuteSecondCmd ={ this, pICmd2,&hrExecute, &eStatementKind2, &hrExecuteOr};
		THRDARG CancelSecondCmd = { this, pICmd2,&hrCancelOr, &eStatementKind2, &hrCancel};

		//Create Thread
		CREATE_THRD(THRD_ONE,Execute,&ExecuteFirstCmd);
		CREATE_THRD(THRD_TWO,Cancel,&CancelFirstCmd);

		CREATE_THRD(THRD_THREE,Execute,&ExecuteSecondCmd);
		CREATE_THRD(THRD_FOUR,Cancel,&CancelSecondCmd);

		//Start Thread
		START_THRDS();
		
		// End Thread
		END_THRDS();
	}

	TPASS;

CLEANUP:

	SAFE_RELEASE(pICmd1);
	SAFE_RELEASE(pICmd2);
	SAFE_RELEASE(pIRowset);
	
	m_fValidateExtended = FALSE;

	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Terminate()
{
	//free error object
	if (m_pExtError)
		delete m_pExtError;
	m_pExtError = NULL;
	
	return (CCmd::Terminate());

}	// }}

// }}



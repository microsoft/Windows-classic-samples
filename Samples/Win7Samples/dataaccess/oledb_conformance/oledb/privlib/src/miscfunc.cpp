//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module MiscFunc Implementation Module | Source file for Miscellaneous Private Library Functions
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 06-30-95	Microsoft	Created <nl>
//	[02] 10-26-95	Microsoft	Added WSTR2DBTYPE <nl>
//	[03] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CCol Elements|
//
// @subindex CCol
//
//---------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#include "privstd.h"	//Precompiled header needed for all private library modules
#include "miscfunc.h"
#include "strings.h"
#include "CStorage.hpp"	//CStorage
#include <locale.h>


///////////////////////////////////////////////////////////////////////////
// Contstants
//
//////////////////////////////////////////////////////////////////////////
//An R8 has 53 bits of precision, 
//Upto 2 chars (2/17 chars) can be lost in string conversions, 
//so we allow opto 4 bits of conversion error = 1/(2^49) = 1.7763568394e-15
const double g_dDoubleTolerance = 1.7763568394e-15;
								   
	
///////////////////////////////////////////////////////////////////////////
// Globals
//
//////////////////////////////////////////////////////////////////////////
extern BOOL TESTB = TEST_PASS;


///////////////////////////////////////////////////////////////////////////
// Privlib Memory Allocation Routines...
//
//////////////////////////////////////////////////////////////////////////
//Toggle Spying
BOOL g_fUseIMallocSpy = FALSE;
BOOL SetUseIMallocSpy(BOOL fValue)		
{ 
	return g_fUseIMallocSpy = fValue; 
}

BOOL GetUseIMallocSpy()		
{ 
	return g_fUseIMallocSpy; 
}


typedef struct 
{
	HRESULT hrActual;
	HRESULT hrExpected;
} HR_MAP;


///////////////////////////////////////////////////////////////////////////
// FindWarning
//
//////////////////////////////////////////////////////////////////////////
BOOL FindWarning(HRESULT hrActual, HRESULT hrExpected, ULONG cHrMaps, const HR_MAP* rgHrMaps)
{
	ULONG i;
	for(i=0; i<cHrMaps; i++)
	{
		if(hrActual == rgHrMaps[i].hrActual && hrExpected == rgHrMaps[i].hrExpected)
			return TRUE;
	}

	return FALSE;
}
	
///////////////////////////////////////////////////////////////////////////
// IsWarning
//
//////////////////////////////////////////////////////////////////////////
DWORD IsWarning(HRESULT hrActual, HRESULT hrExpected)
{
	//For conformance purposes we don't want to fail a provider on conformance
	//if a "trival" error code does not match the expected code.  Many error codes
	//are important and indicate state of the object, others are merely for 
	//debugging purposes.  Also all success codes must match exactly.
	
	//no-op, If they are equal there is no warning, 
	if(hrExpected == hrActual)
		return FALSE;

	//A simple lookup table containing "trival" errors that will be treated
	//as just warnings to indicate violation of the OLE DB Spec, but nothing
	//major enough to fail a provider on conformance...


	//Warning Level 1 (most important warnings)
/*	const static HR_MAP rgWarningMap1[] = 
	{ 
		//Compiler will not allow empty const array
		//so when you have a warning for this level, uncomment
	};

	if(FindWarning(hrActual, hrExpected, NUMELEM(rgWarningMap1), rgWarningMap1))
		return WARNINGLEVEL_1;
*/
	//Warning Level 2 (less important warnings)
/*	const static HR_MAP rgWarningMap2[] = 
	{ 
		//Compiler will not allow empty const array
		//so when you have a warning for this level, uncomment
	};
	
	if(FindWarning(hrActual, hrExpected, NUMELEM(rgWarningMap2), rgWarningMap2))
		return WARNINGLEVEL_2;
*/
	//Warning Level 3 (lesser important warnings)
/*	const static HR_MAP rgWarningMap3[] = 
	{ 
		//Compiler will not allow empty const array
		//so when you have a warning for this level, uncomment
	};
	
	if(FindWarning(hrActual, hrExpected, NUMELEM(rgWarningMap3), rgWarningMap3))
		return WARNINGLEVEL_3;
*/
	
	//Warning Level 4 (least important warnings)
	const static HR_MAP rgWarningMap4[] = 
	{ 
		//Actual										//Expected
		E_INVALIDARG,									E_POINTER,
		E_POINTER,										E_INVALIDARG,
		DB_E_BADSTARTPOSITION,							DB_S_ENDOFROWSET, //Special Case for 1.x providers
		E_NOINTERFACE,									DB_E_NOAGGREGATION,
		
		HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER),		E_INVALIDARG,
		HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER),		E_POINTER,
		HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER),		STG_E_INVALIDPOINTER,
	};

	if(FindWarning(hrActual, hrExpected, NUMELEM(rgWarningMap4), rgWarningMap4))
		return WARNINGLEVEL_ALL;
	 
	return FALSE;
}


///////////////////////////////////////////////////////////////////////////
// Privlib Compare Routines...
//
//////////////////////////////////////////////////////////////////////////
BOOL PrivlibCompare(BOOL fEqual, BOOL fTreatAsWarning, WCHAR* pwszFileName, ULONG ulLine)
{
	BOOL bReturn = FALSE;

	//If this Asserts, you forgot to call CreateModInfo() or ModuleCreateDBSession()
	//in your ModuleInit(), this is required to setup global objects...
	CError* pCError = GetModInfo()->GetErrorObject();
	ASSERT(pCError); 

	//Determine if were dealing with an Error or Warning
	if(fTreatAsWarning && !(GetModInfo()->GetWarningLevel() == WARNINGLEVEL_ERROR))
	{
		//We don't have a way with "Compare" to indicate what are warnings 
		//(like we have with hresults with Validate).  So all we can really do is
		//show them or not depending upon if warnings = none...
		if(GetModInfo()->GetWarningLevel() == WARNINGLEVEL_NONE)
			return TRUE;
		
		//Save the Current ErrorLevel
		ERRORLEVEL eSavedErrorLevel = pCError->GetErrorLevel();
	
		//Treat as a Warning
		pCError->SetErrorLevel(HR_WARNING);
		pCError->Compare(fEqual, pwszFileName, ulLine);
		bReturn = TRUE; //Passed the Check, since its just a warning

		//Restore the Saved ErrorLevel
		pCError->SetErrorLevel(eSavedErrorLevel);
	}
	else
	{
		//Treat as an Error
		bReturn = pCError->Compare(fEqual, pwszFileName, ulLine);
	}

	return bReturn;
}


///////////////////////////////////////////////////////////////////////////
// Privlib Validate (HRESULT) Routines...
//
//////////////////////////////////////////////////////////////////////////
BOOL PrivlibValidate(HRESULT hrActual, HRESULT hrExpected, BOOL fTreatAsWarning, WCHAR* pwszFileName, ULONG ulLine)
{
	BOOL bReturn = FALSE;

	//If this Asserts, you forgot to call CreateModInfo() or ModuleCreateDBSession()
	//in your ModuleInit(), this is required to setup global objects...
	CError* pCError = GetModInfo()->GetErrorObject();
	ASSERT(pCError); 

	//Determine if this is a warning...
	DWORD dwWarning = IsWarning(hrActual, hrExpected);

	//If this was a warning, and this warning is at a higher
	//level than what were interested in, just exit without posting anything
	if(dwWarning && (dwWarning > GetModInfo()->GetWarningLevel()))
		return TRUE;
	
	//Determine if were dealing with an Error or Warning
	if((dwWarning || fTreatAsWarning) && !(GetModInfo()->GetWarningLevel() == WARNINGLEVEL_ERROR))
	{
		//If its a warning or the users wants to treat it as a warning (ie: TESTW)
		//And they have indicated to not show any warnings, exit.
		if(GetModInfo()->GetWarningLevel() == WARNINGLEVEL_NONE)
			return TRUE;

		//Save the Current ErrorLevel
		ERRORLEVEL eSavedErrorLevel = pCError->GetErrorLevel();
	
		//Treat as a Warning
		pCError->SetErrorLevel(HR_WARNING);
		pCError->Validate(hrActual, pwszFileName, ulLine, hrExpected);
		bReturn = TRUE; //Passed the Check, since its just a warning

		//Restore the Saved ErrorLevel
		pCError->SetErrorLevel(eSavedErrorLevel);
	}
	else
	{
		//Treat as an Error
		bReturn = pCError->Validate(hrActual, pwszFileName, ulLine, hrExpected);
	}

	return bReturn;
}


//--------------------------------------------------------------------
// @func Creates Data Source Object and Initializes to user provided Data Source,
// putting the new DSO Interface pointer into pThisTestModule->m_pIUnknown
// and the new DB Session Interface pointer to pThisTestModule->m_pIUnknown2.
//
// @rdesc Success or Failure
//	@flag TRUE  | The interfaces were retreived successfully.
//	@flag FALSE | The retrieval of the interfaces failed.
//
//--------------------------------------------------------------------
BOOL ModuleCreateDBSession(CThisTestModule * pThisTestModule)	//@parm [IN] | Test Module object 
{
	BOOL			fResults		= FALSE;
	HRESULT			hr				= NOERROR;

	IDBInitialize *	pIDBInitialize	= NULL;	
	IDBProperties *	pIDBProperties	= NULL;
	DBPROPSET *		rgPropSets		= NULL;
	ULONG			cPropSets		= 0;

	IDBBinderProperties *	pIDBBinderProperties = NULL;

	ASSERT(pThisTestModule);
	pThisTestModule->m_pIUnknown = NULL;
	pThisTestModule->m_pIUnknown2 = NULL;

	if(!CreateModInfo(pThisTestModule))
		goto CLEANUP;
	
	// Setup the arrays needed for init, based on string LTM passed to us
	TESTC(GetInitProps(&cPropSets, &rgPropSets));
	
	// Get our initial connection to the provider, asking for IDBInitialize since 
	// we must initialize before anything else
	TESTC_(hr = GetModInfo()->CreateProvider(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);

	// Get IDBProperties Pointer
	TESTC(VerifyInterface(pIDBInitialize, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties));

	// Set the properties before we Initialize, only if we have Properties to set...
	TESTC_(pIDBProperties->SetProperties(cPropSets, rgPropSets),S_OK);

	// Initialize
	TESTC_(hr = pIDBInitialize->Initialize(), S_OK);
	
	// Obtain IDBCreateSesson, placing the new DSO interface 
	// in CThisTestModule's m_pIUnknown, so that all testcases can use 
	// it via their back pointer to this object.  No need to call AddRef 
	// here as we will own it, rather than the test module.	 
	TESTC(VerifyInterface(pIDBInitialize, IID_IDBCreateSession, DATASOURCE_INTERFACE, (IUnknown**)&pThisTestModule->m_pIUnknown));
							 
	// Create a DB session object
	// Set the m_pIUnknown2 to IOpenRowset
	TESTC_(((IDBCreateSession*)pThisTestModule->m_pIUnknown)->CreateSession(
			NULL, IID_IOpenRowset, (IUnknown **)&pThisTestModule->m_pIUnknown2), S_OK);

	// By default, set the Root Binder so that it can connect to 
	// the datasource we just connected to
	if(VerifyInterface(GetModInfo()->GetRootBinder(), IID_IDBBinderProperties, BINDER_INTERFACE,(IUnknown**)&pIDBBinderProperties))
	{
		// Following is a warning
		// Let tests continue even if there is a problem with the root binder.
		if((hr = pIDBBinderProperties->SetProperties(cPropSets, rgPropSets)) != S_OK)
		{
			//Display any properties errors to the trace
			VerifyProperties(hr, cPropSets, rgPropSets, FALSE);
			
			//Display that the RootBinder failed for some reason...
			TOUTPUT(L"Root Binder Initialization failed. Some tests may not work properly.");
			//Count this as a warning...
			CHECKW(hr, S_OK);
		}

		// Set up the Conformance Provider
		TESTC(InitializeConfProv(pThisTestModule))
	}

	fResults = TRUE;

CLEANUP:
	
	// Clean up our variants we used in the init
	FreeProperties(&cPropSets, &rgPropSets);

	// Free pIDBProperties
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);	

	// Free pIDBBinderProperties
	SAFE_RELEASE(pIDBBinderProperties);

	if (!fResults)
	{
		// Release the Data Source object we created since we've failed
		if (pThisTestModule->m_pIUnknown)
			COMPAREW(pThisTestModule->m_pIUnknown->Release(), 0);
		pThisTestModule->m_pIUnknown = NULL;
	}
	
	return fResults;
}

//--------------------------------------------------------------------
// @func Performs module level cleanup, such as releasing the DSO
// and DB Session interfaces kept in pThisTestModule->m_pIUnknown and
// pThisTestModule->m_pIUnknown2.
//
// @rdesc Success or Failure
// 	@flag  TRUE  | Successful Termination
//	@flag  FALSE | Termination did not complete successfully
//
//--------------------------------------------------------------------
BOOL ModuleReleaseDBSession(CThisTestModule * pThisTestModule) //@parm [IN] | Test Module object 
{

	COMPARE(ConfProvTerminate(), TRUE);

	// Free the IDBCreateCommand interface we got in ModuleCreateDBSession()	
	// Check to make sure the Release knocks the ref count down to zero
 	if(pThisTestModule->m_pIUnknown2)
		COMPAREW(pThisTestModule->m_pIUnknown2->Release(), 0);

	// Free the IDBCreateSession interface we got in ModuleCreateDBSession()
	// Release the DataSource Object	
	// Check to make sure the Release knocks the ref count down to zero
	if (pThisTestModule->m_pIUnknown)
		COMPAREW(pThisTestModule->m_pIUnknown->Release(), 0);

	pThisTestModule->m_pIUnknown = NULL;
	pThisTestModule->m_pIUnknown2 = NULL;

	//ReleaseModInfo
	return ReleaseModInfo(pThisTestModule);
}


static CModInfo*	g_pCModInfo = NULL;
static CTable*		g_pConfProvTable = NULL;
static CTree*		g_pConfProvTree = NULL;


////////////////////////////////////////////////////////////////////////////////
// GetModInfo
//
////////////////////////////////////////////////////////////////////////////////
CModInfo* GetModInfo()
{
	ASSERT(g_pCModInfo);
	return g_pCModInfo;
}

CTable* GetRootTable()
{
	return g_pConfProvTable;
}

BOOL CreateModInfo(CThisTestModule* pCThisTestModule)
{
	//Construct a new CProvider object
	//Only if it is different than the last one
	if(g_pCModInfo==NULL || (pCThisTestModule != g_pCModInfo->GetThisTestModule()))
	{
		g_pCModInfo = new CModInfo();
		if(g_pCModInfo == NULL)
			return FALSE;

		//Init
		if(!g_pCModInfo->Init(pCThisTestModule))
			return FALSE;
	}

	return TRUE;
}

BOOL ReleaseModInfo(CThisTestModule* pCThisTestModule)
{
	SAFE_DELETE(g_pCModInfo);
	return TRUE;
}


//--------------------------------------------------------------------
// @mfunc GetInterfaceArray
//
// Returns an array of interface IIDs and associated info for a particular object
//
// @rdesc Success or Failure
// 	@flag  TRUE  | Required
//	@flag  FALSE | Not required
//--------------------------------------------------------------------
BOOL GetInterfaceArray(EINTERFACE eInterface, ULONG* pcInterfaces, INTERFACEMAP** prgInterfaces)
{
	ASSERT(pcInterfaces && prgInterfaces);

	//	CoClass Type					//Interface				//Mandatory		//Level			//PropertyID
	static const INTERFACEMAP rgUnknownMap[] = 
	{
		UNKNOWN_INTERFACE,				&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_0,	0,	
	};
	static const INTERFACEMAP rgEnumeratorMap[] = 
	{
		ENUMERATOR_INTERFACE,			&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_0,	0,		
		ENUMERATOR_INTERFACE,			&VALUE_WCHAR(IID_IParseDisplayName),		TRUE,	CONF_LEVEL_0,	0,	
		ENUMERATOR_INTERFACE,			&VALUE_WCHAR(IID_ISourcesRowset),			TRUE,	CONF_LEVEL_0,	0,	
		ENUMERATOR_INTERFACE,			&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgDataSourceMap[] = 
	{
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_0,	0,	
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_IDBCreateSession),			TRUE,	CONF_LEVEL_0,	0,	
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_IDBInitialize),			TRUE,	CONF_LEVEL_0,	0,	
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_IDBProperties),			TRUE,	CONF_LEVEL_0,	0,	
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_IPersist),					TRUE,	CONF_LEVEL_0,	0,	
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_IDBInfo),					FALSE,	CONF_LEVEL_1,	0,	
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_IConnectionPointContainer),FALSE,	CONF_LEVEL_1,	0,	
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_IDBAsynchStatus),			FALSE,	CONF_LEVEL_1,	0,	
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_IDBDataSourceAdmin),		FALSE,	CONF_LEVEL_1,	0,	
		DATASOURCE_INTERFACE,			&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgSessionMap[] = 
	{
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_0,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_IGetDataSource),			TRUE,	CONF_LEVEL_0,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_IOpenRowset),				TRUE,	CONF_LEVEL_0,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_ISessionProperties),		TRUE,	CONF_LEVEL_0,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_IDBCreateCommand),			FALSE,	CONF_LEVEL_1,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_IDBSchemaRowset),			FALSE,	CONF_LEVEL_1,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_ITransaction),				FALSE,	CONF_LEVEL_1,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_ITransactionJoin),			FALSE,	CONF_LEVEL_1,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_ITransactionLocal),		FALSE,	CONF_LEVEL_1,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_ITransactionObject),		FALSE,	CONF_LEVEL_1,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_ITableDefinition),			FALSE,	CONF_LEVEL_1,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_IIndexDefinition),			FALSE,	CONF_LEVEL_1,	0,	
		SESSION_INTERFACE,				&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgCommandMap[] = 
	{
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_IAccessor),				TRUE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_ICommand),					TRUE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_ICommandText),				TRUE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_IColumnsInfo),				TRUE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_IConvertType),				TRUE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_ICommandProperties),		TRUE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_ICommandWithParameters),	FALSE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_ICommandPrepare),			FALSE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_IColumnsRowset),			FALSE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
		COMMAND_INTERFACE,				&VALUE_WCHAR(IID_ICommandStream),			FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgRowsetMap[] = 
	{
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_0,	DBPROP_IRowset,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IAccessor),				TRUE,	CONF_LEVEL_0,	DBPROP_IAccessor,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IConvertType),				TRUE,	CONF_LEVEL_0,	DBPROP_IConvertType,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IColumnsInfo),				TRUE,	CONF_LEVEL_0,	DBPROP_IColumnsInfo,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowset),					TRUE,	CONF_LEVEL_0,	DBPROP_IRowset,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetInfo),				TRUE,	CONF_LEVEL_0,	DBPROP_IRowsetInfo,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetIdentity),			FALSE,	CONF_LEVEL_0,	DBPROP_IRowsetIdentity,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetChange),			FALSE,	CONF_LEVEL_1,	DBPROP_IRowsetChange,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IDBAsynchStatus),			FALSE,	CONF_LEVEL_1,	DBPROP_IDBAsynchStatus,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IConnectionPointContainer),FALSE,	CONF_LEVEL_1,	DBPROP_IConnectionPointContainer,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetFind),				FALSE,	CONF_LEVEL_1,	DBPROP_IRowsetFind,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetLocate),			FALSE,	CONF_LEVEL_1,	DBPROP_IRowsetLocate,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetRefresh),			FALSE,	CONF_LEVEL_1,	DBPROP_IRowsetRefresh,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetResynch),			FALSE,	CONF_LEVEL_1,	DBPROP_IRowsetResynch,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetUpdate),			FALSE,	CONF_LEVEL_1,	DBPROP_IRowsetUpdate,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetScroll),			FALSE,	CONF_LEVEL_1,	DBPROP_IRowsetScroll,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	DBPROP_ISupportErrorInfo,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IGetRow),					FALSE,	CONF_LEVEL_1,	DBPROP_IGetRow,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetIndex),				FALSE,	CONF_LEVEL_1,	DBPROP_IRowsetIndex,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IColumnsRowset),			FALSE,	CONF_LEVEL_1,	DBPROP_IColumnsRowset,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IChapteredRowset),			FALSE,	CONF_LEVEL_1,	DBPROP_IChapteredRowset,	
		ROWSET_INTERFACE,				&VALUE_WCHAR(IID_IRowsetView),				FALSE,	CONF_LEVEL_1,	DBPROP_IRowsetView,	
	};
	static const INTERFACEMAP rgViewMap[] = 
	{
		VIEW_INTERFACE,					&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		VIEW_INTERFACE,					&VALUE_WCHAR(IID_IViewFilter),				FALSE,	CONF_LEVEL_1,	0,	
		VIEW_INTERFACE,					&VALUE_WCHAR(IID_IViewSort),				FALSE,	CONF_LEVEL_1,	0,	
		VIEW_INTERFACE,					&VALUE_WCHAR(IID_IColumnsInfo),				FALSE,	CONF_LEVEL_1,	0,	
		VIEW_INTERFACE,					&VALUE_WCHAR(IID_IAccessor),				FALSE,	CONF_LEVEL_1,	0,	
		VIEW_INTERFACE,					&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
		VIEW_INTERFACE,					&VALUE_WCHAR(IID_IViewRowset),				FALSE,	CONF_LEVEL_1,	0,	
		VIEW_INTERFACE,					&VALUE_WCHAR(IID_IViewChapter),				FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgMultipleResultsMap[] = 
	{
		MULTIPLERESULTS_INTERFACE,		&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		MULTIPLERESULTS_INTERFACE,		&VALUE_WCHAR(IID_IMultipleResults),			TRUE,	CONF_LEVEL_1,	0,	
		MULTIPLERESULTS_INTERFACE,		&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgIndexMap[] = 
	{
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IAccessor),				TRUE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IColumnsInfo),				TRUE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IConvertType),				TRUE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowset),					TRUE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetIndex),				TRUE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetInfo),				TRUE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetChange),			FALSE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetFind),				FALSE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetIdentity),			FALSE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetLocate),			FALSE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetRefresh),			FALSE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetResynch),			FALSE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetScroll),			FALSE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetUpdate),			FALSE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_IRowsetView),				FALSE,	CONF_LEVEL_1,	0,	
		INDEX_INTERFACE,				&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgTransactionMap[] = 
	{
		TRANSACTION_INTERFACE,			&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		TRANSACTION_INTERFACE,			&VALUE_WCHAR(IID_ITransaction),				TRUE,	CONF_LEVEL_1,	0,	
		TRANSACTION_INTERFACE,			&VALUE_WCHAR(IID_IConnectionPointContainer),FALSE,	CONF_LEVEL_1,	0,	
		TRANSACTION_INTERFACE,			&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgTransactionOptionsMap[] = 
	{
		TRANSACTIONOPTIONS_INTERFACE,	&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		TRANSACTIONOPTIONS_INTERFACE,	&VALUE_WCHAR(IID_ITransactionOptions),		TRUE,	CONF_LEVEL_1,	0,	
		TRANSACTIONOPTIONS_INTERFACE,	&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgErrorMap[] = 
	{
		ERROR_INTERFACE,				&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		ERROR_INTERFACE,				&VALUE_WCHAR(IID_IErrorInfo),				TRUE,	CONF_LEVEL_1,	0,	
		ERROR_INTERFACE,				&VALUE_WCHAR(IID_IErrorRecords),			TRUE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgCustomErrorMap[] = 
	{
		CUSTOMERROR_INTERFACE,			&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		CUSTOMERROR_INTERFACE,			&VALUE_WCHAR(IID_ISQLErrorInfo),			FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgRowMap[] = 
	{
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IRow),						TRUE,	CONF_LEVEL_1,	DBPROP_IRow,
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IColumnsInfo),				TRUE,	CONF_LEVEL_1,	DBPROP_IColumnsInfo,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IConvertType),				TRUE,	CONF_LEVEL_1,	DBPROP_IConvertType,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IGetSession),				TRUE,	CONF_LEVEL_1,	DBPROP_IGetSession,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IColumnsInfo2),			FALSE,	CONF_LEVEL_1,	DBPROP_IColumnsInfo2,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IConnectionPointContainer),FALSE,	CONF_LEVEL_1,	DBPROP_IConnectionPointContainer,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_ICreateRow),				FALSE,	CONF_LEVEL_1,	DBPROP_ICreateRow,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IDBCreateCommand),			FALSE,	CONF_LEVEL_1,	0,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IDBAsynchStatus),			FALSE,	CONF_LEVEL_1,	DBPROP_IDBAsynchStatus,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IDBInitialize),			FALSE,	CONF_LEVEL_1,	0,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IRowChange),				FALSE,	CONF_LEVEL_1,	DBPROP_IRowChange,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IRowSchemaChange),			FALSE,	CONF_LEVEL_1,	DBPROP_IRowSchemaChange,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_IScopedOperations),		FALSE,	CONF_LEVEL_1,	DBPROP_IScopedOperations,	
		ROW_INTERFACE,					&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	DBPROP_ISupportErrorInfo,	
	};
	static const INTERFACEMAP rgStreamMap[] = 
	{
		STREAM_INTERFACE,				&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		STREAM_INTERFACE,				&VALUE_WCHAR(IID_ISequentialStream),		TRUE,	CONF_LEVEL_1,	0,	
		STREAM_INTERFACE,				&VALUE_WCHAR(IID_IStream),					FALSE,	CONF_LEVEL_1,	0,	
		STREAM_INTERFACE,				&VALUE_WCHAR(IID_IGetSourceRow),			FALSE,	CONF_LEVEL_1,	0,	
		STREAM_INTERFACE,				&VALUE_WCHAR(IID_IConnectionPointContainer),FALSE,	CONF_LEVEL_1,	0,	
		STREAM_INTERFACE,				&VALUE_WCHAR(IID_IDBAsynchStatus),			FALSE,	CONF_LEVEL_1,	0,	
		STREAM_INTERFACE,				&VALUE_WCHAR(IID_IDBInitialize),			FALSE,	CONF_LEVEL_1,	0,	
		STREAM_INTERFACE,				&VALUE_WCHAR(IID_ISupportErrorInfo),		FALSE,	CONF_LEVEL_1,	0,	
	};
	static const INTERFACEMAP rgBinderMap[] = 
	{
		BINDER_INTERFACE,				&VALUE_WCHAR(IID_IUnknown),					TRUE,	CONF_LEVEL_1,	0,	
		BINDER_INTERFACE,				&VALUE_WCHAR(IID_IBindResource),			TRUE,	CONF_LEVEL_1,	0,	
		BINDER_INTERFACE,				&VALUE_WCHAR(IID_ICreateRow),				TRUE,	CONF_LEVEL_1,	0,	
		BINDER_INTERFACE,				&VALUE_WCHAR(IID_IDBProperties),			TRUE,	CONF_LEVEL_1,	0,	
		BINDER_INTERFACE,				&VALUE_WCHAR(IID_IDBBinderProperties),		TRUE,	CONF_LEVEL_1,	0,	
		BINDER_INTERFACE,				&VALUE_WCHAR(IID_IRegisterProvider),		FALSE,	CONF_LEVEL_1,	0,	
	};

	
	static const struct
	{
		ULONG cInterfaces;
		const INTERFACEMAP* rgInterfaceMap;
	} rgAllInterfaceMaps[] = 
	{
		NUMELEM(rgUnknownMap),				rgUnknownMap,			//UNKNOWN_INTERFACE
		NUMELEM(rgEnumeratorMap),			rgEnumeratorMap,		//ENUMERATOR_INTERFACE
		NUMELEM(rgDataSourceMap),			rgDataSourceMap,		//DATASOURCE_INTERFACE
		NUMELEM(rgSessionMap),				rgSessionMap,			//SESSION_INTERFACE
		NUMELEM(rgCommandMap),				rgCommandMap,			//COMMAND_INTERFACE
		NUMELEM(rgRowsetMap),				rgRowsetMap,			//ROWSET_INTERFACE
		NUMELEM(rgViewMap),					rgViewMap,				//VIEW_INTERFACE
		NUMELEM(rgMultipleResultsMap),		rgMultipleResultsMap,	//MULTIPLERESULTS_INTERFACE
		NUMELEM(rgIndexMap),				rgIndexMap,				//INDEX_INTERFACE
		NUMELEM(rgTransactionMap),			rgTransactionMap,		//TRANSACTION_INTERFACE
		NUMELEM(rgTransactionOptionsMap),	rgTransactionOptionsMap,//TRANSACTIONOPTIONS_INTERFACE
		NUMELEM(rgErrorMap),				rgErrorMap,				//ERROR_INTERFACE
		NUMELEM(rgCustomErrorMap),			rgCustomErrorMap,		//CUSTOMERROR_INTERFACE
		NUMELEM(rgRowMap),					rgRowMap,				//ROW_INTERFACE
		NUMELEM(rgStreamMap),				rgStreamMap,			//STREAM_INTERFACE
		NUMELEM(rgBinderMap),				rgBinderMap,			//BINDER_INTERFACE
	};

	
	//NOTE:
	//The above looks a little large, but is extremly effiecent.
	//Basically we need to have a quick lookup of associated interfaces and info for each object.
	
	//So we need to lookup table for this static info.  The problem is that we have different
	//objects and each objects has an array of variable length of interfaces.  So a 2D table
	//does not work since you would have to search linearly for the right object, then count
	//the number of interfaces.  A 3D table almost works and would allow us to directly index into
	//the correct object based upon the enum, except for the fact the compiler needs to know the inner 
	//array size at compile time.  We could choose something larger than the max interfaces for an
	//object, but then trying to return the number of interfaces we are back to counting!

	//So with all that said, we have a seperate static array for each object.  We then have a second
	//"outer" array of all the variable arrays, which is in order of the enum EINTERFACE.
	//So at run time all we need to do is index into the correct subarray, which has the count
	//based upon the size of the smaller static array.  This is a direct lookup with an effienctcy of
	//of O(1) compared to O(n)!  So it can be a little larger for the improved speed, since this is
	//called from every VerifyInterface!!!

	ASSERT(eInterface >= 0 && eInterface < NUMELEM(rgAllInterfaceMaps));
	*pcInterfaces = rgAllInterfaceMaps[eInterface].cInterfaces;	
	*prgInterfaces = (INTERFACEMAP*)rgAllInterfaceMaps[eInterface].rgInterfaceMap;	
	
	//Just double check to make sure the Arrays are in the same order as the Enum!
	ASSERT((*prgInterfaces)[0].eInterface == eInterface);
	return TRUE;
}

BOOL IsIIDThisType(REFIID riid, EINTERFACE eInterface)
{
	ULONG cInterfaces = 0;
	INTERFACEMAP* rgInterfaces = NULL;

	//Obtain the interface array for this object..
	GetInterfaceArray(eInterface, &cInterfaces, &rgInterfaces);

	//Loop through the interfaces in this array
	for(ULONG iInterface=0; iInterface<cInterfaces; iInterface++)
	{
		if(riid == *rgInterfaces[iInterface].pIID)
			return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc DBGUIDtoEINTERFACE
//
// Converts known DBGUIDs to corresponding EINTERFACE.
//
// @rdesc Success or Failure
// 	
//--------------------------------------------------------------------
EINTERFACE DBGUIDtoEINTERFACE( REFGUID rguid )
{
	if (rguid == DBGUID_DSO)
		return DATASOURCE_INTERFACE;
	else if (rguid == DBGUID_SESSION)
		return SESSION_INTERFACE;
	else if (rguid == DBGUID_ROW)
		return ROW_INTERFACE;
	else if (rguid == DBGUID_ROWSET)
		return ROWSET_INTERFACE;
	else if (rguid == DBGUID_STREAM)
		return STREAM_INTERFACE;
	else if (rguid == DBGUID_COMMAND)
		return COMMAND_INTERFACE;

	return INVALID_INTERFACE;
}


//--------------------------------------------------------------------
// @mfunc IsConfLevel
//
// Checks if the Conformance Level of the provider is >= to the "required"
// conformance level 
//
// @rdesc Success or Failure
// 	@flag  TRUE  | Required
//	@flag  FALSE | Not Required
//--------------------------------------------------------------------
BOOL IsConfLevel(DWORD dwProvLevel, DWORD dwReqLevel)
{
	//Since the Conformance Level DWORD contains both the "LEVEL" and the "EXTRA"
	//functionality required, this method will determine if the passed in ProvLevel
	//meets the passed in ReqLevel.

	//Example:  IRowsetChange is a required interface for CONF_LEVEL_0 only if the
	//provider supports UPDATABILITY.  So to see if the Provider is at least
	//CONF_LEVEL_0 and supports UPDATABILITY call this function with
	// IsConfLevel(dwProvLevel, CONF_LEVEL_0 | CONF_UPDATEABLE)

	//This is more than just a (dwProvLevel && (CONF_LEVEL_0 | CONF_UPDATEABLE))
	//Since the provider maybe a higher level of conformance (LEVEL_1) or 
	//the requirements may contain more than 1 bonus functionality.

	if(CONF_LEVEL(dwProvLevel) >= CONF_LEVEL(dwReqLevel))
	{
		//If there are no extra requirements then this is required
		if(!CONF_REQ(dwReqLevel))
			return TRUE;
		
		//CONF_UPDATEABLE		
		if(CONF_REQ_UPDATEABLE(dwReqLevel) && CONF_REQ_UPDATEABLE(dwProvLevel))
			return TRUE;

		//CONF_TRANSACTIONS   
		if(CONF_REQ_TRANSACTIONS(dwReqLevel) && CONF_REQ_TRANSACTIONS(dwProvLevel))
			return TRUE;

		//CONF_COMMANDS		
		if(CONF_REQ_COMMANDS(dwReqLevel) && CONF_REQ_COMMANDS(dwProvLevel))
			return TRUE;

		//CONF_FILTERS		
		if(CONF_REQ_FILTERS(dwReqLevel) && CONF_REQ_FILTERS(dwProvLevel))
			return TRUE;

		//CONF_INDEXES		
		if(CONF_REQ_INDEXES(dwReqLevel) && CONF_REQ_INDEXES(dwProvLevel))
			return TRUE;
	}
	return FALSE;
}
				
//--------------------------------------------------------------------
// @mfunc IsReqProperty
//
// Checks if the Property is required for the 
// conformance level of the provider. 
//
// @rdesc Success or Failure
// 	@flag  TRUE  | Required
//	@flag  FALSE | Not required
//--------------------------------------------------------------------
BOOL IsReqProperty(DBPROPID dwPropertyID, GUID guidPropertySet)
{
	typedef struct _PROPERTYMAP
	{
		const GUID*		pguidPropertySet;
		DBPROPID		dwPropertyID;
		DWORD			dwLevel;
	} PROPERTYMAP;

	//This is a "table" of Properties their "level" of conformance.
	//All Properties that are required for level 0, should have 0 in the level column,
	//If the Property is required for the level of the provider it will return TRUE,
	//otherwise FALSE.  If the Property is not found in the table this method
	//assumes its an Property interface and also returns FALSE.
	PROPERTYMAP rgPropertyMap[] = 
	{
		//DBPROPSET						//DBPROPID							//Level
		&DBPROPSET_DBINIT,				DBPROP_INIT_PROMPT,					CONF_LEVEL_0,
	
		&DBPROPSET_DATASOURCEINFO,		DBPROP_MAXTABLESINSELECT,			CONF_LEVEL_0 | CONF_COMMANDS,

		&DBPROPSET_ROWSET,				DBPROP_CANHOLDROWS,					CONF_LEVEL_0,
		&DBPROPSET_ROWSET,				DBPROP_CANFETCHBACKWARDS,			CONF_LEVEL_0,
		&DBPROPSET_ROWSET,				DBPROP_COMMITPRESERVE,				CONF_LEVEL_0 | CONF_TRANSACTIONS,
		&DBPROPSET_ROWSET,				DBPROP_OWNUPDATEDELETE,				CONF_LEVEL_0 | CONF_UPDATEABLE,
		&DBPROPSET_ROWSET,				DBPROP_OWNINSERT,					CONF_LEVEL_0 | CONF_UPDATEABLE,
		&DBPROPSET_ROWSET,				DBPROP_REMOVEDELETED,				CONF_LEVEL_0 | CONF_UPDATEABLE,

		&DBPROPSET_ROWSET,				DBPROP_CANSCROLLBACKWARDS,			CONF_LEVEL_1,
	};
	ULONG cPropertyMaps = NUMELEM(rgPropertyMap);
	DWORD dwProviderLevel = GetModInfo()->GetProviderLevel();
		
	//Now loop through the table and determine if this Property is required 
	//for this object, based upon the level of the provider
	for(ULONG i=0; i<cPropertyMaps; i++)
	{
		if(dwPropertyID == rgPropertyMap[i].dwPropertyID &&
			guidPropertySet == *(rgPropertyMap[i].pguidPropertySet))
		{
			//See if the ProviderLevel is greater then the level of the interface
			if(IsConfLevel(dwProviderLevel, rgPropertyMap[i].dwLevel))
				return TRUE;
		}
	}


	//Otherwise the interface is not required
	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc IsReqInterface
//
// Checks if the interface is required on the given object for the 
// conformance level of the provider. 
//
// @rdesc Success or Failure
// 	@flag  TRUE  | Required
//	@flag  FALSE | Not required
//--------------------------------------------------------------------
BOOL IsReqInterface(EINTERFACE eInterface, REFIID riid)
{
	ULONG cInterfaces = 0;
	INTERFACEMAP* rgInterfaces = NULL;

	//Obtain the array of interfaces for this object...
	if(!GetInterfaceArray(eInterface, &cInterfaces, &rgInterfaces))
		return FALSE;
	
	DWORD dwProviderLevel = GetModInfo()->GetProviderLevel();
	
	//Now loop through the table and determine if this interface is required 
	//for this object, based upon the level of the provider
	for(ULONG i=0; i<cInterfaces; i++)
	{
		if(eInterface == rgInterfaces[i].eInterface &&
			riid == *(rgInterfaces[i].pIID))
		{
			//See if the ProviderLevel is greater then the level of the interface
			if(IsConfLevel(dwProviderLevel, rgInterfaces[i].dwConfLevel))
				return TRUE;
			break;
		}
	}


	//Otherwise the interface is not required
	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc IsValidInterface
//
// Checks if the interface is valid on the given object. 
//
// @rdesc Success or Failure
// 	@flag  TRUE  | Required
//	@flag  FALSE | Not required
//--------------------------------------------------------------------
BOOL IsValidInterface(EINTERFACE eInterface, REFIID riid)
{
	ULONG cInterfaces = 0;
	INTERFACEMAP* rgInterfaces = NULL;

	//Obtain the array of interfaces for this object...
	if(!GetInterfaceArray(eInterface, &cInterfaces, &rgInterfaces))
		return FALSE;
	
	//Now loop through the table and determine if this interface is valid 
	//for this object
	for(ULONG i=0; i<cInterfaces; i++)
	{
		if(eInterface == rgInterfaces[i].eInterface &&
			riid == *(rgInterfaces[i].pIID))
			return TRUE;
	}

	//Otherwise the interface is not valid
	return FALSE;
}


//Determines whether a property is usable in the Tests.
//The property is required (for the level) or were not using Strict
BOOL IsUsableProperty(DBPROPID dwPropertyID, GUID guidPropertySet)
{
	return IsReqProperty(dwPropertyID, guidPropertySet) || !GetModInfo()->IsStrictLeveling();
}

//Determines wiether an Interface property is usable in the Tests.
//The Interface is required (for the level) or were not using Strict
BOOL IsUsableInterface(EINTERFACE eInterface, REFIID riid)
{
	return IsReqInterface(eInterface, riid) || !GetModInfo()->IsStrictLeveling();
}


//--------------------------------------------------------------------
// @mfunc VerifyInterface
//
// Checks if the interface is supported on the given object. If
// it is not, a message is logged and FALSE is returned.  If it is 
// supported, the interface is stored in *ppRequestedIUnknown.  
//
//--------------------------------------------------------------------
BOOL VerifyInterface
(
	IUnknown*		pIUnkIn,		// @parm [IN]	Existing interface to QI on
	REFIID			riid,			// @parm [IN]   Interface ID to be returned
	EINTERFACE		eInterface,		// @parm [IN]	EINTERFACE of pIUnkIn
	IUnknown**		ppIUnkOut		// @parm [OUT]  Interface returned
)	
{			
	HRESULT hr = S_OK;
	IUnknown* pIUnknown = NULL;

	//Null output params
	if(ppIUnkOut)
		*ppIUnkOut = NULL;
	
	//Validate input params
	if(pIUnkIn==NULL)
		return FALSE;

	//Check to see if the requested interface is in the correct level
	if(!IsUsableInterface(eInterface, riid))
	{
		//Indicate requested Interface not in Conformance Level
//		odtLog << "Requested Interface not supported in this Level." << ENDL;
		return FALSE;
	}

	//If Query Interface fails, this interface is not supported
	//Make sure its a valid return code...
	hr = pIUnkIn->QueryInterface(riid, (void**)&pIUnknown);
	TEST3C_(hr, S_OK, E_NOINTERFACE, E_UNEXPECTED);

	//Check return results
	if(SUCCEEDED(hr) && !pIUnknown)
	{
		//Indicate Error
		GCHECK(hr = E_FAIL, S_OK);
	}

	//If this interface is a required interface the for level of conformance then 
	//it is an error to fail QI, otherwise its just an optional interface
	if(FAILED(hr) && IsReqInterface(eInterface, riid))
	{
		//Indicate Conformance Level Error
		odtLog << "Required Interface for this level is not supported! " << GetInterfaceName(riid);
		CHECK(hr, S_OK);
	}

CLEANUP:
	if(ppIUnkOut)
		*ppIUnkOut = pIUnknown;
	else
		SAFE_RELEASE(pIUnknown);
	return SUCCEEDED(hr);
}

	
////////////////////////////////////////////////////////////////////////////
//  VerifyEqualInterface
//
////////////////////////////////////////////////////////////////////////////
BOOL VerifyEqualInterface(IUnknown* pInterface1, IUnknown* pInterface2) 
{             
	TBEGIN
    IUnknown* pIUnknown1 = NULL;
    IUnknown* pIUnknown2 = NULL;

    //Invalidarg check
	if(pInterface1 == NULL || pInterface2 == NULL)
		return pInterface1 == pInterface2;
    
    //The only way to determine if the interfaces are equal is to obtain
	//the IUnknown pointer and do direct pointer comparison
    TESTC_(pInterface1->QueryInterface(IID_IUnknown, (void**)&pIUnknown1),S_OK);
    TESTC_(pInterface2->QueryInterface(IID_IUnknown, (void**)&pIUnknown2),S_OK);

	//See if there equal...
	QCOMPARE(pIUnknown1, pIUnknown2);

CLEANUP:
	SAFE_RELEASE(pIUnknown1);
	SAFE_RELEASE(pIUnknown2);
    TRETURN
}


//--------------------------------------------------------------------
// Expected Format of Init string:
//
// @func This function parses the wszInitString (obtained from the LTM) to find
// the necessary init options, and builds the correct arrays needed
// to pass to IDBInitialize::Initialize().  User must call IMalloc->Free 
// on *prgOptionIDs and prgOptionVals, as well as ClearVariant
// on all members of the *prgOptionVals array. 
//
// <opt>=<value>; 
//
// Where:
// opt is the string following the last underscore of a valid IDBIntialize option.
// value is the unquoted string representation of the value for that option.
// For the PROMPT option, use the string following the last underscore of the
// valid constant value.
//
// Example:
// DATASOURCE=MyDSN; USERID=joeuser; PASSWORD=abcdefg; PROMPT=COMPLETE;
//
// Only the options specified in the string passed from the LTM for this provider 
// will be used to create the arrays for initialization.
//
// @rdesc:
// 	@flag  TRUE  | Successful Termination
//	@flag  FALSE | Termination did not complete successfully
//		
//-------------------------------------------------------------------------------------
BOOL GetInitProps(
	ULONG*				pcPropSets,			//@parm [in/out]:	Pointer to memory to hold count of DBPROPSET structs.
	DBPROPSET**			prgPropSets			//@parm [in/out]:	Pointer to memory to hold an array of DBPROPSET
											//					structures containing properties and values to be set.
)
{
	return GetModInfo()->GetInitProps(pcPropSets, prgPropSets);
}	

//-------------------------------------------------------------------------------------
//	This routine checks whether the provider is a read only provider.
//
//	@func: The function checks the property DBPROP_DATASOURCEREADONLY in the Data Source 
//  Information Properties group. 
//
// @rdesc:
// 	@flag  TRUE  | If the boolean value is true  
//	@flag  FALSE | If the boolean value is false
//		
//-------------------------------------------------------------------------------------
BOOL IsProviderReadOnly
(
 IUnknown * pSessionIUnknown	//@parm  [IN] Object pointer
)
{
	//Since its required for all ReadOnly providers to Support DBPROP_DATASOURCEREADONLY
	//We will assume that if its not supported then its NOT READONLY
	//So if the QI, GetDataSource, or GetProperty fail it will return FALSE...

	VARIANT_BOOL    bValue = VARIANT_FALSE;
	IGetDataSource*	pIGetDataSource = NULL;
	IDBProperties*	pIDBProperties = NULL;

	//Get the DataSource
	if(!VerifyInterface(pSessionIUnknown, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDataSource))
		goto CLEANUP;

	if(pIGetDataSource->GetDataSource(IID_IDBProperties, (IUnknown**)&pIDBProperties) != S_OK)
		goto CLEANUP;

	// Call the GetProperty
	if(!GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, pIDBProperties, &bValue))
		goto CLEANUP;

CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIGetDataSource);
	return bValue == VARIANT_TRUE ? TRUE : FALSE;
}

//---------------------------------------------------------------------------
// @func  BOOL|
//        MiscFunc GetProviderName|
//        Return the Providers string Name.
//
//---------------------------------------------------------------------------
WCHAR * GetProviderName(
 IUnknown * pSessionIUnknown	//@parm [IN] Pointer to object
)
{
	WCHAR*	pwszProviderName  = NULL;
	IGetDataSource* pIGetDataSource = NULL;
	IDBProperties* pIDBProperties = NULL;

	ASSERT(pSessionIUnknown);
	if(pSessionIUnknown == NULL)
		goto CLEANUP;
	
	//Get the DataSource
	if(!VerifyInterface(pSessionIUnknown, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDataSource))
		goto CLEANUP;

	if(pIGetDataSource->GetDataSource(IID_IDBProperties, (IUnknown**)&pIDBProperties) != S_OK)
		goto CLEANUP;

	// Call the GetProperty
	if(!GetProperty(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO, pIDBProperties, &pwszProviderName))
		goto CLEANUP;

CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIGetDataSource);
	return pwszProviderName;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToMBCS
//		Previously allocated memory
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToMBCS(LPCWSTR pwsz, CHAR* psz, int cStrLen, UINT CodePage)
{
	ASSERT(pwsz && psz);

	//Convert the string to MBCS
	INT iResult = WideCharToMultiByte(CodePage, 0, pwsz, -1, psz, cStrLen, NULL, NULL);
	return iResult ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToMBCS
//		Dynamically allocated memory
/////////////////////////////////////////////////////////////////////////////
CHAR* ConvertToMBCS(LPCWSTR pwsz, UINT CodePage)
{
	//no-op case
	if(!pwsz)
		return NULL;
	
	// At the worst case each wide char will be a double-byte MBCS char.
	size_t cLen	= wcslen(pwsz) * sizeof(WCHAR);

	//Allocate space for the string
	CHAR* pszBuffer = (CHAR*)PROVIDER_ALLOC((cLen+1)*sizeof(CHAR));
	if(pszBuffer==NULL)
		goto CLEANUP;

	//Now convert the string
	WideCharToMultiByte(CodePage, 0, pwsz, -1, pszBuffer, (int)(cLen+1), NULL, NULL);

CLEANUP:
	return pszBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToWCHAR
//		Previously allocated memory
/////////////////////////////////////////////////////////////////////////////
HRESULT ConvertToWCHAR(LPCSTR psz, WCHAR* pwsz, int cStrLen)
{
	ASSERT(psz && pwsz);

	//Convert the string to MBCS
	INT iResult = MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, cStrLen);
	return iResult ? S_OK : E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT ConvertToWCHAR
//		Dynamically allocated memory
/////////////////////////////////////////////////////////////////////////////
WCHAR* ConvertToWCHAR(LPCSTR psz)
{
	//no-op case
	if(!psz)
		return NULL;
	
	size_t cLen	= strlen(psz);

	//Allocate space for the string
	WCHAR* pwszBuffer = (WCHAR*)PROVIDER_ALLOC((cLen+1)*sizeof(WCHAR));
	if(pwszBuffer==NULL)
		goto CLEANUP;

	//Now convert the string
	MultiByteToWideChar(CP_ACP, 0, psz, -1, pwszBuffer, (int)(cLen+1));

CLEANUP:
	return pwszBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// WCHAR* wcsDuplicate
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* wcsDuplicate(LPCWSTR pwsz)
{
	//no-op case
	if(!pwsz)
		return NULL;
	
	size_t cLen	= wcslen(pwsz);

	//Allocate space for the string
	WCHAR* pwszBuffer = (WCHAR*)PROVIDER_ALLOC((cLen+1)*sizeof(WCHAR));
	if(pwszBuffer==NULL)
		goto CLEANUP;

	//Now copy the string
	wcscpy(pwszBuffer, pwsz);

CLEANUP:
	return pwszBuffer;
}



//-------------------------------------------------------------------------------------
//	This routine only checks data retrieved by DBBINDIO_READWRITE and DBBINDIO_READCOLUMNSBYREF
//	accessors. DBBINDIO_READBYREF accessor is not supported by Kagera and we are testing it.
//
//	@func: The function compares the data in the user's buffer with the data at the backend 
//	table. The data is retrieved from the rowset by an accessor. The status, length and value 
//	bindings for each column are checked as specified in the accessor.  The data is binded
//  to its default data type.
//
// @rdesc:
// 	@flag  TRUE  | Successful Termination
//	@flag  FALSE | Termination did not complete successfully
//		
//-------------------------------------------------------------------------------------
BOOL CompareData(
	DBORDINAL	cColumns,		//@parm [in]:	the count of rgColumnsOrd
	DB_LORDINAL	*rgColumnsOrd,	//@parm [in]:	the array of column ordinals in the backend table.
								//				The column ordinals in the backend table is 
								//				not the same as ordinals in the rowset.  
	DBCOUNTITEM	cRow,			//@parm[in]:	the row number of the data at the backend table
	void		*pData,			//@parm[in]:	the pointer to the buffer which contains the data
								//				to be compared with
	DBCOUNTITEM	cBindings,		//@parm[in]:	the count of the rgBindings
	DBBINDING*	rgBindings,		//@parm[in]:	the binding information of the accessor which 
								//				retrieved the data
	CSchema		*pSchema,		//@parm[in]:	The pointer to CTable object from which the 
								//				the rowset was created.
	IMalloc		*pIMalloc,		//@parm[in]:	the IMalloc pointer used to free memory.
								//				can not be NULL.
	EVALUE		eValue,			//@parm[in]:	whether use PRIMARY or SECONDARY to make a data
	ECOMPARE_FREE eCompareFree,	//@parm[in]:	COMPARE_FREE  compare data and free the memory refereced by the consumer's buffer
								//				COMPARE_ONLY  compare data only.  Do not attempt to free memory
								//				FREE_ONLY	   free memory only.  Do not attemp to compare data
	ECOMPARE_LEVEL eCompareLevel,//@param [in]	whether to stop and return when an error is encountered
								//or to keep comparing all columns even after a failure.
	BOOL		bCompareValue	// @paramopt [in] flag to tell whether or not to compare Value when
								// status is null.  Default is TRUE (i.e. Compare Value when Status is
								// null.  when bCompareValue == FALSE then value field is not compared.
)
{
	HRESULT	hr;
	DBCOUNTITEM	cCount;
	BOOL	fErrorOccured		= FALSE;
	BOOL	fCheckForTruncation = FALSE;
	BOOL	fCheckForNULL		= FALSE;
	BOOL	fCheckForLength		= FALSE;
	BOOL	fColumnError		= FALSE;
	void	*pBackEndData		= NULL;
	void    *pConsumerData		= NULL;
	WCHAR	*wszData			= NULL;
	USHORT	uswSize = 0;
	DBTYPE	wType;
	DBTYPE	wVariantType = DBTYPE_EMPTY;
	LONG	lDBTypeSize;
	DWORD_PTR	dwAddrData;
	DBTYPE	wDBType;
	CCol	ColInfo;
	DBLENGTH	cbConsumerSize = 0;
	DBTYPE	wBackEndType;
	DB_LORDINAL   iOrdinal;
	
	// Input validation
	ASSERT(pSchema);

	//Verify Bindings and pData args
	if(cBindings && (pData==NULL || rgBindings==NULL))
	{
		ASSERT(pData);
		ASSERT(rgBindings);
		fErrorOccured = TRUE;
		goto END;
	}

	if (eCompareFree != FREE_ONLY)
	{
		wszData=(WCHAR *)PROVIDER_ALLOC(sizeof(WCHAR) * DATA_SIZE);
		
		if(!wszData)
		{
			fErrorOccured = TRUE;
			goto END;
		}
	}
	
	// Get the address of the starting point of consumer's buffer
	dwAddrData=(DWORD_PTR)pData;

	// Compare the data column by column
	// Status, Length, and Valud binding are checked
	for(cCount=0; cCount<cBindings; cCount++)
	{
		ASSERT(pData);
		DBBINDING* pBinding = &rgBindings[cCount];

		//We can't compare the bookmark column, since we don't store the bookmark
		//column in the cColList...
		if(pBinding->iOrdinal == 0)
			goto CLEANUP;

		// If you hit this assert then you probably passed in the wrong rgColumnsOrd
		ASSERT(pBinding->iOrdinal <= cColumns || cColumns==0);
		iOrdinal = rgColumnsOrd ? rgColumnsOrd[pBinding->iOrdinal-1] : pBinding->iOrdinal;

		// Get the column information for this column
		if(SUCCEEDED(pSchema->GetColInfo(iOrdinal, ColInfo)))
			wBackEndType = ColInfo.GetProviderType();
		else
			wBackEndType = pBinding->wType;

		// If previous bindings have failed and user has specified
		// to stop on error, exit now
		if (fErrorOccured && eCompareLevel == COMPARE_UNTIL_ERROR)
			goto END;

		// Return FALSE if the DBTYPE is ORed with DBTYPE_RESERVED
		if (pBinding->wType & DBTYPE_RESERVED)
		{
			PRVTRACE(wszReservedUsed);
			PRVTRACE(L"%u", iOrdinal);
			PRVTRACE(wszAtBinding);
			PRVTRACE(L"%u", cCount);
			PRVTRACE(ENDL);

			fErrorOccured=TRUE;
			continue;			
		}

		// Init
		fCheckForTruncation	= FALSE;
		fCheckForNULL		= FALSE;
		fCheckForLength		= FALSE;
		fColumnError		= FALSE;

		// Free memory directly if not need to compareData
		if (eCompareFree==FREE_ONLY)
			goto CLEANUP;

		// Check the status binding   
		if (STATUS_IS_BOUND(*pBinding))
		{
			switch(STATUS_BINDING(*pBinding, dwAddrData))
			{
				// If the data is truncated, we need to check the length binding if appropriate
				case DBSTATUS_S_TRUNCATED:
					fCheckForTruncation=TRUE;
					break;

				// If the data is NULL, we need to check the length binding if appropriate
				case DBSTATUS_S_ISNULL:
					fCheckForNULL=TRUE;
					break;

				case DBSTATUS_S_OK:
					break;

				default:
				{
					//NOTE: DBSTATUS_S_IGNORE and all other status are not valid, since this function
					//should only be called from GetData, GetColumns, etc
					PRVTRACE(wszInvalidStatus);
					PRVTRACE(L"%u", STATUS_BINDING(*pBinding, dwAddrData));
					PRVTRACE(wszForColumn);
					PRVTRACE(L"%u", iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					fErrorOccured=TRUE;
				
					//Display the Bad Status (E_FAIL is basically DisplayAlways - even success)
					VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
					continue;
				}
			}
		}
		else
		{
			// Make sure the data is not NULL before dereferencing to get length or value.  
			// For variants wDBType will be wrong, but we're only interested in whether it's
			// NULL or not so we'll just use DBTYPE_WSTR.
			hr = pSchema->MakeData(wszData, cRow, iOrdinal, eValue, DBTYPE_WSTR); 

			// If MakeData returns S_FALSE then the data is NULL, so both length and value are undefined.
			// No further testing is possible
			if (S_FALSE == hr)
				continue;
		}

		// Check for the length binding
		if(LENGTH_IS_BOUND(*pBinding))
		{
			cbConsumerSize = LENGTH_BINDING(*pBinding, dwAddrData);

			// If cbMaxLen > length, no truncation should occure
			if (fCheckForTruncation)
			{
				if(cbConsumerSize < pBinding->cbMaxLen)
				{
					PRVTRACE(wszMaxLengthGreaterThanLength);
					PRVTRACE(L"%u", iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					fErrorOccured=TRUE;

					//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
					VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
					continue;
				}
				
				// Goto next binding structure
				continue;			
			}

			// If status is NULL, skip length checks
			// Make sure the data at the back end is NULL
			if (fCheckForNULL)
				goto VALUE;

			// Length should be the same as the sizeof(DBTYPE) for fixed length data type
			// Length should be number of bytes of the data for variable length data type,
			// that is, DBTYPE_STR, DBTYPE_BYTES,and DBTYPE_WSTR.
			// Length should not be returned by reference for data ORed with _BYREF
			switch(pBinding->wType)
			{
				// No way to check variable lendth data type without a value binding
				// Remember to check for the length binding if there is a value binding
				case DBTYPE_STR:
				case DBTYPE_STR | DBTYPE_BYREF:
				case DBTYPE_BYTES:
				case DBTYPE_BYTES | DBTYPE_BYREF:
				case DBTYPE_WSTR:
				case DBTYPE_WSTR | DBTYPE_BYREF:
				case DBTYPE_VARNUMERIC:
				case DBTYPE_VARNUMERIC | DBTYPE_BYREF:
					fCheckForLength=TRUE;
					break;

				// For fixed length data type, make sure the length value is 
				// the same as the size of the data type
				default:
					// Get the size of the dta type
					lDBTypeSize=GetDBTypeSize(pBinding->wType);
					
					if (lDBTypeSize==0 || lDBTypeSize==INVALID_DBTYPE_SIZE)
					{
						PRVTRACE(wszInvalidDBTYPE);
						PRVTRACE(L"%u", iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fColumnError=TRUE;

						//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
						VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
						break;
					}

					// Compare the data length with length binding
					// IUNKNOWN columns can return 4 bytes (Sizeof(IUnknown*)) or
					// now are allowed to actually return the stream size if known.
					if (LENGTH_BINDING(*pBinding, dwAddrData) != (DBLENGTH)lDBTypeSize && 
						(pBinding->wType != DBTYPE_IUNKNOWN))
					{
						PRVTRACE(wszLengthInconsist);											
						PRVTRACE(L"%u", iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fColumnError=TRUE;

						//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
						VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
						break;
					}
					break;
			}

			// Free the memory allocated by the provider, goto next binding structure
			if (fColumnError)
			{
				fErrorOccured=TRUE;
				goto CLEANUP;
			}
		}

	
VALUE:
		// Check for value binding
		if (VALUE_IS_BOUND(*pBinding))
		{	
			// Skip checking the value binding for BOOKMARKS
			if (iOrdinal==0)
				goto CLEANUP;

			// Skip checking the value binding if the column does not exist at the backend
			if (iOrdinal == -1)
				goto CLEANUP;

			// Get the data in the consumer's buffer
			pConsumerData= &VALUE_BINDING(*pBinding, dwAddrData);

			// Get the initial data type
			wDBType = pBinding->wType;

			
			//If the type is VARIANT, we need to check the type underneath
			if(wDBType==DBTYPE_VARIANT)
			{
				wDBType = ((VARIANT *)pConsumerData)->vt;

				// VT_NULL variants are allowed to return S_OK or DBSTATUS_S_ISNULL
				if (STATUS_IS_BOUND(*pBinding) &&
					STATUS_BINDING(*pBinding, dwAddrData) == DBSTATUS_S_OK &&
					wDBType == DBTYPE_NULL)
					fCheckForNULL = TRUE;
			}

			hr = pSchema->MakeData(wszData, cRow, iOrdinal, eValue, wDBType, FALSE, &wVariantType); 

			if (hr==DB_E_BADTYPE)
			{
				PRVTRACE(wszColumnNotUpdatable);
				PRVTRACE(L"%u", iOrdinal);
				PRVTRACE(wszAtBinding);
				PRVTRACE(L"%u", cCount);
				PRVTRACE(ENDL);
				goto CLEANUP;
			}

			if (FAILED(hr))
			{
				odtLog<<wszCanNotMakeData<<(ULONG)iOrdinal<<wszAtBinding<<cCount<<ENDL;
				fErrorOccured=TRUE;
				goto END;
			}
			
			// Return an error if the backend table contains NULL value
			// but the status binding did not indicate it
			if ((!fCheckForNULL) && STATUS_IS_BOUND(*pBinding) &&
				(hr==S_FALSE))
			{
				PRVTRACE(wszTableNULL);
				PRVTRACE(L"%u", iOrdinal);
				PRVTRACE(wszAtBinding);
				PRVTRACE(L"%u", cCount);
				PRVTRACE(ENDL);
				fErrorOccured=TRUE;

				//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
				VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
				continue;
			}

			// Check for NULL value
			if (fCheckForNULL)
			{
				if (bCompareValue)
				{
					if(hr!=S_FALSE)
					{	
						PRVTRACE(wszTableNotNULL);
						PRVTRACE(L"%u", iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fErrorOccured=TRUE;
					}

					// If the column is ProviderOwned, the pointer has to be NULL
					if (pBinding->dwMemOwner == DBMEMOWNER_PROVIDEROWNED)
					{
						if ((void *)(*(DWORD_PTR *)pConsumerData)!=NULL)
						{	
							PRVTRACE(wszPointerNotNULL);
							PRVTRACE(L"%u", iOrdinal);
							PRVTRACE(wszAtBinding);
							PRVTRACE(L"%u", cCount);
							PRVTRACE(ENDL);
							fErrorOccured=TRUE;

							//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
							VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
						}
					}
				}

				//Else We don't care about the value if the Status is NULL.
				continue;			
			}

			// Check for non-null values
			// Convert the DB data into DBTYPE data
			// For IUnknown this may be an image(binary) or text(str) column
			
			//IUNKNOWN
			if (pBinding->wType == DBTYPE_IUNKNOWN)
			{
				pBackEndData=WSTR2DBTYPE(wszData, wBackEndType, &uswSize);
			}
			//VARIANT
			else if(pBinding->wType == DBTYPE_VARIANT)
			{
				void* pDBTypeData = WSTR2DBTYPE(wszData, wVariantType, &uswSize);
			
				MapDBTYPE2VARIANT(pDBTypeData, wVariantType, uswSize, (VARIANT **)&pBackEndData);

				PROVIDER_FREE(pDBTypeData);
			}
			//Other
			else
			{
				pBackEndData=WSTR2DBTYPE(wszData, wDBType, &uswSize);
			}

			if (!pBackEndData)
			{					
				odtLog<<wszCanNotConvertData<<iOrdinal<<wszAtBinding<<cCount<<ENDL;
				fErrorOccured=TRUE;
				goto CLEANUP;			
			}

			// If the length was not bound we'll use the length of the backend data.
			// The provider will only be able to use the first null terminator as 
			// the size for string data, so that's what we'll do also
			if(!LENGTH_IS_BOUND(*pBinding))
			{
				if ((wDBType & (~DBTYPE_BYREF)) == DBTYPE_STR)
					cbConsumerSize = strlen((CHAR *)pBackEndData);
				else if ((wDBType & (~DBTYPE_BYREF)) == DBTYPE_WSTR)
					cbConsumerSize = wcslen((WCHAR *)pBackEndData)*sizeof(WCHAR);
				else if ((wDBType & (~DBTYPE_BYREF)) == DBTYPE_BYTES ||
					(wDBType & (~DBTYPE_BYREF)) == DBTYPE_VARNUMERIC)
					cbConsumerSize = uswSize;
				else
					cbConsumerSize=0;			
			}

			// Check data ORed with DBTYPE_ARRAY
			if ((pBinding->wType & DBTYPE_ARRAY))
			{
				wType=pBinding->wType & (~DBTYPE_ARRAY);
				if (!CompareSafeArray(*(SAFEARRAY**)pConsumerData, *(SAFEARRAY**)pBackEndData, wType, NULL, TRUE))
				{
					odtLog<<wszSafeArrayNotEqual;
					fErrorOccured=TRUE;

					PRVTRACE(wszInvalidValueBinding);
					PRVTRACE(L"%u", iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);

					//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
					VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
				}

				goto CLEANUP;			
			}

			// Check data ORed with DBTYPE_VECTOR
			if (pBinding->wType & DBTYPE_VECTOR)
			{	
				// Unset the bit DBTYPE_VECTOR
				wType=pBinding->wType & (~DBTYPE_VECTOR);

				// Compare the data, free memory pointed by pBackEndData
				if (!CompareVector(	(DBVECTOR *)pConsumerData, 
					(DBVECTOR *)pBackEndData, wType, uswSize, pBinding->bPrecision,
												pBinding->bScale, TRUE/*fFreeMemory*/))
					fColumnError=TRUE;

				if (fColumnError)
				{
					odtLog<<wszVectorNotEqual;
					PRVTRACE(wszInvalidValueBinding);
					PRVTRACE(L"%u", iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					fErrorOccured=TRUE;
				}

				goto CLEANUP;			
			}

			// Check data ORed with DBTYPE_BYREF
			if (pBinding->wType & DBTYPE_BYREF)
			{
				// Unset the bit DBTYPE_BYREF
				wType=pBinding->wType & (~DBTYPE_BYREF);

				// Compare the data, free memory pointed by pBackEndData
				if (!CompareDBTypeData( (void *)(*(DWORD_PTR *)pConsumerData), pBackEndData,
					wType, uswSize, pBinding->bPrecision, pBinding->bScale,
					NULL, TRUE, wBackEndType & (~DBTYPE_BYREF), cbConsumerSize, TRUE))
					fColumnError=TRUE;
				
				if (fColumnError)
				{
					PRVTRACE(wszInvalidValueBinding);
					PRVTRACE(L"%u", iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					fColumnError=FALSE;
					fErrorOccured=TRUE;

					//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
					VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
				}
			
				// Make sure for variable length data, the length in *pBinding contains
				// correct information
				if (fCheckForLength)
				{
					switch(wType)
					{
						case DBTYPE_STR:
							if(LENGTH_BINDING(*pBinding, dwAddrData) 
								!= strlen((char *)(*(DWORD_PTR *)pConsumerData)) )
								fColumnError=TRUE;
							break;

						case DBTYPE_WSTR:
							if(LENGTH_BINDING(*pBinding, dwAddrData)
								!= (wcslen((WCHAR *)(*(DWORD_PTR *)pConsumerData))*sizeof(WCHAR)) )
								fColumnError=TRUE;
							break;

						case DBTYPE_BYTES:
							if(LENGTH_BINDING(*pBinding, dwAddrData) != uswSize)
								fColumnError=TRUE;
							break;

						case DBTYPE_VARNUMERIC:
							// can only check that length binding falls within a valid range
							// Max Precision of VARNUMERIC is 255 => 106 bytes of extra storage
							if (cbConsumerSize < sizeof(DB_VARNUMERIC) || 
								cbConsumerSize > 106+sizeof(DB_VARNUMERIC))
								fColumnError=TRUE;
							break;

						default:
							break;
					}

					if (fColumnError)
					{
						fErrorOccured=TRUE;
						PRVTRACE(wszLengthInconsist);											
						PRVTRACE(L"%u", iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);

						//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
						VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
					}
				}

				goto CLEANUP;			
			}

			// Compare the data not ORed with any type modifiers
			// and free memory pointed by pBackEndData
			switch(pBinding->wType)
			{
				case DBTYPE_HCHAPTER:
					//TODO
					TRACE("Privlib - Unable to compare data, unrecognized HCHAPTER column at rgBindings[%d].iOrdinal=%d\n", cCount, iOrdinal);
					break;
				
				case DBTYPE_IUNKNOWN:
				{
					// Create a large enough buffer to handle the entire column
					ULONG cBytes = 0;
					ULONG ulMaxSize = sizeof(WCHAR)*DATA_SIZE;
					void* pBuffer = PROVIDER_ALLOC(ulMaxSize);
					IUnknown* pIUnkObject = *(IUnknown**)pConsumerData;
					DBOBJECT* pObject = pBinding->pObject;
					EINTERFACE eObject = UNKNOWN_INTERFACE;

					//According to the spec if pObject == NULL providers can assume the 
					//iid == IID_IUnknown, Also this is for the case where were are using 
					//a row object are there is no pObject.

					//Since this is an IUnknown column and is not a known object type (Stream, etc),
					//there is very little comparision we can do, we really can't even compare the
					//pointers since this could be a rowset with object pointers, which the object
					//instances would change from the last time persisted, (INI file).
					if(VerifyInterface(pIUnkObject, IID_ISequentialStream) || VerifyInterface(pIUnkObject, IID_IStream))
					{
						DefaultObjectTesting(pIUnkObject, eObject = STREAM_INTERFACE);
					}
					else if(VerifyInterface(pIUnkObject, IID_IRowset))
					{
						DefaultObjectTesting(pIUnkObject, eObject = ROWSET_INTERFACE);
					}
					else if(VerifyInterface(pIUnkObject, IID_IRow))
					{
						DefaultObjectTesting(pIUnkObject, eObject = ROW_INTERFACE);
					}
					else
					{
						DefaultObjectTesting(pIUnkObject, eObject = UNKNOWN_INTERFACE);
						TRACE("Privlib - Unable to compare data, unrecognized IUnknown column at rgBindings[%d].iOrdinal=%d\n", cCount, iOrdinal);
					}

					//If this is a stream object or exposes a stream interface, then read the stream and compare data...
					if((pObject && (pObject->iid==IID_ISequentialStream || pObject->iid==IID_IStream || pObject->iid==IID_ILockBytes)) ||
						(eObject == STREAM_INTERFACE))
					{	
						// If we bind data asking for IID_IUnknown, we try to verify data
						// in case that object supports a stream interface.
						//Need to read all the data from the Storage Object
						hr = StorageRead((pObject ? pObject->iid : IID_IUnknown), pIUnkObject, pBuffer, ulMaxSize, &cBytes);
						
						//Read will return S_OK or S_FALSE if less than the number of bytes read...
						if(hr==S_OK || hr==S_FALSE)
						{
							// If the Read bytes is the same size as the buffer, then there
							// was probably more bytes to read
							CHECK(cBytes < ulMaxSize, TRUE);

							// IUNKNOWN data is not NULL terminated (according to the spec)
							// So if using String data, add the NULL terminator so our compare
							// routines function correctly...
							if(wBackEndType == DBTYPE_STR)
							{
								((CHAR*)pBuffer)[cBytes] = '\0';
							}
							if(wBackEndType == DBTYPE_WSTR || wBackEndType == DBTYPE_BSTR)
							{
								((WCHAR*)pBuffer)[cBytes/2] = L'\0';
							}
							
							// Compare
							if (!CompareDBTypeData(pBuffer, pBackEndData, wBackEndType, (USHORT)cBytes,	
									pBinding->bPrecision, pBinding->bScale, NULL, TRUE,
									wBackEndType & (~DBTYPE_BYREF), cBytes, TRUE))
								fColumnError=TRUE;

							//Call Read one more time to make sure were are truely at the end of the stream
							hr = StorageRead((pObject ? pObject->iid : IID_IUnknown), pIUnkObject, pBuffer, ulMaxSize, &cBytes);
							GCOMPARE(hr==S_OK || hr==S_FALSE, TRUE);
							GCOMPARE(cBytes == 0, TRUE);

						}
						else
						{
							CHECK(hr, S_OK);
							fColumnError=TRUE;
						}
					}
					
					// Free the temp buffer
					PROVIDER_FREE(pBuffer);
					break;
				}
			
			
				default:
				{
					DBTYPE wCmpType = pBinding->wType;
					if (!CompareDBTypeData(pConsumerData,
													pBackEndData,
													wCmpType,
													uswSize,
													pBinding->bPrecision,
													pBinding->bScale,
													NULL,
													TRUE,
													wBackEndType & (~DBTYPE_BYREF),
													cbConsumerSize, TRUE))  
						fColumnError=TRUE;
					break;
				}
			};

			if (fColumnError)
			{				
				PRVTRACE(wszInvalidValueBinding);
				PRVTRACE(L"%u", iOrdinal);
				PRVTRACE(wszAtBinding);
				PRVTRACE(L"%u", cCount);
				PRVTRACE(ENDL);
				fColumnError=FALSE;
				fErrorOccured=TRUE;

				//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
				VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
			}

			// Make sure for variable length data, the length in *pBinding contains
			// correct information
			if (fCheckForLength)	
			{	
				switch(pBinding->wType & (~DBTYPE_BYREF))
				{	
					case DBTYPE_STR:	
						if(LENGTH_BINDING(*pBinding, dwAddrData)
						!= strlen((char *)pConsumerData) )	
							fColumnError=TRUE;	
						break;	

					case DBTYPE_WSTR:	
						if(LENGTH_BINDING(*pBinding, dwAddrData)
						!= (wcslen((WCHAR *)pConsumerData)*sizeof(WCHAR)) )	
							fColumnError=TRUE;	
						break;	

					case DBTYPE_BYTES:	
						if(LENGTH_BINDING(*pBinding, dwAddrData)
						!= uswSize)	
							fColumnError=TRUE;	
						break;

					case DBTYPE_VARNUMERIC:
						// can only check that length binding falls within a valid range
						// Max Precision of VARNUMERIC is 255 => 106 bytes of extra storage
						if (cbConsumerSize < sizeof(DB_VARNUMERIC) || 
							cbConsumerSize > 106+sizeof(DB_VARNUMERIC))
							fColumnError=TRUE;
						break;
					
					default:	
						break;	
				}	

				// Output an error message if the length binding fails
				if (fColumnError)	
				{	
					fErrorOccured=TRUE;	
					PRVTRACE(wszLengthInconsist);										
					PRVTRACE(L"%u", iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);

					//Display the Bad Binding (E_FAIL is basically DisplayAlways - even success)
					VerifyBindings(E_FAIL, 1, pBinding, (void*)dwAddrData);
				}	
			}	
		}

CLEANUP:		
		// Go on to the next binding structure if no need to free memory allocated
		// by the provider or the binding is provider owned or
		// there is no value binding for the current binding structure
		if ((eCompareFree==COMPARE_ONLY)|| pBinding->dwMemOwner == DBMEMOWNER_PROVIDEROWNED || 
			!VALUE_IS_BOUND(*pBinding) )
			continue;
						   
		// Make sure the status binding indicate the column is OK
		// if there is no status binding, we assume the buffer is correct
		if (STATUS_IS_BOUND(*pBinding))
		{
			if(STATUS_BINDING(*pBinding, dwAddrData)!=DBSTATUS_S_OK)
				continue;
		}

		// Get the data in the consumer's buffer
		pConsumerData = &VALUE_BINDING(*pBinding, dwAddrData);

		// If the wType of the binding structure is ORed with DBTYPE_BYREF,
		//and it's client owned, free it
		
		if ((pBinding->wType & DBTYPE_BYREF) && 
			pBinding->dwMemOwner == DBMEMOWNER_CLIENTOWNED)
		{		
			void* pByRef = *(void**)pConsumerData;

			// Free any out-of-line data first
			if(pBinding->wType == (DBTYPE_BYREF | DBTYPE_BSTR))
			{
				SysFreeString(*(BSTR*)pByRef);
			}

			// Free any out-of-line data first
			if (pBinding->wType == (DBTYPE_BYREF | DBTYPE_VARIANT))
			{
				GCHECK(VariantClear((VARIANT*)pByRef),S_OK);
			}

			// For all types free the byref buffer the provider has allocated
			SAFE_FREE(pByRef);
			*(&VALUE_BINDING(*pBinding, dwAddrData)) = NULL;
			continue;
		}

		// If the wType of the binding structure is ORed with DBTYPE_ARRAY,
		// and bMemOwner is CLIENTOWNED,
		// the value in the consumer's buffer is a pointer to a safeArray allocated 
		// by the provider which we must free
		if ((pBinding->wType & DBTYPE_ARRAY) && 
			pBinding->dwMemOwner == DBMEMOWNER_CLIENTOWNED) 
		{
			SafeArrayDestroy(*(SAFEARRAY**)pConsumerData);
			continue;
		}

		// If the wType of the binding structure is ORed with DBTYPE_VECTOR,
		// and bMemOwner is CLIENTOWNED, we must free the vector pointer		
		if (pBinding->wType & DBTYPE_VECTOR &&
			pBinding->dwMemOwner == DBMEMOWNER_CLIENTOWNED) 
		{
			// Clean up the out-of-line memory in the vector is a vector of DBTYPE_BSTR
			// or a vector of DBTYPE_VARIANT
			CleanUpVector((DBVECTOR *)pConsumerData,pBinding->wType);		
			PROVIDER_FREE((LPVOID)(((DBVECTOR *)pConsumerData)->ptr));
			continue;
		}

		// If the wType of the binding structure is not ORed with any type
		// modifier, only need to free memory for DBTYPE_BSTR
		if (pBinding->wType == DBTYPE_BSTR &&
			pBinding->dwMemOwner == DBMEMOWNER_CLIENTOWNED) 
		{
			SysFreeString(*(BSTR*)pConsumerData);
			continue;
		}

		if (pBinding->wType == DBTYPE_VARIANT &&
			pBinding->dwMemOwner == DBMEMOWNER_CLIENTOWNED)
			GCHECK(VariantClear((VARIANT*)pConsumerData),S_OK);

		//It is the consumer's responsibility to release the storage object
		//(only release if requested)
		if (pBinding->wType == DBTYPE_IUNKNOWN &&
			pBinding->dwMemOwner == DBMEMOWNER_CLIENTOWNED)
			(*(ISequentialStream**)pConsumerData)->Release();

	} //end of the main loop

END:
	PROVIDER_FREE(wszData);

	if (fErrorOccured)
		return FALSE;
	else
		return TRUE;
}

//---------------------------------------------------------------------------------------
//  MiscFunc CompareBuffer|
//  The function compares the data in two buffers based on the binding
//	structure.  It assumes the two buffers share the same binding structure.
//	When fSetData is TRUE, pSetData should points to a buffer that is used
//	for setdata/setnewdata.  The status of the buffer should only be 
//	DBSTATUS_OK or DBSTATUS_NULL.
//
//	@func BOOL|
//
// @rdesc Comparision results
//  @flag TRUE | Data in the two buffers is the same.
//  @flag FALSE | Data in the two buffers is not the same..
//		
//-------------------------------------------------------------------------------------
BOOL	CompareBuffer(
	void		*pGetData,			//@parm[in]: the pointer to one buffer.  
	void		*pSetData,			//@parm[in]: the pointer to the second buffer This should be the buffer to set the data with
	DBCOUNTITEM	cBindings,			//@parm[in]: the count of rgBindings	
	DBBINDING*	rgBindings,			//@parm[in]: the binding structure of the buffer
	IMalloc		*pIMalloc,			//@parm[in]: IMalloc pointer for free memory
	BOOL		fSetData,			//@parm[in]: pSetData is used for set data.
	BOOL		fReadColumnsByRef,	//@parm[in]: TRUE if the columns are binded by an accessor of type DBBINDIO_READCOLUMNSBYREF
	ECOMPARE_FREE eCompareFree,		//@parm[in]: COMPARE_FREE  compare data and free the memory refereced by the consumer's buffer
									//			 COMPARE_ONLY  compare data only.  Do not attempt to free memory
									//			 FREE_ONLY	   free memory only.  Do not attemp to compare data
	BOOL		fAllowUnavailable,	//@parm[in]: TRUE if DBSTATUS_S_UNAVAILABLE is a valid value in GetData
									//			 e.g. GetData after a SetData or an InsertRow with DBSTATUS_S_DEFAULT
									//			 default value FALSE
	DBCOUNTITEM	cSetBindings,
	DBBINDING*	rgSetBindings,
	BOOL		fStrictVarType		// VT must match exactly, otherwise we will attempt to convert if mismatch
)
{
	DBCOUNTITEM	cCount;
	void*	pGetDataColumn	= NULL;
	void*	pSetDataColumn	= NULL;
	
	BOOL	fErrorOccured	= FALSE;
	BOOL	fCheckForNULL	= FALSE;
	BOOL	fCheckForLength	= FALSE;
	BOOL	fColumnError	= FALSE;
	BOOL	fStatusError	= FALSE;
	
	DBLENGTH	ulSizeSetData = 0;
	DBLENGTH	ulSizeGetData = 0;
	DBTYPE	wType;
	LONG	lDBTypeSize;
	DWORD_PTR	dwAddrGetData;
	DWORD_PTR	dwAddrSetData;

	// Input validation
	ASSERT(rgBindings);
	ASSERT(pGetData);
	ASSERT(pSetData);
	
	//NOTE:  rgSetBindings is optional.  Its provided so variations can compare data retrived
	//either through different bindings, (into different offsets), or from different methods.
	//For example for 2.5 components we can create bindings ontop of DBCOLUMNACCESS strcutures,
	//but the data doesn't have the same offsets and bindings.  This is a very useful feature,
	//but we must very that some of the fundamental items of the bindings are the same...
	if(rgSetBindings == NULL)
		rgSetBindings = rgBindings;

	// Get the address of the starting point of consumer's buffer
	dwAddrGetData= (DWORD_PTR)pGetData;
	dwAddrSetData= (DWORD_PTR)pSetData;

	// Compare the data column by column
	// Status, Length, and Valud binding are checked
	for(cCount=0; cCount<cBindings; cCount++)
	{
		DBBINDING* pGetBinding = &rgBindings[cCount];
		DBBINDING* pSetBinding = &rgSetBindings[cCount];

		//We need to make sure some fundamental items are the same for the bindings.
		//Otherwise we will have problems comparing types, etc...
		ASSERT(pGetBinding->wType		== pSetBinding->wType);
		ASSERT(pGetBinding->bPrecision	== pSetBinding->bPrecision);
		ASSERT(pGetBinding->bScale		== pSetBinding->bScale);
		ASSERT(pGetBinding->dwPart		== pSetBinding->dwPart);

		// Return FALSE if the DBTYPE is ORed with DBTYPE_RESERVED
		if (pGetBinding->wType & DBTYPE_RESERVED)
		{
			PRVTRACE(wszReservedUsed);
			PRVTRACE(L"%u", pGetBinding->iOrdinal);
			PRVTRACE(wszAtBinding);
			PRVTRACE(L"%u", cCount);
			PRVTRACE(ENDL);
			fErrorOccured=TRUE;
			continue;			
		}

		// Init
		fCheckForNULL	= FALSE;
		fCheckForLength	= FALSE;
		fColumnError	= FALSE;
		fStatusError	= FALSE;

		// Free memory directly if not need to compareData
		if (eCompareFree==FREE_ONLY)
			goto CLEANUP;

		// Check the status binding   
		if (STATUS_IS_BOUND(*pGetBinding))
		{
			switch(STATUS_BINDING(*pGetBinding, dwAddrGetData))
			{
				// Since we are comparing 2 buffers here, we really don't care if the
				// status is SOK or not, we just care that both buffers have the same status
				case DBSTATUS_E_SIGNMISMATCH:
				case DBSTATUS_E_CANTCONVERTVALUE:
				case DBSTATUS_E_CANTCREATE:
				case DBSTATUS_E_UNAVAILABLE:
				case DBSTATUS_E_DATAOVERFLOW:
				case DBSTATUS_E_BADACCESSOR:
				case DBSTATUS_E_INTEGRITYVIOLATION:
				case DBSTATUS_E_SCHEMAVIOLATION:
				case DBSTATUS_S_TRUNCATED:
						if (	fAllowUnavailable 
							&&	DBSTATUS_E_UNAVAILABLE == STATUS_BINDING(*pGetBinding, dwAddrGetData))
							break;
						PRVTRACE(wszInvalidStatus);
						PRVTRACE(L"%u", STATUS_BINDING(*pGetBinding, dwAddrGetData));
						PRVTRACE(wszForColumn);
						PRVTRACE(L"%u",pGetBinding->iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fStatusError = TRUE;
						break;			
				
				// If the data is NULL, we need to check the length binding if appropriate
				case DBSTATUS_S_ISNULL:
						fCheckForNULL=TRUE;
					break;

				case DBSTATUS_S_OK:
					break;
				
				default:
						PRVTRACE(wszInvalidStatus);
						PRVTRACE(L"%u", STATUS_BINDING(*pGetBinding, dwAddrGetData));
						PRVTRACE(wszForColumn);
						PRVTRACE(L"%u",pGetBinding->iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fErrorOccured=TRUE;
			}

			// Make sure the status binding of two buffer has to be correct
			if (STATUS_BINDING(*pGetBinding, dwAddrGetData)!=
			   STATUS_BINDING(*pSetBinding, dwAddrSetData))
			{	
				VARIANT *pVariant;
				
				// Variant w/VT_NULL is equivalent to DBSTATUS_S_ISNULL
				if (STATUS_BINDING(*pGetBinding, dwAddrGetData) == DBSTATUS_S_ISNULL &&
					STATUS_BINDING(*pSetBinding, dwAddrSetData) == DBSTATUS_S_OK )
				{
					if ( pGetBinding->wType == DBTYPE_VARIANT )	
					{
						pVariant = (VARIANT *)&VALUE_BINDING(*pSetBinding, dwAddrSetData);
						if ( pVariant->vt == VT_NULL )
							continue;
					}						
				}
				else if (STATUS_BINDING(*pGetBinding, dwAddrGetData) == DBSTATUS_S_OK &&
						STATUS_BINDING(*pSetBinding, dwAddrSetData) == DBSTATUS_S_ISNULL )
				{
					if ( pGetBinding->wType == DBTYPE_VARIANT )	
					{
						pVariant = (VARIANT *)&VALUE_BINDING(*pGetBinding, dwAddrGetData);
						if ( pVariant->vt == VT_NULL )
							continue;
					}						
				}
				
				// Variant w/VT_NULL is equivalent to DBSTATUS_S_ISNULL
				if (	STATUS_BINDING(*pGetBinding, dwAddrGetData) == DBSTATUS_S_OK &&
						STATUS_BINDING(*pSetBinding, dwAddrSetData) == DBSTATUS_S_DEFAULT )
				{
					continue;
				}
				else if (	STATUS_BINDING(*pGetBinding, dwAddrGetData) == DBSTATUS_S_ISNULL &&
							STATUS_BINDING(*pSetBinding, dwAddrSetData) == DBSTATUS_S_DEFAULT )
				{
					continue;
				}
				else if (	fAllowUnavailable &&
							STATUS_BINDING(*pGetBinding, dwAddrGetData) == DBSTATUS_E_UNAVAILABLE &&
							STATUS_BINDING(*pSetBinding, dwAddrSetData) == DBSTATUS_S_DEFAULT )
				{
					continue;
				}

				
				// DBSTATUS_S_IGNORE stuff
				if (	STATUS_BINDING(*pGetBinding, dwAddrGetData) == DBSTATUS_S_OK &&
						STATUS_BINDING(*pSetBinding, dwAddrSetData) == DBSTATUS_S_IGNORE )
				{
					continue;
				}
				else if (	STATUS_BINDING(*pGetBinding, dwAddrGetData) == DBSTATUS_S_ISNULL &&
							STATUS_BINDING(*pSetBinding, dwAddrSetData) == DBSTATUS_S_IGNORE )
				{
					continue;
				}
				else if (	fAllowUnavailable &&
							STATUS_BINDING(*pGetBinding, dwAddrGetData) == DBSTATUS_E_UNAVAILABLE &&
							STATUS_BINDING(*pSetBinding, dwAddrSetData) == DBSTATUS_S_IGNORE )
				{
					continue;
				}
				
				PRVTRACE(wszStatusNotEqual);
				PRVTRACE(L"%u", pGetBinding->iOrdinal);
				PRVTRACE(wszAtBinding);
				PRVTRACE(L"%u", cCount);
				PRVTRACE(ENDL);
				fErrorOccured=TRUE;
				
				//If not the same, dont continue.
				//This is also here, so if the first STATUS is valid and the 
				//second one is NULL or Error, we don't end up comparing 
				//the garbe data of the second buffer!
				continue;
			}

			// Make sure the status binding for set data should be only
			// DBSTATUS_OK or DBSTATUS_ISNULL or DBSTATUS_S_DEFAULT
			if (fSetData)
			{
				if ((STATUS_BINDING(*pSetBinding, dwAddrSetData)!=
					DBSTATUS_S_OK) &&
					(STATUS_BINDING(*pSetBinding, dwAddrSetData)!=
					DBSTATUS_S_ISNULL) &&
					(STATUS_BINDING(*pSetBinding, dwAddrSetData)!=
					DBSTATUS_S_DEFAULT))
				{
					PRVTRACE(wszStatusOKOrNULL);
					PRVTRACE(L"%u", pSetBinding->iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					fErrorOccured=TRUE;
				}
			}
		}

		// Check for the length binding
		if(LENGTH_IS_BOUND(*pGetBinding) && !fStatusError)
		{
			ulSizeGetData = LENGTH_BINDING(*pGetBinding, dwAddrGetData);

			// Skip length checks if status is NULL
			if (fCheckForNULL)
				goto VALUE;

			// Length should be the same as the sizeof(DBTYPE) for fixed length data type
			// Length should be number of bytes of the data for variable length data type,
			// that is, DBTYPE_STR, DBTYPE_BYTES,and DBTYPE_WSTR.
			// Length should not be returned by reference for data ORed with _BYREF
			switch(pGetBinding->wType)
			{
				// No way to check variable lendth data type without a value binding
				case DBTYPE_STR:
				case DBTYPE_STR | DBTYPE_BYREF:
				case DBTYPE_BYTES:
				case DBTYPE_BYTES | DBTYPE_BYREF:
				case DBTYPE_WSTR:
				case DBTYPE_WSTR | DBTYPE_BYREF:
			

					// The length info should be the same for variable length
					// for SetData and GetData
					if (LENGTH_BINDING(*pGetBinding, dwAddrGetData)!=
					LENGTH_BINDING(*pSetBinding, dwAddrSetData))
					{
						PRVTRACE(wszLengthNotEqual);
						PRVTRACE(L"%u", pGetBinding->iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fErrorOccured=TRUE;
					}

					// Remember to check for the length binding if there is a value binding
					fCheckForLength=TRUE;
					break;

				case DBTYPE_VARNUMERIC:
				case DBTYPE_VARNUMERIC | DBTYPE_BYREF:
					// special case because length does not have to be the same
					// just verify that the len falls within an acceptable range
					if (ulSizeGetData < sizeof(DB_VARNUMERIC) || ulSizeGetData > MAX_VARNUM_BYTE_SIZE)
					{
						PRVTRACE(L"Out of range Varnumeric length :");
						PRVTRACE(L"%u", pGetBinding->iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fErrorOccured=TRUE;
					}					
					break;
				// The length is not requried for SetData for fixed length data types
				default:
				   if (!fSetData)
				   {
						// The length info should be the same
						if (LENGTH_BINDING(*pGetBinding, dwAddrGetData)!=
						LENGTH_BINDING(*pSetBinding, dwAddrSetData))
						{
							PRVTRACE(wszLengthNotEqual);
							PRVTRACE(L"%u", pGetBinding->iOrdinal);
							PRVTRACE(wszAtBinding);
							PRVTRACE(L"%u", cCount);
							PRVTRACE(ENDL);
							fErrorOccured=TRUE;
						}
				   }

					// For fixed length data type, make sure the length value is 
					// the same as the size of the data type
					// Get the size of the dta type
					lDBTypeSize=GetDBTypeSize(pGetBinding->wType);

					if(lDBTypeSize==0 || lDBTypeSize==INVALID_DBTYPE_SIZE)
					{
						PRVTRACE(wszInvalidDBTYPE);
						PRVTRACE(L"%u", pGetBinding->iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fColumnError=TRUE;
						break;
					}

					// Compare the data length with length binding
					//NOTE: IUnknown (bound as ISequentialStream) comes back with the 
					//length actually indicating the bytes in the stream, not the size of the pointer
					if(LENGTH_BINDING(*pGetBinding, dwAddrGetData) != (DBLENGTH)lDBTypeSize && 
						 (pGetBinding->wType != DBTYPE_IUNKNOWN))
					{
						PRVTRACE(wszLengthInconsist);
						PRVTRACE(L"%u", pGetBinding->iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fColumnError=TRUE;
						break;
					}
					break;
			}

			// Free the memory allocated by the provider, goto next binding structure
			if (fColumnError)
			{
				fErrorOccured=TRUE;
				goto CLEANUP;
			}
		}
	
VALUE:
		// Check for value binding
		if(VALUE_IS_BOUND(*pGetBinding) && !fStatusError)
		{	
			// skip checking the value binding for BOOKMARKS
			if (pGetBinding->iOrdinal==0)
				goto CLEANUP;

			// Get the data in the consumer's buffer
			pGetDataColumn=(void *)&VALUE_BINDING(*pGetBinding, dwAddrGetData);
			pSetDataColumn=(void *)&VALUE_BINDING(*pSetBinding, dwAddrSetData);

			// If the accessor is of type ReadColumnsByRef, the pointer has to be NULL
			if (fCheckForNULL)
			{
				if (fReadColumnsByRef)
				{
					if ((void *)(*(DWORD_PTR *)pGetDataColumn)!=NULL)
					{	
						PRVTRACE(wszPointerNotNULL);
						PRVTRACE(L"%u", pGetBinding->iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fErrorOccured=TRUE;
					}

					// If the data is not from SetData, the pointer should be NULL as well.
					if (!fSetData)
					{
					 	if ((void *)(*(DWORD_PTR *)pSetDataColumn)!=NULL)
						{	
							PRVTRACE(wszPointerNotNULL);
							PRVTRACE(L"%u", pGetBinding->iOrdinal);
							PRVTRACE(wszAtBinding);
							PRVTRACE(L"%u", cCount);
							PRVTRACE(ENDL);
							fErrorOccured=TRUE;
						}
					}
				}

				continue;
			}

			// Get the size of the data for DBTYPE_BYTES.  It is set to 0
			// If not length binding is used
			if (LENGTH_IS_BOUND(*pSetBinding))
				ulSizeSetData = LENGTH_BINDING(*pSetBinding, dwAddrSetData);
			else if (pSetBinding->wType == DBTYPE_STR)
				ulSizeSetData = strlen((CHAR *)&VALUE_BINDING(*pSetBinding, dwAddrSetData));
			else if (pSetBinding->wType == DBTYPE_WSTR)
				ulSizeSetData = wcslen((WCHAR *)&VALUE_BINDING(*pSetBinding, dwAddrSetData))*sizeof(WCHAR);
			else if (pSetBinding->wType == DBTYPE_BYTES)
				ulSizeSetData = pSetBinding->cbMaxLen;
			else
				ulSizeSetData=0;

			// Get the size of the data for DBTYPE_BYTES.  It is Get to 0
			// If not length binding is used
			if (LENGTH_IS_BOUND(*pGetBinding))
				ulSizeGetData = LENGTH_BINDING(*pGetBinding, dwAddrGetData);
			else if (pGetBinding->wType == DBTYPE_STR)
				ulSizeGetData = strlen((CHAR *)&VALUE_BINDING(*pGetBinding, dwAddrGetData));
			else if (pGetBinding->wType == DBTYPE_WSTR)
				ulSizeGetData = wcslen((WCHAR *)&VALUE_BINDING(*pGetBinding, dwAddrGetData))*sizeof(WCHAR);
			else if (pGetBinding->wType == DBTYPE_BYTES)
				ulSizeGetData = pGetBinding->cbMaxLen;
			else
				ulSizeGetData=0;


			// Check for non-null values
			// convert the DB data into DBTYPE data
			// check data ORed with DBTYPE_ARRAY
			if (pGetBinding->wType & DBTYPE_ARRAY)
			{
				// Unset the bit DBTYPE_ARRAY
				wType=pGetBinding->wType & (~DBTYPE_ARRAY);
				
				// Compare the data, does not free memory pointed by pSetDataColumn
				if (!CompareSafeArray(*(SAFEARRAY**)pGetDataColumn, *(SAFEARRAY**)pSetDataColumn, wType, NULL, FALSE))
				{
					PRVTRACE(wszSafeArrayNotEqual);
					PRVTRACE(L"%u", pGetBinding->iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					PRVTRACE(wszInvalidValueBinding);
					PRVTRACE(L"%u", pGetBinding->iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					fErrorOccured=TRUE;
				}

				goto CLEANUP;			
			}

			// Check data ORed with DBTYPE_VECTOR
			if (pGetBinding->wType & DBTYPE_VECTOR)
			{	
				// Unset the bit DBTYPE_VECTOR
				wType=pGetBinding->wType & (~DBTYPE_VECTOR);

				// For readColumnsByRef, the data in the consumer's buffer is a 
				// pointer to the real 
				if (fReadColumnsByRef)
				{
					// Compare the data, does not free memory pointed by pSetDataColumn
					if (!CompareVector( (DBVECTOR *)(*(DWORD_PTR *)pGetDataColumn),
										(DBVECTOR *)(*(DWORD_PTR *)pSetDataColumn),
										wType,
										ulSizeSetData,
										pGetBinding->bPrecision,
										pGetBinding->bScale,
										FALSE/*fFreeMemory*/))
						fColumnError=TRUE;
				}
				else
				{
					// Compare the data, no free memory pointed by pSetDataColumn
					if (!CompareVector( (DBVECTOR *)pGetDataColumn,
										(DBVECTOR *)pSetDataColumn,
										wType,
										ulSizeSetData,
										pGetBinding->bPrecision,
										pGetBinding->bScale,
										FALSE/*fFreeMemory*/))
						fColumnError=TRUE;
				}

				if (fColumnError)
				{
					PRVTRACE(wszVectorNotEqual);
					PRVTRACE(L"%u", pGetBinding->iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					PRVTRACE(wszInvalidValueBinding);
					PRVTRACE(L"%u", pGetBinding->iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					fErrorOccured=TRUE;
				}

				goto CLEANUP;			
			}

			// check data ORed with DBTYPE_BYREF
			if (pGetBinding->wType & DBTYPE_BYREF)
			{
				// Unset the bit DBTYPE_BYREF
				wType=pGetBinding->wType & (~DBTYPE_BYREF);

				// If the data is ORed with DBTYPE_BYREF
 				if (fReadColumnsByRef)
				{
					// Compare the data, does not free memory pointed by pSetDataColumn
					if (!CompareDBTypeData((void *)(*(DWORD_PTR *)pGetDataColumn),
										   (void *)(*(DWORD_PTR *)pSetDataColumn),
													wType,
												    ulSizeSetData,
													pGetBinding->bPrecision,
													pGetBinding->bScale,
													NULL,
													FALSE,
													DBTYPE_EMPTY,
													ulSizeGetData,
													fStrictVarType))
						fColumnError=TRUE;
				}
				else
				{
					// Compare the data, does not free memory pointed by pSetDataColumn
					if (!CompareDBTypeData((void *)(*(DWORD_PTR *)pGetDataColumn),
										   (void *)(*(DWORD_PTR *)pSetDataColumn),
												wType,
												ulSizeSetData,
												pGetBinding->bPrecision,
												pGetBinding->bScale,
												NULL,
												FALSE,
												DBTYPE_EMPTY,
												ulSizeGetData,
												fStrictVarType))
						fColumnError=TRUE;
				}

				if (fColumnError)
				{
					PRVTRACE(wszInvalidValueBinding);
					PRVTRACE(L"%u", pGetBinding->iOrdinal);
					PRVTRACE(wszAtBinding);
					PRVTRACE(L"%u", cCount);
					PRVTRACE(ENDL);
					fColumnError=FALSE;
					fErrorOccured=TRUE;
				}
			
				// Make sure for variable length data, the length in *pGetBinding contains
				// correct information
				// If the data is ORed with DBTYPE_BYREF
				if (fCheckForLength)
				{
					switch(wType)
					{
						case DBTYPE_STR:
									if (LENGTH_BINDING(*pGetBinding, dwAddrGetData) 
									!= strlen((char *)(*(DWORD_PTR *)pGetDataColumn)) )
										fColumnError=TRUE;
							break;

						case DBTYPE_WSTR:
									if (LENGTH_BINDING(*pGetBinding, dwAddrGetData)
									!= (wcslen((WCHAR *)(*(DWORD_PTR *)pGetDataColumn))*sizeof(WCHAR)) )
										fColumnError=TRUE;
							break;

						case DBTYPE_BYTES:
							if (LENGTH_BINDING(*pGetBinding, dwAddrGetData) != ulSizeSetData)
								fColumnError=TRUE;

						default:
							break;
					}

					if (fColumnError)
					{
						PRVTRACE(wszLengthInconsist);
						PRVTRACE(L"%u", pGetBinding->iOrdinal);
						PRVTRACE(wszAtBinding);
						PRVTRACE(L"%u", cCount);
						PRVTRACE(ENDL);
						fErrorOccured=TRUE;
					}
				}

				goto CLEANUP;			
			}


			// Compare the data not ORed with any type modifiers
			// and free memory pointed by pSetDataColumn
			if(pGetBinding->wType == DBTYPE_IUNKNOWN)
			{
				//TODO
			}
			else 
			{
				if (!CompareDBTypeData(pGetDataColumn,
												pSetDataColumn,
												pGetBinding->wType,
												ulSizeSetData,
												pGetBinding->bPrecision,
												pGetBinding->bScale,
												NULL,
												FALSE,
												DBTYPE_EMPTY,
												ulSizeGetData,
												fStrictVarType))
					fColumnError=TRUE;
			}

			if (fColumnError)
			{
				//PRVTRACE(wszInvalidValueBinding);
				//PRVTRACE(L"%u", pGetBinding->iOrdinal);
				//PRVTRACE(wszAtBinding);
				//PRVTRACE(L"%u", cCount);
				//PRVTRACE(ENDL);
				fColumnError=FALSE;
				fErrorOccured=TRUE;
			}

			// Make sure for variable length data, the length in *pGetBinding contains
			// correct information
			switch(pGetBinding->wType)	
			{	
				case DBTYPE_STR:
					// The data may contain embedded null termination characters
					if (ulSizeSetData == strlen((char *)pSetDataColumn))
					{
						// No embedded null terminators, strlen should match
						if (ulSizeSetData != strlen((char *)pGetDataColumn))
							fColumnError=TRUE;	
					}

					// The retrieved length binding should always match the set value
					if (fCheckForLength && LENGTH_BINDING(*pGetBinding, dwAddrGetData) != ulSizeSetData)	
						fColumnError=TRUE;	

					break;

				case DBTYPE_WSTR:
					// The data may contain embedded null termination characters
					if (ulSizeSetData == wcslen((WCHAR *)pSetDataColumn) * sizeof(WCHAR))
					{
						// No embedded null terminators, wcslen should match
						if (ulSizeSetData != wcslen((WCHAR *)pGetDataColumn) * sizeof(WCHAR))
							fColumnError=TRUE;	
					}

					// The retrieved length binding should always match the set value
					if (fCheckForLength && LENGTH_BINDING(*pGetBinding, dwAddrGetData) != ulSizeSetData)	
						fColumnError=TRUE;	

					break;	

				case DBTYPE_BYTES:	
					if (fCheckForLength && LENGTH_BINDING(*pGetBinding, dwAddrGetData) != ulSizeSetData)	
						fColumnError=TRUE;	

				default:	
					break;	
			}	

			// Output an error message if the length binding fails
			if (fColumnError)	
			{	
				PRVTRACE(wszLengthInconsist);
				PRVTRACE(L"%u", pGetBinding->iOrdinal);
				PRVTRACE(wszAtBinding);
				PRVTRACE(L"%u", cCount);
				PRVTRACE(ENDL);
				fErrorOccured=TRUE;	
			}	
		}

CLEANUP:
		
		// Go on to the next binding structure if no need to free memory allocated
		// by the provider or the accessor is of type DBACCESSOR_PASSCOLUMNSBYREF, or
		// there is no value binding for the current binding structure
		if ((eCompareFree==COMPARE_ONLY)|| fReadColumnsByRef || 
			!VALUE_IS_BOUND(*pGetBinding) )
			continue;
						   
		// Make sure the status binding indicate the column is OK
		// if there is no status binding, we assume the buffer is correct
		if (STATUS_IS_BOUND(*pGetBinding))
		{
			if (STATUS_BINDING(*pGetBinding, dwAddrGetData) != DBSTATUS_S_OK)
			continue;
		}

		// Get the data in the consumer's buffer
		pGetDataColumn=(void *)&VALUE_BINDING(*pGetBinding, dwAddrGetData);
		pSetDataColumn=(void *)&VALUE_BINDING(*pSetBinding, dwAddrSetData);


		// If the wType of the binding structure is ORed with DBTYPE_BYREF,
		// the value in the consumer's buffer is a pointer to the data allocated 
		// by the provider
		if (pGetBinding->wType & DBTYPE_BYREF)
		{
			DBTYPE wBaseType = pGetBinding->wType & ~DBTYPE_BYREF;
			void* pGetByRef = *(void**)pGetDataColumn;
			void* pSetByRef = *(void**)pSetDataColumn;
			
			//Free Any out-of-line data...
			switch(wBaseType)
			{
				case DBTYPE_BSTR:
					SysFreeString(*(BSTR*)pGetByRef);
					SysFreeString(*(BSTR*)pSetByRef);
					break;
			

				case DBTYPE_VARIANT:
					GCHECK(VariantClear((VARIANT*)pGetByRef),S_OK);
					GCHECK(VariantClear((VARIANT*)pSetByRef),S_OK);
					break;
			};

			//Free the byref pointer.
			CoTaskMemFree(pGetByRef);
			CoTaskMemFree(pSetByRef);
			continue;
		}

		// If the wType of the binding structure is ORed with DBTYPE_ARRAY,
		// the value in the consumer's buffer is a pointer to a safeArray allocated 
		// by the provider
		if (pGetBinding->wType & DBTYPE_ARRAY)
		{
			SafeArrayDestroy(*(SAFEARRAY**)pGetDataColumn);
			SafeArrayDestroy(*(SAFEARRAY**)pSetDataColumn);
			continue;
		}

		// If the wType of the binding structure is ORed with DBTYPE_VECTOR,
		// the value in the consumer's buffer is a DBVECTOR structure
		if (pGetBinding->wType & DBTYPE_VECTOR)
		{
			// Clean up the memory in case the vector contains out-of-line data...
			CleanUpVector((DBVECTOR*)pGetDataColumn, pGetBinding->wType);
			CleanUpVector((DBVECTOR*)pSetDataColumn, pSetBinding->wType);

			//TODO: uncomment after the bug fix
			//PROVIDER_FREE((LPVOID)(((DBVECTOR *)pGetDataColumn)->ptr));
			//PROVIDER_FREE((LPVOID)(((DBVECTOR *)pSetDataColumn)->ptr));
			continue;
		}

		// If the wType of the binding structure is not ORed with any type
		// modifier, only need to free memory for DBTYPE_BSTR and DBTYPE_VECTOR
		if (pGetBinding->wType == DBTYPE_BSTR)
		{
			SysFreeString(*(BSTR*)pGetDataColumn);
			SysFreeString(*(BSTR*)pSetDataColumn);
			continue;
		}

		if (pGetBinding->wType == DBTYPE_VARIANT)
		{
			GCHECK(VariantClear((VARIANT*)pGetDataColumn),S_OK);
			GCHECK(VariantClear((VARIANT*)pSetDataColumn),S_OK);
		}
	}

	if (fErrorOccured)
		return FALSE;
	else
		return TRUE;
}






//--------------------------------------------------------------------------------
// @func compare two SageArrays.
// Free memory pointed by pBackEndData if required.
//
// @rdesc Comparision results
//  @flag TRUE | Data in the two buffers is the same.
//  @flag FALSE | Data in the two buffers is not the same..
//		
//-------------------------------------------------------------------------------------
BOOL CompareSafeArray(
	SAFEARRAY	*pConsumerData,	//@parm [in]: the pointer to a safearray in the consumer's buffer
	SAFEARRAY	*pBackEndData,	//@parm [in]: the pointer to a safeaarry at the backend
	DBTYPE		wType,			//@parm [in]: the expected DBType of the data in the safearray.
								//		It will be checked against the base vt type of 
								//		the safearray.  If wType==INVALID_DBTYPE, the 
								//		SafeArray's base type will not be checked.  It
								//		can not be ORed with any type modifers.
	IMalloc		*pIMalloc,		//@parm [in]: pointer to IMalloc.  Can be NULL if fFreeMemory is FALSE.
	BOOL		fFreeMemory		//parm [in]: whether to free the memory pointed by pBackEndData
)
{	
	BOOL		fSame=FALSE;
	LONG		*pLBound=NULL;
	LONG		*pUBound=NULL;
	LONG		lBound;
	ULONG		ulDim;
	ULONG		ulDimCount;
	ULONG		ulElements;
	ULONG		ulCnt;
	ULONG		ulSum;
	ULONG		ulIndex;
	//VARIANT		DataOrg;
	//VARIANT		DataCpy;
	HRESULT		hr;
	//div_t		div_result;		//the return data type from run time function DIV
	void *		pArrayConsumer=NULL;
	void *		pArrayBackend=NULL;

	// Input validation
	ASSERT(pConsumerData);
	ASSERT(pBackEndData);
	wType = (wType & (~DBTYPE_ARRAY));

	// Compare the dimension of the safearray
	ulDim=SafeArrayGetDim(pConsumerData);
	ulDimCount=SafeArrayGetDim(pBackEndData);
	
	if (ulDim != ulDimCount)
		goto CLEANUP;
	
	// Return TRUE for an empty safeArray
	if (!ulDim)
	{
		fSame=TRUE;
		goto CLEANUP;
	}

	// Allocate memory
	pLBound=(LONG *)PROVIDER_ALLOC(sizeof(LONG)*ulDim);
	pUBound=(LONG *)PROVIDER_ALLOC(sizeof(LONG)*ulDim);

	if (!pUBound || !pLBound)
		goto CLEANUP;

	// Compare the lower bound of all dimensions
	for(ulCnt=1;ulCnt <= ulDim;ulCnt++)
	{
		hr=SafeArrayGetLBound(pConsumerData, ulCnt, &pLBound[ulCnt-1]);
		
		if (FAILED(hr))
			goto CLEANUP;

		hr=SafeArrayGetLBound(pBackEndData, ulCnt, &lBound);
		
		if (FAILED(hr))
			goto CLEANUP;
		
		// Return FALSE if the two safearray have differenct lower bound in a dimension
		if (lBound != pLBound[ulCnt-1])
			goto CLEANUP;
	}

	// Compare the upper bound of all dimensions
	for(ulCnt=1;ulCnt <= ulDim;ulCnt++)
	{
		hr=SafeArrayGetUBound(pConsumerData, ulCnt, &pUBound[ulCnt-1]);
		
		if (FAILED(hr))
			goto CLEANUP;

		hr=SafeArrayGetUBound(pBackEndData, ulCnt, &lBound);
		
		if (FAILED(hr))
			goto CLEANUP;

		// Return FALSE if the two safearray have different upper bound in a dimension
		if (lBound != pUBound[ulCnt-1])
			goto CLEANUP;
	}

	// Initialize
	ulElements=0;

	// Calcualting the count of elements in the safearray
	for(ulCnt=0;ulCnt<ulDim;ulCnt++)
		ulElements += (pUBound[ulCnt]-pLBound[ulCnt]+1);

	// Get a copy of the count of elements in the safearray
	ulSum=ulElements;
	
	//compare each element
	for(ulIndex=0; ulIndex<ulElements; ulIndex++)
	{
		// Assume an element can never be larger than the containing Variant
		BYTE pConsumerElement[sizeof(VARIANT)];
		BYTE pBackEndElement[sizeof(VARIANT)];
		BYTE bPrecision = 28;	// Maximum precision of DBTYPE_DECIMAL, from Appendix A
		BYTE bScale = ~0;		// Ignored for other variant types
		
		if (wType == DBTYPE_DECIMAL)
			bScale = ((DECIMAL *)pBackEndElement)->scale;

		TESTC_(SafeArrayGetElement(pConsumerData, (LONG *)&ulIndex, pConsumerElement), S_OK);
		TESTC_(SafeArrayGetElement(pBackEndData, (LONG *)&ulIndex, pBackEndElement), S_OK);

		// We don't need to pass ulBackEndSize because that only applies to DBTYPE_BYTES
		// which doesn't apply to variants.  But we should pass bPrecision and
		// bScale because they apply to DBTYPE_DECIMAL and can appear in a variant.
		// So we will assume the max precision and backend scale are appropriate.
		if (!CompareDBTypeData(pBackEndElement,pConsumerElement,wType,0,
			bPrecision,bScale,NULL,FALSE))
			goto CLEANUP;
	}


	fSame=TRUE;

CLEANUP:
	PROVIDER_FREE(pLBound);
	PROVIDER_FREE(pUBound);

	//Need to Unlock the Array
	if(pConsumerData && pArrayConsumer)
		SafeArrayUnaccessData(pConsumerData);
	
	if (fFreeMemory)
		SafeArrayDestroy(pBackEndData);

	return fSame;
}

//--------------------------------------------------------------------------------
// @func Compare two vectors.
// Free memory pointed by pBackEndData if required.  
//
// @rdesc Comparision results
//  @flag TRUE | Data in the two buffers is the same.
//  @flag FALSE | Data in the two buffers is not the same..
//		
//-------------------------------------------------------------------------------------
BOOL CompareVector
(
	DBVECTOR	*pConsumerData,	//@parm [in]: the pointer to the vector in consumer's buffer
	DBVECTOR	*pBackEndData,	//@parm [in]: the pointer to the vector at the backend
	DBTYPE		wType,			//@parm [in]: the DBType indicator of the data.  It can not be ORed with any DBType modifiers.
	DBLENGTH	ulSize,			//@parm [in]: the size of one element in the vector. Only valid for DBTYPE_BYTES
	BYTE		bPrecision,		//@parm [in]: the precision Only valid for DBTYPE_NUMERIC or DBTYPE_DECIMAL
	BYTE		bScale,			//@parm [in]: the scale Only valid for DBTYPE_NUMERIC or DBTYPE_DECIMAL
	BOOL		fFreeMemory		//@parm [in]: whether to free the memory pointed by pBackEndData
)
{
	BOOL	fSame = TRUE;
	DBLENGTH	ulCnt;
	void	*pDataOrg;
	void	*pDataCpy;
	DBLENGTH	lSize;

	// Input validation
	ASSERT(pConsumerData);
	ASSERT(pBackEndData);

	// Return FALSE for data of type DBTYPE_EMPTY, DBTYPE_NULL,
	// DBTYPE_IUNKNOWN, DBTYPE_IDISPATCH, or DBTYPE_UDT
	if (wType == DBTYPE_EMPTY	  ||
		wType == DBTYPE_NULL	  ||
		wType == DBTYPE_IUNKNOWN  ||
		wType == DBTYPE_IDISPATCH ||
		wType == DBTYPE_UDT)
		goto END;

	// The input data should not be ORed with any type modifiers
	if ((wType & DBTYPE_RESERVED) ||
		(wType & DBTYPE_ARRAY)	  ||
		(wType & DBTYPE_VECTOR)	  ||
		(wType & DBTYPE_BYREF ))
		goto END;

	// Make sure the two vectors have the same count
	if (pConsumerData->size != pBackEndData->size)
		goto END;
	
	// Calculate the size of one element in the vector
	lSize=GetDBTypeSize(wType);

	// Return FALSE if wType is invalid
	// NOTE: Vector is only valid on Fixed Length types...
	if (lSize==INVALID_DBTYPE_SIZE || lSize==0)
		goto END;

	// Init the pointer 
	pDataOrg=pConsumerData->ptr;
	pDataCpy=pBackEndData->ptr;

	// Check the array in the vector element by element
	for(ulCnt=0;ulCnt<pConsumerData->size;ulCnt++)
	{
		// compare one element in the vector.  Do not attempt to free memory
		if (fSame)
		{
			if (!CompareDBTypeData(pDataOrg,pDataCpy,wType,ulSize,bPrecision,bScale,NULL,FALSE))
				fSame=FALSE;
		}
			
		// Free any out-of-line memory by the element in the vector
		if (fFreeMemory)
		{
			switch(wType)
			{
				case DBTYPE_BSTR:
					SysFreeString(*(BSTR*)pDataCpy);
					break;
			
				case DBTYPE_VARIANT:
					GCHECK(VariantClear((VARIANT*)pDataCpy),S_OK);
					break;
			};
		}

		// Move pointers to the next element
		pDataOrg=(void * )((BYTE *)pDataOrg + lSize);
		pDataCpy=(void * )((BYTE *)pDataCpy + lSize);
	}
	
END:
	// Free the memory is requested
	if (fFreeMemory)
	{
		PROVIDER_FREE(pBackEndData->ptr);
		PROVIDER_FREE(pBackEndData);
	}

	return fSame;
}

//---------------------------------------------------------------------------------
// @func Compare two data of any DBTYPE. 
// Free memory pointed by pBackEndData if requested.
//
// @rdesc Comparision results
//  @flag TRUE | Data in the two buffers is the same.
//  @flag FALSE | Data in the two buffers is not the same..
//		
//-------------------------------------------------------------------------------------
BOOL CompareDBTypeData(
	void	*pConsumerData,	//@parm [in]: the pointer to consumer data
	void	*pBackEndData,	//@parm [in]: the pointer to data at the backend
	DBTYPE	wType,			//@parm [in]: the DBType of the data.  It can not be ORed with any DBType modifers.
	DBLENGTH	ulBackEndSize,	//@parm [in]: the size of the data, only valid for DBTYPE_BYTES or DBTYPE_VARNUMERIC
	BYTE	bPrecision,		//@parm [in]: the precision Only valid for DBTYPE_NUMERIC or DBTYPE_DECIMAL
	BYTE	bScale,			//@parm [in]: the scale Only valid for DBTYPE_NUMERIC or DBTYPE_DECIMAL
	IMalloc	*pIMalloc,		//@parm [in]: the pointer tp IMalloc for freeing memory. pIMalloc can not NULL if fFreeMemory is FALSE.
	BOOL	fFreeMemory,	//@parm [in]: whether to free the memory pointed by pBackEndData.
	DBTYPE	wBackEndType,	//@parm [in]: data type of the backend, default DBTYPE_EMPTY.
	DBLENGTH	cbConsumerSize,	//@parm [in]: the size of data pointed to by pConsumerData, only valid for DBTYPE_VARNUMERIC,
							//				DBTYPE_STR, DBTYPE_WSTR.	
	BOOL	fApproxVarTypes
)
{
	BOOL fSame = FALSE;

	// Input validation
	ASSERT(pConsumerData);
	ASSERT(pBackEndData);
	
	// The input data should not be ORed with any type modifiers
	if ((wType & DBTYPE_RESERVED ) ||
		(wType & DBTYPE_ARRAY)	   ||
		(wType & DBTYPE_VECTOR)    ||
		(wType & DBTYPE_BYREF ))
		goto END;

	switch(wType)
	{
		case DBTYPE_EMPTY:
		case DBTYPE_NULL:
			goto END;

		case DBTYPE_I1:
				if (!memcmp(pConsumerData, pBackEndData,GetDBTypeSize(DBTYPE_I1)))
					fSame=TRUE;
			goto END;

		case DBTYPE_I2:
				if (*(SHORT *)pConsumerData == *(SHORT *)pBackEndData)
					fSame=TRUE;
			goto END;

		case DBTYPE_I4:
				if (*(LONG *)pConsumerData == *(LONG *)pBackEndData)
					fSame=TRUE;
			goto END;

		case DBTYPE_I8:
		case DBTYPE_CY:
		case DBTYPE_UI8:
		case DBTYPE_FILETIME:
				if (!memcmp(pConsumerData, pBackEndData,GetDBTypeSize(DBTYPE_I8)))
					fSame=TRUE;
			goto END;

		case DBTYPE_UI1: 
				if(*(BYTE *)pConsumerData == *(BYTE *)pBackEndData)
					fSame=TRUE;
			goto END;

		case DBTYPE_UI2:
				if (*(USHORT *)pConsumerData == *(USHORT *)pBackEndData)
					fSame=TRUE;
			goto END;

		case DBTYPE_UI4:
		case DBTYPE_HCHAPTER:
				if (*(ULONG *)pConsumerData == *(ULONG *)pBackEndData)
					fSame=TRUE;
			goto END;

		case DBTYPE_R4:
				if (*(float *)pConsumerData == *(float *)pBackEndData)
					fSame=TRUE;
			goto END;

		case DBTYPE_R8:
		case DBTYPE_DATE:
//				if (*(double *)pConsumerData == *(double *)pBackEndData)
 				if (fabs((*(double *)pConsumerData) - (*(double *)pBackEndData)) <= 
					fabs(g_dDoubleTolerance*(*(double *)pConsumerData)))
 				{
					fSame=TRUE;
				}
			goto END;

		case DBTYPE_NUMERIC:
				//The precision and scale of numeric is defined as the maximum
				//So we should always be comparing to the bindings maximum
				//prec/scale of the metadata...
				if ((((DB_NUMERIC *)pConsumerData)->precision == /*((DB_NUMERIC *)pBackEndData)->precision*/bPrecision) &&
				    (((DB_NUMERIC *)pConsumerData)->scale == /*((DB_NUMERIC *)pBackEndData)->scale*/bScale) &&
				    (((DB_NUMERIC *)pConsumerData)->sign == ((DB_NUMERIC *)pBackEndData)->sign) &&
				    (!memcmp( (void *)(((DB_NUMERIC *)pConsumerData)->val),
						(void *)(((DB_NUMERIC *)pBackEndData)->val),MAXNUMERICLEN)))
					fSame=TRUE;
			goto END;

		case DBTYPE_VARNUMERIC:
			// Must be flexible in comparing scale/precision
			// casting cbConsumerSize to USHORT is safe since byte length of VARNUMERIC <= 110 bytes
			fSame = CompareVarNumeric((DB_VARNUMERIC *)pConsumerData,USHORT(cbConsumerSize),(DB_VARNUMERIC *)pBackEndData,(USHORT)ulBackEndSize);					
			goto END;
		
		case DBTYPE_DECIMAL:
			// This comparison performs no metadata checking
			fSame = CompareDecimal((DECIMAL *)pConsumerData, (DECIMAL *)pBackEndData);
			goto END;

		case DBTYPE_BOOL:
				if (!memcmp(pConsumerData, pBackEndData, 2))
					fSame=TRUE;
			goto END;

		case DBTYPE_BYTES:
				if (!memcmp(pConsumerData, pBackEndData,(size_t)ulBackEndSize))
					fSame=TRUE;
			goto END;

		case DBTYPE_STR:
			{
				// Consumer data should always be null terminated
				if (*(CHAR *)((BYTE *)pConsumerData + cbConsumerSize) != '\0')
				{
					fSame = FALSE;
					goto END;
				}

				// We can't compare EMPTY types by reallocating and converting
				if (wBackEndType != DBTYPE_EMPTY &&
					wBackEndType != DBTYPE_NULL)
				{

					WCHAR * pwszBackEnd = (WCHAR *)PROVIDER_ALLOC((ulBackEndSize+1) * sizeof(WCHAR));
					WCHAR * pwszConsumer = (WCHAR *)PROVIDER_ALLOC((cbConsumerSize+1) * sizeof(WCHAR));
					if (pwszBackEnd && pwszConsumer)
					{
						memset(pwszBackEnd, 0, (size_t)((ulBackEndSize+1) * sizeof(WCHAR)));
						memset(pwszConsumer, 0, (size_t)((cbConsumerSize+1) * sizeof(WCHAR)));

						//64bit TODO - remove the (INT) casts eventually.

						if (MultiByteToWideChar(CP_ACP, 0, (CHAR *)pBackEndData, (INT)ulBackEndSize,
							pwszBackEnd, (INT) ulBackEndSize+1) &&
							MultiByteToWideChar(CP_ACP, 0, (CHAR *)pConsumerData, (INT)cbConsumerSize,
							pwszConsumer, (INT) cbConsumerSize+1))
						{
							// MBTWC won't null terminate when the length is passed in, so we have to do it...
							pwszBackEnd[ulBackEndSize] = L'\0';
							pwszConsumer[cbConsumerSize] = L'\0';

							fSame = CompareWCHARData(pwszConsumer, pwszBackEnd, wBackEndType, 
								ulBackEndSize * sizeof(WCHAR), cbConsumerSize * sizeof(WCHAR));
						}

					}
					
					PROVIDER_FREE(pwszBackEnd);
					PROVIDER_FREE(pwszConsumer);
				}
				else
				{
					// Can't use strcmp because the data might contain embedded null terminators
					fSame = (ulBackEndSize == cbConsumerSize);
					if (ulBackEndSize == strlen((CHAR *)pBackEndData))
						fSame &= (ulBackEndSize == strlen((CHAR *)pConsumerData));
					fSame &= !memcmp(pConsumerData, pBackEndData, (size_t)ulBackEndSize);
				}
			}

			goto END;

		case DBTYPE_WSTR:
				fSame = CompareWCHARData(pConsumerData, pBackEndData, wBackEndType,
					ulBackEndSize, cbConsumerSize);

			goto END;

		case DBTYPE_BSTR:
				if (SysStringByteLen(*(BSTR *)pConsumerData) != SysStringByteLen( *(BSTR *)pBackEndData) )
					goto END;

				if (memcmp(*((BSTR *)pConsumerData), *((BSTR *)pBackEndData),
						  (SysStringByteLen(*(BSTR*)pConsumerData)+sizeof(L'\0'))))
					goto END;

				// Make sure the string is NULL terminated
				if (*(WCHAR *)((BYTE *)(*(BSTR *)pConsumerData) +SysStringByteLen(*(BSTR *)pConsumerData))== L'\0')
				   fSame=TRUE;
			goto END;

		case DBTYPE_VARIANT:
				fSame=CompareVariant((VARIANT *)pConsumerData,(VARIANT *)pBackEndData, TRUE, fApproxVarTypes);
			goto END;

		// As we are not testing OLE object, we should not run into data of type
		// DBTYPE_IDISPATCH, an error must have occured
		case DBTYPE_IDISPATCH:
			goto END;

		// Should just be able to compare IUNKNOWN as bytes, since the buffers
		// Should have already been retrieved with ISeqStream->Read
		case DBTYPE_IUNKNOWN:
			if (memcmp(pConsumerData, pBackEndData, (size_t)ulBackEndSize)==0)
					fSame=TRUE;
			goto END;

		case DBTYPE_GUID:
				if (*(GUID *)pConsumerData== *(GUID *)pBackEndData)
					fSame=TRUE;
			goto END;

		case DBTYPE_ERROR:
				if (*(SCODE *)pConsumerData == *(SCODE *)pBackEndData)
					fSame=TRUE;
			goto END;

		// Use a generic function to compare Data Time structs defined in OLE DB
		case DBTYPE_DBDATE:
		case DBTYPE_DBTIME:
		case DBTYPE_DBTIMESTAMP:
				if (CompareDateTime(pConsumerData, pBackEndData,wType))
					fSame=TRUE;
			goto END;

		// No way to compare user defined data types
		case DBTYPE_UDT:
		default:
			goto END;
	}

END:
	// Free the memory if requested
	if (fFreeMemory)
	{
		// Use SysFreeString SafeArray
		// *pBackEndData will not ORed with _ARRAY or _VECTOR, it is handled in CompareSafeArray
		// For any other data types, use IMalloc->Free to free the memory.
		
		if(wType & DBTYPE_ARRAY)
		{
			SafeArrayDestroy(*(SAFEARRAY**)pBackEndData);
		}
		else
		{
			switch(wType)
			{
				case DBTYPE_BSTR:
					SysFreeString(*(BSTR*)pBackEndData);
					break;
		
				case DBTYPE_VARIANT:
					GCHECK(VariantClear((VARIANT*)pBackEndData),S_OK);
					PROVIDER_FREE(pBackEndData);
					break;

				default:
					PROVIDER_FREE(pBackEndData);
					break;
			};
		}
	}
	
	return fSame;
}

//------------------------------------------------------------------------------------
// @func: Compare two date time structs.  Does not attempt to free memory
// The DBType can be either DBTYPE_DATESTRUCT, DBTYPE_TIMESTRUCT, or
// DBTYPE_TIMESTAMPSTRUCT
//
// @rdesc Comparision results
//  @flag TRUE | Data in the two buffers is the same.
//  @flag FALSE | Data in the two buffers is not the same..
//		
//-------------------------------------------------------------------------------------
BOOL CompareDateTime(
	void *pConsumerData,	//@parm [in]: Pointer to the date time struct in the consumer's buffer.
	void *pBackEndData,		//@parm [in]: Pointer to the date time struct at the backend.
	DBTYPE wType			//@parm [in]: the type indicator
)
{
	BOOL fSame = FALSE;

	// Input validation
	ASSERT(pConsumerData);
	ASSERT(pBackEndData);

	switch(wType)
	{
		case DBTYPE_DBDATE:
			if ((((DBDATE *)pConsumerData)->year==((DBDATE *)pBackEndData)->year) &&
			    (((DBDATE *)pConsumerData)->month==((DBDATE *)pBackEndData)->month) &&
                (((DBDATE *)pConsumerData)->day==((DBDATE *)pBackEndData)->day))
				fSame=TRUE;
			break;

		case DBTYPE_DBTIME:
			if ((((DBTIME *)pConsumerData)->hour==((DBTIME *)pBackEndData)->hour) &&
			    (((DBTIME *)pConsumerData)->minute==((DBTIME *)pBackEndData)->minute) &&
                (((DBTIME *)pConsumerData)->second==((DBTIME *)pBackEndData)->second))
				fSame=TRUE;
			break;

		case DBTYPE_DBTIMESTAMP:
		{
			DBTIMESTAMP* pTimeStamp1 = (DBTIMESTAMP*)pConsumerData;
			DBTIMESTAMP* pTimeStamp2 = (DBTIMESTAMP*)pBackEndData;
			if (pTimeStamp1->year		==	pTimeStamp2->year	 &&
			    pTimeStamp1->month		==	pTimeStamp2->month	 &&
                pTimeStamp1->day		==	pTimeStamp2->day	 &&
			    pTimeStamp1->hour		==	pTimeStamp2->hour	 &&
			    pTimeStamp1->minute		==	pTimeStamp2->minute	 &&
                pTimeStamp1->second		==	pTimeStamp2->second	 &&
			    pTimeStamp1->fraction	==	pTimeStamp2->fraction)
				fSame=TRUE;
			break;
		}

		default:	 
			fSame=FALSE;
	}

	return fSame;
}

//------------------------------------------------------------------------------------
// @func: Compare two variants.  Does not attempt to free memory
//
// @rdesc Comparision results
//  @flag TRUE | Data in the two buffers is the same.
//  @flag FALSE | Data in the two buffers is not the same..
//		
//-------------------------------------------------------------------------------------
BOOL CompareVariant
(
	VARIANT *pVariantOrg,	//@parm [in]: Pointer to the variant in the consumer's buffer.
	VARIANT *pVariantCpy,	//@parm [in]: Pointer to the variant at the backend.
	BOOL	fCaseSensitive,
	BOOL	fApproxTypes
)
{
	HRESULT hr;
	VARIANT vOrg;
	BOOL fResult = FALSE;

	//Handle NULL cases...
	if(pVariantOrg==NULL || pVariantCpy==NULL)
	{
		if(pVariantOrg == pVariantCpy)
			return TRUE;
		return FALSE;
	}

	// Make a copy of the original variant as we may change it's type
	VariantInit(&vOrg);
	VariantCopy(&vOrg, pVariantOrg);

	if (V_VT(&vOrg) != V_VT(pVariantCpy))
	{
		//NOTE: Converting from or to Empty is almost always successful, but not something we want
		//to allow to equate equal.  Otherwise a provider could return empty and pass data comparision.
		//If for some reason empty is allowed it needs to be special cased in the calle (if this function return FALSe)
		if( V_VT(&vOrg) != VT_NULL && V_VT(&vOrg) != VT_EMPTY && V_VT(pVariantCpy) != VT_EMPTY)
		{
			// The variant should be the same type
			if(fApproxTypes)
			{
				// If the variant types weren't the same don't bother to compare further
				// We only check certain types for a match
				if(IsStringType(V_VT(&vOrg)) && !IsStringType(V_VT(pVariantCpy)))
					goto CLEANUP;
				if(IsDateTimeType(V_VT(&vOrg)) && !IsDateTimeType(V_VT(pVariantCpy)))
					goto CLEANUP;
				if(IsNumType(V_VT(&vOrg)) && !IsNumType(V_VT(pVariantCpy)))
					goto CLEANUP;
			}

			//Allow if the variant types are compatible
			hr = VariantChangeTypeEx(&vOrg, &vOrg, LOCALE_ENGLISH_US, 0, V_VT(pVariantCpy));
			if( FAILED(hr) )
				goto CLEANUP;
		}
		else
		{
			goto CLEANUP;
		}
	}
	
	// Return FALSE if vt is ORed with VT_RESERVED
	if (V_VT(&vOrg) & VT_RESERVED)
		goto CLEANUP;

	// Return TRUE is the vt is VT_EMPTY or VT_NULL
	if (V_VT(&vOrg)==VT_EMPTY || V_VT(&vOrg)==VT_NULL)
		fResult = TRUE;
	// Recursive call if the variant contains a variant
	else if (vOrg.vt == (VT_VARIANT | VT_BYREF))
		fResult = CompareVariant(V_VARIANTREF(&vOrg), V_VARIANTREF(pVariantCpy));
		
	// Check if the variant is a safearray, no need to verify the corresponding DBTYPE.
	// do not free memory
	else if (V_VT(&vOrg) & VT_ARRAY)
		//return CompareSafeArray(V_ARRAY(&vOrg), V_ARRAY(pVariantCpy), INVALID_DBTYPE, NULL,FALSE);
		fResult = CompareSafeArray(V_ARRAY(&vOrg), V_ARRAY(pVariantCpy), V_VT(&vOrg), NULL,FALSE);
	else
	{
		switch(V_VT(&vOrg))
		{	
			case VT_UI1:
				fResult = V_UI1(&vOrg) == V_UI1(pVariantCpy);
				break;
			
			case VT_I2:
				fResult = V_I2(&vOrg) == V_I2(pVariantCpy);
				break;

			case VT_I4:
				fResult = V_I4(&vOrg) == V_I4(pVariantCpy);
				break;

			case VT_UI2:
				fResult = V_UI2(&vOrg) == V_UI2(pVariantCpy);
				break;

			case VT_UI4:
				fResult = V_UI4(&vOrg) == V_UI4(pVariantCpy);
				break;

			case VT_I8:
				fResult = V_I8(&vOrg) == V_I8(pVariantCpy);
				break;

			case VT_R4:
				fResult = V_R4(&vOrg) == V_R4(pVariantCpy);
				break;

			case VT_DATE:	
				fResult = fabs(V_DATE(pVariantCpy) - V_DATE(&vOrg)) <= (1.000001/(60*60*24));
				break;

			case VT_R8:
 				fResult = fabs(V_R8(pVariantCpy) - V_R8(&vOrg)) <= fabs(g_dDoubleTolerance * V_R8(&vOrg));
				break;

			case VT_BOOL:
				fResult = V_BOOL(&vOrg) == V_BOOL(pVariantCpy);
				break;

			case VT_ERROR:
				fResult = V_ERROR(&vOrg) == V_ERROR(pVariantCpy);
				break;

			case VT_CY:
				fResult = memcmp(&V_CY(&vOrg), &V_CY(pVariantCpy),sizeof(CY))==0;
				break;

			case VT_BSTR:
				if(fCaseSensitive)
					fResult = wcscmp(V_BSTR(&vOrg), V_BSTR(pVariantCpy))==0;
				else
					fResult = _wcsicmp(V_BSTR(&vOrg), V_BSTR(pVariantCpy))==0;
				break;

			case VT_DECIMAL:
				fResult = CompareDecimal(&V_DECIMAL(&vOrg), &V_DECIMAL(pVariantCpy));
				break;

			// As we are not testing OLE object, return FALSE for VT_UNKNOWN
			case VT_UNKNOWN:
				fResult = (V_UNKNOWN(&vOrg) == V_UNKNOWN(pVariantCpy));
				break;

			// As we are not testing OLE object, return FALSE for VT_DISPATCH
			case VT_DISPATCH:
				fResult = (V_DISPATCH(&vOrg) == V_DISPATCH(pVariantCpy));
				break;

			case VT_I1:
				fResult = V_I1(&vOrg) == V_I1(pVariantCpy);
				break;

			case VT_I2 | VT_BYREF:
				fResult = *V_I2REF(&vOrg) == *V_I2REF(pVariantCpy);
				break;

			case VT_I4 | VT_BYREF:
				fResult = *V_I4REF(&vOrg) == *V_I4REF(pVariantCpy);
				break;

			case VT_UI2 | VT_BYREF:
				fResult = *V_UI2REF(&vOrg) == *V_UI2REF(pVariantCpy);
				break;

			case VT_UI4 | VT_BYREF:
				fResult = *V_UI4REF(&vOrg) == *V_UI4REF(pVariantCpy);
				break;

			case VT_R4 | VT_BYREF:
				fResult = *V_R4REF(&vOrg) == *V_R4REF(pVariantCpy);
				break;

			case VT_R8 | VT_BYREF:
				fResult = *V_R8REF(&vOrg) == *V_R8REF(pVariantCpy);
				break;

			case VT_BOOL | VT_BYREF:
				fResult = *V_BOOLREF(&vOrg) == *V_BOOLREF(pVariantCpy);
				break;

			case VT_ERROR | VT_BYREF:
				fResult = *V_ERRORREF(&vOrg) == *V_ERRORREF(pVariantCpy);
				break;

			case VT_CY | VT_BYREF:
				fResult = memcmp(V_CYREF(&vOrg), V_CYREF(pVariantCpy),8)==0;
				break;

			case VT_DATE | VT_BYREF:
				fResult = *V_DATEREF(&vOrg) == *V_DATEREF(pVariantCpy);
				break;

			case VT_BSTR | VT_BYREF:
				if(fCaseSensitive)
					fResult = wcscmp(*V_BSTRREF(&vOrg), *V_BSTRREF(pVariantCpy))==0;
				else
					fResult = _wcsicmp(*V_BSTRREF(&vOrg), *V_BSTRREF(pVariantCpy))==0;
				break;

			case VT_DECIMAL | VT_BYREF:
				fResult = memcmp(V_DECIMALREF(&vOrg), V_DECIMALREF(pVariantCpy),sizeof(DECIMAL))==0;
				break;

			// As we are not testing OLE object, return FALSE for VT_UNKNOWN
			case VT_UNKNOWN | VT_BYREF:
				fResult = FALSE;
				break;

			// As we are not testing OLE object, return FALSE for VT_DISPATCH
			case VT_DISPATCH | VT_BYREF:
				fResult = FALSE;
				break;

			case VT_I1 | VT_BYREF:
				fResult = *V_I1REF(&vOrg) == *V_I1REF(pVariantCpy);
				break;

			default: 
				ASSERT(!L"Unhandled Type!");
				fResult = FALSE;
		}
	}

CLEANUP:

	VariantClear(&vOrg);

	return fResult;
}

//---------------------------------------------------------------------------
// @func BOOL|
// Checks if the type of variant is a string type, date type,
// fixed length num type or varianble length num type.
//---------------------------------------------------------------------------
BOOL IsStringType(DBTYPE	dbtype)
{
	if(dbtype == DBTYPE_BSTR ||
		dbtype == DBTYPE_STR ||
		dbtype == DBTYPE_WSTR)
		return TRUE;
	else
		return FALSE;
}

BOOL IsDateTimeType(DBTYPE	dbtype)
{
	if(dbtype == DBTYPE_DATE ||
		dbtype == DBTYPE_DBDATE ||
		dbtype == DBTYPE_DBTIME ||
		dbtype == DBTYPE_DBTIMESTAMP ||
		dbtype == DBTYPE_FILETIME)
		return TRUE;
	else
		return FALSE;
}

BOOL IsNumType(DBTYPE	dbtype)
{
	if(dbtype == DBTYPE_I1 ||
		dbtype == DBTYPE_I2 ||
		dbtype == DBTYPE_I4 ||
		dbtype == DBTYPE_I8 ||
		dbtype == DBTYPE_UI1 ||
		dbtype == DBTYPE_UI2 ||
		dbtype == DBTYPE_UI4 ||
		dbtype == DBTYPE_UI8 ||
		dbtype == DBTYPE_R4 ||
		dbtype == DBTYPE_R8 ||
		dbtype == DBTYPE_CY ||
//		dbtype == DBTYPE_BOOL ||
		dbtype == DBTYPE_NUMERIC ||
		dbtype == DBTYPE_DECIMAL ||
		dbtype == DBTYPE_VARNUMERIC)
		return TRUE;
	else
		return FALSE;
}


//---------------------------------------------------------------------------
// CreateUniqueDBID
//
//-------------------------------------------------------------------------------------
HRESULT CreateUniqueDBID
(
	DBID*		pDBID,
	BOOL		fInitialize
)
{
	if(!pDBID)
		return E_INVALIDARG;
	HRESULT hr = S_OK;
	DBKIND eKind = pDBID->eKind; //Save before memseting...
	GUID guid;

	//NOTE: The DBID passed in is assumed to be at least Initialized, with a valid eKind.
	//unless you let us know otherwise...
	//(This is mainly called with an existing columnid, that you want to change to be unique...)
	if(fInitialize)
		memset(pDBID, 0, sizeof(DBID));

	//Try to create a new "unique" name for the columnid, so we don't have clashes...
	pDBID->eKind = eKind;
	TESTC_(hr = CoCreateGuid(&guid),S_OK);
	
	switch(pDBID->eKind)
	{
		case DBKIND_GUID_NAME:
		case DBKIND_GUID_PROPID:
		case DBKIND_GUID:
			//Create a new guid (fairly unique!)
			pDBID->uGuid.guid = guid;
			break;

		case DBKIND_PGUID_PROPID:
		case DBKIND_PGUID_NAME:
			//Create a new guid (fairly unique!)
			SAFE_FREE(pDBID->uGuid.pguid);
			SAFE_ALLOC(pDBID->uGuid.pguid, GUID, 1);
			*pDBID->uGuid.pguid = guid;
			break;
			
		case DBKIND_PROPID:
			pDBID->uName.ulPropid = guid.Data1;
			break;

		case DBKIND_NAME:
		{
			SAFE_FREE(pDBID->uName.pwszName);
			TESTC_(hr = StringFromCLSID(guid, &pDBID->uName.pwszName),S_OK);
			break;
		}

		default:
			ASSERT(!"Unknown DBID type?");
			hr = E_INVALIDARG;
			break;
	};

CLEANUP:
	return hr;
}


//---------------------------------------------------------------------------
// CompareDBID
//
// @func BOOL CompareDBID
// Compares DBIDs. No memory for client to release.
//
// @rdesc Comparision results
//  @flag TRUE | Data in the two buffers is the same.
//  @flag FALSE | Data in the two buffers is not the same..
//		
//-------------------------------------------------------------------------------------
BOOL CompareDBID(
	const DBID&	x,						// @parm [IN] First DBCOLID 
	const DBID&	y,						// @parm [IN] Second DBCOLID
	IUnknown	*pIUnknown/*=NULL*/		//@parm [IN] DSO interface 
)
{
	BOOL	fRes;
	if (x.eKind == y.eKind)
	{
		switch(x.eKind)
		{
			case DBKIND_GUID_NAME:
				if ((x.uGuid.guid==y.uGuid.guid) &&
				   x.uName.pwszName==NULL && 
				   y.uName.pwszName==NULL)
				   return TRUE;

				if (x.uGuid.guid!=y.uGuid.guid)
					return FALSE;

				TESTC_(CompareID(&fRes, (x.uName.pwszName), 
					(y.uName.pwszName), pIUnknown), S_OK);
				return fRes;

			case DBKIND_GUID_PROPID:
				if ((x.uGuid.guid == y.uGuid.guid)&&
					(x.uName.ulPropid == y.uName.ulPropid))
					return TRUE;
				break;

			case DBKIND_NAME:
				if (NULL == x.uName.pwszName || NULL == y.uName.pwszName)
					return x.uName.pwszName == y.uName.pwszName;
			
				TESTC_(CompareID(&fRes, (x.uName.pwszName), 
					(y.uName.pwszName), pIUnknown), S_OK);
				return fRes;

			case DBKIND_PGUID_NAME:
				if ((x.uGuid.pguid!=NULL) && (y.uGuid.pguid!=NULL))
				{
					if (*x.uGuid.pguid != *y.uGuid.pguid)
						return FALSE;
				}
				else if ((x.uGuid.pguid!=NULL) || (y.uGuid.pguid!=NULL))
					return FALSE;

				TESTC_(CompareID(&fRes, (x.uName.pwszName), 
					(y.uName.pwszName), pIUnknown), S_OK);
				return fRes;

			case DBKIND_PGUID_PROPID:
				if ((x.uGuid.pguid!=NULL) && (y.uGuid.pguid!=NULL))
				{
					if (*x.uGuid.pguid != *y.uGuid.pguid)
						return FALSE;
				}
				else if ((x.uGuid.pguid!=NULL) || (y.uGuid.pguid!=NULL))
					return FALSE;

				if (x.uName.ulPropid == y.uName.ulPropid)
					return TRUE;
				break;

			case DBKIND_PROPID:
				if (x.uName.ulPropid == y.uName.ulPropid)
					return TRUE;
				break;

			case DBKIND_GUID:
				if (x.uGuid.guid==y.uGuid.guid) 
					return TRUE;
				break;

			default:
				PRVTRACE(L"MiscFunc.cpp, CompareDBID: eKind not recognized.\n");
				break;
		}
	}

CLEANUP:
	return FALSE;
}

//---------------------------------------------------------------------------
// DuplicateDBID
//
// @func	HRESULT	DuplicateDBID
// Copies y into x. If a problem is encountered, x = GUID_NULL. Client provides an empty DBID*
// (whatever is inside doesn't have significance - will not be released...)
// Client is responsible for releasing memory for x.
//
// This version should actually duplicate the DBID, i.e. copy eKind, building a similar uName.pwszName, etc
//
// @rdesc Comparision results
//  @flag NOERROR | Data in the two buffers is the same.
//  @flag E_FAIL  | Data in the two buffers is not the same..
//		
//-------------------------------------------------------------------------------------
HRESULT DuplicateDBID(
	const DBID& y,			// @parm || [IN] Orginal
	DBID * px		// @parm || [OUT] Copy of y
)
{
	if(!px)
		return E_FAIL;

	//Initialize the output DBID. Then manually copy the 
	//eKind, guid and propid.
	memset(px, 0, sizeof(DBID));
	px->eKind = y.eKind;
	if(y.eKind == DBKIND_GUID ||
		y.eKind == DBKIND_GUID_PROPID ||
		y.eKind == DBKIND_GUID_NAME)
		px->uGuid.guid = y.uGuid.guid;

	if(y.eKind == DBKIND_PGUID_PROPID ||
		y.eKind == DBKIND_PROPID ||
		y.eKind == DBKIND_GUID_PROPID)
		px->uName.ulPropid = y.uName.ulPropid;

	switch(px->eKind)
	{
		case DBKIND_PGUID_NAME:
			px->uGuid.pguid = NULL;
			if(y.uGuid.pguid)
			{
				SAFE_ALLOC(px->uGuid.pguid, GUID, 1);
				*(px->uGuid.pguid) = *(y.uGuid.pguid);
			}
		case DBKIND_NAME:
		case DBKIND_GUID_NAME:
			px->uName.pwszName = wcsDuplicate(y.uName.pwszName);
			return NOERROR;

		case DBKIND_PGUID_PROPID:  
			px->uGuid.pguid = NULL;
			if(y.uGuid.pguid)
			{
				SAFE_ALLOC(px->uGuid.pguid, GUID, 1);
				*(px->uGuid.pguid) = *(y.uGuid.pguid);
			}
		case DBKIND_PROPID:
		case DBKIND_GUID_PROPID:
		case DBKIND_GUID:
			return NOERROR;

		default:
			return E_FAIL;
	}
	
CLEANUP:
	return E_FAIL;
}


//---------------------------------------------------------------------------
// ReleaseDBID
//
// @func	HRESULT	ReleaseDBID
// Releases all the data referred inside the DBID, according to its eKind
// @rdesc Comparision results
//  @flag NOERROR | Data in the two buffers is the same.
//  @flag E_FAIL  | Data in the two buffers is not the same..
//		
//-------------------------------------------------------------------------------------
void ReleaseDBID(
	DBID*	pDBID,			// @parm || [IN] DBID to be released
	BOOL	fDrop/*=TRUE*/	// whether to free the pointer or just its content
)
{
	// If NULL return failure
	if (NULL == pDBID)
		return;

	switch (pDBID->eKind)
	{
		case DBKIND_PGUID_NAME:
			 PROVIDER_FREE(pDBID->uGuid.pguid);
		case DBKIND_GUID_NAME:
		case DBKIND_NAME: 
			PROVIDER_FREE(pDBID->uName.pwszName);
			break;
		case DBKIND_PGUID_PROPID:
			PROVIDER_FREE(pDBID->uGuid.pguid);
			break;
	}
	
	// Set everything to 0's
	memset(pDBID, 0, sizeof(DBID));

	// Free the memory for the DBID
	if (fDrop)
		PROVIDER_FREE(pDBID);

} //ReleaseDBID



HRESULT BuildDBID(
	DBID	*pDBID, 
	WCHAR	*pwszName, 
	GUID	*pguid, 
	ULONG	*pulPropID
)
{
	if (!pDBID)
		return E_INVALIDARG;

	memset(pDBID, 0, sizeof(DBID));

	if (pguid)
	{
		if (pwszName)
		{
			pDBID->eKind = DBKIND_GUID_NAME;
			pDBID->uGuid.guid = *pguid;
			pDBID->uName.pwszName = pwszName;
		}
		else if (pulPropID)
		{
			pDBID->eKind = DBKIND_GUID_PROPID;
			pDBID->uGuid.guid = *pguid;
			pDBID->uName.ulPropid = *pulPropID;
		}
		else
		{
			pDBID->eKind = DBKIND_GUID;
			pDBID->uGuid.guid = *pguid;
		}
	}
	else if (pwszName)
	{
		pDBID->eKind = DBKIND_NAME;
		pDBID->uName.pwszName = pwszName;
	}
	else if (pulPropID)
	{
		pDBID->eKind = DBKIND_PROPID;
		pDBID->uName.ulPropid = *pulPropID;
	}
	else
		return E_INVALIDARG;

	return S_OK;
} //BuildDBID



//-----------------------------------------------------------------------------
//	@func: Return the length of a DBTYPE.  It returns 0 for DBTYPE_EMPTY, 
//	DBTYPE_NULL, and any variable length data types.  It returns INVALID_DBTYPE_SIZE 
//	for invalid input.  If the data is ORed with DBTYPE_BYREF, the bit flag is ignored
//
//-----------------------------------------------------------------------------
LONG GetDBTypeSize(
	DBTYPE dwDBType		//@parm [in]: The DBType to compute 
)
{
	// Unset the flag DBTYPE_BYREF
	dwDBType = (DBTYPE)(dwDBType & (~DBTYPE_BYREF));
	
	switch(dwDBType)
	{
		case DBTYPE_EMPTY:
		case DBTYPE_NULL:
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		case DBTYPE_BYTES:
		case DBTYPE_VARNUMERIC:
			return 0;

		case DBTYPE_I1:
		case DBTYPE_UI1:
			return 1;

		case DBTYPE_I2:
		case DBTYPE_UI2:
			return 2;

		case DBTYPE_I4:
		case DBTYPE_UI4:
			return 4;

		case DBTYPE_I8:
		case DBTYPE_UI8:
			return 8;

		case DBTYPE_R4:
			return sizeof(float);

		case DBTYPE_R8:
			return sizeof(double);

		case DBTYPE_CY:
			return 8;

		case DBTYPE_NUMERIC:
			return sizeof(DB_NUMERIC);

		case DBTYPE_DATE:
			return sizeof(double);

		case DBTYPE_BOOL:
			return 2;

		case DBTYPE_BSTR:
			return sizeof(BSTR);

		case DBTYPE_VARIANT:
			return sizeof(VARIANT);

		case DBTYPE_PROPVARIANT:
			return sizeof(PROPVARIANT);

		case DBTYPE_IDISPATCH:
			return sizeof(IDispatch*);

		case DBTYPE_IUNKNOWN:
			return sizeof(IUnknown*);

		case DBTYPE_GUID:
			return sizeof(GUID);

		case DBTYPE_ERROR:
			return sizeof(SCODE);

		case DBTYPE_DBDATE:
			return sizeof(DBDATE);

		case DBTYPE_DBTIME:
			return sizeof(DBTIME);

		case DBTYPE_DBTIMESTAMP:
			return sizeof(DBTIMESTAMP);

		case DBTYPE_DECIMAL:
			return sizeof(DECIMAL);
	
		case DBTYPE_FILETIME:
			return sizeof(FILETIME);

		case DBTYPE_HCHAPTER:
			return sizeof(ULONG);

		default:
		{
			if(dwDBType & DBTYPE_ARRAY)
			{
				return sizeof(SAFEARRAY*);
			}
			
			if(dwDBType & DBTYPE_VECTOR)
			{
				return sizeof(DBVECTOR);
			}
			break;
		}
	};

	//Otherwise
	return INVALID_DBTYPE_SIZE;
}


//-------------------------------------------------------------------------------------
// @func: Return the equivalent DBTYPE for a valid base VARTYPE of a SafeArray.
// The base type of a safearry is restricted to a subset of the variant type.  
// Neither the VT_RESERVED and VT_BYREF flags can be set.  
// VT_EMPTY and VT_NULL are not valid.
//
//------------------------------------------------------------------------------------
DBTYPE	VARTYPE2DBTYPE(
	VARTYPE	vt	//@parm [in]: The vt tag of the safearray
)
{
	//Unset the VT_ARRAY bit
	vt &= ~VT_ARRAY;

	// Check if vt is ORed with VT_RESERVED
	if (vt & VT_RESERVED)
		return INVALID_DBTYPE;

	// Check if vt is ORed with VT_BYREF
	if (vt & VT_BYREF)
		return INVALID_DBTYPE;

	switch(vt)
	{	
		case VT_EMPTY:
		case VT_NULL:
			return INVALID_DBTYPE;

		case VT_I2:
			return DBTYPE_I2;

		case VT_I4:
			return DBTYPE_I4;

		case VT_R4:
			return DBTYPE_R4;

		case VT_R8:
			return DBTYPE_R8;

		case VT_BOOL:
			return DBTYPE_BOOL;

		case VT_ERROR:
			return DBTYPE_ERROR;

		case VT_CY:
			return DBTYPE_CY;

		case VT_DATE:
			return DBTYPE_DATE;

		case VT_BSTR:
			return DBTYPE_BSTR;

		case VT_UNKNOWN:
			return DBTYPE_IUNKNOWN;

		case VT_DISPATCH:
			return DBTYPE_IDISPATCH;

		default:  
			return INVALID_DBTYPE;
	}
}

//--------------------------------------------------------------------
// @func CleanUpVector: Clean up any memory allocated for the vector.
//
// @rdesc Void
//
//-------------------------------------------------------------------------------------
void CleanUpVector(
	DBVECTOR	*pConsumerData, //@parm [in] the pointer to the vector
	DBTYPE		wType			//@parm [in] the wType of the element in the vector It can only ORed with DBTYPE_VECTOR
)
{
	DBLENGTH	uSize;
	void	*pData;
	DBLENGTH	cCount;
	
	// Unset DBTYPE_VECTOR bit
	wType=wType & (~DBTYPE_VECTOR);

	// Nothing to do if the wType != DBTYPE_BSTR and wType != DBTYPE_VARIANT
	if (wType != DBTYPE_BSTR && wType != DBTYPE_VARIANT)
		return;

	if (wType == DBTYPE_BSTR)
		uSize=sizeof(BSTR);
	else
		uSize=sizeof(VARIANT);

	pData=(void *)pConsumerData;

	for(cCount=1;cCount<=pConsumerData->size;cCount++)
	{
		if (wType==DBTYPE_BSTR)
			SysFreeString(*(BSTR*)pData);
		else
			GCHECK(VariantClear((VARIANT*)pData),S_OK);

		// Move pData to the next element
		pData=(void *)((BYTE *)pData+uSize);
	}

	return;
}

//----------------------------------------------------------------------
// @mfunc	Counts rows on rowset
//
// @rdesc	Count of rows	
//
//----------------------------------------------------------------------
HRESULT CountRowsOnRowset(
	IRowset		*pIRowset,		// @parm [IN] rowset to count rows
	DBCOUNTITEM		*pcRows			// @parm [IN/OUT] # of rows in rowset
)
{
	HRESULT 		hr=0;				// Return result
	BOOL			fResult=FALSE;		// Return status
	DBCOUNTITEM		cRowsObtained=0;	// Count of rows
	HROW			*rghRows=NULL;		// Range of row handles

	ASSERT(pIRowset);
	ASSERT(pcRows);

	//Initialize count of rows to zero
	*pcRows = 0;

	//Count the rows by getting one row at a time
	while(SUCCEEDED(hr=pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &rghRows)) && cRowsObtained !=0)
	{
		(*pcRows)++;
		
		if (FAILED(hr=pIRowset->ReleaseRows(cRowsObtained,rghRows,NULL,NULL,NULL)))
			goto CLEANUP;
	}
	
	//Restart the cursor position
	if(FAILED(hr=pIRowset->RestartPosition(NULL)))
		goto CLEANUP;

	fResult = TRUE;

CLEANUP:
	
	//SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(rghRows);

	if(fResult==FALSE)
	{
		PRVTRACE("%sCountRowsOnTable):%s",wszPRIVLIBT,wszFUNCFAIL);
		return hr;
	}

	return NOERROR;
}


//---------------------------------------------------------------------------
//	DBTYPE2VARIANT
//
// @mfunc Converts a DBTYPE to VARIANT
//
// @rdesc Pointer to converted data, of type dbDestType.  Caller
//		  must free this memory using IMalloc::Free.	
//
//---------------------------------------------------------------------------
VARIANT* DBTYPE2VARIANT(
	void*		pvSource,	//@parm [IN] Value to be converted, must NOT be null
	DBTYPE		wType		//@parm [IN] DBTYPE of Value
)
{
	//pvSource can only be NULL if creating a VARIANT of type VT_NULL or VT_EMPTY
	if(pvSource==NULL)
	{
		ASSERT(wType==VT_NULL || wType==VT_EMPTY);
	}
	
	//Create a new VARIANT, (needs to be dynamic to pass back to user...)
	VARIANT* pVariant = (VARIANT*)PROVIDER_ALLOC(sizeof(VARIANT));
	VariantInit(pVariant);

	//Setup vt and value
	V_VT(pVariant) = wType;
	
	//Deal with Modifiers
	if(wType & DBTYPE_ARRAY)
	{
		V_ARRAY(pVariant) = *(SAFEARRAY**)pvSource;
	}
	else
	{
		//We don't have Modifiers
		switch(wType)
		{
			case VT_NULL:
			case VT_EMPTY:
				break;

			case VT_I1:					//		CHAR           
				V_I1(pVariant) = *(CHAR*)pvSource;
				break;
				
			case VT_UI2:				//		USHORT         
				V_UI2(pVariant) = *(USHORT*)pvSource;
				break;
						 				
			case VT_BOOL:
				V_BOOL(pVariant) = *(VARIANT_BOOL*)pvSource;
				break;

			case DBTYPE_WSTR:
				V_VT(pVariant) = VT_BSTR;
				V_BSTR(pVariant) = SysAllocString((LPWSTR)pvSource);
				break;
				
			case VT_BSTR:
				V_BSTR(pVariant) = SysAllocString(*(BSTR*)pvSource);
				break;
				
			case VT_I2:
				V_I2(pVariant) = *(SHORT *)pvSource;
				break;

			case VT_I8:               //      BIGINT
				V_I8(pVariant) = *(LONGLONG *)pvSource;
            break;
				
			case VT_ERROR:				//		SCODE          
			case VT_INT:				//		INT  
			case VT_I4:					//      LONG
				V_I4(pVariant) = *(LONG *)pvSource;
				break;

			case VT_UINT:				//		UINT  
			case VT_UI4:				//		ULONG          
				V_I4(pVariant) = *(ULONG *)pvSource;
				break;
				
			case VT_UI1:
				V_UI1(pVariant) = *(BYTE *)pvSource;
				break;

			case VT_DATE:
			case VT_R8:
				V_R8(pVariant) = *(double *)pvSource;
				break;

			case VT_R4:
				V_R4(pVariant) = *(float *)pvSource;
				break;
				
			case VT_CY:
				V_CY(pVariant) = *(CY *)pvSource;
				break;

			//DECIMAL is the only type that is not part of the VARIANT union
			case VT_DECIMAL:			
				V_DECIMAL(pVariant) = *(DECIMAL*)pvSource;
				break;

			default:
				ASSERT(!L"Unhandled Type!");
				break;
		};
	}

	return pVariant;
}

//---------------------------------------------------------------------------
//	CriticalSection
//
//  a wrap class for a critical section.
//  All objects of this class share a common real Critical Section Object (see static method CriticalSectionObject())
//  Caller should use this class via creating local object of this class,  
//  the default constructor of this class calls EnterCriticalSection and 
//	destructor - LeaveCriticalSection.
//
//---------------------------------------------------------------------------
class MFCriticalSection
{
public:
	MFCriticalSection()
	{
		EnterCriticalSection(&CriticalSectionObject());
	}

	~MFCriticalSection()
	{
		LeaveCriticalSection(&CriticalSectionObject());
	}

	
protected:
	class _CriticalSection
	{
	public:
		_CriticalSection(){	InitializeCriticalSection(&m_criticalSection); }
		~_CriticalSection()	{ DeleteCriticalSection(&m_criticalSection); }
		CRITICAL_SECTION m_criticalSection;
	};

	static CRITICAL_SECTION &CriticalSectionObject()
	{
		static _CriticalSection tmp;
		return tmp.m_criticalSection;
	}
};


//---------------------------------------------------------------------------
//	WSTR2DBTYPE
//
// @mfunc Converts a DBTYPE_WSTR to any other DBTYPE
// Returns count of returned bytes as a USHORT		
//
// @rdesc Pointer to converted data, of type dbDestType.  Caller
//		  must free this memory using IMalloc::Free.	
//
//---------------------------------------------------------------------------
void * WSTR2DBTYPE(
	WCHAR *		pwszSource,	//@parm [IN] String to be converted, must NOT be null
	DBTYPE		dbDestType,	//@parm [IN] DBTYPE to convert to
	USHORT *	pcb,		//@parm [OUT] Pointer to USHORT to store count of bytes.
	BOOL		fLaxConvert	//@param [IN] optional Whether to use strict error checking. DEFAULT = FALSE
)
{
	ULONG_PTR	cbBytes = 0;
	void *		pData = NULL;

	// Newers test want to use data > 65535 bytes.
	// Hence, the new WSTR2DBTYPE uses a ULONG for its count of returned bytes
	// parameter.
	// This function is here to allow so older tests do no break
	pData = WSTR2DBTYPE_EX(pwszSource, dbDestType, &cbBytes, fLaxConvert);

	if( cbBytes > USHRT_MAX)
	{
		ASSERT(!"Output buffer is greater USHRT_MAX, Use WSTR2DBTYPE_EX...");
	}

	if( pcb )
		*pcb = USHORT(cbBytes);

	return pData;	
}



//---------------------------------------------------------------------------
//	WSTR2DBTYPE_EX
//
// @mfunc Converts a DBTYPE_WSTR to any other DBTYPE
// Returns count of returned bytes as a ULONG
//
// @rdesc Pointer to converted data, of type dbDestType.  Caller
//		  must free this memory using IMalloc::Free.	
//
//---------------------------------------------------------------------------
void * WSTR2DBTYPE_EX(
	WCHAR *			pwszSource,	//@parm [IN] String to be converted, must NOT be null
	DBTYPE			dbDestType,	//@parm [IN] DBTYPE to convert to
	ULONG_PTR *		pcb,		//@parm [OUT] Pointer to ULONG to store count of bytes.
	BOOL			fLaxConvert	//@param [IN] optional Whether to use strict error checking. DEFAULT = FALSE
)
{
	BYTE  *		pDestBuf = NULL;	//Pointer we'll eventually return
	WCHAR *		pwszStop=NULL;		// Pointer to stop char for char -> int conversions
	LPSTR		pszLocaleInfo = NULL; // Pointer to locale info

	ASSERT(pwszSource);
	ULONG_PTR cBytes = (ULONG_PTR)GetDBTypeSize(dbDestType);

	/*
	This function requires the 'C' locale to function properly.  We create string data
	for each data type using US settings (i.e. use a '.' for decimal seperator, etc). 
	This is essentially equivalent to the 'C' locale setting. Since LTM and/or TableDump
	may change the locale, then we will break this function.

	For example, on a German machine, TableDump calls SetLocale(LC_ALL, ".ACP"), which
	breaks conversions from string to double (R8/R4) since wcstod then requires a ','
	for decimal separator.  But if TableDump does not call SetLocale, then on a Japanese
	machine we are unable to create strings for SQL statements properly.  So here
	we will cache the current locale settings, change to 'C' locale, then restore...
	*/
	{
		// we can change locale in the several places and
		// the pointer returned by the first call of setlocal can become invalid,
		// so set critical section
		MFCriticalSection lock;

		pszLocaleInfo=_strdup(setlocale(LC_ALL, NULL));
		setlocale(LC_ALL, "C");
	}

	// Ignore BYREF, since we just make the data, other routines
	// should figure out that they need to first dereference
	// the consumer's data pointer before comparing it with this data
	dbDestType &= ~DBTYPE_BYREF;

	switch(dbDestType)
	{
		case DBTYPE_NULL:
		case DBTYPE_EMPTY:
			pDestBuf = NULL;
			break;

		case DBTYPE_WSTR:
			pDestBuf = (BYTE*)wcsDuplicate(pwszSource);
			cBytes = wcslen(pwszSource)*sizeof(WCHAR);
			break;
		
		case DBTYPE_BSTR:
			// Alloc sizeof(BSTR) since a pointer to the data is returned
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(BSTR));
			
			*(BSTR*)pDestBuf = SysAllocString(pwszSource);
			cBytes = sizeof(BSTR);
			break;
		
		case DBTYPE_GUID: 
		{
			// GUID should be in CLSID string notation e.g. "{FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF}"

			// Add 1 for Null terminator
			WCHAR* pwszTempSrcBuf = wcsDuplicate(pwszSource);
			if (!pwszTempSrcBuf)
				break;
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(GUID));
			if (pDestBuf)
				CLSIDFromString(pwszTempSrcBuf, (CLSID*)pDestBuf);
			PROVIDER_FREE(pwszTempSrcBuf);
			break;
		}		

		// For backend types exposed as DBTYPE_IUnknown,
		// treat the type as a byte stream so that in the
		// case the the type supports a storage interface,
		// we can at least check the storage data.
		case DBTYPE_IUNKNOWN:
		case DBTYPE_BYTES:
		{	
			BYTE TempByte;						
			
			// Only need half a byte for each 2 byte wchar, so
			// allocate one half of the length of the wide string
			pDestBuf = (BYTE *)PROVIDER_ALLOC(wcslen(pwszSource)/2);			
			if (!pDestBuf)
				break;
			
			// Go thru two wchars at a time, finding their numeric equivelant
			// to represent in binary
			for(cBytes = 0; *pwszSource; cBytes++, pwszSource++) 
			{
				// Move first wchar's value to first four bits of destination byte
				if (*pwszSource <= '9')					
					TempByte = (BYTE)((*pwszSource - L'0') << 4);
				else				
					TempByte = (BYTE)((*pwszSource - L'A' + 10) << 4);

				// We must have an even number of bytes in our string
				ASSERT(*(pwszSource+1));
				
				// Put the second wchar's value in the last four bits of the destination byte
				if (*++pwszSource <= '9')
					TempByte |= (BYTE)(*pwszSource -  L'0');
				else
					TempByte |= (BYTE)(*pwszSource - L'A' + 10);

				// Put our constructed byte in the array of destination bytes
				pDestBuf[cBytes] = TempByte;
			}
			break;
		}
			
		case DBTYPE_BOOL:
		{			 				
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(VARIANT_BOOL));			
			if (!pDestBuf)
				break;
			
			// Convert the "1" or "0" to TRUE or FALSE		
			if (!wcscmp(pwszSource, L"1")		||
				!_wcsicmp(pwszSource, L"True"))
				*(VARIANT_BOOL *)pDestBuf = VARIANT_TRUE;
			else
				if (!wcscmp(pwszSource, L"0")		||
					!_wcsicmp(pwszSource, L"False"))
					*(VARIANT_BOOL *)pDestBuf = VARIANT_FALSE;
				else
					ASSERT(FALSE);
			break;
		}

		case DBTYPE_UI1:
		{			 		
			// Alloc sizeof(long) bytes since a long is returned
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(long));			
			long l;
			BYTE b;

			if (!pDestBuf)
				break;

			*(long *)pDestBuf =	wcstol(pwszSource, &pwszStop, 10);
			if (!(pwszStop && pwszStop[0] == L'\0'))
			{
				PROVIDER_FREE(pDestBuf);
				break;
			}

			// Check that the value fits in the range of UI1
			// before casting it to a UI1 (BYTE)
			ASSERT(*(long *)pDestBuf <= 255);		
			l = *(long *)pDestBuf;
			b = (BYTE)l;
			*(BYTE *)pDestBuf = b;
			break;
		}
		
		case DBTYPE_I1:
		{			 		
			// Alloc sizeof(long) bytes since a long is returned
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(long));			
			long l;
			BYTE b;

			if (!pDestBuf)
				break;

			*(long *)pDestBuf =	wcstol(pwszSource, &pwszStop, 10);
			if (!(pwszStop && pwszStop[0] == L'\0'))
			{
				PROVIDER_FREE(pDestBuf);
				break;
			}
		
			// Check that the value fits in the range of I1
			// before casting it to a I1 (BYTE)
			ASSERT((*(long *)pDestBuf <= 128) && (*(long *)pDestBuf) >= -128);		
			l = *(long *)pDestBuf;
			b = (signed char)l;
			*(signed char *)pDestBuf = b;
			break;
		}

		// Another way to store binary
		case DBTYPE_UI1 | DBTYPE_VECTOR:			
		{
			BYTE TempByte;						
			USHORT usIdx;
			BYTE * pByteBuf;
					 			
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(DBVECTOR));
			if (!pDestBuf)
				break;

			// Only need half a byte for each 2 byte wchar, so
			// allocate one half of the length of the wide string
			// add 1 if string has an odd length
			if (*(pwszSource+1))
				pByteBuf = (BYTE *)PROVIDER_ALLOC(wcslen(pwszSource)/2);			
			else
				pByteBuf = (BYTE *)PROVIDER_ALLOC((wcslen(pwszSource)/2)+1);			
			
			// Go thru two wchars at a time, finding their numeric equivelant
			// to represent in binary
			for(usIdx = 0; *pwszSource; usIdx++, pwszSource++) 
			{
				// Check 2nd WCHAR for NULL 
				if (*(pwszSource+1))
				{
					// Move first wchar's value to first four bits of destination byte
					if (*pwszSource <= '9')					
						TempByte = (BYTE)((*pwszSource++ - L'0') << 4);
					else				
						TempByte = (BYTE)((*pwszSource++ - L'A' + 10) << 4);
				}
				else
					TempByte = (BYTE)((L'0') << 4);

				// Put the second wchar's value in the last four bits of the destination byte
				if (*pwszSource <= '9')
					TempByte |= (BYTE)(*pwszSource -  L'0');
				else
					TempByte |= (BYTE)(*pwszSource - L'A' + 10);

				// Put our constructed byte in the array of destination bytes
				pByteBuf[usIdx] = TempByte;
			}
			
			((DBVECTOR *)pDestBuf)->size = usIdx;
			((DBVECTOR *)pDestBuf)->ptr = (void *)pByteBuf;		
			cBytes = sizeof(DBVECTOR);
			break;
		}
		
		// We could return several different DBDATATYPES here, 
		// we return ANSI CHARACTER STRING
		case DBTYPE_STR:
		{	
			int cb = 0;

			// First find out how big our ANSI buffer has to be and allocate it
			cb = WideCharToMultiByte(CP_ACP, 0, pwszSource, -1, NULL, 0, NULL, NULL);
			pDestBuf = (BYTE *)PROVIDER_ALLOC(cb);
			if (!pDestBuf)
				break;

			// Now convert string and return
			if (!WideCharToMultiByte(CP_ACP, 0, pwszSource, -1, (CHAR *)pDestBuf, cb, NULL, NULL))
			{
				// We failed, make sure we return NULL
				PROVIDER_FREE(pDestBuf);
				pDestBuf = NULL;
			}
			else
				cBytes = strlen((CHAR*)pDestBuf)*sizeof(CHAR);
			break;
		}		
		
		case DBTYPE_ERROR:
		case DBTYPE_I4:			
			// Use wtol instead of wtoi since long is guaranteed 
			// to be 4 bytes, but int is platform specific.
			pDestBuf = (BYTE *)PROVIDER_ALLOC(4);			
			if (!pDestBuf)
				break;

			*(long *)pDestBuf =	wcstol(pwszSource, &pwszStop, 10);
			if (!(pwszStop && pwszStop[0] == L'\0'))
			{
				PROVIDER_FREE(pDestBuf);
				break;
			}
			break;		 

		case DBTYPE_UI4:
		case DBTYPE_HCHAPTER:
			pDestBuf = (BYTE *)PROVIDER_ALLOC(4);					
			if (!pDestBuf)
				break;

			*(ULONG *)pDestBuf = wcstoul(pwszSource, &pwszStop, 10);
			if (!(pwszStop && pwszStop[0] == L'\0'))
			{
				PROVIDER_FREE(pDestBuf);
				break;
			}
			break;		 

		case DBTYPE_I2:
		{
			// Alloc sizeof(long) bytes since a long is returned
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(long));			
			long l;
			SHORT s;

			if (!pDestBuf)
				break;

			*(long *)pDestBuf =	wcstol(pwszSource, &pwszStop, 10);
			if (!(pwszStop && pwszStop[0] == L'\0'))
			{
				PROVIDER_FREE(pDestBuf);
				break;
			}
		
			// Check that the value fits in the range of short
			// before casting it to a short
			ASSERT(*(long *)pDestBuf <= 32767);
			ASSERT(*(long *)pDestBuf >= -32768);
			l = *(long *)pDestBuf;
			s = (SHORT)l;
			*(SHORT *)pDestBuf = s;
			break;
		}
		case DBTYPE_UI2:
		{
			// Alloc sizeof(long) bytes since a long is returned
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(long));			
			long l;
			USHORT us;

			if (!pDestBuf)
				break;

			*(long *)pDestBuf =	wcstol(pwszSource, &pwszStop, 10);
			if (!(pwszStop && pwszStop[0] == L'\0'))
			{
				PROVIDER_FREE(pDestBuf);
				break;
			}
		
			// Check that the value fits in the range of an
			// unsigned short before casting to it
			ASSERT(*(long *)pDestBuf <= 65535);
			l = *(long *)pDestBuf;
			us = (USHORT)l;
			*(USHORT *)pDestBuf = us;
			break;
			
		}
		case DBTYPE_R8:			  			 
		{	
			// Need new block to limit scope of wszEndChar to this case
			LPWSTR  pwszEndChar = L"";
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(double));			
			if (!pDestBuf)
				break;
			
			// wcstod returns a double, make sure this corresponds with
			// the R8 datatype
			ASSERT(sizeof(double) == 8);
			*(double *)pDestBuf =wcstod(pwszSource,(WCHAR **)&pwszEndChar);
			
			if (!(pwszEndChar && (pwszEndChar[0] == L'\0' || (fLaxConvert && pwszEndChar[0] == L' '))))
				PROVIDER_FREE(pDestBuf);
			break;		
		}
		case DBTYPE_R4:
		{	
			// Need new block to limit scope of wszEndChar
			LPWSTR  pwszEndChar = L"";
			float	flt;
			double	dbl;

			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(double));
			if (!pDestBuf)
				break;
							  
			// Use ansi string to convert to float								 			
			*(double *)pDestBuf =wcstod(pwszSource,(WCHAR **)&pwszEndChar);				

			if (!(pwszEndChar && (pwszEndChar[0] == L'\0' || (fLaxConvert && pwszEndChar[0] == L' '))))
			{
				PROVIDER_FREE(pDestBuf);
				break;
			}

			// Assign from double to a float so conversion takes place		
			dbl = *(double*)pDestBuf;
			flt = (float)dbl;
			*(float *)pDestBuf = flt;
			break;
		}
		case DBTYPE_VARNUMERIC:
		{
			// Need new block to limit scope of wszEndChar
			USHORT	 usIdx;
			BOOL	fScale = FALSE;
			WCHAR	wszValue[400] = L"";
			WCHAR	*pwszBuff = NULL;
			size_t	lScale = 0;
			size_t	ul=0, cchIn=0, ulPrecision=0;
			ULONG	i=0;
			LONG	lExp = 0;

			cchIn = wcslen(pwszSource);

			memset(wszValue, 0, sizeof(wszValue));

			// 3.321928094887  is 1/(LOG(base 10) of 2)
			// compute # of bytes (upper bound) needed to represent string as scaled integer
			cBytes = (USHORT)ceil(((cchIn+1)*3.321928094887 )/8.0);
			cBytes += sizeof(DB_VARNUMERIC); 
				
			pDestBuf = (BYTE *)PROVIDER_ALLOC(cBytes);
			if (!pDestBuf)
				break;

			memset(pDestBuf,0,(size_t)cBytes);

			// Set sign to positive
			((DB_VARNUMERIC *)pDestBuf)->sign = 1;

			// Record sign if it's there -- it's  1 for positive 
			// numbers and 0 for negative numbers
			if (*pwszSource == '-')
			{
				((DB_VARNUMERIC *)pDestBuf)->sign = 0;
				pwszSource++;
				cchIn--;							
			}

			// Remove leading zeros
			while (*pwszSource == '0')
			{
				pwszSource++;
				cchIn--;							
			}

			// Look for scientific notation
			for( i = 0; i < cchIn; i++ )
			{
				if( towlower(*(pwszSource + i)) == L'e' )
				{
					// Try to extract the exponent
					lExp = wcstol(pwszSource+i+1, &pwszStop, 10);
					if (!(pwszStop && pwszStop[0] == L'\0'))
					{
						// Error out
						PROVIDER_FREE(pDestBuf);
						goto CLEANUP;
					}

					// Update length to ignore exponent
					cchIn = i;
				}
			}

			// If we have an exponent, we need to modify the string
			if( lExp )
			{
				// Too large an exponent to fit?
				if( abs(lExp) + i + 1 > sizeof(wszValue)/sizeof(WCHAR) )
				{
					PROVIDER_FREE(pDestBuf)
					break;
				}

				// We're going to write into the temporary buffer
				pwszBuff = wszValue;
			
				// Handle a leading sign character on the input string
				if( *pwszSource == L'-' || *pwszSource == L'+' )
				{
					pwszSource++;
					cchIn--;							
				}

				// If positive exponent, shift decimal point to right
				if( lExp > 0 )
				{
					// Copy leading digits
					for( i = 0; i < cchIn; i++ )
					{
						if( *(pwszSource + i) == L'.' )
							break;
						*pwszBuff++ = *(pwszSource + i);
						ulPrecision++;
					}
					// Copy digits after decimal point
					if( i < cchIn - 1 )
						for( i++; i < cchIn && lExp; i++, lExp--, ulPrecision++ )
							*pwszBuff++ = *(pwszSource + i);
					// Add trailing zeros if necessary
					if( lExp )
					{
						while( lExp-- && ulPrecision++)
							*pwszBuff++ = L'0';
					}
					// Insert decimal point and copy rest of string
					else
					{	
						*pwszBuff++ = L'.';
						fScale = TRUE;
						while( i < cchIn )
						{
							*pwszBuff++ = *(pwszSource + i++);
							lScale++;
							ulPrecision++;
						}
					}
				}
				// Negative exponent, shift decimal point to left
				else
				{
					lExp = -lExp;
					// Find decimal point
					for( i = 0; i < cchIn; i++ )
						if( *(pwszSource + i) == L'.')
							break;
					// Add leading zeros if necessary
					if( i < (ULONG)lExp )
					{
						// This number begins with a decimal point
						fScale = TRUE;
						// Add leading zeros
						ASSERT(i < SHRT_MAX);
						for( lExp -= (SHORT)i; lExp; lExp--, lScale++, ulPrecision++ )
							*pwszBuff++ = L'0';
						// Copy digits originally before the decimal point
						memcpy(pwszBuff, pwszSource, i * sizeof(WCHAR));
						pwszBuff += i;
						lScale += i;
						ulPrecision += i;
					}
					// No leading zeros; just moving decimal point
					else
					{
						// Move up to new decimal point
						memcpy(pwszBuff, pwszSource, (i - lExp) * sizeof(WCHAR));
						pwszBuff += i - lExp;
						ulPrecision += i - lExp;
						// This varnumeric's decimal point begins after ith digit
						fScale = TRUE;

						// Move the rest
						memcpy(pwszBuff, pwszSource + (i - lExp) * sizeof(WCHAR), lExp * sizeof(WCHAR));
						pwszBuff += lExp;
						ulPrecision += lExp;
						lScale += lExp;
					}
					// Adjust for decimal point if necessary
					if( i < cchIn )
						i++;
					cchIn -= i;
					// Move digits after old decimal point if necessary
					if( cchIn )
						memcpy(pwszBuff, pwszSource + i, cchIn * sizeof(WCHAR));
					pwszBuff += cchIn;
					ulPrecision += cchIn;
					if (fScale)
						lScale += cchIn;
				}
			}
			else
			{
				// non-exponent case
				// Go thru the wchars and count the percision and scale
				for(; *pwszSource && *pwszSource != L' '; pwszSource++) 
				{
					// Find out where scale starts
					if (*pwszSource == '.')
						fScale = TRUE;
					else
					{ 
						// Count precision and scale
						if (fScale) 
							lScale++;
										
						ulPrecision++;

						// Make a copy of the Value 
						memcpy(&wszValue[ulPrecision-1],pwszSource,sizeof(BYTE));
					}
				}
			}

			if( ulPrecision != 0 )
			{
				// optimize for leading/trailing zeroes
				if ( fScale == FALSE )
				{
					for(ul=ulPrecision-1; wszValue[ul] == L'0' && ul; ul--, ulPrecision--, lScale--);
				}
				else if ( 0 == (ulPrecision-lScale) )
				{
					WCHAR wszTmpBuf[400] = L"";

					for(ul=0; wszValue[ul] == L'0' && ul < ulPrecision; ul++);

					if( ul != ulPrecision )
					{
						ulPrecision -= USHORT(ul);
					
						wcscpy(wszTmpBuf, wszValue+ul);
						wcscpy(wszValue, wszTmpBuf);

						// remove trailing zeros
						for(ul=ulPrecision-1; wszValue[ul] == L'0' && ul; ul--, ulPrecision--, lScale--);
					}
				}
			}

			// after accounting for leading/trailing zeroes, the precision/scale must fit in an unsigned char
			ASSERT(ulPrecision <= 255 && lScale <= 255);
			
			// Special case for ulPrecision = 0 because even zero technically has precision of 1
			if ( ulPrecision == 0 )
			{
				ulPrecision = 1;
				wszValue[0] = L'0';
			}

			((DB_VARNUMERIC *)pDestBuf)->precision = BYTE(ulPrecision);
			((DB_VARNUMERIC *)pDestBuf)->scale = SBYTE(lScale);

			// Convert to DB_VARNUMERIC
			for (usIdx = 0; usIdx < ((DB_VARNUMERIC *)pDestBuf)->precision; usIdx++)
			{
				BOOL result = AddCharToVarNumericVal(wszValue[usIdx], (DB_VARNUMERIC *)pDestBuf, USHORT(cBytes-sizeof(DB_VARNUMERIC)+1));
				ASSERT( result );
			}

			// Revise cb estimate
			cBytes = (USHORT)ceil(((((DB_VARNUMERIC *)pDestBuf)->precision+1)*3.321928094887 )/8.0);
			if (cBytes > 1 && ((DB_VARNUMERIC *)pDestBuf)->val[cBytes-1] == 0)
				cBytes--;
			cBytes += sizeof(DB_VARNUMERIC)-1;  // -1 because the VARNUMERIC struct already contains one BYTE
			break;
		}
		case DBTYPE_NUMERIC:
		{	
			// Need new block to limit scope of wszEndChar
			USHORT usIdx;
			BOOL fScale = FALSE;
			WCHAR wszValue[128] = L"";

			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(DB_NUMERIC));			
			if (!pDestBuf)
				break;

			memset(pDestBuf,0,sizeof(DB_NUMERIC));

			// Set sign to positive
			((DB_NUMERIC *)pDestBuf)->sign = 1;

			// Record sign if it's there -- it's  1 for positive 
			// numbers and 0 for negative numbers
			if (*pwszSource == '-')
			{
				((DB_NUMERIC *)pDestBuf)->sign = 0;
				pwszSource++;
			}
			
			// Count each leading zero as an indication to increment precision		
			// Go thru the wchars and count the percision and scale
			for(; *pwszSource && *pwszSource != L' '; pwszSource++) 
			{
				// Find out where scale starts
				if (*pwszSource == '.')
					fScale = TRUE;
				else
				{
					// Count precision and scale
					if (fScale) 
						((DB_NUMERIC *)pDestBuf)->scale++;
									
					((DB_NUMERIC *)pDestBuf)->precision++;

					// Make a copy of the Value 
					memcpy(&wszValue[((DB_NUMERIC *)pDestBuf)->precision-1],pwszSource,sizeof(BYTE));
				}
			}
			
			// Special case for ulPrecision = 0 because even zero technically has precision of 1
			if (((DB_NUMERIC *)pDestBuf)->precision == 0)
			{
				((DB_NUMERIC *)pDestBuf)->precision = 1;
				wszValue[0] = L'0';
			}

			// Convert BSTR to DB_NUMERIC
			for (usIdx = 0; usIdx < ((DB_NUMERIC *)pDestBuf)->precision; usIdx++)
			{
				BOOL result = AddCharToNumericVal(wszValue[usIdx], (DB_NUMERIC *)pDestBuf);
				ASSERT( result );
			}
				
			break;
		}

		case DBTYPE_DATE:
		{
			VARIANT vTempBstr;
			HRESULT hr = S_OK;

			pDestBuf = (BYTE *) PROVIDER_ALLOC(sizeof(DATE));
			if (!pDestBuf)
				break;

			//Use OLE Automation to convert from BSTR to DATE.
			//Since DATE is OLE Automations defintion
			VariantInit(&vTempBstr);
			V_VT(&vTempBstr) = VT_BSTR;
			V_BSTR(&vTempBstr) = SysAllocString(pwszSource);

			// Use the US_English LCID first and then the System LCID
			hr = VariantChangeTypeEx(&vTempBstr, &vTempBstr, MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), VARIANT_NOVALUEPROP, VT_DATE);
			if(hr == DISP_E_TYPEMISMATCH)
				hr = VariantChangeTypeEx(&vTempBstr, &vTempBstr, GetSystemDefaultLCID(), VARIANT_NOVALUEPROP, VT_DATE);
			ASSERT(hr==S_OK);
			
			*(DATE*)pDestBuf = V_DATE(&vTempBstr);
			GCHECK(VariantClear(&vTempBstr),S_OK);
			break;
		}

		case DBTYPE_DBDATE:
		{
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(DBDATE));			
			long l;
			WCHAR wszYear[5];
			WCHAR wszMonth[3];
			WCHAR wszDay[3];
			SHORT s;
			USHORT us;		
			
			if (!pDestBuf)
				break;

			// Parse each part of date into a separate buffer
			wcsncpy(wszYear, pwszSource, 4);
			wszYear[(sizeof(wszYear)/sizeof(WCHAR))-1] = L'\0';
			wcsncpy(wszMonth, pwszSource+5, 2);
			wszMonth[(sizeof(wszMonth)/sizeof(WCHAR))-1] = L'\0';
			wcsncpy(wszDay, pwszSource+8, 2);
			wszDay[(sizeof(wszDay)/sizeof(WCHAR))-1] = L'\0';

			// Now convert to a numeric value and place in struct
			l = _wtol(wszYear);				
			ASSERT(l <= 9999);
			s = (SHORT)l;		
			((DBDATE *)pDestBuf)->year = s;
			
			l = _wtol(wszMonth);				
			ASSERT(l <= 12);
			us = (USHORT)l;		
			((DBDATE *)pDestBuf)->month = us;
			
			l = _wtol(wszDay);				
			ASSERT(l <= 31);
			us = (USHORT)l;		
			((DBDATE *)pDestBuf)->day = us;		
			break;
		}
		case DBTYPE_DBTIME:
		{
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(DBTIME));			
			long l;
			WCHAR wszHour[3];
			WCHAR wszMinute[3];
			WCHAR wszSecond[3];		
			USHORT us;		
			
			if (!pDestBuf)
				break;

			// Parse each part of date into a separate buffer
			wcsncpy(wszHour, pwszSource, 2);
			wszHour[(sizeof(wszHour)/sizeof(WCHAR))-1] = L'\0';
			wcsncpy(wszMinute, pwszSource+3, 2);
			wszMinute[(sizeof(wszMinute)/sizeof(WCHAR))-1] = L'\0';
			wcsncpy(wszSecond, pwszSource+6, 2);
			wszSecond[(sizeof(wszSecond)/sizeof(WCHAR))-1] = L'\0';

			// Now convert to a numeric value and place in struct
			l = _wtol(wszHour);				
			ASSERT(l <= 24);
			us = (USHORT)l;		
			((DBTIME *)pDestBuf)->hour = us;
			
			l = _wtol(wszMinute);				
			ASSERT(l <= 59);
			us = (USHORT)l;		
			((DBTIME *)pDestBuf)->minute = us;
			
			l = _wtol(wszSecond);				
			ASSERT(l <= 59);
			us = (USHORT)l;		
			((DBTIME *)pDestBuf)->second = us;		
			break;
		}
		case DBTYPE_DBTIMESTAMP:
		{
			pDestBuf = (BYTE *)PROVIDER_ALLOC(sizeof(DBTIMESTAMP));			
			long l;
			WCHAR wszYear[5];
			WCHAR wszMonth[3];
			WCHAR wszDay[3];		
			WCHAR wszHour[3];
			WCHAR wszMinute[3];
			WCHAR wszSecond[3];		
			WCHAR wszFraction[10];	//Per spec, up to 9 digits may be used
			SHORT	s;
			USHORT	us;		
			size_t	i;

			if (!pDestBuf)
				break;

			// Parse each part of date into a separate buffer
			wcsncpy(wszYear, pwszSource, 4);
			wszYear[(sizeof(wszYear)/sizeof(WCHAR))-1] = L'\0';
			wcsncpy(wszMonth, pwszSource+5, 2);
			wszMonth[(sizeof(wszMonth)/sizeof(WCHAR))-1] = L'\0';
			wcsncpy(wszDay, pwszSource+8, 2);
			wszDay[(sizeof(wszDay)/sizeof(WCHAR))-1] = L'\0';
			wcsncpy(wszHour, pwszSource+11, 2);
			wszHour[(sizeof(wszHour)/sizeof(WCHAR))-1] = L'\0';
			wcsncpy(wszMinute, pwszSource+14, 2);
			wszMinute[(sizeof(wszMinute)/sizeof(WCHAR))-1] = L'\0';
			wcsncpy(wszSecond, pwszSource+17, 2);
			wszSecond[(sizeof(wszSecond)/sizeof(WCHAR))-1] = L'\0';
			
			if (wcslen(pwszSource) < 20)
				wszFraction[0] = L'\0';
			else
			{	
				// Copy all fraction digits to null terminator
				wcscpy(wszFraction, pwszSource+20);

				// We must pad out to the end (for a total of 9 digits)
				// the fraction with 0 to get right ULONG value from the fraction string		
				for(i=9-wcslen(wszFraction); i; i--)
					wcscat(wszFraction, L"0");
			}

			// Now convert to a numeric value and place in struct
			l = _wtol(wszYear);				
			ASSERT(l <= 9999);
			s = (SHORT)l;		
			((DBTIMESTAMP *)pDestBuf)->year = s;
			
			l = _wtol(wszMonth);				
			ASSERT(l <= 12);
			us = (USHORT)l;		
			((DBTIMESTAMP *)pDestBuf)->month = us;
			
			l = _wtol(wszDay);				
			ASSERT(l <= 31);
			us = (USHORT)l;		
			((DBTIMESTAMP *)pDestBuf)->day = us;
			
			l = _wtol(wszHour);				
			ASSERT(l <= 24);
			us = (USHORT)l;		
			((DBTIMESTAMP *)pDestBuf)->hour = us;
			
			l = _wtol(wszMinute);				
			ASSERT(l <= 59);
			us = (USHORT)l;		
			((DBTIMESTAMP *)pDestBuf)->minute = us;
			
			l = _wtol(wszSecond);				
			ASSERT(l <= 59);
			us = (USHORT)l;		
			((DBTIMESTAMP *)pDestBuf)->second = us;		
									
			((DBTIMESTAMP *)pDestBuf)->fraction = _wtol(wszFraction);
			break;	
		}
		case DBTYPE_CY:
		{
			VARIANT vTempBstr;
			VARIANT vTempLARGE;
			HRESULT hr=0;

			pDestBuf = (BYTE *) PROVIDER_ALLOC(sizeof(LARGE_INTEGER));
			ASSERT(sizeof(LARGE_INTEGER) == 8);

			if (!pDestBuf)
				break;

			// Convert wstr to VARIANT_BSTR then change BSTR into CY
			VariantInit(&vTempBstr);
			VariantInit(&vTempLARGE);

			vTempBstr.vt = VT_BSTR;
			vTempBstr.bstrVal = SysAllocString(pwszSource);

			// Use the US_English LCID first and then the System LCID
			hr = VariantChangeTypeEx(&vTempLARGE, &vTempBstr, MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), VARIANT_NOVALUEPROP, VT_CY);
			if(hr == DISP_E_TYPEMISMATCH)
				GCHECK(hr = VariantChangeTypeEx(&vTempLARGE, &vTempBstr, GetSystemDefaultLCID(), VARIANT_NOVALUEPROP, VT_CY),S_OK);
			
			if(SUCCEEDED(hr))
				memcpy((LARGE_INTEGER*)pDestBuf,&V_CY(&vTempLARGE),sizeof(LARGE_INTEGER));

			SysFreeString(vTempBstr.bstrVal);
			GCHECK(VariantClear(&vTempLARGE),S_OK);
			break;
		}
		case DBTYPE_DECIMAL:
		{
			VARIANT vTempBstr;
			VARIANT vTempDECIMAL;
			HRESULT hr=0;

			pDestBuf = (BYTE *) PROVIDER_ALLOC(sizeof(DECIMAL));
			
			if (!pDestBuf)
				break;

			// Convert wstr to VARIANT_BSTR then change BSTR to DECIMAL
			VariantInit(&vTempBstr);
			VariantInit(&vTempDECIMAL);

			vTempBstr.vt = VT_BSTR;
			vTempBstr.bstrVal = SysAllocString((LPOLESTR)pwszSource);

			// Use the US_English LCID first and then the System LCID
			hr=VariantChangeTypeEx(&vTempDECIMAL, &vTempBstr, MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), VARIANT_NOVALUEPROP, VT_DECIMAL);
			if(hr == DISP_E_TYPEMISMATCH)
				hr=VariantChangeTypeEx(&vTempDECIMAL, &vTempBstr, GetSystemDefaultLCID(), VARIANT_NOVALUEPROP, VT_DECIMAL);
			ASSERT(!hr);
			memcpy((DECIMAL*)pDestBuf,&(vTempDECIMAL.decVal),sizeof(DECIMAL));

			SysFreeString(vTempBstr.bstrVal);
			GCHECK(VariantClear(&vTempDECIMAL),S_OK);
			break;
		}
		case DBTYPE_I8:
		case DBTYPE_UI8:
		{			
			pDestBuf = (BYTE *) PROVIDER_ALLOC(sizeof(LARGE_INTEGER));
			if (!pDestBuf)
				break;

			*(__int64 *)pDestBuf = PrivLib_wtoi64(pwszSource);
			break;
		}
		case DBTYPE_FILETIME:
		{
			SYSTEMTIME	sysTime;
			LONG		lTemp=0;
			WCHAR		wch = 0;
			WCHAR*		pwszFileTime = NULL;

			pwszFileTime = wcsDuplicate(pwszSource);
			pDestBuf = (BYTE *) PROVIDER_ALLOC(sizeof(FILETIME));
			if (!pDestBuf || !pwszFileTime)
				break;
					
			lTemp = swscanf(pwszFileTime,
							L"%5hd-%2hu-%2hu %2hu:%2hu:%2hu.%3hu%c",
							&sysTime.wYear,
							&sysTime.wMonth,
							&sysTime.wDay,
							&sysTime.wHour,
							&sysTime.wMinute,
							&sysTime.wSecond,
							&sysTime.wMilliseconds,
							&wch);

			SAFE_FREE(pwszFileTime);

			// Must have 3 digit millisecond or bust
			if (lTemp == 6)
				sysTime.wMilliseconds = 0;
			
			if (lTemp < 6 || (lTemp > 7 && wch != ' ') ||
				!(SystemTimeToFileTime(&sysTime, (FILETIME*)pDestBuf)))
			{
				ASSERT(!"Bad SystemTime string!\n");
				PROVIDER_FREE(pDestBuf);
			}

			break;
		}
		case DBTYPE_VARIANT:
		{			
			pDestBuf = (BYTE *) PROVIDER_ALLOC(sizeof(VARIANT));
			if (!pDestBuf)
				break;

			VariantInit((VARIANT *)pDestBuf);

			V_VT((VARIANT *)pDestBuf) = VT_BSTR;
			V_BSTR((VARIANT *)pDestBuf) = SysAllocString(pwszSource);

			break;
		}
		
		default:
		{
			//DBTYPE_ARRAY
			if(dbDestType & DBTYPE_ARRAY)
			{
				pDestBuf = (BYTE*)PROVIDER_ALLOC(sizeof(SAFEARRAY*));
				if(pDestBuf)
				{	
					//Delegate to our SafeArray conversion functions...
					if(FAILED(StringToSafeArray(pwszSource, dbDestType & ~DBTYPE_ARRAY, (SAFEARRAY**)pDestBuf)))
						PROVIDER_FREE(pDestBuf);
					cBytes = sizeof(SAFEARRAY*);
				}
			}
			//DBTYPE_VECTOR
			else if(dbDestType & DBTYPE_VECTOR)
			{
				pDestBuf = (BYTE*)PROVIDER_ALLOC(sizeof(DBVECTOR));
				if(pDestBuf)
				{
					DBVECTOR* pVector = (DBVECTOR*)pDestBuf;

					//Covert the String into a Vector...
					//NOTE: This function currently assumes a "comma-deliminated" string...
					if(FAILED(StringToVector(pwszSource, dbDestType, pVector)))
						PROVIDER_FREE(pDestBuf);
					cBytes = sizeof(DBVECTOR);
				}				
			}
			else
			{
				// We are currently only converting DBTYPES we know ODBC Provider will give us
				// If we hit this ASSERT we have to add more support
				ASSERT(FALSE);
			}
		}
	}

CLEANUP:
	// This pointer should have been assigned in one of the case
	// statements, otherwise an error occured and it should be null
	if(pcb)
		*pcb = cBytes;

	// Restore the locale and free locale string
	{
		// set critical section
		MFCriticalSection lock;

		if (pszLocaleInfo)
		{
			setlocale(LC_ALL, pszLocaleInfo);
			free(pszLocaleInfo);
		}
	}

	return pDestBuf;
}




/////////////////////////////////////////////////////////////////////////////
// HRESULT VariantToString
//
/////////////////////////////////////////////////////////////////////////////
HRESULT VariantToString(VARIANT* pVariant, WCHAR* pwsz, ULONG ulMaxSize, BOOL fDispVarBool)
{
	//Convert a VARIANT to a WCHAR
	ASSERT(pVariant);
	ASSERT(pwsz);
	ASSERT(ulMaxSize > 0);
	HRESULT hr = S_OK;
	
	//Find the VariantType
	DBTYPE wType = V_VT(pVariant);
	VARIANT VarTemp;
	VariantInit(&VarTemp);
	pwsz[0] = L'\0';

	//VT_ARRAY is not handled by VariantChangeTypeEx
	if(wType & VT_ARRAY)
	{
		hr = SafeArrayToString(V_ARRAY(pVariant), wType, pwsz, ulMaxSize);
		goto CLEANUP;
	}

	switch(wType)
	{
		case VT_NULL:
		case VT_EMPTY:
			pwsz[0] = L'\0';
			break;

		case VT_BOOL:
			if(fDispVarBool)
				swprintf(pwsz, L"%s", V_BOOL(pVariant) == VARIANT_TRUE ? L"VARIANT_TRUE" : L"VARIANT_FALSE");
			else
				swprintf(pwsz, L"%s", V_BOOL(pVariant) == VARIANT_TRUE ? L"True" : L"False");
			break;

		case VT_ERROR:
			swprintf(pwsz, L"%d", V_ERROR(pVariant));
			break;

		default:
		{
			TESTC_(hr = VariantChangeTypeEx(
				&VarTemp,	// Destination (convert not in place)
				pVariant,	// Source
				LOCALE_SYSTEM_DEFAULT,	// LCID
				0,						// dwFlags
				VT_BSTR ),S_OK);

			//Copy the string the from the Variant to buffer
			wcsncpy(pwsz, V_BSTR(&VarTemp), ulMaxSize);
		}
	};

CLEANUP:
	pwsz[ulMaxSize-1] = L'\0';
	GCHECK(VariantClear(&VarTemp),S_OK);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT StringToVariant
//
/////////////////////////////////////////////////////////////////////////////
HRESULT StringToVariant(WCHAR* pwsz, VARTYPE vt, VARIANT* pVariant)
{
	//Convert a VARIANT to a WCHAR
	ASSERT(pwsz);
	ASSERT(pVariant);
	HRESULT hr = S_OK;

	//Assign the type...
	V_VT(pVariant) = vt;

	//VT_ARRAY is not handled by VariantChangeTypeEx
	if(vt & VT_ARRAY)
	{
		SAFEARRAY* pSafeArray = NULL;
		TESTC_(hr = StringToSafeArray(pwsz, vt, &pSafeArray),S_OK);
		V_ARRAY(pVariant) = pSafeArray;
		goto CLEANUP;
	}

	//VariantChangeTypeEx seems to handle most types,
	//except the following cases...
	switch(vt)
	{
		case VT_NULL:
		case VT_EMPTY:
			break;

		case VT_BOOL:
		{	
			if(wcscmp(pwsz, L"VARIANT_TRUE")==0)
				V_BOOL(pVariant) = VARIANT_TRUE;
			else
				V_BOOL(pVariant) = VARIANT_FALSE;
			break;
		}

		case VT_ERROR:
		{	
			LONG lValue = wcstol(pwsz, NULL, 10);
			if(lValue == LONG_MAX)
			{
				hr = E_FAIL;
				goto CLEANUP;
			}
			V_ERROR(pVariant) = lValue;
			break;
		}

		case VT_VARIANT:
		{	
			//Place the string into the BSTR of the VARIANT
			V_VT(pVariant) = VT_BSTR;
			V_BSTR(pVariant) = SysAllocString(pwsz);
			break;
		}

		default:
		{
			//Place the string into the BSTR of the VARIANT
			V_VT(pVariant) = VT_BSTR;
			V_BSTR(pVariant) = SysAllocString(pwsz);
			
			//Now delegate to VariantChangeType...
			TESTC_(hr = VariantChangeTypeEx(
				pVariant,	// Destination (convert in place)
				pVariant,	// Source
				LOCALE_SYSTEM_DEFAULT,	// LCID
				0,						// dwFlags
				vt ),S_OK);
		}
	};

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT SafeArrayToString
//
/////////////////////////////////////////////////////////////////////////////
HRESULT SafeArrayToString(SAFEARRAY* pSafeArray, DBTYPE wType, WCHAR* pwszBuffer, DBLENGTH ulMaxSize)
{
	ASSERT(pSafeArray);
	ASSERT(pwszBuffer);

	//No-op
	if(!ulMaxSize)
		return S_OK;

	//This method is capable of handling N-Dimenstions of Data!!
	//We need to take N-Dimensions of Data and convert it to a string

	//For example:  We have a 3D data, where z-dim has 2 elements, y-Dim has 3 ele
	//and x-dim has 2 elements we end up with the following matrix of points
	
	//	(z, y, x)  => value	[[[
	//	(1, 1, 1)  => 1
	//	(1, 1, 2)  => 2
	//	(1, 2, 1)  => 3		][
	//	(1, 2, 2)  => 4
	//	(1, 3, 1)  => 5		][
	//	(1, 3, 2)  => 6
	//	(2, 1, 1)  => 7		]][[
	//	(2, 1, 2)  => 8
	//	(2, 2, 1)  => 9		][
	//	(2, 2, 2)  => A
	//	(2, 3, 1)  => B		][
	//	(2, 3, 2)  => C
	//						]]]

	//So what we need to generate is a string of:
	//	[ [[1,2][3,4][5,6]] [[7,8][9,A][B,C]] ]

	//The algorythm we are using is bascially based upon a "ripple" counter.
	//We keep a counter for each dimension.  So when CounterDim[0] hits the Upper
	//Limit, we increment CounterDim[1] and set CounterDim[0] back to LowerLimit.  
	//This continues until all have reached the upper limit together:
	//{CounterDim[n-1]==UpperLimt, ... Dim[0]==UpperLimit}

	//The braces are determined by rising/falling edges of the ripple counter.
	//Everytime a dimension restarts its value from Upper->Lower we see a "][".
	//So we have a pre and a post set of - "[[[" braces "]]]" for the number of dimensions.
	//You'll notices the set of braces in the above example on the rising/falling
	//edges of the ripple counter....

	HRESULT hr = S_OK;

	VARIANT Variant;
	VariantInit(&Variant);
	WCHAR* pwsz = pwszBuffer;
	WCHAR* pwszEnd = pwsz + ulMaxSize;

	ULONG i,iDim, ulInnerElements = 0;
	ULONG cDimensions = SafeArrayGetDim(pSafeArray);
	BOOL bDone = FALSE;

	//No-op
	if(!cDimensions)
		return E_FAIL;

	//Make sure there is no Array in the type
	wType &= ~VT_ARRAY;
	pwsz[0] = L'\0';

	LONG* rgIndices = NULL;
	SAFE_ALLOC(rgIndices, LONG, cDimensions);

	//Loop over all dimenstions and fill in "pre" info...
	for(iDim=0; iDim<cDimensions; iDim++)
	{
		//Fill in lower bound
		LONG lLowerBound = 0;
		SafeArrayGetLBound(pSafeArray, iDim+1, &lLowerBound);
		rgIndices[iDim] = lLowerBound;
		
		//Fill in outer dimension indicater
		*pwsz = L'[';
		pwsz++;
	}
	
	//Calculate the total number of inner items...
	//This is easy, all dimensions will have the same number "inner" items
	//IE:  rg[y][x] - all y arrays have x elements.
	//IE:  rg[z][y][x] - all z arrays, have y arrays, which have x elements
	ulInnerElements = pSafeArray->rgsabound[0].cElements;
	while(!bDone && (pwsz < pwszEnd))
	{	
		//Dimension[0] always goes through a complete cycle every time
		//Just do this part of the ripple counter seperately...
		for(i=0; i<ulInnerElements; i++)
		{
			//Initialize Variant
			VariantInit(&Variant);
			
			//Obtain the data from the safe array...
			switch(wType)
			{
				case VT_VARIANT:
					//just place directly into our variant.
					TESTC_(hr = SafeArrayGetElement(pSafeArray, rgIndices, &Variant),S_OK);
					break;
				
				case VT_EMPTY:
				case VT_NULL:
				case VT_I2:
				case VT_I4:
				case VT_R4:
				case VT_R8:
				case VT_CY:
				case VT_DATE:
				case VT_BSTR:
				case VT_DISPATCH:
				case VT_ERROR:
				case VT_BOOL:
				case VT_UNKNOWN:
				case VT_I1:
				case VT_UI1:
				case VT_UI2:
				case VT_UI4:
				case VT_I8:
				case VT_UI8:
				case VT_INT:
				case VT_UINT:
					//Otherwise if its a valid variant type, we can place it within
					//our variant as a subtype and delegate to VariantToString to
					//do the converion
					V_VT(&Variant) = wType;
					TESTC_(hr = SafeArrayGetElement(pSafeArray, rgIndices, &V_I4(&Variant)),S_OK);
					break;

				case VT_DECIMAL:
					//DECIMAL is not part of the VARIANT union
					V_VT(&Variant)		= wType;
					TESTC_(hr = SafeArrayGetElement(pSafeArray, rgIndices, &V_DECIMAL(&Variant)),S_OK);
					break;

				default:
					//Unable to handle this type...
					TESTC_(hr = E_FAIL,S_OK);
			}
			
			rgIndices[0]++;

			//Convert VARIANT To String
			TESTC_(hr = VariantToString(&Variant, pwsz, (ULONG)(pwszEnd - pwsz)),S_OK);
			pwsz += wcslen(pwsz);
			
			//Array Seperator
			if(i<ulInnerElements-1 && (pwsz < pwszEnd))
			{
				*pwsz = L',';
				pwsz++;
			}

			//Clear the Variant, (could be outofline memory...)
			TESTC_(VariantClear(&Variant),S_OK);

			if(!(pwsz < pwszEnd))
				break;
		}

		//Adjust the other Dimensions of the ripple counter
		for(iDim=0; iDim<cDimensions; iDim++)
		{
			LONG lUpperBound = 0;
			SafeArrayGetUBound(pSafeArray, iDim+1, &lUpperBound);
			
			//Increment this ripple if below max bound, and exit out (break)
			if(rgIndices[iDim] < lUpperBound)
			{
				rgIndices[iDim]++;

				//Need to add opening braces...
				for(ULONG j=iDim; j>0 && (pwsz < pwszEnd); j--)
				{
					*pwsz = L'[';
					pwsz++;
				}
				break;
			}
			//Otherwise reset this one and move onto the 
			//next Dimension (ie: Don't break...)
			else if(iDim != cDimensions-1)
			{
				LONG lLowerBound = 0;
				SafeArrayGetLBound(pSafeArray, iDim+1, &lLowerBound);
				rgIndices[iDim] = lLowerBound;
				
				if(pwsz < pwszEnd)
				{
					*pwsz = L']';
					pwsz++;
				}
			}
			//If we have hit the last Dimension and its over the value
			//This means were done...
			else
			{
				bDone = TRUE;
			}
		}
	}

	//Display Right outer braces
	for(iDim=0; iDim<cDimensions && (pwsz < pwszEnd); iDim++)
	{
		*pwsz = L']';
		pwsz++;
	}

CLEANUP:
	VariantClear(&Variant);
	SAFE_FREE(rgIndices);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT StringToSafeArray
//
/////////////////////////////////////////////////////////////////////////////
HRESULT StringToSafeArray(WCHAR* pwszBuffer, DBTYPE wType, SAFEARRAY** ppSafeArray)
{
	ASSERT(pwszBuffer);
	ASSERT(ppSafeArray);

	//We already know we need to create an array
	wType &= ~VT_ARRAY;

	//This method is capable of handling N-Dimenstions of Data!!
	//We need to a String of N Dimensions and turn it into a SafeArray

	//For example:  We have a 3D data, where z-dim has 2 elements, y-Dim has 3 ele
	//and x-dim has 2 elements we end up with the following matrix of points
	
	//	(z, y, x)  => value	[[[
	//	(1, 1, 1)  => 1
	//	(1, 1, 2)  => 2
	//	(1, 2, 1)  => 3		][
	//	(1, 2, 2)  => 4
	//	(1, 3, 1)  => 5		][
	//	(1, 3, 2)  => 6
	//	(2, 1, 1)  => 7		]][[
	//	(2, 1, 2)  => 8
	//	(2, 2, 1)  => 9		][
	//	(2, 2, 2)  => A
	//	(2, 3, 1)  => B		][
	//	(2, 3, 2)  => C
	//						]]]

	//So we could be passed a string of:
	//	[ [[1,2][3,4][5,6]] [[7,8][9,A][B,C]] ]

	//The algorythm we are using is bascially based upon a "ripple" counter.
	//We keep a counter for each dimension.  So when CounterDim[0] hits the Upper
	//Limit, we increment CounterDim[1] and set CounterDim[0] back to LowerLimit.  
	//This continues until all have reached the upper limit together:
	//{CounterDim[n-1]==UpperLimt, ... Dim[0]==UpperLimit}

	//The braces are determined by rising/falling edges of the ripple counter.
	//Everytime a dimension restarts its value from Upper->Lower we see a "][".
	//So we have a pre and a post set of - "[[[" braces "]]]" for the number of dimensions.
	//You'll notices the set of braces in the above example on the rising/falling
	//edges of the ripple counter....

	HRESULT hr = S_OK;
	VARIANT Variant;
	VariantInit(&Variant);

	WCHAR wszBuffer[MAX_COL_SIZE];
	wszBuffer[0] = L'\0';
	WCHAR* pwsz = pwszBuffer;

	ULONG i,iDim, ulInnerElements = 0;

	//Determine the Number of Dimensions...
	ULONG cDimensions = 0;
	while(pwsz[0]==L'[')
	{
		cDimensions++;
		pwsz++;
		wcscat(wszBuffer, L"]");
	}

	//Find the End of the Data (where everever "]...") is...
	WCHAR* pwszNext = pwsz;
	WCHAR* pwszEnd = pwsz;
	WCHAR* pwszCurEnd = wcschr(pwsz, L']');
	WCHAR* pwszEndString = wcsstr(pwsz, wszBuffer);

	//No-op
	if(cDimensions < 1)
		return E_FAIL;

	//Create SafeArray
	SAFEARRAY* pSafeArray = NULL;
	*ppSafeArray = NULL;

	LONG* rgIndices = NULL;
	SAFEARRAYBOUND* rgSafeArrayBounds = NULL;

	//Indices array...
	SAFE_ALLOC(rgIndices, LONG, cDimensions);
	memset(rgIndices, 0, cDimensions * sizeof(LONG));

	//SafeArrayBounds array...
	SAFE_ALLOC(rgSafeArrayBounds, SAFEARRAYBOUND, cDimensions);
	memset(rgSafeArrayBounds, 0, cDimensions*sizeof(SAFEARRAYBOUND));

	//Need to find out how many elements are in Dim[0]
	while(pwszNext && pwszNext <= pwszCurEnd)
	{
		ulInnerElements++;
		rgIndices[0]++;
		pwszNext = wcschr(pwszNext, L',');
		if(pwszNext)
			pwszNext++;
	}

	//Now from the [] notation find out how many elements in the other dimenstions
	//	[ [[1,2][3,4][5,6]] [[7,8][9,A][B,C]] ]
	
	//The algorythm we will use is:
	//Everytime we see a "]" we need to increment the next Dimension elements
	//Everytime we see a "[" we need to reset the previous Dimension elements
	
	//TODO
	//Currently this algorythm only handles 1 dimension.
	//No reason it couldn't use the above method and work for more, just on
	//a time constraint, and the only provider we have supports max 1 dim...

	ASSERT(cDimensions == 1);
	
	//Create the SafeArrayBounds
	for(iDim=0; iDim<cDimensions; iDim++)
	{
		rgSafeArrayBounds[iDim].lLbound = 0;
		rgSafeArrayBounds[iDim].cElements = rgIndices[0];
	}
	
	//Now actually create the SafeArray
	pSafeArray = SafeArrayCreate(wType, cDimensions, rgSafeArrayBounds);
	ASSERT(pSafeArray);
	
	pwszCurEnd = wcschr(pwsz, L']');
	rgIndices[0] = 0;
	pwszNext = pwsz;
	for(i=0; i<ulInnerElements; i++)
	{
		//Obtain the start and end of the value
		pwszEnd = wcschr(pwszNext, L',');
		if(pwszEnd==NULL || pwszEnd>pwszCurEnd)
			pwszEnd = pwszCurEnd;

		//Setup rgIndicaes
		rgIndices[0] = i;


		//Convert Value to a Variant
		void* pValue = NULL;
		wcsncpy(wszBuffer, pwszNext, (ULONG)(pwszEnd-pwszNext)+1);
		wszBuffer[pwszEnd-pwszNext] = L'\0';
		TESTC_(hr = StringToVariant(wszBuffer, wType, &Variant),S_OK);

		//Add this Value to the SafeArray
		//NOTE: According to the spec for SafeArrayPutElement
		//VT_DISPATCH, VT_UNKNOWN, and VT_BSTR are pointers, and do not require another level of indirection. 
		switch(wType)
		{
			case VT_DISPATCH:
			case VT_UNKNOWN:
			case VT_BSTR:
				pValue = V_UNKNOWN(&Variant);
				break;
				
			case VT_DECIMAL:
				//DECIMAL is not part of the VARIANT union
				pValue = &V_DECIMAL(&Variant);
				break;
			
			case VT_VARIANT:
				pValue = &Variant;
				break;

			default:
				pValue = &V_I4(&Variant);
				break;
		};

		//Add this Value to the SafeArray
		TESTC_(hr = SafeArrayPutElement(pSafeArray, rgIndices, pValue),S_OK);
		TESTC_(hr = VariantClear(&Variant),S_OK);

		//Incement to next value...
		pwszNext = wcschr(pwszNext, L',');
		if(pwszNext)
			pwszNext += 1;
	}

	//Everything complete successfully...
	*ppSafeArray = pSafeArray;

CLEANUP:
	VariantClear(&Variant);
	SAFE_FREE(rgIndices);
	SAFE_FREE(rgSafeArrayBounds);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT VectorToString
//
/////////////////////////////////////////////////////////////////////////////
HRESULT VectorToString(DBVECTOR* pVector, DBTYPE wType, WCHAR* pwszBuffer, DBLENGTH ulMaxSize)
{
	ASSERT(pVector);
	ASSERT(pwszBuffer);
	HRESULT hr = S_OK;

	//No-op
	if(!ulMaxSize)
		return S_OK;
	
	VARIANT Variant;
	VARIANT* pVariant = NULL;
	WCHAR* pwsz = pwszBuffer;
	WCHAR* pwszEnd = pwsz + ulMaxSize;
	pwsz[0] = L'\0';

	//Make sure we are dealing with the base type...
	wType &= ~DBTYPE_VECTOR;

	//Loop over the vector...
	for(ULONG iEle=0; iEle<pVector->size; iEle++)
	{
		//Initialize Variant
		pVariant = &Variant;
		pVariant->vt = VT_EMPTY;

		//NOTE: The pVariant is really just a pointer to the data.  We don't free the data
		//since the vector data doesn't belong to us, we are just convering the given data to
		//a string.  The simplest way to do this is to dump into a variant and let our helper
		//function VariantToString deal with this...

		//Obtain the data from the vector...
		switch(wType)
		{
			case VT_EMPTY:
			case VT_NULL:
				V_VT(pVariant) = wType;
				break;
		
			case VT_I2:
			case VT_I4:
			case VT_R4:
			case VT_R8:
			case VT_CY:
			case VT_DATE:
			case VT_BSTR:
			case VT_DISPATCH:
			case VT_ERROR:
			case VT_BOOL:
			case VT_UNKNOWN:
			case VT_I1:
			case VT_UI1:
			case VT_UI2:
			case VT_UI4:
			case VT_I8:
			case VT_UI8:
			case VT_INT:
			case VT_UINT:
			{
				LONG lTypeSize = GetDBTypeSize(wType);
				
				V_VT(pVariant) = wType;
				memcpy(&V_I4(pVariant), (BYTE*)pVector->ptr + (lTypeSize*iEle), (size_t)lTypeSize);
				break;
			}

			case VT_DECIMAL:
				//DECIMAL is not part of the VARIANT union
				V_VT(pVariant)		= wType;
				V_DECIMAL(pVariant) = *(DECIMAL*)((BYTE*)pVector->ptr + (sizeof(DECIMAL)*iEle));
				break;
			
			case VT_VARIANT:
				//just place directly into our variant.
				pVariant = (VARIANT*)((BYTE*)pVector->ptr + (sizeof(VARIANT)*iEle));
				break;
			

			default:
				//Unable to handle this type...
				TESTC_(hr = E_FAIL,S_OK);
		}

		//Convert VARIANT To String
		TESTC_(hr = VariantToString(pVariant, pwsz, (ULONG)(pwszEnd - pwsz)),S_OK);
		pwsz += wcslen(pwsz);

		//Vector Seperator
		if(iEle<pVector->size-1 && (pwsz < pwszEnd))
		{
			*pwsz = L',';
			pwsz++;
		}
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT StringToVector
//
/////////////////////////////////////////////////////////////////////////////
HRESULT StringToVector(WCHAR* pwszBuffer, DBTYPE wType, DBVECTOR* pVector)
{
	ASSERT(pwszBuffer);
	ASSERT(pVector);

	//Make sure we are dealing with the base type...
	wType &= ~DBTYPE_VECTOR;

	HRESULT hr = S_OK;
	VARIANT Variant;
	VariantInit(&Variant);

	WCHAR wszBuffer[MAX_COL_SIZE] = {0};
	ULONG iEle = 0;

	//Determine the Number of Elements...
	pVector->size = 1;
	WCHAR* pwsz = pwszBuffer;
	while(pwsz = wcschr(pwsz, L','))
	{
		pVector->size++;
		pwsz++;
	}

	//Obtain the size of each element
	LONG lTypeSize = GetDBTypeSize(wType);
	
	//Alloc the vector...
	SAFE_ALLOC(pVector->ptr, BYTE, lTypeSize*pVector->size);

	pwsz = pwszBuffer;
	for(iEle=0; iEle<pVector->size; iEle++)
	{
		//Obtain the start and end of the value
		WCHAR* pwszEnd = wcschr(pwsz, L',');
		void* pElement = (BYTE*)pVector->ptr + (lTypeSize*iEle);

		//Convert Value to a Variant
		wcsncpy(wszBuffer, pwsz, pwszEnd ? (ULONG)(pwszEnd-pwsz)+1 : wcslen(pwsz)+1);
		wszBuffer[pwszEnd ? pwszEnd-pwsz : wcslen(pwsz)] = L'\0';
		TESTC_(hr = StringToVariant(wszBuffer, wType, &Variant),S_OK);

		//Add this to the vector...
		switch(wType)
		{
			case VT_EMPTY:
			case VT_NULL:
				break;
		
			case VT_I2:
				*(SHORT*)pElement	= V_I2(&Variant);
				break;

			case VT_I4:
			case VT_R4:
			case VT_R8:
			case VT_CY:
			case VT_DATE:
			case VT_BSTR:
			case VT_DISPATCH:
			case VT_ERROR:
			case VT_BOOL:
			case VT_UNKNOWN:
			case VT_I1:
			case VT_UI1:
			case VT_UI2:
			case VT_UI4:
			case VT_I8:
			case VT_UI8:
			case VT_INT:
			case VT_UINT:
				memcpy(pElement, &V_I4(&Variant), (size_t)lTypeSize);
				break;

			case VT_DECIMAL:
				*(DECIMAL*)pElement	= V_DECIMAL(&Variant);
				break;
			
			case VT_VARIANT:
				//just place directly into our variant.
				*(VARIANT*)pElement = Variant;
				break;
			
			default:
				//Unable to handle this type...
				TESTC_(hr = E_NOTIMPL, S_OK);
		};

		//Incement to next value...
		pwsz = wcschr(pwsz, L',');
		if(pwsz)
			pwsz += 1;
	}

CLEANUP:
	return hr;
}




//---------------------------------------------------------------------------
//	CTable::GetAccessorAndBindings
//
// @mfunc	HRESULT					|
//			CTable					|
//			GetAccessorAndBindings	|
//
// Creates binding array based on parameters passed in, and creates
// an accessor using those bindings. If there is an error, all out params will
// be NULL and return error code. If pIRowset == NULL, function returns E_FAIL.
// <nl>
// The bindings are set up such that the consumer can access each column
// of a row of data by using the DATA structure.  The DATA structure is defined
// as:
// struct DATA 
// {
//	 DBSTATUS sStatus;
//	 DBLENGTH ulLength;
//	 BYTE bValue[1];
// }; <nl>
//
// When status, length or value are not bound, those members in the structure
// should be ignored.  The user finds the place in their buffer where the
// DATA structure should begin by using the obStatus value for the desired column.<nl>
//
// ie, DATA * pStruct = pBuffer + rgBindings[ColNumber].obStatus; <nl>
// 
// Client is responsible for freeing the OUT params if they do not specify as
// NULL when passing.
//
// @rdesc HRESULT indicating success or failure
//
// @flag HRESULT   | HRESULT of the CreateAccessor call
// @flag E_FAIL    | An error occured in something other than CreateAccessor.
//
//---------------------------------------------------------------------------
HRESULT GetAccessorAndBindings(
		IUnknown *			pIUnkObject,			// @parm [IN]  Rowset, Command or RowObject to create Accessor for
		DBACCESSORFLAGS		dwAccessorFlags,		// @parm [IN]  Properties of the Accessor
		HACCESSOR *			phAccessor,				// @parm [OUT] Accessor created
		DBBINDING **		prgDBBINDINGOut,		// @parm [OUT] Array of DBBINDINGS
		DBCOUNTITEM *		pcBindingsOut,			// @parm [OUT] Count of bindings
		DBLENGTH *			pcbRowSizeOut,			// @parm [OUT] length of a row	
		DBPART				dwPart,					// @parm [IN]  Types of binding to do (Value, Status, and/or Length)	
		DWORD				dwColsToBind,			// @parm [IN]  Which columns will be used in the bindings
		ECOLUMNORDER		eBindingOrder,			// @parm [IN]  Order to bind columns in accessor												
		ECOLS_BY_REF		eColsByRef,				// @parm [IN]  Which columns to bind by reference (fixed, variable, all or none)
		DBCOLUMNINFO **		prgDBCOLUMNINFOOut,		// @parm [OUT] Array of DBCOLUMNINFO
		DBORDINAL *			pcColsOut,				// @parm [OUT] Count of Columns, also count of ColInfo elements
		WCHAR **			ppStringsBufferOut,		// @parm [OUT] ppStringsBuffer				
		DBTYPE				dwModifier,				// @parm [IN] Modifier to be OR'd with each binding type.
													// Note, this modifies each binding of the accessor and is in
													// addition to anything done by eColsByRef.  Default is DBTYPE_EMPTY,
													// which means no modifier will be used, except as specified by eColsByRef.
		DBORDINAL			cColsToBind,			// @parm [IN]  Used only if dwColsToBind = USE_COLS_TO_BIND_ARRAY
													// specifies the number of elements in rgColsToBind array
		DB_LORDINAL *		rgColsToBind,			// @parm [IN]  Used only if dwColsToBind = USE_COLS_TO_BIND_ARRAY												 
													// Each element in the array specifies the iNumber of a column to be 
													// bound.  This iNumber should be the same as that returned by iOrdinalsInfo for each column.
													// This iNumber is always ordered the same as the column list in the command 
													// specification, thus the user can use the command specification col list
													// order to determine the appropriate iNumber.
		DB_UPARAMS *		rgColOrdering,			// @parmopt [IN] Corresponds to what ordinals are specified for each binding, if 
													// the ordinal number is not the same as the column number.  For instance, 
													// if the second parameter is to be bound, but it is based on column 5, 
													// the element in rgColsToBind would be 5, but the element in rgColOrdering would be 2.
		ECOLS_MEM_PROV_OWNED  eColsMemProvOwned, 	//@paramopt [IN] Which columns' memory is to be owned by the provider
		DBPARAMIO			eParamIO,				//@paramopt [IN] Parameter kind specified for all bindings 
		BLOBTYPE			dwBlobType,				//@paramopt [IN] how to bind BLOB Columns
		DBBINDSTATUS**		prgBindStatus				//@paramopt [OUT] returned status array from CreateAccessor
)
{
	HRESULT 			hr=NOERROR;			// General HRESULT - assume success until proven otherwise
											// CreateAccessor HRESULT - assume failure until proven otherwise
	IColumnsInfo *		pIColumnsInfo=NULL;	// IColumnsInfo interface pointer
	IAccessor *			pIAccessor=NULL;	// IAccessor interface pointer
	DBORDINAL			i = 0;				// Counter for FOR Loop.
	DBORDINAL			ulColidx = 0;   	// index for DBCOLUMNINFO array
	DBCOUNTITEM			ulBindidx = 0;		// DBBindings array index
	DBCOUNTITEM			ulBindCount = 0;	// Number of bindings in array
	DBCOUNTITEM			ulLoopCounter=0;	// loop counter when building bindings
	DBBYTEOFFSET		ulOffset = 0;		// offset in DATA
	ULONG 				pulErrorBinding=0;	// count of errors
	DBCOUNTITEM			cBindings = 0;		// count of bindings in accessor				
	HACCESSOR			hAccessor = NULL;	// handle of accessor created
	DBBINDING *			rgDBBINDING = NULL;	// array of bindings for accessor
	DBBINDSTATUS*		rgBindStatus = NULL;	// array of status for accessor
	DBORDINAL			cCols;				// number of columns in rowset
	DBLENGTH			cbRowSize;			// size of one row of bound data
	DBCOLUMNINFO *		rgDBCOLUMNINFO=NULL;// array of column info
	WCHAR *				pStringsBuffer=NULL;// memory to hold string column info
	ULONG				ulColInc = 1;		// amount to increment by when looping thru col info array
	ICommandPrepare *	pICommandPrepare=NULL;
	ICommand *			pICommand = NULL;
	ULONG				cRefCounts=ULONG_MAX;// Init to a bogus count for testing purposes
	ULONG               cStorageBlobs=0;    // The current number of storage blobs bound
	IConvertType *		pIConvertType = NULL;// IConvertType interface for checking conversion support
	BOOL				fBYREFSupport = TRUE;// Column can be bound BYREF
	BOOL				fRowsetConv = FALSE;// We can call CanConvert for rowset conversions (DBCONVERTFLAGS_COLUMN)
	BOOL				fParamConv = FALSE;	// We can call CanConvert for parameter conversions (DBCONVERTFLAGS_PARAMETER)
	BOOL				fDup	= FALSE;	// USE_COLS_TO_BIND_ARRAY binds the same column twice

	// This routine doesn't have support to reversing any columns specified in rgColsToBind
	// since the user can reverse them before placing them in the array.
	if (dwColsToBind & USE_COLS_TO_BIND_ARRAY)
	{
		ASSERT(eBindingOrder != REVERSE);
	}

	// Check params
	if(pIUnkObject==NULL)
	{
		hr = E_INVALIDARG;
		goto CLEANUP;
	}

	// Need to prepare if on command object Prepare Command
	if(VerifyInterface(pIUnkObject, IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown**)&pICommandPrepare))
	{
		if(FAILED(hr = pICommandPrepare->Prepare(1)))
			goto CLEANUP;
	}

	// Find out if we're on a command or a rowset
	if(VerifyInterface(pIUnkObject, IID_ICommand, COMMAND_INTERFACE, (IUnknown**)&pICommand))
	{
		IGetDataSource * pIGetDataSource = NULL;
		IDBProperties * pIDBProperties = NULL;
		HRESULT	hr = E_FAIL;

		// We need to obtain an IConvertType interface to make sure any desired conversions are supported for BYREF
		if(!VerifyInterface(pIUnkObject,IID_IConvertType, COMMAND_INTERFACE, (IUnknown **)&pIConvertType))
		{
			hr = E_NOINTERFACE;
			goto CLEANUP;
		}

		// Get a pointer to the session object
		if (FAILED(hr=pICommand->GetDBSession(IID_IGetDataSource, (IUnknown **)&pIGetDataSource)))
			CHECK(hr, S_OK);

		if (S_OK == hr && 
			CHECK(pIGetDataSource->GetDataSource(IID_IDBProperties, (IUnknown **)&pIDBProperties), S_OK) &&
			GetProperty(DBPROP_ROWSETCONVERSIONSONCOMMAND, DBPROPSET_DATASOURCEINFO, pIDBProperties, VARIANT_TRUE))
			fParamConv = TRUE;

		SAFE_RELEASE(pICommand);
		SAFE_RELEASE(pIGetDataSource);
		SAFE_RELEASE(pIDBProperties);

	}
	else
	{
		// We need to obtain an IConvertType interface to make sure any desired conversions are supported for BYREF
		if(!VerifyInterface(pIUnkObject,IID_IConvertType, ROWSET_INTERFACE, (IUnknown **)&pIConvertType))
		{
			hr = E_NOINTERFACE;
			goto CLEANUP;
		}

		fRowsetConv = TRUE;
	}

	// IColumnsInfo->GetColumnInfo
	if(!VerifyInterface(pIUnkObject, IID_IColumnsInfo, ROWSET_INTERFACE, (IUnknown**)&pIColumnsInfo))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}
	else
	{
		if(FAILED(hr = pIColumnsInfo->GetColumnInfo(&cCols, &rgDBCOLUMNINFO, &pStringsBuffer)))
			goto CLEANUP;

		// Figure out how many of these columns we'll bind, and what index in 
		// the column array to begin using for bindings
		if (dwColsToBind == EVEN_COLS_BOUND || dwColsToBind == ODD_COLS_BOUND)
		{				
			// We will increment by two thru column array since we want every other column
			ulColInc = 2;

			if (cCols%2)
			{
				// Odd number of columns, figure out if we need to add one to (*cCount)/2
				if ((((rgDBCOLUMNINFO)[0].iOrdinal == 0) && dwColsToBind == EVEN_COLS_BOUND) ||
					(((rgDBCOLUMNINFO)[0].iOrdinal != 0) && dwColsToBind == ODD_COLS_BOUND))
				{								
					// Either we've got bookmarks and are asking for all even cols,
					// or we don't have bookmarks and are asking for odd cols				
					// We want to start with the first column in the array
					cBindings = (cCols / 2) +1;
					ulColidx = 0;
				}
				else
				{
					// Either we don't have bookmarks and are asking for even cols,
					// or we have bookmarks and are asking for odd cols
					// We want to start with the second column in the array
					cBindings = cCols / 2;
					ulColidx = 1;
				}
			}
			else
			{
				// Even number of columns, just divide by 2
				// Specify which binding to start with
				cBindings = cCols / 2;
				
				if ((dwColsToBind == EVEN_COLS_BOUND) && ((rgDBCOLUMNINFO)[0].iOrdinal == 0) ||
					(dwColsToBind == ODD_COLS_BOUND) && ((rgDBCOLUMNINFO)[0].iOrdinal != 0))	
					ulColidx = 0;
				else
					ulColidx = 1;
			}
		}
		else
		{
			// We want potentially every column, so increment by one thru column array
			ulColInc = 1;
			
			// Use number of elements in array
			// else Make room for the max, even though we won't necessarily use all these
			if (dwColsToBind & USE_COLS_TO_BIND_ARRAY)
				cBindings = cColsToBind;
			else
				cBindings = cCols;
		}

		// Build DBBINDING for CreateAccessor
		rgDBBINDING = (DBBINDING*) PROVIDER_ALLOC(sizeof(DBBINDING) * cBindings ); 
		memset(rgDBBINDING,0xCC, (size_t)(cBindings * sizeof(DBBINDING)));
								 
		// Build DBBINDSTATUS for CreateAccessor
		rgBindStatus = (DBBINDSTATUS*) PROVIDER_ALLOC(sizeof(DBBINDSTATUS) * cBindings ); 
		memset(rgBindStatus, 0xCC, (size_t)(cBindings * sizeof(DBBINDSTATUS)));

		// Loop thru col array, building bindings, incrementing by two if we're not doing all cols
		for(ulLoopCounter=0; ulColidx<cCols; ulLoopCounter++,ulColidx+=ulColInc)
		{				
			// Check for BLOB Columns
			if (rgDBCOLUMNINFO[ulColidx].dwFlags & DBCOLUMNFLAGS_ISLONG)
			{
				// Only bind BLOBs if they are requested
				if (dwBlobType & NO_BLOB_COLS)
					continue;
			
				//Try to bind BLOBs if the desired type
				if ((dwBlobType & BLOB_BIND_BINARY && rgDBCOLUMNINFO[ulColidx].wType != DBTYPE_BYTES)
					|| (dwBlobType & BLOB_BIND_STR && rgDBCOLUMNINFO[ulColidx].wType != DBTYPE_STR && rgDBCOLUMNINFO[ulColidx].wType != DBTYPE_WSTR))
				{
					//We don't want to not bind this BLOB though if its the only 
					//BLOB left in the table.  So if there are other BLOBs remaining
					//then we can skip this BLOB column.  (we want the most testing
					//and all we really trying to do is alternate which BLOBs get bound)
					BOOL fOtherBlobs = FALSE;
					for(DBORDINAL k=ulColidx+ulColInc; k<cCols && !fOtherBlobs; k+=ulColInc)
					{
						if(rgDBCOLUMNINFO[k].dwFlags & DBCOLUMNFLAGS_ISLONG)
							fOtherBlobs = TRUE;
					}

					if(fOtherBlobs)
						continue;
				}
				
				//ODBC Provider only allows 1 column to be a storage column,
				//and will not even alow 1-Storage and 1-long data
				//so skip over any other blob columns, if not already have a blob bounded
				if (cStorageBlobs && !(dwBlobType & BLOB_BIND_ALL_BLOBS) && !(dwBlobType & BLOB_BIND_ALL_COLS))
					continue;
			}
			
			//If there is already a storage object bound, and the user wants to use a forward only
			//rowset (ACCESSORDER=SEQUENTIALSTORAGEOBJECTS) then you cannot bind any other columns
			//after the storage object. (according to the spec)
			if(cStorageBlobs && (dwBlobType & BLOB_BIND_FORWARDONLY))
				continue;
			
			// Since ordinal 0 is invalid for parameter data accessors, skip it
			if ((dwAccessorFlags & DBACCESSOR_PARAMETERDATA) && 
				(rgDBCOLUMNINFO[ulColidx].iOrdinal == 0)) 
				continue;

			//Using the pased in array of columns to bind
			if(dwColsToBind & USE_COLS_TO_BIND_ARRAY)
			{
				LONG_PTR iFoundIndex = -1;

				// Look for this column in the array user passed us
				for(DBCOUNTITEM i=0; i<cColsToBind; i++)
				{
					if(rgDBCOLUMNINFO[ulColidx].iOrdinal == (DBORDINAL)rgColsToBind[i])
					{
						iFoundIndex = i;
						break;
					}
				}

				// Move to next column if we didn't find this one in array
				if(iFoundIndex == -1)
					continue;
				
				//if this is not the last column in the array and 
				//the next column is the same ordinal as this column
				//mark it as a duplicate column so it can be bound again
				if(!fDup && cColsToBind && iFoundIndex<(LONG_PTR)(cColsToBind-1) && 
					rgColsToBind[iFoundIndex] == rgColsToBind[iFoundIndex+1])
				{
					fDup=TRUE;
				}
				else
				{
					fDup=FALSE;
				}
			}
			
			//Filter any columns we don't want
			if(dwColsToBind & FIXED_LEN_COLS_BOUND)
			{
				// Skip the variable length type columns
				if (!IsFixedLength(rgDBCOLUMNINFO[ulColidx].wType))
					continue;
			}
				
			if(dwColsToBind & VARIABLE_LEN_COLS_BOUND)
			{
				// Skip the fixed length type columns
				if (IsFixedLength(rgDBCOLUMNINFO[ulColidx].wType))
					continue;						
			}
				
			if(dwColsToBind & BLOB_COLS_BOUND)
			{
				// Bind only BLOB columns
				if(!(rgDBCOLUMNINFO[ulColidx].dwFlags & DBCOLUMNFLAGS_ISLONG))
					continue;						
			}
			
			if(dwColsToBind & NONINDEX_COLS_BOUND)
			{
				// Skip the index column
				if(rgDBCOLUMNINFO[ulColidx].iOrdinal == 1)
					continue;						
			}

			if(dwColsToBind & NOBOOKMARK_COLS_BOUND)
			{	
				if(rgDBCOLUMNINFO[ulColidx].iOrdinal == 0)
					continue;
			}
				
			if(dwColsToBind & UPDATEABLE_COLS_BOUND)
			{
				// Skip the non updateable cols
				if (!(rgDBCOLUMNINFO[ulColidx].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN) &&
					!(rgDBCOLUMNINFO[ulColidx].dwFlags & DBCOLUMNFLAGS_WRITE))
					continue;						
			}

			if(dwColsToBind & NULLABLE_COLS_BOUND)
			{
				// Skip all columns excpet non-nullable columns
				if(!(rgDBCOLUMNINFO[ulColidx].dwFlags & DBCOLUMNFLAGS_ISNULLABLE))
					continue;						
			}
			
			if(dwColsToBind & NONNULLABLE_COLS_BOUND)
			{
				// Skip all columns excpet non-nullable columns
				if(rgDBCOLUMNINFO[ulColidx].dwFlags & DBCOLUMNFLAGS_ISNULLABLE)
					continue;						
			}
			
			if(dwColsToBind & VECTOR_COLS_BOUND)
			{
				// Skip all columns except vector columns
				if(!(rgDBCOLUMNINFO[ulColidx].wType & DBTYPE_VECTOR))
					continue;						
			}

			if(dwColsToBind & NOVECTOR_COLS_BOUND)
			{
				//Skip all vector columns
				if(rgDBCOLUMNINFO[ulColidx].wType & DBTYPE_VECTOR)
					continue;						
			}

			// Increment count since we'll be binding this column
			ulBindCount++;				

			// Reverse the order of bindings if REVERSE column order is requested
			// else Fill bindings in forward order, so just base 
			// index value on count of bindings done already
			if (eBindingOrder == REVERSE)							
				ulBindidx = (cBindings-1) - ulLoopCounter;									
			else
				ulBindidx = ulBindCount -1;

			ASSERT(ulBindidx < cCols);
					
			rgDBBINDING[ulBindidx].dwPart =	dwPart;
			rgDBBINDING[ulBindidx].eParamIO = eParamIO;
			rgDBBINDING[ulBindidx].iOrdinal = rgDBCOLUMNINFO[ulColidx].iOrdinal; 
			rgDBBINDING[ulBindidx].pBindExt = NULL; 
			rgDBBINDING[ulBindidx].dwFlags = 0; 

			// Either make client or provider own all memory
			// Long columns must have client owned memory
			if (eColsMemProvOwned == NO_COLS_OWNED_BY_PROV || 
				(eColsMemProvOwned == SUPPORTED_COLS_OWNED_BY_PROV && 
				(rgDBCOLUMNINFO[ulColidx].dwFlags & DBCOLUMNFLAGS_ISLONG)))
				rgDBBINDING[ulBindidx].dwMemOwner = DBMEMOWNER_CLIENTOWNED;					
			else
				rgDBBINDING[ulBindidx].dwMemOwner = DBMEMOWNER_PROVIDEROWNED;
			
			// Only fill in precision in scale for numeric and decimal types
			// since these are the only ones to which it applies
			if (rgDBCOLUMNINFO[ulColidx].wType == DBTYPE_NUMERIC ||
				rgDBCOLUMNINFO[ulColidx].wType == DBTYPE_DECIMAL)	
			{
				rgDBBINDING[ulBindidx].bPrecision = rgDBCOLUMNINFO[ulColidx].bPrecision;
				rgDBBINDING[ulBindidx].bScale = rgDBCOLUMNINFO[ulColidx].bScale;
			}

			//We natively bind the column type (same as column info)
			//No exceptions to this since all types supported by the provider must be able
			//to be bound in the native type, including bookmarks...
			rgDBBINDING[ulBindidx].wType = rgDBCOLUMNINFO[ulColidx].wType;

			// BLOBs Setup the pObject if required by dwBlobType				
			BOOL fBindAsStorage = TRUE;
			rgDBBINDING[ulBindidx].pObject = NULL;

			//If not a BLOB column, we obviously can't bind as a Storage Object
			//Unless the user has specifically asked for this BLOB_BIND_ALL_COLS
			if (!(rgDBCOLUMNINFO[ulColidx].dwFlags & DBCOLUMNFLAGS_ISLONG) && !(dwBlobType & BLOB_BIND_ALL_COLS))
				fBindAsStorage = FALSE;	

			// If not requesting Storage Objects, we don't need to bind it
			if (dwBlobType & NO_BLOB_COLS)
				fBindAsStorage = FALSE;

			// If only want long columns and nothing else, don't bind as storage
			if (dwBlobType == BLOB_LONG)
				fBindAsStorage = FALSE;

			// If only want 1 blob, don't bind more than 1
			if (cStorageBlobs && !(dwBlobType & BLOB_BIND_ALL_BLOBS) && !(dwBlobType & BLOB_BIND_ALL_COLS)) 
				fBindAsStorage = FALSE;
							
			// Otherwise, bind this BLOB Column as a Storage Object
			// Also if this column is natively DBTYPE_IUNKNOWN, 
			// we need to setup the pObject (for IColumnsRowset pTypeInfo case)
			if (fBindAsStorage || rgDBBINDING[ulBindidx].wType == DBTYPE_IUNKNOWN)
			{
				// Count stroage BLOB as being bound
				cStorageBlobs++;

				// Setup Storage Object type
				rgDBBINDING[ulBindidx].wType   = DBTYPE_IUNKNOWN;
				
				if (!(dwBlobType & BLOB_NULL_POBJECT))
				{
					rgDBBINDING[ulBindidx].pObject = (DBOBJECT*) PROVIDER_ALLOC(sizeof(DBOBJECT));
				
					// Setup pObject->iid
					if (dwBlobType & BLOB_IID_IUNKNOWN)
						rgDBBINDING[ulBindidx].pObject->iid = IID_IUnknown;
					else if (dwBlobType & BLOB_IID_NULL)
						rgDBBINDING[ulBindidx].pObject->iid = IID_NULL;
					else if (dwBlobType & BLOB_IID_ISTREAM)
						rgDBBINDING[ulBindidx].pObject->iid = IID_IStream;
					else if (dwBlobType & BLOB_IID_ISTORAGE)
						rgDBBINDING[ulBindidx].pObject->iid = IID_IStorage;
					else if (dwBlobType & BLOB_IID_ILOCKBYTES)
						rgDBBINDING[ulBindidx].pObject->iid = IID_ILockBytes;
					else //IID_ISequentialStream by default
						rgDBBINDING[ulBindidx].pObject->iid = IID_ISequentialStream;

					// Setup pObject dwFlags, READ by default 
					rgDBBINDING[ulBindidx].pObject->dwFlags = STGM_READ;
					
					if (dwBlobType & BLOB_STGM_WRITE)
						rgDBBINDING[ulBindidx].pObject->dwFlags |= STGM_WRITE;
					if (dwBlobType & BLOB_STGM_DIRECT)
						rgDBBINDING[ulBindidx].pObject->dwFlags |= STGM_DIRECT;
					if (dwBlobType & BLOB_STGM_TRANSACTED)
						rgDBBINDING[ulBindidx].pObject->dwFlags |= STGM_TRANSACTED;
					if (dwBlobType & BLOB_STGM_INVALID)
						rgDBBINDING[ulBindidx].pObject->dwFlags |= ULONG_MAX;
				}
			}

			// Or on the modifier for this data type, if specified
			if (dwModifier != DBTYPE_EMPTY)
				rgDBBINDING[ulBindidx].wType |= dwModifier;

			// See if a BYREF conversion can be supported on this column for the desired accessor type
			if (eColsByRef == SUPPORTED_COLS_BY_REF ||
				eColsByRef == SUPPORTED_FIXED_LEN_COLS_BY_REF ||
				eColsMemProvOwned == SUPPORTED_COLS_OWNED_BY_PROV)
			{
				fBYREFSupport = TRUE;
				if (dwAccessorFlags & DBACCESSOR_PARAMETERDATA && fParamConv)
				{
					if (FAILED(hr = pIConvertType->CanConvert(rgDBBINDING[ulBindidx].wType|DBTYPE_BYREF,
						(rgDBCOLUMNINFO[ulColidx].wType & ~(DBTYPE_ARRAY | DBTYPE_VECTOR)), DBCONVERTFLAGS_PARAMETER)))
						goto CLEANUP;
					if (S_FALSE == hr)
						fBYREFSupport = FALSE;
				}

				if (dwAccessorFlags & DBACCESSOR_ROWDATA && fRowsetConv)
				{
					if (FAILED(hr = pIConvertType->CanConvert((rgDBCOLUMNINFO[ulColidx].wType & ~(DBTYPE_ARRAY | DBTYPE_VECTOR)),
							rgDBBINDING[ulBindidx].wType|DBTYPE_BYREF, DBCONVERTFLAGS_COLUMN)))
						goto CLEANUP;
					if (S_FALSE == hr)
						fBYREFSupport = FALSE;
				}
			}

			// Check if type needs to be or'ed with BY_REF
			switch (eColsByRef)
			{
				case NO_COLS_BY_REF:
					// Do nothing
					break;

				case ALL_COLS_BY_REF:
					// Make every binding BYREF except the disallowed VECTOR and ARRAY types
					if (!(rgDBBINDING[ulBindidx].wType & DBTYPE_VECTOR) &&
						!(rgDBBINDING[ulBindidx].wType & DBTYPE_ARRAY))
						rgDBBINDING[ulBindidx].wType |= DBTYPE_BYREF;
					break;

				case FIXED_LEN_COLS_BY_REF:
					// Bind only fixed length typed columns BYREF
					if (IsFixedLength(rgDBCOLUMNINFO[ulColidx].wType) &&
						!(rgDBCOLUMNINFO[ulColidx].wType & DBTYPE_VECTOR) &&
						!(rgDBCOLUMNINFO[ulColidx].wType & DBTYPE_ARRAY))
						rgDBBINDING[ulBindidx].wType |= DBTYPE_BYREF;
					break;

				case VARIABLE_LEN_COLS_BY_REF:
					// Bind only variable length typed columns BYREF.  This should always
					// be supported.
					if (!IsFixedLength(rgDBCOLUMNINFO[ulColidx].wType))
						rgDBBINDING[ulBindidx].wType |= DBTYPE_BYREF;					
					break;

				case SUPPORTED_COLS_BY_REF:
					// Make every supported binding BYREF except the disallowed VECTOR and ARRAY types
					if (!(rgDBBINDING[ulBindidx].wType & DBTYPE_VECTOR) &&
						!(rgDBBINDING[ulBindidx].wType & DBTYPE_ARRAY) &&
						fBYREFSupport)
						rgDBBINDING[ulBindidx].wType |= DBTYPE_BYREF;
					break;
				case SUPPORTED_FIXED_LEN_COLS_BY_REF:
					// Bind only supported fixed length typed columns BYREF
					if (IsFixedLength(rgDBCOLUMNINFO[ulColidx].wType) &&
						!(rgDBCOLUMNINFO[ulColidx].wType & DBTYPE_VECTOR) &&
						!(rgDBCOLUMNINFO[ulColidx].wType & DBTYPE_ARRAY) &&
						fBYREFSupport)
						rgDBBINDING[ulBindidx].wType |= DBTYPE_BYREF;
					break;

				default:
					// We have a new value of ECOLS_BY_REF we haven't accounted for
					ASSERT(FALSE);
			}

			// Since it's illegal to specify provider-owned memory for columns that don't
			// support BYREF then change back to client-owned.  BSTR can be provider-owned
			// without BYREF.
			if (eColsMemProvOwned == SUPPORTED_COLS_OWNED_BY_PROV &&
				!fBYREFSupport &&
				!((rgDBBINDING[ulBindidx].wType & DBTYPE_BYREF) || 
				(rgDBCOLUMNINFO[ulColidx].wType & ~(DBTYPE_ARRAY | DBTYPE_VECTOR)) == DBTYPE_BSTR))
				rgDBBINDING[ulBindidx].dwMemOwner = DBMEMOWNER_CLIENTOWNED;

			// pTypeInfo
			rgDBBINDING[ulBindidx].pTypeInfo = rgDBCOLUMNINFO[ulColidx].pTypeInfo;
			
			// Only do this if we're binding the value
			if (dwPart & DBPART_VALUE)
			{
				rgDBBINDING[ulBindidx].obValue = 	ulOffset + offsetof(DATA,bValue);
				
				if(rgDBCOLUMNINFO[ulColidx].wType & DBTYPE_BYREF)
				{
					//DBTYPE_BYREF
					rgDBBINDING[ulBindidx].cbMaxLen = sizeof(void *);
				}
				else if(rgDBCOLUMNINFO[ulColidx].wType & DBTYPE_VECTOR)
				{
					//DBTYPE_VECTOR
					rgDBBINDING[ulBindidx].cbMaxLen = sizeof(DBVECTOR);
				}
				else if(rgDBCOLUMNINFO[ulColidx].wType & DBTYPE_ARRAY)
				{
					//DBTYPE_ARRAY
					rgDBBINDING[ulBindidx].cbMaxLen = sizeof(SAFEARRAY*);
				}
				else
				{
					// Use a max if the column type is larger than it
					// If a IColumnsInfo reports a column size at > MAX_COL_SIZE
					// Chances are that it is a BLOB or PrivLib didn't create the table.
					// Be generous in this case and bind to MAXDATALEN
					//  Be generous, but don't set cbMaxLength too large (like MAXDATALEN)
					// if column size is still within the range (0, MAXDATALEN) set it
					// to rgCOLUMNINFO.ulColumnSize
					if (rgDBBINDING[ulBindidx].wType == DBTYPE_IUNKNOWN)
						rgDBBINDING[ulBindidx].cbMaxLen = sizeof(IUnknown*);
					else if (((~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR) & rgDBBINDING[ulBindidx].wType) == DBTYPE_WSTR))
						rgDBBINDING[ulBindidx].cbMaxLen = min(MAXDATALEN, rgDBCOLUMNINFO[ulColidx].ulColumnSize) * sizeof(WCHAR);
						// Why allocate too much space. Sure, use MAXDATALEN as upper limit, but if columnsize
						// is below MAXDATALEN then whats the use of giving more memory
						// (rgDBCOLUMNINFO[ulColidx].ulColumnSize > MAX_COL_SIZE) ?
						// MAXDATALEN * sizeof(WCHAR) : rgDBCOLUMNINFO[ulColidx].ulColumnSize * sizeof(WCHAR);
					else
						rgDBBINDING[ulBindidx].cbMaxLen = min(MAXDATALEN, rgDBCOLUMNINFO[ulColidx].ulColumnSize);
						//(rgDBCOLUMNINFO[ulColidx].ulColumnSize > MAX_COL_SIZE) ?
						//MAXDATALEN : rgDBCOLUMNINFO[ulColidx].ulColumnSize;

					// Add room for the null terminator on string data
					if (((~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR) &	//Remove all qualifiers from type
						rgDBBINDING[ulBindidx].wType) == DBTYPE_STR))
						rgDBBINDING[ulBindidx].cbMaxLen += sizeof(CHAR);
					else 
						if (((~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR) &
							rgDBBINDING[ulBindidx].wType) == DBTYPE_WSTR))
							rgDBBINDING[ulBindidx].cbMaxLen += sizeof(WCHAR);					
				}			
			}

			// Only set length offset if we're binding length
			if (dwPart & DBPART_LENGTH)
				rgDBBINDING[ulBindidx].obLength = ulOffset + offsetof(DATA,ulLength);

			// Only set status offset if we're binding status
			if (dwPart & DBPART_STATUS)
				rgDBBINDING[ulBindidx].obStatus = ulOffset + offsetof(DATA,sStatus);
			
			// Increment our offset to account for the size of our bound structure
			ulOffset += sizeof(DATA);
							
			// If we bound the value, we need to compensate for any extra room 
			// the value may have taken over the bValue size allocated in the struct
			// Also compensate for a null terminator on impt params
			if (dwPart & DBPART_VALUE)
			{
				if (!(rgDBBINDING[ulBindidx].wType & DBTYPE_BYREF))
				{
					if( (rgDBBINDING[ulBindidx].cbMaxLen > sizeof(DBLENGTH)) &&
						((~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR) & rgDBBINDING[ulBindidx].wType) == DBTYPE_STR) &&
						(eParamIO == DBPARAMIO_INPUT) )
						ulOffset += ((rgDBBINDING[ulBindidx].cbMaxLen - sizeof(DBLENGTH)) + sizeof(CHAR));
					else if ((rgDBBINDING[ulBindidx].cbMaxLen > sizeof(DBLENGTH)) &&
						((~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR) & rgDBBINDING[ulBindidx].wType) == DBTYPE_WSTR) &&
						(eParamIO == DBPARAMIO_INPUT) )
						ulOffset += ((rgDBBINDING[ulBindidx].cbMaxLen - sizeof(DBLENGTH)) + sizeof(WCHAR));
					else
						ulOffset += rgDBBINDING[ulBindidx].cbMaxLen - sizeof(DBLENGTH);
				}
			}

			// Make sure our structure begins on a correct byte alignment
			ulOffset = ROUND_UP(ulOffset, ROUND_UP_AMOUNT);

			if (fDup)
			{
				ulColidx--;
			}
		}
		
		// If we aren't binding even or odd columns, cBindings is really count of columns,
		// adjust it to what actually got bound.
		cBindings = ulBindCount;
		cbRowSize = ulOffset;
		  
		// We've created bindings based on the column type
		// in rgColsToBind (and thus made each binding ordinal
		// match that col number), now we need to fix up
		// the binding ordinal to be a parameter ordinal,
		// so use rgColOrdering to do so.
		if (dwAccessorFlags & DBACCESSOR_PARAMETERDATA)
		{
			//We deviated from the standard all cols bound
			for(i = 0; i<cBindings; i++)						
			{
				if(rgColOrdering &&	dwColsToBind==USE_COLS_TO_BIND_ARRAY)
					rgDBBINDING[i].iOrdinal = (DBORDINAL)rgColOrdering[i];
				else
					rgDBBINDING[i].iOrdinal = i+1;
			}
		}
				
		//If the user wants to create the accessor
		//This maybe NULL if the user only wants the bindings generated, or is 
		//creating bindings for the row object, which doesn't have IAccessor interface...
		if(phAccessor)
		{
			if(!VerifyInterface(pIUnkObject, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor))
			{ 
				hr = E_NOINTERFACE;
				goto CLEANUP;
			}

			//IAccessor::CreateAccessor
			hr = pIAccessor->CreateAccessor(dwAccessorFlags, cBindings,
										rgDBBINDING, cbRowSize, phAccessor, rgBindStatus);

			if(SUCCEEDED(hr))
			{
				//Presently, S_OK is the only success return code
				//for CreateAccessor.
				GCHECK(hr, S_OK);
				for(i=0; i<cBindings; i++)
					COMPAREW(rgBindStatus[i], DBBINDSTATUS_OK);
			}
			else if(hr==DB_E_ERRORSOCCURRED)
			{
				BOOL	bFoundNotOK = FALSE;
				//At least one status will be NOT OK.
				for(i=0; i<cBindings; i++)
				{
					if(rgBindStatus[i] != DBBINDSTATUS_OK)
						bFoundNotOK = TRUE;
				}
				GCOMPARE(bFoundNotOK, TRUE);
			}
		}
	}	

CLEANUP:
	if(cBindings==0)
		PROVIDER_FREE(rgDBBINDING);

	if(prgDBBINDINGOut)
		*prgDBBINDINGOut = rgDBBINDING;
	else
		FreeAccessorBindings(cBindings,rgDBBINDING);

	if(prgBindStatus)
		*prgBindStatus = rgBindStatus;
	else
		PROVIDER_FREE(rgBindStatus);

	if(pcBindingsOut)
		*pcBindingsOut = cBindings;
	
	if(pcbRowSizeOut)
		*pcbRowSizeOut = cbRowSize;
	
	if(prgDBCOLUMNINFOOut)
		*prgDBCOLUMNINFOOut = rgDBCOLUMNINFO;
	else
		PROVIDER_FREE(rgDBCOLUMNINFO);

	if(pcColsOut)
		*pcColsOut = cCols;

	if(ppStringsBufferOut)
		*ppStringsBufferOut = pStringsBuffer;
	else
		PROVIDER_FREE(pStringsBuffer);
			
	// Release interfaces
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pICommandPrepare);
	SAFE_RELEASE(pIConvertType);
	return hr;
}



//----------------------------------------------------------------------
// MiscFunc VerifyColAccess |
// Verify DBCOLUMNACCESS structures and Display any error binding status
// 
// @mfunc BOOL|
// 
//----------------------------------------------------------------------
BOOL VerifyColAccess(HRESULT hrReturned, DBORDINAL cColAccess, DBCOLUMNACCESS* rgColAccess)
{
	ULONG_PTR cErrors = 0;
	
	for(DBORDINAL i=0; i<cColAccess && rgColAccess; i++)
	{
		BOOL fDisplay = FALSE;
		if(hrReturned==S_OK || hrReturned==DB_S_ERRORSOCCURRED || hrReturned==DB_E_ERRORSOCCURRED)
		{
			DBSTATUS dwStatus = rgColAccess[i].dwStatus;

			//If status was success, just skip over this column
			switch(dwStatus)
			{
				case DBSTATUS_S_OK:
				case DBSTATUS_S_ISNULL:
				case DBSTATUS_S_IGNORE:
				case DBSTATUS_S_DEFAULT:
				case DBSTATUS_S_TRUNCATED:
				case DBSTATUS_S_ALREADYEXISTS:		//Returned from IRowSchemaChange::AddColumns
					//All of these are considered succeess (S_OK)
					break;

				default:
					cErrors++;
					fDisplay = TRUE;
					break;
			}
		}
		else
		{
			//We got some other error, display all bindings, 
			//since we don't know which ones are in error.
			fDisplay = TRUE;
		}

		if(fDisplay)
		{													
			TRACE(L"  rgColAccess[%d].columnid   = \"%s\"\n", i , (rgColAccess[i].columnid.eKind == DBKIND_NAME || rgColAccess[i].columnid.eKind ==DBKIND_GUID_NAME || rgColAccess[i].columnid.eKind == DBKIND_PGUID_NAME) ? rgColAccess[i].columnid.uName.pwszName : L"Unknown");
			TRACE(L"  rgColAccess[%d].wType      = %s\n", i , GetDBTypeName(rgColAccess[i].wType));
			TRACE(L"  rgColAccess[%d].dwStatus   = %s\n", i , GetStatusName(rgColAccess[i].dwStatus));
			TRACE(L"  rgColAccess[%d].cbDataLen  = %d\n", i , rgColAccess[i].cbDataLen);
			TRACE(L"  rgColAccess[%d].cbMaxLen   = %d\n", i , rgColAccess[i].cbMaxLen);
			TRACE(L"  rgColAccess[%d].bPrecision = %d\n", i , rgColAccess[i].bPrecision);
			TRACE(L"  rgColAccess[%d].bScale     = %d\n", i , rgColAccess[i].bScale);
		}
	}

	//Make sure the return code matches that status'
	if(hrReturned == S_OK)
		return cErrors == 0;
	if(hrReturned == DB_S_ERRORSOCCURRED)

		// return (cErrors >= 0 && cErrors < cColAccess);
		// Fix for new compile tools
		return ((cErrors>0 || cErrors==0) && cErrors < cColAccess);

	if(hrReturned == DB_E_ERRORSOCCURRED)
		return cErrors == cColAccess;

	return TRUE;
}

//----------------------------------------------------------------------
// MiscFunc VerifyBindings |
// Verify Bindings and Display any error binding status
// 
// @mfunc BOOL|
// 
//----------------------------------------------------------------------
BOOL VerifyBindings(HRESULT hrReturned, DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData)
{
	ULONG cErrors = 0;
	DBCOUNTITEM cStatusBound = 0;

	for(DBCOUNTITEM i=0; i<cBindings && rgBindings; i++)
	{
		BOOL fDisplay = FALSE;
		if(hrReturned==S_OK || hrReturned==DB_S_ERRORSOCCURRED || hrReturned==DB_E_ERRORSOCCURRED)
		{
			//If status is not bound
			if(!STATUS_IS_BOUND(rgBindings[i]))
				continue;
			cStatusBound++;

			//If status was success, just skip over this column
			switch(STATUS_BINDING(rgBindings[i], pData))
			{
				case DBSTATUS_S_OK:
				case DBSTATUS_S_ISNULL:
				case DBSTATUS_S_IGNORE:
				case DBSTATUS_S_DEFAULT:
				case DBSTATUS_S_TRUNCATED:
					//All of these are considered succeess (S_OK)
					break;

				default:
					cErrors++;
					fDisplay = TRUE;
					break;
			}
		}
		else
		{
			//We got some other error, display all bindings, 
			//since we don't know which ones are in error.
			fDisplay = TRUE;
		}

		if(fDisplay)
		{													
			TRACE(L"  rgBindings[%d].iOrdinal   = %d\n", i, rgBindings[i].iOrdinal);
			TRACE(L"  rgBindings[%d].wType      = %s\n", i, GetDBTypeName(rgBindings[i].wType));
			
			TRACE(L"  rgBindings[%d].obStatus   = 0x%08x = &%s\n", i , rgBindings[i].obStatus, STATUS_IS_BOUND(rgBindings[i]) ? GetStatusName(STATUS_BINDING(rgBindings[i], pData)) : L"Not Bound");
			TRACE(L"  rgBindings[%d].obLength   = 0x%08x = &%d %s\n", i , rgBindings[i].obLength, LENGTH_IS_BOUND(rgBindings[i]) ? LENGTH_BINDING(rgBindings[i], pData) : 0, LENGTH_IS_BOUND(rgBindings[i]) ? L"" : L"Not Bound");
			TRACE(L"  rgBindings[%d].obValue    = 0x%08x = &0x%08x %s\n", i , rgBindings[i].obValue, VALUE_IS_BOUND(rgBindings[i]) ? VALUE_BINDING(rgBindings[i], pData) : 0, VALUE_IS_BOUND(rgBindings[i]) ? L"" : L"Not Bound");

			TRACE(L"  rgBindings[%d].pTypeInfo  = 0x%08x\n", i , rgBindings[i].pTypeInfo);
			TRACE(L"  rgBindings[%d].pObject    = 0x%08x\n", i , rgBindings[i].pObject);
			TRACE(L"  rgBindings[%d].pBindExt   = 0x%08x\n", i , rgBindings[i].pBindExt);

			TRACE(L"  rgBindings[%d].dwPart     = 0x%08x\n", i , rgBindings[i].dwPart);
			TRACE(L"  rgBindings[%d].dwMemOwner = 0x%08x\n", i , rgBindings[i].dwMemOwner);
			TRACE(L"  rgBindings[%d].eParamIO   = 0x%08x\n", i , rgBindings[i].eParamIO);

			TRACE(L"  rgBindings[%d].cbMaxLen   = %d\n", i , rgBindings[i].cbMaxLen);
			TRACE(L"  rgBindings[%d].bPrecision = %d\n", i , rgBindings[i].bPrecision);
			TRACE(L"  rgBindings[%d].bScale     = %d\n", i , rgBindings[i].bScale);
		}
	}
	
	//Make sure the return code matches that status'
	//NOTE: We have to keep track of the number of status' bound, since the column that doesn't
	//have the status bound could contain the error.
	if(hrReturned == S_OK)
		return cErrors == 0;
	if(hrReturned == DB_S_ERRORSOCCURRED)
		return (cErrors < cBindings) || (cStatusBound < cBindings);
	if(hrReturned == DB_E_ERRORSOCCURRED)
		return (cErrors == cBindings) || (cStatusBound < cBindings);
	
	return TRUE;
}



//----------------------------------------------------------------------
// MiscFunc VerifyPropertiesInError |
// Verify Properties and Display any error property status
// 
// @mfunc BOOL|
// 
//----------------------------------------------------------------------
BOOL VerifyPropertiesInError(HRESULT hrReturned, IUnknown* pIUnknown)
{
	TBEGIN
	ASSERT(pIUnknown);
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	IDBProperties* pIDBProperties = NULL;
	ICommandProperties* pICommandProperties = NULL;
	ISessionProperties* pISessionProperties = NULL;

	//Setup input DBPROPSET_PROPERTIESINERROR
	const ULONG cPropertyIDSets = 1;
	DBPROPIDSET rgPropertyIDSets[cPropertyIDSets];
	rgPropertyIDSets[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;
	rgPropertyIDSets[0].cPropertyIDs = 0;
	rgPropertyIDSets[0].rgPropertyIDs = NULL;

	//Determine which interface the pointer is...
	if(SUCCEEDED(pIUnknown->QueryInterface(IID_ICommandProperties, (void**)&pICommandProperties)))
	{
		//ICommand::GetProperties 
		TESTC_(pICommandProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets),S_OK);
	}
	else if(SUCCEEDED(pIUnknown->QueryInterface(IID_IDBProperties, (void**)&pIDBProperties)))
	{
		//IDBProperties::GetProperties 
		TESTC_(pIDBProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets),S_OK);
	}
	else if(SUCCEEDED(pIUnknown->QueryInterface(IID_ISessionProperties, (void**)&pISessionProperties)))
	{
		//ISessionProperties::GetProperties 
		TESTC_(pISessionProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets),S_OK);
	}
	else
	{
				
	}
	
	//Now delegate to verify all properties in error
	TESTC(VerifyProperties(hrReturned, cPropSets, rgPropSets));

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pICommandProperties);
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pISessionProperties);
	TRETURN
}


//----------------------------------------------------------------------
// MiscFunc VerifyProperties |
// Verify Properties and Display any error property status
// 
// @mfunc BOOL|
// 
//----------------------------------------------------------------------
BOOL VerifyProperties(HRESULT hrReturned, ULONG cPropSets, DBPROPSET* rgPropSets, BOOL fOpenRowset, BOOL fAlwaysTrace)
{
	ULONG cErrors = 0;
	ULONG cWarnings = 0;
	ULONG cTotalProperties = 0;

	//NOTE:  Methods that take property parameters, like: IOpenRowset will return a warning 
	//DB_S_ERRORSOCCURRED if all optional properties were not set, and DB_E_ERRROSOCCURRED if
	//any required properties are not set.  Other methods, like ICommandProperties::SetProperties 
	//will return an error if only if all properties were not set, and a warning otherwise...

	//Loop through the property sets...
	for(ULONG iPropSet=0; iPropSet<cPropSets && rgPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];

		//Loop through the properties within this set...
		for(ULONG iProp=0; iProp<pPropSet->cProperties && pPropSet->rgProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			cTotalProperties++;

			BOOL fDisplay = FALSE;
			if(hrReturned==S_OK || hrReturned==DB_S_ERRORSOCCURRED || hrReturned==DB_E_ERRORSOCCURRED)
			{
				//If status was success, just skip over this column
				switch(pProp->dwStatus)
				{
					case DBPROPSTATUS_OK:
						//All of these are considered succeess (S_OK)
						break;

					default:
						if(pProp->dwOptions == DBPROPOPTIONS_OPTIONAL)
						{
							//All of these are considered warnings (OPTIONAL)
							cWarnings++;
						}
						else
						{
							//All of these are considered errors (REQUIRED)
							cErrors++;
						}
						
						fDisplay = TRUE;
						break;
				}
			}
			else
			{
				//We got some other error, display all bindings, 
				//since we don't know which ones are in error.
				fDisplay = TRUE;
			}

			if(fDisplay || fAlwaysTrace)
			{													
				WCHAR wszBuffer[MAXDATALEN];
				VariantToString(&pProp->vValue, wszBuffer, MAXDATALEN, FALSE);

				TRACE(L"  Privlib:  %s\n", GetPropSetName(pPropSet->guidPropertySet));
				TRACE(L"  Privlib:     rgPropSets[%d].rgProperties[%d].dwPropertyID  = %d = %s\n", iPropSet, iProp, pProp->dwPropertyID, GetPropertyName(pProp->dwPropertyID, pPropSet->guidPropertySet));
				TRACE(L"  Privlib:     rgPropSets[%d].rgProperties[%d].dwOptions     = %d = %s\n", iPropSet, iProp, pProp->dwOptions, pProp->dwOptions == DBPROPOPTIONS_REQUIRED ? L"DBPROPOPTIONS_REQUIRED" : pProp->dwOptions == DBPROPOPTIONS_OPTIONAL ? L"DBPROPOPTIONS_OPTIONAL" : L"UNKNOWN??");
				TRACE(L"  Privlib:     rgPropSets[%d].rgProperties[%d].dwStatus      = %d = %s\n", iPropSet, iProp, pProp->dwStatus, GetPropStatusName(pProp->dwStatus));
				TRACE(L"  Privlib:     rgPropSets[%d].rgProperties[%d].colid         = XX = XX\n", iPropSet, iProp);

				TRACE(L"  Privlib:     rgPropSets[%d].rgProperties[%d].vValue.vt     = %d = %s\n", iPropSet, iProp, pProp->vValue.vt, GetDBTypeName(pProp->vValue.vt));
				TRACE(L"  Privlib:     rgPropSets[%d].rgProperties[%d].vValue        = %s\n", iPropSet, iProp, wszBuffer);
			}
		}
	}
	
	//Make sure the return code matches that status'
	if(hrReturned == S_OK)
		return (cErrors == 0 && cWarnings == 0);

	//NOTE:  Some methods, like: IOpenRowset will return a warning DB_S_ERRORSOCCURRED if all 
	//optional properties were not set, other methods, ICommandProperties will return an error 
	//if all properties were not set...
	if(fOpenRowset)
	{
		if(hrReturned == DB_E_ERRORSOCCURRED)
			return (cErrors > 0);
		if(hrReturned == DB_S_ERRORSOCCURRED)
			return (cErrors == 0 && cWarnings > 0);
	}
	else
	{
		if(hrReturned == DB_E_ERRORSOCCURRED)
			return ((cErrors + cWarnings) == cTotalProperties);
		if(hrReturned == DB_S_ERRORSOCCURRED)
			return ((cErrors + cWarnings) > 0 && (cErrors + cWarnings) < cTotalProperties);
	}
	
	return TRUE;
}


//----------------------------------------------------------------------
// MiscFunc VerifyPropertyInfo |
// Verify PropertyInfo and Display any error property status
// 
// @mfunc BOOL|
// 
//----------------------------------------------------------------------
BOOL VerifyPropertyInfo(HRESULT hrReturned, ULONG cPropInfoSets, DBPROPINFOSET* rgPropInfoSets, OLECHAR* pDescBuffer, BOOL fAlwaysTrace)
{
	ULONG cErrors = 0;
	ULONG cTotalProperties = 0;

	//Loop through the property sets...
	for(ULONG iPropSet=0; iPropSet<cPropInfoSets && rgPropInfoSets; iPropSet++)
	{
		DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[iPropSet];

		//Loop through the properties within this set...
		for(ULONG iProp=0; iProp<pPropInfoSet->cPropertyInfos && pPropInfoSet->cPropertyInfos; iProp++)
		{
			DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[iProp];
			cTotalProperties++;

			BOOL fDisplay = FALSE;
			if(hrReturned==S_OK || hrReturned==DB_S_ERRORSOCCURRED || hrReturned==DB_E_ERRORSOCCURRED)
			{
				//If status was success, just skip over this column
				if(pPropInfo->dwFlags == DBPROPFLAGS_NOTSUPPORTED)
				{
					COMPARE(pPropInfo->pwszDescription, NULL);
					COMPARE(pPropInfo->vtType == VT_EMPTY, TRUE);
					COMPARE(pPropInfo->vValues.vt == VT_EMPTY, TRUE);
					
					cErrors++;
					fDisplay = TRUE;
				}
				else
				{
					COMPARE(!!pPropInfo->pwszDescription, TRUE);
					COMPARE(pPropInfo->vtType != VT_EMPTY, TRUE);
				}
			}
			else
			{
				//We got some other error, display all Properties, 
				//since we don't know which ones are in error.
				fDisplay = TRUE;
			}

			if(fDisplay || fAlwaysTrace)
			{													
				WCHAR wszBuffer[MAXDATALEN];
				VariantToString(&pPropInfo->vValues, wszBuffer, MAXDATALEN, FALSE);

				TRACE(L"  Privlib:  %s\n", GetPropSetName(pPropInfoSet->guidPropertySet));
				TRACE(L"  Privlib:     rgPropInfoSets[%d].rgPropertyInfos[%d].pwszDescription  = \"%s\"\n", iPropSet, iProp, pPropInfo->pwszDescription);
				TRACE(L"  Privlib:     rgPropInfoSets[%d].rgPropertyInfos[%d].dwPropertyID     = %d = %s\n", iPropSet, iProp, pPropInfo->dwPropertyID, GetPropertyName(pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet));
				TRACE(L"  Privlib:     rgPropInfoSets[%d].rgPropertyInfos[%d].dwFlags          = 0x%08x\n", iPropSet, iProp, pPropInfo->dwFlags);

				TRACE(L"  Privlib:     rgPropInfoSets[%d].rgPropertyInfos[%d].vtType           = %d = %s\n", iPropSet, iProp, pPropInfo->vtType, GetDBTypeName(pPropInfo->vtType));
				TRACE(L"  Privlib:     rgPropInfoSets[%d].rgPropertyInfos[%d].vValues.vt       = %d = %s\n", iPropSet, iProp, pPropInfo->vValues.vt, GetDBTypeName(pPropInfo->vValues.vt));
				TRACE(L"  Privlib:     rgPropInfoSets[%d].rgPropertyInfos[%d].vValues          = %s\n", iPropSet, iProp, wszBuffer);
			}
		}
	}
	
	//Make sure the return code matches that status'
	if(hrReturned == S_OK)
	{
		COMPARE(!!pDescBuffer, TRUE);
		return (cErrors == 0);
	}

	if(hrReturned == DB_E_ERRORSOCCURRED)
	{
		COMPARE(pDescBuffer, NULL);
		return (cErrors == cTotalProperties);
	}
	
	if(hrReturned == DB_S_ERRORSOCCURRED)
	{
		COMPARE(!!pDescBuffer, TRUE);
		return (cErrors > 0 && cErrors < cTotalProperties);
	}
	
	return TRUE;
}


//----------------------------------------------------------------------
// MiscFunc FreeAccessorBindings |
// Frees the memory assoicated with rgBindings
// 
// @mfunc HRESULT|
// 
//----------------------------------------------------------------------
HRESULT FreeAccessorBindings(
	DBCOUNTITEM 		cBindings,				// @parmopt [IN] Count of bindings
	DBBINDING*	rgBindings				// @parmopt [IN] Array of DBBINDINGS
)
{
	// Loop through bindings and free the pObject first
	for(DBCOUNTITEM i=0; i<cBindings; i++)
	{
		ASSERT(rgBindings);
		PROVIDER_FREE(rgBindings[i].pObject);
	}

	PROVIDER_FREE(rgBindings);
	return S_OK;
}


//----------------------------------------------------------------------
// MiscFunc FreeColAccess |
// Frees the memory assoicated with rgColAccess
// 
// @mfunc HRESULT|
// 
//----------------------------------------------------------------------
HRESULT FreeColAccess(
	DBORDINAL		cColAccess,				// @parmopt [IN] Count of ColAccess
	DBCOLUMNACCESS*	rgColAccess, 			// @parmopt [IN] Array of ColAccess
	BOOL			fFreeOuter				// @parmopt [IN] Whether or not to free rgColAccess
)
{
	HRESULT hr = S_OK;

	// Loop through ColAccess and free outofline data
	for(DBORDINAL i=0; i<cColAccess; i++)
	{
		if(rgColAccess[i].pData)
		{
			switch(rgColAccess[i].dwStatus)
			{
				case DBSTATUS_S_OK:
				case DBSTATUS_S_ALREADYEXISTS:
					switch(rgColAccess[i].wType)
					{
						case DBTYPE_IUNKNOWN:
						{
							IUnknown** ppIUnknown = (IUnknown**)rgColAccess[i].pData;
							if(ppIUnknown)
								SAFE_RELEASE(*ppIUnknown);
							break;
						}
					};
			};
		}
		
		// Release the columnids
		ReleaseDBID(&rgColAccess[i].columnid, FALSE);
	}

	//Free the ColAccess array
	if(fFreeOuter)
		SAFE_FREE(rgColAccess);
	return hr;
}


//----------------------------------------------------------------------
// MiscFunc GetAccessorBindings |
// Same thing as GetAccessorAndBindings but binds everycolumn as WCHAR
// 
// @mfunc HRESULT|
// 
//----------------------------------------------------------------------
HRESULT GetStringAccessorAndBindings(
	IUnknown *			pIUnkObject,		// @parm [IN]  Rowset, Command, or RowObject to create Accessor for
	DBCOLUMNINFO *		rgDBCOLUMNINFO,
	DBORDINAL			cDBCOLUMNINFO,
	HACCESSOR *			phAccessorOut,			// @parm [OUT] Accessor created
	DBBINDING **		prgDBBINDINGOut,		// @parm [OUT] Array of DBBINDINGS
	DBORDINAL *			pcBindingsOut,			// @parm [OUT] Count of bindings
	DBLENGTH *			pcbRowSizeOut			// @parm [OUT] Length of a row, DATA	
)
{
	HRESULT 			hr=NOERROR;			// General HRESULT - assume success until proven otherwise
	HRESULT 			hrCreateAcc=E_FAIL;
											// CreateAccessor HRESULT - assume failure until proven otherwise
	IAccessor *			pIAccessor=NULL;	// IAccessor interface pointer
	DBORDINAL			ulColidx = 0;   	// index for DBCOLUMNINFO array
	DBCOUNTITEM			ulBindidx = 0;		// DBBindings array index
	DBCOUNTITEM			ulBindCount = 0;	// Number of bindings in array
	DBCOUNTITEM			ulLoopCounter=0;	// loop counter when building bindings
	DBBYTEOFFSET		ulOffset = 0;		// offset in DATA
	ULONG 				pulErrorBinding=0;	// count of errors
	DBCOUNTITEM			cBindings = 0;		// count of bindings in accessor				
	HACCESSOR			hAccessor = NULL;	// handle of accessor created
	DBBINDING *			rgDBBINDING = NULL;	// array of bindings for accessor
	DBORDINAL			cCols=cDBCOLUMNINFO;				// number of columns in rowset
	DBLENGTH			cbRowSize;			// size of one row of bound data
	ULONG				ulColInc = 1;		// amount to increment by when looping thru col info array
	DBREFCOUNT			cRefCounts=ULONG_MAX;// Init to a bogus count for testing purposes
	
	DBACCESSORFLAGS		dwAccessorFlags=DBACCESSOR_ROWDATA;
	DBPART				dwPart = DBPART_VALUE | DBPART_STATUS |	DBPART_LENGTH;

	// Check params
	if (pIUnkObject==NULL)
	{
		hr = E_FAIL;
		goto CLEANUP;
	}

	// We want potentially every column, so increment by one thru column array
	// Bind each column once
	ulColInc  = 1;
	cBindings = cCols;

	// Build DBBINDING to Create Accessor
	rgDBBINDING = (DBBINDING *) PROVIDER_ALLOC(sizeof(DBBINDING) * cBindings ); 
	memset(rgDBBINDING,0xCC, (size_t)(cBindings * sizeof(DBBINDING)));
							 
	// Loop thru col array, building bindings, incrementing by two if we're not doing all cols
	for(ulLoopCounter=0; ulColidx<cDBCOLUMNINFO; ulLoopCounter++,ulColidx+=ulColInc)
	{				
		// Only one BLOB in the Accessor bindings
		if( rgDBCOLUMNINFO[ulColidx].dwFlags & DBCOLUMNFLAGS_ISLONG )
			continue;

		// Increment count since we'll be binding this column
		ulBindCount++;				

		// Fill bindings in forward order, so just base 
		// index value on count of bindings done already
		ulBindidx = ulBindCount -1;
		ASSERT(ulBindidx < cCols);
	
		rgDBBINDING[ulBindidx].dwPart = dwPart;
		rgDBBINDING[ulBindidx].eParamIO = DBPARAMIO_NOTPARAM;
		rgDBBINDING[ulBindidx].iOrdinal = rgDBCOLUMNINFO[ulColidx].iOrdinal; 

		rgDBBINDING[ulBindidx].wType = DBTYPE_STR;
		rgDBBINDING[ulBindidx].pTypeInfo = rgDBCOLUMNINFO[ulColidx].pTypeInfo;
		rgDBBINDING[ulBindidx].dwMemOwner = DBMEMOWNER_CLIENTOWNED;

		// Only do this if we're binding the value
		if (dwPart & DBPART_VALUE)
		{
			// Use a max if the column type is larger than it
			// If a IColumnsInfo reports a column size at > MAX_COL_SIZE
			// Chances are that it is a BLOB or PrivLib didn't create the table.
			// Be generous in this case and bind to MAXDATALEN
			rgDBBINDING[ulBindidx].obValue = 	ulOffset + offsetof(DATA,bValue);
			rgDBBINDING[ulBindidx].cbMaxLen = 
				(rgDBCOLUMNINFO[ulColidx].ulColumnSize > MAX_COL_SIZE) ?
				MAXDATALEN : rgDBCOLUMNINFO[ulColidx].ulColumnSize;
			
			// Add room for the null terminator on string data
			if ( (~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR) & rgDBBINDING[ulBindidx].wType) == DBTYPE_STR) 
				rgDBBINDING[ulBindidx].cbMaxLen += sizeof(CHAR);
		}

		rgDBBINDING[ulBindidx].pObject = NULL;

		// Only set length offset if we're binding length
		if (dwPart & DBPART_LENGTH)
			rgDBBINDING[ulBindidx].obLength = ulOffset + offsetof(DATA,ulLength);

		// Only set status offset if we're binding status
		if (dwPart & DBPART_STATUS)
			rgDBBINDING[ulBindidx].obStatus = ulOffset + offsetof(DATA,sStatus);
		
		// Increment our offset to account for the size of our bound structure
		ulOffset += sizeof(DATA);
						
		// If we bound the value, we need to compensate for any extra room 
		// the value may have taken over the bValue size allocated in the struct
		if (dwPart & DBPART_VALUE)
			if (rgDBBINDING[ulBindidx].cbMaxLen > sizeof(DBLENGTH))
				ulOffset += rgDBBINDING[ulBindidx].cbMaxLen - sizeof(DBLENGTH);

		// Make sure our structure begins on a correct byte alignment
		ulOffset = ROUND_UP(ulOffset,ROUND_UP_AMOUNT);
	}
	
	// If we aren't binding even or odd columns, cBindings is really count of columns,
	// adjust it to what actually got bound.
	cBindings = ulBindCount;
	cbRowSize = ulOffset;
	  
	// IAccessor->CreateAccessor get HACCESSOR back
	if(!VerifyInterface(pIUnkObject, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}
	else
	{
		hrCreateAcc=pIAccessor->CreateAccessor(dwAccessorFlags, cBindings, 
									rgDBBINDING, cbRowSize, &hAccessor, NULL);
	}

CLEANUP:
	
 	// Set all parameters user wants back if we've succeeded, 
	// free the rest and set user's buffer to NULL
	if (phAccessorOut && SUCCEEDED(hrCreateAcc))
		*phAccessorOut = hAccessor;
	else
	{		
		if (pIAccessor)
		{
			pIAccessor->ReleaseAccessor(hAccessor, &cRefCounts);
 			if (cRefCounts != 0)
				hr = E_FAIL;
		}
		if (phAccessorOut)
			*phAccessorOut=NULL;
	}

	if (prgDBBINDINGOut && SUCCEEDED(hrCreateAcc))
		*prgDBBINDINGOut = rgDBBINDING;
	else
	{	
		FreeAccessorBindings(cBindings,rgDBBINDING);
		
		if (prgDBBINDINGOut)
			*prgDBBINDINGOut = NULL;
	}

	if (pcBindingsOut && SUCCEEDED(hrCreateAcc))
		*pcBindingsOut = cBindings;
	else if (pcBindingsOut)
		*pcBindingsOut = 0;
	
	if (pcbRowSizeOut && SUCCEEDED(hrCreateAcc))
		*pcbRowSizeOut = cbRowSize;
	else if (pcbRowSizeOut)
		*pcbRowSizeOut = 0;
	
	SAFE_RELEASE(pIAccessor);
	
	// If we have had any problems, report the error code else
	// None of the other functions returned an error, so report CreateAccessor return code	
	if (FAILED(hr))
		return hr;
	else
		return hrCreateAcc;
}


//////////////////////////////////////////////////////////////////////////
// FindColInfo
//
//////////////////////////////////////////////////////////////////////////
BOOL FindColInfo(IUnknown* pIUnknown, DBID* pColumnID, DBORDINAL iOrdinal, DBCOLUMNINFO* pColInfo, WCHAR** ppStringBuffer)
{
	TBEGIN
	ASSERT(pIUnknown);
	DBORDINAL cColumns = 0;
	DBCOLUMNINFO* rgColInfo = NULL;
	WCHAR* pLocalStringsBuffer = NULL;
    IColumnsInfo* pIColumnsInfo = NULL; 
	DBORDINAL iCol = 0;
	BOOL bFound = FALSE;

	//Obtain the ColumnsInfo interface
	TESTC_(pIUnknown->QueryInterface(IID_IColumnsInfo, (void**)&pIColumnsInfo),S_OK)
	
	//GetColumnInfo	
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgColInfo, &pLocalStringsBuffer),S_OK)
	TESTC(cColumns!=0 && rgColInfo!=NULL && pLocalStringsBuffer!=NULL)

	//Try to find this columnid
	for(iCol=0; iCol<cColumns && !bFound; iCol++)
	{
		if(pColumnID)
		{
			if(CompareDBID(*pColumnID, rgColInfo[iCol].columnid))
				bFound = TRUE;
		}	
		else
		{
			if(rgColInfo[iCol].iOrdinal == iOrdinal)
				bFound = TRUE;
		}

		if(bFound && pColInfo)
		{
			memcpy(pColInfo, &rgColInfo[iCol], sizeof(DBCOLUMNINFO));

			//May times we just want the flags or something and don't need the overhead
			//of the string buffer.  So if not requesting the string buffer, make sure
			//we null the column names, so they aren't incorrectly referenced...
			if(ppStringBuffer == NULL)
			{
				pColInfo->pwszName = NULL;
				if(pColInfo->columnid.eKind==DBKIND_NAME || pColInfo->columnid.eKind==DBKIND_GUID_NAME || pColInfo->columnid.eKind==DBKIND_PGUID_NAME)
					pColInfo->columnid.uName.pwszName = NULL;
			}
		}
	}

CLEANUP:
    if(ppStringBuffer && bFound)
        *ppStringBuffer = pLocalStringsBuffer;
    else
    	PROVIDER_FREE(pLocalStringsBuffer);

	SAFE_FREE(rgColInfo);
	SAFE_RELEASE(pIColumnsInfo);
    return bFound;
}




//--------------------------------------------------------------------
// @func FillInputBindings, fills the input buffer with the proper
// data for the given bindings' columns and the given row number.
// This function assumes that the accessor was created with length, 
// value and status bound.  It uses the accessor to fill in values in 
// the data buffer.  Any non updateable columns in the accessor result
// in NULL being put in the corresponding buffer status field.
//
// @rdesc Success or Failure
// 	@flag  NOERROR  | Successful buffer filling
//	@flag  OTHER	| Problems
//
//--------------------------------------------------------------------
HRESULT FillInputBindings(				
	CSchema* pSchema,					// @parm [IN] Table object on which to do call MakeData	
	DBACCESSORFLAGS dwAccessorFlags,	// @parm [IN] Flags for accessor.  If its a PARAMETERDATA accessor, 
										// the rgColOrds contains the ordinals to be passed to MakeData
										// to generate the data for each parameter, with each element
										// corresponding to a parameter (ie, element 0 correpsonds to parameter 1 and so on).
										// If its a ROWDATA or a ROWDATA | PARAMETERDATA accessor, rgColOrds
										// is simply the array of all columns IN THE ROWSET, and the bindings
										// are used to determine which columns are bound and thus which ordinals
										// are passed to MakeData to generate the correct values.  Note the
										// PARAMETERDATA only accessor scenario will require the user to build
										// an array with only the columns that pertain to each parameter, and in the 
										// correct order, whereas the ROWDATA accessor scenario will require the 
										// user to pass an array with all columns, regardless of which ones are bound.
	DBCOUNTITEM cBindings,				// @parm [IN] Number of bindings
	DBBINDING* rgBindings,				// @parm [IN] Bindings of hAccessor
	BYTE** ppData,						// @parm [IN/OUT] Data buffer to be filled.  User must free 
										// *ppData unless they pass a non null *ppData when calling, which
										// is large enough to hold a row of data.
	DBCOUNTITEM ulRowNum,				// @parm [IN] Row number to create data with
	DBORDINAL cColOrds,					// @parm [IN] Count of elements in rgColOrds
	DB_LORDINAL* rgColOrds,				// @parm [IN] For PARAMETERDATA accessors, each element in the array 
										// specifies the iNumber of a column to be bound.  This iNumber should 
										// be the same as that returned by IColumnsInfo.
										// This iNumber is always ordered the same as the column list in the command 
										// specification, thus the user can use the command specification col list
										// order to determine the appropriate iNumber.
										// For ROWDATA accessors, the array must contain the table ordinals for
										// all cols in the rowset, regardless of which ones are bound.  The bindings
										// will be used to determine which ones are bound and thus what ordinals are
										// passed to MakeData.

	EVALUE eValue,						// @parm [IN] Type of data to create
	IUnknown* pIUnkObject,				// Pointer to Object
	DWORD	dwColsToBind				// @parmopt [IN] Which columns will be used in the bindings
)
{
	HRESULT		hr				= S_OK;
	BOOL		fByRef			= FALSE;
	DBORDINAL	ulIdx			= 0;
	DBLENGTH	ulBytes			= 0;
	WCHAR *		pwszData		= NULL;
	DB_LORDINAL *	own_rgColOrds	= NULL;
	void *		pvData			= NULL;
	CCol		tempCol;	
	DBTYPE		dbBaseType;
	DBTYPE		wBackEndType;
	DBTYPE		vt;						// Variant type for variant data
	
	// Check the Data Pointer
	if (!ppData)
		return E_INVALIDARG;

	// Find out if the rgColOrds array is packed (ie, PARAMETERDATA accessor)
	// with only the column ordinals we need, or if we need to find what 
	// columns are bound ourselves and pick them out of rgColOrds (ie, ROWDATA accessor)
	if (dwAccessorFlags & DBACCESSOR_ROWDATA)
	{
		// We'll be rebuilding a packed rgColOrds, so remember its unpacked contents
		DB_LORDINAL * rgUnpackedOrds	= rgColOrds;		
		DBORDINAL  ulOrdIdx			= 0;

		// We need to alloc and build our own packed array based on the bindings
		own_rgColOrds = (DB_LORDINAL *)PROVIDER_ALLOC(sizeof(DB_LORDINAL) * cBindings);
		if (!own_rgColOrds)
		{
			hr = E_OUTOFMEMORY;
			goto CLEANUP;
		}
		
		// New count of packed array will be same as count of bindings
		cColOrds = cBindings;

		// Find the table ordinal in the unpacked array that
		// corresponds to this binding, and record it in our packed array
		for(DBCOUNTITEM i=0; i<cBindings; i++)
		{
			// For parameter accessors it is important to use rgUnpackedOrds if passed.
			if ((rgUnpackedOrds) && (rgBindings[i].iOrdinal))
				own_rgColOrds[i] = rgUnpackedOrds[rgBindings[i].iOrdinal-1];
			else
				own_rgColOrds[i] = rgBindings[i].iOrdinal;
		}

		// Copy the pointer
		rgColOrds = own_rgColOrds;
	}

	// If user hasn't allocated the memory, we need to
	if (!*ppData)
	{
		for(ulIdx=0; ulIdx<cColOrds; ulIdx++)
		{		
			//Obtain the CCol for this columns
			if(FAILED(pSchema->GetColInfo(rgColOrds[ulIdx], tempCol)) && pIUnkObject)
			{
				DBCOLUMNINFO dbColumnInfo;
				WCHAR* pStringBuffer = NULL;

				if(FindColInfo(pIUnkObject, NULL, rgColOrds[ulIdx], &dbColumnInfo, &pStringBuffer))
					tempCol.SetColInfo(&dbColumnInfo);
				SAFE_FREE(pStringBuffer);
			}
		
			// Only allocate room for value if its bound
			if (VALUE_IS_BOUND(rgBindings[ulIdx]))
			{
				// If this binding is BYREF or ARRAY, allocate enough memory for a pointer only
				if (rgBindings[ulIdx].wType & (DBTYPE_BYREF | DBTYPE_ARRAY))
					ulBytes += sizeof(VOID *);
				// For DBTYPE_VECTOR we use sizeof(DBVECTOR)
				else if (rgBindings[ulIdx].wType & DBTYPE_VECTOR)
					ulBytes += sizeof(DBVECTOR);
				// For fixed length types the provider ignores cbMaxLen
				else if (IsFixedLength(rgBindings[ulIdx].wType))
				{
					// Allocate enough memory to hold the column value in char form
					// NOTE:  We are assuming that char is the longest form we
					// will have to store for any datatype!
					// We need twice as much precision for the binary types to
					// be made into char form		
					if(tempCol.GetProviderType() == DBTYPE_BYTES)
						ulBytes +=(tempCol.GetMaxSize()+1)*2*sizeof(WCHAR);					
					else if(IsNumericType(tempCol.GetProviderType()) || tempCol.GetProviderType()==DBTYPE_DBTIMESTAMP)
						ulBytes +=(max(tempCol.GetPrecision(), tempCol.GetColumnSize())+1)*sizeof(WCHAR);
					else
						ulBytes +=(tempCol.GetMaxSize()+1)*sizeof(WCHAR);

				}
				else
					// Variable length type, use cbMaxLen
					ulBytes += rgBindings[ulIdx].cbMaxLen;
			}

			// Increment our offset to account for the size of our bound structure
			// GetAccessorAndBindings always accounts for this struct even if
			// Length or Status are not bound
			ulBytes += sizeof(DATA);
						
			// Make sure our structure begins on a correct byte alignment
			ulBytes = ROUND_UP(ulBytes,ROUND_UP_AMOUNT);
		}
		
		*ppData = (BYTE *)PROVIDER_ALLOC(ulBytes);
		if (!*ppData)
		{
			hr = E_OUTOFMEMORY;
			goto CLEANUP;
		}
	}


	// Now fill in the buffer
	for(ulIdx=0; ulIdx<cBindings; ulIdx++)
	{		
		DBBINDING* pBinding = &rgBindings[ulIdx];
		
		// Find out if this binding is BYREF
		if (pBinding->wType & DBTYPE_BYREF)		
			fByRef = TRUE;			 
		else
			fByRef = FALSE;

		// Remove the BYREF from the bitmask, if it is set, so
		// we are left with just the base type
		dbBaseType =  pBinding->wType & (~DBTYPE_BYREF);

		//Obtain the CCol for this columns
		if(FAILED(pSchema->GetColInfo(rgColOrds[ulIdx], tempCol)) && pIUnkObject)
		{
			DBCOLUMNINFO dbColumnInfo;
			WCHAR* pStringBuffer = NULL;

			if(FindColInfo(pIUnkObject, NULL, rgColOrds[ulIdx], &dbColumnInfo, &pStringBuffer))
				tempCol.SetColInfo(&dbColumnInfo);
			SAFE_FREE(pStringBuffer);
		}

		//Backend Type
		wBackEndType = tempCol.GetProviderType();
	
		// Allocate enough memory to hold the column value in char form
		// We need twice as much precision for the binary types to
		// be made into char form		
		vt = DBTYPE_EMPTY;
		if ((DBTYPE_BYTES == wBackEndType) || 
			((DBTYPE_UI1 | DBTYPE_VECTOR) == wBackEndType))
			pwszData = (LPWSTR)PROVIDER_ALLOC((tempCol.GetMaxSize()+1)*2*sizeof(WCHAR));
		else if (DBTYPE_VARIANT == wBackEndType)
		{
			//Since this is a variant, the Length could vary from VT_I1 to a
			//large VT_BSTR.  Just alloc our largest buffer...
//			vt = VT_BSTR;
			pwszData = (LPWSTR)PROVIDER_ALLOC(MAXDATALEN*sizeof(WCHAR));
		}
		else
		{
			//Adding 400 is a total hack since an accurate display length
			//is not available in OLE DB.  The Datetime type requires 4 more chars
			//to display than its precision indicates, and that's our 
			//largest type discrepancy between display length and precision.
			//And for VarNumeric, precision has even less correlation to display size
			pwszData = (LPWSTR)PROVIDER_ALLOC((tempCol.GetMaxSize()+400)*sizeof(WCHAR));
		}

		if (!pwszData)
		{
			hr = E_OUTOFMEMORY;
			goto CLEANUP;
		}

		// Create the data 		  
		// If STATUS is not bound, don't allow MakeData to create NULL columns
		// There is no way to signify DBSTATUS_S_ISNULL if status is not bound!!
		if(!STATUS_IS_BOUND(*pBinding))
			hr = tempCol.MakeData(pwszData, ulRowNum, eValue, NONULLS, &vt, pSchema->GetIndexColumn());
		else
			hr = tempCol.MakeData(pwszData, ulRowNum, eValue, pSchema->GetNull(), &vt, pSchema->GetIndexColumn());
		
		PRVTRACE(L"**PrivLib (FillInputBindings): %8s, %10s, cb=%4u, '%s'\n",tempCol.GetColName(),tempCol.GetProviderTypeName(),wcslen(pwszData),pwszData);
		
		// Non updateable columns are an OK failure, abort on any other failure
		if (FAILED(hr) && hr != DB_E_BADTYPE)
			goto CLEANUP;
		
		// Make sure we don't ever access offsets outside the alloced buffer
		// if we allocated the buffer
		if (ulBytes)
		{
			if (STATUS_IS_BOUND(*pBinding))
			{
				ASSERT(pBinding->obStatus < ulBytes);			
			}
			if (LENGTH_IS_BOUND(*pBinding))
			{
				ASSERT(pBinding->obLength < ulBytes);			
			}
			if (VALUE_IS_BOUND(*pBinding))
			{
				ASSERT(pBinding->obValue  < ulBytes);			
			}
		}
		

		// Initialize the LENGTH and VALUE bindings, in case they are not used
		if (VALUE_IS_BOUND(*pBinding))
			*(BYTE *)&VALUE_BINDING(*pBinding, *ppData) = GARBAGE;
		if (LENGTH_IS_BOUND(*pBinding))
			LENGTH_BINDING(*pBinding, *ppData) = 0xFFFFFFFF;

		// Fill bindings as default for columns which can't be updated
		if(hr == DB_E_BADTYPE || (!tempCol.GetUpdateable() && (dwColsToBind & UPDATEABLE_COLS_BOUND)))
		{
			hr = S_OK;
			
			// MakeData - DB_E_BADTYPE = Not Updatable -> DBSTATUS_S_IGNORE
			//This way you can create an accessor binding all columns, and be able to
			//use that accessor for both GetData and SetData.  In the SetData case all 
			//non-updatable columns will be marked with IGNORE and skipped by the provider.
			if (STATUS_IS_BOUND(*pBinding))
				STATUS_BINDING(*pBinding, *ppData) = DBSTATUS_S_IGNORE;
		}
		// Fill bindings as null for null columns
		else if(hr == S_FALSE)
		{
			hr = S_OK;
			
			// MakeData - S_FALSE = NULL -> DBSTATUS_S_ISNULL
			if (STATUS_IS_BOUND(*pBinding))
				STATUS_BINDING(*pBinding, *ppData) = DBSTATUS_S_ISNULL;
			
		}
		else		
		{
			USHORT cb = 0;

			// Since its not a null value, set status to OK
			if (STATUS_IS_BOUND(*pBinding))
				STATUS_BINDING(*pBinding, *ppData)= DBSTATUS_S_OK;

			// Fill in parameter data according to base type specified in binding,
			if(dbBaseType == DBTYPE_IUNKNOWN)
			{
				pvData = WSTR2DBTYPE(pwszData, wBackEndType, &cb);
			}
			else if(dbBaseType == DBTYPE_VARIANT)
			{
				//Convert MakeData string to the actual vt type
				void* pDBTypeData = WSTR2DBTYPE(pwszData, vt, &cb);

				MapDBTYPE2VARIANT(pDBTypeData, vt, cb, (VARIANT **)&pvData);				

				// Variants are a fixed-length type, set size correctly
				cb = sizeof(VARIANT);
				PROVIDER_FREE(pDBTypeData);
			} 
			else
			{
				pvData = WSTR2DBTYPE(pwszData, dbBaseType, &cb);
			}
			
			if (pvData)
			{	
				// Copy the right number of bytes (or the pointer
				// to the buffer if binding is BYREF) into consumer buffer
				// and specify length, if bound, else provider will
				// use cbMaxLen.  
				switch(dbBaseType)
				{
					case DBTYPE_I1:
					case DBTYPE_UI1: 
					case DBTYPE_I2:
					case DBTYPE_UI2:
					case DBTYPE_BOOL:
					case DBTYPE_I4:
					case DBTYPE_UI4:
					case DBTYPE_R4:
					case DBTYPE_I8:
					case DBTYPE_CY:
					case DBTYPE_UI8:					
					case DBTYPE_R8:
					case DBTYPE_DATE:
					case DBTYPE_NUMERIC:
					case DBTYPE_DECIMAL:
					case DBTYPE_DBDATE:
					case DBTYPE_DBTIME:
					case DBTYPE_DBTIMESTAMP:
					case DBTYPE_FILETIME:
					case DBTYPE_GUID:							
					case DBTYPE_VARIANT:										
					{
						//Fixed Length Types
						if (VALUE_IS_BOUND(*pBinding))
						{
							if (fByRef)
								VALUE_BINDING(*pBinding, *ppData) = pvData;
							else
								memcpy(&VALUE_BINDING(*pBinding, *ppData), pvData, cb);	
						}
						
						if (LENGTH_IS_BOUND(*pBinding))
							LENGTH_BINDING(*pBinding, *ppData) = 0xFFFFFFFF;
						break;
					}

					case DBTYPE_BYTES:
					case DBTYPE_VARNUMERIC:
					{
						if (VALUE_IS_BOUND(*pBinding))
						{
							if (fByRef)
								VALUE_BINDING(*pBinding, *ppData) = (void *)pvData;
							else
								memcpy(&VALUE_BINDING(*pBinding, *ppData), pvData, cb);
						}
						
						if (LENGTH_IS_BOUND(*pBinding))							
							LENGTH_BINDING(*pBinding, *ppData) = cb;
						break;							
					}

					case DBTYPE_BSTR:
					{
						if (VALUE_IS_BOUND(*pBinding))
						{
							if (fByRef)
								VALUE_BINDING(*pBinding, *ppData) = (void*)pvData;
							else
								memcpy(&VALUE_BINDING(*pBinding, *ppData), pvData, cb);
						}
						
						if (LENGTH_IS_BOUND(*pBinding))
							// These should both be the same size, but just to be safe...
							if (fByRef)
								LENGTH_BINDING(*pBinding, *ppData) = sizeof(BSTR *);
							else
								LENGTH_BINDING(*pBinding, *ppData) = sizeof(BSTR);
						break;
					}

					case DBTYPE_STR:
					{
						if (VALUE_IS_BOUND(*pBinding))
						{
							if (fByRef)
								VALUE_BINDING(*pBinding, *ppData) = (void *)pvData;
							else
								strcpy((CHAR*)&VALUE_BINDING(*pBinding, *ppData), (LPSTR)pvData);
						}
						
						if (LENGTH_IS_BOUND(*pBinding))
							LENGTH_BINDING(*pBinding, *ppData) = cb;
						break;
					}
					
					case DBTYPE_WSTR:
					{
						if (VALUE_IS_BOUND(*pBinding))
						{
							if (fByRef)
								VALUE_BINDING(*pBinding, *ppData) = (void *)pvData;
							else
								wcscpy((WCHAR*)&VALUE_BINDING(*pBinding, *ppData), (LPWSTR)pvData);
						}
						
						if (LENGTH_IS_BOUND(*pBinding))
							LENGTH_BINDING(*pBinding, *ppData) = cb;
						break;
					}

					case DBTYPE_IUNKNOWN:
					{
						//Create the Storage Object
						CStorage* pCStorage = new CStorage; //m_cRef == 1
						if(pCStorage)
						{
							//Write the data into our storage object.
							//NOTE:  We use WriteAt as to not move the Seek pointer, so the provider
							//can read from the stream at the begining not the end...
							ULARGE_INTEGER ulOffset = { 0 };
							TEST2C_(hr = pCStorage->WriteAt(ulOffset, pvData, cb, NULL), S_OK, S_FALSE);
							hr = S_OK;
						}
						
						//NOTE: We AddRef the Stream again, m_cRef == 2
						//The reason is that the provider always calls Release on the stream for
						//any SetData, InsertRow, SetColumns, AddColumns, etc.  If we don't addref an
						//extra time any operation (CompareData for example) after the Set* is done 
						//it will crash since the stream has already been released.  And also our
						//FreeBindingData will also be called to free out of line data, which 
						//will release the stream.
						if (VALUE_IS_BOUND(*pBinding))
						{
							SAFE_ADDREF(pCStorage);
							VALUE_BINDING(*pBinding, *ppData) = pCStorage;
						}
							
						//Some providers need to know the length of the stream
						//So set the length here, instead of sizeof(IUnknown*)
						if (LENGTH_IS_BOUND(*pBinding))	
							LENGTH_BINDING(*pBinding, *ppData) = pCStorage ? cb : sizeof(IUnknown*);
						break;							
					}

					default:
					{
						//Handle ARRAY or VECTOR modifiers
						if(dbBaseType & DBTYPE_ARRAY || dbBaseType & DBTYPE_VECTOR)
						{
							//DBTYPE_ARRAY is defined as SAFEARRAY*
							//DBTYPE_VECTOR is defined as DBVECTOR
							//So both are treated no differently than a Fixed Length Type
							if(VALUE_IS_BOUND(*pBinding))
							{
								//DBTYPE_BYREF is not allowed on Modifiers
								ASSERT(!fByRef);
								memcpy(&VALUE_BINDING(*pBinding, *ppData), pvData, cb);	
							}
							
							//Length should be ignored for _ARRAY or _VECTOR
							if (LENGTH_IS_BOUND(*pBinding))
								LENGTH_BINDING(*pBinding, *ppData) = 0xFFFFFFFF;
						}
						else
						{
							// These are the types we don't currently handle
							ASSERT(!L"Unhandled Types!");
						}

						break;
					}
				}
				
				// If binding is not BYREF, or we didn't bind VALUE, we can free the buffer containing
				// the data originally, since we don't need a copy of it 
				if (!fByRef || !(VALUE_IS_BOUND(*pBinding)))
				{
					// For a BSTR, we have to leave the memory around or we throw away our data
					if(dbBaseType != DBTYPE_BSTR)
						PROVIDER_FREE(pvData);
				}
			}
			else
			{
				// We couldn't make the coercion from STR to the binding type
				hr = E_FAIL;
				goto CLEANUP;
			}			
		}

		// Free buffer used for generated character represented data for this parameter
		PROVIDER_FREE(pwszData);
	}

CLEANUP:
	PROVIDER_FREE(own_rgColOrds);
	PROVIDER_FREE(pwszData);
	return hr;
}

//--------------------------------------------------------------------
// @func This function releases any memory associated with the
// input bindings that is not allocated in line in the data buffer.
// ie, vector->ptr memory, etc.  If any of the parameters are null,
// nothing is attempted.
//
// @rdesc Success or Failure
// 	@flag  NOERROR  | Successful buffer filling
//	@flag  E_FAIL	| IMalloc could not be retrieved
//	@flag  E_INVALIDARG	| cBindings was 0, or rgBindings or pData was NULL
//
//--------------------------------------------------------------------
HRESULT ReleaseInputBindingsMemory(		
		DBCOUNTITEM		cBindings,		// @parm [IN] Number of bindings
		DBBINDING * rgBindings,		// @parm [IN] Bindings of hAccessor
		BYTE*		pData,			// @parm [IN] Input Data buffer 
		BOOL		fFreeData		// @parmopt [IN] Whether to free pData or not
)
{

	DBCOUNTITEM		i;
	DBTYPE		dbBaseType;

	// If we don't have the right parameters
	// we assume the input bindings were never filled
	// so we return
	if (!cBindings || !rgBindings || !pData)
		return E_INVALIDARG;

	for(i=0; i<cBindings; i++)
	{
		// If status is bound and its null or an error, our value 
		// is likely not there, so continue without freeing
		if (STATUS_IS_BOUND(rgBindings[i]))
		{
			if(STATUS_BINDING(rgBindings[i], pData) != DBSTATUS_S_OK &&
				STATUS_BINDING(rgBindings[i], pData) != DBSTATUS_S_TRUNCATED)
				continue;
		}
		else
			// If status is not bound then we don't know if it was null or
			// an error and so we will crash for those cases.  A leak is
			// better than a crash.
			continue;
		
		if (VALUE_IS_BOUND(rgBindings[i])) 
		{
			// If the wType of the binding structure is ORed with DBTYPE_BYREF,
			// the value in the consumer's buffer is a pointer to the data allocated 		
			if (rgBindings[i].wType & DBTYPE_BYREF)
			{
				dbBaseType = rgBindings[i].wType & (~DBTYPE_BYREF);
				void* pByRef = VALUE_BINDING(rgBindings[i], pData);

				// Free BSTR buffers using SysFreeString
				if (dbBaseType == DBTYPE_BSTR)
					SysFreeString(*(BSTR*)pByRef);
				// Free Variant buffers using VariantClear
				else if (dbBaseType == DBTYPE_VARIANT)
				{
					GCHECK(VariantClear((VARIANT*)pByRef),S_OK);
					PROVIDER_FREE(VALUE_BINDING(rgBindings[i], pData));
				}
				else
					PROVIDER_FREE(VALUE_BINDING(rgBindings[i], pData));
			
				continue;
			}

			// Check for special types and release associated memory
			if (rgBindings[i].wType & DBTYPE_VECTOR)
			{
				PROVIDER_FREE(((DBVECTOR *)(&VALUE_BINDING(rgBindings[i], pData)))->ptr);
				continue;
			}
			else if (rgBindings[i].wType & DBTYPE_ARRAY)
			{
				SafeArrayDestroy(*(SAFEARRAY**)&VALUE_BINDING(rgBindings[i], pData));
				continue;
			}

			// If the wType of the binding structure is not ORed with any type
			// modifier, only need to free memory for DBTYPE_BSTR and DBTYPE_VARIANT
			if (rgBindings[i].wType == DBTYPE_BSTR)
				SysFreeString((BSTR)VALUE_BINDING(rgBindings[i], pData));
			else if (rgBindings[i].wType == DBTYPE_VARIANT)
				GCHECK(VariantClear((VARIANT *)&VALUE_BINDING(rgBindings[i], pData)),S_OK);
		}	
	}

	// Free the data buffer itself if caller requested it
	if(fFreeData)	
	{
		if (pData)
		{
			PROVIDER_FREE(pData);
			pData=NULL;
		}
	}
	return S_OK;
}

//--------------------------------------------------------------------
// @func Converts BSTR to WCHAR, frees BSTR
//
// Returns the WCHAR string contained in the BSTR, but with a NULL terminator.
// If error occurs, NULL is returned.  Caller is responsible
// for freeing the returned string, but if fFreeBstr is set, the function 
// calls SysFreeString on the input bstr.
// If the input bstr is NULL, the string returned is "NULL!"
//
//--------------------------------------------------------------------
LPWSTR BSTR2WSTR(BSTR bstr, BOOL fFreeBstr)
{
	WCHAR* pwszReturn = wcsDuplicate(bstr ? bstr : L"NULL!");
	
	//Allocation failure
	if(!pwszReturn)
		odtLog << L"Error Allocating Memory" << wszNewLine;
	
	//Free passed in BSTR?
	if(fFreeBstr && bstr)
		SysFreeString(bstr);

	return pwszReturn;
}

//--------------------------------------------------------------------
// @func PrintRowset
//
// @rdesc Success or Failure
// 	@flag  NOERROR  | Successful initialization
//	@flag  OTHER	| Problems
//
//--------------------------------------------------------------------
void PrintRowset(
	 IRowset * pIRowset	// @parm [IN]  Rowset to print
)
{
	HRESULT			hr				= E_FAIL;

	IColumnsInfo *	pIColumnsInfo	= NULL;
	DBCOLUMNINFO *	rgDBCOLUMNINFO	= NULL;
	DBORDINAL		cColumns		= 0;
	WCHAR *			pStringsBuffer;

	size_t			lText			= 0;
	DBCOUNTITEM		iBind			= 0;
	DBCOUNTITEM		iRow			= 0;
	DBCOUNTITEM		cRowsObtained	= 0;
	BYTE*			pRowData		= NULL;
	DATA *			pColumn			= NULL;
	
	HROW 			rghRows[NUMROWS_CHUNK];
	HROW*			pRows			= &rghRows[0];
	DBBINDING *		rgDBBINDING;
	HACCESSOR 		hAccessor;
	DBCOUNTITEM		cBindings;
	DBLENGTH		cbRowSize;
	
	WCHAR			strUnicode[MAXDISPLAYSIZE+1];
	WCHAR			wszDispBuffer[MAXDISPLAYSIZE+1];
	WCHAR			wszTempBuffer[MAXDISPLAYSIZE+1];

	strUnicode[0]=L'\0';

	if(!VerifyInterface(pIRowset, IID_IColumnsInfo, ROWSET_INTERFACE, (IUnknown**)&pIColumnsInfo))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	if (FAILED(hr=pIColumnsInfo->GetColumnInfo(&cColumns, 
								&rgDBCOLUMNINFO, &pStringsBuffer)))
		goto CLEANUP;

	if (FAILED(GetStringAccessorAndBindings(pIRowset, rgDBCOLUMNINFO, cColumns,
							&hAccessor, &rgDBBINDING, &cBindings, &cbRowSize)))
		goto CLEANUP;

	// Create a buffer for row data, big enough to hold the biggest row
	pRowData = (BYTE *) PROVIDER_ALLOC( cbRowSize );
	
	// Make sure buffer is valid
	if (!pRowData)
        goto CLEANUP;

	{
		// Display a description of each column in the result set. 
		// Store the column name in the display buffer and make it
		// the first entry in the results list box of the MDI child window.
		for(USHORT nCount=0; nCount < cColumns; nCount++)
		{
			if (rgDBCOLUMNINFO[nCount].pwszName)
			{
				strUnicode[lText] = L'\0';
				wcscat(strUnicode, rgDBCOLUMNINFO[nCount].pwszName);
			}
			else
				wcscat(strUnicode, wszEmptyString);			

			lText = wcslen(strUnicode);
			strUnicode[lText++] = L',';
			strUnicode[lText] = L'\0';
		}
		
		// NULL Terminate the Display Buffer
		if (*strUnicode)
			strUnicode[wcslen(strUnicode)-1]=L'\0';
	
		// ADD the Column Info to the Screen
		odtLog << strUnicode << L"\n";
	}

	
    // Process all the rows, NUMROWS_CHUNK rows at a time
	while(1)
	{
		// Get some rows to process
		if (FAILED(hr=pIRowset->GetNextRows(0, 0, NUMROWS_CHUNK,
											&cRowsObtained, &pRows)))
			goto CLEANUP;

		// Quit if we are all done
		if(cRowsObtained == 0)
			break;

		{
			// Loop over rows obtained, getting data for each
			for(iRow=0; iRow < cRowsObtained; iRow++)
			{
				// Get this row
				if(FAILED(hr=pIRowset->GetData(rghRows[iRow], hAccessor, pRowData)))
					goto CLEANUP;

				// ASSERTs
				ASSERT(rgDBBINDING);
				ASSERT(pRowData);

				// Print each column we're bound to.
				for(iBind=0, wszDispBuffer[0]=L'\0'; iBind < cBindings; iBind++)
				{
					wszTempBuffer[0] = L'\0';

					// Columns are bound differently; not so easy.
					// Print out to at least DEFAULT_CBMAXLENGTH width (pretty),
					// Limit to first ulLength characters.
					pColumn = (DATA *)(pRowData + rgDBBINDING[iBind].obStatus);

					// Check Status 
					switch(pColumn->sStatus)
					{
						case DBSTATUS_S_ISNULL:
							wcscat(wszDispBuffer, wszReturnNULL);
							break;

						case DBSTATUS_S_OK:
							//64bit TODO - remove (INT) cast.
							MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (const char *)pColumn->bValue, 
												(INT) (pColumn->ulLength), wszTempBuffer, MAXDISPLAYSIZE+1);
							wszTempBuffer[pColumn->ulLength] = L'\0';
							wcscat(wszDispBuffer, wszTempBuffer);
							break;

						case DBSTATUS_E_CANTCONVERTVALUE:
							wcscat(wszDispBuffer, wszCanntCoerce);
							break;

						case DBSTATUS_S_TRUNCATED:
							wcscat(wszDispBuffer, wszTrucated);
							break;

						case DBSTATUS_E_SIGNMISMATCH:
							wcscat(wszDispBuffer, wszMisMatch);
							break;

						case DBSTATUS_E_DATAOVERFLOW:
							wcscat(wszDispBuffer, wszOverflow);
							break;

						case DBSTATUS_E_CANTCREATE:
							wcscat(wszDispBuffer, wszCanntCreate);
							break;

						case DBSTATUS_E_UNAVAILABLE:
							wcscat(wszDispBuffer, wszUnavailable);
							break;

						case DBSTATUS_E_BADACCESSOR:
							wcscat(wszDispBuffer, wszViolation);
							break;

						case DBSTATUS_E_INTEGRITYVIOLATION:
							wcscat(wszDispBuffer, wszIntegrity);
							break;

						case DBSTATUS_E_SCHEMAVIOLATION:
							wcscat(wszDispBuffer, wszSchema);
							break;

						default:
							wcscat(wszDispBuffer, wszUnknownStat);
							break;
					}
					lText = wcslen(wszDispBuffer);
					wszDispBuffer[lText++] = L',';
					wszDispBuffer[lText] = L'\0';
				}
				
				// Take the last \t off the end
				wszDispBuffer[--lText] = L'\0';
				odtLog << wszDispBuffer << L"\n";
			}
		}

		// See if you are over the limit of rows
		if ((iRow + 1) == MAX_ROW)
		{
			swprintf(strUnicode, ROWTRUNC_WARNG, MAX_ROW);
			break;
		}

		// Release row handles
		if (FAILED(hr=pIRowset->ReleaseRows(cRowsObtained,
											rghRows, NULL, NULL, NULL)))
			goto CLEANUP;
	}
	
CLEANUP:

	if (pRowData)
		PROVIDER_FREE(pRowData);
	
	if (rgDBBINDING)
		FreeAccessorBindings(cBindings,rgDBBINDING);
	
	SAFE_RELEASE(pIColumnsInfo);
	PROVIDER_FREE(rgDBCOLUMNINFO);
	PROVIDER_FREE(pStringsBuffer);
}

//--------------------------------------------------------------------
// @func BOOL | SupportedProperty
//
// This function should return TRUE if the fuction succeeded and 
// the property is supported by the Provider.
//
//    eCoType defaults to DATASOURCE_INTERFACE in the prototype
//
//--------------------------------------------------------------------
BOOL SupportedProperty(DBPROPID PropertyID, GUID guidPropertySet, IUnknown* pIUnknown,EINTERFACE eCoType)
{
	return GetPropInfoFlags(PropertyID, guidPropertySet, pIUnknown,eCoType) != DBPROPFLAGS_NOTSUPPORTED; 
}

//--------------------------------------------------------------------
// @func BOOL | SettableProperty
//
// This function should return TRUE if the fuction succeeded and 
// the property is settable by the Provider.
//
//    eCoType defaults to DATASOURCE_INTERFACE in the prototype
//
//--------------------------------------------------------------------
BOOL SettableProperty(DBPROPID PropertyID, GUID guidPropertySet, IUnknown* pIUnknown, EINTERFACE eCoType)
{
	return GetPropInfoFlags(PropertyID, guidPropertySet, pIUnknown, eCoType) & DBPROPFLAGS_WRITE ? TRUE : FALSE; 
}

//--------------------------------------------------------------------
// @func BOOL | GetPropInfoFlags
//
// This function should return the DBPROPFLAGS of the Property.
//
//    eCoType defaults to DATASOURCE_INTERFACE in the prototype
//
//--------------------------------------------------------------------
DBPROPFLAGS GetPropInfoFlags(DBPROPID PropertyID, GUID guidPropertySet, IUnknown* pIUnknown, EINTERFACE eCoType)
{
	DBPROPINFO* pPropInfo = NULL;
	DBPROPFLAGS dwPropFlags = DBPROPFLAGS_NOTSUPPORTED;

	// Call the GetPropertyInfo
	pPropInfo = GetPropInfo(PropertyID, guidPropertySet, pIUnknown, eCoType);
	if(pPropInfo == NULL)
		goto CLEANUP;
	
	// Copy flags values, so we can free the alloced struct
	dwPropFlags = pPropInfo->dwFlags;
	SAFE_FREE(pPropInfo->pwszDescription);
	VariantClear(&pPropInfo->vValues);
	PROVIDER_FREE(pPropInfo);

CLEANUP:
	return dwPropFlags;
}

//--------------------------------------------------------------------
// @func BOOL | GetPropInfo
//
// This function should return the DBPROPINFO of the requested Property.
//
//    eCoType defaults to DATASOURCE_INTERFACE in the prototype
//
//--------------------------------------------------------------------
DBPROPINFO* GetPropInfo(DBPROPID PropertyID, GUID guidPropertySet, IUnknown* pIUnknown, EINTERFACE eCoType)
{
	HRESULT			hr			   = E_FAIL;
	DBPROPINFO *	pPropInfo	   = NULL;

	ULONG			cPropInfoSets  = 0;
	DBPROPINFOSET * rgPropInfoSets = NULL;
	IDBProperties*  pIDBProperties = NULL;
	WCHAR*			pwszStringBuffer = NULL;

	// Setup the input param DBPROPIDSET
	ULONG		cPropIDSets = 1;
	DBPROPIDSET rgPropIDSets;

	rgPropIDSets.cPropertyIDs = 1;
	rgPropIDSets.rgPropertyIDs = &PropertyID;
	rgPropIDSets.guidPropertySet = guidPropertySet;
	ASSERT(pIUnknown);

	// Obtain IDBProperties 
	if (!VerifyInterface(pIUnknown, IID_IDBProperties, eCoType, (IUnknown**)&pIDBProperties))
		goto CLEANUP;

	//IDBProperties::GetPropertyInfo
	hr = pIDBProperties->GetPropertyInfo(cPropIDSets, &rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	
	// Verify returned Info is valid
	if ((cPropInfoSets!=1 || rgPropInfoSets==NULL || 
		 rgPropInfoSets[0].cPropertyInfos!=1 || rgPropInfoSets[0].rgPropertyInfos==NULL) ||
		(rgPropInfoSets[0].rgPropertyInfos[0].dwPropertyID!=PropertyID))
	{
		//Provider has returned something we didn't expect, free it so there
		//isn't a leak and return a NULL pointer.  This can happend if calling
		//GetPrropertyInfo with ROWSETALL which the provider then ignores
		//cPropertyIDs and returns all of them...
		FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
		goto CLEANUP;
	}

	// Verify return code
	// GetPropertyInfo should only fail (DB_E_ERRRSOCCURRED) if not supported property
	if (hr==DB_E_ERRORSOCCURRED)
		ASSERT(rgPropInfoSets[0].rgPropertyInfos[0].dwFlags == DBPROPFLAGS_NOTSUPPORTED);
	else
		ASSERT(hr == S_OK);

	// Get the pointer and Free the rgPropInfo
	pPropInfo = rgPropInfoSets[0].rgPropertyInfos;
	//User May want the Description, instead of having to return the huge
	//String buffer for, we will just duplicate it for this one...
	pPropInfo->pwszDescription = wcsDuplicate(pPropInfo->pwszDescription);

CLEANUP:
	// Free the PropertyInfoSet
	PROVIDER_FREE(rgPropInfoSets);
	SAFE_RELEASE(pIDBProperties);
	SAFE_FREE(pwszStringBuffer);
	return pPropInfo;
}

//--------------------------------------------------------------------
// @func BOOL | GetProperty
//
// This function should return TRUE if the fuction succeeded and 
// the boolean value of a property.
//
//--------------------------------------------------------------------
BOOL GetProperty
(
	DBPROPID PropertyID, 	// @parm  [IN] PropertyID
	GUID guidPropertySet,	// @parm  [IN] PropertySet GUID
	IUnknown* pIUnknown,	// @parm  [IN] Object pointer
	VARIANT_BOOL bValue		// @parm  [IN/OUT] Variant_Bool value
)
{
	ASSERT(pIUnknown);
	VARIANT_BOOL bActualValue;

	if (!GetProperty(PropertyID, guidPropertySet, pIUnknown, &bActualValue))
		goto CLEANUP;

	if (bActualValue == bValue)
		return TRUE;
	
CLEANUP:
	return FALSE;
}


//--------------------------------------------------------------------
// @func BOOL | GetProperty
//
// This function should return TRUE if the fuction succeeded and 
// any value type of a property.
//
//--------------------------------------------------------------------
BOOL GetProperty
(
	DBPROPID PropertyID, 
	GUID guidPropertySet, 
	IUnknown* pIUnknown,
	VARIANT_BOOL* pbValue
)
{
	ASSERT(pIUnknown);
	ASSERT(pbValue);
	BOOL bSuccess = FALSE;
	*pbValue = VARIANT_FALSE;

	VARIANT vVariant;
	VariantInit(&vVariant);
	
	//Get the property
	if(!GetProperty(PropertyID, guidPropertySet, pIUnknown, &vVariant))
		goto CLEANUP;

	//Verify Results
	switch(V_VT(&vVariant))
	{
		case VT_BOOL:
			*pbValue  =	V_BOOL(&vVariant);
			bSuccess = TRUE;
			break;
	}

CLEANUP:
	GCHECK(VariantClear(&vVariant),S_OK);
	return bSuccess;
}


//--------------------------------------------------------------------
// @func ULONG | GetProperty
//
// This function should return TRUE if the fuction succeeded and 
// any value type of a property.
//
//--------------------------------------------------------------------
BOOL GetProperty
(
	DBPROPID PropertyID, 
	GUID guidPropertySet, 
	IUnknown* pIUnknown,
	ULONG_PTR* pulValue
)
{
	ASSERT(pIUnknown);
	ASSERT(pulValue);
	BOOL bSuccess = FALSE;
	*pulValue = 0;

	VARIANT vVariant;
	VariantInit(&vVariant);
	
	//Get the property
	if(!GetProperty(PropertyID, guidPropertySet, pIUnknown, &vVariant))
		goto CLEANUP;

	//Verify Results
	switch(V_VT(&vVariant))
	{
		case VT_I4:
		case VT_UI4:
		case VT_I2:
		case VT_UI2:
			*pulValue = V_I4(&vVariant);
			bSuccess = TRUE;
			break;
	}

CLEANUP:
	GCHECK(VariantClear(&vVariant),S_OK);
	return bSuccess;
}


//--------------------------------------------------------------------
// @func BOOL | GetProperty
//
// This function should return TRUE if the fuction succeeded and 
// any value type of a property.
//
//--------------------------------------------------------------------
BOOL GetProperty
(
	DBPROPID PropertyID, 
	GUID guidPropertySet, 
	IUnknown* pIUnknown,
	WCHAR** ppwszValue
)
{
	ASSERT(pIUnknown);
	ASSERT(ppwszValue);
	BOOL bSuccess = FALSE;
	*ppwszValue = NULL;

	VARIANT vVariant;
	VariantInit(&vVariant);
	
	//Get the property
	if(!GetProperty(PropertyID, guidPropertySet, pIUnknown, &vVariant))
		goto CLEANUP;

	//Verify Results
	switch(V_VT(&vVariant))
	{
		case VT_BSTR:
			*ppwszValue = wcsDuplicate(V_BSTR(&vVariant));
			bSuccess = TRUE;
			break;
		case VT_EMPTY:
			//This case is for default values. For e.g. the default value
			//of PASSWORD may return VT_EMPTY.
			*ppwszValue = NULL;
			bSuccess = TRUE;
			break;
	}

CLEANUP:
	GCHECK(VariantClear(&vVariant),S_OK);
	return bSuccess;
}

//--------------------------------------------------------------------
// @func BOOL | GetProperty
//
// This function should return TRUE if the fuction succeeded and 
// any value type of a property.
//
//--------------------------------------------------------------------
BOOL GetProperty
(
	DBPROPID PropertyID, 
	GUID guidPropertySet, 
	IUnknown* pIUnknown, 
	VARIANT* pVariant
)
{
	ASSERT(pIUnknown && pVariant);
	DBPROP dbProp;

	//Delegate
	if(GetProperty(PropertyID, guidPropertySet, pIUnknown, &dbProp))
	{
		//User only wants the variant (which has already been copied...)
		//Memcpy (assignment)
		*pVariant = dbProp.vValue;
		return TRUE;
	}
	
	return FALSE;
}



//--------------------------------------------------------------------
// @func BOOL | GetProperty
//
// This function should return TRUE if the fuction succeeded and 
// any value type of a property.
//
//--------------------------------------------------------------------
BOOL GetProperty
(
	DBPROPID PropertyID, 
	GUID guidPropertySet, 
	IUnknown* pIUnknown, 
	DBPROP* pProp
)
{
	ASSERT(pIUnknown && pProp);
	
	HRESULT hr		 = E_FAIL;
	BOOL	fSuccess = FALSE;

	// Setup the input param DBPROPIDSET
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	DBPROPIDSET rgPropIDSets;
	rgPropIDSets.cPropertyIDs = 1;
	rgPropIDSets.rgPropertyIDs = &PropertyID;
	rgPropIDSets.guidPropertySet = guidPropertySet;

	// What kind of pointer is IUnknown
	// It could either be IRowset / ICommand / ISession / IDBInit
	IRowsetInfo* pIRowsetInfo = NULL;
	ICommandProperties* pICommandProperties = NULL;
	ISessionProperties* pISessionProperties = NULL;
	IDBProperties* pIDBProperties = NULL;

	// IID_IRowsetInfo
	if((hr=pIUnknown->QueryInterface(IID_IRowsetInfo, (void **)&pIRowsetInfo))==S_OK)
		hr = pIRowsetInfo->GetProperties(1, &rgPropIDSets, &cPropSets, &rgPropSets);
	// IID_ICommandProperties
	else if((hr=pIUnknown->QueryInterface(IID_ICommandProperties, (void **)&pICommandProperties))==S_OK)
		hr = pICommandProperties->GetProperties(1, &rgPropIDSets, &cPropSets, &rgPropSets);
	// IID_ISessionProperties
	else if((hr=pIUnknown->QueryInterface(IID_ISessionProperties, (void **)&pISessionProperties))==S_OK)
		hr = pISessionProperties->GetProperties(1, &rgPropIDSets, &cPropSets, &rgPropSets);
	// IID_IDBProperties
	else if((hr=pIUnknown->QueryInterface(IID_IDBProperties, (void **)&pIDBProperties))==S_OK)
		hr = pIDBProperties->GetProperties(1, &rgPropIDSets, &cPropSets, &rgPropSets);
	// Unsupported Types
	else
		hr = E_FAIL;

	// GetProperties will return DB_E_ERRORSOCCURRED for not supported properties
	// We will just return FALSE from this method if this is the case
	if (FAILED(hr))
	{
		CHECK(hr, DB_E_ERRORSOCCURRED);
		goto CLEANUP;
	}
	else
		CHECK(hr, S_OK);

	// Verify correct returned properties
	TESTC(!((cPropSets!=1 || rgPropSets==NULL || rgPropSets[0].rgProperties==NULL) ||
		(rgPropSets[0].rgProperties[0].dwPropertyID!=PropertyID)));

	//Memcpy (assignment the structure)
	*pProp = rgPropSets[0].rgProperties[0];
	
	//Copy the outofline info...
	VariantInit(&pProp->vValue);
	VariantCopy(&pProp->vValue, &rgPropSets[0].rgProperties[0].vValue);
	fSuccess = TRUE;
	
CLEANUP:
	FreeProperties(&cPropSets,&rgPropSets);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICommandProperties);
	SAFE_RELEASE(pISessionProperties);
	SAFE_RELEASE(pIDBProperties);
	return fSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CreateVariant
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CreateVariant(VARIANT* pVariant, DBTYPE wType, void* pValue)
{
	//pValue can be NULL, such as a (void*)ulValue or (void*)VARIANT_FALSE
	ASSERT(pVariant);
	HRESULT hr = S_OK;

	//Variant property value
	//Since a Variant is a "Union" all values fall into the same place
	//So all values are at the same memory location as I4
	V_VT(pVariant)	= wType;
#ifdef _WIN64
	V_BYREF(pVariant) = reinterpret_cast<PVOID>(pValue);
#else
	V_I4(pVariant)	= (LONG)pValue;
#endif

	//Specical Cases that need extra handling...
	switch(wType)
	{
		case VT_UNKNOWN:
		case VT_DISPATCH:
			//Need to AddRef the pointer, since VariantClear Releases it...
			SAFE_ADDREF((IUnknown*)pValue);
			break;

		case DBTYPE_WSTR:
		case VT_BSTR:
			//Need to Alloc a buffer, since VariantClear Frees it...
			V_BSTR(pVariant) = SysAllocString((WCHAR*)pValue);
			break;

		case DBTYPE_VARIANT:
			TESTC_(hr = VariantCopy(pVariant, (VARIANT*)pValue),S_OK);
			break;

		case DBTYPE_R4:
			//Need to fill in the fltVal field of variant.
			V_R4(pVariant) = (FLOAT) (LONG) PtrToLong(pValue);
			break;

		case DBTYPE_R8:
			//Need to fill in the dblVal field of variant.
			V_R8(pVariant) = (DOUBLE) (LONG) PtrToLong(pValue);
			break;

		case DBTYPE_DATE:
			//Need to fill in the date field of variant.
			V_DATE(pVariant) = (LONG) PtrToLong(pValue);
			break;
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT SetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT SetProperty(DBPROPID PropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, DBTYPE wType, ULONG_PTR ulValue, DBPROPOPTIONS dwOptions, const DBID& colid)
{
	if(wType == DBTYPE_BOOL)
		ulValue = ulValue ? VARIANT_TRUE : VARIANT_FALSE;
	return SetProperty(PropertyID, guidPropertySet, pcPropSets, prgPropSets, wType, &ulValue, dwOptions, colid);
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT SetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT SetProperty(DBPROPID PropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions, const DBID& colid)
{
	ASSERT(PropertyID && prgPropSets && pcPropSets);
	HRESULT hr = S_OK;

	ULONG cProperties = 0;
	DBPROP* rgProperties = NULL;

	//Make our lives a little easier
	ULONG cPropSets = *pcPropSets;
	DBPROPSET* rgPropSets = *prgPropSets;
	VARIANT* pVariant = NULL;

	ULONG iPropSet = ULONG_MAX;
	
	//Find the correct PropSet structure to add the property to
	for(ULONG i=0; i<cPropSets; i++)
		if(guidPropertySet == rgPropSets[i].guidPropertySet)
			iPropSet = i;

	//Do we need to create another PropSets structure for this property
	if(iPropSet==ULONG_MAX)
	{
		iPropSet = cPropSets;
		SAFE_REALLOC(rgPropSets, DBPROPSET, cPropSets+1);
		rgPropSets[iPropSet].cProperties = 0;
		rgPropSets[iPropSet].rgProperties = NULL;
		rgPropSets[iPropSet].guidPropertySet = guidPropertySet;
		cPropSets++;
	}

	//Now make our lives really easy
	cProperties = rgPropSets[iPropSet].cProperties;
	rgProperties = rgPropSets[iPropSet].rgProperties;

	//do we need to enlarge the list
	SAFE_REALLOC(rgProperties, DBPROP, cProperties+1);
	
	//Add the new property to the list
	rgProperties[cProperties].dwPropertyID = PropertyID;
	rgProperties[cProperties].dwOptions    = dwOptions;
	rgProperties[cProperties].colid        = colid;
	pVariant = &rgProperties[cProperties].vValue;
	VariantInit(pVariant);
	
	//Status is supposed to be ignored on input
	rgProperties[cProperties].dwStatus     = INVALID(DBPROPSTATUS);

	//Create the Variant
	switch(wType)
	{
		//Unfortunatly BOOL, I2, and I4 expect to have the "Address" passed
		//This makes use to have to have special handling to take the Address.
		//This should be changed to just be a void*, but would need changing
		//all calling sites which are quite a few...
		case DBTYPE_BOOL:
			ASSERT(pv);
			CreateVariant(pVariant, wType, (void*)*(VARIANT_BOOL*)pv);
			break;
		
		case DBTYPE_I2:
			ASSERT(pv);
			CreateVariant(pVariant, wType, (void*)*(SHORT*)pv);
			break;

		case DBTYPE_I4:
			ASSERT(pv);
			CreateVariant(pVariant, wType, (void *)*(LONG_PTR *)pv);
			break;
	
		default:
			CreateVariant(pVariant, wType, (void*)pv);
			break;
	}

	//Increment the number of properties
	cProperties++;

CLEANUP:
	//Now go back to the rediculous property structures
	rgPropSets[iPropSet].cProperties  = cProperties;
	rgPropSets[iPropSet].rgProperties = rgProperties;
	*pcPropSets = cPropSets;
	*prgPropSets = rgPropSets;
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CompactProperties
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CompactProperties(ULONG* pcPropSets, DBPROPSET** prgPropSets, DBTYPE wType)
{
	ASSERT(pcPropSets);
	ASSERT(prgPropSets);

	//Make our life easier (and faster)...
	ULONG cPropSets = *pcPropSets;
	DBPROPSET* rgPropSets = *prgPropSets;
	HRESULT hr = S_OK;

	//There are a couple of approaches to this.  The simplest and fastest is to just walk 
	//the list and "compact" or remove the default properties.  This doesn't require any more
	//allocations, no copies, no extra free's, and doesn't require any sets of property variables,
	//or bugs related to copying/combining properties into a new set...

	//Trace Properties before compacting...
	VerifyProperties(hr, cPropSets, rgPropSets, FALSE, TRUE);
	for(ULONG iPropSet=0; iPropSet<cPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];
		
		//Loop though all the properties in this set...
		for(ULONG iProp=0; iProp<pPropSet->cProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			if(pProp->vValue.vt == wType)
			{
				//"Compact" this property from the list
				if(iProp < (pPropSet->cProperties-1))
					memmove(&pPropSet->rgProperties[iProp], &pPropSet->rgProperties[iProp+1], (pPropSet->cProperties-iProp-1)*sizeof(DBPROP));
				
				pPropSet->cProperties--;
				iProp--;
			}
		}
		
		if(pPropSet->cProperties == 0)
		{
			//"Compact" this property set, if all properties were compacted...
			if(iPropSet < (cPropSets-1))
				memmove(&rgPropSets[iPropSet], &prgPropSets[iPropSet+1], (cPropSets-iPropSet-1)*sizeof(DBPROPSET));
	
			cPropSets--;
			iPropSet--;
		}
	}

	//Trace Properties After compacting...
	VerifyProperties(hr, cPropSets, rgPropSets, FALSE, TRUE);
	
	//Restore
	*pcPropSets = cPropSets;
	*prgPropSets = rgPropSets;
	return hr;
}


//--------------------------------------------------------------------
// @func VOID | FreeProperties
//
// This function should return TRUE if the fuction succeeded and 
// free all the memory in a DBPROPSET structure.
//
//--------------------------------------------------------------------
BOOL FreeProperties(ULONG* pcPropSets,DBPROPSET** prgPropSets)
{
	TBEGIN
	ASSERT(pcPropSets && prgPropSets);

	//Make our lives easier...
	ULONG cPropSets = *pcPropSets;
	DBPROPSET* rgPropSets = *prgPropSets;

	//Loop over all property sets
	for(ULONG iPropSet=0; iPropSet<cPropSets && rgPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];
		
		//Loop over all the properties of this set
		for(ULONG iProp=0; iProp<pPropSet->cProperties && pPropSet->rgProperties; iProp++)
		{	
			DBPROP* pProp = &pPropSet->rgProperties[iProp];	

			//According to the spec, vValue must be a valid value or VT_EMPTY
			ReleaseDBID(&pProp->colid, FALSE);	
			GCHECK(VariantClear(&pProp->vValue),S_OK);
		}

		SAFE_FREE(pPropSet->rgProperties);
		pPropSet->cProperties = 0;
	}
	
	// Now free the outer structure
	SAFE_FREE(rgPropSets);
	*prgPropSets = NULL;
    *pcPropSets = 0;
	return TRUE;
}

//--------------------------------------------------------------------
// @func VOID | FreeProperties
//
// This function should return TRUE if the fuction succeeded and 
// free all the memory in a DBPROPINFOSET structure.
//
//--------------------------------------------------------------------
BOOL FreeProperties(ULONG* pcPropInfoSets,DBPROPINFOSET** prgPropInfoSets, OLECHAR** ppDescBuffer)
{
	TBEGIN
	ASSERT(pcPropInfoSets && prgPropInfoSets);
	ULONG cPropInfoSets = *pcPropInfoSets;
	DBPROPINFOSET* rgPropInfoSets = *prgPropInfoSets;

	// Free the inner DBPROPINFO first
	for(ULONG i=0; i<cPropInfoSets; i++)
	{
		for(ULONG j=0; j<rgPropInfoSets[i].cPropertyInfos; j++)
		{
			//According to the spec, vValue is undefined if NOTSUPPORTED
			if(rgPropInfoSets[i].rgPropertyInfos[j].dwFlags != DBPROPFLAGS_NOTSUPPORTED)
				GCHECK(VariantClear(&(rgPropInfoSets[i].rgPropertyInfos[j].vValues)),S_OK);
		}

		CoTaskMemFree(rgPropInfoSets[i].rgPropertyInfos);
		rgPropInfoSets[i].cPropertyInfos = 0;
	}
	
	// Now free the outer structure
	CoTaskMemFree(rgPropInfoSets);
	*prgPropInfoSets = NULL;
    *pcPropInfoSets = 0;

	if(ppDescBuffer)
	{
		CoTaskMemFree(*ppDescBuffer);
		*ppDescBuffer = NULL;
	}
	return TRUE;
}

//--------------------------------------------------------------------
// @func VOID | FreeProperties
//
// This function should return TRUE if the fuction succeeded and 
// free all the memory in a DBPROPIDSET structure.
//
//--------------------------------------------------------------------
BOOL FreeProperties(ULONG* pcPropIDSets,DBPROPIDSET** prgPropIDSets)
{
	ASSERT(pcPropIDSets && prgPropIDSets);
	ULONG cPropIDSets = *pcPropIDSets;
	DBPROPIDSET* rgPropIDSets = *prgPropIDSets;

	if(cPropIDSets)
	{
		ASSERT(rgPropIDSets);
	}
		
	// Free the inner DBPROPID first
	for(ULONG i=0; i<cPropIDSets; i++)
	{
		CoTaskMemFree(rgPropIDSets[i].rgPropertyIDs);
		rgPropIDSets[i].cPropertyIDs = 0;
	}
	
	// Now free the outer structure
	CoTaskMemFree(rgPropIDSets);
	*prgPropIDSets = NULL;
    *pcPropIDSets = 0;
	return TRUE;
}

//--------------------------------------------------------------------
// @func HRESULT | SetRowsetProperty
//
// Set Property, returns result of either ICommand::QI for 
// ICommandProperty or ICommandProperties::SetProperties.
// Does all the work of build correct property structures.
// Just tell it what property set and property. 
//
//--------------------------------------------------------------------
HRESULT SetRowsetProperty(
	IUnknown*	pIUnknown,		// @parm  [IN] Command ptr
	GUID		gPropertySet,	// @parm  [IN] Property Set
	DBPROPID	dbPropertyID,	// @parm  [IN] Property
	BOOL		fSetFlag, 		// @parm  [IN] SET or UNSET
	enum DBPROPOPTIONSENUM dwOptions, // @parm  [IN] Option
	BOOL		fLogFailure		// @parm  [IN] Send failure msg to log
)
{
	HRESULT				 hr = E_FAIL;
	ICommandProperties * pICommandProperties;
	DBPROPSET			 dbPropSet[1];
	DBPROP				 dbProp[1];

	if (!pIUnknown)
		return E_FAIL;

	// Status not set but have to check when it comes back
	dbPropSet[0].guidPropertySet = gPropertySet;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;
	dbPropSet[0].rgProperties[0].dwPropertyID=dbPropertyID;
	dbPropSet[0].rgProperties[0].dwOptions=dwOptions;
	dbPropSet[0].rgProperties[0].colid = DB_NULLID;
	dbPropSet[0].rgProperties[0].vValue.vt	= VT_BOOL;
	
	// Set or Unset the Property
	if (fSetFlag)
		V_BOOL(&dbPropSet[0].rgProperties[0].vValue) = VARIANT_TRUE;
	else
		V_BOOL(&dbPropSet[0].rgProperties[0].vValue) = VARIANT_FALSE;
	
	if (!VerifyInterface(pIUnknown, IID_ICommandProperties, COMMAND_INTERFACE, (IUnknown**)&pICommandProperties))
		return E_NOINTERFACE;

	hr=pICommandProperties->SetProperties(1, dbPropSet);
	SAFE_RELEASE(pICommandProperties);

	if (PropertyStatus(dbPropSet[0].rgProperties[0].dwStatus))
		PRVTRACE(L"Property set successfully\n");
	else if (fLogFailure)
	{
		odtLog << L"Property ["
			<< dbPropertyID
			<< L"] failure: " 
			<< dbPropSet[0].rgProperties[0].dwStatus 
			<< ENDL;
	}

	return hr;
}

//--------------------------------------------------------------------
// @func HRESULT | SetRowsetProperty
//
// Set Property, returns result of either ICommand::QI for 
// ICommandProperty or ICommandProperties::SetProperties.
// Does all the work of build correct property structures.
// Just tell it what property set and property. 
//
//--------------------------------------------------------------------
HRESULT SetRowsetProperty(
	IUnknown*	pIUnknown,		// @parm  [IN] Command ptr
	GUID		gPropertySet,	// @parm  [IN] Property Set
	DBPROPID	dbPropertyID,	// @parm  [IN] Property
	LONG_PTR	dwPropValue,	// @parm  [IN] Property value
	enum DBPROPOPTIONSENUM dwOptions, // @parm  [IN] Option
	BOOL		fLogFailure		// @parm  [IN] Send failure msg to log
)
{
	HRESULT				 hr = E_FAIL;
	ICommandProperties * pICommandProperties;
	DBPROPSET			 dbPropSet[1];
	DBPROP				 dbProp[1];

	if (!pIUnknown)
		return E_FAIL;

	// Status not set but have to check when it comes back
	dbPropSet[0].guidPropertySet = gPropertySet;
	dbPropSet[0].cProperties = 1;
	dbPropSet[0].rgProperties = dbProp;
	dbPropSet[0].rgProperties[0].dwPropertyID=dbPropertyID;
	dbPropSet[0].rgProperties[0].dwOptions=dwOptions;
	dbPropSet[0].rgProperties[0].colid = DB_NULLID;
	dbPropSet[0].rgProperties[0].vValue.vt	= VT_I4;
	
	// Set the Property
	V_I4(&dbPropSet[0].rgProperties[0].vValue) = (LONG) dwPropValue;
	
	if (!VerifyInterface(pIUnknown, IID_ICommandProperties, COMMAND_INTERFACE, (IUnknown**)&pICommandProperties))
		return E_NOINTERFACE;

	hr=pICommandProperties->SetProperties(1, dbPropSet);
	SAFE_RELEASE(pICommandProperties);

	if (PropertyStatus(dbPropSet[0].rgProperties[0].dwStatus))
		PRVTRACE(L"Property set successfully\n");
	else if (fLogFailure)
	{
		odtLog << L"Property ["
			<< dbPropertyID
			<< L"] failure: " 
			<< dbPropSet[0].rgProperties[0].dwStatus 
			<< ENDL;
	}

	return hr;
}

//--------------------------------------------------------------------
// @func BOOL | PropertyStatus
//
// Print Property Status, returns FALSE is status is not OK
//
//--------------------------------------------------------------------
BOOL PropertyStatus(
	DBPROPSTATUS dbPropStatus	// @parm  [IN] Property Status
)
{
	switch(dbPropStatus)
	{
		// Status 0
		case DBPROPSTATUS_OK:  
				PRVTRACE(L"DBPROPSTATUS_OK\n");
				return TRUE;
		// Status 1
		case DBPROPSTATUS_NOTSUPPORTED: 
				PRVTRACE(L"DBPROPSTATUS_NOTSUPPORTED\n");
				return FALSE;
		// Status 2
		case DBPROPSTATUS_BADVALUE: 
				PRVTRACE(L"DBPROPSTATUS_BADVALUE\n");
				return FALSE;
		// Status 3
		case DBPROPSTATUS_BADOPTION:
				PRVTRACE(L"DBPROPSTATUS_BADOPTION\n");
				return FALSE;
		// Status 4
		case DBPROPSTATUS_BADCOLUMN:
				PRVTRACE(L"DBPROPSTATUS_BADCOLUMN\n");
				return FALSE;
		// Status 5
		case DBPROPSTATUS_NOTALLSETTABLE:
				PRVTRACE(L"DBPROPSTATUS_NOTALLSETTABLE\n");
				return FALSE;
		// Status 6
		case DBPROPSTATUS_NOTSETTABLE:
				PRVTRACE(L"DBPROPSTATUS_NOTSETTABLE\n");
				return FALSE;
		// Status 7
		case DBPROPSTATUS_NOTSET:
				PRVTRACE(L"DBPROPSTATUS_NOTSET\n");
				return FALSE;
		// Status 8
		case DBPROPSTATUS_CONFLICTING:
				PRVTRACE(L"DBPROPSTATUS_CONFLICTING\n");
				return FALSE;
		default:
				PRVTRACE(L"Property status not known\n");
				return FALSE;
	}
}

//--------------------------------------------------------------------
// @func BOOL | RowsetBindingStatus
//
// Print Binding Status, returns FALSE is status is not OK
//
//--------------------------------------------------------------------
BOOL RowsetBindingStatus(
	DBSTATUS dbStatus			// @parm  [IN] Binding Status
)
{
	switch(dbStatus)
	{
		// Status 0
		case DBSTATUS_S_OK:  
				PRVTRACE(L"DBSTATUS_OK\n");
				return TRUE;
		// Status 1
		case DBSTATUS_E_BADACCESSOR: 
				PRVTRACE(L"DBSTATUS_BADACCESSOR\n");
				return FALSE;
		// Status 2
		case DBSTATUS_E_CANTCONVERTVALUE: 
				PRVTRACE(L"DBSTATUS_CANTCONVERTVALUE\n");
				return FALSE;
		// Status 3
		case DBSTATUS_S_ISNULL:
				PRVTRACE(L"DBSTATUS_ISNULL\n");
				return FALSE;
		// Status 4
		case DBSTATUS_S_TRUNCATED:
				PRVTRACE(L"DBSTATUS_TRUNCATED\n");
				return FALSE;
		// Status 5
		case DBSTATUS_E_SIGNMISMATCH:
				PRVTRACE(L"DBSTATUS_SIGNMISMATCH\n");
				return FALSE;
		// Status 6
		case DBSTATUS_E_DATAOVERFLOW:
				PRVTRACE(L"DBSTATUS_DATAOVERFLOW\n");
				return FALSE;
		// Status 7
		case DBSTATUS_E_CANTCREATE:
				PRVTRACE(L"DBPROPSTATUS_CANTCREATE\n");
				return FALSE;
		// Status 8
		case DBSTATUS_E_UNAVAILABLE:
				PRVTRACE(L"DBPROPSTATUS_UNAVAILABLE\n");
				return FALSE;
		// Status 9
		case DBSTATUS_E_PERMISSIONDENIED:
				PRVTRACE(L"DBSTATUS_E_PERMISSIONDENIED\n");
				return FALSE;
		// Status 10
		case DBSTATUS_E_INTEGRITYVIOLATION:
				PRVTRACE(L"DBPROPSTATUS_INTEGRITYVIOLATION\n");
				return FALSE;
		// Status 11
		case DBSTATUS_E_SCHEMAVIOLATION:
				PRVTRACE(L"DBSTATUS_SCHEMAVIOLATION\n");
				return FALSE;
		// Status 12
		case DBSTATUS_E_BADSTATUS:
				PRVTRACE(L"DBPROPSTATUS_BADSTATUS\n");
				return FALSE;
		// Status 13
		case DBSTATUS_S_DEFAULT:
				PRVTRACE(L"DBSTATUS_S_DEFAULT\n");
				return FALSE;
		default:
				PRVTRACE(L"Binding status not known\n");
				return FALSE;
	}
}

//--------------------------------------------------------------------
// @func HRESULT | SetCommandText
//
// Set Command Text, returns result of ICommand::QI for ICommandText or 
// ICommandText::SetCommandText or CTable::SOMEFUNCTION. Command text can
// be set a couple of ways: 1) request any StmtKd or 2) pass StmtKd as eSQL 
// and pass in your own text in sqlStmt. 
//
//--------------------------------------------------------------------
HRESULT SetCommandText(
	IMalloc	*		pIMalloc,		//@parm [IN] Malloc ptr to free statement
	ICommand *		pICommand,		//@parm [IN] Command ptr
	CTable *		pCTable,			//@parm [IN] CTable ptr to use CreateSqlStmt function
	WCHAR *			pTableName2,	//@parm [IN] 2nd CTable ptr if EQUERY requires 2 tables
	STATEMENTKIND	StmtKd,			//@parm [IN] Kind of Statement
	EQUERY			sqlStmt,		//@parm [IN] EQUERY from Private Library
	WCHAR *			pStmt,			//@parm [IN] Text of statement if StmtKd == eSQL
	DBORDINAL *		pcColumns,		//@parm [IN/OUT] Count of columns.
	DB_LORDINAL **	prgColumns,		//@parm [IN/OUT] Array of column numbers
	CTable *		pTable2,		//@parm [IN] Second table object, if needed 
	WCHAR**			ppCmdText		//@parm [OUT] Cmd Text set.
)
{
	HRESULT			hr				= E_FAIL;
	BOOL			fSucceed		= FALSE;
	ICommandText *	pICommandText	= NULL;
	WCHAR *			sql				= NULL;
	WCHAR *			const_sql		= NULL;
	WCHAR *			stmt1			= NULL;
	WCHAR *			stmt2			= NULL;
	IUnknown *		pSessionIUnknown = NULL;

	if (!pICommand)
		return E_FAIL;

	switch(StmtKd)
	{
		case eSELECT:
			if (!pCTable)
				goto CLEANUP;
			if (FAILED(hr=pCTable->CreateSQLStmt(sqlStmt,pTableName2,&sql,pcColumns,prgColumns,1,pTable2)))
				goto CLEANUP;
			break;

		case eINSERT:
			if (!pCTable)
				goto CLEANUP;
			TESTC_(pCTable->CreateSQLStmt(INSERT_ROW_WITH_LITERALS,pTableName2,&sql, pcColumns, prgColumns,
				pCTable->GetNextRowNumber(), pTable2), S_OK);
			break;

		case eDELETE:
			if (!pCTable)
				goto CLEANUP;
			if (FAILED(hr=pCTable->Delete((pCTable->GetNextRowNumber())-1,PRIMARY,FALSE,&sql)))
				goto CLEANUP;
			break;

		case eUPDATE:
			if (!pCTable)
				goto CLEANUP;
			if (FAILED(hr=pCTable->Update(0,SECONDARY,FALSE,&sql)))
				goto CLEANUP;
			break;

		case eSelectERROR:
			sql = (WCHAR *) wszSelectERROR;
			break;

		case eInsertERROR:
			sql = (WCHAR *) wszInsertERROR;
			break;

		case eNOCOMMAND:
			hr = NOERROR;
			break;

		case eSQL:
			if (pStmt)
 				sql = pStmt;
			break;

		default:
			odtLog << wszDefaultError << ENDL;
			hr = E_FAIL;
			break;
	}

	// Make sure we have pointer
	if (!pICommand)
		goto CLEANUP;

	// Get commandtext
	if (!VerifyInterface(pICommand, IID_ICommandText, COMMAND_INTERFACE, (IUnknown**)&pICommandText))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	if(pICommand->GetDBSession(IID_IUnknown, &pSessionIUnknown)!=S_OK)
		goto CLEANUP;

	// Set text (for NO_COMMAND the sql pointer is NULL)
	hr=pICommandText->SetCommandText(DBGUID_DBSQL,sql);

CLEANUP:
	SAFE_RELEASE(pSessionIUnknown);
	SAFE_RELEASE(pICommandText);

	PROVIDER_FREE(stmt1);
	PROVIDER_FREE(stmt2);

	if(ppCmdText && sql)
		*ppCmdText = wcsDuplicate(sql);

	if (!pStmt && (eSelectERROR!=StmtKd) && (eInsertERROR!=StmtKd))
		PROVIDER_FREE(sql);

	return hr;
}

//--------------------------------------------------------------------
// @func HRESULT | PrepareCommand
//
// PrepareCommand, returns result of either ICommand::QI for ICommandPrepare 
// or for ICommand::methods (Prepare or/and Unprepare
//
//--------------------------------------------------------------------
HRESULT PrepareCommand(
	ICommand *		pICommand,		// @parm  [IN] Command ptr					
	EPREPARE		ePrepare,		// @parm  [IN] Prepare, Unprepare, or Both
	ULONG			cExpectedRuns	// @parm  [IN] Passed to Unprepare only
)
{
	HRESULT				hr				 = E_FAIL;
	ICommandPrepare *	pICommandPrepare = NULL;

	if (!pICommand)
		return E_FAIL;

	if (!VerifyInterface(pICommand, IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown**)&pICommandPrepare))
		return NOERROR;

	switch(ePrepare)
	{
		case PREPARE:
			if (SUCCEEDED(hr=pICommandPrepare->Prepare(cExpectedRuns)))
				PRVTRACE(L"%s\n",wszPrepGood);
			break;

		case UNPREPARE:
 			if (SUCCEEDED(hr=pICommandPrepare->Unprepare()))
				PRVTRACE(L"%s\n",wszUnprepGood);
			break;
		
		case BOTH:
			if (SUCCEEDED(hr=pICommandPrepare->Prepare(cExpectedRuns)))
			{	
				PRVTRACE(L"%s\n",wszPrepGood);
				
 				if(SUCCEEDED(hr=pICommandPrepare->Unprepare()))
					PRVTRACE(L"%s\n",wszUnprepGood);
			}
			break;
		
		case NEITHER:
			hr = S_OK;
			break;
		
		default:
			odtLog << wszDefaultError << ENDL;
			hr = E_FAIL;
			break;
	}

	// Release the interface
	SAFE_RELEASE(pICommandPrepare);
	return hr;
}

//--------------------------------------------------------------------
// @func BOOL | IsFixedLength
//
// Returns TRUE is type is fixed length 
//
//--------------------------------------------------------------------
BOOL IsFixedLength(
	DBTYPE wType	// @parm  [IN] type 
)
{
	// Strip off the Flags
	(wType &= ~(DBTYPE_BYREF | DBTYPE_VECTOR | DBTYPE_ARRAY | DBTYPE_RESERVED));
	
	switch(wType)
	{
		case DBTYPE_STR:
		case DBTYPE_WSTR:
		case DBTYPE_VARNUMERIC:
		case DBTYPE_BYTES:
			//There are far fewer "variable" types then fixed, so if its varible,
			//then its not fixed...
			return FALSE;
	}

	//Otherwise we assume its variable...
	return TRUE;
}


//--------------------------------------------------------------------
// @func BOOL | IsNumericType
//
// Returns TRUE is type is numeric
//
//--------------------------------------------------------------------
BOOL IsNumericType(
	DBTYPE wType	// @parm  [IN] type 
)
{
	// Strip off the Flags
	wType &= ~(DBTYPE_BYREF | DBTYPE_VECTOR | DBTYPE_ARRAY | DBTYPE_RESERVED);
	
	// Check to see if the DBType is Numeric
	switch(wType)
	{
		case DBTYPE_NUMERIC:	// Numeric, Decimal
		case DBTYPE_DECIMAL:
		case DBTYPE_I8:			// Bigint
		case DBTYPE_I4:			// Integer
		case DBTYPE_I2:			// Smallint
		case DBTYPE_I1:			// Tinyint
		case DBTYPE_R8:			// Float, Double
		case DBTYPE_R4:			// Real
		case DBTYPE_UI1:
		case DBTYPE_UI2:
		case DBTYPE_UI4:
		case DBTYPE_UI8:
		case DBTYPE_CY:
		case DBTYPE_VARNUMERIC: // DB_VARNUMERIC
			return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------
// @func BOOL | IsScaleType
//
// Returns TRUE is type can contain a scale
//
//--------------------------------------------------------------------
BOOL IsScaleType(
	DBTYPE wType	// @parm  [IN] type 
)
{
	// Strip off the Flags
	wType &= ~(DBTYPE_BYREF | DBTYPE_VECTOR | DBTYPE_ARRAY | DBTYPE_RESERVED);
	
	// Check to see if the DBType is Numeric
	switch(wType)
	{
		case DBTYPE_NUMERIC:	
		case DBTYPE_DECIMAL:
		case DBTYPE_DBTIMESTAMP:
		case DBTYPE_VARNUMERIC:
			return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// @func BYTE |
// MiscFunc bNumericPrecision |
// Returns Precision of the type
//
//-----------------------------------------------------------------------------
BYTE bNumericPrecision(
	DBTYPE wType,	// @parm  [IN] type 
	CCol & rcol		// @parm  [IN] CCol ref
)
{
	// Strip off the Flags
	wType &= ~(DBTYPE_BYREF | DBTYPE_VECTOR | DBTYPE_ARRAY | DBTYPE_RESERVED);
	
	// Check to see if the DBType is Numeric
	switch(wType)
	{
		case DBTYPE_I1:			// Tinyint
		case DBTYPE_UI1:
			return 3;
		
		case DBTYPE_I2:			// Smallint
		case DBTYPE_UI2:
			return 5;
		
		case DBTYPE_I4:			// Integer
		case DBTYPE_UI4:
			return 10;
		
		case DBTYPE_I8:			// Bigint
			return 19;
		
		case DBTYPE_UI8:
			return 20;
		
		case DBTYPE_R4:			// Real
			ASSERT(rcol.GetPrecision() <= 7);
			return rcol.GetPrecision();
		
		case DBTYPE_R8:			// Float/Double
			ASSERT(rcol.GetPrecision() <= 15);
			return rcol.GetPrecision();
		
		case DBTYPE_CY:			// Currency
			ASSERT(rcol.GetPrecision() <= 19);
				return rcol.GetPrecision();
		
		case DBTYPE_DECIMAL:	// Decimal
			ASSERT(rcol.GetPrecision() <= 29);
			return rcol.GetPrecision();

		case DBTYPE_NUMERIC:	// Numeric
			ASSERT(rcol.GetPrecision() <= 39);
			return rcol.GetPrecision();

		case DBTYPE_VARNUMERIC: // VarNumeric
			return rcol.GetPrecision();
		
		default:
			return ~0;
	}
}

//--------------------------------------------------------------------
// @func BOOL | AddCharToVarNumericVal
//
// Uses 64-bit arithmetic
// Returns TRUE if successfull.
//
//--------------------------------------------------------------------
BOOL AddCharToVarNumericVal(
	WCHAR wLetter,
	DB_VARNUMERIC * pVarNumeric,
	USHORT	cbVarNumericValSize
)
{
	BOOL fPass = FALSE;
	ULONG	cbTempValBuf, ul;
	BYTE *pbVal;

	WCHAR pwszBuf[2];
	pwszBuf[0]  = wLetter;
	pwszBuf[1]  = L'\0';;
	LONG lDigit = _wtol(pwszBuf);
	DWORDLONG dwlAccum = lDigit;

	ASSERT(iswdigit(wLetter) && pVarNumeric && (cbVarNumericValSize > 0));
	cbTempValBuf = (cbVarNumericValSize % 4) ? cbVarNumericValSize + (4 - cbVarNumericValSize % 4) : cbVarNumericValSize ;
	pbVal = (BYTE *)PROVIDER_ALLOC(cbTempValBuf);
	
	// Check parameters
	if (!pVarNumeric || !pbVal)
		goto CLEANUP;

	memset(pbVal, 0, cbTempValBuf);
	memcpy(pbVal, pVarNumeric->val, cbVarNumericValSize);

	for(ul = 0; ul < (cbTempValBuf)/ sizeof(ULONG); ul++ )
	{
		dwlAccum +=(DWORDLONG)(*(((UNALIGNED ULONG *)pbVal) + ul)) * 10;
		*(((UNALIGNED ULONG *)pbVal) + ul) = (ULONG)dwlAccum;
		dwlAccum >>= sizeof(ULONG) * 8;
	}

	//	Adjust length if overflow into next byte
	if (pVarNumeric->precision < cbVarNumericValSize && *(pVarNumeric->val + pVarNumeric->precision) != 0)
		pVarNumeric->precision++;

	// check for overflow
	for (ul=cbVarNumericValSize;ul<cbTempValBuf;ul++)
	{
		if(pbVal[ul] != 0)
			goto CLEANUP;
	}

	// everything worked out fine
	fPass = TRUE;

	// copy the results
	memcpy(pVarNumeric->val, pbVal, cbVarNumericValSize);

CLEANUP:
	PROVIDER_FREE(pbVal);
	return fPass;
}

//--------------------------------------------------------------------
// @func BOOL | AddCharToNumericVal
//
// Uses 64-bit arithmetic
// Returns TRUE if successfull.
//
//--------------------------------------------------------------------
BOOL AddCharToNumericVal(
	WCHAR wLetter,
	DB_NUMERIC * pNumeric
)
{
	ASSERT(iswdigit(wLetter) && pNumeric);
	
	// Check parameters
	if (!pNumeric)
		return FALSE;

	// Convert WCHAR to ULONG
	WCHAR pwszBuf[2];
	pwszBuf[0]  = wLetter;
	pwszBuf[1]  = L'\0';;
	LONG lDigit = _wtol(pwszBuf);

	// If operation won't overflow
	if (pNumeric->precision <= sizeof(DWORDLONG) - 1)
	{
		*(UNALIGNED DWORDLONG *)pNumeric->val *= 10;
		*(UNALIGNED DWORDLONG *)pNumeric->val += lDigit;
	}
	else
	{
		DWORDLONG dwlAccum = lDigit;

		for(ULONG ul = 0; ul < sizeof(pNumeric->val) / sizeof(ULONG); ul++ )
		{
			dwlAccum +=(DWORDLONG)(*(((UNALIGNED ULONG *)pNumeric->val) + ul)) * 10;
			*(((UNALIGNED ULONG *)pNumeric->val) + ul) = (ULONG)dwlAccum;
			dwlAccum >>= sizeof(ULONG) * 8;
		}
	}

	//	Adjust length if overflow into next byte
	if (pNumeric->precision < sizeof(pNumeric->val) && *(pNumeric->val + pNumeric->precision) != 0)
		pNumeric->precision++;

	return TRUE;
}

//--------------------------------------------------------------------
// @func BOOL | CompareStrings
//
// Returns BOOL.
//
//--------------------------------------------------------------------
BOOL CompareStrings
(
	const WCHAR * pwszString1,	// @parm [IN] Provider string
	const WCHAR * pwszString2,	// @parm [IN] Const Static string
	BOOL		  fPrint		// @parm [IN] Print the Provider string to the screen
)
{
	// Find out what LCID the user has set
	WORD wLangID = PRIMARYLANGID(GetUserDefaultLangID());
	BOOL fReturn = FALSE;

	// Check the LangID or the Print flag
	if (wLangID == LANG_ENGLISH)
	{
		//Protect agasint NULL pointers
		if(pwszString1 && pwszString2)
			fReturn = (wcscmp(pwszString1, pwszString2)==0);
		else
			fReturn = (pwszString1 == pwszString2);
	}

	// If not US Print to the Screen
	if ((wLangID != LANG_ENGLISH) || fPrint) 
	{
		odtLog << L" Non-US String: " << (pwszString1 ? pwszString1 : L"(null)") << L" Expected " << (pwszString2 ? pwszString2 : L"(null)") << L"\n";
		fReturn = TRUE;
	}

	return fReturn;
}

// taken from Extralib, required by CheckInterface
ULONG GetRefCount(IUnknown* pIUnknown)
{
	if(pIUnknown == NULL)
		return 0;

	pIUnknown->AddRef();
	return pIUnknown->Release();
}

// taken from Extralib, required by CheckInterface
ULONG SetRefCount(IUnknown* pIUnknown, LONG iCount)
{
	if(pIUnknown == NULL)
		return 0;
	
	ULONG ulRefCount = 0;

	//When adjusting the Reference Count, we cannot got directly
	//by the value returned from AddRef/Release.  First COM/OLE don't 
	//gaurentte accuratcy of these numbers beyond 0 and ~0, and also the 
	//Object could be in error.

	//If iCount > 0, Addref (iCount) times.
	//If iCount < 0, Release (iCount) times.

	//Should be higher than currently is
	if(iCount > 0)
	{
		for(LONG i=0; i<iCount; i++)
			ulRefCount = pIUnknown->AddRef();
	}
	//Should be lower than currently is
	else
	{
		for(LONG i=0; i>iCount; i--)
			ulRefCount = pIUnknown->Release(); 
	}
			
	return ulRefCount;
}


//--------------------------------------------------------------------
// @func BOOL | 
//	Miscfunc DefaultObjectTesting |
//	The function checks the validity of a interface belonging to an object (the interface is accessed thru a ptr)
//	It will check: 
//	- an interface which doesn't belong to the object that owns the interface
//	- mandatory interfaces on the object can be got thru QI
//	- can get IUnknown
//	- IID_NULL returns in error E_NOINTERFACE
//	- QI with a NULL ptr results in E_INVALIDARG
//	- reference count works
//
// Returns BOOL.
//
//--------------------------------------------------------------------
BOOL DefaultObjectTesting
(
	IUnknown*		pIUnkObject,			// [IN]	Interface (Object) to test
	EINTERFACE		eInterface,				// [IN] Interface Type of pIUnknown
	BOOL			fInitializedDSO			// [IN] Flag DSO initialized
)
{
	ULONG i,cInterfaces = 0;
	INTERFACEMAP* rgInterfaces = NULL;
	IUnknown* pIUnknown = NULL;
	ULONG ulOrgRefCount = 0;
	BOOL bReturn = FALSE;
	HRESULT hr = S_OK;

	if(!pIUnkObject)
		return FALSE;

	//Obtain a list of interfaces for this Object
	TESTC(GetInterfaceArray(eInterface, &cInterfaces, &rgInterfaces));
	
	//QUERYINTERFACE - for both Mandatory and Optional interfaces
	for(i=0; i<cInterfaces; i++)
	{
		//Special Case
		if ((IID_IDBCreateSession == *rgInterfaces[i].pIID || IID_IDBInfo == *rgInterfaces[i].pIID)
			&& DATASOURCE_INTERFACE == eInterface && !fInitializedDSO)
		{
			//Not - Initialized DataSource
			//Spec allows E_UNEXPECTED or E_NOINTERFACE for QI on an Unitialized DSO
			hr = pIUnkObject->QueryInterface(*rgInterfaces[i].pIID, (void**)&pIUnknown);
			if(!COMPARE(hr==E_UNEXPECTED || hr==E_NOINTERFACE, TRUE))
				odtLog << "ERROR: QueryInterface for " << GetInterfaceName(*rgInterfaces[i].pIID) << " failed!\n";
		}
		else
		{
			//Initialized DataSource
			if(rgInterfaces[i].fMandatory)
			{
				//[MANDATORY]			
				if(!CHECK(pIUnkObject->QueryInterface(*rgInterfaces[i].pIID, (void**)&pIUnknown),S_OK))
					odtLog << "ERROR: QueryInterface for " << GetInterfaceName(*rgInterfaces[i].pIID) << " failed!\n";
				
			}
			else
			{
				//[OPTIONAL]
				HRESULT hr = pIUnkObject->QueryInterface(*rgInterfaces[i].pIID, (void**)&pIUnknown);
				if(!COMPARE(hr==S_OK || hr==E_NOINTERFACE, TRUE))
					odtLog << "ERROR: QueryInterface for " << GetInterfaceName(*rgInterfaces[i].pIID) << " returned unexpected result!\n";
			}
		}
		SAFE_RELEASE(pIUnknown);
	}
	

	//QUERYINTERFACE - IID_NULL
	if(!CHECK(pIUnkObject->QueryInterface(IID_NULL, (void**)&pIUnknown),E_NOINTERFACE))
		odtLog << "QueryInterface(IID_NULL, &pIUnknown) failed!\n";
	SAFE_RELEASE(pIUnknown);

	//QUERYINTERFACE - with NULL output pointer 
	if(GetModInfo()->GetClassContext() & CLSCTX_INPROC_SERVER)
	{
		try 
		{ 
			hr = pIUnkObject->QueryInterface(IID_IUnknown, NULL); 
		}
		catch (...)
		{
			//Provider threw an exception
			odtLog<<L"Exception was thrown on IUnknown::QueryInterface(IID_IUnknown, NULL)!\n";
			//TERROR("Exception was thrown on IUnknown::QueryInterface(IID_IUnknown, NULL)!");
			//hr = E_FAIL;
		}

		//TO DO : Add in this line after the ATL bug is resolved. Until
		//then this would block testing.
		//Verify the results...
		//CHECK(hr, E_INVALIDARG);
	}

	//REFCOUNT - Testing
	ulOrgRefCount = GetRefCount(pIUnkObject);
	
	//AddRef a few times
	SetRefCount(pIUnkObject, 10);
	//Release a few times
	SetRefCount(pIUnkObject, -2);

	//Verify new RefCount
	TESTC(VerifyRefCounts(GetRefCount(pIUnkObject), ulOrgRefCount + (10-2)));
	//Restore the original ref count
	SetRefCount(pIUnkObject, -(10-2));
	bReturn = TRUE;

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	return bReturn;
}


//--------------------------------------------------------------------
// @func BOOL | 
//	Miscfunc DefaultInterfaceTesting |
//	This function verifies the basic functionality of an interface.
//	It also tests the object to which this interface belongs, by
//	calling DefaultObjectTesting.
//
//	*** WARNING: It is assumed by this function that the 
//	*** interface pointed to by pIUnkObject corresponds to the 
//	*** IID in riid.
//
// Returns BOOL.
//
//--------------------------------------------------------------------
BOOL DefaultInterfaceTesting
(
	IUnknown*		pIUnkObject,			// [IN]	Interface (Object) to test
	EINTERFACE		eInterface,				// [IN] Object Type of pIUnknown
	REFIID			riid					// [IN] Interface to test specifically
)
{
	TBEGIN

	ULONG			i,cInterfaceMaps = 0;
	INTERFACEMAP*	rgInterfaceMaps = NULL;
	HRESULT			hr = S_OK;
	IUnknown*		pIUnknown = NULL;

	//Invalid Arguments
	if(!pIUnkObject)
		return FALSE;

	//Obtain all the valid IIDs for this object...
	TESTC(GetInterfaceArray(eInterface, &cInterfaceMaps, &rgInterfaceMaps));

	//Obtain the desired interface...
	//NOTE:  This isn't an error if the object doesn't support the interface, since it might
	//not be required, VerifyInterface will determine if required and increment the error...
	TESTC(VerifyInterface(pIUnkObject, riid, eInterface, &pIUnknown))

	//Also test the object to which this interface belongs.
	TESTC(DefaultObjectTesting(pIUnkObject, eInterface))

	//IDBInitialize
	if(riid == IID_IDBInitialize)
	{
		//Delegate
		QTESTC(DefTestInterface((IDBInitialize*)pIUnkObject));
	}

	//IDBProperties
	else if(riid == IID_IDBProperties)
	{
		//Delegate
		QTESTC(DefTestInterface((IDBProperties*)pIUnkObject));
	}

	//IDBCreateSession
	else if(riid == IID_IDBCreateSession)
	{
		//Delegate
		QTESTC(DefTestInterface((IDBCreateSession*)pIUnkObject));
	}

	//IPersist
	else if(riid == IID_IPersist)
	{
		//Delegate
		QTESTC(DefTestInterface((IPersist*)pIUnkObject));
	}

	//IOpenRowset
	else if(riid == IID_IOpenRowset)
	{
		//Delegate
		QTESTC(DefTestInterface((IOpenRowset*)pIUnkObject));
	}

	//IGetDataSource
	else if(riid == IID_IGetDataSource)
	{
		//Delegate
		QTESTC(DefTestInterface((IGetDataSource*)pIUnkObject));
	}

	//ISessionProperties
	else if(riid == IID_ISessionProperties)
	{
		//Delegate
		QTESTC(DefTestInterface((ISessionProperties*)pIUnkObject));
	}

	//IRowsetInfo
	else if(riid == IID_IRowsetInfo)
	{
		//Delegate
		QTESTC(DefTestInterface((IRowsetInfo*)pIUnkObject));
	}

	//IRowset
	else if(riid == IID_IRowset)
	{
		//Delegate
		QTESTC(DefTestInterface((IRowset*)pIUnkObject));
	}

	//IConvertType
	else if(riid == IID_IConvertType)
	{
		//Delegate
		QTESTC(DefTestInterface((IConvertType*)pIUnkObject));
	}
	
	//IColumnsInfo
	else if(riid == IID_IColumnsInfo)
	{
		//Delegate
		QTESTC(DefTestInterface((IColumnsInfo*)pIUnkObject));
	}

	//IColumnsRowset
	else if(riid == IID_IColumnsRowset)
	{
		//Delegate
		QTESTC(DefTestInterface((IColumnsRowset*)pIUnkObject));
	}

	//IAccessor
	else if(riid == IID_IAccessor)
	{
		//Delegate
		QTESTC(DefTestInterface((IAccessor*)pIUnkObject));
	}

	//IDBSchemaRowset
	else if(riid == IID_IDBSchemaRowset)
	{
		//Delegate
		QTESTC(DefTestInterface((IDBSchemaRowset*)pIUnkObject));
	}

	//IDBCreateCommand
	else if(riid == IID_IDBCreateCommand)
	{
		//Delegate
		QTESTC(DefTestInterface((IDBCreateCommand*)pIUnkObject));
	}

	//IRow
	else if(riid == IID_IRow)
	{
		//Delegate
		QTESTC(DefTestInterface((IRow*)pIUnkObject));
	}

	//ISequentialStream
	else if(riid == IID_ISequentialStream)
	{
		//Delegate
		QTESTC(DefTestInterface((ISequentialStream*)pIUnkObject));
	}

	//ISupportErrorInfo
	else if(riid == IID_ISupportErrorInfo)
	{
		ISupportErrorInfo* pISupportErrorInfo = (ISupportErrorInfo*)pIUnkObject;
		//Loop through all the interfaces...
		for(i=0; i<cInterfaceMaps; i++)
		{
			IID iid = *rgInterfaceMaps[i].pIID;

			//ISupportErrorInfo::InterfaceSupportsErrorInfo
			TEST2C_(hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(iid), S_OK, S_FALSE);

			//Verify results...
			if(VerifyInterface(pISupportErrorInfo, iid, eInterface))
			{
				if(hr != S_OK)
					TWARNING("Object supports interface, but not for ErrorInfo? " << GetInterfaceName(iid));
			}
			else
			{
				if(hr == S_OK)
					TWARNING("Object doesn't support interface, but does for ErrorInfo? " << GetInterfaceName(iid));
			}
		}
		
		//Try an invalid IID
		TESTC_(pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_NULL),S_FALSE);
	}

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	TRETURN
}

//---------------------------------------------------------------------
//
BOOL DefTestInterface(IDBInitialize* pIDBInitialize)
{
	TBEGIN
	HRESULT				hr;
	IDBCreateSession*	pIDBCS = NULL;
	IOpenRowset*		pIOR = NULL;

	TESTC(pIDBInitialize != NULL)
	TEST2C_(pIDBInitialize->Initialize(), S_OK, DB_E_ALREADYINITIALIZED)
	TEST2C_(pIDBInitialize->Uninitialize(), S_OK, DB_E_OBJECTOPEN)
	TEST2C_(pIDBInitialize->Initialize(), S_OK, DB_E_ALREADYINITIALIZED)

	TESTC(VerifyInterface(pIDBInitialize,IID_IDBCreateSession,
		DATASOURCE_INTERFACE,(IUnknown**)&pIDBCS))

	TEST2C_(hr=pIDBCS->CreateSession(NULL, IID_IOpenRowset, (IUnknown**)
		&pIOR), S_OK, DB_E_OBJECTCREATIONLIMITREACHED)
	if(hr == S_OK)
		TESTC(pIOR != NULL)
	else
	{
		odtLog<<L"WARNING: Trying to create a session resulted in DB_E_OBJECTCREATIONLIMITREACHED.\n";
		CHECKW(hr, S_OK);
	}

CLEANUP:
	SAFE_RELEASE(pIOR);
	SAFE_RELEASE(pIDBCS);
	TRETURN
}

//---------------------------------------------------------------------
//
BOOL DefTestInterface(IDBProperties* pIDBProperties)
{
	TBEGIN
	ULONG				cPropInfoSets = 0;
	ULONG				cPropSets = 0;
	DBPROPINFOSET*		rgPropInfoSets = NULL;
	DBPROPSET*			rgPropSets = NULL;
	OLECHAR*			pDescBuffer = NULL;

	TESTC(pIDBProperties != NULL)
	TESTC_(pIDBProperties->SetProperties(1, NULL), E_INVALIDARG)

	TESTC_(pIDBProperties->GetPropertyInfo(0, NULL, 
		&cPropInfoSets, &rgPropInfoSets, &pDescBuffer), S_OK)

	TESTC(cPropInfoSets>0 && (rgPropInfoSets != NULL) &&
		(rgPropInfoSets[0].rgPropertyInfos != NULL))

	TESTC_(pIDBProperties->GetProperties(0, NULL, 
		&cPropSets, &rgPropSets), S_OK)

	if(cPropSets==0 && rgPropSets==NULL)
		odtLog<<L"INFO: No properties were returned by GetProperties(0, NULL).\n";
	else
		TESTC(cPropSets>0 && (rgPropSets != NULL) &&
			(rgPropSets[0].rgProperties != NULL))

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pDescBuffer);
	TRETURN
}

//---------------------------------------------------------------------
//
BOOL DefTestInterface(IDBCreateSession* pIDBCS)
{
	TBEGIN
	HRESULT				hr = E_FAIL;
	IDBProperties*		pIDBP = NULL;
	IOpenRowset*		pIOR = NULL;

	TESTC(pIDBCS != NULL)
	TEST2C_(pIDBCS->CreateSession(NULL, IID_IDBProperties, (IUnknown**)
		&pIDBP), E_NOINTERFACE, DB_E_OBJECTCREATIONLIMITREACHED)
	TESTC(!pIDBP)
	TEST2C_(pIDBCS->CreateSession(NULL, IID_IOpenRowset,NULL), 
		E_INVALIDARG, DB_E_OBJECTCREATIONLIMITREACHED)
	TEST2C_(hr = pIDBCS->CreateSession(NULL, IID_IOpenRowset, (IUnknown**)
		&pIOR), S_OK, DB_E_OBJECTCREATIONLIMITREACHED)
	if(hr == S_OK)
		TESTC(pIOR != NULL)

CLEANUP:
	SAFE_RELEASE(pIOR);
	TRETURN
}

//---------------------------------------------------------------------
//
BOOL DefTestInterface(IPersist* pIP)
{
	TBEGIN
	CLSID	clsid = DB_NULLGUID;
	CLSID	clsidNULL = DB_NULLGUID;

	TESTC(pIP != NULL)
	TESTC_(pIP->GetClassID(&clsid), S_OK)
	TESTC(clsid != clsidNULL)

CLEANUP:
	TRETURN
}

//--------------------------------------------------------------------
//
BOOL DefTestInterface(IOpenRowset* pIOR)
{
	TBEGIN
	DBID			dbidFake;
	IRowsetInfo*	pIRowsetInfo = NULL;

	TESTC(pIOR != NULL)
	dbidFake.eKind = DBKIND_NAME;
	dbidFake.uName.pwszName = L"privlibfaketable";

	TESTC_(pIOR->OpenRowset(NULL, &dbidFake, NULL, IID_IRowsetInfo,
		0, NULL, (IUnknown**)&pIRowsetInfo), DB_E_NOTABLE)
	TESTC(!pIRowsetInfo)

	TESTC_(pIOR->OpenRowset(NULL, NULL, NULL, IID_IRowsetInfo,
		0, NULL, (IUnknown**)&pIRowsetInfo), E_INVALIDARG)
	TESTC(!pIRowsetInfo)

CLEANUP:
	SAFE_RELEASE(pIRowsetInfo);
	TRETURN
}

//--------------------------------------------------------------------
//
BOOL DefTestInterface(IGetDataSource* pIGDS)
{
	TBEGIN
	IDBInitialize*	pIDBI = NULL;

	TESTC(pIGDS != NULL)
	TESTC_(pIGDS->GetDataSource(IID_IIndexDefinition, (IUnknown**)
		&pIDBI), E_NOINTERFACE)
	TESTC(!pIDBI)

	TESTC_(pIGDS->GetDataSource(IID_IDBInitialize, NULL), E_INVALIDARG)

	TESTC_(pIGDS->GetDataSource(IID_IDBInitialize, (IUnknown**)
		&pIDBI), S_OK)
	TESTC(pIDBI != NULL)

CLEANUP:
	SAFE_RELEASE(pIDBI);
	TRETURN
}

//--------------------------------------------------------------------
//
BOOL DefTestInterface(IColumnsRowset* pICR)
{
	TBEGIN
	DBORDINAL			cOptColumns = 0;
	DBID *				rgOptColumns = NULL;
	ISessionProperties * pISP = NULL;

	TESTC(pICR != NULL)
	TESTC_(pICR->GetAvailableColumns(&cOptColumns,NULL), E_INVALIDARG)
	TESTC(!cOptColumns)

	TESTC_(pICR->GetAvailableColumns(&cOptColumns,&rgOptColumns), S_OK)
	TESTW(cOptColumns > 0);
	
	if (cOptColumns)
		TESTC(rgOptColumns != NULL)

	TESTC_(pICR->GetColumnsRowset(NULL, 0, NULL, IID_ISessionProperties, 0, NULL,
		(IUnknown **)&pISP), E_NOINTERFACE)
	TESTC(!pISP)

CLEANUP:
	PROVIDER_FREE(rgOptColumns);
	SAFE_RELEASE(pISP);
	TRETURN
}

//--------------------------------------------------------------------
//
BOOL DefTestInterface(ISessionProperties* pISP)
{
	TBEGIN
	ULONG				cPropSets = 0;
	DBPROPSET*			rgPropSets = NULL;

	TESTC(pISP != NULL)
	TESTC_(pISP->GetProperties(1, NULL, &cPropSets, &rgPropSets),
		E_INVALIDARG)
	TESTC(!cPropSets && !rgPropSets)

	TESTC_(pISP->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK)
	if(!cPropSets && !rgPropSets)
		PRVTRACE(L"No Session properties are supported");
	else
		TESTC(cPropSets>0 && rgPropSets!=NULL)

	TESTC_(pISP->SetProperties(1, NULL), E_INVALIDARG)

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
}


//--------------------------------------------------------------------
//
BOOL DefTestInterface(IDBCreateCommand* pIDBCC)
{
	TBEGIN
	ICommandText*		pICT = NULL;

	TESTC(pIDBCC != NULL)
	TESTC_(pIDBCC->CreateCommand(NULL, IID_ISessionProperties, 
		(IUnknown**)&pICT), E_NOINTERFACE)
	TESTC(!pICT)

	TESTC_(pIDBCC->CreateCommand(NULL, IID_ICommandText, (IUnknown**)
		&pICT), S_OK)
	TESTC(pICT != NULL)

CLEANUP:
	SAFE_RELEASE(pICT);
	TRETURN
}

//--------------------------------------------------------------------
//
BOOL DefTestInterface(IDBSchemaRowset* pIDBSR)
{
	TBEGIN
	ULONG				cSchemas = 0;
	GUID *				rgSchemas = NULL;
	ULONG *				rgRestrictionSupport = NULL;
	ISessionProperties*	pISP = NULL;

	TESTC(pIDBSR != NULL)
	TESTC_(pIDBSR->GetSchemas(&cSchemas,&rgSchemas, NULL), E_INVALIDARG)
	TESTC(!cSchemas && !rgSchemas)

	TESTC_(pIDBSR->GetSchemas(&cSchemas,&rgSchemas, 
		&rgRestrictionSupport), S_OK)
	TESTC((cSchemas > 2) && (rgSchemas) && (rgRestrictionSupport))

	TESTC_(pIDBSR->GetRowset(NULL, DBSCHEMA_TABLES, 0, NULL, 
		IID_ISessionProperties, 0, NULL, (IUnknown**)&pISP), 
		E_NOINTERFACE)
	TESTC(!pISP)

CLEANUP:
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictionSupport);
	SAFE_RELEASE(pISP);
	TRETURN
}


//--------------------------------------------------------------------
//
BOOL DefTestInterface(IAccessor* pIAccessor)
{
	TBEGIN
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;
	DBACCESSORFLAGS	dwAccFlags = 0;

	TESTC(pIAccessor != NULL)

	TESTC_(pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, NULL,
		200, NULL, NULL), E_INVALIDARG)
	TESTC_(pIAccessor->GetBindings(DB_NULL_HACCESSOR, &dwAccFlags, 
		&cBindings, &rgBindings), DB_E_BADACCESSORHANDLE)
	TESTC(!cBindings && !rgBindings)
	TEST2C_(pIAccessor->GetBindings(hAccessor, &dwAccFlags, NULL, NULL),
		E_INVALIDARG, DB_E_BADACCESSORHANDLE)

CLEANUP:
	if(pIAccessor && hAccessor)
		pIAccessor->ReleaseAccessor(hAccessor, NULL);
	FreeAccessorBindings(cBindings, rgBindings);
	TRETURN
}


//--------------------------------------------------------------------
//
BOOL DefTestInterface(IColumnsInfo* pICI)
{
	TBEGIN
	DBORDINAL			cColumns = 0;
	DBCOLUMNINFO*		rgColumnInfo = NULL;
	OLECHAR*			pStringsBuffer = NULL;
	DBORDINAL		    rgColumns[1];

	TESTC(pICI != NULL)
	TESTC_(pICI->GetColumnInfo(&cColumns, NULL, NULL), E_INVALIDARG)
	TESTC_(pICI->MapColumnIDs(1, NULL, NULL), E_INVALIDARG)

	TESTC_(pICI->GetColumnInfo(&cColumns, &rgColumnInfo,
		&pStringsBuffer), S_OK)

	TESTC(cColumns>0 && pStringsBuffer && rgColumnInfo)
	TESTC(rgColumnInfo[0].iOrdinal < 2)

	TESTC_(pICI->MapColumnIDs(1, &rgColumnInfo[0].columnid, rgColumns), S_OK)
	TESTC(rgColumns[0] == rgColumnInfo[0].iOrdinal)

	TESTC_(pICI->MapColumnIDs(1, &rgColumnInfo[0].columnid, rgColumns), S_OK)
	TESTC(rgColumns[0] == rgColumnInfo[0].iOrdinal)

CLEANUP:
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringsBuffer);
	TRETURN
}


//--------------------------------------------------------------------
//
BOOL DefTestInterface(IConvertType* pICT)
{
	TBEGIN

	TESTC(pICT != NULL)

	TEST2C_(pICT->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
		DBCONVERTFLAGS_COLUMN), S_OK, DB_E_BADCONVERTFLAG)

	TEST2C_(pICT->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
		DBCONVERTFLAGS_PARAMETER), S_OK, DB_E_BADCONVERTFLAG)

CLEANUP:
	TRETURN
}

//--------------------------------------------------------------------
//
BOOL DefTestInterface(IRowset* pIR, BOOL fClean/*=TRUE*/)
{
	TBEGIN
	HRESULT	hr;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;

	TESTC(pIR != NULL)

	hr = pIR->RestartPosition(NULL);

	if (fClean)
	{
		TEST2C_(hr, S_OK, DB_S_COMMANDREEXECUTED);
	}
	else
	{
		// if fClean is not TRUE, there is no guarantee that the restart
		// position succeeds (a row handle could be still open, the row 
		// could be retrieved from a row, etc)
		TEST3C_(hr, S_OK, DB_S_COMMANDREEXECUTED, DB_E_ROWSNOTRELEASED);
	}

	if (!SUCCEEDED(hr))
		goto CLEANUP;

	TESTC_(hr = pIR->GetNextRows(NULL, 0, 1, NULL, NULL), E_INVALIDARG)

	TESTC_(hr = pIR->ReleaseRows(1, NULL, NULL,NULL,NULL), E_INVALIDARG)

	TEST2C_(hr = pIR->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows), 
		S_OK, DB_S_ENDOFROWSET)

	if(DB_S_ENDOFROWSET == hr)
	{
		TESTC(!cRowsObtained)
		COMPAREW(rghRows, NULL);
	}
	else
		TESTC(cRowsObtained==1 && rghRows!=NULL && rghRows[0]!=NULL)

CLEANUP:
	if(pIR && cRowsObtained && rghRows)
		CHECK(pIR->ReleaseRows(cRowsObtained, rghRows, NULL,NULL,NULL), S_OK);
	if(pIR)
		hr = pIR->RestartPosition(NULL);
	SAFE_FREE(rghRows);
	TRETURN
}

//--------------------------------------------------------------------
//
BOOL DefTestInterface(IRowsetInfo* pIRI)
{
	TBEGIN
	HRESULT			hr;
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	IRowset*		pIRowset = NULL;
	IUnknown		*pIUnknown = NULL;

	TESTC(pIRI != NULL)

	TEST3C_(hr=pIRI->GetReferencedRowset(0, IID_IRowset, NULL), E_INVALIDARG,
		DB_E_BADORDINAL, DB_E_NOTAREFERENCECOLUMN)

	TEST3C_(hr = pIRI->GetReferencedRowset(0, IID_IRowset, (IUnknown**)
		&pIRowset), S_OK, DB_E_BADORDINAL, DB_E_NOTAREFERENCECOLUMN)

	if(DB_E_NOTAREFERENCECOLUMN == hr)
		CHECKW(DB_E_NOTAREFERENCECOLUMN, DB_E_BADORDINAL);

	if(FAILED(hr))
		TESTC(!pIRowset)
	else
		TESTC(pIRowset != NULL)

	TESTC_(pIRI->GetSpecification(IID_IOpenRowset, NULL), E_INVALIDARG)
	TEST2C_(hr = pIRI->GetSpecification(IID_IUnknown, (IUnknown**)
		&pIUnknown), S_OK, S_FALSE)
	if(S_FALSE == hr)
		TESTC(!pIUnknown)
	else
		TESTC(pIUnknown != NULL)

	TESTC_(pIRI->GetProperties(1, NULL, NULL, NULL), E_INVALIDARG)
	TESTC_(pIRI->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets > 0 && rgPropSets && rgPropSets[0].cProperties>0
		&& rgPropSets[0].rgProperties)

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIUnknown);
	TRETURN
}

//--------------------------------------------------------------------
//
BOOL DefTestInterface(IRow* pIRow)
{
	TBEGIN
	HRESULT				hr;
	ULONG				cBytes = 399;
	ULONG				cBytesRead = 0;
	void*				pBuffer = NULL;
	DBID				dbid = DBROWCOL_DEFAULTSTREAM;
	IRowset*			pIRowset = NULL;
	ISequentialStream*	pISS = NULL;

	TESTC(pIRow != NULL)

	//No-op
	TESTC_(hr=pIRow->GetColumns(0, NULL), S_OK)

	TESTC_(hr=pIRow->GetColumns(1, NULL), E_INVALIDARG)

	TESTC_(hr=pIRow->Open(NULL, NULL, DBGUID_STREAM, 0, IID_ISequentialStream, (IUnknown**)pISS), E_INVALIDARG)

	TEST2C_(hr=pIRow->Open(NULL, &dbid, DBGUID_STREAM, 0, IID_ISequentialStream, (IUnknown**)&pISS), S_OK, DB_E_BADCOLUMNID)
	if(S_OK == hr)
	{
		SAFE_ALLOC(pBuffer, BYTE, cBytes);
		TEST2C_(hr = StorageRead(IID_ISequentialStream, pISS, pBuffer, cBytes, &cBytesRead), S_OK, S_FALSE)
		TESTC(cBytesRead <= cBytes)
	}

	TEST2C_(hr=pIRow->GetSourceRowset(IID_IRowset, (IUnknown**)&pIRowset, NULL), S_OK, DB_E_NOSOURCEOBJECT)
	if(S_OK == hr)
		TESTC(pIRowset != NULL)
	else
		TESTC(!pIRowset)

CLEANUP:
	SAFE_FREE(pBuffer);
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pIRowset);
	TRETURN
}

//--------------------------------------------------------------------
//
BOOL DefTestInterface(ISequentialStream* pISS)
{
	TBEGIN
	HRESULT			hr;
	ULONG			cBytes = 399;
	ULONG			cBytesRead = 0;
	void*			pBuffer = NULL;

	TESTC(pISS != NULL)

	SAFE_ALLOC(pBuffer, BYTE, cBytes);

	TEST2C_(hr = StorageRead(IID_ISequentialStream, pISS, pBuffer, cBytes, &cBytesRead), S_OK, S_FALSE)
	TESTC(cBytesRead <= cBytes)

	TESTC_(pISS->Write(NULL, 50, NULL), STG_E_INVALIDPOINTER)

CLEANUP:
	SAFE_FREE(pBuffer);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  VerifyRefCounts
//
////////////////////////////////////////////////////////////////////////////
BOOL	VerifyRefCounts(DBREFCOUNT ulActRefCount, DBREFCOUNT ulExpRefCount)
{
	//Delegate
	return VerifyRefCounts(1, &ulActRefCount, ulExpRefCount);
}


////////////////////////////////////////////////////////////////////////////
//  VerifyRefCounts
//
////////////////////////////////////////////////////////////////////////////
BOOL	VerifyRefCounts
(	
	DBCOUNTITEM			cRefCounts,		//[in] cRefCounts
	DBREFCOUNT*			rgActRefCounts,	//[in] rgRefCount
	DBREFCOUNT			ulExpRefCount	//[in] Expected RefCount 
)	
{
	BOOL bReturn = FALSE;
	//Verify RefCounts
	//There is some problem with RefCounts.  
	//You cannot be guareenteed the refcount.  According to the spec
	//you can only be guarneenteed the count is 1+ when active.
	//Add it doesn't have to go to zero if the provider can still use the row..

	//So the only thing we can verify is:
	//If we expect 1+ count that it is 1+, and if we expect 0,
	//it should be either zero or more
	for(ULONG i=0; i<cRefCounts; i++)
	{
		//If we expect 1+ refcount, make sure it is 1+
		if(ulExpRefCount)
		{
			TESTC(rgActRefCounts[i] >= 1);
			if(rgActRefCounts[i] != ulExpRefCount)
				odtLog << "NOTE: RefCounts are not exact values!\n";
		}
		
		//If we expect 0 they still maybe active
		if(ulExpRefCount == 0)
		{
			if(rgActRefCounts[i] != 0)
				odtLog << "NOTE: RefCounts are still active after release!\n";
		}
	}

	bReturn = TRUE;

CLEANUP:
	return bReturn;
}


//--------------------------------------------------------------------
// @func void | 
//	Miscfunc DuplicatePropertySets |
//	The function duplicates the memory allocated for an array of property sets
//
// Returns BOOL.
//
//--------------------------------------------------------------------
BOOL DuplicatePropertySets
(
	ULONG		cPropSets,		// [IN]the number of elements in the array
	DBPROPSET*	rgPropSets,		// [IN] pointer to the array 
	ULONG*		pcPropSets,		// [IN/OUT] pointer to the output size
	DBPROPSET**	prgPropSets		// [IN/OUT] pointer to the output array 
)
{
	ASSERT(pcPropSets && prgPropSets);
	*pcPropSets = 0;
	*prgPropSets = NULL;
	BOOL bReturn = FALSE;
	ULONG iPropSet = 0;

	if (prgPropSets)
		*prgPropSets	= NULL;
	if (pcPropSets)
		*pcPropSets		= cPropSets;

	//No-op
	if(0 == cPropSets || NULL == rgPropSets)
		return TRUE;

	//Duplicate outer
	SAFE_ALLOC(*prgPropSets, DBPROPSET, cPropSets);
	for(iPropSet=0; iPropSet<cPropSets; iPropSet++)
	{
		DBPROPSET* pPropSetSrc	= &rgPropSets[iPropSet];
		DBPROPSET* pPropSetDst	= &(*prgPropSets)[iPropSet];

		pPropSetDst->cProperties		= pPropSetSrc->cProperties;
		pPropSetDst->guidPropertySet	= pPropSetSrc->guidPropertySet;

		//Optimization
		if (	0		== pPropSetSrc->cProperties
			||	NULL	== pPropSetSrc->rgProperties)
		{
			pPropSetDst->rgProperties	= NULL;
			continue;
		}
		
		//Allocate inner
		SAFE_ALLOC(pPropSetDst->rgProperties, DBPROP, pPropSetSrc->cProperties);
		for(ULONG iProp=0; iProp<pPropSetSrc->cProperties; iProp++)
		{
			DBPROP* pPropSrc = &pPropSetSrc->rgProperties[iProp];
			DBPROP* pPropDst = &pPropSetDst->rgProperties[iProp];
			
			pPropDst->dwPropertyID	= pPropSrc->dwPropertyID;
			pPropDst->dwOptions		= pPropSrc->dwOptions;
			pPropDst->dwStatus		= pPropSrc->dwStatus;
			VariantInit(&pPropDst->vValue);
			VariantCopy(&pPropDst->vValue, &pPropSrc->vValue);
			DuplicateDBID(pPropSrc->colid, &pPropDst->colid);
		}
	}
	
	bReturn = TRUE;
	
CLEANUP:
	return bReturn;
}


//--------------------------------------------------------------------
// @func void | 
//	Miscfunc ReleaseColumnDesc |
//	The function releases the memory allocated for an array of column descriptors
//
// Returns void.
//
//--------------------------------------------------------------------
void ReleaseColumnDesc
(
	DBCOLUMNDESC*	rgColumnDesc,	// pointer to the array 
	DBORDINAL		cColumnDesc,		// the number of elements in the array
	BOOL			fFreeArray		// whether rgColumnDesc should be freed
)
{
	// release the column descriptor array
	if( rgColumnDesc )
	{
		for (DBORDINAL i=0; i<cColumnDesc; i++)
		{
			if (rgColumnDesc[i].pwszTypeName)
				PROVIDER_FREE(rgColumnDesc[i].pwszTypeName);
			ReleaseDBID(&rgColumnDesc[i].dbcid, FALSE);
			FreeProperties(&rgColumnDesc[i].cPropertySets, &rgColumnDesc[i].rgPropertySets);
		}

		if( fFreeArray )
		{
			PROVIDER_FREE(rgColumnDesc);
		}
	}
}


//--------------------------------------------------------------------
// @func void | 
//	Miscfunc DuplicateColumnDesc |
//	The function duplicates the memory allocated for an array of column descriptors
//
// Returns DBCOLUMNDESC*.
//
//--------------------------------------------------------------------
DBCOLUMNDESC *DuplicateColumnDesc
(
	DBCOLUMNDESC*	rgColumnDesc,	// [IN] pointer to the array 
	DBORDINAL		cColumnDesc,		// [IN]the number of elements in the array
	DBCOLUMNDESC**	prgColumnDesc	// [IN/OUT] pointer to the output array 
)
{
	DBCOLUMNDESC*	rgColumnDesc1 = NULL;
	
	if (prgColumnDesc)
		*prgColumnDesc = NULL;

	if (!rgColumnDesc || cColumnDesc ==0)
		return NULL;
	rgColumnDesc1 = (DBCOLUMNDESC *)PROVIDER_ALLOC(sizeof(DBCOLUMNDESC) * cColumnDesc);
	if (!rgColumnDesc1)
		return NULL;
	memset(rgColumnDesc1, 0, (size_t)(sizeof(DBCOLUMNDESC) * cColumnDesc));

	if (rgColumnDesc1)
		for (DBORDINAL i=0; i<cColumnDesc; i++)
		{
			// just fo the time being
			//memcpy(&rgColumnDesc1[i].dbcid, &rgColumnDesc[i].dbcid, sizeof(DBID));
			//if  (rgColumnDesc1[i].dbcid.eKind == DBKIND_NAME && rgColumnDesc[i].dbcid.uName.pwszName)
			//	rgColumnDesc1[i].dbcid.uName.pwszName = wcsDuplicate(rgColumnDesc[i].dbcid.uName.pwszName);
			DuplicateDBID(rgColumnDesc[i].dbcid, &rgColumnDesc1[i].dbcid);
			if (rgColumnDesc[i].pwszTypeName)
				rgColumnDesc1[i].pwszTypeName	= wcsDuplicate(rgColumnDesc[i].pwszTypeName);
			else
				rgColumnDesc1[i].pwszTypeName = NULL;
			rgColumnDesc1[i].wType			= rgColumnDesc[i].wType;
			rgColumnDesc1[i].bPrecision		= rgColumnDesc[i].bPrecision;
			rgColumnDesc1[i].bScale			= rgColumnDesc[i].bScale;
			rgColumnDesc1[i].pTypeInfo		= rgColumnDesc[i].pTypeInfo;
			rgColumnDesc1[i].pclsid			= rgColumnDesc[i].pclsid;
			rgColumnDesc1[i].ulColumnSize	= rgColumnDesc[i].ulColumnSize;
			rgColumnDesc1[i].cPropertySets	= rgColumnDesc[i].cPropertySets;
			rgColumnDesc1[i].rgPropertySets	= NULL;
			DuplicatePropertySets(	rgColumnDesc[i].cPropertySets, rgColumnDesc[i].rgPropertySets,
									&rgColumnDesc1[i].cPropertySets, &rgColumnDesc1[i].rgPropertySets);
		}

	return *prgColumnDesc = rgColumnDesc1;
}


//--------------------------------------------------------------------
// ConvertToWSTR
//
// @func	ConvertToWSTR
// Converts the contents of pStringData to Unicode
// Consumer is responsible for freeing memory returned in pwszOut
//		
//--------------------------------------------------------------------
void ConvertToWSTR(
	void *pStringData,			// @parm || [IN] pointer to string data (can be str, bstr, wstr)
	DBTYPE wSrcType,				// @parm || [IN] type of string data
	WCHAR **pwszOut				// @parm || [OUT] converted to Unicode (provider is responsible for freeing)
)
{
	ULONG cchDst;

	switch ( wSrcType )
	{
	case DBTYPE_STR:

		cchDst = MultiByteToWideChar(CP_ACP,
									 MB_PRECOMPOSED,
									 (char *)pStringData,
									 -1,
									 NULL,
									 0);
		*pwszOut = (WCHAR *)PROVIDER_ALLOC(cchDst*sizeof(WCHAR));
		
		cchDst = MultiByteToWideChar(CP_ACP,
										 MB_PRECOMPOSED,
										 (char *)pStringData,
										 -1,
										 *pwszOut,
										 cchDst);
		break;

	case DBTYPE_BSTR:
		*pwszOut = wcsDuplicate(*(BSTR *)pStringData);
		break;
	case DBTYPE_WSTR:
		*pwszOut = wcsDuplicate((WCHAR *)pStringData);
		break;
	}

}


//--------------------------------------------------------------------
// DecimalDiv10Rem
//
// @func	BOOL DecimalDiv10Rem
// Divides the DECIMAL value indicated by pDecimal by ten
// and returns the result in pDecResult.
// Any remainder is returned in pbRemainder
//		
//--------------------------------------------------------------------
BOOL DecimalDiv10Rem(
	DECIMAL * pDecimal,
	DECIMAL * pDecResult,
	BYTE * pbRemainder
)
{
	DWORDLONG	dwlValue = 0;
	ULONG		ulAccum = 0;

	// Check parameters
	if( !pDecimal || !pDecResult || !pbRemainder )
		return FALSE;

	dwlValue += pDecimal->Hi32;
	pDecimal->Hi32 = (ULONG)(dwlValue / 10);
		
	dwlValue %= 10;
	dwlValue <<= sizeof(ULONG) * 8;

	dwlValue += pDecimal->Mid32;
	pDecimal->Mid32 = (ULONG)(dwlValue / 10);

	dwlValue %= 10;
	dwlValue <<= sizeof(ULONG) * 8;

	dwlValue += pDecimal->Lo32;
	pDecimal->Lo32 = ULONG (dwlValue / 10);

	dwlValue %= 10;
	
	ulAccum = (ULONG)dwlValue;
	
	// Save the remainder and adjust scale
    *pbRemainder = (BYTE)ulAccum;

	 (pDecResult->scale)--;

	// And we're done
    return TRUE;
}


//--------------------------------------------------------------------
// ScaleDecimal
//
// @func	BOOL ScaleDecimal
// Scales a Decimal value down to scale indicated by bScale
// Returns TRUE if no truncation occurred.
// Returns FALSE if fractional truncation occurred.
//		
//--------------------------------------------------------------------
BOOL ScaleDecimal(DECIMAL * pDec, BYTE bScale)
{
	BYTE		bScaleT;
	BYTE		bRemainder;
	BOOL		fPass, fPassTemp;

	if( !pDec )
		return FALSE;

	bScaleT = pDec->scale;

	if( bScaleT == bScale )
		return TRUE;

	if( pDec->Hi32 == 0 && pDec->Lo64 == 0 )
	{
		pDec->sign		= 0;
		pDec->scale		= bScale;

		return TRUE;
	}

	// Assume success
	fPass = TRUE;

	if( bScaleT < bScale )
	{
		ASSERT(!"This function can only scale down");  // should only call for scaling down
		return FALSE;
	}
	else if( bScaleT > bScale )
	{
		// Scale down...

		// Do the scaling
		while( bScaleT-- > bScale )
		{
			fPassTemp = DecimalDiv10Rem(pDec, pDec, &bRemainder);
			if( !fPassTemp )
				return fPassTemp;

			// If we're not just dropping a zero, that's truncation
			if( bRemainder )
				fPass = FALSE;
		}
	}

	pDec->scale		= bScale;

	return fPass;
}


//--------------------------------------------------------------------
// CompareDecimal
//
// @func	BOOL CompareDecimal
// Compares two DECIMAL values.
// This function performs the necessary scaling and returns TRUE if
// the two DECIMALs are numerically equivalent.
//		
//--------------------------------------------------------------------
BOOL CompareDecimal(
	DECIMAL *pDec1, 
	DECIMAL *pDec2
)
{
	BOOL		fNoTruncation = TRUE;
	DECIMAL		decVal;   // buffer the scaled decimal because ScaleDecimal is destructive

	if ( pDec1->sign != pDec2->sign )
		return FALSE;

	if ( pDec1->scale != pDec2->scale )
	{
		if ( pDec1->scale > pDec2->scale )
		{
			memcpy(&decVal, pDec1, sizeof(DECIMAL));
			pDec1 = &decVal;
			fNoTruncation = ScaleDecimal(pDec1, pDec2->scale);
		}
		else
		{
			memcpy(&decVal, pDec2, sizeof(DECIMAL));
			pDec2 = &decVal;
			fNoTruncation = ScaleDecimal(pDec2, pDec1->scale);
		}
	}

	if ( !fNoTruncation ) 
		return FALSE;

	return ( pDec1->Hi32 == pDec2->Hi32 && pDec1->Lo64 == pDec2->Lo64 );
}


//--------------------------------------------------------------------
// NumericDiv10Rem
//
// @func	BOOL NumericDiv10Rem
// Divides the DBTYPE_NUMERIC value indicated by pNum by ten
// and returns the result in pNumResult.
// Any remainder is returned in pbRemainder
//		
//--------------------------------------------------------------------
BOOL NumericDiv10Rem(
	DB_NUMERIC * pNumeric,
	DB_NUMERIC * pNumResult,
	BYTE * pbRemainder
)
{
	DWORDLONG	dwlValue = 0;
	ULONG		ulAccum = 0;
	LONG		l;

	// Check parameters
	if( !pNumeric || !pNumResult || !pbRemainder )
		return FALSE;

	for( l = sizeof(pNumeric->val) / sizeof(ULONG) - 1; l >= 0; l-- )
	{
		dwlValue <<= (sizeof(ULONG) * 8);
		dwlValue += *(((UNALIGNED ULONG *)pNumeric->val) + l);
		*(((UNALIGNED ULONG *)pNumResult->val) + l) = (ULONG)(dwlValue / 10);
		dwlValue %= 10;
	}
	ulAccum = (ULONG)dwlValue;
		
	// Save the remainder and adjust scale
    *pbRemainder = (BYTE)ulAccum;

	 (pNumResult->scale)--;

	// And we're done
    return TRUE;
}


//--------------------------------------------------------------------
// ScaleNumeric
//
// @func	BOOL ScaleNumeric
// Scales a Numeric value down to scale indicated by bScale
// Returns TRUE if no truncation occurred.
// Returns FALSE if fractional truncation occurred.
//		
//--------------------------------------------------------------------
BOOL ScaleNumeric(DB_NUMERIC *pNum, BYTE bScale)
{
	BYTE		bScaleT;
	BYTE		bRemainder;
	BOOL		fPass = TRUE, fPassTemp = FALSE;
	BYTE		rgBytes[16];

	if( !pNum )
		return FALSE;

	memset(rgBytes, 0, sizeof(rgBytes));

	bScaleT = pNum->scale;

	if( bScaleT == bScale )
		return TRUE;

	if( memcmp(pNum->val, rgBytes, sizeof(rgBytes)) == 0 )
	{
		pNum->sign		= 0;
		pNum->scale		= bScale;

		return TRUE;
	}

	// Assume success
	fPass = TRUE;

	if( bScaleT < bScale )
	{
		ASSERT(!"This function can only scale down");  // should only call for scaling down
		return FALSE;
	}
	else if( bScaleT > bScale )
	{
		// Scale down...

		// Do the scaling
		while( bScaleT-- > bScale )
		{
			fPassTemp = NumericDiv10Rem(pNum, pNum, &bRemainder);
			if( !fPassTemp )
				return fPassTemp;

			// If we're not just dropping a zero, that's truncation
			if( bRemainder )
				fPass = FALSE;
		}
	}

	pNum->scale		= bScale;

	return fPass;
}

//--------------------------------------------------------------------
// CompareNumeric
//
// @func	BOOL CompareNumeric
// Compares two DBTYPE_NUMERIC values loosely
// This function performs the necessary scaling and returns TRUE if
// the two NUMERICs are numerically equivalent.
// i.e. scale and precision do not have to be equal
//		
//--------------------------------------------------------------------
BOOL CompareNumeric(
	DB_NUMERIC *pNum1, 
	DB_NUMERIC *pNum2
)
{
	BOOL		fNoTruncation = TRUE;
	DB_NUMERIC	NumVal;
	BYTE		bScaleRequest = 0;


	if (pNum1->precision==0 || pNum2->precision==0)
		return FALSE;

	if ( pNum1->sign != pNum2->sign )
		return FALSE;

	if ( pNum1->scale != pNum2->scale )
	{
		if ( pNum1->scale > pNum2->scale )
		{
			memcpy(&NumVal, pNum1, sizeof(DB_NUMERIC));
			pNum1 = &NumVal;
			fNoTruncation = ScaleNumeric(pNum1, pNum2->scale);
		}
		else
		{
			memcpy(&NumVal, pNum2, sizeof(DB_NUMERIC));
			pNum2 = &NumVal;
			fNoTruncation = ScaleNumeric(pNum2, pNum1->scale);			
		}
	}

	if ( !fNoTruncation ) 
		return FALSE;

	return ( memcmp(pNum1->val, pNum2->val, 16) == 0 );
}

//--------------------------------------------------------------------
// VarNumericDiv10Rem
//
// @func	BOOL VarNumericDiv10Rem
// Divides the DBTYPE_VARNUMERIC value indicated by pNum by ten
// and returns the result in pNumResult.
// Any remainder is returned in pbRemainder
//		
//--------------------------------------------------------------------
BOOL VarNumericDiv10Rem(
	DB_VARNUMERIC * pVarNumeric,
	DB_VARNUMERIC * pVarNumResult,
	ULONG	cbVarNumericValSize,
	BYTE * pbRemainder
)
{
	DWORDLONG	dwlValue = 0;
	ULONG		ulAccum = 0, cbTempValBuf = 0, ul;
	LONG		l;
	BYTE		*pbVal = NULL;
	BOOL		fPass = FALSE;

	// Check parameters
	if( !pVarNumeric || !pVarNumResult || !pbRemainder )
		goto CLEANUP;

	cbTempValBuf = (cbVarNumericValSize % 4) ? cbVarNumericValSize + (4 - cbVarNumericValSize % 4) : cbVarNumericValSize;
	pbVal = (BYTE *)PROVIDER_ALLOC(cbTempValBuf);

	memset(pbVal, 0, cbTempValBuf);
	memcpy(pbVal, pVarNumeric->val, cbVarNumericValSize);

	for( l = cbTempValBuf / sizeof(ULONG) - 1; l >= 0; l-- )
	{
		dwlValue <<= (sizeof(ULONG) * 8);
		dwlValue += *(((UNALIGNED ULONG *)pbVal) + l);
		*(((UNALIGNED ULONG *)pbVal) + l) = (ULONG)(dwlValue / 10);
		dwlValue %= 10;
	}
	ulAccum = (ULONG)dwlValue;
		
	// Save the remainder and adjust scale
    *pbRemainder = (BYTE)ulAccum;

	 (pVarNumResult->scale)--;

	 // check for overflow (should never happen on division, but check anyway)
	for (ul=cbVarNumericValSize;ul<cbTempValBuf;ul++)
		if(pbVal[ul] != 0)
			goto CLEANUP;

	// copy results to pVarNumResult and indicate success
	memcpy(&(pVarNumResult->val[0]), pbVal, cbVarNumericValSize);
	fPass = TRUE;

CLEANUP:

	// And we're done
	PROVIDER_FREE(pbVal);
    return fPass;
}

//--------------------------------------------------------------------
// ScaleVarNumeric
//
// @func	BOOL ScaleVarNumeric
// Scales a Numeric value down to scale indicated by bScale
// Returns TRUE if no truncation occurred.
// Returns FALSE if fractional truncation occurred.
//		
//--------------------------------------------------------------------
BOOL ScaleVarNumeric(DB_VARNUMERIC *pVarNum, USHORT cbVarNumericValSize, SBYTE sbScale)
{
	SBYTE		sbScaleT;
	BYTE		bRemainder;
	BOOL		fPass = TRUE, fPassTemp = FALSE;
	BYTE		*rgBytes = (BYTE *)PROVIDER_ALLOC(cbVarNumericValSize);

	if( !pVarNum || !rgBytes )
		return FALSE;

	memset(rgBytes, 0, cbVarNumericValSize);

	sbScaleT = pVarNum->scale;

	if( sbScaleT == sbScale )
	{
		fPass = TRUE;
		goto CLEANUP;
	}

	if( memcmp(pVarNum->val, rgBytes, cbVarNumericValSize) == 0 )
	{
		// if the value is 0, no scaling required
		pVarNum->sign		= 0;
		pVarNum->scale		= sbScale;

		fPass = TRUE;
		goto CLEANUP;
	}

	if( sbScaleT < sbScale )
	{
		ASSERT(!"This function can only scale down");  // should only call for scaling down
		goto CLEANUP;
	}
	else if( sbScaleT > sbScale )
	{
		// Scale down...

		// Do the scaling
		while( sbScaleT-- > sbScale )
		{
			fPassTemp = VarNumericDiv10Rem(pVarNum, pVarNum, cbVarNumericValSize, &bRemainder);
			if( !fPassTemp )
				return fPassTemp;

			// If we're not just dropping a zero, that's truncation
			if( bRemainder )
				fPass = FALSE;
		}
	}

	pVarNum->scale	= sbScale;

CLEANUP:
	PROVIDER_FREE(rgBytes);
	return fPass;
}

//--------------------------------------------------------------------
// CompareVarNumeric
//
// @func	BOOL CompareVarNumeric
// Compares two DBTYPE_VARNUMERIC values loosely
// This function performs the necessary scaling and returns TRUE if
// the two NUMERICs are numerically equivalent.
// i.e. scale and precision do not have to be equal
//		
//--------------------------------------------------------------------
BOOL CompareVarNumeric(
	DB_VARNUMERIC *pVarNum1, 
	USHORT			cbVarNum1,
	DB_VARNUMERIC *pVarNum2,
	USHORT			cbVarNum2
)
{
	USHORT cbMax = 0;
	BOOL fPass = FALSE;
	DB_VARNUMERIC *pVarNumBig, *pVarNumSmall;
	BOOL fNoTruncation = TRUE;

	if (pVarNum1->precision==0 || pVarNum2->precision==0)
		return FALSE;

	if ( pVarNum1->sign != pVarNum2->sign )
		return FALSE;

	// massage both VARNUMERIC to same size
	if ( cbVarNum1 < cbVarNum2 )
		cbMax = cbVarNum2;
	else
		cbMax = cbVarNum1;

	// always create new buffers because the call to ScaleVarNumeric
	// will change to values of the varnumerics.
	pVarNumSmall = (DB_VARNUMERIC *)PROVIDER_ALLOC(cbMax);
	pVarNumBig = (DB_VARNUMERIC *)PROVIDER_ALLOC(cbMax);

	if (!pVarNumSmall || !pVarNumBig)
		goto CLEANUP;

	memset(pVarNumSmall, 0, cbMax);
	memset(pVarNumBig, 0, cbMax);
	memcpy(pVarNumSmall, pVarNum1, cbVarNum1);
	memcpy(pVarNumBig, pVarNum2, cbVarNum2);
	
	// scale if necessary
	if ( pVarNumSmall->scale != pVarNumBig->scale )
	{
		if ( pVarNumSmall->scale > pVarNumBig->scale )
			fNoTruncation = ScaleVarNumeric(pVarNumSmall, cbMax-sizeof(DB_VARNUMERIC)+1, pVarNumBig->scale);
		else
			fNoTruncation = ScaleVarNumeric(pVarNumBig, cbMax-sizeof(DB_VARNUMERIC)+1, pVarNumSmall->scale);
	}

	if ( !fNoTruncation ) 
		goto CLEANUP;

	fPass = memcmp(pVarNumSmall->val, pVarNumBig->val, cbMax-sizeof(DB_VARNUMERIC)+1) == 0 ;

CLEANUP:
	PROVIDER_FREE(pVarNumSmall);
	PROVIDER_FREE(pVarNumBig);
	return fPass;
}

//--------------------------------------------------------------------
// @func Print the list of properties and the associated error code
//       from the array returned by SetProperties.
//
//  @rdesc None
//--------------------------------------------------------------------
void DumpPropertyErrors
(
    ULONG cPropSets,
	DBPROPSET* rgPropSets,
    ULONG* pcErrors,
    ULONG* pcInvalidProps
)		 
{	
	WCHAR wszBuffer[256];
	ULONG cErrors = 0;
	ULONG cInvalidProps = 0;

	for(ULONG iPropSet=0; iPropSet<cPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];
		for(ULONG iProp=0; iProp<pPropSet->cProperties; iProp++)	
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			
			switch(pProp->dwStatus) 
			{
				case DBPROPSTATUS_OK:
					continue;				
			
				case DBPROPSTATUS_NOTSUPPORTED:
				case DBPROPSTATUS_NOTSETTABLE:
				case DBPROPSTATUS_CONFLICTING:
					cInvalidProps++;
					break;

				default:
					cErrors++;
					break;
			}

			//Display Status...
			swprintf(wszBuffer, L"   PropError - [%2d][%2d] %s   - %s\n",
					iPropSet, iProp,
					GetPropertyName(pProp->dwPropertyID, pPropSet->guidPropertySet),
					GetPropStatusName(pProp->dwStatus));
			odtLog << wszBuffer;
		}
	}

	if(pcErrors)
		*pcErrors = cErrors;
	if(pcInvalidProps)
		*pcInvalidProps = cInvalidProps;
}

//--------------------------------------------------------------------
// CompareWCHARData
//
// @func	BOOL CompareWCHARData
// Compares two DBTYPE_WSTR values given the backend type, precision,
// and scale by converting the WSTR values back to the native type
// then comparing as native.  This was necessary to avoid string format
// differences between MakeData and the provider's WCHAR result.  For
// example: MakeData returns "1992-01-01 10:12:20.0", provider returns
// "1992-01-01 10:12:20.000000000" for DBTYPE_DBTIMESTAMP.  These are 
// the same but the strings differ.  
//		
//--------------------------------------------------------------------
BOOL CompareWCHARData(
	LPVOID pConsumerData,		// @parm [in]: Pointer to WCHAR version of consumer data
	LPVOID pBackEndData,		// @parm [in]: Pointer to WCHAR version of backend data
	DBTYPE wBackEndType,		// @parm [in]: the DBType of the backend data, no modifiers
	DBLENGTH ulBackEndSize,		// @parm [in]: size in bytes of the backend data, as it might contain embedded nulls.
	DBLENGTH ulConsumerSize		// @parm [in]: size in bytes of the consumer data, as it might contain embedded nulls.
)
{
	BOOL fSame = FALSE;

	//bPrecision and bScale are Only valid for DBTYPE_NUMERIC or DBTYPE_DECIMAL
	BYTE bPrecision = 0;			
	BYTE bScale = 0;				

	// Consumer data should always be null terminated
	if (*(WCHAR *)((BYTE *)pConsumerData + ulConsumerSize) != L'\0')
		return FALSE;

	// Note: We don't check the length value when converting back to native type because it's legal in
	// many cases for a provider to return a slightly different string than we expect.  For example, we
	// may create an R8 string as L"1.0", size 6, but the provider may return it as L"1", size 2.  But
	// when converted back to R8 the R8's should match.

	// Don't bother to convert back to native types for the following
	if (wBackEndType != DBTYPE_STR && 
		wBackEndType != DBTYPE_WSTR &&
		wBackEndType != DBTYPE_BYTES &&
		wBackEndType != DBTYPE_VARIANT &&
		wBackEndType != DBTYPE_NULL &&
		wBackEndType != DBTYPE_EMPTY)
	{
		// For non-native, non-char/binary types convert back to the native type for comparison
		UWORD uswConsumerSize, uswBackEndSize;

		// Note we have to provide a size for DBTYPE_BYTES, but we'll over-ride with the
		// "correct" backend size.
		void  * pConsumer=WSTR2DBTYPE((WCHAR *)pConsumerData, wBackEndType, &uswConsumerSize);
		void * pBackEnd=WSTR2DBTYPE((WCHAR *)pBackEndData, wBackEndType, &uswBackEndSize);

		// Make sure the conversion to native type was valid, otherwise we fail compare.
		if (pBackEnd && pConsumer)
		{
			// Now we need to change the bPrecision and bScale for DBTYPE_NUMERIC to reflect the expected
			// values based on our conversion to the native type above or we miscompare erroneously.  This
			// is required due to a change in the way numerics are compared.
			if (wBackEndType == DBTYPE_NUMERIC)
			{
				bPrecision = ((DB_NUMERIC *)pBackEnd)->precision;
				bScale = ((DB_NUMERIC *)pBackEnd)->scale;
			}

			fSame = CompareDBTypeData(pConsumer,
									pBackEnd,
									wBackEndType,
									uswBackEndSize,									
									bPrecision,
									bScale,
									NULL,
									FALSE,
									DBTYPE_EMPTY,
									uswConsumerSize);
		}

		PROVIDER_FREE(pBackEnd);
		PROVIDER_FREE(pConsumer);
	}
	else
	{
		fSame = (ulBackEndSize == ulConsumerSize);
		// Can't use wcscmp because the data might contain embedded nulls
		if (ulBackEndSize == wcslen((WCHAR *)pBackEndData) * sizeof(WCHAR))
			fSame &= (ulBackEndSize == wcslen((WCHAR *)pConsumerData) * sizeof(WCHAR));
		fSame &= !memcmp(pConsumerData, pBackEndData, (size_t)ulBackEndSize);
	}

	return fSame;
}


////////////////////////////////////////////////////////////////
// WCHAR* SkipWhiteSpace
//
////////////////////////////////////////////////////////////////
WCHAR* SkipWhiteSpace(WCHAR* pwszStart, WCHAR* pwszEnd, WCHAR* pwszSpaceTokens)
{
	WCHAR* pwszNext = pwszStart;

	//NOTE: pwszSpaceTokens normally just contains " ".  But can contain a list
	//of characters that you want to treat as whitespace, ie: "\n\r\t "
	//All \n, \r, \t, and the ' ' will be treated as whitespace...

	//Skip whitespace forward (until EOL is reached)
	if(pwszEnd == NULL)
	{									 
		while(pwszNext && pwszNext[0] && wcschr(pwszSpaceTokens, pwszNext[0]))
			pwszNext++;
	}
	//Skip whitespace forward (until end pointer is reached)
	else if(pwszStart < pwszEnd)
	{
		while(pwszNext && pwszNext < pwszEnd && wcschr(pwszSpaceTokens, pwszNext[0]))
			pwszNext++;
	}
	//Skip whitespace backward (until start pointer is reached)
	else if(pwszStart > pwszEnd)
	{
		while(pwszNext && pwszNext > pwszEnd && wcschr(pwszSpaceTokens, pwszNext[0]))
			pwszNext--;
	}

	return pwszNext;
}


////////////////////////////////////////////////////////////////
// WCHAR* FindCharacter
//
////////////////////////////////////////////////////////////////
WCHAR* FindCharacter(WCHAR* pwszStart, WCHAR* pwszEnd, WCHAR wChar)
{
	WCHAR* pwszNext = pwszStart;
	
	//Look forward for character (until EOL is reached)
	if(pwszEnd == NULL)
	{
		while(pwszNext && pwszNext[0] && pwszNext[0] != wChar)
			pwszNext++;
	}
	//Look forward for character (until end pointer is reached)
	else if(pwszStart < pwszEnd)
	{
		while(pwszNext && pwszNext < pwszEnd && pwszNext[0] != wChar)
			pwszNext++;
	}
	//Look backward for character (until start pointer is reached)
	else if(pwszStart > pwszEnd)
	{
		while(pwszNext && pwszNext > pwszEnd && pwszNext[0] != wChar)
			pwszNext--;
	}

	if(pwszNext && pwszNext[0] == wChar)
		return pwszNext;  //return pointer to found character
	return NULL;	//Indicate character was not found...
}


//////////////////////////////////////////////////////////////////
// WCHAR FindSubString
//
//////////////////////////////////////////////////////////////////
WCHAR*	FindSubString(WCHAR* pwszBuffer, WCHAR* pwszKeyword, BOOL fCaseSensitive)
{
    //Need to find the substring
	if(fCaseSensitive)
	{
		//Simple
		return wcsstr(pwszBuffer, pwszKeyword);
	}
	else
	{
		//less simple...
		//Not so sure why we don't have C runtime support for this...
		while(*pwszBuffer)
        {
			WCHAR* p1 = pwszBuffer;
			WCHAR* p2 = pwszKeyword;		
            while(*p1 && *p2 && (towlower(*p1)==towlower(*p2)) )
				p1++, p2++;

            if(!*p2)
				return pwszBuffer;

            pwszBuffer++;
        }
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////
// HRESULT ReplaceString
//
//////////////////////////////////////////////////////////////////
HRESULT	ReplaceString(WCHAR* pwszBuffer, WCHAR* pwszTokens, WCHAR* pwszReplace)
{
	//no-op
	if(pwszBuffer == NULL || pwszTokens == NULL || pwszReplace == NULL)
		return E_FAIL;

	//Make sure we have the same replacement as the search string
	//IE: (this function doesn't handle growing the string, but it will handle shrinking...)
	size_t ulTokenLen = wcslen(pwszTokens);
	size_t ulReplaceLen = wcslen(pwszReplace);
	if(ulTokenLen < ulReplaceLen || ulReplaceLen > wcslen(pwszBuffer))
		return E_FAIL;
		
	//Replace all occurrences of pszTokens with pszReplace
	WCHAR* pwszFound = wcsstr(pwszBuffer, pwszTokens);
	while(pwszFound)
	{
		//Replace the substring with the new tokens
		memcpy(pwszFound, pwszReplace, sizeof(WCHAR)*ulReplaceLen);
		
		//May need to "shrink" the string...
		//memmove handles overlapping regions...
		if(ulReplaceLen < ulTokenLen)
			wcscpy(pwszFound + ulReplaceLen, pwszFound + ulTokenLen);

		//Try to find the next occurrence
		pwszFound += ulReplaceLen;
		pwszFound = wcsstr(pwszFound, pwszTokens);
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////
// StorageRead
//
////////////////////////////////////////////////////////////////////////////
HRESULT StorageRead(REFIID riid, IUnknown* pIUnknown, void* pBuffer, ULONG cBytes, ULONG* pcBytesRead, ULONG ulOffset)
{
	HRESULT hr = S_OK;
	if(pIUnknown == NULL)
		return E_INVALIDARG;

	IID iid = riid;
	IUnknown* pIUnkStorage = NULL;

	//Might have bound the object as IUnknown
	if(iid == IID_IUnknown)
	{	
		//Try to obtain a stream interface...
		if(VerifyInterface(pIUnknown, IID_ISequentialStream, UNKNOWN_INTERFACE, &pIUnkStorage))
			iid = IID_ISequentialStream;
		else if(VerifyInterface(pIUnknown, IID_IStream, UNKNOWN_INTERFACE, &pIUnkStorage))
			iid = IID_IStream;
		else if(VerifyInterface(pIUnknown, IID_ILockBytes, UNKNOWN_INTERFACE, &pIUnkStorage))
			iid = IID_ILockBytes;

		pIUnknown = pIUnkStorage;
	}

	//NOTE: Notice we don't QI off the interface passed if the caller informed us it was 
	//of a particular type.  This way we get extra testing that the interface returned from
	//GetData (and other methods) are usable in the native format without needing a QI,
	//as the provider should have already done this for the user according to the pObject
	//request.  The ONLY time we do QI is if the user asked for IID_IUnknown or is not sure
	//of what type of object this should be and just wants to read the data in whatever format
	//possible...

	//Which method to call to read the data...
	if(iid == IID_ISequentialStream)
	{
		ISequentialStream* pISeqStream = (ISequentialStream*)pIUnknown;
		hr = pISeqStream->Read(pBuffer, cBytes, pcBytesRead);
	}
	else if(iid == IID_ILockBytes)
	{
		//Read the Data into our buffer
		ULARGE_INTEGER ulargeOffset = { ulOffset };
		ILockBytes* pILockBytes = (ILockBytes*)pIUnknown;
		hr = pILockBytes->ReadAt(ulargeOffset, pBuffer, cBytes, pcBytesRead);
	}
	else if(iid == IID_IStream)
	{
		//Seek before reading
		//So we are symetric to ILockBytes for all our testcases...
		IStream* pIStream = (IStream*)pIUnknown;
		LARGE_INTEGER largeOffset = { ulOffset };
		TESTC_(hr = pIStream->Seek(largeOffset, STREAM_SEEK_SET, NULL),S_OK);
		
		//Read the Data into our buffer
		hr = pIStream->Read(pBuffer, cBytes, pcBytesRead);
	}
	else if(iid == IID_IStorage)
	{
		//TODO
		hr = E_FAIL;
	}
	else
	{
		//I have no clue what type of object this is.
		hr = E_FAIL;
	}

CLEANUP:
	SAFE_RELEASE(pIUnkStorage);
	return hr;
}



////////////////////////////////////////////////////////////////////////////
// StorageWrite
//
////////////////////////////////////////////////////////////////////////////
HRESULT StorageWrite(REFIID riid, IUnknown* pIUnknown, void* pBuffer, ULONG cBytes, ULONG* pcBytesWrote, ULONG ulOffset)
{
	IUnknown* pIUnkStorage = NULL;
	HRESULT hr = S_OK;
	if(pIUnknown == NULL)
		return E_INVALIDARG;

	IID iid = riid;

	//Might have bound the object as IUnknown
	if(iid == IID_IUnknown)
	{
		//Try to obtain a stream interface...
		if(VerifyInterface(pIUnknown, IID_ISequentialStream))
			iid = IID_ISequentialStream;
		else if(VerifyInterface(pIUnknown, IID_IStream))
			iid = IID_IStream;
		else if(VerifyInterface(pIUnknown, IID_ILockBytes))
			iid = IID_ILockBytes;
	}

	//Which method to call to read the data...
	if(iid == IID_ISequentialStream)
	{
		//Write data into the buffer...
		QTESTC_(hr = pIUnknown->QueryInterface(IID_ISequentialStream, (void**)&pIUnkStorage),S_OK);
		QTESTC_(hr = ((ISequentialStream*)pIUnkStorage)->Write(pBuffer, cBytes, pcBytesWrote),S_OK);
	}
	else if(iid == IID_ILockBytes)
	{
		//Write data into the buffer...
		ULARGE_INTEGER ulargeOffset = { ulOffset };
		QTESTC_(hr = pIUnknown->QueryInterface(IID_ILockBytes, (void**)&pIUnkStorage),S_OK);
		QTESTC_(hr = ((ILockBytes*)pIUnkStorage)->WriteAt(ulargeOffset, pBuffer, cBytes, pcBytesWrote),S_OK);
	}
	else if(iid == IID_IStream)
	{
		//Seek before reading...
		//So we are symetric to ILockBytes for all our testcases...
		LARGE_INTEGER largeOffset = { ulOffset };
		QTESTC_(hr = pIUnknown->QueryInterface(IID_IStream, (void**)&pIUnkStorage),S_OK);
		TESTC_(hr = ((IStream*)pIUnkStorage)->Seek(largeOffset, STREAM_SEEK_SET, NULL),S_OK);

		//Read the Data into our buffer
		QTESTC_(hr = ((IStream*)pIUnkStorage)->Write(pBuffer, cBytes, pcBytesWrote),S_OK);
	}
	else if(iid == IID_IStorage)
	{
		//TODO
		hr = E_FAIL;
	}
	else
	{
		//I have no clue what type of object this is.
		hr = E_FAIL;
	}

CLEANUP:
	SAFE_RELEASE(pIUnkStorage);
	return hr;
}



////////////////////////////////////////////////////////////////
// BOOL SetDCLibraryVersion(ULONG ulVersion)
//
// Takes a pointer to IDataConvert (from msdadc)
// and sets the version 
////////////////////////////////////////////////////////////////
BOOL SetDCLibraryVersion(IUnknown *pIUnknown, ULONG ulVersion)
{
	DCINFO rgInfo[] = {{DCINFOTYPE_VERSION,{VT_UI4, 0, 0, 0, 0x0}}};
	IDCInfo	*pIDCInfo = NULL;

	if ( S_OK != pIUnknown->QueryInterface(IID_IDCInfo, (void **)&pIDCInfo) )
		return FALSE;
	
	V_UI4(&rgInfo->vData) = ulVersion;

	if ( S_OK != pIDCInfo->SetInfo(NUMELEM(rgInfo),rgInfo) )
		return FALSE;

	SAFE_RELEASE(pIDCInfo);

	return TRUE;
}


//---------------------------------------------------------------------------
//	@Fetches registry key that enables international data
//
// @rdesc Returns registry key val
//---------------------------------------------------------------------------
DWORD FindIntlSetting()
{
	long	lResult;
	char	szValueName[128];
	DWORD	dwValue = 0;
	DWORD	dwType = REG_DWORD, dwLen = sizeof(DWORD);
	HKEY	hkResult = 0;

	// Looks under the International key under HKEY_CURRENT_USER
	strcpy(szValueName, "OLEDB Test\\");
	strcat(szValueName, "International");

	lResult = RegOpenKeyExA(
		HKEY_CURRENT_USER,	// handle of open key 
		szValueName,		// address of name of subkey to open 
		0,					// reserved 
		KEY_READ,			// security access mask 
		&hkResult 			// address of handle of open key 
   );

	//Get the value if the Key exists
	if( (lResult == ERROR_SUCCESS) && (hkResult) )
	{
		lResult = RegQueryValueExA(
			hkResult,			// handle of key to query 
			"Enable",			// address of name of value to query 
			NULL,				// reserved 
			&dwType,			// address of buffer for value type 
			(BYTE *)&dwValue,	// address of data buffer 
			&dwLen 				// address of data buffer size 
		);
	
		lResult = RegCloseKey(hkResult);
		return dwValue;
	}
	else 
	{
		//If the Key doesn't exist, assume we want to run in
		//"international" mode as long as the Locale is non-US
		if( LOCALE_ENGLISH_US == GetUserDefaultLCID() ) 
			return 0;
		else
			return 1;
	}
}


////////////////////////////////////////////////////////////////
// __int64 PrivLib_wtoi64(pwszbigint)
//
// Converts source string to __int64
////////////////////////////////////////////////////////////////
__int64  PrivLib_wtoi64(const WCHAR *pwszbigint)
{
        char astring[80];

        WideCharToMultiByte (CP_ACP, 0, pwszbigint, -1,
                            astring, sizeof(astring), NULL, NULL);

        return (_atoi64(astring));
}


//--------------------------------------------------------------------
// Function takes an enumeration of the query and returns a guid
//
// @mfunc SetSchemaGuid
//
//--------------------------------------------------------------------
BOOL GetSchemaGUID
(	
	EQUERY			eQuery,		// @parm [IN] Schema Query requesting
	GUID *			pGuid		// @parm [OUT] Guid associated with query
)
{
	switch(eQuery)
	{
		case SELECT_DBSCHEMA_ASSERTIONS:
			*pGuid = DBSCHEMA_ASSERTIONS;
			return TRUE;

		case SELECT_DBSCHEMA_CATALOGS:
			*pGuid = DBSCHEMA_CATALOGS;
			return TRUE;

		case SELECT_DBSCHEMA_CHARACTER_SETS:
			*pGuid = DBSCHEMA_CHARACTER_SETS;
			return TRUE;

		case SELECT_DBSCHEMA_CHECK_CONSTRAINTS:
			*pGuid = DBSCHEMA_CHECK_CONSTRAINTS;
			return TRUE;

		case SELECT_DBSCHEMA_COLLATIONS:
			*pGuid = DBSCHEMA_COLLATIONS;
			return TRUE;

		case SELECT_DBSCHEMA_COLUMN_DOMAIN_USAGE:
			*pGuid = DBSCHEMA_COLUMN_DOMAIN_USAGE;
			return TRUE;

		case SELECT_DBSCHEMA_COLUMN_PRIVILEGES:
			*pGuid = DBSCHEMA_COLUMN_PRIVILEGES;
			return TRUE;

		case SELECT_DBSCHEMA_COLUMNS:
			*pGuid = DBSCHEMA_COLUMNS;
			return TRUE;

		case SELECT_DBSCHEMA_CONSTRAINT_COLUMN_USAGE:
			*pGuid = DBSCHEMA_CONSTRAINT_COLUMN_USAGE;
			return TRUE;

		case SELECT_DBSCHEMA_CONSTRAINT_TABLE_USAGE:
			*pGuid = DBSCHEMA_CONSTRAINT_TABLE_USAGE;
			return TRUE;

		case SELECT_DBSCHEMA_FOREIGN_KEYS:
			*pGuid = DBSCHEMA_FOREIGN_KEYS;
			return TRUE;

		case SELECT_DBSCHEMA_INDEXES:
			*pGuid = DBSCHEMA_INDEXES;
			return TRUE;

		case SELECT_DBSCHEMA_KEY_COLUMN_USAGE:
			*pGuid = DBSCHEMA_KEY_COLUMN_USAGE;
			return TRUE;

		case SELECT_DBSCHEMA_PRIMARY_KEYS:
			*pGuid = DBSCHEMA_PRIMARY_KEYS;
			return TRUE;

		case SELECT_DBSCHEMA_PROCEDURE_PARAMETERS:
			*pGuid = DBSCHEMA_PROCEDURE_PARAMETERS;
			return TRUE;

		case SELECT_DBSCHEMA_PROCEDURES:
			*pGuid = DBSCHEMA_PROCEDURES;
			return TRUE;

		case SELECT_DBSCHEMA_PROVIDER_TYPES:
			*pGuid = DBSCHEMA_PROVIDER_TYPES;
			return TRUE;

		case SELECT_DBSCHEMA_REFERENTIAL_CONSTRAINTS:
			*pGuid = DBSCHEMA_REFERENTIAL_CONSTRAINTS;
			return TRUE;

		case SELECT_DBSCHEMA_SCHEMATA:
			*pGuid = DBSCHEMA_SCHEMATA;
			return TRUE;

		case SELECT_DBSCHEMA_SQL_LANGUAGES:
			*pGuid = DBSCHEMA_SQL_LANGUAGES;
			return TRUE;

		case SELECT_DBSCHEMA_STATISTICS:
			*pGuid = DBSCHEMA_STATISTICS;
			return TRUE;

		case SELECT_DBSCHEMA_TABLE_CONSTRAINTS:
			*pGuid = DBSCHEMA_TABLE_CONSTRAINTS;
			return TRUE;

		case SELECT_DBSCHEMA_TABLE_PRIVILEGES:
			*pGuid = DBSCHEMA_TABLE_PRIVILEGES;
			return TRUE;

		case SELECT_DBSCHEMA_TABLE:
			*pGuid = DBSCHEMA_TABLES;
			return TRUE;

		case SELECT_DBSCHEMA_TRANSLATIONS:
			*pGuid = DBSCHEMA_TRANSLATIONS;
			return TRUE;

		case SELECT_DBSCHEMA_USAGE_PRIVILEGES:
			*pGuid = DBSCHEMA_USAGE_PRIVILEGES;
			return TRUE;

		case SELECT_DBSCHEMA_VIEW_COLUMN_USAGE:
			*pGuid = DBSCHEMA_VIEW_COLUMN_USAGE;
			return TRUE;

		case SELECT_DBSCHEMA_VIEW_TABLE_USAGE:
			*pGuid  = DBSCHEMA_VIEW_TABLE_USAGE;
			return TRUE;

		case SELECT_DBSCHEMA_VIEWS:
			*pGuid = DBSCHEMA_VIEWS;
			return TRUE;

		default:
		{
			*pGuid = GUID_NULL;
			return FALSE;
		}
	}
}


//--------------------------------------------------------------------
// Function returns the uppercase character
//
// @mfunc MapWCHAR
//
//--------------------------------------------------------------------
WCHAR	MapWCHAR
(
	 WCHAR wch,
	 DWORD dwMapFlags
)
{
	WCHAR	wszBuf[2] = {wch, L'\0'};
	
	W95LCMapString(wszBuf, dwMapFlags);

	return wszBuf[0];
}


//--------------------------------------------------------------------
// Maps a string to upper case
//
// @mfunc MapToUpper
//
//--------------------------------------------------------------------
void W95LCMapString
(
	 WCHAR *pwszSource,
	 DWORD dwMapFlags
)
{
	WCHAR	wchUpper = 0;
	char	szBuf[MAXBUFLEN], szBufUpper[MAXBUFLEN];
	ULONG	cchWritten = 0, cbWritten = 0;	

	cbWritten = WideCharToMultiByte(CP_ACP, 0, pwszSource, -1,  szBuf, sizeof(szBuf), NULL, NULL);
	ASSERT(cbWritten >= wcslen(pwszSource));  

	// map to UpperCase in place
	cbWritten = LCMapStringA(GetUserDefaultLCID(), dwMapFlags, szBuf, -1, szBufUpper, sizeof(szBufUpper));
	ASSERT(cbWritten >= wcslen(pwszSource));

	cchWritten = MultiByteToWideChar(CP_ACP, 0,  szBufUpper, -1, pwszSource, (int)(wcslen(pwszSource)+1));
	ASSERT(cchWritten > 0);


	return;
}

//--------------------------------------------------------------------
// GetLocaleUnderscore
//
// @mfunc GetLocalUnderscore
// Returns a character that is the local equivalent to an underscore
//--------------------------------------------------------------------
WCHAR GetLocalUnderscore()
{
	LCID	lcid = GetUserDefaultLCID();
	LANGID	lang_id = PRIMARYLANGID(lcid);

	switch( lang_id )
	{
		case LANG_ENGLISH:
		case LANG_GERMAN:
		case LANG_FRENCH:
		case LANG_SPANISH:
		case LANG_ITALIAN:
		case LANG_PORTUGUESE:					
		case LANG_DUTCH:
			// regular English Underscore '_'
			return wszUNDERSCORE[0];

		case LANG_JAPANESE:	
		case LANG_CHINESE:
		case LANG_KOREAN:
			// regular a FULL_WIDTH version
			return MapWCHAR(wszUNDERSCORE[0], LCMAP_FULLWIDTH);

		case LANG_ARABIC:
			// return a Kashida 
			return wszKASHIDA[0];

		default:
			return wszUNDERSCORE[0];
	}
}


//----------------------------------------------------------------------
// @func CheckVariant
//
// Checks a variant's value depending on its type. Presently checks
// only a few types. Checks can be added for more types.
//
//----------------------------------------------------------------------
BOOL CheckVariant(
	VARIANT* pVar
	)
{
	BOOL		fErrorOccurred = FALSE;
	LONG_PTR	lStrLength = 0;

	if(pVar == NULL)
		return FALSE;

	switch(V_VT(pVar))
	{
	case VT_BOOL:
		if((V_BOOL(pVar) != VARIANT_TRUE) && (V_BOOL(pVar) != VARIANT_FALSE))
			fErrorOccurred = TRUE;
		break;

	case VT_BSTR:
		if(V_BSTR(pVar) == NULL)
		{
			odtLog<<L"INFO: A BSTR type property has NULL for value.\n";
		}
		else
		{
			lStrLength = wcslen(V_BSTR(pVar));
			if(lStrLength < 0)
				fErrorOccurred = TRUE;
		}
		break;

	default:
		break;
	}//switch ends.

	if(fErrorOccurred)
		return FALSE;
	else
		return TRUE;
}

//---------------------------------------------------------------------------
// GetPid
// Returns the process ID
//
// @mfunc	GetPID
//
// @rdesc WCHAR representing Process ID
//
//---------------------------------------------------------------------------
WCHAR * GetPid(void)	
{
	WCHAR * wszPID = (WCHAR *) PROVIDER_ALLOC(sizeof(WCHAR) * 12);


	_itow((_getpid()>=0 ? _getpid() : -_getpid()), wszPID, 10);		

	
	PRVTRACE(L"%sGetPid):The process ID is '%s'\n",wszPRIVLIBT,wszPID);
	return wszPID;	
}



//---------------------------------------------------------------------------
// GetRandomNumber
// Returns random number seeded by computer time  .
//
// @mfunc GetRandomNumber
//
// @rdesc WCHAR holding random number
//
//---------------------------------------------------------------------------
WCHAR * GetRandomNumber(void)	
{
	int 	iRand=0;	//	random number generated
	static 	BOOL fFlag;	//  has the seed been generated
	WCHAR *	pwszReturn = (WCHAR *) PROVIDER_ALLOC((sizeof(WCHAR) * MAX_ULONG) + sizeof(WCHAR));	// largest int in chars + 1
	pwszReturn[0]=L'\0';

	if (fFlag == FALSE)
	{
		// Initialize with computer time
		srand((unsigned)time(NULL));	
		fFlag = TRUE;
	}

	// Generate random number		
	iRand = rand();			
	swprintf(pwszReturn,L"%d",iRand);

	PRVTRACE(L"%sGetRandomNumber):RandomNumber = '%s'\n",wszPRIVLIBT, pwszReturn);
	return pwszReturn;
}

WCHAR * MakeObjectName(WCHAR * pwszModuleName, size_t ulMaxNameLen)
{
	WCHAR * 		pwszPID	= NULL;				// time
	WCHAR *			pwszObjectName = NULL;
	WCHAR * 		pwszRandNumber = NULL;		// random number generated
	HRESULT 		hr = E_FAIL;				// result
	size_t			ulNameLen;
	WCHAR 			wszPrefix[MAXBUFLEN];		// Intl string to append to table name
	WCHAR			wszLocaleUNDERSCORE[2];		// Locale-specific underscore

#define				NUMERIC_LEN			5		// Note both of these should match
#define				NUMERIC_LEN_WSTR	L"5"	

	pwszPID = GetPid();
	pwszRandNumber = GetRandomNumber();

	if (!pwszModuleName || !pwszPID || !pwszRandNumber)
		goto CLEANUP;

	if ( GetModInfo()->GetLocaleInfo() && GetModInfo()->GetUseIntlIdentifier() )
	{
		GetModInfo()->GetLocaleInfo()->MakeUnicodeIntlString(wszPrefix, 5);
		
		//Convert object name to upper case...
		//TODO:  Temporary hack until we change the privlib completely to
		//correctly quote identifiers.  This gets arround the problem of 
		//generating lower case intl chars that when "unquoted" produce a upper
		//case backend object, then trying to do quoted variations can't find
		//the object name on case sensitive providers (ie: Oracle)...
		W95LCMapString(wszPrefix, LCMAP_UPPERCASE);

		// Try to use locale-specific digits
		// The FULLWIDTH option will be applicable to Asian locales
		W95LCMapString(pwszPID, LCMAP_FULLWIDTH);

		// Try to use a locale specific underscore
		wszLocaleUNDERSCORE[0] = GetLocalUnderscore();
		wszLocaleUNDERSCORE[1] = L'\0';
	}
	else
	{
		//NOTE: We have to copy the name, since it could be a "static" string,
		//and later in this function it changes the string, which would crash
		//on a static string (ie: L"IAccObj" is static).
		wcscpy(wszPrefix, pwszModuleName);

		// Use a regular underscore
		wszLocaleUNDERSCORE[0] = wszUNDERSCORE[0];
		wszLocaleUNDERSCORE[1] = L'\0';
	}

	ulNameLen = wcslen(wszPrefix) + wcslen(wszLocaleUNDERSCORE) +
				__max(wcslen(pwszPID),NUMERIC_LEN) + 
				wcslen(wszLocaleUNDERSCORE) +
				__max(wcslen(pwszRandNumber),NUMERIC_LEN);

	SAFE_ALLOC(pwszObjectName, WCHAR, (ulNameLen+1) * sizeof(WCHAR));

	// If object name is larger than max use leftmost qualifying characters
	// of each portion.
	// On certain DBCS platforms, identifiers are not persisted in a Unicode format
	// and in those cases actual maximum identifier char length is maximum identifier
	// byte length
	if ( (ulNameLen > ulMaxNameLen) ||
		(GetModInfo()->GetLocaleInfo() && ulNameLen > ulMaxNameLen/2) )
	{
		wszPrefix[1] = L'\0';
		pwszPID[2] = L'\0';

		// If available space is less than pwszRandomNumber then truncate number to fit
		if (ulMaxNameLen-5 < wcslen(pwszRandNumber)) 
			pwszRandNumber[ulMaxNameLen-5] = L'\0';
	}

	// Build string to become object name
	// Looks like "MODULENAME_DATE_RANDOMNUMBER"
	// Use "<International String>_DATE_RANDOMNUMBER" if international data flag is set.
	swprintf(pwszObjectName, L"%s%s%0" NUMERIC_LEN_WSTR L"s%s%0" NUMERIC_LEN_WSTR L"s",
		wszPrefix, wszLocaleUNDERSCORE, pwszPID, wszLocaleUNDERSCORE, pwszRandNumber);

	// Force returned identified to be less than ulMaxNameLen
	if( wcslen(pwszObjectName) > ulMaxNameLen )
	{
		pwszObjectName[ulMaxNameLen] = L'\0';
	}

CLEANUP:
	PROVIDER_FREE(pwszPID);
	PROVIDER_FREE(pwszRandNumber);
	
	return pwszObjectName;
}


//---------------------------------------------------------------------------
// iswcharMappable
// Returns true if the input unicode character can safely be mapped to 
// the active ANSI code page.
//
// @mfunc iswcharMappable
//---------------------------------------------------------------------------
BOOL iswcharMappable(WCHAR wch)	
{
	BOOL	fNotMappable = FALSE;	
	WCHAR	wszSource[2];
	char	szDst[2];
	ULONG	cchBytesWritten = 0;

	wszSource[0] = wch;
	wszSource[1] = 0x0000;

	cchBytesWritten = WideCharToMultiByte(	CP_ACP,
											0,
											wszSource, 
											-1,
											szDst, 
											sizeof(szDst),
											NULL, 
											&fNotMappable);

	if (cchBytesWritten!=2)  // one byte for source char, one byte for null terminator
		return FALSE;

	return !fNotMappable;
}


//---------------------------------------------------------------------------
// InitializeConfProv
//
// Sets up the Conformance Provider
//--------------------------------------------------------------------------
BOOL InitializeConfProv(CThisTestModule* pThisTestModule)
{
	BOOL			fSuccess = FALSE;
	WCHAR*			pwszRootURL = NULL;
	WCHAR*			pwszNewURL = NULL;
	WCHAR*			pwszCmdURL = NULL;
	WCHAR*			pwszRowURL = NULL;
	WCHAR*			pwszRowQuery = NULL;

	CList<WCHAR *,WCHAR *>	NativeTypesList;
	CList<DBTYPE,DBTYPE>	ProviderTypesList;
	CCol					col;
	DBCOLUMNDESC			*rgColumnDesc			= NULL;
	DBORDINAL				cColumnDesc				= 0;
	DBORDINAL				ulIndxCol				= 0;
	BOOL					fTableExist				= FALSE;
	
	CList <CCol, CCol&>		colList;
	POSITION				pos;
	DBORDINAL				cIter=0;
	DBORDINAL				ulParentOrdinal;
	DBORDINAL				rgOrdinals[2];	// ConfProv reserves two columns for its own use

#if 0
	// We may be using an INI file
	// In which case, we don't need to do anything
	if(GetModInfo()->GetTreeName())
		return TRUE;
#endif

	pwszRootURL = GetModInfo()->GetRootURL();
	if (!pwszRootURL)
		return TRUE; // bail out, not an error

	//An INI file exists with a URL section. So, no need to create a 
	//table and tree.
	if(GetModInfo()->GetParseObject()->GetURL(ROW_INTERFACE))
		return TRUE;

	// If the specified URL isn't "oledb://dso/session"
	// then we won't do anything more
	if(0 != wcsncmp(pwszRootURL, g_wszConfProvPrefix, wcslen(g_wszConfProvPrefix)))
		return TRUE; // nothing left to do

	// Create a table.
	g_pConfProvTable = new CTable(pThisTestModule->m_pIUnknown2);
	
	g_pConfProvTable->CreateColInfo(NativeTypesList, ProviderTypesList);	
	g_pConfProvTable->DuplicateColList(colList);

	pos = colList.GetHeadPosition();
	TESTC(NULL != pos)
	
	cColumnDesc = 2;
	for (; pos; )
	{
		POSITION	oldPos = pos;

		col = colList.GetNext(pos);
		if (!col.GetNullable())
			colList.RemoveAt(oldPos);
		else
		{
			col.SetColNum(++cColumnDesc);
			colList.SetAt(oldPos, col);
		}
	}

	TESTC(g_pConfProvTable->GetColWithAttr(COL_COND_AUTOINC, &ulIndxCol)); 
	// duplicate the first column - use one for index and one for values
	TESTC_(g_pConfProvTable->GetColInfo(ulIndxCol, col), S_OK);
	col.SetColName(L"RESOURCE_PARSENAME");
	col.SetNullable(FALSE); 
	col.SetColNum(1);
	colList.AddHead(col);

	// Find a candidate for the RESOURCE_PARENTNAME columns
	for(cIter=1; cIter <= g_pConfProvTable->CountColumnsOnTable(); cIter++)
	{
		TESTC_(g_pConfProvTable->GetColInfo(cIter, col), S_OK);
			
		if (col.GetIsLong() == FALSE && col.GetIsFixedLength() == FALSE &&
			(col.GetProviderType() == DBTYPE_WSTR ||
			 col.GetProviderType() == DBTYPE_BSTR ||
			 col.GetProviderType() == DBTYPE_STR ))
		break;
	}
	
	// Did we find a candidate?
	TESTC(cIter < g_pConfProvTable->CountColumnsOnTable());
	ulParentOrdinal = col.GetColNum();

	col.SetColName(L"RESOURCE_PARENTNAME");
	col.SetIsFixedLength(FALSE);
	col.SetColumnSize(200);
	col.SetColNum(2);
	colList.AddHead(col);

	g_pConfProvTable->SetColList(colList);

	
	g_pConfProvTable->SetBuildColumnDesc(FALSE);	// do not create ColList again
	
	cColumnDesc = g_pConfProvTable->CountColumnsOnTable();
	g_pConfProvTable->BuildColumnDescs(&rgColumnDesc);
	
	// make sure the first column is not autoincrementable 
	FreeProperties(&rgColumnDesc[0].cPropertySets, &rgColumnDesc[0].rgPropertySets);
	SAFE_FREE(rgColumnDesc[0].pwszTypeName);
	
	// make sure the parent column doesn't specify a type name
	SAFE_FREE(rgColumnDesc[ulParentOrdinal-1].pwszTypeName);

	g_pConfProvTable->SetColumnDesc(rgColumnDesc, cColumnDesc);
//	TESTC_(g_pConfProvTable->CreateTable(0, cColumnDesc), S_OK);
	TESTC_(g_pConfProvTable->CreateTable(0, 0), S_OK); // avoid creating a rowset on the last col

	// create a unique index on the two special columns
	rgOrdinals[0] = 1;
	rgOrdinals[1] = 2;
	TESTC_(g_pConfProvTable->CreateIndex(rgOrdinals,2,UNIQUE), S_OK);

	// get the name of the created table
	// and alter the ROOT_URL.
	pwszNewURL = (WCHAR *)PROVIDER_ALLOC((wcslen(pwszRootURL)+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszNewURL != NULL);
	wcscpy(pwszNewURL, pwszRootURL);
	wcscat(pwszNewURL, L"/");
	wcscat(pwszNewURL, g_pConfProvTable->GetTableName());

	//CreateTree with one node.
	g_pConfProvTree = new CTree(pThisTestModule->m_pIUnknown2);
	g_pConfProvTree->CreateTree(pwszNewURL, 1, 2);

	pwszRootURL = g_pConfProvTree->GetRootURL();
	TESTC(pwszRootURL && wcslen(pwszRootURL)>1)

	pwszCmdURL = (WCHAR *)PROVIDER_ALLOC((wcslen(L"confprov://dso/session/")+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszCmdURL != NULL);
	wcscpy(pwszCmdURL, L"confprov://dso/session/");
	wcscat(pwszCmdURL, L"select * from ");
	wcscat(pwszCmdURL, g_pConfProvTable->GetTableName());

	pwszRowURL = (WCHAR *)PROVIDER_ALLOC((wcslen(pwszRootURL)+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszRowURL != NULL);
	wcscpy(pwszRowURL, pwszRootURL);
	wcscat(pwszRowURL, L"/0");

	GetModInfo()->SetRootURL(pwszRowURL);

	TESTC(GetModInfo()->GetParseObject()->SetURL(DATASOURCE_INTERFACE, pwszRootURL))
	TESTC(GetModInfo()->GetParseObject()->SetURL(SESSION_INTERFACE, pwszRootURL))
	TESTC(GetModInfo()->GetParseObject()->SetURL(ROWSET_INTERFACE, pwszNewURL))
	TESTC(GetModInfo()->GetParseObject()->SetURL(ROW_INTERFACE, pwszRowURL))
	TESTC(GetModInfo()->GetParseObject()->SetURL(STREAM_INTERFACE, pwszRootURL))
	TESTC(GetModInfo()->GetParseObject()->SetURL(COMMAND_INTERFACE, pwszCmdURL))

	// Override the default Row Scoped Command Query
	pwszRowQuery = (WCHAR *)PROVIDER_ALLOC((wcslen(g_pConfProvTable->GetTableName())+wcslen(wszSELECT_ALLFROMTBL)+1)*sizeof(WCHAR));
	TESTC(pwszRowQuery != NULL);
	swprintf(pwszRowQuery, wszSELECT_ALLFROMTBL, g_pConfProvTable->GetTableName());
	
	GetModInfo()->SetRowScopedQuery(pwszRowQuery);

	fSuccess = TRUE;

CLEANUP:
	PROVIDER_FREE(pwszRowQuery);	
	PROVIDER_FREE(pwszNewURL);
	PROVIDER_FREE(pwszCmdURL);
	PROVIDER_FREE(pwszRowURL);

	return fSuccess;	
}

//---------------------------------------------------------------------------
// ConfProvTerminate
//
// Cleans up Conformance Provider work, if any
//--------------------------------------------------------------------------
BOOL ConfProvTerminate()
{
	BOOL	fSuccess = TRUE;

	if (g_pConfProvTable)
	{
		fSuccess = SUCCEEDED(g_pConfProvTable->DropTable());
		SAFE_DELETE(g_pConfProvTable);
	}

	if(g_pConfProvTree)
	{
		g_pConfProvTree->DestroyTree();
		SAFE_DELETE(g_pConfProvTree);
	}

	return fSuccess;
}

//---------------------------------------------------------------------------
// CoCreate
//
// Checks for existence of CoCreateInstanceEx otherwise uses CoCreateInstance
//--------------------------------------------------------------------------
HRESULT CoCreate
(
	REFCLSID rclsid, 
	LPUNKNOWN pUnkOuter, 
	DWORD dwClsContext, 
	REFIID riid, 
	LPVOID * ppv 
)
{
	HINSTANCE hinst = NULL;
	HRESULT hr = E_FAIL;

	// Find out if DCOM is installed
	// DCOM supports CoCreateInstanceEx, without DCOM you get CoCreateInstance only.
	if (hinst = LoadLibrary("ole32.dll"))
	{
		if (GetProcAddress(hinst, "CoCreateInstanceEx"))
		{
			COSERVERINFO	si;
			MULTI_QI		mq;

			//Set up CreateInstanceEx structures
			memset(&si, 0, sizeof(COSERVERINFO));
			memset(&mq, 0, sizeof(MULTI_QI));

			//Set the Remote Server Name, only if remote.
			//Unfortunatly CoCreateInstanceEx will "upgrade" to REMOTE if the specified name does
			//not match the current machine name.  This is not the intent we want, and using RunList files
			//on different machines will cause this problem...
			if(GetModInfo()->GetClassContext() & CLSCTX_REMOTE_SERVER)
				si.pwszName = GetModInfo()->GetRemoteMachine();

			//Set the IID for the call
			mq.pIID = &riid;

			// Use CoCreateInstanceEx
			QTESTC_(hr = CoCreateInstanceEx(rclsid, pUnkOuter, dwClsContext, &si, 1, &mq), S_OK);

			*ppv = mq.pItf;
		}
		else
			QTESTC_(hr = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv), S_OK);
	}

CLEANUP:

	if (hinst)
		FreeLibrary(hinst);

	return hr;
}


//---------------------------------------------------------------------------
// CoInit
//
// Checks for existence of CoInitializeEx otherwise uses CoInitialize
//--------------------------------------------------------------------------
HRESULT CoInit(DWORD dwCoInit)
{
	HINSTANCE hinst = NULL;
	HRESULT hr = E_FAIL;

	// Find out if DCOM is installed
	// DCOM supports CoInitializeEx, without DCOM you get CoInitialize only.
	if (hinst = LoadLibrary("ole32.dll"))
	{
		if (GetProcAddress(hinst, "CoInitializeEx"))
		{
			// Use CoCreateInstanceEx
			QTESTC_(hr = CoInitializeEx(NULL, dwCoInit), S_OK);
		}
		else
		{	
			// CoInitialize supports apartment threading only
			QTESTC(dwCoInit == COINIT_APARTMENTTHREADED);
			QTESTC_(hr = CoInitialize(NULL), S_OK);
		}
	}

CLEANUP:

	if (hinst)
		FreeLibrary(hinst);

	return hr;
}

//---------------------------------------------------------------------------
// FetchRowScopedQuery
//
// This function retrieves the row scoped query that is in the ini file or
// that has been specified in the initialization string.
// This must be a query that can be issued to row scoped commands (i.e. 
// commands whose parent is a Row object). 
// Also, this query must return all the childs rows and can be
// called from any row in the hierarchy.
// This would be equivalent to Monarch SQL's "select * from (DEFAULT_SCOPE)"
//--------------------------------------------------------------------------
WCHAR* FetchRowScopedQuery(EQUERY eQuery)
{
	WCHAR*	pwszScopedCommand = NULL;

	if(GetModInfo()->GetFileName())
		pwszScopedCommand = wcsDuplicate(GetModInfo()->GetParseObject()->GetQuery(eQuery));
	//Otherwise just use InitString DEFAULTQUERY=
	else
		pwszScopedCommand = wcsDuplicate(GetModInfo()->GetRowScopedQuery());

	return pwszScopedCommand;
}


//--------------------------------------------------------------------
// GetIIDString
//
// @func	WCHAR * GetIIDString
// Helper for converting IID into useful string.  
//--------------------------------------------------------------------
LPCWSTR GetIIDString(REFIID iid)
{
	static WCHAR wszUnfamiliar[80];
	WCHAR wszByte[3];

	struct IID_ENTRY
	{
		const IID* piid;
		LPCWSTR lpwszName;
	};

#define MAKE_IID_ENTRY(name) { &name, L#name }

	static const IID_ENTRY iidNameTable[] =
	{
		// Standard COM IID's
		MAKE_IID_ENTRY(IID_IAdviseSink),
		MAKE_IID_ENTRY(IID_IAdviseSink2),
		MAKE_IID_ENTRY(IID_IBindCtx),
		MAKE_IID_ENTRY(IID_IClassFactory),
/*
#ifndef _MAC
		MAKE_IID_ENTRY(IID_IContinueCallback),
		MAKE_IID_ENTRY(IID_IEnumOleDocumentViews),
		MAKE_IID_ENTRY(IID_IOleCommandTarget),
		MAKE_IID_ENTRY(IID_IOleDocument),
		MAKE_IID_ENTRY(IID_IOleDocumentSite),
		MAKE_IID_ENTRY(IID_IOleDocumentView),
		MAKE_IID_ENTRY(IID_IPrint),
#endif
*/
		MAKE_IID_ENTRY(IID_IDataAdviseHolder),
		MAKE_IID_ENTRY(IID_IDataObject),
		MAKE_IID_ENTRY(IID_IDebug),
		MAKE_IID_ENTRY(IID_IDebugStream),
		MAKE_IID_ENTRY(IID_IDfReserved1),
		MAKE_IID_ENTRY(IID_IDfReserved2),
		MAKE_IID_ENTRY(IID_IDfReserved3),
		MAKE_IID_ENTRY(IID_IDispatch),
		MAKE_IID_ENTRY(IID_IDropSource),
		MAKE_IID_ENTRY(IID_IDropTarget),
		MAKE_IID_ENTRY(IID_IEnumCallback),
		MAKE_IID_ENTRY(IID_IEnumFORMATETC),
		MAKE_IID_ENTRY(IID_IEnumGeneric),
		MAKE_IID_ENTRY(IID_IEnumHolder),
		MAKE_IID_ENTRY(IID_IEnumMoniker),
		MAKE_IID_ENTRY(IID_IEnumOLEVERB),
		MAKE_IID_ENTRY(IID_IEnumSTATDATA),
		MAKE_IID_ENTRY(IID_IEnumSTATSTG),
		MAKE_IID_ENTRY(IID_IEnumString),
		MAKE_IID_ENTRY(IID_IEnumUnknown),
		MAKE_IID_ENTRY(IID_IEnumVARIANT),
//      MAKE_IID_ENTRY(IID_IExternalConnection),
		MAKE_IID_ENTRY(IID_IInternalMoniker),
		MAKE_IID_ENTRY(IID_ILockBytes),
		MAKE_IID_ENTRY(IID_IMalloc),
		MAKE_IID_ENTRY(IID_IMarshal),
		MAKE_IID_ENTRY(IID_IMessageFilter),
		MAKE_IID_ENTRY(IID_IMoniker),
		MAKE_IID_ENTRY(IID_IOleAdviseHolder),
		MAKE_IID_ENTRY(IID_IOleCache),
		MAKE_IID_ENTRY(IID_IOleCache2),
		MAKE_IID_ENTRY(IID_IOleCacheControl),
		MAKE_IID_ENTRY(IID_IOleClientSite),
		MAKE_IID_ENTRY(IID_IOleContainer),
		MAKE_IID_ENTRY(IID_IOleInPlaceActiveObject),
		MAKE_IID_ENTRY(IID_IOleInPlaceFrame),
		MAKE_IID_ENTRY(IID_IOleInPlaceObject),
		MAKE_IID_ENTRY(IID_IOleInPlaceSite),
		MAKE_IID_ENTRY(IID_IOleInPlaceUIWindow),
		MAKE_IID_ENTRY(IID_IOleItemContainer),
		MAKE_IID_ENTRY(IID_IOleLink),
		MAKE_IID_ENTRY(IID_IOleManager),
		MAKE_IID_ENTRY(IID_IOleObject),
		MAKE_IID_ENTRY(IID_IOlePresObj),
		MAKE_IID_ENTRY(IID_IOleWindow),
		MAKE_IID_ENTRY(IID_IPSFactory),
		MAKE_IID_ENTRY(IID_IParseDisplayName),
		MAKE_IID_ENTRY(IID_IPersist),
		MAKE_IID_ENTRY(IID_IPersistFile),
		MAKE_IID_ENTRY(IID_IPersistStorage),
		MAKE_IID_ENTRY(IID_IPersistStream),
		MAKE_IID_ENTRY(IID_IProxyManager),
		MAKE_IID_ENTRY(IID_IRootStorage),
		MAKE_IID_ENTRY(IID_IRpcChannel),
		MAKE_IID_ENTRY(IID_IRpcProxy),
		MAKE_IID_ENTRY(IID_IRpcStub),
		MAKE_IID_ENTRY(IID_IRunnableObject),
		MAKE_IID_ENTRY(IID_IRunningObjectTable),
		MAKE_IID_ENTRY(IID_IStdMarshalInfo),
		MAKE_IID_ENTRY(IID_IStorage),
		MAKE_IID_ENTRY(IID_IStream),
		MAKE_IID_ENTRY(IID_IStubManager),
		MAKE_IID_ENTRY(IID_IUnknown),
		MAKE_IID_ENTRY(IID_IViewObject),
		MAKE_IID_ENTRY(IID_IViewObject2),
		MAKE_IID_ENTRY(IID_NULL),

		// OLE DB IID's
		MAKE_IID_ENTRY(IID_IAccessor),
		MAKE_IID_ENTRY(IID_IAlterIndex),
		MAKE_IID_ENTRY(IID_IAlterTable),
		MAKE_IID_ENTRY(IID_IChapteredRowset),
		MAKE_IID_ENTRY(IID_IColumnsInfo),
		MAKE_IID_ENTRY(IID_IColumnsRowset),
		MAKE_IID_ENTRY(IID_ICommand),
		MAKE_IID_ENTRY(IID_ICommandPersist),
		MAKE_IID_ENTRY(IID_ICommandPrepare),
		MAKE_IID_ENTRY(IID_ICommandProperties),
		MAKE_IID_ENTRY(IID_ICommandText),
		MAKE_IID_ENTRY(IID_ICommandWithParameters),
		MAKE_IID_ENTRY(IID_IConvertType),
		MAKE_IID_ENTRY(IID_IDBAsynchNotify),
		MAKE_IID_ENTRY(IID_IDBAsynchStatus),
		MAKE_IID_ENTRY(IID_IDBCreateCommand),
		MAKE_IID_ENTRY(IID_IDBCreateSession),
		MAKE_IID_ENTRY(IID_IDBDataSourceAdmin),
		MAKE_IID_ENTRY(IID_IDBInfo),
		MAKE_IID_ENTRY(IID_IDBInitialize),
		MAKE_IID_ENTRY(IID_IDBProperties),
		MAKE_IID_ENTRY(IID_IDBSchemaRowset),
		MAKE_IID_ENTRY(IID_IErrorInfo),
		MAKE_IID_ENTRY(IID_IErrorLookup),
		MAKE_IID_ENTRY(IID_IErrorRecords),
		MAKE_IID_ENTRY(IID_IGetDataSource),
		MAKE_IID_ENTRY(IID_IIndexDefinition),
		MAKE_IID_ENTRY(IID_IMultipleResults),
		MAKE_IID_ENTRY(IID_IOpenRowset),
		MAKE_IID_ENTRY(IID_IParentRowset),
		MAKE_IID_ENTRY(IID_IRowset),
		MAKE_IID_ENTRY(IID_IRowsetChange),
		MAKE_IID_ENTRY(IID_IRowsetChapterMember),
		MAKE_IID_ENTRY(IID_IRowsetFind),
		MAKE_IID_ENTRY(IID_IRowsetIdentity),
		MAKE_IID_ENTRY(IID_IRowsetIndex),
		MAKE_IID_ENTRY(IID_IRowsetInfo),
		MAKE_IID_ENTRY(IID_IRowsetLocate),
		MAKE_IID_ENTRY(IID_IRowsetNotify),
		MAKE_IID_ENTRY(IID_IRowsetRefresh),
		MAKE_IID_ENTRY(IID_IRowsetScroll),
		MAKE_IID_ENTRY(IID_IRowsetUpdate),
		MAKE_IID_ENTRY(IID_IRowsetView),
		MAKE_IID_ENTRY(IID_ISequentialStream),
		MAKE_IID_ENTRY(IID_ISessionProperties),
		MAKE_IID_ENTRY(IID_ISourcesRowset),
		MAKE_IID_ENTRY(IID_ISQLErrorInfo),
		MAKE_IID_ENTRY(IID_ISupportErrorInfo),
		MAKE_IID_ENTRY(IID_ITableDefinition),
		MAKE_IID_ENTRY(IID_ITransaction),
		MAKE_IID_ENTRY(IID_ITransactionJoin),
		MAKE_IID_ENTRY(IID_ITransactionLocal),
		MAKE_IID_ENTRY(IID_ITransactionObject),
		MAKE_IID_ENTRY(IID_ITransactionOptions),
		MAKE_IID_ENTRY(IID_IViewChapter),
		MAKE_IID_ENTRY(IID_IViewFilter),
		MAKE_IID_ENTRY(IID_IViewRowset),
		MAKE_IID_ENTRY(IID_IViewSort),
	};
#undef MAKE_IID_ENTRY

	// look for it in the table
	for (int i = 0; i < NUMELEM(iidNameTable); i++)
	{
		if (iid == *iidNameTable[i].piid)
			return iidNameTable[i].lpwszName;
	}
	// if we get here, it is some IID_ we haven't heard of...

	swprintf(wszUnfamiliar, L"%8.8X-%4.4X-%4.4X-",
		iid.Data1, iid.Data2, iid.Data3);
	for (int nIndex = 0; nIndex < 8; nIndex++)
	{
		swprintf(wszByte, L"%2.2X", iid.Data4[nIndex]);
		wcscat(wszUnfamiliar, wszByte);
	}

	return wszUnfamiliar;
}



//--------------------------------------------------------------------
// CompareID
//
// @func	HRESULT CompareID
// Helper for comparing 2 identifiers.  
//--------------------------------------------------------------------
HRESULT CompareID(
	BOOL		*pfResult,			//[out] pointer to result
	WCHAR		*pwszFrontEndID,	// [in]	first identifier
	WCHAR		*pwszBackEndID,		// [in]	second iderntifier
	IUnknown	*pIUnknown/*=NULL*/// [in]	pointer to datasource interface
)
{
	LONG		lIDCase		= 0;	// inexistent value
	HRESULT		hr			= E_FAIL;
	WCHAR		*pwszCustID	= NULL;
	WCHAR		*pwszBackID	= NULL;
	VARIANT		Variant;
	BOOL		fQuoted		= FALSE;
	DBPROPID	dwPropID	= DBPROP_IDENTIFIERCASE;
	WCHAR		wcsQuotePrefix;
	WCHAR		wcsQuoteSuffix;
	
	TESTC(NULL != pfResult);

	if (NULL == pwszFrontEndID || NULL == pwszBackEndID)
	{
		*pfResult = (pwszFrontEndID == pwszBackEndID);
		return S_OK;
	}
	
	if (S_OK == GetQuoteLiteralInfo(pIUnknown, &wcsQuotePrefix, &wcsQuoteSuffix))
	{
		// check whether the symbol is quoted
		fQuoted		=	wcsQuotePrefix == pwszFrontEndID[0] 
					&&	wcsQuotePrefix == pwszFrontEndID[wcslen(pwszFrontEndID)-1];
		if (fQuoted)
			dwPropID = DBPROP_QUOTEDIDENTIFIERCASE;
	}

	if (pIUnknown)
	{
		VariantInit(&Variant);
		TESTC(GetProperty(dwPropID, 
			DBPROPSET_DATASOURCEINFO, pIUnknown, &Variant));
		TESTC(VT_I4 == Variant.vt);
		lIDCase = Variant.lVal;
		TESTC(	DBPROPVAL_IC_UPPER == lIDCase
			||	DBPROPVAL_IC_LOWER == lIDCase
			||	DBPROPVAL_IC_SENSITIVE == lIDCase
			||	DBPROPVAL_IC_MIXED == lIDCase);
	}

	if (!fQuoted)
		pwszCustID = wcsDuplicate(pwszFrontEndID);
	else
	{
		pwszCustID = wcsDuplicate(pwszFrontEndID+1);
		pwszCustID[wcslen(pwszCustID)-1] = L'\0';
	}
	pwszBackID = wcsDuplicate(pwszBackEndID);

	switch (lIDCase)
	{
		case DBPROPVAL_IC_UPPER:
			_wcsupr(pwszCustID);
			break;
		case DBPROPVAL_IC_LOWER:
			_wcslwr(pwszCustID);
			break;
		case DBPROPVAL_IC_SENSITIVE:
			break;
		case DBPROPVAL_IC_MIXED:
			_wcsupr(pwszCustID);
			_wcsupr(pwszBackID);
			break;
	}

	*pfResult = (0 == wcscmp(pwszCustID, pwszBackID));
	hr = S_OK;

CLEANUP:
	SAFE_FREE(pwszCustID);
	SAFE_FREE(pwszBackID);
	return hr;
} //CompareID



//--------------------------------------------------------------------------
//
// GetQuoteLiteralInfo
//--------------------------------------------------------------------------
HRESULT	GetQuoteLiteralInfo(
	IUnknown	*pIUnknown,			// [in]	initialized DSO interface
	WCHAR		*pwcsQuotePrefix,	// [out] prefix char
	WCHAR		*pwcsQuoteSuffix	// [out] suffix char
)
{
	IDBInfo*		pInterface = NULL;
	DBLITERAL		rgLiteral[2]={DBLITERAL_QUOTE_PREFIX, DBLITERAL_QUOTE_SUFFIX};
	DBLITERALINFO*	rgLiteralInfo = NULL;
	ULONG			cLiteralInfo, i;
	OLECHAR*		pCharBuffer = NULL;
	IGetDataSource*	pIGetDataSource=NULL;	// IGetDataSource interface pointer
	HRESULT			hr = E_FAIL;

	if (!pwcsQuotePrefix || !pwcsQuoteSuffix)\
		return E_FAIL;

	if(!VerifyInterface(pIUnknown, IID_IDBInfo, DATASOURCE_INTERFACE, (IUnknown**)&pInterface))
		return E_FAIL;

	hr = pInterface->GetLiteralInfo(2, rgLiteral, &cLiteralInfo, &rgLiteralInfo, &pCharBuffer);
	if (S_OK != hr)
		goto CLEANUP;

	for (i=0; i< cLiteralInfo; i++)
	{
		switch (rgLiteralInfo[i].lt)
		{
		case DBLITERAL_QUOTE_PREFIX:
			// get quote prefix info 
			if (rgLiteralInfo[i].cchMaxLen)
				*pwcsQuotePrefix = rgLiteralInfo[i].pwszLiteralValue[0];
			else
				goto CLEANUP;
			break;
		case DBLITERAL_QUOTE_SUFFIX:
			if (rgLiteralInfo[i].cchMaxLen)
				*pwcsQuoteSuffix = rgLiteralInfo[i].pwszLiteralValue[0];
			else
				goto CLEANUP;
			break;
		}
	}

	hr = S_OK;

CLEANUP:
	SAFE_RELEASE(pInterface);
	PROVIDER_FREE(pCharBuffer);
	PROVIDER_FREE(rgLiteralInfo);
	return hr;
} //GetQuoteLiteralInfo




//--------------------------------------------------------------------------------
//
// Check whether the return value is a legal one
//--------------------------------------------------------------------------------
BOOL CheckResult(
	HRESULT			hr,
	DBCOUNTITEM		cValidRes,
	HRESULT			*rgValidRes
)
{
	DBCOUNTITEM	index;

	for (index = 0; index < cValidRes; index++)
	{
		if (hr == rgValidRes[index])
			return TRUE;
	}

	odtLog << "<ERROR message = \"Illegal return value retrieved " << GetErrorName(hr)
		<< "\"> </ERROR>\n";
	return FALSE;
} //CheckResult




//--------------------------------------------------------------------------------
//
// Check whether the status returned is a legal one
//--------------------------------------------------------------------------------
BOOL CheckStatus(
	DBSTATUS		dbStatus,
	DBCOUNTITEM		cValidStatus,
	DBSTATUS		*rgValidStatus
)
{
	DBCOUNTITEM	index;

	for (index = 0; index < cValidStatus; index++)
	{
		if (dbStatus == rgValidStatus[index])
			return TRUE;
	}

	odtLog << "<ERROR message = \"Illegal status retrieved " << GetStatusName(dbStatus) 
		<< "\"> </ERROR>\n";
	return FALSE;
} //CheckStatus



//------------------------------------------------------------------------------
//
// wrapper for calling IScopedOperations::Copy and doing  basic general checking
//------------------------------------------------------------------------------
HRESULT IScopedOperations_Copy(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	DBCOUNTITEM			cRows,					// [in] number of copy ops	
	WCHAR				**rgpwszSourceURLs,		// [in] source URLs
	WCHAR				**rgpwszDestURLs,		// [in] destination URLs
	DWORD				dwCopyFlags,			// [in] copy flags
	IAuthenticate		*pIAuthenticate,		// [in] authentication interface
	DBSTATUS			*rgdwStatus,			// [out] filled on output
	WCHAR				**rgpwszNewURLs,		// [in/out] new URLs
	WCHAR				**ppStringsBuffer	// [out] buffer for URL strings
)
{
	HRESULT				hr;
	HRESULT				hres		= E_FAIL;
	ULONG				cOp;
	IRow				*pIRow		= NULL;
	DBCOUNTITEM			ulRow;

	// how many operations succeded and how many failed
	DBCOUNTITEM			ulGood		= 0;
	DBCOUNTITEM			ulBad		= 0;

	// all the legal statuses
	DBSTATUS			rgValidStatus[]	= {
											DBSTATUS_S_OK,
											DBSTATUS_E_CANCELED,
											DBSTATUS_E_CANNOTCOMPLETE,
											DBSTATUS_E_DOESNOTEXIST,
											DBSTATUS_E_INVALIDURL,
											DBSTATUS_E_NOTCOLLECTION,
											DBSTATUS_E_OUTOFSPACE,
											DBSTATUS_E_PERMISSIONDENIED,
											DBSTATUS_E_RESOURCEEXISTS,
											DBSTATUS_E_RESOURCELOCKED,
											DBSTATUS_E_RESOURCEOUTOFSCOPE,
											DBSTATUS_E_UNAVAILABLE,
											DBSTATUS_E_VOLUMENOTFOUND, 
										};

	DBCOUNTITEM			cValidStatus = NUMELEM(rgValidStatus);

	// all the legal return values for the call
	HRESULT				rgValidRes[]	= {
											S_OK,
											DB_S_ASYNCHRONOUS,
											DB_S_BUFFERFULL,
											DB_S_ERRORSOCCURRED,
											DB_E_ASYNCNOTSUPPORTED,
											DB_E_CANCELED,
											DB_E_ERRORSOCCURRED,
											DB_SEC_E_SAFEMODE_DENIED,
											E_FAIL,
											E_INVALIDARG,
											E_OUTOFMEMORY,
											E_UNEXPECTED,
										};
	DBCOUNTITEM			cValidRes = NUMELEM(rgValidRes);

	
	PRVTRACE(L"<IScopedOperations_Copy>\n");

	TESTC(NULL != pIScopedOperations);

	PRVTRACE(L"\t<IScopedOperations::Copy cRows = %u "
		L"rgpwszSourceURLs = %p rgpwszDestURLs = %p dwCopyFlags = %u "
		L"pIAuthenticate = %p rgdwStatus = %p rgpwszNewURLs = %p "
		L"ppStringsBuffer = %p>\n", cRows, rgpwszSourceURLs, rgpwszDestURLs, 
		dwCopyFlags, pIAuthenticate, rgdwStatus, rgpwszNewURLs, ppStringsBuffer);

	// the actual tested operation
	hr = pIScopedOperations->Copy(cRows, (const WCHAR**)rgpwszSourceURLs,
		(const WCHAR**)rgpwszDestURLs, dwCopyFlags, pIAuthenticate, rgdwStatus,
		rgpwszNewURLs, ppStringsBuffer);

	// check and print the return value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);
	PRVTRACE(L"\t\t<RET_VAL value = %s> </RET_VAL>\n", GetErrorName(hr)); 

	// check and display statuses
	for (ulRow = 0; rgpwszSourceURLs && rgpwszDestURLs && ulRow < cRows; ulRow++)
	{
		PRVTRACE(L"\t\t<OPERATION name = copy source_url = %s dest_url = %s "
			L" new_url = %s status = %s> </OPERATION>\n", 
			((rgpwszSourceURLs[ulRow])? rgpwszSourceURLs[ulRow]: L"NULL"), 
			((rgpwszDestURLs[ulRow])? rgpwszDestURLs[ulRow]: L"NULL"), 
			((rgpwszNewURLs && ppStringsBuffer && rgpwszNewURLs[ulRow])? rgpwszNewURLs[ulRow]: L"NULL"), 
			(rgdwStatus ? GetStatusName(rgdwStatus[ulRow]): L"unknown")
		);
	}

	PRVTRACE(L"\t</IScopedOperations::Copy>\n"); 

	// check that the return value was one mentioned by the spec
	switch (hr)
	{
		case S_OK:
			// check that all the destination URLs exist (can be bound)
			for (cOp=0; cOp<cRows; cOp++)
			{
				WCHAR	*pwszDestURL;

				//check status
				TESTC(!rgdwStatus || (DBSTATUS_S_OK == rgdwStatus[cOp]));

				// check destination
				pwszDestURL = rgpwszDestURLs? rgpwszDestURLs[cOp]: NULL;
				if (rgpwszNewURLs && ppStringsBuffer)
					pwszDestURL = rgpwszNewURLs[cOp];
				
				TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, pwszDestURL, 
					DBBINDURLFLAG_READ, DBGUID_ROW, 
					IID_IRow, NULL, NULL, NULL, 
					(IUnknown**)&pIRow), S_OK);
				TESTC(NULL != pIRow);
				TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE,  IID_IRow));
				SAFE_RELEASE(pIRow);

				// check source
				TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszSourceURLs[cOp], 
					DBBINDURLFLAG_READ, DBGUID_ROW, 
					IID_IRow, NULL, NULL, NULL, 
					(IUnknown**)&pIRow), S_OK);
				TESTC(NULL != pIRow);
				TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE,  IID_IRow));
				SAFE_RELEASE(pIRow);
			}
			break;
		
		case DB_S_ERRORSOCCURRED:
			TESTC(0<cRows);

			// the op was not atomic
			TESTC(0 == (dwCopyFlags & DBCOPY_ATOMIC));

			if (rgdwStatus)
			{
				// some nodes were copied, some not
				// check all the statuses
				for (cOp=0; cOp<cRows; cOp++)
				{
					WCHAR	*pwszDestURL;
	
					COMPARE(CheckStatus(rgdwStatus[cOp], cValidStatus, rgValidStatus), TRUE);

					if (DBSTATUS_S_OK == rgdwStatus[cOp])
						ulGood++;
					else
						ulBad++;

					pwszDestURL = rgpwszDestURLs? rgpwszDestURLs[cOp]: NULL;
					if (rgpwszNewURLs && ppStringsBuffer)
						pwszDestURL = rgpwszNewURLs[cOp];
			
					if (DBSTATUS_S_OK == rgdwStatus[cOp])
					{
						// destination exist, can be bound
						TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, pwszDestURL, 
							DBBINDURLFLAG_READ, DBGUID_ROW, 
							IID_IRow, NULL, NULL, NULL, 
							(IUnknown**)&pIRow), S_OK);
						TESTC(NULL != pIRow);
						TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE,  IID_IRow));
						SAFE_RELEASE(pIRow);

						// and so does the source
						if (NULL != rgpwszSourceURLs[cOp] && L'\0' != rgpwszSourceURLs[cOp][0])
						{
							TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszSourceURLs[cOp], 
								DBBINDURLFLAG_READ, DBGUID_ROW, 
								IID_IRow, NULL, NULL, NULL, 
								(IUnknown**)&pIRow), S_OK);
							TESTC(NULL != pIRow);
							TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE,  IID_IRow));
							SAFE_RELEASE(pIRow);
						}
					}
				}
				// at least one node was not copied
				TESTC(0 < ulBad && 0 < ulGood && ulGood < cRows);
			}
			break;

		case DB_E_ERRORSOCCURRED:
			TESTC(0<cRows);

			if (rgdwStatus)
			{
				// no row was copied
				for (cOp=0; cOp<cRows; cOp++)
				{
					COMPARE(CheckStatus(rgdwStatus[cOp], cValidStatus, rgValidStatus), TRUE);

					TESTC((dwCopyFlags & DBCOPY_ATOMIC)
						|| (DBSTATUS_S_OK != rgdwStatus[cOp]));


					if (DBSTATUS_S_OK == rgdwStatus[cOp])
					{
						ulGood++;					
	
						// copy is atomic
						// this particular source URL could be copied
						// binding to it should succeed
						TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszDestURLs[cOp], 
							DBBINDURLFLAG_READ, DBGUID_ROW, 
							IID_IRow, NULL, NULL, NULL, 
							(IUnknown**)&pIRow), DB_E_NOTFOUND);
						TESTC(NULL == pIRow);
					}
					else
						ulBad++;			
				}

				TESTC(0 < ulBad);
				if (dwCopyFlags & DBCOPY_ATOMIC)
				{
					TESTC(ulBad == cRows);
				}
				else
				{
					TESTC(ulGood == 0);
				}
			}
			break;

		case E_INVALIDARG:
			// some flags were not supported
			break;

		case DB_S_ASYNCHRONOUS:
			TESTC(dwCopyFlags & DBCOPY_ASYNC);
			break;

		case DB_E_ASYNCNOTSUPPORTED:
			TESTC(dwCopyFlags & DBCOPY_ASYNC);
			break;

		case E_OUTOFMEMORY:
		case E_UNEXPECTED:
		case E_FAIL:
		default:
			TESTC(FALSE);
	}

	hres = S_OK;

CLEANUP:
	SAFE_RELEASE(pIRow);
	PRVTRACE(L"</IScopedOperations_Copy>\n");
	return (S_OK == hres)? hr: E_FAIL;
} //IScopedOperations_Copy




//--------------------------------------------------------------------------------
//
// wrapper for calling IScopedOperations::Delete and doing  basic general checking
//--------------------------------------------------------------------------------
HRESULT IScopedOperations_Delete(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	DBCOUNTITEM			cRows,					// [in] number of delete ops	
	WCHAR				**rgpwszURLs,			// [in] URLs of the rows to be delete
	DWORD				dwDeleteFlags,			// [in] delete flags
	DBSTATUS			*rgdwStatus				// [out] filled on output
)
{
	HRESULT				hr;						// stores the result of the operation
	DBCOUNTITEM			cOp;					// loop index
	HRESULT				hres		= E_FAIL;	
	IRow				*pIRow		= NULL;
	DBCOUNTITEM			ulRow;

	// how many operations succeded and how many failed
	DBCOUNTITEM			ulGood		= 0;
	DBCOUNTITEM			ulBad		= 0;

	// all the legal statuses
	DBSTATUS			rgValidStatus[]	= {
											DBSTATUS_S_OK,
											DBSTATUS_E_CANCELED,
											DBSTATUS_E_DOESNOTEXIST,
											DBSTATUS_E_INVALIDURL,
											DBSTATUS_E_PERMISSIONDENIED,
											DBSTATUS_E_RESOURCELOCKED,
											DBSTATUS_E_RESOURCEOUTOFSCOPE,
											DBSTATUS_E_UNAVAILABLE,
											DBSTATUS_E_VOLUMENOTFOUND,
										};
	DBCOUNTITEM			cValidStatus = NUMELEM(rgValidStatus);

	// all the legal return values for the call
	HRESULT				rgValidRes[]	= {
											S_OK,
											DB_S_ASYNCHRONOUS,
											DB_S_ERRORSOCCURRED,
											DB_E_ASYNCNOTSUPPORTED,
											DB_E_CANCELED,
											DB_E_ERRORSOCCURRED,
											DB_SEC_E_SAFEMODE_DENIED,
											E_FAIL,
											E_INVALIDARG,
											E_OUTOFMEMORY,
											E_UNEXPECTED,
										};
	DBCOUNTITEM			cValidRes = NUMELEM(rgValidRes);


	PRVTRACE(L"<IScopedOperations_Delete>\n");

	TESTC(NULL != pIScopedOperations);

	PRVTRACE(L"\t<IScopedOperations::Delete cRows = %u rgRows = %p dwDeleteFlags = %u rgdwStatus = %p>\n", 
		cRows, rgpwszURLs, dwDeleteFlags, rgdwStatus);

	// the actual tested operation
	hr = pIScopedOperations->Delete(cRows, (const WCHAR**)rgpwszURLs, 
		dwDeleteFlags, rgdwStatus);

	// check and print the return value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);
	PRVTRACE(L"\t\t<RET_VAL value = %s> </RET_VAL>\n", GetErrorName(hr)); 

	// check and display statuses
	for (ulRow = 0; rgpwszURLs && ulRow < cRows; ulRow++)
	{
		PRVTRACE(L"\t\t<OPERATION name = DELETE url = %s status = %s> </OPERATION>\n", 
			((rgpwszURLs[ulRow])? rgpwszURLs[ulRow]: L"NULL"), 
			(rgdwStatus ? GetStatusName(rgdwStatus[ulRow]): L"unknown")
		);
	}

	PRVTRACE(L"\t</IScopedOperations::Delete>\n"); 


	// check that the return value was one mentioned by the spec
	switch (hr)
	{
		case S_OK:
			// check that none of the passed URL exist (can be bound)
			for (cOp=0; cOp<cRows; cOp++)
			{
				TESTC(!rgdwStatus || (DBSTATUS_S_OK == rgdwStatus[cOp]));
				TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszURLs[cOp], 
					DBBINDURLFLAG_READ, DBGUID_ROW, 
					IID_IRow, NULL, NULL, NULL, 
					(IUnknown**)&pIRow), DB_E_NOTFOUND);
				TESTC(NULL == pIRow);
			}
			break;
		
		case DB_S_ERRORSOCCURRED:
			TESTC(0<cRows);

			// the op was not atomic
			TESTC(0 == (dwDeleteFlags & DBDELETE_ATOMIC));

			if (rgdwStatus)
			{
				// some nodes were deleted, some not
				// check all the statuses
				for (cOp=0; cOp<cRows; cOp++)
				{
					COMPARE(CheckStatus(rgdwStatus[cOp], cValidStatus, rgValidStatus), TRUE);

					if (DBSTATUS_S_OK == rgdwStatus[cOp])
						ulGood++;
					else
						ulBad++;

					if (L'\0' == rgpwszURLs[cOp][0])
					{
						// if the row is the current one, no much testing cand be done
						// check that therow is in the zombie state
						CHECK(hres = pIScopedOperations->Delete(0, NULL, dwDeleteFlags, NULL), E_UNEXPECTED);
						continue;
					}

					if (	DBSTATUS_S_OK == rgdwStatus[cOp]
						||	DBSTATUS_E_DOESNOTEXIST == rgdwStatus[cOp]) 
					{
						TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszURLs[cOp], 
							DBBINDURLFLAG_READ, DBGUID_ROW, 
							IID_IRow, NULL, NULL, NULL, 
							(IUnknown**)&pIRow), DB_E_NOTFOUND);
						TESTC(NULL == pIRow);
					}
					else				
					{
						hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszURLs[cOp], 
							DBBINDURLFLAG_READ, DBGUID_ROW, 
							IID_IRow, NULL, NULL, NULL, 
							(IUnknown**)&pIRow);
						if (S_OK == hr)
						{
							TESTC(NULL != pIRow);
							TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE,  IID_IRow));
						}
					}
					SAFE_RELEASE(pIRow);
				}
				// at least one node was not deleted
				TESTC(0 < ulBad && 0 < ulGood && ulGood < cRows);
			}
			break;

		case DB_E_ERRORSOCCURRED:
			TESTC(0<cRows);

			if (rgdwStatus)
			{
				// no row was deleted: all are still there
				for (cOp=0; cOp<cRows; cOp++)
				{
					COMPARE(CheckStatus(rgdwStatus[cOp], cValidStatus, rgValidStatus), TRUE);

					TESTC((dwDeleteFlags & DBDELETE_ATOMIC)
						|| (DBSTATUS_S_OK != rgdwStatus[cOp]));


					if (DBSTATUS_S_OK == rgdwStatus[cOp])
					{
						ulGood++;
						
						if (L'\0' == rgpwszURLs[cOp][0])
							continue;
	
						// delete is atomic
						// this particular URL could be deleted
						// binding to it should succeed
						TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszURLs[cOp], 
							DBBINDURLFLAG_READ, DBGUID_ROW, 
							IID_IRow, NULL, NULL, NULL, 
							(IUnknown**)&pIRow), S_OK);
						TESTC(NULL != pIRow);
						TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE,  IID_IRow));
						SAFE_RELEASE(pIRow);
					}
					else
						ulBad++;			
				}
	
				TESTC(0 < ulBad);
				if (dwDeleteFlags & DBDELETE_ATOMIC)
				{
					TESTC(ulBad == cRows);
				}
				else
				{
					TESTC(0 == ulGood);
				}
			}
			break;

		case E_INVALIDARG:
			// one of the flags used was not supported
			break;

		case DB_S_ASYNCHRONOUS:
			TESTC(dwDeleteFlags & DBDELETE_ASYNC);
			break;

		case DB_E_ASYNCNOTSUPPORTED:
			TESTC(dwDeleteFlags & DBDELETE_ASYNC);
			break;

		case E_OUTOFMEMORY:
		case E_UNEXPECTED:
		case E_FAIL:
		default:
			COMPARE(FALSE, TRUE);
	}
	
	hres = S_OK;

CLEANUP:
	SAFE_RELEASE(pIRow);
	PRVTRACE(L"</IScopedOperations_Delete>\n");
	return (S_OK == hres)? hr: E_FAIL;
} //IScopedOperations_Delete




//--------------------------------------------------------------------------------
//
// wrapper for calling IScopedOperations::Move and doing  basic general checking
//--------------------------------------------------------------------------------
HRESULT IScopedOperations_Move(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	DBCOUNTITEM			cRows,					// [in] number of move ops	
	WCHAR				**rgpwszSourceURLs,		// [in] source URLs
	WCHAR				**rgpwszDestURLs,		// [in] destination URLs
	DWORD				dwMoveFlags,			// [in] Move flags
	IAuthenticate		*pIAuthenticate,		// [in] authentication interface
	DBSTATUS			*rgdwStatus,			// [out] filled on output
	WCHAR				**rgpwszNewURLs,		// [in/out] new URLs
	WCHAR				**ppStringsBuffer		// [out] buffer for URL strings
)
{
	HRESULT				hr;
	HRESULT				hres		= E_FAIL;
	DBCOUNTITEM			cOp;
	IRow				*pIRow		= NULL;
	DBCOUNTITEM			ulRow;

	// how many operations succeded and how many failed
	DBCOUNTITEM			ulGood		= 0;
	DBCOUNTITEM			ulBad		= 0;

	// all the legal statuses
	DBSTATUS			rgValidStatus[]	= {
											DBSTATUS_S_OK,
											DBSTATUS_E_CANCELED,
											DBSTATUS_S_CANNOTDELETESOURCE,
											DBSTATUS_E_CANNOTCOMPLETE,
											DBSTATUS_E_DOESNOTEXIST,
											DBSTATUS_E_INVALIDURL,
											DBSTATUS_E_NOTCOLLECTION,
											DBSTATUS_E_OUTOFSPACE,
											DBSTATUS_E_PERMISSIONDENIED,
											DBSTATUS_E_RESOURCEEXISTS,
											DBSTATUS_E_RESOURCELOCKED,
											DBSTATUS_E_RESOURCEOUTOFSCOPE,
											DBSTATUS_E_UNAVAILABLE,
											DBSTATUS_E_VOLUMENOTFOUND,
										};

	DBCOUNTITEM			cValidStatus = NUMELEM(rgValidStatus);

	// all the legal return values for the call
	HRESULT				rgValidRes[]	= {
											S_OK,
											DB_S_ASYNCHRONOUS,
											DB_S_BUFFERFULL,
											DB_S_ERRORSOCCURRED,
											DB_E_ASYNCNOTSUPPORTED,
											DB_E_CANCELED,
											DB_E_ERRORSOCCURRED,
											DB_SEC_E_SAFEMODE_DENIED,
											E_FAIL,
											E_INVALIDARG,
											E_OUTOFMEMORY,
											E_UNEXPECTED,
										};
	DBCOUNTITEM			cValidRes = NUMELEM(rgValidRes);

	
	PRVTRACE(L"<IScopedOperations_Move>\n");

	TESTC(NULL != pIScopedOperations);

	PRVTRACE(L"\t<IScopedOperations::Move cRows = %u "
		L"rgpwszSourceURLs = %p rgpwszDestURLs = %p dwMoveFlags = %u "
		L"pIAuthenticate = %p rgdwStatus = %p rgpwszNewURLs = %p "
		L"ppStringsBuffer = %p>\n", cRows, rgpwszSourceURLs, rgpwszDestURLs, 
		dwMoveFlags, pIAuthenticate, rgdwStatus, rgpwszNewURLs, ppStringsBuffer);

	// the actual tested operation
	hr = pIScopedOperations->Move(cRows, (const WCHAR**)rgpwszSourceURLs,
		(const WCHAR**)rgpwszDestURLs, dwMoveFlags, pIAuthenticate, rgdwStatus,
		rgpwszNewURLs, ppStringsBuffer);

	// check and print the return value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);
	PRVTRACE(L"\t\t<RET_VAL value = %s> </RET_VAL>\n", GetErrorName(hr)); 

	// check and display statuses
	for (ulRow = 0; rgpwszSourceURLs && rgpwszDestURLs && ulRow < cRows; ulRow++)
	{
		PRVTRACE(L"\t\t<OPERATION name = move source_url = %s dest_url = %s "
			L" new_url = %s status = %s> </OPERATION>\n", 
			((rgpwszSourceURLs[ulRow])? rgpwszSourceURLs[ulRow]: L"NULL"), 
			((rgpwszDestURLs[ulRow])? rgpwszDestURLs[ulRow]: L"NULL"), 
			((rgpwszNewURLs && ppStringsBuffer && rgpwszNewURLs[ulRow])? rgpwszNewURLs[ulRow]: L"NULL"), 
			(rgdwStatus ? GetStatusName(rgdwStatus[ulRow]): L"unknown")
		);
	}

	PRVTRACE(L"\t</IScopedOperations::Move>\n"); 

	switch (hr)
	{
		case S_OK:
			for (cOp=0; cOp<cRows; cOp++)
			{
				WCHAR	*pwszDestURL;

				//check status
				TESTC(!rgdwStatus || (DBSTATUS_S_OK == rgdwStatus[cOp]));

				// check destination
				pwszDestURL = rgpwszDestURLs? rgpwszDestURLs[cOp]: NULL;
				if (rgpwszNewURLs && ppStringsBuffer)
					pwszDestURL = rgpwszNewURLs[cOp];
				
				// check source
				TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszSourceURLs[cOp], 
					DBBINDURLFLAG_READ, DBGUID_ROW, 
					IID_IRow, NULL, NULL, NULL, 
					(IUnknown**)&pIRow), DB_E_NOTFOUND);
				TESTC(NULL == pIRow);

				// check destination
				TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, pwszDestURL, 
					DBBINDURLFLAG_READ, DBGUID_ROW, 
					IID_IRow, NULL, NULL, NULL, 
					(IUnknown**)&pIRow), S_OK);
				TESTC(NULL != pIRow);
				TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE,  IID_IRow));
				SAFE_RELEASE(pIRow);
			}
			break;
		
		case DB_S_ERRORSOCCURRED:
			TESTC(0 < cRows);

			// the op was not atomic, some nodes were copied, some not
			TESTC(0 == (dwMoveFlags & DBMOVE_ATOMIC));

			if (rgdwStatus)
			{
				for (cOp=0; cOp<cRows; cOp++)
				{
					// check valid status
					COMPARE(CheckStatus(rgdwStatus[cOp], cValidStatus, rgValidStatus), TRUE);
			
					if (DBSTATUS_S_OK == rgdwStatus[cOp])
						ulGood++;
					else
						ulBad++;

					if (DBSTATUS_S_OK == rgdwStatus[cOp])
					{
						WCHAR	*pwszDestURL;

						// check source doesn't exist anymore
						TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszSourceURLs[cOp], 
							DBBINDURLFLAG_READ, DBGUID_ROW, 
							IID_IRow, NULL, NULL, NULL, 
							(IUnknown**)&pIRow), DB_E_NOTFOUND);
						TESTC(NULL == pIRow);
			
						// check destination was created
						pwszDestURL = rgpwszDestURLs? rgpwszDestURLs[cOp]: NULL;
						if (rgpwszNewURLs && ppStringsBuffer)
							pwszDestURL = rgpwszNewURLs[cOp];
			
						TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, pwszDestURL, 
							DBBINDURLFLAG_READ, DBGUID_ROW, 
							IID_IRow, NULL, NULL, NULL, 
							(IUnknown**)&pIRow), S_OK);
						TESTC(NULL != pIRow);
						TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE,  IID_IRow));
						SAFE_RELEASE(pIRow);
					}
				}
				// at least one node was not moved
				TESTC(0 < ulBad && 0 < ulGood && ulGood < cRows);
			}
			break;

		case DB_E_ERRORSOCCURRED:
			TESTC(0 < cRows);

			if (rgdwStatus)
			{
				// no row was moved
				for (cOp=0; cOp<cRows; cOp++)
				{				
					COMPARE(CheckStatus(rgdwStatus[cOp], cValidStatus, rgValidStatus), TRUE);

					TESTC((dwMoveFlags & DBMOVE_ATOMIC)
						|| (DBSTATUS_S_OK != rgdwStatus[cOp]));

					if (DBSTATUS_S_OK == rgdwStatus[cOp])
					{
						ulGood++;					
	
						// move is atomic
						// this particular URL could be moved
						// binding to it should succeed
						TESTC_(hres = GetModInfo()->GetRootBinder()->Bind(NULL, rgpwszSourceURLs[cOp], 
							DBBINDURLFLAG_READ, DBGUID_ROW, 
							IID_IRow, NULL, NULL, NULL, 
							(IUnknown**)&pIRow), S_OK);
						TESTC(NULL != pIRow);
						TESTC(DefaultInterfaceTesting(pIRow, ROW_INTERFACE,  IID_IRow));
						SAFE_RELEASE(pIRow);
					}
					else
						ulBad++;			
				}

				TESTC(0 < ulBad);
				if (dwMoveFlags & DBMOVE_ATOMIC)
				{
					TESTC(ulBad == cRows);
				}
				else
				{
					TESTC(ulGood == 0);
				}
			}
			break;

		case E_INVALIDARG:
			// the verification was made by checking args, expected val and ret val
			break;

		case DB_S_ASYNCHRONOUS:
			TESTC(dwMoveFlags & DBMOVE_ASYNC);
			break;

		case DB_E_ASYNCNOTSUPPORTED:
			TESTC(dwMoveFlags & DBMOVE_ASYNC);
			break;

		case E_OUTOFMEMORY:
		case E_UNEXPECTED:
		case E_FAIL:
		default:
			TESTC(FALSE);
	}

	hres = S_OK;

CLEANUP:
	SAFE_RELEASE(pIRow);
	PRVTRACE(L"</IScopedOperations_Move>\n");
	return (S_OK == hres)? hr: E_FAIL;
} //IScopedOperations_Move



//--------------------------------------------------------------------------------
//
// wrapper for calling IScopedOperations::OpenRowset and making general checking
//--------------------------------------------------------------------------------
HRESULT IScopedOperations_OpenRowset(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	IUnknown			*pUnkOuter,				// [in] controlling IUnknown of the rowset (or row)
	DBID				*pTableID,				// [in] URL of the row or rowset
	DBID				*pIndexID,				// [in] should be ignored
	REFIID				riid,					// [in] interface to be retrieved
	ULONG				cPropertySets,			// [in] number of elements in prop array
	DBPROPSET			*rgPropertySets,		// [in|out] property array
	IUnknown			**ppRowset				// [out] row or rowset interface
)
{
	HRESULT				hr;
	HRESULT				hres		= E_FAIL;
	IRowset				*pIRowset	= NULL;
	IRow				*pIRow		= NULL;

	// all the legal return values for the call
	HRESULT				rgValidRes[]	= {
											S_OK,
											DB_S_ASYNCHRONOUS,
											DB_S_ERRORSOCCURRED,
											DB_S_STOPLIMITREACHED,
											DB_E_ABORTLIMITREACHED,
											DB_E_ERRORSOCCURRED,
											DB_E_NOAGGREGATION,
											DB_E_NOINDEX,
											DB_E_NOTABLE,
											DB_E_NOTCOLLECTION,
											DB_E_OBJECTOPEN,
											DB_E_RESOURCEOUTOFSCOPE,
											DB_SEC_E_PERMISSIONDENIED,
											E_FAIL,
											E_INVALIDARG,
											E_NOINTERFACE,
											E_OUTOFMEMORY,
											E_UNEXPECTED,
										};
	DBCOUNTITEM			cValidRes = NUMELEM(rgValidRes);

	
	PRVTRACE(L"<IScopedOperations_OpenRowset>\n");

	TESTC(NULL != pIScopedOperations);

	PRVTRACE(L"\t<IScopedOperations::OpenRowset pUnkOuter = %p"
		L"pTableID = %s pIndexID = %s riid = %s"
		L"cPropertySets %u rgPropertySets = %p ppRowset = %p>\n", pUnkOuter, 
		(pTableID && DBKIND_NAME == pTableID->eKind)? pTableID->uName.pwszName: L"NULL", 
		(pIndexID && DBKIND_NAME == pIndexID->eKind)? pIndexID->uName.pwszName: L"NULL", 
		GetInterfaceName(riid),
		cPropertySets, rgPropertySets, ppRowset);

	// the actual tested operation
	hr = pIScopedOperations->OpenRowset(pUnkOuter, pTableID, pIndexID, riid,
		cPropertySets, rgPropertySets, ppRowset);
	
	// check and print the return value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);
	PRVTRACE(L"\t\t<RET_VAL value = %s> </RET_VAL>\n", GetErrorName(hr)); 

	PRVTRACE(L"\t</IScopedOperations::OpenRowset>\n"); 

	if (NULL != pUnkOuter && IID_IUnknown != riid)
	{
		TESTC_(hr, DB_E_NOAGGREGATION);
	}

	if (FAILED(hr))
	{
		TESTC(NULL == ppRowset || NULL == *ppRowset);
	}
	else
	{
		if (ppRowset)
		{
			TESTC(NULL != *ppRowset);
			if (S_OK == (*ppRowset)->QueryInterface(IID_IRowset, (LPVOID*)&pIRowset))
			{
				TESTC(DefaultObjectTesting(*ppRowset, ROWSET_INTERFACE));
			}
			else if ((*ppRowset)->QueryInterface(IID_IRow, (LPVOID*)&pIRow))
			{
				TESTC(DefaultObjectTesting(*ppRowset, ROW_INTERFACE));
			}
		}
	}

	hres = S_OK;

CLEANUP:
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowset);
	PRVTRACE(L"</IScopedOperations_OpenRowset>\n");
	return (S_OK == hres)? hr: E_FAIL;
} //IScopedOperations_OpenRowset




//------------------------------------------------------------------------------
//
// wrapper for calling IScopedOperations_Bind
//------------------------------------------------------------------------------
HRESULT IScopedOperations_Bind(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	IUnknown			*pUnkOuter,				// [in] controlling IUnknown
	LPCOLESTR			pwszURL,				// [in] object to be bound
	DBBINDURLFLAG		dwBindFlags,			// [in] flags to be used for binding
	REFGUID				rguid,					// [in] indicates the type of the object being requested
	REFIID				riid,					// [in] requested interface
	IAuthenticate		*pAuthenticate,			// [in] pointer to IAuthenticate interface to be used
	DBIMPLICITSESSION	*pImplSession,			// [in] implicit session	
	DBBINDURLSTATUS		*pdwBindStatus,			// [out] bind status
	IUnknown			**ppUnk					// [out] interface on the bound object
)
{
	HRESULT				hr;
	HRESULT				hres				= E_FAIL;

	// all the legal statuses
	DBSTATUS			rgValidStatus[]	= {
											DBBINDURLSTATUS_S_OK,
											DBBINDURLSTATUS_S_DENYNOTSUPPORTED,
											DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED,
											DBBINDURLSTATUS_S_REDIRECTED, 
										};

	DBCOUNTITEM			cValidStatus = NUMELEM(rgValidStatus);

	// all the legal return values for the call
	HRESULT				rgValidRes[]	= {
											S_OK,
											DB_S_ASYNCHRONOUS,
											DB_S_ERRORSOCCURRED,
											DB_E_ASYNCNOTSUPPORTED,
											DB_E_CANNOTCONNECT,
											DB_E_NOAGGREGATION,
											DB_E_NOTCOLLECTION,
											DB_E_NOTFOUND,
											DB_E_NOTSUPPORTED,
											DB_E_OBJECTMISMATCH,
											DB_E_READONLY,
											DB_E_RESOURCELOCKED,
											DB_E_RESOURCEOUTOFSCOPE,
											DB_E_TIMEOUT,
											DB_SEC_E_PERMISSIONDENIED,
											DB_SEC_E_SAFEMODE_DENIED,
											REGDB_E_CLASSNOTREG,
											E_FAIL,
											E_INVALIDARG,
											E_NOINTERFACE,
											E_UNEXPECTED,
										};
	DBCOUNTITEM			cValidRes = NUMELEM(rgValidRes);

	
	PRVTRACE(L"<IScopedOperations_Bind>\n");

	TESTC(NULL != pIScopedOperations);

	PRVTRACE(L"\t<IScopedOperations::Bind pUnkOuter = %p pwszURL = %s dwBindFlags = %u"
		L" rguid = %s riid = %s pAuthenticate = %p pImplSession = %p"
		L" pdwBindStatus = %p ppUnk = %p>\n", pUnkOuter, pwszURL, dwBindFlags,
		GetObjectTypeName(rguid), 
		GetInterfaceName(riid), 
		pAuthenticate, pImplSession, pdwBindStatus, ppUnk);

	hr = pIScopedOperations->Bind(pUnkOuter, pwszURL, dwBindFlags,
		rguid, riid, pAuthenticate, pImplSession, pdwBindStatus, ppUnk);

	// check and print the return value
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);
	PRVTRACE(L"\t\t<RET_VAL value = %s> </RET_VAL>\n", GetErrorName(hr)); 

	if (pdwBindStatus)
		PRVTRACE(L"\t\t<STATUS value = %s> </STATUS>\n", GetBindURLStatusName(*pdwBindStatus)); 

	TESTC(	FAILED(hr) || DBGUID_ROW == rguid 
		||	DBGUID_ROWSET == rguid || DBGUID_STREAM == rguid);

	if(hr == S_OK)
	{
		//Check returned interface pointer.
		TESTC(NULL != *ppUnk);

		//Check bind status.
		if(pdwBindStatus)
			COMPARE(*pdwBindStatus, DBBINDURLSTATUS_S_OK);

		//Do some default object testing on obtained object.
		if(DBGUID_ROW == rguid)
			COMPARE(DefaultObjectTesting(*ppUnk, ROW_INTERFACE), TRUE);
		else if(DBGUID_ROWSET == rguid)
			COMPARE(DefaultObjectTesting(*ppUnk, ROWSET_INTERFACE), TRUE);
		else if(DBGUID_STREAM == rguid)
			COMPARE(DefaultObjectTesting(*ppUnk, STREAM_INTERFACE), TRUE);
	}

	if((DB_S_ERRORSOCCURRED == hr) && pdwBindStatus)
	{
		if(*pdwBindStatus == DBBINDURLSTATUS_S_DENYNOTSUPPORTED
			|| *pdwBindStatus == DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED)
		{
			TESTC(0 != (dwBindFlags &	(	DBBINDURLFLAG_SHARE_DENY_READ 
										|	DBBINDURLFLAG_SHARE_DENY_WRITE 
										|	DBBINDURLFLAG_SHARE_EXCLUSIVE 
										|	DBBINDURLFLAG_SHARE_DENY_NONE)) );
		}
		else if(*pdwBindStatus == DBBINDURLSTATUS_S_REDIRECTED)
		{
			odtLog<<L"WARNING: Got DBBINDURLSTATUS_S_REDIRECTED.\n";
			CHECKW(hr, S_OK);
		}
		else
		{
			odtLog<<L"ERROR: Status returned is" << *pdwBindStatus << ".\n";
			TESTC(FALSE);
		}
	}

	hres = S_OK;

CLEANUP:
	PRVTRACE(L"</IScopedOperations_Bind>\n");
	return (S_OK == hres)? hr: E_FAIL;
} //IScopedOperations_Bind

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// String comparisons used for restrictions, etc
//		This function compares two international WCHAR strings for equality, 
//		using a call to CompareStringA (CompareStringW not supported Win95).
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
LONG RelCompareString(LPWSTR pwsz1, LPWSTR pwsz2)
{
	LONG fComp = -1;

	if (GetModInfo()->IsWin9x())
	{

		LPSTR psz1 = ConvertToMBCS(pwsz1);
		LPSTR psz2 = ConvertToMBCS(pwsz2);

		fComp = RelCompareString(psz1, psz2);

		SAFE_FREE(psz1);
		SAFE_FREE(psz2);
	}
	else
	{
		fComp = CompareStringW(LOCALE_USER_DEFAULT, 
			NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH | SORT_STRINGSORT, 
			pwsz1, -1, pwsz2, -1); 

		// Convert to runtime values -1, 0, 1
		if (fComp)
			fComp -=2;
	}

	return fComp;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// String comparisons used for restrictions, etc
//		This function compares two international CHAR strings for equality, 
//		using a call to CompareStringA (CompareStringW not supported Win95).
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
LONG RelCompareString(LPSTR psz1, LPSTR psz2)
{
	LONG fComp = CompareStringA(LOCALE_USER_DEFAULT, 
		NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH | SORT_STRINGSORT, 
		psz1, -1, psz2, -1); 

	// Convert to runtime values -1, 0, 1
	if (fComp)
		fComp -=2;

	return fComp;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Relative compare of two numeric or varnumeric values for LT, GT, EQ
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
LONG RelCompareNumeric(BYTE * pVal1,
					   BYTE sign1,
					   SBYTE scale1,
					   USHORT cbVal1,
					   BYTE * pVal2,
					   BYTE sign2,
					   SBYTE scale2,
					   USHORT cbVal2
)
{
	USHORT scalediff;
	BYTE * pValMinScale;
	BYTE * pValMaxScale;
	USHORT cbMinScale, cbMaxScale, cbMax;
	USHORT iByte;
	LONG fComp = 0;

	// Note that precision isn't used here.  For varnumeric any unused bytes when precision bytes
	// are less than cbVal must be set to zero.  Therefore they're not important and we can ignore
	// the embedded precision value in both varnumerics.

	// If we must consider precision, use the formula: bytes = ceil(log(pow(10,p)-1)/(8*log(2))) to 
	// determine the significant bytes given precision p.

	// If the signs aren't equal we can tell relative size just from that
	if ( sign1 > sign2)
		return 1;

	if ( sign1 < sign2)
		return -1;
	
	// Find the one with the minimum scale
	if (scale2 >= scale1)
	{
		scalediff = scale2 - scale1;
		pValMinScale = pVal1 - scalediff;
		pValMaxScale = pVal2;
		cbMinScale = cbVal1;
		cbMaxScale = cbVal2;
	}
	else
	{
		scalediff = scale1 - scale2;
		pValMinScale = pVal2 - scalediff;
		pValMaxScale = pVal1;
		cbMinScale = cbVal2;
		cbMaxScale = cbVal1;
	}

	cbMax = max(cbMinScale+scalediff, cbMaxScale);

	for (iByte = cbMax, fComp = 0; iByte > 0 && !fComp; iByte--)
	{
		BYTE bMax = (iByte > cbMaxScale) ? 0 : pValMaxScale[iByte-1];
		BYTE bMin = (iByte < scalediff || iByte > cbMinScale+scalediff) ? 0 : pValMinScale[iByte-1];

		fComp = (bMin < bMax) ? -1 : (bMin > bMax) ? 1 : 0;
	}

	// If we reversed the pointers need to reverse the sense of the compare
	if (scale2 < scale1)
		fComp = 0-fComp;

	// If the values were negative we need to reverse the sense of the compare
	if (sign1 == 0)
		fComp = 0-fComp;

	return fComp;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Relative compare of any two values for LT, GT, EQ
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
LONG RelativeCompare(
	void	*pConsumerData,	//@parm [in]: the pointer to consumer data
	void	*pBackEndData,	//@parm [in]: the pointer to data at the backend
	DBTYPE	wType,			//@parm [in]: the DBType of the data.  It can not be ORed with any DBType modifers.
	USHORT	uswBackEndSize,	//@parm [in]: the size of the data, only valid for DBTYPE_BYTES or DBTYPE_VARNUMERIC
	BYTE	bPrecision,		//@parm [in]: the precision Only valid for DBTYPE_NUMERIC or DBTYPE_DECIMAL
	BYTE	bScale,			//@parm [in]: the scale Only valid for DBTYPE_NUMERIC or DBTYPE_DECIMAL
	ULONG	cbConsumerSize	//@parm [in]: the size of data pointed to by pConsumerData, only valid for DBTYPE_VARNUMERIC
)
{
	LONG fComp = -1;		// Assume pConsumerData < pBackEndData

	// Input validation
	ASSERT(pConsumerData);
	ASSERT(pBackEndData);
	
	// The input data should not be ORed with any type modifiers
	if ((wType & DBTYPE_RESERVED ) ||
		(wType & DBTYPE_ARRAY)	   ||
		(wType & DBTYPE_VECTOR)    ||
		(wType & DBTYPE_BYREF ))
		goto END;

	switch(wType)
	{

		case DBTYPE_I1:
				if (*(CHAR *)pConsumerData > *(CHAR *)pBackEndData)
					fComp = 1;
				else if (*(CHAR *)pConsumerData == *(CHAR *)pBackEndData)
					fComp = 0;
			goto END;

		case DBTYPE_BOOL:
				// Variant bool, -1 = TRUE, 0 = FALSE
				if (*(SHORT *)pConsumerData < *(SHORT *)pBackEndData)
					fComp=1;
				else if (*(SHORT *)pConsumerData == *(SHORT *)pBackEndData)
					fComp=0;
			goto END;
		case DBTYPE_I2:
				if (*(SHORT *)pConsumerData > *(SHORT *)pBackEndData)
					fComp=1;
				else if (*(SHORT *)pConsumerData == *(SHORT *)pBackEndData)
					fComp=0;
			goto END;

		case DBTYPE_I4:
				if (*(LONG *)pConsumerData > *(LONG *)pBackEndData)
					fComp=1;
				if (*(LONG *)pConsumerData == *(LONG *)pBackEndData)
					fComp=0;
			goto END;

		case DBTYPE_CY:
		case DBTYPE_FILETIME:
		case DBTYPE_I8:
				if (*(LONGLONG *)pConsumerData > *(LONGLONG *)pBackEndData)
					fComp=1;
				if (*(LONGLONG *)pConsumerData == *(LONGLONG *)pBackEndData)
					fComp=0;
			goto END;

		case DBTYPE_UI1:
				if (*(UCHAR *)pConsumerData > *(UCHAR *)pBackEndData)
					fComp = 1;
				else if (*(UCHAR *)pConsumerData == *(UCHAR *)pBackEndData)
					fComp = 0;
			goto END;

		case DBTYPE_UI2:
				if (*(USHORT *)pConsumerData > *(USHORT *)pBackEndData)
					fComp=1;
				else if (*(USHORT *)pConsumerData == *(USHORT *)pBackEndData)
					fComp=0;
			goto END;

		case DBTYPE_UI4:
				if (*(ULONG *)pConsumerData > *(ULONG *)pBackEndData)
					fComp=1;
				if (*(ULONG *)pConsumerData == *(ULONG *)pBackEndData)
					fComp=0;
			goto END;

		case DBTYPE_UI8:
				if (*(ULONGLONG *)pConsumerData > *(ULONGLONG *)pBackEndData)
					fComp=1;
				if (*(ULONGLONG *)pConsumerData == *(ULONGLONG *)pBackEndData)
					fComp=0;
			goto END;


		case DBTYPE_R4:
				if (*(float *)pConsumerData > *(float *)pBackEndData)
					fComp=1;
				if (*(float *)pConsumerData == *(float *)pBackEndData)
					fComp=0;
			goto END;

		case DBTYPE_R8:
		case DBTYPE_DATE:
				if (*(double *)pConsumerData > *(double *)pBackEndData)
					fComp=1;
				if (*(double *)pConsumerData == *(double *)pBackEndData)
					fComp=0;
			goto END;

		case DBTYPE_NUMERIC:
			fComp = RelCompareNumeric(
				((DB_NUMERIC *)pConsumerData)->val,
				((DB_NUMERIC *)pConsumerData)->sign,
				((DB_NUMERIC *)pConsumerData)->scale,
				16,
				((DB_NUMERIC *)pBackEndData)->val,
				((DB_NUMERIC *)pBackEndData)->sign,
				((DB_NUMERIC *)pBackEndData)->scale,
				16);
			goto END;

		case DBTYPE_VARNUMERIC:
			fComp = RelCompareNumeric(
				((DB_VARNUMERIC *)pConsumerData)->val,
				((DB_VARNUMERIC *)pConsumerData)->sign,
				((DB_VARNUMERIC *)pConsumerData)->scale,
				uswBackEndSize-sizeof(DB_VARNUMERIC)+1,
				((DB_VARNUMERIC *)pBackEndData)->val,
				((DB_VARNUMERIC *)pBackEndData)->sign,
				((DB_VARNUMERIC *)pBackEndData)->scale,
				(USHORT)cbConsumerSize-sizeof(DB_VARNUMERIC)+1);
			goto END;

		case DBTYPE_BYTES:
				fComp = memcmp(pConsumerData, pBackEndData,uswBackEndSize);
			goto END;

		case DBTYPE_STR:
				fComp = RelCompareString((CHAR *)pConsumerData, (CHAR *)pBackEndData);
			goto END;

		case DBTYPE_BSTR:
		case DBTYPE_WSTR:
				fComp = RelCompareString((WCHAR *)pConsumerData, (WCHAR *)pBackEndData);
			goto END;

		case DBTYPE_VARIANT:
				// Use VarCmp.  Note VarCmp has a bug and returns < when > so
				// we swap arguments.
				fComp = 1 - VarCmp(
					(VARIANT *)pBackEndData,
					(VARIANT *)pConsumerData,
					GetSystemDefaultLCID(),
					NORM_IGNOREWIDTH | NORM_IGNOREKANATYPE);

				/*  This code worked for VT_BSTR and should work for any matching vt
					but now that we're using multiple vt's we need to do better.
					Leave in because we may have to do something similar for some
					vt's that VarCmp doesn't handle.

				// Extract variant type
				wType = ((VARIANT *)pBackEndData)->vt;

				// For non-pointer types we need to give the address of the type to 
				// RelativeCompare
				if (wType & VT_BYREF || wType == VT_BSTR)
				{
					pConsumerData = (void *)((VARIANT *)pConsumerData)->bstrVal;
					pBackEndData = (void *)((VARIANT *)pBackEndData)->bstrVal;
				}
				else
				{
					pConsumerData = &(((VARIANT *)pConsumerData)->bstrVal);
					pBackEndData = &(((VARIANT *)pBackEndData)->bstrVal);
				}

				fComp=RelativeCompare(pConsumerData, pBackEndData, wType, uswBackEndSize, bPrecision, bScale, cbConsumerSize);
				*/

			goto END;

		case DBTYPE_GUID:
				fComp = memcmp(pConsumerData, pBackEndData, sizeof(GUID));
			goto END;

		case DBTYPE_ERROR:
				if (*(SCODE *)pConsumerData > *(SCODE *)pBackEndData)
					fComp=1;
				if (*(SCODE *)pConsumerData == *(SCODE *)pBackEndData)
					fComp=0;
			goto END;

		// Use a generic function to compare Data Time structs defined in OLE DB
		case DBTYPE_DBDATE:
			if (((DBDATE *)pConsumerData)->year > ((DBDATE *)pBackEndData)->year)
				fComp = 1;
			else if (((DBDATE *)pConsumerData)->year == ((DBDATE *)pBackEndData)->year)
				fComp = memcmp((void *)((BYTE *)pConsumerData+sizeof(SHORT)), (void *)((BYTE *)pBackEndData+sizeof(SHORT)), 2*sizeof(USHORT));
			goto END;
		case DBTYPE_DBTIME:
				fComp = memcmp(pConsumerData, pBackEndData, sizeof(DBTIME));
			goto END;
		case DBTYPE_DBTIMESTAMP:
			if (((DBDATE *)pConsumerData)->year > ((DBDATE *)pBackEndData)->year)
				fComp = 1;
			else if (((DBDATE *)pConsumerData)->year == ((DBDATE *)pBackEndData)->year)
				fComp = memcmp((void *)((BYTE *)pConsumerData+sizeof(SHORT)), (void *)((BYTE *)pBackEndData+sizeof(SHORT)),
					5*sizeof(USHORT)+sizeof(ULONG));
			goto END;

		// We can't compare these types
		case DBTYPE_IDISPATCH:
		case DBTYPE_EMPTY:
		case DBTYPE_NULL:
		case DBTYPE_IUNKNOWN:
		case DBTYPE_UDT:
		default:
			ASSERT(!L"Can't compare this type.");
			goto END;
	}

END:
	// Fix up fComp since memcmp etc. will return >1, <-1.  Otherwise
	// we can't use result in a switch statement.
	if (fComp > 1)
		fComp = 1;
	if (fComp < -1)
		fComp = -1;

	return fComp;
}



//----------------------------------------------------------------------
// MiscFunc FreeConstraintDesc |
// Frees the memory associated with prgConstraints
// 
// @mfunc HRESULT|
// 
//----------------------------------------------------------------------
HRESULT FreeConstraintDesc(
	DBORDINAL			*pcConstraints,			// @parmopt [IN] Count of constraint
	DBCONSTRAINTDESC	**prgConstraints, 		// @parmopt [IN] Array of constraint desc
	BOOL				fFreeOuter /*=TRUE*/	// @parmopt [IN] Whether or not to free *prgConstraints
)
{
	HRESULT				hr = E_FAIL;
	DBORDINAL			index, indxCol;
	DBCONSTRAINTDESC	*pConstraint;


	TESTC(NULL != pcConstraints && NULL != prgConstraints);
	TESTC(0 == *pcConstraints || NULL != *prgConstraints);

	pConstraint = *prgConstraints;
	for (index=0; index < *pcConstraints; index++, pConstraint++)
	{
		// release memory for this element
		ReleaseDBID(pConstraint->pConstraintID, TRUE);
		if (DBCONSTRAINTTYPE_CHECK != pConstraint->ConstraintType)
		{
			// release rgColumnList
			if (pConstraint->rgColumnList)
				for (indxCol=0; indxCol < pConstraint->cColumns; indxCol++)
				{
					ReleaseDBID(&pConstraint->rgColumnList[indxCol], FALSE);
				}
			SAFE_FREE(pConstraint->rgColumnList);
		}
		else
			SAFE_FREE(pConstraint->pwszConstraintText);

		if (DBCONSTRAINTTYPE_FOREIGNKEY == pConstraint->ConstraintType)
		{
			// release rgForeignKeyColumnList
			if (pConstraint->rgForeignKeyColumnList)
				for (indxCol=0; indxCol < pConstraint->cForeignKeyColumns; indxCol++)
				{
					ReleaseDBID(&pConstraint->rgForeignKeyColumnList[indxCol], FALSE);
				}
			SAFE_FREE(pConstraint->rgForeignKeyColumnList);
			ReleaseDBID(pConstraint->pReferencedTableID, TRUE);
		}
		// when constraint properties will exist, release props as well
		//FreeProperties(&(pConstraint->cPropertySets), &(pConstraint->rgPropertySets));
	}

	if (fFreeOuter)
	{
		SAFE_FREE(*prgConstraints);
		*prgConstraints	= NULL;
	}
	else
	{
		//put 0 in the whole array
		memset(*prgConstraints, 0, sizeof(*prgConstraints));
	}

	*pcConstraints	= 0;
	hr = S_OK;

CLEANUP:
	return hr;

} //FreeConstraintDesc


//--------------------------------------------------------------------------------
//
// Check for DBID having valid pwszName
//--------------------------------------------------------------------------------
BOOL DBIDHasName(DBID dbid)
{
	switch(dbid.eKind)
	{
		case DBKIND_NAME:
		case DBKIND_GUID_NAME:
		case DBKIND_PGUID_NAME:
			if (dbid.uName.pwszName)
				return TRUE;
	}

	return FALSE;
}


//--------------------------------------------------------------------------------
//
// Maps a OLE DB DBTYPE to a VARIANT value
// The mappings are based on ADO
//--------------------------------------------------------------------------------
HRESULT	MapDBTYPE2VARIANT
(
	void *		pvSource,
	DBTYPE		wType,
	ULONG_PTR	cbData,
	VARIANT **	ppVariant
)
{
	HRESULT		hr = E_FAIL;
	VARIANT *	pVariant = NULL;

	//pvSource can only be NULL if creating a VARIANT of type VT_NULL or VT_EMPTY
	if( pvSource == NULL )
	{
		ASSERT(wType==VT_NULL || wType==VT_EMPTY);
	}

	if( ppVariant )
		*ppVariant = NULL;

	// Check if the source type is compatible with a variant
	if( IsVariantType(wType) )
	{
		pVariant = DBTYPE2VARIANT(pvSource, wType);
		TESTC( pVariant != NULL);
	}
	else
	{
		pVariant = (VARIANT*)PROVIDER_ALLOC(sizeof(VARIANT));
		TESTC(pVariant != NULL);

		VariantInit(pVariant);

		//We don't have Modifiers
		switch(wType)
		{
			case DBTYPE_I8:
			{
				DECIMAL			decVal;
		
				decVal.scale = 0;
				decVal.Hi32 = 0; 
				
				if (*(LONGLONG *)pvSource < 0 ) 
				{
					decVal.sign = 0x80;				
					decVal.Lo64 = - *(LONGLONG *)pvSource;		
				}
				else 
				{
					decVal.sign = 0;				
					decVal.Lo64 = *(LONGLONG *)pvSource;		
				}

				V_DECIMAL(pVariant) = decVal;
				V_VT(pVariant) = VT_DECIMAL;

				break;
			}

			case DBTYPE_UI8:
			{
				DECIMAL			decVal;
			
				decVal.sign = 0;
				decVal.scale = 0;
				decVal.Hi32 = 0; 
				decVal.Lo64 = *(ULONGLONG *)pvSource; 

				V_DECIMAL(pVariant) = decVal;
				V_VT(pVariant) = VT_DECIMAL;

				break;
			}

			case DBTYPE_NUMERIC:
			{
				DECIMAL			decVal;
				DB_NUMERIC *	pNum = (DB_NUMERIC *)pvSource;

				hr = S_FALSE;

				// Numeric scale must be within the DECIMAL's scale range
				QTESTC(pNum->scale <= 28);

				// Numeric value must only use 96 bits to be within DECIMAL's range
				QTESTC( *(ULONG *)&pNum->val[12] == 0 );

				decVal.sign = pNum->sign ? 0 : 0x80;
				decVal.scale = pNum->scale;
				decVal.Hi32 = *(ULONG *)&pNum->val[8]; 
				decVal.Lo64 = *(ULONGLONG *)&pNum->val[0]; 

				V_DECIMAL(pVariant) = decVal;
				V_VT(pVariant) = VT_DECIMAL;

				break;
			}

			case DBTYPE_VARNUMERIC:
			{
				DECIMAL			decVal;
				DB_VARNUMERIC *	pVarNum = (DB_VARNUMERIC *)pvSource;
				BYTE			rgbVal[12];

				hr = S_FALSE;

				// VarNumeric scale must be within the DECIMAL's scale range
				QTESTC(pVarNum->scale  <= 28);
				// VarNumeric value must only use 12 bytes of data
				QTESTC( cbData <= 12 );

				memset(rgbVal, 0, sizeof(rgbVal));
				memcpy(rgbVal, pVarNum->val, (ULONG)cbData);

				decVal.sign = pVarNum->sign ? 0 : 0x80;
				decVal.scale = pVarNum->scale;
				decVal.Hi32 = *(ULONG *)rgbVal[8];
				decVal.Lo64 = *(ULONGLONG *)rgbVal;

				V_DECIMAL(pVariant) = decVal;
				V_VT(pVariant) = VT_DECIMAL;

				break;
			}

			case DBTYPE_BYTES:
			{
				SAFEARRAYBOUND	rgsabound[1];

				// Create the safe array
				rgsabound[0].lLbound	= 0;

				// Data larger than ULONG_MAX cannot be mapped to a Variant
				QTESTC( cbData < ULONG_MAX );

				rgsabound[0].cElements	= (ULONG)cbData;
				V_ARRAY(pVariant) = SafeArrayCreate(VT_UI1, 1, rgsabound);

				// Lock the array to get a pointer to the data
				TESTC_(SafeArrayLock(V_ARRAY(pVariant)), S_OK);

				// Copy the data
				memcpy(V_ARRAY(pVariant)->pvData, pvSource, (ULONG)cbData);

				// Unlock the array
				SafeArrayUnlock(V_ARRAY(pVariant));

				V_VT(pVariant) = VT_UI1 | VT_ARRAY;
				break;
			}

			case DBTYPE_DBTIMESTAMP:
			{	
				SYSTEMTIME	SystemTime = { 0, 0, 0, 0, 0, 0, 0, 0 };
				DATE		dOleDate = 0;	

				SystemTime.wYear = ((DBTIMESTAMP *)pvSource)->year;
				SystemTime.wMonth = ((DBTIMESTAMP *)pvSource)->month;
				SystemTime.wDay = ((DBTIMESTAMP *)pvSource)->day;
				SystemTime.wHour = ((DBTIMESTAMP *)pvSource)->hour;
				SystemTime.wMinute = ((DBTIMESTAMP *)pvSource)->minute;
				SystemTime.wSecond = ((DBTIMESTAMP *)pvSource)->second;
				ASSERT( ((DBTIMESTAMP *)pvSource)->fraction <= 999999999 );
				SystemTime.wMilliseconds = (USHORT)((DBTIMESTAMP *)pvSource)->fraction/1000000;

				TESTC(SystemTimeToVariantTime(&SystemTime, &dOleDate));

				V_DATE(pVariant) = dOleDate;
				V_VT(pVariant) = VT_DATE;

				break;
			}

			case DBTYPE_DBDATE:
			{
				SYSTEMTIME	SystemTime = { 0, 0, 0, 0, 0, 0, 0, 0 };
				DATE		dOleDate = 0;	

				SystemTime.wYear = ((DBDATE *)pvSource)->year;
				SystemTime.wMonth = ((DBDATE *)pvSource)->month;
				SystemTime.wDay = ((DBDATE *)pvSource)->day;
				SystemTime.wHour = 0;
				SystemTime.wMinute = 0;
				SystemTime.wSecond = 0;
				SystemTime.wMilliseconds = 0;

				TESTC(SystemTimeToVariantTime(&SystemTime, &dOleDate));

				V_DATE(pVariant) = dOleDate;
				V_VT(pVariant) = VT_DATE;

				break;
			}

			case DBTYPE_DBTIME:
			{
				DATE		dOleDate = 0;	

				dOleDate =	((DBTIME *)pvSource)->hour * 3600 + 
							((DBTIME *)pvSource)->minute * 60 +
							((DBTIME *)pvSource)->second;
				dOleDate /= 86400;	// 86400 seconds in a day
				
				V_DATE(pVariant) = dOleDate;
				V_VT(pVariant) = VT_DATE;

				break;
			}

			case DBTYPE_WSTR:
			{
				V_BSTR(pVariant) = SysAllocString((WCHAR *)pvSource);
				V_VT(pVariant) = VT_BSTR;

				break;
			}
			default:
				hr = S_FALSE;
				goto CLEANUP;
				break;
		};
	}

	hr = S_OK;

CLEANUP:

	if( S_OK == hr )
	{
		*ppVariant = pVariant;
	}
	else
	{
		VariantClear(pVariant);
		SAFE_FREE(pVariant);
	}

	return hr;
}


//--------------------------------------------------------------------------------
//
// Returns TRUE if a DBTYPE maps directly to a VT type
//--------------------------------------------------------------------------------
BOOL IsVariantType
(
	DBTYPE	wType
)
{
	switch(wType)
	{
		case DBTYPE_NULL:
		case DBTYPE_EMPTY:
		case DBTYPE_I1:					
		case DBTYPE_UI2:					 			
		case DBTYPE_BOOL:
		case DBTYPE_BSTR:
		case DBTYPE_I2:			
		case DBTYPE_ERROR:				
		case DBTYPE_I4:					
		case DBTYPE_UI1:
		case DBTYPE_UI4:
		case DBTYPE_DATE:
		case DBTYPE_R8:
		case DBTYPE_R4:
		case DBTYPE_CY:		
		case DBTYPE_DECIMAL:			
			return TRUE;
		default:
			return FALSE;
	};
}


//--------------------------------------------------------------------------------
//
// Returns the ODBC SQL Type corresponding to a variant type
//--------------------------------------------------------------------------------
WCHAR * VariantTypeToSQLType
(
	DBTYPE wType
)
{
	switch(wType)
	{
	case DBTYPE_NULL:
		return L"SQL_CHAR";
	case DBTYPE_I1:		
	case DBTYPE_UI1:		
		return L"SQL_TINYINT";
	case DBTYPE_I2:	
	case DBTYPE_UI2:
	case DBTYPE_ERROR:
		return L"SQL_SMALLINT";
	case DBTYPE_BOOL:
		return L"SQL_BIT";
	case DBTYPE_STR:
		return L"SQL_VARCHAR";
	case DBTYPE_BSTR:
	case DBTYPE_WSTR:
		return L"SQL_WVARCHAR";
	case DBTYPE_I4:						
	case DBTYPE_UI4:
		return L"SQL_INTEGER";
	case DBTYPE_I8:
	case DBTYPE_UI8:
		return L"SQL_BIGINT";
	case DBTYPE_DATE:
	case DBTYPE_DBTIMESTAMP:
	case DBTYPE_DBDATE:
	case DBTYPE_DBTIME:
		return L"SQL_TYPE_TIMESTAMP";
	case DBTYPE_R8:
		return L"SQL_DOUBLE";
	case DBTYPE_R4:
		return L"SQL_REAL";
	case DBTYPE_CY:		
	case DBTYPE_DECIMAL:			
		return L"SQL_NUMERIC";
	case DBTYPE_BYTES:
		return L"SQL_VARBINARY";
	default:
		return NULL;
	}
}



//--------------------------------------------------------------------------------
//
// Returns TRUE if the DBTYPE is compatible with a VARIANT
//--------------------------------------------------------------------------------
BOOL IsVariantCompatible
(
	DBTYPE	wType,
	CCol *	pCol	
)
{
	if( !pCol->GetUpdateable() )
		return FALSE;

	switch(wType)
	{
		case DBTYPE_NULL:
		case DBTYPE_EMPTY:
		case DBTYPE_I1:					
		case DBTYPE_UI2:					 			
		case DBTYPE_BOOL:
		case DBTYPE_BSTR:
		case DBTYPE_I2:			
		case DBTYPE_ERROR:				
		case DBTYPE_I4:					
		case DBTYPE_UI1:
		case DBTYPE_UI4:
		case DBTYPE_DATE:
		case DBTYPE_R8:
		case DBTYPE_R4:
		case DBTYPE_CY:		
		case DBTYPE_DECIMAL:			
		case DBTYPE_WSTR:
		case DBTYPE_BYTES:
		case DBTYPE_DBTIMESTAMP:
		case DBTYPE_DBDATE:
		case DBTYPE_I8:
		case DBTYPE_UI8:
			return TRUE;
		case DBTYPE_NUMERIC:
			// If the numeric value falls with the range of a DECIMAL,
			// then allow it
			if( pCol && pCol->GetPrecision() <= 28 )
				return TRUE;
			else
				return FALSE;
		default:
			return FALSE;
	};
}

//////////////////////////////////////////////////////////////////////////
// CModInfo::OpenXMLAsText
//
//////////////////////////////////////////////////////////////////////////
char *OpenXMLAsText(WCHAR *pwszFilename)
{
	FILE	*stream;
	long	iFileSize;
	size_t  iNumRead ;
	char	*pszStream;

	QCOMPARE( (stream = _wfopen( pwszFilename, L"rb" )), NULL );

	TESTC_ ( fseek( stream, 0, SEEK_END ), 0 );				//Move to eof
	iFileSize = ftell(stream);
	TESTC_( fseek( stream, iFileSize*-1, SEEK_CUR ), 0 );	//Move to bof
	
	pszStream = (char*)PROVIDER_ALLOC((iFileSize+20)*sizeof(char));	
	QCOMPARE(pszStream, NULL);
	memset(pszStream, 0, iFileSize + 20);

	iNumRead = fread( pszStream, sizeof( char ), iFileSize, stream );	
	TRACE("[FileSize: %d]\t[NumRead: %d]\n", iFileSize,iNumRead);	

CLEANUP:
	return pszStream;
}

////////////////////////////////////////////////////////////////////////////
// IsSQLOLEDB
////////////////////////////////////////////////////////////////////////////
BOOL IsSQLOLEDB()
{
	CLSID	CLSID_SQLOLEDB;

	if ( CLSIDFromProgID(L"SQLOLEDB", &CLSID_SQLOLEDB) != S_OK)
		return FALSE;

	return GetModInfo()->GetProviderCLSID() == CLSID_SQLOLEDB;
}

////////////////////////////////////////////////////////////////////////////
// IsSQLNCLI
////////////////////////////////////////////////////////////////////////////
BOOL IsSQLNCLI()
{
	CLSID	CLSID_SQLNCLI;

	if ( CLSIDFromProgID(L"SQLNCLI", &CLSID_SQLNCLI) != S_OK)
		return FALSE;

	return GetModInfo()->GetProviderCLSID() == CLSID_SQLNCLI;
}

////////////////////////////////////////////////////////////////////////////
// IsSQLProvider - returns true if SQLOLEDB or SQLNCLI.
////////////////////////////////////////////////////////////////////////////
BOOL IsSQLProvider()
{
	return IsSQLOLEDB() || IsSQLNCLI();
}

////////////////////////////////////////////////////////////////////////////
// IsMSDASQL
////////////////////////////////////////////////////////////////////////////
BOOL IsMSDASQL()
{
	CLSID	CLSID_MSDASQL;

	if ( CLSIDFromProgID(L"MSDASQL", &CLSID_MSDASQL) != S_OK ) 
		return FALSE;

	return GetModInfo()->GetProviderCLSID() == CLSID_MSDASQL;
}

////////////////////////////////////////////////////////////////////////////
// IsSqlServer
////////////////////////////////////////////////////////////////////////////
BOOL IsSqlServer()
{	
	const WCHAR *pwszBackend = GetModInfo()->GetBackend();
	ASSERT(pwszBackend);
	return  wcscmp(pwszBackend, L"Microsoft SQL Server") == 0;
}

////////////////////////////////////////////////////////////////////////////
// EscapeChar
//	Escape any char in the document by doubling it
//////////////////////////////////////////////////////////////////////////
HRESULT EscapeChar(WCHAR **ppwszStr, WCHAR chEsc)
{
	HRESULT hr = E_FAIL;
	LPWSTR pS, pD, pwszStr;

	if (!ppwszStr)
		return E_INVALIDARG;

	pwszStr = *ppwszStr;

	// Allocate a new string to hold the escaped value, since it's larger
	LPWSTR pwszEsc = (WCHAR*)PROVIDER_ALLOC((MAXDATALEN)*sizeof(WCHAR));

	if (!pwszEsc)
		return E_OUTOFMEMORY;

	pS = pwszStr;
	pD = pwszEsc;
	while (*pS && (pD - pwszEsc) < MAXDATALEN)
	{
		if (*pS == chEsc)
		{
			*pD = chEsc;
			pD++;
		}
		*pD = *pS;
		pD++;
		pS++;
	}

	// Make sure the destination is null terminated in case adding
	// escape characters overflowed.
	if ((pD-pwszEsc)< MAXDATALEN)
	{
		*pD = L'\0';
		PROVIDER_FREE(pwszStr);
		*ppwszStr = pwszEsc;
		hr = S_OK;
	}
	else
		hr = E_FAIL;

	return hr;
}

